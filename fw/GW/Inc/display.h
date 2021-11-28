/**
 ******************************************************************************
 * File Name          : display.h
 * Date               : 21/08/2016 20:59:16
 * Description        : display GUI header file
 ******************************************************************************
 *
 *
 ******************************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISP_H__
#define __DISP_H__                       FW_BUILD	// version


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Exported Define    --------------------------------------------------------*/
#define RS485_MAX_ATTEMPTS          10
#define DISP_KEYPAD_DELAY           1000
#define DISP_UNLOCK_TIMEOUT         30000
/* Exported types ------------------------------------------------------------*/
/* Exporeted Variable  -------------------------------------------------------*/
extern uint32_t disp_flag;
extern uint32_t disp_tmr;
extern uint32_t disp_toutmr;
extern char logbuf[128];
/* Exported Macro ------------------------------------------------------------*/
#define DISP_uSDCardReadySet()				        (disp_flag |= (1U << 4))
#define DISP_uSDCardErrorSet()				        (disp_flag &= (~ (1U << 4)))
#define IsDISP_uSDCardReady()                       ((disp_flag & (1U << 4)) != 0U)
#define DISP_KeypadSet()                            (disp_flag |= (1U << 5))
#define DISP_KeypadReset()                          (disp_flag &= (~ (1U << 5)))
#define IsDISP_KeypadActiv()                        ((disp_flag & (1U << 5)) != 0U)
#define DISP_RefreshSet()							(disp_flag |= (1U << 6))	
#define DISP_RefreshReset()							(disp_flag &= (~ (1U << 6)))
#define IsDISP_RefreshActiv()						((disp_flag & (1U << 6)) != 0U)
#define DISP_UnlockSet()                            (disp_flag |= (1U << 7))
#define DISP_UnlockReset()							(disp_flag &= (~ (1U << 7)))
#define IsDISP_UnlockActiv()					    ((disp_flag & (1U << 7)) != 0U)	
#define DISP_UpdateTimeSet()					    (disp_flag |= (1U << 8))
#define DISP_UpdateTimeReset()						(disp_flag &= (~ (1U << 8)))
#define IsDISP_UpdateTimeActiv()			        ((disp_flag & (1U << 8)) != 0U)

#define DISP_TimerStart(TIME)						(disp_tmr = TIME)
#define DISP_TimerStop()                            (disp_tmr = 0U)
#define IsDISP_TimerExpired()						(disp_tmr == 0U)
#define DISP_TimeoutTimerStart(TIME)                (disp_toutmr = TIME)
#define DISP_TimeoutTimerStop()						(disp_toutmr = 0U)
#define IsDISP_TimeoutTimerExpired()                (disp_toutmr == 0U)
/* Exported Function  ------------------------------------------------------- */
void DISP_Init(void);
void DISP_Service(void);
void DISP_UpdateLog(const char *pbuf);
void DISP_FileTransferState(uint8_t nsta);
void DISP_ProgbarSetNewState(uint8_t nsta);
void DISP_uSDCardSetNewState(uint8_t nsta);
#endif /* __DISP_H__ */

/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
