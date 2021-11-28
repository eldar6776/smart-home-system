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
File    : Echo1.c
Purpose : USB BULK One byte echo application
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <conio.h>
#include <errno.h>
#include <windows.h>
#include "USBBULK.h"

/*********************************************************************
*
*       defines, configurable
*
**********************************************************************
*/
#define APP_TITLE "USB BULK Sample Echo1"
/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static          HWND _MainHwnd;

/*********************************************************************
*
*       static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _ConsoleGetLine
*
*/
static char * _ConsoleGetLine(char * pBuffer, U32 BufferSize) {
  fgets(pBuffer, BufferSize, stdin);
  pBuffer[strlen(pBuffer) - 1] = 0;
  return pBuffer;
}

/*********************************************************************
*
*       _MessageBox
*
*/
static void _MessageBox(const char * s) {   
  MessageBox(_MainHwnd, s, "USB Bulk sample application", MB_OK | MB_APPLMODAL);
}

/*********************************************************************
*
*       _SendReceive1
*
*/
static int _SendReceive1(USB_BULK_HANDLE hDevice, unsigned char DataTx) {
  unsigned char DataRx;
  int r;

  r = USBBULK_WriteRead(hDevice, &DataTx, 1, &DataRx, 1);
  if (r < 1) {
    _MessageBox("Could not write to device");
    return 1;                      // Error
  }
  if (DataRx != (DataTx + 1)) {
    _MessageBox("Wrong data read");
    return 1;                      // Error
  }
  printf(".");
  return 0;
}

/*********************************************************************
*
*       _ShowDriverInfo
*
*/
static void _ShowDriverInfo(void) {
  char ac[200];
  unsigned Ver;
  USBBULK_GetDriverCompileDate(ac, sizeof(ac));
  Ver = USBBULK_GetDriverVersion();
  printf("USB BULK driver version: %d.%.2d%c, compiled: %s\n", Ver / 10000, (Ver / 100) % 100, (Ver % 100) + 'a', ac);
}

/*********************************************************************
*
*       _Echo1
*
*/
static int _Echo1(unsigned DeviceId) {
  int NumBytes2Send;
  int i;
  int r;
  char ac[100];
  USB_BULK_HANDLE hDevice;

  r = 0;
  hDevice = USBBULK_Open(DeviceId);
  if (hDevice == 0) {
    _MessageBox("Unable to connect to USB BULK device");
    return 1;
  }
  _ShowDriverInfo();
  USBBULK_SetReadTimeout(hDevice, 3600 * 1000);
  USBBULK_SetWriteTimeout(hDevice, 3600 * 1000);
  printf("Enter the number of bytes to be send to the echo client: ");
  _ConsoleGetLine(ac, sizeof(ac));
  NumBytes2Send = atoi(&ac[0]);
  for (i = 0; i < NumBytes2Send; i++) {
    char DataTx;

    DataTx = i % 255;
    if (_SendReceive1(hDevice, DataTx)) {
      r = 1;
      break;
    }
  }
  if (r == 0) {
    printf ("\n%d bytes successfully transferred.", NumBytes2Send);
  }
  USBBULK_Close(hDevice);
  return r;
}

