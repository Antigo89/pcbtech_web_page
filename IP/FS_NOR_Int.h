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
File        : FS_NOR_Int.h
Purpose     : Private header file for the NOR flash driver
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_NOR_INT_H              // Avoid recursive and multiple inclusion
#define FS_NOR_INT_H

#include "SEGGER.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Public types (internal use)
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_NOR_PROGRAM_HW
*/
typedef struct {
  int (*pfRead)       (U8 Unit, void * pDest, unsigned SrcAddr, U32 NumBytes);
  int (*pfEraseSector)(U8 Unit, unsigned BaseAddr, unsigned SectorAddr);
  int (*pfWrite)      (U8 Unit, unsigned BaseAddr, unsigned SectorAddr, unsigned DestAddr, const U16 FS_NOR_FAR * pSrc, unsigned NumItems);
} FS_NOR_PROGRAM_HW;

/*********************************************************************
*
*       FS_NOR_TEST_HOOK_NOTIFICATION
*/
typedef void (FS_NOR_TEST_HOOK_NOTIFICATION)(U8 Unit);

/*********************************************************************
*
*       FS_NOR_TEST_HOOK_DATA_READ_BEGIN
*/
typedef void (FS_NOR_TEST_HOOK_DATA_READ_BEGIN)(U8 Unit, void * pData, U32 * pOff, U32 * pNumBytes);

/*********************************************************************
*
*       FS_NOR_TEST_HOOK_DATA_READ_END
*/
typedef void (FS_NOR_TEST_HOOK_DATA_READ_END)(U8 Unit, void * pData, U32 Off, U32 NumBytes, int * pResult);

/*********************************************************************
*
*       FS_NOR_TEST_HOOK_DATA_WRITE_BEGIN
*/
typedef void (FS_NOR_TEST_HOOK_DATA_WRITE_BEGIN)(U8 Unit, const void ** pData, U32 * pOff, U32 * pNumBytes);

/*********************************************************************
*
*       FS_NOR_TEST_HOOK_DATA_WRITE_END
*/
typedef void (FS_NOR_TEST_HOOK_DATA_WRITE_END)(U8 Unit, const void * pData, U32 Off, U32 NumBytes, int * pResult);

/*********************************************************************
*
*       FS_NOR_TEST_HOOK_SECTOR_ERASE
*/
typedef void (FS_NOR_TEST_HOOK_SECTOR_ERASE)(U8 Unit, U32 PhySectorIndex, int * pResult);

/*********************************************************************
*
*       FS_NOR_BM_PSH_INFO
*/
typedef struct {
  U8 NumBytes;
  U8 OffEraseCnt;
  U8 OffEraseSignature;
  U8 OffDataCnt;
} FS_NOR_BM_PSH_INFO;

/*********************************************************************
*
*       FS_NOR_BM_LSH_INFO
*/
typedef struct {
  U8 NumBytes;
} FS_NOR_BM_LSH_INFO;

/*********************************************************************
*
*       FS_NOR_LSH_INFO
*/
typedef struct {
  U8 NumBytes;
} FS_NOR_LSH_INFO;

/*********************************************************************
*
*       FS_NOR_PSH_INFO
*/
typedef struct {
  U8 NumBytes;
} FS_NOR_PSH_INFO;

/*********************************************************************
*
*       FS_NOR_SPI_POLL_PARA
*/
typedef struct {
  U32 TimeOut;              // Maximum polling time in number of request / response cycles.
  U32 Delay;                // Delay between two polling cycles in number of request / response cycles.
  U16 TimeOut_ms;           // Maximum polling time in milliseconds.
  U16 Delay_ms;             // Delay between two polling cycles in microseconds.
} FS_NOR_SPI_POLL_PARA;

/*********************************************************************
*
*       FS_NOR_SPI_CMD
*/
typedef struct {
  int (*pfControl)          (void * pContext, U8 Cmd, U16 BusWidth);
  int (*pfWriteData)        (void * pContext, U8 Cmd, const U8 * pData, unsigned NumBytes, U16 BusWidth);
  int (*pfReadData)         (void * pContext, U8 Cmd,       U8 * pData, unsigned NumBytes, U16 BusWidth);
  int (*pfWriteDataWithAddr)(void * pContext, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr, const U8 * pData, unsigned NumBytesData, U16 BusWidth);
  int (*pfReadDataWithAddr) (void * pContext, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr,       U8 * pData, unsigned NumBytesData, U16 BusWidth);
  int (*pfPoll)             (void * pContext, U8 Cmd, U8 BitPos, U8 BitValue, U32 Delay, U32 TimeOut_ms, U16 BusWith);
  int (*pfDelay)            (void * pContext, unsigned ms);
} FS_NOR_SPI_CMD;

