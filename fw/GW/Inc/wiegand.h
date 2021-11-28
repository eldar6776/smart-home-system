/**
 ******************************************************************************
 * File Name          : wiegand.h
 * Date               : 8.12.2019.
 * Description        : wiegand interface modul header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
#ifndef __WIEGAND_H__
#define __WIEGAND_H__     		    FW_BUILD // version
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Exported defines    -------------------------------------------------------*/
/* Defines    ----------------------------------------------------------------*/
/* Typedefs   ----------------------------------------------------------------*/
/* Exported variables  -------------------------------------------------------*/
extern __IO uint32_t wgndfl;
extern __IO uint32_t wgndtmr;
extern char wgndrx[];
extern char wgndtx[];
/* Exported macros     -------------------------------------------------------*/
#define WGND_RxSet()                 (wgndfl |=  (0x1U<<0))
#define WGND_RxReset()               (wgndfl &=(~(0x1U<<0)))    
#define IsWGND_RxActiv()             (wgndfl &   (0x1U<<0))
#define WGND_TxSet()                 (wgndfl |=  (0x1U<<1))
#define WGND_TxReset()               (wgndfl &=(~(0x1U<<1)))    
#define IsWGND_TxActiv()             (wgndfl &   (0x1U<<1))
#define WGND_StartTimer(TIME)        (wgndtmr = TIME)
#define WGND_StopTimer()             (wgndtmr = 0U)
#define IsWGND_TimerExpired()        (wgndtmr == 0U)
/* Exported functions  -------------------------------------------------------*/
void WIEGAND_Init(void);
uint8_t WIEGAND_Send(char interface, char *buf);
__STATIC_INLINE void usDelay(__IO uint32_t micros) ;
#endif // __WIEGAND_H__
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
