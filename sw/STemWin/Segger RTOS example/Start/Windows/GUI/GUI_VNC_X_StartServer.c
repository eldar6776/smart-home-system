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
File        : SIM_VNCServer.c
Purpose     : Windows Simulator, Start of the VNC server
---------------------------END-OF-HEADER------------------------------
*/


#include <windows.h>
#include <stdio.h>

#include "GUI.h"
#include "GUI_Debug.h"
#include "GUI_VNC.h"
#include "SIM.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/



/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

/*********************************************************************
*
*       Static variables
*
**********************************************************************
*/
static HINSTANCE _hLib;

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/

static void _LoadWSock32(void) {
  if (_hLib == 0) {
    _hLib = LoadLibrary("WSock32.dll");
  }
}

/*********************************************************************
*
*       _WSAStartup
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) {
  typedef int (__stdcall *tWSAStartup)(WORD wVersionRequested, LPWSADATA lpWSAData);
  _LoadWSock32();
  return ((tWSAStartup)GetProcAddress(_hLib, "WSAStartup"))(wVersionRequested, lpWSAData);
}


/*********************************************************************
*
*       _WSACleanup
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _WSACleanup(void) {
  typedef int (__stdcall *tWSACleanup)(void);
  _LoadWSock32();
  return ((tWSACleanup)GetProcAddress(_hLib, "WSACleanup"))();
}

/*********************************************************************
*
*       _recv
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _recv(SOCKET s, char FAR * buf, int len, int flags) {
  typedef int (__stdcall *trecv)(SOCKET s, char FAR * buf, int len, int flags);
  _LoadWSock32();
  return ((trecv)GetProcAddress(_hLib, "recv"))(s, buf, len, flags);
}

/*********************************************************************
*
*       _send
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _send(SOCKET s, const char FAR * buf, int len, int flags) {
  typedef int (__stdcall *tsend)(SOCKET s, const char FAR * buf, int len, int flags);
  _LoadWSock32();
  return ((tsend)GetProcAddress(_hLib, "send"))(s, buf, len, flags);
}

/*********************************************************************
*
*       _socket
*
* Purpose
*   Stub for the WIN32 library function
*/
static    SOCKET              _socket(int af, int type, int protocol) {
  typedef SOCKET (__stdcall *tsocket)(int af, int type, int protocol);
  _LoadWSock32();
  return ((tsocket)GetProcAddress(_hLib, "socket"))(af, type, protocol);
}

/*********************************************************************
*
*       _listen
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _listen(SOCKET s, int backlog) {
  typedef int (__stdcall *tlisten)(SOCKET s, int backlog);
  _LoadWSock32();
  return ((tlisten)GetProcAddress(_hLib, "listen"))(s, backlog);
}

/*********************************************************************
*
*       _bind
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _bind(SOCKET s, const struct sockaddr FAR *addr, int namelen) {
  typedef int (__stdcall *tbind)(SOCKET s, const struct sockaddr FAR *addr, int namelen);
  _LoadWSock32();
  return ((tbind)GetProcAddress(_hLib, "bind"))(s, addr, namelen);
}

/*********************************************************************
*
*       _closesocket
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int             _closesocket(SOCKET s) {
  typedef int (__stdcall *tclosesocket)(SOCKET s);
  _LoadWSock32();
  return ((tclosesocket)GetProcAddress(_hLib, "closesocket"))(s);
}

/*********************************************************************
*
*       _accept
*
* Purpose
*   Stub for the WIN32 library function
*/
static    SOCKET              _accept(SOCKET s, struct sockaddr FAR *addr,int FAR *addrlen) {
  typedef SOCKET (__stdcall *taccept)(SOCKET s, struct sockaddr FAR *addr,int FAR *addrlen);
  _LoadWSock32();
  return ((taccept)GetProcAddress(_hLib, "accept"))(s, addr, addrlen);
}

/*********************************************************************
*
*       _htons
*
* Purpose
*   Stub for the WIN32 library function
*/
static    u_short              _htons(u_short hostshort) {
  typedef u_short (__stdcall *thtons)(u_short hostshort);
  _LoadWSock32();
  return ((thtons)GetProcAddress(_hLib, "htons"))(hostshort);
}

