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
File    : USB_BULK_ShowDeviceState.c
Purpose : emUSB-Device sample showing the current status
          of the device in the debug output terminal.
--------  END-OF-HEADER  ---------------------------------------------
*/


#include <stdio.h>
#include "BSP.h"
#include "USB_Bulk.h"
#include "RTOS.h"


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
*       Static data
*
**********************************************************************
*/
static OS_STACKPTR int _aUSBStack[512]; /* Task stacks */
static OS_TASK _USBTCB0;               /* Task-control-blocks */
static U32       _Data32;
static int       _State;
static OS_STACKPTR int _aStatusStack[512]; /* Task stacks */
static OS_TASK _StatusTCB0;               /* Task-control-blocks */
static OS_EVENT _Event;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _OnStatusChange
*
*/
static void _OnStatusChange(void * pContext, U8 NewState) {
  (void)pContext;

  _State = NewState;
  OS_EVENT_Pulse(&_Event);
}


/*********************************************************************
*
*       _StatusTask
*
*/
static void _StatusTask(void) {
  static USB_HOOK _Hook;

  OS_EVENT_Create(&_Event);
  USBD_RegisterSCHook(&_Hook, _OnStatusChange, &_Data32);
  while(1) {
    printf("Current state = %s %s %s %s %s\n", (_State & USB_STAT_ATTACHED)  ? "Attached  " : "",
                                               (_State & USB_STAT_READY)     ? "Ready     " : "",
                                               (_State & USB_STAT_ADDRESSED) ? "Addressed " : "",
                                               (_State & USB_STAT_CONFIGURED)? "Configured" : "",
                                               (_State & USB_STAT_SUSPENDED) ? "Suspended " : ""
                                           );
    OS_EVENT_Wait(&_Event);
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
  static U8             _abOutBuffer[USB_MAX_PACKET_SIZE];
  USB_BULK_INIT_DATA    InitData;
  USB_BULK_HANDLE       hInst;

  InitData.EPIn  = USBD_AddEP(1, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, NULL, 0);
  InitData.EPOut = USBD_AddEP(0, USB_TRANSFER_TYPE_BULK, USB_MAX_PACKET_SIZE, _abOutBuffer, USB_MAX_PACKET_SIZE);
  hInst = USBD_BULK_Add(&InitData);
  return hInst;
}

/*********************************************************************
*
*       _USBTask
*/
static void _USBTask(void * pParam) {
  USB_BULK_HANDLE hInst;

  hInst = (USB_BULK_HANDLE)pParam;
  while(1) {
    //
    // Loop: Receive data byte by byte, send back (data + 1)
    //
    while (1) {
      char c;

      while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
        BSP_ToggleLED(0);
        USB_OS_Delay(50);
      }
      BSP_SetLED(0);         // LED on to indicate we are waiting for data
      USBD_BULK_Read(hInst, &c, 1, 0);
      BSP_ClrLED(0);
      c++;
      USBD_BULK_Write(hInst, &c, 1, 0);
    }
  }
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

  USBD_Init();
  hInst = _AddBULK();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  OS_CREATETASK(&_StatusTCB0, "Status Task", _StatusTask, 50, _aStatusStack);
  OS_CREATETASK_EX(&_USBTCB0, "USB Task",    _USBTask,   150, _aUSBStack, (void *)hInst);
  OS_Terminate(OS_Global.pCurrentTask);
}

/**************************** end of file ***************************/

