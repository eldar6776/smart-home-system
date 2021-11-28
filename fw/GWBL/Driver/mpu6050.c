/**
  * @file    mpu6050.c 
  * @author  WB R&D Team - openmcu666
  * @version V0.1
  * @date    2015.6.24
  * @brief   Gyroscope Driver
  */
#include "stm32f4xx.h"                  // Device header
#include "IIC_MPU6050.h"
#include "delay.h"
#include "usart.h"
/**
  * @brief  初始化SDA、SCL
  * @param  None
  * @retval None
  */
void IIC_Bus_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;  //SCCB_SIC:SCCB_SID
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			      
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOH, &GPIO_InitStructure); 
	IIC_SID_H();                                           //拉高SDA、SCL
  delay_us(50);
  IIC_SIC_H();	                                         
  delay_us(50);
}

/**
  * @brief  设置SDA为输出
  * @param  None
  * @retval None
  */
void IIC_Data_Out()
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOH, &GPIO_InitStructure);

}

/**
  * @brief  设置SDA为输入
  * @param  None
  * @retval None
  */
void IIC_Data_In()
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;               
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOH, &GPIO_InitStructure);

}

/**
  * @brief  起始信号
  * @param  None
  * @retval None
  */
void IIC_Start(void)
{
    IIC_SID_H();              //数据线高电平
    delay_us(5);
    IIC_SIC_H();	            //在时钟线高的时候数据线由高至低
    delay_us(5);
    IIC_SID_L();
    delay_us(5);
    IIC_SIC_L();	            //数据线恢复低电平，单操作函数必要
    delay_us(5);

}

/**
  * @brief  停止信号
  * @param  None
  * @retval None
  */
void IIC_Stop()
{
   IIC_SID_L();
   delay_us(5);
   IIC_SIC_H();	
   delay_us(5);  
   IIC_SID_H();	
   delay_us(5);  

}

/**
  * @brief  不应答
  * @param  None
  * @retval None
  */
void IIC_Send_NoAck()
{
  IIC_SID_H();	
	delay_us(5);	
	IIC_SIC_H();	
	delay_us(5);	
	IIC_SIC_L();	
	delay_us(5);	
	IIC_SID_L();	
	delay_us(5);
}

/**
  * @brief  发送一个应答信号
  * @param  None
  * @retval None
  */
void IIC_Send_Ack()
{
 	IIC_SID_L();	
	delay_us(5);	
	IIC_SIC_L();	
	delay_us(5);	
	IIC_SIC_H();	
	delay_us(5);	
	IIC_SIC_L();	
	delay_us(5);	
	IIC_SID_L();	
	delay_us(5);

}

/**
  * @brief  写一个字节
  * @param  Data:写入的数据
  * @retval 1:发送成功;0:发送失败
  */
uint8_t IIC_Bus_Write_Byte(uint8_t Data)
{
	uint32_t i;
	uint8_t tem;

	for(i = 0; i < 8; i++) //循环8次发送数据
	{
		if((Data<<i) & 0x80)
		{
			IIC_SID_H();
		}
		else 
		{
			IIC_SID_L();
		}
		delay_us(5);
		IIC_SIC_H();	
		delay_us(10);
		IIC_SIC_L();	
		delay_us(5);
	}
	delay_us(10);
	IIC_Data_In();
	delay_us(10);
	IIC_SIC_H();	
	delay_us(10);
	if(IIC_SID_STATE)
	{
		tem = 0;//SDA=1发送失败
	}
	else 
	{
		tem = 1;//SDA=0发送成功，返回1
	}

	IIC_SIC_L();	
	delay_us(5);	
	IIC_Data_Out();
	return tem;  
}

/**
  * @brief  读一个字节
  * @param  ack:0:不发应答;1:发送应答
  * @retval read:读取的数据
  */
