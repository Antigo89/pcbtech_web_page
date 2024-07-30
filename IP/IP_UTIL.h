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
----------------------------------------------------------------------
File        : IP_UTIL.h
Purpose     : UTIL API
---------------------------END-OF-HEADER------------------------------
*/

#ifndef IP_UTIL_H
#define IP_UTIL_H

#include "SEGGER.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef struct {
  unsigned NumBytesInContext;
  U16      Carry;  // Up to 2 leftover bytes from the previous chunk (3 would have been encoded) are stored in FIFO order.
} IP_UTIL_BASE64_CONTEXT;

int IP_UTIL_BASE64_Decode     (const U8* pSrc, int SrcLen, U8* pDest, int* pDestLen);
int IP_UTIL_BASE64_Encode     (const U8* pSrc, int SrcLen, U8* pDest, int* pDestLen);
int IP_UTIL_BASE64_EncodeChunk(IP_UTIL_BASE64_CONTEXT* pContext, const U8* pSrc, int SrcLen, U8* pDest, int* pDestLen, char IsLastChunk);


#if defined(__cplusplus)
  }
#endif

#endif   // Avoid multiple inclusion



