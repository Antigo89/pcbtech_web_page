/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2007 - 2021    SEGGER Microcontroller GmbH               *
*                                                                    *
*       www.segger.com     Support: ticket_emnet@segger.com          *
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
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef _IP_CONF_H_
#define _IP_CONF_H_ 1

#ifdef __cplusplus
  extern "C" {
#endif

//
// The ESPRO library has been built with following defines set
//
#ifndef IP_ALLOW_DEINIT
  #define IP_ALLOW_DEINIT                1
#endif
#ifndef IP_OS_DO_NOT_INLINE_CALLS
  #define IP_OS_DO_NOT_INLINE_CALLS      1
#endif
#ifndef IP_SUPPORT_FRAGMENTATION_IPV4
  #define IP_SUPPORT_FRAGMENTATION_IPV4  1
#endif
#ifndef IP_SUPPORT_FRAGMENTATION_IPV6
  #define IP_SUPPORT_FRAGMENTATION_IPV6  1
#endif
#ifndef IP_SUPPORT_IPV6
  #define IP_SUPPORT_IPV6                1
#endif
#ifndef IP_SUPPORT_ON_PACKET_FREE_CB
  #define IP_SUPPORT_ON_PACKET_FREE_CB   1
#endif
#ifndef IP_SUPPORT_PACKET_TIMESTAMP
  #define IP_SUPPORT_PACKET_TIMESTAMP    1
#endif
#ifndef IP_SUPPORT_PROFILE
  #define IP_SUPPORT_PROFILE             1
#endif
//
// Library is build with IP_SUPPORT_PTP set to 1. 
// However you should only set this define in your project to 1
// if you have purchased the PTP addon with its sources. 
// Otherwise issues may appear when building projects.
//
#ifndef IP_SUPPORT_PTP
  #define IP_SUPPORT_PTP                 0
#endif
#ifndef IP_SUPPORT_STATS
  #define IP_SUPPORT_STATS               1
#endif
#ifndef NM_DEBUG
  #define NM_DEBUG                       0
#endif
#ifndef WEBS_SUPPORT_UPLOAD
  #define WEBS_SUPPORT_UPLOAD            1
#endif

//
// Define IP_DEBUG: Debug level for IP stack
//                  0: No checks, no statistics      (Smallest and fastest code)
//                  1: Statistics & "Panic" checks
//                  2: Statistics, warn, log, panic  (Seriously bigger code)
//
#ifndef DEBUG
  #define DEBUG 0
#endif

#if DEBUG
  #ifndef   IP_DEBUG
    #define IP_DEBUG      2      // Default for debug builds
  #endif
#else
  #ifndef   IP_DEBUG
    #define IP_DEBUG      0      // Default for release builds
  #endif
#endif

//
// Configure IPv6 support. Requires the IPv6 addon modules.
//
#ifndef   IP_SUPPORT_IPV6
  #define IP_SUPPORT_IPV6  0
#endif

//
// Configure target to PC communication.
//
#ifndef   USE_RTT
  #define USE_RTT         0
#endif

#ifndef   USE_SYSTEMVIEW
  #define USE_SYSTEMVIEW  0
#endif

//
// Configure profiling support.
//
#if defined(SUPPORT_PROFILE) && (SUPPORT_PROFILE)
  #ifndef   IP_SUPPORT_PROFILE
    #define IP_SUPPORT_PROFILE           1                   // Define as 1 to enable profiling via SystemView.
  #endif
#endif

#if defined(IP_SUPPORT_PROFILE)
  #ifndef   IP_SUPPORT_PROFILE_END_CALL
    #define IP_SUPPORT_PROFILE_END_CALL  IP_SUPPORT_PROFILE  // Define as 1 to profile end of function calls.
  #endif
#else
  #ifndef   IP_SUPPORT_PROFILE_END_CALL
    #define IP_SUPPORT_PROFILE_END_CALL  0                   // May not be used without profiling enabled.
  #endif
#endif

//
// Default stack size for IP_Task() .
//
#ifndef     TASK_STACK_SIZE_IP_TASK
  #if   ((defined(IP_SUPPORT_PROFILE) && IP_SUPPORT_PROFILE) || USE_RTT || USE_SYSTEMVIEW)
    #define TASK_STACK_SIZE_IP_TASK     1280
  #elif IP_SUPPORT_IPV6
    #define TASK_STACK_SIZE_IP_TASK     1024
  #else
    #define TASK_STACK_SIZE_IP_TASK     768
  #endif
#endif

//
// Default stack size for IP_RxTask() .
//
#ifndef     TASK_STACK_SIZE_IP_RX_TASK
  #if   ((defined(IP_SUPPORT_PROFILE) && IP_SUPPORT_PROFILE) || USE_SYSTEMVIEW)
    #define TASK_STACK_SIZE_IP_RX_TASK  1024
  #elif (IP_SUPPORT_IPV6 || USE_RTT)
    #define TASK_STACK_SIZE_IP_RX_TASK  768
  #else
    #define TASK_STACK_SIZE_IP_RX_TASK  512
  #endif
#endif

//
// Default stack size for IP_WIFI_IsrTask() .
//
#ifndef     TASK_STACK_SIZE_IP_WIFI_TASK
  #if   ((defined(IP_SUPPORT_PROFILE) && IP_SUPPORT_PROFILE) || USE_SYSTEMVIEW)
    #define TASK_STACK_SIZE_IP_WIFI_TASK  2560
  #elif (IP_SUPPORT_IPV6 || USE_RTT)
    #define TASK_STACK_SIZE_IP_WIFI_TASK  2304
  #else
    #define TASK_STACK_SIZE_IP_WIFI_TASK  2048
  #endif
#endif

//
// If using PPPoE or VLAN at least two interfaces are needed, one for
// the physical interface and one for the PPPoE or VLAN device.
//
#ifndef   IP_MAX_IFACES
  #define IP_MAX_IFACES  2
#endif

//
// For SNMP Agent support the stack needs to maintain some
// counters. Support for this needs to be enabled in general
// by the following define and by enabling stats per interface
// by calling IP_STATS_EnableIFaceCounters() during IP_X_Config().
//
#ifndef   IP_SUPPORT_STATS
  #define IP_SUPPORT_STATS  1
#endif

//
// Inline OS function calls for higher performance in release builds
//
#ifndef IP_OS_DO_NOT_INLINE_CALLS
  #if !IP_DEBUG
    #include "RTOS.h"
    extern OS_RSEMA IP_OS_RSema;
    #define IP_OS_INC_DI()    OS_IncDI()
    #define IP_OS_DEC_RI()    OS_DecRI()
    #define IP_OS_GET_TIME()  (U32)OS_GetTime32()
    #define IP_OS_LOCK()      (void)OS_Use(&IP_OS_RSema)
    #define IP_OS_UNLOCK()    OS_Unuse(&IP_OS_RSema)
  #endif
#endif

//
// Optimized routines
//
#define IP_SNPRINTF  SEGGER_snprintf

//
// IAR ARM compiler related macros
//
#if defined(__ICCARM__)
  #ifndef __ARM_PROFILE_M__  // For any non profile M core, we will use optimized routines
    #include "SEGGER.h"
    U32 ARM_IP_cksum(void * ptr, unsigned NumHWords, U32 Sum);
    #define IP_CKSUM(p, NumItems, Sum)  ARM_IP_cksum((p), (NumItems), (Sum))
    #define IP_MEMCPY(pDest, pSrc, NumBytes)  SEGGER_ARM_memcpy(pDest, pSrc, NumBytes)  // Speed optimization: Our memcpy is much faster!
  #else                      // For any profile M core except M0, at least use our optimized checksum routine
    #if (__CORE__ != __ARM6__)
      U32 CM_IP_cksum(void * ptr, unsigned NumHWords, U32 Sum);
      #define IP_CKSUM(p, NumItems, Sum)  CM_IP_cksum((p), (NumItems), (Sum))
    #endif
  #endif
  #ifndef   IP_IS_BIG_ENDIAN
    #define IP_IS_BIG_ENDIAN (1 - __LITTLE_ENDIAN__)
  #endif
#endif

//
// GCC/SEGGER compiler related macros
//
#if (defined(__GNUC__) || defined(__SEGGER_CC__))
  #if defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_5T__)|| defined(__ARM_ARCH_5TE__)   // For any ARM CPU core < v7, we will use optimized routines
    #include "SEGGER.h"
    U32 ARM_IP_cksum(void * ptr, unsigned NumHWords, U32 Sum);
    #define IP_CKSUM(p, NumItems, Sum)  ARM_IP_cksum((p), (NumItems), (Sum))
    #define IP_MEMCPY(pDest, pSrc, NumBytes)  SEGGER_ARM_memcpy(pDest, pSrc, NumBytes)   // Speed optimization: Our memcpy is much faster!
  #elif defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)                            // Cortex-M3, -M4, -M4F
    U32 CM_IP_cksum(void * ptr, unsigned NumHWords, U32 Sum);
    #define IP_CKSUM(p, NumItems, Sum)  CM_IP_cksum((p), (NumItems), (Sum))
  #endif
#endif

//
// Renesas RX compiler related macros
//
#if defined(__RX)
  #if (__RX == 1)
    #include "SEGGER.h"
    U32 RX_IP_cksum(void * ptr, unsigned NumHWords, U32 Sum);
    #define IP_CKSUM(p, NumItems, Sum)  RX_IP_cksum((p), (NumItems), (Sum))
  #endif
#endif

#if defined (__ICCRX__)
  #include "SEGGER.h"
  U32 RX_IP_cksum(void * ptr, unsigned NumHWords, U32 Sum);
  #define IP_CKSUM(p, NumItems, Sum)  RX_IP_cksum((p), (NumItems), (Sum))
#endif

//
// Default value is little endian
//
#ifndef   IP_IS_BIG_ENDIAN
  #define IP_IS_BIG_ENDIAN 0
#endif

#ifdef __cplusplus
  }
#endif

#endif     // Avoid multiple inclusion

/*************************** End of file ****************************/
