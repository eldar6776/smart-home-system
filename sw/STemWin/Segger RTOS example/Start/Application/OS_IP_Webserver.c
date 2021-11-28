/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2016  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : OS_IP_Webserver.c
Purpose : Sample program for embOS & TCP/IP
--------- END-OF-HEADER --------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "RTOS.h"
#include "BSP.h"
#include "IP.h"
#include "IP_Webserver.h"
#include "WEBS_Conf.h"        // Stack size depends on configuration

/*********************************************************************
*
*       Local defines, configurable
*
**********************************************************************
*/

#define USE_RX_TASK   0  // 0: Packets are read in ISR, 1: Packets are read in a task of its own.

//
// Web server and IP stack
//
#define MAX_CONNECTIONS           2
#define BACK_LOG                 20
#define MAX_CONNECTION_INFO      10
#define IDLE_TIMEOUT           1000  // Timeout [ms] after which the connection will be closed if no new data is received.
#define SERVER_PORT              80
//
// Task priorities.
//
enum {
   TASK_PRIO_WEBS_CHILD = 150
  ,TASK_PRIO_WEBS_PARENT
  ,TASK_PRIO_IP_TASK           // Priority must be higher as all IP application tasks.
#if USE_RX_TASK
  ,TASK_PRIO_IP_RX_TASK        // Must be the highest priority of all IP related tasks.
#endif
};

//
// Task stack sizes that might not fit for some interfaces (multiples of sizeof(int)).
//
#ifndef   APP_TASK_STACK_OVERHEAD
  #define APP_TASK_STACK_OVERHEAD     0
#endif

#ifndef   STACK_SIZE_SERVER
  #define STACK_SIZE_SERVER           (2000 + APP_TASK_STACK_OVERHEAD)
#endif

//
// UDP discover
//
#define ETH_UDP_DISCOVER_PORT    50020
#define PACKET_SIZE              0x80

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/
// None.

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U32 _aPool[((WEBS_IN_BUFFER_SIZE + WEBS_OUT_BUFFER_SIZE + WEBS_PARA_BUFFER_SIZE + WEBS_FILENAME_BUFFER_SIZE + WEBS_MAX_ROOT_PATH_LEN) * MAX_CONNECTIONS) / sizeof(int)];  // Memory pool for the Web server child tasks

static IP_HOOK_ON_STATE_CHANGE _StateChangeHook;
static int                     _IFaceId;
static int                     _ConnectCnt;

//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int _IPStack[TASK_STACK_SIZE_IP_TASK/sizeof(int)];       // Stack of the IP_Task.
static OS_TASK         _IPTCB;                                              // Task-Control-Block of the IP_Task.

#if USE_RX_TASK
static OS_STACKPTR int _IPRxStack[TASK_STACK_SIZE_IP_RX_TASK/sizeof(int)];  // Stack of the IP_RxTask.
static OS_TASK         _IPRxTCB;                                            // Task-Control-Block of the IP_RxTask.
#endif

//
// Webserver TCBs and stacks
//
static OS_TASK         _aWebTasks[MAX_CONNECTIONS];
static OS_STACKPTR int _aWebStacks[MAX_CONNECTIONS][STACK_SIZE_SERVER/sizeof(int)];

//
// File system info
//
static const IP_FS_API *_pFS_API;

//
// Variables used for METHOD hook demonstration (basic REST implementation)
//
static WEBS_METHOD_HOOK _MethodHook;
static char             _acRestContent[20];

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _CopyString
*
*  Function description
*    Copies a string.
*/
static void _CopyString(char *sDest, const char *sSrc, int DestSize) {
  char c;

  while (--DestSize > 0) {
    c = *sSrc++;
    if (c == 0) {
      break;
    }
    *sDest++ = c;
  }
  *sDest = 0;
}

/*********************************************************************
*
*       _OnStateChange()
*
* Function description
*   Callback that will be notified once the state of an interface
*   changes.
*
* Parameters
*   IFaceId   : Zero-based interface index.
*   AdminState: Is this interface enabled ?
*   HWState   : Is this interface physically ready ?
*/
static void _OnStateChange(unsigned IFaceId, U8 AdminState, U8 HWState) {
  //
  // Check if this is a disconnect from the peer or a link down.
  // In this case call IP_Disconnect() to get into a known state.
  //
  if (((AdminState == IP_ADMIN_STATE_DOWN) && (HWState == 1)) ||  // Typical for dial-up connection e.g. PPP when closed from peer. Link up but app. closed.
      ((AdminState == IP_ADMIN_STATE_UP)   && (HWState == 0))) {  // Typical for any Ethernet connection e.g. PPPoE. App. opened but link down.
    IP_Disconnect(IFaceId);                                       // Disconnect the interface to a clean state.
  }
}

