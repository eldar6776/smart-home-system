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
File    : USB_SmartMSD.h
Purpose : USB_SmartMSD API specification
--------- END-OF-HEADER ----------------------------------------------
*/

#ifndef USB_SMARTMSD_H            // Avoid multiple inclusion
#define USB_SMARTMSD_H

#include "Global.h"
#include "USB_MSD.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define USB_SMARTMSD_ATTR_READ_ONLY       (0x01)
#define USB_SMARTMSD_ATTR_HIDDEN          (0x02)
#define USB_SMARTMSD_ATTR_SYSTEM          (0x04)
#define USB_SMARTMSD_ATTR_VOLUME_ID       (0x08)
#define USB_SMARTMSD_ATTR_DIRECTORY       (0x10)
#define USB_SMARTMSD_ATTR_ARCHIVE         (0x20)
#define USB_SMARTMSD_ATTR_LONG_NAME       (USB_SMARTMSD_ATTR_READ_ONLY | USB_SMARTMSD_ATTR_HIDDEN | USB_SMARTMSD_ATTR_SYSTEM | USB_SMARTMSD_ATTR_VOLUME_ID)
#define USB_SMARTMSD_ATTR_LONG_NAME_MASK  (USB_SMARTMSD_ATTR_READ_ONLY | USB_SMARTMSD_ATTR_HIDDEN | USB_SMARTMSD_ATTR_SYSTEM | USB_SMARTMSD_ATTR_VOLUME_ID | USB_SMARTMSD_ATTR_DIRECTORY | USB_SMARTMSD_ATTR_ARCHIVE)

//
// For use in USB_SMARTMSD_CONST_FILE.Flags
//
#define USB_SMARTMSD_FILE_WRITABLE        USB_SMARTMSD_ATTR_READ_ONLY
#define USB_SMARTMSD_FILE_AHEAD           (1<<8)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  const char* sName;
  const U8* pData;
  int FileSize;
  U32 Flags;
} USB_SMARTMSD_CONST_FILE;

typedef struct {
  U8  acFilename[8];
  U8  acExt[3];
  U8  DirAttr;
  U8  NTRes;
  U8  CrtTimeTenth;
  U16 CrtTime;
  U16 CrtDate;
  U16 LstAccDate;
  U16 FstClusHI;
  U16 WrtTime;
  U16 WrtDate;
  U16 FstClusLO;
  U32 FileSize;
} USB_SMARTMSD_DIR_ENTRY_SHORT;

typedef struct {
  U8  Ord;
  U8  acName1[10];
  U8  Attr;
  U8  Type;
  U8  Chksum;
  U8  acName2[12];
  U16 FstClusLO;
  U8  acName3[4];
} USB_SMARTMSD_DIR_ENTRY_LONG;

typedef union {
  USB_SMARTMSD_DIR_ENTRY_SHORT ShortEntry;
  USB_SMARTMSD_DIR_ENTRY_LONG  LongEntry;
  U8                       ac[32];
} USB_SMARTMSD_DIR_ENTRY;

typedef struct {
  const USB_SMARTMSD_DIR_ENTRY* pDirEntry;
} USB_SMARTMSD_FILE_INFO;

typedef int    USB_SMARTMSD_ON_READ_FUNC  (      U8   * pData, U32 Off, U32 NumBytes, const USB_SMARTMSD_FILE_INFO * pFile);
typedef int    USB_SMARTMSD_ON_WRITE_FUNC (const U8   * pData, U32 Off, U32 NumBytes, const USB_SMARTMSD_FILE_INFO * pFile);
typedef void * USB_SMARTMSD_MEM_ALLOC     (U32          Size);
typedef void   USB_SMARTMSD_MEM_FREE      (void       * p);
typedef void   USB_SMARTMSD_ON_PANIC      (const char * sErr);

typedef struct _USB_SMARTMSD_USER_FUNC_API {
  USB_SMARTMSD_ON_READ_FUNC  * pfOnReadSector;             // Mandatory
  USB_SMARTMSD_ON_WRITE_FUNC * pfOnWriteSector;            // Mandatory
  USB_SMARTMSD_ON_PANIC      * pfOnPanic;                  // Optional
  USB_SMARTMSD_MEM_ALLOC     * pfMemAlloc;                 // Optional
  USB_SMARTMSD_MEM_FREE      * pfMemFree;                  // Optional
} USB_SMARTMSD_USER_FUNC_API;

/*********************************************************************
*
*       Const data
*
**********************************************************************
*/
extern const USB_MSD_STORAGE_API USB_MSD_StorageSmartMSD;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
//
// Configuration functions that can be called within the USB_SmartMSD_X_Config function
//
void USB_SmartMSD_AssignMemory          (U32 * p, U32 NumBytes);
void USB_SmartMSD_SetUserFunc           (const USB_SMARTMSD_USER_FUNC_API         * pUserFunc);
void USB_SmartMSD_SetNumRootDirSectors  (unsigned Lun, int                          NumRootDirSectors);
int  USB_SmartMSD_SetVolumeInfo         (unsigned Lun, const char                 * sVolumeName, const USB_MSD_LUN_INFO * pLunInfo);
void USB_SmartMSD_SetcbRead             (unsigned Lun, USB_SMARTMSD_ON_READ_FUNC  * pfReadSector);
void USB_SmartMSD_SetcbWrite            (unsigned Lun, USB_SMARTMSD_ON_WRITE_FUNC * pfWriteSector);
int  USB_SmartMSD_AddConstFiles         (unsigned Lun, const USB_SMARTMSD_CONST_FILE * paConstFile, int NumFiles);    // Add list of predefined files. such as Readme.txt, ...
void USB_SmartMSD_SetNumSectors         (unsigned Lun, int                          NumSectors);
void USB_SmartMSD_SetSectorsPerCluster  (unsigned Lun, int                          SectorsPerCluster);

int  USB_SmartMSD_Init                  (void);
void USB_SmartMSD_X_Config              (void);    // Has to be defined by user
void USB_SmartMSD_ReInit                (void);
void USB_SmartMSD_DeInit                (void);

#endif  // #ifndef USB_SmartMSD_H

/*************************** End of file ****************************/
