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
#define NOMINMAX 1
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
//#include <pathcch.h>
#include <time.h>
#include <muiload.h>

#include "Edit.h"
#include "Styles.h"
#include "Dialogs.h"
#include "resource.h"
#include "../crypto/crypto.h"
#include "../uthash/utarray.h"
#include "../uthash/utlist.h"
#include "../tinyexpr/tinyexpr.h"
#include "Encoding.h"
#include "Helpers.h"
#include "VersionEx.h"
#include "SciCall.h"
#include "SciLexer.h"
#include "SciXLexer.h"

#include "Notepad3.h"


/******************************************************************************
*
* Local and global Variables for Notepad3.c
*
*/
GLOBALS_T   Globals;
SETTINGS_T  Settings;
SETTINGS2_T Settings2;
FLAGS_T     Flags;
CONSTANTS_T Constants;

static SETTINGS_T  Defaults;
static SETTINGS2_T Defaults2;
static FLAGS_T     DefaultFlags;

// ------------------------------------

static WCHAR     s_wchWndClass[16] = MKWCS(APPNAME);

static HWND      s_hwndEditFrame = NULL;
static HWND      s_hwndNextCBChain = NULL;
static HWND      s_hwndToolbar = NULL;
static HWND      s_hwndReBar = NULL;

static WCHAR     s_wchIniFile2[MAX_PATH + 1] = { L'\0' };
static WCHAR     s_wchTmpFilePath[MAX_PATH + 1] = { L'\0' };

static WCHAR* const s_tchAvailableLanguages = L"af-ZA be-BY de-DE es-ES en-GB fr-FR ja-JP nl-NL ru-RU zh-CN"; // en-US internal

static int    s_iSettingsVersion = CFG_VER_NONE;
static bool   s_bSaveSettings = true;
static bool   s_bEnableSaveSettings = true;

static prefix_t s_mxSBPrefix[STATUS_SECTOR_COUNT];
static prefix_t s_mxSBPostfix[STATUS_SECTOR_COUNT];

static int     s_iStatusbarSections[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;
static bool    s_iStatusbarVisible[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
static int     s_vSBSOrder[STATUS_SECTOR_COUNT] = SBS_INIT_MINUS;
              
static int     s_iStatusbarWidthSpec[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
              
static WCHAR   s_tchToolbarBitmap[MAX_PATH + 1] = { L'\0' };
static WCHAR   s_tchToolbarBitmapHot[MAX_PATH + 1] = { L'\0' };
static WCHAR   s_tchToolbarBitmapDisabled[MAX_PATH + 1] = { L'\0' };

static WCHAR   s_wchPrefixSelection[256] = { L'\0' };
static WCHAR   s_wchAppendSelection[256] = { L'\0' };
static WCHAR   s_wchPrefixLines[256] = { L'\0' };
static WCHAR   s_wchAppendLines[256] = { L'\0' };

static WININFO s_WinInfo = INIT_WININFO;
static WININFO s_DefWinInfo = INIT_WININFO;
static int     s_WinCurrentWidth = 0;

#define FILE_LIST_SIZE 32
static LPWSTR    lpFileList[FILE_LIST_SIZE] = { NULL };
static int       cFileList = 0;
static int       cchiFileList = 0;
static LPWSTR    lpFileArg = NULL;
static LPWSTR    lpSchemeArg = NULL;
static LPWSTR    lpMatchArg = NULL;
static LPWSTR    lpEncodingArg = NULL;

static WCHAR* const _s_RecentFiles = L"Recent Files";
static WCHAR* const _s_RecentFind = L"Recent Find";
static WCHAR* const _s_RecentReplace = L"Recent Replace";

static WCHAR     s_tchLastSaveCopyDir[MAX_PATH + 1] = { L'\0' };
static bool      s_bExternalBitmap = false;

static bool      s_bRunningWatch = false;
static bool      s_bFileReadOnly = false;

static int       s_iHighDpiToolBar = -1;
static int       s_iSortOptions = 0;
static int       s_iAlignMode = 0;
static bool      s_bIsAppThemed = true;
static bool      s_flagIsElevated = false;
static UINT      s_msgTaskbarCreated = 0;
static bool      s_dwChangeNotifyTime = 0;
static HANDLE    s_hChangeHandle = NULL;
static int       s_fKeepTitleExcerpt = 0;
static WCHAR     s_wchTitleExcerpt[MIDSZ_BUFFER] = { L'\0' };
static UINT      s_uidsAppTitle = IDS_MUI_APPTITLE;
static DWORD     s_dwLastCopyTime = 0;
static bool      s_bLastCopyFromMe = false;

static int       s_iInitialLine;
static int       s_iInitialColumn;
static int       s_iInitialLexer;

static int       s_cyReBar;
static int       s_cyReBarFrame;
static int       s_cxEditFrame;
static int       s_cyEditFrame;

// for tiny expression calculation
static double    s_dExpression = 0.0;
static int       s_iExprError = -1;

static WIN32_FIND_DATA s_fdCurFile;

static HMODULE s_hRichEdit = INVALID_HANDLE_VALUE;

#define INISECTIONBUFCNT 32

static TBBUTTON  s_tbbMainWnd[] = { { 0,IDT_FILE_NEW,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 1,IDT_FILE_OPEN,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 3,IDT_FILE_SAVE,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 2,IDT_FILE_BROWSE,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 4,IDT_EDIT_UNDO,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 5,IDT_EDIT_REDO,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 6,IDT_EDIT_CUT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 7,IDT_EDIT_COPY,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 8,IDT_EDIT_PASTE,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 9,IDT_EDIT_FIND,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 10,IDT_EDIT_REPLACE,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 11,IDT_VIEW_WORDWRAP,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 23,IDT_VIEW_TOGGLEFOLDS,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 25,IDT_VIEW_TOGGLE_VIEW,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 21,IDT_FILE_OPENFAV,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 22,IDT_FILE_ADDTOFAV,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 12,IDT_VIEW_ZOOMIN,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 13,IDT_VIEW_ZOOMOUT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 26,IDT_VIEW_CHASING_DOCTAIL,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 14,IDT_VIEW_SCHEME,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 24,IDT_FILE_LAUNCH,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 16,IDT_FILE_EXIT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 15,IDT_VIEW_SCHEMECONFIG,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 0,0,0,BTNS_SEP,{0},0,0 },
                                    { 17,IDT_FILE_SAVEAS,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 18,IDT_FILE_SAVECOPY,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 19,IDT_EDIT_CLEAR,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 },
                                    { 20,IDT_FILE_PRINT,TBSTATE_ENABLED,BTNS_BUTTON,{0},0,0 }
};

#define NUMTOOLBITMAPS  27
#define NUMINITIALTOOLS 33
#define TBBUTTON_DEFAULT_IDS  L"1 2 4 3 0 5 6 0 7 8 9 0 10 11 0 12 0 24 26 0 22 23 0 13 14 0 27 0 15 0 25 0 17"

//=============================================================================

// some Mappings internal idx -> Scintilla values

static int const s_DirectWriteTechnology[4] = {
    SC_TECHNOLOGY_DEFAULT
  , SC_TECHNOLOGY_DIRECTWRITE
  , SC_TECHNOLOGY_DIRECTWRITERETAIN
  , SC_TECHNOLOGY_DIRECTWRITEDC
};

static int const s_SciBidirectional[3] = {
  SC_BIDIRECTIONAL_DISABLED
  , SC_BIDIRECTIONAL_L2R
  , SC_BIDIRECTIONAL_R2L
};


int const g_FontQuality[4] = {
    SC_EFF_QUALITY_DEFAULT
  , SC_EFF_QUALITY_NON_ANTIALIASED
  , SC_EFF_QUALITY_ANTIALIASED
  , SC_EFF_QUALITY_LCD_OPTIMIZED
};

//=============================================================================

// static method declarations

// undo / redo  selections
static UT_icd UndoRedoSelection_icd = { sizeof(UndoRedoSelection_t), NULL, NULL, NULL };
static UT_array* UndoRedoSelectionUTArray = NULL;
static bool  _InUndoRedoTransaction();
static void  _SaveRedoSelection(int token);
static int   _SaveUndoSelection();
static int   _UndoRedoActionMap(int token, UndoRedoSelection_t* selection);


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
      UpdateMarginWidth();
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

static void  _MQ_AppendCmd(CmdMessageQueue_t* const pMsgQCmd, int cycles)
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

/* UNUSED yet
static void _MQ_RemoveCmd(CmdMessageQueue_t* const pMsgQCmd)
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
*/
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
// Static Flags
//
static int s_flagStartAsTrayIcon    = 0;
static int s_flagAlwaysOnTop        = 0;
static int s_flagPosParam           = 0;
static int s_flagWindowPos          = 0;
static int s_flagNewFromClipboard   = 0;
static int s_flagPasteBoard         = 0;
static int s_flagSetEncoding        = 0;
static int s_flagSetEOLMode         = 0;
static int s_flagJumpTo             = 0;
static int s_flagMatchText          = 0;
static int s_flagChangeNotify       = 0;
static int s_flagLexerSpecified     = 0;
static int s_flagQuietCreate        = 0;
static int s_flagRelaunchElevated   = 0;
static int s_flagDisplayHelp        = 0;
static int s_flagBufferFile         = 0;

//==============================================================================

// static forward declarations 
static void  _UpdateStatusbarDelayed(bool bForceRedraw);
static void  _UpdateToolbarDelayed();
static HMODULE  _LoadLanguageResources(const WCHAR* localeName, LANGID langID);

//==============================================================================
//
//  Document Modified Flag
//
//
static bool IsDocumentModified = false;

static void  _SetDocumentModified(bool bModified)
{
  if (IsDocumentModified != bModified) {
    IsDocumentModified = bModified;
    UpdateToolbar();
    UpdateStatusbar(false);
  }
  if (bModified) {
    if (IsWindow(Globals.hwndDlgFindReplace)) {
      SendMessage(Globals.hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDC_DOC_MODIFIED, 1), 0);
    }
  }
}

//==============================================================================


static void _InitGlobals()
{
  Constants.FileBrowserMiniPath = L"minipath.exe";

  Globals.CallTipType = CT_NONE;
  Globals.iWrapCol = 0;
  Globals.bCodeFoldingAvailable = false;
  Globals.bForceLoadASCIIasUTF8 = false;
  Globals.bZeroBasedColumnIndex = false;
  Globals.bZeroBasedCharacterCount = false;
  Globals.iReplacedOccurrences = 0;
  Globals.iMarkOccurrencesCount = 0;
  Globals.bChasingDocTail = false;
  Globals.bUseLimitedAutoCCharSet = false;
  Globals.bIsCJKInputCodePage = false;
  Globals.bIniFileFromScratch = false;
  Globals.bFindReplCopySelOrClip = true;
  Globals.bReplaceInitialized = false;
  Globals.FindReplaceMatchFoundState = FND_NOP;
}


static void _CleanUpResources(const HWND hwnd, bool bIsInitialized) 
{
  if (hwnd) {

    KillTimer(hwnd, IDT_TIMER_MRKALL);
  }

  CmdMessageQueue_t* pmqc = NULL;
  CmdMessageQueue_t* dummy;
  DL_FOREACH_SAFE(MessageQueue, pmqc, dummy)
  {
    DL_DELETE(MessageQueue, pmqc);
    FreeMem(pmqc);
  }

  // Save Settings is done elsewhere

  Scintilla_ReleaseResources();

  if (Globals.hMainMenu) { 
    DestroyMenu(Globals.hMainMenu); 
  }

  if (Globals.hLngResContainer != Globals.hInstance) {
    FreeMUILibrary(Globals.hLngResContainer);
  }

  if (s_hRichEdit) {
    FreeLibrary(s_hRichEdit);
    s_hRichEdit = INVALID_HANDLE_VALUE;
  }

  if (bIsInitialized) {
    UnregisterClass(s_wchWndClass, Globals.hInstance);
  }

  OleUninitialize();
}

//=============================================================================
//
//  WinMain()
//
//
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
  _InitGlobals();

  // Set global variable Globals.hInstance
  Globals.hInstance = hInstance;
  Globals.hPrevInst = hPrevInstance;
  Globals.hndlProcessHeap = GetProcessHeap();

  WCHAR wchAppDir[2 * MAX_PATH + 4] = { L'\0' };
  GetModuleFileName(NULL,wchAppDir,COUNTOF(wchAppDir));
  PathRemoveFileSpec(wchAppDir);
  PathCanonicalizeEx(wchAppDir,COUNTOF(wchAppDir));

  if (!GetCurrentDirectory(COUNTOF(Globals.WorkingDirectory),Globals.WorkingDirectory)) {
    StringCchCopy(Globals.WorkingDirectory,COUNTOF(Globals.WorkingDirectory),wchAppDir);
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
  s_flagIsElevated = IsUserAdmin() || IsElevated();

  // Default Encodings (may already be used for command line parsing)
  Encoding_InitDefaults();

  // Command Line, Ini File and Flags
  ParseCommandLine();
  FindIniFile();
  TestIniFile();
  CreateIniFile();
  LoadFlags();

  // set AppUserModelID
  PrivateSetCurrentProcessExplicitAppUserModelID(Settings2.AppUserModelID);

  // Adapt window class name
  if (s_flagIsElevated) {
    StringCchCat(s_wchWndClass, COUNTOF(s_wchWndClass), L"U");
  }
  if (s_flagPasteBoard) {
    StringCchCat(s_wchWndClass, COUNTOF(s_wchWndClass), L"B");
  }
  // Relaunch with elevated privileges
  if (RelaunchElevated(NULL)) {
    return 0;
  }
  // Try to run multiple instances
  if (RelaunchMultiInst()) {
    return 0;
  }
  // Try to activate another window
  if (ActivatePrevInst()) {
    return 0;
  }

  (void)OleInitialize(NULL);

  INITCOMMONCONTROLSEX icex;
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES;
  InitCommonControlsEx(&icex);

  Scintilla_RegisterClasses(hInstance);

  LoadSettings();

  // ----------------------------------------------------
  // MultiLingual
  //
  bool bPrefLngNotAvail = false;
  
  int res = 0;
  if (StringCchLen(Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName)) > 0)
  {
    WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH];
    res = ResolveLocaleName(Settings2.PreferredLanguageLocaleName, wchLngLocalName, LOCALE_NAME_MAX_LENGTH);
    if (res > 0) {
      StringCchCopy(Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName), wchLngLocalName); // put back resolved name
    }
    // get LANGID
    Globals.iPrefLANGID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    res = GetLocaleInfoEx(Settings2.PreferredLanguageLocaleName, LOCALE_ILANGUAGE | LOCALE_RETURN_NUMBER, (LPWSTR)&Globals.iPrefLANGID, sizeof(LANGID));
  }

  if (res == 0) // No preferred language defined or retrievable, try to get User UI Language
  {
    //~GetUserDefaultLocaleName(&Settings2.PreferredLanguageLocaleName[0], COUNTOF(Settings2.PreferredLanguageLocaleName));
    ULONG numLngs = 0;
    DWORD cchLngsBuffer = 0;
    BOOL hr = GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLngs, NULL, &cchLngsBuffer);
    if (hr) {
      WCHAR* pwszLngsBuffer = AllocMem((cchLngsBuffer + 2) * sizeof(WCHAR), HEAP_ZERO_MEMORY);
      if (pwszLngsBuffer) {
        hr = GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLngs, pwszLngsBuffer, &cchLngsBuffer);
        if (hr && (numLngs > 0)) {
          // get the first 
          StringCchCopy(Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName), pwszLngsBuffer);
          Globals.iPrefLANGID = LANGIDFROMLCID(LocaleNameToLCID(Settings2.PreferredLanguageLocaleName, 0));
          res = 1;
        }
        FreeMem(pwszLngsBuffer);
      }
    }
    if (res == 0) { // last try
      Globals.iPrefLANGID = GetUserDefaultUILanguage();
      LCID const lcid = MAKELCID(Globals.iPrefLANGID, SORT_DEFAULT);
      /*res = */LCIDToLocaleName(lcid, Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName), 0);
    }
  }

  Globals.hLngResContainer = _LoadLanguageResources(Settings2.PreferredLanguageLocaleName, Globals.iPrefLANGID);

  if (!Globals.hLngResContainer) // fallback en-US (1033)
  {
    LANGID const langID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    Globals.hLngResContainer = Globals.hInstance; 
    if (Globals.iPrefLANGID != langID) { bPrefLngNotAvail = true; }
  }
  // ----------------------------------------------------

  if (s_hRichEdit == INVALID_HANDLE_VALUE) {
    //s_hRichEdit = LoadLibrary(L"RICHED20.DLL");  // Use RICHEDIT_CONTROL_VER for control in common_res.h
    s_hRichEdit = LoadLibrary(L"MSFTEDIT.DLL");  // Use "RichEdit50W" for control in common_res.h
  }

  if (!Globals.hDlgIcon) {
    Globals.hDlgIcon = LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON,
      GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
  }

  // Command Line Help Dialog
  if (s_flagDisplayHelp) {
    DisplayCmdLineHelp(NULL);
    _CleanUpResources(NULL, false);
    return 0;
  }

  s_msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

  Globals.hMainMenu = NULL;
  if (Globals.hLngResContainer != Globals.hInstance) {
    Globals.hMainMenu = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_MAINMENU));
    if (!Globals.hMainMenu) {
      GetLastErrorToMsgBox(L"LoadMenu()", 0);
    }
  }

  if (!InitApplication(Globals.hInstance)) 
  {
    _CleanUpResources(NULL, false);
    return 1; 
  }

  HWND const hwnd = InitInstance(Globals.hInstance, lpCmdLine, nShowCmd);
  if (!hwnd) { 
    _CleanUpResources(hwnd, true);
    return 1; 
  }

  if (Globals.hMainMenu) { SetMenu(hwnd, Globals.hMainMenu); }

#ifdef _EXTRA_DRAG_N_DROP_HANDLER_
  DragAndDropInit(NULL);