/*********************************************************************
*
*       _closesocket()
*
*  Function description
*    Wrapper for closesocket()
*/
static int _closesocket(long pConnectionInfo) {
  int r;
  struct linger Linger;

  Linger.l_onoff  = 1;  // Enable linger for this socket to verify that all data is send.
  Linger.l_linger = 1;  // Linger timeout in seconds
  setsockopt((long)pConnectionInfo, SOL_SOCKET, SO_LINGER, &Linger, sizeof(Linger));
  r = closesocket((long)pConnectionInfo);
  return r;
}

/*********************************************************************
*
*       _Recv()
*
*  Function description
*    Wrapper for recv()
*/
static int _Recv(unsigned char *buf, int len, void *pConnectionInfo) {
  int r;

  r = recv((long)pConnectionInfo, (char *)buf, len, 0);
  return r;
}

/*********************************************************************
*
*       _Send()
*
*  Function description
*    Wrapper for send()
*/
static int _Send(const unsigned char *buf, int len, void* pConnectionInfo) {
  int r;

  r = send((long)pConnectionInfo, (const char *)buf, len, 0);
  return r;
}

/*********************************************************************
*
*       WEBS_IP_API
*
*  Description
*   IP related function table
*/
static const WEBS_IP_API _Webs_IP_API = {
  _Send,
  _Recv
};

/*********************************************************************
*
*       _Alloc()
*
*  Function description
*    Wrapper for Alloc(). (embOS/IP: IP_MEM_Alloc())
*/
static void * _Alloc(U32 NumBytesReq) {
  return IP_Alloc(NumBytesReq);
}

/*********************************************************************
*
*       _Free()
*
*  Function description
*    Wrapper for Alloc(). (embOS/IP: IP_MEM_Alloc())
*/
static void _Free(void *p) {
  IP_Free(p);
}

/*********************************************************************
*
*       WEBS_SYS_API
*
*  Description
*   System related function table
*/
static const WEBS_SYS_API _Webs_SYS_API = {
  _Alloc,
  _Free
};

/*********************************************************************
*
*       _CB_HandleParameter
*/
static void _CB_HandleParameter(WEBS_OUTPUT *pOutput, const char *sPara, const char *sValue) {
  IP_USE_PARA(pOutput);
  IP_USE_PARA(sPara);
  IP_USE_PARA(sValue);
}

/*********************************************************************
*
*       WEBS_APPLICATION
*
*  Function description
*    Application data table, defines all application specifics used by the web server
*/
static const WEBS_APPLICATION _Application = {
  &aCGI[0],
  &aAccessControl[0],
  _CB_HandleParameter,
  &aVFiles[0]
};

/*********************************************************************
*
*       _GetTimeDate
*
*  Function description:
*    Returns current time and date.
*
*    Bit 0-4:   2-second count (0-29)
*    Bit 5-10:  Minutes (0-59)
*    Bit 11-15: Hours (0-23)
*    Bit 16-20: Day of month (1-31)
*    Bit 21-24: Month of year (1-12)
*    Bit 25-31: Count of years from 1980 (0-127)
*
*  Note:
*    This is a sample implementation for a clock-less system.
*    It always returns 01 Jan 2009 00:00:00 GMT
*/
static U32 _GetTimeDate(void) {
  U32 r;
  U16 Sec, Min, Hour;
  U16 Day, Month, Year;

  Sec   = 0;        // 0 based.  Valid range: 0..59
  Min   = 0;        // 0 based.  Valid range: 0..59
  Hour  = 0;        // 0 based.  Valid range: 0..23
  Day   = 1;        // 1 based.    Means that 1 is 1. Valid range is 1..31 (depending on month)
  Month = 1;        // 1 based.    Means that January is 1. Valid range is 1..12.
  Year  = 29;       // 1980 based. Means that 2008 would be 28.
  r   = Sec / 2 + (Min << 5) + (Hour  << 11);
  r  |= (U32)(Day + (Month << 5) + (Year  << 9)) << 16;
  return r;
}

