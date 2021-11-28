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
File    : OS_ExtendedTask.c
Purpose : embOS sample program demonstrating the extension of tasks.
          This sample application is described in the generic manual
          in chapter:
          "Extending the task context by using own task structures"
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include <stdio.h>

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

//
// Custom task structure with extended task context.
//
typedef struct {
  OS_TASK Task;     // OS_TASK has to be the first element
  OS_TIME Timeout;  // Any other data type may be used to extend the context
  char*   pString;  // Any number of elements may be used to extend the context
} MY_APP_TASK;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int StackHP[128], StackLP[128];   /* Task stacks */
static MY_APP_TASK     TCBHP, TCBLP;         /* Task-control-blocks */

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       MyTask()
*/
static void MyTask(void) {
  MY_APP_TASK* pThis;
  OS_TIME      Timeout;
  char*        pString;
  
  pThis = (MY_APP_TASK*)OS_GetTaskID();
  while (1) {
    Timeout = pThis->Timeout;
    pString = pThis->pString;
    printf(pString);
    OS_Delay(Timeout);
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
  //
  // Give task contexts individual data
  //
  TCBHP.Timeout = 200;
  TCBHP.pString = "HP task running\n";
  TCBLP.Timeout = 500;
  TCBLP.pString = "LP task running\n";
  OS_CREATETASK(&TCBHP.Task, "HP Task", MyTask, 100, StackHP);
  OS_CREATETASK(&TCBLP.Task, "LP Task", MyTask,  50, StackLP);
  OS_SendString("Start project will start multitasking !\n");
  OS_TerminateTask(NULL);
}

/****** End Of File *************************************************/
