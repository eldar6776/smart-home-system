#include "delay.h"

__IO u16 time;	

/**
  * @brief  延时nms*1 ms
  * @param  nms:延时长度
  * @retval None
  */
void delay_ms(u16 nms)
{	 		  	  
	time=nms;
	SysTick_Config(180000);//1ms产生一次中断
	while(time);
	SysTick->CTRL=0x00;			  	    
}   

/**
  * @brief  延时nus*1 us
  * @param  nus:延时长度
  * @retval None
  */
void delay_us(u32 nus)
{		
	time=nus;
	SysTick_Config(180);//1us产生一次中断
	while(time);
	SysTick->CTRL=0x00;
}

