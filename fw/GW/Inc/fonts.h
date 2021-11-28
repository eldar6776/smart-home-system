/**
  * @file    main.c 
  * @author  WB R&D Team - openmcu666
  * @version V0.1
  * @date    2015.9.22
  * @brief   FONT
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FONTS_H
#define __FONTS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup Utilities
  * @{
  */
  
/** @addtogroup STM32_EVAL
  * @{
  */ 

/** @addtogroup Common
  * @{
  */

/** @addtogroup FONTS
  * @{
  */ 

/** @defgroup FONTS_Exported_Types
  * @{
  */ 
typedef struct _tFont
{    
  const uint16_t *table;
  uint16_t Width;
  uint16_t Height;
  
} sFONT;

typedef struct {                                                 //汉字字模数据结构   
    unsigned char Index[2];                                      // 汉字内码索引                
    unsigned char Msk[72];                                       // 点阵码数据                   
}FNT_GB24;

typedef struct {                                                 //汉字字模数据结构   
    unsigned char Index[2];                                      // 汉字内码索引                
    unsigned char Msk[32];                                       // 点阵码数据                   
}FNT_GB16;

extern sFONT Font16x24;
extern sFONT Font12x12;
extern sFONT Font8x12;
extern sFONT Font8x8;
extern const FNT_GB24 Chinese24x24_Table[];
extern const FNT_GB16 Chinese16x16_Table[];

extern sFONT Font16x16;
extern sFONT Font24x24;

typedef enum{
	size16x16,
  size24x24  
} FONT_SIZE;
/**
  * @}
  */ 

/** @defgroup FONTS_Exported_Constants
  * @{
  */ 
#define LINE(x) ((x) * (((sFONT *)LCD_GetFont())->Height))

/**
  * @}
  */ 

/** @defgroup FONTS_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup FONTS_Exported_Functions
  * @{
  */ 
/**
  * @}
  */

#ifdef __cplusplus
}
#endif
  
#endif /* __FONTS_H */
 
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

/**
  * @}
  */      

