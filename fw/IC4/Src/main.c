/**
 ******************************************************************************
 * Project          : KuceDzevadova
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

#if (__MAIN_H__ != FW_BUILD)
    #error "main header version mismatch"
#endif

#ifndef ROOM_THERMOSTAT
    #error "room thermostat not selected for application in common.h"
#endif

#ifndef APPLICATION
    #error "application not selected for application type in common.h"
#endif
/* Includes ------------------------------------------------------------------*/
#include "png.h"
#include "main.h"
#include "rs485.h"
#include "display.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"
/* Constants -----------------------------------------------------------------*/
/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
RTC_t date_time; 
RTC_TimeTypeDef rtctm;
RTC_DateTypeDef rtcdt;
RTC_HandleTypeDef hrtc;
CRC_HandleTypeDef hcrc;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;
TIM_HandleTypeDef htim9;
I2C_HandleTypeDef hi2c4;
I2C_HandleTypeDef hi2c3;
IWDG_HandleTypeDef hiwdg;
QSPI_HandleTypeDef hqspi;
LTDC_HandleTypeDef hltdc;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA2D_HandleTypeDef hdma2d;
/* Private Define ------------------------------------------------------------*/
#define TS_UPDATE_TIME              20U     // 50ms touch screen update period
#define AMBIENT_NTC_RREF            10000U  // 10k NTC value of at 25 degrees
#define AMBIENT_NTC_B_VALUE         3977U   // NTC beta parameter
#define AMBIENT_NTC_PULLUP          10000U	// 10k pullup resistor
#define FANC_NTC_RREF               2000U  	// 2k fancoil NTC value of at 25 degrees
#define FANC_NTC_B_VALUE            3977U   // NTC beta parameter
#define FANC_NTC_PULLUP             2200U	// 2k2 pullup resistor
#define ADC_READOUT_PERIOD          12U     // 89 ms ntc conversion rate
#define SYSTEM_STARTUP_TIME         8765U   // 8s application startup time
/* Private Variable ----------------------------------------------------------*/
uint8_t sysfl   = 0, initfl = 0;
uint32_t rstsrc = 0;
static float adc_refcor = 1.0f;
/* Private Macro -------------------------------------------------------------*/
#define VREFIN_CAL_ADDRESS      ((uint16_t*) (0x1FF0F44A))
#define TEMPSENSOR_CAL1_ADDR    ((uint16_t*) (0x1FF0F44C))
#define TEMPSENSOR_CAL2_ADDR    ((uint16_t*) (0x1FF0F44E))
/* Private Function Prototype ------------------------------------------------*/
static void RAM_Init(void);
static void MPU_Config(void);
static void MX_RTC_Init(void);
static void MX_CRC_Init(void);
static void SaveResetSrc(void);
static void CACHE_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM9_Init(void);
static void MX_UART_Init(void);
static void MX_IWDG_Init(void);
static void MX_CRC_DeInit(void);
static void MX_RTC_DeInit(void);
static void MX_TIM9_DeInit(void);
static void MX_I2C3_DeInit(void);
static void MX_I2C4_DeInit(void);
static void MX_GPIO_DeInit(void);
static void MX_UART_DeInit(void);
static void SystemClock_Config(void);
static uint32_t RTC_GetUnixTimeStamp(RTC_t* data);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
int main(void){
    SaveResetSrc();
	MPU_Config();
	CACHE_Config();
	HAL_Init(); 
	SystemClock_Config();
	MX_IWDG_Init();
    MX_CRC_Init();
	MX_RTC_Init();   
	MX_TIM9_Init();
	MX_GPIO_Init();
	MX_QSPI_Init();
    QSPI_MemMapMode();
    SDRAM_Init();
	EE_Init();
    TS_Init();
    RAM_Init();
    MX_UART_Init();
    RS485_Init();
    DISPInit();
    PresentSystem();
#ifdef	USE_WATCHDOG    
    HAL_IWDG_Refresh(&hiwdg);
#endif

	while(1){
#ifdef	USE_WATCHDOG
        HAL_IWDG_Refresh(&hiwdg);
#endif
        TS_Service();
		DISPService();
        RS485_Service();
	}
}
/**
  * @brief
  * @param
  * @retval
  */