#endif

  HACCEL const hAccMain = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_MAINWND));
  HACCEL const hAccFindReplace = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCFINDREPLACE));
  HACCEL const hAccCoustomizeSchemes = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCCUSTOMSCHEMES));
 
  SetTimer(hwnd, IDT_TIMER_MRKALL, USER_TIMER_MINIMUM, (TIMERPROC)MQ_ExecuteNext);
  
  if (bPrefLngNotAvail) {
    InfoBoxLng(MBWARN, L"MsgPrefLanguageNotAvailable", IDS_WARN_PREF_LNG_NOT_AVAIL, Settings2.PreferredLanguageLocaleName);
  }

  MSG msg;
  while (GetMessage(&msg,NULL,0,0))
  {
    if (IsWindow(Globals.hwndDlgFindReplace) && ((msg.hwnd == Globals.hwndDlgFindReplace) || IsChild(Globals.hwndDlgFindReplace, msg.hwnd))) 
    {
      const int iTr = TranslateAccelerator(Globals.hwndDlgFindReplace, hAccFindReplace, &msg);
      if (iTr || IsDialogMessage(Globals.hwndDlgFindReplace, &msg))
        continue;
    }
    if (IsWindow(Globals.hwndDlgCustomizeSchemes) && ((msg.hwnd == Globals.hwndDlgCustomizeSchemes) || IsChild(Globals.hwndDlgCustomizeSchemes, msg.hwnd))) {
      const int iTr = TranslateAccelerator(Globals.hwndDlgCustomizeSchemes, hAccCoustomizeSchemes, &msg);
      if (iTr || IsDialogMessage(Globals.hwndDlgCustomizeSchemes, &msg))
        continue;
    }
    if (!TranslateAccelerator(hwnd,hAccMain,&msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  _CleanUpResources(hwnd, true);

  return (int)(msg.wParam);
}


//=============================================================================
//
//  _LngStrToMultiLngStr
//
//
static bool  _LngStrToMultiLngStr(WCHAR* pLngStr, WCHAR* pLngMultiStr, size_t lngMultiStrSize)
{
  bool rtnVal = true;

  size_t strLen = StringCchLenW(pLngStr,0);

  if ((strLen > 0) && pLngMultiStr && (lngMultiStrSize > 0)) {
    WCHAR* lngMultiStrPtr = pLngMultiStr;
    WCHAR* last = pLngStr + (Has_UTF16_LE_BOM((char*)pLngStr,clampi((int)strLen,0,8)) ? 1 : 0);
    while (last && rtnVal) {
      // make sure you validate the user input
      WCHAR* next = StrNextTok(last, L",; :");
      if (next) { *next = L'\0'; }
      strLen = StringCchLenW(last, LOCALE_NAME_MAX_LENGTH);
      if (strLen && IsValidLocaleName(last)) {
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
static HMODULE  _LoadLanguageResources(const WCHAR* localeName, LANGID langID)
{
  bool bLngAvailable = (StrStrIW(s_tchAvailableLanguages, localeName) != NULL);
  if (!bLngAvailable) { return NULL; }

  WCHAR tchAvailLngs[LARGE_BUFFER] = { L'\0' };
  StringCchCopyW(tchAvailLngs, LARGE_BUFFER, s_tchAvailableLanguages);
  WCHAR tchUserLangMultiStrg[LARGE_BUFFER] = { L'\0' };
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
static bool  CheckWaitCursorStack()
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
    StatusSetText(Globals.hwndStatus, STATUS_HELP, text);
    //StatusSetTextID(Globals.hwndStatus, STATUS_HELP, uid);
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
    StatusSetSimple(Globals.hwndStatus, false);
    UpdateStatusbar(false);
  }
}


//=============================================================================
//
//  _InitWindowPosition()
//
//
static WININFO _InitDefaultWndPos(const int flagsPos)
{
  RECT rc;
  GetWindowRect(GetDesktopWindow(), &rc);
  MONITORINFO mi;
  GetMonitorInfoFromRect(&rc, &mi);
  WININFO winfo = INIT_WININFO;
  winfo.y = mi.rcMonitor.top;
  winfo.cy = mi.rcWork.bottom - mi.rcWork.top;
  winfo.cx = (mi.rcWork.right - mi.rcWork.left) / 2;
  winfo.x = (flagsPos == 3) ? mi.rcMonitor.left : winfo.cx;
  winfo.max = 0;
  winfo.zoom = 100;
  return winfo;
}
// ----------------------------------------------------------------------------

static void  _InitWindowPosition(WININFO* pWinInfo, const int flagsPos)
{
  WININFO winfo = *pWinInfo;

  if (flagsPos == 1) {
    winfo.x = winfo.y = winfo.cx = winfo.cy = CW_USEDEFAULT;
    winfo.max = false;
    winfo.zoom = 100;
  }
  else if (flagsPos == 2)
  {
    winfo = s_DefWinInfo; // NP3 default window position
  }
  else if (flagsPos == 3)
  {
    winfo = _InitDefaultWndPos(flagsPos);
  }
  else if (flagsPos >= 4)
  {
    RECT rc;
    GetWindowRect(GetDesktopWindow(), &rc);
    MONITORINFO mi;
    GetMonitorInfoFromRect(&rc, &mi);

    int const width = (mi.rcWork.right - mi.rcWork.left);
    int const height = (mi.rcWork.bottom - mi.rcWork.top);

    if (flagsPos & 8)
      winfo.x = mi.rcMonitor.left + (width >> 1);
    else
      winfo.x = mi.rcMonitor.left;

    if (flagsPos & (4 | 8))
      winfo.cx = (width >> 1);
    else 
      winfo.cx = width;

    if (flagsPos & 32)
      winfo.y = mi.rcMonitor.top + (height >> 1);
    else
      winfo.y = mi.rcMonitor.top;

    if (flagsPos & (16 | 32))
      winfo.cy = (height >> 1);
    else
      winfo.cy = height;

    if (flagsPos & 64) {
      winfo.x = mi.rcMonitor.left;
      winfo.y = mi.rcMonitor.top;
      winfo.cx = width;
      winfo.cy = height;
    }
    if (flagsPos & 128) {
      winfo = s_DefWinInfo;
      winfo.max = true;
      winfo.zoom = 100;
    }
  }
  else { // restore window, move upper left corner to Work Area 
    
    MONITORINFO mi;
    RECT const rc = RectFromWinInfo(&winfo);
    GetMonitorInfoFromRect(&rc, &mi);
    WININFO wi = winfo; wi.cx = wi.cy = 16; // really small
    FitIntoMonitorWorkArea(&(mi.rcWork), &wi, false);
    winfo.x = wi.x;
    winfo.y = wi.y;
  }

  *pWinInfo = winfo;
}


//=============================================================================
//
// InitApplication()
//
//
bool InitApplication(HINSTANCE hInstance)
{
  WNDCLASS wc;
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = CS_BYTEALIGNWINDOW | CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = Globals.hDlgIcon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
  wc.lpszMenuName = MAKEINTRESOURCE(IDR_MUI_MAINMENU);
  wc.lpszClassName = s_wchWndClass;

  return RegisterClass(&wc);
}


//=============================================================================
//
//  InitInstance()
//
HWND InitInstance(HINSTANCE hInstance,LPCWSTR pszCmdLine,int nCmdShow)
{
  UNUSED(pszCmdLine);
 
  _InitWindowPosition(&s_WinInfo, s_flagWindowPos);
  s_WinCurrentWidth = s_WinInfo.cx;

  // get monitor coordinates from g_WinInfo
  WININFO srcninfo = s_WinInfo;
  WinInfoToScreen(&srcninfo);

  Globals.hwndMain = CreateWindowEx(
               0,
               s_wchWndClass,
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

  if (s_WinInfo.max) {
    nCmdShow = SW_SHOWMAXIMIZED;
  }
  if ((Settings.AlwaysOnTop || s_flagAlwaysOnTop == 2) && s_flagAlwaysOnTop != 1) {
    SetWindowPos(Globals.hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }

  if (Settings.TransparentMode) {
    SetWindowTransparentMode(Globals.hwndMain, true, Settings2.OpacityLevel);
  }
  
  if (s_WinInfo.zoom) {
    SciCall_SetZoom(s_WinInfo.zoom);
  }

  // Current file information -- moved in front of ShowWindow()
  FileLoad(true,true,false,Settings.SkipUnicodeDetection,Settings.SkipANSICodePageDetection,L"");

  if (!s_flagStartAsTrayIcon) {
    ShowWindow(Globals.hwndMain,nCmdShow);
    UpdateWindow(Globals.hwndMain);
  }
  else {
    ShowWindow(Globals.hwndMain,SW_HIDE);    // trick ShowWindow()
    ShowNotifyIcon(Globals.hwndMain,true);
  }

  // Source Encoding
  if (lpEncodingArg)
    Encoding_SrcCmdLn(Encoding_MatchW(lpEncodingArg));

  // Pathname parameter
  if (s_flagBufferFile || (lpFileArg /*&& !g_flagNewFromClipboard*/))
  {
    bool bOpened = false;

    // Open from Directory
    if (!s_flagBufferFile && PathIsDirectory(lpFileArg)) {
      WCHAR tchFile[MAX_PATH] = { L'\0' };
      if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), lpFileArg))
        bOpened = FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, tchFile);
    }
    else {
      LPCWSTR lpFileToOpen = s_flagBufferFile ? s_wchTmpFilePath : lpFileArg;
      bOpened = FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, lpFileToOpen);
      if (bOpened) {
        if (s_flagBufferFile) {
          if (lpFileArg) {
            InstallFileWatching(NULL); // Terminate file watching
            StringCchCopy(Globals.CurrentFile,COUNTOF(Globals.CurrentFile),lpFileArg);
            InstallFileWatching(Globals.CurrentFile);
          }
          else
            StringCchCopy(Globals.CurrentFile,COUNTOF(Globals.CurrentFile),L"");

          if (!s_flagLexerSpecified)
            Style_SetLexerFromFile(Globals.hwndEdit,Globals.CurrentFile);

          _SetDocumentModified(true);

          // check for temp file and delete
          if (s_flagIsElevated && PathFileExists(s_wchTmpFilePath)) {
            DeleteFile(s_wchTmpFilePath);
          }
        }
        if (s_flagJumpTo) { // Jump to position
          EditJumpTo(Globals.hwndEdit,s_iInitialLine,s_iInitialColumn);
        }
      }
    }
    if (lpFileArg) {
      FreeMem(lpFileArg);
      lpFileArg = NULL;
    }
    if (bOpened) {
      if (s_flagChangeNotify == 1) {
        Settings.FileWatchingMode = 0;
        Settings.ResetFileWatching = true;
        InstallFileWatching(Globals.CurrentFile);
      }
      else if (s_flagChangeNotify == 2) {
        if (!Globals.bChasingDocTail) { 
          SendMessage(Globals.hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0); 
        }
        else {
          Settings.FileWatchingMode = 2;
          Settings.ResetFileWatching = true;
          InstallFileWatching(Globals.CurrentFile);
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
  s_flagQuietCreate = 0;
  s_fKeepTitleExcerpt = 0;

  // undo / redo selections
  if (UndoRedoSelectionUTArray != NULL) {
    utarray_clear(UndoRedoSelectionUTArray);
    utarray_free(UndoRedoSelectionUTArray);
    UndoRedoSelectionUTArray = NULL;
  }
  utarray_new(UndoRedoSelectionUTArray, &UndoRedoSelection_icd);
  utarray_reserve(UndoRedoSelectionUTArray,256);

  // Check for /c [if no file is specified] -- even if a file is specified
  /*else */if (s_flagNewFromClipboard) {
    if (SendMessage(Globals.hwndEdit, SCI_CANPASTE, 0, 0)) {
      bool bAutoIndent2 = Settings.AutoIndent;
      Settings.AutoIndent = 0;
      EditJumpTo(Globals.hwndEdit, -1, 0);
      _BEGIN_UNDO_ACTION_;
      if (SendMessage(Globals.hwndEdit, SCI_GETLENGTH, 0, 0) > 0) {
        SendMessage(Globals.hwndEdit, SCI_NEWLINE, 0, 0);
      }
      SendMessage(Globals.hwndEdit, SCI_PASTE, 0, 0);
      SendMessage(Globals.hwndEdit, SCI_NEWLINE, 0, 0);
      _END_UNDO_ACTION_;
      Settings.AutoIndent = bAutoIndent2;
      if (s_flagJumpTo)
        EditJumpTo(Globals.hwndEdit, s_iInitialLine, s_iInitialColumn);
      else
        EditEnsureSelectionVisible(Globals.hwndEdit);
    }
  }

  // Encoding
  if (0 != s_flagSetEncoding) {
    SendMessage(
      Globals.hwndMain,
      WM_COMMAND,
      MAKELONG(IDM_ENCODING_ANSI + s_flagSetEncoding -1,1),
      0);
    s_flagSetEncoding = 0;
  }

  // EOL mode
  if (0 != s_flagSetEOLMode) {
    SendMessage(
      Globals.hwndMain,
      WM_COMMAND,
      MAKELONG(IDM_LINEENDINGS_CRLF + s_flagSetEOLMode -1,1),
      0);
    s_flagSetEOLMode = 0;
  }

  // Match Text
  if (s_flagMatchText && lpMatchArg) {
    if (StrIsNotEmpty(lpMatchArg) && SendMessage(Globals.hwndEdit,SCI_GETLENGTH,0,0)) {

      WideCharToMultiByte(Encoding_SciCP,0,lpMatchArg,-1,Settings.EFR_Data.szFind,COUNTOF(Settings.EFR_Data.szFind),NULL,NULL);

      if (s_flagMatchText & 4)
        Settings.EFR_Data.fuFlags |= (SCFIND_REGEXP | SCFIND_POSIX);
      else if (s_flagMatchText & 8)
        Settings.EFR_Data.bTransformBS = true;

      if (s_flagMatchText & 2) {
        if (!s_flagJumpTo) { SendMessage(Globals.hwndEdit, SCI_DOCUMENTEND, 0, 0); }
        EditFindPrev(Globals.hwndEdit,&Settings.EFR_Data,false,false);
        EditEnsureSelectionVisible(Globals.hwndEdit);
      }
      else {
        if (!s_flagJumpTo) { SendMessage(Globals.hwndEdit, SCI_DOCUMENTSTART, 0, 0); }
        EditFindNext(Globals.hwndEdit,&Settings.EFR_Data,false,false);
        EditEnsureSelectionVisible(Globals.hwndEdit);
      }
    }
    LocalFree(lpMatchArg);  // StrDup()
    lpMatchArg = NULL;
  }

  // Check for Paste Board option -- after loading files
  if (s_flagPasteBoard) {
    s_bLastCopyFromMe = true;
    s_hwndNextCBChain = SetClipboardViewer(Globals.hwndMain);
    s_uidsAppTitle = IDS_MUI_APPTITLE_PASTEBOARD;
    s_bLastCopyFromMe = false;

    s_dwLastCopyTime = 0;
    SetTimer(Globals.hwndMain,ID_PASTEBOARDTIMER,100,PasteBoardTimer);
  }

  // check if a lexer was specified from the command line
  if (s_flagLexerSpecified) {
    if (lpSchemeArg) {
      Style_SetLexerFromName(Globals.hwndEdit,Globals.CurrentFile,lpSchemeArg);
      LocalFree(lpSchemeArg);  // StrDup()
    }
    else if (s_iInitialLexer >=0 && s_iInitialLexer < NUMLEXERS)
      Style_SetLexerFromID(Globals.hwndEdit,s_iInitialLexer);
    s_flagLexerSpecified = 0;
  }

  // If start as tray icon, set current filename as tooltip
  if (s_flagStartAsTrayIcon)
    SetNotifyIconTitle(Globals.hwndMain);

  Globals.iReplacedOccurrences = 0;
  Globals.iMarkOccurrencesCount = (Settings.MarkOccurrences > 0) ? 0 : -1;

  UpdateToolbar();
  UpdateStatusbar(false);
  UpdateMarginWidth();

  // print file immediately and quit
  if (Flags.PrintFileAndLeave)
  {
    SHFILEINFO shfi;
    WCHAR *pszTitle;
    WCHAR tchUntitled[32] = { L'\0' };
    WCHAR tchPageFmt[32] = { L'\0' };

    if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
      SHGetFileInfo2(Globals.CurrentFile, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
      pszTitle = shfi.szDisplayName;
    }
    else {
      GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
      pszTitle = tchUntitled;
    }

    GetLngString(IDS_MUI_PRINT_PAGENUM, tchPageFmt, COUNTOF(tchPageFmt));

    if (!EditPrint(Globals.hwndEdit, pszTitle, tchPageFmt)) {
      MsgBoxLng(MBWARN, IDS_MUI_PRINT_ERROR, pszTitle);
    }
    PostMessage(Globals.hwndMain, WM_CLOSE, 0, 0);
  }
  return(Globals.hwndMain);
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
    case WM_KEYDOWN:
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    case WM_SYSKEYDOWN:
      if (GetAsyncKeyState(VK_MENU) & SHRT_MIN)  // ALT-KEY DOWN
      {
        if (!bAltKeyIsDown) {
          bAltKeyIsDown = true;
          if (!Settings2.DenyVirtualSpaceAccess) {
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
          SciCall_SetVirtualSpaceOptions(Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION);
        }
      }
      return DefWindowProc(hwnd, umsg, wParam, lParam);

    case WM_KILLFOCUS:
      if (bAltKeyIsDown) {
        bAltKeyIsDown = false;
        SciCall_SetVirtualSpaceOptions(Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION);
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
      MarkAllOccurrences(0, true);
      UpdateVisibleUrlHotspot(0);
      UpdateMarginWidth();
      return DefWindowProc(hwnd,umsg,wParam,lParam);

    case WM_SIZE:
      return MsgSize(hwnd, wParam, lParam);

    case WM_SETFOCUS:
      SetFocus(Globals.hwndEdit);
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
      if (!s_bLastCopyFromMe)
        s_dwLastCopyTime = GetTickCount();
      else
        s_bLastCopyFromMe = false;

      if (s_hwndNextCBChain)
        SendMessage(s_hwndNextCBChain,WM_DRAWCLIPBOARD,wParam,lParam);
      break;

    case WM_CHANGECBCHAIN:
      if ((HWND)wParam == s_hwndNextCBChain)
        s_hwndNextCBChain = (HWND)lParam;
      if (s_hwndNextCBChain)
        SendMessage(s_hwndNextCBChain,WM_CHANGECBCHAIN,lParam,wParam);
      break;

    case WM_SYSCOMMAND:
      return MsgSysCommand(hwnd, umsg, wParam, lParam);
      break;

    case WM_MOUSEWHEEL:
      if (wParam & MK_CONTROL) { EditShowZoomCallTip(Globals.hwndEdit); }
      break;

    case WM_INPUTLANGCHANGE:
      Globals.bIsCJKInputCodePage = IsDBCSCodePage(Scintilla_InputCodePage());
      break;

    default:
      if (umsg == s_msgTaskbarCreated) {
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
static void  _SetWrapStartIndent(HWND hwndEditCtrl)
{
  int i = 0;
  switch (Settings.WordWrapIndent) {
  case 1: i = 1; break;
  case 2: i = 2; break;
  case 3: i = (Settings.IndentWidth) ? 1 * Settings.IndentWidth : 1 * Settings.TabWidth; break;
  case 4: i = (Settings.IndentWidth) ? 2 * Settings.IndentWidth : 2 * Settings.TabWidth; break;
  }
  SendMessage(hwndEditCtrl, SCI_SETWRAPSTARTINDENT, i, 0);
}


//=============================================================================
//
//  _SetWrapIndentMode()
//
static void  _SetWrapIndentMode(HWND hwndEditCtrl)
{
  int const wrap_mode = (!Settings.WordWrap ? SC_WRAP_NONE : ((Settings.WordWrapMode == 0) ? SC_WRAP_WHITESPACE : SC_WRAP_CHAR));

  SendMessage(hwndEditCtrl, SCI_SETWRAPMODE, wrap_mode, 0);

  if (Settings.WordWrapIndent == 5) {
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
  }
  else if (Settings.WordWrapIndent == 6) {
    SendMessage(hwndEditCtrl, SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_INDENT, 0);
  }
  else if (Settings.WordWrapIndent == 7) {
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
static void  _SetWrapVisualFlags(HWND hwndEditCtrl)
{
  if (Settings.ShowWordWrapSymbols) {
    int wrapVisualFlags = 0;
    int wrapVisualFlagsLocation = 0;
    if (Settings.WordWrapSymbols == 0) {
      Settings.WordWrapSymbols = 22;
    }
    switch (Settings.WordWrapSymbols % 10) {
    case 1:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
      wrapVisualFlagsLocation |= SC_WRAPVISUALFLAGLOC_END_BY_TEXT;
      break;
    case 2:
      wrapVisualFlags |= SC_WRAPVISUALFLAG_END;
      break;
    }
    switch (((Settings.WordWrapSymbols % 100) - (Settings.WordWrapSymbols % 10)) / 10) {
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
static void  _InitializeSciEditCtrl(HWND hwndEditCtrl)
{
  if (IsVista()) {
    // Current platforms perform window buffering so it is almost always better for this option to be turned off.
    // There are some older platforms and unusual modes where buffering may still be useful - so keep it ON
    //~SciCall_SetBufferedDraw(true);  // default is true 
    if (Settings.RenderingTechnology > 0) {
      SciCall_SetTechnology(s_DirectWriteTechnology[Settings.RenderingTechnology]);
      SciCall_SetBufferedDraw(false);
      // experimental
      SciCall_SetBidirectional(s_SciBidirectional[Settings.Bidirectional]);
    }
  }
  Encoding_Current(Settings.DefaultEncoding);

  //int const evtMask = SC_MODEVENTMASKALL;
  // The possible notification types are the same as the modificationType bit flags used by SCN_MODIFIED: 
  // SC_MOD_INSERTTEXT, SC_MOD_DELETETEXT, SC_MOD_CHANGESTYLE, SC_MOD_CHANGEFOLD, SC_PERFORMED_USER, 
  // SC_PERFORMED_UNDO, SC_PERFORMED_REDO, SC_MULTISTEPUNDOREDO, SC_LASTSTEPINUNDOREDO, SC_MOD_CHANGEMARKER, 
  // SC_MOD_BEFOREINSERT, SC_MOD_BEFOREDELETE, SC_MULTILINEUNDOREDO, and SC_MODEVENTMASKALL.
  //
  ///~ Don't use: SC_PERFORMED_USER | SC_MOD_CHANGESTYLE; 
  /// SC_MOD_CHANGESTYLE and SC_MOD_CHANGEINDICATOR needs SCI_SETCOMMANDEVENTS=true

  int const evtMask1 = SC_MOD_CONTAINER | SC_PERFORMED_UNDO | SC_PERFORMED_REDO;
  int const evtMask2 = SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE;

  SendMessage(hwndEditCtrl, SCI_SETMODEVENTMASK, (WPARAM)(evtMask1 | evtMask2), 0);

  SendMessage(hwndEditCtrl, SCI_SETCODEPAGE, (WPARAM)SC_CP_UTF8, 0); // fixed internal UTF-8 

  SendMessage(hwndEditCtrl, SCI_SETEOLMODE, Settings.DefaultEOLMode, 0);
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
  // Idle Styling (very large text)
  //~SendMessage(hwndEditCtrl, SCI_SETIDLESTYLING, SC_IDLESTYLING_ALL, 0);  
  SendMessage(hwndEditCtrl, SCI_SETIDLESTYLING, SC_IDLESTYLING_AFTERVISIBLE, 0);
  SendMessage(hwndEditCtrl, SCI_SETLAYOUTCACHE, SC_CACHE_PAGE, 0);
  SendMessage(hwndEditCtrl, SCI_SETCOMMANDEVENTS, false, 0); // speedup folding

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
  if (!Settings2.UseOldStyleBraceMatching) {
    SendMessage(hwndEditCtrl, SCI_BRACEHIGHLIGHTINDICATOR, true, INDIC_NP3_MATCH_BRACE);
    SendMessage(hwndEditCtrl, SCI_BRACEBADLIGHTINDICATOR, true, INDIC_NP3_BAD_BRACE);
  }

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
  if (Settings2.CurrentLineHorizontalSlop > 0)
    SendMessage(hwndEditCtrl, SCI_SETXCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), Settings2.CurrentLineHorizontalSlop);
  else
    SendMessage(hwndEditCtrl, SCI_SETXCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), (LPARAM)0);

  if (Settings2.CurrentLineVerticalSlop > 0) {
    SendMessage(hwndEditCtrl, SCI_SETYCARETPOLICY, (WPARAM)(CARET_SLOP | _CARET_SYMETRY | _CARET_ENFORCE), Settings2.CurrentLineVerticalSlop);
  } else {
    SendMessage(hwndEditCtrl, SCI_SETYCARETPOLICY, (WPARAM)(_CARET_SYMETRY), 0);
  }
  SendMessage(hwndEditCtrl, SCI_SETVIRTUALSPACEOPTIONS, (WPARAM)(Settings2.DenyVirtualSpaceAccess ? SCVS_NONE : SCVS_RECTANGULARSELECTION), 0);
  SendMessage(hwndEditCtrl, SCI_SETENDATLASTLINE, (WPARAM)((Settings.ScrollPastEOF) ? 0 : 1), 0);

  // Tabs
  SendMessage(hwndEditCtrl, SCI_SETUSETABS, (WPARAM)!Settings.TabsAsSpaces, 0);
  SendMessage(hwndEditCtrl, SCI_SETTABINDENTS, (WPARAM)Settings.TabIndents, 0);
  SendMessage(hwndEditCtrl, SCI_SETBACKSPACEUNINDENTS, (WPARAM)Settings.BackspaceUnindents, 0);
  SendMessage(hwndEditCtrl, SCI_SETTABWIDTH, (WPARAM)Settings.TabWidth, 0);
  SendMessage(hwndEditCtrl, SCI_SETINDENT, (WPARAM)Settings.IndentWidth, 0);

  // Indent Guides
  Style_SetIndentGuides(hwndEditCtrl, Settings.ShowIndentGuides);

  // Word Wrap
  _SetWrapIndentMode(hwndEditCtrl);
  _SetWrapVisualFlags(hwndEditCtrl);

  // Long Lines
  if (Settings.MarkLongLines)
    SendMessage(hwndEditCtrl, SCI_SETEDGEMODE, (WPARAM)((Settings.LongLineMode == EDGE_LINE) ? EDGE_LINE : EDGE_BACKGROUND), 0);
  else
    SendMessage(hwndEditCtrl, SCI_SETEDGEMODE, (WPARAM)EDGE_NONE, 0);

  SendMessage(hwndEditCtrl, SCI_SETEDGECOLUMN, (WPARAM)Settings.LongLinesLimit, 0);

  // general margin
  SendMessage(hwndEditCtrl, SCI_SETMARGINOPTIONS, (WPARAM)SC_MARGINOPTION_SUBLINESELECT, 0);

  // Nonprinting characters
  SendMessage(hwndEditCtrl, SCI_SETVIEWWS, (WPARAM)(Settings.ViewWhiteSpace ? SCWS_VISIBLEALWAYS : SCWS_INVISIBLE), 0);
  SendMessage(hwndEditCtrl, SCI_SETVIEWEOL, (WPARAM)Settings.ViewEOLs, 0);

  // IME Interaction
  SendMessage(hwndEditCtrl, SCI_SETIMEINTERACTION, (WPARAM)Settings2.IMEInteraction, 0);

  // word delimiter handling
  EditInitWordDelimiter(hwndEditCtrl);
  EditSetAccelWordNav(hwndEditCtrl, Settings.AccelWordNavigation);

  UpdateMarginWidth();
}


//=============================================================================
//
//  MsgCreate() - Handles WM_CREATE
//
//

LRESULT MsgCreate(HWND hwnd, WPARAM wParam,LPARAM lParam)
{
  UNUSED(wParam);

  HINSTANCE const hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

  Globals.CurrentDPI = GetCurrentDPI(hwnd);
  Globals.CurrentPPI = GetCurrentPPI(hwnd);

  // Setup edit control
  Globals.hwndEdit = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    L"Scintilla",
    NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
    0, 0, 0, 0,
    hwnd,
    (HMENU)IDC_EDIT,
    hInstance,
    NULL);

  Globals.hndlScintilla = (HANDLE)SendMessage(Globals.hwndEdit, SCI_GETDIRECTPOINTER, 0, 0);

  _InitializeSciEditCtrl(Globals.hwndEdit);

  s_hwndEditFrame = CreateWindowEx(
                    WS_EX_CLIENTEDGE,
                    WC_LISTVIEW,
                    NULL,
                    WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                    0,0,100,100,
                    hwnd,
                    (HMENU)IDC_EDITFRAME,
                    hInstance,
                    NULL);

  if (IsAppThemed()) {

    RECT rc, rc2;

    s_bIsAppThemed = true;

    SetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE,GetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(Globals.hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    if (IsVista()) {
      s_cxEditFrame = 0;
      s_cyEditFrame = 0;
    }

    else {
      GetClientRect(s_hwndEditFrame,&rc);
      GetWindowRect(s_hwndEditFrame,&rc2);

      s_cxEditFrame = ((rc2.right-rc2.left) - (rc.right-rc.left)) / 2;
      s_cyEditFrame = ((rc2.bottom-rc2.top) - (rc.bottom-rc.top)) / 2;
    }
  }
  else {
    s_bIsAppThemed = false;

    s_cxEditFrame = 0;
    s_cyEditFrame = 0;
  }

  // Create Toolbar and Statusbar
  CreateBars(hwnd,hInstance);

  // Window Initialization

  (void)CreateWindow(
    WC_STATIC,
    NULL,
    WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
    0,0,10,10,
    hwnd,
    (HMENU)IDC_FILENAME,
    hInstance,
    NULL);

  SetDlgItemText(hwnd,IDC_FILENAME,Globals.CurrentFile);

  (void)CreateWindow(
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
  pDropTarget = RegisterDragAndDrop(hwnd, &cfDrpF, 1, WM_NULL, DropFilesProc, (void*)Globals.hwndEdit);
#endif

  // File MRU
  Globals.pFileMRU = MRU_Create(_s_RecentFiles, MRU_NOCASE, MRU_ITEMSFILE);
  MRU_Load(Globals.pFileMRU);

  Globals.pMRUfind = MRU_Create(_s_RecentFind, (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
  MRU_Load(Globals.pMRUfind);
  SetFindPattern(Globals.pMRUfind->pszItems[0]);

  Globals.pMRUreplace = MRU_Create(_s_RecentReplace, (/*IsWindowsNT()*/true) ? MRU_UTF8 : 0, MRU_ITEMSFNDRPL);
  MRU_Load(Globals.pMRUreplace);

  if (Globals.hwndEdit == NULL || s_hwndEditFrame == NULL ||
    Globals.hwndStatus == NULL || s_hwndToolbar == NULL || s_hwndReBar == NULL)
    return -1LL;

  Style_SetDefaultLexer(Globals.hwndEdit);

  ObserveNotifyChangeEvent();

  return 0;
}


//=============================================================================
//
//  CreateBars() - Create Toolbar and Statusbar
//
//
void CreateBars(HWND hwnd, HINSTANCE hInstance)
{
  DWORD dwToolbarStyle = NP3_WS_TOOLBAR;
  s_hwndToolbar = CreateWindowEx(0,TOOLBARCLASSNAME,NULL,dwToolbarStyle,
                               0,0,0,0,hwnd,(HMENU)IDC_TOOLBAR,hInstance,NULL);

  SendMessage(s_hwndToolbar,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

  // Add Toolbar Bitmap
  HBITMAP hbmp = NULL;
  HBITMAP hbmpCopy = NULL;
  WCHAR szTmp[MAX_PATH] = { L'\0' };
  if (StringCchLenW(s_tchToolbarBitmap,COUNTOF(s_tchToolbarBitmap)))
  {
    if (!SearchPath(NULL,s_tchToolbarBitmap,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),s_tchToolbarBitmap);
    hbmp = LoadImage(NULL,szTmp,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
  }

  if (hbmp) {
    s_bExternalBitmap = true;
  }
  else {
    LPWSTR toolBarIntRes = (s_iHighDpiToolBar > 0) ? MAKEINTRESOURCE(IDR_MAINWNDTB2) : MAKEINTRESOURCE(IDR_MAINWNDTB);
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
  SendMessage(s_hwndToolbar,TB_SETIMAGELIST,0,(LPARAM)himl);

  // Optionally add hot Toolbar Bitmap
  hbmp = NULL;
  if (StringCchLenW(s_tchToolbarBitmapHot,COUNTOF(s_tchToolbarBitmapHot)))
  {
    if (!SearchPath(NULL,s_tchToolbarBitmapHot,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),s_tchToolbarBitmapHot);

    hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    hbmp = ResizeImageForCurrentDPI(hbmp);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(s_hwndToolbar,TB_SETHOTIMAGELIST,0,(LPARAM)himl);
    }
  }

  // Optionally add disabled Toolbar Bitmap
  hbmp = NULL;
  if (StringCchLenW(s_tchToolbarBitmapDisabled,COUNTOF(s_tchToolbarBitmapDisabled)))
  {
    if (!SearchPath(NULL,s_tchToolbarBitmapDisabled,L".bmp",COUNTOF(szTmp),szTmp,NULL))
      StringCchCopy(szTmp,COUNTOF(szTmp),s_tchToolbarBitmapDisabled);

    hbmp = LoadImage(NULL, szTmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    hbmp = ResizeImageForCurrentDPI(hbmp);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(s_hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
      s_bExternalBitmap = true;
    }
  }

  if (!s_bExternalBitmap) {
    bool fProcessed = false;
    if (Flags.ToolbarLook == 1) {
      fProcessed = BitmapAlphaBlend(hbmpCopy, GetSysColor(COLOR_3DFACE), 0x60);
    }
    else if (Flags.ToolbarLook == 2 || (!IsXP() && Flags.ToolbarLook == 0)) {
      fProcessed = BitmapGrayScale(hbmpCopy);
    }
    if (fProcessed && !IsXP()) {
      BitmapMergeAlpha(hbmpCopy, GetSysColor(COLOR_3DFACE));
    }
    if (fProcessed) {
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmpCopy,CLR_DEFAULT);
      SendMessage(s_hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
    }
  }
  if (hbmpCopy) {
    DeleteObject(hbmpCopy);
  }

  // Load toolbar labels
  size_t const len = 32 * 1024;
  WCHAR* pIniSection = AllocMem(len * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pIniSection) {
    int cchIniSection = (int)len;
    LoadIniSection(L"Toolbar Labels", pIniSection, cchIniSection);
    WCHAR tchDesc[256] = { L'\0' };
    WCHAR tchIndex[256] = { L'\0' };
    for (int i = 0; i < COUNTOF(s_tbbMainWnd); ++i) {
      if (s_tbbMainWnd[i].fsStyle == BTNS_SEP) { continue; }

      int n = s_tbbMainWnd[i].iBitmap + 1;
      StringCchPrintf(tchIndex, COUNTOF(tchIndex), L"%02i", n);

      if (IniSectionGetString(pIniSection, tchIndex, L"", tchDesc, COUNTOF(tchDesc)) > 0) {
        s_tbbMainWnd[i].iString = SendMessage(s_hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc);
        s_tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
      }
      else {
        GetLngString(s_tbbMainWnd[i].idCommand, tchDesc, COUNTOF(tchDesc));
        s_tbbMainWnd[i].iString = SendMessage(s_hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc); // tooltip
        s_tbbMainWnd[i].fsStyle &= ~(BTNS_AUTOSIZE | BTNS_SHOWTEXT);
      }
    }
    FreeMem(pIniSection);
  }

  //~SendMessage(s_hwndToolbar, TB_SETMAXTEXTROWS, 0, 0);

  SendMessage(s_hwndToolbar,TB_SETEXTENDEDSTYLE,0,
    (SendMessage(s_hwndToolbar,TB_GETEXTENDEDSTYLE,0,0) | (TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DOUBLEBUFFER)));

  SendMessage(s_hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)s_tbbMainWnd);

  if (Toolbar_SetButtons(s_hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, s_tbbMainWnd, COUNTOF(s_tbbMainWnd)) == 0) {
    SendMessage(s_hwndToolbar, TB_ADDBUTTONS, NUMINITIALTOOLS, (LPARAM)s_tbbMainWnd);
  }
  RECT rc;
  SendMessage(s_hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
  //SendMessage(s_hwndToolbar,TB_SETINDENT,2,0);


  // Create Statusbar 
  DWORD const dwStatusbarStyle = Settings.ShowStatusbar ? (WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE) : (WS_CHILD | WS_CLIPSIBLINGS);
  Globals.hwndStatus = CreateStatusWindow(dwStatusbarStyle,NULL,hwnd,IDC_STATUSBAR);


  // Create ReBar and add Toolbar
  DWORD const dwReBarStyle = Settings.ShowToolbar ? (NP3_WS_REBAR | WS_VISIBLE) : (NP3_WS_REBAR);
  s_hwndReBar = CreateWindowEx(WS_EX_TOOLWINDOW,REBARCLASSNAME,NULL,dwReBarStyle,
                             0,0,0,0,hwnd,(HMENU)IDC_REBAR,hInstance,NULL);

  REBARINFO rbi;
  rbi.cbSize = sizeof(REBARINFO);
  rbi.fMask  = 0;
  rbi.himl   = (HIMAGELIST)NULL;
  SendMessage(s_hwndReBar,RB_SETBARINFO,0,(LPARAM)&rbi);


  REBARBANDINFO rbBand;

  rbBand.cbSize  = sizeof(REBARBANDINFO);
  rbBand.fMask   = /*RBBIM_COLORS | RBBIM_TEXT | RBBIM_BACKGROUND | */
                   RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE /*| RBBIM_SIZE*/;
  //rbBand.fStyle  = /*RBBS_CHILDEDGE |*//* RBBS_BREAK |*/ RBBS_FIXEDSIZE /*| RBBS_GRIPPERALWAYS*/;
  rbBand.fStyle = s_bIsAppThemed ? (RBBS_FIXEDSIZE | RBBS_CHILDEDGE) : RBBS_FIXEDSIZE;
  rbBand.hbmBack = NULL;
  rbBand.lpText     = L"Toolbar";
  rbBand.hwndChild  = s_hwndToolbar;
  rbBand.cxMinChild = (rc.right - rc.left) * COUNTOF(s_tbbMainWnd);
  rbBand.cyMinChild = (rc.bottom - rc.top) + 2 * rc.top;
  rbBand.cx         = 0;
  SendMessage(s_hwndReBar,RB_INSERTBAND,(WPARAM)-1,(LPARAM)&rbBand);

  SetWindowPos(s_hwndReBar,NULL,0,0,0,0,SWP_NOZORDER);
  GetWindowRect(s_hwndReBar,&rc);
  s_cyReBar = rc.bottom - rc.top;

  s_cyReBarFrame = s_bIsAppThemed ? 0 : 2;
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

    DragAcceptFiles(hwnd, true);
#ifdef _EXTRA_DRAG_N_DROP_HANDLER_
    RevokeDragAndDrop(pDropTarget);
#endif

    // Terminate clipboard watching
    if (s_flagPasteBoard) {
      KillTimer(hwnd, ID_PASTEBOARDTIMER);
      ChangeClipboardChain(hwnd, s_hwndNextCBChain);
    }

    // Destroy find / replace dialog
    if (IsWindow(Globals.hwndDlgFindReplace)) {
      DestroyWindow(Globals.hwndDlgFindReplace);
    }
    // Destroy customize schemes
    if (IsWindow(Globals.hwndDlgCustomizeSchemes)) {
      DestroyWindow(Globals.hwndDlgCustomizeSchemes);
    }
    
    // call SaveSettings() when s_hwndToolbar is still valid
    SaveSettings(false);

    if (StringCchLenW(Globals.IniFile,COUNTOF(Globals.IniFile)) != 0) 
    {
      // Cleanup unwanted MRU's
      if (!Settings.SaveRecentFiles) {
        MRU_Empty(Globals.pFileMRU);
        MRU_Save(Globals.pFileMRU);
      }
      else
        MRU_MergeSave(Globals.pFileMRU, true, Flags.RelativeFileMRU, Flags.PortableMyDocs);

      MRU_Destroy(Globals.pFileMRU);

      if (!Settings.SaveFindReplace) {
        MRU_Empty(Globals.pMRUfind);
        MRU_Empty(Globals.pMRUreplace);
        MRU_Save(Globals.pMRUfind);
        MRU_Save(Globals.pMRUreplace);
      }
      else {
        MRU_MergeSave(Globals.pMRUfind, false, false, false);
        MRU_MergeSave(Globals.pMRUreplace, false, false, false);
      }
      MRU_Destroy(Globals.pMRUfind);
      MRU_Destroy(Globals.pMRUreplace);
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
  Globals.CurrentDPI.x = LOWORD(wParam);
  Globals.CurrentDPI.y = HIWORD(wParam);
  Globals.CurrentPPI = GetCurrentPPI(hwnd);

  DocPos const pos = SciCall_GetCurrentPos();

  HINSTANCE hInstance = (HINSTANCE)(INT_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

#if 0
  char buf[128];
  sprintf(buf, "WM_DPICHANGED: dpi=%u,%u  ppi=%u,%u\n", Globals.CurrentDPI.x, Globals.CurrentDPI.y, Globals.CurrentPPI.x, Globals.CurrentPPI.y);
  SendMessage(Globals.hwndEdit, SCI_INSERTTEXT, 0, (LPARAM)buf);
#endif

  Style_ResetCurrentLexer(Globals.hwndEdit);
  SciCall_GotoPos(pos);
  
  // recreate toolbar and statusbar
  Toolbar_GetButtons(s_hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));

  DestroyWindow(s_hwndToolbar);
  DestroyWindow(s_hwndReBar);
  DestroyWindow(Globals.hwndStatus);
  CreateBars(hwnd, hInstance);

  RECT* const rc = (RECT*)lParam;
  SendWMSize(hwnd, rc);

  UpdateUI();
  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateMarginWidth();
  
  return 0;
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

  if (IsAppThemed()) {
    s_bIsAppThemed = true;

    SetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE,GetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(Globals.hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);

    if (IsVista()) {
      s_cxEditFrame = 0;
      s_cyEditFrame = 0;
    }

    else {
      SetWindowPos(s_hwndEditFrame,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
      GetClientRect(s_hwndEditFrame,&rc);
      GetWindowRect(s_hwndEditFrame,&rc2);

      s_cxEditFrame = ((rc2.right-rc2.left) - (rc.right-rc.left)) / 2;
      s_cyEditFrame = ((rc2.bottom-rc2.top) - (rc.bottom-rc.top)) / 2;
    }
  }

  else {
    s_bIsAppThemed = false;

    SetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE,WS_EX_CLIENTEDGE|GetWindowLongPtr(Globals.hwndEdit,GWL_EXSTYLE));
    SetWindowPos(Globals.hwndEdit,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    s_cxEditFrame = 0;
    s_cyEditFrame = 0;
  }

  // recreate toolbar and statusbar
  Toolbar_GetButtons(s_hwndToolbar,IDT_FILE_NEW,Settings.ToolbarButtons,COUNTOF(Settings.ToolbarButtons));

  DestroyWindow(s_hwndToolbar);
  DestroyWindow(s_hwndReBar);
  DestroyWindow(Globals.hwndStatus);
  CreateBars(hwnd,hInstance);

  SendWMSize(hwnd, NULL);

  EditFinalizeStyling(Globals.hwndEdit, -1);

  if (EditToggleView(Globals.hwndEdit, false)) {
    EditToggleView(Globals.hwndEdit, true);
  }
  MarkAllOccurrences(0, true);
  EditUpdateUrlHotspots(Globals.hwndEdit, 0, Sci_GetDocEndPosition(), Settings.HyperlinkHotspot);

  UpdateUI();
  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateMarginWidth();

  return 0;
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

  if (Settings.ShowToolbar)
  {
/*  SendMessage(s_hwndToolbar,WM_SIZE,0,0);
    RECT rc;
    GetWindowRect(s_hwndToolbar,&rc);
    y = (rc.bottom - rc.top);
    cy -= (rc.bottom - rc.top);*/

    //SendMessage(s_hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
    SetWindowPos(s_hwndReBar,NULL,0,0,LOWORD(lParam),s_cyReBar,SWP_NOZORDER);
    // the ReBar automatically sets the correct height
    // calling SetWindowPos() with the height of one toolbar button
    // causes the control not to temporarily use the whole client area
    // and prevents flickering

    //GetWindowRect(s_hwndReBar,&rc);
    y = s_cyReBar + s_cyReBarFrame;    // define
    cy -= s_cyReBar + s_cyReBarFrame;  // border
  }

  if (Settings.ShowStatusbar)
  {
    RECT rc;
    SendMessage(Globals.hwndStatus,WM_SIZE,0,0);
    GetWindowRect(Globals.hwndStatus,&rc);
    cy -= (rc.bottom - rc.top);
  }

  HDWP hdwp = BeginDeferWindowPos(2);

  DeferWindowPos(hdwp,s_hwndEditFrame,NULL,x,y,cx,cy, SWP_NOZORDER | SWP_NOACTIVATE);

  DeferWindowPos(hdwp,Globals.hwndEdit,NULL,x+s_cxEditFrame,y+s_cyEditFrame,
                 cx-2*s_cxEditFrame,cy-2*s_cyEditFrame,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  EndDeferWindowPos(hdwp);

  s_WinCurrentWidth = cx;

  UpdateToolbar();
  UpdateStatusbar(false);
  UpdateMarginWidth();

  return 0;
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
    if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), szBuf))
      FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, tchFile);
  }
  else if (PathFileExists(szBuf)) {
    FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, szBuf);
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
        FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, tchFile);
    }
    else
      FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, szBuf);

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
    LPnp3params params = AllocMem(pcds->cbData, HEAP_ZERO_MEMORY);
    if (params) {
      CopyMemory(params, pcds->lpData, pcds->cbData);

      if (params->flagLexerSpecified)
        s_flagLexerSpecified = 1;

      if (params->flagQuietCreate)
        s_flagQuietCreate = 1;

      if (params->flagFileSpecified) {

        bool bOpened = false;
        Encoding_SrcCmdLn(params->iSrcEncoding);

        if (PathIsDirectory(&params->wchData)) {
          WCHAR tchFile[MAX_PATH] = { L'\0' };
          if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), &params->wchData))
            bOpened = FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, tchFile);
        }

        else
          bOpened = FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, &params->wchData);

        if (bOpened) {

          if (params->flagChangeNotify == 1) {
            Settings.FileWatchingMode = 0;
            Settings.ResetFileWatching = true;
            InstallFileWatching(Globals.CurrentFile);
          }
          else if (params->flagChangeNotify == 2) {
            if (!Globals.bChasingDocTail) {
              SendMessage(Globals.hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
            }
            else {
              Settings.FileWatchingMode = 2;
              Settings.ResetFileWatching = true;
              InstallFileWatching(Globals.CurrentFile);
            }
          }

          if (0 != params->flagSetEncoding) {
            s_flagSetEncoding = params->flagSetEncoding;
            SendMessage(
              hwnd,
              WM_COMMAND,
              MAKELONG(IDM_ENCODING_ANSI + s_flagSetEncoding - 1, 1),
              0);
            s_flagSetEncoding = 0;
          }

          if (0 != params->flagSetEOLMode) {
            s_flagSetEOLMode = params->flagSetEOLMode;
            SendMessage(Globals.hwndMain, WM_COMMAND, MAKELONG(IDM_LINEENDINGS_CRLF + s_flagSetEOLMode - 1, 1), 0);
            s_flagSetEOLMode = 0;
          }

          if (params->flagLexerSpecified) {
            if (params->iInitialLexer < 0) {
              WCHAR wchExt[32] = L".";
              StringCchCopyN(CharNext(wchExt), 32, StrEnd(&params->wchData, 0) + 1, 31);
              Style_SetLexerFromName(Globals.hwndEdit, &params->wchData, wchExt);
            }
            else if (params->iInitialLexer >= 0 && params->iInitialLexer < NUMLEXERS)
              Style_SetLexerFromID(Globals.hwndEdit, params->iInitialLexer);
          }

          if (params->flagTitleExcerpt) {
            StringCchCopyN(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), StrEnd(&params->wchData, 0) + 1, COUNTOF(s_wchTitleExcerpt));
          }
        }
        // reset
        Encoding_SrcCmdLn(CPI_NONE);
      }

      if (params->flagJumpTo) {
        if (params->iInitialLine == 0)
          params->iInitialLine = 1;
        EditJumpTo(Globals.hwndEdit, params->iInitialLine, params->iInitialColumn);
      }

      s_flagLexerSpecified = 0;
      s_flagQuietCreate = 0;

      FreeMem(params);
    }

    UpdateToolbar();
    UpdateStatusbar(false);
    UpdateMarginWidth();
  }

  return 0;
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

  hmenu = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_POPUPMENU));
  //SetMenuDefaultItem(GetSubMenu(hmenu,1),0,false);

  pt.x = (int)(short)LOWORD(lParam);
  pt.y = (int)(short)HIWORD(lParam);

  switch (nID) {
  case IDC_EDIT:
    {
      if (SciCall_IsSelectionEmpty() && (pt.x != -1) && (pt.y != -1)) {
        POINT ptc;
        ptc.x = pt.x;  ptc.y = pt.y;
        ScreenToClient(Globals.hwndEdit, &ptc);
        DocPos iNewPos = SciCall_PositionFromPoint(ptc.x, ptc.y);
        EditSetSelectionEx(Globals.hwndEdit, iNewPos, iNewPos, -1, -1);
      }

      if (pt.x == -1 && pt.y == -1) {
        DocPos iCurrentPos = SciCall_GetCurrentPos();
        pt.x = (LONG)SciCall_PointXFromPosition(iCurrentPos);
        pt.y = (LONG)SciCall_PointYFromPosition(iCurrentPos);
        ClientToScreen(Globals.hwndEdit, &pt);
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
LRESULT MsgChangeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(wParam);
  UNUSED(lParam);

  if (Settings.FileWatchingMode == 1 || IsDocumentModified || Encoding_HasChanged(CPI_GET)) {
    SetForegroundWindow(hwnd);
  }

  if (PathFileExists(Globals.CurrentFile)) 
  {
    if ((Settings.FileWatchingMode == 2 && !IsDocumentModified && !Encoding_HasChanged(CPI_GET)) ||
      MsgBoxLng(MBYESNOWARN,IDS_MUI_FILECHANGENOTIFY) == IDYES)
    {
      FileRevert(Globals.CurrentFile, Encoding_HasChanged(CPI_GET));
      
      if (Globals.bChasingDocTail) 
      {
        SciCall_SetReadOnly(Globals.bChasingDocTail);
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

  if (!s_bRunningWatch) {
    InstallFileWatching(Globals.CurrentFile);
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

      HMENU hMenu = LoadMenu(Globals.hLngResContainer, MAKEINTRESOURCE(IDR_MUI_POPUPMENU));
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


//=============================================================================
//
//  MsgInitMenu() - Handles WM_INITMENU
//
//
LRESULT MsgInitMenu(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  HMENU const hmenu = (HMENU)wParam;

  bool const ro = SciCall_GetReadOnly();
  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocLn  const iCurLine = SciCall_LineFromPosition(iCurPos);

  int i = (int)StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile));

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
  CheckCmd(hmenu,IDM_FILE_READONLY,s_bFileReadOnly);

  EnableCmd(hmenu,IDM_ENCODING_UNICODEREV,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UNICODE,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UTF8SIGN,!ro);
  EnableCmd(hmenu,IDM_ENCODING_UTF8,!ro);
  EnableCmd(hmenu,IDM_ENCODING_ANSI,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_CRLF,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_LF,!ro);
  EnableCmd(hmenu,IDM_LINEENDINGS_CR,!ro);

  EnableCmd(hmenu,IDM_ENCODING_RECODE,i);

  if (Encoding_IsUNICODE_REVERSE(Encoding_Current(CPI_GET))) {
    i = IDM_ENCODING_UNICODEREV;
  }
  else if (Encoding_IsUNICODE(Encoding_Current(CPI_GET))) {
    i = IDM_ENCODING_UNICODE;
  }
  else if (Encoding_IsUTF8_SIGN(Encoding_Current(CPI_GET))) {
    i = IDM_ENCODING_UTF8SIGN;
  }
  else if (Encoding_IsUTF8(Encoding_Current(CPI_GET))) {
    i = IDM_ENCODING_UTF8;
  }
  else if (Encoding_IsANSI(Encoding_Current(CPI_GET))) {
    i = IDM_ENCODING_ANSI;
  }
  else {
    i = -1;
  }
  CheckMenuRadioItem(hmenu,IDM_ENCODING_ANSI,IDM_ENCODING_UTF8SIGN,i,MF_BYCOMMAND);

  int const _eol_mode = SciCall_GetEOLMode();
  if (_eol_mode == SC_EOL_CRLF) {
    i = IDM_LINEENDINGS_CRLF;
  }
  else if (_eol_mode == SC_EOL_CR) {
    i = IDM_LINEENDINGS_CR;
  }
  else {
    i = IDM_LINEENDINGS_LF;
  }
  CheckMenuRadioItem(hmenu,IDM_LINEENDINGS_CRLF,IDM_LINEENDINGS_LF,i,MF_BYCOMMAND);

  EnableCmd(hmenu,IDM_FILE_RECENT,(MRU_Count(Globals.pFileMRU) > 0));

  EnableCmd(hmenu,IDM_EDIT_UNDO,SciCall_CanUndo() && !ro);
  EnableCmd(hmenu,IDM_EDIT_REDO,SciCall_CanRedo() && !ro);

  bool const s = SciCall_IsSelectionEmpty();
  bool const e = (SciCall_GetTextLength() == 0);
  bool const b = SciCall_CanPaste();
  bool const mls = Sci_IsMultiLineSelection();

  EnableCmd(hmenu,IDM_EDIT_CUT, !e && !ro);       // allow Ctrl-X w/o selection
  EnableCmd(hmenu,IDM_EDIT_COPY, !e);             // allow Ctrl-C w/o selection

  EnableCmd(hmenu,IDM_EDIT_COPYALL, !e);
  EnableCmd(hmenu,IDM_EDIT_COPYADD, !e);

  EnableCmd(hmenu,IDM_EDIT_PASTE, b && !ro);
  EnableCmd(hmenu,IDM_EDIT_SWAP, (!s || b) && !ro);
  EnableCmd(hmenu,IDM_EDIT_CLEAR, !s && !ro);

  EnableCmd(hmenu, IDM_EDIT_SELECTALL, !e);
  
  OpenClipboard(hwnd);
  EnableCmd(hmenu,IDM_EDIT_CLEARCLIPBOARD,CountClipboardFormats());
  CloseClipboard();

  EnableCmd(hmenu,IDM_EDIT_MOVELINEUP,!ro);
  EnableCmd(hmenu,IDM_EDIT_MOVELINEDOWN,!ro);
  EnableCmd(hmenu,IDM_EDIT_DUPLINEORSELECTION,!ro);
  EnableCmd(hmenu,IDM_EDIT_LINETRANSPOSE,!ro);
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
  EnableCmd(hmenu, IDM_EDIT_COMPRESS_BLANKS, !ro);

  EnableCmd(hmenu, IDM_EDIT_MODIFYLINES, !ro);
  EnableCmd(hmenu, IDM_EDIT_ALIGN, mls && !ro);
  EnableCmd(hmenu, IDM_EDIT_SORTLINES, mls && !ro);
 
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

  EnableCmd(hmenu,IDM_EDIT_CHAR2HEX, !ro);  // Char2Hex allowed for char after current pos
  EnableCmd(hmenu,IDM_EDIT_HEX2CHAR, !s && !ro);

  //EnableCmd(hmenu,IDM_EDIT_INCREASENUM,!s && !ro);
  //EnableCmd(hmenu,IDM_EDIT_DECREASENUM,!s && !ro);

  EnableCmd(hmenu,IDM_VIEW_SHOWEXCERPT, !s);

  i = SciCall_GetLexer();

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

  EnableCmd(hmenu, IDM_VIEW_FONT, !IsWindow(Globals.hwndDlgCustomizeSchemes));
  EnableCmd(hmenu, IDM_VIEW_CURRENTSCHEME, !IsWindow(Globals.hwndDlgCustomizeSchemes));

  EnableCmd(hmenu, IDM_VIEW_FOLDING, Globals.bCodeFoldingAvailable);
  CheckCmd(hmenu, IDM_VIEW_FOLDING, (Globals.bCodeFoldingAvailable && Settings.ShowCodeFolding));
  EnableCmd(hmenu,IDM_VIEW_TOGGLEFOLDS,!e && (Globals.bCodeFoldingAvailable && Settings.ShowCodeFolding));

  bool const bF = (SC_FOLDLEVELBASE < (SciCall_GetFoldLevel(iCurLine) & SC_FOLDLEVELNUMBERMASK));
  bool const bH = (SciCall_GetFoldLevel(iCurLine) & SC_FOLDLEVELHEADERFLAG);
  EnableCmd(hmenu,IDM_VIEW_TOGGLE_CURRENT_FOLD, !e && (Globals.bCodeFoldingAvailable && Settings.ShowCodeFolding) && (bF || bH));

  CheckCmd(hmenu,IDM_VIEW_USE2NDDEFAULT,Style_GetUse2ndDefault());

  CheckCmd(hmenu,IDM_VIEW_WORDWRAP,Settings.WordWrap);
  CheckCmd(hmenu,IDM_VIEW_LONGLINEMARKER,Settings.MarkLongLines);
  CheckCmd(hmenu,IDM_VIEW_TABSASSPACES,Settings.TabsAsSpaces);
  CheckCmd(hmenu,IDM_VIEW_SHOWINDENTGUIDES,Settings.ShowIndentGuides);
  CheckCmd(hmenu,IDM_VIEW_AUTOINDENTTEXT,Settings.AutoIndent);
  CheckCmd(hmenu,IDM_VIEW_LINENUMBERS,Settings.ShowLineNumbers);
  CheckCmd(hmenu,IDM_VIEW_MARGIN,Settings.ShowSelectionMargin);
  CheckCmd(hmenu,IDM_VIEW_CHASING_DOCTAIL, Globals.bChasingDocTail);

  EnableCmd(hmenu,IDM_EDIT_COMPLETEWORD,!e && !ro);
  CheckCmd(hmenu,IDM_VIEW_AUTOCOMPLETEWORDS,Settings.AutoCompleteWords && !ro);
  CheckCmd(hmenu,IDM_VIEW_AUTOCLEXKEYWORDS, Settings.AutoCLexerKeyWords && !ro);
  
  CheckCmd(hmenu,IDM_VIEW_ACCELWORDNAV,Settings.AccelWordNavigation);

  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_ONOFF, (Settings.MarkOccurrences > 0));
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, Settings.MarkOccurrencesMatchVisible);
  CheckCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, Settings.MarkOccurrencesMatchCase);

  EnableCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, (Settings.MarkOccurrences > 0) && !Settings.MarkOccurrencesMatchVisible);
  CheckCmd(hmenu, IDM_VIEW_TOGGLE_VIEW, EditToggleView(Globals.hwndEdit, false));

  if (Settings.MarkOccurrencesMatchWholeWords) {
    i = IDM_VIEW_MARKOCCUR_WORD;
  }
  else if (Settings.MarkOccurrencesCurrentWord) {
    i = IDM_VIEW_MARKOCCUR_CURRENT;
  }
  else {
    i = IDM_VIEW_MARKOCCUR_WNONE;
  }
  CheckMenuRadioItem(hmenu, IDM_VIEW_MARKOCCUR_WNONE, IDM_VIEW_MARKOCCUR_CURRENT, i, MF_BYCOMMAND);
  CheckCmdPos(GetSubMenu(GetSubMenu(GetMenu(Globals.hwndMain), 2), 17), 5, (i != IDM_VIEW_MARKOCCUR_WNONE));

  i = (Settings.MarkOccurrences > 0);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_VISIBLE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CASE, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_WNONE, i);
  EnableCmd(hmenu,IDM_VIEW_MARKOCCUR_WORD, i);
  EnableCmd(hmenu, IDM_VIEW_MARKOCCUR_CURRENT, i);
  EnableCmdPos(GetSubMenu(GetSubMenu(GetMenu(Globals.hwndMain), 2), 17), 5, i);


  CheckCmd(hmenu,IDM_VIEW_SHOWBLANKS,Settings.ViewWhiteSpace);
  CheckCmd(hmenu,IDM_VIEW_SHOWEOLS,Settings.ViewEOLs);
  CheckCmd(hmenu,IDM_VIEW_WORDWRAPSYMBOLS,Settings.ShowWordWrapSymbols);
  CheckCmd(hmenu,IDM_VIEW_MATCHBRACES,Settings.MatchBraces);
  CheckCmd(hmenu,IDM_VIEW_TOOLBAR,Settings.ShowToolbar);
  EnableCmd(hmenu,IDM_VIEW_CUSTOMIZETB, Settings.ShowToolbar);
  CheckCmd(hmenu,IDM_VIEW_STATUSBAR,Settings.ShowStatusbar);

  i = SciCall_GetLexer();
  //EnableCmd(hmenu,IDM_VIEW_AUTOCLOSETAGS,(i == SCLEX_HTML || i == SCLEX_XML));
  CheckCmd(hmenu, IDM_VIEW_AUTOCLOSETAGS, Settings.AutoCloseTags /*&& (i == SCLEX_HTML || i == SCLEX_XML)*/);
  CheckCmd(hmenu, IDM_VIEW_HIGHLIGHTCURRENTLINE, Settings.HighlightCurrentLine);
  CheckCmd(hmenu, IDM_VIEW_HYPERLINKHOTSPOTS, Settings.HyperlinkHotspot);
  CheckCmd(hmenu, IDM_VIEW_SCROLLPASTEOF, Settings.ScrollPastEOF);
 

  i = Flags.ReuseWindow;
  CheckCmd(hmenu,IDM_VIEW_REUSEWINDOW,i);
  i = Flags.SingleFileInstance;
  CheckCmd(hmenu,IDM_VIEW_SINGLEFILEINSTANCE,i);
  i = Flags.StickyWindowPosition;
  CheckCmd(hmenu,IDM_VIEW_STICKYWINPOS,i);
  CheckCmd(hmenu,IDM_VIEW_ALWAYSONTOP,((Settings.AlwaysOnTop || s_flagAlwaysOnTop == 2) && s_flagAlwaysOnTop != 1));
  CheckCmd(hmenu,IDM_VIEW_MINTOTRAY,Settings.MinimizeToTray);
  CheckCmd(hmenu,IDM_VIEW_TRANSPARENT,Settings.TransparentMode);

  i = IDM_SET_RENDER_TECH_DEFAULT + Settings.RenderingTechnology;
  CheckMenuRadioItem(hmenu, IDM_SET_RENDER_TECH_DEFAULT, IDM_SET_RENDER_TECH_D2DDC, i, MF_BYCOMMAND);
  
  if (Settings.RenderingTechnology > 0) {
    i = IDM_SET_BIDIRECTIONAL_NONE + Settings.Bidirectional;
    CheckMenuRadioItem(hmenu, IDM_SET_BIDIRECTIONAL_NONE, IDM_SET_BIDIRECTIONAL_R2L, i, MF_BYCOMMAND);
  }
  else {
    i = IDM_SET_BIDIRECTIONAL_NONE;
    CheckMenuRadioItem(hmenu, IDM_SET_BIDIRECTIONAL_NONE, IDM_SET_BIDIRECTIONAL_R2L, i, MF_BYCOMMAND);
  }
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_NONE, (Settings.RenderingTechnology > 0));
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_L2R, (Settings.RenderingTechnology > 0));
  EnableCmd(hmenu, IDM_SET_BIDIRECTIONAL_R2L, (Settings.RenderingTechnology > 0));

  CheckCmd(hmenu,IDM_VIEW_NOSAVERECENT,Settings.SaveRecentFiles);
  CheckCmd(hmenu,IDM_VIEW_NOPRESERVECARET, Settings.PreserveCaretPos);
  CheckCmd(hmenu,IDM_VIEW_NOSAVEFINDREPL,Settings.SaveFindReplace);
  CheckCmd(hmenu,IDM_VIEW_SAVEBEFORERUNNINGTOOLS,Settings.SaveBeforeRunningTools);

  CheckCmd(hmenu,IDM_VIEW_CHANGENOTIFY,Settings.FileWatchingMode);

  if (StringCchLenW(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt)))
    i = IDM_VIEW_SHOWEXCERPT;
  else if (Settings.PathNameFormat == 0)
    i = IDM_VIEW_SHOWFILENAMEONLY;
  else if (Settings.PathNameFormat == 1)
    i = IDM_VIEW_SHOWFILENAMEFIRST;
  else
    i = IDM_VIEW_SHOWFULLPATH;
  CheckMenuRadioItem(hmenu,IDM_VIEW_SHOWFILENAMEONLY,IDM_VIEW_SHOWEXCERPT,i,MF_BYCOMMAND);

  if (Settings.EscFunction == 1)
    i = IDM_VIEW_ESCMINIMIZE;
  else if (Settings.EscFunction == 2)
    i = IDM_VIEW_ESCEXIT;
  else
    i = IDM_VIEW_NOESCFUNC;
  CheckMenuRadioItem(hmenu,IDM_VIEW_NOESCFUNC,IDM_VIEW_ESCEXIT,i,MF_BYCOMMAND);

  i = (int)StringCchLenW(Globals.IniFile,COUNTOF(Globals.IniFile));
  CheckCmd(hmenu,IDM_VIEW_SAVESETTINGS,s_bSaveSettings && i);

  EnableCmd(hmenu,IDM_VIEW_REUSEWINDOW,i);
  EnableCmd(hmenu,IDM_VIEW_STICKYWINPOS,i);
  EnableCmd(hmenu,IDM_VIEW_SINGLEFILEINSTANCE,i);
  EnableCmd(hmenu,IDM_VIEW_NOSAVERECENT,i);
  EnableCmd(hmenu,IDM_VIEW_NOPRESERVECARET,i);
  EnableCmd(hmenu,IDM_VIEW_NOSAVEFINDREPL,i);
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGS,s_bEnableSaveSettings && i);

  CheckCmd(hmenu, IDM_VIEW_TOGGLETB, (s_iHighDpiToolBar > 0));
  EnableCmd(hmenu, IDM_VIEW_TOGGLETB, !s_bExternalBitmap);

  i = (StringCchLenW(Globals.IniFile,COUNTOF(Globals.IniFile)) > 0 || StringCchLenW(s_wchIniFile2,COUNTOF(s_wchIniFile2)) > 0);
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGSNOW,s_bEnableSaveSettings && i);

  bool bIsHLink = false;
  int const iHotSpotStyleID = Style_GetHotspotStyleID();
  char const ccStyleAt = SciCall_GetStyleAt(iCurPos);
  if (SciCall_StyleGetHotspot(iHotSpotStyleID)) {
    bIsHLink = (ccStyleAt == (char)iHotSpotStyleID);
  }
  EnableCmd(hmenu, CMD_OPEN_HYPERLINK, bIsHLink);

  i = (int)StringCchLenW(Settings2.AdministrationTool, COUNTOF(Settings2.AdministrationTool));
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
  WCHAR tchMaxPathBuffer[MAX_PATH + 1] = { L'\0' };

  switch(LOWORD(wParam))
  {
    case SCEN_CHANGE:
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, false);
      break;

    case IDT_TIMER_UPDATE_STATUSBAR:
      _UpdateStatusbarDelayed((bool)lParam);
      break;

    case IDT_TIMER_UPDATE_TOOLBAR:
      _UpdateToolbarDelayed();
      break;

    case IDT_TIMER_MAIN_MRKALL:
      EditMarkAllOccurrences(Globals.hwndEdit, (bool)lParam);
      break;

    case IDT_TIMER_UPDATE_HOTSPOT:
      EditUpdateVisibleUrlHotspot(Settings.HyperlinkHotspot);
      break;

    case IDM_FILE_NEW:
      FileLoad(false,true,false,Settings.SkipUnicodeDetection,Settings.SkipANSICodePageDetection,L"");
      break;


    case IDM_FILE_OPEN:
      FileLoad(false,false,false,Settings.SkipUnicodeDetection,Settings.SkipANSICodePageDetection,L"");
      break;


    case IDM_FILE_REVERT:
      if ((IsDocumentModified || Encoding_HasChanged(CPI_GET)) && MsgBoxLng(MBYESNO,IDS_MUI_ASK_REVERT) != IDYES) {
        break;
      }
      FileRevert(Globals.CurrentFile, Encoding_HasChanged(CPI_GET));
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
      if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)))
      {
        DWORD dwFileAttributes = GetFileAttributes(Globals.CurrentFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES) {
          if (s_bFileReadOnly)
            dwFileAttributes = (dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
          else
            dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
          if (!SetFileAttributes(Globals.CurrentFile,dwFileAttributes))
            MsgBoxLng(MBWARN,IDS_MUI_READONLY_MODIFY,Globals.CurrentFile);
        }
        else
          MsgBoxLng(MBWARN,IDS_MUI_READONLY_MODIFY,Globals.CurrentFile);

        dwFileAttributes = GetFileAttributes(Globals.CurrentFile);
        if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
          s_bFileReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);

        UpdateToolbar();
      }
      break;


    case IDM_FILE_BROWSE:
        DialogFileBrowse(hwnd);
      break;


    case IDM_FILE_NEWWINDOW:
    case IDM_FILE_NEWWINDOW2:
      //~SaveSettings(false); 
      DialogNewWindow(hwnd, Settings.SaveBeforeRunningTools, (LOWORD(wParam) != IDM_FILE_NEWWINDOW2));
      break;


    case IDM_FILE_LAUNCH:
      {
        if (!StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)))
          break;

        if (Settings.SaveBeforeRunningTools && !FileSave(false,true,false,false))
          break;

        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
          PathRemoveFileSpec(tchMaxPathBuffer);
        }

        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = Globals.CurrentFile;
        sei.lpParameters = NULL;
        sei.lpDirectory = tchMaxPathBuffer;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RUN:
      {
        if (Settings.SaveBeforeRunningTools && !FileSave(false, true, false, false)) {
          break;
        }
        StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
        PathQuoteSpaces(tchMaxPathBuffer);

        RunDlg(hwnd,tchMaxPathBuffer);
      }
      break;


    case IDM_FILE_OPENWITH:
      if (Settings.SaveBeforeRunningTools && !FileSave(false,true,false,false))
        break;
      OpenWithDlg(hwnd,Globals.CurrentFile);
      break;


    case IDM_FILE_PAGESETUP:
      EditPrintSetup(Globals.hwndEdit);
      break;

    case IDM_FILE_PRINT:
      {
        SHFILEINFO shfi;
        ZeroMemory(&shfi, sizeof(SHFILEINFO));

        WCHAR *pszTitle;
        WCHAR tchUntitled[32] = { L'\0' };
        WCHAR tchPageFmt[32] = { L'\0' };

        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
          SHGetFileInfo2(Globals.CurrentFile,FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
          pszTitle = shfi.szDisplayName;
        }
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszTitle = tchUntitled;
        }

        GetLngString(IDS_MUI_PRINT_PAGENUM,tchPageFmt,COUNTOF(tchPageFmt));

        if (!EditPrint(Globals.hwndEdit,pszTitle,tchPageFmt))
          MsgBoxLng(MBWARN,IDS_MUI_PRINT_ERROR,pszTitle);
      }
      break;


    case IDM_FILE_PROPERTIES:
      {
        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)) == 0)
          break;

        SHELLEXECUTEINFO sei;
        ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        sei.hwnd = hwnd;
        sei.lpVerb = L"properties";
        sei.lpFile = Globals.CurrentFile;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteEx(&sei);
      }
      break;

    case IDM_FILE_CREATELINK:
      {
        if (!StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)))
          break;

        if (!PathCreateDeskLnk(Globals.CurrentFile))
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
            if (OpenFileDlg(Globals.hwndMain, tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),tchMaxPathBuffer))
              FileLoad(true,false,false,Settings.SkipUnicodeDetection,Settings.SkipANSICodePageDetection, tchMaxPathBuffer);
          }
          else
            FileLoad(true,false,false,Settings.SkipUnicodeDetection,Settings.SkipANSICodePageDetection,tchMaxPathBuffer);
          }
        }
      break;


    case IDM_FILE_ADDTOFAV:
      if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
        SHFILEINFO shfi;
        SHGetFileInfo2(Globals.CurrentFile,FILE_ATTRIBUTE_NORMAL,
          &shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
        AddToFavDlg(hwnd,shfi.szDisplayName,Globals.CurrentFile);
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
        sei.lpFile = Settings.FavoritesDir;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;
        // Run favorites directory
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_RECENT:
      if (MRU_Count(Globals.pFileMRU) > 0) {
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
        if (LOWORD(wParam) == IDM_ENCODING_SELECT && !SelectEncodingDlg(hwnd, &iNewEncoding)) {
          break;
        }
        switch (LOWORD(wParam)) 
        {
        case IDM_ENCODING_UNICODE:    iNewEncoding = CPI_UNICODEBOM; break;
        case IDM_ENCODING_UNICODEREV: iNewEncoding = CPI_UNICODEBEBOM; break;
        case IDM_ENCODING_UTF8:       iNewEncoding = CPI_UTF8; break;
        case IDM_ENCODING_UTF8SIGN:   iNewEncoding = CPI_UTF8SIGN; break;
        case IDM_ENCODING_ANSI:       iNewEncoding = CPI_ANSI_DEFAULT; break;
        }
        BeginWaitCursor(NULL);
        _IGNORE_NOTIFY_CHANGE_;
        if (EditSetNewEncoding(Globals.hwndEdit, iNewEncoding, s_flagSetEncoding,
                               StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)) == 0)) {

          if (SendMessage(Globals.hwndEdit,SCI_GETLENGTH,0,0) == 0) {
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
        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) 
        {
          int iNewEncoding = Encoding_MapUnicode(Encoding_Current(CPI_GET));

          if ((IsDocumentModified || Encoding_HasChanged(CPI_GET)) && MsgBoxLng(MBYESNO, IDS_MUI_ASK_RECODE) != IDYES)
            break;

          if (RecodeDlg(hwnd,&iNewEncoding)) 
          {
            StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
            Encoding_SrcCmdLn(iNewEncoding);
            FileLoad(true,false,true,false,true,tchMaxPathBuffer);
          }
        }
      }
      break;


    case IDM_ENCODING_SETDEFAULT:
      SelectDefEncodingDlg(hwnd,&Settings.DefaultEncoding);
      break;


    case IDM_LINEENDINGS_CRLF:
    case IDM_LINEENDINGS_CR:
    case IDM_LINEENDINGS_LF:
      {
        BeginWaitCursor(NULL);
        _IGNORE_NOTIFY_CHANGE_;
        int const _eol_mode = (LOWORD(wParam)-IDM_LINEENDINGS_CRLF); // SC_EOL_CRLF(0), SC_EOL_CR(1), SC_EOL_LF(2)
        SciCall_SetEOLMode(_eol_mode);
        EditEnsureConsistentLineEndings(Globals.hwndEdit);
        _OBSERVE_NOTIFY_CHANGE_;
        EndWaitCursor();
        UpdateStatusbar(false);
      }
      break;


    case IDM_LINEENDINGS_SETDEFAULT:
        SelectDefLineEndingDlg(hwnd, (LPARAM)&Settings.DefaultEOLMode);
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
        if (s_flagPasteBoard)
          s_bLastCopyFromMe = true;

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
      if (s_flagPasteBoard) {
        s_bLastCopyFromMe = true;
      }
      SciCall_CopyAllowLine();
      UpdateToolbar();
      break;


    case IDM_EDIT_COPYALL:
      {
        if (s_flagPasteBoard)
          s_bLastCopyFromMe = true;
        SciCall_CopyRange(0, Sci_GetDocEndPosition());
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_COPYADD:
      {
        if (s_flagPasteBoard)
          s_bLastCopyFromMe = true;
        EditCopyAppend(Globals.hwndEdit,true);
        UpdateToolbar();
      }
      break;

    case IDM_EDIT_PASTE:
      {
        if (s_flagPasteBoard)
          s_bLastCopyFromMe = true;
        _BEGIN_UNDO_ACTION_;
        _IGNORE_NOTIFY_CHANGE_;
        SciCall_Paste();
        _OBSERVE_NOTIFY_CHANGE_;
        _END_UNDO_ACTION_;
        UpdateToolbar();
        UpdateStatusbar(false);
        UpdateMarginWidth();
      }
      break;

    case IDM_EDIT_SWAP:
      {
        if (s_flagPasteBoard)
          s_bLastCopyFromMe = true;
        _BEGIN_UNDO_ACTION_;
        _IGNORE_NOTIFY_CHANGE_;
        EditSwapClipboard(Globals.hwndEdit, Settings.SkipUnicodeDetection);
        _OBSERVE_NOTIFY_CHANGE_;
        _END_UNDO_ACTION_;
        UpdateToolbar();
        UpdateStatusbar(false);
        UpdateMarginWidth();
      }
      break;

    case IDM_EDIT_CLEARCLIPBOARD:
      EditClearClipboard(Globals.hwndEdit);
      UpdateToolbar();
      break;


    case IDM_EDIT_SELECTALL:
        SendMessage(Globals.hwndEdit,SCI_SELECTALL,0,0);
        UpdateToolbar();
        UpdateStatusbar(false);
      break;


    case IDM_EDIT_SELECTWORD:
      {
        DocPos iPos = SciCall_GetCurrentPos();

        if (SendMessage(Globals.hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0)) {

          DocPos iWordStart = SciCall_WordStartPosition(iPos, true);
          DocPos iWordEnd = SciCall_WordEndPosition(iPos, true);

          if (iWordStart == iWordEnd) // we are in whitespace salad...
          {
            iWordStart = SciCall_WordEndPosition(iPos, false);
            iWordEnd   = SciCall_WordEndPosition(iWordStart, true);
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
        EditMoveUp(Globals.hwndEdit);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_MOVELINEDOWN:
      {
        _BEGIN_UNDO_ACTION_;
        EditMoveDown(Globals.hwndEdit);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_DUPLINEORSELECTION:
      _BEGIN_UNDO_ACTION_;
      if (SciCall_IsSelectionEmpty()) { SciCall_LineDuplicate(); } else { SciCall_SelectionDuplicate(); }
      _END_UNDO_ACTION_;
      break;


    case IDM_EDIT_LINETRANSPOSE:
      _BEGIN_UNDO_ACTION_;
      SciCall_LineTranspose();
      _END_UNDO_ACTION_;
      break;

    case IDM_EDIT_CUTLINE:
      {
        if (s_flagPasteBoard) {
          s_bLastCopyFromMe = true;
        }
        _BEGIN_UNDO_ACTION_;
        SendMessage(Globals.hwndEdit,SCI_LINECUT,0,0);
        _END_UNDO_ACTION_;
        UpdateToolbar();
      }
      break;


    case IDM_EDIT_DELETELINE:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_LineDelete();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_DELETELINELEFT:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_DelLineLeft();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_DELETELINERIGHT:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_DelLineRight();
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_INDENT:
      {
        _BEGIN_UNDO_ACTION_;
        EditIndentBlock(Globals.hwndEdit, SCI_TAB, true, false);
        _END_UNDO_ACTION_;
      }
      break;

    case IDM_EDIT_UNINDENT:
      {
        _BEGIN_UNDO_ACTION_;
        EditIndentBlock(Globals.hwndEdit, SCI_BACKTAB, true, false);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_TAB:
      {
        _BEGIN_UNDO_ACTION_;
        EditIndentBlock(Globals.hwndEdit, SCI_TAB, false, false);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_BACKTAB:
      {
        _BEGIN_UNDO_ACTION_;
        EditIndentBlock(Globals.hwndEdit, SCI_BACKTAB, false, false);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_CTRLTAB:
      {
        _BEGIN_UNDO_ACTION_;
        SciCall_SetUseTabs(true);
        SciCall_SetTabIndents(false);
        EditIndentBlock(Globals.hwndEdit, SCI_TAB, false, false);
        SciCall_SetTabIndents(Settings.TabIndents);
        SciCall_SetUseTabs(!Settings.TabsAsSpaces);
        _END_UNDO_ACTION_;
      }
      break;

    case CMD_DELETEBACK:
      {
        ///~_BEGIN_UNDO_ACTION_;
        SendMessage(Globals.hwndEdit, SCI_DELETEBACK, 0, 0);
        ///~_END_UNDO_ACTION_;
      }
      break;

    case CMD_VK_INSERT:
      SendMessage(Globals.hwndEdit, SCI_EDITTOGGLEOVERTYPE, 0, 0);
      UpdateStatusbar(false);
      break;

    case IDM_EDIT_ENCLOSESELECTION:
      if (EditEncloseSelectionDlg(hwnd,s_wchPrefixSelection,s_wchAppendSelection)) {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(Globals.hwndEdit,s_wchPrefixSelection,s_wchAppendSelection);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_PADWITHSPACES:
      {
        BeginWaitCursor(NULL);
        EditPadWithSpaces(Globals.hwndEdit,false,false);
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STRIP1STCHAR:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditStripFirstCharacter(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_STRIPLASTCHAR:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditStripLastCharacter(Globals.hwndEdit, false, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_TRIMLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditStripLastCharacter(Globals.hwndEdit, false, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_COMPRESS_BLANKS:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditCompressBlanks(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_MERGEBLANKLINES:
    {
      BeginWaitCursor(NULL);
      _BEGIN_UNDO_ACTION_;
      EditRemoveBlankLines(Globals.hwndEdit, true, true);
      _END_UNDO_ACTION_;
      EndWaitCursor();
    }
    break;

    case IDM_EDIT_MERGEEMPTYLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditRemoveBlankLines(Globals.hwndEdit, true, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEBLANKLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditRemoveBlankLines(Globals.hwndEdit, false, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEEMPTYLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditRemoveBlankLines(Globals.hwndEdit, false, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_REMOVEDUPLICATELINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditRemoveDuplicateLines(Globals.hwndEdit, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_MODIFYLINES:
      {
        if (EditModifyLinesDlg(hwnd,s_wchPrefixLines,s_wchAppendLines)) {
          BeginWaitCursor(NULL);
          _BEGIN_UNDO_ACTION_;
          EditModifyLines(Globals.hwndEdit,s_wchPrefixLines,s_wchAppendLines);
          _END_UNDO_ACTION_;
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_ALIGN:
      {
        if (EditAlignDlg(hwnd,&s_iAlignMode)) {
          BeginWaitCursor(NULL);
          _BEGIN_UNDO_ACTION_;
          EditAlignText(Globals.hwndEdit,s_iAlignMode);
          _END_UNDO_ACTION_;
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_SORTLINES:
      {
        if (EditSortDlg(hwnd,&s_iSortOptions)) {
          BeginWaitCursor(NULL);
          _BEGIN_UNDO_ACTION_;
          EditSortLines(Globals.hwndEdit,s_iSortOptions);
          _END_UNDO_ACTION_;
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_COLUMNWRAP:
      {
        if (Globals.iWrapCol == 0) {
          Globals.iWrapCol = Settings.LongLinesLimit;
        }

        UINT uWrpCol = 0;
        if (ColumnWrapDlg(hwnd,IDD_MUI_COLUMNWRAP,&uWrpCol))
        {
          Globals.iWrapCol = (DocPos)clampi((int)uWrpCol, 1, Settings.LongLinesLimit);
          BeginWaitCursor(NULL);
          _BEGIN_UNDO_ACTION_;
          EditWrapToColumn(Globals.hwndEdit,Globals.iWrapCol);
          _END_UNDO_ACTION_;
          EndWaitCursor();
        }
      }
      break;


    case IDM_EDIT_SPLITLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditSplitLines(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_JOINLINES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditJoinLinesEx(Globals.hwndEdit, false, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;

    case IDM_EDIT_JOINLN_NOSP:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditJoinLinesEx(Globals.hwndEdit, false, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;

    case IDM_EDIT_JOINLINES_PARA:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditJoinLinesEx(Globals.hwndEdit, true, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTUPPERCASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        SendMessage(Globals.hwndEdit,SCI_UPPERCASE,0,0);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTLOWERCASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        SendMessage(Globals.hwndEdit,SCI_LOWERCASE,0,0);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_INVERTCASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditInvertCase(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_TITLECASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditTitleCase(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_SENTENCECASE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditSentenceCase(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTTABS:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditTabsToSpaces(Globals.hwndEdit, Settings.TabWidth, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTSPACES:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditSpacesToTabs(Globals.hwndEdit, Settings.TabWidth, false);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTTABS2:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditTabsToSpaces(Globals.hwndEdit, Settings.TabWidth, true);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CONVERTSPACES2:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditSpacesToTabs(Globals.hwndEdit, Settings.TabWidth, true);
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
          EditEncloseSelection(Globals.hwndEdit, wszOpen, wszClose);
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
          SendMessage(Globals.hwndEdit,SCI_REPLACESEL,0,(LPARAM)msz);
          _END_UNDO_ACTION_;
        }
      }
      break;


    case IDM_EDIT_INSERT_SHORTDATE:
    case IDM_EDIT_INSERT_LONGDATE:
      {
        WCHAR tchDateTime[128] = { L'\0' };
        WCHAR tchTemplate[128] = { L'\0' };
        SYSTEMTIME st;
        //int   iSelStart;

        GetLocalTime(&st);

        StringCchCopyW(tchTemplate, COUNTOF(tchTemplate),
          (LOWORD(wParam) == IDM_EDIT_INSERT_SHORTDATE) ? Settings2.DateTimeShort : Settings2.DateTimeLong);

        if (StringCchLenW(tchTemplate,0) > 0)
        {
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
          WCHAR tchDate[64] = { L'\0' };
          WCHAR tchTime[64] = { L'\0' };
          GetDateFormat(LOCALE_USER_DEFAULT,(
            LOWORD(wParam) == IDM_EDIT_INSERT_SHORTDATE) ? DATE_SHORTDATE : DATE_LONGDATE,
            &st,NULL,tchDate,COUNTOF(tchDate));
          GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,NULL,tchTime,COUNTOF(tchTime));

          StringCchPrintf(tchDateTime,COUNTOF(tchDateTime),L"%s %s",tchTime,tchDate);
        }
        char chDateTime[128] = { '\0' };
        WideCharToMultiByte(Encoding_SciCP,0,tchDateTime,-1,chDateTime,COUNTOF(chDateTime),NULL,NULL);
        _BEGIN_UNDO_ACTION_;
        SendMessage(Globals.hwndEdit,SCI_REPLACESEL,0,(LPARAM)chDateTime);
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

        if (StringCchLenW(Globals.CurrentFile, COUNTOF(Globals.CurrentFile))) {
          if (LOWORD(wParam) == IDM_EDIT_INSERT_FILENAME) {
            SHGetFileInfo2(Globals.CurrentFile, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO),
                           SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES);
            pszInsert = shfi.szDisplayName;
          }
          else
            pszInsert = Globals.CurrentFile;
        }
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszInsert = tchUntitled;
        }
        char chPath[MAX_PATH + 1];
        WideCharToMultiByte(Encoding_SciCP, 0, pszInsert, -1, chPath, COUNTOF(chPath), NULL, NULL);
        _BEGIN_UNDO_ACTION_;
        SendMessage(Globals.hwndEdit, SCI_REPLACESEL, 0, (LPARAM)chPath);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_INSERT_GUID:
      {
        GUID guid;
        if (SUCCEEDED(CoCreateGuid(&guid))) {  
          if (StringFromGUID2(&guid, tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer))) {
            StrTrimW(tchMaxPathBuffer, L"{}");
            char chMaxPathBuffer[MAX_PATH + 1] = { '\0' };
            if (WideCharToMultiByte(Encoding_SciCP, 0, tchMaxPathBuffer, -1, chMaxPathBuffer, COUNTOF(chMaxPathBuffer), NULL, NULL)) {
              _BEGIN_UNDO_ACTION_;
              SendMessage(Globals.hwndEdit,SCI_REPLACESEL,0,(LPARAM)chMaxPathBuffer);
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

        switch (SciCall_GetLexer()) {
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
          EditToggleLineComments(Globals.hwndEdit, L"//", false);
          break;
        case SCLEX_VBSCRIPT:
        case SCLEX_VB:
          EditToggleLineComments(Globals.hwndEdit, L"'", false);
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
          EditToggleLineComments(Globals.hwndEdit, L"#", true);
          break;
        case SCLEX_ASM:
        case SCLEX_PROPERTIES:
        case SCLEX_AU3:
        case SCLEX_AHK:
        case SCLEX_NSIS: // # could also be used instead
        case SCLEX_INNOSETUP:
        case SCLEX_REGISTRY:
          EditToggleLineComments(Globals.hwndEdit, L";", true);
          break;
        case SCLEX_SQL:
        case SCLEX_LUA:
        case SCLEX_VHDL:
          EditToggleLineComments(Globals.hwndEdit, L"--", true);
          break;
        case SCLEX_BATCH:
          EditToggleLineComments(Globals.hwndEdit, L"rem ", true);
          break;
        case SCLEX_LATEX:
        case SCLEX_MATLAB:
          EditToggleLineComments(Globals.hwndEdit, L"%", true);
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

        switch (SciCall_GetLexer()) {
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
          EditEncloseSelection(Globals.hwndEdit, L"/*", L"*/");
          break;
        case SCLEX_PASCAL:
        case SCLEX_INNOSETUP:
          EditEncloseSelection(Globals.hwndEdit, L"{", L"}");
          break;
        case SCLEX_LUA:
          EditEncloseSelection(Globals.hwndEdit, L"--[[", L"]]");
          break;
        case SCLEX_COFFEESCRIPT:
          EditEncloseSelection(Globals.hwndEdit, L"###", L"###");
          break;
        case SCLEX_MATLAB:
          EditEncloseSelection(Globals.hwndEdit, L"%{", L"%}");
        }
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_URLENCODE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditURLEncode(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_URLDECODE:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditURLDecode(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_ESCAPECCHARS:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditEscapeCChars(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_UNESCAPECCHARS:
      {
        BeginWaitCursor(NULL);
        _BEGIN_UNDO_ACTION_;
        EditUnescapeCChars(Globals.hwndEdit);
        _END_UNDO_ACTION_;
        EndWaitCursor();
      }
      break;


    case IDM_EDIT_CHAR2HEX:
      {
        _BEGIN_UNDO_ACTION_;
        EditChar2Hex(Globals.hwndEdit);
        _END_UNDO_ACTION_;
      }
      break;


    case IDM_EDIT_HEX2CHAR:
      EditHex2Char(Globals.hwndEdit);
      break;


    case IDM_EDIT_FINDMATCHINGBRACE:
      EditFindMatchingBrace(Globals.hwndEdit);
      break;


    case IDM_EDIT_SELTOMATCHINGBRACE:
      {
        _BEGIN_UNDO_ACTION_;
        EditSelectToMatchingBrace(Globals.hwndEdit);
        _END_UNDO_ACTION_;
      }
      break;


    // Main Bookmark Functions
    case BME_EDIT_BOOKMARKNEXT:
    {
        const DocPos iPos = SciCall_GetCurrentPos();
        const DocLn iLine = SciCall_LineFromPosition(iPos);

        int bitmask = (1 << MARKER_NP3_BOOKMARK);
        DocLn iNextLine = (DocLn)SendMessage( Globals.hwndEdit , SCI_MARKERNEXT , iLine+1 , bitmask );
        if (iNextLine == (DocLn)-1)
        {
            iNextLine = (DocLn)SendMessage( Globals.hwndEdit , SCI_MARKERNEXT , 0 , bitmask );
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
        DocLn iNextLine = (DocLn)SendMessage( Globals.hwndEdit , SCI_MARKERPREVIOUS , iLine-1 , bitmask );
        if (iNextLine == (DocLn)-1)
        {
            iNextLine = (DocLn)SendMessage( Globals.hwndEdit , SCI_MARKERPREVIOUS , SciCall_GetLineCount(), bitmask );
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
          SciCall_MarkerDelete(iLine, MARKER_NP3_BOOKMARK); // unset
        }
        else {
          SciCall_MarkerAdd(iLine, MARKER_NP3_BOOKMARK);    // set
        }
        break;
      }

    case BME_EDIT_BOOKMARKCLEAR:
      SciCall_MarkerDeleteAll(MARKER_NP3_BOOKMARK);
    break;



    case IDM_EDIT_FIND:
      if (!IsWindow(Globals.hwndDlgFindReplace)) {
        Globals.bFindReplCopySelOrClip = true;
        Globals.hwndDlgFindReplace = EditFindReplaceDlg(Globals.hwndEdit, &Settings.EFR_Data, false);
      }
      else {
        Globals.bFindReplCopySelOrClip = (GetForegroundWindow() != Globals.hwndDlgFindReplace);
        if (GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE)) {
          SendMessage(Globals.hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOFIND, 1), 0);
          DestroyWindow(Globals.hwndDlgFindReplace);
          Globals.hwndDlgFindReplace = EditFindReplaceDlg(Globals.hwndEdit, &Settings.EFR_Data, false);
        }
        else {
          SetForegroundWindow(Globals.hwndDlgFindReplace);
          PostMessage(Globals.hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(Globals.hwndDlgFindReplace, IDC_FINDTEXT)), 1);
        }
        UpdateStatusbar(false);
      }
      break;


    case IDM_EDIT_REPLACE:
      if (!IsWindow(Globals.hwndDlgFindReplace)) {
        Globals.bFindReplCopySelOrClip = true;
        Globals.hwndDlgFindReplace = EditFindReplaceDlg(Globals.hwndEdit, &Settings.EFR_Data, true);
      }
      else {
        Globals.bFindReplCopySelOrClip = (GetForegroundWindow() != Globals.hwndDlgFindReplace);
        if (!GetDlgItem(Globals.hwndDlgFindReplace, IDC_REPLACE)) {
          SendMessage(Globals.hwndDlgFindReplace, WM_COMMAND, MAKELONG(IDMSG_SWITCHTOREPLACE, 1), 0);
          DestroyWindow(Globals.hwndDlgFindReplace);
          Globals.hwndDlgFindReplace = EditFindReplaceDlg(Globals.hwndEdit, &Settings.EFR_Data, true);
        }
        else {
          SetForegroundWindow(Globals.hwndDlgFindReplace);
          PostMessage(Globals.hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(Globals.hwndDlgFindReplace, IDC_FINDTEXT)), 1);
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

      if (IsFindPatternEmpty() && !StringCchLenA(Settings.EFR_Data.szFind, COUNTOF(Settings.EFR_Data.szFind)))
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
              EditJumpToSelectionEnd(Globals.hwndEdit);
            }
            EditFindNext(Globals.hwndEdit,&Settings.EFR_Data,false,false);
            break;

          case IDM_EDIT_FINDPREV:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionStart(Globals.hwndEdit);
            }
            EditFindPrev(Globals.hwndEdit,&Settings.EFR_Data,false,false);
            break;

          case IDM_EDIT_REPLACENEXT:
            if (Globals.bReplaceInitialized)
              EditReplace(Globals.hwndEdit,&Settings.EFR_Data);
            else
              SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_EDIT_REPLACE,1),0);
            break;

          case IDM_EDIT_SELTONEXT:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionEnd(Globals.hwndEdit);
            }
            EditFindNext(Globals.hwndEdit,&Settings.EFR_Data,true,false);
            break;

          case IDM_EDIT_SELTOPREV:
            if (!SciCall_IsSelectionEmpty()) {
              EditJumpToSelectionStart(Globals.hwndEdit);
            }
            EditFindPrev(Globals.hwndEdit,&Settings.EFR_Data,true,false);
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

        StringCchCopyA(Settings.EFR_Data.szFind, COUNTOF(Settings.EFR_Data.szFind), mszSelection);
        Settings.EFR_Data.fuFlags &= (~(SCFIND_REGEXP | SCFIND_POSIX));
        Settings.EFR_Data.bTransformBS = false;

        WCHAR wszBuf[FNDRPL_BUFFER];
        MultiByteToWideChar(Encoding_SciCP, 0, mszSelection, -1, wszBuf, FNDRPL_BUFFER);
        MRU_Add(Globals.pMRUfind, wszBuf, 0, 0, NULL);
        SetFindPattern(wszBuf);

        switch (LOWORD(wParam)) {

        case IDM_EDIT_SAVEFIND:
          break;

        case CMD_FINDNEXTSEL:
          if (!SciCall_IsSelectionEmpty()) {
            EditJumpToSelectionEnd(Globals.hwndEdit);
          }
          EditFindNext(Globals.hwndEdit, &Settings.EFR_Data, false, false);
          break;

        case CMD_FINDPREVSEL:
          if (!SciCall_IsSelectionEmpty()) {
            EditJumpToSelectionStart(Globals.hwndEdit);
          }
          EditFindPrev(Globals.hwndEdit, &Settings.EFR_Data, false, false);
          break;
        }
      }
    }
    break;


    case IDM_EDIT_COMPLETEWORD:
      EditAutoCompleteWord(Globals.hwndEdit, true);
      break;


    case IDM_EDIT_GOTOLINE:
      EditLinenumDlg(Globals.hwndEdit);
      UpdateStatusbar(false);
      break;


    case IDM_VIEW_SCHEME:
      Style_SelectLexerDlg(Globals.hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      break;


    case IDM_VIEW_USE2NDDEFAULT:
      Style_ToggleUse2ndDefault(Globals.hwndEdit);
      UpdateToolbar();
      UpdateStatusbar(false);
      break;


    case IDM_VIEW_SCHEMECONFIG:
      if (!IsWindow(Globals.hwndDlgCustomizeSchemes)) {
        Globals.hwndDlgCustomizeSchemes = Style_CustomizeSchemesDlg(Globals.hwndEdit);
      }
      else {
        SetForegroundWindow(Globals.hwndDlgCustomizeSchemes);
      }
      PostMessage(Globals.hwndDlgCustomizeSchemes, WM_COMMAND, MAKELONG(IDC_SETCURLEXERTV, 1), 0);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateMarginWidth();
      break;


    case IDM_VIEW_FONT:
      if (!IsWindow(Globals.hwndDlgCustomizeSchemes))
        Style_SetDefaultFont(Globals.hwndEdit, true);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateMarginWidth();
      break;

    case IDM_VIEW_CURRENTSCHEME:
      if (!IsWindow(Globals.hwndDlgCustomizeSchemes))
        Style_SetDefaultFont(Globals.hwndEdit, false);
      UpdateToolbar();
      UpdateStatusbar(false);
      UpdateMarginWidth();
      break;


    case IDM_VIEW_WORDWRAP:
      Settings.WordWrap = !Settings.WordWrap;
      _SetWrapIndentMode(Globals.hwndEdit);
      EditEnsureSelectionVisible(Globals.hwndEdit);
      Globals.bWordWrap = Settings.WordWrap;
      UpdateToolbar();
      break;


    case IDM_VIEW_WORDWRAPSETTINGS:
      if (WordWrapSettingsDlg(hwnd,IDD_MUI_WORDWRAP, &Settings.WordWrapIndent)) {
        _SetWrapIndentMode(Globals.hwndEdit);
        _SetWrapVisualFlags(Globals.hwndEdit);
      }
      break;


    case IDM_VIEW_WORDWRAPSYMBOLS:
      Settings.ShowWordWrapSymbols = !Settings.ShowWordWrapSymbols;
      _SetWrapVisualFlags(Globals.hwndEdit);
      break;


    case IDM_VIEW_LONGLINEMARKER:
      Settings.MarkLongLines = !Settings.MarkLongLines;
      if (Settings.MarkLongLines) {
        SendMessage(Globals.hwndEdit,SCI_SETEDGEMODE,(Settings.LongLineMode == EDGE_LINE)?EDGE_LINE:EDGE_BACKGROUND,0);
        Style_SetLongLineColors(Globals.hwndEdit);
      }
      else
        SendMessage(Globals.hwndEdit,SCI_SETEDGEMODE,EDGE_NONE,0);

      UpdateToolbar();
      UpdateStatusbar(false);
      break;


    case IDM_VIEW_LONGLINESETTINGS:
      if (LongLineSettingsDlg(hwnd,IDD_MUI_LONGLINES,&Settings.LongLinesLimit)) {
        Settings.MarkLongLines = true;
        SendMessage(Globals.hwndEdit, SCI_SETEDGEMODE, (Settings.LongLineMode == EDGE_LINE) ? EDGE_LINE : EDGE_BACKGROUND, 0);
        Style_SetLongLineColors(Globals.hwndEdit);
        Settings.LongLinesLimit = clampi(Settings.LongLinesLimit, 0, LONG_LINES_MARKER_LIMIT);
        SendMessage(Globals.hwndEdit,SCI_SETEDGECOLUMN,Settings.LongLinesLimit,0);
        Globals.iLongLinesLimit = Settings.LongLinesLimit;
        UpdateToolbar();
        UpdateStatusbar(false);
      }
      break;


    case IDM_VIEW_TABSASSPACES:
      Settings.TabsAsSpaces = !Settings.TabsAsSpaces;
      SendMessage(Globals.hwndEdit,SCI_SETUSETABS,!Settings.TabsAsSpaces,0);
      Globals.bTabsAsSpaces = Settings.TabsAsSpaces;
      break;


    case IDM_VIEW_TABSETTINGS:
      if (TabSettingsDlg(hwnd,IDD_MUI_TABSETTINGS,NULL))
      {
        SciCall_SetUseTabs(!Settings.TabsAsSpaces);
        SciCall_SetTabIndents(Settings.TabIndents);
        SciCall_SetBackSpaceUnIndents(Settings.BackspaceUnindents);
        Settings.TabWidth = clampi(Settings.TabWidth, 1, 256);
        Settings.IndentWidth = clampi(Settings.IndentWidth, 0, 256);
        SciCall_SetTabWidth(Settings.TabWidth);
        SciCall_SetIndent(Settings.IndentWidth);
        Globals.bTabsAsSpaces = Settings.TabsAsSpaces;
        Globals.bTabIndents   = Settings.TabIndents;
        Globals.iTabWidth     = Settings.TabWidth;
        Globals.iIndentWidth  = Settings.IndentWidth;
        if (SendMessage(Globals.hwndEdit, SCI_GETWRAPINDENTMODE, 0, 0) == SC_WRAPINDENT_FIXED) {
          _SetWrapStartIndent(Globals.hwndEdit);
        }
      }
      break;


    case IDM_VIEW_SHOWINDENTGUIDES:
      Settings.ShowIndentGuides = !Settings.ShowIndentGuides;
      Style_SetIndentGuides(Globals.hwndEdit,Settings.ShowIndentGuides);
      break;


    case IDM_VIEW_AUTOINDENTTEXT:
      Settings.AutoIndent = !Settings.AutoIndent;
      break;


    case IDM_VIEW_LINENUMBERS:
      Settings.ShowLineNumbers = !Settings.ShowLineNumbers;
      UpdateMarginWidth();
      break;


    case IDM_VIEW_MARGIN:
      Settings.ShowSelectionMargin = !Settings.ShowSelectionMargin;
      UpdateMarginWidth();
      break;

    case IDM_VIEW_AUTOCOMPLETEWORDS:
      Settings.AutoCompleteWords = !Settings.AutoCompleteWords;
      SciCall_AutoCCancel();
      break;

    case IDM_VIEW_AUTOCLEXKEYWORDS:
      Settings.AutoCLexerKeyWords = !Settings.AutoCLexerKeyWords;
      SciCall_AutoCCancel();
      break;

    case IDM_VIEW_ACCELWORDNAV:
      Settings.AccelWordNavigation = !Settings.AccelWordNavigation;
      EditSetAccelWordNav(Globals.hwndEdit,Settings.AccelWordNavigation);
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_ONOFF:
      Settings.MarkOccurrences = (Settings.MarkOccurrences == 0) ? max_i(1, IniGetInt(L"Settings", L"MarkOccurrences", 1)) : 0;
      MarkAllOccurrences(0, true);
      EnableCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, (Settings.MarkOccurrences > 0) && !Settings.MarkOccurrencesMatchVisible);
      break;

    case IDM_VIEW_MARKOCCUR_VISIBLE:
      Settings.MarkOccurrencesMatchVisible = !Settings.MarkOccurrencesMatchVisible;
      MarkAllOccurrences(0, true);
      EnableCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, (Settings.MarkOccurrences > 0) && !Settings.MarkOccurrencesMatchVisible);
      break;

    case IDM_VIEW_TOGGLE_VIEW:
      if (EditToggleView(Globals.hwndEdit, false)) {
        EditToggleView(Globals.hwndEdit, true);
        MarkAllOccurrences(0, true);
      }
      else {
        EditToggleView(Globals.hwndEdit, true);
      }
      CheckCmd(GetMenu(hwnd), IDM_VIEW_TOGGLE_VIEW, EditToggleView(Globals.hwndEdit, false));
      UpdateToolbar();
      break;

    case IDM_VIEW_MARKOCCUR_CASE:
      Settings.MarkOccurrencesMatchCase = !Settings.MarkOccurrencesMatchCase;
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_WNONE:
      Settings.MarkOccurrencesMatchWholeWords = false;
      Settings.MarkOccurrencesCurrentWord = false;
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_WORD:
      Settings.MarkOccurrencesMatchWholeWords = true;
      Settings.MarkOccurrencesCurrentWord = false;
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_MARKOCCUR_CURRENT:
      Settings.MarkOccurrencesMatchWholeWords = false;
      Settings.MarkOccurrencesCurrentWord = true;
      MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
      break;

    case IDM_VIEW_FOLDING:
      Settings.ShowCodeFolding = !Settings.ShowCodeFolding;
      Style_SetFolding(Globals.hwndEdit, Settings.ShowCodeFolding);
      if (!Settings.ShowCodeFolding) { EditToggleFolds(EXPAND, true); }
      UpdateToolbar();
      break;


    case IDM_VIEW_TOGGLEFOLDS:
      EditToggleFolds(SNIFF, SciCall_IsSelectionEmpty());
      break;
      
    case IDM_VIEW_TOGGLE_CURRENT_FOLD:
      EditToggleFolds(SNIFF, false);
      break;

    case IDM_VIEW_SHOWBLANKS:
      Settings.ViewWhiteSpace = !Settings.ViewWhiteSpace;
      SendMessage(Globals.hwndEdit,SCI_SETVIEWWS,(Settings.ViewWhiteSpace)?SCWS_VISIBLEALWAYS:SCWS_INVISIBLE,0);
      break;


    case IDM_VIEW_SHOWEOLS:
      Settings.ViewEOLs = !Settings.ViewEOLs;
      SendMessage(Globals.hwndEdit,SCI_SETVIEWEOL,Settings.ViewEOLs,0);
      break;


    case IDM_VIEW_MATCHBRACES:
      Settings.MatchBraces = !Settings.MatchBraces;
      if (Settings.MatchBraces)
        EditMatchBrace(Globals.hwndEdit);
      else
        SciCall_BraceHighLight(INVALID_POSITION, INVALID_POSITION);
      break;


    case IDM_VIEW_AUTOCLOSETAGS:
      Settings.AutoCloseTags = !Settings.AutoCloseTags;
      break;

    case IDM_VIEW_HIGHLIGHTCURRENTLINE:
      Settings.HighlightCurrentLine = !Settings.HighlightCurrentLine;
      Style_SetCurrentLineBackground(Globals.hwndEdit, Settings.HighlightCurrentLine);
      break;

    case IDM_VIEW_HYPERLINKHOTSPOTS:
      Settings.HyperlinkHotspot = !Settings.HyperlinkHotspot;
      Style_SetUrlHotSpot(Globals.hwndEdit, Settings.HyperlinkHotspot);
      if (Settings.HyperlinkHotspot) {
        UpdateVisibleUrlHotspot(0);
      }
      else {
        SciCall_StartStyling(0);
        Style_ResetCurrentLexer(Globals.hwndEdit);
      }
      break;

    case IDM_VIEW_ZOOMIN:
      {
        SciCall_ZoomIn();
        UpdateMarginWidth();
        EditShowZoomCallTip(Globals.hwndEdit);
      }
      break;

    case IDM_VIEW_ZOOMOUT:
      {
        SciCall_ZoomOut();
        UpdateMarginWidth();
        EditShowZoomCallTip(Globals.hwndEdit);
      }
      break;

    case IDM_VIEW_RESETZOOM:
      {
        SciCall_SetZoom(100);
        UpdateMarginWidth();
        EditShowZoomCallTip(Globals.hwndEdit);
      }
      break;

    case IDM_VIEW_CHASING_DOCTAIL: 
      {
        static int flagPrevChangeNotify = 0;
        static int iPrevFileWatchingMode = 0;
        static bool bPrevResetFileWatching = false;

        Globals.bChasingDocTail = !Globals.bChasingDocTail;
        SciCall_SetReadOnly(Globals.bChasingDocTail);

        if (Globals.bChasingDocTail) 
        {
          SetForegroundWindow(hwnd);
          flagPrevChangeNotify = s_flagChangeNotify;
          iPrevFileWatchingMode = Settings.FileWatchingMode;
          bPrevResetFileWatching = Settings.ResetFileWatching;
          s_flagChangeNotify = 2;
          Settings.FileWatchingMode = 2;
          Settings.ResetFileWatching = true;
        }
        else {
          s_flagChangeNotify = flagPrevChangeNotify;
          Settings.FileWatchingMode = iPrevFileWatchingMode;
          Settings.ResetFileWatching = bPrevResetFileWatching;
        }
        if (!s_bRunningWatch) { InstallFileWatching(Globals.CurrentFile); }

        CheckCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CHASING_DOCTAIL, Globals.bChasingDocTail);
        UpdateToolbar();
      }
      break;
    
    case IDM_VIEW_SCROLLPASTEOF:
      Settings.ScrollPastEOF = !Settings.ScrollPastEOF;
      SciCall_SetEndAtLastLine(!Settings.ScrollPastEOF);
      break;

    case IDM_VIEW_TOOLBAR:
      Settings.ShowToolbar = !Settings.ShowToolbar;
      ShowWindow(s_hwndReBar, (Settings.ShowToolbar ? SW_SHOW : SW_HIDE));
      UpdateToolbar();
      SendWMSize(hwnd, NULL);
      break;

    case IDM_VIEW_TOGGLETB:
      s_iHighDpiToolBar = (s_iHighDpiToolBar <= 0) ? 1 : 0;
      SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
      break;

    case IDM_VIEW_CUSTOMIZETB:
      SendMessage(s_hwndToolbar,TB_CUSTOMIZE,0,0);
      break;

    case IDM_VIEW_STATUSBAR:
      Settings.ShowStatusbar = !Settings.ShowStatusbar;
      ShowWindow(Globals.hwndStatus, (Settings.ShowStatusbar ? SW_SHOW : SW_HIDE));
      UpdateStatusbar(Settings.ShowStatusbar);
      SendWMSize(hwnd, NULL);
      break;


    case IDM_VIEW_STICKYWINPOS:

      if (Flags.StickyWindowPosition == 0)
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

        // GetWindowPlacement
        WININFO wi = GetMyWindowPlacement(Globals.hwndMain,NULL);
        IniSetInt(L"Window",tchPosX,wi.x);
        IniSetInt(L"Window",tchPosY,wi.y);
        IniSetInt(L"Window",tchSizeX,wi.cx);
        IniSetInt(L"Window",tchSizeY,wi.cy);
        IniSetBool(L"Window",tchMaximized,wi.max);
        IniSetInt(L"Window", tchZoom, wi.zoom);

        Flags.StickyWindowPosition = 1;
        InfoBoxLng(0,L"MsgStickyWinPos",IDS_MUI_STICKYWINPOS);
      }
      else {
        Flags.StickyWindowPosition = 0;
      }
      
      if (Flags.StickyWindowPosition != DefaultFlags.StickyWindowPosition)
        IniSetInt(L"Settings2", L"StickyWindowPosition", Flags.StickyWindowPosition);
      else
        IniSetString(L"Settings2", L"StickyWindowPosition", NULL);

      break;


    case IDM_VIEW_REUSEWINDOW:
      Flags.ReuseWindow = (Flags.ReuseWindow != 0) ? 0 : 1; // reverse
      if (Flags.ReuseWindow != DefaultFlags.ReuseWindow)
        IniSetInt(L"Settings2", L"ReuseWindow", Flags.ReuseWindow);
      else
        IniSetString(L"Settings2", L"ReuseWindow", NULL);
      break;


    case IDM_VIEW_SINGLEFILEINSTANCE:
      Flags.SingleFileInstance = (Flags.SingleFileInstance != 0) ? 0 : 1; // reverse
      if (Flags.SingleFileInstance != DefaultFlags.SingleFileInstance)
        IniSetInt(L"Settings2", L"SingleFileInstance", Flags.SingleFileInstance);
      else
        IniSetString(L"Settings2", L"SingleFileInstance", NULL);
      break;


    case IDM_VIEW_ALWAYSONTOP:
      if ((Settings.AlwaysOnTop || s_flagAlwaysOnTop == 2) && s_flagAlwaysOnTop != 1) {
        Settings.AlwaysOnTop = false;
        s_flagAlwaysOnTop = 0;
        SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
      else {
        Settings.AlwaysOnTop = true;
        s_flagAlwaysOnTop = 0;
        SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
      break;


    case IDM_VIEW_MINTOTRAY:
      Settings.MinimizeToTray = !Settings.MinimizeToTray;
      break;


    case IDM_VIEW_TRANSPARENT:
      Settings.TransparentMode = !Settings.TransparentMode;
      SetWindowTransparentMode(hwnd,Settings.TransparentMode, Settings2.OpacityLevel);
      break;

    case IDM_SET_RENDER_TECH_DEFAULT:
    case IDM_SET_RENDER_TECH_D2D:
    case IDM_SET_RENDER_TECH_D2DRETAIN:
    case IDM_SET_RENDER_TECH_D2DDC:
      Settings.RenderingTechnology = (int)LOWORD(wParam) - IDM_SET_RENDER_TECH_DEFAULT;
      if (Settings.RenderingTechnology == 0) {
        SciCall_SetBidirectional(s_SciBidirectional[0]);
      }
      SciCall_SetBufferedDraw((Settings.RenderingTechnology == 0));
      SciCall_SetTechnology(s_DirectWriteTechnology[Settings.RenderingTechnology]);
      break;

    case IDM_SET_BIDIRECTIONAL_NONE:
    case IDM_SET_BIDIRECTIONAL_L2R:
    case IDM_SET_BIDIRECTIONAL_R2L:
      Settings.Bidirectional = (int)LOWORD(wParam) - IDM_SET_BIDIRECTIONAL_NONE;
      SciCall_SetBidirectional(s_SciBidirectional[Settings.Bidirectional]);
      break;

    //case IDM_SET_INLINE_IME:
    //  Settings2.IMEInteraction = (Settings2.IMEInteraction == SC_IME_WINDOWED) ? SC_IME_INLINE : SC_IME_WINDOWED;
    //  SciCall_SetIMEInteraction(Settings2.IMEInteraction);
    //  break;

    case IDM_VIEW_SHOWFILENAMEONLY:
    case IDM_VIEW_SHOWFILENAMEFIRST:
    case IDM_VIEW_SHOWFULLPATH:
      Settings.PathNameFormat = (int)LOWORD(wParam) - IDM_VIEW_SHOWFILENAMEONLY;
      StringCchCopy(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt),L"");
      UpdateToolbar();
      break;


    case IDM_VIEW_SHOWEXCERPT:
      EditGetExcerpt(Globals.hwndEdit,s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt));
      UpdateToolbar();
      break;


    case IDM_VIEW_NOSAVERECENT:
      Settings.SaveRecentFiles = !Settings.SaveRecentFiles;
      break;


    case IDM_VIEW_NOPRESERVECARET:
      Settings.PreserveCaretPos = !Settings.PreserveCaretPos;
      break;


    case IDM_VIEW_NOSAVEFINDREPL:
      Settings.SaveFindReplace = !Settings.SaveFindReplace;
      break;


    case IDM_VIEW_SAVEBEFORERUNNINGTOOLS:
      Settings.SaveBeforeRunningTools = !Settings.SaveBeforeRunningTools;
      break;


    case IDM_VIEW_CHANGENOTIFY:
      if (ChangeNotifyDlg(hwnd))
        InstallFileWatching(Globals.CurrentFile);
      break;


    case IDM_VIEW_NOESCFUNC:
    case IDM_VIEW_ESCMINIMIZE:
    case IDM_VIEW_ESCEXIT:
      Settings.EscFunction = (int)LOWORD(wParam) - IDM_VIEW_NOESCFUNC;
      break;


    case IDM_VIEW_SAVESETTINGS:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGS)) {
        s_bSaveSettings = !s_bSaveSettings;
        if (s_bSaveSettings)
          IniSetString(L"Settings", L"SaveSettings", NULL);
        else
          IniSetBool(L"Settings", L"SaveSettings", s_bSaveSettings);
      }
      break;


    case IDM_VIEW_SAVESETTINGSNOW:
      if (IsCmdEnabled(hwnd, IDM_VIEW_SAVESETTINGSNOW)) {

        bool bCreateFailure = false;

        if (StringCchLenW(Globals.IniFile,COUNTOF(Globals.IniFile)) == 0) {

          if (StringCchLenW(s_wchIniFile2,COUNTOF(s_wchIniFile2)) > 0) {
            if (CreateIniFileEx(s_wchIniFile2)) {
              StringCchCopy(Globals.IniFile,COUNTOF(Globals.IniFile),s_wchIniFile2);
              StringCchCopy(s_wchIniFile2,COUNTOF(s_wchIniFile2),L"");
            }
            else
              bCreateFailure = true;
          }

          else
            break;
        }

        if (!bCreateFailure) 
        {
          if (WritePrivateProfileString(L"Settings", L"WriteTest", L"ok", Globals.IniFile)) {
            SaveSettings(true);
            MsgBoxLng(MBINFO,IDS_MUI_SAVEDSETTINGS);
          }
          else {
            Globals.dwLastError = GetLastError();
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
        ThemedDialogBox(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_ABOUT), hwnd, AboutDlgProc);
      break;

    case IDM_SETPASS:
      if (GetFileKey(Globals.hwndEdit)) {
        _SetDocumentModified(true);
      }
      break;

    case IDM_HELP_CMD:
      DisplayCmdLineHelp(hwnd);
      break;


    case CMD_ESCAPE:
      if (SciCall_CallTipActive() || SciCall_AutoCActive()) {
        SciCall_CallTipCancel();
        SciCall_AutoCCancel();
      }
      else if (Settings.EscFunction == 1) {
        SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
      }
      else if (Settings.EscFunction == 2) {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
      }
      else {
        if (!SciCall_IsSelectionEmpty()) {
          DocPos const iCurPos = SciCall_GetCurrentPos();
          EditSetSelectionEx(Globals.hwndEdit, iCurPos, iCurPos, -1, -1);
        }
        SciCall_Cancel();
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
      Settings.AutoIndent = !Settings.AutoIndent;
      SciCall_NewLine();
      Settings.AutoIndent = !Settings.AutoIndent;
      break;


    case IDM_EDIT_CLEAR:
    case CMD_DEL:
        ///~_BEGIN_UNDO_ACTION_;
        SciCall_Clear();
        ///~_END_UNDO_ACTION_;
      break;

#if 0
    case CMD_LEFT:
      //Sci_SendMsgV0(CHARLEFT);
      SciCall_GotoPos(SciCall_PositionBefore(SciCall_GetCurrentPos()));
      break;

    case CMD_RIGHT:
      //Sci_SendMsgV0(CHARRIGHT);
      SciCall_GotoPos(SciCall_PositionAfter(SciCall_GetCurrentPos()));
      break;
#endif

    case CMD_CTRLLEFT:
      Sci_SendMsgV0(WORDLEFT);
      break;


    case CMD_CTRLRIGHT:
      Sci_SendMsgV0(WORDRIGHT);
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
        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
          Encoding_SrcCmdLn(Encoding_MapUnicode(Settings.DefaultEncoding));
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
          FileLoad(false,false,true,true,true,tchMaxPathBuffer);
        }
      }
      break;


    case CMD_RECODEANSI:
      {
        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
          Encoding_SrcCmdLn(CPI_ANSI_DEFAULT);
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
          FileLoad(false,false,true,true,Settings.SkipANSICodePageDetection,tchMaxPathBuffer);
        }
      }
      break;


    case CMD_RECODEOEM:
      {
        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
          Encoding_SrcCmdLn(CPI_OEM);
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
          FileLoad(false,false,true,true,true,tchMaxPathBuffer);
        }
      }
      break;


    case CMD_RELOADASCIIASUTF8:
      {
        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
          Globals.bForceLoadASCIIasUTF8 = true;
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
          FileLoad(false, false, true, true, true, tchMaxPathBuffer);
          Globals.bForceLoadASCIIasUTF8 = false;
        }
      }
      break;


    case CMD_RELOADFORCEDETECTION:
    {
      g_bForceCompEncDetection = true;
      if (StringCchLenW(Globals.CurrentFile, COUNTOF(Globals.CurrentFile))) {
        Globals.bForceLoadASCIIasUTF8 = false;
        StringCchCopy(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer), Globals.CurrentFile);
        FileLoad(false, false, true, false, false, tchMaxPathBuffer);
      }
      g_bForceCompEncDetection = false;
    }
    break;

    case CMD_RELOADNOFILEVARS:
      {
        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
          int _fNoFileVariables = Flags.NoFileVariables;
          bool _bNoEncodingTags = Settings.NoEncodingTags;
          Flags.NoFileVariables = 1;
          Settings.NoEncodingTags = true;
          StringCchCopy(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),Globals.CurrentFile);
          FileLoad(false,false,true, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, tchMaxPathBuffer);
          Flags.NoFileVariables = _fNoFileVariables;
          Settings.NoEncodingTags = _bNoEncodingTags;
        }
      }
      break;


    case CMD_LEXDEFAULT:
      Style_SetDefaultLexer(Globals.hwndEdit);
      break;


    //case CMD_LEXHTML:
    //  Style_SetHTMLLexer(Globals.hwndEdit);
    //  break;


    //case CMD_LEXXML:
    //  Style_SetXMLLexer(Globals.hwndEdit);
    //  break;


    case CMD_TIMESTAMPS:
      {
        WCHAR wchFind[128] = { L'\0' };
        WCHAR wchTemplate[128] = { L'\0' };
        WCHAR wchReplace[128] = { L'\0' };

        SYSTEMTIME st;
        struct tm sst;

        EDITFINDREPLACE efrTS = EFR_INIT_DATA;
        efrTS.hwnd = Globals.hwndEdit;
        efrTS.fuFlags = SCFIND_REGEXP;

        StringCchCopyW(wchFind, COUNTOF(wchFind), Settings2.TimeStamp);

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

        WideCharToMultiByte(Encoding_SciCP, 0, wchFind, -1, efrTS.szFind,COUNTOF(efrTS.szFind),NULL,NULL);
        WideCharToMultiByte(Encoding_SciCP, 0, wchReplace, -1, efrTS.szReplace, COUNTOF(efrTS.szReplace), NULL, NULL);

        if (!SendMessage(Globals.hwndEdit, SCI_GETSELECTIONEMPTY, 0, 0))
          EditReplaceAllInSelection(Globals.hwndEdit, &efrTS, true);
        else
          EditReplaceAll(Globals.hwndEdit,&efrTS,true);
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
        StringCchCopyW(tchMaxPathBuffer, COUNTOF(tchMaxPathBuffer),
          (LOWORD(wParam) == CMD_WEBACTION1) ? Settings2.WebTemplate1 : Settings2.WebTemplate2);

        if (StringCchLenW(tchMaxPathBuffer,0) > 0) {

          DocPos const cchSelection = SciCall_GetSelText(NULL);

          char  mszSelection[HUGE_BUFFER] = { '\0' };
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

              WCHAR wszSelection[HUGE_BUFFER] = { L'\0' };
              MultiByteToWideChar(Encoding_SciCP,0,mszSelection,-1,wszSelection, HUGE_BUFFER);

              int cmdsz = (512 + COUNTOF(tchMaxPathBuffer) + MAX_PATH + 32);
              LPWSTR lpszCommand = AllocMem(sizeof(WCHAR)*cmdsz, HEAP_ZERO_MEMORY);
              StringCchPrintf(lpszCommand,cmdsz,tchMaxPathBuffer,wszSelection);
              ExpandEnvironmentStringsEx(lpszCommand, cmdsz);

              WCHAR wchDirectory[MAX_PATH] = { L'\0' };
              if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
                StringCchCopy(wchDirectory,COUNTOF(wchDirectory),Globals.CurrentFile);
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
      if (!Settings.MarkLongLines)
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_LONGLINEMARKER,1),0);
      else {
        if (LOWORD(wParam) == CMD_INCLINELIMIT)
          Settings.LongLinesLimit++;
        else
          Settings.LongLinesLimit--;
        Settings.LongLinesLimit = clampi(Settings.LongLinesLimit, 0, LONG_LINES_MARKER_LIMIT);
        SendMessage(Globals.hwndEdit,SCI_SETEDGECOLUMN,Settings.LongLinesLimit,0);
        UpdateToolbar();
        UpdateStatusbar(false);
        Globals.iLongLinesLimit = Settings.LongLinesLimit;
      }
      break;

    case CMD_STRINGIFY:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(Globals.hwndEdit, L"'", L"'");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_STRINGIFY2:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(Globals.hwndEdit, L"\"", L"\"");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_EMBRACE:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(Globals.hwndEdit, L"(", L")");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_EMBRACE2:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(Globals.hwndEdit, L"[", L"]");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_EMBRACE3:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(Globals.hwndEdit, L"{", L"}");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_EMBRACE4:
      {
        _BEGIN_UNDO_ACTION_;
        EditEncloseSelection(Globals.hwndEdit, L"`", L"`");
        _END_UNDO_ACTION_;
      }
      break;


    case CMD_INCREASENUM:
      EditModifyNumber(Globals.hwndEdit,true);
      break;


    case CMD_DECREASENUM:
      EditModifyNumber(Globals.hwndEdit,false);
      break;


    case CMD_TOGGLETITLE:
      EditGetExcerpt(Globals.hwndEdit,s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt));
      UpdateToolbar();
      break;


    case CMD_JUMP2SELSTART:
      EditJumpToSelectionStart(Globals.hwndEdit);
      SciCall_ChooseCaretX();
      break;

    case CMD_JUMP2SELEND:
      EditJumpToSelectionEnd(Globals.hwndEdit);
      SciCall_ChooseCaretX();
      break;


    case CMD_COPYPATHNAME: {

        WCHAR *pszCopy;
        WCHAR tchUntitled[32] = { L'\0' };
        if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)))
          pszCopy = Globals.CurrentFile;
        else {
          GetLngString(IDS_MUI_UNTITLED, tchUntitled, COUNTOF(tchUntitled));
          pszCopy = tchUntitled;
        }
        SetClipboardTextW(hwnd, pszCopy, StringCchLen(pszCopy,0));
        UpdateToolbar();
      }
      break;


    case CMD_COPYWINPOS: {
        WININFO wi = GetMyWindowPlacement(Globals.hwndMain,NULL);
        StringCchPrintf(tchMaxPathBuffer,COUNTOF(tchMaxPathBuffer),L"/pos %i,%i,%i,%i,%i",wi.x,wi.y,wi.cx,wi.cy,wi.max);
        SetClipboardTextW(hwnd, tchMaxPathBuffer, StringCchLen(tchMaxPathBuffer, 0));
        UpdateToolbar();
      }
      break;


    case CMD_INITIALWINPOS:
      SnapToWinInfoPos(hwnd, &s_WinInfo, false);
      break;

    case CMD_FULLSCRWINPOS:
      {
        WININFO const wi = GetMyWindowPlacement(Globals.hwndMain, NULL);
        SnapToWinInfoPos(hwnd, &wi, true);
      }
      break;

    case CMD_DEFAULTWINPOS:
      SnapToWinInfoPos(hwnd, &s_DefWinInfo, false);
      break;

    case CMD_SAVEASDEFWINPOS:
      {
        WININFO const wi = GetMyWindowPlacement(Globals.hwndMain, NULL);
        WCHAR tchDefWinPos[80];
        StringCchPrintf(tchDefWinPos, COUNTOF(tchDefWinPos), L"%i,%i,%i,%i,%i", wi.x, wi.y, wi.cx, wi.cy, wi.max);
        IniSetString(L"Settings2", L"DefaultWindowPosition", tchDefWinPos);
      }
      break;

    case CMD_CLEARSAVEDWINPOS:
      s_DefWinInfo = _InitDefaultWndPos(2);
      IniSetString(L"Settings2", L"DefaultWindowPosition", NULL);
    break;

    case CMD_OPENINIFILE:
      if (StringCchLenW(Globals.IniFile,COUNTOF(Globals.IniFile))) {
        SaveSettings(false);
        FileLoad(false,false,false,false,true,Globals.IniFile);
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
        SendMessage(Globals.hwndEdit,SCI_CLEARALL,0,0);
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
    if (Settings.MinimizeToTray) {
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
  int const iStyleID = Style_GetHotspotStyleID();

  if (!SciCall_StyleGetHotspot(iStyleID) || (cStyle != (char)iStyleID)) { return; }

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
    MultiByteToWideChar(Encoding_SciCP, 0, chURL, -1, wchURL, HUGE_BUFFER);

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

        if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), szFileName))
          FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, tchFile);
      }
      else
        FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, szFileName);

    }
    else { // open in web browser

      WCHAR wchDirectory[MAX_PATH+1] = { L'\0' };
      if (StringCchLenW(Globals.CurrentFile, COUNTOF(Globals.CurrentFile))) {
        StringCchCopy(wchDirectory, COUNTOF(wchDirectory), Globals.CurrentFile);
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
static void  _HandleAutoIndent(int const charAdded) 
{
  // TODO: handle indent after '{' and un-indent on '}' in C/C++ ?
  // in CRLF mode handle LF only...
  int const _eol_mode = SciCall_GetEOLMode();
  if (((SC_EOL_CRLF == _eol_mode) && (charAdded != '\r')) || (SC_EOL_CRLF != _eol_mode))
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
      DocLn const iPrevLine = iCurLine - 1;
      DocPos const iPrevLineLength = SciCall_LineLength(iPrevLine);
      char* pLineBuf = (char*)AllocMem(iPrevLineLength + 1, HEAP_ZERO_MEMORY);
      if (pLineBuf)
      {
        SciCall_GetLine_Safe(iPrevLine, pLineBuf);
        for (char* pPos = pLineBuf; *pPos; pPos++) {
          if ((*pPos != ' ') && (*pPos != '\t')) {
            *pPos = '\0';
            break;
          }
        }
        if (*pLineBuf) {
          _BEGIN_UNDO_ACTION_;
          SciCall_AddText((DocPos)StringCchLenA(pLineBuf, iPrevLineLength), pLineBuf);
          _END_UNDO_ACTION_;
        }
        FreeMem(pLineBuf);
      }
    }
  }
}


//=============================================================================
//
//  _HandleAutoCloseTags()
//
static void  _HandleAutoCloseTags()
{
  ///int lexerID = SciCall_GetLexer();
  ///if (lexerID == SCLEX_HTML || lexerID == SCLEX_XML)
  DocPos const maxSearchBackward = 4096;
  {
    DocPos const iCurPos = SciCall_GetCurrentPos();
    DocPos const iHelper = iCurPos - maxSearchBackward;
    DocPos const iStartPos = max_p(0, iHelper);
    DocPos const iSize = iCurPos - iStartPos;

    if (iSize >= 3)
    {
      const char* pBegin = SciCall_GetRangePointer(iStartPos, iSize);

      if (pBegin[iSize - 2] != '/') 
      {
        const char* pCur = &pBegin[iSize - 2];
        while (pCur > pBegin && *pCur != '<' && *pCur != '>') { --pCur; }

        int  cchIns = 2;
        char replaceBuf[FNDRPL_BUFFER+2];
        StringCchCopyA(replaceBuf, FNDRPL_BUFFER, "</");
        if (*pCur == '<') {
          ++pCur;
          while ((StrChrA(":_-.", *pCur) || IsCharAlphaNumericA(*pCur)) && (cchIns < (FNDRPL_BUFFER-2))) {
            replaceBuf[cchIns++] = *pCur;
            ++pCur;
          }
        }
        replaceBuf[cchIns++] = '>';
        replaceBuf[cchIns] = '\0';

        // except tags w/o closing tags
        const char* const nonClosingTags[9] = { "</base>", "</bgsound>", "</br>", "</embed>", "</hr>", "</img>", "</input>", "</link>", "</meta>" };
        int const cntCount = COUNTOF(nonClosingTags);

        bool isNonClosingTag = false;
        for (int i = 0; ((i < cntCount) && !isNonClosingTag); ++i) {
          isNonClosingTag = (StringCchCompareXIA(replaceBuf, nonClosingTags[i]) == 0);
        }
        if ((cchIns > 3) && !isNonClosingTag)
        {
          _BEGIN_UNDO_ACTION_;
          SciCall_ReplaceSel(replaceBuf);
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
static void  _HandleTinyExpr()
{
  DocPos const iCurPos = SciCall_GetCurrentPos();
  DocPos const iPosBefore = SciCall_PositionBefore(iCurPos);
  char const chBefore = SciCall_GetCharAt(iPosBefore - 1);
  if (chBefore == '=') // got "=?" evaluate expression trigger
  {
    DocPos lineLen = SciCall_LineLength(SciCall_LineFromPosition(iCurPos));
    char* lineBuf = (char*)AllocMem(lineLen + 1, HEAP_ZERO_MEMORY);
    if (lineBuf) {
      DocPos const iLnCaretPos = SciCall_GetCurLine((unsigned int)lineLen, lineBuf);
      lineBuf[(iLnCaretPos > 1) ? (iLnCaretPos - 2) : 0] = '\0'; // break before "=?"

      int iExprErr = 1;
      const char* pBegin = lineBuf;
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
      FreeMem(lineBuf);
    }
  }
}

#if 0
//=============================================================================
//
//   _IsIMEOpenInNoNativeMode()
//
static bool  _IsIMEOpenInNoNativeMode()
{
  bool result = false;
  HIMC const himc = ImmGetContext(Globals.hwndEdit);
  if (himc) {
    if (ImmGetOpenStatus(himc)) {
      DWORD dwConversion = IME_CMODE_ALPHANUMERIC, dwSentence = 0;
      if (ImmGetConversionStatus(himc, &dwConversion, &dwSentence)) {
        result = (dwConversion != IME_CMODE_ALPHANUMERIC);
      }
    }
    ImmReleaseContext(Globals.hwndEdit, himc);
  }
  return result;
}
#endif

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
        if (EditToggleView(Globals.hwndEdit, false)) {
          EditToggleView(Globals.hwndEdit, true);
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
            if (Settings.MarkOccurrences > 0) {
              MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
            }
            if (scn->linesAdded != 0) {
              UpdateMarginWidth();
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
        //    EditUpdateUrlHotspots(Globals.hwndEdit, SciCall_PositionFromLine(lineNumber), (int)scn->position, bHyperlinkHotspot);
        //    EditUpdateHiddenLineRange(hwnd, &Settings.EFR_Data, 0, SciCall_GetLineCount());
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
            if (Settings.MatchBraces) {
              EditMatchBrace(Globals.hwndEdit);
            }

            if (Settings.MarkOccurrences > 0) {
              // clear marks only, if selection changed
              if (iUpd & SC_UPDATE_SELECTION)
              {
                if (!SciCall_IsSelectionEmpty()) {
                  MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, true);
                }
                else {
                  EditClearAllOccurrenceMarkers(Globals.hwndEdit);
                }
              }
              else if (iUpd & SC_UPDATE_CONTENT) {
                // ignoring SC_UPDATE_CONTENT cause Style and Marker are out of scope here
                // using WM_COMMAND -> SCEN_CHANGE  instead!
                //~MarkAllOccurrences(Settings2.UpdateDelayMarkAllCoccurrences, false);
              }
            }

            if (Settings.HyperlinkHotspot) {
              UpdateVisibleUrlHotspot(Settings2.UpdateDelayHyperlinkStyling);
            }
            UpdateToolbar();
            UpdateStatusbar(false);
          }
          else if (iUpd & SC_UPDATE_V_SCROLL)
          {
            if ((Settings.MarkOccurrences > 0) && Settings.MarkOccurrencesMatchVisible) {
              MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, false);
            }
            if (Settings.HyperlinkHotspot) {
              UpdateVisibleUrlHotspot(Settings2.UpdateDelayHyperlinkStyling);
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
            int const ich = scn->ch;

            if (Globals.CallTipType != CT_NONE) {
              SciCall_CallTipCancel();   
              Globals.CallTipType = CT_NONE;
            }

            switch (ich) {
            case '\r':
            case '\n':
              if (Settings.AutoIndent) { _HandleAutoIndent(ich); }
              break;
            case '>':
              if (Settings.AutoCloseTags) { _HandleAutoCloseTags(); }
              break;
            case '?':
              _HandleTinyExpr();
              break;
            default:
              break;
            }

            if ((Settings.AutoCompleteWords || Settings.AutoCLexerKeyWords)) 
            {
              if (!EditAutoCompleteWord(Globals.hwndEdit, false)) { return 0; }
            }
          }
          break;

        case SCN_AUTOCCHARDELETED:
          if ((Settings.AutoCompleteWords || Settings.AutoCLexerKeyWords)) 
          {
            if (!EditAutoCompleteWord(Globals.hwndEdit, false)) { return 0; }
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
          UpdateMarginWidth();
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
                if (OpenFileDlg(Globals.hwndMain, tchFile, COUNTOF(tchFile), szBuf)) {
                  FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, tchFile);
                }
              }
              else if (PathFileExists(szBuf)) {
                FileLoad(false, false, false, Settings.SkipUnicodeDetection, Settings.SkipANSICodePageDetection, szBuf);
              }
            }
          }
          break;

        default:
          return 0LL;
      }
      // in any case 
      if (Settings.MarkOccurrencesCurrentWord && (Settings.MarkOccurrences > 0)) {
        MarkAllOccurrences(Settings2.UpdateDelayMarkAllOccurrences, false);
      }
      return -1LL;

    // ------------------------------------------------------------------------

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
            if (((LPTBNOTIFY)lParam)->iItem < COUNTOF(s_tbbMainWnd))
            {
              WCHAR tch[SMALL_BUFFER] = { L'\0' };
              GetLngString(s_tbbMainWnd[((LPTBNOTIFY)lParam)->iItem].idCommand,tch,COUNTOF(tch));
              StringCchCopyN(((LPTBNOTIFY)lParam)->pszText,((LPTBNOTIFY)lParam)->cchText,tch,((LPTBNOTIFY)lParam)->cchText);
              CopyMemory(&((LPTBNOTIFY)lParam)->tbButton,&s_tbbMainWnd[((LPTBNOTIFY)lParam)->iItem],sizeof(TBBUTTON));
              return 1LL;
            }
          }
          return 0LL;

        case TBN_RESET:
          {
            int i; int c = (int)SendMessage(s_hwndToolbar,TB_BUTTONCOUNT,0,0);
            for (i = 0; i < c; i++) {
              SendMessage(s_hwndToolbar, TB_DELETEBUTTON, 0, 0);
            }
            SendMessage(s_hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)s_tbbMainWnd);
          }
          return 0LL;

        default:
          return 0LL;
      }
      return 1LL;

    // ------------------------------------------------------------------------

    case IDC_STATUSBAR:

      switch(pnmh->code)
      {

        case NM_CLICK:
          {
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;

            switch (pnmm->dwItemSpec)
            {
              case STATUS_EOLMODE:
                EditEnsureConsistentLineEndings(Globals.hwndEdit);
                return 1LL;

              default:
                return 0LL;
            }
          }
          break;

        case NM_DBLCLK:
          {
            LPNMMOUSE pnmm = (LPNMMOUSE)lParam;

            switch (s_vSBSOrder[pnmm->dwItemSpec])
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
                  int const _eol_mode = SciCall_GetEOLMode();
                  if (_eol_mode == SC_EOL_CRLF)
                    i = IDM_LINEENDINGS_CRLF;
                  else if (_eol_mode == SC_EOL_CR)
                    i = IDM_LINEENDINGS_CR;
                  else
                    i = IDM_LINEENDINGS_LF;
                  ++i;
                  if (i > IDM_LINEENDINGS_LF) { i = IDM_LINEENDINGS_CRLF; }
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
                  if (s_iExprError == 0) {
                    StringCchPrintfA(chBuf, COUNTOF(chBuf), "%.6G", s_dExpression);
                    SciCall_CopyText((DocPos)StringCchLenA(chBuf,80), chBuf);
                  }
                  else if (s_iExprError > 0) {
                    StringCchPrintfA(chBuf, COUNTOF(chBuf), "^[%i]", s_iExprError);
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

    // ------------------------------------------------------------------------

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
  WideCharToMultiByte(Encoding_SciCP, 0, sCurrentFindPattern, -1, 
                      chFindPattern, (MBWC_DocPos_Cast)bufferSize, NULL, NULL);
}


//=============================================================================
//
//  LoadSettings()
//
//
void LoadSettings()
{
  int const cchIniSection = INISECTIONBUFCNT * HUGE_BUFFER;

  WCHAR *pIniSection = AllocMem(sizeof(WCHAR) * cchIniSection, HEAP_ZERO_MEMORY);
  
  if (pIniSection) 
  {
    // prerequisites 
    s_bSaveSettings = IniGetBool(L"Settings", L"SaveSettings", true);
    s_iSettingsVersion = IniGetInt(L"Settings", L"SettingsVersion", CFG_VER_NONE);

    // first load "hard coded" .ini-Settings
    // --------------------------------------------------------------------------
    LoadIniSection(L"Settings2", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    Defaults2.PreferredLanguageLocaleName[0] = L'\0';
    IniSectionGetString(pIniSection, L"PreferredLanguageLocaleName", Defaults2.PreferredLanguageLocaleName,
                        Settings2.PreferredLanguageLocaleName, COUNTOF(Settings2.PreferredLanguageLocaleName));

    StringCchCopyW(Defaults2.DefaultExtension, COUNTOF(Defaults2.DefaultExtension), L"txt");
    IniSectionGetString(pIniSection, L"DefaultExtension", Defaults2.DefaultExtension,
                        Settings2.DefaultExtension, COUNTOF(Settings2.DefaultExtension));
    StrTrim(Settings2.DefaultExtension, L" \t.\"");

    Defaults2.DefaultDirectory[0] = L'\0';
    IniSectionGetString(pIniSection, L"DefaultDirectory", Defaults2.DefaultDirectory,
                        Settings2.DefaultDirectory, COUNTOF(Settings2.DefaultDirectory));

    Defaults2.FileDlgFilters[0] = L'\0';
    IniSectionGetString(pIniSection, L"FileDlgFilters", Defaults2.FileDlgFilters,
                        Settings2.FileDlgFilters, COUNTOF(Settings2.FileDlgFilters) - 2);

    Defaults2.FileCheckInverval = 2000UL;
    Settings2.FileCheckInverval = clampul(IniSectionGetInt(pIniSection, L"FileCheckInverval",
                                                           Defaults2.FileCheckInverval), 250UL, 300000UL);
    Defaults2.AutoReloadTimeout = 2000UL;
    Settings2.AutoReloadTimeout = clampul(IniSectionGetInt(pIniSection, L"AutoReloadTimeout",
                                                           Defaults2.AutoReloadTimeout), 250UL, 300000UL);

    // deprecated
    Defaults.RenderingTechnology = IniSectionGetInt(pIniSection, L"SciDirectWriteTech", -111);
    if ((Defaults.RenderingTechnology != -111) && s_bSaveSettings) {
      // cleanup
      IniSetString(L"Settings2", L"SciDirectWriteTech", NULL);
    }
    Defaults.RenderingTechnology = clampi(Defaults.RenderingTechnology, 0, 3);

    // Settings2 deprecated
    Defaults.Bidirectional = IniSectionGetInt(pIniSection, L"EnableBidirectionalSupport", -111);
    if ((Defaults.Bidirectional != -111) && s_bSaveSettings) {
      // cleanup
      IniSetString(L"Settings2", L"EnableBidirectionalSupport", NULL);
    }
    Defaults.Bidirectional = (clampi(Defaults.Bidirectional, SC_BIDIRECTIONAL_DISABLED, SC_BIDIRECTIONAL_R2L) > 0) ? SC_BIDIRECTIONAL_R2L : 0;

    Defaults2.IMEInteraction = -1;
    Settings2.IMEInteraction = clampi(IniSectionGetInt(pIniSection, L"IMEInteraction", Defaults2.IMEInteraction), -1, SC_IME_INLINE);
    // Korean IME use inline mode by default
    if (Settings2.IMEInteraction == -1) { // auto detection once
      // ScintillaWin::KoreanIME()
      int const codePage = Scintilla_InputCodePage(); 
      Settings2.IMEInteraction = ((codePage == 949 || codePage == 1361) ? SC_IME_INLINE : SC_IME_WINDOWED);
    }

    Defaults2.SciFontQuality = g_FontQuality[3];
    Settings2.SciFontQuality = clampi(IniSectionGetInt(pIniSection, L"SciFontQuality", Defaults2.SciFontQuality), 0, 3);
    
    Defaults2.MarkOccurrencesMaxCount = 2000;
    Settings2.MarkOccurrencesMaxCount = IniSectionGetInt(pIniSection, L"MarkOccurrencesMaxCount", Defaults2.MarkOccurrencesMaxCount);
    if (Settings2.MarkOccurrencesMaxCount <= 0) { Settings2.MarkOccurrencesMaxCount = INT_MAX; }

    Defaults2.UpdateDelayHyperlinkStyling = 100;
    Settings2.UpdateDelayHyperlinkStyling = clampi(IniSectionGetInt(pIniSection, L"UpdateDelayHyperlinkStyling",
                                                                    Defaults2.UpdateDelayHyperlinkStyling), USER_TIMER_MINIMUM, 10000);

    Defaults2.UpdateDelayMarkAllOccurrences = 50;
    Settings2.UpdateDelayMarkAllOccurrences = clampi(IniSectionGetInt(pIniSection, L"UpdateDelayMarkAllOccurrences",
                                                                      Defaults2.UpdateDelayMarkAllOccurrences), USER_TIMER_MINIMUM, 10000);

    Defaults2.DenyVirtualSpaceAccess = false;
    Settings2.DenyVirtualSpaceAccess = IniSectionGetBool(pIniSection, L"DenyVirtualSpaceAccess", Defaults2.DenyVirtualSpaceAccess);

    Defaults2.UseOldStyleBraceMatching = false;
    Settings2.UseOldStyleBraceMatching = IniSectionGetBool(pIniSection, L"UseOldStyleBraceMatching", Defaults2.UseOldStyleBraceMatching);

    Defaults2.CurrentLineHorizontalSlop = 40;
    Settings2.CurrentLineHorizontalSlop = clampi(IniSectionGetInt(pIniSection, L"CurrentLineHorizontalSlop", Defaults2.CurrentLineHorizontalSlop), 0, 240);

    Defaults2.CurrentLineVerticalSlop = 5;
    Settings2.CurrentLineVerticalSlop = clampi(IniSectionGetInt(pIniSection, L"CurrentLineVerticalSlop", Defaults2.CurrentLineVerticalSlop), 0, 25);

    Defaults2.AdministrationTool[0] = L'\0';
    IniSectionGetString(pIniSection, L"AdministrationTool.exe", Defaults2.AdministrationTool,
                        Settings2.AdministrationTool, COUNTOF(Settings2.AdministrationTool));

    Defaults2.DefaultWindowPosition[0] = L'\0';
    IniSectionGetString(pIniSection, L"DefaultWindowPosition", Defaults2.DefaultWindowPosition,
                        Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition));
    bool const bExplicitDefaultWinPos = (StringCchLenW(Settings2.DefaultWindowPosition, 0) != 0);

    Defaults2.FileLoadWarningMB = 1;
    Settings2.FileLoadWarningMB = clampi(IniSectionGetInt(pIniSection, L"FileLoadWarningMB", Defaults2.FileLoadWarningMB), 0, 2048);
    
    Defaults2.OpacityLevel = 75;
    Settings2.OpacityLevel = clampi(IniSectionGetInt(pIniSection, L"OpacityLevel", Defaults2.OpacityLevel), 10, 100);

    Defaults2.FindReplaceOpacityLevel = 50;
    Settings2.FindReplaceOpacityLevel = clampi(IniSectionGetInt(pIniSection, L"FindReplaceOpacityLevel", Defaults2.FindReplaceOpacityLevel), 10, 100);

    Defaults2.FileBrowserPath[0] = L'\0';
    IniSectionGetString(pIniSection, L"filebrowser.exe", Defaults2.FileBrowserPath, Settings2.FileBrowserPath, COUNTOF(Settings2.FileBrowserPath));
    
    StringCchCopyW(Defaults2.AppUserModelID, COUNTOF(Defaults2.AppUserModelID), MKWCS(APPNAME));
    IniSectionGetString(pIniSection, L"ShellAppUserModelID", Defaults2.AppUserModelID, Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID));

    Defaults2.ExtendedWhiteSpaceChars[0] = L'\0';
    IniSectionGetString(pIniSection, L"ExtendedWhiteSpaceChars", Defaults2.ExtendedWhiteSpaceChars,
                        Settings2.ExtendedWhiteSpaceChars, COUNTOF(Settings2.ExtendedWhiteSpaceChars));

    Defaults2.AutoCompleteWordCharSet[0] = L'\0';
    IniSectionGetString(pIniSection, L"AutoCompleteWordCharSet", Defaults2.AutoCompleteWordCharSet,
                        Settings2.AutoCompleteWordCharSet, COUNTOF(Settings2.AutoCompleteWordCharSet));

    StringCchCopyW(Defaults2.TimeStamp, COUNTOF(Defaults2.TimeStamp), L"\\$Date:[^\\$]+\\$ | $Date: %Y/%m/%d %H:%M:%S $");
    IniSectionGetString(pIniSection, L"TimeStamp", Defaults2.TimeStamp, Settings2.TimeStamp, COUNTOF(Settings2.TimeStamp));

    Defaults2.DateTimeShort[0] = L'\0';
    IniSectionGetString(pIniSection, L"DateTimeShort", Defaults2.DateTimeShort, Settings2.DateTimeShort, COUNTOF(Settings2.DateTimeShort));
    
    Defaults2.DateTimeLong[0] = L'\0';
    IniSectionGetString(pIniSection, L"DateTimeLong", Defaults2.DateTimeLong, Settings2.DateTimeLong, COUNTOF(Settings2.DateTimeLong));
    
    StringCchCopyW(Defaults2.WebTemplate1, COUNTOF(Defaults2.WebTemplate1), L"https://google.com/search?q=%s");
    IniSectionGetString(pIniSection, L"WebTemplate1", Defaults2.WebTemplate1, Settings2.WebTemplate1, COUNTOF(Settings2.WebTemplate1));
    
    StringCchCopyW(Defaults2.WebTemplate2, COUNTOF(Defaults2.WebTemplate2), L"https://en.wikipedia.org/w/index.php?search=%s");
    IniSectionGetString(pIniSection, L"WebTemplate2", Defaults2.WebTemplate2, Settings2.WebTemplate2, COUNTOF(Settings2.WebTemplate2));


    // --------------------------------------------------------------------------
    LoadIniSection(L"Settings", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

#define GET_BOOL_VALUE_FROM_INISECTION(VARNAME,DEFAULT) \
  Defaults.VARNAME = DEFAULT;                       \
  Settings.VARNAME = IniSectionGetBool(pIniSection, STRGW(VARNAME), Defaults.VARNAME)

#define GET_INT_VALUE_FROM_INISECTION(VARNAME,DEFAULT,MIN,MAX) \
  Defaults.VARNAME = DEFAULT;                              \
  Settings.VARNAME = clampi(IniSectionGetInt(pIniSection, STRGW(VARNAME), Defaults.VARNAME),MIN,MAX)

    GET_BOOL_VALUE_FROM_INISECTION(SaveRecentFiles, true);
    GET_BOOL_VALUE_FROM_INISECTION(PreserveCaretPos, false);
    GET_BOOL_VALUE_FROM_INISECTION(SaveFindReplace, false);

    Defaults.EFR_Data.bFindClose = false;
    Settings.EFR_Data.bFindClose = IniSectionGetBool(pIniSection, L"CloseFind", Defaults.EFR_Data.bFindClose);
    Defaults.EFR_Data.bReplaceClose = false;
    Settings.EFR_Data.bReplaceClose = IniSectionGetBool(pIniSection, L"CloseReplace", Defaults.EFR_Data.bReplaceClose);
    Defaults.EFR_Data.bNoFindWrap = false;
    Settings.EFR_Data.bNoFindWrap = IniSectionGetBool(pIniSection, L"NoFindWrap", Defaults.EFR_Data.bNoFindWrap);
    Defaults.EFR_Data.bTransformBS = false;
    Settings.EFR_Data.bTransformBS = IniSectionGetBool(pIniSection, L"FindTransformBS", Defaults.EFR_Data.bTransformBS);
    Defaults.EFR_Data.bWildcardSearch = false;
    Settings.EFR_Data.bWildcardSearch = IniSectionGetBool(pIniSection, L"WildcardSearch", Defaults.EFR_Data.bWildcardSearch);
    Defaults.EFR_Data.bMarkOccurences = true;
    Settings.EFR_Data.bMarkOccurences = IniSectionGetBool(pIniSection, L"FindMarkAllOccurrences", Defaults.EFR_Data.bMarkOccurences);
    Defaults.EFR_Data.bHideNonMatchedLines = false;
    Settings.EFR_Data.bHideNonMatchedLines = IniSectionGetBool(pIniSection, L"HideNonMatchedLines", Defaults.EFR_Data.bHideNonMatchedLines);
    Defaults.EFR_Data.bDotMatchAll = false;
    Settings.EFR_Data.bDotMatchAll = IniSectionGetBool(pIniSection, L"RegexDotMatchesAll", Defaults.EFR_Data.bDotMatchAll);
    Defaults.EFR_Data.fuFlags = 0;
    Settings.EFR_Data.fuFlags = IniSectionGetUInt(pIniSection, L"efrData_fuFlags", Defaults.EFR_Data.fuFlags);

    Defaults.OpenWithDir[0] = L'\0';
    if (!IniSectionGetString(pIniSection, L"OpenWithDir", Defaults.OpenWithDir, Settings.OpenWithDir, COUNTOF(Settings.OpenWithDir))) {
      //SHGetSpecialFolderPath(NULL, Settings.OpenWithDir, CSIDL_DESKTOPDIRECTORY, true);
      GetKnownFolderPath(&FOLDERID_Desktop, Settings.OpenWithDir, COUNTOF(Settings.OpenWithDir));
    }
    else {
      PathAbsoluteFromApp(Settings.OpenWithDir, NULL, COUNTOF(Settings.OpenWithDir), true);
    }

    Defaults.FavoritesDir[0] = L'\0';
    //StringCchCopyW(Defaults.FavoritesDir, COUNTOF(Defaults.FavoritesDir), L"%USERPROFILE%");
    if (!IniSectionGetString(pIniSection, L"Favorites", Defaults.FavoritesDir, Settings.FavoritesDir, COUNTOF(Settings.FavoritesDir))) {
      //SHGetFolderPath(NULL,CSIDL_PERSONAL,NULL,SHGFP_TYPE_CURRENT,Settings.FavoritesDir);
      GetKnownFolderPath(&FOLDERID_Favorites, Settings.FavoritesDir, COUNTOF(Settings.FavoritesDir));
    }
    else {
      PathAbsoluteFromApp(Settings.FavoritesDir, NULL, COUNTOF(Settings.FavoritesDir), true);
    }

    GET_INT_VALUE_FROM_INISECTION(PathNameFormat, 1, 0, 2);
    GET_BOOL_VALUE_FROM_INISECTION(WordWrap, false);  Globals.bWordWrap = Settings.WordWrap;
    GET_INT_VALUE_FROM_INISECTION(WordWrapMode, 0, 0, 1);
    GET_INT_VALUE_FROM_INISECTION(WordWrapIndent, 2, 0, 6);

    Defaults.WordWrapSymbols = 22;
    int const iWS = IniSectionGetInt(pIniSection, L"WordWrapSymbols", Defaults.WordWrapSymbols);
    Settings.WordWrapSymbols = clampi(iWS % 10, 0, 2) + clampi((iWS % 100 - iWS % 10) / 10, 0, 2) * 10;
    
    GET_BOOL_VALUE_FROM_INISECTION(ShowWordWrapSymbols, true);
    GET_BOOL_VALUE_FROM_INISECTION(MatchBraces, true);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCloseTags, false);
    GET_BOOL_VALUE_FROM_INISECTION(HighlightCurrentLine, true);
    GET_BOOL_VALUE_FROM_INISECTION(HyperlinkHotspot, true);
    GET_BOOL_VALUE_FROM_INISECTION(ScrollPastEOF, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoIndent, true);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCompleteWords, false);
    GET_BOOL_VALUE_FROM_INISECTION(AutoCLexerKeyWords, false);
    GET_BOOL_VALUE_FROM_INISECTION(AccelWordNavigation, false);
    GET_BOOL_VALUE_FROM_INISECTION(ShowIndentGuides, false);
    GET_BOOL_VALUE_FROM_INISECTION(TabsAsSpaces, false);  Globals.bTabsAsSpaces = Settings.TabsAsSpaces;
    GET_BOOL_VALUE_FROM_INISECTION(TabIndents, true);  Globals.bTabIndents = Settings.TabIndents;
    GET_BOOL_VALUE_FROM_INISECTION(BackspaceUnindents, false);
    GET_INT_VALUE_FROM_INISECTION(TabWidth, 4, 1, 1024);  Globals.iTabWidth = Settings.TabWidth;
    GET_INT_VALUE_FROM_INISECTION(IndentWidth, 4, 0, 1024);  Globals.iIndentWidth = Settings.IndentWidth;
    GET_BOOL_VALUE_FROM_INISECTION(WarnInconsistentIndents, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkLongLines, true);
    GET_INT_VALUE_FROM_INISECTION(LongLinesLimit, 80, 0, LONG_LINES_MARKER_LIMIT);  Globals.iLongLinesLimit = Settings.LongLinesLimit;
    GET_INT_VALUE_FROM_INISECTION(LongLineMode, EDGE_LINE, EDGE_LINE, EDGE_BACKGROUND);
    GET_BOOL_VALUE_FROM_INISECTION(ShowSelectionMargin, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowLineNumbers, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowCodeFolding, true);
    GET_INT_VALUE_FROM_INISECTION(MarkOccurrences, 1, 0, 3);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchVisible, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchCase, false);
    GET_BOOL_VALUE_FROM_INISECTION(MarkOccurrencesMatchWholeWords, true);

    Defaults.MarkOccurrencesCurrentWord = !Defaults.MarkOccurrencesMatchWholeWords;
    Settings.MarkOccurrencesCurrentWord = IniSectionGetBool(pIniSection, L"MarkOccurrencesCurrentWord", Defaults.MarkOccurrencesCurrentWord);
    Settings.MarkOccurrencesCurrentWord = Settings.MarkOccurrencesCurrentWord && !Settings.MarkOccurrencesMatchWholeWords;

    GET_BOOL_VALUE_FROM_INISECTION(ViewWhiteSpace, false);
    GET_BOOL_VALUE_FROM_INISECTION(ViewEOLs, false);

    GET_INT_VALUE_FROM_INISECTION(DefaultEncoding, CPI_ANSI_DEFAULT, CED_NO_MAPPING, INT_MAX);
    // if DefaultEncoding is not defined set to system's current code-page 
    Settings.DefaultEncoding = ((Settings.DefaultEncoding == CPI_NONE) ?
      Encoding_MapIniSetting(true, (int)GetACP()) : Encoding_MapIniSetting(true, Settings.DefaultEncoding));

    GET_BOOL_VALUE_FROM_INISECTION(UseDefaultForFileEncoding, false);
    GET_BOOL_VALUE_FROM_INISECTION(SkipUnicodeDetection, false);
    GET_BOOL_VALUE_FROM_INISECTION(SkipANSICodePageDetection, false);
    GET_BOOL_VALUE_FROM_INISECTION(LoadASCIIasUTF8, false);
    GET_BOOL_VALUE_FROM_INISECTION(LoadNFOasOEM, true);
    GET_BOOL_VALUE_FROM_INISECTION(NoEncodingTags, false);
    GET_INT_VALUE_FROM_INISECTION(DefaultEOLMode, SC_EOL_CRLF, SC_EOL_CRLF, SC_EOL_LF);
    GET_BOOL_VALUE_FROM_INISECTION(WarnInconsistEOLs, true);
    GET_BOOL_VALUE_FROM_INISECTION(FixLineEndings, false);
    GET_BOOL_VALUE_FROM_INISECTION(FixTrailingBlanks, false);
    GET_INT_VALUE_FROM_INISECTION(PrintHeader, 1, 0, 3);
    GET_INT_VALUE_FROM_INISECTION(PrintFooter, 0, 0, 1);
    GET_INT_VALUE_FROM_INISECTION(PrintColorMode, 3, 0, 4);

    int const zoomScale  = float2int(1000.0f / INITIAL_BASE_FONT_SIZE);
    Defaults.PrintZoom = (s_iSettingsVersion < CFG_VER_0001) ? (zoomScale / 10) : zoomScale;
    int iPrintZoom = clampi(IniSectionGetInt(pIniSection, L"PrintZoom", Defaults.PrintZoom), 0, SC_MAX_ZOOM_LEVEL);
    if (s_iSettingsVersion < CFG_VER_0001) { iPrintZoom = 100 + (iPrintZoom - 10) * 10; }
    Settings.PrintZoom = clampi(iPrintZoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

    WCHAR localeInfo[3];
    GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IMEASURE, localeInfo, 3);
    LONG const _margin = (localeInfo[0] == L'0') ? 2000L : 1000L; // Metric system. L'1' is US System
    Defaults.PrintMargin.left = _margin;
    Settings.PrintMargin.left = clampi(IniSectionGetInt(pIniSection, L"PrintMarginLeft", Defaults.PrintMargin.left), 0, 40000);
    Defaults.PrintMargin.top = _margin;
    Settings.PrintMargin.top = clampi(IniSectionGetInt(pIniSection, L"PrintMarginTop", Defaults.PrintMargin.top), 0, 40000);
    Defaults.PrintMargin.right = _margin;
    Settings.PrintMargin.right = clampi(IniSectionGetInt(pIniSection, L"PrintMarginRight", Defaults.PrintMargin.right), 0, 40000);
    Defaults.PrintMargin.bottom = _margin;
    Settings.PrintMargin.bottom = clampi(IniSectionGetInt(pIniSection, L"PrintMarginBottom", Defaults.PrintMargin.bottom), 0, 40000);

    GET_BOOL_VALUE_FROM_INISECTION(SaveBeforeRunningTools, false);
    GET_INT_VALUE_FROM_INISECTION(FileWatchingMode, 0, 0, 2);
    GET_BOOL_VALUE_FROM_INISECTION(ResetFileWatching, true);
    GET_INT_VALUE_FROM_INISECTION(EscFunction, 0, 0, 2);
    GET_BOOL_VALUE_FROM_INISECTION(AlwaysOnTop, false);
    GET_BOOL_VALUE_FROM_INISECTION(MinimizeToTray, false);
    GET_BOOL_VALUE_FROM_INISECTION(TransparentMode, false);
    GET_BOOL_VALUE_FROM_INISECTION(FindReplaceTransparentMode, true);
    GET_INT_VALUE_FROM_INISECTION(RenderingTechnology, Defaults.RenderingTechnology, 0, 3);  // set before
    GET_INT_VALUE_FROM_INISECTION(Bidirectional, Defaults.Bidirectional, 0, 2);  // set before
    ///~Settings2.IMEInteraction = clampi(IniSectionGetInt(pIniSection, L"IMEInteraction", Settings2.IMEInteraction), SC_IME_WINDOWED, SC_IME_INLINE);

    // see TBBUTTON  s_tbbMainWnd[] for initial/reset set of buttons
    StringCchCopyW(Defaults.ToolbarButtons, COUNTOF(Defaults.ToolbarButtons), TBBUTTON_DEFAULT_IDS);
    IniSectionGetString(pIniSection, L"ToolbarButtons", Defaults.ToolbarButtons, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));

    GET_BOOL_VALUE_FROM_INISECTION(ShowToolbar, true);
    GET_BOOL_VALUE_FROM_INISECTION(ShowStatusbar, true);

    GET_INT_VALUE_FROM_INISECTION(EncodingDlgSizeX, 256, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(EncodingDlgSizeY, 262, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(RecodeDlgSizeX, 256, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(RecodeDlgSizeY, 262, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FileMRUDlgSizeX, 412, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FileMRUDlgSizeY, 376, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(OpenWithDlgSizeX, 384, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(OpenWithDlgSizeY, 384, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FavoritesDlgSizeX, 334, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FavoritesDlgSizeY, 334, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgPosX, CW_USEDEFAULT, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(FindReplaceDlgPosY, CW_USEDEFAULT, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgPosX, CW_USEDEFAULT, INT_MIN, INT_MAX);
    GET_INT_VALUE_FROM_INISECTION(CustomSchemesDlgPosY, CW_USEDEFAULT, INT_MIN, INT_MAX);

    // --------------------------------------------------------------------------
    LoadIniSection(L"Statusbar Settings", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    WCHAR tchStatusBar[MIDSZ_BUFFER] = { L'\0' };

    IniSectionGetString(pIniSection, L"SectionPrefixes", STATUSBAR_SECTION_PREFIXES, tchStatusBar, COUNTOF(tchStatusBar));
    ReadStrgsFromCSV(tchStatusBar, s_mxSBPrefix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_PRFX_");

    IniSectionGetString(pIniSection, L"SectionPostfixes", STATUSBAR_SECTION_POSTFIXES, tchStatusBar, COUNTOF(tchStatusBar));
    ReadStrgsFromCSV(tchStatusBar, s_mxSBPostfix, STATUS_SECTOR_COUNT, MICRO_BUFFER, L"_POFX_");

    IniSectionGetString(pIniSection, L"VisibleSections", STATUSBAR_DEFAULT_IDS, tchStatusBar, COUNTOF(tchStatusBar));
    ReadVectorFromString(tchStatusBar, s_iStatusbarSections, STATUS_SECTOR_COUNT, 0, (STATUS_SECTOR_COUNT - 1), -1);

    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      s_iStatusbarVisible[i] = false;
    }
    int cnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      s_vSBSOrder[i] = -1;
      int const id = s_iStatusbarSections[i];
      if (id >= 0) {
        s_vSBSOrder[cnt++] = id;
        s_iStatusbarVisible[id] = true;
      }
    }

    IniSectionGetString(pIniSection, L"SectionWidthSpecs", STATUSBAR_SECTION_WIDTH_SPECS, tchStatusBar, COUNTOF(tchStatusBar));
    ReadVectorFromString(tchStatusBar, s_iStatusbarWidthSpec, STATUS_SECTOR_COUNT, -4096, 4096, 0);

    Globals.bZeroBasedColumnIndex = IniSectionGetBool(pIniSection, L"ZeroBasedColumnIndex", false);
    Globals.bZeroBasedCharacterCount = IniSectionGetBool(pIniSection, L"ZeroBasedCharacterCount", false);


    // --------------------------------------------------------------------------
    LoadIniSection(L"Toolbar Images", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    IniSectionGetString(pIniSection, L"BitmapDefault", L"",
                        s_tchToolbarBitmap, COUNTOF(s_tchToolbarBitmap));
    IniSectionGetString(pIniSection, L"BitmapHot", L"",
                        s_tchToolbarBitmapHot, COUNTOF(s_tchToolbarBitmap));
    IniSectionGetString(pIniSection, L"BitmapDisabled", L"",
                        s_tchToolbarBitmapDisabled, COUNTOF(s_tchToolbarBitmap));

    int const ResX = GetSystemMetrics(SM_CXSCREEN);
    int const ResY = GetSystemMetrics(SM_CYSCREEN);

    // --------------------------------------------------------------------------
    LoadIniSection(L"Window", pIniSection, cchIniSection);
    // --------------------------------------------------------------------------

    WCHAR tchHighDpiToolBar[32] = { L'\0' };
    StringCchPrintf(tchHighDpiToolBar, COUNTOF(tchHighDpiToolBar), L"%ix%i HighDpiToolBar", ResX, ResY);
    s_iHighDpiToolBar = IniSectionGetInt(pIniSection, tchHighDpiToolBar, -1);
    s_iHighDpiToolBar = clampi(s_iHighDpiToolBar, -1, 1);
    if (s_iHighDpiToolBar < 0) { // undefined: determine high DPI (higher than Full-HD)
      s_iHighDpiToolBar = IsFullHDOrHigher(ResX, ResY) ? 1 : 0;
    }

    // --------------------------------------------------------------
    // startup window  (ignore window position if /p was specified)
    // --------------------------------------------------------------

    // 1st set default window position 

    s_DefWinInfo = _InitDefaultWndPos(2); // std. default position

    if (bExplicitDefaultWinPos) {
      int bMaxi = 0;
      int const itok = swscanf_s(Settings2.DefaultWindowPosition, L"%i,%i,%i,%i,%i",
                                 &s_DefWinInfo.x, &s_DefWinInfo.y, &s_DefWinInfo.cx, &s_DefWinInfo.cy, &bMaxi);
      if (itok == 4 || itok == 5) { // scan successful
        if (s_DefWinInfo.cx < 1) s_DefWinInfo.cx = CW_USEDEFAULT;
        if (s_DefWinInfo.cy < 1) s_DefWinInfo.cy = CW_USEDEFAULT;
        if (bMaxi) s_DefWinInfo.max = true;
        if (itok == 4) s_DefWinInfo.max = false;
        _InitWindowPosition(&s_DefWinInfo, 0);
      }
      else {
        // overwrite bad defined default position
        StringCchPrintf(Settings2.DefaultWindowPosition, COUNTOF(Settings2.DefaultWindowPosition),
                        L"%i,%i,%i,%i,%i", s_DefWinInfo.x, s_DefWinInfo.y, s_DefWinInfo.cx, s_DefWinInfo.cy, s_DefWinInfo.max);
        IniSetString(L"Settings2", L"DefaultWindowPosition", Settings2.DefaultWindowPosition);
      }
    }

    // 2nd set initial window position

    s_WinInfo = s_DefWinInfo;

    if (!s_flagPosParam /*|| g_bStickyWinPos*/) {

      WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32], tchZoom[32];

      StringCchPrintf(tchPosX, COUNTOF(tchPosX), L"%ix%i PosX", ResX, ResY);
      StringCchPrintf(tchPosY, COUNTOF(tchPosY), L"%ix%i PosY", ResX, ResY);
      StringCchPrintf(tchSizeX, COUNTOF(tchSizeX), L"%ix%i SizeX", ResX, ResY);
      StringCchPrintf(tchSizeY, COUNTOF(tchSizeY), L"%ix%i SizeY", ResX, ResY);
      StringCchPrintf(tchMaximized, COUNTOF(tchMaximized), L"%ix%i Maximized", ResX, ResY);
      StringCchPrintf(tchZoom, COUNTOF(tchZoom), L"%ix%i Zoom", ResX, ResY);

      s_WinInfo.x = IniSectionGetInt(pIniSection, tchPosX, CW_USEDEFAULT);
      s_WinInfo.y = IniSectionGetInt(pIniSection, tchPosY, CW_USEDEFAULT);
      s_WinInfo.cx = IniSectionGetInt(pIniSection, tchSizeX, CW_USEDEFAULT);
      s_WinInfo.cy = IniSectionGetInt(pIniSection, tchSizeY, CW_USEDEFAULT);
      s_WinInfo.max = IniSectionGetBool(pIniSection, tchMaximized, false);
      s_WinInfo.max = clampi(s_WinInfo.max, 0, 1);
      s_WinInfo.zoom = IniSectionGetInt(pIniSection, tchZoom, (s_iSettingsVersion < CFG_VER_0001) ? 0 : 100);
      if (s_iSettingsVersion < CFG_VER_0001) { s_WinInfo.zoom = (s_WinInfo.zoom + 10) * 10; }
      s_WinInfo.zoom = clampi(s_WinInfo.zoom, SC_MIN_ZOOM_LEVEL, SC_MAX_ZOOM_LEVEL);

      if ((s_WinInfo.x == CW_USEDEFAULT) || (s_WinInfo.y == CW_USEDEFAULT) ||
        (s_WinInfo.cx == CW_USEDEFAULT) || (s_WinInfo.cy == CW_USEDEFAULT)) 
      {
        s_flagWindowPos = 2; // std. default position (CmdLn: /pd)
      }
      else
        s_flagWindowPos = 0; // init to g_WinInfo
    }

    // ------------------------------------------------------------------------

    // ---  override by resolution specific settings  ---
    WCHAR tchSciDirectWriteTech[64];
    StringCchPrintf(tchSciDirectWriteTech, COUNTOF(tchSciDirectWriteTech), L"%ix%i RenderingTechnology", ResX, ResY);
    Settings.RenderingTechnology = clampi(IniSectionGetInt(pIniSection, tchSciDirectWriteTech, Settings.RenderingTechnology), 0, 3);

    WCHAR tchSciFontQuality[64];
    StringCchPrintf(tchSciFontQuality, COUNTOF(tchSciFontQuality), L"%ix%i SciFontQuality", ResX, ResY);
    Settings2.SciFontQuality = clampi(IniSectionGetInt(pIniSection, tchSciFontQuality, Settings2.SciFontQuality), 0, 3);

    FreeMem(pIniSection);
  }

  // define scintilla internal codepage
  const int iSciDefaultCodePage = SC_CP_UTF8; // default UTF8

  // remove internal support for Chinese, Japan, Korean DBCS  use UTF-8 instead
  /*
  if (Settings.DefaultEncoding == CPI_ANSI_DEFAULT)
  {
    // check for Chinese, Japan, Korean DBCS code pages and switch accordingly
    int acp = (int)GetACP();
    if (acp == 932 || acp == 936 || acp == 949 || acp == 950) {
      iSciDefaultCodePage = acp;
    }
    Settings.DefaultEncoding = Encoding_GetByCodePage(iSciDefaultCodePage);
  }
  */

  // set flag for encoding default
  Encoding_SetDefaultFlag(Settings.DefaultEncoding);

  // define default charset
  Globals.iDefaultCharSet = (int)CharSetFromCodePage((UINT)iSciDefaultCodePage);

  // Scintilla Styles
  Style_Load();

}


//=============================================================================
//
//  SaveSettings()
//

#define SAVE_VALUE_IF_NOT_EQ_DEFAULT(TYPE,VARNAME)                           \
  if (Settings.VARNAME != Defaults.VARNAME) {                            \
    IniSectionSet##TYPE(pIniSection, STRGW(VARNAME), Settings.VARNAME);  \
  }

// ----------------------------------------------------------------------------


void SaveSettings(bool bSaveSettingsNow) 
{
  if (StringCchLenW(Globals.IniFile, COUNTOF(Globals.IniFile)) == 0) { return; }

  if (!s_bEnableSaveSettings) { return; }

  CreateIniFile();

  if (!(s_bSaveSettings || bSaveSettingsNow)) {
    IniSetBool(L"Settings", L"SaveSettings", s_bSaveSettings);
    return;
  }
  // update window placement 
  s_WinInfo = GetMyWindowPlacement(Globals.hwndMain, NULL);

  WCHAR tchMsg[80];
  GetLngString(IDS_MUI_SAVINGSETTINGS, tchMsg, COUNTOF(tchMsg));
  BeginWaitCursor(tchMsg);

  int const cchIniSection = INISECTIONBUFCNT * HUGE_BUFFER;
  WCHAR *pIniSection = AllocMem(sizeof(WCHAR) * cchIniSection, HEAP_ZERO_MEMORY);

  if (pIniSection) {
    IniSectionSetInt(pIniSection, L"SettingsVersion", CFG_VER_CURRENT);
    IniSectionSetBool(pIniSection, L"SaveSettings", s_bSaveSettings);

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveRecentFiles);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, PreserveCaretPos);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveFindReplace);

    if (Settings.EFR_Data.bFindClose != Defaults.EFR_Data.bFindClose) {
      IniSectionSetBool(pIniSection, L"CloseFind", Settings.EFR_Data.bFindClose);
    }
    if (Settings.EFR_Data.bReplaceClose != Defaults.EFR_Data.bReplaceClose) {
      IniSectionSetBool(pIniSection, L"CloseReplace", Settings.EFR_Data.bReplaceClose);
    }
    if (Settings.EFR_Data.bNoFindWrap != Defaults.EFR_Data.bNoFindWrap) {
      IniSectionSetBool(pIniSection, L"NoFindWrap", Settings.EFR_Data.bNoFindWrap);
    }
    if (Settings.EFR_Data.bTransformBS != Defaults.EFR_Data.bTransformBS) {
      IniSectionSetBool(pIniSection, L"FindTransformBS", Settings.EFR_Data.bTransformBS);
    }
    if (Settings.EFR_Data.bWildcardSearch != Defaults.EFR_Data.bWildcardSearch) {
      IniSectionSetBool(pIniSection, L"WildcardSearch", Settings.EFR_Data.bWildcardSearch);
    }
    if (Settings.EFR_Data.bMarkOccurences != Defaults.EFR_Data.bMarkOccurences) {
      IniSectionSetBool(pIniSection, L"FindMarkAllOccurrences", Settings.EFR_Data.bMarkOccurences);
    }
    if (Settings.EFR_Data.bHideNonMatchedLines != Defaults.EFR_Data.bHideNonMatchedLines) {
      IniSectionSetBool(pIniSection, L"HideNonMatchedLines", Settings.EFR_Data.bHideNonMatchedLines);
    }
    if (Settings.EFR_Data.bDotMatchAll != Defaults.EFR_Data.bDotMatchAll) {
      IniSectionSetBool(pIniSection, L"RegexDotMatchesAll", Settings.EFR_Data.bDotMatchAll);
    }
    if (Settings.EFR_Data.fuFlags != Defaults.EFR_Data.fuFlags) {
      IniSectionSetInt(pIniSection, L"efrData_fuFlags", Settings.EFR_Data.fuFlags);
    }
    
    WCHAR wchTmp[MAX_PATH] = { L'\0' };
    if (StringCchCompareXIW(Settings.OpenWithDir, Defaults.OpenWithDir) != 0) {
      PathRelativeToApp(Settings.OpenWithDir, wchTmp, COUNTOF(wchTmp), false, true, Flags.PortableMyDocs);
      IniSectionSetString(pIniSection, L"OpenWithDir", wchTmp);
    }
    if (StringCchCompareXIW(Settings.FavoritesDir, Defaults.FavoritesDir) != 0) {
      PathRelativeToApp(Settings.FavoritesDir, wchTmp, COUNTOF(wchTmp), false, true, Flags.PortableMyDocs);
      IniSectionSetString(pIniSection, L"Favorites", wchTmp);
    }

    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PathNameFormat);

    if (Globals.bWordWrap != Defaults.WordWrap) {
      IniSectionSetBool(pIniSection, L"WordWrap", Globals.bWordWrap);
    }
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapIndent);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, WordWrapSymbols);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowWordWrapSymbols);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MatchBraces);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCloseTags);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, HighlightCurrentLine);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, HyperlinkHotspot);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ScrollPastEOF);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoIndent);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCompleteWords);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AutoCLexerKeyWords);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AccelWordNavigation);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowIndentGuides);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TabsAsSpaces);

    if (Globals.bTabIndents != Defaults.TabIndents) {
      IniSectionSetBool(pIniSection, L"", Globals.bTabIndents);
    }
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, BackspaceUnindents);

    if (Globals.iTabWidth != Defaults.TabWidth) {
      IniSectionSetInt(pIniSection, L"TabWidth", Globals.iTabWidth);
    }
    if (Globals.iIndentWidth != Defaults.TabWidth) {
      IniSectionSetInt(pIniSection, L"IndentWidth", Globals.iIndentWidth);
    }
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WarnInconsistentIndents);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkLongLines);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, LongLinesLimit);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, LongLineMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowSelectionMargin);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowLineNumbers);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowCodeFolding);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, MarkOccurrences);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchVisible);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchCase);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesMatchWholeWords);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MarkOccurrencesCurrentWord);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ViewWhiteSpace);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ViewEOLs);

    if (Settings.DefaultEncoding != Defaults.DefaultEncoding) {
      IniSectionSetInt(pIniSection, L"DefaultEncoding", Encoding_MapIniSetting(false, Settings.DefaultEncoding));
    }
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, UseDefaultForFileEncoding);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SkipUnicodeDetection);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SkipANSICodePageDetection);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, LoadASCIIasUTF8);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, LoadNFOasOEM);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, NoEncodingTags);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, DefaultEOLMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, WarnInconsistEOLs);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, FixLineEndings);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, FixTrailingBlanks);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PrintHeader);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PrintFooter);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PrintColorMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, PrintZoom);

    if (Settings.PrintMargin.left != Defaults.PrintMargin.left) {
      IniSectionSetInt(pIniSection, L"PrintMarginLeft", Settings.PrintMargin.left);
    }
    if (Settings.PrintMargin.top != Defaults.PrintMargin.top) {
      IniSectionSetInt(pIniSection, L"PrintMarginTop", Settings.PrintMargin.top);
    }
    if (Settings.PrintMargin.right != Defaults.PrintMargin.right) {
      IniSectionSetInt(pIniSection, L"PrintMarginRight", Settings.PrintMargin.right);
    }
    if (Settings.PrintMargin.bottom != Defaults.PrintMargin.bottom) {
      IniSectionSetInt(pIniSection, L"PrintMarginBottom", Settings.PrintMargin.bottom);
    }
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, SaveBeforeRunningTools);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FileWatchingMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ResetFileWatching);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, EscFunction);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, AlwaysOnTop);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, MinimizeToTray);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, TransparentMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, FindReplaceTransparentMode);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, RenderingTechnology);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, Bidirectional);
    ///~IniSectionSetInt(pIniSection, L"IMEInteraction", Settings2.IMEInteraction);

    Toolbar_GetButtons(s_hwndToolbar, IDT_FILE_NEW, Settings.ToolbarButtons, COUNTOF(Settings.ToolbarButtons));
    if (StringCchCompareX(Settings.ToolbarButtons, Defaults.ToolbarButtons) == 0) {
      IniSectionSetString(pIniSection, L"ToolbarButtons", NULL);
    } else {
      IniSectionSetString(pIniSection, L"ToolbarButtons", Settings.ToolbarButtons);
    }
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowToolbar);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Bool, ShowStatusbar);
    
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, EncodingDlgSizeX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, EncodingDlgSizeY);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, RecodeDlgSizeX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, RecodeDlgSizeY);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FileMRUDlgSizeX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FileMRUDlgSizeY);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, OpenWithDlgSizeX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, OpenWithDlgSizeY);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FavoritesDlgSizeX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FavoritesDlgSizeY);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgPosX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, FindReplaceDlgPosY);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgPosX);
    SAVE_VALUE_IF_NOT_EQ_DEFAULT(Int, CustomSchemesDlgPosY);

    SaveIniSection(L"Settings", pIniSection);

    FreeMem(pIniSection);
  }

  // Scintilla Styles
  Style_Save();

  int ResX = GetSystemMetrics(SM_CXSCREEN);
  int ResY = GetSystemMetrics(SM_CYSCREEN);

  WCHAR tchHighDpiToolBar[32];
  StringCchPrintf(tchHighDpiToolBar,COUNTOF(tchHighDpiToolBar),L"%ix%i HighDpiToolBar", ResX, ResY);
  IniSetInt(L"Window", tchHighDpiToolBar, s_iHighDpiToolBar);

  if (Flags.StickyWindowPosition == 0) {

    WCHAR tchPosX[32], tchPosY[32], tchSizeX[32], tchSizeY[32], tchMaximized[32], tchZoom[32];

    StringCchPrintf(tchPosX,COUNTOF(tchPosX),L"%ix%i PosX",ResX,ResY);
    StringCchPrintf(tchPosY,COUNTOF(tchPosY),L"%ix%i PosY",ResX,ResY);
    StringCchPrintf(tchSizeX,COUNTOF(tchSizeX),L"%ix%i SizeX",ResX,ResY);
    StringCchPrintf(tchSizeY,COUNTOF(tchSizeY),L"%ix%i SizeY",ResX,ResY);
    StringCchPrintf(tchMaximized,COUNTOF(tchMaximized),L"%ix%i Maximized",ResX,ResY);
    StringCchPrintf(tchZoom, COUNTOF(tchMaximized), L"%ix%i Zoom", ResX, ResY);

    IniSetInt(L"Window",tchPosX,s_WinInfo.x);
    IniSetInt(L"Window",tchPosY,s_WinInfo.y);
    IniSetInt(L"Window",tchSizeX,s_WinInfo.cx);
    IniSetInt(L"Window",tchSizeY,s_WinInfo.cy);
    IniSetBool(L"Window",tchMaximized,s_WinInfo.max);
    IniSetInt(L"Window",tchZoom, s_WinInfo.zoom);
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
  LPWSTR lpCmdLine = GetCommandLine();
  if (StrIsEmpty(lpCmdLine)) { return; }

  // Good old console can also send args separated by Tabs
  StrTab2Space(lpCmdLine);

  DocPos const len = (DocPos)(StringCchLenW(lpCmdLine,0) + 2UL);
  LPWSTR lp1 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);
  LPWSTR lp2 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);
  LPWSTR lp3 = AllocMem(sizeof(WCHAR)*len,HEAP_ZERO_MEMORY);

  if (lp1 && lp2 && lp3) 
  {
    bool bIsNotepadReplacement = false;
    
    // Start with 2nd argument
    ExtractFirstArgument(lpCmdLine, lp1, lp3, (int)len);

    bool bContinue = true;
    bool bIsFileArg = false;
    while (bContinue && ExtractFirstArgument(lp3, lp1, lp2, (int)len)) {
      // options
      if (lp1[1] == L'\0') {
        if (!bIsFileArg && (lp1[0] == L'+')) {
          Flags.MultiFileArg = 2;
          bIsFileArg = true;
        }

        else if (!bIsFileArg && (lp1[0] == L'-')) {
          Flags.MultiFileArg = 1;
          bIsFileArg = true;
        }
      }
      else if (!bIsFileArg && ((*lp1 == L'/') || (*lp1 == L'-'))) {

        // LTrim
        StrLTrim(lp1, L"-/");

        // Encoding
        if (StringCchCompareXI(lp1, L"ANSI") == 0 || StringCchCompareXI(lp1, L"A") == 0 || StringCchCompareXI(lp1, L"MBCS") == 0)
          s_flagSetEncoding = IDM_ENCODING_ANSI - IDM_ENCODING_ANSI + 1;
        else if (StringCchCompareXI(lp1, L"UNICODE") == 0 || StringCchCompareXI(lp1, L"W") == 0)
          s_flagSetEncoding = IDM_ENCODING_UNICODE - IDM_ENCODING_ANSI + 1;
        else if (StringCchCompareXI(lp1, L"UNICODEBE") == 0 || StringCchCompareXI(lp1, L"UNICODE-BE") == 0)
          s_flagSetEncoding = IDM_ENCODING_UNICODEREV - IDM_ENCODING_ANSI + 1;
        else if (StringCchCompareXI(lp1, L"UTF8") == 0 || StringCchCompareXI(lp1, L"UTF-8") == 0)
          s_flagSetEncoding = IDM_ENCODING_UTF8 - IDM_ENCODING_ANSI + 1;
        else if (StringCchCompareXI(lp1, L"UTF8SIG") == 0 || StringCchCompareXI(lp1, L"UTF-8SIG") == 0 ||
                 StringCchCompareXI(lp1, L"UTF8SIGNATURE") == 0 || StringCchCompareXI(lp1, L"UTF-8SIGNATURE") == 0 ||
                 StringCchCompareXI(lp1, L"UTF8-SIGNATURE") == 0 || StringCchCompareXI(lp1, L"UTF-8-SIGNATURE") == 0)
          s_flagSetEncoding = IDM_ENCODING_UTF8SIGN - IDM_ENCODING_ANSI + 1;

        // EOL Mode
        else if (StringCchCompareXI(lp1, L"CRLF") == 0 || StringCchCompareXI(lp1, L"CR+LF") == 0)
          s_flagSetEOLMode = IDM_LINEENDINGS_CRLF - IDM_LINEENDINGS_CRLF + 1;
        else if (StringCchCompareXI(lp1, L"CR") == 0)
          s_flagSetEOLMode = IDM_LINEENDINGS_CR - IDM_LINEENDINGS_CRLF + 1;
        else if (StringCchCompareXI(lp1, L"LF") == 0)
          s_flagSetEOLMode = IDM_LINEENDINGS_LF - IDM_LINEENDINGS_CRLF + 1;

        // Shell integration
        else if (StrCmpNI(lp1, L"appid=", CSTRLEN(L"appid=")) == 0) {
          StringCchCopyN(Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID),
                         lp1 + CSTRLEN(L"appid="), len - CSTRLEN(L"appid="));
          StrTrim(Settings2.AppUserModelID, L" ");
          if (StringCchLenW(Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID)) == 0)
            StringCchCopy(Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID), MKWCS(APPNAME));
        }

        else if (StrCmpNI(lp1, L"sysmru=", CSTRLEN(L"sysmru=")) == 0) {
          WCHAR wch[16];
          StringCchCopyN(wch, COUNTOF(wch), lp1 + CSTRLEN(L"sysmru="), COUNTOF(wch));
          StrTrim(wch, L" ");
          if (*wch == L'1')
            Flags.ShellUseSystemMRU = 2;
          else
            Flags.ShellUseSystemMRU = 1;
        }

        // Relaunch elevated
        else if (StrCmpNI(lp1, L"tmpfbuf=", CSTRLEN(L"tmpfbuf=")) == 0) {
          StringCchCopyN(s_wchTmpFilePath, COUNTOF(s_wchTmpFilePath),
                         lp1 + CSTRLEN(L"tmpfbuf="), len - CSTRLEN(L"tmpfbuf="));
          TrimStringW(s_wchTmpFilePath);
          PathUnquoteSpaces(s_wchTmpFilePath);
          NormalizePathEx(s_wchTmpFilePath, COUNTOF(s_wchTmpFilePath));
          s_flagBufferFile = 1;
        }

        else switch (*CharUpper(lp1)) {

        case L'N':
          Flags.ReuseWindow = 0;
          Flags.NoReuseWindow = 1;
          if (*CharUpper(lp1 + 1) == L'S')
            Flags.SingleFileInstance = 1;
          else
            Flags.SingleFileInstance = 0;
          break;

        case L'R':
          Flags.ReuseWindow = 1;
          Flags.NoReuseWindow = 0;
          if (*CharUpper(lp1 + 1) == L'S')
            Flags.SingleFileInstance = 1;
          else
            Flags.SingleFileInstance = 0;
          break;

        case L'F':
          if (*(lp1 + 1) == L'0' || *CharUpper(lp1 + 1) == L'O')
            StringCchCopy(Globals.IniFile, COUNTOF(Globals.IniFile), L"*?");
          else if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
            StringCchCopyN(Globals.IniFile, COUNTOF(Globals.IniFile), lp1, len);
            TrimStringW(Globals.IniFile);
            PathUnquoteSpaces(Globals.IniFile);
            NormalizePathEx(Globals.IniFile, COUNTOF(Globals.IniFile));
          }
          break;

        case L'I':
          s_flagStartAsTrayIcon = 1;
          break;

        case L'O':
          if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O')
            s_flagAlwaysOnTop = 1;
          else
            s_flagAlwaysOnTop = 2;
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
              s_flagPosParam = 1;
              s_flagWindowPos = 1;
            }
            else if (*CharUpper(lp + 1) == L'D' || *CharUpper(lp + 1) == L'S') {
              s_flagPosParam = 1;
              s_flagWindowPos = (StrChrI((lp + 1), L'L')) ? 3 : 2;
            }
            else if (StrChrI(L"FLTRBM", *(lp + 1))) {
              WCHAR *p = (lp + 1);
              s_flagPosParam = 1;
              s_flagWindowPos = 0;
              while (*p) {
                switch (*CharUpper(p)) {
                case L'F':
                  s_flagWindowPos &= ~(4 | 8 | 16 | 32);
                  s_flagWindowPos |= 64;
                  break;
                case L'L':
                  s_flagWindowPos &= ~(8 | 64);
                  s_flagWindowPos |= 4;
                  break;
                case  L'R':
                  s_flagWindowPos &= ~(4 | 64);
                  s_flagWindowPos |= 8;
                  break;
                case L'T':
                  s_flagWindowPos &= ~(32 | 64);
                  s_flagWindowPos |= 16;
                  break;
                case L'B':
                  s_flagWindowPos &= ~(16 | 64);
                  s_flagWindowPos |= 32;
                  break;
                case L'M':
                  if (s_flagWindowPos == 0)
                    s_flagWindowPos |= 64;
                  s_flagWindowPos |= 128;
                  break;
                }
                p = CharNext(p);
              }
            }
            else if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
              WININFO wi;
              int bMaximize = 0;
              int itok = swscanf_s(lp1, L"%i,%i,%i,%i,%i", &wi.x, &wi.y, &wi.cx, &wi.cy, &bMaximize);
              if (itok == 4 || itok == 5) { // scan successful
                s_flagPosParam = 1;
                s_flagWindowPos = 0;
                if (wi.cx < 1) wi.cx = CW_USEDEFAULT;
                if (wi.cy < 1) wi.cy = CW_USEDEFAULT;
                if (bMaximize) wi.max = true;
                if (itok == 4) wi.max = false;
                s_WinInfo = wi; // set window placement
              }
            }
          }
          break;

        case L'T':
          if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
            StringCchCopyN(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), lp1, len);
            s_fKeepTitleExcerpt = 1;
          }
          break;

        case L'C':
          s_flagNewFromClipboard = 1;
          break;

        case L'B':
          s_flagPasteBoard = 1;
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
              swscanf_s(lp1, L"%i,%i", &s_iInitialLine, &s_iInitialColumn);
            if (itok == 1 || itok == 2) { // scan successful
              s_flagJumpTo = 1;
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
              if (lpMatchArg) { LocalFree(lpMatchArg); }  // StrDup()
              lpMatchArg = StrDup(lp1);
              s_flagMatchText = 1;

              if (bFindUp)
                s_flagMatchText |= 2;

              if (bRegex) {
                s_flagMatchText &= ~8;
                s_flagMatchText |= 4;
              }

              if (bTransBS) {
                s_flagMatchText &= ~4;
                s_flagMatchText |= 8;
              }
            }
          }
          break;

        case L'L':
          if (*(lp1 + 1) == L'0' || *(lp1 + 1) == L'-' || *CharUpper(lp1 + 1) == L'O')
            s_flagChangeNotify = 1;
          else
            s_flagChangeNotify = 2;
          break;

        case L'Q':
          s_flagQuietCreate = 1;
          break;

        case L'S':
          if (ExtractFirstArgument(lp2, lp1, lp2, (int)len)) {
            if (lpSchemeArg) { LocalFree(lpSchemeArg); }  // StrDup()
            lpSchemeArg = StrDup(lp1);
            s_flagLexerSpecified = 1;
          }
          break;

        case L'D':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);  // StrDup()
            lpSchemeArg = NULL;
          }
          s_iInitialLexer = 0;
          s_flagLexerSpecified = 1;
          break;

        case L'H':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);  // StrDup()
            lpSchemeArg = NULL;
          }
          s_iInitialLexer = 35;
          s_flagLexerSpecified = 1;
          break;

        case L'X':
          if (lpSchemeArg) {
            LocalFree(lpSchemeArg);  // StrDup()
            lpSchemeArg = NULL;
          }
          s_iInitialLexer = 36;
          s_flagLexerSpecified = 1;
          break;

        case L'U':
          s_flagRelaunchElevated = 1;
          break;

        case L'Z':
          ExtractFirstArgument(lp2, lp1, lp2, (int)len);
          Flags.MultiFileArg = 1;
          bIsNotepadReplacement = true;
          break;

        case L'?':
          s_flagDisplayHelp = 1;
          break;

        case L'V':
          Flags.PrintFileAndLeave = 1;
          if (*CharUpper(lp1 + 1) == L'D')
            Flags.PrintFileAndLeave = 2;  // open printer dialog
          break;

        default:
          break;

        }

      }

      // pathname
      else {
        LPWSTR lpFileBuf = AllocMem(sizeof(WCHAR)*len, HEAP_ZERO_MEMORY);
        if (lpFileBuf) {
          cchiFileList = (int)(StringCchLenW(lpCmdLine, len - 2) - StringCchLenW(lp3, len));

          if (lpFileArg) {
            FreeMem(lpFileArg);
            //lpFileArg = NULL;
          }
          lpFileArg = AllocMem(sizeof(WCHAR)*(MAX_PATH+1), HEAP_ZERO_MEMORY); // changed for ActivatePrevInst() needs
          StringCchCopy(lpFileArg, (MAX_PATH+1), lp3);

          PathFixBackslashes(lpFileArg);

          if (!PathIsRelative(lpFileArg) && !PathIsUNC(lpFileArg) &&
              PathGetDriveNumber(lpFileArg) == -1 /*&& PathGetDriveNumber(Globals.WorkingDirectory) != -1*/) {

            WCHAR wchPath[(MAX_PATH+1)] = { L'\0' };
            StringCchCopy(wchPath, COUNTOF(wchPath), Globals.WorkingDirectory);
            PathStripToRoot(wchPath);
            PathCchAppend(wchPath, COUNTOF(wchPath), lpFileArg);
            StringCchCopy(lpFileArg, (MAX_PATH+1), wchPath);
          }

          StrTrim(lpFileArg, L" \"");

          while ((cFileList < FILE_LIST_SIZE) && ExtractFirstArgument(lp3, lpFileBuf, lp3, (int)len)) {
            PathQuoteSpaces(lpFileBuf);
            lpFileList[cFileList++] = StrDup(lpFileBuf); // LocalAlloc()
          }
          bContinue = false;
        }
      }

      // Continue with next argument
      if (bContinue) {
        StringCchCopy(lp3, len, lp2);
      }
    }

    FreeMem(lp1);
    FreeMem(lp2);
    FreeMem(lp3);
  }
}


