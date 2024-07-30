/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2015 - 2018  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       emCompress-Embed * Compression library                       *
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
*       emCompress-Embed version: V2.14                              *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_CRC.h
Purpose : Header file for the emLib CRC API.
Revision: $Rev: 6050 $
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef SEGGER_CRC_H  // Avoid multiple inclusion
#define SEGGER_CRC_H

#include "Global.h"

#define LIB_VERSION      10000 // Format: Mmmrr. Example: 21201 is 2.12a

#if defined(__cplusplus)
extern "C" {   // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       Definitions
*
**********************************************************************
*/

/*********************************************************************
*
*       Validation return values
*/
#define SEGGER_CRC_VALIDATE_SUCCESS               (0)
#define SEGGER_CRC_VALIDATE_ERROR_ARBITRARY     (-10)
#define SEGGER_CRC_VALIDATE_ERROR_SPECIFIC      (-11)
#define SEGGER_CRC_VALIDATE_ERROR_BITWISE       (-12)
#define SEGGER_CRC_VALIDATE_ERROR_ARBITRARY_MSB (-20)
#define SEGGER_CRC_VALIDATE_ERROR_SPECIFIC_MSB  (-21)
#define SEGGER_CRC_VALIDATE_ERROR_BITWISE_MSB   (-22)

/*********************************************************************
*
*       Validation results
*/
#define SEGGER_CRC_RESULT_KERMIT (0x2189)  // According to CRC-16-CCITT (aka "Kermit"), using the reflected 16-bit polynomial 0x1021 must result in 0x2189.
#define SEGGER_CRC_RESULT_ARC    (0xBB3D)  // According to CRC-16-IBM (aka "ARC"), using the reflected 16-bit polynomial 0x8005 must result in 0xBB3D.
#define SEGGER_CRC_RESULT_DARC     (0x15)  // According to CRC-8-DARC, using the reflected 8-bit polynomial 0x39 must result in 0x15.
#define SEGGER_CRC_RESULT_ITU6     (0x06)  // According to CRC-6-ITU, using the reflected 6-bit polynomial 0x03 must result in 0x06.
#define SEGGER_CRC_RESULT_ITU5     (0x07)  // According to CRC-5-ITU, using the reflected 5-bit polynomial 0x15 must result in 0x07.
#define SEGGER_CRC_RESULT_ITU4      (0x7)  // According to CRC-4-ITU, using the reflected 4-bit polynomial 0x3 must result in 0x7.

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

/*********************************************************************
*
*       CRC API functions for arbitrary polynomials, bit-by-bit
*/
U32 SEGGER_CRC_CalcBitByBit    (const U8 *pData, U32 NumBytes, U32 Crc, U32 Poly);
U32 SEGGER_CRC_CalcBitByBit_MSB(const U8 *pData, U32 NumBytes, U32 Crc, U32 Poly, U8 SizeOfPoly);

/*********************************************************************
*
*       CRC API functions for arbitrary polynomials, table-driven
*/
U32 SEGGER_CRC_Calc            (const U8 *pData, U32 NumBytes, U32 Crc, U32 Poly);
U32 SEGGER_CRC_Calc_MSB        (const U8 *pData, U32 NumBytes, U32 Crc, U32 Poly, U8 SizeOfPoly);

/*********************************************************************
*
*       CRC API functions for specified polynomials, table-driven
*/
U8  SEGGER_CRC_Calc_09         (const U8 *pData, U32 NumBytes, U8  Crc);
U8  SEGGER_CRC_Calc_48         (const U8 *pData, U32 NumBytes, U8  Crc);
U16 SEGGER_CRC_Calc_1021       (const U8 *pData, U32 NumBytes, U16 Crc);
U16 SEGGER_CRC_Calc_8408       (const U8 *pData, U32 NumBytes, U16 Crc);
U32 SEGGER_CRC_Calc_04C11DB7   (const U8 *pData, U32 NumBytes, U32 Crc);
U32 SEGGER_CRC_Calc_EDB88320   (const U8 *pData, U32 NumBytes, U32 Crc);
U32 SEGGER_CRC_Calc_1EDC6F41   (const U8 *pData, U32 NumBytes, U32 Crc);
U32 SEGGER_CRC_Calc_82F63B78   (const U8 *pData, U32 NumBytes, U32 Crc);

/*********************************************************************
*
*       CRC validation API functions
*/
I8 SEGGER_CRC_Validate         (void);

#if defined(__cplusplus)
}       // Make sure we have C-declarations in C++ programs 
#endif

#endif  // Avoid multiple inclusion 

/****** End Of File *************************************************/
