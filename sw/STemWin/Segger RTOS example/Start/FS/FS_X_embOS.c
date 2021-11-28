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
File        : FS_X_embOS.c
Purpose     : embOS OS Layer for the file system.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/
#include "FS_Int.h"
#include "FS_OS.h"
#include "RTOS.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_RSEMA * _paSema;
static unsigned   _NumLocks;
static OS_EVENT   _Event;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_X_OS_Lock
*/
void FS_X_OS_Lock(unsigned LockIndex) {
  OS_RSEMA * pSema;

  pSema = _paSema + LockIndex;
  FS_DEBUG_LOG((FS_MTYPE_OS, "OS: Lock   Index: 0x%8x\n", LockIndex));
  OS_Use(pSema);
}

/*********************************************************************
*
*       FS_X_OS_Unlock
*/
void FS_X_OS_Unlock(unsigned LockIndex) {
  OS_RSEMA * pSema;

  pSema = _paSema + LockIndex;
  FS_DEBUG_LOG((FS_MTYPE_OS, "OS: Unlock Index: 0x%8x\n", LockIndex));
  OS_Unuse(pSema);
}

/*********************************************************************
*
*       FS_X_OS_Init
*
*  Function description
*    Initializes the OS resources. This function is called by
*    FS_Init(). You should create all resources required by the
*    OS to support multi threading of the file system.
*
*  Parameters
*    NumLocks   Number of locks that should be created.
*
*  Return value
*    ==0    Success
*    !=0    An error occurred
*/
void FS_X_OS_Init(unsigned NumLocks) {
  unsigned   i;
  OS_RSEMA * pSema;

  _paSema = (OS_RSEMA *)FS_AllocZeroed((I32)(NumLocks * sizeof(OS_RSEMA)));
  pSema =_paSema;
  for (i = 0; i < NumLocks; i++) {
    OS_CREATERSEMA(pSema++);
  }
  OS_EVENT_Create(&_Event);
  _NumLocks = NumLocks;
}


/*********************************************************************
*
*       FS_X_OS_DeInit
*
*  Function description
*    Delete all locks that have been created by FS_X_OS_Init().
*/
void FS_X_OS_DeInit(void) {
  unsigned i;
  OS_RSEMA  * pSema;
  unsigned    NumLocks;

  NumLocks = _NumLocks;
  pSema   = &_paSema[0];
  for (i = 0; i < NumLocks; i++) {
    OS_DeleteRSema(pSema);
    pSema++;
  }
  OS_EVENT_Delete(&_Event);
  FS_Free(_paSema);
  _paSema  = NULL;
  _NumLocks = 0;
}

/*********************************************************************
*
*       FS_X_OS_GetTime
*/
U32  FS_X_OS_GetTime(void) {
  return (U32)OS_GetTime32();
}

/*********************************************************************
*
*       FS_X_OS_Wait
*
*  Function description
*    Wait for an event to be signaled.
*
*  Parameters
*    Timeout  Time to be wait for the event object.
*
*  Return value:
*    ==0      Event object was signaled within the timeout value
*    !=0      An error or a timeout occurred.
*/
int FS_X_OS_Wait(int Timeout) {
  int r;

  r = -1;
  if (OS_EVENT_WaitTimed(&_Event, Timeout) == 0) {
    r = 0;
  }
  return r;
}

/*********************************************************************
*
*       FS_X_OS_Signal
*
*  Function description
*    Signals a event.
*/
void FS_X_OS_Signal(void) {
  OS_EVENT_Set(&_Event);
}

/*********************************************************************
*
*       FS_X_OS_Delay
*
*  Function description
*    Blocks the execution for the specified number of milliseconds.
*/
void FS_X_OS_Delay(int ms) {
  OS_Delay(ms + 1);
}

/*************************** End of file ****************************/
