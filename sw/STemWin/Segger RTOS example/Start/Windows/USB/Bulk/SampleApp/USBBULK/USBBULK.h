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
File    : USBBULK.h
Purpose : USB functions
---------------------------END-OF-HEADER------------------------------
*/
#ifndef _USBBULK_H
#define _USBBULK_H

#include <windows.h>
#include "Global.h"

#ifdef USBBULK_SIMPLE
  #define USBBULK_API 
#else
  #ifdef USBBULK_EXPORTS
    #define USBBULK_API __declspec(dllexport)
  #else
  #define USBBULK_API __declspec(dllimport)
  #endif
#endif


#if defined(__cplusplus)
  extern "C" {          /* Make sure we have C-declarations in C++ programs */
#endif


/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define USBBULK_BUFFERSIZE            (64*1024)
#ifndef   USBBULK_MAX_DEVICES
  #define USBBULK_MAX_DEVICES              256
#endif

/*********************************************************************
*
*       Defines, non configurable
*
**********************************************************************
*/
#define USBBULK_MODE_BIT_ALLOW_SHORT_READ            (1 << 0)
#define USBBULK_MODE_BIT_ALLOW_SHORT_WRITE           (1 << 1)

#define USBBULK_MTYPE_INIT          (1UL <<  0)
#define USBBULK_MTYPE_CORE          (1UL <<  1)
#define USBBULK_MTYPE_DEVICE        (1UL <<  2)
#define USBBULK_MTYPE_API           (1UL <<  3)
#define USBBULK_MTYPE_READ          (1UL <<  4)
#define USBBULK_MTYPE_WRITE         (1UL <<  5)
#define USBBULK_MTYPE_LOCK          (1UL <<  6)
#define USBBULK_MTYPE_APPLICATION   (1UL << 31)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef enum _USBBULK_DEVICE_EVENT{
  USBBULK_DEVICE_EVENT_ADD,
  USBBULK_DEVICE_EVENT_REMOVE
} USBBULK_DEVICE_EVENT;


typedef struct _USBBULK_DEV_INFO {
  U16  VendorId;
  U16  ProductId;
  char acSN[256];
  char acDevName[256];
} USBBULK_DEV_INFO;


typedef int USB_BULK_HANDLE;

typedef void (__stdcall USBBULK_NOTIFICATION_FUNC)(void *  pContext, unsigned Index, USBBULK_DEVICE_EVENT Event);
typedef void (__stdcall USBBULK_LOG_FUNC) (const char * sLog);
typedef void (__stdcall USBBULK_WARN_FUNC)(const char * sWarn);

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

/*********************************************************************
*
*       USB-Bulk basic functions
*/
USBBULK_API void            WINAPI USBBULK_Close                  (USB_BULK_HANDLE hDevice);
USBBULK_API USB_BULK_HANDLE WINAPI USBBULK_Open                   (unsigned DevIndex);

USBBULK_API void            WINAPI USBBULK_Init                   (USBBULK_NOTIFICATION_FUNC * pfNotification, void * pContext);
USBBULK_API void            WINAPI USBBULK_Exit                   (void);
USBBULK_API void            WINAPI USBBULK_SetUSBId               (U16 VendorId, U16 ProductId);
USBBULK_API void            WINAPI USBBULK_AddAllowedDeviceItem   (U16 VendorId, U16 ProductId);
USBBULK_API void            WINAPI USBBULK_RemoveAllowedDeviceItem(U16 VendorId, U16 ProductId);
        
/*********************************************************************
*
*       USB-Bulk direct input/output functions
*/
USBBULK_API int             WINAPI USBBULK_Read        (USB_BULK_HANDLE hDevice,             void       * pBuffer,   int NumBytes);
USBBULK_API int             WINAPI USBBULK_Write       (USB_BULK_HANDLE hDevice,             const void * pBuffer,   int NumBytes);
USBBULK_API int             WINAPI USBBULK_WriteRead   (USB_BULK_HANDLE hDevice,             const void * pWrBuffer, int WrNumBytes, void *  pRdBuffer, int RdNumBytes);
USBBULK_API void            WINAPI USBBULK_CancelRead  (USB_BULK_HANDLE hDevice);
USBBULK_API int             WINAPI USBBULK_ReadTimed   (USB_BULK_HANDLE hDevice,                   void * pBuffer,   int NumBytes, unsigned ms);
USBBULK_API int             WINAPI USBBULK_WriteTimed  (USB_BULK_HANDLE hDevice,             const void * pBuffer,   int NumBytes, unsigned ms);
USBBULK_API int             WINAPI USBBULK_FlushRx     (USB_BULK_HANDLE hDevice);

