/**
 ******************************************************************************
 * File Name          : mfrc522.c
 * RTC_Date               : 28/02/2016 23:16:19
 * Description        : mifare RC522 software modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_spi.h"
#include "rc522.h"
#include "rtc.h"
#include "dio.h"
#include "stepper.h"
#include "barrier_controller.h"
#include "common.h"
#include "display.h"

/* Imprted   -------- --------------------------------------------------------*/
//extern SPI_HandleTypeDef hspi1;
//extern RTC_HandleTypeDef hrtc;
extern RTC_DateTypeDef RTC_Date;
extern RTC_TimeTypeDef RTC_Time;
extern volatile uint32_t SystickCnt;

/* Private defines    --------------------------------------------------------*/
//#define CARD_TEST						1
//#define CARD_CLEAR					2

#define MIFARE_16_BYTES                 0x01
#define MIFARE_64_BYTES                 0x02
#define MIFARE_512_BYTES                0x03
#define MIFARE_1K_BYTES                 0x04
#define MIFARE_4K_BYTES                 0x05

/* mifare card memory sectors offset */
#define SECTOR_0                        0x00 
#define SECTOR_1                        0x04 
#define SECTOR_2                        0x08 
#define SECTOR_3                        0x0c 
#define SECTOR_4                        0x10 
#define SECTOR_5                        0x14
#define SECTOR_6                        0x18
#define SECTOR_7                        0x1c
#define SECTOR_8                        0x20
#define SECTOR_9                        0x24
#define SECTOR_10                       0x28
#define SECTOR_11                       0x2c
#define SECTOR_12                       0x30
#define SECTOR_13                       0x34
#define SECTOR_14                       0x38
#define SECTOR_15                       0x3c
#define SECTOR_16                       0x40
#define SECTOR_17                       0x44
#define SECTOR_18                       0x48
#define SECTOR_19                       0x4c
#define SECTOR_20                       0x50
#define SECTOR_21                       0x54
#define SECTOR_22                       0x58
#define SECTOR_23                       0x5c
#define SECTOR_24                       0x60
#define SECTOR_25                       0x64
#define SECTOR_26                       0x6c
#define SECTOR_27                       0x70
#define SECTOR_28                       0x74
#define SECTOR_29                       0x78
#define SECTOR_30                       0x7c
#define SECTOR_31                       0x80

/* RC522 Commands */
#define PCD_IDLE					0x00   //NO action; Cancel the current command
#define PCD_AUTHENT					0x0E   //Authentication Key
#define PCD_RECEIVE					0x08   //Receive Data
#define PCD_TRANSMIT				0x04   //Transmit data
#define PCD_TRANSCEIVE				0x0C   //Transmit and receive data,
#define PCD_RESETPHASE				0x0F   //Reset
#define PCD_CALCCRC					0x03   //CRC Calculate

/* Mifare_One card command word */
#define PICC_REQIDL					0x26   // find the antenna area does not enter hibernation
#define PICC_REQALL					0x52   // find all the cards antenna area
#define PICC_ANTICOLL				0x93   // anti-collision
#define PICC_SELECTTAG				0x93   // election card
#define PICC_AUTHENT1A				0x60   // authentication key A
#define PICC_AUTHENT1B				0x61   // authentication key B
#define PICC_READ					0x30   // Read Block
#define PICC_WRITE					0xA0   // write block
#define PICC_DECREMENT				0xC0   // debit
#define PICC_INCREMENT				0xC1   // recharge
#define PICC_RESTORE				0xC2   // transfer block data to the buffer
#define PICC_TRANSFER				0xB0   // save the data in the buffer
#define PICC_HALT					0x50   // Sleep

/* RC522 Registers */
//Page 0: Command and Status
#define RC522_REG_RESERVED00		0x00    
#define RC522_REG_COMMAND			0x01    
#define RC522_REG_COMM_IE_N			0x02    
#define RC522_REG_DIV1_EN			0x03    
#define RC522_REG_COMM_IRQ			0x04    
#define RC522_REG_DIV_IRQ			0x05
#define RC522_REG_ERROR				0x06    
#define RC522_REG_STATUS1			0x07    
#define RC522_REG_STATUS2			0x08    
#define RC522_REG_FIFO_DATA			0x09
#define RC522_REG_FIFO_LEVEL		0x0A
#define RC522_REG_WATER_LEVEL		0x0B
#define RC522_REG_CONTROL			0x0C
#define RC522_REG_BIT_FRAMING		0x0D
#define RC522_REG_COLL				0x0E
#define RC522_REG_RESERVED01		0x0F
//Page 1: Command 
#define RC522_REG_RESERVED10		0x10
#define RC522_REG_MODE				0x11
#define RC522_REG_TX_MODE			0x12
#define RC522_REG_RX_MODE			0x13
#define RC522_REG_TX_CONTROL		0x14
#define RC522_REG_TX_AUTO			0x15
#define RC522_REG_TX_SELL			0x16
#define RC522_REG_RX_SELL			0x17
#define RC522_REG_RX_THRESHOLD		0x18
#define RC522_REG_DEMOD				0x19
#define RC522_REG_RESERVED11		0x1A
#define RC522_REG_RESERVED12		0x1B
#define RC522_REG_MIFARE			0x1C
#define RC522_REG_RESERVED13		0x1D
#define RC522_REG_RESERVED14		0x1E
#define RC522_REG_SERIALSPEED		0x1F
//Page 2: CFG    
#define RC522_REG_RESERVED20		0x20  
#define RC522_REG_CRC_RESULT_M		0x21
#define RC522_REG_CRC_RESULT_L		0x22
#define RC522_REG_RESERVED21		0x23
#define RC522_REG_MOD_WIDTH			0x24
#define RC522_REG_RESERVED22		0x25
#define RC522_REG_RF_CFG			0x26
#define RC522_REG_GS_N				0x27
#define RC522_REG_CWGS_PREG			0x28
#define RC522_REG_MODGS_PREG		0x29
#define RC522_REG_T_MODE			0x2A
#define RC522_REG_T_PRESCALER		0x2B
#define RC522_REG_T_RELOAD_H		0x2C
#define RC522_REG_T_RELOAD_L		0x2D
#define RC522_REG_T_COUNTER_VALUE_H	0x2E
#define RC522_REG_T_COUNTER_VALUE_L	0x2F
//Page 3:TestRegister 
#define RC522_REG_RESERVED30		0x30
#define RC522_REG_TEST_SEL1			0x31
#define RC522_REG_TEST_SEL2			0x32
#define RC522_REG_TEST_PIN_EN		0x33
#define RC522_REG_TEST_PIN_VALUE	0x34
#define RC522_REG_TEST_BUS			0x35
#define RC522_REG_AUTO_TEST			0x36
#define RC522_REG_VERSION			0x37
#define RC522_REG_ANALOG_TEST		0x38
#define RC522_REG_TEST_ADC1			0x39  
#define RC522_REG_TEST_ADC2			0x3A   
#define RC522_REG_TEST_ADC0			0x3B   
#define RC522_REG_RESERVED31		0x3C   
#define RC522_REG_RESERVED32		0x3D
#define RC522_REG_RESERVED33		0x3E   
#define RC522_REG_RESERVED34		0x3F

/* Private types  -----------------------------------------------------------*/
typedef enum 
{
	MI_ERR			= 0x00,
	MI_OK 			= 0x01,
	MI_NOTAGERR		= 0x02

} RC522_StatusTypeDef;

typedef struct 
{
    uint8_t B_Add_0;
    uint8_t B_Add_1;
    uint8_t B_Add_2;
    uint8_t B_Add_3;
	
} RC522_BlockAddressTypeDef;

typedef struct
{
    uint8_t B_Dat_0[RC522_BLOCK_SIZE];
    uint8_t B_Dat_1[RC522_BLOCK_SIZE];
    uint8_t B_Dat_2[RC522_BLOCK_SIZE];
    uint8_t B_Dat_3[RC522_BLOCK_SIZE];
	
} RC522_BlockDataTypeDef;

typedef struct
{    
    RC522_BlockAddressTypeDef BlockAddress;
    RC522_BlockDataTypeDef BlockData;  
	
} RC522_SectorTypeDef;



/* Private variables  --------------------------------------------------------*/
volatile uint32_t mifare_process_timer;
volatile uint32_t mifare_process_flags;

