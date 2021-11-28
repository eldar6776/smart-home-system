/**
 ******************************************************************************
 * File Name          : barrier_controller.c
 * Date               : 21/08/2016 20:59:16
 * Description        : hotel room Barrier controller data link modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "barrier_controller.h"
#include "barrier_address_list.h"
#include "stm32f429i_lcd.h"
#include "common.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "i2c_eeprom.h"
#include "buzzer.h"
#include "spi_flash.h"
#include "Display.h"
#include "stepper.h"
#include "rc522.h"


/* Private typedef -----------------------------------------------------------*/
eBarrierStateTypeDef eBarrierTransferState = BARRIER_INIT;
eBarrierTimeUpdateTypeDef eBarrierTimeUpdate = BARRIER_TIME_UPDATE_INIT;
eBarrierUpdateTypeDef eBarrierUpdate = BARRIER_UPDATE_INIT;

uint16_t rs485_barrier_address;
uint16_t rs485_interface_address;
uint16_t rs485_broadcast_address;
uint16_t rs485_group_address;

uint32_t tmp_ONE_TIME_USER_ENTRY_COUNTER;
uint32_t tmp_ONE_TIME_USER_EXIT_COUNTER;
uint32_t tmp_PREPAID_USER_ENTRY_COUNTER;
uint32_t tmp_PREPAID_USER_EXIT_COUNTER;
uint32_t tmp_CARD_USER_ENTRY_COUNTER;
uint32_t tmp_CARD_USER_EXIT_COUNTER;
uint32_t tmp_WRIST_USER_ENTRY_COUNTER;
uint32_t tmp_WRIST_USER_EXIT_COUNTER;
uint32_t tmp_KEY_RING_USER_ENTRY_COUNTER;
uint32_t tmp_KEY_RING_USER_EXIT_COUNTER;
uint32_t tmp_TOTAL_USER_ENTRY_COUNTER;
uint32_t tmp_TOTAL_USER_EXIT_COUNTER;
uint32_t counter;

/* Private define ------------------------------------------------------------*/
BARRIER_UpdatePacketTypeDef BARRIER_FileUpdatePacket;
BARRIER_UpdatePacketTypeDef BARRIER_FirmwareUpdatePacket;
BARRIER_LogListTransferTypeDef BARRIER_LogListTransfer;
BARRIER_LogListTransferTypeDef HTTP_LogListTransfer;
BARRIER_LogMemoryTypeDef BARRIER_LogMemory;
BARRIER_LogEventTypeDef BARRIER_LogEvent;


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t barrier_display_timer;
volatile uint32_t barrier_timer;
volatile uint32_t barrier_flags;
volatile uint32_t barrier_rx_timer;
volatile uint32_t barrier_fw_update_timer;
volatile uint32_t barrier_tftp_file;
volatile uint32_t barrier_response_timer;

uint16_t rs485_packet_checksum;
uint16_t rs485_packet_lenght;
uint16_t barrier_address_list_cnt;
uint8_t aWorkingBuffer[BARRIER_BUFFER_SIZE];
uint8_t *p_barrier_buffer;
uint8_t config_file_buffer[BARRIER_CONFIG_FILE_BUFFER_SIZE];
uint8_t config_file_image_cnt;
uint32_t config_file_byte_cnt;
uint8_t barrier_ctrl_request;
uint8_t barrier_http_cmd_state;

extern FATFS filesystem;
extern FIL file_SD, file_CR;
extern DIR dir_1, dir_2;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static void delay(__IO uint32_t nCount)
{
    __IO uint32_t index = 0;

    for (index = 80000 * nCount; index != 0; index--)
    {
    }
}

void BARRIER_Init(void)
{
	uint32_t temp_log_list_scan;
	
	rs485_interface_address = RS485_INTERFACE_DEFAULT_ADDRESS;
	rs485_broadcast_address = BARRIER_DEFFAULT_BROADCAST_ADDRESS;
	rs485_group_address = BARRIER_DEFFAULT_GROUP_ADDRESS;
	
	RS485ModeGpio_Init();
	RS485_MODE(RS485_RX);
	
	p_comm_buffer = rx_buffer;
	while (p_comm_buffer < rx_buffer + sizeof (rx_buffer)) *p_comm_buffer++ = NULL;
	
	p_comm_buffer = tx_buffer;
	while (p_comm_buffer < tx_buffer + sizeof (tx_buffer)) *p_comm_buffer++ = NULL;
	
	p_barrier_buffer = aWorkingBuffer;
	while (p_barrier_buffer < aWorkingBuffer + sizeof(aWorkingBuffer)) *p_barrier_buffer++ = NULL;

	p_i2c_ee_buffer = aEepromBuffer;
	while (p_i2c_ee_buffer < aEepromBuffer + sizeof(aEepromBuffer)) *p_i2c_ee_buffer++ = NULL;
	
	barrier_address_list_cnt = 0;
	rs485_interface_address = RS485_INTERFACE_DEFAULT_ADDRESS;
	
	eBarrierTransferState = BARRIER_PACKET_ENUMERATOR;
	eBarrierUpdate = BARRIER_NO_UPDATE;
	eBarrierTimeUpdate = BARRIER_TIME_UPDATE_BROADCAST;
	
	BARRIER_FirmwareUpdatePacket.update_state = FILE_UPDATE_IDLE;
	BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IDLE;
	
	BARRIER_FirmwareUpdatePacket.update_state = FW_UPDATE_IDLE;
	BARRIER_FirmwareUpdatePacket.file_data_read = 0;
	BARRIER_FirmwareUpdatePacket.last_packet_send = 0;
	BARRIER_FirmwareUpdatePacket.packet_send = 0;
	BARRIER_FirmwareUpdatePacket.packet_total = 0;
	BARRIER_FirmwareUpdatePacket.send_attempt = 0;
	
	BARRIER_LogListTransfer.log_transfer_state = LOG_TRANSFER_IDLE;
	BARRIER_LogListTransfer.last_attempt = 0;
	BARRIER_LogListTransfer.send_attempt = 0;
	BARRIER_LogListTransfer.log_transfer_end_address = 0;
	
	BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_UNDEFINED;
	BARRIER_LogMemory.log_list_cnt = 0;
	BARRIER_LogMemory.first_log_address = 0;	
	BARRIER_LogMemory.last_log_address = 0;
	BARRIER_LogMemory.next_log_address = 0;
	
	/**
	*	LOG_LIST_TYPE_1 -> log list is empty and next log address is first address
	*	0000000000000000000000000000000000000000000000000000000000000000000000000
	*
	*	LOG_LIST_TYPE_2 -> log list start at some addres, it's full till last address, next log address is first address and is free for write 
	*	000000000000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	*
	* 	LOG_LIST_TYPE_3 -> log list start at some addres, end at upper address, next log address is upper address and is free for write
	*	000000000000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxx000000000000000000000000000
	*
	*	LOG_LIST_TYPE_4 -> log list start at first address, end at last address, it's full, next log address is first memory address, write is forbiden
	*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	*	
	*	LOG_LIST_TYPE_5 -> log list start at first address, end at upper address, and next upper log address is free for write
	*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx00000000000000000000000000000000000000000
	*
	*	LOG_LIST_TYPE_6 -> log list start at upper address, end at lower address and next upper from end address is free for write
	*	xxxxxxxxxxxx0000000000000000000000000000000000000000000000xxxxxxxxxxxxxxx
	*/	
	
	temp_log_list_scan = EE_LOG_LIST_START_ADDRESS;
	I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
	
	/* CHECK FOR LOG_LIST_TYPE_1 */
	if((aEepromBuffer[0] == NULL) && (aEepromBuffer[1] == NULL)) 
	{
		BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_1;
		BARRIER_LogMemory.first_log_address = EE_LOG_LIST_START_ADDRESS;
		BARRIER_LogMemory.last_log_address = EE_LOG_LIST_START_ADDRESS;
		BARRIER_LogMemory.next_log_address = EE_LOG_LIST_START_ADDRESS;
		
		temp_log_list_scan += BARRIER_LOG_SIZE;
		I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
		
		/* CHECK FOR LOG_LIST_TYPE_2 */
		while(temp_log_list_scan <= (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE))
		{
			if((aEepromBuffer[0] != NULL) || (aEepromBuffer[1] != NULL))
			{
				BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_2;
				BARRIER_LogMemory.first_log_address = temp_log_list_scan;
				BARRIER_LogMemory.last_log_address = (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE);
				++BARRIER_LogMemory.log_list_cnt;
				break;
			}
			else
			{
				temp_log_list_scan += BARRIER_LOG_SIZE;
				
				if(temp_log_list_scan < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
				}
				else
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, temp_log_list_scan, BARRIER_LOG_SIZE);
				}
			}
		}
		/* CHECK FOR LOG_LIST_TYPE_3 */
		if(BARRIER_LogMemory.LOG_MemoryFragment == LOG_LIST_TYPE_2)
		{
			temp_log_list_scan += BARRIER_LOG_SIZE;
			
			if(temp_log_list_scan <= (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE))
			{
				if(temp_log_list_scan < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
				}
				else
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, temp_log_list_scan, BARRIER_LOG_SIZE);
				}
				
				while(temp_log_list_scan <= (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE))
				{	
					if((aEepromBuffer[0] == NULL) && (aEepromBuffer[1] == NULL))
					{
						BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_3;
						BARRIER_LogMemory.last_log_address = temp_log_list_scan - BARRIER_LOG_SIZE;
						BARRIER_LogMemory.next_log_address = temp_log_list_scan;
						break;
					}
					else
					{	
						temp_log_list_scan += BARRIER_LOG_SIZE;
						++BARRIER_LogMemory.log_list_cnt;
						
						if(temp_log_list_scan < I2C_EE_PAGE_SIZE)
						{
							I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
						}
						else
						{
							I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, temp_log_list_scan, BARRIER_LOG_SIZE);
						}
					}					
				}
			}
		}
	}
	/* CHECK FOR LOG_LIST_TYPE_4 */
	else if((aEepromBuffer[0] != NULL) || (aEepromBuffer[1] != NULL))
	{
		BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_4;
		BARRIER_LogMemory.first_log_address = EE_LOG_LIST_START_ADDRESS;
		BARRIER_LogMemory.last_log_address = EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE;
		BARRIER_LogMemory.next_log_address = EE_LOG_LIST_START_ADDRESS;
		++BARRIER_LogMemory.log_list_cnt;
		
		temp_log_list_scan += BARRIER_LOG_SIZE;
		I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
		
		/* CHECK FOR LOG_LIST_TYPE_5 */
		while(temp_log_list_scan <= (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE))
		{	
			if((aEepromBuffer[0] == NULL) && (aEepromBuffer[1] == NULL))
			{
				BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_5;
				BARRIER_LogMemory.last_log_address = temp_log_list_scan - BARRIER_LOG_SIZE;
				BARRIER_LogMemory.next_log_address = temp_log_list_scan;
				break;
			}
			else
			{
				temp_log_list_scan += BARRIER_LOG_SIZE;
				++BARRIER_LogMemory.log_list_cnt;

				if(temp_log_list_scan < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
				}
				else
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, temp_log_list_scan, BARRIER_LOG_SIZE);
				}
			}
		}
		/* CHECK FOR LOG_LIST_TYPE_6 */
		if(BARRIER_LogMemory.LOG_MemoryFragment == LOG_LIST_TYPE_5)
		{
			temp_log_list_scan += BARRIER_LOG_SIZE;
			
			if(temp_log_list_scan <= (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE))
			{
				if(temp_log_list_scan < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
				}
				else
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, temp_log_list_scan, BARRIER_LOG_SIZE);
				}
				
				while(temp_log_list_scan <= (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE))
				{	
					if((aEepromBuffer[0] != NULL) || (aEepromBuffer[1] != NULL))
					{
						BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_6;
						BARRIER_LogMemory.first_log_address = temp_log_list_scan;
						BARRIER_LogMemory.log_list_cnt += ((EE_LOG_LIST_END_ADDRESS - temp_log_list_scan) / BARRIER_LOG_SIZE);
						break;
					}
					else
					{	
						temp_log_list_scan += BARRIER_LOG_SIZE;
						
						if(temp_log_list_scan < I2C_EE_PAGE_SIZE)
						{
							I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, temp_log_list_scan, BARRIER_LOG_SIZE);
						}
						else
						{
							I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, temp_log_list_scan, BARRIER_LOG_SIZE);
						}
					}					
				}
			}
		}
	}
}

