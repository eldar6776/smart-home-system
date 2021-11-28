/**
 ******************************************************************************
 * File Name          : display.c
 * Date               : 21/08/2016 20:59:16
 * Description        : display GUI function set
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
#if (__DISP_H__ != FW_BUILD)
    #error "display header version mismatch"
#endif
/* Includes ------------------------------------------------------------------*/
#include "ff.h"
#include "rtc.h"
#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "i2cee.h"
#include "httpd.h"
#include "fs5206.h"
#include "buzzer.h"
#include "eth_bsp.h"
#include "netconf.h"
#include "netbios.h"
#include "display.h"
#include "spi_flash.h"
#include "tftpserver.h"
#include "hotel_ctrl.h"
#include "stm32f4x7_eth.h"
#include "stm32f429i_lcd.h"
/* Imported variables --------------------------------------------------------*/
extern GUI_CONST_STORAGE GUI_BITMAP bmNetworkError;
extern GUI_CONST_STORAGE GUI_BITMAP bmNetworkConnected;
extern GUI_CONST_STORAGE GUI_BITMAP bmusd_ok;
extern GUI_CONST_STORAGE GUI_BITMAP bmusd_error;
extern TinyFrame tfapp;
extern TF_Result ID_Listener(TinyFrame *tf, TF_Msg *msg);
/* Private define ------------------------------------------------------------*/
PROGBAR_Handle hFileTransfer;

#define OSK_IDLE					0
#define OSK_EDIT_IPADD			    1
#define OSK_EDIT_GWADD			    2
#define OSK_EDIT_SUBNET		        3
#define OSK_EDIT_RS485_INTERFACE	4
#define OSK_EDIT_RS485_BROADCAST	5
#define OSK_EDIT_PASSWORD			6
#define OSK_ENTER_PASSWORD			7
#define OSK_EDIT_SYS_ID				8

#define USD_H_POS                   15
#define USD_V_POS                   5
#define USD_H_SIZE                  40
#define USD_V_SIZE                  70
#define USD_TX_H_POS                (USD_H_POS + (USD_H_SIZE / 2))
#define USD_TX_V_POS                (USD_V_POS + 55)
#define ICO_H_TAB                   20
#define NET_H_POS                   (USD_H_POS + USD_H_SIZE+ ICO_H_TAB)
#define NET_V_POS                   5
#define NET_H_SIZE                  40
#define NET_V_SIZE                  70
#define NET_TX_H_POS                (NET_H_POS + (NET_H_SIZE / 2))
#define NET_TX_V_POS                (NET_V_POS + 55)

#define USER_LOGO_H_POS             120
#define USER_LOGO_V_POS             0
#define USER_LOGO_H_SIZE            360
#define USER_LOGO_V_SIZE            60
/* Exported types ------------------------------------------------------------*/
typedef enum{
	DEFAULT     = ((uint8_t)0x0U),
    SETTINGS    = ((uint8_t)0x1U),
    CONTROL     = ((uint8_t)0x2U),
    TOOLS       = ((uint8_t)0x3U),
    EDIT        = ((uint8_t)0x4U),
    RS485       = ((uint8_t)0x5U)
}DISP_ActiveScreenTypeDef;

