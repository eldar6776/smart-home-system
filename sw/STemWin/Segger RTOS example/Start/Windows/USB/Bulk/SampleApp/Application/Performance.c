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
File    : Performance.c
Purpose : USB BULK performance test application
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
#define APP_TITLE "USB BULK Sample Performance"

#define CMD_TEST_SPEED         0x01
#define CMD_TEST               0x02
#define SUBCMD_SPEED_READ             100
#define SUBCMD_SPEED_WRITE            101

#define MAX_NUM_BYTES_TEST_NET    (4100)

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
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
*       USB__StoreU32LE
*/
static void _StoreU32LE(U8 * p, U32 v) {
  *p       = (U8)((v      ) & 255);
  *(p + 1) = (U8)((v >>  8) & 255);
  *(p + 2) = (U8)((v >> 16) & 255);
  *(p + 3) = (U8)((v >> 24) & 255);
}

/*********************************************************************
*
*       USB__StoreU16LE
*/
static void _StoreU16LE(U8 * p, unsigned v) {
  *p       = (U8)((v      ) & 255);
  *(p + 1) = (U8)((v >>  8) & 255);
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
*       _Receive
*
*/
static int _Receive(USB_BULK_HANDLE hDevice, U8 * pBuffer, U32 NumBytes2Read) {
  int r;
  r = USBBULK_Read(hDevice, pBuffer, NumBytes2Read);
  if (r < 1) {
    _MessageBox("Could not read from device");
    return 1;                      // Error
  }
  printf(".");
  return 0;
}

/*********************************************************************
*
*       _Send
*
*/
static int _Send(USB_BULK_HANDLE hDevice, U8 * pBuffer, U32 NumBytes2Write) {
  int r;
  r = USBBULK_Write(hDevice, pBuffer, NumBytes2Write);
  if (r < 1) {
    _MessageBox("Could not write to device");
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
*       _TestSpeed
*/
int _TestSpeed(USB_BULK_HANDLE hDevice, U8 SubCmd, U32 NumReps, U32 NumBytes) {
  U8  abOut[0x100];
  U8* pData;
  U8* p;
  int NumWords;
  int r = -1;
  int i;

  if ((NumReps | NumBytes) == 0) {
    return 0;
  }
  //
  // Alloc memory
  //
  pData = (U8*)malloc(NumBytes);
  if (pData == NULL) {
    return -1;                        // Error, no memory
  }
  memset(pData, 0, NumBytes);
  NumWords = NumBytes >> 2;
  for (i = 0; i < NumWords; i++) {
    *(U32*)(pData + i * 4) = i;
  }
  //
  // Compose command
  //
  p = &abOut[0];
  *p++ = CMD_TEST_SPEED;
  *p++ = SubCmd;
  _StoreU32LE(p, NumReps);
  _StoreU32LE(p + 4, NumBytes);
  //
  // Transfer command
  //
  if (USBBULK_Write(hDevice, &abOut[0], 10) != 10) {
    goto CleanUp;                     // Communication error
  }
  //
  // Transfer the data
  //
  do {
    switch (SubCmd) {
    case SUBCMD_SPEED_WRITE:
      if (USBBULK_Write(hDevice, pData, NumBytes) != (int)NumBytes) {
        goto CleanUp;                 // Communication error
      }
      break;
    case SUBCMD_SPEED_READ:
      if (USBBULK_Read(hDevice, pData, NumBytes) != (int)NumBytes) {
        goto CleanUp;                 // Communication error
      }
      break;
    }
  } while (--NumReps);
  r = 0;
CleanUp:
  free(pData);
  return r;
}


/*********************************************************************
*
*       _PerformanceTest
*
*/
static int _PerformanceTest(unsigned DevIndex) {
  unsigned   NumBytes = 0x2000;
  unsigned   NumReps  = 0x200;
  U32        t, tWrite, tRead;
  int        r = 0;
  USB_BULK_HANDLE hDevice;

  hDevice = USBBULK_Open(DevIndex);
  if (hDevice == 0) {
    _MessageBox("Unable to connect to USB BULK device");
    return 1;
  } 
  USBBULK_SetReadTimeout(hDevice, 3600 * 1000);
  USBBULK_SetWriteTimeout(hDevice, 3600 * 1000);
  _GetNumberFromConsole("Enter the packet size in bytes (default: 0x2000): ", &NumBytes);
  _GetNumberFromConsole("Enter the number of packets    (default: 512): ", &NumReps);
  printf("\n Transferring %d KB (%d * %d KB)\n", (NumReps * NumBytes) >> 10, NumReps, NumBytes >> 10);
  t = timeGetTime();
  _TestSpeed(hDevice, SUBCMD_SPEED_WRITE, NumReps, NumBytes);
  tWrite = timeGetTime() - t;
  if (tWrite < (int)((NumBytes * NumReps) / 50000)) {
    printf("WARNING: Measured speed is too high (> 50 MB/s). Maybe an error occurred during speed test.\n");
  } else if (r < 0) {
    printf("ERROR: Failed to test write speed.\n");
  } else {
    printf("%d KB written in %dms ! (%.1f KB/s)\n", (NumReps * NumBytes) >> 10, tWrite, (float)(NumReps * NumBytes) / tWrite);
  }
  if (r) {
    goto End;
  }
  //
  // Prepare read command and send to device
  //
  t = timeGetTime();
  r = _TestSpeed(hDevice, SUBCMD_SPEED_READ, NumReps, NumBytes);
  tRead = timeGetTime() - t;
  if (tRead < (int)((NumBytes * NumReps) / 50000)) {
    printf("WARNING: Measured speed is too high (> 50 MB/s). Maybe an error occurred during speed test.\n");
  } else if (r) {
    printf("ERROR: Failed to test read speed.\n");
  } else {
    printf("%d KB read in %dms ! (%.1f KB/s)\n", (NumReps * NumBytes) >> 10, tRead, (float)(NumReps * NumBytes) / tRead);
  }

End:
  USBBULK_Close(hDevice);
  return r;
}

/*********************************************************************
*
*       _TestNet
*/
static int _TestNet(unsigned DevIndex, int SizeInc, int DelayInc, int DelayMax) {
  U32 AllocSize;
  U8* pWrite;
  U8* pRead;
  U32 NumBytes;
  U32 NumBytesRead;
  int Delay;
  int i;
  int r = -1;
  USB_BULK_HANDLE hDevice;

  hDevice = USBBULK_Open(DevIndex);
  if (hDevice == 0) {
    _MessageBox("Unable to connect to USB BULK device");
    return 1;
  }
  //
  // Allocate memory
  //
  AllocSize = (MAX_NUM_BYTES_TEST_NET * 2) + 16;
  pWrite = malloc(AllocSize);
  if (pWrite == NULL) {
    printf("ERROR: Could not alloc memory.\n");
    return -1;        // Error
  }
  pRead = pWrite + (AllocSize / 2);
  //
  // Fill memory with test pattern
  //
  for (i = 0; i < MAX_NUM_BYTES_TEST_NET; i++) {
    *(pWrite + i + 4) = i % 255;
  }
  //
  // Perform test in a loop
  //
  for (Delay = 0; Delay <= DelayMax; Delay += DelayInc) {
    for (NumBytes = 1; NumBytes <= MAX_NUM_BYTES_TEST_NET; NumBytes += SizeInc) {
      if (Delay || (NumBytes > 1)) {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
      }
      printf("  %4d bytes with delay %2d", NumBytes, Delay);
      *(pWrite + 0) = CMD_TEST;
      *(pWrite + 1) = Delay;
      *(pWrite + 2) = (U8)((NumBytes >> 0) & 0xFF);
      *(pWrite + 3) = (U8)((NumBytes >> 8) & 0xFF);
      NumBytesRead = USBBULK_WriteRead(hDevice, pWrite, (NumBytes + 4), pRead, (NumBytes + 1));
      if (NumBytesRead != (NumBytes + 1)) {
        printf("ERROR: Communication error (Expected %d bytes, received %d)!\n", (NumBytes + 1), NumBytesRead);
        goto Done;
      }
      if (*(pRead + NumBytes) != 0) {
        printf("ERROR: Failed to transfer data!\n");
        goto Done;
      }
      if (memcmp(pWrite + 4, pRead, NumBytes) != 0) {
        printf("ERROR: Verification of data failed!\n");
        goto Done;
      }
      if (_kbhit() != 0) {
        _getch();
        printf("\nTest canceled by user!\n");
        goto Done;    // Canceled by user
      }
    }
  }
  printf("\nTest completed successfully!\n");
  r = 0;              // O.K.
  //
  // Free the allocated memory
  //
Done:
  if (pWrite) {
    free(pWrite);
  }
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
  int      r;
  unsigned DeviceId;
  unsigned Choice;
  char     ChangeTransferSize;
  U32      TransferSize;
  USB_BULK_HANDLE hDevice;
  char            ac[256];

  timeBeginPeriod(1);
  _InitApp();
  DeviceId = _GetDeviceId();    
  hDevice = USBBULK_Open(DeviceId);
  printf("Current Read  transfer size down is %d\n", USBBULK_GetReadMaxTransferSizeDown(hDevice));
  printf("Current Write transfer size down is %d\n", USBBULK_GetWriteMaxTransferSizeDown(hDevice));
  printf("Do you want to change these (y/n)?\n");
  printf("Your choice: ");
  scanf("%c", &Choice);
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
  USBBULK_Close(hDevice);
  printf("What kind of test would you like to run?\n");
  printf("(1)Performance test\n");
  printf("(2)Stability test\n");
  printf("Your choice: ");
  scanf("%d", &Choice);
  switch (Choice) {
  case 1:
    printf("Starting performance test...\n");
    r = _PerformanceTest(DeviceId);
    if (r) {      
      goto End;
    }
    break;
  case 2:
    printf("Performing quick test...\n");
    r = _TestNet(DeviceId, 47, 5, 5);
    if (r == 0) {
      printf("Performing intensive test...\n");
      r = _TestNet(DeviceId, 1, 2, 10);
    }
    if (r) {      
      goto End;
    }
    break;
  default:
    break;
  }
End:
  if (r == 0) {
    printf("Communication with USB BULK device was successful!");
  } else {
    printf("Communication with USB BULK device was not successful!\nPress enter to exit.");
    _ConsoleGetLine(ac, sizeof(ac));
  }
  USBBULK_Exit();
  timeEndPeriod(1);
  return r;
}

/******************************* End of file ************************/