void BARRIER_Service(void)
{
  
}

uint16_t BARRIER_GetNextAddress(void)
{
    uint16_t current_address;


    current_address = barrier_address_list[barrier_address_list_cnt];

    ++barrier_address_list_cnt;

    if (barrier_address_list[barrier_address_list_cnt] == 0x0000)
    {
        barrier_address_list_cnt = 0;
    }

    return current_address;
}

uint16_t BARRIER_GetGroupAddress(uint16_t group)
{
    return 0x6776;
}

uint16_t BARRIER_GetBroadcastAddress(void)
{
    return 0x9999;
}

int BARRIER_ScanRS485_Bus(uint16_t start_address, uint16_t end_address, uint8_t option)
{
	int new_fnd;
	static uint16_t address_offset;
	uint8_t scn_pcnt;
	uint8_t tmp_j;
	uint16_t tmp_address, rx_chksm;
	
	static enum
	{
		SCAN_INIT 		= 0x00,
		SCAN_SEND 		= 0x01,
		SCAN_PENDING	= 0x02,
		SCAN_RECEIVE	= 0x03,
		SCAN_SETUP		= 0x04
		
	}eBusScaningState;
	
	if(option == RS485_SCANNER_FIND_NEXT) eBusScaningState = SCAN_SETUP;
	else eBusScaningState = SCAN_INIT;
	
	rs485_rx_cnt = 0;
	scn_pcnt = 0;
	
	while(scn_pcnt == 0)
	{
		switch(eBusScaningState)
		{
			case SCAN_INIT:
			{
				if ((start_address <= RS485_INTERFACE_DEFAULT_ADDRESS) || (start_address >= end_address)) return (-1);
				p_comm_buffer = rx_buffer;
				while(p_comm_buffer < rx_buffer + sizeof(rx_buffer)) *p_comm_buffer++ = NULL;
				p_comm_buffer = tx_buffer;
				while(p_comm_buffer < tx_buffer + sizeof(tx_buffer)) *p_comm_buffer++ = NULL;
				new_fnd = 0;
				address_offset = 0;
				eBusScaningState = SCAN_SETUP;
				break;	
			}
			
			
			case SCAN_SEND:
			{
				if(!IsBARRIER_RxTimeoutTimerExpired()) break;				
				RS485_Send_Data(tx_buffer, (tx_buffer[5] + 9));
				BARRIER_StartRxTimeoutTimer(BARRIER_RESPONSE_TIMEOUT);
				eBusScaningState = SCAN_PENDING;				
				break;	
			}
			
			
			case SCAN_PENDING:
			{	
				if(((rx_buffer[1] == (rs485_interface_address >> 8)) && \
					(rx_buffer[2] == (rs485_interface_address & 0xff))) && \
					((rx_buffer[3] == (rs485_barrier_address >> 8)) && \
					(rx_buffer[4] == (rs485_barrier_address & 0xff))) && \
					(rx_buffer[rx_buffer[5] + 8] == BARRIER_EOT))
				{
					rx_chksm = 0;
					for (tmp_j = 6; tmp_j < (rx_buffer[5] + 6); tmp_j++) rx_chksm += rx_buffer[tmp_j];

					if ((rx_buffer[rx_buffer[5] + 6] == (rx_chksm >> 8)) && \
						(rx_buffer[rx_buffer[5] + 7] == (rx_chksm & 0xff)))
					{
						eBusScaningState = SCAN_RECEIVE;
					}
				}
				else if(IsBARRIER_RxTimeoutTimerExpired())
				{
					
					if(option == RS485_SCANNER_FIND_ADDRESSED)
					{
						return (0);
					}
					else
					{
						eBusScaningState = SCAN_SETUP;
					}
				}
				break;
			}
			
			
			case SCAN_RECEIVE:
			{	
				barrier_firmware_update_address_list[new_fnd] = rs485_barrier_address;
				
				if((option == RS485_SCANNER_FIND_FIRST) || \
					(option == RS485_SCANNER_FIND_NEXT) || \
					(option == RS485_SCANNER_FIND_ADDRESSED))
				{
					return (1);
				}
				else
				{
					BARRIER_StartRxTimeoutTimer(BARRIER_RESPONSE_TIMEOUT);
					eBusScaningState = SCAN_SETUP;
					++new_fnd;
				}	
				break;
			}
			
			
			case SCAN_SETUP:
			{
				if(option == RS485_SCANNER_FIND_NEW)
				{
					tmp_address = 0;
					barrier_address_list_cnt = 0;
					
					while(barrier_address_list[barrier_address_list_cnt + 1] != 0x0000)	// find if call address is allready used 
					{
						tmp_address = BARRIER_GetNextAddress();					
						if((start_address + address_offset) == tmp_address)
						{
							tmp_address = 0;
							++address_offset;
							barrier_address_list_cnt = 0;
						}
					}
				}				
				else if(option == RS485_SCANNER_FIND_ADDRESSED)
				{
					address_offset = 0;
				}
				
				rs485_barrier_address = (start_address + address_offset);
				if((start_address + address_offset) > end_address) scn_pcnt = 1;
				rs485_interface_address = RS485_INTERFACE_DEFAULT_ADDRESS;
				++address_offset;
				BARRIER_PrepareCommandPacket(BARRIER_GET_SYS_INFO, rx_buffer);			
				eBusScaningState = SCAN_SEND;			
				break;
			}
		}
	}
	
	return (new_fnd);
}

void BARRIER_PrepareTimeUpdatePacket(void)
{
    uint8_t i;
    RTC_GetDate(RTC_Format_BCD, &RTC_Date);
    RTC_GetTime(RTC_Format_BCD, &RTC_Time);

    tx_buffer[0] = BARRIER_SOH;
    tx_buffer[1] = rs485_barrier_address >> 8;
    tx_buffer[2] = rs485_barrier_address & 0x00ff;
    tx_buffer[3] = rs485_interface_address >> 8;
    tx_buffer[4] = rs485_interface_address & 0x00ff;
    tx_buffer[5] = 0x0d;
    tx_buffer[6] = BARRIER_SET_RTC_DATE_TIME;
    tx_buffer[7] = (RTC_Date.RTC_Date >> 4) + 48;
    tx_buffer[8] = (RTC_Date.RTC_Date & 0x0f) + 48;
    tx_buffer[9] = (RTC_Date.RTC_Month >> 4) + 48;
    tx_buffer[10] = (RTC_Date.RTC_Month & 0x0f) + 48;
    tx_buffer[11] = (RTC_Date.RTC_Year >> 4) + 48;
    tx_buffer[12] = (RTC_Date.RTC_Year & 0x0f) + 48;
    tx_buffer[13] = (RTC_Time.RTC_Hours >> 4) + 48;
    tx_buffer[14] = (RTC_Time.RTC_Hours & 0x0f) + 48;
    tx_buffer[15] = (RTC_Time.RTC_Minutes >> 4) + 48;
    tx_buffer[16] = (RTC_Time.RTC_Minutes & 0x0f) + 48;
    tx_buffer[17] = (RTC_Time.RTC_Seconds >> 4) + 48;
    tx_buffer[18] = (RTC_Time.RTC_Seconds & 0x0f) + 48;

    rs485_packet_checksum = 0;

    for (i = 6; i < 19; i++)
    {
        rs485_packet_checksum += tx_buffer[i];
    }

    tx_buffer[19] = rs485_packet_checksum >> 8;
    tx_buffer[20] = rs485_packet_checksum;
    tx_buffer[21] = BARRIER_EOT;
}

void BARRIER_PrepareStatusUpdatePacket(void)
{
    tx_buffer[0] = BARRIER_SOH;
    tx_buffer[1] = rs485_barrier_address >> 8;
    tx_buffer[2] = rs485_barrier_address & 0x00ff;
    tx_buffer[3] = rs485_interface_address >> 8;
    tx_buffer[4] = rs485_interface_address & 0x00ff;
    tx_buffer[5] = 0x01;
    tx_buffer[6] = BARRIER_GET_SYS_STATUS;
    tx_buffer[7] = 0x00;
    tx_buffer[8] = tx_buffer[6];
    tx_buffer[9] = BARRIER_EOT;
}

