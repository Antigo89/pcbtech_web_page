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
File    : IP_NI_MICREL_KSZ8851SNL.h
Purpose : Driver specific header file for the external MICREL
          KSZ8851SNL EMAC+PHY.
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef IP_NI_MICREL_KSZ8851SNL_H
#define IP_NI_MICREL_KSZ8851SNL_H

#include "IP.h"

#if defined(__cplusplus)
extern "C" {  // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       Defines, non configurable
*
**********************************************************************
*/

#define IP_MICREL_KSZ8851SNL_TX_INT_DISABLE_TIMEOUT  0xFFFFFFFFuL

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct IP_NI_MICREL_KSZ8851SNL_ACCESS  IP_NI_MICREL_KSZ8851SNL_ACCESS;
typedef struct MICREL_KSZ8851SNL_INST          MICREL_KSZ8851SNL_INST;

struct MICREL_KSZ8851SNL_INST {
  const IP_NI_MICREL_KSZ8851SNL_ACCESS* pAccess;
  const IP_PHY_DRIVER*                  PHY_pDriver;
        IP_PHY_CONTEXT                  PHY_Context;
        U32                             TxIntTimeout;
        unsigned                        Unit;
        int                             LockCnt;
        U16                             RegIer;
        U16                             RxNumBytesPending;  // DMA handling: A Rx interrupt has triggered a DMA read and NumBytes of data is ready to be copied from driver to stack.
        char                            TxIsBusy;
        char                            TxIsPending;        // DMA handling: A Tx interrupt has been fired but has been set pending until all Rx work has been done.
        char                            DoRxLateUnlock;     // DMA handling: An Rx unlock can be done but not from the place where we are now. Unlocking is done after IP_ETH_OnRx()/IP_OnRx() is done.
        U8                              RxFrameCnt;
};

struct IP_NI_MICREL_KSZ8851SNL_ACCESS {
  U16  (*pfReadMacReg)  (unsigned IFaceId, U8 RegIndex);
  void (*pfWriteMacReg) (unsigned IFaceId, U8 RegIndex, U16 Data);
  U16  (*pfReadPhyReg)  (unsigned IFaceId, U8 RegIndex);
  void (*pfWritePhyReg) (unsigned IFaceId, U8 RegIndex, U16 Data);
  void (*pfReadData)    (unsigned IFaceId, U8* pData, U16 NumBytes);
  void (*pfWriteData)   (unsigned IFaceId, U8* pData, U16 NumBytes);
  void (*pfReadDataDMA) (unsigned IFaceId, U8* pData, U16 NumBytes);
  void (*pfWriteDataDMA)(unsigned IFaceId, U8* pData, U16 NumBytes);
};

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_MICREL_KSZ8851SNL;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void IP_NI_MICREL_KSZ8851SNL_ConfigAccess    (unsigned IFaceId, const IP_NI_MICREL_KSZ8851SNL_ACCESS* pAccess);
int  IP_NI_MICREL_KSZ8851SNL_ISR_Handler     (unsigned IFaceId);
void IP_NI_MICREL_KSZ8851SNL_DMA_OnRxComplete(unsigned IFaceId);
void IP_NI_MICREL_KSZ8851SNL_DMA_OnTxComplete(unsigned IFaceId);


#if defined(__cplusplus)
  }           // Make sure we have C-declarations in C++ programs
#endif

/********************************************************************/

#endif        // Avoid multiple inclusion

/*************************** End of file ****************************/



