/**
 ******************************************************************************
 * File Name          : spi_flash.c
 * Date               : 21/08/2016 20:59:16
 * Description        : spi flash driver
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "spi_flash.h"


#if (__SPIFLASH_H__ != FW_BUILD)
    #error "spi flash header version mismatch"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t SPIFlashID;
__IO uint32_t next_address;


/* Private function prototypes -----------------------------------------------*/
static void SPIFLASH_GpioInit(void);
static uint32_t SPIFLASH_ReadId(void);
static void SPIFLASH_WriteEnable(void);
static void SPIFLASH_WriteDisable(void);
static uint8_t SPIFLASH_ReadStatus(void);
static uint8_t SPIFLASH_ReadWrite(uint8_t Byte);


/* Private functions ---------------------------------------------------------*/
void SPIFLASH_Init(void)
{
	SPIFLASH_GpioInit();  
	SPIFlashID = SPIFLASH_ReadId();
	//printf("\r\nSPI-Flash %d ID: \n\r", sizeof(buf));
}


void SPIFLASH_WaitBusy(void)
{
	while(SPIFLASH_ReadStatus() & (0x01));
}


void SPIFLASH_SectorErase(uint32_t Addr, uint8_t sect_size)
{

	SPIFLASH_WriteEnable();								
	SPIFLASH_Select();						
	SPIFLASH_ReadWrite(sect_size);						
	SPIFLASH_ReadWrite((uint8_t)(Addr >> 16));					
	SPIFLASH_ReadWrite((uint8_t)(Addr >> 8));	
	SPIFLASH_ReadWrite(0x00);						
	SPIFLASH_Release();
	SPIFLASH_WaitBusy();						
	SPIFLASH_WriteDisable();			
}


void SPIFLASH_Read(uint32_t Addr, uint8_t *Buf, uint32_t Len)
{
	if (Len == 0)return;

	SPIFLASH_Select();
	SPIFLASH_ReadWrite(SPIFLASH_RDDAT);						
	SPIFLASH_ReadWrite((uint8_t)(Addr >> 16));					
	SPIFLASH_ReadWrite((uint8_t)(Addr >> 8));	
	SPIFLASH_ReadWrite((uint8_t)(Addr & 0xFF));	
	while (Len)
	{
		Len--;
		*Buf++ = SPIFLASH_ReadWrite(0x00);	
	}  
	SPIFLASH_Release();  
}


void SPIFLASH_Write(uint32_t Addr, uint8_t *Buf, uint32_t Len)
{
	uint32_t num;
	if (Len == 0)return;
	
	while(Len)
	{
		num = SPIFLASH_PAGE_SIZE;
		
		if ((Addr % SPIFLASH_SECT_SIZE) == 0)
		{
			SPIFLASH_SectorErase(Addr, SPIFLASH_ER4K);
		}
		
		SPIFLASH_WriteEnable();
		if (Addr & 0xFF) num = Addr % SPIFLASH_PAGE_SIZE;
		if (num > Len) num = Len;
		Len -= num;
		SPIFLASH_Select();
		SPIFLASH_ReadWrite(SPIFLASH_PRPGE);						
		SPIFLASH_ReadWrite((uint8_t)(Addr >> 16));					
		SPIFLASH_ReadWrite((uint8_t)(Addr >> 8));	
		SPIFLASH_ReadWrite((uint8_t)(Addr & 0xFF));
		Addr += num;
		
		while (num--)
		{
		    SPIFLASH_ReadWrite(*Buf++);
		}	
		
		SPIFLASH_Release();
		SPIFLASH_WaitBusy();
		SPIFLASH_WriteDisable();
	}
}


static void SPIFLASH_WriteEnable(void)
{
	SPIFLASH_Select();
	SPIFLASH_ReadWrite(SPIFLASH_WRENA);
	SPIFLASH_Release();
	while (!(SPIFLASH_ReadStatus() & 0x02));
}



static void SPIFLASH_WriteDisable(void)
{
	SPIFLASH_Select();
	SPIFLASH_ReadWrite(SPIFLASH_WRDIS);
	SPIFLASH_Release();
	while (SPIFLASH_ReadStatus() & 0x02);
}


static void SPIFLASH_GpioInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	RCC_AHB1PeriphClockCmd(SPIFLASH_MISO_CLK | SPIFLASH_MOSI_CLK | SPIFLASH_SCK_CLK | SPIFLASH_CS_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(SPIFLASH_RCC_CLK, ENABLE);
						 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_PinAFConfig(SPIFLASH_MISO_GPIO, SPIFLASH_MISO_PINS, SPIFLASH_GPIO_AF);
	GPIO_InitStructure.GPIO_Pin = SPIFLASH_MISO_PIN;
	GPIO_Init(SPIFLASH_MISO_GPIO, &GPIO_InitStructure);

	GPIO_PinAFConfig(SPIFLASH_MOSI_GPIO, SPIFLASH_MOSI_PINS, SPIFLASH_GPIO_AF);
	GPIO_InitStructure.GPIO_Pin = SPIFLASH_MOSI_PIN;
	GPIO_Init(SPIFLASH_MOSI_GPIO, &GPIO_InitStructure);

	GPIO_PinAFConfig(SPIFLASH_SCK_GPIO, SPIFLASH_SCK_PINS, SPIFLASH_GPIO_AF);
	GPIO_InitStructure.GPIO_Pin = SPIFLASH_SCK_PIN;
	GPIO_Init(SPIFLASH_SCK_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = SPIFLASH_CS_PIN;
	GPIO_Init(SPIFLASH_CS_GPIO, &GPIO_InitStructure);  


	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPIFLASH_SPI, &SPI_InitStructure); 
	SPI_Cmd(SPIFLASH_SPI, ENABLE);	
}




static uint8_t SPIFLASH_ReadWrite(uint8_t Byte)
{
 // uint8_t redata;	
	while (SPI_I2S_GetFlagStatus(SPIFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPIFLASH_SPI, Byte);
	while (SPI_I2S_GetFlagStatus(SPIFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPIFLASH_SPI);
}


static uint32_t SPIFLASH_ReadId(void)
{
	uint32_t ID = 0;
	uint8_t i;	

	SPIFLASH_Select();
	SPIFLASH_ReadWrite(SPIFLASH_RDJID);
	for (i = 0; i < 3; i++)
	{
		ID <<= 8;
		ID |= SPIFLASH_ReadWrite(0xFF);
	}  
	SPIFLASH_Release();
	return ID;
}


static uint8_t SPIFLASH_ReadStatus(void)
{
	u8 ReadData;	
	SPIFLASH_Select();									
	SPIFLASH_ReadWrite(0x05);							
	ReadData = SPIFLASH_ReadWrite(0xFF);
	SPIFLASH_Release();				
	return ReadData;	
}




