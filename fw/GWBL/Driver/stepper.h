/**
 ******************************************************************************
 * File Name          : stepper.h
 * Date               : 08/05/2016 23:16:19
 * Description        : stepper motor control modul header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#ifndef STEPPER_H
#define STEPPER_H 						100	// version 1.00

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported defines    -------------------------------------------------------*/
/* Exported types  -----------------------------------------------------------*/
/* Exported variables  -------------------------------------------------------*/
/* Exported macros     -------------------------------------------------------*/
#define BarrierDriveClockAssert()				GPIO_ResetBits(BARRIER_MOTOR_CLOCK_PORT, BARRIER_MOTOR_CLOCK_PIN)
#define BarrierDriveClockRelease()				GPIO_SetBits(BARRIER_MOTOR_CLOCK_PORT, BARRIER_MOTOR_CLOCK_PIN)


/* Exported functions  -------------------------------------------------------*/
void STEPPER_Init(void);

#endif
/******************************   END OF FILE  **********************************/
