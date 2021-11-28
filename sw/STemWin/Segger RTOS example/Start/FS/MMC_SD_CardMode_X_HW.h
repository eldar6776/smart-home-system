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
File        : MMC_SD_CardMode_X_HW.h
Purpose     : MMC hardware layer
---------------------------END-OF-HEADER------------------------------
*/
#ifndef __MMC_SD_CARDMODE_X_HW_H__               // Avoid recursive and multiple inclusion
#define __MMC_SD_CARDMODE_X_HW_H__

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/

/*********************************************************************
*
*       Error codes
*/
#define FS_MMC_CARD_NO_ERROR                0
#define FS_MMC_CARD_RESPONSE_TIMEOUT        1
#define FS_MMC_CARD_RESPONSE_CRC_ERROR      2
#define FS_MMC_CARD_READ_TIMEOUT            3
#define FS_MMC_CARD_READ_CRC_ERROR          4
#define FS_MMC_CARD_WRITE_CRC_ERROR         5
#define FS_MMC_CARD_RESPONSE_GENERIC_ERROR  6
#define FS_MMC_CARD_READ_GENERIC_ERROR      7
#define FS_MMC_CARD_WRITE_GENERIC_ERROR     8

/*********************************************************************
*
*       Response types
*/
#define FS_MMC_RESPONSE_FORMAT_NONE         0
#define FS_MMC_RESPONSE_FORMAT_R1           1
#define FS_MMC_RESPONSE_FORMAT_R2           2
#define FS_MMC_RESPONSE_FORMAT_R3           3
#define FS_MMC_RESPONSE_FORMAT_R6           FS_MMC_RESPONSE_FORMAT_R1   // Response format R6 has the same number of bits as R1.
#define FS_MMC_RESPONSE_FORMAT_R7           FS_MMC_RESPONSE_FORMAT_R1   // Response format R7 has the same number of bits as R1.

/*********************************************************************
*
*       Command flags
*/
#define FS_MMC_CMD_FLAG_DATATRANSFER              (1 <<  0)
#define FS_MMC_CMD_FLAG_WRITETRANSFER             (1 <<  1)
#define FS_MMC_CMD_FLAG_SETBUSY                   (1 <<  2)
#define FS_MMC_CMD_FLAG_INITIALIZE                (1 <<  3)
#define FS_MMC_CMD_FLAG_USE_SD4MODE               (1 <<  4)
#define FS_MMC_CMD_FLAG_STOP_TRANS                (1 <<  5)
#define FS_MMC_CMD_FLAG_REPEAT_SAME_SECTOR_DATA   (1 <<  6)
#define FS_MMC_CMD_FLAG_USE_MMC8MODE              (1 <<  7)

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
void   FS_MMC_HW_X_InitHW             (U8 Unit);
void   FS_MMC_HW_X_Delay              (int ms);

/*********************************************************************
*
*       Card status functions
*/
int    FS_MMC_HW_X_IsPresent          (U8 Unit);
int    FS_MMC_HW_X_IsWriteProtected   (U8 Unit);

/*********************************************************************
*
*       Configuration functions
*/
U16    FS_MMC_HW_X_SetMaxSpeed        (U8 Unit, U16 Freq);
void   FS_MMC_HW_X_SetResponseTimeOut (U8 Unit, U32 Value);
void   FS_MMC_HW_X_SetReadDataTimeOut (U8 Unit, U32 Value);

/*********************************************************************
*
*       Command execution functions
*/
void   FS_MMC_HW_X_SendCmd            (U8 Unit, unsigned Cmd, unsigned CmdFlags, unsigned ResponseType, U32 Arg);
int    FS_MMC_HW_X_GetResponse        (U8 Unit, void *pBuffer, U32 Size);

/*********************************************************************
*
*       Data transfer functions
*/
int    FS_MMC_HW_X_ReadData           (U8 Unit,       void * pBuffer, unsigned NumBytes, unsigned NumBlocks);
int    FS_MMC_HW_X_WriteData          (U8 Unit, const void * pBuffer, unsigned NumBytes, unsigned NumBlocks);
void   FS_MMC_HW_X_SetDataPointer     (U8 Unit, const void * p);
void   FS_MMC_HW_X_SetHWBlockLen      (U8 Unit, U16 BlockSize);
void   FS_MMC_HW_X_SetHWNumBlocks     (U8 Unit, U16 NumBlocks);

/*********************************************************************
*
*       Query functions
*/
U16    FS_MMC_HW_X_GetMaxReadBurst    (U8 Unit);
U16    FS_MMC_HW_X_GetMaxWriteBurst   (U8 Unit);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // __MMC_SD_CARDMODE_X_HW_H__

/*************************** End of file ****************************/
