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
--------  END-OF-HEADER  ---------------------------------------------
File    : SEGGER_RTT_Syscalls_SES.c
Purpose : Reimplementation of printf, puts and 
          implementation of __putchar and __getchar using RTT in SES.
          To use RTT for printf output, include this file in your 
          application.
----------------------------------------------------------------------
*/
#include "SEGGER_RTT.h"
#include "__libc.h"
#include <stdarg.h>
#include <stdio.h>

int printf(const char *fmt,...) {
  int n;
  va_list args; 
  va_start (args, fmt);
  n = SEGGER_RTT_vprintf(0, fmt, &args);
  va_end(args);
  return n;
}

int puts(const char *s) {
  return SEGGER_RTT_WriteString(0, s);
}

int __putchar(int x, __printf_tag_ptr ctx) {
  (void)ctx;
  SEGGER_RTT_Write(0, (char *)&x, 1);
  return x;
}

int __getchar() {
  return SEGGER_RTT_WaitKey();
}

/****** End Of File *************************************************/
