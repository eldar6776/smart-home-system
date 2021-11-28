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
File    : ModbusUART_Slave.c
Purpose : Sample program for Modbus slave using Windows.
          Demonstrates how to implement a Modbus slave using the
          Modbus/RTU or Modbus/ASCII protocol.
          The sample setups a Modbus/RTU or Modbus/ASCII slave via
          UART and provides access to some LEDs and other resources.
          The slave provides the following resources:
          - LEDs on coils BASE_ADDR & BASE_ADDR + 1.
          - Two Discrete Inputs on BASE_ADDR & BASE_ADDR + 1 that
            are toggled separately on each access.
          - Two 16-bits registers on BASE_ADDR & BASE_ADDR + 1.
          - Two 16-bits Input Registers on BASE_ADDR & BASE_ADDR + 1
            that count up on each access.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "MB.h"
#include "Modbus_Slave_IFace.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#define SLAVE_ADDR       1
#define BASE_ADDR        1000

// UART
#define COM              0          // 0 for COM1, 1 for COM2, ...
#define BAUDRATE         CBR_38400
#define DATABITS         8
#define PARITY           NOPARITY
#define STOPBITS         ONESTOPBIT

//TCP
#define IP_ADDR_ALLOWED  INADDR_ANY  // IP addr. to accept a connect from.
#define IP_PORT          502


/*********************************************************************
*
*       Types/structures
*
**********************************************************************
*/

typedef void *SYS_HANDLE;

typedef enum {
  SLAVE_UNDEF,
  SLAVE_TCP,
  SLAVE_RTU,
  SLAVE_ASCII,
  SLAVE_MAX
} SLAVE_TYPE;

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define DEFAULT_IFACE    SLAVE_TCP

#define APP_TITLE_TCP    "Modbus/TCP slave"
#define APP_TITLE_RTU    "Modbus/RTU slave"
#define APP_TITLE_ASCII  "Modbus/ASCII slave"
#define APP_TITLE_UNDEF  "Modbus slave"

#define GUI_LOCK()       WaitForSingleObject(_GUILock, INFINITE)
#define GUI_UNLOCK()     ReleaseMutex(_GUILock)

#define MB_ASCII_COLON   0x3A  // ASCII character colon ':'.
#define MB_ASCII_CR      0x0D  // ASCII Carriage Return.
#define MB_ASCII_LF      0x0A  // ASCII Line Feed.

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

U8         _ConnectCnt;
MB_CHANNEL _MBChannel;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static SYS_HANDLE _GUITaskHandle;
static SYS_HANDLE _SlaveTaskHandle;
static SYS_HANDLE _GUILock;                 // Mutex for synchronizing GUI threads.
static int        _HasError;

//
// Modbus configuration.
//
static MB_IFACE_CONFIG_UART  _MBConfigUART;
static MB_IFACE_CONFIG_IP    _MBConfigIP;

static char                  _MBCoil[2];    // Simulate module with 2 coils.
static char                  _MBToggle[2];  // Simulate module with 2 Discrete Input bits, each toggling with each access.
static U16                   _MBReg[2];     // Simulate module with 2 registers.
static U16                   _MBCnt[2];     // Simulate module with 2 Input Registers used as counter, each counting up with each access.

static U16                   _BaseAddr;
static U8                    _SlaveAddr;

static SLAVE_TYPE            _SlaveType;

//
// Serial port configuration
//
static U8                    _ComPort;
static U32                   _ComBaudRate;
static U8                    _ComDataBits;
static U8                    _ComParity;
static U8                    _ComStopBits;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

static int  _WriteCoil  (U16 Addr, char OnOff);
static int  _ReadCoil   (U16 Addr);
static int  _ReadDI     (U16 Addr);
static int  _WriteReg   (U16 Addr, U16   Val);
static int  _ReadHR     (U16 Addr, U16 *pVal);
static int  _ReadIR     (U16 Addr, U16 *pVal);

