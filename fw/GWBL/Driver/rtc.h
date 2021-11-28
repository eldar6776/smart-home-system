/**
  ******************************************************************************
  * @file    rtc.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   Header for rtc module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_RTC_H
#define __APP_RTC_H

/* Includes ------------------------------------------------------------------*/
//#include "global_includes.h"
#include "stm32f4xx.h"
#include "stdint.h"

/* Exported types ------------------------------------------------------------*/
/**
 * @brief  RTC Struct for date/time
 */
typedef struct {
	uint8_t seconds;     /*!< Seconds parameter, from 00 to 59 */
	uint16_t subseconds; /*!< Subsecond downcounter. When it reaches zero, it's reload value is the same as
                                 @ref RTC_SYNC_PREDIV, so in our case 0x3FF = 1023, 1024 steps in one second */
	uint8_t minutes;     /*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;       /*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t day;         /*!< Day in a week, from 1 to 7 */
	uint8_t date;        /*!< Date in a month, 1 to 31 */
	uint8_t month;       /*!< Month in a year, 1 to 12 */
	uint8_t year;        /*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
	uint32_t unix;       /*!< Seconds from 01.01.1970 00:00:00 */
} TM_RTC_t;

/**
 * @brief  Backward compatibility for RTC time
 */
extern TM_RTC_t TM_RTC_Time_t;

/* Exported constants --------------------------------------------------------*/
#define RTC_CLOCK_SOURCE_LSE
//#define RTC_CLOCK_SOURCE_HSE
//#define RTC_CLOCK_SOURCE_LSI
extern uint8_t RTC_HandlerFlag;
extern int8_t  RTC_Error;
extern RTC_TimeTypeDef   RTC_Time;
extern RTC_DateTypeDef   RTC_Date;
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int8_t RTC_Config(void);
void RTC_HttpCgiDateTimeUpdate(uint8_t *ibuff);
/**
 * @brief  Get number of seconds from date and time since 01.01.1970 00:00:00
 * @param  *data: Pointer to @ref TM_RTC_t data structure
 * @retval Calculated seconds from date and time since 01.01.1970 00:00:00
 */
uint32_t TM_RTC_GetUnixTimeStamp(TM_RTC_t* data);

/**
 * @brief  Get formatted time from seconds till 01.01.1970 00:00:00
 *         It fills struct with valid data
 * @note   Valid if year is greater or equal (>=) than 2000
 * @param  *data: Pointer to @ref TM_RTC_Time_t struct to store formatted data in
 * @param  unix: Seconds from 01.01.1970 00:00:00 to calculate user friendly time
 * @retval None
 */
void TM_RTC_GetDateTimeFromUnix(TM_RTC_t* data, uint32_t unix);
#endif /* __APP_RTC_H */

/***************** (C) COPYRIGHT JUBERA D.O.O. SARAJEVO ***********************/
