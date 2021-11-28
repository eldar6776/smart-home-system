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
File    : OS_IP_FTPClient.c
Purpose : Sample program for embOS & TCP/IP
          Demonstrates use of the FTP client.
--------- END-OF-HEADER --------------------------------------------*/

#include "RTOS.h"
#include "FS.h"
#include "BSP.h"
#include "IP.h"
#include "IP_FTPC.h"

/*********************************************************************
*
*       Local defines, configurable
*
**********************************************************************
*/

#define USE_RX_TASK       0                             // 0: Packets are read in ISR, 1: Packets are read in a task of its own.

#define FTP_HOST  "192.168.88.1"
#define FTP_PORT  21
#define FTP_USER  "Admin"
#define FTP_PASS  "Secret"

/*********************************************************************
*
*       static data
*
**********************************************************************
*/

static IP_HOOK_ON_STATE_CHANGE _StateChangeHook;
static int                     _IFaceId;

//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int _IPStack[TASK_STACK_SIZE_IP_TASK/sizeof(int)];       // Stack of the IP_Task.
static OS_TASK         _IPTCB;                                              // Task-Control-Block of the IP_Task.

#if USE_RX_TASK
static OS_STACKPTR int _IPRxStack[TASK_STACK_SIZE_IP_RX_TASK/sizeof(int)];  // Stack of the IP_RxTask.
static OS_TASK         _IPRxTCB;                                            // Task-Control-Block of the IP_RxTask.
#endif

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
*       static code
*
**********************************************************************
*/

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
*       _atoi
*
*  Function description
*    Converts a string to an interger
*/
static int _atoi(const char  * sValue) {
  int Value;
  int Digit;
  char c;

  Value = 0;
  while ((c = *sValue++) != '\0') {
    if ((c >= '0') && (c <= '9')) {
      Digit = (int) (c - '0');
    } else {
      break;
    }
    Value = (Value * 10) + Digit;
  }
  return Value;
}

/*********************************************************************
*
*       _IsIPAddress()
*
*  Function description
*    Checks if string is a dot-decimal IP address, for example 192.168.1.1
*/
static unsigned _IsIPAddress(const char * sIPAddr) {
  unsigned NumDots;
  unsigned i;
  char c;

  NumDots = 0;
  i       = 0;
  while (1) {
    c = *(sIPAddr + i);
    if ((c >= '0') && (c <= '9')) {
      goto Loop;
    }
    if (c == '.') {
      NumDots++;
      goto Loop;
    }
    if (c == '\0') {
      if ((NumDots < 3) || (i > 15)) { // Error, every dot-decimal IP address includes 3 '.' and is not longer as 15 characters.
Error:
        return 0;
      }
      return 1;
    } else {
      goto Error;
    }
Loop:
    i++;
  }
}

/*********************************************************************
*
*       _ParseIPAddr()
*
*  Function description
*    Parses a string for a dot-decimal IP address and returns the
*    IP as 32-bit number in host endianess.
*/
static long _ParseIPAddr(const char * sIPAddr) {
  long     IPAddr;
  unsigned Value;
  unsigned NumDots;
  unsigned i;
  unsigned j;
  char     acDigits[4];
  char     c;

  IPAddr = 0;
  //
  // Check if string is a valid IP address
  //
  Value = _IsIPAddress(sIPAddr);
  if (Value) {
    //
    // Parse the IP address
    //
    NumDots = 3;
    i       = 0;
    j       = 0;
    while (1) {
      c = *(sIPAddr + i);
      if (c == '\0') {
        //
        // Add the last byte of the IP address.
        //
        acDigits[j] = '\0';
        Value = _atoi(acDigits);
        if (Value < 255) {
          IPAddr |= Value;
        }
        return IPAddr; // O.K., string completely parsed. Returning IP address.
      }
      //
      // Parse the first three byte of the IP address.
      //
      if (c != '.') {
        acDigits[j] = c;
        j++;
      } else {
        acDigits[j] = '\0';
        Value = _atoi(acDigits);
        if (Value < 255) {
          IPAddr |= (Value << (NumDots * 8));
          NumDots--;
          j = 0;
        } else {
          return -1;  // Error, illegal number in IP address.
        }
      }
      i++;
    }
  }
  return -1;
}

