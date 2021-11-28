/**
 ******************************************************************************
 * @file    httpd_cg_ssi.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    31-October-2011
 * @brief   Webserver SSI and CGI handlers
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; Portions COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */
 
#if (__COMMON_H__ != FW_TIME)
    #error "common header version mismatch"
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
#include "wiegand.h"
#include "spi_flash.h"
#include "tftpserver.h"
#include "stm32f4x7_eth.h"
#include "stm32f429i_lcd.h"
#include "sdio_sd.h"
/* Imported Types  -----------------------------------------------------------*/
/* Imported Variables --------------------------------------------------------*/
/* Imported Functions    -----------------------------------------------------*/
/* Private Typedef -----------------------------------------------------------*/

/* Private Define  -----------------------------------------------------------*/
#define ADDRESS_ERROR                   190
#define VALUE_TYPE_ERROR                191
#define VALUE_MISSING_ERROR             192
#define VALUE_OUTOFRANGE_ERROR          193
#define RECEIVER_ADDRESS_ERROR          194
#define WRITE_ADDRESS_ERRROR            195
#define FILE_TRANSFER_ERRROR            196    
#define RESPONSE_TIMEOUT                197
#define COMMAND_FAIL                    198    

#define NEW_FILE_EXTFLASH_ADDRESS       0x90F00000              // default 1MByte storage address for new firmware & bootloader file
#define TempRegOn(x)                    (x |=  (0x1U << 0))     // config On: controll loop is executed periodicaly
#define TempRegOff(x)                   (x &=(~(0x1U << 0)))    // config Off:controll loop stopped,
#define IsTempRegActiv(x)               (x &   (0x1U << 0))
#define TempRegHeating(x)               (x |=  (0x1U << 1))     // config Heating: output activ for setpoint value 
#define TempRegCooling(x)               (x &=(~(0x1U << 1)))    // config Cooling: opposite from heating
#define IsTempRegHeating(x)             (x &   (0x1U << 1))
#define TempRegEnable(x)                (x |=  (0x1U << 2))     // controll flag Enable: controll loop set output state
#define TempRegDisable(x)               (x &=(~(0x1U << 2)))    // controll flag Disable:output is forced to inactiv state, controll loop cannot change outpu
#define IsTempRegEnabled(x)             (x &   (0x1U << 2))
#define TempRegOutOn(x)                 (x |=  (0x1U << 3))     // status On: output demand for actuator to inject energy in to system
#define TempRegOutOff(x)                (x &=(~(0x1U << 3)))    // status Off:stop demanding energy for controlled system, setpoint is reached
#define IsTempRegOutActiv(x)            (x &   (0x1U << 3))
#define TempRegNewStaSet(x)             (x |=  (0x1U << 4))
#define TempRegNewModSet(x)             (x |=  (0x1U << 5))
#define TempRegNewCtrSet(x)             (x |=  (0x1U << 6))
#define TempRegNewOutSet(x)             (x |=  (0x1U << 7))
/* Private Variables  --------------------------------------------------------*/
static bool init_tf = false;
TinyFrame tfapp;
static int retval, rdy = 0;
tCGI CGI_TAB[12]        = { 0 };    // Cgi call table, only one CGI used
const char  *TAGCHAR    = {"t"};    // use character "t" as tag for CGI */
const char **TAGS       = &TAGCHAR;
const char  *weblog     = {"/log.html"};
const char  *webctrl    = {"/sysctrl.html"};
/* Private macros   ----------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
void RS485_Tick(void);
void TINYFRAME_Init(void);
uint8_t SendFwInfo(void);
uint8_t HTTP2RS485(void);
void RS485_Receive(uint16_t rxb);
TF_Result ID_Listener(TinyFrame *tf, TF_Msg *msg);
uint8_t HC2RT_Link(uint8_t *txbuf, uint8_t *rxbuf);
const char  *HTTP_CGI_Handler(int iIndex, int iNumParams, char **pcParam, char **pcValue); // CGI handler for incoming http request */
const tCGI   HTTP_CGI   = {"/sysctrl.cgi", HTTP_CGI_Handler};   // Html request for "/sysctrl.cgi" will start HTTP_CGI_Handler
/* Program Code  -------------------------------------------------------------*/
u16_t HTTP_ResponseHandler(int iIndex, char *pcInsert, int iInsertLen){
    int len;
    if (iIndex == 0){
        if (retval == FS_DRIVE_ERR){
            strcpy (pcInsert, "DRIVE_ERROR");
        }else if (retval == FS_DIRECTORY_ERROR){
            strcpy (pcInsert, "DIRECTORY_ERROR");
        }else if (retval == FS_FILE_ERROR){
            strcpy (pcInsert, "FILE_ERROR");
        }else if (retval == FS_FILE_OK){
            strcpy (pcInsert, "OK");
        }else if (retval == ADDRESS_ERROR){
            strcpy (pcInsert, "ADDRESS_ERROR");
        }else if (retval == VALUE_TYPE_ERROR){
            strcpy (pcInsert, "VALUE_TYPE_ERROR");
        }else if (retval == VALUE_MISSING_ERROR){
            strcpy (pcInsert, "VALUE_MISSING_ERROR");
        }else if (retval == VALUE_OUTOFRANGE_ERROR){
            strcpy (pcInsert, "VALUE_OUTOFRANGE_ERROR");
        }else if (retval == FILE_TRANSFER_ERRROR){
            strcpy (pcInsert, "FILE_TRANSFER_ERRROR");
        }else if (retval == RESPONSE_TIMEOUT){
            strcpy (pcInsert, "RESPONSE_TIMEOUT");
        }else if (retval == COMMAND_FAIL){
            strcpy (pcInsert, "COMMAND_FAIL");
        }
        DISP_FileTransferState(retval);
    }
    len = strlen(pcInsert);
    if(!len) len = iInsertLen;
    return len;
}
/**
 * @brief  CGI handler for HTTP request 
 */
