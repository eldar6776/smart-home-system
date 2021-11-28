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
File    : USB_UVC.h
Purpose : Public header of the human interface device class
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USBD_UVC_H          /* Avoid multiple inclusion */
#define USBD_UVC_H

#include "SEGGER.h"
#include "USB_Private.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Config defaults
*
**********************************************************************
*/
#define USBD_UVC_NUM_BUFFERS                           10
#define USBD_UVC_DATA_BUFFER_SIZE                      1024 // Should match the MaxPacketSize
#define USBD_UVC_SUPPORTED_ALT_SETINGS                 10

#define USBD_UVC_WIDTH                                 (unsigned int)320
#define USBD_UVC_HEIGHT                                (unsigned int)240
#define USBD_UVC_CAM_FPS                               30
#define USBD_UVC_MIN_BIT_RATE                          (unsigned long)(USBD_UVC_WIDTH*USBD_UVC_HEIGHT*16*USBD_UVC_CAM_FPS)//16 bit
#define USBD_UVC_MAX_BIT_RATE                          (unsigned long)(USBD_UVC_WIDTH*USBD_UVC_HEIGHT*16*USBD_UVC_CAM_FPS)
#define USBD_UVC_MAX_FRAME_SIZE                        (unsigned long)(USBD_UVC_WIDTH*USBD_UVC_HEIGHT*2)//yuy2
//#define USBD_UVC_MAX_FRAME_SIZE                        (unsigned long)(USBD_UVC_WIDTH*USBD_UVC_HEIGHT*3/2)//nv12
#define USBD_UVC_INTERVAL                              (unsigned long)(10000000/USBD_UVC_CAM_FPS)
#define USBD_UVC_DEFAULT_COMPRESSION_INDEX             5

#define USBD_UVC_PROBE_REQUEST                         0x0100
#define USBD_UVC_COMMIT_REQUEST                        0x0200
#define USBD_UVC_STILL_PROBE                           0x0300
#define USBD_UVC_STILL_COMMIT                          0x0400
#define USBD_UVC_STILL_TRIGGER                         0x0400

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#define USBD_UVC_PAYLOAD_HEADER_SIZE 2

#define USBD_UVC_END_OF_FRAME (1 << 1)
#define USBD_UVC_BIT_FRAME_ID (1 << 0)

#define USB_DEVICE_DESC_SIZE        (sizeof(USB_DEVICE_DESCRIPTOR))
#define USB_CONFIGUARTION_DESC_SIZE (char)9
#define USB_INTERFACE_DESC_SIZE     (char)9
#define USB_ENDPOINT_DESC_SIZE      (char)7
#define UVC_INTERFACE_ASSOCIATION_DESC_SIZE (char)8
#define UVC_VC_ENDPOINT_DESC_SIZE   (char)5

#define UVC_VC_INTERFACE_HEADER_DESC_SIZE(n)  (char)(12+n)
#define UVC_CAMERA_TERMINAL_DESC_SIZE(n)      (char)(15+n)
#define UVC_OUTPUT_TERMINAL_DESC_SIZE(n)      (char)(9+n)
#define UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(a,b) (char) (13+a*b)


#define VS_FORMAT_UNCOMPRESSED_DESC_SIZE          (0x1b)
#define VS_FORMAT_MJPEG_DESC_SIZE                 (0x0b)
#define VS_FRAME_UNCOMPRESSED_DESC_SIZE           (0x26)
#define VS_STILL_IMAGE_DESC_SIZE                  (0x0B)
#define VS_COLOR_MATCHING_DESC_SIZE               (0x06)

#define USBD_UVC_VCIF_NUM                          0
#define USBD_UVC_VSIF_NUM                          1

#define UVC_TOTAL_IF_NUM                           2

// USB Video device class specification version 1.00
#define UVC_VERSION                               0x0100

// Video Interface Class Codes
// (USB_Video_Class_1.1.pdf, A.1 Video Interface Class Code)
#define CC_VIDEO                                  0x0E

// Video Interface Subclass Codes
// (USB_Video_Class_1.1.pdf, A.2 Video Interface Subclass Code)
#define SC_UNDEFINED                              0x00
#define SC_VIDEOCONTROL                           0x01
#define SC_VIDEOSTREAMING                         0x02
#define SC_VIDEO_INTERFACE_COLLECTION             0x03

