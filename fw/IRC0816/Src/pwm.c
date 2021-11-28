/**
 ******************************************************************************
 * File Name          : pwm.c
 * Date               : 04/01/2018 5:26:19
 * Description        : pwm ouput processing
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
/* Variable ------------------------------------------------------------------*/
uint16_t pwm_0_15_freq;
uint16_t pwm_16_31_freq;
volatile uint32_t pwm_flags;
volatile uint32_t pwm_timer;
uint8_t pwm[PWM_BUFFER_SIZE];
uint8_t pca9685_register[PCA9685_REGISTER_SIZE];
PWM_ServiceStateTypeDef PWM_ServiceState = PWM_INIT;

extern I2C_HandleTypeDef hi2c1;
/* Typedef -------------------------------------------------------------------*/
/* Define --------------------------------------------------------------------*/
/* Macro ---------------------------------------------------------------------*/
/* Program code   ------------------------------------------------------------*/
void PWM_Service(void)
{
	uint8_t i;

	switch(PWM_ServiceState)
	{
		/** ==========================================================================*/
		/**     P C A 9 6 8 5     M O D U L E		I N I T I A L I Z A T I O N		  */
		/** ==========================================================================*/
		case PWM_INIT:
		{
			PWM_Initialized();
			ZEROFILL(pwm, PWM_BUFFER_SIZE);
			ZEROFILL(pca9685_register, PCA9685_REGISTER_SIZE);
			PCA9685_Reset();
			PCA9685_Init();
			if(IsPWM_Initialized()) PCA9685_SetOutputFrequency(PWM_0_15_FREQUENCY_DEFAULT);		
			PWM_ServiceState = PWM_STOP;
			break;
		}
		/** ==========================================================================*/
		/**     P W M 		O U T P U T 	S O T W A R E		S H U T D O W N		  */
		/** ==========================================================================*/
		case PWM_STOP:
		{
			if(!IsPWM_Initialized()) break;
			
			ZEROFILL(pwm, PWM_BUFFER_SIZE);
			
			if(!IsERROR_PwmControllerFailureActiv() 		&&	\
				!IsERROR_PwmOverloadWarningActiv()			&&	\
				!IsERROR_PwmOverloadProtectionActiv() 		&&	\
				!IsERROR_TemperatureSensorActiv()			&&	\
				!IsERROR_RadioLinkFailureActiv()			&&	\
				!IsERROR_PowerFailureActiv())
			{
				PWM_ServiceState = PWM_UPDATE;
			}
			else
			{
				PCA9685_OutputUpdate();
				PWM_OuputDisable();
			}
			break;
		}
		/** ==========================================================================*/
		/**     			P W M 		O U T P U T 	U P D A T E		  			  */
		/** ==========================================================================*/
		case PWM_UPDATE:
		{
			if(IsERROR_PwmControllerFailureActiv() 		||	\
				IsERROR_PwmOverloadProtectionActiv() 	||	\
				IsERROR_TemperatureSensorActiv()		||	\
				IsERROR_RadioLinkFailureActiv()			||	\
				IsERROR_PowerFailureActiv())
			{
				PWM_ServiceState = PWM_STOP;
			}
			else if(IsPWM_TimerExpired())
			{
				for(i = 0U; i < PWM_BUFFER_SIZE; i++)
				{
					if(pwm[i] < PWM_ZERO_TRESHOLD) pwm[i] = 0U;
				}
				
				PCA9685_OutputUpdate();
				PWM_OuputEnable();
				PWM_StartTimer(PWM_REFRESH_TIME);
			}
			break;
		}
		
		
		default:
		{
			PWM_ServiceState = PWM_INIT;
			break;
		}		
	}	
}


void PCA9685_Init(void)
{
	uint8_t buf[2];

	buf[0] = 0x00U;
	buf[1]= 0x00U;
	
	if(HAL_I2C_Master_Transmit(&hi2c1, PWM_0_15_I2C_WRITE_ADDRESS, buf, 2U, PWM_UPDATE_TIMEOUT) != HAL_OK)
	{
		PWM_NotInitialized();
	}
	
	HAL_Delay(5);
	buf[0] = 0x01U;
	buf[1]= 0x04U;
	
	if(HAL_I2C_Master_Transmit(&hi2c1, PWM_0_15_I2C_WRITE_ADDRESS, buf, 2U, PWM_UPDATE_TIMEOUT) != HAL_OK)
	{
		PWM_NotInitialized();
	}
	
	HAL_Delay(2);
}


void PCA9685_Reset(void)
{	
	uint8_t cmd = PCA9685_SW_RESET_COMMAND;
	
	if(HAL_I2C_Master_Transmit(&hi2c1, PCA9685_GENERAL_CALL_ACK, &cmd, 1U, PWM_UPDATE_TIMEOUT) != HAL_OK)
	{
		PWM_NotInitialized();
	}
}


