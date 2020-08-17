// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* minipath.c                                                                  *
*   Main application window functionality                                     *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#define _WIN32_WINNT 0x601
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <strsafe.h>
#include <muiload.h>

#include "dlapi.h"
#include "dialogs.h"
#include "config.h"
#include "minipath.h"
#include "resource.h"


SETTINGS_T Settings;
SETTINGS_T Defaults;

WCHAR     g_wchIniFile[MAX_PATH];
WCHAR     g_wchIniFile2[MAX_PATH];
WCHAR     g_wchNP3IniFile[MAX_PATH];

//HICON     g_hDlgIcon128 = NULL;
HICON     g_hDlgIconBig = NULL;
HICON     g_hDlgIconSmall = NULL;

WCHAR     g_tchPrefLngLocName[LOCALE_NAME_MAX_LENGTH + 1];
LANGID    g_iPrefLANGID;


/******************************************************************************
*
* Local Variables for minipath.c
*
*/
static HWND      hwndStatus;
static HWND      hwndToolbar;
static HWND      hwndReBar;


#define NUMTOOLBITMAPS  15
#define NUMINITIALTOOLS  6

#define TBFILTERBMP 13

TBBUTTON  tbbMainWnd[] = { {0,IDT_HISTORY_BACK,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {1,IDT_HISTORY_FORWARD,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {2,IDT_UPDIR,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {3,IDT_ROOT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {4,IDT_VIEW_FAVORITES,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {5,IDT_FILE_PREV,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {6,IDT_FILE_NEXT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {7,IDT_FILE_RUN,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {8,IDT_FILE_QUICKVIEW,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {9,IDT_FILE_SAVEAS,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {10,IDT_FILE_COPYMOVE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {11,IDT_FILE_DELETE,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {12,IDT_FILE_DELETE2,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {13,IDT_VIEW_FILTER,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0},
                           {0,0,0,TBSTYLE_SEP,0,0} };

HWND      hwndDriveBox;
HWND      hwndDirList;

HWND      hwndMain;

HANDLE    hChangeHandle = NULL;

HISTORY   g_mHistory;

WCHAR     g_wchIniFile[MAX_PATH] = L"";
WCHAR     g_wchIniFile2[MAX_PATH] = L"";
WCHAR     g_wchNP3IniFile[MAX_PATH] = L"";

int       nIdFocus = IDC_DIRLIST;
int       cyReBar;
int       cyReBarFrame;
int       cyDriveBoxFrame;

LPWSTR    lpPathArg = NULL;
LPWSTR    lpFilterArg = NULL;

UINT      wFuncCopyMove = FO_COPY;

UINT      msgTaskbarCreated = 0;

UseTargetApp eUseTargetApplication = UTA_DEFINE_TARGET;
TargetAppMode eTargetApplicationMode = TAM_ALWAYS_RUN;

WCHAR szTargetApplication[MAX_PATH] = L"";
WCHAR szTargetApplicationParams[MAX_PATH] = L"";
WCHAR szTargetApplicationWndClass[MAX_PATH] = L"";
WCHAR szDDEMsg[256] = L"";
WCHAR szDDEApp[256] = L"";
WCHAR szDDETopic[256] = L"";

BOOL bHasQuickview = FALSE;

UINT16    g_uWinVer;

HINSTANCE            g_hInstance = NULL;
HMODULE              g_hLngResContainer = NULL;

WCHAR                g_tchPrefLngLocName[LOCALE_NAME_MAX_LENGTH + 1];
LANGID               g_iPrefLANGID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
// 'en-US' internal default
static WCHAR* const  g_tchAvailableLanguages = L"af-ZA be-BY de-DE en-GB es-ES es-MX fr-FR hi-IN hu-HU id-ID it-IT ja-JP ko-KR nl-NL pl-PL pt-BR pt-PT ru-RU sk-SK sv-SE tr-TR vi-VN zh-CN zh-TW";


//=============================================================================
//
// Flags
//
int flagNoReuseWindow   = 0;
int flagStartAsTrayIcon = 0;
int flagPortableMyDocs  = 0;
int flagGotoFavorites   = 0;
int flagNoFadeHidden    = 0;
int flagToolbarLook     = 0;
int flagPosParam        = 0;



//=============================================================================
//
//  _LngStrToMultiLngStr
//
//
static BOOL __fastcall _LngStrToMultiLngStr(WCHAR* pLngStr, WCHAR* pLngMultiStr, size_t lngMultiStrSize)
{
  BOOL rtnVal = TRUE;

  size_t strLen = (size_t)lstrlen(pLngStr);

  if ((strLen > 0) && pLngMultiStr && (lngMultiStrSize > 0)) {
    WCHAR* lngMultiStrPtr = pLngMultiStr;
    WCHAR* last = pLngStr + (Has_UTF16_BOM(pLngStr) ? 1 : 0);
    while (last && rtnVal) {
      // make sure you validate the user input
      WCHAR* next = StrNextTok(last, L",; :");
      if (next) { *next = L'\0'; }
      strLen = (size_t)lstrlen(last);
      if ((strLen > 0) && IsValidLocaleName(last)) {
        lngMultiStrPtr[0] = L'\0';
        rtnVal &= (lstrcat(lngMultiStrPtr, last) != NULL);
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
static HMODULE __fastcall _LoadLanguageResources(const WCHAR* localeName, LANGID const langID)
{
  BOOL bLngAvailable = (StrStrIW(g_tchAvailableLanguages, localeName) != NULL);
  if (!bLngAvailable) { return NULL; }

  WCHAR tchAvailLngs[512] = { L'\0' };
  StringCchCopyW(tchAvailLngs, 512, g_tchAvailableLanguages);
  WCHAR tchUserLangMultiStrg[512] = { L'\0' };
  if (!_LngStrToMultiLngStr(tchAvailLngs, tchUserLangMultiStrg, 512)) {
    GetLastErrorToMsgBox(L"Trying to load Language resource!", ERROR_MUI_INVALID_LOCALE_NAME);
    return NULL;
  }

  // set the appropriate fallback list
  DWORD langCount = 0;
  // using SetProcessPreferredUILanguages is recommended for new applications (esp. multi-threaded applications)
  if (!SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, tchUserLangMultiStrg, &langCount) || (langCount == 0))
  {
    GetLastErrorToMsgBox(L"Trying to set preferred Language!", ERROR_RESOURCE_LANG_NOT_FOUND);
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

  HMODULE const hLangResourceContainer = LoadMUILibraryW(L"lng/mplng.dll", MUI_LANGUAGE_NAME, langID);

  // MUI Language for common controls
  InitMUILanguage(langID);

  //if (!hLangResourceContainer)
  //{
  //  GetLastErrorToMsgBox(L"LoadMUILibrary", 0);
  //  return NULL;
  //}

  return hLangResourceContainer;
}


//=============================================================================
//
//  WinMain()
//
//
int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInst,LPWSTR lpCmdLine,int nCmdShow)
{
  UNUSED(hPrevInst);

  MSG    msg;
  HWND   hwnd;
  HACCEL hAcc;


  // Set global variable g_hInstance
  g_hInstance = hInstance;

  // Set the Windows version global variable
  #pragma warning (push)
  #pragma warning (disable : 4996 ) /* deprecated method */
  #pragma message ("TODO: use #include <VersionHelpers.h> to replace GetVersion() ")
  g_uWinVer = LOWORD(GetVersion());
  g_uWinVer = MAKEWORD(HIBYTE(g_uWinVer),LOBYTE(g_uWinVer));
  #pragma warning (pop)

  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

  // Command Line, Ini File and Flags
  InitDefaultSettings();
  ParseCommandLine();
  FindIniFile();
  TestIniFile();
  CreateIniFile();
  LoadFlags();

  // Try to activate another window
  if (ActivatePrevInst()) { return 0; }

  // Init OLE and Common Controls
  OleInitialize(NULL);

  INITCOMMONCONTROLSEX icex;
  ZeroMemory(&icex, sizeof(INITCOMMONCONTROLSEX));
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC  = ICC_WIN95_CLASSES|ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_USEREX_CLASSES;
  InitCommonControlsEx(&icex);

  msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

  // Load Settings
  LoadSettings();

  // ----------------------------------------------------
  // MultiLingual
  //
  BOOL bPrefLngNotAvail = FALSE;

  int res = 0;
  if (lstrlenW(g_tchPrefLngLocName) > 0) {
    WCHAR wchLngLocalName[LOCALE_NAME_MAX_LENGTH];
    res = ResolveLocaleName(g_tchPrefLngLocName, wchLngLocalName, LOCALE_NAME_MAX_LENGTH);
    if (res > 0) {
      StringCchCopy(g_tchPrefLngLocName, COUNTOF(g_tchPrefLngLocName), wchLngLocalName); // put back resolved name
    }
    // get LANGID
    g_iPrefLANGID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    res = GetLocaleInfoEx(g_tchPrefLngLocName, LOCALE_ILANGUAGE | LOCALE_RETURN_NUMBER, (LPWSTR)&g_iPrefLANGID, sizeof(LANGID));
  }

  if (res == 0) // No preferred language defined or retrievable, try to get User UI Language
  {
    //~GetUserDefaultLocaleName(&g_tchPrefLngLocName[0], COUNTOF(g_tchPrefLngLocName));
    ULONG numLngs = 0;
    DWORD cchLngsBuffer = 0;
    BOOL hr = GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLngs, NULL, &cchLngsBuffer);
    if (hr) {
      WCHAR* pwszLngsBuffer = LocalAlloc(LPTR, (cchLngsBuffer + 2) * sizeof(WCHAR));
      if (pwszLngsBuffer) {
        hr = GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLngs, pwszLngsBuffer, &cchLngsBuffer);
        if (hr && (numLngs > 0)) {
          // get the first 
          StringCchCopy(g_tchPrefLngLocName, COUNTOF(g_tchPrefLngLocName), pwszLngsBuffer);
          g_iPrefLANGID = LANGIDFROMLCID(LocaleNameToLCID(g_tchPrefLngLocName, 0));
          res = 1;
        }
        LocalFree(pwszLngsBuffer);
      }
    }
    if (res == 0) { // last try
      g_iPrefLANGID = GetUserDefaultUILanguage();
      LCID const lcid = MAKELCID(g_iPrefLANGID, SORT_DEFAULT);
      res = LCIDToLocaleName(lcid, g_tchPrefLngLocName, COUNTOF(g_tchPrefLngLocName), 0);
    }
  }

  g_hLngResContainer = _LoadLanguageResources(g_tchPrefLngLocName, g_iPrefLANGID);

  if (!g_hLngResContainer) // fallback en-US (1033)
  {
    LANGID const langID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    g_hLngResContainer = g_hInstance;
    InitMUILanguage(langID);
    if (g_iPrefLANGID != langID) { bPrefLngNotAvail = TRUE; }
  }
  // ----------------------------------------------------


  if (!InitApplication(hInstance)) { return FALSE; }

  hwnd = InitInstance(hInstance, lpCmdLine, nCmdShow);
  if (!hwnd) { return FALSE; }

  hAcc = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_MAINWND));

  if (bPrefLngNotAvail) {
    ErrorMessage(2, IDS_WARN_PREF_LNG_NOT_AVAIL, g_tchPrefLngLocName);
  }

  while (GetMessage(&msg,NULL,0,0))
  {
    if (!TranslateAccelerator(hwnd,hAcc,&msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  
  OleUninitialize();

  return(int)(msg.wParam);
}


//=============================================================================
//
//  InitApplication()
//
//
BOOL InitApplication(HINSTANCE hInstance)
{
  // ICON_BIG
  int const cxb = GetSystemMetrics(SM_CXICON);
  int const cyb = GetSystemMetrics(SM_CYICON);
  // ICON_SMALL
  int const cxs = GetSystemMetrics(SM_CXSMICON);
  int const cys = GetSystemMetrics(SM_CYSMICON);

  //UINT const fuLoad = LR_DEFAULTCOLOR | LR_SHARED;

  //if (!g_hDlgIcon128) {
  //  LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), 128, 128, &g_hDlgIcon128);
  //}
  if (!g_hDlgIconBig) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), cxb, cyb, &g_hDlgIconBig);
  }
  if (!g_hDlgIconSmall) {
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDR_MAINWND), cxs, cys, &g_hDlgIconSmall);
  }

  WNDCLASS wc;
  ZeroMemory(&wc, sizeof(WNDCLASS));

  wc.style         = CS_BYTEALIGNWINDOW;
  wc.lpfnWndProc   = (WNDPROC)MainWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = g_hDlgIconSmall;
  wc.hCursor       = LoadCursor(hInstance,IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = WC_MINIPATH;

  return RegisterClass(&wc);

}


//=============================================================================
//
//  InitInstance()
//
//
HWND InitInstance(HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow)
{
  UNUSED(pszCmdLine);

  WININFO wi = Settings.wi;

  RECT rc;
  rc.left = wi.x;  
  rc.top = wi.y;  
  rc.right = wi.x + wi.cx;  
  rc.bottom = wi.y + wi.cy;
  RECT rc2;
  MONITORINFO mi;


  HMONITOR hMonitor = MonitorFromRect(&rc,MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor,&mi);

  if (wi.x == CW_USEDEFAULT || wi.y == CW_USEDEFAULT ||
      wi.cx == CW_USEDEFAULT || wi.cy == CW_USEDEFAULT) {

    // default window position
    SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
    wi.x = rc.left + 16;
    wi.y = rc.top + 16;
    wi.cx = 272;
    wi.cy = 640;
  }

  else {

    // fit window into working area of current monitor
    wi.x += (mi.rcWork.left - mi.rcMonitor.left);
    wi.y += (mi.rcWork.top - mi.rcMonitor.top);
    if (wi.x < mi.rcWork.left)
      wi.x = mi.rcWork.left;
    if (wi.y < mi.rcWork.top)
      wi.y = mi.rcWork.top;
    if (wi.x + wi.cx > mi.rcWork.right) {
      wi.x -= (wi.x + wi.cx - mi.rcWork.right);
      if (wi.x < mi.rcWork.left)
        wi.x = mi.rcWork.left;
      if (wi.x + wi.cx > mi.rcWork.right)
        wi.cx = mi.rcWork.right - wi.x;
    }
    if (wi.y + wi.cy > mi.rcWork.bottom) {
      wi.y -= (wi.y + wi.cy - mi.rcWork.bottom);
      if (wi.y < mi.rcWork.top)
        wi.y = mi.rcWork.top;
      if (wi.y + wi.cy > mi.rcWork.bottom)
        wi.cy = mi.rcWork.bottom - wi.y;
    }
    SetRect(&rc,wi.x,wi.y,wi.x+wi.cx,wi.y+wi.cy);
    if (!IntersectRect(&rc2,&rc,&mi.rcWork)) {
      wi.y = mi.rcWork.top + 16;
      wi.cy = mi.rcWork.bottom - mi.rcWork.top - 32;
      wi.cx = min(mi.rcWork.right - mi.rcWork.left - 32,wi.cy);
      wi.x = mi.rcWork.right - wi.cx - 16;
    }
  }

  hwndMain = CreateWindowEx(
               0,
               WC_MINIPATH,
               L"MinPath",
               WS_MINIPATH,
               wi.x,
               wi.y,
               wi.cx,
               wi.cy,
               NULL,
               NULL,
               hInstance,
               NULL);

  if (Settings.bAlwaysOnTop)
    SetWindowPos(hwndMain,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

  if (Settings.g_bTransparentMode) {
    int const iAlphaPercent = IniFileGetInt(g_wchIniFile, L"Settings2", L"OpacityLevel", 75);
    SetWindowTransparentMode(hwndMain, TRUE, clampi(iAlphaPercent, 0, 100));
  }

  if (!flagStartAsTrayIcon) {
    ShowWindow(hwndMain,nCmdShow);
    UpdateWindow(hwndMain);
  }
  else {
    ShowWindow(hwndMain,SW_HIDE);    // trick ShowWindow()
    ShowNotifyIcon(hwndMain,TRUE);
  }

  // Pathname parameter
  if (lpPathArg)
  {
    DisplayPath(lpPathArg,IDS_ERR_CMDLINE);
    GlobalFree(lpPathArg);
  }

  // Use a startup directory
  else if (Settings.iStartupDir)
  {
    if (Settings.iStartupDir == 1)
    {
      WCHAR tch[MAX_PATH];
      if (IniFileGetString(g_wchIniFile, L"Settings", L"MRUDirectory", L"", tch, COUNTOF(tch)))
        DisplayPath(tch,IDS_ERR_STARTUPDIR);
      else
        ErrorMessage(2,IDS_ERR_STARTUPDIR);
    }
    else
      DisplayPath(Settings.g_tchFavoritesDir,IDS_ERR_STARTUPDIR);
  }

  // Favorites
  else if (flagGotoFavorites)
    DisplayPath(Settings.g_tchFavoritesDir,IDS_ERR_FAVORITES);

  // Update Dirlist
  if (!ListView_GetItemCount(hwndDirList))
    PostMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);

  Settings.wi = wi;

  return(hwndMain);
}


//=============================================================================
//
// _SetTargetAppMenuEntry()
//
// Change "Open" Context Menu Item
//
static void __fastcall _SetTargetAppMenuEntry(HMENU hMenu)
{
  if (!hMenu) { return; }

  LoadTargetParamsOnce();

  DLITEM dli = { DLI_ALL, L"", L"", DLE_NONE };
  DirList_GetItem(hwndDirList, -1, &dli);
  if (dli.ntype != DLE_DIR) {
    WCHAR wchMenuEntry[MAX_PATH] = { L'\0' };
    WCHAR wchTargetAppName[MAX_PATH] = { L'\0' };
    if (eUseTargetApplication != 0xFB) {
      lstrcpy(wchTargetAppName, szTargetApplication);
      PathStripPath(wchTargetAppName);
      PathRemoveExtension(wchTargetAppName);
    }
    else if ((eUseTargetApplication != UTA_UNDEFINED) && StrIsEmpty(wchTargetAppName)) {
      eUseTargetApplication = UTA_LAUNCH_TARGET;
      lstrcpy(wchTargetAppName, L"Notepad3");
    }

    if ((eUseTargetApplication == UTA_DEFINE_TARGET) || 
       ((eUseTargetApplication != UTA_UNDEFINED) && StrIsEmpty(wchTargetAppName))) {
      lstrcpy(wchTargetAppName, L"...");
    }

    if (StrIsNotEmpty(wchTargetAppName)){
      FormatLngStringW(wchMenuEntry, COUNTOF(wchMenuEntry), IDS_OPEN_FILE_WITH, wchTargetAppName);
      MENUITEMINFO menuitem;
      ZeroMemory(&menuitem, sizeof(MENUITEMINFO));
      menuitem.cbSize = sizeof(MENUITEMINFO);
      menuitem.fMask = MIIM_TYPE | MIIM_DATA;
      GetMenuItemInfo(hMenu, IDM_FILE_OPEN, FALSE, &menuitem);
      menuitem.dwTypeData = wchMenuEntry;
      SetMenuItemInfo(hMenu, IDM_FILE_OPEN, FALSE, &menuitem);
    }
  }
  SetMenuDefaultItem(GetSubMenu(hMenu, 0), IDM_FILE_OPEN, FALSE);
}



//=============================================================================
//
//  MainWndProc()
//
//  Messages are distributed to the MsgXXX-handlers
//
//
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static BOOL bShutdownOK;

  switch(umsg)
  {

    case WM_CREATE:
      {
        // Init directory watching
        // iAutoRefreshRate is in 1/10 sec
      int iAutoRefreshRate = IniFileGetInt(g_wchIniFile, L"Settings2", L"AutoRefreshRate", 30);
        if (iAutoRefreshRate)
          SetTimer(hwnd,ID_TIMER,(iAutoRefreshRate * 100),NULL);
        return MsgCreate(hwnd,wParam,lParam);
      }


    case WM_DESTROY:
    case WM_ENDSESSION:
      if (!bShutdownOK) {

        WINDOWPLACEMENT wndpl;

        // Terminate directory watching
        KillTimer(hwnd,ID_TIMER);
        FindCloseChangeNotification(hChangeHandle);

        // GetWindowPlacement
        wndpl.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd,&wndpl);

        Settings.wi.x = wndpl.rcNormalPosition.left;
        Settings.wi.y = wndpl.rcNormalPosition.top;
        Settings.wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        Settings.wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;

        DirList_Destroy(hwndDirList);
        DragAcceptFiles(hwnd,FALSE);

        History_Uninit(&g_mHistory);

        // prepare save
        Toolbar_GetButtons(hwndToolbar, IDT_HISTORY_BACK, Settings.tchToolbarButtons, COUNTOF(Settings.tchToolbarButtons));

        SaveSettings(FALSE);

        bShutdownOK = TRUE;
      }
      if (umsg == WM_DESTROY)
        PostQuitMessage(0);
      break;


    // Reinitialize theme-dependent values and resize windows
    case WM_THEMECHANGED:
      MsgThemeChanged(hwnd,wParam,lParam);
      break;


    // update colors of DirList manually
    case WM_SYSCOLORCHANGE:
      {
        LRESULT lret = DefWindowProc(hwnd,umsg,wParam,lParam);

        if (lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter) {
          ListView_SetTextColor(hwndDirList,(Settings.bDefCrFilter) ? GetSysColor(COLOR_WINDOWTEXT) : Settings.crFilter);
          ListView_RedrawItems(hwndDirList,0,ListView_GetItemCount(hwndDirList)-1);
        }
        else {
          ListView_SetTextColor(hwndDirList,(Settings.bDefCrNoFilt) ? GetSysColor(COLOR_WINDOWTEXT) : Settings.crNoFilt);
          ListView_RedrawItems(hwndDirList,0,ListView_GetItemCount(hwndDirList)-1);
        }

        return(lret);
      }


    case WM_NCLBUTTONDOWN:
    case WM_NCMOUSEMOVE:
    case WM_NCLBUTTONUP:
    case WM_NCRBUTTONDOWN:
    case WM_NCMBUTTONDOWN:
    case WM_NCRBUTTONUP:
    case WM_NCMBUTTONUP:
      {
        MSG  msg;
        HWND hwndTT = (HWND)SendMessage(hwndToolbar,TB_GETTOOLTIPS,0,0);

        if (wParam != HTCAPTION) {
          SendMessage(hwndTT,TTM_POP,0,0);
          return DefWindowProc(hwnd,umsg,wParam,lParam);
        }

        msg.hwnd = hwnd;
        msg.message = umsg;
        msg.wParam = wParam;
        msg.lParam = lParam;
        msg.time = GetMessageTime();
        msg.pt.x = HIWORD(GetMessagePos());
        msg.pt.y = LOWORD(GetMessagePos());

        SendMessage(hwndTT,TTM_RELAYEVENT,0,(LPARAM)&msg);
      }
      return DefWindowProc(hwnd,umsg,wParam,lParam);


    case WM_TIMER:
      // Check Change Notification Handle
      if (WAIT_OBJECT_0 == WaitForSingleObject(hChangeHandle,0))
      {
        // Store information about currently selected item
        DLITEM dli;
        dli.mask  = DLI_ALL;
        dli.ntype = DLE_NONE;
        DirList_GetItem(hwndDirList,-1,&dli);

        FindNextChangeNotification(hChangeHandle);
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);

        // must use SendMessage() !!
        if (dli.ntype != DLE_NONE)
          DirList_SelectItem(hwndDirList,dli.szDisplayName,dli.szFileName);
      }
      break;


    case WM_SIZE:
      MsgSize(hwnd,wParam,lParam);
      break;


    case WM_SETFOCUS:
      SetFocus(GetDlgItem(hwnd, nIdFocus));
      break;


    case WM_DROPFILES:
      {
        WCHAR szBuf[MAX_PATH+40];
        HDROP hDrop = (HDROP)wParam;

        if (IsIconic(hwnd))
          ShowWindow(hwnd,SW_RESTORE);

        //SetForegroundWindow(hwnd);

        DragQueryFile(hDrop,0,szBuf,COUNTOF(szBuf));
        DisplayPath(szBuf,IDS_ERR_DROP1);

        if (DragQueryFile(hDrop,(UINT)(-1),NULL,0) > 1)
          ErrorMessage(1,IDS_ERR_DROP2);

        DragFinish(hDrop);
      }
      break;


    case WM_COPYDATA:
      {
        PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;

        if (pcds->dwData == DATA_MINIPATH_PATHARG)
        {
          LPWSTR lpsz = LocalAlloc(LPTR,pcds->cbData);
          CopyMemory(lpsz,pcds->lpData,pcds->cbData);

          DisplayPath(lpsz,IDS_ERR_CMDLINE);

          LocalFree(lpsz);
        }
      }
      return TRUE;


    case WM_CONTEXTMENU:
    {
      int   imenu = 0;
      DWORD dwpts;
      int   nID = GetDlgCtrlID((HWND)wParam);

      if (nID != IDC_DIRLIST && nID != IDC_DRIVEBOX &&
          nID != IDC_TOOLBAR && nID != IDC_STATUSBAR &&
          nID != IDC_REBAR)
        return DefWindowProc(hwnd,umsg,wParam,lParam);

      HMENU hmenu = LoadMenu(g_hLngResContainer,MAKEINTRESOURCE(IDR_MAINWND));
      _SetTargetAppMenuEntry(hmenu);

      switch(nID)
      {
        case IDC_DIRLIST:
          if (ListView_GetSelectedCount(hwndDirList))
            imenu = 0;
          else
            imenu = 1;
          break;

        case IDC_DRIVEBOX:
          imenu = 2;
          break;

        case IDC_TOOLBAR:
        case IDC_STATUSBAR:
        case IDC_REBAR:
          imenu = 3;
          break;
      }

      dwpts = GetMessagePos();

      TrackPopupMenuEx(GetSubMenu(hmenu,imenu),
        TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
        (int)(short)LOWORD(dwpts)+1,(int)(short)HIWORD(dwpts)+1,hwnd,NULL);

      DestroyMenu(hmenu);
    }
    break;


    case WM_INITMENU:
      MsgInitMenu(hwnd,wParam,lParam);
      break;


    case WM_NOTIFY:
      return MsgNotify(hwnd,wParam,lParam);


    case WM_COMMAND:
      return MsgCommand(hwnd,wParam,lParam);


    case WM_SYSCOMMAND:
      switch (wParam)
      {
        case SC_MINIMIZE:
        case SC_MINIMIZE | 0x02:
          ShowOwnedPopups(hwnd,FALSE);
          if (!Settings.bMinimizeToTray)
            return DefWindowProc(hwnd,umsg,wParam,lParam);
          else {
            MinimizeWndToTray(hwnd);
            ShowNotifyIcon(hwnd,TRUE);
          }
          break;

        case SC_RESTORE: {
          LRESULT lrv = DefWindowProc(hwnd,umsg,wParam,lParam);
          ShowOwnedPopups(hwnd,TRUE);
          return(lrv);
        }

        case SC_ALWAYSONTOP:
          if (Settings.bAlwaysOnTop) {
            Settings.bAlwaysOnTop = 0;
            SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
          }
          else {
            Settings.bAlwaysOnTop = 1;
            SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
          }
          break;

        case SC_ABOUT:
          ThemedDialogBox(g_hLngResContainer,MAKEINTRESOURCE(IDD_ABOUT),hwnd,AboutDlgProc);
          break;

        default:
          return DefWindowProc(hwnd,umsg,wParam,lParam);
      }
      break;


    case WM_TRAYMESSAGE:
      switch(lParam)
      {
        case WM_RBUTTONUP: {

            HMENU hMenu = LoadMenu(g_hLngResContainer,MAKEINTRESOURCE(IDR_MAINWND));
            HMENU hMenuPopup = GetSubMenu(hMenu,4);

            POINT pt;
            int iCmd;

            SetForegroundWindow(hwnd);

            GetCursorPos(&pt);
            SetMenuDefaultItem(hMenuPopup,IDM_TRAY_RESTORE,FALSE);
            iCmd = TrackPopupMenu(hMenuPopup,
                    TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
                    pt.x,pt.y,0,hwnd,NULL);

            PostMessage(hwnd,WM_NULL,0,0);

            DestroyMenu(hMenu);

            if (iCmd == IDM_TRAY_RESTORE) {
              ShowNotifyIcon(hwnd,FALSE);
              RestoreWndFromTray(hwnd);
              ShowOwnedPopups(hwnd,TRUE);
            }

            else if (iCmd == IDM_TRAY_EXIT) {
              ShowNotifyIcon(hwnd,FALSE);
              PostMessage(hwnd,WM_CLOSE,0,0);
            }
          }
          return TRUE;

        case WM_LBUTTONUP:
          ShowNotifyIcon(hwnd,FALSE);
          RestoreWndFromTray(hwnd);
          ShowOwnedPopups(hwnd,TRUE);
          return TRUE;
      }
      break;


    default:

      if (umsg == msgTaskbarCreated) {
        if (!IsWindowVisible(hwnd))
          ShowNotifyIcon(hwnd,TRUE);
        return(0);
      }

      return DefWindowProc(hwnd,umsg,wParam,lParam);

  }

  return(0);

}


//=============================================================================
//
//  MsgCreate() - Handles WM_CREATE
//
//
LRESULT MsgCreate(HWND hwnd,WPARAM wParam,LPARAM lParam)
{

  HWND hwndTT;
  TOOLINFO ti;

  LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

  HMENU hmenu;
  WCHAR  tch[64];
  MENUITEMINFO mii;

  HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
  DWORD dwDriveBoxStyle = WS_DRIVEBOX;

  hwndDirList = CreateWindowEx(
                  WS_EX_CLIENTEDGE,
                  WC_LISTVIEW,
                  NULL,
                  WS_DIRLIST,
                  0,0,0,0,
                  hwnd,
                  (HMENU)IDC_DIRLIST,
                  hInstance,
                  NULL);

  if (IsVista() && PrivateIsAppThemed()) {
    SetWindowLongPtr(hwndDirList,GWL_EXSTYLE,GetWindowLongPtr(hwndDirList,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(hwndDirList,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
  }

  if (Settings.bShowDriveBox)
    dwDriveBoxStyle |= WS_VISIBLE;

  hwndDriveBox = CreateWindowEx(
                   0,
                   WC_COMBOBOXEX,
                   NULL,
                   dwDriveBoxStyle,
                   0,0,0,GetSystemMetrics(SM_CYFULLSCREEN),
                   hwnd,
                   (HMENU)IDC_DRIVEBOX,
                   hInstance,
                   NULL);

  // Create Toolbar and Statusbar
  CreateBars(hwnd,hInstance);

  // Window Initialization
  // DriveBox
  DriveBox_Init(hwndDriveBox);
  SendMessage(hwndDriveBox,CB_SETEXTENDEDUI,TRUE,0);
  // DirList
  ListView_SetExtendedListViewStyle(hwndDirList,LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
  ListView_InsertColumn(hwndDirList,0,&lvc);
  DirList_Init(hwndDirList,NULL);
  if (Settings.bTrackSelect)
    ListView_SetExtendedListViewStyleEx(hwndDirList,
      LVS_EX_TRACKSELECT|LVS_EX_ONECLICKACTIVATE,
      LVS_EX_TRACKSELECT|LVS_EX_ONECLICKACTIVATE);
  if (Settings.bFullRowSelect) {
    ListView_SetExtendedListViewStyleEx(hwndDirList,
      LVS_EX_FULLROWSELECT,
      LVS_EX_FULLROWSELECT);
    if (IsVista())
      SetTheme(hwndDirList,L"Explorer");
  }
  ListView_SetHoverTime(hwndDirList,10);
  // Drag & Drop
  DragAcceptFiles(hwnd,TRUE);
  // History
  History_Init(&g_mHistory);
  History_UpdateToolbar(&g_mHistory,hwndToolbar,
        IDT_HISTORY_BACK,IDT_HISTORY_FORWARD);
  // ToolTip with Current Directory
  ZeroMemory(&ti,sizeof(TOOLINFO));
  ti.cbSize = sizeof(TOOLINFO);
  ti.uFlags = TTF_IDISHWND;
  ti.hwnd = hwnd;
  ti.uId = (UINT_PTR)hwnd;
  ti.lpszText = LPSTR_TEXTCALLBACK;

  hwndTT = (HWND)SendMessage(hwndToolbar,TB_GETTOOLTIPS,0,0);
  SendMessage(hwndTT,TTM_ADDTOOL,0,(LPARAM)&ti);

  // System Menu
  hmenu = GetSystemMenu(hwnd,FALSE);

  // Remove unwanted items
  DeleteMenu(hmenu,SC_RESTORE, MF_BYCOMMAND);
  DeleteMenu(hmenu,SC_MAXIMIZE,MF_BYCOMMAND);

  // Mofify the L"Minimize" item
  mii.cbSize = sizeof(MENUITEMINFO);
  mii.fMask = MIIM_ID;
  GetMenuItemInfo(hmenu,SC_MINIMIZE,FALSE,&mii);
  mii.wID = SC_MINIMIZE | 0x02;
  SetMenuItemInfo(hmenu,SC_MINIMIZE,FALSE,&mii);

  // Add specific items
  GetLngString(SC_ALWAYSONTOP,tch,COUNTOF(tch));
  InsertMenu(hmenu,SC_MOVE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_ALWAYSONTOP,tch);
  GetLngString(SC_ABOUT,tch,COUNTOF(tch));
  InsertMenu(hmenu,SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_ABOUT,tch);
  InsertMenu(hmenu,SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

  UNUSED(wParam);

  return(0);
}


//=============================================================================
//
//  _LoadBitmapFile()
//
static HBITMAP _LoadBitmapFile(LPCWSTR path)
{
  WCHAR szTmp[MAX_PATH];
  if (PathIsRelative(path)) {
    GetModuleFileName(NULL, szTmp, COUNTOF(szTmp));
    PathRemoveFileSpec(szTmp);
    PathAppend(szTmp, path);
    path = szTmp;
  }

  if (!PathIsExistingFile(path)) {
    return NULL;
  }
  HBITMAP const hbmp = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
  return hbmp;
}


//=============================================================================
//
//  CreateBars() - Create Toolbar and Statusbar
//
//
void CreateBars(HWND hwnd,HINSTANCE hInstance)
{

  REBARINFO rbi;
  REBARBANDINFO rbBand;
  RECT rc;

  BITMAP bmp;
  HBITMAP hbmp, hbmpCopy = NULL;
  HIMAGELIST himl;
  BOOL bExternalBitmap = FALSE;

  DWORD dwToolbarStyle   = WS_TOOLBAR | TBSTYLE_FLAT | CCS_ADJUSTABLE;
  DWORD dwStatusbarStyle = WS_CHILD | WS_CLIPSIBLINGS;
  DWORD dwReBarStyle = WS_REBAR;

  BOOL bIsAppThemed = PrivateIsAppThemed();

  int i,n;
  WCHAR tchDesc[256];
  WCHAR tchIndex[256];

  if (Settings.bShowToolbar)
    dwReBarStyle |= WS_VISIBLE;

  hwndToolbar = CreateWindowEx(0,TOOLBARCLASSNAME,NULL,dwToolbarStyle,
                               0,0,0,0,hwnd,(HMENU)IDC_TOOLBAR,hInstance,NULL);

  SendMessage(hwndToolbar,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

  // Add normal Toolbar Bitmap
  hbmp = NULL;
  if (StrIsNotEmpty(Settings.tchToolbarBitmap))
  {
    hbmp = _LoadBitmapFile(Settings.tchToolbarBitmap);
  }
  if (hbmp) {
    bExternalBitmap = TRUE;
  }
  else {
    hbmp = LoadImage(hInstance,MAKEINTRESOURCE(IDR_MAINWND),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
    hbmpCopy = CopyImage(hbmp,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
  }
  GetObject(hbmp,sizeof(BITMAP),&bmp);
  if (!IsXP()) {
    BitmapMergeAlpha(hbmp, GetSysColor(COLOR_3DFACE));
  }
  himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
  ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
  DeleteObject(hbmp);
  SendMessage(hwndToolbar,TB_SETIMAGELIST,0,(LPARAM)himl);

  // Optionally add hot Toolbar Bitmap
  hbmp = NULL;
  if (StrIsNotEmpty(Settings.tchToolbarBitmapHot))
  {
    hbmp = _LoadBitmapFile(Settings.tchToolbarBitmapHot);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(hwndToolbar,TB_SETHOTIMAGELIST,0,(LPARAM)himl);
    }
  }

  // Optionally add disabled Toolbar Bitmap
  hbmp = NULL;
  if (StrIsNotEmpty(Settings.tchToolbarBitmapDisabled))
  {
    hbmp = _LoadBitmapFile(Settings.tchToolbarBitmapDisabled);
    if (hbmp)
    {
      GetObject(hbmp,sizeof(BITMAP),&bmp);
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
      DeleteObject(hbmp);
      SendMessage(hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
      bExternalBitmap = TRUE;
    }
  }

  if (!bExternalBitmap) {
    BOOL fProcessed = FALSE;
    if (flagToolbarLook == 1)
      fProcessed = BitmapAlphaBlend(hbmpCopy,GetSysColor(COLOR_3DFACE),0x60);
    else if (flagToolbarLook == 2 || (!IsXP() && flagToolbarLook == 0))
      fProcessed = BitmapGrayScale(hbmpCopy);
    if (fProcessed && !IsXP())
      BitmapMergeAlpha(hbmpCopy,GetSysColor(COLOR_3DFACE));
    if (fProcessed) {
      himl = ImageList_Create(bmp.bmWidth/NUMTOOLBITMAPS,bmp.bmHeight,ILC_COLOR32|ILC_MASK,0,0);
      ImageList_AddMasked(himl,hbmpCopy,CLR_DEFAULT);
      SendMessage(hwndToolbar,TB_SETDISABLEDIMAGELIST,0,(LPARAM)himl);
    }
  }
  if (hbmpCopy)
    DeleteObject(hbmpCopy);

  // Load toolbar labels
  __try {
    LoadIniFileCache(g_wchIniFile);
    const WCHAR* const ToolbarLabels_Section = L"Toolbar Labels";

    n = 0;
    for (i = 0; i < COUNTOF(tbbMainWnd); i++) {

      if (tbbMainWnd[i].fsStyle == TBSTYLE_SEP)
        continue;
      else
        n++;

      wsprintf(tchIndex, L"%02i", n);

      if (IniSectionGetString(ToolbarLabels_Section, tchIndex, L"", tchDesc, COUNTOF(tchDesc)) &&
        lstrcmpi(tchDesc, L"(none)") != 0) {

        tbbMainWnd[i].iString = SendMessage(hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc);
        tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
      }

      else if ((n == 5 || n == 8) && lstrcmpi(tchDesc, L"(none)") != 0) {

        GetLngString(42000 + n, tchDesc, COUNTOF(tchDesc));
        tbbMainWnd[i].iString = SendMessage(hwndToolbar, TB_ADDSTRING, 0, (LPARAM)tchDesc);
        tbbMainWnd[i].fsStyle |= BTNS_AUTOSIZE | BTNS_SHOWTEXT;
      }

      else
        tbbMainWnd[i].fsStyle &= ~(BTNS_AUTOSIZE | BTNS_SHOWTEXT);
    }
  }
  __finally {
    ResetIniFileCache();
  }

  SendMessage(hwndToolbar,TB_SETEXTENDEDSTYLE,0,
    SendMessage(hwndToolbar,TB_GETEXTENDEDSTYLE,0,0) | TBSTYLE_EX_MIXEDBUTTONS);

  SendMessage(hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)tbbMainWnd);
  //SendMessage(hwndToolbar,TB_SAVERESTORE,FALSE,(LPARAM)lptbsp);
  if (Toolbar_SetButtons(hwndToolbar,IDT_HISTORY_BACK, Settings.tchToolbarButtons,tbbMainWnd,COUNTOF(tbbMainWnd)) == 0)
    SendMessage(hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)tbbMainWnd);
  SendMessage(hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
  //SendMessage(hwndToolbar,TB_SETINDENT,2,0);

  if (Settings.bShowStatusbar)
    dwStatusbarStyle |= WS_VISIBLE;

  hwndStatus = CreateStatusWindow(dwStatusbarStyle,NULL,hwnd,IDC_STATUSBAR);

  // Create ReBar and add Toolbar
  hwndReBar = CreateWindowEx(WS_EX_TOOLWINDOW,REBARCLASSNAME,NULL,dwReBarStyle,
                             0,0,0,0,hwnd,(HMENU)IDC_REBAR,hInstance,NULL);

  rbi.cbSize = sizeof(REBARINFO);
  rbi.fMask  = 0;
  rbi.himl   = (HIMAGELIST)NULL;
  SendMessage(hwndReBar,RB_SETBARINFO,0,(LPARAM)&rbi);

  rbBand.cbSize  = sizeof(REBARBANDINFO);
  rbBand.fMask   = /*RBBIM_COLORS | RBBIM_TEXT | RBBIM_BACKGROUND | */
                   RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE /*| RBBIM_SIZE*/;
  rbBand.fStyle  = /*RBBS_CHILDEDGE | *//*RBBS_BREAK |*/ RBBS_FIXEDSIZE /*| RBBS_GRIPPERALWAYS*/;
  if (bIsAppThemed)
    rbBand.fStyle |= RBBS_CHILDEDGE;
  rbBand.hbmBack = NULL;
  rbBand.lpText     = L"Toolbar";
  rbBand.hwndChild  = hwndToolbar;
  rbBand.cxMinChild = (rc.right - rc.left) * COUNTOF(tbbMainWnd);
  rbBand.cyMinChild = (rc.bottom - rc.top) + 2 * rc.top;
  rbBand.cx         = 0;
  SendMessage(hwndReBar,RB_INSERTBAND,(WPARAM)-1,(LPARAM)&rbBand);

  SetWindowPos(hwndReBar,NULL,0,0,0,0,SWP_NOZORDER);
  GetWindowRect(hwndReBar,&rc);
  cyReBar = rc.bottom - rc.top;

  cyReBarFrame = bIsAppThemed ? 0 : 2;
  cyDriveBoxFrame = (bIsAppThemed && IsVista()) ? 0 : 2;

}


//=============================================================================
//
//  MsgThemeChanged() - Handles WM_THEMECHANGED
//
//
void MsgThemeChanged(HWND hwnd,WPARAM wParam,LPARAM lParam)
{
  RECT rc;
  WCHAR chStatus[255];
  HINSTANCE hInstance = (HINSTANCE)(INT_PTR)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);

  BOOL bIsAppThemed = PrivateIsAppThemed();

  if (IsVista() && bIsAppThemed) {
    SetWindowLongPtr(hwndDirList,GWL_EXSTYLE,GetWindowLongPtr(hwndDirList,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
    SetWindowPos(hwndDirList,NULL,0,0,0,0,SWP_NOZORDER|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);
    if (Settings.bFullRowSelect)
      SetTheme(hwndDirList,L"Explorer");
    else
      SetTheme(hwndDirList,L"Listview");
  }
  else {
    SetWindowLongPtr(hwndDirList,GWL_EXSTYLE,WS_EX_CLIENTEDGE|GetWindowLongPtr(hwndDirList,GWL_EXSTYLE));
    SetWindowPos(hwndDirList,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
    if (bIsAppThemed)
      SetTheme(hwndDirList,L"Listview");
  }

  // recreate toolbar and statusbar
  SendMessage(hwndStatus,SB_GETTEXT,ID_FILEINFO,(LPARAM)chStatus);

  // recreate toolbar and statusbar
  Toolbar_GetButtons(hwndToolbar,IDT_HISTORY_BACK, Settings.tchToolbarButtons,COUNTOF(Settings.tchToolbarButtons));

  DestroyWindow(hwndToolbar);
  DestroyWindow(hwndReBar);
  DestroyWindow(hwndStatus);
  CreateBars(hwnd,hInstance);

  GetClientRect(hwnd,&rc);
  SendMessage(hwnd,WM_SIZE,SIZE_RESTORED,MAKELONG(rc.right,rc.bottom));

  StatusSetText(hwndStatus,ID_FILEINFO,chStatus);

  UNUSED(hwnd);
  UNUSED(wParam);
  UNUSED(lParam);

}


//=============================================================================
//
//  MsgSize() - Handles WM_SIZE
//
//
void MsgSize(HWND hwnd,WPARAM wParam,LPARAM lParam)
{

  RECT rc;
  int  x,y,cx,cy;
  HDWP hdwp;
  int  aWidth[1];

  if (wParam == SIZE_MINIMIZED)
    return;

  x  = 0;
  y  = 0;

  cx = LOWORD(lParam);
  cy = HIWORD(lParam);

  if (Settings.bShowToolbar)
  {
/*  SendMessage(hwndToolbar,WM_SIZE,0,0);
    GetWindowRect(hwndToolbar,&rc);
    y = (rc.bottom - rc.top);
    cy -= (rc.bottom - rc.top);*/

    //SendMessage(hwndToolbar,TB_GETITEMRECT,0,(LPARAM)&rc);
    SetWindowPos(hwndReBar,NULL,0,0,LOWORD(lParam),cyReBar,SWP_NOZORDER);
    // the ReBar automatically sets the correct height
    // calling SetWindowPos() with the height of one toolbar button
    // causes the control not to temporarily use the whole client area
    // and prevents flickering

    GetWindowRect(hwndReBar,&rc);
    y = cyReBar + cyReBarFrame;    // define
    cy -= cyReBar + cyReBarFrame;  // border
  }

  if (Settings.bShowStatusbar)
  {
    SendMessage(hwndStatus,WM_SIZE,0,0);
    GetWindowRect(hwndStatus,&rc);
    cy -= (rc.bottom - rc.top);
  }

  hdwp = BeginDeferWindowPos(2);

  DeferWindowPos(hdwp,hwndDriveBox,NULL,x,y,cx,max(cy,100),
                SWP_NOZORDER | SWP_NOACTIVATE);

  if (Settings.bShowDriveBox) {
    GetWindowRect(hwndDriveBox,&rc);
    y += (rc.bottom - rc.top) + cyDriveBoxFrame;
    cy -= (rc.bottom - rc.top) + cyDriveBoxFrame;
  }

  DeferWindowPos(hdwp,hwndDirList,NULL,x,y,cx,cy,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  EndDeferWindowPos(hdwp);

  // Rebuild DirList Columns
  ListView_SetColumnWidth(hwndDirList,0,LVSCW_AUTOSIZE_USEHEADER);

  GetClientRect(hwndStatus,&rc);
  aWidth[0] = -1;
  SendMessage(hwndStatus,SB_SETPARTS,COUNTOF(aWidth),(LPARAM)aWidth);
  InvalidateRect(hwndStatus,NULL,TRUE);

  UNUSED(hwnd);
}


//=============================================================================
//
//  MsgInitMenu() - Handles WM_INITMENU
//
//
void MsgInitMenu(HWND hwnd,WPARAM wParam,LPARAM lParam)
{

  int i; // Helper
  DLITEM dli;
  HMENU hmenu = (HMENU)wParam;

  i = (ListView_GetSelectedCount(hwndDirList));
  dli.mask = DLI_TYPE;
  dli.ntype = DLE_NONE;
  DirList_GetItem(hwndDirList,-1,&dli);

  EnableCmd(hmenu,IDM_FILE_LAUNCH,(i && dli.ntype == DLE_FILE));
  EnableCmd(hmenu,IDM_FILE_QUICKVIEW,(i && dli.ntype == DLE_FILE) && bHasQuickview);
  EnableCmd(hmenu,IDM_FILE_OPENWITH,i);
  EnableCmd(hmenu,IDM_FILE_CREATELINK,i);
  EnableCmd(hmenu,IDM_FILE_SAVEAS,(i && dli.ntype == DLE_FILE));
  EnableCmd(hmenu,IDM_FILE_COPYMOVE,i);
  EnableCmd(hmenu,IDM_FILE_DELETE,i);
  EnableCmd(hmenu,IDM_FILE_RENAME,i);

  i = (SendMessage(hwndDriveBox,CB_GETCURSEL,0,0) != CB_ERR);
  EnableCmd(hmenu,IDM_FILE_DRIVEPROP,i);

  CheckCmd(hmenu,IDM_VIEW_FOLDERS,(Settings.dwFillMask & DL_FOLDERS));
  CheckCmd(hmenu,IDM_VIEW_FILES,(Settings.dwFillMask & DL_NONFOLDERS));
  CheckCmd(hmenu,IDM_VIEW_HIDDEN,(Settings.dwFillMask & DL_INCLHIDDEN));

  EnableCmd(hmenu,IDM_VIEW_FILTERALL,(lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter));

  CheckCmd(hmenu,IDM_VIEW_TOOLBAR, Settings.bShowToolbar);
  EnableCmd(hmenu,IDM_VIEW_CUSTOMIZETB, Settings.bShowToolbar);
  CheckCmd(hmenu,IDM_VIEW_STATUSBAR, Settings.bShowStatusbar);
  CheckCmd(hmenu,IDM_VIEW_DRIVEBOX, Settings.bShowDriveBox);

  CheckMenuRadioItem(hmenu,IDM_SORT_NAME,IDM_SORT_DATE,
    IDM_SORT_NAME + Settings.nSortFlags,MF_BYCOMMAND);

  CheckCmd(hmenu,IDM_SORT_REVERSE, Settings.fSortRev);

  CheckCmd(hmenu,SC_ALWAYSONTOP, Settings.bAlwaysOnTop);

  i = (StrIsNotEmpty(g_wchIniFile) || StrIsNotEmpty(g_wchIniFile2));
  EnableCmd(hmenu,IDM_VIEW_SAVESETTINGS,i);

  UNUSED(hwnd);
  UNUSED(wParam);
  UNUSED(lParam);
}


//=============================================================================
//
//  MsgCommand() - Handles WM_COMMAND
//
//
LRESULT MsgCommand(HWND hwnd,WPARAM wParam,LPARAM lParam)
{

  switch(LOWORD(wParam))
  {

    case IDC_DRIVEBOX:

      switch(HIWORD(wParam))
      {

        case CBN_SETFOCUS:
          nIdFocus = IDC_DRIVEBOX;
          break;

        case CBN_CLOSEUP:
          {
            WCHAR tch[64];

            if (DriveBox_GetSelDrive(hwndDriveBox,tch,COUNTOF(tch),TRUE)
                && !PathIsSameRoot(Settings.szCurDir,tch))
            {
              if (!ChangeDirectory(hwnd,tch,1))
              {
                ErrorMessage(2,IDS_ERR_CD);
                DriveBox_SelectDrive(hwndDriveBox, Settings.szCurDir);
              }
            }
            SetFocus(hwndDirList);
          }
          break;

      }
      break;


    case IDM_FILE_OPEN:
      {
        DLITEM dli = { DLI_ALL, L"", L"", DLE_NONE };

        DirList_GetItem(hwndDirList,-1,&dli);

        switch(dli.ntype)
        {

          case DLE_DIR:
            if (!ChangeDirectory(hwnd,dli.szFileName,1))
              ErrorMessage(2,IDS_ERR_CD);
            break;

          case DLE_FILE:
            BeginWaitCursor();

            if (!PathIsLnkFile(dli.szFileName))
              LaunchTarget(dli.szFileName,0);

            else // PathIsLinkFile()
            {
              DWORD dwAttr;
              WCHAR  tch[MAX_PATH];

              if (PathGetLnkPath(dli.szFileName,tch,COUNTOF(tch)))
              {
                ExpandEnvironmentStringsEx(tch,COUNTOF(tch));
                dwAttr = GetFileAttributes(tch);
                if ((dwAttr == (DWORD)(-1)) || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                  DisplayLnkFile(dli.szFileName);
                else
                  // Made sure link points to a file
                  LaunchTarget(tch,0);
              }
              else
                DisplayLnkFile(dli.szFileName);
            }

            EndWaitCursor();
            break;

        }
      }
      break;


    case IDM_FILE_OPENNEW:
      {
        DLITEM dli;
        dli.mask = DLI_ALL;

        DirList_GetItem(hwndDirList,-1,&dli);

        if (dli.ntype == DLE_FILE)
        {
          BeginWaitCursor();

          if (!PathIsLnkFile(dli.szFileName))
            LaunchTarget(dli.szFileName,1);

          else // PathIsLinkFile()
          {
            DWORD dwAttr;
            WCHAR  tch[MAX_PATH];

            if (PathGetLnkPath(dli.szFileName,tch,COUNTOF(tch)))
            {
              ExpandEnvironmentStringsEx(tch,COUNTOF(tch));
              dwAttr = GetFileAttributes(tch);
              if ((dwAttr == (DWORD)(-1)) || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                DisplayLnkFile(dli.szFileName);
              else
                // Made sure link points to a file
                LaunchTarget(tch,1);
            }
            else
              DisplayLnkFile(dli.szFileName);
          }

          EndWaitCursor();
        }

        else
          MessageBeep(0);
      }
      break;


    case IDM_FILE_RUN:
      RunDlg(hwnd);
      break;


    case IDM_FILE_LAUNCH:
      {
        DLITEM dli;

        if (!DirList_IsFileSelected(hwndDirList))
        {
          MessageBeep(0);
          return(0);
        }

        dli.mask = DLI_FILENAME;
        if (DirList_GetItem(hwndDirList,-1,&dli) == -1)
          break;

        SHELLEXECUTEINFO sei = { 0 };
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = dli.szFileName;
        sei.lpParameters = NULL;
        sei.lpDirectory = Settings.szCurDir;
        sei.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_QUICKVIEW:
      {
        DLITEM dli;
        WCHAR szParam[MAX_PATH] = L"";
        WCHAR szTmp[MAX_PATH];

        if (!DirList_IsFileSelected(hwndDirList)) {
          MessageBeep(0);
          return(0);
        }

        dli.mask = DLI_FILENAME;
        if (DirList_GetItem(hwndDirList,-1,&dli) == -1)
          break;

        if (PathIsLnkFile(dli.szFileName) &&
            PathGetLnkPath(dli.szFileName,szTmp,COUNTOF(szTmp)))
          GetShortPathName(szTmp,szTmp,COUNTOF(szTmp));

        else
          GetShortPathName(dli.szFileName,szTmp,COUNTOF(szTmp));

        if (StrIsNotEmpty(Settings.szQuickviewParams)) {
          StrCatBuff(szParam, Settings.szQuickviewParams,COUNTOF(szParam));
          StrCatBuff(szParam,L" ",COUNTOF(szParam));
        }
        StrCatBuff(szParam,szTmp,COUNTOF(szParam));

        SHELLEXECUTEINFO sei = { 0 };
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = Settings.szQuickview;
        sei.lpParameters = szParam;
        sei.lpDirectory = Settings.szCurDir;
        sei.nShow = SW_SHOWNORMAL;

        ShellExecuteEx(&sei);
      }
      break;


    case IDM_FILE_OPENWITH:
      {
        DLITEM dli;

        if (!ListView_GetSelectedCount(hwndDirList))
        {
          MessageBeep(0);
          return(0);
        }

        dli.mask = DLI_FILENAME;
        if (DirList_GetItem(hwndDirList,-1,&dli) == -1)
          break;

        OpenWithDlg(hwnd,&dli);
      }
      break;


    case IDM_FILE_GOTO:
      GotoDlg(hwnd);
      break;


    case IDM_FILE_NEW:
      {
        OPENFILENAME ofn;
        HANDLE       hFile;
        WCHAR szNewFile[MAX_PATH];
        WCHAR szFilter[128];
        WCHAR szTitle[32];
        WCHAR szPath[MAX_PATH];

        lstrcpy(szNewFile,L"");
        GetLngString(IDS_FILTER_ALL,szFilter,COUNTOF(szFilter));
        PrepareFilterStr(szFilter);
        GetLngString(IDS_NEWFILE,szTitle,COUNTOF(szTitle));

        ZeroMemory(&ofn,sizeof(OPENFILENAME));

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = szFilter;
        ofn.lpstrFile = szNewFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = szTitle;
        ofn.lpstrInitialDir = Settings.szCurDir;
        ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT |
                    OFN_NODEREFERENCELINKS | OFN_OVERWRITEPROMPT |
                    OFN_PATHMUSTEXIST;

        if (!GetSaveFileName(&ofn))
          break;

        hFile = CreateFile(szNewFile,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
          SHFILEINFO shfi;

          CloseHandle(hFile);

          // Extract dir from filename
          lstrcpy(szPath,szNewFile);
          PathRemoveFileSpec(szPath);
          SetCurrentDirectory(szPath);

          // Select new file
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
          SHGetFileInfo(szNewFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
          if (!DirList_SelectItem(hwndDirList,shfi.szDisplayName,szNewFile))
            ListView_EnsureVisible(hwndDirList,0,FALSE);
        }

        else
          ErrorMessage(2,IDS_ERR_NEW);
      }
      break;


    case IDM_FILE_NEWDIR: {

      WCHAR tchNewDir[MAX_PATH];

      if (NewDirDlg(hwnd,tchNewDir)) {
        if (!CreateDirectory(tchNewDir,NULL))
          ErrorMessage(2,IDS_ERR_NEWDIR);
        }

      } break;


    case IDM_FILE_CREATELINK:
      {
        DLITEM dli;
        WCHAR tchLinkDestination[MAX_PATH];

        dli.mask = DLI_FILENAME;
        if (DirList_GetItem(hwndDirList,-1,&dli) == -1)
          break;

        if (GetDirectory(hwnd,IDS_CREATELINK,tchLinkDestination,NULL,FALSE)) {
          if (!PathCreateLnk(tchLinkDestination,dli.szFileName))
            ErrorMessage(2,IDS_ERR_CREATELINK);
          }
      }
      break;


    case IDM_FILE_SAVEAS:
      {
        DLITEM dli;
        OPENFILENAME ofn;
        WCHAR szNewFile[MAX_PATH];
        WCHAR tch[MAX_PATH];
        WCHAR szFilter[128];
        BOOL bSuccess = FALSE;

        if (!DirList_IsFileSelected(hwndDirList))
        {
          MessageBeep(0);
          return(0);
        }

        dli.mask = DLI_ALL;
        if (DirList_GetItem(hwndDirList,-1,&dli) == -1)
          break;

        lstrcpy(szNewFile,dli.szFileName);
        GetLngString(IDS_FILTER_ALL,szFilter,COUNTOF(szFilter));
        PrepareFilterStr(szFilter);

        ZeroMemory(&ofn,sizeof(OPENFILENAME));

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = szFilter;
        ofn.lpstrFile = szNewFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT |
                    OFN_NODEREFERENCELINKS | OFN_OVERWRITEPROMPT |
                    OFN_PATHMUSTEXIST;

        if (!GetSaveFileName(&ofn))
          break;

        BeginWaitCursor();

        FormatLngStringW(tch,COUNTOF(tch),IDS_SAVEFILE,dli.szDisplayName);
        StatusSetText(hwndStatus,ID_MENUHELP,tch);
        StatusSetSimple(hwndStatus,TRUE);
        InvalidateRect(hwndStatus,NULL,TRUE);
        UpdateWindow(hwndStatus);

        bSuccess = CopyFile(dli.szFileName,szNewFile,FALSE);

        if (!bSuccess)
          ErrorMessage(2,IDS_ERR_SAVEAS1,dli.szDisplayName);

        if (bSuccess && Settings.bClearReadOnly)
        {
          DWORD dwFileAttributes = GetFileAttributes(szNewFile);
          if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
          {
            dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
            if (!SetFileAttributes(szNewFile,dwFileAttributes))
              ErrorMessage(2,IDS_ERR_SAVEAS2);
          }
        }
        StatusSetSimple(hwndStatus,FALSE);

        EndWaitCursor();
      }
      break;


    case IDM_FILE_COPYMOVE:
      if (ListView_GetSelectedCount(hwndDirList))
        CopyMoveDlg(hwnd,&wFuncCopyMove);
      else
        MessageBeep(0);
      break;


    case IDM_FILE_DELETE:
    case IDM_FILE_DELETE2:
    case IDM_FILE_DELETE3:
      {
        DLITEM dli;
        int    iItem;
        WCHAR   tch[512];
        SHFILEOPSTRUCT shfos;

        dli.mask = DLI_ALL;
        if ((iItem =DirList_GetItem(hwndDirList,-1,&dli)) == -1)
          break;

        ZeroMemory(&shfos,sizeof(SHFILEOPSTRUCT));
        ZeroMemory(tch,sizeof(WCHAR)*512);
        lstrcpy(tch,dli.szFileName);

        shfos.hwnd = hwnd;
        shfos.wFunc = FO_DELETE;
        shfos.pFrom = tch;
        shfos.pTo = NULL;
        if (Settings.fUseRecycleBin && (LOWORD(wParam) != IDM_FILE_DELETE2))
          shfos.fFlags = FOF_ALLOWUNDO;
        if (Settings.fNoConfirmDelete || LOWORD(wParam) == IDM_FILE_DELETE3)
          shfos.fFlags |= FOF_NOCONFIRMATION;

        SHFileOperation(&shfos);

        // Check if there are any changes in the directory, then update!
        if (WAIT_OBJECT_0 == WaitForSingleObject(hChangeHandle,0)) {

          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
          if (iItem > 0)
            iItem--;
          iItem = min(iItem,ListView_GetItemCount(hwndDirList) - 1);
          ListView_SetItemState(hwndDirList,iItem,LVIS_FOCUSED,LVIS_FOCUSED);
          ListView_EnsureVisible(hwndDirList,iItem,FALSE);

          FindNextChangeNotification(hChangeHandle);
        }
      }
      break;


    case IDM_FILE_RENAME:
      if (ListView_GetSelectedCount(hwndDirList))
        RenameFileDlg(hwnd);
      else
        MessageBeep(0);
      break;


    case IDM_FILE_PROPERTIES:
      if (!ListView_GetSelectedCount(hwndDirList))
        MessageBeep(0);
      else
        DirList_PropertyDlg(hwndDirList,-1);
      break;


    case IDM_FILE_CHANGEDIR:
      {

        WCHAR tch[MAX_PATH];

        if (GetDirectory(hwnd,IDS_GETDIRECTORY,tch,NULL,FALSE))
        {

          if (!ChangeDirectory(hwnd,tch,1))
            ErrorMessage(2,IDS_ERR_CD);

        }

      }
      break;


    case IDM_FILE_DRIVEPROP:
      DriveBox_PropertyDlg(hwndDriveBox);
      break;


    case IDM_VIEW_NEWWINDOW:
      {
        WCHAR szModuleName[MAX_PATH];
        WCHAR szParameters[MAX_PATH+64];

        MONITORINFO mi;
        HMONITOR hMonitor;
        WINDOWPLACEMENT wndpl;
        int x,y,cx,cy;
        WCHAR tch[64];

        GetModuleFileName(NULL,szModuleName,COUNTOF(szModuleName));

        lstrcpy(szParameters, Settings.szCurDir);
        PathQuoteSpaces(szParameters);

        lstrcat(szParameters,L" -f");
        if (StrIsNotEmpty(g_wchIniFile)) {
          lstrcat(szParameters,L" \"");
          lstrcat(szParameters,g_wchIniFile);
          lstrcat(szParameters,L"\"");
        }
        else
          lstrcat(szParameters,L"0");

        lstrcat(szParameters,L" -n");

        wndpl.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd,&wndpl);

        hMonitor = MonitorFromRect(&wndpl.rcNormalPosition,MONITOR_DEFAULTTONEAREST);
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(hMonitor,&mi);

        // offset new window position +10/+10
        x = wndpl.rcNormalPosition.left + 10;
        y = wndpl.rcNormalPosition.top  + 10;
        cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;

        // check if window fits monitor
        if ((x + cx) > mi.rcWork.right || (y + cy) > mi.rcWork.bottom) {
          x = mi.rcMonitor.left;
          y = mi.rcMonitor.top;
        }

        wsprintf(tch,L" -p %i,%i,%i,%i",x,y,cx,cy);
        lstrcat(szParameters,tch);

        ShellExecute(hwnd,NULL,szModuleName,szParameters,NULL,SW_SHOWNORMAL);
      }
      break;


    case IDM_VIEW_FOLDERS:
      if (Settings.dwFillMask & DL_FOLDERS)
        Settings.dwFillMask &= (~DL_FOLDERS);
      else
        Settings.dwFillMask |= DL_FOLDERS;
      SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
      ListView_EnsureVisible(hwndDirList,0,FALSE); // not done by update
      break;


    case IDM_VIEW_FILES:
      if (Settings.dwFillMask & DL_NONFOLDERS)
        Settings.dwFillMask &= (~DL_NONFOLDERS);
      else
        Settings.dwFillMask |= DL_NONFOLDERS;
      SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
      ListView_EnsureVisible(hwndDirList,0,FALSE); // not done by update
      break;


    case IDM_VIEW_HIDDEN:
      if (Settings.dwFillMask & DL_INCLHIDDEN)
        Settings.dwFillMask &= (~DL_INCLHIDDEN);
      else
        Settings.dwFillMask |= DL_INCLHIDDEN;
      SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
      ListView_EnsureVisible(hwndDirList,0,FALSE); // not done by update
      break;


    case IDM_VIEW_FILTER:
      if (GetFilterDlg(hwnd)) {
        // Store information about currently selected item
        DLITEM dli;
        dli.mask  = DLI_ALL;
        dli.ntype = DLE_NONE;
        DirList_GetItem(hwndDirList,-1,&dli);

        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);

        if (dli.ntype != DLE_NONE) {
          if (!DirList_SelectItem(hwndDirList,dli.szDisplayName,dli.szFileName))
            ListView_EnsureVisible(hwndDirList,0,FALSE);
          }
        }
        Toolbar_SetButtonImage(hwndToolbar,IDT_VIEW_FILTER,
          (lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter) ? TBFILTERBMP : TBFILTERBMP+1);
      break;


    case IDM_VIEW_FILTERALL:
      if (lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter) {
        DLITEM dli;

        lstrcpy(Settings.tchFilter,L"*.*");
        Settings.bNegFilter = FALSE;

        // Store information about currently selected item
        dli.mask  = DLI_ALL;
        dli.ntype = DLE_NONE;
        DirList_GetItem(hwndDirList,-1,&dli);

        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);

        if (dli.ntype != DLE_NONE) {
          if (!DirList_SelectItem(hwndDirList,dli.szDisplayName,dli.szFileName))
            ListView_EnsureVisible(hwndDirList,0,FALSE);
          }
        }
        Toolbar_SetButtonImage(hwndToolbar,IDT_VIEW_FILTER,TBFILTERBMP+1);
      break;


    case IDM_VIEW_UPDATE:
      ChangeDirectory(hwnd,NULL,1);
      break;


    case IDM_VIEW_FAVORITES:
      // Goto Favorites Directory
      DisplayPath(Settings.g_tchFavoritesDir,IDS_ERR_FAVORITES);
      break;


    case IDM_VIEW_EDITFAVORITES:
      {
        SHELLEXECUTEINFO sei = { 0 };
        sei.cbSize = sizeof(SHELLEXECUTEINFO);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = Settings.g_tchFavoritesDir;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;

        // Run favorites directory
        ShellExecuteEx(&sei);
      }
      break;


    case IDM_VIEW_TOOLBAR:
      if (Settings.bShowToolbar) {
        ShowWindow(hwndReBar,SW_HIDE);
        Settings.bShowToolbar = 0;
      }
      else {
        ShowWindow(hwndReBar,SW_SHOW);
        Settings.bShowToolbar = 1;
      }
      SendWMSize(hwnd);
      break;


    case IDM_VIEW_CUSTOMIZETB:
      SendMessage(hwndToolbar,TB_CUSTOMIZE,0,0);
      break;


    case IDM_VIEW_STATUSBAR:
      if (Settings.bShowStatusbar) {
        ShowWindow(hwndStatus,SW_HIDE);
        Settings.bShowStatusbar = 0;
      }
      else {
        ShowWindow(hwndStatus,SW_SHOW);
        Settings.bShowStatusbar = 1;
      }
      SendWMSize(hwnd);
      break;


    case IDM_VIEW_DRIVEBOX:
      if (Settings.bShowDriveBox) {
        ShowWindow(hwndDriveBox,SW_HIDE);
        Settings.bShowDriveBox = 0;
        if (GetDlgCtrlID(GetFocus()) == IDC_DRIVEBOX)
          SetFocus(hwndDirList);
        }
      else {
        ShowWindow(hwndDriveBox,SW_SHOW);
        Settings.bShowDriveBox = 1;
      }
      SendWMSize(hwnd);
      break;


    case IDM_VIEW_SAVESETTINGS:
      {
        BOOL bCreateFailure = FALSE;

        if (StrIsEmpty(g_wchIniFile)) {
          if (StrIsNotEmpty(g_wchIniFile2)) {
            lstrcpy(g_wchIniFile, g_wchIniFile2);
            if (CreateIniFile()) {
              lstrcpy(g_wchIniFile2,L"");
            }
            else {
              lstrcpy(g_wchIniFile, L""); // reset
              bCreateFailure = TRUE;
            }
          }
          else
            break;
        }

        if (!bCreateFailure) 
        {
          if (IniFileSetString(g_wchIniFile, L"Settings", L"WriteTest", L"ok")) 
          {
            // prepare save
            Toolbar_GetButtons(hwndToolbar, IDT_HISTORY_BACK, Settings.tchToolbarButtons, COUNTOF(Settings.tchToolbarButtons));

            BeginWaitCursor();
            SaveSettings(TRUE);
            EndWaitCursor();
            ErrorMessage(0,IDS_SAVESETTINGS);
          }
          else {
            ErrorMessage(2, IDS_ERR_INIWRITE);
          }
        }
        else {
          ErrorMessage(2, IDS_ERR_INICREATE);
        }
      }
      break;


    case IDM_VIEW_FINDTARGET:
      ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_FINDTARGET),hwnd,FindTargetDlgProc,(LPARAM)NULL);
      break;


    case IDM_VIEW_OPTIONS:
      OptionsPropSheet(hwnd, g_hLngResContainer);
      bHasQuickview = PathIsExistingFile(Settings.szQuickview);
      break;


    case IDM_SORT_NAME:
      Settings.nSortFlags = DS_NAME;
      DirList_Sort(hwndDirList, Settings.nSortFlags, Settings.fSortRev);
      break;


    case IDM_SORT_SIZE:
      Settings.nSortFlags = DS_SIZE;
      DirList_Sort(hwndDirList, Settings.nSortFlags, Settings.fSortRev);
      break;


    case IDM_SORT_TYPE:
      Settings.nSortFlags = DS_TYPE;
      DirList_Sort(hwndDirList, Settings.nSortFlags, Settings.fSortRev);
      break;


    case IDM_SORT_DATE:
      Settings.nSortFlags = DS_LASTMOD;
      DirList_Sort(hwndDirList, Settings.nSortFlags, Settings.fSortRev);
      break;


    case IDM_SORT_REVERSE:
      Settings.fSortRev = !Settings.fSortRev;
      DirList_Sort(hwndDirList, Settings.nSortFlags, Settings.fSortRev);
      break;


    case IDM_POP_COPYNAME:
    {
      DLITEM  dli;
      HGLOBAL hData;
      LPWSTR   pData;

      dli.mask = DLI_FILENAME;
      DirList_GetItem(hwndDirList,-1,&dli);

      hData = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
                          sizeof(WCHAR)*(lstrlen(dli.szFileName) + 1));
      pData = GlobalLock(hData);
      lstrcpy(pData,dli.szFileName);
      GlobalUnlock(hData);

      if (OpenClipboard(hwnd))
      {
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT,hData);
        CloseClipboard();
      }
    }
    break;


    case ACC_ESCAPE:
      if (SendMessage(hwndDriveBox,CB_GETDROPPEDSTATE,0,0))
        SendMessage(hwndDriveBox,CB_SHOWDROPDOWN,0,0);
      else if (Settings.iEscFunction == 1)
        SendMessage(hwnd,WM_SYSCOMMAND,SC_MINIMIZE,0);
      else if (Settings.iEscFunction == 2)
        PostMessage(hwnd,WM_CLOSE,0,0);
      break;


    case ACC_NEXTCTL:
    case ACC_PREVCTL:
    {

      int nId = GetDlgCtrlID(GetFocus());

      if (LOWORD(wParam) == ACC_NEXTCTL)
      {
        if (++nId > IDC_DIRLIST)
          nId = IDC_DRIVEBOX;
      }
      else
      {
        if (--nId < IDC_DRIVEBOX)
          nId = IDC_DIRLIST;
      }

      if (nId == IDC_DRIVEBOX && !Settings.bShowDriveBox)
        nId = IDC_DIRLIST;

      SetFocus(GetDlgItem(hwnd,nId));

    }
    break;


    case ACC_TOGGLE_FOCUSEDIT:
      if (Settings.bFocusEdit)
        Settings.bFocusEdit = 0;
      else
        Settings.bFocusEdit = 1;
      break;


    case ACC_SWITCHTRANSPARENCY:
      Settings.g_bTransparentMode = !Settings.g_bTransparentMode;
      int const iAlphaPercent = IniFileGetInt(g_wchIniFile, L"Settings2", L"OpacityLevel", 75);
      SetWindowTransparentMode(hwndMain, Settings.g_bTransparentMode, clampi(iAlphaPercent, 0, 100));
      break;


    case ACC_GOTOTARGET:
      {
        DLITEM dli = { DLI_ALL, L"", L"", DLE_NONE };
        DirList_GetItem(hwndDirList,-1,&dli);

        if (dli.ntype == DLE_FILE) {

          if (PathIsLnkFile(dli.szFileName)) {

            WCHAR szFullPath[MAX_PATH];
            WCHAR szDir[MAX_PATH];
            WCHAR *p;

            //SetFocus(hwndDirList);
            if (PathGetLnkPath(dli.szFileName,szFullPath,COUNTOF(szFullPath)))
            {
              if (PathFileExists(szFullPath))
              {
                lstrcpy(szDir,szFullPath);
                p = StrRChr(szDir, NULL, L'\\');
                if (p)
                {
                  *(p+1) = 0;
                  if (!PathIsRoot(szDir))
                    *p = 0;
                  SetCurrentDirectory(szDir);
                  SendMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
                  if (!DirList_SelectItem(hwndDirList,NULL,szFullPath))
                    ListView_EnsureVisible(hwndDirList,0,FALSE);
      } } } } } }
      break;


    case ACC_SELECTTARGET:
      ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_FINDTARGET),hwnd,FindTargetDlgProc,(LPARAM)NULL);
      break;


    case ACC_FIRETARGET:
      LaunchTarget(L"",TRUE);
      break;


    case ACC_SNAPTOTARGET:
      SnapToTarget(hwnd);
      break;


    case ACC_DEFAULTWINPOS:
      SnapToDefaultPos(hwnd);
      break;


    case ACC_SELECTINIFILE:
      if (StrIsNotEmpty(g_wchIniFile)) {
        CreateIniFile();
        DisplayPath(g_wchIniFile,IDS_ERR_INIOPEN);
      }
      break;


    case IDT_HISTORY_BACK:
      if (History_CanBack(&g_mHistory))
      {
        WCHAR tch[MAX_PATH];
        History_Back(&g_mHistory,tch,COUNTOF(tch));
        if (!ChangeDirectory(hwnd,tch,0))
          ErrorMessage(2,IDS_ERR_CD);
      }
      else
        MessageBeep(0);
      History_UpdateToolbar(&g_mHistory,hwndToolbar,
        IDT_HISTORY_BACK,IDT_HISTORY_FORWARD);
      break;


    case IDT_HISTORY_FORWARD:
      if (History_CanForward(&g_mHistory))
      {
        WCHAR tch[MAX_PATH];
        History_Forward(&g_mHistory,tch,COUNTOF(tch));
        if (!ChangeDirectory(hwnd,tch,0))
          ErrorMessage(2,IDS_ERR_CD);
      }
      else
        MessageBeep(0);
      History_UpdateToolbar(&g_mHistory,hwndToolbar,
        IDT_HISTORY_BACK,IDT_HISTORY_FORWARD);
      break;


    case IDT_UPDIR:
    {
      if (!PathIsRoot(Settings.szCurDir))
      {
        if (!ChangeDirectory(hwnd,L"..",1))
          ErrorMessage(2,IDS_ERR_CD);
      }
      else
        MessageBeep(0);
    }
    break;


    case IDT_ROOT:
      {
        if (!PathIsRoot(Settings.szCurDir))
        {
          if (!ChangeDirectory(hwnd,L"\\",1))
            ErrorMessage(2,IDS_ERR_CD);
        }
        else
          MessageBeep(0);
      }
      break;


    case IDT_VIEW_FAVORITES:
      SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_FAVORITES,1),0);
      break;


    case IDT_FILE_NEXT:
      {
        DLITEM dli;
        WCHAR   tch[MAX_PATH];

        int i,d;
        int iItem = ListView_GetNextItem(hwndDirList,-1,LVNI_ALL | LVNI_FOCUSED);

        dli.ntype = DLE_NONE;
        dli.mask = DLI_TYPE | DLI_FILENAME;

        d = (ListView_GetSelectedCount(hwndDirList)) ? 1 : 0;

        for (i = iItem + d; DirList_GetItem(hwndDirList,i,&dli) != (-1); i++) {
          if (dli.ntype == DLE_FILE && !PathIsLnkToDirectory(dli.szFileName,tch,COUNTOF(tch)))
            break;
        }

        if (dli.ntype != DLE_FILE) {
          for (i = 0; i <= iItem; i++) {
            DirList_GetItem(hwndDirList,i,&dli);
            if (dli.ntype == DLE_FILE && !PathIsLnkToDirectory(dli.szFileName,tch,COUNTOF(tch)))
              break;
          }
        }

        if (dli.ntype == DLE_FILE) {
          ListView_SetItemState(hwndDirList,i,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
          ListView_EnsureVisible(hwndDirList,i,FALSE);
          ListView_Update(hwndDirList,i);
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPEN,1),0);
        }

        else
          MessageBeep(0);
      }
      break;


    case IDT_FILE_PREV:
      {
        DLITEM dli;
        WCHAR   tch[MAX_PATH];

        int i,d;
        int iItem = ListView_GetNextItem(hwndDirList,-1,LVNI_ALL | LVNI_FOCUSED);

        dli.ntype = DLE_NONE;
        dli.mask = DLI_TYPE | DLI_FILENAME;

        d = (ListView_GetSelectedCount(hwndDirList) || iItem == 0) ? 1 : 0;

        for (i = iItem - d; i > (-1); i--) {
          DirList_GetItem(hwndDirList,i,&dli);
          if (dli.ntype == DLE_FILE && !PathIsLnkToDirectory(dli.szFileName,tch,COUNTOF(tch)))
            break;
        }

        if (dli.ntype != DLE_FILE) {
          for (i = ListView_GetItemCount(hwndDirList) - 1; i >= iItem; i--) {
            DirList_GetItem(hwndDirList,i,&dli);
            if (dli.ntype == DLE_FILE && !PathIsLnkToDirectory(dli.szFileName,tch,COUNTOF(tch)))
              break;
          }
        }

        if (dli.ntype == DLE_FILE) {
          ListView_SetItemState(hwndDirList,i,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
          ListView_EnsureVisible(hwndDirList,i,FALSE);
          ListView_Update(hwndDirList,i);
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPEN,1),0);
        }

        else
          MessageBeep(0);
      }
      break;


    case IDT_FILE_RUN:
      SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_RUN,1),0);
      break;


    case IDT_FILE_QUICKVIEW:
      if (DirList_IsFileSelected(hwndDirList))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_QUICKVIEW,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_SAVEAS:
      if (DirList_IsFileSelected(hwndDirList))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_SAVEAS,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_COPYMOVE:
      if (ListView_GetSelectedCount(hwndDirList))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_COPYMOVE,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_FILE_DELETE:
      if (ListView_GetSelectedCount(hwndDirList)) {
        BOOL const fUseRecycleBin2 = Settings.fUseRecycleBin;
        Settings.fUseRecycleBin = 1;
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_DELETE,1),0);
        Settings.fUseRecycleBin = fUseRecycleBin2;
      }
      else
        MessageBeep(0);
      break;


    case IDT_FILE_DELETE2:
      if (ListView_GetSelectedCount(hwndDirList))
        SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_DELETE2,1),0);
      else
        MessageBeep(0);
      break;


    case IDT_VIEW_FILTER:
      SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_FILTER,1),0);
      break;

  }

  UNUSED(wParam);
  UNUSED(lParam);

  return(0);
}


//=============================================================================
//
//  MsgNotify() - Handles WM_NOTIFY
//
//
LRESULT MsgNotify(HWND hwnd,WPARAM wParam,LPARAM lParam)
{

  LPNMHDR pnmh = (LPNMHDR)lParam;

  switch(pnmh->idFrom)
  {

    case IDC_DIRLIST:

      switch(pnmh->code)
      {

        case NM_SETFOCUS:
          nIdFocus = IDC_DIRLIST;
          break;

        case LVN_GETDISPINFO:
          DirList_GetDispInfo(hwndDirList,lParam,flagNoFadeHidden);
          break;

        case LVN_DELETEITEM:
          DirList_DeleteItem(hwndDirList,lParam);
          break;

        case LVN_BEGINDRAG:
        case LVN_BEGINRDRAG:
          DirList_DoDragDrop(hwndDirList,lParam);
          break;

        case LVN_ITEMCHANGED:
          {

            WCHAR tch[64],tchnum[64];
            WCHAR tchsize[64],tchdate[64],tchtime[64],tchattr[64];
            HANDLE hFile;
            WIN32_FIND_DATA fd;
            DLITEM dli;
            LONGLONG isize;
            FILETIME   ft;
            SYSTEMTIME st;

            NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;

            if ((pnmlv->uNewState & LVIS_SELECTED|LVIS_FOCUSED) !=
                (pnmlv->uOldState & LVIS_SELECTED|LVIS_FOCUSED))
            {

              if ((pnmlv->uNewState & LVIS_SELECTED))
              {
                dli.mask  = DLI_FILENAME;
                dli.ntype = DLE_NONE;
                DirList_GetItem(hwndDirList,-1,&dli);
                DirList_GetItemEx(hwndDirList,-1,&fd);

                if (fd.nFileSizeLow >= MAXDWORD) {
                  if ((hFile = FindFirstFile(dli.szFileName,&fd)) != INVALID_HANDLE_VALUE)
                    FindClose(hFile);
                }

                isize = ( ((LONGLONG)fd.nFileSizeHigh) << 32 ) + fd.nFileSizeLow;
                StrFormatByteSize((LONGLONG)isize,tchsize,COUNTOF(tchsize));

                FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ft);
                FileTimeToSystemTime(&ft,&st);
                GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,tchdate,COUNTOF(tchdate));
                GetTimeFormat(LOCALE_USER_DEFAULT,0,&st,NULL,tchtime,COUNTOF(tchdate));

                lstrcpy(tchattr,(fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)?L"A":L"-");
                lstrcat(tchattr,(fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)?L"R":L"-");
                lstrcat(tchattr,(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)?L"H":L"-");
                lstrcat(tchattr,(fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)?L"S":L"-");

                wsprintf(tch,L"%s | %s %s | %s",tchsize,tchdate,tchtime,tchattr);
              }

              else
              {
                wsprintf(tchnum,L"%u",ListView_GetItemCount(hwndDirList));
                FormatNumberStr(tchnum);
                FormatLngStringW(tch,COUNTOF(tch),(lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter)?IDS_NUMFILES2:IDS_NUMFILES,tchnum);
              }

              StatusSetText(hwndStatus,ID_FILEINFO,tch);

            }
          }
          break;

        case NM_CLICK:

          if (Settings.bSingleClick && ListView_GetSelectedCount(hwndDirList))
          {
            if (IsKeyDown(VK_MENU))
              SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_PROPERTIES,1),0);
            else if (IsKeyDown(VK_SHIFT))
              SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPENNEW,1),0);
            else
              SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPEN,1),0);
          }
          break;

        case NM_DBLCLK:
        case NM_RETURN:

          if (IsKeyDown(VK_MENU))
            SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_PROPERTIES,1),0);
          else if (IsKeyDown(VK_SHIFT))
              SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPENNEW,1),0);
          else
            SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_FILE_OPEN,1),0);
          break;

      }
      break;


    case IDC_DRIVEBOX:

      switch(pnmh->code)
      {

        case CBEN_GETDISPINFO:
          DriveBox_GetDispInfo(hwndDriveBox,lParam);
          break;

        case CBEN_DELETEITEM:
          DriveBox_DeleteItem(hwndDriveBox,lParam);
          break;

      }
      break;


    case IDC_TOOLBAR:

      switch(pnmh->code)
      {

        case TBN_ENDADJUST:
          History_UpdateToolbar(&g_mHistory,hwndToolbar,
            IDT_HISTORY_BACK,IDT_HISTORY_FORWARD);
          Toolbar_SetButtonImage(hwndToolbar,IDT_VIEW_FILTER,
            (lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter) ? TBFILTERBMP : TBFILTERBMP+1);
          break;

        case TBN_QUERYDELETE:
        case TBN_QUERYINSERT:
          return TRUE;

        case TBN_GETBUTTONINFO:
          {
            if (((LPTBNOTIFY)lParam)->iItem < COUNTOF(tbbMainWnd))
            {
              WCHAR tch[256];
              GetLngString(tbbMainWnd[((LPTBNOTIFY)lParam)->iItem].idCommand,tch,COUNTOF(tch));
              lstrcpyn(((LPTBNOTIFY)lParam)->pszText,tch,((LPTBNOTIFY)lParam)->cchText);
              CopyMemory(&((LPTBNOTIFY)lParam)->tbButton,&tbbMainWnd[((LPTBNOTIFY)lParam)->iItem],sizeof(TBBUTTON));
              return TRUE;
            }
          }
          return FALSE;

        case TBN_RESET:
          {
            int i; int c = (int)SendMessage(hwndToolbar,TB_BUTTONCOUNT,0,0);
            for (i = 0; i < c; i++)
              SendMessage(hwndToolbar,TB_DELETEBUTTON,0,0);
            if (Toolbar_SetButtons(hwndToolbar,IDT_HISTORY_BACK,L"1 2 3 4 5 0 8",tbbMainWnd,COUNTOF(tbbMainWnd)) == 0)
              SendMessage(hwndToolbar,TB_ADDBUTTONS,NUMINITIALTOOLS,(LPARAM)tbbMainWnd);
            return(0);
          }

      }
      break;


    default:

      switch(pnmh->code)
      {

        case TTN_NEEDTEXT:
          {
            WCHAR tch[256];

            if (((LPTOOLTIPTEXT)lParam)->uFlags & TTF_IDISHWND)
            {
              PathCompactPathEx(((LPTOOLTIPTEXT)lParam)->szText, Settings.szCurDir,
                                COUNTOF(((LPTOOLTIPTEXT)lParam)->szText),0);
            }

            else
            {
              GetLngString((UINT)pnmh->idFrom,tch,COUNTOF(tch));
              lstrcpyn(((LPTOOLTIPTEXT)lParam)->szText,tch,80);
            }
          }
          break;

      }
      break;

  }

  UNUSED(wParam);

  return(0);
}


