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
File        : IP_WebServer.h
Purpose     : Publics for the WebServer
---------------------------END-OF-HEADER------------------------------

Attention : Do not modify this file !

Note:
  TimeDate is a 32 bit variable in the following format:
    Bit 0-4:   2-second count (0-29)
    Bit 5-10:  Minutes (0-59)
    Bit 11-15: Hours (0-23)
    Bit 16-20: Day of month (1-31)
    Bit 21-24: Month of year (1-12)
    Bit 25-31: Count of years from 1980 (0-127)
*/

#ifndef  IP_WEBS_H
#define  IP_WEBS_H

#ifdef __ICCARM__  // IAR
  #pragma diag_suppress=Pa029  // No warning for unknown pragmas in earlier verions of EWARM
  #pragma diag_suppress=Pa137  // No warning for C-Style-Casts with C++
#endif

#include "IP_FS.h"
#include "WEBS_Conf.h"

/*********************************************************************
*
*       Defaults for config values
*
**********************************************************************
*/

#ifndef   WEBS_IN_BUFFER_SIZE
  #define WEBS_IN_BUFFER_SIZE               256
#endif

#ifndef   WEBS_OUT_BUFFER_SIZE
  #define WEBS_OUT_BUFFER_SIZE              512
#endif

#ifndef   WEBS_TEMP_BUFFER_SIZE
  #define WEBS_TEMP_BUFFER_SIZE             512         // Used as file input buffer and for form parameters
#endif

#ifndef   WEBS_PARA_BUFFER_SIZE
  #define WEBS_PARA_BUFFER_SIZE             0           // Required for dynamic content parameter handling
#endif

#ifndef   WEBS_ERR_BUFFER_SIZE
  #define WEBS_ERR_BUFFER_SIZE              128         // Used in case of connection limit only
#endif

#ifndef   WEBS_AUTH_BUFFER_SIZE
  #define WEBS_AUTH_BUFFER_SIZE             32
#endif

#ifndef   WEBS_FILENAME_BUFFER_SIZE
  #define WEBS_FILENAME_BUFFER_SIZE         32
#endif

#ifndef   WEBS_UPLOAD_FILENAME_BUFFER_SIZE
  #define WEBS_UPLOAD_FILENAME_BUFFER_SIZE  64
#endif

#ifndef   WEBS_SUPPORT_UPLOAD
  #define WEBS_SUPPORT_UPLOAD               0
#endif

#ifndef   WEBS_URI_BUFFER_SIZE
  #define WEBS_URI_BUFFER_SIZE              0
#endif

#ifndef   WEBS_MAX_ROOT_PATH_LEN
  #define WEBS_MAX_ROOT_PATH_LEN            0
#endif

#ifndef   WEBS_STACK_SIZE_CHILD
  #define WEBS_STACK_SIZE_CHILD             (1536 + WEBS_IN_BUFFER_SIZE + WEBS_OUT_BUFFER_SIZE + WEBS_TEMP_BUFFER_SIZE + WEBS_PARA_BUFFER_SIZE + WEBS_AUTH_BUFFER_SIZE + WEBS_FILENAME_BUFFER_SIZE + WEBS_UPLOAD_FILENAME_BUFFER_SIZE)  // This size can not be guaranteed on all systems. Actual size depends on CPU & compiler
#endif

#ifndef   WEBS_USE_PARA                                 // Some compiler complain about unused parameters.
  #define WEBS_USE_PARA(Para)               (void)Para  // This works for most compilers.
#endif

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Enums
*
**********************************************************************
*/

enum {
  METHOD_NONE,
  METHOD_GET,
  METHOD_HEAD,
  METHOD_POST
};

enum {
  HTTP_ENCODING_RAW,
  HTTP_ENCODING_CHUNKED,
  HTTP_ENCODING_FROM_CONTEXT
};

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct WEBS_BUFFER_SIZES {
  U32 NumBytesInBuf;
  U32 NumBytesOutBuf;
  U32 NumBytesFilenameBuf;
  U32 MaxRootPathLen;
  U32 NumBytesParaBuf;
} WEBS_BUFFER_SIZES;

typedef int   (*IP_WEBS_tSend)   (const unsigned char *pData, int len, void *pConnectInfo);
typedef int   (*IP_WEBS_tReceive)(      unsigned char *pData, int len, void *pConnectInfo);
typedef void *(*IP_WEBS_tAlloc)  (U32 NumBytesReq);
typedef void  (*IP_WEBS_tFree)   (void *p);

typedef void *WEBS_OUTPUT;

typedef struct {
  const char *sName;  // e.g. "Counter"
  void  (*pf)(WEBS_OUTPUT *pOutput, const char *sParameters, const char *sValue);
} WEBS_CGI;

typedef struct {
  const char *sName;  // e.g. "Counter.cgi"
  void  (*pf)(WEBS_OUTPUT *pOutput, const char *sParameters);
} WEBS_VFILES;

typedef struct {
  const char *sPath;
  const char *sRealm;
  const char *sUserPass;
} WEBS_ACCESS_CONTROL;

