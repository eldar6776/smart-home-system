
#ifndef __W25QXX_H__
#define __W25QXX_H__

#include "stm32f4xx.h"


#define W25QXX_FW_START_ADDRESS			0x00000000
#define W25QXX_FW_END_ADDRESS			0x0000ffff
#define W25QXX_IMAGE_1_START_ADDRESS	0x00010000
#define W25QXX_IMAGE_1_END_ADDRESS		0x0003ffff
#define W25QXX_IMAGE_2_START_ADDRESS	0x00040000
#define W25QXX_IMAGE_2_END_ADDRESS		0x0006ffff
#define W25QXX_IMAGE_3_START_ADDRESS	0x00070000
#define W25QXX_IMAGE_3_END_ADDRESS		0x0009ffff
#define W25QXX_IMAGE_4_START_ADDRESS	0x000a0000
#define W25QXX_IMAGE_4_END_ADDRESS		0x000cffff
#define W25QXX_IMAGE_5_START_ADDRESS	0x000d0000
#define W25QXX_IMAGE_5_END_ADDRESS		0x000fffff
#define W25QXX_IMAGE_6_START_ADDRESS	0x00100000
#define W25QXX_IMAGE_6_END_ADDRESS		0x0012ffff
#define W25QXX_IMAGE_7_START_ADDRESS	0x00130000
#define W25QXX_IMAGE_7_END_ADDRESS		0x0015ffff
#define W25QXX_IMAGE_8_START_ADDRESS	0x00160000
#define W25QXX_IMAGE_8_END_ADDRESS		0x0018ffff
#define W25QXX_IMAGE_9_START_ADDRESS	0x00190000
#define W25QXX_IMAGE_9_END_ADDRESS		0x001bffff
#define W25QXX_IMAGE_10_START_ADDRESS	0x001c0000
#define W25QXX_IMAGE_10_END_ADDRESS		0x001effff
#define W25QXX_IMAGE_11_START_ADDRESS	0x001f0000
#define W25QXX_IMAGE_11_END_ADDRESS		0x0021ffff
#define W25QXX_IMAGE_12_START_ADDRESS	0x00220000
#define W25QXX_IMAGE_12_END_ADDRESS		0x0024ffff
#define W25QXX_IMAGE_13_START_ADDRESS	0x00250000
#define W25QXX_IMAGE_13_END_ADDRESS		0x0027ffff
#define W25QXX_IMAGE_14_START_ADDRESS	0x00280000
#define W25QXX_IMAGE_14_END_ADDRESS		0x002affff
#define W25QXX_IMAGE_15_START_ADDRESS	0x002b0000
#define W25QXX_IMAGE_15_END_ADDRESS		0x002dffff
#define W25QXX_IMAGE_16_START_ADDRESS	0x002e0000
#define W25QXX_IMAGE_16_END_ADDRESS		0x0030ffff
#define W25QXX_IMAGE_17_START_ADDRESS	0x00310000
#define W25QXX_IMAGE_17_END_ADDRESS		0x0033ffff
#define W25QXX_IMAGE_18_START_ADDRESS	0x00340000
#define W25QXX_IMAGE_18_END_ADDRESS		0x0036ffff
#define W25QXX_IMAGE_19_START_ADDRESS	0x00370000
#define W25QXX_IMAGE_19_END_ADDRESS		0x0039ffff


#define W25QXX_SPI              SPI5

#define W25QXX_GPIO_AF          GPIO_AF_SPI5
#define W25QXX_RCC_CLK          RCC_APB2Periph_SPI5

#define W25QXX_SCK_GPIO         GPIOF
#define W25QXX_SCK_PIN          GPIO_Pin_7
#define W25QXX_SCK_PINS         GPIO_PinSource7
#define W25QXX_SCK_CLK          RCC_AHB1Periph_GPIOF

#define W25QXX_MISO_GPIO        GPIOF
#define W25QXX_MISO_PIN         GPIO_Pin_8
#define W25QXX_MISO_PINS        GPIO_PinSource8
#define W25QXX_MISO_CLK         RCC_AHB1Periph_GPIOF

#define W25QXX_MOSI_GPIO        GPIOF
#define W25QXX_MOSI_PIN         GPIO_Pin_9
#define W25QXX_MOSI_PINS        GPIO_PinSource9
#define W25QXX_MOSI_CLK         RCC_AHB1Periph_GPIOF


#define W25QXX_CS_GPIO          GPIOG
#define W25QXX_CS_PIN           GPIO_Pin_3
#define W25QXX_CS_PINS          GPIO_PinSource3
#define W25QXX_CS_CLK           RCC_AHB1Periph_GPIOG

#define W25QXX_ChipSelect()		GPIO_ResetBits(W25QXX_CS_GPIO, W25QXX_CS_PIN) 
#define W25QXX_ChipDeselect()	GPIO_SetBits(W25QXX_CS_GPIO, W25QXX_CS_PIN)

#define W25QXX_CMD_WRSR         (0x01)  /* Write Status Register */
#define W25QXX_CMD_PP           (0x02)  /* Page Program */
#define W25QXX_CMD_READ         (0x03)  /* Read Data */
#define W25QXX_CMD_WRDI         (0x04)  /* Write Disable */
#define W25QXX_CMD_RDSR1        (0x05)  /* Read Status Register-1 */
#define W25QXX_CMD_WREN         (0x06)  /* Write Enable */
#define W25QXX_CMD_FAST_REA     (0x0B)  /* Fast Read */
#define W25QXX_CMD_ERASE_4K     (0x20)  /* Sector Erase: */
#define W25QXX_CMD_RDSR2        (0x35)  /* Read Status Register-2 */
#define W25QXX_CMD_ERASE_32K    (0x52)  /* 32KB Block Erase */
#define W25QXX_CMD_JEDEC_ID     (0x9F)  /* Read JEDEC ID */
#define W25QXX_CMD_ERASE_full   (0xC7)  /* Chip Erase */
#define W25QXX_CMD_ERASE_64K    (0xD8)  /* 64KB Block Erase */

#define W25QXX_PAGE_SIZE        256
#define W25QXX_SECTOR_SIZE      4096


extern volatile uint32_t SpiFLashID;
extern volatile uint32_t next_address;


void W25Qxx_Init(void);
void W25Qxx_Read(uint32_t addr, uint8_t *buf, uint32_t len);
void W25Qxx_Write(uint32_t addr, uint8_t *buf, uint32_t len);
void W25Qxx_SectorErase(uint32_t Addr, uint8_t size);
void W25Qxx_WaitBusy(void);

#endif /* __W25QXX_H__ */
