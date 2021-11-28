/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "common.h"
#include "LuxNET.h"
#include "TinyFrame.h"
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
extern CRC_HandleTypeDef hcrc;
extern ADC_HandleTypeDef hadc;
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void RS485_Tick(void);
void Error_Handler(void);
void PresentSystem(void);
/* Private defines -----------------------------------------------------------*/
#define EEPROM_WR_Pin GPIO_PIN_0
#define EEPROM_WR_GPIO_Port GPIOF
#define HWADDR2_Pin GPIO_PIN_1
#define HWADDR2_GPIO_Port GPIOF
#define HWADDR0_Pin GPIO_PIN_0
#define HWADDR0_GPIO_Port GPIOA
#define HWADDR1_Pin GPIO_PIN_1
#define HWADDR1_GPIO_Port GPIOB
//
/** ==========================================================================*/
/**    	 P C A 9 6 8 5    	R E G I S T E R  		A D D R E S S E	  		  */
/** ==========================================================================*/
//
#define PWM_0_15_FREQUENCY_DEFAULT			1000U		// i2c pwm controller 1 default frequency in Hertz 
#define PWM_16_31_FREQUENCY_DEFAULT			1000U		// i2c pwm controller 2 default frequency in Hertz 
#define PCA9685_REGISTER_SIZE				256U        // nuber of pca9685 registers
#define PWM_BUFFER_SIZE						16U			// number of pwm output
#define PWM_UPDATE_TIMEOUT					12U			// 20 ms pwm data transfer timeout
#define PWM_REFRESH_TIME					23U			// update pwm output every 23 ms
#define PWM_NUMBER_OF_TRIAL					34U			// try max. 50 times to accomplish pca9685 operation
#define PWM_ZERO_TRESHOLD					8U			// +/- zero treshold to start pwm
#define PWM_DEFAULT_PRESCALE				0x1eU		// prescale value for 200Hz default pwm frequncy

	
#define PCA9685_GENERAL_CALL_ACK			0x00U		// pca9685 general call address with ACK response
#define PCA9685_GENERAL_CALL_NOACK			0x01U		// pca9685 general call address without ACK response
#define PCA9685_DEFAULT_ALLCALLADR			0xe0U		// pca9685 default all device call address
#define PCA9685_DEFAULT_SUBADR_1			0xe2U		// pca9685 default subaddress 1
#define PCA9685_DEFAULT_SUBADR_2			0xe4U		// pca9685 default subaddress 2
#define PCA9685_DEFAULT_SUBADR_3			0xe2U		// pca9685 default subaddress 3
#define PCA9685_SW_RESET_COMMAND			0x06U		// i2c pwm controller reset command

