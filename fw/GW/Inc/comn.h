/**
 ******************************************************************************
 * File Name          : common.h
 * Date               : 21/08/2016 20:59:16
 * Description        : usefull function and macros set header file
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
#ifndef __COMMON_H__
#define __COMMON_H__					HC090519	// version


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define ISCAPLETTER(c)    	    ((*(char*)c >= 'A') && (*(char*)c <= 'F'))
#define ISLCLETTER(c)     	    ((*(char*)c >= 'a') && (*(char*)c <= 'f'))
#define IS09(c)                 ((*(char*)c >= '0') && (*(char*)c <= '9'))
#define ISVALIDHEX(c)       	(ISCAPLETTER(c) || ISLCLETTER(c) || IS09(c))
#define ISVALIDDEC(c)     		((c >= '0') && (c <= '9'))
#define CONVERTDEC(c)       	(*(char*)c - '0')
#define CONVERTALPHA(c) 	    (ISCAPLETTER(c) ? (*(char*)c - 'A' + 10U) : (*(char*)c - 'a' + 10U))
#define CONVERTHEX(c)       	(IS09(c) ? CONVERTDEC(c) : CONVERTALPHA(c))
#define BCD2DEC(x)              ((((x) >> 4U) & 0x0FU) * 10U + ((x) & 0x0FU))
#define LEAP_YEAR(year)         ((((year) % 4U == 0U) && ((year) % 100U != 0U)) || ((year) % 400U == 0U))
#define DAYS_IN_YEAR(x)         (LEAP_YEAR(x) ? 366U : 365U)
#define UNIX_OFFSET_YEAR        1970U
#define SECONDS_PER_DAY         86400U
#define SECONDS_PER_HOUR        3600U
#define SECONDS_PER_MINUTE      60U
#define BUFF_LEN(__BUFFER__)   	(sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define COUNTOF(a)          	(sizeof(a) / sizeof(a[0]))
#define MIN(a,b)				(((a) < (b)) ? (a) : (b))
#define MAX(a,b)				(((a) > (b)) ? (a) : (b))
#define ZEROFILL(p, Size)   	(memset(p, 0, Size))
#define HEX2STR(s,h)            (Hex2Str(s, h, 2U)) // convert single hex to 2 char string null teminated
/** ============================================================================*/
/**			A S C I I       C O N T R O L       C H A R A C T E R S             */
/** ============================================================================*/
#define NUL     ((uint8_t)0x00U)    /* null control char                   */
#define SOH     ((uint8_t)0x01U)    /* start of header control character   */
#define STX     ((uint8_t)0x02U)    /* start of text control character     */
#define ETX     ((uint8_t)0x03U)    /* end of text control character       */
#define EOT     ((uint8_t)0x04U)    /* end of transmission control char    */
#define ENQ     ((uint8_t)0x05U)    /* enquiry control character           */
#define ACK     ((uint8_t)0x06U)    /* acknowledge control character       */
#define BEL     ((uint8_t)0x07U)    /* bell control character              */
#define BS      ((uint8_t)0x08U)    /* backspace control character         */
#define TAB     ((uint8_t)0x09U)    /* horizontal tab control character    */
#define LF      ((uint8_t)0x0AU)    /* line feed control character         */
#define VT      ((uint8_t)0x0BU)    /* vertical tab control character      */
#define FF      ((uint8_t)0x0CU)    /* form feed new page control char     */
#define CRT     ((uint8_t)0x0DU)    /* carriage return control char        */
#define SO      ((uint8_t)0x0EU)    /* shift out control character         */
#define SI      ((uint8_t)0x0FU)    /* shift in control character          */
#define NAK     ((uint8_t)0x15U)    /* negative acknowledge control char   */
#define ETB     ((uint8_t)0x17U)    /* end of transfer block control char  */
#define CAN     ((uint8_t)0x17U)    /* cancel control char                 */
#define SUB     ((uint8_t)0x1AU)    /* supstitute control char             */
#define ESC     ((uint8_t)0x1BU)    /* escape control char                 */
#define FS      ((uint8_t)0x1CU)    /* file separator control char         */
#define GS      ((uint8_t)0x1DU)    /* group separator control char        */
#define RS      ((uint8_t)0x1EU)    /* record separator control char       */
#define US      ((uint8_t)0x1FU)    /* unit separator control char         */

/* Exported function  ------------------------------------------------------- */
void DelayMs(__IO uint32_t delay_time);
uint8_t Bcd2Dec(uint8_t val);
uint8_t Dec2Bcd(uint8_t val);
void CharToBinStr(char c, char *pstr);
uint32_t GetSize(const uint8_t *pbuf);
uint32_t BaseToPower(uint16_t base, uint8_t power);
uint8_t CalcCRC(const uint8_t *pbuf, uint16_t size);
int32_t Str2Int(const char *pstr, uint8_t str_size);
uint8_t Int2Str(char *pstr, int32_t val, uint8_t str_size);
void Str2Hex(const char *pstr, uint8_t *phex, uint16_t str_size);
void Hex2Str(char *pstr, const uint8_t *phex, uint16_t str_size);
uint8_t RTC_ByteToBcd2(uint8_t Value);
uint8_t RTC_Bcd2ToByte(uint8_t Value);

#endif  /* __COMMON_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
