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

File    : IP_COAP_Int.h
Purpose : Internal header file for the Constrained Application Protocol (CoAP)
          Client / Server application.
*/

#ifndef IP_COAP_INT_H         // Avoid multiple inclusion.
#define IP_COAP_INT_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif


#include "SEGGER.h"
#include "IP_COAP.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

//
// Internal definition.
//
#define IP_COAP_CODE_REQ_POST_EXISTING    IP_COAP_CODE2NUM(0u, 5u)
#define IP_COAP_CODE_REQ_PUT_NEW          IP_COAP_CODE2NUM(0u, 6u)

#define IP_COAP_OPTMASK_ETAG_MATCH        (1uL << 31)
#define IP_COAP_OPTMASK_IS_MULTICAST      (1uL << 30)

#define IP_COAP_OBS_UPDATE_REQ_CON        (1u << 7)
#define IP_COAP_OBS_UPDATE_REQ_NON        (1u << 6)
#define IP_COAP_OBS_CONNECT_ONGOING       (1u << 5)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

//
// Option parsing definition.
//
typedef struct {
  U8  Option;
  U16 Length;
  U8* pValue;
} IP_COAP_OPTION;

//
// Internal return value.
//
enum {
  IP_COAP_ERR_OPT_CRITICAL         = IP_COAP_RETURN_OPTION_ERROR,
  IP_COAP_ERR_MSG_IGNORE           = IP_COAP_RETURN_NO_PAYLOAD,
  IP_COAP_ERR_BUFFER_TOO_SMALL     = IP_COAP_RETURN_BUFFER_TOO_SMALL,
  IP_COAP_ERR_PROXY                = IP_COAP_RETURN_PROXY_NOT_SUPPORTED,
  IP_COAP_ERR_MSG_INVALID          = IP_COAP_RETURN_ERR,

  IP_COAP_ERR_OK                   = IP_COAP_RETURN_OK,
  IP_COAP_ERR_MSG_END              = 1,
  IP_COAP_ERR_PAYLOAD              = 2
} ;


/*********************************************************************
*
*       Types
*
**********************************************************************
*/

//
// Early parsing.
//
typedef struct {
  IP_COAP_OPTIONS_INFO*  pOptDesc;
  U8*                    pIFMatchETag;
  U8*                    pETag;
  U8*                    pURI;
  const U8*              pHostname;
  U16                    BadOption;
  U8                     URILength;
  U8                     IFMatchAll;
  U8                     IFMatchETagNumber;
  U8                     NumEtag;
} IP_COAP_EARLY_PARSING;

//
// Block definition for the formatting of URI.
//
typedef struct {
  U16 BlockSize;      // Overall block size.
  U16 RemainingSize;  // Number of bytes left in this block.
  U16 PreviousTaken;  // Number of bytes used from the previous block.
  U8  URIEnd;         // The end of the URI is in this block.
} IP_COAP_BLOCK_DATA;

/*********************************************************************
*
*       API functions / Function prototypes
*
**********************************************************************
*/

//
// Read.
//
int IP_COAP_ParseHeader  (U8** ppBuffer, U16* pLength, IP_COAP_HEADER_INFO* pHeader);
int IP_COAP_ParseOptions (U8** ppBuffer, U16* pLength, IP_COAP_EARLY_PARSING* pEarly, U32* pOptMask);
int IP_COAP_GetNextOption(U8** ppBuffer, U16* pLength, IP_COAP_OPTION* pOptData, int PrevOpt);

//
// Write.
//
int IP_COAP_BuildHeader        (U8** ppBuffer, U16* pLength, IP_COAP_HEADER_INFO* pHeader);
int IP_COAP_AddOptionsFromDesc (U8** ppBuffer, U16* pLength, IP_COAP_OPTIONS_INFO* pOptions, U32 OptionMask);
int IP_COAP_AddFormatedURI     (U8** ppBuffer, U16* pLength, const U8* pURI);
int IP_COAP_AddFormatedURIBlock(U8** ppBuffer, U16* pLength, const U8* pURI, IP_COAP_BLOCK_DATA* pBlock);
int IP_COAP_AddPayloadMarker   (U8** ppBuffer, U16* pLength);
int IP_COAP_ClearMoreBlock     (IP_COAP_OPTIONS_INFO* pOptions);

//
// Utility.
//
U8  IP_COAP_DefaultBlockSize      (U16 MsgBufferSize, U16 BlockSize);
U16 IP_COAP_GetBlockSize          (U32 BlockValue);
U16 IP_COAP_GetRetryInitialTimeout(U32 Time);
int IP_COAP_IsTimeElapsed         (U32 CurrentTime, U32 RefTime);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
