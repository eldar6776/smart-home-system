/**
 ******************************************************************************
 * File Name          : radio.h
 * Date               : 04/01/2018 5:24:19
 * Description        : radio transciever processing header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIO_H
#define __RADIO_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"


/* Typedef -------------------------------------------------------------------*/
typedef enum
{
	RADIO_INIT				= 0x00U,
	RADIO_SEND_STATUS		= 0x01U,
	RADIO_RECEIVE_STATUS	= 0x02U,
	RADIO_SUSPEND			= 0x03U
	
} RADIO_ServiceStateTypeDef;
	
	
/* Define --------------------------------------------------------------------*/
#define RADIO_CYCLE_PERIOD				95U					/*	xxx ms status refresh time over radio */
#define RADIO_TIMEOUT					345U                /*	watchdog time frame for every radio function from start to end*/
#define RADIO_MAX_ERROR					16U					/*	max number of error beffore trigger status */
#define UART_BUFFER_SIZE				64U					/* 	64 bytes buffer lenght */
#define BYTE_TRANSFER_TIMEOUT			5U					/*	next receiving byte max. latency time */
#define RADIO_RX_TX_DELAY				5U					/*	radio rx to tx mode delay */
#define RADIO_LINK_TIMEOUT				650U                /* 	radio link error timeout before output shutdown	*/	




/* Variable ------------------------------------------------------------------*/
extern uint8_t usart_buffer[UART_BUFFER_SIZE];

extern volatile uint8_t receive_pcnt;
extern volatile uint8_t transfer_error;
extern volatile uint8_t received_byte_cnt;

extern volatile uint32_t radio_timer;
extern volatile uint32_t radio_flags;
extern volatile uint32_t radio_cycle_timer;
extern volatile uint32_t radio_link_timeout;

extern uint8_t radio_cycle_cnt;
extern uint8_t radio_error_cnt;
extern uint8_t radio_link_qt;

extern  RADIO_ServiceStateTypeDef RADIO_ServiceState;


/* Macro ---------------------------------------------------------------------*/
#define RADIO_StartTimer(TIME)						(radio_timer = TIME)
#define RADIO_StopTimer()							(radio_timer = 0U)
#define IsRADIO_TimerExpired()						(radio_timer == 0U)
#define RADIO_StartCycleTimer(RADIO_CYCLE_TIME)		(radio_cycle_timer = RADIO_CYCLE_TIME)
#define RADIO_StopCycleTimer()						(radio_cycle_timer = 0U)
#define IsRADIO_CycleTImerExpired()					(radio_cycle_timer == 0U)
#define RADIO_RxStatusReady()						(radio_flags |= 0x00000001U)
#define RADIO_RxStatusReset()						(radio_flags &= 0xfffffffeU)
#define IsRADIO_RxStatusReady()						(radio_flags & 0x00000001U)
#define RADIO_TxStatusReady()						(radio_flags |= 0x00000002U)
#define RADIO_TxStatusReset()						(radio_flags &= 0xfffffffdU)
#define IsRADIO_TxStatusReady()						(radio_flags & 0x00000002U)
#define RADIO_StartLinkTimeoutTimer(TIMEOUT)		(radio_link_timeout = TIMEOUT)
#define RADIO_StopLinkTimeoutTimer()				(radio_link_timeout = 0U)
#define IsRADIO_LinkTimeoutTimerExpired()			(radio_link_timeout == 0U)

/* Function prototypes   -----------------------------------------------------*/
void RADIO_Service(void);


#endif  /* __RADIO_H */


/******************************   END OF FILE  ********************************/
