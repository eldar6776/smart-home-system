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
File    : OS_Queue.c
Purpose : embOS sample program demonstrating the usage of queues.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int StackHP[128];                  /* Task stack */
static OS_TASK         TCBHP;                 /* Task-control-block */
static OS_Q            MyQueue;
static char            MyQBuffer[100];

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
  char* pData;
  int   Len;
  
  while (1) {
    Len = OS_Q_GetPtr(&MyQueue, (void**)&pData);
    OS_Delay(10);
    //
    // Evaluate Message
    //
    if (Len) {
      OS_SendString(pData);
      OS_Q_Purge(&MyQueue);
    }
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
  OS_Q_Create(&MyQueue, &MyQBuffer, sizeof(MyQBuffer));
  OS_CREATETASK(&TCBHP, "HP Task", HPTask, 150, StackHP);
  OS_SendString("embOS OS_Q example");
  OS_SendString("\n\nDemonstrating message passing\n");
  while (1) {
    OS_Q_Put(&MyQueue, "\nHello", 7);
    OS_Q_Put(&MyQueue, "\nWorld !", 9);
    OS_Delay(500);
  }
}

/****** End Of File *************************************************/
