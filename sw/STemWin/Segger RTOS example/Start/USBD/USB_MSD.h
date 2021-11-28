/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2016  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : USB_MSD.h
Purpose : Public header of the mass storage device client
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef MSD_H          /* Avoid multiple inclusion */
#define MSD_H

#include "SEGGER.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Config defaults
*
**********************************************************************
*/

#ifndef   MSD_DEBUG_LEVEL
  #define MSD_DEBUG_LEVEL 0
#endif

#ifndef   MSD_USE_PARA
  #define MSD_USE_PARA(para) para = para
#endif

#ifndef   USB_MSD_MAX_UNIT
  #define USB_MSD_MAX_UNIT 4
#endif

#ifndef   MSD_USB_CLASS
  #define MSD_USB_CLASS     8       // 8: Mass storage
#endif

#ifndef   MSD_USB_SUBCLASS
  #define MSD_USB_SUBCLASS  6       // 1: RBC (reduced SCSI) 2: ATAPI, 3: QIC 157, 4: UFI, 6: SCSI
#endif

#ifndef   MSD_USB_PROTOCOL
  #define MSD_USB_PROTOCOL  0x50    // 0x50: BOT (Bulk-only-transport)
#endif

#ifndef USB_V2_V3_MIGRATION_MSD_LUN_INFO
  #define USB_V2_V3_MIGRATION_MSD_LUN_INFO   0
#endif

/*********************************************************************
*
*       Define non-configurable
*
**********************************************************************
*/
#define USB_MSD_MT_WRITE_INFO_SIZE         sizeof(USB_MSD_MT_WRITE_INFO)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct _LUN_INFO LUN_INFO;

typedef void (PREVENT_ALLOW_REMOVAL_HOOK)(U8 PreventRemoval);
typedef void (PREVENT_ALLOW_REMOVAL_HOOK_EX)(U8 Lun, U8 PreventRemoval);
typedef void (START_STOP_UNIT_HOOK)(U8 Lun, U8 StartLoadEject);
typedef void (READ_WRITE_HOOK)(U8 Lun, U8 IsRead, U8 OnOff, U32 StartLBA, U32 NumBlocks);
typedef U8   (USB_MSD_HANDLE_CMD)(LUN_INFO * pLUNInfo, U8 * pCmdBlock, U32 * pNumBytes);

/*********************************************************************
*
*       Storage interface
*/
typedef struct {
  U32 NumSectors;
  U16 SectorSize;
} USB_MSD_INFO;


/*********************************************************************
*
*       Storage interface
*/
typedef struct {
  char * pVendorName;
  char * pProductName;
  char * pProductVer;
  char * pSerialNo;
} USB_MSD_LUN_INFO;

typedef struct {
  U8 EPIn;
  U8 EPOut;
  U8 InterfaceNum;
} USB_MSD_INIT_DATA;

typedef struct {
  void     * pStart;
  U32        StartSector;
  U32        NumSectors;
  U16        SectorSize;
  void     * pSectorBuffer;
  unsigned   NumBytes4Buffer;
  U8         NumBuffers;
} USB_MSD_INST_DATA_DRIVER;

typedef struct {
  void       (*pfInit)           (U8 Lun, const USB_MSD_INST_DATA_DRIVER * pDriverData);
  void       (*pfGetInfo)        (U8 Lun, USB_MSD_INFO * pInfo);
  U32        (*pfGetReadBuffer)  (U8 Lun, U32 SectorIndex,       void ** ppData, U32 NumSectors);
  char       (*pfRead)           (U8 Lun, U32 SectorIndex,       void *   pData, U32 NumSectors);
  U32        (*pfGetWriteBuffer) (U8 Lun, U32 SectorIndex,       void ** ppData, U32 NumSectors);
  char       (*pfWrite)          (U8 Lun, U32 SectorIndex, const void *   pData, U32 NumSectors);
  char       (*pfMediumIsPresent)(U8 Lun);
  void       (*pfDeInit)         (U8 Lun);
} USB_MSD_STORAGE_API;

typedef struct {
  const USB_MSD_STORAGE_API * pAPI;
  USB_MSD_INST_DATA_DRIVER    DriverData;
  U8                          DeviceType;      // 0: Direct access block device ... 5: CD/DVD
  U8                          IsPresent;
  USB_MSD_HANDLE_CMD        * pfHandleCmd;
  U8                          IsWriteProtected;
  const USB_MSD_LUN_INFO    * pLunInfo;
} USB_MSD_INST_DATA;