//=============================================================================
//
//  ChangeDirectory()
//
//
BOOL ChangeDirectory(HWND hwnd,LPCWSTR lpszNewDir,BOOL bUpdateHistory)
{

  BOOL fUpdate = FALSE;

  if (lpszNewDir && !SetCurrentDirectory(lpszNewDir))
    return FALSE;

  if (!lpszNewDir) // Update call
  {
    WCHAR szTest[MAX_PATH];
    WCHAR szWinDir[MAX_PATH];

    GetCurrentDirectory(COUNTOF(szTest),szTest);
    if (!PathFileExists(szTest))
    {
      GetWindowsDirectory(szWinDir,COUNTOF(szWinDir));
      SetCurrentDirectory(szWinDir);
      ErrorMessage(2,IDS_ERR_CD);
    }

    fUpdate = TRUE;
  }

  BeginWaitCursor();
  {

    WCHAR tch[256],tchnum[64];
    int cItems;

    int iTopItem = ListView_GetTopIndex(hwndDirList);

    GetCurrentDirectory(COUNTOF(Settings.szCurDir), Settings.szCurDir);

    SetWindowPathTitle(hwnd, Settings.szCurDir);

    if (lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter) {
      ListView_SetTextColor(hwndDirList,(Settings.bDefCrFilter) ? GetSysColor(COLOR_WINDOWTEXT) : Settings.crFilter);
      Toolbar_SetButtonImage(hwndToolbar,IDT_VIEW_FILTER,TBFILTERBMP);
    }
    else {
      ListView_SetTextColor(hwndDirList,(Settings.bDefCrNoFilt) ? GetSysColor(COLOR_WINDOWTEXT) : Settings.crNoFilt);
      Toolbar_SetButtonImage(hwndToolbar,IDT_VIEW_FILTER,TBFILTERBMP+1);
    }

    cItems = DirList_Fill(hwndDirList, Settings.szCurDir, Settings.dwFillMask, 
      Settings.tchFilter, Settings.bNegFilter,flagNoFadeHidden, Settings.nSortFlags, Settings.fSortRev);
    DirList_StartIconThread(hwndDirList);

    // Get long pathname
    DirList_GetLongPathName(hwndDirList, Settings.szCurDir);
    SetCurrentDirectory(Settings.szCurDir);

    if (cItems > 0)
      ListView_SetItemState(hwndDirList,0,LVIS_FOCUSED,LVIS_FOCUSED);

    // new directory -- scroll to 0,0
    if (!fUpdate)
      ListView_EnsureVisible(hwndDirList,0,FALSE);
    else {
      int iJump = min(iTopItem + ListView_GetCountPerPage(hwndDirList),cItems - 1);
      ListView_EnsureVisible(hwndDirList,iJump,TRUE);
      ListView_EnsureVisible(hwndDirList,iTopItem,TRUE);
    }

    // setup new change notification handle
    FindCloseChangeNotification(hChangeHandle);
    hChangeHandle = FindFirstChangeNotification(Settings.szCurDir,FALSE,
        FILE_NOTIFY_CHANGE_FILE_NAME  | \
        FILE_NOTIFY_CHANGE_DIR_NAME   | \
        FILE_NOTIFY_CHANGE_ATTRIBUTES | \
        FILE_NOTIFY_CHANGE_SIZE | \
        FILE_NOTIFY_CHANGE_LAST_WRITE);

    DriveBox_Fill(hwndDriveBox);
    DriveBox_SelectDrive(hwndDriveBox, Settings.szCurDir);

    wsprintf(tchnum,L"%u",cItems);
    FormatNumberStr(tchnum);
    FormatLngStringW(tch,COUNTOF(tch),(lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter)?IDS_NUMFILES2:IDS_NUMFILES,tchnum);
    StatusSetText(hwndStatus,ID_FILEINFO,tch);

    // Update History
    if (bUpdateHistory)
    {
      History_Add(&g_mHistory, Settings.szCurDir);
      History_UpdateToolbar(&g_mHistory,hwndToolbar,
        IDT_HISTORY_BACK,IDT_HISTORY_FORWARD);
    }

  }
  EndWaitCursor();

  return TRUE;

}


