/**
  * @file    USART.c 
  * @author  WB R&D Team - openmcu666
  * @version V0.1
  * @date    2015.6.9
  * @brief   USART Driver
  */
#include "stm32f4xx.h"
#include "stdio.h"
#include "usart.h"


void USART1_Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);


	USART_InitStructure.USART_BaudRate   = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;
	USART_InitStructure.USART_Parity     = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                                      //开启接收中断
}

/**
  * @brief  串口1发送一个字符
  * @param  ch:要发送的data
  * @retval None
  */
//void USART1_SendChar(unsigned char ch)
//{
//   USART_SendData (USART1,ch);                                               
//   while( USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET );                   
//   USART_ClearFlag(USART1,USART_FLAG_TC);                                   
//}

///**
//  * @brief  发送字符串
//  * @param  s:要发送的数据
//  * @retval None
//  */
//void USART1_SendString(unsigned char *s)
//{
//    while (*s != '\0') {
//         USART1_SendChar(*s++);
//    }
//}


//int fputc(int ch, FILE *f)
//{
//	/* Place your implementation of fputc here */
//	/* e.g. write a character to the USART */
//	USART_SendData(USART1, (uint8_t) ch);

//	/* Loop until the end of transmission */
//	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
//	{}

//	return ch;
//}


