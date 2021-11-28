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
File    : USB_BULK_Performance.c
Purpose : This sample application is to be used with it's Windows
          counterpart "Performance.exe".
          The sample includes two tests which can be performed.
          The performance test shows the read and write transfer
          speed. The stability test sends packets of different sizes
          and with increasing delays between the packets to verify
          that the communication can handle all packet forms.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include "SEGGER.h"
#include "USB_Bulk.h"
#include "USB_Private.h"
#include "USB_Bulk.h"
#include "BSP.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define MAX_PACKET_SIZE      USB__IsHighSpeedMode() ? (512) : (64)

#define MAX_BYTES            (5 * MAX_PACKET_SIZE)
#define NUM_BYTES_BUFFER     0x2000

/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/
#define CMD_TEST_SPEED         0x01
#define CMD_TEST               0x02
#define SUBCMD_SPEED_READ       100
#define SUBCMD_SPEED_WRITE      101

#define MAX_NUM_BYTES_TEST_NET    (4100)

#define USB_BULK_ENDWRITE()

/*********************************************************************
*
*       Information that are used during enumeration
*/
static const USB_DEVICE_INFO _DeviceInfo = {
  0x8765,         // VendorId
  0x1234,         // ProductId
  "Vendor",       // VendorName
  "Bulk device",  // ProductName
  "1324567890"    // SerialNumber
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32 _aMemBlock[NUM_BYTES_BUFFER / 4];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _AddBULK
*
*  Function description
*    Add generic USB BULK interface to USB stack
*/
static USB_BULK_HANDLE _AddBULK(void) {
  static U8             _abOutBuffer[USB_MAX_PACKET_SIZE];
  USB_BULK_INIT_DATA    InitData;
  USB_BULK_HANDLE       hInst;

  InitData.EPIn  = USBD_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USBD_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, USB_MAX_PACKET_SIZE);
  hInst = USBD_BULK_Add(&InitData);
  return hInst;
}


/*********************************************************************
*
*       _ReadDummy
*
*  Function description
*    Reads data block via USB or ethernet, but simply ignores data
*
*  Return value
*    0     OK
*   -1     Error
*/
static int _ReadDummy(USB_BULK_HANDLE hInst, unsigned NumBytes) {
  U32 NumBytesAtOnce;
  int r;
  U8 abData[32];      // Dummy buffer

  do {
    NumBytesAtOnce = SEGGER_MIN(sizeof(abData), NumBytes);
    r = USBD_BULK_Read(hInst, abData, NumBytesAtOnce, 0);
    if (r) {
      return r;
    }
    NumBytes -= NumBytesAtOnce;
  } while (NumBytes);
  return 0;
}

/*********************************************************************
*
*       _WriteDummy
*
*  Function description
*    This function writes 0 data. Used in some cases to make sure we send the number of bytes expected to stay in-sync
*/
static void _WriteDummy(USB_BULK_HANDLE hInst, unsigned NumBytes) {
  U32 NumBytesAtOnce;
  U8  abData[32] = {0};

  do {
    NumBytesAtOnce = SEGGER_MIN(sizeof(abData), NumBytes);
    USBD_BULK_Write(hInst, abData, NumBytesAtOnce, 0);
    NumBytes -= NumBytesAtOnce;
  } while (NumBytes);
}

/*********************************************************************
*
*       _Delay
*
*/
static unsigned _Delay(volatile unsigned NumCycles) {
  do {} while (--NumCycles);
  return NumCycles;
}

/*********************************************************************
*
*       _Alloc
*
*/
static void * _Alloc(unsigned NumBytes) {
  if (NumBytes <= NUM_BYTES_BUFFER) {
    return _aMemBlock;
  }
  return NULL;
}

/*********************************************************************
*
*       _GetMaxBlock
*
*/
static unsigned _GetMaxBlock(void) {
  return sizeof(_aMemBlock);
}


/*********************************************************************
*
*       _Free
*
*/
static void _Free(void * p) {
  (void)p;
}


