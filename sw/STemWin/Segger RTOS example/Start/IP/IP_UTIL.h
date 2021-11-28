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
File        : IP_UTIL.h
Purpose     : UTIL API
---------------------------END-OF-HEADER------------------------------
*/

#ifndef IP_UTIL_H
#define IP_UTIL_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

int IP_UTIL_BASE64_Encode(const unsigned char * pSrc, int SrcLen, unsigned char *pDest, int * pDestLen);
int IP_UTIL_BASE64_Decode(const unsigned char * pSrc, int SrcLen, unsigned char *pDest, int * pDestLen);


#if defined(__cplusplus)
  }
#endif

#endif   // Avoid multiple inclusion



