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
File        : LCDConf.c
Purpose     : Display controller configuration

The part between 'DISPLAY CONFIGURATION START' and 'DISPLAY CONFIGURA-
TION END' can be used to configure the following for each layer:

- Color mode
- Layer size
- Layer orientation

Further the background color used on positions without a valid layer
can be set here.

---------------------------END-OF-HEADER------------------------------
*/

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"

#include "GUI.h"
#include "PIDConf.h"
#include "GUI_Private.h"
#include "GUIDRV_Lin.h"
#include "LCDConf.h"

/*********************************************************************
**********************************************************************
*
*       DISPLAY CONFIGURATION START (TO BE MODIFIED)
*
**********************************************************************
**********************************************************************
*/
/*********************************************************************
*
*       Common
*/
#undef  LCD_SWAP_XY
#undef  LCD_MIRROR_Y
#undef  LCD_SWAP_RB
#define LCD_SWAP_XY  1
#define LCD_MIRROR_Y 1
#define LCD_SWAP_RB  1

//
// Physical display size
//
#define XSIZE_PHYS 480
#define YSIZE_PHYS 272

//
// Buffers / VScreens
//
#define NUM_BUFFERS  3 // Number of multiple buffers to be used
#define NUM_VSCREENS 1 // Number of virtual screens to be used

//
// Redefine number of layers for this configuration file.
// Must be equal or less than in GUIConf.h!
//
#undef  GUI_NUM_LAYERS
#define GUI_NUM_LAYERS 1

#define BK_COLOR GUI_DARKBLUE

#define COLOR_CONVERSION_0    GUICC_M8888I
#define DISPLAY_DRIVER_0      GUIDRV_LIN_32

#if (GUI_NUM_LAYERS > 1)
  #define COLOR_CONVERSION_1  GUICC_M1555I
  #define DISPLAY_DRIVER_1    GUIDRV_LIN_16
#endif

#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens and multiple buffers are not allowed!
#endif

//
// Define the layer start address. Placed in SDRAM.
//
#define LCD_LAYER0_FRAME_BUFFER    ((int)0xC0200000)
#if GUI_NUM_LAYERS > 1
  #define LCD_LAYER1_FRAME_BUFFER  ((int)0xC0400000)
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  I32      Address;
  __IO I32 PendingBuffer;
  I32      BufferIndex;
  I32      xSize;
  I32      ySize;
  I32      BytesPerPixel;
  LCD_API_COLOR_CONV   *pColorConvAPI;
} LCD_LayerPropTypedef;

//
// hltdc is global because it is used by stm32f7xxit.c
//
LTDC_HandleTypeDef          hltdc;
static DMA2D_HandleTypeDef  _hDMA2d;
static LCD_LayerPropTypedef _aLayerProp[GUI_NUM_LAYERS];

static const LCD_API_COLOR_CONV * _apColorConvAPI[] = {
  COLOR_CONVERSION_0,
#if GUI_NUM_LAYERS > 1
  COLOR_CONVERSION_1,
#endif
};
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _LCD_LL_GetPixelformat
*/
static inline U32 _LCD_LL_GetPixelformat(U32 LayerIndex) {
  if (LayerIndex == 0) {
    return LTDC_PIXEL_FORMAT_ARGB8888;
  } else {
    return LTDC_PIXEL_FORMAT_ARGB1555;
  }
}

/*********************************************************************
*
*       _LCD_LL_LayerInit
*/
static void _LCD_LL_LayerInit(U32 LayerIndex) {
  LTDC_LayerCfgTypeDef Layer_cfg;

  if (LayerIndex < GUI_NUM_LAYERS) {
    //
    // Layer configuration
    //
    Layer_cfg.WindowX0 = 0;
    Layer_cfg.WindowX1 = XSIZE_PHYS;
    Layer_cfg.WindowY0 = 0;
    Layer_cfg.WindowY1 = YSIZE_PHYS;
    Layer_cfg.PixelFormat = _LCD_LL_GetPixelformat(LayerIndex);
    switch (LayerIndex) {
    case 0:
      Layer_cfg.FBStartAdress = ((U32)LCD_LAYER0_FRAME_BUFFER);
      break;
#if GUI_NUM_LAYERS > 1
    case 1:
      Layer_cfg.FBStartAdress = ((U32)LCD_LAYER1_FRAME_BUFFER);
      break;
#endif
    }
    Layer_cfg.Alpha = 255;
    Layer_cfg.Alpha0 = 0;
    Layer_cfg.Backcolor.Blue = 0;
    Layer_cfg.Backcolor.Green = 0;
    Layer_cfg.Backcolor.Red = 0;
    Layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    Layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    Layer_cfg.ImageWidth = XSIZE_PHYS;
    Layer_cfg.ImageHeight = YSIZE_PHYS;
    HAL_LTDC_ConfigLayer(&hltdc, &Layer_cfg, LayerIndex);
    //
    // Enable LUT on demand
    //
    if (LCD_GetBitsPerPixelEx(LayerIndex) <= 8) {
      //
      // Enable usage of LUT for all modes with <= 8bpp*/
      //
      HAL_LTDC_EnableCLUT(&hltdc, LayerIndex);
    }
  }
}

