/**
 ******************************************************************************
 * File Name          : nrf24l01.c
 * Date               : 28/02/2016 23:16:19
 * Description        : Hotel Room Thermostat nrf24l01 radio modul services
 ******************************************************************************
 *
 *
 ******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/
#include "png.h"
#include "main.h"
#include "rs485.h"
#include "logger.h"
#include "display.h"
#include "onewire.h"
#include "nrf24l01.h"
#include "thermostat.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"


#if (__RF24_H__ != FW_BUILD)
    #error "radio header version mismatch"
#endif



/* Defines    ----------------------------------------------------------------*/
/* NRF24L01+ registers*/
#define RF24_REG_CONFIG			0x00	//Configuration Register
#define RF24_REG_EN_AA			0x01	//Enable ҁuto AcknowledgmentҠFunction
#define RF24_REG_EN_RXADDR		0x02	//Enabled RX Addresses
#define RF24_REG_SETUP_AW		0x03	//Setup of Address Widths (common for all data pipes)
#define RF24_REG_SETUP_RETR		0x04	//Setup of Automatic Retransmission
#define RF24_REG_RF_CH			0x05	//RF Channel
#define RF24_REG_RF_SETUP		0x06	//RF Setup Register	
#define RF24_REG_STATUS			0x07	//Status Register
#define RF24_REG_OBSERVE_TX		0x08	//Transmit observe register
#define RF24_REG_RPD			0x09	
#define RF24_REG_RX_ADDR_P0		0x0A	//Receive address data pipe 0. 5 Bytes maximum length.
#define RF24_REG_RX_ADDR_P1		0x0B	//Receive address data pipe 1. 5 Bytes maximum length.
#define RF24_REG_RX_ADDR_P2		0x0C	//Receive address data pipe 2. Only LSB
#define RF24_REG_RX_ADDR_P3		0x0D	//Receive address data pipe 3. Only LSB
#define RF24_REG_RX_ADDR_P4		0x0E	//Receive address data pipe 4. Only LSB
#define RF24_REG_RX_ADDR_P5		0x0F	//Receive address data pipe 5. Only LSB
#define RF24_REG_TX_ADDR		0x10	//Transmit address. Used for a PTX device only
#define RF24_REG_RX_PW_P0		0x11	
#define RF24_REG_RX_PW_P1		0x12	
#define RF24_REG_RX_PW_P2		0x13	
#define RF24_REG_RX_PW_P3		0x14	
#define RF24_REG_RX_PW_P4		0x15	
#define RF24_REG_RX_PW_P5		0x16	
#define RF24_REG_FIFO_STATUS	0x17	//FIFO Status Register
#define RF24_REG_DYNPD			0x1C	//Enable dynamic payload length
#define RF24_REG_FEATURE		0x1D

