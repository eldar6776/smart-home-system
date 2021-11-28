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
#include "i2cee.h"
/* Private typedef -----------------------------------------------------------*/
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
/* Private define ------------------------------------------------------------*/
I2C2_DEV_t I2C2DEV = {
// PORT , PIN      ,  Clock              ,  Source 
  {GPIOH, GPIO_Pin_4, RCC_AHB1Periph_GPIOH, GPIO_PinSource4}, // SCL an PH4
  {GPIOH, GPIO_Pin_5, RCC_AHB1Periph_GPIOH, GPIO_PinSource5}, // SDA an PH5
};
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t eebuff[I2CEE_BSIZE];
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  :
  * @param  :
  * @retval :
  */
void I2CEE_Init(void)
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
/**
  * @brief  :
  * @param  :
  * @retval :
  */
void I2C2_Init(void)
{
    I2C_InitTypeDef  I2C_InitStructure;

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C2_CLOCK_FRQ;
    I2C_Cmd(I2C2, ENABLE);
    I2C_Init(I2C2, &I2C_InitStructure);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2C_EEPROM_timeout  (uint8_t retval)
{
  I2C_GenerateSTOP(I2C2, ENABLE);
  I2C_SoftwareResetCmd(I2C2, ENABLE);
  I2C_SoftwareResetCmd(I2C2, DISABLE);
  I2C_DeInit(I2C2);
  I2C2_Init();
  return(retval);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_WriteCMD      (uint8_t slave_address, uint8_t cmd)
{
	uint32_t timeout = I2C2_TIMEOUT;

	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB))		
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, cmd);
	timeout = I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_GenerateSTOP(I2C2, ENABLE);
	return(0x0U);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_ReadByte      (uint8_t slave_address, uint8_t  data_address, uint8_t *data)
{
	uint32_t timeout = I2C2_TIMEOUT;

	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C_AcknowledgeConfig(I2C2, DISABLE);
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter); 
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C_SendData(I2C2, data_address);
	timeout = I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--;  
		else return(0x1U);
	}
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Receiver);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR))		
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C_GenerateSTOP(I2C2, ENABLE);
	*data = I2C_ReceiveData(I2C2);
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	return(0x0U);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_WriteByte     (uint8_t slave_address, uint8_t  data_address, uint8_t *data)
{
	uint32_t timeout=I2C2_TIMEOUT;
	
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--;  
		else return(0x1U);
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C_SendData(I2C2, data_address);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--;  
		else return(0x1U);
	}
	I2C_SendData(I2C2, *data);
	timeout = I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(0x1U);
	}
	I2C_GenerateSTOP(I2C2, ENABLE);
	return(0x0U);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_ReadByte16    (uint8_t slave_address, uint16_t data_address, uint8_t *data)
{
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t lo, hi;

	lo = (data_address & 0x00ff);
	hi = (data_address >> 8);

	I2C_GenerateSTART(I2C2, ENABLE);  
	timeout = I2C2_TIMEOUT;
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else I2C_EEPROM_timeout(0x01U);
	}
	I2C_AcknowledgeConfig(I2C2, DISABLE);
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout!=0) timeout--; 
		else return(0x01U);
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x01U);
	}
	I2C_SendData(I2C2, hi);
	timeout=I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(0x01U);
	}
	I2C_SendData(I2C2, lo);
	timeout = I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(0x01U);
	}
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x01U);
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Receiver);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR))
	{
		if(timeout != 0) timeout--; 
		else return(0x01U);
	}	
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE)) 
	{
		if(timeout != 0) timeout--; 
		else return(0x01U);
	}
	I2C_GenerateSTOP(I2C2, ENABLE);
	*data = I2C_ReceiveData(I2C2);
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	return(0x0U);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_WriteByte16   (uint8_t slave_address, uint16_t data_address, uint8_t *data)
{
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t lo, hi;

	lo = (data_address & 0x00ff);
	hi = (data_address >> 8);  
	I2C_GenerateSTART(I2C2, ENABLE); 
	timeout = I2C2_TIMEOUT;
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
        else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, hi);
	timeout=I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, lo);
	timeout=I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{	
		if(timeout != 0) timeout--;
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, *data);
	timeout = I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{	
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_GenerateSTOP(I2C2, ENABLE);
	return(0x0U);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_ReadBytes     (uint8_t slave_address, uint8_t  data_address, uint8_t *data, uint16_t cnt)
{
	uint32_t timeout = I2C2_TIMEOUT;

	if(cnt == 0) return(0x1U);
    else if(cnt > I2CEE_BSIZE) return(0x1U);
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x1U));
	}
	if(cnt == 1) I2C_AcknowledgeConfig(I2C2, DISABLE);
	else I2C_AcknowledgeConfig(I2C2, ENABLE);
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x1U));
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x1U));
	}
	I2C_SendData(I2C2, data_address);
	timeout = I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x1U));
	}
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x1U));
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Receiver);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x1U));
	}
	I2C2->SR2;
	while(cnt) 
	{
		if((cnt - 1U) == 0U) 
		{
			I2C_AcknowledgeConfig(I2C2, DISABLE);
			I2C_GenerateSTOP(I2C2, ENABLE);
		}
		timeout = I2C2_TIMEOUT;		
		while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE)) 
		{
			if(timeout != 0) timeout--; 
			else return(I2C_EEPROM_timeout(0x1U));
		}		
		*data++ = I2C_ReceiveData(I2C2);
        --cnt;
	}
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	return(0x0U);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_WriteBytes    (uint8_t slave_address, uint8_t  data_address, uint8_t *data, uint16_t cnt)
{
	uint32_t timeout = I2C2_TIMEOUT;

	if(cnt == 0) return(0x01U);
    else if(cnt > I2CEE_BLOCK) return(0x01U);
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, data_address);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	while(cnt) 
	{
        --cnt;
		I2C_SendData(I2C2, *data++);
		timeout = I2C2_TIMEOUT;			
		while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
		{
			if(timeout != 0) timeout--; 
			else return(I2C_EEPROM_timeout(0x01U));
		}
	}
	I2C_GenerateSTOP(I2C2, ENABLE);
	return(0x0U);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_ReadBytes16   (uint8_t slave_address, uint16_t data_address, uint8_t *data, uint16_t cnt)
{
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t lo, hi;

	if(cnt == 0) return(0x01U);
    else if(cnt > I2CEE_BSIZE) return(0x01U);
	lo = (data_address & 0x00ff);
	hi = (data_address >> 8);
	I2C_GenerateSTART(I2C2, ENABLE);  
	timeout = I2C2_TIMEOUT;
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	if(cnt == 1) I2C_AcknowledgeConfig(I2C2, DISABLE);
	else I2C_AcknowledgeConfig(I2C2, ENABLE);
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout!=0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, hi);
	timeout=I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, lo);
	timeout = I2C2_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_GenerateSTART(I2C2, ENABLE);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Receiver);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR))
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}	
	I2C2->SR2;	
	while(cnt) 
	{
		if((cnt - 1U) == 0x0U) 
		{
			I2C_AcknowledgeConfig(I2C2, DISABLE);
			I2C_GenerateSTOP(I2C2, ENABLE);
		}
		timeout = I2C2_TIMEOUT;		
		while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE)) 
		{
			if(timeout != 0) timeout--; 
			else return(I2C_EEPROM_timeout(0x01U));
		}		
		*data++ = I2C_ReceiveData(I2C2);
        --cnt;
	}
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	return(0x0U);
}
/**
  * @brief  :
  * @param  :
  * @retval :
  */
