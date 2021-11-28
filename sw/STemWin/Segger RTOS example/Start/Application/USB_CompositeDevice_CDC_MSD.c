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
File    : USB_CompositeDevice_CDC_MSD.c
Purpose : Sample showing a USB device with multiple interfaces (CDC+MSD).
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <stdio.h>
#include "USB.h"
#include "USB_CDC.h"
#include "BSP.h"
#include "USB_MSD.h"
#include "FS.h"
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
  0x1256,         // ProductId
  "Vendor",       // VendorName
  "MSD/CDC Composite device",  // ProductName
  "1234567890ABCDEF"           // SerialNumber
};
//
// String information used when inquiring the volume 0.
//
static const USB_MSD_LUN_INFO _Lun0Info = {
  "Vendor",     // MSD VendorName
  "MSD Volume", // MSD ProductName
  "1.00",       // MSD ProductVer
  "134657890"   // MSD SerialNo
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
// Data for MSD Task
static OS_STACKPTR int _aMSDStack[512]; /* Task stacks */
static OS_TASK _MSDTCB;               /* Task-control-blocks */

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _AddMSD
*
*  Function description
*    Add mass storage device to USB stack
*/
static void _AddMSD(void) {
  static U8 _abOutBuffer[USB_MAX_PACKET_SIZE];
  USB_MSD_INIT_DATA     InitData;
  USB_MSD_INST_DATA     InstData;

  InitData.EPIn  = USBD_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USBD_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, USB_MAX_PACKET_SIZE);
  USBD_MSD_Add(&InitData);
  //
  // Add logical unit 0: RAM drive, using SDRAM
  //
  memset(&InstData, 0,  sizeof(InstData));
  InstData.pAPI                    = &USB_MSD_StorageByName;
  InstData.DriverData.pStart       = (void *)"";
  InstData.pLunInfo = &_Lun0Info;
  USBD_MSD_AddUnit(&InstData);
}
/*********************************************************************
*
*       _MSDTask
*
*  Function description
*    Add mass storage device to USB stack
*/
static void _MSDTask(void) {
  while (1) {
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      USB_OS_Delay(50);
    }
    USBD_MSD_Task();
  }
}

/*********************************************************************
*
*       _OnLineCoding
*
*  Function description
*    Called whenever a "SetLineCoding" Packet has been received
*
*  Notes
*    (1) Context
*        This function is called directly from an ISR in most cases.
*/
static void _OnLineCoding(USB_CDC_LINE_CODING * pLineCoding) {
#if 0
  printf("DTERate=%u, CharFormat=%u, ParityType=%u, DataBits=%u\n",
          pLineCoding->DTERate,
          pLineCoding->CharFormat,
          pLineCoding->ParityType,
          pLineCoding->DataBits);
#else
  BSP_USE_PARA(pLineCoding);
#endif
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
  USBD_CDC_SetOnLineCoding(hInst, _OnLineCoding);
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
  USB_CDC_HANDLE hInstCDC;

  USBD_Init();
  USBD_EnableIAD();
  USBD_SetDeviceInfo(&_DeviceInfo);
  hInstCDC = _AddCDC();
  _AddMSD();
  USBD_Start();
  BSP_SetLED(0);
  OS_CREATETASK(&_MSDTCB,  "MSDTask",  _MSDTask, 200, _aMSDStack);

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
    NumBytesReceived = USBD_CDC_Receive(hInstCDC, &ac[0], sizeof(ac), 0);
    if (NumBytesReceived > 0) {
      USBD_CDC_Write(hInstCDC, &ac[0], NumBytesReceived, 0);
    }
  }
}

/**************************** end of file ***************************/