/*********************************************************************
*
*       _Connect
*
*  Function description
*    Creates a socket and opens a TCP connection to the FTP host.
*/
static FTPC_SOCKET _Connect(const char * sSrvAddr, unsigned SrvPort) {
  long Ip;
  long Sock;
  struct hostent * pHostEntry;
  struct sockaddr_in sin;
  int r;

  if (_IsIPAddress(sSrvAddr)) {
    Ip = _ParseIPAddr(sSrvAddr);
    Ip = htonl(Ip);
  } else {
    //
    // Convert host into IP address
    //
    pHostEntry = gethostbyname((char*)sSrvAddr);
    if (pHostEntry == NULL) {
      FTPC_LOG(("APP: gethostbyname failed: %s\r\n", sSrvAddr));
      return NULL;
    }
    Ip = *(unsigned*)(*pHostEntry->h_addr_list);
  }
  //
  // Create socket and connect to the FTP server
  //
  Sock = socket(AF_INET, SOCK_STREAM, 0);
  if(Sock  == -1) {
    FTPC_LOG(("APP: Could not create socket!" ));
    return NULL;
  }
  IP_MEMSET(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(SrvPort);
  sin.sin_addr.s_addr = Ip;
  r = connect(Sock, (struct sockaddr*)&sin, sizeof(sin));
  if(r == SOCKET_ERROR) {
    closesocket(Sock);
    FTPC_LOG(("APP: \nSocket error :"));
    return NULL;
  }
  FTPC_LOG(("APP: Connected to %i, port %d.", Ip, SrvPort));
  return (FTPC_SOCKET)Sock;
}

/*********************************************************************
*
*       _Disconnect
*
*  Function description
*    Closes a socket.
*/
static void _Disconnect(FTPC_SOCKET Socket) {
  if (Socket) {
    closesocket((long)Socket);
  }
}

/*********************************************************************
*
*       _Send
*
*  Function description
*    Sends data via socket interface.
*/
static int _Send(const char * buf, int len, void * pConnectionInfo) {
  return send((long)pConnectionInfo, buf, len, 0);
}

/*********************************************************************
*
*       _Recv
*
*  Function description
*    Receives data via socket interface.
*/
static int _Recv(char * buf, int len, void * pConnectionInfo) {
  return recv((long)pConnectionInfo, buf, len, 0);
}

static const IP_FTPC_API _IP_Api = {
  _Connect,
  _Disconnect,
  _Send,
  _Recv
};

/*********************************************************************
*
*       _FSTest
*
*  Function description
*    Initializes the file system and creates a test file on storage medium
*/
static void _FSTest(void) {
  FS_FILE    * pFile;
  unsigned     Len;
  const char * sInfo = "SEGGER embOS/IP FTP client.\r\nFor further information please visit: www.segger.com\r\n";

  FS_Init();
  Len = strlen(sInfo);
  if (FS_IsLLFormatted("") == 0) {
    FS_X_Log("Low level formatting");
    FS_FormatLow("");          // Erase & Low-level format the volume
  }
  if (FS_IsHLFormatted("") == 0) {
    FS_X_Log("High level formatting\n");
    FS_Format("", NULL);       // High-level format the volume
  }
  pFile = FS_FOpen("Readme.txt", "w");
  FS_Write(pFile, sInfo, Len);
  FS_FClose(pFile);
  FS_Unmount("");
}

/*********************************************************************
*
*       MainTask
*
*  Note:
*   The size of the stack of this task should be at least
*   1200 bytes + FTPC_CTRL_BUFFER_SIZE + 2 * FTPC_BUFFER_SIZE.
*/
void MainTask(void) {
  IP_FTPC_CONTEXT FTPConnection;
  U8 acCtrlIn[FTPC_CTRL_BUFFER_SIZE];
  U8 acDataIn[FTPC_BUFFER_SIZE];
  U8 acDataOut[FTPC_BUFFER_SIZE];
  int r;

  //
  // Create a test file on storage medium
  //
  _FSTest();
  //
  // Initialize the IP stack
  //
  IP_Init();
  _IFaceId = IP_INFO_GetNumInterfaces() - 1;                             // Get the last registered interface ID as this is most likely the interface we want to use in this sample.
  OS_CREATETASK(&_IPTCB, "IP_Task", IP_Task  , 150, _IPStack);           // Start the IP_Task
#if USE_RX_TASK
  OS_CREATETASK(&_IPRxTCB, "IP_RxTask", IP_RxTask, 160, _IPRxStack);     // Start the IP_RxTask, optional.
#endif
  IP_AddStateChangeHook(&_StateChangeHook, _OnStateChange);              // Register hook to be notified on disconnects.
  IP_Connect(_IFaceId);                                                  // Connect the interface if necessary.
  //
  // Check if target is configured
  //
  while (IP_IFaceIsReadyEx(_IFaceId) == 0) {
    BSP_ToggleLED(1);
    OS_Delay(50);
  }
  //
  // FTP client task
  //
  while (1) {
    BSP_SetLED(0);
    //
    // Initialize FTP client context
    //
    memset(&FTPConnection, 0, sizeof(FTPConnection));
    //
    // Initialize the FTP client
    //
    IP_FTPC_Init(&FTPConnection, &_IP_Api, &IP_FS_FS, acCtrlIn, sizeof(acCtrlIn), acDataIn, sizeof(acDataIn), acDataOut, sizeof(acDataOut));
    //
    // Connect to the FTP server
    //
    r = IP_FTPC_Connect(&FTPConnection, FTP_HOST, FTP_USER, FTP_PASS, FTP_PORT, FTPC_MODE_PASSIVE);
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Could not connect to FTP server.\r\n"));
      goto Disconnect;
    }
    //
    // Create the directory "Test"
    //
    r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_MKD, "Test");
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Could not change working directory.\r\n"));
      goto Disconnect;
    }
    //
    // Change from root directory into directory "Test"
    //
    r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_CWD, "/Test/");
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Could not change working directory.\r\n"));
      goto Disconnect;
    }
    //
    // Upload the file "Readme.txt"
    //
    r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_STOR, "Readme.txt");
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Could not upload data file.\r\n"));
      goto Disconnect;
    }
    //
    // List directory content
    //
    r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_LIST, NULL);
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Could not list directory.\r\n"));
      goto Disconnect;
    }
    IP_Logf_Application("%s", acDataIn);
    //
    // Delete the file "Readme.txt"
    //
    r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_DELE, "Readme.txt");
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Could not upload data file.\r\n"));
      goto Disconnect;
    }
    //
    // List directory content
    //
    r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_LIST, NULL);
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Could not list directory.\r\n"));
      goto Disconnect;
    }
    IP_Logf_Application("%s", acDataIn);
    //
    // Change back to root directory.
    //
    r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_CDUP, NULL);
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Change to parent directory failed.\r\n"));
      goto Disconnect;
    }
    //
    // Delete the directory "Test"
    //
    r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_RMD, "Test");
    if (r == FTPC_ERROR) {
      FTPC_LOG(("APP: Could not change working directory.\r\n"));
      goto Disconnect;
    }
    //
    // Disconnect.
    //
Disconnect:
    IP_FTPC_Disconnect(&FTPConnection);
    FTPC_LOG(("APP: Done.\r\n"));
    BSP_ClrLED(0);
    OS_Delay (10000);
  }
}

/*************************** End of file ****************************/
