/**
 ******************************************************************************
 * File Name          : eeprom.c
 * Date               : 28/02/2016 23:16:19
 * Description        : eeprom memory manager modul 
 ******************************************************************************
 *
 *
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "display.h"
#include "logger.h"
#include "eeprom.h"
#include "rc522.h"
#include "rs485.h"
#include "owire.h"
#include "room.h"
#include "main.h"
#include "dio.h"


#if (__EEPROM_H__ != FW_BUILD)
    #error "eeprom header version mismatch"
#endif


/* Variables  ----------------------------------------------------------------*/
static uint8_t spi_buff[I2CEE_PGBSZ+2];
/* Macros   ------------------------------------------------------------------*/
/* Function prototypes    ---------------------------------------------------*/
static uint8_t FLASH_ReadStatusRegister(void);
static uint8_t FLASH_ReadDeviceInfo(void);
static void FLASH_WriteEnable(void);
/* Program code   ------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
uint8_t FLASH_ReadByte(uint32_t address)
{
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_READ;
    spi_buff[1] = address>>16;
    spi_buff[2] = address>>8;
    spi_buff[3] = address; 
    spi_buff[4] = 0;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 4, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
    if (HAL_SPI_Receive(&hspi2, &spi_buff[4], 1, DRV_TOUT)!=HAL_OK) ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
    return (spi_buff[4]);
}
/**
  * @brief
  * @param
  * @retval
  */
uint16_t FLASH_ReadInt(uint32_t address)
{
    uint16_t data;

    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_READ;
    spi_buff[1] = address>>16;
    spi_buff[2] = address>>8;
    spi_buff[3] = address;
    spi_buff[4] = 0;
    spi_buff[5] = 0;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 4, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
    if (HAL_SPI_Receive(&hspi2, &spi_buff[4], 2, DRV_TOUT)!=HAL_OK) ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
    data = (spi_buff[4]<<8)|spi_buff[5];
    return (data);
}
/**
  * @brief
  * @param
  * @retval
  */
void FLASH_ReadPage(uint32_t address, uint8_t *data, uint16_t size)
{
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_READ;
    spi_buff[1] = address>>16;
    spi_buff[2] = address>>8;
    spi_buff[3] = address;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 4, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
    if (HAL_SPI_Receive(&hspi2, data, size, DRV_TOUT) != HAL_OK)    ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}
/**
  * @brief
  * @param
  * @retval
  */
void FLASH_WritePage(uint32_t address, uint8_t *data, uint16_t size)
{
    FLASH_WriteEnable();
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_PAGE_PGM;
    spi_buff[1] = address>>16;
    spi_buff[2] = address>>8;
    spi_buff[3] = address;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 4, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
    if (HAL_SPI_Transmit(&hspi2, data, size, DRV_TOUT) != HAL_OK)   ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}
/**
  * @brief
  * @param
  * @retval
  */
void FLASH_WriteStatusRegister(uint8_t status)
{
	FLASH_WriteEnable();
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_WRITE_STATUS_REG;
    spi_buff[1] = status;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 2, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}
/**
  * @brief
  * @param
  * @retval
  */
void FLASH_UnprotectSector(uint32_t address)
{
    FLASH_WriteEnable();
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_UNPROTECT_SECTOR;
    spi_buff[1] = address>>16;
    spi_buff[2] = address>>8;
    spi_buff[3] = address;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 4, DRV_TOUT) != HAL_OK) ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}
/**
  * @brief
  * @param
  * @retval
  */
uint8_t FLASH_WaitReadyStatus(uint32_t timeout)
{
    uint32_t tickstart = HAL_GetTick();
    
    while((HAL_GetTick() - tickstart) < timeout)
    {
        if((FLASH_ReadStatusRegister() & FLASH_STATUS_BUSY_MASK) == 0)
        {
            return (1);
        }
#ifdef	USE_WATCHDOG
        HAL_IWDG_Refresh(&hiwdg);
#endif
    }
    
    return (0);
}
/**
  * @brief
  * @param
  * @retval
  */
