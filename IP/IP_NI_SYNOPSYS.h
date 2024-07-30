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

File    : IP_NI_SYNOPSYS.h
Purpose : Driver specific header file for Synopsys based Ethernet controllers.
*/

#ifndef IP_NI_SYNOPSYS_H      // Avoid multiple inclusion.
#define IP_NI_SYNOPSYS_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// Compatibility defines that have never been used internally but might
// have been used by the user application.
//
#ifndef    IP_NI_XMC45XX_RX_BUFFER_SIZE
  #define  IP_NI_XMC45XX_RX_BUFFER_SIZE  IP_NI_SYNOPSYS_RX_BUFFER_SIZE
#endif

#ifndef    IP_NI_LPC43XX_BUFFER_SIZE
  #define  IP_NI_LPC43XX_BUFFER_SIZE     IP_NI_SYNOPSYS_RX_BUFFER_SIZE
#endif

//
// Device specific buffer configurations.
// Can be used to calculate driver internal memory requirements.
// Default values used if not set via configuration functions.
//
#ifndef   IP_NI_XMC45XX_NUM_RX_BUFFERS
  #define IP_NI_XMC45XX_NUM_RX_BUFFERS  (36)
#endif
#ifndef   IP_NI_XMC45XX_NUM_TX_BUFFERS
  #define IP_NI_XMC45XX_NUM_TX_BUFFERS  (1)     // Ideal configuration is one more than IP packet buffers used for sending.
#endif

#ifndef   IP_NI_LPC43XX_NUM_RX_BUFFERS
  #define IP_NI_LPC43XX_NUM_RX_BUFFERS  (36)
#endif
#ifndef   IP_NI_LPC43XX_NUM_TX_BUFFERS
  #define IP_NI_LPC43XX_NUM_TX_BUFFERS  (1)     // Fixed to 1. The stack will only use one buffer.
#endif

//
// Generic buffer configuration for all other devices.
// Can be used to calculate driver internal memory requirements.
// Default values used if not set via configuration functions.
// This is used for all devices not listed above.
//
#ifndef   IP_NI_SYNOPSYS_NUM_RX_BUFFERS
  #define IP_NI_SYNOPSYS_NUM_RX_BUFFERS  (36)
#endif
#ifndef   IP_NI_SYNOPSYS_NUM_TX_BUFFERS
  #define IP_NI_SYNOPSYS_NUM_TX_BUFFERS  (4)    // Ideal configuration is one more than IP packet buffers used for sending.
#endif
#ifndef   IP_NI_SYNOPSYS_RX_BUFFER_SIZE
  #define IP_NI_SYNOPSYS_RX_BUFFER_SIZE  (128)  // 16 byte aligned to be compatible to any bus width and cache line aligned to avoid problems if the MCU uses cache.
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define IP_NI_XMC45XX_TX_BUFFER_SIZE  (1520)  // 16 byte aligned to be compatible to any bus width.

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  U32 MACBaseAddress[2];       // Base address of the Ethernet module(s) MAC registers.
  U32 MMCBaseAddress[2];       // Base address of the Ethernet module(s) MMC registers.
  U32 DMABaseAddress[2];       // Base address of the Ethernet module(s) DMA registers.
  //
  //  Clock and peripheral reset management.
  //
  U32 SetMACResetAddr;         // Address of the register to activate the mac reset.
  U32 ClrMACResetAddr;         // If another register is used to clear the MAC reset. Otherwise set 0.
  U32 MACResetMask[2];         // Bit mask to activate the MAC reset.
  U32 SetClockAddr;            // Address of the register to set the clock (RX, TX, PTP...).
  U32 ClrClockAddr;            // If another register is used to clear the clock. Otherwise set 0.
  U32 ClockMask[2];            // Bits to set the clocks.
  U32 SysCfgAddr;              // Address of the register to set RMII/MII support.
  U32 RMIIMask;                // Mask to set the support of RMII.
  //
  // Buffer and Ethernet MAC reset management.
  //
  U8  UseExtendedBufferDesc;   // Use of extended buffer descriptors (for IEEE 1588 PTP support or to use one complete cache line size per buffer).
  U8  EnableResetOnError;      // Reset the buffer descriptors when errors are detected.
  //
  // Other configuration parameters.
  //
  int DriverCaps;              // Available hardware capabilities.
  U32 MaccrMask;               // Additional bits for the MACCR register.
  U32 RxDescErrAddMask;        // Additional error bits for for the Rx buffer descriptor.
  U8  HasCache;                // Do we know that the MCU makes use of cache ?
  U8  Supports1GHz;            // Does the MCU have a Gigabit Ethernet MAC ?
  U8  SupportsRSF;             // Does the MAC support Receive-Store-Forward ?
} IP_NI_SYNOPSYS_CPU_CONFIG;