/*********************************************************************
*
*       _LCD_LL_Init
*/
static void _LCD_LL_Init(void) {
  //
  // DeInit
  //
  HAL_LTDC_DeInit(&hltdc);
  //
  // Set LCD Timings
  //
  hltdc.Init.HorizontalSync     = 40;
  hltdc.Init.VerticalSync       = 9;
  hltdc.Init.AccumulatedHBP     = 53;
  hltdc.Init.AccumulatedVBP     = 11;
  hltdc.Init.AccumulatedActiveH = 283;
  hltdc.Init.AccumulatedActiveW = 533;
  hltdc.Init.TotalHeigh         = 285;
  hltdc.Init.TotalWidth         = 565;
  //
  // background value
  //
  hltdc.Init.Backcolor.Blue  = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red   = 0;
  //
  // Polarity
  //
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Instance = LTDC;
  HAL_LTDC_Init(&hltdc);
  HAL_LTDC_ProgramLineEvent(&hltdc, 0);
  //
  // Enable dithering
  //
  HAL_LTDC_EnableDither(&hltdc);
  //
  // Configure the DMA2D default mode
  //
  _hDMA2d.Init.Mode         = DMA2D_R2M;
  _hDMA2d.Init.ColorMode    = DMA2D_RGB565;
  _hDMA2d.Init.OutputOffset = 0x0;
  _hDMA2d.Instance          = DMA2D;
  if (HAL_DMA2D_Init(&_hDMA2d) != HAL_OK) {
    while (1) {
    }
  }
  //
  // Assert display enable LCD_DISP pin
  //
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET);
}

/*********************************************************************
*
*       _LCD_LL_Init
*/
static void _Backlight_Init(void) {
  //
  // Assert backlight LCD_BL_CTRL pin
  //
  HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_SET);
}

/*********************************************************************
*
*       _DMA2D_CopyBuffer
*
*  Function description
*    Return Pixel format for a given layer
*/
static void _DMA2D_CopyBuffer(U32 LayerIndex, void * pSrc, void * pDst, U32 xSize, U32 ySize, U32 OffLineSrc, U32 OffLineDst) {
  U32 PixelFormat;

  PixelFormat = _LCD_LL_GetPixelformat(LayerIndex);
  DMA2D->CR   = 0x00000000UL | (1 << 9);
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pSrc;
  DMA2D->OMAR    = (U32)pDst;
  DMA2D->FGOR    = OffLineSrc;
  DMA2D->OOR     = OffLineDst;
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = PixelFormat;
  //
  //  Set up size
  //
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;
  DMA2D->CR     |= DMA2D_CR_START;
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
  }
}

/*********************************************************************
*
*       _DMA2D_FillBuffer
*/
static void _DMA2D_FillBuffer(U32 LayerIndex, void * pDst, U32 xSize, U32 ySize, U32 OffLine, U32 ColorIndex) {
  U32 PixelFormat;

  PixelFormat = _LCD_LL_GetPixelformat(LayerIndex);
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00030000UL | (1 << 9);
  DMA2D->OCOLR   = ColorIndex;
  //
  // Set up pointers
  //
  DMA2D->OMAR    = (U32)pDst;
  //
  // Set up offsets
  //
  DMA2D->OOR     = OffLine;
  //
  // Set up pixel format
  //
  DMA2D->OPFCCR  = PixelFormat;
  //
  //  Set up size
  //
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;
  DMA2D->CR     |= DMA2D_CR_START;
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
  }
}
/*********************************************************************
*
*       _GetBufferSize
*/
static U32 _GetBufferSize(U32 LayerIndex) {
  U32 BufferSize;

  BufferSize = _aLayerProp[LayerIndex].xSize * _aLayerProp[LayerIndex].ySize * _aLayerProp[LayerIndex].BytesPerPixel;
  return BufferSize;
}

