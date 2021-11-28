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
File    : ModbusTCP_Master.c
Purpose : Sample program for Modbus master using Windows.
          Provides utility functions to make the TCP/IP interface for
          the Modbus Master main when using the Modbus/TCP protocol.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "MB.h"
#include "Modbus_Master_IFace.h"

/*********************************************************************
*
*       Types/structures
*
**********************************************************************
*/

typedef void *SYS_HANDLE;

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
  WORD wVersionRequested;
  WSADATA wsaData;

  //
  // Initialize Winsock.
  //
  wVersionRequested = MAKEWORD(2, 0);
  if (WSAStartup(wVersionRequested, &wsaData) != 0) {
    while (1);  // Should never happen.
  }
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
  MB_USE_PARA(pConfig);

  WSACleanup();
}

/*********************************************************************
*
*       _Connect()
*
* Function description
*   Create socket, connect to slave and store socket in configuration.
*
* Parameters
*   pConfig: Pointer to configuration.
*   Timeout: Timeout [ms] for the connect operation to return.
*
* Return value
*   0    : Connection to slave established.
*   Other: Error (or no new connection in case of non-blocking).
*/
static int _Connect(MB_IFACE_CONFIG_IP *pConfig, U32 Timeout) {
  struct sockaddr_in Addr;
  struct fd_set      SelectSet;
  struct timeval     Time;
         int         hSock;
         int         r;
         int         t0;
         int         v;

  //
  // Get socket for connecting.
  //
  hSock = socket(AF_INET, SOCK_STREAM, 0);
  if (hSock == SOCKET_ERROR) {
    while(1);  // This should never happen!
  }
  v = 1;
  ioctlsocket(hSock,FIONBIO,&v);                 // Set socket non blocking.
  //
  // Configure socket and connect.
  //
  Addr.sin_family      = AF_INET;
  Addr.sin_port        = htons(pConfig->Port);
  Addr.sin_addr.s_addr = htonl(pConfig->IPAddr);
  t0                   = MB_OS_GET_TIME();
  FD_ZERO(&SelectSet);
  FD_SET(hSock, &SelectSet);
  Time.tv_sec  = Timeout / 1000;
  Time.tv_usec = Timeout % 1000;
  connect(hSock, (struct sockaddr*)&Addr, sizeof(struct sockaddr_in));
  r = select(0, NULL, &SelectSet, NULL, &Time);
  if (r == 0) {
    return MB_ERR_CONNECT_TIMEOUT;               // Unable to connect to slave within timeout.
  }
  if (r == 1) {
    v = 0;
    ioctlsocket(hSock,FIONBIO,&v);               // Set socket blocking.
    pConfig->Sock = (MB_SOCKET)hSock;            // Store socket into configuration.
    return 0;                                    // Connected to slave.
  }
  return MB_ERR_CONNECT;                         // Unable to connect to slave.
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
  int SoError;
  int hSock;
  int r;

  hSock = (int)pConfig->Sock;
  //
  // Set current timeout. Might be less than the total timeout configured for the
  // channel if less than the requested number of bytes has been received before.
  //
  setsockopt(hSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&Timeout, sizeof(Timeout));
  NumBytesReceived = recv(hSock, (char*)pData, NumBytes, 0);
  if (NumBytesReceived > 0) {
    r = NumBytesReceived;
  } else {
    if (NumBytesReceived == 0) {  // Connection closed ?
      r = MB_ERR_DISCONNECT;
    } else {
      SoError = WSAGetLastError();
      if (SoError == WSAETIMEDOUT) {
        r = MB_ERR_TIMEOUT;
      } else {
        r = MB_ERR_MISC;
      }
    }
  }
  return r;
}

/****** End Of File *************************************************/
