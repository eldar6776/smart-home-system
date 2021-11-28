/**
 ******************************************************************************
 * Project          : KuceDzevadova
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

 
#if (__DISPH__ != FW_BUILD)
    #error "display header version mismatch"
#endif
/* Include  ------------------------------------------------------------------*/
#include "png.h"
#include "main.h"
#include "rs485.h"
#include "display.h"
#include "thermostat.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"
/* Private Define ------------------------------------------------------------*/
#define KEY_PAUSE                       20      // 
#define GUI_REFRESH_TIME                100     // refresh gui 10 time in second
#define DATE_TIME_REFRESH_TIME          500     // refresh date & time info every 1 sec. 
#define SETTINGS_MENU_ENABLE_TIME       3456    // press and holde upper left corrner for period to enter setup menu
#define WFC_TOUT                        8765    // 9 sec. weather display timeout   
#define BUTTON_RESPONSE_TIME            1234    // button response delay till all onewire device update state
#define DISPIMG_TIME_MULTI              30000   // 30 sec. min time increment for image display time * 255 max
#define SETTINGS_MENU_TIMEOUT           59000   // 1 min. settings menu timeout
#define KEYPAD_TOUT                     30000   // 
#define DISPMSG_TIME                    45      // time to display message on event * GUI_REFRESH_TIME
#define EVENT_ONOFF_TOUT                500    // on/off event touch max. time 1s  
#define VALUE_STEP_TOUT                 15      // light value chage time
#define WFC_CHECK_TOUT                  (SECONDS_PER_HOUR*1000) // check weather forecast data validity every hour
#define CLR_DARK_BLUE                   GUI_MAKE_COLOR(0x613600)
#define CLR_LIGHT_BLUE                  GUI_MAKE_COLOR(0xaa7d67)
#define CLR_BLUE                        GUI_MAKE_COLOR(0x855a41)
#define CLR_LEMON                       GUI_MAKE_COLOR(0x00d6d3)
#define BTN_SETTINGS_X0                 0
#define BTN_SETTINGS_Y0                 0
#define BTN_SETTINGS_X1                 100
#define BTN_SETTINGS_Y1                 100
#define BTN_DEC_X0                      0
#define BTN_DEC_Y0                      90
#define BTN_DEC_X1                      (BTN_DEC_X0+120)
#define BTN_DEC_Y1                      (BTN_DEC_Y0+179)
#define BTN_INC_X0                      200
#define BTN_INC_Y0                      90
#define BTN_INC_X1                      (BTN_INC_X0+120)
#define BTN_INC_Y1                      (BTN_INC_Y0+179)
#define BTN_OK_X0                       269
#define BTN_OK_Y0                       135
#define BTN_OK_X1                       (BTN_OK_X0+200)
#define BTN_OK_Y1                       (BTN_OK_Y0+134)
#define SP_H_POS                        200
#define SP_V_POS                        150
#define CLOCK_H_POS                     240
#define CLOCK_V_POS                     136
#define ID_Ok                           0x800
#define ID_Next                         0x801
#define ID_OSK_1                        0x802
#define ID_OSK_2                        0x803
#define ID_OSK_3                        0x804
#define ID_OSK_4                        0x805
#define ID_OSK_5                        0x806
#define ID_OSK_6                        0x807
#define ID_OSK_7                        0x808
#define ID_OSK_8                        0x809
#define ID_OSK_9   	                    0x80A
#define ID_OSK_0                        0x80B
#define ID_OSK_OK                       0x80C
#define ID_OSK_BK                       0x80B
#define ID_DisplayHighBrightness        0x810
#define ID_DisplayLowBrightness         0x811
#define ID_ScrnsvrTimeout               0x812
#define ID_ScrnsvrEnableHour            0x813
#define ID_ScrnsvrDisableHour           0x814
#define ID_ScrnsvrClkColour             0x815
#define ID_Hour                         0x816
#define ID_Minute                       0x817
#define ID_Day                          0x818
#define ID_Month                        0x819
#define ID_Year                         0x81A
#define ID_Scrnsvr                      0x81B
#define ID_ScrnsvrClock                 0x81C
#define ID_ScrnsvrLogoClock             0x81D
#define ID_WeekDay                      0x81F
#define ID_DEV_ID                       0x820
#define ID_BIN_MAIN                     0x821

#define ID_BIN_ALARM1                   0x823
#define ID_BIN_ALARM2                   0x824
#define ID_BIN_ALARM3                   0x825
#define ID_ThstControl                  0x830
#define ID_FanControl                   0x831
#define ID_ThstMaxSetPoint              0x832
#define ID_ThstMinSetPoint              0x833
#define ID_FanDiff                      0x834
#define ID_FanLowBand                   0x835
#define ID_FanHiBand                    0x836
/* Private Type --------------------------------------------------------------*/
BUTTON_Handle   hBUTTON_Ok;
BUTTON_Handle   hBUTTON_Next;
BUTTON_Handle   hBUTTON_OSK_1;
BUTTON_Handle   hBUTTON_OSK_2;
BUTTON_Handle   hBUTTON_OSK_3;
BUTTON_Handle   hBUTTON_OSK_4;
BUTTON_Handle   hBUTTON_OSK_5;
BUTTON_Handle   hBUTTON_OSK_6;
BUTTON_Handle   hBUTTON_OSK_7;
BUTTON_Handle   hBUTTON_OSK_8;
BUTTON_Handle   hBUTTON_OSK_9;
BUTTON_Handle   hBUTTON_OSK_0;
BUTTON_Handle   hBUTTON_OSK_OK;
BUTTON_Handle   hBUTTON_OSK_BK;
SPINBOX_Handle  hDEV_ID;
SPINBOX_Handle  hBIN_MAIN;

