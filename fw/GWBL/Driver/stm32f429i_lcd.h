/**
  * @file    stm32f429i_lcd.h 
  * @author  WB R&D Team - openmcu666
  * @version V0.1
  * @date    2015.6.24
  * @brief   RGB LCD Driver
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F429I_LCD_H
#define __STM32F429I_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f429i_sdram.h"
#include "fonts.h"
//extern uint16_t address[];


typedef struct 
{
  int16_t X;
  int16_t Y;
} Point, * pPoint;   
/**
  * @}
  */ 


/* LCD Size (Width and Height) */
#define  LCD_PIXEL_WIDTH    	((uint16_t)480)
#define  LCD_PIXEL_HEIGHT   	((uint16_t)272)

#define LCD_FRAME_BUFFER      	((uint32_t)0xD0000000) //(uint32_t)(&address[0])
//#define BUFFER_OFFSET         	((uint32_t)0x28000)//320x240
#define BUFFER_OFFSET         	((uint32_t)0xC0000)//480*272*2
//#define LCD_FRAME_BUFFER_test	(uint32_t)(&address[0])
/**
 * @brief Uncomment the line below if you want to use user defined Delay function
 *        (for precise timing), otherwise default _delay_ function defined within
 *         this driver is used (less precise timing).  
 */
/* #define USE_Delay */

#ifdef USE_Delay
#include "main.h" 
  #define _delay_     Delay  /* !< User can provide more timing precise _delay_ function
                                   (with 10ms time base), using SysTick for example */
#else
  #define _delay_     delay      /* !< Default _delay_ function with less precise timing */
#endif

/** 
  * @brief  LCD color  
  */ 

#define  	RED          	0xF800	  
#define 	GREEN        	0x07E0
#define 	BLUE         	0x001F
#define 	WHITE        	0xFFFF
#define 	GRAY  			0x8430
#define 	LGRAY        	0xC618
#define 	DARKGRAY     	0x8410
#define 	PORPO        	0x801F
#define 	PINK         	0xF81F
#define 	GRAYBLUE     	0x5458 
#define		LGRAYBLUE    	0xA651
#define 	DARKBLUE     	0x01CF	
#define		LIGHTBLUE    	0x7D7C	
#define  	BLACK        	0x0000	
#define 	YELLOW       	0xFFE0
#define  	ORANGE       	0xFC08

static sFONT *Efont;
/** 
  * @brief  LCD Lines depending on the chosen fonts.  
  */
#define LCD_LINE_0               LINE(0)
#define LCD_LINE_1               LINE(1)
#define LCD_LINE_2               LINE(2)
#define LCD_LINE_3               LINE(3)
#define LCD_LINE_4               LINE(4)
#define LCD_LINE_5               LINE(5)
#define LCD_LINE_6               LINE(6)
#define LCD_LINE_7               LINE(7)
#define LCD_LINE_8               LINE(8)
#define LCD_LINE_9               LINE(9)
#define LCD_LINE_10              LINE(10)
#define LCD_LINE_11              LINE(11)
#define LCD_LINE_12              LINE(12)
#define LCD_LINE_13              LINE(13)
#define LCD_LINE_14              LINE(14)
#define LCD_LINE_15              LINE(15)
#define LCD_LINE_16              LINE(16)
#define LCD_LINE_17              LINE(17)
#define LCD_LINE_18              LINE(18)
#define LCD_LINE_19              LINE(19)
#define LCD_LINE_20              LINE(20)
#define LCD_LINE_21              LINE(21)
#define LCD_LINE_22              LINE(22)
#define LCD_LINE_23              LINE(23)
#define LCD_LINE_24              LINE(24)
#define LCD_LINE_25              LINE(25)
#define LCD_LINE_26              LINE(26)
#define LCD_LINE_27              LINE(27)
#define LCD_LINE_28              LINE(28)
#define LCD_LINE_29              LINE(29)
#define LCD_LINE_30              LINE(30)
#define LCD_LINE_31              LINE(31)
#define LCD_LINE_32              LINE(32)
#define LCD_LINE_33              LINE(33)
#define LCD_LINE_34              LINE(34)
#define LCD_LINE_35              LINE(35)
#define LCD_LINE_36              LINE(36)
#define LCD_LINE_37              LINE(37)
#define LCD_LINE_38              LINE(38)
#define LCD_LINE_39              LINE(39)