/*********************************************************************
*
*       _Custom_CopyBuffer
*/
static void _Custom_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst) {
  U32 BufferSize, AddrSrc, AddrDst;

  BufferSize = _GetBufferSize(LayerIndex);
  AddrSrc    = _aLayerProp[LayerIndex].Address + BufferSize * IndexSrc;
  AddrDst    = _aLayerProp[LayerIndex].Address + BufferSize * IndexDst;
  _DMA2D_CopyBuffer(LayerIndex, (void *)AddrSrc, (void *)AddrDst, _aLayerProp[LayerIndex].xSize, _aLayerProp[LayerIndex].ySize, 0, 0);
  _aLayerProp[LayerIndex].BufferIndex = IndexDst;
}

/*********************************************************************
*
*       _Custom_CopyRect
*/
static void _Custom_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize) {
  U32 AddrSrc, AddrDst;

  AddrSrc = _aLayerProp[LayerIndex].Address + (y0 * _aLayerProp[LayerIndex].xSize + x0) * _aLayerProp[LayerIndex].BytesPerPixel;
  AddrDst = _aLayerProp[LayerIndex].Address + (y1 * _aLayerProp[LayerIndex].xSize + x1) * _aLayerProp[LayerIndex].BytesPerPixel;
  _DMA2D_CopyBuffer(LayerIndex, (void *)AddrSrc, (void *)AddrDst, xSize, ySize, _aLayerProp[LayerIndex].xSize - xSize, _aLayerProp[LayerIndex].xSize - xSize);
}

/*********************************************************************
*
*       _Custom_FillRect
*/
static void _Custom_FillRect(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex) {
  U32 BufferSize, AddrDst;
  int xSize, ySize;

  if (GUI_GetDrawMode() == GUI_DM_XOR) {
    LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, NULL);
    LCD_FillRect(x0, y0, x1, y1);
    LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, (void(*)(void))_Custom_FillRect);
  } else {
    xSize = x1 - x0 + 1;
    ySize = y1 - y0 + 1;
    BufferSize = _GetBufferSize(LayerIndex);
    AddrDst = _aLayerProp[LayerIndex].Address
              + BufferSize * _aLayerProp[LayerIndex].BufferIndex
              + (y0 * _aLayerProp[LayerIndex].xSize + x0) * _aLayerProp[LayerIndex].BytesPerPixel;
    _DMA2D_FillBuffer(LayerIndex, (void *)AddrDst, xSize, ySize, _aLayerProp[LayerIndex].xSize - xSize, PixelIndex);
  }
}

/*********************************************************************
*
*       _Custom_DrawBitmap32bpp
*/
static void _Custom_DrawBitmap32bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine) {
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;

  BufferSize = _GetBufferSize(LayerIndex);
  AddrDst = _aLayerProp[LayerIndex].Address
            + BufferSize * _aLayerProp[LayerIndex].BufferIndex
            + (y * _aLayerProp[LayerIndex].xSize + x) * _aLayerProp[LayerIndex].BytesPerPixel;
  OffLineSrc = (BytesPerLine / 4) - xSize;
  OffLineDst = _aLayerProp[LayerIndex].xSize - xSize;
  _DMA2D_CopyBuffer(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       HAL_DMA2D_MspInit
*
*  Function description
*    DMA2D MSP Initialization
*    This function configures the hardware resources used in this example:
*    - Peripheral's clock enable
*    - Peripheral's GPIO Configuration
*
*    This is a function declared as __weak in the HAL library.
*/
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *__hDMA2d) {
  GUI_USE_PARA(__hDMA2d);
  //
  // Enable peripheral
  //
  __HAL_RCC_DMA2D_CLK_ENABLE();
}

/*********************************************************************
*
*       HAL_DMA2D_MspDeInit
*
*  Function description
*    DMA2D MSP De-Initialization
*    This function frees the hardware resources used in this example:
*    - Disable the Peripheral's clock
*
*    This is a function declared as __weak in the HAL library.
*/
void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef *__hDMA2d) {
  GUI_USE_PARA(__hDMA2d);
  //
  // Enable DMA2D reset state
  //
  __HAL_RCC_DMA2D_FORCE_RESET();
  //
  // Release DMA2D from reset state
  //
  __HAL_RCC_DMA2D_RELEASE_RESET();
}

