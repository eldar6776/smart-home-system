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
File        : FS_DF_X_HW.h
Purpose     : Hardware layer for DataFlash.
---------------------------END-OF-HEADER------------------------------
*/
#ifndef __FS_DF_X_HW_H__               // Avoid recursive and multiple inclusion
#define __FS_DF_X_HW_H__

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
int           FS_DF_HW_X_Init       (U8 Unit);
void          FS_DF_HW_X_EnableCS   (U8 Unit);
void          FS_DF_HW_X_DisableCS  (U8 Unit);

/*********************************************************************
*
*       Data transfer functions
*/
void          FS_DF_HW_X_Read       (U8 Unit,       U8 * pData, int NumBytes);
void          FS_DF_HW_X_Write      (U8 Unit, const U8 * pData, int NumBytes);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // __FS_DF_X_HW_H__

/*************************** End of file ****************************/
