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
File    : GUI_HelloWorld.c
Purpose : emWin and embOS demo application
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       main()
*/
void MainTask(void);
void MainTask(void) {
  GUI_Init();
  GUI_DispString("Hello World!");
  while(1) {
    GUI_Delay(500);
  }
}

/****** End of File *************************************************/
