#pragma once
/******************************************************************************
* encoding: UTF-8
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* TypeDefs.h                                                                  *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2023   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601  /*_WIN32_WINNT_WIN7*/
#endif
#ifndef WINVER
#define WINVER 0x0601  /*_WIN32_WINNT_WIN7*/
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif

#if 0
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 /*_WIN32_WINNT_WIN7*/
#undef WINVER
#define WINVER 0x0A00 /*_WIN32_WINNT_WIN7*/
#undef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000000 /*NTDDI_WIN7*/
#endif


#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#endif

// Want to use std::min and std::max so don't want Windows.h version of min and max
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <CommCtrl.h>

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE      // don't allow deprecated functions
#include <strsafe.h>
#include <intsafe.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "Sci_Position.h"
#include "Scintilla.h"

// - explicitly use MAX_PATH vs. PATHLONG_MAX_CCH
#define MAX_PATH_EXPLICIT MAX_PATH

// no Analyze warning "prefer: enum class"
#pragma warning(disable : 26812)

/**************************************************/
/*             Declared in WINNT.H                */
/*                                                */
/*  Provides bottom line type safety in function  */
/*  calls instead of using void* pointer          */
/**************************************************/
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) \
    struct name##__ {        \
        ptrdiff_t unused;    \
    };                       \
    typedef struct name##__* name
#endif

DECLARE_HANDLE(HSTRINGW);
DECLARE_HANDLE(HPATHL);

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
#define DOCMODDIFYD L"* "
#define URLPLACEHLDR L"${URL}"

// --------------------------------------------------------------------------

//typedef intptr_t cpi_enc_t;
typedef int cpi_enc_t;

typedef struct WININFO {

    int x;
    int y;
    int cx;
    int cy;
    bool max;
    int zoom;
    UINT dpi;

} WININFO;

#define INIT_WININFO { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, false, 100, USER_DEFAULT_SCREEN_DPI }

typedef enum SCREEN_MODE { SCR_NORMAL = 0, SCR_FULL_SCREEN = 1 } SCREEN_MODE;

__forceinline void RectFromWinInfo(const WININFO* const pWinInfo, LPRECT pRect)
{
    SetRect(pRect, pWinInfo->x, pWinInfo->y, pWinInfo->x + pWinInfo->cx, pWinInfo->y + pWinInfo->cy);
}
__forceinline void WinInfoFromRect(LPCRECT pRect, WININFO* pWinInfo)
{
    pWinInfo->x = pRect->left;
    pWinInfo->y = pRect->top;
    pWinInfo->cx = pRect->right - pRect->left;
    pWinInfo->cy = pRect->bottom - pRect->top;
    pWinInfo->max = false;
    pWinInfo->zoom = 100;
    pWinInfo->dpi = USER_DEFAULT_SCREEN_DPI;
}

extern WININFO g_IniWinInfo;
extern WININFO g_DefWinInfo;

// ----------------------------------------------------------------------------
// see windef.h  and wingdi.h
//::typedef DWORD COLORREF;
typedef COLORREF COLORALPHAREF;
// typedef unsigned __int32 COLORALPHAREF; //: warning(C4057) different base types
//::#define RGB(r, g, b) ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define ARGB(a, r, g, b) ((COLORALPHAREF)(((BYTE)(((r)&0xff)) | ((WORD)((BYTE)((g)&0xff)) << 8)) | (((DWORD)(BYTE)((b)&0xff)) << 16) | (((DWORD)(BYTE)((a)&0xff)) << 24)))
#define AxRGB(a, rgb) ((COLORALPHAREF)(((COLORREF)((rgb)&0xffffff)) | (((COLORALPHAREF)(BYTE)((a)&0xff)) << 24)))
#define RGB2RGBAREF(rgb) AxRGB(SC_ALPHA_OPAQUE, (COLORREF)((rgb)&0xffffff))
//::#define GetRValue(rgba) (LOBYTE(rgba))
//::#define GetGValue(rgba) (LOBYTE(((WORD)(rgba)) >> 8))
//::#define GetBValue(rgba) (LOBYTE((rgba) >> 16))
#define GetAValue(rgba) (LOBYTE((rgba) >> 24))

