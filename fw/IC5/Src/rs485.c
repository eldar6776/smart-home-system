/**
 ******************************************************************************
 * Project          : KuceDzevadova
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

 
#if (__RS485_H__ != FW_BUILD)
    #error "rs485 header version mismatch"
#endif 
/* Includes ------------------------------------------------------------------*/
#include "png.h"
#include "main.h"
#include "rs485.h"
#include "display.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"
/* Imported Types  -----------------------------------------------------------*/
/* Imported Variables --------------------------------------------------------*/
/* Imported Functions    -----------------------------------------------------*/
/* Private Typedef -----------------------------------------------------------*/
static TinyFrame tfapp;
/* Private Define  -----------------------------------------------------------*/
/* Private Variables  --------------------------------------------------------*/
bool init_tf = false;
static uint32_t rstmr = 0;
static uint32_t wradd = 0;
static uint32_t bcnt = 0;
uint16_t sysid;
uint32_t rsflg, tfbps, dlen, etmr;
uint8_t lbuf[32], lcnt = 0, cmd = 0;
uint8_t  ethst, efan,  etsp,  rec, tfifa, tfgra, tfbra, tfgwa;
uint8_t *lctrl1 =(uint8_t*) &Ctrl1.Light;
/* Private macros   ----------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
TF_Result ID_Listener(TinyFrame *tf, TF_Msg *msg){
    return TF_CLOSE;
}
TF_Result FWREQ_Listener(TinyFrame *tf, TF_Msg *msg){          
    if (IsFwUpdateActiv()){
        MX_QSPI_Init();
        if (QSPI_Write ((uint8_t*)msg->data, wradd, msg->len) == QSPI_OK){
            wradd += msg->len; 
        }else{
            wradd = 0;
            bcnt = 0;
        }            
        MX_QSPI_Init();
        QSPI_MemMapMode();        
    }
    TF_Respond(tf, msg);
    rstmr = HAL_GetTick();
    return TF_STAY;
}
TF_Result GEN_Listener(TinyFrame *tf, TF_Msg *msg){
    if (!IsFwUpdateActiv()){
        if ((msg->data[9] == ST_FIRMWARE_REQUEST)&& (msg->data[8] == tfifa)){
            wradd = ((msg->data[0]<<24)|(msg->data[1]<<16)|(msg->data[2] <<8)|msg->data[3]);
            bcnt  = ((msg->data[4]<<24)|(msg->data[5]<<16)|(msg->data[6] <<8)|msg->data[7]);
            MX_QSPI_Init();
            if (QSPI_Erase(wradd, wradd + bcnt) == QSPI_OK){
                StartFwUpdate();
                TF_AddTypeListener(&tfapp, ST_FIRMWARE_REQUEST, FWREQ_Listener);
                TF_Respond(tf, msg);
                rstmr = HAL_GetTick();
            }else{
                wradd = 0;
                bcnt = 0;
            }
            MX_QSPI_Init();
            QSPI_MemMapMode();
        }else if((msg->data[1] == tfifa)
            &&  ((msg->data[0] == RESTART_CTRL)
            ||  (msg->data[0] == LOAD_DEFAULT)
            ||  (msg->data[0] == FORMAT_EXTFLASH)
            ||  (msg->data[0] == GET_APPL_STAT))){
                TF_Respond(tf, msg);
                cmd = msg->data[0];
        }else if(msg->data[1] == DEF_TFBRA){
            if  (msg->data[0] == SET_RTC_DATE_TIME){
                rtcdt.WeekDay = msg->data[2];
                rtcdt.Date    = msg->data[3];
                rtcdt.Month   = msg->data[4];
                rtcdt.Year    = msg->data[5];
                rtctm.Hours   = msg->data[6];
                rtctm.Minutes = msg->data[7];
                rtctm.Seconds = msg->data[8];
                HAL_RTC_SetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
                HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
                RtcTimeValidSet();
            }
        }
    }
    return TF_STAY;
}
/**
* @brief :  init usart interface to rs485 9 bit receiving 
* @param :  and init state to receive packet control block 
* @retval:  wait to receive:
*           packet start address marker SOH or STX  2 byte  (1 x 9 bit)
*           packet receiver address 4 bytes msb + lsb       (2 x 9 bit)
*           packet sender address msb + lsb 4 bytes         (2 x 9 bit)
*           packet lenght msb + lsb 4 bytes                 (2 x 9 bit)
*/
void RS485_Init(void){
    if(!init_tf){
        init_tf = TF_InitStatic(&tfapp, TF_SLAVE); // 1 = master, 0 = slave
        TF_AddGenericListener(&tfapp, GEN_Listener);
    }
	HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
* @brief  : rs485 service function is  called from every
* @param  : main loop cycle to service rs485 communication
* @retval : receive and send on regulary base 
*/
void RS485_Service(void){
    uint8_t i; 
    if (IsFwUpdateActiv()){
        if(HAL_GetTick() > rstmr + 5000){
            TF_RemoveTypeListener(&tfapp, ST_FIRMWARE_REQUEST);
            StopFwUpdate();
            wradd = 0;
            bcnt = 0;
        }
    } else if (HAL_GetTick() - etmr >= TF_PARSER_TIMEOUT_TICKS){
        if (cmd){
            switch (cmd){
                case LOAD_DEFAULT:
                    i = 1;
                    EE_WriteBuffer(&i, EE_INIT_ADDR, 1);
                case RESTART_CTRL:
                    SYSRestart();
                    break;
                case FORMAT_EXTFLASH:
                    MX_QSPI_Init();
                    QSPI_Erase(0x90000000, 0x90FFFFFF);
                    MX_QSPI_Init();
                    QSPI_MemMapMode();
                    break;
                case GET_APPL_STAT:
                    PresentSystem();
                    HAL_UART_Receive_IT(&huart1, &rec, 1);
                    break;
            }
            cmd = 0;
        } else if (lcnt) {
            TF_QuerySimple(&tfapp, S_BINARY, lbuf, lcnt, ID_Listener, TF_PARSER_TIMEOUT_TICKS);
            etmr = HAL_GetTick();
            lcnt = 0;
        } else {
            lcnt = 0;
            ZEROFILL(lbuf,COUNTOF(lbuf));
            lctrl1 =(uint8_t*)&Ctrl1.Light;
            for(i = 0; i < 3; i++){
                if (*(lctrl1+3) != *(lctrl1+2)){
                    *(lctrl1+3)  = *(lctrl1+2);
                    if (*lctrl1){
                        lbuf[lcnt++] = *lctrl1;
                        lbuf[lcnt++] = *(lctrl1+2);                       
                    }
                }
                lctrl1 += 4;
            }
        }
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void RS485_Tick(void){
    if (init_tf == true) {
        TF_Tick(&tfapp);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len){
    HAL_UART_Transmit(&huart1,(uint8_t*)buff, len, RESP_TOUT);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief  
  * @param  
  * @retval 
  */
void RS485_RxCpltCallback(void){
    TF_AcceptChar(&tfapp, rec);
	HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
* @brief : all data send from buffer ?
* @param : what  should one to say   ? well done,   
* @retval: well done, and there will be more..
*/
void RS485_TxCpltCallback(void){
}
/**
* @brief : usart error occured during transfer
* @param : clear error flags and reinit usaart
* @retval: and wait for address mark from master 
*/
void RS485_ErrorCallback(void){
    __HAL_UART_CLEAR_PEFLAG(&huart1);
    __HAL_UART_CLEAR_FEFLAG(&huart1);
    __HAL_UART_CLEAR_NEFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    __HAL_UART_CLEAR_OREFLAG(&huart1);
	__HAL_UART_FLUSH_DRREGISTER(&huart1);
	huart1.ErrorCode = HAL_UART_ERROR_NONE;
	HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
