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
#include <stdio.h>
#include "common.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/
extern __IO uint32_t TimingDelay;
__IO uint32_t sys_tick = 0;

uint32_t Get_SysTick(void)
{
	return sys_tick;
}

void SysTick_Handler(void)
{
	if (TimingDelay != 0x00) TimingDelay--;
}

extern void udp_echoclient_connect(void);
/**
  * @brief  This function handles EXTI0_IRQn exception.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void) 
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line0);                   /* Clear the EXTI line  pending bit */
	}
}
/**
  * @brief  This function handles EXTI1_IRQn exception.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void) 
{
	if(EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line1);                   /* Clear the EXTI line  pending bit */
	}
}
/**
  * @brief  This function handles EXTI2_IRQn exception.
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void) 
{
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line2);                   /* Clear the EXTI line  pending bit */
	}
}
/**
  * @brief  This function handles EXTI3_IRQn exception. Card stacker card inserted event.
  * @param  None
  * @retval None
  */
void EXTI3_IRQHandler(void) 
{
	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line3);                   /* Clear the EXTI line  pending bit */
	}
}
/**
  * @brief  This function handles EXTI4_IRQn exception.
  * @param  None
  * @retval None
  */
void EXTI4_IRQHandler(void) 
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line4);                   /* Clear the EXTI line  pending bit */
	}
}
/**
  * @brief  This function handles EXTI9_5_IRQn exception.
  * @param  None
  * @retval None
  */
void EXTI9_5_IRQHandler(void) 
{	
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)		//-----------------------------------------------------------	hall ccw dir irq
	{
		EXTI_ClearITPendingBit(EXTI_Line5); 
	}
	else if(EXTI_GetITStatus(EXTI_Line6) != RESET)	//-----------------------------------------------------------	hall 10 degree irq
	{
		EXTI_ClearITPendingBit(EXTI_Line6);                   	
	}
	else if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line7);                   /* Clear the EXTI line  pending bit */
	}
	else if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line8);                   /* Clear the EXTI line  pending bit */
	}
	else if(EXTI_GetITStatus(EXTI_Line9) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line9);                   /* Clear the EXTI line  pending bit */
	}
}
/**
  * @brief  This function handles EXTI15_10 exception.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void) 
{
	if(EXTI_GetITStatus(EXTI_Line10) != RESET)
	{ 		
	 // udp_echoclient_connect();
		EXTI_ClearITPendingBit(EXTI_Line10);                   /* Clear the EXTI line  pending bit */
	}
	else if(EXTI_GetITStatus(EXTI_Line11) != RESET)
	{ 		
	 // udp_echoclient_connect();
		EXTI_ClearITPendingBit(EXTI_Line11);                   /* Clear the EXTI line  pending bit */
	}
	else if(EXTI_GetITStatus(EXTI_Line12) != RESET)
	{ 		
	 // udp_echoclient_connect();
		EXTI_ClearITPendingBit(EXTI_Line12);                   /* Clear the EXTI line  pending bit */
	}
	else if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{ 		
	 // udp_echoclient_connect();
		EXTI_ClearITPendingBit(EXTI_Line13);                   /* Clear the EXTI line  pending bit */
	}
	else if(EXTI_GetITStatus(EXTI_Line14) != RESET)
	{ 		
	 // udp_echoclient_connect();
		EXTI_ClearITPendingBit(EXTI_Line14);                   /* Clear the EXTI line  pending bit */
	}
	else if(EXTI_GetITStatus(EXTI_Line15) != RESET)//-----------------------------------------------------------	hall cw dir irq
	{ 		
	 // udp_echoclient_connect();
		EXTI_ClearITPendingBit(EXTI_Line15); 
	}
}
/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

void TIM2_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
		printf("\r\nHardFault\r\n");
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

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
//void PendSV_Handler(void)
//{
//}


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

void USART2_IRQHandler(void)
{
	
}

//void Uart3_IRQ(void);
void USART3_IRQHandler(void)
{
   // Uart3_IRQ(); 	
}

/**
  * @brief  This function handles CAN1 RX0 request.
  * @param  None
  * @retval None
  */
void CAN1_RX0_IRQHandler(void)
{
//    CAN1_RX0_ISR();
}

/**
  * @brief  This function handles CAN2 RX0 request.
  * @param  None
  * @retval None
  */
void CAN2_RX0_IRQHandler(void)
{
//    CAN2_RX0_ISR();
	
}
/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
