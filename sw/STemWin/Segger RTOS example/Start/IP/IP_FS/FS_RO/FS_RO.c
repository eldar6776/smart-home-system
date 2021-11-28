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
File        : FS_RO.c
Purpose     : Implementation of read only file system
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       Includes
*
**********************************************************************
*/

#include <stdio.h>   // For NULL.
#include <string.h>  // For memcpy(), strcmp().

#include "IP_FS.h"
#include "WEBS_Conf.h"

/*********************************************************************
*
*       RO file system includes
*
**********************************************************************
*/

#include "BGround.h"                    // png page, required for every sample page
#include "favicon.h"                    // ico file, required for every sample page
#include "Styles.h"                     // CSS file
#include "About.h"                      // HTML page
#include "Authen.h"                     // HTML page
#include "FormGET.h"                    // HTML page
#include "FormPOST.h"                   // HTML page
#include "index.h"                      // HTML page
#include "Logo.h"                       // HTML page
#include "OSInf.h"                      // HTML page
#include "IPInf.h"                      // HTML page
#include "SSE_OS.h"                     // HTML page
#include "SSE_IP.h"                     // HTML page
#include "SSE_Time.h"                   // HTML page
#include "VirtFile.h"                   // HTML page
#include "events.h"                     // Javascript library to make SSE usable in IE, required for every pages which uses SSE.

#if WEBS_SUPPORT_UPLOAD
#include "Upl.h"                        // HTML page
#include "Upl_AJAX.h"                   // HTML page
#endif

#if INCLUDE_SHARE_SAMPLE
#include "Shares.h"                     // HTML page
#include "jquery.h"                     // Javascript library, required for Shares.htm
#include "RGraphCC.h"                   // Javascript library, required for Shares.htm
#include "RGraphCE.h"                   // Javascript library, required for Shares.htm
#include "RGraphLi.h"                   // Javascript library, required for Shares.htm
#include "GreenRUp.h"                   // gif file, required for Shares.htm
#include "RedRDown.h"                   // gif file, required for Shares.htm
#include "WhiteR.h"                     // gif file, required for Shares.htm
#endif

#if INCLUDE_PRESENTATION_SAMPLE
#include "Products.h"                   // HTML page
#include "Empty.h"                      // gif file, required for Products.htm
#include "BTL_Det.h"                    // jpg file, required for Products.htm
#include "BTL_Pic.h"                    // jpg file, required for Products.htm
#include "FS_Det.h"                     // jpg file, required for Products.htm
#include "FS_Pic.h"                     // jpg file, required for Products.htm
#include "GUI_Det.h"                    // jpg file, required for Products.htm
#include "GUI_Pic.h"                    // jpg file, required for Products.htm
#include "IP_Det.h"                     // jpg file, required for Products.htm
#include "IP_Pic.h"                     // jpg file, required for Products.htm
#include "OS_Det.h"                     // jpg file, required for Products.htm
#include "OS_Pic.h"                     // jpg file, required for Products.htm
#include "USBD_Det.h"                   // jpg file, required for Products.htm
#include "USBD_Pic.h"                   // jpg file, required for Products.htm
#include "USBH_Det.h"                   // jpg file, required for Products.htm
#include "USBH_Pic.h"                   // jpg file, required for Products.htm
#endif

#if INCLUDE_IP_CONFIG_SAMPLE
#include "IPConf.h"                     // HTML page
#endif

/*********************************************************************
*
*       typedefs
*
**********************************************************************
*/

typedef struct {
  const          char* sFilename;
  const unsigned char* pData;
        unsigned int   FileSize;
} DIR_ENTRY;


