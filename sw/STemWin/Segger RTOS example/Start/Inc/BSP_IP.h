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
-------------------------- END-OF-HEADER -----------------------------

File    : BSP_IP.h
Purpose : Header file for IP related BSP functions.
*/

#ifndef BSP_IP_H              // Avoid multiple inclusion.
#define BSP_IP_H

#include "IP.h"               // For IP_BSP_API structure.

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

extern const IP_BSP_API BSP_IP_Api;  // Default BSP API.

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
