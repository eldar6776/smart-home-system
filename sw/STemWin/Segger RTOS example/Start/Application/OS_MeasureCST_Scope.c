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
File    : OS_MeasureCST_Scope.c
Purpose : embOS sample program allowing measurement of the embOS 
          context switching time by using an oscilloscope. To do so,
          it sets and clears a port pin (as defined in BSP.c). The
          context switching time is

            Time = (d - c) - (b - a)

             -----   --                   ---------------
                  | |  |                 |
                   -    -----------------
                  ^ ^  ^                 ^
                  a b  c                 d

          The time between c and d is the context switching time, but
          note that the real context switching time is shorter, as the
          signal also contains the overhead of switching the LED on
          and off. The time of this overhead is visible on an
          oscilloscope as a small peak between a and b.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include "BSP.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int StackHP[128];                 /* Task stacks */
static OS_TASK         TCBHP;                /* Task-control-blocks */

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
    OS_Suspend(NULL);  // Suspend high priority task
    BSP_ClrLED(0);     // Stop measurement
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
  OS_CREATETASK(&TCBHP, "HP Task", HPTask, 150, StackHP);
  OS_Delay(1);
  //
  // Perform measurements in endless loop
  //
  while (1) {
    OS_Delay(100);      // Synchronize to tick to avoid jitter
    //
    // Display measurement overhead
    //
    BSP_SetLED(0);
    BSP_ClrLED(0);
    //
    // Perform measurement
    //
    BSP_SetLED(0);      // Start measurement
    OS_Resume(&TCBHP);  // Resume high priority task to force task switch
  }
}

/****** End Of File *************************************************/
