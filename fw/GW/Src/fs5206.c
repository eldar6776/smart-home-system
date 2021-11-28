/************************************************************************
This is a library to handling capacitive multitouch sensors using FT5x06.
Originally written to work with ER-TFTM070-5 (LCD module) from EastRising.
Written by Helge Langehaug, February 2014
BSD license, all text above must be included in any redistribution
*************************************************************************/


#include "main.h"
#include "fs5206.h"
#include "hotel_ctrl.h"


int16_t ax_Phys[2],ay_Phys[2];
sMinMaxTypeDef xyMinMax[2];
struct sTouchEventTypeDef ts_event; 
GUI_PID_STATE State;

void FS5206_Init(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;
	
	// enable APB1 peripheral clock for I2C1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	// enable clock for SCL and SDA pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	/* setup SCL and SDA pins
	 * You can connect I2C1 to two different
	 * pairs of pins:
	 * 1. SCL on PB6 and SDA on PB7 
	 * 2. SCL on PB8 and SDA on PB9
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // we are going to use PB6 and PB7
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;       // enable pull up resistors
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB
	
	// Connect I2C1 pins to AF  
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA
	
	// configure I2C1 
	I2C_InitStruct.I2C_ClockSpeed = 100000; 		// 100kHz
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			// I2C mode
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	// 50% duty cycle --> standard
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;			// own address, not relevant in master mode
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;		// disable acknowledge when reading (can be changed later on)
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
	I2C_Init(I2C1, &I2C_InitStruct);				// init I2C1
	
	// enable I2C1
	I2C_Cmd(I2C1, ENABLE);
	DelayMs(1000);
	if (FS5206_WriteByte(FT5206_WRITE_ADDRESS, FT5206_DEVICE_MODE, 0) != 0) NVIC_SystemReset();;
	FS5206_ReadXY();
}



void FS5206_GetRegInfo(uint8_t *reg){
    uint8_t reg_cnt, tmp;	
	tmp = FS5206_ReadByte(FT5206_I2C_ADDRESS, FT5206_NUMBER_OF_REGISTERS);	
    while(tmp != 0)    {
		reg[reg_cnt++] = tmp;
		tmp = FS5206_ReadByte(FT5206_I2C_ADDRESS, FT5206_NUMBER_OF_REGISTERS);
    }	
}


uint8_t FS5206_GetTouchPos(uint16_t *touch_pos, uint8_t *reg){
    uint8_t touch_cnt = FS5206_ReadByte(FT5206_I2C_ADDRESS, FT5206_TD_STATUS)  & 0x0f;	
    if (touch_cnt > 0){
      touch_pos[0] = (uint16_t)(((reg[FT5206_TOUCH1_XH] & 0x0f) << 8) + reg[FT5206_TOUCH1_XL]);
      touch_pos[1] = (uint16_t)(((reg[FT5206_TOUCH1_YH] & 0x0f) << 8) + reg[FT5206_TOUCH1_YL]);
    }	
    if (touch_cnt > 1){
      touch_pos[2] = (uint16_t)(((reg[FT5206_TOUCH2_XH] & 0x0f) << 8) + reg[FT5206_TOUCH2_XL]);
      touch_pos[3] = (uint16_t)(((reg[FT5206_TOUCH2_YH] & 0x0f) << 8) + reg[FT5206_TOUCH2_YL]);
    }	
    if (touch_cnt > 2){
      touch_pos[4] = (uint16_t)(((reg[FT5206_TOUCH3_XH] & 0x0f) << 8) + reg[FT5206_TOUCH3_XL]);
      touch_pos[5] = (uint16_t)(((reg[FT5206_TOUCH3_YH] & 0x0f) << 8) + reg[FT5206_TOUCH3_YL]);
    }	
    if (touch_cnt > 3){
      touch_pos[6] = (uint16_t)(((reg[FT5206_TOUCH4_XH] & 0x0f) << 8) + reg[FT5206_TOUCH4_XL]);
      touch_pos[7] = (uint16_t)(((reg[FT5206_TOUCH4_YH] & 0x0f) << 8) + reg[FT5206_TOUCH4_YL]);
    }	
    if (touch_cnt > 4){
      touch_pos[8] = (uint16_t)(((reg[FT5206_TOUCH5_XH] & 0x0f) << 8) + reg[FT5206_TOUCH5_XL]);
      touch_pos[9] = (uint16_t)(((reg[FT5206_TOUCH5_YH] & 0x0f) << 8) + reg[FT5206_TOUCH5_YL]);
    }	
    return touch_cnt;	
}


