/**
 ******************************************************************************
 * File Name          : rubicon_address_list.h
 * Date               : 21/08/2016 20:59:16
 * Description        : instaled Rubicon controller address list header
 ******************************************************************************
 *
 ******************************************************************************
 */
#ifndef RTC_H
#define RTC_H 			171

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup STM32F4xx_Libraries
 * @{
 */

/**
 * @defgroup RTC
 * @brief    RTC for STM32F429
 * @{
 *
 * \par Features
 *
@verbatim
- Support Internal or external clock source
  - PC14 and PC15 pins are used for external crystal oscillator
  - STM32F4/429 Discovery does not have RTC crystal onboard. Check board's manual on how to set it up
- Support wakeup interrupt
- Support to set 2 internal alarms to trigger interrupts
  - They can also wake up STM32F4 from any low power mode
- Get seconds from 01.01.1970 00:00:00
- Get readable time from seconds from 01.01.1970 00:00:00
- Support to save/get data in binary or BCD format
- Support for read/write data to/from RTC backup registers
- Support for subsecond
- Support to write data in string format
- Date and time are checked before saved for valid input data
- Get days in month and year
@endverbatim
 *
 * \par Pinout for RTC external 32768Hz crystal
 *
@verbatim
 STM32F4XX  Oscillator   Description
	
 PC14       OSC1         Oscillator terminal 1
 PC15       OSC2         Oscillator terminal 2
 *
 * \par Dependencies
 *
@verbatim
 - STM32F4xx
 - STM32F4xx RCC
 - STM32F4xx RTC
 - STM32F4xx PWR
 - STM32F4xx EXTI
 - MISC
 - defines.h
 - attributes.h
@endverbatim
 */
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_exti.h"


/* RTC clock is: f_clk = RTCCLK(LSI or LSE) / ((RTC_SYNC_PREDIV + 1) * (RTC_ASYNC_PREDIV + 1)) */
/* Sync pre division for clock */
#ifndef RTC_SYNC_PREDIV
#define RTC_SYNC_PREDIV			 		0x3ff
#endif
/* Async pre division for clock */
#ifndef RTC_ASYNC_PREDIV
#define RTC_ASYNC_PREDIV					0x1f
#endif
/* NVIC global Priority set */
#ifndef RTC_PRIORITY
#define RTC_PRIORITY					0x04
#endif
/* Sub priority for wakeup trigger */
#ifndef RTC_WAKEUP_SUBPRIORITY
#define RTC_WAKEUP_SUBPRIORITY			0x00
#endif
/* Sub priority for alarm trigger */
#ifndef RTC_ALARM_SUBPRIORITY
#define RTC_ALARM_SUBPRIORITY				0x01
#endif


/**
 * @brief  RTC date/time struct
 */
typedef struct 
{
	uint8_t seconds;     	/*!< Seconds parameter, from 00 to 59 */
	uint16_t subseconds; 	/*!< Subsecond downcounter. When it reaches zero, it's reload value is the same as
						@ref RTC_SYNC_PREDIV, so in our case 0x3FF = 1023, 1024 steps in one second */
	uint8_t minutes;     	/*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;       	/*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t day;         	/*!< Day in a week, from 1 to 7 */
	uint8_t date;        	/*!< Date in a month, 1 to 31 */
	uint8_t month;       	/*!< Month in a year, 1 to 12 */
	uint8_t year;        	/*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
	uint32_t unix;       	/*!< Seconds from 01.01.1970 00:00:00 */
} RTC_t;

/**
 * @brief  Backward compatibility for RTC time
 */
typedef RTC_t RTC_Time_t;

/**
 * @brief RTC Result enumeration
 */
typedef enum 
{
	RTC_Result_Ok,   		/*!< Everything OK */
	RTC_Result_Error 		/*!< An error occurred */
} RTC_Result_t;


/**
 * @brief  RTC Interrupt enumeration
 */
typedef enum 
{
	RTC_Int_Disable = 0x00, 	/*!< Disable RTC wakeup interrupts */
	RTC_Int_60s,            	/*!< RTC Wakeup interrupt every 60 seconds */
	RTC_Int_30s,            	/*!< RTC Wakeup interrupt every 30 seconds */
	RTC_Int_15s,            	/*!< RTC Wakeup interrupt every 15 seconds */
	RTC_Int_10s,            	/*!< RTC Wakeup interrupt every 10 seconds */
	RTC_Int_5s,             	/*!< RTC Wakeup interrupt every 5 seconds */
	RTC_Int_2s,             	/*!< RTC Wakeup interrupt every 2 seconds */
	RTC_Int_1s,             	/*!< RTC Wakeup interrupt every 1 seconds */
	RTC_Int_500ms,          	/*!< RTC Wakeup interrupt every 500 milliseconds */
	RTC_Int_250ms,          	/*!< RTC Wakeup interrupt every 250 milliseconds */
	RTC_Int_125ms           	/*!< RTC Wakeup interrupt every 125 milliseconds */
} RTC_Int_t;

/**
 * @brief  Select RTC clock source
 * @note   Internal clock is not accurate and should not be used in production
 */
typedef enum 
{
	RTC_ClockSource_Internal = 0x00, /*!< Use internal clock source for RTC (LSI oscillator) */
	RTC_ClockSource_External         /*!< Use external clock source for RTC (LSE oscillator) */
} RTC_ClockSource_t;

/**
 * @brief  RTC Alarm type
 */
typedef enum 
{
	RTC_AlarmType_DayInWeek,	/*!< Trigger alarm every day in a week, days from 1 to 7 (Monday to Sunday) */
	RTC_AlarmType_DayInMonth	/*!< Trigger alarm every month */
} RTC_AlarmType_t;


/**
 * @brief  RTC structure for alarm time
 */
typedef struct 
{
	RTC_AlarmType_t alarmtype; 	/*!< Alarm type setting. @ref TM_RTC_AlarmType_t for more info */
	uint8_t seconds;              /*!< Alarm seconds value */
	uint8_t minutes;              /*!< Alarm minutes value */
	uint8_t hours;                /*!< Alarm hours value */
	uint8_t day;                  /*!< Alarm day value. If you select trigger for alarm every week, then this parameter has value between
								1 and 7, representing days in a week, Monday to Sunday
								If you select trigger for alarm every month, then this parameter has value between
								1 - 31, representing days in a month. */
} RTC_AlarmTime_t;


/**
 * @brief  Initializes RTC and starts counting
 * @param  source. RTC Clock source @ref TM_RTC_ClockSource_t to be used for RTC
 * @note   Internal clock source is not so accurate
 * @note   If you reset your MCU and RTC still has power, it will count independent of MCU status
 * @retval Returns RTC status.
 *            - 1: RTC has already been initialized and time is set
 *            - 0: RTC was now initialized first time. Now you can set your first clock
 */
uint32_t CLOCK_Init(RTC_ClockSource_t source);

/**
 * @brief  Get number of seconds from date and time since 01.01.1970 00:00:00
 * @param  *data: Pointer to @ref TM_RTC_t data structure
 * @retval Calculated seconds from date and time since 01.01.1970 00:00:00
 */
uint32_t RTC_GetUnixTimeStamp(RTC_t* data);

/**
 * @brief  Get formatted time from seconds till 01.01.1970 00:00:00
 *         It fills struct with valid data
 * @note   Valid if year is greater or equal (>=) than 2000
 * @param  *data: Pointer to @ref TM_RTC_Time_t struct to store formatted data in
 * @param  unix: Seconds from 01.01.1970 00:00:00 to calculate user friendly time
 * @retval None
 */
void RTC_GetDateTimeFromUnix(RTC_t* data, uint32_t unix);

/**
 * @brief  Select RTC wakeup interrupts interval
 * @note   This function can also be used to disable interrupt
 * @param  int_value: Look for @ref TM_RTC_Int_t for valid inputs
 * @retval None
 */
void RTC_Interrupts(RTC_Int_t int_value);

/**
 * @brief  Set date and time to internal RTC registers
 * @param  *data: Pointer to filled @ref TM_RTC_t structure with date and time
 * @param  format: Format of your structure. This parameter can be a value of @ref TM_RTC_Format_t enumeration
 * @retval RTC datetime status @ref TM_RTC_Result_t:
 *            - @ref TM_RTC_Result_Ok: Date and Time set OK
 *            - @ref TM_RTC_Result_Error: Date and time is wrong
 */
RTC_Result_t RTC_SetDateTime(RTC_t* data, uint32_t format);

/**
 * @brief  Set date and time using string formatted date time
 * @note   Valid string format is: <b>dd.mm.YY.x;HH:ii:ss</b>
 *            - <b>dd</b>: date, 2 digits, decimal
 *            - <b>mm</b>: month, 2 digits, decimal
 *            - <b>YY</b>: year, last 2 digits, decimal
 *            - <b>x</b>: day in a week: 1 digit, 1 = Monday, 7 = Sunday
 *            - <b>HH</b>: hours, 24-hour mode, 2 digits, decimal
 *            - <b>ii</b>: minutes, 2 digits, decimal
 *            - <b>ss</b>: seconds, 2 digits, decimal
 * @param  *str: Pointer to string with datetime format
 * @retval RTC datetime status @ref TM_RTC_Result_t:
 *            - @ref TM_RTC_Result_Ok: Date and Time set OK
 *            - @ref TM_RTC_Result_Error: Date and time is wrong
 */
RTC_Result_t RTC_SetDateTimeString(char* str);

/**
 * @brief  Get date and time from internal RTC registers
 * @param  *data: Pointer to @ref TM_RTC_t structure to save data to
 * @param  format: Format of your structure. This parameter can be a value of @ref TM_RTC_Format_t enumeration
 * @retval None
 */
void RTC_GetDateTime(RTC_t* data, uint32_t format);

/**
 * @brief  Get number of days in month
 * @note   This function also detects if it is leap year and returns different number for February
 * @param  month: Month number in year, valid numbers are 1 - 12 
 * @param  year: Year number where you want to get days in month, last 2 digits
 * @retval Number of days in specific month and year
 */
uint8_t RTC_GetDaysInMonth(uint8_t month, uint8_t year);

/**
 * @brief  Get number of days in specific year
 * @note   This function also detects if it is leap year
 * @param  year: Year number where you want to get days in month, last 2 digits
 * @retval Number of days in year
 */
uint16_t RTC_GetDaysInYear(uint8_t year);



/**
 * @brief  Disables specific alarm
 * @param  Alarm: Select which alarm you want to disable. This parameter can be a value of @ref TM_RTC_Alarm_t enumeration
 * @retval None
 */
void RTC_DisableAlarm(uint32_t Alarm);

/**
 * @brief  RTC Wakeup handler function. Called when wakeup interrupt is triggered
 * @note   Called from my RTC library
 * @param  None
 * @retval None
 * @note   With __weak parameter to prevent link errors if not defined by user
 */
void RTC_RequestHandler(void);

/**
 * @brief  RTC Alarm A handler function. Called when interrupt is triggered for alarm A
 * @note   Called from my RTC library
 * @param  None
 * @retval None
 * @note   With __weak parameter to prevent link errors if not defined by user
 */
void RTC_AlarmAHandler(void);

/**
 * @brief  RTC Alarm B handler function. Called when interrupt is triggered for alarm B.
 * @note   Called from my RTC library
 * @param  None
 * @retval None
 * @note   With __weak parameter to prevent link errors if not defined by user
 */
void RTC_AlarmBHandler(void);


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif

