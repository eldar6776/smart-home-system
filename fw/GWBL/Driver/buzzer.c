/**
 ******************************************************************************
 * File Name          : buzzer.h
 * Date               : 10-September-2016 20:00:16
 * Description        : onboard buzzer control modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "buzzer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//volatile uint32_t buzzer_timer;
//volatile uint32_t buzzer_flags;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void BUZZER_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD , ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD, GPIO_Pin_3);
}

void BUZZER_Off(void)
{
	GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);
}

void BUZZER_On(void)
{
	GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
}

void BUZZER_Service(void)
{
	
}