uint8_t aMifareAuthenticationKeyA[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 	//buffer A password, 16 buffer, the password of every buffer is 6 bytes 
uint8_t aMifareAuthenticationKeyB[6]= {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t aRC522_DataBuffer[RC522_BUFFER_SIZE];
uint8_t aCardSerial[5];
uint8_t aCardID[5]; //Recognized card ID

RC522_CardDataTypeDef sCardData;

static enum
{
	RC522_UNSELECT = 0x00,
	RC522_READER_1 = 0x01,
	RC522_READER_2 = 0x02,
	RC522_READER_3 = 0x03
	
}RC522_Selected = RC522_UNSELECT;


RC522_SectorTypeDef Sector_0;
RC522_SectorTypeDef Sector_1;
RC522_SectorTypeDef Sector_2;
RC522_SectorTypeDef Sector_3;
RC522_SectorTypeDef Sector_4;
RC522_SectorTypeDef Sector_5;
RC522_SectorTypeDef Sector_6;
RC522_SectorTypeDef Sector_7;
RC522_SectorTypeDef Sector_8;
RC522_SectorTypeDef Sector_9;
RC522_SectorTypeDef Sector_10;
RC522_SectorTypeDef Sector_11;
RC522_SectorTypeDef Sector_12;
RC522_SectorTypeDef Sector_13;
RC522_SectorTypeDef Sector_14;
RC522_SectorTypeDef Sector_15;
RC522_SectorTypeDef Sector_16;
RC522_SectorTypeDef Sector_17;
RC522_SectorTypeDef Sector_18;
RC522_SectorTypeDef Sector_19;
RC522_SectorTypeDef Sector_20;
RC522_SectorTypeDef Sector_21;
RC522_SectorTypeDef Sector_22;
RC522_SectorTypeDef Sector_23;
RC522_SectorTypeDef Sector_24;
RC522_SectorTypeDef Sector_25;
RC522_SectorTypeDef Sector_26;
RC522_SectorTypeDef Sector_27;
RC522_SectorTypeDef Sector_28;
RC522_SectorTypeDef Sector_29;
RC522_SectorTypeDef Sector_30;
RC522_SectorTypeDef Sector_31;
    
/* Private macros   ----------------------------------------------------------*/

/* Private prototypes    -----------------------------------------------------*/
RC522_StatusTypeDef RC522_Check(uint8_t* id);
RC522_StatusTypeDef RC522_Compare(uint8_t* aCardID, uint8_t* CompareID);
void RC522_WriteRegister(uint8_t addr, uint8_t val);
uint8_t RC522_ReadRegister(uint8_t addr);
void RC522_SetBitMask(uint8_t reg, uint8_t mask);
void RC522_ClearBitMask(uint8_t reg, uint8_t mask);
void RC522_AntennaOn(void);
void RC522_AntennaOff(void);
RC522_StatusTypeDef RC522_Reset(void);
RC522_StatusTypeDef RC522_Request(uint8_t reqMode, uint8_t* TagType);
RC522_StatusTypeDef RC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen);
RC522_StatusTypeDef RC522_Anticoll(uint8_t* serNum);
void RC522_CalculateCRC(uint8_t* pIndata, uint8_t len, uint8_t* pOutData);
uint8_t RC522_SelectTag(uint8_t* serNum);
RC522_StatusTypeDef RC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum);
RC522_StatusTypeDef RC522_Read(uint8_t blockAddr, uint8_t* recvData);
RC522_StatusTypeDef RC522_Write(uint8_t blockAddr, uint8_t* writeData);
void RC522_Halt(void);
RC522_StatusTypeDef RC522_ReadCard(void);
void RC522_WriteCard(void);
RC522_StatusTypeDef RC522_VerifyData(void);
void RC522_ClearData(void);
void RC522_CheckCardData(uint8_t *pbuf);
void RCC522_InitSPI(void);

/* Program code   ------------------------------------------------------------*/
void RC522_Init(void) 
{
	RCC522_InitSPI();
	
	Sector_0.BlockAddress.B_Add_0 = 0;
	Sector_0.BlockAddress.B_Add_1 = 1;
	Sector_0.BlockAddress.B_Add_2 = 2;
	Sector_0.BlockAddress.B_Add_3 = 3;

	Sector_1.BlockAddress.B_Add_0 = 4;
	Sector_1.BlockAddress.B_Add_1 = 5;
	Sector_1.BlockAddress.B_Add_2 = 6;
	Sector_1.BlockAddress.B_Add_3 = 7;

	Sector_2.BlockAddress.B_Add_0 = 8;
	Sector_2.BlockAddress.B_Add_1 = 9;
	Sector_2.BlockAddress.B_Add_2 = 10;
	Sector_2.BlockAddress.B_Add_3 = 11;

	Sector_3.BlockAddress.B_Add_0 = 12;
	Sector_3.BlockAddress.B_Add_1 = 13;
	Sector_3.BlockAddress.B_Add_2 = 14;
	Sector_3.BlockAddress.B_Add_3 = 15;

	Sector_4.BlockAddress.B_Add_0 = 16;
	Sector_4.BlockAddress.B_Add_1 = 17;
	Sector_4.BlockAddress.B_Add_2 = 18;
	Sector_4.BlockAddress.B_Add_3 = 19;

	Sector_5.BlockAddress.B_Add_0 = 20;
	Sector_5.BlockAddress.B_Add_1 = 21;
	Sector_5.BlockAddress.B_Add_2 = 22;
	Sector_5.BlockAddress.B_Add_3 = 23;

	Sector_6.BlockAddress.B_Add_0 = 24;
	Sector_6.BlockAddress.B_Add_1 = 25;
	Sector_6.BlockAddress.B_Add_2 = 26;
	Sector_6.BlockAddress.B_Add_3 = 27;

	Sector_7.BlockAddress.B_Add_0 = 28;
	Sector_7.BlockAddress.B_Add_1 = 29;
	Sector_7.BlockAddress.B_Add_2 = 30;
	Sector_7.BlockAddress.B_Add_3 = 31;

	Sector_8.BlockAddress.B_Add_0 = 32;
	Sector_8.BlockAddress.B_Add_1 = 33;
	Sector_8.BlockAddress.B_Add_2 = 34;
	Sector_8.BlockAddress.B_Add_3 = 35;

	Sector_9.BlockAddress.B_Add_0 = 36;
	Sector_9.BlockAddress.B_Add_1 = 37;
	Sector_9.BlockAddress.B_Add_2 = 38;
	Sector_9.BlockAddress.B_Add_3 = 39;

	Sector_10.BlockAddress.B_Add_0 = 40;
	Sector_10.BlockAddress.B_Add_1 = 41;
	Sector_10.BlockAddress.B_Add_2 = 42;
	Sector_10.BlockAddress.B_Add_3 = 43;

	Sector_11.BlockAddress.B_Add_0 = 44;
	Sector_11.BlockAddress.B_Add_1 = 45;
	Sector_11.BlockAddress.B_Add_2 = 46;
	Sector_11.BlockAddress.B_Add_3 = 47;

	Sector_12.BlockAddress.B_Add_0 = 48;
	Sector_12.BlockAddress.B_Add_1 = 49;
	Sector_12.BlockAddress.B_Add_2 = 50;
	Sector_12.BlockAddress.B_Add_3 = 51;

	Sector_13.BlockAddress.B_Add_0 = 52;
	Sector_13.BlockAddress.B_Add_1 = 53;
	Sector_13.BlockAddress.B_Add_2 = 54;
	Sector_13.BlockAddress.B_Add_3 = 55;

	Sector_14.BlockAddress.B_Add_0 = 56;
	Sector_14.BlockAddress.B_Add_1 = 57;
	Sector_14.BlockAddress.B_Add_2 = 58;
	Sector_14.BlockAddress.B_Add_3 = 59;

	Sector_15.BlockAddress.B_Add_0 = 60;
	Sector_15.BlockAddress.B_Add_1 = 61;
	Sector_15.BlockAddress.B_Add_2 = 62;
	Sector_15.BlockAddress.B_Add_3 = 63;
	
	Mifare1_Unselect();
	Mifare2_Unselect();
	Mifare3_Unselect();
	MifareResetAssert();
	DOUT_Service();
	Delay(10);
	MifareResetRelease();
	DOUT_Service();
	Delay(50);
	RC522_Selected = RC522_READER_1;
//	Mifare1_Select();
//	DOUT_Service();
	//RC522_Reset();
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_RESETPHASE);
	
	RC522_WriteRegister(RC522_REG_T_MODE, 0x8d);
	RC522_WriteRegister(RC522_REG_T_PRESCALER, 0x3e);
	RC522_WriteRegister(RC522_REG_T_RELOAD_H, 0);
	RC522_WriteRegister(RC522_REG_T_RELOAD_L, 30);	
	//RC522_WriteRegister(RC522_REG_RF_CFG, 0x38);	
	RC522_WriteRegister(RC522_REG_TX_AUTO, 0x40);
	RC522_WriteRegister(RC522_REG_MODE, 0x3D);
	RC522_AntennaOn();
