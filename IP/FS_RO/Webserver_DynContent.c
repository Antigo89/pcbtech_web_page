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

File    : Webserver_DynContent.c
Purpose : Dynamic content for emWeb.
*/

#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include "RTOS.h"
#include "BSP.h"
#include "IP.h"
#else
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#endif

#include "IP_Webserver.h"
#include "WEBS_Conf.h"        // Stack size depends on configuration

#ifndef   WEBS_USE_SAMPLE_2018
  #define WEBS_USE_SAMPLE_2018  0
#endif

#if (WEBS_USE_SAMPLE_2018 == 0)

/*********************************************************************
*
*       Local defines, configurable
*
**********************************************************************
*/

#define MAX_CONNECTION_INFO      10
#define NUM_STOCKS               26
#define NUM_VALUES               30

//
// UPnP
//
#define UPNP_FRIENDLY_NAME     "SEGGER UPnP Demo"
#define UPNP_MANUFACTURER      "SEGGER Microcontroller GmbH"
#define UPNP_MANUFACTURER_URL  "http://www.segger.com"
#define UPNP_MODEL_DESC        "SEGGER Web server with UPnP"
#define UPNP_MODEL_NAME        "SEGGER UPnP Demo"
#define UPNP_MODEL_URL         "http://www.segger.com/emnet.html"

//
// HTTP cookie
//
#define COOKIE_BUFFER_SIZE  (80)  // Buffer size required to form a HTTP only cookie "Set-Cookie: pagecnt=123456789; Max-Age=600; HttpOnly; SameSite=strict\r\n\0".

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

//
// Abstract the OS layer for different use cases.
//
#ifndef   OS_CHECKSTACK
  #define OS_CHECKSTACK  (0)
#endif

#if (OS_CHECKSTACK != 0)
  #define OS_INFO_GET_TASK_STACK_BASE(p)  OS_GetStackBase(p)
  #define OS_INFO_GET_TASK_STACK_SIZE(p)  OS_GetStackSize(p)
  #define OS_INFO_GET_TASK_STACK_USED(p)  OS_GetStackUsed(p)
#else
  #define OS_INFO_GET_TASK_STACK_BASE(p)  ((void*)NULL)
  #define OS_INFO_GET_TASK_STACK_SIZE(p)  (0u)
  #define OS_INFO_GET_TASK_STACK_USED(p)  (0u)
#endif

#ifndef   OS_SUPPORT_SYSSTACK_INFO
  #define OS_SUPPORT_SYSSTACK_INFO  (0)
#endif

#if (OS_SUPPORT_SYSSTACK_INFO != 0)
  #define OS_INFO_GET_SYS_STACK_BASE()  OS_GetSysStackBase()
  #define OS_INFO_GET_SYS_STACK_SIZE()  OS_GetSysStackSize()
#else
  #define OS_INFO_GET_SYS_STACK_BASE()  ((void*)NULL)
  #define OS_INFO_GET_SYS_STACK_SIZE()  (0u)
#endif

#ifndef   OS_TRACKNAME
  #define OS_TRACKNAME  (0)
#endif

#if (OS_TRACKNAME != 0)
  #define OS_INFO_GET_TASK_NAME(p)  OS_GetTaskName(p)
#else
  #define OS_INFO_GET_TASK_NAME(p)  ("n/a")
#endif

#ifndef   OS_SUPPORT_STAT
  #define OS_SUPPORT_STAT  (0)
#endif

#if (OS_SUPPORT_STAT != 0)
  #define OS_INFO_GET_TASK_NUM_ACTIVATIONS(p)  OS_STAT_GetNumActivations(p)
#else
  #define OS_INFO_GET_TASK_NUM_ACTIVATIONS(p)  (0u)
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  int  Price[30];
  int  Size;
  int  RdOff;
  char CompanyName;
} STOCK_INFO;

typedef struct {
  const char*    sFileName;
  const char*    pData;
        unsigned NumBytes;
} VFILE_LIST;

typedef struct {
  WEBS_HEADER_FIELD_HOOK HeaderFieldHook;
  unsigned               NumPagesVisited;               // Page visits/loads based on user cookie.
  char                   acCookie[COOKIE_BUFFER_SIZE];  // Temp-buffer to form a HTTP only cookie.
#if (WEBS_USE_ENHANCED_FORM_HANDLING != 0)
  char                   IgnoreUnknownCGI;              // Ignore unknown CGI callback. Used for SSE with (WEBS_USE_ENHANCED_FORM_HANDLING=1).
#endif
} USER_CONTEXT;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

//
// Variables used for CGI samples.
//
static U32        _Cnt;
static U32        _Percentage = 50;
static char       _aLEDState[8];
static char       _aLEDStateNew[8];
static char       _acFirstName[12];
static char       _acLastName[12];
#if INCLUDE_SHARE_SAMPLE
static STOCK_INFO _StockInfo[26];
#endif
#if INCLUDE_IP_CONFIG_SAMPLE
static char       _DHCPOnOff;
static U32        _IPAddr;
static U32        _IPMask;
static U32        _IPGW;
#endif

//
// Variables used for METHOD hook demonstration (basic REST implementation).
//
static WEBS_METHOD_HOOK             _MethodHook;
static char                         _acRestContent[20];

//
// Variables used for HEADER hook and HTTP cookie demonstration.
//
static WEBS_PRE_CONTENT_OUTPUT_HOOK _PreContentOutputHook;
static WEBS_PROGRESS_HOOK           _ProgressHook;

//
// UPnP, virtual files.
//
static const char _acFile_dummy_xml[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
  "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\r\n"
    "<specVersion>\r\n"
      "<major>1</major>\r\n"
      "<minor>0</minor>\r\n"
    "</specVersion>\r\n"
    "<serviceStateTable>\r\n"
      "<stateVariable>\r\n"
        "<name>Dummy</name>\r\n"
        "<dataType>i1</dataType>\r\n"
      "</stateVariable>\r\n"
    "</serviceStateTable>\r\n"
  "</scpd>";

static const VFILE_LIST _VFileList[] = {
  { "/dummy.xml", _acFile_dummy_xml, sizeof(_acFile_dummy_xml) - 1 }  // Do not count in the null terminator of the string
};

//
//lint -esym(534, IP_WEBS_SendString, IP_WEBS_SendFormattedString, IP_PrintIPAddr, IP_WEBS_SendUnsigned, IP_WEBS_SendMem) don't check output of some functions.
//lint -esym(534, IP_WEBS_SendHeaderEx, IP_WEBS_SendHeader, IP_WEBS_METHOD_CopyData, SEGGER_snprintf, getsockname, IP_SOCK_getsockname)
//

/******************************************************************************************************************************************
*
*       Cookie handling sample functions
*
*******************************************************************************************************************************************
*/

/*********************************************************************
*
*       _cbOnCookie()
*
*  Function description
*    Callback notified about reading the "Cookie: " header field
*    in a HTTP request.
*
*  Parameters
*    pInfo: Structure of type WEBS_HEADER_FIELD_INFO that contains
*           more information like the connection context.
*
*  Return value
*    Header field read     : WEBS_OK
*    Header field untouched: WEBS_HEADER_FIELD_UNTOUCHED
*
*  Additional information
*    This callback is executed if a header line with the field name
*    "Cookie: " has been found (line has to fit into input buffer).
*    In our case, the only thing that this callback does is parsing
*    the last page count value we have told the browser to store
*    into a cookie.
*/
static int _cbOnCookie(WEBS_HEADER_FIELD_INFO* pInfo) {
        USER_CONTEXT* pUserContext;
        int           ValLen;
        int           r;
        char          ac[10];  // We know what to expect for the cookie size as we have created the cookie (max. 9 digits + \0).

  pUserContext = SEGGER_PTR2PTR(USER_CONTEXT, IP_WEBS_RetrieveUserContext(pInfo->pOutput));
  if (pUserContext == NULL) {  // Check that the memory allocation succeeded, otherwise it makes no sense to try to create/update a cookie.
    r = WEBS_HEADER_FIELD_UNTOUCHED;
  } else {
    r = WEBS_OK;
    //
    // Load old page count from cookie.
    //
    ValLen                            = IP_WEBS_HEADER_GetFindToken(pInfo->pOutput, "pagecnt", sizeof("pagecnt") -1, &ac[0], sizeof(ac) - 1);
    if (ValLen > 0) {                       // Token "pagecnt" found ?
      if ((unsigned)ValLen < sizeof(ac)) {  // Value fits into our buffer ? Otherwise ignore cookie value.
        ac[ValLen]                    = '\0';
        pUserContext->NumPagesVisited = SEGGER_atoi(&ac[0]);
      }
    }
  }
  return r;
}

/*********************************************************************
*
*       _cbOnProgressNotify()
*
*  Function description
*    Callback notified about begin and end of processing a request.
*
*  Parameters
*    pInfo: Structure of type WEBS_PROGRESS_INFO that contains more
*           information like the connection context.
*
*  Additional information
*    This callback is used to catch the BEGIN and END of a request
*    that will be processed. In our case the BEGIN is used to
*    allocate some memory to maintain a temporary buffer and the
*    page visit counter over several callbacks. The user context
*    of the connection is used to make the pointer to this chunk
*    of memory available to all other callbacks involved.
*
*    Each time we register a BEGIN we also hook in a catch for the
*    "Cookie: " field in an HTTP header. If the browser sends us
*    a cookie we will get notified about it.
*
*    The END event is used to free the allocated memory once the
*    request has been processed. The next even will be BEGIN again.
*/
static void _cbOnProgressNotify(WEBS_PROGRESS_INFO* pInfo) {
  WEBS_OUTPUT*  pOutput;
  USER_CONTEXT* pUserContext;
  U8            Status;

  pOutput      = pInfo->pOutput;
  Status       = pInfo->Status;
  //
  // Allocate a new user context on BEGIN or get the previously allocated one.
  //
  if (Status == (U8)WEBS_PROGRESS_STATUS_BEGIN) {
    //
    // A request is about to be processed.
    // Context sensitive allocations can easily be done here.
    //
#ifdef _WIN32
    pUserContext = SEGGER_PTR2PTR(USER_CONTEXT, malloc(sizeof(*pUserContext)));
#else
    pUserContext = SEGGER_PTR2PTR(USER_CONTEXT, IP_Alloc(sizeof(*pUserContext)));
#endif
    if (pUserContext != NULL) {
      memset(pUserContext, 0, sizeof(*pUserContext));
      IP_WEBS_HEADER_AddFieldHook(pOutput, &pUserContext->HeaderFieldHook, _cbOnCookie, "Cookie: ");  // Needs to be registered again for each request.
    }
  } else {
    //
    // Get a user context previously allocated on BEGIN.
    //
    pUserContext = SEGGER_PTR2PTR(USER_CONTEXT, IP_WEBS_RetrieveUserContext(pOutput));
  }
  //
  // Handle all progress states except for BEGIN.
  //
  if        (Status == (U8)WEBS_PROGRESS_STATUS_END) {
    //
    // A request has been completed.
    // Now that everything is done, this is the perfect time to free allocated resources.
    //
    pUserContext = SEGGER_PTR2PTR(USER_CONTEXT, IP_WEBS_RetrieveUserContext(pOutput));
    if (pUserContext != NULL) {
#ifdef _WIN32
      free(pUserContext);
#else
      IP_Free(pUserContext);
#endif
    }
    pUserContext = NULL;  // Make sure that the user context is cleaned as it endures reusing the connection.
#if 0
  } else if (Status == (U8)WEBS_PROGRESS_STATUS_METHOD_URI_VER_PARSED) {
    //
    // First line of request has been been parsed.
    // Redirect all requests for .htm or .html pages to index.htm .
    // Enabling this code will prevent the web pages as shipped to work as intended.
    //
    const char* sURI;
    unsigned    Len;

    sURI = IP_WEBS_GetURI(pInfo->pOutput, 0);
    Len  = SEGGER_strlen(sURI);
    if ((memcmp(sURI + Len - 4, ".htm" , sizeof(".htm"))  == 0) ||
        (memcmp(sURI + Len - 5, ".html", sizeof(".html")) == 0)) {
      IP_WEBS_Redirect(pInfo->pOutput, "/index.htm", NULL);
    }
#endif
#if 0
  } else if (Status == (U8)WEBS_PROGRESS_STATUS_HEADER_PARSED) {
    //
    // HTML header and its fields have been been parsed.
    // Cookies have been parsed at this point.
    //
#endif
  }
  IP_WEBS_StoreUserContext(pOutput, pUserContext);
}

/*********************************************************************
*
*       _cbBeforeContentOutput()
*
*  Function description
*    Callback notified right before any content is sent.
*
*  Parameters
*    pOutput: Context for outgoing data.
*
*  Return value
*    Header field read     : WEBS_OK
*    Header field untouched: WEBS_HEADER_FIELD_UNTOUCHED
*
*  Additional information
*    This callback gets executed before the first byte gets sent out
*    by the server, which is typically the header. This is the last
*    chance to add something to the header like create/update our
*    cookie.
*
*    This callback gets executed before any placeholders get filled
*    in a page that is sent out. In our case this is the place where
*    we increment the page count. The updated page count will then
*    sent out when the placeholder gets filled in.
*/
static int _cbBeforeContentOutput(WEBS_OUTPUT* pOutput) {
  USER_CONTEXT* pUserContext;

  pUserContext = SEGGER_PTR2PTR(USER_CONTEXT, IP_WEBS_RetrieveUserContext(pOutput));
  if (pUserContext != NULL) {         // Check that the memory allocation succeeded, otherwise it makes no sense to try to create/update a cookie.
    pUserContext->NumPagesVisited++;  // We are now serving one more page.
    //
    // Create/update the cookie. The content has to remain valid until the request
    // has been served (or at least the header). Use IP_WEBS_AddProgressHook() to
    // catch the END event. Then it is safe to free our resources in any case.
    // The cookie in this sample consists of the following parts (delimiter is semi-colon):
    //   "Set-Cookie: "   : Name of the header field to include when sending the HTTP header. Tells the browser to create/update a cookie or parts of it.
    //   "pagecnt=%u"     : Cookie para/value pair to create/update. Multiple para/value pairs can be used. US-ASCII characters excluding CTLs,
    //                      whitespace DQUOTE, comma, semicolon, and backslash. Value does not need to be double-quoted but is adviced for non numeric
    //                      values. If unsure about the, the value should be base64 encoded as this results in valid characters only.
    //                      Please be aware that if a cookie value is stored with quotes, the browser will return the quotes as well!
    //   "Max-Age=600"    : Time-To-Live of the cookie in the browser in seconds, beginning with the browser receiving the cookie.
    //                      An empty cookie with Max-Age=0 can be used to remove a cookie: "Set-Cookie: Max-Age=0".
    //   "HttpOnly"       : Tell the browser to not forward the cookie content to its JavaScript interpreter.
    //   "SameSite=strict": Tell the browser to not send the cookie to other cross-site requests, preventing xss attacks.
    //   "\r\n"           : All lines added have to follow the HTTP syntax which is that lines end with CRLF.
    //
    (void)SEGGER_snprintf(&pUserContext->acCookie[0], sizeof(pUserContext->acCookie), "Set-Cookie: pagecnt=%u; Max-Age=600; HttpOnly; SameSite=strict\r\n", pUserContext->NumPagesVisited);
    IP_WEBS_HEADER_SetCustomFields(pOutput, SEGGER_PTR2PTR(const char, &pUserContext->acCookie[0]));
  }
  return WEBS_OK;
}

/******************************************************************************************************************************************
*
*       Custom error pages
*
*******************************************************************************************************************************************
*/

/*********************************************************************
*
*       _cbSendCustomErrorPage()
*
*  Function description
*    Callback notified in case an error page shall be sent.
*
*  Parameters
*    pInfo: Structure of type WEBS_SEND_ERROR_PAGE_INFO that contains
*           more information like the connection context.
*
*  Return value
*    Custom error page sent              : WEBS_OK
*    Let the server sent its default page: WEBS_NO_ERROR_PAGE_SENT
*/
static int _cbSendCustomErrorPage(WEBS_SEND_ERROR_PAGE_INFO* pInfo) {
  int r;

  r = WEBS_NO_ERROR_PAGE_SENT;

  switch (pInfo->ErrorCode) {
  case 404u:
    (void)IP_WEBS_Redirect(pInfo->pOutput, "/Error404.htm", NULL);
    break;
  case 503u:
    IP_WEBS_SendString(pInfo->pOutput, "\r\n<html><body>");
    IP_WEBS_SendFormattedString(pInfo->pOutput, "<h1>Error %d</h1>", pInfo->ErrorCode);
    IP_WEBS_SendString(pInfo->pOutput, "</body></html>");
    r = WEBS_OK;
    break;
  default:
    break;
  }
  return r;
}

/******************************************************************************************************************************************
*
*       CGI sample functions
*
*******************************************************************************************************************************************
*/

/*********************************************************************
*
*       _CopyString
*
*  Function description
*    Local helper function. Copies a string.
*/
static void _CopyString(char* sDest, const char* sSrc, int DestSize) {
  char c;

  while (--DestSize > 0) {
    c = *sSrc++;
    if (c == 0) {
      break;
    }
    *sDest++ = c;
  }
  *sDest = 0;
}

/*********************************************************************
*
*       _callback_DefaultHandler
*/
static void _callback_DefaultHandler(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
#if (WEBS_USE_ENHANCED_FORM_HANDLING != 0)
  USER_CONTEXT* pUserContext;
  char          IgnoreUnknownCGI;

  IgnoreUnknownCGI = 0;

  //
  // When using enhanced form handling all parameters including lastEventId
  // and other randomly added parameters added by the browser for an SSE eventsource
  // need to be handled by callbacks. As there is no way to make sure that all current
  // and future randomly added parameters will be supressed instead of sending back
  // an unknown CGI error text. Therefore we have to remember that we do not want to
  // send it for this connection.
  // Luckily the first parameter sent seems to always be lastEventId, so we can use
  // it as switch.
  //
  pUserContext = SEGGER_PTR2PTR(USER_CONTEXT, IP_WEBS_RetrieveUserContext(pOutput));
  if (pUserContext!= NULL) {                            // Did we get some memory for management ?
    IgnoreUnknownCGI = pUserContext->IgnoreUnknownCGI;  // ==> Use the previously saved switch state. Default is do not ignore if not set before.
    //
    // If we do not suppress the output so far, check if we have to do it from now on.
    // Compare case insensitive so we do not get tricked if some browser sends the
    // lastEventId string all lowercase.
    //
    if (IgnoreUnknownCGI == 0) {
      if (SEGGER_strcasecmp(sPara, "lastEventId") == 0) {
        IgnoreUnknownCGI               = 1;
        pUserContext->IgnoreUnknownCGI = IgnoreUnknownCGI;
      }
    }
  }
  //
  // Output the error about the unknown CGI if not suppressed.
  //
  if (IgnoreUnknownCGI == 0)
#endif
  {
    WEBS_USE_PARA(sValue);

    IP_WEBS_SendString(pOutput, "<h1 style=\"color:red\">Unknown CGI ");
    IP_WEBS_SendString(pOutput, sPara);
    IP_WEBS_SendString(pOutput, " - Ignored</h1>");
  }
}

/*********************************************************************
*
*       _callback_ExecCookieCounter
*/
static void _callback_ExecCookieCounter(WEBS_OUTPUT* pOutput, const char* sParameters, const char* sValue) {
  USER_CONTEXT* pUserContext;
  unsigned      NumPagesVisited;

  WEBS_USE_PARA(sParameters);  // Always called as a placeholder.
  WEBS_USE_PARA(sValue);

  NumPagesVisited = 1;         // Always start with 1 in case we have no cookie or user context could not be allocated.
  pUserContext    = SEGGER_PTR2PTR(USER_CONTEXT, IP_WEBS_RetrieveUserContext(pOutput));
  if (pUserContext != NULL) {  // Check that the memory allocation succeeded.
    //
    // Use page counter from cookie. If we had no cookie before
    // the counter will simply remain 0.
    //
    NumPagesVisited = pUserContext->NumPagesVisited;
  }
  IP_WEBS_SendFormattedString(pOutput, "<br><span class=\"hint\">Page visits based on cookie: %u</span>", NumPagesVisited);
}

/*********************************************************************
*
*       _callback_ExecCounter
*/
static void _callback_ExecCounter(WEBS_OUTPUT* pOutput, const char* sParameters, const char* sValue) {
  WEBS_USE_PARA(sParameters);
  WEBS_USE_PARA(sValue);

  _Cnt++;
  IP_WEBS_SendFormattedString(pOutput, "<br><span class=\"hint\">Current page hit count: %lu</span>", _Cnt);
}

#if IP_SUPPORT_IPV6

/*********************************************************************
*
*       _GetNetworkInfo
*
*  Function description
*    Sends the network configuration of the target infromation
*    and the connection information to the browser.
*/
static void _GetNetworkInfo(WEBS_OUTPUT* pOutput) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<H2>This sample runs on a Windows host.<br>No network statistics available.</H2>");
#else
  IP_CONNECTION_HANDLE hConnection[MAX_CONNECTION_INFO];
  IP_CONNECTION_HANDLE hTemp;
  U16                  sConnection[MAX_CONNECTION_INFO];
  U16                  sTemp;
  IP_IPV6_CONNECTION   Info;
  int                  NumConnections;
  int                  NumValidConnections;
  int                  i;
  int                  j;
  int                  minIndex;
  int                  r;
  char                 acBuffer[40];
  U32                  IPAddr;
  IPV6_ADDR            IPv6Addr;
  U8                   NumIPv6Addr;

  IP_WEBS_SendString(pOutput, "<h2>Configuration</h2>");
  IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  IP_WEBS_SendString(pOutput, "<b>-- IPv4 --------------------</b><br>");
  IP_WEBS_SendString(pOutput, "Server IP<br>");
  IP_WEBS_SendString(pOutput, "Gateway IP<br><br>");
  IP_WEBS_SendString(pOutput, "<b>-- IPv6 --------------------</b><br>");
  IP_WEBS_SendString(pOutput, "Server IPv6<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"colon\">");
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendString(pOutput, ":<br>");
  IP_WEBS_SendString(pOutput, ":<br>");
  IP_WEBS_SendString(pOutput, "<br><br>");
  IP_WEBS_SendString(pOutput, ":<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"value\">");
  IP_WEBS_SendString(pOutput, "<br>");
  //
  // Get IPv4 address
  //
  IPAddr = IP_GetIPAddr(0);
  IP_PrintIPAddr(acBuffer, IPAddr, sizeof(acBuffer));
  IP_WEBS_SendString(pOutput, acBuffer);
  IP_WEBS_SendString(pOutput, "<br>");
  //
  // Get gateway address
  //
  IPAddr = IP_GetGWAddr(0);
  IP_PrintIPAddr(acBuffer, IPAddr, sizeof(acBuffer));
  IP_WEBS_SendString(pOutput, acBuffer);
  IP_WEBS_SendString(pOutput, "<br><br><br>");
  //
  // Get IPv6 address
  //
  IP_IPV6_GetIPv6Addr(0, 0, &IPv6Addr, &NumIPv6Addr);
  SEGGER_snprintf(acBuffer, sizeof(acBuffer), "%n", &IPv6Addr.Union.aU8[0]);
  IP_WEBS_SendString(pOutput, acBuffer);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  OS_EnterRegion();
#if 0
  //
  // Buffer configuration of the stack
  //
  IP_WEBS_SendString(pOutput, "<h2>Stack buffer Info</h2>");
  IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  IP_WEBS_SendString(pOutput, "Small buffers<br>");
  IP_WEBS_SendString(pOutput, "Small buffer size<br>");
  IP_WEBS_SendString(pOutput, "Big buffer<br>");
  IP_WEBS_SendString(pOutput, "Big buffer size<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"colon\">");
  IP_WEBS_SendString(pOutput, ":<br>:<br>:<br>:<br>");
  IP_WEBS_SendString(pOutput, "<div class=\"value\">");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_AVAIL), 10, 0);
  IP_WEBS_SendString(pOutput, "/");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_CONFIG), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_SIZE), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_AVAIL), 10, 0);
  IP_WEBS_SendString(pOutput, "/");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_CONFIG), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_SIZE), 10, 0);
  IP_WEBS_SendString(pOutput, "</div>");