//=============================================================================
//
//  LoadFlags()
//
//
void LoadFlags()
{
  DefaultFlags.StickyWindowPosition = 0;
  DefaultFlags.ReuseWindow = 0;
  DefaultFlags.NoReuseWindow = 1;
  DefaultFlags.SingleFileInstance = 1;
  DefaultFlags.MultiFileArg = 0;
  DefaultFlags.RelativeFileMRU = 1;
  DefaultFlags.PortableMyDocs = DefaultFlags.RelativeFileMRU;
  DefaultFlags.NoFadeHidden = 0;
  DefaultFlags.ToolbarLook = IsXP() ? 1 : 2;
  DefaultFlags.SimpleIndentGuides = 0;
  DefaultFlags.NoHTMLGuess = 0;
  DefaultFlags.NoCGIGuess = 0;
  DefaultFlags.NoFileVariables = 0;
  DefaultFlags.ShellUseSystemMRU = 1;
  DefaultFlags.PrintFileAndLeave = 0;

  // --------------------------------------------------------------------------

  int const cchIniSection = INISECTIONBUFCNT * HUGE_BUFFER;

  WCHAR *pIniSection = AllocMem(sizeof(WCHAR) * cchIniSection, HEAP_ZERO_MEMORY);

  if (pIniSection) 
  {
    LoadIniSection(L"Settings2", pIniSection, cchIniSection);

    if (IniSectionGetInt(pIniSection, L"StickyWindowPosition", DefaultFlags.StickyWindowPosition))
      Flags.StickyWindowPosition = 1;

    if (!Flags.ReuseWindow && !Flags.NoReuseWindow) {

      if (!IniSectionGetInt(pIniSection, L"ReuseWindow", DefaultFlags.ReuseWindow))
        Flags.NoReuseWindow = 1;

      if (IniSectionGetInt(pIniSection, L"SingleFileInstance", DefaultFlags.SingleFileInstance))
        Flags.SingleFileInstance = 1;
    }

    if (Flags.MultiFileArg == 0) {
      if (IniSectionGetInt(pIniSection, L"MultiFileArg", DefaultFlags.MultiFileArg))
        Flags.MultiFileArg = 2;
    }

    if (IniSectionGetInt(pIniSection, L"RelativeFileMRU", DefaultFlags.RelativeFileMRU))
      Flags.RelativeFileMRU = 1;

    if (IniSectionGetInt(pIniSection, L"PortableMyDocs", DefaultFlags.PortableMyDocs))
      Flags.PortableMyDocs = 1;

    if (IniSectionGetInt(pIniSection, L"NoFadeHidden", DefaultFlags.NoFadeHidden))
      Flags.NoFadeHidden = 1;

    Flags.ToolbarLook = IniSectionGetInt(pIniSection, L"ToolbarLook", DefaultFlags.ToolbarLook);
    Flags.ToolbarLook = clampi(Flags.ToolbarLook, 0, 2);

    if (IniSectionGetInt(pIniSection, L"SimpleIndentGuides", DefaultFlags.SimpleIndentGuides))
      Flags.SimpleIndentGuides = 1;

    if (IniSectionGetInt(pIniSection, L"NoHTMLGuess", DefaultFlags.NoHTMLGuess))
      Flags.NoHTMLGuess = 1;

    if (IniSectionGetInt(pIniSection, L"NoCGIGuess", DefaultFlags.NoCGIGuess))
      Flags.NoCGIGuess = 1;

    if (IniSectionGetInt(pIniSection, L"NoFileVariables", DefaultFlags.NoFileVariables))
      Flags.NoFileVariables = 1;

    if (Flags.ShellUseSystemMRU == 0) {
      if (IniSectionGetInt(pIniSection, L"ShellUseSystemMRU", DefaultFlags.ShellUseSystemMRU)) {
        Flags.ShellUseSystemMRU = 2;
      }
    }

    FreeMem(pIniSection);

    // -------------------------
    // non-settings global flags
    // -------------------------
    Flags.PrintFileAndLeave = DefaultFlags.PrintFileAndLeave = 0;

  }
}


