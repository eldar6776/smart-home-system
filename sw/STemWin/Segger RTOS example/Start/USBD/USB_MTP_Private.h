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
File    : USB_MTP_Private.h
Purpose : Private header of USB MTP (Media Transfer Protocol)
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef MTP_PRIVATE_H
#define MTP_PRIVATE_H

#include "USB_MTP.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       File system types
*/
#define FS_TYPE_UNDEFINED               0x0000
#define FS_TYPE_GENERIC_FLAT            0x0001
#define FS_TYPE_GENERIC_HIERACHICAL     0x0002

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
