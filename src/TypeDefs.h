/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* TypeDefs.h                                                                  *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2018   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_TYPEDEFS_H_
#define _NP3_TYPEDEFS_H_

#include <stdbool.h>
#include "Sci_Position.h"

//~#define NP3_COMPILE_TEST 1

//
// SC_DOCUMENTOPTION_TEXT_LARGE
//
#if defined(SCI_LARGE_FILE_SUPPORT)
  typedef Sci_Position   DocPos;
  typedef Sci_PositionU  DocPosU;
  typedef Sci_PositionCR DocCR;
  typedef Sci_Line       DocLn;
  #define DOCPOSFMTA "%ti"
  #define DOCPOSFMTW L"%ti"
#else

  #ifdef NP3_COMPILE_TEST
    typedef ptrdiff_t DocPos;
    typedef size_t DocPosU;
    typedef long DocPosCR;
    typedef ptrdiff_t DocLn;
    #define DOCPOSFMTA "%ti"
    #define DOCPOSFMTW L"%ti"
  #else
    typedef int  DocPos;
    typedef unsigned int DocPosU;
    typedef long DocPosCR;
    typedef int  DocLn;
    #define DOCPOSFMTA "%i"
    #define DOCPOSFMTW L"%i"
  #endif

#endif

// --------------------------------------------------------------------------
    
typedef struct _wi
{
  int x;
  int y;
  int cx;
  int cy;
  bool max;
  int zoom;
} WININFO;

#define INIT_WININFO { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, false, 100 }


inline RECT RectFromWinInfo(const WININFO* const pWinInfo) {
  RECT rc; SetRect(&rc, pWinInfo->x, pWinInfo->y, pWinInfo->x + pWinInfo->cx, pWinInfo->y + pWinInfo->cy); return rc;
}

// ----------------------------------------------------------------------------

// keep backward compatible with older settings-file versions
typedef enum
{
  CFG_VER_NONE = 0, /// old version
  CFG_VER_0001 = 1,  /// ZoomLevel and PrintZoom changed from relative font size in point to absolute percentage.

  CFG_VER_CURRENT = CFG_VER_0001
} CFG_VERSION;

// --------------------------------------------------------------------------

typedef enum 
{
  MICRO_BUFFER = 32,
  MINI_BUFFER = 64,
  SMALL_BUFFER = 128,
  MIDSZ_BUFFER = 256,
  LARGE_BUFFER = 512,
  HUGE_BUFFER = 1024,
  XHUGE_BUFFER = 2048,
  XXXL_BUFFER = 4096,

  ANSI_CHAR_BUFFER = 258,
  FILE_ARG_BUF = MAX_PATH + 2,
  FNDRPL_BUFFER = 1024,
  LONG_LINES_MARKER_LIMIT = 4096

} BUFFER_SIZES;


typedef enum { FND_NOP = 0, NXT_NOT_FND, NXT_FND, NXT_WRP_FND, PRV_NOT_FND, PRV_FND, PRV_WRP_FND } FR_STATES;
typedef enum { FRMOD_IGNORE = 0, FRMOD_NORM, FRMOD_WRAPED } FR_UPD_MODES;
typedef enum { MBINFO = 0, MBWARN, MBYESNO, MBYESNOWARN, MBYESNOCANCEL, MBOKCANCEL, MBRETRYCANCEL } MBTYPES;

//==== Statusbar ==============================================================

typedef WCHAR prefix_t[MICRO_BUFFER];

typedef enum {
  STATUS_DOCLINE = 0, STATUS_DOCCOLUMN, STATUS_SELECTION, STATUS_SELCTBYTES, STATUS_SELCTLINES,
  STATUS_OCCURRENCE, STATUS_DOCSIZE, STATUS_CODEPAGE, STATUS_EOLMODE, STATUS_OVRMODE, STATUS_2ND_DEF,
  STATUS_LEXER, STATUS_DOCCHAR, STATUS_OCCREPLACE, STATUS_TINYEXPR,
  STATUS_SECTOR_COUNT,
  STATUS_HELP = 255
} STATUS_SECTOR_T;

#define SBS_INIT_ZERO  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } 
#define SBS_INIT_MINUS { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } 
#define SBS_INIT_ORDER { 0, 1, 2, 3, 4, 5, 6, 7. 8. 9, 10, 11, 12, 13, 14 }

#define STATUSBAR_SECTION_PREFIXES L"Ln  ,Col  ,Sel  ,Sb  ,SLn  ,Occ  ,,,,,,,Ch  ,Repl  ,Eval  ,"
#define STATUSBAR_SECTION_POSTFIXES L",,, [UTF-8],,, [UTF-8],,,,,,,,,"
#define STATUSBAR_DEFAULT_IDS  L"0 1 12 14 2 4 5 6 7 8 9 10 11"
#define STATUSBAR_SECTION_WIDTH_SPECS L"30 20 20 20 20 20 0 0 0 0 0 0 20 20 20"
#define STAUSBAR_RIGHT_MARGIN 20

// --------------------------------------------------------------------------

