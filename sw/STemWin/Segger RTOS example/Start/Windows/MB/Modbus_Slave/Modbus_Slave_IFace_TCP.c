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
File    : Modbus_Slave_IFace_TCP.c
Purpose : Sample program for Modbus slave using Windows.
          Provides utility functions to make the TCP/IP interface for
          the Modbus Slave main when using the Modbus/TCP protocol.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "MB.h"
#include "Modbus_Slave_IFace.h"

/*********************************************************************
*
*       Types/structures
*
**********************************************************************
*/

typedef void *SYS_HANDLE;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static SYS_HANDLE _PollTaskHandle;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

static int  _Init      (MB_IFACE_CONFIG_IP *pConfig);
static void _DeInit    (MB_IFACE_CONFIG_IP *pConfig);
static int  _Send      (MB_IFACE_CONFIG_IP *pConfig, const U8 *pData, U32 NumBytes);
static int  _Recv      (MB_IFACE_CONFIG_IP *pConfig,       U8 *pData, U32 NumBytes, U32 Timeout);
static int  _Connect   (MB_IFACE_CONFIG_IP *pConfig, U32 Timeout);
static void _Disconnect(MB_IFACE_CONFIG_IP *pConfig);

static UINT __stdcall _PollSlaveChannelTask(void *pContext);

/*********************************************************************
*
*       Static const, Modbus API
*
**********************************************************************
*/

const MB_IFACE_IP_API _IFaceTCP = {
  NULL,         // pfSendByte
  _Init,        // pfInit
  _DeInit,      // pfDeInit
  _Send,        // pfSend
  _Recv,        // pfRecv
  _Connect,     // pfConnect
  _Disconnect,  // pfDisconnect
  NULL,         // pfInitTimer
  NULL          // pfDeInitTimer
};

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       Local functions, Modbus network interface
*
**********************************************************************
*/

/*********************************************************************
*
*       _Init()
*
* Function description
*   Gets listen socket and sets it into listen state.
*
* Parameters
*   pConfig: Pointer to configuration.
*
* Return value
*   O.K. : 0
*   Error: Other
*/
static int _Init(MB_IFACE_CONFIG_IP *pConfig) {
         WORD        wVersionRequested;
         WSADATA     wsaData;
  struct sockaddr_in Addr;
         int         hSock;
         int         r;
         int         ThreadId;

  //
  // Initialize Winsock.
  //
  wVersionRequested = MAKEWORD(2, 0);
  if (WSAStartup(wVersionRequested, &wsaData) != 0) {
    return MB_ERR_MISC;
  }
  //
  // Get listen socket.
  //
  hSock = socket(AF_INET, SOCK_STREAM, 0);
  if (hSock == SOCKET_ERROR) {
    return MB_ERR_MISC;
  }
  Addr.sin_family      = AF_INET;
  Addr.sin_port        = htons(pConfig->Port);
  Addr.sin_addr.s_addr = htonl(pConfig->IPAddr);
  r = bind(hSock, (struct sockaddr*)&Addr, sizeof(Addr));
  if (r) {
    return MB_ERR_MISC;
  }
  r = listen(hSock, 1);
  if (r) {
    return MB_ERR_MISC;
  }
  //
  // Store socket into configuration.
  //
  pConfig->ListenSock = (MB_SOCKET)hSock;
  //
  // Create the polling thread
  //
  _PollTaskHandle  = (SYS_HANDLE)CreateThread(NULL, 0, _PollSlaveChannelTask, NULL, 0, &ThreadId); // Start channel polling task for the slave channel.
  //
  return 0;
}

/*********************************************************************
*
*       _DeInit()
*
* Function description
*   Close listen socket.
*
* Parameters
*   pConfig: Pointer to configuration.
*/
static void _DeInit(MB_IFACE_CONFIG_IP *pConfig) {
  if (_PollTaskHandle) {
    TerminateThread(_PollTaskHandle, 0);
  }
  closesocket((int)pConfig->ListenSock);
  WSACleanup();
}

