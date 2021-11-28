/**
 ******************************************************************************
 * File Name          : barrier_controller.h
 * Date               : 15/05/2017 
 * Description        : barrier controller data link module header
 ******************************************************************************
 *
 *  RS485 DATA PACKET FORMAT
 *  ================================================================
 *  B0 = SOH                    - start of master to slave packet
 *  B0 = STX                    - start of slave to master packet
 *  B1 = ADDRESS MSB            - addressed unit high byte
 *  B2 = ADDRESS LSB            - addressed unit low byte
 *  B3 = ADDRESS MSB            - sender unit address high byte
 *  B4 = ADDRESS LSB            - sender unit address low byte
 *  B5 = MESSAGE LENGHT         - data lenght
 *  B6 = DATA [0]               - data first byte
 *  Bn = DATA [B5 + 5]          - data last byte
 *  Bn+1 = CRC MSB              - crc16 high byte
 *  Bn+2 = CRC LSB              - crc16 low byte
 *  Bn+3 = EOT                  - end of transmission
 ******************************************************************************
 */
#ifndef BARRIER_CONTROLLER_H
#define BARRIER_CONTROLLER_H   					200	// version

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "rtc.h"
#include "GUI.h"

/* Exported defines    -------------------------------------------------------*/

#define BARRIER_LOG_ENTRY						((uint8_t)0x90)
#define BARRIER_LOG_EXIT						((uint8_t)0x91)
#define BARRIER_LOG_ENTRY_REMOTE				((uint8_t)0x92)
#define BARRIER_LOG_ENTRY_REMOTE				((uint8_t)0x92)

/** ==========================================================================*/
/**                                                                           */
/**    R U B I C O N    L O G    L I S T     C O N S T A N T S     			  */
/**                                                                           */
/** ==========================================================================*/
#define BARRIER_PIN_RESET						((uint8_t)0xd0)
#define BARRIER_POWER_ON_RESET					((uint8_t)0xd1)
#define BARRIER_SOFTWARE_RESET					((uint8_t)0xd2)
#define BARRIER_IWDG_RESET						((uint8_t)0xd3)
#define BARRIER_WWDG_RESET						((uint8_t)0xd4)
#define BARRIER_LOW_POWER_RESET					((uint8_t)0xd5)
#define BARRIER_FIRMWARE_UPDATE					((uint8_t)0xd6)
#define BARRIER_LOG_NO_EVENT                	((uint8_t)0xe0)
#define BARRIER_LOG_GUEST_CARD_VALID        	((uint8_t)0xe1)
#define BARRIER_LOG_GUEST_CARD_INVALID      	((uint8_t)0xe2)
#define BARRIER_LOG_HANDMAID_CARD_VALID     	((uint8_t)0xe3)
#define BARRIER_LOG_ENTRY_DOOR_CLOSED			((uint8_t)0xe4)
#define BARRIER_LOG_PRESET_CARD					((uint8_t)0xe5)
#define BARRIER_LOG_HANDMAID_SERVICE_END    	((uint8_t)0xe6)
#define BARRIER_LOG_MANAGER_CARD            	((uint8_t)0xe7)
#define BARRIER_LOG_SERVICE_CARD            	((uint8_t)0xe8)
#define BARRIER_LOG_ENTRY_DOOR_OPENED          	((uint8_t)0xe9)
#define BARRIER_LOG_MINIBAR_USED            	((uint8_t)0xea)
#define BARRIER_LOG_BALCON_DOOR_OPENED			((uint8_t)0xeb)
#define BARRIER_LOG_BALCON_DOOR_CLOSED			((uint8_t)0xec)
#define BARRIER_LOG_CARD_STACKER_ON				((uint8_t)0xed)		
#define BARRIER_LOG_CARD_STACKER_OFF			((uint8_t)0xee)
#define BARRIER_LOG_DO_NOT_DISTURB_SWITCH_ON 	((uint8_t)0xef)
#define BARRIER_LOG_DO_NOT_DISTURB_SWITCH_OFF	((uint8_t)0xf0)
#define BARRIER_LOG_HANDMAID_SWITCH_ON			((uint8_t)0xf1)
#define BARRIER_LOG_HANDMAID_SWITCH_OFF			((uint8_t)0xf2)
#define BARRIER_LOG_SOS_ALARM_TRIGGER			((uint8_t)0xf3)
#define BARRIER_LOG_SOS_ALARM_RESET				((uint8_t)0xf4)
#define BARRIER_LOG_FIRE_ALARM_TRIGGER			((uint8_t)0xf5)
#define BARRIER_LOG_FIRE_ALARM_RESET          	((uint8_t)0xf6)
#define BARRIER_LOG_UNKNOWN_CARD				((uint8_t)0xf7)
#define BARRIER_LOG_CARD_EXPIRED				((uint8_t)0xf8)
#define BARRIER_LOG_WRONG_ROOM					((uint8_t)0xf9)
#define BARRIER_LOG_WRONG_SYSTEM_ID				((uint8_t)0xfa)
#define BARRIER_CONTROLLER_RESET				((uint8_t)0xfb)
#define BARRIER_ENTRY_DOOR_NOT_CLOSED			((uint8_t)0xfc)
#define	BARRIER_DOOR_BELL_ACTIVE				((uint8_t)0xfd)

