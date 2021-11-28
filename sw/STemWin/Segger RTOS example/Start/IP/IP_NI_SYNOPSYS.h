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

File    : IP_NI_SYNOPSYS.h
Purpose : Driver specific header file for Synopsys based Ethernet controllers.
*/

#ifndef IP_NI_SYNOPSYS_H      // Avoid multiple inclusion.
#define IP_NI_SYNOPSYS_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   IP_NI_XMC45XX_NUM_RX_BUFFERS
  #define IP_NI_XMC45XX_NUM_RX_BUFFERS  (36)
#endif
#ifndef   IP_NI_XMC45XX_NUM_TX_BUFFERS
  #define IP_NI_XMC45XX_NUM_TX_BUFFERS  (1)    // Ideal configuration is one more than IP packet buffers used for sending.
#endif
#ifndef   IP_NI_XMC45XX_RX_BUFFER_SIZE
  #define IP_NI_XMC45XX_RX_BUFFER_SIZE  (128)  // 16 byte aligned to be compatible to any bus width.
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define IP_NI_XMC45XX_TX_BUFFER_SIZE  (1520)  // 16 byte aligned to be compatible to any bus width.

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  U32 EthBaseAddress[2];       // Base address of the Ethernet module(s).
  //
  //  Clock and peripheral reset management.
  //
  U32 SetMACResetAddr;         // Address of the register to activate the mac reset.
  U32 ClrMACResetAddr;         // If another register is used to clear the MAC reset. Otherwise set 0.
  U32 MACResetMask[2];         // Bit mask to activate the MAC reset.
  U32 SetClockAddr;            // Address of the register to set the clock (RX, TX, PTP...).
  U32 ClrClockAddr;            // If another register is used to clear the clock. Otherwise set 0.
  U32 ClockMask[2];            // Bits to set the clocks.
  U32 SysCfgAddr;              // Address of the register to set RMMII/MII support.
  U32 RMIIMask;                // Mask to set the support of RMII.
  //
  // Buffer and Ethernet MAC reset management.
  //
  U16 TxBufferSize;            // Size of the Tx buffer when copy is used (XMC45xx only).
  U8  UseExtendedBufferDesc;   // Use of extended buffer descriptors (for IEEE 1588 PTP support or to use one complete cache line size per buffer).
  U8  EnableResetOnError;      // Reset the buffer descriptors when errors are detected.
  //
  // Other configuration parameters.
  //
  int DriverCaps;              // Available hardware capabilities.
  U8  HasCache;                // Do we know that the MCU makes use of cache ?
  U8  Supports1GHz;            // Does the MCU have a Gigabit Ethernet MAC ?
  U32 MaccrMask;               // Additional bits for the MACCR register.
} IP_NI_SYNOPSYS_CPU_CONFIG;

typedef struct {
  U32 BufDesc0;
  U32 BufDesc1;
  U32 BufDesc2;
  U32 BufDesc3;
} IP_NI_XMC45XX_BUFFER_DESC;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_STM32F1;
extern const IP_HW_DRIVER IP_Driver_STM32F2;
extern const IP_HW_DRIVER IP_Driver_STM32F7;
extern const IP_HW_DRIVER IP_Driver_XMC45xx;
extern const IP_HW_DRIVER IP_Driver_MB9Bx10T;

//
// ST STM32F1, STM32F2, STM32F4, STM32F7
//
#define IP_Driver_STM32F107                    IP_Driver_STM32F1
#define IP_Driver_STM32F207                    IP_Driver_STM32F2
#define IP_Driver_STM32F4                      IP_Driver_STM32F2

#define IP_NI_STM32F107_ConfigNumRxBuffers(n)  IP_NI_SYNOPSYS_ConfigNumRxBuffers(0, n)
#define IP_NI_STM32F107_ConfigNumTxBuffers(n)  IP_NI_SYNOPSYS_ConfigNumTxBuffers(0, n)
#define IP_NI_STM32F207_ConfigNumRxBuffers(n)  IP_NI_SYNOPSYS_ConfigNumRxBuffers(0, n)
#define IP_NI_STM32F207_ConfigNumTxBuffers(n)  IP_NI_SYNOPSYS_ConfigNumTxBuffers(0, n)

//
// Infineon XMC45xx
//
#define IP_NI_XMC45xx_ConfigNumRxBuffers(n)  IP_NI_SYNOPSYS_ConfigNumRxBuffers(0, n)
#define IP_NI_XMC45xx_ConfigNumTxBuffers(n)  IP_NI_SYNOPSYS_ConfigNumTxBuffers(0, n)

#define IP_NI_XMC45xx_ConfigRAMAddr(Addr)    IP_NI_SYNOPSYS_ConfigEthRamAddr(0, Addr)

//
// Fujitsu MB9Bx10T
//
#define IP_NI_MB9BX10T_ConfigNumRxBuffers(i, n)  IP_NI_SYNOPSYS_ConfigNumRxBuffers(i, n)

//
// Generic functions
//
void IP_NI_SYNOPSYS_ConfigEthRamAddr  (unsigned IFaceId, U32 EthRamAddr);
void IP_NI_SYNOPSYS_ConfigNumRxBuffers(unsigned IFaceId, U16 NumRxBuffers);
void IP_NI_SYNOPSYS_ConfigNumTxBuffers(unsigned IFaceId, U16 NumRxBuffers);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
