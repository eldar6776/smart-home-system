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
File    : Modbus_Master_IFace.h
Purpose : Global types etc & general purpose utility functions
---------------------------END-OF-HEADER------------------------------
*/

#ifndef MODBUS_MASTER_IFACE_H            // Guard against multiple inclusion
#define MODBUS_MASTER_IFACE_H

#include "Global.h"         // Type definitions: U8, U16, U32, I8, I16, I32
#include "MB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Variables
*
**********************************************************************
*/

extern const MB_IFACE_UART_API _IFaceRTU;
extern const MB_IFACE_UART_API _IFaceASCII;
extern const MB_IFACE_IP_API   _IFaceTCP;

extern       U8                _ConnectCnt;
extern       MB_CHANNEL        _MBChannel;

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif                      // Avoid multiple inclusion

/*************************** End of file ****************************/
