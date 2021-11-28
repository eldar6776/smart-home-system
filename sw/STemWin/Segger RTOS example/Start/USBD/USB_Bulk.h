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
File    : USB_Bulk.h
Purpose : Public header of the bulk component
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef BULK_H          /* Avoid multiple inclusion */
#define BULK_H

#include "SEGGER.h"
#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  U8 EPIn;
  U8 EPOut;
} USB_BULK_INIT_DATA;

typedef int USB_BULK_HANDLE;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void            USBD_BULK_Init                 (void);
USB_BULK_HANDLE USBD_BULK_Add                  (const USB_BULK_INIT_DATA * pInitData);
void            USBD_BULK_CancelRead           (USB_BULK_HANDLE hInst);
void            USBD_BULK_CancelWrite          (USB_BULK_HANDLE hInst);
unsigned        USBD_BULK_GetNumBytesInBuffer  (USB_BULK_HANDLE hInst);
unsigned        USBD_BULK_GetNumBytesRemToRead (USB_BULK_HANDLE hInst);
unsigned        USBD_BULK_GetNumBytesRemToWrite(USB_BULK_HANDLE hInst);
int             USBD_BULK_Read                 (USB_BULK_HANDLE hInst, void * pData, unsigned NumBytes, unsigned Timeout);
int             USBD_BULK_ReadOverlapped       (USB_BULK_HANDLE hInst, void * pData, unsigned NumBytes);
int             USBD_BULK_Receive              (USB_BULK_HANDLE hInst, void * pData, unsigned NumBytes, int Timeout);
void            USBD_BULK_SetOnRXHook          (USB_BULK_HANDLE hInst, USB_ON_RX_FUNC * pfOnRx);
void            USBD_BULK_SetOnTXEvent         (USB_BULK_HANDLE hInst, USB_EVENT_CALLBACK *pEventCb, USB_EVENT_CALLBACK_FUNC *pfEventCb, void *pContext);
void            USBD_BULK_SetOnRXEvent         (USB_BULK_HANDLE hInst, USB_EVENT_CALLBACK *pEventCb, USB_EVENT_CALLBACK_FUNC *pfEventCb, void *pContext);
int             USBD_BULK_WaitForRX            (USB_BULK_HANDLE hInst, unsigned Timeout);
int             USBD_BULK_WaitForTX            (USB_BULK_HANDLE hInst, unsigned Timeout);
int             USBD_BULK_WaitForTXReady       (USB_BULK_HANDLE hInst, int Timeout);
int             USBD_BULK_Write                (USB_BULK_HANDLE hInst, const void * pData, unsigned NumBytes, int Timeout);
int             USBD_BULK_WriteEx              (USB_BULK_HANDLE hInst, const void* pData, unsigned NumBytes, char Send0PacketIfRequired, int Timeout);
void            USBD_BULK_SetContinuousReadMode(USB_BULK_HANDLE hInst);
int             USBD_BULK_TxIsPending          (USB_BULK_HANDLE hInst);

/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/
#define USB_BULK_Init                          USBD_BULK_Init
#define USB_BULK_Add(x)                        USBD_BULK_Add(x)
#define USB_BULK_CancelRead()                  USBD_BULK_CancelRead(0)
#define USB_BULK_CancelWrite()                 USBD_BULK_CancelWrite(0)
#define USB_BULK_GetNumBytesInBuffer()         USBD_BULK_GetNumBytesInBuffer(0)
#define USB_BULK_GetNumBytesRemToRead()        USBD_BULK_GetNumBytesRemToRead(0)
#define USB_BULK_GetNumBytesToWrite()          USBD_BULK_GetNumBytesToWrite(0)
#define USB_BULK_Read(p, n)                    USBD_BULK_Read(0, p, n, 0)
#define USB_BULK_ReadTimed(p, n, t)            USBD_BULK_Read(0, p, n, t)
#define USB_BULK_ReadOverlapped(p, n)          USBD_BULK_ReadOverlapped(0, p, n)
#define USB_BULK_Receive(p, n)                 USBD_BULK_Receive(0, p, n, 0)
#define USB_BULK_ReceiveTimed(p, n, t)         USBD_BULK_Receive(0, p, n, t)
#define USB_BULK_SetOnRXHook(x)                USBD_BULK_SetOnRXHook(0, x)
#define USB_BULK_WaitForRX()                   USBD_BULK_WaitForRX(0, 0)
#define USB_BULK_WaitForTX()                   USBD_BULK_WaitForTX(0, 0)
#define USB_BULK_Write(p, n)                   USBD_BULK_Write(0, p, n, 0)
#define USB_BULK_WriteEx(p, n, s)              USBD_BULK_WriteEx(0, p, n, s, 0)
#define USB_BULK_WriteExTimed(p, n, s, t)      USBD_BULK_WriteEx(0, p, n, s, t)
#define USB_BULK_WriteOverlapped(p, n)         USBD_BULK_Write(0, p, n, -1)
#define USB_BULK_WriteOverlappedEx(p, n, s)    USBD_BULK_WriteEx(0, p, n, s, -1)
#define USB_BULK_WriteTimed(p, n, t)           USBD_BULK_Write(0, p, n, t)
#define USB_BULK_WriteNULLPacket()             USBD_BULK_Write(0, NULL, 0, 0)
#define USB_BULK_StartReadTransfer()           USBD_BULK_Receive(0, NULL, 0, -1)
#define USB_BULK_TxIsPending()                 USBD_BULK_TxIsPending(0)

/*********************************************************************
*  End of Wrapper
**********************************************************************/

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