SPINBOX_Handle  hBIN_ALARM1;
SPINBOX_Handle  hBIN_ALARM2;
SPINBOX_Handle  hBIN_ALARM3;
RADIO_Handle    hThstControl;
RADIO_Handle    hFanControl;
SPINBOX_Handle  hThstMaxSetPoint;
SPINBOX_Handle  hThstMinSetPoint;
SPINBOX_Handle  hFanDiff;
SPINBOX_Handle  hFanLowBand;
SPINBOX_Handle  hFanHiBand;
SPINBOX_Handle  hSPNBX_DisplayHighBrightness;               //  lcd display backlight led brightness level for activ user interface (high level)
SPINBOX_Handle  hSPNBX_DisplayLowBrightness;                //  lcd display backlight led brightness level for activ screensaver (low level)
SPINBOX_Handle  hSPNBX_ScrnsvrTimeout;                      //  start screensaver (value x 10 s) after last touch event or disable screensaver for 0
SPINBOX_Handle  hSPNBX_ScrnsvrEnableHour;                   //  when to start display big digital clock for unused thermostat display 
SPINBOX_Handle  hSPNBX_ScrnsvrDisableHour;                  //  when to stop display big digital clock but to just dimm thermostat user interfaca
SPINBOX_Handle  hSPNBX_ScrnsvrClockColour;                  //  set colour for full display screensaver digital clock digits 
SPINBOX_Handle  hSPNBX_Hour;
SPINBOX_Handle  hSPNBX_Minute;
SPINBOX_Handle  hSPNBX_Day;
SPINBOX_Handle  hSPNBX_Month;
SPINBOX_Handle  hSPNBX_Year;
CHECKBOX_Handle hCHKBX_ScrnsvrClock;                        //  select full display screensaver digital clock 
CHECKBOX_Handle hCHKBX_ScrnsvrLogoClock;                    //  select user logo size screensaver digital clock
DROPDOWN_Handle hDRPDN_WeekDay;
static uint32_t clk_clrs[COLOR_BSIZE] = {                   //  selectable screensaver clock colours
    GUI_GRAY, GUI_RED, GUI_BLUE, GUI_GREEN, GUI_CYAN, GUI_MAGENTA, 
    GUI_YELLOW, GUI_LIGHTGRAY, GUI_LIGHTRED, GUI_LIGHTBLUE, GUI_LIGHTGREEN, 
    GUI_LIGHTCYAN, GUI_LIGHTMAGENTA, GUI_LIGHTYELLOW, GUI_DARKGRAY, GUI_DARKRED,
    GUI_DARKBLUE,  GUI_DARKGREEN, GUI_DARKCYAN, GUI_DARKMAGENTA, GUI_DARKYELLOW, 
    GUI_WHITE, GUI_BROWN, GUI_ORANGE, CLR_DARK_BLUE, CLR_LIGHT_BLUE, CLR_BLUE, CLR_LEMON
};
static const char * _acContent[] = {
  "PON",
  "UTO",
  "SRI",
  "CET",
  "PET",
  "SUB",
  "NED"
};
CtrlTypeDef Ctrl1;
/* Private Macro   -----------------------------------------------------------*/
/* Private Variable ----------------------------------------------------------*/
char oskin[6];
uint32_t rtc_tmr,out1_tmr,refresh_tmr;
uint32_t dispfl,lightauto,onofftmr,keypadtmr,keypadtout;
uint32_t menu_tmr,clean_tmr,clrtmr,guitmr,scrnsvr_tmr,rtctmr;
uint8_t btn_ok_state,btnset,low_bcklght,high_bcklght,light_ldr;
uint8_t menu_alarm,menu_light,menu_clean,menu_thst,menu_lc,btndec,_btndec;
uint8_t scrnsvr_ena_hour,scrnsvr_dis_hour,scrnsvr_clk_clr,scrnsvr_tout,oskb;
uint8_t menu_dim,menu_rel123,menu_out1,thsta,kyret,fwmsg=2,old_min=60;
uint8_t screen,ctrl1,ctrl2,ctrl3,disp_rot,menu_screen,last_state,btninc,_btninc;
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
static void DISPDateTime(void);
static void DISPKillSet1Scrn(void);
static void DISPKillSet2Scrn(void);
static void DISPKillSet3Scrn(void);
static void DISPInitSet1Scrn(void);
static void DISPInitSet2Scrn(void);
static void DISPInitSet3Scrn(void);
static void DISPCreateKeypad(void);
static void DISPDeleteKeypad(void);
static uint8_t DISPKeypad(char key);
static void SaveController(CtrlTypeDef* lc, uint16_t addr);
static void ReadController(CtrlTypeDef* lc, uint16_t addr);
/* Program Code  -------------------------------------------------------------*/
void DISPInit(void){
    GUI_Init();
    GUI_PID_SetHook(PID_Hook);
    WM_MULTIBUF_Enable(1);
    GUI_UC_SetEncodeUTF8();
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    ReadController(&Ctrl1, EE_CTRL1);
    GUI_Exec();
}

