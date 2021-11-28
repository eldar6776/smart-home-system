/**
 ******************************************************************************
 * File Name          : rs485.c
 * Date               : 28/02/2016 23:16:19
 * Description        : rs485 communication modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
#if (__RS485_H__ != FW_BUILD)
    #error "rs485 header version mismatch"
#endif 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dio.h"
#include "pwm.h"
#include "anin.h"
#include "rs485.h"
/* Imported Types  -----------------------------------------------------------*/
/* Imported Variables --------------------------------------------------------*/
/* Imported Functions    -----------------------------------------------------*/
/* Private Typedef -----------------------------------------------------------*/
static TinyFrame tfapp;
eComStateTypeDef eComState = COM_INIT;
SEND_CmdTypeDef SEND_Command = SEND_INIT;
DALI_CmdTypeDef DALI_Command = DALI_OFF;
/* Private Define  -----------------------------------------------------------*/
/* Private Variables  --------------------------------------------------------*/
uint16_t sysid;
uint8_t  tftype[16];
bool init_tf = false;
uint8_t  edin[sizeof(din)];
uint8_t  epwm[sizeof(pwm)];
uint16_t eain[sizeof(ain)];
uint32_t rstmr, rsflg, tfbps;
uint8_t  tfifa, tfgra, tfbra, tfgwa, rec;
/* Private macros   ----------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
TF_Result ID_Listener(TinyFrame *tf, TF_Msg *msg);
TF_Result GEN_Listener(TinyFrame *tf, TF_Msg *msg);
TF_Result TYPE_Listener(TinyFrame *tf, TF_Msg *msg);
/* Program Code  -------------------------------------------------------------*/
TF_Result ID_Listener(TinyFrame *tf, TF_Msg *msg){
    return TF_CLOSE;
}
TF_Result BINARY_Listener(TinyFrame *tf, TF_Msg *msg){  
    rel[msg->data[0]] = msg->data[1];
    TF_Respond(tf, msg);
    return TF_STAY;
}
TF_Result GEN_Listener(TinyFrame *tf, TF_Msg *msg){    
    return TF_STAY;
}
/**
  * @brief
  * @param
  * @retval
  */
void RS485_Init(void){
    if(!init_tf){
        init_tf = TF_InitStatic(&tfapp, TF_SLAVE);
        TF_AddGenericListener(&tfapp, GEN_Listener);
        TF_AddTypeListener(&tfapp, S_BINARY, BINARY_Listener);
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
void RS485_Service(void) {
    
    
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
 * This is an example of integrating TinyFrame into the application.
 * 
 * TF_WriteImpl() is required, the mutex functions are weak and can
 * be removed if not used. They are called from all TF_Send/Respond functions.
 * 
 * Also remember to periodically call TF_Tick() if you wish to use the 
 * listener timeout feature.
 */
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len){
    HAL_UART_AbortReceive_IT(&huart1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    if (HAL_UART_Transmit(&huart1,(uint8_t*)buff, len, RESP_TOUT) != HAL_OK) _Error_Handler(__FILE__, __LINE__);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}

/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    
    TF_AcceptChar(&tfapp, rec);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    __HAL_UART_CLEAR_PEFLAG(&huart1);
    __HAL_UART_CLEAR_FEFLAG(&huart1);
    __HAL_UART_CLEAR_NEFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    __HAL_UART_CLEAR_OREFLAG(&huart1);
	__HAL_UART_FLUSH_DRREGISTER(&huart1);
	huart->ErrorCode = HAL_UART_ERROR_NONE;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