/*********************************************************************
*
*       _pfGetFileInfo
*/
static void _pfGetFileInfo(const char *sFilename, IP_WEBS_FILE_INFO *pFileInfo){
  int v;

  //
  // .cgi files are virtual, everything else is not
  //
  v = IP_WEBS_CompareFilenameExt(sFilename, ".cgi");
  pFileInfo->IsVirtual = v ? 0 : 1;
  //
  // .htm files contain dynamic content, everything else is not
  //
  v = IP_WEBS_CompareFilenameExt(sFilename, ".htm");
  pFileInfo->AllowDynContent = v ? 0 : 1;
  //
  // If file is a virtual file or includes dynamic content,
  // get current time and date stamp as file time
  //
  pFileInfo->DateLastMod = _GetTimeDate();
  if (pFileInfo->IsVirtual || pFileInfo->AllowDynContent) {
    //
    // Set expiration time and date
    //
    pFileInfo->DateExp = _GetTimeDate(); // If "Expires" HTTP header field should be transmitted, set expiration date.
  } else {
    pFileInfo->DateExp = 0xEE210000;     // Expiration far away (01 Jan 2099) if content is static
  }
}

/*********************************************************************
*
*       _AddToConnectCnt
*
*/
static void _AddToConnectCnt(int Delta) {
  OS_IncDI();
  _ConnectCnt += Delta;
  OS_DecRI();
}

/*********************************************************************
*
*       _WebServerChildTask
*
*/
static void _WebServerChildTask(void *pContext) {
  WEBS_CONTEXT ChildContext;
  long hSock;
  int Opt;

  hSock    = (long)pContext;
  Opt      = 1;
  setsockopt(hSock, SOL_SOCKET, SO_KEEPALIVE, &Opt, sizeof(Opt));
  IP_WEBS_Init(&ChildContext, &_Webs_IP_API, &_Webs_SYS_API, _pFS_API, &_Application);  // Initialize the context of the child task.
  if (_ConnectCnt < MAX_CONNECTIONS) {
    IP_WEBS_ProcessEx(&ChildContext, pContext, NULL);
  } else {
    IP_WEBS_ProcessLastEx(&ChildContext, pContext, NULL);
  }
  _closesocket(hSock);
  OS_EnterRegion();
  _AddToConnectCnt(-1);
  OS_Terminate(0);
  OS_LeaveRegion();
}

/*********************************************************************
*
*       _WebServerParentTask
*
*/
static void _WebServerParentTask(void) {
  struct sockaddr    Addr;
  struct sockaddr_in InAddr;
  U32  Timeout;
  long hSockListen;
  long hSock;
  int  AddrLen;
  int  i;
  int  t;
  int  t0;
  int  r;
  WEBS_BUFFER_SIZES BufferSizes;

  Timeout = IDLE_TIMEOUT;
  IP_WEBS_SetFileInfoCallback(&_pfGetFileInfo);
  //
  // Assign file system
  //
  _pFS_API = &IP_FS_ReadOnly;  // To use a a real filesystem like emFile replace this line.
//  _pFS_API = &IP_FS_FS;        // Use emFile
//  IP_WEBS_AddUpload();         // Enable upload
  //
  // Configure buffer size.
  //
  IP_MEMSET(&BufferSizes, 0, sizeof(BufferSizes));
  BufferSizes.NumBytesInBuf       = WEBS_IN_BUFFER_SIZE;
  BufferSizes.NumBytesOutBuf      = IP_TCP_GetMTU(_IFaceId) - 72;  // Use max. MTU configured for the last interface added minus worst case IPv4/TCP/VLAN headers.
                                                                   // Calculation for the memory pool is done under assumption of the best case headers with - 40 bytes.
  BufferSizes.NumBytesParaBuf     = WEBS_PARA_BUFFER_SIZE;
  BufferSizes.NumBytesFilenameBuf = WEBS_FILENAME_BUFFER_SIZE;
  BufferSizes.MaxRootPathLen      = WEBS_MAX_ROOT_PATH_LEN;
  //
  // Configure the size of the buffers used by the Webserver child tasks.
  //
  IP_WEBS_ConfigBufSizes(&BufferSizes);
  //
  // Give the stack some more memory to enable the dynamical memory allocation for the Web server child tasks
  //
  IP_AddMemory(_aPool, sizeof(_aPool));
  //
  // Get a socket into listening state
  //
  hSockListen = socket(AF_INET, SOCK_STREAM, 0);
  if (hSockListen == SOCKET_ERROR) {
    while(1); // This should never happen!
  }
  IP_MEMSET(&InAddr, 0, sizeof(InAddr));
  InAddr.sin_family      = AF_INET;
  InAddr.sin_port        = htons(SERVER_PORT);
  InAddr.sin_addr.s_addr = INADDR_ANY;
  bind(hSockListen, (struct sockaddr *)&InAddr, sizeof(InAddr));
  listen(hSockListen, BACK_LOG);
  //
  // Loop once per client and create a thread for the actual server
  //
  do {
Next:
    //
    // Wait for an incoming connection
    //
    hSock = 0;
    AddrLen = sizeof(Addr);
    if ((hSock = accept(hSockListen, &Addr, &AddrLen)) == SOCKET_ERROR) {
      continue;    // Error
    }
    //
    // Create server thread to handle connection.
    // If connection limit is reached, keep trying for some time before giving up and outputting an error message
    //
    t0 = OS_GetTime32() + 1000;
    do {
      if (_ConnectCnt < MAX_CONNECTIONS) {
        for (i = 0; i < MAX_CONNECTIONS; i++) {
          r = OS_IsTask(&_aWebTasks[i]);
          if (r == 0) {
            setsockopt(hSock, SOL_SOCKET, SO_RCVTIMEO, &Timeout, sizeof(Timeout));  // Set receive timeout for the client.
            OS_CREATETASK_EX(&_aWebTasks[i], "Webserver Child", _WebServerChildTask, TASK_PRIO_WEBS_CHILD, _aWebStacks[i], (void *)hSock);
            _AddToConnectCnt(1);
            goto Next;
          }
        }
      }
      //
      // Check time out
      //
      t = OS_GetTime32();
      if ((t - t0) > 0) {
        IP_WEBS_OnConnectionLimit(_Send, _Recv, (void*)hSock);
        _closesocket(hSock);
        break;
      }
      OS_Delay(10);
    } while(1);
  }  while(1);
}

