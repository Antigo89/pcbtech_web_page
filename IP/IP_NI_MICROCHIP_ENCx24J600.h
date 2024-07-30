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
File    : IP_NI_ENCx24J600.h
Purpose : Driver specific header file for the Microchip ENCx24J600
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef IP_NI_ENCX24J600_H
#define IP_NI_ENCX24J600_H

#if defined(__cplusplus)
extern "C" {  // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
extern const IP_HW_DRIVER IP_Driver_ENCx24J600;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
typedef struct IP_NI_ENCX24J600_ACCESS  IP_NI_ENCX24J600_ACCESS;
typedef struct ENCX24J600_INST ENCX24J600_INST;

struct ENCX24J600_INST {
  IP_NI_ENCX24J600_ACCESS *pAccess;
  U8                      *pHardware;
  U8                      *pTxStart;
  U8                      *pRxStart;
  U8                      *pNextPacket;
  unsigned                 Unit;
  int                      InterruptLockCnt;
  U8                       BusWidth;
  U8                       UseSPI;
  U8                       TxIsBusy;
  U8                       IsInited;
};

struct IP_NI_ENCX24J600_ACCESS {
  void (*pfWriteReg)    (ENCX24J600_INST * pInst, unsigned RegIndex, unsigned val, U8 Opcode);
  U16  (*pfReadReg)     (ENCX24J600_INST * pInst, unsigned RegIndex);
  void (*pfReadData)    (ENCX24J600_INST * pInst, U8*   pDest, U32 NumBytes);
  void (*pfWriteData)   (ENCX24J600_INST * pInst, void* pPacket, U32 NumBytes);
};

void IP_NI_ENCx24J600_ISR_Handler (unsigned Unit);
void IP_NI_ENCx24J600_ConfigAddr  (unsigned Unit, void* pBase);
void IP_NI_ENCx24J600_ConfigAccess(unsigned Unit, IP_NI_ENCX24J600_ACCESS * pAccess);
void IP_NI_ENCx24J600_ConfigUseSPI(unsigned Unit, char OnOff);

#if defined(__cplusplus)
  }           // Make sure we have C-declarations in C++ programs
#endif

/********************************************************************/

#endif        // Avoid multiple inclusion

/*************************** End of file ****************************/



