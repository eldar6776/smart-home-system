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
File        : NAND_X_HW.h
Purpose     : Generic header file for the HW layer of NAND flash driver
---------------------------END-OF-HEADER------------------------------
*/
#ifndef NAND_X_HW_H               // Avoid recursive and multiple inclusion
#define NAND_X_HW_H

#include "SEGGER.h"

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
*       Initialization and control functions
*/
void FS_NAND_HW_X_Init_x8         (U8 Unit);
void FS_NAND_HW_X_Init_x16        (U8 Unit);
void FS_NAND_HW_X_DisableCE       (U8 Unit);
void FS_NAND_HW_X_EnableCE        (U8 Unit);

/*********************************************************************
*
*       Access mode functions
*/
void FS_NAND_HW_X_SetAddrMode     (U8 Unit);
void FS_NAND_HW_X_SetCmdMode      (U8 Unit);
void FS_NAND_HW_X_SetDataMode     (U8 Unit);

/*********************************************************************
*
*       Status checking functions
*/
int  FS_NAND_HW_X_WaitWhileBusy   (U8 Unit, unsigned us);

/*********************************************************************
*
*       Data transfer functions
*/
void FS_NAND_HW_X_Read_x8         (U8 Unit,       void * pBuffer, unsigned NumBytes);
void FS_NAND_HW_X_Write_x8        (U8 Unit, const void * pBuffer, unsigned NumBytes);
void FS_NAND_HW_X_Read_x16        (U8 Unit,       void * pBuffer, unsigned NumBytes);
void FS_NAND_HW_X_Write_x16       (U8 Unit, const void * pBuffer, unsigned NumBytes);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // NAND_X_HW_H

/*************************** End of file ****************************/
