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
File    : Webserver.c
Purpose : Sample program for embOS & TCP/IP.
--------- END-OF-HEADER --------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include "RTOS.h"
#include "BSP.h"
#include "IP.h"
#else
#include <Windows.h>
#include <time.h>
#endif

#include "TaskPrio.h"
#include "IP_Webserver.h"
#include "WEBS_Conf.h"        // Stack size depends on configuration

/*********************************************************************
*
*       Local defines, configurable
*
**********************************************************************
*/
#define MAX_CONNECTION_INFO      10
#define NUM_STOCKS               26
#define NUM_VALUES               30

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
*       typedefs
*
**********************************************************************
*/
typedef struct STOCK_INFO {
  int        Price[30];
  int        Size;
  int        RdOff;
  char       CompanyName;
} STOCK_INFO;

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
//
// Variables used for CGI samples
//
static U32  _Cnt;
static U32  _Percentage = 50;
static char _aLEDState[8];
static char _aLEDStateNew[8];
static char _acFirstName[12];
static char _acLastName[12];

#if INCLUDE_SHARE_SAMPLE
static STOCK_INFO _StockInfo[26];
#endif

static U32 _aPool[((WEBS_IN_BUFFER_SIZE + WEBS_OUT_BUFFER_SIZE + WEBS_PARA_BUFFER_SIZE + WEBS_FILENAME_BUFFER_SIZE + WEBS_MAX_ROOT_PATH_LEN) * MAX_CONNECTIONS) / sizeof(int)];  // Memory pool for the Web server child tasks

static int                     _IFaceId;
static int                     _ConnectCnt;

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

/******************************************************************************************************************************************
*
*       Static code
*
*******************************************************************************************************************************************
*/

/*********************************************************************
*
*       _atoi
*
*  Function description
*    Local helper function. Converts a string to an integer
*/
static int _atoi(const char  * sValue) {
  int Value;
  int Digit;
  char c;

  Value = 0;
  while ((c = *sValue++) != '\0') {
    if (c >= '0' && c <= '9') {
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
*       _CopyString
*
*  Function description
*    Local helper function. Copies a string.
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
*       _callback_DefaultHandler
*/
static void _callback_DefaultHandler(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  WEBS_USE_PARA(sValue);
  IP_WEBS_SendString(pOutput, "<h1 style=\"color:red\">Unknown CGI ");
  IP_WEBS_SendString(pOutput, sPara);
  IP_WEBS_SendString(pOutput, " - Ignored</h1>");
}

/*********************************************************************
*
*       _callback_ExecCounter
*/
static void _callback_ExecCounter(WEBS_OUTPUT * pOutput, const char * sParameters, const char * sValue ) {
  char ac[80];

  WEBS_USE_PARA(sParameters);
  WEBS_USE_PARA(sValue);
  _Cnt++;
  SEGGER_snprintf(ac, sizeof(ac), "<br><span class=\"hint\">Current page hit count: %lu</span></ul>", _Cnt);
  IP_WEBS_SendString(pOutput, ac);
}

/*********************************************************************
*
*       _GetNetworkInfo
*
*  Function description
*    Sends the network configuration of the target infromation
*    and the connection information to the browser.
*/
static void _GetNetworkInfo(WEBS_OUTPUT * pOutput) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<H2>This sample runs on a Windows host. No embOS/IP statistics available.</H2>");
#else
  IP_CONNECTION_HANDLE hConnection[MAX_CONNECTION_INFO];
  IP_CONNECTION Info;
  int  NumConnections;
  int  NumValidConnections;
  int  i;
  int  r;
  char ac[16];
  U32  IPAddr;

  IP_WEBS_SendString(pOutput, "<h2>Configuration</h2>");
  IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  IP_WEBS_SendString(pOutput, "Server IP<br>");
  IP_WEBS_SendString(pOutput, "Gateway IP<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"colon\">");
  IP_WEBS_SendString(pOutput, ":<br>");
  IP_WEBS_SendString(pOutput, ":<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"value\">");
  IPAddr = IP_GetIPAddr(0);
  IP_PrintIPAddr(ac, IPAddr, sizeof(ac));
  IP_WEBS_SendString(pOutput, ac);
  IPAddr = IP_GetGWAddr(0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_PrintIPAddr(ac, IPAddr, sizeof(ac));
  IP_WEBS_SendString(pOutput, ac);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  OS_EnterRegion();
#if 0
  //
  // Buffer configuration of the stack
  //
  IP_WEBS_SendString(pOutput, "<h2>Stack buffer Info</h2>");
  IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  IP_WEBS_SendString(pOutput, "Small buffers<br>");
  IP_WEBS_SendString(pOutput, "Small buffer size<br>");
  IP_WEBS_SendString(pOutput, "Big buffer<br>");
  IP_WEBS_SendString(pOutput, "Big buffer size<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"colon\">");
  IP_WEBS_SendString(pOutput, ":<br>:<br>:<br>:<br>");
  IP_WEBS_SendString(pOutput, "<div class=\"value\">");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_AVAIL), 10, 0);
  IP_WEBS_SendString(pOutput, "/");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_CONFIG), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_SIZE), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_AVAIL), 10, 0);
  IP_WEBS_SendString(pOutput, "/");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_CONFIG), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_SIZE), 10, 0);
  IP_WEBS_SendString(pOutput, "</div>");
#endif
  IP_WEBS_SendString(pOutput, "<h2>Connection info</h2>");
  //
  // Number of connections
  //
  IP_WEBS_SendString(pOutput, "Total connections: ");
  NumConnections = IP_INFO_GetConnectionList(&hConnection[0], MAX_CONNECTION_INFO);
  NumValidConnections = NumConnections;
  for (i = 0; i < NumConnections; i++) {
    r = IP_INFO_GetConnectionInfo(hConnection[i], &Info);
    if (r != 0) {
      NumValidConnections--;
    }
  }
  IP_WEBS_SendUnsigned(pOutput, NumValidConnections, 10, 0);
  //
  // Table with connection infos
  //
  IP_WEBS_SendString(pOutput, "<h2>List of TCP connections</h2>");
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Socket</th>");
  IP_WEBS_SendString(pOutput, "<th>Local</td>");
  IP_WEBS_SendString(pOutput, "<th>Peer</td>");
  IP_WEBS_SendString(pOutput, "<th>State</td>");
  IP_WEBS_SendString(pOutput, "<th>MTU/MSS</td>");
  IP_WEBS_SendString(pOutput, "<th>Retrans. delay</td>");
  IP_WEBS_SendString(pOutput, "<th>Idle time</td>");
  IP_WEBS_SendString(pOutput, "<th>Local window</td>");
  IP_WEBS_SendString(pOutput, "<th>Peer window</td>");
  for (i = 0; i < NumConnections; i++) {
    r = IP_INFO_GetConnectionInfo(hConnection[i], &Info);
    if (r == 0) {
      IP_WEBS_SendString(pOutput, "<tr><td>");
      IP_WEBS_SendUnsigned(pOutput, (unsigned)Info.hSock, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output Local Addr & Port, such as    "192.168.1.1:81"
      //
      if (Info.LocalAddr == 0) {
        IP_WEBS_SendString(pOutput, "Any");
      } else {
        IP_PrintIPAddr(ac, Info.LocalAddr, sizeof(ac));
        IP_WEBS_SendString(pOutput, ac);
      }
      IP_WEBS_SendString(pOutput, ":");
      IP_WEBS_SendUnsigned(pOutput, Info.LocalPort, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output Peer Addr & Port, such as    "192.168.1.1:81"
      //
      if (Info.ForeignAddr == 0) {
        IP_WEBS_SendString(pOutput, "---");
      } else {
        IP_PrintIPAddr(ac, Info.ForeignAddr, sizeof(ac));
        IP_WEBS_SendString(pOutput, ac);
        IP_WEBS_SendString(pOutput, ":");
        IP_WEBS_SendUnsigned(pOutput, Info.ForeignPort, 10, 0);
      }
      IP_WEBS_SendString(pOutput, "</td><td class=\"alignLeft\">");
      //
      // Output State, such as   "LISTEN"
      //
      IP_WEBS_SendString(pOutput, IP_INFO_ConnectionState2String(Info.TcpState));
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output MTU/MSS
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpMtu, 10, 0);
      IP_WEBS_SendString(pOutput, "/");
      IP_WEBS_SendUnsigned(pOutput, Info.TcpMss, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output retrans. delay
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpRetransDelay, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output TCP idle time
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpIdleTime, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output local window size
      //
      IP_WEBS_SendUnsigned(pOutput, Info.RxWindowCur, 10, 0);
      IP_WEBS_SendString(pOutput, "/");
      IP_WEBS_SendUnsigned(pOutput, Info.RxWindowMax, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output peer window size
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TxWindow, 10, 0);
      IP_WEBS_SendString(pOutput, "</td></tr>");
    }
  }
  OS_LeaveRegion();
  IP_WEBS_SendString(pOutput, "</table>");
#endif
}

/*********************************************************************
*
*       _callback_ExecGetConnectionInfo
*/
static void _callback_ExecGetConnectionInfo(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);
  _GetNetworkInfo(pOutput);
}

