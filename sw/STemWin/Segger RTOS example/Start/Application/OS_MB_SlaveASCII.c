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
File    : OS_MB_SlaveASCII.c
Purpose : Sample program for Modbus slave using embOS.
          Demonstrates how to implement a Modbus slave using the
          Modbus ASCII protocol over a serial connection.
          The sample setups a Modbus ASCII slave via a serial
          connection and provides access to some LEDs and other
          resources.
          The slave provides the following resources:
          - LEDs on coils BASE_ADDR & BASE_ADDR + 1.
          - Two Discrete Inputs on BASE_ADDR & BASE_ADDR + 1 that
            are toggled separately on each access.
          - Two 16-bit registers on BASE_ADDR & BASE_ADDR + 1.
          - Two 16-bit Input Registers on BASE_ADDR & BASE_ADDR + 1
            that count up on each access.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include "BSP.h"
#include "MB.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#define UART_PORT           0
#define UART_BAUDRATE       38400
#define UART_NUM_DATA_BITS  8
#define UART_PARITY         UART_PARITY_NONE
#define UART_NUM_STOP_BITS  1
#define SLAVE_ADDR          1
#define BASE_ADDR           1000

//
// Task priorities
//
enum {
  TASK_PRIO_MB_SLAVE_TASK = 150  // Priority must be higher as all Modbus related tasks.
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int _MBSlaveStack[1024/sizeof(int)];  // Define the stack of the MB_SLAVE_Task to 1024 bytes.
static OS_TASK         _MBSlaveTCB;

//
// Modbus configuration.
//
static MB_CHANNEL            _MBChannel;
static MB_IFACE_CONFIG_UART  _MBConfig;

static char                  _MBCoil[2];    // Simulate module with 2 coils.
static char                  _MBToggle[2];  // Simulate module with 2 Discrete Input bits, each toggling with each access.
static U16                   _MBReg[2];     // Simulate module with 2 registers.
static U16                   _MBCnt[2];     // Simulate module with 2 Input Registers used as counter, each counting up with each access.

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

static void _SendByte (MB_IFACE_CONFIG_UART *pConfig, U8 Data);
static int  _Init     (MB_IFACE_CONFIG_UART *pConfig);
static void _DeInit   (MB_IFACE_CONFIG_UART *pConfig);

static int  _WriteCoil(U16 Addr, char OnOff);
static int  _ReadCoil (U16 Addr);
static int  _ReadDI   (U16 Addr);
static int  _WriteReg (U16 Addr, U16   Val);
static int  _ReadHR   (U16 Addr, U16 *pVal);
static int  _ReadIR   (U16 Addr, U16 *pVal);

/*********************************************************************
*
*       Static const, Modbus API
*
**********************************************************************
*/

static const MB_IFACE_UART_API _IFaceAPI = {
  _SendByte,  // pfSendByte
  _Init,      // pfInit
  _DeInit,    // pfDeInit
  NULL,       // pfSend
  NULL,       // pfRecv
  NULL,       // pfConnect
  NULL,       // pfDisconnect
  NULL,       // pfInitTimer
  NULL        // pfDeInitTimer
};

static const MB_SLAVE_API _SlaveAPI = {
  _WriteCoil,  // pfWriteCoil
  _ReadCoil,   // pfReadCoil
  _ReadDI,     // pfReadDI
  _WriteReg,   // pfWriteReg
  _ReadHR,     // pfReadHR
  _ReadIR      // pfReadIR
};

/*********************************************************************
*
*       Local functions, Modbus network interface
*
**********************************************************************
*/

/*********************************************************************
*
*       _SendByte()
*
* Function description
*   Sends one byte via UART.
*
* Parameters
*   pConfig: Pointer to configuration.
*   Data   : Byte to send via UART.
*/
static void _SendByte(MB_IFACE_CONFIG_UART *pConfig, U8 Data) {
  BSP_UART_Write1(pConfig->Port, Data);
}

/*********************************************************************
*
*       _OnRx()
*
* Function description
*   Wrapper function to link BSP_UART and Modbus API.
*
* Parameters
*   Unit: Zero-based interface index.
*   Data: Byte to send via UART.
*/
static void _OnRx(unsigned Unit, U8 Data) {
  BSP_USE_PARA(Unit);

  MB_OnRx(&_MBChannel, Data);
}

/*********************************************************************
*
*       _OnTx()
*
* Function description
*   Wrapper function to link BSP_UART and Modbus API.
*
* Parameters
*   Unit: Zero-based interface index.
*   Data: Byte to send via UART.
*
* Return value
*   More data sent      : 0.
*   No more data to send: 1.
*/
static int _OnTx(unsigned Unit) {
  BSP_USE_PARA(Unit);

  return MB_OnTx(&_MBChannel);
}

/*********************************************************************
*
*       _Init()
*
* Function description
*   Initialize UART communication.
*
* Parameters
*   pConfig: Pointer to configuration.
*
* Return value
*   O.K. : 0
*   Error: Other
*/
static int _Init(MB_IFACE_CONFIG_UART *pConfig) {
  BSP_UART_SetReadCallback(pConfig->Port, _OnRx);
  BSP_UART_SetWriteCallback(pConfig->Port, _OnTx);
  BSP_UART_Init(pConfig->Port, pConfig->Baudrate, pConfig->DataBits, pConfig->Parity, pConfig->StopBits);
  return 0;
}

/*********************************************************************
*
*       _DeInit()
*
* Function description
*   De-initialize UART communication.
*
* Parameters
*   pConfig: Pointer to configuration.
*/
static void _DeInit(MB_IFACE_CONFIG_UART *pConfig) {
  BSP_UART_DeInit(pConfig->Port);
}

/*********************************************************************
*
*       Local functions, Modbus slave API
*
**********************************************************************
*/

/*********************************************************************
*
*       _WriteCoil()
*
* Function description
*   Writes a single coil.
*
* Parameters
*   Addr: Addr. of coil to write.
*
* Return value
*     0: O.K.
*   < 0: Error
*/
static int _WriteCoil(U16 Addr, char OnOff) {
  int r;

  if ((Addr >= BASE_ADDR) && (Addr <= (BASE_ADDR + 1))) {
    Addr -= BASE_ADDR;
    _MBCoil[Addr] = OnOff;
    if (OnOff) {
      BSP_SetLED(Addr);
    } else {
      BSP_ClrLED(Addr);
    }
    r = 0;
  } else {
    r = MB_ERR_ILLEGAL_DATA_ADDR;
  }
  return r;
}

/*********************************************************************
*
*       _ReadCoil()
*
* Function description
*   Reads status of a single coil.
*
* Parameters
*   Addr: Addr. of coil to read.
*
* Return value
*     1: On
*     0: Off
*   < 0: Error
*/
static int _ReadCoil(U16 Addr) {
  int r;

  if ((Addr >= BASE_ADDR) && (Addr <= (BASE_ADDR + 1))) {
    Addr -= BASE_ADDR;
    r = _MBCoil[Addr];
  } else {
    r = MB_ERR_ILLEGAL_DATA_ADDR;
  }
  return r;
}

/*********************************************************************
*
*       _ReadDI()
*
* Function description
*   Reads single Discrete Input status.
*
* Parameters
*   Addr: Addr. of input to read.
*
* Return value
*     1: On
*     0: Off
*   < 0: Error
*/
static int _ReadDI(U16 Addr) {
  int r;

  if ((Addr >= BASE_ADDR) && (Addr <= (BASE_ADDR + 1))) {
    Addr -= BASE_ADDR;
    r = _MBToggle[Addr];
    _MBToggle[Addr] ^= 1;
  } else {
    r = MB_ERR_ILLEGAL_DATA_ADDR;
  }
  return r;
}

/*********************************************************************
*
*       _WriteReg()
*
* Function description
*   Writes a single register.
*
* Parameters
*   Addr: Addr. of register to write.
*
* Return value
*     0: O.K.
*   < 0: Error
*/
static int _WriteReg(U16 Addr, U16 Val) {
  int r;

  if ((Addr >= BASE_ADDR) && (Addr <= (BASE_ADDR + 1))) {
    Addr -= BASE_ADDR;
    _MBReg[Addr] = Val;
    r = 0;
  } else {
    r = MB_ERR_ILLEGAL_DATA_ADDR;
  }
  return r;
}

/*********************************************************************
*
*       _ReadHR()
*
* Function description
*   Reads a single Holding Register.
*
* Parameters
*   Addr: Addr. of Holding Register to read.
*
* Return value
*     0: O.K.
*   < 0: Error
*/
static int _ReadHR(U16 Addr, U16 *pVal) {
  int r;

  if ((Addr >= BASE_ADDR) && (Addr <= (BASE_ADDR + 1))) {
    Addr -= BASE_ADDR;
    *pVal = _MBReg[Addr];
    r = 0;
  } else {
    r = MB_ERR_ILLEGAL_DATA_ADDR;
  }
  return r;
}

/*********************************************************************
*
*       _ReadIR()
*
* Function description
*   Reads a single Input Register.
*
* Parameters
*   Addr: Addr. of Holding Register to read.
*
* Return value
*     0: O.K.
*   < 0: Error
*/
static int _ReadIR(U16 Addr, U16 *pVal) {
  int r;

  if ((Addr >= BASE_ADDR) && (Addr <= (BASE_ADDR + 1))) {
    Addr -= BASE_ADDR;
    *pVal = _MBCnt[Addr];
    _MBCnt[Addr]++;
    r = 0;
  } else {
    r = MB_ERR_ILLEGAL_DATA_ADDR;
  }
  return r;
}

/*********************************************************************
*
*       _HandleCustomFunctionCode()
*
* Function description
*   Handles custom function codes or overwrites stack internal handling.
*
* Parameters
*   pChannel: Pointer to element of type MB_CHANNEL.
*   pPara   : Input/output parameter structure.
*
* Return value
*               >= 0: O.K., function code handled. Number of bytes to send back.
*               <  0: Error, use official Modbus error codes like MB_ERR_ILLEGAL_DATA_VAL .
*   MB_ERR_FUNC_CODE: Function code not handled. Try stack internal handling.
*
* Additional information
*   Function code 0x08 (Diagnostic), subfunction code 0x00 0x00 (Return Query Data) :
*     This very basic diagnostic subfunction echoes back a two byte
*     value that has just been received.
*   Function code 0x30 :
*     The payload of the message is expected to be a printable
*     string with termination. As the string itself is properly
*     terminated no length field is necessary.
*     One U8 is expected as return code that lets the master
*     know if the string has been printed or not. In this sample
*     this is decided by checking if MB_DEBUG is active or not.
*/
static int _HandleCustomFunctionCode(MB_CHANNEL *pChannel, MB_CUSTOM_FUNC_CODE_PARA *pPara) {
  U32 SubCode;
  int r;

  MB_USE_PARA(pChannel);

  r = MB_ERR_FUNC_CODE;  // Assume that we can not handle this function code.

  //
  // Handle custom function codes.
  //
  switch (pPara->Function) {
  case 0x08:
    SubCode = MB_LoadU16BE((const U8*)pPara->pData);
    switch (SubCode) {
    case 0x0000:
      r = 4;  // Send back Subfunction Hi/Lo & Data Hi/Lo fields. Data is echoed back as it is in the input/output buffer.
      break;
    }
    break;
  case 0x30:
    //
    // Output the string that has been sent.
    //
    MB_Log((const char*)pPara->pData);
    //
    // Store MB_DEBUG level as 1 byte answer.
    // Up to pPara->BufferSize bytes might be stored.
    //
    *pPara->pData  = MB_DEBUG;
    r              = 1;  // Tell the stack that we are sending back 1 byte data.
    break;
  }
  return r;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  OS_SetPriority(OS_GetTaskID(), 255);  // This task has highest prio for real-time application. This is only allowed when this task does not use Modbus API.
  //
  // Initially clear all LEDs that are used in Modbus sample.
  //
  BSP_ClrLED(0);
  BSP_ClrLED(1);
  //
  // Start Modbus slave using serial ASCII protocol.
  //
  MB_SLAVE_Init();
  MB_SLAVE_AddASCIIChannel(&_MBChannel, &_MBConfig, &_SlaveAPI, &_IFaceAPI, SLAVE_ADDR, 0, UART_BAUDRATE, UART_NUM_DATA_BITS, UART_PARITY, UART_NUM_STOP_BITS, UART_PORT);  // Add a slave channel.
  MB_SLAVE_SetCustomFunctionCodeHandler(&_MBChannel, _HandleCustomFunctionCode);                                                                                            // Add a custom function code handler for this channel.
  OS_CREATETASK(&_MBSlaveTCB, "MB_SLAVE_Task", MB_SLAVE_Task, TASK_PRIO_MB_SLAVE_TASK, _MBSlaveStack);                                                                      // Start the MB_SLAVE_Task.
  while (1) {
    OS_Delay(1000);
  }
}

/*************************** End of file ****************************/
