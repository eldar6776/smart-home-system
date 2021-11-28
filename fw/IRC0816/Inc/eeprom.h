/**
 ******************************************************************************
 * File Name          : eeprom.h
 * Date               : 28/02/2016 23:16:19
 * Description        : eeprom memory manager modul header
 ******************************************************************************
*
* DISPLAY           pins    ->  STM32F103 Rubicon controller
* ----------------------------------------------------------------------------
* DISPLAY   +3V3    pin 1   ->  controller +3V3
* DISPLAY   GND     pin 2   ->  controller VSS
* DISPLAY   CS      pin 3   ->  PA8
* DISPLAY   RST     pin 4   ->  PA3
* DISPLAY   DC      pin 5   ->  PA2
* DISPLAY   MOSI    pin 6   ->  PA7 - SPI1 MOSI
* DISPLAY   SCK     pin 7   ->  PA5 - SPI1 SCK
* DISPLAY   LED     pin 8   ->  PB7 - PWM TIM4 CH2
* DISPLAY   MISO    pin 9   ->  PA6 - SPI1 MISO
* SD CARD   CS      pin 10  ->  PA4
* 
*
******************************************************************************
*/
#ifndef __EEPROM_H__
#define __EEPROM_H__					    FW_BUILD	// version

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
/* Defines    ----------------------------------------------------------------*/
#define EEADDR                      0xAE
#define EEDENS                      131072          // available memory bytes
#define EEPGNM                      512             // number of pages
#define EEPGSZ                      256             // number of bytes per page
#define EETOUT     				    20			    // 20 ms eeprom operation timeout
#define EETIME					    100			    // number of i2c operation trials
#define EE_INIT_MARK                0x5A
/** ==========================================================================*/
/**     	S Y S T E M   C O N F I G   M E M O R Y   A D D R E S S E      	  */
/** ==========================================================================*/
#define EE_SYS_STATE                                0x2
#define EE_FW_UPDATE_BYTE_CNT                       0xA	    // firmware update byte count
#define EE_FW_UPDATE_STATUS                         0xE	    // firmware update status
#define EE_TFIFA			                        0x10	// tinyframe interface address
#define EE_TFGRA				                    0x11	// tinyframe group address
#define EE_TFBRA			                        0x12	// tinyframe broadcast address
#define EE_TFGWA                                    0x13    // tinyframe gateway address
#define EE_TFBPS					                0x14	// tinyframe interface baudrate
#define EE_SYSID				                    0x18	// system id (system unique number)
#define EE_NTC_BYPASS_VOLTAGE_ADDRESS				0x1A
#define EE_NTC_OPEN_VOLTAGE_ADDRESS					0x1C
#define EE_NTC_WARNING_TEMPERATURE_ADDRESS			0x1E
#define EE_NTC_SHUTDOWN_TEMPERATURE_ADDRESS			0x20
#define EE_SUPPLY_VOLTAGE_SHUTDOWN_ADDRESS			0x22
#define EE_PWM_0_15_FREQUENCY_ADDRESS				0x24
#define EE_PWM_16_31_FREQUENCY_ADDRESS				0x26
#define EE_HC12_BAUDRATE_ADDRESS					0x28
#define EE_HC12_CHANEL_ADDRESS						0x2A
#define EE_HC12_LINK_TIMEOUT_ADDRESS				0x2C
#define EE_NRF24L01_BAUDRATE_ADDRESS				0x2E
#define EE_NRF24L01_CHANEL_ADDRESS					0x30
#define EE_NRF24L01_LINK_TIMEOUT_ADDRESS			0x32

#define EE_TFTYPE1_ADDR                             0x40    // registered type listener for relay 0
#define EE_TFTYPE2_ADDR                             0x41    // registered type listener for relay 1
#define EE_TFTYPE3_ADDR                             0x42    // registered type listener for relay 2
#define EE_TFTYPE4_ADDR                             0x43    // registered type listener for relay 3
#define EE_TFTYPE5_ADDR                             0x44    // registered type listener for relay 4
#define EE_TFTYPE6_ADDR                             0x45    // registered type listener for relay 5
#define EE_TFTYPE7_ADDR                             0x46    // registered type listener for relay 6
#define EE_TFTYPE8_ADDR                             0x47    // registered type listener for relay 7
#define EE_TFTYPE9_ADDR                             0x48    // registered type listener for relay 8
#define EE_TFTYPE10_ADDR                            0x49    // registered type listener for relay 9
#define EE_TFTYPE11_ADDR                            0x4A    // registered type listener for relay 10
#define EE_TFTYPE12_ADDR                            0x4B    // registered type listener for relay 11
#define EE_TFTYPE13_ADDR                            0x4C    // registered type listener for relay 12
#define EE_TFTYPE14_ADDR                            0x4D    // registered type listener for relay 13
#define EE_TFTYPE15_ADDR                            0x4E    // registered type listener for relay 14
#define EE_TFTYPE16_ADDR                            0x4F    // registered type listener for relay 15

#define EE_INIT_ADDR                                0x50    // value 0xA5U written if default value written
#define EE_LOG_LIST_START_ADDR                      0x400
#define EE_LOG_LIST_END_ADDR                        (EEDENS-1)
/* Types  --------------------------------------------------------------------*/
/* Variables  ----------------------------------------------------------------*/
/* Macros   ------------------------------------------------------------------*/
/* Function prototypes    ---------------------------------------------------*/
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
