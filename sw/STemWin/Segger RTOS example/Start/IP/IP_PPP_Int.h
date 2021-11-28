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
File        : IP_PPP_Int.h
Purpose     : Internal header file for PPP
---------------------------END-OF-HEADER------------------------------
*/

#ifndef IP_PPP_INT_H
#define IP_PPP_INT_H

#include "IP_Int.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#define MAX_OPT         24   // Maximum option number supported.

#define PPP_PROT_IP        0x0021
#define PPP_PROT_CD        0x00FD            // Compressed datagram
#define PPP_PROT_ILCD      0x00FB            // Individual link compressed datagram
#define PPP_PROT_CCP       0x80FD            // Compression control protocol
#define PPP_PROT_IPCP      0x8021            // IP control protocol
#define PPP_PROT_LCP       0xC021            // Link Configuration Protocol
#define PPP_PROT_SPAP      0xC027            // Shiva Password Authentication Protocol
#define PPP_PROT_CHAP      0xC223            // Challenge Handshake Authentication Protocol
#define PPP_PROT_PAP       0xC023            // Password Authentication Protocol

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef enum {
  PPP_LCP_STATE_NOT_CONNECTED,
  PPP_LCP_STATE_INIT_SENT,
  PPP_LCP_STATE_CONNECTED,
  PPP_LCP_STATE_TERM_REQ_SENT,
  PPP_LCP_STATE_GIVEN_UP
} PPP_LCP_STATE;

typedef enum {
  PPP_CCP_STATE_NOT_CONNECTED,
  PPP_CCP_STATE_INIT_SENT,
  PPP_CCP_STATE_NOT_USED,         // CCP not used, but connection can continue anyhow
  PPP_CCP_STATE_CONNECTED,
  PPP_CCP_STATE_GIVEN_UP
} PPP_CCP_STATE;

typedef enum {
  PPP_IPCP_STATE_NOT_CONNECTED,
  PPP_IPCP_STATE_INIT_SENT,
  PPP_IPCP_STATE_CONNECTED,
  PPP_IPCP_STATE_GIVEN_UP
} PPP_IPCP_STATE;

typedef enum {
  PPP_AUTH_STATE_NOT_CONNECTED,
  PPP_AUTH_STATE_REQ_SENT,
  PPP_AUTH_STATE_ACK,
  PPP_AUTH_STATE_WRONG_PASS,
  PPP_AUTH_STATE_ERROR
} PPP_AUTH_STATE;

typedef int  PPP_SEND_FUNC        (IP_PACKET * pPacket, void * pSendContext);
typedef void PPP_TERM_FUNC        (void * pSendContext);
typedef void PPP_INFORM_USER_FUNC (U32 IFaceId, U32 Status);

typedef struct IP_PPP_CONTEXT {
  PPP_SEND_FUNC *  pfSend;
  PPP_TERM_FUNC *  pfTerm;
  PPP_INFORM_USER_FUNC * pfInformUser;
  void          *  pSendContext;
  int              NumBytesPrepend;
  U8               IFaceId;
  struct {
    U32            NumTries;
    I32            Timeout;
  } Config;
  struct {
    U8             Id;
    U8             aOptCnt[MAX_OPT];  // Used to count the number of negotiations for a particular option, allowing us to terminate hopeless cases
    U8             UsePFC;            // Protocol Filed Compression
    U8             UseACFC;           // Addr. and Control Field Compression
    PPP_LCP_STATE  AState;
    PPP_LCP_STATE  PState;
    RESEND_INFO    Resend;
    U16            MRU;
    U32            ACCM;
    U32            OptMask;
    U32            MagicNumber;
  } LCP;
  struct {
    U8             Id;
    U8             aOptCnt[MAX_OPT];  // Used to count the number of negotiations for a particular option, allowing us to terminate hopeless cases
    PPP_CCP_STATE  AState;
    PPP_CCP_STATE  PState;
    RESEND_INFO    Resend;
    U32            OptMask;
  } CCP;
  struct {
    U8             Id;
    U8             aOptCnt[MAX_OPT];  // Used to count the number of negotiations for a particular option, allowing us to terminate hopeless cases
    PPP_IPCP_STATE AState;
    PPP_IPCP_STATE PState;
    RESEND_INFO    Resend;
    IP_ADDR        IpAddr;
    IP_ADDR        aDNSServer[IP_MAX_DNS_SERVERS];
    U32            OptMask;
  } IPCP;
  struct {
    U8             UserLen;
    U8             abUser[64];
    U8             PassLen;
    U8             abPass[64];
    U16            Prot;              // Number defines the authentication protocol. 0 for None, RFC defined numbers for the others such as PAP, CHAP, ...
    U32            Data;
    PPP_AUTH_STATE State;
    RESEND_INFO    Resend;
    U32            OptMask;
  } Auth;
  IP_PPP_LINE_DRIVER * pLineDriver;
} IP_PPP_CONTEXT;



