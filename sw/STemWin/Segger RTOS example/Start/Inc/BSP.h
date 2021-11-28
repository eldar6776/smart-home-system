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
File        : BSP.h
Purpose     : BSP (Board support package).
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _BSP_H_     // Avoid multiple/recursive inclusion.
#define _BSP_H_  1

#include "SEGGER.h"

#if defined(__cplusplus)
extern "C" {  /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

//
// In order to avoid warnings for unused parameters.
//
#ifndef BSP_USE_PARA
  #if defined(NC30) || defined(NC308)
    #define BSP_USE_PARA(para)
  #else
    #define BSP_USE_PARA(para) para=para;
  #endif
#endif

//
// BSP UART parity.
//
#define UART_PARITY_NONE  0
#define UART_PARITY_ODD   1
#define UART_PARITY_EVEN  2

//
// Key/button status.
//
#define KEY_STAT_UP       (1 << 0)
#define KEY_STAT_DOWN     (1 << 1)
#define KEY_STAT_LEFT     (1 << 2)
#define KEY_STAT_RIGHT    (1 << 3)
#define KEY_STAT_BUTTON1  (1 << 4)
#define KEY_STAT_BUTTON2  (1 << 5)

/*********************************************************************
*
*       Functions
*
**********************************************************************
*/

/*********************************************************************
*
*       General
*/
void     BSP_Init      (void);
void     BSP_SetLED    (int Index);
void     BSP_ClrLED    (int Index);
void     BSP_ToggleLED (int Index);
unsigned BSP_GetPoti   (void);
unsigned BSP_GetKeyStat(void);

/*********************************************************************
*
*       USB
*
* Functions for USB controllers (as far as present).
*/
void BSP_USB_Attach       (void);
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
*       ETH
*
* Functions for ethernet controllers (as far as present).
*/
void BSP_ETH_Init          (unsigned Unit);
void BSP_ETH_InstallISR    (void (*pfISR)(void));
void BSP_ETH_InstallISR_Ex (int ISRIndex, void (*pfISR)(void), int Prio);
void BSP_ETH_ISR_Handler   (void);
U32  BSP_ETH_GetTransferMem(U32 *pPAddr, U32 *pVAddr);

/*********************************************************************
*
*       GUI
*
* Functions for GUI handling (as far as present).
*/
void BSP_GUI_Init(void);

/*********************************************************************
*
*       CACHE
*
* Functions for cache handling (as far as present).
*/
void BSP_CACHE_CleanInvalidateRange(void *p, unsigned NumBytes);
void BSP_CACHE_CleanRange          (void *p, unsigned NumBytes);
void BSP_CACHE_InvalidateRange     (void *p, unsigned NumBytes);

/*********************************************************************
*
*       UART
*
* Functions for UART controllers (as far as present).
*/
void BSP_UART_Init            (unsigned Unit, U32 Baudrate, U8 DataBits, U8 Parity, U8 StopBits);
void BSP_UART_DeInit          (unsigned Unit);
void BSP_UART_SetBaudrate     (unsigned Unit, U32 Baudrate);
void BSP_UART_SetReadCallback (unsigned Unit, void (*pfOnRx)(unsigned Unit, unsigned char Data));
void BSP_UART_SetWriteCallback(unsigned Unit, int (*pfOnTx)(unsigned Unit));
void BSP_UART_Write1          (unsigned Unit, U8 Data);

/*********************************************************************
*
*       SD  (used by file system)
*/

/*********************************************************************
*
*       BSP_SD_GetTransferMem()
*
* Function description
*   Delivers a memory area to be used by the SD-Card controller as transfer.
*   This function delivers the physical address and the virtual address of the tranfer memory.
*   The transfer area needs to be:
*   - Word aligned
*   - Uncached
*   - Have identical virtual and physical addresses
*   - The virtual address of the transfer area must be non-cacheable.
*   Additional requirements are that the memory used is fast enough to not block DMA transfers for too long.
*   In most systems, IRAM is used instead of external SDRAM, since the SDRAM can have relatively long latencies, primarily due to refresh cycles.
*   The size of the memory are is also returned (via pointer). It needs to be at least 512 bytes. In general, bigger values allow higher
*   performance since it allows transfer of multiple sectors without break.
*/
U32 BSP_SD_GetTransferMem(U32 *pPAddr, U32 *pVAddr);


#if defined(__cplusplus)
  }     // Make sure we have C-declarations in C++ programs
#endif

#endif  // Avoid multiple/recursive inclusion

/****** End Of File *************************************************/