void DISPService(void){
    uint8_t ebuf[8];
    GUI_PID_STATE Tsta;
    
    if (HAL_GetTick() - guitmr >= 100){
        guitmr = HAL_GetTick();
        GUI_Exec();
    }
    
    if (IsFwUpdateActiv()){
        if (!fwmsg){
            fwmsg = 1;
            GUI_MULTIBUF_BeginEx(1);
            GUI_Clear();
            GUI_SetFont(GUI_FONT_24B_1);
            GUI_SetColor(GUI_ORANGE);
            GUI_SetTextMode(GUI_TM_TRANS);
            GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
            GUI_GotoXY(240, 135);
            GUI_DispString("FIRMWARE UPDATE RUN\n\rPLEASE WAIT");
            GUI_MULTIBUF_EndEx(1);
            DISPResetScrnsvr();
        }
        return;
    } else if(fwmsg == 1){
        fwmsg = 0;
        scrnsvr_tmr = 0;
    } else if(fwmsg == 2){
        GUI_Clear();
        Ctrl1.Main.value = 1;
        onofftmr = 0;
        screen = 1;
        fwmsg = 0;
        ctrl1 = 1;
    }
    
    switch(screen){
        //
        //  MAIN LIGHT ON/OFF/DIMM
        //
        case 1:
        {
            if (onofftmr) break;
            onofftmr = HAL_GetTick();
            GUI_MULTIBUF_BeginEx(1);
            GUI_Clear();
            GUI_SetPenSize(9);
            if      ( ctrl1 && !Ctrl1.Main.value) GUI_ClearRect(190,80,290,180), Ctrl1.Main.value=1, GUI_SetColor(GUI_GREEN),GUI_DrawEllipse(240,136,50,50), Light1On();
            else if ( ctrl1 &&  Ctrl1.Main.value) GUI_ClearRect(190,80,290,180), Ctrl1.Main.value=0, GUI_SetColor(GUI_RED),  GUI_DrawEllipse(240,136,50,50), Light1Off();
            else if (!ctrl1 && !Ctrl1.Main.value) GUI_ClearRect(190,80,290,180),                     GUI_SetColor(GUI_RED),  GUI_DrawEllipse(240,136,50,50), Light1Off();
            else if (!ctrl1 &&  Ctrl1.Main.value) GUI_ClearRect(190,80,290,180),                     GUI_SetColor(GUI_GREEN),GUI_DrawEllipse(240,136,50,50), Light1On();
            GUI_DrawLine(400,20,450,20);
            GUI_DrawLine(400,40,450,40);
            GUI_DrawLine(400,60,450,60);
            GUI_MULTIBUF_EndEx(1);
            menu_screen = 0;
            menu_light = 0;
            menu_alarm = 0;
            old_min = 60;
            rtctmr = 0;
            screen = 0;
            break;
        }
        //
        //  SELECT  SCREEN
        //
        case 2:
        {
            if (menu_screen==0){
                ++menu_screen;
                GUI_MULTIBUF_BeginEx(1);
                GUI_Clear();
                GUI_SetPenSize(9);
                GUI_SetColor(GUI_RED);
                GUI_DrawLine( 20,136,220,136);
                GUI_DrawLine(260,136,460,136);
                GUI_DrawLine(240, 20,240,116);
                GUI_DrawLine(240,156,240,252);
                GUI_DrawBitmap(&bmCLIMATE,  80, 23);
                GUI_DrawBitmap(&bmSETTINGS,320, 23);
                GUI_DrawBitmap(&bmLIGHT,    80,159);
                GUI_DrawBitmap(&bmCLEAN,   320,159);
                GUI_SetFont(GUI_FONT_24B_1);
                GUI_SetColor(GUI_ORANGE);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                GUI_GotoXY(120,120);
                GUI_DispString("CLIMATE");
                GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                GUI_GotoXY(360,120);
                GUI_DispString("SETTINGS");
                GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                GUI_GotoXY(120,250);
                GUI_DispString("LIGHT");
                GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                GUI_GotoXY(360,250);
                GUI_DispString("CLEAN");
                GUI_MULTIBUF_EndEx(1);
                menu_alarm=0;
                menu_light=0; 
                menu_thst=0;
            }           
            break;
        }
        //
        //  ALARM CONTROL
        //
        case 3:
        {
            switch (menu_alarm){
                case 0:
                {
                    oskb = 0;
                    menu_alarm = 1;
                    DISPCreateKeypad();
                    keypadtmr = HAL_GetTick();
                    ZEROFILL(oskin, sizeof(oskin));
                    GUI_SetPenSize(9);            
                    GUI_SetColor(GUI_GREEN);
                    GUI_DrawLine(400,20,450,20);
                    GUI_DrawLine(400,40,450,40);
                    GUI_DrawLine(400,60,450,60);
                    keypadtout = KEYPAD_TOUT;
                    while(GUI_PID_GetState(&Tsta)){
                        TS_Service();
                        GUI_Delay(KEY_PAUSE);
                    }
                    break;
                }
                
                case 1:
                {
                    if (HAL_GetTick() - keypadtmr >= keypadtout){
                        DISPDeleteKeypad();
                        screen = 6;
                    }else{
                        if      (BUTTON_IsPressed(hBUTTON_OSK_0)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_0)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('0');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_1)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_1)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('1');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_2)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_2)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('2');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_3)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_3)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('3');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_4)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_4)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('4');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_5)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_5)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('5');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_6)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_6)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(50);
                            }
                            kyret = DISPKeypad('6');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_7)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_7)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('7');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_8)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_8)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('8');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_9)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_9)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('9');
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_OK)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_OK)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('O');
                            DISPDeleteKeypad();
                            if      (kyret == 0){
                                screen = 6;
                            }else if(kyret == 1){
                                menu_alarm = 2;
                            }else if(kyret == 2){
                                DISPInitSet1Scrn();
                                screen = 7;
                            }                            
                            kyret = 0;
                        }else if (BUTTON_IsPressed(hBUTTON_OSK_BK)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_BK)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('B');
                        }
                    }
                    break;
                }
            }
            break;
        }
        //
        //  THERMOSTAT CONTROL
        //
        case 4:
        {
            if (menu_thst==0){
                ++menu_thst;
                GUI_SelectLayer(0);
                GUI_SetColor(GUI_BLACK);
                GUI_Clear();
                GUI_BMP_Draw(&thstat,0,0);
                GUI_SelectLayer(1);
                GUI_SetBkColor(GUI_TRANSPARENT);
                GUI_Clear();
                GUI_SetPenSize(9);            
                GUI_SetColor(GUI_GREEN);
                GUI_DrawLine(400,20,450,20);
                GUI_DrawLine(400,40,450,40);
                GUI_DrawLine(400,60,450,60);
                DISPSetPoint();
                DISPDateTime();
                MVUpdateSet();
                menu_dim=0;
                menu_rel123=0;
                menu_out1=0;
                menu_lc=0;
            } else if (menu_thst==1){
                /************************************/
                /*      SETPOINT  VALUE  INCREASED  */
                /************************************/
                if ( btninc&& !_btninc){
                    _btninc=1;
                    if (thst.sp_temp < thst.sp_max) ++thst.sp_temp;
                    SaveThermostatController(&thst, EE_THST1);
                    DISPSetPoint();
                } else if (!btninc&&  _btninc) _btninc=0;
                /************************************/
                /*      SETPOINT  VALUE  DECREASED  */
                /************************************/
                if ( btndec&& !_btndec){
                    _btndec=1;
                    if (thst.sp_temp > thst.sp_min)  --thst.sp_temp;
                    SaveThermostatController(&thst, EE_THST1);
                    DISPSetPoint();
                } else if (!btndec&&  _btndec) _btndec=0;
                /** ==========================================================================*/
                /**   R E W R I T E   A N D   S A V E   N E W   S E T P O I N T   V A L U E   */
                /** ==========================================================================*/
                if  (IsMVUpdateActiv()){
                    MVUpdateReset();
                    GUI_MULTIBUF_BeginEx(1);
                    GUI_ClearRect(410,185,480,235);
                    GUI_ClearRect(310,230,480,255);
                    GUI_GotoXY(310,242);
                    GUI_SetFont(GUI_FONT_20_1);
                    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                    if (IsTempRegHeating()){
                        GUI_SetColor(GUI_RED);
                        GUI_DispString("HEATING");
                        GUI_GotoXY(415,220);
                        GUI_SetColor(GUI_WHITE);
                    } else if (IsTempRegCooling()){
                        GUI_SetColor(GUI_BLUE);
                        GUI_DispString("COOLING");
                        GUI_GotoXY(415,197);
                        GUI_SetColor(GUI_ORANGE);
                    }
                    GUI_SetFont(GUI_FONT_24_1);
                    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                    GUI_DispSDec(thst.mv_temp/10,3);
                    GUI_DispString("°c");
                    GUI_MULTIBUF_EndEx(1);
                }
            }
            break;
        }
        //
        //  CLEAN DISPLAY
        //
        case 5:
        {
            if (menu_clean == 0){
                ++menu_clean;
                GUI_Clear();
                clrtmr = 30;
                menu_alarm = 0;
                menu_light = 0;
                menu_screen = 0;
            }else if (menu_clean == 1){
                if (HAL_GetTick()-clean_tmr >= 1000){
                    clean_tmr = HAL_GetTick();
                    GUI_MULTIBUF_BeginEx(1);
                    GUI_ClearRect(0,50,480,200);
                    if (clrtmr > 5) GUI_SetColor(GUI_GREEN);
                    else GUI_SetColor(GUI_RED);
                    GUI_SetFont(GUI_FONT_32_1);
                    GUI_SetTextMode(GUI_TM_TRANS);
                    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                    GUI_GotoXY(240, 80);
                    GUI_DispString("DISPLAY CLEAN TIMEOUT:");
                    GUI_SetFont(GUI_FONT_D64);
                    GUI_SetTextMode(GUI_TM_TRANS);
                    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                    GUI_GotoXY(240, 156);
                    GUI_DispDecMin(clrtmr);
                    GUI_MULTIBUF_EndEx(1);
                    if (clrtmr) clrtmr--;
                    else screen = 6;
                }
            }
            break;
        }
        //
        //  RETURN TO FIRST MENU
        //
        case 6:
        {
            GUI_SelectLayer(0);
            GUI_SetColor(GUI_BLACK);
            GUI_Clear();
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT);
            GUI_Clear();
            if (IsDISPKeypadActiv()) DISPDeleteKeypad();
            menu_screen = 0;
            menu_rel123 = 0;
            menu_alarm = 0;
            menu_light = 0;
            menu_clean = 0;
            menu_out1 = 0;
            menu_thst = 0;
            onofftmr = 0;
            menu_dim = 0;
            menu_lc = 0;
            screen = 1;
            break;
        }
        //
        //  SETTINGS MENU 1
        //
        case 7:
        {
            if (rtctm.Hours != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Hour))){
                rtctm.Hours = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Hour));
                HAL_RTC_SetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
                RtcTimeValidSet();
            }            
            if (rtctm.Minutes != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Minute))){
                rtctm.Minutes = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Minute));
                HAL_RTC_SetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
                RtcTimeValidSet();
            }            
            if (rtcdt.Date != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Day))){
                rtcdt.Date = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Day));
                HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
                RtcTimeValidSet();
            }            
            if (rtcdt.Month != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Month))){
                rtcdt.Month = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Month));
                HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
                RtcTimeValidSet();
            }            
            if (rtcdt.Year != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Year) - 2000)){
                rtcdt.Year = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Year) - 2000);
                HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
                RtcTimeValidSet();
            }
            if (scrnsvr_clk_clr != SPINBOX_GetValue(hSPNBX_ScrnsvrClockColour)){
                scrnsvr_clk_clr = SPINBOX_GetValue(hSPNBX_ScrnsvrClockColour);
                GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
                GUI_FillRect(340, 51, 430, 59);
            }
            if (CHECKBOX_GetState(hCHKBX_ScrnsvrClock) == 1) ScrnsvrClkSet();
            else ScrnsvrClkReset();
            if (rtcdt.WeekDay != Dec2Bcd(DROPDOWN_GetSel(hDRPDN_WeekDay)+1)){
                rtcdt.WeekDay  = Dec2Bcd(DROPDOWN_GetSel(hDRPDN_WeekDay)+1);
                HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
                RtcTimeValidSet();
            }
            high_bcklght = SPINBOX_GetValue(hSPNBX_DisplayHighBrightness);
            low_bcklght = SPINBOX_GetValue(hSPNBX_DisplayLowBrightness);
            scrnsvr_tout = SPINBOX_GetValue(hSPNBX_ScrnsvrTimeout);
            scrnsvr_ena_hour = SPINBOX_GetValue(hSPNBX_ScrnsvrEnableHour);
            scrnsvr_dis_hour = SPINBOX_GetValue(hSPNBX_ScrnsvrDisableHour);
            scrnsvr_clk_clr = SPINBOX_GetValue(hSPNBX_ScrnsvrClockColour);            
            if (BUTTON_IsPressed(hBUTTON_Ok)){
                if (thsta){
                    thsta = 0;
                    SaveThermostatController(&thst, EE_THST1);
                }
                SaveController(&Ctrl1, EE_CTRL1);
                ebuf[0] = low_bcklght;
                ebuf[1] = high_bcklght;
                ebuf[2] = scrnsvr_tout;
                ebuf[3] = scrnsvr_ena_hour;
                ebuf[4] = scrnsvr_dis_hour;
                ebuf[5] = scrnsvr_clk_clr;
                if(IsScrnsvrClkActiv()) ebuf[6] = 1;
                else ebuf[6] = 0;
                EE_WriteBuffer(ebuf, EE_DISP_LOW_BCKLGHT, 7);
                EE_WriteBuffer(&tfifa, EE_TFIFA, 1);
                DISPKillSet1Scrn();
                screen = 6;
            } else if (BUTTON_IsPressed(hBUTTON_Next)){
                DISPKillSet1Scrn();
                DISPInitSet2Scrn();
                screen = 8;
            }
            break;
        }
        //
        //  SETTINGS MENU 2
        //
        case 8:
        {
            tfifa = SPINBOX_GetValue(hDEV_ID);           
            Ctrl1.Main.index   = SPINBOX_GetValue(hBIN_MAIN);
            Ctrl1.Alarm1.index  = SPINBOX_GetValue(hBIN_ALARM1);
            Ctrl1.Alarm2.index  = SPINBOX_GetValue(hBIN_ALARM2);
            Ctrl1.Alarm3.index  = SPINBOX_GetValue(hBIN_ALARM3);        
            if (BUTTON_IsPressed(hBUTTON_Ok)){   
                if (thsta){
                    thsta = 0;
                    SaveThermostatController(&thst, EE_THST1);
                }                
                SaveController(&Ctrl1, EE_CTRL1);
                ebuf[0] = low_bcklght;
                ebuf[1] = high_bcklght;
                ebuf[2] = scrnsvr_tout;
                ebuf[3] = scrnsvr_ena_hour;
                ebuf[4] = scrnsvr_dis_hour;
                ebuf[5] = scrnsvr_clk_clr;
                if(IsScrnsvrClkActiv()) ebuf[6] = 1;
                else ebuf[6] = 0;
                EE_WriteBuffer(ebuf, EE_DISP_LOW_BCKLGHT, 7);
                EE_WriteBuffer(&tfifa, EE_TFIFA, 1);
                DISPKillSet2Scrn();
                screen = 6;
            } else if (BUTTON_IsPressed(hBUTTON_Next)){
                DISPKillSet2Scrn();
                DISPInitSet3Scrn();
                screen = 9;
            }
            break;
        }
        //
        //  SETTINGS MENU 3
        //
        case 9:
        {            
            /** ==========================================================================*/
            /**    S E T T I N G S     M E N U     U S E R     I N P U T   U P D A T E    */
            /** ==========================================================================*/
            if (thst.th_ctrl    != RADIO_GetValue(hThstControl)){
                thst.th_ctrl  = RADIO_GetValue(hThstControl);
                ++thsta;
            }
            if (thst.fan_ctrl   != RADIO_GetValue(hFanControl)){
                thst.fan_ctrl  = RADIO_GetValue(hFanControl);
                ++thsta;
            }
            if (thst.sp_max     != SPINBOX_GetValue(hThstMaxSetPoint)){
                thst.sp_max  = SPINBOX_GetValue(hThstMaxSetPoint);
                ++thsta;
            }            
            if (thst.sp_min     != SPINBOX_GetValue(hThstMinSetPoint)){
                thst.sp_min  = SPINBOX_GetValue(hThstMinSetPoint);
                ++thsta;
            }
            if (thst.fan_diff   != SPINBOX_GetValue(hFanDiff)){
                thst.fan_diff  = SPINBOX_GetValue(hFanDiff);
                ++thsta;
            }
            if (thst.fan_loband != SPINBOX_GetValue(hFanLowBand)){
                thst.fan_loband  = SPINBOX_GetValue(hFanLowBand);
                ++thsta;
            }
            if (thst.fan_hiband != SPINBOX_GetValue(hFanHiBand)){
                thst.fan_hiband  = SPINBOX_GetValue(hFanHiBand);
                ++thsta;
            }           
            if (BUTTON_IsPressed(hBUTTON_Ok)){
                if (thsta){
                    thsta = 0;
                    SaveThermostatController(&thst, EE_THST1);
                }
                SaveController(&Ctrl1, EE_CTRL1);
                ebuf[0] = low_bcklght;
                ebuf[1] = high_bcklght;
                ebuf[2] = scrnsvr_tout;
                ebuf[3] = scrnsvr_ena_hour;
                ebuf[4] = scrnsvr_dis_hour;
                ebuf[5] = scrnsvr_clk_clr;
                if(IsScrnsvrClkActiv()) ebuf[6] = 1;
                else ebuf[6] = 0;
                EE_WriteBuffer(ebuf, EE_DISP_LOW_BCKLGHT, 7);
                EE_WriteBuffer(&tfifa, EE_TFIFA, 1);
                DISPKillSet3Scrn();
                screen = 6;
            } else if (BUTTON_IsPressed(hBUTTON_Next)){                
                DISPKillSet3Scrn();
                DISPInitSet1Scrn();
                screen = 7;
            }
            break;
        }
        //
        //  RESET FLAGS
        //        
        case 0:
        default:
        {
            menu_screen = 0;
            menu_alarm = 0;
            menu_light = 0;
            break;
        }
    }
    if (!IsScrnsvrActiv()){
        if (HAL_GetTick() - scrnsvr_tmr >= (uint32_t)(scrnsvr_tout*1000)){
            if      (screen==7) DISPKillSet1Scrn();
            else if (screen==8) DISPKillSet2Scrn();
            else if (screen==9) DISPKillSet3Scrn();
            DISPSetBrightnes(low_bcklght);
            ScrnsvrInitReset();
            ScrnsvrSet();
            screen = 6;
        }        
    }
    if (onofftmr){
        if (HAL_GetTick()-onofftmr >= EVENT_ONOFF_TOUT){
            onofftmr = 0;
        }
    }
    
    if (HAL_GetTick() - rtctmr >= 1000){
        rtctmr = HAL_GetTick();
        if (screen < 2) DISPDateTime();
        if (!IsScrnsvrActiv()) MVUpdateSet(); 
    }
}

