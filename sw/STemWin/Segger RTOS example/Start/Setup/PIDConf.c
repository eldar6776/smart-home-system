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
File        : PIDConf.c
Purpose     : Touch screen controller configuration
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "PIDConf.h"
#include "RTOS.h"
#include "TaskPrio.h"
#include "stm32746g_discovery_ts.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define XMAX 494
#define YMAX 300

#define XPOS(x) (((480 * 1000) / XMAX) * x) / 1000
#define YPOS(y) (((272 * 1000) / YMAX) * y) / 1000

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8 _IsInitialized;
static int _LayerIndex;

static OS_STACKPTR int Stack_Touch[128];
static OS_TASK TCB_TOUCH;

static void PID_X_Exec(void);
static void TouchTask(void);

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       HAL_Delay
*
*  Function description
*    overwrite HAL_Delay from stm32f7xx_hal.c
*/
void HAL_Delay(__IO uint32_t Delay) {
  OS_Delay(Delay);
}

/*********************************************************************
*
*       TouchTask
*/
static void TouchTask(void) {
  while(1) {
    PID_X_Exec();
    OS_Delay(25);
  }
}

/*********************************************************************
*
*       PID_X_Exec
*/
static void PID_X_Exec(void) {
  TS_StateTypeDef TS_State;
  static GUI_PID_STATE StatePID;
  static int IsTouched;
  static int ySize;

  if (ySize == 0) {
    ySize = LCD_GetYSizeEx(0);
  }
  if (_IsInitialized) {
    BSP_TS_GetState(&TS_State);
    StatePID.Layer = _LayerIndex;
    if (TS_State.touchDetected) {
      IsTouched = 1;
      StatePID.Pressed = 1;
      StatePID.x = (int)(TS_State.touchX[0]);
      StatePID.y = (int)(TS_State.touchY[0]);
      GUI_PID_StoreState(&StatePID);
    } else {
      if (IsTouched == 1) {
        IsTouched = 0;
        StatePID.Pressed = 0;
        GUI_PID_StoreState(&StatePID);
      }
    }
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       PID_X_SetLayerIndex
*/
void PID_X_SetLayerIndex(int LayerIndex) {
  _LayerIndex = LayerIndex;
}

/*********************************************************************
*
*       PID_X_Init
*/
void PID_X_Init(void) {
  int xSize, ySize;

  if (_IsInitialized == 0) {
    xSize = LCD_GetXSize();
    ySize = LCD_GetYSize();
    BSP_TS_Init(xSize, ySize);
    OS_CREATETASK(&TCB_TOUCH, "TouchTask", TouchTask, TASKPRIO_TOUCH, Stack_Touch);
    _IsInitialized = 1;
  }
}

/*************************** End of file ****************************/
