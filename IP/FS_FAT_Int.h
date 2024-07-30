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
File        : FS_FAT_Int.h
Purpose     : Internal FAT File System Layer header
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_FAT_INT_H              // Avoid recursive and multiple inclusion
#define FS_FAT_INT_H

#include "FS_Int.h"
#include "FS_FAT.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       File and directory attributes
*/
#define FS_FAT_ATTR_READ_ONLY             FS_ATTR_READ_ONLY
#define FS_FAT_ATTR_HIDDEN                FS_ATTR_HIDDEN
#define FS_FAT_ATTR_SYSTEM                FS_ATTR_SYSTEM
#define FS_FAT_ATTR_VOLUME_ID             0x08u
#define FS_FAT_ATTR_ARCHIVE               FS_ATTR_ARCHIVE
#define FS_FAT_ATTR_DIRECTORY             FS_ATTR_DIRECTORY
#define FS_FAT_ATTR_LONGNAME              (FS_FAT_ATTR_READ_ONLY | FS_FAT_ATTR_HIDDEN | FS_FAT_ATTR_SYSTEM | FS_FAT_ATTR_VOLUME_ID)
#define FS_FAT_ATTR_MASK                  (FS_FAT_ATTR_READ_ONLY | FS_FAT_ATTR_HIDDEN | FS_FAT_ATTR_SYSTEM | FS_FAT_ATTR_VOLUME_ID | FS_FAT_ATTR_ARCHIVE | FS_FAT_ATTR_DIRECTORY)

/*********************************************************************
*
*       File system types
*/
#define FS_FAT_TYPE_UNKONWN               0u
#define FS_FAT_TYPE_FAT12                 12u
#define FS_FAT_TYPE_FAT16                 16u
#define FS_FAT_TYPE_FAT32                 32u

/*********************************************************************
*
*       Defines for the reserved flags of director entry
*
*  These flags are set if the base or the extension of a file name
*  should be displayed in lower case.
*/
#define FS_FAT_FLAG_LOWER_CASE_BASE       0x08u
#define FS_FAT_FLAG_LOWER_CASE_EXT        0x10u

/*********************************************************************
*
*       Defines for Boot parameter block offsets
*/
#define BPB_OFF_SIGNATURE                 510
#define BPB_OFF_BYTES_PER_SECTOR          11
#define BPB_OFF_NUM_FATS                  16
#define BPB_OFF_SECTOR_PER_CLUSTER        13
#define BPB_OFF_NUMSECTORS_16BIT          19
#define BPB_OFF_NUMSECTORS_32BIT          32
#define BPB_OFF_FATSIZE_16BIT             22
#define BPB_OFF_FATSIZE_32BIT             36
#define BPB_OFF_FAT16_RESERVED1           37
#define BPB_OFF_FAT32_EXTFLAGS            40
#define BPB_OFF_ROOTDIR_CLUSTER           44
#define BPB_OFF_FAT32_FSINFO_SECTOR       48
#define BPB_OFF_FAT32_RESERVED1           65

/*********************************************************************
*
*       Defines for FAT32 FSInfo Sector offsets
*/
#define FSINFO_OFF_SIGNATURE_1            0
#define FSINFO_OFF_SIGNATURE_2            484
#define FSINFO_OFF_FREE_CLUSTERS          488
#define FSINFO_OFF_NEXT_FREE_CLUSTER      492
#define FSINFO_OFF_SIGNATURE_3            508

/*********************************************************************
*
*       Defines for FAT32 FSInfo related values
*/
#define FSINFO_SIGNATURE_1                0x41615252UL
#define FSINFO_SIGNATURE_2                0x61417272UL
#define FSINFO_SIGNATURE_3                0xAA550000UL

/*********************************************************************
*
*       Defines related tor directory entries
*/
#define DIR_ENTRY_OFF_EXTENSION           8
#define DIR_ENTRY_OFF_ATTRIBUTES          11
#define DIR_ENTRY_OFF_FLAGS               12
#define DIR_ENTRY_OFF_SIZE                28
#define DIR_ENTRY_OFF_FIRSTCLUSTER_LOW    26
#define DIR_ENTRY_OFF_FIRSTCLUSTER_HIGH   20
#define DIR_ENTRY_OFF_CREATION_TIME       14
#define DIR_ENTRY_OFF_CREATION_DATE       16
#define DIR_ENTRY_OFF_LAST_ACCESS_DATE    18
#define DIR_ENTRY_OFF_WRITE_TIME          22
#define DIR_ENTRY_OFF_WRITE_DATE          24
#define DIR_ENTRY_SHIFT                   5u
#define DIR_ENTRY_SIZE                    (1u << DIR_ENTRY_SHIFT)
#define DIR_ENTRY_INVALID_MARKER          0xE5u
#define DIR_ENTRY_INDEX_INVALID           (-1)