void TS_Service(void){
    uint16_t xDiff, yDiff;
    __IO TS_StateTypeDef  ts;    
    static GUI_PID_STATE TS_State = {0};
    static uint32_t ts_update_tmr = 0U;    
    if (IsDISPCleaningActiv()) return;
    else if (HAL_GetTick() - ts_update_tmr  >= TS_UPDATE_TIME){
        ts_update_tmr = HAL_GetTick();
        BSP_TS_GetState((TS_StateTypeDef *)&ts);
        if((ts.touchX[0] >= LCD_GetXSize())||(ts.touchY[0] >= LCD_GetYSize())){
            ts.touchX[0] = 0U;
            ts.touchY[0] = 0U;
            ts.touchDetected = 0U;
        }
        xDiff = (TS_State.x > ts.touchX[0]) ? (TS_State.x - ts.touchX[0]) : (ts.touchX[0] - TS_State.x);
        yDiff = (TS_State.y > ts.touchY[0]) ? (TS_State.y - ts.touchY[0]) : (ts.touchY[0] - TS_State.y);        
        if((TS_State.Pressed != ts.touchDetected) || (xDiff > 30U) || (yDiff > 30U)){
            TS_State.Pressed = ts.touchDetected;
            TS_State.Layer = TS_LAYER;            
            if(ts.touchDetected){
                TS_State.x = ts.touchX[0];
                TS_State.y = ts.touchY[0];
            }else{
                TS_State.x = 0;
                TS_State.y = 0;
            }
            GUI_TOUCH_StoreStateEx(&TS_State);
        }
    }
}
/**
  * @brief  
  * @param  
  * @retval 
  */
void SYSRestart(void){
    MX_GPIO_DeInit();
    MX_I2C3_DeInit();
    MX_I2C4_DeInit();
    MX_TIM9_DeInit();
    MX_UART_DeInit();
    HAL_QSPI_DeInit(&hqspi);
    MX_RTC_DeInit();
    MX_CRC_DeInit();
    HAL_RCC_DeInit();
    HAL_DeInit();
    SCB_DisableICache();
    SCB_DisableDCache();
    HAL_NVIC_SystemReset();
}
/**
  * @brief
  * @param
  * @retval
  */
void ErrorHandler(uint8_t function, uint8_t driver){	
    SYSRestart();
}
/**
  * @brief  Convert from Binary to 2 digit BCD.
  * @param  Value: Binary value to be converted.
  * @retval Converted word
  */