#endif
  IP_WEBS_SendString(pOutput, "<h2>Connection info</h2>");
  //
  // Number of connections
  //
  IP_WEBS_SendString(pOutput, "Total connections: ");
  NumConnections = IP_INFO_GetConnectionList(&hConnection[0], MAX_CONNECTION_INFO);
  NumValidConnections = NumConnections;
  for (i = 0; i < NumConnections; i++) {
    r = IP_IPV6_INFO_GetConnectionInfo(hConnection[i], &Info);
    sConnection[i] = Info.hSocket;
    if (r != 0) {
      NumValidConnections--;
    }
  }
  IP_WEBS_SendUnsigned(pOutput, NumValidConnections, 10, 0);
  //
  // Sort by hSock
  //
  for (i = 0; i < NumConnections; i++) {
    minIndex = i;
    for(j = i + 1; j < NumConnections; j++) {
      if(sConnection[j] < sConnection[minIndex]) {
        minIndex = j;
      }
    }
    if(i != minIndex) {
      sTemp = sConnection[i];
      hTemp = hConnection[i];
      sConnection[i] = sConnection[minIndex];
      hConnection[i] = hConnection[minIndex];
      sConnection[minIndex] = sTemp;
      hConnection[minIndex] = hTemp;
    }
  }
  //
  // Table with connection infos
  //
  IP_WEBS_SendString(pOutput, "<h2>List of TCP connections</h2>");
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Socket<br>(Type)</th>");
  IP_WEBS_SendString(pOutput, "<th>Local<br>Peer</td>");
  IP_WEBS_SendString(pOutput, "<th>LPort<br>FPort</td>");
  IP_WEBS_SendString(pOutput, "<th>State</td>");
  IP_WEBS_SendString(pOutput, "<th>MTU<br>MSS</td>");
  IP_WEBS_SendString(pOutput, "<th>Retr.<br>delay</td>");
  IP_WEBS_SendString(pOutput, "<th>Idle time</td>");
  IP_WEBS_SendString(pOutput, "<th>Local<br>win</td>");
  IP_WEBS_SendString(pOutput, "<th>Peer win</td>");
  for (i = 0; i < NumConnections; i++) {
    r = IP_IPV6_INFO_GetConnectionInfo(hConnection[i], &Info);
    if (r == 0) {
      IP_WEBS_SendString(pOutput, "<tr><td>");
      IP_WEBS_SendUnsigned(pOutput, (unsigned)Info.hSocket, 10, 0);
      if (Info.AddrFam == AF_INET) {
        IP_WEBS_SendString(pOutput, "<br>(IPv4)");
      } else {
        IP_WEBS_SendString(pOutput, "<br>(IPv6)");
      }
      IP_WEBS_SendString(pOutput, "</td><td class=\"alignLeft\">");
      //
      // Output local address, such as    "192.168.1.1"
      //
      if (Info.LocalAddr.Union.aU32[0] == 0) {
        IP_WEBS_SendString(pOutput, "Any");
      } else {
        if (Info.AddrFam == AF_INET) {
          IP_PrintIPAddr(acBuffer, Info.LocalAddr.Union.aU32[0], sizeof(acBuffer));
          IP_WEBS_SendString(pOutput, acBuffer);
        } else {
          SEGGER_snprintf(acBuffer, sizeof(acBuffer), "%n", &Info.LocalAddr.Union.aU8[0]);
          IP_WEBS_SendString(pOutput, acBuffer);
        }
      }
      IP_WEBS_SendString(pOutput, "<br>");
      //
      // Output Peer Addr & Port, such as    "192.168.1.2"
      //
      if (Info.ForeignAddr.Union.aU32[0] == 0) {
        IP_WEBS_SendString(pOutput, "- No peer connected. -");
      } else {
        if (Info.AddrFam == AF_INET) {
          IP_PrintIPAddr(acBuffer, Info.ForeignAddr.Union.aU32[0], sizeof(acBuffer));
          IP_WEBS_SendString(pOutput, acBuffer);
        } else {
          SEGGER_snprintf(acBuffer, sizeof(acBuffer), "%n", &Info.ForeignAddr.Union.aU8[0]);
          IP_WEBS_SendString(pOutput, acBuffer);
        }
      }
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output local port / foreign port
      //
      IP_WEBS_SendUnsigned(pOutput, Info.LocalPort, 10, 0);
      IP_WEBS_SendString(pOutput, "<br>");
      if (Info.ForeignPort == 0) {
        IP_WEBS_SendString(pOutput, "-");
      } else {
        IP_WEBS_SendUnsigned(pOutput, Info.ForeignPort, 10, 0);
      }
      IP_WEBS_SendString(pOutput, "</td><td class=\"alignLeft\">");
      //
      // Output State, such as   "LISTEN"
      //
      IP_WEBS_SendString(pOutput, IP_INFO_ConnectionState2String(Info.TcpState));
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output MTU/MSS
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpMtu, 10, 0);
      IP_WEBS_SendString(pOutput, "</br>");
      IP_WEBS_SendUnsigned(pOutput, Info.TcpMss, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output retrans. delay
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpRetransDelay, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output TCP idle time
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpIdleTime, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output local window size
      //
      IP_WEBS_SendUnsigned(pOutput, Info.RxWindowMax, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output peer window size
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TxWindow, 10, 0);
      IP_WEBS_SendString(pOutput, "</td></tr>");
    }
  }
  OS_LeaveRegion();
  IP_WEBS_SendString(pOutput, "</table>");
#endif
}

#else
/*********************************************************************
*
*       _GetNetworkInfo
*
*  Function description
*    Sends the network configuration of the target infromation
*    and the connection information to the browser.
*/
static void _GetNetworkInfo(WEBS_OUTPUT* pOutput) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<H2>This sample runs on a Windows host.<br>No network statistics available.</H2>");
#else
  IP_CONNECTION_HANDLE hConnection[MAX_CONNECTION_INFO];
  IP_CONNECTION_HANDLE hTemp;
  U16                  sConnection[MAX_CONNECTION_INFO];
  U16                  sTemp;
  IP_CONNECTION Info;
  int  NumConnections;
  int  NumValidConnections;
  int  i;
  int  j;
  int  minIndex;
  int  r;
  char ac[16];
  U32  IPAddr;

  IP_WEBS_SendString(pOutput, "<h2>Configuration</h2>");
  IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  IP_WEBS_SendString(pOutput, "Server IP<br>");
  IP_WEBS_SendString(pOutput, "Gateway IP<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"colon\">");
  IP_WEBS_SendString(pOutput, ":<br>");
  IP_WEBS_SendString(pOutput, ":<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"value\">");
  IPAddr = IP_GetIPAddr(0);
  IP_PrintIPAddr(ac, IPAddr, sizeof(ac));
  IP_WEBS_SendString(pOutput, ac);
  IPAddr = IP_GetGWAddr(0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_PrintIPAddr(ac, IPAddr, sizeof(ac));
  IP_WEBS_SendString(pOutput, ac);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  OS_EnterRegion();
#if 0
  //
  // Buffer configuration of the stack
  //
  IP_WEBS_SendString(pOutput, "<h2>Stack buffer Info</h2>");
  IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  IP_WEBS_SendString(pOutput, "Small buffers<br>");
  IP_WEBS_SendString(pOutput, "Small buffer size<br>");
  IP_WEBS_SendString(pOutput, "Big buffer<br>");
  IP_WEBS_SendString(pOutput, "Big buffer size<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"colon\">");
  IP_WEBS_SendString(pOutput, ":<br>:<br>:<br>:<br>");
  IP_WEBS_SendString(pOutput, "<div class=\"value\">");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_AVAIL), 10, 0);
  IP_WEBS_SendString(pOutput, "/");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_CONFIG), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_SMALL_BUFFERS_SIZE), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_AVAIL), 10, 0);
  IP_WEBS_SendString(pOutput, "/");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_CONFIG), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, IP_INFO_GetBufferInfo(IP_INFO_BIG_BUFFERS_SIZE), 10, 0);
  IP_WEBS_SendString(pOutput, "</div>");