/** ==========================================================================*/
/**                                                                           */
/**    R U B I C O N    R S 4 8 5   P R O T O C O L     C O N S T A N T S     */
/**                                                                           */
/** ==========================================================================*/
#define BARRIER_DEFAULT_INTERFACE_ADDRESS		((uint32_t)0x0005)
#define BARRIER_DEFFAULT_GROUP_ADDRESS			((uint32_t)0x6776)
#define BARRIER_DEFFAULT_BROADCAST_ADDRESS		((uint32_t)0x9999)

#define BARRIER_SOH                 			((uint8_t)0x01) 	/* start of command packet */
#define BARRIER_STX                  			((uint8_t)0x02) 	/* start of 1024-byte data packet */
#define BARRIER_EOT                       		((uint8_t)0x04) 	/* end of transmission */
#define BARRIER_ACK                    			((uint8_t)0x06) 	/* acknowledge */
#define BARRIER_NAK                     		((uint8_t)0x15) 	/* negative acknowledge */


/** ==========================================================================*/
/**                                                                           */
/**       R U B I C O N    R S  4 8 5   P A C K E T   F O R M A T        	  */
/**                                                                           */
/** ==========================================================================*/
/** 	
 *		command packet
 */
//		BARRIER_PACKET_START_IDENTIFIER
//		BARRIER_PACKET_RECEIVER_ADDRESS_MSB
//		BARRIER_PACKET_RECEIVER_ADDRESS_LSB	
//		BARRIER_PACKET_SENDER_ADDRESS_MSB
//		BARRIER_PACKET_SENDER_ADDRESS_LSB
//		BARRIER_PACKET_LENGHT						
//		BARRIER_PACKET_DATA		
//		BARRIER_PACKET_CHECKSUM_MSB	
//		BARRIER_PACKET_CHECKSUM_LSB	
//		BARRIER_PACKET_END_IDENTIFIER
/** 	
 *		data packet
 */
//		BARRIER_PACKET_START_IDENTIFIER
//		BARRIER_PACKET_RECEIVER_ADDRESS_MSB
//		BARRIER_PACKET_RECEIVER_ADDRESS_LSB	
//		BARRIER_PACKET_SENDER_ADDRESS_MSB
//		BARRIER_PACKET_SENDER_ADDRESS_LSB
//		BARRIER_PACKET_LENGHT						
//		BARRIER_PACKET_NUMBER_MSB
//		BARRIER_PACKET_NUMBER_LSB
//		BARRIER_PACKET_CHECKSUM_MSB	
//		BARRIER_PACKET_CHECKSUM_LSB	
//		BARRIER_PACKET_END_IDENTIFIER