//	Delay(50);
	RC522_Selected = RC522_READER_2;
//	RC522_Reset();
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_RESETPHASE);
	
	RC522_WriteRegister(RC522_REG_T_MODE, 0x8d);
	RC522_WriteRegister(RC522_REG_T_PRESCALER, 0x3e);
	RC522_WriteRegister(RC522_REG_T_RELOAD_H, 0);
	RC522_WriteRegister(RC522_REG_T_RELOAD_L, 30);	
	//RC522_WriteRegister(RC522_REG_RF_CFG, 0x38);	
	RC522_WriteRegister(RC522_REG_TX_AUTO, 0x40);
	RC522_WriteRegister(RC522_REG_MODE, 0x3D);
	RC522_AntennaOn();
//	Delay(250);
	RC522_Selected = RC522_READER_3;
//	RC522_Reset();
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_RESETPHASE);
	
	RC522_WriteRegister(RC522_REG_T_MODE, 0x8d);
	RC522_WriteRegister(RC522_REG_T_PRESCALER, 0x3e);
	RC522_WriteRegister(RC522_REG_T_RELOAD_H, 0);
	RC522_WriteRegister(RC522_REG_T_RELOAD_L, 30);	
	//RC522_WriteRegister(RC522_REG_RF_CFG, 0x38);	
	RC522_WriteRegister(RC522_REG_TX_AUTO, 0x40);
	RC522_WriteRegister(RC522_REG_MODE, 0x3D);
	RC522_AntennaOn();

}// End of mifare modul init


void RC522_Service(void)
{
	if(BarrierState != BARRIER_STOP) return;
	if(!IsRC522_TimerExpired()) return;
		
	RCC522_InitSPI();
	
	if (RC522_Check(aCardSerial) == MI_OK)
	{
		RC522_ClearData();
		/**
		*	vrite init to card
		*/
#ifdef RC522_CARD_WRITE_INIT		
		RC522_WriteCard();
#endif		
		BARRIER_LogEvent.log_cycle = 0; // using as local variable
		
		while (RC522_ReadCard() != MI_OK)
		{
			if(++BARRIER_LogEvent.log_cycle == 10) return;
		}
		
		RC522_VerifyData();		
		
		sCardData.card_reader = RC522_Selected;
		RTC_GetTime(RTC_Format_BCD, &RTC_Time);
		RTC_GetDate(RTC_Format_BCD, &RTC_Date);
		BARRIER_LogEvent.log_event = 0;
		BARRIER_LogEvent.log_cycle = 0; 
		BARRIER_LogEvent.log_group = 0;
		BARRIER_LogEvent.log_time_stamp[0] = RTC_Date.RTC_Date;
		BARRIER_LogEvent.log_time_stamp[1] = RTC_Date.RTC_Month;
		BARRIER_LogEvent.log_time_stamp[2] = RTC_Date.RTC_Year;
		BARRIER_LogEvent.log_time_stamp[3] = RTC_Time.RTC_Hours;
		BARRIER_LogEvent.log_time_stamp[4] = RTC_Time.RTC_Minutes;
		BARRIER_LogEvent.log_time_stamp[5] = RTC_Time.RTC_Seconds;
		BARRIER_LogEvent.log_card_id[0] = sCardData.aUserCardID[0];
		BARRIER_LogEvent.log_card_id[1] = sCardData.aUserCardID[1];
		BARRIER_LogEvent.log_card_id[2] = sCardData.aUserCardID[2];
		BARRIER_LogEvent.log_card_id[3] = sCardData.aUserCardID[3];
		BARRIER_LogEvent.log_card_id[4] = sCardData.aUserCardID[4];
		
		if(sCardData.card_status == CARD_VALID)
		{	
			BarrierLedSignal_AllOff();
			
			if ((RC522_Selected == RC522_READER_1) || (RC522_Selected == RC522_READER_2))
			{
				BarrierState = BARRIER_EXIT;					
				BARRIER_LogEvent.log_event = BARRIER_LOG_EXIT;

			}
			else if (RC522_Selected == RC522_READER_3)
			{
				BarrierState = BARRIER_ENTRY;
				BARRIER_LogEvent.log_event = BARRIER_LOG_ENTRY;	
			}
			
			
			barrier_cycle_cnt = sCardData.card_number_of_users;
			BARRIER_LogEvent.log_group = sCardData.card_user_group;
			
			DISPLAY_AccessPermitted();
			//BARRIER_WriteLogToList();
			if(CardStackerState == CARD_STACKER_INSERTED)
			{
				if(sCardData.card_usage_type == CARD_TYPE_ONE_TIME) CardStackerState = CARD_STACKER_TAKE_CMD;
				else CardStackerState = CARD_STACKER_EJECT_CMD;
			}
			
			RC522_StartTimer(RC522_CARD_VALID_EVENT_TIME);
		}
		else
		{
			BarrierLedSignal_AllOff();
			ExitRedSignal_On();
			EntryRedSignal_On();
			DOUT_Service();
			barrier_pcnt = 0;
			barrier_cycle_cnt = 0;
			BarrierState = BARRIER_STOP;
			
			
			if (sCardData.system_id == CARD_SYSTEM_ID_INVALID) 
			{
				BARRIER_LogEvent.log_event = BARRIER_LOG_WRONG_SYSTEM_ID;
			}
			else if(sCardData.card_status == CARD_EXPIRY_TIME_INVALID)
			{
				BARRIER_LogEvent.log_event = BARRIER_LOG_CARD_EXPIRED;
			}
			else
			{
				BARRIER_LogEvent.log_event = BARRIER_LOG_GUEST_CARD_INVALID;
			}
			
			DISPLAY_AccessForbiden();
			BARRIER_WriteLogToList();
			BarrierProcessTimerStart(BARRIER_BRAKE_TIME);
			RC522_StartTimer(RC522_CARD_INVALID_EVENT_TIME);
			if(CardStackerState == CARD_STACKER_INSERTED) CardStackerState = CARD_STACKER_EJECT_CMD;
		}		
		
		RC522_Reset();
		RC522_WriteRegister(RC522_REG_T_MODE, 0x8D);
		RC522_WriteRegister(RC522_REG_T_PRESCALER, 0x3E);
		RC522_WriteRegister(RC522_REG_T_RELOAD_L, 30);           
		RC522_WriteRegister(RC522_REG_T_RELOAD_H, 0);
		RC522_WriteRegister(RC522_REG_TX_AUTO, 0x40);
		RC522_WriteRegister(RC522_REG_MODE, 0x3D);
		RC522_AntennaOn();// Open the antenna
		RC522_StartTimer(RC522_CARD_INVALID_EVENT_TIME);
		
	}
	else
	{
		aCardSerial[0] = 0;
		aCardSerial[1] = 0;
		aCardSerial[2] = 0;
		aCardSerial[3] = 0;
		aCardSerial[4] = 0;
		RC522_StartTimer(RC522_PROCESS_LOOP_TIME);
	}

	if(RC522_Selected == RC522_READER_1) RC522_Selected = RC522_READER_2;
	else if(RC522_Selected == RC522_READER_2) RC522_Selected = RC522_READER_3;
	else if(RC522_Selected == RC522_READER_3)RC522_Selected = RC522_READER_1;

}// End of mifare service function

RC522_StatusTypeDef RC522_Check(uint8_t* id) 
{
   
	RC522_StatusTypeDef status;
	
	status = RC522_Request(PICC_REQIDL, id);	            // Find cards, return card type
    
	if (status == MI_OK) {                                  // Card detected
		
		status = RC522_Anticoll(id);	                    // Anti-collision, return card serial number 4 bytes
        
	}// End of if...
    
	RC522_Halt();			                                // Command card into hibernation 

	return (status);
    
}// End of check command