#endif
  IP_WEBS_SendString(pOutput, "<h2>Connection info</h2>");
  //
  // Number of connections
  //
  IP_WEBS_SendString(pOutput, "Total connections: ");
  NumConnections = IP_INFO_GetConnectionList(&hConnection[0], MAX_CONNECTION_INFO);
  NumValidConnections = NumConnections;
  for (i = 0; i < NumConnections; i++) {
    r = IP_INFO_GetConnectionInfo(hConnection[i], &Info);
    sConnection[i] = Info.hSock;
    if (r != 0) {
      NumValidConnections--;
    }
  }
  IP_WEBS_SendUnsigned(pOutput, NumValidConnections, 10, 0);
  //
  // Sort by hSock
  //
  for (i = 0; i < NumConnections; i++) {
    minIndex = i;
    for(j = i + 1; j < NumConnections; j++) {
      if(sConnection[j] < sConnection[minIndex]) {
        minIndex = j;
      }
    }
    if(i != minIndex) {
      sTemp = sConnection[i];
      hTemp = hConnection[i];
      sConnection[i] = sConnection[minIndex];
      hConnection[i] = hConnection[minIndex];
      sConnection[minIndex] = sTemp;
      hConnection[minIndex] = hTemp;
    }
  }
  //
  // Table with connection infos
  //
  IP_WEBS_SendString(pOutput, "<h2>List of TCP connections</h2>");
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Socket</th>");
  IP_WEBS_SendString(pOutput, "<th>Local</td>");
  IP_WEBS_SendString(pOutput, "<th>Peer</td>");
  IP_WEBS_SendString(pOutput, "<th>State</td>");
  IP_WEBS_SendString(pOutput, "<th>MTU/MSS</td>");
  IP_WEBS_SendString(pOutput, "<th>Retrans. delay</td>");
  IP_WEBS_SendString(pOutput, "<th>Idle time</td>");
  IP_WEBS_SendString(pOutput, "<th>Local window</td>");
  IP_WEBS_SendString(pOutput, "<th>Peer window</td>");
  for (i = 0; i < NumConnections; i++) {
    r = IP_INFO_GetConnectionInfo(hConnection[i], &Info);
    if (r == 0) {
      IP_WEBS_SendString(pOutput, "<tr><td>");
      IP_WEBS_SendUnsigned(pOutput, (unsigned)Info.hSock, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output Local Addr & Port, such as    "192.168.1.1:81"
      //
      if (Info.LocalAddr == 0) {
        IP_WEBS_SendString(pOutput, "Any");
      } else {
        IP_PrintIPAddr(ac, Info.LocalAddr, sizeof(ac));
        IP_WEBS_SendString(pOutput, ac);
      }
      IP_WEBS_SendString(pOutput, ":");
      IP_WEBS_SendUnsigned(pOutput, Info.LocalPort, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output Peer Addr & Port, such as    "192.168.1.1:81"
      //
      if (Info.ForeignAddr == 0) {
        IP_WEBS_SendString(pOutput, "---");
      } else {
        IP_PrintIPAddr(ac, Info.ForeignAddr, sizeof(ac));
        IP_WEBS_SendString(pOutput, ac);
        IP_WEBS_SendString(pOutput, ":");
        IP_WEBS_SendUnsigned(pOutput, Info.ForeignPort, 10, 0);
      }
      IP_WEBS_SendString(pOutput, "</td><td class=\"alignLeft\">");
      //
      // Output State, such as   "LISTEN"
      //
      IP_WEBS_SendString(pOutput, IP_INFO_ConnectionState2String(Info.TcpState));
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output MTU/MSS
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpMtu, 10, 0);
      IP_WEBS_SendString(pOutput, "/");
      IP_WEBS_SendUnsigned(pOutput, Info.TcpMss, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output retrans. delay
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpRetransDelay, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output TCP idle time
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TcpIdleTime, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output local window size
      //
      IP_WEBS_SendUnsigned(pOutput, Info.RxWindowCur, 10, 0);
      IP_WEBS_SendString(pOutput, "/");
      IP_WEBS_SendUnsigned(pOutput, Info.RxWindowMax, 10, 0);
      IP_WEBS_SendString(pOutput, "</td><td>");
      //
      // Output peer window size
      //
      IP_WEBS_SendUnsigned(pOutput, Info.TxWindow, 10, 0);
      IP_WEBS_SendString(pOutput, "</td></tr>");
    }
  }
  OS_LeaveRegion();
  IP_WEBS_SendString(pOutput, "</table>");
#endif
}

#endif

/*********************************************************************
*
*       _callback_ExecGetConnectionInfo
*/
static void _callback_ExecGetConnectionInfo(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  _GetNetworkInfo(pOutput);
}

/*********************************************************************
*
*       _GetOSInfo
*
*  Function description
*    This function generates and sends the HTML page with embOS
*    system information. It is called by _callback_ExecGetOSInfo()
*    and _callback_CGI_SSEembOS().
*/
static void _GetOSInfo(WEBS_OUTPUT* pOutput) {
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<H2>This sample runs on a Windows host.<br>No embOS statistics available.</H2>");
#else
  OS_TASK* pTask;
  //
  // Get embOS information and build webpage
  //
  IP_WEBS_SendString(pOutput, "<h2>System info</h2>");
  IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  IP_WEBS_SendString(pOutput, "Number of tasks<br>");
  IP_WEBS_SendString(pOutput, "System time<br>");
  IP_WEBS_SendString(pOutput, "System stack (size@base)<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  IP_WEBS_SendString(pOutput, "<div class=\"colon\">");
  IP_WEBS_SendString(pOutput, ":<br>:<br>:<br>");
  IP_WEBS_SendString(pOutput, "</div>");
  OS_EnterRegion();
  IP_WEBS_SendString(pOutput, "<div class=\"value\">");
  IP_WEBS_SendUnsigned(pOutput, OS_GetNumTasks(), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, OS_GetTime32(), 10, 0);
  IP_WEBS_SendString(pOutput, "<br>");
  IP_WEBS_SendUnsigned(pOutput, OS_INFO_GET_SYS_STACK_SIZE(), 10, 0);
  IP_WEBS_SendString(pOutput, "@0x");
  IP_WEBS_SendUnsigned(pOutput, (OS_U32)OS_INFO_GET_SYS_STACK_BASE(), 16, 0);
  IP_WEBS_SendString(pOutput, "</div>");
  //
  // Table with task infos
  //
  IP_WEBS_SendString(pOutput, "<h2>List of tasks</h2>");
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Id</td>");
  IP_WEBS_SendString(pOutput, "<th>Priority</td>");
  IP_WEBS_SendString(pOutput, "<th>Task names</td>");
  IP_WEBS_SendString(pOutput, "<th>Context switches</td>");
  IP_WEBS_SendString(pOutput, "<th>Task Stack</td>");
  IP_WEBS_SendString(pOutput, "</tr>");
  for (pTask = OS_pTask; pTask; pTask = pTask->pNext) {
    IP_WEBS_SendString(pOutput, "<tr>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendString(pOutput, "0x");
    IP_WEBS_SendUnsigned(pOutput, (U32)pTask, 16, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Priorities
    //
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, OS_GetPriority(pTask), 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Task names
    //
    IP_WEBS_SendString(pOutput, "<td class=\"alignLeft\">");
    IP_WEBS_SendString(pOutput, OS_INFO_GET_TASK_NAME(pTask));
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // NumActivations
    //
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, OS_INFO_GET_TASK_NUM_ACTIVATIONS(pTask), 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Task stack info
    //
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, OS_INFO_GET_TASK_STACK_USED(pTask), 10, 0);
    IP_WEBS_SendString(pOutput, "/");
    IP_WEBS_SendUnsigned(pOutput, OS_INFO_GET_TASK_STACK_SIZE(pTask), 10, 0);
    IP_WEBS_SendString(pOutput, "@0x");
    IP_WEBS_SendUnsigned(pOutput, (U32)OS_INFO_GET_TASK_STACK_BASE(pTask), 16, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "</tr>");
  }
  OS_LeaveRegion();
  IP_WEBS_SendString(pOutput, "</table>");
#endif
}

/*********************************************************************
*
*       _callback_ExecGetOSInfo
*/
static void _callback_ExecGetOSInfo(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  _GetOSInfo(pOutput);
}

#if 0
/*********************************************************************
*
*       _callback_ExecGetIFaceInfo
*/
static void _callback_ExecGetIFaceInfo(WEBS_OUTPUT* pOutput, const char* sParameters, const char* sValue) {
  IP_INFO_INTERFACE InterfaceInfo;
  int NumIFaces;
  int i;

  WEBS_USE_PARA(sParameters);
  WEBS_USE_PARA(sValue);

  OS_EnterRegion();
  //
  // Send interface information
  //
  IP_WEBS_SendString(pOutput, "<H2>Interface information:</H2>");
  IP_WEBS_SendString(pOutput, "<PRE>Number of available interfaces: ");
  NumIFaces = IP_INFO_GetNumInterfaces();
  IP_WEBS_SendUnsigned(pOutput, NumIFaces, 10, 0);
  IP_WEBS_SendString(pOutput, "</PRE>");
  //
  // Table with interface information
  //
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tb>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Interface type</td>");
  IP_WEBS_SendString(pOutput, "<th>Interface name</td>");
  IP_WEBS_SendString(pOutput, "<th>Admin state</td>");
  IP_WEBS_SendString(pOutput, "<th>Hardware state</td>");
  IP_WEBS_SendString(pOutput, "<th>Speed</td>");
  IP_WEBS_SendString(pOutput, "<th>Change admin state</td>");
  IP_WEBS_SendString(pOutput, "</tr>");
  for (i = 0; i < NumIFaces; i++) {
    IP_INFO_GetInterfaceInfo (i, &InterfaceInfo);
    IP_WEBS_SendString(pOutput, "<tr>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendString(pOutput, InterfaceInfo.sTypeName);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendString(pOutput, InterfaceInfo.sTypeName);
    IP_WEBS_SendUnsigned(pOutput, InterfaceInfo.TypeIndex, 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    if (InterfaceInfo.AdminState) {
      IP_WEBS_SendString(pOutput, "Up");
    } else {
      IP_WEBS_SendString(pOutput, "Down");
    }
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    if (InterfaceInfo.HWState) {
      IP_WEBS_SendString(pOutput, "Up");
    } else {
      IP_WEBS_SendString(pOutput, "Down");
    }
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, InterfaceInfo.Speed, 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendString(pOutput, "<FORM ACTION=\"Redirect.cgi\" METHOD=\"GET\">");
    IP_WEBS_SendString(pOutput, "<INPUT TYPE=\"submit\" VALUE=\"");
    if (InterfaceInfo.AdminState) {
      IP_WEBS_SendString(pOutput, "Disconnect");
    } else {
      IP_WEBS_SendString(pOutput, "Connect");
    }
    IP_WEBS_SendString(pOutput, "\" NAME=\"");

    if (InterfaceInfo.AdminState) {
      IP_WEBS_SendString(pOutput, "D");
    } else {
      IP_WEBS_SendString(pOutput, "C");
    }
    IP_WEBS_SendUnsigned(pOutput, i, 10, 0);
    IP_WEBS_SendString(pOutput, "\">");
    IP_WEBS_SendString(pOutput, "</FORM>");
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "</tr>");
  }
  OS_LeaveRegion();
  IP_WEBS_SendString(pOutput, "</tb>");
  IP_WEBS_SendString(pOutput, "</table>");
}
#endif

/*********************************************************************
*
*       _callback_ExecGetIPAddr
*/
static void _callback_ExecGetIPAddr(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
#ifdef _WIN32
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  IP_WEBS_SendString(pOutput, "This sample runs on localhost.</H2>");
#else
  char ac[16];
  U32  IPAddr;

  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  IPAddr = IP_GetIPAddr(0);
  IP_PrintIPAddr(ac, IPAddr, sizeof(ac));
  IP_WEBS_SendString(pOutput, ac);
#endif
}

/*********************************************************************
*
*       _callback_ExecPercentage
*/
static void _callback_ExecPercentage(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    IP_WEBS_SendUnsigned(pOutput, _Percentage, 10, 0);
  } else {
    int v;
    v = SEGGER_atoi(sValue);
    if (v > 100) {
      v = 100;
    }
    if (v < 0) {
      v = 0;
    }
    _Percentage = v;
  }
}

/*********************************************************************
*
*       _callback_ExecLastName
*/
static void _callback_ExecLastName(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    IP_WEBS_SendString(pOutput, _acLastName);
  } else {
    _CopyString(_acLastName, sValue, sizeof(_acLastName));
  }
}

/*********************************************************************
*
*       _callback_ExecFirstName
*/
static void _callback_ExecFirstName(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    IP_WEBS_SendString(pOutput, _acFirstName);
  } else {
    _CopyString(_acFirstName, sValue, sizeof(_acFirstName));
  }
}

/*********************************************************************
*
*       _callback_LEDx
*
*  Function description
*    This function is called to set or get the LED state.
*/
static void _callback_LEDx(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue, unsigned LEDIndex) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  if (sValue == NULL) {
    if (_aLEDState[LEDIndex] == 1) {             // Get LED state
      IP_WEBS_SendString(pOutput, "checked");
    } else {
      IP_WEBS_SendString(pOutput, "");
#ifndef _WIN32
      BSP_ClrLED(LEDIndex);
#endif
    }
  } else {
    //
    // Set new LED state
    //
    if (strcmp(sValue, "on") == 0) {
      _aLEDStateNew[LEDIndex] = 1;
#ifndef _WIN32
      BSP_SetLED(LEDIndex);
#endif
    }
  }
}

/*********************************************************************
*
*       _callback_LED0
*       _callback_LED1
*       _callback_LED2
*       _callback_LED3
*       _callback_LED4
*       _callback_LED5
*       _callback_LED6
*       _callback_LED7
*/
static void _callback_LED0(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 0);
}
static void _callback_LED1(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 1);
}
static void _callback_LED2(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 2);
}
static void _callback_LED3(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 3);
}
static void _callback_LED4(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 4);
}
static void _callback_LED5(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 5);
}
static void _callback_LED6(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 6);
}
static void _callback_LED7(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_LEDx(pOutput, sPara, sValue, 7);
}

static void _callback_SetLEDs(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  unsigned i;

  WEBS_USE_PARA(pOutput);
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  for (i = 0; i < sizeof(_aLEDState); i++) {
    _aLEDState[i] = _aLEDStateNew[i];
    _aLEDStateNew[i] = 0;
  }
}