void DISPSetBrightnes(uint8_t val){
    if      (val < DISP_BRGHT_MIN) val = DISP_BRGHT_MIN;
    else if (val > DISP_BRGHT_MAX) val = DISP_BRGHT_MAX;
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, (uint16_t) (val * 10));
}

void DISPSetPoint(void){
    GUI_MULTIBUF_BeginEx(1);
    GUI_ClearRect(SP_H_POS - 5, SP_V_POS - 5, SP_H_POS + 120, SP_V_POS + 85);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_D48);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextAlign(GUI_TA_RIGHT);
    GUI_GotoXY(SP_H_POS, SP_V_POS);
    GUI_DispDec(thst.sp_temp , 2);
    GUI_MULTIBUF_EndEx(1);
}

void DISPResetScrnsvr(void){
    if(IsScrnsvrActiv() && IsScrnsvrEnabled()) screen = 6;
    ScrnsvrReset();
    ScrnsvrInitReset();
    scrnsvr_tmr = HAL_GetTick();
    DISPSetBrightnes(high_bcklght);
}

void PID_Hook(GUI_PID_STATE * pTS){
    uint8_t click = 0;
    if (pTS->Pressed == 1){
        pTS->Layer = 1;
        if ((pTS->y > 60) && (pTS->y< 200) && (screen == 3) && (menu_alarm > 1)){
            click = 1;
            ctrl1 = 0;
            ctrl2 = 0;
            ctrl3 = 0;
            if      (pTS->x > 320)  ctrl3 = 1;
            else if (pTS->x > 160)  ctrl2 = 1;
            else                    ctrl1 = 1;
        }else if((pTS->x > 420) && (pTS->y < 60) && (screen < 5)){
            click = 1;
            if  (screen == 0) screen = 2;
            else screen = 6;
        }else if((pTS->x > 240) && (pTS->x < 420) && (pTS->y > 40) && (pTS->y < 136) && (screen == 2)){
            click = 1;
            screen = 3;
        }else if((pTS->x < 240) && (pTS->y < 136) && (screen == 2)){
            click = 1;
            screen = 4;
        }else if((pTS->x > 240) && (pTS->y > 136) && (screen == 2)){
            click = 1;
            screen = 5;
        }else if((pTS->y > 136) && (pTS->x < 240) && (screen == 2)){
            click = 1;
            screen = 6;
        }else if((pTS->x > BTN_INC_X0) && (pTS->y > BTN_INC_Y0) && (pTS->x < BTN_INC_X1) && (pTS->y < BTN_INC_Y1) && (screen == 4)){
            click = 1;
            btninc = 1;
        }else if((pTS->x > BTN_DEC_X0) && (pTS->y > BTN_DEC_Y0) && (pTS->x < BTN_DEC_X1) && (pTS->y < BTN_DEC_Y1) && (screen == 4)){
            click = 1;
            btndec = 1;
        }else if((pTS->x < 420) && (pTS->y > 60) && (screen == 0)){
            if (!onofftmr){
                ctrl1 = 1;
                click = 1;
                screen = 1;                
            }
        }
        if (click){
            BuzzerOn();
            HAL_Delay(1);
            BuzzerOff();
        }
    }else{
        if(screen == 1) screen =0 ;
        btnset = 0;
        btndec = 0;   
        btninc = 0;
        ctrl1 = 0;
        ctrl2 = 0;
        ctrl3 = 0;
    }
    DISPResetScrnsvr(); 
}