uint8_t IIC_Bus_Read_Byte(unsigned char ack)
{	
	uint32_t i;
	uint8_t read = 0;
	
	IIC_Data_In();
	delay_us(5);
	for(i = 8; i > 0; i--) //循环8次接收数据
	{		     
		delay_us(5);
		IIC_SIC_H();
		delay_us(5);
		read = read << 1;
		if(IIC_SID_STATE)
		{
			read += 1; 
		}
		IIC_SIC_L();
		delay_us(5);
	}	
    IIC_Data_Out();
	if(!ack)
	{
		IIC_Send_NoAck();
	}
	else
	{
	  IIC_Send_Ack();
	}
	return read;
}

/**
  * @brief  向MPU6050写一个字节数据
  * @param  Reg:寄存器地址
  * @param  Data:数据
  * @retval 0:发送成功;1:发送失败
  */
u8 MPU_Write_Byte(u8 Reg,u8 Data) 				 
{ 
    IIC_Start(); 
	if(!IIC_Bus_Write_Byte((MPU_ADDR<<1)|0))                //发送器件地址
	{
		IIC_Stop();		 
		return 1;		
	}
    IIC_Bus_Write_Byte(Reg);	                            //写寄存器地址
  
	if(!IIC_Bus_Write_Byte(Data))                           //发送数据

	{
		IIC_Stop();	 
		return 1;		 
	}		 
    IIC_Stop();	 
	return 0;
}

/**
  * @brief  从MPU6050读一个字节数据
  * @param  Reg:寄存器地址 
  * @retval Data:读取的数据
  */
u8 MPU_Read_Byte(u8 reg)
{
	u8 Data;
  IIC_Start(); 
	IIC_Bus_Write_Byte((MPU_ADDR<<1)|0);         //发送器件地址和写命令	

  IIC_Bus_Write_Byte(reg);	                   //写寄存器地址  
  IIC_Start();
	IIC_Bus_Write_Byte((MPU_ADDR<<1)|1);         //发送器件地址和读命令	
	Data=IIC_Bus_Read_Byte(0);                   //读取数据,不发应答
  IIC_Stop();			                            
	return Data;		
}

/**
  * @brief  向MPU6050写指定长度的数据
  * @param  Addr:器件地址 
  * @param  Reg:寄存器地址
  * @param  Len:写入长度
  * @param  Buf:数据区
  * @retval 0:写入成功;1:写入失败
  */
u8 MPU_Write_Len(u8 Addr,u8 Reg,u8 Len,u8 *Buf)
{
	u8 i; 
  IIC_Start(); 
	if(!IIC_Bus_Write_Byte((Addr<<1)|0))
	{
		IIC_Stop();		 
		return 1;		
	}
  IIC_Bus_Write_Byte(Reg);	

	for(i=0;i<Len;i++)
	{
		if(!IIC_Bus_Write_Byte(Buf[i]))	//发送数据

		{
			IIC_Stop();	 
			return 1;		 
		}		
	}    
  IIC_Stop();	 
	return 0;	
} 

/**
  * @brief  从MPU6050读指定长度的数据
  * @param  Addr:器件地址 
  * @param  Reg:寄存器地址
  * @param  Len:写入长度
  * @param  Buf:读取数据缓冲区
  * @retval 0:读取成功;1:读取失败
  */
u8 MPU_Read_Len(u8 Addr,u8 Reg,u8 Len,u8 *Buf)
{ 
 	IIC_Start(); 
	if(!IIC_Bus_Write_Byte((Addr<<1)|0))
	{
		IIC_Stop();		 
		return 1;		
	}
  IIC_Bus_Write_Byte(Reg);	
  IIC_Start();
  IIC_Bus_Write_Byte((Addr<<1)|1);
  
	while(Len)
	{
		if(Len==1)*Buf=IIC_Bus_Read_Byte(0);          //读数据,发送nACK 
		else *Buf=IIC_Bus_Read_Byte(1);		            //读数据,发送ACK  
		Len--;
		Buf++; 
	}    
  IIC_Stop();	
	return 0;	
}

/**
  * @brief  设置MPU6050加速度传感器满量程范围
  * @param  Fsr:0,±2g;1,±4g;2,±8g;3,±16g
  * @retval 0:成功;1:失败
  */
