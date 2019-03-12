/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* TypeDefs.h                                                                  *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_TYPEDEFS_H_
#define _NP3_TYPEDEFS_H_

#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE      // don't allow deprecated functions
#include <strsafe.h>
#include <intsafe.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "Sci_Position.h"

//
// TODO(rkotten): 
// SCI_CREATEDOCUMENT (SC_DOCUMENTOPTION_TEXT_LARGE)
//

/// deprecated:
///typedef int            DocPos;
///typedef unsigned int   DocPosU;
///typedef long           DocPosCR;
///typedef int            DocLn;
///#define DOCPOSFMTA "%i"
///#define DOCPOSFMTW L"%i"

typedef Sci_Position   DocPos;
typedef Sci_PositionU  DocPosU;
typedef Sci_PositionCR DocPosCR;
typedef DocPos         DocLn;   // Sci::Line
#define DOCPOSFMTA "%ti"
#define DOCPOSFMTW L"%ti"

// TODO(rkotten): refactoring of MultiByteToWideChar / WideCharToMultiByte DocPos casting refactoring
typedef int MBWC_DocPos_Cast; 

// --------------------------------------------------------------------------
    
typedef struct _dpi_t
{
  UINT x;
  UINT y;
} DPI_T;

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

typedef enum { BACKGROUND_LAYER = 0, FOREGROUND_LAYER = 1 } COLOR_LAYER;  // Style_GetColor()
typedef enum { OPEN_WITH_BROWSER = 1, OPEN_WITH_NOTEPAD3 = 2, COPY_HYPERLINK = 4 } HYPERLINK_OPS;  // Hyperlink Operations

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
  FNDRPL_BUFFER = 2048,
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

typedef enum { CT_NONE = 0, CT_ZOOM, CT_ZEROLEN_MATCH, CT_ENC_INFO } CALLTIPTYPE;

// --------------------------------------------------------------------------

typedef struct _filevars
{

  int mask;
  int iTabWidth;
  int iIndentWidth;
  bool bTabsAsSpaces;
  bool bTabIndents;
  bool fWordWrap;
  int iLongLinesLimit;
  char tchEncoding[64];
  int  iEncoding;
  char tchMode[32];

} FILEVARS, *LPFILEVARS;

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

#define MRU_MAXITEMS    32
#define MRU_ITEMSFILE   32
#define MRU_ITEMSFNDRPL 16
#define MRU_NOCASE    1
#define MRU_UTF8      2
#define MRU_BMRK_SIZE 512

typedef struct _mrulist
{
  LPCWSTR szRegKey;
  int     iFlags;
  int     iSize;
  LPWSTR  pszItems[MRU_MAXITEMS];
  int     iEncoding[MRU_MAXITEMS];
  DocPos  iCaretPos[MRU_MAXITEMS];
  LPWSTR  pszBookMarks[MRU_MAXITEMS];
}
MRULIST, *PMRULIST, *LPMRULIST;

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

#define LINESTATE_OCCURRENCE_MARK (1 << 13) 

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
  HINSTANCE hLngResContainer;
  int       iAvailLngCount;
  bool      bPrefLngNotAvail;
  HWND      hwndMain;
  HANDLE    hndlProcessHeap;
  HWND      hwndEdit;
  HANDLE    hndlScintilla;
  HWND      hwndStatus;
  DWORD     dwLastError;
  HMENU     hMainMenu;
  HICON     hDlgIcon;
  HICON     hIcon128;
  HWND      hwndDlgFindReplace;
  HWND      hwndDlgCustomizeSchemes;
  int       iDefaultCharSet;
  DPI_T     CurrentDPI;
  DPI_T     CurrentPPI;
  LANGID    iPrefLANGID;
  LPMRULIST pFileMRU;
  LPMRULIST pMRUfind;
  LPMRULIST pMRUreplace;
  CALLTIPTYPE CallTipType;
  FILEVARS  fvCurFile;
  bool      bWordWrap;
  bool      bTabsAsSpaces;
  bool      bTabIndents;
  int       iTabWidth;
  int       iIndentWidth;
  int       iLongLinesLimit;
  int       iWrapCol;

  bool      bCodeFoldingAvailable;
  bool      bForceLoadASCIIasUTF8;
  bool      bZeroBasedColumnIndex;
  bool      bZeroBasedCharacterCount;
  int       iReplacedOccurrences;
  int       iMarkOccurrencesCount;
  bool      bChasingDocTail;
  bool      bUseLimitedAutoCCharSet;
  bool      bIsCJKInputCodePage;
  bool      bIniFileFromScratch;
  bool      bFindReplCopySelOrClip;
  bool      bReplaceInitialized;

  FR_STATES FindReplaceMatchFoundState;

  WCHAR     WorkingDirectory[MAX_PATH];
  WCHAR     IniFile[MAX_PATH];
  WCHAR     CurrentFile[MAX_PATH];

} GLOBALS_T, *PGLOBALS_T;

extern GLOBALS_T Globals;

// ------------------------------------