/*********************************************************************
*
*       PPP functions
*/
void IP_PPP_Disconnect     (IP_PPP_CONTEXT * pContext);
void IP_PPP_Init           (IP_PPP_CONTEXT * pContext, PPP_SEND_FUNC * pfSend, PPP_TERM_FUNC * pfTerm, void * pSendContext, int NumBytesPrepend);
void IP_PPP_OnAuthCompleted(IP_PPP_CONTEXT * pContext);
void IP_PPP_OnLCPConnect   (IP_PPP_CONTEXT * pContext);
void IP_PPP_OnLCPTerminate (IP_PPP_CONTEXT * pContext);
void IP_PPP_OnCCPConnect   (IP_PPP_CONTEXT * pContext);
void IP_PPP_OnIPCPConnect  (IP_PPP_CONTEXT * pContext);
void IP_PPP_OnIPCPTerminate(IP_PPP_CONTEXT * pContext);
void IP_PPP_OnCCPTerminate (IP_PPP_CONTEXT * pContext);
void IP_PPP_OnCCPCompletion(IP_PPP_CONTEXT * pContext);
void IP_PPP_OnRxPacket     (IP_PPP_CONTEXT * pContext, IP_PACKET * pPacket);
void IP_PPP_Reset          (IP_PPP_CONTEXT * pContext);
int  IP_PPP_Send           (IP_PPP_CONTEXT * pContext, IP_PACKET * pPacket);
void IP_PPP_SetAuthInfo    (IP_PPP_CONTEXT * pContext, const char * sUser, const char * sPass);
void IP_PPP_Start          (IP_PPP_CONTEXT * pContext);
void IP_PPP_Timer          (IP_PPP_CONTEXT * pContext);

/*********************************************************************
*
*       PPP LPC functions
*/
void IP_PPP_LCP_OnRx          (IP_PPP_CONTEXT * pContext, IP_PACKET * pPacket);
void IP_PPP_LCP_Reset         (IP_PPP_CONTEXT * pContext);
void IP_PPP_LCP_SendProtReject(IP_PPP_CONTEXT * pContext, IP_PACKET * pPacket);
void IP_PPP_LCP_Start         (IP_PPP_CONTEXT * pContext);
void IP_PPP_LCP_Terminate     (IP_PPP_CONTEXT * pContext);
void IP_PPP_LCP_Timer         (IP_PPP_CONTEXT * pContext);

//
// OO: Temporary workaround where ISPs seem not to follow RFCs
//
void IP_PPP_LCP_ConfigureOptions(U8 MN, U8 PFC, U8 ACFC, U8 PSFC);

/*********************************************************************
*
*       PPP CCP functions
*/
void IP_PPP_CCP_OnRx (IP_PPP_CONTEXT * pContext, IP_PACKET * pPacket);
void IP_PPP_CCP_Reset(IP_PPP_CONTEXT * pContext);
void IP_PPP_CCP_Start(IP_PPP_CONTEXT * pContext);
void IP_PPP_CCP_Timer(IP_PPP_CONTEXT * pContext);

/*********************************************************************
*
*       PPP IPPC functions
*/
void IP_PPP_IPCP_OnRx (IP_PPP_CONTEXT * pContext, IP_PACKET * pPacket);
void IP_PPP_IPCP_Reset(IP_PPP_CONTEXT * pContext);
void IP_PPP_IPCP_Start(IP_PPP_CONTEXT * pContext);
void IP_PPP_IPCP_Timer(IP_PPP_CONTEXT * pContext);

/*********************************************************************
*
*       PPP PAP functions
*/
void IP_PPP_PAP_OnRx (IP_PPP_CONTEXT * pContext, IP_PACKET * pPacket);
void IP_PPP_PAP_Reset(IP_PPP_CONTEXT * pContext);
void IP_PPP_PAP_Start(IP_PPP_CONTEXT * pContext);
void IP_PPP_PAP_Timer(IP_PPP_CONTEXT * pContext);

#if defined(__cplusplus)
  }
#endif

#endif   // Avoid multiple inclusion



