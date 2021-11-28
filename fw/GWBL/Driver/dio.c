/**
 ******************************************************************************
 * File Name          : dio.c
 * Date               : 28/02/2016 23:16:19
 * Description        : digital io interface software modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
/* Includes ------------------------------------------------------------------*/
#include "dio.h"

/* Variables  ----------------------------------------------------------------*/


/* Defines    ----------------------------------------------------------------*/


/* Types  --------------------------------------------------------------------*/
/* Macros     ----------------------------------------------------------------*/
/* Private prototypes    -----------------------------------------------------*/
/* Program code   ------------------------------------------------------------*/
void DIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;

//-----------------------------------------------------------	onboard leds	

	RCC_AHB1PeriphClockCmd(LED_GPIO_CLK , ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = LED1_GPIO_PIN | LED2_GPIO_PIN;
	GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(LED_GPIO_PORT,LED1_GPIO_PIN|LED2_GPIO_PIN);

//-----------------------------------------------------------	onboard buzzer

	RCC_AHB1PeriphClockCmd(BEEP_GPIO_CLK , ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = BEEP_GPIO_PIN;
	GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(BEEP_GPIO_PORT,BEEP_GPIO_PIN);
	
//-----------------------------------------------------------	stacker step motor


	
//-----------------------------------------------------------	barrier motor clock output

	
	
	
	RCC_AHB1PeriphClockCmd(TASTER_S2_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = TASTER_S2_PIN;
	GPIO_Init(TASTER_S2_PORT, &GPIO_InitStructure);                   
    
    RCC_AHB1PeriphClockCmd(TASTER_S3_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = TASTER_S3_PIN;
	GPIO_Init(TASTER_S3_PORT, &GPIO_InitStructure); 
}


/**
  * @brief  翻转端口电平
  * @param  选择相应LED 1-LED1 2-LED2
  * @retval None
  */
void LED_Toggle(uint8_t led)
{
	if (led == 1) 
	{
		LED_GPIO_PORT->ODR ^= LED1_GPIO_PIN;	 
	} 
	else if (led == 2) 
	{
		LED_GPIO_PORT->ODR ^= LED2_GPIO_PIN; 
	}
}


void BUZZER_Off(void)
{
	GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);
}

void BUZZER_On(void)
{
	GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
}


/******************************   END OF FILE  **********************************/