/*********************************************************************
*
*       _callback_ExecGetIndex
*
*  Function description:
*    Sends the content of the index page.
*    We use a dynamic web page to generate the content to enhance
*    the portability of the sample.
*/
static void _callback_ExecGetIndex(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  IP_WEBS_SendString(pOutput, "<div class=\"nav\">");
  IP_WEBS_SendString(pOutput, "<h3>Simple dynamic web pages</h3><ul>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"FormGET.htm\">Form samples (\"GET\")</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"FormPOST.htm\">Form samples (\"POST\")</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"VirtFile.htm\">Virtual file sample</a><br>");
#if WEBS_USE_AUTH_DIGEST
  IP_WEBS_SendString(pOutput, "<li><a href=\"/conf/Authen.htm\">Digest authentication sample</a>");
#else
  IP_WEBS_SendString(pOutput, "<li><a href=\"/conf/Authen.htm\">Basic authentication sample</a>");
#endif
  IP_WEBS_SendString(pOutput, " <span class=\"hint\">(User: user | Pass: pass)</span>");
#if WEBS_SUPPORT_UPLOAD
  IP_WEBS_SendString(pOutput, "<li><a href=\"Upl.htm\">Upload a file</a> <span class=\"hint\">(Real file system required)</span>");
#endif
#if INCLUDE_IP_CONFIG_SAMPLE
  IP_WEBS_SendString(pOutput, "<li><a href=\"IPConf.htm\">IP Configuration</a><br>");
#endif
  IP_WEBS_SendString(pOutput, "</ul>");
  IP_WEBS_SendString(pOutput, "<h3>System info samples using reload</h3><ul>");
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<li>Network statistics (embedded target only)<br>");
  IP_WEBS_SendString(pOutput, "<li>embOS statistics (embedded target only)<br></ul>");
#else
  IP_WEBS_SendString(pOutput, "<li><a href=\"IPInf.htm\">Network statistics</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"OSInf.htm\">embOS statistics</a><br></ul>");
#endif
  IP_WEBS_SendString(pOutput, "<h3>System info samples using Server-Sent Events (SSE)</h3>");
  IP_WEBS_SendString(pOutput, "<ul><li><a href=\"SSE_Time.htm\">System time</a><br>");
#ifdef _WIN32
  IP_WEBS_SendString(pOutput, "<li>Network statistics (embedded target only)<br>");
  IP_WEBS_SendString(pOutput, "<li>embOS statistics (embedded target only)<br></ul>");
#else
  IP_WEBS_SendString(pOutput, "<li><a href=\"SSE_IP.htm\">Network statistics</a><br>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"SSE_OS.htm \">embOS statistics</a><br></ul>");
#endif
#if INCLUDE_PRESENTATION_SAMPLE || WEBS_SUPPORT_UPLOAD
  IP_WEBS_SendString(pOutput, "<h3>Generic AJAX samples</h3><ul>");
#if INCLUDE_PRESENTATION_SAMPLE
  IP_WEBS_SendString(pOutput, "<li><a href=\"Products.htm\">SEGGER middleware presentation</a><br>");
#endif
#if WEBS_SUPPORT_UPLOAD
  IP_WEBS_SendString(pOutput, "<li><a href=\"Upl_AJAX.htm\">Upload a file</a> <span class=\"hint\">(Real file system required)</span>");
#endif
  IP_WEBS_SendString(pOutput, "</ul>");
#endif
#if INCLUDE_SHARE_SAMPLE
  IP_WEBS_SendString(pOutput, "<h3>Generic combined SSE/AJAX samples</h3><ul>");
  IP_WEBS_SendString(pOutput, "<li><a href=\"Shares.htm\">Stock quotes sample </a><br></ul>");
#endif
  IP_WEBS_SendString(pOutput, "</div>");
}

/*********************************************************************
*
*       _callback_CGI_Send
*/
static void _callback_CGI_Send(WEBS_OUTPUT* pOutput, const char* sParameters) {
  int  r;
  char ac[64];

  //
  // Header of the page
  //
  WebsSample_SendPageHeader(pOutput, "Virtual file sample");
  //
  // Content
  //
  r  = IP_WEBS_GetParaValuePtr(sParameters, 0, NULL, 0, NULL, 0);
  r |= IP_WEBS_GetParaValuePtr(sParameters, 1, NULL, 0, NULL, 0);
  if (r == 0) {
    (void)IP_WEBS_GetParaValue(sParameters, 0, NULL, 0, &ac[0], sizeof(ac));
    IP_WEBS_SendString(pOutput, "<h3>First name: ");
    IP_WEBS_SendString(pOutput, &ac[0]);
    IP_WEBS_SendString(pOutput, "</h3>");
    (void)IP_WEBS_GetParaValue(sParameters, 1, NULL, 0, &ac[0], sizeof(ac));
    IP_WEBS_SendString(pOutput, "<h3>Last name: ");
    IP_WEBS_SendString(pOutput, &ac[0]);
    IP_WEBS_SendString(pOutput, "</h3>");
  } else {
    IP_WEBS_SendString(pOutput, "<br>Error!");
  }
  //
  // Footer of the page
  //
  WebsSample_SendPageFooter(pOutput);
}

/*********************************************************************
*
*       _callback_CGI_UploadFile
*/
static void _callback_CGI_UploadFile(WEBS_OUTPUT* pOutput, const char* sParameters) {
  const char* pFileName;
  const char* pState;        // Can be 0: Upload failed; 1: Upload succeeded; Therefore we do not need to know the length, it will always be 1.
        int   StateParaNum;  // Always the last para/value pair added.
        int   FileNameLen;
        int   r;

  //
  // Header of the page
  //
  WebsSample_SendPageHeader(pOutput, "Virtual file sample");
  //
  // Content
  //
  r  = IP_WEBS_GetParaValuePtr(sParameters, 0, NULL, 0, &pFileName, &FileNameLen);
  StateParaNum = IP_WEBS_GetNumParas(sParameters);
  r |= IP_WEBS_GetParaValuePtr(sParameters, (StateParaNum - 1), NULL, 0, &pState, NULL);
  if (r == 0) {
    IP_WEBS_SendString(pOutput, "Upload of \"");
    IP_WEBS_SendMem(pOutput, pFileName, FileNameLen);
    if (*pState == '1') {
      IP_WEBS_SendString(pOutput, "\" successful!<br>");
      IP_WEBS_SendString(pOutput, "<a href=\"");
      IP_WEBS_SendMem(pOutput, pFileName, FileNameLen);
      IP_WEBS_SendString(pOutput, "\">Go to ");
      IP_WEBS_SendMem(pOutput, pFileName, FileNameLen);
      IP_WEBS_SendString(pOutput, "</a><br>");
    } else {
      IP_WEBS_SendString(pOutput, "\" not successful!<br>");
    }
  } else {
    IP_WEBS_SendString(pOutput, "Upload not successful!");
  }
  //
  // Footer of the page
  //
  WebsSample_SendPageFooter(pOutput);
}

/*********************************************************************
*
*       _SendTime
*
*
*  Function description:
*    Sends the system time to the Web browser.
*
*  Return value:
*    0: Data successfully sent.
*   -1: Data not sent.
*    1: Data successfully sent. Connection should be closed.
*/
static int _SendTime(WEBS_OUTPUT* pOutput) {
  int r;
  U32 Time;

#ifdef _WIN32
  Time = GetTickCount();
#else
  Time = OS_GetTime32();
#endif
  //
  // Send implementation specific header to client
  //
  IP_WEBS_SendString(pOutput, "data: ");
  IP_WEBS_SendString(pOutput, "System time: ");
  IP_WEBS_SendUnsigned(pOutput, Time, 10, 0);
  IP_WEBS_SendString(pOutput, "\r\n");
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
  r = IP_WEBS_Flush(pOutput);
  return r;
}

/*********************************************************************
*
*       _callback_CGI_SSETime
*
*  Function description:
*    Sends the system time to the Web browser every 500 ms.
*/
static void _callback_CGI_SSETime(WEBS_OUTPUT* pOutput, const char* sParameters) {
  int r;

  WEBS_USE_PARA(sParameters);
  //
  // Construct the SSE Header
  //
#if (WEBS_USE_ENHANCED_FORM_HANDLING == 0)
  IP_WEBS_SendHeaderEx(pOutput, NULL, "text/event-stream", 1);
#else
  {
    WEBS_HEADER_CONFIG HeaderConfig;

    memset(&HeaderConfig, 0, sizeof(HeaderConfig));
    HeaderConfig.sMIMEType  = "text/event-stream";
    HeaderConfig.ReqKeepCon = 1;
    IP_WEBS_SendHeader(pOutput, &HeaderConfig);
  }
#endif
  IP_WEBS_SendString(pOutput, "retry: 1000\n");  // Normally, the browser attempts to reconnect to the server ~3 seconds after each connection is closed. We change that timeout 1 sec.
  for (;;) {
    r = _SendTime(pOutput);
    if (r == 0) {     // Data transmitted, Web browser is still waiting for new data.
#ifdef _WIN32
      Sleep(500);
#else
      OS_Delay(500);
#endif
    } else {          // Even if the data transmission was successful, it could be necessary to close the connection after transmission.
      break;          // This is normally the case if the Web server otherwise could not process new connections.
    }
  }
}

/*********************************************************************
*
*       _SendOSInfo
*/
static int _SendOSInfo(WEBS_OUTPUT* pOutput) {
  int r;

  IP_WEBS_SendString(pOutput, "data: ");        // Start of the SSE data
  _GetOSInfo(pOutput);
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
  r = IP_WEBS_Flush(pOutput);
  return r;
}

/*********************************************************************
*
*       _callback_CGI_SSEembOS
*
*  Function description:
*    Sends some OS informartion to the Web browser every 500 ms.
*/
static void _callback_CGI_SSEembOS(WEBS_OUTPUT* pOutput, const char* sParameters) {
  int r;

  WEBS_USE_PARA(sParameters);
  //
  // Construct the SSE Header
  //
#if (WEBS_USE_ENHANCED_FORM_HANDLING == 0)
  IP_WEBS_SendHeaderEx(pOutput, NULL, "text/event-stream", 1);
#else
  {
    WEBS_HEADER_CONFIG HeaderConfig;

    memset(&HeaderConfig, 0, sizeof(HeaderConfig));
    HeaderConfig.sMIMEType  = "text/event-stream";
    HeaderConfig.ReqKeepCon = 1;
    IP_WEBS_SendHeader(pOutput, &HeaderConfig);
  }
#endif
  IP_WEBS_SendString(pOutput, "retry: 1000\n");  // Normally, the browser attempts to reconnect to the server ~3 seconds after each connection is closed. We change that timeout 1 sec.
  for (;;) {
    r = _SendOSInfo(pOutput);
    if (r == 0) {     // Data transmitted, Web browser is still waiting for new data.
#ifdef _WIN32
      Sleep(500);
#else
      OS_Delay(500);
#endif
    } else {          // Even if the data transmission was successful, it could be necessary to close the connection after transmission.
      break;          // This is normally the case, if the Web server otherwise could not process new connections.
    }
  }
}

/*********************************************************************
*
*       _SendNetInfo
*/
static int _SendNetInfo(WEBS_OUTPUT* pOutput) {
  int r;

  IP_WEBS_SendString(pOutput, "data: ");        // Start of the SSE data
  _GetNetworkInfo(pOutput);
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
  r = IP_WEBS_Flush(pOutput);
  return r;
}

/*********************************************************************
*
*       _callback_CGI_SSENet
*
*  Function description:
*    Sends some network informartion to the Web browser every 500 ms.
*/
static void _callback_CGI_SSENet(WEBS_OUTPUT* pOutput, const char* sParameters) {
  int r;

  WEBS_USE_PARA(sParameters);
  //
  // Construct the SSE Header
  //
#if (WEBS_USE_ENHANCED_FORM_HANDLING == 0)
  IP_WEBS_SendHeaderEx(pOutput, NULL, "text/event-stream", 1);
#else
  {
    WEBS_HEADER_CONFIG HeaderConfig;

    memset(&HeaderConfig, 0, sizeof(HeaderConfig));
    HeaderConfig.sMIMEType  = "text/event-stream";
    HeaderConfig.ReqKeepCon = 1;
    IP_WEBS_SendHeader(pOutput, &HeaderConfig);
  }
#endif
  IP_WEBS_SendString(pOutput, "retry: 1000\n");  // Normally, the browser attempts to reconnect to the server ~3 seconds after each connection is closed. We change that timeout 1 sec.
  for (;;) {
    r = _SendNetInfo(pOutput);
    if (r == 0) {     // Data transmitted, Web browser is still waiting for new data.
#ifdef _WIN32
      Sleep(500);
#else
      OS_Delay(500);
#endif
    } else {          // Even if the data transmission was successful, it could be necessary to close the connection after transmission.
      break;          // This is normally the case if the Web server otherwise could not process new connections.
    }
  }
}

