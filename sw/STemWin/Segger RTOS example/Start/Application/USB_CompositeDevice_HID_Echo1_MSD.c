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
File    : USB_CompositeDevice_HID_Echo1_MSD.c
Purpose : Sample showing a USB device with multiple interfaces (HID+MSD).
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "USB.h"
#include "USB_MSD.h"
#include "USB_HID.h"
#include "BSP.h"
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
  0x4400,         // ProductId. Should be unique for this sample
  "Vendor",       // VendorName
  "HID Echo1/MSD  sample",  // ProductName
  "12345678"      // SerialNumber
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
static char _ac[64];
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
*       _AddHID
*
*  Function description
*    Add HID class to USB stack
*/
static USB_HID_HANDLE _AddHID(void) {
  static U8           _abOutBuffer[64];
  USB_HID_INIT_DATA   InitData;
  U8                  Interval = 1;
  USB_HID_HANDLE      hInst;

  memset(&InitData, 0, sizeof(InitData));
  InitData.EPIn    = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_INT, Interval, NULL, 0);
  InitData.EPOut   = USBD_AddEP(USB_DIR_OUT, USB_TRANSFER_TYPE_INT, Interval, _abOutBuffer, USB_MAX_PACKET_SIZE);
  InitData.pReport = _aHIDReport;
  InitData.NumBytesReport  = sizeof(_aHIDReport);
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
  USB_HID_HANDLE hInstHID;

  USBD_Init();
  hInstHID = _AddHID();
  _AddMSD();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  OS_CREATETASK(&_MSDTCB,  "MSDTask",  _MSDTask, 200, _aMSDStack);
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
    BSP_ClrLED(0);
    USBD_HID_Read(hInstHID, &_ac[0], OUTPUT_REPORT_SIZE, 0);
    _ac[0]++;
    USBD_HID_Write(hInstHID, &_ac[0], INPUT_REPORT_SIZE, 0);
    USB_OS_Delay(50);
  }
}