/* Registers default values */
#define RF24_REG_DEF_CONFIG			0x08	
#define RF24_REG_DEF_EN_AA			0x3F	
#define RF24_REG_DEF_EN_RXADDR		0x03	
#define RF24_REG_DEF_SETUP_AW		0x03	
#define RF24_REG_DEF_SETUP_RETR		0x03	
#define RF24_REG_DEF_RF_CH			0x02	
#define RF24_REG_DEF_RF_SETUP		0x0E	
#define RF24_REG_DEF_STATUS			0x0E	
#define RF24_REG_DEF_OBSERVE_TX		0x00	
#define RF24_REG_DEF_RPD			0x00
#define RF24_REG_DEF_RX_ADDR_P0_0	0xE7
#define RF24_REG_DEF_RX_ADDR_P0_1	0xE7
#define RF24_REG_DEF_RX_ADDR_P0_2	0xE7
#define RF24_REG_DEF_RX_ADDR_P0_3	0xE7
#define RF24_REG_DEF_RX_ADDR_P0_4	0xE7
#define RF24_REG_DEF_RX_ADDR_P1_0	0xC2
#define RF24_REG_DEF_RX_ADDR_P1_1	0xC2
#define RF24_REG_DEF_RX_ADDR_P1_2	0xC2
#define RF24_REG_DEF_RX_ADDR_P1_3	0xC2
#define RF24_REG_DEF_RX_ADDR_P1_4	0xC2
#define RF24_REG_DEF_RX_ADDR_P2		0xC3	
#define RF24_REG_DEF_RX_ADDR_P3		0xC4	
#define RF24_REG_DEF_RX_ADDR_P4		0xC5
#define RF24_REG_DEF_RX_ADDR_P5		0xC6
#define RF24_REG_DEF_TX_ADDR_0		0xE7
#define RF24_REG_DEF_TX_ADDR_1		0xE7
#define RF24_REG_DEF_TX_ADDR_2		0xE7
#define RF24_REG_DEF_TX_ADDR_3		0xE7
#define RF24_REG_DEF_TX_ADDR_4		0xE7
#define RF24_REG_DEF_RX_PW_P0		0x00
#define RF24_REG_DEF_RX_PW_P1		0x00
#define RF24_REG_DEF_RX_PW_P2		0x00
#define RF24_REG_DEF_RX_PW_P3		0x00
#define RF24_REG_DEF_RX_PW_P4		0x00
#define RF24_REG_DEF_RX_PW_P5		0x00
#define RF24_REG_DEF_FIFO_STATUS	0x11
#define RF24_REG_DEF_DYNPD			0x00
#define RF24_REG_DEF_FEATURE		0x00

/* Configuration register*/
#define RF24_MASK_RX_DR		6
#define RF24_MASK_TX_DS		5
#define RF24_MASK_MAX_RT	4
#define RF24_EN_CRC			3
#define RF24_CRCO			2
#define RF24_PWR_UP			1
#define RF24_PRIM_RX		0

/* Enable auto acknowledgment*/
#define RF24_ENAA_P5		5
#define RF24_ENAA_P4		4
#define RF24_ENAA_P3		3
#define RF24_ENAA_P2		2
#define RF24_ENAA_P1		1
#define RF24_ENAA_P0		0

/* Enable rx addresses */
#define RF24_ERX_P5			5
#define RF24_ERX_P4			4
#define RF24_ERX_P3			3
#define RF24_ERX_P2			2
#define RF24_ERX_P1			1
#define RF24_ERX_P0			0

/* Setup of address width */
#define RF24_AW				0 //2 bits

/* Setup of auto re-transmission*/
#define RF24_ARD			4 //4 bits
#define RF24_ARC			0 //4 bits

/* RF setup register*/
#define RF24_PLL_LOCK		4
#define RF24_RF_DR_LOW		5
#define RF24_RF_DR_HIGH		3
#define RF24_RF_DR			3
#define RF24_RF_PWR			1 //2 bits   

/* General status register */
#define RF24_RX_DR			6
#define RF24_TX_DS			5
#define RF24_MAX_RT			4
#define RF24_RX_P_NO		1 //3 bits
#define RF24_TX_FULL		0

/* Transmit observe register */
#define RF24_PLOS_CNT		4 //4 bits
#define RF24_ARC_CNT		0 //4 bits

/* FIFO status*/
#define RF24_TX_REUSE		6
#define RF24_FIFO_FULL		5
#define RF24_TX_EMPTY		4
#define RF24_RX_FULL		1
#define RF24_RX_EMPTY		0

//Dynamic length
#define RF24_DPL_P0			0
#define RF24_DPL_P1			1
#define RF24_DPL_P2			2
#define RF24_DPL_P3			3
#define RF24_DPL_P4			4
#define RF24_DPL_P5			5

/* Transmitter power*/
#define RF24_M18DBM			0 //-18 dBm
#define RF24_M12DBM			1 //-12 dBm
#define RF24_M6DBM			2 //-6 dBm
#define RF24_0DBM			3 //0 dBm

/* Data rates */
#define RF24_2MBPS			0
#define RF24_1MBPS			1
#define RF24_250KBPS		2

/* Configuration */
#define RF24_CONFIG			((1 << RF24_EN_CRC) | (0 << RF24_CRCO))

