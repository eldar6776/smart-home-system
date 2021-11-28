/**
 ******************************************************************************
 * File Name          : i2c_eeprom.c
 * Date               : 21/08/2016 20:59:16
 * Description        : 24c1024 i2c eeprom control modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "i2c_eeprom.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
I2C2_DEV_t I2C2DEV = {
// PORT , PIN      ,  Clock              ,  Source 
  {GPIOH, GPIO_Pin_4, RCC_AHB1Periph_GPIOH, GPIO_PinSource4}, // SCL an PH4
  {GPIOH, GPIO_Pin_5, RCC_AHB1Periph_GPIOH, GPIO_PinSource5}, // SDA an PH5
};

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t aEepromBuffer[I2C_EE_BUFFER_SIZE];
uint8_t *p_i2c_ee_buffer;

/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
void I2C_EERPOM_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_AHB1PeriphClockCmd(I2C2DEV.SCL.CLK, ENABLE); 
	RCC_AHB1PeriphClockCmd(I2C2DEV.SDA.CLK, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE); 
	
	GPIO_PinAFConfig(I2C2DEV.SCL.PORT, I2C2DEV.SCL.SOURCE, GPIO_AF_I2C2); 
	GPIO_PinAFConfig(I2C2DEV.SDA.PORT, I2C2DEV.SDA.SOURCE, GPIO_AF_I2C2);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = I2C2DEV.SCL.PIN;
	GPIO_Init(I2C2DEV.SCL.PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = I2C2DEV.SDA.PIN;
	GPIO_Init(I2C2DEV.SDA.PORT, &GPIO_InitStructure);

	I2C2_Init();
}


int16_t I2C_EERPOM_ReadByte(uint8_t slave_address, uint8_t data_address)
{
	int16_t ret_data = 0;
	uint32_t timeout = I2C2_TIMEOUT;

	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	}

	I2C_AcknowledgeConfig(I2C2, DISABLE);
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter); 
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-2));
	}  

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}  

	I2C_SendData(I2C2, data_address);
	timeout = I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--;  
		else return(I2C_EEPROM_timeout(-4));
	}

	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-5));
	}

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Receiver);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR))		
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-6));
	}

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-7));
	} 

	I2C_GenerateSTOP(I2C2, ENABLE);
	ret_data = (int16_t)(I2C_ReceiveData(I2C2));
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	return(ret_data);
}


int16_t I2C_EERPOM_WriteByte(uint8_t slave_address, uint8_t data_address, uint8_t data)
{
	int16_t ret_data = 0;
	uint32_t timeout=I2C2_TIMEOUT;
	
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	} 

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--;  
		else return(I2C_EEPROM_timeout(-2));
	}  

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}

	I2C_SendData(I2C2, data_address);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--;  
		else return(I2C_EEPROM_timeout(-4));
	}

	I2C_SendData(I2C2, data);
	timeout = I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-5));
	}

	I2C_GenerateSTOP(I2C2, ENABLE);
	ret_data = 0;
	return(ret_data);
}


int16_t I2C_EERPOM_ReadBytes(uint8_t slave_address, uint8_t data_address, uint8_t cnt)
{
	int16_t ret_data = 0;
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t n;

	if(cnt == 0) return(-8);
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	}

	if(cnt == 1) 
	{
		I2C_AcknowledgeConfig(I2C2, DISABLE);
	}
	else 
	{
		I2C_AcknowledgeConfig(I2C2, ENABLE);
	}

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-2));
	}

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}

	I2C_SendData(I2C2, data_address);
	timeout = I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}

	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-5));
	}

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Receiver);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-6));
	}

	I2C2->SR2;

	for(n = 0; n < cnt; n++) 
	{

		if((n + 1) >= cnt) 
		{
			I2C_AcknowledgeConfig(I2C2, DISABLE);
			I2C_GenerateSTOP(I2C2, ENABLE);
		}

		timeout = I2C2_TIMEOUT;
		
		while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE)) 
		{
			if(timeout != 0) timeout--; 
			else return(I2C_EEPROM_timeout(-7));
		}
		
		aEepromBuffer[n] = I2C_ReceiveData(I2C2);
	}

	I2C_AcknowledgeConfig(I2C2, ENABLE);
	ret_data = 0;
	return(ret_data);
}


int16_t I2C_EERPOM_WriteBytes(uint8_t slave_address, uint8_t data_address, uint8_t cnt)
{
	int16_t ret_data = 0;
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t n;

	if(cnt == 0) return(-6);
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	}

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-2));
	}

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}

	I2C_SendData(I2C2, data_address);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}

	for(n = 0; n < cnt; n++) 
	{
		I2C_SendData(I2C2, aEepromBuffer[n]);
		timeout = I2C2_TIMEOUT;
			
		while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
		{
			if(timeout != 0) timeout--; 
			else return(I2C_EEPROM_timeout(-5));
		}
	}

	I2C_GenerateSTOP(I2C2, ENABLE);
	ret_data = 0;
	return(ret_data);
}


int16_t I2C_EERPOM_WriteCMD(uint8_t slave_address, uint8_t cmd)
{
	int16_t ret_data = 0;
	uint32_t timeout = I2C2_TIMEOUT;

	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB))		
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	}

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-2));
	}

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}

	I2C_SendData(I2C2, cmd);
	timeout = I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}

	I2C_GenerateSTOP(I2C2, ENABLE);
	ret_data = 0;
	return(ret_data);
}


int16_t I2C_EERPOM_ReadByte16(uint8_t slave_address, uint16_t data_address)
{
	int16_t ret_data = 0;
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t lo, hi;

	lo = (data_address & 0x00ff);
	hi = (data_address >> 8);

	I2C_GenerateSTART(I2C2, ENABLE);  
	timeout = I2C2_TIMEOUT;

	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	}

	I2C_AcknowledgeConfig(I2C2, DISABLE);
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout!=0) timeout--; 
		else return(I2C_EEPROM_timeout(-2));
	}  

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}  

	I2C_SendData(I2C2, hi);
	timeout=I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}

	I2C_SendData(I2C2, lo);
	timeout = I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}  

	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-5));
	}

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Receiver);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR))
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-6));
	}

	
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-7));
	} 

	I2C_GenerateSTOP(I2C2, ENABLE);
	ret_data = (int16_t)(I2C_ReceiveData(I2C2));
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	return(ret_data);
}


int16_t I2C_EERPOM_WriteByte16(uint8_t slave_address, uint16_t data_address, uint8_t data)
{
	int16_t ret_data = 0;
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t lo, hi;

	lo = (data_address & 0x00ff);
	hi = (data_address >> 8);  
	I2C_GenerateSTART(I2C2, ENABLE); 
	timeout = I2C2_TIMEOUT;

	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	} 

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-2));
	}  

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}

	I2C_SendData(I2C2, hi);
	timeout=I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}

	I2C_SendData(I2C2, lo);
	timeout=I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{	
		if(timeout != 0) timeout--;
		else return(I2C_EEPROM_timeout(-4));
	}  

	I2C_SendData(I2C2, data);
	timeout = I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{	
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-5));
	}

	I2C_GenerateSTOP(I2C2, ENABLE);
	ret_data = 0;
	return(ret_data);
}


int16_t I2C_EERPOM_ReadBytes16(uint8_t slave_address, uint16_t data_address, uint8_t cnt)
{
	int16_t ret_data = 0;
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t lo, hi, n;

	if(cnt == 0) return(-8);
	lo = (data_address & 0x00ff);
	hi = (data_address >> 8);

	I2C_GenerateSTART(I2C2, ENABLE);  
	timeout = I2C2_TIMEOUT;

	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	}

	if(cnt == 1) 
	{
		I2C_AcknowledgeConfig(I2C2, DISABLE);
	}
	else 
	{
		I2C_AcknowledgeConfig(I2C2, ENABLE);
	}
	
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout!=0) timeout--; 
		else return(I2C_EEPROM_timeout(-2));
	}  

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}  

	I2C_SendData(I2C2, hi);
	timeout=I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}

	I2C_SendData(I2C2, lo);
	timeout = I2C2_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}  

	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-5));
	}

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Receiver);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR))
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-6));
	}
	
	I2C2->SR2;
	
	for(n = 0; n < cnt; n++) 
	{

		if((n + 1) >= cnt) 
		{
			I2C_AcknowledgeConfig(I2C2, DISABLE);
			I2C_GenerateSTOP(I2C2, ENABLE);
		}

		timeout = I2C2_TIMEOUT;
		
		while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE)) 
		{
			if(timeout != 0) timeout--; 
			else return(I2C_EEPROM_timeout(-7));
		}
		
		aEepromBuffer[n] = I2C_ReceiveData(I2C2);
	}

	I2C_AcknowledgeConfig(I2C2, ENABLE);
	ret_data = 0;
	return(ret_data);
}


int16_t I2C_EERPOM_WriteBytes16(uint8_t slave_address, uint16_t data_address, uint8_t cnt)
{
	int16_t ret_data = 0;
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t lo, hi, n;

	lo = (data_address & 0x00ff);
	hi = (data_address >> 8);  
	I2C_GenerateSTART(I2C2, ENABLE); 
	timeout = I2C2_TIMEOUT;

	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-1));
	} 

	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-2));
	}  

	I2C2->SR2;
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-3));
	}

	I2C_SendData(I2C2, hi);
	timeout=I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(-4));
	}

	I2C_SendData(I2C2, lo);
	timeout = I2C2_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{	
		if(timeout != 0) timeout--;
		else return(I2C_EEPROM_timeout(-4));
	} 
	
	for(n = 0; n < cnt; n++) 
	{
		I2C_SendData(I2C2, aEepromBuffer[n]);
		timeout = I2C2_TIMEOUT;
			
		while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
		{
			if(timeout != 0) timeout--; 
			else return(I2C_EEPROM_timeout(-5));
		}
	}

	I2C_GenerateSTOP(I2C2, ENABLE);
	ret_data = 0;
	return(ret_data);	
}
//--------------------------------------------------------------
// nCount :  	20.000 = ca. 1,2ms
//          	200.000 = ca. 12ms
//--------------------------------------------------------------
void I2C_EERPOM_Delay(volatile uint32_t nCount)
{
  while(nCount--)
  {
  }
}


void I2C2_Init(void)
{
  I2C_InitTypeDef  I2C_InitStructure;

  // I2C-Konfiguration
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C2_CLOCK_FRQ;

  // I2C enable
  I2C_Cmd(I2C2, ENABLE);

  // Init Struktur
  I2C_Init(I2C2, &I2C_InitStructure);
}


int16_t I2C_EEPROM_timeout(int16_t time)
{
  int16_t ret_time = time;

  // Stop und Reset
  I2C_GenerateSTOP(I2C2, ENABLE);
  I2C_SoftwareResetCmd(I2C2, ENABLE);
  I2C_SoftwareResetCmd(I2C2, DISABLE);

  // I2C deinit
  I2C_DeInit(I2C2);
  // I2C init
  I2C2_Init();
    
  return(ret_time);
}
