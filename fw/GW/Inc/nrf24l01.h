/**
 ******************************************************************************
 * File Name          : nrf24l01.h
 * Date               : 08/05/2016 23:15:16
 * Description        : nrf24l01 radio modul services header
 ******************************************************************************
 *
 *	NRF24L01+	STM32Fxxx	DESCRIPTION
 *
 *	GND			GND			Ground
 * 	VCC			3.3V		3.3V
 *	CE			PB6			RF activated pin
 *	CSN			PB5			Chip select pin
 *	SCK			PB13		SCK pin for SPI2
 *	MOSI		PB15		MOSI pin for SPI2
 *	MISO		PB14		MISO pin for SPI2
 *	IRQ			Not used	Interrupt pin. Goes low when active. Pin functionality is active, but not used in library
 ******************************************************************************
 */
 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RF24_H__
#define __RF24_H__                                FW_BUILD // version


/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"


/* Exported defines    -------------------------------------------------------*/
#define RF24_BSIZE			    32


/* Interrupt masks */
#define RF24_IRQ_DATA_READY     0x40 /*!< Data ready for receive */
#define RF24_IRQ_TRAN_OK        0x20 /*!< Transmission went OK */
#define RF24_IRQ_MAX_RT         0x10 /*!< Max retransmissions reached, last transmission failed */


/* Exported types    ---------------------------------------------------------*/
typedef union _RF24_IRQ_t 
{
	struct 
	{
		uint8_t reserved0:4;
		uint8_t MaxRT:1;     /*!< Set to 1 if MAX retransmissions flag is set */
		uint8_t DataSent:1;  /*!< Set to 1 if last transmission is OK */
		uint8_t DataReady:1; /*!< Set to 1 if data are ready to be read */
		uint8_t reserved1:1;
		
	} F;
	
	uint8_t Status;          /*!< NRF status register value */
	
} RF24_IRQ_t;

/**
 * @brief  Transmission status enumeration
 */
typedef enum _RF24_Transmit_Status_t 
{
	RF24_Transmit_Status_Lost = 0x00,   /*!< Message is lost, reached maximum number of retransmissions */
	RF24_Transmit_Status_Ok = 0x01,     /*!< Message sent successfully */
	RF24_Transmit_Status_Sending = 0xFF /*!< Message is still sending */
	
} RF24_Transmit_Status_t;

/**
 * @brief  Data rate enumeration
 */
typedef enum _RF24_DataRate_t 
{
	RF24_DataRate_2M = 0x00, /*!< Data rate set to 2Mbps */
	RF24_DataRate_1M,        /*!< Data rate set to 1Mbps */
	RF24_DataRate_250k       /*!< Data rate set to 250kbps */
	
} RF24_DataRate_t;

/**
 * @brief  Output power enumeration
 */
typedef enum _RF24_OutputPower_t 
{
	RF24_OutputPower_M18dBm = 0x00,/*!< Output power set to -18dBm */
	RF24_OutputPower_M12dBm,       /*!< Output power set to -12dBm */
	RF24_OutputPower_M6dBm,        /*!< Output power set to -6dBm */
	RF24_OutputPower_0dBm          /*!< Output power set to 0dBm */
	
} RF24_OutputPower_t;



/* Exported variables  -------------------------------------------------------*/
extern __IO uint32_t rf24_flag;
extern __IO uint32_t rf24_timer;
extern __IO  uint8_t rf24_rxbuf[RF24_BSIZE]; 
extern __IO  uint8_t rf24_txbuf[RF24_BSIZE];


/* Exported macros     -------------------------------------------------------*/
#define RADIO_TxBufferReady()			(rf24_flag  |=  (1U << 0))
#define RADIO_TxBufferNotReady()		(rf24_flag  &=(~(1U << 0)))
#define IsRADIO_TxBufferReady()			((rf24_flag &   (1U << 0)) != 0)
#define RADIO_RxBufferReady()			(rf24_flag  |=  (1U << 1))
#define RADIO_RxBufferNotReady()		(rf24_flag  &=(~(1U << 1)))
#define IsRADIO_RxBufferReady()			((rf24_flag &   (1U << 1)) != 0)

/* Exported functions  -------------------------------------------------------*/
void RF24_Init(void);
void RF24_Service(void);


#endif  //  __RF24_H__
/******************************   END OF FILE  **********************************/

