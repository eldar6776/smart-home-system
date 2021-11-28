#ifndef _USART_H_
#define _USART_H_
#include "stm32f4xx.h"
#include "stdio.h"

void USART1_Init(void);
void USART1_SendChar(unsigned char ch);
void USART1_SendString(unsigned char *s);

#endif