/*********************************************************************
*
*       _htonl
*
* Purpose
*   Stub for the WIN32 library function
*/
static    u_long              _htonl(u_long hostlong) {
  typedef u_long (__stdcall *thtonl)(u_long hostlong);
  _LoadWSock32();
  return ((thtonl)GetProcAddress(_hLib, "htonl"))(hostlong);
}

/*********************************************************************
*
*       _ntohl
*
* Purpose
*   Stub for the WIN32 library function
*/
static    u_long              _ntohl(u_long netlong) {
  typedef u_long (__stdcall *tntohl)(u_long netlong);
  _LoadWSock32();
  return ((tntohl)GetProcAddress(_hLib, "ntohl"))(netlong);
}

/*********************************************************************
*
*       _setsockopt
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _setsockopt(SOCKET s, int level, int optname, const char FAR * optval, int optlen) {
  typedef int (__stdcall *tsetsockopt)(SOCKET s, int level, int optname, const char FAR * optval, int optlen);
  _LoadWSock32();
  return ((tsetsockopt)GetProcAddress(_hLib, "setsockopt"))(s, level, optname, optval, optlen);
}



/*********************************************************************
*
*       _ListenAtTcpAddr
*
* Starts listening at the given TCP port.
*/
static int _ListenAtTcpAddr(unsigned short port) {
  #define DO(x) if ((x) < 0) { _closesocket(sock); return -1; }
  int sock;
  struct sockaddr_in addr;
  int one = 1;

  addr.sin_family = AF_INET;
  addr.sin_port = _htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  DO(sock = _socket(AF_INET, SOCK_STREAM, 0));
  DO(_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &one, sizeof(one)));
  DO(_bind(sock, (struct sockaddr *)&addr, sizeof(addr)));
  DO(_listen(sock, 5));
  return sock;
}

static int _Send(const U8* buf, int len, void* pConnectionInfo) {
  return _send((SOCKET)pConnectionInfo, buf, len, 0);
}

static int _Recv(U8* buf, int len, void* pConnectionInfo) {
  return _recv((SOCKET)pConnectionInfo, buf, len, 0);
}

/*********************************************************************
*
*       _ServerThread
*
* Purpose
*   The server thread. The 32 bit parameter is used to pass the
*   LayerIndex and ServerIndex.
*/
static UINT __stdcall _ServerThread(void* pPara) {
  WORD wVersionRequested;
  WSADATA wsaData;
  int s, sock;
  struct sockaddr addr;
  GUI_VNC_CONTEXT * pContext = (GUI_VNC_CONTEXT *)pPara;
  // Initialise winsock
  wVersionRequested = MAKEWORD(2, 0);
  if (_WSAStartup(wVersionRequested, &wsaData) != 0) {
    return 1;
  }
  s = _ListenAtTcpAddr(5900 + pContext->ServerIndex);
  if (s < 0) {
    return 1;    // Error
  }
  while (1) {
    int r;
    // Wait for an incoming connection
    int addrlen = sizeof(addr);
    if ((sock = _accept(s, &addr, &addrlen)) < 0) {
      return 1;    // Error
    }
    // Optional: Disable Nagle's algorithm - improves performance 
    if (0) {
      int one = 1;
      _setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *) &one, sizeof(one));
    }
    r = GUI_VNC_Process(pContext, _Send, _Recv, (void*)sock);    // Then process this client
    switch (r) {
    case GUI_VNC_ERROR_WRONGFORMAT:
      GUI_DEBUG_ERROROUT("VNC: Client requests unsupported pixel format");
      break;
    }
    _closesocket(sock);
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_VNC_X_StartServer
*/
int GUI_VNC_X_StartServer(int LayerIndex, int ServerIndex) {
  int ThreadId;
  HANDLE hThread;
  GUI_VNC_CONTEXT * pContext = malloc(sizeof(GUI_VNC_CONTEXT));
  if (!pContext) {
    return 1;   // No memory error
  }
  GUI_VNC_AttachToLayer(pContext, LayerIndex);
  pContext->ServerIndex = ServerIndex;
  hThread = CreateThread(NULL, 0, _ServerThread, (void*)pContext, 0, &ThreadId);
  return 0;
}

/*************************** End of file ****************************/