/* Instruction Mnemonics */
#define RF24_REG_MASK				    0x1F

#define RF24_READ_REGISTER_MASK(reg)	(0x00 | (RF24_REG_MASK & reg)) //Last 5 bits will indicate reg. address
#define RF24_WRITE_REGISTER_MASK(reg)	(0x20 | (RF24_REG_MASK & reg)) //Last 5 bits will indicate reg. address
#define RF24_R_RX_PAYLOAD_MASK			0x61
#define RF24_W_TX_PAYLOAD_MASK			0xA0
#define RF24_FLUSH_TX_MASK				0xE1
#define RF24_FLUSH_RX_MASK				0xE2
#define RF24_REUSE_TX_PL_MASK			0xE3
#define RF24_ACTIVATE_MASK				0x50 
#define RF24_R_RX_PL_WID_MASK			0x60
#define RF24_NOP_MASK					0xFF
#define RF24_TRANSMISSON_OK 			0x0U
#define RF24_MESSAGE_LOST   			0x1U


/* Types  --------------------------------------------------------------------*/
typedef struct 
{
	uint8_t PayloadSize;            // Payload size
	uint8_t Channel;                // Channel selected
	RF24_OutputPower_t OutPwr;	// Output power
	RF24_DataRate_t DataRate;	// Data rate
    
} RF24_t;


/* Variables  ----------------------------------------------------------------*/
__IO uint32_t rf24_flag;
__IO uint32_t rf24_timer;
__IO  uint8_t rf24_rxbuf[RF24_BSIZE]; 
__IO  uint8_t rf24_txbuf[RF24_BSIZE];
static RF24_t RF24_Struct;


/* Macros     ----------------------------------------------------------------*/
#define RF24_CSN_LOW				(HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET))
#define RF24_CSN_HIGH 				(HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET))
#define RF24_CE_LOW					(HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3,  GPIO_PIN_RESET))
#define RF24_CE_HIGH 				(HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3,  GPIO_PIN_SET))
#define RF24_CHECK_BIT(reg, bit)  	(reg & (1 << bit))


/* Private prototypes    -----------------------------------------------------*/
static void RF24_WriteBit(uint8_t reg, uint8_t bit, uint8_t value);
static uint8_t RF24_ReadBit(uint8_t reg, uint8_t bit);
static uint8_t RF24_ReadRegister(uint8_t reg);
static void RF24_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t count);
static void RF24_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t count);
static void RF24_SoftwareReset(void);
static uint8_t RF24_RxFifoEmpty(void);
static void RF24_Flush(uint8_t mask);
static void RF24_SetMyAddress(uint8_t* adr);
static void RF24_SetTxAddress(uint8_t* adr);
static uint8_t RF24_GetRetransmissionsCount(void);
static void RF24_PowerUpTx(void);
static void RF24_PowerUpRx(void);
static void RF24_PowerDown(void);
static void RF24_Transmit(uint8_t *data);
static RF24_Transmit_Status_t RF24_GetTransmissionStatus(void);
static uint8_t RF24_DataReady(void);
static void RF24_GetData(uint8_t *data);
static void RF24_SetChannel(uint8_t channel);
static void RF24_SetRF(RF24_DataRate_t DataRate, RF24_OutputPower_t OutPwr);
static uint8_t RF24_GetStatus(void);
static uint8_t RF24_Read_Interrupts(RF24_IRQ_t* IRQ);
static void RF24_Clear_Interrupts(void);
static void RF24_WriteRegister(uint8_t reg, uint8_t value);

/* Program code   ------------------------------------------------------------*/
/**
 * @brief  Initializes NRF24L01+ module
 * @param  channel: channel you will use for communication, from 0 to 125 eg. working frequency from 2.4 to 2.525 GHz
 * @param  payload_size: maximum data to be sent in one packet from one NRF to another.
 * @note   Maximal payload size is 32bytes
 * @retval 1
 */
