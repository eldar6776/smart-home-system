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
File        : GUIDRV_TemplateI.h
Purpose     : Interface definition for GUIDRV_TemplateI driver
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUIDRV_TEMPLATE_I_H
#define GUIDRV_TEMPLATE_I_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Configuration structure
*/
typedef struct {
  //
  // Driver specific configuration items
  //
  int Dummy;
} CONFIG_TEMPLATE_I;

/*********************************************************************
*
*       Display drivers
*/
//
// Addresses
//
extern const GUI_DEVICE_API GUIDRV_TEMPLATE_I_16_API;
extern const GUI_DEVICE_API GUIDRV_TEMPLATE_I_OY_16_API;
extern const GUI_DEVICE_API GUIDRV_TEMPLATE_I_OX_16_API;
extern const GUI_DEVICE_API GUIDRV_TEMPLATE_I_OXY_16_API;
extern const GUI_DEVICE_API GUIDRV_TEMPLATE_I_OS_16_API;
extern const GUI_DEVICE_API GUIDRV_TEMPLATE_I_OSY_16_API;
extern const GUI_DEVICE_API GUIDRV_TEMPLATE_I_OSX_16_API;
extern const GUI_DEVICE_API GUIDRV_TEMPLATE_I_OSXY_16_API;

//
// Macros to be used in configuration files
//
#if defined(WIN32) && !defined(LCD_SIMCONTROLLER)

  #define GUIDRV_TEMPLATE_I_16       &GUIDRV_Win_API
  #define GUIDRV_TEMPLATE_I_OY_16    &GUIDRV_Win_API
  #define GUIDRV_TEMPLATE_I_OX_16    &GUIDRV_Win_API
  #define GUIDRV_TEMPLATE_I_OXY_16   &GUIDRV_Win_API
  #define GUIDRV_TEMPLATE_I_OS_16    &GUIDRV_Win_API
  #define GUIDRV_TEMPLATE_I_OSY_16   &GUIDRV_Win_API
  #define GUIDRV_TEMPLATE_I_OSX_16   &GUIDRV_Win_API
  #define GUIDRV_TEMPLATE_I_OSXY_16  &GUIDRV_Win_API

#else

  #define GUIDRV_TEMPLATE_I_16       &GUIDRV_TEMPLATE_I_16_API
  #define GUIDRV_TEMPLATE_I_OY_16    &GUIDRV_TEMPLATE_I_OY_16_API
  #define GUIDRV_TEMPLATE_I_OX_16    &GUIDRV_TEMPLATE_I_OX_16_API
  #define GUIDRV_TEMPLATE_I_OXY_16   &GUIDRV_TEMPLATE_I_OXY_16_API
  #define GUIDRV_TEMPLATE_I_OS_16    &GUIDRV_TEMPLATE_I_OS_16_API
  #define GUIDRV_TEMPLATE_I_OSY_16   &GUIDRV_TEMPLATE_I_OSY_16_API
  #define GUIDRV_TEMPLATE_I_OSX_16   &GUIDRV_TEMPLATE_I_OSX_16_API
  #define GUIDRV_TEMPLATE_I_OSXY_16  &GUIDRV_TEMPLATE_I_OSXY_16_API

#endif

/*********************************************************************
*
*       Public routines
*/
void GUIDRV_TemplateI_Config    (GUI_DEVICE * pDevice, CONFIG_TEMPLATE_I * pConfig);
void GUIDRV_TemplateI_SetBus_XXX(GUI_DEVICE * pDevice, GUI_PORT_API * pHW_API);
void GUIDRV_TemplateI_SetFuncXXX(GUI_DEVICE * pDevice);

#if defined(__cplusplus)
}
#endif

#endif

/*************************** End of file ****************************/