#if INCLUDE_PRESENTATION_SAMPLE
/*********************************************************************
*
*       _callback_CGI_GetDetails
*
*  Function description:
*    Sends the selected product information to the browser.
*/
static void _callback_CGI_GetDetails(WEBS_OUTPUT* pOutput, const char* sParameters) {
  char acPara[10];

  IP_WEBS_GetParaValue(sParameters, 0, NULL, 0, acPara, sizeof(acPara));
  if (strcmp(acPara, "OS") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>embOS - Real Time Operating System</h2>");
    IP_WEBS_SendString(pOutput, "<b>embOS</b> is a priority-controlled real time operating system, designed to be used as foundation for the development of embedded real-time applications. It is a zero interrupt latency, high-performance RTOS that has been optimized for minimum memory consumption in both RAM and ROM, as well as high speed and versatility.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>Preemptive scheduling: Guarantees that of all tasks in READY state, the one with the highest priority executes, except for situations where priority inversion applies.</li>");
    IP_WEBS_SendString(pOutput, "<li>Round-robin scheduling for tasks with identical priorities.</li>");
    IP_WEBS_SendString(pOutput, "<li>Preemptions can be disabled for entire tasks or for sections of a program.</li>");
    IP_WEBS_SendString(pOutput, "<li>Thread local storage support.</li>");
    IP_WEBS_SendString(pOutput, "<li>Thread safe system library support.</li>");
    IP_WEBS_SendString(pOutput, "<li>No configuration needed</li>");
    IP_WEBS_SendString(pOutput, "<li>Up to 255 priorities: Every task can have an individual priority => the response of tasks can be precisely defined according to the requirements of the application.</li>");
    IP_WEBS_SendString(pOutput, "<li>Unlimited number of tasks (limited only by available memory).</li>");
    IP_WEBS_SendString(pOutput, "<li>Unlimited number of semaphores (limited only by available memory).</li>");
    IP_WEBS_SendString(pOutput, "<li>Unlimited number of mailboxes (limited only by available memory).</li>");
    IP_WEBS_SendString(pOutput, "<li>Size and number of messages can be freely defined.</li>");
    IP_WEBS_SendString(pOutput, "<li>Unlimited number of software timers (limited only by available memory).</li>");
    IP_WEBS_SendString(pOutput, "<li>Time resolution tick can be freely selected (default is 1ms).</li>");
    IP_WEBS_SendString(pOutput, "<li>High resolution time measurement (more accurate than tick).</li>");
    IP_WEBS_SendString(pOutput, "<li>Power management: Unused CPU time can automatically be spent in halt mode, minimizing power consumption.</li>");
    IP_WEBS_SendString(pOutput, "<li>Full interrupt support: Most API functions can be used from within the Interrupt Service Routines (ISRs).</li>");
    IP_WEBS_SendString(pOutput, "<li>Zero interrupt latency time.</li>");
    IP_WEBS_SendString(pOutput, "<li>Nested interrupts are permitted.</li>");
    IP_WEBS_SendString(pOutput, "<li>Start application and projects (BSPs) for an easy start.</li>");
    IP_WEBS_SendString(pOutput, "<li>Debug build performs runtime checks, simplifying development.</li>");
    IP_WEBS_SendString(pOutput, "<li>High precision per task profiling.</li>");
    IP_WEBS_SendString(pOutput, "<li>Real time kernel viewer (embOSView) included.</li>");
    IP_WEBS_SendString(pOutput, "<li>Very fast and efficient, yet small code.</li>");
    IP_WEBS_SendString(pOutput, "<li>Minimum RAM usage.</li>");
    IP_WEBS_SendString(pOutput, "<li>Core written in assembly language.</li>");
    IP_WEBS_SendString(pOutput, "<li>All API functions can be called from C /C++/assembly.</li>");
    IP_WEBS_SendString(pOutput, "<li>Initialization of microcontroller hardware as sources.</li>");
    IP_WEBS_SendString(pOutput, "<li>BSP for any unsupported hardware with the same CPU can easily be written by user.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/embos.html\" target=\"_blank\">http://segger.com/embos.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "IP") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emNet - TCP/IP Stack</h2>");
    IP_WEBS_SendString(pOutput, "<b>emNet</b> is a CPU independent TCP/IP stack. emNet is a high-performance library that has been optimized for speed, versatility and memory footprint. It is written in ANSI C and can be used on virtually any CPU.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>ANSI C socket.h-like API for user applications. An application using the standard C socket library can easily be ported to use emNet.</li>");
    IP_WEBS_SendString(pOutput, "<li>High performance</li>");
    IP_WEBS_SendString(pOutput, "<li>Small footprint</li>");
    IP_WEBS_SendString(pOutput, "<li>Runs \"out-of-the-box\"</li>");
    IP_WEBS_SendString(pOutput, "<li>No configuration required</li>");
    IP_WEBS_SendString(pOutput, "<li>Works with any RTOS in a multitasking environment (embOS recommended)</li>");
    IP_WEBS_SendString(pOutput, "<li>Zero data copy for ultra fast performance</li>");
    IP_WEBS_SendString(pOutput, "<li>Standard sockets Interface</li>");
    IP_WEBS_SendString(pOutput, "<li>Raw Socket Support</li>");
    IP_WEBS_SendString(pOutput, "<li>Non-blocking versions of all functions</li>");
    IP_WEBS_SendString(pOutput, "<li>Connections limited only by memory availability</li>");
    IP_WEBS_SendString(pOutput, "<li>Re-assembly of fragmented packets</li>");
    IP_WEBS_SendString(pOutput, "<li>Optional drivers for the most common devices are available</li>");
    IP_WEBS_SendString(pOutput, "<li>Fully runtime configurable</li>");
    IP_WEBS_SendString(pOutput, "<li>Developed from ground up for embedded systems</li>");
    IP_WEBS_SendString(pOutput, "<li>PPP/PPPOE available</li>");
    IP_WEBS_SendString(pOutput, "<li>Various upper layer protocols available</li>");
    IP_WEBS_SendString(pOutput, "<li>Drivers for most popular microcontrollers and external MACs available</li>");
    IP_WEBS_SendString(pOutput, "<li>Easy to use!</ul></li>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emnet.html\" target=\"_blank\">http://segger.com/emnet.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "FS") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emFile - File System</h2>");
    IP_WEBS_SendString(pOutput, "<b>emFile</b> is a file system for embedded applications, which can be used on any media, for which you can provide basic hardware access functions. emFile is a high performance library that has been optimized for minimum memory consumption in RAM and ROM, high speed and versatility. It is written in ANSI C and can be used on any CPU.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>MS DOS/MS Windows-compatible FAT12, FAT16 and FAT32 support.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional module that handles long file names of FAT media.</li>");
    IP_WEBS_SendString(pOutput, "<li>Multiple device driver support. You can use different device drivers with emFile, which allows you to access different types of hardware with the file system at the same time.</li>");
    IP_WEBS_SendString(pOutput, "<li>Multiple media support. A device driver allows you to access different media at the same time.</li>");
    IP_WEBS_SendString(pOutput, "<li>Cache support. Improves the performance of the file system by keeping the last recently used sectors in RAM.</li>");
    IP_WEBS_SendString(pOutput, "<li>Works with any operating system to accomplish a thread-safe environment.</li>");
    IP_WEBS_SendString(pOutput, "<li>ANSI C stdio.h-like API for user applications. An application using the standard C I/O library can easily be ported to use emFile.</li>");
    IP_WEBS_SendString(pOutput, "<li>Very simple device driver structure. emFile device drivers need only basic functions for reading and writing blocks. There is a template included.</li>");
    IP_WEBS_SendString(pOutput, "<li>Optional NOR flash (EEPROM) driver. Any CFI-compliant NOR flash is supported. Wear leveling included.</li>");
    IP_WEBS_SendString(pOutput, "<li>Optional device driver for NAND flash devices. Very high read/write speeds. ECC and wear leveling included.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional device driver for MultiMedia & SD cards using SPI mode or card mode that can be easily integrated.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional IDE driver, which is also suitable for CompactFlash using either True IDE or Memory Mapped mode.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional proprietary file system (EFS) with native long file name support.</li>");
    IP_WEBS_SendString(pOutput, "<li>An optional journaling add-on. It protects the integrity of file system against unexpected resets.</li>");
    IP_WEBS_SendString(pOutput, "<li>NAND flash evaluation board available.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emfile.html\" target=\"_blank\">http://segger.com/emfile.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "GUI") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emWin - Graphic Software and GUI</h2>");
    IP_WEBS_SendString(pOutput, "<b>emWin</b> is designed to provide an efficient, processor- and LCD controller-independent graphical user interface (GUI) for any application that operates with a graphical LCD. It is compatible with single-task and multitask environments, with a proprietary operating system or with any commercial RTOS. emWin is shipped as \"C\" source code.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b>");
    IP_WEBS_SendString(pOutput, "<ul>");
    IP_WEBS_SendString(pOutput, "<li>Any 8/16/32-bit CPU; only an ANSI \"C\" compiler is required.</li>");
    IP_WEBS_SendString(pOutput, "<li>Any (monochrome, grayscale or color) LCD with any controller supported (if the right driver is available).</li>");
    IP_WEBS_SendString(pOutput, "<li>May work without LCD controller on smaller displays.</li>");
    IP_WEBS_SendString(pOutput, "<li>PC tool emWinView for a detailed (magnified) view of all layers in the simulation.</li>");
    IP_WEBS_SendString(pOutput, "<li>Any interface supported using configuration macros.</li>");
    IP_WEBS_SendString(pOutput, "<li>Display-size configurable.</li>");
    IP_WEBS_SendString(pOutput, "<li>Characters and bitmaps may be written at any point on the LCD, not just on even-numbered byte addresses.</li>");
    IP_WEBS_SendString(pOutput, "<li>Routines are optimized for both size and speed.</li>");
    IP_WEBS_SendString(pOutput, "<li>Compile time switches allow for different optimizations.</li>");
    IP_WEBS_SendString(pOutput, "<li>For slower LCD controllers, LCD can be cached in memory, reducing access to a minimum and resulting in very high speed.</li>");
    IP_WEBS_SendString(pOutput, "<li>Clear structure.</li>");
    IP_WEBS_SendString(pOutput, "<li>Virtual display support; the virtual display can be larger than the actual display.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emwin.html\" target=\"_blank\">http://segger.com/emwin.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "BTL") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emLoad - Software Updater</h2>");
    IP_WEBS_SendString(pOutput, "<b>emLoad</b> is software for program updates for embedded applications via serial interface from a PC. The software consists of a Windows program and a program for the target application (bootloader) in source code form.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b>");
    IP_WEBS_SendString(pOutput, "<ul><li>Low memory footprint.</li>");
    IP_WEBS_SendString(pOutput, "<li>Straightforward configuration.</li>");
    IP_WEBS_SendString(pOutput, "<li>ANSI-C code is completely portable and runs on any target.</li>");
    IP_WEBS_SendString(pOutput, "<li>100% save & fast: CRC-check implemented.</li>");
    IP_WEBS_SendString(pOutput, "<li>Tools for Windows PC included.</li>");
    IP_WEBS_SendString(pOutput, "<li>Optional support for firmware passwords (emLoad V3).</li>");
    IP_WEBS_SendString(pOutput, "<li>Follows the SEGGER coding standards: Efficient and compact, yet easy to read, understand & debug.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emload.html\" target=\"_blank\">http://segger.com/emload.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "USBD") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emUSB Device - USB Device Stack</h2>");
    IP_WEBS_SendString(pOutput, "<b>emUSB</b> is a high speed USB device stack specifically designed for embedded systems. The software is written in ANSI \"C\" and can run on any platform. emUSB can be used with embOS or any other supported RTOS. A variety of target drivers are already available. Support for new platforms can usually be added for no extra charge.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>High speed</li>");
    IP_WEBS_SendString(pOutput, "<li>Optimized to be used with embOS but works with any other supported RTOS.</li>");
    IP_WEBS_SendString(pOutput, "<li>Easy to use</li>");
    IP_WEBS_SendString(pOutput, "<li>Easy to port</li>");
    IP_WEBS_SendString(pOutput, "<li>No custom USB host driver necessary</li>");
    IP_WEBS_SendString(pOutput, "<li>Start / test application supplied</li>");
    IP_WEBS_SendString(pOutput, "<li>Highly efficient, portable, and commented ANSI C source code</li>");
    IP_WEBS_SendString(pOutput, "<li>Hardware abstraction layer allows rapid addition of support for new devices.</li></ul>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emusb.html\" target=\"_blank\">http://segger.com/emusb.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else if (strcmp(acPara, "USBH") == 0) {
    IP_WEBS_SendString(pOutput, "<h2>emUSB Host - USB Host Stack</h2>");
    IP_WEBS_SendString(pOutput, "<b>emUSB</b> Host implements full USB host functionality, including external hub support, and optionally provides device class drivers. It enables developers to easily add USB host functionality to embedded systems.");
    IP_WEBS_SendString(pOutput, "<br><br><b>Features</b><ul>");
    IP_WEBS_SendString(pOutput, "<li>Optimized to be used with embOS but works with any other supported RTOS.</li>");
    IP_WEBS_SendString(pOutput, "<li>Highly efficient, portable, and commented ANSI \"C\" source code</li>");
    IP_WEBS_SendString(pOutput, "<li>Hardware abstraction layer allows rapid addition of support for new devices.</li>");
    IP_WEBS_SendString(pOutput, "<br><br><a href=\"http://segger.com/emusb-host.html\" target=\"_blank\">http://segger.com/emusb-host.html</a> <span class=\"hint\"><br>(external link, an internet connection is required)</span>");
  } else {
    IP_WEBS_SendString(pOutput, "<center><b>Please click on one of the pictures on the left to select a product.</b></center>");
  }
}
#endif  // INCLUDE_PRESENTATION_SAMPLE

#if INCLUDE_SHARE_SAMPLE
/*********************************************************************
*
*       _UpdateStockPrices
*
*  Function description:
*    Generates a new stock prices and puts it in the ring buffers.
*/
static void _UpdateStockPrices(void) {
  int WrOff;
  int Size;
  int Pre;
  int Limit;
  int v;
  int i;

#ifndef _WIN32
  srand(OS_GetTime32());
#else
  srand((unsigned int)time(NULL));
#endif
  for (i = 0; i < NUM_STOCKS; i++) {
    //
    // Update the read offset.
    //
    _StockInfo[i].RdOff++;
    if (_StockInfo[i].RdOff == _StockInfo[i].Size) {
      _StockInfo[i].RdOff = 0;
    }
    Size  = _StockInfo[i].Size;
    WrOff = _StockInfo[i].RdOff - 1;
    if (WrOff < 0) {
      WrOff = Size - 1;
    }
    Pre   = WrOff - 1;
    if (Pre < 0) {
      Pre = Size - 1;
    }
    //
    //
    //
    v = rand() % 8 + 1;
    if (v < 4) {
      _StockInfo[i].Price[WrOff] = _StockInfo[i].Price[Pre];
      continue;
    }
    //
    // Generate the stock price variation.
    // Normally; the stock price variation is in a range between -4 and 4.
    // If the stock price is > 80, the range for variation will be enhanced.
    // We need the limitation of the stock prices to verify that the graph looks nice.
    //
    if (_StockInfo[i].Price[Pre] <= 80) {
      Limit = 80;
    } else {
      Limit = (rand() % 900 + 1);
    }
    if (_StockInfo[i].Price[Pre] < Limit) {
      v  = (rand() % 8 + 1);
      v -= 4;
    } else {
      v  = (rand() % 60 + 1);
      if (v > 0) {
        v = -v;
      }
    }
    v += _StockInfo[i].Price[Pre];
    if (v > 0) {
      _StockInfo[i].Price[WrOff] = v;
    } else {
      _StockInfo[i].Price[WrOff] = 0;
    }
  }
}

/*********************************************************************
*
*       _CreateGraphData
*
*  Function description:
*    Initializes the stock price information.
*    Pseudo random values used to simulate the stock prices.
*    The ring buffer used to store the stock prices is always filled
*    completely.
*/
static void _CreateGraphData(void) {
  int  i;
  int  j;
  int  v;
  char c;

#ifndef _WIN32
  srand(OS_GetTime32());
#else
  srand((unsigned int)time(NULL));
#endif
  c = 'A';
  for(i = 0; i < NUM_STOCKS; i++) {
    //
    // Initialize buffer
    //
    _StockInfo[i].Size    = sizeof(_StockInfo[i].Price) / sizeof(int);
    _StockInfo[i].RdOff   = 0; // _StockInfo[i].Size - 1;
    //
    // Add company name
    //
    _StockInfo[i].CompanyName = c;
    c++;
    //
    // Generate initial stock price
    //
    v = rand() % 50 + 21;
    _StockInfo[i].Price[0] = v;
    //
    // Generate the stock prices
    //
    for (j = 1; j < _StockInfo[i].Size; j++) {
      v  = rand() % 8 + 1;
      v -= 4;
      v += _StockInfo[i].Price[j-1];
      _StockInfo[i].Price[j] = v;
    }
  }
}

/*********************************************************************
*
*       _callback_CGI_GetData
*
*  Function description:
*    Generates a comma separated list of values used to draw the
*    graph of stock prices.
*/
static void _callback_CGI_GetData(WEBS_OUTPUT* pOutput, const char* sParameters) {
  char  acPara[10];
  char  ac[160];
  char* pStr;
  int   NumBytes;
  int   NumBytesFree;
  int   Index;
  int   RdOff;
  int   Size;
  int   i;
  int   r;
  int   v;

  NumBytesFree = sizeof(ac);
  //
  // Check if we have data, which can be send...
  //
  if (_StockInfo[0].CompanyName == 0) {
    _CreateGraphData();
  }
  pStr = ac;
  r    = IP_WEBS_GetParaValue(sParameters, 0, NULL, 0, acPara, sizeof(acPara));
  //
  // Just for the case that the CGI function has been called without parameter.
  //
  if (r != 0) {
    acPara[0] = '0';
  }
  Index = atoi(acPara);
  RdOff = _StockInfo[Index].RdOff;
  Size  = _StockInfo[Index].Size;
  for(i = 0; i < Size; i++) {
    if (NumBytesFree <= 1) {
      break;
    }
    if (RdOff>= Size) {
      RdOff = 0;
    }
    v             = _StockInfo[Index].Price[RdOff];
    NumBytes      = SEGGER_snprintf(pStr, NumBytesFree, "%d,", v);
    NumBytes      = SEGGER_MIN(NumBytes, NumBytesFree - 1);
    pStr         += NumBytes;
    NumBytesFree -= NumBytes;
    RdOff++;
  }
  *pStr = '\0';
  IP_WEBS_SendString(pOutput, ac);
  _UpdateStockPrices();
}

#ifndef _WIN32
/*********************************************************************
*
*       _itoa
*
*  Function description:
*    Simple itoa implementation.
*    Converts integer into zero-terminated string.
*    Works only with base 10.
*/
static char* _itoa(int Value, char* pStr, int radix) {
  WEBS_USE_PARA(radix);

  SEGGER_snprintf(pStr, 10, "%d", Value);  // Just assume that we have enough space for up to 10 digits in buffer.
  return pStr;
}
#endif

/*********************************************************************
*
*       _fToStr
*
*  Function description:
*    Converts a float to a string with two decimal places.
*/
static void _fToStr(float Value, char* pStr) {
  int v;
  int f;
  int Len;

  //
  // Get int value
  //
  v = (int)Value;
  //
  // Get decimal place
  //
  f = (int)((Value - (float)v) * 100);
  //
  // Convert int to string.
  //
  _itoa(v, pStr, 10);
  Len = strlen(pStr);
  //
  // Add dot
  //
  *(pStr + Len) = '.';
  Len++;
  //
  // Add decimal places
  //
  if (f != 0) {
    _itoa(f, pStr + Len, 10);
    *(pStr + Len + 2) = '\0';
  } else {
    *(pStr + Len) = '0';
    Len++;
    *(pStr + Len) = '0';
    Len++;
    *(pStr + Len) = '\0';
  }
}

/*********************************************************************
*
*       _SendShareTable
*
*  Function description:
*    Generates and sends a table with the stock prices of some
*    fictional companies.
*/
static void _SendShareTable(WEBS_OUTPUT* pOutput) {
  float Percentage;
  int   Curr;
  int   Pre;
  int   Trend;
  int   Change;
  int   i;
  char  ac[10];

  if (_StockInfo[0].CompanyName == 0) {
    _CreateGraphData();
  }
  //
  // Send table with sample shares to the client.
  //
  IP_WEBS_SendString(pOutput, "<h2>Stock prices</h2>");
  IP_WEBS_SendString(pOutput, "<table>");
  IP_WEBS_SendString(pOutput, "<tr>");
  IP_WEBS_SendString(pOutput, "<th>Company</th>");
  IP_WEBS_SendString(pOutput, "<th>Current price</th>");
  IP_WEBS_SendString(pOutput, "<th>Trend</th>");
  IP_WEBS_SendString(pOutput, "<th>Change</th>");
  IP_WEBS_SendString(pOutput, "<th>Change %</th>");
  IP_WEBS_SendString(pOutput, "</tr>");
  for (i = 0; i < 26; i++) {
    //
    // Get position of the current stock price and his predecessor in ring buffer
    //
    Curr = _StockInfo[i].RdOff - 1;
    if (Curr < 0) {
      Curr = _StockInfo[i].Size - 1;
    }
    if (Curr != 0) {
      Pre = Curr - 1;
    } else {
      Pre = _StockInfo[i].Size - 1;
    }
    //
    // We need to get the trend of the stock price for the presentation.
    //
    if (_StockInfo[i].Price[Curr] == _StockInfo[i].Price[Pre]) {
      Trend = 0;    // No change...
    } else if (_StockInfo[i].Price[Curr] > _StockInfo[i].Price[Pre]) {
      Trend = 1;    // Up...
    } else {
      Trend = -1;   // Down...
    }
    //
    // Start table row
    //
    IP_WEBS_SendString(pOutput, "<tr ");
    //
    // Change table row background, if the stock price has been changed.
    //
    switch(Trend) {
    case -1: IP_WEBS_SendString(pOutput, "class=\"Down\">"); break; // All visualizations come from the CSS file...
    case  0: IP_WEBS_SendString(pOutput, ">"); break;
    case  1: IP_WEBS_SendString(pOutput, "class=\"Up\">"); break;
    }
    //
    // Send company name
    //
    IP_WEBS_SendString(pOutput, "<td style=\"cursor:pointer;text-align:left;\" onmouseover=\"style.color='blue'\" onmouseout=\"style.color='black'\" onclick=GetDetails(");
    IP_WEBS_SendUnsigned(pOutput, i, 10, 0);
    IP_WEBS_SendString(pOutput, ")>");
    IP_WEBS_SendString(pOutput, "Company ");
    IP_WEBS_SendString(pOutput, &_StockInfo[i].CompanyName);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Send current stock price
    //
    IP_WEBS_SendString(pOutput, "<td>");
    IP_WEBS_SendUnsigned(pOutput, _StockInfo[i].Price[Curr], 10, 0);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Visualize the trend with an image.
    //
    switch(Trend) {
    case -1: IP_WEBS_SendString(pOutput, "<td class=\"PointerDown\" Title=\"Trend\">");  break;
    case  0: IP_WEBS_SendString(pOutput, "<td class=\"PointerRight\" Title=\"Trend\">"); break;
    case  1: IP_WEBS_SendString(pOutput, "<td class=\"PointerUp\" Title=\"Trend\">");    break;
    }
    IP_WEBS_SendString(pOutput, "</td><td>");
    //
    // Calculate stock price variation
    //
    if (_StockInfo[i].Price[Curr] > 0) {
      if (_StockInfo[i].Price[Pre] != 0) {
        if (_StockInfo[i].Price[Curr] == _StockInfo[i].Price[Pre]) {  // No change.
          Change     = 0;
          Percentage = 0;
        } else {                                                      // Calculate variation
          Change     = _StockInfo[i].Price[Curr] - _StockInfo[i].Price[Pre];
          Percentage = ((float)Change / ((float)_StockInfo[i].Price[Pre] / 100));
        }
      } else {
        Change     = _StockInfo[i].Price[Curr];
        Percentage = 100;
      }
    }
    if (_StockInfo[i].Price[Curr] == 0) {
      if (_StockInfo[i].Price[Pre] != 0) {
        Change     = _StockInfo[i].Price[Pre] * -1;
        Percentage = -100;
      } else {
        Change     = 0;
        Percentage = 0;
      }
    }
    _itoa(Change, ac, 10);
    IP_WEBS_SendString(pOutput, ac);
    IP_WEBS_SendString(pOutput, "</td>");
    //
    // Send the change of the stock price in percentage
    //
    IP_WEBS_SendString(pOutput, "<td>");
    if (Percentage < 0) {
      IP_WEBS_SendString(pOutput, "-");
      Percentage *= -1;
    }
    _fToStr(Percentage, ac);
    IP_WEBS_SendString(pOutput, ac);
    IP_WEBS_SendString(pOutput, "</td>");
    IP_WEBS_SendString(pOutput, "</tr>");
  }
  IP_WEBS_SendString(pOutput, "</table>");
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
}

/*********************************************************************
*
*       _SendSSEShareTable
*/
static int _SendSSEShareTable(WEBS_OUTPUT* pOutput) {
  int r;

  IP_WEBS_SendString(pOutput, "data: ");        // Start of the SSE data
  _SendShareTable(pOutput);
  IP_WEBS_SendString(pOutput, "\n\n");          // End of the SSE data
  r = IP_WEBS_Flush(pOutput);
  return r;
}

/*********************************************************************
*
*       _callback_CGI_SSEGetShareTable
*
*  Function description:
*    Sends the stock price table
*/
static void _callback_CGI_SSEGetShareTable(WEBS_OUTPUT* pOutput, const char* sParameters) {
  int r;

  WEBS_USE_PARA(sParameters);
  //
  // Construct the SSE Header
  //
  IP_WEBS_SendHeaderEx(pOutput, NULL, "text/event-stream", 1);
  IP_WEBS_SendString(pOutput, "retry: 1000\n");  // Normally, the browser attempts to reconnect to the server ~3 seconds after each connection is closed. We change that timeout 1 sec.
  while(1) {
    r = _SendSSEShareTable((void*)pOutput);
    if (r == 0) {     // Data transmitted, Web browser is still waiting for new data.
#ifdef _WIN32
      Sleep(500);
#else
      OS_Delay(500);
#endif
    } else {          // Even if the data transmission was successful, it could be necessary to close the connection after transmission.
      break;          // This is normally the case if the Web server otherwise could not process new connections.
    }
  }
}
#endif // INCLUDE_SHARE_SAMPLE

#if INCLUDE_IP_CONFIG_SAMPLE
/*********************************************************************
*
*       _callback_DHCP_On
*
*  Function description
*    This function is called to set or get the DHCP usage radio button.
*/
static void _callback_DHCP_On(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    _DHCPOnOff = IP_DHCPC_GetState(0);
    if (_DHCPOnOff != 0) {
      IP_WEBS_SendString(pOutput, "checked");
    } else {
      IP_WEBS_SendString(pOutput, "");
    }
  } else {
    _DHCPOnOff = SEGGER_atoi(sValue);
  }
}

