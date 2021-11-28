/**
  * @file    sccb_bus.h
  * @author  WB R&D Team - openmcu666
  * @version V0.1
  * @date    2015.11.09
  * @brief   sccb_bus
  */
 
#ifndef _SCCB_BUS_H_
#define _SCCB_BUS_H_


#include "stm32f4xx.h"


#define SCCB_SIC_H()     GPIOB->BSRRL = GPIO_Pin_6
#define SCCB_SIC_L()     GPIOB->BSRRH = GPIO_Pin_6
#define SCCB_SID_H()     GPIOB->BSRRL = GPIO_Pin_7
#define SCCB_SID_L()     GPIOB->BSRRH = GPIO_Pin_7


#define SCCB_DATA_IN     sccb_bus_data_in()
#define SCCB_DATA_OUT    sccb_bus_data_out()
#define SCCB_SID_STATE	 GPIOB->IDR&0x0080//0000 0000 1000 0000


void systick_delay_ms(u16 nms);	    								   
void systick_delay_us(u32 nus);

void sccb_bus_init(void);
void sccb_bus_start(void);
void sccb_bus_stop(void);
void sccb_bus_send_noack(void);
void sccb_bus_send_ack(void);
uint8_t sccb_bus_write_byte(uint8_t data);
uint8_t sccb_bus_read_byte(void);


#endif /* _SCCB_BUS_H_ */

/******************* (C) COPYRIGHT 2015 WangBao *****END OF FILE****/