RC522_StatusTypeDef RC522_Compare(uint8_t* aCardID, uint8_t* CompareID) 
{
    
	uint8_t i;
    
	for (i = 0; i < 5; i++) {
        
		if (aCardID[i] != CompareID[i]) {
            
			return (MI_ERR);
            
		}// End of if...
	}// End of for loop
    
	return (MI_OK);
    
}// End of compare function

void RC522_WriteRegister(uint8_t addr, uint8_t val) 
{

    if(RC522_Selected == RC522_READER_1) Mifare1_Select();
	else if(RC522_Selected == RC522_READER_2) Mifare2_Select();
	else if(RC522_Selected == RC522_READER_3) Mifare3_Select();
	DOUT_Service();

	while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI5, ((addr << 1) & 0x7E));
	while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI5);
	while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI5, val);
	while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI5);
    
    if(RC522_Selected == RC522_READER_1) Mifare1_Unselect();
	else if(RC522_Selected == RC522_READER_2)Mifare2_Unselect();
	else if(RC522_Selected == RC522_READER_3)Mifare3_Unselect();	
	DOUT_Service();
	
}// End of write register function

uint8_t RC522_ReadRegister(uint8_t addr) 
{
	uint8_t b_ret;
	
	if(RC522_Selected == RC522_READER_1) Mifare1_Select();
	else if(RC522_Selected == RC522_READER_2) Mifare2_Select();
	else if(RC522_Selected == RC522_READER_3) Mifare3_Select(); 
	DOUT_Service();
  
	while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI5, (((addr << 1) & 0x7E) | 0x80));
	while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_RXNE) == RESET);
	b_ret = SPI_I2S_ReceiveData(SPI5);
	while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI5, 0x00);
	while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_RXNE) == RESET);
	b_ret = SPI_I2S_ReceiveData(SPI5);
	
    if(RC522_Selected == RC522_READER_1) Mifare1_Unselect();
	else if(RC522_Selected == RC522_READER_2)Mifare2_Unselect();
	else if(RC522_Selected == RC522_READER_3)Mifare3_Unselect();
	DOUT_Service();
	
    return (b_ret);
        
}// End of read register function

void RC522_SetBitMask(uint8_t reg, uint8_t mask) 
{   
	RC522_WriteRegister(reg, RC522_ReadRegister(reg) | mask);
    
}// End of set bit mask function


void RC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    
	RC522_WriteRegister(reg, RC522_ReadRegister(reg) & (~mask));
    
}// End of clear bit mask function

void RC522_AntennaOn(void) 
{
    
	uint8_t temp;

	temp = RC522_ReadRegister(RC522_REG_TX_CONTROL);
    
	if (!(temp & 0x03)) {
        
		RC522_SetBitMask(RC522_REG_TX_CONTROL, 0x03);
        
	}// End of if...
    
}// End of antena on function

void RC522_AntennaOff(void) 
{   
	RC522_ClearBitMask(RC522_REG_TX_CONTROL, 0x03);
    
}// End of antena off function

RC522_StatusTypeDef RC522_Reset(void) 
{
    
    uint16_t delay;
    /**
    *   Issue the SoftReset command.
    */
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_RESETPHASE);
    /**
    *   The datasheet does not mention how long the SoftRest command takes to complete.
    *   But the RC522 might have been in soft power-down mode (triggered by bit 4 of CommandReg)
    *   Section 8.8.2 in the datasheet says the oscillator start-up RTC_Time is the start up RTC_Time of the crystal + 37,74us. Let us be generous: 50ms.
    */
//	Delay(5);
	delay = 0;
    /**
    *   Wait for the PowerDown bit in CommandReg to be cleared
    */
    while (RC522_ReadRegister(RC522_REG_COMMAND) & (1<<4)){
        /**
        *   RC522 still restarting - unlikely after waiting 50ms and more
        *   mifare modul is unresponsive so return error status
        */
        if(++delay > 50) return (MI_ERR);
        else Delay(2);
		
    }// End of while...
    /**
    *   reset finished - return OK flag
    */
    return (MI_OK);
    
}// End of software reset function

RC522_StatusTypeDef RC522_Request(uint8_t reqMode, uint8_t* TagType) 
{
    
	RC522_StatusTypeDef status;  
	uint16_t backBits;			                            //The received data bits

	RC522_WriteRegister(RC522_REG_BIT_FRAMING, 0x07);	// TxLastBits = BitFramingReg[2..0]	???

	TagType[0] = reqMode;
	status = RC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10)) {
        
		status = MI_ERR;
        
	}// End of if...

	return (status);
    
}// End of request function

RC522_StatusTypeDef RC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) 
{
	
    RC522_StatusTypeDef status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;
  

	switch (command) 
	{
        
		case PCD_AUTHENT:
			irqEn = 0x12;
			waitIRq = 0x10;            
			break;
        
		case PCD_TRANSCEIVE:            
			irqEn = 0x77;
			waitIRq = 0x30;            
			break;
        
		default:
			break;
            
			
	}// End of switch command
	
	RC522_WriteRegister(RC522_REG_COMM_IE_N, irqEn|0x80);
	RC522_ClearBitMask(RC522_REG_COMM_IRQ, 0x80);
	RC522_SetBitMask(RC522_REG_FIFO_LEVEL, 0x80);
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_IDLE);

	//Writing data to the FIFO
	for (i = 0; i < sendLen; i++) {   
        
		RC522_WriteRegister(RC522_REG_FIFO_DATA, sendData[i]);   
        
	}// End of for loop

	//Execute the command
	RC522_WriteRegister(RC522_REG_COMMAND, command);
    
	if (command == PCD_TRANSCEIVE) {    
        
		RC522_SetBitMask(RC522_REG_BIT_FRAMING, 0x80);  //StartSend=1,transmission of data starts  
	
    }// End of if... 
    /**
    *   Waiting to receive data to complete
    */
	i = 2000;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
	do {
        /**
        *   CommIrqReg[7..0]
        *   Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        */
		n = RC522_ReadRegister(RC522_REG_COMM_IRQ);
		i--;
        
	} while ((i != 0) && !(n & 0x01) && !(n&waitIRq));          // End of do...while loop            
    /**
    *   StartSend=0
    */
	RC522_ClearBitMask(RC522_REG_BIT_FRAMING, 0x80);

	if (i != 0)  {
		
		if (!(RC522_ReadRegister(RC522_REG_ERROR) & 0x1B)) {
            
			status = MI_OK;
            
			if (n & irqEn & 0x01) {
                
				status = MI_NOTAGERR;
                
			}// End of if...

			if (command == PCD_TRANSCEIVE) {
                
				n = RC522_ReadRegister(RC522_REG_FIFO_LEVEL);
				lastBits = RC522_ReadRegister(RC522_REG_CONTROL) & 0x07;
                
				if (lastBits) *backLen = (n - 1)*8 + lastBits;  
                else *backLen = n * 8;  

				if (n == 0) n = 1;
                
				if (n > RC522_BUFFER_SIZE) n = RC522_BUFFER_SIZE;   
				/**
                *   Reading the received data in FIFO
                */
				for (i = 0; i < n; i++) {
                    
					backData[i] = RC522_ReadRegister(RC522_REG_FIFO_DATA);
                    
				}// End of for loop
			}// End of if (command == PCD_TRANSCEIVE)
            
		} else {
            
			status = MI_ERR;
            
		}// End of else
	}// End of if (i != 0)

	return (status);
    
}// End of to card function

RC522_StatusTypeDef RC522_Anticoll(uint8_t* serNum) 
{
    
	RC522_StatusTypeDef status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	RC522_WriteRegister(RC522_REG_BIT_FRAMING, 0x00);   // TxLastBists = BitFramingReg[2..0]

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = RC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		/**
        *   Check card serial number
        */
		for (i = 0; i < 4; i++) {
            
			serNumCheck ^= serNum[i];
            
		}// End of for loop....
        
		if (serNumCheck != serNum[i]) {
            
			status = MI_ERR;
            
		}// End of if...
	}// End of if (status == MI_OK)
    
	return (status);
    
}// End of anticollision function

