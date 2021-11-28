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
#define GUI_REFRESH_TIME                100U    // refresh gui 10 time in second
#define DATE_TIME_REFRESH_TIME          1000U   // refresh date & time info every 1 sec. 
#define SETTINGS_MENU_ENABLE_TIME       3456U   // press and holde upper left corrner for period to enter setup menu
#define WFC_TOUT                        8765U   // 9 sec. weather display timeout   
#define BUTTON_RESPONSE_TIME            1234U   // button response delay till all onewire device update state
#define DISPIMG_TIME_MULTI              30000U  // 30 sec. min time increment for image display time * 255 max
#define SETTINGS_MENU_TIMEOUT           59000U  // 1 min. settings menu timeout
#define WFC_CHECK_TOUT                  (SECONDS_PER_HOUR * 1000U) // check weather forecast data validity every hour
#define KEYPAD_SIGNAL_TIME              3000
#define KEYPAD_UNLOCK_TIME              30000
#define DISPMSG_TIME                    45U     // time to display message on event * GUI_REFRESH_TIME
#define EVENT_ONOFF_TOUT                1000    // on/off event touch max. time 1s  
#define VALUE_STEP_TOUT                 15      // light value chage time

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

#define ID_Ok                           0x803
#define ID_Next                         0x805

#define ID_AmbientNtcOffset             0x830
#define ID_MaxSetpoint                  0x832
#define ID_MinSetpoint                  0x833
#define ID_DisplayHighBrightness        0x834
#define ID_DisplayLowBrightness         0x835
#define ID_ScrnsvrTimeout               0x836
#define ID_ScrnsvrEnableHour            0x837
#define ID_ScrnsvrDisableHour           0x838
#define ID_ScrnsvrClkColour             0x839
#define ID_ScrnsvrLogoClockColour       0x83A
#define ID_Hour                         0x83B
#define ID_Minute                       0x83C
#define ID_Day                          0x83D
#define ID_Month                        0x83E
#define ID_Year                         0x83F

#define ID_Scrnsvr                      0x850
#define ID_ScrnsvrClock                 0x851
#define ID_ScrnsvrLogoClock             0x852
#define ID_RTcfg                        0x853
#define ID_RCcfg                        0x854

#define ID_ThstControl                  0x860
#define ID_FanControl                   0x861
#define ID_ThstMaxSetPoint              0x862
#define ID_ThstMinSetPoint              0x863
#define ID_FanDiff                      0x864
#define ID_FanLowBand                   0x865
#define ID_FanHiBand                    0x866

#define ID_DEV_ID                       0x870
#define ID_DIM_LIGHT1                   0x871
#define ID_DIM_LIGHT2                   0x872
#define ID_DIM_LIGHT3                   0x873
#define ID_BIN_MAIN1                    0x874
#define ID_BIN_LED1                     0x875
#define ID_BIN_LED2                     0x876
#define ID_BIN_LED3                     0x877
#define ID_BIN_OUT1                     0x878
                       
#define ID2_DIM_LIGHT1                  0x880
#define ID2_DIM_LIGHT2                  0x881
#define ID2_DIM_LIGHT3                  0x882
#define ID2_BIN_MAIN1                   0x883
#define ID2_BIN_LED1                    0x884
#define ID2_BIN_LED2                    0x885
#define ID2_BIN_LED3                    0x886
#define ID2_BIN_OUT1                    0x887
/* Private Type --------------------------------------------------------------*/
BUTTON_Handle   hBUTTON_Increase;
BUTTON_Handle   hBUTTON_Decrease;
BUTTON_Handle   hBUTTON_Ok;
BUTTON_Handle   hBUTTON_Next;

SPINBOX_Handle  hSPNBX_MaxSetpoint;                         //  set thermostat user maximum setpoint value
SPINBOX_Handle  hSPNBX_MinSetpoint;                         //  set thermostat user minimum setpoint value

RADIO_Handle    hThstControl;
RADIO_Handle    hFanControl;
SPINBOX_Handle  hThstMaxSetPoint;
SPINBOX_Handle  hThstMinSetPoint;
SPINBOX_Handle  hFanDiff;
SPINBOX_Handle  hFanLowBand;
SPINBOX_Handle  hFanHiBand;

SPINBOX_Handle  hDEV_ID;
SPINBOX_Handle  hDIM_LIGHT1;
SPINBOX_Handle  hDIM_LIGHT2;
SPINBOX_Handle  hDIM_LIGHT3;
SPINBOX_Handle  hBIN_MAIN1;
SPINBOX_Handle  hBIN_LED1;
SPINBOX_Handle  hBIN_LED2;
SPINBOX_Handle  hBIN_LED3;
SPINBOX_Handle  hBIN_OUT1;

SPINBOX_Handle  hDIM2_LIGHT1;
SPINBOX_Handle  hDIM2_LIGHT2;
SPINBOX_Handle  hDIM2_LIGHT3;
SPINBOX_Handle  hBIN2_MAIN1;
SPINBOX_Handle  hBIN2_LED1;
SPINBOX_Handle  hBIN2_LED2;
SPINBOX_Handle  hBIN2_LED3;
SPINBOX_Handle  hBIN2_OUT1;


LIGHT_CtrlTypeDef LIGHT_Ctrl1;
LIGHT_CtrlTypeDef LIGHT_Ctrl2;
/* Private Macro   --------------------------------------------------------- */
/* Private Variable ----------------------------------------------------------*/
uint32_t dispfl;
uint8_t btn_ok_state, btnset;
uint8_t low_bcklght, high_bcklght, light_ldr;
uint8_t btn_increase_state, btn_increase_old_state;
uint8_t btn_decrease_state, btn_decrease_old_state;
uint8_t scrnsvr_ena_hour, scrnsvr_dis_hour, disp_rot;
uint8_t scrnsvr_clk_clr, scrnsvr_semiclk_clr, scrnsvr_tout;
char logbuf[128];
static uint32_t scrnsvr_tmr;
static uint8_t menu_dim = 0, menu_rel123 = 0, menu_out1 = 0;
static uint8_t menu_thst = 0, screen, ctrl1, ctrl2, ctrl3;
static uint8_t btninc, _btninc, btndec, _btndec, menu_lc = 0;
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
void DSP_KillSet1Scrn(void);
void DSP_KillSet2Scrn(void);
void DSP_InitSet1Scrn(void);
void DSP_InitSet2Scrn(void);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void DISP_Init(void){
    GUI_Init();
    GUI_PID_SetHook(PID_Hook);
    WM_MULTIBUF_Enable(1);
    GUI_UC_SetEncodeUTF8();
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    ReadLightController(&LIGHT_Ctrl1, EE_LIGHT_CTRL1);
    ReadLightController(&LIGHT_Ctrl2, EE_LIGHT_CTRL2);
    GUI_Exec();
}
/**
  * @brief
  * @param
  * @retval
  */
