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
File    : IPV6_IPv6.h
Purpose : IPv6 header file.
--------  END-OF-HEADER  ---------------------------------------------
*/
#ifndef IPV6_IPV6_H
#define IPV6_IPV6_H                // Avoid multiple/recursive inclusion

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       IPv6 API functions
*/

//
// General API functions
//
void                IP_IPV6_Add                                (unsigned IFaceId);
int                 IP_IPV6_AddUnicastAddr                     (U8 IFaceId, const U8* pAddr);
void                IP_IPV6_GetIPv6Addr                        (U8 IFaceId, U8 AddrId, IPV6_ADDR* pIPv6Addr, U8* pNumAddr);
int                 IP_IPV6_IFaceIsReady                       (unsigned IFaceId);
int                 IP_IPV6_ParseIPv6Addr                      (const char* pIn, IPV6_ADDR* pIPv6Addr);
int                 IP_IPV6_ResolveHost                        (unsigned IFaceId, const char* sHost, IPV6_ADDR* pIPv6Addr, U32 ms);
int                 IP_IPV6_SetDefHopLimit                     (U8 IFaceId, U8 HopLimit);

//
// IP DNS server address
//
IPV6_ADDR*          IP_IPV6_DNS_GetServer                      (unsigned IFaceId, unsigned DNSIndex);
int                 IP_IPV6_DNS_SetServer                      (U8 IFaceId, const char* sDNSServerAddr);

//
// Multicast Listener related API functions
//
int                 IP_ICMPV6_MLD_AddMulticastAddr             (U8 IFaceId, IPV6_ADDR* pMultiCAddr);
int                 IP_ICMPV6_MLD_RemoveMulticastAddr          (U8 IFaceId, IPV6_ADDR* pMultiCAddr);

//
// Internet Control Message Protocol version 6 related API functions
//
int                 IP_ICMPV6_SendEchoReq                      (U32 IFaceId, IPV6_ADDR* pDestAddr, char* pData, unsigned NumBytes, U16 Id, U16 SeqNum);
void                IP_ICMPV6_SetRxHook                        (IP_RX_HOOK* pf);

//
// TCP related API functions
//
void                IP_IPV6_TCP_Add                            (void);

//
// UDP related API functions
//
void                IP_IPV6_UDP_Add                            (void);
IP_UDP_CONNECTION*  IP_IPV6_UDP_AddEchoServer                  (U16 LPort);
IP_PACKET*          IP_IPV6_UDP_Alloc                          (int NumBytesData);
IP_PACKET*          IP_IPV6_UDP_AllocEx                        (unsigned IFaceId, int NumBytesIPHeader, int NumBytesData);
void                IP_IPV6_UDP_Free                           (IP_PACKET* pPacket);
void                IP_IPV6_UDP_GetDestAddr                    (const IP_PACKET* pPacket, void* pDestAddr, int AddrLen);
void                IP_IPV6_UDP_GetSrcAddr                     (const IP_PACKET* pPacket, void* pSrcAddr, int AddrLen);
IP_UDP_CONNECTION*  IP_IPV6_UDP_OpenEx                         (IPV6_ADDR* pFAddr, U16 FPort, IPV6_ADDR* pLAddr, U16 LPort, int (*handler)(IP_PACKET*, void*), void* pContext);
int                 IP_IPV6_UDP_Send                           (int IFaceId, IPV6_ADDR* pFHost, U16 FPort, U16 LPort, IP_PACKET* pPacket);
int                 IP_IPV6_UDP_SendAndFree                    (int IFaceId, IPV6_ADDR* pFHost, U16 FPort, U16 LPort, IP_PACKET* pPacket);


//
// UDP function related defines
// (To avoid code doubling and present a clean API with convenient name prefix.)
//
#define IP_IPV6_UDP_Close          IP_UDP_Close
#define IP_IPV6_FindFreePort       IP_UDP_FindFreePort
#define IP_IPV6_GetFPort           IP_UDP_GetFPort
#define IP_IPV6_GetLPort           IP_UDP_GetLPort
#define IP_IPV6_GetIFIndex         IP_UDP_GetIFIndex
#define IP_IPV6_GetDataPtr         IP_UDP_GetDataPtr
#define IP_IPV6_GetDataSize        IP_UDP_GetDataSize

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif

/****** End Of File *************************************************/