uint8_t FS5206_WriteByte(uint8_t dev_address, uint8_t data_address, uint8_t data){
    uint32_t timeout = I2C1_TIMEOUT;    
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)){
		if(timeout != 0) timeout--; 
		else return 1;
	} 
	I2C_Send7bitAddress(I2C1, dev_address, I2C_Direction_Transmitter);
	timeout = I2C1_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) 	{
		if(timeout != 0) timeout--; 
		else return 1;
	}  
	I2C1->SR2;
	timeout = I2C1_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) 	{
		if(timeout != 0) timeout--; 
		else return 1;
	}
	I2C_SendData(I2C1, data_address);
	timeout = I2C1_TIMEOUT;	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) 	{
		if(timeout != 0) timeout--; 
		else return 1;
	}
	I2C_SendData(I2C1, data);
	timeout = I2C1_TIMEOUT;	
	while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))){
		if(timeout != 0) timeout--; 
		else return 1;
	}
	I2C_GenerateSTOP(I2C1, ENABLE);
    return 0;
}


uint8_t FS5206_ReadByte(uint8_t dev_address, uint8_t data_address){
	uint8_t ret_data;
	uint32_t timeout;

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C1, ENABLE);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return 0;
	}

	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_Send7bitAddress(I2C1, dev_address, I2C_Direction_Transmitter); 
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return 0;
	}  

	I2C1->SR2;
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return 0;
	}  

	I2C_SendData(I2C1, data_address);
	timeout = I2C1_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return 0;
	}

	I2C_GenerateSTART(I2C1, ENABLE);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return 0;
	}

	I2C_Send7bitAddress(I2C1, dev_address, I2C_Direction_Receiver);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR))		
	{
		if(timeout != 0) timeout--; 
		else return 0;
	}

	I2C1->SR2;
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) 
	{
		if(timeout != 0) timeout--; 
		else return 0;
	} 

	I2C_GenerateSTOP(I2C1, ENABLE);
	ret_data = I2C_ReceiveData(I2C1);
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	return(ret_data);	
}



uint8_t FS5206_ReadBytes(uint8_t dev_address, uint8_t data_address, uint8_t *buff, uint8_t size)
{
	uint32_t timeout;
	uint8_t n;

	if(size == 0)return 1;
	I2C_GenerateSTART(I2C1, ENABLE);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	if(size == 1) I2C_AcknowledgeConfig(I2C1, DISABLE);
	else I2C_AcknowledgeConfig(I2C1, ENABLE);

	I2C_Send7bitAddress(I2C1, dev_address, I2C_Direction_Transmitter);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	I2C1->SR2;
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	I2C_SendData(I2C1, data_address);
	timeout = I2C1_TIMEOUT;
	
	while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	I2C_GenerateSTART(I2C1, ENABLE);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	I2C_Send7bitAddress(I2C1, dev_address, I2C_Direction_Receiver);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	I2C1->SR2;

	for(n = 0; n < size; n++) 
	{

		if((n + 1) >= size) 
		{
			I2C_AcknowledgeConfig(I2C1, DISABLE);
			I2C_GenerateSTOP(I2C1, ENABLE);
		}

		timeout = I2C1_TIMEOUT;
		
		while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) 
		{
			if(timeout != 0) timeout--; 
			else return 1;
		}
		
		buff[n] = I2C_ReceiveData(I2C1);
	}

	I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 0;
}


uint8_t FS5206_WriteBytes(uint8_t dev_address, uint8_t data_address, uint8_t *buff, uint8_t size)
{
	uint32_t timeout;
	uint8_t n;

	if(size == 0) return 1;
	I2C_GenerateSTART(I2C1, ENABLE);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	I2C_Send7bitAddress(I2C1, dev_address, I2C_Direction_Transmitter);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	I2C1->SR2;
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	I2C_SendData(I2C1, data_address);
	timeout = I2C1_TIMEOUT;
	
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) 
	{
		if(timeout != 0) timeout--; 
		else return 1;
	}

	for(n = 0; n < size; n++) 
	{
		I2C_SendData(I2C1, buff[n]);
		timeout = I2C1_TIMEOUT;
			
		while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) 
		{
			if(timeout != 0) timeout--; 
			else return 1;
		}
	}

	I2C_GenerateSTOP(I2C1, ENABLE);
    return 0;
}

