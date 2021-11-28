/*
 * =====================================================================================
 *
 *       Filename:  GUI_X_LINUX_SIM.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/03/2011 04:58:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  caicry@gmail.com
 *        Company:  
 *
 * =====================================================================================
 */


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
#define LCD_FILE_PATH  "/tmp/lcdMem"
/*
*********************************************************************************************************
*                                             uC/GUI V3.98
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              礐/GUI is protected by international copyright laws. Knowledge of the
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

