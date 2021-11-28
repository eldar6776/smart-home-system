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
File    : Webserver_DynContent.c
Purpose : Dynamic content the embOS/IP Web server.
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

/*********************************************************************
*
*       typedefs
*
**********************************************************************
*/

typedef struct STOCK_INFO {
  int  Price[30];
  int  Size;
  int  RdOff;
  char CompanyName;
} STOCK_INFO;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

//
// Variables used for CGI samples
//
static U32        _Cnt;
static U32        _Percentage = 50;
static char       _aLEDState[8];
static char       _aLEDStateNew[8];
static char       _acFirstName[12];
static char       _acLastName[12];
#if INCLUDE_SHARE_SAMPLE
static STOCK_INFO _StockInfo[26];
#endif
#if INCLUDE_IP_CONFIG_SAMPLE
static char       _DHCPOnOff;
static U32        _IPAddr;
static U32        _IPMask;
static U32        _IPGW;
#endif

/******************************************************************************************************************************************
*
*       CGI sample functions
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
static int _atoi(const char* sValue) {
  int  Value;
  int  Digit;
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
static void _CopyString(char* sDest, const char* sSrc, int DestSize) {
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
static void _callback_DefaultHandler(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sValue);

  IP_WEBS_SendString(pOutput, "<h1 style=\"color:red\">Unknown CGI ");
  IP_WEBS_SendString(pOutput, sPara);
  IP_WEBS_SendString(pOutput, " - Ignored</h1>");
}

/*********************************************************************
*
*       _callback_ExecCounter
*/
static void _callback_ExecCounter(WEBS_OUTPUT* pOutput, const char* sParameters, const char* sValue ) {
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
static void _GetNetworkInfo(WEBS_OUTPUT* pOutput) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<H2>This sample runs on a Windows host. No embOS/IP statistics available.</H2>");
#else
  IP_CONNECTION_HANDLE hConnection[MAX_CONNECTION_INFO];
  IP_CONNECTION_HANDLE hTemp;
  U16                  sConnection[MAX_CONNECTION_INFO];
  U16                  sTemp;
  IP_CONNECTION Info;
  int  NumConnections;
  int  NumValidConnections;
  int  i;
  int  j;
  int  minIndex;
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
    sConnection[i] = Info.hSock;
    if (r != 0) {
      NumValidConnections--;
    }
  }
  IP_WEBS_SendUnsigned(pOutput, NumValidConnections, 10, 0);
  //
  // Sort by hSock
  //
  for (i = 0; i < NumConnections; i++) {
    minIndex = i;
    for(j = i + 1; j < NumConnections; j++) {
      if(sConnection[j] < sConnection[minIndex]) {
        minIndex = j;
      }
    }
    if(i != minIndex) {
      sTemp = sConnection[i];
      hTemp = hConnection[i];
      sConnection[i] = sConnection[minIndex];
      hConnection[i] = hConnection[minIndex];
      sConnection[minIndex] = sTemp;
      hConnection[minIndex] = hTemp;
    }
  }
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
static void _callback_ExecGetConnectionInfo(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
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
static void _GetOSInfo(WEBS_OUTPUT* pOutput) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<H2>This sample runs on a Windows host. No embOS statistics available.</H2>");
#else
  OS_TASK* pTask;
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
static void _callback_ExecGetOSInfo(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  _GetOSInfo(pOutput);
}

#if 0
/*********************************************************************
*
*       _callback_ExecGetIFaceInfo
*/
static void _callback_ExecGetIFaceInfo(WEBS_OUTPUT* pOutput, const char* sParameters, const char* sValue) {
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
static void _callback_ExecGetIPAddr(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "This sample runs on localhost.</H2>");
#else
  char ac[16];
  U32  IPAddr;

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
static void _callback_ExecPercentage(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
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
static void _callback_ExecLastName(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
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
static void _callback_ExecFirstName(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
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
static void _callback_LEDx(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue, unsigned LEDIndex) {
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
static void _callback_LED0(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 0);
}
static void _callback_LED1(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 1);
}
static void _callback_LED2(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 2);
}
static void _callback_LED3(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 3);
}
static void _callback_LED4(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 4);
}
static void _callback_LED5(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 5);
}
static void _callback_LED6(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 6);
}
static void _callback_LED7(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 7);
}

