/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2016 SEGGER Microcontroller GmbH & Co. KG         *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       OS version: Internal                                         *
*                                                                    *
*       Current version number will be inserted here                 *
*       when shipment is built.                                      *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : embOSPluginSES.js
Purpose : Script for thread windows for embOS and SEGGER Embedded Studio
--------  END-OF-HEADER  ---------------------------------------------
*/

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

/**** ARM Register indices, constant ********************************/
var r0   =  0;
var r1   =  1;
var r2   =  2;
var r3   =  3;
var r4   =  4;
var r5   =  5;
var r6   =  6;
var r7   =  7;
var r8   =  8;
var r9   =  9;
var r10  = 10;
var r11  = 11;
var r12  = 12;
var sp   = 13;
var lr   = 14;
var pc   = 15;
var psr  = 16;

//
// embOS preserves an additional [pseudo] register
// (exec location with ARM and a modified LR with Cortex-M)
//
var exec = 15;

/**** embOS task states, constant ***********************************/
var TS_MASK_SUSPEND_CNT = (0x03 << 0);
var TS_MASK_TIMEOUT     = (0x01 << 2);
var TS_MASK_STATE       = (0x1f << 3);
var TS_READY            = (0 << 3);  // ready
var TS_WAIT_EVENT       = (1 << 3);  // waiting for event
var TS_WAIT_SEMAZ       = (2 << 3);  // waiting for sema zero
var TS_WAIT_ANY         = (3 << 3);  // Waiting for ...  any reason
var TS_WAIT_SEMANZ      = (4 << 3);  // Waiting for sema not zero
var TS_WAIT_MEMF        = (5 << 5);  // Waiting for Memory pool
var TS_WAIT_Q           = (6 << 3);  // Waiting for Queue
var TS_WAIT_MBNF        = (7 << 3);  // Waiting for mailbox not full
var TS_WAIT_MBNE        = (8 << 3);  // Waiting for mailbox not empty
var TS_WAIT_EVENTOBJ    = (9 << 3);  // Waiting for event object

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       getregs()
*  Functions description:
*    If a thread is selected in the threads window, this function returns
*    an array containing the registers r0-r12, sp, lr, pc, and psr.
*/
function getregs(x) {
  var aRegs       = new Array(17);
  var Interrupted = 0;
  var FPU_Used    = 0;
  var OSSwitch;

  //
  // Retrieve current top-of-stack
  //
  aRegs[sp] = Debug.evaluate("((OS_TASK*)" + x + ")->pStack");
  //
  // Handle well known registers
  //
  aRegs[sp]   += 4;  // "pop" Counters
  aRegs[r4]    = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" R4
  aRegs[r5]    = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" R5
  aRegs[r6]    = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" R6
  aRegs[r7]    = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" R7
  aRegs[r8]    = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" R8
  aRegs[r9]    = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" R9
  aRegs[r10]   = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" R10
  aRegs[r11]   = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" R11
  aRegs[exec]  = TargetInterface.peekWord(aRegs[sp]);
  aRegs[sp]   += 4;  // "pop" exec location
  //
  // Handle remaining registers through special treatment
  //
  if (Debug.evaluate("OS_SwitchAfterISR_ARM")) {  // This is for embOS for ARM
    //
    // Check if this task has been interrupted (i.e., exec location is addr. of OS_Switch() + 12)
    //
    OSSwitch = Debug.evaluate("OS_SwitchAfterISR_ARM");
    if ((aRegs[exec] & ~1) == (OSSwitch + 12)) {
      Interrupted = 1;
    }
    //
    // Restore appropriate register contents
    //
    if (Interrupted == 0) {  // Remaining register contents have NOT been preserved.
      aRegs[r0]  = "0x00000000";                     // unknown after cooperative task switch
      aRegs[r1]  = "0x00000000";                     // unknown after cooperative task switch
      aRegs[r2]  = "0x00000000";                     // unknown after cooperative task switch
      aRegs[r3]  = "0x00000000";                     // unknown after cooperative task switch
      aRegs[r12] = "0x00000000";                     // unknown after cooperative task switch
      aRegs[lr]  = aRegs[exec];                      // Set LR to exec location
      aRegs[psr] = (aRegs[exec] & 1) ? 0x3F : 0x1F;  // Thumb vs. ARM mode?
    } else {                 // Remaining register contents have been preserved.
      aRegs[r0]  = TargetInterface.peekWord(aRegs[sp]);
      aRegs[sp] += 4;  // "pop" R0
      aRegs[r1]  = TargetInterface.peekWord(aRegs[sp]);
      aRegs[sp] += 4;  // "pop" R1
      aRegs[r2]  = TargetInterface.peekWord(aRegs[sp]);
      aRegs[sp] += 4;  // "pop" R2
      aRegs[r3]  = TargetInterface.peekWord(aRegs[sp]);
      aRegs[sp] += 4;  // "pop" R3
      aRegs[r12] = TargetInterface.peekWord(aRegs[sp]);
      aRegs[sp] += 4;  // "pop" R12
      aRegs[lr]  = TargetInterface.peekWord(aRegs[sp]);
      aRegs[sp] += 4;  // "pop" LR
      aRegs[pc]  = TargetInterface.peekWord(aRegs[sp]);
      aRegs[sp] += 4;  // "pop" PC
      aRegs[psr] = TargetInterface.peekWord(aRegs[sp]);
      aRegs[sp] += 4;  // "pop" CPSR
    }
  } else {                                    // This is for embOS for Cortex-M
    var OSSwitchEnd;
    var v;
    //
    // Check if this task has used the FPU
    //
    v = TargetInterface.peekWord(0xE000ED88);
    if ((v & 0x00F00000) != 0) {                // Is the FPU enabled (CPACR b23..b20)?
      v = TargetInterface.peekWord(0xE000EF34);
      if ((v & 0xC0000000) != 0) {              // Is the (lazy) hardware state preservation enabled (FPCCR b31..b30)?
        if ((aRegs[exec] & 0x00000010) == 0) {  // Has this task used the FPU (LR b4)?
          FPU_Used = 1;
        }
      }
    }
    //
    // Check if this task has been interrupted (i.e., pc is inside OS_Switch() function)
    //
    if (FPU_Used == 0) {  // FPU registers have not been preserved, PC is located on stack 6 bytes after current SP
      aRegs[pc] = TargetInterface.peekWord(aRegs[sp] + (4 *  6));
    } else {              // FPU registers have been preserved, PC is located on stack 22 bytes after current SP
      aRegs[pc] = TargetInterface.peekWord(aRegs[sp] + (4 * 22));
    }
    OSSwitch    = Debug.evaluate("OS_Switch");
    OSSwitchEnd = Debug.evaluate("OS_Switch_End");
    if ((aRegs[pc] <= OSSwitch) || (aRegs[pc] >= OSSwitchEnd)) {
      Interrupted = 1;
    }
    //
    // Restore appropriate register contents
    //
    if (FPU_Used == 0) {       // FPU registers have not been preserved, use regular stack layout
      if (Interrupted == 0) {  // Task called OS_Switch(), unwind stack to show previous state.
        aRegs[r0]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R0
        aRegs[r1]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R1
        aRegs[r2]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R2
        aRegs[r3]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R3
        aRegs[r12]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R12
        aRegs[lr]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" LR
        // aRegs[pc] has already been read above
        aRegs[sp]  += 4;  // "pop" PC
        aRegs[psr]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" PSR
        //
        // unwind OS Switch
        //
        aRegs[r2]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R2
        aRegs[r3]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R3
        aRegs[pc] = aRegs[lr];  
      } else {                 // Task was preempted, no additional unwinding required.
        aRegs[r0]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R0
        aRegs[r1]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R1
        aRegs[r2]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R2
        aRegs[r3]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R3
        aRegs[r12]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" R12
        aRegs[lr]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" LR
        // aRegs[pc] has already been read above
        aRegs[sp]  += 4;  // "pop" PC
        aRegs[psr]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;  // "pop" APSR
      }
    } else {                   // FPU registers have been preserved, use extended stack layout
      if (Interrupted == 0) {  // Task called OS_Switch(), unwind stack to show previous state.
        aRegs[sp]  += (4 * 16);  // "pop" S16..S31
        aRegs[r0]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R0
        aRegs[r1]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R1
        aRegs[r2]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R2
        aRegs[r3]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R3
        aRegs[r12]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R12
        aRegs[lr]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" LR
        // aRegs[pc] has already been read above
        aRegs[sp]  += 4;         // "pop" PC
        aRegs[psr]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" PSR
        aRegs[sp]  += (4 * 18);  // "pop" S0..S15, FPSCR, and Res.
        //
        // unwind OS Switch
        //
        aRegs[r2]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R2
        aRegs[r3]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R3
        aRegs[pc] = aRegs[lr];
      } else {                 // Task was preempted, no additional unwinding required.
        aRegs[sp]  += (4 * 16);  // "pop" S16..S31
        aRegs[r0]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R0
        aRegs[r1]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R1
        aRegs[r2]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R2
        aRegs[r3]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R3
        aRegs[r12]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" R12
        aRegs[lr]   = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" LR
        // aRegs[pc] has already been read above
        aRegs[sp]  += 4;         // "pop" PC
        aRegs[psr]  = TargetInterface.peekWord(aRegs[sp]);
        aRegs[sp]  += 4;         // "pop" APSR
        aRegs[sp]  += (4 * 18);  // "pop" S0..S15, FPSCR, and Res.
      }
    }
  }
  return aRegs;
}