//=============================================================================
//
//  FindIniFile()
//
//
static bool  _CheckIniFile(LPWSTR lpszFile,LPCWSTR lpszModule)
{
  WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
  ExpandEnvironmentStrings(lpszFile,tchFileExpanded,COUNTOF(tchFileExpanded));

  if (PathIsRelative(tchFileExpanded)) 
  {
    WCHAR tchBuild[MAX_PATH] = { L'\0' };
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


static bool  _CheckIniFileRedirect(LPWSTR lpszAppName, LPWSTR lpszKeyName, LPWSTR lpszFile,LPCWSTR lpszModule)
{
  WCHAR tch[MAX_PATH] = { L'\0' };
  if (GetPrivateProfileString(lpszAppName, lpszKeyName, L"", tch, COUNTOF(tch), lpszFile)) {
    if (_CheckIniFile(tch,lpszModule)) {
      StringCchCopy(lpszFile,MAX_PATH,tch);
      return true;
    }
    WCHAR tchFileExpanded[MAX_PATH] = { L'\0' };
    ExpandEnvironmentStrings(tch,tchFileExpanded,COUNTOF(tchFileExpanded));
    if (PathIsRelative(tchFileExpanded)) {
      StringCchCopy(lpszFile,MAX_PATH,lpszModule);
      StringCchCopy(PathFindFileName(lpszFile),MAX_PATH,tchFileExpanded);
      return true;
    }
    StringCchCopy(lpszFile,MAX_PATH,tchFileExpanded);
    return true;
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

  if (StringCchLenW(Globals.IniFile,COUNTOF(Globals.IniFile))) {
    if (StringCchCompareXI(Globals.IniFile, L"*?") == 0) {
      return(0);
    }
    if (!_CheckIniFile(Globals.IniFile,tchModule)) {
      ExpandEnvironmentStringsEx(Globals.IniFile,COUNTOF(Globals.IniFile));
      if (PathIsRelative(Globals.IniFile)) {
        StringCchCopy(tchPath,COUNTOF(tchPath),tchModule);
        PathRemoveFileSpec(tchPath);
        PathCchAppend(tchPath,COUNTOF(tchPath),Globals.IniFile);
        StringCchCopy(Globals.IniFile,COUNTOF(Globals.IniFile),tchPath);
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
      if (_CheckIniFileRedirect(MKWCS(APPNAME), MKWCS(APPNAME) L".ini", tchPath, tchModule)) {
        _CheckIniFileRedirect(MKWCS(APPNAME), MKWCS(APPNAME) L".ini", tchPath, tchModule);
      }
      StringCchCopy(Globals.IniFile,COUNTOF(Globals.IniFile),tchPath);
    }
    else {
      StringCchCopy(Globals.IniFile,COUNTOF(Globals.IniFile),tchModule);
      PathCchRenameExtension(Globals.IniFile,COUNTOF(Globals.IniFile),L".ini");
    }
  }

  NormalizePathEx(Globals.IniFile,COUNTOF(Globals.IniFile));
 
  return(1);
}


int TestIniFile() {

  if (StringCchCompareXI(Globals.IniFile,L"*?") == 0) {
    StringCchCopy(s_wchIniFile2,COUNTOF(s_wchIniFile2),L"");
    StringCchCopy(Globals.IniFile,COUNTOF(Globals.IniFile),L"");
    return(0);
  }

  if (PathIsDirectory(Globals.IniFile) || *CharPrev(Globals.IniFile,StrEnd(Globals.IniFile, COUNTOF(Globals.IniFile))) == L'\\') {
    WCHAR wchModule[MAX_PATH] = { L'\0' };
    GetModuleFileName(NULL,wchModule,COUNTOF(wchModule));
    PathCchAppend(Globals.IniFile,COUNTOF(Globals.IniFile),PathFindFileName(wchModule));
    PathCchRenameExtension(Globals.IniFile,COUNTOF(Globals.IniFile),L".ini");
    if (!PathFileExists(Globals.IniFile)) {
      StringCchCopy(PathFindFileName(Globals.IniFile),COUNTOF(Globals.IniFile),L"Notepad3.ini");
      if (!PathFileExists(Globals.IniFile)) {
        StringCchCopy(PathFindFileName(Globals.IniFile),COUNTOF(Globals.IniFile),PathFindFileName(wchModule));
        PathCchRenameExtension(Globals.IniFile,COUNTOF(Globals.IniFile),L".ini");
      }
    }
  }
  
  NormalizePathEx(Globals.IniFile,COUNTOF(Globals.IniFile));

  if (!PathFileExists(Globals.IniFile) || PathIsDirectory(Globals.IniFile)) {
    StringCchCopy(s_wchIniFile2,COUNTOF(s_wchIniFile2),Globals.IniFile);
    StringCchCopy(Globals.IniFile,COUNTOF(Globals.IniFile),L"");
    return(0);
  }
  return(1);
}


int CreateIniFile() 
{
  return(CreateIniFileEx(Globals.IniFile));
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
    Globals.dwLastError = GetLastError();
    if (hFile != INVALID_HANDLE_VALUE) {
      if (GetFileSize(hFile,NULL) == 0) {
        DWORD dw;
        WriteFile(hFile,(LPCVOID)L"\xFEFF[Notepad3]\r\n",26,&dw,NULL);
        Globals.bIniFileFromScratch = true;
      }
      CloseHandle(hFile);
      return(1);
    }
  }
  return(0);
}



//=============================================================================
//
//  DelayUpdateStatusbar()
//  
//
static void  DelayUpdateStatusbar(int delay, bool bForceRedraw)
{
  static CmdMessageQueue_t mqc = { NULL, WM_COMMAND, (WPARAM)MAKELONG(IDT_TIMER_UPDATE_STATUSBAR, 1), (LPARAM)0, 0 };
  mqc.hwnd = Globals.hwndMain;
  mqc.lparam = (LPARAM)bForceRedraw;
  _MQ_AppendCmd(&mqc, (UINT)(delay <= 0 ? 0 : _MQ_ms(delay)));
}


//=============================================================================
//
//  DelayUpdateToolbar()
//  
//
static void  DelayUpdateToolbar(int delay)
{
  static CmdMessageQueue_t mqc = { NULL, WM_COMMAND, (WPARAM)MAKELONG(IDT_TIMER_UPDATE_TOOLBAR, 1), (LPARAM)0, 0 };
  mqc.hwnd = Globals.hwndMain;
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
  mqc.hwnd = Globals.hwndMain;
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
  mqc.hwnd = Globals.hwndMain;
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

#define EnableTool(id,b) SendMessage(s_hwndToolbar,TB_ENABLEBUTTON,id, MAKELONG(((b) ? 1 : 0), 0))
#define CheckTool(id,b)  SendMessage(s_hwndToolbar,TB_CHECKBUTTON,id, MAKELONG((b),0))

static void  _UpdateToolbarDelayed()
{
  SetWindowTitle(Globals.hwndMain, s_uidsAppTitle, s_flagIsElevated, IDS_MUI_UNTITLED, Globals.CurrentFile,
                 Settings.PathNameFormat, IsDocumentModified || Encoding_HasChanged(CPI_GET),
                 IDS_MUI_READONLY, s_bFileReadOnly, s_wchTitleExcerpt);

  if (!Settings.ShowToolbar) { return; }

  EnableTool(IDT_FILE_ADDTOFAV, StringCchLenW(Globals.CurrentFile, COUNTOF(Globals.CurrentFile)));
  EnableTool(IDT_FILE_SAVE, (IsDocumentModified || Encoding_HasChanged(CPI_GET)) /*&& !bReadOnly*/);
  CheckTool(IDT_VIEW_WORDWRAP, Settings.WordWrap);
  CheckTool(IDT_VIEW_CHASING_DOCTAIL, Globals.bChasingDocTail);

  bool b1 = SciCall_IsSelectionEmpty();
  bool b2 = (bool)(SciCall_GetTextLength() > 0);
  bool ro = SciCall_GetReadOnly();
  bool tv = EditToggleView(Globals.hwndEdit, false);

  EnableTool(IDT_EDIT_UNDO, SciCall_CanUndo() && !ro);
  EnableTool(IDT_EDIT_REDO, SciCall_CanRedo() && !ro);
  EnableTool(IDT_EDIT_PASTE, SciCall_CanPaste() && !ro);

  EnableTool(IDT_FILE_LAUNCH, b2);


  EnableTool(IDT_EDIT_FIND, b2);
  //EnableTool(IDT_EDIT_FINDNEXT,b2);
  //EnableTool(IDT_EDIT_FINDPREV,b2 && StringCchLenA(Settings.EFR_Data.szFind,0));
  EnableTool(IDT_EDIT_REPLACE, b2 && !ro);

  EnableTool(IDT_EDIT_CUT, !b1 && !ro);
  EnableTool(IDT_EDIT_COPY, !b1 && !ro);
  EnableTool(IDT_EDIT_CLEAR, !b1 && !ro);

  EnableTool(IDT_VIEW_TOGGLEFOLDS, b2 && (Globals.bCodeFoldingAvailable && Settings.ShowCodeFolding));

  EnableTool(IDT_VIEW_TOGGLE_VIEW, b2 && ((Settings.MarkOccurrences > 0) && !Settings.MarkOccurrencesMatchVisible));
  CheckTool(IDT_VIEW_TOGGLE_VIEW, tv);
}



//=============================================================================
//
//  _StatusCalcPaneWidth()
//
static LONG  _StatusCalcPaneWidth(HWND hwnd, LPCWSTR lpsz)
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

static void  _CalculateStatusbarSections(int vSectionWidth[], sectionTxt_t tchStatusBar[], bool* bIsUpdNeeded)
{
  static int s_iWinFormerWidth = -1;
  if (s_iWinFormerWidth != s_WinCurrentWidth) {
    *bIsUpdNeeded = true;
    s_iWinFormerWidth = s_WinCurrentWidth;
  }
  if (!(*bIsUpdNeeded)) { return; }

  // count fixed and dynamic optimized pixels
  int pxCount = 0;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if (s_iStatusbarVisible[i]) {
      if (s_iStatusbarWidthSpec[i] == 0) { // dynamic optimized
        vSectionWidth[i] = _StatusCalcPaneWidth(Globals.hwndStatus, tchStatusBar[i]);
      }
      else if (s_iStatusbarWidthSpec[i] < -1) { // fixed pixel count
        vSectionWidth[i] = -(s_iStatusbarWidthSpec[i]);
      }
      //else { /* 0,-1 : relative counts */ }
      // accumulate
      if (vSectionWidth[i] > 0) { pxCount += vSectionWidth[i]; }
    }
  }

  int const iPropSectTotalWidth = s_WinCurrentWidth - pxCount - STAUSBAR_RIGHT_MARGIN;

  // init proportional section checker
  bool bIsPropSection[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if ((s_iStatusbarVisible[i]) && (vSectionWidth[i] < 0)) {
      assert(s_iStatusbarWidthSpec[i] > 0);
      bIsPropSection[i] = true;
    }
  }

  // get min. required widths
  int vMinWidth[STATUS_SECTOR_COUNT] = SBS_INIT_ZERO;
  int iTotalMinWidth = 0;
  for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
    if (bIsPropSection[i]) {
      int const iMinWidth = _StatusCalcPaneWidth(Globals.hwndStatus, tchStatusBar[i]);
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
        vPropWidth[i] = iPropSectTotalWidth * s_iStatusbarWidthSpec[i];
        totalCnt += s_iStatusbarWidthSpec[i];
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
static double  _InterpRectSelTinyExpr(int* piExprError)
{
  #define _tmpBufCnt 128
  char tmpRectSelN[_tmpBufCnt] = { '\0' };
  
  DocPosU const selCount = SciCall_GetSelections();
  DocPosU const calcBufSize = _tmpBufCnt * selCount;
  char* calcBuffer = (char*)AllocMem(calcBufSize + 1, HEAP_ZERO_MEMORY);

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
        StringCchCatA(calcBuffer, calcBufSize, "+"); // default: add numbers
      }
      bLastCharWasDigit = IsDigitA(tmpRectSelN[StringCchLenA(tmpRectSelN,COUNTOF(tmpRectSelN)) - 1]);
      StringCchCatA(calcBuffer, calcBufSize, tmpRectSelN);
    }
  }
  return te_interp(calcBuffer, piExprError);
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

static void  _UpdateStatusbarDelayed(bool bForceRedraw)
{
  if (!Settings.ShowStatusbar) { return; }

  static sectionTxt_t tchStatusBar[STATUS_SECTOR_COUNT];

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

  bool bIsUpdateNeeded = bForceRedraw;

  static WCHAR tchLn[32] = { L'\0' };
  static WCHAR tchLines[32] = { L'\0' };

  // ------------------------------------------------------

  if (s_iStatusbarVisible[STATUS_DOCLINE] || Globals.hwndDlgFindReplace)
  {
    static DocLn s_iLnFromPos = -1;
    static DocLn s_iLnCnt = -1;

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
        s_mxSBPrefix[STATUS_DOCLINE], tchLn, tchLines, s_mxSBPostfix[STATUS_DOCLINE]);
      s_iLnFromPos = iLnFromPos;
      s_iLnCnt = iLnCnt;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static WCHAR tchCol[32] = { L'\0' };

  if (s_iStatusbarVisible[STATUS_DOCCOLUMN] || Globals.hwndDlgFindReplace)
  {
    DocPos const colOffset = Globals.bZeroBasedColumnIndex ? 0 : 1;

    static DocPos s_iCol = -1;
    DocPos const iCol = SciCall_GetColumn(iPos) + SciCall_GetSelectionNCaretVirtualSpace(0);
    if (s_iCol != iCol) {
      StringCchPrintf(tchCol, COUNTOF(tchCol), DOCPOSFMTW, iCol + colOffset);
      FormatNumberStr(tchCol);
    }

    static DocPos s_iCols = -1;
    static WCHAR tchCols[32] = { L'\0' };
    DocPos const iCols = SciCall_GetColumn(iLineBack);
    if (s_iCols != iCols) {
      StringCchPrintf(tchCols, COUNTOF(tchCols), DOCPOSFMTW, iCols);
      FormatNumberStr(tchCols);
    }

    if ((s_iCol != iCol) || (s_iCols != iCols)) {
      StringCchPrintf(tchStatusBar[STATUS_DOCCOLUMN], txtWidth, L"%s%s / %s%s",
        s_mxSBPrefix[STATUS_DOCCOLUMN], tchCol, tchCols, s_mxSBPostfix[STATUS_DOCCOLUMN]);

      s_iCol = iCol;
      s_iCols = iCols;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (s_iStatusbarVisible[STATUS_DOCCHAR])
  {
    static WCHAR tchChr[32] = { L'\0' };
    static WCHAR tchChrs[32] = { L'\0' };
    static DocPos s_iChr = -1;
    DocPos const chrOffset = Globals.bZeroBasedCharacterCount ? 0 : 1;

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
        s_mxSBPrefix[STATUS_DOCCHAR], tchChr, tchChrs, s_mxSBPostfix[STATUS_DOCCHAR]);

      s_iChr = iChr;
      s_iChrs = iChrs;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static WCHAR tchSel[32] = { L'\0' };

  // number of selected chars in statusbar
  if (s_iStatusbarVisible[STATUS_SELECTION] || s_iStatusbarVisible[STATUS_SELCTBYTES] || Globals.hwndDlgFindReplace)
  {
    static bool s_bIsSelCountable = false;
    static DocPos s_iSelStart = -1;
    static DocPos s_iSelEnd = -1;

    if ((s_bIsSelCountable != bIsSelCountable) || (s_iSelStart != iSelStart) || (s_iSelEnd != iSelEnd)) 
    {
      static WCHAR tchSelB[64] = { L'\0' };
      if (bIsSelCountable)
      {
        const DocPos iSel = (DocPos)SendMessage(Globals.hwndEdit, SCI_COUNTCHARACTERS, iSelStart, iSelEnd);
        StringCchPrintf(tchSel, COUNTOF(tchSel), DOCPOSFMTW, iSel);
        FormatNumberStr(tchSel);
        StrFormatByteSize((iSelEnd - iSelStart), tchSelB, COUNTOF(tchSelB));
      }
      else {
        tchSel[0] = L'-'; tchSel[1] = L'-'; tchSel[2] = L'\0';
        tchSelB[0] = L'0'; tchSelB[1] = L'\0';
      }
      StringCchPrintf(tchStatusBar[STATUS_SELECTION], txtWidth, L"%s%s%s",
        s_mxSBPrefix[STATUS_SELECTION], tchSel, s_mxSBPostfix[STATUS_SELECTION]);

      StringCchPrintf(tchStatusBar[STATUS_SELCTBYTES], txtWidth, L"%s%s%s",
        s_mxSBPrefix[STATUS_SELCTBYTES], tchSelB, s_mxSBPostfix[STATUS_SELCTBYTES]);

      s_bIsSelCountable = bIsSelCountable;
      s_iSelStart = iSelStart;
      s_iSelEnd = iSelEnd;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // number of selected lines in statusbar
  if (s_iStatusbarVisible[STATUS_SELCTLINES])
  {
    static bool s_bIsSelectionEmpty = true;
    static DocLn s_iLinesSelected = -1;

    DocLn const iLineStart = SciCall_LineFromPosition(iSelStart);
    DocLn const iLineEnd = SciCall_LineFromPosition(iSelEnd);
    DocPos const iStartOfLinePos = SciCall_PositionFromLine(iLineEnd);

    DocLn const iLinesSelected = ((iSelStart != iSelEnd) && (iStartOfLinePos != iSelEnd)) ? ((iLineEnd - iLineStart) + 1) : (iLineEnd - iLineStart);

    if ((s_bIsSelectionEmpty != bIsSelectionEmpty) || (s_iLinesSelected != iLinesSelected))
    {
      static WCHAR tchLinesSelected[32] = { L'\0' };
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
        s_mxSBPrefix[STATUS_SELCTLINES], tchLinesSelected, s_mxSBPostfix[STATUS_SELCTLINES]);

      s_bIsSelectionEmpty = bIsSelectionEmpty;
      s_iLinesSelected = iLinesSelected;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // try calculate expression of selection
  if (s_iStatusbarVisible[STATUS_TINYEXPR])
  {
    static WCHAR tchExpression[32] = { L'\0' };
    static int s_iExErr = -3;
    s_dExpression = 0.0;
    tchExpression[0] = L'-';
    tchExpression[1] = L'-';
    tchExpression[2] = L'\0';

    if (bIsSelCountable)
    {
      DocPos const iSelSize = SciCall_GetSelText(NULL);
      if (iSelSize < 2048) // should be fast !
      {
        char* selectionBuffer = AllocMem(iSelSize, HEAP_ZERO_MEMORY);
        if (selectionBuffer) {
          SciCall_GetSelText(selectionBuffer);
          //StrDelChrA(chExpression, " \r\n\t\v");
          StrDelChrA(selectionBuffer, "\r\n");
          s_dExpression = te_interp(selectionBuffer, &s_iExprError);
          FreeMem(selectionBuffer);
        }
      }
      else {
        s_iExprError = -1;
      }
    }
    else if (SciCall_IsSelectionRectangle() && !bIsSelectionEmpty)
    {
      s_dExpression = _InterpRectSelTinyExpr(&s_iExprError);
    }
    else
      s_iExprError = -2;


    if (!s_iExprError) {
      if (fabs(s_dExpression) > 99999999.9999)
        StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"%.4E", s_dExpression);
      else
        StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"%.6G", s_dExpression);
    }
    else if (s_iExprError > 0) {
      StringCchPrintf(tchExpression, COUNTOF(tchExpression), L"^[%i]", s_iExprError);
    }

    if (!s_iExprError || (s_iExErr != s_iExprError)) {

      StringCchPrintf(tchStatusBar[STATUS_TINYEXPR], txtWidth, L"%s%s%s ",
        s_mxSBPrefix[STATUS_TINYEXPR], tchExpression, s_mxSBPostfix[STATUS_TINYEXPR]);

      s_iExErr = s_iExprError;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  static WCHAR tchOcc[32] = { L'\0' };

  // number of occurrence marks found
  if (s_iStatusbarVisible[STATUS_OCCURRENCE] || Globals.hwndDlgFindReplace)
  {
    static int s_iMarkOccurrencesCount = -111;
    static bool s_bMOVisible = false;
    if ((s_bMOVisible != Settings.MarkOccurrencesMatchVisible) || (s_iMarkOccurrencesCount != Globals.iMarkOccurrencesCount))
    {
      if ((Globals.iMarkOccurrencesCount >= 0) && !Settings.MarkOccurrencesMatchVisible)
      {
        if ((Settings2.MarkOccurrencesMaxCount < 0) || (Globals.iMarkOccurrencesCount < Settings2.MarkOccurrencesMaxCount))
        {
          StringCchPrintf(tchOcc, COUNTOF(tchOcc), L"%i", Globals.iMarkOccurrencesCount);
          FormatNumberStr(tchOcc);
        }
        else {
          static WCHAR tchTmp[32] = { L'\0' };
          StringCchPrintf(tchTmp, COUNTOF(tchTmp), L"%i", Globals.iMarkOccurrencesCount);
          FormatNumberStr(tchTmp);
          StringCchPrintf(tchOcc, COUNTOF(tchOcc), L">= %s", tchTmp);
        }
      }
      else {
        StringCchCopy(tchOcc, COUNTOF(tchOcc), L"--");
      }

      StringCchPrintf(tchStatusBar[STATUS_OCCURRENCE], txtWidth, L"%s%s%s",
        s_mxSBPrefix[STATUS_OCCURRENCE], tchOcc, s_mxSBPostfix[STATUS_OCCURRENCE]);

      s_bMOVisible = Settings.MarkOccurrencesMatchVisible;
      s_iMarkOccurrencesCount = Globals.iMarkOccurrencesCount;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // number of replaced pattern
  if (s_iStatusbarVisible[STATUS_OCCREPLACE] || Globals.hwndDlgFindReplace)
  {
    static int s_iReplacedOccurrences = -1;

    if (s_iReplacedOccurrences != Globals.iReplacedOccurrences)
    {
      static WCHAR tchRepl[32] = { L'\0' };
      if (Globals.iReplacedOccurrences > 0)
      {
        StringCchPrintf(tchRepl, COUNTOF(tchRepl), L"%i", Globals.iReplacedOccurrences);
        FormatNumberStr(tchRepl);
      }
      else {
        StringCchCopy(tchRepl, COUNTOF(tchRepl), L"--");
      }

      StringCchPrintf(tchStatusBar[STATUS_OCCREPLACE], txtWidth, L"%s%s%s",
        s_mxSBPrefix[STATUS_OCCREPLACE], tchRepl, s_mxSBPostfix[STATUS_OCCREPLACE]);

      s_iReplacedOccurrences = Globals.iReplacedOccurrences;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  // get number of bytes in current encoding
  if (s_iStatusbarVisible[STATUS_DOCSIZE])
  {
    static DocPos s_iTextLength = -1;
    DocPos const iTextLength = SciCall_GetTextLength();
    if (s_iTextLength != iTextLength) {
      static WCHAR tchBytes[32] = { L'\0' };
      StrFormatByteSize(iTextLength, tchBytes, COUNTOF(tchBytes));

      StringCchPrintf(tchStatusBar[STATUS_DOCSIZE], txtWidth, L"%s%s%s",
        s_mxSBPrefix[STATUS_DOCSIZE], tchBytes, s_mxSBPostfix[STATUS_DOCSIZE]);

      s_iTextLength = iTextLength;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (s_iStatusbarVisible[STATUS_CODEPAGE])
  {
    static int s_iEncoding = -1;
    int const iEncoding = Encoding_Current(CPI_GET);
    if (s_iEncoding != iEncoding) {
      Encoding_SetLabel(iEncoding);

      StringCchPrintf(tchStatusBar[STATUS_CODEPAGE], txtWidth, L"%s%s%s",
        s_mxSBPrefix[STATUS_CODEPAGE], Encoding_GetLabel(iEncoding), s_mxSBPostfix[STATUS_CODEPAGE]);

      s_iEncoding = iEncoding;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (s_iStatusbarVisible[STATUS_EOLMODE]) 
  {
    static int s_iEOLMode = -1;
    int const _eol_mode = SciCall_GetEOLMode();

    if (s_iEOLMode != _eol_mode) 
    {
      if (_eol_mode == SC_EOL_LF) {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sLF%s",
                        s_mxSBPrefix[STATUS_EOLMODE], s_mxSBPostfix[STATUS_EOLMODE]);
      }
      else if (_eol_mode == SC_EOL_CR) {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sCR%s",
                        s_mxSBPrefix[STATUS_EOLMODE], s_mxSBPostfix[STATUS_EOLMODE]);
      }
      else {
        StringCchPrintf(tchStatusBar[STATUS_EOLMODE], txtWidth, L"%sCR+LF%s",
                        s_mxSBPrefix[STATUS_EOLMODE], s_mxSBPostfix[STATUS_EOLMODE]);
      }
      s_iEOLMode = _eol_mode;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (s_iStatusbarVisible[STATUS_OVRMODE])
  {
    static bool s_bIsOVR = -1;
    bool const bIsOVR = (bool)SendMessage(Globals.hwndEdit, SCI_GETOVERTYPE, 0, 0);
    if (s_bIsOVR != bIsOVR) {
      if (bIsOVR)
      {
        StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sOVR%s",
          s_mxSBPrefix[STATUS_OVRMODE], s_mxSBPostfix[STATUS_OVRMODE]);
      }
      else {
        StringCchPrintf(tchStatusBar[STATUS_OVRMODE], txtWidth, L"%sINS%s",
          s_mxSBPrefix[STATUS_OVRMODE], s_mxSBPostfix[STATUS_OVRMODE]);
      }
      s_bIsOVR = bIsOVR;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (s_iStatusbarVisible[STATUS_2ND_DEF])
  {
    static bool s_bUse2ndDefault = -1;
    bool const bUse2ndDefault = Style_GetUse2ndDefault();
    if (s_bUse2ndDefault != bUse2ndDefault)
    {
      if (bUse2ndDefault)
        StringCchPrintf(tchStatusBar[STATUS_2ND_DEF], txtWidth, L"%s2ND%s",
          s_mxSBPrefix[STATUS_2ND_DEF], s_mxSBPostfix[STATUS_2ND_DEF]);
      else
        StringCchPrintf(tchStatusBar[STATUS_2ND_DEF], txtWidth, L"%sSTD%s",
          s_mxSBPrefix[STATUS_2ND_DEF], s_mxSBPostfix[STATUS_2ND_DEF]);

      s_bUse2ndDefault = bUse2ndDefault;
      bIsUpdateNeeded = true;
    }
  }
  // ------------------------------------------------------

  if (s_iStatusbarVisible[STATUS_LEXER])
  {
    static int s_iCurLexer = -1;
    int const iCurLexer = Style_GetCurrentLexerRID();
    if (s_iCurLexer != iCurLexer)
    {
      static WCHAR tchLexerName[MINI_BUFFER];
      if (Style_IsCurLexerStandard())
        Style_GetLexerDisplayName(NULL, tchLexerName, MINI_BUFFER); // don't distinguish between STD/2ND
      else
        Style_GetLexerDisplayName(Style_GetCurrentLexerPtr(), tchLexerName, MINI_BUFFER);

      StringCchPrintf(tchStatusBar[STATUS_LEXER], txtWidth, L"%s%s%s",
        s_mxSBPrefix[STATUS_LEXER], tchLexerName, s_mxSBPostfix[STATUS_LEXER]);

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
      int const id = s_vSBSOrder[i];
      if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0))
      {
        totalWidth += g_vStatusbarSectionWidth[id];
        aStatusbarSections[cnt++] = totalWidth;
      }
    }

    if (cnt > 0) { aStatusbarSections[cnt - 1] = -1; }
    else { aStatusbarSections[0] = -1; Settings.ShowStatusbar = false; }

    SendMessage(Globals.hwndStatus, SB_SETPARTS, (WPARAM)cnt, (LPARAM)aStatusbarSections);

    cnt = 0;
    for (int i = 0; i < STATUS_SECTOR_COUNT; ++i) {
      int const id = s_vSBSOrder[i];
      if ((id >= 0) && (g_vStatusbarSectionWidth[id] >= 0)) {
        StatusSetText(Globals.hwndStatus, cnt++, tchStatusBar[id]);
      }
    }
    //InvalidateRect(Globals.hwndStatus,NULL,true);
  }
  // --------------------------------------------------------------------------

  // update Find/Replace dialog (if any)
  if (Globals.hwndDlgFindReplace) {
    static WCHAR tchReplOccs[32] = { L'\0' };
    if (Globals.iReplacedOccurrences > 0)
      StringCchPrintf(tchReplOccs, COUNTOF(tchReplOccs), L"%i", Globals.iReplacedOccurrences);
    else
      StringCchCopy(tchReplOccs, COUNTOF(tchReplOccs), L"--");

    const WCHAR* SBFMT = L" %s%s / %s     %s%s     %s%s     %s%s     %s%s     (  %s  )              ";

    static WCHAR tchFRStatus[128] = { L'\0' };
    StringCchPrintf(tchFRStatus, COUNTOF(tchFRStatus), SBFMT,
      s_mxSBPrefix[STATUS_DOCLINE], tchLn, tchLines,
      s_mxSBPrefix[STATUS_DOCCOLUMN], tchCol,
      s_mxSBPrefix[STATUS_SELECTION], tchSel,
      s_mxSBPrefix[STATUS_OCCURRENCE], tchOcc,
      s_mxSBPrefix[STATUS_OCCREPLACE], tchReplOccs,
      FR_Status[Globals.FindReplaceMatchFoundState]);

    SetWindowText(GetDlgItem(Globals.hwndDlgFindReplace, IDS_FR_STATUS_TEXT), tchFRStatus);
  }

}


//=============================================================================
//
//  UpdateMarginWidth()
//
//
void UpdateMarginWidth()
{
  if (Settings.ShowLineNumbers)
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

  Style_SetFolding(Globals.hwndEdit, (Globals.bCodeFoldingAvailable && Settings.ShowCodeFolding));
  Style_SetBookmark(Globals.hwndEdit, Settings.ShowSelectionMargin);
}



//=============================================================================
//
//  UpdateSettingsCmds()
//
//
void UpdateSettingsCmds()
{
    HMENU hmenu = GetSystemMenu(Globals.hwndMain, false);
    bool hasIniFile = (StringCchLenW(Globals.IniFile,COUNTOF(Globals.IniFile)) > 0 || StringCchLenW(s_wchIniFile2,COUNTOF(s_wchIniFile2)) > 0);
    CheckCmd(hmenu, IDM_VIEW_SAVESETTINGS, s_bSaveSettings && s_bEnableSaveSettings);
    EnableCmd(hmenu, IDM_VIEW_SAVESETTINGS, hasIniFile && s_bEnableSaveSettings);
    EnableCmd(hmenu, IDM_VIEW_SAVESETTINGSNOW, hasIniFile && s_bEnableSaveSettings);
    if (SciCall_GetZoom() != 100) { EditShowZoomCallTip(Globals.hwndEdit); }
}


//=============================================================================
//
//  UpdateUI()
//
void UpdateUI()
{
  struct SCNotification scn;
  scn.nmhdr.hwndFrom = Globals.hwndEdit;
  scn.nmhdr.idFrom = IDC_EDIT;
  scn.nmhdr.code = SCN_UPDATEUI;
  scn.updated = (SC_UPDATE_CONTENT | SC_UPDATE_NP3_INTERNAL_NOTIFY);
  SendMessage(Globals.hwndMain, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
  //PostMessage(Globals.hwndMain, WM_NOTIFY, IDC_EDIT, (LPARAM)&scn);
}



//=============================================================================

#define UNDOREDO_FREE (-1L)
#define UNDOREDO_BLOCKED (-2L)
static volatile LONG UndoActionToken = UNDOREDO_BLOCKED; // block

//=============================================================================

static bool  _InUndoRedoTransaction() {
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
static int  _SaveUndoSelection()
{
  UndoRedoSelection_t sel = INIT_UNDOREDOSEL;
  sel.selMode_undo = (int)SendMessage(Globals.hwndEdit, SCI_GETSELECTIONMODE, 0, 0);

  switch (sel.selMode_undo)
  {
  case SC_SEL_RECTANGLE:
  case SC_SEL_THIN:
    sel.anchorPos_undo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);
    sel.curPos_undo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
    if (!Settings2.DenyVirtualSpaceAccess) {
      sel.anchorVS_undo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);
      sel.curVS_undo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETRECTANGULARSELECTIONCARETVIRTUALSPACE, 0, 0);
    }
    break;

  case SC_SEL_LINES:
  case SC_SEL_STREAM:
  default:
    sel.anchorPos_undo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETANCHOR, 0, 0);
    sel.curPos_undo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETCURRENTPOS, 0, 0);
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
static void  _SaveRedoSelection(int token)
{
  if (token >= 0) {
    UndoRedoSelection_t sel = INIT_UNDOREDOSEL;

    if (_UndoRedoActionMap(token, &sel) >= 0)
    {
      sel.selMode_redo = (int)SendMessage(Globals.hwndEdit, SCI_GETSELECTIONMODE, 0, 0);

      switch (sel.selMode_redo)
      {
      case SC_SEL_RECTANGLE:
      case SC_SEL_THIN:
        sel.anchorPos_redo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETRECTANGULARSELECTIONANCHOR, 0, 0);
        sel.curPos_redo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETRECTANGULARSELECTIONCARET, 0, 0);
        if (!Settings2.DenyVirtualSpaceAccess) {
          sel.anchorVS_redo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETRECTANGULARSELECTIONANCHORVIRTUALSPACE, 0, 0);
        }
        break;

      case SC_SEL_LINES:
      case SC_SEL_STREAM:
      default:
        sel.anchorPos_redo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETANCHOR, 0, 0);
        sel.curPos_redo = (DocPos)SendMessage(Globals.hwndEdit, SCI_GETCURRENTPOS, 0, 0);
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
    ISSUE_MESSAGE(Globals.hwndEdit, SCI_ENSUREVISIBLE, anchorPosLine, 0);
    if (anchorPosLine != currPosLine) { ISSUE_MESSAGE(Globals.hwndEdit, SCI_ENSUREVISIBLE, currPosLine, 0); }


    int const selectionMode = (doAct == UNDO ? sel.selMode_undo : sel.selMode_redo);
    ISSUE_MESSAGE(Globals.hwndEdit, SCI_SETSELECTIONMODE, (WPARAM)selectionMode, 0);

    // independent from selection mode
    ISSUE_MESSAGE(Globals.hwndEdit, SCI_SETANCHOR, (WPARAM)_anchorPos, 0);
    ISSUE_MESSAGE(Globals.hwndEdit, SCI_SETCURRENTPOS, (WPARAM)_curPos, 0);

    switch (selectionMode)
    {
    case SC_SEL_RECTANGLE: 
      ISSUE_MESSAGE(Globals.hwndEdit, SCI_SETRECTANGULARSELECTIONANCHOR, (WPARAM)_anchorPos, 0);
      ISSUE_MESSAGE(Globals.hwndEdit, SCI_SETRECTANGULARSELECTIONCARET, (WPARAM)_curPos, 0);
      // fall-through

    case SC_SEL_THIN:
      {
        DocPos const anchorVS = (doAct == UNDO ? sel.anchorVS_undo : sel.anchorVS_redo);
        DocPos const currVS = (doAct == UNDO ? sel.curVS_undo : sel.curVS_redo);
        if ((anchorVS != 0) || (currVS != 0)) {
          ISSUE_MESSAGE(Globals.hwndEdit, SCI_SETRECTANGULARSELECTIONANCHORVIRTUALSPACE, (WPARAM)anchorVS, 0);
          ISSUE_MESSAGE(Globals.hwndEdit, SCI_SETRECTANGULARSELECTIONCARETVIRTUALSPACE, (WPARAM)currVS, 0);
        }
      }
      break;

    case SC_SEL_LINES:
    case SC_SEL_STREAM:
    default:
      // nothing to do here
      break;
    }
    ISSUE_MESSAGE(Globals.hwndEdit, SCI_SCROLLCARET, 0, 0);
    ISSUE_MESSAGE(Globals.hwndEdit, SCI_CHOOSECARETX, 0, 0);
    ISSUE_MESSAGE(Globals.hwndEdit, SCI_CANCEL, 0, 0);

  #undef ISSUE_MASSAGE
  }
}


//=============================================================================
//
//  _UndoSelectionMap()
//
//
static int  _UndoRedoActionMap(int token, UndoRedoSelection_t* selection)
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
            EditFileIOStatus* status, bool bSaveCopy)
{
  WCHAR tch[MAX_PATH+40];
  bool fSuccess;
  DWORD dwFileAttributes;

  FormatLngStringW(tch,COUNTOF(tch),(fLoad) ? IDS_MUI_LOADFILE : IDS_MUI_SAVEFILE, PathFindFileName(pszFileName));

  BeginWaitCursor(tch);

  if (fLoad) {
    fSuccess = EditLoadFile(Globals.hwndEdit,pszFileName,bSkipUnicodeDetect,bSkipANSICPDetection,status);
  }
  else {
    int idx;
    if (MRU_FindFile(Globals.pFileMRU,pszFileName,&idx)) {
      Globals.pFileMRU->iEncoding[idx] = status->iEncoding;
      Globals.pFileMRU->iCaretPos[idx] = (Settings.PreserveCaretPos ? SciCall_GetCurrentPos() : 0);
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (Globals.pFileMRU->pszBookMarks[idx])
        LocalFree(Globals.pFileMRU->pszBookMarks[idx]);  // StrDup()
      Globals.pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    fSuccess = EditSaveFile(Globals.hwndEdit,pszFileName, status, bSaveCopy);
  }

  dwFileAttributes = GetFileAttributes(pszFileName);
  s_bFileReadOnly = (dwFileAttributes != INVALID_FILE_ATTRIBUTES && dwFileAttributes & FILE_ATTRIBUTE_READONLY);

  EndWaitCursor();

  return(fSuccess);
}

//=============================================================================
//
// _WarnInconsistentIndentation()
//
static void _WarnInconsistentIndentation(const EditFileIOStatus* const status)
{
  if (((status->indentCount[0] > 0) && (status->indentCount[1] > 0))
      //|| (Settings.TabsAsSpaces && (tabCount > 0))      // existing tabs, should be replaced by spaces
      //|| (!Settings.TabsAsSpaces && (spaceCount > 0))   // indent space, should be populated with tabs
      ) {
    WCHAR szDefault[32];
    WCHAR szStatistic[80];
    StringCchPrintf(szDefault, COUNTOF(szDefault), L"%s(%i)",
      (Settings.TabsAsSpaces ? L"BLANK" : L"TABULATOR"), (Settings.TabsAsSpaces ? Settings.IndentWidth : Settings.TabWidth));
    StringCchPrintf(szStatistic, COUNTOF(szStatistic), L"  # TABULATOR(%i) = %i\n  # BLANK(%i) = %i\n",
                    Settings.TabWidth, status->indentCount[0], Settings.IndentWidth, status->indentCount[1]);

    int const res = MsgBoxLng(MBYESNOWARN, IDS_MUI_WARN_INCONS_INDENTS, szStatistic, szDefault);

    if (res == IDYES) {
      BeginWaitCursor(NULL);
      _BEGIN_UNDO_ACTION_;
      EditIndentBlock(Globals.hwndEdit, SCI_TAB, true, true);
      EditIndentBlock(Globals.hwndEdit, SCI_BACKTAB, true, true);
      _END_UNDO_ACTION_;
      EndWaitCursor();
    }
  }
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
  bool fSuccess = false;

  EditFileIOStatus fioStatus = INIT_FILEIO_STATUS;
  fioStatus.iEOLMode = Settings.DefaultEOLMode;
  fioStatus.iEncoding = CPI_ANSI_DEFAULT;

  if (bNew || bReload) {
    if (EditToggleView(Globals.hwndEdit, false)) {
      EditToggleView(Globals.hwndEdit, true);
    }
  }

  if (!bDontSave)
  {
    if (!FileSave(false,true,false,false))
      return false;
  }

  if (!bReload) { ResetEncryption(); }

  if (bNew) {
    StringCchCopy(Globals.CurrentFile,COUNTOF(Globals.CurrentFile),L"");
    SetDlgItemText(Globals.hwndMain,IDC_FILENAME,Globals.CurrentFile);
    SetDlgItemInt(Globals.hwndMain,IDC_REUSELOCK,GetTickCount(),false);
    if (!s_fKeepTitleExcerpt) { StringCchCopy(s_wchTitleExcerpt, COUNTOF(s_wchTitleExcerpt), L""); }
    FileVars_Init(NULL,0,&Globals.fvCurFile);
    EditSetNewText(Globals.hwndEdit, "", 0);

    SciCall_SetEOLMode(Settings.DefaultEOLMode);
    Encoding_Current(Settings.DefaultEncoding);
    Encoding_HasChanged(Settings.DefaultEncoding);
    
    EditSetNewText(Globals.hwndEdit, "", 0);
    Style_SetDefaultLexer(Globals.hwndEdit);

    s_bFileReadOnly = false;
    _SetDocumentModified(false);

    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateMarginWidth();

    // Terminate file watching
    if (Settings.ResetFileWatching) {
      if (Globals.bChasingDocTail) {
        SendMessage(Globals.hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
      }
      Settings.FileWatchingMode = 0;
    }
    InstallFileWatching(NULL);
    s_bEnableSaveSettings = true;
    UpdateSettingsCmds();
    return true;
  }

  if (StrIsEmpty(lpszFile)) {
    if (!OpenFileDlg(Globals.hwndMain,tch,COUNTOF(tch),NULL))
      return false;
  }
  else
    StringCchCopy(tch,COUNTOF(tch),lpszFile);

  ExpandEnvironmentStringsEx(tch,COUNTOF(tch));

  if (PathIsRelative(tch)) {
    StringCchCopyN(szFileName,COUNTOF(szFileName),Globals.WorkingDirectory,COUNTOF(Globals.WorkingDirectory));
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
    if (s_flagQuietCreate || MsgBoxLng(MBYESNO,IDS_MUI_ASK_CREATE,szFileName) == IDYES) {
      HANDLE hFile = CreateFile(szFileName,
                      GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
                      NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
      Globals.dwLastError = GetLastError();
      fSuccess = (hFile != INVALID_HANDLE_VALUE);
      if (fSuccess) {
        FileVars_Init(NULL,0,&Globals.fvCurFile);
        EditSetNewText(Globals.hwndEdit,"",0);
        Style_SetDefaultLexer(Globals.hwndEdit);
        SciCall_SetEOLMode(Settings.DefaultEOLMode);
        if (Encoding_SrcCmdLn(CPI_GET) != CPI_NONE) {
          fioStatus.iEncoding = Encoding_SrcCmdLn(CPI_GET);
          Encoding_Current(fioStatus.iEncoding);
          Encoding_HasChanged(fioStatus.iEncoding);
        }
        else {
          Encoding_Current(Settings.DefaultEncoding);
          Encoding_HasChanged(Settings.DefaultEncoding);
        }
        s_bFileReadOnly = false;
        EditSetNewText(Globals.hwndEdit,"",0);
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
    if (!bReload && MRU_FindFile(Globals.pFileMRU,szFileName,&idx)) {
      fioStatus.iEncoding = Globals.pFileMRU->iEncoding[idx];
      if (fioStatus.iEncoding > 0)
        Encoding_SrcCmdLn(Encoding_MapUnicode(fioStatus.iEncoding));
    }
    else
      fioStatus.iEncoding = Encoding_Current(CPI_GET);

    fSuccess = FileIO(true,szFileName,bSkipUnicodeDetect,bSkipANSICPDetection,&fioStatus,false);
  }
  if (fSuccess) {
    StringCchCopy(Globals.CurrentFile,COUNTOF(Globals.CurrentFile),szFileName);
    SetDlgItemText(Globals.hwndMain,IDC_FILENAME,Globals.CurrentFile);
    SetDlgItemInt(Globals.hwndMain,IDC_REUSELOCK,GetTickCount(),false);

    if (!s_fKeepTitleExcerpt)
      StringCchCopy(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt),L"");

    if (!s_flagLexerSpecified) // flag will be cleared
      Style_SetLexerFromFile(Globals.hwndEdit,Globals.CurrentFile);

    SciCall_SetEOLMode(fioStatus.iEOLMode);
    Encoding_Current(fioStatus.iEncoding); // load may change encoding
    Encoding_HasChanged(fioStatus.iEncoding);

    int idx = 0;
    DocPos iCaretPos = 0;
    LPCWSTR pszBookMarks = L"";
    if (!bReload && MRU_FindFile(Globals.pFileMRU,szFileName,&idx)) {
      iCaretPos = Globals.pFileMRU->iCaretPos[idx];
      pszBookMarks = Globals.pFileMRU->pszBookMarks[idx];
    }
    MRU_AddFile(Globals.pFileMRU,szFileName,Flags.RelativeFileMRU,Flags.PortableMyDocs,fioStatus.iEncoding,iCaretPos,pszBookMarks);
   
    EditSetBookmarkList(Globals.hwndEdit, pszBookMarks);
    SetFindPattern((Globals.pMRUfind ? Globals.pMRUfind->pszItems[0] : L""));

    if (Flags.ShellUseSystemMRU == 2)
      SHAddToRecentDocs(SHARD_PATHW,szFileName);

    // Install watching of the current file
    if (!bReload && Settings.ResetFileWatching) {
      if (Globals.bChasingDocTail) {
        SendMessage(Globals.hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
      }
      Settings.FileWatchingMode = 0;
    }
    InstallFileWatching(Globals.CurrentFile);

    // the .LOG feature ...
    if (SciCall_GetTextLength() >= 4) {
      char tchLog[6] = { '\0','\0','\0','\0','\0','\0' };
      SciCall_GetText(5, tchLog);
      if (StringCchCompareXA(tchLog,".LOG") == 0) {
        EditJumpTo(Globals.hwndEdit,-1,0);
        _BEGIN_UNDO_ACTION_;
        SendMessage(Globals.hwndEdit,SCI_NEWLINE,0,0);
        SendMessage(Globals.hwndMain,WM_COMMAND,MAKELONG(IDM_EDIT_INSERT_SHORTDATE,1),0);
        EditJumpTo(Globals.hwndEdit,-1,0);
        SendMessage(Globals.hwndEdit,SCI_NEWLINE,0,0);
        _END_UNDO_ACTION_;
        SendMessage(Globals.hwndEdit, SCI_DOCUMENTEND, 0, 0);
        EditEnsureSelectionVisible(Globals.hwndEdit);
      }
      // set historic caret pos
      else if (iCaretPos > 0) 
      {
        SendMessage(Globals.hwndEdit, SCI_GOTOPOS, (WPARAM)iCaretPos, 0);
        // adjust view
        const DocPos iCurPos = SciCall_GetCurrentPos();
        EditJumpTo(Globals.hwndEdit, SciCall_LineFromPosition(iCurPos) + 1, SciCall_GetColumn(iCurPos) + 1);
      }
    }

    //bReadOnly = false;
    _SetDocumentModified(false);
    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateMarginWidth();
    UpdateVisibleUrlHotspot(0);

    // consistent settings file handling (if loaded in editor)
    s_bEnableSaveSettings = (StringCchCompareNIW(Globals.CurrentFile, COUNTOF(Globals.CurrentFile), Globals.IniFile, COUNTOF(Globals.IniFile)) == 0) ? false : true;
    UpdateSettingsCmds();

    // Show warning: Unicode file loaded as ANSI
    if (fioStatus.bUnicodeErr) {
      MsgBoxLng(MBWARN, IDS_MUI_ERR_UNICODE);
    }

    // Show inconsistent line endings warning
    if (fioStatus.bInconsistentEOLs && Settings.WarnInconsistEOLs) 
    {
      WCHAR szDefault[32];
      WCHAR szStatistic[80];
      int const eolm = SciCall_GetEOLMode(); //Settings.DefaultEOLMode;
      StringCchPrintf(szDefault, COUNTOF(szDefault), L"%s", 
        ((eolm == SC_EOL_CRLF) ? L"CRLF (\\r\\n)" : ((eolm == SC_EOL_CR) ? L"CR (\\r)" : L"LF (\\n)")));
      StringCchPrintf(szStatistic, COUNTOF(szStatistic), L"  #CRLF = %i\n  #CR = %i\n  #LF = %i\n",
                      fioStatus.eolCount[SC_EOL_CRLF], fioStatus.eolCount[SC_EOL_CR], fioStatus.eolCount[SC_EOL_LF]);
      int const res = MsgBoxLng(MBYESNOWARN, IDS_MUI_WARN_INCONSIST_EOLS, szStatistic, szDefault);
      if (res == IDYES) {
        SciCall_ConvertEOLs(eolm);
      }
    }

    if (Settings.WarnInconsistentIndents && !Style_IsCurLexerStandard()) {
      EditCheckIndentationConsistency(Globals.hwndEdit, &fioStatus);
      _WarnInconsistentIndentation(&fioStatus);
      // TODO: Set correct Indent mode / verify settings vs. file majority
    }

  }
  else if (!(fioStatus.bFileTooBig || fioStatus.bUnknownExt)) {
    MsgBoxLng(MBWARN, IDS_MUI_ERR_LOADFILE, szFileName);
  }
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
      if (bIsTail && Settings.FileWatchingMode == 2) {
        SendMessage(Globals.hwndEdit, SCI_DOCUMENTEND, 0, 0);
        EditEnsureSelectionVisible(Globals.hwndEdit);
      }
      else if (SciCall_GetTextLength() >= 4) 
      {
        char tch[6] = { '\0','\0','\0','\0','\0','\0' };
        SciCall_GetText(5, tch);
        if (StringCchCompareXA(tch,".LOG") != 0) {
          SciCall_ClearSelections();
          //~EditSetSelectionEx(Globals.hwndEdit, iAnchorPos, iCurPos, vSpcAnchorPos, vSpcCaretPos);
          EditJumpTo(Globals.hwndEdit, iCurrLine+1, iCurColumn+1);
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

  EditFileIOStatus fioStatus = INIT_FILEIO_STATUS;
  fioStatus.iEncoding = Encoding_Current(CPI_GET);
  fioStatus.iEOLMode = SciCall_GetEOLMode();

  bool bIsEmptyNewFile = false;
  if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)) == 0) {
    const DocPos cchText = SciCall_GetTextLength();
    if (cchText == 0)
      bIsEmptyNewFile = true;
    else if (cchText < 1023) {
      char chTextBuf[1024] = { '\0' };
      SciCall_GetText(1023, chTextBuf);
      StrTrimA(chTextBuf," \t\n\r");
      if (StringCchLenA(chTextBuf,COUNTOF(chTextBuf)) == 0)
        bIsEmptyNewFile = true;
    }
  }

  if (!bSaveAlways && ((!IsDocumentModified && !Encoding_HasChanged(CPI_GET)) || bIsEmptyNewFile) && !bSaveAs) {
    int idx;
    if (MRU_FindFile(Globals.pFileMRU,Globals.CurrentFile,&idx)) {
      Globals.pFileMRU->iEncoding[idx] = Encoding_Current(CPI_GET);
      Globals.pFileMRU->iCaretPos[idx] = (Settings.PreserveCaretPos) ? SciCall_GetCurrentPos() : 0;
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      if (Globals.pFileMRU->pszBookMarks[idx])
        LocalFree(Globals.pFileMRU->pszBookMarks[idx]);  // StrDup()
      Globals.pFileMRU->pszBookMarks[idx] = StrDup(wchBookMarks);
    }
    return true;
  }

  if (bAsk)
  {
    // File or "Untitled" ...
    WCHAR tch[MAX_PATH] = { L'\0' };
    if (StringCchLenW(Globals.CurrentFile, COUNTOF(Globals.CurrentFile))) {
      StringCchCopy(tch, COUNTOF(tch), Globals.CurrentFile);
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
  if (!bSaveAs && !bSaveCopy && StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)))
  {
    DWORD dwFileAttributes = GetFileAttributes(Globals.CurrentFile);
    if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
      s_bFileReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY);
    if (s_bFileReadOnly) {
      UpdateToolbar();
      if (MsgBoxLng(MBYESNOWARN,IDS_MUI_READONLY_SAVE,Globals.CurrentFile) == IDYES)
        bSaveAs = true;
      else
        return false;
    }
  }

  // Save As...
  if (bSaveAs || bSaveCopy || StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)) == 0)
  {
    WCHAR tchInitialDir[MAX_PATH] = { L'\0' };
    if (bSaveCopy && StringCchLenW(s_tchLastSaveCopyDir,COUNTOF(s_tchLastSaveCopyDir))) {
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),s_tchLastSaveCopyDir);
      StringCchCopy(tchFile,COUNTOF(tchFile),s_tchLastSaveCopyDir);
      PathCchAppend(tchFile,COUNTOF(tchFile),PathFindFileName(Globals.CurrentFile));
    }
    else
      StringCchCopy(tchFile,COUNTOF(tchFile),Globals.CurrentFile);

    if (SaveFileDlg(Globals.hwndMain,tchFile,COUNTOF(tchFile),tchInitialDir))
    {
      fSuccess = FileIO(false, tchFile, false, true, &fioStatus, bSaveCopy);
      //~if (fSuccess) Encoding_Current(fioStatus.iEncoding); // save should not change encoding
      if (fSuccess)
      {
        if (!bSaveCopy)
        {
          StringCchCopy(Globals.CurrentFile,COUNTOF(Globals.CurrentFile),tchFile);
          SetDlgItemText(Globals.hwndMain,IDC_FILENAME,Globals.CurrentFile);
          SetDlgItemInt(Globals.hwndMain,IDC_REUSELOCK,GetTickCount(),false);
          if (!s_fKeepTitleExcerpt)
            StringCchCopy(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt),L"");
          Style_SetLexerFromFile(Globals.hwndEdit,Globals.CurrentFile);
        }
        else {
          StringCchCopy(s_tchLastSaveCopyDir,COUNTOF(s_tchLastSaveCopyDir),tchFile);
          PathRemoveFileSpec(s_tchLastSaveCopyDir);
        }
      }
    }
    else
      return false;
  }
  else {
    fSuccess = FileIO(false, Globals.CurrentFile, false, true, &fioStatus, false);
    //~if (fSuccess) Encoding_Current(fioStatus.iEncoding); // save should not change encoding
  }

  if (fSuccess)
  {
    if (!bSaveCopy)
    {
      int iCurrEnc = Encoding_Current(CPI_GET);
      Encoding_HasChanged(iCurrEnc);
      const DocPos iCaretPos = SciCall_GetCurrentPos();
      WCHAR wchBookMarks[MRU_BMRK_SIZE] = { L'\0' };
      EditGetBookmarkList(Globals.hwndEdit, wchBookMarks, COUNTOF(wchBookMarks));
      MRU_AddFile(Globals.pFileMRU,Globals.CurrentFile,Flags.RelativeFileMRU,Flags.PortableMyDocs,iCurrEnc,iCaretPos,wchBookMarks);
      if (Flags.ShellUseSystemMRU == 2) {
        SHAddToRecentDocs(SHARD_PATHW, Globals.CurrentFile);
      }
      _SetDocumentModified(false);

      // Install watching of the current file
      if (bSaveAs && Settings.ResetFileWatching) {
        if (Globals.bChasingDocTail) {
          SendMessage(Globals.hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0);
        }
        Settings.FileWatchingMode = 0;
      }
      InstallFileWatching(Globals.CurrentFile);
    }
  }
  else if (!fioStatus.bCancelDataLoss)
  {
    if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile)) > 0) {
      StringCchCopy(tchFile,COUNTOF(tchFile),Globals.CurrentFile);
      StringCchCopy(tchBase,COUNTOF(tchBase),Globals.CurrentFile);
      PathStripPath(tchBase);
    }
    if (!s_flagIsElevated && Globals.dwLastError == ERROR_ACCESS_DENIED) {
      if (IDYES == MsgBoxLng(MBYESNOWARN,IDS_MUI_ERR_ACCESSDENIED,tchFile)) {
        WCHAR lpTempPathBuffer[MAX_PATH];
        WCHAR szTempFileName[MAX_PATH];

        if (GetTempPath(MAX_PATH,lpTempPathBuffer) &&
            GetTempFileName(lpTempPathBuffer,TEXT("NP3"),0,szTempFileName)) {
          if (FileIO(false,szTempFileName,false,true,&fioStatus,true)) {
            //~Encoding_Current(fioStatus.iEncoding); // save should not change encoding
            WCHAR szArguments[2048] = { L'\0' };
            LPWSTR lpCmdLine = GetCommandLine();
            size_t const wlen = StringCchLenW(lpCmdLine,0) + 2;
            LPWSTR lpExe = AllocMem(sizeof(WCHAR)*wlen,HEAP_ZERO_MEMORY);
            LPWSTR lpArgs = AllocMem(sizeof(WCHAR)*wlen,HEAP_ZERO_MEMORY);
            ExtractFirstArgument(lpCmdLine,lpExe,lpArgs,(int)wlen);
            // remove relaunch elevated, we are doing this here already
            lpArgs = StrCutI(lpArgs,L"/u ");
            lpArgs = StrCutI(lpArgs,L"-u ");
            WININFO wi = GetMyWindowPlacement(Globals.hwndMain,NULL);
            StringCchPrintf(szArguments,COUNTOF(szArguments),
              L"/pos %i,%i,%i,%i,%i /tmpfbuf=\"%s\" %s",wi.x,wi.y,wi.cx,wi.cy,wi.max,szTempFileName,lpArgs);
            if (StringCchLenW(tchFile,COUNTOF(tchFile))) {
              if (!StrStrI(szArguments,tchBase)) {
                StringCchPrintf(szArguments,COUNTOF(szArguments),L"%s \"%s\"",szArguments,tchFile);
              }
            }
            s_flagRelaunchElevated = 1;
            if (RelaunchElevated(szArguments)) {
              // set no change and quit
              Encoding_HasChanged(Encoding_Current(CPI_GET));
              _SetDocumentModified(false);
              PostMessage(Globals.hwndMain,WM_CLOSE,0,0);
            }
            else {
              if (PathFileExists(szTempFileName)) {
                DeleteFile(szTempFileName);
              }
              UpdateToolbar();
              MsgBoxLng(MBWARN,IDS_MUI_ERR_SAVEFILE,tchFile);
            }
            FreeMem(lpExe);
            FreeMem(lpArgs);
          }
        }
      }
    }
    else {
      UpdateToolbar();
      MsgBoxLng(MBWARN,IDS_MUI_ERR_SAVEFILE,tchFile);
    }
  }
  //???EditEnsureSelectionVisible(Globals.hwndEdit);
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
    if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),Globals.CurrentFile);
      PathRemoveFileSpec(tchInitialDir);
    }
    else if (StringCchLenW(Settings2.DefaultDirectory,COUNTOF(Settings2.DefaultDirectory))) {
      ExpandEnvironmentStrings(Settings2.DefaultDirectory,tchInitialDir,COUNTOF(tchInitialDir));
      if (PathIsRelative(tchInitialDir)) {
        WCHAR tchModule[MAX_PATH] = { L'\0' };
        GetModuleFileName(NULL,tchModule,COUNTOF(tchModule));
        PathRemoveFileSpec(tchModule);
        PathCchAppend(tchModule,COUNTOF(tchModule),tchInitialDir);
        PathCchCanonicalize(tchInitialDir,COUNTOF(tchInitialDir),tchModule);
      }
    }
    else
      StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),Globals.WorkingDirectory);
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
  ofn.lpstrDefExt = (StringCchLenW(Settings2.DefaultExtension,COUNTOF(Settings2.DefaultExtension))) ? Settings2.DefaultExtension : NULL;

  if (GetOpenFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szFile,COUNTOF(szFile));
    return true;
  }
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
  else if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),Globals.CurrentFile);
    PathRemoveFileSpec(tchInitialDir);
  }
  else if (StringCchLenW(Settings2.DefaultDirectory,COUNTOF(Settings2.DefaultDirectory))) {
    ExpandEnvironmentStrings(Settings2.DefaultDirectory,tchInitialDir,COUNTOF(tchInitialDir));
    if (PathIsRelative(tchInitialDir)) {
      WCHAR tchModule[MAX_PATH] = { L'\0' };
      GetModuleFileName(NULL,tchModule,COUNTOF(tchModule));
      PathRemoveFileSpec(tchModule);
      PathCchAppend(tchModule,COUNTOF(tchModule),tchInitialDir);
      PathCchCanonicalize(tchInitialDir,COUNTOF(tchInitialDir),tchModule);
    }
  }
  else
    StringCchCopy(tchInitialDir,COUNTOF(tchInitialDir),Globals.WorkingDirectory);

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
  ofn.lpstrDefExt = (StringCchLenW(Settings2.DefaultExtension,COUNTOF(Settings2.DefaultExtension))) ? Settings2.DefaultExtension : NULL;

  if (GetSaveFileName(&ofn)) {
    StringCchCopyN(lpstrFile,cchFile,szNewFile,COUNTOF(szNewFile));
    return true;
  }
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

    if (StringCchCompareNIW(szClassName,COUNTOF(szClassName),s_wchWndClass,COUNTOF(s_wchWndClass)) == 0) {

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

    if (StringCchCompareNIW(szClassName,COUNTOF(szClassName),s_wchWndClass,COUNTOF(s_wchWndClass)) == 0) {

      DWORD dwReuseLock = GetDlgItemInt(hwnd,IDC_REUSELOCK,NULL,false);
      if (GetTickCount() - dwReuseLock >= REUSEWINDOWLOCKTIMEOUT) {

        WCHAR tchFileName[MAX_PATH] = { L'\0' };

        if (IsWindowEnabled(hwnd))
          bContinue = FALSE;

        GetDlgItemText(hwnd,IDC_FILENAME,tchFileName,COUNTOF(tchFileName));
        if (StringCchCompareXI(tchFileName,lpFileArg) == 0)
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

  if ((Flags.NoReuseWindow && !Flags.SingleFileInstance) || s_flagStartAsTrayIcon || s_flagNewFromClipboard || s_flagPasteBoard)
    return(false);

  if (Flags.SingleFileInstance && lpFileArg) {

    // Search working directory from second instance, first!
    // lpFileArg is at least MAX_PATH+4 WCHARS
    WCHAR tchTmp[(MAX_PATH+1)] = { L'\0' };

    ExpandEnvironmentStringsEx(lpFileArg,(DWORD)SizeOfMem(lpFileArg)/sizeof(WCHAR));

    if (PathIsRelative(lpFileArg)) {
      StringCchCopyN(tchTmp,COUNTOF(tchTmp),Globals.WorkingDirectory,COUNTOF(Globals.WorkingDirectory));
      PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
      if (PathFileExists(tchTmp))
        StringCchCopy(lpFileArg,(MAX_PATH+1),tchTmp);
      else {
        if (SearchPath(NULL,lpFileArg,NULL,COUNTOF(tchTmp),tchTmp,NULL))
          StringCchCopy(lpFileArg,(MAX_PATH+1),tchTmp);
        else {
          StringCchCopyN(tchTmp,COUNTOF(tchTmp),Globals.WorkingDirectory,COUNTOF(Globals.WorkingDirectory));
          PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
          StringCchCopy(lpFileArg,(MAX_PATH+1),tchTmp);
        }
      }
    }

    else if (SearchPath(NULL,lpFileArg,NULL,COUNTOF(tchTmp),tchTmp,NULL))
      StringCchCopy(lpFileArg,(MAX_PATH+1),tchTmp);

    NormalizePathEx(lpFileArg,(MAX_PATH+1));

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
        params->flagLexerSpecified = s_flagLexerSpecified;
        if (s_flagLexerSpecified && lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData,0)+1,(StringCchLen(lpSchemeArg,0)+1),lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else
          params->iInitialLexer = s_iInitialLexer;
        params->flagJumpTo = s_flagJumpTo;
        params->iInitialLine = s_iInitialLine;
        params->iInitialColumn = s_iInitialColumn;

        params->iSrcEncoding = (lpEncodingArg) ? Encoding_MatchW(lpEncodingArg) : CPI_NONE;
        params->flagSetEncoding = s_flagSetEncoding;
        params->flagSetEOLMode = s_flagSetEOLMode;
        params->flagTitleExcerpt = 0;

        cds.dwData = DATA_NOTEPAD3_PARAMS;
        cds.cbData = (DWORD)SizeOfMem(params);
        cds.lpData = params;

        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
        FreeMem(params);

        return(true);
      }
      // IsWindowEnabled()
      if (IDYES == MsgBoxLng(MBYESNOWARN, IDS_MUI_ERR_PREVWINDISABLED)) {
        return(false);
      }
      return(true);
    }
  }

  if (Flags.NoReuseWindow) {
    return(false);
  }

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
        WCHAR tchTmp[(MAX_PATH+1)] = { L'\0' };

        ExpandEnvironmentStringsEx(lpFileArg,(DWORD)SizeOfMem(lpFileArg)/sizeof(WCHAR));

        if (PathIsRelative(lpFileArg)) {
          StringCchCopyN(tchTmp,COUNTOF(tchTmp),Globals.WorkingDirectory,COUNTOF(Globals.WorkingDirectory));
          PathCchAppend(tchTmp,COUNTOF(tchTmp),lpFileArg);
          if (PathFileExists(tchTmp)) {
            StringCchCopy(lpFileArg, (MAX_PATH+1), tchTmp);
          }
          else {
            if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL)) {
              StringCchCopy(lpFileArg, (MAX_PATH+1), tchTmp);
            }
          }
        }
        else if (SearchPath(NULL, lpFileArg, NULL, COUNTOF(tchTmp), tchTmp, NULL)) {
          StringCchCopy(lpFileArg, (MAX_PATH+1), tchTmp);
        }

        size_t cb = sizeof(np3params);
        cb += (StringCchLenW(lpFileArg,0) + 1) * sizeof(WCHAR);

        if (lpSchemeArg)
          cb += (StringCchLenW(lpSchemeArg,0) + 1) * sizeof(WCHAR);

        size_t cchTitleExcerpt = StringCchLenW(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt));
        if (cchTitleExcerpt) {
          cb += (cchTitleExcerpt + 1) * sizeof(WCHAR);
        }
        LPnp3params params = AllocMem(cb, HEAP_ZERO_MEMORY);
        params->flagFileSpecified = true;
        StringCchCopy(&params->wchData, StringCchLenW(lpFileArg,0)+1,lpFileArg);
        params->flagChangeNotify = s_flagChangeNotify;
        params->flagQuietCreate = s_flagQuietCreate;
        params->flagLexerSpecified = s_flagLexerSpecified;
        if (s_flagLexerSpecified && lpSchemeArg) {
          StringCchCopy(StrEnd(&params->wchData,0)+1, StringCchLen(lpSchemeArg,0)+1,lpSchemeArg);
          params->iInitialLexer = -1;
        }
        else {
          params->iInitialLexer = s_iInitialLexer;
        }
        params->flagJumpTo = s_flagJumpTo;
        params->iInitialLine = s_iInitialLine;
        params->iInitialColumn = s_iInitialColumn;

        params->iSrcEncoding = (lpEncodingArg) ? Encoding_MatchW(lpEncodingArg) : CPI_NONE;
        params->flagSetEncoding = s_flagSetEncoding;
        params->flagSetEOLMode = s_flagSetEOLMode;

        if (cchTitleExcerpt) {
          StringCchCopy(StrEnd(&params->wchData,0)+1,cchTitleExcerpt+1,s_wchTitleExcerpt);
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
    // IsWindowEnabled()
    return ((IDYES == MsgBoxLng(MBYESNOWARN, IDS_MUI_ERR_PREVWINDISABLED)) ? false : true);
  }
  return(false);
}


