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

File    : BSP_IP.c
Purpose : BSP for embOS/IP on ST STM32F40G-EVAL with ST STM32F407 MCU.
*/

#include "BSP_IP.h"
#include "RTOS.h"       // For OS_Enter/LeaveInterrupt() .

#include "stm32746g_discovery_sdram.h"

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

typedef void ETH_ISR_HANDLER(void);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static ETH_ISR_HANDLER* _pfEthISRHandler;

/*********************************************************************
*
*       Prototypes
*
*  Declare ISR handler here to avoid "no prototype" warning.
*  They are not declared in any CMSIS header.
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

void ETH_IRQHandler(void);

#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _Init()
*
*  Function description
*    This routine is called from the Ethernet driver to initialize
*    port pins and enable clocks required for Ethernet.
*
*  Parameters
*    IFaceId: Zero-based interface index.
*
*  Additional information
*    Port         Pin function
*      PA1          RMII_REF_CLK
*      PA2          RMII_MDIO
*      PA7          RMII_CRS_DV
*      PC1          RMII_MDC
*      PC4          RMII_RXD0
*      PC5          RMII_RXD1
*      PG2          RMII_RXER
*      PG11         RMII_TX_EN
*      PG13         RMII_TXD0
*      PG14         RMII_TXD1
*/
static void _Init(unsigned IFaceId) {
  unsigned int v;

  IP_USE_PARA(IFaceId);

  //
  // Enable I/O clocks
  //
  RCC->AHB1ENR |= 0
                | (1u <<  6u)  // GPIOAEN: IO port G clock enable
                | (1u <<  2u)  // GPIOAEN: IO port C clock enable
                | (1u <<  0u)  // GPIOAEN: IO port A clock enable
                ;
  //
  // Set PA1 (RMII_REF_CLK) as alternate function
  //
  v              = GPIOA->MODER;
  v             &= ~(0x3uL << (2u * 1u));
  v             |=  (0x2uL << (2u * 1u));
  GPIOA->MODER   = v;
  v              = GPIOA->AFR[0];
  v             &= ~(0xFuL << (4u * 1u));
  v             |=  (0xBuL << (4u * 1u));
  GPIOA->AFR[0]  = v;
  //
  // Set PA2 (RMII_MDIO) as alternate function
  //
  v              = GPIOA->MODER;
  v             &= ~(0x3uL << (2u * 2u));
  v             |=  (0x2uL << (2u * 2u));
  GPIOA->MODER   = v;
  v              = GPIOA->AFR[0];
  v             &= ~(0xFuL << (4u * 2u));
  v             |=  (0xBuL << (4u * 2u));
  GPIOA->AFR[0]  = v;
  //
  // Set PA7 (RMII_CRS_DV) as alternate function
  //
  v              = GPIOA->MODER;
  v             &= ~(0x3uL << (2u * 7u));
  v             |=  (0x2uL << (2u * 7u));
  GPIOA->MODER   = v;
  v              = GPIOA->AFR[0];
  v             &= ~(0xFuL << (4u * 7u));
  v             |=  (0xBuL << (4u * 7u));
  GPIOA->AFR[0]  = v;
  //
  // Set PC1 (RMII_MDC) as alternate function
  //
  v              = GPIOC->MODER;
  v             &= ~(0x3uL << (2u * 1u));
  v             |=  (0x2uL << (2u * 1u));
  GPIOC->MODER   = v;
  v              = GPIOC->AFR[0];
  v             &= ~(0xFuL << (4u * 1u));
  v             |=  (0xBuL << (4u * 1u));
  GPIOC->AFR[0]  = v;
  //
  // Set PC4 (RMII_RXD0) as alternate function
  //
  v              = GPIOC->MODER;
  v             &= ~(0x3uL << (2u * 4u));
  v             |=  (0x2uL << (2u * 4u));
  GPIOC->MODER   = v;
  v              = GPIOC->AFR[0];
  v             &= ~(0xFuL << (4u * 4u));
  v             |=  (0xBuL << (4u * 4u));
  GPIOC->AFR[0]  = v;
  //
  // Set PC5 (RMII_RXD1) as alternate function
  //
  v              = GPIOC->MODER;
  v             &= ~(0x3uL << (2u * 5u));
  v             |=  (0x2uL << (2u * 5u));
  GPIOC->MODER   = v;
  v              = GPIOC->AFR[0];
  v             &= ~(0xFuL << (4u * 5u));
  v             |=  (0xBuL << (4u * 5u));
  GPIOC->AFR[0]  = v;
  //
  // Set PG2 (RMII_RXER) as alternate function
  //
  v              = GPIOG->MODER;
  v             &= ~(0x3uL << (2u * 2u));
  v             |=  (0x2uL << (2u * 2u));
  GPIOG->MODER   = v;
  v              = GPIOG->AFR[0];
  v             &= ~(0xFuL << (4u * 2u));
  v             |=  (0xBuL << (4u * 2u));
  GPIOG->AFR[0]  = v;
  //
  // Set PG11 (RMII_TX_EN) as alternate function speed 100MHz
  //
  v               = GPIOG->MODER;
  v              &= ~(0x3uL << (2 * 11));
  v              |=  (0x2uL << (2 * 11));
  GPIOG->MODER    = v;
  GPIOG->OSPEEDR |= (0x3uL << (2 * 11));
  v               = GPIOG->AFR[1];
  v              &= ~(0xFuL << (4 * 3));
  v              |=  (0xBuL << (4 * 3));
  GPIOG->AFR[1]   = v;
  //
  // Set PG13 (RMII_TXD0) as alternate function speed 100MHz
  //
  v               = GPIOG->MODER;
  v              &= ~(0x3uL << (2 * 13));
  v              |=  (0x2uL << (2 * 13));
  GPIOG->MODER    = v;
  GPIOG->OSPEEDR |= (0x3uL << (2 * 13));
  v               = GPIOG->AFR[1];
  v              &= ~(0xFuL << (4 * 5));
  v              |=  (0xBuL << (4 * 5));
  GPIOG->AFR[1]   = v;
  //
  // Set PG14 (ETH_RMII_TXD1) as alternate function speed 100MHz
  //
  v               = GPIOG->MODER;
  v              &= ~(0x3uL << (2 * 14));
  v              |=  (0x2uL << (2 * 14));
  GPIOG->MODER    = v;
  GPIOG->OSPEEDR |= (0x3uL << (2 * 14));
  v               = GPIOG->AFR[1];
  v              &= ~(0xFuL << (4 * 6));
  v              |=  (0xBuL << (4 * 6));
  GPIOG->AFR[1]   = v;
}