uint8_t FS5206_ReadXY(void)
{
	uint8_t buf[32] = {0};
	uint8_t ret = 0;
	
#ifdef CONFIG_FT5X0X_MULTITOUCH
	FS5206_ReadBytes(FT5206_READ_ADDRESS, 0, buf, 31);
#else
	FS5206_ReadBytes(FT5206_READ_ADDRESS, 0, buf, 7);
#endif
	
	ts_event.touch_point = buf[2] & 0x0f;
	ret = ts_event.touch_point;
#ifdef CONFIG_FT5X0X_MULTITOUCH
	switch (ts_event.touch_point) 
	{
		case 5:
		{
           ts_event.x5 = (int16_t)(buf[19] & 0x0F)<<8 | (int16_t)buf[20];
           ts_event.y5 = (int16_t)(buf[21] & 0x0F)<<8 | (int16_t)buf[22];
		}
				
		case 4:
		{
           ts_event.x4 = (int16_t)(buf[15] & 0x0F)<<8 | (int16_t)buf[16];
           ts_event.y4 = (int16_t)(buf[17] & 0x0F)<<8 | (int16_t)buf[18];
		}
				
		case 3:
		{
           ts_event.x3 = (int16_t)(buf[11] & 0x0F)<<8 | (int16_t)buf[12];
           ts_event.y3 = (int16_t)(buf[13] & 0x0F)<<8 | (int16_t)buf[14];
		}		
		
		case 2:
		{
           ts_event.x2 = (int16_t)(buf[7] & 0x0F)<<8 | (int16_t)buf[8];
           ts_event.y2 = (int16_t)(buf[9] & 0x0F)<<8 | (int16_t)buf[10];
		}
				
		case 1:
		{
			ts_event.x1 = (int16_t)(buf[3] & 0x0F)<<8 | (int16_t)buf[4];
			ts_event.y1 = (int16_t)(buf[5] & 0x0F)<<8 | (int16_t)buf[6];
			
		}
		break;					 
		
		default:
		return 0;	
	}
#else
	if(ts_event.touch_point == 1)
	{
		ts_event.x1 = (int16_t)(buf[3] & 0x0F)<<8 | (int16_t)buf[4];
		ts_event.y1 = (int16_t)(buf[5] & 0x0F)<<8 | (int16_t)buf[6];
		ret = 1;
	}
	else
	{
		
		ret = 0;
	}
#endif
    
	return ret;
}


void Calibration(int16_t x_size, int16_t y_size)
{
	int16_t ax[5], ay[5], tp_x[5], tp_y[5], cnt;
	ax[0] = 20;             ay[0] = 20;
	ax[1] = x_size -20;     ay[1] = 20;
	ax[2] = x_size -20;     ay[2] = y_size-20;
	ax[3] = 20;             ay[3] = y_size-20;
	ax[4] = x_size/2;       ay[4] = y_size/2;

	for(cnt = 0; cnt < 5; cnt++)
	{
		GUI_SetFont(&GUI_Font13_ASCII);
		GUI_SetBkColor(GUI_BLACK);  
		GUI_Clear();
		GUI_SetColor(GUI_WHITE);  
		GUI_FillCircle(ax[cnt], ay[cnt], 10);
		GUI_SetColor(GUI_BLACK);    
		GUI_FillCircle(ax[cnt], ay[cnt], 5);
		GUI_SetColor(GUI_WHITE);
		GUI_DispStringAt("Press here", ax[cnt]+20, ay[cnt]);
		
		while (State.Pressed == 0) 
		{
			GUI_Delay(100);
			GUI_TOUCH_Exec();
		}
		
		GUI_Delay(500);
		State.Pressed = 0;
		tp_x[cnt] = State.x;
		tp_y[cnt] = State.y;
		GUI_Clear();
		GUI_DispStringAt("OK", ax[cnt], ay[cnt]);			
	}

	/* calculate and display values for configuration file */  
	{ 
		GUI_Clear();
		GUI_DispString  ("x0: ");	GUI_DispDec(tp_x[0], 4);	
		GUI_DispString  ("  y0: ");	GUI_DispDec(tp_y[0], 4);   
		GUI_DispNextLine();
		GUI_DispString  ("x1: ");	GUI_DispDec(tp_x[1], 4);
		GUI_DispString  ("  y1: ");	GUI_DispDec(tp_y[1], 4);	
		GUI_DispNextLine();
		GUI_DispString  ("x2: ");	GUI_DispDec(tp_x[2], 4);
		GUI_DispString  ("  y2: ");	GUI_DispDec(tp_y[2], 4);	
		GUI_DispNextLine();
		GUI_DispString  ("x3: ");	GUI_DispDec(tp_x[3], 4);
		GUI_DispString  ("  y3: ");	GUI_DispDec(tp_y[3], 4);	
		GUI_DispNextLine();
		GUI_DispString  ("x4: ");	GUI_DispDec(tp_x[4], 4);
		GUI_DispString  ("  y4: ");	GUI_DispDec(tp_y[4], 4);	
		GUI_DispNextLine();
		GUI_DispString  ("Please touch display to continue...");

		while (State.Pressed == 0) 
		{
			GUI_Delay(100);
			GUI_TOUCH_Exec();
		}
		
		GUI_Delay(500);
	}
// 	GUI_TOUCH_AD_TOP  Press the touch at the top and write down the analog input value in Y
// 	GUI_TOUCH_AD_BOTTOM  Press the touch at the bottom and write down the analog input value in Y.
// 	GUI_TOUCH_AD_LEFT Press the touch at the left and write down the analog input value in X.
// 	GUI_TOUCH_AD_RIGHT Press the touch at the right and write down the analog input value in X
//	GUI_TOUCH_Calibrate(GUI_COORD_X, 0, 240, GUI_TOUCH_AD_TOP , GUI_TOUCH_AD_BOTTOM);
// 	GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, 320, GUI_TOUCH_AD_LEFT, GUI_TOUCH_AD_RIGHT);

	GUI_TOUCH_Calibrate(GUI_COORD_X, 0, 480, tp_x[0], tp_x[1]);
	GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, 272, tp_y[1], tp_y[2]);
}

