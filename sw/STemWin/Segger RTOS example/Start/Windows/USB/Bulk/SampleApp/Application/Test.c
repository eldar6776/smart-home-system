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
File    : Test.c
Purpose : USB BULK Test Application
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "USBBULK.h"

/*********************************************************************
*
*       defines, configurable
*
**********************************************************************
*/
#define APP_TITLE "USB BULK Sample Test"

#define SIZEOF_BUFFER  0x4000
#define INC_TEST_START  1
#define INC_TEST_END    1024

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static char _acTxBuffer[SIZEOF_BUFFER];
static char _acRxBuffer[SIZEOF_BUFFER];
static HWND _MainHwnd;

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
*       _SendBuffer
*
*/
static int _SendBuffer(USB_BULK_HANDLE hDevice, unsigned char* pData, int Len) {
  int  r;
  char ac[2];

  if (Len) {
    printf("Writing %d bytes\n", Len);
    //
    //  Send 16bit Len, MSB first to be compatible with SampleApp
    //
    ac[0] = (Len >> 8);
    ac[1] = (Len & 0xFF);
    r = USBBULK_Write(hDevice, ac, 2);
    if (r == 0) {
      _MessageBox("Could not write to device");
      return 0;
    }
    r = USBBULK_Write(hDevice, pData, Len);
    return r;
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       _ReadBuffer
*
*/
static int _ReadBuffer(USB_BULK_HANDLE hDevice, unsigned char* pDest, int Len) {
  int r;

  printf("Reading %d bytes\n", Len);
  r = USBBULK_Read(hDevice, pDest, Len);
  if (r == 0) {
    _MessageBox("Could not read from device (time out)");
  }
  return r;
}

/*********************************************************************
*
*       _SendReceive1
*
*/
static int _SendReceive1(USB_BULK_HANDLE hDevice, unsigned char DataTx) {
  unsigned char DataRx;
  int r;

  printf("Writing one byte\n");
  r = USBBULK_Write(hDevice, &DataTx, 1);
  if (r == 0) {
    _MessageBox("Could not write to device");
  }
  printf("Reading one byte\n");
  r = USBBULK_Read (hDevice, &DataRx, 1);
  if (r == 0) {
    _MessageBox("Could not read from device (time out)");
  }
  if (DataRx != (DataTx + 1)) {
    _MessageBox("Wrong data read");
    return 1;
  }
  printf("Operation successful!\n\n");
  return 0;
}

/*********************************************************************
*
*       _Test
*
*/
static int _Test(USB_BULK_HANDLE hDevice) {
  int i;
  int NumBytes;
  int r;
  int t;
  //
  // Do a simple 1 byte test first
  //
  r = _SendReceive1(hDevice, 0x12);
  if (r) {
    return r;
  }
  r = _SendReceive1(hDevice, 0x13);
  if (r) {
    return r;
  }
  //
  // Initially fill buffer
  //
  for (NumBytes = 0; NumBytes < SIZEOF_BUFFER; NumBytes++) {
    _acTxBuffer[NumBytes] = NumBytes % 255; 
  }
  //
  // Test different sizes
  //
  for (NumBytes = INC_TEST_START; NumBytes <= INC_TEST_END; NumBytes++) {  // Send and receive various data packets
    r = _SendBuffer(hDevice, _acTxBuffer, NumBytes);
    if (r != NumBytes) {
      _MessageBox("Could not write to device (time out)");
      return 1;
    }
    r = _ReadBuffer(hDevice, _acRxBuffer, NumBytes);
    if (r != NumBytes) {
      _MessageBox("Could not read from device (time out)");
      return 1;
    }
    if (memcmp(_acRxBuffer, _acTxBuffer, NumBytes)) {
      _MessageBox("Wrong data received");
      return 1;
    }
  }
  //
  // Test speed
  //
  printf("Testing speed:");
  t = GetTickCount();
  for (i = 0; i< 500; i++) {
    int NumBytes = 4 * 1024;
    _acTxBuffer[0] = NumBytes >> 8;
    _acTxBuffer[1] = NumBytes & 255;
    USBBULK_WriteRead(hDevice, _acTxBuffer, NumBytes + 2, _acTxBuffer, NumBytes);
    if (i %10 == 0) {
      printf(".");
    }
  }
  t = GetTickCount() - t;
  printf("\nPerformance: %d ms for 4MB", t);
  return 0;
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
  char            ac[256] = {0};
  U32             Ver;
  //
  // Get the handle of the console window in order to allow
  // task modal message boxes.
  //
  GetConsoleTitle(ac, sizeof(ac));
  _MainHwnd = FindWindow("ConsoleWindowClass", ac);
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
*       main
*
* Function description
*/
int main(int argc, char* argv[]) {
  int             r;
  char            Restart;
  unsigned        DeviceId;
  char            ChangeTransferSize;
  U32             TransferSize;
  USB_BULK_HANDLE hDevice;
  char            ac[256];

  _InitApp();
  do {
    Restart = 'N';
    DeviceId   = _GetDeviceId();
    hDevice = USBBULK_Open(DeviceId);
    if (hDevice == 0) {
      _MessageBox("Unable to connect to USB BULK device");
      break;
    }
    USBBULK_SetReadTimeout(hDevice, 3600 * 1000);
    USBBULK_SetWriteTimeout(hDevice, 3600 * 1000);
    printf("Current Read  transfer size down is %d\n", USBBULK_GetReadMaxTransferSizeDown(hDevice));
    printf("Current Write transfer size down is %d\n", USBBULK_GetWriteMaxTransferSizeDown(hDevice));
    printf("Do you want to change these (y/n)?\n");
    printf("Your choice: ");
    _ConsoleGetLine(ac, sizeof(ac));
    ChangeTransferSize = toupper(ac[0]);
    if ((ChangeTransferSize != 'Y') && (ChangeTransferSize != 'N')) {
      ChangeTransferSize = 'N';
    }
    if (ChangeTransferSize == 'Y') {
      printf("Enter the new Read transfer size down size: ");
      scanf("%d", &TransferSize);
      USBBULK_SetReadMaxTransferSizeDown(hDevice, TransferSize);
      printf("Enter the new Write transfer size down size: ");
      scanf("%d", &TransferSize);
      USBBULK_SetWriteMaxTransferSizeDown(hDevice, TransferSize);               
    }
    r = _Test(hDevice);
    USBBULK_Close(hDevice);
    if (MessageBox(_MainHwnd, "Start again?", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
      Restart = 'Y';
    }
  } while (Restart == 'Y');  
  if (r == 0) {
    printf("Communication with USB BULK device was successful!");
  } else {
    printf("Communication with USB BULK device was not successful!\nPress enter to exit.");
    _ConsoleGetLine(ac, sizeof(ac));
  }
  USBBULK_Exit();
  return r;
}

/******************************* End of file ************************/