void RC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData)
{
    
	uint8_t i, n;

	RC522_ClearBitMask(RC522_REG_DIV_IRQ, 0x04);			//CRCIrq = 0
	RC522_SetBitMask(RC522_REG_FIFO_LEVEL, 0x80);			//Clear the FIFO pointer
	/**
    *   Write_RC522(CommandReg, PCD_IDLE);
    *   Writing data to the FIFO
    */
	for (i = 0; i < len; i++) {
        
		RC522_WriteRegister(RC522_REG_FIFO_DATA, *(pIndata+i)); 
        
	}// End of for loop...
    
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_CALCCRC);

	//Wait CRC calculation is complete
	i = 0xFF;
	do {
		n = RC522_ReadRegister(RC522_REG_DIV_IRQ);
		i--;
	} while ((i!=0) && !(n&0x04));			//CRCIrq = 1

	//Read CRC calculation result
	pOutData[0] = RC522_ReadRegister(RC522_REG_CRC_RESULT_L);
	pOutData[1] = RC522_ReadRegister(RC522_REG_CRC_RESULT_M);
    
}// End of calculate CRC function

uint8_t RC522_SelectTag(uint8_t* serNum) 
{
    
    RC522_StatusTypeDef status;
	uint8_t i;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[9]; 

	buffer[0] = PICC_SELECTTAG;
	buffer[1] = 0x70;
    
	for (i = 0; i < 5; i++) {
        
		buffer[i+2] = *(serNum+i);
        
	}// End of for loop...
    
	RC522_CalculateCRC(buffer, 7, &buffer[7]);		//??
	status = RC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {
        
		size = buffer[0]; 
        
	} else {
        
		size = 0;
        
	}// End of else

	return (size);
    
}// End of select tag function

RC522_StatusTypeDef RC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum) 
{
	
    RC522_StatusTypeDef status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12]; 

	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
    
	for (i = 0; i < 6; i++) { 
        
		buff[i + 2] = *(Sectorkey + i); 
        
	}// End of for loop...
    
	for (i = 0; i < 4; i++) {
        
		buff[i + 8] = *(serNum + i);
        
	}// End of for loop...
    
	status = RC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK) || (!(RC522_ReadRegister(RC522_REG_STATUS2) & 0x08))) {
        
		status = MI_ERR;
        
	}// End of if....

	return (status);
    
}// End of auth function

RC522_StatusTypeDef RC522_Read(uint8_t blockAddr, uint8_t* recvData) 
{
    
	RC522_StatusTypeDef status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	RC522_CalculateCRC(recvData, 2, &recvData[2]);
	status = RC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
        
		status = MI_ERR;
        
	}// End of if...

	return (status);
    
}// End of read function

RC522_StatusTypeDef RC522_Write(uint8_t blockAddr, uint8_t* writeData) 
{
    
	RC522_StatusTypeDef status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18]; 

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	RC522_CalculateCRC(buff, 2, &buff[2]);
	status = RC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0f) != 0x0a)) {   
        
		status = MI_ERR;   
        
	}// End of if...

	if (status == MI_OK) {
		/**
        *   Data to the FIFO write 16Byte
        */
		for (i = 0; i < 16; i++) {  
            
			buff[i] = *(writeData+i);
            
		}// End of for loop..
        
		RC522_CalculateCRC(buff, 16, &buff[16]);
		status = RC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0f) != 0x0a)) { 
            
			status = MI_ERR;   
            
		}// End of if...
	}// End of if (status == MI_OK)

	return (status);
    
}// End of write function

void RC522_Halt(void) 
{
    
	uint16_t unLen;
	uint8_t buff[4]; 

	buff[0] = PICC_HALT;
	buff[1] = 0;
	RC522_CalculateCRC(buff, 2, &buff[2]);

	RC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
    
}// End of halt function

RC522_StatusTypeDef RC522_ReadCard(void)
{
	uint8_t i;
    RC522_StatusTypeDef status;
	
    RC522_Request(PICC_REQIDL, aRC522_DataBuffer);	
    status = RC522_Anticoll(aRC522_DataBuffer);

    for(i = 0; i < 5; i++){	 
        
        aCardID[i]=aRC522_DataBuffer[i];
        
    }// End of for loop...

    RC522_SelectTag(aCardID);	
	
    status = RC522_Auth(PICC_AUTHENT1A, SECTOR_0, aMifareAuthenticationKeyA, aCardID);
	
    if(status != MI_OK)
	{	
		RC522_Halt();
		return (status);
	}
	else
	{    
        RC522_Read(Sector_0.BlockAddress.B_Add_0, &Sector_0.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_0.BlockAddress.B_Add_1, &Sector_0.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_0.BlockAddress.B_Add_2, &Sector_0.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_0.BlockAddress.B_Add_3, &Sector_0.BlockData.B_Dat_3[0]);
	}
	
	status = RC522_Auth(PICC_AUTHENT1A, SECTOR_1, aMifareAuthenticationKeyA, aCardID);
	
    if(status != MI_OK)
	{
		RC522_Halt();
		return (status);
	}
	else
	{      
        RC522_Read(Sector_1.BlockAddress.B_Add_0, &Sector_1.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_1.BlockAddress.B_Add_1, &Sector_1.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_1.BlockAddress.B_Add_2, &Sector_1.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_1.BlockAddress.B_Add_3, &Sector_1.BlockData.B_Dat_3[0]);  
	}
	
	status = RC522_Auth(PICC_AUTHENT1A, SECTOR_2, aMifareAuthenticationKeyA, aCardID);
	
    if(status != MI_OK)
	{		
		RC522_Halt();
		return (status);
	}
	else
	{      
        RC522_Read(Sector_2.BlockAddress.B_Add_0, &Sector_2.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_2.BlockAddress.B_Add_1, &Sector_2.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_2.BlockAddress.B_Add_2, &Sector_2.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_2.BlockAddress.B_Add_3, &Sector_2.BlockData.B_Dat_3[0]); 
	}
	
    status = RC522_Auth(PICC_AUTHENT1A, SECTOR_3, aMifareAuthenticationKeyA, aCardID);
	
    if(status != MI_OK)
	{		
		RC522_Halt();
		return (status);
	}
	else
	{      
        RC522_Read(Sector_3.BlockAddress.B_Add_0, &Sector_3.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_3.BlockAddress.B_Add_1, &Sector_3.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_3.BlockAddress.B_Add_2, &Sector_3.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_3.BlockAddress.B_Add_3, &Sector_3.BlockData.B_Dat_3[0]);
	}
	
    status = RC522_Auth(PICC_AUTHENT1A, SECTOR_4, aMifareAuthenticationKeyA, aCardID);
	
    if(status != MI_OK)
	{		
		RC522_Halt();
		return (status);
	}
	else
	{      
        RC522_Read(Sector_4.BlockAddress.B_Add_0, &Sector_4.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_4.BlockAddress.B_Add_1, &Sector_4.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_4.BlockAddress.B_Add_2, &Sector_4.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_4.BlockAddress.B_Add_3, &Sector_4.BlockData.B_Dat_3[0]);
	}
	
    status = RC522_Auth(PICC_AUTHENT1A, SECTOR_5, aMifareAuthenticationKeyA, aCardID);
	
    if(status != MI_OK)
	{ 			
		RC522_Halt();
		return (status);
	}
	else
	{      
        RC522_Read(Sector_5.BlockAddress.B_Add_0, &Sector_5.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_5.BlockAddress.B_Add_1, &Sector_5.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_5.BlockAddress.B_Add_2, &Sector_5.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_5.BlockAddress.B_Add_3, &Sector_5.BlockData.B_Dat_3[0]); 
	}
	
    status = RC522_Auth(PICC_AUTHENT1A, SECTOR_6, aMifareAuthenticationKeyA, aCardID); 
   
	if(status != MI_OK)
	{		
		RC522_Halt();
		return (status);
	}
	else
	{       
        RC522_Read(Sector_6.BlockAddress.B_Add_0, &Sector_6.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_6.BlockAddress.B_Add_1, &Sector_6.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_6.BlockAddress.B_Add_2, &Sector_6.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_6.BlockAddress.B_Add_3, &Sector_6.BlockData.B_Dat_3[0]); 	
	}
	
	status = RC522_Auth(PICC_AUTHENT1A, SECTOR_7, aMifareAuthenticationKeyA, aCardID);
    
    if(status != MI_OK)
	{
		RC522_Halt();
		return (status);
	}
	else
	{
		RC522_Read(Sector_7.BlockAddress.B_Add_0, &Sector_7.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_7.BlockAddress.B_Add_1, &Sector_7.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_7.BlockAddress.B_Add_2, &Sector_7.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_7.BlockAddress.B_Add_3, &Sector_7.BlockData.B_Dat_3[0]);	
	}
	
	status = RC522_Auth(PICC_AUTHENT1A, SECTOR_8, aMifareAuthenticationKeyA, aCardID); 
	
    if(status != MI_OK)
	{			
		RC522_Halt();
		return (status);
	}
	else
	{
        RC522_Read(Sector_8.BlockAddress.B_Add_0, &Sector_8.BlockData.B_Dat_0[0]);
        RC522_Read(Sector_8.BlockAddress.B_Add_1, &Sector_8.BlockData.B_Dat_1[0]);
        RC522_Read(Sector_8.BlockAddress.B_Add_2, &Sector_8.BlockData.B_Dat_2[0]);
        RC522_Read(Sector_8.BlockAddress.B_Add_3, &Sector_8.BlockData.B_Dat_3[0]);
	}

    RC522_Halt();
	return (status);
            
}// End of read card function

