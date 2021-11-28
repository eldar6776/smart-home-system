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
File    : ModbusUART_Master.c
Purpose : Sample program for Modbus master using Windows.
          Demonstrates how to implement a Modbus master using the
          Modbus/RTU or Modbus/ASCII protocol.
          The sample connects to a Modbus/RTU or Modbus/ASCII slave
          via UART and toggles some LEDs on the slave.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "MB.h"
#include "Modbus_Master_IFace.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#define TIMEOUT          3000       // Timeout for Modbus response [ms].
#define SLAVE_ADDR       1
#define BASE_ADDR        1000

// TCP
#define SLAVE_IP_PORT    502

// UART
#define COM              0          // 0 for COM1, 1 for COM2, ...
#define BAUDRATE         CBR_38400
#define DATABITS         8
#define PARITY           NOPARITY
#define STOPBITS         ONESTOPBIT

/*********************************************************************
*
*       Types/structures
*
**********************************************************************
*/

typedef void *SYS_HANDLE;

typedef enum {
  MASTER_UNDEF,
  MASTER_TCP,
  MASTER_RTU,
  MASTER_ASCII,
  MASTER_MAX
} MASTER_TYPE;

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define DEFAULT_IFACE    MASTER_TCP

#define APP_TITLE_TCP    "Modbus/TCP master"
#define APP_TITLE_RTU    "Modbus/RTU master"
#define APP_TITLE_ASCII  "Modbus/ASCII master"
#define APP_TITLE_UNDEF  "Modbus master"

#define LOCK()           WaitForSingleObject(_Lock, INFINITE)
#define UNLOCK()         ReleaseMutex(_Lock)

#define MB_ASCII_COLON   0x3A  // ASCII character colon ':'.
#define MB_ASCII_CR      0x0D  // ASCII Carriage Return.
#define MB_ASCII_LF      0x0A  // ASCII Line Feed.

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

U8                       _ConnectCnt;
MB_CHANNEL               _MBChannel;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static SYS_HANDLE            _HPTaskHandle;
static SYS_HANDLE            _LPTaskHandle;
static SYS_HANDLE            _Lock;      // Mutex for locking between tasks addressing same slave.
static int                   _HasError;

//
// Modbus configuration.
//
static MB_IFACE_CONFIG_UART  _MBConfigUART;
static MB_IFACE_CONFIG_IP    _MBConfigIP;

static U16                   _BaseAddr;

static MASTER_TYPE           _MasterType;