#define PCA9685_MODE_1_REG_ADDRESS				0x00U
#define PCA9685_MODE_1_RESTART_BIT				(1U << 7U)
#define PCA9685_MODE_1_EXTCLK_BIT				(1U << 6U)
#define PCA9685_MODE_1_AI_BIT					(1U << 5U)
#define PCA9685_MODE_1_SLEEP_BIT				(1U << 4U)
#define PCA9685_MODE_1_SUB_1_BIT				(1U << 3U)
#define PCA9685_MODE_1_SUB_2_BIT				(1U << 2U)
#define PCA9685_MODE_1_SUB_3_BIT				(1U << 1U)
#define PCA9685_MODE_1_ALLCALL_BIT				(1U << 0U)
#define PCA9685_MODE_2_REG_ADDRESS				0x01U
#define PCA9685_MODE_2_INVRT_BIT				(1U << 4U)
#define PCA9685_MODE_2_OCH_BIT					(1U << 3U)
#define PCA9685_MODE_2_OUTDRV_BIT				(1U << 2U)
#define PCA9685_MODE_2_OUTNE_1_BIT				(1U << 1U)
#define PCA9685_MODE_2_OUTNE_0_BIT				(1U << 0U)
#define PCA9685_SUBADR_1_REG_ADDRESS			0x02U
#define PCA9685_SUBADR_2_REG_ADDRESS			0x03U
#define PCA9685_SUBADR_3_REG_ADDRESS			0x04U
#define PCA9685_ALLCALLADR_REG_ADDRESS			0x05U
#define PCA9685_LED_0_ON_L_REG_ADDRESS			0x06U
#define PCA9685_LED_0_ON_H_REG_ADDRESS			0x07U
#define PCA9685_LED_0_OFF_L_REG_ADDRESS			0x08U
#define PCA9685_LED_0_OFF_H_REG_ADDRESS			0x09U
#define PCA9685_LED_1_ON_L_REG_ADDRESS			0x0aU
#define PCA9685_LED_1_ON_H_REG_ADDRESS			0x0bU
#define PCA9685_LED_1_OFF_L_REG_ADDRESS			0x0cU
#define PCA9685_LED_1_OFF_H_REG_ADDRESS			0x0dU
#define PCA9685_LED_2_ON_L_REG_ADDRESS			0x0eU
#define PCA9685_LED_2_ON_H_REG_ADDRESS			0x0fU
#define PCA9685_LED_2_OFF_L_REG_ADDRESS			0x10U
#define PCA9685_LED_2_OFF_H_REG_ADDRESS			0x11U
#define PCA9685_LED_3_ON_L_REG_ADDRESS			0x12U
#define PCA9685_LED_3_ON_H_REG_ADDRESS			0x13U
#define PCA9685_LED_3_OFF_L_REG_ADDRESS			0x14U
#define PCA9685_LED_3_OFF_H_REG_ADDRESS			0x15U
#define PCA9685_LED_4_ON_L_REG_ADDRESS			0x16U
#define PCA9685_LED_4_ON_H_REG_ADDRESS			0x17U
#define PCA9685_LED_4_OFF_L_REG_ADDRESS			0x18U
#define PCA9685_LED_4_OFF_H_REG_ADDRESS			0x19U
#define PCA9685_LED_5_ON_L_REG_ADDRESS			0x1aU
#define PCA9685_LED_5_ON_H_REG_ADDRESS			0x1bU
#define PCA9685_LED_5_OFF_L_REG_ADDRESS			0x1cU
#define PCA9685_LED_5_OFF_H_REG_ADDRESS			0x1dU
#define PCA9685_LED_6_ON_L_REG_ADDRESS			0x1eU
#define PCA9685_LED_6_ON_H_REG_ADDRESS			0x1fU
#define PCA9685_LED_6_OFF_L_REG_ADDRESS			0x20U
#define PCA9685_LED_6_OFF_H_REG_ADDRESS			0x21U
#define PCA9685_LED_7_ON_L_REG_ADDRESS			0x22U
#define PCA9685_LED_7_ON_H_REG_ADDRESS			0x23U
#define PCA9685_LED_7_OFF_L_REG_ADDRESS			0x24U
#define PCA9685_LED_7_OFF_H_REG_ADDRESS			0x25U
#define PCA9685_LED_8_ON_L_REG_ADDRESS			0x26U
#define PCA9685_LED_8_ON_H_REG_ADDRESS			0x27U
#define PCA9685_LED_8_OFF_L_REG_ADDRESS			0x28U
#define PCA9685_LED_8_OFF_H_REG_ADDRESS			0x29U
#define PCA9685_LED_9_ON_L_REG_ADDRESS			0x2aU
#define PCA9685_LED_9_ON_H_REG_ADDRESS			0x2bU
#define PCA9685_LED_9_OFF_L_REG_ADDRESS			0x2cU
#define PCA9685_LED_9_OFF_H_REG_ADDRESS			0x2dU
#define PCA9685_LED_10_ON_L_REG_ADDRESS			0x2eU
#define PCA9685_LED_10_ON_H_REG_ADDRESS			0x2eU
#define PCA9685_LED_10_OFF_L_REG_ADDRESS		0x30U
#define PCA9685_LED_10_OFF_H_REG_ADDRESS		0x31U
#define PCA9685_LED_11_ON_L_REG_ADDRESS			0x32U
#define PCA9685_LED_11_ON_H_REG_ADDRESS			0x33U
#define PCA9685_LED_11_OFF_L_REG_ADDRESS		0x34U
#define PCA9685_LED_11_OFF_H_REG_ADDRESS		0x35U
#define PCA9685_LED_12_ON_L_REG_ADDRESS			0x36U
#define PCA9685_LED_12_ON_H_REG_ADDRESS			0x37U
#define PCA9685_LED_12_OFF_L_REG_ADDRESS		0x38U
#define PCA9685_LED_12_OFF_H_REG_ADDRESS		0x39U
#define PCA9685_LED_13_ON_L_REG_ADDRESS			0x3aU
#define PCA9685_LED_13_ON_H_REG_ADDRESS			0x3bU
#define PCA9685_LED_13_OFF_L_REG_ADDRESS		0x3cU
#define PCA9685_LED_13_OFF_H_REG_ADDRESS		0x3dU
#define PCA9685_LED_14_ON_L_REG_ADDRESS			0x3eU
#define PCA9685_LED_14_ON_H_REG_ADDRESS			0x3fU
#define PCA9685_LED_14_OFF_L_REG_ADDRESS		0x40U
#define PCA9685_LED_14_OFF_H_REG_ADDRESS		0x41U
#define PCA9685_LED_15_ON_L_REG_ADDRESS			0x42U
#define PCA9685_LED_15_ON_H_REG_ADDRESS			0x43U
#define PCA9685_LED_15_OFF_L_REG_ADDRESS		0x44U
#define PCA9685_LED_15_OFF_H_REG_ADDRESS		0x45U
/**
*	Reserved addresse for future use
*/
#define PCA9685_ALL_LED_ON_L_REG_ADDRESS		0xfaU
#define PCA9685_ALL_LED_ON_H_REG_ADDRESS		0xfbU
#define PCA9685_ALL_LED_OFF_L_REG_ADDRESS		0xfcU
#define PCA9685_ALL_LED_OFF_H_REG_ADDRESS		0xfdU
#define PCA9685_PRE_SCALE_REG_ADDRESS			0xfeU
#define PCA9685_TEST_MODE_REG_ADDRESS			0xffU
//
/** ==========================================================================*/
/**    	 P C A 9 6 8 5    	R E G I S T E R S	M N E M O N I C				  */
/** ==========================================================================*/
//
#define PCA9685_MODE_1_REGISTER					(pca9685_register[0])
#define PCA9685_MODE_2_REGISTER					(pca9685_register[1])
#define PCA9685_SUBADR_1_REGISTER				(pca9685_register[2])
#define PCA9685_SUBADR_2_REGISTER				(pca9685_register[3])
#define PCA9685_SUBADR_3_REGISTER				(pca9685_register[4])
#define PCA9685_ALLCALLADR_REGISTER				(pca9685_register[5])
#define PCA9685_LED_0_ON_L_REGISTER				(pca9685_register[6])
#define PCA9685_LED_0_ON_H_REGISTER				(pca9685_register[7])
#define PCA9685_LED_0_OFF_L_REGISTER			(pca9685_register[8])
#define PCA9685_LED_0_OFF_H_REGISTER			(pca9685_register[9])
#define PCA9685_LED_1_ON_L_REGISTER				(pca9685_register[10])
#define PCA9685_LED_1_ON_H_REGISTER				(pca9685_register[11])
#define PCA9685_LED_1_OFF_L_REGISTER			(pca9685_register[12])
#define PCA9685_LED_1_OFF_H_REGISTER			(pca9685_register[13])
#define PCA9685_LED_2_ON_L_REGISTER				(pca9685_register[14])
#define PCA9685_LED_2_ON_H_REGISTER				(pca9685_register[15])
#define PCA9685_LED_2_OFF_L_REGISTER			(pca9685_register[16])
#define PCA9685_LED_2_OFF_H_REGISTER			(pca9685_register[17])
#define PCA9685_LED_3_ON_L_REGISTER				(pca9685_register[18])
#define PCA9685_LED_3_ON_H_REGISTER				(pca9685_register[19])
#define PCA9685_LED_3_OFF_L_REGISTER			(pca9685_register[20])
#define PCA9685_LED_3_OFF_H_REGISTER			(pca9685_register[21])
#define PCA9685_LED_4_ON_L_REGISTER				(pca9685_register[22])
#define PCA9685_LED_4_ON_H_REGISTER				(pca9685_register[23])
#define PCA9685_LED_4_OFF_L_REGISTER			(pca9685_register[24])
#define PCA9685_LED_4_OFF_H_REGISTER			(pca9685_register[25])
#define PCA9685_LED_5_ON_L_REGISTER				(pca9685_register[26])
#define PCA9685_LED_5_ON_H_REGISTER				(pca9685_register[27])
#define PCA9685_LED_5_OFF_L_REGISTER			(pca9685_register[28])
#define PCA9685_LED_5_OFF_H_REGISTER			(pca9685_register[29])
#define PCA9685_LED_6_ON_L_REGISTER				(pca9685_register[30])
#define PCA9685_LED_6_ON_H_REGISTER				(pca9685_register[31])
#define PCA9685_LED_6_OFF_L_REGISTER			(pca9685_register[32])
#define PCA9685_LED_6_OFF_H_REGISTER			(pca9685_register[33])
#define PCA9685_LED_7_ON_L_REGISTER				(pca9685_register[34])
#define PCA9685_LED_7_ON_H_REGISTER				(pca9685_register[35])
#define PCA9685_LED_7_OFF_L_REGISTER			(pca9685_register[36])
#define PCA9685_LED_7_OFF_H_REGISTER			(pca9685_register[37])
#define PCA9685_LED_8_ON_L_REGISTER				(pca9685_register[38])
#define PCA9685_LED_8_ON_H_REGISTER				(pca9685_register[39])
#define PCA9685_LED_8_OFF_L_REGISTER			(pca9685_register[40])
#define PCA9685_LED_8_OFF_H_REGISTER			(pca9685_register[41])
#define PCA9685_LED_9_ON_L_REGISTER				(pca9685_register[42])			
#define PCA9685_LED_9_ON_H_REGISTER				(pca9685_register[43])			
#define PCA9685_LED_9_OFF_L_REGISTER			(pca9685_register[44])		
#define PCA9685_LED_9_OFF_H_REGISTER			(pca9685_register[45])			
#define PCA9685_LED_10_ON_L_REGISTER			(pca9685_register[46])		
#define PCA9685_LED_10_ON_H_REGISTER			(pca9685_register[47])	
#define PCA9685_LED_10_OFF_L_REGISTER			(pca9685_register[48])	
#define PCA9685_LED_10_OFF_H_REGISTER			(pca9685_register[49])		
#define PCA9685_LED_11_ON_L_REGISTER			(pca9685_register[50])			
#define PCA9685_LED_11_ON_H_REGISTER			(pca9685_register[51])		
#define PCA9685_LED_11_OFF_L_REGISTER			(pca9685_register[52])	
#define PCA9685_LED_11_OFF_H_REGISTER			(pca9685_register[53])	
#define PCA9685_LED_12_ON_L_REGISTER			(pca9685_register[54])		
#define PCA9685_LED_12_ON_H_REGISTER			(pca9685_register[55])
#define PCA9685_LED_12_OFF_L_REGISTER			(pca9685_register[56])
#define PCA9685_LED_12_OFF_H_REGISTER			(pca9685_register[57])		
#define PCA9685_LED_13_ON_L_REGISTER			(pca9685_register[58])		
#define PCA9685_LED_13_ON_H_REGISTER			(pca9685_register[59])		
#define PCA9685_LED_13_OFF_L_REGISTER			(pca9685_register[60])	
#define PCA9685_LED_13_OFF_H_REGISTER			(pca9685_register[61])	
#define PCA9685_LED_14_ON_L_REGISTER			(pca9685_register[62])		
#define PCA9685_LED_14_ON_H_REGISTER			(pca9685_register[63])		
#define PCA9685_LED_14_OFF_L_REGISTER			(pca9685_register[64])		
#define PCA9685_LED_14_OFF_H_REGISTER			(pca9685_register[65])		
#define PCA9685_LED_15_ON_L_REGISTER			(pca9685_register[66])
#define PCA9685_LED_15_ON_H_REGISTER			(pca9685_register[67])
#define PCA9685_LED_15_OFF_L_REGISTER			(pca9685_register[68])
#define PCA9685_LED_15_OFF_H_REGISTER			(pca9685_register[69])
#define PCA9685_END_1_REGISTER					(pca9685_register[70])
#define PCA9685_ALL_LED_ON_L_REGISTER			(pca9685_register[250])
#define PCA9685_ALL_LED_ON_H_REGISTER			(pca9685_register[251])
#define PCA9685_ALL_LED_OFF_L_REGISTER			(pca9685_register[252])
#define PCA9685_ALL_LED_OFF_H_REGISTER			(pca9685_register[253])
#define PCA9685_PRE_SCALE_REGISTER				(pca9685_register[254])
#define PCA9685_END_2_REGISTER					(pca9685_register[255])
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