typedef struct {
  const WEBS_CGI      *paCGI;
  WEBS_ACCESS_CONTROL *paAccess;
  void               (*pfHandleParameter)(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue);
  const WEBS_VFILES   *paVFiles;
} WEBS_APPLICATION;

typedef struct {
  U32 DateLastMod;      // Used for "Last modified" header field
  U32 DateExp;          // Used for "Expires" header field
  U8  IsVirtual;
  U8  AllowDynContent;  // Guard to avoid unnecessary parsing of static files.
} IP_WEBS_FILE_INFO;

typedef struct WEBS_FILE_TYPE {
  const char *sExt;
  const char *sContent;
} WEBS_FILE_TYPE;

typedef struct WEBS_FILE_TYPE_HOOK {
  struct WEBS_FILE_TYPE_HOOK *pNext;
         WEBS_FILE_TYPE       FileType;
} WEBS_FILE_TYPE_HOOK;

typedef struct WEBS_IP_API {
  IP_WEBS_tSend    pfSend;
  IP_WEBS_tReceive pfReceive;
} WEBS_IP_API;

typedef struct WEBS_SYS_API {
  IP_WEBS_tAlloc   pfAlloc;
  IP_WEBS_tFree    pfFree;
} WEBS_SYS_API;

typedef struct WEBS_CONTEXT {
  const WEBS_IP_API      *pIP_API;
  const WEBS_SYS_API     *pSYS_API;
  const IP_FS_API        *pFS_API;
  const WEBS_APPLICATION *pApplication;
        void             *pWebsPara;
        void             *pUpload;
} WEBS_CONTEXT;

typedef void (*IP_WEBS_pfGetFileInfo)(const char *sFilename, IP_WEBS_FILE_INFO *pFileInfo);

//
// VFile extension
//
typedef struct WEBS_VFILE_APPLICATION {
  int  (*pfCheckVFile)(const char *sFileName, unsigned *pIndex);
  void (*pfSendVFile) (void *pContextIn, unsigned Index, const char *sFileName, void (*pf)(void *pContextOut, const char *pData, unsigned NumBytes));
} WEBS_VFILE_APPLICATION;

typedef struct WEBS_VFILE_HOOK {
  struct WEBS_VFILE_HOOK        *pNext;
         WEBS_VFILE_APPLICATION *pVFileApp;
         U8                      ForceEncoding;
} WEBS_VFILE_HOOK;

//
// METHOD extension
//
typedef int (*IP_WEBS_pfMethod)(void *pContext, WEBS_OUTPUT *pOutput, const char *sMethod, const char *sAccept, const char *sContentType, const char *sResource, U32 ContentLen);

typedef struct WEBS_METHOD_HOOK {
  struct WEBS_METHOD_HOOK *pNext;
         IP_WEBS_pfMethod  pf;
  const  char             *sURI;
} WEBS_METHOD_HOOK;

//
// Request notifier
//
typedef struct WEBS_REQUEST_NOTIFY_INFO {
  const char *sUri;
        U8    Method;
} WEBS_REQUEST_NOTIFY_INFO;

typedef void (*IP_WEBS_pfRequestNotify)(WEBS_REQUEST_NOTIFY_INFO* pInfo);

typedef struct WEBS_REQUEST_NOTIFY_HOOK {
  struct WEBS_REQUEST_NOTIFY_HOOK *pNext;
         IP_WEBS_pfRequestNotify   pf;
} WEBS_REQUEST_NOTIFY_HOOK;

//
// Pre-content output hook.
//
typedef void (*IP_WEBS_pfPreContentOutput)(WEBS_OUTPUT* pOutput);

typedef struct WEBS_PRE_CONTENT_OUTPUT_HOOK {
  struct WEBS_PRE_CONTENT_OUTPUT_HOOK* pNext;
         IP_WEBS_pfPreContentOutput    pf;
} WEBS_PRE_CONTENT_OUTPUT_HOOK;

#define WEBS_PRE_DYNAMIC_CONTENT_OUTPUT  (1uL << 0)

/*********************************************************************
*
*       extern
*
**********************************************************************
*/

extern const WEBS_CGI            aCGI[];
extern const WEBS_VFILES         aVFiles[];
extern       WEBS_ACCESS_CONTROL aAccessControl[];

/*********************************************************************
*
*       Functions
*
**********************************************************************
*/

extern const IP_FS_API IP_FS_ReadOnly;

