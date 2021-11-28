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
File    : OS_SWTimer.c
Purpose : embOS sample program running two simple software timer.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include "BSP.h"

static OS_TIMER TIMER50, TIMER200;

static void Timer50(void) {
  BSP_ToggleLED(0);
  OS_RetriggerTimer(&TIMER50);
}

static void Timer200(void) {
  BSP_ToggleLED(1);
  OS_RetriggerTimer(&TIMER200);
}

/*********************************************************************
*
*       main()
*/
int main(void) {
  OS_IncDI();                      /* Initially disable interrupts  */
  OS_InitKern();                   /* Initialize OS                 */
  OS_InitHW();                     /* Initialize Hardware for OS    */
  BSP_Init();                      /* Initialize LED ports          */
  OS_CREATETIMER(&TIMER50,  Timer50,   50);
  OS_CREATETIMER(&TIMER200, Timer200, 200);
  OS_Start();                      /* Start multitasking            */
  return 0;
}

/****** End Of File *************************************************/