void RF24_Init(void)
{	
    uint8_t channel;
	uint8_t payload_size;
    /* Max payload is 32bytes */
	if (payload_size > 32) {
		payload_size = 32;
	}
	/* Fill structure */
	RF24_Struct.Channel = !channel; /* Set channel to some different value for RF24_SetChannel() function */
	RF24_Struct.PayloadSize = payload_size;
	RF24_Struct.OutPwr = RF24_OutputPower_0dBm;
	RF24_Struct.DataRate = RF24_DataRate_2M;
	/* Reset nRF24L01+ to power on registers values */
	RF24_SoftwareReset();
	/* Channel select */
	RF24_SetChannel(channel);
	/* Set pipeline to max possible 32 bytes */
	RF24_WriteRegister(RF24_REG_RX_PW_P0, RF24_Struct.PayloadSize); // Auto-ACK pipe
	RF24_WriteRegister(RF24_REG_RX_PW_P1, RF24_Struct.PayloadSize); // Data payload pipe
	RF24_WriteRegister(RF24_REG_RX_PW_P2, RF24_Struct.PayloadSize);
	RF24_WriteRegister(RF24_REG_RX_PW_P3, RF24_Struct.PayloadSize);
	RF24_WriteRegister(RF24_REG_RX_PW_P4, RF24_Struct.PayloadSize);
	RF24_WriteRegister(RF24_REG_RX_PW_P5, RF24_Struct.PayloadSize);
	/* Set RF settings (2mbps, output power) */
	RF24_SetRF(RF24_Struct.DataRate, RF24_Struct.OutPwr);
	/* Config register */
	RF24_WriteRegister(RF24_REG_CONFIG, RF24_CONFIG);
	/* Enable auto-acknowledgment for all pipes */
	RF24_WriteRegister(RF24_REG_EN_AA, 0x3F);
	/* Enable RX addresses */
	RF24_WriteRegister(RF24_REG_EN_RXADDR, 0x3F);
	/* Auto retransmit delay: 1000 (4x250) us and Up to 15 retransmit trials */
	RF24_WriteRegister(RF24_REG_SETUP_RETR, 0x4F);
	/* Dynamic length configurations: No dynamic length */
	RF24_WriteRegister(RF24_REG_DYNPD, (0 << RF24_DPL_P0) 
                                             | (0 << RF24_DPL_P1) 
                                             | (0 << RF24_DPL_P2) 
                                             | (0 << RF24_DPL_P3) 
                                             | (0 << RF24_DPL_P4) 
                                             | (0 << RF24_DPL_P5));
	RF24_Flush(RF24_FLUSH_TX_MASK);/* Clear FIFOs */
	RF24_Flush(RF24_FLUSH_RX_MASK);
//	RF24_CLEAR_INTERRUPTS;
	RF24_Clear_Interrupts();/* Clear interrupts */
	RF24_PowerUpRx();/* Go to RX mode */
	
}

/**
 * @brief  
 * @param 
 * @retval 
 */
void RF24_Service(void)
{
    
}
/**
 * @brief  Sets own address. This is used for settings own id when communication with other modules
 * @note   "Own" address of one device must be the same as "TX" address of other device (and vice versa),
 *         if you want to get successful communication
 * @param  *adr: Pointer to 5-bytes length array with address
 * @retval None
 */
static void RF24_SetMyAddress(uint8_t *adr) 
{
	RF24_CE_LOW;
	RF24_WriteRegisterMulti(RF24_REG_RX_ADDR_P1, adr, 5);
	RF24_CE_HIGH;
}


/**
 * @brief  Sets address you will communicate with
 * @note   "Own" address of one device must be the same as "TX" address of other device (and vice versa),
 *         if you want to get successful communication
 * @param  *adr: Pointer to 5-bytes length array with address
 * @retval None
 */
static void RF24_SetTxAddress(uint8_t *adr) 
{
	RF24_WriteRegisterMulti(RF24_REG_RX_ADDR_P0, adr, 5);
	RF24_WriteRegisterMulti(RF24_REG_TX_ADDR, adr, 5);
}


/**
 * @brief  
 * @param 
 * @retval 
 */
