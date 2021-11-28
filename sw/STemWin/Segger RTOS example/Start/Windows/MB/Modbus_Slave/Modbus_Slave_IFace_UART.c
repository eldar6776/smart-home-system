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
          Implement the UART interface (RTU and ASCII) for Modbus
          Slave when using the RTU or ASCII protocol
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "MB.h"
#include "MB_Int.h"
#include "Modbus_Slave_IFace.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#define POLL_FREQUENCY   25         // in ms

/*********************************************************************
*
*       Types/structures
*
**********************************************************************
*/


/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define MB_ASCII_COLON  0x3A  // ASCII character colon ':'.
#define MB_ASCII_CR     0x0D  // ASCII Carriage Return.
#define MB_ASCII_LF     0x0A  // ASCII Line Feed.

#define RX_BUFFER_SIZE  MB_BUFFER_SIZE

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

//
// Modbus configuration.
//

static HANDLE                _hEvent;
static HANDLE                _hCom;
static OVERLAPPED            _Over;

static unsigned int          _TimerId;

static U8                    _RxBuffer[RX_BUFFER_SIZE];

static int                   _WriteMsg;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
static int  _InitRTU       (MB_IFACE_CONFIG_UART *pConfig);
static void _DeInitRTU     (MB_IFACE_CONFIG_UART *pConfig);
static int  _InitASCII     (MB_IFACE_CONFIG_UART *pConfig);
static void _DeInitASCII   (MB_IFACE_CONFIG_UART *pConfig);
static void _SendByte      (MB_IFACE_CONFIG_UART *pConfig, const U8  Data);
static int  _Send          (MB_IFACE_CONFIG_UART *pConfig, const U8 *pData, U32 NumBytes);
static int  _Recv          (MB_IFACE_CONFIG_UART *pConfig,       U8 *pData, U32 NumBytes, U32 Timeout);
static int  _Connect       (MB_IFACE_CONFIG_UART *pConfig, U32 Timeout);
static void _Disconnect    (MB_IFACE_CONFIG_UART *pConfig);
static void _InitTimerRTU  (U32 MaxFreq);
static void _InitTimerASCII(U32 MaxFreq);
static void _DeInitTimer   (void);

/*********************************************************************
*
*       Static const, Modbus API
*
**********************************************************************
*/
const MB_IFACE_UART_API _IFaceRTU = {
  NULL,           // pfSendByte
  _InitRTU,       // pfInit
  _DeInitRTU,     // pfDeInit
  _Send,          // pfSend
  _Recv,          // pfRecv
  _Connect,       // pfConnect
  _Disconnect,    // pfDisconnect
  _InitTimerRTU,  // pfInitTimer
  _DeInitTimer    // pfDeInitTimer
};
const MB_IFACE_UART_API _IFaceASCII = {
  _SendByte,      // pfSendByte
  _InitASCII,     // pfInit
  _DeInitASCII,   // pfDeInit
  NULL,           // pfSend
  NULL,           // pfRecv
  NULL,           // pfConnect
  NULL,           // pfDisconnect
  NULL,           // pfInitTimer
  NULL            // pfDeInitTimer
};


/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       Local functions, Modbus network interface
*
**********************************************************************
*/

/*********************************************************************
*
*       _InitRTU()
*
* Function description
*   Create the handle for the COM port.
*
* Parameters
*   pConfig: Pointer to configuration.
*
* Return value
*   O.K. : 0
*   Error: Other
*/
static int _InitRTU(MB_IFACE_CONFIG_UART *pConfig) {

  _hEvent = CreateEvent(
                        NULL,   // Pointer to security attributes
                        0,      // flag for manual-reset event
                        0,      // flag for initial state
                        NULL    // pointer to event-object name
  );
  if (_hEvent == 0) {
    printf("Init: CreateEvent failed\n");
    return MB_ERR_MISC;
  }

  return 0;
}

/*********************************************************************
*
*       _DeInitRTU()
*
* Function description
*   Destroy the handle for the COM port.
*
* Parameters
*   pConfig: Pointer to configuration.
*/
static void _DeInitRTU(MB_IFACE_CONFIG_UART *pConfig) {
  (void)pConfig;
  CloseHandle(_hEvent);
}

