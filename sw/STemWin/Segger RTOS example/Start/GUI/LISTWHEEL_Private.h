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
File        : LISTWHEEL_Private.h
Purpose     : Private LISTWHEEL include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef LISTWHEEL_PRIVATE_H
#define LISTWHEEL_PRIVATE_H

#include "LISTWHEEL.h"
#include "WM.h"
#include "GUI_ARRAY.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define LISTWHEEL_STATE_PRESSED WIDGET_STATE_USER0

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/

typedef struct {
  void * pData;
  char   acText[1];
} LISTWHEEL_ITEM;

typedef struct {
  const GUI_FONT * pFont;
  GUI_COLOR        aBackColor[2];
  GUI_COLOR        aTextColor[2];
  I16              Align;
  unsigned         Deceleration;
} LISTWHEEL_PROPS;

typedef struct {
  WIDGET                  Widget;
  GUI_ARRAY               ItemArray;
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
  LISTWHEEL_PROPS         Props;
  WM_HMEM                 hTimer;
  unsigned                LBorder;
  unsigned                RBorder;
  unsigned                LineHeight;
  int                     Sel;
  GUI_TIMER_TIME          TimeTouched;      // Time stamp of last touch event
  GUI_TIMER_TIME          TimeTouchedLast;  // Time of the last touch
  int                     PosTouchedLast;   // Last touched position in pixels
  int                     Pos;              // Current position in pixels
  int                     Velocity;         // Motion in pixels
  int                     SnapPosition;     // Snap position in pixels
  int                     TouchPos;         // Y-position of last touch event
  int                     ySizeData;        // Data size in pixels
  int                     Destination;      // Destination position in pixels
  GUI_TIMER_TIME          TimerPeriod;      // Period of timer events
} LISTWHEEL_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define LISTWHEEL_INIT_ID(p)  (p->Widget.DebugId = LISTWHEEL_ID)
#else
  #define LISTWHEEL_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  LISTWHEEL_OBJ * LISTWHEEL_LockH(LISTWHEEL_Handle h);
  #define LISTWHEEL_LOCK_H(h)   LISTWHEEL_LockH(h)
#else
  #define LISTWHEEL_LOCK_H(h)   (LISTWHEEL_OBJ *)GUI_LOCK_H(h)
#endif

/*********************************************************************
*
*       Private (module internal) data
*
**********************************************************************
*/
extern LISTWHEEL_PROPS LISTWHEEL_DefaultProps;

/*********************************************************************
*
*       Private (module internal) functions
*
**********************************************************************
*/
const char * LISTWHEEL__GetpStringLocked(LISTWHEEL_Handle hObj, int Index, LISTWHEEL_ITEM ** ppItem);

#endif // GUI_WINSUPPORT
#endif // LISTWHEEL_PRIVATE_H

/*************************** End of file ****************************/
