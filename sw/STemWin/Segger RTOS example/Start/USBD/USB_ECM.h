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
File    : USB_ECM.h
Purpose : Public header of the USB Ethernet Control Model (ECM)
          The Ethernet Control Model (ECM) is one of the 
          Communication Device Class protocols defined by usb.org to
          create a virtual Ethernet connection between a USB
          device and a host PC.
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USB_ECM_H          /* Avoid multiple inclusion */
#define USB_ECM_H

#include "USB.h"
#include "SEGGER.h"
#include "USB_Driver_IP_NI.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       defines
*
**********************************************************************
*/

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       USB_ECM_INIT_DATA
*/
typedef struct USB_ECM_INIT_DATA {
  U8   EPIn;      // Endpoint to send data packets to USB host
  U8   EPOut;     // Endpoint to receive data packets from USB host
  U8   EPInt;     // Endpoint to send notifications to USB host
  const USB_IP_NI_DRIVER_API * pDriverAPI;   // Network interface driver API
  USB_IP_NI_DRIVER_DATA        DriverData;   // Data passed at initialization to low-level driver
} USB_ECM_INIT_DATA;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void USBD_ECM_Add  (const USB_ECM_INIT_DATA * pInitData);
void USBD_ECM_Task (void);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
