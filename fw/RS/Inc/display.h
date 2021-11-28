/**
 ******************************************************************************
 * File Name          : display.h
 * Date               : 10.3.2018
 * Description        : GUI Display Module Header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
 /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISP_H__
#define __DISP_H__                           FW_BUILD // version
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "main.h"
/* Exported Define -----------------------------------------------------------*/
#define SCRNSVR_TOUT    30U  // 10 sec timeout increment to set display in low brigntnes after last touch event
#define TS_LAYER        1U      // touch screen layer event
/* Exported types ------------------------------------------------------------*/
typedef enum	// display button states
{
	RELEASED    = 0U,
	PRESSED     = 1U,
    BUTTON_SHIT = 2U
	
}BUTTON_StateTypeDef;


typedef struct{
    uint8_t index;
    uint8_t old_index;
    uint8_t value;
    uint8_t old_value;
}LIGHT_CmdTypeDef;

typedef struct{
    
    LIGHT_CmdTypeDef Main1;
    LIGHT_CmdTypeDef Led1;
    LIGHT_CmdTypeDef Led2;
    LIGHT_CmdTypeDef Led3;
    LIGHT_CmdTypeDef Out1;
    LIGHT_CmdTypeDef Light1;
    LIGHT_CmdTypeDef Light2;
    LIGHT_CmdTypeDef Light3;
    
}LIGHT_CtrlTypeDef;