static void RF24_WriteBit(uint8_t reg, uint8_t bit, uint8_t value) 
{
	uint8_t tmp;
	
	tmp = RF24_ReadRegister(reg);/* Read register */
	if (value) {/* Make operation */
		tmp |= 1 << bit;
	} else {
		tmp &= ~(1 << bit);
	}
	RF24_WriteRegister(reg, tmp);/* Write back */
}


/**
 * @brief  
 * @param 
 * @retval 
 */
static uint8_t RF24_ReadBit(uint8_t reg, uint8_t bit) 
{
	uint8_t tmp;
	tmp = RF24_ReadRegister(reg);
	if (!RF24_CHECK_BIT(tmp, bit)) return 0;
	return 1;
}
/**
 * @brief  
 * @param 
 * @retval 
 */
static uint8_t RF24_ReadRegister(uint8_t reg) 
{
	uint8_t value;
	
	value =  RF24_READ_REGISTER_MASK(reg);
//	RF24_CSN_LOW;
	HAL_SPI_Transmit(&hspi2, &value, 1, 10);
	HAL_SPI_Receive(&hspi2, &value, 1, 10);
//	RF24_CSN_HIGH;
	return (value);
}


/**
 * @brief  
 * @param 
 * @retval 
 */
static void RF24_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t count) 
{
	uint8_t rd_reg;
	
	rd_reg = RF24_READ_REGISTER_MASK(reg);
//	RF24_CSN_LOW;
	HAL_SPI_Transmit(&hspi2, &rd_reg, 1, 10);
	HAL_SPI_Receive(&hspi2, data, count, 10);
//	RF24_CSN_HIGH;
}


/**
 * @brief  
 * @param 
 * @retval 
 */
static void RF24_WriteRegister(uint8_t reg, uint8_t value) 
{
	uint8_t wr[2];
	
	wr[0] = RF24_WRITE_REGISTER_MASK(reg);
	wr[1] = value;
//	RF24_CSN_LOW;
//	HAL_SPI_Transmit(&hspi2, wr, 2, 10);
//	RF24_CSN_HIGH;
}


/**
 * @brief  
 * @param 
 * @retval 
 */
static void RF24_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t count) 
{
	uint8_t wr_reg;
	
	wr_reg = RF24_WRITE_REGISTER_MASK(reg);
//	RF24_CSN_LOW;
	HAL_SPI_Transmit(&hspi2, &wr_reg, 1, 10);
	HAL_SPI_Transmit(&hspi2, data, count, 10);
//	RF24_CSN_HIGH;
}
/**
 * @brief  Sets NRF24L01+ to TX mode
 * @note   In this mode is NRF able to send data to another NRF module
 * @param  None
 * @retval None
 */
static void RF24_PowerUpTx(void) 
{
//	RF24_CLEAR_INTERRUPTS;
	RF24_Clear_Interrupts();
	RF24_WriteRegister (RF24_REG_CONFIG, 
                            RF24_CONFIG 
                    | (0 << RF24_PRIM_RX) 
                    | (1 << RF24_PWR_UP));
}


/**
 * @brief  Sets NRF24L01+ to RX mode
 * @note   In this mode is NRF able to receive data from another NRF module.
 *         This is default mode and should be used all the time, except when sending data
 * @param  None
 * @retval None
 */
static void RF24_PowerUpRx(void) 
{
	RF24_CE_LOW;/* Disable RX/TX mode */
	RF24_Flush(RF24_FLUSH_RX_MASK);/* Clear RX buffer */
//	RF24_CLEAR_INTERRUPTS;
	RF24_Clear_Interrupts();/* Clear interrupts */
	
	RF24_WriteRegister(RF24_REG_CONFIG, /* Setup RX mode */
                           RF24_CONFIG | 
                      1 << RF24_PWR_UP | 
                      1 << RF24_PRIM_RX);
	RF24_CE_HIGH;/* Start listening */
}