void RTC_GetDateTime(RTC_t* data, uint32_t format){
	uint32_t unix;
	HAL_RTC_GetTime(&hrtc, &rtctm, format);
	data->hours = rtctm.Hours;
	data->minutes = rtctm.Minutes;
	data->seconds = rtctm.Seconds;
	data->subseconds = RTC->SSR;
	HAL_RTC_GetDate(&hrtc, &rtcdt, format);
	data->year = rtcdt.Year;
	data->month = rtcdt.Month;
	data->date = rtcdt.Date;
	data->day = rtcdt.WeekDay;
	unix = RTC_GetUnixTimeStamp(data);
	data->unix = unix;
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart){
    if (huart->Instance == USART1){
        RS485_RxCpltCallback();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart){
    if (huart->Instance == USART1){
        RS485_TxCpltCallback();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart){
    if (huart->Instance == USART1){
        RS485_ErrorCallback();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc){
	__HAL_RCC_RTC_ENABLE(); 
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc){
	__HAL_RCC_RTC_DISABLE();
}
/**
  * @brief
  * @param
  * @retval
  */
void PresentSystem(void){
#ifdef	USE_WATCHDOG
    char buf[64];
    uint16_t AD_value;
    int32_t raw_value, x;
    float core_temp_avg_slope, TemperatureC, McuVoltage;
    uint32_t romsz = *(__IO uint16_t*)(0x1FF0F442);
    ADC_ChannelConfTypeDef sConfig;
    __HAL_RCC_ADC1_CLK_ENABLE();
    hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.NbrOfDiscConversion = 0U;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1U;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if(HAL_ADC_Init(&hadc1) != HAL_OK){
		ErrorHandler(MAIN_FUNC, ADC_DRV);
	}
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    ADC->CCR |= (uint32_t)ADC_CCR_TSVREFE;
    __HAL_ADC_ENABLE(&hadc1);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    raw_value = HAL_ADC_GetValue(&hadc1);
    core_temp_avg_slope = (*TEMPSENSOR_CAL2_ADDR - *TEMPSENSOR_CAL1_ADDR) / 80.0;
    TemperatureC = (((float)raw_value * adc_refcor - *TEMPSENSOR_CAL1_ADDR) / core_temp_avg_slope) + 30.0f;
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    AD_value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_DeInit(&hadc1);
    McuVoltage = (4095.0f * 1.15f / (float)AD_value);
    x = HAL_GetUIDw0();
    while (x>10000) x /= 10;
    HAL_Delay(x);
    GUI_SelectLayer(0);
    GUI_SetColor(GUI_BLACK);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT);
    GUI_Clear();
    GUI_Exec();
    strcpy(buf, "\r\n\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    strcpy(buf, "--------------------------------------------------------\r\n");
    GUI_GotoXY(40 ,10);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    strcpy(buf, "LuxHOME Intergrated Controller\r\n");
    GUI_GotoXY(40 ,25);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    strcpy(buf, "--------------------------------------------------------\r\n");
    GUI_GotoXY(40 ,40);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    if (romsz >= 0x400){
        romsz /= 0x400;
        sprintf(buf, "Flash size: %d MB\r\n", romsz); 
    } else sprintf(buf, "Flash size: %d KB\r\n", romsz); 
    GUI_GotoXY(40 ,55);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Core Clock Freqency: %d MHz\r\n", HAL_RCC_GetSysClockFreq()/1000000);
    GUI_GotoXY(40 ,70);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Core Supply Voltage: %.3f mV\r\n", McuVoltage);
    GUI_GotoXY(40 ,85);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Core Temperature: %.1f 'C\r\n", TemperatureC);
    GUI_GotoXY(40 ,100);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "ID Device: %d\r\n", HAL_GetDEVID());
    GUI_GotoXY(40 ,115);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "ID Unique: %d-%d-%d\r\n", HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2());
    GUI_GotoXY(40 ,130);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Version Bootloader: ICBL%X\r\n", *(__IO uint32_t*)(RT_BLDR_VERS_ADDR+8)&0x00FFFFFF);
    GUI_GotoXY(40 ,145);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Version Firmware: IC%X\r\n", *(__IO uint32_t*)(RT_APPL_VERS_ADDR+8)&0x00FFFFFF);
    GUI_GotoXY(40 ,160);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "RS485 Interface Address: %d\r\n", tfifa);
    GUI_GotoXY(40 ,175);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "RS485 Group Address: %d\r\n", tfgra);
    GUI_GotoXY(40 ,190);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "RS485 Broadcast Address: %d\r\n", tfbra);
    GUI_GotoXY(40 ,205);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "RS485 Gateway Address: %d\r\n", tfgwa);
    GUI_GotoXY(40 ,220);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    strcpy(buf, "--------------------------------------------------------\r\n\r\n");
    GUI_GotoXY(40 ,235);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString(buf);
    GUI_Exec();
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
    DISPResetScrnsvr();
    for (x=0; x<10; x++){
        HAL_Delay(1000);
        HAL_IWDG_Refresh(&hiwdg);
    }
#endif 
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_IWDG_Init(void){
#ifdef	USE_WATCHDOG
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    hiwdg.Init.Window = 4095;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK){
        SYSRestart();
    }
#endif
}
/**
  * @brief
  * @param
  * @retval
  */