// Video Interface Protocol Codes
// (USB_Video_Class_1.1.pdf, A.3 Video Interface Protocol Codes)
#define PC_PROTOCOL_UNDEFINED                     0x00

// Video Class-Specific Descriptor Types
// (USB_Video_Class_1.1.pdf, A.4 Video Class-Specific Descriptor Types)
#define CS_UNDEFINED                              0x20
#define CS_DEVICE                                 0x21
#define CS_CONFIGURATION                          0x22
#define CS_STRING                                 0x23
#define CS_INTERFACE                              0x24
#define CS_ENDPOINT                               0x25

// Video Class-Specific VideoControl Interface Descriptor Subtypes
// (USB_Video_Class_1.1.pdf, A.5 Video Class-Specific VC Interface Descriptor Subtypes)
#define VC_DESCRIPTOR_UNDEFINED                   0x00
#define VC_HEADER                                 0x01
#define VC_INPUT_TERMINAL                         0x02
#define VC_OUTPUT_TERMINAL                        0x03
#define VC_SELECTOR_UNIT                          0x04
#define VC_PROCESSING_UNIT                        0x05
#define VC_EXTENSION_UNIT                         0x06

// Video Class-Specific VideoStreaming Interface Descriptor Subtypes
// (USB_Video_Class_1.1.pdf, A.6 Video Class-Specific VS Interface Descriptor Subtypes)
#define VS_UNDEFINED                              0x00
#define VS_INPUT_HEADER                           0x01
#define VS_OUTPUT_HEADER                          0x02
#define VS_STILL_IMAGE_FRAME                      0x03
#define VS_FORMAT_UNCOMPRESSED                    0x04
#define VS_FRAME_UNCOMPRESSED                     0x05
#define VS_FORMAT_MJPEG                           0x06
#define VS_FRAME_MJPEG                            0x07
#define VS_FORMAT_MPEG2TS                         0x0A
#define VS_FORMAT_DV                              0x0C
#define VS_COLORFORMAT                            0x0D
#define VS_FORMAT_FRAME_BASED                     0x10
#define VS_FRAME_FRAME_BASED                      0x11
#define VS_FORMAT_STREAM_BASED                    0x12

// Video Class-Specific Endpoint Descriptor Subtypes
// (USB_Video_Class_1.1.pdf, A.7 Video Class-Specific Endpoint Descriptor Subtypes)
#define EP_UNDEFINED                              0x00
#define EP_GENERAL                                0x01
#define EP_ENDPOINT                               0x02
#define EP_INTERRUPT                              0x03

// Video Class-Specific Request Codes
// (USB_Video_Class_1.1.pdf, A.8 Video Class-Specific Request Codes)
#define RC_UNDEFINED                              0x00
#define SET_CUR                                   0x01
#define GET_CUR                                   0x81
#define GET_MIN                                   0x82
#define GET_MAX                                   0x83
#define GET_RES                                   0x84
#define GET_LEN                                   0x85
#define GET_INFO                                  0x86
#define GET_DEF                                   0x87

// VideoControl Interface Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.1 VideoControl Interface Control Selectors)
#define VC_CONTROL_UNDEFINED                      0x00
#define VC_VIDEO_POWER_MODE_CONTROL               0x01
#define VC_REQUEST_ERROR_CODE_CONTROL             0x02

// Request Error Code Control
// (USB_Video_Class_1.1.pdf, 4.2.1.2 Request Error Code Control)
#define NO_ERROR_ERR                              0x00
#define NOT_READY_ERR                             0x01
#define WRONG_STATE_ERR                           0x02
#define POWER_ERR                                 0x03
#define OUT_OF_RANGE_ERR                          0x04
#define INVALID_UNIT_ERR                          0x05
#define INVALID_CONTROL_ERR                       0x06
#define INVALID_REQUEST_ERR                       0x07
#define UNKNOWN_ERR                               0xFF

// Terminal Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.2 Terminal Control Selectors)
#define TE_CONTROL_UNDEFINED                      0x00

// Selector Unit Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.3 Selector Unit Control Selectors)
#define SU_CONTROL_UNDEFINED                      0x00
#define SU_INPUT_SELECT_CONTROL                   0x01