//=============================================================================
//
//  ParseCommandLine()
//
//
void ParseCommandLine()
{

  LPWSTR lpCmdLine = GetCommandLine();
  LPWSTR lp1,lp2;

  if (StrIsEmpty(lpCmdLine))
    return;

  // Good old console can also send args separated by Tabs
  StrTab2Space(lpCmdLine);

  lp1 = LocalAlloc(LPTR,sizeof(WCHAR)*(lstrlen(lpCmdLine) + 1));
  lp2 = LocalAlloc(LPTR,sizeof(WCHAR)*(lstrlen(lpCmdLine) + 1));

  // Start with 2nd argument
  ExtractFirstArgument(lpCmdLine,lp1,lp2);

  while (ExtractFirstArgument(lp2,lp1,lp2))
  {

    // options
    if ((*lp1 == L'/') || (*lp1 == L'-'))
    {

      StrTrim(lp1,L"-/");

      switch (*CharUpper((lp1)))
      {

        case L'N':
          flagNoReuseWindow = 1;
          break;

        case L'I':
          flagStartAsTrayIcon = 1;
          break;

        case L'G':
          flagGotoFavorites = 1;
          break;

        case L'F':
          if (*(lp1+1) == L'0' || *CharUpper(lp1+1) == L'O')
            lstrcpy(g_wchIniFile,L"*?");
          else if (ExtractFirstArgument(lp2,lp1,lp2)) {
            StrCpyN(g_wchIniFile,lp1,COUNTOF(g_wchIniFile));
            TrimStringW(g_wchIniFile);
            PathUnquoteSpaces(g_wchIniFile);
          }
          break;

        case L'P':
          if (*CharUpper(lp1+1) == L'D' || *CharUpper(lp1+1) == L'S') {
            flagPosParam = 1;
            Settings.wi.x = Settings.wi.y = Settings.wi.cx = Settings.wi.cy = CW_USEDEFAULT;
          }
          else if (ExtractFirstArgument(lp2,lp1,lp2)) {
            int itok =
              swscanf_s(lp1,L"%i,%i,%i,%i",&Settings.wi.x,&Settings.wi.y,&Settings.wi.cx,&Settings.wi.cy);
            if (itok == 4) { // scan successful
              flagPosParam = 1;
              if (Settings.wi.cx < 1) Settings.wi.cx = CW_USEDEFAULT;
              if (Settings.wi.cy < 1) Settings.wi.cy = CW_USEDEFAULT;
            }
          }
          break;

        case L'M':
          if (ExtractFirstArgument(lp2,lp1,lp2)) {
            if (lpFilterArg)
              GlobalFree(lpFilterArg);

            lpFilterArg = GlobalAlloc(GPTR,sizeof(WCHAR)*(lstrlen(lp1)+1));
            lstrcpy(lpFilterArg,lp1);
          }
          break;

        default:
          break;

      }

    }

    // pathname
    else
    {
      if (lpPathArg)
        GlobalFree(lpPathArg);

      lpPathArg = GlobalAlloc(GPTR,sizeof(WCHAR)*(MAX_PATH+2));
      lstrcpy(lpPathArg,lp1);
    }

  }

  LocalFree(lp1);
  LocalFree(lp2);

}


