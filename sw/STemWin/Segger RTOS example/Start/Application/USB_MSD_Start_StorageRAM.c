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
File    : USB_MSD_Start_StorageRAM.c
Purpose : Sample startup, using a simple RAM disk driver.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <string.h>
#include <stdio.h>
#include "USB.h"
#include "USB_MSD.h"
#include "BSP.h"


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#ifndef   MSD_RAM_NUM_SECTORS
  #define MSD_RAM_NUM_SECTORS  46
#endif

#ifndef   MSD_RAM_SECTOR_SIZE
  #define MSD_RAM_SECTOR_SIZE  512
#endif

#ifndef MSD_RAM_ADDR
  static U8 _aRAMDisk[MSD_RAM_SECTOR_SIZE * MSD_RAM_NUM_SECTORS];  // RAM disk must be at least 23 KByte otherwise Windows host cannot format the disk.
  #define MSD_RAM_ADDR &_aRAMDisk[0]
#endif

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/
//
// Information that is used during enumeration.
//
static const USB_DEVICE_INFO _DeviceInfo = {
  0x8765,         // VendorId
  0x1000,         // ProductId
  "Vendor",       // VendorName
  "MSD device",   // ProductName
  "000013245678"  // SerialNumber. Should be 12 character or more for compliance with Mass Storage Device Bootability spec.
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
*
*  Note:
*   (1)  -     The members of the DriverData are used as follows:
*                DriverData.pStart       = Start address of the RAM disk.
*                DriverData.NumSectors   = Number of sectors to be used.
*                DriverData.StartSector  = Is ignored.
*                DriverData.SectorSize   = Bytes per sector that shall be used.
*
*/
static void _AddMSD(void) {
  static U8 _abOutBuffer[USB_MAX_PACKET_SIZE];
  USB_MSD_INIT_DATA     InitData;
  USB_MSD_INST_DATA     InstData;

  InitData.EPIn  = USBD_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USBD_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, USB_MAX_PACKET_SIZE);
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_MSD_Add(&InitData);
  //
  // Add logical unit 0: RAM drive, using SDRAM
  //
  memset(&InstData, 0,  sizeof(InstData));
  InstData.pAPI                    = &USB_MSD_StorageRAM;
  InstData.DriverData.pStart       = (void*)MSD_RAM_ADDR;
  InstData.DriverData.NumSectors   = MSD_RAM_NUM_SECTORS;
  InstData.DriverData.SectorSize   = MSD_RAM_SECTOR_SIZE;
  InstData.pLunInfo = &_Lun0Info;
  USBD_MSD_AddUnit(&InstData);
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
  _AddMSD();
  USBD_Start();
  while (1) {
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);
    USBD_MSD_Task();
  }
}

/**************************** end of file ***************************/