/*********************************************************************
*
*       HAL_LTDC_MspInit
*
*  Function description
*    LTDC MSP Initialization
*    This function configures the hardware resources used in this example:
*    - Peripheral's clock enable
*    - Peripheral's GPIO Configuration
*
*    This is a function declared as __weak in the HAL library.
*/
void HAL_LTDC_MspInit(LTDC_HandleTypeDef *__hLTDC) {
  GPIO_InitTypeDef GpioInitStructure;
  static RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  GUI_USE_PARA(__hLTDC);
  //
  // Enable the LTDC clocks
  //
  __HAL_RCC_LTDC_CLK_ENABLE();
  //
  // LCD clock configuration
  // PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz
  // PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 429 Mhz
  // PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 429/5 = 85 Mhz
  // LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_2 = 85/4 = 21 Mhz
  //
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  //
  // Enable GPIOs clock
  //
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  //
  // LTDC Pins configuration
  // GPIOE configuration
  //
  GpioInitStructure.Pin       = GPIO_PIN_4;
  GpioInitStructure.Mode      = GPIO_MODE_AF_PP;
  GpioInitStructure.Pull      = GPIO_NOPULL;
  GpioInitStructure.Speed     = GPIO_SPEED_FAST;
  GpioInitStructure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOE, &GpioInitStructure);
  //
  // GPIOG configuration
  //
  GpioInitStructure.Pin       = GPIO_PIN_12;
  GpioInitStructure.Mode      = GPIO_MODE_AF_PP;
  GpioInitStructure.Alternate = GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOG, &GpioInitStructure);
  //
  // GPIOI LTDC alternate configuration
  //
  GpioInitStructure.Pin       = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                                GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GpioInitStructure.Mode      = GPIO_MODE_AF_PP;
  GpioInitStructure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOI, &GpioInitStructure);
  //
  // GPIOJ configuration
  //
  GpioInitStructure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
                                GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
                                GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GpioInitStructure.Mode      = GPIO_MODE_AF_PP;
  GpioInitStructure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOJ, &GpioInitStructure);
  //
  // GPIOK configuration
  //
  GpioInitStructure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 |
                                GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  GpioInitStructure.Mode      = GPIO_MODE_AF_PP;
  GpioInitStructure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOK, &GpioInitStructure);
  //
  // LCD_DISP GPIO configuration
  //
  GpioInitStructure.Pin       = GPIO_PIN_12;    // LCD_DISP pin has to be manually controlled
  GpioInitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOI, &GpioInitStructure);
  //
  // LCD_BL_CTRL GPIO configuration
  //
  GpioInitStructure.Pin       = GPIO_PIN_3;     // LCD_BL_CTRL pin has to be manually controlled
  GpioInitStructure.Mode      = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOK, &GpioInitStructure);
  //
  // Set LTDC Interrupt to the lowest priority
  //
  HAL_NVIC_SetPriority(LTDC_IRQn, 0xE, 0);
  //
  // Enable LTDC Interrupt
  //
  HAL_NVIC_EnableIRQ(LTDC_IRQn);
}

/*********************************************************************
*
*       HAL_LTDC_MspDeInit
*
*  Function description
*    LTDC MSP De-Initialization
*    This function frees the hardware resources used in this example:
*    - Disable the Peripheral's clock
*
*    This is a function declared as __weak in the HAL library.
*/
void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *__hLTDC) {
  GUI_USE_PARA(__hLTDC);
  //
  // Reset peripherals
  // Enable LTDC reset state
  //
  __HAL_RCC_LTDC_FORCE_RESET();
  //
  // Release LTDC from reset state
  //
  __HAL_RCC_LTDC_RELEASE_RESET();
}

