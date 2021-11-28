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
File    : OS_MeasureCST_HRTimer_embOSView.c
Purpose : embOS sample program that measures the embOS context
          switching time and displays the result in the terminal
          window of embOSView. It is completly generic and runs on
          every target that is configured for embOSView.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include <stdio.h>

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int StackHP[128];                  /* Task stack */
static OS_TASK         TCBHP;                 /* Task-control-block */
static OS_U32          Time;

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       HPTask()
*/
static void HPTask(void) {
  while (1) {
    OS_Suspend(NULL);      // Suspend high priority task
    OS_Timing_End(&Time);  // Stop measurement
  }
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       MainTask()
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif
void MainTask(void) {
  char   acBuffer[100];    // Output buffer
  OS_U32 MeasureOverhead;  // Time for Measure Overhead
  OS_U32 v;                // Real context switching time
  
  OS_CREATETASK(&TCBHP, "HP Task", HPTask, 150, StackHP);
  OS_Delay(1);
  //
  // Measure overhead for time measurement so we can take this into account by subtracting it
  //
  OS_Timing_Start(&MeasureOverhead);
  OS_Timing_End(&MeasureOverhead);
  //
  // Perform measurements in endless loop
  //
  while (1) {
    OS_Delay(100);                              // Synchronize to tick to avoid jitter
    OS_Timing_Start(&Time);                     // Start measurement
    OS_Resume(&TCBHP);                          // Resume high priority task to force task switch
    v  = OS_Timing_GetCycles(&Time);
    v -= OS_Timing_GetCycles(&MeasureOverhead); // Calculate real context switching time (w/o measurement overhead)
    v  = OS_ConvertCycles2us(1000 * v);         // Convert cycles to nano-seconds, increase time resolution
    sprintf(acBuffer, "Context switch time: %lu. %.3lu usec\r", (v / 1000uL), (v % 1000uL));  // Create result text
    OS_SendString(acBuffer);                    // Print out result
  }
}


/****** End Of File *************************************************/