/**
 * @brief  Sets NRF24L01+ to power down mode
 * @note   In power down mode, you are not able to transmit/receive data.
 *         You can wake up device using @ref RF24_PowerUpTx() or @ref RF24_PowerUpRx() functions
 * @param  None
 * @retval None
 */
static void RF24_PowerDown(void) 
{
	RF24_CE_LOW;
	RF24_WriteBit(RF24_REG_CONFIG, RF24_PWR_UP, 0);
}

/**
 * @brief  Transmits data with NRF24L01+ to another NRF module
 * @param  *data: Pointer to 8-bit array with data.
 *         Maximum length of array can be the same as "payload_size" parameter on initialization
 * @retval None
 */
static void RF24_Transmit(uint8_t *data) 
{
	uint8_t mask;
	uint8_t count = RF24_Struct.PayloadSize;
	RF24_CE_LOW;/* Chip enable put to low, disable it */
	RF24_PowerUpTx();/* Go to power up tx mode */
	RF24_Flush(RF24_FLUSH_TX_MASK);/* Clear TX FIFO from NRF24L01+ */
//	RF24_CSN_LOW;
	mask = RF24_W_TX_PAYLOAD_MASK;/* Send payload to nRF24L01+ */
	HAL_SPI_Transmit(&hspi2, &mask, 1, 10);/* Send write payload command */
	HAL_SPI_Transmit(&hspi2, data, count, 10);/* Fill payload with data*/
//	RF24_CSN_HIGH;/* Disable SPI */
	RF24_CE_HIGH;/* Send data! */
}

/**
 * @brief  Gets data from NRF24L01+
 * @param  *data: Pointer to 8-bits array where data from NRF will be saved
 * @retval None
 */
static void RF24_GetData(uint8_t* data) 
{
	uint8_t rx_mask;
	
//	RF24_CSN_LOW;/* Pull down chip select */
	rx_mask = RF24_R_RX_PAYLOAD_MASK;/* Send read payload command*/
	HAL_SPI_Transmit(&hspi2, &rx_mask, 1, 10);
	HAL_SPI_Receive(&hspi2, data, RF24_Struct.PayloadSize, 10);/* Read payload */
//	RF24_CSN_HIGH;/* Pull up chip select */
    /* Reset status register, clear RX_DR interrupt flag */
    RF24_WriteRegister(RF24_REG_STATUS, (1 << RF24_RX_DR));
}
	


/**
 * @brief  Checks if data is ready to be read from NRF24L01+
 * @param  None
 * @retval Data ready status:
 *            - 0: No data available for receive in bufferReturns
 *            - > 0: Data is ready to be collected
 */
static uint8_t RF24_DataReady(void) 
{
	uint8_t status = RF24_GetStatus();
	if (RF24_CHECK_BIT(status, RF24_RX_DR)) return 1;
	return !RF24_RxFifoEmpty();
}


/**
 * @brief  
 * @param 
 * @retval 
 */
static uint8_t RF24_RxFifoEmpty(void) 
{
	uint8_t reg = RF24_ReadRegister(RF24_REG_FIFO_STATUS);
	return RF24_CHECK_BIT(reg, RF24_RX_EMPTY);
}
/**
 * @brief  
 * @param 
 * @retval 
 */
static uint8_t RF24_GetStatus(void) 
{
	uint8_t status, nopb;
	nopb = RF24_NOP_MASK;
//	RF24_CSN_LOW;/* First received byte is always status register */
	HAL_SPI_TransmitReceive(&hspi2, &nopb, &status, 1, 10);
//	RF24_CSN_HIGH;/* Pull up chip select */
	return status;
}
/**
 * @brief  Gets transmissions status
 * @param  None
 * @retval Transmission status. Return is based on @ref RF24_Transmit_Status_t enumeration
 */
static RF24_Transmit_Status_t RF24_GetTransmissionStatus(void) 
{
	uint8_t status = RF24_GetStatus();
	
	if (RF24_CHECK_BIT(status, RF24_TX_DS)) 
	{
		return RF24_Transmit_Status_Ok;/* Successfully sent */
	} 
	else if (RF24_CHECK_BIT(status, RF24_MAX_RT)) 
	{
		return RF24_Transmit_Status_Lost;/* Message lost */
	}
	return RF24_Transmit_Status_Sending;/* Still sending */
}