/*********************************************************************
*
*       Static const, Modbus API
*
**********************************************************************
*/
static const MB_SLAVE_API _SlaveAPI = {
  _WriteCoil,   // pfWriteCoil
  _ReadCoil,    // pfReadCoil
  _ReadDI,      // pfReadDI
  _WriteReg,    // pfWriteReg
  _ReadHR,      // pfReadHR
  _ReadIR       // pfReadIR
};

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _ShowVersionInfo()
*/
static void _ShowVersionInfo(void) {
  int  AppVersionMajor;
  int  AppVersionMinor;
  char acAppRevision[2];

  AppVersionMajor =  MB_VERSION / 10000;
  AppVersionMinor = (MB_VERSION / 100) % 100;
  if (MB_VERSION % 100) {
    acAppRevision[0] = 'a' + (MB_VERSION % 100) - 1;
    acAppRevision[1] = '\0';
  } else {
    acAppRevision[0] = '\0';
  }
  switch (_SlaveType) {
    case SLAVE_TCP:
      printf("SEGGER %s V%d.%02d%s\n", APP_TITLE_TCP, AppVersionMajor, AppVersionMinor, acAppRevision);
      break;
    case SLAVE_RTU:
      printf("SEGGER %s V%d.%02d%s\n", APP_TITLE_RTU, AppVersionMajor, AppVersionMinor, acAppRevision);
      break;
    case SLAVE_ASCII:
      printf("SEGGER %s V%d.%02d%s\n", APP_TITLE_ASCII, AppVersionMajor, AppVersionMinor, acAppRevision);
      break;
    case SLAVE_UNDEF:
    default:
      printf("SEGGER %s V%d.%02d%s\n", APP_TITLE_UNDEF, AppVersionMajor, AppVersionMinor, acAppRevision);
      break;
  }
  printf("Compiled on " __DATE__ " " __TIME__ "\n\n");
}

/*********************************************************************
*
*       _Exit()
*
* Function description
*   Cleanup resources used in sample.
*/
static void _Exit(void) {
  //
  // Terminate all Modbus related tasks.
  //
  if (_GUITaskHandle) {
    TerminateThread(_GUITaskHandle, 0);
  }
  if (_SlaveTaskHandle) {
    TerminateThread(_SlaveTaskHandle, 0);
  }
  //
  // De-init.
  //
  MB_CHANNEL_Disconnect(&_MBChannel);
  MB_SLAVE_DeInit();
  if (_GUILock) {
    CloseHandle(_GUILock);
  }
}

/*********************************************************************
*
*       Local functions, I/O API
*
**********************************************************************
*/

/*********************************************************************
*
*       _EatWhite()
*
* Function description
*   Eats all whitespaces from buffer start up to next character.
*
* Parameters
*   ps: Pointer to pointer of string.
*/
static void _EatWhite(const char** ps) {
  const char* s = *ps;
  while ((*s == ' ') || (*s == '\t') || (*s == '\r') || (*s == '\n')) {
    s++;
  }
  *ps = s;
}

/*********************************************************************
*
*       _ParseDec()
*
* Function description
*   Parses a decimal value out of a string.
*
* Parameters
*   ps   : Pointer to pointer of string.
*   pData: Pointer to buffer where to store the parsed value.
*
* Return value
*   NULL        : O.K.
*   Error string: Pointer to string with error description.
*/
static const char* _ParseDec(const char** ps, U32* pData) {
  unsigned int Data = 0;
  int          NumDigits = 0;

  _EatWhite(ps);
  do {
    int v;
    char c;

    c = **ps;
    v =  ((c >= '0') && (c <= '9')) ? c - '0' : -1;
    if (v >= 0) {
      Data = (Data * 10) + v;
      (*ps)++;
      NumDigits++;
    } else {
      if (NumDigits == 0) {
        return "Expected a dec value";
      }
      *pData = Data;
      return NULL;
    }
  } while (1);
}

