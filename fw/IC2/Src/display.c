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
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"
/* Private Define ------------------------------------------------------------*/
#define KEY_PAUSE                       20      //
#define GUI_REFRESH_TIME                100     // refresh gui 10 time in second
#define DATE_TIME_REFRESH_TIME          1000    // refresh date & time info every 1 sec. 
#define SETTINGS_MENU_ENABLE_TIME       3456    // press and holde upper left corrner for period to enter setup menu
#define WFC_TOUT                        8765    // 9 sec. weather display timeout   
#define BUTTON_RESPONSE_TIME            1234    // button response delay till all onewire device update state
#define DISPIMG_TIME_MULTI              30000   // 30 sec. min time increment for image display time * 255 max
#define SETTINGS_MENU_TIMEOUT           59000   // 1 min. settings menu timeout
#define KEYPAD_TOUT                     30000   //
#define DISPMSG_TIME                    45U     // time to display message on event * GUI_REFRESH_TIME
#define EVENT_ONOFF_TOUT                500     // on/off event touch max. time 1s  
#define VALUE_STEP_TOUT                 15      // light value chage time
#define WFC_CHECK_TOUT                  (SECONDS_PER_HOUR * 1000U) // check weather forecast data validity every hour
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
#define BTN_DEC_X1                      (BTN_DEC_X0 + 120)
#define BTN_DEC_Y1                      (BTN_DEC_Y0 + 179)
#define BTN_INC_X0                      200
#define BTN_INC_Y0                      90
#define BTN_INC_X1                      (BTN_INC_X0 + 120)
#define BTN_INC_Y1                      (BTN_INC_Y0 + 179)
#define BTN_OK_X0                       269
#define BTN_OK_Y0                       135
#define BTN_OK_X1                       (BTN_OK_X0  + 200)
#define BTN_OK_Y1                       (BTN_OK_Y0  + 134)
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
#define ID_BIN_MAIN1                    0x821
#define ID_BIN_ALARM1                   0x822
#define ID_BIN_ALARM2                   0x823
#define ID_BIN_ALARM3                   0x824
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
SPINBOX_Handle  hBIN_MAIN1;
SPINBOX_Handle  hBIN_ALARM1;
SPINBOX_Handle  hBIN_ALARM2;
SPINBOX_Handle  hBIN_ALARM3;
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
int i, r, x;
uint32_t dispfl, lightauto, onofftmr, keypadtmr, keypadtout;
uint32_t menu_tmr, clean_tmr, clrtmr, guitmr, scrnsvr_tmr, rtctmr;
uint8_t btn_ok_state, btnset, low_bcklght, high_bcklght, light_ldr;
uint8_t screen, ctrl1, ctrl2, ctrl3, disp_rot, menu_screen, last_state;
uint8_t menu_alarm, menu_light, menu_clean, fwmsg=2, kyret, old_min=60;
uint8_t scrnsvr_ena_hour, scrnsvr_dis_hour, scrnsvr_clk_clr, scrnsvr_tout, oskb;
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
static void DISPDateTime(void);
static void DISPKillSet1Scrn(void);
static void DISPKillSet2Scrn(void);
static void DISPInitSet1Scrn(void);
static void DISPInitSet2Scrn(void);
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
    
    if (HAL_GetTick() - guitmr >= 89){
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
        fwmsg = 0;
        GUI_MULTIBUF_BeginEx(1);
        GUI_Clear();
        GUI_SetPenSize(9);
        GUI_SetColor(GUI_RED);
        GUI_DrawEllipse(240, 136, 50, 50);
        GUI_DrawLine(400,20,450,20);
        GUI_DrawLine(400,40,450,40);
        GUI_DrawLine(400,60,450,60);
        GUI_MULTIBUF_EndEx(1);
    }
    
    if (!x && HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_13) == GPIO_PIN_SET){
        ++x;
        DISPLightOn();
    }else if (x && HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_13) == GPIO_PIN_RESET){
        x = 0;
        HAL_Delay(100);
    }
    
    switch (screen){
        //
        //  MAIN LIGHT ON/OFF/DIMM
        //
        case 1:
        {
            if (onofftmr) break;
            GUI_MULTIBUF_BeginEx(1);
            GUI_Clear();
            GUI_SetPenSize(9);
            if (r){
                r = 0; // SKEEP FOR MENU TIMEOUT CALL
                if      (i == 1) i = 0;
                else if (i == 0) i = 1;
            } else {
                ScrnsvrReset();
                DISPSetBrightnes(high_bcklght);
            }
            //
            //  TOUCH EVENT TIME SHORT(ON/OFF)/LONG(DIMM)
            //
            switch(i){
                //
                //  MAIN LIGHT SWITCH ON POSSITION
                //
                case 0:
                {
                    i = 1;
                    LightOn();
                    Ctrl1.Main1.value = 1;                    
                    if (Ctrl1.AutoOff.value && !lightauto) lightauto = HAL_GetTick();
                    GUI_SetColor(GUI_GREEN);
                    break;
                }
                //
                //  MAIN LIGHT SWITCH OFF POSITION
                //
                case 1:
                default:
                {
                    i = 0;
                    LightOff();
                    lightauto = 0;
                    Ctrl1.Main1.value = 0;
                    GUI_SetColor(GUI_RED);
                    break;
                }
            }
            GUI_DrawLine(400,20,450,20);
            GUI_DrawLine(400,40,450,40);
            GUI_DrawLine(400,60,450,60);
            GUI_DrawEllipse(240,136,50,50);
            GUI_MULTIBUF_EndEx(1);
            onofftmr = HAL_GetTick();
            menu_screen = 0;
            menu_alarm = 0;
            menu_light = 0;            
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
                GUI_SetColor(GUI_GREEN);
                GUI_DrawLine(400,20,450,20);
                GUI_DrawLine(400,40,450,40);
                GUI_DrawLine(400,60,450,60);
                GUI_SetColor(GUI_RED);
                GUI_DrawLine(160,36,160,236);
                GUI_DrawLine(320,36,320,236);
                GUI_DrawBitmap(&bmSijalica,35,80);
                GUI_DrawBitmap(&bmKatanac,205,90);
                GUI_DrawBitmap(&bmClean,350,90);
                GUI_SetFont(GUI_FONT_24B_1);
                GUI_SetColor(GUI_ORANGE);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                GUI_GotoXY(45,220);
                GUI_DispString("LIGHT");
                GUI_GotoXY(200,220);
                GUI_DispString("ALARM");
                GUI_GotoXY(360,220);
                GUI_DispString("CLEAN");
                GUI_MULTIBUF_EndEx(1);
                menu_alarm=0;
                menu_light=0; 
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
                    oskb=0;
                    menu_alarm=1;
                    DISPCreateKeypad();
                    keypadtmr=HAL_GetTick();
                    ZEROFILL(oskin,sizeof(oskin));
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
                        screen=6;
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
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_1)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_1)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('1');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_2)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_2)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('2');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_3)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_3)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('3');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_4)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_4)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('4');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_5)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_5)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('5');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_6)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_6)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(50);
                            }
                            kyret = DISPKeypad('6');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_7)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_7)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('7');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_8)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_8)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('8');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_9)){
                            while(1){
                                if(!BUTTON_IsPressed(hBUTTON_OSK_9)){
                                    break;
                                }
                                TS_Service();
                                GUI_Delay(KEY_PAUSE);
                            }
                            kyret = DISPKeypad('9');
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_OK)){
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
                        }
                        else if (BUTTON_IsPressed(hBUTTON_OSK_BK)){
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
                
                case 2:
                {
                    menu_alarm=3;
                    GUI_MULTIBUF_BeginEx(1);
                    GUI_Clear();
                    GUI_SetPenSize(9);            
                    GUI_SetColor(GUI_GREEN);
                    GUI_DrawLine(400,20,450,20);
                    GUI_DrawLine(400,40,450,40);
                    GUI_DrawLine(400,60,450,60);
                    GUI_SetPenSize(5);
                    GUI_SetColor(GUI_RED);
                    GUI_DrawLine(160,36,160,216);
                    GUI_DrawLine(320,36,320,216);
                    if (Ctrl1.Alarm1.value) GUI_DrawBitmap(&bmArmed,40,80);
                    else GUI_DrawBitmap(&bmDisarmed,30,80);
                    if (Ctrl1.Alarm2.value) GUI_DrawBitmap(&bmArmed,210,80);
                    else GUI_DrawBitmap(&bmDisarmed,200,80);
                    if (Ctrl1.Alarm3.value) GUI_DrawBitmap(&bmArmed,370,80);
                    else GUI_DrawBitmap(&bmDisarmed,360,80);
                    GUI_SetColor(GUI_WHITE);
                    GUI_SetFont(GUI_FONT_24B_1);
                    GUI_SetTextMode(GUI_TM_TRANS);                    
                    GUI_GotoXY(80,200);
                    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                    GUI_DispString("GUEST HOUSE");                    
                    GUI_GotoXY(240,200);
                    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                    GUI_DispString("HOUSE");
                    GUI_GotoXY(400,200);
                    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                    GUI_DispString("FITNESS");
                    GUI_SetFont(GUI_FONT_24B_1);
                    GUI_SetColor(GUI_ORANGE);
                    GUI_SetTextMode(GUI_TM_TRANS);                    
                    GUI_GotoXY(5,250);
                    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                    GUI_DispString("ALARM CONTROL");
                    GUI_MULTIBUF_EndEx(1);
                    menu_light=0;
                    menu_screen=0;
                    break;
                }
                
                case 3:
                {
                    if      (ctrl1 ||  ctrl2 || ctrl3) menu_alarm = 4;
                    if      (ctrl1 &&  Ctrl1.Alarm1.value) Ctrl1.Alarm1.value = 0;
                    else if (ctrl1 && !Ctrl1.Alarm1.value) Ctrl1.Alarm1.value = 1;
                    if      (ctrl2 &&  Ctrl1.Alarm2.value) Ctrl1.Alarm2.value = 0;
                    else if (ctrl2 && !Ctrl1.Alarm2.value) Ctrl1.Alarm2.value = 1;
                    if      (ctrl3 &&  Ctrl1.Alarm3.value) Ctrl1.Alarm3.value = 0;
                    else if (ctrl3 && !Ctrl1.Alarm3.value) Ctrl1.Alarm3.value = 1;
                    break;
                }
                
                case 4:
                {
                    if (!ctrl1 && !ctrl2 && !ctrl3) menu_alarm = 2;
                    break;
                }
            }
            break;
        }
        //
        //  LIGHT CONTROL
        //
        case 4:
        {
            if (menu_light == 0){
                ++menu_light;
                GUI_MULTIBUF_BeginEx(1);
                GUI_Clear();
                GUI_SetPenSize(9);            
                GUI_SetColor(GUI_GREEN);
                GUI_DrawLine(400,20,450,20);
                GUI_DrawLine(400,40,450,40);
                GUI_DrawLine(400,60,450,60);
                GUI_SetPenSize(5);                
                if      ( Ctrl1.AutoOn.value)   GUI_SetColor(GUI_GREEN);
                else if (!Ctrl1.AutoOn.value)   GUI_SetColor(GUI_RED);
                GUI_DrawRect(140,20,340,100);
                GUI_SetFont(GUI_FONT_24B_1);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                GUI_GotoXY(240, 60);               
                GUI_DispString("AUTO ON");
                if      ( Ctrl1.AutoOff.value)  GUI_SetColor(GUI_GREEN);
                else if (!Ctrl1.AutoOff.value)  GUI_SetColor(GUI_RED);
                GUI_DrawRect(140,140,340,220);
                GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
                GUI_GotoXY(240, 180);                
                GUI_DispString("AUTO OFF 5 MIN.");
                GUI_SetFont(GUI_FONT_24B_1);
                GUI_SetColor(GUI_ORANGE);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                GUI_GotoXY(5, 250);
                GUI_DispString("LIGHT CONTROL");
                GUI_MULTIBUF_EndEx(1);
                menu_alarm =0;
                menu_screen = 0;
            } else if (menu_light == 1){
                if      (ctrl1 &&  Ctrl1.AutoOn.value){
                    Ctrl1.AutoOn.value = 0;
                    SaveController(&Ctrl1, EE_CTRL1);
                }else if (ctrl1 && !Ctrl1.AutoOn.value){
                    Ctrl1.AutoOn.value = 1;
                    SaveController(&Ctrl1, EE_CTRL1);
                }
                if      (ctrl2 &&  Ctrl1.AutoOff.value)  {
                    Ctrl1.AutoOff.value = 0;
                    lightauto = 0;
                    SaveController(&Ctrl1, EE_CTRL1);
                }else if(ctrl2 && !Ctrl1.AutoOff.value)  {
                    Ctrl1.AutoOff.value = 1;
                    lightauto = HAL_GetTick();
                    SaveController(&Ctrl1, EE_CTRL1);
                }
                if (ctrl1 || ctrl2) menu_light++;
            } else if (menu_light == 2){
                if (!ctrl1 && !ctrl2) menu_light = 0;
            }
            break;
        }
        //
        //  CLEAN DISPLAY
        //
        case 5:
        {
            if (menu_clean==0){                
                menu_screen=0;
                menu_alarm=0;
                menu_light=0;
                ++menu_clean;
                GUI_Clear();
                clrtmr=30;
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
            DISPSetBrightnes(low_bcklght);
            menu_screen=0;
            menu_alarm=0;
            menu_light=0;
            menu_clean=0;
            screen=1;
            r=1;
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
            if(CHECKBOX_GetState(hCHKBX_ScrnsvrClock) == 1) ScrnsvrClkSet();
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
            Ctrl1.Main1.index  = SPINBOX_GetValue(hBIN_MAIN1);
            Ctrl1.Alarm1.index  = SPINBOX_GetValue(hBIN_ALARM1);
            Ctrl1.Alarm2.index  = SPINBOX_GetValue(hBIN_ALARM2);
            Ctrl1.Alarm3.index  = SPINBOX_GetValue(hBIN_ALARM3);        
            if (BUTTON_IsPressed(hBUTTON_Ok)){
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
        if ((HAL_GetTick() - scrnsvr_tmr) >= (uint32_t)(scrnsvr_tout*1000)){
            if      (screen == 7) DISPKillSet1Scrn();
            else if (screen == 8) DISPKillSet2Scrn(); 
            DISPSetBrightnes(low_bcklght);
            ScrnsvrInitReset();
            ScrnsvrSet();
            screen = 6;
        }
    }
    
    if (lightauto){
        if(HAL_GetTick() - lightauto >= 300000){
            lightauto = 0;
            GUI_SelectLayer(0);
            GUI_SetColor(GUI_BLACK);
            GUI_Clear();
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT);
            GUI_Clear();
            DISPResetScrnsvr();
            BuzzerOn();
            HAL_Delay(1);
            BuzzerOff();
            screen = 1;
            r = 0;
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
    }
}

void DISPLightOn(void){
    if (Ctrl1.AutoOn.value){
        if (!Ctrl1.Main1.value){
            DISPResetScrnsvr();
            BuzzerOn();
            HAL_Delay(1);
            BuzzerOff();
            lightauto = 0;
            screen = 1;
            r = 0;
        }
    }
}

void DISPSetBrightnes(uint8_t val){
    if      (val < DISP_BRGHT_MIN) val = DISP_BRGHT_MIN;
    else if (val > DISP_BRGHT_MAX) val = DISP_BRGHT_MAX;
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, (uint16_t) (val * 10));
}