/*********************************************************************
*
*       _callback_DHCP_Off
*
*  Function description
*    This function is called to set or get the DHCP usage radio button.
*/
static void _callback_DHCP_Off(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    _DHCPOnOff = IP_DHCPC_GetState(0);
    if (_DHCPOnOff == 0) {
      IP_WEBS_SendString(pOutput, "checked");
    } else {
      IP_WEBS_SendString(pOutput, "");
    }
  } else {
    _DHCPOnOff = SEGGER_atoi(sValue);
  }
}

/*********************************************************************
*
*       _callback_IPAddrX
*/
static void _callback_IPAddrX(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue, unsigned Pos) {
  U32 Tmp;

  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    _IPAddr = IP_GetIPAddr(0);
    Tmp     = (_IPAddr & (0xFF000000 >> (8 * Pos))) >> (24 - 8 * Pos);
    IP_WEBS_SendUnsigned(pOutput, Tmp, 10, 0);
  } else {
    int v;
    v = SEGGER_atoi(sValue);
    if (v > 255) {
      v = 255;
    }
    if (v < 0) {
      v = 0;
    }
    _IPAddr  &= ~(0xFF000000 >> (8 * Pos));
    _IPAddr  |= v << (24 - 8 * Pos);
  }
}

/*********************************************************************
*
*       _callback_IPMaskX
*/
static void _callback_IPMaskX(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue, unsigned Pos) {
  U32 Tmp;

  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    IP_GetAddrMask(0, &Tmp, &_IPMask);
    Tmp = (_IPMask & (0xFF000000 >> (8 * Pos))) >> (24 - 8 * Pos);
    IP_WEBS_SendUnsigned(pOutput, Tmp, 10, 0);
  } else {
    int v;
    v = SEGGER_atoi(sValue);
    if (v > 255) {
      v = 255;
    }
    if (v < 0) {
      v = 0;
    }
    _IPMask  &= ~(0xFF000000 >> (8 * Pos));  // Clear the bits before changing the value
    _IPMask  |= v << (24 - 8 * Pos);
  }
}

/*********************************************************************
*
*       _callback_IPGWX
*/
static void _callback_IPGWX(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue, unsigned Pos) {
  U32 Tmp;

  WEBS_USE_PARA(sPara);

  if (sValue == NULL) {
    _IPGW = IP_GetGWAddr(0);
    Tmp   = (_IPGW & (0xFF000000 >> (8 * Pos))) >> (24 - 8 * Pos);
    IP_WEBS_SendUnsigned(pOutput, Tmp, 10, 0);
  } else {
    int v;
    v = SEGGER_atoi(sValue);
    if (v > 255) {
      v = 255;
    }
    if (v < 0) {
      v = 0;
    }
    _IPGW  &= ~(0xFF000000 >> (8 * Pos));  // Clear the bits before changing the value
    _IPGW  |= v << (24 - 8 * Pos);
  }
}

/*********************************************************************
*
*       _callback_IPAddrX
*       _callback_IPMaskX
*       _callback_IPGWX
*/
static void _callback_IPAddr0(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPAddrX(pOutput, sPara, sValue, 0);
}
static void _callback_IPAddr1(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPAddrX(pOutput, sPara, sValue, 1);
}
static void _callback_IPAddr2(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPAddrX(pOutput, sPara, sValue, 2);
}
static void _callback_IPAddr3(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPAddrX(pOutput, sPara, sValue, 3);
}
static void _callback_IPMask0(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPMaskX(pOutput, sPara, sValue, 0);
}
static void _callback_IPMask1(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPMaskX(pOutput, sPara, sValue, 1);
}
static void _callback_IPMask2(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPMaskX(pOutput, sPara, sValue, 2);
}
static void _callback_IPMask3(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPMaskX(pOutput, sPara, sValue, 3);
}
static void _callback_IPGW0(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPGWX(pOutput, sPara, sValue, 0);
}
static void _callback_IPGW1(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPGWX(pOutput, sPara, sValue, 1);
}
static void _callback_IPGW2(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPGWX(pOutput, sPara, sValue, 2);
}
static void _callback_IPGW3(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {
  _callback_IPGWX(pOutput, sPara, sValue, 3);
}

static void _callback_SetIP(WEBS_OUTPUT* pOutput, const char* sPara, const char* sValue) {

  WEBS_USE_PARA(pOutput);
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);

  if (_DHCPOnOff != 0) {
    //
    // Check if DHCP is already activated.
    //
    if (IP_DHCPC_GetState(0) == 0) {
      (void)IP_DHCPC_Activate(0, "Target", NULL, NULL);
    }
  } else {
    //
    // Check if DHCP is already activated.
    //
    if (IP_DHCPC_GetState(0)) {
      (void)IP_DHCPC_Halt(0);
    }
    //
    IP_SetGWAddr(0, _IPGW);
    IP_SetAddrMask(_IPAddr, _IPMask);
  }
}

#endif // INCLUDE_IP_CONFIG_SAMPLE