/*********************************************************************
*
*       Defines for special sector indexes
*/
#define SECTOR_INDEX_BPB                  0
#define SECTOR_INDEX_FSINFO               1
#define SECTOR_INDEX_BPB_BACKUP           6
#define SECTOR_INDEX_FSINFO_BACKUP        7

/*********************************************************************
*
*       Other defines
*/
#define FAT_FIRST_CLUSTER                 2u
#define FAT_NUM_ALLOC_TABLES              2u
#define FAT_MAX_NUM_CHARS_LFN             255u    // Maximum number of characters in a long file name. Windows supports maximum 255 characters.
#define FAT_WRITE_IN_PROGRESS             0x1u    // Mask of the bit set by Windows to 1 when a volume has been removed during a write operation.
#define FAT32_CLUSTER_ID_MASK             0x0FFFFFFFuL
#define NUM_FREE_CLUSTERS_INVALID         0xFFFFFFFFuL
#define FAT_MAX_NUM_CLUSTERS_DIR          1000
#define FAT_MAX_NUM_BYTES_BASE            8u      // Maximum number of bytes in a file name base.
#define FAT_MAX_NUM_BYTES_EXT             3u      // Maximum number of bytes in a file name extension.
#define FAT_MAX_NUM_BYTES_SFN             (FAT_MAX_NUM_BYTES_BASE + FAT_MAX_NUM_BYTES_EXT)    // Maximum number of bytes in a short file name (not including the extension separator)
#define FAT_MAX_FILE_SIZE                 0xFFFFFFFFuL

/*********************************************************************
*
*       Public types
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FAT_DENTRY
*
*  FAT directory entry
*/
typedef struct {
  U8 Data[DIR_ENTRY_SIZE];
} FS_FAT_DENTRY;

/*********************************************************************
*
*       FS_83NAME
*/
typedef struct {
  U8 ac[11];
} FS_83NAME;

/*********************************************************************
*
*       FAT_DIRENTRY_API
*/
typedef struct {
  int             (*pfReadDirEntryInfo)(FS_DIR_OBJ * pDirObj, FS_DIRENTRY_INFO * pDirEntryInfo, FS_DIR_POS * pDirPosLFN, FS_SB * pSB);
  FS_FAT_DENTRY * (*pfFindDirEntry)    (FS_VOLUME * pVolume, FS_SB * pSB, const char * sLongName, int Len, U32 DirStart, FS_DIR_POS * pDirPos, unsigned AttribRequired, FS_DIR_POS * pDirPosLFN);
  FS_FAT_DENTRY * (*pfCreateDirEntry)  (FS_VOLUME * pVolume, FS_SB * pSB, const char * pFileName, U32 DirStart, U32 ClusterId, unsigned Attributes, U32 Size, unsigned Time, unsigned Date);
  int             (*pfDelLongEntry)    (FS_VOLUME * pVolume, FS_SB * pSB, FS_DIR_POS * pDirPosLFN);
} FAT_DIRENTRY_API;