#define ARGB_TO_COLREF(X) ((X) & 0xffffff)
#define RGBA_TO_COLREF(X) RGB(((X) >> 24)&0xff, ((X) >> 16)&0xff, ((X) >> 8)&0xff)
#define BGRA_TO_COLREF(X) RGB(((X) >> 8)&0xff, ((X) >> 16)&0xff, ((X) >> 24)&0xff)

//#define ARGB_TO_COLORALPHAREF(X) (X)
#define RGBA_TO_COLORALPHAREF(X) AxRGB((X)&0xff, RGBA_TO_COLREF(X))
#define BGRA_TO_COLORALPHAREF(X) AxRGB((X)&0xff, BGRA_TO_COLREF(X))

#define ARGB_GET_ALPHA(A) (((A) >> 24)&0xff)
#define RGBA_GET_ALPHA(A) ((A)&0xff)
#define BGRA_GET_ALPHA(A) RGBA_GET_ALPHA(A)

// ----------------------------------------------------------------------------

typedef enum COLOR_LAYER { BACKGROUND_LAYER = 0, FOREGROUND_LAYER = 1 } COLOR_LAYER;  // Style_GetColor()
typedef enum HYPERLINK_OPS { OPEN_WITH_BROWSER = 1, OPEN_IN_NOTEPAD3 = (1<<1), OPEN_NEW_NOTEPAD3 = (1<<2), COPY_HYPERLINK = (1<<3), SELECT_HYPERLINK = (1<<4) } HYPERLINK_OPS;  // Hyperlink Operations
typedef enum FILE_WATCHING_MODE { FWM_NO_INIT = -1, FWM_DONT_CARE = 0, FWM_INDICATORSILENT = 1, FWM_MSGBOX = 2, FWM_AUTORELOAD = 3, FWM_EXCLUSIVELOCK = 4 } FILE_WATCHING_MODE;
typedef enum FOCUSVIEW_MARKER_MODE { FVMM_MARGIN = 1, FVMM_LN_BACKGR = 2, FVMM_FOLD = 4 } FOCUSVIEW_MARKER_MODE;
typedef enum DEFAULT_FONT_STYLES { DFS_GLOBAL = 0,
    DFS_CURR_LEXER = 1,
    DFS_GENERIC_USE } DEFAULT_FONT_STYLES;

// ----------------------------------------------------------------------------

// keep backward compatible with older settings-file versions
typedef enum CFG_VERSION {

    CFG_VER_NONE = 0,  /// old version
    CFG_VER_0001 = 1,  /// ZoomLevel and PrintZoom changed from relative font size in point to absolute percentage.
    CFG_VER_0002 = 2,  /// LongLine Marker Off by default
    CFG_VER_0003 = 3,  /// SimpleIni UTF-8 BOM
    CFG_VER_0004 = 4,  /// Text Files lexer vs. Default Text => (2nd) Common Style
    CFG_VER_0005 = 5,  /// FileWatchingMode numbering changed

    CFG_VER_CURRENT = CFG_VER_0005

} CFG_VERSION;

// --------------------------------------------------------------------------

typedef enum BUFFER_SIZES {

    MICRO_BUFFER = 32,
    MINI_BUFFER = 64,
    SMALL_BUFFER = 128,
    MIDSZ_BUFFER = 256,
    LARGE_BUFFER = 512,
    HUGE_BUFFER = 1024,
    XHUGE_BUFFER = 2048,
    XXXL_BUFFER = 4096,

    ANSI_CHAR_BUFFER = 258,
    STYLE_EXTENTIONS_BUFFER = 512,
    EXTENTIONS_FILTER_BUFFER = (STYLE_EXTENTIONS_BUFFER << 1),
    FNDRPL_BUFFER = 4096, // TODO: eliminate limit
    LONG_LINES_MARKER_LIMIT = 8192,
    CMDLN_LENGTH_LIMIT = 8192

} BUFFER_SIZES;


typedef enum FR_STATES { FND_NOP = 0, NXT_NOT_FND, NXT_FND, NXT_WRP_FND, PRV_NOT_FND, PRV_FND, PRV_WRP_FND } FR_STATES;
typedef enum FR_UPD_MODES { FRMOD_IGNORE = 0, FRMOD_NORM, FRMOD_WRAPED } FR_UPD_MODES;

//==== Statusbar ==============================================================

typedef WCHAR prefix_t[MICRO_BUFFER];

