/**
 ******************************************************************************
 * File Name          : uart.h
 * Date               : 21/08/2016 20:59:16
 * Description        : rs485 function set header file
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#ifndef __UART_H__
#define __UART_H__ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "common.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define RS485_RX		                Bit_RESET
#define RS485_TX		                Bit_SET
#define RS485_MODE(x)                   GPIO_WriteBit(GPIOE,GPIO_Pin_4,x)
#define UART_IRQ     	                USART2_IRQn
/* Exported variables  -------------------------------------------------------*/
//extern uint32_t rxbcnt;
//extern uint8_t rx_buff[RS_BSIZE];
//extern uint8_t tx_buff[RS_BSIZE];
/* Exported macros     -------------------------------------------------------*/
/* Exported functions  -------------------------------------------------------*/
void RS485_Init(void);
void UART_Init(void);
void RS485_Send(uint8_t *buf, uint16_t len);
void UART_ReInit(uint8_t mode, uint8_t baudrate);

#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/

