/**
 ******************************************************************************
 * File Name          : anin.h
 * Date               : 04/01/2018 5:21:19
 * Description        : analog input processing header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ANIN_H
#define __ANIN_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
/* Typedef -------------------------------------------------------------------*/
/* Define --------------------------------------------------------------------*/
#define ADC_TIMEOUT							5U			// 5 ms timeout for ad conversion
#define ANIN_STARTUP_TIME					100U        // 100 ms analog signal settling time
#define NTC_OPEN_VOLTAGE_DEFAULT			0x0bc0U		// NTC sensor open min. voltage 
#define NTC_BYPASS_VOLTAGE_DEFAULT			0x0010U     // NTC sensor short max. voltage 
#define NTC_WARNING_TEMPERATURE_DEFAULT		0x0589U		// NTC temperatue warning limit value  
#define NTC_SHUTDOWN_TEMPERATURE_DEFAULT	0x0422U		// NTC temperatue shutdown limit value 
#define SUPPLY_VOLTAGE_SHUTDOWN_DEFAULT		0x0480U		// power supply voltage shutdown limit value 
/* Variable ------------------------------------------------------------------*/
extern uint16_t ain[2];
extern uint16_t psu_voltage;
extern uint16_t psu_shutdown_voltage;
extern uint16_t ntc_temperature;
extern uint16_t ntc_warning_temperature;
extern uint16_t ntc_shutdown_temperature;
extern uint16_t ntc_open_voltage;
extern uint16_t ntc_bypass_voltage;
extern volatile uint32_t anin_timer;
/* Macro ---------------------------------------------------------------------*/
#define ANIN_StartTimer(ANIN_TIME)				(anin_timer = ANIN_TIME)
#define ANIN_StopTimer()						(anin_timer = 0U)
#define IsANIN_TimerExpired()					(anin_timer == 0U)
/* Function prototypes   -----------------------------------------------------*/
void ANIN_Service(void);
 
#endif  /* __ANIN_H */
 
 
/******************************   END OF FILE  *******************************/