/*********************************************************************
*
*       _OnRx
*
*  Function descrition
*    Discover client UDP callback. Called from stack
*    whenever we get a discover request.
*
*  Return value
*    IP_RX_ERROR  if packet is invalid for some reason
*    IP_OK        if packet is valid
*/
#if ETH_UDP_DISCOVER_PORT
static int _OnRx(IP_PACKET *pInPacket, void *pContext) {
  char *      pInData;
  IP_PACKET * pOutPacket;
  char *      pOutData;
  U32         TargetAddr;
  U32         IPAddr;
  unsigned    IFaceId;

  (void)pContext;

  IFaceId = IP_UDP_GetIFIndex(pInPacket);  // Find out the interface that the packet came in.
  IPAddr  = htonl(IP_GetIPAddr(IFaceId));
  if (IPAddr == 0) {
    goto Done;
  }
  pInData = (char*)IP_UDP_GetDataPtr(pInPacket);
  if (memcmp(pInData, "Discover", 8)) {
    goto Done;
  }
  //
  // Alloc packet
  //
  pOutPacket = IP_UDP_AllocEx(IFaceId, PACKET_SIZE);
  if (pOutPacket == NULL) {
    goto Done;
  }
  //
  // Fill packet with data
  //
  pOutData = (char*)IP_UDP_GetDataPtr(pOutPacket);
  IP_UDP_GetSrcAddr(pInPacket, &TargetAddr, sizeof(TargetAddr));    // Send back Unicast
  memset(pOutData, 0, PACKET_SIZE);
  strcpy(pOutData + 0x00, "Found");
  IPAddr = htonl(IP_GetIPAddr(IFaceId));
  memcpy(pOutData + 0x20, (void*)&IPAddr, 4);       // 0x20: IP address
  IP_GetHWAddr(IFaceId, (U8*)pOutData + 0x30, 6);  // 0x30: MAC address
  //
  // Send packet
  //
  IP_UDP_SendAndFree(IFaceId, TargetAddr, ETH_UDP_DISCOVER_PORT, ETH_UDP_DISCOVER_PORT, pOutPacket);
Done:
  return IP_OK;
}
#endif

