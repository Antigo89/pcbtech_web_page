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
*       emSSH * Embedded Secure Shell                                *
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
*       emSSH version: V2.54.0                                       *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File        : SEGGER_SHELL.h
Purpose     : Simple shell API.
Revision    : $Rev: 16293 $

*/

#ifndef SEGGER_SHELL_H
#define SEGGER_SHELL_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "SEGGER.h"
#include "SEGGER_MEM.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define SEGGER_SHELL_HELP_FLAG_SHOW_USAGE       0x0001   // Usage: foo -bar
#define SEGGER_SHELL_HELP_FLAG_SHOW_OPTIONS     0x0002   //   -x   Exclude baz.
#define SEGGER_SHELL_HELP_FLAG_SHOW_COMMAND     0x0004   // foo

#define SEGGER_SHELL_KEY_CTRL(X)                ((X) & 0x1F)

#ifndef SEGGER_SHELL_MONO
  #if 1
  #define SEGGER_SHELL_MONO(X)                    X
  #else
  // If no ANSI colorization required:
  #define SEGGER_SHELL_MONO(X)                    ""
  #endif
#endif

#define SEGGER_SHELL_CRLF "\r\n"

/*********************************************************************
*
*       Function status values
*
*  Description
*    Shell errors.
*
*  Additional information
*    Values returned as errors by API calls to indicate completion
*    status.  All errors are negative.
*/
#define SEGGER_SHELL_ERROR_MORE_ARGS_EXPECTED          (-400)   // Read beyond end of argument list
#define SEGGER_SHELL_ERROR_COMMAND_NOT_RECOGNIZED      (-401)   // Command cannot be found
#define SEGGER_SHELL_ERROR_COMMAND_ALREADY_INSTALLED   (-402)   // Command is already installed into shell
#define SEGGER_SHELL_ERROR_OUT_OF_MEMORY               (-403)   // Memory API reports no more storage
#define SEGGER_SHELL_ERROR_STRING_TOO_LONG             (-404)   // Constructed string would overflow allocated buffer

/*********************************************************************
*
*       ANSI color codes.
*
*  Description
*    List of ANSI color codes.
*/
#define SEGGER_SHELL_ANSI_BLACK                 0
#define SEGGER_SHELL_ANSI_RED                   1
#define SEGGER_SHELL_ANSI_GREEN                 2
#define SEGGER_SHELL_ANSI_YELLOW                3
#define SEGGER_SHELL_ANSI_BLUE                  4
#define SEGGER_SHELL_ANSI_MAGENTA               5
#define SEGGER_SHELL_ANSI_CYAN                  6
#define SEGGER_SHELL_ANSI_WHITE                 7
#define SEGGER_SHELL_ANSI_DEFAULT               9

/*********************************************************************
*
*       ANSI control sequence introducers
*
*  Description
*    List of ANSI control sequence introducers.
*/
#define SEGGER_SHELL_ANSI_CSI                   "\x1b["  // 7-bit control sequence introducer
#define SEGGER_SHELL_ANSI_SS3                   "\217"   // Single shift 3

/*********************************************************************
*
*       Cursor movement sequences
*
*  Description
*    List of cursor movement sequences.
*/
#define SEGGER_SHELL_ANSI_CURSOR_UP             SEGGER_SHELL_ANSI_CSI "A"  // CUU - cursor up one line
#define SEGGER_SHELL_ANSI_CURSOR_DOWN           SEGGER_SHELL_ANSI_CSI "B"  // CUD - cursor down one line
#define SEGGER_SHELL_ANSI_CURSOR_FORWARD        SEGGER_SHELL_ANSI_CSI "C"  // CUF - cursor forward one character
#define SEGGER_SHELL_ANSI_CURSOR_BACK           SEGGER_SHELL_ANSI_CSI "D"  // CUB - cursor back one character
#define SEGGER_SHELL_ANSI_CURSOR_NEXT_LINE      SEGGER_SHELL_ANSI_CSI "E"  // CNL - cursor to start of next line
#define SEGGER_SHELL_ANSI_CURSOR_PREV_LINE      SEGGER_SHELL_ANSI_CSI "F"  // CPL - cursor to start of previous line
#define SEGGER_SHELL_ANSI_SAVE_CURSOR           SEGGER_SHELL_ANSI_CSI "s"  // SCP - save cursor position
#define SEGGER_SHELL_ANSI_RESTORE_CURSOR        SEGGER_SHELL_ANSI_CSI "u"  // RCP - restore cursor position
#define SEGGER_SHELL_ANSI_CURSOR_HOME           SEGGER_SHELL_ANSI_CSI "H"  // Move cursor to home position

