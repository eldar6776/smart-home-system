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
File    : FS_Start.c
Purpose : Start application for file system.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include "FS.h"
#include "FS_Int.h"


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
  U32          v;
  FS_FILE    * pFile;
  char         ac[256];
  char         acFileName[32];
  const char * sVolumeName = "";

  FS_X_Log("Start\n");
  //
  // Initialize file system
  //
  FS_Init();
  //
  // Check if low-level format is required
  //
  FS_FormatLLIfRequired(sVolumeName);
  //
  // Check if volume needs to be high level formatted.
  //
  if (FS_IsHLFormatted(sVolumeName) == 0) {
    FS_X_Log("High-level format\n");
    FS_Format(sVolumeName, NULL);
  }
  sprintf(ac, "Running sample on \"%s\"\n", sVolumeName);
  FS_X_Log(ac);
  v = FS_GetVolumeFreeSpaceKB(sVolumeName);
  if (v <= 0x7fff) {
    sprintf(ac, "  Free space: %lu kbytes\n", v);
  } else {
    v >>= 10;
    sprintf(ac, "  Free space: %lu MBytes\n", v);
  }
  FS_X_Log(ac);
  sprintf(acFileName, "%s\\File.txt", sVolumeName);
  sprintf(ac, "  Write test data to file %s\n", acFileName);
  FS_X_Log(ac);
  pFile = FS_FOpen(acFileName, "w");
  if (pFile) {
    FS_Write(pFile, "Test", 4);
    FS_FClose(pFile);
  } else {
    sprintf(ac, "Could not open file: %s to write.\n", acFileName);
    FS_X_Log(ac);
  }
  v = FS_GetVolumeFreeSpaceKB(sVolumeName);
  if (v <= 0x7fff) {
    sprintf(ac, "  Free space: %lu kbytes\n", v);
  } else {
    v >>= 10;
    sprintf(ac, "  Free space: %lu MBytes\n", v);
  }
  FS_X_Log(ac);
  FS_Unmount(sVolumeName);
  FS_X_Log("Finished\n");
  while (1) {
    ;
  }
}

/*************************** End of file ****************************/
