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
File    : USB_MSD_FS_MT_Start.c
Purpose : Sample startup, storage driver for MSD using multitasking.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "USB.h"
#include "USB_MSD.h"
#include "FS_Int.h"
#include "BSP.h"
#include "RTOS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define NUM_VOLUMES     2           // Number of volumes to be used as MSD
#define NUM_BUFFERS     2           // Number of transfer buffers, it has to be at least 2,
                                    // otherwise the behaviour is undefined.
#define BUFFER_SIZE     (NUM_BUFFERS * ((8 * 1024) + USB_MSD_MT_WRITE_INFO_SIZE)) // Size of a transfer buffer in bytes

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
// Array to store information used when inquiring the volumes.
//
static USB_MSD_LUN_INFO _LunInfo[NUM_VOLUMES];

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_STACKPTR int _aStackStorageTask[1536/sizeof(int)];
static OS_TASK         _TCBStorageTask;
static U32             _aBuffer[NUM_VOLUMES][BUFFER_SIZE / 4];

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
  unsigned     Len;
  const char * sInfo = "This sample based on the Segger emUSB software with MSD component.\r\nFor further information please visit: www.segger.com\r\n";
  unsigned     NumVolumes;
  unsigned     iVolume;
  char         acVolumeName[20];

  Len        = strlen(sInfo);
  NumVolumes = FS_GetNumVolumes();
  for (iVolume = 0; iVolume < NumVolumes; ++iVolume) {
    FS_GetVolumeName(iVolume, &acVolumeName[0], sizeof(acVolumeName));
    if (FS_IsLLFormatted(acVolumeName) == 0) {
      FS_X_Log("Low level formatting");
      FS_FormatLow(acVolumeName);
    }
    if (FS_IsHLFormatted(acVolumeName) == 0) {
      FS_X_Log("High level formatting\n");
      FS_Format(acVolumeName, NULL);
    }
    strcat(acVolumeName, "\\Readme.txt");
    pFile = FS_FOpen(acVolumeName, "w");
    FS_Write(pFile, sInfo, Len);
    FS_FClose(pFile);
    FS_SetVolumeLabel(acVolumeName, "SEGGER");
    FS_Unmount(acVolumeName);
  }
}

/*********************************************************************
*
*       _AddMSD
*
*   Function description
*     Add one ore more mass storage devices to USB stack.
*/
static void _AddMSD(void) {
  static U8         _abOutBuffer[2 * USB_MAX_PACKET_SIZE];
  USB_MSD_INIT_DATA InitData;
  USB_MSD_INST_DATA InstData;
  int               NumVolumes;
  int               iVolume;
  char              acVolumeName[20];

  //
  // Allocated 1 in and 1 out end point for the MSD module.
  //
  InitData.EPIn  = USBD_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USBD_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, sizeof(_abOutBuffer));
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_MSD_Add(&InitData);
  //
  // For each file system module create a MSD instance.
  //
  NumVolumes = FS_GetNumVolumes();
  NumVolumes = MIN(NumVolumes, NUM_VOLUMES);
  for (iVolume = 0; iVolume < NumVolumes; ++iVolume) {
    //
    // Create the MSD unit.
    //
    FS_GetVolumeName(iVolume, &acVolumeName[0], sizeof(acVolumeName));
    memset(&InstData, 0,  sizeof(InstData));
    InstData.pAPI              = &USB_MSD_StorageMT;
    InstData.DriverData.pStart = (void *)acVolumeName;
    InstData.DriverData.StartSector     = 0;
    InstData.DriverData.NumBytes4Buffer = BUFFER_SIZE;
    InstData.DriverData.NumBuffers      = NUM_BUFFERS;
    InstData.DriverData.pSectorBuffer   = _aBuffer[iVolume];
    _LunInfo[iVolume].pVendorName       = "Vendor";
    _LunInfo[iVolume].pProductName      = "MSD Volume";
    _LunInfo[iVolume].pProductVer       = "1.00";
    _LunInfo[iVolume].pSerialNo         = "134657890";
    InstData.pLunInfo                   = &_LunInfo[iVolume];
    USBD_MSD_AddUnit(&InstData);
  }
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
  U8 Prio;

  USBD_Init();
  FS_Init();
  _FSTest();
  _AddMSD();
  //
  // Create a task that handles the writes sectors of the MSD stack
  //
  Prio = OS_GET_PRIORITY(OS_Global.pTask) + 1;
  USBD_MSD_Storage_MTInit();
  OS_CREATETASK(&_TCBStorageTask, "StorageTask", USBD_MSD_StorageTask, Prio, _aStackStorageTask);
  USBD_Start();
  while (1) {
    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);
    USBD_MSD_Task();
  }
}

/**************************** end of file ***************************/

