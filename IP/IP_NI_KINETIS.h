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

File    : IP_NI_KINETIS.h
Purpose : Driver specific header file for NXP (Freescale) Kinetis devices.
*/

#ifndef IP_DRIVER_KINETIS_H  // Avoid multiple inclusion
#define IP_DRIVER_KINETIS_H

#if defined(__cplusplus)
extern "C" {  // Make sure we have C-declarations in C++ programs */
#endif


/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  U32 ENETBaseAddress[2];      // Base address of the Ethernet module(s) registers.
  //
  // Other configuration parameters.
  //
  U16 mscr;                    // MDIO divider: Internal module clock / ((x + 1) * 2) = less or equal 2.5MHz .
  U8  Supports1GHz;            // Do MAC unit and PHY support 1Gbit mode ?
} IP_NI_KINETIS_CPU_CONFIG;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

//
// The controller explicitly needs to know about a Gigabit PHY or not.
// Therefore variations like ENET_1G (Giogabit capable controller) with
// and without Gbit exist.
//
extern const IP_HW_DRIVER IP_Driver_Kinetis;
extern const IP_HW_DRIVER IP_Driver_S32K;
extern const IP_HW_DRIVER IP_Driver_iMXRT10xx;
extern const IP_HW_DRIVER IP_Driver_iMXRT11xx_ENET;
extern const IP_HW_DRIVER IP_Driver_iMXRT11xx_ENET_1G;
extern const IP_HW_DRIVER IP_Driver_iMXRT11xx_ENET_1G_Gbit;
extern const IP_HW_DRIVER IP_Driver_iMXRT11xx_Gbit;
extern const IP_HW_DRIVER IP_Driver_iMX6Solo;
extern const IP_HW_DRIVER IP_Driver_iMX6Solo_Gbit;

//
// Macros for compatible devices.
//
#define IP_Driver_K60                IP_Driver_Kinetis
#define IP_Driver_K64                IP_Driver_Kinetis
#define IP_Driver_K66                IP_Driver_Kinetis
#define IP_Driver_K70                IP_Driver_Kinetis
#define IP_Driver_iMXRT              IP_Driver_iMXRT10xx
#define IP_Driver_iMXRT11xx          IP_Driver_iMXRT11xx_ENET
#define IP_Driver_iMX6SoloGbit       IP_Driver_iMX6Solo_Gbit
#define IP_Driver_iMX6DualLite       IP_Driver_iMX6Solo
#define IP_Driver_iMX6DualLiteGbit   IP_Driver_iMX6Solo_Gbit
#define IP_Driver_iMX6DualLite_Gbit  IP_Driver_iMX6Solo_Gbit

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#define IP_NI_K60_EnableHWChecksumComputation  IP_NI_KINETIS_EnableHWChecksumComputation

U16 IP_NI_KINETIS_ConfigNumRxBuffers           (unsigned IFaceId, U16 NumRxBuffers);
U8  IP_NI_KINETIS_EnableHWChecksumComputation  (U8 OnOff);
U8  IP_NI_KINETIS_EnableHWChecksumComputationEx(unsigned IFaceId, U8 OnOff);

#if defined(__cplusplus)
  }           // Make sure we have C-declarations in C++ programs
#endif

/********************************************************************/

#endif        // Avoid multiple inclusion

/*************************** End of file ****************************/
