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
-------------------------- END-OF-HEADER -----------------------------

File    : IP_OS_embOS.c
Purpose : Kernel abstraction for embOS. Do not modify to allow easy updates!
*/

#include "IP_Int.h"
#include "RTOS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// The default tick is expected to be 1ms. For a finer tick
// like 1us a multiplicator has to be configured. The tick
// should match the OS tick.
// Examples:
//   - 1ms   = 1
//   - 100us = 10
//   - 10us  = 100
//
#define TICK_MULTIPLICATOR  1  // Default, 1 = 1ms.

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

typedef struct TCP_WAIT {
  struct TCP_WAIT* pNext;
  struct TCP_WAIT* pPrev;
  void*            pWaitItem;
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
  OS_TASK*         pTask;
#endif
  OS_EVENT         Event;
} TCP_WAIT;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static TCP_WAIT*            _pTCPWait;  // Head of List. One entry per waiting task
static char                 _IsInited;
#if (OS_VERSION >= 38606)               // OS_AddOnTerminateHook() is supported since embOS v3.86f .
static OS_ON_TERMINATE_HOOK _OnTerminateTaskHook;
#endif

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

OS_RSEMA IP_OS_RSema;     // Public only to allow inlining (direct call from IP-Stack).
OS_EVENT IP_OS_EventNet;  // Public only to allow inlining (direct call from IP-Stack).
OS_EVENT IP_OS_EventRx;   // Public only to allow inlining (direct call from IP-Stack).
OS_TASK* IP_OS_pIPTask;   // Public only to allow inlining (direct call from IP-Stack).

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _DLIST_RemoveDelete()
*
*  Function description
*    Removes a waitable object from the doubly linked list and deletes
*    its wait object from embOS lists.
*
*  Parameters
*    pTCPWait: Item to remove.
*
*  Additional information
*    Function is called from IP_OS_WaitItemTimed() and _OnTerminateTask().
*    Calling functions have to make sure that it is not called recursive
*    by disabling task switch before calling this routine.
*/
static void _DLIST_RemoveDelete(TCP_WAIT* pTCPWait) {
  //
  // Remove entry from doubly linked list.
  //
  if (pTCPWait->pPrev) {
    pTCPWait->pPrev->pNext = pTCPWait->pNext;
  } else {
    _pTCPWait = pTCPWait->pNext;
  }
  if (pTCPWait->pNext) {
    pTCPWait->pNext->pPrev = pTCPWait->pPrev;
  }
  //
  // Delete the event object.
  //
  OS_EVENT_Set(&pTCPWait->Event);  // Set event to prevent error on removing an unsignalled event.
  OS_EVENT_Delete(&pTCPWait->Event);
}

#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
/*********************************************************************
*
*       _OnTerminateTask()
*
*  Function description
*    This routine removes the registered wait objects from the doubly
*    linked list of wait objects upon termination of its task. This
*    is necessary due to the fact that the element is publically known
*    due to a doubly linked list but is stored on a task stack. In case
*    this task gets terminated we need to gracefully remove the element
*    from all resources and even remove it from any embOS list.
*
*  Parameters
*    pTask: Task handle of task that will be terminated.
*
*  Additional information
*    Function is called from an application task via OS hook with
*    task switching disabled.
*/
static void _OnTerminateTask(OS_CONST_PTR OS_TASK* pTask) {
  TCP_WAIT* pTCPWait;

  for (pTCPWait = _pTCPWait; pTCPWait; pTCPWait = pTCPWait->pNext) {
    if (pTCPWait->pTask == pTask) {
      //
      // Prior to deleting an event object it needs to be set to be unused
      // (no task waiting for it). Setting the EVENT object is safe as in
      // all cases only one the task that created the object on its stack
      // is waiting for the event and task switching is disabled. Therefore
      // we will stay in this routine and finish our work.
      //
      OS_EVENT_Set(&pTCPWait->Event);
      _DLIST_RemoveDelete(pTCPWait);
      break;
    }
  }
}
#endif

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       IP_OS_Init()
*
*  Function description
*    Initialize (create) all objects required for task syncronisation.
*    These are 2 events (for IP_Task and RxTask)
*    and one semaphore for protection of critical code which  may not be
*    executed from multiple task at the same time.
*/
void IP_OS_Init(void) {
  if (_IsInited == 0) {
    OS_CREATERSEMA(&IP_OS_RSema);
    OS_EVENT_Create(&IP_OS_EventNet);
    OS_EVENT_Create(&IP_OS_EventRx);
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
    OS_AddOnTerminateHook(&_OnTerminateTaskHook, _OnTerminateTask);
#endif
    _IsInited = 1;
  }
}

