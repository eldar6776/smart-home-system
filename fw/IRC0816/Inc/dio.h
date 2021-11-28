/**
 ******************************************************************************
 * File Name          : dio.h
 * Date               : 04/01/2018 5:22:19
 * Description        : digital input / output processing header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DIO_H
#define __DIO_H
 
 
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"


/* Typedef -------------------------------------------------------------------*/
/* Define --------------------------------------------------------------------*/
#define ERROR_NTC_WARNING_TIME			30000U	// min. 30 s ntc temperature warning activ
#define ERROR_NTC_SHUTDOWN_TIME			600000U	// min. 10 min ntc temperature shutdown activ
#define ERROR_PSU_SHUTDOWN_TIME			20000U	// error activ min. 10 s after brownout event


/* Variable ------------------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1;
extern uint8_t din_state;
extern uint8_t relay_output[2];
extern uint8_t rel[16], din[8], ter[8];
extern volatile uint32_t error_signal_flags;
extern volatile uint32_t error_signal_timer;


/* Macro ---------------------------------------------------------------------*/
#define ERROR_PwmControllerFailureSet()		(error_signal_flags |= 0x00000001U)
#define ERROR_PwmControllerFailureReset()	(error_signal_flags &= 0xfffffffeU)
#define IsERROR_PwmControllerFailureActiv()	(error_signal_flags & 0x00000001U)

#define ERROR_PwmOverloadWarningSet()		(error_signal_flags |= 0x00000002U)
#define ERROR_PwmOverloadWarningReset()		(error_signal_flags &= 0xfffffffdU)
#define IsERROR_PwmOverloadWarningActiv()	(error_signal_flags & 0x00000002U)

#define ERROR_PwmOverloadProtectionSet()	(error_signal_flags |= 0x00000004U)
#define ERROR_PwmOverloadProtectionReset()	(error_signal_flags &= 0xfffffffbU)
#define IsERROR_PwmOverloadProtectionActiv()(error_signal_flags & 0x00000004U)

#define ERROR_TemperatureSensorSet()		(error_signal_flags |= 0x00000008U)
#define ERROR_TemperatureSensorReset()		(error_signal_flags &= 0xfffffff7U)
#define IsERROR_TemperatureSensorActiv()	(error_signal_flags & 0x00000008U)

#define ERROR_RadioLinkFailureSet()			(error_signal_flags |= 0x00000010U)
#define ERROR_RadioLinkFailureReset()		(error_signal_flags &= 0xffffffefU)
#define IsERROR_RadioLinkFailureActiv()		(error_signal_flags & 0x00000010U)

#define ERROR_PowerFailureSet()				(error_signal_flags |= 0x00000020U)
#define ERROR_PowerFailureReset()			(error_signal_flags &= 0xffffffdfU)
#define IsERROR_PowerFailureActiv()			(error_signal_flags & 0x00000020U)

#define ERROR_StartTimer(TIME)				(error_signal_timer = TIME)
#define ERROR_StopTimer()					(error_signal_timer = 0U)
#define IsERROR_TimerExpired()				(error_signal_timer == 0U)

#define HC595_ShiftLatch()					((HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET)), (HAL_GPIO_WritePin (GPIOB, GPIO_PIN_10, GPIO_PIN_RESET)))
#define HC595_OutputEnable()				(HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET))
#define HC595_OutputDisable()				(HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET))


/* Function prototypes   -----------------------------------------------------*/
void DIO_Service(void);


#endif  /* __DIO_H */


/******************************   END OF FILE  ********************************/