static void DISPDateTime(void){
    char dbuf[32];
    static uint8_t old_day = 0;
    if(!IsRtcTimeValid()) return; // nothing to display untill system rtc validated
    HAL_RTC_GetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
    /************************************/
    /*   CHECK IS SCREENSAVER ENABLED   */
    /************************************/ 
    if(scrnsvr_ena_hour>=scrnsvr_dis_hour){
        if      (Bcd2Dec(rtctm.Hours)>=scrnsvr_ena_hour) ScrnsvrEnable();
        else if (Bcd2Dec(rtctm.Hours)<scrnsvr_dis_hour) ScrnsvrEnable();
        else if (IsScrnsvrEnabled())ScrnsvrDisable(), screen = 6;
    }else if(scrnsvr_ena_hour<scrnsvr_dis_hour){
        if((Bcd2Dec(rtctm.Hours<scrnsvr_dis_hour))&&(Bcd2Dec(rtctm.Hours)>=scrnsvr_ena_hour)) ScrnsvrEnable();
        else if(IsScrnsvrEnabled()) ScrnsvrDisable(), screen = 6;
    }
    /************************************/
    /*      DISPLAY  DATE  &  TIME      */
    /************************************/ 
    if (IsScrnsvrActiv()&&IsScrnsvrEnabled()&&IsScrnsvrClkActiv()){
        if(!IsScrnsvrInitActiv()||(old_day!=rtcdt.WeekDay)){
            ScrnsvrInitSet();
            GUI_MULTIBUF_BeginEx(0);
            GUI_SelectLayer(0);
            GUI_Clear();
            GUI_MULTIBUF_EndEx(0);
            GUI_MULTIBUF_BeginEx(1);
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT); 
            GUI_Clear();
            old_min=60U;
            old_day=rtcdt.WeekDay;
            GUI_SetPenSize(9);
            GUI_SetColor(GUI_GREEN);
            GUI_DrawLine(400,20,450,20);
            GUI_DrawLine(400,40,450,40);
            GUI_DrawLine(400,60,450,60);
            GUI_MULTIBUF_EndEx(1);
        }        
        HEX2STR(dbuf,&rtctm.Hours);
        if(rtctm.Seconds&1) dbuf[2]=':';
        else dbuf[2]=' ';
        HEX2STR(&dbuf[3],&rtctm.Minutes);
        GUI_GotoXY(CLOCK_H_POS, CLOCK_V_POS);
        GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
        GUI_SetFont(GUI_FONT_D80);
        GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
        GUI_MULTIBUF_BeginEx(1);
        GUI_ClearRect(0,80,480,192);
        GUI_ClearRect(0,220,100,270);
        GUI_DispString(dbuf);
        if(rtcdt.WeekDay==0) rtcdt.WeekDay=7;
        if(rtcdt.WeekDay==1) memcpy(dbuf,"  Monday  ",10);
        else if(rtcdt.WeekDay==2) memcpy(dbuf," Tuestday ",10);
        else if(rtcdt.WeekDay==3) memcpy(dbuf,"Wednesday ",10);
        else if(rtcdt.WeekDay==4) memcpy(dbuf,"Thurstday ",10);
        else if(rtcdt.WeekDay==5) memcpy(dbuf,"  Friday  ",10);
        else if(rtcdt.WeekDay==6) memcpy(dbuf," Saturday ",10);
        else if(rtcdt.WeekDay==7) memcpy(dbuf,"  Sunday  ",10);
        HEX2STR(&dbuf[10],&rtcdt.Date);
        if(rtcdt.Month==1) memcpy(&dbuf[12],". January ",10);		
        else if(rtcdt.Month==2) memcpy(&dbuf[12],". February",10);
        else if(rtcdt.Month==3) memcpy(&dbuf[12],".  March  ",10);
        else if(rtcdt.Month==4) memcpy(&dbuf[12],".  April  ",10);
        else if(rtcdt.Month==5) memcpy(&dbuf[12],".   May   ",10);
        else if(rtcdt.Month==6) memcpy(&dbuf[12],".   June  ",10);
        else if(rtcdt.Month==7) memcpy(&dbuf[12],".   July  ",10);
        else if(rtcdt.Month==8) memcpy(&dbuf[12],". August  ",10);
        else if(rtcdt.Month==9) memcpy(&dbuf[12],".September",10);
        else if(rtcdt.Month==0x10) memcpy(&dbuf[12],". October ",10);
        else if(rtcdt.Month==0x11) memcpy(&dbuf[12],". November",10);
        else if(rtcdt.Month==0x12) memcpy(&dbuf[12],". December",10);
        memcpy(&dbuf[22]," 20",3U);
        HEX2STR(&dbuf[25],&rtcdt.Year);
        dbuf[27]='.';
        dbuf[28]=NUL;
        GUI_SetFont(GUI_FONT_24B_1);
        GUI_GotoXY(CLOCK_H_POS,CLOCK_V_POS+70);
        GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
        GUI_DispString(dbuf);
        GUI_MULTIBUF_EndEx(1); 
    }else if(old_min != rtctm.Minutes){
        old_min = rtctm.Minutes;
        HEX2STR(dbuf,&rtctm.Hours);
        dbuf[2]=':';
        HEX2STR(&dbuf[3],&rtctm.Minutes);
        GUI_SetFont(GUI_FONT_32_1);
        GUI_SetColor(GUI_WHITE);
        GUI_SetTextMode(GUI_TM_TRANS);
        GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
        GUI_MULTIBUF_BeginEx(1);
        GUI_GotoXY(5,245);
        GUI_ClearRect(0,220,100,270);
        GUI_DispString(dbuf);
        GUI_MULTIBUF_EndEx(1);
    }
    
    if (old_day != rtcdt.WeekDay){
        old_day  = rtcdt.WeekDay;
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, rtcdt.Date);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, rtcdt.Month);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, rtcdt.WeekDay);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, rtcdt.Year);
    }
}