/**
 * @brief  
 * @param 
 * @retval 
 */
static void RF24_SoftwareReset(void) 
{
	uint8_t data[5];
	
	RF24_WriteRegister(RF24_REG_CONFIG, 	RF24_REG_DEF_CONFIG);
	RF24_WriteRegister(RF24_REG_EN_AA,		RF24_REG_DEF_EN_AA);
	RF24_WriteRegister(RF24_REG_EN_RXADDR, 	RF24_REG_DEF_EN_RXADDR);
	RF24_WriteRegister(RF24_REG_SETUP_AW, 	RF24_REG_DEF_SETUP_AW);
	RF24_WriteRegister(RF24_REG_SETUP_RETR, RF24_REG_DEF_SETUP_RETR);
	RF24_WriteRegister(RF24_REG_RF_CH, 		RF24_REG_DEF_RF_CH);
	RF24_WriteRegister(RF24_REG_RF_SETUP, 	RF24_REG_DEF_RF_SETUP);
	RF24_WriteRegister(RF24_REG_STATUS, 	RF24_REG_DEF_STATUS);
	RF24_WriteRegister(RF24_REG_OBSERVE_TX, RF24_REG_DEF_OBSERVE_TX);
	RF24_WriteRegister(RF24_REG_RPD, 		RF24_REG_DEF_RPD);
	
	//P0
	data[0] = RF24_REG_DEF_RX_ADDR_P0_0;
	data[1] = RF24_REG_DEF_RX_ADDR_P0_1;
	data[2] = RF24_REG_DEF_RX_ADDR_P0_2;
	data[3] = RF24_REG_DEF_RX_ADDR_P0_3;
	data[4] = RF24_REG_DEF_RX_ADDR_P0_4;
	RF24_WriteRegisterMulti(RF24_REG_RX_ADDR_P0, data, 5);
	
	//P1
	data[0] = RF24_REG_DEF_RX_ADDR_P1_0;
	data[1] = RF24_REG_DEF_RX_ADDR_P1_1;
	data[2] = RF24_REG_DEF_RX_ADDR_P1_2;
	data[3] = RF24_REG_DEF_RX_ADDR_P1_3;
	data[4] = RF24_REG_DEF_RX_ADDR_P1_4;
	RF24_WriteRegisterMulti(RF24_REG_RX_ADDR_P1, data, 5);
	
	RF24_WriteRegister(RF24_REG_RX_ADDR_P2, RF24_REG_DEF_RX_ADDR_P2);
	RF24_WriteRegister(RF24_REG_RX_ADDR_P3, RF24_REG_DEF_RX_ADDR_P3);
	RF24_WriteRegister(RF24_REG_RX_ADDR_P4, RF24_REG_DEF_RX_ADDR_P4);
	RF24_WriteRegister(RF24_REG_RX_ADDR_P5, RF24_REG_DEF_RX_ADDR_P5);
	
	//TX
	data[0] = RF24_REG_DEF_TX_ADDR_0;
	data[1] = RF24_REG_DEF_TX_ADDR_1;
	data[2] = RF24_REG_DEF_TX_ADDR_2;
	data[3] = RF24_REG_DEF_TX_ADDR_3;
	data[4] = RF24_REG_DEF_TX_ADDR_4;
	RF24_WriteRegisterMulti(RF24_REG_TX_ADDR, data, 5);
	
	RF24_WriteRegister(RF24_REG_RX_PW_P0, 	RF24_REG_DEF_RX_PW_P0);
	RF24_WriteRegister(RF24_REG_RX_PW_P1, 	RF24_REG_DEF_RX_PW_P1);
	RF24_WriteRegister(RF24_REG_RX_PW_P2, 	RF24_REG_DEF_RX_PW_P2);
	RF24_WriteRegister(RF24_REG_RX_PW_P3, 	RF24_REG_DEF_RX_PW_P3);
	RF24_WriteRegister(RF24_REG_RX_PW_P4, 	RF24_REG_DEF_RX_PW_P4);
	RF24_WriteRegister(RF24_REG_RX_PW_P5, 	RF24_REG_DEF_RX_PW_P5);
	RF24_WriteRegister(RF24_REG_FIFO_STATUS, RF24_REG_DEF_FIFO_STATUS);
	RF24_WriteRegister(RF24_REG_DYNPD, 		RF24_REG_DEF_DYNPD);
	RF24_WriteRegister(RF24_REG_FEATURE, 	RF24_REG_DEF_FEATURE);
}
/**
 * @brief  Gets number of retransmissions needed in last transmission
 * @param  None
 * @retval Number of retransmissions, between 0 and 15.
 */