/*********************************************************************
*
*       init()
*  Functions description:
*    This function describes the look of the threads window.
*/
function init() {
  Threads.setColumns("Priority", "Id", "Name", "State", "Timeout", "Stack max./total @ location", "Run count");
  Threads.setSortByNumber("Priority");
}

/*********************************************************************
*
*       getstate()
*  Functions description:
*    This function returns the state of a task.
*/
function getState(state) {
  if ((state & TS_MASK_SUSPEND_CNT) != 0) {
    return "Suspended";
  } else if (state ==  TS_MASK_TIMEOUT) {
    return "Delay";
  } else {
    switch (state & (TS_MASK_STATE | TS_MASK_TIMEOUT)) {
    case TS_READY:
      return "Ready";
    case TS_WAIT_EVENT:
    case TS_WAIT_EVENT | TS_MASK_TIMEOUT:
      return "Wait Task event";
    case TS_WAIT_SEMAZ:
    case TS_WAIT_SEMAZ  | TS_MASK_TIMEOUT:
      return "Wait R-Sema";
    case TS_WAIT_SEMANZ:
    case TS_WAIT_SEMANZ | TS_MASK_TIMEOUT:
      return "Wait C-Sema";
    case TS_WAIT_MBNE:
    case TS_WAIT_MBNE | TS_MASK_TIMEOUT:
      return "Get Mail";
    case TS_WAIT_MBNF:
    case TS_WAIT_MBNF | TS_MASK_TIMEOUT:
      return "Put Mail";
    case TS_WAIT_Q:
    case TS_WAIT_Q | TS_MASK_TIMEOUT:
      return "Wait Queue";
    case TS_WAIT_ANY:
    case TS_WAIT_ANY | TS_MASK_TIMEOUT:
      return "Blocked";
    case TS_WAIT_EVENTOBJ:
    case TS_WAIT_EVENTOBJ | TS_MASK_TIMEOUT:
      return "Wait Event object";
    default:
      return "Invalid";
    }
  }
}

