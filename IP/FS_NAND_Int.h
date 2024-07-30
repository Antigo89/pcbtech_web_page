/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2022  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support_emfile@segger.com        *
*                                                                    *
**********************************************************************
*                                                                    *
*       emFile * File system for embedded applications               *
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
*       emFile version: V5.16.0                                      *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File        : FS_NAND_Int.h
Purpose     : Private header file for the NAND flash driver.
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_NAND_INT_H             // Avoid recursive and multiple inclusion
#define FS_NAND_INT_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Public types
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_NOTIFICATION
*/
typedef void (FS_NAND_TEST_HOOK_NOTIFICATION)(U8 Unit);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_DATA_READ_BEGIN
*/
typedef void (FS_NAND_TEST_HOOK_DATA_READ_BEGIN)(U8 Unit, U32 PageIndex, void * pData, unsigned * pOff, unsigned * pNumBytes);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_DATA_READ_END
*/
typedef void (FS_NAND_TEST_HOOK_DATA_READ_END)(U8 Unit, U32 PageIndex, void * pData, unsigned Off, unsigned NumBytes, int * pResult);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_DATA_READ_EX_BEGIN
*/
typedef void (FS_NAND_TEST_HOOK_DATA_READ_EX_BEGIN)(U8 Unit, U32 PageIndex, void * pData, unsigned * pOff, unsigned * pNumBytes, void * pSpare, unsigned * OffSpare, unsigned * NumBytesSpare);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_DATA_READ_EX_END
*/
typedef void (FS_NAND_TEST_HOOK_DATA_READ_EX_END)(U8 Unit, U32 PageIndex, void * pData, unsigned Off, unsigned NumBytes, void * pSpare, unsigned OffSpare, unsigned NumBytesSpare, int * pResult);    //lint !e621 Identifier clash. This symbol is used only in test builds.

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_DATA_WRITE_EX_BEGIN
*/
typedef void (FS_NAND_TEST_HOOK_DATA_WRITE_EX_BEGIN)(U8 Unit, U32 PageIndex, const void ** ppData, unsigned * pOff, unsigned * pNumBytes, const void ** pSpare, unsigned * pOffSpare, unsigned * pNumBytesSpare);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_DATA_WRITE_EX_END
*/
typedef void (FS_NAND_TEST_HOOK_DATA_WRITE_EX_END)(U8 Unit, U32 PageIndex, const void * pData, unsigned Off, unsigned NumBytes, const void * pSpare, unsigned OffSpare, unsigned NumBytesSpare, int * pResult);     //lint !e621 Identifier clash. This symbol is used only in test builds.

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_BLOCK_ERASE
*/
typedef void (FS_NAND_TEST_HOOK_BLOCK_ERASE)(U8 Unit, U32 PageIndex, int * pResult);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_READ_BEGIN
*/
typedef void (FS_NAND_TEST_HOOK_READ_BEGIN)(U8 Unit, void * pData, unsigned * pNumBytes);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_READ_END
*/
typedef void (FS_NAND_TEST_HOOK_READ_END)(U8 Unit, void * pData, unsigned NumBytes, int * pResult);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_WRITE_BEGIN
*/
typedef void (FS_NAND_TEST_HOOK_WRITE_BEGIN)(U8 Unit, const void ** ppData, unsigned * pNumBytes);

/*********************************************************************
*
*       FS_NAND_TEST_HOOK_WRITE_END
*/
typedef void (FS_NAND_TEST_HOOK_WRITE_END)(U8 Unit, const void * pData, unsigned NumBytes, int * pResult);

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Functions to access ONFI parameters.
*/
int  FS__NAND_IsONFISupported    (U8 Unit, const FS_NAND_HW_TYPE * pHWType);
int  FS__NAND_ReadONFIPara       (U8 Unit, const FS_NAND_HW_TYPE * pHWType, void * pPara);
int  FS__NAND_SPI_ReadONFIPara   (U8 Unit, void * pPara);
int  FS__NAND_ONFI_ReadONFIPara  (U8 Unit, void * pPara);