static void RAM_Init(void){
    int x;
    uint8_t ebuf[EE_INIT_ADDR+2];
    EE_ReadBuffer(ebuf, EE_TERMFL, EE_INIT_ADDR+2);    
    if (ebuf[EE_INIT_ADDR] != EE_MARKER){
        ZEROFILL(ebuf, COUNTOF(ebuf));
        ebuf[EE_DISP_LOW_BCKLGHT]  =  5;
        ebuf[EE_DISP_HIGH_BCKLGHT] = 70;
        ebuf[EE_SCRNSVR_TOUT] = SCRNSVR_TOUT;
        ebuf[EE_SCRNSVR_ENABLE_HOUR] = 22;
        ebuf[EE_SCRNSVR_DISABLE_HOUR]= 6;
        ebuf[EE_SCRNSVR_CLK_COLOR] = 15;
        x = HAL_GetUIDw0();
        while (x>255) x /= 10;
        while ((x == DEF_TFBRA)||(x == DEF_TFGRA)||(x == DEF_TFGWA)){
            if (++x > 255) x = 0;
        }
        ebuf[EE_TFIFA]  = x&0xFF;
        ebuf[EE_TFGRA]  = DEF_TFGRA;
        ebuf[EE_TFBRA]  = DEF_TFBRA;
        ebuf[EE_TFGWA]  = DEF_TFGWA;
        ebuf[EE_TFBPS]  =((DEF_TFBPS>>24)&0xFF);
        ebuf[EE_TFBPS+1]=((DEF_TFBPS>>16)&0xFF);
        ebuf[EE_TFBPS+2]=((DEF_TFBPS>> 8)&0xFF);
        ebuf[EE_TFBPS+3]= (DEF_TFBPS&0xFF);
        ebuf[EE_SYSID]  =((DEF_SYSID>> 8)&0xFF);
        ebuf[EE_SYSID+1]= (DEF_SYSID&0xFF);
        ebuf[EE_CTRL1]   = 1;
        ebuf[EE_CTRL1+1] = 2;
        ebuf[EE_CTRL1+2] = 3;
        ebuf[EE_CTRL1+3] = 4;
        ebuf[EE_CTRL1+4] = 5;
        ebuf[EE_CTRL1+5] = 1;
        ebuf[EE_CTRL1+6] = 2;
        ebuf[EE_CTRL1+7] = 3;        
        ebuf[EE_CTRL1]   = 1;
        ebuf[EE_CTRL1+1] = 0;
        ebuf[EE_CTRL1+2] = 0;
        ebuf[EE_CTRL1+3] = 0;
        ebuf[EE_CTRL1+4] = 0;
        ebuf[EE_CTRL1+5] = 0x27;
        ebuf[EE_CTRL1+6] = 0x10;
        ebuf[EE_CTRL1+7] = 0x0F;
        ebuf[EE_CTRL1+8] = 0x89;
        ebuf[EE_CTRL1+9] = 24;
        ebuf[EE_CTRL1+10] = 5;
        ebuf[EE_CTRL1+11] = 32;
        ebuf[EE_CTRL1+12] = 16;
        ebuf[EE_CTRL1+13] = 1;
        ebuf[EE_CTRL1+14] = 0;
        ebuf[EE_CTRL1+15] = 5;
        ebuf[EE_CTRL1+16] = 10;
        ebuf[EE_CTRL1+17] = 15;
        ebuf[EE_CTRL1+18] = 22;
        ebuf[EE_CTRL1+19] = 22;
        ebuf[EE_CTRL1+20] = 1;        
        ebuf[EE_INIT_ADDR] = EE_MARKER;
        EE_WriteBuffer(ebuf, EE_TERMFL, EE_INIT_ADDR+2);
        ZEROFILL(ebuf, COUNTOF(ebuf));
        EE_ReadBuffer(ebuf, EE_TERMFL, EE_INIT_ADDR+2);
    }    
    low_bcklght         = ebuf[EE_DISP_LOW_BCKLGHT];        // EE_DISPLOW_BCKLGHT       0xAU
    high_bcklght        = ebuf[EE_DISP_HIGH_BCKLGHT];       // EE_DISPHIGH_BCKLGHT      0xBU
    scrnsvr_tout        = ebuf[EE_SCRNSVR_TOUT];            // EE_SCRNSVR_TOUT          0xCU
    scrnsvr_ena_hour    = ebuf[EE_SCRNSVR_ENABLE_HOUR];     // EE_SCRNSVR_ENABLE_HOUR   0xDU
    scrnsvr_dis_hour    = ebuf[EE_SCRNSVR_DISABLE_HOUR];    // EE_SCRNSVR_DISABLE_HOUR  0xEU
    scrnsvr_clk_clr     = ebuf[EE_SCRNSVR_CLK_COLOR];       // EE_SCRNSVR_CLK_COLOR     0xFU
    if (ebuf[EE_SCRNSVR_ON_OFF] == 1) ScrnsvrClkSet();
    else ScrnsvrClkReset();
    sysfl               = ebuf[EE_SYS_STATE];
    tfifa               = ebuf[EE_TFIFA];                   // EE_TFIFA                 0x1CU	// rs485 device address
    tfgra               = ebuf[EE_TFGRA];                   // EE_TFGRA                 0x1EU	// rs485 group broadcast address
    tfbra               = ebuf[EE_TFBRA];                   // EE_TFBRA                 0x20U	// rs485 broadcast address msb
    tfgwa               = ebuf[EE_TFGWA];                   // EE_TFGWA
    tfbps = ((ebuf[EE_TFBPS]<<24)|(ebuf[EE_TFBPS+1]<<16)|(ebuf[EE_TFBPS+2]<<8)|ebuf[EE_TFBPS+3]); // EE_RSBPS 0x22U	// rs485 interface baudrate
    sysid = ((ebuf[EE_SYSID]<<8)|ebuf[EE_SYSID+1]);         // EE_SYSID                 0x2AU	// system id (system unique number)
    if (high_bcklght == 0) high_bcklght = 1;
    if (low_bcklght >= high_bcklght) low_bcklght = high_bcklght;
    if (scrnsvr_tout == 0) scrnsvr_tout = 1;
    if (scrnsvr_ena_hour > 23) scrnsvr_ena_hour = 0;
    if (scrnsvr_dis_hour > 23) scrnsvr_dis_hour = 0;
    if (scrnsvr_clk_clr >= COLOR_BSIZE) scrnsvr_clk_clr = 0;
}
/**
  * @brief
  * @param
  * @retval
  */
