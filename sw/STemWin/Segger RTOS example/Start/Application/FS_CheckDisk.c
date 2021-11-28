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
File        : FS_CheckDisk.c
Purpose     : Sample program demonstrating disk checking functionality.
---------------------------END-OF-HEADER------------------------------
*/

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "FS_API.h"
#include "Global.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define VOLUME_NAME       ""
#define MAX_RECURSION     5

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32 _aBuffer[2000];   // The more space is used the faster the disk checking can run.
static int _NumErrors;

/*********************************************************************
*
*       _OnError
*/
static int _OnError(int ErrCode, ...) {
  va_list      ParamList;
  const char * sFormat;
  int          c;
  char         ac[1000];

  (void)memset(&ParamList, 0, sizeof(ParamList));
  sFormat = FS_FAT_CheckDisk_ErrCode2Text(ErrCode);
  if (sFormat) {
    va_start(ParamList, ErrCode);
    vsprintf(ac, sFormat, ParamList);
    FS_X_Log(ac);
    FS_X_Log("\n");
  }
  if (ErrCode != FS_CHECKDISK_ERRCODE_CLUSTER_UNUSED) {
    FS_X_Log("  Do you want to repair this error? (y/n/a) ");
  } else {
    FS_X_Log("  * Convert lost cluster chain into file (y)\n"
             "  * Delete cluster chain                 (d)\n"
             "  * Do not repair                        (n)\n"
             "  * Abort                                (a) ");
    FS_X_Log("\n");
  }
  ++_NumErrors;
  c = getchar();
  FS_X_Log("\n");
  if ((c == 'y') || (c == 'Y')) {
    return FS_CHECKDISK_ACTION_SAVE_CLUSTERS;
  } else if ((c == 'a') || (c == 'A')) {
    return FS_CHECKDISK_ACTION_ABORT;
  } else if ((c == 'd') || (c == 'D')) {
    return FS_CHECKDISK_ACTION_DELETE_CLUSTERS;
  }
  return FS_CHECKDISK_ACTION_DO_NOT_REPAIR;
}

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
  FS_X_Log("Start\n");
  FS_Init();
  _NumErrors = 0;
  //
  // Call the function repeatedly until all errors are fixed.
  //
  while (FS_CheckDisk(VOLUME_NAME, _aBuffer, sizeof(_aBuffer), MAX_RECURSION, _OnError) == FS_CHECKDISK_RETVAL_RETRY) {
    ;
  }
  if (_NumErrors == 0) {
    FS_X_Log("No errors found.\n");
  }
  FS_X_Log("Finished\n");
  while (1) {
    ;
  }
}

/*************************** End of file ****************************/