const char *HTTP_CGI_Handler(int iIndex, int iNumParams, char **pcParam, char **pcValue){
    uint8_t buf[512], sta = 0, ifadd = 0;
    uint32_t dr = 0, wradd = 0, remain = 0;
    
    retval = 0;
    
    if (iIndex == 0){
        if      (!strcmp(pcParam[0],"RELAY")){  // SET RELAY STATE
            if(ISVALIDDEC(*pcValue[0])){
                if (!strcmp(pcParam[1], "VALUE")){
                    if(ISVALIDDEC(*pcValue[1])){
                        buf[0] = atoi(pcValue[0]); 
                        buf[1] = atoi(pcValue[1]);
                        TF_QuerySimple(&tfapp, S_BINARY, buf, 2, ID_Listener, TF_PARSER_TIMEOUT_TICKS*4);
                        rdy = TF_PARSER_TIMEOUT_TICKS*2;
                        do{
                            --rdy;
                            DelayMs(1);
#ifdef USE_WATCHDOG
                            IWDG_ReloadCounter();
#endif
                        }while(rdy > 0);                
                        if (rdy == 0) {
                            retval = RESPONSE_TIMEOUT;
                        } else retval = FS_FILE_OK;
                    } else retval = VALUE_TYPE_ERROR; 
                } else  retval = VALUE_MISSING_ERROR; 
            } else retval = VALUE_TYPE_ERROR;
        }else if(!strcmp(pcParam[0],"DIMMER")){ // SET DIMMER STATE
            if(ISVALIDDEC(*pcValue[0])){
                if (!strcmp(pcParam[1], "VALUE")){
                    if(ISVALIDDEC(*pcValue[1])){
                        buf[0] = atoi(pcValue[0]); 
                        buf[1] = atoi(pcValue[1]);
                        TF_QuerySimple(&tfapp, S_DIMMER, buf, 2, ID_Listener, TF_PARSER_TIMEOUT_TICKS*4); 
                        rdy = TF_PARSER_TIMEOUT_TICKS*2;
                        do{
                            --rdy;
                            DelayMs(1);
#ifdef USE_WATCHDOG
                            IWDG_ReloadCounter();
#endif
                        }while(rdy > 0);                
                        if (rdy == 0) {
                            retval = RESPONSE_TIMEOUT;
                        } else retval = FS_FILE_OK;
                    } else retval = VALUE_TYPE_ERROR; 
                } else  retval = VALUE_MISSING_ERROR; 
            } else retval = VALUE_TYPE_ERROR;
        }else if(!strcmp(pcParam[0],"DATE")){   // SET DATE & TIME
            if(ISVALIDDEC(*pcValue[0])){
                if (!strcmp(pcParam[1], "TIME")){
                    if(ISVALIDDEC(*pcValue[1])){
                        RTC_TimeTypeDef tm;
                        RTC_DateTypeDef dt;
                        dt.RTC_WeekDay = CONVERTDEC(pcValue[0]);
                        if(!rtc_date.RTC_WeekDay)dt.RTC_WeekDay=7;
                        Str2Hex (pcValue[0]+1, &dt.RTC_Date,    2);
                        Str2Hex (pcValue[0]+3, &dt.RTC_Month,   2);
                        Str2Hex (pcValue[0]+5, &dt.RTC_Year,    2);
                        Str2Hex (pcValue[1],   &tm.RTC_Hours,   2);
                        Str2Hex (pcValue[1]+2, &tm.RTC_Minutes, 2);
                        Str2Hex (pcValue[1]+4, &tm.RTC_Seconds, 2);
                        PWR_BackupAccessCmd(ENABLE);
                        rdy  = RTC_SetTime(RTC_Format_BCD, &tm);
                        rdy += RTC_SetDate(RTC_Format_BCD, &dt);
                        PWR_BackupAccessCmd(DISABLE);
                        RTC_State = RTC_VALID;
                        DISP_UpdateTimeSet();
                        if (rdy == 0) {
                            retval = COMMAND_FAIL;
                        } else retval = FS_FILE_OK;
                    } else retval = VALUE_TYPE_ERROR; 
                } else  retval = VALUE_MISSING_ERROR; 
            } else retval = VALUE_TYPE_ERROR;
        }else if(!strcmp(pcParam[0],"CMD")){    // EXE COMMAND
            if  (!strcmp(pcParam[1],"ADDRESS")){
                if(ISVALIDDEC(*pcValue[1])){
                    buf[0] = 0;
                    buf[1] = atoi(pcValue[1]);
                    if      (strcmp(pcValue[0], "RESTART") == 0){
                        buf[0] = RESTART_CTRL;
                    }else if(strcmp(pcValue[0], "DEFAULT") == 0){
                        buf[0] = LOAD_DEFAULT;
                    }else if(strcmp(pcValue[0], "EXTFLASH") == 0){
                        buf[0] = FORMAT_EXTFLASH;
                    }else if(strcmp(pcValue[0], "GETSTATE") == 0){
                        buf[0] = GET_APPL_STAT;
                    }else retval = VALUE_TYPE_ERROR;
                    if (buf[0]){
                        TF_QuerySimple(&tfapp, S_CUSTOM, buf, 2, ID_Listener, TF_PARSER_TIMEOUT_TICKS*4);
                        rdy = TF_PARSER_TIMEOUT_TICKS*2;
                        do {
                            --rdy;
                            DelayMs(1);
#ifdef USE_WATCHDOG
                            IWDG_ReloadCounter();
#endif
                        } while(rdy > 0);
                        if (rdy == 0) {
                            retval = RESPONSE_TIMEOUT;
                        } else retval = FS_FILE_OK;
                    } else retval = VALUE_TYPE_ERROR;
                } else retval = VALUE_TYPE_ERROR; 
            } else  retval = VALUE_MISSING_ERROR;
        }else if(!strcmp(pcParam[0],"SETPOINT")){   // SET DATE & TIME
            if(ISVALIDDEC(*pcValue[0])){
                if (!strcmp(pcParam[1], "ADDRESS")){
                    if(ISVALIDDEC(*pcValue[1])){
                        buf[0] = atoi(pcValue[0]); 
                        buf[1] = atoi(pcValue[1]);
                        TF_QuerySimple(&tfapp, S_TEMP, buf, 2, ID_Listener, TF_PARSER_TIMEOUT_TICKS*4); 
                        rdy = TF_PARSER_TIMEOUT_TICKS*2;
                        do{
                            --rdy;
                            DelayMs(1);
#ifdef USE_WATCHDOG
                            IWDG_ReloadCounter();
#endif
                        }while(rdy > 0);                
                        if (rdy == 0) {
                            retval = RESPONSE_TIMEOUT;
                        } else retval = FS_FILE_OK;
                    } else retval = VALUE_TYPE_ERROR; 
                } else  retval = VALUE_MISSING_ERROR; 
            } else retval = VALUE_TYPE_ERROR;
        }else if(!strcmp(pcParam[0],"FILE")){   // UPDATE FIRMWARE
            if (f_mount(&fatfs, "0:", 0) == FR_OK){
                if (f_opendir(&dir_1, "/") == FR_OK){
                    if (f_open(&file_SD, pcValue[0], FA_READ) == FR_OK){
                        if (!strcmp(pcParam[1],"WRITE")){
                            if (!strcmp(pcParam[2],"ADDRESS")){
                                wradd = Str2Int(pcValue[1],0);
                                ifadd = Str2Int(pcValue[2],0);
                            } else retval = VALUE_MISSING_ERROR;
                        } else if (!strcmp(pcValue[0],"IC.BIN") || !strcmp(pcValue[0],"ICBL.BIN")){
                            if (!strcmp(pcParam[1],"ADDRESS")){
                                wradd = 0x90F00000;
                                ifadd = Str2Int(pcValue[1],0);
                            } else retval = VALUE_MISSING_ERROR;
                        }else if (!strcmp(pcValue[0],"EXT.BIN")){
                            if (!strcmp(pcParam[1],"ADDRESS")){
                                wradd = 0x90000000;
                                ifadd = Str2Int(pcValue[1],0);
                            } else retval = VALUE_MISSING_ERROR;
                        }else retval = VALUE_TYPE_ERROR; 
                        if ((wradd > 0x08000000) && (wradd < 0x91000000)){   
                            if ((ifadd > 0) && (ifadd < 0xFF)){
                                    retval = FS_FILE_OK;
                            } else retval = VALUE_OUTOFRANGE_ERROR;
                        } else retval = ADDRESS_ERROR;                   
                    } else retval = FS_FILE_ERROR;
                } else retval = FS_DIRECTORY_ERROR;
            } else retval = FS_DRIVE_ERR;          
                
            if (retval == FS_FILE_OK){
                buf[0] = wradd>>24;
                buf[1] = wradd>>16;
                buf[2] = wradd>>8;
                buf[3] = wradd;
                buf[4] = file_SD.obj.objsize>>24;
                buf[5] = file_SD.obj.objsize>>16;
                buf[6] = file_SD.obj.objsize>>8;
                buf[7] = file_SD.obj.objsize;
                buf[8]= ifadd;
                buf[9]= ST_FIRMWARE_REQUEST;
                remain = file_SD.obj.objsize;
                DISP_ProgbarSetNewState(1);
                DISP_FileTransferState(FW_UPDATE_RUN);
                TF_QuerySimple(&tfapp, ST_FIRMWARE_REQUEST, buf, 10, ID_Listener, 30000);
                rdy = 29000;
                while(--rdy > 0){
                    DelayMs(1);
#ifdef USE_WATCHDOG
                    IWDG_ReloadCounter();
#endif                
                }
                while(remain > 0){
                    f_read(&file_SD, buf, sizeof(buf), &dr);
                    remain -= dr;
                    sta = (100 -((remain*100)/file_SD.obj.objsize));
                    if (sta > 1) DISP_ProgbarSetNewState(sta);
                    TF_QuerySimple(&tfapp, ST_FIRMWARE_REQUEST, buf, dr, ID_Listener, TF_PARSER_TIMEOUT_TICKS*4);
                    rdy = TF_PARSER_TIMEOUT_TICKS*2;
                    do{
                        --rdy;
                        DelayMs(1);
#ifdef USE_WATCHDOG
            IWDG_ReloadCounter();
#endif
                    }while(rdy > 0);                
                    if (rdy == 0) {
                        retval = FILE_TRANSFER_ERRROR;
                        DISP_ProgbarSetNewState(0);
                        return weblog;
                    }
                }
                DISP_ProgbarSetNewState(0);
            } else f_mount(NULL,"0:",0);
            return weblog;            
        }
	}
    if (retval) return weblog;
    else        return webctrl;
}
/**
 * @brief  uljucuje LED
 * @param  1 - LED1
 * @param  2 - LED2
 * @param  3 - LED1 &L ED2
 * @retval None
 */