static void DISPInitSet1Scrn(void){
    int i;
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    HAL_RTC_GetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
    hSPNBX_DisplayHighBrightness = SPINBOX_CreateEx(10, 20, 90, 30, 0, WM_CF_SHOW, ID_DisplayHighBrightness, 1, 90);
    SPINBOX_SetEdge(hSPNBX_DisplayHighBrightness, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_DisplayHighBrightness, high_bcklght);
    hSPNBX_DisplayLowBrightness = SPINBOX_CreateEx(10, 60, 90, 30, 0, WM_CF_SHOW, ID_DisplayLowBrightness, 1, 90);
    SPINBOX_SetEdge(hSPNBX_DisplayLowBrightness, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_DisplayLowBrightness, low_bcklght);
    hSPNBX_ScrnsvrTimeout = SPINBOX_CreateEx(10, 130, 90, 30, 0, WM_CF_SHOW, ID_ScrnsvrTimeout, 1, 240);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrTimeout, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrTimeout, scrnsvr_tout);
    hSPNBX_ScrnsvrEnableHour = SPINBOX_CreateEx(10, 170, 90, 30, 0, WM_CF_SHOW, ID_ScrnsvrEnableHour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrEnableHour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrEnableHour, scrnsvr_ena_hour);
    hSPNBX_ScrnsvrDisableHour = SPINBOX_CreateEx(10, 210, 90, 30, 0, WM_CF_SHOW, ID_ScrnsvrDisableHour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrDisableHour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrDisableHour, scrnsvr_dis_hour);
    hSPNBX_Hour = SPINBOX_CreateEx(190, 20, 90, 30, 0, WM_CF_SHOW, ID_Hour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_Hour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Hour, Bcd2Dec(rtctm.Hours));    
    hSPNBX_Minute = SPINBOX_CreateEx(190, 60, 90, 30, 0, WM_CF_SHOW, ID_Minute, 0, 59);
    SPINBOX_SetEdge(hSPNBX_Minute, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Minute, Bcd2Dec(rtctm.Minutes));    
    hSPNBX_Day = SPINBOX_CreateEx(190, 130, 90, 30, 0, WM_CF_SHOW, ID_Day, 1, 31);
    SPINBOX_SetEdge(hSPNBX_Day, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Day, Bcd2Dec(rtcdt.Date));    
    hSPNBX_Month = SPINBOX_CreateEx(190, 170, 90, 30, 0, WM_CF_SHOW, ID_Month, 1, 12);
    SPINBOX_SetEdge(hSPNBX_Month, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Month, Bcd2Dec(rtcdt.Month));    
    hSPNBX_Year = SPINBOX_CreateEx(190, 210, 90, 30, 0, WM_CF_SHOW, ID_Year, 2000, 2099);
    SPINBOX_SetEdge(hSPNBX_Year, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Year, (Bcd2Dec(rtcdt.Year) + 2000));
    hSPNBX_ScrnsvrClockColour = SPINBOX_CreateEx(340, 20, 90, 30, 0, WM_CF_SHOW, ID_ScrnsvrClkColour, 1, COLOR_BSIZE);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrClockColour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrClockColour, scrnsvr_clk_clr);
    hCHKBX_ScrnsvrClock = CHECKBOX_Create(340, 70, 110, 20, 0, ID_ScrnsvrClock, WM_CF_SHOW);
    CHECKBOX_SetTextColor(hCHKBX_ScrnsvrClock, GUI_GREEN);	
    CHECKBOX_SetText(hCHKBX_ScrnsvrClock, "SCREENSAVER");
    if(IsScrnsvrClkActiv()) CHECKBOX_SetState(hCHKBX_ScrnsvrClock, 1);
    else CHECKBOX_SetState(hCHKBX_ScrnsvrClock, 0);
    hDRPDN_WeekDay = DROPDOWN_CreateEx(340, 100, 130, 100, 0, WM_CF_SHOW, DROPDOWN_CF_AUTOSCROLLBAR, ID_WeekDay);
    for (i = 0; i < GUI_COUNTOF(_acContent); i++) {
        DROPDOWN_AddString(hDRPDN_WeekDay, *(_acContent + i));
    }
    DROPDOWN_SetSel(hDRPDN_WeekDay, rtcdt.WeekDay-1);
    hBUTTON_Next = BUTTON_Create(340, 180, 130, 30, ID_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");
    hBUTTON_Ok = BUTTON_Create(340, 230, 130, 30, ID_Ok, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Ok, "SAVE");
    GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
    GUI_FillRect(340, 51, 430, 59);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_13_1);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    /************************************/
    /* DISPLAY BACKLIGHT LED BRIGHTNESS */
    /************************************/
    GUI_DrawHLine   ( 15,   5, 160);
    GUI_GotoXY      ( 10,   5);
    GUI_DispString  ("DISPLAY BACKLIGHT");
    GUI_GotoXY      (110,  35);
    GUI_DispString  ("HIGH");
    GUI_GotoXY      (110,  75);
    GUI_DispString  ("LOW");
    GUI_DrawHLine   ( 15, 185, 320);
    GUI_GotoXY      (190,   5);
    /************************************/
    /*          SET        TIME         */
    /************************************/
    GUI_DispString  ("SET TIME");
    GUI_GotoXY      (290,  35);
    GUI_DispString  ("HOUR");
    GUI_GotoXY      (290,  75);
    GUI_DispString  ("MINUTE");
    GUI_DrawHLine   ( 15, 335, 475);
    /************************************/
    /*    SET SCREENSAVER CLOCK COOLOR  */
    /************************************/
    GUI_GotoXY      (340,   5);
    GUI_DispString  ("SET COLOR");
    GUI_GotoXY      (440, 26);
    GUI_DispString  ("FULL");
    GUI_GotoXY      (440, 38);
    GUI_DispString  ("CLOCK");
    /************************************/
    /*      SCREENSAVER     OPTION      */
    /************************************/ 
    GUI_DrawHLine   (125,   5, 160);
    GUI_GotoXY      ( 10, 115);
    GUI_DispString  ("SCREENSAVER OPTION");
    GUI_GotoXY      (110, 145);
    GUI_DispString  ("TIMEOUT");
    GUI_GotoXY      (110, 176);
    GUI_DispString  ("ENABLE");
    GUI_GotoXY      (110, 188);
    GUI_DispString  ("HOUR");
    GUI_GotoXY      (110, 216);
    GUI_DispString  ("DISABLE");
    GUI_GotoXY      (110, 228);
    GUI_DispString  ("HOUR");
    /************************************/
    /*          SET        DATE         */
    /************************************/
    GUI_DrawHLine   (125, 185, 320);
    GUI_GotoXY      (190, 115);
    GUI_DispString  ("SET DATE");
    GUI_GotoXY      (290, 145);
    GUI_DispString  ("DAY");
    GUI_GotoXY      (290, 185);
    GUI_DispString  ("MONTH");
    GUI_GotoXY      (290, 225);
    GUI_DispString  ("YEAR");
    GUI_MULTIBUF_EndEx(1);
}

static void DISPKillSet1Scrn(void){
    WM_DeleteWindow(hSPNBX_DisplayHighBrightness);
    WM_DeleteWindow(hSPNBX_DisplayLowBrightness);
    WM_DeleteWindow(hSPNBX_ScrnsvrDisableHour);
    WM_DeleteWindow(hSPNBX_ScrnsvrClockColour);
    WM_DeleteWindow(hSPNBX_ScrnsvrEnableHour);
    WM_DeleteWindow(hSPNBX_ScrnsvrTimeout);
    WM_DeleteWindow(hCHKBX_ScrnsvrClock);
    WM_DeleteWindow(hDRPDN_WeekDay);
    WM_DeleteWindow(hSPNBX_Minute);
    WM_DeleteWindow(hSPNBX_Month);
    WM_DeleteWindow(hBUTTON_Next);
    WM_DeleteWindow(hSPNBX_Hour);
    WM_DeleteWindow(hSPNBX_Year);
    WM_DeleteWindow(hSPNBX_Day);
    WM_DeleteWindow(hBUTTON_Ok);
}