void DISPResetScrnsvr(void){
    if(IsScrnsvrActiv() && IsScrnsvrEnabled()) screen = 6;
    ScrnsvrReset();
    ScrnsvrInitReset();
    scrnsvr_tmr = HAL_GetTick();
    DISPSetBrightnes(high_bcklght);
}

void PID_Hook(GUI_PID_STATE * pTS){
    uint8_t click=0;
    if (pTS->Pressed == 1){
        pTS->Layer = 1;
        if ((pTS->y > 60) && (pTS->y < 200) && (screen == 3) && (menu_alarm > 1)){
            click = 1;
            ctrl1 = 0;
            ctrl2 = 0;
            ctrl3 = 0;
            if      ((pTS->x >   0) && (pTS->x < 160)) ctrl1 = 1;
            else if ((pTS->x > 160) && (pTS->x < 320)) ctrl2 = 1;
            else if ((pTS->x > 320) && (pTS->x < 480)) ctrl3 = 1;
        }else if((pTS->x > 420) && (pTS->y < 60) && (screen < 5)){
            click = 1;
            if  (screen == 0) screen = 2;
            else screen = 6;
        }else if((pTS->x < 160) && (screen == 2)){
            click = 1;
            screen = 4;
        }else if((pTS->x > 140) && (pTS->x < 340) && (pTS->y > 20) && (pTS->y < 100) && (screen == 4)){
            click = 1;
            ctrl1 = 1;
        }else if((pTS->x > 140) && (pTS->x < 340) && (pTS->y > 140) && (pTS->y < 220) && (screen == 4)){
            click = 1;
            ctrl2 = 1;
        }else if((pTS->x > 160) && (pTS->x < 320) && (screen == 2)){
            click = 1;
            screen = 3;
        }else if((pTS->x > 320) && (pTS->y > 100) && (screen == 2)){
            click = 1;
            screen = 5;
        }else if((pTS->x > 100) && (pTS->x < 400) && (pTS->y > 100) && (screen == 0)){
            if (!onofftmr){
                click = 1;
                screen = 1;                
            }
        }
        if ((pTS->x > 380) && (pTS->y < 100)){
            btnset = 1;
        }
        if (click){
            BuzzerOn();
            HAL_Delay(1);
            BuzzerOff();
        }
    }else{
        if(screen == 1) screen = 0;
        btnset = 0;
        ctrl1 = 0;
        ctrl2 = 0;
        ctrl3 = 0;
    }
    DISPResetScrnsvr(); 
}