static DIR_ENTRY _aFile[] = {
//  file name                          file array                              size
//  -------------------                --------------                          -------------
  { "/favicon.ico",                    favicon_file,                           FAVICON_SIZE                     },
  { "/BGround.png",                    bground_file,                           BGROUND_SIZE                     },
  { "/Styles.css",                     styles_file,                            STYLES_SIZE                      },
  { "/Logo.gif",                       logo_file,                              LOGO_SIZE                        },
  { "/About.htm",                      about_file,                             ABOUT_SIZE                       },
  { "/conf/Authen.htm",                authen_file,                            AUTHEN_SIZE                      },
  { "/OSInf.htm",                      osinf_file,                             OSINF_SIZE                       },
  { "/IPInf.htm",                      ipinf_file,                             IPINF_SIZE                       },
  { "/FormGET.htm",                    formget_file,                           FORMGET_SIZE                     },
  { "/FormPOST.htm",                   formpost_file,                          FORMPOST_SIZE                    },
#if INCLUDE_IP_CONFIG_SAMPLE
  { "/IPConf.htm",                     ipconf_file,                            IPCONF_SIZE                      },
#endif
  { "/index.htm",                      index_file,                             INDEX_SIZE                       },
  { "/SSE_OS.htm",                     sse_os_file,                            SSE_OS_SIZE                      },
  { "/SSE_IP.htm",                     sse_ip_file,                            SSE_IP_SIZE                      },
  { "/SSE_Time.htm",                   sse_time_file,                          SSE_TIME_SIZE                    },
#if WEBS_SUPPORT_UPLOAD
  { "/Upl.htm",                        upl_file,                               UPL_SIZE                         },
  { "/Upl_AJAX.htm",                   upl_ajax_file,                          UPL_AJAX_SIZE                    },
#endif
  { "/VirtFile.htm",                   virtfile_file,                          VIRTFILE_SIZE                    },
  { "/events.js",                      events_file,                            EVENTS_SIZE                      },
#if INCLUDE_SHARE_SAMPLE
  { "/Shares.htm",                     shares_file,                            SHARES_SIZE                      },
  { "/GreenRUp.gif",                   greenrup_file,                          GREENRUP_SIZE                    },
  { "/RedRDown.gif",                   redrdown_file,                          REDRDOWN_SIZE                    },
  { "/WhiteR.gif",                     whiter_file,                            WHITER_SIZE                      },
  { "/jquery.js",                      jquery_file,                            JQUERY_SIZE                      },
  { "/RGraphCC.js",                    rgraphcc_file,                          RGRAPHCC_SIZE                    },
  { "/RGraphCE.js",                    rgraphce_file,                          RGRAPHCE_SIZE                    },
  { "/RGraphLi.js",                    rgraphli_file,                          RGRAPHLI_SIZE                    },
#endif
#if INCLUDE_PRESENTATION_SAMPLE
  { "/Products.htm",                   products_file,                          PRODUCTS_SIZE                    },
  { "/Empty.gif",                      empty_file,                             EMPTY_SIZE                       },
  { "/BTL_Det.jpg",                    btl_det_file,                           BTL_DET_SIZE                     },
  { "/BTL_Pic.jpg",                    btl_pic_file,                           BTL_PIC_SIZE                     },
  { "/FS_Det.jpg",                     fs_det_file,                            FS_DET_SIZE                      },
  { "/FS_Pic.jpg",                     fs_pic_file,                            FS_PIC_SIZE                      },
  { "/GUI_Det.jpg",                    gui_det_file,                           GUI_DET_SIZE                     },
  { "/GUI_Pic.jpg",                    gui_pic_file,                           GUI_PIC_SIZE                     },
  { "/IP_Det.jpg",                     ip_det_file,                            IP_DET_SIZE                      },
  { "/IP_Pic.jpg",                     ip_pic_file,                            IP_PIC_SIZE                      },
  { "/OS_Det.jpg",                     os_det_file,                            OS_DET_SIZE                      },
  { "/OS_Pic.jpg",                     os_pic_file,                            OS_PIC_SIZE                      },
  { "/USBD_Det.jpg",                   usbd_det_file,                          USBD_DET_SIZE                    },
  { "/USBD_Pic.jpg",                   usbd_pic_file,                          USBD_PIC_SIZE                    },
  { "/USBH_Det.jpg",                   usbh_det_file,                          USBH_DET_SIZE                    },
  { "/USBH_Pic.jpg",                   usbh_pic_file,                          USBH_PIC_SIZE                    },
#endif
  { 0,                                 0,                                      0                                }
};

