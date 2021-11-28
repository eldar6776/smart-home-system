
#ifndef __RTP_TOUCH_H__
#define __RTP_TOUCH_H__

#include "stm32f4xx.h"

#define TSC_ChipSelected()       		GPIO_ResetBits(TOUCH_CS_GPIO, TOUCH_CS_GPIO_PIN) 
#define TSC_ChipDeselected()          	GPIO_SetBits(TOUCH_CS_GPIO, TOUCH_CS_GPIO_PIN)

#define TOUCH_MISO_GPIO          		GPIOB              
#define TOUCH_MISO_GPIO_CLK    			RCC_AHB1Periph_GPIOB
#define TOUCH_MISO_GPIO_PIN        		GPIO_Pin_4
#define TOUCH_MISO_GPIO_SOURCE   		GPIO_PinSource4

#define TOUCH_MOSI_GPIO           		GPIOB
#define TOUCH_MOSI_GPIO_CLK      		RCC_AHB1Periph_GPIOB
#define TOUCH_MOSI_GPIO_PIN        		GPIO_Pin_5
#define TOUCH_MOSI_GPIO_SOURCE      	GPIO_PinSource5

#define TOUCH_SCK_GPIO             		GPIOB
#define TOUCH_SCK_GPIO_CLK        		RCC_AHB1Periph_GPIOB
#define TOUCH_SCK_GPIO_PIN          	GPIO_Pin_3
#define TOUCH_SCK_GPIO_SOURCE   		GPIO_PinSource3

#define TOUCH_CS_GPIO             		GPIOI
#define TOUCH_CS_GPIO_CLK        		RCC_AHB1Periph_GPIOI
#define TOUCH_CS_GPIO_PIN       		GPIO_Pin_8
#define TOUCH_CS_GPIO_SOURCE 			GPIO_PinSource8

#define TOUCH_BUSY_GPIO           		GPIOD
#define TOUCH_BUSY_GPIO_CLK      		RCC_AHB1Periph_GPIOD
#define TOUCH_BUSY_GPIO_PIN     		GPIO_Pin_4

#define TOUCH_INT_GPIO        			GPIOC
#define TOUCH_INT_GPIO_CLK				RCC_AHB1Periph_GPIOC
#define TOUCH_INT_GPIO_PIN   			GPIO_Pin_13

#define TOUCH_INT_EXTI_LINE        		EXTI_Line13
#define TOUCH_INT_EXTI_IRQn      		EXTI15_10_IRQn
#define TOUCH_INT_EXTI_Port       		EXTI_PortSourceGPIOC
#define TOUCH_INT_EXTI_Pin      		EXTI_PinSource13

#define TOUCH_GPIO_AF           		GPIO_AF_SPI1
#define TOUCH_RCC_CLK     				RCC_APB2Periph_SPI1
#define	T_INT		  					GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)
#define TOUCH_SPI          				SPI1

#define TOUCH_CMD_X             		0xD0
#define TOUCH_CMD_Y              		0x90
#define THRESHOLD 2                       //≤Ó÷µ√≈œﬁ

typedef	struct POINT 
{
   u16 x;
   u16 y;
}Coordinate;

typedef struct {
      int x[5], xfb[5];
      int y[5], yfb[5];
      int a[7];
} calibration;


void TOUCH_Init(void);
uint16_t Get_TouchADX(void);
uint16_t Get_TouchADY(void);
Coordinate *_CheckUpdateTouch(void);
uint8_t Touch_SPI_ReadWrite(uint8_t Byte);
void my_ExecCalibration( int x_size, int y_size );
int  Calibrate_X(unsigned  int ad_x,unsigned int ad_y);
int  Calibrate_Y(unsigned  int ad_x,unsigned int ad_y);
#endif 
