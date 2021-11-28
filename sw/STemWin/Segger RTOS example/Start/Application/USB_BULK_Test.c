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
File    : USB_BULK_Test.c
Purpose : This sample application is to be used with it's Windows
          counterpart "Test.exe".
          The sample sends/receives different packet sizes and
          measures the transfer speed.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "USB_Bulk.h"
#include "BSP.h"
#include <stdio.h>

static U8 _ac[0x400];

/*********************************************************************
*
*       Information that are used during enumeration
*/
static const USB_DEVICE_INFO _DeviceInfo = {
  0x8765,         // VendorId
  0x1234,         // ProductId
  "Vendor",       // VendorName
  "Bulk device",  // ProductName
  "13245678"      // SerialNumber
};

/*********************************************************************
*
*       _AddBULK
*
*  Function description
*    Add generic USB BULK interface to USB stack
*/
static USB_BULK_HANDLE _AddBULK(void) {
  static U8             _abOutBuffer[USB_MAX_PACKET_SIZE * 2];
  USB_BULK_INIT_DATA    InitData;
  USB_BULK_HANDLE       hInst;

  InitData.EPIn  = USBD_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USBD_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, sizeof(_abOutBuffer));
  hInst = USBD_BULK_Add(&InitData);
  return hInst;
}

/*********************************************************************
*
*       public code
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

  USBD_Init();
  hInst = _AddBULK();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  while (1) {
    unsigned char c;

    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0); // Toggle LED to indicate configuration
      USB_OS_Delay(50);
    }
    BSP_ClrLED(0);
    //
    // Loop: Receive data byte by byte, send back (data + 1)
    //
    USBD_BULK_Read(hInst, &c, 1, 0);
    if (c > 0x10) {
      c++;
      USBD_BULK_Write(hInst, &c, 1, 0);
    } else {
      int NumBytes = c << 8;
      USBD_BULK_Read(hInst, &c, 1, 0);
      NumBytes |= c;
      if (NumBytes <= (int)sizeof(_ac)) {
        USBD_BULK_Read(hInst, _ac, NumBytes, 0);
        USBD_BULK_Write(hInst, _ac, NumBytes, 0);
      } else {
        int i;
        int NumBytesAtOnce;
        for (i = 0; i < NumBytes; i += NumBytesAtOnce) {
          NumBytesAtOnce = NumBytes - i;
          if (NumBytesAtOnce > (int)sizeof(_ac)) {
            NumBytesAtOnce = sizeof(_ac);
          }
          USBD_BULK_Read(hInst, _ac, NumBytesAtOnce, 0);
          USBD_BULK_Write(hInst, _ac, NumBytesAtOnce, 0);
        }
      }
    }
  }
}
