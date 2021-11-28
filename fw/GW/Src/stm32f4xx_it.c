/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    06-March-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"
#include "uart.h"
#include "wiegand.h"
#include "display.h"
#include "GUIDRV_stm32f429i.h"
#include "GUI.h"
/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup SysTick_Example
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/
extern volatile GUI_TIMER_TIME OS_TimeMS;
extern void RS485_Receive(uint16_t rxb);
extern void RS485_Tick(void);
__IO uint32_t sys_tick = 0;
/**
  * @brief  Get system timer value
  * @param  None
  * @retval None
  */
uint32_t Get_SysTick(void)
{
	return sys_tick;
}
/**
  * @brief  This function handles SysTick IRQ update
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	++OS_TimeMS;
    ++sys_tick;
    RS485_Tick();
	if (disp_tmr)   --disp_tmr;
	if (disp_toutmr)--disp_toutmr;
}

void LTDC_IRQHandler(void)
{
    LTDC_ISR_Handler();
}

void EXTI0_IRQHandler(void) 
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    { 	/* Clear the EXTI line  pending bit */	
        EXTI_ClearITPendingBit(EXTI_Line0);                   
    }
}

void EXTI9_5_IRQHandler(void)
{
	if      (EXTI_GetITStatus(EXTI_Line5) != RESET) // WIEGAND D0 LINE IRQ
	{   /* Clear the EXTI line 5 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
	else if (EXTI_GetITStatus(EXTI_Line6) != RESET) // WIEGAND D1 LINE IRQ
	{   /* Clear the EXTI line 6 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	else if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{   /* Clear the EXTI line 7 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line7);                   
	}
	else if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{   /* Clear the EXTI line 8 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line8);                   
	}
	else if(EXTI_GetITStatus(EXTI_Line9) != RESET)
	{   /* Clear the EXTI line 9 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line9);                   
	}
}

void EXTI15_10_IRQHandler(void) 
{
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    { 	/* Clear the EXTI line  pending bit */	
        EXTI_ClearITPendingBit(EXTI_Line11);                   
    }
}

void DMA2D_IRQHandler(void)
{
    DMA2D_ISR_Handler();
}

void DMA2_Stream1_IRQHandler(void) 	 
{
    if (DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1))   
    {	/* Test on DMA Stream Transfer Complete interrupt */
        DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1); 
    }
}

void DMA2_Stream2_IRQHandler (void)
{
    if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2))   
    {	/* Test on DMA Stream Transfer Complete interrupt */
        DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2); 
    }
}

void DMA2_Stream4_IRQHandler(void){	
	if (DMA_GetITStatus(DMA2_Stream4, DMA_IT_TCIF4)){
		DMA_ClearITPendingBit(DMA2_Stream4, DMA_IT_TCIF4); 
	}
}

void USART2_IRQHandler(void){
	uint16_t rb;    
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){	 	
		rb = USART_ReceiveData(USART2);
        RS485_Receive(rb);
	}		
}
/**
  * @brief  This function handles RTC Alarm exception.
  * @param  None
  * @retval None
  */
void RTC_Alarm_IRQHandler(void){    
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET){
		RTC_ClearFlag(RTC_FLAG_ALRAF);        	
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);
}
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void){
    
}
/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{   /* Go to infinite loop when Hard Fault exception occurs */
    ErrorHandler(FUNC_OR_DRV_FAIL, SYS_EXEPTION);
}
/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{   /* Go to infinite loop when Memory Manage exception occurs */
    ErrorHandler(FUNC_OR_DRV_FAIL, SYS_EXEPTION);
}
/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{   /* Go to infinite loop when Bus Fault exception occurs */
    ErrorHandler(FUNC_OR_DRV_FAIL, SYS_EXEPTION);
}
/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{   /* Go to infinite loop when Usage Fault exception occurs */
    ErrorHandler(FUNC_OR_DRV_FAIL, SYS_EXEPTION);
}
/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
    
}
/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
    
}
/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{

}
/**
  * @brief  This function handles SDIO_DMA_IRQ
  * @param  None
  * @retval None
  */
#ifdef SD_DMA_MODE
void SD_SDIO_DMA_IRQHANDLER(void)
{
    SD_ProcessDMAIRQ();  
} 
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
