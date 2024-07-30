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

File    : IP_NI_SYNOPSYS_QOS.h
Purpose : Driver specific header file for Synopsys QoS based Ethernet controllers.
*/

#ifndef IP_NI_SYNOPSYS_QOS_H      // Avoid multiple inclusion.
#define IP_NI_SYNOPSYS_QOS_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  U32 MACBaseAddress[2];       // Base address of the Ethernet module(s) MAC registers.
  U32 MTLBaseAddress[2];       // Base address of the Ethernet module(s) MMC registers.
  U32 DMABaseAddress[2];       // Base address of the Ethernet module(s) DMA registers.
  //
  //  Clock and peripheral reset management.
  //
  U32 SetMACResetAddr;         // Address of the register to activate the mac reset (e.g. RCC_AHB1RSTR).
  U32 MACResetMask[2];         // Bit mask to activate the MAC reset.
  U32 SetClockAddr;            // Address of the register to set the clock (e.g. RCC_AHB1ENR).
  U32 ClockMask[2];            // Bits to set the clocks.
  U32 SysCfgAddr;              // Address of the register to set RMII/MII support.
  U32 RMIIMask;                // Mask to set the support of RMII.
  //
  // Other configuration parameters.
  //
  int DriverCaps;              // Available hardware capabilities.
  U8  CacheLineSize;           // MCU makes use of cache.
  U8  Supports1GHz;            // Does the MCU have a Gigabit Ethernet MAC ?
  U8  NumPreciseFilters;       // Number of precise MAC filters.
  U8  HasHashFilters;          // Does the MCU have hash filters ?
  U8  HasMMCRegs;              // Does the MCU have MMC counter regs ?
  U8  HasRXQCTRLRegs;          // Does the MCU have RXQCTRLx regs ?
  U8  EnableItcmDtcmCheck;     // Enable checking that we do not use tightly coupled RAM.
  U8  DslShift;                // Descriptor Skip Length shift.
} IP_NI_SYNOPSYS_QOS_CPU_CONFIG;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_LPC54xxx;
extern const IP_HW_DRIVER IP_Driver_STM32H7;
extern const IP_HW_DRIVER IP_Driver_STM32MP1;

//
// Generic functions
//
void IP_NI_SYNOPSYS_QOS_ConfigMDIOClockRange      (U8 cr);
void IP_NI_SYNOPSYS_QOS_ConfigNumRxBuffers        (unsigned IFaceId, U16 NumRxBuffers);
void IP_NI_SYNOPSYS_QOS_ConfigNumTxBuffers        (unsigned IFaceId, U16 NumRxBuffers);
void IP_NI_SYNOPSYS_QOS_ConfigTransmitStoreForward(unsigned IFaceId, U8 OnOff);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