/*********************************************************************
*
*       _PerformanceTest
*
*  Function description:
*
*
*/
static void _TestNet(USB_BULK_HANDLE hInst) {
  U8  acIn[4];
  U8  SubCmd;
  int NumBytes;
  int NumBytesAtOnce;
  int Off;
  int OffNew;
  int NumBytesToSend;
  U8  r = 0;
  U8* pData;
  unsigned DelayusPerByte;
  //
  // Read the number of bytes
  //
  USBD_BULK_Read(hInst, acIn, 3, 0);
  SubCmd   = acIn[0];
  NumBytes = acIn[1] + (acIn[2] << 8);
  DelayusPerByte = (1 << (SubCmd & 15)) >> 1;
  //
  // Reserve space for data + one byte for status
  //
  pData = (U8*)_Alloc(NumBytes + 1);
  if (pData == NULL) {
    _ReadDummy(hInst, NumBytes);
    _WriteDummy(hInst, NumBytes + 1);
    NumBytesAtOnce = NumBytes;
    return;
  }
  //
  // Perform operation depending on transfer type
  //
  if ((SubCmd >> 4) == 0) {
    //
    // Read data overlapped
    //
    USBD_BULK_ReadOverlapped(hInst, pData, NumBytes);
    Off = 0;
    do {
      //
      // Wait until a sufficient number of bytes has been received
      //
      do {
        int NumBytesRem = USBD_BULK_GetNumBytesRemToRead(hInst);
        OffNew = NumBytes - NumBytesRem;
        NumBytesAtOnce = OffNew - Off;
        if (NumBytesRem == 0) {
          break;
        }
        if (NumBytesAtOnce >= MAX_BYTES) {
          NumBytesAtOnce &= ~(MAX_PACKET_SIZE  - 1);
          break;
        }
      } while (1);
      //
      // Delay the transfer to simulate JTAG transaction
      //
      if (DelayusPerByte) {
        USB_OS_Delay((NumBytesAtOnce * DelayusPerByte) >> 10);
      }
      r = 0;
      //
      // Send data back via USB (use same data as received)
      //
      NumBytesToSend = NumBytesAtOnce;
      //
      // Send status byte on last transfer
      //
      if (Off + NumBytesAtOnce == NumBytes) {   // Last transfer ?
        *(pData + Off + NumBytesAtOnce) = r;
        NumBytesToSend++;
      }
      USBD_BULK_Write(hInst, pData + Off, NumBytesToSend, -1);
      Off += NumBytesAtOnce;
    } while (Off != NumBytes);
    //
    // End USB transaction
    //
    USB_BULK_ENDWRITE();
  } else {
    //
    // Read data non-overlapped
    //
    USBD_BULK_Read(hInst, pData, NumBytes, 0);
    //
    // Delay the transfer to simulate work
    //
    if (DelayusPerByte) {
      _Delay(NumBytes * DelayusPerByte);
    }
    r = 0;
    //
    // Send data back via USB (use same data as received)
    //
    *(pData + NumBytes) = r;
    USBD_BULK_Write(hInst, pData, NumBytes + 1, 0);
  }
  //
  // Free the reserved memory
  //
  _Free(pData);
}

/*********************************************************************
*
*       _TestSpeed
*/
static void _TestSpeed(USB_BULK_HANDLE hInst) {
  U8  SubCmd;
  U32 NumReps;
  U32 NumBytes;
  U32 NumBytesRem;
  U32 NumBytesAtOnce;
  U32 AllocSize;
  int r;
  U8* p;
  U8  aData[8];

  USBD_BULK_Read(hInst, &SubCmd, 1, 0);
  USBD_BULK_Read(hInst, aData, 8, 0);
  //
  //  Get the command parameters
  //
  NumReps  = USB__GetU32LE(&aData[0]);
  NumBytes = USB__GetU32LE(&aData[4]);
  //
  // Check for valid parameters
  //
  if (NumBytes == 0) {
    return;
  }
  if ((NumReps | NumBytes) == 0) {
    return;
  }
  //
  //  Allocate a buffer in order to store or retrieve the data
  //
  AllocSize = NumBytes;
  if (AllocSize > _GetMaxBlock()) {
    AllocSize = _GetMaxBlock();
  }
  p = (U8*)_Alloc(AllocSize);
  if (p == NULL) {
    return;
  }
  //
  // Repeat the speed test acc. to NumReps.
  //
  do {
    NumBytesRem = NumBytes;
    //
    // Check which direction and execute command accordingly
    switch (SubCmd) {
    case SUBCMD_SPEED_WRITE:
      do {
        NumBytesAtOnce = SEGGER_MIN(NumBytesRem, AllocSize);
        NumBytesRem -= NumBytesAtOnce;
        r = USBD_BULK_Read(hInst, p, NumBytesAtOnce, 0);
        if (r < 0) {
          return;
        }
      } while (NumBytesRem);
      break;
    case SUBCMD_SPEED_READ:
      do {
        NumBytesAtOnce = SEGGER_MIN(NumBytesRem, AllocSize);
        NumBytesRem -= NumBytesAtOnce;
        USBD_BULK_Write(hInst, p, NumBytesAtOnce, 0);
      } while (NumBytesRem);
      break;
    default:
      return;
    }
  } while (--NumReps);
  //
  // Free the buffer
  //
  _Free(p);
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       MainTask
*
* Function description
*   USB handling task.
*   Modify to implement the desired protocol
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif
void MainTask(void) {
  USB_BULK_HANDLE hInst;
  U8 Cmd;
  int r;

  USBD_Init();
  hInst = _AddBULK();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  //
  // Loop: Receive data byte by byte, send back (data + 1)
  //
  while (1) {
    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);         // LED on to indicate we are waiting for data
    r = USBD_BULK_Read(hInst, &Cmd, 1, 0);
    if (r > 0) {
      switch (Cmd) {
      case CMD_TEST_SPEED:
        _TestSpeed(hInst);
        break;
      case CMD_TEST:
        _TestNet(hInst);
        break;
      }
    }
  }
}