/*********************************************************************
*
*       _GetOSInfo
*
*  Function description
*    This function generates and sends the HTML page with embOS
*    system information. It is called by _callback_ExecGetOSInfo()
*    and _callback_CGI_SSEembOS().
*/
static void _GetOSInfo(WEBS_OUTPUT * pOutput) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<H2>This sample runs on a Windows host. No embOS statistics available.</H2>");
#else
  OS_TASK *pTask;
  //
  // Get embOS information and build webpage
  //
  IP_WEBS_SendString(pOutput, "<h2>System info</h2>");
  IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  IP_WEBS_SendString(pOutput, "Number of tasks<br>");
  IP_WEBS_SendString(pOutput, "System time<br>");
  IP_WEBS_SendString(pOutput, "System stack (size@base)<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"colon\">");
  IP_WEBS_SendString(pOutput, ":<br>:<br>:<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  OS_EnterRegion();
  IP_WEBS_SendString(pOutput, "<div class=\"value\">");
  IP_WEBS_SendUnsigned(pOutput, OS_GetNumTasks(), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, OS_GetTime32(), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, OS_GetSysStackSize(), 10, 0);
  IP_WEBS_SendString(pOutput, "@0x");
  IP_WEBS_SendUnsigned(pOutput, (OS_U32)OS_GetSysStackBase(), 16, 0);
  IP_WEBS_SendString(pOutput, "</div>");
  //
  // Table with task infos
  //
  IP_WEBS_SendString(pOutput, "<h2>List of tasks</h2>");
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Id</td>");
  IP_WEBS_SendString(pOutput, "<th>Priority</td>");
  IP_WEBS_SendString(pOutput, "<th>Task names</td>");
  IP_WEBS_SendString(pOutput, "<th>Context switches</td>");
  IP_WEBS_SendString(pOutput, "<th>Task Stack</td>");
  IP_WEBS_SendString(pOutput, "</tr>");
  for (pTask = OS_pTask; pTask; pTask = pTask->pNext) {
    IP_WEBS_SendString(pOutput, "<tr>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendString(pOutput, "0x");
    IP_WEBS_SendUnsigned(pOutput, (U32)pTask, 16, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Priorities
    //
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, OS_GetPriority(pTask), 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Task names
    //
    IP_WEBS_SendString(pOutput, "<td class=\"alignLeft\">");
    IP_WEBS_SendString(pOutput, OS_GetTaskName(pTask));
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // NumActivations
    //
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, OS_STAT_GetNumActivations(pTask), 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Task stack info
    //
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, OS_GetStackUsed(pTask), 10, 0);
    IP_WEBS_SendString(pOutput, "/");
    IP_WEBS_SendUnsigned(pOutput, OS_GetStackSize(pTask), 10, 0);
    IP_WEBS_SendString(pOutput, "@0x");
    IP_WEBS_SendUnsigned(pOutput, (U32)OS_GetStackBase(pTask), 16, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "</tr>");
  }
  OS_LeaveRegion();
  IP_WEBS_SendString(pOutput, "</table>");
#endif
}

/*********************************************************************
*
*       _callback_ExecGetOSInfo
*/
static void _callback_ExecGetOSInfo(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);
  _GetOSInfo(pOutput);
}

#if 0
/*********************************************************************
*
*       _callback_ExecGetIFaceInfo
*/
static void _callback_ExecGetIFaceInfo(WEBS_OUTPUT * pOutput, const char * sParameters, const char * sValue) {
  IP_INFO_INTERFACE InterfaceInfo;
  int NumIFaces;
  int i;

  WEBS_USE_PARA(sParameters);
  WEBS_USE_PARA(sValue);
OS_EnterRegion();
  //
  // Send interface information
  //
  IP_WEBS_SendString(pOutput, "<H2>Interface information:</H2>");
  IP_WEBS_SendString(pOutput, "<PRE>Number of available interfaces: ");
  NumIFaces = IP_INFO_GetNumInterfaces();
  IP_WEBS_SendUnsigned(pOutput, NumIFaces, 10, 0);
  IP_WEBS_SendString(pOutput, "</PRE>");
  //
  // Table with interface information
  //
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tb>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Interface type</td>");
  IP_WEBS_SendString(pOutput, "<th>Interface name</td>");
  IP_WEBS_SendString(pOutput, "<th>Admin state</td>");
  IP_WEBS_SendString(pOutput, "<th>Hardware state</td>");
  IP_WEBS_SendString(pOutput, "<th>Speed</td>");
  IP_WEBS_SendString(pOutput, "<th>Change admin state</td>");
  IP_WEBS_SendString(pOutput, "</tr>");
  for (i = 0; i < NumIFaces; i++) {
    IP_INFO_GetInterfaceInfo (i, &InterfaceInfo);
    IP_WEBS_SendString(pOutput, "<tr>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendString(pOutput, InterfaceInfo.sTypeName);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendString(pOutput, InterfaceInfo.sTypeName);
    IP_WEBS_SendUnsigned(pOutput, InterfaceInfo.TypeIndex, 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    if (InterfaceInfo.AdminState) {
      IP_WEBS_SendString(pOutput, "Up");
    } else {
      IP_WEBS_SendString(pOutput, "Down");
    }
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    if (InterfaceInfo.HWState) {
      IP_WEBS_SendString(pOutput, "Up");
    } else {
      IP_WEBS_SendString(pOutput, "Down");
    }
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, InterfaceInfo.Speed, 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendString(pOutput, "<FORM ACTION=\"Redirect.cgi\" METHOD=\"GET\">");
    IP_WEBS_SendString(pOutput, "<INPUT TYPE=\"submit\" VALUE=\"");
    if (InterfaceInfo.AdminState) {
      IP_WEBS_SendString(pOutput, "Disconnect");
    } else {
      IP_WEBS_SendString(pOutput, "Connect");
    }
    IP_WEBS_SendString(pOutput, "\" NAME=\"");

    if (InterfaceInfo.AdminState) {
      IP_WEBS_SendString(pOutput, "D");
    } else {
      IP_WEBS_SendString(pOutput, "C");
    }
    IP_WEBS_SendUnsigned(pOutput, i, 10, 0);
    IP_WEBS_SendString(pOutput, "\">");
    IP_WEBS_SendString(pOutput, "</FORM>");
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "</tr>");
  }
OS_LeaveRegion();
  IP_WEBS_SendString(pOutput, "</tb>");
  IP_WEBS_SendString(pOutput, "</table>");
}
#endif

/*********************************************************************
*
*       _callback_ExecGetIPAddr
*/
static void _callback_ExecGetIPAddr(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "This sample runs on localhost.</H2>");
#else
  char ac[16];
  U32 IPAddr;

  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);
  IPAddr = IP_GetIPAddr(0);
  IP_PrintIPAddr(ac, IPAddr, sizeof(ac));
  IP_WEBS_SendString(pOutput, ac);
