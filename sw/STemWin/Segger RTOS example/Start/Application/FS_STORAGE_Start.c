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
File    : FS_STORAGE_Start.c
Purpose : Start application for the storage layer of the file system.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "FS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define VOLUME_NAME         ""
#define MAX_SECTOR_SIZE     2048
#define SECTOR_INDEX        0

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32 _aBuffer[MAX_SECTOR_SIZE / 4];

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
  int           r;
  FS_DEV_INFO   DevInfo;
  U32         * pData32;
  U32           Cnt;
  U32           NumLoops;
  U32           BytesPerSector;

  printf("Start\n");
  //
  // Initialize file system.
  //
  FS_STORAGE_Init();
  //
  // Perform a low-level format if required.
  //
  if (FS_IsLLFormatted(VOLUME_NAME) == 0) {
    r = FS_FormatLow(VOLUME_NAME);
    if (r < 0) {
      printf("Low-level format failed.\n");
      goto Done;
    }
    printf("Low-level format\n");
  }
  //
  // Get and show information about the storage device.
  //
  memset(&DevInfo, 0, sizeof(DevInfo));
  r = FS_STORAGE_GetDeviceInfo(VOLUME_NAME, &DevInfo);
  if (r) {
    printf("Could not get device info.\n");
    goto Done;
  }
  BytesPerSector = DevInfo.BytesPerSector;
  printf("Device info:\n");
  printf("  Volume name:       \"%s\"\n",    VOLUME_NAME);
  printf("  Number of sectors: %lu\n",       DevInfo.NumSectors);
  printf("  Sector size:       %lu bytes\n", BytesPerSector);
  //
  // Check if the sector buffer is large enough.
  //
  if (BytesPerSector > sizeof(_aBuffer)) {
    printf("Sector buffer too small.\n");
    goto Done;
  }
  //
  // Write some data to a sector, read it back and verify it.
  //
  printf("Write, read and verify sector %lu\n", (long unsigned)SECTOR_INDEX);
  NumLoops = DevInfo.BytesPerSector >> 2;
  pData32  = _aBuffer;
  Cnt      = 0;
  do {
    *pData32++ = Cnt++;
  } while (--NumLoops);
  r = FS_STORAGE_WriteSector(VOLUME_NAME, _aBuffer, SECTOR_INDEX);
  if (r) {
    printf("Could not write sector data.\n");
    goto Done;
  }
  r = FS_STORAGE_ReadSector(VOLUME_NAME, _aBuffer, SECTOR_INDEX);
  if (r) {
    printf("Could not read sector data.\n");
    goto Done;
  }
  NumLoops = DevInfo.BytesPerSector >> 2;
  pData32  = _aBuffer;
  Cnt      = 0;
  do {
    if (*pData32++ != Cnt++) {
      printf("Verification failed.\n");
      break;
    }
  } while (--NumLoops);
Done:
  FS_STORAGE_Unmount(VOLUME_NAME);
  printf("Finished\n");
  while (1) {
    ;
  }
}

/*************************** End of file ****************************/