static void _callback_SetLEDs(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
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
static void _callback_ExecGetIndex(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

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
#if INCLUDE_IP_CONFIG_SAMPLE
  IP_WEBS_SendString(pOutput, "<li><a href=\"IPConf.htm\">IP Configuration</a><br>");
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
static void _SendPageHeader(WEBS_OUTPUT* pOutput, const char* sName) {
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
static void _SendPageFooter(WEBS_OUTPUT* pOutput) {
  IP_WEBS_SendString(pOutput, "<br><br><br>");
  IP_WEBS_SendString(pOutput, "</div><img src=\"Logo.gif\" alt=\"Segger logo\" class=\"logo\">");
  IP_WEBS_SendString(pOutput, "<footer><p><a href=\"index.htm\">Back to main</a></p>");
  IP_WEBS_SendString(pOutput, "<p>SEGGER Microcontroller GmbH &amp; Co. KG || <a href=\"http://www.segger.com\">www.segger.com</a> ");
  IP_WEBS_SendString(pOutput, "<span class=\"hint\">(external link)</span></p></footer></body></html>");
}

/*********************************************************************
*
*       _callback_CGI_Send
*/
static void _callback_CGI_Send(WEBS_OUTPUT* pOutput, const char* sParameters) {
        int   r;
  const char* pFirstName;
        int   FirstNameLen;
  const char* pLastName;
        int   LastNameLen;

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
static void _callback_CGI_UploadFile(WEBS_OUTPUT* pOutput, const char* sParameters) {
        int   r;
  const char* pFileName;
        int   FileNameLen;
  const char* pState;       // Can be 0: Upload failed; 1: Upload succeeded; Therefore we do not need to know the length, it will always be 1.

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
static int _SendTime(WEBS_OUTPUT* pOutput) {
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
static void _callback_CGI_SSETime(WEBS_OUTPUT* pOutput, const char* sParameters) {
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
static int _SendOSInfo(WEBS_OUTPUT* pOutput) {
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
static void _callback_CGI_SSEembOS(WEBS_OUTPUT* pOutput, const char* sParameters) {
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
static int _SendNetInfo(WEBS_OUTPUT* pOutput) {
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
static void _callback_CGI_SSENet(WEBS_OUTPUT* pOutput, const char* sParameters) {
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
static void _callback_CGI_GetDetails(WEBS_OUTPUT* pOutput, const char* sParameters) {
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
  srand((unsigned int)time(NULL));
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
  int  i;
  int  j;
  int  v;
  char c;

#ifndef _WIN32
  srand(OS_GetTime32());
#else
  srand((unsigned int)time(NULL));
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
static void _callback_CGI_GetData(WEBS_OUTPUT* pOutput, const char* sParameters) {
  char  acPara[10];
  char  ac[160];
  char* pStr;
  int   NumBytes;
  int   NumBytesFree;
  int   Index;
  int   RdOff;
  int   Size;
  int   i;
  int   r;
  int   v;

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
static char* _itoa(int Value, char* pStr, int radix) {
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
static void _fToStr(float Value, char* pStr) {
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
static void _SendShareTable(WEBS_OUTPUT* pOutput) {
  float Percentage;
  int   Curr;
  int   Pre;
  int   Trend;
  int   Change;
  int   i;
  char  ac[10];

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
static int _SendSSEShareTable(WEBS_OUTPUT* pOutput) {
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
static void _callback_CGI_SSEGetShareTable(WEBS_OUTPUT* pOutput, const char* sParameters) {
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

#if INCLUDE_IP_CONFIG_SAMPLE
/*********************************************************************
*
*       _callback_DHCP_On
*
*  Function description
*    This function is called to set or get the DHCP usage radio button.
*/
static void _callback_DHCP_On(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  if (sValue == NULL) {
    _DHCPOnOff = IP_DHCPC_GetState(0);
    if (_DHCPOnOff != 0) {
      IP_WEBS_SendString(pOutput, "checked");
    } else {
      IP_WEBS_SendString(pOutput, "");
    }
  } else {
    _DHCPOnOff = _atoi(sValue);
  }
}

/*********************************************************************
*
*       _callback_DHCP_Off
*
*  Function description
*    This function is called to set or get the DHCP usage radio button.
*/
static void _callback_DHCP_Off(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  if (sValue == NULL) {
    _DHCPOnOff = IP_DHCPC_GetState(0);
    if (_DHCPOnOff == 0) {
      IP_WEBS_SendString(pOutput, "checked");
    } else {
      IP_WEBS_SendString(pOutput, "");
    }
  } else {
    _DHCPOnOff = _atoi(sValue);
  }
}

/*********************************************************************
*
*       _callback_IPAddrX
*/
static void _callback_IPAddrX(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue, unsigned Pos) {
  U32 Tmp;

  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    _IPAddr = IP_GetIPAddr(0);
    Tmp     = (_IPAddr & (0xFF000000 >> (8 * Pos))) >> (24 - 8 * Pos);
    IP_WEBS_SendUnsigned(pOutput, Tmp, 10, 0);
  } else {
    int v;
    v = _atoi(sValue);
    if (v > 255) {
      v = 255;
    }
    if (v < 0) {
      v = 0;
    }
    _IPAddr  &= ~(0xFF000000 >> (8 * Pos));
    _IPAddr  |= v << (24 - 8 * Pos);
  }
}

/*********************************************************************
*
*       _callback_IPMaskX
*/
static void _callback_IPMaskX(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue, unsigned Pos) {
  U32 Tmp;

  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    IP_GetAddrMask(0, &Tmp, &_IPMask);
    Tmp = (_IPMask & (0xFF000000 >> (8 * Pos))) >> (24 - 8 * Pos);
    IP_WEBS_SendUnsigned(pOutput, Tmp, 10, 0);
  } else {
    int v;
    v = _atoi(sValue);
    if (v > 255) {
      v = 255;
    }
    if (v < 0) {
      v = 0;
    }
    _IPMask  &= ~(0xFF000000 >> (8 * Pos));  // Clear the bits before changing the value
    _IPMask  |= v << (24 - 8 * Pos);
  }
}

/*********************************************************************
*
*       _callback_IPGWX
*/
static void _callback_IPGWX(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue, unsigned Pos) {
  U32 Tmp;

  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    _IPGW = IP_GetGWAddr(0);
    Tmp   = (_IPGW & (0xFF000000 >> (8 * Pos))) >> (24 - 8 * Pos);
    IP_WEBS_SendUnsigned(pOutput, Tmp, 10, 0);
  } else {
    int v;
    v = _atoi(sValue);
    if (v > 255) {
      v = 255;
    }
    if (v < 0) {
      v = 0;
    }
    _IPGW  &= ~(0xFF000000 >> (8 * Pos));  // Clear the bits before changing the value
    _IPGW  |= v << (24 - 8 * Pos);
  }
}

/*********************************************************************
*
*       _callback_IPAddrX
*       _callback_IPMaskX
*       _callback_IPGWX
*/
static void _callback_IPAddr0(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPAddrX(pOutput, sPara, sValue, 0);
}
static void _callback_IPAddr1(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPAddrX(pOutput, sPara, sValue, 1);
}
static void _callback_IPAddr2(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPAddrX(pOutput, sPara, sValue, 2);
}
static void _callback_IPAddr3(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPAddrX(pOutput, sPara, sValue, 3);
}
static void _callback_IPMask0(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPMaskX(pOutput, sPara, sValue, 0);
}
static void _callback_IPMask1(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPMaskX(pOutput, sPara, sValue, 1);
}
static void _callback_IPMask2(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPMaskX(pOutput, sPara, sValue, 2);
}
static void _callback_IPMask3(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPMaskX(pOutput, sPara, sValue, 3);
}
static void _callback_IPGW0(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPGWX(pOutput, sPara, sValue, 0);
}
static void _callback_IPGW1(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPGWX(pOutput, sPara, sValue, 1);
}
static void _callback_IPGW2(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPGWX(pOutput, sPara, sValue, 2);
}
static void _callback_IPGW3(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPGWX(pOutput, sPara, sValue, 3);
}

static void _callback_SetIP(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {

  WEBS_USE_PARA(pOutput);
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  if (_DHCPOnOff != 0) {
    //
    // Check if DHCP is already activated.
    //
    if (IP_DHCPC_GetState(0) == 0) {
      IP_DHCPC_Activate(0, "Target", NULL, NULL);
    }
  } else {
    //
    // Check if DHCP is already activated.
    //
    if (IP_DHCPC_GetState(0)) {
      IP_DHCPC_Halt(0);
    }
    //
    IP_SetGWAddr(0, _IPGW);
    IP_SetAddrMask(_IPAddr, _IPMask);
  }
}

#endif // INCLUDE_IP_CONFIG_SAMPLE

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       aAccessControl
*/
WEBS_ACCESS_CONTROL aAccessControl[] = {
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
const WEBS_VFILES aVFiles[]  = {
#if INCLUDE_SHARE_SAMPLE
  {"GetData.cgi",        _callback_CGI_GetData           },   // Called from Shares.htm
  {"GetShareTable.cgi",  _callback_CGI_SSEGetShareTable  },   // Called from Shares.htm
#endif
#if INCLUDE_PRESENTATION_SAMPLE
  {"GetDetails.cgi",     _callback_CGI_GetDetails        },   // Called from Presentation.htm
#endif
  {"Send.cgi",           _callback_CGI_Send              },   // Called from VirtFile.htm
  {"SSETime.cgi" ,       _callback_CGI_SSETime           },   // Called from SSE_Time.htm
  {"SSEembOS.cgi",       _callback_CGI_SSEembOS          },   // Called from SSE_OS.htm
  {"SSENet.cgi",         _callback_CGI_SSENet            },   // Called from SSE_IP.htm
  {"Upload.cgi",         _callback_CGI_UploadFile        },   // Called from Upl.htm and Upl_AJAX.htm
  { NULL, NULL }
};

/*********************************************************************
*
*       CGI table
*
*  Function description
*    CGI table, defining callback routines for dynamic content (SSI)
*/
const WEBS_CGI aCGI[] = {
  {"Counter"            , _callback_ExecCounter           },  // Called from index.htm
  {"GetIndex"           , _callback_ExecGetIndex          },  // Called from index.htm
  {"GetIPAddr"          , _callback_ExecGetIPAddr         },
  {"GetOSInfo"          , _callback_ExecGetOSInfo         },
  {"GetIPInfo"          , _callback_ExecGetConnectionInfo },
//  {"GetIFaceInfos"      , _callback_ExecGetIFaceInfo      },
  {"FirstName"          , _callback_ExecFirstName         },
  {"LastName"           , _callback_ExecLastName          },
  {"Percentage"         , _callback_ExecPercentage        },
  {"LED0"               , _callback_LED0                  },
  {"LED1"               , _callback_LED1                  },
  {"LED2"               , _callback_LED2                  },
  {"LED3"               , _callback_LED3                  },
  {"LED4"               , _callback_LED4                  },
  {"LED5"               , _callback_LED5                  },
  {"LED6"               , _callback_LED6                  },
  {"LED7"               , _callback_LED7                  },
  {"SetLEDs"            , _callback_SetLEDs               },
#if INCLUDE_IP_CONFIG_SAMPLE
  {"AssignIPType_Auto"  , _callback_DHCP_On               },
  {"AssignIPType_Manual", _callback_DHCP_Off              },
  {"IPAddr_0"           , _callback_IPAddr0               },
  {"IPAddr_1"           , _callback_IPAddr1               },
  {"IPAddr_2"           , _callback_IPAddr2               },
  {"IPAddr_3"           , _callback_IPAddr3               },
  {"IPMask_0"           , _callback_IPMask0               },
  {"IPMask_1"           , _callback_IPMask1               },
  {"IPMask_2"           , _callback_IPMask2               },
  {"IPMask_3"           , _callback_IPMask3               },
  {"IPGateway_0"        , _callback_IPGW0                 },
  {"IPGateway_1"        , _callback_IPGW1                 },
  {"IPGateway_2"        , _callback_IPGW2                 },
  {"IPGateway_3"        , _callback_IPGW3                 },
  {"SetIP"              , _callback_SetIP                 },
#endif // INCLUDE_IP_CONFIG_SAMPLE
  {NULL                 , _callback_DefaultHandler        }
};

/*************************** End of file ****************************/
