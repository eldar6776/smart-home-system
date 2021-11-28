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
-------------------------- END-OF-HEADER -----------------------------

File    : IP_SNMP_AGENT_Int.h
Purpose : Internal header file for SNMP agent.
*/

#include "IP_SNMP_AGENT.h"

#include <stdio.h>   // For NULL.
#include <string.h>  // For memset(), memcpy(), strcmp() and memcmp() .

#ifndef IP_SNMP_AGENT_INT_H       // Avoid multiple inclusion.
#define IP_SNMP_AGENT_INT_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   IP_SNMP_AGENT_WARN
  #define IP_SNMP_AGENT_WARN(p)
#endif

#ifndef   IP_SNMP_AGENT_LOG
  #define IP_SNMP_AGENT_LOG(p)
#endif

#ifndef   IP_SNMP_AGENT_SUPPORT_PANIC_CHECK
  #define IP_SNMP_AGENT_SUPPORT_PANIC_CHECK   0  // Disabled by default.
#endif

#if IP_SNMP_AGENT_SUPPORT_PANIC_CHECK
  #define IP_SNMP_AGENT_PANIC()               _Panic()
#else
  #define IP_SNMP_AGENT_PANIC()
#endif

#ifndef   IP_SNMP_AGENT_WORK_BUFFER
  #define IP_SNMP_AGENT_WORK_BUFFER           64  // 64 bytes should fit for most needs and is enough to store ~32 '.' separated IDs into a string for debug outputs.
#endif

#ifndef   IP_SNMP_AGENT_MEMCPY
  #include "string.h"  // For memcpy() .
  #define IP_SNMP_AGENT_MEMCPY                memcpy
#endif

#ifndef   IP_SNMP_AGENT_MEMSET
  #include "string.h"  // For memset() .
  #define IP_SNMP_AGENT_MEMSET                memset
#endif

#ifndef   IP_SNMP_AGENT_MEMCMP
  #include "string.h"  // For memcmp() .
  #define IP_SNMP_AGENT_MEMCMP                memcmp
#endif

#ifndef   IP_SNMP_AGENT_STRCMP
  #include "string.h"  // For strcmp() .
  #define IP_SNMP_AGENT_STRCMP                strcmp
#endif

#ifndef   IP_SNMP_AGENT_STRLEN
  #include "string.h"  // For strlen() .
  #define IP_SNMP_AGENT_STRLEN                strlen
#endif

#ifndef   IP_SNMP_AGENT_FUNCTION_NAME
  #define IP_SNMP_AGENT_FUNCTION_NAME         __func__
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define IP_SNMP_BYTES2OID(x, y)  ((U8)(((x & 0x03) * 40) + (y % 40)))

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  IP_SNMP_AGENT_MIB* pLastMIBAccessed;  // Last MIB accessed will be saved before calling the callback for a MIB as some store functions should be aware of pMIB without forwarding it to the callback.
  U8*                pLastOIDStored;    // Remember the last OID stored to the output buffer for a getbulk-request to use it for the next repeater.
  U32                LastOIDLenStored;  // Remember the length of the last OID stored to the output buffer for a getbulk-request to use it for the next repeater.
  U32                LastMIBLenStored;  // Remember the length of MIB part of the last OID stored to the output buffer for a getbulk-request to use it for the next repeater.
  U8                 ExcNoObjUsed;
  U8                 ExcInstNAUsed;
  U8                 ExcNoNextUsed;
  U8                 OIDStored;         // Has to be set by all public store OID API.
} IP_SNMP_AGENT_CONTEXT_VARBIND_FLAGS;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

      int                IP_SNMP_AGENT__AddMIB(const U8* pParentOID, U32 ParentOIDLen, IP_SNMP_AGENT_MIB* pMIB, IP_SNMP_AGENT_pfMIB pf, U8 FirstOIDs, U32 Id, void* pContext);
      IP_SNMP_AGENT_MIB* IP_SNMP_AGENT_FindMIB(const U8* pOID, U32* pLen, U8 FirstOIDs, char ExactMatch);
const IP_SNMP_AGENT_API* IP_SNMP_AGENT_GetAPI (void);
      void               IP_SNMP_AGENT_Lock   (void);
      void               IP_SNMP_AGENT_Unlock (void);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
