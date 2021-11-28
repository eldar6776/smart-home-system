/**
  ******************************************************************************
  * @file    rtc.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   RTC functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
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
#include "rtc.h"
#include "main.h"
/* Private typedef -----------------------------------------------------------*/
RTC_TimeTypeDef     rtc_time;
RTC_DateTypeDef     rtc_date;
RTC_StateTypeDef    RTC_State;
/* Private define ------------------------------------------------------------*/
/* Private constants  --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void RTC_Config(void)
{
	SysTick_Config(SystemCoreClock / 1000U);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	
	if (RTC_ReadBackupRegister(RTC_BKP_DR1) != 0xA5A5U)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		PWR_BackupAccessCmd(ENABLE);
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);
		RCC_LSEConfig(RCC_LSE_ON);
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
        }
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
        RTC_WriteProtectionCmd(DISABLE);
        RTC_WaitForSynchro();
        RTC_WriteBackupRegister(RTC_BKP_DR1, 0xA5A5U);
        RTC_State = RTC_INVALID;
	}
	else
	{
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)        // Power On Reset occurred
        {
        }
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)   // External Reset occurred
        {
        }
        RTC_WaitForSynchro();
        RTC_State = RTC_VALID;
	}
	RCC_ClearFlag();
}
/***************** (C) COPYRIGHT JUBERA D.O.O. SARAJEVO ***********************/
