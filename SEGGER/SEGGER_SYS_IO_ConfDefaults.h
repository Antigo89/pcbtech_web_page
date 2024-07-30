/*********************************************************************
*               (c) SEGGER Microcontroller GmbH & Co. KG             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYS_IO_ConfDefaults.h
Purpose : Configuration of SEGGER_SYS_IO output
          implementation on embedded Targets
Revision: $Rev: 14032 $

*/

//
// Default output implementation
//
#define SYSIO_USE_PRINTF 1
//
// Non-default output implementations
//
#ifndef USE_RTT
  #define USE_RTT 0
#endif
//
// If any non-default output implementation is used,
// disable default output implementation
//
#if USE_RTT
  #undef SYSIO_USE_PRINTF
  #define SYSIO_USE_PRINTF 0
#endif

