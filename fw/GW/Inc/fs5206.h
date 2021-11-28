/************************************************************************
This is a library to handling capacitive multitouch sensors using FT5x06.
Originally written to work with ER-TFTM070-5 (LCD module) from EastRising.
Written by Helge Langehaug, February 2014
BSD license, all text above must be included in any redistribution
*************************************************************************/
#include "GUI.h"
#include <stdint.h>
#include "stm32f4xx.h"
/* FT5206 definitions */

#define CONFIG_FT5X0X_MULTITOUCH 

#define Key_Down 0x01
#define Key_Up   0x00 

struct sTouchEventTypeDef
{
    uint16_t    x1;
    uint16_t    y1;
    uint16_t    x2;
    uint16_t    y2;
    uint16_t    x3;
    uint16_t    y3;
    uint16_t    x4;
    uint16_t    y4;
    uint16_t    x5;
    uint16_t    y5;
    uint8_t     touch_point;
	uint8_t     key_sta; 
};



typedef struct 
{
	int16_t Min; 
	int16_t Max; 
	
} sMinMaxTypeDef;


extern sMinMaxTypeDef xyMinMax[2];
extern struct sTouchEventTypeDef ts_event;
extern GUI_PID_STATE State;

#define FT5206_WRITE_ADDRESS  0x70
#define FT5206_READ_ADDRESS   0x71


#define FT5206_I2C_ADDRESS 0x38
#define FT5206_NUMBER_OF_REGISTERS 31     // there are more registers, but this
                                          // is enought to get all 5 touch coordinates.
                                          
#define FT5206_NUMBER_OF_TOTAL_REGISTERS 0xfe

#define FT5206_DEVICE_MODE 0x00

#define FT5206_GEST_ID 				0x01
#define FT5206_GEST_ID_MOVE_UP    	0x10
#define FT5206_GEST_ID_MOVE_LEFT   	0x14
#define FT5206_GEST_ID_MOVE_DOWN   	0x18
#define FT5206_GEST_ID_MOVE_RIGHT  	0x1c
#define FT5206_GEST_ID_ZOOM_IN     	0x48
#define FT5206_GEST_ID_ZOOM_OUT    	0x49
#define FT5206_GEST_ID_NO_GESTURE  	0x00

#define FT5206_TD_STATUS 0x02

#define FT5206_TOUCH1_XH 0x03
#define FT5206_TOUCH1_XL 0x04
#define FT5206_TOUCH1_YH 0x05
#define FT5206_TOUCH1_YL 0x06

#define FT5206_TOUCH2_XH 0x09
#define FT5206_TOUCH2_XL 0x0a
#define FT5206_TOUCH2_YH 0x0b
#define FT5206_TOUCH2_YL 0x0c

#define FT5206_TOUCH3_XH 0x0f
#define FT5206_TOUCH3_XL 0x10
#define FT5206_TOUCH3_YH 0x11
#define FT5206_TOUCH3_YL 0x12

#define FT5206_TOUCH4_XH 0x15
#define FT5206_TOUCH4_XL 0x16
#define FT5206_TOUCH4_YH 0x17
#define FT5206_TOUCH4_YL 0x18

#define FT5206_TOUCH5_XH 0x1b
#define FT5206_TOUCH5_XL 0x1c
#define FT5206_TOUCH5_YH 0x1d
#define FT5206_TOUCH5_YL 0x1e

#define FS5206_TOUCH_LIB_VERSION_H 0xa1
#define FS5206_TOUCH_LIB_VERSION_L 0xa2

#define I2C1_TIMEOUT     		0x3000	// Timeout Zeit


void FS5206_Init(void);
void FS5206_GetRegInfo(uint8_t *reg);
uint8_t FS5206_GetTouchPos(uint16_t *touch_pos, uint8_t *reg);
uint8_t FS5206_ReadByte(uint8_t dev_address, uint8_t data_address);
uint8_t FS5206_WriteByte(uint8_t dev_address, uint8_t data_address, uint8_t data);
uint8_t FS5206_ReadBytes(uint8_t dev_address, uint8_t data_address, uint8_t *buff, uint8_t size);
uint8_t FS5206_WriteBytes(uint8_t dev_address, uint8_t data_address, uint8_t *buff, uint8_t size);
uint8_t FS5206_ReadXY(void);
void Calibration(int16_t x_size, int16_t y_size);
