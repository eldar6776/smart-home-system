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
File        : GUIDRV_S1D13781_Private.h
Purpose     : Interface definition for GUIDRV_S1D13781 driver
---------------------------END-OF-HEADER------------------------------
*/

#include "GUIDRV_S1D13781.h"

#ifndef GUIDRV_S1D13781_PRIVATE_H
#define GUIDRV_S1D13781_PRIVATE_H

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifndef   LCD_WRITE_BUFFER_SIZE
  #define LCD_WRITE_BUFFER_SIZE 500
#endif

//
// BitBLT registers
//
#define REG_BITBLT_CR0    0x60880
#define REG_BITBLT_CR1    0x60882
#define REG_BITBLT_SR     0x60884
#define REG_BITBLT_CR     0x60886
#define REG_BITBLT_SSAR0  0x60888
#define REG_BITBLT_SSAR1  0x6088A
#define REG_BITBLT_DSAR0  0x6088C
#define REG_BITBLT_DSAR1  0x6088E
#define REG_BITBLT_ROR    0x60890
#define REG_BITBLT_WR     0x60892
#define REG_BITBLT_HR     0x60894
#define REG_BITBLT_BCR0   0x60896
#define REG_BITBLT_BCR1   0x60898
#define REG_BITBLT_FCR0   0x6089A
#define REG_BITBLT_FCR1   0x6089C

#define REG_BITBLT_CR_MOVEP  (0 << 0)
#define REG_BITBLT_CR_FILL   (1 << 1)
#define REG_BITBLT_CR0_START (1 << 0)

#define REG_MAIN_XSIZE    0x60824
#define REG_MAIN_YSIZE    0x60828

#define REG_MAIN_SETUP    0x60840
#define REG_MAIN_SADDR0   0x60842
#define REG_MAIN_SADDR1   0x60844

#define REG_PIP_SETUP     0x60850
#define REG_PIP_SADDR0    0x60852
#define REG_PIP_SADDR1    0x60854
#define REG_PIP_XSIZE     0x60856
#define REG_PIP_YSIZE     0x60858
#define REG_PIP_XPOS      0x6085A
#define REG_PIP_YPOS      0x6085C
#define REG_PIP_ENABLE    0x60860

#define REG_PIP_ALPHA     0x60862
#define REG_PIP_TRANS     0x60864
#define REG_PIP_TCOL0     0x60866
#define REG_PIP_TCOL1     0x60868

#define REG_PCR_DSR       0x60822
#define REG_CR_PSR        0x60802

//
// Use unique context identified
//
#define DRIVER_CONTEXT DRIVER_CONTEXT_S1D13781

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct DRIVER_CONTEXT DRIVER_CONTEXT;

/*********************************************************************
*
*       MANAGE_VMEM_API
*/
typedef struct {
  //
  // Private function pointers
  //
  void (* pfWriteDataAuto_8) (DRIVER_CONTEXT * pContext, U32 Addr, U8 Data);
  void (* pfWriteDataFlush_8)(DRIVER_CONTEXT * pContext);

  void (* pfWriteDataM_8) (DRIVER_CONTEXT * pContext, U32 Addr, U8 Data, U32 NumItems);
  void (* pfWriteDataMP_8)(DRIVER_CONTEXT * pContext, U32 Addr, U8 * pData, U32 NumItems);
  U8 * (* pfReadDataM_8)  (DRIVER_CONTEXT * pContext, U32 Addr, U8 * pData, U32 NumItems);
  U8   (* pfReadData_8)   (DRIVER_CONTEXT * pContext, U32 Addr);
  void (* pfWriteReg)     (DRIVER_CONTEXT * pContext, U32 Addr, U16 Data);
  U16  (* pfReadReg)      (DRIVER_CONTEXT * pContext, U32 Addr);

} MANAGE_VMEM_API;

