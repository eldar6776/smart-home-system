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
File        : WebServer_Win32.c
Purpose     : Windows Simulator
---------------------------END-OF-HEADER------------------------------
*/

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "IP_WebServer.h"
#include "IP_UTIL.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   USE_RO_FS
  #define USE_RO_FS      1  // 0: Use windows file system to store websites
#endif                      // 1: Use read only file system with websites compiled into binary

#define MAX_CONNECTIONS  2

/*********************************************************************
*
*       Defines, defaults
*
**********************************************************************
*/

#ifndef   WEBS_SUPPORT_UPLOAD
  #define WEBS_SUPPORT_UPLOAD  0
#endif

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/
// None.

/*********************************************************************
*
*       Extern
*
**********************************************************************
*/

extern const WEBS_CGI    aCGI[];
extern const WEBS_VFILES aVFiles[];

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static int _ConnectCnt;

//
// Variables used for METHOD hook demonstration (basic REST implementation)
//
static WEBS_METHOD_HOOK _MethodHook;
static char _acRestContent[20];


/*********************************************************************
*
*       Static variables
*
**********************************************************************
*/

static HINSTANCE _hLib;
static const IP_FS_API    *_pFS_API;
static const WEBS_IP_API   _Webs_IP_API;
static const WEBS_SYS_API  _Webs_SYS_API;

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
*       _recv
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _recv(SOCKET s, char FAR * buf, int len, int flags) {
  typedef int (__stdcall *trecv)(SOCKET s, char FAR * buf, int len, int flags);
  static trecv pf;
  _LoadWSock32();
  if (pf == NULL) {
    pf = (trecv)GetProcAddress(_hLib, "recv");
  }
  return (pf)(s, buf, len, flags);
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
  DO(_bind(sock, (struct sockaddr *)&addr, sizeof(addr)));
  DO(_listen(sock, 1));
  return sock;
}

static int _Send(const unsigned char * buf, int len, void* pConnectionInfo) {
  return _send((SOCKET)pConnectionInfo, buf, len, 0);
}