typedef enum STATUS_SECTOR_T {

    STATUS_DOCLINE = 0, STATUS_DOCCOLUMN, STATUS_SELECTION, STATUS_SELCTBYTES, STATUS_SELCTLINES,
    STATUS_OCCURRENCE, STATUS_DOCSIZE, STATUS_CODEPAGE, STATUS_EOLMODE, STATUS_OVRMODE, STATUS_2ND_DEF,
    STATUS_LEXER, STATUS_DOCCHAR, STATUS_OCCREPLACE, STATUS_TINYEXPR, STATUS_UNICODEPT,
    STATUS_SECTOR_COUNT,
    STATUS_HELP = SB_SIMPLEID // (!)
} STATUS_SECTOR_T;

#define SBS_INIT_ZERO  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define SBS_INIT_MINUS { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }

#define STATUSBAR_DEFAULT_IDS  L"0 1 15 14 2 4 5 6 7 8 9 10 11"
#define STATUSBAR_SECTION_WIDTH_SPECS L"30 20 20 20 20 20 20 0 0 0 0 0 0 0 20 24"
#define STAUSBAR_RIGHT_MARGIN 20

#define MB_ICONSHIELD 0x000000B0L

#define COLORREF_MAX (DWORD_MAX)

#define GLOBAL_INITIAL_FONTSIZE 11.0f

// --------------------------------------------------------------------------

//        |- len -|
// 0000000111111111000000000000
//                 |-- pos ---|
#define BITMASK_GEN(typ, pos, len) (~(~((typ)0ull) << (len)) << (pos))
#define TEST_BIT(typ, pos, set) (BITMASK_GEN(typ, pos, 1) & (set))

// --------------------------------------------------------------------------