/*********************************************************************
*
*       _REST_cb
*
*  Function description
*    Callback for demonstrating REST implementation using a METHOD
*    hook. As there is no clearly defined standard for REST this
*    implementation shall act as sample and starting point on how
*    REST support could be implemented by you.
*
*  Parameters
*    pContext      - Context for incoming data
*    pOutput       - Context for outgoing data
*    sMethod       - String containing used METHOD
*    sAccept       - NULL or string containing value of "Accept" field of HTTP header
*    sContentType  - NULL or string containing value of "Content-Type" field of HTTP header
*    sResource     - String containing URI that was accessed
*    ContentLen    - 0 or length of data submitted by client
*
*  Return value
*    0             - O.K.
*    Other         - Error
*/
static int _REST_cb(void *pContext, WEBS_OUTPUT *pOutput, const char *sMethod, const char *sAccept, const char *sContentType, const char *sResource, U32 ContentLen) {
  const char* pURI;
        int   Len;
        char  acAccept[128];
        char  acContentType[32];

  //
  // Strings located at sAccept and sContentType need to be copied to
  // another location before calling any other Web Server API as they
  // will be overwritten.
  //
  if (sAccept) {
    _CopyString(acAccept, sAccept, sizeof(acAccept));
  }
  if (sContentType) {
    _CopyString(acContentType, sContentType, sizeof(acContentType));
  }
  //
  // Send implementation specific header to client
  //
#if (WEBS_USE_ENHANCED_FORM_HANDLING == 0)
  IP_WEBS_SendHeader(pOutput, NULL, "application/REST");
#else
  {
    WEBS_HEADER_CONFIG HeaderConfig;

    memset(&HeaderConfig, 0, sizeof(HeaderConfig));
    HeaderConfig.sMIMEType  = "application/REST";
    HeaderConfig.sAddFields = "TestField: TestValue\n";  // Add a custom field. Multiple fields seperated by newline are possible.
    IP_WEBS_SendHeader(pOutput, &HeaderConfig);
  }
#endif
  //
  // Output information about the METHOD used by the client
  //
  IP_WEBS_SendString(pOutput, "METHOD:       ");
  IP_WEBS_SendString(pOutput, sMethod);
  IP_WEBS_SendString(pOutput, "\n");
  //
  // Output information about which URI has been accessed by the client.
  // Try to get the "full URI" with URI/GET parameters.
  //
  IP_WEBS_SendString(pOutput, "URI:          ");
  pURI = IP_WEBS_GetURI(pOutput, 1);
  if (pURI != NULL) {
    IP_WEBS_SendString(pOutput, pURI);
  } else {
    IP_WEBS_SendString(pOutput, sResource);
  }
  IP_WEBS_SendString(pOutput, "\n");
  //
  // Output information about "Accept" field given in header sent by client, if any
  //
  if (sAccept) {
    IP_WEBS_SendString(pOutput, "Accept:       ");
    IP_WEBS_SendString(pOutput, acAccept);  //lint !e645
    IP_WEBS_SendString(pOutput, "\n");
  }
  //
  // Output information about "Content-Type" field given in header sent by client, if any
  //
  if (sContentType) {
    IP_WEBS_SendString(pOutput, "Content-Type: ");
    IP_WEBS_SendString(pOutput, acContentType);  //lint !e645
  }
  //
  // Output content sent by client, or content previously sent by client that has been saved
  //
  if ((_acRestContent[0] || ContentLen) && sContentType) {
    IP_WEBS_SendString(pOutput, "\n");
  }
  if (_acRestContent[0] || ContentLen) {
    IP_WEBS_SendString(pOutput, "Content:\n");
  }
  if (ContentLen) {
    //
    // Update saved content
    //
    Len = SEGGER_MIN(sizeof(_acRestContent) - 1, ContentLen);
    IP_WEBS_METHOD_CopyData(pContext, _acRestContent, Len);
    _acRestContent[Len] = 0;
  }
  if (_acRestContent[0]) {
    IP_WEBS_SendString(pOutput, _acRestContent);
  }
  return 0;
}

#if WEBS_USE_AUTH_DIGEST
/*********************************************************************
*
*       _cbAuthDigest_StoreNonce()
*
*  Function description
*    Looks up previously used nonces or generates a new one upon
*    request to be used with clients that are told to use digest
*    authentication.
*
*  Parameters
*    pOutput      : Connection context.
*    pDigestOutput: Auth digest context.
*    pfStore      : Callback to execute for storing the nonce in the
*                   message to send.
*    GenerateNew  : Generate a new nonce (for use in a 401 response).
*
*  Additional information
*    A new nonce should be generated on each 401 response sent to
*    a client (wrong authentication or no authentication sent). This
*    is indicated by "GenerateNew != 0". If "GenerateNew == 0" a
*    nonce previously sent to a client should be looked up and sent
*    again. Therefore the nonces used with each client connection
*    should be stored in a cache (for example one nonce for each
*    client IP address and protected path) so they can be reused.
*    A simple but not the most secure solution in terms of replay
*    attacks is to generate always the same nonce based on fixed
*    parameters. For the simplicity of this sample this solution
*    is used here.
*/
static void _cbAuthDigest_StoreNonce(WEBS_OUTPUT* pOutput, WEBS_AUTH_DIGEST_OUTPUT* pDigestOutput, void (*pfStore)(WEBS_OUTPUT* pOutput, WEBS_AUTH_DIGEST_OUTPUT* pDigestOutput, const char* pNonce, unsigned NonceLen), int GenerateNew) {
  const char* sPath;
        char acMD5[32];

  WEBS_USE_PARA(GenerateNew);  // Should be used to generate unique nonces for each client on a 401 response.
  //
  // For simplicity our nonce is always based on the protected
  // path that is tried to be accessed. We do not check for
  // a NULL pointer on sPath because we would not be in this
  // callback if no protected path is accessed.
  //
  sPath = IP_WEBS_GetProtectedPath(pOutput);
  IP_WEBS_AUTH_DIGEST_CalcHA1(sPath, strlen(sPath), &acMD5[0], sizeof(acMD5));
  pfStore(pOutput, pDigestOutput, (const char*)&acMD5[0], sizeof(acMD5));
}

static const WEBS_AUTH_DIGEST_APP_API _DigestAPI = {
  _cbAuthDigest_StoreNonce
};
#endif

/*********************************************************************
*
*       _GetTimeDate
*
*  Function description:
*    Returns current time and date.
*
*    Bit 0-4:   2-second count (0-29)
*    Bit 5-10:  Minutes (0-59)
*    Bit 11-15: Hours (0-23)
*    Bit 16-20: Day of month (1-31)
*    Bit 21-24: Month of year (1-12)
*    Bit 25-31: Count of years from 1980 (0-127)
*
*  Note:
*    This is a sample implementation for a clock-less system.
*    It always returns 01 Jan 2009 00:00:00 GMT
*/
static U32 _GetTimeDate(void) {
  U32 r;
  U16 Sec, Min, Hour;
  U16 Day, Month, Year;

  Sec   = 0;        // 0 based.  Valid range: 0..59
  Min   = 0;        // 0 based.  Valid range: 0..59
  Hour  = 0;        // 0 based.  Valid range: 0..23
  Day   = 1;        // 1 based.    Means that 1 is 1. Valid range is 1..31 (depending on month)
  Month = 1;        // 1 based.    Means that January is 1. Valid range is 1..12.
  Year  = 29;       // 1980 based. Means that 2008 would be 28.
  r   = Sec / 2 + (Min << 5) + (Hour  << 11);
  r  |= (U32)(Day + (Month << 5) + (Year  << 9)) << 16;
  return r;
}

/*********************************************************************
*
*       _pfGetFileInfo
*/
static void _pfGetFileInfo(const char *sFilename, IP_WEBS_FILE_INFO *pFileInfo){
  int v;

  //
  // .cgi files are virtual, everything else is not
  //
  v = IP_WEBS_CompareFilenameExt(sFilename, ".cgi");
  pFileInfo->IsVirtual = v ? 0 : 1;
  //
  // .htm/.html files contain dynamic content, everything else is not
  //
  v   = IP_WEBS_CompareFilenameExt(sFilename, ".htm");
  if (v != 0) {
    v = IP_WEBS_CompareFilenameExt(sFilename, ".html");
  }
  pFileInfo->AllowDynContent = v ? 0 : 1;
  //
  // If file is a virtual file or includes dynamic content,
  // get current time and date stamp as file time
  //
  pFileInfo->DateLastMod = _GetTimeDate();
  if (pFileInfo->IsVirtual || pFileInfo->AllowDynContent) {
    //
    // Set expiration time and date
    //
    pFileInfo->DateExp = _GetTimeDate(); // If "Expires" HTTP header field should be transmitted, set expiration date.
  } else {
    pFileInfo->DateExp = 0xEE210000;     // Expiration far away (01 Jan 2099) if content is static
  }
}

/*********************************************************************
*
*       _CB_HandleParameter
*/
static void _CB_HandleParameter(WEBS_OUTPUT *pOutput, const char *sPara, const char *sValue) {
  WEBS_USE_PARA(pOutput);
  WEBS_USE_PARA(sPara);
  WEBS_USE_PARA(sValue);
}

#if WEBS_USE_AUTH_DIGEST

/*********************************************************************
*
*       _aAccessControl_Digest
*
*  The HA1 hash assigned per user and protected path can be
*  calculated by using the following function (the HA1 value/string
*  can then be fetched from acHA1 and stored in the table manually):
*    char acHA1[33];
*    const char* sUserRealmPass = "User:Login for configuration:Pass";
*    IP_WEBS_AUTH_DIGEST_CalcHA1(sUserRealmPass, strlen(sUserRealmPass), &acHA1[0], sizeof(acHA1));
*    acHA1[32] = 0;
*/
static const WEBS_ACCESS_CONTROL _aAccessControl_Digest[] = {
  { "/conf/", "Login for configuration", "user", "59294bd4737af087eea5da392cc23c14" },  // Password: pass
  { "/"     , NULL                     , NULL  , NULL },
  { NULL    , NULL                     , NULL  , NULL }
};

#else

/*********************************************************************
*
*       _aAccessControl_Basic
*/
static const WEBS_ACCESS_CONTROL _aAccessControl_Basic[] = {
  { "/conf/", "Login for configuration", "user:pass", NULL },
  { "/"     , NULL                     , NULL       , NULL },
  { NULL    , NULL                     , NULL       , NULL }
};

#endif

/*********************************************************************
*
*       _aVFiles
*
*  Function description
*    Defines all virtual files used by the web server.
*/
static const WEBS_VFILES _aVFiles[]  = {
#if INCLUDE_SHARE_SAMPLE
  {"GetData.cgi",        _callback_CGI_GetData           },   // Called from Shares.htm
  {"GetShareTable.cgi",  _callback_CGI_SSEGetShareTable  },   // Called from Shares.htm
#endif
#if INCLUDE_PRESENTATION_SAMPLE
  {"GetDetails.cgi",     _callback_CGI_GetDetails        },   // Called from Presentation.htm
#endif
  {"Send.cgi",           _callback_CGI_Send              },   // Called from VirtFile.htm
  {"SSETime.cgi" ,       _callback_CGI_SSETime           },   // Called from SSE_Time.htm
  {"SSEembOS.cgi",       _callback_CGI_SSEembOS          },   // Called from SSE_OS.htm
  {"SSENet.cgi",         _callback_CGI_SSENet            },   // Called from SSE_IP.htm
  {"Upload.cgi",         _callback_CGI_UploadFile        },   // Called from Upl.htm and Upl_AJAX.htm
  { NULL, NULL }
};

/*********************************************************************
*
*       _aCGI
*
*  Function description
*    CGI table, defining callback routines for dynamic content (SSI).
*/
static const WEBS_CGI _aCGI[] = {
  {"CookieCounter"      , _callback_ExecCookieCounter     },  // Called from every page. Page visit counter based on a user cookie.
  {"Counter"            , _callback_ExecCounter           },  // Called from index.htm
  {"GetIndex"           , _callback_ExecGetIndex          },  // Called from index.htm
  {"GetIPAddr"          , _callback_ExecGetIPAddr         },
  {"GetOSInfo"          , _callback_ExecGetOSInfo         },
  {"GetIPInfo"          , _callback_ExecGetConnectionInfo },
//  {"GetIFaceInfos"      , _callback_ExecGetIFaceInfo      },
  {"FirstName"          , _callback_ExecFirstName         },
  {"LastName"           , _callback_ExecLastName          },
  {"Percentage"         , _callback_ExecPercentage        },
  {"LED0"               , _callback_LED0                  },
  {"LED1"               , _callback_LED1                  },
  {"LED2"               , _callback_LED2                  },
  {"LED3"               , _callback_LED3                  },
  {"LED4"               , _callback_LED4                  },
  {"LED5"               , _callback_LED5                  },
  {"LED6"               , _callback_LED6                  },
  {"LED7"               , _callback_LED7                  },
  {"SetLEDs"            , _callback_SetLEDs               },
#if INCLUDE_IP_CONFIG_SAMPLE
  {"AssignIPType_Auto"  , _callback_DHCP_On               },
  {"AssignIPType_Manual", _callback_DHCP_Off              },
  {"IPAddr_0"           , _callback_IPAddr0               },
  {"IPAddr_1"           , _callback_IPAddr1               },
  {"IPAddr_2"           , _callback_IPAddr2               },
  {"IPAddr_3"           , _callback_IPAddr3               },
  {"IPMask_0"           , _callback_IPMask0               },
  {"IPMask_1"           , _callback_IPMask1               },
  {"IPMask_2"           , _callback_IPMask2               },
  {"IPMask_3"           , _callback_IPMask3               },
  {"IPGateway_0"        , _callback_IPGW0                 },
  {"IPGateway_1"        , _callback_IPGW1                 },
  {"IPGateway_2"        , _callback_IPGW2                 },
  {"IPGateway_3"        , _callback_IPGW3                 },
  {"SetIP"              , _callback_SetIP                 },
#endif // INCLUDE_IP_CONFIG_SAMPLE
  {NULL                 , _callback_DefaultHandler        }
};

#ifndef _WIN32  // UPnP currently can not be used standalone.

/*********************************************************************
*
*       _UPnP_GetURLBase
*
* Function description
*   This function copies the information needed for the URLBase parameter
*   into the given buffer and returns a pointer to the start of the buffer
*   for easy readable code.
*
* Parameters
*   IFaceId        - Zero-based interface index.
*   pBuffer        - Pointer to the buffer that can be temporarily used to
*                    store the requested data.
*   NumBytes       - Size of the given buffer used for checks
*
* Return value
*   Pointer to the start of the buffer used for storage.
*/
static const char * _UPnP_GetURLBase(unsigned IFaceId, char * pBuffer, unsigned NumBytes) {
#define URL_BASE_PREFIX  "http://"
  char * p;

  p = pBuffer;

  *p = '\0';  // Just to be on the safe if the buffer is too small
  strncpy(pBuffer, URL_BASE_PREFIX, NumBytes);
  p        += (sizeof(URL_BASE_PREFIX) - 1);
  NumBytes -= (sizeof(URL_BASE_PREFIX) - 1);
  IP_PrintIPAddr(p, IP_GetIPAddr(IFaceId), NumBytes);
  return pBuffer;
}

/*********************************************************************
*
*       _UPnP_GetModelNumber
*
* Function description
*   This function copies the information needed for the ModelNumber parameter
*   into the given buffer and returns a pointer to the start of the buffer
*   for easy readable code.
*
* Parameters
*   IFaceId        - Zero-based interface index.
*   pBuffer        - Pointer to the buffer that can be temporarily used to
*                    store the requested data.
*   NumBytes       - Size of the given buffer used for checks
*
* Return value
*   Pointer to the start of the buffer used for storage.
*/
static const char * _UPnP_GetModelNumber(unsigned IFaceId, char * pBuffer, unsigned NumBytes) {
  U8 aHWAddr[6];

  if (NumBytes <= 12) {
    *pBuffer = '\0';  // Just to be on the safe if the buffer is too small
  } else {
    IP_GetHWAddr(IFaceId, aHWAddr, sizeof(aHWAddr));
    SEGGER_snprintf(pBuffer, NumBytes, "%02X%02X%02X%02X%02X%02X", aHWAddr[0], aHWAddr[1], aHWAddr[2], aHWAddr[3], aHWAddr[4], aHWAddr[5]);
  }
  return pBuffer;
}

/*********************************************************************
*
*       _UPnP_GetSN
*
* Function description
*   This function copies the information needed for the SerialNumber parameter
*   into the given buffer and returns a pointer to the start of the buffer
*   for easy readable code.
*
* Parameters
*   IFaceId        - Zero-based interface index.
*   pBuffer        - Pointer to the buffer that can be temporarily used to
*                    store the requested data.
*   NumBytes       - Size of the given buffer used for checks
*
* Return value
*   Pointer to the start of the buffer used for storage.
*/
static const char * _UPnP_GetSN(unsigned IFaceId, char * pBuffer, unsigned NumBytes) {
  U8 aHWAddr[6];

  if (NumBytes <= 12) {
    *pBuffer = '\0';  // Just to be on the safe if the buffer is too small
  } else {
    IP_GetHWAddr(IFaceId, aHWAddr, sizeof(aHWAddr));
    SEGGER_snprintf(pBuffer, NumBytes, "%02X%02X%02X%02X%02X%02X", aHWAddr[0], aHWAddr[1], aHWAddr[2], aHWAddr[3], aHWAddr[4], aHWAddr[5]);
  }
  return pBuffer;
}

