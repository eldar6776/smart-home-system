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
File    : SpeedTestServer.c
Purpose : TCP/IP speed and stability test sample server.
          Start the server on host and run OS_IP_SpeedClient.c on 
          target hardware.
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <conio.h>
#include <windows.h>
#else
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define VERSION          10000
#define PORT             1234
#define DATA_TO_SEND     (0x00)
#define TRANSMISSION_ACK 'X'
#define TIMEOUT          5  // In seconds.

#ifdef WIN32
#define TIMETYPE         LONGLONG
#define SOCKLEN          int
#define CLOSESOCKET(s)   closesocket(s)
#else
#define TIMETYPE         uint64_t
#define SOCKLEN          socklen_t
#define CLOSESOCKET(s)   close(s)
#endif

/*********************************************************************
*
*       Static Variables
*
**********************************************************************
*/

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _ListenAtTcpAddr
*
* Starts listening at the given TCP port.
*/
static int _ListenAtTcpAddr(unsigned short port) {
  int                r;
  int                sock;
  struct sockaddr_in addr;
  int                one;

  one = 1; 
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return -1;
  }
  r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &one, sizeof(one));
  if (r < 0) {
    return -1;
  }
  r = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  if (r < 0) {
    return -1;
  }
  r = listen(sock, 5);
  if (r < 0) {
    return -1;
  }
  return sock;
}

/*********************************************************************
*
*       _GetTime64()
*
*  Function description
*    Retrieves high resolution time value as 64bit integer
*/
static TIMETYPE _GetTime64(void) {

#ifdef WIN32
  FILETIME       FileTime;
  LARGE_INTEGER  Time;

  GetSystemTimeAsFileTime(&FileTime);
  memcpy(&Time, &FileTime, sizeof(Time));
  return Time.QuadPart;
#else
  TIMETYPE          Time;
  struct timespec   spec;

  clock_gettime(CLOCK_REALTIME, &spec);
  Time = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
  return Time; // In ms
#endif
}

/*********************************************************************
*
*       _CalculateTransSpeed
*
*  Function description
*    Send the requested number of bytes
*/
static TIMETYPE _CalculateTransSpeed(TIMETYPE StartTime, TIMETYPE EndTime, unsigned int NumBytes) {
  TIMETYPE tDiff;
  TIMETYPE BytesPerSecond;

  tDiff = EndTime - StartTime;
  if (tDiff) {
#ifndef WIN32
    tDiff *= 10000;  // On Linux, ms are used. Convert it to the windows range.
#endif
    printf("Total Time: %ld ms\n", tDiff/10000);
    BytesPerSecond  =  NumBytes;
    BytesPerSecond *=  10000000;
    BytesPerSecond /= tDiff;
    return BytesPerSecond;
  }
  return 0;
}

/*********************************************************************
*
*       _SendData
*
*  Function description
*    Send the requested number of bytes
*/
static int _SendData(unsigned int sock) {
  char         ac[10];
  char     *   pBulk;
  int          TxDataLen;
  unsigned int NumBytesTotal;
  unsigned int NumBytesAtOnce;
  unsigned int SendCnt;
  unsigned int SendLen;
  TIMETYPE     r;
  TIMETYPE     tStart;
  TIMETYPE     tEnd;
  int          Mtu;

  r = recv(sock, (void *)&NumBytesTotal, 4, 0);     // Waiting for number of bytes that should be sent
  if (r < 0) {
    return -1;
  }
  r = recv(sock, (void *)&Mtu, 4, 0);               // Receive MTU of the client (unused in this case)
  if (r < 0) {
    return -1;
  }
  NumBytesAtOnce = 3 * Mtu;
  pBulk = malloc(NumBytesAtOnce);
  if (pBulk == NULL) {
    printf("Could not get memory for %d byte(s)\n", NumBytesAtOnce);
    printf("Client-Handler aborted...\n");
    return -1;
  } else {
    memset(pBulk, DATA_TO_SEND, NumBytesAtOnce);
  }
  printf("Server starts sending %8d bytes.\n", NumBytesTotal);
  SendCnt = 0;
  //
  //  Send bulk data
  //
  tStart = _GetTime64();
  do {
    if ((NumBytesTotal - SendCnt) < NumBytesAtOnce) {
      SendLen = NumBytesTotal - SendCnt;
    } else {
      SendLen = NumBytesAtOnce;
    }
    TxDataLen = send(sock, pBulk, SendLen, 0);
    if (TxDataLen <= 0) {
      return TxDataLen;
    } else {
      SendCnt += TxDataLen;
      printf(".");
    }
  } while (SendCnt < NumBytesTotal);
  r = recv(sock, ac, 1, 0);                    // Waiting for acknowledgement
  if (ac[0] == 'X') {
    printf("\nServer sent %8d bytes.\n", SendCnt);
    tEnd  = _GetTime64();
    r = _CalculateTransSpeed(tStart, tEnd, SendCnt);
    printf("Bytes per second:      %8ld \n", r);
    printf("*******************************\n");
  }
  free(pBulk);
  return 0;
}

