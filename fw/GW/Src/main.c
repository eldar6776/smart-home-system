/**
 ******************************************************************************
 * File Name          : main.c
 * Date               : 21/08/2016 20:59:16
 * Description        : hotel controller main function
 ******************************************************************************
 *
 *
 ******************************************************************************
 *
 */
 
#if (__MAIN_H__ != FW_BUILD)
    #error "main header version mismatch"
#endif

#ifndef HOTEL_CONTROLLER
    #error "hotel controller not selected for application in common.h"
#endif

#ifndef APPLICATION
    #error "application not selected for application type in common.h"
#endif 
/* Includes ------------------------------------------------------------------*/
#include "ff.h"
#include "rtc.h"
#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "i2cee.h"
#include "httpd.h"
#include "fs5206.h"
#include "buzzer.h"
#include "eth_bsp.h"
#include "netconf.h"
#include "netbios.h"
#include "display.h"
#include "spi_flash.h"
#include "tftpserver.h"
#include "stm32f4x7_eth.h"
#include "stm32f429i_lcd.h"
/* Constante ----------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t min = 60;
uint16_t tfbps, tfbra, tfgra, tfifa,sysid;
uint32_t sys_cfg;
extern void TINYFRAME_Init(void);
/* Private function prototypes -----------------------------------------------*/

#ifdef USE_WATCHDOG
static void IWDG_Init(void);
#endif
/* Program Code --------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
int main(void){
#ifdef USE_WATCHDOG
    IWDG_Init();
#endif
	RTC_Config();
	I2CEE_Init();
	SPIFLASH_Init();
    RAM_Init();
	RS485_Init();
	LCD_Init();
	BUZZER_Init();
	LED_Init();
	Key_Init();
	FS5206_Init();
	ETHERNET_Init();
	LwIP_Init();
	TFTPD_Init();
	HTTPD_Init();
    NETBIOS_Init();
	DISP_Init();
    TINYFRAME_Init();
    
	while (1){
        LwIP_Periodic_Handle(Get_SysTick());
		if (ETH_CheckFrameReceived()) LwIP_Pkt_Handle();
		DISP_Service();
#ifdef USE_WATCHDOG
		IWDG_ReloadCounter();
#endif
	}
}
/**
  * @brief
  * @param
  * @retval
  */
