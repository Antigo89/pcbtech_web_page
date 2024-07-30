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
File    : IP_NI_AM1808.h
Purpose : Driver specific header file for the TI AM1808
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef IP_NI_AM1808_H
#define IP_NI_AM1808_H

#if defined(__cplusplus)
  extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

//
// EMAC IRQ index for interchange between driver and BSP.
//
#define IP_NI_AM1808_EMAC_RX_ISR    0
#define IP_NI_AM1808_EMAC_TX_ISR    1
#define IP_NI_AM1808_EMAC_MISC_ISR  2

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_AM1808;

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/

void IP_NI_AM1808_ConfigNumRxBuffers(U16 NumRxBuffers);

#if defined(__cplusplus)
  }    // Make sure we have C-declarations in C++ programs
#endif

/********************************************************************/

#endif // Avoid multiple inclusion

/*************************** End of file ****************************/