/*********************************************************************
*
*       _InitASCII()
*
* Function description
*   Open the COM port and configure it.
*
* Parameters
*   pConfig: Pointer to configuration. Start the periodic timer.
*
* Return value
*   O.K. : 0
*   Error: Other
*/
static int _InitASCII(MB_IFACE_CONFIG_UART *pConfig) {
  int r;

  r = _InitRTU(pConfig);
  if (r != 0) {
    return r;
  }
  //
  _WriteMsg = 0;
  //
  r = _Connect(pConfig, 0 /* Not used */);
  if (r != 0) {
    return r;
  }
  //
  _InitTimerASCII(0);
  return 0;
}

/*********************************************************************
*
*       _DeInitASCII()
*
* Function description
*   Diconnect and close the COM port. Stop the periodic timer.
*
* Parameters
*   pConfig: Pointer to configuration.
*/
static void _DeInitASCII(MB_IFACE_CONFIG_UART *pConfig) {
  (void)pConfig;
  _WriteMsg = 0;
  _Disconnect(pConfig);
  _DeInitTimer();
  _DeInitRTU(pConfig);
}

/*********************************************************************
*
*       _Connect()
*
* Function description
*   Open the COM port and configure it.
*
* Parameters
*   pConfig: Pointer to configuration.
*   Timeout: Not used for slave.
*
* Return value
*   0    : New connection accepted.
*   Other: Error (or no new connection in case of non-blocking).
*/
static int _Connect(MB_IFACE_CONFIG_UART *pConfig, U32 Timeout) {
  char *       s     = 0;
  HANDLE *     ph    = &_hCom;
  OVERLAPPED * pOver = &_Over;
  char         ac[80];
  HANDLE       hCom;
  DCB          dcb;                 // device control block
  COMMTIMEOUTS ct;

  MB_OS_DISABLE_INTERRUPT();
  if (_ConnectCnt > 0) {
    _ConnectCnt--;
  }
  MB_OS_ENABLE_INTERRUPT();

  sprintf(ac, "\\\\.\\COM%d", pConfig->Port + 1);
  *ph = hCom = CreateFile(
                          ac,
	                        GENERIC_READ | GENERIC_WRITE,
	                        0,			                         // share mode
	                        NULL,			                       // security attr.
	                        OPEN_EXISTING,
	                        FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,	  // Flags & attributes
	                        0                                // hTemplateFile
	                       );
  if (hCom==INVALID_HANDLE_VALUE) {
    DWORD Err = GetLastError();
    
    printf("Can not open com-port %d\n", pConfig->Port + 1);
    return MB_ERR_CONNECT;
  }
  pOver->hEvent = _hEvent;
  //
  // Setup queue sizes
  //
  if (!SetupComm(hCom, 1000, 1000)) {
    printf("SetupComm failed !\n");
    return MB_ERR_CONNECT;
  }
  //
  // Set Com state
  //
  if (!GetCommState(hCom, &dcb)) {
    printf("Com-port problem\n");
    return MB_ERR_CONNECT;
  }
  dcb.BaudRate     = pConfig->Baudrate;
  dcb.fBinary      = 1;
  dcb.fParity      = 0;
  dcb.fOutxCtsFlow = 0;                       // Ignore CTS (send anyway)
  dcb.fOutxDsrFlow = 0;                       // Ignore DSR
  dcb.ByteSize     = pConfig->DataBits;
  dcb.Parity       = pConfig->Parity;
  dcb.StopBits     = pConfig->StopBits;
  dcb.fOutX        = 0;                       // Ignore incoming XOFFs
  dcb.fInX         = 0;                       // Do not send XOFFs
  dcb.fRtsControl  = RTS_CONTROL_ENABLE;// RTS_CONTROL_TOGGLE;
  dcb.fDtrControl  = DTR_CONTROL_ENABLE;// DTR_CONTROL_HANDSHAKE;
  if (!SetCommState(hCom, &dcb)) {
    int Err;

    Err = GetLastError();
    printf("_Connect: Com-port problem\n");
    return MB_ERR_CONNECT;
  }
  //
  // Set Com timeouts
  //
  GetCommTimeouts(hCom, &ct);
  ct.ReadIntervalTimeout         = MAXDWORD;
  ct.ReadTotalTimeoutMultiplier  = 0;
  ct.ReadTotalTimeoutConstant    = 0;
  ct.WriteTotalTimeoutMultiplier = 0;
  ct.WriteTotalTimeoutConstant   = 0;
  SetCommTimeouts(hCom, &ct);
  //
  _ConnectCnt++;
  //
  // Empty the port before starting
  //
  PurgeComm(_hCom, PURGE_TXCLEAR|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_RXABORT);
  //
  return 0;
}