void DISP_Service(void){
    static int i = 0, s = 0, t = 0, r = 0;
    static uint32_t onoff_tmr = 0, dimm_tmr = 0, dir = 0;
    static uint32_t value_step_tmr = 0;
    static uint32_t rtc_tmr = 0, out1_tmr = 0;
    static uint32_t refresh_tmr = 0, guitmr = 0;
    static uint8_t c1= 0, c2= 0, c3= 0;
    static uint8_t thsta = 0, lcsta = 0, fwmsg = 2; 
    //
    //  GUI REDRAW AND TOUCH SCREEN REFRESH
    //
    if ((HAL_GetTick() - guitmr) >= 100){
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
    
    switch(screen){
        //
        //  MAIN LIGHT CONTROL ON/OFF/DIMM
        //
        case 1:
        {
            //
            //  TOUCH EVENT TIME SHORT(ON/OFF)/LONG(DIMM)
            //
            switch(i){
                //
                //  FIRST TOUCH EVENT IS ON/OFF
                //
                case 0:
                {
                    //
                    //  MAIN LIGHT CONTROL STATE ON/OFF
                    //
                    switch(t)
                    {
                        //
                        //  MAIN LIGHT SWITCH ON POSSITION
                        //
                        case 0:
                        {
                            i = 1; // START TOUCH PRESSED EVENT TIMER
                            t = 1; // CHECK TOUCH EVENT FIRST
                            LIGHT_Ctrl1.Main1.value = 1;
                            GUI_MULTIBUF_BeginEx(1);
                            GUI_Clear();
                            GUI_SetPenSize(9);
                            GUI_SetColor(GUI_GREEN);
                            GUI_DrawEllipse(240,136,50,50);
                            GUI_DrawLine(400,20,450,20);
                            GUI_DrawLine(400,40,450,40);
                            GUI_DrawLine(400,60,450,60);
                            GUI_MULTIBUF_EndEx(1);
                            onoff_tmr = HAL_GetTick();
                            break;
                        }
                        //
                        //  MAIN LIGHT SWITCH OFF POSITION
                        //
                        default:
                        case 1:
                        {
                            i = 4; // DO NOT ENTER HERE UNTIL TOUCH SCREEN UNPRESSED EVENT
                            t = 0; // CHECK TOUCH EVENT FIRST
                            LIGHT_Ctrl1.Main1.value = 0;
                            GUI_MULTIBUF_BeginEx(1);
                            GUI_Clear();
                            GUI_SetPenSize(9);
                            GUI_SetColor(GUI_RED);
                            GUI_DrawEllipse(240,136,50,50);
                            GUI_DrawLine(400,20,450,20);
                            GUI_DrawLine(400,40,450,40);
                            GUI_DrawLine(400,60,450,60);
                            GUI_MULTIBUF_EndEx(1);
                            onoff_tmr = HAL_GetTick();
                            break;
                        }
                    }

                    if (r){
                        screen = 0;
                        r = 0; // SKEEP FOR MENU TIMEOUT CALL
                    } else {
                        if      (t == 0){
                            LIGHT_Ctrl1.Led1.value = 0;
                            LIGHT_Ctrl1.Led2.value = 0;
                            LIGHT_Ctrl1.Led3.value = 0;
                            LIGHT_Ctrl1.Light1.value = 0;
                            LIGHT_Ctrl1.Light2.value = 0;
                            LIGHT_Ctrl1.Light3.value = 0;
                        } else if (t == 1){
                            LIGHT_Ctrl1.Light1.value = 0xFF;
                            LIGHT_Ctrl1.Light2.value = 0xFF;
                            LIGHT_Ctrl1.Light3.value = 0xFF/3;
                        }
                        ScrnsvrReset();
                        DISPSetBrightnes(DISP_BRGHT_MAX);
                    }
                    break;
                }
                //
                //  TOUCH EVENT TIME TRESHOLD
                //
                case 1:
                {
                    if ((HAL_GetTick() -  onoff_tmr) >= EVENT_ONOFF_TOUT){
                        ++i;    // touch event last over 1s, switch to light increase/decrease state
                        value_step_tmr = HAL_GetTick(); // load change step timer
                    }
                    break;
                }
                //
                //  LONG TOUCH IS MAIN LIGHT DIMMER CONTROL
                //
                case 2:
                {
                    if ((HAL_GetTick() -  value_step_tmr) >= VALUE_STEP_TOUT){
                        value_step_tmr = HAL_GetTick();
                        //
                        //  DIMMER INCREASE/DECREASE
                        //
                        switch(s)
                        {
                            //
                            //  DIMMER INCREASE
                            //
                            case 0:
                            {
                                if (LIGHT_Ctrl1.Light1.value < 0xFF) ++LIGHT_Ctrl1.Light1.value;
                                if (LIGHT_Ctrl1.Light2.value < 0xFF) ++LIGHT_Ctrl1.Light2.value;
                                if (LIGHT_Ctrl1.Light3.value < 0xFF) ++LIGHT_Ctrl1.Light3.value;
                                if ((LIGHT_Ctrl1.Light1.value == 0xFF) 
                                &&  (LIGHT_Ctrl1.Light2.value == 0xFF)
                                &&  (LIGHT_Ctrl1.Light3.value == 0xFF)){
                                    s = 1;
                                }
                                break;
                            }
                            //
                            //  DIMMER DECREASE
                            //
                            case 1:
                            default:
                            {
                                if (LIGHT_Ctrl1.Light1.value) --LIGHT_Ctrl1.Light1.value;
                                if (LIGHT_Ctrl1.Light2.value) --LIGHT_Ctrl1.Light2.value;
                                if (LIGHT_Ctrl1.Light3.value) --LIGHT_Ctrl1.Light3.value;
                                if (!LIGHT_Ctrl1.Light1.value 
                                &&  !LIGHT_Ctrl1.Light2.value 
                                &&  !LIGHT_Ctrl1.Light3.value){
                                    s = 0;
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            scrnsvr_tmr = HAL_GetTick();
            menu_thst = 0;
            menu_dim = 0;
            menu_rel123 = 0;
            menu_out1 = 0;
            menu_lc = 0;
            break;
        }
        //
        //  CONTROL SELECT  LIGHT/CLIMATE
        //
        case 2:
        {
            if (menu_lc == 0){
                ++menu_lc;
                GUI_MULTIBUF_BeginEx(1);
                GUI_Clear();
                GUI_SetPenSize(9);
                GUI_SetColor(GUI_RED);
                GUI_DrawLine(240,36,240,236);
                GUI_DrawBitmap(&bmSijalica, 120, 80);
                GUI_DrawBitmap(&bmTermometar, 300, 100);
                GUI_SetFont(GUI_FONT_24B_1);
                GUI_SetColor(GUI_ORANGE);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                GUI_GotoXY(120, 220);
                GUI_DispString("LIGHT");
                GUI_GotoXY(300, 220);
                GUI_DispString("CLIMATE");
                GUI_MULTIBUF_EndEx(1);
                menu_thst = 0;
                menu_dim = 0;
                menu_rel123 = 0;
                menu_out1 = 0;    
            }           
            break;
        }
        //
        //  LIGHT DIMMER 0-100% CONTROL
        //
        case 3:
        {
            switch (menu_dim){
                case 0:
                {
                    ++menu_dim;
                    GUI_MULTIBUF_BeginEx(1);
                    GUI_Clear();
                    GUI_SetPenSize(5);
                    GUI_SetColor(GUI_RED);
                    GUI_DrawLine(120,220,120,272);
                    GUI_DrawLine(240,220,240,272);
                    GUI_DrawLine(360,220,360,272);
                    GUI_DrawBitmap(&bmSijalicicaOn,   65, 215);
                    GUI_DrawBitmap(&bmSijalicicaOff, 185, 215);
                    GUI_DrawBitmap(&bmSijalicicaOff, 305, 215);
                    GUI_DrawBitmap(&bmHome,          400, 190);
                    if (LIGHT_Ctrl1.Light1.value)   GUI_DrawBitmap(&bmSijalicaOn,  60, 60);
                    else                            GUI_DrawBitmap(&bmSijalicaOff, 60, 60);
                    if (LIGHT_Ctrl1.Light2.value)   GUI_DrawBitmap(&bmSijalicaOn, 200, 60);
                    else                            GUI_DrawBitmap(&bmSijalicaOff,200, 60);
                    if (LIGHT_Ctrl1.Light3.value)   GUI_DrawBitmap(&bmSijalicaOn, 340, 60);
                    else                            GUI_DrawBitmap(&bmSijalicaOff,340, 60);                    
                    GUI_SetFont(GUI_FONT_24B_1);
                    GUI_SetColor(GUI_ORANGE);
                    GUI_SetTextMode(GUI_TM_TRANS);
                    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                    GUI_GotoXY(5, 250);
                    GUI_DispString("LIGHT");
                    GUI_GotoXY(135, 250);
                    GUI_DispString("LED");
                    GUI_GotoXY(255, 250);
                    GUI_DispString("OUT");
                    GUI_SetTextAlign(GUI_TA_HORIZONTAL|GUI_TA_VCENTER);
                    GUI_GotoXY(100,40);
                    if  (LIGHT_Ctrl1.Light1.value) GUI_DispDecMin((LIGHT_Ctrl1.Light1.value*100)/255);
                    GUI_GotoXY(240,40);
                    if  (LIGHT_Ctrl1.Light2.value) GUI_DispDecMin((LIGHT_Ctrl1.Light2.value*100)/255);
                    GUI_GotoXY(380,40);
                    if  (LIGHT_Ctrl1.Light3.value) GUI_DispDecMin((LIGHT_Ctrl1.Light3.value*100)/255);
                    GUI_MULTIBUF_EndEx(1);
                    menu_thst = 0;
                    menu_rel123 = 0;
                    menu_out1 = 0;
                    menu_lc = 0;
                    c1 = 0;
                    break;
                }
                //
                //  WAIT FOR DISPLAY TOUCH TO SELECT DIMMER
                //
                case 1:
                {
                    if (ctrl1 || ctrl2 || ctrl3){
                        c1 = ctrl1;
                        c2 = ctrl2;
                        c3 = ctrl3;
                        menu_dim = 2;
                        dimm_tmr = HAL_GetTick();
                    }
                    break;
                }
                //
                //  LONG TOUCH WILL START DIMMER CONTROL
                //  SHORT ONE WILL TRIGGER ON/OFF CONTROL
                //
                case 2:
                {
                    if ((HAL_GetTick() -  dimm_tmr) >= 1000){
                        value_step_tmr = HAL_GetTick();
                        ++menu_dim;
                        c1 = 0;
                        c2 = 0;
                        c3 = 0;
                    }
                    else if (!ctrl1 && !ctrl2 && !ctrl3){
                        if      (c1 &&  LIGHT_Ctrl1.Light1.value) LIGHT_Ctrl1.Light1.value = 0;
                        else if (c1 && !LIGHT_Ctrl1.Light1.value) LIGHT_Ctrl1.Light1.value = 0xFF;
                        if      (c2 &&  LIGHT_Ctrl1.Light2.value) LIGHT_Ctrl1.Light2.value = 0;
                        else if (c2 && !LIGHT_Ctrl1.Light2.value) LIGHT_Ctrl1.Light2.value = 0xFF;
                        if      (c3 &&  LIGHT_Ctrl1.Light3.value) LIGHT_Ctrl1.Light3.value = 0;
                        else if (c3 && !LIGHT_Ctrl1.Light3.value) LIGHT_Ctrl1.Light3.value = 0xFF;
                        menu_dim = 0;
                        c1 = 0;
                        c2 = 0;
                        c3 = 0;                        
                    }
                    break;
                }

                case 3:
                {
                    if      (!ctrl1 && !ctrl2 && !ctrl3) menu_dim = 0;
                    else if ((HAL_GetTick() -  value_step_tmr) >= VALUE_STEP_TOUT){                        
                        value_step_tmr = HAL_GetTick();
                        if      (ctrl1){                            
                            GUI_MULTIBUF_BeginEx(1);
                            switch (dir){
                                case 0:
                                {
                                    if  (!LIGHT_Ctrl1.Light1.value) GUI_DrawBitmap(&bmSijalicaOn, 60, 60);
                                    if  (LIGHT_Ctrl1.Light1.value < 0xFF) LIGHT_Ctrl1.Light1.value++;
                                    else dir = 1;
                                    break;
                                }

                                case 1:
                                default:
                                {
                                    if  (LIGHT_Ctrl1.Light1.value) LIGHT_Ctrl1.Light1.value--;
                                    else {
                                        dir = 0;
                                        GUI_ClearRect(60,60,160,160);
                                        GUI_DrawBitmap(&bmSijalicaOff, 60, 60);
                                    }
                                    break;
                                }
                            }
                            GUI_ClearRect(100,30,200,50);
                            GUI_GotoXY(100,40);
                            GUI_SetTextAlign(GUI_TA_HORIZONTAL|GUI_TA_VCENTER);
                            GUI_DispDecMin((LIGHT_Ctrl1.Light1.value*100)/255);
                            GUI_MULTIBUF_EndEx(1);
                        }
                        else if (ctrl2){
                            GUI_MULTIBUF_BeginEx(1);
                            switch (dir){
                                case 0:
                                {
                                    if  (!LIGHT_Ctrl1.Light2.value) GUI_DrawBitmap(&bmSijalicaOn, 200, 60);
                                    if   (LIGHT_Ctrl1.Light2.value < 0xFF) LIGHT_Ctrl1.Light2.value++;
                                    else dir = 1;
                                    break;
                                }

                                case 1:
                                default:
                                {
                                    if  (LIGHT_Ctrl1.Light2.value) LIGHT_Ctrl1.Light2.value--;
                                    else{
                                        dir = 0;
                                        GUI_ClearRect(200,60,300,160);
                                        GUI_DrawBitmap(&bmSijalicaOff, 200, 60);
                                    }
                                    break;
                                }
                            }
                            GUI_ClearRect(220,30,320,50);
                            GUI_GotoXY(240,40);
                            GUI_SetTextAlign(GUI_TA_HORIZONTAL|GUI_TA_VCENTER);
                            GUI_DispDecMin((LIGHT_Ctrl1.Light2.value*100)/255);
                            GUI_MULTIBUF_EndEx(1);
                        }
                        else if (ctrl3){
                            GUI_MULTIBUF_BeginEx(1);
                            switch (dir){
                                case 0:
                                {
                                    if  (!LIGHT_Ctrl1.Light3.value) GUI_DrawBitmap(&bmSijalicaOn, 340, 60);
                                    if   (LIGHT_Ctrl1.Light3.value < 0xFF) LIGHT_Ctrl1.Light3.value++;
                                    else dir = 1;
                                    break;
                                }

                                case 1:
                                default:
                                {
                                    if  (LIGHT_Ctrl1.Light3.value) LIGHT_Ctrl1.Light3.value--;
                                    else{
                                        dir = 0;
                                        GUI_ClearRect(340,60,440,160);
                                        GUI_DrawBitmap(&bmSijalicaOff, 340, 60);
                                    }
                                    break;
                                }
                            }
                            GUI_ClearRect(340,30,440,50);
                            GUI_GotoXY(380,40);
                            GUI_SetTextAlign(GUI_TA_HORIZONTAL|GUI_TA_VCENTER);
                            GUI_DispDecMin((LIGHT_Ctrl1.Light3.value*100)/255);
                            GUI_MULTIBUF_EndEx(1);
                        }
                    }
                    break;
                }
            }
            break;
        }
        //
        //  LED 1-3 RELAY ON/OFF CONTROL
        //
        case 4:
        {
            if      (menu_rel123 == 0){
                ++menu_rel123;
                GUI_MULTIBUF_BeginEx(1);
                GUI_Clear();
                GUI_SetPenSize(5);
                GUI_SetColor(GUI_RED);
                GUI_DrawLine(120,220,120,272);
                GUI_DrawLine(240,220,240,272);
                GUI_DrawLine(360,220,360,272);
                GUI_DrawBitmap(&bmSijalicicaOff, 65,215);
                GUI_DrawBitmap(&bmSijalicicaOn, 185,215);
                GUI_DrawBitmap(&bmSijalicicaOff,305,215);
                GUI_DrawBitmap(&bmHome,         400,190);
                GUI_SetFont(GUI_FONT_24B_1);
                GUI_SetColor(GUI_ORANGE);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                GUI_GotoXY(5, 250);
                GUI_DispString("LIGHT");
                GUI_GotoXY(135, 250);
                GUI_DispString("LED");
                GUI_GotoXY(255, 250);
                GUI_DispString("OUT");
                if (LIGHT_Ctrl1.Led1.value) GUI_DrawBitmap(&bmSijalicaOn,  60,60);
                else                        GUI_DrawBitmap(&bmSijalicaOff, 60,60);
                if (LIGHT_Ctrl1.Led2.value) GUI_DrawBitmap(&bmSijalicaOn, 200,60);
                else                        GUI_DrawBitmap(&bmSijalicaOff,200,60);
                if (LIGHT_Ctrl1.Led3.value) GUI_DrawBitmap(&bmSijalicaOn, 340,60);
                else                        GUI_DrawBitmap(&bmSijalicaOff,340,60);
                GUI_MULTIBUF_EndEx(1);
                menu_thst=0;
                menu_dim =0;
                menu_out1=0;
                menu_lc = 0;
            } else if (menu_rel123 == 1){
                if      (ctrl1 &&  LIGHT_Ctrl1.Led1.value)  LIGHT_Ctrl1.Led1.value = 0;
                else if (ctrl1 && !LIGHT_Ctrl1.Led1.value)  LIGHT_Ctrl1.Led1.value = 1;
                if      (ctrl2 &&  LIGHT_Ctrl1.Led2.value)  LIGHT_Ctrl1.Led2.value = 0;
                else if (ctrl2 && !LIGHT_Ctrl1.Led2.value)  LIGHT_Ctrl1.Led2.value = 1;
                if      (ctrl3 &&  LIGHT_Ctrl1.Led3.value)  LIGHT_Ctrl1.Led3.value = 0;
                else if (ctrl3 && !LIGHT_Ctrl1.Led3.value)  LIGHT_Ctrl1.Led3.value = 1;
                if (ctrl1 || ctrl2 || ctrl3) menu_rel123++;
            } else if (menu_rel123 == 2){
                if (!ctrl1 && !ctrl2 && !ctrl3) menu_rel123 = 0;
            }
            break;
        }
        //
        //  LED 4 RELAY ON/OFF CONTROL
        //
        case 5:
         {
            if      (menu_out1 == 0){
                ++menu_out1;
                GUI_MULTIBUF_BeginEx(1);
                GUI_Clear();
                GUI_SetPenSize(5);
                GUI_SetColor(GUI_RED);
                GUI_DrawLine(120,220,120,272);
                GUI_DrawLine(240,220,240,272);
                GUI_DrawLine(360,220,360,272);
                if (LIGHT_Ctrl1.Out1.value) GUI_DrawBitmap(&bmSijalicaOn, 200,60);
                else                        GUI_DrawBitmap(&bmSijalicaOff,200,60);
                GUI_DrawBitmap(&bmSijalicicaOff,  65, 215);
                GUI_DrawBitmap(&bmSijalicicaOff, 185, 215);
                GUI_DrawBitmap(&bmSijalicicaOn,  305, 215);
                GUI_DrawBitmap(&bmHome,          400, 190);
                GUI_SetFont(GUI_FONT_24B_1);
                GUI_SetColor(GUI_ORANGE);
                GUI_SetTextMode(GUI_TM_TRANS);
                GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                GUI_GotoXY(5, 250);
                GUI_DispString("LIGHT");
                GUI_GotoXY(135, 250);
                GUI_DispString("LED");
                GUI_GotoXY(255, 250);
                GUI_DispString("OUT");
                GUI_MULTIBUF_EndEx(1);
                menu_thst = 0;
                menu_dim = 0;
                menu_rel123 = 0;
                menu_lc = 0;
            }
            else if (menu_out1 == 1){
                if (ctrl2){
                    if (!LIGHT_Ctrl1.Out1.value){
                        LIGHT_Ctrl1.Out1.value = 1;
                        out1_tmr = HAL_GetTick(); // load to start relay 4 shutdown timer
                    }
                    else if (LIGHT_Ctrl1.Out1.value){
                        LIGHT_Ctrl1.Out1.value = 0;
                        out1_tmr = 0; // clear to disable relay 4 shutdown  timer
                    }
                    ++menu_out1;
                }
            }
            else if (menu_out1 == 2){
                if (!ctrl2) menu_out1 = 0;
            }
            break;
        }
        //
        //  THERMOSTAT
        //
        case 6:
        {
            if      (menu_thst == 0){
                ++menu_thst;
                GUI_SelectLayer(0);
                GUI_SetColor(GUI_BLACK);
                GUI_Clear();
                GUI_BMP_Draw(&thstat, 0, 0);
                GUI_SelectLayer(1);
                GUI_SetBkColor(GUI_TRANSPARENT);
                GUI_Clear();
                DISPSetPoint(); // show setpoint temperature
                DISPDateTime(); // show clock time
                MVUpdateSet(); // show room temperature
                menu_dim = 0;
                menu_rel123 = 0;
                menu_out1 = 0;
                menu_lc = 0;
            } else if (menu_thst == 1){
                /************************************/
                /*      SETPOINT  VALUE  INCREASED  */
                /************************************/
                if      ( btninc&& !_btninc){
                    _btninc = 1;
//                    if (thst.sp_temp < thst.sp_max) ++thst.sp_temp;
//                    SaveThermostatController(&thst, EE_THST_CTRL1);
                    DISPSetPoint();
                }
                else if (!btninc &&  _btninc) _btninc = 0;
                /************************************/
                /*      SETPOINT  VALUE  DECREASED  */
                /************************************/
                if      (btndec && !_btndec){
                    _btndec = 1;
//                    if (thst.sp_temp > thst.sp_min)  --thst.sp_temp;
//                    SaveThermostatController(&thst, EE_THST_CTRL1);
                    DISPSetPoint();
                }
                else if (!btndec &&  _btndec) _btndec = 0;
                /** ==========================================================================*/
                /**   R E W R I T E   A N D   S A V E   N E W   S E T P O I N T   V A L U E   */
                /** ==========================================================================*/
                if  (IsMVUpdateActiv()){
                    MVUpdateReset();
                    GUI_MULTIBUF_BeginEx(1);
                    GUI_ClearRect(410, 185, 480, 235);
                    GUI_ClearRect(310, 230, 480, 255);
                    GUI_GotoXY(310, 242);
                    GUI_SetFont(GUI_FONT_20_1);
                    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
//                    if (IsTempRegHeating()){
//                        GUI_SetColor(GUI_RED);
//                        GUI_DispString("HEATING");
//                        GUI_GotoXY(415, 220);
//                        GUI_SetColor(GUI_WHITE);
//                    } else if (IsTempRegCooling()){
//                        GUI_SetColor(GUI_BLUE);
//                        GUI_DispString("COOLING");
//                        GUI_GotoXY(415, 197);
//                        GUI_SetColor(GUI_ORANGE);
//                    }                    
                    GUI_SetFont(GUI_FONT_24_1);
                    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
//                    GUI_DispSDec(thst.mv_temp /10, 3);
                    GUI_DispString("°c");
                    GUI_MULTIBUF_EndEx(1);
                }
                /** ==========================================================================*/
                /**             W R I T E    D A T E    &    T I M E    S C R E E N           */
                /** ==========================================================================*/
                if ((HAL_GetTick() - rtc_tmr) >= DATE_TIME_REFRESH_TIME){ 
                    rtc_tmr = HAL_GetTick();// screansaver clock time update
                    if(++refresh_tmr > 10){// regular intervals
                        refresh_tmr = 0;// refresh room temperature
                        if (!IsScrnsvrActiv()) MVUpdateSet(); 
                    }
                    DISPDateTime();
                }
            }
            break;
        }
        //
        //  RETURN TO FIRST MENU
        //
        case 7:
        {
            GUI_SelectLayer(0);
            GUI_SetColor(GUI_BLACK);
            GUI_Clear();
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT);
            GUI_Clear();
            ScrnsvrSet();
            DISPSetBrightnes(DISP_BRGHT_MIN);
            screen = 1;
            i = 0;  // SET SWITCH TO ENTER ON/OFF TOUCH EVENT
            r = 1;  // SKEEP BACKLIGHT LED MAX BRIGHTNESS
            if (t == 0) t = 1; // TOGGLE NEXT TOUCH EVENT
            else t = 0;
            menu_thst = 0;
            menu_dim = 0;
            menu_rel123 = 0;
            menu_out1 = 0;
            menu_lc = 0;
            lcsta = 0;
            thsta = 0;
            break;
        }
        //
        //  SETTINGS MENU
        //
        case 8:
        {            
            /** ==========================================================================*/
            /**    S E T T I N G S     M E N U     U S E R     I N P U T   U P D A T E    */
            /** ==========================================================================*/
//            if (thst.th_ctrl != RADIO_GetValue(hThstControl)){
//                thst.th_ctrl  = RADIO_GetValue(hThstControl);
//                ++thsta;
//            }
//            if (thst.fan_ctrl != RADIO_GetValue(hFanControl)){
//                thst.fan_ctrl  = RADIO_GetValue(hFanControl);
//                ++thsta;
//            }
//            if (thst.sp_max != SPINBOX_GetValue(hThstMaxSetPoint)){
//                thst.sp_max  = SPINBOX_GetValue(hThstMaxSetPoint);
//                ++thsta;
//            }            
//            if (thst.sp_min != SPINBOX_GetValue(hThstMinSetPoint)){
//                thst.sp_min  = SPINBOX_GetValue(hThstMinSetPoint);
//                ++thsta;
//            }
//            if (thst.fan_diff != SPINBOX_GetValue(hFanDiff)){
//                thst.fan_diff  = SPINBOX_GetValue(hFanDiff);
//                ++thsta;
//            }
//            if (thst.fan_loband != SPINBOX_GetValue(hFanLowBand)){
//                thst.fan_loband  = SPINBOX_GetValue(hFanLowBand);
//                ++thsta;
//            }
//            if (thst.fan_hiband != SPINBOX_GetValue(hFanHiBand)){
//                thst.fan_hiband  = SPINBOX_GetValue(hFanHiBand);
//                ++thsta;
//            }           
            if (BUTTON_IsPressed(hBUTTON_Ok)){
                if (thsta){
                    thsta = 0;
//                    SaveThermostatController(&thst, EE_THST_CTRL1);
                }
                if (lcsta){
                    lcsta = 0;
                    SaveLightController(&LIGHT_Ctrl1, EE_LIGHT_CTRL1);
                    SaveLightController(&LIGHT_Ctrl2, EE_LIGHT_CTRL2);
                }
                DSP_KillSet1Scrn();
                screen = 7;
            } else if (BUTTON_IsPressed(hBUTTON_Next)){                
                DSP_KillSet1Scrn();
                DSP_InitSet2Scrn();
                screen = 9;
            }
            break;
        }
        //
        //  SETTINGS MENU
        //
        case 9:
        {
            if (tfifa != SPINBOX_GetValue(hDEV_ID)){
                tfifa = SPINBOX_GetValue(hDEV_ID);
                EE_WriteBuffer(&tfifa, EE_TFIFA, 1);
            }            
            if (LIGHT_Ctrl1.Main1.index != SPINBOX_GetValue(hBIN_MAIN1)){
                LIGHT_Ctrl1.Main1.index  = SPINBOX_GetValue(hBIN_MAIN1);
                if (LIGHT_Ctrl1.Main1.index == 0) SPINBOX_SetValue(hBIN2_MAIN1, 0);
                ++lcsta;
            }
            if (LIGHT_Ctrl1.Led1.index != SPINBOX_GetValue(hBIN_LED1)){
                LIGHT_Ctrl1.Led1.index  = SPINBOX_GetValue(hBIN_LED1);
                if (LIGHT_Ctrl1.Led1.index == 0) SPINBOX_SetValue(hBIN2_LED1, 0);
                ++lcsta;
            }
            if (LIGHT_Ctrl1.Led2.index != SPINBOX_GetValue(hBIN_LED2)){
                LIGHT_Ctrl1.Led2.index  = SPINBOX_GetValue(hBIN_LED2);
                if (LIGHT_Ctrl1.Led2.index == 0) SPINBOX_SetValue(hBIN2_LED2, 0);
                ++lcsta;
            }
            if (LIGHT_Ctrl1.Led3.index != SPINBOX_GetValue(hBIN_LED3)){
                LIGHT_Ctrl1.Led3.index  = SPINBOX_GetValue(hBIN_LED3);
                if (LIGHT_Ctrl1.Led3.index == 0) SPINBOX_SetValue(hBIN2_LED3, 0);
                ++lcsta;
            }
            if (LIGHT_Ctrl1.Out1.index != SPINBOX_GetValue(hBIN_OUT1)){
                LIGHT_Ctrl1.Out1.index  = SPINBOX_GetValue(hBIN_OUT1);
                if (LIGHT_Ctrl1.Out1.index == 0) SPINBOX_SetValue(hBIN2_OUT1,0);
                ++lcsta;
            }
            if (LIGHT_Ctrl1.Light1.index != SPINBOX_GetValue(hDIM_LIGHT1)){
                LIGHT_Ctrl1.Light1.index  = SPINBOX_GetValue(hDIM_LIGHT1);
                ++lcsta;
            }
            if (LIGHT_Ctrl1.Light2.index != SPINBOX_GetValue(hDIM_LIGHT2)){
                LIGHT_Ctrl1.Light2.index  = SPINBOX_GetValue(hDIM_LIGHT2);
                ++lcsta;
            }
            if (LIGHT_Ctrl1.Light3.index != SPINBOX_GetValue(hDIM_LIGHT3)){
                LIGHT_Ctrl1.Light3.index  = SPINBOX_GetValue(hDIM_LIGHT3);
                ++lcsta;
            }
            if (LIGHT_Ctrl2.Main1.index != SPINBOX_GetValue(hBIN2_MAIN1)){
                LIGHT_Ctrl2.Main1.index  = SPINBOX_GetValue(hBIN2_MAIN1);
                ++lcsta;
            }
            if (LIGHT_Ctrl2.Led1.index != SPINBOX_GetValue(hBIN2_LED1)){
                LIGHT_Ctrl2.Led1.index  = SPINBOX_GetValue(hBIN2_LED1);
                ++lcsta;
            }
            if (LIGHT_Ctrl2.Led2.index != SPINBOX_GetValue(hBIN2_LED2)){
                LIGHT_Ctrl2.Led2.index  = SPINBOX_GetValue(hBIN2_LED2);
                ++lcsta;
            }
            if (LIGHT_Ctrl2.Led3.index != SPINBOX_GetValue(hBIN2_LED3)){
                LIGHT_Ctrl2.Led3.index  = SPINBOX_GetValue(hBIN2_LED3);
                ++lcsta;
            }
            if (LIGHT_Ctrl2.Out1.index != SPINBOX_GetValue(hBIN2_OUT1)){
                LIGHT_Ctrl2.Out1.index  = SPINBOX_GetValue(hBIN2_OUT1);
                ++lcsta;
            }            
            if (BUTTON_IsPressed(hBUTTON_Ok)){
                if (thsta){
                    thsta = 0;
//                    SaveThermostatController(&thst,  EE_THST_CTRL1);
                }
                if (lcsta){
                    lcsta = 0;
                    SaveLightController(&LIGHT_Ctrl1, EE_LIGHT_CTRL1);
                    SaveLightController(&LIGHT_Ctrl2, EE_LIGHT_CTRL2);
                }
                DSP_KillSet2Scrn();
                screen = 7;
            } else if (BUTTON_IsPressed(hBUTTON_Next)){                
                DSP_KillSet2Scrn();
                DSP_InitSet1Scrn();
                screen = 8;
            }
            break;
        }
        //
        //  RESET MENU SWITCHES
        //        
        case 0:
        default:
        {
            i = 0;
            menu_lc = 0;
            menu_thst = 0;
            menu_dim = 0;
            menu_rel123 = 0;
            menu_out1 = 0;
            break;
        }
    }
    if (!IsScrnsvrActiv()){
        if ((HAL_GetTick() - scrnsvr_tmr) >= (uint32_t)(scrnsvr_tout*1000)){
            if      (screen == 8) DSP_KillSet1Scrn();
            else if (screen == 9) DSP_KillSet2Scrn();            
            screen = 7;
        }
    } else DISPSetBrightnes(light_ldr/3);
    //
    //  CHECK IF ENABLED RELAY 4 AUTO SHUTDOWN TIMER
    //
    if (out1_tmr){
        if ((HAL_GetTick() - out1_tmr) >= (uint32_t)(SECONDS_PER_HOUR*4000)){
            LIGHT_Ctrl1.Out1.value = 0;
            out1_tmr = 0;
        }
    }
    //
    //  DISPLAY SETTINGS MENU
    //
    if (DISPMenuSettings(btnset)&&(screen < 8)){
        DSP_InitSet1Scrn();
        screen = 8;
    }    
}
/**
  * @brief
  * @param
  * @retval
  */

void DSP_InitSet1Scrn(void){
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
//    RADIO_SetValue(hThstControl, thst.th_ctrl);

    hFanControl = RADIO_CreateEx(10, 150, 150, 80, 0,WM_CF_SHOW, 0, ID_FanControl, 2, 20);
    RADIO_SetTextColor(hFanControl, GUI_GREEN);
    RADIO_SetText(hFanControl, "ON / OFF", 0);
    RADIO_SetText(hFanControl, "3 SPEED", 1);
//    RADIO_SetValue(hFanControl, thst.fan_ctrl);

    hThstMaxSetPoint = SPINBOX_CreateEx(110, 20, 90, 30, 0, WM_CF_SHOW, ID_ThstMaxSetPoint, 15, 40);
    SPINBOX_SetEdge(hThstMaxSetPoint, SPINBOX_EDGE_CENTER);
//    SPINBOX_SetValue(hThstMaxSetPoint, thst.sp_max);

    hThstMinSetPoint = SPINBOX_CreateEx(110, 70, 90, 30, 0, WM_CF_SHOW, ID_ThstMinSetPoint, 15, 40);
    SPINBOX_SetEdge(hThstMinSetPoint, SPINBOX_EDGE_CENTER);
//    SPINBOX_SetValue(hThstMinSetPoint, thst.sp_min);

    hFanDiff = SPINBOX_CreateEx(110, 150, 90, 30, 0, WM_CF_SHOW, ID_FanDiff, 0, 10);
    SPINBOX_SetEdge(hFanDiff, SPINBOX_EDGE_CENTER);
//    SPINBOX_SetValue(hFanDiff, thst.fan_diff);

    hFanLowBand = SPINBOX_CreateEx(110, 190, 90, 30, 0, WM_CF_SHOW, ID_FanLowBand, 0, 50);
    SPINBOX_SetEdge(hFanLowBand, SPINBOX_EDGE_CENTER);
//    SPINBOX_SetValue(hFanLowBand, thst.fan_loband);

    hFanHiBand = SPINBOX_CreateEx(110, 230, 90, 30, 0, WM_CF_SHOW, ID_FanHiBand, 0, 100);
    SPINBOX_SetEdge(hFanHiBand, SPINBOX_EDGE_CENTER);
//    SPINBOX_SetValue(hFanHiBand, thst.fan_hiband);

    hBUTTON_Next = BUTTON_Create(340, 180, 130, 30, ID_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");

    hBUTTON_Ok = BUTTON_Create(340, 230, 130, 30, ID_Ok, WM_CF_SHOW);
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
/**
  * @brief
  * @param
  * @retval
  */
void DSP_KillSet1Scrn(void){
    WM_DeleteWindow(hThstControl);
    WM_DeleteWindow(hFanControl);
    WM_DeleteWindow(hThstMaxSetPoint);
    WM_DeleteWindow(hThstMinSetPoint);
    WM_DeleteWindow(hFanDiff);
    WM_DeleteWindow(hFanLowBand);
    WM_DeleteWindow(hFanHiBand);
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBUTTON_Next);
}
/**
  * @brief
  * @param
  * @retval
  */
void DSP_InitSet2Scrn(void){
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
    SPINBOX_SetValue(hBIN_MAIN1, LIGHT_Ctrl1.Main1.index);
    
    hBIN_LED1 = SPINBOX_CreateEx(10, 90, 90, 30, 0, WM_CF_SHOW, ID_BIN_LED1, 0, 24);
    SPINBOX_SetEdge(hBIN_LED1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_LED1, LIGHT_Ctrl1.Led1.index);
    
    hBIN_LED2 = SPINBOX_CreateEx(10, 130, 90, 30, 0, WM_CF_SHOW, ID_BIN_LED2, 0, 24);
    SPINBOX_SetEdge(hBIN_LED2, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_LED2, LIGHT_Ctrl1.Led2.index);
    
    hBIN_LED3 = SPINBOX_CreateEx(10, 170, 90, 30, 0, WM_CF_SHOW, ID_BIN_LED3, 0, 24);
    SPINBOX_SetEdge(hBIN_LED3, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_LED3, LIGHT_Ctrl1.Led3.index);
    
    hBIN_OUT1 = SPINBOX_CreateEx(10, 210, 90, 30, 0, WM_CF_SHOW, ID_BIN_OUT1, 0, 24);
    SPINBOX_SetEdge(hBIN_OUT1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN_OUT1, LIGHT_Ctrl1.Out1.index);
    
    hDIM_LIGHT1 = SPINBOX_CreateEx(340, 50, 90, 30, 0, WM_CF_SHOW, ID_DIM_LIGHT1, 0, 32);
    SPINBOX_SetEdge(hDIM_LIGHT1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hDIM_LIGHT1, LIGHT_Ctrl1.Light1.index);
    
    hDIM_LIGHT2 = SPINBOX_CreateEx(340, 90, 90, 30, 0, WM_CF_SHOW, ID_DIM_LIGHT2, 0, 32);
    SPINBOX_SetEdge(hDIM_LIGHT2, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hDIM_LIGHT2, LIGHT_Ctrl1.Light2.index);
    
    hDIM_LIGHT3 = SPINBOX_CreateEx(340, 130, 90, 30, 0, WM_CF_SHOW, ID_DIM_LIGHT3, 0, 32);
    SPINBOX_SetEdge(hDIM_LIGHT3, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hDIM_LIGHT3, LIGHT_Ctrl1.Light3.index);


    hBIN2_MAIN1 = SPINBOX_CreateEx(170, 50, 90, 30, 0, WM_CF_SHOW, ID2_BIN_MAIN1, 0, 24);
    SPINBOX_SetEdge(hBIN2_MAIN1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN2_MAIN1, LIGHT_Ctrl2.Main1.index);
    
    hBIN2_LED1 = SPINBOX_CreateEx(170, 90, 90, 30, 0, WM_CF_SHOW, ID2_BIN_LED1, 0, 24);
    SPINBOX_SetEdge(hBIN2_LED1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN2_LED1, LIGHT_Ctrl2.Led1.index);
    
    hBIN2_LED2 = SPINBOX_CreateEx(170, 130, 90, 30, 0, WM_CF_SHOW, ID2_BIN_LED2, 0, 24);
    SPINBOX_SetEdge(hBIN2_LED2, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN2_LED2, LIGHT_Ctrl2.Led2.index);
    
    hBIN2_LED3 = SPINBOX_CreateEx(170, 170, 90, 30, 0, WM_CF_SHOW, ID2_BIN_LED3, 0, 24);
    SPINBOX_SetEdge(hBIN2_LED3, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN2_LED3, LIGHT_Ctrl2.Led3.index);
    
    hBIN2_OUT1 = SPINBOX_CreateEx(170, 210, 90, 30, 0, WM_CF_SHOW, ID2_BIN_OUT1, 0, 24);
    SPINBOX_SetEdge(hBIN2_OUT1, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hBIN2_OUT1, LIGHT_Ctrl2.Out1.index);

    hBUTTON_Next = BUTTON_Create(340, 180, 130, 30, ID_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");

    hBUTTON_Ok = BUTTON_Create(340, 230, 130, 30, ID_Ok, WM_CF_SHOW);
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
    GUI_DispString("LED1 SW.");
    GUI_GotoXY(110, 106);
    GUI_DispString("RELAY1");
    
    GUI_GotoXY(110, 134);
    GUI_DispString("LED2 SW.");
    GUI_GotoXY(110, 146);
    GUI_DispString("RELAY1");
    
    GUI_GotoXY(110, 174);
    GUI_DispString("LED3 SW.");
    GUI_GotoXY(110, 186);
    GUI_DispString("RELAY1");
    
    GUI_GotoXY(110, 214);
    GUI_DispString("OUT1 SW.");
    GUI_GotoXY(110, 226);
    GUI_DispString("RELAY1");
    
    GUI_GotoXY(270, 54);
    GUI_DispString("MAIN SW.");
    GUI_GotoXY(270, 66);
    GUI_DispString("RELAY2");
    
    GUI_GotoXY(270, 94);
    GUI_DispString("LED1 SW.");
    GUI_GotoXY(270, 106);
    GUI_DispString("RELAY2");
    
    GUI_GotoXY(270, 134);
    GUI_DispString("LED2 SW.");
    GUI_GotoXY(270, 146);
    GUI_DispString("RELAY2");
    
    GUI_GotoXY(270, 174);
    GUI_DispString("LED3 SW.");
    GUI_GotoXY(270, 186);
    GUI_DispString("RELAY2");
    
    GUI_GotoXY(270, 214);
    GUI_DispString("OUT1 SW.");
    GUI_GotoXY(270, 226);
    GUI_DispString("RELAY2");
    
    GUI_GotoXY(440, 54);
    GUI_DispString("LIGHT1");
    GUI_GotoXY(440, 66);
    GUI_DispString("DIMMER");
    
    GUI_GotoXY(440, 94);
    GUI_DispString("LIGHT2");
    GUI_GotoXY(440, 106);
    GUI_DispString("DIMMER");
    
    GUI_GotoXY(440, 134);
    GUI_DispString("LIGHT3");
    GUI_GotoXY(440, 146);
    GUI_DispString("DIMMER");
    
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief
  * @param
  * @retval
  */
void DSP_KillSet2Scrn(void){
    WM_DeleteWindow(hDEV_ID);
    WM_DeleteWindow(hBIN_MAIN1);
    WM_DeleteWindow(hBIN_LED1);
    WM_DeleteWindow(hBIN_LED2);
    WM_DeleteWindow(hBIN_LED3);
    WM_DeleteWindow(hBIN_OUT1);
    WM_DeleteWindow(hDIM_LIGHT1);
    WM_DeleteWindow(hDIM_LIGHT2);
    WM_DeleteWindow(hDIM_LIGHT3);
    WM_DeleteWindow(hBIN2_MAIN1);
    WM_DeleteWindow(hBIN2_LED1);
    WM_DeleteWindow(hBIN2_LED2);
    WM_DeleteWindow(hBIN2_LED3);
    WM_DeleteWindow(hBIN2_OUT1);
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBUTTON_Next);
}
/**
  * @brief  Display Backlight LED brightnes control
  * @param  brightnes_high_level
  * @retval DISP sreensvr_tmr loaded with system_tmr value
  */
void DISPSetBrightnes(uint8_t val){
    if      (val < DISP_BRGHT_MIN) val = DISP_BRGHT_MIN;
    else if (val > DISP_BRGHT_MAX) val = DISP_BRGHT_MAX;
    
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, (uint16_t) (val * 10U));
}
/**
  * @brief  Display Date and Time in deifferent font size colour and position
  * @param  Flags: IsRtcTimeValid, IsScrnsvrActiv, BUTTON_Dnd, BUTTON_BTNMaid,
            BTNSosReset, IsScrnsvrSemiClkActiv, IsScrnsvrClkActiv
  * @retval None
  */
void DISPDateTime(void){
    char dbuf[8];
    if(!IsRtcTimeValid()) return; // nothing to display untill system rtc validated
    HAL_RTC_GetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
    HEX2STR(dbuf, &rtctm.Hours);
    dbuf[2] = ':';
    HEX2STR(&dbuf[3], &rtctm.Minutes);
    dbuf[5]  = NUL;
    GUI_SetFont(GUI_FONT_32_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_GotoXY(5, 245);
    GUI_MULTIBUF_BeginEx(1);
    GUI_ClearRect(0, 220, 100, 270);
    GUI_DispString(dbuf);
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief  Display Backlight LED brightnes control
  * @param  brightnes_high_level
  * @retval DISP sreensvr_tmr loaded with system_tmr value
  */
void DISPResetScrnsvr(void){
    if(IsScrnsvrActiv() && IsScrnsvrEnabled()){
        DISPUpdateSet();
    }    
    ScrnsvrReset();
    ScrnsvrInitReset();
    scrnsvr_tmr = HAL_GetTick();
    scrnsvr_tout = SCRNSVR_TOUT;
    DISPSetBrightnes(DISP_BRGHT_MAX);
}
/**
  * @brief
  * @param
  * @retval
  */
void DISPSetPoint(void){
    GUI_MULTIBUF_BeginEx(1);
    GUI_ClearRect(SP_H_POS - 5, SP_V_POS - 5, SP_H_POS + 120, SP_V_POS + 85);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_D48);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextAlign(GUI_TA_RIGHT);
    GUI_GotoXY(SP_H_POS, SP_V_POS);
//    GUI_DispDec(thst.sp_temp , 2);
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief
  * @param
  * @retval
  */
void PID_Hook(GUI_PID_STATE * pTS){
   uint8_t click = 0;

    if (pTS->Pressed  == 1U){
        pTS->Layer = 1U;

        if ((pTS->y > 60)
        &&  (pTS->y < 200)
        && ((screen == 3)
        ||  (screen == 4)
        ||  (screen == 5)))
        {
            click = 1;
            ctrl1 = 0;
            ctrl2 = 0;
            ctrl3 = 0;
            if      ((pTS->x >   0) && (pTS->x < 160)) ctrl1 = 1;
            else if ((pTS->x > 160) && (pTS->x < 320)) ctrl2 = 1;
            else if ((pTS->x > 320) && (pTS->x < 480)) ctrl3 = 1;
        }


        if ((pTS->x > 400)
        &&  (pTS->x < 480)
        &&  (pTS->y > 0)
        &&  (pTS->y < 60)
        &&  ((screen < 2)||(screen == 6)))
        {
            click = 1;
            if      (screen  < 2) screen = 2;
            else if (screen == 6) screen = 7;
        }
        else if((pTS->x > 0)
        &&      (pTS->x < 240)
        &&      (pTS->y > 0)
        &&      (pTS->y < 272)
        &&      (screen == 2))
        {
            click = 1;
            screen = 3;
        }
        else if((pTS->x > 0)
        &&      (pTS->x < 120)
        &&      (pTS->y > 200)
        &&      (pTS->y < 272)
        &&      ((screen == 4)||(screen == 5)))
        {
            click = 1;
            screen = 3;
        }
        else if((pTS->x > 120)
        &&      (pTS->x < 240)
        &&      (pTS->y > 200)

        &&      (pTS->y < 272)
        &&      ((screen == 3)||(screen == 5)))
        {
            click = 1;
            screen = 4;
        }
        else if((pTS->x > 240)
        &&      (pTS->y > 200)
        &&      (pTS->x < 360)
        &&      (pTS->y < 272)
        &&      ((screen == 3)||(screen == 4)))
        {
            click = 1;
            screen = 5;
        }
        else if((pTS->x > 360)
        &&      (pTS->y > 200)
        &&      (pTS->x < 480)
        &&      (pTS->y < 272)
        &&      ((screen == 3)||(screen == 4)||(screen == 5)))
        {
            click = 1;
            screen = 7; // RETURN TO FIRST MENU
        }
        else if((pTS->x > 240)
        &&      (pTS->y > 100)
        &&      (pTS->x < 480)
        &&      (pTS->y < 272)
        &&      (screen == 2))
        {
            click = 1;
            screen = 6;
        }
        else if((pTS->x > BTN_INC_X0)
        &&      (pTS->y > BTN_INC_Y0)
        &&      (pTS->x < BTN_INC_X1)
        &&      (pTS->y < BTN_INC_Y1)
        &&      (screen == 6))
        {
            click = 1;
            btninc = 1;
        }
        else if((pTS->x > BTN_DEC_X0)
        &&      (pTS->y > BTN_DEC_Y0)
        &&      (pTS->x < BTN_DEC_X1)
        &&      (pTS->y < BTN_DEC_Y1)
        &&      (screen == 6))
        {
            click = 1;
            btndec = 1;
        }
        else if((pTS->x > 100)
        &&      (pTS->y > 100)
        &&      (pTS->x < 400)
        &&      (pTS->y < 272)
        &&      (screen == 0))
        {
            click = 1;
            screen = 1;
        }
        
        if ((pTS->x > 380)
        &&  (pTS->y > 5)
        &&  (pTS->x < 475)
        &&  (pTS->y < 100))
        {
            btnset = 1;
        }
        
        if (click){
            BuzzerOn();
            HAL_Delay(1);
            BuzzerOff();
        }
    }
    else{
        if(screen == 1) screen = 0;
        btnset = 0;
        btndec = 0U;   
        btninc = 0U;
        ctrl1 = 0;
        ctrl2 = 0;
        ctrl3 = 0;
    }
    DISPResetScrnsvr(); 
}
/**
  * @brief
  * @param
  * @retval
  */
uint8_t DISPMenuSettings(uint8_t btn){
    static uint8_t last_state = 0U;
    static uint32_t menu_tmr = 0U;
    if      ((btn == 1U) && (last_state == 0U)){
        last_state = 1U;
        menu_tmr = HAL_GetTick(); 
    } else if ((btn == 1U) && (last_state == 1U)){
        if((HAL_GetTick() - menu_tmr) >= SETTINGS_MENU_ENABLE_TIME){
            last_state = 0U;
            return (1U);
        }
    } else if ((btn == 0U) && (last_state == 1U)) last_state = 0U;    
    return (0U);
}

void DISP_UpdateLog(const char *pbuf){
    static char displog[6][128];	
    int i = 5;	
    GUI_ClearRect(120, 80, 480, 240);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_TOP);
    GUI_SetBkColor(GUI_TRANSPARENT);
    GUI_SetFont(&GUI_Font16B_1);
	GUI_SetColor(GUI_WHITE);    
    do{
        ZEROFILL(displog[i], COUNTOF(displog[i]));
        strcpy(displog[i], displog[i-1]);
        GUI_DispStringAt(displog[i], 125, 200-(i*20));
    }while(--i);
    GUI_SetColor(GUI_YELLOW);
    ZEROFILL(displog[0], COUNTOF(displog[0]));
    strcpy(displog[0], pbuf);
	GUI_DispStringAt(displog[0], 125, 200);
    GUI_Exec();	
}

void SaveLightController(LIGHT_CtrlTypeDef* lc, uint16_t addr){
    uint8_t buf[8];
    buf[0] = lc->Main1.index;
    buf[1] = lc->Led1.index;
    buf[2] = lc->Led2.index;
    buf[3] = lc->Led3.index;
    buf[4] = lc->Out1.index;
    buf[5] = lc->Light1.index;
    buf[6] = lc->Light2.index;
    buf[7] = lc->Light3.index;
    EE_WriteBuffer(buf, addr, 8);
}

void ReadLightController(LIGHT_CtrlTypeDef* lc, uint16_t addr){
    uint8_t buf[8];
    EE_ReadBuffer(buf, addr, 8);
    lc->Main1.index     = buf[0];
    lc->Led1.index      = buf[1];
    lc->Led2.index      = buf[2];
    lc->Led3.index      = buf[3];
    lc->Out1.index      = buf[4];
    lc->Light1.index    = buf[5];
    lc->Light2.index    = buf[6];
    lc->Light3.index    = buf[7];
}

//void SaveThermostatController(THERMOSTAT_TypeDef* tc, uint16_t addr){
//    uint8_t buf[21];
//    buf[0] = tc->th_ctrl;
//    buf[1] = tc->th_state;
//    buf[2] = tc->mv_temp>>8;
//    buf[3] = tc->mv_temp&0xFF;
//    buf[4] = tc->mv_offset;
//    buf[5] = tc->mv_ntcref>>8;
//    buf[6] = tc->mv_ntcref&0xFF;
//    buf[7] = tc->mv_nctbeta>>8;
//    buf[8] = tc->mv_nctbeta&0xFF;
//    buf[9] = tc->sp_temp;
//    buf[10] = tc->sp_diff;
//    buf[11] = tc->sp_max;
//    buf[12] = tc->sp_min;
//    buf[13] = tc->fan_ctrl;
//    buf[14] = tc->fan_speed;
//    buf[15] = tc->fan_diff;
//    buf[16] = tc->fan_loband;
//    buf[17] = tc->fan_hiband;
//    buf[18] = tc->fan_quiet_start;
//    buf[19] = tc->fan_quiet_end;
//    buf[20] = tc->fan_quiet_speed;
//    EE_WriteBuffer(buf, addr, 21);
//}

//void ReadThermostatController(THERMOSTAT_TypeDef* tc, uint16_t addr){
//    uint8_t buf[21];
//    EE_ReadBuffer(buf, addr, 21);
//    tc->th_ctrl         = buf[0];
//    tc->th_state        = buf[1];
//    tc->mv_temp         =(buf[2]<<8)|buf[3];
//    tc->mv_offset       = buf[4];
//    tc->mv_ntcref       =(buf[5]<<8)|buf[6];
//    tc->mv_nctbeta      =(buf[7]<<8)|buf[8];
//    tc->sp_temp         = buf[9];
//    tc->sp_diff         = buf[10];
//    tc->sp_max          = buf[11];
//    tc->sp_min          = buf[12];
//    tc->fan_ctrl        = buf[13];
//    tc->fan_speed       = buf[14];
//    tc->fan_diff        = buf[15];
//    tc->fan_loband      = buf[16];
//    tc->fan_hiband      = buf[17];
//    tc->fan_quiet_start = buf[18];
//    tc->fan_quiet_end   = buf[19];
//    tc->fan_quiet_speed = buf[20];
//}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/

