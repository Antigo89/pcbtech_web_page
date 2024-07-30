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
File        : FS_FAT.h
Purpose     : FAT File System Layer header
-------------------------- END-OF-HEADER -----------------------------
*/
#ifndef FS_FAT_H            // Avoid recursive and multiple inclusion
#define FS_FAT_H

#include "FS_Int.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       FAT_DiskInfo
*/
int  FS_FAT_GetDiskInfo         (FS_VOLUME * pVolume, FS_DISK_INFO * pDiskInfo, int Flags);
int  FS_FAT_GetVolumeLabel      (FS_VOLUME * pVolume, char * pVolumeLabel, unsigned VolumeLabelSize);
int  FS_FAT_SetVolumeLabel      (FS_VOLUME * pVolume, const char * pVolumeLabel);

/*********************************************************************
*
*       FAT_Read
*/
U32  FS_FAT_Read                (FS_FILE * pFile, void * pData, U32 NumBytesReq);

/*********************************************************************
*
*       FAT_Write
*/
U32  FS_FAT_Write               (FS_FILE   * pFile, const void * pData, U32 NumBytes);
int  FS_FAT_CloseFile           (FS_FILE   * pFile);
void FS_FAT_Clean               (FS_VOLUME * pVolume);

/*********************************************************************
*
*       FAT_Open
*/
int  FS_FAT_OpenFile            (const char * sFileName, FS_FILE * pFile, int DoDel, int DoOpen, int DoCreate);

/*********************************************************************
*
*       FAT_Misc
*/
int  FS_FAT_CheckBPB            (      FS_VOLUME * pVolume);
int  FS_FAT_CreateJournalFile   (      FS_VOLUME * pVolume, U32 NumBytes, U32 * pFirstSector, U32 * pNumSectors);
int  FS_FAT_OpenJournalFile     (      FS_VOLUME * pVolume);
U32  FS_FAT_GetIndexOfLastSector(      FS_VOLUME * pVolume);
int  FS_FAT_FreeSectors         (      FS_VOLUME * pVolume);
int  FS_FAT_GetFreeSpace        (      FS_VOLUME * pVolume, void * pBuffer, int SizeOfBuffer, U32 FirstClusterId, U32 * pNumClustersFree, U32 * pNumClustersChecked);
int  FS_FAT_GetATInfo           (      FS_VOLUME * pVolume, FS_AT_INFO * pATInfo);
I32  FS_FAT_ReadATEntry         (      FS_VOLUME * pVolume, U32 ClusterId);
void FS_FAT_Save                (      FS_CONTEXT * pContext);
void FS_FAT_Restore             (const FS_CONTEXT * pContext);

/*********************************************************************
*
*       FAT_Format
*/
int  FS_FAT_Format              (      FS_VOLUME * pVolume, const FS_FORMAT_INFO    * pFormatInfo);

/*********************************************************************
*
*       FAT_Move
*/
int  FS_FAT_Move                (      FS_VOLUME * pVolume, const char * sNameSrc, const char * sNameDest);

/*********************************************************************
*
*       FS_FAT_DirEntry
*
*/
int  FS_FAT_SetDirEntryInfo     (      FS_VOLUME  * pVolume, const char * sName, const void * p, int Mask);
int  FS_FAT_GetDirEntryInfo     (      FS_VOLUME  * pVolume, const char * sName,       void * p, int Mask);
int  FS_FAT_SetDirEntryInfoEx   (      FS_VOLUME  * pVolume, const FS_DIRENTRY_POS * pDirEntryPos, const void * p, int Mask);
int  FS_FAT_GetDirEntryInfoEx   (      FS_VOLUME  * pVolume, const FS_DIRENTRY_POS * pDirEntryPos,       void * p, int Mask);

/*********************************************************************
*
*       FAT_Rename
*/
int   FS_FAT_Rename             (      FS_VOLUME  * pVolume, const char * sOldName, const char * sNewName);

/*********************************************************************
*
*       FAT_Dir
*/
int  FS_FAT_OpenDir             (const char       * sDirName, FS_DIR_OBJ *pDirObj);
int  FS_FAT_CloseDir            (      FS_DIR_OBJ * pDirObj);
int  FS_FAT_ReadDir             (      FS_DIR_OBJ * pDirObj, FS_DIRENTRY_INFO * pDirEntryInfo);
int  FS_FAT_RemoveDir           (      FS_VOLUME  * pVolume, const char * sDirName);
int  FS_FAT_CreateDir           (      FS_VOLUME  * pVolume, const char * sDirName);
int  FS_FAT_DeleteDir           (      FS_VOLUME  * pVolume, const char * sDirName, int MaxRecursionLevel);

/*********************************************************************
*
*       FAT_SetEndOfFile
*/
int  FS_FAT_SetEndOfFile        (FS_FILE * pFile);
int  FS_FAT_SetFileSize         (FS_FILE * pFile, U32 NumBytes);

/*********************************************************************
*
*       FS_FAT_CheckDisk
*/
int  FS_FAT_CheckVolume         (FS_VOLUME * pVolume, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
int  FS_FAT_CheckDir            (FS_VOLUME * pVolume, const char * sPath, FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
int  FS_FAT_CheckAT             (FS_VOLUME * pVolume, const FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // FAT_H

/*************************** End of file ****************************/