#endif
}

/*********************************************************************
*
*       _callback_ExecPercentage
*/
static void _callback_ExecPercentage(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  WEBS_USE_PARA(sPara);
  if (sValue == NULL) {
    IP_WEBS_SendUnsigned(pOutput, _Percentage, 10, 0);
  } else {
    int v;
    v = _atoi(sValue);
    if (v > 100) {
      v = 100;
    }
    if (v < 0) {
      v = 0;
    }
    _Percentage = v;
  }
}

/*********************************************************************
*
*       _callback_LastName
*/
static void _callback_ExecLastName(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  WEBS_USE_PARA(sPara);
  if (sValue == NULL) {
    IP_WEBS_SendString(pOutput, _acLastName);
  } else {
    _CopyString(_acLastName, sValue, sizeof(_acLastName));
  }
}

/*********************************************************************
*
*       _callback_FirstName
*/
static void _callback_ExecFirstName(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  WEBS_USE_PARA(sPara);
  if (sValue == NULL) {
    IP_WEBS_SendString(pOutput, _acFirstName);
  } else {
    _CopyString(_acFirstName, sValue, sizeof(_acFirstName));
  }
}

/*********************************************************************
*
*       _callback_LEDx
*
*  Function description
*    This function is called to set or get the LED state.
*/
static void _callback_LEDx(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue, unsigned LEDIndex) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  if (sValue == NULL) {
    if (_aLEDState[LEDIndex] == 1) {             // Get LED state
      IP_WEBS_SendString(pOutput, "checked");
    } else {
      IP_WEBS_SendString(pOutput, "");
#ifndef _WIN32
      BSP_ClrLED(LEDIndex);
#endif
    }
  } else {
    //
    // Set new LED state
    //
    if (strcmp(sValue, "on") == 0) {
      _aLEDStateNew[LEDIndex] = 1;
#ifndef _WIN32
      BSP_SetLED(LEDIndex);
#endif
    }
  }
}

/*********************************************************************
*
*       _callback_LED0
*       _callback_LED1
*       _callback_LED2
*       _callback_LED3
*       _callback_LED4
*       _callback_LED5
*       _callback_LED6
*       _callback_LED7
*/
static void _callback_LED0(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 0);
}
static void _callback_LED1(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 1);
}
static void _callback_LED2(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 2);
}
static void _callback_LED3(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 3);
}
static void _callback_LED4(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 4);
}
static void _callback_LED5(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 5);
}
static void _callback_LED6(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 6);
}
static void _callback_LED7(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 7);
}

static void _callback_SetLEDs(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  unsigned i;

  WEBS_USE_PARA(pOutput);
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);
  for (i = 0; i < sizeof(_aLEDState); i++) {
    _aLEDState[i] = _aLEDStateNew[i];
    _aLEDStateNew[i] = 0;
  }
}

/*********************************************************************
*
*       _callback_ExecGetIndex
*
*  Function description:
*    Sends the content of the index page.
*    We use a dynamic web page to generate the content to enhance
*    the portability of the sample.
*/
static void _callback_ExecGetIndex(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue) {
  (void)sPara;
  (void)sValue;
  IP_WEBS_SendString(pOutput, "<div class=\"nav\">");
  IP_WEBS_SendString(pOutput, "<h3>Simple dynamic web pages</h3><ul>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"FormGET.htm\">Form samples (\"GET\")</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"FormPOST.htm\">Form samples (\"POST\")</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"VirtFile.htm\">Virtual file sample</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"/conf/Authen.htm\">Basic authentication sample</a>");
  IP_WEBS_SendString(pOutput, " <span class=\"hint\">(User: user | Pass: pass)</span>");
#if WEBS_SUPPORT_UPLOAD
  IP_WEBS_SendString(pOutput, "<li><a href=\"Upl.htm\">Upload a file</a> <span class=\"hint\">(Real file system required)</span>");
#endif
  IP_WEBS_SendString(pOutput, "</ul>");
  IP_WEBS_SendString(pOutput, "<h3>System info samples using reload</h3><ul>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"IPInf.htm\">Network statistics</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"OSInf.htm\">embOS statistics</a><br></ul>");
  IP_WEBS_SendString(pOutput, "<h3>System info samples using Server Side Events (SSE)</h3>");
  IP_WEBS_SendString(pOutput, "<ul><li><a href=\"SSE_Time.htm\">System time</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"SSE_IP.htm\">Network statistics</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"SSE_OS.htm \">embOS statistics</a><br></ul>");
#if INCLUDE_PRESENTATION_SAMPLE || WEBS_SUPPORT_UPLOAD
  IP_WEBS_SendString(pOutput, "<h3>Generic AJAX samples</h3><ul>");
#if INCLUDE_PRESENTATION_SAMPLE
  IP_WEBS_SendString(pOutput, "<li><a href=\"Products.htm\">SEGGER middleware presentation</a><br>");
#endif
#if WEBS_SUPPORT_UPLOAD
  IP_WEBS_SendString(pOutput, "<li><a href=\"Upl_AJAX.htm\">Upload a file</a> <span class=\"hint\">(Real file system required)</span>");
#endif
  IP_WEBS_SendString(pOutput, "</ul>");
#endif
#if INCLUDE_SHARE_SAMPLE
  IP_WEBS_SendString(pOutput, "<h3>Generic combined SSE/AJAX samples</h3><ul>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"Shares.htm\">Stock quotes sample </a><br></ul>");
#endif
  IP_WEBS_SendString(pOutput, "</div>");
}

/*********************************************************************
*
*       _SendPageHeader
*
*  Function description:
*    Sends the header of the virtual file.
*    The virtual files in our sample application use the same HTML layout.
*    The only difference between the virtual files is the content and that
*    each of them use an own title/heading.
*/
static void _SendPageHeader(WEBS_OUTPUT * pOutput, const char * sName) {
  IP_WEBS_SendString(pOutput, "<!DOCTYPE html><html><head><title>");
  IP_WEBS_SendString(pOutput, sName);
  IP_WEBS_SendString(pOutput, "</title>");
  IP_WEBS_SendString(pOutput, "<link href=\"Styles.css\" rel=\"stylesheet\"></head><body><header>");
  IP_WEBS_SendString(pOutput, sName);
  IP_WEBS_SendString(pOutput, "</header>");
  IP_WEBS_SendString(pOutput, "<div class=\"content\">");
}

