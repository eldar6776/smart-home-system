/**
 ******************************************************************************
 * File Name          : anin.c
 * Date               : 04/01/2018 5:19:19
 * Description        : analog input processing
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
uint16_t ain[2];
uint16_t psu_voltage;
uint16_t psu_shutdown_voltage;
uint16_t ntc_temperature;
uint16_t ntc_warning_temperature;
uint16_t ntc_shutdown_temperature;
uint16_t ntc_open_voltage;
uint16_t ntc_bypass_voltage;
volatile uint32_t anin_timer;
/* Macro ---------------------------------------------------------------------*/
/* Program code   ------------------------------------------------------------*/
void ANIN_Service(void)
{
	static enum	{
		ANIN_INIT = 0,
		ANIN_SUPPLY_VOLTAGE,
		ANIN_HEATSINK_TEMPERATURE	
	} ANIN_ServiceState = ANIN_INIT;
	
	ADC_ChannelConfTypeDef sConfig;
	
	switch(ANIN_ServiceState){
		/** ==========================================================================*/
		/**     A N A L O G   C O N V E R T E R     I N I T I A L I Z A T I O N		  */
		/** ==========================================================================*/
		case ANIN_INIT:
		{
			ntc_open_voltage = NTC_OPEN_VOLTAGE_DEFAULT;
			ntc_bypass_voltage = NTC_BYPASS_VOLTAGE_DEFAULT;
			psu_shutdown_voltage = SUPPLY_VOLTAGE_SHUTDOWN_DEFAULT;			
			ntc_warning_temperature = NTC_WARNING_TEMPERATURE_DEFAULT;
			ntc_shutdown_temperature = NTC_SHUTDOWN_TEMPERATURE_DEFAULT;
			ANIN_StartTimer(ANIN_STARTUP_TIME);
			ANIN_ServiceState = ANIN_SUPPLY_VOLTAGE;
			break;
		}		
		/** ==========================================================================*/
		/**   			S U P P L Y    V O L T A G E   C O N V E R S I O N   		  */
		/** ==========================================================================*/
		case ANIN_SUPPLY_VOLTAGE:
		{
			sConfig.Channel = ADC_CHANNEL_1;
			sConfig.Rank = 1;
			sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
			if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
			{
				_Error_Handler(__FILE__, __LINE__);
			}
			/**
			*	set adc chanel 1 input and start 
			*	power supply voltage value conversion
			*/
			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT);
			psu_voltage = HAL_ADC_GetValue(&hadc1);
			ain[0] = psu_voltage;
			if((psu_voltage <= psu_shutdown_voltage) && IsANIN_TimerExpired())
			{
				ERROR_PowerFailureSet();
				ERROR_StartTimer(ERROR_PSU_SHUTDOWN_TIME);
			}
			else if((psu_voltage > psu_shutdown_voltage) && IsERROR_TimerExpired() && IsANIN_TimerExpired())
			{
				ERROR_PowerFailureReset();
			}
			
			ANIN_ServiceState = ANIN_HEATSINK_TEMPERATURE;
			break;
		}
		/** ==========================================================================*/
		/**  	P W M    D R I V E R     H E A T S I N K 	 T E M P E R A T  U R E   */
		/** ==========================================================================*/
		case ANIN_HEATSINK_TEMPERATURE:
		{
			sConfig.Channel = ADC_CHANNEL_2;
			sConfig.Rank = 1U;
			sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
			if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
			{
				_Error_Handler(__FILE__, __LINE__);
			}
			/**
			*	set adc chanel 2 input and start 
			*	pwm driver heatsink teperature value conversion
			*/
			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT);
			ntc_temperature = HAL_ADC_GetValue(&hadc1);
			ain[1] = ntc_temperature;
			if((ntc_temperature < ntc_bypass_voltage)  && IsANIN_TimerExpired())
			{
				ERROR_TemperatureSensorReset();
				ERROR_PwmOverloadWarningReset();
				ERROR_PwmOverloadProtectionReset();
			}
			else if ((ntc_temperature <= ntc_shutdown_temperature) && IsANIN_TimerExpired())
			{
				ERROR_PwmOverloadProtectionSet();
				ERROR_StartTimer(ERROR_NTC_SHUTDOWN_TIME);
			}
			else if ((ntc_temperature >= ntc_open_voltage) && IsANIN_TimerExpired())
			{
				ERROR_TemperatureSensorSet();
				ERROR_StartTimer(ERROR_NTC_SHUTDOWN_TIME);
			}
			else if((ntc_temperature > ntc_shutdown_temperature) && IsERROR_TimerExpired() && IsANIN_TimerExpired())
			{
				ERROR_PwmOverloadProtectionReset();
				if (ntc_temperature <= ntc_warning_temperature)
				{
					ERROR_PwmOverloadWarningSet();
					ERROR_StartTimer(ERROR_NTC_WARNING_TIME);
				}
				else if((ntc_temperature > ntc_warning_temperature) && IsERROR_TimerExpired())
				{
					ERROR_PwmOverloadWarningReset();
				}
			}
			else if((ntc_temperature < ntc_open_voltage) && IsERROR_TimerExpired() && IsANIN_TimerExpired())
			{
				ERROR_TemperatureSensorReset();
			}

			
			ANIN_ServiceState = ANIN_SUPPLY_VOLTAGE;
			break;
		}
		
		
		default:
		{
			ANIN_ServiceState = ANIN_INIT;
			break;
		}
	}
}


/******************************   END OF FILE  *******************************/