typedef struct {
  U32         SectorIndex;
  U32         NumSectors;
} USB_MSD_MT_WRITE_INFO;


/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void USBD_MSD_Init    (void);
void USBD_MSD_Add     (const USB_MSD_INIT_DATA * pInitData);
void USBD_MSD_AddUnit (const USB_MSD_INST_DATA * pInstData);
void USBD_MSD_AddCDRom(const USB_MSD_INST_DATA * pInstData);

void USBD_MSD_SetPreventAllowRemovalHook  (U8 Lun, PREVENT_ALLOW_REMOVAL_HOOK    * pfOnPreventAllowRemoval);
void USBD_MSD_SetPreventAllowRemovalHookEx(U8 Lun, PREVENT_ALLOW_REMOVAL_HOOK_EX * pfOnPreventAllowRemoval);
void USBD_MSD_SetStartStopUnitHook        (U8 Lun, START_STOP_UNIT_HOOK          * pfOnStartStopUnit);
void USBD_MSD_SetReadWriteHook            (U8 Lun, READ_WRITE_HOOK               * pfOnReadWrite);

void USBD_MSD_Task   (void);

#if USB_V2_V3_MIGRATION_MSD_LUN_INFO > 0
const char * USB_MSD_GetProductVer (U8 Lun);
const char * USB_MSD_GetProductName(U8 Lun);
const char * USB_MSD_GetVendorName (U8 Lun);
const char * USB_MSD_GetSerialNo   (U8 Lun);
#endif

void USBD_MSD_RequestDisconnect   (U8 Lun);
void USBD_MSD_Disconnect          (U8 Lun);
int  USBD_MSD_WaitForDisconnection(U8 Lun, U32 TimeOut);
void USBD_MSD_Connect             (U8 Lun);
void USBD_MSD_UpdateWriteProtect  (U8 Lun, U8 IsWriteProtected);
void USBD_MSD_UpdateSenseInfo     (U8 Lun, U8 SenseKey, U8 AddSenseCode, U8 AddSenseCodeQualifier);

/*********************************************************************
*
*       API functions for the multi-task storage
*
**********************************************************************
*/
void USBD_MSD_StorageTask(void);
void USBD_MSD_Storage_MTInit(void);

/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/
#define  USB_MSD_Init                                   USBD_MSD_Init
#define  USB_MSD_Add                                    USBD_MSD_Add
#define  USB_MSD_AddUnit                                USBD_MSD_AddUnit
#define  USB_MSD_AddCDRom                               USBD_MSD_AddCDRom
#define  USB_MSD_SetPreventAllowRemovalHook             USBD_MSD_SetPreventAllowRemovalHook
#define  USB_MSD_SetPreventAllowRemovalHookEx           USBD_MSD_SetPreventAllowRemovalHookEx
#define  USB_MSD_SetStartStopUnitHook                   USBD_MSD_SetStartStopUnitHook
#define  USB_MSD_SetReadWriteHook                       USBD_MSD_SetReadWriteHook
#define  USB_MSD_Task                                   USBD_MSD_Task
#define  USB_MSD_GetProductVer                          USBD_MSD_GetProductVer
#define  USB_MSD_GetProductName                         USBD_MSD_GetProductName
#define  USB_MSD_GetVendorName                          USBD_MSD_GetVendorName
#define  USB_MSD_GetSerialNo                            USBD_MSD_GetSerialNo
#define  USB_MSD_RequestDisconnect                      USBD_MSD_RequestDisconnect
#define  USB_MSD_Disconnect                             USBD_MSD_Disconnect
#define  USB_MSD_WaitForDisconnection                   USBD_MSD_WaitForDisconnection
#define  USB_MSD_Connect                                USBD_MSD_Connect
#define  USB_MSD_UpdateWriteProtect                     USBD_MSD_UpdateWriteProtect
#define  USB_MSD_UpdateSenseInfo                        USBD_MSD_UpdateSenseInfo
#define  USB_MSD_StorageTask                            USBD_MSD_StorageTask
#define  USB_MSD_Storage_MTInit(x)                      USBD_MSD_Storage_MTInit()

/*********************************************************************
*
*       Storage interface
*
**********************************************************************
*/

extern const USB_MSD_STORAGE_API USB_MSD_StorageRAM;
extern const USB_MSD_STORAGE_API USB_MSD_StorageByIndex;
extern const USB_MSD_STORAGE_API USB_MSD_StorageByName;
extern const USB_MSD_STORAGE_API USB_MSD_StorageTrim;
extern const USB_MSD_STORAGE_API USB_MSD_StorageMT;


#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/

