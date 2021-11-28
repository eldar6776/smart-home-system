/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2016  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File        : NAND_HW_SPI_X.h
Purpose     : Hardware layer for SPI NAND devices
---------------------------END-OF-HEADER------------------------------
*/
#ifndef __NAND_HW_SPI_X_H__               // Avoid recursive and multiple inclusion
#define __NAND_HW_SPI_X_H__

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Global function prototypes
*
**********************************************************************
*/

/*********************************************************************
*
*       Initialisation and control functions
*/
int  FS_NAND_HW_SPI_X_Init     (U8 Unit);
void FS_NAND_HW_SPI_X_DisableCS(U8 Unit);
void FS_NAND_HW_SPI_X_EnableCS (U8 Unit);
void FS_NAND_HW_SPI_X_Delay    (U8 Unit, int ms);

/*********************************************************************
*
*       Data transfer functions
*/
int  FS_NAND_HW_SPI_X_Read     (U8 Unit,       void * pData, unsigned NumBytes);
int  FS_NAND_HW_SPI_X_Write    (U8 Unit, const void * pData, unsigned NumBytes);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // __NAND_HW_SPI_X_H__

/*************************** End of file ****************************/
