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
File    : USB_Private.h
Purpose : Private include file.
          Do not modify to allow easy updates !
Literature:
  [1]  Universal Serial Bus Specification Revision 2.0
       \\fileserver\Techinfo\Subject\USB\USB_20\usb_20.pdf
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef __USB_PRIVATE_H__
#define __USB_PRIVATE_H__

#include "USB.h"

/*********************************************************************
*
*       Config defaults
*
**********************************************************************
*/
#ifndef   USB_NUM_EPS
  #define USB_NUM_EPS 8
#endif

#ifndef   USB_MAX_NUM_IF
  #define USB_MAX_NUM_IF 4
#endif

#ifndef   USB_MAX_NUM_ALT_IF
  #define USB_MAX_NUM_ALT_IF 2
#endif

#ifndef   USB_SUPPORT_TRANSFER_INT
  #define USB_SUPPORT_TRANSFER_INT 1
#endif

#ifndef   USB_SUPPORT_TRANSFER_ISO
  #define USB_SUPPORT_TRANSFER_ISO 1
#endif

#ifndef   USB_MAX_NUM_IAD
  #define USB_MAX_NUM_IAD 3
#endif

#ifndef   USB_MAX_NUM_COMPONENTS
  #define USB_MAX_NUM_COMPONENTS 3
#endif

#ifndef   USB_EXTRA_EVENTS
  #define USB_EXTRA_EVENTS 0
#endif

#ifndef USB_MAX_STRING_DESC
  #define USB_MAX_STRING_DESC   (USB_MAX_NUM_IF + USB_MAX_NUM_ALT_IF)
#endif

#ifndef   USB_MEMCPY
  #include <string.h>
  #define USB_MEMCPY(pD, pS, NumBytes)  memcpy((pD), (pS), (NumBytes))
#endif

#ifndef   USB_MEMSET
  #include <string.h>
  #define USB_MEMSET(p, c, NumBytes)  memset((p), (c), (NumBytes))
#endif

#ifndef   USB_MEMCMP
  #include <string.h>
  #define USB_MEMCMP(p1, p2, NumBytes)  memcmp((p1), (p2), (NumBytes))
#endif


/* In order to avoid warnings for undefined parameters */
#ifndef USB_USE_PARA
  #define USB_USE_PARA(para) (void)(para)
#endif
/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define USB_NUM_TX_DATA_PARTS       3

/*********************************************************************
*
*       Function-like macros
*
**********************************************************************
*/
#ifndef COUNTOF
  #define COUNTOF(a)          (sizeof((a))/sizeof(((a)[0])))
#endif
#ifndef MIN
  #define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
  #define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#define  USB__DecRI()         USB_Global.pfDecRI()
#define  USB__IncDI()         USB_Global.pfIncDI()

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

struct _USB_INFO_BUFFER {
  unsigned Cnt;
  unsigned Sizeof;
  U8 * pBuffer;
};

typedef struct {
  U8       * pData;
  unsigned   Size;
  unsigned   NumBytesIn;
  unsigned   RdPos;
} USB_BUFFER;