/*********************************************************************
*
*       IP_OS_DisableInterrupt()
*
*  Function description
*    Disables interrupts to lock against calls from interrupt routines.
*/
void IP_OS_DisableInterrupt(void) {
  OS_IncDI();
}

/*********************************************************************
*
*       IP_OS_EnableInterrupt()
*
*  Function description
*    Enables interrupts that have previously been disabled.
*/
void IP_OS_EnableInterrupt(void) {
  OS_DecRI();
}

/*********************************************************************
*
*       IP_OS_GetTime32()
*
*  Function description
*    Return the current system time in ms.
*    The value will wrap around after app. 49.7 days. This is taken
*    into account by the stack.
*
*  Return value
*    U32 timestamp in system ticks (typically 1ms).
*/
U32 IP_OS_GetTime32(void) {
  return OS_GetTime32();
}

/*********************************************************************
*
*       IP_OS_Delay()
*
*  Function description
*    Blocks the calling task for a given time.
*
*  Parameters
*    ms: Time to block in system ticks (typically 1ms).
*/
void IP_OS_Delay(unsigned ms) {
#if (TICK_MULTIPLICATOR != 1)
  ms = ms * TICK_MULTIPLICATOR;
#endif
  OS_Delay(ms + 1);
}

/*********************************************************************
*
*       IP_OS_WaitNetEvent()
*
*  Function description
*    Called from IP_Task() only.
*    Blocks until the timeout expires or a NET-event occurs,
*    meaning IP_OS_SignalNetEvent() is called from an other task or
*    ISR.
*
*  Parameters
*    ms: Timeout for waiting in system ticks (typically 1ms).
*/
void IP_OS_WaitNetEvent(unsigned ms) {
#if (TICK_MULTIPLICATOR != 1)
  ms = ms * TICK_MULTIPLICATOR;
#endif
//  OS_EVENT_WaitTimed(&IP_OS_EventNet, ms);
  IP_OS_pIPTask = OS_GetpCurrentTask();
  OS_WaitEventTimed(1, ms);
}

/*********************************************************************
*
*       IP_OS_SignalNetEvent()
*
*  Function description
*    Wakes the IP_Task if it is waiting for a NET-event or timeout in
*    the function IP_OS_WaitNetEvent().
*/
void IP_OS_SignalNetEvent(void) {
//  OS_EVENT_Set(&IP_OS_EventNet);
  if (IP_OS_pIPTask) {
    OS_SignalEvent(1, IP_OS_pIPTask);
  }
}

/*********************************************************************
*
*       IP_OS_WaitRxEvent()
*
*  Function description
*    Called by the IP_RxTask() whenever idle (no more packets in the
*    In-Fifo).
*/
void IP_OS_WaitRxEvent(void) {
  OS_EVENT_Wait(&IP_OS_EventRx);
}

/*********************************************************************
*
*       IP_OS_SignalRxEvent()
*/
void IP_OS_SignalRxEvent(void) {
  OS_EVENT_Set(&IP_OS_EventRx);
}

/*********************************************************************
*
*       IP_OS_Lock()
*
*  Function description
*    The stack requires a single lock, typically a resource semaphore
*    or mutex. This function locks this object, guarding sections of
*    the stack code against other threads.
*    If the entire stack executes from a single task, no
*    functionality is required here.
*/
void IP_OS_Lock(void) {
  OS_Use(&IP_OS_RSema);
}

/*********************************************************************
*
*       IP_OS_Unlock()
*
*  Function description
*    Unlocks the single lock, locked by a previous call to IP_OS_Lock()
*    and signals the IP_Task() if a packet has been freed.
*/
void IP_OS_Unlock(void) {
  int Status;

  //
  // Read the current lock count before unlocking to prevent
  // directly being locked again by a higher priority task.
  //
  Status = OS_GetSemaValue(&IP_OS_RSema);
  OS_Unuse(&IP_OS_RSema);
  //
  // If this would have been the last unlock we check if we need to
  // to signal the IP_Task().
  //
  if ((Status - 1) == 0) {
    if (IP_Global.OnPacketFreeUsed != 0) {
      IP_OS_SignalNetEvent();
    }
  }
}

/*********************************************************************
*
*       IP_OS_AssertLock()
*
*  Function description
*    Makes sure that the lock is in use. Called in debug builds only.
*/
void IP_OS_AssertLock(void) {
  if (IP_OS_RSema.UseCnt == 0) {
    while (1);    // Allows setting a breakpoint on this condition
  } else {
    if (IP_OS_RSema.pTask != OS_GetpCurrentTask()) {
      while (1);  // Allows setting a breakpoint on this condition
    }
  }
}