/*********************************************************************
*
*       _UPnP_GetUDN
*
* Function description
*   This function copies the information needed for the UDN parameter
*   into the given buffer and returns a pointer to the start of the buffer
*   for easy readable code.
*
* Parameters
*   IFaceId        - Zero-based interface index.
*   pBuffer        - Pointer to the buffer that can be temporarily used to
*                    store the requested data.
*   NumBytes       - Size of the given buffer used for checks
*
* Return value
*   Pointer to the start of the buffer used for storage.
*/
static const char * _UPnP_GetUDN(unsigned IFaceId, char * pBuffer, unsigned NumBytes) {
#define UDN_PREFIX "uuid:95232DE0-3AF7-11E2-81C1-"
  char * p;
  U8     aHWAddr[6];

  p = pBuffer;

  *pBuffer = '\0';  // Just to be on the safe if the buffer is too small
  strncpy(pBuffer, UDN_PREFIX, NumBytes);
  p        += (sizeof(UDN_PREFIX) - 1);
  NumBytes -= (sizeof(UDN_PREFIX) - 1);
  if (NumBytes > 12) {
    IP_GetHWAddr(IFaceId, aHWAddr, sizeof(aHWAddr));
    SEGGER_snprintf(p, NumBytes, "%02X%02X%02X%02X%02X%02X", aHWAddr[0], aHWAddr[1], aHWAddr[2], aHWAddr[3], aHWAddr[4], aHWAddr[5]);
  }
  return pBuffer;
}

/*********************************************************************
*
*       _UPnP_GetPresentationURL
*
* Function description
*   This function copies the information needed for the presentation URL parameter
*   into the given buffer and returns a pointer to the start of the buffer
*   for easy readable code.
*
* Parameters
*   IFaceId        - Zero-based interface index.
*   pBuffer        - Pointer to the buffer that can be temporarily used to
*                    store the requested data.
*   NumBytes       - Size of the given buffer used for checks
*
* Return value
*   Pointer to the start of the buffer used for storage.
*/
static const char * _UPnP_GetPresentationURL(unsigned IFaceId, char * pBuffer, unsigned NumBytes) {
#define PRESENTATION_URL_PREFIX   "http://"
#define PRESENTATION_URL_POSTFIX  "/index.htm"
  char * p;
  int    i;

  p = pBuffer;

  *p = '\0';  // Just to be on the safe if the buffer is too small
  strncpy(pBuffer, PRESENTATION_URL_PREFIX, NumBytes);
  p        += (sizeof(PRESENTATION_URL_PREFIX) - 1);
  NumBytes -= (sizeof(PRESENTATION_URL_PREFIX) - 1);
  i = IP_PrintIPAddr(p, IP_GetIPAddr(IFaceId), NumBytes);
  //p        += i;
  NumBytes -= i;
  strncat(pBuffer, PRESENTATION_URL_POSTFIX, NumBytes);
  return pBuffer;
}

/*********************************************************************
*
*       _UPnP_GetFriendlyName
*
* Function description
*   This function copies the information needed for the friendly name parameter
*   into the given buffer and returns a pointer to the start of the buffer
*   for easy readable code.
*
* Parameters
*   IFaceId        - Zero-based interface index.
*   pBuffer        - Pointer to the buffer that can be temporarily used to
*                    store the requested data.
*   NumBytes       - Size of the given buffer used for checks
*
* Return value
*   Pointer to the start of the buffer used for storage.
*/
static const char * _UPnP_GetFriendlyName(unsigned IFaceId, char * pBuffer, unsigned NumBytes) {
  char * p;

  p = pBuffer;

  *pBuffer = '\0';  // Just to be on the safe if the buffer is too small
  strncpy(pBuffer, UPNP_FRIENDLY_NAME, NumBytes);
  p        += (sizeof(UPNP_FRIENDLY_NAME) - 1);
  NumBytes -= (sizeof(UPNP_FRIENDLY_NAME) - 1);
  SEGGER_snprintf(p, NumBytes, " IFace #%lu", IFaceId);
  return pBuffer;
}

/*********************************************************************
*
*       _UPnP_GenerateSend_upnp_xml
*
* Function description
*   Send the content for the requested file using the callback provided.
*
* Parameters
*   IFaceId        - Zero-based interface index.
*   pContextIn     - Send context of the connection processed for
*                    forwarding it to the callback used for output.
*   pf             - Function pointer to the callback that has to be
*                    for sending the content of the VFile.
*     pContextOut    - Out context of the connection processed.
*     pData          - Pointer to the data that will be sent
*     NumBytes       - Number of bytes to send from pData. If NumBytes
*                      is passed as 0 the send function will run a strlen()
*                      on pData expecting a string.
*
* Notes
*   (1) The data does not need to be sent in one call of the callback
*       routine. The data can be sent in blocks of data and will be
*       flushed out automatically at least once returning from this
*       routine.
*/
static void _UPnP_GenerateSend_upnp_xml(unsigned IFaceId, void * pContextIn, void (*pf) (void * pContextOut, const char * pData, unsigned NumBytes)) {
  char ac[128];

  pf(pContextIn, "<?xml version=\"1.0\"?>\r\n"
                 "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n"
                   "<specVersion>\r\n"
                     "<major>1</major>\r\n"
                     "<minor>0</minor>\r\n"
                   "</specVersion>\r\n"                                                       , 0);

  pf(pContextIn,   "<URLBase>"                                                                , 0);
  pf(pContextIn,     _UPnP_GetURLBase(IFaceId, ac, sizeof(ac))                                , 0);
  pf(pContextIn,   "</URLBase>\r\n"                                                           , 0);

  pf(pContextIn,   "<device>\r\n"
                     "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>\r\n"       , 0);
  pf(pContextIn,     "<friendlyName>"                                                         , 0);
  pf(pContextIn,       _UPnP_GetFriendlyName(IFaceId, ac, sizeof(ac))                         , 0);
  pf(pContextIn,     "</friendlyName>\r\n"                                                    , 0);
  pf(pContextIn,     "<manufacturer>" UPNP_MANUFACTURER "</manufacturer>\r\n"                 , 0);
  pf(pContextIn,     "<manufacturerURL>" UPNP_MANUFACTURER_URL "</manufacturerURL>\r\n"       , 0);
  pf(pContextIn,     "<modelDescription>" UPNP_MODEL_DESC "</modelDescription>\r\n"           , 0);
  pf(pContextIn,     "<modelName>" UPNP_MODEL_NAME "</modelName>\r\n"                         , 0);

  pf(pContextIn,     "<modelNumber>"                                                          , 0);
  pf(pContextIn,       _UPnP_GetModelNumber(IFaceId, ac, sizeof(ac))                          , 0);
  pf(pContextIn,     "</modelNumber>\r\n"                                                     , 0);

  pf(pContextIn,     "<modelURL>" UPNP_MODEL_URL "</modelURL>\r\n"                            , 0);

  pf(pContextIn,     "<serialNumber>"                                                         , 0);
  pf(pContextIn,       _UPnP_GetSN(IFaceId, ac, sizeof(ac))                                   , 0);
  pf(pContextIn,     "</serialNumber>\r\n"                                                    , 0);

  pf(pContextIn,     "<UDN>"                                                                  , 0);
  pf(pContextIn,       _UPnP_GetUDN(IFaceId, ac, sizeof(ac))                                  , 0);
  pf(pContextIn,     "</UDN>\r\n"                                                             , 0);

  pf(pContextIn,     "<serviceList>\r\n"
                       "<service>\r\n"
                         "<serviceType>urn:schemas-upnp-org:service:Dummy:1</serviceType>\r\n"
                         "<serviceId>urn:upnp-org:serviceId:Dummy</serviceId>\r\n"
                         "<SCPDURL>/dummy.xml</SCPDURL>\r\n"
                         "<controlURL>/</controlURL>\r\n"
                         "<eventSubURL></eventSubURL>\r\n"
                       "</service>\r\n"
                     "</serviceList>\r\n"                                                     , 0);

  pf(pContextIn,     "<presentationURL>"                                                      , 0);
  pf(pContextIn,       _UPnP_GetPresentationURL(IFaceId, ac, sizeof(ac))                      , 0);
  pf(pContextIn,     "</presentationURL>\r\n"                                                 , 0);

  pf(pContextIn,   "</device>\r\n"
                   "</root>"                                                                  , 0);
}

/*********************************************************************
*
*       _UPnP_CheckVFile
*
* Function description
*   Check if we have content that we can deliver for the requested
*   file using the VFile hook system.
*
* Parameters
*   sFileName      - Name of the file that is requested
*   pIndex         - Pointer to a variable that has to be filled with
*                    the index of the entry found in case of using a
*                    filename<=>content list.
*                    Alternative all comparisons can be done using the
*                    filename. In this case the index is meaningless
*                    and does not need to be returned by this routine.
*
* Return value
*   0              - We do not have content to send for this filename,
*                    fall back to the typical methods for retrieving
*                    a file from the web server.
*   1              - We have content that can be sent using the VFile
*                    hook system.
*/
static int _UPnP_CheckVFile(const char * sFileName, unsigned * pIndex) {
  unsigned i;

  //
  // Generated VFiles
  //
  if (strcmp(sFileName, "/upnp.xml") == 0) {
    return 1;
  }
  //
  // Static VFiles
  //
  for (i = 0; i < SEGGER_COUNTOF(_VFileList); i++) {
    if (strcmp(sFileName, _VFileList[i].sFileName) == 0) {
      *pIndex = i;
      return 1;
    }
  }
  return 0;
}

/*********************************************************************
*
*       _UPnP_SendVFile
*
* Function description
*   Send the content for the requested file using the callback provided.
*
* Parameters
*   pContextIn     - Send context of the connection processed for
*                    forwarding it to the callback used for output.
*   Index          - Index of the entry of a filename<=>content list
*                    if used. Alternative all comparisons can be done
*                    using the filename. In this case the index is
*                    meaningless. If using a filename<=>content list
*                    this is faster than searching again.
*   sFileName      - Name of the file that is requested. In case of
*                    working with the Index this is meaningless.
*   pf             - Function pointer to the callback that has to be
*                    for sending the content of the VFile.
*     pContextOut    - Out context of the connection processed.
*     pData          - Pointer to the data that will be sent
*     NumBytes       - Number of bytes to send from pData. If NumBytes
*                      is passed as 0 the send function will run a strlen()
*                      on pData expecting a string.
*/
static void _UPnP_SendVFile(void * pContextIn, unsigned Index, const char * sFileName, void (*pf) (void * pContextOut, const char * pData, unsigned NumBytes)) {
  struct sockaddr_in LocalAddr;
         U32         IPAddr;
         long        hSock;
         int         IFaceId;
         int         Len;

  (void)sFileName;

  //
  // Generated VFiles
  //
  if (strcmp(sFileName, "/upnp.xml") == 0) {
    //
    // Retrieve socket that is used by connection.
    //
    hSock   = (long)IP_WEBS_GetConnectInfo((WEBS_OUTPUT*)pContextIn);
    Len     = sizeof(LocalAddr);
    getsockname(hSock, (struct sockaddr*)&LocalAddr, &Len);
    IPAddr  = ntohl(LocalAddr.sin_addr.s_addr);
    IFaceId = IP_FindIFaceByIP(&IPAddr, sizeof(IPAddr));
    if (IFaceId >= 0) {  // Only send back if we have found the interface.
      _UPnP_GenerateSend_upnp_xml(IFaceId, pContextIn, pf);
    }
    return;
  }
  //
  // Static VFiles
  //
  pf(pContextIn, _VFileList[Index].pData, _VFileList[Index].NumBytes);
}

#endif  // _WIN32

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#ifndef _WIN32  // UPnP currently can not be used standalone.

/*********************************************************************
*
*       WebsSample_UPnP_VFileAPI
*
*  Function description
*    Application data table, defines all UPnP application specifics
*    used by the web server.
*/
const WEBS_VFILE_APPLICATION WebsSample_UPnP_VFileAPI = {
  _UPnP_CheckVFile,
  _UPnP_SendVFile
};

#endif  // _WIN32

/*********************************************************************
*
*       WebsSample_SendPageHeader
*
*  Function description:
*    Sends the header of the virtual file.
*    The virtual files in our sample application use the same HTML layout.
*    The only difference between the virtual files is the content and that
*    each of them use an own title/heading.
*/
void WebsSample_SendPageHeader(WEBS_OUTPUT* pOutput, const char* sName) {
  IP_WEBS_SendString(pOutput, "<!DOCTYPE html><html><head><title>");
  IP_WEBS_SendString(pOutput, sName);
  IP_WEBS_SendString(pOutput, "</title>");
  IP_WEBS_SendString(pOutput, "<link href=\"Styles.css\" rel=\"stylesheet\"></head><body><header>");
  IP_WEBS_SendString(pOutput, sName);
  IP_WEBS_SendString(pOutput, "</header>");
  IP_WEBS_SendString(pOutput, "<div class=\"content\">");
}

/*********************************************************************
*
*       WebsSample_SendPageFooter
*
*  Function description:
*    Sends the footer of the virtual file.
*    The virtual files in our sample application use the same HTML layout.
*/
void WebsSample_SendPageFooter(WEBS_OUTPUT* pOutput) {
  IP_WEBS_SendString(pOutput, "<br><br><br>");
  IP_WEBS_SendString(pOutput, "</div><img src=\"Logo.gif\" alt=\"Segger logo\" class=\"logo\">");
  IP_WEBS_SendString(pOutput, "<footer><p><a href=\"index.htm\">Back to main</a></p>");
  IP_WEBS_SendString(pOutput, "<p>SEGGER Microcontroller GmbH || <a href=\"http://www.segger.com\">www.segger.com</a> ");
  IP_WEBS_SendString(pOutput, "<span class=\"hint\">(external link)</span></p></footer></body></html>");
}

/*********************************************************************
*
*       WebsSample_Application
*
*  Function description
*    Application data table, defines all application specifics used
*    by the web server.
*/
const WEBS_APPLICATION WebsSample_Application = {
  &_aCGI[0],
#if WEBS_USE_AUTH_DIGEST
  (WEBS_ACCESS_CONTROL*)&_aAccessControl_Digest[0],
#else
  (WEBS_ACCESS_CONTROL*)&_aAccessControl_Basic[0],
#endif
  _CB_HandleParameter,
  &_aVFiles[0]
};

/*********************************************************************
*
*       IP_WEBS_X_SampleConfig()
*
*  Function description
*    Helper function used in web server samples. It is used as central
*    point to configure sample functionality like adding a REST
*    resource or switching between Basic and Digest authentication.
*/
void IP_WEBS_X_SampleConfig(void) {
  IP_WEBS_SetFileInfoCallback(_pfGetFileInfo);
  //
  // Add REST sample code.
  //
  IP_WEBS_METHOD_AddHook(&_MethodHook, _REST_cb, "/REST");  // Register URI "http://<ip>/REST" to demonstrate REST implementation.
  //
  // Add cookie sample code.
  //
  IP_WEBS_AddPreContentOutputHook(&_PreContentOutputHook, _cbBeforeContentOutput, WEBS_PRE_DYNAMIC_CONTENT_OUTPUT);
  IP_WEBS_AddProgressHook(&_ProgressHook, _cbOnProgressNotify);
  //
  // Add custom error page code.
  //
  IP_WEBS_SetErrorPageCallback(_cbSendCustomErrorPage, NULL);
#if WEBS_USE_AUTH_DIGEST
  //
  // Use digest authentication over basic authentication.
  //
  IP_WEBS_UseAuthDigest(&_DigestAPI);
#endif
}

#endif

/*************************** End of file ****************************/
