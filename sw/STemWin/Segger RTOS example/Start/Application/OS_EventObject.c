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
File    : OS_EventObject.c
Purpose : embOS sample program demonstrating the usage of event objects.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_STACKPTR int StackHP[128], StackHW[128];   /* Task stacks */
static OS_TASK         TCBHP, TCBHW;         /* Task-control-blocks */
static OS_EVENT        HW_Event;

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _HW_Wait()
*/
static void _HW_Wait(void) {
  OS_EVENT_Wait(&HW_Event);
}

/*********************************************************************
*
*       _HW_Free()
*/
static void _HW_Free(void) {
  OS_EVENT_Set(&HW_Event);
}

/*********************************************************************
*
*       HPTask()
*/
static void HPTask(void) {
  //
  // Wait until HW module is set up
  //
  _HW_Wait();
  while (1) {
    OS_Delay(50);
  }
}

/*********************************************************************
*
*       HWTask()
*/
static void HWTask(void) {
  //
  // Wait until HW module is set up
  //
  OS_Delay(100);
  //
  // Init done, send broadcast to waiting tasks
  //
  _HW_Free();
  while (1) {
    OS_Delay(40);
  }
}

/*********************************************************************
*
*       _HW_Init()
*/
static void _HW_Init(void) {
  OS_CREATETASK(&TCBHW, "HWTask", HWTask, 25, StackHW);
  OS_EVENT_Create(&HW_Event);
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
  _HW_Init();
  OS_CREATETASK(&TCBHP, "HP Task", HPTask, 150, StackHP);
  OS_SendString("Start project will start multitasking !\n");
  _HW_Wait();                      /* Wait until HW module is set up */
  while (1) {
    OS_Delay (200);
  }
}
/****** End Of File *************************************************/