/*********************************************************************
*
*       FAT_FORMAT_INFO
*/
typedef struct {
  U16 SectorsPerCluster;    // Number of sectors in a cluster.
  U16 NumRootDirEntries;    // Number of entries in the root directory.
  U32 NumSectorsReserved;   // Number of reserved sectors (for alignment purposes)
  U32 NumClusters;          // Total number of clusters available.
  U32 NumSectorsAT;         // Number of sectors in the allocation table.
  U16 SectorsPerTrack;      // Number of sectors on a disk track.
  U16 NumHeads;             // Number of read / write heads.
  U8  FATType;              // Type of the file system (one of FS_FAT_TYPE_...)
} FAT_FORMAT_INFO;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
extern const FAT_DIRENTRY_API   FAT_SFN_API;
extern const FAT_DIRENTRY_API   FAT_LFN_API;
extern const FAT_DIRENTRY_API * FAT_pDirEntryAPI;
extern U8                       FAT_UseFSInfoSector;
extern U8                       FAT_MaintainFATCopy;
extern U8                       FAT_PermitROFileMove;
extern U8                       FAT_UpdateDirtyFlag;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FAT_Misc
*/
U32             FS_FAT_ClusterId2SectorNo      (const FS_FAT_INFO * pFATInfo, U32 ClusterId);
U32             FS_FAT_FindFreeCluster         (FS_VOLUME * pVolume, FS_SB * pSB, U32 FirstCluster, FS_FILE * pFile);
int             FS_FAT_LinkCluster             (FS_VOLUME * pVolume, FS_SB * pSB, U32 LastCluster, U32 NewCluster);
int             FS_FAT_MarkClusterEOC          (FS_VOLUME * pVolume, FS_SB * pSB, U32 Cluster);
U32             FS_FAT_WalkCluster             (FS_VOLUME * pVolume, FS_SB * pSB, U32 ClusterId, U32   NumClusters);
U32             FS_FAT_WalkClusterEx           (FS_VOLUME * pVolume, FS_SB * pSB, U32 ClusterId, U32 * pNumClusters);
U32             FS_FAT_AllocCluster            (FS_VOLUME * pVolume, FS_SB * pSB, U32 LastCluster);
U32             FS_FAT_ReadFATEntry            (FS_VOLUME * pVolume, FS_SB * pSB, U32 ClusterId);
U32             FS_FAT_FindLastCluster         (FS_VOLUME * pVolume, FS_SB * pSB, U32 ClusterId, U32 * pNumClusters);
int             FS_FAT_FreeClusterChain        (FS_VOLUME * pVolume, FS_SB * pSB, U32 ClusterId, U32 NumClusters);
int             FS_FAT_GotoCluster             (const FS_FILE * pFile, FS_SB * pSB);
int             FS_FAT_AllocClusterBlock       (FS_VOLUME * pVolume, U32 FirstCluster, U32 NumClusters, FS_SB * pSB);
FS_FAT_DENTRY * FS_FAT_FindDirEntry            (FS_VOLUME * pVolume, FS_SB * pSB, const char * pEntryName, int Len, U32 DirStart, unsigned AttrRequired, FS_DIR_POS * pDirPosLFN);
FS_FAT_DENTRY * FS_FAT_FindDirEntryEx          (FS_VOLUME * pVolume, FS_SB * pSB, const char * pEntryName, int Len, U32 DirStart, FS_DIR_POS * pDirPos, unsigned AttrRequired, FS_DIR_POS * pDirPosLFN);
int             FS_FAT_DelLongDirEntry         (FS_VOLUME * pVolume, FS_SB * pSB, FS_DIR_POS * pDirPos);
unsigned        FS_FAT_GetFATType              (U32 NumClusters);
int             FS_FAT_MakeShortName           (FS_83NAME * pOutName, const char *pOrgName, int Len, int AcceptMultipleDots);
#if FS_FAT_SUPPORT_FREE_CLUSTER_CACHE
  int           FS_FAT_SyncAT                  (FS_VOLUME * pVolume,  FS_SB * pSB);
#endif // FS_FAT_SUPPORT_FREE_CLUSTER_CACHE
int             FS_FAT_IsValidShortNameChar    (U8 c);
void            FS_FAT_LoadShortName           (char * sName, unsigned MaxNumBytes, const U8 * pShortName);
void            FS_FAT_CopyDirEntryInfo        (const FS_FAT_DENTRY * pDirEntry, FS_DIRENTRY_INFO * pDirEntryInfo);
int             FS_FAT_StoreShortName          (FS_83NAME * pShortName, const U8 * pName, unsigned NumBytes, int AcceptMultipleDots);
int             FS_FAT_StoreShortNameMB        (FS_83NAME * pShortName, const U8 * pName, unsigned NumBytes, int AcceptMultipleDots);
void            FS_FAT_StoreShortNamePartial   (U8 * pShortName, const U8 * pName, unsigned NumBytes, int EndPos);
void            FS_FAT_StoreShortNamePartialMB (U8 * pShortName, const U8 * pName, unsigned NumBytes, int EndPos);
void            FS_FAT_StoreShortNameCompleteMB(FS_83NAME * pShortName, const U8 * pName, unsigned NumBytes, unsigned ExtPos);
unsigned        FS_FAT_CalcCheckSum            (const U8 * pShortName);
int             FS_FAT_Validate                (void);
I32             FS_FAT_CalcDirEntryIndex       (FS_SB * pSB, const FS_FAT_DENTRY * pDirEntry);
int             FS_FAT_IsClusterFree           (FS_VOLUME * pVolume, FS_SB * pSB, U32 ClusterId);