/** ==========================================================================*/
/**                                                                           */
/**    	R U B I C O N    R S 4 8 5	C O M M A N D		L I S T           	  */
/**                                                                           */
/** ==========================================================================*/
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_1 		((uint8_t)0xce)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_2 		((uint8_t)0xcd)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_3		((uint8_t)0xcc)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_4		((uint8_t)0xcb)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_5		((uint8_t)0xca)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_6		((uint8_t)0xc9)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_7		((uint8_t)0xc8)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_8		((uint8_t)0xc7)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_9		((uint8_t)0xc6)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_10		((uint8_t)0xc5)
#define BARRIER_DOWNLOAD_SMALL_FONTS			((uint8_t)0xc4)
#define BARRIER_DOWNLOAD_MIDDLE_FONTS			((uint8_t)0xc3)
#define BARRIER_DOWNLOAD_BIG_FONTS				((uint8_t)0xc2)
#define BARRIER_DOWNLOAD_TEXT_DATE_TIME			((uint8_t)0xc1)
#define BARRIER_DOWNLOAD_TEXT_EVENTS 			((uint8_t)0xc0)
#define BARRIER_DOWNLOAD_FIRMWARE           	((uint8_t)0xbf)
#define BARRIER_FLASH_PROTECTION_ENABLE			((uint8_t)0xbe)
#define BARRIER_FLASH_PROTECTION_DISABLE		((uint8_t)0xbd)
#define BARRIER_START_BOOTLOADER				((uint8_t)0xbc)
#define BARRIER_EXECUTE_APPLICATION				((uint8_t)0xbb)
#define BARRIER_GET_SYS_STATUS					((uint8_t)0xba)
#define BARRIER_GET_SYS_INFO					((uint8_t)0xb9)
#define BARRIER_GET_DISPLAY_BRIGHTNESS			((uint8_t)0xb8)
#define BARRIER_SET_DISPLAY_BRIGHTNESS			((uint8_t)0xb7)
#define BARRIER_GET_RTC_DATE_TIME				((uint8_t)0xb6)
#define BARRIER_SET_RTC_DATE_TIME				((uint8_t)0xb5)
#define BARRIER_GET_LOG_LIST 					((uint8_t)0xb4)
#define BARRIER_DELETE_LOG_LIST 				((uint8_t)0xb3)
#define BARRIER_GET_RS485_CONFIG				((uint8_t)0xb2)
#define BARRIER_SET_RS485_CONFIG				((uint8_t)0xb1)
#define BARRIER_GET_DIN_STATE					((uint8_t)0xb0)
#define BARRIER_SET_DOUT_STATE					((uint8_t)0xaf)
#define BARRIER_GET_DOUT_STATE					((uint8_t)0xae)
#define BARRIER_GET_PCB_TEMPERATURE				((uint8_t)0xad)
#define BARRIER_GET_TEMP_CARD_BUFFER			((uint8_t)0xac)
#define BARRIER_GET_MIFARE_AUTHENTICATION_KEY_A	((uint8_t)0xab)
#define BARRIER_SET_MIFARE_AUTHENTICATION_KEY_A	((uint8_t)0xaa)
#define BARRIER_GET_MIFARE_AUTHENTICATION_KEY_B	((uint8_t)0xa9)
#define BARRIER_SET_MIFARE_AUTHENTICATION_KEY_B	((uint8_t)0xa8)
#define BARRIER_GET_MIFARE_PERMITED_GROUP		((uint8_t)0xa7)
#define BARRIER_SET_MIFARE_PERMITED_GROUP 		((uint8_t)0xa6)
#define BARRIER_GET_MIFARE_PERMITED_CARD_1		((uint8_t)0xa5)
#define BARRIER_SET_MIFARE_PERMITED_CARD_1		((uint8_t)0xa4)
#define BARRIER_GET_MIFARE_PERMITED_CARD_2		((uint8_t)0xa3)
#define BARRIER_SET_MIFARE_PERMITED_CARD_2		((uint8_t)0xa2)
#define BARRIER_GET_MIFARE_PERMITED_CARD_3		((uint8_t)0xa1)
#define BARRIER_SET_MIFARE_PERMITED_CARD_3		((uint8_t)0xa0)
#define BARRIER_GET_MIFARE_PERMITED_CARD_4		((uint8_t)0x9f)
#define BARRIER_SET_MIFARE_PERMITED_CARD_4		((uint8_t)0x9e)
#define BARRIER_GET_MIFARE_PERMITED_CARD_5		((uint8_t)0x9d)
#define BARRIER_SET_MIFARE_PERMITED_CARD_5		((uint8_t)0x9c)
#define BARRIER_GET_MIFARE_PERMITED_CARD_6		((uint8_t)0x9b)
#define BARRIER_SET_MIFARE_PERMITED_CARD_6		((uint8_t)0x9a)
#define BARRIER_GET_MIFARE_PERMITED_CARD_7		((uint8_t)0x99)
#define BARRIER_SET_MIFARE_PERMITED_CARD_7		((uint8_t)0x98)
#define BARRIER_GET_MIFARE_PERMITED_CARD_8		((uint8_t)0x97)
#define BARRIER_SET_MIFARE_PERMITED_CARD_8		((uint8_t)0x96)
#define BARRIER_GET_ROOM_STATUS					((uint8_t)0x95)
#define BARRIER_SET_ROOM_STATUS					((uint8_t)0x94)
#define BARRIER_RESET_SOS_ALARM					((uint8_t)0x93)
#define BARRIER_GET_ROOM_TEMPERATURE			((uint8_t)0x92)
#define BARRIER_SET_ROOM_TEMPERATURE			((uint8_t)0x91)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_11		((uint8_t)0x90)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_12		((uint8_t)0x8f)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_13		((uint8_t)0x8e)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_14		((uint8_t)0x8d)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_15		((uint8_t)0x8c)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_16		((uint8_t)0x8b)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_17		((uint8_t)0x8a)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_18		((uint8_t)0x89)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_19		((uint8_t)0x88)
#define BARRIER_DOWNLOAD_DISPLAY_IMAGE_20		((uint8_t)0x87)
#define BARRIER_RESET_COUNTERS					((uint8_t)0x86)
#define BARRIER_GET_COUNTERS					((uint8_t)0x85)