struct _EP_STAT {
  U16                       MaxPacketSize;
  U16                       Interval;                   // In frames. Valid only for ISO and interrupt endpoints. Not used by stack except for creating descriptors.
  U8                        EPType;                     // 0: Control, 1: Isochronous, 2: Bulk, 3: Interrupt See [1], chapter "9.6.6  Endpoint", table 9-13.
  U8                        EPAddr;                     // b[4:0]: EPAddr => This is the physical endpoint number.
                                                        // b[6:5]: Unused, SBZ.
                                                        // b[7]:   Direction, 1: Device to Host (IN), 0: Host to device (OUT).
  volatile U8               IsHalted;                   // 1: Endpoint is stalled. Typically set by application via call to USBD_StallEP() in case of fatal error.
  union {
    struct {
      U32                   MaxTransferSize;            // Holds the maximum transfer size in bytes the driver can send at once.
      int                   TxNumBytesPending;          // It indicates whether a IN transfer is already queued for the endpoint.
                                                        // 0: No TX transfer is in progress.
                                                        // 1: Transfer of a NULL packet is in progress.
                                                        // >0: Transfer of (TxNumBytesPending - 1) bytes of data is in progress.
                                                        // -1: Intermediate state: TX transfer is just finished, but a new transfer cannot be started yet.
      U16                   NumBytesWithout0Packet;     // No 0-packet is send after a transfer of a multiple of this number of bytes.
                                                        // Must be a multiple of MaxPacketSize and a power of 2.
      U16                   NumFullPacketsSend;         // Number of full packets (MaxPacketSize) send within a transfer.
      U8                    Send0PacketIfRequired;      // This flag is used with IN endpoints. When set the stack will check at the end of a transaction whether
                                                        // the last transfer was a multiple of MaxPacketSize, if yes it will send a zero packet to complete the transaction.
                                                        // This flag is normally depending on which API function is used. Some API functions even allow the user to set it.
      U8                    SendEventOccured;           // Set if the driver has triggered USB_EVENT_DATA_SEND.
                                                        // If not, the event is later triggered together with USB_EVENT_DATA_ACKED.
      U8                    SignalOnTXReady;            // Send event if TX queue gets a free entry
      U8                    NumDataParts;
      USB_DATA_PART         DataParts[USB_NUM_TX_DATA_PARTS];
    } TxInfo;
    struct {
      USB_ON_RX_FUNC *      pfOnRx;                     // Pointer to a callback function set either by USBD_SetOnRxEP0() or by USBD_SetOnRXHookEP() functions.
      U8             *      pData;
      volatile U32          NumBytesRem;                // Volatile since modified by ISR and checked by task/application.
      U8                    RxInterruptEnabled;         // Keep track of driver RX interrupt enable / disable
      U8                    ReadMode;                   // See USB_READ_MODE... defines.
      U8                    AllowShortPacket;           // Controls the behavior of the internal _OnRxCheckDone() function.
      U8                    RxPacketByPacket;           // Perform read operation packet by packet (only request a single packet from the driver)
      USB_BUFFER            Buffer;
    } RxInfo;
  } Dir;
  USB_EVENT_CALLBACK        *pEventCallbacks;           // List of callback functions for this endpoint.
};

typedef struct {
  void (*pfAdd)(U8 FirstInterFaceNo, U8 NumInterfaces, U8 ClassNo, U8 SubClassNo, U8 ProtocolNo);
  void (*pfAddIadDesc)(int InterFaceNo, USB_INFO_BUFFER * pInfoBuffer);
} USB_IAD_API;

typedef struct {
  U16                       EPs;
  U8                        IFAlternateSetting;
  U8                        IFClass   ;    // Interface Class
  U8                        IFSubClass;    // Interface Subclass
  U8                        IFProtocol;    // Interface Protocol
  U8                        IFNum;
  U8                        iName;         // Index of String descriptor
  USB_ADD_FUNC_DESC       * pfAddFuncDesc;
  USB_ON_CLASS_REQUEST    * pfOnClassRequest;
  USB_ON_CLASS_REQUEST    * pfOnVendorRequest;
  USB_ON_SETUP            * pfOnSetup;
} INTERFACE;

typedef struct {
  U8        IFNo;   // Index of the real interface
  INTERFACE AltIF;  // Interface structure
} ALT_INTERFACE;