static void SaveResetSrc(void){
    if      (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))  rstsrc = LOW_POWER_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))   rstsrc = POWER_ON_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))   rstsrc = SOFTWARE_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))  rstsrc = IWDG_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))   rstsrc = PIN_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))  rstsrc = WWDG_RESET;
    else                                            rstsrc = 0U;
	 __HAL_RCC_CLEAR_RESET_FLAGS();
}
/**
  * @brief
  * @param
  * @retval
  */
static void MPU_Config(void){
	MPU_Region_InitTypeDef MPU_InitStruct;
	HAL_MPU_Disable();
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x20010000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_256KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
	MPU_InitStruct.BaseAddress      = 0x90000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_256MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
	MPU_InitStruct.BaseAddress      = 0x90000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_16MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER4;
	MPU_InitStruct.BaseAddress      = 0xC0000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_512MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER5;
	MPU_InitStruct.BaseAddress      = 0xC0000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
	FMC_Bank1->BTCR[0] = 0x000030D2U;
}
/**
  * @brief
  * @param
  * @retval
  */
static void CACHE_Config(void){	
    (*(uint32_t *) 0xE000ED94) &= ~0x5;
	(*(uint32_t *) 0xE000ED98) = 0x0; //MPU->RNR
	(*(uint32_t *) 0xE000ED9C) = 0x20010000 | 1 << 4; //MPU->RBAR
	(*(uint32_t *) 0xE000EDA0) = 0 << 28 | 3 << 24 | 0 << 19 | 0 << 18 | 1 << 17 | 0 << 16 | 0 << 8 | 30 << 1 | 1 << 0; //MPU->RASE  WT
	(*(uint32_t *) 0xE000ED94) = 0x5;
	SCB_InvalidateICache();
	SCB->CCR |= (1 << 18);
	__DSB();
	SCB_EnableICache();
	SCB_InvalidateDCache();
	SCB_EnableDCache();
}
/**
  * @brief
  * @param
  * @retval
  */
