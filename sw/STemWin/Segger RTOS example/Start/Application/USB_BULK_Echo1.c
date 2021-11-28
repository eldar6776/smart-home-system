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
File    : USB_BULK_Echo1.c
Purpose : USB BULK sample showing a simple 1-byte echo server.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include "USB_Bulk.h"
#include "BSP.h"

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
  USBD_Init();
  hInst = _AddBULK();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  //
  // Loop: Receive data byte by byte, send back (data + 1)
  //
  while (1) {
    char c;

    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);         // LED on to indicate we are waiting for data
    USBD_BULK_Read(hInst, &c, 1, 0);
    BSP_ClrLED(0);
    c++;
    USBD_BULK_Write(hInst, &c, 1, 0);
  }
}
