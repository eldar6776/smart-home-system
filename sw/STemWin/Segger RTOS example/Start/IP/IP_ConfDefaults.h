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
File    : IP_ConfDefaults.h
Purpose : Defines defaults for most configurable defines used in the stack.
          If you want to change a value, please do so in IP_Conf.h, do NOT modify this file.
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef IP_CONFDEFAULTS_H
#define IP_CONFDEFAULTS_H

#include <string.h>  // Required for memset
#include "IP_Conf.h"

//
// Operating system interface. For embOS,
// the functions can be inlined
//
#ifndef   IP_OS_DISABLE_INTERRUPT
  #define IP_OS_DISABLE_INTERRUPT         IP_OS_DisableInterrupt
#endif

#ifndef   IP_OS_ENABLE_INTERRUPT
  #define IP_OS_ENABLE_INTERRUPT          IP_OS_EnableInterrupt
#endif

#ifndef   IP_OS_GET_TIME
  #define IP_OS_GET_TIME                  IP_OS_GetTime32
#endif

//
// IP_OS_UNLOCK might be defined in IP_Conf.h that is
// meant to be kept by the customer when updating the
// stack. In a release build typically the OS functions
// shall be inlined directly if possible. IP_OS_UNLOCK()
// we no longer want to inline as we want to signal the
// IP_Task() from an unlock if a packet has been freed
// while the locking was active.
// To avoid changes to the customers IP_Conf.h we simply
// undefine IP_OS_UNLOCK() here again and define it with
// our default OS layer function.
//
#ifdef    IP_OS_UNLOCK
  #undef  IP_OS_UNLOCK
#endif
#ifndef   IP_OS_UNLOCK
  #define IP_OS_UNLOCK()                  IP_OS_Unlock()
#endif

#ifndef   IP_OS_LOCK
  #define IP_OS_LOCK()                    IP_OS_Lock()
#endif

#ifndef   IP_OS_WAIT_NET_EVENT
  #define IP_OS_WAIT_NET_EVENT(ms)        IP_OS_WaitNetEvent(ms)
#endif

#ifndef   IP_OS_WAIT_RX_EVENT
  #define IP_OS_WAIT_RX_EVENT()           IP_OS_WaitRxEvent()
#endif

#ifndef   IP_OS_SIGNAL_RX_EVENT
  #define IP_OS_SIGNAL_RX_EVENT()         IP_OS_SignalRxEvent()
#endif

#ifndef   IP_DEBUG
  #define IP_DEBUG                        0       // Debug level: 0: Release, 1: Support "Panic" checks, 2: Support warn & log
#endif

#ifndef   IP_MEMCPY
  #define IP_MEMCPY                       memcpy
#endif

#ifndef   IP_MEMSET
  #define IP_MEMSET                       memset
#endif

#ifndef   IP_MEMMOVE
  #define IP_MEMMOVE                      memmove
#endif

#ifndef   IP_MEMCMP
  #define IP_MEMCMP                       memcmp
#endif

#ifndef   IP_SNPRINTF
  #define IP_SNPRINTF                     snprintf
#endif

#ifndef   IP_STRLEN
  #define IP_STRLEN                       strlen
#endif

#ifndef   IP_STRCMP
  #define IP_STRCMP                       strcmp
#endif

#ifndef   IP_CKSUM
  #define IP_CKSUM(p, NumHWords, Sum)     IP_cksum(p, NumHWords, Sum)
#endif

#ifndef   IP_OPTIMIZE
  #define IP_OPTIMIZE
#endif

#ifndef   IP_IS_BIG_ENDIAN
  #define IP_IS_BIG_ENDIAN                0       // Little endian is default
#endif

#ifndef   IP_USE_PARA                             // Some compiler complain about unused parameters.
  #define IP_USE_PARA(Para) (void)Para            // This works for most compilers.
#endif

#ifndef   IP_INCLUDE_STAT
  #define IP_INCLUDE_STAT (IP_DEBUG > 0)          // Can be set to 0 to disable statistics for extremly small release builds
#endif

#ifndef   IP_DEBUG_FIFO                           // Allow override in IP_Conf.h
  #define IP_DEBUG_FIFO IP_DEBUG
#endif

#ifndef   IP_DEBUG_MEM                            // Allow override in IP_Conf.h
  #define IP_DEBUG_MEM (IP_DEBUG > 1)
