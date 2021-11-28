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
File    : USB_Printer.c
Purpose : Sample implementation of USB printer device class
----------Literature--------------------------------------------------
Universal Serial Bus Device Class Definition for Printing Devices
Version 1.1 January 2000
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "USB_PrinterClass.h"
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
  0x2114,         // ProductId. Should be unique for this sample
  "Vendor",               // VendorName
  "Printer",              // ProductName
  "12345678901234567890"  // SerialNumber
};

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static U8 _acData[512 + 1]; // +1 for the terminating zero character

/*********************************************************************
*
*       static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _GetDeviceIdString
*
*/
static const char * _GetDeviceIdString(void) {
  const char * s = "CLASS:PRINTER;"
                   "MODEL:HP LaserJet 6MP;"
                   "MANUFACTURER:Hewlett-Packard;"
                   "DESCRIPTION:Hewlett-Packard LaserJet 6MP Printer;"
                   "COMMAND SET:PJL,MLC,PCLXL,PCL,POSTSCRIPT;";
  return s;
}
/*********************************************************************
*
*       _GetHasNoError
*
*/
static U8 _GetHasNoError(void) {
  return 1;
}
/*********************************************************************
*
*       _GetIsSelected
*
*/
static U8 _GetIsSelected(void) {
  return 1;
}

/*********************************************************************
*
*       _GetIsPaperEmpty
*
*/
static U8 _GetIsPaperEmpty(void) {
  return 0;
}

/*********************************************************************
*
*       _OnDataReceived
*
*/
static int _OnDataReceived(const U8 * pData, unsigned NumBytes) {
  memcpy(_acData, pData, NumBytes);
  _acData[NumBytes] = 0;
  printf("%s", _acData);
  return 0;
}
/*********************************************************************
*
*       _OnReset
*
*/
static void _OnReset(void) {

}

static USB_PRINTER_API _PrinterAPI = {
  _GetDeviceIdString,
  _OnDataReceived,
  _GetHasNoError,
  _GetIsSelected,
  _GetIsPaperEmpty,
  _OnReset
};

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
  USBD_Init();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USB_PRINTER_Init(&_PrinterAPI);
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
    USB_PRINTER_Task();
  }
}




/**************************** end of file ***************************/

