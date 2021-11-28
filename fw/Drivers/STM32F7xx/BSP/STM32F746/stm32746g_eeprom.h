/**
  ******************************************************************************
  * @file    stm32746g_discovery_eeprom.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for 
  *          the stm32746g_discovery_eeprom.c firmware driver.
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
#ifndef __EEPROM_H__                        
#define __EEPROM_H__                        FW_BUILD // version

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32746g.h"


/* EEPROM hardware address and page size */ 
#define EE_PGSIZE                           64
#define EE_PGNUM                            0x100  // number of pages
#define EE_MAXSIZE                          0x4000 /* 64Kbit */
#define EE_ENDADDR                          0x3FFF    
#define EE_ADDR                             0xA0
#define EETOUT                              1000
#define EE_WR_TIME                          15
#define EE_TRIALS                           200
#define EE_MAX_TRIALS                       3000
#define EE_OK                               0x0
#define EE_FAIL                             0x1
#define EE_TOUT                             0x2
#define EE_MARKER                           0x55

#define EE_TERMFL                           0x0    // first group of system flags
#define EE_MIN_SETPOINT                     0x4
#define EE_MAX_SETPOINT                     0x5
#define EE_THST_SETPOINT                    0x6
#define EE_NTC_OFFSET                       0x7
#define EE_DISP_LOW_BCKLGHT                 0xA
#define EE_DISP_HIGH_BCKLGHT                0xB
#define EE_SCRNSVR_TOUT                     0xC
#define EE_SCRNSVR_ENABLE_HOUR              0xD
#define EE_SCRNSVR_DISABLE_HOUR             0xE
#define EE_SCRNSVR_CLK_COLOR                0xF
#define EE_SCRNSVR_ON_OFF                   0x10    
#define EE_SYS_STATE                        0x11
#define EE_FW_UPDATE_BYTE_CNT               0x14	// firmware update byte count
#define EE_FW_UPDATE_STATUS                 0x18	// firmware update status
#define EE_ROOM_TEMP_SP                     0x19	// room setpoint temp  in degree of Celsious
#define EE_ROOM_TEMP_DIFF		            0x1B	// room tempreature on / off difference
#define EE_TFIFA			                0x20	// tinyframe interface address
#define EE_TFGRA				            0x21	// tinyframe group  address
#define EE_TFBRA			                0x22	// tinyframe broadcast address
#define EE_TFGWA                            0x23    // tinyframe gateway address
#define EE_TFBPS					        0x24	// tinyframe interface baudrate
#define EE_SYSID				            0x28	// system id (system unique number)
#define EE_BLIND_TOUT                       0x2A
#define EE_CTRL1                            0x30
#define EE_CTRL2                            0x50
#define EE_THST1                            0x70
#define EE_INIT_ADDR                        0x90    // value 0xA5U written if default value written
/* Link function for I2C EEPROM peripheral */
void     EE_Init         (void);
uint32_t EE_ReadBuffer   (uint8_t *pBuffer, uint16_t ReadAddr,  uint16_t NumByteToRead);
uint32_t EE_WriteBuffer  (uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);


#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H__ */

/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