uint8_t I2CEE_WriteBytes16  (uint8_t slave_address, uint16_t data_address, uint8_t *data, uint16_t cnt)
{
	uint32_t timeout = I2C2_TIMEOUT;
	uint8_t lo, hi;
    if(cnt == 0) return(0x01U);
    else if(cnt > I2CEE_BLOCK) return(0x01U);
	lo = (data_address & 0x00ff);
	hi = (data_address >> 8);  
	I2C_GenerateSTART(I2C2, ENABLE); 
	timeout = I2C2_TIMEOUT;
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_Send7bitAddress(I2C2, slave_address, I2C_Direction_Transmitter);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C2->SR2;
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, hi);
	timeout=I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return(I2C_EEPROM_timeout(0x01U));
	}
	I2C_SendData(I2C2, lo);
	timeout = I2C2_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) 
	{	
		if(timeout != 0) timeout--;
		else return(I2C_EEPROM_timeout(0x01U));
	}	
	while(cnt) 
	{
        --cnt;
		I2C_SendData(I2C2, *data++);
		timeout = I2C2_TIMEOUT;			
		while ((!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C2, I2C_FLAG_BTF))) 
		{
			if(timeout != 0) timeout--; 
			else return(I2C_EEPROM_timeout(0x01U));
		}
	}
	I2C_GenerateSTOP(I2C2, ENABLE);
	return(0x0U);	
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