/*********************************************************************
*
*       _SendPageFooter
*
*  Function description:
*    Sends the footer of the virtual file.
*    The virtual files in our sample application use the same HTML layout.
*/
static void _SendPageFooter(WEBS_OUTPUT * pOutput) {
  IP_WEBS_SendString(pOutput, "<br><br><br>");
  IP_WEBS_SendString(pOutput, "</div><img src=\"Logo.gif\" alt=\"Segger logo\" class=\"logo\">");
  IP_WEBS_SendString(pOutput, "<footer><p><a href=\"index.htm\">Back to main</a></p>");
  IP_WEBS_SendString(pOutput, "<p>SEGGER Microcontroller GmbH &amp; Co. KG || <a href=\"http://www.segger.com\">www.segger.com</a> ");
  IP_WEBS_SendString(pOutput, "<span class=\"hint\">(external link)</span></p></footer></body></html>");
}

/*********************************************************************
*
*       _callback_CGI_Send
*
*  Function description:
*
*/
static void _callback_CGI_Send(WEBS_OUTPUT * pOutput, const char * sParameters) {
        int    r;
  const char * pFirstName;
        int    FirstNameLen;
  const char * pLastName;
        int    LastNameLen;

  //
  // Header of the page
  //
  _SendPageHeader(pOutput, "Virtual file sample");
  //
  // Content
  //
  r  = IP_WEBS_GetParaValuePtr(sParameters, 0, NULL, 0, &pFirstName, &FirstNameLen);
  r |= IP_WEBS_GetParaValuePtr(sParameters, 1, NULL, 0, &pLastName, &LastNameLen);
  if (r == 0) {
    IP_WEBS_SendString(pOutput, "<h3>First name: ");
    IP_WEBS_SendMem(pOutput, pFirstName, FirstNameLen);
    IP_WEBS_SendString(pOutput, "</h3>");
    IP_WEBS_SendString(pOutput, "<h3>Last name: ");
    IP_WEBS_SendMem(pOutput, pLastName, LastNameLen);
    IP_WEBS_SendString(pOutput, "</h3>");
  } else {
    IP_WEBS_SendString(pOutput, "<br>Error!");
  }
  //
  // Footer of the page
  //
  _SendPageFooter(pOutput);
}

/*********************************************************************
*
*       _callback_CGI_UploadFile
*/
static void _callback_CGI_UploadFile(WEBS_OUTPUT * pOutput, const char * sParameters) {
        int    r;
  const char * pFileName;
        int    FileNameLen;
  const char * pState;       // Can be 0: Upload failed; 1: Upload succeeded; Therefore we do not need to know the length, it will always be 1.

  //
  // Header of the page
  //
  _SendPageHeader(pOutput, "Virtual file sample");
  //
  // Content
  //
  r  = IP_WEBS_GetParaValuePtr(sParameters, 0, NULL, 0, &pFileName, &FileNameLen);
  r |= IP_WEBS_GetParaValuePtr(sParameters, 1, NULL, 0, &pState   , NULL);
  if (r == 0) {
    IP_WEBS_SendString(pOutput, "Upload of \"");
    IP_WEBS_SendMem(pOutput, pFileName, FileNameLen);
    if (*pState == '1') {
      IP_WEBS_SendString(pOutput, "\" successful!<br>");
      IP_WEBS_SendString(pOutput, "<a href=\"");
      IP_WEBS_SendMem(pOutput, pFileName, FileNameLen);
      IP_WEBS_SendString(pOutput, "\">Go to ");
      IP_WEBS_SendMem(pOutput, pFileName, FileNameLen);
      IP_WEBS_SendString(pOutput, "</a><br>");
    } else {
      IP_WEBS_SendString(pOutput, "\" not successful!<br>");
    }
  } else {
    IP_WEBS_SendString(pOutput, "Upload not successful!");
  }
  //
  // Footer of the page
  //
  _SendPageFooter(pOutput);
}

/*********************************************************************
*
*       _SendTime
*
*
*  Function description:
*    Sends the system time to the Web browser.
*
*  Return value:
*    0: Data successfully sent.
*   -1: Data not sent.
*    1: Data successfully sent. Connection should be closed.
*/
static int _SendTime(WEBS_OUTPUT * pOutput) {
  int r;
#ifdef _WIN32
  static U32 Time;
#else
  U32 Time;
#endif

#ifdef _WIN32
#else
  Time = OS_GetTime32();
#endif
  //
  // Send implementation specific header to client
  //
  IP_WEBS_SendString(pOutput, "data: ");
  IP_WEBS_SendString(pOutput, "System time: ");
  IP_WEBS_SendUnsigned(pOutput, Time, 10, 0);
  IP_WEBS_SendString(pOutput, "\r\n");
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
  r = IP_WEBS_Flush(pOutput);
#ifdef _WIN32
  //
  // Simulate the time passed under Windows.
  //
  if (r == 0) {
    Time += 500;
  } else {
    Time += 1000;
  }
#endif
  return r;
}

/*********************************************************************
*
*       _callback_CGI_SSETime
*
*  Function description:
*    Sends the system time to the Web browser every 500 ms.
*/
static void _callback_CGI_SSETime(WEBS_OUTPUT * pOutput, const char * sParameters) {
  int r;

  WEBS_USE_PARA(sParameters);
  //
  // Construct the SSE Header
  //
  IP_WEBS_SendHeaderEx(pOutput, NULL, "text/event-stream", 1);
  IP_WEBS_SendString(pOutput, "retry: 1000\n");  // Normally, the browser attempts to reconnect to the server ~3 seconds after each connection is closed. We change that timeout 1 sec.
  while(1) {
    r = _SendTime(pOutput);
    if (r == 0) {     // Data transmitted, Web browser is still waiting for new data.
#ifdef _WIN32
      Sleep(500);
#else
      OS_Delay(500);
#endif
    } else {          // Even if the data transmission was successful, it could be necessary to close the connection after transmission.
      break;          // This is normally the case if the Web server otherwise could not process new connections.
    }
  }
}

/*********************************************************************
*
*       _SendOSInfo
*/
static int _SendOSInfo(WEBS_OUTPUT * pOutput) {
  int r;

  IP_WEBS_SendString(pOutput, "data: ");        // Start of the SSE data
  _GetOSInfo(pOutput);
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
  r = IP_WEBS_Flush(pOutput);
  return r;
}

/*********************************************************************
*
*       _callback_CGI_SSEembOS
*
*  Function description:
*    Sends some OS informartion to the Web browser every 500 ms.
*
*  Return value:
*    0: Data successfully sent.
*   -1: Data not sent.
*    1: Data successfully sent. Connection should be closed.
*/
static void _callback_CGI_SSEembOS(WEBS_OUTPUT * pOutput, const char * sParameters) {
  int r;

  WEBS_USE_PARA(sParameters);
  //
  // Construct the SSE Header
  //
  IP_WEBS_SendHeaderEx(pOutput, NULL, "text/event-stream", 1);
  IP_WEBS_SendString(pOutput, "retry: 1000\n");  // Normally, the browser attempts to reconnect to the server ~3 seconds after each connection is closed. We change that timeout 1 sec.
  while(1) {
    r = _SendOSInfo(pOutput);
    if (r == 0) {     // Data transmitted, Web browser is still waiting for new data.
#ifdef _WIN32
      Sleep(500);
#else
      OS_Delay(500);
#endif
    } else {          // Even if the data transmission was successful, it could be necessary to close the connection after transmission.
      break;          // This is normally the case, if the Web server otherwise could not process new connections.
    }
  }
}

/*********************************************************************
*
*       _SendNetInfo
*/
static int _SendNetInfo(WEBS_OUTPUT * pOutput) {
  int r;

  IP_WEBS_SendString(pOutput, "data: ");        // Start of the SSE data
  _GetNetworkInfo(pOutput);
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
  r = IP_WEBS_Flush(pOutput);
  return r;
}