uint8_t DISPMenuSettings(uint8_t btn){
    if ((btn == 1) && (last_state == 0)){
        last_state = 1;
        menu_tmr = HAL_GetTick(); 
    } else if ((btn == 1) && (last_state == 1)){
        if(HAL_GetTick() - menu_tmr >= SETTINGS_MENU_ENABLE_TIME){
            last_state = 0;
            return (1);
        }
    } else if ((btn == 0) && (last_state == 1)) {
        last_state = 0;
    }
    return (0);
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
    if(scrnsvr_ena_hour >= scrnsvr_dis_hour){
        if      (Bcd2Dec(rtctm.Hours)   >=  scrnsvr_ena_hour) ScrnsvrEnable();
        else if (Bcd2Dec(rtctm.Hours)   <   scrnsvr_dis_hour) ScrnsvrEnable();
        else if (IsScrnsvrEnabled())    ScrnsvrDisable(), screen = 6;
    }else if    (scrnsvr_ena_hour       <   scrnsvr_dis_hour){
        if      ((Bcd2Dec(rtctm.Hours   <   scrnsvr_dis_hour)) && (Bcd2Dec(rtctm.Hours) >= scrnsvr_ena_hour)) ScrnsvrEnable();
        else if (IsScrnsvrEnabled())    ScrnsvrDisable(), screen = 6;
    }
    /************************************/
    /*      DISPLAY  DATE  &  TIME      */
    /************************************/ 
    if (IsScrnsvrActiv() && IsScrnsvrEnabled() && IsScrnsvrClkActiv()){
        if(!IsScrnsvrInitActiv() || (old_day != rtcdt.WeekDay)){
            ScrnsvrInitSet();
            GUI_MULTIBUF_BeginEx(0);
            GUI_SelectLayer(0);
            GUI_Clear();
            GUI_MULTIBUF_EndEx(0);
            GUI_MULTIBUF_BeginEx(1);
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT); 
            GUI_Clear();
            old_min = 60;
            old_day = rtcdt.WeekDay;
            GUI_SetPenSize(9);
            GUI_SetColor(GUI_GREEN);
            GUI_DrawLine(400, 20, 450, 20);
            GUI_DrawLine(400, 40, 450, 40);
            GUI_DrawLine(400, 60, 450, 60);
            GUI_MULTIBUF_EndEx(1);
        }        
        HEX2STR(dbuf,&rtctm.Hours);
        if(rtctm.Seconds&1) dbuf[2]= ':';
        else dbuf[2] = ' ';
        HEX2STR(&dbuf[3],&rtctm.Minutes);
        GUI_GotoXY(CLOCK_H_POS, CLOCK_V_POS);
        GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
        GUI_SetFont(GUI_FONT_D80);
        GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
        GUI_MULTIBUF_BeginEx(1);
        GUI_ClearRect(0,  80, 480, 192);
        GUI_ClearRect(0, 220, 100, 270);
        GUI_DispString(dbuf);
        if      (rtcdt.WeekDay == 0)rtcdt.WeekDay = 7;
        if      (rtcdt.WeekDay == 1)memcpy(dbuf,     "  Monday  ",10);
        else if (rtcdt.WeekDay == 2)memcpy(dbuf,     " Tuestday ",10);
        else if (rtcdt.WeekDay == 3)memcpy(dbuf,     "Wednesday ",10);
        else if (rtcdt.WeekDay == 4)memcpy(dbuf,     "Thurstday ",10);
        else if (rtcdt.WeekDay == 5)memcpy(dbuf,     "  Friday  ",10);
        else if (rtcdt.WeekDay == 6)memcpy(dbuf,     " Saturday ",10);
        else if (rtcdt.WeekDay == 7)memcpy(dbuf,     "  Sunday  ",10);
        HEX2STR (&dbuf[10],&rtcdt.Date);
        if      (rtcdt.Month == 1)  memcpy(&dbuf[12],". January ",10);		
        else if (rtcdt.Month == 2)  memcpy(&dbuf[12],". February",10);
        else if (rtcdt.Month == 3)  memcpy(&dbuf[12],".  March  ",10);
        else if (rtcdt.Month == 4)  memcpy(&dbuf[12],".  April  ",10);
        else if (rtcdt.Month == 5)  memcpy(&dbuf[12],".   May   ",10);
        else if (rtcdt.Month == 6)  memcpy(&dbuf[12],".   June  ",10);
        else if (rtcdt.Month == 7)  memcpy(&dbuf[12],".   July  ",10);
        else if (rtcdt.Month == 8)  memcpy(&dbuf[12],".  August ",10);
        else if (rtcdt.Month == 9)  memcpy(&dbuf[12],".September",10);
        else if (rtcdt.Month ==0x10)memcpy(&dbuf[12],". October ",10);
        else if (rtcdt.Month ==0x11)memcpy(&dbuf[12],". November",10);
        else if (rtcdt.Month ==0x12)memcpy(&dbuf[12],". December",10);
        memcpy(&dbuf[22]," 20", 3);
        HEX2STR(&dbuf[25],&rtcdt.Year);
        dbuf[27] = '.';
        dbuf[28] = NUL;
        GUI_SetFont(GUI_FONT_24B_1);
        GUI_GotoXY(CLOCK_H_POS,CLOCK_V_POS + 70);
        GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
        GUI_DispString(dbuf);
        GUI_MULTIBUF_EndEx(1);
    }else if(old_min != rtctm.Minutes){
        old_min = rtctm.Minutes;
        HEX2STR(dbuf,&rtctm.Hours);
        dbuf[2] = ':';
        HEX2STR(&dbuf[3],&rtctm.Minutes);
        GUI_SetFont(GUI_FONT_32_1);
        GUI_SetColor(GUI_WHITE);
        GUI_SetTextMode(GUI_TM_TRANS);
        GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
        GUI_MULTIBUF_BeginEx(1);
        GUI_GotoXY(5, 245);
        GUI_ClearRect(0, 220, 100, 270);
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
    WM_DeleteWindow(hCHKBX_ScrnsvrClock);
    WM_DeleteWindow(hCHKBX_ScrnsvrLogoClock);
    WM_DeleteWindow(hSPNBX_ScrnsvrTimeout);
    WM_DeleteWindow(hSPNBX_ScrnsvrEnableHour);
    WM_DeleteWindow(hSPNBX_ScrnsvrDisableHour);
    WM_DeleteWindow(hDRPDN_WeekDay);
    WM_DeleteWindow(hSPNBX_Hour);
    WM_DeleteWindow(hSPNBX_Minute);
    WM_DeleteWindow(hSPNBX_Day);
    WM_DeleteWindow(hSPNBX_Month);
    WM_DeleteWindow(hSPNBX_Year);
    WM_DeleteWindow(hSPNBX_ScrnsvrClockColour);
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBUTTON_Next);
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
    hBIN_MAIN1 = SPINBOX_CreateEx(10, 50, 90, 30, 0, WM_CF_SHOW, ID_BIN_MAIN1, 0, 24);
    SPINBOX_SetEdge(hBIN_MAIN1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_MAIN1, Ctrl1.Main1.index);
    hBIN_ALARM1 = SPINBOX_CreateEx(10, 90, 90, 30, 0, WM_CF_SHOW, ID_BIN_ALARM1, 0, 24);
    SPINBOX_SetEdge(hBIN_ALARM1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_ALARM1, Ctrl1.Alarm1.index);
    hBIN_ALARM2 = SPINBOX_CreateEx(10, 130, 90, 30, 0, WM_CF_SHOW, ID_BIN_ALARM2, 0, 24);
    SPINBOX_SetEdge(hBIN_ALARM2, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_ALARM2, Ctrl1.Alarm2.index);
    hBIN_ALARM3 = SPINBOX_CreateEx(10, 170, 90, 30, 0, WM_CF_SHOW, ID_BIN_ALARM3, 0, 24);
    SPINBOX_SetEdge(hBIN_ALARM3, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_ALARM3, Ctrl1.Alarm3.index);
    hBUTTON_Next = BUTTON_Create(340, 160, 130, 40, ID_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");
    hBUTTON_Ok = BUTTON_Create(340, 220, 130, 40, ID_Ok, WM_CF_SHOW);
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
    WM_DeleteWindow(hDEV_ID);
    WM_DeleteWindow(hBIN_MAIN1);
    WM_DeleteWindow(hBIN_ALARM1);
    WM_DeleteWindow(hBIN_ALARM2);
    WM_DeleteWindow(hBIN_ALARM3);
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBUTTON_Next);
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
    int srvpwd=9999, kpdpwd=0, usrpsw=2345;
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
            if ((kpdpwd == srvpwd) && (srvpwd > 0) && (srvpwd < 100000)){
                GUI_SetColor (GUI_GREEN);
                GUI_DispString("SERVICE PASSWORD OK");
                DISPSettingsInitSet();
                BuzzerOn();
                HAL_Delay(500);
                BuzzerOff();
                HAL_Delay(1000);
                ret = 2;
            } else if ((kpdpwd == usrpsw) && (usrpsw > 0) && (usrpsw < 100000)){
                GUI_SetColor (GUI_GREEN);
                GUI_DispString("USER PASSWORD OK");
                BuzzerOn();
                HAL_Delay(500);
                BuzzerOff();
                HAL_Delay(1000);
                loop = 8;
                ret = 1;
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
    uint8_t buf[6];
    buf[0] = lc->Main1.index;
    buf[1] = lc->AutoOn.value;
    buf[2] = lc->AutoOff.value;
    buf[3] = lc->Alarm1.index;
    buf[4] = lc->Alarm2.index;
    buf[5] = lc->Alarm3.index;
    EE_WriteBuffer(buf, addr, 6);
}

static void ReadController(CtrlTypeDef* lc, uint16_t addr){
    uint8_t buf[6];
    EE_ReadBuffer(buf, addr, 6);
    lc->Main1.index     = buf[0];
    lc->AutoOn.value    = buf[1];
    lc->AutoOff.value   = buf[2];
    lc->Alarm1.index    = buf[3];
    lc->Alarm2.index    = buf[4];
    lc->Alarm3.index    = buf[5];
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