typedef struct FILEVARS {

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

typedef struct EDITFINDREPLACE {

    UINT fuFlags;
    bool bTransformBS;
    bool bFindClose;
    bool bReplaceClose;
    bool bNoFindWrap;
    bool bRegExprSearch;
    bool bWildcardSearch;
    bool bMarkOccurences;
    bool bHideNonMatchedLines;
    bool bStateChanged;
    HWND hwnd;
    HSTRINGW chFindPattern;
    HSTRINGW chReplaceTemplate;

} EDITFINDREPLACE, *LPEDITFINDREPLACE;

typedef const EDITFINDREPLACE* const CLPCEDITFINDREPLACE;

#define INIT_EFR_DATA  { 0, false, false, false, false, false, false, false, false, true, NULL, NULL, NULL }
// USE: void DuplicateEFR(LPEDITFINDREPLACE dst, CLPCEDITFINDREPLACE src);
//      void ReleaseEFR(LPEDITFINDREPLACE efr);

#define IDMSG_SWITCHTOFIND    300
#define IDMSG_SWITCHTOREPLACE 301

#define MRU_MAXITEMS    32
#define MRU_ITEMSFILE   32
#define MRU_ITEMSFNDRPL 16
#define MRU_NOCASE       1
#define MRU_UTF8         2
#define MRU_BMRK_SIZE  512

typedef struct MRULIST {

    LPCWSTR   szRegKey;
    int       iFlags;
    int       iSize;
    LPWSTR    pszItems[MRU_MAXITEMS];
    cpi_enc_t iEncoding[MRU_MAXITEMS];
    DocPos    iCaretPos[MRU_MAXITEMS];
    DocPos    iSelAnchPos[MRU_MAXITEMS];
    LPWSTR    pszBookMarks[MRU_MAXITEMS];

} MRULIST, *PMRULIST, *LPMRULIST;

// --------------------------------------------------------------------------

typedef struct CmdMessageQueue_t {

    HWND hwnd;
    UINT  cmd;
    WPARAM wparam;
    LPARAM lparam;
    int delay;
    struct CmdMessageQueue_t* next;
    struct CmdMessageQueue_t* prev;

} CmdMessageQueue_t;

#define MESSAGE_QUEUE_INIT(hwnd,cmd,wp,lp)  { (hwnd), (cmd), MAKEWPARAM((wp), 1), ((LPARAM)(DWORD)(lp)), -1, NULL, NULL }
#define MQ_WM_CMD_INIT(hw,wp,lp)            MESSAGE_QUEUE_INIT((hw), WM_COMMAND, (wp), (lp))

// --------------------------------------------------------------------------

#define rgbRedColorRef       (RGB(255, 170, 170))
#define rgbGreenColorRef     (RGB(170, 255, 170))
#define rgbBlueColorRef      (RGB(170, 200, 255))

// GetSysColor(...) not working for DarkMode 
//#define rgbDarkBtnFcColorRef (RGB(0x33, 0x33, 0x33))
#define rgbDarkBtnFcColorRef (RGB(0x41, 0x41, 0x41))
#define rgbDarkTxtColorRef   (RGB(0xDE, 0xDE, 0xDE))
#define rgbDarkBkgColorRef   (RGB(0x14, 0x14, 0x14))

// --------------------------------------------------------------------------

typedef enum MARKER_ID {

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
// ASSERT( MARKER_NP3_BOOKMARK < SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN )


// SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN 21
// SC_MARKNUM_HISTORY_SAVED 22
// SC_MARKNUM_HISTORY_MODIFIED 23
// SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED 24
// SC_MARKNUM_FOLDEREND 25
// SC_MARKNUM_FOLDEROPENMID 26
// SC_MARKNUM_FOLDERMIDTAIL 27
// SC_MARKNUM_FOLDERTAIL 28
// SC_MARKNUM_FOLDERSUB 29
// SC_MARKNUM_FOLDER 30
// SC_MARKNUM_FOLDEROPEN 31


#define BOOKMARK_BITMASK() BITMASK_GEN(int, MARKER_NP3_BOOKMARK, 1)
#define OCC_INVISIBLE_BITMASK() BITMASK_GEN(int, MARKER_NP3_OCCURRENCE, 1)
#define OCCURRENCE_MARKER_BITMASK() BITMASK_GEN(int, 0, MARKER_NP3_8 + 1)
#define ALL_MARKERS_BITMASK() BITMASK_GEN(int, 0, MARKER_NP3_BOOKMARK + 1)

#define CHANGE_HISTORY_MARKER_BITMASK() BITMASK_GEN(int, SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN, \
                                                    (SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED - SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN + 1))

extern LPCWSTR WordBookMarks[];

// --------------------------------------------------------------------------


typedef enum INDIC_ID {

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

#define ENCLDATA_SIZE 256
typedef struct ENCLOSESELDATA {

    WCHAR pwsz1[ENCLDATA_SIZE];
    WCHAR pwsz2[ENCLDATA_SIZE];

} ENCLOSESELDATA, *PENCLOSESELDATA;

// --------------------------------------------------------------------------

#define SC_NP3_CURSORARROW 2
#define SC_NP3_CURSORHAND  8

//=============================================================================

typedef enum AutoSaveBackupOptions {

    ASB_None       = 0,
    ASB_Periodic   = 1 << 0,
    ASB_Suspend    = 1 << 1,
    ASB_Shutdown   = 1 << 2,

    ASB_Backup     = 1 << 3,
    ASB_OnAutoSave = 1 << 4,
    ASB_SideBySide = 1 << 5,

    ASB_Default = ASB_None // ASB_Suspend | ASB_Shutdown | ASB_SideBySide,

} AutoSaveBackupOptions;

//=============================================================================

typedef struct CONSTANTS_T {

    int const          StdDefaultLexerID; // Pure Text Files
    const WCHAR* const FileBrowserMiniPath;
    const WCHAR* const FileSearchGrepWin;
    const WCHAR* const Settings_Section;
    const WCHAR* const Settings2_Section;
    const WCHAR* const Window_Section;
    const WCHAR* const Styles_Section;
    const WCHAR* const SectionSuppressedMessages;

} CONSTANTS_T, *PCONSTANTS_T;

extern CONSTANTS_T const Constants;

#define DEF_WIN_POSITION_STRG L"DefaultWindowPosition"
#define WINDOWPOS_STRGFORMAT  L"%i,%i,%i,%i,%i,%i"

// ------------------------------------

typedef struct GLOBALS_T {

    int       iCfgVersionRead;
    HINSTANCE hInstance;
    HINSTANCE hPrevInst;
    HINSTANCE hLngResContainer;
    bool      bCanSaveIniFile;
    HWND      hwndMain;
    HANDLE    hndlProcessHeap;
    HWND      hwndEdit;
    HANDLE    hwndToolbar;
    HANDLE    hwndRebar;
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
    LPMRULIST pFileMRU;
    LPMRULIST pMRUfind;
    LPMRULIST pMRUreplace;
    FILEVARS  fvCurFile;
    int       iWrapCol;
    unsigned  uAvailLngCount;
    bool      CmdLnFlag_PosParam;
    int       CmdLnFlag_AlwaysOnTop;
    int       CmdLnFlag_WindowPos;
    int       CmdLnFlag_ReuseWindow;
    int       CmdLnFlag_SingleFileInstance;
    int       CmdLnFlag_MultiFileArg;
    int       CmdLnFlag_ShellUseSystemMRU;
    int       CmdLnFlag_PrintFileAndLeave;

    bool      bMinimizedToTray;
    bool      bZeroBasedColumnIndex;
    bool      bZeroBasedCharacterCount;
    int       iReplacedOccurrences;
    DocPosU   iSelectionMarkNumber;
    DocPosU   iMarkOccurrencesCount;
    bool      bUseLimitedAutoCCharSet;
    bool      bIsCJKInputCodePage;
    bool      bIniFileFromScratch;
    bool      bFindReplCopySelOrClip;
    bool      bReplaceInitialized;
    bool      bDocHasInconsistentEOLs;

    unsigned  uCurrentThemeIndex;
    char *    pStdDarkModeIniStyles;

#ifdef D_NP3_WIN10_DARK_MODE
    HBRUSH hbrDarkModeBkgBrush;
    HBRUSH hbrDarkModeBtnFcBrush;
    HBRUSH hbrDarkModeBkgHotBrush;
    HBRUSH hbrDarkModeBkgSelBrush;
#endif

    FR_STATES FindReplaceMatchFoundState;

    WCHAR CurrentLngLocaleName[LOCALE_NAME_MAX_LENGTH + 1];

} GLOBALS_T, *PGLOBALS_T;

extern GLOBALS_T Globals;

// ------------------------------------

typedef struct PATHS_T {
    
    HPATHL CurrentFile;
    HPATHL ModuleDirectory;
    HPATHL WorkingDirectory;
    HPATHL IniFile;
    HPATHL IniFileDefault;

} PATHS_T, *PPATHS_T;

extern PATHS_T Paths;

// ------------------------------------

typedef struct SETTINGS_T {

    bool SaveSettings;
    bool SaveRecentFiles;
    bool PreserveCaretPos;
    bool SaveFindReplace;
    bool AutoLoadMRUFile;
    int  PathNameFormat;
    bool WordWrap;
    int  WordWrapMode;
    int  WordWrapIndent;
    int  WordWrapSymbols;
    bool ShowWordWrapSymbols;
    bool DocReadOnlyMode;
    bool MatchBraces;
    bool AutoCloseTags;
    bool AutoCloseQuotes;
    bool AutoCloseBrackets;
    int  HighlightCurrentLine;
    int  ChangeHistoryMode;
    bool HyperlinkHotspot;
    int  ColorDefHotspot;
    bool ScrollPastEOF;
    bool ShowHypLnkToolTip;
    bool HighlightUnicodePoints;
    bool AutoIndent;
    bool AutoCompleteWords;
    bool AutoCLexerKeyWords;
    bool AccelWordNavigation;
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
    bool PreferredLocale4DateFmt;
    int  AutoSaveInterval;
    bool ReplaceByClipboardTag;

    AutoSaveBackupOptions AutoSaveOptions;

#ifdef D_NP3_WIN10_DARK_MODE
    bool WinThemeDarkMode;
#endif

    RECT PrintMargin;
    EDITFINDREPLACE EFR_Data;
    HPATHL OpenWithDir;
    HPATHL FavoritesDir;
    WCHAR ToolbarButtons[MIDSZ_BUFFER];
    WCHAR MultiEdgeLines[MIDSZ_BUFFER];
    WCHAR CurrentThemeName[SMALL_BUFFER];

} SETTINGS_T, *PSETTINGS_T;

extern SETTINGS_T Defaults;
extern SETTINGS_T Settings;

#define IsMarkOccurrencesEnabled() (Settings.MarkOccurrences)
#define IsFocusedViewAllowed() (IsMarkOccurrencesEnabled() && !Settings.MarkOccurrencesMatchVisible)
#define IsColorDefHotspotEnabled() (Settings.ColorDefHotspot != 0)

//=============================================================================

typedef struct FLAGS_T {

    int  ToolbarLook;
    int  PrintFileAndLeave;

    bool bHugeFileLoadState;
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

typedef struct SETTINGS2_T {

    int    FileLoadWarningMB;
    int    OpacityLevel;
    int    FindReplaceOpacityLevel;
    DWORD  FileCheckInterval;
    DWORD  UndoTransactionTimeout;
    int    IMEInteraction;
    int    SciFontQuality;
    int    LaunchInstanceWndPosOffset;
    bool   LaunchInstanceFullVisible;
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
    int    WrapAroundTooltipTimeout;
    int    LargeIconScalePrecent;

    float  AnalyzeReliableConfidenceLevel;
    float  LocaleAnsiCodePageAnalysisBonus;
    float  DarkModeHiglightContrast;

#ifdef D_NP3_WIN10_DARK_MODE
    COLORREF DarkModeBkgColor;
    COLORREF DarkModeBtnFaceColor;
    COLORREF DarkModeTxtColor;
#endif

    HPATHL DefaultDirectory;
    HPATHL FileBrowserPath;
    HPATHL GrepWinPath;
    HPATHL AdministrationTool;

    HSTRINGW WebTemplate1;
    HSTRINGW WebTemplate2;
    HSTRINGW HyperlinkShellExURLWithApp;
    HSTRINGW HyperlinkShellExURLCmdLnArgs;
    HSTRINGW FileDlgFilters;

    WCHAR FileChangedIndicator[4];
    WCHAR FileDeletedIndicator[4];

    WCHAR DefaultExtension[MINI_BUFFER];

    WCHAR AppUserModelID[SMALL_BUFFER];
    WCHAR AutoCompleteFillUpChars[MINI_BUFFER];
    WCHAR LineCommentPostfixStrg[MINI_BUFFER];
    WCHAR ExtendedWhiteSpaceChars[ANSI_CHAR_BUFFER + 1];
    WCHAR AutoCompleteWordCharSet[ANSI_CHAR_BUFFER + 1];

    WCHAR DateTimeFormat[SMALL_BUFFER];
    WCHAR DateTimeLongFormat[SMALL_BUFFER];
    WCHAR TimeStampRegEx[SMALL_BUFFER];
    WCHAR TimeStampFormat[SMALL_BUFFER];

    WCHAR WebTmpl1MenuName[MICRO_BUFFER];
    WCHAR WebTmpl2MenuName[MICRO_BUFFER];
    WCHAR DefaultWindowPosition[MINI_BUFFER];

    WCHAR PreferredLanguageLocaleName[LOCALE_NAME_MAX_LENGTH + 1];

    WCHAR HyperlinkFileProtocolVerb[MICRO_BUFFER];

    const WCHAR* CodeFontPrefPrioList[MICRO_BUFFER];
    const WCHAR* TextFontPrefPrioList[MICRO_BUFFER];

} SETTINGS2_T, *PSETTINGS2_T;

#define Default_ExitOnESCSkipLevel 2

extern SETTINGS2_T Settings2;
extern WCHAR Default_PreferredLanguageLocaleName[];


//=============================================================================

typedef enum SpecialUndoRedoToken {
    // undoredo token >= 0
    URTok_TokenStart    =  0L,
    URTok_NoTransaction = -1L,
    URTok_NoRecording   = -2L

} SpecialUndoRedoToken;

//=============================================================================

typedef struct FOCUSEDVIEW_T {

    bool HideNonMatchedLines;
    bool CodeFoldingAvailable;
    bool ShowCodeFolding;       // <-> Settings.ShowCodeFolding

} FOCUSEDVIEW_T, *PFOCUSEDVIEW_T;

extern FOCUSEDVIEW_T FocusedView;

//=============================================================================

typedef struct FILEWATCHING_T {

    FILE_WATCHING_MODE flagChangeNotify;  // <-> s_flagChangeNotify;
    FILE_WATCHING_MODE FileWatchingMode;  // <-> Settings.FileWatchingMode;
    DWORD              FileCheckInterval; // <-> Settings2.FileCheckInterval;
    bool               MonitoringLog;

} FILEWATCHING_T, *PFILEWATCHING_T;

extern FILEWATCHING_T FileWatching;

//=============================================================================

typedef enum FileLoadFlags {

    FLF_None                = 0,
    FLF_DontSave            = 1 << 0,
    FLF_New                 = 1 << 1,
    FLF_Reload              = 1 << 2,
    FLF_SkipUnicodeDetect   = 1 << 3,
    FLF_SkipANSICPDetection = 1 << 4,
    FLF_ForceEncDetection   = 1 << 5

} FileLoadFlags;

typedef enum FileSaveFlags {

    FSF_None              = 0,
    FSF_SaveAlways        = 1 << 0,
    FSF_Ask               = 1 << 1,
    FSF_SaveAs            = 1 << 2,
    FSF_SaveCopy          = 1 << 3,
    FSF_AutoSave          = 1 << 4,
    FSF_EndSession        = 1 << 5

} FileSaveFlags;

//=============================================================================


typedef enum INDENT_TYPE { I_TAB_LN = 0, I_SPC_LN = 1, I_MIX_LN = 2, I_TAB_MOD_X = 3, I_SPC_MOD_X = 4 } INDENT_TYPE;

typedef struct EditFileIOStatus {

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

typedef struct THEMEFILES {

    UINT    rid;
    WCHAR   szName[MINI_BUFFER];
    HPATHL  hStyleFilePath;

} THEMEFILES, * PTHEMEFILES;

//=============================================================================

// ---------   common defines   --------

#define IS_VALID_HANDLE(HNDL) ((HNDL) && ((HNDL) != INVALID_HANDLE_VALUE))

#define NOTEPAD3_MODULE_DIR_ENV_VAR  L"NOTEPAD3MODULEDIR"

//~#define NP3_VIRTUAL_SPACE_ACCESS_OPTIONS (Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : (SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART | SCVS_USERACCESSIBLE))
//~#define NP3_VIRTUAL_SPACE_ACCESS_OPTIONS (Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : (SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART))
// don't use 'SCVS_NOWRAPLINESTART'
#define NP3_VIRTUAL_SPACE_ACCESS_OPTIONS (Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION)


// The possible notification types are the same as the modificationType bit flags used by SCN_MODIFIED:
// SC_MOD_INSERTTEXT, SC_MOD_DELETETEXT, SC_MOD_CHANGESTYLE, SC_MOD_CHANGEFOLD, SC_PERFORMED_USER,
// SC_PERFORMED_UNDO, SC_PERFORMED_REDO, SC_MULTISTEPUNDOREDO, SC_LASTSTEPINUNDOREDO, SC_MOD_CHANGEMARKER,
// SC_MOD_BEFOREINSERT, SC_MOD_BEFOREDELETE, SC_MULTILINEUNDOREDO, and SC_MODEVENTMASKALL.
// SC_MOD_INSERTCHECK
//
///~ int const evtMask = SC_MODEVENTMASKALL; (!) - don't listen to all events (SC_MOD_CHANGESTYLE) => RECURSION!
///~ SciCall_SetModEventMask(evtMask);
///~ Don't use: SC_PERFORMED_USER | SC_MOD_CHANGESTYLE;
/// SC_MOD_CHANGESTYLE and SC_MOD_CHANGEINDICATOR needs SCI_SETCOMMANDEVENTS=true
//
typedef enum SciEventMask {

    EVM_None     = SC_MOD_NONE,
    EVM_UndoRedo = SC_MOD_CONTAINER | SC_PERFORMED_UNDO | SC_PERFORMED_REDO | SC_MULTILINEUNDOREDO | SC_MULTISTEPUNDOREDO | SC_LASTSTEPINUNDOREDO |
                   SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE | SC_MOD_INSERTCHECK,
    EVM_Default  = EVM_UndoRedo
    //~EVM_All = SC_MODEVENTMASKALL ~ don't use

} SciEventMask;




// from <wininet.h>
#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32          // longest protocol name length
#define INTERNET_MAX_URL_LENGTH         (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof("://") \
                                        + INTERNET_MAX_PATH_LENGTH)

// ----------------------------------------------------------------------------

#define SET_FCT_GUARD(RET) {         \
    static bool _fctguard = false;   \
    if (_fctguard) { return (RET); } \
    { _fctguard = true;

#define RESET_FCT_GUARD()  } _fctguard = false; }

// ----------------------------------------------------------------------------

inline void SetDialogIconNP3(HWND hwnd)
{
    if (Globals.hDlgIconSmall) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIconSmall);
    }
    if (Globals.hDlgIconBig) {
        SendMessage((hwnd), WM_SETICON, ICON_BIG, (LPARAM)Globals.hDlgIconBig);
    }
}

// ----------------------------------------------------------------------------

//=============================================================================
