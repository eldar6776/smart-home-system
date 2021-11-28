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
File    : USB_HID_Mouse.c
Purpose : Demonstrates usage of the HID component of the USB stack as mouse.
          Makes the mouse jump left & right.
--------  END-OF-HEADER  ---------------------------------------------
*/


#include <string.h>
#include <stdio.h>
#include "USB.h"
#include "USB_HID.h"
#include "BSP.h"

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
  0x1112,         // ProductId
  "Vendor",       // VendorName
  "HID mouse sample",  // ProductName
  "12345678"      // SerialNumber
};

/*********************************************************************
*
*       _aHIDReport
*
*  This report is generated according to HID spec and
*  HID Usage Tables specifications.
*/
const U8 _aHIDReport[] = {
  USB_HID_GLOBAL_USAGE_PAGE + 1, USB_HID_USAGE_PAGE_GENERIC_DESKTOP,
  USB_HID_LOCAL_USAGE + 1, USB_HID_USAGE_MOUSE,
  USB_HID_MAIN_COLLECTION + 1, USB_HID_COLLECTION_APPLICATION,
    USB_HID_LOCAL_USAGE + 1, USB_HID_USAGE_POINTER,
    USB_HID_MAIN_COLLECTION + 1, USB_HID_COLLECTION_PHYSICAL,
      USB_HID_GLOBAL_USAGE_PAGE + 1, USB_HID_USAGE_PAGE_BUTTON,
      USB_HID_LOCAL_USAGE_MINIMUM + 1, 1,
      USB_HID_LOCAL_USAGE_MAXIMUM + 1, 3,
      USB_HID_GLOBAL_LOGICAL_MINIMUM + 1, 0,
      USB_HID_GLOBAL_LOGICAL_MAXIMUM + 1, 1,
      USB_HID_GLOBAL_REPORT_COUNT + 1, 3,
      USB_HID_GLOBAL_REPORT_SIZE + 1, 1,
      USB_HID_MAIN_INPUT + 1, USB_HID_VARIABLE,  // 3 button bits
      USB_HID_GLOBAL_REPORT_COUNT + 1, 1,
      USB_HID_GLOBAL_REPORT_SIZE + 1, 5,
      USB_HID_MAIN_INPUT + 1, USB_HID_CONSTANT,  // 5 bit padding
      USB_HID_GLOBAL_USAGE_PAGE + 1, USB_HID_USAGE_PAGE_GENERIC_DESKTOP,
      USB_HID_LOCAL_USAGE + 1, USB_HID_USAGE_X,
      USB_HID_LOCAL_USAGE + 1, USB_HID_USAGE_Y,
      USB_HID_GLOBAL_LOGICAL_MINIMUM + 1, (unsigned char) -127,
      USB_HID_GLOBAL_LOGICAL_MAXIMUM + 1, 127,
      USB_HID_GLOBAL_REPORT_SIZE + 1, 8,
      USB_HID_GLOBAL_REPORT_COUNT + 1, 2,
      USB_HID_MAIN_INPUT + 1, USB_HID_VARIABLE | USB_HID_RELATIVE,
    USB_HID_MAIN_ENDCOLLECTION,
  USB_HID_MAIN_ENDCOLLECTION
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8 _IntervalTime;

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
*    Add HID mouse class to USB stack
*/
static USB_HID_HANDLE _AddHID(void) {
  USB_HID_INIT_DATA InitData;
  USB_HID_HANDLE    hInst;

  memset(&InitData, 0, sizeof(InitData));
  InitData.EPIn    = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_INT, _IntervalTime, NULL, 0);
  InitData.pReport = _aHIDReport;
  InitData.NumBytesReport = sizeof(_aHIDReport);
  USBD_SetDeviceInfo(&_DeviceInfo);
  hInst = USBD_HID_Add(&InitData);
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
  U8             ac[3];
  USB_HID_HANDLE hInst;

  _IntervalTime = 10;   // We set a interval time of 10 ms
  USBD_Init();
  hInst = _AddHID();
  USBD_Start();
  while (1) {

    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_ClrLED(0);
    memset (ac, 0, sizeof(ac));
    ac[1] = 20;   // To the left !
    USBD_HID_Write(hInst, &ac[0], 3, 0);      // Make sure we send the number of bytes defined in REPORT
    USB_OS_Delay(500);
    ac[1] = (U8)-20;  // To the right !
    USBD_HID_Write(hInst, &ac[0], 3, 0);      // Make sure we send the number of bytes defined in REPORT
    USB_OS_Delay(100);

  }
}

/**************************** end of file ***************************/

