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
File    : UDPCOM.h
Purpose : Header file for embOSView communication using UDP
---------------------------END-OF-HEADER------------------------------
*/

#ifndef UDPCOM_H
#define UDPCOM_H             // Avoid multiple inclusion

void UDP_Process_Init (void);
void UDP_Process_Send1(char c);

#endif                         // Avoid multiple inclusion

/****** End Of File *************************************************/


