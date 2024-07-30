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

File    : IP_NI_LPC.h
Purpose : Driver specific header file for LPC Ethernet controllers.
*/

#ifndef IP_NI_LPC_H      // Avoid multiple inclusion.
#define IP_NI_LPC_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  U32 MACBaseAddress;          // Base address of the Ethernet module(s) MAC registers.
  U32 RAMBaseAddress;          // Base address of the RAM for the descriptors and buffers.
  U32 SCBBaseAddress;          // Base address for the SCB registers.
  U8  NumRxBuffers;            // Number of Rx buffers.
  U8  NumTxBuffers;            // Number of Tx buffers.
  U8  ForceRMII;               // Indication to use RMII (to keep old config files valid).
} IP_NI_LPC_CPU_CONFIG;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_LPC17;
extern const IP_HW_DRIVER IP_Driver_LPC24;
extern const IP_HW_DRIVER IP_Driver_LPC32;

//
// LPC17xx
//
#define IP_Driver_LPC17xx                   IP_Driver_LPC17
#define IP_Driver_LPC40xx                   IP_Driver_LPC17
#define IP_NI_LPC17xx_ConfigAddr(m, r)      IP_NI_LPC_ConfigAddr(m, r)

//
// LPC24xx
//
#define IP_Driver_LPC23xx                   IP_Driver_LPC24
#define IP_Driver_LPC24xx                   IP_Driver_LPC24

//
// LPC32xx
//
#define IP_Driver_LPC32xx                   IP_Driver_LPC32

//
// Generic functions
//
void IP_NI_LPC_ConfigAddr(U32 EthMacAddr, U32 EthRamAddr);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