/*********************************************************************
*
*       _ParseIP
*
*  Function description
*    Returns U32 IP address out of string.
*
* Parameters
*   s: Pointer to string.
*
* Return value
*   IP addr. parsed from string.
*/
static U32 _ParseIP(const char* s) {
  int i;
  U32 IPByte[4];

  _EatWhite(&s);
  if (*s) {
    for (i = 0; i <= 3; i++) {
      if (_ParseDec(&s, &IPByte[i])) {
        goto Error;
      }
      if (i < 3) {
        if (*s == '.') {
          s++;
        } else {
          goto Error;
        }
      }
    }
    return ((IPByte[0] << 24) |
            (IPByte[1] << 16) |
            (IPByte[2] <<  8) |
             IPByte[3]);
  } else {
    return 0;  // Return dummy IP addr to avoid spamming the network
  }
Error:
  printf("No valid IP addr. Please input in format: 192.168.0.10\n");
  return 0;  // Return dummy IP addr to avoid spamming the network
}

/*********************************************************************
*
*       _ParseU32
*
*  Function description
*    Returns U32 out of string.
*
* Parameters
*   s    : Pointer to string.
*   pData: Pointer to buffer where to store the parsed value.
*
* Return value
*   0: O.K.
*   1: Error.
*/
static int _ParseU32(const char* s, U32 *pData) {
  U32 v;

  _EatWhite(&s);
  if (*s) {
    if (_ParseDec(&s, &v)) {
      goto Error;
    }
    *pData = v;
    return 0;  // O.K.
  }
Error:
  printf("No valid 32-bit input.\n");
  return 1;  // Error.
}

/*********************************************************************
*
*       _ParseU16
*
*  Function description
*    Returns U16 out of string.
*
* Parameters
*   s    : Pointer to string.
*   pData: Pointer to buffer where to store the parsed value.
*
* Return value
*   0: O.K.
*   1: Error.
*/
static int _ParseU16(const char* s, U16 *pData) {
  U32 v;

  _EatWhite(&s);
  if (*s) {
    if (_ParseDec(&s, &v)) {
      goto Error;
    }
    if (v > 0xFFFF) {
      goto Error;
    }
    *pData = (U16)v;
    return 0;  // O.K.
  }
Error:
  printf("No valid 16-bit input.\n");
  return 1;  // Error.
}

/*********************************************************************
*
*       _ParseU8
*
*  Function description
*    Returns U8 out of string.
*
* Parameters
*   s    : Pointer to string.
*   pData: Pointer to buffer where to store the parsed value.
*
* Return value
*   0: O.K.
*   1: Error.
*/
static int _ParseU8(const char* s, U8 *pData) {
  U32 v;

  _EatWhite(&s);
  if (*s) {
    if (_ParseDec(&s, &v)) {
      goto Error;
    }
    if (v > 0xFF) {
      goto Error;
    }
    *pData = (U8)v;
    return 0;  // O.K.
  }
Error:
  printf("No valid 8-bit input.\n");
  return 1;  // Error.
}

/*********************************************************************
*
*       MB_Panic()
*
* Function description
*   This function is called if the stack encounters a critical
*   situation. In a release build, this function may not be linked in.
*
* Parameters
*   s: String to output before running into endless loop.
*/
void MB_Panic(const char *s) {
  MB_OS_DISABLE_INTERRUPT();
#if MB_DEBUG > 1
  printf("*** Fatal error, System halted: %s\n", s);
#endif
  while(s);
}

/*********************************************************************
*
*       MB_Log()
*
* Function description
*   This function is called by the stack in debug builds with log
*   output. In a release build, this function may not be linked in.
*
* Parameters
*   s: String to output.
*
* Notes
*   (1) Interrupts and task switches
*       printf() has a re-entrance problem on a lot of systems if
*       interrupts are not disabled. This would scramble strings
*       if printf(), called from an ISR, interrupts another printf().
*       In order to avoid this problem, interrupts are disabled.
*/
void MB_Log(const char *s) {
  MB_OS_DISABLE_INTERRUPT();
  printf("%s\n", s);
  MB_OS_ENABLE_INTERRUPT();
}