/*********************************************************************
*
*       update()
*  Functions description:
*    This function is called to update the threads window and its
*    entries upon debug stop.
*/
function update() {
  var pCurrentTask;
  var OS_TASK;
  var pTask;
  var Id;
  var i;
  var abStack;
  var pStackUsage;
  var StackUsage;
  var MaxStackUsed;
  var pNumActivations;
  var NumActivations;
  var pTaskName;
  var TaskName;

  //
  // Initially, clear entire threads window
  //
  Threads.clear();
  //
  // Retrieve start of linked task list from target
  //
  pTask        = Debug.evaluate("OS_Global->pTask");
  pCurrentTask = Debug.evaluate("OS_Global->pCurrentTask");
  //
  // Create new queue if at least one task is present
  //
  if (pTask != 0) {
    Threads.newqueue("Task List");
  }
  //
  // Iterate through linked list of tasks and create an entry to the queue for each task
  //
  while (pTask != 0) {
    OS_TASK = Debug.evaluate("*(OS_TASK*)" + pTask);
    Id      = Debug.evaluate("(OS_TASK*)" + pTask);
    //
    // TaskName is unavailable in some libmodes
    //
    pTaskName = Debug.evaluate("*(const char *)((OS_TASK*)" + pTask + ").Name");
    TaskName  = (pTaskName != NULL) ? OS_TASK.Name : "n.a.";
    //
    // NumActivations is unavailable in some libmodes
    //
    pNumActivations = Debug.evaluate("*(OS_U32*)((OS_TASK*)" + pTask + ").NumActivations");
    NumActivations  = (pNumActivations != NULL) ? OS_TASK.NumActivations : "n.a.";
    //
    // Stackinfo is unavailable in some libmodes
    //
    pStackUsage = Debug.evaluate("*(OS_UINT*)((OS_TASK*)" + pTask + ").StackSize");
    if (pStackUsage != NULL) {
      abStack = TargetInterface.peekBytes(OS_TASK.pStackBot, OS_TASK.StackSize);
      for (i = 0; i < OS_TASK.StackSize; i++) {
        if (abStack[i].toString(16) != "cd") {
          break;
        }
      }
      MaxStackUsed = OS_TASK.StackSize - i;
      StackUsage   = MaxStackUsed + " / " + OS_TASK.StackSize + " @ 0x" + OS_TASK.pStackBot.toString(16).toUpperCase();
    } else {
      StackUsage = "n.a.";
    }
    //
    // Current or suspended task?
    //
    if (pTask == pCurrentTask) {
      Threads.add(OS_TASK.Priority,                      // Priority
                  "0x" + Id.toString(16).toUpperCase(),  // ID
                  TaskName,                              // Taskname
                  "Executing",                           // artificial Taskstate
                  OS_TASK.Timeout,                       // Timeout in ms
                  StackUsage,                            // Stack usage
                  NumActivations,                        // Run count
                  []                                     // read current context for this task
                 );
    } else {
      Threads.add(OS_TASK.Priority,                      // Priority
                  "0x" + Id.toString(16).toUpperCase(),  // ID
                  TaskName,                              // Taskname
                  getState(OS_TASK.Stat),                // actual Taskstate
                  OS_TASK.Timeout,                       // Timeout in ms
                  StackUsage,                            // Stack usage
                  NumActivations,                        // Run count
                  pTask                                  // call getregs() for this task
                 );
    }
    pTask = OS_TASK.pNext;
  }
}

/****** End Of File *************************************************/
