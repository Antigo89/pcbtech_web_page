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

File    : IP_NI_CADENCE.h
Purpose : Driver specific header file for the CADENCE NIs.
*/

#ifndef IP_NI_CADENCE_H       // Avoid multiple inclusion.
#define IP_NI_CADENCE_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Compatibility macros
*
**********************************************************************
*/

#define IP_NI_SAM9XE_ConfigNumRxBuffers(x)  IP_NI_CADENCE_ConfigNumRxBuffers(0, x)
#define IP_NI_SAM7X_ConfigNumRxBuffers(x)   IP_NI_CADENCE_ConfigNumRxBuffers(0, x)
#define IP_NI_SAM9260_ConfigNumRxBuffers(x) IP_NI_CADENCE_ConfigNumRxBuffers(0, x)
#define IP_NI_SAM9263_ConfigNumRxBuffers(x) IP_NI_CADENCE_ConfigNumRxBuffers(0, x)
#define IP_NI_SAM9G20_ConfigNumRxBuffers(x) IP_NI_CADENCE_ConfigNumRxBuffers(0, x)
#define IP_NI_SAM9G45_ConfigNumRxBuffers(x) IP_NI_CADENCE_ConfigNumRxBuffers(0, x)
#define IP_NI_SAM3X_ConfigNumRxBuffers(x)   IP_NI_CADENCE_ConfigNumRxBuffers(0, x)
#define IP_NI_CAP9_ConfigNumRxBuffers(x)    IP_NI_CADENCE_ConfigNumRxBuffers(0, x)

#define IP_Driver_SAM9XE  IP_Driver_AT91SAM9XE
#define IP_Driver_SAM9X25 IP_Driver_AT91SAM9X25
#define IP_Driver_SAM9X35 IP_Driver_AT91SAM9X35
#define IP_Driver_SAM7X   IP_Driver_AT91SAM7X
#define IP_Driver_SAM9G20 IP_Driver_AT91SAM9G20
#define IP_Driver_SAM9G45 IP_Driver_AT91SAM9G45
#define IP_Driver_SAM9260 IP_Driver_AT91SAM9260
#define IP_Driver_SAM9263 IP_Driver_AT91SAM9263
#define IP_Driver_SAM3X   IP_Driver_AT91SAM3X
#define IP_Driver_CAP9    IP_Driver_AT91CAP9

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  U32 UsrioMask;
  U32 NcfgrMask;
  U32 MclkDivDefaultMask;
  U32 DcfgrMask;
  U32 MacBaseAddr[2];
  int DriverCaps;
  U8  HasCache;
  U8  Supports1GHz;
  U8  WriteIsrToClr;
  U8  UseIPv6Promiscuous;
  U8  UsrioOffset;
  U8  FilterOffset;
  U8  HasPrioQueues;
  U8  HasInvertedRmiiFlag;
  U8  RxBufferOffset;
} IP_NI_CADENCE_CPU_CONFIG;

typedef void IP_NI_CADENCE_ON_PHY_UPDATE_CB(unsigned Unit, U32 Speed, void* pContext);

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_AT91SAM9XE;
extern const IP_HW_DRIVER IP_Driver_AT91SAM9X25;
extern const IP_HW_DRIVER IP_Driver_AT91SAM9X35;
extern const IP_HW_DRIVER IP_Driver_ATSAMA5D3_GMAC;
extern const IP_HW_DRIVER IP_Driver_ATSAMA5D3_EMAC;
extern const IP_HW_DRIVER IP_Driver_ATSAMA5D4;
extern const IP_HW_DRIVER IP_Driver_Zynq_7000;
extern const IP_HW_DRIVER IP_Driver_ATSAM4E_GMAC;
extern const IP_HW_DRIVER IP_Driver_ATSAMV71;
extern const IP_HW_DRIVER IP_Driver_AT91SAM7X;
extern const IP_HW_DRIVER IP_Driver_AT91SAM9G20;
extern const IP_HW_DRIVER IP_Driver_AT91SAM9G45;
extern const IP_HW_DRIVER IP_Driver_AT91SAM9260;
extern const IP_HW_DRIVER IP_Driver_AT91SAM9263;
extern const IP_HW_DRIVER IP_Driver_AT91SAM3X;
extern const IP_HW_DRIVER IP_Driver_AT91CAP9;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

void IP_NI_CADENCE_ConfigMDIOClockDivider(unsigned Unit, U32 DividerMask);
void IP_NI_CADENCE_ConfigNumRxBuffers    (unsigned Unit, U16 NumRxBuffers);
void IP_NI_CADENCE_ConfigNumTxBuffers    (unsigned Unit, U16 NumTxBuffers);
int  IP_NI_CADENCE_Init                  (unsigned Unit, const IP_NI_CADENCE_CPU_CONFIG* pCpuConfig);
void IP_NI_CADENCE_SetPhyUpdateCallback  (unsigned Unit, IP_NI_CADENCE_ON_PHY_UPDATE_CB* pf);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