extern LIGHT_CtrlTypeDef LIGHT_Ctrl1;
extern LIGHT_CtrlTypeDef LIGHT_Ctrl2;
/* Exported variables  -------------------------------------------------------*/
extern uint32_t dispfl;
extern char logbuf[128];
void DISP_UpdateLog(const char *pbuf);
extern uint8_t scrnsvr_ena_hour, scrnsvr_dis_hour, scrnsvr_tout;
extern uint8_t high_bcklght, low_bcklght, light_ldr;
extern uint8_t scrnsvr_clk_clr, scrnsvr_semiclk_clr;
/* Exported Macro   --------------------------------------------------------- */
#define DISPUpdateSet()                     (dispfl |=  (1U<<0))
#define DISPUpdateReset()                   (dispfl &=(~(1U<<0)))
#define IsDISPUpdateActiv()                 (dispfl &   (1U<<0))
#define DISPBldrUpdSet()                    (dispfl |=  (1U<<1))
#define DISPBldrUpdReset()		            (dispfl &=(~(1U<<1)))
#define IsDISPBldrUpdSetActiv()		        (dispfl &   (1U<<1))
#define DISPBldrUpdFailSet()			    (dispfl |=  (1U<<2))
#define DISPBldrUpdFailReset()	            (dispfl &=(~(1U<<2)))
#define IsDISPBldrUpdFailActiv()	        (dispfl &   (1U<<2))
#define DISPUpdProgMsgSet()                 (dispfl |=  (1U<<3))
#define DISPUpdProgMsgDel()                 (dispfl &=(~(1U<<3)))
#define IsDISPUpdProgMsgActiv()             (dispfl &   (1U<<3))
#define DISPFwrUpd()                        (dispfl |=  (1U<<4))
#define DISPFwrUpdDelete()                  (dispfl &=(~(1U<<4)))
#define IsDISPFwrUpdActiv()                 (dispfl &   (1U<<4))
#define DISPFwrUpdFail()                    (dispfl |=  (1U<<5))
#define DISPFwrUpdFailDelete()              (dispfl &=(~(1U<<5)))
#define IsDISPFwrUpdFailActiv()             (dispfl &   (1U<<5))
#define DISPFwUpdSet()                      (dispfl |=  (1U<<6))
#define DISPFwUpdReset()                    (dispfl &=(~(1U<<6)))
#define IsDISPFwUpdActiv()                  (dispfl &   (1U<<6))
#define DISPFwUpdFailSet()                  (dispfl |=  (1U<<7))
#define DISPFwUpdFailReset()                (dispfl &=(~(1U<<7)))
#define IsDISPFwUpdFailActiv()              (dispfl &   (1U<<7))
#define PWMErrorSet()                       (dispfl |=  (1U<<8)) 
#define PWMErrorReset()                     (dispfl &=(~(1U<<8)))
#define IsPWMErrorActiv()                   (dispfl &   (1U<<8))
#define DISPKeypadSet()                     (dispfl |=  (1U<<9))
#define DISPKeypadReset()                   (dispfl &=(~(1U<<9)))
#define IsDISPKeypadActiv()                 (dispfl &   (1U<<9))
#define DISPUnlockSet()                     (dispfl |=  (1U<<10))
#define DISPUnlockReset()                   (dispfl &=(~(1U<<10)))
#define IsDISPUnlockActiv()                 (dispfl &   (1U<<10))
#define DISPLanguageSet()                   (dispfl |=  (1U<<11))
#define DISPLanguageReset()                 (dispfl &=(~(1U<<11)))
#define IsDISPLanguageActiv()               (dispfl &   (1U<<11))
#define DISPSettingsInitSet()               (dispfl |=  (1U<<12))	
#define DISPSettingsInitReset()             (dispfl &=(~(1U<<12)))
#define IsDISPSetInitActiv()                (dispfl &   (1U<<12))
#define DISPRefreshSet()					(dispfl |=  (1U<<13))
#define DISPRefreshReset()                  (dispfl &=(~(1U<<13)))
#define IsDISPRefreshActiv()			    (dispfl &   (1U<<13))
#define ScreenInitSet()                     (dispfl |=  (1U<<14))
#define ScreenInitReset()                   (dispfl &=(~(1U<<14)))
#define IsScreenInitActiv()                 (dispfl &   (1U<<14))
#define RtcTimeValidSet()                   (dispfl |=  (1U<<15))
#define RtcTimeValidReset()                 (dispfl &=(~(1U<<15)))
#define IsRtcTimeValid()                    (dispfl &   (1U<<15))
#define SPUpdateSet()                       (dispfl |=  (1U<<16)) 
#define SPUpdateReset()                     (dispfl &=(~(1U<<16)))
#define IsSPUpdateActiv()                   (dispfl &   (1U<<16))
#define ScrnsvrSet()                        (dispfl |=  (1U<<17)) 
#define ScrnsvrReset()                      (dispfl &=(~(1U<<17)))
#define IsScrnsvrActiv()                    (dispfl &   (1U<<17))
#define ScrnsvrClkSet()                     (dispfl |=  (1U<<18))
#define ScrnsvrClkReset()                   (dispfl &=(~(1U<<18)))
#define IsScrnsvrClkActiv()                 (dispfl &   (1U<<18))
#define ScrnsvrSemiClkSet()                 (dispfl |=  (1U<<19))
#define ScrnsvrSemiClkReset()               (dispfl &=(~(1U<<19)))
#define IsScrnsvrSemiClkActiv()             (dispfl &   (1U<<19))
#define MVUpdateSet()                       (dispfl |=  (1U<<20)) 
#define MVUpdateReset()                     (dispfl &=(~(1U<<20)))
#define IsMVUpdateActiv()                   (dispfl &   (1U<<20))
#define ScrnsvrEnable()                     (dispfl |=  (1U<<21)) 
#define ScrnsvrDisable()                    (dispfl &=(~(1U<<21)))
#define IsScrnsvrEnabled()                  (dispfl &   (1U<<21))
#define ScrnsvrInitSet()                    (dispfl |=  (1U<<22)) 
#define ScrnsvrInitReset()                  (dispfl &=(~(1U<<22)))
#define IsScrnsvrInitActiv()                (dispfl &   (1U<<22))
#define BTNUpdSet()                         (dispfl |=  (1U<<23))
#define BTNUpdReset()                       (dispfl &=(~(1U<<23)))
#define IsBTNUpdActiv()                     (dispfl &   (1U<<23))
#define DISPCleaningSet()                   (dispfl |=  (1U<<24))
#define DISPCleaningReset()                 (dispfl &=(~(1U<<24)))
#define IsDISPCleaningActiv()               (dispfl &   (1U<<24))
/* Exported functions  -------------------------------------------------------*/
void DISP_Init(void);
void DISP_Service(void);
void DISPDateTime(void);
void DISPSetPoint(void);
void DISPResetScrnsvr(void);
void DISPSetBrightnes(uint8_t val);
void PID_Hook(GUI_PID_STATE* pState);
uint8_t DISPMenuSettings(uint8_t btn);
void SaveLightController(LIGHT_CtrlTypeDef* lc, uint16_t addr);
void ReadLightController(LIGHT_CtrlTypeDef* lc, uint16_t addr);
//void SaveThermostatController(THERMOSTAT_TypeDef* tc, uint16_t addr);
//void ReadThermostatController(THERMOSTAT_TypeDef* tc, uint16_t addr);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
