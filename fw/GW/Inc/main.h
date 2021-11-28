/**
 ******************************************************************************
 * File Name          : main.h
 * Date               : 21/08/2016 20:59:16
 * Description        : hotel controller main function file header
 ******************************************************************************
 *
 *
 ******************************************************************************/
 
 /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__					FW_BUILD	// version
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "common.h"
#include "LuxNET.h"
#include "TinyFrame.h"
/* Exported types ------------------------------------------------------------*/
extern uint16_t tfbps, tfbra, tfgra, tfifa, sysid;
/* Exported constants --------------------------------------------------------*/
/* Exporeted Variable  -------------------------------------------------------*/
extern uint32_t sys_cfg;
/* Exported Macro ------------------------------------------------------------*/
#define BLDR_Clear()                (sys_cfg &= 0xFFFFFF00)
#define BLDR_Disable()              (sys_cfg &= 0xFFFFFF0F)
#define BLDR_Enable()               (BLDR_Clear(),(sys_cfg |= (1U<<7)))
#define IsBLDR_FileError()          (sys_cfg & (1U<<4))
#define IsBLDR_UpdateOk()           (sys_cfg & (1U<<5))
#define IsBLDR_CardError()          (sys_cfg & (1U<<6))
#define IsBLDR_Activated()          (sys_cfg & (1U<<7))  

#define HTTP_ServerEnable()         (sys_cfg |= (0x1U << 8))
#define HTTP_ServerDisable()        (sys_cfg &= (~(0x1U<<8)))
#define IsHTTP_ServerEnabled()      (sys_cfg &  (0x1U << 8))
#define WEB_ConfigEnable()          (sys_cfg |= (0x1U << 9))
#define WEB_ConfigDisable()         (sys_cfg &= (~(0x1U<<9)))
#define IsWEB_ConfigEnabled()       (sys_cfg &  (0x1U << 9))
#define TFTP_ServerEnable()         (sys_cfg |= (0x1U << 10))
#define TFTP_ServerDisable()        (sys_cfg &= (~(0x1U<<10)))
#define IsTFTP_ServerEnabled()      (sys_cfg &  (0x1U << 10))
#define TIME_BroadcastEnable()      (sys_cfg |= (0x1U << 11))
#define TIME_BroadcastDisable()     (sys_cfg &= (~(0x1U<<11)))
#define IsTIME_BroadcastEnabled()   (sys_cfg &  (0x1U << 11))
#define DHCP_ClientEnable()         (sys_cfg |= (0x1U << 12))	
#define DHCP_ClientDisable()        (sys_cfg &= (~(0x1U<<12)))
#define IsDHCP_ClientEnabled()      (sys_cfg &  (0x1U << 12))
/* Exported Function  ------------------------------------------------------- */
void RAM_Init(void);
void ErrorHandler(uint8_t function, uint8_t driver);
#endif 
