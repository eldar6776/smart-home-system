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
File    : BSP.c
Purpose : BSP for SEGGER emPower V2 with Freescale K66 MCU
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "BSP_USB.h"
#include "SEGGER.h"
#include "RTOS.h"
#include "stm32f7xx.h"     // Device specific header file, contains CMSIS

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/


/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define USB_HS_ISR_ID                           (77)
#define USB_FS_ISR_ID                           (67)


/*********************************************************************
*
*       Typedefs
*
**********************************************************************
*/

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

typedef void USB_ISR_HANDLER  (void);


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static USB_ISR_HANDLER * _pfUSBISRHandler;
static USB_ISR_HANDLER * _pfUSBHISRHandler_FS;
static USB_ISR_HANDLER * _pfUSBHISRHandler_HS;

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/****** Declare ISR handler here to avoid "no prototype" warning. They are not declared in any CMSIS header */

#ifdef __cplusplus
extern "C" {
#endif
void OTG_FS_IRQHandler(void);
void OTG_HS_IRQHandler(void);
#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       OTG_FS_IRQHandler
*/
void OTG_FS_IRQHandler(void) {
  OS_EnterInterrupt(); // Inform embOS that interrupt code is running
  if (_pfUSBISRHandler) {
    (_pfUSBISRHandler)();
  }
  if (_pfUSBHISRHandler_FS) {
    (_pfUSBHISRHandler_FS)();
  }
  OS_LeaveInterrupt(); // Inform embOS that interrupt code is left
}

/*********************************************************************
*
*       OTG_HS_IRQHandler
*/
void OTG_HS_IRQHandler(void) {
  OS_EnterInterrupt(); // Inform embOS that interrupt code is running
  if (_pfUSBISRHandler) {
    (_pfUSBISRHandler)();
  }
  if (_pfUSBHISRHandler_HS) {
    (_pfUSBHISRHandler_HS)();
  }
  OS_LeaveInterrupt(); // Inform embOS that interrupt code is left
}

/*********************************************************************
*
*       BSP_USB_InstallISR_Ex()
*/
void BSP_USB_InstallISR_Ex(int ISRIndex, void (*pfISR)(void), int Prio){
  _pfUSBISRHandler = pfISR;

  NVIC_SetPriority((IRQn_Type)ISRIndex, (uint32_t)Prio);
  NVIC_EnableIRQ((IRQn_Type)ISRIndex);
}

/*********************************************************************
*
*       BSP_USBH_InstallISR_Ex
*/
void BSP_USBH_InstallISR_Ex(int ISRIndex, void (*pfISR)(void), int Prio){
  if (ISRIndex == USB_HS_ISR_ID) {
    _pfUSBHISRHandler_HS = pfISR;
  } else {
    _pfUSBHISRHandler_FS = pfISR;
  }
  NVIC_SetPriority((IRQn_Type)ISRIndex, (uint32_t)Prio);
  NVIC_EnableIRQ((IRQn_Type)ISRIndex);
}

/****** End Of File *************************************************/