void led_set(uint8_t led){
    if      (led == 0U) GPIO_ResetBits(LED_GPIO_PORT,LED1_GPIO_PIN|LED2_GPIO_PIN);
    else if (led == 1U) GPIO_ResetBits(LED_GPIO_PORT,LED1_GPIO_PIN);
    else if (led == 2U) GPIO_ResetBits(LED_GPIO_PORT,LED2_GPIO_PIN);
}
/**
 * @brief  iskljucuje LED
 * @param  0 - LED1 &L ED2
 * @param  1 - LED1
 * @param  2 - LED2
 * @retval None
 */
void led_clr (uint8_t led){
    if      (led == 0U) GPIO_SetBits(LED_GPIO_PORT,LED1_GPIO_PIN|LED2_GPIO_PIN);
    else if (led == 1U) GPIO_SetBits(LED_GPIO_PORT,LED1_GPIO_PIN);
    else if (led == 2U) GPIO_SetBits(LED_GPIO_PORT,LED2_GPIO_PIN);  
}
/**
 * @brief  change LED state to oposite 1-0-1-0...
 * @param  0 - LED1 &L ED2
 * @param  1 - LED1
 * @param  2 - LED2
 * @retval None
 */
void led_tgl (uint8_t led){
    if      (led == 0U) GPIO_ToggleBits(LED_GPIO_PORT, LED1_GPIO_PIN|LED2_GPIO_PIN);
    else if (led == 1U) GPIO_ToggleBits(LED_GPIO_PORT, LED1_GPIO_PIN);
    else if (led == 2U) GPIO_ToggleBits(LED_GPIO_PORT, LED2_GPIO_PIN);
}
/**
 * @brief  insert data to HTTP response
 * @param  iIndex       = 0
 * @param  pcInsert     = pointer to response data buffer
 * @param  iInsertLen   = max. length of response data in bytes 
 * @retval length in bytes of response data set by handler 
 */
