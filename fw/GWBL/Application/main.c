/**
  * @file    main.c 
  * @author  eldar6776@hotmail.com
  * @version V1,0
  * @date    06.03.2017
  * @brief   Main Program
  */
#include "ff.h"
#include "rtc.h"
#include "dio.h"
#include "main.h"
#include "command.h"
#include "flash_if.h"
#include "spi_flash.h"
#include "i2c_eeprom.h"
#include "stm32f429i_lcd.h"
/* Constante ----------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern FATFS fatfs;
extern FIL file;
extern FIL fileR;
extern DIR dir;
extern FILINFO fno;
/* Private function prototypes -----------------------------------------------*/
uint32_t system_id;
uint32_t system_config;
__IO uint32_t TimingDelay;
#ifdef USE_WATCHDOG
static void IWDG_Init(void);
#endif
/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int main(void)
{
	RTC_Config();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
#ifdef USE_WATCHDOG
    IWDG_Init();
#endif
	DIO_Init();
	I2C_EERPOM_Init();
	system_config = (uint8_t)I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_SYSTEM_CONFIG_ADDRESS);
	if(GPIO_ReadInputDataBit(TASTER_S3_PORT, TASTER_S3_PIN) == Bit_RESET)
	{
		BootloaderDeactivate();
        I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_SYSTEM_CONFIG_ADDRESS, (uint8_t) system_config);
        ApplicationExe();
        
	}
	if(!IsBootloaderRequested() && (GPIO_ReadInputDataBit(TASTER_S2_PORT, TASTER_S2_PIN) == Bit_RESET))
	{
		ApplicationExe();
	}
		
	LCD_Init(); 
	LCD_LayerInit();
	LTDC_Cmd(ENABLE);
	LCD_SetLayer(LCD_FOREGROUND_LAYER);
	LCD_Clear(LIGHTBLUE);
	LCD_SetTextColor(WHITE);
	LCD_SetBackColor(LIGHTBLUE);
	LCD_DisplayStringLine(LCD_LINE_2, (uint8_t*)"    BOOTLOADER AKTIVAN");
	BUZZER_On();
	TimingDelay = 100;
	while(TimingDelay) continue;
	BUZZER_Off();
#ifdef USE_WATCHDOG
		IWDG_ReloadCounter();
#endif
	/* Initialises the File System*/
	if (f_mount(&fatfs, "0:", 0) != FR_OK ) 
	{
		BootloaderSDcardError();
		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_SYSTEM_CONFIG_ADDRESS, (uint8_t) system_config);
		LCD_DisplayStringLine(LCD_LINE_4, (uint8_t*)"Greska kod citanja kartice");
		LCD_DisplayStringLine(LCD_LINE_5, (uint8_t*)"1. SD kartica nije ubacena");
		LCD_DisplayStringLine(LCD_LINE_6, (uint8_t*)"2. SD kartica neispravna");
		LCD_DisplayStringLine(LCD_LINE_7, (uint8_t*)"3. pogresan format SD kartice");
		LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*)"Restart aplikacije za 2 sekunde");
#ifdef USE_WATCHDOG
		IWDG_ReloadCounter();
#endif
		TimingDelay = 2000;
		while(TimingDelay) continue;
		ApplicationExe();
	}
	/* Flash unlock */
	FLASH_If_FlashUnlock();
	COMMAND_Download();
	BUZZER_On();
	TimingDelay = 100;
	while(TimingDelay) continue;
	BUZZER_Off();
	TimingDelay = 100;
	while(TimingDelay) continue;
	BUZZER_On();
	TimingDelay = 100;
	while(TimingDelay) continue;
	BUZZER_Off();
	BootloaderUpdateSucces();
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_SYSTEM_CONFIG_ADDRESS, (uint8_t) system_config);
	LCD_DisplayStringLine(LCD_LINE_8, (uint8_t*)"-upis firmware-a uspjesan");
	LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*)"Start aplikacije za 3 sekunde");
#ifdef USE_WATCHDOG
		IWDG_ReloadCounter();
#endif
	TimingDelay = 1000;
	while(TimingDelay) continue;
	ApplicationExe();
	
	while (1) 
	{
		LED2_Toggle();
		Delay(500);
	}
}

void ApplicationExe(void)
{
    pFunction Jump_To_Application;
    uint32_t JumpAddress;
	
    if (((*(__IO uint32_t*)HC_APPL_ADDR) & 0x2FFC0000) == 0x20000000)
    {
		/* Jump to user application */
		JumpAddress = *(__IO uint32_t*) (HC_APPL_ADDR + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) HC_APPL_ADDR);
		Jump_To_Application();
    }
	BUZZER_On();
	TimingDelay = 200;
	while(TimingDelay) continue;
	BUZZER_Off();
	BootloaderActivate();
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_SYSTEM_CONFIG_ADDRESS, (uint8_t) system_config);
	Fail_Handler();
}

void Fail_Handler(void)
{
	while(1)
	{
		/* Toggle Red LED */
		LED1_Toggle();
		Delay(50);
	}
}

void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}


#ifdef USE_WATCHDOG
static void IWDG_Init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);   // Can make at first before visit the register is written
	IWDG_SetPrescaler(IWDG_Prescaler_128);           // 
	IWDG_SetReload(4095U);                          // 
	IWDG_ReloadCounter();                           // Reload IWDG counter
	IWDG_Enable();                                  // Enable IWDG the LSI oscillator will be enabled by hardware
}
#endif