//=============================================================================
//
//  DisplayPath()
//
//
BOOL DisplayPath(LPCWSTR lpPath,UINT uIdError)
{

  DWORD dwAttr;
  WCHAR  szPath[MAX_PATH];
  WCHAR  szTmp[MAX_PATH];

  if (StrIsEmpty(lpPath))
    return FALSE;

  lstrcpy(szTmp,lpPath);
  ExpandEnvironmentStringsEx(szTmp,COUNTOF(szTmp));

  if (!SearchPathEx(szTmp, COUNTOF(szPath), szPath)) {
    lstrcpy(szPath, szTmp);
  }
  if (PathIsLnkFile(szPath)) {
    return DisplayLnkFile(szPath);
  }
  dwAttr = GetFileAttributes(szPath);

  if (dwAttr != INVALID_FILE_ATTRIBUTES)
  {
    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (!SetCurrentDirectory(szPath))
      {
        ErrorMessage(2,uIdError);
        return FALSE;
      }
      else
      {
        PostMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
        ListView_EnsureVisible(hwndDirList,0,FALSE);
        return TRUE;
      }
    }

    else // dwAttr & FILE_ATTRIBUTE_DIRECTORY
    {
      // szPath will be modified...
      lstrcpy(szTmp,szPath);

      SHFILEINFO shfi;
      ZeroMemory(&shfi, sizeof(SHFILEINFO));
      SHGetFileInfo(szPath,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);

      WCHAR *p = StrRChr(szPath, NULL, L'\\');
      if (p)
      {
        *(p+1) = 0;
        if (!PathIsRoot(szPath))
          *p = 0;
        SetCurrentDirectory(szPath);
      }

      SendMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);

      if (!DirList_SelectItem(hwndDirList,shfi.szDisplayName,szTmp))
        ListView_EnsureVisible(hwndDirList,0,FALSE);

      return TRUE;
    }
  }

  else // dwAttr != (DWORD)(-1)
  {
    ErrorMessage(2,uIdError);
    return FALSE;
  }

}


