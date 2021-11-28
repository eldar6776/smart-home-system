/**
 ******************************************************************************
 * File Name          : logger.c
 * Date               : 28/02/2016 23:16:19
 * Description        : data logger software modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/
#include "dio.h"
#include "min.h"
#include "pwm.h"
#include "main.h"
#include "anin.h"
#include "rs485.h"


#if (__LOGGER_H__ != FW_BUILD)
    #error "logger header version mismatch"
#endif


/* Imported Types  -----------------------------------------------------------*/
LOGGER_EventTypeDef LogEvent;
/* Imported Variables --------------------------------------------------------*/
/* Imported Functions    -----------------------------------------------------*/
/* Private Variables  --------------------------------------------------------*/
static uint16_t logger_next_log_address;
static uint16_t logger_next_log_id;
static uint16_t logger_list_count;
/* Private Macros    ---------------------------------------------------------*/
/* Private Prototypes    -----------------------------------------------------*/
/* Program code   ------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void LOGGER_Init(void)
{
    uint8_t log_buff[LOG_DSIZE];
    
    logger_list_count = 0U;
    logger_next_log_id = 1U;
    mem_set(&LogEvent, 0U, sizeof(LogEvent));
    log_buff[0] = (EE_LOG_START_ADD >> 8U);
    log_buff[1] = (EE_LOG_START_ADD & 0xFFU);
    logger_next_log_address = EE_LOG_START_ADD;
    if(HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT)              != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    if(HAL_I2C_Master_Transmit(&hi2c1, I2CEE_ADD, log_buff, 2U, DRV_TOUT)         != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    if(HAL_I2C_Master_Receive(&hi2c1, I2CEE_ADD, log_buff, LOG_DSIZE, DRV_TOUT)   != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    
    while(logger_next_log_address <= (EE_LOG_END_ADD - LOG_DSIZE))
    {	
        if((log_buff[0] == 0U) && (log_buff[1] == 0U)) break;
        ++logger_next_log_id;
        ++logger_list_count;
        logger_next_log_address += LOG_DSIZE;
        log_buff[0] = logger_next_log_address >> 8U;
        log_buff[1] = logger_next_log_address & 0xFFU;
        if(HAL_I2C_Master_Transmit(&hi2c1, I2CEE_ADD, log_buff, 2U, DRV_TOUT)       != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
        if(HAL_I2C_Master_Receive(&hi2c1, I2CEE_ADD, log_buff, LOG_DSIZE, DRV_TOUT) != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    }
    /**
    *	set log list not empty 	-> system status flag
    *	set log list full  		-> system status flag
    */
    if(logger_list_count != 0U) SYS_NewLogSet();
    if(logger_next_log_address > (EE_LOG_END_ADD - LOG_DSIZE)) SYS_LogListFullSet();
}
/**
  * @brief
  * @param
  * @retval
  */
LOGGER_StatusTypeDef LOGGER_Write(void)
{
    uint8_t log_buff[18];
    
	if(logger_next_log_address > (EE_LOG_END_ADD - LOG_DSIZE))
	{
		SYS_LogListFullSet();
		return (LOGGER_FULL);
	}
    log_buff[0] = logger_next_log_address >> 8;
    log_buff[1] = logger_next_log_address & 0xFFU;
    log_buff[2] = logger_next_log_id >> 8;
    log_buff[3] = logger_next_log_id & 0xFFU;
    log_buff[4] = LogEvent.log_event;
    log_buff[5] = LogEvent.log_type;
    log_buff[6] = LogEvent.log_group;
    log_buff[7] = LogEvent.log_card_id[0];
    log_buff[8] = LogEvent.log_card_id[1];
    log_buff[9] = LogEvent.log_card_id[2];
    log_buff[10]= LogEvent.log_card_id[3];
    log_buff[11]= LogEvent.log_card_id[4];
    log_buff[12]= rdate.Date;
    log_buff[13]= rdate.Month;
    log_buff[14]= rdate.Year;
    log_buff[15]= rtime.Hours;
    log_buff[16]= rtime.Minutes;
    log_buff[17]= rtime.Seconds;
    mem_set(&LogEvent, 0U, sizeof(LogEvent));
    if (HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT)       != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    if (HAL_I2C_Master_Transmit(&hi2c1, I2CEE_ADD, log_buff, 18U, DRV_TOUT) != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    if (HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT)       != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    logger_next_log_address += LOG_DSIZE;
    ++logger_list_count;
    ++logger_next_log_id;
    if (logger_next_log_address > (EE_LOG_END_ADD - LOG_DSIZE)) SYS_LogListFullSet();
    SYS_NewLogSet();
    return (LOGGER_OK);
}
/**
  * @brief
  * @param
  * @retval
  */
LOGGER_StatusTypeDef LOGGER_Read(uint8_t *buff)
{
    uint8_t log_buff[2];
    
	if (logger_list_count == 0U) return(LOGGER_EMPTY);
    log_buff[0] = ((logger_next_log_address - LOG_DSIZE) >> 8);
    log_buff[1] = ((logger_next_log_address - LOG_DSIZE) & 0xFFU);
    if (HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT)       != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    if (HAL_I2C_Master_Transmit(&hi2c1, I2CEE_ADD, log_buff, 2U, DRV_TOUT)  != HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    if (HAL_I2C_Master_Receive(&hi2c1, I2CEE_ADD, buff, LOG_DSIZE, DRV_TOUT)!= HAL_OK)  _Error_Handler(__FILE__, __LINE__);
    return(LOGGER_OK);
}
/**
  * @brief
  * @param
  * @retval
  */
LOGGER_StatusTypeDef LOGGER_Delete(void)
{
    uint8_t log_buff[18];
    
	if (logger_list_count == 0U) return(LOGGER_EMPTY);
    mem_set(log_buff, 0U, sizeof(log_buff));
    log_buff[0] = ((logger_next_log_address - LOG_DSIZE) >> 8U);
    log_buff[1] = ((logger_next_log_address - LOG_DSIZE) & 0xFFU);
    if (HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT)       != HAL_OK)  ErrorHandler(LOGGER_FUNC, I2C_DRV);
    if (HAL_I2C_Master_Transmit(&hi2c1, I2CEE_ADD, log_buff, 18U, DRV_TOUT) != HAL_OK)  ErrorHandler(LOGGER_FUNC, I2C_DRV);
    if (HAL_I2C_IsDeviceReady(&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT)       != HAL_OK)  ErrorHandler(LOGGER_FUNC, I2C_DRV);			
    logger_next_log_address -= LOG_DSIZE;
    --logger_list_count;
    if (logger_list_count == 0U) SYS_NewLogReset();
    SYS_LogListFullReset();
    return (LOGGER_OK);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
