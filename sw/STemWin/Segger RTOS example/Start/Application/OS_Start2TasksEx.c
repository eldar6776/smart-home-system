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
File    : OS_Start2TasksEx.c
Purpose : embOS sample program running two extended tasks.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int StackHP[128], StackLP[128];   /* Task stacks */
static OS_TASK         TCBHP, TCBLP;         /* Task-control-blocks */

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       TaskEx()
*/
static void TaskEx(void* pData) {
  while (1) {
    OS_Delay((OS_TIME)pData);
  }
}

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
  OS_CREATETASK_EX(&TCBHP, "HP Task", TaskEx, 100, StackHP, (void*) 50);
  OS_CREATETASK_EX(&TCBLP, "LP Task", TaskEx,  50, StackLP, (void*) 200);
  OS_SendString("Start project will start multitasking !\n");
  OS_TerminateTask(NULL);
}

/****** End Of File *************************************************/