void RC522_WriteCard(void)
{
	uint8_t i;
	uint8_t status;
	
#ifdef RC522_CARD_WRITE_INIT
	static uint8_t card_write_cnt = 0;
	
	BarrierLedSignal_AllOff();
	DOUT_Service();
	
	if(card_write_cnt == 0)
	{
		Sector_1.BlockData.B_Dat_0[3] = CARD_USER_GROUP_POOL;
		Sector_3.BlockData.B_Dat_1[0] = 0x01;
		Sector_3.BlockData.B_Dat_1[1] = 0x06;
		Sector_3.BlockData.B_Dat_1[2] = 0x17;
		Sector_3.BlockData.B_Dat_1[3] = 0x12;
		Sector_3.BlockData.B_Dat_1[4] = 0x00;
		Sector_3.BlockData.B_Dat_1[5] = 0x00;
		Sector_3.BlockData.B_Dat_1[6] = 0x01;
		Sector_3.BlockData.B_Dat_1[7] = CARD_TYPE_ONE_TIME;
		Sector_3.BlockData.B_Dat_1[8] = 0xab;
		Sector_3.BlockData.B_Dat_1[9] = 0xba;
		Sector_3.BlockData.B_Dat_1[10] = CARD_TAG_TYPE_CLASIC;
		++card_write_cnt;
	}
	else if(card_write_cnt == 1)
	{
		Sector_1.BlockData.B_Dat_0[3] = CARD_USER_GROUP_POOL;
		Sector_3.BlockData.B_Dat_1[0] = 0x01;
		Sector_3.BlockData.B_Dat_1[1] = 0x06;
		Sector_3.BlockData.B_Dat_1[2] = 0x17;
		Sector_3.BlockData.B_Dat_1[3] = 0x12;
		Sector_3.BlockData.B_Dat_1[4] = 0x00;
		Sector_3.BlockData.B_Dat_1[5] = 0x00;
		Sector_3.BlockData.B_Dat_1[6] = 0x03;
		Sector_3.BlockData.B_Dat_1[7] = CARD_TYPE_MULTI;
		Sector_3.BlockData.B_Dat_1[8] = 0xab;
		Sector_3.BlockData.B_Dat_1[9] = 0xba;
		Sector_3.BlockData.B_Dat_1[10] = CARD_TAG_TYPE_CLASIC;
		++card_write_cnt;
	}
	else if(card_write_cnt == 2)
	{
		Sector_1.BlockData.B_Dat_0[2] = CARD_USER_GROUP_KINDERGARDEN;
		Sector_3.BlockData.B_Dat_0[0] = 0x01;
		Sector_3.BlockData.B_Dat_0[1] = 0x06;
		Sector_3.BlockData.B_Dat_0[2] = 0x17;
		Sector_3.BlockData.B_Dat_0[3] = 0x12;
		Sector_3.BlockData.B_Dat_0[4] = 0x00;
		Sector_3.BlockData.B_Dat_0[5] = 0x00;
		Sector_3.BlockData.B_Dat_0[6] = 0x01;
		Sector_3.BlockData.B_Dat_0[7] = CARD_TYPE_ONE_TIME;
		Sector_3.BlockData.B_Dat_0[8] = 0xab;
		Sector_3.BlockData.B_Dat_0[9] = 0xba;
		Sector_3.BlockData.B_Dat_0[10] = CARD_TAG_TYPE_CLASIC;
		++card_write_cnt;
	}
	else if(card_write_cnt == 3)
	{
		Sector_1.BlockData.B_Dat_0[2] = CARD_USER_GROUP_KINDERGARDEN;
		Sector_3.BlockData.B_Dat_0[0] = 0x01;
		Sector_3.BlockData.B_Dat_0[1] = 0x06;
		Sector_3.BlockData.B_Dat_0[2] = 0x17;
		Sector_3.BlockData.B_Dat_0[3] = 0x12;
		Sector_3.BlockData.B_Dat_0[4] = 0x00;
		Sector_3.BlockData.B_Dat_0[5] = 0x00;
		Sector_3.BlockData.B_Dat_0[6] = 0x03;
		Sector_3.BlockData.B_Dat_0[7] = CARD_TYPE_MULTI;
		Sector_3.BlockData.B_Dat_0[8] = 0xab;
		Sector_3.BlockData.B_Dat_0[9] = 0xba;
		Sector_3.BlockData.B_Dat_0[10] = CARD_TAG_TYPE_CLASIC;
		++card_write_cnt;
	}
#endif

    status = RC522_Request(PICC_REQIDL, aRC522_DataBuffer);	
    status = RC522_Anticoll(aRC522_DataBuffer);
	

    for(i = 0; i < 5; i++){	 

        aCardID[i]=aRC522_DataBuffer[i]; 
        
    }// End of for loop....

    status = RC522_SelectTag(aCardID);           
    status = RC522_Auth(PICC_AUTHENT1A, SECTOR_0, aMifareAuthenticationKeyA, aCardID);

    if(status == MI_OK){
        //sector 0
        //status = RC522_Write(Sector_0.BlockAddress.B_Add_0, Sector_0.BlockData.B_Dat_0);
        status = RC522_Write(Sector_0.BlockAddress.B_Add_1, Sector_0.BlockData.B_Dat_1);
        status = RC522_Write(Sector_0.BlockAddress.B_Add_2, Sector_0.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_0.BlockAddress.B_Add_3, Sector_0.BlockData.B_Dat_3);
        status = RC522_Auth(PICC_AUTHENT1A, SECTOR_1, aMifareAuthenticationKeyA, aCardID);
	}// End of if...

    if(status == MI_OK){
        //sector 1
        status = RC522_Write(Sector_1.BlockAddress.B_Add_0, Sector_1.BlockData.B_Dat_0);
        status = RC522_Write(Sector_1.BlockAddress.B_Add_1, Sector_1.BlockData.B_Dat_1);
        status = RC522_Write(Sector_1.BlockAddress.B_Add_2, Sector_1.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_1.BlockAddress.B_Add_3, Sector_1.BlockData.B_Dat_3);
        status = RC522_Auth(PICC_AUTHENT1A, SECTOR_2, aMifareAuthenticationKeyA, aCardID);
    }// End of if...
	
    if(status == MI_OK){
        
        status = RC522_Write(Sector_2.BlockAddress.B_Add_0, Sector_2.BlockData.B_Dat_0);
        status = RC522_Write(Sector_2.BlockAddress.B_Add_1, Sector_2.BlockData.B_Dat_1);
        status = RC522_Write(Sector_2.BlockAddress.B_Add_2, Sector_2.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_2.BlockAddress.B_Add_3, Sector_2.BlockData.B_Dat_3);
        status = RC522_Auth(PICC_AUTHENT1A, SECTOR_3, aMifareAuthenticationKeyA, aCardID);
    }// End of if...
    
     if(status == MI_OK){
        
        status = RC522_Write(Sector_3.BlockAddress.B_Add_0, Sector_3.BlockData.B_Dat_0);
        status = RC522_Write(Sector_3.BlockAddress.B_Add_1, Sector_3.BlockData.B_Dat_1);
        status = RC522_Write(Sector_3.BlockAddress.B_Add_2, Sector_3.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_3.BlockAddress.B_Add_3, Sector_3.BlockData.B_Dat_3);
        status = RC522_Auth(PICC_AUTHENT1A, SECTOR_4, aMifareAuthenticationKeyA, aCardID);  
    }// End of if...
     
     if(status == MI_OK){
        
        status = RC522_Write(Sector_4.BlockAddress.B_Add_0, Sector_4.BlockData.B_Dat_0);
        status = RC522_Write(Sector_4.BlockAddress.B_Add_1, Sector_4.BlockData.B_Dat_1);
        status = RC522_Write(Sector_4.BlockAddress.B_Add_2, Sector_4.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_4.BlockAddress.B_Add_3, Sector_4.BlockData.B_Dat_3);
        status = RC522_Auth(PICC_AUTHENT1A, SECTOR_5, aMifareAuthenticationKeyA, aCardID);   
    }// End of if...
     
     if(status == MI_OK){
        
        status = RC522_Write(Sector_5.BlockAddress.B_Add_0, Sector_5.BlockData.B_Dat_0);
        status = RC522_Write(Sector_5.BlockAddress.B_Add_1, Sector_5.BlockData.B_Dat_1);
        status = RC522_Write(Sector_5.BlockAddress.B_Add_2, Sector_5.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_5.BlockAddress.B_Add_3, Sector_5.BlockData.B_Dat_3);
        status = RC522_Auth(PICC_AUTHENT1A, SECTOR_6, aMifareAuthenticationKeyA, aCardID);    
    }// End of if...
     
     if(status == MI_OK){
        
        status = RC522_Write(Sector_6.BlockAddress.B_Add_0, Sector_6.BlockData.B_Dat_0);
        status = RC522_Write(Sector_6.BlockAddress.B_Add_1, Sector_6.BlockData.B_Dat_1);
        status = RC522_Write(Sector_6.BlockAddress.B_Add_2, Sector_6.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_6.BlockAddress.B_Add_3, Sector_6.BlockData.B_Dat_3);
        status = RC522_Auth(PICC_AUTHENT1A, SECTOR_7, aMifareAuthenticationKeyA, aCardID);
    
    }// End of if...
     
     if(status == MI_OK){
        
        status = RC522_Write(Sector_7.BlockAddress.B_Add_0, Sector_7.BlockData.B_Dat_0);
        status = RC522_Write(Sector_7.BlockAddress.B_Add_1, Sector_7.BlockData.B_Dat_1);
        status = RC522_Write(Sector_7.BlockAddress.B_Add_2, Sector_7.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_7.BlockAddress.B_Add_3, Sector_7.BlockData.B_Dat_3);
        status = RC522_Auth(PICC_AUTHENT1A, SECTOR_8, aMifareAuthenticationKeyA, aCardID);  
    }// End of if...
     
     if(status == MI_OK){
        
        status = RC522_Write(Sector_8.BlockAddress.B_Add_0, Sector_8.BlockData.B_Dat_0);
        status = RC522_Write(Sector_8.BlockAddress.B_Add_1, Sector_8.BlockData.B_Dat_1);
        status = RC522_Write(Sector_8.BlockAddress.B_Add_2, Sector_8.BlockData.B_Dat_2);
        //status = RC522_Write(Sector_8.BlockAddress.B_Add_3, Sector_8.BlockData.B_Dat_3);     
    }// End of if...

    RC522_Halt();
	
	BarrierProcessTimerStart(BARRIER_BRAKE_TIME);
	BarrierLedSignal_AllOff();
	ExitRedSignal_On();
	DOUT_Service();
            
}// End of write card function