/*********************************************************************
*
*       _Connect()
*
* Function description
*   Accept a new connection.
*
* Parameters
*   pConfig: Pointer to configuration.
*   Timeout: Not used for slave.
*
* Return value
*   0    : New connection accepted.
*   Other: Error (or no new connection in case of non-blocking).
*/
static int _Connect(MB_IFACE_CONFIG_IP *pConfig, U32 Timeout) {
  struct sockaddr Addr;
         int      AddrSize;
         int      hSock;

  MB_USE_PARA(Timeout);

  MB_OS_DISABLE_INTERRUPT();
  if (_ConnectCnt > 0) {
    _ConnectCnt--;
  }
  MB_OS_ENABLE_INTERRUPT();
  AddrSize = sizeof(Addr);
  hSock    = accept((int)pConfig->ListenSock, &Addr, &AddrSize);
  if (hSock != SOCKET_ERROR) {
    MB_OS_DISABLE_INTERRUPT();
    _ConnectCnt++;
    MB_OS_ENABLE_INTERRUPT();
    pConfig->Sock = (MB_SOCKET)hSock;  // Store socket into configuration.

    return 0;                          // O.K., new connection accepted.
  }
  return MB_ERR_CONNECT;               // Error (or no new connection in case of non-blocking).
}

/*********************************************************************
*
*       _Disconnect()
*
* Function description
*   Close connection.
*
* Parameters
*   pConfig: Pointer to configuration.
*/
static void _Disconnect(MB_IFACE_CONFIG_IP *pConfig) {
  closesocket((int)pConfig->Sock);
}

/*********************************************************************
*
*       _Send()
*
* Function description
*   Sends data on the interface.
*
* Parameters
*   pConfig : Pointer to configuration.
*   pData   : Pointer to data to send.
*   NumBytes: Number of bytes to send from pData.
*
* Return value
*   >= 0: NumBytes sent.
*   <  0: Error.
*/
static int _Send(MB_IFACE_CONFIG_IP *pConfig, const U8 *pData, U32 NumBytes) {
  int NumBytesSent;
  int r;

  NumBytesSent = send((int)pConfig->Sock, (const char*)pData, NumBytes, 0);
  if (NumBytesSent > 0) {
    r = NumBytesSent;
  } else {
    if (NumBytesSent == 0) {  // Connection closed ?
      r = MB_ERR_DISCONNECT;
    } else {
      r = MB_ERR_MISC;
    }
  }
  return r;
}

/*********************************************************************
*
*       _Recv()
*
* Function description
*   Receives data from the interface.
*
* Parameters
*   pConfig : Pointer to configuration.
*   Timeout : Timeout [ms] for the receive operation to return. Always 0 if slave channel.
*   pData   : Pointer to buffer to store data received.
*   NumBytes: Number of bytes received.
*
* Return value
*   >= 0: Number of bytes read.
*   <  0: Error.
*/
static int _Recv(MB_IFACE_CONFIG_IP *pConfig, U8 *pData, U32 NumBytes, U32 Timeout) {
  int NumBytesReceived;
  int r;

  MB_USE_PARA(Timeout);  // Not used for slave in task mode.

  NumBytesReceived = recv((int)pConfig->Sock, (char*)pData, NumBytes, 0);
  if (NumBytesReceived > 0) {
    r = NumBytesReceived;
  } else {
    if (NumBytesReceived == 0) {  // Connection closed ?
      r = MB_ERR_DISCONNECT;
    } else {
      r = MB_ERR_MISC;
    }
  }
  return r;
}

/*********************************************************************
*
*       Local functions, Modbus slave channel polling tasks
*
* All slave channels that are not able to pass their received data to
* the Modbus stack from an interrupt directly need to be polled.
*
**********************************************************************
*/

/*********************************************************************
*
*       _PollSlaveChannelTask()
*
* Function description
*   Polls a slave channel in a task, allowing the task to sleep when
*   nothing to do.
*/
static UINT __stdcall _PollSlaveChannelTask(void *pContext) {
  while (1) {
    MB_SLAVE_PollChannel(&_MBChannel);
  }
}

/****** End Of File *************************************************/