/*********************************************************************
*
*       _Disconnect()
*
* Function description
*   Close COM port.
*
* Parameters
*   pConfig: Pointer to configuration.
*/
static void _Disconnect(MB_IFACE_CONFIG_UART *pConfig) {
  (void)pConfig;
  if (_hCom != INVALID_HANDLE_VALUE) {
    CloseHandle(_hCom);
    _hCom = INVALID_HANDLE_VALUE;
    _ConnectCnt--;
  }
}

/*********************************************************************
*
*       _SendByte()
*
* Function description
*   Sends 1 byte of data on the interface. (ASCII case)
*
* Parameters
*   pConfig : Pointer to configuration.
*   Data    : Character to send.
*/
static void  _SendByte(MB_IFACE_CONFIG_UART *pConfig, const U8  Data) {
  U32 nofBytes;
  int r;
  U32 dwRes;

  if (_hCom != INVALID_HANDLE_VALUE) {
    //
    // Don't call OnTx here as it will call _SendByte again,
    // Instead let's start from the timer thread.
    //
    if (_WriteMsg == 0) {
      _WriteMsg = 1;
    } else {
      //
      // If _WriteMsg is already set, it means this function is called from the timer thread
      // so it is possible to send directly
      //
      r = WriteFile(_hCom, &Data, 1, &nofBytes, &_Over);
      if (r == 0) {
        int Err;
    
        Err = GetLastError();
        if (Err != ERROR_IO_PENDING) {
          printf("Error %d on COM%d\n", Err, pConfig->Port + 1);
          return;
        } else {
          //
          // Write is pending.
          //
          dwRes = WaitForSingleObject(_Over.hEvent, INFINITE);
          switch(dwRes) {
            //
            // OVERLAPPED structure's event has been signaled.
            //
            case WAIT_OBJECT_0:
              if (!GetOverlappedResult(_hCom, &_Over, &nofBytes, FALSE)) {
                return;
              }
              //
              // Else, write operation completed successfully
              //
              break;
            default:
              //
              // An error has occurred in WaitForSingleObject.
              // This usually indicates a problem with the
              // OVERLAPPED structure's event handle.
              //
              return;
          }
        }
      }
    }
  }
}

/*********************************************************************
*
*       _Send()
*
* Function description
*   Sends data on the interface (RTU case).
*
* Parameters
*   pConfig : Pointer to configuration.
*   pData   : Pointer to data to send.
*   NumBytes: Number of bytes to send from pData.
*
* Return value
*   >= 0: NumBytes sent.
*   <  0: Error.
*/
static int _Send(MB_IFACE_CONFIG_UART *pConfig, const U8 *pData, U32 NumBytes) {
  int r;
  U32 nofBytes = 0;
  U32 dwRes;
  int SilentTimeMs;

  if (_hCom != INVALID_HANDLE_VALUE) {

    // Windows is not a realtime system, thus it is not really garantied that
    // the 3,5 silent period could be done accurately
    SilentTimeMs = (11 * 1000 * 7 + 2 * pConfig->Baudrate - 1) / (pConfig->Baudrate * 2);
    Sleep(SilentTimeMs);
    //
    r = WriteFile(_hCom, pData, NumBytes, &nofBytes, &_Over);
    if (r == 0) {
      int Err;
    
      Err = GetLastError();
      if (Err != ERROR_IO_PENDING) {
        printf("Error %d on COM%d\n", Err, pConfig->Port + 1);
        goto SendError;
      } else {
        //
        // Write is pending.
        //
        dwRes = WaitForSingleObject(_Over.hEvent, POLL_FREQUENCY);
        switch(dwRes) {
          //
          // OVERLAPPED structure's event has been signaled.
          //
          case WAIT_OBJECT_0:
            if (!GetOverlappedResult(_hCom, &_Over, &nofBytes, FALSE)) {
              goto SendError;
            }
            //
            // Else, write operation completed successfully
            //
            break;
          default:
            //
            // An error has occurred in WaitForSingleObject.
            // This usually indicates a problem with the
            // OVERLAPPED structure's event handle.
            //
            goto SendError;
        }
      }
    }

    Sleep(SilentTimeMs);
    return nofBytes;
  } else {
    return MB_ERR_DISCONNECT;
  }

SendError:
  return MB_ERR_MISC;
}