uint8_t BARRIER_CheckConfigFile(void)
{
	UINT bytes_rd;
	char *ret;
	
	bytes_rd = 0;
	config_file_byte_cnt = 0;
		
	if (f_mount(&filesystem, "0:", 0) != FR_OK)
	{
		return FILE_SYS_ERROR;
	}
	
	if (f_opendir(&dir_1, "/") != FR_OK)
	{
		f_mount(NULL,"0:",0);
		return FILE_DIR_ERROR;
	}
	
	if (f_open(&file_SD, "UPDATE.CFG", FA_READ) != FR_OK) 
	{
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	if(f_read (&file_SD, aWorkingBuffer, BARRIER_CONFIG_FILE_MAX_SIZE, &bytes_rd) != FR_OK)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	if(bytes_rd >= BARRIER_CONFIG_FILE_MAX_SIZE)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	p_barrier_buffer = config_file_buffer;			
	while(p_barrier_buffer < config_file_buffer + sizeof(config_file_buffer)) *p_barrier_buffer++ = NULL;
	p_barrier_buffer = config_file_buffer;
	
	ret = strstr((const char *) aWorkingBuffer, "<HWI>");
		
	if(ret == NULL)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	ret += BARRIER_CONFIG_FILE_TAG_LENGHT;
	
	while((*ret != '<') && (*ret != NULL))
	{
		*p_barrier_buffer++ = *ret++;
	}
	
	++p_barrier_buffer;
	
	ret = strstr((const char *) aWorkingBuffer, "<FWI>");
		
	if(ret == NULL)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	ret += BARRIER_CONFIG_FILE_TAG_LENGHT;
	
	while((*ret != '<') && (*ret != NULL))
	{
		*p_barrier_buffer++ = *ret++;
	}
	
	++p_barrier_buffer;
	
	ret = strstr((const char *) aWorkingBuffer, "<UDT>");
		
	if(ret == NULL)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	ret += BARRIER_CONFIG_FILE_TAG_LENGHT;
	
	while((*ret != '<') && (*ret != NULL))
	{
		*p_barrier_buffer++ = *ret++;
	}
	
	config_file_byte_cnt = p_barrier_buffer - config_file_buffer;
	
	return (FILE_OK);
}

uint8_t BARRIER_CreateUpdateAddresseList(void)
{
	UINT brd;
	char *rtn;
	uint8_t fual_add[8];
	uint16_t tmp_add, fual_cnt, tmp_cnt;
	
	brd = 0;
	tmp_cnt = 0;
	tmp_add = 0;
	fual_cnt = 0;
	
//	if (f_mount(&filesystem, "0:", 0) != FR_OK)
//	{
//		return FILE_SYS_ERROR;
//	}
//	
//	if (f_opendir(&dir_1, "/") != FR_OK)
//	{
//		f_mount(NULL,"0:",0);
//		return FILE_DIR_ERROR;
//	}
//	
//	if (f_open(&file_SD, "UPDATE.CFG", FA_READ) != FR_OK) 
//	{
//		f_mount(NULL,"0:",0);
//		return FILE_ERROR;
//	}
	
	if(f_lseek (&file_SD, config_file_byte_cnt) != FR_OK)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	if(f_read (&file_SD, aWorkingBuffer, BARRIER_CONFIG_FILE_MAX_SIZE, &brd) != FR_OK)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	if(brd >= BARRIER_CONFIG_FILE_MAX_SIZE)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	rtn = strstr((const char *) aWorkingBuffer, "<UFA>");
		
	if(rtn == NULL)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	p_barrier_buffer = fual_add;			
	while(p_barrier_buffer < fual_add + sizeof(fual_add)) *p_barrier_buffer++ = NULL;
	p_barrier_buffer = fual_add;
	
	rtn += BARRIER_CONFIG_FILE_TAG_LENGHT;
	
	while((*rtn != '<') && (*rtn != NULL))
	{
		*p_barrier_buffer++ = *rtn++;
		
		if((*rtn == ',') || (*rtn == '<'))
		{
			if(*rtn == ',') rtn++;
			
			tmp_add = atoi((char *) fual_add);
			tmp_cnt = 0;
			
			while(barrier_address_list[tmp_cnt] != 0x0000)
			{
				if(barrier_address_list[tmp_cnt] == tmp_add)
				{
					barrier_firmware_update_address_list[fual_cnt] = tmp_add;
					p_barrier_buffer = fual_add;			
					while(p_barrier_buffer < fual_add + sizeof(fual_add)) *p_barrier_buffer++ = NULL;
					p_barrier_buffer = fual_add;
					fual_cnt++;
				}
				tmp_cnt++;
			}
		}
	}
	
	rtn++;
	brd = 0;
	tmp_cnt = 0;
	tmp_add = 0;
	fual_cnt = 0;
	
	config_file_byte_cnt = ((uint8_t *)rtn - aWorkingBuffer) + config_file_byte_cnt;
	
	
	if(f_lseek (&file_SD, config_file_byte_cnt) != FR_OK)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	if(f_read (&file_SD, aWorkingBuffer, BARRIER_CONFIG_FILE_MAX_SIZE, &brd) != FR_OK)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	if(brd >= BARRIER_CONFIG_FILE_MAX_SIZE)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	rtn = strstr((const char *) aWorkingBuffer, "<UIA>");
		
	if(rtn == NULL)
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
	p_barrier_buffer = fual_add;			
	while(p_barrier_buffer < fual_add + sizeof(fual_add)) *p_barrier_buffer++ = NULL;
	p_barrier_buffer = fual_add;
	
	rtn += BARRIER_CONFIG_FILE_TAG_LENGHT;
	
	while((*rtn != '<') && (*rtn != NULL))
	{
		*p_barrier_buffer++ = *rtn++;
		
		if((*rtn == ',') || (*rtn == '<'))
		{
			if(*rtn == ',') rtn++;
			
			tmp_add = atoi((char *) fual_add);
			tmp_cnt = 0;
			
			while(barrier_address_list[tmp_cnt] != 0x0000)
			{
				if(barrier_address_list[tmp_cnt] == tmp_add)
				{
					barrier_image_update_address_list[fual_cnt] = tmp_add;
					p_barrier_buffer = fual_add;			
					while(p_barrier_buffer < fual_add + sizeof(fual_add)) *p_barrier_buffer++ = NULL;
					p_barrier_buffer = fual_add;
					fual_cnt++;
				}
				tmp_cnt++;
			}
		}
	}
	
	f_close(&file_SD);
	
	return FILE_OK;
}

uint8_t BARRIER_CheckNewFirmwareFile(void)
{
	
//	W25Qxx_Read((W25QXX_FW_END_ADDRESS - 3), file_size, 4);
//	
//	if((file_size[0] == 0) && (file_size[1] == 0) && (file_size[2] == 0) && (file_size[3] == 0)) return FILE_SYS_ERROR;
//	else if((file_size[0] == 0xff) && (file_size[1] == 0xff) && (file_size[2] == 0xff) && (file_size[3] == 0xff)) return FILE_SYS_ERROR;

	
	if (f_mount(&filesystem, "0:", 0) != FR_OK)
	{
		return FILE_SYS_ERROR;
	}

	if (f_opendir(&dir_1, "/") != FR_OK)
	{
		f_mount(NULL,"0:",0);
		return FILE_DIR_ERROR;
	}
		
	if (f_open(&file_SD, "NEW.BIN", FA_READ) != FR_OK) 
	{
		f_mount(NULL,"0:",0);
		return FILE_ERROR;
	}
	
    if (barrier_ctrl_request != BARRIER_UPDATE_FROM_CONFIG_FILE) 
	{
		rs485_barrier_address = atoi((char *) aWorkingBuffer);
	}
	else if(barrier_firmware_update_address_list[barrier_address_list_cnt] != 0x0000)
	{
		rs485_barrier_address = barrier_firmware_update_address_list[barrier_address_list_cnt];
	}
	else
	{
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
		return FW_UPDATE_FINISHED;
	}
	
    BARRIER_FirmwareUpdatePacket.update_state = FW_UPDATE_INIT;
    BARRIER_FirmwareUpdatePacket.send_attempt = 0;
    BARRIER_FirmwareUpdatePacket.packet_send = 0;
    BARRIER_FirmwareUpdatePacket.last_packet_send = 0;
//	BARRIER_FirmwareUpdatePacket.packet_total = ((file_size[0] << 24) + (file_size[1] << 16) + (file_size[2] << 8) + file_size[3]) / BARRIER_PACKET_BUFFER_SIZE;

//    if ((BARRIER_FirmwareUpdatePacket.packet_total * BARRIER_PACKET_BUFFER_SIZE) < ((file_size[0] << 24) + (file_size[1] << 16) + (file_size[2] << 8) + file_size[3]))
//    {
//        ++BARRIER_FirmwareUpdatePacket.packet_total;
//    }
    BARRIER_FirmwareUpdatePacket.packet_total = file_SD.obj.objsize / BARRIER_PACKET_BUFFER_SIZE;

    if ((BARRIER_FirmwareUpdatePacket.packet_total * BARRIER_PACKET_BUFFER_SIZE) < file_SD.obj.objsize)
    {
        ++BARRIER_FirmwareUpdatePacket.packet_total;
    }

    return FILE_OK;	
}

uint8_t BARRIER_CheckNewImageFile(void)
{
	uint32_t k;
	
	if (f_mount(&filesystem, "0:", 0) != FR_OK)
	{
		return FILE_SYS_ERROR;
	}

	if (f_opendir(&dir_1, "/") != FR_OK)
	{
		f_mount(NULL,"0:",0);
		return FILE_DIR_ERROR;
	}

	if (barrier_ctrl_request == BARRIER_UPDATE_FROM_CONFIG_FILE)
	{
		if(config_file_image_cnt == 1)
		{
			if (f_open(&file_SD, "IMG1.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_1;
		}
		else if(config_file_image_cnt == 2)
		{
			if (f_open(&file_SD, "IMG2.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_2;
		}
		else if(config_file_image_cnt == 3)
		{
			if (f_open(&file_SD, "IMG3.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_3;
		}
		else if(config_file_image_cnt == 4)
		{
			if (f_open(&file_SD, "IMG4.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_4;
		}
		else if(config_file_image_cnt == 5)
		{
			if (f_open(&file_SD, "IMG5.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_5;
		}
		else if(config_file_image_cnt == 6)
		{
			if (f_open(&file_SD, "IMG6.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_6;
		}
		else if(config_file_image_cnt == 7)
		{
			if (f_open(&file_SD, "IMG7.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_7;
		}
		else if(config_file_image_cnt == 8)
		{
			if (f_open(&file_SD, "IMG8.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_8;
		}
		else if(config_file_image_cnt == 9)
		{
			if (f_open(&file_SD, "IMG9.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_9;
		}
		else if(config_file_image_cnt == 10)
		{
			if (f_open(&file_SD, "IMG10.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_10;
		}
		else if(config_file_image_cnt == 11)
		{
			if (f_open(&file_SD, "IMG11.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_11;
		}
		else if(config_file_image_cnt == 12)
		{
			if (f_open(&file_SD, "IMG12.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_12;
		}
		else if(config_file_image_cnt == 13)
		{
			if (f_open(&file_SD, "IMG13.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_13;
		}
		else if(config_file_image_cnt == 14)
		{
			if (f_open(&file_SD, "IMG14.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_14;
		}
		else if(config_file_image_cnt == 15)
		{
			if (f_open(&file_SD, "IMG15.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_15;
		}
		else if(config_file_image_cnt == 16)
		{
			if (f_open(&file_SD, "IMG16.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_16;
		}
		else if(config_file_image_cnt == 17)
		{
			if (f_open(&file_SD, "IMG17.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_17;
		}
		else if(config_file_image_cnt == 18)
		{
			if (f_open(&file_SD, "IMG18.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_18;
		}
		else if(config_file_image_cnt == 19)
		{
			if (f_open(&file_SD, "IMG19.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_19;
		}
		else
		{
			f_mount(NULL,"0:",0);
			return FILE_ERROR;
		}
	}
	else
	{	
		k = 0;
		
		while(k < sizeof(aWorkingBuffer))
		{
			if(aWorkingBuffer[k] == NULL) break;
			else k++;
		}
		k++;
		
		if(aWorkingBuffer[k] == '1')
		{
			k++;
			
			if(aWorkingBuffer[k] == NULL)
			{
				if (f_open(&file_SD, "IMG1.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_1;
			}
			else if(aWorkingBuffer[k] == '0')
			{
				if (f_open(&file_SD, "IMG10.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}			
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_10;
			}
			else if(aWorkingBuffer[k] == '1')
			{
				if (f_open(&file_SD, "IMG11.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_11;
			}
			else if(aWorkingBuffer[k] == '2')
			{
				if (f_open(&file_SD, "IMG12.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_12;
			}
			else if(aWorkingBuffer[k] == '3')
			{
				if (f_open(&file_SD, "IMG13.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_13;
			}
			else if(aWorkingBuffer[k] == '4')
			{
				if (f_open(&file_SD, "IMG14.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_14;
			}
			else if(aWorkingBuffer[k] == '5')
			{
				if (f_open(&file_SD, "IMG15.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_15;
			}
			else if(aWorkingBuffer[k] == '6')
			{
				if (f_open(&file_SD, "IMG16.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_16;
			}
			else if(aWorkingBuffer[k] == '7')
			{
				if (f_open(&file_SD, "IMG17.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_17;
			}
			else if(aWorkingBuffer[k] == '8')
			{
				if (f_open(&file_SD, "IMG18.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_18;
			}
			else if(aWorkingBuffer[k] == '9')
			{
				if (f_open(&file_SD, "IMG19.RAW", FA_READ) != FR_OK) 
				{
					f_mount(NULL,"0:",0);
					return FILE_ERROR;
				}
				else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_19;
			}
		}
		else if(aWorkingBuffer[k] == '2') 
		{
			if (f_open(&file_SD, "IMG2.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_2;
		}
		else if(aWorkingBuffer[k] == '3') 
		{
			if (f_open(&file_SD, "IMG3.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_3;
		}
		else if(aWorkingBuffer[k] == '4') 
		{
			if (f_open(&file_SD, "IMG4.RAW", FA_READ) != FR_OK)
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_4;
		}
		else if(aWorkingBuffer[k] == '5') 
		{
			if (f_open(&file_SD, "IMG5.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_5;
		}
		else if(aWorkingBuffer[k] == '6') 
		{
			if (f_open(&file_SD, "IMG6.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_6;
		}
		else if(aWorkingBuffer[k] == '7') 
		{
			if (f_open(&file_SD, "IMG7.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_7;
		}
		else if(aWorkingBuffer[k] == '8') 
		{
			if (f_open(&file_SD, "IMG8.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_8;
		}
		else if(aWorkingBuffer[k] == '9') 
		{
			if (f_open(&file_SD, "IMG9.RAW", FA_READ) != FR_OK) 
			{
				f_mount(NULL,"0:",0);
				return FILE_ERROR;
			}
			else BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IMAGE_9;
		}
		else return FILE_ERROR;
	}
	
//	if((file_size[0] == 0) && (file_size[1] == 0) && (file_size[2] == 0) && (file_size[3] == 0)) return FILE_SYS_ERROR;
//	else if((file_size[0] == 0xff) && (file_size[1] == 0xff) && (file_size[2] == 0xff) && (file_size[3] == 0xff)) return FILE_SYS_ERROR;
	
	if (barrier_ctrl_request != BARRIER_UPDATE_FROM_CONFIG_FILE) 
	{
		rs485_barrier_address = atoi((char *) aWorkingBuffer);
	}
	else if(barrier_image_update_address_list[barrier_address_list_cnt] != 0x0000)
	{
		rs485_barrier_address = barrier_image_update_address_list[barrier_address_list_cnt];
	}
	else
	{
		f_close(&file_SD);
		return FILE_UPDATE_FINISHED;
	}
	
	//rs485_barrier_address = atoi((char *) aWorkingBuffer);
    BARRIER_FileUpdatePacket.send_attempt = 0;
    BARRIER_FileUpdatePacket.packet_send = 0;
    BARRIER_FileUpdatePacket.last_packet_send = 0;
//    BARRIER_FileUpdatePacket.packet_total = ((file_size[0] << 24) + (file_size[1] << 16) + (file_size[2] << 8) + file_size[3]) / BARRIER_PACKET_BUFFER_SIZE;
	BARRIER_FileUpdatePacket.packet_total = file_SD.obj.objsize / BARRIER_PACKET_BUFFER_SIZE;
//    if ((BARRIER_FileUpdatePacket.packet_total * BARRIER_PACKET_BUFFER_SIZE) < ((file_size[0] << 24) + (file_size[1] << 16) + (file_size[2] << 8) + file_size[3]))
//    {
//        ++BARRIER_FileUpdatePacket.packet_total;
//    }
	if ((BARRIER_FileUpdatePacket.packet_total * BARRIER_PACKET_BUFFER_SIZE) < file_SD.obj.objsize)
    {
        ++BARRIER_FileUpdatePacket.packet_total;
    }
    return FILE_OK;
}

void BARRIER_PrepareFirmwareUpdatePacket(void)
{
//    static uint32_t fwup_address = 0;
	uint32_t i;

    //FW_UPDATE_IDLE			16
    //FW_UPDATE_INIT 			17
    //FW_UPDATE_BOOTLOADER 		18
    //FW_UPDATE_RUN				19
    //FW_UPDATE_FINISHED		20
    //FW_UPDATE_FAIL			21

    if (BARRIER_FirmwareUpdatePacket.send_attempt >= MAX_QUERY_ATTEMPTS)
    {
        BARRIER_FirmwareUpdatePacket.update_state = FW_UPDATE_FAIL;
    }

    if ((BARRIER_FirmwareUpdatePacket.update_state == FW_UPDATE_FAIL) || \
	   (BARRIER_FirmwareUpdatePacket.update_state == FW_UPDATE_FINISHED))
    {
        BARRIER_FirmwareUpdatePacket.update_state = FW_UPDATE_IDLE;
        f_close(&file_SD);
		f_mount(NULL,"0:",0);
        return;
    }
    else if (BARRIER_FirmwareUpdatePacket.update_state == FW_UPDATE_INIT)
    {
        tx_buffer[0] = BARRIER_SOH;
        tx_buffer[5] = 0x01;
        tx_buffer[6] = BARRIER_START_BOOTLOADER;
        BARRIER_FirmwareUpdatePacket.update_state = FW_UPDATE_BOOTLOADER;
    }
    else if (BARRIER_FirmwareUpdatePacket.update_state == FW_UPDATE_BOOTLOADER)
    {
        tx_buffer[0] = BARRIER_SOH;
        tx_buffer[5] = 0x03;
        tx_buffer[6] = BARRIER_DOWNLOAD_FIRMWARE;
        tx_buffer[7] = BARRIER_FirmwareUpdatePacket.packet_total >> 8;
        tx_buffer[8] = BARRIER_FirmwareUpdatePacket.packet_total & 0xff;
        BARRIER_FirmwareUpdatePacket.update_state = FW_UPDATE_RUN;
		//fwup_address = W25QXX_FW_START_ADDRESS;
    }
    else if (BARRIER_FirmwareUpdatePacket.update_state == FW_UPDATE_RUN)
    {
        tx_buffer[0] = BARRIER_STX;
        tx_buffer[5] = 0x42;
        tx_buffer[6] = BARRIER_FirmwareUpdatePacket.packet_send >> 8;
        tx_buffer[7] = BARRIER_FirmwareUpdatePacket.packet_send & 0xff;

        f_read(&file_SD, (uint8_t*) &tx_buffer[8], BARRIER_PACKET_BUFFER_SIZE, (UINT*) (&BARRIER_FirmwareUpdatePacket.file_data_read));
		
		if (BARRIER_FileUpdatePacket.packet_send == 1) delay(500);
		
//		W25Qxx_Read(fwup_address, &tx_buffer[8], BARRIER_PACKET_BUFFER_SIZE);
//		fwup_address += BARRIER_PACKET_BUFFER_SIZE;	
	}

    tx_buffer[1] = rs485_barrier_address >> 8;
    tx_buffer[2] = rs485_barrier_address & 0x00ff;
    tx_buffer[3] = rs485_interface_address >> 8;
    tx_buffer[4] = rs485_interface_address & 0x00ff;

    rs485_packet_checksum = 0;

    for (i = 6; i < (tx_buffer[5] + 6); i++)
    {
        rs485_packet_checksum += tx_buffer[i];
    }

    tx_buffer[tx_buffer[5] + 6] = rs485_packet_checksum >> 8;
    tx_buffer[tx_buffer[5] + 7] = rs485_packet_checksum;
    tx_buffer[tx_buffer[5] + 8] = BARRIER_EOT;
}

void BARRIER_PrepareFileUpdatePacket(void)
{
//	static uint32_t pfup_address = 0;
	uint32_t i;

	//	FILE_UPDATE_IDLE			40
	//	FILE_UPDATE_INIT 			41
	//	FILE_UPDATE_RUN				42
	//	FILE_UPDATE_FINISHED		43
	//	FILE_UPDATE_FAIL			44

    if (BARRIER_FileUpdatePacket.send_attempt >= MAX_QUERY_ATTEMPTS)
    {
        BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_FAIL;
    }

    if ((BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_FAIL) || \
	   (BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_FINISHED))
    {
        BARRIER_FileUpdatePacket.update_state = FILE_UPDATE_IDLE;
		f_close(&file_SD);
		f_mount(NULL,"0:",0);
        return;
    }
	else if (BARRIER_FileUpdatePacket.packet_send == 0)
    {
        tx_buffer[0] = BARRIER_SOH;
        tx_buffer[5] = 0x03;
		
        if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_1) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_1;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_2) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_2;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_3) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_3;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_4) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_4;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_5) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_5;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_6) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_6;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_7) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_7;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_8) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_8;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_9) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_9;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_10) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_10;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_11) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_11;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_12) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_12;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_13) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_13;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_14) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_14;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_15) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_15;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_16) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_16;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_17) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_17;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_18) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_18;
		}
		else if(BARRIER_FileUpdatePacket.update_state == FILE_UPDATE_IMAGE_19) 
		{
			tx_buffer[6] = BARRIER_DOWNLOAD_DISPLAY_IMAGE_19;
		}
		
        tx_buffer[7] = BARRIER_FileUpdatePacket.packet_total >> 8;
        tx_buffer[8] = BARRIER_FileUpdatePacket.packet_total & 0xff;
    }
    else
    {
        tx_buffer[0] = BARRIER_STX;
        tx_buffer[5] = 0x42;
        tx_buffer[6] = BARRIER_FileUpdatePacket.packet_send >> 8;
        tx_buffer[7] = BARRIER_FileUpdatePacket.packet_send & 0xff;

        f_read(&file_SD, (uint8_t*) &tx_buffer[8], BARRIER_PACKET_BUFFER_SIZE, (UINT*) (&BARRIER_FileUpdatePacket.file_data_read));
		
		if (BARRIER_FileUpdatePacket.packet_send == 1) delay(500);
		
//		W25Qxx_Read(pfup_address, &tx_buffer[8], BARRIER_PACKET_BUFFER_SIZE);
	
//		if(BARRIER_FileUpdatePacket.last_packet_send < BARRIER_FileUpdatePacket.packet_send)
//		{
//			pfup_address += BARRIER_PACKET_BUFFER_SIZE;
//		}
			
	}

    tx_buffer[1] = rs485_barrier_address >> 8;
    tx_buffer[2] = rs485_barrier_address & 0x00ff;
    tx_buffer[3] = rs485_interface_address >> 8;
    tx_buffer[4] = rs485_interface_address & 0x00ff;

    rs485_packet_checksum = 0;

    for (i = 6; i < (tx_buffer[5] + 6); i++)
    {
        rs485_packet_checksum += tx_buffer[i];
    }

    tx_buffer[tx_buffer[5] + 6] = rs485_packet_checksum >> 8;
    tx_buffer[tx_buffer[5] + 7] = rs485_packet_checksum;
    tx_buffer[tx_buffer[5] + 8] = BARRIER_EOT;
}

void BARRIER_PrepareCommandPacket(uint8_t command, uint8_t *ibuff)
{
    uint32_t i;
	unsigned long card_id;
	uint8_t hex[6];

    tx_buffer[0] = BARRIER_SOH;
    tx_buffer[1] = rs485_barrier_address >> 8;
    tx_buffer[2] = rs485_barrier_address & 0x00ff;
    tx_buffer[3] = rs485_interface_address >> 8;
    tx_buffer[4] = rs485_interface_address & 0x00ff;
    tx_buffer[5] = 0x01;
    tx_buffer[6] = command;
    /** 
     *	command list without formating 
     */
    // BARRIER_FLASH_PROTECTION_ENABLE)
    // BARRIER_FLASH_PROTECTION_DISABLE)
    // BARRIER_START_BOOTLOADER)
    // BARRIER_EXECUTE_APPLICATION)
    // BARRIER_GET_SYS_STATUS)
    // BARRIER_GET_SYS_INFO)
    // BARRIER_GET_DISPLAY_BRIGHTNESS)
    // BARRIER_GET_RTC_DATE_TIME
    // BARRIER_GET_LOG_LIST
    // BARRIER_DELETE_LOG_LIST
    // BARRIER_GET_RS485_CONFIG
    // BARRIER_GET_DIN_STATE
    // BARRIER_GET_DOUT_STATE
    // BARRIER_GET_PCB_TEMPERATURE
    // BARRIER_GET_TEMP_CARD_BUFFER	
    // BARRIER_GET_MIFARE_AUTHENTICATION_KEY_A
    // BARRIER_GET_MIFARE_AUTHENTICATION_KEY_B
    // BARRIER_GET_MIFARE_PERMITED_GROUP
    // BARRIER_GET_MIFARE_PERMITED_CARD_1
    // BARRIER_GET_MIFARE_PERMITED_CARD_2
    // BARRIER_GET_MIFARE_PERMITED_CARD_3
    // BARRIER_GET_MIFARE_PERMITED_CARD_4
    // BARRIER_GET_MIFARE_PERMITED_CARD_5
    // BARRIER_GET_MIFARE_PERMITED_CARD_6
    // BARRIER_GET_MIFARE_PERMITED_CARD_7
    // BARRIER_GET_MIFARE_PERMITED_CARD_8
    // BARRIER_GET_ROOM_STATUS
	// BARRIER_GET_ROOM_TEMPERATURE
	// BARRIER_SET_ROOM_TEMPERATURE
	
    if (command == BARRIER_SET_DISPLAY_BRIGHTNESS)
    {		
        tx_buffer[5] = 0x03;
		while(*ibuff != NULL) ++ibuff;
		++ibuff;
		i = atoi((char *) ibuff);
        tx_buffer[7] = i >> 8;		// display brightness MSB
        tx_buffer[8] = i & 0xff;	// display brightness LSB
    }
    else if (command == BARRIER_SET_RS485_CONFIG)
    {
        tx_buffer[5] = 13;
		while(*ibuff != NULL) ++ibuff;
		++ibuff;
		i = atoi((char *) ibuff);
		hex[0] = i >> 8;
		hex[1] = i & 0xff;
		hex[2] = BARRIER_DEFFAULT_GROUP_ADDRESS >> 8;
		hex[3] = BARRIER_DEFFAULT_GROUP_ADDRESS & 0xff;
		hex[4] = BARRIER_DEFFAULT_BROADCAST_ADDRESS >> 8;
		hex[5] = BARRIER_DEFFAULT_BROADCAST_ADDRESS & 0xff;
		Hex2Str(hex, 6, &tx_buffer[7]);		
    }
    else if (command == BARRIER_SET_DOUT_STATE)
    {
        tx_buffer[5] = 17;
		
		while(*ibuff != NULL) ++ibuff;
		
		++ibuff;
        i = 7;
        while (i < 23) tx_buffer[i++] = *ibuff++;
    }
    else if ((command == BARRIER_SET_MIFARE_AUTHENTICATION_KEY_A) || \
			 (command == BARRIER_SET_MIFARE_AUTHENTICATION_KEY_B))
    {
        tx_buffer[5] = 13;
    }
    else if (command == BARRIER_SET_MIFARE_PERMITED_GROUP)
    {
        tx_buffer[5] = 17;
    }
    else if (command == BARRIER_SET_MIFARE_PERMITED_CARD)
    {
        tx_buffer[5] = 21;
		
		while(*ibuff != NULL) ++ibuff;
		
		++ibuff;
		
		if(*ibuff == '1') tx_buffer[6] = BARRIER_SET_MIFARE_PERMITED_CARD_1;
		else if(*ibuff == '2') tx_buffer[6] = BARRIER_SET_MIFARE_PERMITED_CARD_2;
		else if(*ibuff == '3') tx_buffer[6] = BARRIER_SET_MIFARE_PERMITED_CARD_3;
		else if(*ibuff == '4') tx_buffer[6] = BARRIER_SET_MIFARE_PERMITED_CARD_4;
		else if(*ibuff == '5') tx_buffer[6] = BARRIER_SET_MIFARE_PERMITED_CARD_5;
		else if(*ibuff == '6') tx_buffer[6] = BARRIER_SET_MIFARE_PERMITED_CARD_6;
		else if(*ibuff == '7') tx_buffer[6] = BARRIER_SET_MIFARE_PERMITED_CARD_7;
		else if(*ibuff == '8') tx_buffer[6] = BARRIER_SET_MIFARE_PERMITED_CARD_8;
			
		while(*ibuff != NULL) ++ibuff;
		++ibuff;
		while(*ibuff == '0') ++ibuff;
		card_id = strtoul((char *) ibuff, NULL, 0);
		hex[0] = card_id  & 0xff;
		hex[1] = (card_id >> 8) & 0xff;
		hex[2] = (card_id >> 16) & 0xff;
		hex[3] = (card_id >> 24) & 0xff;				
		Hex2Str(hex, 4, &tx_buffer[7]);
		tx_buffer[15] = NULL;
		tx_buffer[16] = NULL;			
		
		while(*ibuff != NULL) ++ibuff;
		
		++ibuff;		
        i = 17;
        while (i < 27) tx_buffer[i++] = *ibuff++;
    }
    else if (command == BARRIER_SET_ROOM_STATUS)
    {		
        tx_buffer[5] = 0x02;
		while(*ibuff != NULL) ++ibuff;
		++ibuff;
        tx_buffer[7] = *ibuff;
		++ibuff;
		if(*ibuff != NULL) 
		{
			tx_buffer[5] = 0x03;
			tx_buffer[8] = *ibuff;
		}
    }
	else if (command == BARRIER_RESET_SOS_ALARM)
    {
        tx_buffer[5] = 2;
		tx_buffer[7] = '1';
    }
	else if (command == BARRIER_SET_ROOM_TEMPERATURE)
    {
        tx_buffer[5] = 8;
		while(*ibuff != NULL) ++ibuff;
		++ibuff;
		i = atoi((char *) ibuff);
		
		if (i > 127)
		{
			tx_buffer[7] = 'E';
			i -= 128;
		}
		else
		{
			tx_buffer[7] = 'D';
		}
		
		if (i > 63)
		{
			tx_buffer[8] = 'H';
			i -= 64;
		}
		else
		{
			tx_buffer[8] = 'C';
		}
		
		tx_buffer[9] = NULL;
		tx_buffer[10] = NULL;
		
		while(i > 9)
		{
			++tx_buffer[9];
			i -= 10;
		}
		
		tx_buffer[9] += 48;
		tx_buffer[10] = i + 48;		
		while(*ibuff != NULL) ++ibuff;
		++ibuff;
		i = atoi((char *) ibuff);
		
		if (i > 127)
		{
			tx_buffer[7] = 'O';
			i -= 128;
		}		
		tx_buffer[11] = NULL;
		tx_buffer[12] = NULL;
		tx_buffer[13] = NULL;
		
		while(i > 99)
		{
			++tx_buffer[11];
			i -= 100;
		}
		
		tx_buffer[11] += 48;
		
		while(i > 9)
		{
			++tx_buffer[12];
			i -= 10;
		}
		
		tx_buffer[12] += 48;
		tx_buffer[13] = i + 48;	
    }
	
    rs485_packet_checksum = 0;

    for (i = 6; i < (tx_buffer[5] + 6); i++)
    {
        rs485_packet_checksum += tx_buffer[i];
    }

    tx_buffer[tx_buffer[5] + 6] = rs485_packet_checksum >> 8;
    tx_buffer[tx_buffer[5] + 7] = rs485_packet_checksum;
    tx_buffer[tx_buffer[5] + 8] = BARRIER_EOT;
}

void BARRIER_PrepareLogUpdatePacket(void)
{
    if (BARRIER_LogListTransfer.send_attempt >= MAX_QUERY_ATTEMPTS)
    {
        BARRIER_LogListTransfer.log_transfer_state = LOG_TRANSFER_IDLE;
        return;
    }
    else if (BARRIER_LogListTransfer.log_transfer_state == LOG_TRANSFER_QUERY_LIST)
    {
        tx_buffer[6] = BARRIER_GET_LOG_LIST;
    }
    else if (BARRIER_LogListTransfer.log_transfer_state == LOG_TRANSFER_DELETE_LOG)
    {
        tx_buffer[6] = BARRIER_DELETE_LOG_LIST;
    }

    tx_buffer[0] = BARRIER_SOH;
    tx_buffer[1] = rs485_barrier_address >> 8;
    tx_buffer[2] = rs485_barrier_address & 0x00ff;
    tx_buffer[3] = rs485_interface_address >> 8;
    tx_buffer[4] = rs485_interface_address & 0x00ff;
    tx_buffer[5] = 0x01;
    tx_buffer[7] = 0x00;
    tx_buffer[8] = tx_buffer[6];
    tx_buffer[9] = BARRIER_EOT;
}

void BARRIER_WriteLogToList(void)
{
	uint8_t e;
	
	e = 0;
	
	++BARRIER_LogEvent.log_id;
		
	while(e < BARRIER_LOG_SIZE) 
	{
		aEepromBuffer[e] = rx_buffer[7 + e];
		++e;
	}
	aEepromBuffer[0] = (BARRIER_LogEvent.log_id >> 8);
	aEepromBuffer[1] = (BARRIER_LogEvent.log_id & 0xff);
	aEepromBuffer[2] = BARRIER_LogEvent.log_event;
	aEepromBuffer[3] = BARRIER_LogEvent.log_cycle;
	aEepromBuffer[4] = BARRIER_LogEvent.log_group;
	aEepromBuffer[5] = BARRIER_LogEvent.log_card_id[0];
	aEepromBuffer[6] = BARRIER_LogEvent.log_card_id[1];
	aEepromBuffer[7] = BARRIER_LogEvent.log_card_id[2];
	aEepromBuffer[8] = BARRIER_LogEvent.log_card_id[3];
	aEepromBuffer[9] = BARRIER_LogEvent.log_card_id[4];
	aEepromBuffer[10] = BARRIER_LogEvent.log_time_stamp[0];
	aEepromBuffer[11] = BARRIER_LogEvent.log_time_stamp[1];
	aEepromBuffer[12] = BARRIER_LogEvent.log_time_stamp[2];
	aEepromBuffer[13] = BARRIER_LogEvent.log_time_stamp[3];
	aEepromBuffer[14] = BARRIER_LogEvent.log_time_stamp[4];
	aEepromBuffer[15] = BARRIER_LogEvent.log_time_stamp[5];
	
	/**
	*	LOG_LIST_TYPE_1 -> log list is empty and next log address is first address
	*	0000000000000000000000000000000000000000000000000000000000000000000000000
	*
	*	LOG_LIST_TYPE_2 -> log list start at some addres, it's full till last address, next log address is first address and is free for write 
	*	000000000000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	*
	* 	LOG_LIST_TYPE_3 -> log list start at some addres, end at upper address, next log address is next upper from end address and is free for write
	*	000000000000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxx000000000000000000000000000
	*
	*	LOG_LIST_TYPE_4 -> log list start at first address, end at last address, it's full, next log address is first memory address, write is forbiden
	*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	*	
	*	LOG_LIST_TYPE_5 -> log list start at first address, end at upper address, and next upper log address is free for write
	*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx00000000000000000000000000000000000000000
	*
	*	LOG_LIST_TYPE_6 -> log list start at upper address, end at lower address and next upper from end address is free for write
	*	xxxxxxxxxxxx0000000000000000000000000000000000000000000000xxxxxxxxxxxxxxx
	*/
	
	switch (BARRIER_LogMemory.LOG_MemoryFragment)
	{
		case LOG_LIST_UNDEFINED:
			/** should newer get here */
//			BARRIER_StartDisplayTimer();
//			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log list undefined state");
			break;
		
		
		case LOG_LIST_TYPE_1:
			
			I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
			delay(I2C_EE_WRITE_DELAY);	
			++BARRIER_LogMemory.log_list_cnt;
			BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_5;
			BARRIER_LogMemory.next_log_address += BARRIER_LOG_SIZE;
//			BARRIER_StartDisplayTimer();
//			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log switch to type 5");
			break;
		
		
		case LOG_LIST_TYPE_2:
			
			if(BARRIER_LogMemory.next_log_address < I2C_EE_PAGE_SIZE)
			{
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
				delay(I2C_EE_WRITE_DELAY);
			}
			else
			{
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
				delay(I2C_EE_WRITE_DELAY);
			}
			
			BARRIER_LogMemory.last_log_address = BARRIER_LogMemory.next_log_address;
			BARRIER_LogMemory.next_log_address += BARRIER_LOG_SIZE;
			++BARRIER_LogMemory.log_list_cnt;
			
			if (BARRIER_LogMemory.next_log_address == BARRIER_LogMemory.first_log_address)
			{
				BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_4;
//				BARRIER_StartDisplayTimer();
//				LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log list full !!!");
			}			
			break;
		
			
		case LOG_LIST_TYPE_3:
			
			if(BARRIER_LogMemory.next_log_address < I2C_EE_PAGE_SIZE)
			{
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
				delay(I2C_EE_WRITE_DELAY);
			}
			else
			{
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
				delay(I2C_EE_WRITE_DELAY);
			}
			
			BARRIER_LogMemory.last_log_address = BARRIER_LogMemory.next_log_address;
			BARRIER_LogMemory.next_log_address += BARRIER_LOG_SIZE;
			++BARRIER_LogMemory.log_list_cnt;
			
			if (BARRIER_LogMemory.next_log_address > (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE))
			{
				BARRIER_LogMemory.next_log_address = EE_LOG_LIST_START_ADDRESS;
			}
			else if (BARRIER_LogMemory.next_log_address == BARRIER_LogMemory.first_log_address)
			{
				BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_4;
//				BARRIER_StartDisplayTimer();
//				LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log list full !!!");
			}
			break;
		
			
		case LOG_LIST_TYPE_4:
			
//			BARRIER_StartDisplayTimer();
//			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log list full !!!");
			break;
		
		
		case LOG_LIST_TYPE_5:
			
			if(BARRIER_LogMemory.next_log_address < I2C_EE_PAGE_SIZE)
			{
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
				delay(I2C_EE_WRITE_DELAY);
			}
			else
			{
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
				delay(I2C_EE_WRITE_DELAY);
			}
			
			BARRIER_LogMemory.last_log_address = BARRIER_LogMemory.next_log_address;
			BARRIER_LogMemory.next_log_address += BARRIER_LOG_SIZE;
			++BARRIER_LogMemory.log_list_cnt;			
			
			if (BARRIER_LogMemory.next_log_address > (EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE))
			{
				BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_4;
//				BARRIER_StartDisplayTimer();
//				LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log list full !!!");
			}
			break;
		
			
		case LOG_LIST_TYPE_6:
			
			if(BARRIER_LogMemory.next_log_address < I2C_EE_PAGE_SIZE)
			{
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
				delay(I2C_EE_WRITE_DELAY);
			}
			else
			{
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.next_log_address, BARRIER_LOG_SIZE);
				delay(I2C_EE_WRITE_DELAY);
			}
			
			BARRIER_LogMemory.last_log_address = BARRIER_LogMemory.next_log_address;
			BARRIER_LogMemory.next_log_address += BARRIER_LOG_SIZE;
			++BARRIER_LogMemory.log_list_cnt;
			
			if (BARRIER_LogMemory.next_log_address == BARRIER_LogMemory.first_log_address)
			{
				BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_4;
//				BARRIER_StartDisplayTimer();
//				LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log list full !!!");
			}
			break;
		
			
		default:
			
//			BARRIER_StartDisplayTimer();
//			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Shit just happen, puta madre");
			break;
		
	}// End of switch
}

void BARRIER_DeleteBlockFromLogList(void)
{
	uint16_t x_cnt;
	uint32_t delete_cnt;
	/**
	*	LOG_LIST_TYPE_1 -> log list is empty and next log address is first address
	*	0000000000000000000000000000000000000000000000000000000000000000000000000
	*
	*	LOG_LIST_TYPE_2 -> log list start at some addres, it's full till last address, next log address is first address and is free for write 
	*	000000000000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	*
	* 	LOG_LIST_TYPE_3 -> log list start at some addres, end at upper address, next log address is next upper from end address and is free for write
	*	000000000000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxx000000000000000000000000000
	*
	*	LOG_LIST_TYPE_4 -> log list start at first address, end at last address, it's full, next log address is first memory address, write is forbiden
	*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	*	
	*	LOG_LIST_TYPE_5 -> log list start at first address, end at upper address, and next upper log address is free for write
	*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx00000000000000000000000000000000000000000
	*
	*	LOG_LIST_TYPE_6 -> log list start at upper address, end at lower address and next upper from end address is free for write
	*	xxxxxxxxxxxx0000000000000000000000000000000000000000000000xxxxxxxxxxxxxxx
	*/
	
	switch (BARRIER_LogMemory.LOG_MemoryFragment)
	{
		case LOG_LIST_UNDEFINED:
			/** should newer get here */
//			BARRIER_StartDisplayTimer();
//			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log list undefined state");
			break;
		
		
		case LOG_LIST_TYPE_1:
			
			break;
		
		
		case LOG_LIST_TYPE_2:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_DELETE_LOG_LIST)
			{				
				x_cnt = 0;				
				while(x_cnt < I2C_EE_BLOCK_SIZE) aEepromBuffer[x_cnt++] = NULL;				
				delete_cnt = BARRIER_LogMemory.first_log_address;				
				while(delete_cnt >= I2C_EE_BLOCK_SIZE) delete_cnt -= I2C_EE_BLOCK_SIZE;				
				if(delete_cnt != 0) delete_cnt = ((I2C_EE_BLOCK_SIZE - delete_cnt) - 1);
				else delete_cnt = I2C_EE_BLOCK_SIZE - 1;
				
				/**
				*	delete current block
				*/
				if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.first_log_address, delete_cnt);
					delay(I2C_EE_WRITE_DELAY);
				}
				else
				{
					I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.first_log_address, delete_cnt);
					delay(I2C_EE_WRITE_DELAY);
				}
				/**
				*	set first log address
				*/
				if((BARRIER_LogMemory.first_log_address + delete_cnt + 1) >= EE_LOG_LIST_END_ADDRESS)
				{
					/**
					*	set memory fragmentation type
					*/
					if(BARRIER_LogMemory.next_log_address != EE_LOG_LIST_START_ADDRESS)
					{
						BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_5;
					}
					else
					{
						BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_1;
					}
					
					BARRIER_LogMemory.first_log_address = EE_LOG_LIST_START_ADDRESS;
				}
				else 
				{
					/**
					*	set memory fragmentation type
					*/
					if(BARRIER_LogMemory.next_log_address != EE_LOG_LIST_START_ADDRESS)
					{
						BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_6;
					}
					
					BARRIER_LogMemory.first_log_address += delete_cnt + 1;
				}
				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_DELETED;
				BARRIER_LogMemory.log_list_cnt -= ((delete_cnt + 1) / BARRIER_LOG_SIZE);				
			}
			break;
		
			
		case LOG_LIST_TYPE_3:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_DELETE_LOG_LIST)
			{				
				x_cnt = 0;				
				while(x_cnt < I2C_EE_BLOCK_SIZE) aEepromBuffer[x_cnt++] = NULL;				
				delete_cnt = BARRIER_LogMemory.first_log_address;				
				while(delete_cnt >= I2C_EE_BLOCK_SIZE) delete_cnt -= I2C_EE_BLOCK_SIZE;				
				if(delete_cnt != 0) delete_cnt = ((I2C_EE_BLOCK_SIZE - delete_cnt) - 1);
				else delete_cnt = I2C_EE_BLOCK_SIZE - 1;
				
				if((BARRIER_LogMemory.first_log_address + delete_cnt + 1) >= (BARRIER_LogMemory.last_log_address + BARRIER_LOG_SIZE))
				{
					delete_cnt = ((BARRIER_LogMemory.last_log_address + BARRIER_LOG_SIZE) - BARRIER_LogMemory.first_log_address) - 1;
					if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
					{
						I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.first_log_address, delete_cnt);
						delay(I2C_EE_WRITE_DELAY);
					}
					else
					{
						I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.first_log_address, delete_cnt);
						delay(I2C_EE_WRITE_DELAY);
					}
					BARRIER_LogMemory.first_log_address = EE_LOG_LIST_START_ADDRESS;
					BARRIER_LogMemory.last_log_address = EE_LOG_LIST_START_ADDRESS;
					BARRIER_LogMemory.next_log_address = EE_LOG_LIST_START_ADDRESS;
					BARRIER_LogMemory.log_list_cnt = 0;
					BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_1;
				}
				else
				{
					if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
					{
						I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.first_log_address, delete_cnt);
						delay(I2C_EE_WRITE_DELAY);
					}
					else
					{
						I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.first_log_address, delete_cnt);
						delay(I2C_EE_WRITE_DELAY);
					}
					BARRIER_LogMemory.log_list_cnt -= ((delete_cnt + 1) / BARRIER_LOG_SIZE);
					BARRIER_LogMemory.first_log_address += delete_cnt + 1;
				}
				/**
				*	delete current block
				*/
				
				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_DELETED;
								
			}
			break;
		
			
		case LOG_LIST_TYPE_4:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_DELETE_LOG_LIST)
			{
				x_cnt = 0;
				 
				while(x_cnt < I2C_EE_BLOCK_SIZE) aEepromBuffer[x_cnt++] = NULL;
				
				I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, EE_LOG_LIST_START_ADDRESS, I2C_EE_BLOCK_SIZE - 1);
				delay(I2C_EE_WRITE_DELAY);
				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_DELETED;
				BARRIER_LogMemory.first_log_address = HTTP_LogListTransfer.log_transfer_end_address;
				BARRIER_LogMemory.last_log_address = EE_LOG_LIST_END_ADDRESS - BARRIER_LOG_SIZE;
				BARRIER_LogMemory.next_log_address = EE_LOG_LIST_START_ADDRESS;
				BARRIER_LogMemory.log_list_cnt -= (I2C_EE_BLOCK_SIZE / BARRIER_LOG_SIZE);
				BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_2;
			}
			break;
		
		
		case LOG_LIST_TYPE_5:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_DELETE_LOG_LIST)
			{				
				x_cnt = 0;				
				while(x_cnt < I2C_EE_BLOCK_SIZE) aEepromBuffer[x_cnt++] = NULL;				
				delete_cnt = BARRIER_LogMemory.first_log_address;				
				while(delete_cnt >= I2C_EE_BLOCK_SIZE) delete_cnt -= I2C_EE_BLOCK_SIZE;				
				if(delete_cnt != 0) delete_cnt = ((I2C_EE_BLOCK_SIZE - delete_cnt) - 1);
				else delete_cnt = I2C_EE_BLOCK_SIZE - 1;
				
				if((BARRIER_LogMemory.first_log_address + delete_cnt + 1) >= (BARRIER_LogMemory.last_log_address + BARRIER_LOG_SIZE))
				{
					delete_cnt = ((BARRIER_LogMemory.last_log_address + BARRIER_LOG_SIZE) - BARRIER_LogMemory.first_log_address) - 1;
					/**
					*	delete current block
					*/
					if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
					{
						I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.first_log_address, delete_cnt);
						delay(I2C_EE_WRITE_DELAY);
					}
					else
					{
						I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.first_log_address, delete_cnt);
						delay(I2C_EE_WRITE_DELAY);
					}
					BARRIER_LogMemory.first_log_address = EE_LOG_LIST_START_ADDRESS;
					BARRIER_LogMemory.last_log_address = EE_LOG_LIST_START_ADDRESS;
					BARRIER_LogMemory.next_log_address = EE_LOG_LIST_START_ADDRESS;
					BARRIER_LogMemory.log_list_cnt = 0;
					BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_1;
				}
				else
				{
					/**
					*	delete current block
					*/
					if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
					{
						I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.first_log_address, delete_cnt);
						delay(I2C_EE_WRITE_DELAY);
					}
					else
					{
						I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.first_log_address, delete_cnt);
						delay(I2C_EE_WRITE_DELAY);
					}
					BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_3;
					BARRIER_LogMemory.log_list_cnt -= ((delete_cnt + 1) / BARRIER_LOG_SIZE);
					BARRIER_LogMemory.first_log_address += delete_cnt + 1;
				}
				
				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_DELETED;								
			}
			break;
		
			
		case LOG_LIST_TYPE_6:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_DELETE_LOG_LIST)
			{				
				x_cnt = 0;				
				while(x_cnt < I2C_EE_BLOCK_SIZE) aEepromBuffer[x_cnt++] = NULL;				
				delete_cnt = BARRIER_LogMemory.first_log_address;				
				while(delete_cnt >= I2C_EE_BLOCK_SIZE) delete_cnt -= I2C_EE_BLOCK_SIZE;				
				if(delete_cnt != 0) delete_cnt = ((I2C_EE_BLOCK_SIZE - delete_cnt) - 1);
				else delete_cnt = I2C_EE_BLOCK_SIZE - 1;
				
				/**
				*	delete current block
				*/
				if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_0, BARRIER_LogMemory.first_log_address, delete_cnt);
					delay(I2C_EE_WRITE_DELAY);
				}
				else
				{
					I2C_EERPOM_WriteBytes16(I2C_EE_WRITE_PAGE_1, BARRIER_LogMemory.first_log_address, delete_cnt);
					delay(I2C_EE_WRITE_DELAY);
				}
				/**
				*	set first log address
				*/
				if((BARRIER_LogMemory.first_log_address + delete_cnt + 1) >= EE_LOG_LIST_END_ADDRESS)
				{
					/**
					*	set memory fragmentation type
					*/
					BARRIER_LogMemory.LOG_MemoryFragment = LOG_LIST_TYPE_5;
					BARRIER_LogMemory.first_log_address = EE_LOG_LIST_START_ADDRESS;
				}
				else 
				{
					BARRIER_LogMemory.first_log_address += delete_cnt + 1;
				}
				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_DELETED;
				BARRIER_LogMemory.log_list_cnt -= ((delete_cnt + 1) / BARRIER_LOG_SIZE);			
			}
			break;
		
			
		default:
			
//			BARRIER_StartDisplayTimer();
//			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Shit just happen, puta madre");
			break;
		
	}// End of switch
}

void BARRIER_ReadBlockFromLogList(void)
{
	uint32_t read_cnt;
	/**
	*	LOG_LIST_TYPE_1 -> log list is empty and next log address is first address
	*	0000000000000000000000000000000000000000000000000000000000000000000000000
	*
	*	LOG_LIST_TYPE_2 -> log list start at some addres, it's full till last address, next log address is first address and is free for write 
	*	000000000000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	*
	* 	LOG_LIST_TYPE_3 -> log list start at some addres, end at upper address, next log address is next upper from end address and is free for write
	*	000000000000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxx000000000000000000000000000
	*
	*	LOG_LIST_TYPE_4 -> log list start at first address, end at last address, it's full, next log address is first memory address, write is forbiden
	*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	*	
	*	LOG_LIST_TYPE_5 -> log list start at first address, end at upper address, and next upper log address is free for write
	*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx00000000000000000000000000000000000000000
	*
	*	LOG_LIST_TYPE_6 -> log list start at upper address, end at lower address and next upper from end address is free for write
	*	xxxxxxxxxxxx0000000000000000000000000000000000000000000000xxxxxxxxxxxxxxx
	*/
	
	switch (BARRIER_LogMemory.LOG_MemoryFragment)
	{
		case LOG_LIST_UNDEFINED:
			/** should newer get here */
//			BARRIER_StartDisplayTimer();
//			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Log list undefined state");
			break;
		
		
		case LOG_LIST_TYPE_1:
			
			break;
		
		
		case LOG_LIST_TYPE_2:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_GET_LOG_LIST)
			{
				read_cnt = BARRIER_LogMemory.first_log_address;				
				while(read_cnt >= I2C_EE_BLOCK_SIZE) read_cnt -= I2C_EE_BLOCK_SIZE;				
				if(read_cnt != 0) read_cnt = ((I2C_EE_BLOCK_SIZE - read_cnt) - 1);
				else read_cnt = I2C_EE_BLOCK_SIZE - 1;
				
				if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, BARRIER_LogMemory.first_log_address, read_cnt);
				}
				else
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, BARRIER_LogMemory.first_log_address, read_cnt);
				}
				
				Hex2Str(aEepromBuffer, I2C_EE_BLOCK_SIZE, aWorkingBuffer);				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_READY;				
			}
			break;
		
			
		case LOG_LIST_TYPE_3:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_GET_LOG_LIST)
			{
				read_cnt = BARRIER_LogMemory.first_log_address;				
				while(read_cnt >= I2C_EE_BLOCK_SIZE) read_cnt -= I2C_EE_BLOCK_SIZE;				
				if(read_cnt != 0) read_cnt = ((I2C_EE_BLOCK_SIZE - read_cnt) - 1);
				else read_cnt = I2C_EE_BLOCK_SIZE - 1;
				
				if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, BARRIER_LogMemory.first_log_address, read_cnt);
				}
				else
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, BARRIER_LogMemory.first_log_address, read_cnt);
				}
				
				Hex2Str(aEepromBuffer, I2C_EE_BLOCK_SIZE, aWorkingBuffer);				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_READY;				
			}
			break;
		
			
		case LOG_LIST_TYPE_4:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_GET_LOG_LIST)
			{
				I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, BARRIER_LogMemory.first_log_address, I2C_EE_BLOCK_SIZE - 1);
			
				Hex2Str(aEepromBuffer, I2C_EE_BLOCK_SIZE, aWorkingBuffer);
				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_READY;
				HTTP_LogListTransfer.log_transfer_end_address = EE_LOG_LIST_START_ADDRESS + I2C_EE_BLOCK_SIZE;
			}
			break;
		
		
		case LOG_LIST_TYPE_5:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_GET_LOG_LIST)
			{
				read_cnt = BARRIER_LogMemory.first_log_address;				
				while(read_cnt >= I2C_EE_BLOCK_SIZE) read_cnt -= I2C_EE_BLOCK_SIZE;				
				if(read_cnt != 0) read_cnt = ((I2C_EE_BLOCK_SIZE - read_cnt) - 1);
				else read_cnt = I2C_EE_BLOCK_SIZE - 1;
				
				if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, BARRIER_LogMemory.first_log_address, read_cnt);
				}
				else
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, BARRIER_LogMemory.first_log_address, read_cnt);
				}
									
				Hex2Str(aEepromBuffer, I2C_EE_BLOCK_SIZE, aWorkingBuffer);				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_READY;				
			}
			break;
		
			
		case LOG_LIST_TYPE_6:
			
			if(HTTP_LogListTransfer.log_transfer_state == HTTP_GET_LOG_LIST)
			{
				read_cnt = BARRIER_LogMemory.first_log_address;				
				while(read_cnt >= I2C_EE_BLOCK_SIZE) read_cnt -= I2C_EE_BLOCK_SIZE;				
				if(read_cnt != 0) read_cnt = ((I2C_EE_BLOCK_SIZE - read_cnt) - 1);
				else read_cnt = I2C_EE_BLOCK_SIZE - 1;
				
				if(BARRIER_LogMemory.first_log_address < I2C_EE_PAGE_SIZE)
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, BARRIER_LogMemory.first_log_address, read_cnt);
				}
				else
				{
					I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_1, BARRIER_LogMemory.first_log_address, read_cnt);
				}
				
				Hex2Str(aEepromBuffer, I2C_EE_BLOCK_SIZE, aWorkingBuffer);				
				HTTP_LogListTransfer.log_transfer_state = HTTP_LOG_LIST_READY;				
			}
			break;
		
			
		default:
			
//			BARRIER_StartDisplayTimer();
//			LCD_DisplayStringLine(LCD_LINE_10, (uint8_t*) "  Shit just happen, puta madre");
			break;
		
	}// End of switch	
}

void BARRIER_IncreaseCounter(void)
{
	//uint32_t counter;
	
	if(BarrierState == BARRIER_ENTRY)
	{
		if(sCardData.card_usage_type == CARD_TYPE_ONE_TIME)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 3);
			
			counter += 1;
			
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);
			
			tmp_ONE_TIME_USER_ENTRY_COUNTER = counter;
		}
		else if(sCardData.card_usage_type == CARD_TYPE_MULTI)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 3);
			
			counter += 1;
			
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);
			
			tmp_PREPAID_USER_ENTRY_COUNTER = counter;
		}
		
		if(sCardData.card_tag_type == CARD_TAG_TYPE_CLASIC)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 3);

			counter += 1;

			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);
			
			tmp_CARD_USER_ENTRY_COUNTER = counter;
		}
		else if(sCardData.card_tag_type == CARD_TAG_TYPE_WRIST)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 3);

			counter += 1;

			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);

			tmp_WRIST_USER_ENTRY_COUNTER = counter;
		}
		else if(sCardData.card_tag_type == CARD_TAG_TYPE_KEY_RING)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 3);

			counter += 1;

			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);

			tmp_KEY_RING_USER_ENTRY_COUNTER = counter;
		}
	
		counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS);
		counter = (counter << 8);
		counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS + 1);
		counter = (counter << 8);
		counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS + 2);
		counter = (counter << 8);
		counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS + 3);

		counter += 1;

		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS, (counter >> 24));
		Delay(I2C_EE_WRITE_DELAY);
		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS + 1, (counter >> 16));
		Delay(I2C_EE_WRITE_DELAY);
		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS + 2, (counter >> 8));
		Delay(I2C_EE_WRITE_DELAY);
		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_TOTAL_USER_ENTRY_COUNTER_ADDRESS + 3, (counter & 0xff));
		Delay(I2C_EE_WRITE_DELAY);
		
		tmp_TOTAL_USER_ENTRY_COUNTER = counter;
	
	}
	else if(BarrierState == BARRIER_EXIT)
	{
		if(sCardData.card_usage_type == CARD_TYPE_ONE_TIME)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 3);
			
			counter += 1;
			
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);
			
			tmp_ONE_TIME_USER_EXIT_COUNTER = counter;
		}
		else if(sCardData.card_usage_type == CARD_TYPE_MULTI)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 3);
			
			counter += 1;
			
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);
			
			tmp_PREPAID_USER_EXIT_COUNTER = counter;
		}
		
		if(sCardData.card_tag_type == CARD_TAG_TYPE_CLASIC)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 3);

			counter += 1;

			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);
			
			tmp_CARD_USER_EXIT_COUNTER = counter;
		}
		else if(sCardData.card_tag_type == CARD_TAG_TYPE_WRIST)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 3);

			counter += 1;

			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);

			tmp_WRIST_USER_EXIT_COUNTER = counter;
		}
		else if(sCardData.card_tag_type == CARD_TAG_TYPE_KEY_RING)
		{
			counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 1);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 2);
			counter = (counter << 8);
			counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 3);

			counter += 1;

			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS, (counter >> 24));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 1, (counter >> 16));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 2, (counter >> 8));
			Delay(I2C_EE_WRITE_DELAY);
			I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 3, (counter & 0xff));
			Delay(I2C_EE_WRITE_DELAY);

			tmp_KEY_RING_USER_EXIT_COUNTER = counter;
		}
	
		counter = (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_TOTAL_USER_EXIT_COUNTER_ADDRESS);
		counter = (counter << 8);
		counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_TOTAL_USER_EXIT_COUNTER_ADDRESS + 1);
		counter = (counter << 8);
		counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_TOTAL_USER_EXIT_COUNTER_ADDRESS + 2);
		counter = (counter << 8);
		counter += (uint8_t) I2C_EERPOM_ReadByte16(I2C_EE_READ_PAGE_0, EE_TOTAL_USER_EXIT_COUNTER_ADDRESS + 3);

		counter += 1;

		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_TOTAL_USER_EXIT_COUNTER_ADDRESS, (counter >> 24));
		Delay(I2C_EE_WRITE_DELAY);
		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_TOTAL_USER_EXIT_COUNTER_ADDRESS + 1, (counter >> 16));
		Delay(I2C_EE_WRITE_DELAY);
		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_TOTAL_USER_EXIT_COUNTER_ADDRESS + 2, (counter >> 8));
		Delay(I2C_EE_WRITE_DELAY);
		I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_TOTAL_USER_EXIT_COUNTER_ADDRESS + 3, (counter & 0xff));
		Delay(I2C_EE_WRITE_DELAY);		

		tmp_TOTAL_USER_EXIT_COUNTER = counter;
	}
}