/*********************************************************************
*
*       IP_OS_WaitItemTimed()
*
*  Function description
*    Suspend a task which needs to wait for a object.
*    This object is identified by a pointer to it and can be of any
*    type, e.g. socket.
*
*  Parameters
*    pWaitItem: Item to wait for.
*    Timeout  : Timeout for waiting in system ticks (typically 1ms).
*
*  Additional information
*    Function is called from an application task and is locked in
*    every case.
*/
void IP_OS_WaitItemTimed(void* pWaitItem, unsigned Timeout) {
  TCP_WAIT TCPWait;

#if (TICK_MULTIPLICATOR != 1)
  Timeout = Timeout * TICK_MULTIPLICATOR;
#endif
  //
  // Create the wait object which contains the OS-Event object.
  //
  TCPWait.pWaitItem = pWaitItem;
  OS_EVENT_Create(&TCPWait.Event);
  //
  // Add to beginning of doubly-linked list.
  //
  TCPWait.pPrev = NULL;
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
  TCPWait.pTask = OS_GetpCurrentTask();
  OS_EnterRegion();        // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
#endif
  TCPWait.pNext = _pTCPWait;
  _pTCPWait     = &TCPWait;
  if (TCPWait.pNext) {
    TCPWait.pNext->pPrev = &TCPWait;
  }
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
  //
  // Unlock mutex.
  //
  IP_OS_UNLOCK();
  //
  //  Suspend this task.
  //
  if (Timeout == 0) {
    OS_EVENT_Wait(&TCPWait.Event);
  } else {
    OS_EVENT_WaitTimed(&TCPWait.Event, Timeout);
  }
  //
  // Lock the mutex again.
  //
  IP_OS_LOCK();
  //
  // Remove it from doubly linked list and delete event object.
  //
#if (OS_VERSION >= 38606)  // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
  OS_EnterRegion();
#endif
  _DLIST_RemoveDelete(&TCPWait);
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
}

/*********************************************************************
*
*       IP_OS_WaitItem()
*
*  Function description
*    Suspend a task which needs to wait for an object.
*    This object is identified by a pointer to it and can be of any
*    type, e.g. socket.
*
*  Parameters
*    pWaitItem: Item to wait for.
*
*  Additional information
*    Function is called from an application task.
*/
void IP_OS_WaitItem(void* pWaitItem) {
  IP_OS_WaitItemTimed(pWaitItem, 0);
}

/*********************************************************************
*
*       IP_OS_SignalItem()
*
*  Function description
*    Sets an object to signaled state, or resumes tasks which are
*    waiting at the event object.
*
*  Parameters
*    pWaitItem: Item to signal.
*
*  Additional information
*    Function is called from a task, not an ISR and is locked in
*    every case.
*/
void IP_OS_SignalItem(void* pWaitItem) {
  TCP_WAIT* pTCPWait;

#if (OS_VERSION >= 38606)  // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
  OS_EnterRegion();
#endif
  for (pTCPWait = _pTCPWait; pTCPWait; pTCPWait = pTCPWait->pNext) {
    if (pTCPWait->pWaitItem == pWaitItem) {
      OS_EVENT_Set(&pTCPWait->Event);
    }
  }
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
}

/*********************************************************************
*
*       IP_OS_AddTickHook()
*
*  Function description
*    Add tick hook. This is a function which is called from the tick
*    handler, typically because the driver's interrupt handler is not
*    called via it's own hardware ISR. (We poll 1000 times per second)
*
*  Parameters
*    pfHook: Callback to be called on every tick.
*
*  Additional information
*    Function is called from a task, not an ISR.
*/
void IP_OS_AddTickHook(void (*pfHook)(void)) {
#if (OS_VERSION >= 36000)
  static OS_TICK_HOOK _cb;
  OS_AddTickHook(&_cb, pfHook);
#else
  IP_PANIC("IP_OS_AddTickHook() requires an OS version >= 3.60");  // This requires a newer version of the OS.
#endif
}

/*********************************************************************
*
*       IP_OS_GetTaskName()
*
*  Function description
*    Retrieves the task name (if available from the OS and not in
*    interrupt) for the currently active task.
*
*  Parameters
*    pTask: Pointer to a task identifier such as a task control block.
*
*  Return value
*    Terminated string with task name.
*/
const char* IP_OS_GetTaskName(void* pTask) {
  return OS_GetTaskName((OS_TASK*)pTask);
}

/*************************** End of file ****************************/