/*********************************************************************
*
*       _REST_cb
*
*  Function descrition
*    Callback for demonstrational REST implementation using a METHOD
*    hook. As there is no clearly defined standard for REST this
*    implementation shall act as sample and starting point on how
*    REST support could be implemented by you.
*
*  Parameters
*    pContext      - Context for incoming data
*    pOutput       - Context for outgoing data
*    sMethod       - String containing used METHOD
*    sAccept       - NULL or string containing value of "Accept" field of HTTP header
*    sContentType  - NULL or string containing value of "Content-Type" field of HTTP header
*    sResource     - String containing URI that was accessed
*    ContentLen    - 0 or length of data submitted by client
*
*  Return value
*    0             - O.K.
*    Other         - Error
*/
static int _REST_cb(void *pContext, WEBS_OUTPUT *pOutput, const char *sMethod, const char *sAccept, const char *sContentType, const char *sResource, U32 ContentLen) {
  int  Len;
  char acAccept[128];
  char acContentType[32];

  //
  // Strings located at sAccept and sContentType need to be copied to
  // another location before calling any other Web Server API as they
  // will be overwritten.
  //
  if (sAccept) {
    _CopyString(acAccept, sAccept, sizeof(acAccept));
  }
  if (sContentType) {
    _CopyString(acContentType, sContentType, sizeof(acContentType));
  }
  //
  // Send implementation specific header to client
  //
  IP_WEBS_SendHeader(pOutput, NULL, "application/REST");
  //
  // Output information about the METHOD used by the client
  //
  IP_WEBS_SendString(pOutput, "METHOD:       ");
  IP_WEBS_SendString(pOutput, sMethod);
  IP_WEBS_SendString(pOutput, "\n");
  //
  // Output information about which URI has been accessed by the client
  //
  IP_WEBS_SendString(pOutput, "URI:          ");
  IP_WEBS_SendString(pOutput, sResource);
  IP_WEBS_SendString(pOutput, "\n");
  //
  // Output information about "Accept" field given in header sent by client, if any
  //
  if (sAccept) {
    IP_WEBS_SendString(pOutput, "Accept:       ");
    IP_WEBS_SendString(pOutput, acAccept);
    IP_WEBS_SendString(pOutput, "\n");
  }
  //
  // Output information about "Content-Type" field given in header sent by client, if any
  //
  if (sContentType) {
    IP_WEBS_SendString(pOutput, "Content-Type: ");
    IP_WEBS_SendString(pOutput, acContentType);
  }
  //
  // Output content sent by client, or content previously sent by client that has been saved
  //
  if ((_acRestContent[0] || ContentLen) && sContentType) {
    IP_WEBS_SendString(pOutput, "\n");
  }
  if (_acRestContent[0] || ContentLen) {
    IP_WEBS_SendString(pOutput, "Content:\n");
  }
  if (ContentLen) {
    //
    // Update saved content
    //
    Len = SEGGER_MIN(sizeof(_acRestContent) - 1, ContentLen);
    IP_WEBS_METHOD_CopyData(pContext, _acRestContent, Len);
    _acRestContent[ContentLen] = 0;
  }
  if (_acRestContent[0]) {
    IP_WEBS_SendString(pOutput, _acRestContent);
  }
  return 0;
}

/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  IP_Init();
  _IFaceId = IP_INFO_GetNumInterfaces() - 1;                                           // Get the last registered interface ID as this is most likely the interface we want to use in this sample.
  //
  // Start TCP/IP task
  //
  OS_CREATETASK(&_IPTCB,   "IP_Task",   IP_Task,   TASK_PRIO_IP_TASK,    _IPStack);
#if USE_RX_TASK
  OS_CREATETASK(&_IPRxTCB, "IP_RxTask", IP_RxTask, TASK_PRIO_IP_RX_TASK, _IPRxStack);  // Start the IP_RxTask, optional.
#endif
  IP_AddStateChangeHook(&_StateChangeHook, _OnStateChange);                            // Register hook to be notified on disconnects.
  IP_Connect(_IFaceId);                                                                // Connect the interface if necessary.
  //
  // IPv4 address configured ?
  //
  while (IP_IFaceIsReadyEx(_IFaceId) == 0) {
    BSP_ToggleLED(0);
    OS_Delay(200);
  }
  BSP_ClrLED(0);
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_WEBS_PARENT);                                  // This task has highest prio!
  OS_SetTaskName(OS_GetTaskID(), "IP_WebServer");
#if ETH_UDP_DISCOVER_PORT
  //
  // Open UDP port ETH_UDP_DISCOVER_PORT for embOS/IP discover
  //
  IP_UDP_Open(0L /* any foreign host */,  ETH_UDP_DISCOVER_PORT, ETH_UDP_DISCOVER_PORT,  _OnRx, 0L);
#endif
  IP_WEBS_METHOD_AddHook(&_MethodHook, _REST_cb, "/REST");  // Register URI "http://<ip>/REST" for demonstrational REST implementation
  _WebServerParentTask();
}

/*************************** End of file ****************************/
