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
File        : GUI_SIM_Win32.h
Purpose     : Declares public functions of Simulation
----------------------------------------------------------------------
*/

#ifndef SIM_GUI_H
#define SIM_GUI_H

#include <windows.h>

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/********************************************************************
*
*       Data
*
*********************************************************************
*/
extern HINSTANCE SIM_GUI_hInst;
extern HWND      SIM_GUI_hWndMain;

/********************************************************************
*
*       Types
*
*********************************************************************
*/
typedef struct {
  HWND hWndMain;
  HWND ahWndLCD[16];
  HWND ahWndColor[16];
} SIM_GUI_INFO;

typedef int  SIM_GUI_tfHook           (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int * pResult);
typedef void SIM_GUI_tfDelayHandler   (int ms);
typedef void SIM_GUI_tfExecIdleHandler(void);

/********************************************************************
*
*       Interface
*
*********************************************************************
*/
void SIM_GUI_ShowDevice           (int OnOff);
void SIM_GUI_SetCallback          (int (* pfCallback)(SIM_GUI_INFO * pInfo));
void SIM_GUI_HandleKeyEvents      (unsigned Msg, WPARAM wParam);
HWND SIM_GUI_CreateCompositeWindow(HWND hParent, int x, int y, int xSize, int ySize, int DisplayIndex);
HWND SIM_GUI_CreateLCDWindow      (HWND hParent, int x, int y, int xSize, int ySize, int LayerIndex);
HWND SIM_GUI_CreateLOGWindow      (HWND hParent, int x, int y, int xSize, int ySize);
HWND SIM_GUI_CreateLCDInfoWindow  (HWND hParent, int x, int y, int xSize, int ySize, int LayerIndex);
void SIM_GUI_Enable               (void);
int  SIM_GUI_Init                 (HINSTANCE hInst, HWND hWndMain, char * pCmdLine, const char * sAppName);
void SIM_GUI_CopyToClipboard      (int LayerIndex);
void SIM_GUI_SetLCDWindowHook     (SIM_GUI_tfHook         * pfHook);
void SIM_GUI_SetDelayHandler      (SIM_GUI_tfDelayHandler * pfHandler);
void SIM_GUI_SetExecIdleHandler   (SIM_GUI_tfExecIdleHandler * pfHandler);
void SIM_GUI_GetCompositeSize     (int * pxSize, int * pySize);
int  SIM_GUI_GetTransColor        (void);
void SIM_GUI_GetLCDPos            (int * px, int * py);
void SIM_GUI_Exit                 (void);
void SIM_GUI_SetMessageBoxOnError (int OnOff);
int  SIM_GUI_App                  (HINSTANCE hInstance, HINSTANCE hPrevInstance,  LPSTR lpCmdLine, int nCmdShow);
void SIM_GUI_SetPixel             (int x, int y, unsigned Color);

void         SIM_GUI_LOG_Time  (void);
void __cdecl SIM_GUI_LOG_Add   (const char *format ,... );
void         SIM_GUI_LOG_AddRed(void);
void         SIM_GUI_LOG_Clear (void);

void LCDSIM_Paint         (HWND hWnd);
void LCDSIM_PaintComposite(HWND hWnd);
void LCDSIM_SetTransMode  (int LayerIndex, int TransMode);
void LCDSIM_SetChroma     (int LayerIndex, unsigned long ChromaMin, unsigned long ChromaMax);

#if defined(__cplusplus)
}
#endif 

#endif /* SIM_GUI_H */