static void DISPInitSet2Scrn(void){
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    hDEV_ID = SPINBOX_CreateEx(10, 10, 90, 30, 0, WM_CF_SHOW, ID_DEV_ID, 1, 254);
    SPINBOX_SetEdge(hDEV_ID, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hDEV_ID, tfifa);    
    hBIN_MAIN = SPINBOX_CreateEx(10, 50, 90, 30, 0, WM_CF_SHOW, ID_BIN_MAIN, 0, 24);
    SPINBOX_SetEdge(hBIN_MAIN, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_MAIN, Ctrl1.Main.index);     
    hBIN_ALARM1 = SPINBOX_CreateEx(10,90, 90, 30, 0, WM_CF_SHOW, ID_BIN_ALARM1, 0, 24);
    SPINBOX_SetEdge(hBIN_ALARM1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_ALARM1, Ctrl1.Alarm1.index);    
    hBIN_ALARM2 = SPINBOX_CreateEx(10,130, 90, 30, 0, WM_CF_SHOW, ID_BIN_ALARM2, 0, 24);
    SPINBOX_SetEdge(hBIN_ALARM2, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_ALARM2, Ctrl1.Alarm2.index);    
    hBIN_ALARM3 = SPINBOX_CreateEx(10, 170, 90, 30, 0, WM_CF_SHOW, ID_BIN_ALARM3, 0, 24);
    SPINBOX_SetEdge(hBIN_ALARM3, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_ALARM3, Ctrl1.Alarm3.index);   
    hBUTTON_Next = BUTTON_Create(340, 140, 130, 50, ID_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");
    hBUTTON_Ok = BUTTON_Create(340, 200, 130, 50, ID_Ok, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Ok, "SAVE");    
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_13_1);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);    
    GUI_GotoXY(110, 14);
    GUI_DispString("DEVICE");
    GUI_GotoXY(110, 26);
    GUI_DispString("BUS ID");    
    GUI_GotoXY(110, 54);
    GUI_DispString("MAIN SW.");
    GUI_GotoXY(110, 66);
    GUI_DispString("RELAY1");       
    GUI_GotoXY(110, 94);
    GUI_DispString("ALARM GUEST");
    GUI_GotoXY(110, 106);
    GUI_DispString("RELAY1");    
    GUI_GotoXY(110, 134);
    GUI_DispString("ALARM HOUSE");
    GUI_GotoXY(110, 146);
    GUI_DispString("RELAY1");    
    GUI_GotoXY(110, 174);
    GUI_DispString("ALARM FITNES");
    GUI_GotoXY(110, 186);
    GUI_DispString("RELAY1");
    GUI_MULTIBUF_EndEx(1);
}

static void DISPKillSet2Scrn(void){
    WM_DeleteWindow(hBUTTON_Next);
    WM_DeleteWindow(hBIN_ALARM1);
    WM_DeleteWindow(hBIN_ALARM2);
    WM_DeleteWindow(hBIN_ALARM3);
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBIN_MAIN);
    WM_DeleteWindow(hDEV_ID);
}

static void DISPInitSet3Scrn(void){
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    hThstControl = RADIO_CreateEx(10, 20, 150, 80, 0,WM_CF_SHOW, 0, ID_ThstControl, 3, 20);
    RADIO_SetTextColor(hThstControl, GUI_GREEN);
    RADIO_SetText(hThstControl, "OFF",     0);
    RADIO_SetText(hThstControl, "COOLING", 1);
    RADIO_SetText(hThstControl, "HEATING", 2);
    RADIO_SetValue(hThstControl, thst.th_ctrl);
    hFanControl = RADIO_CreateEx(10, 150, 150, 80, 0,WM_CF_SHOW, 0, ID_FanControl, 2, 20);
    RADIO_SetTextColor(hFanControl, GUI_GREEN);
    RADIO_SetText(hFanControl, "ON / OFF", 0);
    RADIO_SetText(hFanControl, "3 SPEED", 1);
    RADIO_SetValue(hFanControl, thst.fan_ctrl);
    hThstMaxSetPoint = SPINBOX_CreateEx(110, 20, 90, 30, 0, WM_CF_SHOW, ID_ThstMaxSetPoint, 15, 40);
    SPINBOX_SetEdge(hThstMaxSetPoint, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hThstMaxSetPoint, thst.sp_max);
    hThstMinSetPoint = SPINBOX_CreateEx(110, 70, 90, 30, 0, WM_CF_SHOW, ID_ThstMinSetPoint, 15, 40);
    SPINBOX_SetEdge(hThstMinSetPoint, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hThstMinSetPoint, thst.sp_min);
    hFanDiff = SPINBOX_CreateEx(110, 150, 90, 30, 0, WM_CF_SHOW, ID_FanDiff, 0, 10);
    SPINBOX_SetEdge(hFanDiff, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hFanDiff, thst.fan_diff);
    hFanLowBand = SPINBOX_CreateEx(110, 190, 90, 30, 0, WM_CF_SHOW, ID_FanLowBand, 0, 50);
    SPINBOX_SetEdge(hFanLowBand, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hFanLowBand, thst.fan_loband);
    hFanHiBand = SPINBOX_CreateEx(110, 230, 90, 30, 0, WM_CF_SHOW, ID_FanHiBand, 0, 100);
    SPINBOX_SetEdge(hFanHiBand, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hFanHiBand, thst.fan_hiband);
    hBUTTON_Next = BUTTON_Create(340, 140, 130, 50, ID_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");
    hBUTTON_Ok = BUTTON_Create(340, 200, 130, 50, ID_Ok, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Ok, "SAVE"); 
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_13_1);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_GotoXY(210, 24);
    GUI_DispString("MAX. USER SETPOINT");
    GUI_GotoXY(210, 36);
    GUI_DispString("TEMP. x1*C");
    GUI_GotoXY(210, 74);
    GUI_DispString("MIN. USER SETPOINT");
    GUI_GotoXY(210, 86);
    GUI_DispString("TEMP. x1*C");
    GUI_GotoXY(210, 154);
    GUI_DispString("FAN SPEED DIFFERENCE");
    GUI_GotoXY(210, 166);
    GUI_DispString("TEMP. x0.1*C");
    GUI_GotoXY(210, 194);
    GUI_DispString("FAN LOW SPEED BAND");
    GUI_GotoXY(210, 206);
    GUI_DispString("SETPOINT +/- x0.1*C");
    GUI_GotoXY(210, 234);
    GUI_DispString("FAN HI SPEED BAND");
    GUI_GotoXY(210, 246);
    GUI_DispString("SETPOINT +/- x0.1*C");    
    GUI_GotoXY(10, 4);
    GUI_DispString("THERMOSTAT CONTROL MODE");
    GUI_GotoXY(10, 120);
    GUI_DispString("FAN SPEED CONTROL MODE");
    GUI_DrawHLine(12, 5, 320);
    GUI_DrawHLine(130, 5, 320);
    GUI_MULTIBUF_EndEx(1);
}

static void DISPKillSet3Scrn(void){
    WM_DeleteWindow(hThstMaxSetPoint);
    WM_DeleteWindow(hThstMinSetPoint);
    WM_DeleteWindow(hBUTTON_Next);
    WM_DeleteWindow(hThstControl);
    WM_DeleteWindow(hFanControl);
    WM_DeleteWindow(hFanLowBand);
    WM_DeleteWindow(hFanHiBand);
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hFanDiff);
}