typedef struct _settings_t
{
  bool SaveSettings;
  bool SaveRecentFiles;
  bool PreserveCaretPos;
  bool SaveFindReplace;
  int PathNameFormat;
  bool WordWrap;
  int WordWrapMode;
  int WordWrapIndent;
  int WordWrapSymbols;
  bool ShowWordWrapSymbols;
  bool MatchBraces;
  bool AutoCloseTags;
  int  HighlightCurrentLine;
  bool HyperlinkHotspot;
  bool ScrollPastEOF;
  bool AutoIndent;
  bool AutoCompleteWords;
  bool AutoCLexerKeyWords;
  bool AccelWordNavigation;
  bool ShowIndentGuides;
  bool TabsAsSpaces;
  bool TabIndents;
  bool BackspaceUnindents;
  int TabWidth;
  int IndentWidth;
  bool WarnInconsistentIndents;
  bool AutoDetectIndentSettings;
  bool MarkLongLines;
  int LongLinesLimit;
  int LongLineMode;
  bool ShowSelectionMargin;
  bool ShowLineNumbers;
  bool ShowCodeFolding;
  int MarkOccurrences;
  bool MarkOccurrencesMatchVisible;
  bool MarkOccurrencesMatchCase;
  bool MarkOccurrencesMatchWholeWords;
  bool MarkOccurrencesCurrentWord;
  bool ViewWhiteSpace;
  bool ViewEOLs;
  int DefaultEncoding; // default new file encoding
  bool UseDefaultForFileEncoding;
  bool LoadASCIIasUTF8;
  bool UseReliableCEDonly;
  bool LoadNFOasOEM;
  bool NoEncodingTags;
  bool SkipUnicodeDetection;
  bool SkipANSICodePageDetection;
  int DefaultEOLMode;
  bool WarnInconsistEOLs;
  bool FixLineEndings;
  bool FixTrailingBlanks;
  int PrintHeader;
  int PrintFooter;
  int PrintColorMode;
  int PrintZoom;
  bool SaveBeforeRunningTools;
  int FileWatchingMode;
  bool ResetFileWatching;
  int EscFunction;
  bool AlwaysOnTop;
  bool MinimizeToTray;
  bool TransparentMode;
  bool FindReplaceTransparentMode;
  int RenderingTechnology;
  int Bidirectional;
  bool ShowToolbar;
  bool ShowStatusbar;
  int EncodingDlgSizeX;
  int EncodingDlgSizeY;
  int RecodeDlgSizeX;
  int RecodeDlgSizeY;
  int FileMRUDlgSizeX;
  int FileMRUDlgSizeY;
  int OpenWithDlgSizeX;
  int OpenWithDlgSizeY;
  int FavoritesDlgSizeX;
  int FavoritesDlgSizeY;
  int FindReplaceDlgPosX;
  int FindReplaceDlgPosY;
  int CustomSchemesDlgPosX;
  int CustomSchemesDlgPosY;

  RECT PrintMargin;
  EDITFINDREPLACE EFR_Data;
  WCHAR OpenWithDir[MAX_PATH];
  WCHAR FavoritesDir[MAX_PATH];
  WCHAR ToolbarButtons[MIDSZ_BUFFER];

} SETTINGS_T, *PSETTINGS_T;

extern SETTINGS_T Settings;

// ------------------------------------

typedef struct _flags_t
{
  bool bDevDebugMode;
  bool bStickyWindowPosition;
  bool bReuseWindow;
  bool bSingleFileInstance;
  int fReuseWindow;
  int fSingleFileInstance;
  int fNoReuseWindow;

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

} FLAGS_T, * PFLAGS_T;

extern FLAGS_T Flags;

// ------------------------------------

typedef struct _settings2_t
{
  int    FileLoadWarningMB;
  int    OpacityLevel;
  int    FindReplaceOpacityLevel;
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

  float  AnalyzeReliableConfidenceLevel;
  float  ReliableCEDConfidenceMapping;   // = 0.66f;
  float  UnReliableCEDConfidenceMapping; //= 0.20f;

  WCHAR PreferredLanguageLocaleName[LOCALE_NAME_MAX_LENGTH+1];
  WCHAR DefaultExtension[64];
  WCHAR DefaultDirectory[MAX_PATH];
  WCHAR FileDlgFilters[XHUGE_BUFFER];

  WCHAR FileBrowserPath[MAX_PATH];
  WCHAR AppUserModelID[32];
  WCHAR ExtendedWhiteSpaceChars[ANSI_CHAR_BUFFER + 1];
  WCHAR AutoCompleteWordCharSet[ANSI_CHAR_BUFFER + 1];
  WCHAR TimeStamp[128];
  WCHAR DateTimeShort[128];
  WCHAR DateTimeLong[128];
  WCHAR WebTemplate1[MAX_PATH];
  WCHAR WebTemplate2[MAX_PATH];
  WCHAR AdministrationTool[MAX_PATH];
  WCHAR DefaultWindowPosition[64];

} SETTINGS2_T, *PSETTINGS2_T;

extern SETTINGS2_T Settings2;

//=============================================================================

typedef enum { I_TAB_LN = 0, I_SPC_LN = 1, I_MIX_LN = 2, I_TAB_MOD_X = 3, I_SPC_MOD_X = 4 } INDENT_TYPE;

typedef struct _editfileiostatus
{
  int iEncoding;
  int iEOLMode;

  bool bFileTooBig;
  bool bUnicodeErr;

  // inconsistent line endings
  bool bInconsistentEOLs;
  DocLn eolCount[3];

  bool bCancelDataLoss;
  bool bUnknownExt;

  // inconsistent indentation
  INDENT_TYPE iGlobalIndent;
  DocLn indentCount[5];

} EditFileIOStatus;

#define INIT_FILEIO_STATUS { CPI_ANSI_DEFAULT, SC_EOL_CRLF, false, false, false, {0,0,0}, false, false, I_MIX_LN, {0,0,0,0,0} }

//=============================================================================

#endif //_NP3_TYPEDEFS_H_