/*********************************************************************
*
*       _Recv()
*
* Function description
*   Receives data from the interface. (Used only in RTU mode)
*
* Parameters
*   pConfig : Pointer to configuration.
*   Timeout : Timeout [ms] for the receive operation to return. Always 0 if slave channel.
*   pData   : Pointer to buffer to store data received.
*   NumBytes: Number of bytes received.
*
* Return value
*   >= 0: Number of bytes read.
*   <  0: Error.
*/
static int _Recv(MB_IFACE_CONFIG_UART *pConfig, U8 *pData, U32 NumBytes, U32 Timeout) {
  unsigned long nofBytes = 0;
  unsigned long nofBytesPrevious = 1;
  U32 NumReceivedBytes = 0;

  if (_hCom != INVALID_HANDLE_VALUE) {
    do {
      ReadFile(_hCom,(void*)(pData + NumReceivedBytes), NumBytes - NumReceivedBytes, &nofBytes, &_Over);
      NumReceivedBytes += nofBytes;
      if ((nofBytes == 0) && (NumReceivedBytes) && (nofBytesPrevious == 0)) {
        //
        // Nothing left to read
        //
        return NumReceivedBytes;
      }
      if ((NumReceivedBytes == 0) && (nofBytes == 0)) { // No need to wait as there is nothing on-going
        return 0;
      }
      nofBytesPrevious = nofBytes;

      if (nofBytes == 0) {
        Sleep(POLL_FREQUENCY);
      }
      Sleep(1);
    } while (1);
  }

  return MB_ERR_DISCONNECT;
}

/*********************************************************************
*
*       _TimeProcRTU()
*
* Function description
*   Callback of timer to poll in case of RTU mode.
*/
static void CALLBACK _TimeProcRTU(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {

  static int HandleTimeProc = 0;

  if (HandleTimeProc) {
    return;
  }
  HandleTimeProc = 1;

  _MBChannel.NumBytesInBuffer = 0;
  if (MB_SLAVE_PollChannel(&_MBChannel)) {
    // Wait for the send to be executed
    Sleep(10);
  }

  HandleTimeProc = 0;
}

/*********************************************************************
*
*       _TimeProcASCII()
*
* Function description
*   Callback of timer to simulate the interrupt in case of ASCII mode.
*/
static void CALLBACK _TimeProcASCII(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {

  static int HandleTimeProc = 0;
  unsigned long nofBytes = 0;
  unsigned long i;
  int r;
  const MB_IFACE_PROT_API *pIFaceProtAPI = (MB_IFACE_PROT_API *)_MBChannel.pIFaceProtAPI;

  if (HandleTimeProc) {
    return;
  }
  HandleTimeProc = 1;

  if (_hCom != INVALID_HANDLE_VALUE) {
    ReadFile(_hCom,(void*)_RxBuffer, RX_BUFFER_SIZE, &nofBytes, &_Over);
    for (i = 0; i < nofBytes; i++) {
      r = pIFaceProtAPI->pfOnRx(&_MBChannel, _RxBuffer[i]);
      if (r > 0) {
        // Complete Modbus message received
        MB_CHANNEL_Signal(&_MBChannel);
      }
    }
    //
    // A message needs to be sent, start with the colon and then call the OnTx
    //
    if (_WriteMsg) {
      _SendByte(_MBChannel.pConfig, MB_ASCII_COLON);
      // And call the OnTx interrupt handler to send the other characters
      while (pIFaceProtAPI->pfOnTx(&_MBChannel) == 0); // will call _SendByte for each character
      _WriteMsg = 0;
    }
  }

  HandleTimeProc = 0;
}

/*********************************************************************
*
*       _InitTimerRTU()
*
* Function description
*   Initialize timer for UART RTU interface.
*
* Parameters
*   U32: Maximum frequency
*/
static void _InitTimerRTU(U32 MaxFreq) {
  _TimerId = timeSetEvent(POLL_FREQUENCY, 0, _TimeProcRTU, 0, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
  if (_TimerId == 0) {
    MB_Panic("Error creating the RTU timer");
  }
}

/*********************************************************************
*
*       _InitTimerASCII()
*
* Function description
*   Initialize timer for UART ASCII interface.
*
* Parameters
*   U32: Maximum frequency
*/
static void _InitTimerASCII(U32 MaxFreq) {
  _TimerId = timeSetEvent(POLL_FREQUENCY, 0, _TimeProcASCII, 0, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
  if (_TimerId == 0) {
    MB_Panic("Error creating the ASCII timer");
  }
}

/*********************************************************************
*
*       _DeInitTimer()
*
* Function description
*   Deinitialize timer for UART interface (both ASCII and RTU).
*/
static void _DeInitTimer(void) {
  timeKillEvent(_TimerId);
}


/****** End Of File *************************************************/
