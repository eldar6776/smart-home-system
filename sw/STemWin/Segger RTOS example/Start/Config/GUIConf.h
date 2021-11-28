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
File        : GUIConf.h
Purpose     : Configuration of available features and default values
----------------------------------------------------------------------
*/

#ifndef GUICONF_H
#define GUICONF_H

/*********************************************************************
*
*       Multi layer/display support
*/
#define GUI_NUM_LAYERS      16   // Maximum number of available layers

/*********************************************************************
*
*       Multi tasking support
*/
#define GUI_OS              (1)  // Compile with multitasking support

/*********************************************************************
*
*       Configuration of available packages
*/
#define GUI_SUPPORT_TOUCH   (1)  // Support a touch screen (req. win-manager)
#define GUI_SUPPORT_MOUSE   (1)  // Support a mouse
#define GUI_SUPPORT_MEMDEV  (1)  // Memory devices available
#define GUI_WINSUPPORT      (1)  // Window manager package available
#define GUI_USE_ARGB        (0)  // Use color conversion defined for ST

#endif  /* Avoid multiple inclusion */

/*************************** End of file ****************************/