/*********************************************************************
*
*       DRIVER_CONTEXT_S1D13781
*/
struct DRIVER_CONTEXT {
  //
  // Common data
  //
  int xSize, ySize;
  int vxSize, vySize;
  int MemSize;
  int SwapXY;
  int XOff;
  U32 BufferOffset;
  int NumBytesInBuffer;
  U32 Addr;
  U32 BaseAddr;
  int WaitUntilVNDP;
  //
  // PIP layer
  //
  int IsVisible;
  int xPos, yPos;
  //
  // Driver specific data
  //
  int WriteBufferSize;
  int WriteBufferSizeNew;
  int UseLayer;
  U8 aInitialWriteBuffer[10];
  //
  // Accelerators for calculation
  //
  int BitsPerPixel;
  int BytesPerPixel;
  //
  // VRAM
  //
  U8 * pReadBuffer;
  U8 * pWriteBuffer;
  U8 * pWrite;
  //
  // API-Tables
  //
  MANAGE_VMEM_API ManageVMEM_API; // Memory management
  GUI_PORT_API    HW_API;         // Hardware routines
};

/*********************************************************************
*
*       _SetPixelIndex_##EXT
*/
#define DEFINE_SETPIXELINDEX(EXT)                                                                 \
static void _SetPixelIndex_##EXT(GUI_DEVICE * pDevice, int x, int y, LCD_PIXELINDEX PixelIndex) { \
  DRIVER_CONTEXT_S1D13781 * pContext;                                                             \
                                                                                                  \
  pContext = (DRIVER_CONTEXT_S1D13781 *)pDevice->u.pContext;                                      \
  _SetPixelIndex(pContext, x, y, PixelIndex);                                                     \
}

/*********************************************************************
*
*       _GetPixelIndex_##EXT
*/
#define DEFINE_GETPIXELINDEX(EXT)                                                \
static LCD_PIXELINDEX _GetPixelIndex_##EXT(GUI_DEVICE * pDevice, int x, int y) { \
  DRIVER_CONTEXT_S1D13781 * pContext;                                            \
  LCD_PIXELINDEX PixelIndex;                                                     \
                                                                                 \
  pContext = (DRIVER_CONTEXT_S1D13781 *)pDevice->u.pContext;                     \
  PixelIndex = _GetPixelIndex(pContext, x, y);                                   \
  return PixelIndex;                                                             \
}

/*********************************************************************
*
*       _GetDevProp_##EXT
*/
#define DEFINE_GETDEVPROP(EXT, MX, MY, SWAP)                    \
static I32 _GetDevProp_##EXT(GUI_DEVICE * pDevice, int Index) { \
  switch (Index) {                                              \
  case LCD_DEVCAP_MIRROR_X: return MX;                          \
  case LCD_DEVCAP_MIRROR_Y: return MY;                          \
  case LCD_DEVCAP_SWAP_XY:  return SWAP;                        \
  }                                                             \
  return _GetDevProp(pDevice, Index);                           \
}

/*********************************************************************
*
*       DEFINE_FUNCTIONS
*/
#define DEFINE_FUNCTIONS(EXT, MX, MY, SWAP) \
  DEFINE_SETPIXELINDEX(EXT)                 \
  DEFINE_GETPIXELINDEX(EXT)                 \
  DEFINE_GETDEVPROP(EXT, MX, MY, SWAP)      \
  DEFINE_GUI_DEVICE_API(EXT)

/*********************************************************************
*
*       Private functions
*
**********************************************************************
*/
void (*GUIDRV__S1D13781_GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void);
void   GUIDRV__S1D13781_SetOrg    (GUI_DEVICE *  pDevice,  int x, int y);
I32    GUIDRV__S1D13781_GetDevProp(GUI_DEVICE *  pDevice,  int Index);
void   GUIDRV__S1D13781_GetRect   (GUI_DEVICE *  pDevice,  LCD_RECT * pRect);
int    GUIDRV__S1D13781_InitOnce  (GUI_DEVICE *  pDevice);
int    GUIDRV__S1D13781_Init      (GUI_DEVICE *  pDevice);

U32    GUIDRV__S1D13781_GetAddr   (DRIVER_CONTEXT * pContext, int x, int y);

#endif /* GUIDRV_S1D13781_PRIVATE_H */

/*************************** End of file ****************************/