//=============================================================================
//
//  DisplayLnkFile()
//
//
BOOL DisplayLnkFile(LPCWSTR pszLnkFile)
{

  DWORD dwAttr;
  WCHAR  szPath[MAX_PATH];
  WCHAR  szTmp[MAX_PATH];

  if (!PathGetLnkPath(pszLnkFile,szTmp,COUNTOF(szTmp)))
  {
    // Select lnk-file if target is not available
    if (PathIsExistingFile(pszLnkFile))
    {
      SHFILEINFO shfi;

      lstrcpy(szTmp,pszLnkFile);
      PathRemoveFileSpec(szTmp);
      SetCurrentDirectory(szTmp);

      // Select new file
      SendMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
      SHGetFileInfo(pszLnkFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
      if (!DirList_SelectItem(hwndDirList,shfi.szDisplayName,pszLnkFile))
        ListView_EnsureVisible(hwndDirList,0,FALSE);
    }

    ErrorMessage(2,IDS_ERR_LNK_GETPATH);
    return FALSE;
  }

  ExpandEnvironmentStringsEx(szTmp,COUNTOF(szTmp));

  if (!SearchPathEx(szTmp, COUNTOF(szPath), szPath)) {
    lstrcpy(szPath, szTmp);
  }
  dwAttr = GetFileAttributes(szPath);

  if (dwAttr != INVALID_FILE_ATTRIBUTES)
  {
    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (!SetCurrentDirectory(szPath))
      {
        ErrorMessage(2,IDS_ERR_LNK_NOACCESS);
        return FALSE;
      }
      else
      {
        PostMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
        ListView_EnsureVisible(hwndDirList,0,FALSE);
        return TRUE;
      }
    }

    else // dwAttr & FILE_ATTRIBUTE_DIRECTORY
    {
      int  i;
      SHFILEINFO  shfi;
      LV_FINDINFO lvfi;

      // Current file is ShellLink, get dir and desc
      lstrcpy(szPath,pszLnkFile);

      SHGetFileInfo(szPath,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);

      WCHAR *p = StrRChr(szPath, NULL, L'\\');
      if (p)
      {
        *(p+1) = 0;
        if (!PathIsRoot(szPath))
          *p = 0;
        SetCurrentDirectory(szPath);
      }

      lvfi.flags = LVFI_STRING;
      lvfi.psz   = shfi.szDisplayName;

      SendMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
      i = ListView_FindItem(hwndDirList,-1,&lvfi);

      // found item that is currently displayed
      if (i != -1)
      {
        ListView_SetItemState(hwndDirList,i,LVIS_SELECTED|LVIS_FOCUSED,
                                            LVIS_SELECTED|LVIS_FOCUSED);
        ListView_EnsureVisible(hwndDirList,i,FALSE);
      }
      else
        ListView_EnsureVisible(hwndDirList,0,FALSE);

      return TRUE;
    }
  }

  // GetFileAttributes() failed
  else
  {
    // Select lnk-file if target is not available
    if (PathIsExistingFile(pszLnkFile))
    {
      SHFILEINFO shfi;

      lstrcpy(szTmp,pszLnkFile);
      PathRemoveFileSpec(szTmp);
      SetCurrentDirectory(szTmp);

      // Select new file
      SendMessage(hwndMain,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
      SHGetFileInfo(pszLnkFile,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
      if (!DirList_SelectItem(hwndDirList,shfi.szDisplayName,pszLnkFile))
        ListView_EnsureVisible(hwndDirList,0,FALSE);
    }

    ErrorMessage(2,IDS_ERR_LNK_NOACCESS);
    return FALSE;
  }

}


/******************************************************************************
*
* ActivatePrevInst()
*
* Tries to find and activate an already open MiniPath Window
*
*
******************************************************************************/
BOOL CALLBACK EnumWndProc(HWND hwnd,LPARAM lParam)
{

  BOOL bContinue = TRUE;
  WCHAR szClassName[64];

  if (GetClassName(hwnd,szClassName,COUNTOF(szClassName)))

    if (lstrcmpi(szClassName,WC_MINIPATH) == 0)
    {

      *(HWND*)lParam = hwnd;

      if (IsWindowEnabled(hwnd))
        bContinue = FALSE;

    }

  return(bContinue);

}

BOOL ActivatePrevInst()
{

  HWND hwnd = NULL;
  COPYDATASTRUCT cds;

  if (flagNoReuseWindow || flagStartAsTrayIcon)
    return(FALSE);

  EnumWindows(EnumWndProc,(LPARAM)&hwnd);

  // Found a window
  if (hwnd != NULL)
  {
    // Enabled
    if (IsWindowEnabled(hwnd))
    {
      if (IsIconic(hwnd))
        ShowWindowAsync(hwnd,SW_RESTORE);

      if (!IsWindowVisible(hwnd)) {
        SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONDBLCLK);
        SendMessage(hwnd,WM_TRAYMESSAGE,0,WM_LBUTTONUP);
      }

      SetForegroundWindow(hwnd);

      if (lpPathArg)
      {
        ExpandEnvironmentStringsEx(lpPathArg,(DWORD)GlobalSize(lpPathArg)/sizeof(WCHAR));

        if (PathIsRelative(lpPathArg)) {
          WCHAR tchTmp[MAX_PATH];
          GetCurrentDirectory(COUNTOF(tchTmp), tchTmp);
          PathAppend(tchTmp, lpPathArg);
          lstrcpy(lpPathArg, tchTmp);
        }
        cds.dwData = DATA_MINIPATH_PATHARG;
        cds.cbData = (DWORD)GlobalSize(lpPathArg);
        cds.lpData = lpPathArg;

        // Send lpPathArg to previous instance
        SendMessage(hwnd,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);

        GlobalFree(lpPathArg);
      }
      return(TRUE);
    }

    else // IsWindowEnabled()
    {
      WCHAR szBuf[256];
      WCHAR *c;

      // Prepare message
      GetLngString(IDS_ERR_PREVWINDISABLED,szBuf,COUNTOF(szBuf));
      c = StrChr(szBuf,L'\n');
      if (c)
      {
        *c = 0;
        c++;
      }

      // Ask...
      if (MessageBox(NULL,c,szBuf,MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND) == IDYES)
        return(FALSE);
      else
        return(TRUE);

    }
  }

  else
    return(FALSE);

}


//=============================================================================
//
//  ShowNotifyIcon()
//
//
void ShowNotifyIcon(HWND hwnd,BOOL bAdd)
{
  static HICON hIcon = NULL;
  if (!hIcon) {
    hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
  }
  NOTIFYICONDATA nid;
  ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 0;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_TRAYMESSAGE;
  nid.hIcon = hIcon;
  lstrcpy(nid.szTip,L"MiniPath");

  if(bAdd)
    Shell_NotifyIcon(NIM_ADD,&nid);
  else
    Shell_NotifyIcon(NIM_DELETE,&nid);
}


WCHAR szGlobalWndClass[256] = L"";

BOOL CALLBACK EnumWndProc2(HWND hwnd,LPARAM lParam)
{

  BOOL bContinue = TRUE;
  WCHAR szClassName[64];

  if (GetClassName(hwnd,szClassName,COUNTOF(szClassName)))

    if (lstrcmpi(szClassName,szGlobalWndClass) == 0)
    {

      *(HWND*)lParam = hwnd;

      if (IsWindowEnabled(hwnd))
        bContinue = FALSE;

    }

  return(bContinue);

}

void LoadTargetParamsOnce(void)
{
  static BOOL fLoaded;
  
  if (fLoaded)
    return;

  __try {
    LoadIniFileCache(g_wchIniFile);
    const WCHAR* const TargetApp_Section = L"Target Application";

    if (IniSectionGetInt(TargetApp_Section, L"UseTargetApplication", 0xFB) != 0xFB) {
      eUseTargetApplication = IniSectionGetInt(TargetApp_Section, L"UseTargetApplication", eUseTargetApplication);
      eTargetApplicationMode = IniSectionGetInt(TargetApp_Section, L"TargetApplicationMode", eTargetApplicationMode);
      IniSectionGetString(TargetApp_Section, L"TargetApplicationPath", szTargetApplication, szTargetApplication, COUNTOF(szTargetApplication));
      IniSectionGetString(TargetApp_Section, L"TargetApplicationParams", szTargetApplicationParams, szTargetApplicationParams, COUNTOF(szTargetApplicationParams));
      IniSectionGetString(TargetApp_Section, L"TargetApplicationWndClass", szTargetApplicationWndClass, szTargetApplicationWndClass, COUNTOF(szTargetApplicationWndClass));
      IniSectionGetString(TargetApp_Section, L"DDEMessage", szDDEMsg, szDDEMsg, COUNTOF(szDDEMsg));
      IniSectionGetString(TargetApp_Section, L"DDEApplication", szDDEApp, szDDEApp, COUNTOF(szDDEApp));
      IniSectionGetString(TargetApp_Section, L"DDETopic", szDDETopic, szDDETopic, COUNTOF(szDDETopic));
    }
    else if ((eUseTargetApplication != UTA_UNDEFINED) && StrIsEmpty(szTargetApplication)) {
      eUseTargetApplication = UTA_LAUNCH_TARGET;
      eTargetApplicationMode = TAM_SEND_DROP_MSG;
      lstrcpy(szTargetApplication, L"Notepad3.exe");
      lstrcpy(szTargetApplicationParams, L"");
      lstrcpy(szTargetApplicationWndClass, L"Notepad3");
      lstrcpy(szDDEMsg, L"");
      lstrcpy(szDDEApp, L"");
      lstrcpy(szDDETopic, L"");
    }
  }
  __finally {
    ResetIniFileCache();
  }
  fLoaded = TRUE;
}

//=============================================================================
//
//  LaunchTarget()
//
//  Launches the selected file in an existing target window
//  Runs target.exe if necessary
//
//
void LaunchTarget(LPCWSTR lpFileName,BOOL bOpenNew)
{

  HWND  hwnd  = NULL;
  HDROP hDrop = NULL;

  LoadTargetParamsOnce();

  if ((eUseTargetApplication == UTA_DEFINE_TARGET) || ((eUseTargetApplication != UTA_UNDEFINED) && StrIsEmpty(szTargetApplication))) 
  {
    ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_FINDTARGET),hwndMain,FindTargetDlgProc,(LPARAM)NULL);
    return;
  }

  if ((eUseTargetApplication != UTA_UNDEFINED) && (eTargetApplicationMode == TAM_SEND_DROP_MSG))
  {
    lstrcpy(szGlobalWndClass,szTargetApplicationWndClass);

    if (!bOpenNew) // hwnd == NULL
      EnumWindows(EnumWndProc2,(LPARAM)&hwnd);

    // Found a window
    if (hwnd != NULL && IsWindowEnabled(hwnd))
    {
      if (IsIconic(hwnd))
        ShowWindowAsync(hwnd,SW_RESTORE);

      if (Settings.bFocusEdit)
        SetForegroundWindow(hwnd);

      if (lpFileName)
      {
        hDrop = CreateDropHandle(lpFileName);
        PostMessage(hwnd,WM_DROPFILES,(WPARAM)hDrop,(LPARAM)0);
      }
    }
    else // Either no window or disabled - run target.exe
    {
      WCHAR  szFile[MAX_PATH];
      LPWSTR lpParam;
      WCHAR  szParam[MAX_PATH] = L"";
      WCHAR  szTmp[MAX_PATH];

      if (hwnd) // disabled window
      {
        WCHAR szBuf[256];
        WCHAR *c;

        // Prepare message
        GetLngString(IDS_ERR_TARGETDISABLED,szBuf,COUNTOF(szBuf));
        c = StrChr(szBuf,L'\n');
        if (c) {
          *c = 0;
          c++;
        }

        // Ask...
        if (MessageBox(hwndMain,c,szBuf,
              MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND) == IDNO)
          return;
      }

      if (PathIsLnkFile(lpFileName) &&
          PathGetLnkPath(lpFileName,szTmp,COUNTOF(szTmp)))
        lpParam = szTmp;
      else
        lpParam = (LPWSTR)lpFileName;

      //if (Is32bitExe(szTargetApplication))
      //  PathQuoteSpaces(lpParam);
      //else
        GetShortPathName(lpParam,lpParam,MAX_PATH);

      if (StrIsNotEmpty(szTargetApplicationParams)) {
        StrCpyN(szParam,szTargetApplicationParams,COUNTOF(szParam));
        StrCatBuff(szParam,L" ",COUNTOF(szParam));
      }
      StrCatBuff(szParam,lpParam,COUNTOF(szParam));

      lstrcpy(szTmp,szTargetApplication);
      PathAbsoluteFromApp(szTmp,szFile,COUNTOF(szFile),TRUE);

      //~if (!PathIsExistingFile(szFile)) {
      //~  if (!SearchPath(NULL,szTmp,NULL,COUNTOF(szFile),szFile,NULL)) {
      //~    GetModuleFileName(NULL,szFile,COUNTOF(szFile));
      //~    PathRemoveFileSpec(szFile);
      //~    PathAppend(szFile,szTmp);
      //~  }
      //~}

      SHELLEXECUTEINFO sei = { 0 };
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = 0;
      sei.hwnd = hwndMain;
      sei.lpVerb = NULL;
      sei.lpFile = szFile;
      sei.lpParameters = szParam;
      sei.lpDirectory = Settings.szCurDir;
      sei.nShow = SW_SHOWNORMAL;

      ShellExecuteEx(&sei);
    }
  }
  else
  {
    LPWSTR lpParam;
    WCHAR  szParam[MAX_PATH] = L"";
    WCHAR  szTmp[MAX_PATH];

    if ((eUseTargetApplication != UTA_UNDEFINED) &&
        (eTargetApplicationMode == TAM_SEND_DDE_MSG) &&
        ExecDDECommand(lpFileName,szDDEMsg,szDDEApp,szDDETopic))
      return;

    if ((eUseTargetApplication == UTA_UNDEFINED) && StrIsEmpty(lpFileName))
      return;

    else {

      if (PathIsLnkFile(lpFileName) &&
          PathGetLnkPath(lpFileName,szTmp,COUNTOF(szTmp)))
        lpParam = szTmp;
      else
        lpParam = (LPWSTR)lpFileName;

      //if (Is32bitExe(szTargetApplication))
      //  PathQuoteSpaces(lpParam);
      //else
        GetShortPathName(lpParam,lpParam,MAX_PATH);

      if (StrIsNotEmpty(szTargetApplicationParams)) {
        StrCpyN(szParam,szTargetApplicationParams,COUNTOF(szParam));
        StrCatBuff(szParam,L" ",COUNTOF(szParam));
      }
      StrCatBuff(szParam,lpParam,COUNTOF(szParam));

      lstrcpy(szTmp,szTargetApplication);
      ExpandEnvironmentStringsEx(szTmp,COUNTOF(szTmp));

      WCHAR  szFile[MAX_PATH];

      //~if (PathIsRelative(szTmp)) {
      //~  PathAbsoluteFromApp(szTmp,szFile,COUNTOF(szFile),TRUE);
      //~  if (!PathIsExistingFile(szFile)) {
      //~    if (!SearchPath(NULL,szTmp,NULL,COUNTOF(szFile),szFile,NULL)) {
      //~      GetModuleFileName(NULL,szFile,COUNTOF(szFile));
      //~      PathRemoveFileSpec(szFile);
      //~      PathAppend(szFile,szTmp);
      //~    }
      //~  }
      //~}
      //~else {
      //~  lstrcpy(szFile, szTmp);
      //~}
      PathAbsoluteFromApp(szTmp, szFile, COUNTOF(szFile), TRUE);

      SHELLEXECUTEINFO sei = { 0 };
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = 0;
      sei.hwnd = hwndMain;
      sei.lpVerb = NULL;
      if (eUseTargetApplication != UTA_UNDEFINED) {
        sei.lpFile = szFile;
        sei.lpParameters = szParam;
      }
      else {
        sei.lpFile = lpParam;
        sei.lpParameters = NULL;
      }
      sei.lpDirectory = Settings.szCurDir;
      sei.nShow = SW_SHOWNORMAL;

      ShellExecuteEx(&sei);
    }
  }

}


