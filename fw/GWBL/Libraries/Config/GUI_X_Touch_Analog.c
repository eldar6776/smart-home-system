/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2012  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.16 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUI_TOUCH_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/
#include "GUI.h"
#include "resistive_touch.h"

//static void delay(__IO uint32_t nCount)
//{
//    __IO uint32_t index = 0;

//    for (index = 10000 * nCount; index != 0; index--)
//    {
//    }
//}
//int CalibrationComplete = 0;

void GUI_TOUCH_X_ActivateX(void) 
{
//	TSC_ChipSelected();
//	Touch_SPI_ReadWrite(0xdf);
	//TSC_ChipDeselected(); 
}


void GUI_TOUCH_X_ActivateY(void)
{
//	TSC_ChipSelected();
//	Touch_SPI_ReadWrite(0x9f);
	//TSC_ChipDeselected(); 
}


int  GUI_TOUCH_X_MeasureX(void) 
{
//	int an_x;

//	TSC_ChipSelected();
//	Touch_SPI_ReadWrite(0xd0);
//	an_x = Touch_SPI_ReadWrite(0x00);
//	an_x <<= 8;
//	an_x |= Touch_SPI_ReadWrite(0x00);
//	an_x >>= 4;
//	an_x &= 0x0fff;
//	TSC_ChipDeselected();
	return(0);
}


int  GUI_TOUCH_X_MeasureY(void) 
{
//	int an_y;

//	TSC_ChipSelected();
//	Touch_SPI_ReadWrite(0x90);
//	an_y = Touch_SPI_ReadWrite(0x00);
//	an_y <<= 8;
//	an_y |= Touch_SPI_ReadWrite(0x00);
//	an_y >>= 4;
//	an_y &= 0x0fff;
//	TSC_ChipDeselected();
	return(0);
}