/*********************************************************************
*
*       FS_NOR_SPI_SECTOR_BLOCK
*/
typedef struct {
  U8  ldBytesPerSector;         // Number of bytes in the physical sector
  U8  CmdErase;                 // Command to erase the physical sector
  U32 NumSectors;               // Total number of physical sectors
} FS_NOR_SPI_SECTOR_BLOCK;

/*********************************************************************
*
*       FS_NOR_SPI_INST
*/
typedef struct {
  FS_NOR_SPI_SECTOR_BLOCK   aSectorBlock[FS_NOR_MAX_SECTOR_BLOCKS];   // Stores information about the physical sectors.
  FS_NOR_SPI_POLL_PARA      PollParaRegWrite;       // Number of cycles to wait for a register write operation to complete
  const FS_NOR_SPI_CMD    * pCmd;                   // Pointer to functions to be used for the data transfer
  void                    * pContext;               // Pointer to a user defined data which is passed as first argument to all functions in pCmd
  U16                       BusWidthRead;           // Number of data lines to be used for the read operation
  U16                       BusWidthWrite;          // Number of data lines to be used for the write operation
  U8                        NumBytesAddr;           // Number of address bytes (4 bytes for capacity > 128MBit, else 3 bytes)
  U8                        Allow2bitMode;          // Set to 1 if data can be exchanged via 2 data lines (half-duplex)
  U8                        Allow4bitMode;          // Set to 1 if data can be exchanged via 4 data lines (half-duplex)
  U8                        CmdRead;                // Command to be used for reading the data.
  U8                        CmdWrite;               // Command to be used for writing the data.
  U8                        ReadModesDisabled;      // Bit-mask of the read modes disabled for a NOR flash device because of incorrect device parameters.
  U8                        NumBytesReadDummy;      // Number of dummy bytes to read after the address.
  U8                        NumSectorBlocks;        // Number of items stored in aSectorBlock
  U8                        ldBytesPerSector;       // Sector size configured by the application (as power of 2).
  U8                        IsDualDeviceMode;       // Set to 1 if two NOR flash devices are connected in parallel.
} FS_NOR_SPI_INST;

/*********************************************************************
*
*       FS_NOR_SPI_TYPE
*
*  Operations on serial NOR devices
*/
struct FS_NOR_SPI_TYPE {            //lint -esym(9058, FS_NOR_SPI_TYPE) tag unused outside of typedefs. Reason: the typedef is used as forward declaration.
  int  (*pfIdentify)                (      FS_NOR_SPI_INST * pInst, const U8 * pId);
  void (*pfInit)                    (const FS_NOR_SPI_INST * pInst);
  int  (*pfSetBusWidth)             (const FS_NOR_SPI_INST * pInst, unsigned BusWidth);    // BusWidth: 4/2/1
  int  (*pfSetNumBytesAddr)         (const FS_NOR_SPI_INST * pInst, unsigned NumBytes);
  int  (*pfReadApplyPara)           (      FS_NOR_SPI_INST * pInst);
  int  (*pfRemoveWriteProtection)   (const FS_NOR_SPI_INST * pInst, U32 StartAddr, U32 NumBytes);
  int  (*pfEraseSector)             (const FS_NOR_SPI_INST * pInst, U8 CmdErase, U32 SectorOff);
  int  (*pfWritePage)               (const FS_NOR_SPI_INST * pInst, U32 Addr, const U8 * pData, U32 NumBytes);
  int  (*pfWaitForEndOfOperation)   (const FS_NOR_SPI_INST * pInst, const FS_NOR_SPI_POLL_PARA * pPollPara);
};

/*********************************************************************
*
*       FS_NOR_SPI_DEVICE
*/
typedef struct {
  FS_NOR_SPI_INST         Inst;
  const FS_NOR_SPI_TYPE * pType;
} FS_NOR_SPI_DEVICE;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

/*********************************************************************
*
*       HW layers for CFI compliant NOR devices
*/
extern const FS_NOR_PROGRAM_HW FS_NOR_Program_Intel_1x16;
extern const FS_NOR_PROGRAM_HW FS_NOR_Program_AMD_1x16;
extern const FS_NOR_PROGRAM_HW FS_NOR_Program_IntelFast_1x16;
extern const FS_NOR_PROGRAM_HW FS_NOR_Program_AMDFast_1x16;
extern const FS_NOR_PROGRAM_HW FS_NOR_Program_Intel_2x16;
extern const FS_NOR_PROGRAM_HW FS_NOR_Program_AMD_2x16;
extern const FS_NOR_PROGRAM_HW FS_NOR_Program_AMDFast_2x16;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Functions to read the CFI information from the NOR flash device.
*/
void FS_NOR_CFI_ReadCFI_1x16(U8 Unit, U32 BaseAddr, U32 Off, U8 * pData, unsigned NumItems);
void FS_NOR_CFI_ReadCFI_2x16(U8 Unit, U32 BaseAddr, U32 Off, U8 * pData, unsigned NumItems);

