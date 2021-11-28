/**
 ******************************************************************************
 * Project          : KuceDzevadova
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
#ifndef __ASK_H__
#define __ASK_H__                               FW_BUILD // version
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "askcfg.h"
/* Exported Define  ----------------------------------------------------------*/
#define   _ASK_EDGE    (_ASK_MAX_DATA_BYTE*16+2)
/* Exported Type  ------------------------------------------------------------*/
typedef struct{
    GPIO_TypeDef  *gpio;
    uint16_t      pin;
    uint32_t      lastPinChangeTimeMs;
    uint16_t      lastCNT;
    uint8_t       newFrame;
    uint8_t       endFrame;
    uint8_t       index;    
    uint16_t      dataRawStart;       
    uint8_t       dataRaw[_ASK_EDGE];
    uint16_t      dataRawEnd;
    uint8_t       data[_ASK_MAX_DATA_BYTE];
    uint8_t       dataLast[_ASK_MAX_DATA_BYTE];
    uint8_t       dataLen;
    uint8_t       dataAvailable;
    uint32_t      dataTime;
}ask_t;
/* Exported functions ------------------------------------------------------- */
void    ask_init(ask_t *rf, GPIO_TypeDef  *gpio, uint16_t  pin);
void    ask_callBackPinChange(ask_t *rf);
void    ask_loop(ask_t *rf);
bool    ask_available(ask_t *rf);
bool    ask_read(ask_t *rf, uint8_t *code, uint8_t *codeLenInByte, uint8_t *syncTime_us);
/*  return -1 if faild: return >= 0 , pressed channel.*/
int16_t ask_checkChannelLast4Bit(uint8_t *newCode, uint8_t *refrence, uint8_t len);
int16_t ask_checkChannelLast8Bit(uint8_t *newCode, uint8_t *refrence, uint8_t len);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
