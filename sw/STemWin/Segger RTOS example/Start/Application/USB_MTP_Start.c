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
File    : USB_MTP_Start.c
Purpose : Sample application for USB MTP (Media Transfer Protocol).
--------  END-OF-HEADER  ---------------------------------------------
*/


#include <stdio.h>
#include <string.h>
#include "USB.h"
#include "USB_MTP.h"
#include "FS.h"
#include "BSP.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define NUM_BYTES_OBJECT_LIST     (4 * 1024)
#define NUM_BYTES_DATA_BUFFER     (4 * USB_MAX_PACKET_SIZE)
#define NUM_BYTES_RECEIVE_BUFFER  (2 * USB_MAX_PACKET_SIZE)

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
  0x8765,           // VendorId
  0x1001,           // ProductId
  "Vendor",         // VendorName
  "MTP device",     // ProductName
  "000013245678"    // SerialNumber. Should be 12 character or more for compliance with Mass Storage Device Bootability spec.
};
//
// MTP device information.
//
static USB_MTP_INFO _MTPInfo = {
  "Vendor",         // Manufacturer
  "Storage device", // Model
  "1.0",            // DeviceVersion
  "0123456789ABCDEF0123456789ABCDEF" // SerialNumber. It must be exactly 32 characters long.
};
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8  _acReceiveBuffer[NUM_BYTES_RECEIVE_BUFFER];    // Receive buffer for the USB device
static U32 _aDataBuffer[NUM_BYTES_DATA_BUFFER / 4];       // Send/receive buffer for MTP transactions
static U32 _aObjectList[NUM_BYTES_OBJECT_LIST / 4];       // Memory pool for the MTP object list

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _FSTest
*/
static void _FSTest(void) {
  FS_FILE    * pFile;
  unsigned     NumBytes;
  const char * sInfo = "This sample is based on the Segger emUSB software with the MTP component.\r\nFor further information please visit: www.segger.com\r\n";
  unsigned     NumVolumes;
  unsigned     i;
  char         acVolumeName[32];

  NumBytes   = strlen(sInfo);
  NumVolumes = FS_GetNumVolumes();
  for (i = 0; i < NumVolumes; i++) {
    FS_GetVolumeName(i, &acVolumeName[0], sizeof(acVolumeName));
    if (FS_IsLLFormatted(acVolumeName) == 0) {
      FS_X_Log("Low-level format\n");
      FS_FormatLow(acVolumeName);         // Erase and low-level format the volume.
    }
    if (FS_IsHLFormatted(acVolumeName) == 0) {
      FS_X_Log("High-level formatting\n");
      FS_Format(acVolumeName, NULL);      // High-level format the volume.
    }
    strcat(acVolumeName, "\\Readme.txt");
    pFile = FS_FOpen(acVolumeName, "w");
    if (pFile) {
      FS_Write(pFile, sInfo, NumBytes);
      FS_FClose(pFile);
    }
  }
}

/*********************************************************************
*
*       _AddMTP
*
*  Function description
*    Adds MTP component to USB stack. This is a 2 step operation.
*    First the MTP component is added to USB stack. Then the one or more
*    storage drivers are added to MTP component.
*
*  Notes
*     (1) The interrupt endpoint is used to report events to USB host.
*         It must be allocated even when the device does not support events.
*/
static void _AddMTP(void) {
  USB_MTP_INIT_DATA InitData;
  USB_MTP_INST_DATA InstData;

  //
  // Add the MTP component to USB stack.
  //
  InitData.EPIn                = USBD_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut               = USBD_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _acReceiveBuffer, sizeof(_acReceiveBuffer));
  InitData.EPInt               = USBD_AddEP(1, USB_TRANSFER_TYPE_INT, 10, NULL, 0);    // Note 1
  InitData.pObjectList         = _aObjectList;
  InitData.NumBytesObjectList  = sizeof(_aObjectList);
  InitData.pDataBuffer         = _aDataBuffer;
  InitData.NumBytesDataBuffer  = sizeof(_aDataBuffer);
  InitData.pMTPInfo            = &_MTPInfo;
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_MTP_Add(&InitData);
  //
  // Add a storage driver to MTP component.
  //
  InstData.pAPI                = &USB_MTP_StorageFS;
  InstData.sDescription        = "MTP volume";
  InstData.sVolumeId           = "0123456789";
  InstData.DriverData.pRootDir = "";
  USBD_MTP_AddStorage(&InstData);
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
  FS_Init();
  FS_FAT_SupportLFN();
  _FSTest();
  _AddMTP();
  USBD_Start();
  while (1) {
    //
    // Wait for the host to configure the USB device.
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);
    USBD_MTP_Task();
  }
}

/**************************** end of file ***************************/
