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
File    : IP_NI_ASIX_AX88180.h
Purpose : Driver specific header file for the ASIX AX88180
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef IP_NI_ASIX_AX88180_H
#define IP_NI_ASIX_AX88180_H

#if defined(__cplusplus)
  extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types/structures
*
**********************************************************************
*/

typedef struct {
  U32  (*pfReadReg)  (unsigned IFaceId, U16 RegAddr);
  void (*pfWriteReg) (unsigned IFaceId, U16 RegAddr, U32 Data);
  void (*pfReadData) (unsigned IFaceId, U8 *pData, unsigned NumBytesToRead, unsigned NumBytesToStore);
  void (*pfWriteData)(unsigned IFaceId, U8 *pData, unsigned NumBytes);
} IP_NI_ASIX_AX88180_ACCESS;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_ASIX_AX88180;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void IP_NI_ASIX_AX88180_ConfigAccess(const IP_NI_ASIX_AX88180_ACCESS *pAccess);
void IP_NI_ASIX_AX88180_ISR_Handler (unsigned IfaceId);

#if defined(__cplusplus)
  }    // Make sure we have C-declarations in C++ programs
#endif

/********************************************************************/

#endif // Avoid multiple inclusion

/*************************** End of file ****************************/