#define BARRIER_SET_MIFARE_PERMITED_CARD		((uint8_t)0x71)
#define BARRIER_DOWNLOAD_DIPLAY_IMAGE			((uint8_t)0x70)
#define BARRIER_DOWNLOAD_ALL_IMAGE				((uint8_t)0x6f)
#define BARRIER_UPDATE_FROM_CONFIG_FILE			((uint8_t)0x6e)


#define RS485_INTERFACE_DEFAULT_ADDRESS			0x0005
#define BARRIER_TIME_UPDATE_PERIOD				6789
#define BARRIER_RESTART_TIME					7890
#define MSG_DISPL_TIME							567
#define BARRIER_BUFFER_SIZE						512
#define BARRIER_PACKET_BUFFER_SIZE				64
#define BARRIER_BOOTLOADER_START_TIME			3456
#define BARRIER_FW_UPLOAD_TIMEOUT				2345
#define BARRIER_FW_EXE_BOOT_TIME				1567
#define BARRIER_FILE_UPLOAD_TIMEOUT				321
#define BARRIER_LOG_SIZE						16
#define BARRIER_RESPONSE_TIMEOUT				78	
#define BARRIER_BYTE_RX_TIMEOUT					3	
#define BARRIER_RX_TO_TX_DELAY					10
#define BARRIER_MAX_ERRORS          			10
#define BARRIER_HTTP_RESPONSE_TIMEOUT			189
#define BARRIER_CONFIG_FILE_MAX_SIZE			(BARRIER_BUFFER_SIZE - 16)
#define BARRIER_CONFIG_FILE_BUFFER_SIZE			64
#define BARRIER_CONFIG_FILE_TAG_LENGHT			5


