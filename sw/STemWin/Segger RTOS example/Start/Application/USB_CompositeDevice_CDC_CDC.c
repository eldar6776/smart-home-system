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
File    : USB_CompositeDevice_CDC_CDC.c
Purpose : Sample showing a USB device with multiple interfaces
          (2 CDC class).
          The data received from PC (com ports) over CDC are
          echoed back.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <stdio.h>
#include "USB_Bulk.h"
#include "USB_CDC.h"
#include "BSP.h"
#include "RTOS.h"

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/
//
//  Information that is used during enumeration.
//
static const USB_DEVICE_INFO _DeviceInfo = {
  0x8765,         // VendorId
  0x1120,         // ProductId
  "Vendor",       // VendorName
  "CDC/CDC Composite device",  // ProductName
  "1234567890ABCDEF"           // SerialNumber
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
// Data for MSD Task
static OS_STACKPTR int      _a2ndCDCStack[128]; /* Task stacks */
static OS_TASK              _2ndCDCTCB;         /* Task-control-blocks */

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _2ndCDCTask
*
*  Function description
*    Task to make the mouse jump from left to right.
*/
static void _2ndCDCTask(void * pParam) {
  USB_CDC_HANDLE hInst;
  char           ac[64];
  int            NumBytesReceived;

  hInst = (USB_CDC_HANDLE)pParam;
  //
  // Loop: Receive data byte by byte, send back (data + 1)
  //
  while (1) {
    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(1);
      USB_OS_Delay(50);
    }
    BSP_SetLED(1);
    NumBytesReceived = USBD_CDC_Receive(hInst, &ac[0], sizeof(ac), 0);
    if (NumBytesReceived > 0) {
      USBD_CDC_Write(hInst, &ac[0], NumBytesReceived, 0);
    }
    BSP_ClrLED(1);
  }
}

/*********************************************************************
*
*       _AddCDC
*
*  Function description
*    Add communication device class to USB stack
*/
static USB_CDC_HANDLE _AddCDC(void) {
  static U8 _abOutBuffer[USB_MAX_PACKET_SIZE];
  USB_CDC_INIT_DATA     InitData;
  USB_CDC_HANDLE        hInst;

  InitData.EPIn  = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_BULK, 0, NULL, 0);
  InitData.EPOut = USBD_AddEP(USB_DIR_OUT, USB_TRANSFER_TYPE_BULK, 0, _abOutBuffer, USB_MAX_PACKET_SIZE);
  InitData.EPInt = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_INT,  8,  NULL, 0);
  hInst = USBD_CDC_Add(&InitData);
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
* USB handling task.
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
  USB_CDC_HANDLE hInst1;
  USB_CDC_HANDLE hInst2;

  USBD_Init();
  USBD_EnableIAD();
  USBD_SetDeviceInfo(&_DeviceInfo);
  hInst1 = _AddCDC();
  hInst2 = _AddCDC();
  USBD_Start();
  BSP_SetLED(0);
  OS_CREATETASK_EX(&_2ndCDCTCB,  "2ndCDCTask",  _2ndCDCTask, 200, _a2ndCDCStack, (void *)hInst2);
  while (1) {
    char ac[64];
    int  NumBytesReceived;

    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);
    NumBytesReceived = USBD_CDC_Receive(hInst1, &ac[0], sizeof(ac), 0);
    if (NumBytesReceived > 0) {
      USBD_CDC_Write(hInst1, &ac[0], NumBytesReceived, 0);
    }
    BSP_ClrLED(0);
  }
}

/**************************** end of file ***************************/