void BARRIER_ReadCounters(void)
{
	uint8_t cnt;
	
	for(cnt = 0; cnt < I2C_EE_BUFFER_SIZE; ++cnt)
	{
		aEepromBuffer[cnt] = 0;
	}
	
	I2C_EERPOM_ReadBytes16(I2C_EE_READ_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS, 54);
}

void BARRIER_ResetCounters(void)
{
	RTC_GetTime(RTC_Format_BCD, &RTC_Time);
	RTC_GetDate(RTC_Format_BCD, &RTC_Date);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_ENTRY_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_ONE_TIME_USER_EXIT_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_ENTRY_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_PREPAID_USER_EXIT_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_ENTRY_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_CARD_USER_EXIT_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_ENTRY_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_WRIST_USER_EXIT_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_ENTRY_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 1, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 2, 0);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_KEY_RING_USER_EXIT_COUNTER_ADDRESS + 3, 0);
	
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_COUNTER_RESET_DATE_ADDRESS, RTC_Date.RTC_Date);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_COUNTER_RESET_DATE_ADDRESS + 1, RTC_Date.RTC_Month);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_COUNTER_RESET_DATE_ADDRESS + 2, RTC_Date.RTC_Year);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_COUNTER_RESET_DATE_ADDRESS + 3, RTC_Time.RTC_Hours);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_COUNTER_RESET_DATE_ADDRESS + 4, RTC_Time.RTC_Minutes);
	I2C_EERPOM_WriteByte16(I2C_EE_WRITE_PAGE_0, EE_COUNTER_RESET_DATE_ADDRESS + 5, RTC_Time.RTC_Seconds);
}