typedef enum { CT_NONE = 0, CT_ZOOM, CT_ZEROLEN_MATCH } CALLTIPTYPE;

// --------------------------------------------------------------------------

typedef struct _editfindreplace
{
  char szFind[FNDRPL_BUFFER];
  char szReplace[FNDRPL_BUFFER];
  UINT fuFlags;
  bool bTransformBS;
  bool bFindClose;
  bool bReplaceClose;
  bool bNoFindWrap;
  bool bWildcardSearch;
  bool bMarkOccurences;
  bool bHideNonMatchedLines;
  bool bDotMatchAll;
  bool bStateChanged;
  HWND hwnd;

} EDITFINDREPLACE, *LPEDITFINDREPLACE, *LPCEDITFINDREPLACE;

#define EFR_INIT_DATA  { "", "", 0, false, false, false, false, false, false, false, false, true, NULL }
#define IDMSG_SWITCHTOFIND    300
#define IDMSG_SWITCHTOREPLACE 301

// --------------------------------------------------------------------------

typedef struct _cmq
{
  HWND hwnd;
  UINT  cmd;
  WPARAM wparam;
  LPARAM lparam;
  int delay;
  struct _cmq* next;
  struct _cmq* prev;

} CmdMessageQueue_t;

#define MESSAGE_QUEUE_INIT = { NULL, WM_COMMAND, NULL, NULL, -1 };

// --------------------------------------------------------------------------

#define rgbRedColorRef    (RGB(255, 170, 170))
#define rgbGreenColorRef  (RGB(170, 255, 170))
#define rgbBlueColorRef   (RGB(170, 200, 255))

// --------------------------------------------------------------------------

#define MARKER_NP3_BOOKMARK      1

#define LINESTATE_OCCURRENCE_MARK 0x4

#define INDIC_NP3_MARK_OCCURANCE 1
#define INDIC_NP3_MATCH_BRACE    2
#define INDIC_NP3_BAD_BRACE      3


//=============================================================================

typedef struct _constants_t
{
  const WCHAR* FileBrowserMiniPath;

} CONSTANTS_T, *PCONSTANTS_T;

extern CONSTANTS_T Constants;

// ------------------------------------

typedef struct _globals_t
{
  HINSTANCE hInstance;
  HINSTANCE hPrevInst;
  HMODULE   hLngResContainer;
  HWND      hwndMain;
  HANDLE    hndlProcessHeap;

  WCHAR     WorkingDirectory[MAX_PATH + 1];

} GLOBALS_T, *PGLOBALS_T;

extern GLOBALS_T Globals;

// ------------------------------------

typedef struct _settings_t
{
  int RenderingTechnology;
  int Bidirectional;

} SETTINGS_T, *PSETTINGS_T;

extern SETTINGS_T Settings;

// ------------------------------------

typedef struct _settings2_t
{
  int    FileLoadWarningMB;
  int    OpacityLevel;
  DWORD  FileCheckInverval;
  DWORD  AutoReloadTimeout;
  int    IMEInteraction;
  int    SciFontQuality;
  int    MarkOccurrencesMaxCount;
  int    UpdateDelayHyperlinkStyling;
  int    UpdateDelayMarkAllOccurrences;
  bool   DenyVirtualSpaceAccess;
  bool   UseOldStyleBraceMatching;
  int    CurrentLineHorizontalSlop;
  int    CurrentLineVerticalSlop;

  WCHAR PreferredLanguageLocaleName[LOCALE_NAME_MAX_LENGTH+1];
  WCHAR DefaultExtension[64];
  WCHAR DefaultDirectory[MAX_PATH + 1];
  WCHAR FileDlgFilters[XHUGE_BUFFER];

  WCHAR FileBrowserPath[MAX_PATH + 1];
  WCHAR AppUserModelID[32];
  WCHAR ExtendedWhiteSpaceChars[ANSI_CHAR_BUFFER + 1];
  WCHAR AutoCompleteWordCharSet[ANSI_CHAR_BUFFER + 1];
  WCHAR TimeStamp[128];
  WCHAR DateTimeShort[128];
  WCHAR DateTimeLong[128];
  WCHAR WebTemplate1[MAX_PATH + 1];
  WCHAR WebTemplate2[MAX_PATH + 1];
  WCHAR AdministrationTool[MAX_PATH + 1];
  WCHAR DefaultWindowPosition[64];

} SETTINGS2_T, *PSETTINGS2_T;

extern SETTINGS2_T Settings2;

// ------------------------------------

typedef struct _flags_t
{
  int StickyWindowPosition;
  int ReuseWindow;
  int NoReuseWindow;
  int SingleFileInstance;
  int MultiFileArg;
  int RelativeFileMRU;
  int PortableMyDocs;
  int NoFadeHidden;
  int ToolbarLook;
  int SimpleIndentGuides;
  int NoHTMLGuess;
  int NoCGIGuess;
  int NoFileVariables;
  int ShellUseSystemMRU;
  int PrintFileAndLeave;

} FLAGS_T, *PFLAGS_T;

extern FLAGS_T Flags;



//=============================================================================

#endif //_NP3_TYPEDEFS_H_
