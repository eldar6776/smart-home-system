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
File        : FTPServer_Win32.c
Purpose     : Windows Simulator, Start of the FTP server
---------------------------END-OF-HEADER------------------------------
*/

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "IP_FTPServer.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define MAX_CONNECTIONS 2

enum {
  USER_ID_ANONYMOUS = 1,
  USER_ID_ADMIN
};

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static int  _ConnectCnt;

/*********************************************************************
*
*       Static variables
*
**********************************************************************
*/
static HINSTANCE _hLib;
static const IP_FS_API * _pFS_API;     // File system info

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
*       _getpeername
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _getpeername(SOCKET s, struct sockaddr FAR *addr,int FAR *addrlen) {
  typedef int (__stdcall *tgetpeername)(SOCKET s, struct sockaddr FAR *addr,int FAR *addrlen);
  _LoadWSock32();
  return ((tgetpeername)GetProcAddress(_hLib, "getpeername"))(s, addr, addrlen);
}

/*********************************************************************
*
*       _getsockname
*
* Purpose
*   Stub for the WIN32 library function
*/
static    int              _getsockname(SOCKET s, struct sockaddr FAR *addr,int FAR *addrlen) {
  typedef int (__stdcall *tgetsockname)(SOCKET s, struct sockaddr FAR *addr,int FAR *addrlen);
  _LoadWSock32();
  return ((tgetsockname)GetProcAddress(_hLib, "getsockname"))(s, addr, addrlen);
}