//=============================================================================
//
//  SnapToTarget()
//
//  Aligns minipath to either side of target window
//
//
void SnapToTarget(HWND hwnd)
{

  RECT rcOld,rcNew,rc2;
  int  cxScreen;
  HWND hwnd2;

  if (IniFileGetInt(g_wchIniFile,L"Target Application",L"UseTargetApplication",0xFB) != 0xFB) {

    IniFileGetString(g_wchIniFile, L"Target Application",L"TargetApplicationWndClass",
      szTargetApplicationWndClass,szTargetApplicationWndClass,COUNTOF(szTargetApplicationWndClass));

    if (StrIsEmpty(szTargetApplicationWndClass))
      return;

    else
      lstrcpy(szGlobalWndClass,szTargetApplicationWndClass);
  }
  else
    lstrcpy(szGlobalWndClass,L"Notepad3");

  hwnd2 = NULL;
  EnumWindows(EnumWndProc2,(LPARAM)&hwnd2);

  // Found a window
  if (hwnd2 != NULL)
  {
    if (IsIconic(hwnd2) || IsZoomed(hwnd2))
      SendMessage(hwnd2,WM_SYSCOMMAND,SC_RESTORE,0);

    SetForegroundWindow(hwnd2);
    BringWindowToTop(hwnd2);

    SetForegroundWindow(hwnd);

    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    GetWindowRect(hwnd,&rcOld);
    GetWindowRect(hwnd2,&rc2);

    if (rc2.left > cxScreen - rc2.right)
      rcNew.left = rc2.left - (rcOld.right - rcOld.left);

    else
      rcNew.left = rc2.right;

    rcNew.top  = rc2.top;
    rcNew.right = rcNew.left + (rcOld.right - rcOld.left);
    rcNew.bottom = rcNew.top + (rcOld.bottom - rcOld.top);

    if (!EqualRect(&rcOld,&rcNew)) {

      if (GetDoAnimateMinimize())
        DrawAnimatedRects(hwnd,IDANI_CAPTION,&rcOld,&rcNew);

      SetWindowPos(hwnd,NULL,rcNew.left,rcNew.top,0,0,SWP_NOSIZE|SWP_NOZORDER);
    }
  }

}