/*********************************************************************
*
*       Cursor control sequences
*
*  Description
*    List of cursor control sequences.
*
*  Additional information
*    These control sequences conform to DECTCEM - DEC Text Cursor Enable Mode.
*/
#define SEGGER_SHELL_ANSI_HIDE_CURSOR           SEGGER_SHELL_ANSI_CSI "?25l" // Hide the cursor
#define SEGGER_SHELL_ANSI_SHOW_CURSOR           SEGGER_SHELL_ANSI_CSI "?25h" // Show the cursor

/*********************************************************************
*
*       Editing sequences
*
*  Description
*    List of editing sequences.
*/
#define SEGGER_SHELL_ANSI_ERASE_FORWARD         SEGGER_SHELL_ANSI_CSI "K"  // Erase to end of line
#define SEGGER_SHELL_ANSI_CLEAR_SCREEN          SEGGER_SHELL_ANSI_CSI "2J" // Erase the entire screen

/*********************************************************************
*
*       Attribute control sequences
*
*  Description
*    List of attribute sequences.
*/
#define SEGGER_SHELL_ANSI_RESET                 SEGGER_SHELL_ANSI_CSI "m"   // Reset all attributes
#define SEGGER_SHELL_ANSI_BRIGHT                SEGGER_SHELL_ANSI_CSI "1m"  // Enable bright (or bold) attribute
#define SEGGER_SHELL_ANSI_FEINT                 SEGGER_SHELL_ANSI_CSI "2m"  // Enable feint attribute
#define SEGGER_SHELL_ANSI_BLINK_SLOW            SEGGER_SHELL_ANSI_CSI "5m"  // Commonly render background as "bright"
#define SEGGER_SHELL_ANSI_NORMAL                SEGGER_SHELL_ANSI_CSI "22m" // Cancel bright and feint attributes
#define SEGGER_SHELL_ANSI_BLINK_OFF             SEGGER_SHELL_ANSI_CSI "25m" // Turn off blinking

/*********************************************************************
*
*       Text attribute sequences
*
*  Description
*    List of ANSI codes to set terminal text color.
*/
#define SEGGER_SHELL_ANSI_BLACK_TEXT            SEGGER_SHELL_ANSI_CSI "30m"  // Set foreground to black
#define SEGGER_SHELL_ANSI_RED_TEXT              SEGGER_SHELL_ANSI_CSI "31m"  // Set foreground to red
#define SEGGER_SHELL_ANSI_GREEN_TEXT            SEGGER_SHELL_ANSI_CSI "32m"  // Set foreground to green
#define SEGGER_SHELL_ANSI_YELLOW_TEXT           SEGGER_SHELL_ANSI_CSI "33m"  // Set foreground to yellow
#define SEGGER_SHELL_ANSI_BLUE_TEXT             SEGGER_SHELL_ANSI_CSI "34m"  // Set foreground to blue
#define SEGGER_SHELL_ANSI_MAGENTA_TEXT          SEGGER_SHELL_ANSI_CSI "35m"  // Set foreground to magenta
#define SEGGER_SHELL_ANSI_CYAN_TEXT             SEGGER_SHELL_ANSI_CSI "36m"  // Set foreground to cyan
#define SEGGER_SHELL_ANSI_WHITE_TEXT            SEGGER_SHELL_ANSI_CSI "37m"  // Set foreground to white