/*********************************************************************
*
*       _connect
*
* Purpose
*   Stub for the WIN32 library function
*/
static    SOCKET              _connect(SOCKET s, const struct sockaddr FAR *name,int len) {
  typedef SOCKET (__stdcall *tconnect)(SOCKET s, const struct sockaddr FAR *name,int len);
  _LoadWSock32();
  return ((tconnect)GetProcAddress(_hLib, "connect"))(s, name, len);
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

/**************************************************************************************************************************************************************
*
*       IP interface.
*
*  Mapping of the required socket functions to the actual IP stack.
*  This is required becasue the socket functions are slightly different on different systems.
*/

static int _Send(const unsigned char * buf, int len, void* pConnectionInfo) {
  return _send((SOCKET)pConnectionInfo, buf, len, 0);
}

static int _Recv(unsigned char * buf, int len, void* pConnectionInfo) {
  return _recv((SOCKET)pConnectionInfo, buf, len, 0);
}

static FTPS_SOCKET _Connect(FTPS_SOCKET CtrlSocket, U16 Port) {
  int                DataSock;
  struct sockaddr_in PeerAddr;
  int                ConnectStatus;
  int                AddrSize;

  DataSock = _socket(AF_INET, SOCK_STREAM, 0);  // Create a new socket for data connection to the client
  if (DataSock > 0) {                          // Socket created?
    //
    //  Get IP address of connected client and connect to listening port
    //
    AddrSize = sizeof(struct sockaddr_in);
    _getpeername((SOCKET)CtrlSocket, (struct sockaddr *)&PeerAddr, &AddrSize);
    PeerAddr.sin_port = _htons(Port);
    ConnectStatus  = _connect(DataSock, (struct sockaddr *)&PeerAddr, sizeof(struct sockaddr_in));
    if (ConnectStatus == 0) {
      return (void*)DataSock;
    }
  }
  return NULL;
}

static void _Disconnect(FTPS_SOCKET socket) {
  _closesocket((SOCKET) socket);
}

static FTPS_SOCKET _Listen(FTPS_SOCKET CtrlSocket, U16 *pPort, U8 * pIPAddr) {
  struct sockaddr_in addr;
  U16  Port;
  int  DataSock;
  int  AddrSize;

  addr.sin_family = AF_INET;
  addr.sin_port   = 0;                          // Let Stack find a free port
  addr.sin_addr.s_addr = INADDR_ANY;
  DataSock = _socket(AF_INET, SOCK_STREAM, 0);  // Create a new socket for data connection to the client

  _bind(DataSock, (struct sockaddr *)&addr, sizeof(addr));
  _listen(DataSock, 1);

  //
  //  Get port number stack has assigned
  //
  AddrSize = sizeof(struct sockaddr_in);
  _getsockname((SOCKET)DataSock, (struct sockaddr *)&addr, &AddrSize);
  Port = htons(addr.sin_port);
  *pPort = Port;

  _getsockname((SOCKET)CtrlSocket, (struct sockaddr *)&addr, &AddrSize);
  Port = htons(addr.sin_port);

  *pIPAddr++ = addr.sin_addr.S_un.S_un_b.s_b1;
  *pIPAddr++ = addr.sin_addr.S_un.S_un_b.s_b2;
  *pIPAddr++ = addr.sin_addr.S_un.S_un_b.s_b3;
  *pIPAddr   = addr.sin_addr.S_un.S_un_b.s_b4;

  return (FTPS_SOCKET)DataSock;
}

static int _Accept(FTPS_SOCKET CtrlSocket, FTPS_SOCKET * pSocket) {
  int Socket;
  struct sockaddr Addr; 
  int AddrSize;

  AddrSize = sizeof(Addr);
// Wait for an incoming connection
  Socket = (int)*pSocket;
  Socket = _accept(Socket, &Addr, &AddrSize);
  if (Socket < 0) {
    return -1;    // Error
  }
  *pSocket = (FTPS_SOCKET)Socket;
  return 0;
}

static const IP_FTPS_API _IP_API = {
  _Send,
  _Recv,
  _Connect,
  _Disconnect,
  _Listen,
  _Accept
};

/**************************************************************************************************************************************************************
*
*       User management.
*/

/*********************************************************************
*
*       _FindUser
*
*  Function description
*    Callback function for user management.
*    Checks if user name is valid.
*
*  Return value
*    0    UserID invalid or unknown
*  > 0    UserID, no password required
*  < 0    - UserID, password required
*/
static int _FindUser (const char * sUser) {
  if (strcmp(sUser, "Admin") == 0) {
    return (0 - USER_ID_ADMIN);
  }
  if (strcmp(sUser, "anonymous") == 0) {
    return USER_ID_ANONYMOUS;
  }
  return 0;
}

/*********************************************************************
*
*       _CheckPass
*
*  Function description
*    Callback function for user management.
*    Checks user password.
*
*  Return value
*    0    UserID know, password valid
*    1    UserID unknown or password invalid
*/
static int _CheckPass (int UserId, const char * sPass) {
  if ((UserId == USER_ID_ADMIN) && (strcmp(sPass, "Secret") == 0)) {
    return 0;
  } else {
    return 1;
  }
}

/*********************************************************************
*
*       _GetDirInfo
*
*  Function description
*    Callback function for permission management.
*    Checks directory permissions.
*
*  Return value
*    Returns a combination of the following:
*    IP_FTPS_PERM_VISIBLE    - Directory is visible as a directory entry
*    IP_FTPS_PERM_READ       - Directory can be read/entered
*    IP_FTPS_PERM_WRITE      - Directory can be written to
*
*  Parameters
*    UserId        - User ID returned by _FindUser()
*    sDirIn        - Full directory path and with trailing slash
*    sDirOut       - Reserved for future use
*    DirOutSize    - Reserved for future use
*
*  Notes
*    In this sample configuration anonymous user is allowed to do anything.
*    Samples for folder permissions show how to set permissions for different
*    folders and users. The sample configures permissions for the following
*    directories:
*      - /READONLY/: This directory is read only and can not be written to.
*      - /VISIBLE/ : This directory is visible from the folder it is located
*                    in but can not be accessed.
*      - /ADMIN/   : This directory can only be accessed by the user "Admin".
*/
static int _GetDirInfo(int UserId, const char * sDirIn, char * sDirOut, int DirOutSize) {
  int Perm;

  (void)sDirOut;
  (void)DirOutSize;

  Perm = IP_FTPS_PERM_VISIBLE | IP_FTPS_PERM_READ | IP_FTPS_PERM_WRITE;

  if (strcmp(sDirIn, "/READONLY/") == 0) {
    Perm = IP_FTPS_PERM_VISIBLE | IP_FTPS_PERM_READ;
  }
  if (strcmp(sDirIn, "/VISIBLE/") == 0) {
    Perm = IP_FTPS_PERM_VISIBLE;
  }
  if (strcmp(sDirIn, "/ADMIN/") == 0) {
    if (UserId != USER_ID_ADMIN) {
      return 0;  // Only Admin is allowed for this directory
    }
  }
  return Perm;
}

/*********************************************************************
*
*       _GetFileInfo
*
*  Function description
*    Callback function for permission management.
*    Checks file permissions.
*
*  Return value
*    Returns a combination of the following:
*    IP_FTPS_PERM_VISIBLE    - File is visible as a file entry
*    IP_FTPS_PERM_READ       - File can be read
*    IP_FTPS_PERM_WRITE      - File can be written to
*
*  Parameters
*    UserId        - User ID returned by _FindUser()
*    sFileIn       - Full path to the file
*    sFileOut      - Reserved for future use
*    FileOutSize   - Reserved for future use
*
*  Notes
*    In this sample configuration all file accesses are allowed. File
*    permissions are checked against directory permissions. Therefore it
*    is not necessary to limit permissions on files that reside in a
*    directory that already limits access.
*    Setting permissions works the same as for _GetDirInfo() .
*/
static int _GetFileInfo(int UserId, const char * sFileIn, char * sFileOut, int FileOutSize) {
  int Perm;

  (void)UserId;
  (void)sFileIn;
  (void)sFileOut;
  (void)FileOutSize;

  Perm = IP_FTPS_PERM_VISIBLE | IP_FTPS_PERM_READ | IP_FTPS_PERM_WRITE;

  return Perm;
}

/*********************************************************************
*
*       FTPS_ACCESS_CONTROL
*
*  Description
*   Access control function table
*/
static FTPS_ACCESS_CONTROL _Access_Control = {
  _FindUser,
  _CheckPass,
  _GetDirInfo,
  _GetFileInfo  // Optional, only required if permissions for individual files shall be used
};

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
*
*  Note:
*    FTP server requires a real time clock for to transmit the
*    correct timestamp of files. Lists transmits either the
*    year or the HH:MM. For example:
*    -rw-r--r--   1 root 1000 Jan  1  1980 DEFAULT.TXT
*    or
*    -rw-r--r--   1 root 1000 Jul 29 11:40 PAKET01.TXT
*    The decision which of both infos the server transmits
*    depends on the system time. If the year of the system time
*    is identical to the year stored in the timestamp of the file,
*    the time will be transmitted, if not the year.
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
  Year  = 28;        // 1980 based. Means that 2008 would be 28.
  r   = Sec / 2 + (Min << 5) + (Hour  << 11);
  r  |= (U32)(Day + (Month << 5) + (Year  << 9)) << 16;
  return r;
}

