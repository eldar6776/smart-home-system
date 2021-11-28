/**
  * @file    RTP_Touch.c 
  * @author  WB R&D Team - openmcu666
  * @version V0.1
  * @date    2015.7.13
  * @brief   Texas Instruments TSC2046 (ADS7846) touch screen controller driver
  */

#include "resistive_touch.h"
#include "GUI.h"


static void Touch_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(TOUCH_RCC_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOD|
						   RCC_AHB1Periph_GPIOI, ENABLE);

	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource4,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_PinAFConfig(TOUCH_MISO_GPIO, TOUCH_MISO_GPIO_SOURCE, TOUCH_GPIO_AF);
	GPIO_InitStructure.GPIO_Pin = TOUCH_MISO_GPIO_PIN;
	GPIO_Init(TOUCH_MISO_GPIO, &GPIO_InitStructure);

	GPIO_PinAFConfig(TOUCH_MOSI_GPIO, TOUCH_MOSI_GPIO_SOURCE, TOUCH_GPIO_AF);
	GPIO_InitStructure.GPIO_Pin = TOUCH_MOSI_GPIO_PIN;
	GPIO_Init(TOUCH_MOSI_GPIO, &GPIO_InitStructure);

	GPIO_PinAFConfig(TOUCH_SCK_GPIO, TOUCH_SCK_GPIO_SOURCE, TOUCH_GPIO_AF);
	GPIO_InitStructure.GPIO_Pin = TOUCH_SCK_GPIO_PIN;
	GPIO_Init(TOUCH_SCK_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = TOUCH_CS_GPIO_PIN;
	GPIO_Init(TOUCH_CS_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = TOUCH_BUSY_GPIO_PIN;
	GPIO_Init(TOUCH_BUSY_GPIO, &GPIO_InitStructure);
}



static void Touch_SPI_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB1PeriphClockCmd(TOUCH_RCC_CLK, ENABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(TOUCH_SPI, &SPI_InitStructure); 
	SPI_Cmd(TOUCH_SPI, ENABLE);	
}


uint8_t Touch_SPI_ReadWrite(uint8_t Byte)
{
	while (SPI_I2S_GetFlagStatus(TOUCH_SPI, SPI_I2S_FLAG_TXE) == RESET);	// Loop while DR register in not emplty 
	SPI_I2S_SendData(TOUCH_SPI, Byte);	                                 	// Send byte through the SPI1 peripheral 
	while (SPI_I2S_GetFlagStatus(TOUCH_SPI, SPI_I2S_FLAG_RXNE) == RESET);	// Wait to receive a byte 
	return SPI_I2S_ReceiveData(TOUCH_SPI);                               	// Return the byte read from the SPI bus
}


void TOUCH_Init(void)
{
	Touch_GPIO_Init();
	Touch_SPI_Init();
}


//uint8_t Touch_Detected(void)
//{
//	return GPIO_ReadInputDataBit(TOUCH_BUSY_GPIO, TOUCH_BUSY_GPIO_PIN);
//}

//uint16_t Touch_ReadAD(uint8_t Cmd)
//{
//	u16 Num,Date; 

//	TSC_ChipSelected();
//	Touch_SPI_ReadWrite(Cmd);
//	Num=0;
//	Date=0;
//	Delay();

//	Num = Touch_SPI_ReadWrite(0x00);
//	Date = Num << 8 ;
//	Num |= Touch_SPI_ReadWrite(0x00);
//	Date |= Num; 
//	Date >>=4;
//	Date &=0xFFF;

//	TSC_ChipDeselected();  
//	return (Date);	  
//}

///**
//  * @brief  读取XPT2046采集的X轴AD值
//  * @param  None
//  * @retval 采集的AD值
//  */
//uint16_t Get_TouchADX(void)
//{
//	uint32_t sum = 0;
//	uint32_t i;

//	for (i = 0; i < 32; i++) 
//	{
//		sum += Touch_ReadAD(TOUCH_CMD_X);
//		Delay();
//	}
//	sum >>= 5;

//	return ((uint16_t)sum);
//}

///**
//  * @brief  读取XPT2046采集的Y轴AD值
//  * @param  None
//  * @retval 采集的AD值
//  */
//uint16_t Get_TouchADY(void)
//{
//	uint32_t sum;
//	uint32_t i;

//	for (i = 0; i < 32; i++) 
//	{
//		sum += Touch_ReadAD(TOUCH_CMD_Y);
//		Delay();
//	}
//	sum >>= 5;

//	return ((uint16_t)sum);
//}

///**
//  * @brief  读取X Y的AD值
//  * @param  x,y:指向读取缓冲区
//  * @retval 1:读数成功
//  */
//u8 Read_ADS(u16 *x,u16 *y)
//{
//	u16 xtemp,ytemp;		
//	xtemp=Get_TouchADX();
//	ytemp=Get_TouchADY();	
//	xtemp=Touch_ReadAD(TOUCH_CMD_X);
//	ytemp=Touch_ReadAD(TOUCH_CMD_Y);	

//	*x=xtemp;
//	*y=ytemp;
//	return 1;//读数成功
//}


///*********************************************************************
//*
//*       _CheckUpdateTouch
//*
//* Function description
//*   Reads touch values from ADC on Truly LCD display
//*   controller IC on Embedded Artists QVGA Base Board.
//*   Checks if a touch event has occurred. If we found an event the
//*   static variables for x and y will be filled with new values that
//*   can be processed via GUI touch routines.
//*
//* Return value:
//*      0  No touch event, x, y have not been updated
//*   != 0  Touch event occurred, x, y have been updated and return value is read z value
//*/
//Coordinate *_CheckUpdateTouch(void)
//{
//	u16 m0,m1,m2,TP_X[1],TP_Y[1],temp[3];
//	u8 count=0;
//	
//	int buffer[2][9]={{0},{0}};
//	
//	do					      
//	{		    
//		Read_ADS(TP_X,TP_Y);	
//		buffer[0][count]=TP_X[0];  
//		buffer[1][count]=TP_Y[0];
//		count++;  
//	}
//	
//	while(!T_INT && count<9);
//	
//	if(count==9)
//	{  
//		temp[0]=(buffer[0][0]+buffer[0][1]+buffer[0][2])/3;
//		temp[1]=(buffer[0][3]+buffer[0][4]+buffer[0][5])/3;
//		temp[2]=(buffer[0][6]+buffer[0][7]+buffer[0][8])/3;
//		
//		m0=temp[0]-temp[1];
//		m1=temp[1]-temp[2];
//		m2=temp[2]-temp[0];
//		
//		m0=m0>0?m0:(-m0);
//		m1=m1>0?m1:(-m1);
//		m2=m2>0?m2:(-m2);
//		
//		if( m0>THRESHOLD  &&  m1>THRESHOLD  &&  m2>THRESHOLD ) return 0;
//		
//		if(m0<m1)
//		{
//			if(m2<m0) 
//			screen.x=(temp[0]+temp[2])/2;
//			else 
//			screen.x=(temp[0]+temp[1])/2;	
//		}
//		else if(m2<m1) 
//			screen.x=(temp[0]+temp[2])/2;
//		else 
//			screen.x=(temp[1]+temp[2])/2;

//		
//		temp[0]=(buffer[1][0]+buffer[1][1]+buffer[1][2])/3;
//		temp[1]=(buffer[1][3]+buffer[1][4]+buffer[1][5])/3;
//		temp[2]=(buffer[1][6]+buffer[1][7]+buffer[1][8])/3;
//		m0=temp[0]-temp[1];
//		m1=temp[1]-temp[2];
//		m2=temp[2]-temp[0];
//		m0=m0>0?m0:(-m0);
//		m1=m1>0?m1:(-m1);
//		m2=m2>0?m2:(-m2);
//		if(m0>THRESHOLD&&m1>THRESHOLD&&m2>THRESHOLD) return 0;

//		if(m0<m1)
//		{
//			if(m2<m0) 
//				screen.y=(temp[0]+temp[2])/2;
//			else 
//				screen.y=(temp[0]+temp[1])/2;	
//		}
//		else if(m2<m1) 
//			screen.y=(temp[0]+temp[2])/2;
//		else
//			screen.y=(temp[1]+temp[2])/2;

//		return (&screen);
//	}  
//	return 0; 
//}

////校准
//int perform_calibration(calibration *cal) 
//{
//	int j;
//	float n, x, y, x2, y2, xy, z, zx, zy;
//	float det, a, b, c, e, f, i;
//	float scaling = 65536.0;

//// Get sums for matrix
//	n = x = y = x2 = y2 = xy = 0;
//	
//	for(j=0;j<5;j++) 
//	{
//		n += 1.0f;
//		x += (float)cal->x[j];
//		y += (float)cal->y[j];
//		x2 += (float)(cal->x[j]*cal->x[j]);
//		y2 += (float)(cal->y[j]*cal->y[j]);
//		xy += (float)(cal->x[j]*cal->y[j]);
//	}

//// Get determinant of matrix -- check if determinant is too small
//	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
//	if(det < 0.1f && det > -0.1f) {
//	//	printf("ts_calibrate: determinant is too small -- %f\n\r",det);
//		return 0;
//	}

//// Get elements of inverse matrix
//	a = (x2*y2 - xy*xy)/det;
//	b = (xy*y - x*y2)/det;
//	c = (x*xy - y*x2)/det;
//	e = (n*y2 - y*y)/det;
//	f = (x*y - n*xy)/det;
//	i = (n*x2 - x*x)/det;

//// Get sums for x calibration
//	z = zx = zy = 0;
//	for(j=0;j<5;j++) {
//		z += (float)cal->xfb[j];
//		zx += (float)(cal->xfb[j]*cal->x[j]);
//		zy += (float)(cal->xfb[j]*cal->y[j]);
//	}

//// Now multiply out to get the calibration for framebuffer x coord
//	cal->a[2] = (int)((a*z + b*zx + c*zy)*(scaling));
//	cal->a[0] = (int)((b*z + e*zx + f*zy)*(scaling));
//	cal->a[1] = (int)((c*z + f*zx + i*zy)*(scaling));
//	
////	cal->a[2] = 0xffe46b34;
////	cal->a[0] = 0x00006ab8;
////	cal->a[1] = 0x00000055;

//// 	printf("%f %f %f\n\r",(a*z + b*zx + c*zy),
//// 				(b*z + e*zx + f*zy),
//// 				(c*z + f*zx + i*zy));

//// Get sums for y calibration
//	z = zx = zy = 0;
//	for(j=0;j<5;j++) {
//		z += (float)cal->yfb[j];
//		zx += (float)(cal->yfb[j]*cal->x[j]);
//		zy += (float)(cal->yfb[j]*cal->y[j]);
//	}

//// Now multiply out to get the calibration for framebuffer y coord
//	cal->a[5] = (int)((a*z + b*zx + c*zy)*(scaling));
//	cal->a[3] = (int)((b*z + e*zx + f*zy)*(scaling));
//	cal->a[4] = (int)((c*z + f*zx + i*zy)*(scaling));
////	cal->a[5] = 0x01f1d1b8;
////	cal->a[3] = 0x0000006a;
////	cal->a[4] = 0xffffbc93;

//// 	printf("%f %f %f\n\r",(a*z + b*zx + c*zy),
//// 				(b*z + e*zx + f*zy),
//// 				(c*z + f*zx + i*zy));

//// If we got here, we're OK, so assign scaling to a[6] and return
//	cal->a[6] = (int)scaling;
////	cal->a[6] = 0x00010000;
//	return 1;
//}

//typedef struct {int Min; int Max; } tMinMax;
//extern tMinMax xyMinMax[2];
////extern int CalibrationComplete;
//int ax_Phys[2],ay_Phys[2];

//void my_ExecCalibration( int x_size, int y_size )
//{
//	/* calculate log. Positions */
//	int ax[5], ay[5];
//	Coordinate *p;
//	ax[0] = 20;             ay[0] = 20;
//	ax[1] = x_size -20;     ay[1] = 20;
//	ax[2] = x_size -20;     ay[2] = y_size-20;
//	ax[3] = 20;             ay[3] = y_size-20;
//	ax[4] = x_size/2;       ay[4] = y_size/2;

//	GUI_SetFont(&GUI_Font13_ASCII);
//	//GUI_SetBkColor(GUI_BLACK);  
//	GUI_Clear();
//	GUI_SetColor(GUI_WHITE);  GUI_FillCircle(ax[0], ay[0], 10);//绘制填充的圆
//	GUI_SetColor(GUI_BLACK);    GUI_FillCircle(ax[0], ay[0], 5);
//	GUI_SetColor(GUI_WHITE);
//	GUI_DispStringAt("Press here", ax[0]+20, ay[0]);
//	
//	do {
//		p=_CheckUpdateTouch();
//		if (p!=(void*)0) 
//		{
//		  cal.xfb[0] = 20;
//		  cal.yfb[0] = 20;
//		  cal.x[0] = p->x;//111
//		  cal.y[0] = p->y;//1809
//			break;
//		}
//		Delay();
//	} while (1); 
//	
//	GUI_Clear();
//	GUI_DispStringAt("OK", ax[0]+20, ay[0]);	
//	while(!T_INT)
//	//GUI_Delay(300);
//	//GUI_SetBkColor(GUI_BLACK);  
//	GUI_Clear();
//	GUI_SetColor(GUI_WHITE);  GUI_FillCircle(ax[1], ay[1], 10);
//	GUI_SetColor(GUI_BLACK);    GUI_FillCircle(ax[1], ay[1], 5);
//	GUI_SetColor(GUI_WHITE);
//	GUI_SetTextAlign(GUI_TA_RIGHT);
//	GUI_DispStringAt("Press here", ax[1]-20, ay[1]);
//	
//	do {
//		p=_CheckUpdateTouch();
//		
//		if (p!=(void*)0) 
//		{
//			cal.xfb[1] = x_size-20;
//			cal.yfb[1] = 20;
//			cal.x[1] = p->x ;//1922
//			cal.y[1] = p->y;//1828
//			break;
//		}
//		Delay();
//	} while (1);
//	
//	GUI_Clear();
//	GUI_DispStringAt("OK", ax[1]-20, ay[1]);	
//	while(!T_INT) Delay();
//	GUI_Clear();
//	GUI_SetColor(GUI_WHITE);  GUI_FillCircle(ax[2], ay[2], 10);
//	GUI_SetColor(GUI_BLACK);    GUI_FillCircle(ax[2], ay[2], 5);
//	GUI_SetColor(GUI_WHITE);
//	GUI_SetTextAlign(GUI_TA_RIGHT);
//	GUI_DispStringAt("Press here", ax[2]-20, ay[2]);
//	
//	do {
//		p=_CheckUpdateTouch();
//		if (p!=(void*)0) 
//		{
//			cal.xfb[2] = x_size -20;
//			cal.yfb[2] = y_size -20;
//			cal.x[2] = p->x;//1821
//			cal.y[2] = p->y;//183
//			break;           
//		}
//		Delay();
//	} while (1);
//	
//	GUI_Clear();
//	GUI_DispStringAt("OK", ax[2]-20, ay[2]);	
//	while(!T_INT) Delay();
//	GUI_Clear();
//	GUI_SetColor(GUI_WHITE);  GUI_FillCircle(ax[3], ay[3], 10);
//	GUI_SetColor(GUI_BLACK);    GUI_FillCircle(ax[3], ay[3], 5);
//	GUI_SetColor(GUI_WHITE);
//	//GUI_SetTextAlign(GUI_TA_RIGHT);
//	GUI_SetTextAlign(GUI_TA_LEFT);
//	GUI_DispStringAt("Press here", ax[3]+20, ay[3]);
//	
//	do {
//		p=_CheckUpdateTouch();
//		
//		if (p!=(void*)0) 
//		{
//			cal.xfb[3] = 20;
//			cal.yfb[3] = y_size -20;
//			cal.x[3] = p->x ;//161
//			cal.y[3] = p->y ;//188
//			break;
//		}
//		Delay();
//	} while (1);
//	
//	GUI_Clear();
//	GUI_DispStringAt("OK", ax[3]+20, ay[3]);	
//	while(!T_INT) Delay();
//	GUI_Clear();
//	GUI_SetColor(GUI_WHITE);  GUI_FillCircle(ax[4], ay[4], 10);
//	GUI_SetColor(GUI_BLACK);   GUI_FillCircle(ax[4], ay[4], 5);
//	GUI_SetColor(GUI_WHITE);
//	GUI_SetTextAlign(GUI_TA_LEFT);
//	GUI_DispStringAt("Press here", ax[4]+20, ay[4]);
//	
//	do {
//		p=_CheckUpdateTouch();
//		
//		if (p!=(void*)0)  
//		{
//			cal.xfb[4] = x_size/2;
//			cal.yfb[4] = y_size/2;
//			cal.x[4] = p->x ;//1015
//			cal.y[4] = p->y ;//976
//			break;
//		}
//		Delay();
//	} while (1);

//	/* calculate and display values for configuration file */  
//	{ 
//		GUI_Clear();
//		GUI_DispString  ("x0: ");	GUI_DispDec(cal.x[0], 4);	
//		GUI_DispString  ("  y0: ");	GUI_DispDec(cal.y[0], 4);   GUI_DispNextLine();

//		GUI_DispString  ("x1: ");	GUI_DispDec(cal.x[1], 4);
//		GUI_DispString  ("  y1: ");	GUI_DispDec(cal.y[1], 4);	GUI_DispNextLine();

//		GUI_DispString  ("x2: ");	GUI_DispDec(cal.x[2], 4);
//		GUI_DispString  ("  y2: ");	GUI_DispDec(cal.y[2], 4);	GUI_DispNextLine();

//		GUI_DispString  ("x3: ");	GUI_DispDec(cal.x[3], 4);
//		GUI_DispString  ("  y3: ");	GUI_DispDec(cal.y[3], 4);	GUI_DispNextLine();

//		GUI_DispString  ("x4: ");	GUI_DispDec(cal.x[4], 4);
//		GUI_DispString  ("  y4: ");	GUI_DispDec(cal.y[4], 4);	GUI_DispNextLine();

//		GUI_DispString  ("Please touch display to continue...");

//		do {
//			p=_CheckUpdateTouch();		
//			if (p!=(void*)0) break;
//			Delay();
//		} while (1);
//	}
//	perform_calibration(&cal);
//}

//int  Calibrate_X(unsigned  int ad_x,unsigned int ad_y)
//{
//        int temp;
//        temp =(unsigned int)((ad_x*cal.a[0]+ad_y*cal.a[1]+cal.a[2])/cal.a[6]); 
//        return temp;     //屏幕X
//}
//int  Calibrate_Y(unsigned  int ad_x,unsigned int ad_y)
//{
//        int temp;
//        temp =(unsigned int)((ad_x*cal.a[3]+ad_y*cal.a[4]+cal.a[5])/cal.a[6]); 
//        return temp;
//}