/*********************************************************************
*
*       _callback_CGI_SSENet
*
*  Function description:
*    Sends some network informartion to the Web browser every 500 ms.
*
*  Return value:
*    0: Data successfully sent.
*   -1: Data not sent.
*    1: Data successfully sent. Connection should be closed.
*/
static void _callback_CGI_SSENet(WEBS_OUTPUT * pOutput, const char * sParameters) {
  int r;

  WEBS_USE_PARA(sParameters);
  //
  // Construct the SSE Header
  //
  IP_WEBS_SendHeaderEx(pOutput, NULL, "text/event-stream", 1);
  IP_WEBS_SendString(pOutput, "retry: 1000\n");  // Normally, the browser attempts to reconnect to the server ~3 seconds after each connection is closed. We change that timeout 1 sec.
  while(1) {
    r = _SendNetInfo(pOutput);
    if (r == 0) {     // Data transmitted, Web browser is still waiting for new data.
#ifdef _WIN32
      Sleep(500);
#else
      OS_Delay(500);
#endif
    } else {          // Even if the data transmission was successful, it could be necessary to close the connection after transmission.
      break;          // This is normally the case if the Web server otherwise could not process new connections.
    }
  }
}

#if INCLUDE_PRESENTATION_SAMPLE
/*********************************************************************
*
*       _callback_CGI_GetDetails
*
*  Function description:
*    Sends the selected product information to the browser.
*/
static void _callback_CGI_GetDetails(WEBS_OUTPUT * pOutput, const char * sParameters) {
  char acPara[10];

  IP_WEBS_GetParaValue(sParameters, 0, NULL, 0, acPara, sizeof(acPara));
  if (strcmp(acPara, "OS") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>embOS - Real Time Operating System</h2>");
    IP_WEBS_SendString(pOutput, "<b>embOS</b> is a priority-controlled real time operating system, designed to be used as foundation for the development of embedded real-time applications. It is a zero interrupt latency, high-performance RTOS that has been optimized for minimum memory consumption in both RAM and ROM, as well as high speed and versatility.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>Preemptive scheduling: Guarantees that of all tasks in READY state, the one with the highest priority executes, except for situations where priority inversion applies.</li>");
    IP_WEBS_SendString(pOutput, "<li>Round-robin scheduling for tasks with identical priorities.</li>");
    IP_WEBS_SendString(pOutput, "<li>Preemptions can be disabled for entire tasks or for sections of a program.</li>");
    IP_WEBS_SendString(pOutput, "<li>Thread local storage support.</li>");
    IP_WEBS_SendString(pOutput, "<li>Thread safe system library support.</li>");
    IP_WEBS_SendString(pOutput, "<li>No configuration needed</li>");
    IP_WEBS_SendString(pOutput, "<li>Up to 255 priorities: Every task can have an individual priority => the response of tasks can be precisely defined according to the requirements of the application.</li>");
    IP_WEBS_SendString(pOutput, "<li>Unlimited number of tasks (limited only by available memory).</li>");
    IP_WEBS_SendString(pOutput, "<li>Unlimited number of semaphores (limited only by available memory).</li>");
    IP_WEBS_SendString(pOutput, "<li>Unlimited number of mailboxes (limited only by available memory).</li>");
    IP_WEBS_SendString(pOutput, "<li>Size and number of messages can be freely defined.</li>");
    IP_WEBS_SendString(pOutput, "<li>Unlimited number of software timers (limited only by available memory).</li>");
    IP_WEBS_SendString(pOutput, "<li>Time resolution tick can be freely selected (default is 1ms).</li>");
    IP_WEBS_SendString(pOutput, "<li>High resolution time measurement (more accurate than tick).</li>");
    IP_WEBS_SendString(pOutput, "<li>Power management: Unused CPU time can automatically be spent in halt mode, minimizing power consumption.</li>");
    IP_WEBS_SendString(pOutput, "<li>Full interrupt support: Most API functions can be used from within the Interrupt Service Routines (ISRs).</li>");
    IP_WEBS_SendString(pOutput, "<li>Zero interrupt latency time.</li>");
    IP_WEBS_SendString(pOutput, "<li>Nested interrupts are permitted.</li>");
    IP_WEBS_SendString(pOutput, "<li>Start application and projects (BSPs) for an easy start.</li>");
    IP_WEBS_SendString(pOutput, "<li>Debug build performs runtime checks, simplifying development.</li>");
    IP_WEBS_SendString(pOutput, "<li>High precision per task profiling.</li>");
    IP_WEBS_SendString(pOutput, "<li>Real time kernel viewer (embOSView) included.</li>");
    IP_WEBS_SendString(pOutput, "<li>Very fast and efficient, yet small code.</li>");
    IP_WEBS_SendString(pOutput, "<li>Minimum RAM usage.</li>");
    IP_WEBS_SendString(pOutput, "<li>Core written in assembly language.</li>");
    IP_WEBS_SendString(pOutput, "<li>All API functions can be called from C /C++/assembly.</li>");
    IP_WEBS_SendString(pOutput, "<li>Initialization of microcontroller hardware as sources.</li>");
    IP_WEBS_SendString(pOutput, "<li>BSP for any unsupported hardware with the same CPU can easily be written by user.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/embos.html\" target=\"_blank\">http://segger.com/embos.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "IP") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>embOS/IP - TCP/IP Stack</h2>");
    IP_WEBS_SendString(pOutput, "<b>embOS/IP</b> is a CPU independent TCP/IP stack. embOS/IP is a high-performance library that has been optimized for speed, versatility and memory footprint. It is written in ANSI C and can be used on virtually any CPU.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>ANSI C socket.h-like API for user applications. An application using the standard C socket library can easily be ported to use embOS/IP.</li>");
    IP_WEBS_SendString(pOutput, "<li>High performance</li>");
    IP_WEBS_SendString(pOutput, "<li>Small footprint</li>");
    IP_WEBS_SendString(pOutput, "<li>Runs \"out-of-the-box\"</li>");
    IP_WEBS_SendString(pOutput, "<li>No configuration required</li>");
    IP_WEBS_SendString(pOutput, "<li>Works with any RTOS in a multitasking environment (embOS recommended)</li>");
    IP_WEBS_SendString(pOutput, "<li>Zero data copy for ultra fast performance</li>");
    IP_WEBS_SendString(pOutput, "<li>Standard sockets Interface</li>");
    IP_WEBS_SendString(pOutput, "<li>Raw Socket Support</li>");
    IP_WEBS_SendString(pOutput, "<li>Non-blocking versions of all functions</li>");
    IP_WEBS_SendString(pOutput, "<li>Connections limited only by memory availability</li>");
    IP_WEBS_SendString(pOutput, "<li>Re-assembly of fragmented packets</li>");
    IP_WEBS_SendString(pOutput, "<li>Optional drivers for the most common devices are available</li>");
    IP_WEBS_SendString(pOutput, "<li>Fully runtime configurable</li>");
    IP_WEBS_SendString(pOutput, "<li>Developed from ground up for embedded systems</li>");
    IP_WEBS_SendString(pOutput, "<li>PPP/PPPOE available</li>");
    IP_WEBS_SendString(pOutput, "<li>Various upper layer protocols available</li>");
    IP_WEBS_SendString(pOutput, "<li>Drivers for most popular microcontrollers and external MACs available</li>");
    IP_WEBS_SendString(pOutput, "<li>Easy to use!</ul></li>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/embosip.html\" target=\"_blank\">http://segger.com/embosip.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "FS") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emFile - File System</h2>");
    IP_WEBS_SendString(pOutput, "<b>emFile</b> is a file system for embedded applications, which can be used on any media, for which you can provide basic hardware access functions. emFile is a high performance library that has been optimized for minimum memory consumption in RAM and ROM, high speed and versatility. It is written in ANSI C and can be used on any CPU.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>MS DOS/MS Windows-compatible FAT12, FAT16 and FAT32 support.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional module that handles long file names of FAT media.</li>");
    IP_WEBS_SendString(pOutput, "<li>Multiple device driver support. You can use different device drivers with emFile, which allows you to access different types of hardware with the file system at the same time.</li>");
    IP_WEBS_SendString(pOutput, "<li>Multiple media support. A device driver allows you to access different media at the same time.</li>");
    IP_WEBS_SendString(pOutput, "<li>Cache support. Improves the performance of the file system by keeping the last recently used sectors in RAM.</li>");
    IP_WEBS_SendString(pOutput, "<li>Works with any operating system to accomplish a thread-safe environment.</li>");
    IP_WEBS_SendString(pOutput, "<li>ANSI C stdio.h-like API for user applications. An application using the standard C I/O library can easily be ported to use emFile.</li>");
    IP_WEBS_SendString(pOutput, "<li>Very simple device driver structure. emFile device drivers need only basic functions for reading and writing blocks. There is a template included.</li>");
    IP_WEBS_SendString(pOutput, "<li>Optional NOR flash (EEPROM) driver. Any CFI-compliant NOR flash is supported. Wear leveling included.</li>");
    IP_WEBS_SendString(pOutput, "<li>Optional device driver for NAND flash devices. Very high read/write speeds. ECC and wear leveling included.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional device driver for MultiMedia & SD cards using SPI mode or card mode that can be easily integrated.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional IDE driver, which is also suitable for CompactFlash using either True IDE or Memory Mapped mode.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional proprietary file system (EFS) with native long file name support.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional journaling add-on. It protects the integrity of file system against unexpected resets.</li>");
    IP_WEBS_SendString(pOutput, "<li>NAND flash evaluation board available.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emfile.html\" target=\"_blank\">http://segger.com/emfile.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "GUI") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emWin - Graphic Software and GUI</h2>");
    IP_WEBS_SendString(pOutput, "<b>emWin</b> is designed to provide an efficient, processor- and LCD controller-independent graphical user interface (GUI) for any application that operates with a graphical LCD. It is compatible with single-task and multitask environments, with a proprietary operating system or with any commercial RTOS. emWin is shipped as \"C\" source code.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b>");
    IP_WEBS_SendString(pOutput, "<ul>");
    IP_WEBS_SendString(pOutput, "<li>Any 8/16/32-bit CPU; only an ANSI \"C\" compiler is required.</li>");
    IP_WEBS_SendString(pOutput, "<li>Any (monochrome, grayscale or color) LCD with any controller supported (if the right driver is available).</li>");
    IP_WEBS_SendString(pOutput, "<li>May work without LCD controller on smaller displays.</li>");
    IP_WEBS_SendString(pOutput, "<li>PC tool emWinView for a detailed (magnified) view of all layers in the simulation.</li>");
    IP_WEBS_SendString(pOutput, "<li>Any interface supported using configuration macros.</li>");
    IP_WEBS_SendString(pOutput, "<li>Display-size configurable.</li>");
    IP_WEBS_SendString(pOutput, "<li>Characters and bitmaps may be written at any point on the LCD, not just on even-numbered byte addresses.</li>");
    IP_WEBS_SendString(pOutput, "<li>Routines are optimized for both size and speed.</li>");
    IP_WEBS_SendString(pOutput, "<li>Compile time switches allow for different optimizations.</li>");
    IP_WEBS_SendString(pOutput, "<li>For slower LCD controllers, LCD can be cached in memory, reducing access to a minimum and resulting in very high speed.</li>");
    IP_WEBS_SendString(pOutput, "<li>Clear structure.</li>");
    IP_WEBS_SendString(pOutput, "<li>Virtual display support; the virtual display can be larger than the actual display.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emwin.html\" target=\"_blank\">http://segger.com/emwin.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "BTL") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emLoad - Software Updater</h2>");
    IP_WEBS_SendString(pOutput, "<b>emLoad</b> is software for program updates for embedded applications via serial interface from a PC. The software consists of a Windows program and a program for the target application (bootloader) in source code form.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b>");
    IP_WEBS_SendString(pOutput, "<ul><li>Low memory footprint.</li>");
    IP_WEBS_SendString(pOutput, "<li>Straightforward configuration.</li>");
    IP_WEBS_SendString(pOutput, "<li>ANSI-C code is completely portable and runs on any target.</li>");
    IP_WEBS_SendString(pOutput, "<li>100% save & fast: CRC-check implemented.</li>");
    IP_WEBS_SendString(pOutput, "<li>Tools for Windows PC included.</li>");
    IP_WEBS_SendString(pOutput, "<li>Optional support for firmware passwords (emLoad V3).</li>");
    IP_WEBS_SendString(pOutput, "<li>Follows the SEGGER coding standards: Efficient and compact, yet easy to read, understand & debug.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emload.html\" target=\"_blank\">http://segger.com/emload.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "USBD") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emUSB Device - USB Device Stack</h2>");
    IP_WEBS_SendString(pOutput, "<b>emUSB</b> is a high speed USB device stack specifically designed for embedded systems. The software is written in ANSI \"C\" and can run on any platform. emUSB can be used with embOS or any other supported RTOS. A variety of target drivers are already available. Support for new platforms can usually be added for no extra charge.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>High speed</li>");
    IP_WEBS_SendString(pOutput, "<li>Optimized to be used with embOS but works with any other supported RTOS.</li>");
    IP_WEBS_SendString(pOutput, "<li>Easy to use</li>");
    IP_WEBS_SendString(pOutput, "<li>Easy to port</li>");
    IP_WEBS_SendString(pOutput, "<li>No custom USB host driver necessary</li>");
    IP_WEBS_SendString(pOutput, "<li>Start / test application supplied</li>");
    IP_WEBS_SendString(pOutput, "<li>Highly efficient, portable, and commented ANSI C source code</li>");
    IP_WEBS_SendString(pOutput, "<li>Hardware abstraction layer allows rapid addition of support for new devices.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emusb.html\" target=\"_blank\">http://segger.com/emusb.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "USBH") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emUSB Host - USB Host Stack</h2>");
    IP_WEBS_SendString(pOutput, "<b>emUSB</b> Host implements full USB host functionality, including external hub support, and optionally provides device class drivers. It enables developers to easily add USB host functionality to embedded systems.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>Optimized to be used with embOS but works with any other supported RTOS.</li>");
    IP_WEBS_SendString(pOutput, "<li>Highly efficient, portable, and commented ANSI \"C\" source code</li>");
    IP_WEBS_SendString(pOutput, "<li>Hardware abstraction layer allows rapid addition of support for new devices.</li>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emusb-host.html\" target=\"_blank\">http://segger.com/emusb-host.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else {
    IP_WEBS_SendString(pOutput, "<center><b>Please click on one of the pictures on the left to select a product.</b></center>");
  }
}
#endif  // INCLUDE_PRESENTATION_SAMPLE

#if INCLUDE_SHARE_SAMPLE
/*********************************************************************
*
*       _UpdateStockPrices
*
*  Function description:
*    Generates a new stock prices and puts it in the ring buffers.
*/
static void _UpdateStockPrices(void) {
  int WrOff;
  int Size;
  int Pre;
  int Limit;
  int v;
  int i;

#ifndef _WIN32
  srand(OS_GetTime32());
#else
  srand(time(NULL));
#endif
  for (i = 0; i < NUM_STOCKS; i++) {
    //
    // Update the read offset.
    //
    _StockInfo[i].RdOff++;
    if (_StockInfo[i].RdOff == _StockInfo[i].Size) {
      _StockInfo[i].RdOff = 0;
    }
    Size  = _StockInfo[i].Size;
    WrOff = _StockInfo[i].RdOff - 1;
    if (WrOff < 0) {
      WrOff = Size - 1;
    }
    Pre   = WrOff - 1;
    if (Pre < 0) {
      Pre = Size - 1;
    }
    //
    //
    //
    v = rand() % 8 + 1;
    if (v < 4) {
      _StockInfo[i].Price[WrOff] = _StockInfo[i].Price[Pre];
      continue;
    }
    //
    // Generate the stock price variation.
    // Normally; the stock price variation is in a range between -4 and 4.
    // If the stock price is > 80, the range for variation will be enhanced.
    // We need the limitation of the stock prices to verify that the graph looks nice.
    //
    if (_StockInfo[i].Price[Pre] <= 80) {
      Limit = 80;
    } else {
      Limit = (rand() % 900 + 1);
    }
    if (_StockInfo[i].Price[Pre] < Limit) {
      v  = (rand() % 8 + 1);
      v -= 4;
    } else {
      v  = (rand() % 60 + 1);
      if (v > 0) {
        v = -v;
      }
    }
    v += _StockInfo[i].Price[Pre];
    if (v > 0) {
      _StockInfo[i].Price[WrOff] = v;
    } else {
      _StockInfo[i].Price[WrOff] = 0;
    }
  }
}

/*********************************************************************
*
*       _CreateGraphData
*
*  Function description:
*    Initializes the stock price information.
*    Pseudo random values used to simulate the stock prices.
*    The ring buffer used to store the stock prices is always filled
*    completely.
*/
static void _CreateGraphData(void) {
  int i;
  int j;
  int v;
  char c;

#ifndef _WIN32
  srand(OS_GetTime32());
#else
  srand(time(NULL));
#endif
  c = 'A';
  for(i = 0; i < NUM_STOCKS; i++) {
    //
    // Initialize buffer
    //
    _StockInfo[i].Size    = sizeof(_StockInfo[i].Price) / sizeof(int);
    _StockInfo[i].RdOff   = 0; // _StockInfo[i].Size - 1;
    //
    // Add company name
    //
    _StockInfo[i].CompanyName = c;
    c++;
    //
    // Generate initial stock price
    //
    v = rand() % 50 + 21;
    _StockInfo[i].Price[0] = v;
    //
    // Generate the stock prices
    //
    for (j = 1; j < _StockInfo[i].Size; j++) {
      v  = rand() % 8 + 1;
      v -= 4;
      v += _StockInfo[i].Price[j-1];
      _StockInfo[i].Price[j] = v;
    }
  }
}

/*********************************************************************
*
*       _callback_CGI_GetData
*
*  Function description:
*    Generates a comma separated list of values used to draw the
*    graph of stock prices.
*/
static void _callback_CGI_GetData(WEBS_OUTPUT * pOutput, const char * sParameters) {
  char acPara[10];
  char ac[160];
  char * pStr;
  int NumBytes;
  int NumBytesFree;
  int Index;
  int RdOff;
  int Size;
  int i;
  int r;
  int v;

  NumBytesFree = sizeof(ac);
  //
  // Check if we have data, which can be send...
  //
  if (_StockInfo[0].CompanyName == 0) {
    _CreateGraphData();
  }
  pStr = ac;
  r    = IP_WEBS_GetParaValue(sParameters, 0, NULL, 0, acPara, sizeof(acPara));
  //
  // Just for the case that the CGI function has been called without parameter.
  //
  if (r != 0) {
    acPara[0] = '0';
  }
  Index = atoi(acPara);
  RdOff = _StockInfo[Index].RdOff;
  Size  = _StockInfo[Index].Size;
  for(i = 0; i < Size; i++) {
    if (RdOff>= Size) {
      RdOff = 0;
    }
    v             = _StockInfo[Index].Price[RdOff];
    NumBytes      = SEGGER_snprintf(pStr, NumBytesFree, "%d", v);
    pStr         += NumBytes;
    NumBytesFree -= NumBytes;
    *pStr         = ',';
    pStr++;
    RdOff++;
  }
  *(pStr-1) = '\0';
  IP_WEBS_SendString(pOutput, ac);
  _UpdateStockPrices();
}

#ifndef _WIN32
/*********************************************************************
*
*       _itoa
*
*  Function description:
*    Simple itoa implementation.
*    Converts integer into zero-terminated string.
*    Works only with base 10.
*/
static char * _itoa(int Value, char * pStr, int radix) {
  WEBS_USE_PARA(radix);
  SEGGER_snprintf(pStr, 10, "%d", Value);  // Just assume that we have enough space for up to 10 digits in buffer.
  return pStr;
}
#endif

/*********************************************************************
*
*       _fToStr
*
*  Function description:
*    Converts a float to a string with two decimal places.
*/
static void _fToStr(float Value, char * pStr) {
  int v;
  int f;
  int Len;

  //
  // Get int value
  //
  v = (int)Value;
  //
  // Get decimal place
  //
  f = (int)((Value - (float)v) * 100);
  //
  // Convert int to string.
  //
  _itoa(v, pStr, 10);
  Len = strlen(pStr);
  //
  // Add dot
  //
  *(pStr + Len) = '.';
  Len++;
  //
  // Add decimal places
  //
  if (f != 0) {
    _itoa(f, pStr + Len, 10);
    *(pStr + Len + 2) = '\0';
  } else {
    *(pStr + Len) = '0';
    Len++;
    *(pStr + Len) = '0';
    Len++;
    *(pStr + Len) = '\0';
  }
}

/*********************************************************************
*
*       _SendShareTable
*
*  Function description:
*    Generates and sends a table with the stock prices of some
*    fictional companies.
*/
static void _SendShareTable(WEBS_OUTPUT * pOutput) {
  float Percentage;
  char ac[10];
  int Curr;
  int Pre;
  int Trend;
  int Change;
  int i;

  if (_StockInfo[0].CompanyName == 0) {
    _CreateGraphData();
  }
  //
  // Send table with sample shares to the client.
  //
  IP_WEBS_SendString(pOutput, "<h2>Stock prices</h2>");
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Company</th>");
  IP_WEBS_SendString(pOutput, "<th>Current price</th>");
  IP_WEBS_SendString(pOutput, "<th>Trend</th>");
  IP_WEBS_SendString(pOutput, "<th>Change</th>");
  IP_WEBS_SendString(pOutput, "<th>Change %</th>");
  IP_WEBS_SendString(pOutput, "</tr>");
  for (i = 0; i < 26; i++) {
    //
    // Get position of the current stock price and his predecessor in ring buffer
    //
    Curr = _StockInfo[i].RdOff - 1;
    if (Curr < 0) {
      Curr = _StockInfo[i].Size - 1;
    }
    if (Curr != 0) {
      Pre = Curr - 1;
    } else {
      Pre = _StockInfo[i].Size - 1;
    }
    //
    // We need to get the trend of the stock price for the presentation.
    //
    if (_StockInfo[i].Price[Curr] == _StockInfo[i].Price[Pre]) {
      Trend = 0;    // No change...
    } else if (_StockInfo[i].Price[Curr] > _StockInfo[i].Price[Pre]) {
      Trend = 1;    // Up...
    } else {
      Trend = -1;   // Down...
    }
    //
    // Start table row
    //
    IP_WEBS_SendString(pOutput, "<tr ");
    //
    // Change table row background, if the stock price has been changed.
    //
    switch(Trend) {
    case -1: IP_WEBS_SendString(pOutput, "class=\"Down\">"); break; // All visualizations come from the CSS file...
    case  0: IP_WEBS_SendString(pOutput, ">"); break;
    case  1: IP_WEBS_SendString(pOutput, "class=\"Up\">"); break;
    }
    //
    // Send company name
    //
    IP_WEBS_SendString(pOutput, "<td style=\"cursor:pointer;text-align:left;\" onmouseover=\"style.color='blue'\" onmouseout=\"style.color='black'\" onclick=GetDetails(");
    IP_WEBS_SendUnsigned(pOutput, i, 10, 0);
    IP_WEBS_SendString(pOutput, ")>");
    IP_WEBS_SendString(pOutput, "Company ");
    IP_WEBS_SendString(pOutput, &_StockInfo[i].CompanyName);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Send current stock price
    //
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, _StockInfo[i].Price[Curr], 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Visualize the trend with an image.
    //
    switch(Trend) {
    case -1: IP_WEBS_SendString(pOutput, "<td class=\"PointerDown\" Title=\"Trend\">");  break;
    case  0: IP_WEBS_SendString(pOutput, "<td class=\"PointerRight\" Title=\"Trend\">"); break;
    case  1: IP_WEBS_SendString(pOutput, "<td class=\"PointerUp\" Title=\"Trend\">");    break;
    }
    IP_WEBS_SendString(pOutput, "</td><td>");
    //
    // Calculate stock price variation
    //
    if (_StockInfo[i].Price[Curr] > 0) {
      if (_StockInfo[i].Price[Pre] != 0) {
        if (_StockInfo[i].Price[Curr] == _StockInfo[i].Price[Pre]) {  // No change.
          Change     = 0;
          Percentage = 0;
        } else {                                                      // Calculate variation
          Change     = _StockInfo[i].Price[Curr] - _StockInfo[i].Price[Pre];
          Percentage = ((float)Change / ((float)_StockInfo[i].Price[Pre] / 100));
        }
      } else {
        Change     = _StockInfo[i].Price[Curr];
        Percentage = 100;
      }
    }
    if (_StockInfo[i].Price[Curr] == 0) {
      if (_StockInfo[i].Price[Pre] != 0) {
        Change     = _StockInfo[i].Price[Pre] * -1;
        Percentage = -100;
      } else {
        Change     = 0;
        Percentage = 0;
      }
    }
    _itoa(Change, ac, 10);
    IP_WEBS_SendString(pOutput, ac);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Send the change of the stock price in percentage
    //
    IP_WEBS_SendString(pOutput, "<td>");
    if (Percentage < 0) {
      IP_WEBS_SendString(pOutput, "-");
      Percentage *= -1;
    }
    _fToStr(Percentage, ac);
    IP_WEBS_SendString(pOutput, ac);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "</tr>");
  }
  IP_WEBS_SendString(pOutput, "</table>");
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
}

