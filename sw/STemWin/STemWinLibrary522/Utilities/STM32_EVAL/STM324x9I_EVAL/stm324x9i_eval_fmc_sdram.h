/**
  ******************************************************************************
  * @file    stm324x9i_eval_fmc_sdram.h
  * @author  MCD Application Team
  * @version V1.0.4
  * @date    22-September-2016
  * @brief   This file contains all the functions prototypes for the 
  *          stm324x9i_eval_fmc_sdram.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */   

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM324X9I_EVAL_FMC_SDRAM_H
#define __STM324X9I_EVAL_FMC_SDRAM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm324x9i_eval.h"


/** @defgroup STM324x9I_EVAL_FMC_SDRAM_Private_Defines
  * @{
  */

/**
  * @brief  FMC SDRAM Bank address
  */   
#define SDRAM_BANK_ADDR     ((uint32_t)0xC0000000)
/**
  * @}
  */  
  
/**
  * @brief  FMC SDRAM Memory Width
  */  
/* #define SDRAM_MEMORY_WIDTH   FMC_SDMemory_Width_8b  */
/* #define SDRAM_MEMORY_WIDTH    FMC_SDMemory_Width_16b */
#define SDRAM_MEMORY_WIDTH    FMC_SDMemory_Width_32b  /* Default configuration used with LCD */
/**
  * @}
  */  

/**
  * @brief  FMC SDRAM Memory clock period
  */  
#define SDCLOCK_PERIOD    FMC_SDClock_Period_2        /* Default configuration used with LCD */
/* #define SDCLOCK_PERIOD    FMC_SDClock_Period_3 */
/**
  * @}
  */  


/**
  * @brief  FMC SDRAM Bank Remap
  */    
/* #define SDRAM_BANK_REMAP */   
/**
  * @}
  */ 

/**
  * @brief  FMC SDRAM Timeout
  */    
#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)  
/**
  * @}
  */
/**
  * @}
  */       
  


/**
 * @brief Uncomment the line below if you want to use user defined Delay function
 *        (for precise timing), otherwise default _delay_ function defined within
 *         this driver is used (less precise timing).  
 */
 
/* #define USE_Delay */

#ifdef USE_Delay
  #define __Delay     Delay      /*  User can provide more timing precise __Delay function
                                    (with 10ms time base), using SysTick for example */
#else
  #define __Delay     delay      /*  Default __Delay function with less precise timing */
#endif

/**
  * @}
  */ 

/**
  * @brief  FMC SDRAM Mode definition register defines
  */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000) 
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)      

/**
  * @}
  */  

/** @defgroup STM324x9I_EVAL_FMC_SDRAM_Exported_Functions
  * @{
  */ 
void  SDRAM_Init(void);
void  SDRAM_GPIOConfig(void);
void  SDRAM_InitSequence(void);
void  SDRAM_WriteBuffer(uint32_t* pBuffer, uint32_t uwWriteAddress, uint32_t uwBufferSize);
void  SDRAM_ReadBuffer(uint32_t* pBuffer, uint32_t uwReadAddress, uint32_t uwBufferSize);

#ifdef __cplusplus
}
#endif

#endif /* __STM324X9I_EVAL_FMC_SDRAM_H */

/**
  * @}
  */ 
  
  

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
