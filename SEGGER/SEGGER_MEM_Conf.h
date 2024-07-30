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
File        : SEGGER_MEM_Conf.h
Purpose     : Configuration file for SEGGER memory allocators.
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef SEGGER_MEM_CONF_H
#define SEGGER_MEM_CONF_H

//
// Define SEGGER_MEM_DEBUG: Debug level for SSL product
//                  0: No checks                      (Smallest and fastest code)
//                  1: Warnings & Panic checks
//                  2: Warnings, logs, & panic checks (Bigger code)
//
#ifndef   DEBUG
  #define DEBUG            0
#endif

#if DEBUG
  #ifndef   SEGGER_MEM_DEBUG
    #define SEGGER_MEM_DEBUG      2      // Default for debug builds
  #endif
  #ifndef   SEGGER_MEM_STATS
    #define SEGGER_MEM_STATS      1      // Default for debug builds, include statistics
  #endif
#else
  #ifndef   SEGGER_MEM_DEBUG
    #define SEGGER_MEM_DEBUG      0      // Default for release builds
  #endif
  #ifndef   SEGGER_MEM_STATS
    #define SEGGER_MEM_STATS      0      // Default for release builds, don't include statistics
  #endif
#endif

#endif

/****** End Of File *************************************************/
