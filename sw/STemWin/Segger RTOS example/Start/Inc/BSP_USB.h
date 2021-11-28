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
File        : BSP_USBD.h
Purpose     : BSP (Board support package) for USB device.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _BSP_USBD_H_     // Avoid multiple/recursive inclusion.
#define _BSP_USBD_H_  1

#if defined(__cplusplus)
extern "C" {  /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

/*********************************************************************
*
*       USBD
*
* Functions for USB device controllers (as far as present).
*/
void BSP_USB_InstallISR   (void (*pfISR)(void));
void BSP_USB_InstallISR_Ex(int ISRIndex, void (*pfISR)(void), int Prio);
void BSP_USB_ISR_Handler  (void);
void BSP_USB_Init         (void);

/*********************************************************************
*
*       USBH
*
* Functions for USB Host controllers (as far as present).
*/
void BSP_USBH_InstallISR   (void (*pfISR)(void));
void BSP_USBH_InstallISR_Ex(int ISRIndex, void (*pfISR)(void), int Prio);
void BSP_USBH_Init         (void);

/*********************************************************************
*
*       CACHE
*
* Functions for cache handling (as far as present).
*/
void BSP_CACHE_CleanInvalidateRange(void *p, unsigned NumBytes);
void BSP_CACHE_CleanRange          (void *p, unsigned NumBytes);
void BSP_CACHE_InvalidateRange     (void *p, unsigned NumBytes);


#if defined(__cplusplus)
  }     // Make sure we have C-declarations in C++ programs
#endif

#endif  // Avoid multiple/recursive inclusion

/****** End Of File *************************************************/
