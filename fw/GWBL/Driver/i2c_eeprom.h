/**
 ******************************************************************************
 * File Name          : i2c_eeprom.h
 * Date               : 21/08/2016 20:59:16
 * Description        : 24c1024 i2c eeprom control modul header
 ******************************************************************************
 */
#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H   			100	// version 1.00

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* Exported defines    -------------------------------------------------------*/
#define I2C2_CLOCK_FRQ   		100000	// I2C-Frq in Hz (400 kHz) 
#define I2C2_TIMEOUT     		0x3000	// Timeout Zeit
#define	I2C_EE_BUFFER_SIZE		512	
#define I2C_EE_BLOCK_SIZE		256
#define I2C_EE_READ_PAGE_0  	0xa1
#define I2C_EE_READ_PAGE_1  	0xa3
#define I2C_EE_WRITE_PAGE_0 	0xa0
#define I2C_EE_WRITE_PAGE_1 	0xa2
#define I2C_EE_PAGE_SIZE		0x10000
#define I2C_EE_WRITE_DELAY		2

/** ==========================================================================*/
/**                                                                           */
/**     	I 2 C    E E P R O M    M E M O R Y    A D D R E S S E      	  */ 
/**                                                                           */
/** ==========================================================================*/

#define EE_ETHERNET_IP_ADDRESS							((uint32_t)0x00000000)
#define EE_ETHERNET_SUBNET_ADDRESS						((uint32_t)0x00000004)
#define EE_ETHERNET_GATEWAY_ADDRESS						((uint32_t)0x00000008)
#define EE_ETHERNET_DNS_ADDRESS							((uint32_t)0x0000000c)
#define EE_RS485_INTERFACE_ADDRESS						((uint32_t)0x00000010)
#define EE_RS485_GROUP_ADDRESS							((uint32_t)0x00000012)
#define EE_RS485_BROADCAST_ADDRESS						((uint32_t)0x00000014)
#define EE_SYSTEM_CONFIG_ADDRESS						((uint32_t)0x00000016)
#define EE_SYSTEM_ID_ADDRESS							((uint32_t)0x0000001a)

#define EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS			((uint32_t)0x00000020)
#define EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS			((uint32_t)0x00000024)
#define EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS			((uint32_t)0x00000028)
#define EE_PREPAID_USER_EXIT_COUNTER_ADDRESS			((uint32_t)0x0000002c)
#define EE_CARD_USER_ENTRY_COUNTER_ADDRESS				((uint32_t)0x00000030)
#define EE_CARD_USER_EXIT_COUNTER_ADDRESS				((uint32_t)0x00000034)
#define EE_WRIST_USER_ENTRY_COUNTER_ADDRESS				((uint32_t)0x00000038)
#define EE_WRIST_USER_EXIT_COUNTER_ADDRESS				((uint32_t)0x0000003c)
#define EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS			((uint32_t)0x00000040)
#define EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS			((uint32_t)0x00000044)
#define EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS				((uint32_t)0x00000048)
#define EE_TOTAL_USER_EXIT_COUNTER_ADDRESS				((uint32_t)0x0000004c)
#define EE_COUNTER_RESET_DATE_ADDRESS					((uint32_t)0x00000050)

#define EE_MIFARE_AUTH_KEY_A_ADDRESS					((uint32_t)0x00000060)
#define EE_MIFARE_AUTH_KEY_B_ADDRESS					((uint32_t)0x00000065)
#define EE_PERMITED_USER_GROUP_ADDRESS					((uint32_t)0x00000070)


#define EE_LOG_LIST_START_ADDRESS  						((uint32_t)0x00000100)	// beginning of log list
#define EE_LOG_LIST_END_ADDRESS   						((uint32_t)0x00020000)	// end of log list

/* Exported types    ---------------------------------------------------------*/
typedef struct {
	GPIO_TypeDef* PORT;     // Port
	const uint16_t PIN;     // Pin
	const uint32_t CLK;     // Clock
	const uint8_t SOURCE;   // Source
}I2C2_PIN_t; 

typedef struct {
	I2C2_PIN_t  SCL;       // Clock-Pin
	I2C2_PIN_t  SDA;       // Data-Pin
}I2C2_DEV_t; 


/* Exported variables  -------------------------------------------------------*/
extern uint8_t aEepromBuffer[I2C_EE_BUFFER_SIZE];
extern uint8_t *p_i2c_ee_buffer;

/* Exported macros     -------------------------------------------------------*/

/* Exported functions  -------------------------------------------------------*/
void I2C_EERPOM_Init(void);
int16_t I2C_EERPOM_ReadByte(uint8_t slave_address, uint8_t data_address);
int16_t I2C_EERPOM_WriteByte(uint8_t slave_address, uint8_t data_address, uint8_t data);
int16_t I2C_EERPOM_ReadBytes(uint8_t slave_address, uint8_t data_address, uint8_t cnt);
int16_t I2C_EERPOM_WriteBytes(uint8_t slave_address, uint8_t data_address, uint8_t cnt);
int16_t I2C_EERPOM_WriteCMD(uint8_t slave_address, uint8_t cmd);
int16_t I2C_EERPOM_ReadByte16(uint8_t slave_address, uint16_t data_address);
int16_t I2C_EERPOM_WriteByte16(uint8_t slave_address, uint16_t data_address, uint8_t data);
int16_t I2C_EERPOM_ReadBytes16(uint8_t slave_address, uint16_t data_address, uint8_t cnt);
int16_t I2C_EERPOM_WriteBytes16(uint8_t slave_address, uint16_t data_address, uint8_t cnt);
void I2C_EERPOM_Delay(volatile uint32_t nCount);
int16_t I2C_EEPROM_timeout(int16_t time);
void I2C2_Init(void);

#endif
/******************************   END OF FILE  **********************************/