//
// General Web Server API functions
//
      void   IP_WEBS_Init                     (WEBS_CONTEXT *pContext, const WEBS_IP_API *pIP_API, const WEBS_SYS_API *pSYS_API, const IP_FS_API *pFS_API, const WEBS_APPLICATION *pApplication);
      int    IP_WEBS_AddUpload                (void);
      int    IP_WEBS_ProcessEx                (WEBS_CONTEXT *pContext, void *pConnectInfo, const char *sRootPath);
      int    IP_WEBS_ProcessLastEx            (WEBS_CONTEXT *pContext, void *pConnectInfo, const char *sRootPath);
      void   IP_WEBS_ConfigBufSizes           (WEBS_BUFFER_SIZES *pBufferSizes);
      int    IP_WEBS_Flush                    (WEBS_OUTPUT *pOutput);

      int    IP_WEBS_Process                  (IP_WEBS_tSend pfSend, IP_WEBS_tReceive pfReceive, void *pConnectInfo, const IP_FS_API *pFS_API, const WEBS_APPLICATION *pApplication);
      int    IP_WEBS_ProcessLast              (IP_WEBS_tSend pfSend, IP_WEBS_tReceive pfReceive, void *pConnectInfo, const IP_FS_API *pFS_API, const WEBS_APPLICATION *pApplication);
      void   IP_WEBS_OnConnectionLimit        (IP_WEBS_tSend pfSend, IP_WEBS_tReceive pfReceive, void *pConnectInfo);

      void * IP_WEBS_GetConnectInfo           (WEBS_OUTPUT *pOutput);
const char * IP_WEBS_GetURI                   (WEBS_OUTPUT *pOutput, char GetFullURI);
      int    IP_WEBS_Redirect                 (WEBS_OUTPUT *pOutput, const char *sFileName, const char *sMimeType);
      void   IP_WEBS_Reset                    (void);
      void * IP_WEBS_RetrieveUserContext      (WEBS_OUTPUT *pOutput);
      int    IP_WEBS_SendHeader               (WEBS_OUTPUT *pOutput, const char *sFileName, const char *sMimeType);
      int    IP_WEBS_SendHeaderEx             (WEBS_OUTPUT *pOutput, const char *sFileName, const char *sMIMEType, U8 ReqKeepCon);
      int    IP_WEBS_SendMem                  (WEBS_OUTPUT *pOutput, const char *s, unsigned NumBytes);
      int    IP_WEBS_SendString               (WEBS_OUTPUT *pOutput, const char *s);
      int    IP_WEBS_SendStringEnc            (WEBS_OUTPUT *pOutput, const char *s);
      int    IP_WEBS_SendUnsigned             (WEBS_OUTPUT *pOutput, unsigned v, unsigned Base, int NumDigits);
      void   IP_WEBS_StoreUserContext         (WEBS_OUTPUT *pOutput, void *pContext);
      void   IP_WEBS_UseRawEncoding           (WEBS_OUTPUT* pOutput);

      int    IP_WEBS_ConfigRootPath           (const char *sRootPath);
      int    IP_WEBS_ConfigUploadRootPath     (const char *sUploadRootPath);
      void   IP_WEBS_ConfigSendVFileHeader    (U8 OnOff);
      void   IP_WEBS_ConfigSendVFileHookHeader(U8 OnOff);
      int    IP_WEBS_GetDecodedStrLen         (const char *sBuffer, int Len);
      int    IP_WEBS_GetNumParas              (const char *sParameters);
      int    IP_WEBS_GetParaValue             (const char *sBuffer, int ParaNum,       char   *sPara, int   ParaLen,       char   *sValue, int   ValueLen);
      int    IP_WEBS_GetParaValuePtr          (const char *sBuffer, int ParaNum, const char **ppPara, int *pParaLen, const char **ppValue, int *pValueLen);
      void   IP_WEBS_DecodeAndCopyStr         (char *pDest, int DestLen, const char *pSrc, int SrcLen);
      int    IP_WEBS_DecodeString             (const char *s);
      void   IP_WEBS_SendLocationHeader       (WEBS_OUTPUT* pOutput, const char* sURI, const char* sCodeDesc);
      void   IP_WEBS_SetFileInfoCallback      (IP_WEBS_pfGetFileInfo pf);
      void   IP_WEBS_SetHeaderCacheControl    (const char* sCacheControl);
      char   IP_WEBS_CompareFilenameExt       (const char *sFilename, const char *sExt);

      void   IP_WEBS_AddFileTypeHook          (WEBS_FILE_TYPE_HOOK *pHook, const char *sExt, const char *sContent);
      void   IP_WEBS_AddRequestNotifyHook     (WEBS_REQUEST_NOTIFY_HOOK* pHook, IP_WEBS_pfRequestNotify pf);
      void   IP_WEBS_AddPreContentOutputHook  (WEBS_PRE_CONTENT_OUTPUT_HOOK* pHook, IP_WEBS_pfPreContentOutput pf, U32 Mask);

//
// Web Server VFile extension API functions
//
      void   IP_WEBS_AddVFileHook             (WEBS_VFILE_HOOK *pHook, WEBS_VFILE_APPLICATION *pVFileApp, U8 ForceEncoding);

//
// Web Server METHOD extension API functions
//
      void   IP_WEBS_METHOD_AddHook           (WEBS_METHOD_HOOK *pHook, IP_WEBS_pfMethod pf, const char *sURI);
      int    IP_WEBS_METHOD_CopyData          (void *pContext, void *pBuffer, unsigned BufferSize);


#if defined(__cplusplus)
  }
#endif


#endif   /* Avoid multiple inclusion */

/*************************** End of file ****************************/