/*********************************************************************
*
*       FAT_Open
*/
FS_FAT_DENTRY * FS_FAT_FindDirEntryShort       (FS_VOLUME * pVolume, FS_SB * pSB, const char * pEntryName, int Len, U32 DirStart, FS_DIR_POS * pDirPos, unsigned AttributeReq);
FS_FAT_DENTRY * FS_FAT_FindDirEntryShortEx     (FS_VOLUME * pVolume, FS_SB * pSB, const char * pEntryName, int Len, FS_DIR_POS * pDirPos, unsigned AttributeReq);
FS_FAT_DENTRY * FS_FAT_FindEmptyDirEntry       (FS_VOLUME * pVolume, FS_SB * pSB, U32 DirStart);
FS_FAT_DENTRY * FS_FAT_GetDirEntry             (FS_VOLUME * pVolume, FS_SB * pSB,  FS_DIR_POS * pDirPos);
FS_FAT_DENTRY * FS_FAT_GetDirEntryEx           (const FS_VOLUME * pVolume, FS_SB * pSB, const FS_DIRENTRY_POS * pDirEntryPos);
void            FS_FAT_IncDirPos               (FS_DIR_POS * pDirPos);
void            FS_FAT_InvalidateDirPos        (FS_DIR_POS * pDirPos);
int             FS_FAT_IsValidDirPos           (const FS_DIR_POS * pDirPos);
void            FS_FAT_WriteDirEntryCluster    (FS_FAT_DENTRY * pDirEntry, U32 Cluster);
U32             FS_FAT_GetFirstCluster         (const FS_FAT_DENTRY * pDirEntry);
void            FS_FAT_WriteDirEntry83         (FS_FAT_DENTRY * pDirEntry, const FS_83NAME * pFileName, U32 ClusterId, unsigned Attributes, U32 Size, unsigned Time, unsigned Date, unsigned Flags);
int             FS_FAT_FindPath                (FS_VOLUME * pVolume, FS_SB * pSB,  const char * pFullName, const char ** ppFileName, U32 * pFirstCluster);
int             FS_FAT_FindPathEx              (FS_VOLUME * pVolume, FS_SB * pSB,  const char * pFullName, const char ** ppFileName, U32 * pFirstCluster, FS_FAT_DENTRY ** ppDirEntry, U32 ClusterId);
int             FS_FAT_DeleteFileOrDir         (FS_VOLUME * pVolume, FS_SB * pSB, FS_FAT_DENTRY * pDirEntry, int IsFile, FS_DIR_POS * pDirPos);
void            FS_FAT_InitDirEntryScan        (const FS_FAT_INFO * pFATInfo, FS_DIR_POS * pDirPos, U32 DirCluster);
int             FS_FAT_IsFileOpen              (const FS_VOLUME * pVolume, U32 SectorIndex, U32 DirEntryIndex);

/*********************************************************************
*
*       FAT_Write
*/
int  FS_FAT_WriteDataSector          (U32 SectorIndex, int WriteToJournal, FS_SB * pSB);
#if FS_SUPPORT_ENCRYPTION
  int FS_FAT_WriteDataSectorEncrypted(U32 SectorIndex, U32 FilePos, U32 NumBytesToWrite, U32 FileSize, int WriteToJournal, FS_FILE_OBJ * pFileObj, FS_SB * pSBData, FS_SB * pSBCrypt);
#endif // FS_SUPPORT_ENCRYPTION
void FS_FAT_UpdateDirtyFlagIfRequired(FS_VOLUME * pVolume, int IsDirty);
int  FS_FAT_UpdateDirEntry           (const FS_FILE_OBJ * pFileObj, FS_SB * pSB, int WriteToJournal);
int  FS_FAT_GotoClusterAllocIfReq    (FS_FILE *pFile, FS_SB * pSB);

/*********************************************************************
*
*       FAT_Read
*/
int  FS_FAT_ReadDataSector           (U32 SectorIndex, U32 FilePos, const FS_FILE_OBJ * pFileObj, FS_SB * pSBData, FS_SB * pSBCrypt);

/*********************************************************************
*
*       FAT_Move
*/
int  FS_FAT_MoveEx                   (FS_VOLUME * pVolume, U32 DirStartSrc, U32 DirStartDest, const char * sNameSrcNQ, const char * sNameDestNQ, FS_SB * pSB);

/*********************************************************************
*
*       FAT_Dir
*/
int  FS_FAT_CreateDirEx              (FS_VOLUME * pVolume, const char *pDirName, U32 DirStart, FS_SB * pSB);

/*********************************************************************
*
*       FAT_Format
*/
int  FS_FAT_FormatVolume             (FS_VOLUME * pVolume, const FS_DEV_INFO * pDevInfo, const FAT_FORMAT_INFO * pFormatInfo, int UpdatePartition);

/*********************************************************************
*
*       FAT_CheckDisk
*/
void  FS_FAT_CHECKDISK_Init          (void);
void  FS_FAT_CHECKDISK_Save          (FS_CONTEXT       * pContext);
void  FS_FAT_CHECKDISK_Restore       (const FS_CONTEXT * pContext);

/*********************************************************************
*
*       FS_FAT_LFN
*/
void FS_FAT_LFN_Save            (      FS_CONTEXT * pContext);
void FS_FAT_LFN_Restore         (const FS_CONTEXT * pContext);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // FAT_INTERN_H

/*************************** End of file ****************************/
