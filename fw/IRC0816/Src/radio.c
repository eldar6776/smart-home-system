/**
 ******************************************************************************
 * File Name          : radio.c
 * Date               : 04/01/2018 5:24:19
 * Description        : radio transciever processing
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dio.h"
#include "pwm.h"
#include "anin.h"
#include "radio.h"
#include "common.h"


/* Typedef -------------------------------------------------------------------*/
/* Define --------------------------------------------------------------------*/
const uint8_t hc12_at_baud[] = 	 {"AT+B"};
const uint8_t hc12_at_chanel[] = {"AT+C"};
const uint8_t hc12_at_transp[] = {"AT+FU"};
const uint8_t hc12_at_power[] =  {"AT+P"};
const uint8_t hc12_at_param[] =  {"AT+R"};
const uint8_t hc12_at_setdb[] =  {"AT+U"};
const uint8_t hc12_at_fwver[] =  {"AT+V"};
const uint8_t hc12_at_sleep[] =  {"AT+SLEEP"};
const uint8_t hc12_at_default[] ={"AT+DEFAULT"};


/* Variable ------------------------------------------------------------------*/
extern UART_HandleTypeDef huart1;

uint8_t usart_buffer[UART_BUFFER_SIZE];

volatile uint8_t receive_pcnt;
volatile uint8_t transfer_error;
volatile uint8_t received_byte_cnt;

volatile uint32_t radio_timer;
volatile uint32_t radio_flags;
volatile uint32_t radio_cycle_timer;
volatile uint32_t radio_link_timeout;

uint8_t radio_cycle_cnt;
uint8_t radio_error_cnt;
uint8_t radio_link_qt;

RADIO_ServiceStateTypeDef RADIO_ServiceState = RADIO_INIT;


/* Macro ---------------------------------------------------------------------*/
#define HC12_SetMode()		(HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET));
#define HC12_RxTxMode()		(HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET));


/* Program code   ------------------------------------------------------------*/
void RADIO_Service(void)
{	
	/** ==========================================================================*/
	/**    	R E C E I V E D		S T A T U S		P A C K E T  	S E R V I C E	  */
	/** ==========================================================================*/
	if(IsRADIO_RxStatusReady())
	{
		RADIO_RxStatusReset();
		relay_output[0] = usart_buffer[0];
		relay_output[1] = usart_buffer[1];
		pwm[0] = usart_buffer[2];
		pwm[1] = usart_buffer[3];
		pwm[2] = usart_buffer[4];
		pwm[3] = usart_buffer[5];
		pwm[4] = usart_buffer[6];
		pwm[5] = usart_buffer[7];
		pwm[6] = usart_buffer[8];
		pwm[7] = usart_buffer[9];
		pwm[8] = usart_buffer[10];
		pwm[9] = usart_buffer[11];
		pwm[10] = usart_buffer[12];
		pwm[11] = usart_buffer[13];
		pwm[12] = usart_buffer[14];
		pwm[13] = usart_buffer[15];
		pwm[14] = usart_buffer[16];
		pwm[15] = usart_buffer[17];
		ERROR_RadioLinkFailureReset();
		RADIO_ServiceState = RADIO_SEND_STATUS;
		RADIO_StartCycleTimer(RADIO_RX_TX_DELAY);
		RADIO_StartLinkTimeoutTimer(RADIO_LINK_TIMEOUT);
	}
			
	switch(RADIO_ServiceState)
	{
		/** ==========================================================================*/
		/**     	R A D I O      M O D U L E		I N I T I A L I Z A T I O N		  */
		/** ==========================================================================*/
		case RADIO_INIT:
		{
			HC12_RxTxMode();
			RADIO_StopTimer();
			RADIO_RxStatusReset();
			RADIO_TxStatusReset();
			receive_pcnt = 0U;
			received_byte_cnt = 0U;
			ClearBuffer(usart_buffer, UART_BUFFER_SIZE);
			/**
			*   clear uart rx busy flag if previous interrupt receiving
			*	function is disrupted before successfuly completed
			*/
			if (huart1.RxState == HAL_UART_STATE_BUSY_RX)
			{
				__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
				huart1.RxState = HAL_UART_STATE_READY;
				huart1.gState = HAL_UART_STATE_READY;
			}
			/**
			*   start usart receiving in interrupt mode
			*   to get packet header for address checking
			*/
			if(HAL_UART_Receive_IT(&huart1, usart_buffer, UART_BUFFER_SIZE) != HAL_OK)
			{
				_Error_Handler(__FILE__, __LINE__);				
			}
			
			RADIO_StartLinkTimeoutTimer(RADIO_LINK_TIMEOUT);
			RADIO_ServiceState = RADIO_RECEIVE_STATUS;
			break;
		}			
		/** ==========================================================================*/
		/**    	S E N D 	S T A T U S	 	T O 	C O M M A N D		U N I T 	  */
		/** ==========================================================================*/
		case RADIO_SEND_STATUS:
		{
			if(!IsRADIO_CycleTImerExpired()) break;
			/**
			*   clear uart rx busy flag if previous interrupt receiving
			*	function is disrupted before successfuly completed
			*/
			if (huart1.RxState == HAL_UART_STATE_BUSY_RX)
			{
				__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
				huart1.RxState = HAL_UART_STATE_READY;
				huart1.gState = HAL_UART_STATE_READY;
			}
			
			ClearBuffer(usart_buffer, UART_BUFFER_SIZE);
			usart_buffer[0] = SOH;
			usart_buffer[1] = din_state;
			usart_buffer[2] = error_signal_flags;
			usart_buffer[3] = CalcCRC(&usart_buffer[1], 2U);
			usart_buffer[4] = EOT;			
			HAL_UART_Transmit(&huart1, usart_buffer, 5U, 50U);
			while(HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY) continue;	
			RADIO_ServiceState = RADIO_INIT;
			break;
		}
		/** ==========================================================================*/
		/**    	S T A T U S		R E C E I V I N G	 T I M E O U T 		C H E C K 	  */
		/** ==========================================================================*/
		case RADIO_RECEIVE_STATUS:
		{			
			if(IsRADIO_LinkTimeoutTimerExpired())
			{
				ERROR_RadioLinkFailureSet();
				RADIO_ServiceState = RADIO_INIT;
			}
			break;
		}
		
		
		default:
		{
			RADIO_ServiceState = RADIO_INIT;
			break;
		}
	}
}


/******************************   END OF FILE  *******************************/