typedef struct {
  U8                        NumEPs;               // Count of currently used endpoints.
  U8                        NumIFs;               // Count of currently used interfaces.
  U8                        NumAltIFs;            // Count of currently used alternative interfaces.
  U8                        NumEvents;            // Next event, that may be allocated.
  U8                        Class;                // Device Class    (when IAD is not used the class is defined in the device descriptor)
  U8                        SubClass;             // Device Subclass (when IAD is not used the subclass is defined in the device descriptor)
  U8                        Protocol;             // Device Protocol (when IAD is not used the Protocol is defined in the device descriptor)
  U8                        MultiPacketRxBehavior;   // True if driver can receive multiple packets via DMA but does not report each packet
                                                     // received to the stack. Used for correct timeout behavior.
  U8                        SetAddressBehavior;
  U8                        IsInited;             // Flag indicating whether emUSB-Device was initialized.
  volatile U8               State;                // Global USB state, similar to [1]: chapter 9.1.1.
                                                  // Bitwise combination of USB_STAT_ATTACHED, USB_STAT_READY, USB_STAT_ADDRESSED, USB_STAT_CONFIGURED, USB_STAT_SUSPENDED
  volatile U8               AllowRemoteWakeup;    // b[0]: 1: Remote wake-up feature allowed by the application (through USBD_SetAllowRemoteWakeUp()).
                                                  // b[1]: 1: Remote wake-up feature allowed by the host.
  volatile U8               Addr;                 // The USB device address, assigned by the host. Zero when the device is not enumerated.
  volatile U8               IsSelfPowered;        // Flag indicating whether the device is allowed to draw current from the bus or whether it has a differnt power supply.
  U8                        NumStringDesc;        // Number of String descriptors
  const char *              aStringDesc[USB_MAX_STRING_DESC];
  INTERFACE                 aIF[USB_MAX_NUM_IF];  // Array of available interfaces.
#if USB_MAX_NUM_ALT_IF > 0
  ALT_INTERFACE             aAltIF[USB_MAX_NUM_ALT_IF];  // Array of available alternative interfaces.
#endif
  U8                        NumOnRxEP0Callbacks;  // Count of receive callbacks for endpoint zero.
  USB_ON_RX_FUNC          * apfOnRxEP0[USB_MAX_NUM_COMPONENTS]; // Array of function pointers to receive callbacks for endpoint zero.
  const USB_HW_DRIVER     * pDriver;              // Pointer to the hardware driver structure.
  USB_ENABLE_ISR_FUNC     * pfEnableISR;          // Pointer to function to enable the USB interrupt and set the interrupt handler.
  USB_INC_DI_FUNC         * pfIncDI;              // Pointer to the function to increment interrupt disable count and disable interrupts.
  USB_DEC_RI_FUNC         * pfDecRI;              // Pointer to the function to decrement interrupt disable count and enable interrupts.
  USB_IAD_API             * pIadAPI;              // Pointer to the USB Interface Association Descriptor API.
  USB_DEINIT_FUNC         * apfDeInitHandler[5];  // Array of pointers to functions which should be called when USBD_DeInit() is called. These callbacks are normally set by class modules.
  USB_DETACH_FUNC         * pfDetach;             // Pointer to a callback which is called on a detach event. Set from the application (USBD_SetDetachFunc()).
  USB_GET_STRING_FUNC     * pfGetString;          // Pointer to a callback which is called when the host requests a string descriptor from the device. Set from the application (USBD_SetGetStringFunc()).
  USB_ON_BCD_VERSION_FUNC * pfOnBCDVersion;       // Pointer to a callback which is called when the host requests a device descriptor from the device. Set from the application (USBD_SetOnBCDVersionFunc()).
  USB_DEINIT_FUNC         * pfDeInitUserHandler;  // Pointer to a user-set callback which is called at the end of the execution of USBD_DeInit(). Set from the application (USBD_SetDeInitUserFunc()).
  USB_ATTACH_FUNC         * pfAttach;             // Pointer to the attach function (optional).
  USB_ON_SET_IF_FUNC      * pfOnSetInterface;     // Pointer to a callback which is called when a Set Interface command is received. Set from the application (USB_SetOnSetInterfaceFunc()).
  const USB_DEVICE_INFO   * pDeviceInfo;          // Pointer to device information used during enumeration.
} GLOBAL;

enum {
  STRING_INDEX_LANGUAGE = 0,  // Language index. MUST BE 0 acc. to spec.
  STRING_INDEX_MANUFACTURER,  // iManufacturer:      Index of String Desc (Manuf)    (variable, but needs to be unique)
  STRING_INDEX_PRODUCT,       // iProduct:           Index of String Desc (Product)  (variable, but needs to be unique)
  STRING_INDEX_SN,            // iSerialNumber:      Index of String Desc (Serial #) (variable, but needs to be unique)
  STRING_INDEX_CONFIG,        // iConfiguration:     Index of String Desc (Configuration name) (variable, but needs to be unique)
  STRING_INDEX_OTHER          // Start index of other string descriptors store in aStringDesc[]
};

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

#ifdef USB_MAIN_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN EP_STAT USB_aEPStat[USB_NUM_EPS];
EXTERN GLOBAL  USB_Global;
#undef EXTERN

// UVC class, subclass codes
// (USB_Video_Class_1.1.pdf, 3.2 Device Descriptor)
#define UVC_DEVICE_CLASS_MISCELLANEOUS             0xFE
#define UVC_DEVICE_SUBCLASS                        0x02
#define UVC_DEVICE_PROTOCOL                        0x01

