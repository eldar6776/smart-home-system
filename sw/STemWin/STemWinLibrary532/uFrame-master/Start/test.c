#include "GUI.h"
#include "PROGBAR.h"
#include "WM.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#if 0
/*
*********************************************************************************************************
*                                             uC/GUI V3.98
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
---Author-Explanation
* 
* 1.00.00 020519 JJL    First release of uC/GUI to uC/OS-II interface
* 
*
* Known problems or limitations with current version
*
*    None.
*
*
* Open issues
*
*    None
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         GLOBAL VARIABLES
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        TIMING FUNCTIONS
*
* Notes: Some timing dependent routines of uC/GUI require a GetTime and delay funtion. 
*        Default time unit (tick), normally is 1 ms.
*********************************************************************************************************
*/

int  GUI_X_GetTime (void) 
{
	  struct timespec tv; 
		int now;

		clock_gettime(CLOCK_MONOTONIC, &tv);

		now = tv.tv_sec*1000 + (tv.tv_nsec) / 1.0E6;
		return now;
  
}

// ms
void  GUI_X_Delay (int period) 
{
    //usleep(1);
}


/*
*********************************************************************************************************
*                                          GUI_X_ExecIdle()
*********************************************************************************************************
*/
void GUI_X_ExecIdle (void) 
{
    //sleep(1);
    
}


/*
*********************************************************************************************************
*                                    MULTITASKING INTERFACE FUNCTIONS
*
* Note(1): 1) The following routines are required only if uC/GUI is used in a true multi task environment, 
*             which means you have more than one thread using the uC/GUI API.  In this case the #define 
*             GUI_OS 1   needs to be in GUIConf.h
*********************************************************************************************************
*/

void  GUI_X_InitOS (void)
{ 
	//todo
}


void  GUI_X_Lock (void)
{ 
   // mutex lock
}


void  GUI_X_Unlock (void)
{ 
	// mutex unlock
}


U32  GUI_X_GetTaskId (void) 
{ 
  // thread id
  return 1;
}

/*
*********************************************************************************************************
*                                        GUI_X_WaitEvent()
*                                        GUI_X_SignalEvent()
*********************************************************************************************************
*/


void GUI_X_WaitEvent (void) 
{
    // wait message 
}


void GUI_X_SignalEvent (void) 
{
    // singal event?
}

/*
*********************************************************************************************************
*                                      KEYBOARD INTERFACE FUNCTIONS
*
* Purpose: The keyboard routines are required only by some widgets.
*          If widgets are not used, they may be eliminated.
*
* Note(s): If uC/OS-II is used, characters typed into the log window will be placed	in the keyboard buffer. 
*          This is a neat feature which allows you to operate your target system without having to use or 
*          even to have a keyboard connected to it. (useful for demos !)
*********************************************************************************************************
*/

static  void  CheckInit (void) 
{
    
}


void GUI_X_Init (void) 
{
    
}


int  GUI_X_GetKey (void) 
{
    return 1;
}


int  GUI_X_WaitKey (void) 
{
    // wait key
    return 1;
}


void  GUI_X_StoreKey (int k) 
{
    // store key?
    
}

void GUI_X_Log(const char *ptr)
{
	printf("%s\n", ptr);
}

void GUI_X_ErrorOut(const char *s)
{
	printf("%s\n", s);
}

void GUI_X_Warn(const char *s)
{
	printf("%s\n", s);
}
#endif


extern const GUI_BITMAP bmMicriumLogo;
extern const GUI_BITMAP bmMicriumLogo_1bpp;


int main(int argc, char **argv)
{
	int LCDXSize = LCD_GET_XSIZE();
  int LCDYSize = LCD_GET_YSIZE();
  const GUI_BITMAP *pBitmap;
  int i, YPos;
  
  GUI_Init();
  GUI_SetBkColor(GUI_BLACK); 
  GUI_SetColor(GUI_WHITE);
  GUI_Clear();

  PROGBAR_Handle hProgBar;
  GUI_DispStringAt("Progress bar", 80, 20);
  hProgBar = PROGBAR_Create(65, 40, 100, 20, WM_CF_SHOW);
  //PROGBAR_SetBarColor(hProgBar, 0, GUI_GREEN);
  //PROGBAR_SetBarColor(hProgBar, 1, GUI_RED);
  PROGBAR_SetValue(hProgBar, 25);
  WM_Exec();
return 0;
 
  GUI_Delay(1000);
  GUI_SetBkColor(GUI_BLUE); GUI_Clear();
  GUI_Delay(1000);
  GUI_SetColor(GUI_WHITE);
  for (i=0; i<1000; i+=10) {
    GUI_DrawHLine(i,0,100);
    GUI_DispStringAt("Line ",0,i);
    GUI_DispDecMin(i);
  }
  GUI_Delay(1000);
  GUI_SetColor(0x0);
  GUI_SetBkColor(0xffffff);
  for (i=0; i<160; i++) {
    int len = (i<80) ? i : 160-i;
    GUI_DrawHLine(i,20,len+20);
  }
  GUI_Delay(1000);
  GUI_Clear();
  if (LCD_GET_YSIZE()>(100+bmMicriumLogo_1bpp.YSize)) {
    pBitmap=&bmMicriumLogo;
  } else {
    GUI_SetColor(GUI_BLUE);
    pBitmap=&bmMicriumLogo_1bpp;
  }
  GUI_DrawBitmap(pBitmap,(LCDXSize-pBitmap->XSize)/2,10);
  YPos=20+pBitmap->YSize;
  GUI_SetFont(&GUI_FontComic24B_1);
  GUI_DispStringHCenterAt("www.micrium.com",LCDXSize/2,YPos);
  GUI_Delay(1000);
  GUI_SetColor(GUI_RED);
  GUI_DispStringHCenterAt("2005\n", LCDXSize/2,YPos+30);
  GUI_SetFont(&GUI_Font10S_1);
  GUI_DispStringHCenterAt("Micri Inc.",LCDXSize/2,YPos+60);;
  GUI_Delay(1000);

	return 0;
}