DISP_ActiveScreenTypeDef ActiveScreen;
/* Private variables ---------------------------------------------------------*/
char logbuf[128];
uint8_t row = 0;
uint8_t usd_status;
uint8_t cursor_pos;
uint32_t disp_flag;
uint32_t disp_tmr;
uint32_t disp_toutmr;
int32_t ret_1, ret_2, ret_3, ret_4;
/* Function prototypes -------------------------------------------------------*/
void DISP_UpdateDateTime(void);
int APP_GetData(void * p, const unsigned char ** ppData, unsigned NumBytes, unsigned long Off);
/* Private functions ---------------------------------------------------------*/
void DISP_Init(void){
	GUI_Init();
	GUI_SelectLayer(0);
	GUI_Clear();
	if (f_mount(&fatfs, "0:", 0x0U) == FR_OK){
		if (f_opendir(&dir_1, "/") == FR_OK){
			if (f_open(&file_SD, "BCK_GND.BMP", FA_READ) == FR_OK){
				GUI_BMP_DrawEx(APP_GetData, (void *) &file_SD, 0x0U, 0x0U);
				DISP_uSDCardReadySet();
			} else DISP_uSDCardErrorSet();
		} else DISP_uSDCardErrorSet();
	} else DISP_uSDCardErrorSet();
	f_mount(NULL, "0:", 0x0U);
	GUI_SelectLayer(1);
	GUI_SetBkColor(GUI_TRANSPARENT); 
	GUI_Clear();
	GUI_SetFont(&GUI_Font20B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	hFileTransfer = PROGBAR_CreateEx(120, 50, 340, 25, 0, WM_CF_SHOW, 0, GUI_ID_PROGBAR0);
	PROGBAR_SetMinMax(hFileTransfer, 0, 100);
	PROGBAR_SetTextColor(hFileTransfer, 1, GUI_WHITE);
	PROGBAR_SetBarColor(hFileTransfer, 1, GUI_LIGHTGRAY);
	PROGBAR_SetTextAlign(hFileTransfer, GUI_TA_HCENTER);
	PROGBAR_SetFont(hFileTransfer, &GUI_Font13_1);
	PROGBAR_SetText(hFileTransfer, "0%");
	PROGBAR_SetSkin(hFileTransfer, PROGBAR_SKIN_FLEX);	
	WM_HideWindow(hFileTransfer);
}

void DISP_Service(void){
    static uint32_t gui_clock = 0;
    static uint32_t gui_timing = 0;
    static uint32_t touch_timing = 0;    
    
	if((Get_SysTick() - touch_timing) >= 10){
		touch_timing = Get_SysTick();
		GUI_TOUCH_Exec();
	}    
    if((Get_SysTick() - gui_timing) >= 100){
		gui_timing = Get_SysTick();
        if      (IsDISP_UpdateTimeActiv()) DISP_UpdateDateTime();
        else if (gui_clock) --gui_clock;
        else{
            gui_clock = 600;
            DISP_UpdateDateTime();
        }
		GUI_Exec();	
	}    
}
      
int APP_GetData(void * p, const U8 * * ppData, unsigned NumBytesReq, U32 Off){
	UINT NumBytesRead;
	FIL *phFile = (FIL*)p;
	static char _acBuffer[0x1000];	
	f_lseek(phFile,Off);
	f_read(phFile, _acBuffer, NumBytesReq, &NumBytesRead);
	*ppData = (const U8*) &_acBuffer;
	return NumBytesRead;
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

void DISP_FileTransferState(uint8_t nsta){
	GUI_ClearRect(120, 0, 480, 50);
//	GUI_SetColor((0x62ul << 24) | GUI_BLACK);
//	GUI_FillRect(120, 65, 475, 105);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_TOP);
    GUI_SetTextMode(GUI_TEXTMODE_TRANS);
    GUI_SetFont(&GUI_Font24_1);
	GUI_SetColor(GUI_WHITE);
	GUI_GotoXY(125, 20);
    //ADDRESS_ERROR
    //VALUE_TYPE_ERROR
    //VALUE_MISSING_ERROR 
    //RECEIVER_ADDRESS_ERROR
    //WRITE_ADDRESS_ERRROR
	if      (nsta == FS_DRIVE_ERR) 			GUI_DispString(" UPDATE FAIL - FILE SYS. ERROR");
	else if (nsta == FS_DIRECTORY_ERROR)    GUI_DispString(" UPDATE FAIL - FILE DIR. ERROR");
	else if (nsta == FS_FILE_ERROR)         GUI_DispString(" UPDATE FAIL - FILE ERROR");
	else if (nsta == FW_UPDATE_RUN)         GUI_DispString(" FIRMWARE UPDATE RUN");
    else if (nsta == FW_UPDATE_END)         GUI_DispString(" FIRMWARE UPDATE SUCCESS");
	else if (nsta == BLDR_UPDATED)          GUI_DispString(" BOOTLOADER UPDATE SUCCESS");
	else if (nsta == FILE_UPDATE_FINISHED)  GUI_DispString(" FILE UPDATE SUCCESS");
	else if (nsta == FILE_UPDATE_FAIL)      GUI_DispString(" FILE UPDATE FAIL - TRANSFER  ERROR");
    else if (nsta == FW_UPDATE_FAIL)        GUI_DispString(" FW UPDATE FAIL - TRANSFER  ERROR");
    GUI_Exec();	
}

void DISP_ProgbarSetNewState(uint8_t nsta){
    switch (nsta){
        case 0:
            WM_HideWindow(hFileTransfer);
            GUI_ClearRect(120, 50, 480, 80);
            break;        
        case 1:
            PROGBAR_SetValue(hFileTransfer, 0);
            WM_ShowWindow(hFileTransfer);
            break;
        default:
            PROGBAR_SetValue(hFileTransfer, nsta);
            break;
    }
    GUI_Exec();	
}


void DISP_uSDCardSetNewState(uint8_t nsta){
    switch (nsta){
        case 0:
            DISP_uSDCardErrorSet();
            GUI_ClearRect(USD_H_POS, USD_V_POS, USD_H_POS + USD_H_SIZE, USD_V_POS + USD_V_SIZE);
            GUI_DrawBitmap(&bmusd_error, USD_H_POS, USD_V_POS);
            GUI_SetColor(GUI_RED);
            GUI_SetBkColor(GUI_BLACK);
            GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
            GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
            GUI_GotoXY(USD_TX_H_POS, USD_TX_V_POS);
            GUI_SetFont(&GUI_Font16B_1);
            GUI_DispString("uSD");
            GUI_SetBkColor(GUI_TRANSPARENT);
            break;        
        case 1:
            DISP_uSDCardReadySet();
            GUI_ClearRect(USD_H_POS, USD_V_POS, USD_H_POS + USD_H_SIZE, USD_V_POS + USD_V_SIZE);
            GUI_DrawBitmap(&bmusd_ok, USD_H_POS, USD_V_POS);
            GUI_SetColor(GUI_GREEN);
            GUI_SetBkColor(GUI_BLACK);
            GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
            GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
            GUI_GotoXY(USD_TX_H_POS, USD_TX_V_POS);
            GUI_SetFont(&GUI_Font16B_1);
            GUI_DispString("uSD");
            GUI_SetBkColor(GUI_TRANSPARENT);
            break;
    }
    GUI_Exec();	
}



void DISP_UpdateDateTime(void){
    char disp[16];
    uint32_t x;
    uint8_t buf[9];
	static uint8_t old_min = 60;    
    RTC_GetTime(RTC_Format_BCD, &rtc_time);
    RTC_GetDate(RTC_Format_BCD, &rtc_date);
	if(ActiveScreen != DEFAULT) return;
	else if((old_min != rtc_time.RTC_Minutes) || IsDISP_UpdateTimeActiv()){
		DISP_UpdateTimeReset();
		old_min = rtc_time.RTC_Minutes;
		GUI_ClearRect(5, 65, 115, 200);
		GUI_SetColor((0x62 << 24) | GUI_BLACK);
		GUI_FillRect(5, 65, 115, 200);
		GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
        Hex2Str(disp, &rtc_time.RTC_Hours, 2);
		disp[2] = ':';
        Hex2Str(&disp[3], &rtc_time.RTC_Minutes, 2);
		GUI_GotoXY(60, 100);
		GUI_SetColor(GUI_RED);
		GUI_SetFont(&GUI_Font32B_1);
		GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
		GUI_DispString(disp);
        if (rtc_date.RTC_WeekDay == 0) ++rtc_date.RTC_WeekDay;
		GUI_GotoXY(60, 130);		
		GUI_SetColor(GUI_WHITE);
		GUI_SetFont(&GUI_Font24_1);
		GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
		GUI_DispString(day[rtc_date.RTC_WeekDay]);		
        Hex2Str(disp, &rtc_date.RTC_Date, 2);
		disp[2] = '.';
		strcpy(&disp[3], month[Bcd2Dec(rtc_date.RTC_Month)]);
        x = strlen(disp);
		if(x > 10U) GUI_SetFont(&GUI_Font16B_1);
		else GUI_SetFont(&GUI_Font20B_1);
		GUI_SetColor(GUI_RED);
		GUI_GotoXY(60, 160);
		GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
		GUI_DispString(disp);
		memcpy(disp, "20  \0", 5);
        Hex2Str(&disp[2], &rtc_date.RTC_Year, 2);
		GUI_SetFont(&GUI_Font20B_1);
		GUI_SetColor(GUI_WHITE);
		GUI_GotoXY(60, 190);
		GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
		GUI_DispString(disp);
		GUI_Exec();
        buf[0] = SET_RTC_DATE_TIME;
        buf[1] = DEF_TFBRA;
        buf[2] = rtc_date.RTC_WeekDay;
        buf[3] = rtc_date.RTC_Date;
        buf[4] = rtc_date.RTC_Month;
        buf[5] = rtc_date.RTC_Year;
        buf[6] = rtc_time.RTC_Hours;
        buf[7] = rtc_time.RTC_Minutes;
        buf[8] = rtc_time.RTC_Seconds;
        TF_SendSimple(&tfapp, S_CUSTOM, buf, 9);
	}
}
/*************************** End of file ****************************/
