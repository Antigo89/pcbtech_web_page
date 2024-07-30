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

File    : IP_NI_ETHERC.h
Purpose : Driver specific header file for Renesas ETHERC Ethernet
          controllers (RX, Synergy).
*/

#ifndef IP_NI_ETHERC_H      // Avoid multiple inclusion.
#define IP_NI_ETHERC_H

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
// Generic buffer configuration.
// Can be used to calculate driver internal memory requirements.
// Default values used if not set via configuration functions.
//
#ifndef   IP_NI_ETHERC_NUM_RX_BUFFERS
  #define IP_NI_ETHERC_NUM_RX_BUFFERS  (37u)
#endif
#ifndef   IP_NI_ETHERC_RX_BUFFER_SIZE
  #define IP_NI_ETHERC_RX_BUFFER_SIZE  (128u)
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define IP_Driver_RX65N IP_Driver_RX64M
#define IP_Driver_RX71M IP_Driver_RX64M

#define IP_NI_RX_ConfigNumRxBuffers IP_NI_ETHERC_ConfigNumRxBuffers

#define IP_NI_ETHERC_NUM_TX_BUFFERS  (1u)

//
// The driver aligns its memory pool used for descriptors to 32 bytes internally.
// When using IP_NI_ETHERC_ConfigEthRamAddr() the memory pool given either needs
// to be already aligned or provide IP_NI_ETHERC_NUM_BYTES_ALIGNMENT additional
// bytes for the driver to be able to align on its own.
//
#define IP_NI_ETHERC_NUM_BYTES_ALIGNMENT  (31u)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  U32 MACBaseAddress[2];  // Base address of the Ethernet module(s) MAC registers.
  U32 PRCRAddress;        // Protection register.
  U32 PFENETAddress;      // ENET configuration register.
  U32 MSTPCRBAddress;     // Address of the module stop control register B.
  U32 IPRAddress;         // RX interrupt IPR register address.
  U32 IERAddress;         // RX interrupt IER address.
  U32 GENAddress;         // Group Interrupt Request Enable Register
  U16 PTPBaseOffset;      // Base address offset of the PTP registers to the MAC0 register base addresses.
  U8  MSTPCRBBit[2];      // Bit to be set for the MSTPCRB register.
  U8  GENBit[2];          // Bit to set in the group interrupt request enable register.
  U8  IERBit;             // Bit to set for the interrupt setting in IER register.
  U8  ResetOnRRF;         // Perform a reset when a RRF (alignment error) is detected.
} IP_NI_ETHERC_CPU_CONFIG;

//
// Ethernet DMA descriptor size.
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
} IP_NI_ETHERC_BUFFER_DESC;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_RX62N;
extern const IP_HW_DRIVER IP_Driver_RX63N;
extern const IP_HW_DRIVER IP_Driver_RX64M;
extern const IP_HW_DRIVER IP_Driver_S7G2;
extern const IP_HW_DRIVER IP_Driver_RA6M2;
extern const IP_HW_DRIVER IP_Driver_RA6M4;

void IP_NI_ETHERC_ConfigNumRxBuffers(unsigned IFaceId, U16 NumRxBuffers);
void IP_NI_ETHERC_ConfigSkipIntSetup(unsigned IFaceId, U8 OnOff);
void IP_NI_ETHERC_ISRHandler        (unsigned IFaceId);
void IP_NI_ETHERC_ConfigEthRamAddr  (unsigned IFaceId, U32 EthRamAddr);
void IP_NI_ETHERC_ConfigUseTxCopy   (unsigned IFaceId, U8 OnOff, U16 TxBufferSize);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
