/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2007 - 2022    SEGGER Microcontroller GmbH               *
*                                                                    *
*       www.segger.com     Support: www.segger.com/ticket            *
*                                                                    *
**********************************************************************
*                                                                    *
*       emNet * TCP/IP stack for embedded applications               *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product for in-house use.         *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       emNet version: V3.42.6                                       *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : IP_FS_ReadOnly.c
Purpose : Implementation of read only file system layer.
*/

#include <stdio.h>   // For NULL.
#include <string.h>  // For memcpy(), strcmp().

#include "IP_FS.h"
#include "WEBS_Conf.h"

#ifndef   WEBS_USE_SAMPLE_2018
  #define WEBS_USE_SAMPLE_2018  0
#endif

#if (WEBS_USE_SAMPLE_2018 == 0)

/*********************************************************************
*
*       Includes for read only files
*
**********************************************************************
*/

#include "BGround.h"                    // png page, required for every sample page
#include "favicon.h"                    // ico file, required for every sample page
#include "Styles.h"                     // CSS file
#include "About.h"                      // HTML page
#include "Authen.h"                     // HTML page
#include "Error404.h"                   // HTML page
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
  const IP_FS_READ_ONLY_FILE_ENTRY* pEntry;
  unsigned                          DirLength;
  unsigned                          LastFolderLength;
  const IP_FS_READ_ONLY_FILE_ENTRY* pPrevious;
} IP_FS_READ_ONLY_CONTEXT;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static IP_FS_READ_ONLY_FILE_HOOK* _pFirstFileHook;