TF_Result ID_Listener(TinyFrame *tf, TF_Msg *msg){
    rdy = -1;
    return TF_CLOSE;
}
/**
  * @brief
  * @param
  * @retval
  */
TF_Result GEN_Listener(TinyFrame *tf, TF_Msg *msg){
    char buf[128];    
    sprintf(buf,"RECEIVED FRAME:TYPE=%d:ID=%d:LENGTH=%d%s", msg->type, msg->frame_id, msg->len, msg->is_response == true ? ":RESPONSE" : "\0");
    DISP_UpdateLog(buf); 
    return TF_STAY;
}
void httpd_ssi_init(void){
    http_set_ssi_handler(HTTP_ResponseHandler, (char const **) TAGS, 1);
}
void httpd_cgi_init(void){
    CGI_TAB[0] = HTTP_CGI;
    http_set_cgi_handlers(CGI_TAB, 1);
}
void RS485_Tick(void){
    if (init_tf == true) {
        TF_Tick(&tfapp);
    }
}
void RS485_Receive(uint16_t rxb){
    uint8_t tfrxb = rxb;
    TF_AcceptChar(&tfapp, tfrxb);
}
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len){
    RS485_Send((uint8_t*)buff, len);
}
void TINYFRAME_Init(void){
    
    init_tf = TF_InitStatic(&tfapp, TF_MASTER); // 1 = master, 0 = slave
    TF_AddGenericListener(&tfapp, GEN_Listener);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