/*********************************************************************
*
*       Functions to register a test hook.
*/
void FS__NOR_SetTestHookFailSafe         (FS_NOR_TEST_HOOK_NOTIFICATION     * pfTestHook);
void FS__NOR_SetTestHookDataReadBegin    (FS_NOR_TEST_HOOK_DATA_READ_BEGIN  * pfTestHook);
void FS__NOR_SetTestHookDataReadEnd      (FS_NOR_TEST_HOOK_DATA_READ_END    * pfTestHook);
void FS__NOR_SetTestHookDataWriteBegin   (FS_NOR_TEST_HOOK_DATA_WRITE_BEGIN * pfTestHook);
void FS__NOR_SetTestHookDataWriteEnd     (FS_NOR_TEST_HOOK_DATA_WRITE_END   * pfTestHook);
void FS__NOR_SetTestHookSectorErase      (FS_NOR_TEST_HOOK_SECTOR_ERASE     * pfTestHook);
void FS__NOR_BM_SetTestHookFailSafe      (FS_NOR_TEST_HOOK_NOTIFICATION     * pfTestHook);
void FS__NOR_BM_SetTestHookDataReadBegin (FS_NOR_TEST_HOOK_DATA_READ_BEGIN  * pfTestHook);
void FS__NOR_BM_SetTestHookDataReadEnd   (FS_NOR_TEST_HOOK_DATA_READ_END    * pfTestHook);
void FS__NOR_BM_SetTestHookDataWriteBegin(FS_NOR_TEST_HOOK_DATA_WRITE_BEGIN * pfTestHook);  //lint -esym(621, FS__NOR_BM_SetTestHookDataWriteBegin) Identifier clash. Rationale: this symbol is used only in test builds.
void FS__NOR_BM_SetTestHookDataWriteEnd  (FS_NOR_TEST_HOOK_DATA_WRITE_END   * pfTestHook);  //lint -esym(621, FS__NOR_BM_SetTestHookDataWriteEnd) Identifier clash. Rationale: this symbol is used only in test builds.
void FS__NOR_BM_SetTestHookSectorErase   (FS_NOR_TEST_HOOK_SECTOR_ERASE     * pfTestHook);
void FS__NOR_PHY_SFDP_SetTestHookFailSafe(FS_NOR_TEST_HOOK_NOTIFICATION     * pfTestHook);

/*********************************************************************
*
*       Functions that return information about the driver.
*/
void                    FS__NOR_GetLSHInfo                 (U8 Unit, FS_NOR_LSH_INFO * pLSHInfo);
void                    FS__NOR_GetPSHInfo                 (U8 Unit, FS_NOR_PSH_INFO * pPSHInfo);
U8                      FS__NOR_IsRewriteSupported         (U8 Unit);
void                    FS__NOR_BM_GetPSHInfo              (U8 Unit, FS_NOR_BM_PSH_INFO * pPSHInfo);
void                    FS__NOR_BM_GetLSHInfo              (U8 Unit, FS_NOR_BM_LSH_INFO * pLSHInfo);
U8                      FS__NOR_BM_IsRewriteSupported      (U8 Unit);
U8                      FS__NOR_BM_IsFailSafeEraseSupported(U8 Unit);
const FS_NOR_PHY_TYPE * FS__NOR_BM_GetPhyType              (U8 Unit);

/*********************************************************************
*
*       Functions that operate on a physical layer.
*/
int FS__NOR_SPIFI_ReadData(U8 Unit, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr, U8 * pData, unsigned NumBytesData);

/*********************************************************************
*
*       Functions specific to serial NOR flash devices.
*/
void FS_NOR_SPI_ReadId           (const FS_NOR_SPI_INST * pInst, U8 * pData, unsigned NumBytes);
int  FS_NOR_SPI_ReadApplyParaById(      FS_NOR_SPI_INST * pInst);
U32  FS_NOR_SPI_GetSectorOff     (const FS_NOR_SPI_INST * pInst, U32 SectorIndex);
U32  FS_NOR_SPI_GetSectorSize    (const FS_NOR_SPI_INST * pInst, U32 SectorIndex);
U8   FS_NOR_SPI_GetSectorEraseCmd(const FS_NOR_SPI_INST * pInst, U32 SectorIndex);

/*********************************************************************
*
*       Functions to validate the implementation.
*/
int  FS_NOR_ECC_SW_1Bit_Validate(void);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // NOR_PRIVATE_H

/*************************** End of file ****************************/
