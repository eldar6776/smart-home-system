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


extern GUI_CONST_STORAGE GUI_BITMAP bmHOME;
extern GUI_CONST_STORAGE GUI_BITMAP bmCLEAN;
extern GUI_CONST_STORAGE GUI_BITMAP bmCUP_ON;
extern GUI_CONST_STORAGE GUI_BITMAP bmCUP_OFF;
extern GUI_CONST_STORAGE GUI_BITMAP bmSETTINGS;
extern GUI_CONST_STORAGE GUI_BITMAP bmBLIND_BIG;
extern GUI_CONST_STORAGE GUI_BITMAP bmBLIND_SMALL;
extern const unsigned long thstat_size;
extern const unsigned char thstat[];
#endif // RESOURCE_H
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