//=============================================================================
//
//  SnapToDefaultPos()
//
//  Aligns minipath to the default window position on the current screen
//
//
void SnapToDefaultPos(HWND hwnd)
{
  WINDOWPLACEMENT wndpl;
  int x,y,cx,cy;
  RECT rcOld;

  GetWindowRect(hwnd,&rcOld);

  MONITORINFO mi = {0};
  mi.cbSize = sizeof(MONITORINFO);
  HMONITOR const hMonitor = MonitorFromRect(&rcOld, MONITOR_DEFAULTTONEAREST);
  GetMonitorInfo(hMonitor,&mi);

  x = mi.rcWork.left + 16;
  y = mi.rcWork.top + 16;
  cx = 272;
  cy = 640;

  wndpl.length = sizeof(WINDOWPLACEMENT);
  wndpl.flags = WPF_ASYNCWINDOWPLACEMENT;
  wndpl.showCmd = SW_RESTORE;

  wndpl.rcNormalPosition.left = x;
  wndpl.rcNormalPosition.top = y;
  wndpl.rcNormalPosition.right = x + cx;
  wndpl.rcNormalPosition.bottom = y + cy;

  if (EqualRect(&rcOld,&wndpl.rcNormalPosition)) {
    x = mi.rcWork.right - cx - 16;
    wndpl.rcNormalPosition.left = x;
    wndpl.rcNormalPosition.right = x + cx;
  }

  if (GetDoAnimateMinimize()) {
    DrawAnimatedRects(hwnd,IDANI_CAPTION,&rcOld,&wndpl.rcNormalPosition);
    OffsetRect(&wndpl.rcNormalPosition,mi.rcMonitor.left - mi.rcWork.left,mi.rcMonitor.top - mi.rcWork.top);
  }

  SetWindowPlacement(hwnd, &wndpl); // 1st set correct screen (DPI Aware)
  SetWindowPlacement(hwnd, &wndpl); // 2nd resize position to correct DPI settings
}



///  End of minipath.c  \\\