// Camera Terminal Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.4 Camera Terminal Control Selectors)
#define CT_CONTROL_UNDEFINED                      0x00
#define CT_SCANNING_MODE_CONTROL                  0x01
#define CT_AE_MODE_CONTROL                        0x02
#define CT_AE_PRIORITY_CONTROL                    0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL         0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL         0x05
#define CT_FOCUS_ABSOLUTE_CONTROL                 0x06
#define CT_FOCUS_RELATIVE_CONTROL                 0x07
#define CT_FOCUS_AUTO_CONTROL                     0x08
#define CT_IRIS_ABSOLUTE_CONTROL                  0x09
#define CT_IRIS_RELATIVE_CONTROL                  0x0A
#define CT_ZOOM_ABSOLUTE_CONTROL                  0x0B
#define CT_ZOOM_RELATIVE_CONTROL                  0x0C
#define CT_PANTILT_ABSOLUTE_CONTROL               0x0D
#define CT_PANTILT_RELATIVE_CONTROL               0x0E
#define CT_ROLL_ABSOLUTE_CONTROL                  0x0F
#define CT_ROLL_RELATIVE_CONTROL                  0x10
#define CT_PRIVACY_CONTROL                        0x11

// Processing Unit Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.5 Processing Unit Control Selectors)
#define PU_CONTROL_UNDEFINED                      0x00
#define PU_BACKLIGHT_COMPENSATION_CONTROL         0x01
#define PU_BRIGHTNESS_CONTROL                     0x02
#define PU_CONTRAST_CONTROL                       0x03
#define PU_GAIN_CONTROL                           0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL           0x05
#define PU_HUE_CONTROL                            0x06
#define PU_SATURATION_CONTROL                     0x07
#define PU_SHARPNESS_CONTROL                      0x08
#define PU_GAMMA_CONTROL                          0x09
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL      0x0A
#define PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL 0x0B
#define PU_WHITE_BALANCE_COMPONENT_CONTROL        0x0C
#define PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL   0x0D
#define PU_DIGITAL_MULTIPLIER_CONTROL             0x0E
#define PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL       0x0F
#define PU_HUE_AUTO_CONTROL                       0x10
#define PU_ANALOG_VIDEO_STANDARD_CONTROL          0x11
#define PU_ANALOG_LOCK_STATUS_CONTROL             0x12

// Extension Unit Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.6 Extension Unit Control Selectors)
#define XU_CONTROL_UNDEFINED                      0x00

// VideoStreaming Interface Control Selectors
// (USB_Video_Class_1.1.pdf, A.9.7 VideoStreaming Interface Control Selectors)
#define VS_CONTROL_UNDEFINED                      0x00
#define VS_PROBE_CONTROL                          0x01
#define VS_COMMIT_CONTROL                         0x02
#define VS_STILL_PROBE_CONTROL                    0x03
#define VS_STILL_COMMIT_CONTROL                   0x04
#define VS_STILL_IMAGE_TRIGGER_CONTROL            0x05
#define VS_STREAM_ERROR_CODE_CONTROL              0x06
#define VS_GENERATE_KEY_FRAME_CONTROL             0x07
#define VS_UPDATE_FRAME_SEGMENT_CONTROL           0x08
#define VS_SYNC_DELAY_CONTROL                     0x09


// Defined Bits Containing Capabilities of the Control
// (USB_Video_Class_1.1.pdf, 4.1.2 Table 4-3 Defined Bits Containing Capabilities of the Control)
#define SUPPORTS_GET                              0x01
#define SUPPORTS_SET                              0x02
#define STATE_DISABLED                            0x04
#define AUTOUPDATE_CONTROL                        0x08
#define ASYNCHRONOUS_CONTROL                      0x10

// USB Terminal Types
// (USB_Video_Class_1.1.pdf, B.1 USB Terminal Types)
#define TT_VENDOR_SPECIFIC                        0x0100
#define TT_STREAMING                              0x0101

// Input Terminal Types
// (USB_Video_Class_1.1.pdf, B.2 Input Terminal Types)
#define ITT_VENDOR_SPECIFIC                       0x0200
#define ITT_CAMERA                                0x0201
#define ITT_MEDIA_TRANSPORT_INPUT                 0x0202

