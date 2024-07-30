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

File    : IP_NI_WIFI_ATMEL_ATWILC.h
Purpose : Driver specific header file for ATMEL ATWILC1000 WiFi controllers.
*/

#ifndef IP_NI_WIFI_ATMEL_ATWILC_H  // Avoid multiple inclusion.
#define IP_NI_WIFI_ATMEL_ATWILC_H

#include "IP_Int.h"

#if defined(__cplusplus)
  extern "C" {                       // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/


/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_Atmel_ATWILC;

int IP_NI_WIFI_ATMEL_ATWILC_DownloadCert(unsigned IFaceId, U8* pCert, U32 Size, void* p);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