/*********************************************************************
*
*       _SendSSEShareTable
*/
static int _SendSSEShareTable(WEBS_OUTPUT * pOutput) {
  int r;

  IP_WEBS_SendString(pOutput, "data: ");        // Start of the SSE data
  _SendShareTable(pOutput);
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
  r = IP_WEBS_Flush(pOutput);
  return r;
}

/*********************************************************************
*
*       _callback_CGI_SSEGetShareTable
*
*  Function description:
*    Sends the stock price table
*
*  Return value:
*    0: Data successfully sent.
*   -1: Data not sent.
*    1: Data successfully sent. Connection should be closed.
*/
static void _callback_CGI_SSEGetShareTable(WEBS_OUTPUT * pOutput, const char * sParameters) {
  int r;

  WEBS_USE_PARA(sParameters);
  //
  // Construct the SSE Header
  //
  IP_WEBS_SendHeaderEx(pOutput, NULL, "text/event-stream", 1);
  IP_WEBS_SendString(pOutput, "retry: 1000\n");  // Normally, the browser attempts to reconnect to the server ~3 seconds after each connection is closed. We change that timeout 1 sec.
  while(1) {
    r = _SendSSEShareTable((void*)pOutput);
    if (r == 0) {     // Data transmitted, Web browser is still waiting for new data.
#ifdef _WIN32
      Sleep(500);
#else
      OS_Delay(500);
#endif
    } else {          // Even if the data transmission was successful, it could be necessary to close the connection after transmission.
      break;          // This is normally the case if the Web server otherwise could not process new connections.
    }
  }
}
#endif // INCLUDE_SHARE_SAMPLE

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       aAccessControlDemo
*/
WEBS_ACCESS_CONTROL aAccessControlDemo[] = {
  { "/conf/", "Login for configuration", "user:pass" },
  { "/"     , NULL                     , NULL        },
  { NULL    , NULL                     , NULL        }
};