/*********************************************************************
*
*       Background attribute sequences
*
*  Description
*    List of ANSI codes to set terminal background color.
*/
#define SEGGER_SHELL_ANSI_BLACK_BACKGROUND      SEGGER_SHELL_ANSI_CSI "40m"  // Set background to black
#define SEGGER_SHELL_ANSI_RED_BACKGROUND        SEGGER_SHELL_ANSI_CSI "41m"  // Set background to red
#define SEGGER_SHELL_ANSI_GREEN_BACKGROUND      SEGGER_SHELL_ANSI_CSI "42m"  // Set background to green
#define SEGGER_SHELL_ANSI_YELLOW_BACKGROUND     SEGGER_SHELL_ANSI_CSI "43m"  // Set background to yellow
#define SEGGER_SHELL_ANSI_BLUE_BACKGROUND       SEGGER_SHELL_ANSI_CSI "44m"  // Set background to blue
#define SEGGER_SHELL_ANSI_MAGENTA_BACKGROUND    SEGGER_SHELL_ANSI_CSI "45m"  // Set background to magenta
#define SEGGER_SHELL_ANSI_CYAN_BACKGROUND       SEGGER_SHELL_ANSI_CSI "46m"  // Set background to cyan
#define SEGGER_SHELL_ANSI_WHITE_BACKGROUND      SEGGER_SHELL_ANSI_CSI "47m"  // Set background to white

/*********************************************************************
*
*       Compressed keyboard codes
*
*  Description
*    Logical keyboard codes.
*
*  Additional information
*    This is a single-item list of keyboard codes that
*    are translated from ANSI escape sequences.
*/
#define SEGGER_SHELL_KEY_CTRL_C     SEGGER_SHELL_KEY_CTRL('C')  // Ctrl+C
#define SEGGER_SHELL_KEY_CTRL_D     SEGGER_SHELL_KEY_CTRL('D')  // Ctrl+D
#define SEGGER_SHELL_KEY_CTRL_G     SEGGER_SHELL_KEY_CTRL('G')  // Ctrl+G
#define SEGGER_SHELL_KEY_CTRL_K     SEGGER_SHELL_KEY_CTRL('K')  // Ctrl+K
#define SEGGER_SHELL_KEY_CTRL_L     SEGGER_SHELL_KEY_CTRL('L')  // Ctrl+L
#define SEGGER_SHELL_KEY_CTRL_U     SEGGER_SHELL_KEY_CTRL('U')  // Ctrl+U
#define SEGGER_SHELL_KEY_CTRL_X     SEGGER_SHELL_KEY_CTRL('X')  // Ctrl+X
#define SEGGER_SHELL_KEY_ENTER      0x0D                        // Return key
#define SEGGER_SHELL_KEY_LF         0x0A                        // Line feed, also Ctrl+J
#define SEGGER_SHELL_KEY_ESCAPE     0x1B                        // Escape key
#define SEGGER_SHELL_KEY_BACKSPACE  0x7F                        // Backspace key
#define SEGGER_SHELL_KEY_DELETE     0x86                        // Delete forward
#define SEGGER_SHELL_KEY_INSERT     0x87                        // Insert
#define SEGGER_SHELL_KEY_LEFT       0x88                        // Cursor Left key
#define SEGGER_SHELL_KEY_RIGHT      0x89                        // Cursor Right key
#define SEGGER_SHELL_KEY_UP         0x8A                        // Cursor Up key
#define SEGGER_SHELL_KEY_DOWN       0x8B                        // Cursor Down key
#define SEGGER_SHELL_KEY_HOME       0x8C                        // Cursor Home key
#define SEGGER_SHELL_KEY_END        0x8D                        // Cursor End key
#define SEGGER_SHELL_KEY_PGUP       0x8E                        // Page Up key
#define SEGGER_SHELL_KEY_PGDN       0x8F                        // Page Down key
#define SEGGER_SHELL_KEY_F1         0xA1                        // Function key 1
#define SEGGER_SHELL_KEY_F2         0xA2                        // Function key 2
#define SEGGER_SHELL_KEY_F3         0xA3                        // Function key 3
#define SEGGER_SHELL_KEY_F4         0xA4                        // Function key 4
#define SEGGER_SHELL_KEY_F5         0xA5                        // Function key 5
#define SEGGER_SHELL_KEY_F6         0xA6                        // Function key 6
#define SEGGER_SHELL_KEY_F7         0xA7                        // Function key 7
#define SEGGER_SHELL_KEY_F8         0xA8                        // Function key 8
#define SEGGER_SHELL_KEY_F9         0xA9                        // Function key 9
#define SEGGER_SHELL_KEY_F10        0xAA                        // Function key 10
#define SEGGER_SHELL_KEY_F11        0xAB                        // Function key 11
#define SEGGER_SHELL_KEY_F12        0xAC                        // Function key 12
#define SEGGER_SHELL_KEY_F13        0xAD                        // Function key 13
#define SEGGER_SHELL_KEY_F14        0xAE                        // Function key 14
#define SEGGER_SHELL_KEY_F15        0xAF                        // Function key 15
#define SEGGER_SHELL_KEY_F16        0xB0                        // Function key 16
#define SEGGER_SHELL_KEY_F17        0xB1                        // Function key 17
#define SEGGER_SHELL_KEY_F18        0xB2                        // Function key 18
#define SEGGER_SHELL_KEY_F19        0xB3                        // Function key 19
#define SEGGER_SHELL_KEY_F20        0xB4                        // Function key 20
#define SEGGER_SHELL_KEY_HELP       0xB5                        // Help

