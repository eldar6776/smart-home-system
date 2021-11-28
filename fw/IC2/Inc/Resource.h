/*
----------------------------------------------------------------------
File        : Resource.h
Content     : Main resource header file of weather forecast demo
---------------------------END-OF-HEADER------------------------------
*/

#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdlib.h>
#include "GUI.h"

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif

extern GUI_CONST_STORAGE GUI_BITMAP bmSijalicaOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmSijalicicaOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmSijalicaOff;
extern GUI_CONST_STORAGE GUI_BITMAP bmSijalicicaOff;
extern GUI_CONST_STORAGE GUI_BITMAP bmSijalica;
extern GUI_CONST_STORAGE GUI_BITMAP bmSijalicica;
extern GUI_CONST_STORAGE GUI_BITMAP bmTermometar;
extern GUI_CONST_STORAGE GUI_BITMAP bmHome;
extern GUI_CONST_STORAGE GUI_BITMAP bmArmed;
extern GUI_CONST_STORAGE GUI_BITMAP bmDisarmed;
extern GUI_CONST_STORAGE GUI_BITMAP bmClean;
extern GUI_CONST_STORAGE GUI_BITMAP bmKatanac;

extern GUI_CONST_STORAGE GUI_BITMAP bmmist_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmmist_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmrain_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmrain_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmsnow_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmsnow_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmclear_sky_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmclear_sky_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmfew_clouds_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmfew_clouds_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmshower_rain_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmshower_rain_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmthunderstorm_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmthunderstorm_icon;
extern GUI_CONST_STORAGE GUI_BITMAP bmscattered_clouds_img;
extern GUI_CONST_STORAGE GUI_BITMAP bmscattered_clouds_icon;

extern const unsigned long thstat_size;
extern const unsigned char thstat[];
#endif // RESOURCE_H
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
