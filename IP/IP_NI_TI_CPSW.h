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

File    : IP_NI_TI_CPSW.h
Purpose : Driver specific header file for TI CPSW NIs.
*/

#ifndef IP_NI_TI_CPSW_H
#define IP_NI_TI_CPSW_H

#if defined(__cplusplus)
extern "C" {  /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types/structures
*
**********************************************************************
*/

typedef struct {
  U32 CpswAleBaseAddr;
  U32 CpswCpdmaBaseAddr;
  U32 CpswCptsBaseAddr;
  U32 CpswStatsBaseAddr;
  U32 CpdmaStateramBaseAddr;
  U32 CpswPortBaseAddr;
  U32 aCpswSlBaseAddr[2];
  U32 CpswSsBaseAddr;
  U32 CpswWrBaseAddr;
  U32 MdioBaseAddr;
  U8  HasCache;
  U8  Supports1GHz;
} IP_NI_TI_CPSW_CPU_CONFIG;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_AM335x;

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

void IP_NI_TI_CPSW_ConfigNumRxBuffers(U16 NumRxBuffers);
int  IP_NI_TI_CPSW_Init              (unsigned Unit, const IP_NI_TI_CPSW_CPU_CONFIG *pCpuConfig);


#if defined(__cplusplus)
  }     // Make sure we have C-declarations in C++ programs
#endif

#endif  // Avoid multiple inclusion

/****** End Of File *************************************************/
