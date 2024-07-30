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
File        : FS_Int.h
Purpose     : Internals used across different layers of the file system
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_INT_H                    // Avoid recursive and multiple inclusion
#define FS_INT_H

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "FS.h"
#include "FS_Debug.h"
#include "FS_OS.h"

#if FS_SUPPORT_FAT
  #include "FS_FAT.h"
#endif
#if FS_SUPPORT_EFS
  #include "FS_EFS.h"
#endif

#if defined(__cplusplus)
extern "C" {                        // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#define CLUSTER_ID_INVALID                0xFFFFFFFFuL
#define CLUSTER_INDEX_INVALID             0xFFFFFFFFuL
#define FS_MAX_FILE_SIZE                  0xFFFFFFFFuL
#define SECTOR_INDEX_INVALID              0xFFFFFFFFuL

#define FILE_BUFFER_FLAGS_VALID           (1uL << 7)  // The most significant bit of the file buffer flags indicates if the other flags are valid.

#define FS_VERSION2STRING_HELPER(x)       #x          //lint !e9024  '#/##' operator used in macro [MISRA 2012 Rule 20.10, advisory] N:999.
#define FS_VERSION2STRING(x)              FS_VERSION2STRING_HELPER(x)
#define TIME_DATE_DEFAULT                 0x00210000uL

/*********************************************************************
*
*       Defines related to Master Boot Record
*/
#define MBR_OFF_PARTITION0                0x01BEu    // Start of partition table
#define MBR_OFF_SIGNATURE                 0x01FEu
#define MBR_SIGNATURE                     0xAA55u

#define PART_ENTRY_STATUS_ACTIVE          0x80u
#define PART_ENTRY_STATUS_INACTIVE        0x00u

#define PART_ENTRY_SIZE                   16u
#define PART_ENTRY_OFF_START_SECTOR       0x08u
#define PART_ENTRY_OFF_NUM_SECTORS        0x0Cu

/*********************************************************************
*
*       Defines related to bit error correction
*
*  Positive values indicate the number of bit errors corrected.
*/
#define ECC_CORR_NOT_APPLIED              0           // No bit errors detected
#define ECC_CORR_FAILURE                  (-1)        // Bit errors detected but not able to correct

/*********************************************************************
*
*       Defines related to BigFile implementation
*/
#ifndef   BIGFILE_MAX_FILE_SIZE
  #define BIGFILE_MAX_FILE_SIZE           (0xFFFFFFFFuL - ((128uL * 1024uL) - 1uL))     // Maximum size of a fragment file. Configurable for testing purposes only.
#endif
#define BIGFILE_EXT_TEXT                  ".BigFile"              // Extension for the fragment files.
#define BIGFILE_EXT_TEXT_LEN              8u                      // strlen(BIGFILE_EXT_TEXT) This value has to be kept in sync with BIGFILE_EXT_TEXT.
#define BIGFILE_EXT_MIN_INDEX             1u                      // Maximum value of the index in the extension of a fragment file.
#define BIGFILE_EXT_MAX_INDEX             999u                    // Maximum value of the index in the extension of a fragment file.
#define BIGFILE_EXT_NUM_DIGITS            3u                      // This value has to be kept in sync with BIGFILE_EXT_MAX_INDEX
#define BIGFILE_EXT_NUM_BYTES             (1u + BIGFILE_EXT_NUM_DIGITS + BIGFILE_EXT_TEXT_LEN)  // Total number of bytes in the extension. +1 for the extension separator.
#define BIGFILE_MAX_VOUME_NAME            16u                     // Maximum number of bytes in the name of the volume.
#define BIGFILE_SIZE_INVALID              0xFFFFFFFFFFFFFFFFuLL   // Value to indicate that the size of the file is not known and it has to be calculated.

/*********************************************************************
*
*       File allocation mode
*
*  Description
*    Modes of allocating a cluster for a file.
*
*  Additional information
*    These values can be passed to FS_SetFileAllocMode(). The default
*    allocation mode is DISK_ALLOC_MODE_FIRST_FREE.
*
*    DISK_ALLOC_MODE_NEXT_FREE can potentially decrease the time
*    required for searching for a free cluster when the application
*    creates files that grow very large with the time such as log files.
*/
#if FS_SUPPORT_TEST
  #define DISK_ALLOC_MODE_FIRST_FREE      0u        // The search for a free cluster starts from the last cluster allocated to file.
  #define DISK_ALLOC_MODE_NEXT_FREE       1u        // The search for a free cluster starts from the last known free cluster.
  #define DISK_ALLOC_MODE_BEST_FREE       2u        // The search for a free cluster starts from the last known free cluster if the next cluster in the row is not free.
  #define DISK_ALLOC_MODE_UNKNOWN         3u        // End of enumeration. For internal use only.
#endif

/*********************************************************************
*
*       Cache command codes
*/
#define FS_CMD_CACHE_SET_MODE             6000L
#define FS_CMD_CACHE_CLEAN                6001L     // Write out all dirty sectors
#define FS_CMD_CACHE_SET_QUOTA            6002L
#define FS_CMD_CACHE_FREE_SECTORS         6003L
#define FS_CMD_CACHE_INVALIDATE           6004L     // Invalidate all sectors in cache
#define FS_CMD_CACHE_SET_ASSOC_LEVEL      6005L     // Sets the associativity level for the multi-way cache
#define FS_CMD_CACHE_GET_NUM_SECTORS      6006L     // Returns the number of sectors the cache is able to store
#define FS_CMD_CACHE_GET_TYPE             6007L     // Returns the type of the cache configured

/*********************************************************************
*
*       OS layer
*/
#if   (FS_OS_LOCKING == 0)
  #define FS_OS                           0
  #define FS_OS_LOCK_PER_DRIVER           0
#elif (FS_OS_LOCKING == 1)
  #define FS_OS                           1
  #define FS_OS_LOCK_PER_DRIVER           0
#elif (FS_OS_LOCKING == 2)
  #define FS_OS                           1
  #define FS_OS_LOCK_PER_DRIVER           1
#else
  #error FS_OS_LOCKING is set to an invalid value.
#endif

#ifndef   FS_OS
  #define FS_OS                           0
#endif

#ifndef   FS_OS_LOCK_PER_DRIVER
  #define FS_OS_LOCK_PER_DRIVER           0     // 0 means a single lock for all files, 1 means one lock per file.
#endif

#if FS_OS_LOCK_PER_DRIVER
  #define FS_VOLATILE                     volatile
#else
  #define FS_VOLATILE
#endif // FS_OS_LOCK_PER_DRIVER

/*********************************************************************
*
*       Sector buffers
*/
#ifndef FS_NUM_SECTOR_BUFFERS_PER_OPERATION
  #if (FS_SUPPORT_FAT != 0) && (FS_FAT_UPDATE_DIRTY_FLAG != 0)
    //
    // An additional sector buffer is required to update the "dirty" flag in the boot sector.
    //
    #define FS_NUM_SECTOR_BUFFERS_FS              3u
  #else
    //
    // FAT and EFS file systems require at least 2 sector buffers for an FS operation.
    //
    #define FS_NUM_SECTOR_BUFFERS_FS              2u
  #endif
  //
  // File encryption requires one additional sector buffer.
  //
  #if FS_SUPPORT_ENCRYPTION
    #define FS_NUM_SECTOR_BUFFERS_ENCRYPTION      1u
  #else
    #define FS_NUM_SECTOR_BUFFERS_ENCRYPTION      0u
  #endif
  //
  // Journaling requires one additional sector buffer.
  //
  #if FS_SUPPORT_JOURNAL
    #define FS_NUM_SECTOR_BUFFERS_JOURNAL         1u
  #else
    #define FS_NUM_SECTOR_BUFFERS_JOURNAL         0u
  #endif
  //
  // Total number of sector buffers.
  //
  #define FS_NUM_SECTOR_BUFFERS_PER_OPERATION     (FS_NUM_SECTOR_BUFFERS_FS + FS_NUM_SECTOR_BUFFERS_ENCRYPTION + FS_NUM_SECTOR_BUFFERS_JOURNAL)
#endif

/*********************************************************************
*
*       Callbacks for testing
*/

/*********************************************************************
*
*       FS_STORAGE_TEST_HOOK_SECTOR_READ_BEGIN
*/
typedef void (FS_STORAGE_TEST_HOOK_SECTOR_READ_BEGIN)(const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, U32 * pSectorIndex, void * pBuffer, U32 * pNumSectors);

/*********************************************************************
*
*       FS_STORAGE_TEST_HOOK_SECTOR_READ_END
*/
typedef void (FS_STORAGE_TEST_HOOK_SECTOR_READ_END)(const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, U32 SectorIndex, void * pBuffer, U32 NumSectors, int * pResult);           //lint !e621 Identifier clash N:107. Rationale: This symbol is used only in test builds.

/*********************************************************************
*
*       FS_STORAGE_TEST_HOOK_SECTOR_WRITE_BEGIN
*/
typedef void (FS_STORAGE_TEST_HOOK_SECTOR_WRITE_BEGIN)(const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, U32 * pSectorIndex, const void ** ppBuffer, U32 * pNumSectors);

/*********************************************************************
*
*       FS_STORAGE_TEST_HOOK_SECTOR_WRITE_END
*/
typedef void (FS_STORAGE_TEST_HOOK_SECTOR_WRITE_END)(const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, U32 SectorIndex, const void * pBuffer, U32 NumSectors, int * pResult);    //lint !e621 Identifier clash  N:107. Rationale: This symbol is used only in test builds.

/*********************************************************************
*
*       FS_ON_DEINIT_CALLBACK
*/
typedef void (FS_ON_DEINIT_CALLBACK)(void);

/*********************************************************************
*
*       Public types
*
**********************************************************************
*/

/*********************************************************************
*
*       CACHE_MODE
*/
typedef struct {
  int TypeMask;    // Combination of FS_SECTOR_TYPE_DATA, FS_SECTOR_TYPE_MAN, FS_SECTOR_TYPE_DIR
  int ModeMask;    // Combination of FS_CACHE_MODE_R, FS_CACHE_MODE_W, FS_CACHE_MODE_D
} CACHE_MODE;

/*********************************************************************
*
*       CACHE_QUOTA
*/
typedef struct {
  int TypeMask;    // Combination of FS_SECTOR_TYPE_DATA, FS_SECTOR_TYPE_MAN, FS_SECTOR_TYPE_DIR
  U32 NumSectors;  // Number of sectors to use for the type(s) specified
} CACHE_QUOTA;

/*********************************************************************
*
*       CACHE_FREE
*/
typedef struct {
  U32 FirstSector;
  U32 NumSectors;
} CACHE_FREE;

/*********************************************************************
*
*       FS_FREE_CLUSTER_CACHE
*
*  Additional information
*    The range of cluster ids stored in the cache is:
*    StartCluster to StartCluster + NumClustersTotal - 1.
*/
typedef struct {
  U32          StartCluster;      // Id of the first cluster in the cache.
  U32          NumClustersTotal;  // Total number of consecutive cluster ids in the cache.
  U32          NumClustersInUse;  // Number of clusters that are currently in use.
  FS_FILE *    pFile;             // Opened file that is using the cache.
} FS_FREE_CLUSTER_CACHE;

/*********************************************************************
*
*       FAT_FSINFO_SECTOR
*
*  Additional information
*    SectorIndex is U16 instead of U32 because the FSInfo sector
*    is always located at the beginning of a partition and therefore
*    it can never be larger than 65535.
*/
typedef struct {
  U16 SectorIndex;          // Index of the FSInfo sector.
  U8  IsPresent;            // Set to 1 if the FSInfo sector is present on the volume.
  U8  IsUpdateRequired;     // Set to 1 if the FSInfo sector has to be updated when the volume is unmounted or synchronized.
} FAT_FSINFO_SECTOR;

/*********************************************************************
*
*       FS_FAT_INFO
*
*  Additional information
*    The first 3 members of the structure have to be identical
*    to the members of FS_INFO::Info
*/
typedef struct {
  U32                   NumSectors;                 // Total number of sectors available for the file system: RSVD + FAT + ROOT + DATA
  U16                   BytesPerSector;             // Number of bytes in a logical sector. Typical values are: 512, 1024, 2048, 4096
  U16                   ldBytesPerSector;           // Number of bytes in a logical sector as a power of 2 value. Typical values are: 9, 10, 11, 12.
  U32                   FirstDataSector;            // Index of the first logical sector that stores file or directory data.
  U32                   BytesPerCluster;            // Number of bytes in a cluster.
  U16                   ldBytesPerCluster;          // Number of bytes in a cluster as a power of 2 value.
  U32                   FATSize;                    // Number of sectors in the allocation table.
  U32                   RootDirPos;                 // Position of root directory. FAT32: Cluster, FAT12/16: Sector
  U16                   RootEntCnt;                 // Number of root directory entries
  U16                   RsvdSecCnt;                 // Number of reserved sectors. Typically, 1 for FAT12 and FAT16.
  U8                    SectorsPerCluster;          // Number of sector in an allocation unit
  U8                    NumFATs;                    // Number of allocation tables. Typical value is 2.
  U8                    FATType;                    // FS_FAT_TYPE_FAT12/16/32
  U32                   NumClusters;                // Total number of clusters on the storage device.
  U32                   NumFreeClusters;            // Once known, we keep track of the number of free clusters.
  U32                   NextFreeCluster;            // Once known, we keep track of the next free cluster.
#if FS_FAT_USE_FSINFO_SECTOR
  FAT_FSINFO_SECTOR     FSInfoSector;               // Information related to FSInfo sector of a FAT32 partition.
#endif
#if FS_FAT_UPDATE_DIRTY_FLAG
  U8                    IsDirty;                    // Set to 1 as soon as the file system writes to storage. Set to 0 when the file system is unmounted or synchronized.
#endif
#if FS_FAT_SUPPORT_FREE_CLUSTER_CACHE
  FS_FREE_CLUSTER_CACHE FreeClusterCache;           // Cache for the ids of consecutive free clusters. Used with file write operation mode set to fast.
#endif // FS_FAT_SUPPORT_FREE_CLUSTER_CACHE
  U32                   WriteCntAT;                 // Counts the number of times the file system has modified the allocation table.
} FS_FAT_INFO;

/*********************************************************************
*
*       FS_EFS_INFO
*
*  Additional information
*    The first 3 members of the structure have to be identical
*    to the members of FS_INFO::Info
*/
typedef struct {
  U32                   NumSectors;                     // RSVD + EFS allocation table + DATA
  U16                   BytesPerSector;                 // 512,1024,2048,4096
  U16                   ldBytesPerSector;               // 9, 10, 11, 12
  U32                   FirstDataSector;                // First data cluster -> First Sector after InfoSector, EFSAllocTable.
  U32                   BytesPerCluster;                // Bytes for each cluster: 512 - 32768
  U32                   EFSTableSize;                   // EFS allocation table size in number of sectors
  U8                    SectorsPerCluster;              // Number of sectors for a cluster
  U32                   NumClusters;                    // Number of available clusters
  U32                   NumFreeClusters;                // Once known, we keep track of the number of free clusters
  U32                   NextFreeCluster;                // Once known, we keep track of the next free cluster
#if FS_EFS_SUPPORT_STATUS_SECTOR
  U8                    IsNumFreeClustersInvalidated;   // Set to 1 if the number of free clusters in the status sector has been invalidated.
#endif
#if FS_EFS_SUPPORT_FREE_CLUSTER_CACHE
  FS_FREE_CLUSTER_CACHE FreeClusterCache;               // Cache for the ids of consecutive free clusters. Used with file write operation mode set to fast.
#endif // FS_EFS_SUPPORT_FREE_CLUSTER_CACHE
  U32                   WriteCntAT;                     // Counts the number of times the file system has modified the allocation table.
} FS_EFS_INFO;

/*********************************************************************
*
*       FS_INFO
*/
typedef union {
  FS_FAT_INFO FATInfo;
  FS_EFS_INFO EFSInfo;
  struct {
    U32       NumSectors;           // RSVD + EFS allocation table + DATA
    U16       BytesPerSector;       // 512,1024,2048,4096
    U16       ldBytesPerSector;     // 9, 10, 11, 12
  } Info;
} FS_INFO;

/*********************************************************************
*
*       FS_JOURNAL_DATA
*/
typedef struct {
  U8  Unit;                 // Unit number of the journal instance.
  U8  IsActive;             // Set to 1 if the journal instance is enabled.
  U8  IsInited;             // Set to 1 if the journal instance is allocated.
  U8  IsNewDataLogged;      // Set to 1 if all new data has to be written to journal.
  U32 MinClusterId;         // Smallest cluster id freed during the current journal transaction.
  U32 MaxClusterId;         // Largest cluster id freed during the current journal transaction.
  U8  IsTransactionNested;  // Set to 1 if the current transaction has been opened more than once.
} FS_JOURNAL_DATA;

/*********************************************************************
*
*       FS_DEVICE_DATA
*/
typedef struct {
  U8                               Unit;
  U8                               IsInited;
#if FS_SUPPORT_JOURNAL
  FS_JOURNAL_DATA                  JournalData;
#endif
#if FS_SUPPORT_CACHE
  const FS_CACHE_API             * pCacheAPI;
  void                           * pCacheData;
#endif
#if FS_SUPPORT_BUSY_LED
  FS_BUSY_LED_CALLBACK           * pfSetBusyLED;
#endif
#if FS_SUPPORT_CHECK_MEMORY
  FS_MEM_CHECK_CALLBACK          * pfMemCheck;
#endif
#if FS_STORAGE_SUPPORT_DEVICE_ACTIVITY
  FS_ON_DEVICE_ACTIVITY_CALLBACK * pfOnDeviceActivity;
#endif
} FS_DEVICE_DATA;

/*********************************************************************
*
*       FS_DEVICE
*/
struct FS_DEVICE {                    //lint -esym(9058, FS_DEVICE) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  const FS_DEVICE_TYPE * pType;
  FS_DEVICE_DATA         Data;
};

/*********************************************************************
*
*       FS_PARTITION
*/
struct FS_PARTITION {                 //lint -esym(9058, FS_PARTITION) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  FS_DEVICE Device;
  U32       StartSector;
  U32       NumSectors;
};

/*********************************************************************
*
*       SECTOR_BUFFER
*/
typedef struct {
  U8             InUse;               // Usage indicator.
  U32          * pBuffer;             // Data of the logical sector.
#if FS_SUPPORT_SECTOR_BUFFER_CACHE
  FS_PARTITION * pPart;               // Partition on which the logical sector is located.
  U32            SectorIndex;         // Index of the logical sector.
#endif // FS_SUPPORT_SECTOR_BUFFER_CACHE
} SECTOR_BUFFER;

/*********************************************************************
*
*       FS_FS_API
*
*  Description:
*    This structure is used to allow to use different file system types
*    such as FAT and EFS. It contains all API functions that are used
*    by the public API.
*/
typedef struct {
  int   (*pfCheckBootSector)      (      FS_VOLUME  * pVolume);
  int   (*pfOpenFile)             (const char       * pFileName, FS_FILE * pFile, int DoDel, int DoOpen, int DoCreate);
  int   (*pfCloseFile)            (      FS_FILE    * pFile);
  U32   (*pfRead)                 (      FS_FILE    * pFile,           void  * pData, U32 NumBytes);
  U32   (*pfWrite)                (      FS_FILE    * pFile,     const void  * pData, U32 NumBytes);
  int   (*pfFormat)               (      FS_VOLUME  * pVolume,   const FS_FORMAT_INFO * pFormatInfo);
  int   (*pfOpenDir)              (const char       * pDirName,  FS_DIR_OBJ * pDirObj);
  int   (*pfCloseDir)             (      FS_DIR_OBJ * pDirObj);
  int   (*pfReadDir)              (      FS_DIR_OBJ * pDirObj,   FS_DIRENTRY_INFO * pDirEntryInfo);
  int   (*pfRemoveDir)            (      FS_VOLUME  * pVolume,   const char * sDirName);
  int   (*pfCreateDir)            (      FS_VOLUME  * pVolume,   const char * sDirName);
  int   (*pfDeleteDir)            (      FS_VOLUME  * pVolume,   const char * sDirName, int MaxRecursionLevel);
  int   (*pfRename)               (      FS_VOLUME  * pVolume,   const char * sOldName, const char * sNewName);
  int   (*pfMove)                 (      FS_VOLUME  * pVolume,   const char * sOldName, const char * sNewName);
  int   (*pfSetDirEntryInfo)      (      FS_VOLUME  * pVolume,   const char * sName, const void * p, int Mask);
  int   (*pfGetDirEntryInfo)      (      FS_VOLUME  * pVolume,   const char * sName,       void * p, int Mask);
  int   (*pfSetEndOfFile)         (      FS_FILE    * pFile);
  void  (*pfUnmount)              (      FS_VOLUME  * pVolume);
  int   (*pfGetDiskInfo)          (      FS_VOLUME  * pVolume,   FS_DISK_INFO * pDiskData, int Flags);
  int   (*pfGetVolumeLabel)       (      FS_VOLUME  * pVolume,   char * pVolumeLabel, unsigned VolumeLabelSize);
  int   (*pfSetVolumeLabel)       (      FS_VOLUME  * pVolume,   const char * pVolumeLabel);
  int   (*pfCreateJournalFile)    (      FS_VOLUME  * pVolume,   U32 NumBytes, U32 * pFirstSector, U32 * pNumSectors);
  int   (*pfOpenJournalFile)      (      FS_VOLUME  * pVolume);
  U32   (*pfGetIndexOfLastSector) (      FS_VOLUME  * pVolume);
  int   (*pfCheckVolume)          (      FS_VOLUME  * pVolume,   void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
  int   (*pfUpdateFile)           (      FS_FILE    * pFile);
  int   (*pfSetFileSize)          (      FS_FILE    * pFile,     U32 NumBytes);
  int   (*pfFreeSectors)          (      FS_VOLUME  * pVolume);
  int   (*pfGetFreeSpace)         (      FS_VOLUME  * pVolume,   void * pBuffer, int SizeOfBuffer, U32 FirstClusterId, U32 * pNumClustersFree, U32 * pNumClustersChecked);
  int   (*pfGetATInfo)            (      FS_VOLUME  * pVolume,   FS_AT_INFO * pATInfo);
  int   (*pfCheckDir)             (      FS_VOLUME  * pVolume,   const char * sPath, FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
  int   (*pfCheckAT)              (      FS_VOLUME  * pVolume,   const FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
  I32   (*pfReadATEntry)          (      FS_VOLUME  * pVolume,   U32 ClusterId);
  int   (*pfSetDirEntryInfoEx)    (      FS_VOLUME  * pVolume,   const FS_DIRENTRY_POS * pDirEntryPos, const void * p, int Mask);
  int   (*pfGetDirEntryInfoEx)    (      FS_VOLUME  * pVolume,   const FS_DIRENTRY_POS * pDirEntryPos,       void * p, int Mask);
} FS_FS_API;

/*********************************************************************
*
*       FS_VOLUME
*/
struct FS_VOLUME {                      //lint -esym(9058, FS_VOLUME) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  FS_PARTITION      Partition;
  FS_INFO           FSInfo;
  U8                PartitionIndex;
  FS_VOLATILE U8    MountType;          // 0 if the volume is not mounted. It has to be declared as volatile because the member is set to 0 by an unmount operation.
  U8                AllowAutoMount;
  U8                InUse;
  FS_WRITEMODE      WriteMode;
#if FS_SUPPORT_MULTIPLE_FS
  const FS_FS_API * pFS_API;
#endif
#if FS_SUPPORT_FREE_SECTOR
  U8                FreeSector;
#endif
#if FS_SUPPORT_FILE_BUFFER
  U8                FileBufferFlags;
#endif
#if (FS_SUPPORT_JOURNAL != 0) && (FS_MAX_LEN_JOURNAL_FILE_NAME > 0)
  char              acJournalFileName[FS_MAX_LEN_JOURNAL_FILE_NAME];
#endif
#if FS_SUPPORT_VOLUME_ALIAS
#if (FS_MAX_LEN_VOLUME_ALIAS > 0)
  char              acAlias[FS_MAX_LEN_VOLUME_ALIAS];
#else
  const char      * sAlias;
#endif
#endif // FS_SUPPORT_VOLUME_ALIAS
#if FS_SUPPORT_TEST
  U8                SectorType;
#endif // FS_SUPPORT_TEST
  FS_VOLUME       * pNext;
};

/*********************************************************************
*
*       FS_DIRENTRY_INFO
*/
struct FS_DIRENTRY_INFO {             //lint -esym(9058, FS_DIRENTRY_INFO) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  U8             Attributes;
  U32            CreationTime;
  U32            LastAccessTime;
  U32            LastWriteTime;
  FS_FILE_SIZE   FileSize;
  char         * sFileName;
  int            SizeofFileName;
  U32            FirstClusterId;
};

/*********************************************************************
*
*       FS_INT_DATA_FAT
*
*  Description
*    Information about continuous clusters in the chain.
*
*  Additional information
*    CurClusterIndex is the index of the cluster corresponding to
*    the current file position that is relative to the beginning
*    of the file. That is the first cluster in the chain has the
*    index 0, the next cluster in the chain has the index 1 ans so on.
*    CurClusterIndex is not directly related to the cluster id
*    that is stored in the allocation table.
*
*    CurClusterId is he id of the cluster corresponding to the
*    current file position. This is the value that is stored in
*    the allocation table or in the directory entry. This value
*    has to be determined by evaluating the allocation table.
*/
typedef struct {
  U32 CurClusterIndex;  // Current cluster relative to beginning of the file.
  U32 CurClusterId;     // Current cluster in the allocation table.
#if FS_FAT_OPTIMIZE_LINEAR_ACCESS
  U16 NumAdjClusters;   // Number of sequential clusters in the chain from the current file position.
#endif // FS_FAT_OPTIMIZE_LINEAR_ACCESS
} FS_INT_DATA_FAT;

/*********************************************************************
*
*       FS_INT_DATA_EFS
*
*  Description
*    Information about continuous clusters in the chain.
*
*  Additional information
*    CurClusterIndex is the index of the cluster corresponding to
*    the current file position that is relative to the beginning
*    of the file. That is the first cluster in the chain has the
*    index 0, the next cluster in the chain has the index 1 ans so on.
*    CurClusterIndex is not directly related to the cluster id
*    that is stored in the allocation table.
*
*    CurClusterId is he id of the cluster corresponding to the
*    current file position. This is the value that is stored in
*    the allocation table or in the directory entry. This value
*    has to be determined by evaluating the allocation table.
*/
typedef struct {
  U32 CurClusterIndex;  // Current cluster relative to beginning of the file.
  U32 CurClusterId;     // Current cluster in the allocation table.
#if FS_EFS_OPTIMIZE_LINEAR_ACCESS
  U16 NumAdjClusters;   // Number of sequential clusters in the chain from the current file position.
#endif // FS_EFS_OPTIMIZE_LINEAR_ACCESS
} FS_INT_DATA_EFS;

/*********************************************************************
*
*       FS_INT_DATA
*/
typedef union {
  FS_INT_DATA_FAT Fat;
  FS_INT_DATA_EFS Efs;
} FS_INT_DATA;

/*********************************************************************
*
*       FS_FILE_OBJ
*
*  Description
*    Describes the file object structure below the handle.
*/
struct FS_FILE_OBJ {                //lint -esym(9058, FS_FILE_OBJ) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  U32               FirstCluster;   // First cluster used for file
  FS_FILE_SIZE      Size;           // Size of the file in bytes
  FS_VOLUME       * pVolume;        // Volume information
  U8                UseCnt;         // Counts how many file handles use this file object
  FS_DIRENTRY_POS   DirEntryPos;    // Position in the parent directory.
  FS_INT_DATA       Data;           // Cached data related to file system.
#if (FS_MULTI_HANDLE_SAFE != 0)
  char              acFullFileName[FS_MAX_LEN_FULL_FILE_NAME];   // Name of the opened file including the full path to it
#endif
#if FS_SUPPORT_ENCRYPTION
  FS_CRYPT_OBJ    * pCryptObj;      // Information about the encryption algorithm
  FS_FILE_SIZE      SizeEncrypted;  // Actual size of the file encrypted on the storage device. Can be different from Size if the file buffer is active.
#endif
  FS_FILE_OBJ     * pNext;
};

/*********************************************************************
*
*       FS_FILE
*
*  Description
*    Structure of a file handle.
*/
struct FS_FILE {                          //lint -esym(9058, FS_FILE) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  FS_FILE_OBJ * FS_VOLATILE pFileObj;              // Pointer to the associated file object. It has to be declared as volatile because it is set to NULL by a forced unmount operation.
  FS_FILE_SIZE              FilePos;               // Current position in file
#if FS_SUPPORT_FILE_BUFFER
  FS_FILE_BUFFER          * pBuffer;               // File buffer
#endif
  I16                       Error;                 // Last processing error
  FS_VOLATILE U8            InUse;                 // Set to 1 if the file handle is opened. It has to be declared as volatile because it is set to 0 by an unmount operation.
  U8                        AccessFlags;           // Access mode required by the application
  U8                        IsDirUpdateRequired;   // Set to 1 if the directory entry must be updated on file close
  FS_FILE                 * pNext;
};

/*********************************************************************
*
*       FS_SB
*
*  Description
*    Smart buffer (SB) type, internal
*/
struct FS_SB {                    //lint -esym(9058, FS_SB) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  U32            SectorIndex;     // Index of the sector to be accessed.
#if FS_MAINTAIN_FAT_COPY
  U32            WriteCopyOff;    // Duplicate on write if value is not 0.
#endif
  FS_PARTITION * pPart;           // Pointer to information about the partition on which the sector is stored.
  U8           * pBuffer;         // Sector data.
  U8             IsDirty;         // 1 if the sector data has been modified and it has to be written to storage.
  I8             Error;           // Error code indicating the failure reason.
  U8             Type;            // Type of data stored to sector.
  U8             Read;            // 1 if data is valid, usually because the sector has been read
  U8             WriteToJournal;  // 1 if the write should go through journal
};

/*********************************************************************
*
*       FS_PROFILE
*/
typedef struct {
  U32                    IdOffset;
  const FS_PROFILE_API * pAPI;
} FS_PROFILE;

/*********************************************************************
*
*       FS_WRBUF_STAT_COUNTERS
*/
typedef struct {
  U32 WriteOperationCnt;
  U32 WriteSectorCnt;
  U32 FreeOperationCnt;
  U32 FreeSectorCnt;
} FS_WRBUF_STAT_COUNTERS;

/*********************************************************************
*
*       FS_AT_INFO
*
*  Description
*    Information about the allocation table.
*/
struct FS_AT_INFO {               //lint -esym(9058, FS_AT_INFO) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  U32 NumClusters;
  U32 FirstClusterId;
  U32 WriteCnt;
};

/*********************************************************************
*
*       BURST_INFO_R
*
*  Description
*    Burst read operation.
*/
typedef struct {
  U32     FirstSector;
  U32     NumSectors;
  FS_SB * pSBData;
  void  * pData;
} BURST_INFO_R;

/*********************************************************************
*
*       BURST_INFO_W
*
*  Description
*    Burst write operation.
*/
typedef struct {
  U32          FirstSector;
  U32          NumSectors;
  FS_SB      * pSBData;
  const void * pData;
  U8           WriteToJournal;
} BURST_INFO_W;

/*********************************************************************
*
*       FS_CONTEXT
*
*  Description
*    Runtime context of file system.
*
*  Additional information
*    This structure contains all static and global variables used in the
*    API and FS layer. It is used as argument by the FS__SaveContext()
*    and FS__RestoreContext() pair of functions. These functions can be used
*    to save to and restore from the task stack the values of these internal
*    variables in order to make sure that that settings made by one task are
*    not overwritten by another one.
*/
struct FS_CONTEXT {                 //lint -esym(9058, FS_CONTEXT) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  FS_TIME_DATE_CALLBACK              * pfTimeDate;
  const FS_CHARSET_TYPE              * pCharSetType;
  FS_STORAGE_COUNTERS                  STORAGE_Counters;
  U16                                  PRNG_Value;
#if FS_SUPPRESS_EOF_ERROR
  U8                                   IsEOFErrorSuppressed;
#endif  // FS_SUPPRESS_EOF_ERROR
#if FS_SUPPORT_POSIX
  U8                                   IsPOSIXSupported;
#endif // FS_SUPPORT_POSIX
#if FS_VERIFY_WRITE
  U8                                   IsWriteVerificationEnabled;
#endif // FS_SUPPORT_POSIX
#if FS_SUPPORT_EFS
  int                                  EFS_CHECKDISK_FileIndex;
#if FS_EFS_SUPPORT_STATUS_SECTOR
  U8                                   EFS_SupportStatusSector;
#endif // FS_EFS_SUPPORT_STATUS_SECTOR
#if FS_EFS_CASE_SENSITIVE
  U8                                   EFS_IsCaseSensitive;
#endif // FS_EFS_CASE_SENSITIVE
#if FS_SUPPORT_FILE_NAME_ENCODING
  const FS_UNICODE_CONV              * EFS_pUnicodeConv;
#endif // FS_SUPPORT_FILE_NAME_ENCODING
#endif // FS_SUPPORT_EFS
#if FS_SUPPORT_FAT
  int                                  FAT_CHECKDISK_FileIndex;
  int                                  FAT_CHECKDISK_LastDirIndex;
  int                                  FAT_CHECKDISK_UseSameDir;
  int                                  FAT_CHECKDISK_AbortRequested;
  const void                         * FAT_pDirEntryAPI;                  // Declared as void pointer in order to avoid including FS_FAT_Int.h
#if FS_FAT_USE_FSINFO_SECTOR
  U8                                   FAT_UseFSInfoSector;
#endif
#if FS_MAINTAIN_FAT_COPY
  U8                                   FAT_MaintainFATCopy;
#endif
#if FS_FAT_PERMIT_RO_FILE_MOVE
  U8                                   FAT_PermitROFileMove;
#endif
#if FS_FAT_UPDATE_DIRTY_FLAG
  U8                                   FAT_UpdateDirtyFlag;
#endif
#if FS_SUPPORT_FILE_NAME_ENCODING
  const FS_UNICODE_CONV              * FAT_LFN_pUnicodeConv;
#endif
#endif // FS_SUPPORT_FAT
#if FS_SUPPORT_JOURNAL
  FS_JOURNAL_ON_OVERFLOW_CALLBACK    * JOURNAL_pfOnOverflow;
  FS_JOURNAL_ON_OVERFLOW_EX_CALLBACK * JOURNAL_pfOnOverflowEx;
#endif // FS_SUPPORT_JOURNAL
};

/*********************************************************************
*
*       FS_MEM_MANAGER
*
*  Description
*    Manager for the (semi-)dynamic memory allocation.
*/
typedef struct {
  U32                     NumBytesAllocated;
#if FS_SUPPORT_EXT_MEM_MANAGER
  FS_MEM_ALLOC_CALLBACK * pfAlloc;
  FS_MEM_FREE_CALLBACK  * pfFree;
#else
  U32                   * pData;
  U32                     NumBytesTotal;
#if FS_SUPPORT_DEINIT
  U32                     NumBlocksAllocated;
#endif  // FS_SUPPORT_DEINIT
#endif  // FS_SUPPORT_EXT_MEM_MANAGER
} FS_MEM_MANAGER;

/*********************************************************************
*
*       FS_ACCESS_MODE
*
*  Description
*    Access mode for files.
*/
typedef struct {
  const char * sMode;           // Access mode passed to FS_FOpen() or FS_FOpenEx()
  U8           AccessFlags;     // Access flags used by the internal open functions.
  U8           DoDel;           // Set to 1 if the file has to be deleted.
  U8           DoOpen;          // Set to 1 if the file has to be opened.
  U8           DoCreate;        // Set to 1 if the file has to be deleted.
} FS_ACCESS_MODE;

/*********************************************************************
*
*       Functions for sector cache access
*/
struct FS_CACHE_API {             //lint -esym(9058, FS_CACHE_API) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  int    (*pfReadFromCache)   (FS_DEVICE * pDevice,      U32 SectorIndex,       void * pData, U8 Type);
  int    (*pfUpdateCache)     (FS_DEVICE * pDevice,      U32 SectorIndex, const void * pData, U8 Type);    // Returns 0 if write cached, which means no further write is required
  void   (*pfInvalidateCache) (void      * pCacheData);
  int    (*pfCommand)         (FS_DEVICE * pDevice,      int Cmd, void *p);
  int    (*pfWriteIntoCache)  (FS_DEVICE * pDevice,      U32 SectorIndex, const void * pData, U8 Type);    // Returns 0 if write cached, which means no further write is required
};

/*********************************************************************
*
*       Functions for file access
*
*  Description
*    Set of functions that can be used to access the files.
*/
struct FS_FILE_API {              //lint -esym(9058, FS_FILE_API) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  int (*pfOpen)   (FS_VOLUME * pVolume, const char * sFileName, U8 AccessFlags, U8 DoCreate, U8 DoDel, U8 DoOpen, FS_FILE ** ppFile);
  U32 (*pfRead)   (FS_FILE * pFile,       void * pData, U32 NumBytes);
  U32 (*pfWrite)  (FS_FILE * pFile, const void * pData, U32 NumBytes);
  int (*pfSetPos) (FS_FILE * pFile, FS_FILE_OFF Offset, int Origin);
  int (*pfSetEnd) (FS_FILE * pFile);
  int (*pfSetSize)(FS_FILE * pFile, FS_FILE_SIZE NumBytes);
};

/*********************************************************************
*
*       Partition related functions
*/
int             FS__LocatePartition           (FS_VOLUME * pVolume);
int             FS__CreateMBR                 (FS_VOLUME * pVolume,       FS_PARTITION_INFO * pPartInfo, int NumPartitions);
int             FS__WriteMBR                  (FS_VOLUME * pVolume, const FS_PARTITION_INFO * pPartInfo, int NumPartitions);
int             FS__GetPartitionInfo          (FS_VOLUME * pVolume,       FS_PARTITION_INFO * pPartInfo, U8 PartIndex);
void            FS__LoadPartitionInfo         (U8 PartIndex,       FS_PARTITION_INFO * pPartInfo, const U8 * pBuffer);
void            FS__StorePartitionInfo        (U8 PartIndex, const FS_PARTITION_INFO * pPartInfo,       U8 * pBuffer);
void            FS__CalcPartitionInfo         (FS_PARTITION_INFO * pPartInfo, U32 NumSectorsDevice);

/*********************************************************************
*
*       Little-endian translation functions, internal
*/
U16             FS_LoadU16LE                  (const U8 *pBuffer);
U32             FS_LoadU32LE                  (const U8 *pBuffer);
void            FS_StoreU16LE                 (      U8 *pBuffer, unsigned Data);
void            FS_StoreU24LE                 (      U8 *pBuffer, U32 Data);
void            FS_StoreU32LE                 (      U8 *pBuffer, U32 Data);

/*********************************************************************
*
*       Big-endian translation functions, internal
*/
U32             FS_LoadU32BE                  (const U8 *pBuffer);
U16             FS_LoadU16BE                  (const U8 *pBuffer);
void            FS_StoreU32BE                 (      U8 *pBuffer, U32 Data);
void            FS_StoreU24BE                 (      U8 *pBuffer, U32 Data);
void            FS_StoreU16BE                 (      U8 *pBuffer, unsigned Data);

/*********************************************************************
*
*       Smart buffer (SB) API-functions, internal
*/
void            FS__SB_Clean                  (      FS_SB * pSB);
int             FS__SB_Create                 (      FS_SB * pSB, FS_PARTITION * pPart);
void            FS__SB_Delete                 (      FS_SB * pSB);
void            FS__SB_Flush                  (      FS_SB * pSB);
U8            * FS__SB_GetBuffer              (      FS_SB * pSB);
int             FS__SB_GetError               (const FS_SB * pSB);
FS_PARTITION  * FS__SB_GetPartition           (const FS_SB * pSB);
U32             FS__SB_GetSectorIndex         (const FS_SB * pSB);
void            FS__SB_MarkDirty              (      FS_SB * pSB);
void            FS__SB_MarkValid              (      FS_SB * pSB, U32 SectorIndex, unsigned Type, int WriteToJournal);
void            FS__SB_MarkNotDirty           (      FS_SB * pSB);
void            FS__SB_MarkNotValid           (      FS_SB * pSB);
int             FS__SB_Read                   (      FS_SB * pSB);
void            FS__SB_SetSector              (      FS_SB * pSB, U32 SectorIndex, unsigned Type, int WriteToJournal);
#if FS_MAINTAIN_FAT_COPY
  void          FS__SB_SetWriteCopyOff        (      FS_SB * pSB, U32 Off);
#endif
void            FS__SB_SetWriteToJournal      (      FS_SB * pSB, int WriteToJournal);
int             FS__SB_Write                  (      FS_SB * pSB);

/*********************************************************************
*
*       Cache related functions, internal
*/
int             FS__CACHE_CommandVolume       (FS_VOLUME * pVolume, int Cmd, void * pData);
int             FS__CACHE_CommandVolumeNL     (FS_VOLUME * pVolume, int Cmd, void * pData);
int             FS__CACHE_CommandDevice       (FS_DEVICE * pDevice, int Cmd, void * pData);
int             FS__CACHE_CleanNL             (FS_DEVICE * pDevice);
int             FS__CACHE_Clean               (FS_VOLUME * pVolume);

/*********************************************************************
*
*       Sector allocation API-functions, internal
*/
U8            * FS__AllocSectorBuffer         (void);
void            FS__FreeSectorBuffer          (const void * pBuffer);
#if FS_SUPPORT_SECTOR_BUFFER_CACHE
U8            * FS__AllocSectorBufferEx       (const FS_PARTITION * pPart, U32 SectorIndex, int * pIsMatching);
void            FS__FreeSectorBufferEx        (const void * pBuffer, FS_PARTITION * pPart, U32 SectorIndex, int IsValid);
void            FS__InvalidateSectorBuffer    (const FS_PARTITION * pPart, U32 SectorIndex, U32 NumSectors);
#endif // FS_SUPPORT_SECTOR_BUFFER_CACHE

/*********************************************************************
*
*       String operation API-functions, internal
*/
void            FS__AddSpaceHex               (U32 v, U8 Len, char ** ps);
void            FS__AddEscapedHex             (U32 v, U8 Len, char ** ps);

/*********************************************************************
*
*       Debug message filtering
*/
void            FS__SetErrorFilterNL          (U32 FilterMask);
U32             FS__GetErrorFilterNL          (void);

/*********************************************************************
*
*       Volume API-functions, internal
*/
FS_VOLUME     * FS__FindVolume                (const char * sVolumeName);
FS_VOLUME     * FS__FindVolumeEx              (const char * sName, const char ** psName);
int             FS__Mount                     (FS_VOLUME * pVolume, U8 MountType);
int             FS__AutoMount                 (FS_VOLUME * pVolume);
void            FS__Unmount                   (FS_VOLUME * pVolume);
void            FS__UnmountLL_NL              (FS_VOLUME * pVolume);
void            FS__UnmountForcedLL_NL        (FS_VOLUME * pVolume);
int             FS__Sync                      (FS_VOLUME * pVolume);
int             FS__SyncNL                    (FS_VOLUME * pVolume);
int             FS__CheckDisk                 (const char * sVolumeName, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);

/*********************************************************************
*
*       API-functions, internal (without Global locking)
*/
int             FS__FClose                    (FS_FILE * pFile);
int             FS__CloseFile                 (FS_FILE * pFile);
int             FS__SyncFile                  (FS_FILE * pFile);
FS_FILE_SIZE    FS__GetFileSize               (const FS_FILE * pFile);
U32             FS__Read                      (FS_FILE * pFile,       void * pData, U32 NumBytes);
U32             FS__Write                     (FS_FILE * pFile, const void * pData, U32 NumBytes);
int             FS__Verify                    (FS_FILE * pFile, const void * pData, U32 NumBytes);
int             FS__Move                      (const char * sNameSrc, const char * sNameDest);
int             FS__MoveEx                    (FS_VOLUME * pVolume, const char * sNameSrc, const char * sNameDest);
int             FS__Remove                    (const char * sFileName);
int             FS__RemoveEx                  (FS_VOLUME * pVolume, const char * sFileName);
int             FS__FOpenEx                   (const char * sFileName, const char * sMode, FS_FILE ** ppFile);
int             FS__OpenFile                  (const char * sFileName, FS_FILE * pFile, int DoDel, int DoOpen, int DoCreate);
int             FS__OpenFileDL                (FS_VOLUME * pVolume, const char * sFileName, unsigned AccessFlags, int DoCreate, int DoDel, int DoOpen, FS_FILE ** ppFile);
int             FS__OpenFileEx                (const char * sFileName, unsigned AccessFlags, int DoCreate, int DoDel, int DoOpen, FS_FILE ** ppFile);
U32             FS__CalcSizeInBytes           (U32 NumClusters, U32 SectorsPerCluster, U32 BytesPerSector);
U32             FS__CalcSizeInKB              (U32 NumClusters, U32 SectorsPerCluster, U32 BytesPerSector);
FS_VOLUME     * FS__AddDevice                 (const FS_DEVICE_TYPE * pDevType);
int             FS__AddPhysDevice             (const FS_DEVICE_TYPE * pDevType);
int             FS__IoCtl                     (FS_VOLUME * pVolume, I32 Cmd, I32 Aux, void *pBuffer);
int             FS__Format                    (FS_VOLUME * pVolume, const FS_FORMAT_INFO * pFormatInfo);
int             FS__SD_Format                 (FS_VOLUME * pVolume);
int             FS__IsSDFormatted             (FS_VOLUME * pVolume);
int             FS__GetNumVolumes             (void);
int             FS__CopyFile                  (const char * sFileNameSrc, const char * sFileNameDest);
int             FS__CopyFileEx                (const char * sFileNameSrc, const char * sFileNameDest, void * pBuffer, U32 NumBytes);
int             FS__GetVolumeInfo             (const char * sVolumeName, FS_DISK_INFO * pInfo);
int             FS__GetVolumeInfoEx           (const char * sVolumeName, FS_DISK_INFO * pInfo, int Flags);
int             FS__CreateDir                 (const char * sDirName);
int             FS__MkDir                     (const char * sDirName);
int             FS__MkDirEx                   (FS_VOLUME * pVolume, const char * sDirName);
int             FS__RmDir                     (const char * sDirName);
int             FS__RmDirEx                   (FS_VOLUME * pVolume, const char * sDirName);
int             FS__DeleteDir                 (const char * sDirName, int MaxRecursionLevel);
void            FS__SetFilePos                (FS_FILE * pFile, FS_FILE_OFF Off, int Origin);
int             FS__FSeek                     (FS_FILE * pFile, FS_FILE_OFF Off, int Origin);
I32             FS__FTell                     (const FS_FILE * pFile);
int             FS__GetFileTimeEx             (const char * sName, U32 * pTimeStamp, int TimeIndex);
int             FS__SetFileTimeEx             (const char * sName, U32   TimeStamp,  int TimeIndex);
int             FS__SetFileAttributes         (const char * sName, U8    Attr);
U8              FS__GetFileAttributes         (const char * sName);
U8              FS__GetFileAttributesEx       (FS_VOLUME * pVolume, const char * sName);
U8              FS__ModifyFileAttributes      (const char * sName, U8 AttrSet, U8 AttrClr);
int             FS__GetFileInfo               (const char * sName, FS_FILE_INFO * pInfo);
int             FS__SetEndOfFile              (FS_FILE    * pFile);
int             FS__SetFileSize               (FS_FILE    * pFile, FS_FILE_SIZE NumBytes);
void            FS__RemoveDevices             (void);
FS_DIR        * FS__OpenDir                   (const char * sDirName);
FS_DIR        * FS__OpenDirEx                 (FS_VOLUME * pVolume, const char * sDirName);
FS_DIRENT     * FS__ReadDir                   (FS_DIR * pDir);
int             FS__CloseDir                  (FS_DIR * pDir);
void            FS__RewindDir                 (FS_DIR * pDir);
void            FS__DirEnt2Attr               (const FS_DIRENT * pDirEnt, U8 * pAttr);
U32             FS__GetNumFiles               (FS_DIR * pDir);
int             FS__IsDirHandle               (const FS_DIR * pDir);
int             FS__FindFirstFile             (FS_FIND_DATA * pFD, const char * sDirName, char * sFileName, int SizeOfFileName);
int             FS__FindFirstFileEx           (FS_FIND_DATA * pFD, FS_VOLUME * pVolume, const char * sDirName, char * sFileName, int SizeOfFileName);
int             FS__FindNextFile              (FS_FIND_DATA * pFD);
void            FS__FindClose                 (FS_FIND_DATA * pFD);
int             FS__FormatLowNL               (FS_VOLUME * pVolume);
void            FS__UnmountForced             (FS_VOLUME * pVolume);
int             FS__GetVolumeStatusNL         (FS_VOLUME * pVolume);
int             FS__GetVolumeName             (FS_VOLUME * pVolume, char * sVolumeName, int VolumeNameSize);
int             FS__GetVolumeInfoDL           (FS_VOLUME * pVolume, FS_DISK_INFO * pInfo, int Flags);
int             FS__ReadSectorNL              (FS_VOLUME * pVolume,       void * pData, U32 SectorIndex);
int             FS__ReadSectorsNL             (FS_VOLUME * pVolume,       void * pData, U32 SectorIndex, U32 NumSectors);
int             FS__WriteSectorNL             (FS_VOLUME * pVolume, const void * pData, U32 SectorIndex);
int             FS__WriteSectorsNL            (FS_VOLUME * pVolume, const void * pData, U32 SectorIndex, U32 NumSectors);
int             FS__GetDeviceInfoNL           (FS_VOLUME * pVolume, FS_DEV_INFO * pDevInfo);
int             FS__IsLLFormattedNL           (FS_VOLUME * pVolume);
void            FS__CalcDeviceInfo            (FS_DEV_INFO * pDevInfo);
FS_WRITEMODE    FS__GetFileWriteModeEx        (const FS_VOLUME * pVolume);
U32             FS__GetTimeDate               (void);
const char    * FS__GetJournalFileName        (const FS_VOLUME * pVolume);
const char    * FS__FindDirDelimiter          (const char * sFileName);
I16             FS__FError                    (const FS_FILE * pFile);
int             FS__FEof                      (const FS_FILE * pFile);
int             FS__SetFileBufferFlags        (const FS_FILE * pFile, int Flags);
int             FS__SetFileBuffer             (FS_FILE * pFile, void * pData, I32 NumBytes, int Flags);
int             FS__SetEncryptionObject       (const FS_FILE * pFile, FS_CRYPT_OBJ * pCryptObj);
const FS_ACCESS_MODE *
                FS__GetAccessMode             (const char * sMode);
int             FS__GetFileAttr               (const FS_FILE * pFile, U8  * pAttr);
int             FS__SetFileAttr               (const FS_FILE * pFile, U8    Attr);
int             FS__GetFileTime               (const FS_FILE * pFile, U32 * pTimeStamp, int TimeIndex);
int             FS__SetFileTime               (const FS_FILE * pFile, U32   TimeStamp,  int TimeIndex);
int             FS__GetDirAttr                (      FS_DIR  * pDir,  U8  * pAttr);
int             FS__SetDirAttr                (      FS_DIR  * pDir,  U8    Attr);
int             FS__GetDirTime                (      FS_DIR  * pDir,  U32 * pTimeStamp, int TimeIndex);
int             FS__SetDirTime                (      FS_DIR  * pDir,  U32   TimeStamp,  int TimeIndex);
int             FS__TruncateFile              (      FS_FILE * pFile, FS_FILE_SIZE FileSizeNew);
int             FS__BuildFileNameFQ           (FS_VOLUME * pVolume, const char * sFileName, char * sDest, unsigned DestSize);
int             FS__SyncFileNL                (FS_VOLUME * pVolume, FS_FILE * pFile);

/*********************************************************************
*
*       Operations on files (without Global locking)
*/
int             FS_FILE_Open                  (FS_VOLUME * pVolume, const char * sFileName, unsigned AccessFlags, int DoCreate, int DoDel, int DoOpen, FS_FILE ** ppFile);
U32             FS_FILE_Read                  (FS_FILE * pFile,       void * pData, U32 NumBytes);
U32             FS_FILE_Write                 (FS_FILE * pFile, const void * pData, U32 NumBytes);
int             FS_FILE_SetPos                (FS_FILE * pFile, FS_FILE_OFF Off, int Origin);
int             FS_FILE_SetEnd                (FS_FILE * pFile);
int             FS_FILE_SetSize               (FS_FILE * pFile, FS_FILE_SIZE NumBytes);
FS_FILE_SIZE    FS_FILE_GetSize               (FS_FILE * pFile);

#if FS_SUPPORT_FILE_BUFFER

/*********************************************************************
*
*       File buffer related functions, internal
*/
int             FS__FB_Read                   (      FS_FILE * pFile,       void * pData, U32 NumBytes);
int             FS__FB_Write                  (      FS_FILE * pFile, const void * pData, U32 NumBytes);
int             FS__FB_Clean                  (      FS_FILE * pFile);
int             FS__FB_Sync                   (      FS_FILE * pFile);
FS_FILE_SIZE    FS__FB_GetFileSize            (const FS_FILE * pFile);
void            FS__FB_SetFileSize            (const FS_FILE * pFile);

#endif

/*********************************************************************
*
*       Storage layer related functions, internal
*/
void            FS__STORAGE_Sync              (FS_VOLUME * pVolume);
void            FS__STORAGE_SyncNL            (FS_VOLUME * pVolume);
int             FS__STORAGE_RefreshSectors    (FS_VOLUME * pVolume, U32 FirstSector, U32 NumSectors, void * pBuffer, unsigned NumSectorsInBuffer);
int             FS__STORAGE_GetSectorUsage    (FS_VOLUME * pVolume, U32 SectorIndex);
int             FS__STORAGE_GetSectorUsageNL  (FS_VOLUME * pVolume, U32 SectorIndex);
#if FS_SUPPORT_TEST
  int           FS__STORAGE_SetSectorType     (const char * sVolumeName, U8 SectorType);
#endif // FS_SUPPORT_TEST

/*********************************************************************
*
*       API functions, internal (without driver locking)
*/
int             FS__CloseFileNL               (FS_FILE   * pFile);
int             FS__CACHE_CommandDeviceNL     (FS_DEVICE * pDevice, int Cmd, void * pData);
int             FS__IoCtlNL                   (FS_VOLUME * pVolume, I32 Cmd, I32 Aux, void * pBuffer);
void            FS__UnmountForcedNL           (FS_VOLUME * pVolume);
void            FS__UnmountNL                 (FS_VOLUME * pVolume);
int             FS__MountNL                   (FS_VOLUME * pVolume, U8 MountType);
int             FS__AutoMountNL               (FS_VOLUME * pVolume);

/*********************************************************************
*
*       API functions, file handle operations
*/
FS_FILE       * FS__AllocFileHandle           (void);
int             FS__CreateFileHandle          (const FS_VOLUME * pVolume, unsigned AccessFlags, FS_FILE_OBJ * pFileObj, FS_FILE ** ppFile);
void            FS__FreeFileHandle            (FS_FILE * pFile);

/*********************************************************************
*
*       API functions, file object operations
*/
FS_FILE_OBJ   * FS__AllocFileObj              (void);
int             FS__CreateFileObj             (FS_VOLUME * pVolume, const char * sFileName, FS_FILE_OBJ ** ppFileObj);
void            FS__FreeFileObj               (FS_FILE_OBJ * pFileObj);
void            FS__FreeFileObjNL             (FS_FILE_OBJ * pFileObj);

/*********************************************************************
*
*       Disk allocation mode
*/
#if FS_SUPPORT_TEST
  void          FS__SetDiskAllocMode          (int AllocMode);
  int           FS__GetDiskAllocMode          (void);
#endif // FS_SUPPORT_TEST

/*********************************************************************
*
*       Multi-tasking context
*/
void            FS__SaveContext               (      FS_CONTEXT * pContext);
void            FS__RestoreContext            (const FS_CONTEXT * pContext);

/*********************************************************************
*
*       ECC256
*/
int             FS__ECC256_Apply              (U32 * pData, U32 eccRead);
FS_OPTIMIZE
U32             FS__ECC256_Calc               (const U32 * pData);
int             FS__ECC256_IsValid            (U32 ecc);
U32             FS__ECC256_Load               (const U8 * p);
void            FS__ECC256_Store              (U8 * p, U32 ecc);
int             FS__ECC256_Validate           (void);

/*********************************************************************
*
*       Helper functions
*/
U32             FS__DivideU32Up               (      U32  Nom,      U32      Div);
U32             FS__DivModU32                 (      U32  v,        U32      Div,      U32 * pRem);
U32             FS_CRC32_Calc                 (const U8 * pData,    unsigned NumBytes, U32 crc);
FS_OPTIMIZE
U32             FS_CRC32_CalcBitByBit         (const U8 * pData,    unsigned NumBytes, U32 crc, U32 Poly);
int             FS_CRC32_Validate             (void);
#ifdef _WIN32
  void          FS_CRC32_BuildTable           (void);
#endif
U16             FS_CRC16_Calc                 (const U8 * pData,    unsigned NumBytes, U16 crc);
FS_OPTIMIZE
U16             FS_CRC16_CalcBitByBit         (const U8 * pData,    unsigned NumBytes, U16 crc, U16 Poly);
int             FS_CRC16_Validate             (void);
#ifdef _WIN32
  void          FS_CRC16_BuildTable           (void);
#endif
U8              FS_CRC8_Calc                  (const U8 * pData,    unsigned NumBytes, U8 crc);
FS_OPTIMIZE
U8              FS_CRC8_CalcBitByBit          (const U8 * pData,    unsigned NumBytes, U8 crc, U8 Poly);
int             FS_CRC8_Validate              (void);
#ifdef _WIN32
  void          FS_CRC8_BuildTable            (void);
#endif
U32             FS_BITFIELD_CalcSize          (      U32  NumItems, unsigned BitsPerItem);
unsigned        FS_BITFIELD_CalcNumBitsUsed   (      U32  NumItems);
U32             FS_BITFIELD_ReadEntry         (const U8 * pBase,    U32      Index,       unsigned NumBits);
void            FS_BITFIELD_WriteEntry        (      U8 * pBase,    U32      Index,       unsigned NumBits, U32 v);
void            FS_PRNG_Init                  (U16 Value);
U16             FS_PRNG_Generate              (void);
void            FS_PRNG_Save                  (      FS_CONTEXT * pContext);
void            FS_PRNG_Restore               (const FS_CONTEXT * pContext);
#if FS_SUPPORT_TEST
  I32           FS__ReadATEntry               (const char * sVolumeName, U32 ClusterId);
#endif // FS_SUPPORT_TEST
#if (FS_SUPPORT_TEST != 0) && (FS_SUPPORT_DEINIT != 0)
  void          FS__SetOnDeInitCallback       (FS_ON_DEINIT_CALLBACK * pfOnDeInit);
#endif // FS_SUPPORT_TEST != 0 && FS_SUPPORT_DEINIT != 0

#if FS_SUPPORT_TEST

/*********************************************************************
*
*       Memory allocation (testing only)
*/

/*********************************************************************
*
*       FS_TEST_HOOK_MEM_ALLOC_BEGIN
*/
typedef void (FS_TEST_HOOK_MEM_ALLOC_BEGIN)(const char * sDesc, I32 * pNumBytes);

/*********************************************************************
*
*       FS_LB_TEST_HOOK_MEM_ALLOC_END
*/
typedef void (FS_TEST_HOOK_MEM_ALLOC_END)(const char * sDesc, I32 NumBytes, void ** ppData);

void FS__SetTestHookMemAllocBegin(FS_TEST_HOOK_MEM_ALLOC_BEGIN * pfTestHook);
void FS__SetTestHookMemAllocEnd  (FS_TEST_HOOK_MEM_ALLOC_END   * pfTestHook);

#endif // FS_SUPPORT_TEST

/*********************************************************************
*
*       FS_GLOBAL
*/
typedef struct {
  SECTOR_BUFFER         * paSectorBuffer;
  U8                      NumSectorBuffers;
  U16                     MaxSectorSize;
  U8                      IsInited;
  U8                      IsStorageInited;
  FS_FILE               * pFirstFileHandle;
  FS_FILE_OBJ           * pFirstFileObj;
  FS_VOLUME               FirstVolume;
  U8                      NumVolumes;
  FS_WRITEMODE            WriteMode;
#if FS_SUPPORT_TEST
  U8                      AllocMode;
#endif // FS_SUPPORT_TEST
  const char            * sCopyright;            // Reference and link the copyright string for libraries.
#if FS_SUPPORT_DEINIT
  FS_ON_EXIT_CB         * pFirstOnExit;
#if FS_SUPPORT_TEST
  FS_ON_DEINIT_CALLBACK * pfOnDeInit;
#endif // FS_SUPPORT_TEST
#endif // FS_SUPPORT_DEINIT
#if FS_SUPPORT_PROFILE
  FS_PROFILE              Profile;
#endif
#if (FS_SUPPORT_FILE_BUFFER)
  U32                     FileBufferSize;
  U8                      FileBufferFlags;
#endif
#if ((FS_SUPPORT_EFS != 0) && (FS_EFS_SUPPORT_DIRENTRY_BUFFERS != 0))
  SECTOR_BUFFER         * paDirEntryBuffer;
  U8                      NumDirEntryBuffers;
#endif
  FS_MEM_MANAGER          MemManager;
} FS_GLOBAL;

extern FS_GLOBAL FS_Global;

/*********************************************************************
*
*       OS mapping macros (multi tasking locks)
*
* Notes
*   These macros map to locking routines or are empty,
*   depending on the configuration
*   There are 3 different lock-levels:
*   FS_OS == 0                     -> No locking
*   FS_OS == 1
*     FS_OS_LOCK_PER_DRIVER == 0   -> Single, global lock in every API function
*     FS_OS_LOCK_PER_DRIVER == 1   -> Multiple locks
*/
#if (FS_OS == 0)
  //
  // No locking
  //
  #define FS_LOCK()
  #define FS_UNLOCK()
  #define FS_LOCK_SYS()
  #define FS_UNLOCK_SYS()
  #define FS_OS_INIT(MaxNumLocks)
  #define FS_OS_DEINIT()
  #define FS_LOCK_DRIVER(pDriver)
  #define FS_UNLOCK_DRIVER(pDriver)
  #define FS_OS_ADD_DRIVER(pDevice)
  #define FS_OS_REMOVE_DRIVER(pDevice)
  #define FS_OS_GETNUM_DRIVERLOCKS()          0u
  #define FS_OS_GETNUM_SYSLOCKS()             0u
  #define FS_OS_WAIT(Timeout)
  #define FS_OS_SIGNAL()
#elif (FS_OS != 0) && (FS_OS_LOCK_PER_DRIVER == 0)
  //
  // Coarse lock granularity:
  //   One global lock for all FS API functions
  //
  #define FS_LOCK_ID_SYSTEM                   0
  #define FS_LOCK()                           FS_OS_Lock(FS_LOCK_ID_SYSTEM)
  #define FS_UNLOCK()                         FS_OS_Unlock(FS_LOCK_ID_SYSTEM)
  #define FS_LOCK_SYS()
  #define FS_UNLOCK_SYS()
  #define FS_LOCK_DRIVER(pDriver)
  #define FS_UNLOCK_DRIVER(pDriver)
  #define FS_OS_ADD_DRIVER(pDevice)
  #define FS_OS_REMOVE_DRIVER(pDevice)
  #define FS_OS_GETNUM_DRIVERLOCKS()          0u
  #define FS_OS_GETNUM_SYSLOCKS()             1u
  #define FS_OS_INIT(MaxNumLocks)             FS_OS_Init(MaxNumLocks)
  #define FS_OS_DEINIT()                      FS_OS_DeInit()
  #define FS_OS_WAIT(Timeout)                 FS_X_OS_Wait(Timeout)
  #define FS_OS_SIGNAL()                      FS_X_OS_Signal()
#else
  //
  // Fine lock granularity:
  //   Lock for different FS functions
  //
  #define FS_LOCK_ID_SYSTEM                   0u
  #define FS_LOCK_ID_DEVICE                   1u
  #define FS_LOCK()
  #define FS_UNLOCK()
  #define FS_LOCK_SYS()                       FS_OS_Lock(FS_LOCK_ID_SYSTEM)
  #define FS_UNLOCK_SYS()                     FS_OS_Unlock(FS_LOCK_ID_SYSTEM)
  #define FS_LOCK_DRIVER(pDevice)             FS_OS_LockDriver(pDevice)
  #define FS_UNLOCK_DRIVER(pDevice)           FS_OS_UnlockDriver(pDevice)
  #define FS_OS_GETNUM_DRIVERLOCKS()          FS_OS_GetNumDriverLocks()
  #define FS_OS_GETNUM_SYSLOCKS()             1u
  #define FS_OS_ADD_DRIVER(pDevice)           FS_OS_AddDriver(pDevice)
  #define FS_OS_REMOVE_DRIVER(pDevice)        FS_OS_RemoveDriver(pDevice)
  #define FS_OS_INIT(MaxNumLocks)             FS_OS_Init(MaxNumLocks)
  #define FS_OS_DEINIT()                      FS_OS_DeInit()
  #define FS_OS_WAIT(Timeout)                 FS_X_OS_Wait(Timeout)
  #define FS_OS_SIGNAL()                      FS_X_OS_Signal()
  void     FS_OS_LockDriver       (const FS_DEVICE * pDevice);
  void     FS_OS_UnlockDriver     (const FS_DEVICE * pDevice);
  unsigned FS_OS_GetNumDriverLocks(void);
  void     FS_OS_AddDriver        (const FS_DEVICE_TYPE * pDriver);
  void     FS_OS_RemoveDriver     (const FS_DEVICE_TYPE * pDriver);
#endif

void FS_OS_Lock  (unsigned LockIndex);
void FS_OS_Unlock(unsigned LockIndex);
void FS_OS_Init  (unsigned NumLocks);
void FS_OS_DeInit(void);

/*********************************************************************
*
*       FS_JOURNAL
*
*       Journal to make file system layer transaction and fail safe
*/
typedef void (FS_JOURNAL_TEST_HOOK)(U8 Unit);

/*********************************************************************
*
*       FS_JOURNAL_LAYOUT
*
*  Description
*    Layout of the data in the journal file.
*/
typedef struct {
  U32 SectorIndexInfo;
  U32 SectorIndexStatus;
  U32 SectorIndexList;
  U32 SectorIndexData;
} FS_JOURNAL_LAYOUT;

void FS__JOURNAL_DeInit           (const FS_VOLUME * pVolume);
int  FS__JOURNAL_GetNumFreeSectors(FS_VOLUME * pVolume);
int  FS__JOURNAL_IsPresent        (const FS_DEVICE * pDevice);
int  FS__JOURNAL_Mount            (FS_VOLUME * pVolume);
int  FS__JOURNAL_Read             (const FS_DEVICE * pDevice, U32 SectorIndex, void       * pBuffer, U32 NumSectors);
int  FS__JOURNAL_Write            (const FS_DEVICE * pDevice, U32 SectorIndex, const void * pBuffer, U32 NumSectors, U8 RepeatSame);
int  FS__JOURNAL_Begin            (FS_VOLUME * pVolume);
int  FS__JOURNAL_End              (FS_VOLUME * pVolume);
void FS__JOURNAL_Invalidate       (FS_VOLUME * pVolume);
int  FS__JOURNAL_Clean            (FS_VOLUME * pVolume);
int  FS__JOURNAL_FreeSectors      (const FS_DEVICE * pDevice, U32 SectorIndex, U32 NumSectors);
int  FS__JOURNAL_GetOpenCnt       (FS_VOLUME * pVolume);
void FS__JOURNAL_Save             (FS_CONTEXT * pContext);
void FS__JOURNAL_Restore          (const FS_CONTEXT * pContext);
void FS__JOURNAL_SetError         (FS_VOLUME * pVolume, int Error);
#if FS_SUPPORT_TEST
  int  FS__JOURNAL_GetLayout      (const char * sVolumeName, FS_JOURNAL_LAYOUT * pFileInfo);
  void FS__JOURNAL_SetTestHook    (FS_JOURNAL_TEST_HOOK * pfTestHook);
#endif

#if FS_SUPPORT_JOURNAL
  #define  FS_JOURNAL_MOUNT(pVolume)                  FS__JOURNAL_Mount(pVolume)
  #define  FS_JOURNAL_INVALIDATE(pVolume)             FS__JOURNAL_Invalidate(pVolume)
  #define  FS_JOURNAL_CLEAN(pVolume)                  FS__JOURNAL_Clean(pVolume)
#else
  #define  FS_JOURNAL_MOUNT(pVolume)                  0
  #define  FS_JOURNAL_INVALIDATE(pVolume)
  #define  FS_JOURNAL_CLEAN(pVolume)                  0
#endif

#if (FS_SUPPORT_DEINIT != 0) && (FS_SUPPORT_JOURNAL != 0)
  #define  FS_JOURNAL_DEINIT(pVolume)                 FS__JOURNAL_DeInit(pVolume)
#else
  #define  FS_JOURNAL_DEINIT(pVolume)
#endif

/*********************************************************************
*
*       API mapping macros
*
*  Description
*    These macros map to the functions of the file system (Currently FAT or EFS)
*    or - in case of multiple file systems - to a mapping layer, which calls the
*    appropriate function depending on the file system of the volume
*/
#if (FS_SUPPORT_FAT != 0) && (FS_SUPPORT_EFS == 0)
  #define FS_CHECK_INFOSECTOR(pVolume)                                                                             FS_FAT_CheckBPB(pVolume)
  #define FS_CLOSEDIR(pDir)                                                                                        FS_FAT_CloseDir(pDir);
  #define FS_CREATE_DIR(pVolume, sDirName)                                                                         FS_FAT_CreateDir(pVolume, sDirName)
  #define FS_CLOSE_FILE(pFile)                                                                                     FS_FAT_CloseFile(pFile)
  #define FS_OPEN_FILE(sFileName, pFile, DoDel, DoOpen, DoCreate)                                                  FS_FAT_OpenFile(sFileName, pFile, DoDel, DoOpen, DoCreate)
  #define FS_FORMAT(pVolume, pFormatInfo)                                                                          FS_FAT_Format(pVolume, pFormatInfo)
  #define FS_FREAD(pFile, pData, NumBytes)                                                                         FS_FAT_Read(pFile, pData, NumBytes)
  #define FS_FWRITE(pFile, pData, NumBytes)                                                                        FS_FAT_Write(pFile, pData, NumBytes)
  #define FS_GET_DIRENTRY_INFO(pVolume, sName, p, Mask)                                                            FS_FAT_GetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_GET_DISKINFO(pVolume, pInfo, Flags)                                                                   FS_FAT_GetDiskInfo(pVolume, pInfo, Flags)
  #define FS_MOVE(pVolume, sOldName, sNewName)                                                                     FS_FAT_Move(pVolume, sOldName, sNewName)
  #define FS_OPENDIR(s, pDirHandle)                                                                                FS_FAT_OpenDir(s, pDirHandle)
  #define FS_READDIR(pDir, pDirEntryInfo)                                                                          FS_FAT_ReadDir(pDir, pDirEntryInfo)
  #define FS_REMOVE_DIR(pVolume, sDirName)                                                                         FS_FAT_RemoveDir(pVolume, sDirName)
  #define FS_DELETE_DIR(pVolume, sDirName, MaxRecursionLevel)                                                      FS_FAT_DeleteDir(pVolume, sDirName, MaxRecursionLevel)
  #define FS_RENAME(pVolume, sOldName, sNewName)                                                                   FS_FAT_Rename(pVolume, sOldName, sNewName)
  #define FS_SET_DIRENTRY_INFO(pVolume, sName, p, Mask)                                                            FS_FAT_SetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_SET_END_OF_FILE(pFile)                                                                                FS_FAT_SetEndOfFile(pFile)
  #define FS_CLEAN_FS(pVolume)                                                                                     FS_FAT_Clean(pVolume)
  #define FS_GET_VOLUME_LABEL(pVolume, pVolLabel, VolLabelSize)                                                    FS_FAT_GetVolumeLabel(pVolume, pVolLabel, VolLabelSize)
  #define FS_SET_VOLUME_LABEL(pVolume, pVolLabel)                                                                  FS_FAT_SetVolumeLabel(pVolume, pVolLabel)
  #define FS_CREATE_JOURNAL_FILE(pVolume, NumBytes, pFirstS, pNumS)                                                FS_FAT_CreateJournalFile(pVolume, NumBytes, pFirstS, pNumS)
  #define FS_OPEN_JOURNAL_FILE(pVolume)                                                                            FS_FAT_OpenJournalFile(pVolume)
  #define FS_GET_INDEX_OF_LAST_SECTOR(pVolume)                                                                     FS_FAT_GetIndexOfLastSector(pVolume)
  #define FS_CHECK_VOLUME(pVolume, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)                              FS_FAT_CheckVolume(pVolume, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
  #define FS_UPDATE_FILE(pFile)                                                                                    FS_FAT_CloseFile(pFile)
  #define FS_SET_FILE_SIZE(pFile, NumBytes)                                                                        FS_FAT_SetFileSize(pFile, NumBytes)
  #define FS_FREE_SECTORS(pVolume)                                                                                 FS_FAT_FreeSectors(pVolume)
  #define FS_GET_FREE_SPACE(pVolume, pBuffer, SizeOfBuffer, FirstClusterId, pNumClustersFree, pNumClustersChecked) FS_FAT_GetFreeSpace(pVolume, pBuffer, SizeOfBuffer, FirstClusterId, pNumClustersFree, pNumClustersChecked)
  #define FS_GET_AT_INFO(pVolume, pATInfo)                                                                         FS_FAT_GetATInfo(pVolume, pATInfo);
  #define FS_CHECK_DIR(pVolume, sPath, pClusterMap, pfOnError)                                                     FS_FAT_CheckDir(pVolume, sPath, pClusterMap, pfOnError);
  #define FS_CHECK_AT(pVolume, pClusterMap, pfOnError)                                                             FS_FAT_CheckAT(pVolume, pClusterMap, pfOnError)
  #define FS_READ_AT_ENTRY(pVolume, ClusterId)                                                                     FS_FAT_ReadATEntry(pVolume, ClusterId)
  #define FS_GET_DIRENTRY_INFO_EX(pVolume, pDirEntryPos, p, Mask)                                                  FS_FAT_GetDirEntryInfoEx(pVolume, pDirEntryPos, p, Mask)
  #define FS_SET_DIRENTRY_INFO_EX(pVolume, pDirEntryPos, p, Mask)                                                  FS_FAT_SetDirEntryInfoEx(pVolume, pDirEntryPos, p, Mask)
#elif (FS_SUPPORT_FAT == 0) && (FS_SUPPORT_EFS != 0)
  #define FS_CHECK_INFOSECTOR(pVolume)                                                                             FS_EFS_CheckInfoSector(pVolume)
  #define FS_CLOSEDIR(pDir)                                                                                        FS_EFS_CloseDir(pDir)
  #define FS_CREATE_DIR(pVolume, sDirName)                                                                         FS_EFS_CreateDir(pVolume, sDirName)
  #define FS_CLOSE_FILE(pFile)                                                                                     FS_EFS_CloseFile(pFile)
  #define FS_OPEN_FILE(sFileName, pFile, DoDel, DoOpen, DoCreate)                                                  FS_EFS_OpenFile(sFileName, pFile, DoDel, DoOpen, DoCreate)
  #define FS_FORMAT(pVolume, pFormatInfo)                                                                          FS_EFS_Format(pVolume, pFormatInfo)
  #define FS_FREAD(pFile, pData, NumBytes)                                                                         FS_EFS_Read(pFile, pData, NumBytes)
  #define FS_FWRITE(pFile, pData, NumBytes)                                                                        FS_EFS_Write(pFile, pData, NumBytes)
  #define FS_GET_DIRENTRY_INFO(pVolume, sName, p, Mask)                                                            FS_EFS_GetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_GET_DISKINFO(pVolume, pInfo, Flags)                                                                   FS_EFS_GetDiskInfo(pVolume, pInfo, Flags)
  #define FS_MOVE(pVolume, sOldName, sNewName)                                                                     FS_EFS_Move(pVolume, sOldName, sNewName)
  #define FS_OPENDIR(s, pDirHandle)                                                                                FS_EFS_OpenDir(s, pDirHandle)
  #define FS_READDIR(pDir, pDirEntryInfo)                                                                          FS_EFS_ReadDir(pDir, pDirEntryInfo)
  #define FS_REMOVE_DIR(pVolume, sDirName)                                                                         FS_EFS_RemoveDir(pVolume, sDirName)
  #define FS_DELETE_DIR(pVolume, sDirName, MaxRecursionLevel)                                                      FS_EFS_DeleteDir(pVolume, sDirName, MaxRecursionLevel)
  #define FS_RENAME(pVolume, sOldName, sNewName)                                                                   FS_EFS_Rename(pVolume, sOldName, sNewName)
  #define FS_SET_DIRENTRY_INFO(pVolume, sName, p, Mask)                                                            FS_EFS_SetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_SET_END_OF_FILE(pFile)                                                                                FS_EFS_SetEndOfFile(pFile)
  #define FS_CLEAN_FS(pVolume)                                                                                     FS_EFS_Clean(pVolume)
  #define FS_GET_VOLUME_LABEL(pVolume, pVolLabel, VolLabelSize)                                                    FS_EFS_GetVolumeLabel(pVolume, pVolLabel, VolLabelSize)
  #define FS_SET_VOLUME_LABEL(pVolume, pVolLabel)                                                                  FS_EFS_SetVolumeLabel(pVolume, pVolLabel)
  #define FS_CREATE_JOURNAL_FILE(pVolume, NumBytes, pFirstS, pNumS)                                                FS_EFS_CreateJournalFile(pVolume, NumBytes, pFirstS, pNumS)
  #define FS_OPEN_JOURNAL_FILE(pVolume)                                                                            FS_EFS_OpenJournalFile(pVolume)
  #define FS_GET_INDEX_OF_LAST_SECTOR(pVolume)                                                                     FS_EFS_GetIndexOfLastSector(pVolume)
  #define FS_CHECK_VOLUME(pVolume, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)                              FS_EFS_CheckVolume(pVolume, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
  #define FS_UPDATE_FILE(pFile)                                                                                    FS_EFS_CloseFile(pFile)
  #define FS_SET_FILE_SIZE(pFile, NumBytes)                                                                        FS_EFS_SetFileSize(pFile, NumBytes)
  #define FS_FREE_SECTORS(pVolume)                                                                                 FS_EFS_FreeSectors(pVolume)
  #define FS_GET_FREE_SPACE(pVolume, pBuffer, SizeOfBuffer, FirstClusterId, pNumClustersFree, pNumClustersChecked) FS_EFS_GetFreeSpace(pVolume, pBuffer, SizeOfBuffer, FirstClusterId, pNumClustersFree, pNumClustersChecked)
  #define FS_GET_AT_INFO(pVolume, pATInfo)                                                                         FS_EFS_GetATInfo(pVolume, pATInfo);
  #define FS_CHECK_DIR(pVolume, sPath, pClusterMap, pfOnError)                                                     FS_EFS_CheckDir(pVolume, sPath, pClusterMap, pfOnError);
  #define FS_CHECK_AT(pVolume, pClusterMap, pfOnError)                                                             FS_EFS_CheckAT(pVolume, pClusterMap, pfOnError)
  #define FS_READ_AT_ENTRY(pVolume, ClusterId)                                                                     FS_EFS_ReadATEntry(pVolume, ClusterId)
  #define FS_GET_DIRENTRY_INFO_EX(pVolume, pDirEntryPos, p, Mask)                                                  FS_EFS_GetDirEntryInfoEx(pVolume, pDirEntryPos, p, Mask)
  #define FS_SET_DIRENTRY_INFO_EX(pVolume, pDirEntryPos, p, Mask)                                                  FS_EFS_SetDirEntryInfoEx(pVolume, pDirEntryPos, p, Mask)
#elif (FS_SUPPORT_FAT != 0) && (FS_SUPPORT_EFS != 0)
  #define FS_CHECK_INFOSECTOR(pVolume)                                                                             FS_MAP_CheckFS_API(pVolume)
  #define FS_CLOSEDIR(pDir)                                                                                        FS_MAP_CloseDir(pDir)
  #define FS_CREATE_DIR(pVolume, sDirName)                                                                         FS_MAP_CreateDir(pVolume, sDirName)
  #define FS_CLOSE_FILE(pFile)                                                                                     FS_MAP_CloseFile(pFile)
  #define FS_OPEN_FILE(sFileName, pFile, DoDel, DoOpen, DoCreate)                                                  FS_MAP_OpenFile(sFileName, pFile, DoDel, DoOpen, DoCreate)
  #define FS_FORMAT(pVolume, pFormatInfo)                                                                          FS_MAP_Format(pVolume, pFormatInfo)
  #define FS_FREAD(pFile, pData, NumBytes)                                                                         FS_MAP_Read(pFile, pData, NumBytes)
  #define FS_FWRITE(pFile, pData, NumBytes)                                                                        FS_MAP_Write(pFile, pData, NumBytes)
  #define FS_GET_DIRENTRY_INFO(pVolume, sName, p, Mask)                                                            FS_MAP_GetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_GET_DISKINFO(pVolume, pInfo, Flags)                                                                   FS_MAP_GetDiskInfo(pVolume, pInfo, Flags)
  #define FS_MOVE(pVolume, sOldName, sNewName)                                                                     FS_MAP_Move(pVolume, sOldName, sNewName)
  #define FS_OPENDIR(s, pDirHandle)                                                                                FS_MAP_OpenDir(s, pDirHandle)
  #define FS_READDIR(pDir, pDirEntryInfo)                                                                          FS_MAP_ReadDir(pDir, pDirEntryInfo)
  #define FS_REMOVE_DIR(pVolume, sDirName)                                                                         FS_MAP_RemoveDir(pVolume, sDirName)
  #define FS_DELETE_DIR(pVolume, sDirName, MaxRecursionLevel)                                                      FS_MAP_DeleteDir(pVolume, sDirName, MaxRecursionLevel)
  #define FS_RENAME(pVolume, sOldName, sNewName)                                                                   FS_MAP_Rename(pVolume, sOldName, sNewName)
  #define FS_SET_DIRENTRY_INFO(pVolume, sName, p, Mask)                                                            FS_MAP_SetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_SET_END_OF_FILE(pFile)                                                                                FS_MAP_SetEndOfFile(pFile)
  #define FS_CLEAN_FS(pVolume)                                                                                     FS_MAP_Clean(pVolume)
  #define FS_GET_VOLUME_LABEL(pVolume, pVolLabel, VolLabelSize)                                                    FS_MAP_GetVolumeLabel(pVolume, pVolLabel, VolLabelSize)
  #define FS_SET_VOLUME_LABEL(pVolume, pVolLabel)                                                                  FS_MAP_SetVolumeLabel(pVolume, pVolLabel)
  #define FS_CREATE_JOURNAL_FILE(pVolume, NumBytes, pFirstS, pNumS)                                                FS_MAP_CreateJournalFile(pVolume, NumBytes, pFirstS, pNumS)
  #define FS_OPEN_JOURNAL_FILE(pVolume)                                                                            FS_MAP_OpenJournalFile(pVolume)
  #define FS_GET_INDEX_OF_LAST_SECTOR(pVolume)                                                                     FS_MAP_GetIndexOfLastSector(pVolume)
  #define FS_CHECK_VOLUME(pVolume, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)                              FS_MAP_CheckVolume(pVolume, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
  #define FS_UPDATE_FILE(pFile)                                                                                    FS_MAP_UpdateFile(pFile)
  #define FS_SET_FILE_SIZE(pFile, NumBytes)                                                                        FS_MAP_SetFileSize(pFile, NumBytes)
  #define FS_FREE_SECTORS(pVolume)                                                                                 FS_MAP_FreeSectors(pVolume)
  #define FS_GET_FREE_SPACE(pVolume, pBuffer, SizeOfBuffer, FirstClusterId, pNumClustersFree, pNumClustersChecked) FS_MAP_GetFreeSpace(pVolume, pBuffer, SizeOfBuffer, FirstClusterId, pNumClustersFree, pNumClustersChecked)
  #define FS_GET_AT_INFO(pVolume, pATInfo)                                                                         FS_MAP_GetATInfo(pVolume, pATInfo);
  #define FS_CHECK_DIR(pVolume, sPath, pClusterMap, pfOnError)                                                     FS_MAP_CheckDir(pVolume, sPath, pClusterMap, pfOnError);
  #define FS_CHECK_AT(pVolume, pClusterMap, pfOnError)                                                             FS_MAP_CheckAT(pVolume, pClusterMap, pfOnError)
  #define FS_READ_AT_ENTRY(pVolume, ClusterId)                                                                     FS_MAP_ReadATEntry(pVolume, ClusterId)
  #define FS_GET_DIRENTRY_INFO_EX(pVolume, pDirEntryPos, p, Mask)                                                  FS_MAP_GetDirEntryInfoEx(pVolume, pDirEntryPos, p, Mask)
  #define FS_SET_DIRENTRY_INFO_EX(pVolume, pDirEntryPos, p, Mask)                                                  FS_MAP_SetDirEntryInfoEx(pVolume, pDirEntryPos, p, Mask)
#else
  #define FS_CHECK_INFOSECTOR(pVolume)                                                                             FS_NONE_CheckFS_API(pVolume)
  #define FS_CLOSEDIR(pDir)                                                                                        FS_NONE_CloseDir(pDir)
  #define FS_CREATE_DIR(pVolume, sDirName)                                                                         FS_NONE_CreateDir(pVolume, sDirName)
  #define FS_CLOSE_FILE(pFile)                                                                                     FS_NONE_CloseFile(pFile)
  #define FS_OPEN_FILE(sFileName, pFile, DoDel, DoOpen, DoCreate)                                                  FS_NONE_OpenFile(sFileName, pFile, DoDel, DoOpen, DoCreate)
  #define FS_FORMAT(pVolume, pFormatInfo)                                                                          FS_NONE_Format(pVolume, pFormatInfo)
  #define FS_FREAD(pFile, pData, NumBytes)                                                                         FS_NONE_Read(pFile, pData, NumBytes)
  #define FS_FWRITE(pFile, pData, NumBytes)                                                                        FS_NONE_Write(pFile, pData, NumBytes)
  #define FS_GET_DIRENTRY_INFO(pVolume, sName, p, Mask)                                                            FS_NONE_GetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_GET_DISKINFO(pVolume, pInfo, Flags)                                                                   FS_NONE_GetDiskInfo(pVolume, pInfo, Flags)
  #define FS_MOVE(pVolume, sOldName, sNewName)                                                                     FS_NONE_Move(pVolume, sOldName, sNewName)
  #define FS_OPENDIR(s, pDirHandle)                                                                                FS_NONE_OpenDir(s, pDirHandle)
  #define FS_READDIR(pDir, pDirEntryInfo)                                                                          FS_NONE_ReadDir(pDir, pDirEntryInfo)
  #define FS_REMOVE_DIR(pVolume, sDirName)                                                                         FS_NONE_RemoveDir(pVolume, sDirName)
  #define FS_DELETE_DIR(pVolume, s, MaxRecursionLevel)                                                             FS_NONE_DeleteDir(pVolume, sDirName, MaxRecursionLevel)
  #define FS_RENAME(pVolume, sOldName, sNewName)                                                                   FS_NONE_Rename(pVolume, sOldName, sNewName)
  #define FS_SET_DIRENTRY_INFO(pVolume, sName, p, Mask)                                                            FS_NONE_SetDirEntryInfo(pVolume, sName, p, Mask)
  #define FS_SET_END_OF_FILE(pFile)                                                                                FS_NONE_SetEndOfFile(pFile)
  #define FS_CLEAN_FS(pVolume)                                                                                     FS_NONE_Clean(pVolume)
  #define FS_GET_VOLUME_LABEL(pVolume, pVolLabel, VolLabelSize)                                                    FS_NONE_GetVolumeLabel(pVolume, pVolLabel, VolLabelSize)
  #define FS_SET_VOLUME_LABEL(pVolume, pVolLabel)                                                                  FS_NONE_SetVolumeLabel(pVolume, pVolLabel)
  #define FS_CREATE_JOURNAL_FILE(pVolume, NumBytes, pFirstS, pNumS)                                                FS_NONE_CreateJournalFile(pVolume, NumBytes, pFirstS, pNumS)
  #define FS_OPEN_JOURNAL_FILE(pVolume)                                                                            FS_NONE_OpenJournalFile(pVolume)
  #define FS_GET_INDEX_OF_LAST_SECTOR(pVolume)                                                                     FS_NONE_GetIndexOfLastSector(pVolume)
  #define FS_CHECK_VOLUME(pVolume, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)                              FS_NONE_CheckVolume(pVolume, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
  #define FS_UPDATE_FILE(pFile)                                                                                    FS_NONE_UpdateFile(pFile)
  #define FS_SET_FILE_SIZE(pFile, NumBytes)                                                                        FS_NONE_SetFileSize(pFile, NumBytes)
  #define FS_FREE_SECTORS(pVolume)                                                                                 FS_NONE_FreeSectors(pVolume)
  #define FS_GET_FREE_SPACE(pVolume, pBuffer, SizeOfBuffer, FirstClusterId, pNumClustersFree, pNumClustersChecked) FS_NONE_GetFreeSpace(pVolume, pBuffer, SizeOfBuffer, FirstClusterId, pNumClustersFree, pNumClustersChecked)
  #define FS_GET_AT_INFO(pVolume, pATInfo)                                                                         FS_NONE_GetATInfo(pVolume, pATInfo);
  #define FS_CHECK_DIR(pVolume, sPath, pClusterMap, pfOnError)                                                     FS_NONE_CheckDir(pVolume, sPath, pClusterMap, pfOnError);
  #define FS_CHECK_AT(pVolume, pClusterMap, pfOnError)                                                             FS_NONE_CheckAT(pVolume, pClusterMap, pfOnError)
  #define FS_READ_AT_ENTRY(pVolume, ClusterId)                                                                     FS_NONE_ReadATEntry(pVolume, ClusterId)
  #define FS_GET_DIRENTRY_INFO_EX(pVolume, pDirEntryPos, p, Mask)                                                  FS_NONE_GetDirEntryInfoEx(pVolume, pDirEntryPos, p, Mask)
  #define FS_SET_DIRENTRY_INFO_EX(pVolume, pDirEntryPos, p, Mask)                                                  FS_NONE_SetDirEntryInfoEx(pVolume, pDirEntryPos, p, Mask)
#endif

/*********************************************************************
*
*       API mapping macros for the file access
*/
#define FS_FILE_OPEN(pVolume, sFileName, AccessFlags, DoCreate, DoDel, DoOpen, ppFile)                              FS_FILE_Open(pVolume, sFileName, AccessFlags, DoCreate, DoDel, DoOpen, ppFile)
#define FS_FILE_READ(pFile, pData, NumBytes)                                                                        FS_FILE_Read(pFile, pData, NumBytes)
#define FS_FILE_WRITE(pFile, pData, NumBytes)                                                                       FS_FILE_Write(pFile, pData, NumBytes)
#define FS_FILE_SET_POS(pFile, Off, Origin)                                                                         FS_FILE_SetPos(pFile, Off, Origin)
#define FS_FILE_SET_END(pFile)                                                                                      FS_FILE_SetEnd(pFile)
#define FS_FILE_SET_SIZE(pFile, NumBytes)                                                                           FS_FILE_SetSize(pFile, NumBytes)
#define FS_FILE_GET_SIZE(pFile)                                                                                     FS_FILE_GetSize(pFile)

/*********************************************************************
*
*       API mapping macros for file system access
*/
int         FS_MAP_CloseFile           (      FS_FILE    * pFile);
int         FS_MAP_CheckFS_API         (      FS_VOLUME  * pVolume);
U32         FS_MAP_Read                (      FS_FILE    * pFile,           void  * pData, U32 NumBytes);
U32         FS_MAP_Write               (      FS_FILE    * pFile,     const void  * pData, U32 NumBytes);
int         FS_MAP_OpenFile            (const char       * sFileName, FS_FILE * pFile, int DoDel, int DoOpen, int DoCreate);
int         FS_MAP_Format              (      FS_VOLUME  * pVolume,   const FS_FORMAT_INFO * pFormatInfo);
int         FS_MAP_OpenDir             (const char       * pDirName,  FS_DIR_OBJ * pDirObj);
int         FS_MAP_CloseDir            (      FS_DIR_OBJ * pDirObj);
int         FS_MAP_ReadDir             (      FS_DIR_OBJ * pDirObj,   FS_DIRENTRY_INFO * pDirEntryInfo);
int         FS_MAP_RemoveDir           (      FS_VOLUME  * pVolume,   const char * sDirName);
int         FS_MAP_CreateDir           (      FS_VOLUME  * pVolume,   const char * sDirName);
int         FS_MAP_Rename              (      FS_VOLUME  * pVolume,   const char * sOldName, const char * sNewName);
int         FS_MAP_DeleteDir           (      FS_VOLUME  * pVolume,   const char * sDirName, int MaxRecursionLevel);
int         FS_MAP_Move                (      FS_VOLUME  * pVolume,   const char * sOldName, const char * sNewName);
int         FS_MAP_SetDirEntryInfo     (      FS_VOLUME  * pVolume,   const char * sName, const void * p, int Mask);
int         FS_MAP_GetDirEntryInfo     (      FS_VOLUME  * pVolume,   const char * sName,       void * p, int Mask);
int         FS_MAP_SetEndOfFile        (      FS_FILE    * pFile);
void        FS_MAP_Clean               (      FS_VOLUME  * pVolume);
int         FS_MAP_GetDiskInfo         (      FS_VOLUME  * pVolume,   FS_DISK_INFO * pDiskData, int Flags);
int         FS_MAP_GetVolumeLabel      (      FS_VOLUME  * pVolume,   char * pVolumeLabel, unsigned VolumeLabelSize);
int         FS_MAP_SetVolumeLabel      (      FS_VOLUME  * pVolume,   const char * pVolumeLabel);
int         FS_MAP_CreateJournalFile   (      FS_VOLUME  * pVolume,   U32 NumBytes, U32 * pFirstSector, U32 * pNumSectors);
int         FS_MAP_OpenJournalFile     (      FS_VOLUME  * pVolume);
U32         FS_MAP_GetIndexOfLastSector(      FS_VOLUME  * pVolume);
int         FS_MAP_CheckVolume         (      FS_VOLUME  * pVolume,   void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
int         FS_MAP_UpdateFile          (      FS_FILE    * pFile);
int         FS_MAP_SetFileSize         (      FS_FILE    * pFile,     U32 NumBytes);
int         FS_MAP_FreeSectors         (      FS_VOLUME  * pVolume);
int         FS_MAP_GetFreeSpace        (      FS_VOLUME  * pVolume,   void * pBuffer, int SizeOfBuffer, U32 FirstClusterId, U32 * pNumClustersFree, U32 * pNumClustersChecked);
int         FS_MAP_GetATInfo           (      FS_VOLUME  * pVolume,   FS_AT_INFO * pATInfo);
int         FS_MAP_CheckDir            (      FS_VOLUME  * pVolume,   const char * sPath, FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
int         FS_MAP_CheckAT             (      FS_VOLUME  * pVolume,   const FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
I32         FS_MAP_ReadATEntry         (      FS_VOLUME  * pVolume,   U32 ClusterId);
int         FS_MAP_SetDirEntryInfoEx   (      FS_VOLUME  * pVolume,   const FS_DIRENTRY_POS * pDirEntryPos, const void * p, int Mask);
int         FS_MAP_GetDirEntryInfoEx   (      FS_VOLUME  * pVolume,   const FS_DIRENTRY_POS * pDirEntryPos,       void * p, int Mask);
int         FS_MAP_GetFSType           (const FS_VOLUME  * pVolume);

/*********************************************************************
*
*       Dummy functions for a configuration without file system
*/
int         FS_NONE_CloseFile           (      FS_FILE    * pFile);
int         FS_NONE_CheckFS_API         (      FS_VOLUME  * pVolume);
U32         FS_NONE_Read                (      FS_FILE    * pFile,           void  * pData, U32 NumBytes);
U32         FS_NONE_Write               (      FS_FILE    * pFile,     const void  * pData, U32 NumBytes);
int         FS_NONE_OpenFile            (const char       * sFileName, FS_FILE * pFile, int DoDel, int DoOpen, int DoCreate);
int         FS_NONE_Format              (      FS_VOLUME  * pVolume,   const FS_FORMAT_INFO * pFormatInfo);
int         FS_NONE_OpenDir             (const char       * pDirName,  FS_DIR_OBJ * pDirObj);
int         FS_NONE_CloseDir            (      FS_DIR_OBJ * pDirObj);
int         FS_NONE_ReadDir             (      FS_DIR_OBJ * pDirObj,   FS_DIRENTRY_INFO * pDirEntryInfo);
int         FS_NONE_RemoveDir           (      FS_VOLUME  * pVolume,   const char * sDirName);
int         FS_NONE_CreateDir           (      FS_VOLUME  * pVolume,   const char * sDirName);
int         FS_NONE_Rename              (      FS_VOLUME  * pVolume,   const char * sOldName, const char * sNewName);
int         FS_NONE_DeleteDir           (      FS_VOLUME  * pVolume,   const char * sDirName, int MaxRecursionLevel);
int         FS_NONE_Move                (      FS_VOLUME  * pVolume,   const char * sOldName, const char * sNewName);
int         FS_NONE_SetDirEntryInfo     (      FS_VOLUME  * pVolume,   const char * sName, const void * p, int Mask);
int         FS_NONE_GetDirEntryInfo     (      FS_VOLUME  * pVolume,   const char * sName,       void * p, int Mask);
int         FS_NONE_SetEndOfFile        (      FS_FILE    * pFile);
void        FS_NONE_Clean               (      FS_VOLUME  * pVolume);
int         FS_NONE_GetDiskInfo         (      FS_VOLUME  * pVolume,   FS_DISK_INFO * pDiskData, int Flags);
int         FS_NONE_GetVolumeLabel      (      FS_VOLUME  * pVolume,   char * pVolumeLabel, unsigned VolumeLabelSize);
int         FS_NONE_SetVolumeLabel      (      FS_VOLUME  * pVolume,   const char * pVolumeLabel);
int         FS_NONE_CreateJournalFile   (      FS_VOLUME  * pVolume,   U32 NumBytes, U32 * pFirstSector, U32 * pNumSectors);
int         FS_NONE_OpenJournalFile     (      FS_VOLUME  * pVolume);
U32         FS_NONE_GetIndexOfLastSector(      FS_VOLUME  * pVolume);
int         FS_NONE_CheckVolume         (      FS_VOLUME  * pVolume,   void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
int         FS_NONE_UpdateFile          (      FS_FILE    * pFile);
int         FS_NONE_SetFileSize         (      FS_FILE    * pFile,     U32 NumBytes);
int         FS_NONE_FreeSectors         (      FS_VOLUME  * pVolume);
int         FS_NONE_GetFreeSpace        (      FS_VOLUME  * pVolume, void * pBuffer, int SizeOfBuffer, U32 FirstClusterId, U32 * pNumClustersFree, U32 * pNumClustersChecked);
int         FS_NONE_GetATInfo           (      FS_VOLUME  * pVolume, FS_AT_INFO * pATInfo);
int         FS_NONE_CheckDir            (      FS_VOLUME  * pVolume, const char * sPath, FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
int         FS_NONE_CheckAT             (      FS_VOLUME  * pVolume, const FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
I32         FS_NONE_ReadATEntry         (      FS_VOLUME  * pVolume, U32 ClusterId);
int         FS_NONE_GetFSType           (const FS_VOLUME  * pVolume);
int         FS_NONE_SetDirEntryInfoEx   (      FS_VOLUME  * pVolume, const FS_DIRENTRY_POS * pDirEntryPos, const void * p, int Mask);
int         FS_NONE_GetDirEntryInfoEx   (      FS_VOLUME  * pVolume, const FS_DIRENTRY_POS * pDirEntryPos,       void * p, int Mask);

/*********************************************************************
*
*       CLIB
*
*   Optional replacements for standard "C" library routines.
*/
int         FS__CLIB_memcmp  (const void * s1, const void * s2, unsigned NumBytes);
void *      FS__CLIB_memset  (void * pData, int Fill, U32 NumBytes);
int         FS__CLIB_strcmp  (const char *s1, const char * s2);
char *      FS__CLIB_strcpy  (char * s1, const char * s2);
unsigned    FS__CLIB_strlen  (const char * s);
int         FS__CLIB_strncmp (const char * s1, const char * s2, unsigned n);
char *      FS__CLIB_strncpy (char * s1, const char * s2, U32 n);
char *      FS__CLIB_strchr  (const char * s, int c);
int         FS__CLIB_toupper (int c);
int         FS__CLIB_tolower (int c);
char *      FS__CLIB_strncat (char * s1, const char * s2, U32 n);
int         FS__CLIB_isupper (int c);
int         FS__CLIB_islower (int c);
int         FS_CLIB_Validate (void);

//lint -emacro(534, FS_MEMSET) N:999. The return value of memset() can be safely ignored because it does not report any error.
#if FS_NO_CLIB
#ifndef   FS_MEMCMP
  #define FS_MEMCMP(s1, s2, n)   FS__CLIB_memcmp(s1, s2, n)
#endif
#ifndef   FS_MEMSET
  #define FS_MEMSET(s, c, n)     FS__CLIB_memset(s, c, n)
#endif
#ifndef   FS_STRCMP
  #define FS_STRCMP(s1, s2)      FS__CLIB_strcmp(s1, s2)
#endif
#ifndef   FS_STRCPY
  #define FS_STRCPY(s1, s2)      FS__CLIB_strcpy(s1, s2)
#endif
#ifndef   FS_STRNCPY
  #define FS_STRNCPY(s1, s2, n)  FS__CLIB_strncpy(s1, s2, n)
#endif
#ifndef   FS_STRLEN
  #define FS_STRLEN(s)           FS__CLIB_strlen(s)
#endif
#ifndef   FS_STRNCAT
  #define FS_STRNCAT(s1, s2, n)  FS__CLIB_strncat(s1, s2, n)
#endif
#ifndef   FS_STRNCMP
  #define FS_STRNCMP(s1, s2, n)  FS__CLIB_strncmp(s1, s2, n)
#endif
#ifndef   FS_STRCHR
  #define FS_STRCHR(s, c)        FS__CLIB_strchr(s, c)
#endif
#ifndef   FS_TOUPPER
  #define FS_TOUPPER(c)          FS__CLIB_toupper(c)
#endif
#ifndef   FS_TOLOWER
  #define FS_TOLOWER(c)          FS__CLIB_tolower(c)
#endif
#ifndef   FS_ISUPPER
  #define FS_ISUPPER(c)          FS__CLIB_isupper(c)
#endif
#ifndef   FS_ISLOWER
  #define FS_ISLOWER(c)          FS__CLIB_islower(c)
#endif
#endif // FS_NO_CLIB

/*********************************************************************
*
*       Logical block layer
*/

/*********************************************************************
*
*       FS_LB_TEST_HOOK_SECTOR_READ_BEGIN
*/
typedef void (FS_LB_TEST_HOOK_SECTOR_READ_BEGIN)(U8 Unit, U32 * pSectorIndex, void * pData, U32 * pNumSectors);

/*********************************************************************
*
*       FS_LB_TEST_HOOK_SECTOR_READ_END
*/
typedef void (FS_LB_TEST_HOOK_SECTOR_READ_END)(U8 Unit, U32 SectorIndex, void * pData, U32 NumSectors, int * pResult);

/*********************************************************************
*
*       FS_LB_TEST_HOOK_SECTOR_WRITE_BEGIN
*/
typedef void (FS_LB_TEST_HOOK_SECTOR_WRITE_BEGIN)(U8 Unit, U32 * pSectorIndex, const void ** ppData, U32 * pNumSectors, U8 * pRepeatSame);

/*********************************************************************
*
*       FS_LB_TEST_HOOK_SECTOR_WRITE_END
*/
typedef void (FS_LB_TEST_HOOK_SECTOR_WRITE_END)(U8 Unit, U32 SectorIndex, const void * pData, U32 NumSectors, U8 RepeatSame, int * pResult);

/*********************************************************************
*
*       Internal API functions
*/
int  FS_LB_GetStatus                   (const FS_DEVICE    * pDevice);
U16  FS_GetSectorSize                  (      FS_DEVICE    * pDevice);
int  FS_LB_GetDeviceInfo               (      FS_DEVICE    * pDevice, FS_DEV_INFO * pDevInfo);
int  FS_LB_InitMedium                  (      FS_DEVICE    * pDevice);
int  FS_LB_InitMediumIfRequired        (      FS_DEVICE    * pDevice);
int  FS_LB_Ioctl                       (      FS_DEVICE    * pDevice, I32 Cmd,         I32 Aux,              void * pData);
int  FS_LB_FreeSectors                 (const FS_DEVICE    * pDevice, U32 SectorIndex, U32 NumSectors);
int  FS_LB_FreeSectorsPart             (      FS_PARTITION * pPart,   U32 SectorIndex, U32 NumSectors);
int  FS_LB_ReadBurst                   (      FS_DEVICE    * pDevice, U32 SectorIndex, U32 NumSectors,       void * pData, U8 Type);
int  FS_LB_ReadBurstPart               (      FS_PARTITION * pPart,   U32 SectorIndex, U32 NumSectors,       void * pData, U8 Type);
int  FS_LB_ReadDevice                  (      FS_DEVICE    * pDevice, U32 SectorIndex,                       void * pData, U8 Type);
int  FS_LB_ReadPart                    (      FS_PARTITION * pPart,   U32 SectorIndex,                       void * pData, U8 Type);
int  FS_LB_ReadSectors                 (const FS_DEVICE    * pDevice, U32 SectorIndex, U32 NumSectors,       void * pData);
int  FS_LB_WriteBack                   (      FS_DEVICE    * pDevice, U32 SectorIndex,                 const void * pData);
int  FS_LB_WriteBurst                  (      FS_DEVICE    * pDevice, U32 SectorIndex, U32 NumSectors, const void * pData, U8 Type, U8 WriteToJournal);
int  FS_LB_WriteBurstPart              (      FS_PARTITION * pPart,   U32 SectorIndex, U32 NumSectors, const void * pData, U8 Type, U8 WriteToJournal);
int  FS_LB_WriteDevice                 (      FS_DEVICE    * pDevice, U32 SectorIndex,                 const void * pData, U8 Type, U8 WriteToJournal);
int  FS_LB_WritePart                   (      FS_PARTITION * pPart,   U32 SectorIndex,                 const void * pData, U8 Type, U8 WriteToJournal);
int  FS_LB_WriteMultiple               (      FS_DEVICE    * pDevice, U32 SectorIndex, U32 NumSectors, const void * pData, U8 Type, U8 WriteToJournal);
int  FS_LB_WriteMultiplePart           (      FS_PARTITION * pPart,   U32 SectorIndex, U32 NumSectors, const void * pData, U8 Type, U8 WriteToJournal);
int  FS_LB_WriteSectors                (      FS_DEVICE    * pDevice, U32 SectorIndex, U32 NumSectors, const void * pData, U8 RepeatSame);

/*********************************************************************
*
*       Functions to register a test hook
*/
void FS__LB_SetTestHookSectorReadBegin (FS_LB_TEST_HOOK_SECTOR_READ_BEGIN  * pfTestHook);
void FS__LB_SetTestHookSectorReadEnd   (FS_LB_TEST_HOOK_SECTOR_READ_END    * pfTestHook);
void FS__LB_SetTestHookSectorWriteBegin(FS_LB_TEST_HOOK_SECTOR_WRITE_BEGIN * pfTestHook);
void FS__LB_SetTestHookSectorWriteEnd  (FS_LB_TEST_HOOK_SECTOR_WRITE_END   * pfTestHook);

/*********************************************************************
*
*       File encryption
*/
void FS__CRYPT_DecryptBytes(U8 * pDest, const U8 * pSrc, U32 NumBytes, U8 RandKey, const U8 * pFirstKey);
void FS__CRYPT_EncryptBytes(U8 * pDest, const U8 * pSrc, U32 NumBytes, U8 RandKey, const U8 * pFirstKey);
int  FS_CRYPT_AES_Validate(void);

/*********************************************************************
*
*       Read-ahead cache
*/
int FS__READAHEAD_SetBuffer(U8 Unit, U32 * pData, U32 NumBytes);
#if FS_SUPPORT_READ_AHEAD
  #define FS_ENABLE_READ_AHEAD(pVolume)    (void)FS__IoCtlNL(pVolume, FS_CMD_ENABLE_READ_AHEAD, 0, NULL)
  #define FS_DISABLE_READ_AHEAD(pVolume)   (void)FS__IoCtlNL(pVolume, FS_CMD_DISABLE_READ_AHEAD, 0, NULL)
#else
  #define FS_ENABLE_READ_AHEAD(pVolume)
  #define FS_DISABLE_READ_AHEAD(pVolume)
#endif

/*********************************************************************
*
*       Sector write buffer
*/
U32  FS__WRBUF_GetNumSectors(U8 Unit);
void FS__WRBUF_GetStatCounters(U8 Unit, FS_WRBUF_STAT_COUNTERS * pStat);
void FS__WRBUF_ResetStatCounters(U8 Unit);

/*********************************************************************
*
*       RAID5 logical driver
*/
int  FS__RAID5_GetNumPartitions           (U8 Unit);
int  FS__RAID5_CheckParity                (U8 Unit);
void FS__RAID5_SetTestHookSectorReadBegin (FS_STORAGE_TEST_HOOK_SECTOR_READ_BEGIN  * pfTestHook);   //lint -esym(621, FS__RAID5_SetTestHookSectorReadBegin) Identifier clash N:107. Rationale: This symbol is used only in test builds.
void FS__RAID5_SetTestHookSectorReadEnd   (FS_STORAGE_TEST_HOOK_SECTOR_READ_END    * pfTestHook);   //lint -esym(621, FS__RAID5_SetTestHookSectorReadEnd) Identifier clash N:107. Rationale: This symbol is used only in test builds.
void FS__RAID5_SetTestHookSectorWriteBegin(FS_STORAGE_TEST_HOOK_SECTOR_WRITE_BEGIN * pfTestHook);   //lint -esym(621, FS__RAID5_SetTestHookSectorWriteBegin) Identifier clash N:107. Rationale: This symbol is used only in test builds.
void FS__RAID5_SetTestHookSectorWriteEnd  (FS_STORAGE_TEST_HOOK_SECTOR_WRITE_END   * pfTestHook);   //lint -esym(621, FS__RAID5_SetTestHookSectorWriteEnd) Identifier clash N:107. Rationale: This symbol is used only in test builds.

/*********************************************************************
*
*       Instrumentation via SystemView
*/

/*********************************************************************
*
*       Profile event identifiers
*/
enum {
  //
  // Events for the FS API functions
  //
  FS_EVTID_INIT = 0,
  FS_EVTID_DEINIT,
  FS_EVTID_MOUNT,
  FS_EVTID_MOUNTEX,
  FS_EVTID_SETAUTOMOUNT,
  FS_EVTID_SYNC,
  FS_EVTID_UNMOUNT,
  FS_EVTID_UNMOUNTFORCED,
  FS_EVTID_ADDDEVICE,
  FS_EVTID_ADDPHYSDEVICE,
  FS_EVTID_ASSIGNMEMORY,
  FS_EVTID_CONFIGFILEBUFFERDEFAULT,
  FS_EVTID_CONFIGONWRITEDIRUPDATE,
  FS_EVTID_LOGVOL_CREATE,
  FS_EVTID_LOGVOL_ADDDEVICE,
  FS_EVTID_SETFILEBUFFERFLAGS,
  FS_EVTID_SETFILEWRITEMODE,
  FS_EVTID_SETFILEWRITEMODEEX,
  FS_EVTID_SETMEMHANDLER,
  FS_EVTID_SETMAXSECTORSIZE,
  FS_EVTID_FCLOSE,
  FS_EVTID_FOPEN,
  FS_EVTID_FOPENEX,
  FS_EVTID_FREAD,
  FS_EVTID_FWRITE,
  FS_EVTID_READ,
  FS_EVTID_SYNCFILE,
  FS_EVTID_WRITE,
  FS_EVTID_FSEEK,
  FS_EVTID_FTELL,
  FS_EVTID_COPYFILE,
  FS_EVTID_COPYFILEEX,
  FS_EVTID_GETFILEATTRIBUTES,
  FS_EVTID_GETFILETIME,
  FS_EVTID_GETFILETIMEEX,
  FS_EVTID_MODIFYFILEATTRIBUTES,
  FS_EVTID_MOVE,
  FS_EVTID_REMOVE,
  FS_EVTID_RENAME,
  FS_EVTID_SETENDOFFILE,
  FS_EVTID_SETFILEATTRIBUTES,
  FS_EVTID_SETFILETIME,
  FS_EVTID_SETFILETIMEEX,
  FS_EVTID_SETFILESIZE,
  FS_EVTID_TRUNCATE,
  FS_EVTID_VERIFY,
  FS_EVTID_WIPEFILE,
  FS_EVTID_CREATEDIR,
  FS_EVTID_FINDCLOSE,
  FS_EVTID_FINDFIRSTFILE,
  FS_EVTID_FINDNEXTFILE,
  FS_EVTID_MKDIR,
  FS_EVTID_RMDIR,
  FS_EVTID_FORMAT,
  FS_EVTID_FORMATLLIFREQUIRED,
  FS_EVTID_FORMATLOW,
  FS_EVTID_ISHLFORMATTED,
  FS_EVTID_ISLLFORMATTED,
  FS_EVTID_CHECKDISK,
  FS_EVTID_CHECKDISK_ERRCODE2TEXT,
  FS_EVTID_CREATEMBR,
  FS_EVTID_FILETIMETOTIMESTAMP,
  FS_EVTID_FREESECTORS,
  FS_EVTID_GETFILESIZE,
  FS_EVTID_GETMAXSECTORSIZE,
  FS_EVTID_GETNUMFILESOPEN,
  FS_EVTID_GETNUMVOLUMES,
  FS_EVTID_GETPARTITIONINFO,
  FS_EVTID_GETVOLUMEFREESPACE,
  FS_EVTID_GETVOLUMEFREESPACEKB,
  FS_EVTID_GETVOLUMEINFO,
  FS_EVTID_GETVOLUMEINFOEX,
  FS_EVTID_GETVOLUMELABEL,
  FS_EVTID_GETVOLUMENAME,
  FS_EVTID_GETVOLUMESIZE,
  FS_EVTID_GETVOLUMESIZEKB,
  FS_EVTID_GETVOLUMESTATUS,
  FS_EVTID_ISVOLUMEMOUNTED,
  FS_EVTID_LOCK,
  FS_EVTID_LOCKVOLUME,
  FS_EVTID_SETBUSYLEDCALLBACK,
  FS_EVTID_SETMEMACCESSCALLBACK,
  FS_EVTID_SETVOLUMELABEL,
  FS_EVTID_TIMESTAMPTOFILETIME,
  FS_EVTID_UNLOCK,
  FS_EVTID_UNLOCKVOLUME,
  FS_EVTID_CLEARERR,
  FS_EVTID_ERRORNO2TEXT,
  FS_EVTID_FEOF,
  FS_EVTID_FERROR,
  FS_EVTID_FAT_GROWROOTDIR,
  FS_EVTID_FORMATSD,
  FS_EVTID_FAT_SUPPORTLFN,
  FS_EVTID_FAT_DISABLELFN,
  FS_EVTID_FAT_CONFIGFATCOPYMAINTENANCE,
  FS_EVTID_FAT_CONFIGFSINFOSECTORUSE,
  FS_EVTID_FAT_CONFIGROFILEMOVEPERMISSION,
  //
  // Events for the storage layer API functions
  //
  FS_EVTID_STORAGE_CLEAN,
  FS_EVTID_STORAGE_CLEANONE,
  FS_EVTID_STORAGE_FREESECTORS,
  FS_EVTID_STORAGE_GETCLEANCNT,
  FS_EVTID_STORAGE_GETCOUNTERS,
  FS_EVTID_STORAGE_GETDEVICEINFO,
  FS_EVTID_STORAGE_GETSECTORUSAGE,
  FS_EVTID_STORAGE_INIT,
  FS_EVTID_STORAGE_READSECTOR,
  FS_EVTID_STORAGE_READSECTORS,
  FS_EVTID_STORAGE_REFRESHSECTORS,
  FS_EVTID_STORAGE_RESETCOUNTERS,
  FS_EVTID_STORAGE_SYNC,
  FS_EVTID_STORAGE_SYNCSECTORS,
  FS_EVTID_STORAGE_UNMOUNT,
  FS_EVTID_STORAGE_WRITESECTOR,
  FS_EVTID_STORAGE_WRITESECTORS,
  //
  // Events for the cache API functions
  //
  FS_EVTID_ASSIGNCACHE,
  FS_EVTID_CACHE_CLEAN,
  FS_EVTID_CACHE_INVALIDATE,
  FS_EVTID_CACHE_SETASSOCLEVEL,
  FS_EVTID_CACHE_SETMODE,
  FS_EVTID_CACHE_SETQUOTA,
  //
  // Events for the journal API functions
  //
  FS_EVTID_JOURNAL_BEGIN,
  FS_EVTID_JOURNAL_CREATE,
  FS_EVTID_JOURNAL_CREATEEX,
  FS_EVTID_JOURNAL_DISABLE,
  FS_EVTID_JOURNAL_ENABLE,
  FS_EVTID_JOURNAL_END,
  //
  // Events for the encryption API functions
  //
  FS_EVTID_CRYPT_PREPARE,
  FS_EVTID_SETENCRYPTIONOBJECT,
  //
  // Events for the logical block layer.
  //
  FS_EVTID_LB_GETSTATUS,
  FS_EVTID_LB_GETDEVICEINFO,
  FS_EVTID_LB_INITMEDIUM,
  FS_EVTID_LB_IOCTL,
  FS_EVTID_LB_FREESECTORS,
  FS_EVTID_LB_READBURST,
  FS_EVTID_LB_READDEVICE,
  FS_EVTID_LB_WRITEBURST,
  FS_EVTID_LB_WRITEDEVICE,
  FS_EVTID_LB_WRITEMULTIPLE,
  FS_EVTID_LB_WRITEBACK,
  //
  // Make sure this is always the last entry.
  //
  FS_NUM_EVTIDS
};

#define FS_PROFILE_GET_EVENT_ID(EvtId)    ((unsigned)(EvtId) + (unsigned)FS_Global.Profile.IdOffset)

/*********************************************************************
*
*       FS_PROFILE_END_CALL
*/
#if (FS_SUPPORT_PROFILE != 0) && (FS_SUPPORT_PROFILE_END_CALL != 0)
  #define FS_PROFILE_END_CALL(EventId)                                           \
    if (FS_Global.Profile.pAPI != NULL) {                                        \
      FS_Global.Profile.pAPI->pfRecordEndCall(FS_PROFILE_GET_EVENT_ID(EventId)); \
    }
#else
  #define FS_PROFILE_END_CALL(EventId)
#endif

/*********************************************************************
*
*       FS_PROFILE_END_CALL_U32
*/
#if (FS_SUPPORT_PROFILE != 0) && (FS_SUPPORT_PROFILE_END_CALL != 0)
  #define FS_PROFILE_END_CALL_U32(EventId, Para0)                                                 \
    if (FS_Global.Profile.pAPI != NULL) {                                                         \
      FS_Global.Profile.pAPI->pfRecordEndCallU32(FS_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0)); \
    }
#else
  #define FS_PROFILE_END_CALL_U32(EventId, ReturnValue)
#endif

/*********************************************************************
*
*       FS_PROFILE_CALL_VOID
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_VOID(EventId)                                       \
    if (FS_Global.Profile.pAPI != NULL) {                                     \
      FS_Global.Profile.pAPI->pfRecordVoid(FS_PROFILE_GET_EVENT_ID(EventId)); \
    }
#else
  #define FS_PROFILE_CALL_VOID(EventId)
#endif

/*********************************************************************
*
*       FS_PROFILE_CALL_U32
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_U32(EventId, Para0)                                              \
    if (FS_Global.Profile.pAPI != NULL) {                                                  \
      FS_Global.Profile.pAPI->pfRecordU32(FS_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0)); \
    }
#else
  #define FS_PROFILE_CALL_U32(EventId, Para0)
#endif

/*********************************************************************
*
*       FS_PROFILE_CALL_U32x2
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_U32x2(EventId, Para0, Para1)                                                     \
    if (FS_Global.Profile.pAPI != NULL) {                                                                  \
      FS_Global.Profile.pAPI->pfRecordU32x2(FS_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1)); \
    }
#else
  #define FS_PROFILE_CALL_U32x2(Id, Para0, Para1)
#endif

/*********************************************************************
*
*       FS_PROFILE_CALL_U32x3
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_U32x3(EventId, Para0, Para1, Para2)                                                            \
    if (FS_Global.Profile.pAPI != NULL) {                                                                                \
      FS_Global.Profile.pAPI->pfRecordU32x3(FS_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2)); \
    }
#else
  #define FS_PROFILE_CALL_U32x3(EventId, Para0, Para1, Para2)
#endif

/*********************************************************************
*
*       FS_PROFILE_CALL_U32x4
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_U32x4(EventId, Para0, Para1, Para2, Para3)                                                                   \
    if (FS_Global.Profile.pAPI != NULL) {                                                                                              \
      FS_Global.Profile.pAPI->pfRecordU32x4(FS_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3)); \
    }
#else
  #define FS_PROFILE_CALL_U32x4(EventId, Para0, Para1, Para2, Para3)
#endif

/*********************************************************************
*
*       FS_PROFILE_RECORD_API_U32x5
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_U32x5(EventId, Para0, Para1, Para2, Para3, Para4)                                                                          \
    if (FS_Global.Profile.pAPI != NULL) {                                                                                                            \
      FS_Global.Profile.pAPI->pfRecordU32x5(FS_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3), (U32)(Para4)); \
    }
#else
  #define FS_PROFILE_CALL_U32x5(EventId, Para0, Para1, Para2, Para3, Para4)
#endif

/*********************************************************************
*
*       FS_PROFILE_CALL_U32x6
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_U32x6(EventId, Para0, Para1, Para2, Para3, Para4, Para5)                                                                                 \
    if (FS_Global.Profile.pAPI != NULL) {                                                                                                                          \
      FS_Global.Profile.pAPI->pfRecordU32x6(FS_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3), (U32)(Para4), (U32)(Para5)); \
    }
#else
  #define FS_PROFILE_CALL_U32x6(EventId, Para0, Para1, Para2, Para3, Para4, Para5)
#endif

/*********************************************************************
*
*       FS_PROFILE_CALL_U32x7
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_U32x7(EventId, Para0, Para1, Para2, Para3, Para4, Para5, Para6)                                                                                        \
    if (FS_Global.Profile.pAPI != NULL) {                                                                                                                                        \
      FS_Global.Profile.pAPI->pfRecordU32x7(FS_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3), (U32)(Para4), (U32)(Para5), (U32)(Para6)); \
    }
#else
  #define FS_PROFILE_CALL_U32x7(EventId, Para0, Para1, Para2, Para3, Para4, Para5, Para6)
#endif

/*********************************************************************
*
*       FS_PROFILE_CALL_STRING
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_STRING(EventId, pPara0)                                                        \
    if (FS_Global.Profile.pAPI != NULL) {                                                                \
      FS_Global.Profile.pAPI->pfRecordString(FS_PROFILE_GET_EVENT_ID(EventId), (const char *)(pPara0));  \
    }
#else
  #define FS_PROFILE_CALL_STRING(EventId, pPara0)
#endif

/*********************************************************************
*
*       FS_PROFILE_RECORD_API_STRINGx2
*/
#if FS_SUPPORT_PROFILE
  #define FS_PROFILE_CALL_STRINGx2(EventId, pPara0, pPara1)                                                                       \
    if (FS_Global.Profile.pAPI != NULL) {                                                                                         \
      FS_Global.Profile.pAPI->pfRecordStringx2(FS_PROFILE_GET_EVENT_ID(EventId), (const char *)(pPara0), (const char *)(pPara1)); \
    }
#else
  #define FS_PROFILE_CALL_STRINGx2(EventId, pPara0, pPara1)
#endif

/*********************************************************************
*
*       Character set processing
*/
FS_WCHAR FS_UNICODE_ToUpper(FS_WCHAR UnicodeChar);
FS_WCHAR FS_UNICODE_ToLower(FS_WCHAR UnicodeChar);

/*********************************************************************
*
*       Public const data
*
**********************************************************************
*/
#if FS_SUPPORT_MULTIPLE_FS
  extern const FS_FS_API FS_FAT_API;
  extern const FS_FS_API FS_EFS_API;
#endif

extern const FS_UNICODE_CONV FS_UNICODE_CONV_CP437_ASCII;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
extern FS_STORAGE_COUNTERS     FS_STORAGE_Counters;
extern const FS_CHARSET_TYPE * FS_pCharSetType;
#if FS_SUPPORT_POSIX
  extern U8                    FS_IsPOSIXSupported;
#endif // FS_SUPPORT_POSIX
#if FS_VERIFY_WRITE
  extern U8                    FS_IsWriteVerificationEnabled;
#endif // FS_SUPPORT_POSIX

/*********************************************************************
*
*       Compatibility defines
*/
#define FS_FindVolume(pVolume)                                      FS_STORAGE_FindVolume(pVolume)      // This function is currently called only by emUSB-Device.
#define FS__FormatLow(pVolume)                                      FS_STORAGE_FormatLowEx(pVolume)
#define FS__GetDeviceInfo(pVolume, pDeviceInfo)                     FS_STORAGE_GetDeviceInfoEx(pVolume, pDeviceInfo)
#define FS__GetVolumeStatus(pVolume)                                FS_STORAGE_GetVolumeStatusEx(pVolume)
#define FS__IsLLFormatted(pVolume)                                  FS_STORAGE_IsLLFormattedEx(pVolume)
#define FS__ReadSector(pVolume, pData, SectorIndex)                 FS_STORAGE_ReadSectorEx(pVolume, pData, SectorIndex)
#define FS__ReadSectors(pVolume, pData, SectorIndex, NumSectors)    FS_STORAGE_ReadSectorsEx(pVolume, pData, SectorIndex, NumSectors)
#define FS__UnmountLL(pVolume)                                      FS_STORAGE_UnmountEx(pVolume)
#define FS__UnmountForcedLL(pVolume)                                FS_STORAGE_UnmountForcedEx(pVolume)
#define FS__WriteSector(pVolume, pData, SectorIndex)                FS_STORAGE_WriteSectorEx(pVolume, pData, SectorIndex)
#define FS__WriteSectors(pVolume, pData, SectorIndex, NumSectors)   FS_STORAGE_WriteSectorsEx(pVolume, pData, SectorIndex, NumSectors)

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // FS_INT_H

/*************************** End of file ****************************/