uint8_t FLASH_ReleasePowerDown(void)
{
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_RESUME_POWER_DOWN;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 1, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    return (FLASH_ReadDeviceInfo());
}
/*************************************************************************/
/**         S A V E         V A L U E       T O     E E P R O M         **/
/*************************************************************************/
void EEPROM_Save(uint16_t ee_address, uint8_t* value, uint16_t size)
{
    uint32_t frst_page_bcnt = (uint32_t)(I2CEE_PGBSZ-(ee_address%I2CEE_PGBSZ));   // number of bytes to write to first page 
    uint32_t full_page_bcnt = (uint32_t)(size/I2CEE_PGBSZ);                       // number of full pages to write
    uint32_t last_page_bcnt = (uint32_t)((ee_address+size)%I2CEE_PGBSZ);          // number of bytes to write to last page
    uint32_t wr_address = ee_address;
    /* write to first page till page boundary*/
    if (frst_page_bcnt > size) frst_page_bcnt = size;
    spi_buff[0] = wr_address >> 8;
    spi_buff[1] = wr_address & 0xFF;
    mem_cpy(&spi_buff[2], value, frst_page_bcnt);
    HAL_I2C_Master_Transmit  (&hi2c1, I2CEE_ADD, spi_buff, frst_page_bcnt+2, DRV_TOUT);
    if (HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT) != HAL_OK)    ErrorHandler(EEPROM_FUNC, I2C_DRV);
    size -= frst_page_bcnt;
    if (size == 0) return;
    /*  write full pages */
    value += frst_page_bcnt; // data pointer offset
    wr_address += frst_page_bcnt; // write address offset
    while(full_page_bcnt)
    {
        spi_buff[0] = wr_address >> 8;
        spi_buff[1] = wr_address & 0xFF;
        mem_cpy(&spi_buff[2], value, I2CEE_PGBSZ);
        HAL_I2C_Master_Transmit  (&hi2c1,I2CEE_ADD, spi_buff, I2CEE_PGBSZ+2, DRV_TOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT)!=HAL_OK)  ErrorHandler(EEPROM_FUNC, I2C_DRV);
        wr_address += I2CEE_PGBSZ; // write address offset
        value += I2CEE_PGBSZ; // data pointer offset
        --full_page_bcnt;
        size -= I2CEE_PGBSZ;
        if (size == 0) return;
    }
    /* write to last page */
    spi_buff[0] = wr_address >> 8;
    spi_buff[1] = wr_address & 0xFF;
    mem_cpy(&spi_buff[2], value, last_page_bcnt+2);                                   
    HAL_I2C_Master_Transmit  (&hi2c1, I2CEE_ADD, spi_buff, last_page_bcnt+2, DRV_TOUT);
    if (HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT)!=HAL_OK)      ErrorHandler(EEPROM_FUNC, I2C_DRV);
}
/**
  * @brief
  * @param
  * @retval
  */
void FLASH_Erase(uint32_t address, uint8_t erase_type)
{
    FLASH_WriteEnable();
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = erase_type;
    spi_buff[1] = address>>16;
    spi_buff[2] = address>>8;
    spi_buff[3] = address;
    
    if      ((erase_type == FLASH_CHIP_ERASE) 
    ||       (erase_type == FLASH_CHIP_ERASE_2))
	{
        if (HAL_SPI_Transmit(&hspi2, spi_buff, 1, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);       
    }
	else if ((erase_type == FLASH_4K_BLOCK_ERASE) 
    ||       (erase_type == FLASH_32K_BLOCK_ERASE) 
    ||       (erase_type == FLASH_64K_BLOCK_ERASE))
	{
        if (HAL_SPI_Transmit(&hspi2, spi_buff, 4, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);  
    }
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t FLASH_ReadStatusRegister(void)
{
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_READ_STATUS_REG_1;
    spi_buff[1] = 0;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 1, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
	if (HAL_SPI_Receive(&hspi2, &spi_buff[1], 1, DRV_TOUT)!=HAL_OK) ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
    return (spi_buff[1]);
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t FLASH_ReadDeviceInfo(void)
{
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	spi_buff[0] = FLASH_JEDEC_ID;
    spi_buff[1] = 0;
    spi_buff[2] = 0;
    spi_buff[3] = 0;
	if (HAL_SPI_Transmit(&hspi2, spi_buff, 1, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
	if (HAL_SPI_Receive(&hspi2, &spi_buff[1], 3, DRV_TOUT)!=HAL_OK) ErrorHandler(EEPROM_FUNC, SPI_DRV);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
	if (((spi_buff[1]<<16)|(spi_buff[2]<<8)|spi_buff[3]) == FLASH_MANUFACTURER_WINBOND) return(1);
	else return(0);
}
/**
  * @brief
  * @param
  * @retval
  */
static void FLASH_WriteEnable(void)
{
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    spi_buff[0] = FLASH_WRITE_ENABLE;
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 1, DRV_TOUT) != HAL_OK)  ErrorHandler(EEPROM_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
