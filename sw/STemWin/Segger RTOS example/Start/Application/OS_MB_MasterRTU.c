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
File    : OS_MB_MasterRTU.c
Purpose : Sample program for Modbus master using embOS.
          Demonstrates how to implement a Modbus master using the
          Modbus RTU protocol over a serial connection.
          The sample connects to a Modbus RTU slave via a serial
          connection and toggles some LEDs on the slave.
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

#define TIMEOUT             3000  // Timeout for Modbus response [ms].
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
  TASK_PRIO_MB_MASTER_LP_TASK = 150,  // Low priority Modbus task.
  TASK_PRIO_MB_MASTER_HP_TASK         // High priority Modbus task.
};

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define LOCK()    OS_Use(&_Lock)
#define UNLOCK()  OS_Unuse(&_Lock)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_TICK_HOOK _cbHook;

//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int _MBMasterHPStack[1024/sizeof(int)];  // Define the stack of the Modbus High Priority master to 1024 bytes.
static OS_TASK         _MBMasterHPTCB;

static OS_STACKPTR int _MBMasterLPStack[1024/sizeof(int)];  // Define the stack of the Modbus Low Priority master to 1024 bytes.
static OS_TASK         _MBMasterLPTCB;

static OS_RSEMA        _Lock;                               // Resource semaphore for locking between tasks addressing same slave.

//
// Modbus configuration.
//
static MB_CHANNEL            _MBChannel;
static MB_IFACE_CONFIG_UART  _MBConfig;

static volatile int          _ResultHP;
static volatile int          _ResultLP;

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

static void _SendByte   (MB_IFACE_CONFIG_UART *pConfig, U8 Data);
static int  _Init       (MB_IFACE_CONFIG_UART *pConfig);
static void _DeInit     (MB_IFACE_CONFIG_UART *pConfig);
static void _InitTimer  (U32 MaxFreq);
static void _DeInitTimer(void);

/*********************************************************************
*
*       Static const, Modbus API
*
**********************************************************************
*/

static const MB_IFACE_UART_API _IFaceAPI = {
  _SendByte,    // pfSendByte
  _Init,        // pfInit
  _DeInit,      // pfDeInit
  NULL,         // pfSend
  NULL,         // pfRecv
  NULL,         // pfConnect
  NULL,         // pfDisconnect
  _InitTimer,   // pfInitTimer
  _DeInitTimer  // pfDeInitTimer
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
*       _InitTimer()
*
* Function description
*   Initialize RTU periodic timer.
*
* Parameters
*   MaxFreq: Maximum timer frequency [Hz].
*/
static void _InitTimer(U32 MaxFreq) {
  BSP_USE_PARA(MaxFreq);

  //
  // The default frequency is 1kHz. For simplicity we
  // will be using the embOS tick hook that typically
  // runs with the same frequency. The timer must not
  // use a higher frequency than specified by MaxFreq
  // but might use a lower frequency if necessary.
  //
  OS_AddTickHook(&_cbHook, MB_TimerTick);
}

/*********************************************************************
*
*       _DeInitTimer()
*
* Function description
*   De-initialize RTU periodic timer.
*/
static void _DeInitTimer(void) {
  OS_RemoveTickHook(&_cbHook);
}

/*********************************************************************
*
*       Local functions, Modbus master task
*
**********************************************************************
*/

/*********************************************************************
*
*       _ModbusHPMasterTask()
*
* Function description
*   Modbus High Priority master task executing request to a slave device.
*/
static void _ModbusHPMasterTask(void) {
  U8 State;

  State = 0;

  MB_MASTER_Init();
  MB_MASTER_AddRTUChannel(&_MBChannel, &_MBConfig, &_IFaceAPI, TIMEOUT, SLAVE_ADDR, UART_BAUDRATE, UART_NUM_DATA_BITS, UART_PARITY, UART_NUM_STOP_BITS, UART_PORT);
  do {
    State ^= 1;
    LOCK();
    _ResultHP = MB_MASTER_WriteCoil(&_MBChannel, BASE_ADDR, State);
    UNLOCK();
    OS_Delay(50);
  } while (_ResultHP == 0);
  while (1) {
    BSP_ToggleLED(0);  // Error.
    OS_Delay(1000);
  }
}

/*********************************************************************
*
*       _ModbusLPMasterTask()
*
* Function description
*   Modbus Low Priority master task executing request to a slave device.
*/
static void _ModbusLPMasterTask(void) {
  U8 State;

  State = 0;

  do {
    State ^= 1;
    LOCK();
    _ResultLP = MB_MASTER_WriteCoil(&_MBChannel, BASE_ADDR + 1, State);
    UNLOCK();
    OS_Delay(200);
  } while (_ResultLP == 0);
  while (1) {
    BSP_ToggleLED(0);  // Error.
    OS_Delay(1000);
  }
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
  OS_CREATERSEMA(&_Lock);
  OS_CREATETASK(&_MBMasterHPTCB, "MBMasterHP", _ModbusHPMasterTask, TASK_PRIO_MB_MASTER_HP_TASK, _MBMasterHPStack);  // Start the Modbus HP task.
  OS_CREATETASK(&_MBMasterLPTCB, "MBMasterLP", _ModbusLPMasterTask, TASK_PRIO_MB_MASTER_LP_TASK, _MBMasterLPStack);  // Start the Modbus LP task.
  while (1) {
    BSP_ToggleLED(1);
    OS_Delay (200);
  }
}

/*************************** End of file ****************************/