static const FTPS_APPLICATION _Application = {
  &_Access_Control,
  _GetTimeDate
};

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
*       _ServerThread
*
* Purpose
*   The server thread. The 32 bit parameter is used to pass the
*   socket.
*/
static UINT __stdcall _ServerThread(void * Context) {
  struct sockaddr_in RAddr;
  int AddrSize;
  int Sock;


  Sock = (int)Context;
  IP_FTPS_Process(&_IP_API, Context, _pFS_API, &_Application);
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
*       _FTPServerParent
*/
static int _FTPServerParent(const IP_FS_API * pFS_API) {
  WORD wVersionRequested;
  WSADATA wsaData;
  int s, sock;
  struct sockaddr addr;
  int ThreadId;
  
  _pFS_API = pFS_API;
  // Initialize winsock
  wVersionRequested = MAKEWORD(2, 0);
  if (_WSAStartup(wVersionRequested, &wsaData) != 0) {
    return 1;
  }
  s = _ListenAtTcpAddr(21);
  if (s < 0) {
    return 1;    // Error
  }
  while (1) {
    struct sockaddr_in RAddr; 
    int AddrSize;
    // Wait for an incoming connection
    int addrlen = sizeof(addr);
    if ((sock = _accept(s, &addr, &addrlen)) < 0) {
      return 1;    // Error
    }
    // Optional: Disable Nagle's algorithm - improves performance 
    if (1) {
      int one = 1;
      _setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *) &one, sizeof(one));
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
    if (_ConnectCnt++ < MAX_CONNECTIONS) {
      CreateThread(NULL, 0, _ServerThread, (void*)sock, 0, &ThreadId);
    } else {
      IP_FTPS_OnConnectionLimit(&_IP_API, (void*)sock);
      Sleep(2000);          // Give connection some time to complete
      _closesocket(sock);
      _ConnectCnt--;
    }
  }
  return 0;
}

/*********************************************************************
*
*       main()
*/
int main(int argc, char* argv[], char* envp[]) { 
  printf("Windows FTP server\n");
  printf("Compiled " __TIME__ " on " __DATE__ ".\n\n");
  printf("Using windows filesystem (C:\\ftp\\).\n");
  _FTPServerParent( &IP_FS_Win32 );
  return 0;
}

/*************************** End of file ****************************/