/*********************************************************************
*
*       Functions to register a test hook.
*/
void FS__NAND_SetTestHookFailSafe            (FS_NAND_TEST_HOOK_NOTIFICATION        * pfTestHook);
void FS__NAND_UNI_SetTestHookFailSafe        (FS_NAND_TEST_HOOK_NOTIFICATION        * pfTestHook);
void FS__NAND_UNI_SetTestHookDataReadBegin   (FS_NAND_TEST_HOOK_DATA_READ_BEGIN     * pfTestHook);    //lint -esym(621, FS__NAND_UNI_SetTestHookDataReadBegin) Identifier clash. Rationale: This symbol is used only in test builds.
void FS__NAND_UNI_SetTestHookDataReadEnd     (FS_NAND_TEST_HOOK_DATA_READ_END       * pfTestHook);    //lint -esym(621, FS__NAND_UNI_SetTestHookDataReadEnd) Identifier clash. Rationale: This symbol is used only in test builds.
void FS__NAND_UNI_SetTestHookDataReadExBegin (FS_NAND_TEST_HOOK_DATA_READ_EX_BEGIN  * pfTestHook);    //lint -esym(621, FS__NAND_UNI_SetTestHookDataReadExBegin) Identifier clash. Rationale: This symbol is used only in test builds.
void FS__NAND_UNI_SetTestHookDataReadExEnd   (FS_NAND_TEST_HOOK_DATA_READ_EX_END    * pfTestHook);    //lint -esym(621, FS__NAND_UNI_SetTestHookDataReadExEnd) Identifier clash. Rationale: This symbol is used only in test builds.
void FS__NAND_UNI_SetTestHookDataWriteExBegin(FS_NAND_TEST_HOOK_DATA_WRITE_EX_BEGIN * pfTestHook);    //lint -esym(621, FS__NAND_UNI_SetTestHookDataWriteExBegin) Identifier clash. Rationale: This symbol is used only in test builds.
void FS__NAND_UNI_SetTestHookDataWriteExEnd  (FS_NAND_TEST_HOOK_DATA_WRITE_EX_END   * pfTestHook);    //lint -esym(621, FS__NAND_UNI_SetTestHookDataWriteExEnd) Identifier clash. Rationale: This symbol is used only in test builds.
void FS__NAND_UNI_SetTestHookBlockErase      (FS_NAND_TEST_HOOK_BLOCK_ERASE         * pfTestHook);
void FS__NAND_SPI_SetTestHookReadBegin       (FS_NAND_TEST_HOOK_READ_BEGIN          * pfTestHook);
void FS__NAND_SPI_SetTestHookReadEnd         (FS_NAND_TEST_HOOK_READ_END            * pfTestHook);
void FS__NAND_SPI_SetTestHookWriteBegin      (FS_NAND_TEST_HOOK_WRITE_BEGIN         * pfTestHook);
void FS__NAND_SPI_SetTestHookWriteEnd        (FS_NAND_TEST_HOOK_WRITE_END           * pfTestHook);

/*********************************************************************
*
*       Functions that return information about the driver.
*/
const FS_NAND_PHY_TYPE * FS__NAND_GetPhyType(U8 Unit);
const FS_NAND_PHY_TYPE * FS__NAND_UNI_GetPhyType(U8 Unit);
int                      FS__NAND_UNI_MarkBlockAsBad(U8 Unit, unsigned BlockIndex);

/*********************************************************************
*
*       Functions used for testing.
*/
int  FS__NAND_SPI_SetCompatibilityMode(U8 Unit, U8 Mode);
int  FS__NAND_SPI_EnableECC           (U8 Unit);
int  FS__NAND_SPI_DisableECC          (U8 Unit);
int  FS__NAND_SPI_ReadId              (U8 Unit, U8 * pData, unsigned NumBytes);

/*********************************************************************
*
*       Functions to validate the implementation.
*/
int  FS_NAND_Validate            (void);
int  FS_NAND_ECC_SW_1Bit_Validate(void);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // NAND_PRIVATE_H

/*************************** End of file ****************************/