/*********************************************************************
*
*       HAL_LTDC_LineEvenCallback
*
*    This is a function declared as __weak in the HAL library.
*/
void HAL_LTDC_LineEvenCallback(LTDC_HandleTypeDef *__hLTDC) {
  U32 Addr;
  U32 layer;

  for (layer = 0; layer < GUI_NUM_LAYERS; layer++) {
    if (_aLayerProp[layer].PendingBuffer >= 0) {
      //
      // Calculate Address of buffer to be used  as visible frame buffer
      //
      Addr = _aLayerProp[layer].Address +
             _aLayerProp[layer].xSize * _aLayerProp[layer].ySize * _aLayerProp[layer].PendingBuffer * _aLayerProp[layer].BytesPerPixel;
      __HAL_LTDC_LAYER(__hLTDC, layer)->CFBAR = Addr;
      __HAL_LTDC_RELOAD_CONFIG(__hLTDC);
      //
      // Notify STemWin that buffer is used
      //
      GUI_MULTIBUF_ConfirmEx(layer, _aLayerProp[layer].PendingBuffer);
      //
      // Clear pending buffer flag of layer
      //
      _aLayerProp[layer].PendingBuffer = -1;
    }
  }
  HAL_LTDC_ProgramLineEvent(__hLTDC, 0);
}


/*********************************************************************
*
*       LCD_TFT_IRQHandler
*
*  Function description:
*    This is the interrupt handler for the LCD.
*/
#ifdef __SES_ARM
void LCD_TFT_IRQHandler(void);
void LCD_TFT_IRQHandler(void) {
  HAL_LTDC_IRQHandler(&hltdc);
}
#endif

