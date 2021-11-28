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
File    : USB_CDC_Start.c
Purpose : Sample showing a USB2COM echo server.
          Additionally to the echo this sample displays the amount
          of bytes received.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include "BSP.h"
#include "USB.h"
#include "USB_CDC.h"

/*********************************************************************
*
*       Information that are used during enumeration
*/
static const USB_DEVICE_INFO _DeviceInfo = {
  0x8765,         // VendorId
  0x1111,         // ProductId
  "Vendor",       // VendorName
  "CDC device",   // ProductName
  "13245678"      // SerialNumber
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _OnLineCoding
*
*  Function description
*    Called whenever a "SetLineCoding" Packet has been received
*
*  Notes
*    (1) Context
*        This function is called directly from an ISR in most cases.
*/
static void _OnLineCoding(USB_CDC_LINE_CODING * pLineCoding) {
#if 0
  printf("DTERate=%u, CharFormat=%u, ParityType=%u, DataBits=%u\n",
          pLineCoding->DTERate,
          pLineCoding->CharFormat,
          pLineCoding->ParityType,
          pLineCoding->DataBits);
#else
  BSP_USE_PARA(pLineCoding);
#endif
}


/*********************************************************************
*
*       _AddCDC
*
*  Function description
*    Add communication device class to USB stack
*/
static USB_CDC_HANDLE _AddCDC(void) {
  static U8             _abOutBuffer[USB_MAX_PACKET_SIZE];
  USB_CDC_INIT_DATA     InitData;
  USB_CDC_HANDLE        hInst;

  InitData.EPOut = USBD_AddEP(USB_DIR_OUT, USB_TRANSFER_TYPE_BULK, 0, _abOutBuffer, USB_MAX_PACKET_SIZE);
  InitData.EPIn  = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_BULK, 0, NULL, 0);
  InitData.EPInt = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_INT,  8, NULL, 0);
  hInst = USBD_CDC_Add(&InitData);
  USBD_CDC_SetOnLineCoding(hInst, _OnLineCoding);
  return hInst;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       MainTask
*
* USB handling task.
*   Modify to implement the desired protocol
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif
void MainTask(void) {
  USB_CDC_HANDLE hInst;
  U32 i = 0;

  USBD_Init();
  hInst = _AddCDC();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  while (1) {
    char ac[64];
    char acOut[30];
    int  NumBytesReceived;
    int  NumBytesToSend;

    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);
    //
    // Receive at maximum of 64 Bytes
    // If less data has been received,
    // this should be OK.
    //
    NumBytesReceived = USBD_CDC_Receive(hInst, &ac[0], sizeof(ac), 0);
    i++;
    NumBytesToSend = sprintf(acOut, "%.3lu: Received %d byte(s) - \"", i, NumBytesReceived);
    if (NumBytesReceived > 0) {
      USBD_CDC_Write(hInst, &acOut[0], NumBytesToSend, 0);
      USBD_CDC_Write(hInst, &ac[0], NumBytesReceived, 0);
      USBD_CDC_Write(hInst, "\"\n\r", 3, 0);
    }
  }
}

/**************************** end of file ***************************/

