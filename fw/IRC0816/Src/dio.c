/**
 ******************************************************************************
 * File Name          : dio.c
 * Date               : 04/01/2018 5:22:19
 * Description        : digital input / output processing
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dio.h"
#include "pwm.h"
#include "anin.h"
#include "rs485.h"
/* Typedef -------------------------------------------------------------------*/
/* Define --------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
uint8_t din_state;
uint8_t relay_output[2];
volatile uint32_t error_signal_flags;
volatile uint32_t error_signal_timer;
uint8_t rel[16];
uint8_t din[8];
uint8_t ter[8];
/* Macro ---------------------------------------------------------------------*/
/* Program code   ------------------------------------------------------------*/
void DIO_Service(void)
{
	uint8_t j;
	
	static enum {
		DIO_INIT = 0,
		DIO_SERVICE			
	} DIN_ServiceState = DIO_INIT;	
	
	switch(DIN_ServiceState){
		/** ==========================================================================*/
		/**    		D I G I T A L    I / O   	I N I T I A L I Z A T I O N			  */
		/** ==========================================================================*/
		case DIO_INIT:
		{
			HC595_OutputDisable();
			relay_output[0] = 0U;
			relay_output[1] = 0U;
			HAL_SPI_Transmit(&hspi1, relay_output, 2U, 10U);
			HC595_ShiftLatch();
			HC595_OutputEnable();
			DIN_ServiceState = DIO_SERVICE;
			break;
		}		
		/** ==========================================================================*/
		/**  		D I G I T A L   I N P U T  	&  	R E L A Y   O U P U T    		  */
		/** ==========================================================================*/		
		case DIO_SERVICE:
		{
			/**
			*	get digital input state
			*/
			din_state = 0U;			
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_SET) din_state |= 0x01U;
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) == GPIO_PIN_SET) din_state |= 0x02U;
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13) == GPIO_PIN_SET) din_state |= 0x04U;
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14) == GPIO_PIN_SET) din_state |= 0x08U;
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_SET) din_state |= 0x10U;
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)  == GPIO_PIN_SET) din_state |= 0x20U;
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)  == GPIO_PIN_SET) din_state |= 0x40U;
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)  == GPIO_PIN_SET) din_state |= 0x80U;
			/**
			*	set loop ventil state
			*/
			for(j = 0U; j  < 16; j++){
                if (j > 7){
                    if(rel[j] != 0) relay_output[1] |= (1U<<(j-8));
                    else relay_output[1] &= (~(1U<<(j-8)));
                }
                else{
                    if(rel[j] != 0) relay_output[0] |= (1U<<j);
                    else relay_output[0] &= (~(1U<<j));
                    
                    if (din_state & (1U<<j)) din[j] = '1';
                    else din[j] = '0';
                }
			}			
			HAL_SPI_Transmit(&hspi1, relay_output, 2U, 10U);
			HC595_ShiftLatch();
			break;
		}
		
		
		default:
		{
			DIN_ServiceState = DIO_INIT;
			break;
		}
	}
}


/******************************   END OF FILE  ********************************/