/*********************************************************************
*
*       Keyboard modifier codes
*
*  Description
*    Logical keyboard modifier codes.
*
*  Description
*    This value is bitwise-or'd into a keyboard code when the
*    such modifiers care capable of being recognized.
*/
#define SEGGER_SHELL_KEY_MODIFIER_SHIFT   0x100   // Shift modifier
#define SEGGER_SHELL_KEY_MODIFIER_CTRL    0x200   // Control modifier (not set for typed ASCII control characters)
#define SEGGER_SHELL_KEY_MODIFIER_ALT     0x400   // Alternate or Meta modifier

/*********************************************************************
*
*       Types required for API
*
**********************************************************************
*/

struct SEGGER_SHELL_CONTEXT;

// A shell command with name, help, and execution function
typedef struct SEGGER_SHELL_COMMAND_API {
  const char * sName;
  const char * sDescription;
  const char * sSyntax;
  const char * sOptions;
  int        (*pfExecute)(struct SEGGER_SHELL_CONTEXT *pContext);
} SEGGER_SHELL_COMMAND_API;

// Linked list of shell commands with management data.
typedef struct SEGGER_SHELL_COMMAND {
  const SEGGER_SHELL_COMMAND_API * pAPI;
  struct SEGGER_SHELL_COMMAND    * pNext;
  U8                               Flags;
} SEGGER_SHELL_COMMAND;

typedef struct {
  void     (*pfPrintString) (const char *sFormat, va_list va);
  int      (*pfGetString)   (char *pInput, unsigned InputLen);
  unsigned (*pfGetWidth)    (void);
  unsigned (*pfGetHeight)   (void);
} SEGGER_SHELL_CONSOLE_API;

typedef struct {
  void     (*pfPrintString) (void *pContext, const char *sFormat, va_list va);
  int      (*pfGetString)   (void *pContext, char *pInput, unsigned InputLen);
  unsigned (*pfGetWidth)    (void *pContext);
  unsigned (*pfGetHeight)   (void *pContext);
} SEGGER_SHELL_CONSOLE_EX_API;

typedef enum {
  SEGGER_SHELL_INPUT_STATE_EMPTY,
  SEGGER_SHELL_INPUT_STATE_SS3,
  SEGGER_SHELL_INPUT_STATE_ESCAPE,
  SEGGER_SHELL_INPUT_STATE_CSI,
  SEGGER_SHELL_INPUT_STATE_ARG,
} SEGGER_SHELL_INPUT_STATE;

// Shell context contains enough to extract command arguments.
typedef struct SEGGER_SHELL_CONTEXT {
  unsigned                            argc;            // Well-known argc, count of command line arguments
  char                             ** argv;            // Well-known argv, vector of command line arguments
  char                              * pWork;           // Command line working storage
  unsigned                            ArgumentIdx;     // Index into arguments being extracted
  char                              * sProgramName;    // Selected command or provided program name
  SEGGER_SHELL_COMMAND              * pCommands;       // List of commands understood by the shell
  int                                 Done;            // Nonzero if the shell is closed
  const char                        * sName;           // Name of shell context.
  SEGGER_MEM_CONTEXT                * pMemory;         // Memory allocation API
  const SEGGER_SHELL_CONSOLE_API    * pConsoleAPI;     // Console I/O API
  const SEGGER_SHELL_CONSOLE_EX_API * pConsoleExAPI;   // Console I/O API with context
  void                              * pConsoleContext; // Pointer to user context pass through to console API
  //
  unsigned                            Cursor;          // Input cursor
  char                              * pBuf;            // Pointer to input buffer.
  unsigned                            BufLen;          // Capacity of input buffer.
  SEGGER_SHELL_INPUT_STATE            InputState;      // State of accumulation of ANSI control sequences
  unsigned                            AnsiArgLen;      // Number of arguments.
  unsigned                            aAnsiArg[2];     // Numeric arguments gathered, e.g. \x1B[5;1A would set ArgLen=2 with Arg[0]=5 and Arg[1]=1.
} SEGGER_SHELL_CONTEXT;

