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
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LSE_MAX_TRIALS_NB         6

/* Internal status registers for RTC */
#define RTC_STATUS_REG                  RTC_BKP_DR19 /* Status Register */
#define RTC_STATUS_INIT_OK              0x1234       /* RTC initialised */
#define RTC_STATUS_TIME_OK              0x4321       /* RTC time OK */
#define	RTC_STATUS_ZERO                 0x0000

/* Internal RTC defines */
#define RTC_LEAP_YEAR(year)             ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define RTC_DAYS_IN_YEAR(x)             RTC_LEAP_YEAR(x) ? 366 : 365
#define RTC_OFFSET_YEAR                 1970
#define RTC_SECONDS_PER_DAY             86400
#define RTC_SECONDS_PER_HOUR            3600
#define RTC_SECONDS_PER_MINUTE          60
#define RTC_BCD2BIN(x)                  ((((x) >> 4) & 0x0F) * 10 + ((x) & 0x0F))
#define RTC_CHAR2NUM(x)                 ((x) - '0')
#define RTC_CHARISNUM(x)                ((x) >= '0' && (x) <= '9')

/* Days in a month */
uint8_t TM_RTC_Months[2][12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	/* Not leap year */
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}	/* Leap year */
};

TM_RTC_t TM_RTC_Time_t;
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RTC_InitTypeDef   RTC_InitStructure;
uint32_t RTC_Timeout = 0x10000;
int8_t RTC_Error = 0;
uint8_t RTC_HandlerFlag;
RTC_TimeTypeDef   RTC_Time;
RTC_DateTypeDef   RTC_Date;

uint8_t __IO alarm_now = 1;
__IO uint32_t LsiFreq = 0;
__IO uint32_t CaptureNumber = 0, PeriodValue = 0;

/* Private function prototypes -----------------------------------------------*/
uint32_t GetLSIFrequency(void);

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Configures the RTC peripheral.
* @param  None
* @retval None
*/
int8_t RTC_Config(void)
{
	SysTick_Config(SystemCoreClock / 1000);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	RTC_Error = 0;
	
	if (RTC_ReadBackupRegister(RTC_BKP_DR1) != 0xA5A5)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		PWR_BackupAccessCmd(ENABLE);
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);
		RCC_LSEConfig(RCC_LSE_ON);
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) continue;
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForSynchro();
		RTC_WriteBackupRegister(RTC_BKP_DR1, 0xA5A5);
	}
	else
	{
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
			//Power On Reset occurred
		}
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
			//External Reset occurred
		}
		
		RTC_WaitForSynchro();
	} 
	
	RCC_ClearFlag();
	return RTC_Error;
}

/**
  * @brief  Configures TIM5 to measure the LSI oscillator frequency. 
  * @param  None
  * @retval LSI Frequency
  */
uint32_t GetLSIFrequency(void)
{
  NVIC_InitTypeDef   NVIC_InitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;

  /* Enable the LSI oscillator ************************************************/
  RCC_LSICmd(ENABLE);
  
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}

  /* TIM5 configuration *******************************************************/ 
  /* Enable TIM5 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  /* Connect internally the TIM5_CH4 Input Capture to the LSI clock output */
  TIM_RemapConfig(TIM5, TIM5_LSI);

  /* Configure TIM5 presclaer */
  TIM_PrescalerConfig(TIM5, 0, TIM_PSCReloadMode_Immediate);
  
  /* TIM5 configuration: Input Capture mode ---------------------
     The LSI oscillator is connected to TIM5 CH4
     The Rising edge is used as active edge,
     The TIM5 CCR4 is used to compute the frequency value 
  ------------------------------------------------------------ */
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV8;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM5, &TIM_ICInitStructure);
  
  /* Enable TIM5 Interrupt channel */
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable TIM5 counter */
  TIM_Cmd(TIM5, ENABLE);

  /* Reset the flags */
  TIM5->SR = 0;
    
  /* Enable the CC4 Interrupt Request */  
  TIM_ITConfig(TIM5, TIM_IT_CC4, ENABLE);


  /* Wait until the TIM5 get 2 LSI edges (refer to TIM5_IRQHandler() in 
    stm32fxxx_it.c file) ******************************************************/
  while(CaptureNumber != 2)
  {
  }
  /* Deinitialize the TIM5 peripheral registers to their default reset values */
  TIM_DeInit(TIM5);
  
  /* Get PCLK1 prescaler */
  if ((RCC->CFGR & RCC_CFGR_PPRE1) == 0)
  { 
    /* PCLK1 prescaler equal to 1 => TIMCLK = PCLK1 */
    return (((SystemCoreClock/4) / PeriodValue) * 8);
  }
  else
  { /* PCLK1 prescaler different from 1 => TIMCLK = 2 * PCLK1 */
    return (((2 * (SystemCoreClock/4)) / PeriodValue) * 8) ;
  }
}

