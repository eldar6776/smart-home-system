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
File        : GUIDRV_1611.h
Purpose     : Interface definition for GUIDRV_1611 driver
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUIDRV_1611_H
#define GUIDRV_1611_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Display drivers
*/
//
// Addresses
//
extern const GUI_DEVICE_API GUIDRV_Win_API;

extern const GUI_DEVICE_API GUIDRV_1611_API;

//
// Macros to be used in configuration files
//
#if defined(WIN32) && !defined(LCD_SIMCONTROLLER)

  #define GUIDRV_1611             &GUIDRV_Win_API

#else

  #define GUIDRV_1611             &GUIDRV_1611_API

#endif

#if defined(__cplusplus)
}
#endif

#endif

/*************************** End of file ****************************/
