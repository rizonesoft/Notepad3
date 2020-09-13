// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* TypeDefs.h                                                                  *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_TYPEDEFS_H_
#define _NP3_TYPEDEFS_H_

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#ifndef WINVER
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif

// Want to use std::min and std::max so don't want Windows.h version of min and max
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE      // don't allow deprecated functions
#include <strsafe.h>
#include <intsafe.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "Sci_Position.h"
#include "Scintilla.h"

//
// TODO: 
// SCI_CREATEDOCUMENT (SC_DOCUMENTOPTION_TEXT_LARGE)
//

/// deprecated:
///typedef long           DocPos;
///typedef unsigned long  DocPosU;
///typedef long           DocPosCR;
///typedef long           DocLn;
///#define DOCPOSFMTA "%i"
///#define DOCPOSFMTW L"%i"

typedef Sci_Position   DocPos;
typedef Sci_PositionU  DocPosU;
typedef Sci_PositionCR DocPosCR;
typedef DocPos         DocLn;   // Sci::Line
#define DOCPOSFMTA "%ti"
#define DOCPOSFMTW L"%ti"

// --------------------------------------------------------------------------

//typedef intptr_t cpi_enc_t;
typedef int cpi_enc_t;

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

typedef enum { SCR_NORMAL = 0, SCR_FULL_SCREEN = 1 } SCREEN_MODE;

inline RECT RectFromWinInfo(const WININFO* const pWinInfo) {
  RECT rc; SetRect(&rc, pWinInfo->x, pWinInfo->y, pWinInfo->x + pWinInfo->cx, pWinInfo->y + pWinInfo->cy); return rc;
}

// ----------------------------------------------------------------------------

typedef enum { BACKGROUND_LAYER = 0, FOREGROUND_LAYER = 1 } COLOR_LAYER;  // Style_GetColor()
typedef enum { OPEN_WITH_BROWSER = 1, OPEN_WITH_NOTEPAD3 = 2, COPY_HYPERLINK = 4, SELECT_HYPERLINK = 8 } HYPERLINK_OPS;  // Hyperlink Operations
typedef enum { FWM_DONT_CARE = 0, FWM_MSGBOX = 1, FWM_AUTORELOAD = 2 } FILE_WATCHING_MODE;
typedef enum { FVMM_MARGIN = 1, FVMM_LN_BACKGR = 2, FVMM_FOLD = 4 } FOCUSVIEW_MARKER_MODE;

// ----------------------------------------------------------------------------