static uint8_t RF24_GetRetransmissionsCount(void) 
{
	return RF24_ReadRegister(RF24_REG_OBSERVE_TX) & 0x0F;/* Low 4 bits */
}
/**
 * @brief  Sets working channel
 * @note   Channel value is just an offset in units MHz from 2.4GHz
 *         For example, if you select channel 65, then operation frequency will be set to 2.465GHz.
 * @param  channel: RF channel where device will operate
 * @retval None 
 */
static void RF24_SetChannel(uint8_t channel)
{
	if (channel <= 125 && channel != RF24_Struct.Channel) 
	{
		RF24_Struct.Channel = channel;/* Store new channel setting */
		RF24_WriteRegister(RF24_REG_RF_CH, channel);/* Write channel */
	}
}


/**
 * @brief  Sets RF parameters for NRF24L01+
 * @param  DataRate: Data rate selection for NRF module. This parameter can be a value of @ref RF24_DataRate_t enumeration
 * @param  OutPwr: Output power selection for NRF module. This parameter can be a value of @ref RF24_OutputPower_t enumeration
 * @retval None
 */
static void RF24_SetRF(RF24_DataRate_t DataRate, RF24_OutputPower_t OutPwr) 
{
	uint8_t tmp = 0;
	RF24_Struct.DataRate = DataRate;
	RF24_Struct.OutPwr = OutPwr;
	
	if (DataRate == RF24_DataRate_2M) 
	{
		tmp |= 1 << RF24_RF_DR_HIGH;
	} 
	else if (DataRate == RF24_DataRate_250k) 
	{
		tmp |= 1 << RF24_RF_DR_LOW;
	}
	
	if (OutPwr == RF24_OutputPower_0dBm) /* If 1Mbps, all bits set to 0 */
	{
		tmp |= 3 << RF24_RF_PWR;
	} 
	else if (OutPwr == RF24_OutputPower_M6dBm) 
	{
		tmp |= 2 << RF24_RF_PWR;
	} 
	else if (OutPwr == RF24_OutputPower_M12dBm) 
	{
		tmp |= 1 << RF24_RF_PWR;
	}
	
	RF24_WriteRegister(RF24_REG_RF_SETUP, tmp);
}


/**
 * @brief  Reads interrupts from NRF 
 * @param  *IRQ: Pointer to @ref RF24_IRQ_t where IRQ status will be saved
 * @retval IRQ status
 *            - 0: No interrupts are active
 *            - > 0: At least one interrupt is active
 */
static uint8_t RF24_Read_Interrupts(RF24_IRQ_t* IRQ) 
{
	IRQ->Status = RF24_GetStatus();
	return(IRQ->Status);
}


/**
 * @brief  Clears interrupt status
 * @param  None
 * @retval None
 */
static void RF24_Clear_Interrupts(void) 
{
	RF24_WriteRegister(0x07, 0x70);
}


/**
 * @brief  
 * @param 
 * @retval 
 */
static void RF24_Flush(uint8_t mask) 
{
//	RF24_CSN_LOW; 
	HAL_SPI_Transmit(&hspi2, &mask, 1, 10); 
//	RF24_CSN_HIGH; 
}


/******************************   END OF FILE  **********************************/

