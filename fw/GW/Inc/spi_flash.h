/**
 ******************************************************************************
 * File Name          : spi_flash.h
 * Date               : 21/08/2016 20:59:16
 * Description        : spi flash driver header file
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__					FW_BUILD	// version


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"


/* Exported types ------------------------------------------------------------*/
#define SPIFLASH_SPI                      SPI5

#define SPIFLASH_GPIO_AF                  GPIO_AF_SPI5
#define SPIFLASH_RCC_CLK                  RCC_APB2Periph_SPI5

#define SPIFLASH_SCK_GPIO                 GPIOF
#define SPIFLASH_SCK_PIN                  GPIO_Pin_7
#define SPIFLASH_SCK_PINS                 GPIO_PinSource7
#define SPIFLASH_SCK_CLK                  RCC_AHB1Periph_GPIOF

#define SPIFLASH_MISO_GPIO                GPIOF
#define SPIFLASH_MISO_PIN                 GPIO_Pin_8
#define SPIFLASH_MISO_PINS                GPIO_PinSource8
#define SPIFLASH_MISO_CLK                 RCC_AHB1Periph_GPIOF

#define SPIFLASH_MOSI_GPIO                GPIOF
#define SPIFLASH_MOSI_PIN                 GPIO_Pin_9
#define SPIFLASH_MOSI_PINS                GPIO_PinSource9
#define SPIFLASH_MOSI_CLK                 RCC_AHB1Periph_GPIOF

#define SPIFLASH_CS_GPIO                  GPIOG
#define SPIFLASH_CS_PIN                   GPIO_Pin_3
#define SPIFLASH_CS_PINS                  GPIO_PinSource3
#define SPIFLASH_CS_CLK                   RCC_AHB1Periph_GPIOG


/* Exported constants --------------------------------------------------------*/
#define ADDR_LIST_START_ADDR    ((uint32_t)0x00000000U)	// 64 kB romm controller address list
#define ADDR_LIST_END_ADDR      ((uint32_t)0x0000FFFFU)

#define SPIFLASH_WRSTA          (0x01)  /* Write Status Register */
#define SPIFLASH_PRPGE          (0x02)  /* Page Program */
#define SPIFLASH_RDDAT          (0x03)  /* Read Data */
#define SPIFLASH_WRDIS          (0x04)  /* Write Disable */
#define SPIFLASH_RDSR1          (0x05)  /* Read Status Register-1 */
#define SPIFLASH_WRENA          (0x06)  /* Write Enable */
#define SPIFLASH_RDFAST         (0x0B)  /* Fast Read */

#define SPIFLASH_RDSR2          (0x35)  /* Read Status Register-2 */
#define SPIFLASH_ER4K           (0x20)  /* Sector Erase: */
#define SPIFLASH_ER32K          (0x52)  /* 32KB Block Erase */
#define SPIFLASH_ER64K          (0xD8)  /* 64KB Block Erase */
#define SPIFLASH_ERALL          (0xC7)  /* Chip Erase */
#define SPIFLASH_RDJID          (0x9F)  /* Read JEDEC ID */



#define SPIFLASH_PAGE_SIZE      0x100U
#define SPIFLASH_SECT_SIZE      0x1000U
#define SPIFLASH_BLOCK_SIZE     0x10000U


/* Exported variables  -------------------------------------------------------*/
extern __IO uint32_t SpiFLashID;
extern __IO uint32_t next_address;


/* Exported macros     -------------------------------------------------------*/
#define SPIFLASH_Select()   GPIO_ResetBits(SPIFLASH_CS_GPIO, SPIFLASH_CS_PIN) 
#define SPIFLASH_Release()	GPIO_SetBits(SPIFLASH_CS_GPIO, SPIFLASH_CS_PIN)


/* Exported functions  -------------------------------------------------------*/
void SPIFLASH_Init(void);
void SPIFLASH_WaitBusy(void);
void SPIFLASH_SectorErase(uint32_t Addr, uint8_t sect_size);
void SPIFLASH_Read(uint32_t addr, uint8_t *buf, uint32_t len);
void SPIFLASH_Write(uint32_t addr, uint8_t *buf, uint32_t len);


#endif /* __SPIFLASH_H__ */