/*********************************************************************
*
*       _InstallISR()
*
*  Function description
*    Installs the IP stack interrupt handler for Ethernet.
*
*  Parameters
*    IFaceId: Zero-based interface index.
*    pPara  : Parameters for the interrupt handler to install:
*               - pfISR   : Pointer to the interrupt handler of the
*                           IP stack.
*               - ISRIndex: Default interrupt index as far as known by
*                           the driver.
*                           For drivers that fit multiple devices this
*                           might not always be the correct index and
*                           should then be replaced by the correct
*                           index within this function.
*                - Prio   : Suggested priority by the IP stack. This
*                           is only a suggestion and should be
*                           replaced with the desired priority within
*                           this function.
*/
static void _InstallISR(unsigned IFaceId, IP_BSP_INSTALL_ISR_PARA* pPara) {
  IP_USE_PARA(IFaceId);

  _pfEthISRHandler = pPara->pfISR;
  NVIC_SetPriority(ETH_IRQn, (1 << __NVIC_PRIO_BITS) - 2);
  NVIC_EnableIRQ(ETH_IRQn);
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       ETH_IRQHandler()
*
*  Function description
*    Ethernet interrupt handler.
*
*  Additional information
*    Needs to inform the OS that we are now in interrupt context.
*/
void ETH_IRQHandler(void) {
  OS_EnterInterrupt();
  if (_pfEthISRHandler) {
    (_pfEthISRHandler)();
  }
  OS_LeaveInterrupt();
}

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/

const IP_BSP_API BSP_IP_Api = {
  _Init,       // pfInit
  NULL,        // pfDeInit
  _InstallISR  // pfInstallISR
};

/*************************** End of file ****************************/
