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
File    : FS_DeviceActivity.c
Purpose : Demonstrates the usage of the callback invoked on each device operation.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include "FS.h"
#include "FS_Int.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define VOLUME_NAME       ""

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int          Type;
  const char * s;
} TYPE_DESC;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8   _acBuffer[511] = {0};
static char _ac[128] = {0};

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/
static const TYPE_DESC _aSectorType[] = {
  { FS_SECTOR_TYPE_DATA, "DATA" },
  { FS_SECTOR_TYPE_MAN,  "MAN " },
  { FS_SECTOR_TYPE_DIR,  "DIR " },
};

static const TYPE_DESC _aOperationType[] = {
  { FS_OPERATION_READ,   "Read  " },
  { FS_OPERATION_WRITE,  "Write " },
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _Num2Name
*/
static const char * _Num2Name(int Type, const TYPE_DESC * pDesc, unsigned NumItems) {
  unsigned i;

  for (i = 0; i < NumItems; i++) {
    if (pDesc->Type == Type) {
      return pDesc->s;
    }
    pDesc++;
  }
  return "Unknown Type";
}

/*********************************************************************
*
*       _cbOnDeviceActivity
*/
static void _cbOnDeviceActivity(FS_DEVICE * pDevice, unsigned Operation, U32 StartSector, U32 NumSectors, int SectorType) {
  const char * sOperation;
  const char * sSectorType;

  FS_USE_PARA(pDevice);
  sOperation  = _Num2Name((int)Operation, _aOperationType, COUNTOF(_aOperationType));
  sSectorType = _Num2Name(SectorType, _aSectorType, COUNTOF(_aSectorType));
  sprintf(_ac, "  %s: StartSector: 0x%08lx, NumSectors: 0x%08lx, SectorType: %s \n", sOperation, StartSector, NumSectors, sSectorType);
  FS_X_Log(_ac);
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
  FS_FILE * pFile;
  char      acFileName[64];
  U32       NumBytes;

  FS_X_Log("Start\n");
  //
  // Initialize file system
  //
  FS_Init();
  //
  // Check if low-level format is required
  //
  FS_FormatLLIfRequired(VOLUME_NAME);
  //
  // Check if volume needs to be high level formatted.
  //
  if (FS_IsHLFormatted(VOLUME_NAME) == 0) {
    FS_X_Log("High level formatting\n");
    FS_Format("", NULL);
  }
  FS_SetOnDeviceActivityHook(VOLUME_NAME, _cbOnDeviceActivity);
  sprintf(_ac, "Running sample on \"%s\"\n", VOLUME_NAME);
  FS_X_Log(_ac);
  sprintf(acFileName, "%s\\File.txt", VOLUME_NAME);
  //
  // Open the file
  //
  printf("Open/create file\n");
  pFile = FS_FOpen(acFileName, "w");
  if (pFile) {
    //
    // 1st write to file.
    //
    NumBytes = 4;
    sprintf(_ac, "1st Write (%lu bytes) to file.\n", NumBytes);
    FS_X_Log(_ac);
    FS_Write(pFile, "Test", NumBytes);
    //
    // 2nd write to file 511 bytes.
    //
    NumBytes = sizeof(_acBuffer);
    sprintf(_ac, "2nd write (%lu bytes) to file.\n", NumBytes);
    FS_X_Log(_ac);
    FS_Write(pFile, _acBuffer, NumBytes);
    FS_X_Log("Close file\n");
    //
    // Close the file.
    //
    FS_FClose(pFile);
  } else {
    printf("Could not open file \"%s\".\n", acFileName);
  }
  FS_X_Log("Finished\n");
  while (1) {
    ;
  }
}

/*************************** End of file ****************************/
