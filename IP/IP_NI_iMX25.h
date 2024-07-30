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
File        : IP_NI_iMX25.h
Purpose     : Driver specific header file for Freescale i.MX25
---------------------------END-OF-HEADER------------------------------
*/

#ifndef IP_DRIVER_IMX25_H  // Avoid multiple inclusion
#define IP_DRIVER_IMX25_H

#if defined(__cplusplus)
extern "C" {  // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       Defines, non configurable
*
**********************************************************************
*/

#define IP_NI_IMX25_TX_BUFFER_SIZE  1536  // Buffer size MUST be divisible by 16 and less than 2047 bytes, but big enough to hold entire frame (MTU 1500 bytes + headers)

/*********************************************************************
*
*       Types, descriptors
*
**********************************************************************
*/

typedef struct {
  union {
    U32 Ctrl;
    struct {
      U32 DataLength  : 16;
      U32 Reserved0   :  9;
      U32 ABC         :  1;
      U32 TC          :  1;
      U32 L           :  1;
      U32 TO2         :  1;
      U32 W           :  1;
      U32 TO1         :  1;
      U32 R           :  1;
    } CtrlBits;
  } Ctrl;
  U32 Addr;
} IP_NI_IMX25_TX_BUFFER_DESC;

typedef struct {
  union {
    U32 Ctrl;
    struct {
      U32 DataLength  : 16;
      U32 TR          :  1;
      U32 OV          :  1;
      U32 CR          :  1;
      U32 Reserved0   :  1;
      U32 NO          :  1;
      U32 LG          :  1;
      U32 MC          :  1;
      U32 BC          :  1;
      U32 M           :  1;
      U32 Reserved1   :  2;
      U32 L           :  1;
      U32 RO2         :  1;
      U32 W           :  1;
      U32 RO1         :  1;
      U32 E           :  1;
    } CtrlBits;
  } Ctrl;
  U32 Addr;
} IP_NI_IMX25_RX_BUFFER_DESC;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

extern const IP_HW_DRIVER IP_Driver_iMX25;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void IP_NI_iMX25_ConfigBuffers(U16 NumTxBuffers, U16 NumRxBuffers, U16 RxBufferSize, void * pTxDesc, void * pRxDesc, void * pBuffer);

#if defined(__cplusplus)
  }           // Make sure we have C-declarations in C++ programs
#endif

/********************************************************************/

#endif        // Avoid multiple inclusion

/*************************** End of file ****************************/
