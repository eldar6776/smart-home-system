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
File        : NOR_SPI_Dev.h
Purpose     : Declarations related to serial NOR flash devices
---------------------------END-OF-HEADER------------------------------
*/

#ifndef NOR_SPI_DEV_H                 // Avoid recursive and multiple inclusion
#define NOR_SPI_DEV_H

#include "SEGGER.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#ifndef   FS_NOR_MAX_SECTOR_BLOCKS
  #define FS_NOR_MAX_SECTOR_BLOCKS  5       // Worst (known) case serial NOR device has 5 sector blocks.
                                            // Typical serial NOR devices have only 1 (uniform sectors)
#endif

/*********************************************************************
*
*       Bus width handling
*/
#define FS_NOR_BUSWIDTH_CMD_SHIFT             8
#define FS_NOR_BUSWIDTH_ADDR_SHIFT            4
#define FS_NOR_BUSWIDTH_DATA_SHIFT            0
#define FS_NOR_BUSWIDTH_MASK                  0x0F
#define FS_NOR_BUSWIDTH_CMD_1BIT              (1u << FS_NOR_BUSWIDTH_CMD_SHIFT)
#define FS_NOR_BUSWIDTH_CMD_2BIT              (2u << FS_NOR_BUSWIDTH_CMD_SHIFT)
#define FS_NOR_BUSWIDTH_CMD_4BIT              (4u << FS_NOR_BUSWIDTH_CMD_SHIFT)
#define FS_NOR_BUSWIDTH_ADDR_1BIT             (1u << FS_NOR_BUSWIDTH_ADDR_SHIFT)
#define FS_NOR_BUSWIDTH_ADDR_2BIT             (2u << FS_NOR_BUSWIDTH_ADDR_SHIFT)
#define FS_NOR_BUSWIDTH_ADDR_4BIT             (4u << FS_NOR_BUSWIDTH_ADDR_SHIFT)
#define FS_NOR_BUSWIDTH_DATA_1BIT             (1u << FS_NOR_BUSWIDTH_DATA_SHIFT)
#define FS_NOR_BUSWIDTH_DATA_2BIT             (2u << FS_NOR_BUSWIDTH_DATA_SHIFT)
#define FS_NOR_BUSWIDTH_DATA_4BIT             (4u << FS_NOR_BUSWIDTH_DATA_SHIFT)
#define FS_NOR_MAKE_BUSWIDTH(Cmd, Addr, Data) (((Cmd) << FS_NOR_BUSWIDTH_CMD_SHIFT) | ((Addr) << FS_NOR_BUSWIDTH_ADDR_SHIFT) | ((Data) << FS_NOR_BUSWIDTH_DATA_SHIFT))
#define FS_NOR_GET_BUSWIDTH_CMD(BusWidth)     (((BusWidth) >> FS_NOR_BUSWIDTH_CMD_SHIFT)  & FS_NOR_BUSWIDTH_MASK)
#define FS_NOR_GET_BUSWIDTH_ADDR(BusWidth)    (((BusWidth) >> FS_NOR_BUSWIDTH_ADDR_SHIFT) & FS_NOR_BUSWIDTH_MASK)
#define FS_NOR_GET_BUSWIDTH_DATA(BusWidth)    (((BusWidth) >> FS_NOR_BUSWIDTH_DATA_SHIFT) & FS_NOR_BUSWIDTH_MASK)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_NOR_SPI_CMD
*/
typedef struct FS_NOR_SPI_CMD {
  void (*pfControl)          (void * pContext, U8 Cmd, U16 BusWidth);
  void (*pfWriteData)        (void * pContext, U8 Cmd, const U8 * pData, unsigned NumBytes, U16 BusWidth);
  void (*pfReadData)         (void * pContext, U8 Cmd,       U8 * pData, unsigned NumBytes, U16 BusWidth);
  void (*pfWriteDataWithAddr)(void * pContext, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr, const U8 * pData, unsigned NumBytesData, U16 BusWidth);
  void (*pfReadDataWithAddr) (void * pContext, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr,       U8 * pData, unsigned NumBytesData, U16 BusWidth);
} FS_NOR_SPI_CMD;