//
// Ethernet DMA descriptor size for regular usage (such as without
// PTP HW driver installed).
//
// This structure can be used to calculate how much memory has to
// be provided either for the stack memory pool or relocating the
// NI descriptors and buffers.
//
// If unsure if the regular descriptor applies to your configuration
// or the extended descriptor is used by the driver (for example if
// a PTP HW driver has been installed), it is safe to simply use
// IP_NI_SYNOPSYS_BUFFER_DESC_EXT for the calculation. The driver
// will decide upon init how much memory it needs from the provided
// memory pool and will simply leave the rest unprovisioned.
// This might waste some memory if you do not know for sure what mode
// is used by the driver but it is an easy and safe way.
//
typedef struct {
  U32 BufDesc0;
  U32 BufDesc1;
  U32 BufDesc2;
  U32 BufDesc3;
} IP_NI_SYNOPSYS_BUFFER_DESC;

//
// Ethernet DMA descriptor size for extended usage (such as with
// PTP HW driver installed).
//
// This structure can be used to calculate how much memory has to
// be provided either for the stack memory pool or relocating the
// NI descriptors and buffers.
//
typedef struct {
  U32 BufDesc0;
  U32 BufDesc1;
  U32 BufDesc2;
  U32 BufDesc3;
  U32 BufDesc4;
  U32 BufDesc5;
  U32 BufDesc6;
  U32 BufDesc7;
} IP_NI_SYNOPSYS_BUFFER_DESC_EXT;

//
// Compatibility typedefs for names that might already be in use in older configurations.
//
typedef IP_NI_SYNOPSYS_BUFFER_DESC IP_NI_LPC43XX_BUFFER_DESC;
typedef IP_NI_SYNOPSYS_BUFFER_DESC IP_NI_XMC45XX_BUFFER_DESC;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_STM32F1;
extern const IP_HW_DRIVER IP_Driver_STM32F2;
extern const IP_HW_DRIVER IP_Driver_STM32F7;
extern const IP_HW_DRIVER IP_Driver_XMC45xx;
extern const IP_HW_DRIVER IP_Driver_MB9Bx10T;
extern const IP_HW_DRIVER IP_Driver_TM4C;
extern const IP_HW_DRIVER IP_Driver_LPC43;
extern const IP_HW_DRIVER IP_Driver_GD32F450;

//
// ST STM32F1, STM32F2, STM32F4, STM32F7
//
#define IP_Driver_STM32F107                      IP_Driver_STM32F1
#define IP_Driver_STM32F207                      IP_Driver_STM32F2
#define IP_Driver_STM32F4                        IP_Driver_STM32F2

#define IP_NI_STM32F107_ConfigNumRxBuffers(n)    IP_NI_SYNOPSYS_ConfigNumRxBuffers(0, n)
#define IP_NI_STM32F107_ConfigNumTxBuffers(n)    IP_NI_SYNOPSYS_ConfigNumTxBuffers(0, n)
#define IP_NI_STM32F207_ConfigNumRxBuffers(n)    IP_NI_SYNOPSYS_ConfigNumRxBuffers(0, n)
#define IP_NI_STM32F207_ConfigNumTxBuffers(n)    IP_NI_SYNOPSYS_ConfigNumTxBuffers(0, n)

//
// Infineon XMC45xx
//
#define IP_NI_XMC45xx_ConfigNumRxBuffers(n)      IP_NI_SYNOPSYS_ConfigNumRxBuffers(0, n)
#define IP_NI_XMC45xx_ConfigNumTxBuffers(n)      IP_NI_SYNOPSYS_ConfigNumTxBuffers(0, n)

#define IP_NI_XMC45xx_ConfigRAMAddr(Addr)        IP_NI_SYNOPSYS_ConfigEthRamAddr(0, Addr)

//
// Fujitsu MB9Bx10T
//
#define IP_NI_MB9BX10T_ConfigNumRxBuffers(i, n)  IP_NI_SYNOPSYS_ConfigNumRxBuffers(i, n)

//
// TI TM4C, MSP432E
//
#define IP_Driver_MSP432E                        IP_Driver_TM4C

#define IP_NI_TM4C_ConfigNumRxBuffers(n)         IP_NI_SYNOPSYS_ConfigNumRxBuffers(0, n)

//
// NXP LPC18xx LPC43xx
//
#define IP_Driver_LPC43xx                        IP_Driver_LPC43
#define IP_Driver_LPC18xx                        IP_Driver_LPC43
#define IP_NI_LPC43xx_ConfigNumRxBuffers(n)      IP_NI_SYNOPSYS_ConfigNumRxBuffers(0, n)
#define IP_NI_LPC43xx_ConfigDriverMem(p, n)      IP_NI_SYNOPSYS_ConfigEthRamAddr(0, (U32)p)

//
// Generic functions
//
void IP_NI_SYNOPSYS_ConfigEthRamAddr          (unsigned IFaceId, U32 EthRamAddr);
void IP_NI_SYNOPSYS_ConfigMDIOClockRange      (U8 cr);
void IP_NI_SYNOPSYS_ConfigNumRxBuffers        (unsigned IFaceId, U16 NumRxBuffers);
void IP_NI_SYNOPSYS_ConfigNumTxBuffers        (unsigned IFaceId, U16 NumRxBuffers);
void IP_NI_SYNOPSYS_ConfigTransmitStoreForward(unsigned IFaceId, U8 OnOff);
void IP_NI_SYNOPSYS_ConfigUseTxCopy           (unsigned IFaceId, U8 OnOff, U16 TxBufferSize);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
