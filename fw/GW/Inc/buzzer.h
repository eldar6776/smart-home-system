/**
 ******************************************************************************
 * File Name          : buzzer.h
 * Date               : 10-September-2016 20:00:16
 * Description        : onboard buzzer control modul header
 ******************************************************************************
 */
#ifndef BUZZER_H
#define BUZZER_H   			100	// version 1.00

/* Includes ------------------------------------------------------------------*/

/* Exported defines    -------------------------------------------------------*/

/* Exported types    ---------------------------------------------------------*/

/* Exported variables  -------------------------------------------------------*/
//extern volatile uint32_t buzzer_timer;
//extern volatile uint32_t buzzer_flags;

/* Exported macros     -------------------------------------------------------*/
//#define BUZZER_StartTimer(TIME)	((signal_timer = TIME), (signal_flags &= 0xfffffffe))
//#define BUZZER_StopTimer()		(signal_flags |= 0x00000001)
//#define IsBUZZER_TimerExpired()	(signal_flags & 0x00000001)

/* Exported functions  -------------------------------------------------------*/
void BUZZER_Init(void);
void BUZZER_Off(void);
void BUZZER_On(void);
void BUZZER_Service(void);

#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