/*********************************************************************
*
*       _GetDeviceId
*
*/
static unsigned _GetDeviceId(void) {
  U32      DeviceMask;
  char     Restart;
  char     Msg = 0;
  unsigned i;
  unsigned NumDevices = 0;
  unsigned DeviceId;
  USB_BULK_HANDLE hDevice;
  char            acName[256];
  char            ac[20];
  char          * pEnd = NULL;
  char          * pEndExpected = NULL;
  do {
    Restart = 'N';
    for (;;) {
      NumDevices = USBBULK_GetNumAvailableDevices(&DeviceMask);
      if (NumDevices) {
        break;
      }
      if (Msg == 0) {
        Msg = 1;
        printf("Waiting for USB BULK devices to connect....\n");
      }
      Sleep(100);
    }
    printf("\nFound %d %s\n", NumDevices, NumDevices == 1 ? "device" : "devices");
    for (i = 0; i < NumDevices; i++) {
      if (DeviceMask & (1 << i)) {
        hDevice = USBBULK_Open(i);
        printf("Found the following device %d:\n", i);
        acName[0] = 0;
        USBBULK_GetVendorName(hDevice, acName, sizeof(acName));
        printf("  Vendor Name : %s\n", acName);
        acName[0] = 0;
        USBBULK_GetProductName(hDevice, acName, sizeof(acName));
        printf("  Product Name: %s\n", acName);
        acName[0] = 0;
        USBBULK_GetSN(hDevice, acName, sizeof(acName));      
        printf("  Serial no.  : %s\n", acName);
        USBBULK_Close(hDevice);
      }
    }
    printf("To which device do you want to connect?\nPlease type in device number (e.g. '0' for the first device, q/a for abort):");
    _ConsoleGetLine(ac, sizeof(ac));
    pEndExpected = &ac[0] +strlen(ac);
    DeviceId = strtol(&ac[0], &pEnd, 0);
    if ((pEnd != pEndExpected)) {
      printf("Invalid device id was entered!!!!\n");
      if ((toupper(ac[0]) == 'Q') || (toupper(ac[0]) == 'A')) {
        DeviceId = -1;
        break;
      } else {
        Restart = 'Y';
        continue;
      }
    }
    if (DeviceId < USBBULK_MAX_DEVICES) {
      break;
    }
  } while (Restart == 'Y');  
  return DeviceId;
}

/*********************************************************************
*
*       _InitApp
*
*/
static void _InitApp(void) {
  char            ac[256];
  U32             Ver;
  //
  // Get the handle of the console window in order to allow
  // task modal message boxes.
  //
  GetConsoleTitle(ac, sizeof(ac));
  _MainHwnd = FindWindow("ConsoleWindowClass", ac);
  memset(ac, 0, sizeof(ac));
  //
  // Update console title
  //
  SetConsoleTitle("USBBULK Performance application");
  //
  //  Init the USBBULK module
  //
  USBBULK_Init(NULL, NULL);
  //
  //  Add all allowed devices via (VendorId, ProductId)
  //
  USBBULK_AddAllowedDeviceItem(0x8765, 0x1234);
  //
  // Retrieve some version information from the USBBULK module
  //
  ac[0] = 0;
  USBBULK_GetDriverCompileDate(ac, sizeof(ac));
  Ver = USBBULK_GetDriverVersion();
  //
  // Show info
  //
  printf(APP_TITLE"\n");
  if (ac[0]) {
    printf("USB BULK driver version: %d.%.2d%c, compiled: %s\n", Ver / 10000, (Ver / 100) % 100, (Ver % 100) + 'a', ac);
  }
}

/*********************************************************************
*
*       public code
*
**********************************************************************
*/

/*********************************************************************
*
*       main
*
*/
int main(int argc, char* argv[]) {
  int        r;
  char       Restart;
  unsigned   NumDevices = 0;
  unsigned   DeviceId;
  char       acName[256];
  char     * pEnd = NULL;

  _InitApp();
  do {
    Restart = 'N';
    //
    // Get a valid device id.
    //
    DeviceId = _GetDeviceId();
    if (DeviceId == -1) {
      r = -1;
      printf("Invalid Id, exiting....");
      break;
    }
    //
    //  
    //
    printf("Starting Echo...\n");
    r = _Echo1(DeviceId);
    if (r) {      
      break;
    }
    if (MessageBox(_MainHwnd, "Start again?", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
      Restart = 'Y';
    }
  } while (Restart == 'Y');  
  if (r == 0) {
    printf("Communication with USB BULK device was successful!");
  } else {
    printf("Communication with USB BULK device was not successful!\nPress enter to exit.");
    _ConsoleGetLine(acName, sizeof(acName));
  }
  USBBULK_Exit();
  return r;
}

/******************************* End of file ************************/