/*********************************************************************
*
*       _ReceiveData
*
*  Function description
*    Receive a number of bytes
*/
static int _ReceiveData(unsigned int sock) {
  char         Command;
  unsigned int NumBytes;
  unsigned int ReceiveCnt;
  unsigned int aRxBuffer[1024];
  int          RxDataLen;
  int          r;
  TIMETYPE     tResult;
  TIMETYPE     tStart;
  TIMETYPE     tEnd;
  int          Mtu;

  ReceiveCnt = 0;
  r = recv(sock, (void *)&NumBytes, 4, 0);      // Receive number of bytes that the client would sent
  if (r < 0) {
    return -1;
  }
  r = recv(sock, (void *)&Mtu, 4, 0);           // Receive MTU of the client (unused in this case)
  if (r < 0) {
    return -1;
  }
  printf("Client starts sending %8d bytes.\n", NumBytes);
  tStart = _GetTime64();
  do {
    RxDataLen   = recv(sock, (void *)&aRxBuffer[0], sizeof(aRxBuffer), 0);
    if (RxDataLen <= 0) {
      return RxDataLen;
    } else {
      ReceiveCnt += RxDataLen;
      printf(".");
    }
  } while (ReceiveCnt < NumBytes);
  Command = TRANSMISSION_ACK;
  r = send(sock, (void *) &Command, 1, 0);
  if (r == -1) {
    return r;
  }
  tEnd  = _GetTime64();
  printf("\nClient sent %8d bytes.\n", NumBytes);
  tResult = _CalculateTransSpeed(tStart, tEnd, NumBytes);
  printf("Bytes per second:      %8ld \n", tResult);
  printf("*******************************\n");
  return 0;
}

/*********************************************************************
*
*       _HandleClient
*
*  Function description
*    Communicate with client
*/
static void _HandleClient(unsigned int sock) {
  int   r;
  char  ac[2048];

  do {
    ac[0] = 0;
    r = recv(sock, ac, 1, 0);                    // Waiting for command
    if (r <= 0) {
      break;
    }
    if (ac[0] == 'S') {
      r = _SendData(sock);
      if (r != 0) {
        break;
      }
    }
    if (ac[0] == 'R') {
      r = _ReceiveData(sock);
      if (r != 0) {
        break;
      }
    }
  } while (1);
}

/*********************************************************************
*
*       main()
*/
int main(int argc, char* argv[], char* envp[]) {
#ifdef WIN32
  WORD wVersionRequested;
  WSADATA wsaData;
#endif
  int s, sock;
  struct sockaddr addr;
  
  printf("TCP/IP Echo server V%d.%.2d.%.2d.\n", VERSION / 10000, (VERSION /100) % 100, VERSION %100);
  printf("Compiled " __TIME__ " on " __DATE__ ".\n");

#ifdef WIN32
  //
  // Initialize winsock. Required to use TCP/IP
  //
  wVersionRequested = MAKEWORD(2, 0);
  if (WSAStartup(wVersionRequested, &wsaData) != 0) {
    printf("Could not init WinSock.\n");
    return 1;
  }
#endif
  //
  // Get a socket in listen state
  //
  sock = _ListenAtTcpAddr(PORT);
  if (sock < 0) {
    goto Cleanup;
  }
  printf("Waiting for client on port %d\n", PORT);
  //
  // Loop: Wait for and handle clients
  //
  do {
    //
    // Wait for an incoming connection
    //
    SOCKLEN addrlen = sizeof(addr);
    if ((s = accept(sock, &addr, &addrlen)) < 0) {
      return 1;    // Error
    }
    printf("Client connected\n");
    // Optional: Disable Nagle's algorithm - improves performance 
    if (1) {
      int one = 1;
#ifdef WIN32
      DWORD  tv;
#else
      struct timeval tv;
#endif
      //
      // Enable keep-alive on socket + timeout on receive.
      //
      if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char *)&one, sizeof(one)) < 0) {
        printf("Couldn't set keep-alive option");
      }
#ifdef WIN32
      tv = TIMEOUT * 1000;            // Timeout in ms.
#else
      tv.tv_sec  = TIMEOUT;           // Timeout in seconds.
      tv.tv_usec = 0;
#endif
      setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(const char *)&tv,sizeof(tv));
      setsockopt(s, SOL_SOCKET, SO_SNDTIMEO,(const char *)&tv,sizeof(tv));
      setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *) &one, sizeof(one));
    } 
    _HandleClient(s);
    CLOSESOCKET(s);
    printf("Client disconnected\n");
  } while (1);

Cleanup:
  CLOSESOCKET(sock);
#ifdef WIN32
  WSACleanup();
#endif
  return 0;
}

/*************************** end of file ****************************/