/*********************************************************************
*
*       WEBS_VFILES
*
*  Function description
*    Defines all virtual files used by the web server
*/
const WEBS_VFILES aVFilesDemo[]  = {
#if INCLUDE_SHARE_SAMPLE
  {"GetData.cgi",        _callback_CGI_GetData           },   // Called from Shares.htm
  {"GetShareTable.cgi",  _callback_CGI_SSEGetShareTable  },   // Called from Shares.htm
#endif
#if INCLUDE_PRESENTATION_SAMPLE
  {"GetDetails.cgi",     _callback_CGI_GetDetails        },   // Called from Presentation.htm
#endif
  {"Send.cgi",           _callback_CGI_Send              },   // Called from VirtFile.htm
  {"SSETime.cgi" ,       _callback_CGI_SSETime           },   // Called from SSETime.htm
  {"SSEembOS.cgi",       _callback_CGI_SSEembOS          },   // Called from SSEembOS.htm
  {"SSENet.cgi",         _callback_CGI_SSENet            },   // Called from embOSIP.htm
  {"Upload.cgi",         _callback_CGI_UploadFile        },   // Called from Upload.htm
  { NULL, NULL }
};

/*********************************************************************
*
*       CGI table
*
*  Function description
*    CGI table, defining callback routines for dynamic content (SSI)
*/
const WEBS_CGI aCGIDemo[] = {
  {"Counter"   ,    _callback_ExecCounter           },   // Called from index.htm
  {"GetIndex",      _callback_ExecGetIndex          },   // Called from index.htm
  {"GetIPAddr" ,    _callback_ExecGetIPAddr         },
  {"GetOSInfo",     _callback_ExecGetOSInfo         },
  {"GetIPInfo",     _callback_ExecGetConnectionInfo },
//  {"GetIFaceInfo",  _callback_ExecGetIFaceInfo      },
  {"FirstName",     _callback_ExecFirstName         },
  {"LastName",      _callback_ExecLastName          },
  {"Percentage",    _callback_ExecPercentage        },
  {"LED0",          _callback_LED0                  },
  {"LED1",          _callback_LED1                  },
  {"LED2",          _callback_LED2                  },
  {"LED3",          _callback_LED3                  },
  {"LED4",          _callback_LED4                  },
  {"LED5",          _callback_LED5                  },
  {"LED6",          _callback_LED6                  },
  {"LED7",          _callback_LED7                  },
  {"SetLEDs",       _callback_SetLEDs               },
  {NULL,            _callback_DefaultHandler        }
};

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
  &aCGIDemo[0],
  &aAccessControlDemo[0],
  _CB_HandleParameter,
  &aVFilesDemo[0]
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
*       WebserverTask()
*
* Function description
*   Web server task started by the SEGGERDEMO.
*/
void WebserverTask(void);
void WebserverTask(void) {
  OS_SetPriority(OS_GetTaskID(), TASKPRIO_WEBSPARENT);
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