/**
* @brief  This function handles RTC Alarm A interrupt request.
* @param  None
* @retval None
*/
void RTC_Alarm_IRQHandler(void)
{
  /* Clear the EXTIL line 17 */
  EXTI_ClearITPendingBit(EXTI_Line17);

  /* Check on the AlarmA falg and on the number of interrupts per Second (60*8) */
  if (RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {

    alarm_now = 0;
    /* Clear RTC AlarmA Flags */
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    RTC_HandlerFlag = ENABLE;
  }
}

void RTC_HttpCgiDateTimeUpdate(uint8_t *ibuff)
{
	RTC_Date.RTC_Date = (ibuff[0] - 48) << 4;
	RTC_Date.RTC_Date += ibuff[1] - 48;
	RTC_Date.RTC_Month = (ibuff[2] - 48) << 4;
	RTC_Date.RTC_Month += ibuff[3] - 48;
	//RTC_Date.RTC_WeekDay = RTC_Weekday_Wednesday;
	// skipp two byte for full year format "20"
	RTC_Date.RTC_Year = (ibuff[6] - 48) << 4;
	RTC_Date.RTC_Year += ibuff[7] - 48;
	RTC_Time.RTC_Hours = (ibuff[8] - 48) << 4;
	RTC_Time.RTC_Hours += ibuff[9] - 48;
	RTC_Time.RTC_Minutes = (ibuff[10] - 48) << 4;
	RTC_Time.RTC_Minutes += ibuff[11] - 48;
	RTC_Time.RTC_Seconds = (ibuff[12] - 48) << 4;
	RTC_Time.RTC_Seconds += ibuff[13] - 48;

	PWR_BackupAccessCmd(ENABLE);
	RTC_SetTime(RTC_Format_BCD, &RTC_Time);
	RTC_SetDate(RTC_Format_BCD, &RTC_Date);
	PWR_BackupAccessCmd(DISABLE);
	
}


uint32_t TM_RTC_GetUnixTimeStamp(TM_RTC_t* data) {
	uint32_t days = 0, seconds = 0;
	uint16_t i;
	uint16_t year = (uint16_t) (data->year + 2000);
	/* Year is below offset year */
	if (year < RTC_OFFSET_YEAR) {
		return 0;
	}
	/* Days in back years */
	for (i = RTC_OFFSET_YEAR; i < year; i++) {
		days += RTC_DAYS_IN_YEAR(i);
	}
	/* Days in current year */
	for (i = 1; i < data->month; i++) {
		days += TM_RTC_Months[RTC_LEAP_YEAR(year)][i - 1];
	}
	/* Day starts with 1 */
	days += data->date - 1;
	seconds = days * RTC_SECONDS_PER_DAY;
	seconds += data->hours * RTC_SECONDS_PER_HOUR;
	seconds += data->minutes * RTC_SECONDS_PER_MINUTE;
	seconds += data->seconds;
	
	/* seconds = days * 86400; */
	return seconds;
}

void TM_RTC_GetDateTimeFromUnix(TM_RTC_t* data, uint32_t unix) {
	uint16_t year;
	
	/* Store unix time to unix in struct */
	data->unix = unix;
	/* Get seconds from unix */
	data->seconds = unix % 60;
	/* Go to minutes */
	unix /= 60;
	/* Get minutes */
	data->minutes = unix % 60;
	/* Go to hours */
	unix /= 60;
	/* Get hours */
	data->hours = unix % 24;
	/* Go to days */
	unix /= 24;
	
	/* Get week day */
	/* Monday is day one */
	data->day = (unix + 3) % 7 + 1;

	/* Get year */
	year = 1970;
	while (1) {
		if (RTC_LEAP_YEAR(year)) {
			if (unix >= 366) {
				unix -= 366;
			} else {
				break;
			}
		} else if (unix >= 365) {
			unix -= 365;
		} else {
			break;
		}
		year++;
	}
	/* Get year in xx format */
	data->year = (uint8_t) (year - 2000);
	/* Get month */
	for (data->month = 0; data->month < 12; data->month++) {
		if (RTC_LEAP_YEAR(year)) {
			if (unix >= (uint32_t)TM_RTC_Months[1][data->month]) {
				unix -= TM_RTC_Months[1][data->month];
			} else {
				break;
			}
		} else if (unix >= (uint32_t)TM_RTC_Months[0][data->month]) {
			unix -= TM_RTC_Months[0][data->month];
		} else {
			break;
		}
	}
	/* Get month */
	/* Month starts with 1 */
	data->month++;
	/* Get date */
	/* Date starts with 1 */
	data->date = unix + 1;
}
/***************** (C) COPYRIGHT JUBERA D.O.O. SARAJEVO ***********************/