u8 MPU_Set_Accel_Fsr(u8 Fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG,Fsr<<3);//设置加速度传感器满量程范围  
}

/**
  * @brief  设置MPU6050陀螺仪传感器满量程范围
  * @param  Fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
  * @retval 0:成功;1:失败
  */
u8 MPU_Set_Gyro_Fsr(u8 Fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG,Fsr<<3);
}

/**
  * @brief  设置MPU6050的数字低通滤波器
  * @param  Lpf:数字低通滤波频率(Hz)
  * @retval 0:成功;1:失败
  */
u8 MPU_Set_LPF(u16 Lpf)
{
	u8 Data=0;
	if(Lpf>=188)Data=1;
	else if(Lpf>=98)Data=2;
	else if(Lpf>=42)Data=3;
	else if(Lpf>=20)Data=4;
	else if(Lpf>=10)Data=5;
	else Data=6; 
	return MPU_Write_Byte(MPU_CFG_REG,Data);//设置数字低通滤波器  
}

/**
  * @brief  设置MPU6050的速率
  * @param  Rate:4~1000(Hz)
  * @retval 0:成功;1:失败
  */
u8 MPU_Set_Rate(u16 Rate)
{
	u8 data;
	if(Rate>1000)Rate=1000;
	if(Rate<4)Rate=4;
	data=1000/Rate-1;
	data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	 
 	return MPU_Set_LPF(Rate/2);	                         //自动设置LPF为采样率的一半
}

/**
  * @brief  初始化6050
  * @param  None
  * @retval 0:成功;1:失败
  */
u8 MPU6050_Init(void)
{
	u8 Res=0;
  IIC_Bus_Init();
  MPU_Write_Byte(MPU_PWR_MGMT1_REG,0x80);	          //复位MPU6050
	delay_ms(80);
  MPU_Write_Byte(MPU_PWR_MGMT1_REG,0x00);	          //唤醒MPU6050 
	MPU_Set_Gyro_Fsr(3);					                   
	MPU_Set_Accel_Fsr(0);					                    
	MPU_Set_Rate(50);						                      //采样率50Hz
  MPU_Write_Byte(MPU_INT_EN_REG,0x00);	            //关闭所有中断
	MPU_Write_Byte(MPU_USER_CTRL_REG,0x00);        
	MPU_Write_Byte(MPU_FIFO_EN_REG,0x00);	           
	MPU_Write_Byte(MPU_INTBP_CFG_REG,0x80);	          
	Res=MPU_Read_Byte(MPU_DEVICE_ID_REG);
	if(Res==MPU_ADDR)                                 //器件ID正确
	{
		MPU_Write_Byte(MPU_PWR_MGMT1_REG,0x01);	         //设置CLKSEL,PLL X轴为参考
		MPU_Write_Byte(MPU_PWR_MGMT2_REG,0x00);	         //加速度与陀螺仪都工作
		MPU_Set_Rate(50);						                     //设置采样率为50Hz
 	}else return 1;
	return 0;
}

/**
  * @brief  得到温度值
  * @param  None
  * @retval 温度值
  */
short MPU_Get_Temperature(void)
{
  u8 Buf[2]; 
  short Raw;
	float Temp;
	MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,Buf); 
  Raw=((u16)Buf[0]<<8)|Buf[1];  
  Temp=36.53+((double)Raw)/340;  
  return Temp*100;;                                     //扩大100倍
}

/**
  * @brief  得到原始加速度值
  * @param  gx,gy,gz:陀螺仪x,y,z轴的原始度数
  * @retval 0:成功;1:失败
  */
u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
  u8 Res,Buf[6];  
	Res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,Buf);
	if(Res==0)
	{
		*ax=((u16)Buf[0]<<8)|Buf[1];  
		*ay=((u16)Buf[2]<<8)|Buf[3];  
		*az=((u16)Buf[4]<<8)|Buf[5];
	} 	
    return Res;;
}

/**
  * @brief  读取陀螺仪原始数据
  * @param  gx,gy,gz:陀螺仪x,y,z轴的原始度数
  * @retval 0:成功;1:失败
  */
