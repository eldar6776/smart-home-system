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
File    : USB_PrinterClass.h
Purpose : Sample implementation of USB printer device class
----------Literature--------------------------------------------------
Universal Serial Bus Device Class Definition for Printing Devices
Version 1.1 January 2000
--------  END-OF-HEADER  ---------------------------------------------
*/
#ifndef USB_PRINTERCLASS_H__
#define USB_PRINTERCLASS_H__

#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef struct {
  const char * (*pfGetDeviceIdString)(void);
  int          (*pfOnDataReceived)(const U8 * pData, unsigned NumBytes);
  U8           (*pfGetHasNoError)(void);
  U8           (*pfGetIsSelected)(void);
  U8           (*pfGetIsPaperEmpty)(void);
  void         (*pfOnReset)(void);
} USB_PRINTER_API;

void   USB_PRINTER_Init        (USB_PRINTER_API * pAPI);
void   USB_PRINTER_Task        (void);
int    USB_PRINTER_Read        (      void * pData, unsigned NumBytes);
int    USB_PRINTER_ReadTimed   (      void * pData, unsigned NumBytes, unsigned ms);
int    USB_PRINTER_Receive     (      void * pData, unsigned NumBytes);
int    USB_PRINTER_ReceiveTimed(      void * pData, unsigned NumBytes, unsigned ms);
int    USB_PRINTER_Write       (const void * pData, unsigned NumBytes);
int    USB_PRINTER_WriteTimed  (const void * pData, unsigned NumBytes, unsigned ms);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif


#endif

/*************************** End of file ****************************/