/** ==========================================================================*/
/**                                                                           */
/**    	C O N T R O L L E R 	C O M M A N D		L I S T            		  */
/**                                                                           */
/** ==========================================================================*/

#define HTTP_LOG_TRANSFER_IDLE					40
#define HTTP_GET_LOG_LIST						41
#define HTTP_LOG_LIST_READY						42
#define HTTP_DELETE_LOG_LIST					43
#define HTTP_LOG_LIST_DELETED					44
#define HTTP_LOG_TRANSFER_FAIL					45
#define HTTP_GET_BARRIER_TEMPERATURE			46
#define HTTP_BARRIER_TEMPERATURE_READY			47
#define HTTP_SET_BARRIER_TEMPERATURE			48
#define HTTP_BARRIER_TEMPERATURE_FAIL			49
#define HTTP_GET_BARRIER_STATUS					50
#define HTTP_BARRIER_STATUS_READY				51
#define HTTP_SET_BARRIER_STATUS					52
#define HTTP_BARRIER_STATUS_FAIL				53
#define HTTP_GET_BARRIER_SYS_INFO				54
#define HTTP_BARRIER_SYS_INFO_READY				55
#define HTTP_BARRIER_SYS_INFO_FAIL				56
#define HTTP_GET_BARRIER_COUNTERS				57
#define HTTP_BARRIER_COUNTERS_READY				58
#define HTTP_RESET_BARRIER_COUNTERS				59
#define HTTP_BARRIER_COUNTERS_FAIL				60

#define FILE_OK									1
#define FILE_SYS_ERROR							2	
#define FILE_DIR_ERROR							3
#define FILE_ERROR								4
#define MAX_QUERY_ATTEMPTS						5

#define FW_UPDATE_IDLE							16
#define FW_UPDATE_INIT 							17
#define FW_UPDATE_BOOTLOADER 					18
#define FW_UPDATE_RUN							19
#define FW_UPDATE_FINISHED						20
#define FW_UPDATE_FAIL							21
#define FW_UPDATE_FROM_CONFIG_FILE				22

#define LOG_TRANSFER_IDLE						30
#define LOG_TRANSFER_QUERY_LIST					31
#define LOG_TRANSFER_DELETE_LOG					32
#define LOG_TRANSFER_FAIL						33

#define FILE_UPDATE_IDLE						40
#define FILE_UPDATE_IMAGE_1 					41
#define FILE_UPDATE_IMAGE_2 					42
#define FILE_UPDATE_IMAGE_3 					43
#define FILE_UPDATE_IMAGE_4 					44
#define FILE_UPDATE_IMAGE_5 					45
#define FILE_UPDATE_IMAGE_6 					46
#define FILE_UPDATE_IMAGE_7 					47
#define FILE_UPDATE_IMAGE_8 					48
#define FILE_UPDATE_IMAGE_9 					49
#define FILE_UPDATE_IMAGE_10 					50
#define FILE_UPDATE_IMAGE_11 					51
#define FILE_UPDATE_IMAGE_12 					52
#define FILE_UPDATE_IMAGE_13 					53
#define FILE_UPDATE_IMAGE_14 					54
#define FILE_UPDATE_IMAGE_15 					55
#define FILE_UPDATE_IMAGE_16 					56
#define FILE_UPDATE_IMAGE_17 					57
#define FILE_UPDATE_IMAGE_18 					58
#define FILE_UPDATE_IMAGE_19 					59
#define FILE_UPDATE_RUN							60
#define FILE_UPDATE_FINISHED					61
#define FILE_UPDATE_FAIL						62
#define FILE_UPDATE_FROM_CONFIG_FILE			63