/*********************************************************************
*
*       LCD_X_Config
*/
void LCD_X_Config(void) {
  U32 i;

  _LCD_LL_Init();
  //
  // At first initialize use of multiple buffers on demand
  //
#if (NUM_BUFFERS > 1)
    for (i = 0; i < GUI_NUM_LAYERS; i++) {
      GUI_MULTIBUF_ConfigEx(i, NUM_BUFFERS);
    }
#endif
  //
  // Set display driver and color conversion for 1st layer
  //
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_0, COLOR_CONVERSION_0, 0, 0);
  //
  // Set size of 1st layer
  //
  if (LCD_GetSwapXYEx(0)) {
    LCD_SetSizeEx (0, YSIZE_PHYS, XSIZE_PHYS);
    LCD_SetVSizeEx(0, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
  } else {
    LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
    LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
  }
#if (GUI_NUM_LAYERS > 1)
  //
  // Set display driver and color conversion for 2nd layer
  //
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_1, COLOR_CONVERSION_1, 0, 1);
  //
  // Set size of 2nd layer
  //
  if (LCD_GetSwapXYEx(1)) {
    LCD_SetSizeEx (1, YSIZE_PHYS, XSIZE_PHYS);
    LCD_SetVSizeEx(1, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
  } else {
    LCD_SetSizeEx (1, XSIZE_PHYS, YSIZE_PHYS);
    LCD_SetVSizeEx(1, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
  }
#endif
  //
  // Initialize GUI Layer structure
  //
  _aLayerProp[0].Address = LCD_LAYER0_FRAME_BUFFER;
#if (GUI_NUM_LAYERS > 1)
  _aLayerProp[1].Address = LCD_LAYER1_FRAME_BUFFER;
#endif
  //
  // Setting up VRam Address and custom functions for CopyBuffer-, CopyRect- and FillRect operations
  //
  for (i = 0; i < GUI_NUM_LAYERS; i++) {
    _aLayerProp[i].pColorConvAPI = (LCD_API_COLOR_CONV *)_apColorConvAPI[i];
    _aLayerProp[i].PendingBuffer = -1;
    //
    // Set VRAM Address
    //
    LCD_SetVRAMAddrEx(i, (void *)(_aLayerProp[i].Address));
    //
    // Remember color depth for further operations
    //
    _aLayerProp[i].BytesPerPixel = LCD_GetBitsPerPixelEx(i) >> 3;
    //
    // Set custom functions for several operations
    //
    LCD_SetDevFunc(i, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))_Custom_CopyBuffer);
    LCD_SetDevFunc(i, LCD_DEVFUNC_COPYRECT,   (void(*)(void))_Custom_CopyRect);
    LCD_SetDevFunc(i, LCD_DEVFUNC_FILLRECT,   (void(*)(void))_Custom_FillRect);
    //
    // Set up drawing routine for 32bpp bitmap using DMA2D
    //
    if (_LCD_LL_GetPixelformat(i) == LTDC_PIXEL_FORMAT_ARGB8888) {
      //
      // Set up drawing routine for 32bpp bitmap using DMA2D. Makes only sense with ARGB8888
      //
      LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_32BPP, (void(*)(void))_Custom_DrawBitmap32bpp);
    }
  }
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
*  Function description
*    This function is called by the display driver for several purposes.
*    To support the according task the routine needs to be adapted to
*    the display controller. Please note that the commands marked with
*    'optional' are not cogently required and should only be adapted if
*    the display controller supports these features
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r = 0;
  U32 Addr;
  int xPos, yPos;
  U32 Color;

  switch (Cmd) {
  case LCD_X_INITCONTROLLER:
    _LCD_LL_LayerInit(LayerIndex);
    PID_X_Init();
    break;
  case LCD_X_SETORG:
    Addr = _aLayerProp[LayerIndex].Address + ((LCD_X_SETORG_INFO *)pData)->yPos * _aLayerProp[LayerIndex].xSize * _aLayerProp[LayerIndex].BytesPerPixel;
    HAL_LTDC_SetAddress(&hltdc, Addr, LayerIndex);
    break;
  case LCD_X_SHOWBUFFER:
    _aLayerProp[LayerIndex].PendingBuffer = ((LCD_X_SHOWBUFFER_INFO *)pData)->Index;
    break;
  case LCD_X_SETLUTENTRY:
    //
    // Use of (uint32_t *) because (U32 *) causes compiler warnings
    //
    HAL_LTDC_ConfigCLUT(&hltdc, (uint32_t *) & (((LCD_X_SETLUTENTRY_INFO *)pData)->Color), 1, LayerIndex);
    break;
  case LCD_X_ON:
    __HAL_LTDC_ENABLE(&hltdc);
    //
    // Enable backlight later on, to prevent flickering after startup
    //
    GUI_X_Delay(50);
    _Backlight_Init();
    break;
  case LCD_X_OFF:
    __HAL_LTDC_DISABLE(&hltdc);
    break;
  case LCD_X_SETVIS:
    if (((LCD_X_SETVIS_INFO *)pData)->OnOff == ENABLE ) {
      __HAL_LTDC_LAYER_ENABLE(&hltdc, LayerIndex);
    } else {
      __HAL_LTDC_LAYER_DISABLE(&hltdc, LayerIndex);
    }
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
    break;
  case LCD_X_SETPOS:
    HAL_LTDC_SetWindowPosition(&hltdc,
                               ((LCD_X_SETPOS_INFO *)pData)->xPos,
                               ((LCD_X_SETPOS_INFO *)pData)->yPos,
                               LayerIndex);
    break;
  case LCD_X_SETSIZE:
    GUI_GetLayerPosEx(LayerIndex, &xPos, &yPos);
    _aLayerProp[LayerIndex].xSize = ((LCD_X_SETSIZE_INFO *)pData)->xSize;
    _aLayerProp[LayerIndex].ySize = ((LCD_X_SETSIZE_INFO *)pData)->ySize;
    HAL_LTDC_SetWindowPosition(&hltdc, xPos, yPos, LayerIndex);
    break;
  case LCD_X_SETALPHA:
    HAL_LTDC_SetAlpha(&hltdc, ((LCD_X_SETALPHA_INFO *)pData)->Alpha, LayerIndex);
    break;
  case LCD_X_SETCHROMAMODE:
    if (((LCD_X_SETCHROMAMODE_INFO *)pData)->ChromaMode != 0) {
      HAL_LTDC_EnableColorKeying(&hltdc, LayerIndex);
    } else {
      HAL_LTDC_DisableColorKeying(&hltdc, LayerIndex);
    }
    break;
  case LCD_X_SETCHROMA:
    Color = ((((LCD_X_SETCHROMA_INFO *)pData)->ChromaMin & 0xFF0000) >> 16) |
            (((LCD_X_SETCHROMA_INFO *)pData)->ChromaMin & 0x00FF00) |
            ((((LCD_X_SETCHROMA_INFO *)pData)->ChromaMin & 0x0000FF) << 16);
    HAL_LTDC_ConfigColorKeying(&hltdc, Color, LayerIndex);
    break;
  default:
    r = -1;
  }
  return r;
}

/*************************** End of file ****************************/
