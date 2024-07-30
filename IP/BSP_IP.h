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

File    : BSP_IP.h
Purpose : Header file for IP related BSP functions.
*/

#ifndef BSP_IP_H              // Avoid multiple inclusion.
#define BSP_IP_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Types, global
*
**********************************************************************
*/

/*********************************************************************
*
*  BSP_IP_
*/
typedef struct {
  void (*pfISR)(void);
  int ISRIndex;
  int Prio;
} BSP_IP_INSTALL_ISR_PARA;

typedef struct {
  void          (*pfInit)       (unsigned IFaceId);                                  // Initializes port pins and clocks for Ethernet. Can be NULL.
  void          (*pfDeInit)     (unsigned IFaceId);                                  // De-initializes port pins and clocks for Ethernet. Can be NULL.
  void          (*pfInstallISR) (unsigned IFaceId, BSP_IP_INSTALL_ISR_PARA* pPara);  // Installs the driver interrupt handler. Can be NULL.
  unsigned      (*pfGetMiiMode) (unsigned IFaceId);                                  // Returns the MII mode that the pins have been configured for (0: MII, 1: RMII). Can be NULL.
  unsigned long (*pfGetEthClock)(void);                                              // Returns the clock frequency [Hz] used by the Ethernet peripheral for auto-configuration of internal parameters. Can be NULL.
} BSP_IP_API;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

extern const BSP_IP_API BSP_IP_Api;  // Default BSP_IP API.

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