//
// Serial port configuration
//
static U8                    _ComPort;
static U32                   _ComBaudRate;
static U8                    _ComDataBits;
static U8                    _ComParity;
static U8                    _ComStopBits;
//
// TCP
//
static U32                   _IPAddr;

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _ShowVersionInfo
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
  switch (_MasterType) {
    case MASTER_TCP:
      printf("SEGGER %s V%d.%02d%s\n", APP_TITLE_TCP, AppVersionMajor, AppVersionMinor, acAppRevision);
      break;
    case MASTER_RTU:
      printf("SEGGER %s V%d.%02d%s\n", APP_TITLE_RTU, AppVersionMajor, AppVersionMinor, acAppRevision);
      break;
    case MASTER_ASCII:
      printf("SEGGER %s V%d.%02d%s\n", APP_TITLE_ASCII, AppVersionMajor, AppVersionMinor, acAppRevision);
      break;
    case MASTER_UNDEF:
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
  if (_HPTaskHandle) {
    TerminateThread(_HPTaskHandle, 0);
  }
  if (_LPTaskHandle) {
    TerminateThread(_LPTaskHandle, 0);
  }
  //
  // De-init.
  //
  MB_CHANNEL_Disconnect(&_MBChannel);
  MB_MASTER_DeInit();
  if (_Lock) {
    CloseHandle(_Lock);
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
*       _EatWhite
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
*       _ParseDec
*/
static const char* _ParseDec(const char** ps, unsigned int* pData) {
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
static UINT __stdcall _ModbusHPMasterTask(void *pContext) {
  int r;
  U8  State;
  U8  Data = 0;

  MB_USE_PARA(pContext);

  State = 0;

  do {
    State ^= 1;
    LOCK();
    if (_HasError == 0) {
      r = MB_MASTER_WriteCoil(&_MBChannel, (U16)(_BaseAddr), State);
      if (r) {
        printf("Error in HP Task.\nPress any key to abort.\n");
        _HasError = 1;
      }
#if 0
      r = MB_MASTER_ReadDI(&_MBChannel, &Data, (U16)(_BaseAddr), 1);
      if (r) {
        printf("Error in LP Task.\nPress any key to abort.\n");
        _HasError = 1;
      }
#endif
    }
    UNLOCK();
    Sleep(150);
  } while (_HasError == 0);
  while (1) {
    Sleep(1000);
  }
}

/*********************************************************************
*
*       _ModbusLPMasterTask()
*
* Function description
*   Modbus Low Priority master task executing request to a slave device.
*/
static UINT __stdcall _ModbusLPMasterTask(void *pContext) {
  int r;
  U8  State;
  U16 Data = 0;

  MB_USE_PARA(pContext);

  State = 0;

  do {
    State ^= 1;
    LOCK();
    if (_HasError == 0) {
      r = MB_MASTER_WriteCoil(&_MBChannel, (U16)(_BaseAddr + 1), State);
      if (r) {
        printf("Error in LP Task.\nPress any key to abort.\n");
        _HasError = 1;
      }
#if 0
      r = MB_MASTER_WriteReg(&_MBChannel, Data++,(U16)(_BaseAddr + 1));
      if (r) {
        printf("Error in LP Task.\nPress any key to abort.\n");
        _HasError = 1;
      }
#endif
    }
    UNLOCK();
    Sleep(500);
  } while (_HasError == 0);
  while (1) {
    Sleep(1000);
  }
}

/*********************************************************************
*
*       Print utilities
*
**********************************************************************
*/

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
  printf("COM configuration used:\n\tCOM%d, Baud:%d, Data bits:%d, Stop bits:",
         _ComPort+1, _ComBaudRate, _ComDataBits);
  _PrintStopBits(_ComStopBits);
  printf(", Parity:");
  _PrintParity(_ComParity);
  printf("\n\n");
}

/*********************************************************************
*
*       _PrintIFace()
*
* Function description
*   Print the iterface type in a readable format.
*/
static void _PrintIFace(MASTER_TYPE t) {
  switch (t) {
    case MASTER_TCP:
      printf("TCP");
      break;
    case MASTER_RTU:
      printf("RTU");
      break;
    case MASTER_ASCII:
      printf("ASCII");
      break;
    case MASTER_UNDEF:
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
  U8   SlaveAddr;

  atexit(_Exit);  // Register cleanup code.
  //
  _MasterType = MASTER_UNDEF;
  _ShowVersionInfo();
  //
  // Ask which interface to use (TCP, RTU or ASCII)
  //
  printf("Enter interface type (1:TCP|2:RTU|3:ASCII) [");
  _PrintIFace(DEFAULT_IFACE);
  printf("]: ");
  fgets(ac, sizeof(ac), stdin);
  if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
    _MasterType = DEFAULT_IFACE;
  } else {
    U8 tmp;
    if (_ParseU8(&ac[0], &tmp) != 0) {
      printf("\nPress any key to continue\n");
      getch();
      return 1;
    }
    _MasterType = tmp;
  }
  if ((_MasterType == MASTER_UNDEF) || (_MasterType >= MASTER_MAX)) {
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
  // Let's request the various parameters according to the link type
  //
  if (_MasterType == MASTER_TCP) {
    printf("Enter network address of Modbus/TCP slave [127.0.0.1]: ");
    fgets(ac, sizeof(ac), stdin);
    if ((ac[0] == MB_ASCII_LF) || (ac[0] == MB_ASCII_CR)) {
      _IPAddr = 0x7f000001;
    } else {
      _IPAddr = _ParseIP(&ac[0]);
      if (_IPAddr == 0) {
        printf("\nPress any key to continue\n");
        getch();
        return 1;
      }
    }
  } else {
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
    SlaveAddr = SLAVE_ADDR;
  } else {
    if (_ParseU8(&ac[0], &SlaveAddr) != 0) {
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
  printf("\n");
  printf("Executing blinky on slave addr. %d by toggling coils on addr. %d & %d .\n", SlaveAddr, _BaseAddr, _BaseAddr + 1);
  //
  // Init resources necessary for sample.
  //
  _Lock = CreateMutex(NULL, FALSE, NULL);
  //
  // Init master and start threads as necessary.
  //
  MB_MASTER_Init();
  //
  // Add the master Modbus channel
  //
  switch (_MasterType) {
    case MASTER_TCP:
      MB_MASTER_AddIPChannel(&_MBChannel, &_MBConfigIP, &_IFaceTCP, TIMEOUT, SlaveAddr, _IPAddr, SLAVE_IP_PORT);
      break;
    case MASTER_RTU:
      MB_MASTER_AddRTUChannel(&_MBChannel, &_MBConfigUART, &_IFaceRTU, TIMEOUT, SlaveAddr,
                              _ComBaudRate, _ComDataBits, _ComParity, _ComStopBits, _ComPort);
      break;
    case MASTER_ASCII:
      MB_MASTER_AddASCIIChannel(&_MBChannel, &_MBConfigUART, &_IFaceASCII, TIMEOUT, SlaveAddr,
                                _ComBaudRate, _ComDataBits, _ComParity, _ComStopBits, _ComPort);
      break;
    default:
      printf("Error in chosen interface, press a key\n");
  }
  _HPTaskHandle = (SYS_HANDLE)CreateThread(NULL, 0, _ModbusHPMasterTask, NULL, 0, &ThreadId);  // Start the first Modbus task.
  _LPTaskHandle = (SYS_HANDLE)CreateThread(NULL, 0, _ModbusLPMasterTask, NULL, 0, &ThreadId);  // Start the second Modbus task.
  //
  // Wait for any key to signal exit.
  //
  printf("\nPress any key to close.\n\n");
  getch();
  LOCK();  // Make sure that the other threads do not execute Modbus code.
  //
  return 0;
}

/****** End Of File *************************************************/
