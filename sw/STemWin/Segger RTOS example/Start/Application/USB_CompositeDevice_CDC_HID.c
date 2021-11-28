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
File    : USB_CompositeDevice_CDC_HID.c
Purpose : Sample showing a USB device with multiple interfaces (CDC+HID).
          The data received from PC over CDC and HID is echoed back.
          For HID you have to use the provided Windows sample HIDEcho1.exe
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <stdio.h>
#include "USB.h"
#include "USB_CDC.h"
#include "BSP.h"
#include "USB_HID.h"
#include "RTOS.h"

/*********************************************************************
*
*       Defines configurable
*
**********************************************************************
*/
#define INPUT_REPORT_SIZE   64    // Defines the input (device -> host) report size
#define OUTPUT_REPORT_SIZE  64    // Defines the output (Host -> device) report size
#define VENDOR_PAGE_ID      0x12  // Defines the vendor specific page that
                                  // shall be used, allowed values 0x00 - 0xff
                                  // This value must be identical to HOST application
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
  0x1119,         // ProductId
  "Vendor",       // VendorName
  "HID/CDC Composite device",  // ProductName
  "1234567890ABCDEF"           // SerialNumber
};

/*********************************************************************
*
*       Const data
*
**********************************************************************
*/

const U8 _aHIDReport[] = {
    0x06, VENDOR_PAGE_ID, 0xFF,    // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x06,                    //   USAGE_PAGE (Generic Device)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x95, INPUT_REPORT_SIZE,       //   REPORT_COUNT (64)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x06,                    //   USAGE_PAGE (Generic Device)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x95, OUTPUT_REPORT_SIZE,      //   REPORT_COUNT (64)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0xc0                           // END_COLLECTION
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
// Data for MSD Task
static OS_STACKPTR int      _aHIDStack[512]; // Task stacks
static OS_TASK              _HIDTCB;         // Task-control-blocks
static char                 _aHIDData[64];   // Data buffer.

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _AddHID
*
*  Function description
*    Adds the HID class to the stack.
*/
static USB_HID_HANDLE _AddHID(void) {
  static U8         _abOutBuffer[64];
  USB_HID_INIT_DATA InitData;
  U8                Interval = 1;
  USB_HID_HANDLE    hInst;

  memset(&InitData, 0, sizeof(InitData));
  InitData.EPIn    = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_INT, Interval, NULL, 0);
  InitData.EPOut   = USBD_AddEP(USB_DIR_OUT, USB_TRANSFER_TYPE_INT, Interval, _abOutBuffer, sizeof(_abOutBuffer));
  InitData.pReport = _aHIDReport;
  InitData.NumBytesReport  = sizeof(_aHIDReport);
  USBD_SetDeviceInfo(&_DeviceInfo);
  hInst = USBD_HID_Add(&InitData);
  return hInst;
}
/*********************************************************************
*
*       _HIDTask
*
*  Function description
*    This task will wait for a device to connect, then receive a single
*    byte, increase it's value by one and send it back to the host.
*/
static void _HIDTask(void * pParam) {
  USB_HID_HANDLE hInst;

  hInst = (USB_HID_HANDLE)pParam;
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
    USBD_HID_Read(hInst, &_aHIDData[0], OUTPUT_REPORT_SIZE, 0);
    _aHIDData[0]++;
    USBD_HID_Write(hInst, &_aHIDData[0], INPUT_REPORT_SIZE, 0);
    USB_OS_Delay(50);
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
  BSP_USE_PARA(pLineCoding);
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
  USB_HID_HANDLE hInstHID;

  USBD_Init();
  USBD_EnableIAD();
  USBD_SetDeviceInfo(&_DeviceInfo);
  hInstCDC = _AddCDC();
  hInstHID = _AddHID();
  USBD_Start();
  BSP_SetLED(0);
  OS_CREATETASK_EX(&_HIDTCB,  "HIDTask",  _HIDTask, 200, _aHIDStack, (void *)hInstHID);

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

