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
File    : USB_BULK_EchoFast.c
Purpose : This sample application is to be used with it's Windows
          counterpart "EchoFast.exe".
          This sample is used to demonstrate the usage of the
          "USBBULK_WriteRead()" function on the host side.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "USB_Bulk.h"
#include "BSP.h"
#include "SEGGER.h"
#include <stdio.h>

/*********************************************************************
*
*       defines, configurable
*
**********************************************************************
*/
#define CMD_ECHO_BULK         0x01


/*********************************************************************
*
*       Information that are used during enumeration
*/
static const USB_DEVICE_INFO _DeviceInfo = {
  0x8765,         // VendorId
  0x1234,         // ProductId
  "Vendor",       // VendorName
  "Bulk device",  // ProductName
  "13245678"      // SerialNumber
};

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static U8  _acBuffer[0x400];

/*********************************************************************
*
*       static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _LoadU16LE
*
*  Function description:
*    Reads a 16 bit little endian from a char array.
*
*  Parameters:
*    pBuffer     - Pointer to a char array.
*
*  Return value:
*    The value as U16 data type
*
*  Notes
*    (1) This cast should not be necessary, but on some compilers (NC30)
*        it is required in higher opt. levels since otherwise the
*        argument promotion to integer size is skipped, leading to wrong result of 0.
*
*/
static U16 _LoadU16LE(const U8 *pBuffer) {
  U16 r;
  r = (U16)(*pBuffer | ((unsigned)*(pBuffer + 1) << 8));
  return r;
}

/*********************************************************************
*
*       _ExecEchoBulk
*
* Function description
*   Execute the echo bulk command.
*/
static void _ExecEchoBulk(void) {
  U16 NumBytes;
  U16 NumBytesAtOnce;


  USBD_BULK_Read(0, &_acBuffer[0], 2, 0);
  NumBytes = _LoadU16LE(&_acBuffer[0]);
  while (NumBytes) {
    NumBytesAtOnce = SEGGER_MIN(NumBytes, sizeof(_acBuffer));
    USBD_BULK_Read(0, _acBuffer,  NumBytesAtOnce, 0);
    USBD_BULK_Write(0, _acBuffer, NumBytesAtOnce, 0);
    NumBytes -= NumBytesAtOnce;
  }
}


/*********************************************************************
*
*       _AddBULK
*
*  Function description
*    Add generic USB BULK interface to USB stack
*/
static USB_BULK_HANDLE _AddBULK(void) {
  static U8             _abOutBuffer[USB_MAX_PACKET_SIZE*2];
  USB_BULK_INIT_DATA    InitData;
  USB_BULK_HANDLE       hInst;

  InitData.EPIn  = USBD_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USBD_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, USB_MAX_PACKET_SIZE);
  hInst = USBD_BULK_Add(&InitData);
  return hInst;
}



/*********************************************************************
*
*       public code
*
**********************************************************************
*/

/*********************************************************************
*
*       MainTask
*
* Function description
*   USB handling task.
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
  USB_BULK_HANDLE hInst;
  U8 Cmd = 0;

  USBD_Init();
  hInst = _AddBULK();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  while (1) {
    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0); // Toggle LED to indicate configuration
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);
    //
    // Receive and handle cmd from host
    //
    if (USBD_BULK_Read(hInst, &Cmd, 1, 0) == 1) {
      switch (Cmd) {
      case CMD_ECHO_BULK:
        _ExecEchoBulk();
        break;

      }
    }
  }
}

/**************************** end of file ***************************/
