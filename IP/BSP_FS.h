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
File        : BSP_FS.h
Purpose     : BSP (Board support package) for file system.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef BSP_FS_H      // Avoid multiple/recursive inclusion.
#define BSP_FS_H

#include "SEGGER.h"

#if defined(__cplusplus)
extern "C" {          // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       BSP_FS_GetTransferMem
*
*  Function description
*    Delivers a memory area that can be used for data transfer.
*
*  Parameters
*    pPAddr     [OUT] Physical address of the allocated buffer.
*    pVAddr     [OUT] Virtual address of the allocated buffer.
*
*  Return value
*    > 0    Number of bytes allocated.
*    ==0    An error occurred.
*
*  Additional information
*    This function is typically used the the hardware layers for the SD/MMC driver
*    that transfer data via DMA. It delivers the physical address and the virtual
*    address of the transfer memory. The transfer area needs to be word aligned
*    and located at a memory location that is not cached and for which the write
*    buffering is disabled.
*    Additional requirements are that the memory used is fast enough to not block
*    DMA transfers for too long. In most systems, the internal RAM has to be used
*    instead of external SDRAM, since the SDRAM can have relatively long latencies,
*    primarily due to refresh cycles. The size of the memory is also returned.
*    It needs to be at least 512 bytes. In general, bigger values allow higher
*    performance since it allows transfer of multiple sectors without break.
*/
U32 BSP_FS_GetTransferMem       (U32 * pPAddr, U32 * pVAddr);

/*********************************************************************
*
*       BSP_FS_CleanCacheRange
*
*  Function description
*    Writes modified cache data to memory.
*
*  Parameters
*    p          Start address of the memory region to be cleaned.
*    NumBytes   Number of bytes in the memory region to be cleaned.
*
*  Additional information
*    This function can be used to make sure that changes made to
*    cache memory are actually stored to main memory.
*/
void BSP_FS_CleanCacheRange     (void * p, unsigned NumBytes);

/*********************************************************************
*
*       BSP_FS_InvalidateCacheRange
*
*  Function description
*    Marks cached data as invalid.
*
*  Parameters
*    p          Start address of the memory region to be invalidated.
*    NumBytes   Number of bytes in the memory region to be invalidated.
*
*  Additional information
*    This function can be used to make sure that changes made to the main memory
*    (for example via DMA) are loaded to cache memory.
*/
void BSP_FS_InvalidateCacheRange(void * p, unsigned NumBytes);

/*********************************************************************
*
*       BSP_FS_InstallISR
*
*  Function description
*    Registers and enables an interrupt handler.
*
*  Parameters
*    ISRIndex   Identifies the interrupt.
*    pfISR      Interrupt handling function.
*    Prio       Interrupt priority.
*/
void BSP_FS_InstallISR          (int ISRIndex, void (*pfISR)(void), int Prio);

/*********************************************************************
*
*       BSP_FS_v2p
*
*  Function description
*    Convert virtual to physical address.
*
*  Parameters
*    pVAddr     Virtual address to be converted.
*
*  Return value
*    Physical address where the virtual address is mapped.
*/
U32 BSP_FS_v2p                  (void * pVAddr);

/*********************************************************************
*
*       Compatibility defines
*/
#define BSP_SD_GetTransferMem       BSP_FS_GetTransferMem
#define BSP_CACHE_CleanRange        BSP_FS_CleanCacheRange
#define BSP_CACHE_InvalidateRange   BSP_FS_InvalidateCacheRange

#if defined(__cplusplus)
  }                   // Make sure we have C-declarations in C++ programs
#endif

#endif                // Avoid multiple/recursive inclusion

/*************************** End of file ****************************/