/** 
  * @brief LCD default font 
  */ 
#define LCD_DEFAULT_FONT         Font16x24

/** 
  * @brief  LCD Direction  
  */ 
#define LCD_DIR_HORIZONTAL       0x0000
#define LCD_DIR_VERTICAL         0x0001

/**
  * @}
  */ 

/** 
  * @brief  LCD Layer  
  */ 
#define LCD_BACKGROUND_LAYER     0x0000
#define LCD_FOREGROUND_LAYER     0x0001

/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Exported_Macros
  * @{
  */ 
#define ASSEMBLE_RGB(R, G, B)    ((((R)& 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))  

/**
  * @}
  */ 

void     	LCD_DeInit(void);   
void     	LCD_Init(void);
void     	LCD_LayerInit(void);
void     	LCD_ChipSelect(FunctionalState NewState);
void     	LCD_SetLayer(uint32_t Layerx);
void     	LCD_SetColors(uint16_t _TextColor, uint16_t _BackColor); 
void     	LCD_GetColors(uint16_t *_TextColor, uint16_t *_BackColor);
void     	LCD_SetTextColor(uint16_t Color);
void     	LCD_SetBackColor(uint16_t Color);
void     	LCD_SetTransparency(uint8_t transparency);
void     	LCD_ClearLine(uint16_t Line);
void     	LCD_Clear(uint16_t Color);
uint32_t 	LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     	LCD_SetColorKeying(uint32_t RGBValue);
void     	LCD_ReSetColorKeying(void);
void     	LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c);
void     	LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii);
void     	LCD_SetFont(sFONT *fonts);
sFONT *  	LCD_GetFont(void);
void     	LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr);
void     	LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width);
void     	LCD_WindowModeDisable(void);
void     	LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction);
void     	LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width);
void     	LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void     	LCD_DrawEllipse(int Xpos, int Ypos, int Radius, int Radius2);
void     	LCD_DrawFullEllipse(int Xpos, int Ypos, int Radius, int Radius2);
void     	LCD_DrawMonoPict(const uint32_t *Pict);
void     	LCD_WriteBMP(uint32_t BmpAddress);
void     	LCD_DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void     	LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     	LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void     	LCD_PolyLine(pPoint Points, uint16_t PointCount);
void     	LCD_PolyLineRelative(pPoint Points, uint16_t PointCount);
void     	LCD_ClosedPolyLine(pPoint Points, uint16_t PointCount);
void     	LCD_ClosedPolyLineRelative(pPoint Points, uint16_t PointCount);
void     	LCD_FillPolyLine(pPoint Points, uint16_t PointCount);
void     	LCD_Triangle(pPoint Points, uint16_t PointCount);
void     	LCD_FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3);
void     	LCD_WriteCommand(uint8_t LCD_Reg);
void     	LCD_WriteData(uint8_t value);
void     	LCD_PowerOn(void);
void     	LCD_DisplayOn(void);
void     	LCD_DisplayOff(void);
void     	LCD_CtrlLinesConfig(void);
void     	LCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins, BitAction BitVal);
void     	LCD_SPIConfig(void);
void		PutPixelColor(int16_t x, int16_t y,uint16_t Color);
void 		LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size,uint16_t PenColor, uint16_t BackColor);
void 		LCD_DisplayStr(uint16_t x, uint16_t y, uint8_t * s, uint16_t PenColor, uint16_t BackColor, uint8_t font);
void 		LCD_AF_GPIOConfig(void);
/**
  * @}
  */    
  
#ifdef __cplusplus
}
#endif

#endif 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */
  