/*********************************************************************
*
*       Public const data
*
**********************************************************************
*/

// Common system commands.
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_Bye;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_Exit;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_Quit;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_Help;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_QuestionMark;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_Commands;

// emFile commands.
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Dir;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Ls;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Del;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Rm;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Mkdir;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Rd;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Rmdir;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Format;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Mount;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Unmount;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_FS_Volumes;

// emNet/IP commands.
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_IP_Dns;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_IP_Arp;
extern const SEGGER_SHELL_COMMAND_API SEGGER_SHELL_COMMAND_IP_Icmp;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

/*********************************************************************
*
*       Initialization functions
*/
void   SEGGER_SHELL_Init              (SEGGER_SHELL_CONTEXT *pSelf, const SEGGER_SHELL_CONSOLE_API *pConsoleAPI, SEGGER_MEM_CONTEXT *pMem);
void   SEGGER_SHELL_InitEx            (SEGGER_SHELL_CONTEXT *pSelf, const SEGGER_SHELL_CONSOLE_EX_API *pConsoleAPI, void *pConsoleContext, SEGGER_MEM_CONTEXT *pMem);
int    SEGGER_SHELL_ParseInput        (SEGGER_SHELL_CONTEXT *pSelf, char *sInput);
void   SEGGER_SHELL_Inherit           (SEGGER_SHELL_CONTEXT *pSelf, SEGGER_SHELL_CONTEXT *pFrom, const char *sName);
void   SEGGER_SHELL_InheritExternal   (SEGGER_SHELL_CONTEXT *pSelf, char *sProgramName, int argc, char *argv[]);

/*********************************************************************
*
*       Shell command set management functions
*/
int    SEGGER_SHELL_AddCommand        (SEGGER_SHELL_CONTEXT *pSelf, const SEGGER_SHELL_COMMAND_API *pCommand);

/*********************************************************************
*
*       Command line execution functions
*/
int    SEGGER_SHELL_Process           (SEGGER_SHELL_CONTEXT *pSelf);
int    SEGGER_SHELL_Enter             (SEGGER_SHELL_CONTEXT *pSelf);
void   SEGGER_SHELL_Exit              (SEGGER_SHELL_CONTEXT *pSelf);
int    SEGGER_SHELL_IsExited          (const SEGGER_SHELL_CONTEXT *pSelf);

/*********************************************************************
*
*       Command line utility functions
*/
void   SEGGER_SHELL_SetName           (SEGGER_SHELL_CONTEXT *pSelf, const char *sName);
void   SEGGER_SHELL_PrintHelp         (SEGGER_SHELL_CONTEXT *pSelf, unsigned Flags);
void   SEGGER_SHELL_PrintUsage        (SEGGER_SHELL_CONTEXT *pSelf, const SEGGER_SHELL_COMMAND_API *pAPI, unsigned Flags);
void   SEGGER_SHELL_PrintCommands     (SEGGER_SHELL_CONTEXT *pSelf);
int    SEGGER_SHELL_ExecuteHelp       (SEGGER_SHELL_CONTEXT *pSelf);
int    SEGGER_SHELL_FindCommand       (const SEGGER_SHELL_CONTEXT *pSelf, const char *sName, const SEGGER_SHELL_COMMAND_API **pCommandAPI);
void   SEGGER_SHELL_DecodeError       (int Status, char *sText, unsigned TextLen);