// Output Terminal Types
// (USB_Video_Class_1.1.pdf, B.3 Output Terminal Types)
#define OTT_VENDOR_SPECIFIC                       0x0300
#define OTT_DISPLAY                               0x0301
#define OTT_MEDIA_TRANSPORT_OUTPUT                0x0302

// External Terminal Types
// (USB_Video_Class_1.1.pdf, B.4 External Terminal Types)
#define EXTERNAL_VENDOR_SPECIFIC                  0x0400
#define COMPOSITE_CONNECTOR                       0x0401
#define SVIDEO_CONNECTOR                          0x0402
#define COMPONENT_CONNECTOR                       0x0403


#define USB_VIDEO_DESC_SIZ (unsigned long)(\
                    USB_CONFIGUARTION_DESC_SIZE +\
                    UVC_INTERFACE_ASSOCIATION_DESC_SIZE +\
                    USB_INTERFACE_DESC_SIZE +  \
                    UVC_VC_INTERFACE_HEADER_DESC_SIZE(1) + \
                    UVC_CAMERA_TERMINAL_DESC_SIZE(2) + \
                    UVC_OUTPUT_TERMINAL_DESC_SIZE(0) + \
                    USB_INTERFACE_DESC_SIZE +   \
                    UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(1,1) + \
                    VS_FORMAT_UNCOMPRESSED_DESC_SIZE +  \
                    VS_FRAME_UNCOMPRESSED_DESC_SIZE  +  \
                    VS_COLOR_MATCHING_DESC_SIZE  +\
                    USB_INTERFACE_DESC_SIZE +  \
                    USB_ENDPOINT_DESC_SIZE)

#define VC_TERMINAL_SIZ (unsigned int)(UVC_VC_INTERFACE_HEADER_DESC_SIZE(1) + UVC_CAMERA_TERMINAL_DESC_SIZE(2) + UVC_OUTPUT_TERMINAL_DESC_SIZE(0))
#define VC_HEADER_SIZ (unsigned int)(UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(1,1) + VS_FORMAT_MJPEG_DESC_SIZE + VS_FRAME_UNCOMPRESSED_DESC_SIZE + VS_COLOR_MATCHING_DESC_SIZE)

#define UVC_DBVAL(x) (x & 0xFF),((x >> 8) & 0xFF),((x >> 16) & 0xFF),((x >> 24) & 0xFF)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef int USBD_UVC_HANDLE;
typedef void USB_UVC_ON_RESOLUTION_CHANGE(unsigned FrameIndex);

typedef struct _USBD_UVC_DATA_BUFFER{
  U8  Data[USBD_UVC_DATA_BUFFER_SIZE];
  U8  Flags;
  U16 NumBytesIn;
  U8  FrameID;
} USBD_UVC_DATA_BUFFER;

typedef struct _USBD_UVC_BUFFER {
  USBD_UVC_DATA_BUFFER  Buf[USBD_UVC_NUM_BUFFERS];
  unsigned              NumBlocksIn;
  unsigned              RdPos;
  unsigned              WrPos;
} USBD_UVC_BUFFER;

typedef struct _USBD_UVC_INIT_DATA {
  U8                EPIn;
  USBD_UVC_BUFFER * pBuf;
} USBD_UVC_INIT_DATA;


typedef struct _USBD_UVC_FRAME_SETTING {
  U16 Width;
  U16 Height;
  U32 MinBitRate;
  U32 MaxBitRate;
  U32 MaxFrameSize;
  U32 MinInterval;
  U32 MaxInterval;
} USBD_UVC_FRAME_SETTING;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
USBD_UVC_HANDLE USBD_UVC_Add                    (const USBD_UVC_INIT_DATA * pInitData);
int             USBD_UVC_AddFrameSetting        (USBD_UVC_HANDLE hInst, const USBD_UVC_FRAME_SETTING * pAltSetting);
int             USBD_UVC_Write                  (USBD_UVC_HANDLE hInst, const void* pData, unsigned NumBytes, U8 Flags);
void            USBD_UVC_SetOnResolutionChange  (USBD_UVC_HANDLE hInst, USB_UVC_ON_RESOLUTION_CHANGE * pfOnResChange);
#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