#endif

#ifndef   IP_MAX_ADD_ETH_TYPES
  #define IP_MAX_ADD_ETH_TYPES            2
#endif

//
// TCP retransmission range defaults
//
#ifndef   IP_TCP_RETRANS_MIN
  #define IP_TCP_RETRANS_MIN              210     // Min. delay for retransmit. Real delay is computed, this minimum applies only if computed delay is shorter. Min should be > 200, since 200 ms is a typ. value for delayed ACKs
#endif

#ifndef   IP_TCP_RETRANS_MAX
  #define IP_TCP_RETRANS_MAX              5000    // Max. delay for retransmit. Real delay is computed, this maximum applies only if computed delay is longer.
#endif

//
// TCP keep-alive defaults
//
#ifndef   IP_TCP_KEEPALIVE_INIT
  #define IP_TCP_KEEPALIVE_INIT           10000   // Initial connect keep alive [ms]
#endif

#ifndef   IP_TCP_KEEPALIVE_IDLE
  #define IP_TCP_KEEPALIVE_IDLE           10000   // Default time before probing [ms]
#endif

#ifndef   IP_TCP_KEEPALIVE_PERIOD
  #define IP_TCP_KEEPALIVE_PERIOD         10000   // Default probe interval [ms]
#endif

#ifndef   IP_TCP_KEEPALIVE_MAX_REPS
  #define IP_TCP_KEEPALIVE_MAX_REPS       8       // Max probes before drop
#endif

#ifndef   IP_TCP_MSL
  #define IP_TCP_MSL                      100     // Max segment lifetime. Used primarily for the TCP TIME_WAIT state. Large value wastes a lot of time!
#endif

#ifndef   IP_SOCKET_MAX_CONN
  #define IP_SOCKET_MAX_CONN              16      // Max. Backlog for a socket in listening state. The maximum length of the queue of pending connections
#endif


#ifndef   IP_TCP_PERIOD
  #define IP_TCP_PERIOD                   10
#endif

#ifndef   TCP_PERS_MIN_TIME
  #define TCP_PERS_MIN_TIME               500     // Min. persistence time
#endif

#ifndef   TCP_PERS_MAX_TIME
  #define TCP_PERS_MAX_TIME               6000    // Max. persistence time
#endif

#ifndef   IP_TCP_SUPPORT_TIMESTAMP
  #define IP_TCP_SUPPORT_TIMESTAMP        1       // Do we support RFC-1323 TCP timestamp feature to compute RTT ?
#endif

#ifndef   IP_SUPPORT_MULTICAST
  #define IP_SUPPORT_MULTICAST            1
#endif

#ifndef   IP_UPNP_FULFIL_SPEC
  #define IP_UPNP_FULFIL_SPEC             0       // Fulfill the UPnP spec. This has been tested under windows and does not seem to be needed
#endif

#ifndef   IP_MAX_DNS_SERVERS
  #define IP_MAX_DNS_SERVERS              2
#endif

#ifndef IP_PANIC
  #if   IP_DEBUG
    #define IP_PANIC(s)                   IP_Panic(s)
  #else
    #define IP_PANIC(s)
  #endif
#endif

#ifndef IP_SUPPORT_LOG
  #if   (IP_DEBUG > 1)
    #define IP_SUPPORT_LOG                1
  #else
    #define IP_SUPPORT_LOG                0
  #endif
#endif

#ifndef IP_SUPPORT_WARN
  #if   (IP_DEBUG > 1)
    #define IP_SUPPORT_WARN               1
  #else
    #define IP_SUPPORT_WARN               0
  #endif
#endif


#if IP_INCLUDE_STAT
  #define IP_STAT_DEC(Cnt)                (Cnt)--
  #define IP_STAT_INC(Cnt)                (Cnt)++
  #define IP_STAT_ADD(Cnt, v)             { Cnt += v; }
#else
  #define IP_STAT_DEC(Cnt)
  #define IP_STAT_INC(Cnt)
  #define IP_STAT_ADD(Cnt, v)
#endif

#ifndef   IP_MAX_IFACES
  #define IP_MAX_IFACES                   1       // Maximum number of interfaces to support at one time
#endif

#ifndef   IP_MAX_TIMERS
  #define IP_MAX_TIMERS                   10
#endif

#ifndef   IP_TCP_DELAY_ACK_DEFAULT
  #define IP_TCP_DELAY_ACK_DEFAULT        200     // [ms]