/*********************************************************************
*
*       MB_Warn()
*
* Function description
*   This function is called by the stack in debug builds with warning
*   output. In a release build, this function may not be linked in.
*
* Parameters
*   s: String to output.
*
* Notes
*   (1) Interrupts and task switches
*       See MB_Log() .
*/
void MB_Warn(const char *s) {
  MB_OS_DISABLE_INTERRUPT();
  printf("*** Warning *** %s\n", s);
  MB_OS_ENABLE_INTERRUPT();
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

  if ((Addr >= _BaseAddr) && (Addr <= (_BaseAddr + 1))) {
    Addr -= _BaseAddr;
    _MBCoil[Addr] = OnOff;
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

  if ((Addr >= _BaseAddr) && (Addr <= (_BaseAddr + 1))) {
    Addr -= _BaseAddr;
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

  if ((Addr >= _BaseAddr) && (Addr <= (_BaseAddr + 1))) {
    Addr -= _BaseAddr;
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

  if ((Addr >= _BaseAddr) && (Addr <= (_BaseAddr + 1))) {
    Addr -= _BaseAddr;
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

  if ((Addr >= _BaseAddr) && (Addr <= (_BaseAddr + 1))) {
    Addr -= _BaseAddr;
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

  if ((Addr >= _BaseAddr) && (Addr <= (_BaseAddr + 1))) {
    Addr -= _BaseAddr;
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
*   Handles custom function codes or overwrites stack internal handling
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
*       Local functions, Modbus slave task
*
**********************************************************************
*/

/*********************************************************************
*
*       _ModbusSlaveTask()
*
* Function description
*   Modbus slave task wrapper for system thread.
*/
static UINT __stdcall _ModbusSlaveTask(void *pContext) {
  while (1) {
    MB_SLAVE_Task();
  }
}

/*********************************************************************
*
*       _PrintParity()
*
* Function description
*   Print the parity of the COM configuration in a readable format.
*/
static void _PrintParity(U8 Parity) {
  switch (Parity) {
  case NOPARITY:
    printf("None");
    break;
  case ODDPARITY:
    printf("Odd");
    break;
  case EVENPARITY:
    printf("Even");
    break;
  case MARKPARITY:
    printf("Mark");
    break;
  case SPACEPARITY:
    printf("Space");
    break;
  default:
    printf("???");
  }
}

/*********************************************************************
*
*       _PrintStopBits()
*
* Function description
*   Print the stop bits of the COM configuration in a readable format.
*/
static void _PrintStopBits(U8 StopBits) {
  switch (StopBits) {
  case ONESTOPBIT:
    printf("One");
    break;
  case ONE5STOPBITS:
    printf("1.5");
    break;
  case TWOSTOPBITS:
    printf("Two");
    break;
  default:
    printf("???");
  }
}

/*********************************************************************
*
*       _PrintUARTConfig()
*
* Function description
*   Print a sumary of the COM configuration.
*/
static void _PrintUARTConfig(void) {
  printf("COM configuration used:\n\tCOM%d, Baud:%d, Data bits:%d, Parity:",
         _ComPort+1, _ComBaudRate, _ComDataBits);
  _PrintParity(_ComParity);
  printf(", Stop bits:");
  _PrintStopBits(_ComStopBits);
  printf("\n\n");
}

/*********************************************************************
*
*       _Cls()
*
* Function description
*   Clear the console window. Results in less flickering most of the
*   time than clearing it with system("cls").
*/
static void _Cls(void) {
  HANDLE ConsoleHandle;
  COORD  Pos;
  DWORD  v;

  ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  Pos.X  = 0;
  Pos.Y  = 0;
  FillConsoleOutputCharacter(ConsoleHandle,' ', 1000000, Pos, &v);
  SetConsoleCursorPosition(ConsoleHandle, Pos);
}

/*********************************************************************
*
*       _GUITask()
*
* Function description
*   GUI task that shows the current status of the slave.
*/
static UINT __stdcall _GUITask(void *pContext) {
  while (1) {
    GUI_LOCK();
    _Cls();
    _ShowVersionInfo();
    printf("Started server with slave addr. %d and base addr. %d .\n\n", _SlaveAddr, _BaseAddr);
    printf("Slave status:\n\n");
    if (_SlaveType == SLAVE_TCP) {
      if (_ConnectCnt) {
        printf("A");
      } else {
        printf("No");
      }
      printf(" master is connected to this slave.\n\n");
    } else { // UART
      printf(" COM%d Port ", _MBChannel.pConfig->Port + 1);
      if (_ConnectCnt) {
        printf("successfully ");
      } else {
        printf("NOT ");
      }
      printf("opened (b:%d d:%d s:", _ComBaudRate, _ComDataBits);
      _PrintStopBits(_ComStopBits);
      printf(" p:");
      _PrintParity(_ComParity);
      printf(")\n\n");
    }
    printf(".-----------------------.\n");
    printf("|Addr./ |       |       |\n");
    printf("|Func.  | ");
              printf("%5d", _BaseAddr);
               printf(" | ");
                  printf("%5d", _BaseAddr + 1);
                   printf(" |\n");
    printf("|-----------------------|\n");
    printf("|Coils  |  [");
    if (_MBCoil[0]) {
                printf("X");
    } else {
                printf(" ");
    }
                 printf("]  |  [");
    if (_MBCoil[1]) {
                        printf("X");
    } else {
                        printf(" ");
    }
                         printf("]  |\n");
    printf("|-----------------------|\n");
    printf("|DI     |  [");
    if (_MBToggle[0]) {
                printf("X");
    } else {
                printf(" ");
    }
                 printf("]  |  [");
    if (_MBToggle[1]) {
                        printf("X");
    } else {
                        printf(" ");
    }
                         printf("]  |\n");
    printf("|-----------------------|\n");
    printf("|Reg    | ");
              printf("%5d", _MBReg[0]);
               printf(" | ");
                  printf("%5d", _MBReg[1]);
                           printf(" |\n");
    printf("|-----------------------|\n");
    printf("|IR     | ");
              printf("%5d", _MBCnt[0]);
               printf(" | ");
                  printf("%5d", _MBCnt[1]);
                           printf(" |\n");
    printf("`-----------------------'\n");
    printf("\nPress any key to close.\n\n");
    GUI_UNLOCK();
    Sleep(50);
  }
}

/*********************************************************************
*
*       Utilities for COM description
*
**********************************************************************
*/
/*********************************************************************
*
*       _PrintIFace()
*
* Function description
*   Print the iterface type in a readable format.
*/
static void _PrintIFace(SLAVE_TYPE t) {
  switch (t) {
    case SLAVE_TCP:
      printf("TCP");
      break;
    case SLAVE_RTU:
      printf("RTU");
      break;
    case SLAVE_ASCII:
      printf("ASCII");
      break;
    case SLAVE_UNDEF:
    default:
      printf("Undefined");
      break;
  }
}

/*********************************************************************
*
*       _IsBaudRateValid()
*
* Function description
*   Verifies that the argument correspond to an existing Baud rate.
*
* Return value
*     0: O.K.
*   < 0: Error
*/
static int _IsBaudRateValid(U32 BaudRate) {
  U32 aBaudRates[] = { CBR_110,
                       CBR_300,
                       CBR_600,
                       CBR_1200,
                       CBR_2400,
                       CBR_4800,
                       CBR_9600,
                       CBR_14400,
                       CBR_19200,
                       CBR_38400,
                       CBR_56000,
                       CBR_57600,
                       CBR_115200,
                       CBR_128000,
                       CBR_256000
                     };
  int i;
  int NumElements;

  NumElements = SEGGER_COUNTOF(aBaudRates);

  for (i = 0; i < NumElements; i++) {
    if (BaudRate == aBaudRates[i]) {
      return 0; // Found it
    }
  }
  return -1;
}

/*********************************************************************
*
*       _PrintValidBaudRate()
*
* Function description
*   Print all the defined baud rate to help the user.
*/
static void _PrintValidBaudRate(void) {
  U32 aBaudRates[] = { CBR_110,
                       CBR_300,
                       CBR_600,
                       CBR_1200,
                       CBR_2400,
                       CBR_4800,
                       CBR_9600,
                       CBR_14400,
                       CBR_19200,
                       CBR_38400,
                       CBR_56000,
                       CBR_57600,
                       CBR_115200,
                       CBR_128000,
                       CBR_256000
                     };
  int i;
  int NumElements;

  NumElements = SEGGER_COUNTOF(aBaudRates);

  printf("Defined baud rates: ");
  for (i = 0; i < NumElements; i++) {
    printf("%d ", aBaudRates[i]);
  }
  printf("\n");
}

/*********************************************************************
*
*       _GetComConfiguration()
*
* Function description
*   Get from the user, the various parameters of the COM configuration.
*
* Return value
*     0: O.K.
*   < 0: Error
*/
static int _GetComConfiguration(void) {
  char ac[128];
  //
  // COM Port
  //
  printf("Enter COM port to use (dec.) [COM%d]: ", COM+1);
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _ComPort = COM;
  } else {
    if (_ParseU8(&ac[0], &_ComPort) != 0) {
      printf("\nError! Press any key to continue\n");
      getch();
      return -1;
    }
    if (_ComPort == 0) {
      printf("\nError! Press any key to continue\n");
      getch();
      return -1;
    } else {
      _ComPort--;
    }
  }
  //
  // Baud rate
  //
  printf("Enter Baud rate to use (dec.) [%d]: ", BAUDRATE);
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _ComBaudRate = BAUDRATE;
  } else {
    if (_ParseU32(&ac[0], &_ComBaudRate) != 0) {
      printf("\nError! Press any key to continue\n");
      getch();
      return -1;
    }
    // Check it is a valid baudrate
    if (_IsBaudRateValid(_ComBaudRate) != 0)   {
      _PrintValidBaudRate();
      printf("\nError! Press any key to continue\n");
      getch();
      return -1;
    }
  }
  //
  // Data bits
  //
  printf("Enter the number of data bits (dec. 4 - 8) [%d]: ", DATABITS);
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _ComDataBits = DATABITS;
  } else {
    if (_ParseU8(&ac[0], &_ComDataBits) != 0) {
      printf("\nPress any key to continue\n");
      getch();
      return -1;
    }
    if ((_ComDataBits < 4) || (_ComDataBits > 8)) {
      printf("\nError! Press any key to continue\n");
      getch();
      return -1;
    }
  }
  //
  // Parity
  //
  printf("Enter the parity (0:None|1:Odd|2:Even|3:Mark|4:Space) [");
  _PrintParity(PARITY);
  printf("]: ");
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _ComParity = PARITY;
  } else {
    if (_ParseU8(&ac[0], &_ComParity) != 0) {
      printf("\nPress any key to continue\n");
      getch();
      return -1;
    }
    if (_ComParity > 4) {
      printf("\nError! Press any key to continue\n");
      getch();
      return -1;
    }
  }
  //
  // Stop bit
  //
  printf("Enter the number of stop bits (0:One|1:One and half|2:Two) [");
  _PrintStopBits(STOPBITS);
  printf("]: ");
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _ComStopBits = STOPBITS;
  } else {
    if (_ParseU8(&ac[0], &_ComStopBits) != 0) {
      printf("\nPress any key to continue\n");
      getch();
      return -1;
    }
    if (_ComStopBits > 2) {
      printf("\nError! Press any key to continue\n");
      getch();
      return -1;
    }
  }
  return 0;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       main
*/
int main(void) {
  int  ThreadId;
  char ac[128];

  atexit(_Exit);  // Register cleanup code.
  //
  _SlaveType = SLAVE_UNDEF;
  _ShowVersionInfo();
  //
  // Ask which interface to use (TCP, RTU or ASCII)
  //
  printf("Enter interface type (1:TCP|2:RTU|3:ASCII) [");
  _PrintIFace(DEFAULT_IFACE);
  printf("]: ");
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _SlaveType = DEFAULT_IFACE;
  } else {
    U8 tmp;
    if (_ParseU8(&ac[0], &tmp) != 0) {
      printf("\nPress any key to continue\n");
      getch();
      return 1;
    }
    _SlaveType = tmp;
  }
  if ((_SlaveType == SLAVE_UNDEF) || (_SlaveType >= SLAVE_MAX)) {
    printf("\nError! Press any key to continue\n");
    getch();
    return 1;
  }
  //
  // Change the application description with TCP, RTU or ASCII
  //
  _Cls();
  _ShowVersionInfo();
  printf("\n");
  //
  // If it is a serial link, let's request the various parameters
  //
  if (_SlaveType != SLAVE_TCP) {
    if (_GetComConfiguration()) {
      return 1;
    }
    //
    // Print the result
    //
    _Cls();
    _ShowVersionInfo();
    printf("\n");
    _PrintUARTConfig();
    printf("\n");
  }
  //
  // Ask user for some parameters.
  //
  printf("Enter slave address (dec.) [%d]: ", SLAVE_ADDR);
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _SlaveAddr = SLAVE_ADDR;
  } else {
    if (_ParseU8(&ac[0], &_SlaveAddr) != 0) {
      printf("\nPress any key to continue\n");
      getch();
      return 1;
    }
  }
  printf("Enter base address of registers (dec.) [%d]: ", BASE_ADDR);
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _BaseAddr = BASE_ADDR;
  } else {
    if (_ParseU16(&ac[0], &_BaseAddr) != 0) {
      printf("\nPress any key to continue\n");
      getch();
      return 1;
    }
  }
  //
  printf("\n");
  //
  // Init resources necessary for sample.
  //
  _GUILock  = CreateMutex(NULL, FALSE, NULL);
  //
  // Init slave and start threads as necessary.
  //
  MB_SLAVE_Init();
  _GUITaskHandle   = (SYS_HANDLE)CreateThread(NULL, 0, _GUITask, NULL, 0, &ThreadId);                                // Start GUI task.
  _SlaveTaskHandle = (SYS_HANDLE)CreateThread(NULL, 0, _ModbusSlaveTask, NULL, 0, &ThreadId);                        // Start the MB_SLAVE_Task.
  //
  // Add the slave channel of the corresponding interface
  //
  switch (_SlaveType) {
    case SLAVE_TCP:
      MB_SLAVE_AddIPChannel(&_MBChannel, &_MBConfigIP, &_SlaveAPI, &_IFaceTCP, _SlaveAddr, 0, IP_ADDR_ALLOWED, IP_PORT);   // Add a slave channel.
      MB_SLAVE_SetCustomFunctionCodeHandler(&_MBChannel, _HandleCustomFunctionCode);                                                                           // Add a custom function code handler for this channel.
      break;
    case SLAVE_RTU:
      MB_SLAVE_AddRTUChannel(&_MBChannel, &_MBConfigUART, &_SlaveAPI, &_IFaceRTU, _SlaveAddr, 0,
                             _ComBaudRate, _ComDataBits, _ComParity, _ComStopBits, _ComPort);
      MB_SLAVE_SetCustomFunctionCodeHandler(&_MBChannel, _HandleCustomFunctionCode);                                                                           // Add a custom function code handler for this channel.
      break;
    case SLAVE_ASCII:
      MB_SLAVE_AddASCIIChannel(&_MBChannel, &_MBConfigUART, &_SlaveAPI, &_IFaceASCII, _SlaveAddr, 0,
                               _ComBaudRate, _ComDataBits, _ComParity, _ComStopBits, _ComPort);
      MB_SLAVE_SetCustomFunctionCodeHandler(&_MBChannel, _HandleCustomFunctionCode);                                                                           // Add a custom function code handler for this channel.
      break;
    default:
      printf("Error in chosen interface, press a key\n");
  }
  //
  // Wait for any key to signal exit.
  //
  getch();
  GUI_LOCK();
  //
  return 0;
}

/****** End Of File *************************************************/
