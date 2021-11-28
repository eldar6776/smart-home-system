/**
 ******************************************************************************
 * File Name          : wiegand.c
 * Date               : 
 * Description        : wiegand interface control module
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "wiegand.h"

#if (__WIEGAND_H__ != FW_BUILD)
    #error "wiegand header version mismatch"
#endif

/* Private define ------------------------------------------------------------*/
#define WGDIG0  "0000"
#define WGDIG1  "0001"
#define WGDIG2  "0010"
#define WGDIG3  "0011"
#define WGDIG4  "0100"
#define WGDIG5  "0101"
#define WGDIG6  "0110"
#define WGDIG7  "0111"
#define WGDIG8  "1000"
#define WGDIG9  "1001"
#define WGSTAR  "1010"
#define WGHASH  "1011"
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
char wgbuf[256];
/* Private macro -------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
__STATIC_INLINE void usDelay(__IO uint32_t micros);
/* Private functions ---------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void WIEGAND_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE); 
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE , ENABLE);    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_5|GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_9|GPIO_Pin_10);
    GPIO_SetBits(GPIOC, GPIO_Pin_6|GPIO_Pin_7);
    GPIO_SetBits(GPIOE, GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_5|GPIO_Pin_6);
}
  /**
  * @brief
  * @param
  * @retval
  */
uint8_t WIEGAND_Send(char interface, char *buf)
{
    char *pwgbuf = wgbuf;
    uint32_t bcnt, bsz;
    bsz = strlen(buf);
    if (!bsz) return (1U);
    ZEROFILL(wgbuf, COUNTOF(wgbuf));
    do
    {
        bcnt = strlen(pwgbuf);
        if      (*buf == '0') strcpy(&pwgbuf[bcnt],WGDIG0); 
        else if (*buf == '1') strcpy(&pwgbuf[bcnt],WGDIG1);
        else if (*buf == '2') strcpy(&pwgbuf[bcnt],WGDIG2);
        else if (*buf == '3') strcpy(&pwgbuf[bcnt],WGDIG3);
        else if (*buf == '4') strcpy(&pwgbuf[bcnt],WGDIG4);
        else if (*buf == '5') strcpy(&pwgbuf[bcnt],WGDIG5);
        else if (*buf == '6') strcpy(&pwgbuf[bcnt],WGDIG6);
        else if (*buf == '7') strcpy(&pwgbuf[bcnt],WGDIG7);
        else if (*buf == '8') strcpy(&pwgbuf[bcnt],WGDIG8);
        else if (*buf == '9') strcpy(&pwgbuf[bcnt],WGDIG9);
        else if (*buf == '*') strcpy(&pwgbuf[bcnt],WGSTAR);
        else if (*buf == '%') strcpy(&pwgbuf[bcnt],WGHASH);
        else return (1U);
    }
    while(--bsz && *buf++);
    
    pwgbuf = wgbuf;
    bsz = strlen(wgbuf);
    
    do
    {
        bcnt = 4;
        do
        {
            if      (interface == '1')
            {
                if      (*pwgbuf == '0') GPIO_ResetBits(GPIOB, GPIO_Pin_9);
                else if (*pwgbuf == '1') GPIO_ResetBits(GPIOB, GPIO_Pin_10);  
            }
            else if (interface == '2')
            {
                if      (*pwgbuf == '0') GPIO_ResetBits(GPIOC, GPIO_Pin_6);
                else if (*pwgbuf == '1') GPIO_ResetBits(GPIOC, GPIO_Pin_7);
                else return (1U);                
            }
            else if (interface == '3')
            {
                if      (*pwgbuf == '0') GPIO_ResetBits(GPIOE, GPIO_Pin_2);
                else if (*pwgbuf == '1') GPIO_ResetBits(GPIOE, GPIO_Pin_3);
                else return (1U);                
            }
            else if (interface == '4')
            {
                if      (*pwgbuf == '0') GPIO_ResetBits(GPIOE, GPIO_Pin_5);
                else if (*pwgbuf == '1') GPIO_ResetBits(GPIOE, GPIO_Pin_6);
                else return (1U);
            }
            else return (1U);
            usDelay(100);   // bit valid time
            GPIO_SetBits(GPIOB, GPIO_Pin_9|GPIO_Pin_10);
            GPIO_SetBits(GPIOC, GPIO_Pin_6|GPIO_Pin_7);
            GPIO_SetBits(GPIOE, GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_5|GPIO_Pin_6);
            DelayMs(2);     // delay till next bit
            ++pwgbuf;
            --bcnt;
        }
        while (bcnt && *pwgbuf);
        DelayMs(200);       // delay till next byte
        --bsz;
    }
    while (bsz && *pwgbuf);
    
    return (0U);
}
  /**
  * @brief
  * @param
  * @retval
  */
__STATIC_INLINE void usDelay(__IO uint32_t micros) 
{
    __IO uint32_t usdel = micros * 22;
    while (usdel--);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
