/**
 ******************************************************************************
 * File Name          : dio.h
 * Date               : 08/05/2016 23:16:19
 * Description        : digital io interface modul header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#ifndef DIO_H
#define DIO_H     						100	// version 1.00

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported defines    -------------------------------------------------------*/
#define RED_SIGNAL_TIME		5000
#define GREEN_SIGNAL_TIME	5000
#define DIO_TIMEOUT     	0x3000	// Timeout Zeit

/* Defines    ----------------------------------------------------------------*/
#define TASTER_S2_PORT				GPIOA
#define TASTER_S2_PIN				GPIO_Pin_0
#define TASTER_S2_CLK				RCC_AHB1Periph_GPIOA

#define BEEP_GPIO_PORT        		GPIOD
#define BEEP_GPIO_PIN       		GPIO_Pin_3
#define BEEP_GPIO_CLK       		RCC_AHB1Periph_GPIOD

#define TASTER_S3_PORT				GPIOD
#define TASTER_S3_PIN				GPIO_Pin_11
#define TASTER_S3_CLK				RCC_AHB1Periph_GPIOD

#define LED_GPIO_PORT           	GPIOD
#define LED1_GPIO_PIN          		GPIO_Pin_11
#define LED2_GPIO_PIN          		GPIO_Pin_12
#define LED_GPIO_CLK           		RCC_AHB1Periph_GPIOD
/* Exported variables  -------------------------------------------------------*/
/* Exported macros     -------------------------------------------------------*/
/* Exported functions  -------------------------------------------------------*/
void DIO_Init(void);
void LED_Toggle(uint8_t led);
void BUZZER_Off(void);
void BUZZER_On(void);
#endif
/******************************   END OF FILE  **********************************/