#endif

#ifndef   DO_DELAY_ACKS
  #define DO_DELAY_ACKS                   1       // Defining enables delayed acks
#endif

#ifndef   IP_PTR_OP_IS_ATOMIC
  #define IP_PTR_OP_IS_ATOMIC             1
#endif

#ifndef   IP_ALLOW_NOLOCK
  #define IP_ALLOW_NOLOCK                 ((IP_MAX_IFACES == 1) ? 1 : 0)
#endif

#ifndef   IP_ALLOW_DEINIT
  #define IP_ALLOW_DEINIT                 0       // IP_DeInit() can be used to de-initialize the stack
#endif

#ifndef   IP_PPP_RESEND_TIMEOUT
  #define IP_PPP_RESEND_TIMEOUT           2000
#endif

#ifndef   IP_SUPPORT_VLAN
  #define IP_SUPPORT_VLAN                 1
#endif

#ifndef   IP_SUPPORT_MICREL_TAIL_TAGGING
  #define IP_SUPPORT_MICREL_TAIL_TAGGING  1
#endif

#ifndef   IP_IFACE_REROUTE
  #define IP_IFACE_REROUTE                (IP_SUPPORT_VLAN || IP_SUPPORT_MICREL_TAIL_TAGGING)
#endif

#ifndef   IP_ARP_SNIFF_ON_RX
  #define IP_ARP_SNIFF_ON_RX              1       // Create a short time ARP entry on Rx to avoid sending ARPs if we could already know the destination
#endif

#ifndef   IP_NUM_MULTICAST_ADDRESSES
  #define IP_NUM_MULTICAST_ADDRESSES      5
#endif

#ifndef   IP_NUM_LINK_UP_PROBES
  #define IP_NUM_LINK_UP_PROBES           1       // Link probes before a link up is used by the stack. 1 means direct, 0 disables the code.
#endif

#ifndef   IP_NUM_LINK_DOWN_PROBES
  #define IP_NUM_LINK_DOWN_PROBES         1       // Link probes before a link down is used by the stack. 1 means direct, 0 disables the code.
#endif

//
// Allow ICMP to answer for a packet that has been sent to a broadcast or multicast destination.
// Default is OFF as this might instrument us for a DDOS attack in case someone sends an ICMP
// packet to a broadcast or multicast address with a spoofed address of a target to attack.
//
#ifndef   IP_ICMP_RX_ALLOW_BC_MC
  #define IP_ICMP_RX_ALLOW_BC_MC          0
#endif

//
// IPv6 add-on related configuration defaults
//
#ifndef   IP_SUPPORT_IPV6
  #define IP_SUPPORT_IPV6                 0       // Switch for the IPv6 add-on.
#endif

#ifndef   IP_IPV6_DNS_MAX_IPV6_SERVER
  #define IP_IPV6_DNS_MAX_IPV6_SERVER     1
#endif

#ifndef   IP_IPV6_DNS_MAX_NUM_IPV6_ADDR
  #define IP_IPV6_DNS_MAX_NUM_IPV6_ADDR   1
#endif

//
// Log buffer related configuration defaults.
//
#ifndef     IP_LOG_BUFFER_SIZE
  #if IP_SUPPORT_IPV6
    #define IP_LOG_BUFFER_SIZE            160
  #else
    #define IP_LOG_BUFFER_SIZE            100
  #endif
#endif

//
// Profiling/SystemView related configuration defaults.
//
#ifndef   IP_SUPPORT_PROFILE
  #define IP_SUPPORT_PROFILE           0
#endif

#ifndef   IP_SUPPORT_PROFILE_END_CALL  
  #define IP_SUPPORT_PROFILE_END_CALL  0
#endif

#ifndef   IP_SUPPORT_PROFILE_FIFO
  #define IP_SUPPORT_PROFILE_FIFO      0
#endif

//
// Statistics related configuration defaults.
// By default the global IP_SUPPORT_STATS enables
// all specific stats defines.
//
#ifndef   IP_SUPPORT_STATS
  #define IP_SUPPORT_STATS                0
#endif

#ifndef   IP_SUPPORT_STATS_IFACE
  #define IP_SUPPORT_STATS_IFACE          IP_SUPPORT_STATS
#endif


#endif // Avoid multiple inclusion

/*************************** End of file ****************************/