void RAM_Init(void){
    
#ifdef INIT_DEFAULT
    //
    //  init system flags
    //
    ZEROFILL(eebuff, COUNTOF(eebuff));
    sys_cfg = 0x0U;
    DHCP_ClientEnable();
    TIME_BroadcastEnable();
    HTTP_ServerEnable();
    WEB_ConfigEnable();
    TFTP_ServerEnable();
    //
    //  init all parameters to default values 
    //
    eebuff[EE_ETH_IP_ADD]       = IP_ADDR0;
    eebuff[EE_ETH_IP_ADD + 1]   = IP_ADDR1;
    eebuff[EE_ETH_IP_ADD + 2]   = IP_ADDR2;
    eebuff[EE_ETH_IP_ADD + 3]   = IP_ADDR3;
    eebuff[EE_ETH_SUB_ADD]      = SUBNET_ADDR0;
    eebuff[EE_ETH_SUB_ADD + 1]  = SUBNET_ADDR1;
    eebuff[EE_ETH_SUB_ADD + 2]  = SUBNET_ADDR2;
    eebuff[EE_ETH_SUB_ADD + 3]  = SUBNET_ADDR3;
    eebuff[EE_ETH_GW_ADD]       = GW_ADDR0;
    eebuff[EE_ETH_GW_ADD + 1]   = GW_ADDR1;
    eebuff[EE_ETH_GW_ADD + 2]   = GW_ADDR2;
    eebuff[EE_ETH_GW_ADD + 3]   = GW_ADDR3;
    eebuff[EE_RS485_IFADD]      = (FST_HC_RSIFA >> 8);
    eebuff[EE_RS485_IFADD + 1]  = (FST_HC_RSIFA & 0xFFU);
    eebuff[EE_RS485_GRADD]      = (DEF_HC_RSGRA >> 8);
    eebuff[EE_RS485_GRADD + 1]  = (DEF_HC_RSGRA & 0xFFU);
    eebuff[EE_RS485_BRADD]      = (DEF_RSBRA >> 8);
    eebuff[EE_RS485_BRADD + 1]  = (DEF_RSBRA & 0xFFU);
    eebuff[EE_RS485_BAUD_ADD]   = 6; // convert to char digit 0-9
    eebuff[EE_SYS_CFG_ADD]      = ((sys_cfg >> 24) & 0xFFU);
    eebuff[EE_SYS_CFG_ADD + 1]  = ((sys_cfg >> 16) & 0xFFU);
    eebuff[EE_SYS_CFG_ADD + 2]  = ((sys_cfg >>  8) & 0xFFU);
    eebuff[EE_SYS_CFG_ADD + 3]  =  (sys_cfg        & 0xFFU);
    eebuff[EE_SYS_ID_ADD]       = (DEF_SYSID >> 8);
    eebuff[EE_SYS_ID_ADD + 1]   = (DEF_SYSID & 0xFFU);
    Int2Str((char*)&eebuff[EE_PASSWORD_ADD], DEF_PASSW, 0);
    //
    //  write all parameter to eeprom
    //
    if (I2CEE_WriteBytes16(I2CEE_PAGE_0, EE_ETH_IP_ADD, eebuff, 32) != 0)   ErrorHandler(MAIN_FUNC, I2C_DRV);
    DelayMs(I2CEE_WRITE_DELAY);
    
    if(HC_LoadAddrList() != FS_FILE_OK) DISP_uSDCardSetNewState(0);
    HC_CreateAddrList();
#endif

    ZEROFILL(eebuff, COUNTOF(eebuff));
    if(I2CEE_ReadBytes16(I2CEE_PAGE_0, EE_ETH_IP_ADD, eebuff, 32) != 0)     ErrorHandler(MAIN_FUNC, I2C_DRV);
    
#ifdef USE_CONSTANT_IP
    ip_add[0] = IP_ADDR0;
    ip_add[1] = IP_ADDR1;
    ip_add[2] = IP_ADDR2;
    ip_add[3] = IP_ADDR3;
    
    subnet[0] = SUBNET_ADDR0;
    subnet[1] = SUBNET_ADDR1;
    subnet[2] = SUBNET_ADDR2;
    subnet[3] = SUBNET_ADDR3;
    
    gw_add[0] = GW_ADDR0;
    gw_add[1] = GW_ADDR1;
    gw_add[2] = GW_ADDR2;
    gw_add[3] = GW_ADDR3;
#else
    memcpy(ip_add, &eebuff[EE_ETH_IP_ADD],  0x4U);
    memcpy(subnet, &eebuff[EE_ETH_SUB_ADD], 0x4U);
    memcpy(gw_add, &eebuff[EE_ETH_GW_ADD],  0x4U);
#endif

    tfifa = (uint16_t)((eebuff[EE_RS485_IFADD] << 8) | eebuff[EE_RS485_IFADD + 0x1U]);
    tfgra = (uint16_t)((eebuff[EE_RS485_GRADD] << 8) | eebuff[EE_RS485_GRADD + 0x1U]);
    tfbra = (uint16_t)((eebuff[EE_RS485_BRADD] << 8) | eebuff[EE_RS485_BRADD + 0x1U]);
                                        
    if(!IS09(&eebuff[EE_RS485_BAUD_ADD])) tfbps = BR_115200;
    else tfbps = TODEC(eebuff[EE_RS485_BAUD_ADD]);
    
    sys_cfg = (((eebuff[EE_SYS_CFG_ADD]         << 24) & 0xFF000000U) + 
               ((eebuff[EE_SYS_CFG_ADD + 0x1U]  << 16) & 0x00FF0000U) +
               ((eebuff[EE_SYS_CFG_ADD + 0x2U]  <<  8) & 0x0000FF00U) + 
                (eebuff[EE_SYS_CFG_ADD + 0x3U]         & 0x000000FFU));
                                
    sysid = (uint16_t)((eebuff[EE_SYS_ID_ADD] << 8) | eebuff[EE_SYS_ID_ADD + 0x1U]);

}
/**
  * @brief
  * @param
  * @retval
  */
void ErrorHandler(uint8_t function, uint8_t driver){
    NVIC_SystemReset();
}
/**
  * @brief
  * @param
  * @retval
  */
#ifdef USE_WATCHDOG
static void IWDG_Init(void){
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);   // Can make at first before visit the register is written
	IWDG_SetPrescaler(IWDG_Prescaler_32);           // (1/(32000/32))*4095 = 4,095s
	IWDG_SetReload(4095U);                          // 
	IWDG_ReloadCounter();                           // Reload IWDG counter
	IWDG_Enable();                                  // Enable IWDG the LSI oscillator will be enabled by hardware
}
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