/*********************************************************************
*
*       USB-Bulk control functions
*/
USBBULK_API int             WINAPI USBBULK_GetConfigDescriptor        (USB_BULK_HANDLE hDevice, void * pBuffer, int Size);
USBBULK_API unsigned        WINAPI USBBULK_GetMode                    (USB_BULK_HANDLE hDevice);
USBBULK_API unsigned        WINAPI USBBULK_GetReadMaxTransferSize     (USB_BULK_HANDLE hDevice);
USBBULK_API unsigned        WINAPI USBBULK_GetWriteMaxTransferSize    (USB_BULK_HANDLE hDevice);
USBBULK_API int             WINAPI USBBULK_ResetPipe                  (USB_BULK_HANDLE hDevice);
USBBULK_API int             WINAPI USBBULK_ResetDevice                (USB_BULK_HANDLE hDevice);
USBBULK_API unsigned        WINAPI USBBULK_SetMode                    (USB_BULK_HANDLE hDevice, unsigned Mode);
USBBULK_API void            WINAPI USBBULK_SetReadTimeout             (USB_BULK_HANDLE hDevice, int    Timeout);
USBBULK_API void            WINAPI USBBULK_SetWriteTimeout            (USB_BULK_HANDLE hDevice, int    Timeout);
USBBULK_API U32             WINAPI USBBULK_GetEnumTickCount           (USB_BULK_HANDLE hDevice);
USBBULK_API U32             WINAPI USBBULK_GetReadMaxTransferSizeDown (USB_BULK_HANDLE hDevice);
USBBULK_API U32             WINAPI USBBULK_GetWriteMaxTransferSizeDown(USB_BULK_HANDLE hDevice);
USBBULK_API unsigned        WINAPI USBBULK_SetWriteMaxTransferSizeDown(USB_BULK_HANDLE hDevice, U32 TransferSize);
USBBULK_API unsigned        WINAPI USBBULK_SetReadMaxTransferSizeDown (USB_BULK_HANDLE hDevice, U32 TransferSize);
USBBULK_API unsigned        WINAPI USBBULK_GetSN                      (USB_BULK_HANDLE hDevice, U8 * pBuffer, unsigned BufferSize);
USBBULK_API void            WINAPI USBBULK_GetDevInfo                 (USB_BULK_HANDLE hDevice, USBBULK_DEV_INFO * pDevInfo);
USBBULK_API void            WINAPI USBBULK_GetUSBId                   (USB_BULK_HANDLE hDevice, U16 * pVendorIdMask, U16 * pProductIdMask);
USBBULK_API int             WINAPI USBBULK_GetProductName             (USB_BULK_HANDLE hDevice, char * sProductName, unsigned BufferSize);
USBBULK_API int             WINAPI USBBULK_GetVendorName              (USB_BULK_HANDLE hDevice, char * sVendorName, unsigned BufferSize);
USBBULK_API int             WINAPI USBBULK_GetLocationInfo            (USB_BULK_HANDLE hDevice, char * pBuffer, unsigned NumBytesBuffer);

/*********************************************************************
*
*       USB-Bulk general GET functions
*/
USBBULK_API unsigned        WINAPI USBBULK_GetDriverCompileDate  (char * s, unsigned Size);
USBBULK_API unsigned        WINAPI USBBULK_GetDriverVersion      (void);
USBBULK_API unsigned        WINAPI USBBULK_GetVersion            (void);
USBBULK_API unsigned        WINAPI USBBULK_GetNumAvailableDevices(U32 * pMask);
USBBULK_API const GUID      WINAPI USBBULK_GetGUID               (void);


/*********************************************************************
*
*       USB-Bulk general Logging functions
*/
USBBULK_API void            WINAPI USBBULK_EnableLog          (USBBULK_LOG_FUNC * pfLog, USBBULK_WARN_FUNC * pfWarn);
USBBULK_API void            WINAPI USBBULK_SetLogFilter       (U32 FilterMask);
USBBULK_API void            WINAPI USBBULK_SetWarnFilter      (U32 FilterMask);
USBBULK_API void            WINAPI USBBULK_AddLogFilter       (U32 FilterMask);
USBBULK_API void            WINAPI USBBULK_AddWarnFilter      (U32 FilterMask);


#if defined(__cplusplus)
  }     /* Make sure we have C-declarations in C++ programs */
#endif


#endif

/*************************** End of file ****************************/