/*********************************************************************
*
*       USB descriptor defines.
*       Refer to [1]: chapter 9.6 "Standard USB Descriptor Definitions" for details.
*
*/
#define USB_DESC_TYPE_DEVICE           1
#define USB_DESC_TYPE_CONFIG           2
#define USB_DESC_TYPE_STRING           3
#define USB_DESC_TYPE_INTERFACE        4
#define USB_DESC_TYPE_ENDPOINT         5
#define USB_DESC_TYPE_QUALIFIER        6
#define USB_DESC_TYPE_SPEED_CONFIG     7
#define USB_DESC_TYPE_INTERFACE_POWER  8
#define USB_DESC_TYPE_IAD             11

#define USB_STORE_U16(u16) ((u16) & 255), ((u16) / 256)

/*********************************************************************
*
*       Internal functions.
*
**********************************************************************
*/
unsigned USB__CalcMaxPacketSize     (unsigned MaxPacketSize, U8 TransferType, U8 IsHighSpeedMode);
U8       USB__EPAddr2Index          (unsigned EPAddr);
U8       USB__EPIndex2Addr          (unsigned EPIndex);
void*    USB__GetpDest              (unsigned EPIndex,    unsigned NumBytes);
U32      USB__GetNextRX             (unsigned EPIndex, U8 **p);
U16      USB__GetU16BE              (const U8 * p);
U16      USB__GetU16LE              (const U8 * p);
U32      USB__GetU32BE              (const U8 * p);
U32      USB__GetU32LE              (const U8 * p);
void     USB__StoreU16BE            (U8 * p, unsigned v);
void     USB__StoreU16LE            (U8 * p, unsigned v);
void     USB__StoreU32LE            (U8 * p, U32 v);
void     USB__StoreU32BE            (U8 * p, U32 v);
U32      USB__SwapU32               (U32 v);
void     USB__HandleSetup           (const USB_SETUP_PACKET * pSetupPacket);
void     USB__OnBusReset            (void);
void     USB__OnResume              (void);
void     USB__OnRx                  (unsigned EPIndex, const U8 * pData, unsigned Len);
void     USB__OnRxZeroCopy          (unsigned EpIndex, unsigned NumBytes);
void     USB__OnRxZeroCopyEx        (unsigned EPIndex, unsigned NumBytes, int Done);
void     USB__OnSetupCancel         (void);
void     USB__OnStatusChange        (U8 State);
void     USB__OnSuspend             (void);
void     USB__OnTx                  (unsigned EPIndex);
void     USB__OnTx0Done             (void);
void     USB__Send                  (unsigned EPIndex);
void     USB__UpdateEPHW            (void);
void     USB__WriteEP0FromISR       (const void* pData, unsigned NumBytes, char Send0PacketIfRequired);
int      USB__IsHighSpeedCapable    (void);
int      USB__IsHighSpeedMode       (void);
U8       USB__AllocIF               (void);
U8       USB__AllocAltIF            (U8 InterFaceNo);
U8       USB__AllocEvent            (void);
void     USB__InvalidateEP          (unsigned EPIndex);
void     USB__StallEP0              (void);
void     USB__ResetDataToggleEP     (unsigned EPIndex);
int      USB__AddDeInitHandler      (USB_DEINIT_FUNC * pfHandler);
void     USB__RemovePendingOperation(unsigned EPIndex, U8 SignalTask);
void     USB__EventDataSend         (unsigned EPIndex);
U8       USB__AllocStringDesc       (const char *sString);
void     USB__EmptyBuffer           (unsigned EPIndex);

const U8 * USB__BuildConfigDesc     (void);
const U8 * USB__BuildDeviceDesc     (void);

/*********************************************************************
*
*       InfoBuffer routines
*
**********************************************************************
*/
void USB_IB_Init                (USB_INFO_BUFFER * pInfoBuffer, U8 * pBuffer, unsigned SizeofBuffer);
void USB_IB_AddU8               (USB_INFO_BUFFER * pInfoBuffer, U8  Data);
void USB_IB_AddU16              (USB_INFO_BUFFER * pInfoBuffer, U16 Data);
void USB_IB_AddU32              (USB_INFO_BUFFER * pInfoBuffer, U32 Data);

/*********************************************************************
*
*       Buffer routines
*
**********************************************************************
*/
unsigned BUFFER_Read            (USB_BUFFER * pBuffer,       U8 * pData, unsigned NumBytesReq);
int      BUFFER_Write           (USB_BUFFER * pBuffer, const U8 * pData, unsigned NumBytes);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif // __USB_PRIVATE_H__

/*************************** End of file ****************************/
