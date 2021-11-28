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
File        : MB_Int.h
Purpose     : Internals used accross different layers of the Modbus stack.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _MB_INT_H_     // Avoid multiple/recursive inclusion.
#define _MB_INT_H_  1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "MB.h"
#include "MB_ConfDefaults.h"

#if defined(__cplusplus)
extern "C" {  /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#ifdef MB_CORE_C
  #define EXTERN
#else
  #define EXTERN  extern
#endif

#if MB_SUPPORT_LOG
  #define MB_LOG(p)  MB_Logf p
#else
  #define MB_LOG(p)
#endif

#if MB_SUPPORT_WARN
  #define MB_WARN(p) MB_Warnf p
#else
  #define MB_WARN(p)
#endif

#define MB_COIL_ON        0xFF

#define MB_MASTER         0
#define MB_SLAVE          1
#define MB_NUM_ENDPOINTS  2

/*********************************************************************
*
*       MB_GLOBAL_
*
**********************************************************************
*/

typedef void (*MB_ENDPOINT_pfSignal)(MB_CHANNEL *pChannel);

typedef struct {
  MB_CHANNEL           *pFirstChannel;
  MB_ENDPOINT_pfSignal  pfSignal;
} MB_ENDPOINT;

typedef struct {
  MB_ENDPOINT *pEndpoint[MB_NUM_ENDPOINTS];  // We have two endpoints. Master and slave.
  U32          TimerFreq;                    // RTU timeout counter frequency used for all channels.
  U8           NumRTUTimerInits;             // RTU timer has to be initialized once. However we count the number of inits for de-initialization.
} MB_GLOBAL;

EXTERN MB_GLOBAL MB_Global;

/*********************************************************************
*
*       Types/structures
*
**********************************************************************
*/

typedef struct {
  U16 xID;
  U16 ProtID;
  U16 Len;
} MB_IP_HEADER;

typedef struct {
  U8  SlaveAddr;
  U8  Function;
  U16 Addr;
  U16 NumItems;
} MB_REQUEST_READ_HEADER;

typedef struct {
  U8 SlaveAddr;
  U8 Function;
  U8 ByteCnt;
  U8 Data;
} MB_RESPONSE_READ_HEADER;

typedef struct {
  U8  SlaveAddr;
  U8  Function;
  U16 Addr;
  U16 Data;
} MB_REQUEST_WRITE_SINGLE_HEADER;

typedef struct {
  U8  SlaveAddr;
  U8  Function;
  U16 Addr;
  U16 NumItems;
  U8  NumBytes;
  U8  Data;
} MB_REQUEST_WRITE_MULTI_HEADER;

typedef int (*MB_SLAVE_API_pfReadCoil)(U16 Addr);
typedef int (*MB_SLAVE_API_pfReadReg) (U16 Addr, U16 *pVal);

/*********************************************************************
*
*       Commands
*
**********************************************************************
*/

#define MB_CMD_READ_COILS   1
#define MB_CMD_READ_DI      2
#define MB_CMD_READ_HR      3
#define MB_CMD_READ_IR      4
#define MB_CMD_WRITE_COIL   5
#define MB_CMD_WRITE_REG    6
#define MB_CMD_WRITE_COILS  15
#define MB_CMD_WRITE_REGS   16

/*********************************************************************
*
*       Protocol function table
*
* Implementation for handling different protocol data conversion for:
*   - Modbus RTU or ASCII (via serial connection)
*   - Modbus/TCP or Modbus/UDP
*   - Modbus RTU or ASCII over TCP/UDP
*
**********************************************************************
*/

typedef struct {
  int  (*pfIsValid)    (MB_CHANNEL *pChannel);                                                       // Verifies data in packet being valid or not.
  int  (*pfOnRx)       (MB_CHANNEL *pChannel, U8 Data);                                              // Stores data into Rx buffer in internal RTU format.
  int  (*pfOnTx)       (MB_CHANNEL *pChannel);                                                       // Sends the next byte.
  int  (*pfRead)       (MB_CHANNEL *pChannel, MB_IFACE_CONFIG *pConfig, U32 Timeout);                // Reads data from interface and stores it in Rx buffer in internal RTU format.
  int  (*pfWrite)      (MB_CHANNEL *pChannel, MB_IFACE_CONFIG *pConfig, U16 NumBytes, U8 Endpoint);  // Starts writing data stored in channel buffer in RTU format into to interface.
  void (*pfInitTimer)  (const MB_IFACE_UART_API *pIFaceAPI);                                         // Initialize timer if necessary for interface. Typically only used for RTU.
  void (*pfDeInitTimer)(const MB_IFACE_UART_API *pIFaceAPI);                                         // De-Initialize timer if necessary for interface. Typically only used for RTU.
  void (*pfResetBuffer)(MB_CHANNEL *pChannel, char IsError);                                         // Resets the channel buffer.
  U16 MaxDataLen;                                                                                    // Maximum data length without protocol overhead like Addr. (1), Function code (1) and others.
} MB_IFACE_PROT_API;

extern const MB_IFACE_PROT_API MB_IFACE_PROT_Ascii;
extern const MB_IFACE_PROT_API MB_IFACE_PROT_Ip;
extern const MB_IFACE_PROT_API MB_IFACE_PROT_Rtu;

/*********************************************************************
*
*       API functions (internal)
*
**********************************************************************
*/

void MB_Connect         (MB_CHANNEL *pChannel);
void MB_Disconnect      (MB_CHANNEL *pChannel);
void MB_ConfigRTUTimeout(MB_IFACE_CONFIG_UART *pConfig, U32 Baudrate, U8 DataBits, U8 StopBits);
void MB_Send            (MB_CHANNEL *pChannel, const U8 *pData, U32 NumBytes);

/*********************************************************************
*
*       MB_CHANNEL_
*
**********************************************************************
*/

void MB_CHANNEL_AddIP  (MB_CHANNEL *pChannel, MB_IFACE_CONFIG_IP   *pConfig, const MB_SLAVE_API *pSlaveAPI, const MB_IFACE_IP_API   *pIFaceAPI, U32 Timeout, U8 SlaveAddr, U8 DisableWrite, U32 IPAddr, U16 Port, U8 Endpoint);
void MB_CHANNEL_AddUART(MB_CHANNEL *pChannel, MB_IFACE_CONFIG_UART *pConfig, const MB_SLAVE_API *pSlaveAPI, const MB_IFACE_UART_API *pIFaceAPI, const MB_IFACE_PROT_API *pIFaceProtAPI, U32 Timeout, U8 SlaveAddr, U8 DisableWrite, U32 Baudrate, U8 DataBits, U8 Parity, U8 StopBits, U8 Port, U8 Endpoint);
void MB_CHANNEL_Signal (MB_CHANNEL *pChannel);

/*********************************************************************
*
*       Logging functions
*
**********************************************************************
*/

void MB_Logf      (const char *sFormat, ...);
void MB_Warnf     (const char *sFormat, ...);
void MB_PrintfSafe(char *pBuffer, const char *sFormat, int BufferSize, va_list *pParamList);


#if defined(__cplusplus)
  }     // Make sure we have C-declarations in C++ programs
#endif

#endif  // Avoid multiple/recursive inclusion

/****** End Of File *************************************************/
