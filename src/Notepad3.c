/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Notepad3.c                                                                  *
*   Main application window functionality                                     *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2017   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

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
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
//#include <pathcch.h>
#include <time.h>
#include <muiload.h>

#include "edit.h"
#include "styles.h"
#include "dialogs.h"
#include "resource.h"
#include "../crypto/crypto.h"
#include "../uthash/utarray.h"
#include "../uthash/utlist.h"
#include "../tinyexpr/tinyexpr.h"
#include "encoding.h"
#include "helpers.h"
#include "VersionEx.h"
#include "SciCall.h"
#include "scilexer.h"

#include "notepad3.h"


/******************************************************************************
*
* Local and global Variables for Notepad3.c
*
*/
HWND      g_hwndMain = NULL;
HWND      g_hwndStatus = NULL;
HWND      g_hwndToolbar = NULL;
HWND      g_hwndDlgFindReplace = NULL;
HWND      g_hwndDlgCustomizeSchemes = NULL;
HWND      g_hwndReBar = NULL;
HWND      hwndEditFrame = NULL;
HWND      hwndNextCBChain = NULL;
HICON     g_hDlgIcon = NULL;
UINT		  g_uCurrentDPI = USER_DEFAULT_SCREEN_DPI;
UINT		  g_uCurrentPPI = USER_DEFAULT_SCREEN_DPI;

bool g_bExternalBitmap = false;

#define INISECTIONBUFCNT 32

TBBUTTON  tbbMainWnd[] = {  { 0,IDT_FILE_NEW,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 1,IDT_FILE_OPEN,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 3,IDT_FILE_SAVE,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 2,IDT_FILE_BROWSE,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 4,IDT_EDIT_UNDO,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 5,IDT_EDIT_REDO,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 6,IDT_EDIT_CUT,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 7,IDT_EDIT_COPY,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 8,IDT_EDIT_PASTE,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 9,IDT_EDIT_FIND,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 10,IDT_EDIT_REPLACE,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 11,IDT_VIEW_WORDWRAP,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 23,IDT_VIEW_TOGGLEFOLDS,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 25,IDT_VIEW_TOGGLE_VIEW,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 21,IDT_FILE_OPENFAV,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 22,IDT_FILE_ADDTOFAV,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 12,IDT_VIEW_ZOOMIN,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 13,IDT_VIEW_ZOOMOUT,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 26,IDT_VIEW_CHASING_DOCTAIL,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 14,IDT_VIEW_SCHEME,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 24,IDT_FILE_LAUNCH,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 16,IDT_FILE_EXIT,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 15,IDT_VIEW_SCHEMECONFIG,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 0,0,0,BTNS_SEP,0,0 },
                            { 17,IDT_FILE_SAVEAS,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 18,IDT_FILE_SAVECOPY,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 19,IDT_EDIT_CLEAR,TBSTATE_ENABLED,BTNS_BUTTON,0,0 },
                            { 20,IDT_FILE_PRINT,TBSTATE_ENABLED,BTNS_BUTTON,0,0 }
};

#define NUMTOOLBITMAPS  27
#define NUMINITIALTOOLS 33
#define TBBUTTON_DEFAULT_IDS  L"1 2 4 3 0 5 6 0 7 8 9 0 10 11 0 12 0 24 26 0 22 23 0 13 14 0 27 0 15 0 25 0 17"


WCHAR         g_wchIniFile[MAX_PATH] = { L'\0' };
WCHAR         g_wchIniFile2[MAX_PATH] = { L'\0' };
static WCHAR  g_szTmpFilePath[MAX_PATH] = { L'\0' };

static int    g_iSettingsVersion = CFG_VER_NONE;
static bool   g_bSaveSettings;
static bool   g_bEnableSaveSettings;
bool          g_bIniFileFromScratch = false;
bool          g_bSaveRecentFiles;
bool          g_bPreserveCaretPos;
bool          g_bSaveFindReplace;
bool          g_bFindReplCopySelOrClip = true;

WCHAR         g_tchPrefLngLocName[LOCALE_NAME_MAX_LENGTH];
LANGID        g_iPrefLngLocID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
HMODULE       g_hLngResContainer = NULL;
#define       LNG_AVAILABLE_COUNT 8
static WCHAR* const  g_tchAvailableLanguages = L"af-ZA de-DE es-ES en-GB fr-FR ja-JP nl-NL zh-CN"; // en-US internal
static LANGID const  g_iAvailableLanguages[LNG_AVAILABLE_COUNT] = { 1078, 1031, 3082, 2057, 1036, 1041, 1043, 2052 }; // 1033 internal

WCHAR         g_tchFileDlgFilters[XXXL_BUFFER] = { L'\0' };

WCHAR         g_tchLastSaveCopyDir[MAX_PATH] = { L'\0' };
WCHAR         g_tchOpenWithDir[MAX_PATH] = { L'\0' };
WCHAR         g_tchFavoritesDir[MAX_PATH] = { L'\0' };
WCHAR         g_tchAdministrationExe[MAX_PATH] = { L'\0' };

static WCHAR  g_tchDefaultExtension[64] = { L'\0' };
static WCHAR  g_tchDefaultDir[MAX_PATH] = { L'\0' };
static WCHAR  g_tchToolbarButtons[MIDSZ_BUFFER] = { L'\0' };


static prefix_t g_mxSBPrefix[STATUS_SECTOR_COUNT];
static prefix_t g_mxSBPostfix[STATUS_SECTOR_COUNT];

static int    g_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;
static bool   g_iStatusbarVisible[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
static int    g_vSBSOrder[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

static int    g_iStatusbarWidthSpec[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;

static WCHAR  g_tchToolbarBitmap[MAX_PATH] = { L'\0' };
static WCHAR  g_tchToolbarBitmapHot[MAX_PATH] = { L'\0' };
static WCHAR  g_tchToolbarBitmapDisabled[MAX_PATH] = { L'\0' };

static  WININFO g_WinInfo = INIT_WININFO;
static  int     g_WinCurrentWidth = 0;

int       iPathNameFormat;
bool      g_bWordWrap;
bool      bWordWrapG;
int       iWordWrapMode;
int       iWordWrapIndent;
int       iWordWrapSymbols;
bool      bShowWordWrapSymbols;
bool      bMatchBraces;
bool      bAutoIndent;
bool      bAutoCloseTags;
bool      bShowIndentGuides;
bool      bHiliteCurrentLine;
bool      g_bHyperlinkHotspot;
bool      bScrollPastEOF;
bool      g_bTabsAsSpaces;
bool      bTabsAsSpacesG;
bool      g_bTabIndents;
bool      bTabIndentsG;
bool      bBackspaceUnindents;
int       g_iTabWidth;
int       iTabWidthG;
int       g_iIndentWidth;
int       iIndentWidthG;
bool      g_bMarkLongLines;
int       g_iLongLinesLimit;
int       iLongLinesLimitG;
int       iLongLineMode;
int       iWrapCol = 0;
bool      g_bShowSelectionMargin;
bool      g_bShowLineNumbers;
bool      g_bZeroBasedColumnIndex;
bool      g_bZeroBasedCharacterCount;
int       g_iReplacedOccurrences;
int       g_iMarkOccurrences;
int       g_iMarkOccurrencesCount;
int       g_iMarkOccurrencesMaxCount;
bool      g_bMarkOccurrencesMatchVisible;
bool      g_bMarkOccurrencesMatchCase;
bool      g_bMarkOccurrencesMatchWords;
bool      g_bMarkOccurrencesCurrentWord;
bool      g_bUseOldStyleBraceMatching;
bool      g_bAutoCompleteWords;
bool      g_bAccelWordNavigation;
bool      g_bDenyVirtualSpaceAccess;
bool      g_bCodeFoldingAvailable;
bool      g_bShowCodeFolding;
bool      bViewWhiteSpace;
bool      bViewEOLs;
bool      bUseDefaultForFileEncoding;
bool      bSkipUnicodeDetection;
bool      bSkipANSICodePageDetection;
bool      bLoadASCIIasUTF8 = false;
bool      bForceLoadASCIIasUTF8 = false;
bool      bLoadNFOasOEM;
bool      bNoEncodingTags;
bool      bFixLineEndings;
bool      bAutoStripBlanks;
int       iPrintHeader;
int       iPrintFooter;
int       iPrintColor;
int       iPrintZoom;
RECT      pagesetupMargin;
bool      bSaveBeforeRunningTools;
int       g_iFileWatchingMode;
bool      g_bResetFileWatching;
DWORD     dwFileCheckInverval;
DWORD     dwAutoReloadTimeout;
int       iEscFunction;
bool      bAlwaysOnTop;
bool      bMinimizeToTray;
bool      bTransparentMode;
bool      bShowToolbar;
bool      bShowStatusbar;
int       iHighDpiToolBar;
int       iUpdateDelayHyperlinkStyling;
int       iUpdateDelayMarkAllCoccurrences;
int       iCurrentLineHorizontalSlop = 0;
int       iCurrentLineVerticalSlop = 0;
bool      g_bChasingDocTail = false;

CALLTIPTYPE g_CallTipType = CT_NONE;


int  g_iRenderingTechnology = 0;
const int DirectWriteTechnology[4] = {
    SC_TECHNOLOGY_DEFAULT
  , SC_TECHNOLOGY_DIRECTWRITE
  , SC_TECHNOLOGY_DIRECTWRITERETAIN
  , SC_TECHNOLOGY_DIRECTWRITEDC
};


static int  g_iBidirectional = 0;
const int SciBidirectional[3] = {
  SC_BIDIRECTIONAL_DISABLED
  , SC_BIDIRECTIONAL_L2R
  , SC_BIDIRECTIONAL_R2L
};


int g_iSciFontQuality = 0;
const int FontQuality[4] = {
    SC_EFF_QUALITY_DEFAULT
  , SC_EFF_QUALITY_NON_ANTIALIASED
  , SC_EFF_QUALITY_ANTIALIASED
  , SC_EFF_QUALITY_LCD_OPTIMIZED
};

bool    g_bStickyWinPos;

bool    bIsAppThemed;
int     cyReBar;
int     cyReBarFrame;
int     cxEditFrame;
int     cyEditFrame;

int     cxEncodingDlg;
int     cyEncodingDlg;
int     cxRecodeDlg;
int     cyRecodeDlg;
int     cxFileMRUDlg;
int     cyFileMRUDlg;
int     cxOpenWithDlg;
int     cyOpenWithDlg;
int     cxFavoritesDlg;
int     cyFavoritesDlg;
int     xFindReplaceDlg;
int     yFindReplaceDlg;
int     xCustomSchemesDlg;
int     yCustomSchemesDlg;

#define FILE_LIST_SIZE 32
LPWSTR    lpFileList[FILE_LIST_SIZE] = { NULL };
int       cFileList = 0;
int       cchiFileList = 0;
LPWSTR    lpFileArg = NULL;
LPWSTR    lpSchemeArg = NULL;
LPWSTR    lpMatchArg = NULL;
LPWSTR    lpEncodingArg = NULL;
LPMRULIST g_pFileMRU;
LPMRULIST g_pMRUfind;
LPMRULIST g_pMRUreplace;

DWORD     dwLastIOError;

int       g_iDefaultNewFileEncoding;
int       g_iDefaultCharSet;
int       g_IMEInteraction;

int       g_iEOLMode;
int       g_iDefaultEOLMode;

int       iInitialLine;
int       iInitialColumn;

int       iInitialLexer;

bool      bLastCopyFromMe = false;
DWORD     dwLastCopyTime;

UINT      uidsAppTitle = IDS_MUI_APPTITLE;
WCHAR     szTitleExcerpt[MIDSZ_BUFFER] = { L'\0' };
int       fKeepTitleExcerpt = 0;

HANDLE    hChangeHandle = NULL;
bool      dwChangeNotifyTime = 0;
WIN32_FIND_DATA fdCurFile;

UINT      msgTaskbarCreated = 0;

HMODULE   hModUxTheme = NULL;
HMODULE   hRichEdit = NULL;

static bool g_bRunningWatch = false;

static EDITFINDREPLACE g_efrData = EFR_INIT_DATA;
bool bReplaceInitialized = false;

int iLineEndings[3] = {
  SC_EOL_CRLF,
  SC_EOL_LF,
  SC_EOL_CR
};

WCHAR wchPrefixSelection[256] = { L'\0' };
WCHAR wchAppendSelection[256] = { L'\0' };

WCHAR wchPrefixLines[256] = { L'\0' };
WCHAR wchAppendLines[256] = { L'\0' };

static int iSortOptions = 0;
static int iAlignMode   = 0;

bool      flagIsElevated = false;
WCHAR     wchWndClass[16] = L"" APPNAME;


HINSTANCE g_hInstance = NULL;
HANDLE    g_hScintilla = NULL;
HANDLE    g_hwndEdit = NULL;

WCHAR     g_wchAppUserModelID[32] = { L'\0' };
WCHAR     g_wchWorkingDirectory[MAX_PATH+2] = { L'\0' };
WCHAR     g_wchCurFile[FILE_ARG_BUF] = { L'\0' };
FILEVARS  fvCurFile;
bool      g_bFileReadOnly = false;

// for tiny expression calculation
static double g_dExpression = 0.0;
static int    g_iExprError = -1;

// temporary line buffer for fast line ops 
static char g_pTempLineBufferMain[TEMPLINE_BUFFER];

// declarations

// undo / redo  selections
static UT_icd UndoRedoSelection_icd = { sizeof(UndoRedoSelection_t), NULL, NULL, NULL };
static UT_array* UndoRedoSelectionUTArray = NULL;
static bool __fastcall _InUndoRedoTransaction();
static void __fastcall _SaveRedoSelection(int token);
static int __fastcall  _SaveUndoSelection();
static int __fastcall  _UndoRedoActionMap(int token, UndoRedoSelection_t* const selection);


#ifdef _EXTRA_DRAG_N_DROP_HANDLER_
static CLIPFORMAT cfDrpF = CF_HDROP;
static POINTL ptDummy = { 0, 0 };
static PDROPTARGET pDropTarget = NULL;
static DWORD DropFilesProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData);
#endif


//=============================================================================
//
//  IgnoreNotifyChangeEvent(), ObserveNotifyChangeEvent(), CheckNotifyChangeEvent()
//
static volatile LONG iNotifyChangeStackCounter = 0L;

bool CheckNotifyChangeEvent()
{
  return (InterlockedOr(&iNotifyChangeStackCounter, 0L) == 0L);
}

void IgnoreNotifyChangeEvent() 
{
  InterlockedIncrement(&iNotifyChangeStackCounter);
}

void ObserveNotifyChangeEvent() 
{
  if (!CheckNotifyChangeEvent()) {
    InterlockedDecrement(&iNotifyChangeStackCounter);
  }
  if (CheckNotifyChangeEvent()) {
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
  }
}

// SCN_UPDATEUI notification
#define SC_UPDATE_NP3_INTERNAL_NOTIFY (SC_UPDATE_H_SCROLL << 1)


//=============================================================================
//
//  Delay Message Queue Handling  (TODO: MultiThreading)
//

static CmdMessageQueue_t* MessageQueue = NULL;

// ----------------------------------------------------------------------------

static int msgcmp(void* mqc1, void* mqc2)
{
  CmdMessageQueue_t* const pMQC1 = (CmdMessageQueue_t*)mqc1;
  CmdMessageQueue_t* const pMQC2 = (CmdMessageQueue_t*)mqc2;

  if ((pMQC1->hwnd == pMQC2->hwnd)
       && (pMQC1->cmd == pMQC2->cmd)
       && (pMQC1->wparam == pMQC2->wparam)
       && (pMQC1->lparam == pMQC2->lparam))
  {
    return 0;
  }
  return 1;
}
// ----------------------------------------------------------------------------

#define _MQ_ms(T) ((T) / USER_TIMER_MINIMUM)

static void __fastcall _MQ_AppendCmd(CmdMessageQueue_t* const pMsgQCmd, int cycles)
{
  CmdMessageQueue_t* pmqc = NULL;
  DL_SEARCH(MessageQueue, pmqc, pMsgQCmd, msgcmp);

  if (!pmqc) { // NOT found
    pmqc = AllocMem(sizeof(CmdMessageQueue_t), HEAP_ZERO_MEMORY);
    pmqc->hwnd = pMsgQCmd->hwnd;
    pmqc->cmd = pMsgQCmd->cmd;
    pmqc->wparam = pMsgQCmd->wparam;
    pmqc->lparam = pMsgQCmd->lparam;
    pmqc->delay = cycles;
    DL_APPEND(MessageQueue, pmqc);
  }

  if (cycles < 2) {
    pmqc->delay = -1; // execute now (do not use PostMessage() here)
    SendMessage(pMsgQCmd->hwnd, pMsgQCmd->cmd, pMsgQCmd->wparam, pMsgQCmd->lparam);
  }
  else {
    pmqc->delay = (pmqc->delay + cycles) / 2; // increase delay
  }
}
// ----------------------------------------------------------------------------

static void __fastcall _MQ_RemoveCmd(CmdMessageQueue_t* const pMsgQCmd)
{
  CmdMessageQueue_t* pmqc;

  DL_FOREACH(MessageQueue, pmqc)
  {
    if ((pMsgQCmd->hwnd == pmqc->hwnd)
      && (pMsgQCmd->cmd == pmqc->cmd)
      && (pMsgQCmd->wparam == pmqc->wparam))
    {
      pmqc->delay = -1;
    }
  }
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//
// called by Timer(IDT_TIMER_MRKALL)
//
static void CALLBACK MQ_ExecuteNext(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  UNUSED(hwnd);    // must be main wnd
  UNUSED(uMsg);    // must be WM_TIMER
  UNUSED(idEvent); // must be IDT_TIMER_MRKALL
  UNUSED(dwTime);  // This is the value returned by the GetTickCount function

  CmdMessageQueue_t* pmqc;

  DL_FOREACH(MessageQueue, pmqc) 
  {
    if (pmqc->delay == 0) {
      pmqc->delay = -1;
      SendMessage(pmqc->hwnd, pmqc->cmd, pmqc->wparam, pmqc->lparam);
    }
    else if (pmqc->delay >= 0) {
      pmqc->delay -= 1;
    }
  }
}


//=============================================================================
//
// Flags
//
int g_flagNoFadeHidden              = 0;
int g_flagSimpleIndentGuides        = 0;
int g_flagNoHTMLGuess               = 0;
int g_flagNoCGIGuess                = 0;
int g_flagNoFileVariables           = 0;
int g_flagUseSystemMRU              = 0;
int g_flagPrintFileAndLeave         = 0;

static int g_flagNoReuseWindow      = 0;
static int g_flagReuseWindow        = 0;
static int g_flagMultiFileArg       = 0;
static int g_flagSingleFileInstance = 0;
static int g_flagStartAsTrayIcon    = 0;
static int g_flagAlwaysOnTop        = 0;
static int g_flagRelativeFileMRU    = 0;
static int g_flagPortableMyDocs     = 0;
static int g_flagToolbarLook        = 0;
static int g_flagPosParam           = 0;
static int g_flagDefaultPos         = 0;
static int g_flagNewFromClipboard   = 0;
static int g_flagPasteBoard         = 0;
static int g_flagSetEncoding        = 0;
static int g_flagSetEOLMode         = 0;
static int g_flagJumpTo             = 0;
static int g_flagMatchText          = 0;
static int g_flagChangeNotify       = 0;
static int g_flagLexerSpecified     = 0;
static int g_flagQuietCreate        = 0;
static int g_flagRelaunchElevated   = 0;
static int g_flagDisplayHelp        = 0;
static int g_flagBufferFile         = 0;

//==============================================================================

// decalarations 
static void __fastcall _UpdateStatusbarDelayed(bool bForceRedraw);
static void __fastcall _UpdateToolbarDelayed();
static HMODULE __fastcall _LoadLanguageResources(LANGID const langID, const WCHAR* locName);
static bool __fastcall _RegisterWndClass(HINSTANCE hInstance);

//==============================================================================
//
//  Document Modified Flag
//
//
static bool IsDocumentModified = false;

static void __fastcall _SetDocumentModified(bool bModified)
{
  if (IsDocumentModified != bModified) {
    IsDocumentModified = bModified;
    UpdateToolbar();
    UpdateStatusbar(false);
  }
  if (bModified) {
    if (IsWindow(g_hwndDlgFindReplace)) {
      SendMessage(g_hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDC_DOC_MODIFIED, 1), 0);
    }
  }
}

//==============================================================================


//=============================================================================
//
//  WinMain()
//
//
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInst, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  UNUSED(hPrevInst);

  HACCEL hAccMain;
  HACCEL hAccFindReplace;
  HACCEL hAccCoustomizeSchemes;

  INITCOMMONCONTROLSEX icex;
  //HMODULE hSciLexer;
  WCHAR wchAppDir[2*MAX_PATH+4] = { L'\0' };

  // Set global variable g_hInstance
  g_hInstance = hInstance;

  GetModuleFileName(NULL,wchAppDir,COUNTOF(wchAppDir));
  PathRemoveFileSpec(wchAppDir);
  PathCanonicalizeEx(wchAppDir,COUNTOF(wchAppDir));

  if (!GetCurrentDirectory(COUNTOF(g_wchWorkingDirectory),g_wchWorkingDirectory)) {
    StringCchCopy(g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory),wchAppDir);
  }

  // Don't keep working directory locked
  SetCurrentDirectory(wchAppDir);

  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

  // check if running at least on Windows 7
  if (!IsWin7()) {
    GetLastErrorToMsgBox(L"WinMain", ERROR_OLD_WIN_VERSION);
    return 1; // exit
  }

  // Check if running with elevated privileges
  flagIsElevated = IsUserAdmin() || IsElevated();

  // Default Encodings (may already be used for command line parsing)
  Encoding_InitDefaults();

  // Command Line, Ini File and Flags
  ParseCommandLine();
  FindIniFile();
  TestIniFile();
  CreateIniFile();
  LoadFlags();

  // set AppUserModelID
  PrivateSetCurrentProcessExplicitAppUserModelID(g_wchAppUserModelID);

  // Command Line Help Dialog
  if (g_flagDisplayHelp) {
    DisplayCmdLineHelp(NULL);
    return 0;
  }

  // Adapt window class name
  if (flagIsElevated)
    StringCchCat(wchWndClass,COUNTOF(wchWndClass),L"U");
  if (g_flagPasteBoard)
    StringCchCat(wchWndClass,COUNTOF(wchWndClass),L"B");

  // Relaunch with elevated privileges
  if (RelaunchElevated(NULL))
    return 0;

  // Try to run multiple instances
  if (RelaunchMultiInst())
    return 0;

  // Try to activate another window
  if (ActivatePrevInst())
    return 0;

  // Load Settings
  LoadSettings();

  // ----------------------------------------------------
  // MultiLingual
  //
  bool bPrefLngNotAvail = false;

  DWORD dwLocID = 0UL;
  if (StringCchLen(g_tchPrefLngLocName, COUNTOF(g_tchPrefLngLocName)) > 0) {
    GetLocaleInfoEx(g_tchPrefLngLocName, LOCALE_ILANGUAGE | LOCALE_RETURN_NUMBER, (LPWSTR)&dwLocID, sizeof(DWORD));
  }
  if (dwLocID == 0UL) {
    //GetLocaleInfoEx(LOCALE_USER_DEFAULT, LOCALE_SNAME, g_tchPrefLngLocName, COUNTOF(g_tchPrefLngLocName));
    GetUserDefaultLocaleName(&g_tchPrefLngLocName[0], COUNTOF(g_tchPrefLngLocName));
    g_iPrefLngLocID = GetUserDefaultUILanguage();
  }
  else {
    g_iPrefLngLocID = (LANGID)dwLocID;
  }

  g_hLngResContainer = _LoadLanguageResources(g_iPrefLngLocID, g_tchPrefLngLocName);

  if (!g_hLngResContainer) // fallback en-US (1033)
  {
    g_hLngResContainer = g_hInstance; 
    if (g_iPrefLngLocID != 1033) { bPrefLngNotAvail = true; }
  }
  // ----------------------------------------------------

  // Init OLE and Common Controls
  (void)OleInitialize(NULL);
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC  = ICC_WIN95_CLASSES|ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_USEREX_CLASSES;
  InitCommonControlsEx(&icex);

  msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

  if (!IsWin8()) { hModUxTheme = LoadLibrary(L"uxtheme.dll"); }

  hRichEdit = LoadLibrary(L"RICHED20.DLL");  // Use "RichEdit20W" for control in .rc
  //hRichEdit = LoadLibrary(L"MSFTEDIT.DLL");  // Use "RichEdit50W" for control in .rc

  if (!_RegisterWndClass(g_hInstance)) { return 1; }

  Scintilla_RegisterClasses(g_hInstance);

  HMENU  hMainMenu = NULL;
  if (g_hLngResContainer != g_hInstance) {
    hMainMenu = LoadMenu(g_hLngResContainer, MAKEINTRESOURCE(IDR_MUI_MAINMENU));
    if (!hMainMenu) {
      GetLastErrorToMsgBox(L"LoadMenu()", 0);
    }
  }

  HWND hwnd = InitInstance(g_hInstance, lpCmdLine, nCmdShow);
  if (!hwnd) { return 1; }

  if (hMainMenu) { SetMenu(hwnd, hMainMenu); }

#ifdef _EXTRA_DRAG_N_DROP_HANDLER_
  DragAndDropInit(NULL);
#endif

  hAccMain = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_MAINWND));
  hAccFindReplace = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCFINDREPLACE));
  hAccCoustomizeSchemes = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCCUSTOMSCHEMES));
 
  SetTimer(hwnd, IDT_TIMER_MRKALL, USER_TIMER_MINIMUM, (TIMERPROC)MQ_ExecuteNext);
  
  if (bPrefLngNotAvail) {
    InfoBoxLng(MBWARN, L"MsgPrefLanguageNotAvailable", IDS_WARN_PREF_LNG_NOT_AVAIL, g_tchPrefLngLocName);
  }

  MSG msg;
  while (GetMessage(&msg,NULL,0,0))
  {
    if (IsWindow(g_hwndDlgFindReplace) && ((msg.hwnd == g_hwndDlgFindReplace) || IsChild(g_hwndDlgFindReplace, msg.hwnd))) 
    {
      const int iTr = TranslateAccelerator(g_hwndDlgFindReplace, hAccFindReplace, &msg);
      if (iTr || IsDialogMessage(g_hwndDlgFindReplace, &msg))
        continue;
    }
    if (IsWindow(g_hwndDlgCustomizeSchemes) && ((msg.hwnd == g_hwndDlgCustomizeSchemes) || IsChild(g_hwndDlgCustomizeSchemes, msg.hwnd))) {
      const int iTr = TranslateAccelerator(g_hwndDlgCustomizeSchemes, hAccCoustomizeSchemes, &msg);
      if (iTr || IsDialogMessage(g_hwndDlgCustomizeSchemes, &msg))
        continue;
    }
    if (!TranslateAccelerator(hwnd,hAccMain,&msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  KillTimer(hwnd, IDT_TIMER_MRKALL);

  CmdMessageQueue_t* pmqc = NULL;
  CmdMessageQueue_t* dummy;
  DL_FOREACH_SAFE(MessageQueue, pmqc, dummy)
  {
    DL_DELETE(MessageQueue, pmqc);
    FreeMem(pmqc);
  }

  // Save Settings is done elsewhere

  if (hMainMenu) { DestroyMenu(hMainMenu); }
  Scintilla_ReleaseResources();
  UnregisterClass(wchWndClass, g_hInstance);

  if (hModUxTheme) { FreeLibrary(hModUxTheme); }

  OleUninitialize();

  if (g_hLngResContainer != g_hInstance) { FreeMUILibrary(g_hLngResContainer); }

  return (int)(msg.wParam);
}


//=============================================================================
//
//  _RegisterWndClass()
//
//
static bool __fastcall _RegisterWndClass(HINSTANCE hInstance)
{
  if (!g_hDlgIcon) {
    g_hDlgIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON,
                      GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
  }

  WNDCLASS wc;
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style         = CS_BYTEALIGNWINDOW | CS_DBLCLKS;
  wc.lpfnWndProc   = (WNDPROC)MainWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = g_hDlgIcon;
  wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MUI_MAINMENU);
  wc.lpszClassName = wchWndClass;

  return RegisterClass(&wc);
}


//=============================================================================
//
//  _LngStrToMultiLngStr
//
//
static bool __fastcall _LngStrToMultiLngStr(WCHAR* pLngStr, WCHAR* pLngMultiStr, size_t lngMultiStrSize)
{
  bool rtnVal = true;

  size_t strLen = (size_t)lstrlenW(pLngStr);

  if ((strLen > 0) && pLngMultiStr && (lngMultiStrSize > 0)) {
    WCHAR* lngMultiStrPtr = pLngMultiStr;
    WCHAR* last = pLngStr + (Has_UTF16_LE_BOM((char*)pLngStr,clampi((int)strLen,0,8)) ? 1 : 0);
    while (last && rtnVal) {
      // make sure you validate the user input
      WCHAR* next = StrNextTok(last, L",; :");
      if (next) { *next = L'\0'; }
      strLen = (size_t)StringCchLenW(last, LOCALE_NAME_MAX_LENGTH);
      if ((strLen > 0) && IsValidLocaleName(last)) {
        lngMultiStrPtr[0] = L'\0';
        rtnVal &= SUCCEEDED(StringCchCatW(lngMultiStrPtr, (lngMultiStrSize - (lngMultiStrPtr - pLngMultiStr)), last));
        lngMultiStrPtr += strLen + 1;
      }
      last = (next ? next + 1 : next);
    }
    if (rtnVal && (lngMultiStrSize - (lngMultiStrPtr - pLngMultiStr))) // make sure there is a double null term for the multi-string
    {
      lngMultiStrPtr[0] = L'\0';
    }
    else // fail and guard anyone whom might use the multi-string
    {
      lngMultiStrPtr[0] = L'\0';
      lngMultiStrPtr[1] = L'\0';
    }
  }
  return rtnVal;
}


//=============================================================================
//
//  _LoadLanguageResources
//
//
static HMODULE __fastcall _LoadLanguageResources(LANGID const langID, const WCHAR* locName)
{
  UNUSED(locName);
  bool bLngAvailable = false;
  for (int i = 0; i < COUNTOF(g_iAvailableLanguages); ++i) {
    if (g_iAvailableLanguages[i] == langID) {
      bLngAvailable = true;
      break;
    }
  }
  if (!bLngAvailable) { return NULL; }

  WCHAR tchAvailLngs[SMALL_BUFFER] = { L'\0' };
  StringCchCopyW(tchAvailLngs, SMALL_BUFFER, g_tchAvailableLanguages);
  WCHAR tchUserLangMultiStrg[SMALL_BUFFER] = { L'\0' };
  if (!_LngStrToMultiLngStr(tchAvailLngs, tchUserLangMultiStrg, LARGE_BUFFER))
  {
    GetLastErrorToMsgBox(L"_LngStrToMultiLngStr()", ERROR_MUI_INVALID_LOCALE_NAME);
    return NULL;
  }

  // set the appropriate fallback list
  DWORD langCount = 0;
  // using SetProcessPreferredUILanguages is recommended for new applications (esp. multi-threaded applications)
  if (!SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, tchUserLangMultiStrg, &langCount) || (langCount == 0))
  {
    GetLastErrorToMsgBox(L"SetProcessPreferredUILanguages()", 0);
    return NULL;
  }

  SetThreadUILanguage(langID);



  // NOTES:
  // an application developer that makes the assumption the fallback list provided by the
  // system / OS is entirely sufficient may or may not be making a good assumption based  mostly on:
  // A. your choice of languages installed with your application
  // B. the languages on the OS at application install time
  // C. the OS users propensity to install/uninstall language packs
  // D. the OS users propensity to change language settings

  // obtains access to the proper resource container 
  // for standard Win32 resource loading this is normally a PE module - use LoadLibraryEx
 
  HMODULE hLangResourceContainer = LoadMUILibraryW(L"lng/np3lng.dll", MUI_LANGUAGE_NAME, langID);

  //if (!hLangResourceContainer)
  //{
  //  GetLastErrorToMsgBox(L"LoadMUILibrary", 0);
  //  return NULL;
  //}

  return hLangResourceContainer;
}


//=============================================================================
//
//  WaitCursorStack
//
//
static volatile LONG iWaitCursorStackCounter = 0L;

//=============================================================================
//
//  CheckWaitCursorStack()
//
//
static bool __fastcall CheckWaitCursorStack()
{
  return (InterlockedOr(&iWaitCursorStackCounter, 0L) == 0L);
}


//=============================================================================
//
//  BeginWaitCursor()
//
//
void BeginWaitCursor(LPCWSTR text)
{
  if (CheckWaitCursorStack())
  {
    SciCall_SetCursor(SC_CURSORWAIT); // delayed to SCN_DWELLSTART
    StatusSetText(g_hwndStatus, STATUS_HELP, (LPCWSTR)text);
    //StatusSetTextID(g_hwndStatus, STATUS_HELP, uid);
  }
  InterlockedIncrement(&iWaitCursorStackCounter);
}


//=============================================================================
//
//  EndWaitCursor()
//
//
void EndWaitCursor()
{
  if (!CheckWaitCursorStack()) {
    InterlockedDecrement(&iWaitCursorStackCounter);
  }
  if (CheckWaitCursorStack()) 
  {
    POINT pt;
    SciCall_SetCursor(SC_CURSORNORMAL);
    GetCursorPos(&pt); SetCursorPos(pt.x, pt.y);
    StatusSetSimple(g_hwndStatus, false);
    UpdateStatusbar(false);
  }
}


//=============================================================================
//
//  _InitWindowPosition()
//
//
#define _BORDEROFFSET (IsWin10() ? 8 : 16)


static void __fastcall _InitDefaultWndPos(WININFO* pWinInfo)
{
  RECT rc = RectFromWinInfo(pWinInfo);
  GetMonitorWorkArea(&rc);

  pWinInfo->y = rc.top + _BORDEROFFSET;
  pWinInfo->cy = rc.bottom - rc.top - (_BORDEROFFSET * 2);
  pWinInfo->cx = (rc.right - rc.left) / 2; //min(rc.right - rc.left - 32, g_WinInfo.cy);
  pWinInfo->x = (g_flagDefaultPos == 3) ? rc.left + _BORDEROFFSET : rc.right - g_WinInfo.cx - _BORDEROFFSET;
}
// ----------------------------------------------------------------------------


static void __fastcall _InitWindowPosition()
{
  if (g_flagDefaultPos == 1) 
  {
    g_WinInfo.x = g_WinInfo.y = g_WinInfo.cx = g_WinInfo.cy = CW_USEDEFAULT;
    g_WinInfo.max = 0;
    g_WinInfo.zoom = 100;
  }
  else if (g_flagDefaultPos >= 4) 
  {
    RECT rcMon = RectFromWinInfo(&g_WinInfo);
    GetMonitorWorkArea(&rcMon);

    WININFO wiWorkArea = INIT_WININFO;
    FitIntoMonitorWorkArea(&rcMon, &wiWorkArea, true); // get Monitor and Work Area 
    RECT const rc = RectFromWinInfo(&wiWorkArea); // use Work Area as RECT

    if (g_flagDefaultPos & 8)
      g_WinInfo.x = (rc.right - rc.left) / 2;
    else
      g_WinInfo.x = rc.left;
    g_WinInfo.cx = rc.right - rc.left;
    if (g_flagDefaultPos & (4 | 8))
      g_WinInfo.cx /= 2;
    if (g_flagDefaultPos & 32)
      g_WinInfo.y = (rc.bottom - rc.top) / 2;
    else
      g_WinInfo.y = rc.top;
    g_WinInfo.cy = rc.bottom - rc.top;
    if (g_flagDefaultPos & (16 | 32))
      g_WinInfo.cy /= 2;
    if (g_flagDefaultPos & 64) {
      g_WinInfo.x = rc.left;
      g_WinInfo.y = rc.top;
      g_WinInfo.cx = rc.right - rc.left;
      g_WinInfo.cy = rc.bottom - rc.top;
    }
    if (g_flagDefaultPos & 128) {
      g_WinInfo.x += (g_flagDefaultPos & 8) ? 4 : 8;
      g_WinInfo.cx -= (g_flagDefaultPos & (4 | 8)) ? 12 : 16;
      g_WinInfo.y += (g_flagDefaultPos & 32) ? 4 : 8;
      g_WinInfo.cy -= (g_flagDefaultPos & (16 | 32)) ? 12 : 16;
      g_WinInfo.max = 1;
      g_WinInfo.zoom = 100;
    }
  }
  else if (g_flagDefaultPos == 2 || g_flagDefaultPos == 3) // NP3 default window position
  {
    _InitDefaultWndPos(&g_WinInfo);
  }
  else { // restore window, move upper left corner to Work Area 
    
    RECT rcMon = RectFromWinInfo(&g_WinInfo);
    GetMonitorWorkArea(&rcMon);

    WININFO wiWin = g_WinInfo; wiWin.cx = wiWin.cy = _BORDEROFFSET * 2; // really small
    FitIntoMonitorWorkArea(&rcMon, &wiWin, false);

    g_WinInfo.x = wiWin.x;
    g_WinInfo.y = wiWin.y;

  }

  g_WinCurrentWidth = g_WinInfo.cx;
}


//=============================================================================
//
//  InitInstance()
//
//
HWND InitInstance(HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow)
{
  UNUSED(pszCmdLine);
 
  _InitWindowPosition();
  
  // get monitor coordinates from g_WinInfo
  WININFO srcninfo = g_WinInfo;
  WinInfoToScreen(&srcninfo);

  g_hwndMain = CreateWindowEx(
               0,
               wchWndClass,
               TEXT(APPNAME),
               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
               srcninfo.x,
               srcninfo.y,
               srcninfo.cx,
               srcninfo.cy,
               NULL,
               NULL,
               hInstance,
               NULL);

  if (g_WinInfo.max) {
    nCmdShow = SW_SHOWMAXIMIZED;
  }
  if ((bAlwaysOnTop || g_flagAlwaysOnTop == 2) && g_flagAlwaysOnTop != 1) {
    SetWindowPos(g_hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }

  if (bTransparentMode) {
    SetWindowTransparentMode(g_hwndMain, true);
  }
  
  if (g_WinInfo.zoom) {
    SciCall_SetZoom(g_WinInfo.zoom);
  }
  // Current file information -- moved in front of ShowWindow()
  FileLoad(true,true,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,L"");

  if (!g_flagStartAsTrayIcon) {
    ShowWindow(g_hwndMain,nCmdShow);
    UpdateWindow(g_hwndMain);
  }
  else {
    ShowWindow(g_hwndMain,SW_HIDE);    // trick ShowWindow()
    ShowNotifyIcon(g_hwndMain,true);
  }

  // Source Encoding
  if (lpEncodingArg)
    Encoding_SrcCmdLn(Encoding_MatchW(lpEncodingArg));

  // Pathname parameter
  if (g_flagBufferFile || (lpFileArg /*&& !g_flagNewFromClipboard*/))
  {
    bool bOpened = false;

    // Open from Directory
    if (!g_flagBufferFile && PathIsDirectory(lpFileArg)) {
      WCHAR tchFile[MAX_PATH] = { L'\0' };
      if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), lpFileArg))
        bOpened = FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
    }
    else {
      LPCWSTR lpFileToOpen = g_flagBufferFile ? g_szTmpFilePath : lpFileArg;
      bOpened = FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, lpFileToOpen);
      if (bOpened) {
        if (g_flagBufferFile) {
          if (lpFileArg) {
            InstallFileWatching(NULL); // Terminate file watching
            StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),lpFileArg);
            InstallFileWatching(g_wchCurFile);
          }
          else
            StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),L"");

          if (!g_flagLexerSpecified)
            Style_SetLexerFromFile(g_hwndEdit,g_wchCurFile);

          _SetDocumentModified(true);
          UpdateLineNumberWidth();

          // check for temp file and delete
          if (flagIsElevated && PathFileExists(g_szTmpFilePath)) {
            DeleteFile(g_szTmpFilePath);
          }
        }
        if (g_flagJumpTo) { // Jump to position
          EditJumpTo(g_hwndEdit,iInitialLine,iInitialColumn);
        }
      }
    }
    if (lpFileArg) {
      FreeMem(lpFileArg);
      lpFileArg = NULL;
    }
    if (bOpened) {
      if (g_flagChangeNotify == 1) {
        g_iFileWatchingMode = 0;
        g_bResetFileWatching = true;
        InstallFileWatching(g_wchCurFile);
      }
      else if (g_flagChangeNotify == 2) {
        if (!g_bChasingDocTail) { 
          SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0); 
        }
        else {
          g_iFileWatchingMode = 2;
          g_bResetFileWatching = true;
          InstallFileWatching(g_wchCurFile);
        }
      }
    }
  }
  else {
    if (Encoding_SrcCmdLn(CPI_GET) != CPI_NONE) {
      Encoding_Current(Encoding_SrcCmdLn(CPI_GET));
      Encoding_HasChanged(Encoding_SrcCmdLn(CPI_GET));
    }
  }

  // reset
  Encoding_SrcCmdLn(CPI_NONE);
  g_flagQuietCreate = 0;
  fKeepTitleExcerpt = 0;

  // undo / redo selections
  if (UndoRedoSelectionUTArray != NULL) {
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_free(UndoRedoSelectionUTArray);
    UndoRedoSelectionUTArray = NULL;
  }
  utarray_new(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
  utarray_reserve(UndoRedoSelectionUTArray,256);

  // Check for /c [if no file is specified] -- even if a file is specified
  /*else */if (g_flagNewFromClipboard) {
    if (SendMessage(g_hwndEdit, SCI_CANPASTE, 0, 0)) {
      bool bAutoIndent2 = bAutoIndent;
      bAutoIndent = 0;
      EditJumpTo(g_hwndEdit, -1, 0);
      _BEGIN_UNDO_ACTION_;
      if (SendMessage(g_hwndEdit, SCI_GETLENGTH, 0, 0) > 0) {
        SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      }
      SendMessage(g_hwndEdit, SCI_PASTE, 0, 0);
      SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      _END_UNDO_ACTION_;
      bAutoIndent = bAutoIndent2;
      if (g_flagJumpTo)
        EditJumpTo(g_hwndEdit, iInitialLine, iInitialColumn);
      else
        EditEnsureSelectionVisible(g_hwndEdit);
    }
  }

  // Encoding
  if (0 != g_flagSetEncoding) {
    SendMessage(
      g_hwndMain,
      WM_COMMAND,
      MAKELONG(IDM_ENCODING_ANSI + g_flagSetEncoding -1,1),
      0);
    g_flagSetEncoding = 0;
  }

  // EOL mode
  if (0 != g_flagSetEOLMode) {
    SendMessage(
      g_hwndMain,
      WM_COMMAND,
      MAKELONG(IDM_LINEENDINGS_CRLF + g_flagSetEOLMode -1,1),
      0);
    g_flagSetEOLMode = 0;
  }

  // Match Text
  if (g_flagMatchText && lpMatchArg) {
    if (StrIsNotEmpty(lpMatchArg) && SendMessage(g_hwndEdit,SCI_GETLENGTH,0,0)) {

      WideCharToMultiByteStrg(Encoding_SciCP,lpMatchArg,g_efrData.szFind);

      if (g_flagMatchText & 4)
        g_efrData.fuFlags |= (SCFIND_REGEXP | SCFIND_POSIX);
      else if (g_flagMatchText & 8)
        g_efrData.bTransformBS = true;

      if (g_flagMatchText & 2) {
        if (!g_flagJumpTo) { SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0); }
        EditFindPrev(g_hwndEdit,&g_efrData,false,false);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      else {
        if (!g_flagJumpTo) { SendMessage(g_hwndEdit, SCI_DOCUMENTSTART, 0, 0); }
        EditFindNext(g_hwndEdit,&g_efrData,false,false);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
    }
    LocalFree(lpMatchArg);
    lpMatchArg = NULL;
  }

  // Check for Paste Board option -- after loading files
  if (g_flagPasteBoard) {
    bLastCopyFromMe = true;
    hwndNextCBChain = SetClipboardViewer(g_hwndMain);
    uidsAppTitle = IDS_MUI_APPTITLE_PASTEBOARD;
    bLastCopyFromMe = false;

    dwLastCopyTime = 0;
    SetTimer(g_hwndMain,ID_PASTEBOARDTIMER,100,PasteBoardTimer);
  }

  // check if a lexer was specified from the command line
  if (g_flagLexerSpecified) {
    if (lpSchemeArg) {
      Style_SetLexerFromName(g_hwndEdit,g_wchCurFile,lpSchemeArg);
      LocalFree(lpSchemeArg);
    }
    else if (iInitialLexer >=0 && iInitialLexer < NUMLEXERS)
      Style_SetLexerFromID(g_hwndEdit,iInitialLexer);
    g_flagLexerSpecified = 0;
  }

  // If start as tray icon, set current filename as tooltip
  if (g_flagStartAsTrayIcon)
    SetNotifyIconTitle(g_hwndMain);

  g_iReplacedOccurrences = 0;
  g_iMarkOccurrencesCount = (g_iMarkOccurrences > 0) ? 0 : -1;

  UpdateToolbar();
  UpdateStatusbar(false);
  UpdateLineNumberWidth();

  // print file immediately and quit
  if (g_flagPrintFileAndLeave)
  {
    SHFILEINFO shfi;
    WCHAR *pszTitle;
    WCHAR tchUntitled[32] = { L'\0' };
    WCHAR tchPageFmt[32] = { L'\0' };

    if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
      SHGetFileInfo2(g_wchCurFile, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
      pszTitle = shfi.szDisplayName;
    }
    else {
      GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
      pszTitle = tchUntitled;
    }

    GetLngString(IDS_MUI_PRINT_PAGENUM, tchPageFmt, COUNTOF(tchPageFmt));

    if (!EditPrint(g_hwndEdit, pszTitle, tchPageFmt))
      MsgBoxLng(MBWARN, IDS_MUI_PRINT_ERROR, pszTitle);

    PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
  }
  return(g_hwndMain);
}


//=============================================================================
//
//  MainWndProc()
//
//  Messages are distributed to the MsgXXX-handlers
//
//
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static bool bAltKeyIsDown = false;

  switch(umsg)
  {
    // Quickly handle painting and sizing messages, found in ScintillaWin.cxx
    // Cool idea, don't know if this has any effect... ;-)
    case WM_MOVE:
    case WM_MOUSEACTIVATE:
    case WM_NCHITTEST:
    case WM_NCCALCSIZE:
    case WM_NCPAINT:
    case WM_PAINT:
    case WM_ERASEBKGND:
    case WM_NCMOUSEMOVE:
    case WM_NCLBUTTONDOWN:
    case WM_WINDOWPOSCHANGING:
    case WM_WINDOWPOSCHANGED:
    case WM_TIMER:
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    case WM_SYSKEYDOWN:
      if (GetAsyncKeyState(VK_MENU) & SHRT_MIN)  // ALT-KEY DOWN
      {
        if (!bAltKeyIsDown) {
          bAltKeyIsDown = true;
          if (!g_bDenyVirtualSpaceAccess) {
            SciCall_SetVirtualSpaceOptions(SCVS_RECTANGULARSELECTION | SCVS_NOWRAPLINESTART | SCVS_USERACCESSIBLE);
          }
        }
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);
       
    case WM_SYSKEYUP:
      if (!(GetAsyncKeyState(VK_MENU) & SHRT_MIN))  // NOT ALT-KEY DOWN
      {
        if (bAltKeyIsDown) {
          bAltKeyIsDown = false;
          SciCall_SetVirtualSpaceOptions(g_bDenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION);
        }
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    case WM_KILLFOCUS:
      if (bAltKeyIsDown) {
        bAltKeyIsDown = false;
        SciCall_SetVirtualSpaceOptions(g_bDenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION);
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    case WM_CREATE:
      return MsgCreate(hwnd, wParam, lParam);


    case WM_DESTROY:
    case WM_ENDSESSION:
      return MsgEndSession(hwnd, umsg, wParam, lParam);

    case WM_CLOSE:
      if (FileSave(false, true, false, false)) {
        DestroyWindow(hwnd);
      }
      break;

    case WM_QUERYENDSESSION:
      if (FileSave(false, true, false, false)) {
        return 1LL;
      }
      break;

    // Reinitialize theme-dependent values and resize windows
    case WM_THEMECHANGED:
      return MsgThemeChanged(hwnd, wParam, lParam);

    case WM_DPICHANGED:
      return MsgDPIChanged(hwnd, wParam, lParam);

    // update Scintilla colors
    case WM_SYSCOLORCHANGE:
      UpdateLineNumberWidth();
      MarkAllOccurrences(0, true);
      UpdateVisibleUrlHotspot(0);
      return DefWindowProc(hwnd,umsg,wParam,lParam);

    case WM_SIZE:
      return MsgSize(hwnd, wParam, lParam);

    case WM_SETFOCUS:
      SetFocus(g_hwndEdit);
      break;

    case WM_DROPFILES:
      return MsgDropFiles(hwnd, wParam, lParam);
      // see SCN_URIDROPP
      break;

    case WM_COPYDATA:
      return MsgCopyData(hwnd, wParam, lParam);

    case WM_CONTEXTMENU:
      return MsgContextMenu(hwnd, umsg, wParam, lParam);

    case WM_INITMENU:
      return MsgInitMenu(hwnd, wParam, lParam);

    case WM_NOTIFY:
      return MsgNotify(hwnd, wParam, lParam);

    case WM_CHANGENOTIFY:
      return MsgChangeNotify(hwnd, wParam, lParam);

    //case WM_PARENTNOTIFY:
    //  if (LOWORD(wParam) & WM_DESTROY) {
    //    if (IsWindow(hDlgFindReplace) && (hDlgFindReplace == (HWND)lParam)) {
    //      hDlgFindReplace = NULL;
    //    }
    //  }
    //  break;

    case WM_COMMAND:
      return MsgCommand(hwnd, umsg, wParam, lParam);

    case WM_TRAYMESSAGE:
      return MsgTrayMessage(hwnd, wParam, lParam);

    //// This message is posted before Notepad3 reactivates itself
    //case WM_CHANGENOTIFYCLEAR:
    //  bPendingChangeNotify = false;
    //  break;

    case WM_DRAWCLIPBOARD:
      if (!bLastCopyFromMe)
        dwLastCopyTime = GetTickCount();
      else
        bLastCopyFromMe = false;

      if (hwndNextCBChain)
        SendMessage(hwndNextCBChain,WM_DRAWCLIPBOARD,wParam,lParam);
      break;

    case WM_CHANGECBCHAIN:
      if ((HWND)wParam == hwndNextCBChain)
        hwndNextCBChain = (HWND)lParam;
      if (hwndNextCBChain)
        SendMessage(hwndNextCBChain,WM_CHANGECBCHAIN,lParam,wParam);
      break;

    case WM_SYSCOMMAND:
      return MsgSysCommand(hwnd, umsg, wParam, lParam);
      break;

    case WM_MOUSEWHEEL:
      if (wParam & MK_CONTROL) { EditShowZoomCallTip(g_hwndEdit); }
      break;

    default:
      if (umsg == msgTaskbarCreated) {
        if (!IsWindowVisible(hwnd)) { ShowNotifyIcon(hwnd, true); }
        SetNotifyIconTitle(hwnd);
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);
  }
  return 0; // 0 = swallow message
}



//=============================================================================
//
//  _SetWrapStartIndent()
//
static void __fastcall _SetWrapStartIndent(HWND hwndEditCtrl)
{
  int i = 0;
  switch (iWordWrapIndent) {
  case 1: i = 1; break;
  case 2: i = 2; break;
  case 3: i = (g_iIndentWidth) ? 1 * g_iIndentWidth : 1 * g_iTabWidth; break;
  case 4: i = (g_iIndentWidth) ? 2 * g_iIndentWidth : 2 * g_iTabWidth; break;
  }
  SendMessage(hwndEditCtrl, SCI_SETWRAPSTARTINDENT, i, 0);
}


//=============================================================================
//
//  _SetWrapIndentMode()
//
static void __fastcall _SetWrapIndentMode(HWND hwndEditCtrl)
{
  int const wrap_mode = (!g_bWordWrap ? SC_WRAP_NONE : ((iWordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR));

  SendMessage(hwndEditCtrl, SCI_SETWRAPMODE, wrap_mode, 0);

  if (iWordWrapIndent == 5) {
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
  }
  else if (iWordWrapIndent == 6) {
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);
  }
  else if (iWordWrapIndent == 7) {
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_DEEPINDENT, 0);
  }
  else {
    _SetWrapStartIndent(hwndEditCtrl);
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_FIXED, 0);
  }
}


//=============================================================================
//
//  _SetWrapVisualFlags()
//
static void __fastcall _SetWrapVisualFlags(HWND hwndEditCtrl)
{
  if (bShowWordWrapSymbols) {
    int wrapVisualFlags = 0;
    int wrapVisualFlagsLocation = 0;
    if (iWordWrapSymbols == 0) {
      iWordWrapSymbols = 22;
    }
    switch (iWordWrapSymbols % 10) {
    case 1:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
      wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_END_BY_TEXT;
      break;
    case 2:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
      break;
    }
    switch (((iWordWrapSymbols % 100) - (iWordWrapSymbols % 10)) / 10) {
    case 1:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
      wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_START_BY_TEXT;
      break;
    case 2:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_START;
      break;
    }
    SendMessage(hwndEditCtrl, SCI_SETWRAPVISUALFLAGSLOCATION, wrapVisualFlagsLocation, 0);
    SendMessage(hwndEditCtrl, SCI_SETWRAPVISUALFLAGS, wrapVisualFlags, 0);
  }
  else {
    SendMessage(hwndEditCtrl, SCI_SETWRAPVISUALFLAGS, 0, 0);
  }
}



//=============================================================================
//
//  InitializeSciEditCtrl()
//
static void __fastcall _InitializeSciEditCtrl(HWND hwndEditCtrl)
{
  if (IsVista()) {
    // Current platforms perform window buffering so it is almost always better for this option to be turned off.
    // There are some older platforms and unusual modes where buffering may still be useful - so keep it ON
    //~SciCall_SetBufferedDraw(true);  // default is true 
    if (g_iRenderingTechnology > 0) {
      SciCall_SetTechnology(DirectWriteTechnology[g_iRenderingTechnology]);
      SciCall_SetBufferedDraw(false);
      // experimental
      SciCall_SetBidirectional(SciBidirectional[g_iBidirectional]);
    }
  }
  Encoding_Current(g_iDefaultNewFileEncoding);

  //int const evtMask = SC_MODEVENTMASKALL;
  // The possible notification types are the same as the modificationType bit flags used by SCN_MODIFIED: 
  // SC_MOD_INSERTTEXT, SC_MOD_DELETETEXT, SC_MOD_CHANGESTYLE, SC_MOD_CHANGEFOLD, SC_PERFORMED_USER, 
  // SC_PERFORMED_UNDO, SC_PERFORMED_REDO, SC_MULTISTEPUNDOREDO, SC_LASTSTEPINUNDOREDO, SC_MOD_CHANGEMARKER, 
  // SC_MOD_BEFOREINSERT, SC_MOD_BEFOREDELETE, SC_MULTILINEUNDOREDO, and SC_MODEVENTMASKALL.
  //
  ///~ Don't use: SC_PERFORMED_USER | SC_MOD_CHANGESTYLE;
  int const evtMask1 = SC_MOD_CONTAINER | SC_PERFORMED_UNDO | SC_PERFORMED_REDO;
  int const evtMask2 = SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE;

  SendMessage(hwndEditCtrl, SCI_SETMODEVENTMASK, (WPARAM)(evtMask1 | evtMask2), 0);

  SendMessage(hwndEditCtrl, SCI_SETCODEPAGE, (WPARAM)SC_CP_UTF8, 0); // fixed internal UTF-8 

  SendMessage(hwndEditCtrl, SCI_SETEOLMODE, SC_EOL_CRLF, 0);
  SendMessage(hwndEditCtrl, SCI_SETPASTECONVERTENDINGS, true, 0);
  SendMessage(hwndEditCtrl, SCI_USEPOPUP, false, 0);
  SendMessage(hwndEditCtrl, SCI_SETSCROLLWIDTH, 1, 0);
  SendMessage(hwndEditCtrl, SCI_SETSCROLLWIDTHTRACKING, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETENDATLASTLINE, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETMOUSESELECTIONRECTANGULARSWITCH, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETMULTIPLESELECTION, false, 0);
  SendMessage(hwndEditCtrl, SCI_SETADDITIONALSELECTIONTYPING, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETADDITIONALCARETSBLINK, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETADDITIONALCARETSVISIBLE, true, 0);
  SendMessage(hwndEditCtrl, SCI_SETVIRTUALSPACEOPTIONS, SCVS_NONE, 0);
  SendMessage(hwndEditCtrl, SCI_SETLAYOUTCACHE, SC_CACHE_PAGE, 0);
  
  // assign command keys
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_NEXT + (SCMOD_CTRL << 16)), SCI_PARADOWN);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_PRIOR + (SCMOD_CTRL << 16)), SCI_PARAUP);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_NEXT + ((SCMOD_CTRL | SCMOD_SHIFT) << 16)), SCI_PARADOWNEXTEND);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_PRIOR + ((SCMOD_CTRL | SCMOD_SHIFT) << 16)), SCI_PARAUPEXTEND);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_HOME + (0 << 16)), SCI_VCHOMEWRAP);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_END + (0 << 16)), SCI_LINEENDWRAP);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_HOME + (SCMOD_SHIFT << 16)), SCI_VCHOMEWRAPEXTEND);
  SendMessage(hwndEditCtrl, SCI_ASSIGNCMDKEY, (SCK_END + (SCMOD_SHIFT << 16)), SCI_LINEENDWRAPEXTEND);

  // set indicator styles (foreground and alpha maybe overridden by style settings)
  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_MARK_OCCURANCE, INDIC_ROUNDBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_MARK_OCCURANCE, RGB(0x00, 0x00, 0xFF));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_MARK_OCCURANCE, 100);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MARK_OCCURANCE, 100);

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_MATCH_BRACE, INDIC_FULLBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_MATCH_BRACE, RGB(0x00, 0xFF, 0x00));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_MATCH_BRACE, 120);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MATCH_BRACE, 120);

  SendMessage(hwndEditCtrl, SCI_INDICSETSTYLE, INDIC_NP3_BAD_BRACE, INDIC_FULLBOX);
  SendMessage(hwndEditCtrl, SCI_INDICSETFORE, INDIC_NP3_BAD_BRACE, RGB(0xFF, 0x00, 0x00));
  SendMessage(hwndEditCtrl, SCI_INDICSETALPHA, INDIC_NP3_BAD_BRACE, 120);
  SendMessage(hwndEditCtrl, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_BAD_BRACE, 120);

  // paste into rectangular selection
  SendMessage(hwndEditCtrl, SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH, 0);

  // No SC_AUTOMATICFOLD_CLICK, performed by 
  SendMessage(hwndEditCtrl, SCI_SETAUTOMATICFOLD, (WPARAM)(SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CHANGE), 0);

  // Properties
  SendMessage(hwndEditCtrl, SCI_SETCARETSTICKY, (WPARAM)SC_CARETSTICKY_OFF, 0);
  //SendMessage(hwndEditCtrl,SCI_SETCARETSTICKY,SC_CARETSTICKY_WHITESPACE,0);
  
  SendMessage(hwndEditCtrl, SCI_SETMOUSEDWELLTIME, (WPARAM)SC_TIME_FOREVER, 0); // default
  //SendMessage(hwndEditCtrl, SCI_SETMOUSEDWELLTIME, (WPARAM)500, 0);
  

  #define _CARET_SYMETRY CARET_EVEN /// CARET_EVEN or 0
  #define _CARET_ENFORCE CARET_STRICT /// CARET_STRICT or 0
  if (iCurrentLineHorizontalSlop > 0)
    SendMessage(hwndEditCtrl, SCI_SETXCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), iCurrentLineHorizontalSlop);
  else
    SendMessage(hwndEditCtrl, SCI_SETXCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), (LPARAM)0);

  if (iCurrentLineVerticalSlop > 0)
    SendMessage(hwndEditCtrl, SCI_SETYCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), iCurrentLineVerticalSlop);
  else
    SendMessage(hwndEditCtrl, SCI_SETYCARETPOLICY, (WPARAM)(_CARET_SYMETRY), 0);

  SendMessage(hwndEditCtrl, SCI_SETVIRTUALSPACEOPTIONS, (WPARAM)(g_bDenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION), 0);
  SendMessage(hwndEditCtrl, SCI_SETENDATLASTLINE, (WPARAM)((bScrollPastEOF) ? 0 : 1), 0);

  // Tabs
  SendMessage(hwndEditCtrl, SCI_SETUSETABS, (WPARAM)!g_bTabsAsSpaces, 0);
  SendMessage(hwndEditCtrl, SCI_SETTABINDENTS, (WPARAM)g_bTabIndents, 0);
  SendMessage(hwndEditCtrl, SCI_SETBACKSPACEUNINDENTS, (WPARAM)bBackspaceUnindents, 0);
  SendMessage(hwndEditCtrl, SCI_SETTABWIDTH, (WPARAM)g_iTabWidth, 0);
  SendMessage(hwndEditCtrl, SCI_SETINDENT, (WPARAM)g_iIndentWidth, 0);

  // Indent Guides
  Style_SetIndentGuides(hwndEditCtrl, bShowIndentGuides);

  // Word Wrap
  _SetWrapIndentMode(hwndEditCtrl);
  _SetWrapVisualFlags(hwndEditCtrl);

  // Long Lines
  if (g_bMarkLongLines)
    SendMessage(hwndEditCtrl, SCI_SETEDGEMODE, (WPARAM)((iLongLineMode == EDGE_LINE) ? EDGE_LINE : EDGE_BACKGROUND), 0);
  else
    SendMessage(hwndEditCtrl, SCI_SETEDGEMODE, (WPARAM)EDGE_NONE, 0);

  SendMessage(hwndEditCtrl, SCI_SETEDGECOLUMN, (WPARAM)g_iLongLinesLimit, 0);

  // general margin
  SendMessage(hwndEditCtrl, SCI_SETMARGINOPTIONS, (WPARAM)SC_MARGINOPTION_SUBLINESELECT, 0);

  // Nonprinting characters
  SendMessage(hwndEditCtrl, SCI_SETVIEWWS, (WPARAM)(bViewWhiteSpace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE), 0);
  SendMessage(hwndEditCtrl, SCI_SETVIEWEOL, (WPARAM)bViewEOLs, 0);

  // IME Interaction
  SendMessage(hwndEditCtrl, SCI_SETIMEINTERACTION, (WPARAM)(g_IMEInteraction ? SC_IME_INLINE : SC_IME_WINDOWED), 0);

  // word delimiter handling
  EditInitWordDelimiter(hwndEditCtrl);
  EditSetAccelWordNav(hwndEditCtrl, g_bAccelWordNavigation);

  // Init default values for printing
  EditPrintInit();

  //SciInitThemes(hwndEditCtrl);

  UpdateLineNumberWidth();
}


//=============================================================================
//
//  MsgCreate() - Handles WM_CREATE
//
//
LRESULT MsgCreate(HWND hwnd, WPARAM wParam,LPARAM lParam)
{
  UNUSED(wParam);

  HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

  g_uCurrentDPI = GetCurrentDPI(hwnd);
  g_uCurrentPPI = GetCurrentPPI(hwnd);

  // Setup edit control
  g_hwndEdit = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    L"Scintilla",
    NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
    0, 0, 0, 0,
    hwnd,
    (HMENU)IDC_EDIT,
    hInstance,
    NULL);

  g_hScintilla = (HANDLE)SendMessage(g_hwndEdit, SCI_GETDIRECTPOINTER, 0, 0);

  _InitializeSciEditCtrl(g_hwndEdit);

  hwndEditFrame = CreateWindowEx(
                    WS_EX_CLIENTEDGE,
                    WC_LISTVIEW,
                    NULL,
                    WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                    0,0,100,100,
                    hwnd,
                    (HMENU)IDC_EDITFRAME,
                    hInstance,
                    NULL);

  if (PrivateIsAppThemed()) {

    RECT rc, rc2;

    bIsAppThemed = true;

    SetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE,GetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(g_hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    if (IsVista()) {
      cxEditFrame = 0;
      cyEditFrame = 0;
    }

    else {
      GetClientRect(hwndEditFrame,&rc);
      GetWindowRect(hwndEditFrame,&rc2);

      cxEditFrame = ((rc2.right-rc2.left) - (rc.right-rc.left)) / 2;
      cyEditFrame = ((rc2.bottom-rc2.top) - (rc.bottom-rc.top)) / 2;
    }
  }
  else {
    bIsAppThemed = false;

    cxEditFrame = 0;
    cyEditFrame = 0;
  }

  // Create Toolbar and Statusbar
  CreateBars(hwnd,hInstance);

  // Window Initialization

  CreateWindow(
    WC_STATIC,
    NULL,
    WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
    0,0,10,10,
    hwnd,
    (HMENU)IDC_FILENAME,
    hInstance,
    NULL);

  SetDlgItemText(hwnd,IDC_FILENAME,g_wchCurFile);

  CreateWindow(
    WC_STATIC,
    NULL,
    WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
    10,10,10,10,
    hwnd,
    (HMENU)IDC_REUSELOCK,
    hInstance,
    NULL);

  SetDlgItemInt(hwnd,IDC_REUSELOCK,GetTickCount(),false);

  // Menu
  //SetMenuDefaultItem(GetSubMenu(GetMenu(hwnd),0),0);

  // Drag & Drop
  DragAcceptFiles(hwnd,true);
#ifdef _EXTRA_DRAG_N_DROP_HANDLER_
  pDropTarget = RegisterDragAndDrop(hwnd, &cfDrpF, 1, WM_NULL, DropFilesProc, (void*)g_hwndEdit);
#endif

  // File MRU
  g_pFileMRU = MRU_Create(L"Recent Files", MRU_NOCASE, MRU_ITEMSFILE);
  MRU_Load(g_pFileMRU);

  g_pMRUfind = MRU_Create(L"Recent Find", (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
  MRU_Load(g_pMRUfind);
  SetFindPattern(g_pMRUfind->pszItems[0]);

  g_pMRUreplace = MRU_Create(L"Recent Replace", (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
  MRU_Load(g_pMRUreplace);

  if (g_hwndEdit == NULL || hwndEditFrame == NULL ||
    g_hwndStatus == NULL || g_hwndToolbar == NULL || g_hwndReBar == NULL)
    return -1LL;

  Style_SetDefaultLexer(g_hwndEdit);

  UpdateLineNumberWidth();
  ObserveNotifyChangeEvent();

  return 0LL;
}


//=============================================================================
//
//  CreateBars() - Create Toolbar and Statusbar
//
//
void CreateBars(HWND hwnd, HINSTANCE hInstance)
{
  DWORD dwToolbarStyle = NP3_WS_TOOLBAR;
  g_hwndToolbar = CreateWindowEx(0,TOOLBARCLASSNAME,NULL,dwToolbarStyle,
                               0,0,0,0,hwnd,(HMENU)IDC_TOOLBAR,hInstance,NULL);

  SendMessage(g_hwndToolbar,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

  // Add Toolbar Bitmap
  HBITMAP hbmp = NULL;
  HBITMAP hbmpCopy = NULL;
  WCHAR szTmp[MAX_PATH] = { L'\0' };
  if (StringCchLenW(g_tchToolbarBitmap,COUNTOF(g_tchToolbarBitmap)))
  {
    if (!SearchPath(NULL,g_tchToolbarBitmap,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),g_tchToolbarBitmap);
    hbmp = LoadImage(NULL,szTmp,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
  }

  if (hbmp) {
    g_bExternalBitmap = true;
  }
  else {
    LPWSTR toolBarIntRes = (iHighDpiToolBar > 0) ? MAKEINTRESOURCE(IDR_MAINWNDTB2) : MAKEINTRESOURCE(IDR_MAINWNDTB);
    hbmp = LoadImage(hInstance, toolBarIntRes, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  }

  hbmpCopy = CopyImage(hbmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

  // adjust to current DPI
  hbmp = ResizeImageForCurrentDPI(hbmp);
  hbmpCopy = ResizeImageForCurrentDPI(hbmpCopy);
 

  BITMAP bmp;
  GetObject(hbmp,sizeof(BITMAP),&bmp);
  HIMAGELIST himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
  ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
  DeleteObject(hbmp);
  SendMessage(g_hwndToolbar,TB_SETIMAGELIST,0,(LPARAM)himl);

  // Optionally add hot Toolbar Bitmap
  hbmp = NULL;
  if (StringCchLenW(g_tchToolbarBitmapHot,COUNTOF(g_tchToolbarBitmapHot)))
  {
    if (!SearchPath(NULL,g_tchToolbarBitmapHot,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),g_tchToolbarBitmapHot);

    hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    hbmp = ResizeImageForCurrentDPI(hbmp);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(g_hwndToolbar,TB_SETHOTIMAGELIST,0,(LPARAM)himl);
    }
  }

  // Optionally add disabled Toolbar Bitmap
  hbmp = NULL;
  if (StringCchLenW(g_tchToolbarBitmapDisabled,COUNTOF(g_tchToolbarBitmapDisabled)))
  {
    if (!SearchPath(NULL,g_tchToolbarBitmapDisabled,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),g_tchToolbarBitmapDisabled);

    hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    hbmp = ResizeImageForCurrentDPI(hbmp);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(g_hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
      g_bExternalBitmap = true;
    }
  }

  if (!g_bExternalBitmap) {
    bool fProcessed = false;
    if (g_flagToolbarLook == 1)
      fProcessed = BitmapAlphaBlend(hbmpCopy,GetSysColor(COLOR_3DFACE),0x60);
    else if (g_flagToolbarLook == 2 || (!IsXP() && g_flagToolbarLook == 0))
      fProcessed = BitmapGrayScale(hbmpCopy);
    if (fProcessed && !IsXP())
      BitmapMergeAlpha(hbmpCopy,GetSysColor(COLOR_3DFACE));
    if (fProcessed) {
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmpCopy,CLR_DEFAULT);
      SendMessage(g_hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
    }
  }
  if (hbmpCopy) {
    DeleteObject(hbmpCopy);
  }

  // Load toolbar labels
  WCHAR* pIniSection = LocalAlloc(LPTR,sizeof(WCHAR) * 32 * 1024);
  if (pIniSection) {
    int cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);
    LoadIniSection(L"Toolbar Labels", pIniSection, cchIniSection);
    WCHAR tchDesc[256] = { L'\0' };
    WCHAR tchIndex[256] = { L'\0' };
    for (int i = 0; i < COUNTOF(tbbMainWnd); ++i) {
      if (tbbMainWnd[i].fsStyle == BTNS_SEP) { continue; }

      int n = tbbMainWnd[i].iBitmap + 1;
      StringCchPrintf(tchIndex, COUNTOF(tchIndex), L"%02i", n);

      if (IniSectionGetString(pIniSection, tchIndex, L"", tchDesc, COUNTOF(tchDesc)) > 0) {
        tbbMainWnd[i].iString = SendMessage(g_hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc);
        tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
      }
      else {
        GetLngString(tbbMainWnd[i].idCommand, tchDesc, COUNTOF(tchDesc));
        tbbMainWnd[i].iString = SendMessage(g_hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc); // tooltip
        tbbMainWnd[i].fsStyle &= ~(BTNS_AUTOSIZE | BTNS_SHOWTEXT);
      }
    }
    LocalFree(pIniSection);
  }

  //~SendMessage(g_hwndToolbar, TB_SETMAXTEXTROWS, 0, 0);

  SendMessage(g_hwndToolbar,TB_SETEXTENDEDSTYLE,0,
    (SendMessage(g_hwndToolbar,TB_GETEXTENDEDSTYLE,0,0) | (TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DOUBLEBUFFER)));

  SendMessage(g_hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)tbbMainWnd);

  if (Toolbar_SetButtons(g_hwndToolbar, IDT_FILE_NEW, g_tchToolbarButtons, tbbMainWnd, COUNTOF(tbbMainWnd)) == 0) {
    SendMessage(g_hwndToolbar, TB_ADDBUTTONS, NUMINITIALTOOLS, (LPARAM)tbbMainWnd);
  }
  RECT rc;
  SendMessage(g_hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
  //SendMessage(g_hwndToolbar,TB_SETINDENT,2,0);


  // Create Statusbar 
  DWORD const dwStatusbarStyle = bShowStatusbar ? (WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE) : (WS_CHILD | WS_CLIPSIBLINGS);
  g_hwndStatus = CreateStatusWindow(dwStatusbarStyle,NULL,hwnd,IDC_STATUSBAR);


  // Create ReBar and add Toolbar
  DWORD const dwReBarStyle = bShowToolbar ? (NP3_WS_REBAR | WS_VISIBLE) : (NP3_WS_REBAR);
  g_hwndReBar = CreateWindowEx(WS_EX_TOOLWINDOW,REBARCLASSNAME,NULL,dwReBarStyle,
                             0,0,0,0,hwnd,(HMENU)IDC_REBAR,hInstance,NULL);

  REBARINFO rbi;
  rbi.cbSize = sizeof(REBARINFO);
  rbi.fMask  = 0;
  rbi.himl   = (HIMAGELIST)NULL;
  SendMessage(g_hwndReBar,RB_SETBARINFO,0,(LPARAM)&rbi);


  REBARBANDINFO rbBand;
  bool const bIsPrivAppThemed = PrivateIsAppThemed();

  rbBand.cbSize  = sizeof(REBARBANDINFO);
  rbBand.fMask   = /*RBBIM_COLORS | RBBIM_TEXT | RBBIM_BACKGROUND | */
                   RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE /*| RBBIM_SIZE*/;
  //rbBand.fStyle  = /*RBBS_CHILDEDGE |*//* RBBS_BREAK |*/ RBBS_FIXEDSIZE /*| RBBS_GRIPPERALWAYS*/;
  rbBand.fStyle = bIsPrivAppThemed ? (RBBS_FIXEDSIZE | RBBS_CHILDEDGE) : RBBS_FIXEDSIZE;
  rbBand.hbmBack = NULL;
  rbBand.lpText     = L"Toolbar";
  rbBand.hwndChild  = g_hwndToolbar;
  rbBand.cxMinChild = (rc.right - rc.left) * COUNTOF(tbbMainWnd);
  rbBand.cyMinChild = (rc.bottom - rc.top) + 2 * rc.top;
  rbBand.cx         = 0;
  SendMessage(g_hwndReBar,RB_INSERTBAND,(WPARAM)-1,(LPARAM)&rbBand);

  SetWindowPos(g_hwndReBar,NULL,0,0,0,0,SWP_NOZORDER);
  GetWindowRect(g_hwndReBar,&rc);
  cyReBar = rc.bottom - rc.top;

  cyReBarFrame = bIsPrivAppThemed ? 0 : 2;
}


//=============================================================================
//
//  MsgEndSession() - Handle WM_ENDSESSION,WM_DESTROY
//
//
LRESULT MsgEndSession(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);
  UNUSED(lParam);

  static bool bShutdownOK = false;

  if (!bShutdownOK) {

    // Terminate file watching
    InstallFileWatching(NULL);

    // GetWindowPlacement
    g_WinInfo = GetMyWindowPlacement(hwnd, NULL);

    DragAcceptFiles(hwnd, true);
#ifdef _EXTRA_DRAG_N_DROP_HANDLER_
    RevokeDragAndDrop(pDropTarget);
#endif

    // Terminate clipboard watching
    if (g_flagPasteBoard) {
      KillTimer(hwnd, ID_PASTEBOARDTIMER);
      ChangeClipboardChain(hwnd, hwndNextCBChain);
    }

    // Destroy find / replace dialog
    if (IsWindow(g_hwndDlgFindReplace))
      DestroyWindow(g_hwndDlgFindReplace);

    // Destroy customize schemes
    if (IsWindow(g_hwndDlgCustomizeSchemes))
      DestroyWindow(g_hwndDlgCustomizeSchemes);

    // call SaveSettings() when g_hwndToolbar is still valid
    SaveSettings(false);

    if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) != 0) {

      // Cleanup unwanted MRU's
      if (!g_bSaveRecentFiles) {
        MRU_Empty(g_pFileMRU);
        MRU_Save(g_pFileMRU);
      }
      else
        MRU_MergeSave(g_pFileMRU, true, g_flagRelativeFileMRU, g_flagPortableMyDocs);

      MRU_Destroy(g_pFileMRU);

      if (!g_bSaveFindReplace) {
        MRU_Empty(g_pMRUfind);
        MRU_Empty(g_pMRUreplace);
        MRU_Save(g_pMRUfind);
        MRU_Save(g_pMRUreplace);
      }
      else {
        MRU_MergeSave(g_pMRUfind, false, false, false);
        MRU_MergeSave(g_pMRUreplace, false, false, false);
      }
      MRU_Destroy(g_pMRUfind);
      MRU_Destroy(g_pMRUreplace);
    }

    // Remove tray icon if necessary
    ShowNotifyIcon(hwnd, false);

    bShutdownOK = true;
  }

  if (umsg == WM_DESTROY)
    PostQuitMessage(0);

  return 0LL;
}



//=============================================================================
//
// MsgDPIChanged() - Handle WM_DPICHANGED
//
//
LRESULT MsgDPIChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  g_uCurrentDPI = HIWORD(wParam);
  g_uCurrentPPI = GetCurrentPPI(hwnd);

  DocPos const pos = SciCall_GetCurrentPos();

  HINSTANCE hInstance = (HINSTANCE)(INT_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

#if 0
  char buf[128];
  sprintf(buf, "WM_DPICHANGED: dpi=%u, %u\n", g_uCurrentDPI, g_uCurrentPPI);
  SendMessage(g_hwndEdit, SCI_INSERTTEXT, 0, (LPARAM)buf);
#endif

  Style_ResetCurrentLexer(g_hwndEdit);
  SciCall_GotoPos(pos);
  
  // recreate toolbar and statusbar
  Toolbar_GetButtons(g_hwndToolbar, IDT_FILE_NEW, g_tchToolbarButtons, COUNTOF(g_tchToolbarButtons));

  DestroyWindow(g_hwndToolbar);
  DestroyWindow(g_hwndReBar);
  DestroyWindow(g_hwndStatus);
  CreateBars(hwnd, hInstance);

  RECT* const rc = (RECT*)lParam;
  SendWMSize(hwnd, rc);

  UpdateUI();
  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateLineNumberWidth();
  
  return 0LL;
}


//=============================================================================
//
//  MsgThemeChanged() - Handle WM_THEMECHANGED
//
//
LRESULT MsgThemeChanged(HWND hwnd, WPARAM wParam ,LPARAM lParam)
{
  UNUSED(hwnd);
  UNUSED(lParam);
  UNUSED(wParam);
  
  RECT rc, rc2;
  HINSTANCE hInstance = (HINSTANCE)(INT_PTR)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);

  // reinitialize edit frame

  if (PrivateIsAppThemed()) {
    bIsAppThemed = true;

    SetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE,GetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(g_hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);

    if (IsVista()) {
      cxEditFrame = 0;
      cyEditFrame = 0;
    }

    else {
      SetWindowPos(hwndEditFrame,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
      GetClientRect(hwndEditFrame,&rc);
      GetWindowRect(hwndEditFrame,&rc2);

      cxEditFrame = ((rc2.right-rc2.left) - (rc.right-rc.left)) / 2;
      cyEditFrame = ((rc2.bottom-rc2.top) - (rc.bottom-rc.top)) / 2;
    }
  }

  else {
    bIsAppThemed = false;

    SetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE,WS_EX_CLIENTEDGE|GetWindowLongPtr(g_hwndEdit,GWL_EXSTYLE));
    SetWindowPos(g_hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    cxEditFrame = 0;
    cyEditFrame = 0;
  }

  // recreate toolbar and statusbar
  Toolbar_GetButtons(g_hwndToolbar,IDT_FILE_NEW,g_tchToolbarButtons,COUNTOF(g_tchToolbarButtons));

  DestroyWindow(g_hwndToolbar);
  DestroyWindow(g_hwndReBar);
  DestroyWindow(g_hwndStatus);
  CreateBars(hwnd,hInstance);

  SendWMSize(hwnd, NULL);

  EditFinalizeStyling(g_hwndEdit, -1);

  if (EditToggleView(g_hwndEdit, false)) {
    EditToggleView(g_hwndEdit, true);
  }
  MarkAllOccurrences(0, true);
  EditUpdateUrlHotspots(g_hwndEdit, 0, Sci_GetDocEndPosition(), g_bHyperlinkHotspot);

  UpdateUI();
  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateLineNumberWidth();

  return 0LL;
}


//=============================================================================
//
//  MsgSize() - Handles WM_SIZE
//
//
LRESULT MsgSize(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(hwnd);
  UNUSED(lParam);

  if (wParam == SIZE_MINIMIZED) { return 0LL; }

  int x = 0;
  int y = 0;

  int cx = LOWORD(lParam);
  int cy = HIWORD(lParam);

  if (bShowToolbar)
  {
/*  SendMessage(g_hwndToolbar,WM_SIZE,0,0);
    RECT rc;
    GetWindowRect(g_hwndToolbar,&rc);
    y = (rc.bottom - rc.top);
    cy -= (rc.bottom - rc.top);*/

    //SendMessage(g_hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
    SetWindowPos(g_hwndReBar,NULL,0,0,LOWORD(lParam),cyReBar,SWP_NOZORDER);
    // the ReBar automatically sets the correct height
    // calling SetWindowPos() with the height of one toolbar button
    // causes the control not to temporarily use the whole client area
    // and prevents flickering

    //GetWindowRect(g_hwndReBar,&rc);
    y = cyReBar + cyReBarFrame;    // define
    cy -= cyReBar + cyReBarFrame;  // border
  }

  if (bShowStatusbar)
  {
    RECT rc;
    SendMessage(g_hwndStatus,WM_SIZE,0,0);
    GetWindowRect(g_hwndStatus,&rc);
    cy -= (rc.bottom - rc.top);
  }

  HDWP hdwp = BeginDeferWindowPos(2);

  DeferWindowPos(hdwp,hwndEditFrame,NULL,x,y,cx,cy,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  DeferWindowPos(hdwp,g_hwndEdit,NULL,x+cxEditFrame,y+cyEditFrame,
                 cx-2*cxEditFrame,cy-2*cyEditFrame,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  EndDeferWindowPos(hdwp);

  g_WinCurrentWidth = cx;

  UpdateToolbar();
  UpdateStatusbar(false);
  UpdateLineNumberWidth();

  return 0LL;
}


//=============================================================================
//
//  MsgDropFiles() - Handles WM_DROPFILES
//
//
LRESULT MsgDropFiles(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  WCHAR szBuf[MAX_PATH + 40];
  HDROP hDrop = (HDROP)wParam;

  // Reset Change Notify
  //bPendingChangeNotify = false;

  if (IsIconic(hwnd))
    ShowWindow(hwnd, SW_RESTORE);

  //SetForegroundWindow(hwnd);

  DragQueryFile(hDrop, 0, szBuf, COUNTOF(szBuf));

  if (PathIsDirectory(szBuf)) {
    WCHAR tchFile[MAX_PATH] = { L'\0' };
    if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), szBuf))
      FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
  }
  else if (PathFileExists(szBuf)) {
    FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, szBuf);
  }
  else {
#ifndef _EXTRA_DRAG_N_DROP_HANDLER_
    // Windows Bug: wParam (HDROP) pointer is corrupted if dropped from 32-bit App
    MsgBoxLng(MBWARN, IDS_MUI_DROP_NO_FILE);
#endif
    // delegated to SCN_URIDROPPED
  }

  if (DragQueryFile(hDrop, (UINT)(-1), NULL, 0) > 1)
    MsgBoxLng(MBWARN, IDS_MUI_ERR_DROP);

  DragFinish(hDrop);

  return 0LL;
}


//=============================================================================
//
//  DropFilesProc() - Handles DROPFILES
//
//
static DWORD DropFilesProc(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd, DWORD dwKeyState, POINTL pt, void *pUserData)
{
  DWORD dwEffect = DROPEFFECT_NONE;

  //HWND hEditWnd = (HWND)pUserData;
  UNUSED(pUserData);

  if (cf == CF_HDROP)
  {
    WCHAR szBuf[MAX_PATH + 40];
    HDROP hDrop = (HDROP)hData;

    if (IsIconic(hWnd))
      ShowWindow(hWnd, SW_RESTORE);

    DragQueryFile(hDrop, 0, szBuf, COUNTOF(szBuf));

    if (PathIsDirectory(szBuf)) {
      WCHAR tchFile[MAX_PATH] = { L'\0' };
      if (OpenFileDlg(hWnd, tchFile, COUNTOF(tchFile), szBuf))
        FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
    }
    else
      FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, szBuf);

    if (DragQueryFile(hDrop, (UINT)(-1), NULL, 0) > 1)
      MsgBoxLng(MBWARN, IDS_MUI_ERR_DROP);

    dwEffect = DROPEFFECT_COPY;
  }

  UNUSED(dwKeyState);
  UNUSED(pt);

  return dwEffect;
} 


//=============================================================================
//
//  MsgCopyData() - Handles WM_COPYDATA
//
//
LRESULT MsgCopyData(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);

  PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;

  // Reset Change Notify
  //bPendingChangeNotify = false;

  SetDlgItemInt(hwnd, IDC_REUSELOCK, GetTickCount(), false);

  if (pcds->dwData == DATA_NOTEPAD3_PARAMS) 
  {
    LPnp3params params = LocalAlloc(LPTR, pcds->cbData);
    if (params) {
      CopyMemory(params, pcds->lpData, pcds->cbData);

      if (params->flagLexerSpecified)
        g_flagLexerSpecified = 1;

      if (params->flagQuietCreate)
        g_flagQuietCreate = 1;

      if (params->flagFileSpecified) {

        bool bOpened = false;
        Encoding_SrcCmdLn(params->iSrcEncoding);

        if (PathIsDirectory(&params->wchData)) {
          WCHAR tchFile[MAX_PATH] = { L'\0' };
          if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), &params->wchData))
            bOpened = FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
        }

        else
          bOpened = FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, &params->wchData);

        if (bOpened) {

          if (params->flagChangeNotify == 1) {
            g_iFileWatchingMode = 0;
            g_bResetFileWatching = true;
            InstallFileWatching(g_wchCurFile);
          }
          else if (params->flagChangeNotify == 2) {
            if (!g_bChasingDocTail) {
              SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
            }
            else {
              g_iFileWatchingMode = 2;
              g_bResetFileWatching = true;
              InstallFileWatching(g_wchCurFile);
            }
          }

          if (0 != params->flagSetEncoding) {
            g_flagSetEncoding = params->flagSetEncoding;
            SendMessage(
              hwnd,
              WM_COMMAND,
              MAKELONG(IDM_ENCODING_ANSI + g_flagSetEncoding - 1, 1),
              0);
            g_flagSetEncoding = 0;
          }

          if (0 != params->flagSetEOLMode) {
            g_flagSetEOLMode = params->flagSetEOLMode;
            SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_LINEENDINGS_CRLF + g_flagSetEOLMode - 1, 1), 0);
            g_flagSetEOLMode = 0;
          }

          if (params->flagLexerSpecified) {
            if (params->iInitialLexer < 0) {
              WCHAR wchExt[32] = L".";
              StringCchCopyN(CharNext(wchExt), 32, StrEnd(&params->wchData, 0) + 1, 31);
              Style_SetLexerFromName(g_hwndEdit, &params->wchData, wchExt);
            }
            else if (params->iInitialLexer >= 0 && params->iInitialLexer < NUMLEXERS)
              Style_SetLexerFromID(g_hwndEdit, params->iInitialLexer);
          }

          if (params->flagTitleExcerpt) {
            StringCchCopyN(szTitleExcerpt, COUNTOF(szTitleExcerpt), StrEnd(&params->wchData, 0) + 1, COUNTOF(szTitleExcerpt));
          }
        }
        // reset
        Encoding_SrcCmdLn(CPI_NONE);
      }

      if (params->flagJumpTo) {
        if (params->iInitialLine == 0)
          params->iInitialLine = 1;
        EditJumpTo(g_hwndEdit, params->iInitialLine, params->iInitialColumn);
      }

      g_flagLexerSpecified = 0;
      g_flagQuietCreate = 0;

      LocalFree(params);
    }

    UpdateToolbar();
    UpdateStatusbar(false);
    UpdateLineNumberWidth();

  }

  return 0LL;
}

//=============================================================================
//
//  MsgContextMenu() - Handles WM_CONTEXTMENU
//
//
LRESULT MsgContextMenu(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  HMENU hmenu;
  int imenu = 0;
  POINT pt;
  int nID = GetDlgCtrlID((HWND)wParam);

  if ((nID != IDC_EDIT) && (nID != IDC_STATUSBAR) &&
    (nID != IDC_REBAR) && (nID != IDC_TOOLBAR))
    return DefWindowProc(hwnd, umsg, wParam, lParam);

  hmenu = LoadMenu(g_hLngResContainer, MAKEINTRESOURCE(IDR_MUI_POPUPMENU));
  //SetMenuDefaultItem(GetSubMenu(hmenu,1),0,false);

  pt.x = (int)(short)LOWORD(lParam);
  pt.y = (int)(short)HIWORD(lParam);

  switch (nID) {
  case IDC_EDIT:
    {
      if (SciCall_IsSelectionEmpty() && (pt.x != -1) && (pt.y != -1)) {
        POINT ptc;
        ptc.x = pt.x;  ptc.y = pt.y;
        ScreenToClient(g_hwndEdit, &ptc);
        DocPos iNewPos = SciCall_PositionFromPoint(ptc.x, ptc.y);
        EditSelectEx(g_hwndEdit, iNewPos, iNewPos, -1, -1);
      }

      if (pt.x == -1 && pt.y == -1) {
        DocPos iCurrentPos = SciCall_GetCurrentPos();
        pt.x = (LONG)SciCall_PointXFromPosition(iCurrentPos);
        pt.y = (LONG)SciCall_PointYFromPosition(iCurrentPos);
        ClientToScreen(g_hwndEdit, &pt);
      }
      imenu = 0;
    }
    break;

  case IDC_TOOLBAR:
  case IDC_STATUSBAR:
  case IDC_REBAR:
    if (pt.x == -1 && pt.y == -1)
      GetCursorPos(&pt);
    imenu = 1;
    break;
  }

  TrackPopupMenuEx(GetSubMenu(hmenu, imenu),
                   TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x + 1, pt.y + 1, hwnd, NULL);

  DestroyMenu(hmenu);

  return 0LL;
}



//=============================================================================
//
//  MsgChangeNotify() - Handles WM_CHANGENOTIFY
//
//
LRESULT MsgChangeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);
  UNUSED(lParam);

  if (g_iFileWatchingMode == 1 || IsDocumentModified || Encoding_HasChanged(CPI_GET)) {
    SetForegroundWindow(hwnd);
  }

  if (PathFileExists(g_wchCurFile)) 
  {
    if ((g_iFileWatchingMode == 2 && !IsDocumentModified && !Encoding_HasChanged(CPI_GET)) ||
      MsgBoxLng(MBYESNOWARN,IDS_MUI_FILECHANGENOTIFY) == IDYES)
    {
      FileRevert(g_wchCurFile, Encoding_HasChanged(CPI_GET));
      
      if (g_bChasingDocTail) 
      {
        SciCall_SetReadOnly(g_bChasingDocTail);
        //SetForegroundWindow(hwnd);
        SciCall_ScrollToEnd(); 
      }
    }
  }
  else {
    if (MsgBoxLng(MBYESNOWARN, IDS_MUI_FILECHANGENOTIFY2) == IDYES) {
      FileSave(true, false, false, false);
    }
  }

  if (!g_bRunningWatch) {
    InstallFileWatching(g_wchCurFile);
  }

  return 0LL;
}


//=============================================================================
//
//  MsgTrayMessage() - Handles WM_TRAYMESSAGE
//
//
LRESULT MsgTrayMessage(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);

  switch (lParam) 
  {
    case WM_RBUTTONUP:
    {

      HMENU hMenu = LoadMenu(g_hLngResContainer, MAKEINTRESOURCE(IDR_MUI_POPUPMENU));
      HMENU hMenuPopup = GetSubMenu(hMenu, 2);

      POINT pt;
      int iCmd;

      SetForegroundWindow(hwnd);

      GetCursorPos(&pt);
      SetMenuDefaultItem(hMenuPopup, IDM_TRAY_RESTORE, false);
      iCmd = TrackPopupMenu(hMenuPopup,
        TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
        pt.x, pt.y, 0, hwnd, NULL);

      PostMessage(hwnd, WM_NULL, 0, 0);

      DestroyMenu(hMenu);

      if (iCmd == IDM_TRAY_RESTORE) {
        ShowNotifyIcon(hwnd, false);
        RestoreWndFromTray(hwnd);
        ShowOwnedPopups(hwnd, true);
      }
      else if (iCmd == IDM_TRAY_EXIT) {
        //ShowNotifyIcon(hwnd,false);
        PostMessage(hwnd, WM_CLOSE, 0, 0);
      }
    }
    break;

    case WM_LBUTTONUP:
      ShowNotifyIcon(hwnd, false);
      RestoreWndFromTray(hwnd);
      ShowOwnedPopups(hwnd, true);
      break;

    default:
      return 1LL;

  }
  return 0LL;
}


static bool __fastcall _IsInlineIMEActive()
{
  bool result = false;
  if (g_IMEInteraction) {
    HIMC himc = ImmGetContext(g_hwndEdit);
    if (himc) {
      DWORD dwConversion = IME_CMODE_ALPHANUMERIC, dwSentence = 0;
      if (ImmGetConversionStatus(himc, &dwConversion, &dwSentence)) {
        result = !(dwConversion == IME_CMODE_ALPHANUMERIC);
      }
      ImmReleaseContext(g_hwndEdit, himc);
    }
  }
  return result;
}


//=============================================================================
//
//  MsgInitMenu() - Handles WM_INITMENU
//
//
LRESULT MsgInitMenu(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  //DocPos p;
  bool b,e,s;

  HMENU hmenu = (HMENU)wParam;

  bool ro = SciCall_GetReadOnly();

  int i = (int)StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile));
  EnableCmd(hmenu,IDM_FILE_REVERT,i);
  EnableCmd(hmenu, CMD_RELOADASCIIASUTF8, i);
  EnableCmd(hmenu, CMD_RELOADFORCEDETECTION, i);
  EnableCmd(hmenu, CMD_RECODEANSI, i);
  EnableCmd(hmenu, CMD_RECODEOEM, i);
  EnableCmd(hmenu, CMD_RELOADNOFILEVARS, i);
  EnableCmd(hmenu, CMD_RECODEDEFAULT, i);
  EnableCmd(hmenu, IDM_FILE_LAUNCH, i);

  EnableCmd(hmenu,IDM_FILE_LAUNCH,i);
  EnableCmd(hmenu,IDM_FILE_PROPERTIES,i);
  EnableCmd(hmenu,IDM_FILE_CREATELINK,i);
  EnableCmd(hmenu,IDM_FILE_ADDTOFAV,i);

  EnableCmd(hmenu,IDM_FILE_READONLY,i);
  CheckCmd(hmenu,IDM_FILE_READONLY,g_bFileReadOnly);

  EnableCmd(hmenu,IDM_ENCODING_UNICODEREV,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UNICODE,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UTF8SIGN,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UTF8,!ro);
  EnableCmd(hmenu,IDM_ENCODING_ANSI,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_CRLF,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_LF,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_CR,!ro);

  EnableCmd(hmenu,IDM_ENCODING_RECODE,i);

  if (Encoding_IsUNICODE_REVERSE(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_UNICODEREV;
  else if (Encoding_IsUNICODE(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_UNICODE;
  else if (Encoding_IsUTF8_SIGN(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_UTF8SIGN;
  else if (Encoding_IsUTF8(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_UTF8;
  else if (Encoding_IsANSI(Encoding_Current(CPI_GET)))
    i = IDM_ENCODING_ANSI;
  else
    i = -1;
  CheckMenuRadioItem(hmenu,IDM_ENCODING_ANSI,IDM_ENCODING_UTF8SIGN,i,MF_BYCOMMAND);

  if (g_iEOLMode == SC_EOL_CRLF)
    i = IDM_LINEENDINGS_CRLF;
  else if (g_iEOLMode == SC_EOL_LF)
    i = IDM_LINEENDINGS_LF;
  else
    i = IDM_LINEENDINGS_CR;
  CheckMenuRadioItem(hmenu,IDM_LINEENDINGS_CRLF,IDM_LINEENDINGS_CR,i,MF_BYCOMMAND);

  EnableCmd(hmenu,IDM_FILE_RECENT,(MRU_Count(g_pFileMRU) > 0));

  EnableCmd(hmenu,IDM_EDIT_UNDO,SciCall_CanUndo() && !ro);
  EnableCmd(hmenu,IDM_EDIT_REDO,SciCall_CanRedo() && !ro);

  s = SciCall_IsSelectionEmpty();
  e = (SciCall_GetTextLength() == 0);
  b = SciCall_CanPaste();
  
  EnableCmd(hmenu,IDM_EDIT_CUT, !e && !ro);       // allow Ctrl-X w/o selection
  EnableCmd(hmenu,IDM_EDIT_COPY, !e);             // allow Ctrl-C w/o selection

  EnableCmd(hmenu,IDM_EDIT_COPYALL, !e);
  EnableCmd(hmenu,IDM_EDIT_COPYADD, !s);

  EnableCmd(hmenu,IDM_EDIT_PASTE, b && !ro);
  EnableCmd(hmenu,IDM_EDIT_SWAP, (!s || b) && !ro);
  EnableCmd(hmenu,IDM_EDIT_CLEAR, !s && !ro);

  EnableCmd(hmenu, IDM_EDIT_SELECTALL, !e);
  
  OpenClipboard(hwnd);
  EnableCmd(hmenu,IDM_EDIT_CLEARCLIPBOARD,CountClipboardFormats());
  CloseClipboard();

  EnableCmd(hmenu,IDM_EDIT_MOVELINEUP,!ro);
  EnableCmd(hmenu,IDM_EDIT_MOVELINEDOWN,!ro);
  EnableCmd(hmenu,IDM_EDIT_DUPLICATELINE,!ro);
  EnableCmd(hmenu,IDM_EDIT_CUTLINE,!ro);
  EnableCmd(hmenu,IDM_EDIT_COPYLINE,true);
  EnableCmd(hmenu,IDM_EDIT_DELETELINE,!ro);

  EnableCmd(hmenu, IDM_EDIT_MERGEBLANKLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_MERGEEMPTYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEBLANKLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEEMPTYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_REMOVEDUPLICATELINES, !ro);

  EnableCmd(hmenu,IDM_EDIT_INDENT, !s && !ro);
  EnableCmd(hmenu,IDM_EDIT_UNINDENT, !s && !ro);

  EnableCmd(hmenu,IDM_EDIT_PADWITHSPACES,!ro);
  EnableCmd(hmenu,IDM_EDIT_STRIP1STCHAR,!ro);
  EnableCmd(hmenu,IDM_EDIT_STRIPLASTCHAR,!ro);
  EnableCmd(hmenu,IDM_EDIT_TRIMLINES,!ro);
  EnableCmd(hmenu, IDM_EDIT_SELECTIONDUPLICATE, !ro);
  EnableCmd(hmenu, IDM_EDIT_COMPRESS_BLANKS, !ro);

  EnableCmd(hmenu, IDM_EDIT_MODIFYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_ALIGN, !ro);
  EnableCmd(hmenu, IDM_EDIT_SORTLINES,
    (SciCall_LineFromPosition(SciCall_GetSelectionEnd()) -
      SciCall_LineFromPosition(SciCall_GetSelectionStart())) >= 1);
 
  //EnableCmd(hmenu,IDM_EDIT_COLUMNWRAP,i /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_SPLITLINES,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_JOINLINES,!s && !ro);
  EnableCmd(hmenu, IDM_EDIT_JOINLN_NOSP,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_JOINLINES_PARA,!s && !ro);

  EnableCmd(hmenu,IDM_EDIT_CONVERTUPPERCASE,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_CONVERTLOWERCASE,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_INVERTCASE,!s && !ro /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_TITLECASE,!s && !ro /*&& IsWindowsNT()*/);
  EnableCmd(hmenu,IDM_EDIT_SENTENCECASE,!s && !ro /*&& IsWindowsNT()*/);

  EnableCmd(hmenu,IDM_EDIT_CONVERTTABS,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_CONVERTSPACES,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_CONVERTTABS2,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_CONVERTSPACES2,!s && !ro);

  EnableCmd(hmenu,IDM_EDIT_URLENCODE,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_URLDECODE,!s && !ro);

  EnableCmd(hmenu,IDM_EDIT_ESCAPECCHARS,!s && !ro);
  EnableCmd(hmenu,IDM_EDIT_UNESCAPECCHARS,!s && !ro);

  EnableCmd(hmenu,IDM_EDIT_CHAR2HEX, !ro);  // Char2Hex allowed for char after curr pos
  EnableCmd(hmenu,IDM_EDIT_HEX2CHAR, !s && !ro);

  //EnableCmd(hmenu,IDM_EDIT_INCREASENUM,!s && !ro);
  //EnableCmd(hmenu,IDM_EDIT_DECREASENUM,!s && !ro);

  EnableCmd(hmenu,IDM_VIEW_SHOWEXCERPT, !s);

  i = (int)SendMessage(g_hwndEdit,SCI_GETLEXER,0,0);

  EnableCmd(hmenu,IDM_EDIT_LINECOMMENT,
    !(i == SCLEX_NULL || i == SCLEX_CSS || i == SCLEX_DIFF || i == SCLEX_MARKDOWN || i == SCLEX_JSON) && !ro);
  EnableCmd(hmenu,IDM_EDIT_STREAMCOMMENT,
    !(i == SCLEX_NULL || i == SCLEX_VBSCRIPT || i == SCLEX_MAKEFILE || i == SCLEX_VB || i == SCLEX_ASM ||
      i == SCLEX_SQL || i == SCLEX_PERL || i == SCLEX_PYTHON || i == SCLEX_PROPERTIES ||i == SCLEX_CONF ||
      i == SCLEX_POWERSHELL || i == SCLEX_BATCH || i == SCLEX_DIFF || i == SCLEX_BASH || i == SCLEX_TCL ||
      i == SCLEX_AU3 || i == SCLEX_LATEX || i == SCLEX_AHK || i == SCLEX_RUBY || i == SCLEX_CMAKE || i == SCLEX_MARKDOWN ||
      i == SCLEX_YAML || i == SCLEX_REGISTRY || i == SCLEX_NIMROD) && !ro);

  EnableCmd(hmenu, CMD_CTRLENTER, !ro);
  EnableCmd(hmenu, IDM_EDIT_INSERT_TAG, !ro);
  EnableCmd(hmenu, IDM_EDIT_INSERT_ENCODING, (Encoding_GetParseNames(Encoding_Current(CPI_GET)) != NULL) && !ro);

  EnableCmd(hmenu,IDM_EDIT_INSERT_SHORTDATE,!ro);
  EnableCmd(hmenu,IDM_EDIT_INSERT_LONGDATE,!ro);
  EnableCmd(hmenu,IDM_EDIT_INSERT_FILENAME,!ro);
  EnableCmd(hmenu,IDM_EDIT_INSERT_PATHNAME,!ro);

  EnableCmd(hmenu, IDM_EDIT_INSERT_GUID, !ro);

  e = (SciCall_GetTextLength() == 0);

  EnableCmd(hmenu,IDM_EDIT_FIND, !e);
  EnableCmd(hmenu,IDM_EDIT_SAVEFIND, !e);
  EnableCmd(hmenu,IDM_EDIT_FINDNEXT, !e);
  EnableCmd(hmenu,IDM_EDIT_FINDPREV, !e);
  EnableCmd(hmenu,IDM_EDIT_REPLACE, !e && !ro);
  EnableCmd(hmenu,IDM_EDIT_REPLACENEXT, !e && !ro);
  EnableCmd(hmenu,IDM_EDIT_SELTONEXT, !e);
  EnableCmd(hmenu,IDM_EDIT_SELTOPREV, !e);
  EnableCmd(hmenu,IDM_EDIT_FINDMATCHINGBRACE, !e);
  EnableCmd(hmenu,IDM_EDIT_SELTOMATCHINGBRACE, !e);

  EnableCmd(hmenu,BME_EDIT_BOOKMARKPREV, !e);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKNEXT, !e);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKTOGGLE, !e);
  EnableCmd(hmenu,BME_EDIT_BOOKMARKCLEAR, !e);

  EnableCmd(hmenu, IDM_EDIT_DELETELINELEFT, !e && !ro);
  EnableCmd(hmenu, IDM_EDIT_DELETELINERIGHT, !e && !ro);
  EnableCmd(hmenu, CMD_CTRLBACK, !e && !ro);
  EnableCmd(hmenu, CMD_CTRLDEL, !e && !ro);
  EnableCmd(hmenu, CMD_TIMESTAMPS, !e && !ro);

  EnableCmd(hmenu, IDM_VIEW_FONT, !IsWindow(g_hwndDlgCustomizeSchemes));
  EnableCmd(hmenu, IDM_VIEW_CURRENTSCHEME, !IsWindow(g_hwndDlgCustomizeSchemes));

  EnableCmd(hmenu,IDM_VIEW_TOGGLEFOLDS,!e && (g_bCodeFoldingAvailable && g_bShowCodeFolding));
  CheckCmd(hmenu,IDM_VIEW_FOLDING, (g_bCodeFoldingAvailable && g_bShowCodeFolding));
  EnableCmd(hmenu, IDM_VIEW_FOLDING, g_bCodeFoldingAvailable);

  CheckCmd(hmenu,IDM_VIEW_USE2NDDEFAULT,Style_GetUse2ndDefault());

  CheckCmd(hmenu,IDM_VIEW_WORDWRAP,g_bWordWrap);
  CheckCmd(hmenu,IDM_VIEW_LONGLINEMARKER,g_bMarkLongLines);
  CheckCmd(hmenu,IDM_VIEW_TABSASSPACES,g_bTabsAsSpaces);
  CheckCmd(hmenu,IDM_VIEW_SHOWINDENTGUIDES,bShowIndentGuides);
  CheckCmd(hmenu,IDM_VIEW_AUTOINDENTTEXT,bAutoIndent);
  CheckCmd(hmenu,IDM_VIEW_LINENUMBERS,g_bShowLineNumbers);
  CheckCmd(hmenu,IDM_VIEW_MARGIN,g_bShowSelectionMargin);
  CheckCmd(hmenu,IDM_VIEW_CHASING_DOCTAIL, g_bChasingDocTail);

  EnableCmd(hmenu,IDM_EDIT_COMPLETEWORD,!e && !ro);
  CheckCmd(hmenu,IDM_VIEW_AUTOCOMPLETEWORDS,g_bAutoCompleteWords && !ro);
  CheckCmd(hmenu,IDM_VIEW_ACCELWORDNAV,g_bAccelWordNavigation);

  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_ONOFF, (g_iMarkOccurrences > 0));
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, g_bMarkOccurrencesMatchVisible);
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, g_bMarkOccurrencesMatchCase);

  EnableCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, (g_iMarkOccurrences > 0) && !g_bMarkOccurrencesMatchVisible);
  CheckCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, EditToggleView(g_hwndEdit, false));

  if (g_bMarkOccurrencesMatchWords)
    i = IDM_VIEW_MARKOCCUR_WORD;
  else if (g_bMarkOccurrencesCurrentWord)
    i = IDM_VIEW_MARKOCCUR_CURRENT;
  else
    i = IDM_VIEW_MARKOCCUR_WNONE;

  CheckMenuRadioItem(hmenu, IDM_VIEW_MARKOCCUR_WNONE, IDM_VIEW_MARKOCCUR_CURRENT, i, MF_BYCOMMAND);
  CheckCmdPos(GetSubMenu(GetSubMenu(GetMenu(g_hwndMain), 2), 17), 5, (i != IDM_VIEW_MARKOCCUR_WNONE));

  i = (int)(g_iMarkOccurrences > 0);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_WNONE, i);
  EnableCmd(hmenu,IDM_VIEW_MARKOCCUR_WORD, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CURRENT, i);
  EnableCmdPos(GetSubMenu(GetSubMenu(GetMenu(g_hwndMain), 2), 17), 5, i);


  CheckCmd(hmenu,IDM_VIEW_SHOWBLANKS,bViewWhiteSpace);
  CheckCmd(hmenu,IDM_VIEW_SHOWEOLS,bViewEOLs);
  CheckCmd(hmenu,IDM_VIEW_WORDWRAPSYMBOLS,bShowWordWrapSymbols);
  CheckCmd(hmenu,IDM_VIEW_MATCHBRACES,bMatchBraces);
  CheckCmd(hmenu,IDM_VIEW_TOOLBAR,bShowToolbar);
  EnableCmd(hmenu,IDM_VIEW_CUSTOMIZETB,bShowToolbar);
  CheckCmd(hmenu,IDM_VIEW_STATUSBAR,bShowStatusbar);

  i = (int)SendMessage(g_hwndEdit,SCI_GETLEXER,0,0);
  //EnableCmd(hmenu,IDM_VIEW_AUTOCLOSETAGS,(i == SCLEX_HTML || i == SCLEX_XML));
  CheckCmd(hmenu, IDM_VIEW_AUTOCLOSETAGS, bAutoCloseTags /*&& (i == SCLEX_HTML || i == SCLEX_XML)*/);
  CheckCmd(hmenu, IDM_VIEW_HILITECURRENTLINE, bHiliteCurrentLine);
  CheckCmd(hmenu, IDM_VIEW_HYPERLINKHOTSPOTS, g_bHyperlinkHotspot);
  CheckCmd(hmenu, IDM_VIEW_SCROLLPASTEOF, bScrollPastEOF);
 

  i = IniGetInt(L"Settings2",L"ReuseWindow",0);
  CheckCmd(hmenu,IDM_VIEW_REUSEWINDOW,i);
  i = IniGetInt(L"Settings2",L"SingleFileInstance",0);
  CheckCmd(hmenu,IDM_VIEW_SINGLEFILEINSTANCE,i);
  g_bStickyWinPos = IniGetBool(L"Settings2",L"StickyWindowPosition",false);
  CheckCmd(hmenu,IDM_VIEW_STICKYWINPOS,g_bStickyWinPos);
  CheckCmd(hmenu,IDM_VIEW_ALWAYSONTOP,((bAlwaysOnTop || g_flagAlwaysOnTop == 2) && g_flagAlwaysOnTop != 1));
  CheckCmd(hmenu,IDM_VIEW_MINTOTRAY,bMinimizeToTray);
  CheckCmd(hmenu,IDM_VIEW_TRANSPARENT,bTransparentMode);

  i = IDM_SET_RENDER_TECH_DEFAULT + g_iRenderingTechnology;
  CheckMenuRadioItem(hmenu, IDM_SET_RENDER_TECH_DEFAULT, IDM_SET_RENDER_TECH_D2DDC, i, MF_BYCOMMAND);
  
  if (g_iRenderingTechnology > 0) {
    i = IDM_SET_BIDIRECTIONAL_NONE + g_iBidirectional;
    CheckMenuRadioItem(hmenu, IDM_SET_BIDIRECTIONAL_NONE, IDM_SET_BIDIRECTIONAL_R2L, i, MF_BYCOMMAND);
  }
  else {
    i = IDM_SET_BIDIRECTIONAL_NONE;
    CheckMenuRadioItem(hmenu, IDM_SET_BIDIRECTIONAL_NONE, IDM_SET_BIDIRECTIONAL_R2L, i, MF_BYCOMMAND);
  }
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_NONE, (g_iRenderingTechnology > 0));
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_L2R, (g_iRenderingTechnology > 0));
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_R2L, (g_iRenderingTechnology > 0));

  CheckCmd(hmenu,IDM_VIEW_NOSAVERECENT,g_bSaveRecentFiles);
  CheckCmd(hmenu,IDM_VIEW_NOPRESERVECARET, g_bPreserveCaretPos);
  CheckCmd(hmenu,IDM_VIEW_NOSAVEFINDREPL,g_bSaveFindReplace);
  CheckCmd(hmenu,IDM_VIEW_SAVEBEFORERUNNINGTOOLS,bSaveBeforeRunningTools);

  CheckCmd(hmenu,IDM_VIEW_CHANGENOTIFY,g_iFileWatchingMode);

  if (StringCchLenW(szTitleExcerpt,COUNTOF(szTitleExcerpt)))
    i = IDM_VIEW_SHOWEXCERPT;
  else if (iPathNameFormat == 0)
    i = IDM_VIEW_SHOWFILENAMEONLY;
  else if (iPathNameFormat == 1)
    i = IDM_VIEW_SHOWFILENAMEFIRST;
  else
    i = IDM_VIEW_SHOWFULLPATH;
  CheckMenuRadioItem(hmenu,IDM_VIEW_SHOWFILENAMEONLY,IDM_VIEW_SHOWEXCERPT,i,MF_BYCOMMAND);

  if (iEscFunction == 1)
    i = IDM_VIEW_ESCMINIMIZE;
  else if (iEscFunction == 2)
    i = IDM_VIEW_ESCEXIT;
  else
    i = IDM_VIEW_NOESCFUNC;
  CheckMenuRadioItem(hmenu,IDM_VIEW_NOESCFUNC,IDM_VIEW_ESCEXIT,i,MF_BYCOMMAND);

  i = (int)StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile));
  CheckCmd(hmenu,IDM_VIEW_SAVESETTINGS,g_bSaveSettings && i);

  EnableCmd(hmenu,IDM_VIEW_REUSEWINDOW,i);
  EnableCmd(hmenu,IDM_VIEW_STICKYWINPOS,i);
  EnableCmd(hmenu,IDM_VIEW_SINGLEFILEINSTANCE,i);
  EnableCmd(hmenu,IDM_VIEW_NOSAVERECENT,i);
  EnableCmd(hmenu,IDM_VIEW_NOPRESERVECARET,i);
  EnableCmd(hmenu,IDM_VIEW_NOSAVEFINDREPL,i);
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGS,g_bEnableSaveSettings && i);

  CheckCmd(hmenu, IDM_VIEW_TOGGLETB, (iHighDpiToolBar > 0));
  EnableCmd(hmenu, IDM_VIEW_TOGGLETB, !g_bExternalBitmap);

  i = (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) > 0 || StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0);
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGSNOW,g_bEnableSaveSettings && i);

  bool bIsHLink = false;
  if ((bool)SendMessage(g_hwndEdit, SCI_STYLEGETHOTSPOT, Style_GetHotspotStyleID(), 0))
  {
    bIsHLink = (SciCall_GetStyleAt(SciCall_GetCurrentPos()) == (char)Style_GetHotspotStyleID());
  }
  EnableCmd(hmenu, CMD_OPEN_HYPERLINK, bIsHLink);

  i = (int)StringCchLenW(g_tchAdministrationExe, COUNTOF(g_tchAdministrationExe));
  EnableCmd(hmenu, IDM_HELP_ADMINEXE, i);

  return 0LL;
}


//=============================================================================
//
//  MsgCommand() - Handles WM_COMMAND
//
//
LRESULT MsgCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  WCHAR tchMaxPathBuffer[MAX_PATH+4] = { L'\0' };

  switch(LOWORD(wParam))
  {
    case SCEN_CHANGE:
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, false);
      break;

    case IDT_TIMER_UPDATE_STATUSBAR:
      _UpdateStatusbarDelayed((bool)lParam);
      break;

    case IDT_TIMER_UPDATE_TOOLBAR:
      _UpdateToolbarDelayed();
      break;

    case IDT_TIMER_MAIN_MRKALL:
      EditMarkAllOccurrences(g_hwndEdit, (bool)lParam);
      break;

    case IDT_TIMER_UPDATE_HOTSPOT:
      EditUpdateVisibleUrlHotspot(g_bHyperlinkHotspot);
      break;

    case IDM_FILE_NEW:
      FileLoad(false,true,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,L"");
      break;


    case IDM_FILE_OPEN:
      FileLoad(false,false,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,L"");
      break;


    case IDM_FILE_REVERT:
      if ((IsDocumentModified || Encoding_HasChanged(CPI_GET)) && MsgBoxLng(MBYESNO,IDS_MUI_ASK_REVERT) != IDYES) {
        break;
      }
      FileRevert(g_wchCurFile, Encoding_HasChanged(CPI_GET));
      break;


    case IDM_FILE_SAVE:
      FileSave(true,false,false,false);
      break;


    case IDM_FILE_SAVEAS:
      FileSave(true,false,true,false);
      break;


    case IDM_FILE_SAVECOPY:
      FileSave(true,false,true,true);
      break;


    case IDM_FILE_READONLY:
      if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
      {
        DWORD dwFileAttributes = GetFileAttributes(g_wchCurFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES) {
          if (g_bFileReadOnly)
            dwFileAttributes = (dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
          else
            dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
          if (!SetFileAttributes(g_wchCurFile,dwFileAttributes))
            MsgBoxLng(MBWARN,IDS_MUI_READONLY_MODIFY,g_wchCurFile);
        }
        else
          MsgBoxLng(MBWARN,IDS_MUI_READONLY_MODIFY,g_wchCurFile);

        dwFileAttributes = GetFileAttributes(g_wchCurFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
          g_bFileReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);

        UpdateToolbar();
      }
      break;


    case IDM_FILE_BROWSE:
        DialogFileBrowse(hwnd);
      break;


    case IDM_FILE_NEWWINDOW:
    case IDM_FILE_NEWWINDOW2:
      //~SaveSettings(false); 
      DialogNewWindow(hwnd, bSaveBeforeRunningTools, (LOWORD(wParam) != IDM_FILE_NEWWINDOW2));
      break;


    case IDM_FILE_LAUNCH:
      {
        if (!StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
          break;

        if (bSaveBeforeRunningTools && !FileSave(false,true,false,false))
          break;

        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),g_wchCurFile);
          PathRemoveFileSpec(tchMaxPathBuffer);
        }

        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = g_wchCurFile;
        sei.lpParameters = NULL;
        sei.lpDirectory = tchMaxPathBuffer;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RUN:
      {
        if (bSaveBeforeRunningTools && !FileSave(false, true, false, false)) {
          break;
        }
        StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),g_wchCurFile);
        PathQuoteSpaces(tchMaxPathBuffer);

        RunDlg(hwnd,tchMaxPathBuffer);
      }
      break;


    case IDM_FILE_OPENWITH:
      if (bSaveBeforeRunningTools && !FileSave(false,true,false,false))
        break;
      OpenWithDlg(hwnd,g_wchCurFile);
      break;


    case IDM_FILE_PAGESETUP:
      EditPrintSetup(g_hwndEdit);
      break;

    case IDM_FILE_PRINT:
      {
        SHFILEINFO shfi;
        ZeroMemory(&shfi, sizeof(SHFILEINFO));

        WCHAR *pszTitle;
        WCHAR tchUntitled[32] = { L'\0' };
        WCHAR tchPageFmt[32] = { L'\0' };

        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          SHGetFileInfo2(g_wchCurFile,FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
          pszTitle = shfi.szDisplayName;
        }
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszTitle = tchUntitled;
        }

        GetLngString(IDS_MUI_PRINT_PAGENUM,tchPageFmt,COUNTOF(tchPageFmt));

        if (!EditPrint(g_hwndEdit,pszTitle,tchPageFmt))
          MsgBoxLng(MBWARN,IDS_MUI_PRINT_ERROR,pszTitle);
      }
      break;


    case IDM_FILE_PROPERTIES:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0)
          break;

        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        sei.hwnd = hwnd;
        sei.lpVerb = L"properties";
        sei.lpFile = g_wchCurFile;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
      }
      break;

    case IDM_FILE_CREATELINK:
      {
        if (!StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
          break;

        if (!PathCreateDeskLnk(g_wchCurFile))
          MsgBoxLng(MBWARN,IDS_MUI_ERR_CREATELINK);
      }
      break;


    case IDM_FILE_OPENFAV:
      if (FileSave(false,true,false,false)) {
        if (FavoritesDlg(hwnd,tchMaxPathBuffer))
        {
          if (PathIsLnkToDirectory(tchMaxPathBuffer,NULL,0))
            PathGetLnkPath(tchMaxPathBuffer,tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer));

          if (PathIsDirectory(tchMaxPathBuffer))
          {
            if (OpenFileDlg(g_hwndMain, tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),tchMaxPathBuffer))
              FileLoad(true,false,false,bSkipUnicodeDetection,bSkipANSICodePageDetection, tchMaxPathBuffer);
          }
          else
            FileLoad(true,false,false,bSkipUnicodeDetection,bSkipANSICodePageDetection,tchMaxPathBuffer);
          }
        }
      break;


    case IDM_FILE_ADDTOFAV:
      if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
        SHFILEINFO shfi;
        SHGetFileInfo2(g_wchCurFile,FILE_ATTRIBUTE_NORMAL,
          &shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
        AddToFavDlg(hwnd,shfi.szDisplayName,g_wchCurFile);
      }
      break;


    case IDM_FILE_MANAGEFAV:
      {
        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = g_tchFavoritesDir;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;
        // Run favorites directory
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RECENT:
      if (MRU_Count(g_pFileMRU) > 0) {
        if (FileSave(false,true,false,false)) {
          WCHAR tchFile[MAX_PATH] = { L'\0' };
          if (FileMRUDlg(hwnd,tchFile))
            FileLoad(true,false,false,false,true,tchFile);
          }
        }
      break;


    case IDM_FILE_EXIT:
      PostMessage(hwnd,WM_CLOSE,0,0);
      break;


    case IDM_ENCODING_ANSI:
    case IDM_ENCODING_UNICODE:
    case IDM_ENCODING_UNICODEREV:
    case IDM_ENCODING_UTF8:
    case IDM_ENCODING_UTF8SIGN:
    case IDM_ENCODING_SELECT:
      {
        int iNewEncoding = Encoding_Current(CPI_GET);
        if (LOWORD(wParam) == IDM_ENCODING_SELECT && !SelectEncodingDlg(hwnd,&iNewEncoding))
          break;
        else {
          switch (LOWORD(wParam)) {
            case IDM_ENCODING_UNICODE:    iNewEncoding = CPI_UNICODEBOM; break;
            case IDM_ENCODING_UNICODEREV: iNewEncoding = CPI_UNICODEBEBOM; break;
            case IDM_ENCODING_UTF8:       iNewEncoding = CPI_UTF8; break;
            case IDM_ENCODING_UTF8SIGN:   iNewEncoding = CPI_UTF8SIGN; break;
            case IDM_ENCODING_ANSI:       iNewEncoding = CPI_ANSI_DEFAULT; break;
          }
        }

        BeginWaitCursor(NULL);
        _IGNORE_NOTIFY_CHANGE_;
        if (EditSetNewEncoding(g_hwndEdit, iNewEncoding, g_flagSetEncoding,
                               StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0)) {

          if (SendMessage(g_hwndEdit,SCI_GETLENGTH,0,0) == 0) {
            Encoding_Current(iNewEncoding);
            Encoding_HasChanged(iNewEncoding);
          }
          else {
            if (Encoding_IsANSI(Encoding_Current(CPI_GET)) || Encoding_IsANSI(iNewEncoding))
              Encoding_HasChanged(CPI_NONE);
            Encoding_Current(iNewEncoding);
          }
        }
        _OBSERVE_NOTIFY_CHANGE_;
        EndWaitCursor();
        UpdateStatusbar(false);
      }
      break;


    case IDM_ENCODING_RECODE:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) 
        {
          int iNewEncoding = Encoding_MapUnicode(Encoding_Current(CPI_GET));

          if ((IsDocumentModified || Encoding_HasChanged(CPI_GET)) && MsgBoxLng(MBYESNO, IDS_MUI_ASK_RECODE) != IDYES)
            break;

          if (RecodeDlg(hwnd,&iNewEncoding)) 
          {
            StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),g_wchCurFile);
            Encoding_SrcCmdLn(iNewEncoding);
            FileLoad(true,false,true,false,true,tchMaxPathBuffer);
          }
        }
      }
      break;


    case IDM_ENCODING_SETDEFAULT:
      SelectDefEncodingDlg(hwnd,&g_iDefaultNewFileEncoding);
      break;


    case IDM_LINEENDINGS_CRLF:
    case IDM_LINEENDINGS_LF:
    case IDM_LINEENDINGS_CR:
      {
        BeginWaitCursor(NULL);
        _IGNORE_NOTIFY_CHANGE_;
        int iNewEOLMode = iLineEndings[LOWORD(wParam)-IDM_LINEENDINGS_CRLF];
        g_iEOLMode = iNewEOLMode;
        SendMessage(g_hwndEdit,SCI_SETEOLMODE,g_iEOLMode,0);
        SendMessage(g_hwndEdit,SCI_CONVERTEOLS,g_iEOLMode,0);
        EditFixPositions(g_hwndEdit);
        _OBSERVE_NOTIFY_CHANGE_;
        EndWaitCursor();
        UpdateStatusbar(false);
      }
      break;


    case IDM_LINEENDINGS_SETDEFAULT:
      SelectDefLineEndingDlg(hwnd,&g_iDefaultEOLMode);
      break;


    case IDM_EDIT_UNDO:
      if (SciCall_CanUndo()) {
        _IGNORE_NOTIFY_CHANGE_;
        SciCall_Undo();
        _OBSERVE_NOTIFY_CHANGE_;
        UpdateToolbar();
      }
      break;

    case IDM_EDIT_REDO:
      if (SciCall_CanRedo()) {
        _IGNORE_NOTIFY_CHANGE_;
        SciCall_Redo();
        _OBSERVE_NOTIFY_CHANGE_;
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_CUT:
      {
        if (g_flagPasteBoard)
          bLastCopyFromMe = true;

        _BEGIN_UNDO_ACTION_;
        if (!SciCall_IsSelectionEmpty())
        {
          SciCall_Cut();
        }
        else { // VisualStudio behavior
          SciCall_CopyAllowLine();
          SciCall_LineDelete();
        }
        _END_UNDO_ACTION_;
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_COPY:
    case IDM_EDIT_COPYLINE:
      if (g_flagPasteBoard)
        bLastCopyFromMe = true;
      SciCall_CopyAllowLine();
      UpdateToolbar();
      break;


    case IDM_EDIT_COPYALL:
      {
        if (g_flagPasteBoard)
          bLastCopyFromMe = true;
        SciCall_CopyRange(0, Sci_GetDocEndPosition());
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_COPYADD:
      {
        if (g_flagPasteBoard)
          bLastCopyFromMe = true;
        EditCopyAppend(g_hwndEdit,true);
        UpdateToolbar();
      }
      break;

    case IDM_EDIT_PASTE:
      {
        if (g_flagPasteBoard)
          bLastCopyFromMe = true;
        _BEGIN_UNDO_ACTION_;
        _IGNORE_NOTIFY_CHANGE_;
        SciCall_Paste();
        _OBSERVE_NOTIFY_CHANGE_;
        _END_UNDO_ACTION_;
        UpdateToolbar();
        UpdateStatusbar(false);
        UpdateLineNumberWidth();
      }
      break;

    case IDM_EDIT_SWAP:
      {
        if (g_flagPasteBoard)
          bLastCopyFromMe = true;
        _BEGIN_UNDO_ACTION_;
        _IGNORE_NOTIFY_CHANGE_;
        EditSwapClipboard(g_hwndEdit, bSkipUnicodeDetection);
        _OBSERVE_NOTIFY_CHANGE_;
        _END_UNDO_ACTION_;
        UpdateToolbar();
        UpdateStatusbar(false);
        UpdateLineNumberWidth();
      }
      break;

    case IDM_EDIT_CLEARCLIPBOARD:
      EditClearClipboard(g_hwndEdit);
      UpdateToolbar();
      break;


    case IDM_EDIT_SELECTALL:
        SendMessage(g_hwndEdit,SCI_SELECTALL,0,0);
        UpdateToolbar();
        UpdateStatusbar(false);
      break;


    case IDM_EDIT_SELECTWORD:
      {
        DocPos iPos = SciCall_GetCurrentPos();

        if (SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0)) {

          DocPos iWordStart = (DocPos)SendMessage(g_hwndEdit,SCI_WORDSTARTPOSITION,iPos,true);
          DocPos iWordEnd   = (DocPos)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iPos,true);

          if (iWordStart == iWordEnd) // we are in whitespace salad...
          {
            iWordStart = (DocPos)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iPos,false);
            iWordEnd   = (DocPos)SendMessage(g_hwndEdit,SCI_WORDENDPOSITION,iWordStart,true);
            if (iWordStart != iWordEnd) {
              SciCall_SetSel(iWordStart, iWordEnd);
            }
          }
          else {
            SciCall_SetSel(iWordStart, iWordEnd);
          }

          if (SciCall_IsSelectionEmpty()) {
            const DocLn iLine = SciCall_LineFromPosition(iPos);
            const DocPos iLineStart = SciCall_GetLineIndentPosition(iLine);
            const DocPos iLineEnd = SciCall_GetLineEndPosition(iLine);
            SciCall_SetSel(iLineStart, iLineEnd);
          }
        }
        else {
          const DocLn iLine = SciCall_LineFromPosition(iPos);
          const DocPos iLineStart = SciCall_GetLineIndentPosition(iLine);
          const DocPos iLineEnd = SciCall_GetLineEndPosition(iLine);
          SciCall_SetSel(iLineStart, iLineEnd);
        }
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_SELECTLINE:
      {
        const DocPos iSelStart = SciCall_GetSelectionStart();
        const DocPos iSelEnd = SciCall_GetSelectionEnd();
        const DocPos iLineStart = SciCall_LineFromPosition(iSelStart);
        const DocPos iLineEnd = SciCall_LineFromPosition(iSelEnd);
        SciCall_SetSel(SciCall_PositionFromLine(iLineStart), SciCall_PositionFromLine(iLineEnd + 1));
        SciCall_ChooseCaretX();
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_MOVELINEUP:
      {
        _BEGIN_UNDO_ACTION_;
        EditMoveUp(g_hwndEdit);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_MOVELINEDOWN:
      {
        _BEGIN_UNDO_ACTION_;
        EditMoveDown(g_hwndEdit);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_DUPLICATELINE:
      SendMessage(g_hwndEdit,SCI_LINEDUPLICATE,0,0);
      break;


    case IDM_EDIT_CUTLINE:
      {
        if (g_flagPasteBoard)
          bLastCopyFromMe = true;
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit,SCI_LINECUT,0,0);
        _END_UNDO_ACTION_;
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_DELETELINE:
      {
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit, SCI_LINEDELETE, 0, 0);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_DELETELINELEFT:
      {
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit, SCI_DELLINELEFT, 0, 0);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_DELETELINERIGHT:
      {
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit, SCI_DELLINERIGHT, 0, 0);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_INDENT:
      {
        _BEGIN_UNDO_ACTION_;
        EditIndentBlock(g_hwndEdit, SCI_TAB, true);
        _END_UNDO_ACTION_;
      }
      break;

    case IDM_EDIT_UNINDENT:
      {
        _BEGIN_UNDO_ACTION_;
        EditIndentBlock(g_hwndEdit, SCI_BACKTAB, true);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_TAB:
      {
        _BEGIN_UNDO_ACTION_;
        EditIndentBlock(g_hwndEdit, SCI_TAB, false);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_BACKTAB:
      {
        _BEGIN_UNDO_ACTION_;
        EditIndentBlock(g_hwndEdit, SCI_BACKTAB, false);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_CTRLTAB:
      {
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit, SCI_SETUSETABS, true, 0);
        SendMessage(g_hwndEdit, SCI_SETTABINDENTS, false, 0);
        EditIndentBlock(g_hwndEdit, SCI_TAB, false);
        SendMessage(g_hwndEdit, SCI_SETTABINDENTS, g_bTabIndents, 0);
        SendMessage(g_hwndEdit, SCI_SETUSETABS, !g_bTabsAsSpaces, 0);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_DELETEBACK:
      {
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit, SCI_DELETEBACK, 0, 0);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_VK_INSERT:
      SendMessage(g_hwndEdit, SCI_EDITTOGGLEOVERTYPE, 0, 0);
      UpdateStatusbar(false);
      break;

    case IDM_EDIT_ENCLOSESELECTION:
      if (EditEncloseSelectionDlg(hwnd,wchPrefixSelection,wchAppendSelection)) {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(g_hwndEdit,wchPrefixSelection,wchAppendSelection);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_SELECTIONDUPLICATE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit,SCI_SELECTIONDUPLICATE,0,0);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_PADWITHSPACES:
      {
        BeginWaitCursor(NULL);
        EditPadWithSpaces(g_hwndEdit,false,false);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STRIP1STCHAR:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditStripFirstCharacter(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STRIPLASTCHAR:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditStripLastCharacter(g_hwndEdit, false, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_TRIMLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditStripLastCharacter(g_hwndEdit, false, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_COMPRESS_BLANKS:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditCompressBlanks(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_MERGEBLANKLINES:
    {
      BeginWaitCursor(NULL);
      _BEGIN_UNDO_ACTION_;
      EditRemoveBlankLines(g_hwndEdit, true, true);
      _END_UNDO_ACTION_;
      EndWaitCursor();
    }
    break;

    case IDM_EDIT_MERGEEMPTYLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditRemoveBlankLines(g_hwndEdit, true, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEBLANKLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditRemoveBlankLines(g_hwndEdit, false, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEEMPTYLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditRemoveBlankLines(g_hwndEdit, false, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEDUPLICATELINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditRemoveDuplicateLines(g_hwndEdit, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_MODIFYLINES:
      {
        if (EditModifyLinesDlg(hwnd,wchPrefixLines,wchAppendLines)) {
          BeginWaitCursor(NULL);
          _BEGIN_UNDO_ACTION_;
          EditModifyLines(g_hwndEdit,wchPrefixLines,wchAppendLines);
          _END_UNDO_ACTION_;
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_ALIGN:
      {
        if (EditAlignDlg(hwnd,&iAlignMode)) {
          BeginWaitCursor(NULL);
          _BEGIN_UNDO_ACTION_;
          EditAlignText(g_hwndEdit,iAlignMode);
          _END_UNDO_ACTION_;
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_SORTLINES:
      {
        if (EditSortDlg(hwnd,&iSortOptions)) {
          BeginWaitCursor(NULL);
          _BEGIN_UNDO_ACTION_;
          EditSortLines(g_hwndEdit,iSortOptions);
          _END_UNDO_ACTION_;
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_COLUMNWRAP:
      {
        if (iWrapCol == 0) {
          iWrapCol = g_iLongLinesLimit;
        }

        UINT uWrpCol = 0;
        if (ColumnWrapDlg(hwnd,IDD_MUI_COLUMNWRAP,&uWrpCol))
        {
          iWrapCol = (DocPos)clampi((int)uWrpCol, 1, g_iLongLinesLimit);
          BeginWaitCursor(NULL);
          _BEGIN_UNDO_ACTION_;
          EditWrapToColumn(g_hwndEdit,iWrapCol);
          _END_UNDO_ACTION_;
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_SPLITLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditSplitLines(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_JOINLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditJoinLinesEx(g_hwndEdit, false, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;

    case IDM_EDIT_JOINLN_NOSP:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditJoinLinesEx(g_hwndEdit, false, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;

    case IDM_EDIT_JOINLINES_PARA:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditJoinLinesEx(g_hwndEdit, true, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTUPPERCASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit,SCI_UPPERCASE,0,0);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTLOWERCASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit,SCI_LOWERCASE,0,0);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_INVERTCASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditInvertCase(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_TITLECASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditTitleCase(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_SENTENCECASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditSentenceCase(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTTABS:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditTabsToSpaces(g_hwndEdit, g_iTabWidth, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTSPACES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditSpacesToTabs(g_hwndEdit, g_iTabWidth, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTTABS2:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditTabsToSpaces(g_hwndEdit, g_iTabWidth, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTSPACES2:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditSpacesToTabs(g_hwndEdit, g_iTabWidth, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_INSERT_TAG:
      {
        WCHAR wszOpen[256] = { L'\0' };
        WCHAR wszClose[256] = { L'\0' };
        if (EditInsertTagDlg(hwnd, wszOpen, wszClose)) {
          _BEGIN_UNDO_ACTION_;
          EditEncloseSelection(g_hwndEdit, wszOpen, wszClose);
          _END_UNDO_ACTION_;
        }
      }
      break;


    case IDM_EDIT_INSERT_ENCODING:
      {
        if (*Encoding_GetParseNames(Encoding_Current(CPI_GET))) {
          char msz[32] = { '\0' };
          //int iSelStart;
          StringCchCopyNA(msz,COUNTOF(msz), Encoding_GetParseNames(Encoding_Current(CPI_GET)),COUNTOF(msz));
          char *p = StrChrA(msz, ',');
          if (p)
            *p = 0;
          _BEGIN_UNDO_ACTION_;
          SendMessage(g_hwndEdit,SCI_REPLACESEL,0,(LPARAM)msz);
          _END_UNDO_ACTION_;
        }
      }
      break;


    case IDM_EDIT_INSERT_SHORTDATE:
    case IDM_EDIT_INSERT_LONGDATE:
      {
        WCHAR tchDate[64] = { L'\0' };
        WCHAR tchTime[64] = { L'\0' };
        WCHAR tchDateTime[128] = { L'\0' };
        WCHAR tchTemplate[128] = { L'\0' };
        SYSTEMTIME st;
        //int   iSelStart;

        GetLocalTime(&st);

        if (IniGetString(L"Settings2",
              (LOWORD(wParam) == IDM_EDIT_INSERT_SHORTDATE) ? L"DateTimeShort" : L"DateTimeLong",
              L"",tchTemplate,COUNTOF(tchTemplate))) {
          struct tm sst;
          sst.tm_isdst       = -1;
          sst.tm_sec         = (int)st.wSecond;
          sst.tm_min         = (int)st.wMinute;
          sst.tm_hour        = (int)st.wHour;
          sst.tm_mday        = (int)st.wDay;
          sst.tm_mon         = (int)st.wMonth - 1;
          sst.tm_year        = (int)st.wYear - 1900;
          sst.tm_wday        = (int)st.wDayOfWeek;
          mktime(&sst);
          wcsftime(tchDateTime,COUNTOF(tchDateTime),tchTemplate,&sst);
        }
        else {
          GetDateFormat(LOCALE_USER_DEFAULT,(
            LOWORD(wParam) == IDM_EDIT_INSERT_SHORTDATE) ? DATE_SHORTDATE : DATE_LONGDATE,
            &st,NULL,tchDate,COUNTOF(tchDate));
          GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,NULL,tchTime,COUNTOF(tchTime));

          StringCchPrintf(tchDateTime,COUNTOF(tchDateTime),L"%s %s",tchTime,tchDate);
        }

        WideCharToMultiByteStrg(Encoding_SciCP,tchDateTime, g_pTempLineBufferMain);
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit,SCI_REPLACESEL,0,(LPARAM)g_pTempLineBufferMain);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_INSERT_FILENAME:
    case IDM_EDIT_INSERT_PATHNAME:
      {
        SHFILEINFO shfi;
        WCHAR *pszInsert;
        WCHAR tchUntitled[32];
        //int   iSelStart;

        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          if (LOWORD(wParam) == IDM_EDIT_INSERT_FILENAME) {
            SHGetFileInfo2(g_wchCurFile,FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),
              SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
            pszInsert = shfi.szDisplayName;
          }
          else
            pszInsert = g_wchCurFile;
        }
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszInsert = tchUntitled;
        }

        WideCharToMultiByteStrg(Encoding_SciCP,pszInsert, g_pTempLineBufferMain);
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit,SCI_REPLACESEL,0,(LPARAM)g_pTempLineBufferMain);
        _END_UNDO_ACTION_;
    }
      break;


    case IDM_EDIT_INSERT_GUID:
      {
        GUID guid;
        if (SUCCEEDED(CoCreateGuid(&guid))) {  
          if (StringFromGUID2(&guid, tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer))) {
            StrTrimW(tchMaxPathBuffer, L"{}");
            if (WideCharToMultiByteStrg(Encoding_SciCP, tchMaxPathBuffer, g_pTempLineBufferMain)) {
              _BEGIN_UNDO_ACTION_;
              SendMessage(g_hwndEdit,SCI_REPLACESEL,0,(LPARAM)g_pTempLineBufferMain);
              _END_UNDO_ACTION_;
            }
          }
        }
      }
      break;


    case IDM_EDIT_LINECOMMENT:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;

        switch (SendMessage(g_hwndEdit, SCI_GETLEXER, 0, 0)) {
        default:
        case SCLEX_NULL:
        case SCLEX_CSS:
        case SCLEX_DIFF:
        case SCLEX_MARKDOWN:
        case SCLEX_JSON:
          break;
        case SCLEX_HTML:
        case SCLEX_XML:
        case SCLEX_CPP:
        case SCLEX_PASCAL:
          EditToggleLineComments(g_hwndEdit, L"//", false);
          break;
        case SCLEX_VBSCRIPT:
        case SCLEX_VB:
          EditToggleLineComments(g_hwndEdit, L"'", false);
          break;
        case SCLEX_MAKEFILE:
        case SCLEX_PERL:
        case SCLEX_PYTHON:
        case SCLEX_CONF:
        case SCLEX_BASH:
        case SCLEX_TCL:
        case SCLEX_RUBY:
        case SCLEX_POWERSHELL:
        case SCLEX_CMAKE:
        case SCLEX_AVS:
        case SCLEX_YAML:
        case SCLEX_COFFEESCRIPT:
        case SCLEX_NIMROD:
          EditToggleLineComments(g_hwndEdit, L"#", true);
          break;
        case SCLEX_ASM:
        case SCLEX_PROPERTIES:
        case SCLEX_AU3:
        case SCLEX_AHK:
        case SCLEX_NSIS: // # could also be used instead
        case SCLEX_INNOSETUP:
        case SCLEX_REGISTRY:
          EditToggleLineComments(g_hwndEdit, L";", true);
          break;
        case SCLEX_SQL:
        case SCLEX_LUA:
        case SCLEX_VHDL:
          EditToggleLineComments(g_hwndEdit, L"--", true);
          break;
        case SCLEX_BATCH:
          EditToggleLineComments(g_hwndEdit, L"rem ", true);
          break;
        case SCLEX_LATEX:
        case SCLEX_MATLAB:
          EditToggleLineComments(g_hwndEdit, L"%", true);
          break;
        }

        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STREAMCOMMENT:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;

        switch (SendMessage(g_hwndEdit, SCI_GETLEXER, 0, 0)) {
        default:
        case SCLEX_NULL:
        case SCLEX_VBSCRIPT:
        case SCLEX_MAKEFILE:
        case SCLEX_VB:
        case SCLEX_ASM:
        case SCLEX_SQL:
        case SCLEX_PERL:
        case SCLEX_PYTHON:
        case SCLEX_PROPERTIES:
        case SCLEX_CONF:
        case SCLEX_POWERSHELL:
        case SCLEX_BATCH:
        case SCLEX_DIFF:
        case SCLEX_BASH:
        case SCLEX_TCL:
        case SCLEX_AU3:
        case SCLEX_LATEX:
        case SCLEX_AHK:
        case SCLEX_RUBY:
        case SCLEX_CMAKE:
        case SCLEX_MARKDOWN:
        case SCLEX_YAML:
        case SCLEX_JSON:
        case SCLEX_REGISTRY:
        case SCLEX_NIMROD:
          break;
        case SCLEX_HTML:
        case SCLEX_XML:
        case SCLEX_CSS:
        case SCLEX_CPP:
        case SCLEX_NSIS:
        case SCLEX_AVS:
        case SCLEX_VHDL:
          EditEncloseSelection(g_hwndEdit, L"/*", L"*/");
          break;
        case SCLEX_PASCAL:
        case SCLEX_INNOSETUP:
          EditEncloseSelection(g_hwndEdit, L"{", L"}");
          break;
        case SCLEX_LUA:
          EditEncloseSelection(g_hwndEdit, L"--[[", L"]]");
          break;
        case SCLEX_COFFEESCRIPT:
          EditEncloseSelection(g_hwndEdit, L"###", L"###");
          break;
        case SCLEX_MATLAB:
          EditEncloseSelection(g_hwndEdit, L"%{", L"%}");
        }
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_URLENCODE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditURLEncode(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_URLDECODE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditURLDecode(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_ESCAPECCHARS:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditEscapeCChars(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_UNESCAPECCHARS:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditUnescapeCChars(g_hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CHAR2HEX:
      {
        _BEGIN_UNDO_ACTION_;
        EditChar2Hex(g_hwndEdit);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_HEX2CHAR:
      EditHex2Char(g_hwndEdit);
      break;


    case IDM_EDIT_FINDMATCHINGBRACE:
      EditFindMatchingBrace(g_hwndEdit);
      break;


    case IDM_EDIT_SELTOMATCHINGBRACE:
      {
        _BEGIN_UNDO_ACTION_;
        EditSelectToMatchingBrace(g_hwndEdit);
        _END_UNDO_ACTION_;
      }
      break;


    // Main Bookmark Functions
    case BME_EDIT_BOOKMARKNEXT:
    {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);

        int bitmask = (1 << MARKER_NP3_BOOKMARK);
        DocLn iNextLine = (DocLn)SendMessage( g_hwndEdit , SCI_MARKERNEXT , iLine+1 , bitmask );
        if (iNextLine == (DocLn)-1)
        {
            iNextLine = (DocLn)SendMessage( g_hwndEdit , SCI_MARKERNEXT , 0 , bitmask );
        }

        if (iNextLine != (DocLn)-1)
        {
            SciCall_EnsureVisible(iNextLine);
            SciCall_GotoLine(iNextLine);
            SciCall_ScrollCaret();
        }
        break;
    }

    case BME_EDIT_BOOKMARKPREV:
    {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);

        int bitmask = (1 << MARKER_NP3_BOOKMARK);
        DocLn iNextLine = (DocLn)SendMessage( g_hwndEdit , SCI_MARKERPREVIOUS , iLine-1 , bitmask );
        if (iNextLine == (DocLn)-1)
        {
            iNextLine = (DocLn)SendMessage( g_hwndEdit , SCI_MARKERPREVIOUS , SciCall_GetLineCount(), bitmask );
        }

        if (iNextLine != (DocLn)-1)
        {
            SciCall_EnsureVisible(iNextLine);
            SciCall_GotoLine(iNextLine);
            SciCall_ScrollCaret();
        }
        break;
    }

    case BME_EDIT_BOOKMARKTOGGLE:
      {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);

        int bitmask = SciCall_MarkerGet(iLine);

        if (bitmask & (1 << MARKER_NP3_BOOKMARK)) {
          // unset
          SciCall_MarkerDelete(iLine, MARKER_NP3_BOOKMARK);
        }
        else {
          Style_SetBookmark(g_hwndEdit, g_bShowSelectionMargin);
          // set
          SciCall_MarkerAdd(iLine, MARKER_NP3_BOOKMARK);
          UpdateLineNumberWidth();
        }
        break;
      }

    case BME_EDIT_BOOKMARKCLEAR:
      SciCall_MarkerDeleteAll(MARKER_NP3_BOOKMARK);
    break;



    case IDM_EDIT_FIND:
      if (!IsWindow(g_hwndDlgFindReplace)) {
        g_bFindReplCopySelOrClip = true;
        g_hwndDlgFindReplace = EditFindReplaceDlg(g_hwndEdit, &g_efrData, false);
      }
      else {
        g_bFindReplCopySelOrClip = (GetForegroundWindow() != g_hwndDlgFindReplace);
        if (GetDlgItem(g_hwndDlgFindReplace, IDC_REPLACE)) {
          SendMessage(g_hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOFIND, 1), 0);
          DestroyWindow(g_hwndDlgFindReplace);
          g_hwndDlgFindReplace = EditFindReplaceDlg(g_hwndEdit, &g_efrData, false);
        }
        else {
          SetForegroundWindow(g_hwndDlgFindReplace);
          PostMessage(g_hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(g_hwndDlgFindReplace, IDC_FINDTEXT)), 1);
        }
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_REPLACE:
      if (!IsWindow(g_hwndDlgFindReplace)) {
        g_bFindReplCopySelOrClip = true;
        g_hwndDlgFindReplace = EditFindReplaceDlg(g_hwndEdit, &g_efrData, true);
      }
      else {
        g_bFindReplCopySelOrClip = (GetForegroundWindow() != g_hwndDlgFindReplace);
        if (!GetDlgItem(g_hwndDlgFindReplace, IDC_REPLACE)) {
          SendMessage(g_hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOREPLACE, 1), 0);
          DestroyWindow(g_hwndDlgFindReplace);
          g_hwndDlgFindReplace = EditFindReplaceDlg(g_hwndEdit, &g_efrData, true);
        }
        else {
          SetForegroundWindow(g_hwndDlgFindReplace);
          PostMessage(g_hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(g_hwndDlgFindReplace, IDC_FINDTEXT)), 1);
        }
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_FINDNEXT:
    case IDM_EDIT_FINDPREV:
    case IDM_EDIT_REPLACENEXT:
    case IDM_EDIT_SELTONEXT:
    case IDM_EDIT_SELTOPREV:

      if (SciCall_GetTextLength() == 0)
        break;

      if (IsFindPatternEmpty() && !StringCchLenA(g_efrData.szFind, COUNTOF(g_efrData.szFind)))
      {
        if (LOWORD(wParam) != IDM_EDIT_REPLACENEXT)
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_FIND,1),0);
        else
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
      }
      else {

        switch (LOWORD(wParam)) {

          case IDM_EDIT_FINDNEXT:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionEnd(g_hwndEdit);
            }
            EditFindNext(g_hwndEdit,&g_efrData,false,false);
            break;

          case IDM_EDIT_FINDPREV:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionStart(g_hwndEdit);
            }
            EditFindPrev(g_hwndEdit,&g_efrData,false,false);
            break;

          case IDM_EDIT_REPLACENEXT:
            if (bReplaceInitialized)
              EditReplace(g_hwndEdit,&g_efrData);
            else
              SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
            break;

          case IDM_EDIT_SELTONEXT:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionEnd(g_hwndEdit);
            }
            EditFindNext(g_hwndEdit,&g_efrData,true,false);
            break;

          case IDM_EDIT_SELTOPREV:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionStart(g_hwndEdit);
            }
            EditFindPrev(g_hwndEdit,&g_efrData,true,false);
            break;
        }
      }
      break;


    case CMD_FINDNEXTSEL:
    case CMD_FINDPREVSEL:
    case IDM_EDIT_SAVEFIND:
    {
      DocPos cchSelection = SciCall_GetSelText(NULL);

      if (1 >= cchSelection)
      {
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_SELECTWORD, 1), 0);
        cchSelection = SciCall_GetSelText(NULL);
      }

      if ((1 < cchSelection) && (cchSelection < FNDRPL_BUFFER))
      {
        char  mszSelection[FNDRPL_BUFFER];
        SciCall_GetSelText(mszSelection);

        // Check lpszSelection and truncate newlines
        char *lpsz = StrChrA(mszSelection, '\n');
        if (lpsz) *lpsz = '\0';

        lpsz = StrChrA(mszSelection, '\r');
        if (lpsz) *lpsz = '\0';

        StringCchCopyA(g_efrData.szFind, COUNTOF(g_efrData.szFind), mszSelection);
        g_efrData.fuFlags &= (~(SCFIND_REGEXP | SCFIND_POSIX));
        g_efrData.bTransformBS = false;

        WCHAR wszBuf[FNDRPL_BUFFER];
        MultiByteToWideCharStrg(Encoding_SciCP, mszSelection, wszBuf);
        MRU_Add(g_pMRUfind, wszBuf, 0, 0, NULL);
        SetFindPattern(wszBuf);

        switch (LOWORD(wParam)) {

        case IDM_EDIT_SAVEFIND:
          break;

        case CMD_FINDNEXTSEL:
          if (!SciCall_IsSelectionEmpty()) {
            EditJumpToSelectionEnd(g_hwndEdit);
          }
          EditFindNext(g_hwndEdit, &g_efrData, false, false);
          break;

        case CMD_FINDPREVSEL:
          if (!SciCall_IsSelectionEmpty()) {
            EditJumpToSelectionStart(g_hwndEdit);
          }
          EditFindPrev(g_hwndEdit, &g_efrData, false, false);
          break;
        }
      }
    }
    break;



    case IDM_EDIT_COMPLETEWORD:
      EditCompleteWord(g_hwndEdit, true);
      break;


    case IDM_EDIT_GOTOLINE:
      EditLinenumDlg(g_hwndEdit);
      UpdateStatusbar(false);
      break;


    case IDM_VIEW_SCHEME:
      Style_SelectLexerDlg(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_USE2NDDEFAULT:
      Style_ToggleUse2ndDefault(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_SCHEMECONFIG:
      if (!IsWindow(g_hwndDlgCustomizeSchemes)) {
        g_hwndDlgCustomizeSchemes = Style_CustomizeSchemesDlg(g_hwndEdit);
      }
      else {
        SetForegroundWindow(g_hwndDlgCustomizeSchemes);
      }
      PostMessage(g_hwndDlgCustomizeSchemes, WM_COMMAND, MAKELONG(IDC_SETCURLEXERTV, 1), 0);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_FONT:
      if (!IsWindow(g_hwndDlgCustomizeSchemes))
        Style_SetDefaultFont(g_hwndEdit, true);
      UpdateToolbar();
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_CURRENTSCHEME:
      if (!IsWindow(g_hwndDlgCustomizeSchemes))
        Style_SetDefaultFont(g_hwndEdit, false);
      UpdateToolbar();
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_WORDWRAP:
      g_bWordWrap = (g_bWordWrap) ? false : true;
      _SetWrapIndentMode(g_hwndEdit);
      EditEnsureSelectionVisible(g_hwndEdit);
      bWordWrapG = g_bWordWrap;
      UpdateToolbar();
      break;


    case IDM_VIEW_WORDWRAPSETTINGS:
      if (WordWrapSettingsDlg(hwnd,IDD_MUI_WORDWRAP,&iWordWrapIndent)) {
        _SetWrapIndentMode(g_hwndEdit);
        _SetWrapVisualFlags(g_hwndEdit);
      }
      break;


    case IDM_VIEW_WORDWRAPSYMBOLS:
      bShowWordWrapSymbols = (bShowWordWrapSymbols) ? false : true;
      _SetWrapVisualFlags(g_hwndEdit);
      break;


    case IDM_VIEW_LONGLINEMARKER:
      g_bMarkLongLines = (g_bMarkLongLines) ? false: true;
      if (g_bMarkLongLines) {
        SendMessage(g_hwndEdit,SCI_SETEDGEMODE,(iLongLineMode == EDGE_LINE)?EDGE_LINE:EDGE_BACKGROUND,0);
        Style_SetLongLineColors(g_hwndEdit);
      }
      else
        SendMessage(g_hwndEdit,SCI_SETEDGEMODE,EDGE_NONE,0);

      UpdateToolbar();
      UpdateStatusbar(false);
      break;


    case IDM_VIEW_LONGLINESETTINGS:
      if (LongLineSettingsDlg(hwnd,IDD_MUI_LONGLINES,&g_iLongLinesLimit)) {
        g_bMarkLongLines = true;
        SendMessage(g_hwndEdit, SCI_SETEDGEMODE, (iLongLineMode == EDGE_LINE) ? EDGE_LINE : EDGE_BACKGROUND, 0);
        Style_SetLongLineColors(g_hwndEdit);
        g_iLongLinesLimit = clampi(g_iLongLinesLimit, 0, 4096);
        SendMessage(g_hwndEdit,SCI_SETEDGECOLUMN,g_iLongLinesLimit,0);
        iLongLinesLimitG = g_iLongLinesLimit;
        UpdateToolbar();
        UpdateStatusbar(false);
      }
      break;


    case IDM_VIEW_TABSASSPACES:
      g_bTabsAsSpaces = (g_bTabsAsSpaces) ? false : true;
      SendMessage(g_hwndEdit,SCI_SETUSETABS,!g_bTabsAsSpaces,0);
      bTabsAsSpacesG = g_bTabsAsSpaces;
      break;


    case IDM_VIEW_TABSETTINGS:
      if (TabSettingsDlg(hwnd,IDD_MUI_TABSETTINGS,NULL))
      {
        SendMessage(g_hwndEdit,SCI_SETUSETABS,!g_bTabsAsSpaces,0);
        SendMessage(g_hwndEdit,SCI_SETTABINDENTS,g_bTabIndents,0);
        SendMessage(g_hwndEdit,SCI_SETBACKSPACEUNINDENTS,bBackspaceUnindents,0);
        g_iTabWidth = clampi(g_iTabWidth, 1, 256);
        g_iIndentWidth = clampi(g_iIndentWidth, 0, 256);
        SendMessage(g_hwndEdit,SCI_SETTABWIDTH,g_iTabWidth,0);
        SendMessage(g_hwndEdit,SCI_SETINDENT,g_iIndentWidth,0);
        bTabsAsSpacesG = g_bTabsAsSpaces;
        bTabIndentsG   = g_bTabIndents;
        iTabWidthG     = g_iTabWidth;
        iIndentWidthG  = g_iIndentWidth;
        if (SendMessage(g_hwndEdit, SCI_GETWRAPINDENTMODE, 0, 0) == SC_WRAPINDENT_FIXED) {
          _SetWrapStartIndent(g_hwndEdit);
        }
      }
      break;


    case IDM_VIEW_SHOWINDENTGUIDES:
      bShowIndentGuides = (bShowIndentGuides) ? false : true;
      Style_SetIndentGuides(g_hwndEdit,bShowIndentGuides);
      break;


    case IDM_VIEW_AUTOINDENTTEXT:
      bAutoIndent = (bAutoIndent) ? false : true;
      break;


    case IDM_VIEW_LINENUMBERS:
      g_bShowLineNumbers = (g_bShowLineNumbers) ? false : true;
      UpdateLineNumberWidth();
      break;


    case IDM_VIEW_MARGIN:
      g_bShowSelectionMargin = (g_bShowSelectionMargin) ? false : true;
      Style_SetBookmark(g_hwndEdit, g_bShowSelectionMargin);
      UpdateLineNumberWidth();
      break;

    case IDM_VIEW_AUTOCOMPLETEWORDS:
      g_bAutoCompleteWords = (g_bAutoCompleteWords) ? false : true;  // toggle
      if (!g_bAutoCompleteWords) {
        SciCall_AutoCCancel();  // close the auto completion list
      }
      break;

    case IDM_VIEW_ACCELWORDNAV:
      g_bAccelWordNavigation = (g_bAccelWordNavigation) ? false : true;  // toggle  
      EditSetAccelWordNav(g_hwndEdit,g_bAccelWordNavigation);
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_ONOFF:
      g_iMarkOccurrences = (g_iMarkOccurrences == 0) ? max(1, IniGetInt(L"Settings", L"MarkOccurrences", 1)) : 0;
      MarkAllOccurrences(0, true);
      EnableCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, (g_iMarkOccurrences > 0) && !g_bMarkOccurrencesMatchVisible);
      break;

    case IDM_VIEW_MARKOCCUR_VISIBLE:
      g_bMarkOccurrencesMatchVisible = (g_bMarkOccurrencesMatchVisible) ? false : true;
      MarkAllOccurrences(0, true);
      EnableCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, (g_iMarkOccurrences > 0) && !g_bMarkOccurrencesMatchVisible);
      break;

    case IDM_VIEW_TOGGLE_VIEW:
      if (EditToggleView(g_hwndEdit, false)) {
        EditToggleView(g_hwndEdit, true);
        MarkAllOccurrences(0, true);
      }
      else {
        EditToggleView(g_hwndEdit, true);
      }
      CheckCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, EditToggleView(g_hwndEdit, false));
      UpdateToolbar();
      break;

    case IDM_VIEW_MARKOCCUR_CASE:
      g_bMarkOccurrencesMatchCase = (g_bMarkOccurrencesMatchCase) ? false : true;
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_WNONE:
      g_bMarkOccurrencesMatchWords = false;
      g_bMarkOccurrencesCurrentWord = false;
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_WORD:
      g_bMarkOccurrencesMatchWords = true;
      g_bMarkOccurrencesCurrentWord = false;
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_CURRENT:
      g_bMarkOccurrencesMatchWords = false;
      g_bMarkOccurrencesCurrentWord = true;
      MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, true);
      break;

    case IDM_VIEW_FOLDING:
      g_bShowCodeFolding = (g_bShowCodeFolding) ? false : true;
      Style_SetFolding(g_hwndEdit, g_bShowCodeFolding);
      if (!g_bShowCodeFolding) { EditToggleFolds(EXPAND, true); }
      UpdateToolbar();
      break;


    case IDM_VIEW_TOGGLEFOLDS:
      EditToggleFolds(SNIFF, false);
      break;


    case IDM_VIEW_SHOWBLANKS:
      bViewWhiteSpace = (bViewWhiteSpace) ? false : true;
      SendMessage(g_hwndEdit,SCI_SETVIEWWS,(bViewWhiteSpace)?SCWS_VISIBLEALWAYS:SCWS_INVISIBLE,0);
      break;


    case IDM_VIEW_SHOWEOLS:
      bViewEOLs = (bViewEOLs) ? false : true;
      SendMessage(g_hwndEdit,SCI_SETVIEWEOL,bViewEOLs,0);
      break;


    case IDM_VIEW_MATCHBRACES:
      bMatchBraces = (bMatchBraces) ? false : true;
      if (bMatchBraces)
        EditMatchBrace(g_hwndEdit);
      else
        SendMessage(g_hwndEdit,SCI_BRACEHIGHLIGHT,(WPARAM)-1,(LPARAM)-1);
      break;


    case IDM_VIEW_AUTOCLOSETAGS:
      bAutoCloseTags = (bAutoCloseTags) ? false : true;
      break;

    case IDM_VIEW_HILITECURRENTLINE:
      bHiliteCurrentLine = (bHiliteCurrentLine) ? false : true;
      Style_SetCurrentLineBackground(g_hwndEdit, bHiliteCurrentLine);
      break;

    case IDM_VIEW_HYPERLINKHOTSPOTS:
      g_bHyperlinkHotspot = (g_bHyperlinkHotspot) ? false : true;
      Style_SetUrlHotSpot(g_hwndEdit, g_bHyperlinkHotspot);
      if (g_bHyperlinkHotspot) {
        UpdateVisibleUrlHotspot(0);
      }
      else {
        SciCall_StartStyling(0);
        Style_ResetCurrentLexer(g_hwndEdit);
      }
      break;

    case IDM_VIEW_ZOOMIN:
      {
        SciCall_ZoomIn();
        UpdateLineNumberWidth();
        EditShowZoomCallTip(g_hwndEdit);
      }
      break;

    case IDM_VIEW_ZOOMOUT:
      {
        SciCall_ZoomOut();
        UpdateLineNumberWidth();
        EditShowZoomCallTip(g_hwndEdit);
      }
      break;

    case IDM_VIEW_RESETZOOM:
      {
        SciCall_SetZoom(100);
        UpdateLineNumberWidth();
        EditShowZoomCallTip(g_hwndEdit);
      }
      break;

    case IDM_VIEW_CHASING_DOCTAIL: 
      {
        static int flagPrevChangeNotify = 0;
        static int iPrevFileWatchingMode = 0;
        static bool bPrevResetFileWatching = false;

        g_bChasingDocTail = (g_bChasingDocTail) ? false : true;
        SciCall_SetReadOnly(g_bChasingDocTail);

        if (g_bChasingDocTail) 
        {
          SetForegroundWindow(hwnd);
          flagPrevChangeNotify = g_flagChangeNotify;
          iPrevFileWatchingMode = g_iFileWatchingMode;
          bPrevResetFileWatching = g_bResetFileWatching;
          g_flagChangeNotify = 2;
          g_iFileWatchingMode = 2;
          g_bResetFileWatching = true;
        }
        else {
          g_flagChangeNotify = flagPrevChangeNotify;
          g_iFileWatchingMode = iPrevFileWatchingMode;
          g_bResetFileWatching = bPrevResetFileWatching;
        }
        if (!g_bRunningWatch) { InstallFileWatching(g_wchCurFile); }

        CheckCmd(GetMenu(g_hwndMain), IDM_VIEW_CHASING_DOCTAIL, g_bChasingDocTail);
        UpdateToolbar();
      }
      break;
    
    case IDM_VIEW_SCROLLPASTEOF:
      bScrollPastEOF = (bScrollPastEOF) ? false : true;
      SciCall_SetEndAtLastLine(!bScrollPastEOF);
      break;

    case IDM_VIEW_TOOLBAR:
      bShowToolbar = !bShowToolbar;
      ShowWindow(g_hwndReBar, (bShowToolbar ? SW_SHOW : SW_HIDE));
      UpdateToolbar();
      SendWMSize(hwnd, NULL);
      break;

    case IDM_VIEW_TOGGLETB:
      iHighDpiToolBar = (iHighDpiToolBar <= 0) ? 1 : 0;
      SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
      break;

    case IDM_VIEW_CUSTOMIZETB:
      SendMessage(g_hwndToolbar,TB_CUSTOMIZE,0,0);
      break;

    case IDM_VIEW_STATUSBAR:
      bShowStatusbar = !bShowStatusbar;
      ShowWindow(g_hwndStatus, (bShowStatusbar ? SW_SHOW : SW_HIDE));
      UpdateStatusbar(bShowStatusbar);
      SendWMSize(hwnd, NULL);
      break;


    case IDM_VIEW_STICKYWINPOS:
      g_bStickyWinPos = IniGetBool(L"Settings2",L"StickyWindowPosition",g_bStickyWinPos);
      if (!g_bStickyWinPos) 
      {
        WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32], tchZoom[32];

        int ResX = GetSystemMetrics(SM_CXSCREEN);
        int ResY = GetSystemMetrics(SM_CYSCREEN);

        StringCchPrintf(tchPosX,COUNTOF(tchPosX),L"%ix%i PosX",ResX,ResY);
        StringCchPrintf(tchPosY,COUNTOF(tchPosY),L"%ix%i PosY",ResX,ResY);
        StringCchPrintf(tchSizeX,COUNTOF(tchSizeX),L"%ix%i SizeX",ResX,ResY);
        StringCchPrintf(tchSizeY,COUNTOF(tchSizeY),L"%ix%i SizeY",ResX,ResY);
        StringCchPrintf(tchMaximized,COUNTOF(tchMaximized),L"%ix%i Maximized",ResX,ResY);
        StringCchPrintf(tchZoom, COUNTOF(tchZoom), L"%ix%i Zoom", ResX, ResY);

        g_bStickyWinPos = true;
        IniSetInt(L"Settings2",L"StickyWindowPosition",1);

        // GetWindowPlacement
        WININFO wi = GetMyWindowPlacement(g_hwndMain,NULL);
        IniSetInt(L"Window",tchPosX,wi.x);
        IniSetInt(L"Window",tchPosY,wi.y);
        IniSetInt(L"Window",tchSizeX,wi.cx);
        IniSetInt(L"Window",tchSizeY,wi.cy);
        IniSetInt(L"Window",tchMaximized,wi.max);
        IniSetInt(L"Window", tchZoom, wi.zoom);

        InfoBoxLng(0,L"MsgStickyWinPos",IDS_MUI_STICKYWINPOS);
      }
      else {
        g_bStickyWinPos = false;
        IniSetInt(L"Settings2",L"StickyWindowPosition",0);
      }
      break;


    case IDM_VIEW_REUSEWINDOW:
      if (IniGetInt(L"Settings2",L"ReuseWindow",0))
        IniSetInt(L"Settings2",L"ReuseWindow",0);
      else
        IniSetInt(L"Settings2",L"ReuseWindow",1);
      break;


    case IDM_VIEW_SINGLEFILEINSTANCE:
      if (IniGetInt(L"Settings2",L"SingleFileInstance",0))
        IniSetInt(L"Settings2",L"SingleFileInstance",0);
      else
        IniSetInt(L"Settings2",L"SingleFileInstance",1);
      break;


    case IDM_VIEW_ALWAYSONTOP:
      if ((bAlwaysOnTop || g_flagAlwaysOnTop == 2) && g_flagAlwaysOnTop != 1) {
        bAlwaysOnTop = 0;
        g_flagAlwaysOnTop = 0;
        SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
      else {
        bAlwaysOnTop = 1;
        g_flagAlwaysOnTop = 0;
        SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
      break;


    case IDM_VIEW_MINTOTRAY:
      bMinimizeToTray =(bMinimizeToTray) ? false : true;
      break;


    case IDM_VIEW_TRANSPARENT:
      bTransparentMode =(bTransparentMode) ? false : true;
      SetWindowTransparentMode(hwnd,bTransparentMode);
      break;

    case IDM_SET_RENDER_TECH_DEFAULT:
    case IDM_SET_RENDER_TECH_D2D:
    case IDM_SET_RENDER_TECH_D2DRETAIN:
    case IDM_SET_RENDER_TECH_D2DDC:
      g_iRenderingTechnology = (int)LOWORD(wParam) - IDM_SET_RENDER_TECH_DEFAULT;
      if (g_iRenderingTechnology == 0) {
        SciCall_SetBidirectional(SciBidirectional[0]);
      }
      SciCall_SetBufferedDraw((g_iRenderingTechnology == 0));
      SciCall_SetTechnology(DirectWriteTechnology[g_iRenderingTechnology]);
      break;

    case IDM_SET_BIDIRECTIONAL_NONE:
    case IDM_SET_BIDIRECTIONAL_L2R:
    case IDM_SET_BIDIRECTIONAL_R2L:
      g_iBidirectional = (int)LOWORD(wParam) - IDM_SET_BIDIRECTIONAL_NONE;
      SciCall_SetBidirectional(SciBidirectional[g_iBidirectional]);
      break;

    
    case IDM_VIEW_SHOWFILENAMEONLY:
    case IDM_VIEW_SHOWFILENAMEFIRST:
    case IDM_VIEW_SHOWFULLPATH:
      iPathNameFormat = (int)LOWORD(wParam) - IDM_VIEW_SHOWFILENAMEONLY;
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
      UpdateToolbar();
      break;


    case IDM_VIEW_SHOWEXCERPT:
      EditGetExcerpt(g_hwndEdit,szTitleExcerpt,COUNTOF(szTitleExcerpt));
      UpdateToolbar();
      break;


    case IDM_VIEW_NOSAVERECENT:
      g_bSaveRecentFiles = (g_bSaveRecentFiles) ? false : true;
      break;


    case IDM_VIEW_NOPRESERVECARET:
      g_bPreserveCaretPos = (g_bPreserveCaretPos) ? false : true;
      break;


    case IDM_VIEW_NOSAVEFINDREPL:
      g_bSaveFindReplace = (g_bSaveFindReplace) ? false : true;
      break;


    case IDM_VIEW_SAVEBEFORERUNNINGTOOLS:
      bSaveBeforeRunningTools = (bSaveBeforeRunningTools) ? false : true;
      break;


    case IDM_VIEW_CHANGENOTIFY:
      if (ChangeNotifyDlg(hwnd))
        InstallFileWatching(g_wchCurFile);
      break;


    case IDM_VIEW_NOESCFUNC:
    case IDM_VIEW_ESCMINIMIZE:
    case IDM_VIEW_ESCEXIT:
      iEscFunction = (int)LOWORD(wParam) - IDM_VIEW_NOESCFUNC;
      break;


    case IDM_VIEW_SAVESETTINGS:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGS)) {
        g_bSaveSettings = (g_bSaveSettings) ? false : true;
        IniSetInt(L"Settings", L"SaveSettings", g_bSaveSettings);
      }
      break;


    case IDM_VIEW_SAVESETTINGSNOW:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGSNOW)) {

        bool bCreateFailure = false;

        if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) == 0) {

          if (StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0) {
            if (CreateIniFileEx(g_wchIniFile2)) {
              StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),g_wchIniFile2);
              StringCchCopy(g_wchIniFile2,COUNTOF(g_wchIniFile2),L"");
            }
            else
              bCreateFailure = true;
          }

          else
            break;
        }

        if (!bCreateFailure) {

          if (WritePrivateProfileString(L"Settings",L"WriteTest",L"ok",g_wchIniFile)) {
            SaveSettings(true);
            MsgBoxLng(MBINFO,IDS_MUI_SAVEDSETTINGS);
          }
          else {
            dwLastIOError = GetLastError();
            MsgBoxLng(MBWARN,IDS_MUI_WRITEINI_FAIL);
          }
        }
        else
          MsgBoxLng(MBWARN,IDS_MUI_CREATEINI_FAIL);
      }
      break;


    case IDM_HELP_ONLINEDOCUMENTATION:
      ShellExecute(0, 0, ONLINE_HELP_WEBSITE, 0, 0, SW_SHOW);
      break;

    case IDM_HELP_ABOUT:
        ThemedDialogBox(g_hLngResContainer, MAKEINTRESOURCE(IDD_MUI_ABOUT), hwnd, AboutDlgProc);
      break;

    case IDM_SETPASS:
      if (GetFileKey(g_hwndEdit)) {
        _SetDocumentModified(true);
      }
      break;

    case IDM_HELP_CMD:
      DisplayCmdLineHelp(hwnd);
      break;

    case CMD_ESCAPE:
      //close the autocomplete box
      SciCall_AutoCCancel();

      if (iEscFunction == 1) {
        SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
      }
      else if (iEscFunction == 2) {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
      }
      break;


    case CMD_SHIFTESC:
      if (FileSave(true, false, false, false)) {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
      }
      break;


    case CMD_CTRLENTER:
      {
        _BEGIN_UNDO_ACTION_;
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);
        if (iLine <= 0) {
          SciCall_GotoLine(0);
          SciCall_NewLine();
          SciCall_GotoLine(0);
        }
        else {
          SciCall_GotoPos(SciCall_GetLineEndPosition(iLine - 1));
          SciCall_NewLine();
        }
        _END_UNDO_ACTION_;
      }
      break;


    // Newline with toggled auto indent setting
    case CMD_SHIFTCTRLENTER:
      bAutoIndent = (bAutoIndent) ? 0 : 1;
      SciCall_NewLine();
      bAutoIndent = (bAutoIndent) ? 0 : 1;
      break;


    case IDM_EDIT_CLEAR:
    case CMD_DEL:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_Clear();
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_CTRLLEFT:
        SendMessage(g_hwndEdit, SCI_WORDLEFT, 0, 0);
      break;


    case CMD_CTRLRIGHT:
        SendMessage(g_hwndEdit, SCI_WORDRIGHT, 0, 0);
      break;


    case CMD_CTRLBACK:
      {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocPos iAnchor = SciCall_GetAnchor();
        const DocLn iLine = SciCall_LineFromPosition(iPos);
        const DocPos iStartPos = SciCall_PositionFromLine(iLine);
        const DocPos iIndentPos = SciCall_GetLineIndentPosition(iLine);

        if (iPos != iAnchor) {
          _BEGIN_UNDO_ACTION_;
          SciCall_SetSel(iPos, iPos);
          _END_UNDO_ACTION_;
        }
        else {
          if (iPos == iStartPos)
            Sci_SendMsgV0(DELETEBACK);
          else if (iPos <= iIndentPos)
            Sci_SendMsgV0(DELLINELEFT);
          else
            Sci_SendMsgV0(DELWORDLEFT);
        }
      }
      break;


    case CMD_CTRLDEL:
      {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocPos iAnchor = SciCall_GetAnchor();
        const DocLn iLine = SciCall_LineFromPosition(iPos);
        const DocPos iStartPos = SciCall_PositionFromLine(iLine);
        const DocPos iEndPos = SciCall_GetLineEndPosition(iLine);

        if (iPos != iAnchor) {
          _BEGIN_UNDO_ACTION_;
          SciCall_SetSel(iPos, iPos);
          _END_UNDO_ACTION_;
        }
        else {
          if (iStartPos != iEndPos)
            Sci_SendMsgV0(DELWORDRIGHT);
          else // iStartPos == iEndPos
            Sci_SendMsgV0(LINEDELETE);
        }
      }
      break;


    case CMD_RECODEDEFAULT:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_SrcCmdLn(Encoding_MapUnicode(g_iDefaultNewFileEncoding));
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),g_wchCurFile);
          FileLoad(false,false,true,true,true,tchMaxPathBuffer);
        }
      }
      break;


    case CMD_RECODEANSI:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_SrcCmdLn(CPI_ANSI_DEFAULT);
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),g_wchCurFile);
          FileLoad(false,false,true,true,bSkipANSICodePageDetection,tchMaxPathBuffer);
        }
      }
      break;


    case CMD_RECODEOEM:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          Encoding_SrcCmdLn(CPI_OEM);
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),g_wchCurFile);
          FileLoad(false,false,true,true,true,tchMaxPathBuffer);
        }
      }
      break;


    case CMD_RELOADASCIIASUTF8:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          bForceLoadASCIIasUTF8 = true;
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),g_wchCurFile);
          FileLoad(false, false, true, true, true, tchMaxPathBuffer);
          bForceLoadASCIIasUTF8 = false;
        }
      }
      break;


    case CMD_RELOADFORCEDETECTION:
    {
      g_bForceCompEncDetection = true;
      if (StringCchLenW(g_wchCurFile, COUNTOF(g_wchCurFile))) {
        bForceLoadASCIIasUTF8 = false;
        StringCchCopy(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), g_wchCurFile);
        FileLoad(false, false, true, false, false, tchMaxPathBuffer);
      }
      g_bForceCompEncDetection = false;
    }
    break;

    case CMD_RELOADNOFILEVARS:
      {
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
          int _fNoFileVariables = g_flagNoFileVariables;
          bool _bNoEncodingTags = bNoEncodingTags;
          g_flagNoFileVariables = 1;
          bNoEncodingTags = 1;
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),g_wchCurFile);
          FileLoad(false,false,true, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchMaxPathBuffer);
          g_flagNoFileVariables = _fNoFileVariables;
          bNoEncodingTags = _bNoEncodingTags;
        }
      }
      break;


    case CMD_LEXDEFAULT:
      Style_SetDefaultLexer(g_hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateLineNumberWidth();
      break;


    //case CMD_LEXHTML:
    //  Style_SetHTMLLexer(g_hwndEdit);
    //  UpdateToolbar();
    //  UpdateStatusbar(false);
    //  UpdateLineNumberWidth();
    //  break;


    //case CMD_LEXXML:
    //  Style_SetXMLLexer(g_hwndEdit);
    //  UpdateToolbar();
    //  UpdateStatusbar(false);
    //  UpdateLineNumberWidth();
    //  break;


    case CMD_TIMESTAMPS:
      {
        WCHAR wchFind[128] = { L'\0' };
        WCHAR wchTemplate[128] = { L'\0' };
        WCHAR wchReplace[128] = { L'\0' };

        SYSTEMTIME st;
        struct tm sst;

        EDITFINDREPLACE efrTS = EFR_INIT_DATA;
        efrTS.hwnd = g_hwndEdit;
        efrTS.fuFlags = SCFIND_REGEXP;

        IniGetString(L"Settings2",L"TimeStamp",L"\\$Date:[^\\$]+\\$ | $Date: %Y/%m/%d %H:%M:%S $",wchFind,COUNTOF(wchFind));

        WCHAR *pwchSep = StrChr(wchFind, L'|');
        if (pwchSep) {
          StringCchCopy(wchTemplate,COUNTOF(wchTemplate),pwchSep + 1);
          *pwchSep = 0;
        }

        StrTrim(wchFind,L" ");
        StrTrim(wchTemplate,L" ");

        if (StringCchLenW(wchFind,COUNTOF(wchFind)) == 0 || StringCchLenW(wchTemplate,COUNTOF(wchTemplate)) == 0)
          break;

        GetLocalTime(&st);
        sst.tm_isdst = -1;
        sst.tm_sec   = (int)st.wSecond;
        sst.tm_min   = (int)st.wMinute;
        sst.tm_hour  = (int)st.wHour;
        sst.tm_mday  = (int)st.wDay;
        sst.tm_mon   = (int)st.wMonth - 1;
        sst.tm_year  = (int)st.wYear - 1900;
        sst.tm_wday  = (int)st.wDayOfWeek;
        mktime(&sst);
        wcsftime(wchReplace,COUNTOF(wchReplace),wchTemplate,&sst);

        WideCharToMultiByteStrg(Encoding_SciCP,wchFind,efrTS.szFind);
        WideCharToMultiByteStrg(Encoding_SciCP,wchReplace,efrTS.szReplace);

        if (!SendMessage(g_hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0))
          EditReplaceAllInSelection(g_hwndEdit, &efrTS, true);
        else
          EditReplaceAll(g_hwndEdit,&efrTS,true);
      }
      break;


    case IDM_HELP_ADMINEXE:
      DialogAdminExe(hwnd, true);
      break;

    case IDM_HELP_UPDATEWEBSITE:
      DialogAdminExe(hwnd, false);
      break;

    case CMD_WEBACTION1:
    case CMD_WEBACTION2:
      {
        LPWSTR lpszTemplateName = (LOWORD(wParam) == CMD_WEBACTION1) ? L"WebTemplate1" : L"WebTemplate2";

        bool bCmdEnabled = IniGetString(L"Settings2",lpszTemplateName,L"",tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer));

        if (bCmdEnabled) {

          const DocPos cchSelection = SciCall_GetSelText(NULL);

          char  mszSelection[512] = { '\0' };
          if ((1 < cchSelection) && (cchSelection < (DocPos)COUNTOF(mszSelection)))
          {
            SciCall_GetSelText(mszSelection);

            // Check lpszSelection and truncate bad WCHARs
            char* lpsz = StrChrA(mszSelection,13);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(mszSelection,10);
            if (lpsz) *lpsz = '\0';

            lpsz = StrChrA(mszSelection,9);
            if (lpsz) *lpsz = '\0';

            if (StringCchLenA(mszSelection,COUNTOF(mszSelection))) {

              WCHAR wszSelection[512] = { L'\0' };
              MultiByteToWideCharStrg(Encoding_SciCP,mszSelection,wszSelection);

              int cmdsz = (512 + COUNTOF(tchMaxPathBuffer) + MAX_PATH + 32);
              LPWSTR lpszCommand = AllocMem(sizeof(WCHAR)*cmdsz, HEAP_ZERO_MEMORY);
              StringCchPrintf(lpszCommand,cmdsz,tchMaxPathBuffer,wszSelection);
              ExpandEnvironmentStringsEx(lpszCommand, cmdsz);

              WCHAR wchDirectory[MAX_PATH] = { L'\0' };
              if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
                StringCchCopy(wchDirectory,COUNTOF(wchDirectory),g_wchCurFile);
                PathRemoveFileSpec(wchDirectory);
              }

              SHELLEXECUTEINFO sei;
              ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
              sei.cbSize = sizeof(SHELLEXECUTEINFO);
              sei.fMask = SEE_MASK_NOZONECHECKS;
              sei.hwnd = NULL;
              sei.lpVerb = NULL;
              sei.lpFile = lpszCommand;
              sei.lpParameters = NULL;
              sei.lpDirectory = wchDirectory;
              sei.nShow = SW_SHOWNORMAL;
              ShellExecuteEx(&sei);

              FreeMem(lpszCommand);
            }
          }
        }
      }
      break;


    case CMD_INCLINELIMIT:
    case CMD_DECLINELIMIT:
      if (!g_bMarkLongLines)
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_LONGLINEMARKER,1),0);
      else {
        if (LOWORD(wParam) == CMD_INCLINELIMIT)
          g_iLongLinesLimit++;
        else
          g_iLongLinesLimit--;
        g_iLongLinesLimit = clampi(g_iLongLinesLimit, 0, 4096);
        SendMessage(g_hwndEdit,SCI_SETEDGECOLUMN,g_iLongLinesLimit,0);
        UpdateToolbar();
        UpdateStatusbar(false);
        iLongLinesLimitG = g_iLongLinesLimit;
      }
      break;

    case CMD_STRINGIFY:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(g_hwndEdit, L"'", L"'");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_STRINGIFY2:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(g_hwndEdit, L"\"", L"\"");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_EMBRACE:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(g_hwndEdit, L"(", L")");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_EMBRACE2:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(g_hwndEdit, L"[", L"]");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_EMBRACE3:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(g_hwndEdit, L"{", L"}");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_EMBRACE4:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(g_hwndEdit, L"`", L"`");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_INCREASENUM:
      EditModifyNumber(g_hwndEdit,true);
      break;


    case CMD_DECREASENUM:
      EditModifyNumber(g_hwndEdit,false);
      break;


    case CMD_TOGGLETITLE:
      EditGetExcerpt(g_hwndEdit,szTitleExcerpt,COUNTOF(szTitleExcerpt));
      UpdateToolbar();
      break;


    case CMD_JUMP2SELSTART:
      EditJumpToSelectionStart(g_hwndEdit);
      SciCall_ChooseCaretX();
      break;

    case CMD_JUMP2SELEND:
      EditJumpToSelectionEnd(g_hwndEdit);
      SciCall_ChooseCaretX();
      break;


    case CMD_COPYPATHNAME: {

        WCHAR *pszCopy;
        WCHAR tchUntitled[32] = { L'\0' };
        if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
          pszCopy = g_wchCurFile;
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszCopy = tchUntitled;
        }
        SetClipboardTextW(hwnd, pszCopy);
        UpdateToolbar();
      }
      break;


    case CMD_COPYWINPOS: {
        WININFO wi = GetMyWindowPlacement(g_hwndMain,NULL);
        StringCchPrintf(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),L"/pos %i,%i,%i,%i,%i",wi.x,wi.y,wi.cx,wi.cy,wi.max);
        SetClipboardTextW(hwnd, tchMaxPathBuffer);
        UpdateToolbar();
      }
      break;


    case CMD_INITIALWINPOS:
      SnapToWinInfoPos(hwnd, &g_WinInfo, false);
      break;

    case CMD_FULLSCRWINPOS:
      {
        WININFO winfo = GetMyWindowPlacement(g_hwndMain, NULL);
        SnapToWinInfoPos(hwnd, &winfo, true);
      }
      break;

    case CMD_DEFAULTWINPOS:
      {
        WININFO winfo = GetMyWindowPlacement(g_hwndMain, NULL);
        _InitDefaultWndPos(&winfo);
        SnapToWinInfoPos(hwnd, &winfo, false);
      }
      break;


    case CMD_OPENINIFILE:
      if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile))) {
        SaveSettings(false);
        FileLoad(false,false,false,false,true,g_wchIniFile);
      }
      break;


    case CMD_OPEN_HYPERLINK:
        OpenHotSpotURL(SciCall_GetCurrentPos(), false);
      break;


    case CMD_ALTDOWN:
      EditFoldAltArrow(DOWN, SNIFF);
      break;

    case CMD_ALTUP:
      EditFoldAltArrow(UP, SNIFF);
      break;

    case CMD_ALTLEFT:
      EditFoldAltArrow(NONE, FOLD);
      break;

    case CMD_ALTRIGHT:
      EditFoldAltArrow(NONE, EXPAND);
      break;


    case IDT_FILE_NEW:
      if (IsCmdEnabled(hwnd,IDM_FILE_NEW))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_NEW,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_OPEN:
      if (IsCmdEnabled(hwnd,IDM_FILE_OPEN))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPEN,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_BROWSE:
      if (IsCmdEnabled(hwnd,IDM_FILE_BROWSE))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_BROWSE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_SAVE:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVE))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_SAVE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_UNDO:
      if (IsCmdEnabled(hwnd,IDM_EDIT_UNDO))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_UNDO,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_REDO:
      if (IsCmdEnabled(hwnd,IDM_EDIT_REDO))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REDO,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_CUT:
      if (IsCmdEnabled(hwnd,IDM_EDIT_CUT))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_CUT,1),0);
      else
        MessageBeep(0);
        //SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_CUTLINE,1),0);
      break;


    case IDT_EDIT_COPY:
      if (IsCmdEnabled(hwnd,IDM_EDIT_COPY))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_COPY,1),0);
      else
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_COPYALL,1),0);     // different to Keyboard-Shortcut
      break;


    case IDT_EDIT_PASTE:
      if (IsCmdEnabled(hwnd,IDM_EDIT_PASTE))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_PASTE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_FIND:
      if (IsCmdEnabled(hwnd,IDM_EDIT_FIND))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_FIND,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_REPLACE:
      if (IsCmdEnabled(hwnd,IDM_EDIT_REPLACE))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_WORDWRAP:
      if (IsCmdEnabled(hwnd,IDM_VIEW_WORDWRAP))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_WORDWRAP,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_ZOOMIN:
      if (IsCmdEnabled(hwnd,IDM_VIEW_ZOOMIN))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_ZOOMIN,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_ZOOMOUT:
      if (IsCmdEnabled(hwnd,IDM_VIEW_ZOOMOUT))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_ZOOMOUT,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_CHASING_DOCTAIL:
      if (IsCmdEnabled(hwnd, IDM_VIEW_CHASING_DOCTAIL))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_CHASING_DOCTAIL,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_SCHEME:
      if (IsCmdEnabled(hwnd,IDM_VIEW_SCHEME))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_SCHEME,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_SCHEMECONFIG:
      if (IsCmdEnabled(hwnd,IDM_VIEW_SCHEMECONFIG))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_SCHEMECONFIG,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_EXIT:
      PostMessage(hwnd,WM_CLOSE,0,0);
      break;


    case IDT_FILE_SAVEAS:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVEAS))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_SAVEAS,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_SAVECOPY:
      if (IsCmdEnabled(hwnd,IDM_FILE_SAVECOPY))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_SAVECOPY,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_EDIT_CLEAR:
      if (IsCmdEnabled(hwnd,IDM_EDIT_CLEAR))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_CLEAR,1),0);
      else
        SendMessage(g_hwndEdit,SCI_CLEARALL,0,0);
      break;


    case IDT_FILE_PRINT:
      if (IsCmdEnabled(hwnd,IDM_FILE_PRINT))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_PRINT,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_OPENFAV:
      if (IsCmdEnabled(hwnd,IDM_FILE_OPENFAV))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPENFAV,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_ADDTOFAV:
      if (IsCmdEnabled(hwnd,IDM_FILE_ADDTOFAV))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_ADDTOFAV,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_TOGGLEFOLDS:
      if (IsCmdEnabled(hwnd,IDM_VIEW_TOGGLEFOLDS))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_TOGGLEFOLDS,1),0);
      else
        MessageBeep(0);
      break;

      
    case IDT_VIEW_TOGGLE_VIEW:
      if (IsCmdEnabled(hwnd, IDM_VIEW_TOGGLE_VIEW))
        SendMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_TOGGLE_VIEW, 1), 0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_LAUNCH:
      if (IsCmdEnabled(hwnd,IDM_FILE_LAUNCH))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_LAUNCH,1),0);
      else
        MessageBeep(0);
      break;

    default:
      return DefWindowProc(hwnd, umsg, wParam, lParam);
  }

  return 0LL;
}


//=============================================================================
//
//  MsgSysCommand() - Handles WM_SYSCOMMAND
//
//
LRESULT MsgSysCommand(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (wParam) {
  case SC_MINIMIZE:
    ShowOwnedPopups(hwnd, false);
    if (bMinimizeToTray) {
      MinimizeWndToTray(hwnd);
      ShowNotifyIcon(hwnd, true);
      SetNotifyIconTitle(hwnd);
      return 0LL; // swallowed
    }
    break;

  case SC_RESTORE:
    {
      LRESULT lrv = DefWindowProc(hwnd, umsg, wParam, lParam);
      ShowOwnedPopups(hwnd, true);
      return(lrv);
    }

  default:
    break;
  }
  return DefWindowProc(hwnd, umsg, wParam, lParam);
}


//=============================================================================
//
//  OpenHotSpotURL()
//
//
void OpenHotSpotURL(DocPos position, bool bForceBrowser)
{
  char const cStyle = SciCall_GetStyleAt(position);

  if (cStyle != (char)Style_GetHotspotStyleID()) { return; }
  if (!(bool)SendMessage(g_hwndEdit, SCI_STYLEGETHOTSPOT, Style_GetHotspotStyleID(), 0)) { return; }

  // get left most position of style
  DocPos pos = position;
  char cNewStyle = cStyle;
  while ((cNewStyle == cStyle) && (--pos > 0)) {
    cNewStyle = SciCall_GetStyleAt(pos);
  }
  DocPos firstPos = (pos != 0) ? (pos + 1) : 0;

  // get right most position of style
  pos = position;
  cNewStyle = cStyle;
  DocPos posTextLength = SciCall_GetTextLength();
  while ((cNewStyle == cStyle) && (++pos < posTextLength)) {
    cNewStyle = SciCall_GetStyleAt(pos);
  }
  DocPos lastPos = pos;
  DocPos length = (lastPos - firstPos);

  if ((length > 0) && (length < XHUGE_BUFFER))
  {
    char chURL[XHUGE_BUFFER] = { '\0' };

    StringCchCopyNA(chURL, XHUGE_BUFFER, SciCall_GetRangePointer(firstPos, length), length);
    StrTrimA(chURL, " \t\n\r");
    
    if (!StringCchLenA(chURL, COUNTOF(chURL))) { return; }

    WCHAR wchURL[HUGE_BUFFER] = { L'\0' };
    MultiByteToWideCharStrg(Encoding_SciCP, chURL, wchURL);

    const WCHAR* chkPreFix = L"file://";
    size_t const len = StringCchLenW(chkPreFix,0);

    if (!bForceBrowser && (StrStrIW(wchURL, chkPreFix) == wchURL))
    {
      WCHAR* szFileName = &(wchURL[len]);
      StrTrimW(szFileName, L"/");

      PathCanonicalizeEx(szFileName, COUNTOF(wchURL) - (int)len);

      if (PathIsDirectory(szFileName))
      {
        WCHAR tchFile[MAX_PATH + 1] = { L'\0' };

        if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), szFileName))
          FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
      }
      else
        FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, szFileName);

    }
    else { // open in web browser

      WCHAR wchDirectory[MAX_PATH+1] = { L'\0' };
      if (StringCchLenW(g_wchCurFile, COUNTOF(g_wchCurFile))) {
        StringCchCopy(wchDirectory, COUNTOF(wchDirectory), g_wchCurFile);
        PathRemoveFileSpec(wchDirectory);
      }

      SHELLEXECUTEINFO sei;
      ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = SEE_MASK_NOZONECHECKS;
      sei.hwnd = NULL;
      sei.lpVerb = NULL;
      sei.lpFile = wchURL;
      sei.lpParameters = NULL;
      sei.lpDirectory = wchDirectory;
      sei.nShow = SW_SHOWNORMAL;
      ShellExecuteEx(&sei);

    }

  }
}



//=============================================================================
//
//  _HandleAutoIndent()
//
static void __fastcall _HandleAutoIndent(int const charAdded) {
  // in CRLF mode handle LF only...
  if (((SC_EOL_CRLF == g_iEOLMode) && (charAdded != '\r')) || (SC_EOL_CRLF != g_iEOLMode))
  {
    DocPos const iCurPos = SciCall_GetCurrentPos();
    DocLn const iCurLine = SciCall_LineFromPosition(iCurPos);

    // Move bookmark along with line if inserting lines (pressing return within indent area of line) because Scintilla does not do this for us
    if (iCurLine > 0)
    {
      //DocPos const iPrevLineLength = Sci_GetNetLineLength(iCurLine - 1);
      if (SciCall_GetLineEndPosition(iCurLine - 1) == SciCall_GetLineIndentPosition(iCurLine - 1))
      {
        int const bitmask = SciCall_MarkerGet(iCurLine - 1);
        if (bitmask & (1 << MARKER_NP3_BOOKMARK))
        {
          SciCall_MarkerDelete(iCurLine - 1, MARKER_NP3_BOOKMARK);
          SciCall_MarkerAdd(iCurLine, MARKER_NP3_BOOKMARK);
        }
      }
    }

    if (iCurLine > 0/* && iLineLength <= 2*/)
    {
      DocPos const iPrevLineLength = SciCall_LineLength(iCurLine - 1);
      char* pLineBuf = NULL;
      bool bAllocLnBuf = false;
      if (iPrevLineLength < TEMPLINE_BUFFER) {
        pLineBuf = g_pTempLineBufferMain;
      }
      else {
        bAllocLnBuf = true;
        pLineBuf = AllocMem(iPrevLineLength + 1, HEAP_ZERO_MEMORY);
      }
      if (pLineBuf)
      {
        SciCall_GetLine(iCurLine - 1, pLineBuf);
        *(pLineBuf + iPrevLineLength) = '\0';
        for (char* pPos = pLineBuf; *pPos; pPos++) {
          if (*pPos != ' ' && *pPos != '\t')
            *pPos = '\0';
        }
        if (*pLineBuf) {
          _BEGIN_UNDO_ACTION_;
          SciCall_AddText(lstrlenA(pLineBuf), pLineBuf);
          _END_UNDO_ACTION_;
        }
        if (bAllocLnBuf) { FreeMem(pLineBuf); }
      }
    }
  }
}


//=============================================================================
//
//  _HandleAutoCloseTags()
//
static void __fastcall _HandleAutoCloseTags()
{
  //int lexerID = (int)SendMessage(g_hwndEdit,SCI_GETLEXER,0,0);
  //if (lexerID == SCLEX_HTML || lexerID == SCLEX_XML)
  {
    DocPos const iCurPos = SciCall_GetCurrentPos();
    DocPos const iHelper = iCurPos - (DocPos)(COUNTOF(g_pTempLineBufferMain) - 1);
    DocPos const iStartPos = max(0, iHelper);
    DocPos const iSize = iCurPos - iStartPos;

    if (iSize >= 3)
    {
      const char* pBegin = SciCall_GetRangePointer(iStartPos, iSize);

      if (pBegin[iSize - 2] != '/') {

        const char* pCur = &pBegin[iSize - 2];

        while (pCur > pBegin && *pCur != '<' && *pCur != '>') { --pCur; }

        int  cchIns = 2;
        StringCchCopyA(g_pTempLineBufferMain, FNDRPL_BUFFER, "</");
        if (*pCur == '<') {
          pCur++;
          while (StrChrA(":_-.", *pCur) || IsCharAlphaNumericA(*pCur)) {
            g_pTempLineBufferMain[cchIns++] = *pCur;
            pCur++;
          }
        }
        g_pTempLineBufferMain[cchIns++] = '>';
        g_pTempLineBufferMain[cchIns] = '\0';

        if (cchIns > 3 &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</base>", -1) &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</bgsound>", -1) &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</br>", -1) &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</embed>", -1) &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</hr>", -1) &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</img>", -1) &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</input>", -1) &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</link>", -1) &&
          StringCchCompareINA(g_pTempLineBufferMain, COUNTOF(g_pTempLineBufferMain), "</meta>", -1))
        {
          _BEGIN_UNDO_ACTION_;
          SciCall_ReplaceSel(g_pTempLineBufferMain);
          SciCall_SetSel(iCurPos, iCurPos);
          _END_UNDO_ACTION_;
        }
      }
    }
  }
}


//=============================================================================
//
//  _HandleTinyExpr() - called on '?' insert
//
static void __fastcall _HandleTinyExpr()
{
  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iPosBefore = SciCall_PositionBefore(iCurPos);
  char const chBefore = SciCall_GetCharAt(iPosBefore - 1);
  if (chBefore == '=') // got "=?" evaluate expression trigger
  {
    DocPos const iLnCaretPos = SciCall_GetCurLine(COUNTOF(g_pTempLineBufferMain), g_pTempLineBufferMain);
    g_pTempLineBufferMain[(iLnCaretPos > 1) ? (iLnCaretPos-2) : 0] = '\0'; // breakbefore "=?"

    int iExprErr = 1;
    const char* pBegin = &g_pTempLineBufferMain[0];
    double dExprEval = 0.0;

    while (*pBegin && iExprErr) {
      dExprEval = te_interp(pBegin++, &iExprErr);
    }
    if (*pBegin && !iExprErr) {
      char chExpr[64] = { '\0' };
      StringCchPrintfA(chExpr, COUNTOF(chExpr), "%.6G", dExprEval);
      SciCall_SetSel(iPosBefore, iCurPos);
      SciCall_ReplaceSel(chExpr);
    }
  }
}


//=============================================================================
//
//  MsgNotify() - Handles WM_NOTIFY
//
//  !!! Set correct SCI_SETMODEVENTMASK in _InitializeSciEditCtrl()
//
LRESULT MsgNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);

  LPNMHDR pnmh = (LPNMHDR)lParam;
  struct SCNotification* scn = (struct SCNotification*)lParam;

  if (!CheckNotifyChangeEvent()) 
  {
    // --- check only mandatory events (must be fast !!!) ---
    if (pnmh->idFrom == IDC_EDIT) {
      if (pnmh->code == SCN_MODIFIED) {
        bool bModified = true;
        int const iModType = scn->modificationType;
        if ((iModType & SC_MOD_BEFOREINSERT) || ((iModType & SC_MOD_BEFOREDELETE))) {
          if (!((iModType & SC_PERFORMED_UNDO) || (iModType & SC_PERFORMED_REDO))) {
            if (!SciCall_IsSelectionEmpty() && !_InUndoRedoTransaction())
              _SaveRedoSelection(_SaveUndoSelection());
          }
          bModified = false; // not yet
        }
        // check for ADDUNDOACTION step
        if (iModType & SC_MOD_CONTAINER)
        {
          if (iModType & SC_PERFORMED_UNDO) {
            RestoreAction(scn->token, UNDO);
          }
          else if (iModType & SC_PERFORMED_REDO) {
            RestoreAction(scn->token, REDO);
          }
        }
        if (bModified) { _SetDocumentModified(true); }
      }
      else if (pnmh->code == SCN_SAVEPOINTREACHED) {
        _SetDocumentModified(false);
      }
      else if (pnmh->code == SCN_SAVEPOINTLEFT) {
        _SetDocumentModified(true);
      }
      else if (pnmh->code == SCN_MODIFYATTEMPTRO) {
        if (EditToggleView(g_hwndEdit, false)) {
          EditToggleView(g_hwndEdit, true);
        }
      }
    }
    return 1;
  }

  // --- check ALL events ---

  switch(pnmh->idFrom)
  {
    case IDC_EDIT:

      switch (pnmh->code)
      {
        case SCN_MODIFIED:
        {
          int const iModType = scn->modificationType;
          bool bModified = true;
          if ((iModType & SC_MOD_BEFOREINSERT) || ((iModType & SC_MOD_BEFOREDELETE))) {
            if (!((iModType & SC_PERFORMED_UNDO) || (iModType & SC_PERFORMED_REDO))) {
              if (!SciCall_IsSelectionEmpty() && !_InUndoRedoTransaction())
                _SaveRedoSelection(_SaveUndoSelection());
            }
            bModified = false; // not yet
          }
          if (iModType & SC_MOD_CONTAINER) {
            if (iModType & SC_PERFORMED_UNDO) {
              RestoreAction(scn->token, UNDO);
            }
            else if (iModType & SC_PERFORMED_REDO) {
              RestoreAction(scn->token, REDO);
            }
          }
          if (bModified) {
            if (g_iMarkOccurrences > 0) {
              MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, true);
            }
            if (scn->linesAdded != 0) {
              UpdateLineNumberWidth();
            }
            _SetDocumentModified(true);
            UpdateToolbar();
            UpdateStatusbar(false);
          }
        }
        break;


        //case SCN_STYLENEEDED:  // this event needs SCI_SETLEXER(SCLEX_CONTAINER)
        //  {
        //    int lineNumber = SciCall_LineFromPosition(SciCall_GetEndStyled());
        //    EditUpdateUrlHotspots(g_hwndEdit, SciCall_PositionFromLine(lineNumber), (int)scn->position, bHyperlinkHotspot);
        //    EditUpdateHiddenLineRange(hwnd, &g_efrData, 0, SciCall_GetLineCount());
        //  }
        //  break;

        case SCN_UPDATEUI:
        {
          int const iUpd = scn->updated;
          //if (scn->updated & SC_UPDATE_NP3_INTERNAL_NOTIFY) {
          //  // special case
          //}
          //else

          if (iUpd & (SC_UPDATE_SELECTION | SC_UPDATE_CONTENT))
          {
            //~InvalidateSelections(); // fixed in SCI ?

            // Brace Match
            if (bMatchBraces) {
              EditMatchBrace(g_hwndEdit);
            }

            if (g_iMarkOccurrences > 0) {
              // clear marks only, if selection changed
              if (iUpd & SC_UPDATE_SELECTION)
              {
                if (!SciCall_IsSelectionEmpty()) {
                  MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, true);
                }
                else {
                  EditClearAllOccurrenceMarkers(g_hwndEdit);
                }
              }
              else if (iUpd & SC_UPDATE_CONTENT) {
                // ignoring SC_UPDATE_CONTENT cause Style and Marker are out of scope here
                // using WM_COMMAND -> SCEN_CHANGE  instead!
                //~MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, false);
              }
            }

            if (g_bHyperlinkHotspot) {
              UpdateVisibleUrlHotspot(iUpdateDelayHyperlinkStyling);
            }
            UpdateToolbar();
            UpdateStatusbar(false);
          }
          else if (iUpd & SC_UPDATE_V_SCROLL)
          {
            if ((g_iMarkOccurrences > 0) && g_bMarkOccurrencesMatchVisible) {
              MarkAllOccurrences(iUpdateDelayMarkAllCoccurrences, false);
            }
            if (g_bHyperlinkHotspot) {
              UpdateVisibleUrlHotspot(iUpdateDelayHyperlinkStyling);
            }
          }
        }
        break;


        case SCN_HOTSPOTCLICK:
        {
          if (scn->modifiers & SCMOD_CTRL) {
            // open in browser
            OpenHotSpotURL((int)scn->position, true);
          }
          if (scn->modifiers & SCMOD_ALT) {
            // open in application, if applicable (file://)
            OpenHotSpotURL((int)scn->position, false);
          }
        }
        break;


        case SCN_CHARADDED:
          {
            if (g_CallTipType != CT_NONE) { 
              SciCall_CallTipCancel();   
              g_CallTipType = CT_NONE;
            }

            // Auto indent
            if (bAutoIndent && (scn->ch == '\r' || scn->ch == '\n'))
            {
              _HandleAutoIndent(scn->ch);
            }
            // Auto close tags
            else if (bAutoCloseTags && scn->ch == '>')
            {
              _HandleAutoCloseTags();
            }
            else if (scn->ch == '?') {
              _HandleTinyExpr();
            }
            else if (g_bAutoCompleteWords && !SciCall_AutoCActive() && !_IsInlineIMEActive()) {
              EditCompleteWord(g_hwndEdit, false);
            }
          }
          break;


        case SCN_NEEDSHOWN:
          {
            DocLn iFirstLine = SciCall_LineFromPosition((DocPos)scn->position);
            DocLn iLastLine = SciCall_LineFromPosition((DocPos)(scn->position + scn->length - 1));
            for (DocLn i = iFirstLine; i <= iLastLine; ++i) { SciCall_EnsureVisible(i); }
          }
          break;


        case SCN_MARGINCLICK:
          if (scn->margin == MARGIN_SCI_FOLDING) {
            EditFoldClick(SciCall_LineFromPosition((DocPos)scn->position), scn->modifiers);
          }
          break;


        // ~~~ Not used in Windows ~~~
        // see: CMD_ALTUP / CMD_ALTDOWN
        //case SCN_KEY:
        //  // Also see the corresponding patch in scintilla\src\Editor.cxx
        //  FoldAltArrow(scn->ch, scn->modifiers);
        //  break;


        case SCN_SAVEPOINTREACHED:
          _SetDocumentModified(false);
          break;


        case SCN_SAVEPOINTLEFT:
          _SetDocumentModified(true);
          break;


        case SCN_ZOOM:
          UpdateLineNumberWidth();
          break;

        case SCN_URIDROPPED: 
          {
            // see WM_DROPFILES
            WCHAR szBuf[MAX_PATH + 40];
            if (MultiByteToWideChar(CP_UTF8, 0, scn->text, -1, szBuf, COUNTOF(szBuf)) > 0) 
            {
              if (IsIconic(hwnd)) {
                ShowWindow(hwnd, SW_RESTORE);
              }
              //SetForegroundWindow(hwnd);
              if (PathIsDirectory(szBuf)) {
                WCHAR tchFile[MAX_PATH];
                if (OpenFileDlg(g_hwndMain, tchFile, COUNTOF(tchFile), szBuf)) {
                  FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, tchFile);
                }
              }
              else if (PathFileExists(szBuf)) {
                FileLoad(false, false, false, bSkipUnicodeDetection, bSkipANSICodePageDetection, szBuf);
              }
            }
          }
          break;

        default:
          return 0LL;
      }
      return -1LL;


    case IDC_TOOLBAR:

      switch(pnmh->code)
      {
        case TBN_ENDADJUST:
          UpdateToolbar();
          break;

        case TBN_QUERYDELETE:
        case TBN_QUERYINSERT:
          break;

        case TBN_GETBUTTONINFO:
          {
            if (((LPTBNOTIFY)lParam)->iItem < COUNTOF(tbbMainWnd))
            {
              WCHAR tch[SMALL_BUFFER] = { L'\0' };
              GetLngString(tbbMainWnd[((LPTBNOTIFY)lParam)->iItem].idCommand,tch,COUNTOF(tch));
              StringCchCopyN(((LPTBNOTIFY)lParam)->pszText,((LPTBNOTIFY)lParam)->cchText,tch,((LPTBNOTIFY)lParam)->cchText);
              CopyMemory(&((LPTBNOTIFY)lParam)->tbButton,&tbbMainWnd[((LPTBNOTIFY)lParam)->iItem],sizeof(TBBUTTON));
              return 1LL;
            }
          }
          return 0LL;

        case TBN_RESET:
          {
            int i; int c = (int)SendMessage(g_hwndToolbar,TB_BUTTONCOUNT,0,0);
            for (i = 0; i < c; i++) {
              SendMessage(g_hwndToolbar, TB_DELETEBUTTON, 0, 0);
            }
            SendMessage(g_hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)tbbMainWnd);
          }
          return 0LL;

        default:
          return 0LL;
      }
      return 1LL;


    case IDC_STATUSBAR:

      switch(pnmh->code)
      {

        case NM_CLICK:
          {
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;

            switch (pnmm->dwItemSpec)
            {
              case STATUS_EOLMODE:
                SendMessage(g_hwndEdit,SCI_CONVERTEOLS, SciCall_GetEOLMode(),0);
                EditFixPositions(g_hwndEdit);
                return 1LL;

              default:
                return 0LL;
            }
          }
          break;

        case NM_DBLCLK:
          {
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;

            switch (g_vSBSOrder[pnmm->dwItemSpec])
            {
              case STATUS_DOCLINE:
              case STATUS_DOCCOLUMN:
                PostMessage(hwnd, WM_COMMAND, MAKELONG(IDM_EDIT_GOTOLINE,1),0);
                return 1LL;

              case STATUS_CODEPAGE:
                PostMessage(hwnd,WM_COMMAND,MAKELONG(IDM_ENCODING_SELECT,1),0);
                return 1LL;

              case STATUS_EOLMODE:
                {
                  int i;
                  if (g_iEOLMode == SC_EOL_CRLF)
                    i = IDM_LINEENDINGS_CRLF;
                  else if (g_iEOLMode == SC_EOL_LF)
                    i = IDM_LINEENDINGS_LF;
                  else
                    i = IDM_LINEENDINGS_CR;
                  i++;
                  if (i > IDM_LINEENDINGS_CR) { i = IDM_LINEENDINGS_CRLF; }
                  PostMessage(hwnd, WM_COMMAND, MAKELONG(i, 1), 0);
                }
                return 1LL;

              case STATUS_OVRMODE:
                PostMessage(hwnd, WM_COMMAND, MAKELONG(CMD_VK_INSERT, 1), 0);
                return 1LL;

              case STATUS_2ND_DEF:
                PostMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_USE2NDDEFAULT, 1), 0);
                return 1LL;

              case STATUS_LEXER:
                PostMessage(hwnd, WM_COMMAND, MAKELONG(IDM_VIEW_SCHEME, 1), 0);
                return 1LL;

              case STATUS_TINYEXPR:
                {
                  char chBuf[80];
                  if (g_iExprError == 0) {
                    StringCchPrintfA(chBuf, COUNTOF(chBuf), "%.6G", g_dExpression);
                    SciCall_CopyText((DocPos)StringCchLenA(chBuf,80), chBuf);
                  }
                  else if (g_iExprError > 0) {
                    StringCchPrintfA(chBuf, COUNTOF(chBuf), "^[%i]", g_iExprError);
                    SciCall_CopyText((DocPos)StringCchLenA(chBuf,80), chBuf);
                  }
                  else
                    SciCall_CopyText(0, "");
                }
                break;

              default:
                return 0LL;
            }
          }
          break;

        default:
          break;
      }
      break;


    default:
      switch(pnmh->code)
      {
        case TTN_NEEDTEXT:
          {
            if (!(((LPTOOLTIPTEXT)lParam)->uFlags & TTF_IDISHWND))
            {
              WCHAR tch[MIDSZ_BUFFER] = { L'\0' };
              GetLngString((UINT)pnmh->idFrom,tch,COUNTOF(tch));
              StringCchCopyN(((LPTOOLTIPTEXT)lParam)->szText,COUNTOF(((LPTOOLTIPTEXT)lParam)->szText),tch,COUNTOF(((LPTOOLTIPTEXT)lParam)->szText));
            }
          }
          break;
          
        default:
          break;
      }
      break;

  }
  return 0LL;
}



//=============================================================================
//
//  Set/Get FindPattern()
// 
static WCHAR sCurrentFindPattern[FNDRPL_BUFFER] = { L'\0' };

bool IsFindPatternEmpty()
{
  return (StringCchLenW(sCurrentFindPattern, COUNTOF(sCurrentFindPattern)) == 0);
}

//=============================================================================
//
//  SetFindPattern()
// 
void SetFindPattern(LPCWSTR wchFindPattern)
{
  StringCchCopyW(sCurrentFindPattern, COUNTOF(sCurrentFindPattern), (wchFindPattern ? wchFindPattern : L""));
}

//=============================================================================
//
//  SetFindPatternMB()
// 
void SetFindPatternMB(LPCSTR chFindPattern)
{
  MultiByteToWideChar(Encoding_SciCP, 0, chFindPattern, -1, sCurrentFindPattern, COUNTOF(sCurrentFindPattern));
}

//=============================================================================
//
//  GetFindPattern()
// 
void GetFindPattern(LPWSTR wchFindPattern, size_t bufferSize)
{
  StringCchCopyW(wchFindPattern, bufferSize, sCurrentFindPattern);
}

//=============================================================================
//
//  GetFindPatternMB()
// 
void GetFindPatternMB(LPSTR chFindPattern, size_t bufferSize)
{
  WideCharToMultiByte(Encoding_SciCP, 0, sCurrentFindPattern, -1, chFindPattern, (int)bufferSize, NULL, NULL);
}


//=============================================================================
//
//  LoadSettings()
//
//
void LoadSettings()
{
  WCHAR *pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * INISECTIONBUFCNT * HUGE_BUFFER);
  if (pIniSection) 
  {
    int const cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);

    g_iSettingsVersion = IniGetInt(L"Settings", L"SettingsVersion", CFG_VER_NONE);

    g_bEnableSaveSettings = true; // false: if settings-file is loaded in editor
    g_bSaveSettings = IniGetBool(L"Settings", L"SaveSettings", true);


    // first load "hard coded" .ini-Settings
    // --------------------------------------------------------------------------
    LoadIniSection(L"Settings2", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    IniSectionGetString(pIniSection, L"PreferredLanguageLocaleName", L"",
                        g_tchPrefLngLocName, COUNTOF(g_tchPrefLngLocName));

    g_IMEInteraction = IniSectionGetInt(pIniSection, L"IMEInteraction", 0);
    g_IMEInteraction = clampi(g_IMEInteraction, 0, 1);

    g_bStickyWinPos = IniSectionGetBool(pIniSection, L"StickyWindowPosition", false);

    IniSectionGetString(pIniSection, L"DefaultExtension", L"txt", g_tchDefaultExtension, COUNTOF(g_tchDefaultExtension));
    StrTrim(g_tchDefaultExtension, L" \t.\"");

    IniSectionGetString(pIniSection, L"DefaultDirectory", L"", g_tchDefaultDir, COUNTOF(g_tchDefaultDir));

    ZeroMemory(g_tchFileDlgFilters, sizeof(WCHAR)*COUNTOF(g_tchFileDlgFilters));
    IniSectionGetString(pIniSection, L"FileDlgFilters", L"", g_tchFileDlgFilters, COUNTOF(g_tchFileDlgFilters) - 2);

    dwFileCheckInverval = IniSectionGetInt(pIniSection, L"FileCheckInverval", 2000);
    dwFileCheckInverval = clampul(dwFileCheckInverval, 250, 300000);

    dwAutoReloadTimeout = IniSectionGetInt(pIniSection, L"AutoReloadTimeout", 2000);
    dwAutoReloadTimeout = clampul(dwAutoReloadTimeout, 250, 300000);

    // deprecated
    g_iRenderingTechnology = IniSectionGetInt(pIniSection, L"SciDirectWriteTech", -111);
    if ((g_iRenderingTechnology != -111) && g_bSaveSettings) {
      // cleanup
      IniSetString(L"Settings2", L"SciDirectWriteTech", NULL);
      IniSetInt(L"Settings", L"RenderingTechnology", g_iRenderingTechnology);
    }
    g_iRenderingTechnology = clampi(g_iRenderingTechnology, 0, 3);

    // deprecated
    g_iBidirectional = IniSectionGetInt(pIniSection, L"EnableBidirectionalSupport", -111);
    if ((g_iBidirectional != -111) && g_bSaveSettings) {
      // cleanup
      IniSetString(L"Settings2", L"EnableBidirectionalSupport", NULL);
      IniSetInt(L"Settings", L"Bidirectional", g_iBidirectional);
    }
    g_iBidirectional = (clampi(g_iBidirectional, 0, 2) > 0) ? 2 : 0;

    g_iSciFontQuality = IniSectionGetInt(pIniSection, L"SciFontQuality", FontQuality[3]);
    g_iSciFontQuality = clampi(g_iSciFontQuality, 0, 3);

    g_iMarkOccurrencesMaxCount = IniSectionGetInt(pIniSection, L"MarkOccurrencesMaxCount", 2000);
    g_iMarkOccurrencesMaxCount = (g_iMarkOccurrencesMaxCount <= 0) ? INT_MAX : g_iMarkOccurrencesMaxCount;

    iUpdateDelayHyperlinkStyling = IniSectionGetInt(pIniSection, L"UpdateDelayHyperlinkStyling", 100);
    iUpdateDelayHyperlinkStyling = clampi(iUpdateDelayHyperlinkStyling, USER_TIMER_MINIMUM, 10000);

    iUpdateDelayMarkAllCoccurrences = IniSectionGetInt(pIniSection, L"UpdateDelayMarkAllCoccurrences", 50);
    iUpdateDelayMarkAllCoccurrences = clampi(iUpdateDelayMarkAllCoccurrences, USER_TIMER_MINIMUM, 10000);

    g_bDenyVirtualSpaceAccess = IniSectionGetBool(pIniSection, L"DenyVirtualSpaceAccess", false);
    g_bUseOldStyleBraceMatching = IniSectionGetBool(pIniSection, L"UseOldStyleBraceMatching", false);

    iCurrentLineHorizontalSlop = IniSectionGetInt(pIniSection, L"CurrentLineHorizontalSlop", 40);
    iCurrentLineHorizontalSlop = clampi(iCurrentLineHorizontalSlop, 0, 2000);

    iCurrentLineVerticalSlop = IniSectionGetInt(pIniSection, L"CurrentLineVerticalSlop", 5);
    iCurrentLineVerticalSlop = clampi(iCurrentLineVerticalSlop, 0, 200);

    IniSectionGetString(pIniSection, L"AdministrationTool.exe", L"", g_tchAdministrationExe, COUNTOF(g_tchAdministrationExe));


    // --------------------------------------------------------------------------
    LoadIniSection(L"Settings", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    g_bSaveRecentFiles = IniSectionGetBool(pIniSection, L"SaveRecentFiles", true);
    g_bPreserveCaretPos = IniSectionGetBool(pIniSection, L"PreserveCaretPos", false);
    g_bSaveFindReplace = IniSectionGetBool(pIniSection, L"SaveFindReplace", false);

    g_efrData.bFindClose = IniSectionGetBool(pIniSection, L"CloseFind", false);
    g_efrData.bReplaceClose = IniSectionGetBool(pIniSection, L"CloseReplace", false);
    g_efrData.bNoFindWrap = IniSectionGetBool(pIniSection, L"NoFindWrap", false);
    g_efrData.bTransformBS = IniSectionGetBool(pIniSection, L"FindTransformBS", false);
    g_efrData.bWildcardSearch = IniSectionGetBool(pIniSection, L"WildcardSearch", false);
    g_efrData.bMarkOccurences = IniSectionGetBool(pIniSection, L"FindMarkAllOccurrences", false);
    g_efrData.bHideNonMatchedLines = IniSectionGetBool(pIniSection, L"HideNonMatchedLines", false);
    g_efrData.bDotMatchAll = IniSectionGetBool(pIniSection, L"RegexDotMatchesAll", false);
    g_efrData.fuFlags = IniSectionGetUInt(pIniSection, L"efrData_fuFlags", 0);

    if (!IniSectionGetString(pIniSection, L"OpenWithDir", L"", g_tchOpenWithDir, COUNTOF(g_tchOpenWithDir))) {
      //SHGetSpecialFolderPath(NULL, g_tchOpenWithDir, CSIDL_DESKTOPDIRECTORY, true);
      GetKnownFolderPath(&FOLDERID_Desktop, g_tchOpenWithDir, COUNTOF(g_tchOpenWithDir));
    }
    else {
      PathAbsoluteFromApp(g_tchOpenWithDir, NULL, COUNTOF(g_tchOpenWithDir), true);
    }
    if (!IniSectionGetString(pIniSection, L"Favorites", L"", g_tchFavoritesDir, COUNTOF(g_tchFavoritesDir))) {
      //SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,g_tchFavoritesDir);
      GetKnownFolderPath(&FOLDERID_Favorites, g_tchFavoritesDir, COUNTOF(g_tchFavoritesDir));
    }
    else {
      PathAbsoluteFromApp(g_tchFavoritesDir, NULL, COUNTOF(g_tchFavoritesDir), true);
    }

    iPathNameFormat = IniSectionGetInt(pIniSection, L"PathNameFormat", 1);
    iPathNameFormat = clampi(iPathNameFormat, 0, 2);

    g_bWordWrap = IniSectionGetBool(pIniSection, L"WordWrap", false);
    bWordWrapG = g_bWordWrap;

    iWordWrapMode = IniSectionGetInt(pIniSection, L"WordWrapMode", 0);
    iWordWrapMode = clampi(iWordWrapMode, 0, 1);

    iWordWrapIndent = IniSectionGetInt(pIniSection, L"WordWrapIndent", 0);
    iWordWrapIndent = clampi(iWordWrapIndent, 0, 6);

    iWordWrapSymbols = IniSectionGetInt(pIniSection, L"WordWrapSymbols", 22);
    iWordWrapSymbols = clampi(iWordWrapSymbols % 10, 0, 2) +
      clampi((iWordWrapSymbols % 100 - iWordWrapSymbols % 10) / 10, 0, 2) * 10;

    bShowWordWrapSymbols = IniSectionGetBool(pIniSection, L"ShowWordWrapSymbols", 0);

    bMatchBraces = IniSectionGetBool(pIniSection, L"MatchBraces", true);

    bAutoCloseTags = IniSectionGetBool(pIniSection, L"AutoCloseTags", false);

    bHiliteCurrentLine = IniSectionGetBool(pIniSection, L"HighlightCurrentLine", false);

    g_bHyperlinkHotspot = IniSectionGetBool(pIniSection, L"HyperlinkHotspot", false);

    bScrollPastEOF = IniSectionGetBool(pIniSection, L"ScrollPastEOF", false);

    bAutoIndent = IniSectionGetBool(pIniSection, L"AutoIndent", true);

    g_bAutoCompleteWords = IniSectionGetBool(pIniSection, L"AutoCompleteWords", false);

    g_bAccelWordNavigation = IniSectionGetBool(pIniSection, L"AccelWordNavigation", false);

    bShowIndentGuides = IniSectionGetBool(pIniSection, L"ShowIndentGuides", false);

    g_bTabsAsSpaces = IniSectionGetBool(pIniSection, L"TabsAsSpaces", false);
    bTabsAsSpacesG = g_bTabsAsSpaces;

    g_bTabIndents = IniSectionGetBool(pIniSection, L"TabIndents", true);
    bTabIndentsG = g_bTabIndents;

    bBackspaceUnindents = IniSectionGetBool(pIniSection, L"BackspaceUnindents", false);

    g_iTabWidth = IniSectionGetInt(pIniSection, L"TabWidth", 4);
    g_iTabWidth = clampi(g_iTabWidth, 1, 256);
    iTabWidthG = g_iTabWidth;

    g_iIndentWidth = IniSectionGetInt(pIniSection, L"IndentWidth", 0);
    g_iIndentWidth = clampi(g_iIndentWidth, 0, 256);
    iIndentWidthG = g_iIndentWidth;

    g_bMarkLongLines = IniSectionGetBool(pIniSection, L"MarkLongLines", true);

    g_iLongLinesLimit = IniSectionGetInt(pIniSection, L"LongLinesLimit", 80);
    g_iLongLinesLimit = clampi(g_iLongLinesLimit, 0, 4096);
    iLongLinesLimitG = g_iLongLinesLimit;

    iLongLineMode = IniSectionGetInt(pIniSection, L"LongLineMode", EDGE_LINE);
    iLongLineMode = clampi(iLongLineMode, EDGE_LINE, EDGE_BACKGROUND);

    g_bShowSelectionMargin = IniSectionGetBool(pIniSection, L"ShowSelectionMargin", false);

    g_bShowLineNumbers = IniSectionGetBool(pIniSection, L"ShowLineNumbers", true);

    g_bShowCodeFolding = IniSectionGetBool(pIniSection, L"ShowCodeFolding", true);

    g_iMarkOccurrences = IniSectionGetInt(pIniSection, L"MarkOccurrences", 1);
    g_iMarkOccurrences = clampi(g_iMarkOccurrences, 0, 3);

    g_bMarkOccurrencesMatchVisible = IniSectionGetBool(pIniSection, L"MarkOccurrencesMatchVisible", false);
    g_bMarkOccurrencesMatchCase = IniSectionGetBool(pIniSection, L"MarkOccurrencesMatchCase", false);
    g_bMarkOccurrencesMatchWords = IniSectionGetBool(pIniSection, L"MarkOccurrencesMatchWholeWords", true);
    g_bMarkOccurrencesCurrentWord = IniSectionGetBool(pIniSection, L"MarkOccurrencesCurrentWord", !g_bMarkOccurrencesMatchWords);
    g_bMarkOccurrencesCurrentWord = g_bMarkOccurrencesCurrentWord && !g_bMarkOccurrencesMatchWords;

    bViewWhiteSpace = IniSectionGetBool(pIniSection, L"ViewWhiteSpace", false);

    bViewEOLs = IniSectionGetBool(pIniSection, L"ViewEOLs", false);

    g_iDefaultNewFileEncoding = IniSectionGetInt(pIniSection, L"DefaultEncoding", CPI_NONE);
    // if DefaultEncoding is not defined set to system's current code-page 
    g_iDefaultNewFileEncoding = (g_iDefaultNewFileEncoding == CPI_NONE) ?
      Encoding_MapIniSetting(true, (int)GetACP()) : Encoding_MapIniSetting(true, g_iDefaultNewFileEncoding);

    bUseDefaultForFileEncoding = IniSectionGetBool(pIniSection, L"UseDefaultForFileEncoding", false);

    bSkipUnicodeDetection = IniSectionGetBool(pIniSection, L"SkipUnicodeDetection", false);

    bSkipANSICodePageDetection = IniSectionGetBool(pIniSection, L"SkipANSICodePageDetection", true);

    bLoadASCIIasUTF8 = IniSectionGetBool(pIniSection, L"LoadASCIIasUTF8", false);

    bLoadNFOasOEM = IniSectionGetBool(pIniSection, L"LoadNFOasOEM", true);

    bNoEncodingTags = IniSectionGetBool(pIniSection, L"NoEncodingTags", false);

    g_iDefaultEOLMode = IniSectionGetInt(pIniSection, L"DefaultEOLMode", 0);
    g_iDefaultEOLMode = clampi(g_iDefaultEOLMode, 0, 2);

    bFixLineEndings = IniSectionGetBool(pIniSection, L"FixLineEndings", false);

    bAutoStripBlanks = IniSectionGetBool(pIniSection, L"FixTrailingBlanks", false);

    iPrintHeader = IniSectionGetInt(pIniSection, L"PrintHeader", 1);
    iPrintHeader = clampi(iPrintHeader, 0, 3);

    iPrintFooter = IniSectionGetInt(pIniSection, L"PrintFooter", 0);
    iPrintFooter = clampi(iPrintFooter, 0, 1);

    iPrintColor = IniSectionGetInt(pIniSection, L"PrintColorMode", 3);
    iPrintColor = clampi(iPrintColor, 0, 4);

    iPrintZoom = IniSectionGetInt(pIniSection, L"PrintZoom", (g_iSettingsVersion < CFG_VER_0001) ? 10 : 100);
    if (g_iSettingsVersion < CFG_VER_0001) { iPrintZoom = 100 + (iPrintZoom-10) * 10; }
    iPrintZoom = clampi(iPrintZoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

    pagesetupMargin.left = IniSectionGetInt(pIniSection, L"PrintMarginLeft", -1);
    pagesetupMargin.left = max(pagesetupMargin.left, -1);

    pagesetupMargin.top = IniSectionGetInt(pIniSection, L"PrintMarginTop", -1);
    pagesetupMargin.top = max(pagesetupMargin.top, -1);

    pagesetupMargin.right = IniSectionGetInt(pIniSection, L"PrintMarginRight", -1);
    pagesetupMargin.right = max(pagesetupMargin.right, -1);

    pagesetupMargin.bottom = IniSectionGetInt(pIniSection, L"PrintMarginBottom", -1);
    pagesetupMargin.bottom = max(pagesetupMargin.bottom, -1);

    bSaveBeforeRunningTools = IniSectionGetBool(pIniSection, L"SaveBeforeRunningTools", false);

    g_iFileWatchingMode = IniSectionGetInt(pIniSection, L"FileWatchingMode", 0);
    g_iFileWatchingMode = clampi(g_iFileWatchingMode, 0, 2);

    g_bResetFileWatching = IniSectionGetBool(pIniSection, L"ResetFileWatching", true);

    iEscFunction = IniSectionGetInt(pIniSection, L"EscFunction", 0);
    iEscFunction = clampi(iEscFunction, 0, 2);

    bAlwaysOnTop = IniSectionGetBool(pIniSection, L"AlwaysOnTop", false);

    bMinimizeToTray = IniSectionGetBool(pIniSection, L"MinimizeToTray", false);

    bTransparentMode = IniSectionGetBool(pIniSection, L"TransparentMode", false);

    g_iRenderingTechnology = IniSectionGetInt(pIniSection, L"RenderingTechnology", g_iRenderingTechnology);
    g_iRenderingTechnology = clampi(g_iRenderingTechnology, 0, 3);

    g_iBidirectional = IniSectionGetInt(pIniSection, L"Bidirectional", g_iBidirectional);
    g_iBidirectional = clampi(g_iBidirectional, 0, 2);

    // see TBBUTTON  tbbMainWnd[] for initial/reset set of buttons
    IniSectionGetString(pIniSection, L"ToolbarButtons", L"", g_tchToolbarButtons, COUNTOF(g_tchToolbarButtons));

    bShowToolbar = IniSectionGetBool(pIniSection, L"ShowToolbar", true);

    bShowStatusbar = IniSectionGetBool(pIniSection, L"ShowStatusbar", true);

    cxEncodingDlg = IniSectionGetInt(pIniSection, L"EncodingDlgSizeX", 256);
    cxEncodingDlg = max(cxEncodingDlg, 0);

    cyEncodingDlg = IniSectionGetInt(pIniSection, L"EncodingDlgSizeY", 262);
    cyEncodingDlg = max(cyEncodingDlg, 0);

    cxRecodeDlg = IniSectionGetInt(pIniSection, L"RecodeDlgSizeX", 256);
    cxRecodeDlg = max(cxRecodeDlg, 0);

    cyRecodeDlg = IniSectionGetInt(pIniSection, L"RecodeDlgSizeY", 262);
    cyRecodeDlg = max(cyRecodeDlg, 0);

    cxFileMRUDlg = IniSectionGetInt(pIniSection, L"FileMRUDlgSizeX", 412);
    cxFileMRUDlg = max(cxFileMRUDlg, 0);

    cyFileMRUDlg = IniSectionGetInt(pIniSection, L"FileMRUDlgSizeY", 376);
    cyFileMRUDlg = max(cyFileMRUDlg, 0);

    cxOpenWithDlg = IniSectionGetInt(pIniSection, L"OpenWithDlgSizeX", 384);
    cxOpenWithDlg = max(cxOpenWithDlg, 0);

    cyOpenWithDlg = IniSectionGetInt(pIniSection, L"OpenWithDlgSizeY", 386);
    cyOpenWithDlg = max(cyOpenWithDlg, 0);

    cxFavoritesDlg = IniSectionGetInt(pIniSection, L"FavoritesDlgSizeX", 334);
    cxFavoritesDlg = max(cxFavoritesDlg, 0);

    cyFavoritesDlg = IniSectionGetInt(pIniSection, L"FavoritesDlgSizeY", 316);
    cyFavoritesDlg = max(cyFavoritesDlg, 0);

    xFindReplaceDlg = IniSectionGetInt(pIniSection, L"FindReplaceDlgPosX", 0);
    yFindReplaceDlg = IniSectionGetInt(pIniSection, L"FindReplaceDlgPosY", 0);

    xCustomSchemesDlg = IniSectionGetInt(pIniSection, L"CustomSchemesDlgPosX", 0);
    yCustomSchemesDlg = IniSectionGetInt(pIniSection, L"CustomSchemesDlgPosY", 0);

    // --------------------------------------------------------------------------
    LoadIniSection(L"Statusbar Settings", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    WCHAR tchStatusBar[MIDSZ_BUFFER] = { L'\0' };

    IniSectionGetString(pIniSection, L"SectionPrefixes", STATUSBAR_SECTION_PREFIXES, tchStatusBar, COUNTOF(tchStatusBar));
    ReadStrgsFromCSV(tchStatusBar, g_mxSBPrefix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_PRFX_");

    IniSectionGetString(pIniSection, L"SectionPostfixes", STATUSBAR_SECTION_POSTFIXES, tchStatusBar, COUNTOF(tchStatusBar));
    ReadStrgsFromCSV(tchStatusBar, g_mxSBPostfix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_POFX_");

    IniSectionGetString(pIniSection, L"VisibleSections", STATUSBAR_DEFAULT_IDS, tchStatusBar, COUNTOF(tchStatusBar));
    ReadVectorFromString(tchStatusBar, g_iStatusbarSections, STATUS_SECTOR_COUNT, 0, (STATUS_SECTOR_COUNT - 1), -1);

    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      g_iStatusbarVisible[i] = false;
    }
    int cnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      g_vSBSOrder[i] = -1;
      int const id = g_iStatusbarSections[i];
      if (id >= 0) {
        g_vSBSOrder[cnt++] = id;
        g_iStatusbarVisible[id] = true;
      }
    }

    IniSectionGetString(pIniSection, L"SectionWidthSpecs", STATUSBAR_SECTION_WIDTH_SPECS, tchStatusBar, COUNTOF(tchStatusBar));
    ReadVectorFromString(tchStatusBar, g_iStatusbarWidthSpec, STATUS_SECTOR_COUNT, -4096, 4096, 0);

    g_bZeroBasedColumnIndex = IniSectionGetBool(pIniSection, L"ZeroBasedColumnIndex", false);
    g_bZeroBasedCharacterCount = IniSectionGetBool(pIniSection, L"ZeroBasedCharacterCount", false);


    // --------------------------------------------------------------------------
    LoadIniSection(L"Toolbar Images", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    IniSectionGetString(pIniSection, L"BitmapDefault", L"",
                        g_tchToolbarBitmap, COUNTOF(g_tchToolbarBitmap));
    IniSectionGetString(pIniSection, L"BitmapHot", L"",
                        g_tchToolbarBitmapHot, COUNTOF(g_tchToolbarBitmap));
    IniSectionGetString(pIniSection, L"BitmapDisabled", L"",
                        g_tchToolbarBitmapDisabled, COUNTOF(g_tchToolbarBitmap));

    int ResX = GetSystemMetrics(SM_CXSCREEN);
    int ResY = GetSystemMetrics(SM_CYSCREEN);


    // --------------------------------------------------------------------------
    LoadIniSection(L"Window", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    WCHAR tchHighDpiToolBar[32] = { L'\0' };
    StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i HighDpiToolBar", ResX, ResY);
    iHighDpiToolBar = IniSectionGetInt(pIniSection, tchHighDpiToolBar, -1);
    iHighDpiToolBar = clampi(iHighDpiToolBar, -1, 1);
    if (iHighDpiToolBar < 0) { // undefined: determine high DPI (higher than Full-HD)
      iHighDpiToolBar = ((ResX > 1920) && (ResY > 1080)) ? 1 : 0;
    }

    if (!g_flagPosParam /*|| g_bStickyWinPos*/) { // ignore window position if /p was specified

      WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32], tchZoom[32];

      StringCchPrintf(tchPosX, COUNTOF(tchPosX), L"%ix%i PosX", ResX, ResY);
      StringCchPrintf(tchPosY, COUNTOF(tchPosY), L"%ix%i PosY", ResX, ResY);
      StringCchPrintf(tchSizeX, COUNTOF(tchSizeX), L"%ix%i SizeX", ResX, ResY);
      StringCchPrintf(tchSizeY, COUNTOF(tchSizeY), L"%ix%i SizeY", ResX, ResY);
      StringCchPrintf(tchMaximized, COUNTOF(tchMaximized), L"%ix%i Maximized", ResX, ResY);
      StringCchPrintf(tchZoom, COUNTOF(tchZoom), L"%ix%i Zoom", ResX, ResY);

      g_WinInfo.x = IniSectionGetInt(pIniSection, tchPosX, (INT_MAX >> 1));
      g_WinInfo.y = IniSectionGetInt(pIniSection, tchPosY, (INT_MAX >> 1));
      g_WinInfo.cx = IniSectionGetInt(pIniSection, tchSizeX, (INT_MAX >> 1));
      g_WinInfo.cy = IniSectionGetInt(pIniSection, tchSizeY, (INT_MAX >> 1));
      g_WinInfo.max = IniSectionGetInt(pIniSection, tchMaximized, 0);
      g_WinInfo.max = clampi(g_WinInfo.max, 0, 1);
      g_WinInfo.zoom = IniSectionGetInt(pIniSection, tchZoom, (g_iSettingsVersion < CFG_VER_0001) ? 0 : 100);
      if (g_iSettingsVersion < CFG_VER_0001) { g_WinInfo.zoom = (g_WinInfo.zoom + 10) * 10; }
      g_WinInfo.zoom = clampi(g_WinInfo.zoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

      if (((g_WinInfo.x  & ~CW_USEDEFAULT) == (INT_MAX >> 1)) ||
          ((g_WinInfo.y  & ~CW_USEDEFAULT) == (INT_MAX >> 1)) ||
          ((g_WinInfo.cx & ~CW_USEDEFAULT) == (INT_MAX >> 1)) ||
          ((g_WinInfo.cy & ~CW_USEDEFAULT) == (INT_MAX >> 1))) {
        g_flagDefaultPos = 2; // std. default position (CmdLn: /pd)
      }
    }

    // ---  override by resolution specific settings  ---
    WCHAR tchSciDirectWriteTech[64];
    StringCchPrintf(tchSciDirectWriteTech, COUNTOF(tchSciDirectWriteTech), L"%ix%i RenderingTechnology", ResX, ResY);
    g_iRenderingTechnology = IniSectionGetInt(pIniSection, tchSciDirectWriteTech, g_iRenderingTechnology);
    g_iRenderingTechnology = clampi(g_iRenderingTechnology, 0, 3);

    WCHAR tchSciFontQuality[64];
    StringCchPrintf(tchSciFontQuality, COUNTOF(tchSciFontQuality), L"%ix%i SciFontQuality", ResX, ResY);
    g_iSciFontQuality = IniSectionGetInt(pIniSection, tchSciFontQuality, g_iSciFontQuality);
    g_iSciFontQuality = clampi(g_iSciFontQuality, 0, 3);

    LocalFree(pIniSection);
  }

  // define scintilla internal codepage
  const int iSciDefaultCodePage = SC_CP_UTF8; // default UTF8

  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  if (g_iDefaultNewFileEncoding == CPI_ANSI_DEFAULT)
  {
    // check for Chinese, Japan, Korean DBCS code pages and switch accordingly
    int acp = (int)GetACP();
    if (acp == 932 || acp == 936 || acp == 949 || acp == 950) {
      iSciDefaultCodePage = acp;
    }
    g_iDefaultNewFileEncoding = Encoding_GetByCodePage(iSciDefaultCodePage);
  }
  */

  // set flag for encoding default
  Encoding_SetDefaultFlag(g_iDefaultNewFileEncoding);

  // define default charset
  g_iDefaultCharSet = (int)CharSetFromCodePage((UINT)iSciDefaultCodePage);

  // Scintilla Styles
  Style_Load();

}


//=============================================================================
//
//  SaveSettings()
//
//
void SaveSettings(bool bSaveSettingsNow) {
  WCHAR *pIniSection = NULL;

  WCHAR wchTmp[MAX_PATH] = { L'\0' };

  if (StringCchLenW(g_wchIniFile, COUNTOF(g_wchIniFile)) == 0) { return; }

  if (!g_bEnableSaveSettings) { return; }

  CreateIniFile();

  if (!g_bSaveSettings && !bSaveSettingsNow) {
    IniSetBool(L"Settings", L"SaveSettings", g_bSaveSettings);
    return;
  }

  WCHAR tchMsg[80];
  GetLngString(IDS_MUI_SAVINGSETTINGS, tchMsg, COUNTOF(tchMsg));
  BeginWaitCursor(tchMsg);

  pIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * INISECTIONBUFCNT * HUGE_BUFFER);
  //int cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);

  IniSectionSetInt(pIniSection, L"SettingsVersion", CFG_VER_CURRENT);
  IniSectionSetBool(pIniSection, L"SaveSettings", g_bSaveSettings); 
  IniSectionSetBool(pIniSection, L"SaveRecentFiles", g_bSaveRecentFiles);
  IniSectionSetBool(pIniSection, L"PreserveCaretPos", g_bPreserveCaretPos);
  IniSectionSetBool(pIniSection, L"SaveFindReplace", g_bSaveFindReplace);
  IniSectionSetBool(pIniSection, L"CloseFind", g_efrData.bFindClose);
  IniSectionSetBool(pIniSection, L"CloseReplace", g_efrData.bReplaceClose);
  IniSectionSetBool(pIniSection, L"NoFindWrap", g_efrData.bNoFindWrap);
  IniSectionSetBool(pIniSection, L"FindTransformBS", g_efrData.bTransformBS);
  IniSectionSetBool(pIniSection, L"WildcardSearch", g_efrData.bWildcardSearch);
  IniSectionSetBool(pIniSection, L"FindMarkAllOccurrences", g_efrData.bMarkOccurences);
  IniSectionSetBool(pIniSection, L"HideNonMatchedLines", g_efrData.bHideNonMatchedLines);
  IniSectionSetBool(pIniSection, L"RegexDotMatchesAll", g_efrData.bDotMatchAll);
  IniSectionSetInt(pIniSection, L"efrData_fuFlags", g_efrData.fuFlags);
  PathRelativeToApp(g_tchOpenWithDir, wchTmp, COUNTOF(wchTmp), false, true, g_flagPortableMyDocs);
  IniSectionSetString(pIniSection, L"OpenWithDir", wchTmp);
  PathRelativeToApp(g_tchFavoritesDir, wchTmp, COUNTOF(wchTmp), false, true, g_flagPortableMyDocs);
  IniSectionSetString(pIniSection, L"Favorites", wchTmp);
  IniSectionSetInt(pIniSection, L"PathNameFormat", iPathNameFormat);
  IniSectionSetBool(pIniSection, L"WordWrap", bWordWrapG);
  IniSectionSetInt(pIniSection, L"WordWrapMode", iWordWrapMode);
  IniSectionSetInt(pIniSection, L"WordWrapIndent", iWordWrapIndent);
  IniSectionSetInt(pIniSection, L"WordWrapSymbols", iWordWrapSymbols);
  IniSectionSetBool(pIniSection, L"ShowWordWrapSymbols", bShowWordWrapSymbols);
  IniSectionSetBool(pIniSection, L"MatchBraces", bMatchBraces);
  IniSectionSetBool(pIniSection, L"AutoCloseTags", bAutoCloseTags);
  IniSectionSetBool(pIniSection, L"HighlightCurrentLine", bHiliteCurrentLine);
  IniSectionSetBool(pIniSection, L"HyperlinkHotspot", g_bHyperlinkHotspot);
  IniSectionSetBool(pIniSection, L"ScrollPastEOF", bScrollPastEOF);
  IniSectionSetBool(pIniSection, L"AutoIndent", bAutoIndent);
  IniSectionSetBool(pIniSection, L"AutoCompleteWords", g_bAutoCompleteWords);
  IniSectionSetBool(pIniSection, L"AccelWordNavigation", g_bAccelWordNavigation);
  IniSectionSetBool(pIniSection, L"ShowIndentGuides", bShowIndentGuides);
  IniSectionSetBool(pIniSection, L"TabsAsSpaces", bTabsAsSpacesG);
  IniSectionSetBool(pIniSection, L"TabIndents", bTabIndentsG);
  IniSectionSetBool(pIniSection, L"BackspaceUnindents", bBackspaceUnindents);
  IniSectionSetInt(pIniSection, L"TabWidth", iTabWidthG);
  IniSectionSetInt(pIniSection, L"IndentWidth", iIndentWidthG);
  IniSectionSetBool(pIniSection, L"MarkLongLines", g_bMarkLongLines);
  IniSectionSetPos(pIniSection, L"LongLinesLimit", iLongLinesLimitG);
  IniSectionSetInt(pIniSection, L"LongLineMode", iLongLineMode);
  IniSectionSetBool(pIniSection, L"ShowSelectionMargin", g_bShowSelectionMargin);
  IniSectionSetBool(pIniSection, L"ShowLineNumbers", g_bShowLineNumbers);
  IniSectionSetBool(pIniSection, L"ShowCodeFolding", g_bShowCodeFolding);
  IniSectionSetInt(pIniSection, L"MarkOccurrences", g_iMarkOccurrences);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchVisible", g_bMarkOccurrencesMatchVisible);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchCase", g_bMarkOccurrencesMatchCase);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesMatchWholeWords", g_bMarkOccurrencesMatchWords);
  IniSectionSetBool(pIniSection, L"MarkOccurrencesCurrentWord", g_bMarkOccurrencesCurrentWord);
  IniSectionSetBool(pIniSection, L"ViewWhiteSpace", bViewWhiteSpace);
  IniSectionSetBool(pIniSection, L"ViewEOLs", bViewEOLs);
  IniSectionSetInt(pIniSection, L"DefaultEncoding", Encoding_MapIniSetting(false, g_iDefaultNewFileEncoding));
  IniSectionSetBool(pIniSection, L"UseDefaultForFileEncoding", bUseDefaultForFileEncoding);
  IniSectionSetBool(pIniSection, L"SkipUnicodeDetection", bSkipUnicodeDetection);
  IniSectionSetBool(pIniSection, L"SkipANSICodePageDetection", bSkipANSICodePageDetection);
  IniSectionSetInt(pIniSection, L"LoadASCIIasUTF8", bLoadASCIIasUTF8);
  IniSectionSetBool(pIniSection, L"LoadNFOasOEM", bLoadNFOasOEM);
  IniSectionSetBool(pIniSection, L"NoEncodingTags", bNoEncodingTags);
  IniSectionSetInt(pIniSection, L"DefaultEOLMode", g_iDefaultEOLMode);
  IniSectionSetBool(pIniSection, L"FixLineEndings", bFixLineEndings);
  IniSectionSetBool(pIniSection, L"FixTrailingBlanks", bAutoStripBlanks);
  IniSectionSetInt(pIniSection, L"PrintHeader", iPrintHeader);
  IniSectionSetInt(pIniSection, L"PrintFooter", iPrintFooter);
  IniSectionSetInt(pIniSection, L"PrintColorMode", iPrintColor);
  IniSectionSetInt(pIniSection, L"PrintZoom", iPrintZoom);
  IniSectionSetInt(pIniSection, L"PrintMarginLeft", pagesetupMargin.left);
  IniSectionSetInt(pIniSection, L"PrintMarginTop", pagesetupMargin.top);
  IniSectionSetInt(pIniSection, L"PrintMarginRight", pagesetupMargin.right);
  IniSectionSetInt(pIniSection, L"PrintMarginBottom", pagesetupMargin.bottom);
  IniSectionSetBool(pIniSection, L"SaveBeforeRunningTools", bSaveBeforeRunningTools);
  IniSectionSetInt(pIniSection, L"FileWatchingMode", g_iFileWatchingMode);
  IniSectionSetBool(pIniSection, L"ResetFileWatching", g_bResetFileWatching);
  IniSectionSetInt(pIniSection, L"EscFunction", iEscFunction);
  IniSectionSetBool(pIniSection, L"AlwaysOnTop", bAlwaysOnTop);
  IniSectionSetBool(pIniSection, L"MinimizeToTray", bMinimizeToTray);
  IniSectionSetBool(pIniSection, L"TransparentMode", bTransparentMode);
  IniSectionSetInt(pIniSection, L"RenderingTechnology", g_iRenderingTechnology);
  IniSectionSetInt(pIniSection, L"Bidirectional", g_iBidirectional);
  IniSectionSetBool(pIniSection, L"ShowToolbar", bShowToolbar);
  IniSectionSetBool(pIniSection, L"ShowStatusbar", bShowStatusbar);
  IniSectionSetInt(pIniSection, L"EncodingDlgSizeX", cxEncodingDlg);
  IniSectionSetInt(pIniSection, L"EncodingDlgSizeY", cyEncodingDlg);
  IniSectionSetInt(pIniSection, L"RecodeDlgSizeX", cxRecodeDlg);
  IniSectionSetInt(pIniSection, L"RecodeDlgSizeY", cyRecodeDlg);
  IniSectionSetInt(pIniSection, L"FileMRUDlgSizeX", cxFileMRUDlg);
  IniSectionSetInt(pIniSection, L"FileMRUDlgSizeY", cyFileMRUDlg);
  IniSectionSetInt(pIniSection, L"OpenWithDlgSizeX", cxOpenWithDlg);
  IniSectionSetInt(pIniSection, L"OpenWithDlgSizeY", cyOpenWithDlg);
  IniSectionSetInt(pIniSection, L"FavoritesDlgSizeX", cxFavoritesDlg);
  IniSectionSetInt(pIniSection, L"FavoritesDlgSizeY", cyFavoritesDlg);
  IniSectionSetInt(pIniSection, L"FindReplaceDlgPosX", xFindReplaceDlg);
  IniSectionSetInt(pIniSection, L"FindReplaceDlgPosY", yFindReplaceDlg);
  IniSectionSetInt(pIniSection, L"CustomSchemesDlgPosX", xCustomSchemesDlg);
  IniSectionSetInt(pIniSection, L"CustomSchemesDlgPosY", yCustomSchemesDlg);

  Toolbar_GetButtons(g_hwndToolbar, IDT_FILE_NEW, g_tchToolbarButtons, COUNTOF(g_tchToolbarButtons));
  if (StringCchCompareX(g_tchToolbarButtons, TBBUTTON_DEFAULT_IDS) == 0) { g_tchToolbarButtons[0] = L'\0'; }
  IniSectionSetString(pIniSection, L"ToolbarButtons", g_tchToolbarButtons);

  SaveIniSection(L"Settings", pIniSection);
  LocalFree(pIniSection);

  
  // Scintilla Styles
  Style_Save();


  /*
    SaveSettingsNow(): query Window Dimensions
  */

  if (bSaveSettingsNow) {
    // GetWindowPlacement
    g_WinInfo = GetMyWindowPlacement(g_hwndMain,NULL);
  }

  int ResX = GetSystemMetrics(SM_CXSCREEN);
  int ResY = GetSystemMetrics(SM_CYSCREEN);

  WCHAR tchHighDpiToolBar[32];
  StringCchPrintf(tchHighDpiToolBar,COUNTOF(tchHighDpiToolBar),L"%ix%i HighDpiToolBar", ResX, ResY);
  IniSetInt(L"Window", tchHighDpiToolBar, iHighDpiToolBar);

  if (!IniGetInt(L"Settings2",L"StickyWindowPosition",0)) {

    WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32], tchZoom[32];

    StringCchPrintf(tchPosX,COUNTOF(tchPosX),L"%ix%i PosX",ResX,ResY);
    StringCchPrintf(tchPosY,COUNTOF(tchPosY),L"%ix%i PosY",ResX,ResY);
    StringCchPrintf(tchSizeX,COUNTOF(tchSizeX),L"%ix%i SizeX",ResX,ResY);
    StringCchPrintf(tchSizeY,COUNTOF(tchSizeY),L"%ix%i SizeY",ResX,ResY);
    StringCchPrintf(tchMaximized,COUNTOF(tchMaximized),L"%ix%i Maximized",ResX,ResY);
    StringCchPrintf(tchZoom, COUNTOF(tchMaximized), L"%ix%i Zoom", ResX, ResY);

    IniSetInt(L"Window",tchPosX,g_WinInfo.x);
    IniSetInt(L"Window",tchPosY,g_WinInfo.y);
    IniSetInt(L"Window",tchSizeX,g_WinInfo.cx);
    IniSetInt(L"Window",tchSizeY,g_WinInfo.cy);
    IniSetInt(L"Window",tchMaximized,g_WinInfo.max);
    IniSetInt(L"Window",tchZoom, g_WinInfo.zoom);
  }

  EndWaitCursor();
}


//=============================================================================
//
//  ParseCommandLine()
//
//
void ParseCommandLine()
{
  LPWSTR lp1,lp2,lp3;
  bool bContinue = true;
  bool bIsFileArg = false;
  bool bIsNotepadReplacement = false;

  LPWSTR lpCmdLine = GetCommandLine();

  if (StrIsEmpty(lpCmdLine)) { return; }

  // Good old console can also send args separated by Tabs
  StrTab2Space(lpCmdLine);

  DocPos const len = (DocPos)(StringCchLenW(lpCmdLine,0) + 2UL);
  lp1 = LocalAlloc(LPTR,sizeof(WCHAR)*len);
  lp2 = LocalAlloc(LPTR,sizeof(WCHAR)*len);
  lp3 = LocalAlloc(LPTR,sizeof(WCHAR)*len);

  if (lp1 && lp2 && lp3) {
    // Start with 2nd argument
    ExtractFirstArgument(lpCmdLine, lp1, lp3, (int)len);

    while (bContinue && ExtractFirstArgument(lp3, lp1, lp2, (int)len)) {
      // options
      if (!bIsFileArg && (StringCchCompareN(lp1, len, L"+", -1) == 0)) {
        g_flagMultiFileArg = 2;
        bIsFileArg = true;
      }

      else if (!bIsFileArg && (StringCchCompareN(lp1, len, L"-", -1) == 0)) {
        g_flagMultiFileArg = 1;
        bIsFileArg = true;
      }

      else if (!bIsFileArg && ((*lp1 == L'/') || (*lp1 == L'-'))) {

        // LTrim
        StrLTrim(lp1, L"-/");

        // Encoding
        if (StringCchCompareIX(lp1, L"ANSI") == 0 || StringCchCompareIX(lp1, L"A") == 0 || StringCchCompareIX(lp1, L"MBCS") == 0)
          g_flagSetEncoding = IDM_ENCODING_ANSI - IDM_ENCODING_ANSI + 1;
        else if (StringCchCompareIX(lp1, L"UNICODE") == 0 || StringCchCompareIX(lp1, L"W") == 0)
          g_flagSetEncoding = IDM_ENCODING_UNICODE - IDM_ENCODING_ANSI + 1;
        else if (StringCchCompareIX(lp1, L"UNICODEBE") == 0 || StringCchCompareIX(lp1, L"UNICODE-BE") == 0)
          g_flagSetEncoding = IDM_ENCODING_UNICODEREV - IDM_ENCODING_ANSI + 1;
        else if (StringCchCompareIX(lp1, L"UTF8") == 0 || StringCchCompareIX(lp1, L"UTF-8") == 0)
          g_flagSetEncoding = IDM_ENCODING_UTF8 - IDM_ENCODING_ANSI + 1;
        else if (StringCchCompareIX(lp1, L"UTF8SIG") == 0 || StringCchCompareIX(lp1, L"UTF-8SIG") == 0 ||
                 StringCchCompareIX(lp1, L"UTF8SIGNATURE") == 0 || StringCchCompareIX(lp1, L"UTF-8SIGNATURE") == 0 ||
                 StringCchCompareIX(lp1, L"UTF8-SIGNATURE") == 0 || StringCchCompareIX(lp1, L"UTF-8-SIGNATURE") == 0)
          g_flagSetEncoding = IDM_ENCODING_UTF8SIGN - IDM_ENCODING_ANSI + 1;

        // EOL Mode
        else if (StringCchCompareIX(lp1, L"CRLF") == 0 || StringCchCompareIX(lp1, L"CR+LF") == 0)
          g_flagSetEOLMode = IDM_LINEENDINGS_CRLF - IDM_LINEENDINGS_CRLF + 1;
        else if (StringCchCompareIX(lp1, L"LF") == 0)
          g_flagSetEOLMode = IDM_LINEENDINGS_LF - IDM_LINEENDINGS_CRLF + 1;
        else if (StringCchCompareIX(lp1, L"CR") == 0)
          g_flagSetEOLMode = IDM_LINEENDINGS_CR - IDM_LINEENDINGS_CRLF + 1;

        // Shell integration
        else if (StrCmpNI(lp1, L"appid=", CSTRLEN(L"appid=")) == 0) {
          StringCchCopyN(g_wchAppUserModelID, COUNTOF(g_wchAppUserModelID),
                         lp1 + CSTRLEN(L"appid="), len - CSTRLEN(L"appid="));
          StrTrim(g_wchAppUserModelID, L" ");
          if (StringCchLenW(g_wchAppUserModelID, COUNTOF(g_wchAppUserModelID)) == 0)
            StringCchCopy(g_wchAppUserModelID, COUNTOF(g_wchAppUserModelID), L"" APPNAME);
        }

        else if (StrCmpNI(lp1, L"sysmru=", CSTRLEN(L"sysmru=")) == 0) {
          WCHAR wch[16];
          StringCchCopyN(wch, COUNTOF(wch), lp1 + CSTRLEN(L"sysmru="), COUNTOF(wch));
          StrTrim(wch, L" ");
          if (*wch == L'1')
            g_flagUseSystemMRU = 2;
          else
            g_flagUseSystemMRU = 1;
        }

        // Relaunch elevated
        else if (StrCmpNI(lp1, L"tmpfbuf=", CSTRLEN(L"tmpfbuf=")) == 0) {
          StringCchCopyN(g_szTmpFilePath, COUNTOF(g_szTmpFilePath),
                         lp1 + CSTRLEN(L"tmpfbuf="), len - CSTRLEN(L"tmpfbuf="));
          TrimStringW(g_szTmpFilePath);
          PathUnquoteSpaces(g_szTmpFilePath);
          NormalizePathEx(g_szTmpFilePath, COUNTOF(g_szTmpFilePath));
          g_flagBufferFile = 1;
        }

        else switch (*CharUpper(lp1)) {

        case L'N':
          g_flagReuseWindow = 0;
          g_flagNoReuseWindow = 1;
          if (*CharUpper(lp1 + 1) == L'S')
            g_flagSingleFileInstance = 1;
          else
            g_flagSingleFileInstance = 0;
          break;

        case L'R':
          g_flagReuseWindow = 1;
          g_flagNoReuseWindow = 0;
          if (*CharUpper(lp1 + 1) == L'S')
            g_flagSingleFileInstance = 1;
          else
            g_flagSingleFileInstance = 0;
          break;

        case L'F':
          if (*(lp1 + 1) == L'0' || *CharUpper(lp1 + 1) == L'O')
            StringCchCopy(g_wchIniFile, COUNTOF(g_wchIniFile), L"*?");
          else if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
            StringCchCopyN(g_wchIniFile, COUNTOF(g_wchIniFile), lp1, len);
            TrimStringW(g_wchIniFile);
            PathUnquoteSpaces(g_wchIniFile);
            NormalizePathEx(g_wchIniFile, COUNTOF(g_wchIniFile));
          }
          break;

        case L'I':
          g_flagStartAsTrayIcon = 1;
          break;

        case L'O':
          if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O')
            g_flagAlwaysOnTop = 1;
          else
            g_flagAlwaysOnTop = 2;
          break;

        case L'P':
          {
            WCHAR *lp = lp1;
            if (StrCmpNI(lp1, L"POS:", CSTRLEN(L"POS:")) == 0)
              lp += CSTRLEN(L"POS:") - 1;
            else if (StrCmpNI(lp1, L"POS", CSTRLEN(L"POS")) == 0)
              lp += CSTRLEN(L"POS") - 1;
            else if (*(lp1 + 1) == L':')
              lp += 1;
            else if (bIsNotepadReplacement) {
              if (*(lp1 + 1) == L'T')
                ExtractFirstArgument(lp2, lp1, lp2, (int)len);
              break;
            }
            if (*(lp + 1) == L'0' || *CharUpper(lp + 1) == L'O') {
              g_flagPosParam = 1;
              g_flagDefaultPos = 1;
            }
            else if (*CharUpper(lp + 1) == L'D' || *CharUpper(lp + 1) == L'S') {
              g_flagPosParam = 1;
              g_flagDefaultPos = (StrChrI((lp + 1), L'L')) ? 3 : 2;
            }
            else if (StrChrI(L"FLTRBM", *(lp + 1))) {
              WCHAR *p = (lp + 1);
              g_flagPosParam = 1;
              g_flagDefaultPos = 0;
              while (*p) {
                switch (*CharUpper(p)) {
                case L'F':
                  g_flagDefaultPos &= ~(4 | 8 | 16 | 32);
                  g_flagDefaultPos |= 64;
                  break;
                case L'L':
                  g_flagDefaultPos &= ~(8 | 64);
                  g_flagDefaultPos |= 4;
                  break;
                case  L'R':
                  g_flagDefaultPos &= ~(4 | 64);
                  g_flagDefaultPos |= 8;
                  break;
                case L'T':
                  g_flagDefaultPos &= ~(32 | 64);
                  g_flagDefaultPos |= 16;
                  break;
                case L'B':
                  g_flagDefaultPos &= ~(16 | 64);
                  g_flagDefaultPos |= 32;
                  break;
                case L'M':
                  if (g_flagDefaultPos == 0)
                    g_flagDefaultPos |= 64;
                  g_flagDefaultPos |= 128;
                  break;
                }
                p = CharNext(p);
              }
            }
            else if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
              int itok =
                swscanf_s(lp1, L"%i,%i,%i,%i,%i", &g_WinInfo.x, &g_WinInfo.y, &g_WinInfo.cx, &g_WinInfo.cy, &g_WinInfo.max);
              if (itok == 4 || itok == 5) { // scan successful
                g_flagPosParam = 1;
                g_flagDefaultPos = 0;
                if (g_WinInfo.cx < 1) g_WinInfo.cx = CW_USEDEFAULT;
                if (g_WinInfo.cy < 1) g_WinInfo.cy = CW_USEDEFAULT;
                if (g_WinInfo.max) g_WinInfo.max = 1;
                if (itok == 4) g_WinInfo.max = 0;
              }
            }
          }
          break;

        case L'T':
          if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
            StringCchCopyN(szTitleExcerpt, COUNTOF(szTitleExcerpt), lp1, len);
            fKeepTitleExcerpt = 1;
          }
          break;

        case L'C':
          g_flagNewFromClipboard = 1;
          break;

        case L'B':
          g_flagPasteBoard = 1;
          break;

        case L'E':
          if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
            if (lpEncodingArg)
              LocalFree(lpEncodingArg);
            lpEncodingArg = StrDup(lp1);
          }
          break;

        case L'G':
          if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
            int itok =
              swscanf_s(lp1, L"%i,%i", &iInitialLine, &iInitialColumn);
            if (itok == 1 || itok == 2) { // scan successful
              g_flagJumpTo = 1;
            }
          }
          break;

        case L'M':
          {
            bool bFindUp = false;
            bool bRegex = false;
            bool bTransBS = false;

            if (StrChr(lp1, L'-'))
              bFindUp = true;
            if (StrChr(lp1, L'R'))
              bRegex = true;
            if (StrChr(lp1, L'B'))
              bTransBS = true;

            if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
              if (lpMatchArg)
                LocalFree(lpMatchArg);
              lpMatchArg = StrDup(lp1);
              g_flagMatchText = 1;

              if (bFindUp)
                g_flagMatchText |= 2;

              if (bRegex) {
                g_flagMatchText &= ~8;
                g_flagMatchText |= 4;
              }

              if (bTransBS) {
                g_flagMatchText &= ~4;
                g_flagMatchText |= 8;
              }
            }
          }
          break;

        case L'L':
          if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O')
            g_flagChangeNotify = 1;
          else
            g_flagChangeNotify = 2;
          break;

        case L'Q':
          g_flagQuietCreate = 1;
          break;

        case L'S':
          if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
            if (lpSchemeArg)
              LocalFree(lpSchemeArg);
            lpSchemeArg = StrDup(lp1);
            g_flagLexerSpecified = 1;
          }
          break;

        case L'D':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 0;
          g_flagLexerSpecified = 1;
          break;

        case L'H':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 35;
          g_flagLexerSpecified = 1;
          break;

        case L'X':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);
            lpSchemeArg = NULL;
          }
          iInitialLexer = 36;
          g_flagLexerSpecified = 1;
          break;

        case L'U':
          g_flagRelaunchElevated = 1;
          break;

        case L'Z':
          ExtractFirstArgument(lp2, lp1, lp2, (int)len);
          g_flagMultiFileArg = 1;
          bIsNotepadReplacement = true;
          break;

        case L'?':
          g_flagDisplayHelp = 1;
          break;

        case L'V':
          g_flagPrintFileAndLeave = 1;
          if (*CharUpper(lp1 + 1) == L'D')
            g_flagPrintFileAndLeave = 2;  // open printer dialog
          break;

        default:
          break;

        }

      }

      // pathname
      else {
        LPWSTR lpFileBuf = LocalAlloc(LPTR, sizeof(WCHAR)*len);
        if (lpFileBuf) {
          cchiFileList = (int)(StringCchLenW(lpCmdLine, len - 2) - StringCchLenW(lp3, len));

          if (lpFileArg) {
            FreeMem(lpFileArg);
            //lpFileArg = NULL;
          }
          lpFileArg = AllocMem(sizeof(WCHAR)*FILE_ARG_BUF, HEAP_ZERO_MEMORY); // changed for ActivatePrevInst() needs
          StringCchCopy(lpFileArg, FILE_ARG_BUF, lp3);

          PathFixBackslashes(lpFileArg);

          if (!PathIsRelative(lpFileArg) && !PathIsUNC(lpFileArg) &&
              PathGetDriveNumber(lpFileArg) == -1 /*&& PathGetDriveNumber(g_wchWorkingDirectory) != -1*/) {

            WCHAR wchPath[FILE_ARG_BUF] = { L'\0' };
            StringCchCopy(wchPath, COUNTOF(wchPath), g_wchWorkingDirectory);
            PathStripToRoot(wchPath);
            PathCchAppend(wchPath, COUNTOF(wchPath), lpFileArg);
            StringCchCopy(lpFileArg, FILE_ARG_BUF, wchPath);
          }

          StrTrim(lpFileArg, L" \"");

          while ((cFileList < FILE_LIST_SIZE) && ExtractFirstArgument(lp3, lpFileBuf, lp3, (int)len)) {
            PathQuoteSpaces(lpFileBuf);
            lpFileList[cFileList++] = StrDup(lpFileBuf);
          }

          bContinue = false;
          LocalFree(lpFileBuf);
        }
      }

      // Continue with next argument
      if (bContinue)
        StringCchCopy(lp3, len, lp2);
    }

    LocalFree(lp1);
    LocalFree(lp2);
    LocalFree(lp3);
  }
}


//=============================================================================
//
//  LoadFlags()
//
//
void LoadFlags()
{
  WCHAR *pIniSection = LocalAlloc(LPTR,sizeof(WCHAR)*32*1024);
  if (pIniSection) {
    int   cchIniSection = (int)LocalSize(pIniSection) / sizeof(WCHAR);

    LoadIniSection(L"Settings2", pIniSection, cchIniSection);

    if (!g_flagReuseWindow && !g_flagNoReuseWindow) {

      if (!IniSectionGetInt(pIniSection, L"ReuseWindow", 0))
        g_flagNoReuseWindow = 1;

      if (IniSectionGetInt(pIniSection, L"SingleFileInstance", 0))
        g_flagSingleFileInstance = 1;
    }

    if (g_flagMultiFileArg == 0) {
      if (IniSectionGetInt(pIniSection, L"MultiFileArg", 0))
        g_flagMultiFileArg = 2;
    }

    if (IniSectionGetInt(pIniSection, L"RelativeFileMRU", 1))
      g_flagRelativeFileMRU = 1;

    if (IniSectionGetInt(pIniSection, L"PortableMyDocs", g_flagRelativeFileMRU))
      g_flagPortableMyDocs = 1;

    if (IniSectionGetInt(pIniSection, L"NoFadeHidden", 0))
      g_flagNoFadeHidden = 1;

    g_flagToolbarLook = IniSectionGetInt(pIniSection, L"ToolbarLook", IsXP() ? 1 : 2);
    g_flagToolbarLook = clampi(g_flagToolbarLook, 0, 2);

    if (IniSectionGetInt(pIniSection, L"SimpleIndentGuides", 0))
      g_flagSimpleIndentGuides = 1;

    if (IniSectionGetInt(pIniSection, L"NoHTMLGuess", 0))
      g_flagNoHTMLGuess = 1;

    if (IniSectionGetInt(pIniSection, L"NoCGIGuess", 0))
      g_flagNoCGIGuess = 1;

    if (IniSectionGetInt(pIniSection, L"NoFileVariables", 0))
      g_flagNoFileVariables = 1;

    if (StringCchLenW(g_wchAppUserModelID, COUNTOF(g_wchAppUserModelID)) == 0) {
      IniSectionGetString(pIniSection, L"ShellAppUserModelID", L"" APPNAME,
                          g_wchAppUserModelID, COUNTOF(g_wchAppUserModelID));
    }

    if (g_flagUseSystemMRU == 0) {
      if (IniSectionGetInt(pIniSection, L"ShellUseSystemMRU", 0)) {
        g_flagUseSystemMRU = 2;
      }
    }

    LocalFree(pIniSection);
  }
}


//=============================================================================
//
//  FindIniFile()
//
//
static bool __fastcall _CheckIniFile(LPWSTR lpszFile,LPCWSTR lpszModule)
{
  WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
  WCHAR tchBuild[MAX_PATH] = { L'\0' };
  ExpandEnvironmentStrings(lpszFile,tchFileExpanded,COUNTOF(tchFileExpanded));

  if (PathIsRelative(tchFileExpanded)) {
    // program directory
    StringCchCopy(tchBuild,COUNTOF(tchBuild),lpszModule);
    StringCchCopy(PathFindFileName(tchBuild),COUNTOF(tchBuild),tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile,MAX_PATH,tchBuild);
      return true;
    }
    // sub directory (.\np3\) 
    StringCchCopy(tchBuild, COUNTOF(tchBuild), lpszModule);
    PathRemoveFileSpec(tchBuild);
    StringCchCat(tchBuild,COUNTOF(tchBuild),L"\\np3\\");
    StringCchCat(tchBuild,COUNTOF(tchBuild),tchFileExpanded);
    if (PathFileExists(tchBuild)) {
      StringCchCopy(lpszFile, MAX_PATH, tchBuild);
      return true;
    }
    // %APPDATA%
    //if (S_OK == SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tchBuild)) {
    if (GetKnownFolderPath(&FOLDERID_RoamingAppData, tchBuild, COUNTOF(tchBuild))) {
      PathCchAppend(tchBuild,COUNTOF(tchBuild),tchFileExpanded);
      if (PathFileExists(tchBuild)) {
        StringCchCopy(lpszFile,MAX_PATH,tchBuild);
        return true;
      }
    }
    // general
    if (SearchPath(NULL,tchFileExpanded,L".ini",COUNTOF(tchBuild),tchBuild,NULL)) {
      StringCchCopy(lpszFile,MAX_PATH,tchBuild);
      return true;
    }
  }
  else if (PathFileExists(tchFileExpanded)) {
    StringCchCopy(lpszFile,MAX_PATH,tchFileExpanded);
    return true;
  }
  return false;
}


static bool __fastcall _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile,LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch,lpszModule)) {
      StringCchCopy(lpszFile,MAX_PATH,tch);
      return true;
    }
    else {
      WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
      ExpandEnvironmentStrings(tch,tchFileExpanded,COUNTOF(tchFileExpanded));
      if (PathIsRelative(tchFileExpanded)) {
        StringCchCopy(lpszFile,MAX_PATH,lpszModule);
        StringCchCopy(PathFindFileName(lpszFile),MAX_PATH,tchFileExpanded);
        return true;
      }
      else {
        StringCchCopy(lpszFile,MAX_PATH,tchFileExpanded);
        return true;
      }
    }
  }
  return false;
}


int FindIniFile() {

  WCHAR tchPath[MAX_PATH] = { L'\0' };
  WCHAR tchModule[MAX_PATH] = { L'\0' };
  
  GetModuleFileName(NULL,tchModule,COUNTOF(tchModule));

  // set env path to module dir
  StringCchCopy(tchPath, COUNTOF(tchPath), tchModule);
  PathRemoveFileSpec(tchPath);
  SetEnvironmentVariable(L"NOTEPAD3MODULEDIR", tchPath);

  if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile))) {
    if (StringCchCompareIX(g_wchIniFile,L"*?") == 0)
      return(0);
    else {
      if (!_CheckIniFile(g_wchIniFile,tchModule)) {
        ExpandEnvironmentStringsEx(g_wchIniFile,COUNTOF(g_wchIniFile));
        if (PathIsRelative(g_wchIniFile)) {
          StringCchCopy(tchPath,COUNTOF(tchPath),tchModule);
          PathRemoveFileSpec(tchPath);
          PathCchAppend(tchPath,COUNTOF(tchPath),g_wchIniFile);
          StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),tchPath);
        }
      }
    }
  }
  else {
    StringCchCopy(tchPath,COUNTOF(tchPath),PathFindFileName(tchModule));
    PathCchRenameExtension(tchPath,COUNTOF(tchPath),L".ini");
    bool bFound = _CheckIniFile(tchPath,tchModule);

    if (!bFound) {
      StringCchCopy(tchPath,COUNTOF(tchPath),L"Notepad3.ini");
      bFound = _CheckIniFile(tchPath,tchModule);
    }

    if (bFound) {

      // allow two redirections: administrator -> user -> custom
      if (_CheckIniFileRedirect(L"" APPNAME, L"" APPNAME ".ini", tchPath, tchModule)) {
        _CheckIniFileRedirect(L"" APPNAME, L"" APPNAME ".ini", tchPath, tchModule);
      }
      StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),tchPath);
    }
    else {
      StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),tchModule);
      PathCchRenameExtension(g_wchIniFile,COUNTOF(g_wchIniFile),L".ini");
    }
  }

  NormalizePathEx(g_wchIniFile,COUNTOF(g_wchIniFile));
 
  return(1);
}


int TestIniFile() {

  if (StringCchCompareIX(g_wchIniFile,L"*?") == 0) {
    StringCchCopy(g_wchIniFile2,COUNTOF(g_wchIniFile2),L"");
    StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),L"");
    return(0);
  }

  if (PathIsDirectory(g_wchIniFile) || *CharPrev(g_wchIniFile,StrEnd(g_wchIniFile, COUNTOF(g_wchIniFile))) == L'\\') {
    WCHAR wchModule[MAX_PATH] = { L'\0' };
    GetModuleFileName(NULL,wchModule,COUNTOF(wchModule));
    PathCchAppend(g_wchIniFile,COUNTOF(g_wchIniFile),PathFindFileName(wchModule));
    PathCchRenameExtension(g_wchIniFile,COUNTOF(g_wchIniFile),L".ini");
    if (!PathFileExists(g_wchIniFile)) {
      StringCchCopy(PathFindFileName(g_wchIniFile),COUNTOF(g_wchIniFile),L"Notepad3.ini");
      if (!PathFileExists(g_wchIniFile)) {
        StringCchCopy(PathFindFileName(g_wchIniFile),COUNTOF(g_wchIniFile),PathFindFileName(wchModule));
        PathCchRenameExtension(g_wchIniFile,COUNTOF(g_wchIniFile),L".ini");
      }
    }
  }
  
  NormalizePathEx(g_wchIniFile,COUNTOF(g_wchIniFile));

  if (!PathFileExists(g_wchIniFile) || PathIsDirectory(g_wchIniFile)) {
    StringCchCopy(g_wchIniFile2,COUNTOF(g_wchIniFile2),g_wchIniFile);
    StringCchCopy(g_wchIniFile,COUNTOF(g_wchIniFile),L"");
    return(0);
  }
  else
    return(1);
}


int CreateIniFile() 
{
  return(CreateIniFileEx(g_wchIniFile));
}


int CreateIniFileEx(LPCWSTR lpszIniFile) 
{
  if (*lpszIniFile) {

    WCHAR *pwchTail = StrRChrW(lpszIniFile, NULL, L'\\');
    if (pwchTail) {
      *pwchTail = 0;
      SHCreateDirectoryEx(NULL,lpszIniFile,NULL);
      *pwchTail = L'\\';
    }

    HANDLE hFile = CreateFile(lpszIniFile,
              GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
              NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    dwLastIOError = GetLastError();
    if (hFile != INVALID_HANDLE_VALUE) {
      if (GetFileSize(hFile,NULL) == 0) {
        DWORD dw;
        WriteFile(hFile,(LPCVOID)L"\xFEFF[Notepad3]\r\n",26,&dw,NULL);
        g_bIniFileFromScratch = true;
      }
      CloseHandle(hFile);
      return(1);
    }
    else
      return(0);
  }
  else
    return(0);
}



//=============================================================================
//
//  DelayUpdateStatusbar()
//  
//
static void __fastcall DelayUpdateStatusbar(int delay, bool bForceRedraw)
{
  static CmdMessageQueue_t mqc = { NULL, WM_COMMAND, (WPARAM)MAKELONG(IDT_TIMER_UPDATE_STATUSBAR, 1), (LPARAM)0, 0 };
  mqc.hwnd = g_hwndMain;
  mqc.lparam = (LPARAM)bForceRedraw;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  DelayUpdateToolbar()
//  
//
static void __fastcall DelayUpdateToolbar(int delay)
{
  static CmdMessageQueue_t mqc = { NULL, WM_COMMAND, (WPARAM)MAKELONG(IDT_TIMER_UPDATE_TOOLBAR, 1), (LPARAM)0, 0 };
  mqc.hwnd = g_hwndMain;
  //mqc.lparam = (LPARAM)bForceRedraw;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  MarkAllOccurrences()
// 
void MarkAllOccurrences(int delay, bool bForceClear)
{
  static CmdMessageQueue_t mqc = { NULL, WM_COMMAND, (WPARAM)MAKELONG(IDT_TIMER_MAIN_MRKALL, 1), (LPARAM)0 , 0 };
  mqc.hwnd = g_hwndMain;
  mqc.lparam = (LPARAM)bForceClear;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  UpdateVisibleUrlHotspot()
// 
void UpdateVisibleUrlHotspot(int delay)
{
  static CmdMessageQueue_t mqc = { NULL, WM_COMMAND, (WPARAM)MAKELONG(IDT_TIMER_UPDATE_HOTSPOT, 1), (LPARAM)0 , 0 };
  mqc.hwnd = g_hwndMain;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}





//=============================================================================
//
//  UpdateToolbar()
//
void UpdateToolbar()
{
  DelayUpdateToolbar(40);
}

//=============================================================================

#define EnableTool(id,b) SendMessage(g_hwndToolbar,TB_ENABLEBUTTON,id, MAKELONG(((b) ? 1 : 0), 0))
#define CheckTool(id,b)  SendMessage(g_hwndToolbar,TB_CHECKBUTTON,id, MAKELONG((b),0))

static void __fastcall _UpdateToolbarDelayed()
{
  SetWindowTitle(g_hwndMain, uidsAppTitle, flagIsElevated, IDS_MUI_UNTITLED, g_wchCurFile,
                 iPathNameFormat, IsDocumentModified || Encoding_HasChanged(CPI_GET),
                 IDS_MUI_READONLY, g_bFileReadOnly, szTitleExcerpt);

  if (!bShowToolbar) { return; }

  EnableTool(IDT_FILE_ADDTOFAV, StringCchLenW(g_wchCurFile, COUNTOF(g_wchCurFile)));
  EnableTool(IDT_FILE_SAVE, (IsDocumentModified || Encoding_HasChanged(CPI_GET)) /*&& !bReadOnly*/);
  CheckTool(IDT_VIEW_WORDWRAP, g_bWordWrap);
  CheckTool(IDT_VIEW_CHASING_DOCTAIL, g_bChasingDocTail);

  bool b1 = SciCall_IsSelectionEmpty();
  bool b2 = (bool)(SciCall_GetTextLength() > 0);
  bool ro = SciCall_GetReadOnly();
  bool tv = EditToggleView(g_hwndEdit, false);

  EnableTool(IDT_EDIT_UNDO, SciCall_CanUndo() && !ro);
  EnableTool(IDT_EDIT_REDO, SciCall_CanRedo() && !ro);
  EnableTool(IDT_EDIT_PASTE, SciCall_CanPaste() && !ro);

  EnableTool(IDT_FILE_LAUNCH, b2);


  EnableTool(IDT_EDIT_FIND, b2);
  //EnableTool(IDT_EDIT_FINDNEXT,b2);
  //EnableTool(IDT_EDIT_FINDPREV,b2 && StringCchLenA(g_efrData.szFind,0));
  EnableTool(IDT_EDIT_REPLACE, b2 && !ro);

  EnableTool(IDT_EDIT_CUT, !b1 && !ro);
  EnableTool(IDT_EDIT_COPY, !b1 && !ro);
  EnableTool(IDT_EDIT_CLEAR, !b1 && !ro);

  EnableTool(IDT_VIEW_TOGGLEFOLDS, b2 && (g_bCodeFoldingAvailable && g_bShowCodeFolding));

  EnableTool(IDT_VIEW_TOGGLE_VIEW, b2 && ((g_iMarkOccurrences > 0) && !g_bMarkOccurrencesMatchVisible));
  CheckTool(IDT_VIEW_TOGGLE_VIEW, tv);
}



//=============================================================================
//
//  _StatusCalcPaneWidth()
//
static LONG __fastcall _StatusCalcPaneWidth(HWND hwnd, LPCWSTR lpsz)
{
  HDC const hdc = GetDC(hwnd);
  HGDIOBJ const hfont = (HGDIOBJ)SendMessage(hwnd, WM_GETFONT, 0, 0);
  HGDIOBJ const hfold = SelectObject(hdc, hfont);
  int const mmode = SetMapMode(hdc, MM_TEXT);

  SIZE size = { 0L, 0L };
  GetTextExtentPoint32(hdc, lpsz, (int)StringCchLenW(lpsz,0), &size);

  SetMapMode(hdc, mmode);
  SelectObject(hdc, hfold);
  ReleaseDC(hwnd, hdc);

  return (size.cx + 8L);
}



//=============================================================================
//
//  _CalculateStatusbarSections
//  vSectionWidth[] must be pre-filled with -1
//
#define txtWidth 80
typedef WCHAR sectionTxt_t[txtWidth];

static void __fastcall _CalculateStatusbarSections(int vSectionWidth[], sectionTxt_t tchStatusBar[], bool* bIsUpdNeeded)
{
  static int s_iWinFormerWidth = -1;
  if (s_iWinFormerWidth != g_WinCurrentWidth) {
    *bIsUpdNeeded = true;
    s_iWinFormerWidth = g_WinCurrentWidth;
  }
  if (!(*bIsUpdNeeded)) { return; }

  // count fixed and dynamic optimized pixels
  int pxCount = 0;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if (g_iStatusbarVisible[i]) {
      if (g_iStatusbarWidthSpec[i] == 0) { // dynamic optimized
        vSectionWidth[i] = _StatusCalcPaneWidth(g_hwndStatus, tchStatusBar[i]);
      }
      else if (g_iStatusbarWidthSpec[i] < -1) { // fixed pixel count
        vSectionWidth[i] = -(g_iStatusbarWidthSpec[i]);
      }
      //else { /* 0,-1 : relative counts */ }
      // accumulate
      if (vSectionWidth[i] > 0) { pxCount += vSectionWidth[i]; }
    }
  }

  int const iPropSectTotalWidth = g_WinCurrentWidth - pxCount - STAUSBAR_RIGHT_MARGIN;

  // init proportional section checker
  bool bIsPropSection[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if ((g_iStatusbarVisible[i]) && (vSectionWidth[i] < 0)) {
      assert(g_iStatusbarWidthSpec[i] > 0);
      bIsPropSection[i] = true;
    }
  }

  // get min. required widths
  int vMinWidth[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
  int iTotalMinWidth = 0;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if (bIsPropSection[i]) {
      int const iMinWidth = _StatusCalcPaneWidth(g_hwndStatus, tchStatusBar[i]);
      vMinWidth[i] = iMinWidth;
      iTotalMinWidth += iMinWidth;
    }
  }

  if (iTotalMinWidth >= iPropSectTotalWidth) 
  {
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        vSectionWidth[i] = vMinWidth[i];
      }
    }
  }
  else // space left for proportional elements
  {
    int vPropWidth[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
    int totalCnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        vPropWidth[i] = iPropSectTotalWidth * g_iStatusbarWidthSpec[i];
        totalCnt += g_iStatusbarWidthSpec[i];
      }
    }
    // normalize
    int const iCeilFloor = (totalCnt + 1) / 2;
    int iTotalPropWidth = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        int const width = (totalCnt > 1) ? ((vPropWidth[i] + iCeilFloor) / totalCnt) : 0;
        vPropWidth[i] = width;
        iTotalPropWidth += width;
      }
    }
    // check for fitting
    int iOverlappingText = 0;
    int iOvlTxtCount = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      if (bIsPropSection[i]) {
        if (vMinWidth[i] > vPropWidth[i]) {
          iOverlappingText += (vMinWidth[i] - vPropWidth[i]);
          ++iOvlTxtCount;
        }
      }
    }
    if (iOvlTxtCount == 0) {
      // we are fine
      for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
        if (bIsPropSection[i]) {
          vSectionWidth[i] = vPropWidth[i];
        }
      }
    }
    else // handling overlaps
    {
      while (iOverlappingText > 0) {
        const int iNoProgress = iOverlappingText;
        for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
          if (bIsPropSection[i]) {
            if (vMinWidth[i] < vPropWidth[i]) {
              vPropWidth[i] -= 1;
              --iOverlappingText;
            }
            else if (vMinWidth[i] > vPropWidth[i]) {
              vPropWidth[i] = vMinWidth[i];
            }
          }
          if (iOverlappingText == 0) { break; /* for */}
        }
        if ((iOverlappingText == 0) || (iNoProgress == iOverlappingText)) { break; }
      }
      // fill missing widths
      for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
        if (bIsPropSection[i]) {
          vSectionWidth[i] = vPropWidth[i];
        }
      }
    }
  }
}



//=============================================================================
//
//  _InterpRectSelTinyExpr()
//
//
static double __fastcall _InterpRectSelTinyExpr(int* piExprError)
{
  #define _tmpBufCnt 256
  char tmpRectSelN[_tmpBufCnt] = { '\0' };

  g_pTempLineBufferMain[0] = '\0';
  size_t const tmpLineBufSize = COUNTOF(g_pTempLineBufferMain);

  DocPosU const selCount = SciCall_GetSelections();

  bool bLastCharWasDigit = false;
  for (DocPosU i = 0; i < selCount; ++i)
  {
    DocPos const posSelStart = SciCall_GetSelectionNStart(i);
    DocPos const posSelEnd = SciCall_GetSelectionNEnd(i);
    size_t const cchToCopy = (size_t)(posSelEnd - posSelStart);
    StringCchCopyNA(tmpRectSelN, _tmpBufCnt, SciCall_GetRangePointer(posSelStart, (DocPos)cchToCopy), cchToCopy);
    StrTrimA(tmpRectSelN, " ");

    if (!StrIsEmptyA(tmpRectSelN))
    {
      if (IsDigitA(tmpRectSelN[0]) && bLastCharWasDigit) {
        StringCchCatA(g_pTempLineBufferMain, tmpLineBufSize, "+"); // default: add numbers
      }
      bLastCharWasDigit = IsDigitA(tmpRectSelN[StringCchLenA(tmpRectSelN,COUNTOF(tmpRectSelN)) - 1]);
      StringCchCatA(g_pTempLineBufferMain, tmpLineBufSize, tmpRectSelN);
    }
  }

  return te_interp(g_pTempLineBufferMain, piExprError);
}


//=============================================================================
//
//  UpdateStatusbar()
//
//
void UpdateStatusbar(bool bForceRedraw)
{
  DelayUpdateStatusbar(40, bForceRedraw);
}

//=============================================================================

const static WCHAR* FR_Status[] = { L"[>--<]", L"[>>--]", L"[>>-+]", L"[+->]>", L"[--<<]", L"[+-<<]", L"<[<-+]"};

FR_STATES g_FindReplaceMatchFoundState = FND_NOP;

static void __fastcall _UpdateStatusbarDelayed(bool bForceRedraw)
{
  if (!bShowStatusbar) { return; }

  bool bIsUpdateNeeded = bForceRedraw;

  static sectionTxt_t tchStatusBar[STATUS_SECTOR_COUNT];
  static WCHAR tchFRStatus[128] = { L'\0' };
  static WCHAR tchTmp[32] = { L'\0' };


  // ------------------------------------------------------
  // common calculations 
  // ------------------------------------------------------
  DocPos const iPos              = SciCall_GetCurrentPos();
  DocLn  const iLnFromPos        = SciCall_LineFromPosition(iPos);
  DocPos const iLineBegin        = SciCall_PositionFromLine(iLnFromPos);
  DocPos const iLineBack         = SciCall_GetLineEndPosition(iLnFromPos);
  DocPos const iSelStart         = SciCall_GetSelectionStart();
  DocPos const iSelEnd           = SciCall_GetSelectionEnd();


  bool const   bIsSelectionEmpty = SciCall_IsSelectionEmpty();
  bool const   bIsSelCountable   = !(bIsSelectionEmpty || SciCall_IsSelectionRectangle());

  // ------------------------------------------------------

  static WCHAR tchLn[32] = { L'\0' };
  static DocLn s_iLnFromPos = -1;
  static WCHAR tchLines[32] = { L'\0' };
  static DocLn s_iLnCnt = -1;

  if (g_iStatusbarVisible[STATUS_DOCLINE] || g_hwndDlgFindReplace)
  {
    if (s_iLnFromPos != iLnFromPos) {
      StringCchPrintf(tchLn, COUNTOF(tchLn), DOCPOSFMTW, iLnFromPos + 1);
      FormatNumberStr(tchLn);
    }

    DocLn const  iLnCnt = SciCall_GetLineCount();
    if (s_iLnCnt != iLnCnt) {
      StringCchPrintf(tchLines, COUNTOF(tchLines), DOCPOSFMTW, iLnCnt);
      FormatNumberStr(tchLines);
    }

    if ((s_iLnFromPos != iLnFromPos) || (s_iLnCnt != iLnCnt))
    {
      StringCchPrintf(tchStatusBar[STATUS_DOCLINE], txtWidth, L"%s%s / %s%s",
        g_mxSBPrefix[STATUS_DOCLINE], tchLn, tchLines, g_mxSBPostfix[STATUS_DOCLINE]);
      s_iLnFromPos = iLnFromPos;
      s_iLnCnt = iLnCnt;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static WCHAR tchCol[32] = { L'\0' };
  static WCHAR tchCols[32] = { L'\0' };

  if (g_iStatusbarVisible[STATUS_DOCCOLUMN] || g_hwndDlgFindReplace)
  {
    DocPos const colOffset = g_bZeroBasedColumnIndex ? 0 : 1;

    static DocPos s_iCol = -1;
    DocPos const iCol = SciCall_GetColumn(iPos) + SciCall_GetSelectionNCaretVirtualSpace(0);
    if (s_iCol != iCol) {
      StringCchPrintf(tchCol, COUNTOF(tchCol), DOCPOSFMTW, iCol + colOffset);
      FormatNumberStr(tchCol);
    }

    static DocPos s_iCols = -1;
    DocPos const iCols = SciCall_GetColumn(iLineBack);
    if (s_iCols != iCols) {
      StringCchPrintf(tchCols, COUNTOF(tchCols), DOCPOSFMTW, iCols);
      FormatNumberStr(tchCols);
    }

    if ((s_iCol != iCol) || (s_iCols != iCols)) {
      StringCchPrintf(tchStatusBar[STATUS_DOCCOLUMN], txtWidth, L"%s%s / %s%s",
        g_mxSBPrefix[STATUS_DOCCOLUMN], tchCol, tchCols, g_mxSBPostfix[STATUS_DOCCOLUMN]);

      s_iCol = iCol;
      s_iCols = iCols;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static WCHAR tchChr[32] = { L'\0' };
  static WCHAR tchChrs[32] = { L'\0' };

  if (g_iStatusbarVisible[STATUS_DOCCHAR])
  {
    DocPos const chrOffset = g_bZeroBasedCharacterCount ? 0 : 1;

    static DocPos s_iChr = -1;
    DocPos const iChr = SciCall_CountCharacters(iLineBegin, iPos);
    if (s_iChr != iChr) {
      StringCchPrintf(tchChr, COUNTOF(tchChr), DOCPOSFMTW, iChr + chrOffset);
      FormatNumberStr(tchChr);
    }

    static DocPos s_iChrs = -1;
    DocPos const iChrs = SciCall_CountCharacters(iLineBegin, iLineBack);
    if (s_iChrs != iChrs) {
      StringCchPrintf(tchChrs, COUNTOF(tchChrs), DOCPOSFMTW, iChrs);
      FormatNumberStr(tchChrs);
    }

    if ((s_iChr != iChr) || (s_iChrs != iChrs)) {
      StringCchPrintf(tchStatusBar[STATUS_DOCCHAR], txtWidth, L"%s%s / %s%s",
        g_mxSBPrefix[STATUS_DOCCHAR], tchChr, tchChrs, g_mxSBPostfix[STATUS_DOCCHAR]);

      s_iChr = iChr;
      s_iChrs = iChrs;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // number of selected chars in statusbar
  static WCHAR tchSel[32] = { L'\0' };
  static WCHAR tchSelB[64] = { L'\0' };
  static bool s_bIsSelCountable = false;
  static DocPos s_iSelStart = -1;
  static DocPos s_iSelEnd = -1;

  if (g_iStatusbarVisible[STATUS_SELECTION] || g_iStatusbarVisible[STATUS_SELCTBYTES] || g_hwndDlgFindReplace)
  {
    if ((s_bIsSelCountable != bIsSelCountable) || (s_iSelStart != iSelStart) || (s_iSelEnd != iSelEnd)) {

      if (bIsSelCountable)
      {
        const DocPos iSel = (DocPos)SendMessage(g_hwndEdit, SCI_COUNTCHARACTERS, iSelStart, iSelEnd);
        StringCchPrintf(tchSel, COUNTOF(tchSel), DOCPOSFMTW, iSel);
        FormatNumberStr(tchSel);
        StrFormatByteSize((iSelEnd - iSelStart), tchSelB, COUNTOF(tchSelB));
      }
      else {
        tchSel[0] = L'-'; tchSel[1] = L'-'; tchSel[2] = L'\0';
        tchSelB[0] = L'0'; tchSelB[1] = L'\0';
      }
      StringCchPrintf(tchStatusBar[STATUS_SELECTION], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_SELECTION], tchSel, g_mxSBPostfix[STATUS_SELECTION]);

      StringCchPrintf(tchStatusBar[STATUS_SELCTBYTES], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_SELCTBYTES], tchSelB, g_mxSBPostfix[STATUS_SELCTBYTES]);

      s_bIsSelCountable = bIsSelCountable;
      s_iSelStart = iSelStart;
      s_iSelEnd = iSelEnd;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // number of selected lines in statusbar
  static WCHAR tchLinesSelected[32] = { L'\0' };
  static bool s_bIsSelectionEmpty = true;
  static DocLn s_iLinesSelected = -1;

  if (g_iStatusbarVisible[STATUS_SELCTLINES])
  {
    DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
    DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);
    DocPos const iStartOfLinePos = SciCall_PositionFromLine(iLineEnd);

    DocLn const iLinesSelected = ((iSelStart != iSelEnd) && (iStartOfLinePos != iSelEnd)) ? ((iLineEnd - iLineStart) + 1) : (iLineEnd - iLineStart);

    if ((s_bIsSelectionEmpty != bIsSelectionEmpty) || (s_iLinesSelected != iLinesSelected))
    {
      if (bIsSelectionEmpty) {
        tchLinesSelected[0] = L'-';
        tchLinesSelected[1] = L'-';
        tchLinesSelected[2] = L'\0';
      }
      else {
        StringCchPrintf(tchLinesSelected, COUNTOF(tchLinesSelected), L"%i", iLinesSelected);
        FormatNumberStr(tchLinesSelected);
      }
      StringCchPrintf(tchStatusBar[STATUS_SELCTLINES], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_SELCTLINES], tchLinesSelected, g_mxSBPostfix[STATUS_SELCTLINES]);

      s_bIsSelectionEmpty = bIsSelectionEmpty;
      s_iLinesSelected = iLinesSelected;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // try calculate expression of selection
  static WCHAR tchExpression[32] = { L'\0' };
  static int s_iExprError = -3;

  if (g_iStatusbarVisible[STATUS_TINYEXPR])
  {
    g_dExpression = 0.0;
    tchExpression[0] = L'-';
    tchExpression[1] = L'-';
    tchExpression[2] = L'\0';

    if (bIsSelCountable)
    {
      if (SciCall_GetSelText(NULL) < COUNTOF(g_pTempLineBufferMain))
      {
        SciCall_GetSelText(g_pTempLineBufferMain);
        //StrDelChrA(chExpression, " \r\n\t\v");
        StrDelChrA(g_pTempLineBufferMain, "\r\n");
        g_dExpression = te_interp(g_pTempLineBufferMain, &g_iExprError);
      }
      else {
        g_iExprError = -1;
      }
    }
    else if (SciCall_IsSelectionRectangle() && !bIsSelectionEmpty)
    {
      g_dExpression = _InterpRectSelTinyExpr(&g_iExprError);
    }
    else
      g_iExprError = -2;


    if (!g_iExprError) {
      if (fabs(g_dExpression) > 99999999.9999)
        StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"%.4E", g_dExpression);
      else
        StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"%.6G", g_dExpression);
    }
    else if (g_iExprError > 0) {
      StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"^[%i]", g_iExprError);
    }

    if (!g_iExprError || (s_iExprError != g_iExprError)) {

      StringCchPrintf(tchStatusBar[STATUS_TINYEXPR], txtWidth, L"%s%s%s ",
        g_mxSBPrefix[STATUS_TINYEXPR], tchExpression, g_mxSBPostfix[STATUS_TINYEXPR]);

      s_iExprError = g_iExprError;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // number of occurrence marks found
  static WCHAR tchOcc[32] = { L'\0' };
  static int s_iMarkOccurrencesCount = -111;
  static bool s_bMOVisible = false;

  if (g_iStatusbarVisible[STATUS_OCCURRENCE] || g_hwndDlgFindReplace)
  {
    if ((s_bMOVisible != g_bMarkOccurrencesMatchVisible) || (s_iMarkOccurrencesCount != g_iMarkOccurrencesCount))
    {
      if ((g_iMarkOccurrencesCount >= 0) && !g_bMarkOccurrencesMatchVisible)
      {
        if ((g_iMarkOccurrencesMaxCount < 0) || (g_iMarkOccurrencesCount < g_iMarkOccurrencesMaxCount))
        {
          StringCchPrintf(tchOcc, COUNTOF(tchOcc), L"%i", g_iMarkOccurrencesCount);
          FormatNumberStr(tchOcc);
        }
        else {
          StringCchPrintf(tchTmp, COUNTOF(tchTmp), L"%i", g_iMarkOccurrencesCount);
          FormatNumberStr(tchTmp);
          StringCchPrintf(tchOcc, COUNTOF(tchOcc), L">= %s", tchTmp);
        }
      }
      else {
        StringCchCopy(tchOcc, COUNTOF(tchOcc), L"--");
      }

      StringCchPrintf(tchStatusBar[STATUS_OCCURRENCE], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_OCCURRENCE], tchOcc, g_mxSBPostfix[STATUS_OCCURRENCE]);

      s_bMOVisible = g_bMarkOccurrencesMatchVisible;
      s_iMarkOccurrencesCount = g_iMarkOccurrencesCount;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // number of replaced pattern
  static WCHAR tchRepl[32] = { L'\0' };
  static int s_iReplacedOccurrences = -1;

  if (g_iStatusbarVisible[STATUS_OCCREPLACE] || g_hwndDlgFindReplace)
  {
    if (s_iReplacedOccurrences != g_iReplacedOccurrences)
    {
      if (g_iReplacedOccurrences > 0)
      {
        StringCchPrintf(tchRepl, COUNTOF(tchRepl), L"%i", g_iReplacedOccurrences);
        FormatNumberStr(tchRepl);
      }
      else {
        StringCchCopy(tchRepl, COUNTOF(tchRepl), L"--");
      }

      StringCchPrintf(tchStatusBar[STATUS_OCCREPLACE], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_OCCREPLACE], tchRepl, g_mxSBPostfix[STATUS_OCCREPLACE]);

      s_iReplacedOccurrences = g_iReplacedOccurrences;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // get number of bytes in current encoding
  static WCHAR tchBytes[32] = { L'\0' };
  static DocPos s_iTextLength = -1;

  if (g_iStatusbarVisible[STATUS_DOCSIZE])
  {
    DocPos const iTextLength = SciCall_GetTextLength();
    if (s_iTextLength != iTextLength) {
      StrFormatByteSize(iTextLength, tchBytes, COUNTOF(tchBytes));

      StringCchPrintf(tchStatusBar[STATUS_DOCSIZE], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_DOCSIZE], tchBytes, g_mxSBPostfix[STATUS_DOCSIZE]);

      s_iTextLength = iTextLength;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static int s_iEncoding = -1;

  if (g_iStatusbarVisible[STATUS_CODEPAGE])
  {
    int const iEncoding = Encoding_Current(CPI_GET);
    if (s_iEncoding != iEncoding) {
      Encoding_SetLabel(iEncoding);

      StringCchPrintf(tchStatusBar[STATUS_CODEPAGE], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_CODEPAGE], Encoding_GetLabel(iEncoding), g_mxSBPostfix[STATUS_CODEPAGE]);

      s_iEncoding = iEncoding;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static int s_iEOLMode = -1;

  if (g_iStatusbarVisible[STATUS_EOLMODE])
  {
    if (s_iEOLMode != g_iEOLMode) {
      if (g_iEOLMode == SC_EOL_CR)
      {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sCR%s",
          g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
      }
      else if (g_iEOLMode == SC_EOL_LF)
      {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sLF%s",
          g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
      }
      else {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sCR+LF%s",
          g_mxSBPrefix[STATUS_EOLMODE], g_mxSBPostfix[STATUS_EOLMODE]);
      }
      s_iEOLMode = g_iEOLMode;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static bool s_bIsOVR = -1;

  if (g_iStatusbarVisible[STATUS_OVRMODE])
  {
    bool const bIsOVR = (bool)SendMessage(g_hwndEdit, SCI_GETOVERTYPE, 0, 0);
    if (s_bIsOVR != bIsOVR) {
      if (bIsOVR)
      {
        StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sOVR%s",
          g_mxSBPrefix[STATUS_OVRMODE], g_mxSBPostfix[STATUS_OVRMODE]);
      }
      else {
        StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sINS%s",
          g_mxSBPrefix[STATUS_OVRMODE], g_mxSBPostfix[STATUS_OVRMODE]);
      }
      s_bIsOVR = bIsOVR;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static bool s_bUse2ndDefault = -1;

  if (g_iStatusbarVisible[STATUS_2ND_DEF])
  {
    bool const bUse2ndDefault = Style_GetUse2ndDefault();
    if (s_bUse2ndDefault != bUse2ndDefault)
    {
      if (bUse2ndDefault)
        StringCchPrintf(tchStatusBar[STATUS_2ND_DEF], txtWidth, L"%s2ND%s",
          g_mxSBPrefix[STATUS_2ND_DEF], g_mxSBPostfix[STATUS_2ND_DEF]);
      else
        StringCchPrintf(tchStatusBar[STATUS_2ND_DEF], txtWidth, L"%sSTD%s",
          g_mxSBPrefix[STATUS_2ND_DEF], g_mxSBPostfix[STATUS_2ND_DEF]);

      s_bUse2ndDefault = bUse2ndDefault;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static int s_iCurLexer = -1;
  static WCHAR tchLexerName[MINI_BUFFER];

  if (g_iStatusbarVisible[STATUS_LEXER])
  {

    int const iCurLexer = Style_GetCurrentLexerRID();
    if (s_iCurLexer != iCurLexer)
    {
      if (Style_IsCurLexerStandard())
        Style_GetLexerDisplayName(NULL, tchLexerName, MINI_BUFFER); // don't distinguish between STD/2ND
      else
        Style_GetLexerDisplayName(Style_GetCurrentLexerPtr(), tchLexerName, MINI_BUFFER);

      StringCchPrintf(tchStatusBar[STATUS_LEXER], txtWidth, L"%s%s%s",
        g_mxSBPrefix[STATUS_LEXER], tchLexerName, g_mxSBPostfix[STATUS_LEXER]);

      s_iCurLexer = iCurLexer;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // ====  Statusbar widths  ====

  int g_vStatusbarSectionWidth[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;

  _CalculateStatusbarSections(g_vStatusbarSectionWidth, tchStatusBar, &bIsUpdateNeeded);

  if (bIsUpdateNeeded) {
    int aStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
    int cnt = 0;
    int totalWidth = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      int const id = g_vSBSOrder[i];
      if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0))
      {
        totalWidth += g_vStatusbarSectionWidth[id];
        aStatusbarSections[cnt++] = totalWidth;
      }
    }

    if (cnt > 0) { aStatusbarSections[cnt - 1] = -1; }
    else { aStatusbarSections[0] = -1; bShowStatusbar = false; }

    SendMessage(g_hwndStatus, SB_SETPARTS, (WPARAM)cnt, (LPARAM)aStatusbarSections);

    cnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      int const id = g_vSBSOrder[i];
      if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0)) {
        StatusSetText(g_hwndStatus, cnt++, tchStatusBar[id]);
      }
    }
    //InvalidateRect(g_hwndStatus,NULL,true);
  }
  // --------------------------------------------------------------------------

  // update Find/Replace dialog (if any)
  static WCHAR tchReplOccs[32] = { L'\0' };
  if (g_hwndDlgFindReplace) {
    if (g_iReplacedOccurrences > 0)
      StringCchPrintf(tchReplOccs, COUNTOF(tchReplOccs), L"%i", g_iReplacedOccurrences);
    else
      StringCchCopy(tchReplOccs, COUNTOF(tchReplOccs), L"--");

    const WCHAR* SBFMT = L" %s%s / %s     %s%s     %s%s     %s%s     %s%s     (  %s  )              ";

    StringCchPrintf(tchFRStatus, COUNTOF(tchFRStatus), SBFMT,
      g_mxSBPrefix[STATUS_DOCLINE], tchLn, tchLines,
      g_mxSBPrefix[STATUS_DOCCOLUMN], tchCol,
      g_mxSBPrefix[STATUS_SELECTION], tchSel,
      g_mxSBPrefix[STATUS_OCCURRENCE], tchOcc,
      g_mxSBPrefix[STATUS_OCCREPLACE], tchReplOccs,
      FR_Status[g_FindReplaceMatchFoundState]);

    SetWindowText(GetDlgItem(g_hwndDlgFindReplace, IDS_FR_STATUS_TEXT), tchFRStatus);
  }

}


//=============================================================================
//
//  UpdateLineNumberWidth()
//
//
void UpdateLineNumberWidth()
{
  if (g_bShowLineNumbers)
  {
    char chLines[32] = { '\0' };
    StringCchPrintfA(chLines, COUNTOF(chLines), "_%td", (size_t)SciCall_GetLineCount());

    int iLineMarginWidthNow = SciCall_GetMarginWidthN(MARGIN_SCI_LINENUM);
    int iLineMarginWidthFit = SciCall_TextWidth(STYLE_LINENUMBER, chLines);

    if (iLineMarginWidthNow != iLineMarginWidthFit) {
      SciCall_SetMarginWidthN(MARGIN_SCI_LINENUM, iLineMarginWidthFit);
    }
  }
  else {
    SciCall_SetMarginWidthN(MARGIN_SCI_LINENUM, 0);
  }

  Style_SetFolding(g_hwndEdit, (g_bCodeFoldingAvailable && g_bShowCodeFolding));
  Style_SetBookmark(g_hwndEdit, g_bShowSelectionMargin);
}



//=============================================================================
//
//  UpdateSettingsCmds()
//
//
void UpdateSettingsCmds()
{
    HMENU hmenu = GetSystemMenu(g_hwndMain, false);
    bool hasIniFile = (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) > 0 || StringCchLenW(g_wchIniFile2,COUNTOF(g_wchIniFile2)) > 0);
    CheckCmd(hmenu, IDM_VIEW_SAVESETTINGS, g_bSaveSettings && g_bEnableSaveSettings);
    EnableCmd(hmenu, IDM_VIEW_SAVESETTINGS, hasIniFile && g_bEnableSaveSettings);
    EnableCmd(hmenu, IDM_VIEW_SAVESETTINGSNOW, hasIniFile && g_bEnableSaveSettings);
    if (SciCall_GetZoom() != 100) { EditShowZoomCallTip(g_hwndEdit); }
}


//=============================================================================
//
//  UpdateUI()
//
void UpdateUI()
{
  struct SCNotification scn;
  scn.nmhdr.hwndFrom = g_hwndEdit;
  scn.nmhdr.idFrom = IDC_EDIT;
  scn.nmhdr.code = SCN_UPDATEUI;
  scn.updated = (SC_UPDATE_CONTENT | SC_UPDATE_NP3_INTERNAL_NOTIFY);
  SendMessage(g_hwndMain, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
  //PostMessage(g_hwndMain, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
}



//=============================================================================

#define UNDOREDO_FREE (-1L)
#define UNDOREDO_BLOCKED (-2L)
static volatile LONG UndoActionToken = UNDOREDO_BLOCKED; // block

//=============================================================================

static bool __fastcall _InUndoRedoTransaction() {
  return (InterlockedOr(&UndoActionToken, 0L) != UNDOREDO_FREE);
}

//=============================================================================
//
//  UndoRedoRecordingStart()
//
void UndoRedoRecordingStart()
{
  InterlockedExchange(&UndoActionToken, UNDOREDO_FREE); // clear
  _UndoRedoActionMap(-1, NULL);
  SciCall_SetUndoCollection(true);
}


//=============================================================================
//
//  UndoRedoRecordingStop()
//
void UndoRedoRecordingStop()
{
  int const curToken = InterlockedOr(&UndoActionToken, 0L);
  if (curToken >= 0) { EndUndoAction(curToken); }

  _UndoRedoActionMap(-1, NULL);

  SciCall_SetUndoCollection(false);
  SciCall_EmptyUndoBuffer();
}


//=============================================================================
//
//  _SaveUndoSelection()
//
//
static int __fastcall _SaveUndoSelection()
{
  UndoRedoSelection_t sel = INIT_UNDOREDOSEL;
  sel.selMode_undo = (int)SendMessage(g_hwndEdit, SCI_GETSELECTIONMODE, 0, 0);

  switch (sel.selMode_undo)
  {
  case SC_SEL_RECTANGLE:
  case SC_SEL_THIN:
    sel.anchorPos_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);
    sel.curPos_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
    if (!g_bDenyVirtualSpaceAccess) {
      sel.anchorVS_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);
      sel.curVS_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARETVIRTUALSPACE, 0, 0);
    }
    break;

  case SC_SEL_LINES:
  case SC_SEL_STREAM:
  default:
    sel.anchorPos_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETANCHOR, 0, 0);
    sel.curPos_undo = (DocPos)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
    break;
  }

  int const token = _UndoRedoActionMap(-1, &sel);

  if (token >= 0) {
    SciCall_AddUndoAction(token, 0);
  }
  
  return token;
}


//=============================================================================
//
//  _SaveRedoSelection()
//
//
static void __fastcall _SaveRedoSelection(int token)
{
  if (token >= 0) {
    UndoRedoSelection_t sel = INIT_UNDOREDOSEL;

    if (_UndoRedoActionMap(token, &sel) >= 0)
    {
      sel.selMode_redo = (int)SendMessage(g_hwndEdit, SCI_GETSELECTIONMODE, 0, 0);

      switch (sel.selMode_redo)
      {
      case SC_SEL_RECTANGLE:
      case SC_SEL_THIN:
        sel.anchorPos_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);
        sel.curPos_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
        if (!g_bDenyVirtualSpaceAccess) {
          sel.anchorVS_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);
        }
        break;

      case SC_SEL_LINES:
      case SC_SEL_STREAM:
      default:
        sel.anchorPos_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETANCHOR, 0, 0);
        sel.curPos_redo = (DocPos)SendMessage(g_hwndEdit, SCI_GETCURRENTPOS, 0, 0);
        break;
      }

      _UndoRedoActionMap(token, &sel); // set with redo action filled
    }
  }
}


//=============================================================================
//
//  BeginUndoAction()
//
//
int BeginUndoAction()
{
  if (_InUndoRedoTransaction()) { return -1; }

  SciCall_BeginUndoAction();

  int const token = _SaveUndoSelection();

  InterlockedExchange(&UndoActionToken, (LONG)token);

  return token;
}



//=============================================================================
//
//  EndUndoAction()
//
//
void EndUndoAction(int token)
{
  if ((token >= 0) && (token == (int)InterlockedOr(&UndoActionToken, 0L)))
  {
    _SaveRedoSelection(token);

    SciCall_EndUndoAction();
    
    InterlockedExchange(&UndoActionToken, UNDOREDO_FREE);
  }
}


//=============================================================================
//
//  RestoreAction()
//
//
void RestoreAction(int token, DoAction doAct)
{
  if (_InUndoRedoTransaction()) { return; }

  UndoRedoSelection_t sel = INIT_UNDOREDOSEL;

  if (_UndoRedoActionMap(token, &sel) >= 0)
  {
    // we are inside undo/redo transaction, so do delayed PostMessage() instead of SendMessage()
  #define ISSUE_MESSAGE PostMessage

    DocPos const _anchorPos = (doAct == UNDO ? sel.anchorPos_undo : sel.anchorPos_redo);
    DocPos const _curPos = (doAct == UNDO ? sel.curPos_undo : sel.curPos_redo);

    // Ensure that the first and last lines of a selection are always unfolded
    // This needs to be done _before_ the SCI_SETSEL message
    DocLn const anchorPosLine = SciCall_LineFromPosition(_anchorPos);
    DocLn const currPosLine = SciCall_LineFromPosition(_curPos);
    ISSUE_MESSAGE(g_hwndEdit, SCI_ENSUREVISIBLE, anchorPosLine, 0);
    if (anchorPosLine != currPosLine) { ISSUE_MESSAGE(g_hwndEdit, SCI_ENSUREVISIBLE, currPosLine, 0); }


    int const selectionMode = (doAct == UNDO ? sel.selMode_undo : sel.selMode_redo);
    ISSUE_MESSAGE(g_hwndEdit, SCI_SETSELECTIONMODE, (WPARAM)selectionMode, 0);

    // independent from selection mode
    ISSUE_MESSAGE(g_hwndEdit, SCI_SETANCHOR, (WPARAM)_anchorPos, 0);
    ISSUE_MESSAGE(g_hwndEdit, SCI_SETCURRENTPOS, (WPARAM)_curPos, 0);

    switch (selectionMode)
    {
    case SC_SEL_RECTANGLE: 
      ISSUE_MESSAGE(g_hwndEdit, SCI_SETRECTANGULARSELECTIONANCHOR, (WPARAM)_anchorPos, 0);
      ISSUE_MESSAGE(g_hwndEdit, SCI_SETRECTANGULARSELECTIONCARET, (WPARAM)_curPos, 0);
      // fall-through

    case SC_SEL_THIN:
      {
        DocPos const anchorVS = (doAct == UNDO ? sel.anchorVS_undo : sel.anchorVS_redo);
        DocPos const currVS = (doAct == UNDO ? sel.curVS_undo : sel.curVS_redo);
        if ((anchorVS != 0) || (currVS != 0)) {
          ISSUE_MESSAGE(g_hwndEdit, SCI_SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, (WPARAM)anchorVS, 0);
          ISSUE_MESSAGE(g_hwndEdit, SCI_SETRECTANGULARSELECTIONCARETVIRTUALSPACE, (WPARAM)currVS, 0);
        }
      }
      break;

    case SC_SEL_LINES:
    case SC_SEL_STREAM:
    default:
      // nothing to do here
      break;
    }
    ISSUE_MESSAGE(g_hwndEdit, SCI_SCROLLCARET, 0, 0);
    ISSUE_MESSAGE(g_hwndEdit, SCI_CHOOSECARETX, 0, 0);
    ISSUE_MESSAGE(g_hwndEdit, SCI_CANCEL, 0, 0);

  #undef ISSUE_MASSAGE
  }
}


//=============================================================================
//
//  _UndoSelectionMap()
//
//
static int __fastcall _UndoRedoActionMap(int token, UndoRedoSelection_t* const selection)
{
  if (UndoRedoSelectionUTArray == NULL)  { return -1; }

  static unsigned int uiTokenCnt = 0U;

  // indexing is unsigned
  unsigned int utoken = (token >= 0) ? (unsigned int)token : 0U;

  if (selection == NULL) {
    // reset / clear
    int const curToken = InterlockedOr(&UndoActionToken, 0L);
    if (curToken >= 0) { EndUndoAction(curToken); }
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_init(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
    uiTokenCnt = 0U;
    InterlockedExchange(&UndoActionToken, UNDOREDO_FREE);
    return -1;
  }

  if (!SciCall_GetUndoCollection()) { return -1; }

  // get or set map item request ?
  if ((token >= 0) && (utoken < uiTokenCnt)) 
  {
    if (selection->anchorPos_undo < 0) {
      // this is a get request
      *selection = *(UndoRedoSelection_t*)utarray_eltptr(UndoRedoSelectionUTArray, utoken);
    }
    else {
      // this is a set request (fill redo pos)
      utarray_insert(UndoRedoSelectionUTArray, (void*)selection, utoken);
    }
    // don't clear map item here (token used in redo/undo again)
  }
  else if (token < 0) {
    // set map new item request
    token = (int)uiTokenCnt;
    utarray_insert(UndoRedoSelectionUTArray, (void*)selection, uiTokenCnt);
    uiTokenCnt = (uiTokenCnt < (unsigned int)INT_MAX) ? (uiTokenCnt + 1U) : 0U;  // round robin next
  }
  return token;
}


//=============================================================================
//
//  FileIO()
//
//
bool FileIO(bool fLoad,LPWSTR pszFileName,bool bSkipUnicodeDetect,bool bSkipANSICPDetection,
            int *ienc,int *ieol,
            bool *pbUnicodeErr,bool *pbFileTooBig, bool* pbUnknownExt,
            bool *pbCancelDataLoss,bool bSaveCopy)
{
  WCHAR tch[MAX_PATH+40];
  bool fSuccess;
  DWORD dwFileAttributes;

  FormatLngStringW(tch,COUNTOF(tch),(fLoad) ? IDS_MUI_LOADFILE : IDS_MUI_SAVEFILE, PathFindFileName(pszFileName));

  BeginWaitCursor(tch);

  if (fLoad) {
    fSuccess = EditLoadFile(g_hwndEdit,pszFileName,bSkipUnicodeDetect,bSkipANSICPDetection,ienc,ieol,pbUnicodeErr,pbFileTooBig,pbUnknownExt);
  }
  else {
    int idx;
    if (MRU_FindFile(g_pFileMRU,pszFileName,&idx)) {
      g_pFileMRU->iEncoding[idx] = *ienc;
      g_pFileMRU->iCaretPos[idx] = (g_bPreserveCaretPos ? SciCall_GetCurrentPos() : 0);
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (g_pFileMRU->pszBookMarks[idx])
        LocalFree(g_pFileMRU->pszBookMarks[idx]);
      g_pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    fSuccess = EditSaveFile(g_hwndEdit,pszFileName,*ienc,pbCancelDataLoss,bSaveCopy);
  }

  dwFileAttributes = GetFileAttributes(pszFileName);
  g_bFileReadOnly = (dwFileAttributes != INVALID_FILE_ATTRIBUTES && dwFileAttributes & FILE_ATTRIBUTE_READONLY);

  EndWaitCursor();

  return(fSuccess);
}


//=============================================================================
//
//  FileLoad()
//
//
bool FileLoad(bool bDontSave, bool bNew, bool bReload, bool bSkipUnicodeDetect, bool bSkipANSICPDetection, LPCWSTR lpszFile)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  WCHAR szFileName[MAX_PATH] = { L'\0' };
  bool bUnicodeErr = false;
  bool bFileTooBig = false;
  bool bUnknownExt = false;
  bool fSuccess;
  int fileEncoding = CPI_ANSI_DEFAULT;

  if (bNew || bReload) {
    if (EditToggleView(g_hwndEdit, false)) {
      EditToggleView(g_hwndEdit, true);
    }
  }

  if (!bDontSave)
  {
    if (!FileSave(false,true,false,false))
      return false;
  }

  if (!bReload) { ResetEncryption(); }

  if (bNew) {
    StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),L"");
    SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
    SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),false);
    if (!fKeepTitleExcerpt) { StringCchCopy(szTitleExcerpt, COUNTOF(szTitleExcerpt), L""); }
    FileVars_Init(NULL,0,&fvCurFile);
    EditSetNewText(g_hwndEdit, "", 0);

    g_iEOLMode = iLineEndings[g_iDefaultEOLMode];
    SendMessage(g_hwndEdit,SCI_SETEOLMODE,iLineEndings[g_iDefaultEOLMode],0);
    Encoding_Current(g_iDefaultNewFileEncoding);
    Encoding_HasChanged(g_iDefaultNewFileEncoding);
    
    EditSetNewText(g_hwndEdit, "", 0);
    Style_SetDefaultLexer(g_hwndEdit);

    g_bFileReadOnly = false;
    _SetDocumentModified(false);

    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateLineNumberWidth();

    // Terminate file watching
    if (g_bResetFileWatching) {
      if (g_bChasingDocTail) {
        SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
      }
      g_iFileWatchingMode = 0;
    }
    InstallFileWatching(NULL);
    g_bEnableSaveSettings = true;
    UpdateSettingsCmds();
    return true;
  }

  if (StrIsEmpty(lpszFile)) {
    if (!OpenFileDlg(g_hwndMain,tch,COUNTOF(tch),NULL))
      return false;
  }
  else
    StringCchCopy(tch,COUNTOF(tch),lpszFile);

  ExpandEnvironmentStringsEx(tch,COUNTOF(tch));

  if (PathIsRelative(tch)) {
    StringCchCopyN(szFileName,COUNTOF(szFileName),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
    PathCchAppend(szFileName,COUNTOF(szFileName),tch);
    if (!PathFileExists(szFileName)) {
      WCHAR wchFullPath[MAX_PATH] = { L'\0' };
      if (SearchPath(NULL,tch,NULL,COUNTOF(wchFullPath),wchFullPath,NULL)) {
        StringCchCopy(szFileName,COUNTOF(szFileName),wchFullPath);
      }
    }
  }
  else
    StringCchCopy(szFileName,COUNTOF(szFileName),tch);

  NormalizePathEx(szFileName,COUNTOF(szFileName));

  if (PathIsLnkFile(szFileName))
    PathGetLnkPath(szFileName,szFileName,COUNTOF(szFileName));

  // change current directory to prevent directory lock on another path
  WCHAR szFolder[MAX_PATH+2];
  if (SUCCEEDED(StringCchCopy(szFolder,COUNTOF(szFolder),tch))) {
    if (SUCCEEDED(PathCchRemoveFileSpec(szFolder,COUNTOF(szFolder)))) {
      SetCurrentDirectory(szFolder);
    }
  }
 
  // Ask to create a new file...
  if (!bReload && !PathFileExists(szFileName))
  {
    if (g_flagQuietCreate || MsgBoxLng(MBYESNO,IDS_MUI_ASK_CREATE,szFileName) == IDYES) {
      HANDLE hFile = CreateFile(szFileName,
                      GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
                      NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
      dwLastIOError = GetLastError();
      fSuccess = (hFile != INVALID_HANDLE_VALUE);
      if (fSuccess) {
        FileVars_Init(NULL,0,&fvCurFile);
        EditSetNewText(g_hwndEdit,"",0);
        Style_SetDefaultLexer(g_hwndEdit);
        g_iEOLMode = iLineEndings[g_iDefaultEOLMode];
        SendMessage(g_hwndEdit,SCI_SETEOLMODE,iLineEndings[g_iDefaultEOLMode],0);
        if (Encoding_SrcCmdLn(CPI_GET) != CPI_NONE) {
          fileEncoding = Encoding_SrcCmdLn(CPI_GET);
          Encoding_Current(fileEncoding);
          Encoding_HasChanged(fileEncoding);
        }
        else {
          Encoding_Current(g_iDefaultNewFileEncoding);
          Encoding_HasChanged(g_iDefaultNewFileEncoding);
        }
        g_bFileReadOnly = false;
        EditSetNewText(g_hwndEdit,"",0);
      }
      if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
        CloseHandle(hFile);
      }
    }
    else
      return false;
  }
  else {
    int idx;
    if (!bReload && MRU_FindFile(g_pFileMRU,szFileName,&idx)) {
      fileEncoding = g_pFileMRU->iEncoding[idx];
      if (fileEncoding > 0)
        Encoding_SrcCmdLn(Encoding_MapUnicode(fileEncoding));
    }
    else
      fileEncoding = Encoding_Current(CPI_GET);

    fSuccess = FileIO(true,szFileName,bSkipUnicodeDetect,bSkipANSICPDetection,&fileEncoding,&g_iEOLMode,&bUnicodeErr,&bFileTooBig,&bUnknownExt,NULL,false);
    if (fSuccess)
      Encoding_Current(fileEncoding); // load may change encoding
  }
  if (fSuccess) {
    StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),szFileName);
    SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
    SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),false);

    if (!fKeepTitleExcerpt)
      StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");

    if (!g_flagLexerSpecified) // flag will be cleared
      Style_SetLexerFromFile(g_hwndEdit,g_wchCurFile);

    SendMessage(g_hwndEdit,SCI_SETEOLMODE,g_iEOLMode,0);
    fileEncoding = Encoding_Current(CPI_GET);
    Encoding_HasChanged(fileEncoding);
    int idx = 0;
    DocPos iCaretPos = 0;
    LPCWSTR pszBookMarks = L"";
    if (!bReload && MRU_FindFile(g_pFileMRU,szFileName,&idx)) {
      iCaretPos = g_pFileMRU->iCaretPos[idx];
      pszBookMarks = g_pFileMRU->pszBookMarks[idx];
    }
    MRU_AddFile(g_pFileMRU,szFileName,g_flagRelativeFileMRU,g_flagPortableMyDocs,fileEncoding,iCaretPos,pszBookMarks);
   
    EditSetBookmarkList(g_hwndEdit, pszBookMarks);
    SetFindPattern((g_pMRUfind ? g_pMRUfind->pszItems[0] : L""));

    if (g_flagUseSystemMRU == 2)
      SHAddToRecentDocs(SHARD_PATHW,szFileName);

    // Install watching of the current file
    if (!bReload && g_bResetFileWatching) {
      if (g_bChasingDocTail) {
        SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
      }
      g_iFileWatchingMode = 0;
    }
    InstallFileWatching(g_wchCurFile);

    // the .LOG feature ...
    if (SciCall_GetTextLength() >= 4) {
      char tchLog[5] = { '\0' };
      SendMessage(g_hwndEdit,SCI_GETTEXT,5,(LPARAM)tchLog);
      if (StringCchCompareXA(tchLog,".LOG") == 0) {
        EditJumpTo(g_hwndEdit,-1,0);
        _BEGIN_UNDO_ACTION_;
        SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
        SendMessage(g_hwndMain,WM_COMMAND,MAKELONG(IDM_EDIT_INSERT_SHORTDATE,1),0);
        EditJumpTo(g_hwndEdit,-1,0);
        SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
        _END_UNDO_ACTION_;
        SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      // set historic caret pos
      else if (iCaretPos > 0) 
      {
        SendMessage(g_hwndEdit, SCI_GOTOPOS, (WPARAM)iCaretPos, 0);
        // adjust view
        const DocPos iCurPos = SciCall_GetCurrentPos();
        EditJumpTo(g_hwndEdit, SciCall_LineFromPosition(iCurPos) + 1, SciCall_GetColumn(iCurPos) + 1);
      }
    }

    //bReadOnly = false;
    _SetDocumentModified(false);
    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateLineNumberWidth();
    UpdateVisibleUrlHotspot(0);

    // consistent settings file handling (if loaded in editor)
    g_bEnableSaveSettings = (StringCchCompareINW(g_wchCurFile, COUNTOF(g_wchCurFile), g_wchIniFile, COUNTOF(g_wchIniFile)) == 0) ? false : true;
    UpdateSettingsCmds();

    // Show warning: Unicode file loaded as ANSI
    if (bUnicodeErr)
      MsgBoxLng(MBWARN,IDS_MUI_ERR_UNICODE);
  }

  else if (!(bFileTooBig || bUnknownExt))
    MsgBoxLng(MBWARN,IDS_MUI_ERR_LOADFILE,szFileName);

  return(fSuccess);
}



//=============================================================================
//
//  FileRevert()
//
//
bool FileRevert(LPCWSTR szFileName, bool bIgnoreCmdLnEnc) 
{
  if (StringCchLen(szFileName, MAX_PATH) != 0) {

    const DocPos iCurPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionCaret() : SciCall_GetCurrentPos();
    const DocPos iAnchorPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionAnchor() : SciCall_GetAnchor();
    //const int vSpcCaretPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionCaretVirtualSpace() : -1;
    //const int vSpcAnchorPos = SciCall_IsSelectionRectangle() ? SciCall_GetRectangularSelectionAnchorVirtualSpace() : -1;

    const DocLn iCurrLine = SciCall_LineFromPosition(iCurPos);
    const DocPos iCurColumn = SciCall_GetColumn(iCurPos);
    const DocLn iVisTopLine = SciCall_GetFirstVisibleLine();
    const DocLn iDocTopLine = SciCall_DocLineFromVisible(iVisTopLine);
    const int   iXOffset = SciCall_GetXOffset();
    const bool bIsTail = (iCurPos == iAnchorPos) && (iCurrLine >= (SciCall_GetLineCount() - 1));

    if (bIgnoreCmdLnEnc) { 
      Encoding_SrcCmdLn(CPI_NONE); // ignore history too
    }
    Encoding_SrcWeak(Encoding_Current(CPI_GET));

    WCHAR tchFileName2[MAX_PATH] = { L'\0' };
    StringCchCopyW(tchFileName2,COUNTOF(tchFileName2),szFileName);

    if (FileLoad(true,false,true,false,true,tchFileName2))
    {
      if (bIsTail && g_iFileWatchingMode == 2) {
        SendMessage(g_hwndEdit, SCI_DOCUMENTEND, 0, 0);
        EditEnsureSelectionVisible(g_hwndEdit);
      }
      else if (SciCall_GetTextLength() >= 4) {
        char tch[5] = { '\0' };
        
        SciCall_GetText(5, tch);
        if (StringCchCompareXA(tch,".LOG") != 0) {
          SciCall_ClearSelections();
          //~EditSelectEx(g_hwndEdit, iAnchorPos, iCurPos, vSpcAnchorPos, vSpcCaretPos);
          EditJumpTo(g_hwndEdit, iCurrLine+1, iCurColumn+1);
          SciCall_EnsureVisible(iDocTopLine);
          const DocLn iNewTopLine = SciCall_GetFirstVisibleLine();
          SciCall_LineScroll(0,iVisTopLine - iNewTopLine);
          SciCall_SetXOffset(iXOffset);
        }
      }
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  FileSave()
//
//
bool FileSave(bool bSaveAlways,bool bAsk,bool bSaveAs,bool bSaveCopy)
{
  WCHAR tchFile[MAX_PATH] = { L'\0' };
  WCHAR tchBase[MAX_PATH] = { L'\0' };
  bool fSuccess = false;
  bool bCancelDataLoss = false;

  bool bIsEmptyNewFile = false;
  if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0) {
    const DocPos cchText = SciCall_GetTextLength();
    if (cchText == 0)
      bIsEmptyNewFile = true;
    else if (cchText < 1023) {
      char chTextBuf[1024];
      SciCall_GetText(1023, chTextBuf);
      StrTrimA(chTextBuf," \t\n\r");
      if (lstrlenA(chTextBuf) == 0)
        bIsEmptyNewFile = true;
    }
  }

  if (!bSaveAlways && (!IsDocumentModified && !Encoding_HasChanged(CPI_GET) || bIsEmptyNewFile) && !bSaveAs) {
    int idx;
    if (MRU_FindFile(g_pFileMRU,g_wchCurFile,&idx)) {
      g_pFileMRU->iEncoding[idx] = Encoding_Current(CPI_GET);
      g_pFileMRU->iCaretPos[idx] = (g_bPreserveCaretPos) ? SciCall_GetCurrentPos() : 0;
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (g_pFileMRU->pszBookMarks[idx])
        LocalFree(g_pFileMRU->pszBookMarks[idx]);
      g_pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    return true;
  }

  if (bAsk)
  {
    // File or "Untitled" ...
    WCHAR tch[MAX_PATH] = { L'\0' };
    if (StringCchLenW(g_wchCurFile, COUNTOF(g_wchCurFile))) {
      StringCchCopy(tch, COUNTOF(tch), g_wchCurFile);
    }
    else {
      GetLngString(IDS_MUI_UNTITLED, tch, COUNTOF(tch));
    }
    switch (MsgBoxLng(MBYESNOCANCEL,IDS_MUI_ASK_SAVE,tch)) {
      case IDCANCEL:
        return false;
      case IDNO:
        return true;
    }
  }

  // Read only...
  if (!bSaveAs && !bSaveCopy && StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)))
  {
    DWORD dwFileAttributes = GetFileAttributes(g_wchCurFile);
    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
      g_bFileReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);
    if (g_bFileReadOnly) {
      UpdateToolbar();
      if (MsgBoxLng(MBYESNOWARN,IDS_MUI_READONLY_SAVE,g_wchCurFile) == IDYES)
        bSaveAs = true;
      else
        return false;
    }
  }

  // Save As...
  if (bSaveAs || bSaveCopy || StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) == 0)
  {
    WCHAR tchInitialDir[MAX_PATH] = { L'\0' };
    if (bSaveCopy && StringCchLenW(g_tchLastSaveCopyDir,COUNTOF(g_tchLastSaveCopyDir))) {
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_tchLastSaveCopyDir);
      StringCchCopy(tchFile,COUNTOF(tchFile),g_tchLastSaveCopyDir);
      PathCchAppend(tchFile,COUNTOF(tchFile),PathFindFileName(g_wchCurFile));
    }
    else
      StringCchCopy(tchFile,COUNTOF(tchFile),g_wchCurFile);

    if (SaveFileDlg(g_hwndMain,tchFile,COUNTOF(tchFile),tchInitialDir))
    {
      int fileEncoding = Encoding_Current(CPI_GET);
      fSuccess = FileIO(false, tchFile, false, true, &fileEncoding, &g_iEOLMode, NULL, NULL, NULL, &bCancelDataLoss, bSaveCopy);
      //~if (fSuccess) Encoding_Current(fileEncoding); // save should not change encoding
      if (fSuccess)
      {
        if (!bSaveCopy)
        {
          StringCchCopy(g_wchCurFile,COUNTOF(g_wchCurFile),tchFile);
          SetDlgItemText(g_hwndMain,IDC_FILENAME,g_wchCurFile);
          SetDlgItemInt(g_hwndMain,IDC_REUSELOCK,GetTickCount(),false);
          if (!fKeepTitleExcerpt)
            StringCchCopy(szTitleExcerpt,COUNTOF(szTitleExcerpt),L"");
          Style_SetLexerFromFile(g_hwndEdit,g_wchCurFile);
          UpdateToolbar();
          UpdateStatusbar(false);
          UpdateLineNumberWidth();
        }
        else {
          StringCchCopy(g_tchLastSaveCopyDir,COUNTOF(g_tchLastSaveCopyDir),tchFile);
          PathRemoveFileSpec(g_tchLastSaveCopyDir);
        }
      }
    }
    else
      return false;
  }
  else {
    int fileEncoding = Encoding_Current(CPI_GET);
    fSuccess = FileIO(false, g_wchCurFile, false, true, &fileEncoding, &g_iEOLMode, NULL, NULL, NULL, &bCancelDataLoss, false);
    //~if (fSuccess) Encoding_Current(fileEncoding); // save should not change encoding
  }

  if (fSuccess)
  {
    if (!bSaveCopy)
    {
      int iCurrEnc = Encoding_Current(CPI_GET);
      Encoding_HasChanged(iCurrEnc);
      const DocPos iCaretPos = SciCall_GetCurrentPos();
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(g_hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      MRU_AddFile(g_pFileMRU,g_wchCurFile,g_flagRelativeFileMRU,g_flagPortableMyDocs,iCurrEnc,iCaretPos,wchBookMarks);
      if (g_flagUseSystemMRU == 2) {
        SHAddToRecentDocs(SHARD_PATHW, g_wchCurFile);
      }
      _SetDocumentModified(false);

      // Install watching of the current file
      if (bSaveAs && g_bResetFileWatching) {
        if (g_bChasingDocTail) {
          SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
        }
        g_iFileWatchingMode = 0;
      }
      InstallFileWatching(g_wchCurFile);
    }
  }
  else if (!bCancelDataLoss)
  {
    if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile)) > 0) {
      StringCchCopy(tchFile,COUNTOF(tchFile),g_wchCurFile);
      StringCchCopy(tchBase,COUNTOF(tchBase),g_wchCurFile);
      PathStripPath(tchBase);
    }
    if (!flagIsElevated && dwLastIOError == ERROR_ACCESS_DENIED) {
      if (IDYES == MsgBoxLng(MBYESNOWARN,IDS_MUI_ERR_ACCESSDENIED,tchFile)) {
        WCHAR lpTempPathBuffer[MAX_PATH];
        WCHAR szTempFileName[MAX_PATH];

        if (GetTempPath(MAX_PATH,lpTempPathBuffer) &&
            GetTempFileName(lpTempPathBuffer,TEXT("NP3"),0,szTempFileName)) {
          int fileEncoding = Encoding_Current(CPI_GET);
          if (FileIO(false,szTempFileName,false,true,&fileEncoding,&g_iEOLMode,NULL,NULL,NULL,&bCancelDataLoss,true)) {
            //~Encoding_Current(fileEncoding); // save should not change encoding
            WCHAR szArguments[2048] = { L'\0' };
            LPWSTR lpCmdLine = GetCommandLine();
            size_t const wlen = StringCchLenW(lpCmdLine,0) + 2;
            LPWSTR lpExe = LocalAlloc(LPTR,sizeof(WCHAR)*wlen);
            LPWSTR lpArgs = LocalAlloc(LPTR,sizeof(WCHAR)*wlen);
            ExtractFirstArgument(lpCmdLine,lpExe,lpArgs,(int)wlen);
            // remove relaunch elevated, we are doing this here already
            lpArgs = StrCutI(lpArgs,L"/u ");
            lpArgs = StrCutI(lpArgs,L"-u ");
            WININFO wi = GetMyWindowPlacement(g_hwndMain,NULL);
            StringCchPrintf(szArguments,COUNTOF(szArguments),
              L"/pos %i,%i,%i,%i,%i /tmpfbuf=\"%s\" %s",wi.x,wi.y,wi.cx,wi.cy,wi.max,szTempFileName,lpArgs);
            if (StringCchLenW(tchFile,COUNTOF(tchFile))) {
              if (!StrStrI(szArguments,tchBase)) {
                StringCchPrintf(szArguments,COUNTOF(szArguments),L"%s \"%s\"",szArguments,tchFile);
              }
            }
            g_flagRelaunchElevated = 1;
            if (RelaunchElevated(szArguments)) {
              LocalFree(lpExe);
              LocalFree(lpArgs);
              // set no change and quit
              Encoding_HasChanged(Encoding_Current(CPI_GET));
              _SetDocumentModified(false);
              PostMessage(g_hwndMain,WM_CLOSE,0,0);
            }
            else {
              if (PathFileExists(szTempFileName)) {
                DeleteFile(szTempFileName);
              }
              UpdateToolbar();
              MsgBoxLng(MBWARN,IDS_MUI_ERR_SAVEFILE,tchFile);
            }
          }
        }
      }
    }
    else {
      UpdateToolbar();
      MsgBoxLng(MBWARN,IDS_MUI_ERR_SAVEFILE,tchFile);
    }
  }
  //???EditEnsureSelectionVisible(g_hwndEdit);
  return(fSuccess);
}


//=============================================================================
//
//  OpenFileDlg()
//
//
static WCHAR s_szFilter[NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER * 100];

bool OpenFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir)
{
  OPENFILENAME ofn;
  WCHAR szFile[MAX_PATH] = { L'\0' };
  WCHAR tchInitialDir[MAX_PATH] = { L'\0' };

  Style_GetOpenDlgFilterStr(s_szFilter,COUNTOF(s_szFilter));

  if (!lpstrInitialDir) {
    if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_wchCurFile);
      PathRemoveFileSpec(tchInitialDir);
    }
    else if (StringCchLenW(g_tchDefaultDir,COUNTOF(g_tchDefaultDir))) {
      ExpandEnvironmentStrings(g_tchDefaultDir,tchInitialDir,COUNTOF(tchInitialDir));
      if (PathIsRelative(tchInitialDir)) {
        WCHAR tchModule[MAX_PATH] = { L'\0' };
        GetModuleFileName(NULL,tchModule,COUNTOF(tchModule));
        PathRemoveFileSpec(tchModule);
        PathCchAppend(tchModule,COUNTOF(tchModule),tchInitialDir);
        PathCchCanonicalize(tchInitialDir,COUNTOF(tchInitialDir),tchModule);
      }
    }
    else
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_wchWorkingDirectory);
  }

  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = s_szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrInitialDir = (lpstrInitialDir) ? lpstrInitialDir : tchInitialDir;
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | /* OFN_NOCHANGEDIR |*/
              OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST |
              OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/;
  ofn.lpstrDefExt = (StringCchLenW(g_tchDefaultExtension,COUNTOF(g_tchDefaultExtension))) ? g_tchDefaultExtension : NULL;

  if (GetOpenFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szFile,COUNTOF(szFile));
    return true;
  }

  else
    return false;
}


//=============================================================================
//
//  SaveFileDlg()
//
//
bool SaveFileDlg(HWND hwnd,LPWSTR lpstrFile,int cchFile,LPCWSTR lpstrInitialDir)
{
  OPENFILENAME ofn;
  WCHAR szNewFile[MAX_PATH] = { L'\0' };
  WCHAR tchInitialDir[MAX_PATH] = { L'\0' };

  StringCchCopy(szNewFile,COUNTOF(szNewFile),lpstrFile);
  Style_GetOpenDlgFilterStr(s_szFilter,COUNTOF(s_szFilter));

  if (StrIsNotEmpty(lpstrInitialDir))
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),lpstrInitialDir);
  else if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_wchCurFile);
    PathRemoveFileSpec(tchInitialDir);
  }
  else if (StringCchLenW(g_tchDefaultDir,COUNTOF(g_tchDefaultDir))) {
    ExpandEnvironmentStrings(g_tchDefaultDir,tchInitialDir,COUNTOF(tchInitialDir));
    if (PathIsRelative(tchInitialDir)) {
      WCHAR tchModule[MAX_PATH] = { L'\0' };
      GetModuleFileName(NULL,tchModule,COUNTOF(tchModule));
      PathRemoveFileSpec(tchModule);
      PathCchAppend(tchModule,COUNTOF(tchModule),tchInitialDir);
      PathCchCanonicalize(tchInitialDir,COUNTOF(tchInitialDir),tchModule);
    }
  }
  else
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),g_wchWorkingDirectory);

  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = s_szFilter;
  ofn.lpstrFile = szNewFile;
  ofn.lpstrInitialDir = tchInitialDir;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_HIDEREADONLY /*| OFN_NOCHANGEDIR*/ |
            /*OFN_NODEREFERENCELINKS |*/ OFN_OVERWRITEPROMPT |
            OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST;
  ofn.lpstrDefExt = (StringCchLenW(g_tchDefaultExtension,COUNTOF(g_tchDefaultExtension))) ? g_tchDefaultExtension : NULL;

  if (GetSaveFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szNewFile,COUNTOF(szNewFile));
    return true;
  }

  else
    return false;
}


/******************************************************************************
*
* ActivatePrevInst()
*
* Tries to find and activate an already open Notepad3 Window
*
*
******************************************************************************/
BOOL CALLBACK EnumWndProc(HWND hwnd,LPARAM lParam)
{
  BOOL bContinue = TRUE;
  WCHAR szClassName[64] = { L'\0' };

  if (GetClassName(hwnd,szClassName,COUNTOF(szClassName)))

    if (StringCchCompareINW(szClassName,COUNTOF(szClassName),wchWndClass,COUNTOF(wchWndClass)) == 0) {

      DWORD dwReuseLock = GetDlgItemInt(hwnd,IDC_REUSELOCK,NULL,FALSE);
      if (GetTickCount() - dwReuseLock >= REUSEWINDOWLOCKTIMEOUT) {

        *(HWND*)lParam = hwnd;

        if (IsWindowEnabled(hwnd))
          bContinue = FALSE;
      }
    }
  return bContinue;
}

BOOL CALLBACK EnumWndProc2(HWND hwnd,LPARAM lParam)
{
  BOOL bContinue = TRUE;
  WCHAR szClassName[64] = { L'\0' };

  if (GetClassName(hwnd,szClassName,COUNTOF(szClassName)))

    if (StringCchCompareINW(szClassName,COUNTOF(szClassName),wchWndClass,COUNTOF(wchWndClass)) == 0) {

      DWORD dwReuseLock = GetDlgItemInt(hwnd,IDC_REUSELOCK,NULL,false);
      if (GetTickCount() - dwReuseLock >= REUSEWINDOWLOCKTIMEOUT) {

        WCHAR tchFileName[MAX_PATH] = { L'\0' };

        if (IsWindowEnabled(hwnd))
          bContinue = FALSE;

        GetDlgItemText(hwnd,IDC_FILENAME,tchFileName,COUNTOF(tchFileName));
        if (StringCchCompareIN(tchFileName,COUNTOF(tchFileName),lpFileArg,-1) == 0)
          *(HWND*)lParam = hwnd;
        else
          bContinue = TRUE;
      }
    }
  return bContinue;
}

bool ActivatePrevInst()
{
  HWND hwnd = NULL;
  COPYDATASTRUCT cds;

  if ((g_flagNoReuseWindow && !g_flagSingleFileInstance) || g_flagStartAsTrayIcon || g_flagNewFromClipboard || g_flagPasteBoard)
    return(false);

  if (g_flagSingleFileInstance && lpFileArg) {

    // Search working directory from second instance, first!
    // lpFileArg is at least MAX_PATH+4 WCHARS
    WCHAR tchTmp[FILE_ARG_BUF] = { L'\0' };

    ExpandEnvironmentStringsEx(lpFileArg,(DWORD)SizeOfMem(lpFileArg)/sizeof(WCHAR));

    if (PathIsRelative(lpFileArg)) {
      StringCchCopyN(tchTmp,COUNTOF(tchTmp),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
      PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
      if (PathFileExists(tchTmp))
        StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);
      else {
        if (SearchPath(NULL,lpFileArg,NULL,COUNTOF(tchTmp),tchTmp,NULL))
          StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);
        else {
          StringCchCopyN(tchTmp,COUNTOF(tchTmp),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
          PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
          StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);
        }
      }
    }

    else if (SearchPath(NULL,lpFileArg,NULL,COUNTOF(tchTmp),tchTmp,NULL))
      StringCchCopy(lpFileArg,FILE_ARG_BUF,tchTmp);

    NormalizePathEx(lpFileArg,FILE_ARG_BUF);

    EnumWindows(EnumWndProc2,(LPARAM)&hwnd);

    if (hwnd != NULL)
    {
      // Enabled
      if (IsWindowEnabled(hwnd))
      {
        // Make sure the previous window won't pop up a change notification message
        //SendMessage(hwnd,WM_CHANGENOTIFYCLEAR,0,0);

        if (IsIconic(hwnd))
          ShowWindowAsync(hwnd,SW_RESTORE);

        if (!IsWindowVisible(hwnd)) {
          SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
          SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
        }

        SetForegroundWindow(hwnd);

        size_t cb = sizeof(np3params);
        if (lpSchemeArg) {
          cb += ((StringCchLen(lpSchemeArg, 0) + 1) * sizeof(WCHAR));
        }
        LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
        params->flagFileSpecified = false;
        params->flagChangeNotify = 0;
        params->flagQuietCreate = false;
        params->flagLexerSpecified = g_flagLexerSpecified;
        if (g_flagLexerSpecified && lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData,0)+1,(StringCchLen(lpSchemeArg,0)+1),lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else
          params->iInitialLexer = iInitialLexer;
        params->flagJumpTo = g_flagJumpTo;
        params->iInitialLine = iInitialLine;
        params->iInitialColumn = iInitialColumn;

        params->iSrcEncoding = (lpEncodingArg) ? Encoding_MatchW(lpEncodingArg) : CPI_NONE;
        params->flagSetEncoding = g_flagSetEncoding;
        params->flagSetEOLMode = g_flagSetEOLMode;
        params->flagTitleExcerpt = 0;

        cds.dwData = DATA_NOTEPAD3_PARAMS;
        cds.cbData = (DWORD)SizeOfMem(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        FreeMem(params);

        return(true);
      }

      else // IsWindowEnabled()
      {
        // Ask...
        if (IDYES == MsgBoxLng(MBYESNOWARN,IDS_MUI_ERR_PREVWINDISABLED))
          return(false);
        else
          return(true);
      }
    }
  }

  if (g_flagNoReuseWindow)
    return(false);

  hwnd = NULL;
  EnumWindows(EnumWndProc,(LPARAM)&hwnd);

  // Found a window
  if (hwnd != NULL)
  {
    // Enabled
    if (IsWindowEnabled(hwnd))
    {
      // Make sure the previous window won't pop up a change notification message
      //SendMessage(hwnd,WM_CHANGENOTIFYCLEAR,0,0);

      if (IsIconic(hwnd))
        ShowWindowAsync(hwnd,SW_RESTORE);

      if (!IsWindowVisible(hwnd)) {
        SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
        SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
      }

      SetForegroundWindow(hwnd);

      if (lpFileArg)
      {
        // Search working directory from second instance, first!
        // lpFileArg is at least MAX_PATH+4 WCHAR
        WCHAR tchTmp[FILE_ARG_BUF] = { L'\0' };

        ExpandEnvironmentStringsEx(lpFileArg,(DWORD)SizeOfMem(lpFileArg)/sizeof(WCHAR));

        if (PathIsRelative(lpFileArg)) {
          StringCchCopyN(tchTmp,COUNTOF(tchTmp),g_wchWorkingDirectory,COUNTOF(g_wchWorkingDirectory));
          PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
          if (PathFileExists(tchTmp)) {
            StringCchCopy(lpFileArg, FILE_ARG_BUF, tchTmp);
          }
          else {
            if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL)) {
              StringCchCopy(lpFileArg, FILE_ARG_BUF, tchTmp);
            }
          }
        }
        else if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL)) {
          StringCchCopy(lpFileArg, FILE_ARG_BUF, tchTmp);
        }

        size_t cb = sizeof(np3params);
        cb += (StringCchLenW(lpFileArg,0) + 1) * sizeof(WCHAR);

        if (lpSchemeArg)
          cb += (StringCchLenW(lpSchemeArg,0) + 1) * sizeof(WCHAR);

        size_t cchTitleExcerpt = StringCchLenW(szTitleExcerpt,COUNTOF(szTitleExcerpt));
        if (cchTitleExcerpt) {
          cb += (cchTitleExcerpt + 1) * sizeof(WCHAR);
        }
        LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
        params->flagFileSpecified = true;
        StringCchCopy(&params->wchData, StringCchLenW(lpFileArg,0)+1,lpFileArg);
        params->flagChangeNotify = g_flagChangeNotify;
        params->flagQuietCreate = g_flagQuietCreate;
        params->flagLexerSpecified = g_flagLexerSpecified;
        if (g_flagLexerSpecified && lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData,0)+1, StringCchLen(lpSchemeArg,0)+1,lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else {
          params->iInitialLexer = iInitialLexer;
        }
        params->flagJumpTo = g_flagJumpTo;
        params->iInitialLine = iInitialLine;
        params->iInitialColumn = iInitialColumn;

        params->iSrcEncoding = (lpEncodingArg) ? Encoding_MatchW(lpEncodingArg) : CPI_NONE;
        params->flagSetEncoding = g_flagSetEncoding;
        params->flagSetEOLMode = g_flagSetEOLMode;

        if (cchTitleExcerpt) {
          StringCchCopy(StrEnd(&params->wchData,0)+1,cchTitleExcerpt+1,szTitleExcerpt);
          params->flagTitleExcerpt = 1;
        }
        else {
          params->flagTitleExcerpt = 0;
        }
        cds.dwData = DATA_NOTEPAD3_PARAMS;
        cds.cbData = (DWORD)SizeOfMem(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        FreeMem(params);    params = NULL;
        FreeMem(lpFileArg); lpFileArg = NULL;
      }
      return(true);
    }
    else // IsWindowEnabled()
    {
      // Ask...
      return ((IDYES == MsgBoxLng(MBYESNOWARN, IDS_MUI_ERR_PREVWINDISABLED)) ? false : true);
    }
  }
  else
    return(false);
}


//=============================================================================
//
//  RelaunchMultiInst()
//
//
bool RelaunchMultiInst() {

  if (g_flagMultiFileArg == 2 && cFileList > 1) {

    LPWSTR lpCmdLineNew = StrDup(GetCommandLine());
    size_t len = StringCchLen(lpCmdLineNew,0) + 1UL;
    LPWSTR lp1 = LocalAlloc(LPTR,sizeof(WCHAR)*len);
    LPWSTR lp2 = LocalAlloc(LPTR,sizeof(WCHAR)*len);

    StrTab2Space(lpCmdLineNew);
    StringCchCopy(lpCmdLineNew + cchiFileList,2,L"");

    WCHAR* pwch = CharPrev(lpCmdLineNew,StrEnd(lpCmdLineNew,len));
    int k = 0;
    while (*pwch == L' ' || *pwch == L'-' || *pwch == L'+') {
      *pwch = L' ';
      pwch = CharPrev(lpCmdLineNew,pwch);
      if (k++ > 1)
        cchiFileList--;
    }

    for (int i = 0; i < cFileList; i++) 
    {
      StringCchCopy(lpCmdLineNew + cchiFileList,8,L" /n - ");
      StringCchCat(lpCmdLineNew,len,lpFileList[i]);
      LocalFree(lpFileList[i]);

      STARTUPINFO si;
      ZeroMemory(&si,sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);

      PROCESS_INFORMATION pi;
      ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));

      CreateProcess(NULL,lpCmdLineNew,NULL,NULL,false,0,NULL,g_wchWorkingDirectory,&si,&pi);
    }

    LocalFree(lpCmdLineNew);
    LocalFree(lp1);
    LocalFree(lp2);
    FreeMem(lpFileArg); lpFileArg = NULL;

    return true;
  }

  else {
    for (int i = 0; i < cFileList; i++) {
      LocalFree(lpFileList[i]);
    }
    return false;
  }
}


//=============================================================================
//
//  RelaunchElevated()
//
//
bool RelaunchElevated(LPWSTR lpArgs) {

  bool result = false;

  if (!IsVista() || flagIsElevated || !g_flagRelaunchElevated || g_flagDisplayHelp)
    return result;

  STARTUPINFO si;
  si.cb = sizeof(STARTUPINFO);
  GetStartupInfo(&si);

  LPWSTR lpCmdLine = GetCommandLine();
  size_t wlen = StringCchLenW(lpCmdLine,0) + 2UL;

  WCHAR lpExe[MAX_PATH + 2] = { L'\0' };
  WCHAR szArgs[2032] = { L'\0' };
  WCHAR szArguments[2032] = { L'\0' };

  ExtractFirstArgument(lpCmdLine,lpExe,szArgs,(int)wlen);

  if (lpArgs) {
    StringCchCopy(szArgs,COUNTOF(szArgs),lpArgs); // override
  }

  if (StrStrI(szArgs,L"/f ") || StrStrI(szArgs,L"-f ")) {
    StringCchCopy(szArguments,COUNTOF(szArguments),szArgs);
  }
  else {
    if (StringCchLenW(g_wchIniFile,COUNTOF(g_wchIniFile)) > 0)
      StringCchPrintf(szArguments,COUNTOF(szArguments),L"/f \"%s\" %s",g_wchIniFile,szArgs);
    else
      StringCchCopy(szArguments,COUNTOF(szArguments),szArgs);
  }

  if (StrIsNotEmpty(szArguments)) {
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | SEE_MASK_NOZONECHECKS;
    sei.hwnd = GetForegroundWindow();
    sei.lpVerb = L"runas";
    sei.lpFile = lpExe;
    sei.lpParameters = szArguments;
    sei.lpDirectory = g_wchWorkingDirectory;
    sei.nShow = si.wShowWindow ? si.wShowWindow : SW_SHOWNORMAL;
    result = ShellExecuteEx(&sei);
  }

  return result;
}


//=============================================================================
//
//  SnapToWinInfoPos()
//  Aligns Notepad3 to the default window position on the current screen
//
void SnapToWinInfoPos(HWND hwnd, const WININFO* const pWinInfo, bool bFullWorkArea)
{
  static WINDOWPLACEMENT s_wndplPrev;
  static bool s_bPrevFullWAFlag = false;
  static bool s_bPrevShowToolbar = true;
  static bool s_bPrevShowStatusbar = true;

  WINDOWPLACEMENT wndpl;
  RECT rcCurrent; GetWindowRect(hwnd, &rcCurrent);

  if (bFullWorkArea) {
    if (s_bPrevFullWAFlag) { // snap to previous rect
      bShowToolbar = s_bPrevShowToolbar;
      bShowStatusbar = s_bPrevShowStatusbar;
      wndpl = s_wndplPrev;
    }
    else {
      GetWindowPlacement(hwnd, &s_wndplPrev);
      s_bPrevShowToolbar = bShowToolbar;
      s_bPrevShowStatusbar = bShowStatusbar;
      bShowToolbar = bShowStatusbar = false;
      wndpl = WindowPlacementFromInfo(hwnd, NULL);
    }
    s_bPrevFullWAFlag = !s_bPrevFullWAFlag;
  }
  else {
    wndpl = WindowPlacementFromInfo(hwnd, pWinInfo);
    if (s_bPrevFullWAFlag) {
      bShowToolbar = s_bPrevShowToolbar;
      bShowStatusbar = s_bPrevShowStatusbar;
    }
    s_bPrevFullWAFlag = false;
  }

  if (GetDoAnimateMinimize()) {
    DrawAnimatedRects(hwnd, IDANI_CAPTION, &rcCurrent, &wndpl.rcNormalPosition);
    //OffsetRect(&wndpl.rcNormalPosition,mi.rcMonitor.left - mi.rcWork.left,mi.rcMonitor.top - mi.rcWork.top);
  }

  SetWindowPlacement(hwnd, &wndpl);
  SciCall_SetZoom(pWinInfo->zoom);

  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateLineNumberWidth();
}


//=============================================================================
//
//  ShowNotifyIcon()
//
//
void ShowNotifyIcon(HWND hwnd,bool bAdd)
{
  static HICON hIcon = NULL;
  if (!hIcon) {
    hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, 
                      GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
  }
  NOTIFYICONDATA nid;
  ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_TRAYMESSAGE;
  nid.hIcon = hIcon;
  StringCchCopy(nid.szTip,COUNTOF(nid.szTip), L"" APPNAME);

  if(bAdd)
    Shell_NotifyIcon(NIM_ADD,&nid);
  else
    Shell_NotifyIcon(NIM_DELETE,&nid);

}


//=============================================================================
//
//  SetNotifyIconTitle()
//
//
void SetNotifyIconTitle(HWND hwnd)
{

  NOTIFYICONDATA nid;
  SHFILEINFO shfi;
  WCHAR tchTitle[256] = { L'\0' };
  WCHAR tchFormat[32] = { L'\0' };

  ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_TIP;

  if (StringCchLenW(szTitleExcerpt,COUNTOF(szTitleExcerpt))) {
    GetLngString(IDS_MUI_TITLEEXCERPT,tchFormat,COUNTOF(tchFormat));
    StringCchPrintf(tchTitle,COUNTOF(tchTitle),tchFormat,szTitleExcerpt);
  }

  else if (StringCchLenW(g_wchCurFile,COUNTOF(g_wchCurFile))) {
    SHGetFileInfo2(g_wchCurFile,FILE_ATTRIBUTE_NORMAL,
      &shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
    PathCompactPathEx(tchTitle,shfi.szDisplayName,COUNTOF(tchTitle)-4,0);
  }
  else {
    GetLngString(IDS_MUI_UNTITLED, tchTitle, COUNTOF(tchTitle) - 4);
  }
  if (IsDocumentModified || Encoding_HasChanged(CPI_GET))
    StringCchCopy(nid.szTip,COUNTOF(nid.szTip),L"* ");
  else
    StringCchCopy(nid.szTip,COUNTOF(nid.szTip),L"");

  StringCchCat(nid.szTip,COUNTOF(nid.szTip),tchTitle);

  Shell_NotifyIcon(NIM_MODIFY,&nid);
}


//=============================================================================
//
//  InstallFileWatching()
//
//
void InstallFileWatching(LPCWSTR lpszFile)
{

  WCHAR tchDirectory[MAX_PATH] = { L'\0' };
  HANDLE hFind;

  // Terminate
  if (!g_iFileWatchingMode || !lpszFile || StringCchLen(lpszFile,MAX_PATH) == 0)
  {
    if (g_bRunningWatch)
    {
      if (hChangeHandle) {
        FindCloseChangeNotification(hChangeHandle);
        hChangeHandle = NULL;
      }
      KillTimer(NULL,ID_WATCHTIMER);
      g_bRunningWatch = false;
      dwChangeNotifyTime = 0;
    }
  }
  else  // Install
  {
    // Terminate previous watching
    if (g_bRunningWatch) {
      if (hChangeHandle) {
        FindCloseChangeNotification(hChangeHandle);
        hChangeHandle = NULL;
      }
      dwChangeNotifyTime = 0;
    }

    // No previous watching installed, so launch the timer first
    else
      SetTimer(NULL,ID_WATCHTIMER,dwFileCheckInverval,WatchTimerProc);

    StringCchCopy(tchDirectory,COUNTOF(tchDirectory),lpszFile);
    PathRemoveFileSpec(tchDirectory);

    // Save data of current file
    hFind = FindFirstFile(g_wchCurFile,&fdCurFile);
    if (hFind != INVALID_HANDLE_VALUE)
      FindClose(hFind);
    else
      ZeroMemory(&fdCurFile,sizeof(WIN32_FIND_DATA));

    hChangeHandle = FindFirstChangeNotification(tchDirectory,false,
      FILE_NOTIFY_CHANGE_FILE_NAME  | \
      FILE_NOTIFY_CHANGE_DIR_NAME   | \
      FILE_NOTIFY_CHANGE_ATTRIBUTES | \
      FILE_NOTIFY_CHANGE_SIZE | \
      FILE_NOTIFY_CHANGE_LAST_WRITE);

    g_bRunningWatch = true;
    dwChangeNotifyTime = 0;
  }
  UpdateToolbar();
}


//=============================================================================
//
//  WatchTimerProc()
//
//
void CALLBACK WatchTimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
  if (g_bRunningWatch)
  {
    if (dwChangeNotifyTime > 0 && GetTickCount() - dwChangeNotifyTime > dwAutoReloadTimeout)
    {
      if (hChangeHandle) {
        FindCloseChangeNotification(hChangeHandle);
        hChangeHandle = NULL;
      }
      KillTimer(NULL,ID_WATCHTIMER);
      g_bRunningWatch = false;
      dwChangeNotifyTime = 0;
      SendMessage(g_hwndMain,WM_CHANGENOTIFY,0,0);
    }

    // Check Change Notification Handle
    else if (WAIT_OBJECT_0 == WaitForSingleObject(hChangeHandle,0))
    {
      // Check if the changes affect the current file
      WIN32_FIND_DATA fdUpdated;
      HANDLE hFind = FindFirstFile(g_wchCurFile,&fdUpdated);
      if (INVALID_HANDLE_VALUE != hFind)
        FindClose(hFind);
      else
        // The current file has been removed
        ZeroMemory(&fdUpdated,sizeof(WIN32_FIND_DATA));

      // Check if the file has been changed
      if (CompareFileTime(&fdCurFile.ftLastWriteTime,&fdUpdated.ftLastWriteTime) != 0 ||
            fdCurFile.nFileSizeLow != fdUpdated.nFileSizeLow ||
            fdCurFile.nFileSizeHigh != fdUpdated.nFileSizeHigh)
      {
        // Shutdown current watching and give control to main window
        if (hChangeHandle) {
          FindCloseChangeNotification(hChangeHandle);
          hChangeHandle = NULL;
        }
        if (g_iFileWatchingMode == 2) {
          g_bRunningWatch = true; /* ! */
          dwChangeNotifyTime = GetTickCount();
        }
        else {
          KillTimer(NULL,ID_WATCHTIMER);
          g_bRunningWatch = false;
          dwChangeNotifyTime = 0;
          SendMessage(g_hwndMain,WM_CHANGENOTIFY,0,0);
        }
      }

      else
        FindNextChangeNotification(hChangeHandle);
    }
  }

  UNUSED(dwTime);
  UNUSED(idEvent);
  UNUSED(uMsg);
  UNUSED(hwnd);
}


//=============================================================================
//
//  PasteBoardTimer()
//
//
void CALLBACK PasteBoardTimer(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
  if ((dwLastCopyTime > 0) && ((GetTickCount() - dwLastCopyTime) > 200)) {

    if (SendMessage(g_hwndEdit,SCI_CANPASTE,0,0)) {

      bool bAutoIndent2 = bAutoIndent;
      bAutoIndent = 0;
      EditJumpTo(g_hwndEdit,-1,0);
      _BEGIN_UNDO_ACTION_;
      if (SendMessage(g_hwndEdit, SCI_GETLENGTH, 0, 0) > 0) {
        SendMessage(g_hwndEdit, SCI_NEWLINE, 0, 0);
      }
      SendMessage(g_hwndEdit,SCI_PASTE,0,0);
      SendMessage(g_hwndEdit,SCI_NEWLINE,0,0);
      _END_UNDO_ACTION_;
      EditEnsureSelectionVisible(g_hwndEdit);
      bAutoIndent = bAutoIndent2;
    }
    dwLastCopyTime = 0;
  }

  UNUSED(dwTime);
  UNUSED(idEvent);
  UNUSED(uMsg);
  UNUSED(hwnd);
}



///  End of Notepad3.c  \\\