static void SystemClock_Config(void){
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    if (HAL_PWREx_EnableOverDrive() != HAL_OK){
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK){
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_I2C3|RCC_PERIPHCLK_I2C4;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 57;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 3;
    PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
    PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
    PeriphClkInitStruct.PLLSAIDivQ = 1;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000U);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_RTC_Init(void){
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    /**Initialize RTC Only 
    */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&hrtc) != HAL_OK){
        ErrorHandler(MAIN_FUNC, RTC_DRV);
    }    
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2){
        sTime.Hours = 0x0U;
        sTime.Minutes = 0x0U;
        sTime.Seconds = 0x0U;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;
        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK){
            ErrorHandler(MAIN_FUNC, RTC_DRV);
        }
        sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
        sDate.Month = RTC_MONTH_JANUARY;
        sDate.Date = 1;
        sDate.Year = 20;
        RtcTimeValidReset();
    }else{
        sDate.Date      = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
                sDate.Month     = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
                sDate.WeekDay   = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
                sDate.Year      = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);
        RtcTimeValidSet();
    }
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK){
        ErrorHandler(MAIN_FUNC, RTC_DRV);
    }
    __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
    if (HAL_RTC_WaitForSynchro(&hrtc) != HAL_OK){
        ErrorHandler(MAIN_FUNC, RTC_DRV);
    }
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_RTC_DeInit(void){
	HAL_RTC_DeInit(&hrtc);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_TIM9_Init(void){
	TIM_OC_InitTypeDef sConfigOC;
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_TIM9_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	htim9.Instance = TIM9;
	htim9.Init.Prescaler = 200U;
	htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim9.Init.Period = 1000U;
	htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim9) != HAL_OK){
		ErrorHandler(MAIN_FUNC, TMR_DRV);
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = DISP_BRGHT_MAX;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
		ErrorHandler(MAIN_FUNC, TMR_DRV);
	}
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_TIM9_DeInit(void){
	__HAL_RCC_TIM9_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_5);
	HAL_TIM_PWM_DeInit(&htim9);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_UART_Init(void){
	GPIO_InitTypeDef  GPIO_InitStruct;
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_RS485Ex_Init(&huart1, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK) ErrorHandler (MAIN_FUNC, USART_DRV);
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_UART_DeInit(void){
	__HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_USART2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_12);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_UART_DeInit(&huart1);
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static void MX_CRC_Init(void){
    hcrc.Instance                       = CRC;
    hcrc.Init.DefaultPolynomialUse      = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse       = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode    = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode   = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat                = CRC_INPUTDATA_FORMAT_BYTES;
    __HAL_RCC_CRC_CLK_ENABLE();
    if (HAL_CRC_Init(&hcrc) != HAL_OK){
        ErrorHandler(MAIN_FUNC, CRC_DRV);
    }
}
/**
  * @brief  
  * @param  
  * @retval 
  */
static void MX_CRC_DeInit(void){
    __HAL_RCC_CRC_CLK_DISABLE();
	HAL_CRC_DeInit(&hcrc);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_I2C3_DeInit(void){
    __HAL_RCC_I2C3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_9);
    HAL_I2C_DeInit(&hi2c3);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_I2C4_DeInit(void){
    __HAL_RCC_I2C4_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12 | GPIO_PIN_13);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_2);
    HAL_I2C_DeInit(&hi2c4);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_GPIO_Init(void){
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7,  GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3,  GPIO_PIN_RESET);
    
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
	GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_GPIO_DeInit(void){
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8|GPIO_PIN_12);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_11);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_3|GPIO_PIN_13|GPIO_PIN_14);
}
/**
  * @brief  Convert from Binary to 2 digit BCD.
  * @param  Value: Binary value to be converted.
  * @retval Converted word
  */
static uint32_t RTC_GetUnixTimeStamp(RTC_t* data){
	uint32_t days = 0U, seconds = 0U;
	uint16_t i;
	uint16_t year = (uint16_t) (data->year + 2000U);
	/* Year is below offset year */
	if (year < UNIX_OFFSET_YEAR) {
		return 0U;
	}	
	/* Days in back years */
	for (i = UNIX_OFFSET_YEAR; i < year; i++) {
		days += DAYS_IN_YEAR(i);
	}	
	/* Days in current year */
	for (i = 1U; i < data->month; i++) {
		days += rtc_months[LEAP_YEAR(year)][i - 1U];
	}	
	/* Day starts with 1 */
	days += data->date - 1U;
	seconds = days * SECONDS_PER_DAY;
	seconds += data->hours * SECONDS_PER_HOUR;
	seconds += data->minutes * SECONDS_PER_MINUTE;
	seconds += data->seconds;	
	return seconds;
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