#define RS485_SCANNER_FIND_FIRST				0
#define RS485_SCANNER_FIND_NEXT					1
#define RS485_SCANNER_FIND_NEW					2
#define RS485_SCANNER_FIND_ALL					3
#define RS485_SCANNER_FIND_ADDRESSED			4

/* Exported types    ---------------------------------------------------------*/
typedef enum 
{
    BARRIER_INIT 					= 0x00,
    BARRIER_PACKET_ENUMERATOR 		= 0x01,
    BARRIER_PACKET_SEND 			= 0x02,
    BARRIER_PACKET_PENDING 			= 0x03,
	BARRIER_PACKET_RECEIVING		= 0x04,
    BARRIER_PACKET_RECEIVED 		= 0x05,
    BARRIER_PACKET_ERROR 			= 0x06

} eBarrierStateTypeDef;

typedef enum 
{
    BARRIER_UPDATE_INIT 			= 0x00,
    BARRIER_UPDATE_TIME 			= 0x01,
    BARRIER_UPDATE_STATUS 			= 0x02,
    BARRIER_UPDATE_FIRMWARE 		= 0x03,
    BARRIER_UPDATE_FILE 			= 0x04,
    BARRIER_UPDATE_LOG 				= 0x05,
    BARRIER_HTTP_REQUEST 			= 0x06,
    BARRIER_NO_UPDATE 				= 0x07

} eBarrierUpdateTypeDef;

typedef enum 
{
    BARRIER_TIME_UPDATE_INIT 		= 0x00,
    BARRIER_TIME_UPDATE_P2P 		= 0x01,
    BARRIER_TIME_UPDATE_GROUP		= 0x02,
    BARRIER_TIME_UPDATE_BROADCAST	= 0x03,
    BARRIER_NO_TIME_UPDATE 			= 0x04,

} eBarrierTimeUpdateTypeDef;

typedef enum
{
	LOG_LIST_UNDEFINED	= 0x00,
	LOG_LIST_TYPE_1		= 0x01,
	LOG_LIST_TYPE_2		= 0x02,
	LOG_LIST_TYPE_3		= 0x03,
	LOG_LIST_TYPE_4		= 0x04,
	LOG_LIST_TYPE_5		= 0x05,
	LOG_LIST_TYPE_6		= 0x06
	
} LOG_MemoryFragmentTypeDef;

typedef struct 
{
	LOG_MemoryFragmentTypeDef LOG_MemoryFragment;
    uint16_t log_list_cnt;
	uint32_t first_log_address;
    uint32_t last_log_address;
	uint32_t next_log_address;
	
} BARRIER_LogMemoryTypeDef;

typedef struct
{
    uint8_t log_transfer_state;
    uint8_t last_attempt;
    uint8_t send_attempt;
	uint32_t log_transfer_end_address;
	
} BARRIER_LogListTransferTypeDef;

typedef struct
{
    uint8_t update_state;
    uint8_t send_attempt;
    uint32_t packet_total;
    uint32_t packet_send;
    uint32_t last_packet_send;
    uint16_t file_data_read;

} BARRIER_UpdatePacketTypeDef;

typedef struct
{
	uint16_t log_id;
	uint8_t log_event;
	uint8_t log_cycle;
	uint8_t log_group;
	uint8_t log_card_id[5];
	uint8_t log_time_stamp[6];
	
} BARRIER_LogEventTypeDef;

/* Exported variables  -------------------------------------------------------*/
extern volatile uint32_t barrier_timer;
extern volatile uint32_t barrier_flags;
extern volatile uint32_t barrier_display_timer;
extern volatile uint32_t barrier_rx_timer;
extern volatile uint32_t barrier_fw_update_timer;
extern volatile uint32_t barrier_tftp_file;
extern volatile uint32_t barrier_response_timer;
extern uint16_t rs485_barrier_address;
extern uint16_t rs485_interface_address;
extern uint16_t rs485_broadcast_address;
extern uint16_t rs485_group_address;
extern uint8_t aWorkingBuffer[BARRIER_BUFFER_SIZE];
extern uint8_t *p_barrier_buffer;
extern uint8_t barrier_ctrl_request;
extern uint8_t barrier_http_cmd_state;