static int _Recv(unsigned char * buf, int len, void* pConnectionInfo) {
  return _recv((SOCKET)pConnectionInfo, buf, len, 0);
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
*    Wrapper for malloc(). (embOS/IP: IP_Alloc())
*/
static void * _Alloc(U32 NumBytesReq) {  
  return malloc(NumBytesReq);
}

/*********************************************************************
*
*       _Free()
*
*  Function description
*    Wrapper for free(). (embOS/IP: IP_Free())
*/
static void _Free(void * p) {
  free(p);
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
*       _CopyString
*/
static void _CopyString(char * sDest, const char * sSrc, int DestSize) {
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
*       WIN32_OutputDebugStringf
*/
void WIN32_OutputDebugStringf(const char * sFormat, ...) {
  char ac[2048];
  va_list ParamList;
  if (sFormat) {
    if (strlen(sFormat) < 1024) {
      va_start(ParamList, sFormat);
      vsprintf(ac, sFormat, ParamList);
      OutputDebugString(ac);
    }
  }
}

/*********************************************************************
*
*       _CB_HandleParameter
*
*  Function description
*    TBD.
*/
static void _CB_HandleParameter(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  (void)pOutput;
  (void)sPara;
  (void)sValue;
}

/*********************************************************************
*
*       WEBS_APPLICATION
*
*  Description
*   Application data table, defines all application specifics used by the web server
*/
static const WEBS_APPLICATION _Application = {
  &aCGI[0],
  &aAccessControl[0],
  _CB_HandleParameter,
  &aVFiles[0]  
};

/*********************************************************************
*
*       _ServerThread
*
* Purpose
*   The server thread. The 32 bit parameter is used to pass the
*   socket.
*/
static UINT __stdcall _ServerThread(void * pContext) {
  WEBS_CONTEXT ChildContext;
  struct sockaddr_in RAddr;
  int AddrSize;
  int Sock;

  Sock = (int)pContext;
#if (WEBS_SUPPORT_UPLOAD == 1)
  IP_WEBS_AddUpload();
#endif
  IP_WEBS_Init(&ChildContext, &_Webs_IP_API, &_Webs_SYS_API, _pFS_API, &_Application);  // Initialize the context of the child task.  
  if (_ConnectCnt < MAX_CONNECTIONS) {
    IP_WEBS_ProcessEx(&ChildContext, pContext, NULL);
  } else {
    IP_WEBS_ProcessLastEx(&ChildContext, pContext, NULL);
  }
  AddrSize = sizeof(RAddr);
  getpeername(Sock, (struct sockaddr*)&RAddr, &AddrSize);
  WIN32_OutputDebugStringf("Closing connection to %d.%d.%d.%d:%d\n"
                            ,RAddr.sin_addr.S_un.S_un_b.s_b1
                            ,RAddr.sin_addr.S_un.S_un_b.s_b2
                            ,RAddr.sin_addr.S_un.S_un_b.s_b3
                            ,RAddr.sin_addr.S_un.S_un_b.s_b4
                            ,htons(RAddr.sin_port)
                          );
  Sleep(100);          // Give connection some time to complete
  _closesocket(Sock);
  _ConnectCnt--;
  return 0;
}

/*********************************************************************
*
*       _GetTimeDate
*
*  Description:
*    Current time and date in a format suitable for the FTP server.
*
*    Bit 0-4:   2-second count (0-29)
*    Bit 5-10:  Minutes (0-59)
*    Bit 11-15: Hours (0-23)
*    Bit 16-20: Day of month (1-31)
*    Bit 21-24: Month of year (1-12)
*    Bit 25-31: Count of years from 1980 (0-127)
*/
static U32 _GetTimeDate(void) {
  U32 r;
  U16 Sec, Min, Hour;
  U16 Day, Month, Year;

  Sec   = 0;        // 0 based. Valid range: 0..59
  Min   = 0;        // 0 based. Valid range: 0..59
  Hour  = 0;        // 0 based. Valid range: 0..23
  Day   = 1;        // 1 based. Means that 1 is 1. Valid range is 1..31 (depending on month)
  Month = 1;        // 1 based. Means that January is 1. Valid range is 1..12.
  Year  = 15;       // 1980 based. Means that 2008 would be 28.
  r   = Sec / 2 + (Min << 5) + (Hour  << 11);
  r  |= (U32)(Day + (Month << 5) + (Year  << 9)) << 16;
  return r;
}

/*********************************************************************
*
*       _pfGetFileInfo
*
*  Function description:
*    Returns the file information used to differentiate between static
*    and dynamic content.
*/
static void _pfGetFileInfo(const char * sFilename, IP_WEBS_FILE_INFO * pFileInfo){
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
    // Set last-modified and expiration time and date
    //
    pFileInfo->DateExp     = _GetTimeDate(); // If "Expires" HTTP header field should be transmitted, set expiration date.
  } else {
    pFileInfo->DateExp     = 0xEE210000; // Expiration far away (01 Jan 2099) if content is static
  }
}

/*********************************************************************
*
*       _WebserverParent
*/
static int _WebserverParent(const IP_FS_API * pFS_API) {
  WORD wVersionRequested;
  WSADATA wsaData;
  int s, sock;
  struct sockaddr addr;
  int ThreadId;
  int t;
  int t0;
  
  IP_WEBS_SetFileInfoCallback(&_pfGetFileInfo);
  _pFS_API = pFS_API;
  //
  // Initialise winsock
  //
  wVersionRequested = MAKEWORD(2, 0);
  if (_WSAStartup(wVersionRequested, &wsaData) != 0) {
    return 1;
  }
  s = _ListenAtTcpAddr(8181);  // Use a port other than 80 as on Windows 10 this port is used by default.
  if (s < 0) {
    return 1;    // Error
  }
  while (1) {
    struct sockaddr_in RAddr; 
    int AddrSize;
    // 
    // Wait for an incoming connection
    //
    int addrlen = sizeof(addr);
    if ((sock = _accept(s, &addr, &addrlen)) < 0) {
      return 1;    // Error
    }
    AddrSize = sizeof(RAddr);
    getpeername(sock, (struct sockaddr*)&RAddr, &AddrSize);
    WIN32_OutputDebugStringf("New connection to %d.%d.%d.%d:%d\n"
                              ,RAddr.sin_addr.S_un.S_un_b.s_b1
                              ,RAddr.sin_addr.S_un.S_un_b.s_b2
                              ,RAddr.sin_addr.S_un.S_un_b.s_b3
                              ,RAddr.sin_addr.S_un.S_un_b.s_b4
                              ,htons(RAddr.sin_port)
    );
    //
    // Create server thread to handle connection.
    // If connection limit is reached, keep trying for some time before giving up and outputting an error message
    //
    t0 = GetTickCount() + 1000;
    do {
      if (_ConnectCnt < MAX_CONNECTIONS) {
        _ConnectCnt++;
        CreateThread(NULL, 0, _ServerThread, (void*)sock, 0, &ThreadId);
        break;
      }
      //
      // Check time out
      //
      t = GetTickCount();
      if ((t - t0) > 0) {
        IP_WEBS_OnConnectionLimit(_Send, _Recv, (void*)sock);
        Sleep(100);          // Give connection some time to complete
        _closesocket(sock);
        break;
      }
      Sleep(10);
    } while (1);
  }
  return 0;
}

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
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       main()
*/
int main(int argc, char* argv[], char* envp[]) { 
  printf("Windows Webserver running on local port 8181\n");
  printf("Compiled " __TIME__ " on " __DATE__ ".\n\n");
  IP_WEBS_METHOD_AddHook(&_MethodHook, _REST_cb, "/REST");  // Register URI "http://<ip>/REST" for demonstrational REST implementation
#if USE_RO_FS
  printf("Using read only filesystem.\n");
  printf("Upload disabled. No write access to RO filesystem.\n");
  printf("\n");
  printf("To switch to windows filesystem set the define\n");
  printf("\"USE_RO_FS=0\" in your project.\n");
  printf("\n");
  printf("To enable webserver upload set the define\n");
  printf("\"WEBS_SUPPORT_UPLOAD=1\" in your project.\n");
  printf("Only when NOT using RO filesystem.\n");
  _WebserverParent(&IP_FS_ReadOnly);
#else
  printf("Using windows filesystem (C:\\ftp\\).\n\n");
#if (WEBS_SUPPORT_UPLOAD == 0)
  printf("Upload disabled.\n");
  printf("To enable webserver upload set the define\n");
  printf("\"WEBS_SUPPORT_UPLOAD=1\" in your project\n");
  printf("and call IP_WEBS_AddUpload() in your application.\n");  
#endif
  _WebserverParent( &IP_FS_Win32 );
#endif
  return 0;
}

/*************************** End of file ****************************/
