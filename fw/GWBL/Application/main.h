
#ifndef __MAIN_H__
#define __MAIN_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "common.h"

#define TASTER_S2_PORT				GPIOA
#define TASTER_S2_PIN				GPIO_Pin_0
#define TASTER_S2_CLK				RCC_AHB1Periph_GPIOA
#define TASTER_S3_PORT				GPIOD
#define TASTER_S3_PIN				GPIO_Pin_11
#define TASTER_S3_CLK				RCC_AHB1Periph_GPIOD
#define LED_GPIO_PORT           	GPIOD
#define LED1_GPIO_PIN          		GPIO_Pin_11
#define LED2_GPIO_PIN          		GPIO_Pin_12
#define LED_GPIO_CLK           		RCC_AHB1Periph_GPIOD
 

#define BootloaderActivate()					((system_config &= 0x0f),(system_config |= 0x80))
#define BootloaderDeactivate()					(system_config &= 0x0f)
#define IsBootloaderRequested()					(system_config & 0x80)
#define BootloaderSDcardError()					((system_config &= 0x0f),(system_config |= 0x40))
#define BootloaderUpdateSucces()				((system_config &= 0x0f),(system_config |= 0x20))
#define BootloaderFileError()					((system_config &= 0x0f),(system_config |= 0x10))
#define LED1_Toggle()   (LED_GPIO_PORT->ODR ^= LED1_GPIO_PIN)
#define LED2_Toggle()   (LED_GPIO_PORT->ODR ^= LED2_GPIO_PIN)
#define BUZZER_On()     (GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET))
#define BUZZER_Off()    (GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET))
#define RMII_MODE
extern __IO uint32_t TimingDelay;
extern uint32_t system_config;

uint32_t get_systick(void);
void Fail_Handler(void);
void ApplicationExe(void);
void Delay(__IO uint32_t nTime);

#endif 