extern eBarrierStateTypeDef eBarrierTransferState;
extern eBarrierUpdateTypeDef eBarrierUpdate;
extern BARRIER_LogMemoryTypeDef BARRIER_LogMemory;
extern BARRIER_LogListTransferTypeDef HTTP_LogListTransfer;
extern BARRIER_LogEventTypeDef BARRIER_LogEvent;

/* Exported macros     -------------------------------------------------------*/
#define BARRIER_StartTimer(TIME)			((barrier_timer = TIME), (barrier_flags &= 0xfffffffe))
#define BARRIER_StopTimer()					(barrier_flags |= 0x00000001)
#define IsBARRIER_TimerExpired()			(barrier_flags & 0x00000001)	
#define BARRIER_StartDisplayTimer()			((barrier_display_timer = MSG_DISPL_TIME),(barrier_flags &= 0xfffffffd)) 
#define BARRIER_StopDisplayTimer()			(barrier_flags |= 0x00000002)
#define IsBARRIER_DisplayTimerExpired()		(barrier_flags & 0x00000002)	
#define BARRIER_StartRxTimeoutTimer(TIME)	((barrier_rx_timer = TIME),(barrier_flags &= 0xfffffffb)) 
#define BARRIER_StopRxTimeoutTimer()		(barrier_flags |= 0x00000004)
#define IsBARRIER_RxTimeoutTimerExpired()	(barrier_flags & 0x00000004)
#define BARRIER_StartFwUpdateTimer(TIME)	((barrier_fw_update_timer = TIME),(barrier_flags &= 0xfffffff7)) 
#define BARRIER_StopFwUpdateTimer()			(barrier_flags |= 0x00000008)
#define IsBARRIER_FwUpdateTimerExpired()	(barrier_flags & 0x00000008)
#define BARRIER_StartResponseTimer(TIME)	((barrier_response_timer = TIME),(barrier_flags &= 0xffffffef)) 
#define BARRIER_StopResponseTimer()			(barrier_flags |= 0x00000010)
#define IsBARRIER_ResponseTimerExpired()	(barrier_flags & 0x00000010)

/* Exported functions  -------------------------------------------------------*/
void BARRIER_Init(void);
void BARRIER_Service(void);
void BARRIER_PrepareTimeUpdatePacket(void);
void BARRIER_PrepareStatusUpdatePacket(void);
void BARRIER_PrepareFirmwareUpdatePacket(void);
void BARRIER_PrepareFileUpdatePacket(void);
void BARRIER_PrepareLogUpdatePacket(void);
void BARRIER_PrepareCommandPacket(uint8_t command, uint8_t *ibuff);
void BARRIER_WriteLogToList(void);
void BARRIER_DeleteBlockFromLogList(void);
void BARRIER_ReadBlockFromLogList(void);
void BARRIER_IncreaseCounter(void);
void BARRIER_ReadCounters(void);
void BARRIER_ResetCounters(void);
uint8_t BARRIER_CheckNewFirmwareFile(void);
uint8_t BARRIER_CheckNewImageFile(void);
uint16_t BARRIER_GetNextAddress(void);
uint16_t BARRIER_GetGroupAddress(uint16_t group);
uint16_t BARRIER_GetBroadcastAddress(void);
uint8_t BARRIER_CheckConfigFile(void);
uint8_t BARRIER_CreateUpdateAddresseList(void);

int BARRIER_ScanRS485_Bus(uint16_t start_address, uint16_t end_address, uint8_t option);
#endif
/******************************   END OF FILE  **********************************/

