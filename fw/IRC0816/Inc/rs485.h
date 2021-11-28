/**
 ******************************************************************************
 * File Name          : rs485.h
 * Date               : 28/02/2016 23:16:19
 * Description        : rs485 communication modul header
 ******************************************************************************
 *
 *	RS485 DEFAULT INTERFACE ADDRESS         0xffff
 *	RS485_DEFFAULT GROUP ADDRESS            0x6776
 * 	RS485_DEFFAULT BROADCAST ADDRESS        0x9999
 *	RS485 DEFAULT BAUDRATE                  115200
 *
 *
 ******************************************************************************
 */
 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RS485_H__
#define __RS485_H__					FW_BUILD	// version


/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "main.h"
#include "TinyFrame.h"
/* Exported Type  ------------------------------------------------------------*/
typedef enum
{
	COM_INIT = 0U,
	COM_PACKET_PENDING,
	COM_PACKET_RECEIVED,
	COM_RECEIVE_SUSPEND,
	COM_ERROR
	
}eComStateTypeDef;

typedef enum{
    SEND_INIT = 0,
    SEND_PWM,
    SEND_DOUT,
    SEND_ANOUT,
    SEND_RELAY,
    SEND_THERMO,
    SEND_DIMER,
    SEND_GROUP,
    SEND_SCENE,
}SEND_CmdTypeDef;

typedef enum{
    DALI_DAPC  = 0,
    DALI_UP,    
    DALI_DOWN,
    DALI_STEP_UP,
    DALI_STEP_DOWN,
    DALI_RECALL_MAX_LEVEL,
    DALI_RECALL_MIN_LEVEL,
    DALI_STEP_DOWN_AND_OFF,
    DALI_ON_AND_STEP_UP,
    DALI_RESET = 32,
    DALI_SET_SCENE_DTR0,
    DALI_STORE_ACTUAL_LEVEL_IN_DTR0,
    DALI_SET_MAX_LEVEL_DTR0 = 42,
    DALI_SET_MIN_LEVEL_DTR0,
    DALI_SET_SYSTEM_FAILURE_LEVEL_DTR0,
    DALI_SET_POWER_ON_LEVEL_DTR0,
    DALI_SET_FADE_TIME_DTR0,
    DALI_SET_FADE_RATE_DTR0, 
    DALI_ADD_TO_SCENE_DTR0,
    DALI_DELETE_FROM_SCENE,
    DALI_GO_TO_LAST_ACTIVE_LEVEL,
    DALI_GO_TO_SCENE,
    DALI_ADD_TO_GROUP,
    DALI_DELETE_FROM_GROUP,
    DALI_SET_SHORT_ADDR_DTR0,
    DALI_QUERY_STATUS,
    DALI_QUERY_LAMP_FAILURE,
    DALI_QUERY_ACTUAL_LEVEL,
    DALI_SET_EXTENDED_FADE_TIME_DTR0,
    DALI_IDENTIFY_DEVICE,
    DALI_OFF,
}DALI_CmdTypeDef;

extern eComStateTypeDef eComState;
extern DALI_CmdTypeDef DALI_Command;
extern SEND_CmdTypeDef SEND_Command;
/* Exported variables  -------------------------------------------------------*/
extern uint32_t rstmr;
extern uint32_t rsflg;
extern uint32_t tfbps;
extern uint16_t sysid;
extern uint8_t  tfifa;
extern uint8_t  tfgra;
extern uint8_t  tfbra;
extern uint8_t  tfgwa;
extern uint8_t tftype[16];
extern uint8_t rec;
/* Exported macros     -------------------------------------------------------*/
#define SYS_NewLogSet()						(sys_stat |= (1U << 0))
#define SYS_NewLogReset()					(sys_stat &= (~ (1U << 0)))
#define IsSYS_NewLogSet()					((sys_stat & (1U << 0)) != 0U)

#define SYS_LogListFullSet()				(sys_stat |= (1U << 1))
#define SYS_LogListFullReset()				(sys_stat &= (~ (1U << 1)))
#define IsSYS_LogListFullSet()				((sys_stat & (1U << 1)) != 0U)

#define SYS_FileTransferSuccessSet()		(sys_stat |= (1U << 2))
#define SYS_FileTransferSuccessReset()		(sys_stat &= (~ (1U << 2)))
#define IsSYS_FileTransferSuccessSet()		((sys_stat & (1U << 2)) != 0U)

#define SYS_FileTransferFailSet()			(sys_stat |= (1U << 3))
#define SYS_FileTransferFailReset()			(sys_stat &= (~ (1U << 3)))
#define IsSYS_FileTransferFailSet()			((sys_stat & (1U << 3)) != 0U)

#define SYS_UpdateSuccessSet()				(sys_stat |= (1U << 4))
#define SYS_UpdateSuccessReset()			(sys_stat &= (~ (1U << 4)))
#define IsSYS_UpdateSuccessSet()			((sys_stat & (1U << 4)) != 0U)

#define SYS_UpdateFailSet()					(sys_stat |= (1U << 5))
#define SYS_UpdateFailReset()				(sys_stat &= (~ (1U << 5)))
#define IsSYS_UpdateFailSet()				((sys_stat & (1U << 5)) != 0U)

#define SYS_ImageUpdateRequestSet()			(sys_stat |= (1U << 6))
#define SYS_ImageUpdateRequestReset()		(sys_stat &= (~ (1U << 6)))
#define IsSYS_ImageUpdateRequestSet()		((sys_stat & (1U << 6)) != 0U)

#define SYS_FwrUpdRequestSet()		        (sys_stat |= (1U << 7))
#define SYS_FwrUpdRequestReset()	        (sys_stat &= (~ (1U << 7)))
#define IsSYS_FwrUpdRequestSet()	        ((sys_stat & (1U << 7)) != 0U)

#define RS485_StartTimer(TIME)				(rs485_timer = TIME)
#define RS485_StopTimer()					(rs485_timer = 0U)
#define IsRS485_TimerExpired()				(rs485_timer == 0U)

#define RS485_StartUpdate()					(rs485_flags |= (1U << 0))
#define RS485_StopUpdate()					(rs485_flags &= (~ (1U << 0)))
#define IsRS485_UpdateActiv()				((rs485_flags & (1U << 0)) != 0U)

#define RS485_ResponsePacketReady()			(rs485_flags |= (1U << 1))
#define RS485_NoResponse()					(rs485_flags &= (~ (1U << 1)))
#define IsRS485_ResponsePacketPending()		((rs485_flags & (1U << 1)) != 0U)
/* Exported functions ------------------------------------------------------- */
void RS485_Init(void);
void RS485_Tick(void);
void RS485_Service(void);

#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