//=============================================================================
//
//  RelaunchMultiInst()
//
//
bool RelaunchMultiInst() {

  if (Flags.MultiFileArg == 2 && cFileList > 1) {

    LPWSTR lpCmdLineNew = StrDup(GetCommandLine());
    size_t len = StringCchLen(lpCmdLineNew,0) + 1UL;
    LPWSTR lp1 = AllocMem(sizeof(WCHAR)*len, HEAP_ZERO_MEMORY);
    LPWSTR lp2 = AllocMem(sizeof(WCHAR)*len, HEAP_ZERO_MEMORY);

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
      LocalFree(lpFileList[i]); // StrDup()

      STARTUPINFO si;
      ZeroMemory(&si,sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);

      PROCESS_INFORMATION pi;
      ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));

      CreateProcess(NULL,lpCmdLineNew,NULL,NULL,false,0,NULL,Globals.WorkingDirectory,&si,&pi);
    }

    LocalFree(lpCmdLineNew); // StrDup()
    FreeMem(lp1);
    FreeMem(lp2);
    FreeMem(lpFileArg); lpFileArg = NULL;

    return true;
  }

  for (int i = 0; i < cFileList; i++) {
    LocalFree(lpFileList[i]); // StrDup()
  }
  return false;
}


//=============================================================================
//
//  RelaunchElevated()
//
//
bool RelaunchElevated(LPWSTR lpArgs) {

  bool result = false;

  if (!IsVista() || s_flagIsElevated || !s_flagRelaunchElevated || s_flagDisplayHelp)
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
    if (StringCchLenW(Globals.IniFile,COUNTOF(Globals.IniFile)) > 0)
      StringCchPrintf(szArguments,COUNTOF(szArguments),L"/f \"%s\" %s",Globals.IniFile,szArgs);
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
    sei.lpDirectory = Globals.WorkingDirectory;
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
void SnapToWinInfoPos(HWND hwnd, const WININFO* pWinInfo, bool bFullWorkArea)
{
  static WINDOWPLACEMENT s_wndplPrev;
  static bool s_bPrevFullWAFlag = false;
  static bool s_bPrevShowToolbar = true;
  static bool s_bPrevShowStatusbar = true;

  WINDOWPLACEMENT wndpl;
  RECT rcCurrent; GetWindowRect(hwnd, &rcCurrent);

  if (bFullWorkArea) {
    if (s_bPrevFullWAFlag) { // snap to previous rect
      Settings.ShowToolbar = s_bPrevShowToolbar;
      Settings.ShowStatusbar = s_bPrevShowStatusbar;
      wndpl = s_wndplPrev;
    }
    else {
      GetWindowPlacement(hwnd, &s_wndplPrev);
      s_bPrevShowToolbar = Settings.ShowToolbar;
      s_bPrevShowStatusbar = Settings.ShowStatusbar;
      Settings.ShowToolbar = Settings.ShowStatusbar = false;
      wndpl = WindowPlacementFromInfo(hwnd, NULL);
    }
    s_bPrevFullWAFlag = !s_bPrevFullWAFlag;
  }
  else {
    wndpl = WindowPlacementFromInfo(hwnd, pWinInfo);
    if (s_bPrevFullWAFlag) {
      Settings.ShowToolbar = s_bPrevShowToolbar;
      Settings.ShowStatusbar = s_bPrevShowStatusbar;
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
  UpdateMarginWidth();
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
    hIcon = LoadImage(Globals.hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, 
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
  StringCchCopy(nid.szTip,COUNTOF(nid.szTip), MKWCS(APPNAME));

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
  ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_TIP;

  WCHAR tchTitle[256] = { L'\0' };
  if (StringCchLenW(s_wchTitleExcerpt,COUNTOF(s_wchTitleExcerpt))) {
    WCHAR tchFormat[32] = { L'\0' };
    GetLngString(IDS_MUI_TITLEEXCERPT,tchFormat,COUNTOF(tchFormat));
    StringCchPrintf(tchTitle,COUNTOF(tchTitle),tchFormat,s_wchTitleExcerpt);
  }

  else if (StringCchLenW(Globals.CurrentFile,COUNTOF(Globals.CurrentFile))) {
    SHFILEINFO shfi;
    SHGetFileInfo2(Globals.CurrentFile,FILE_ATTRIBUTE_NORMAL,
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
  // Terminate
  if (!Settings.FileWatchingMode || !lpszFile || StringCchLen(lpszFile,MAX_PATH) == 0)
  {
    if (s_bRunningWatch)
    {
      if (s_hChangeHandle) {
        FindCloseChangeNotification(s_hChangeHandle);
        s_hChangeHandle = NULL;
      }
      KillTimer(NULL,ID_WATCHTIMER);
      s_bRunningWatch = false;
      s_dwChangeNotifyTime = 0;
    }
  }
  else  // Install
  {
    // Terminate previous watching
    if (s_bRunningWatch) {
      if (s_hChangeHandle) {
        FindCloseChangeNotification(s_hChangeHandle);
        s_hChangeHandle = NULL;
      }
      s_dwChangeNotifyTime = 0;
    }

    // No previous watching installed, so launch the timer first
    else
      SetTimer(NULL,ID_WATCHTIMER,Settings2.FileCheckInverval,WatchTimerProc);

    WCHAR tchDirectory[MAX_PATH] = { L'\0' };
    StringCchCopy(tchDirectory,COUNTOF(tchDirectory),lpszFile);
    PathRemoveFileSpec(tchDirectory);

    // Save data of current file
    HANDLE hFind = FindFirstFile(Globals.CurrentFile,&s_fdCurFile);
    if (hFind != INVALID_HANDLE_VALUE)
      FindClose(hFind);
    else
      ZeroMemory(&s_fdCurFile,sizeof(WIN32_FIND_DATA));

    s_hChangeHandle = FindFirstChangeNotification(tchDirectory,false,
      FILE_NOTIFY_CHANGE_FILE_NAME  | \
      FILE_NOTIFY_CHANGE_DIR_NAME   | \
      FILE_NOTIFY_CHANGE_ATTRIBUTES | \
      FILE_NOTIFY_CHANGE_SIZE | \
      FILE_NOTIFY_CHANGE_LAST_WRITE);

    s_bRunningWatch = true;
    s_dwChangeNotifyTime = 0;
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
  if (s_bRunningWatch)
  {
    if (s_dwChangeNotifyTime > 0 && GetTickCount() - s_dwChangeNotifyTime > Settings2.AutoReloadTimeout)
    {
      if (s_hChangeHandle) {
        FindCloseChangeNotification(s_hChangeHandle);
        s_hChangeHandle = NULL;
      }
      KillTimer(NULL,ID_WATCHTIMER);
      s_bRunningWatch = false;
      s_dwChangeNotifyTime = 0;
      SendMessage(Globals.hwndMain,WM_CHANGENOTIFY,0,0);
    }

    // Check Change Notification Handle
    else if (WAIT_OBJECT_0 == WaitForSingleObject(s_hChangeHandle,0))
    {
      // Check if the changes affect the current file
      WIN32_FIND_DATA fdUpdated;
      HANDLE hFind = FindFirstFile(Globals.CurrentFile,&fdUpdated);
      if (INVALID_HANDLE_VALUE != hFind)
        FindClose(hFind);
      else
        // The current file has been removed
        ZeroMemory(&fdUpdated,sizeof(WIN32_FIND_DATA));

      // Check if the file has been changed
      if (CompareFileTime(&s_fdCurFile.ftLastWriteTime,&fdUpdated.ftLastWriteTime) != 0 ||
            s_fdCurFile.nFileSizeLow != fdUpdated.nFileSizeLow ||
            s_fdCurFile.nFileSizeHigh != fdUpdated.nFileSizeHigh)
      {
        // Shutdown current watching and give control to main window
        if (s_hChangeHandle) {
          FindCloseChangeNotification(s_hChangeHandle);
          s_hChangeHandle = NULL;
        }
        if (Settings.FileWatchingMode == 2) {
          s_bRunningWatch = true; /* ! */
          s_dwChangeNotifyTime = GetTickCount();
        }
        else {
          KillTimer(NULL,ID_WATCHTIMER);
          s_bRunningWatch = false;
          s_dwChangeNotifyTime = 0;
          SendMessage(Globals.hwndMain,WM_CHANGENOTIFY,0,0);
        }
      }

      else
        FindNextChangeNotification(s_hChangeHandle);
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
  if ((s_dwLastCopyTime > 0) && ((GetTickCount() - s_dwLastCopyTime) > 200)) {

    if (SendMessage(Globals.hwndEdit,SCI_CANPASTE,0,0)) {

      bool bAutoIndent2 = Settings.AutoIndent;
      Settings.AutoIndent = 0;
      EditJumpTo(Globals.hwndEdit,-1,0);
      _BEGIN_UNDO_ACTION_;
      if (SendMessage(Globals.hwndEdit, SCI_GETLENGTH, 0, 0) > 0) {
        SendMessage(Globals.hwndEdit, SCI_NEWLINE, 0, 0);
      }
      SendMessage(Globals.hwndEdit,SCI_PASTE,0,0);
      SendMessage(Globals.hwndEdit,SCI_NEWLINE,0,0);
      _END_UNDO_ACTION_;
      EditEnsureSelectionVisible(Globals.hwndEdit);
      Settings.AutoIndent = bAutoIndent2;
    }
    s_dwLastCopyTime = 0;
  }

  UNUSED(dwTime);
  UNUSED(idEvent);
  UNUSED(uMsg);
  UNUSED(hwnd);
}

///  End of Notepad3.c  ///
