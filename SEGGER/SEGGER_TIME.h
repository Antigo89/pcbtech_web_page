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
*       emSSL * Embedded Transport Layer Security                    *
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
*       emSSL version: V2.64.0                                       *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File        : SEGGER_TIME.h
Purpose     : Utility functions related to time and date.
Revision    : $Rev: 12387 $
*/

#ifndef SEGGER_TIME_H
#define SEGGER_TIME_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "SEGGER.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Data types
*
**********************************************************************
*/

typedef struct {
  int Second;   // Seconds after the minute - [0, 60] including leap second.
  int Minute;   // Minutes after the hour   - [0, 59].
  int Hour;     // Hours since midnight     - [0, 23].
  int Day;      // Day of the month         - [1, 31].
  int Month;    // Month of year            - [1, 12].
  int Year;     // Julian year.
} SEGGER_TIME_CALENDAR;

typedef struct {
  U32 Day;   // Integer part: day number.
  U32 Time;  // Fractional part: seconds since 12:00:00 for JD, seconds since 00:00:00 for MJD.
} SEGGER_TIME_JULIAN;

typedef struct {
  I32 Value;  // Seconds since 1-Jan-1970.
} SEGGER_TIME_UNIX;

typedef struct {
  I64 Value;  // Seconds since 1-Jan-1970.
} SEGGER_TIME_UNIX_EX;

typedef struct {
  U32 Integer;   // Seconds since 1-Jan-1900.
  U32 Fraction;  // Fraction of a second in units of 2^-32.
} SEGGER_TIME_NTP;

typedef struct {
  U16 Date;  // Bits 15-9: Year, 0=1980, 127=2107; 8-5: Month (1-12); 4-0 Day (1-31)
  U16 Time;  // Bits 15-11: Hour (0-23); 10-5: Minute (0-59); 4-0: Second/2 (0-29)
} SEGGER_TIME_DOS;

typedef struct {
  I32 Date;  // Internally maintained.
  U32 Time;  // Internally maintained.
} SEGGER_TIME;

/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/

/*********************************************************************
*
*       Assignment functions.
*/
void     SEGGER_TIME_PutUnix        (SEGGER_TIME *pTime, I32 UnixTime);
void     SEGGER_TIME_PutUnixEx      (SEGGER_TIME *pTime, I64 UnixTime);
void     SEGGER_TIME_PutFS          (SEGGER_TIME *pTime, U32 FSTime);
void     SEGGER_TIME_PutDos         (SEGGER_TIME *pTime, U16 Date, U16 Time);
void     SEGGER_TIME_PutNTP         (SEGGER_TIME *pTime, U32 Integer, U32 Fraction);
void     SEGGER_TIME_PutJD          (SEGGER_TIME *pTime, U32 DayNumber, U32 SecondsSinceMidday);
void     SEGGER_TIME_PutMJD         (SEGGER_TIME *pTime, U32 DayNumber, U32 SecondsSinceMidnight);
void     SEGGER_TIME_PutCalendar    (SEGGER_TIME *pTime, int Year, unsigned Month, unsigned Day, unsigned Hour, unsigned Minute, unsigned Second);

/*********************************************************************
*
*       Retrieval functions.
*/
void     SEGGER_TIME_GetUnix        (const SEGGER_TIME *pTime, I32 *pOutput);
void     SEGGER_TIME_GetUnixEx      (const SEGGER_TIME *pTime, I64 *pOutput);
void     SEGGER_TIME_GetFS          (const SEGGER_TIME *pTime, U32 *pOutput);
void     SEGGER_TIME_GetDos         (const SEGGER_TIME *pTime, SEGGER_TIME_DOS      *pOutput);
void     SEGGER_TIME_GetNTP         (const SEGGER_TIME *pTime, SEGGER_TIME_NTP      *pOutput);
void     SEGGER_TIME_GetJD          (const SEGGER_TIME *pTime, SEGGER_TIME_JULIAN   *pOutput);
void     SEGGER_TIME_GetMJD         (const SEGGER_TIME *pTime, SEGGER_TIME_JULIAN   *pOutput);
void     SEGGER_TIME_GetCalendar    (const SEGGER_TIME *pTime, SEGGER_TIME_CALENDAR *pOutput);
unsigned SEGGER_TIME_GetJDN         (const SEGGER_TIME *pTime);

/*********************************************************************
*
*       Retrieval functions.
*/
unsigned SEGGER_TIME_GetDay         (const SEGGER_TIME *pTime);
unsigned SEGGER_TIME_GetMonth       (const SEGGER_TIME *pTime);
unsigned SEGGER_TIME_GetYear        (const SEGGER_TIME *pTime);
unsigned SEGGER_TIME_GetHour        (const SEGGER_TIME *pTime);
unsigned SEGGER_TIME_GetMinute      (const SEGGER_TIME *pTime);
unsigned SEGGER_TIME_GetSecond      (const SEGGER_TIME *pTime);

/*********************************************************************
*
*       Julian calendar functions.
*/
unsigned SEGGER_TIME_GetWeekDay     (const SEGGER_TIME *pTime);
unsigned SEGGER_TIME_GetYearDay     (const SEGGER_TIME *pTime);
unsigned SEGGER_TIME_GetLeapYear    (const SEGGER_TIME *pTime);

/*********************************************************************
*
*       Utility conversion functions.
*/
U32      SEGGER_TIME_ConvUnix2FS    (I32 UnixTime);
U32      SEGGER_TIME_ConvUnixEx2FS  (I64 UnixTime);
I32      SEGGER_TIME_ConvFS2Unix    (U32 FSTime);
I64      SEGGER_TIME_ConvFS2UnixEx  (U32 FSTime);

/*********************************************************************
*
*       Time formatting functions.
*/
void         SEGGER_TIME_Format             (char *pBuf, unsigned BufLen, const char *sFormat, SEGGER_TIME *pTime);
const char * SEGGER_TIME_FormatUnixExStatic (I64 UnixTime);

/*********************************************************************
*
*       Testing functions.
*/
int          SEGGER_TIME_SelfTest   (void);

#ifdef __cplusplus
}
#endif

#endif


/*************************** End of file ****************************/
