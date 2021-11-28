/**
  ******************************************************************************
  * @file    FW_upgrade/src/command.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   This file provides all the IAP command functions.
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
#include "ff.h"
#include "main.h"
#include "command.h"
#include "i2c_eeprom.h"
#include "stm32f429i_lcd.h"

/** @addtogroup STM32F429I-Discovery_FW_Upgrade
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define UPLOAD_FILENAME            "0:GW.BIN"
#define DOWNLOAD_FILENAME          "0:CTRL_NEW.BIN"
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FATFS fatfs;
FIL file;
FIL fileR;
DIR dir;
FILINFO fno;
static uint8_t RAM_Buf[BUFFER_SIZE] ={0};
static uint32_t TmpProgramCounter = 0x00, TmpReadSize = 0x00 , RamAddress = 0x00;
static uint32_t LastPGAddress = HC_APPL_ADDR;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  IAP Read all flash memory.
  * @param  None
  * @retval None
  */
void COMMAND_Upload(void){
	__IO uint32_t address = HC_APPL_ADDR;
	__IO uint32_t counterread = 0x00;
	uint32_t tmpcounter = 0x00, indexoffset = 0x00;
	FlagStatus readoutstatus = SET;
	uint16_t bytesWritten;
	readoutstatus = FLASH_If_ReadOutProtectionStatus();	
	if (readoutstatus == RESET){
		f_unlink (UPLOAD_FILENAME);
		indexoffset = (HC_APPL_ADDR - FLASH_ADDR);
		if (f_open(&file, UPLOAD_FILENAME, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK){
			while (indexoffset != FLASH_SIZE){
				for (counterread = 0; counterread < BUFFER_SIZE; counterread++){
					if (indexoffset + counterread != FLASH_SIZE){
						tmpcounter = counterread;
						RAM_Buf[tmpcounter] = (*(uint8_t*)(address++));
					} else {
                        break;
                    }
				}
				f_write (&file, RAM_Buf, BUFFER_SIZE, (void *)&bytesWritten);
				indexoffset = indexoffset + counterread;
			}
			f_close (&file);
			f_mount(0,"0:",0);
		}
	} else {
		Fail_Handler();
	}
}

/**
  * @brief  IAP write memory.
  * @param  None
  * @retval None
  */
void COMMAND_Download(void){
	if (f_open(&fileR, DOWNLOAD_FILENAME, FA_READ) == FR_OK){
		LCD_DisplayStringLine(LCD_LINE_4, (uint8_t*)"-file pronadjen");		
		if (fileR.obj.objsize > FLASH_SIZE){
			BootloaderFileError();
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_SYSTEM_CONFIG_ADDRESS, (uint8_t) system_config);
			LCD_DisplayStringLine(LCD_LINE_5, (uint8_t*)"Greska velicine fajla");
			LCD_DisplayStringLine(LCD_LINE_6, (uint8_t*)"-fajl je prevelik");
			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*)"Restart aplikacije za 2 sekunde");
			TimingDelay = 2000;
			while(TimingDelay) continue;
			ApplicationExe();
			Fail_Handler();
		} else {  
			LCD_DisplayStringLine(LCD_LINE_5, (uint8_t*)"-formatiranje flash memorije");
			if (FLASH_If_EraseSectors(HC_APPL_ADDR) != 0x00){
				LCD_DisplayStringLine(LCD_LINE_6, (uint8_t*)"Greska formatiranja memorije");
				LCD_DisplayStringLine(LCD_LINE_7, (uint8_t*)"1. smetnja napajanja");
				LCD_DisplayStringLine(LCD_LINE_8, (uint8_t*)"2. neispravan flash uControlera");
				LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*)"Pokusajte restart");
				Fail_Handler();
			}			
			LCD_DisplayStringLine(LCD_LINE_6, (uint8_t*)"-memorija formatirana");
			LCD_DisplayStringLine(LCD_LINE_7, (uint8_t*)"-upis novog firmware-a");
			COMMAND_ProgramFlashMemory();
			f_close (&fileR);
		}
	} else {		
		Fail_Handler();
	}
}

/**
  * @brief  IAP jump to user program.
  * @param  None
  * @retval None
  */
void COMMAND_Jump(void)
{
  /* Software reset */
  NVIC_SystemReset();
}

/**
  * @brief  Programs the internal Flash memory. 
  * @param  None
  * @retval None
  */
void COMMAND_ProgramFlashMemory(void){
	__IO uint32_t programcounter = 0x00;
	uint8_t readflag = TRUE;
	uint16_t BytesRead;
    
	RamAddress = (uint32_t) & RAM_Buf;
	LastPGAddress = HC_APPL_ADDR;
	while (readflag == TRUE){
		f_read (&fileR, RAM_Buf, BUFFER_SIZE, (void *)&BytesRead);
		TmpReadSize = BytesRead;
		if (TmpReadSize < BUFFER_SIZE){
			readflag = FALSE;
		}        
		for (programcounter = TmpReadSize; programcounter != 0; programcounter -= 4){
			TmpProgramCounter = programcounter;
			if (FLASH_If_ProgramWord((LastPGAddress-TmpProgramCounter+TmpReadSize),*(__IO uint32_t*)(RamAddress-programcounter+TmpReadSize))!=FLASH_COMPLETE){
				LCD_DisplayStringLine(LCD_LINE_8, (uint8_t*)"Greska upisa              ");
				LCD_DisplayStringLine(LCD_LINE_9, (uint8_t*)"1. smetnja napajanja");
				LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*)"2. neispravan flash uControlera");
				LCD_DisplayStringLine(LCD_LINE_11, (uint8_t*)"Pokusajte restart");
				Fail_Handler();
			}
		}
		LastPGAddress = LastPGAddress + TmpReadSize;
	}
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