static void DISPCreateKeypad(void){
	GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
	hBUTTON_OSK_1   = BUTTON_Create(100,  50, 60, 60, ID_OSK_1, WM_CF_SHOW);	
	hBUTTON_OSK_2   = BUTTON_Create(170,  50, 60, 60, ID_OSK_2, WM_CF_SHOW);	
	hBUTTON_OSK_3   = BUTTON_Create(240,  50, 60, 60, ID_OSK_3, WM_CF_SHOW);	
	hBUTTON_OSK_4   = BUTTON_Create(310,  50, 60, 60, ID_OSK_4, WM_CF_SHOW);	
	hBUTTON_OSK_5   = BUTTON_Create(100, 120, 60, 60, ID_OSK_5, WM_CF_SHOW);	
	hBUTTON_OSK_6   = BUTTON_Create(170, 120, 60, 60, ID_OSK_6, WM_CF_SHOW);	
	hBUTTON_OSK_7   = BUTTON_Create(240, 120, 60, 60, ID_OSK_7, WM_CF_SHOW);	
	hBUTTON_OSK_8   = BUTTON_Create(310, 120, 60, 60, ID_OSK_8, WM_CF_SHOW);
	hBUTTON_OSK_9   = BUTTON_Create(170, 190, 60, 60, ID_OSK_9, WM_CF_SHOW);
    hBUTTON_OSK_0   = BUTTON_Create(240, 190, 60, 60, ID_OSK_0, WM_CF_SHOW);
    hBUTTON_OSK_BK  = BUTTON_Create(100, 190, 60, 60, ID_OSK_BK,WM_CF_SHOW);
	hBUTTON_OSK_OK  = BUTTON_Create(310, 190, 60, 60, ID_OSK_OK,WM_CF_SHOW);    
    BUTTON_SetFont(hBUTTON_OSK_0, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_1, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_2, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_3, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_4, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_5, GUI_FONT_24B_1);    
    BUTTON_SetFont(hBUTTON_OSK_6, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_7, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_8, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_9, GUI_FONT_24B_1);
    BUTTON_SetFont(hBUTTON_OSK_OK,GUI_FONT_20B_1);
    BUTTON_SetFont(hBUTTON_OSK_BK,GUI_FONT_20B_1);    
    BUTTON_SetText(hBUTTON_OSK_0, "0");
    BUTTON_SetText(hBUTTON_OSK_1, "1");
    BUTTON_SetText(hBUTTON_OSK_2, "2");	
    BUTTON_SetText(hBUTTON_OSK_3, "3");
    BUTTON_SetText(hBUTTON_OSK_4, "4");
    BUTTON_SetText(hBUTTON_OSK_5, "5");
    BUTTON_SetText(hBUTTON_OSK_6, "6");
    BUTTON_SetText(hBUTTON_OSK_7, "7");
    BUTTON_SetText(hBUTTON_OSK_8, "8");
    BUTTON_SetText(hBUTTON_OSK_9, "9");
    BUTTON_SetText(hBUTTON_OSK_OK,"OK");
	BUTTON_SetText(hBUTTON_OSK_BK,"BCK");    
    GUI_GotoXY(235, 20);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32_1);
    GUI_SetTextMode(GUI_TEXTMODE_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    GUI_DispString("ENTER PASSWORD");
    GUI_MULTIBUF_EndEx(1);
    DISPKeypadSet();
}

static void DISPDeleteKeypad(void){
	WM_DeleteWindow(hBUTTON_OSK_0);
	WM_DeleteWindow(hBUTTON_OSK_1);
	WM_DeleteWindow(hBUTTON_OSK_2);
	WM_DeleteWindow(hBUTTON_OSK_3);
	WM_DeleteWindow(hBUTTON_OSK_4);
	WM_DeleteWindow(hBUTTON_OSK_5);
	WM_DeleteWindow(hBUTTON_OSK_6);
	WM_DeleteWindow(hBUTTON_OSK_7);
	WM_DeleteWindow(hBUTTON_OSK_8);
	WM_DeleteWindow(hBUTTON_OSK_9);
	WM_DeleteWindow(hBUTTON_OSK_OK);
	WM_DeleteWindow(hBUTTON_OSK_BK);
    DISPKeypadReset();
}

static uint8_t DISPKeypad(char key){
	uint8_t  ret = 0;
    uint32_t loop;
    int srvpwd = 9999, kpdpwd = 0;
    BuzzerOn();
    HAL_Delay(1);
    BuzzerOff();
    GUI_SetColor    (GUI_WHITE);
    GUI_SetFont     (&GUI_Font24B_1);
    GUI_SetTextMode (GUI_TEXTMODE_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    if((key == 'B') || (key == 'O')){
        GUI_ClearRect(100, 0, 370, 50);        
        if (key == 'B'){
            oskin[oskb] = NUL;
            if(oskb > 0){
                --oskb;
                loop = 0;
                while(loop < oskb){
                    GUI_GotoXY      (235, 20);
                    GUI_DispCharAt  ('*',loop*50+130,15);
                    ++loop;
                }
            }
            oskin[oskb] = NUL;
        } else if (key == 'O'){
            oskin[5] = 0;
            kpdpwd = Str2Int(oskin, oskb);
            GUI_GotoXY      (235, 20);
            if ((kpdpwd == srvpwd)  &&  (srvpwd > 0) && (srvpwd < 100000)){
                GUI_SetColor (GUI_GREEN);
                GUI_DispString("SERVICE PASSWORD OK");
                DISPSettingsInitSet();
                BuzzerOn();
                HAL_Delay(500);
                BuzzerOff();
                HAL_Delay(1000);
                ret = 2;
            }
            
            if (ret == 0){
                GUI_GotoXY      (235, 20);
                GUI_SetColor    (GUI_RED);
                GUI_DispString  ("WRONG PASSWORD");
                BuzzerOn();
                HAL_Delay(50);
                BuzzerOff();
                HAL_Delay(100);
                BuzzerOn();
                HAL_Delay(50);
                BuzzerOff();
                HAL_Delay(100);
                BuzzerOn();
                HAL_Delay(50);
                BuzzerOff();
                HAL_Delay(1000);
            }
        }
    }else if (ISVALIDDEC(key)){
        if(oskb == 0U){
            GUI_ClearRect(100, 0, 370, 50);
        }
        GUI_SetTextAlign(GUI_TA_VCENTER);
        if(oskb < 5) GUI_DispCharAt('*', ((oskb*50U)+ 130U), 15U);
        oskin[oskb++] = key;
        if(oskb > 5) --oskb;
    }
    GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
    keypadtmr = HAL_GetTick();
    keypadtout = KEYPAD_TOUT;
    return (ret);
}

static void SaveController(CtrlTypeDef* lc, uint16_t addr){
    uint8_t buf[4];
    buf[0] = lc->Main.index;
    buf[1] = lc->Alarm1.index;
    buf[2] = lc->Alarm2.index;
    buf[3] = lc->Alarm3.index;
    EE_WriteBuffer(buf, addr, 4);
}

static void ReadController(CtrlTypeDef* lc, uint16_t addr){
    uint8_t buf[4];
    EE_ReadBuffer(buf, addr, 4);
    lc->Main.index      = buf[0];
    lc->Alarm1.index    = buf[1];
    lc->Alarm2.index    = buf[2];
    lc->Alarm3.index    = buf[3];
}
void SaveThermostatController(THERMOSTAT_TypeDef* tc, uint16_t addr){
    uint8_t buf[21];
    buf[0] = tc->th_ctrl;
    buf[1] = tc->th_state;
    buf[2] = tc->mv_temp>>8;
    buf[3] = tc->mv_temp&0xFF;
    buf[4] = tc->mv_offset;
    buf[5] = tc->mv_ntcref>>8;
    buf[6] = tc->mv_ntcref&0xFF;
    buf[7] = tc->mv_nctbeta>>8;
    buf[8] = tc->mv_nctbeta&0xFF;
    buf[9] = tc->sp_temp;
    buf[10] = tc->sp_diff;
    buf[11] = tc->sp_max;
    buf[12] = tc->sp_min;
    buf[13] = tc->fan_ctrl;
    buf[14] = tc->fan_speed;
    buf[15] = tc->fan_diff;
    buf[16] = tc->fan_loband;
    buf[17] = tc->fan_hiband;
    buf[18] = tc->fan_quiet_start;
    buf[19] = tc->fan_quiet_end;
    buf[20] = tc->fan_quiet_speed;
    EE_WriteBuffer(buf, addr, 21);
}

void ReadThermostatController(THERMOSTAT_TypeDef* tc, uint16_t addr){
    uint8_t buf[21];
    EE_ReadBuffer(buf, addr, 21);
    tc->th_ctrl         = buf[0];
    tc->th_state        = buf[1];
    tc->mv_temp         =(buf[2]<<8)|buf[3];
    tc->mv_offset       = buf[4];
    tc->mv_ntcref       =(buf[5]<<8)|buf[6];
    tc->mv_nctbeta      =(buf[7]<<8)|buf[8];
    tc->sp_temp         = buf[9];
    tc->sp_diff         = buf[10];
    tc->sp_max          = buf[11];
    tc->sp_min          = buf[12];
    tc->fan_ctrl        = buf[13];
    tc->fan_speed       = buf[14];
    tc->fan_diff        = buf[15];
    tc->fan_loband      = buf[16];
    tc->fan_hiband      = buf[17];
    tc->fan_quiet_start = buf[18];
    tc->fan_quiet_end   = buf[19];
    tc->fan_quiet_speed = buf[20];
}

/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