void PCA9685_SetOutputFrequency(uint16_t frequency)
{
	uint8_t buf[2];
	
	buf[0] = 0x00U;
	buf[1]= 0x10U;
	
	if(HAL_I2C_Master_Transmit(&hi2c1, PWM_0_15_I2C_WRITE_ADDRESS, buf, 2U, PWM_UPDATE_TIMEOUT) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);	
	}
	
	PWM_CalculatePrescale(frequency);	
	buf[0] = PCA9685_PRE_SCALE_REG_ADDRESS;
	buf[1]= PCA9685_PRE_SCALE_REGISTER;
	
	if(HAL_I2C_Master_Transmit(&hi2c1, PWM_0_15_I2C_WRITE_ADDRESS, buf, 2U, PWM_UPDATE_TIMEOUT) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);	
	}
	
	buf[0] = 0x00U;
	buf[1]= 0xa0U;
	
	if(HAL_I2C_Master_Transmit(&hi2c1, PWM_0_15_I2C_WRITE_ADDRESS, buf, 2U, PWM_UPDATE_TIMEOUT) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);	
	}
	
	HAL_Delay(5);
	buf[0] = 0x01U;
	buf[1]= 0x04U;
	
	if(HAL_I2C_Master_Transmit(&hi2c1, PWM_0_15_I2C_WRITE_ADDRESS, buf, 2U, PWM_UPDATE_TIMEOUT) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);	
	}
}


void PCA9685_WriteIncremental(uint8_t first_address, uint8_t lenght)
{
	uint8_t buf[70],i;
	
	buf[0] = first_address;
	
	for(i = 1U; i < (lenght + 1U); i++)
	{
		buf[i] = pca9685_register[first_address++];
	}
	
	if(HAL_I2C_Master_Transmit(&hi2c1, PWM_0_15_I2C_WRITE_ADDRESS, buf, lenght + 1U, PWM_UPDATE_TIMEOUT) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);	
	}
}


void PCA9685_OutputUpdate(void)
{
	uint16_t pwm_out;
	
	PCA9685_LED_0_ON_L_REGISTER = 0U;
	PCA9685_LED_0_ON_H_REGISTER = 0U;
	PCA9685_LED_1_ON_L_REGISTER = 0U;
	PCA9685_LED_1_ON_H_REGISTER = 0U;
	PCA9685_LED_2_ON_L_REGISTER = 0U;
	PCA9685_LED_2_ON_H_REGISTER = 0U;
	PCA9685_LED_3_ON_L_REGISTER = 0U;
	PCA9685_LED_3_ON_H_REGISTER = 0U;
	PCA9685_LED_4_ON_L_REGISTER = 0U;
	PCA9685_LED_4_ON_H_REGISTER = 0U;
	PCA9685_LED_5_ON_L_REGISTER = 0U;
	PCA9685_LED_5_ON_H_REGISTER = 0U;
	PCA9685_LED_6_ON_L_REGISTER = 0U;
	PCA9685_LED_6_ON_H_REGISTER = 0U;
	PCA9685_LED_7_ON_L_REGISTER = 0U;
	PCA9685_LED_7_ON_H_REGISTER = 0U;
	PCA9685_LED_8_ON_L_REGISTER = 0U;
	PCA9685_LED_8_ON_H_REGISTER = 0U;
	PCA9685_LED_9_ON_L_REGISTER = 0U;
	PCA9685_LED_9_ON_H_REGISTER = 0U;
	PCA9685_LED_10_ON_L_REGISTER = 0U;
	PCA9685_LED_10_ON_H_REGISTER = 0U;
	PCA9685_LED_11_ON_L_REGISTER = 0U;
	PCA9685_LED_11_ON_H_REGISTER = 0U;
	PCA9685_LED_12_ON_L_REGISTER = 0U;
	PCA9685_LED_12_ON_H_REGISTER = 0U;
	PCA9685_LED_13_ON_L_REGISTER = 0U;
	PCA9685_LED_13_ON_H_REGISTER = 0U;
	PCA9685_LED_14_ON_L_REGISTER = 0U;
	PCA9685_LED_14_ON_H_REGISTER = 0U;
	PCA9685_LED_15_ON_L_REGISTER = 0U;
	PCA9685_LED_15_ON_H_REGISTER = 0U;
	
	pwm_out= pwm[0] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_0_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_0_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[1] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_1_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_1_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[2] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_2_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_2_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[3] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_3_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_3_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[4] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_4_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_4_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[5] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_5_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_5_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[6] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_6_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_6_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[7] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_7_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_7_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[8] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_8_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_8_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[9] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_9_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_9_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[10] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_10_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_10_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[11] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_11_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_11_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[12] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_12_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_12_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[13] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_13_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_13_OFF_H_REGISTER = (pwm_out >> 8U);
	
	pwm_out= pwm[14] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_14_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_14_OFF_H_REGISTER = (pwm_out >> 8U);
		
	pwm_out= pwm[15] * 16U;
	if(pwm_out > 4070U) pwm_out = 4095U;	
	PCA9685_LED_15_OFF_L_REGISTER = (pwm_out & 0xffU);
	PCA9685_LED_15_OFF_H_REGISTER = (pwm_out >> 8U);
	
	PCA9685_WriteIncremental(PCA9685_LED_0_ON_L_REG_ADDRESS, 64U);
}



/******************************   END OF FILE  ********************************/ 