//
// List of static read only files.
// For FTP server, all the entries under the same folder need to be regrouped.
//
static const IP_FS_READ_ONLY_FILE_ENTRY _aFile[] = {
//  path                               file data                               file size
//  -------------------                --------------                          -------------
  { "/Error404.htm",                   error404_file,                          ERROR404_SIZE                    },
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
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _CompareDir()
*/
static int _CompareDir(const char* sDir, const char* sPath) {
  int  i;
  char c0;
  char c1;

  for (i = 0; ; i++) {
    c0 = *sDir++;
    if (c0 == 0) {
      break;
    }
    c1 = *sPath++;
    if (c0 != c1) {
      return 1;  // No match, file not in this directory.
    }
  }
  return 0;      // Match.
}

/*********************************************************************
*
*       _GetFolderLength()
*/
static int _GetFolderLength(const char* sPath) {
  const char     *pEnd;
        unsigned  r;

  pEnd = strstr(sPath, "/");
  if (pEnd == NULL) {
    r = 0u;
  } else {
    r = (unsigned)(pEnd - sPath);
  }
  return r;
}

/*********************************************************************
*
*       _FS_RO_FS_Open()
*/
static void* _FS_RO_FS_Open(const char* sPath) {
  const IP_FS_READ_ONLY_FILE_ENTRY* pEntry;
        IP_FS_READ_ONLY_FILE_HOOK*  pHook;

  //
  // Some applications/samples have been designed with a real
  // filesystem in mind which does not use a root slash "/"
  // at the beginning to describe that the file to open is actually
  // in the root of the medium. The FS/RO layer however has been
  // designed with URIs in mind, which typically start with it.
  // To unify both versions we now skip the root slash as everything
  // this FS/RO layer uses a path relative to root. This then works
  // with paths with and without root slash as both do the same,
  // access a path starting at root, whether this is explicitly
  // stated or not.
  //
  if (*sPath == '/') {
    sPath++;
  }
  //
  // Use dynamically added files list first.
  //
  for (pHook = _pFirstFileHook; pHook != NULL; pHook = pHook->pNext) {
    pEntry = &pHook->FileEntry;
    if (strcmp(sPath, pEntry->sPath + 1) == 0) {
      return (void*)pEntry;
    }
  }
  //
  // Use fixed list.
  //
  pEntry = &_aFile[0];
  for (;;) {
    if (pEntry->sPath == NULL) {
      break;
    }
    if (strcmp(sPath, pEntry->sPath + 1) == 0) {
      return (void*)pEntry;
    }
    pEntry++;
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
  IP_FS_READ_ONLY_FILE_ENTRY* pEntry;

  pEntry = (IP_FS_READ_ONLY_FILE_ENTRY*)hFile;
  memcpy(pDest, pEntry->pData + Pos, NumBytes);
  return 0;
}

/*********************************************************************
*
*       _FS_RO_GetLen()
*/
static long _FS_RO_GetLen(void* hFile) {
  IP_FS_READ_ONLY_FILE_ENTRY* pEntry;

  pEntry = (IP_FS_READ_ONLY_FILE_ENTRY*)hFile;
  return pEntry->FileSize;
}

/*********************************************************************
*
*       _FS_RO_ForEachDirEntry()
*/
static void _FS_RO_ForEachDirEntry(void* pContext, const char* sDir, void (*pf)(void* pContext, void* pFileEntry)) {
  IP_FS_READ_ONLY_CONTEXT     Context;
  IP_FS_READ_ONLY_FILE_ENTRY* pEntry;
  IP_FS_READ_ONLY_FILE_HOOK*  pHook;
  int i;

  Context.DirLength        = (unsigned)strlen(sDir);
  Context.LastFolderLength = 0u;
  Context.pPrevious        = NULL;
  //
  // Use dynamically added files list first.
  //
  for (pHook = _pFirstFileHook; pHook != NULL; pHook = pHook->pNext) {
    pEntry = &pHook->FileEntry;
    if (_CompareDir(sDir, pEntry->sPath) == 0) {
      //
      // Check if this entry corresponds to the folder previously reported.
      //
      if (Context.LastFolderLength != 0u) {
        if (memcmp(Context.pPrevious->sPath, pEntry->sPath, Context.LastFolderLength) == 0) {
          continue;
        }
      }
      Context.pEntry = pEntry;
      pf(pContext, (void*)&Context);
    }
  }
  //
  // Use fixed list.
  // Might report a filename for the second time as
  // we do not check if a filename has been overwritten
  // using the dynamic file list.
  //
  i = 0;
  while (1) {
    if (_aFile[i].sPath == NULL) {
      break;
    }
    if (_CompareDir(sDir, _aFile[i].sPath) == 0) {
      //
      // Check if this entry corresponds to the folder previously reported.
      //
      if (Context.LastFolderLength != 0u) {
        if (memcmp(Context.pPrevious->sPath, _aFile[i].sPath, Context.LastFolderLength) == 0) {
          i++;
          continue;
        }
      }
      Context.pEntry = &_aFile[i];
      pf(pContext, (void*)&Context);
    }
    i++;
  }
}

/*********************************************************************
*
*       _FS_RO_GetFileName()
*/
static void _FS_RO_GetFileName(void* pFileEntry, char* pBuffer, U32 BufferSize) {
        IP_FS_READ_ONLY_CONTEXT*    pContext;
  const IP_FS_READ_ONLY_FILE_ENTRY* pEntry;
  const char* pStart;
  unsigned FilenameLen;
  unsigned n;

  BufferSize--;                             // Reserve one byte for string termination.
  pContext    = (IP_FS_READ_ONLY_CONTEXT*)pFileEntry;
  pEntry      = pContext->pEntry;
  n           = 0u;
  //
  // Check the presence of a folder.
  //
  pStart = pEntry->sPath + pContext->DirLength;
  if (*pStart == '/') {
    pStart++;
    n++;
  }
  FilenameLen = _GetFolderLength(pStart);
  if (FilenameLen == 0u) {
    FilenameLen = (unsigned)strlen(pStart);
  } else {
    pContext->LastFolderLength = pContext->DirLength + n + FilenameLen + 1u; // Add 1 to add the '/' at the end.
    pContext->pPrevious        = pEntry;
  }
  FilenameLen = SEGGER_MIN(FilenameLen, BufferSize);
  memcpy(pBuffer, pStart, FilenameLen);
  *(pBuffer + FilenameLen) = 0;             // Terminate string.
}

/*********************************************************************
*
*       _FS_RO_GetFileSize()
*/
static U32 _FS_RO_GetFileSize(void * pFileEntry, U32 * pFileSizeHigh) {
  const IP_FS_READ_ONLY_CONTEXT*    pContext;
  const IP_FS_READ_ONLY_FILE_ENTRY* pEntry;
  const char* pStart;
  unsigned DirLen;
  U32 r;

  (void)pFileSizeHigh;

  pContext    = (IP_FS_READ_ONLY_CONTEXT*)pFileEntry;
  pEntry      = pContext->pEntry;
  r           = 0uL;  // folder, indicate no length.
  //
  // Check the presence of a folder.
  //
  pStart = pEntry->sPath + pContext->DirLength;
  if (*pStart == '/') {
    pStart++;
  }
  DirLen = _GetFolderLength(pStart);
  if (DirLen == 0u) {
    r = (U32)pEntry->FileSize;
  }
  return r;
}

/*********************************************************************
*
*       _FS_RO_GetFileAttributes()
*/
static int _FS_RO_GetFileAttributes(void* pFileEntry) {
  const IP_FS_READ_ONLY_CONTEXT*    pContext;
  const IP_FS_READ_ONLY_FILE_ENTRY* pEntry;
  const char* pStart;
  unsigned FilenameLen;
  int r;

  pContext    = (IP_FS_READ_ONLY_CONTEXT*)pFileEntry;
  pEntry      = pContext->pEntry;
  r           = 0;
  //
  // Check the presence of a folder.
  //
  pStart = pEntry->sPath + pContext->DirLength;
  if (*pStart == '/') {
    pStart++;
  }
  FilenameLen = _GetFolderLength(pStart);
  if (FilenameLen != 0u) {
    r = 1;  // folder.
  }
  return r;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       IP_FS_READ_ONLY_ClrFileHooks()
*
*  Function description
*    Clears all files that have been dynamically added.
*/
void IP_FS_READ_ONLY_ClrFileHooks(void) {
  _pFirstFileHook = NULL;
}

/*********************************************************************
*
*       IP_FS_READ_ONLY_AddFileHook()
*
*  Function description
*    Adds a file to the list of static read only files.
*
*  Parameters
*    pHook   : Management element of type IP_FS_READ_ONLY_FILE_HOOK.
*    sPath   : Path (filename) of the file to add.
*    pData   : Content of the file.
*    FileSize: Size of content.
*/
void IP_FS_READ_ONLY_AddFileHook(IP_FS_READ_ONLY_FILE_HOOK* pHook, const char* sPath, const unsigned char* pData, unsigned int FileSize) {
  IP_FS_READ_ONLY_FILE_ENTRY* pEntry;

  pHook->pNext     = _pFirstFileHook;
  pEntry           = &pHook->FileEntry;
  pEntry->sPath    = sPath;
  pEntry->pData    = pData;
  pEntry->FileSize = FileSize;
  _pFirstFileHook  = pHook;
}

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/

const IP_FS_API IP_FS_ReadOnly = {
  _FS_RO_FS_Open,            // pfOpenFile
  _FS_RO_Close,              // pfCloseFile
  _FS_RO_ReadAt,             // pfReadAt
  _FS_RO_GetLen,             // pfGetLen
  _FS_RO_ForEachDirEntry,    // pfForEachDirEntry
  _FS_RO_GetFileName,        // pfGetDirEntryFileName
  _FS_RO_GetFileSize,        // pfGetDirEntryFileSize
  NULL,                      // pfGetDirEntryFileTime
  _FS_RO_GetFileAttributes,  // pfGetDirEntryAttributes
  NULL,                      // pfCreate
  NULL,                      // pfDeleteFile
  NULL,                      // pfRenameFile
  NULL,                      // pfWriteAt
  NULL,                      // pfMKDir
  NULL,                      // pfRMDir
  NULL,                      // pfIsFolder
  NULL                       // pfMove
};

#endif

/*************************** End of file ****************************/
