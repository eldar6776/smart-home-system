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
File    : OS_RSemaphore.c
Purpose : embOS sample program for resource semaphore.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include "stdio.h"

static OS_STACKPTR int StackHP[128], StackLP[128];   /* Task stacks */
static OS_TASK         TCBHP, TCBLP;         /* Task-control-blocks */
static OS_RSEMA        RSema;

/*********************************************************************
*
*       _Write()
*/
static void _Write(char const* s) {
  OS_Use(&RSema);
  printf(s);
  OS_Unuse(&RSema);
}

/*********************************************************************
*
*       HPTask()
*/
static void HPTask(void) {
  while (1) {
    _Write("HPTask\n");
    OS_Delay(50);
  }
}

/*********************************************************************
*
*       LPTask()
*/
static void LPTask(void) {
  while (1) {
    _Write("LPTask\n");
    OS_Delay(200);
  }
}

/*********************************************************************
*
*       main()
*/
int main(void) {
  OS_IncDI();                      /* Initially disable interrupts  */
  OS_InitKern();                   /* Initialize OS                 */
  OS_InitHW();                     /* Initialize Hardware for OS    */
  /* You need to create at least one task before calling OS_Start() */
  OS_CREATETASK(&TCBHP, "HP Task", HPTask, 100, StackHP);
  OS_CREATETASK(&TCBLP, "LP Task", LPTask,  50, StackLP);
  OS_CreateRSema(&RSema);          /* Creates resource semaphore    */
  OS_Start();                      /* Start multitasking            */
  return 0;
}

/****** End Of File *************************************************/