RC522_StatusTypeDef RC522_VerifyData(void)
{
	uint8_t b_cnt;
	
	b_cnt = 0;
	/*
	*	copy readed card serial to card data buffer
	*/
	for(b_cnt = 0; b_cnt < 5; b_cnt++)
	{
		sCardData.aUserCardID[b_cnt] = aCardSerial[b_cnt];
	}
	/*
	*	search for valid user group in readed card data
	*/
	if (Sector_1.BlockData.B_Dat_0[2] == CARD_USER_GROUP_KINDERGARDEN)
	{
		sCardData.card_user_group = CARD_USER_GROUP_KINDERGARDEN;
		RC522_CheckCardData(&CARD_KINDERGARDEN_EXPIRY_TIME_ADDRESS);
	}
	else if (Sector_1.BlockData.B_Dat_0[3] == CARD_USER_GROUP_POOL)
	{
		sCardData.card_user_group = CARD_USER_GROUP_POOL;
		RC522_CheckCardData(&CARD_POOL_EXPIRY_TIME_ADDRESS);
	}
	else
	{
		sCardData.card_user_group = CARD_USER_GROUP_DATA_INVALID;
		sCardData.card_status = CARD_INVALID;
	}
	/*
	*	check card status
	*/	
	if ((sCardData.card_user_group == CARD_USER_GROUP_INVALID) || (sCardData.card_user_group == CARD_USER_GROUP_DATA_INVALID) 			\
		|| (sCardData.card_usage_type == CARD_TYPE_INVALID) || (sCardData.card_usage_type == CARD_TYPE_DATA_INVALID)			\
		|| (sCardData.aCardExpiryTime[0] == CARD_EXPIRY_TIME_DATA_INVALID) || (sCardData.aCardExpiryTime[1] == CARD_EXPIRY_TIME_DATA_INVALID) 	\
		|| (sCardData.aCardExpiryTime[2] == CARD_EXPIRY_TIME_DATA_INVALID) || (sCardData.aCardExpiryTime[3] == CARD_EXPIRY_TIME_DATA_INVALID) 	\
		|| (sCardData.aCardExpiryTime[4] == CARD_EXPIRY_TIME_DATA_INVALID) || (sCardData.aCardExpiryTime[5] == CARD_EXPIRY_TIME_DATA_INVALID) 	\
		|| (sCardData.card_number_of_users == CARD_NUMBER_OF_USERS_INVALID) || (sCardData.card_number_of_users == CARD_NUMBER_OF_USERS_DATA_INVALID)	\
		|| (sCardData.card_tag_type == CARD_TAG_TYPE_INVALID) || (sCardData.card_tag_type == CARD_TAG_TYPE_DATA_INVALID)) \
	 {
		 sCardData.card_status = CARD_INVALID;
	 }
	 else if (sCardData.aCardExpiryTime[5] == CARD_EXPIRY_TIME_INVALID)
	 {
		 sCardData.card_status = CARD_EXPIRY_TIME_INVALID;
	 }
	 else sCardData.card_status = CARD_VALID;
	
	return (MI_OK);
}