/*********************************************************************
*
*       Command line argument functions
*/
int    SEGGER_SHELL_ReadNextArg       (SEGGER_SHELL_CONTEXT *pSelf, char **pArg);
int    SEGGER_SHELL_PeekNextArg       (SEGGER_SHELL_CONTEXT *pSelf, char **pArg);
int    SEGGER_SHELL_CountTotalArgs    (const SEGGER_SHELL_CONTEXT *pSelf);
int    SEGGER_SHELL_CanReadArg        (const SEGGER_SHELL_CONTEXT *pSelf);
void   SEGGER_SHELL_ResetArgIterator  (SEGGER_SHELL_CONTEXT *pSelf, unsigned Index);
int    SEGGER_SHELL_ReadNextU32       (SEGGER_SHELL_CONTEXT *pSelf, U32 *pArg, U32 DefaultValue);

/*********************************************************************
*
*       Interactive input functions
*/
int    SEGGER_SHELL_GetWidth          (SEGGER_SHELL_CONTEXT *pSelf);
int    SEGGER_SHELL_GetHeight         (SEGGER_SHELL_CONTEXT *pSelf);
int    SEGGER_SHELL_InputText         (SEGGER_SHELL_CONTEXT *pSelf, char *sText, unsigned TextLen);
int    SEGGER_SHELL_INPUT_AddOctet    (SEGGER_SHELL_CONTEXT *pSelf, int Octet);
int    SEGGER_SHELL_INPUT_ProcessKey  (SEGGER_SHELL_CONTEXT *pSelf, int Key);
int    SEGGER_SHELL_INPUT_AddInput    (SEGGER_SHELL_CONTEXT *pSelf, int Octet);
void   SEGGER_SHELL_INPUT_EraseLine   (SEGGER_SHELL_CONTEXT *pSelf);
void   SEGGER_SHELL_INPUT_Start       (SEGGER_SHELL_CONTEXT *pSelf, char *pBuf, unsigned BufLen, unsigned Pos);
void   SEGGER_SHELL_INPUT_ReplaceText (SEGGER_SHELL_CONTEXT *pSelf, const char *sText);

/*********************************************************************
*
*       Shell output functions
*/
void   SEGGER_SHELL_Printf            (SEGGER_SHELL_CONTEXT *pSelf, const char *sFormat, ...);
void   SEGGER_SHELL_Printvf           (SEGGER_SHELL_CONTEXT *pSelf, const char *sFormat, va_list Params);
void   SEGGER_SHELL_Puts              (SEGGER_SHELL_CONTEXT *pSelf, const char *sText);
void   SEGGER_SHELL_PrintError        (SEGGER_SHELL_CONTEXT *pSelf, int Status);

/*********************************************************************
*
*       Add SEGGER product commands
*/
void   SEGGER_SHELL_AddCommands       (SEGGER_SHELL_CONTEXT *pSelf);
void   SEGGER_SHELL_FS_AddCommands    (SEGGER_SHELL_CONTEXT *pSelf);
void   SEGGER_SHELL_IP_AddCommands    (SEGGER_SHELL_CONTEXT *pSelf);

/*********************************************************************
*
*       Directory functions
*/
int    SEGGER_SHELL_PATH_Add          (char *sPath, unsigned PathLen, const char *sNew);
int    SEGGER_SHELL_PATH_AddExtension (char *sPath, unsigned PathLen, const char *sExt);
char * SEGGER_SHELL_PATH_Split        (char *sPath, unsigned PathLen);
int    SEGGER_SHELL_PATH_Match        (const char *sPat, const char *sStr);
int    SEGGER_SHELL_PATH_CaseMatch    (const char *sPat, const char *sStr);

/*********************************************************************
*
*       Compatibility macros
*/
#define SEGGER_SHELL_HasUnreadArgs(X) (SEGGER_SHELL_CanReadArg(X) > 0)
#define SEGGER_SHELL_AddCommandAPI    SEGGER_SHELL_AddCommand
#define SEGGER_SHELL_ByeAPI           SEGGER_SHELL_COMMAND_Bye
#define SEGGER_SHELL_ExitAPI          SEGGER_SHELL_COMMAND_Exit
#define SEGGER_SHELL_QuitAPI          SEGGER_SHELL_COMMAND_Quit
#define SEGGER_SHELL_HelpAPI          SEGGER_SHELL_COMMAND_Help
#define SEGGER_SHELL_QuestionMarkAPI  SEGGER_SHELL_COMMAND_QuestionMark
#define SEGGER_SHELL_CommandsAPI      SEGGER_SHELL_COMMAND_Commands


#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
