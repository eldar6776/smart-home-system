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
File    : FTPS_Conf.h
Purpose : FTP server add-on configuration file
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FTPS_CONF_H_
#define _FTPS_CONF_H_ 1

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   DEBUG
  #define DEBUG  0
#endif

//
// Logging
//
#if DEBUG
  #if   defined(__ICCARM__)  // IAR ARM toolchain
    #include "IP.h"
    #define FTPS_WARN(p)  IP_Warnf_Application p
    #define FTPS_LOG(p)   IP_Logf_Application  p
  #elif defined(__ICCRX__)   // IAR RX toolchain
    #include "IP.h"
    #define FTPS_WARN(p)  IP_Warnf_Application p
    #define FTPS_LOG(p)   IP_Logf_Application  p
  #elif defined(__GNUC__)    // GCC toolchain
    #include "IP.h"
    #define FTPS_WARN(p)  IP_Warnf_Application p
    #define FTPS_LOG(p)   IP_Logf_Application  p
  #elif defined(WIN32)       // Microsoft Visual Studio
    void WIN32_OutputDebugStringf(const char * sFormat, ...);
    #define FTPS_WARN(p)  WIN32_OutputDebugStringf p
    #define FTPS_LOG(p)   WIN32_OutputDebugStringf p
  #else                      // Other toolchains
    #define FTPS_WARN(p)
    #define FTPS_LOG(p)
  #endif
#else                        // Release builds
  #define   FTPS_WARN(p)
  #define   FTPS_LOG(p)
#endif

#define FTPS_AUTH_BUFFER_SIZE   32
#define FTPS_BUFFER_SIZE       512
#define FTPS_MAX_PATH          128
#define FTPS_MAX_PATH_DIR       64
#define FTPS_MAX_FILE_NAME      13  // The default is 13 characters, because filenames can not be longer than an 8.3 without long file name support.
                                    // 8.3 + 1 character for string termination
#ifndef   FTPS_SIGN_ON_MSG
  #define FTPS_SIGN_ON_MSG  "Welcome to embOS/IP FTP server"
#endif

#endif     // Avoid multiple inclusion

/*************************** End of file ****************************/