// keep backward compatible with older settings-file versions
typedef enum
{
  CFG_VER_NONE = 0,  /// old version
  CFG_VER_0001 = 1,  /// ZoomLevel and PrintZoom changed from relative font size in point to absolute percentage.
  CFG_VER_0002 = 2,  /// LongLine Marker Off by default
  CFG_VER_0003 = 3,  /// SimpleIni UTF-8 BOM
  CFG_VER_0004 = 4,  /// Text Files lexer vs. Default Text => (2nd) Common Style

  CFG_VER_CURRENT = CFG_VER_0004

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

#define STATUSBAR_DEFAULT_IDS  L"0 1 12 14 2 4 5 6 7 8 9 10 11"
#define STATUSBAR_SECTION_WIDTH_SPECS L"30 20 20 20 20 20 0 0 0 0 0 0 20 20 20"
#define STAUSBAR_RIGHT_MARGIN 20

#define MB_ICONSHIELD 0x000000B0L

#define COLORREF_MAX (DWORD_MAX)
// --------------------------------------------------------------------------

typedef enum { CT_NONE = 0, CT_ZOOM, CT_ZEROLEN_MATCH, CT_ENC_INFO, CT_DWELL } CALLTIPTYPE;

// --------------------------------------------------------------------------

typedef struct _filevars
{
  int        mask;
  bool       bTabsAsSpaces;
  bool       bTabIndents;
  bool       bWordWrap;
  int        iTabWidth;
  int        iIndentWidth;
  int        iWrapColumn;
  cpi_enc_t  iEncoding;
  char       chMode[32];
  char       chEncoding[64];
  WCHAR      wchMultiEdgeLines[SMALL_BUFFER];

} FILEVARS, *LPFILEVARS;

// --------------------------------------------------------------------------

typedef struct _editfindreplace
{
  UINT fuFlags;
  bool bTransformBS;
  bool bFindClose;
  bool bReplaceClose;
  bool bOverlappingFind;
  bool bNoFindWrap;
  bool bWildcardSearch;
  bool bMarkOccurences;
  bool bHideNonMatchedLines;
  bool bStateChanged;
  HWND hwnd;
  char szFind[FNDRPL_BUFFER];
  char szReplace[FNDRPL_BUFFER];

} EDITFINDREPLACE, *LPEDITFINDREPLACE, *LPCEDITFINDREPLACE;

#define INIT_EFR_DATA  { 0, false, false, false, false, false, false, false, false, true, NULL, "", ""  }
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
  LPCWSTR   szRegKey;
  int       iFlags;
  int       iSize;
  LPWSTR    pszItems[MRU_MAXITEMS];
  cpi_enc_t iEncoding[MRU_MAXITEMS];
  DocPos    iCaretPos[MRU_MAXITEMS];
  DocPos    iSelAnchPos[MRU_MAXITEMS];
  LPWSTR    pszBookMarks[MRU_MAXITEMS];
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

#define MESSAGE_QUEUE_INIT(cmd,wp,lp)  { NULL, (cmd), MAKEWPARAM((wp), 1), ((LPARAM)(DWORD)(lp)), -1, NULL, NULL }
#define MQ_WM_CMD_INIT(wp,lp)            MESSAGE_QUEUE_INIT(WM_COMMAND, (wp), (lp))

// --------------------------------------------------------------------------

#define rgbRedColorRef    (RGB(255, 170, 170))
#define rgbGreenColorRef  (RGB(170, 255, 170))
#define rgbBlueColorRef   (RGB(170, 200, 255))

// --------------------------------------------------------------------------

typedef enum
{
  MARKER_NP3_OCCURRENCE = 0, // invisible
  MARKER_NP3_1,
  MARKER_NP3_2,
  MARKER_NP3_3,
  MARKER_NP3_4,
  MARKER_NP3_5,
  MARKER_NP3_6,
  MARKER_NP3_7,
  MARKER_NP3_8,
  // std bookmark -> counter is last
  MARKER_NP3_BOOKMARK
} MARKER_ID;

// ASSERT( MARKER_NP3_BOOKMARK < SC_MARKNUM_FOLDEREND )

#define OCCURRENCE_MARKER_BITMASK() (bitmask32_n(MARKER_NP3_BOOKMARK + 1) & ~(1 << MARKER_NP3_OCCURRENCE))

extern LPCWSTR WordBookMarks[];

// --------------------------------------------------------------------------


typedef enum
{
  INDIC_NP3_MARK_OCCURANCE = INDICATOR_CONTAINER,
  INDIC_NP3_MATCH_BRACE,
  INDIC_NP3_BAD_BRACE,
  INDIC_NP3_HYPERLINK,
  INDIC_NP3_HYPERLINK_U,
  INDIC_NP3_COLOR_DEF,
  INDIC_NP3_COLOR_DEF_T,
  INDIC_NP3_MULTI_EDIT,
  INDIC_NP3_UNICODE_POINT,
  // counter is last
  INDIC_NP3_ID_CNT
} INDIC_ID;

// ASSERT( INDIC_NP3_ID_CNT < INDICATOR_IME )

// --------------------------------------------------------------------------

#define SC_NP3_CURSORHAND 8

//=============================================================================

typedef struct _constants_t
{
  int const          StdDefaultLexerID; // Pure Text Files
  const WCHAR* const FileBrowserMiniPath;
  const WCHAR* const FileSearchGrepWin;
  const WCHAR* const StylingThemeName;

  const WCHAR* const Settings_Section;
  const WCHAR* const Settings2_Section;
  const WCHAR* const Window_Section;
  const WCHAR* const Styles_Section;
  const WCHAR* const SectionSuppressedMessages;

} CONSTANTS_T, *PCONSTANTS_T;

extern CONSTANTS_T const Constants;

// ------------------------------------

typedef struct _globals_t
{
  int       iCfgVersionRead;
  HINSTANCE hInstance;
  HINSTANCE hPrevInst;
  HINSTANCE hLngResContainer;
  bool      bCanSaveIniFile;
  int       iAvailLngCount;
  bool      bPrefLngNotAvail;
  HWND      hwndMain;
  HANDLE    hndlProcessHeap;
  HWND      hwndEdit;
  HANDLE    hndlScintilla;
  HANDLE    hwndToolbar;
  HWND      hwndStatus;
  DWORD     dwLastError;
  HMENU     hMainMenu;
  HICON     hDlgIcon256;   // Notepad3 Icon (256x256)
  HICON     hDlgIcon128;   // Notepad3 Icon (128x128)
  HICON     hDlgIconBig;
  HICON     hDlgIconSmall;
  HICON     hDlgIconPrefs256;
  HICON     hDlgIconPrefs128;
  HICON     hDlgIconPrefs64;
  HICON     hIconMsgUser;
  HICON     hIconMsgInfo;
  HICON     hIconMsgWarn;
  HICON     hIconMsgError;
  HICON     hIconMsgQuest;
  HICON     hIconMsgShield;
  //HICON     hIconMsgWinLogo;
  HWND      hwndDlgFindReplace;
  HWND      hwndDlgCustomizeSchemes;
  int       iDefaultCharSet;
  cpi_enc_t DOSEncoding;
  LANGID    iPrefLANGID;
  LPMRULIST pFileMRU;
  LPMRULIST pMRUfind;
  LPMRULIST pMRUreplace;
  CALLTIPTYPE CallTipType;
  FILEVARS  fvCurFile;
  int       iWrapCol;
  int       InitialFontSize;

  bool      CmdLnFlag_PosParam;
  int       CmdLnFlag_WindowPos;
  int       CmdLnFlag_ReuseWindow;
  int       CmdLnFlag_SingleFileInstance;
  int       CmdLnFlag_MultiFileArg;
  int       CmdLnFlag_ShellUseSystemMRU;
  int       CmdLnFlag_PrintFileAndLeave;
   
  bool      bZeroBasedColumnIndex;
  bool      bZeroBasedCharacterCount;
  int       iReplacedOccurrences;
  DocPosU   iMarkOccurrencesCount;
  bool      bUseLimitedAutoCCharSet;
  bool      bIsCJKInputCodePage;
  bool      bIniFileFromScratch;
  bool      bFindReplCopySelOrClip;
  bool      bReplaceInitialized;
  bool      bDocHasInconsistentEOLs;
  unsigned  idxSelectedTheme;

  FR_STATES FindReplaceMatchFoundState;

  WCHAR     SelectedThemeName[SMALL_BUFFER];
  WCHAR     WorkingDirectory[MAX_PATH];
  WCHAR     IniFile[MAX_PATH];
  WCHAR     IniFileDefault[MAX_PATH];
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
  int  PathNameFormat;
  bool WordWrap;
  int  WordWrapMode;
  int  WordWrapIndent;
  int  WordWrapSymbols;
  bool ShowWordWrapSymbols;
  bool MatchBraces;
  bool AutoCloseTags;
  int  HighlightCurrentLine;
  bool HyperlinkHotspot;
  int  ColorDefHotspot;
  bool ScrollPastEOF;
  bool ShowHypLnkToolTip;
  bool HighlightUnicodePoints;
  bool AutoIndent;
  bool AutoCompleteWords;
  bool AutoCLexerKeyWords;
  bool AccelWordNavigation;
  bool EditLineCommentBlock;
  bool ShowIndentGuides;
  bool TabsAsSpaces;
  bool TabIndents;
  bool BackspaceUnindents;
  int  TabWidth;
  int  IndentWidth;
  bool WarnInconsistentIndents;
  bool AutoDetectIndentSettings;
  bool MarkLongLines;
  int  LongLinesLimit;
  int  LongLineMode;
  bool ShowBookmarkMargin;
  bool ShowLineNumbers;
  bool ShowCodeFolding;
  bool MarkOccurrences;
  bool MarkOccurrencesBookmark;
  bool MarkOccurrencesMatchVisible;
  bool MarkOccurrencesMatchCase;
  bool MarkOccurrencesMatchWholeWords;
  bool MarkOccurrencesCurrentWord;
  bool ViewWhiteSpace;
  bool ViewEOLs;
  cpi_enc_t DefaultEncoding; // default new file encoding
  bool UseDefaultForFileEncoding;
  bool LoadASCIIasUTF8;
  bool UseReliableCEDonly;
  bool LoadNFOasOEM;
  bool NoEncodingTags;
  bool SkipUnicodeDetection;
  bool SkipANSICodePageDetection;
  int  DefaultEOLMode;
  bool WarnInconsistEOLs;
  bool FixLineEndings;
  bool FixTrailingBlanks;
  int  PrintHeader;
  int  PrintFooter;
  int  PrintColorMode;
  int  PrintZoom;
  bool SaveBeforeRunningTools;
  bool EvalTinyExprOnSelection;
  FILE_WATCHING_MODE FileWatchingMode;
  bool ResetFileWatching;
  int  EscFunction;
  bool AlwaysOnTop;
  bool MinimizeToTray;
  bool TransparentMode;
  bool FindReplaceTransparentMode;
  int  RenderingTechnology;
  int  Bidirectional;
  bool ShowMenubar;
  bool ShowToolbar;
  bool ShowStatusbar;
  int  ToolBarTheme;
  bool DpiScaleToolBar;
  int  EncodingDlgSizeX;
  int  EncodingDlgSizeY;
  int  RecodeDlgSizeX;
  int  RecodeDlgSizeY;
  int  FileMRUDlgSizeX;
  int  FileMRUDlgSizeY;
  int  OpenWithDlgSizeX;
  int  OpenWithDlgSizeY;
  int  FavoritesDlgSizeX;
  int  FavoritesDlgSizeY;
  int  AddToFavDlgSizeX;
  int  FindReplaceDlgPosX;
  int  FindReplaceDlgPosY;
  int  CustomSchemesDlgPosX;
  int  CustomSchemesDlgPosY;
  bool MuteMessageBeep;
  bool SplitUndoTypingSeqOnLnBreak;
  bool EditLayoutRTL;
  bool DialogsLayoutRTL;
  int  FocusViewMarkerMode;

  RECT PrintMargin;
  EDITFINDREPLACE EFR_Data;
  WCHAR OpenWithDir[MAX_PATH];
  WCHAR FavoritesDir[MAX_PATH];
  WCHAR ToolbarButtons[MIDSZ_BUFFER];
  WCHAR MultiEdgeLines[MIDSZ_BUFFER];

} SETTINGS_T, *PSETTINGS_T;

extern SETTINGS_T Defaults;
extern SETTINGS_T Settings;

#define IsMarkOccurrencesEnabled() (Settings.MarkOccurrences)
#define IsFocusedViewAllowed() (IsMarkOccurrencesEnabled() && !Settings.MarkOccurrencesMatchVisible)
#define IsColorDefHotspotEnabled() (Settings.ColorDefHotspot != 0)

//=============================================================================

typedef struct _flags_t
{
  int  ToolbarLook;
  int PrintFileAndLeave;

  bool bLargeFileLoaded;
  bool bDevDebugMode;
  bool bStickyWindowPosition;
  bool bReuseWindow;
  bool bSingleFileInstance;
  bool MultiFileArg;
  bool RelativeFileMRU;
  bool PortableMyDocs;
  bool NoFadeHidden;
  bool SimpleIndentGuides;
  bool NoHTMLGuess;
  bool NoCGIGuess;
  bool NoFileVariables;
  bool ShellUseSystemMRU;
  bool bPreserveFileModTime;

  bool bDoRelaunchElevated;
  bool bSearchPathIfRelative;

  bool bSettingsFileSoftLocked;

} FLAGS_T, *PFLAGS_T;

extern FLAGS_T Flags;
extern FLAGS_T DefaultFlags;

//=============================================================================

typedef struct _settings2_t
{
  int    FileLoadWarningMB;
  int    OpacityLevel;
  int    FindReplaceOpacityLevel;
  DWORD  FileCheckInverval;
  DWORD  AutoReloadTimeout;
  DWORD  UndoTransactionTimeout;
  int    IMEInteraction;
  int    SciFontQuality;

  int    UpdateDelayMarkAllOccurrences;
  bool   DenyVirtualSpaceAccess;
  bool   UseOldStyleBraceMatching;
  int    CurrentLineHorizontalSlop;
  int    CurrentLineVerticalSlop;
  bool   NoCopyLineOnEmptySelection;
  bool   NoCutLineOnEmptySelection;
  bool   LexerSQLNumberSignAsComment;
  int    ExitOnESCSkipLevel;
  int    ZoomTooltipTimeout;
  int    LargeIconScalePrecent;

  float  AnalyzeReliableConfidenceLevel;
  float  LocaleAnsiCodePageAnalysisBonus;

  WCHAR PreferredLanguageLocaleName[LOCALE_NAME_MAX_LENGTH + 1];
  WCHAR DefaultExtension[MINI_BUFFER];
  WCHAR DefaultDirectory[MAX_PATH];
  WCHAR FileDlgFilters[XHUGE_BUFFER];

  WCHAR FileBrowserPath[MAX_PATH];
  WCHAR GrepWinPath[MAX_PATH];
  WCHAR AppUserModelID[SMALL_BUFFER];
  WCHAR AutoCompleteFillUpChars[MINI_BUFFER];
  WCHAR LineCommentPostfixStrg[MINI_BUFFER];
  WCHAR ExtendedWhiteSpaceChars[ANSI_CHAR_BUFFER + 1];
  WCHAR AutoCompleteWordCharSet[ANSI_CHAR_BUFFER + 1];

  WCHAR DateTimeFormat[SMALL_BUFFER];
  WCHAR DateTimeLongFormat[SMALL_BUFFER];
  WCHAR TimeStampRegEx[SMALL_BUFFER];
  WCHAR TimeStampFormat[SMALL_BUFFER];

  WCHAR WebTemplate1[MAX_PATH];
  WCHAR WebTemplate2[MAX_PATH];
  WCHAR AdministrationTool[MAX_PATH];
  WCHAR DefaultWindowPosition[MINI_BUFFER];

} SETTINGS2_T, *PSETTINGS2_T;

extern SETTINGS2_T Settings2;
extern SETTINGS2_T Defaults2;

//=============================================================================
#if 0
typedef enum {
  FONT_STRETCH_UNDEFINED = 0,
  FONT_STRETCH_ULTRA_CONDENSED,
  FONT_STRETCH_EXTRA_CONDENSED,
  FONT_STRETCH_CONDENSED,
  FONT_STRETCH_SEMI_CONDENSED,
  FONT_STRETCH_NORMAL,
  FONT_STRETCH_MEDIUM,
  FONT_STRETCH_SEMI_EXPANDED,
  FONT_STRETCH_EXPANDED,
  FONT_STRETCH_EXTRA_EXPANDED,
  FONT_STRETCH_ULTRA_EXPANDED
} FONT_STRETCH_T;
#endif
//=============================================================================

typedef struct _focusedview_t
{
  bool HideNonMatchedLines;
  bool CodeFoldingAvailable;
  bool ShowCodeFolding;       // <-> Settings.ShowCodeFolding

} FOCUSEDVIEW_T, *PFOCUSEDVIEW_T;

extern FOCUSEDVIEW_T FocusedView;

//=============================================================================

typedef struct _filewatching_t
{
  FILE_WATCHING_MODE flagChangeNotify;  // <-> s_flagChangeNotify;
  FILE_WATCHING_MODE FileWatchingMode;  // <-> Settings.FileWatchingMode;
  bool ResetFileWatching;               // <-> Settings.ResetFileWatching;
  DWORD FileCheckInverval;              // <-> Settings2.FileCheckInverval;
  DWORD AutoReloadTimeout;              // <-> Settings2.AutoReloadTimeout;
  bool MonitoringLog;

} FILEWATCHING_T, *PFILEWATCHING_T;

extern FILEWATCHING_T FileWatching;

//=============================================================================

typedef enum { I_TAB_LN = 0, I_SPC_LN = 1, I_MIX_LN = 2, I_TAB_MOD_X = 3, I_SPC_MOD_X = 4 } INDENT_TYPE;

typedef struct _editfileiostatus
{
  cpi_enc_t iEncoding;
  int iEOLMode;
  bool bUnicodeErr;
  bool bCancelDataLoss;
  bool bUnknownExt;
  bool bEncryptedRaw;

  // inconsistent line endings
  bool bInconsistentEOLs;
  // inconsistent indentation
  INDENT_TYPE iGlobalIndent;

  DocLn eolCount[3];
  DocLn indentCount[5];

} EditFileIOStatus;

#define INIT_FILEIO_STATUS { CPI_ANSI_DEFAULT, SC_EOL_CRLF, false, false, false, false, false, I_MIX_LN, {0,0,0}, {0,0,0,0,0} }

//=============================================================================

typedef struct _themeFiles
{
  UINT    rid;
  WCHAR   szName[MINI_BUFFER];
  WCHAR   szFilePath[MAX_PATH];

} THEMEFILES, * PTHEMEFILES;

//=============================================================================

// ---------   common defines   --------

#define NOTEPAD3_MODULE_DIR_ENV_VAR  L"NOTEPAD3MODULEDIR"

//~#define NP3_VIRTUAL_SPACE_ACCESS_OPTIONS (Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : (SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART | SCVS_USERACCESSIBLE))
//~#define NP3_VIRTUAL_SPACE_ACCESS_OPTIONS (Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : (SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART))
// don't use 'SCVS_NOWRAPLINESTART'
#define NP3_VIRTUAL_SPACE_ACCESS_OPTIONS (Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION)

// from <wininet.h>
#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32          // longest protocol name length
#define INTERNET_MAX_URL_LENGTH         (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof("://") \
                                        + INTERNET_MAX_PATH_LENGTH)

// ----------------------------------------------------------------------------

//=============================================================================

#endif //_NP3_TYPEDEFS_H_