/*********************************************************************
*
*       FS_NOR_SPI_SECTOR_BLOCK
*/
typedef struct FS_NOR_SPI_SECTOR_BLOCK {
  U8  ldBytesPerSector;         // Number of bytes in the physical sector
  U8  CmdErase;                 // Command to erase the physical sector
  U16 NumSectors;               // Total number of physical sectors
} FS_NOR_SPI_SECTOR_BLOCK;

/*********************************************************************
*
*       FS_NOR_SPI_INST
*/
typedef struct FS_NOR_SPI_INST {
  U8                        NumBytesAddr;            // Number of address bytes (4 bytes for capacity > 128MBit, else 3 bytes)
  U8                        Allow2bitMode;           // Set to 1 if data can be exchanged via 2 data lines (half-duplex)
  U8                        Allow4bitMode;           // Set to 1 if data can be exchanged via 4 data lines (half-duplex)
  U8                        CmdRead;                 // Command to be used for reading data
  U8                        NumBytesReadDummy;       // Number of dummy bytes to read after the address.
  U8                        NumSectorBlocks;         // Number of items stored in aSectorBlock
  U16                       BusWidthRead;            // Number of data lines to be used for the read operation
  FS_NOR_SPI_SECTOR_BLOCK   aSectorBlock[FS_NOR_MAX_SECTOR_BLOCKS];   // Stores information about the physical sectors.
  U32                       NtimeoutRegWrite;        // Number of cycles to wait for a register write operation to complete
  const FS_NOR_SPI_CMD    * pCmd;                    // Pointer to functions to be used for the data transfer
  void                    * pContext;                // Pointer to a user defined data which is passed as first argument to all functions in pCmd
} FS_NOR_SPI_INST;

/*********************************************************************
*
*       FS_NOR_SPI_TYPE
*
*  Operations on serial NOR devices
*/
struct FS_NOR_SPI_TYPE {
  int  (*pfIdentify)                (const FS_NOR_SPI_INST * pInst, const U8 * pId);
  void (*pfInit)                    (const FS_NOR_SPI_INST * pInst);
  int  (*pfSetBusWidth)             (const FS_NOR_SPI_INST * pInst, unsigned BusWidth);    // BusWidth: 4/2/1
  int  (*pfSetNumBytesAddr)         (const FS_NOR_SPI_INST * pInst, unsigned NumBytes);
  int  (*pfReadApplyPara)           (      FS_NOR_SPI_INST * pInst);
  int  (*pfRemoveWriteProtection)   (const FS_NOR_SPI_INST * pInst, U32 StartAddr, U32 NumBytes);
  int  (*pfEraseSector)             (const FS_NOR_SPI_INST * pInst, U8 CmdErase, U32 SectorOff);
  int  (*pfWritePage)               (const FS_NOR_SPI_INST * pInst, U32 Addr, const U8 * pData, U32 NumBytes);
  int  (*pfWaitForEndOfOperation)   (const FS_NOR_SPI_INST * pInst, U32 TimeOut);
};

/*********************************************************************
*
*       FS_NOR_SPI_DEVICE
*/
typedef struct FS_NOR_SPI_DEVICE {
  FS_NOR_SPI_INST         Inst;
  const FS_NOR_SPI_TYPE * pType;
} FS_NOR_SPI_DEVICE;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
void     FS_NOR_SPI_ReadId           (const FS_NOR_SPI_INST * pInst, U8 * pData, unsigned NumBytes);
int      FS_NOR_SPI_ReadApplyParaById(      FS_NOR_SPI_INST * pInst);
U32      FS_NOR_SPI_GetSectorOff     (const FS_NOR_SPI_INST * pInst, unsigned SectorIndex);
unsigned FS_NOR_SPI_GetSectorSize    (const FS_NOR_SPI_INST * pInst, unsigned SectorIndex);
U8       FS_NOR_SPI_GetSectorEraseCmd(const FS_NOR_SPI_INST * pInst, unsigned SectorIndex);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // NOR_SPI_DEV_H

/*************************** End of file ****************************/
