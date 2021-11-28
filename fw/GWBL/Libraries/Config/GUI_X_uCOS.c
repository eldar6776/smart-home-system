/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2013  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.20 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  NXP Semiconductors USA, Inc.  whose
registered  office  is  situated  at 411 E. Plumeria Drive, San  Jose,
CA 95134, USA  solely for  the  purposes  of  creating  libraries  for
NXPs M0, M3/M4 and  ARM7/9 processor-based  devices,  sublicensed  and
distributed under the terms and conditions of the NXP End User License
Agreement.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUI_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/

#include <ucos_ii.h>
#include "os_cfg.h"
#include "os_cpu.h"
#include <stdio.h>
#include "GUI.h"

/*
*********************************************************************************************************
*                                         GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_EVENT  *DispSem;
static  OS_EVENT  *EventMbox;

static  OS_EVENT  *KeySem;
static  int        KeyPressed;
static  char       KeyIsInited;


/*********************************************************************
*
*      Timing:
*                 GUI_X_GetTime()
*                 GUI_X_Delay(int)

  Some timing dependent routines require a GetTime
  and delay function. Default time unit (tick), normally is
1 ms.
*/

int GUI_X_GetTime(void) {
    return ((int)OSTimeGet());
}

void GUI_X_Delay(int period) {
    INT32U  ticks;


    ticks = (period * 1000) / OS_TICKS_PER_SEC;
    OSTimeDly((INT16U)ticks);

}

/*********************************************************************
*
*       GUI_X_Init()
*
* Note:
*     GUI_X_Init() is called from GUI_Init is a possibility to init
*     some hardware which needs to be up and running before the GUI.
*     If not required, leave this routine blank.
*/

void GUI_X_Init(void) {
    KeySem = OSSemCreate(0);
}


/*********************************************************************
*
*       GUI_X_ExecIdle
*
* Note:
*  Called if WM is in idle state
*/

void GUI_X_ExecIdle(void) 
{
	GUI_X_Delay(1);
}

/*********************************************************************
*
*      Multitasking:
*
*                 GUI_X_InitOS()
*                 GUI_X_GetTaskId()
*                 GUI_X_Lock()
*                 GUI_X_Unlock()
*
* Note:
*   The following routines are required only if emWin is used in a
*   true multi task environment, which means you have more than one
*   thread using the emWin API.
*   In this case the
*                       #define GUI_OS 1
*  needs to be in GUIConf.h
*/


//static OS_RSEMA RSema;

void GUI_X_InitOS(void)    
{
    DispSem   = OSSemCreate(1);
    EventMbox = OSMboxCreate((void *)0);
}
void GUI_X_Unlock(void)
{ 
    OSSemPost(DispSem);
}

void GUI_X_Lock(void)      
{
    INT8U  err;
    
    
    OSSemPend(DispSem, 0, &err);
}

U32  GUI_X_GetTaskId(void) 
{
    return ((U32)(OSTCBCur->OSTCBPrio));
}


/*
*********************************************************************************************************
*                                        GUI_X_WaitEvent()
*                                        GUI_X_SignalEvent()
*********************************************************************************************************
*/


void GUI_X_WaitEvent (void) 
{
    INT8U  err;


    (void)OSMboxPend(EventMbox, 0, &err);
}


void GUI_X_SignalEvent (void) 
{
    (void)OSMboxPost(EventMbox, (void *)1);
}


/*********************************************************************
*
*      Logging: OS dependent

Note:
  Logging is used in higher debug levels only. The typical target
  build does not use logging and does therefor not require any of
  the logging routines below. For a release build without logging
  the routines below may be eliminated to save some space.
  (If the linker is not function aware and eliminates unreferenced
  functions automatically)

*/

void GUI_X_Log     (const char *s) { GUI_USE_PARA(s); }
void GUI_X_Warn    (const char *s) { GUI_USE_PARA(s); }
void GUI_X_ErrorOut(const char *s) { GUI_USE_PARA(s); }

/*************************** End of file ****************************/
