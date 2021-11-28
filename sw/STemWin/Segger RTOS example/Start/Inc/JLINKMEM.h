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
File    : JLINKMEM.h
Purpose : Header file for J-Link ARM communication using memory
---------------------------END-OF-HEADER------------------------------
*/

#ifndef JLINKMEM_H
#define JLINKMEM_H             // Avoid multiple inclusion

#ifdef __cplusplus
extern "C" {
#endif

void JLINKMEM_Process(void);
void JLINKMEM_SetpfOnRx(void (* pf)(unsigned char Data));
void JLINKMEM_SetpfOnTx(void (* pf)(void));
void JLINKMEM_SetpfGetNextChar(OS_INT (* pf)(void));
void JLINKMEM_SendChar(unsigned char Data);

#ifdef __cplusplus
}
#endif

#endif                         // Avoid multiple inclusion

/*************************** end of file ****************************/

