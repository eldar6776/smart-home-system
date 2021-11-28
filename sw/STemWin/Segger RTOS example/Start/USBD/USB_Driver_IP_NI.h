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
File    : USB_Driver_IP_NI.h
Purpose : Network interface driver
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USB_DRIVER_IP_NI_H
#define USB_DRIVER_IP_NI_H

/*********************************************************************
*
*       defines, non-configurable
*/

#define USB_IP_NI_ETH_HEADER_SIZE   14        // Number of bytes in the header of an Ethernet frame
#define USB_IP_NI_HW_ADDR_SIZE      6         // Number of bytes in the HW address

/*********************************************************************
*
*       Link status
*/
#define USB_IP_NI_LINK_STATUS_DISCONNECTED        0
#define USB_IP_NI_LINK_STATUS_CONNECTED           1

/*********************************************************************
*
*       IDs of the statistical counters
*/
#define USB_IP_NI_STATS_WRITE_PACKET_OK           0
#define USB_IP_NI_STATS_WRITE_PACKET_ERROR        1
#define USB_IP_NI_STATS_READ_PACKET_OK            2
#define USB_IP_NI_STATS_READ_PACKET_ERROR         3
#define USB_IP_NI_STATS_READ_NO_BUFFER            4
#define USB_IP_NI_STATS_READ_ALIGN_ERROR          5
#define USB_IP_NI_STATS_WRITE_ONE_COLLISION       6
#define USB_IP_NI_STATS_WRITE_MORE_COLLISIONS     7


#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef int USB_IP_WRITE_PACKET(const void * pData, U32 NumBytes);  // Callback function to write a packet to USB host

/*********************************************************************
*
*       USB_IP_NI_DRIVER_DATA
*/
typedef struct USB_IP_NI_DRIVER_DATA {
  const U8 * pHWAddr;         // HW address of host interface
  unsigned   NumBytesHWAddr;  // Size of HW address in bytes
} USB_IP_NI_DRIVER_DATA;

/*********************************************************************
*
*       USB_IP_NI_DRIVER_API
*/
typedef struct USB_IP_NI_DRIVER_API {
  void   (*pfInit)             (const USB_IP_NI_DRIVER_DATA * pDriverData, USB_IP_WRITE_PACKET *pfWritePacket);
  void * (*pfGetPacketBuffer)  (unsigned NumBytes);
  void   (*pfWritePacket)      (const void * pData, unsigned NumBytes);
  void   (*pfSetPacketFilter)  (U32 Mask);
  int    (*pfGetLinkStatus)    (void);
  U32    (*pfGetLinkSpeed)     (void);
  void   (*pfGetHWAddr)        (U8 * pAddr, unsigned NumBytes);
  U32    (*pfGetStats)         (int Type);
  U32    (*pfGetMTU)           (void);
  void   (*pfReset)            (void);
} USB_IP_NI_DRIVER_API;

/*********************************************************************
*
*       Communication drivers
*
**********************************************************************
*/
extern USB_IP_NI_DRIVER_API USB_Driver_IP_NI;    // Network interface

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
