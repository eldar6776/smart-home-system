/**
 ******************************************************************************
 * File Name          : rc522.h
 * Date               : 08/05/2016 23:15:16
 * Description        : mifare RC522 modul header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#ifndef RC522_H
#define RC522_H   						204	// version 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

//#define RC522_CARD_WRITE_INIT

/* Exported defines    -------------------------------------------------------*/
#define DEFAULT_SYSTEM_ID					0xabba
#define RC522_SYSTEM_INIT_DISABLE_TIME		8901	// 8 s on power up reader is disbled 
#define RC522_CARD_VALID_EVENT_TIME			3210	// 3 s reader unlisten time after card read
#define RC522_CARD_INVALID_EVENT_TIME		1234	// 1 s reader unlisten time after card read
#define RC522_PROCESS_LOOP_TIME				89		// 89 ms x 3 reader = 267 ms rfid card scan time
#define RC522_BUFFER_SIZE					16      // FIFO & buffer byte lenght
#define RC522_BLOCK_SIZE              		16      // 16 byte in block 
#define RC522_CARD_VALID_EVENT_TIME			3210	// 3 s reader unlisten time after card read
#define RC522_CARD_INVALID_EVENT_TIME		1234	// 1 s reader unlisten time after card read
/**
*-----------------        card data status     --------------------------
*/
#define CARD_PENDING						(NULL)
#define CARD_VALID							(0x06)
#define CARD_INVALID						(0x15)
#define CARD_DATA_FORMATED					(0x7f)
/**
*--------------      card user groups predefine     ---------------------
*/
#define CARD_USER_GROUP_GUEST				('G')
#define CARD_USER_GROUP_HANDMAID			('H')
#define CARD_USER_GROUP_MANAGER				('M')
#define CARD_USER_GROUP_SERVICE				('S')
#define CARD_USER_GROUP_PRESET				('P')
#define CARD_USER_GROUP_KINDERGARDEN		('K')
#define CARD_USER_GROUP_POOL				('B')
#define CARD_USER_GROUP_PARKING				('R')
/**
*--------------      card type predefine     ---------------------
*/
#define CARD_TYPE_ONE_TIME					('O')
#define CARD_TYPE_MULTI						('E')
#define CARD_TAG_TYPE_KEY_RING				('Q')
#define CARD_TAG_TYPE_CLASIC				('C')
#define CARD_TAG_TYPE_WRIST					('W')
/**
*--------------   card data invalid date predefine    -------------------
*/
#define CARD_ID_INVALID						(':')
#define CARD_ID_INVALID_DATA				(';')
#define CARD_USER_GROUP_INVALID				('<')	
#define CARD_USER_GROUP_DATA_INVALID		('=')
#define CARD_EXPIRY_TIME_INVALID			('>')
#define CARD_EXPIRY_TIME_DATA_INVALID		('?')
#define CARD_CONTROLLER_ID_INVALID			('.')
#define CARD_CONTROLLER_ID_DATA_INVALID		('/')
#define CARD_SYSTEM_ID_INVALID				('{')
#define CARD_SYSTEM_ID_DATA_INVALID			('}')
#define CARD_TYPE_INVALID					('q')
#define CARD_TYPE_DATA_INVALID				('?')
#define CARD_NUMBER_OF_USERS_INVALID		('w')
#define CARD_NUMBER_OF_USERS_DATA_INVALID	('?')
#define CARD_TAG_TYPE_INVALID				('x')
#define CARD_TAG_TYPE_DATA_INVALID			('?')
#define CARD_MAX_NUMBER_OF_USERS			100

/**
*---------------     card data predefined addresse    --------------------
*/
#define CARD_USER_FIRST_NAME_ADDRESS				(Sector_0.BlockData.B_Dat_1[0])
#define CARD_USER_LAST_NAME_ADDRESS					(Sector_0.BlockData.B_Dat_2[0])
#define CARD_USER_GROUP_ADDRESS						(Sector_1.BlockData.B_Dat_0[0])
#define CARD_SYSTEM_ID_ADDRESS						(Sector_1.BlockData.B_Dat_1[0])
#define CARD_EXPIRY_TIME_ADDRESS					(Sector_2.BlockData.B_Dat_0[0])
#define CARD_CTRL_ID_ADDRESS						(Sector_2.BlockData.B_Dat_0[6])

#define CARD_KINDERGARDEN_EXPIRY_TIME_ADDRESS		(Sector_3.BlockData.B_Dat_0[0])
#define CARD_KINDERGARDEN_NUMBER_OF_USERS_ADDRESS	(Sector_3.BlockData.B_Dat_0[6])
#define CARD_KINDERGARDEN_TYPE_ADDRESS				(Sector_3.BlockData.B_Dat_0[7])
#define CARD_KINDERGARDEN_SYSTEM_ID					(Sector_3.BlockData.B_Dat_0[8])
#define CARD_KINDERGARDEN_TAG_TYPE_ADDRESS			(Sector_3.BlockData.B_Dat_0[10])

#define CARD_POOL_EXPIRY_TIME_ADDRESS				(Sector_3.BlockData.B_Dat_1[0])
#define CARD_POOL_NUMBER_OF_USERS_ADDRESS			(Sector_3.BlockData.B_Dat_1[6])
#define CARD_POOL_TYPE_ADDRESS						(Sector_3.BlockData.B_Dat_1[7])
#define CARD_POOL_SYSTEM_ID							(Sector_3.BlockData.B_Dat_1[8])
#define CARD_POOL_TAG_TYPE_ADDRESS					(Sector_3.BlockData.B_Dat_1[10])


/* Exported types    ---------------------------------------------------------*/
typedef struct
{
	uint8_t card_reader;
	uint8_t card_status;
	uint8_t aUserCardID[5];
	uint8_t card_user_group;
	uint8_t card_usage_type;
	uint8_t card_number_of_users;
	uint8_t aCardExpiryTime[6];
	uint16_t system_id;
	uint8_t card_tag_type;
	
}RC522_CardDataTypeDef;

/* Exported variables  -------------------------------------------------------*/
extern uint8_t rc522_config;
extern volatile uint32_t mifare_process_timer;
extern volatile uint32_t mifare_process_flags;

extern uint8_t aMifareAuthenticationKeyA[6];
extern uint8_t aMifareAuthenticationKeyB[6];

extern RC522_CardDataTypeDef sCardData;

/* Exported macros     -------------------------------------------------------*/
#define RC522_StartTimer(TIME)			((mifare_process_timer = TIME), (mifare_process_flags &= 0xfffffffe))
#define RC522_StopTimer()				(mifare_process_flags |= 0x00000001)
#define IsRC522_TimerExpired()			(mifare_process_flags &  0x00000001)


/* Exported functions  -------------------------------------------------------*/
extern void RC522_Init(void);
extern void RC522_Service(void);

#endif
/******************************   END OF FILE  **********************************/