void RC522_CheckCardData(uint8_t *pbuf)
{
	uint8_t j;
	
	/*
	*	check card date time data
	*/
	RTC_GetTime(RTC_Format_BCD, &RTC_Time);
	RTC_GetDate(RTC_Format_BCD, &RTC_Date);
			
	if (((pbuf[0] >> 4) > 0x03) || (((pbuf[0] >> 4) ==  0x00) && ((pbuf[0] & 0x0f) == 0x00)) || ((pbuf[0] & 0x0f) > 0x09))
	{
		for(j = 0; j < 6; j++)
		{
			sCardData.aCardExpiryTime[j] = CARD_EXPIRY_TIME_DATA_INVALID;
	
		}
	}
	else if (((pbuf[1] >> 4) > 0x02) || (((pbuf[1] >> 4) == 0x00) && ((pbuf[1] & 0x0f) == 0x00)) || ((pbuf[1] & 0x0f) > 0x09))
	{
		for(j = 0; j < 6; j++)
		{
			sCardData.aCardExpiryTime[j] = CARD_EXPIRY_TIME_DATA_INVALID;
	
		}
	}
	else if (((pbuf[2] >> 4) > 0x09) || ((pbuf[2] & 0x0f) > 0x09))
	{
		for(j = 0; j < 6; j++)
		{
			sCardData.aCardExpiryTime[j] = CARD_EXPIRY_TIME_DATA_INVALID;
	
		}
	}
	else if (((pbuf[3] >> 4) > 0x02) || ((pbuf[3] & 0x0f) > 0x09))
	{
		for(j = 0; j < 6; j++)
		{
			sCardData.aCardExpiryTime[j] = CARD_EXPIRY_TIME_DATA_INVALID;
	
		}
	}
	else if (((pbuf[4] >> 4) > 0x05) || ((pbuf[4] & 0x0f) > 0x09))
	{
		for(j = 0; j < 6; j++)
		{
			sCardData.aCardExpiryTime[j] = CARD_EXPIRY_TIME_DATA_INVALID;
	
		}
	}
	else
	{
		if((pbuf[2] > RTC_Date.RTC_Year) \
			|| ((pbuf[2] == RTC_Date.RTC_Year) && (pbuf[1] > RTC_Date.RTC_Month))	\
			|| ((pbuf[2] == RTC_Date.RTC_Year) && (pbuf[1] == RTC_Date.RTC_Month) && (pbuf[0] > RTC_Date.RTC_Date))	\
			|| ((pbuf[2] == RTC_Date.RTC_Year) && (pbuf[1] == RTC_Date.RTC_Month) && (pbuf[0] == RTC_Date.RTC_Date) && (pbuf[3] > RTC_Time.RTC_Hours))	\
			|| ((pbuf[2] == RTC_Date.RTC_Year) && (pbuf[1] == RTC_Date.RTC_Month) && (pbuf[0] == RTC_Date.RTC_Date) && (pbuf[3] == RTC_Time.RTC_Hours) && (pbuf[4] >= RTC_Time.RTC_Minutes)))
		{
			for(j = 0; j < 6; j++)
			{
				sCardData.aCardExpiryTime[j] = pbuf[j];	
			}	
		}
		else
		{
			for(j = 0; j < 6; j++)
			{
				sCardData.aCardExpiryTime[j] = pbuf[j];	
			}
			
			sCardData.aCardExpiryTime[5] = CARD_EXPIRY_TIME_INVALID;
		}
	}
	/*
	*	check card number of user data
	*/
	if((pbuf[6] == 0) || (pbuf[6] == CARD_DATA_FORMATED))
	{
		sCardData.card_number_of_users = CARD_NUMBER_OF_USERS_INVALID;		
	}
	else if(pbuf[6] > CARD_MAX_NUMBER_OF_USERS)
	{
		sCardData.card_number_of_users = CARD_NUMBER_OF_USERS_DATA_INVALID;
	}
	else 
	{
		sCardData.card_number_of_users = pbuf[6];		
	}
	/*
	*	check card type data
	*/
	if((pbuf[7] == 0) || (pbuf[7] == CARD_DATA_FORMATED))
	{
		sCardData.card_usage_type = CARD_TYPE_INVALID;		
	}
	else if ((pbuf[7] != CARD_TYPE_ONE_TIME) && (pbuf[7] != CARD_TYPE_MULTI))
	{
		sCardData.card_usage_type = CARD_TYPE_DATA_INVALID;
	}
	else 
	{
		sCardData.card_usage_type = pbuf[7];		
	}
	/*
	*	check card system id data
	*/
	if(((pbuf[8] == 0) && (pbuf[9] == 0)) || ((pbuf[8] == CARD_DATA_FORMATED) && (pbuf[9] == CARD_DATA_FORMATED)))
	{
		sCardData.system_id = CARD_SYSTEM_ID_INVALID;		
	}
	else if ((pbuf[8] != (DEFAULT_SYSTEM_ID >> 8)) && (pbuf[9] != (DEFAULT_SYSTEM_ID & 0xff)))
	{
		sCardData.system_id = CARD_SYSTEM_ID_DATA_INVALID;
	}
	else 
	{
		sCardData.system_id = (pbuf[8] << 8) + pbuf[9];		
	}
	/*
	*	check card tag type
	*/
	if((pbuf[10] == 0) || (pbuf[10] == CARD_DATA_FORMATED))
	{
		sCardData.card_tag_type = CARD_TAG_TYPE_INVALID;		
	}
	else if ((pbuf[10] != CARD_TAG_TYPE_KEY_RING) && (pbuf[10] != CARD_TAG_TYPE_CLASIC) && (pbuf[10] != CARD_TAG_TYPE_WRIST))
	{
		sCardData.card_tag_type = CARD_TAG_TYPE_DATA_INVALID;
	}
	else 
	{
		sCardData.card_tag_type = pbuf[10];		
	}
}



void RC522_ClearData(void)
{
	uint8_t i;
	
	for(i = 0; i < 16; i++)
	{
		Sector_0.BlockData.B_Dat_0[i] = 0;
		Sector_0.BlockData.B_Dat_1[i] = 0;
		Sector_0.BlockData.B_Dat_2[i] = 0;
		Sector_0.BlockData.B_Dat_3[i] = 0;
		
		Sector_1.BlockData.B_Dat_0[i] = 0;
		Sector_1.BlockData.B_Dat_1[i] = 0;
		Sector_1.BlockData.B_Dat_2[i] = 0;
		Sector_1.BlockData.B_Dat_3[i] = 0;
		
		Sector_2.BlockData.B_Dat_0[i] = 0;
		Sector_2.BlockData.B_Dat_1[i] = 0;
		Sector_2.BlockData.B_Dat_2[i] = 0;
		Sector_2.BlockData.B_Dat_3[i] = 0;
		
		Sector_3.BlockData.B_Dat_0[i] = 0;
		Sector_3.BlockData.B_Dat_1[i] = 0;
		Sector_3.BlockData.B_Dat_2[i] = 0;
		Sector_3.BlockData.B_Dat_3[i] = 0;
		
		Sector_4.BlockData.B_Dat_0[i] = 0;
		Sector_4.BlockData.B_Dat_1[i] = 0;
		Sector_4.BlockData.B_Dat_2[i] = 0;
		Sector_4.BlockData.B_Dat_3[i] = 0;
		
		Sector_5.BlockData.B_Dat_0[i] = 0;
		Sector_5.BlockData.B_Dat_1[i] = 0;
		Sector_5.BlockData.B_Dat_2[i] = 0;
		Sector_5.BlockData.B_Dat_3[i] = 0;
		
		Sector_6.BlockData.B_Dat_0[i] = 0;
		Sector_6.BlockData.B_Dat_1[i] = 0;
		Sector_6.BlockData.B_Dat_2[i] = 0;
		Sector_6.BlockData.B_Dat_3[i] = 0;
		
		Sector_7.BlockData.B_Dat_0[i] = 0;
		Sector_7.BlockData.B_Dat_1[i] = 0;
		Sector_7.BlockData.B_Dat_2[i] = 0;
		Sector_7.BlockData.B_Dat_3[i] = 0;
		
		Sector_8.BlockData.B_Dat_0[i] = 0;
		Sector_8.BlockData.B_Dat_1[i] = 0;
		Sector_8.BlockData.B_Dat_2[i] = 0;
		Sector_8.BlockData.B_Dat_3[i] = 0;
		
		Sector_9.BlockData.B_Dat_0[i] = 0;
		Sector_9.BlockData.B_Dat_1[i] = 0;
		Sector_9.BlockData.B_Dat_2[i] = 0;
		Sector_9.BlockData.B_Dat_3[i] = 0;
		
		Sector_10.BlockData.B_Dat_0[i] = 0;
		Sector_10.BlockData.B_Dat_1[i] = 0;
		Sector_10.BlockData.B_Dat_2[i] = 0;
		Sector_10.BlockData.B_Dat_3[i] = 0;
		
		Sector_11.BlockData.B_Dat_0[i] = 0;
		Sector_11.BlockData.B_Dat_1[i] = 0;
		Sector_11.BlockData.B_Dat_2[i] = 0;
		Sector_11.BlockData.B_Dat_3[i] = 0;
		
		Sector_12.BlockData.B_Dat_0[i] = 0;
		Sector_12.BlockData.B_Dat_1[i] = 0;
		Sector_12.BlockData.B_Dat_2[i] = 0;
		Sector_12.BlockData.B_Dat_3[i] = 0;
		
		Sector_13.BlockData.B_Dat_0[i] = 0;
		Sector_13.BlockData.B_Dat_1[i] = 0;
		Sector_13.BlockData.B_Dat_2[i] = 0;
		Sector_13.BlockData.B_Dat_3[i] = 0;
		
		Sector_14.BlockData.B_Dat_0[i] = 0;
		Sector_14.BlockData.B_Dat_1[i] = 0;
		Sector_14.BlockData.B_Dat_2[i] = 0;
		Sector_14.BlockData.B_Dat_3[i] = 0;
		
		Sector_15.BlockData.B_Dat_0[i] = 0;
		Sector_15.BlockData.B_Dat_1[i] = 0;
		Sector_15.BlockData.B_Dat_2[i] = 0;
		Sector_15.BlockData.B_Dat_3[i] = 0;	
	}
	
	sCardData.card_status = 0;
	sCardData.card_user_group = 0;
	sCardData.card_usage_type = 0;
	sCardData.card_number_of_users = 0;
	sCardData.system_id = 0;
	sCardData.card_tag_type = 0;
	
	i = 0;
	while (i < 5) sCardData.aUserCardID[i++] = 0;
	i = 0;
	while (i < 6) sCardData.aCardExpiryTime[i++] = 0;
}

void RCC522_InitSPI(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI5, &SPI_InitStructure); 
	SPI_Cmd(SPI5, ENABLE);	
}