u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
{
    u8 Res,Buf[6];  
	Res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,Buf);
	if(Res==0)
	{
		*gx=((u16)Buf[0]<<8)|Buf[1];  
		*gy=((u16)Buf[2]<<8)|Buf[3];  
		*gz=((u16)Buf[4]<<8)|Buf[5];
	} 	
    return Res;
}

/**
  * @brief  传送数据给匿名四轴上位机软件
  * @param  Fun:功能字. 0xA0~0xAF
  * @param  Data:数据缓存区
  * @param  Len:有效数据个数
  * @retval None
  */
void USART1_Report(u8 Fun,u8*Data,u8 Len)
{
	u8 Send_Buf[32];
	u8 i;
	Send_Buf[Len+3]=0;	                        //校验数置零
	Send_Buf[0]=0x88;	                          //帧头
	Send_Buf[1]=Fun;	                           //功能字
	Send_Buf[2]=Len;	                            //数据长度
	for(i=0;i<Len;i++)Send_Buf[3+i]=Data[i];			//拷贝数据
	for(i=0;i<Len+3;i++)Send_Buf[Len+3]+=Send_Buf[i];	//计算校验和	
	for(i=0;i<Len+4;i++) USART1_SendChar(Send_Buf[i]);	//发送数据到串口1 
}

/**
  * @brief  发送加速度传感器和陀螺仪的数据
  * @param  aacx,aacy,aacz:x,y,z三个方向上面的加速度值
  * @param  gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
  * @retval None
  */
void MPU6050_SendData(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz)
{
	u8 tbuf[12]; 
	tbuf[0]=(aacx>>8)&0xFF;
	tbuf[1]=aacx&0xFF;
	tbuf[2]=(aacy>>8)&0xFF;
	tbuf[3]=aacy&0xFF;
	tbuf[4]=(aacz>>8)&0xFF;
	tbuf[5]=aacz&0xFF; 
	tbuf[6]=(gyrox>>8)&0xFF;
	tbuf[7]=gyrox&0xFF;
	tbuf[8]=(gyroy>>8)&0xFF;
	tbuf[9]=gyroy&0xFF;
	tbuf[10]=(gyroz>>8)&0xFF;
	tbuf[11]=gyroz&0xFF;
	USART1_Report(0xA1,tbuf,12);                          //自定义帧,0xA1
}	

/**
  * @brief  发送解算后的姿态数据给电脑
  * @param  aacx,aacy,aacz:x,y,z三个方向上面的加速度值
  * @param  gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
  * @param  roll:横滚角.
  * @param  pitch:俯仰角.
  * @param  yaw:航向角.
  * @retval None
  */
void Report_imu(short aacx,short aacy,short aacz,short gyrox,short gyroy,short gyroz,short roll,short pitch,short yaw)
{
	u8 tbuf[28]; 
	u8 i;
	for(i=0;i<28;i++)tbuf[i]=0;
	tbuf[0]=(aacx>>8)&0xFF;
	tbuf[1]=aacx&0xFF;
	tbuf[2]=(aacy>>8)&0xFF;
	tbuf[3]=aacy&0xFF;
	tbuf[4]=(aacz>>8)&0xFF;
	tbuf[5]=aacz&0xFF; 
	tbuf[6]=(gyrox>>8)&0xFF;
	tbuf[7]=gyrox&0xFF;
	tbuf[8]=(gyroy>>8)&0xFF;
	tbuf[9]=gyroy&0xFF;
	tbuf[10]=(gyroz>>8)&0xFF;
	tbuf[11]=gyroz&0xFF;	
	tbuf[18]=(roll>>8)&0xFF;
	tbuf[19]=roll&0xFF;
	tbuf[20]=(pitch>>8)&0xFF;
	tbuf[21]=pitch&0xFF;
	tbuf[22]=(yaw>>8)&0xFF;
	tbuf[23]=yaw&0xFF;
	USART1_Report(0xAF,tbuf,28);//飞控显示帧,0xAF
} 
