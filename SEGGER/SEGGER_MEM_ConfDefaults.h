/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
*                                                                    *
*       (c) 2014 - 2022    SEGGER Microcontroller GmbH               *
*                                                                    *
*       www.segger.com     Support: www.segger.com/ticket            *
*                                                                    *
**********************************************************************
*                                                                    *
*       emCrypt * Cryptographic algorithm library                    *
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
*       emCrypt version: V2.38.0                                     *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File        : SEGGER_MEM_ConfDefaults.h
Purpose     : Defines defaults for configurable defines used in the
              SEGGER_MEM allocators.
Revision    : $Rev: 7453 $
--------  END-OF-HEADER  ----------------------------------------------
*/

#ifndef SEGGER_MEM_CONFDEFAULTS_H
#define SEGGER_MEM_CONFDEFAULTS_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "SEGGER_MEM_Conf.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Configuration defaults
*
**********************************************************************
*/

//
// Define SEGGER_MEM_DEBUG: Debug level for SEGGER_MEM
//                  0: No checks                      (Smallest and fastest code)
//                  1: Warnings & Panic checks
//                  2: Warnings, logs, & panic checks (Seriously bigger code)
//
#ifndef   DEBUG
  #define DEBUG                           0
#endif

#if DEBUG
  #ifndef   SEGGER_MEM_DEBUG
    #define SEGGER_MEM_DEBUG              2           // Default for debug builds
  #endif
#else
  #ifndef   SEGGER_MEM_DEBUG
    #define SEGGER_MEM_DEBUG              0           // Default for release builds
  #endif
#endif

#if DEBUG
  #ifndef   SEGGER_MEM_STATS
    #define SEGGER_MEM_STATS              1           // Default for debug builds
  #endif
#else
  #ifndef   SEGGER_MEM_STATS
    #define SEGGER_MEM_STATS              0           // Default for release builds
  #endif
#endif

#ifndef   SEGGER_MEM_MEMCPY
  #define SEGGER_MEM_MEMCPY               memcpy
#endif

#ifndef   SEGGER_MEM_MEMSET
  #define SEGGER_MEM_MEMSET               memset
#endif

#ifndef   SEGGER_MEM_MEMMOVE
  #define SEGGER_MEM_MEMMOVE              memmove
#endif

#ifndef   SEGGER_MEM_MEMCMP
  #define SEGGER_MEM_MEMCMP               memcmp
#endif

#ifndef   SEGGER_MEM_USE_PARA                         // Some compiler complain about unused parameters.
  #define SEGGER_MEM_USE_PARA(Para)       (void)Para  // This works for most compilers.
#endif

#if SEGGER_MEM_DEBUG > 0
  #define SEGGER_MEM_PANIC(s)             SEGGER_MEM_Panic(s)
#else
  #define SEGGER_MEM_PANIC(s)
#endif

#ifdef __cplusplus
}
#endif

#endif

/****** End Of File *************************************************/