/*********************************************************************
*
*       _CompareDir()
*/
static int _CompareDir(const char* sDir, const char* sFilename) {
  int  i;
  char c0;
  char c1;

  for (i = 0; ; i++) {
    c0 = *sDir++;
    if (c0 == 0) {
      break;
    }
    c1 = *sFilename++;
    if (c0 != c1) {
      return 1;  // No match, file not in this directory.
    }
  }
  return 0;      // Match.
}

/*********************************************************************
*
*       _FS_RO_FS_Open()
*/
static void* _FS_RO_FS_Open(const char* sFilename) {
  DIR_ENTRY* pEntry;
  int        i;

  for (i = 0; ; i++) {
    pEntry = &_aFile[i];
    if (pEntry->sFilename == NULL) {
      break;
    }
    if (strcmp(sFilename, pEntry->sFilename) == 0) {
      return pEntry;
    }
  }
  return NULL;
}

/*********************************************************************
*
*       _FS_RO_Close()
*/
static int _FS_RO_Close (void* hFile) {
  (void)hFile;

  return 0;
}

/*********************************************************************
*
*       _FS_RO_ReadAt()
*/
static int _FS_RO_ReadAt(void* hFile, void* pDest, U32 Pos, U32 NumBytes) {
  DIR_ENTRY* pEntry;

  pEntry = (DIR_ENTRY*)hFile;
  memcpy(pDest, pEntry->pData + Pos, NumBytes);
  return 0;
}

/*********************************************************************
*
*       _FS_RO_GetLen()
*/
static long _FS_RO_GetLen(void* hFile) {
  DIR_ENTRY* pEntry;

  pEntry = (DIR_ENTRY*)hFile;
  return pEntry->FileSize;
}

/*********************************************************************
*
*       _FS_RO_ForEachDirEntry()
*/
static void _FS_RO_ForEachDirEntry(void* pContext, const char* sDir, void (*pf)(void* pContext, void* pFileEntry)) {
  int i;

  for (i = 0; ; i++) {
    if (_aFile[i].sFilename == NULL) {
      break;
    }
    if (_CompareDir(sDir, _aFile[i].sFilename) == 0) {
      pf(pContext, &_aFile[i]);
    }
  }
}

/*********************************************************************
*
*       _FS_RO_GetFileName()
*/
static void _FS_RO_GetFileName(void* pFileEntry, char* sBuffer, U32 SizeofBuffer) {
  DIR_ENTRY* pEntry;

  (void)SizeofBuffer;

  pEntry = (DIR_ENTRY*)pFileEntry;
  strcpy(sBuffer, pEntry->sFilename + 1);
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

const IP_FS_API IP_FS_ReadOnly = {
  _FS_RO_FS_Open,          // pfOpenFile
  _FS_RO_Close,            // pfCloseFile
  _FS_RO_ReadAt,           // pfReadAt
  _FS_RO_GetLen,           // pfGetLen
  _FS_RO_ForEachDirEntry,  // pfForEachDirEntry
  _FS_RO_GetFileName,      // pfGetDirEntryFileName
  NULL,                    // pfGetDirEntryFileSize
  NULL,                    // pfGetDirEntryFileTime
  NULL,                    // pfGetDirEntryAttributes
  NULL,                    // pfCreate
  NULL,                    // pfDeleteFile
  NULL,                    // pfRenameFile
  NULL,                    // pfWriteAt
  NULL,                    // pfMKDir
  NULL,                    // pfRMDir
  NULL,                    // pfIsFolder
  NULL                     // pfMove
};

/*************************** End of file ****************************/

