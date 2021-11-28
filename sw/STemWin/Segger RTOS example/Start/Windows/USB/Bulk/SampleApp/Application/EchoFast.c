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
File    : EchoFast.c
Purpose : USB BULK fast echo application
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
#define APP_TITLE "USB BULK Sample EchoFast"

/*********************************************************************
*
*       defines, non-configurable
*
**********************************************************************
*/
#define CMD_ECHO_BULK         0x01

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static          HWND _MainHwnd;
static volatile U32  DeviceMask;
static volatile U32  NumDevices;

/*********************************************************************
*
*       static code
*
**********************************************************************
*/


/*********************************************************************
*
*       _ConsoleClear
*
*/
static void _ConsoleClear(void)  {
  COORD coordScreen = { 0, 0 };    /* here's where we'll home the cursor */ 
  BOOL bSuccess;
  DWORD cCharsWritten;
  CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */ 
  DWORD dwConSize;                 /* number of character cells in the current buffer */ 
  HANDLE hConsole;
  
  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  //
  // Get the number of character cells in the current buffer
  //
  bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
  dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
  /* fill the entire screen with blanks */ 
  bSuccess = FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten );
  /* get the current text attribute */ 
  bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
  /* now set the buffer's attributes accordingly */ 
  bSuccess = FillConsoleOutputAttribute( hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten );
  /* put the cursor at (0, 0) */ 
  bSuccess = SetConsoleCursorPosition( hConsole, coordScreen );
  return;
}
	
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
*       _GetNumberFromConsole
*
*/
static void _GetNumberFromConsole(const char * s, unsigned * pNumber) {
  int    r;
  char   ac[100];

  printf(s);
  _ConsoleGetLine(ac, sizeof(ac));
  r = atoi(&ac[0]);
  if (r > 0) {
    *pNumber = r;
  }
}

/*********************************************************************
*
*       _MessageBox
*
*/
static void _MessageBox(const char * s) {
  MessageBox(_MainHwnd, s, "USB Bulk sample application", MB_OK | MB_TOPMOST | MB_ICONERROR | MB_TASKMODAL);
}


/*********************************************************************
*
*       _SendReceive
*
*/
static int _SendReceive(USB_BULK_HANDLE hDevice, unsigned char * pBufferTx, unsigned short NumBytes2Write,
                        unsigned char * pBufferRx, unsigned short NumBytes2Read) {
  int r;
#if 1
  r = USBBULK_WriteTimed(hDevice, pBufferTx, NumBytes2Write, 10000);
  if (r < 1) {
    _MessageBox("Could not write to device");
    return 1;                      // Error
  }
  USBBULK_ReadTimed(hDevice, pBufferRx, NumBytes2Read * 2, 500);
#else
  r = USBBULK_WriteRead(hDevice, pBufferTx, NumBytes2Write, pBufferRx, NumBytes2Read);
  if (r < 1) {
    _MessageBox("Could not write to device");
    return 1;                      // Error
  }
#endif
  if (memcmp(pBufferTx + 3, pBufferRx, NumBytes2Read)) {
    _MessageBox("Wrong data read");
    return 1;                      // Error
  }
  printf(".");
  return 0;
}

/*********************************************************************
*
*       _FillBuffer
*
*/
static void _FillBuffer(unsigned char * pBuffer, unsigned NumBytes) {
  unsigned i;
  for (i = 0; i < NumBytes; i++) {
    *(pBuffer + i) = (unsigned char )(i % 255);
  }
}


/*********************************************************************
*
*       _EchoFast
*
*/
static int _EchoFast(U32 DeviceId) {
  unsigned short  NumBytes2Send;
  unsigned short  NumBytes2Read;
  unsigned        PacketSize;
  unsigned        NumPackets;
  unsigned        i;
  unsigned char * pBufferTx;
  unsigned char * pBufferRx;
  unsigned        t;
  int r = 0;
  USB_BULK_HANDLE hDevice;

  //
  // Open the device
  //
  hDevice = USBBULK_Open(DeviceId);
  if (hDevice == 0) {
    _MessageBox("USB BULK device cannot be opened");
    return -1;
  }
  //
  // Get the parameters for sending
  //
  PacketSize = 500;
  _GetNumberFromConsole("Enter the packet size in bytes (default: 500): ", &PacketSize);
  NumPackets = 500;
  _GetNumberFromConsole("Enter the number of packets    (default: 500): ", &NumPackets);
  //
  //  Update the variables
  //
  NumBytes2Read  = PacketSize;
  NumBytes2Send  = PacketSize + 3; 
  //
  // Allocate buffers
  //
  pBufferTx = (unsigned char *)malloc(NumBytes2Send);
  pBufferRx = (unsigned char *)malloc(NumBytes2Read * 2);
  //
  // Write the packet information
  //
  *(pBufferTx + 0) = CMD_ECHO_BULK;
  *(pBufferTx + 1) = PacketSize & 0xff;
  *(pBufferTx + 2) = PacketSize>> 8;
  //
  // Fill the buffer with data
  //
  _FillBuffer((pBufferTx + 3), PacketSize);
  //
  // Perform the USB BULK operation
  //
  t = timeGetTime();
  for (i = 0; i < NumPackets; i++) {
    if (_SendReceive(hDevice, pBufferTx, NumBytes2Send, pBufferRx, NumBytes2Read)) {
      r = 1;
      break;
    }
  }
  //
  // Print out the results
  //
  if (r == 0) {
    t = timeGetTime() - t;
    printf ("\n2 * %d packets of %d bytes successfully transferred in %d msec.", NumPackets, PacketSize, t);
  }
  //
  // Free the allocated buffers
  //
  free(pBufferTx);
  free(pBufferRx);
  //
  // Close the device
  //
  USBBULK_Close(hDevice);
  return r;
}

/*********************************************************************
*
*       _OnDevNotify
*
*  Function description:
*    Is called when a new device is found or an existing device is removed.
*
*  Parameters:
*    pContext  - Pointer to a context given when USBBULK_Init is called
*    Index     - Device Index that has been added or removed.
*    Event     - Type of event, currently the following are available:
*                  USBBULK_DEVICE_EVENT_ADD 
*                  USBBULK_DEVICE_EVENT_REMOVE
*
*/
static void __stdcall _OnDevNotify(void *  pContext, unsigned Index, USBBULK_DEVICE_EVENT Event) {
  switch(Event) {
  case USBBULK_DEVICE_EVENT_ADD:
    NumDevices = USBBULK_GetNumAvailableDevices((U32 *)&DeviceMask);
    break;
  case USBBULK_DEVICE_EVENT_REMOVE:
    NumDevices = USBBULK_GetNumAvailableDevices((U32 *)&DeviceMask);
    break;
  }
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
  U32             NumDevices = 0;
  U32             DeviceId;
  int             r = 0;
  char            Restart;
  char            acBuff[50];

  timeBeginPeriod(1);
  _InitApp();
  do {
    Restart = 'N';
    _ConsoleClear();
    DeviceId = _GetDeviceId();
    if (DeviceId == -1) {
      r = -1;
      printf("Invalid Id, exiting....");
      break;
    }
    //
    //  
    //
    printf("Starting EchoFast...\n");
    r = _EchoFast(DeviceId);
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
    _ConsoleGetLine(acBuff, sizeof(acBuff));
  }
  USBBULK_Exit();
  timeEndPeriod(1);
  return r;
}

/******************************* End of file ************************/