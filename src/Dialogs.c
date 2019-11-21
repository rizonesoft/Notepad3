// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Dialogs.c                                                                   *
*   Notepad3 dialog boxes implementation                                      *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#include "Helpers.h"

#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <commdlg.h>

#include <string.h>

#pragma warning( push )
#pragma warning( disable : 4201) // union/struct w/o name
#include <richedit.h>
#pragma warning( pop ) 

#include "Edit.h"
#include "Dlapi.h"
#include "resource.h"
#include "Version.h"
#include "Encoding.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"

#include "SciCall.h"

#include "Dialogs.h"

//=============================================================================

#define OIC_SAMPLE          32512
#define OIC_HAND            32513
#define OIC_QUES            32514
#define OIC_BANG            32515
#define OIC_NOTE            32516
#if(WINVER >= 0x0400)
#define OIC_WINLOGO         32517
#define OIC_WARNING         OIC_BANG
#define OIC_ERROR           OIC_HAND
#define OIC_INFORMATION     OIC_NOTE
#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0600)
#define OIC_SHIELD          32518
#endif /* WINVER >= 0x0600 */

//=============================================================================
//
//  MessageBoxLng()
//
static HHOOK s_hhkMsgBox = NULL;

static LRESULT CALLBACK CenterInParentHook(INT nCode, WPARAM wParam, LPARAM lParam)
{
  // notification that a window is about to be activated  
  if (nCode == HCBT_ACTIVATE) {
    // set window handles
    HWND const hChildWnd = (HWND)wParam; // window handle is wParam
    HWND const hParentWnd = GetParent(hChildWnd);

    RECT  rParent, rChild, rDesktop;
    if ((hParentWnd && hChildWnd) &&
        (GetWindowRect(GetDesktopWindow(), &rDesktop) == TRUE) &&
        (GetWindowRect(hParentWnd, &rParent) == TRUE) &&
        (GetWindowRect(hChildWnd, &rChild) == TRUE))
    {
      if (s_hhkMsgBox) {
        UnhookWindowsHookEx(s_hhkMsgBox);
        s_hhkMsgBox = NULL;
      }
      CenterDlgInParent(hChildWnd, hParentWnd);
      PostMessage(hChildWnd, WM_SETFOCUS, 0, 0);
      PostMessage(hChildWnd, WM_PAINT, 0, 0);
    }
  }
  // continue with any possible chained hooks
  return s_hhkMsgBox ? CallNextHookEx(s_hhkMsgBox, nCode, wParam, lParam) : 0;
}
// -----------------------------------------------------------------------------


int MessageBoxLng(HWND hwnd, UINT uType, UINT uidMsg, ...)
{
  WCHAR szFormat[HUGE_BUFFER] = { L'\0' };
  if (!GetLngString(uidMsg, szFormat, COUNTOF(szFormat))) { return -1; }

  WCHAR szTitle[128] = { L'\0' };
  GetLngString(IDS_MUI_APPTITLE, szTitle, COUNTOF(szTitle));

  WCHAR szText[HUGE_BUFFER] = { L'\0' };
  const PUINT_PTR argp = (PUINT_PTR)&uidMsg + 1;
  if (argp && *argp) {
    StringCchVPrintfW(szText, COUNTOF(szText), szFormat, (LPVOID)argp);
  }
  else {
    StringCchCopy(szText, COUNTOF(szText), szFormat);
  }

  uType |= (MB_TOPMOST | MB_SETFOREGROUND);

  // center message box to focus or main
  s_hhkMsgBox = SetWindowsHookEx(WH_CBT, &CenterInParentHook, 0, GetCurrentThreadId());

  return MessageBoxEx(hwnd, szText, szTitle, uType, Globals.iPrefLANGID);
}


//=============================================================================
//
//  GetLastErrorToMsgBox()
//
DWORD GetLastErrorToMsgBox(LPWSTR lpszFunction, DWORD dwErrID)
{
  // Retrieve the system error message for the last-error code
  if (!dwErrID) {
    dwErrID = GetLastError();
  }

  LPVOID lpMsgBuf = NULL;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dwErrID,
    Globals.iPrefLANGID,
    (LPTSTR)&lpMsgBuf,
    0, NULL);

  if (lpMsgBuf) {
    // Display the error message and exit the process
    size_t const len = StringCchLenW((LPCWSTR)lpMsgBuf, 0) + StringCchLenW((LPCWSTR)lpszFunction, 0) + 80;
    LPWSTR lpDisplayBuf = (LPWSTR)AllocMem(len * sizeof(WCHAR), HEAP_ZERO_MEMORY);

    if (lpDisplayBuf) {
      StringCchPrintf(lpDisplayBuf, len, L"Error: '%s' failed with error id %d:\n%s.\n",
        lpszFunction, dwErrID, (LPCWSTR)lpMsgBuf);

      // center message box to main
      HWND focus = GetFocus();
      HWND hwnd = focus ? focus : Globals.hwndMain;
      s_hhkMsgBox = SetWindowsHookEx(WH_CBT, &CenterInParentHook, 0, GetCurrentThreadId());

      MessageBoxEx(hwnd, lpDisplayBuf, L"Notepad3 - ERROR", MB_ICONERROR, Globals.iPrefLANGID);

      FreeMem(lpDisplayBuf);
    }
    LocalFree(lpMsgBuf); // LocalAlloc()
  }
  return dwErrID;
}


//=============================================================================
//
//  _InfoBoxLngDlgProc()
//
//

typedef struct _infbox {
  UINT   uType;
  LPWSTR lpstrMessage;
  LPWSTR lpstrSetting;
  bool   bDisableCheckBox;
} INFOBOXLNG, *LPINFOBOXLNG;


static INT_PTR CALLBACK _InfoBoxLngDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {
  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

      LPINFOBOXLNG const lpMsgBox = (LPINFOBOXLNG)lParam;

      int const cx = GetSystemMetrics(SM_CXSMICON);
      int const cy = GetSystemMetrics(SM_CYSMICON);
      UINT const fuLoad = LR_DEFAULTCOLOR | LR_SHARED;

      switch (lpMsgBox->uType & MB_ICONMASK)
      {
      case MB_ICONQUESTION:
        SendDlgItemMessage(hwnd, IDC_INFOBOXICON, STM_SETICON, (WPARAM)LoadImage(NULL, MAKEINTRESOURCE(OIC_QUES), IMAGE_ICON, cx, cy, fuLoad), 0);
        break;
      case MB_ICONWARNING:  // = MB_ICONEXCLAMATION
        SendDlgItemMessage(hwnd, IDC_INFOBOXICON, STM_SETICON, (WPARAM)LoadImage(NULL, MAKEINTRESOURCE(OIC_WARNING), IMAGE_ICON, cx, cy, fuLoad), 0);
        break;
      case MB_ICONERROR:  // = MB_ICONSTOP, MB_ICONHAND
        SendDlgItemMessage(hwnd, IDC_INFOBOXICON, STM_SETICON, (WPARAM)LoadImage(NULL, MAKEINTRESOURCE(OIC_ERROR), IMAGE_ICON, cx, cy, fuLoad), 0);
        break;
      case MB_USERICON:
        SendDlgItemMessage(hwnd, IDC_INFOBOXICON, STM_SETICON, (WPARAM)Globals.hIcon48, 0);
        break;
      case MB_ICONINFORMATION:  // = MB_ICONASTERISK
      default:
        SendDlgItemMessage(hwnd, IDC_INFOBOXICON, STM_SETICON, (WPARAM)LoadImage(NULL, MAKEINTRESOURCE(OIC_INFORMATION), IMAGE_ICON, cx, cy, fuLoad), 0);
        break;
      }

      SetDlgItemText(hwnd, IDC_INFOBOXTEXT, lpMsgBox->lpstrMessage);

      if (lpMsgBox->bDisableCheckBox) {
        DialogEnableControl(hwnd, IDC_INFOBOXCHECK, false);
        DialogHideControl(hwnd, IDC_INFOBOXCHECK, true);
      }

      CenterDlgInParent(hwnd, NULL);
      AttentionBeep(lpMsgBox->uType);

      FreeMem(lpMsgBox->lpstrMessage);
    }
    return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;


  case WM_COMMAND:
    {
      LPINFOBOXLNG const lpMsgBox = (LPINFOBOXLNG)GetWindowLongPtr(hwnd, DWLP_USER);
      switch (LOWORD(wParam))
      {
      case IDOK:
      case IDYES:
      case IDRETRY:
      case IDIGNORE:
      case IDTRYAGAIN:
      case IDCONTINUE:
        if (IsButtonChecked(hwnd, IDC_INFOBOXCHECK) && StrIsNotEmpty(lpMsgBox->lpstrSetting)) {
          IniFileSetInt(Globals.IniFile, Constants.SectionSuppressedMessages, lpMsgBox->lpstrSetting, LOWORD(wParam));
        }
      case IDNO:
      case IDABORT:
      case IDCLOSE:
      case IDCANCEL:
        EndDialog(hwnd, LOWORD(wParam));
        return true;

      case IDC_INFOBOXCHECK:
        DialogEnableControl(hwnd, IDNO, !IsButtonChecked(hwnd, IDC_INFOBOXCHECK));
        DialogEnableControl(hwnd, IDABORT, !IsButtonChecked(hwnd, IDC_INFOBOXCHECK));
        DialogEnableControl(hwnd, IDCLOSE, !IsButtonChecked(hwnd, IDC_INFOBOXCHECK));
        DialogEnableControl(hwnd, IDCANCEL, !IsButtonChecked(hwnd, IDC_INFOBOXCHECK));
        break;

      default:
        break;
      }
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  InfoBoxLng()
//
//
INT_PTR InfoBoxLng(UINT uType, LPCWSTR lpstrSetting, UINT uidMsg, ...)
{
  int const iMode = StrIsEmpty(lpstrSetting) ? 0 : IniFileGetInt(Globals.IniFile, Constants.SectionSuppressedMessages, lpstrSetting, 0);
  switch (iMode) {
    case IDOK:
    case IDYES:
    case IDCONTINUE:
      return iMode;

    case 0:
      // no entry found
    case -1:
      // disable "Don't display again" check-box
      break;

    default:
      IniFileDelete(Globals.IniFile, Constants.SectionSuppressedMessages, lpstrSetting, false);
      break;
  }

  WCHAR wchMessage[LARGE_BUFFER];
  if (!GetLngString(uidMsg, wchMessage, COUNTOF(wchMessage))) { return -1LL; }

  INFOBOXLNG msgBox;
  msgBox.uType = uType;
  msgBox.lpstrMessage = AllocMem((COUNTOF(wchMessage)+1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);

  const PUINT_PTR argp = (PUINT_PTR)& uidMsg + 1;
  if (argp && *argp) {
    StringCchVPrintfW(msgBox.lpstrMessage, COUNTOF(wchMessage), wchMessage, (LPVOID)argp);
  }
  else {
    StringCchCopy(msgBox.lpstrMessage, COUNTOF(wchMessage), wchMessage);
  }

  if (uidMsg == IDS_MUI_ERR_LOADFILE || uidMsg == IDS_MUI_ERR_SAVEFILE ||
    uidMsg == IDS_MUI_CREATEINI_FAIL || uidMsg == IDS_MUI_WRITEINI_FAIL ||
    uidMsg == IDS_MUI_EXPORT_FAIL || uidMsg == IDS_MUI_ERR_ELEVATED_RIGHTS) 
  {

    LPVOID lpMsgBuf = NULL;
    if (Globals.dwLastError != ERROR_SUCCESS) {
      FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        Globals.dwLastError,
        Globals.iPrefLANGID,
        (LPWSTR)&lpMsgBuf, 0,
        NULL);

      Globals.dwLastError = ERROR_SUCCESS; // reset;
    }

    if (lpMsgBuf) {
      StringCchCat(msgBox.lpstrMessage, COUNTOF(wchMessage), L"\n\n");
      StringCchCat(msgBox.lpstrMessage, COUNTOF(wchMessage), lpMsgBuf);
      LocalFree(lpMsgBuf);
    }

    WCHAR wcht = *CharPrev(msgBox.lpstrMessage, StrEnd(msgBox.lpstrMessage, COUNTOF(wchMessage)));
    if (IsCharAlphaNumeric(wcht) || wcht == '"' || wcht == '\'')
      StringCchCat(msgBox.lpstrMessage, COUNTOF(wchMessage), L".");
  }

  msgBox.lpstrSetting = (LPWSTR)lpstrSetting;
  msgBox.bDisableCheckBox = (StrIsEmpty(Globals.IniFile) || StrIsEmpty(lpstrSetting) || (iMode < 0)) ? true : false;


  int idDlg;
  switch (uType & MB_TYPEMASK) {

  case MB_YESNO:  // contains two push buttons : Yes and No.
    idDlg = IDD_MUI_INFOBOX2;
    break;

  case MB_OKCANCEL:  // contains two push buttons : OK and Cancel.
    idDlg = IDD_MUI_INFOBOX3;
    break;

  case MB_YESNOCANCEL:  // contains three push buttons : Yes, No, and Cancel.
    idDlg = IDD_MUI_INFOBOX4;
    break;

  case MB_RETRYCANCEL:  // contains two push buttons : Retry and Cancel.
    idDlg = IDD_MUI_INFOBOX5;
    break;

  case MB_ABORTRETRYIGNORE:   // three push buttons : Abort, Retry, and Ignore.
  case MB_CANCELTRYCONTINUE:  // three push buttons : Cancel, Try Again, Continue.Use this message box type instead of MB_ABORTRETRYIGNORE.

  case MB_OK:  // one push button : OK. This is the default.
  default:
    idDlg = IDD_MUI_INFOBOX;
    break;
  }

  HWND focus = GetFocus();
  HWND hwnd = focus ? focus : Globals.hwndMain;

  return ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(idDlg), hwnd, _InfoBoxLngDlgProc, (LPARAM)&msgBox);
}


//=============================================================================
//
//  DisplayCmdLineHelp()
//
#if 0
void DisplayCmdLineHelp(HWND hwnd)
{
  WCHAR szTitle[32] = { L'\0' };
  WCHAR szText[2048] = { L'\0' };

  GetLngString(IDS_MUI_APPTITLE,szTitle,COUNTOF(szTitle));
  GetLngString(IDS_MUI_CMDLINEHELP,szText,COUNTOF(szText));

  MSGBOXPARAMS mbp;
  ZeroMemory(&mbp, sizeof(MSGBOXPARAMS));
  mbp.cbSize = sizeof(MSGBOXPARAMS);
  mbp.hwndOwner = hwnd;
  mbp.hInstance = Globals.hInstance;
  mbp.lpszText = szText;
  mbp.lpszCaption = szTitle;
  mbp.dwStyle = MB_OK | MB_USERICON | MB_SETFOREGROUND;
  mbp.lpszIcon = MAKEINTRESOURCE(IDR_MAINWND48);
  mbp.dwContextHelpId = 0;
  mbp.lpfnMsgBoxCallback = NULL;
  mbp.dwLanguageId = Globals.iPrefLANGID;

  hhkMsgBox = SetWindowsHookEx(WH_CBT, &_MsgBoxProc, 0, GetCurrentThreadId());

  MessageBoxIndirect(&mbp);
  //MsgBoxLng(MBINFO, IDS_MUI_CMDLINEHELP);
}
#else

static INT_PTR CALLBACK CmdLineHelpProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  switch (umsg) {
  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }
      WCHAR szText[4096] = { L'\0' };
      GetLngString(IDS_MUI_CMDLINEHELP, szText, COUNTOF(szText));
      SetDlgItemText(hwnd, IDC_CMDLINEHELP, szText);
      CenterDlgInParent(hwnd, NULL);
    }
    return true;

  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
    case IDCANCEL:
    case IDYES:
    case IDNO:
      EndDialog(hwnd, LOWORD(wParam));
      break;
    }
    return true;

  default:
    break;
  }
  return false;
}

INT_PTR DisplayCmdLineHelp(HWND hwnd)
{
  return ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_CMDLINEHELP), hwnd, CmdLineHelpProc, (LPARAM)L"");

  //if (!hwnd) {
  //  // text to std-out
  //  //RedirectIOToConsole();
  //  //fwprintf(stdout, L"\n!!! blahblub ???\n");
  //  //fflush(stdout);
  //  //SleepEx(5000,FALSE);
  //}
  //return(0);
}

#endif

//=============================================================================
//
//  BFFCallBack()
//
int CALLBACK BFFCallBack(HWND hwnd,UINT umsg,LPARAM lParam,LPARAM lpData)
{
  if (umsg == BFFM_INITIALIZED)
    SendMessage(hwnd,BFFM_SETSELECTION,true,lpData);

  UNUSED(lParam);
  return(0);
}


//=============================================================================
//
//  GetDirectory()
//
bool GetDirectory(HWND hwndParent,int uiTitle,LPWSTR pszFolder,LPCWSTR pszBase,bool bNewDialogStyle)
{
  BROWSEINFO bi;
  WCHAR szTitle[MIDSZ_BUFFER] = { L'\0' };;
  WCHAR szBase[MAX_PATH] = { L'\0' };

  GetLngString(uiTitle,szTitle,COUNTOF(szTitle));

  if (!pszBase || !*pszBase)
    GetCurrentDirectory(MAX_PATH, szBase);
  else
    StringCchCopyN(szBase, COUNTOF(szBase), pszBase, MAX_PATH);

  ZeroMemory(&bi, sizeof(BROWSEINFO));
  bi.hwndOwner = hwndParent;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = pszFolder;
  bi.lpszTitle = szTitle;
  bi.ulFlags = BIF_RETURNONLYFSDIRS;
  if (bNewDialogStyle)
    bi.ulFlags |= BIF_NEWDIALOGSTYLE;
  bi.lpfn = &BFFCallBack;
  bi.lParam = (LPARAM)szBase;
  bi.iImage = 0;

  LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
  if (pidl) {
    SHGetPathFromIDList(pidl,pszFolder);
    CoTaskMemFree(pidl);
    return true;
  }
  return false;
}


/*
//=============================================================================
//
//  _LoadStringEx()
//
static DWORD _LoadStringEx(UINT nResId, LPCTSTR pszRsType, LPSTR strOut)
{
  LPTSTR pszResId = MAKEINTRESOURCE(nResId);

  if (Globals.hInstance == NULL)
    return 0L;

  HRSRC hRsrc = FindResource(Globals.hInstance, pszResId, pszRsType);

  if (hRsrc == NULL) {
    return 0L;
  }

  HGLOBAL hGlobal = LoadResource(Globals.hInstance, hRsrc);

  if (hGlobal == NULL) {
    return 0L;
  }

  const BYTE* pData = (const BYTE*)LockResource(hGlobal);

  if (pData == NULL) {
    FreeResource(hGlobal);
    return 0L;
  }

  DWORD dwSize = SizeofResource(Globals.hInstance, hRsrc);

  if (strOut) {
    memcpy(strOut, (LPCSTR)pData, dwSize);
  }

  UnlockResource(hGlobal);

  FreeResource(hGlobal);

  return dwSize;
}

*/

//=============================================================================
//
//  (EditStreamCallback)
//  _LoadRtfCallback() RTF edit control StreamIn's callback function 
//
#if true

static DWORD CALLBACK _LoadRtfCallback(
  DWORD_PTR dwCookie,  // (in) pointer to the string
  LPBYTE pbBuff,       // (in) pointer to the destination buffer
  LONG cb,             // (in) size in bytes of the destination buffer
  LONG FAR* pcb        // (out) number of bytes transfered
)
{
  LPSTR* pstr = (LPSTR*)dwCookie;
  LONG const len = (LONG)StringCchLenA(*pstr,0);

  if (len < cb)
  {
    *pcb = len;
    memcpy(pbBuff, (LPCSTR)*pstr, *pcb);
    *pstr += len;
    //*pstr = '\0';
  }
  else
  {
    *pcb = cb;
    memcpy(pbBuff, (LPCSTR)*pstr, *pcb);
    *pstr += cb;
  }
  return 0;
}
// ----------------------------------------------------------------------------

#else

static DWORD CALLBACK _LoadRtfCallbackW(
  DWORD_PTR dwCookie,  // (in) pointer to the string
  LPBYTE pbBuff,       // (in) pointer to the destination buffer
  LONG cb,             // (in) size in bytes of the destination buffer
  LONG FAR* pcb        // (out) number of bytes transfered
)
{
  LPWSTR* pstr = (LPWSTR*)dwCookie;
  LONG const len = (LONG)StringCchLen(*pstr, 0);
  LONG const size = len * sizeof(WCHAR);

  cb -= (cb % sizeof(WCHAR));

  if (size < cb) {
    *pcb = size;
    memcpy(pbBuff, (LPCWSTR)*pstr, *pcb);
    *pstr += len;
    //*pstr = '\0';
  }
  else {
    *pcb = cb;
    memcpy(pbBuff, (LPCWSTR)*pstr, *pcb);
    *pstr += (cb / sizeof(WCHAR));
  }
  return 0;
}
// ----------------------------------------------------------------------------

#endif

//=============================================================================
//
//  AboutDlgProc()
//
INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {
  case WM_INITDIALOG:
  {
    SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
    if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

    SetDlgItemText(hwnd, IDC_VERSION, _W(_STRG(VERSION_FILEVERSION_LONG)));

    static HFONT hFontTitle = NULL;
    if (hFontTitle) { DeleteObject(hFontTitle); }
    if (NULL == (hFontTitle = (HFONT)SendDlgItemMessage(hwnd, IDC_VERSION, WM_GETFONT, 0, 0))) {
      hFontTitle = GetStockObject(DEFAULT_GUI_FONT);
    }
    LOGFONT lf;
    GetObject(hFontTitle, sizeof(LOGFONT), &lf);
    lf.lfWeight = FW_BOLD;
    lf.lfWidth  = ScaleIntFontSize(8);
    lf.lfHeight = ScaleIntFontSize(22);
    // lf.lfQuality = ANTIALIASED_QUALITY;
    hFontTitle = CreateFontIndirect(&lf);

    SendDlgItemMessage(hwnd, IDC_VERSION, WM_SETFONT, (WPARAM)hFontTitle, true);

    SetDlgItemText(hwnd, IDC_SCI_VERSION, VERSION_SCIVERSION);
    SetDlgItemText(hwnd, IDC_COPYRIGHT, _W(VERSION_LEGALCOPYRIGHT));
    SetDlgItemText(hwnd, IDC_AUTHORNAME, _W(VERSION_AUTHORNAME));
    SetDlgItemText(hwnd, IDC_COMPILER, VERSION_COMPILER);

    WCHAR wch[256] = { L'\0' };
    if (GetDlgItem(hwnd, IDC_WEBPAGE) == NULL) {
      SetDlgItemText(hwnd, IDC_WEBPAGE2, _W(VERSION_WEBPAGEDISPLAY));
      ShowWindow(GetDlgItem(hwnd, IDC_WEBPAGE2), SW_SHOWNORMAL);
    }
    else {
      StringCchPrintf(wch, COUNTOF(wch), L"<A>%s</A>", _W(VERSION_WEBPAGEDISPLAY));
      SetDlgItemText(hwnd, IDC_WEBPAGE, wch);
    }
    GetLngString(IDS_MUI_TRANSL_AUTHOR, wch, COUNTOF(wch));
    SetDlgItemText(hwnd, IDC_TRANSL_AUTH, wch);

    // --- Rich Edit Control ---
    //SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETBKGNDCOLOR, 0, (LPARAM)GetBackgroundColor(hwnd));
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETBKGNDCOLOR, 0, (LPARAM)GetSysColor(COLOR_3DFACE));
    
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SHOWSCROLLBAR, SB_VERT, (LPARAM)true);
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SHOWSCROLLBAR, SB_HORZ, (LPARAM)false);

    DWORD styleFlags = SES_EXTENDBACKCOLOR; // | SES_HYPERLINKTOOLTIPS;
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETEDITSTYLE, (WPARAM)styleFlags, (LPARAM)styleFlags);
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_AUTOURLDETECT, (WPARAM)1, (LPARAM)0);

    //CHARFORMAT2 cf2;
    //ZeroMemory(&cf2, sizeof(CHARFORMAT2));
    //cf2.dwMask = CFM_LINK | CFM_UNDERLINE | CFM_COLOR | CFM_LINKPROTECTED;
    //cf2.dwEffects = CFE_LINK | CFE_UNDERLINE | CFE_LINKPROTECTED;
    //cf2.crTextColor = RGB(255, 0, 0);
    //cf2.bUnderlineType = CFU_UNDERLINENONE;
    //SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETEDITSTYLEEX, 0, (LPARAM)SES_EX_HANDLEFRIENDLYURL);
    //SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2);

    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETEVENTMASK, 0, (LPARAM)(ENM_LINK)); // link click

  #if true

    static char pAboutResource[8192] = { '\0' };
    static char* pAboutInfo = NULL;

    char pAboutRes[4000];
    GetLngStringA(IDS_MUI_ABOUT_RTF_0, pAboutRes, COUNTOF(pAboutRes));
    StringCchCopyA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_DEV, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_RTF_1, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_CONTRIBS, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_RTF_2, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_LIBS, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_RTF_3, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_ACKNOWLEDGES, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_RTF_4, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_MORE, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_RTF_5, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_LICENSES, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_RTF_6, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);

    pAboutInfo = pAboutResource;

    EDITSTREAM editStreamIn = { (DWORD_PTR)&pAboutInfo, 0, _LoadRtfCallback };
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, SF_RTF, (LPARAM)&editStreamIn);

  #else

    static WCHAR pAboutResource[8192] = { L'\0' };
    static PWCHAR pAboutInfo = NULL;

    WCHAR pAboutRes[4000];
    GetLngString(IDS_MUI_ABOUT_RTF_1, pAboutRes, COUNTOF(pAboutRes));
    StringCchCopy(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_CONTRIBS, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_RTF_2, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_LIBS, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_RTF_3, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_ACKNOWLEDGES, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_RTF_4, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_MORE, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_RTF_5, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_LICENSES, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngString(IDS_MUI_ABOUT_RTF_6, pAboutRes, COUNTOF(pAboutRes));
    StringCchCat(pAboutResource, COUNTOF(pAboutResource), pAboutRes);

    pAboutInfo = pAboutResource;

    EDITSTREAM editStreamIn = { (DWORD_PTR)&pAboutInfo, 0, _LoadRtfCallbackW };
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, (WPARAM)(UINT)(SF_TEXT | SF_UNICODE), (LPARAM)&editStreamIn);

    // EM_SETTEXTEX is Richedit 3.0 only
    //SETTEXTEX ste;
    //ste.flags = ST_SELECTION;  // replace everything
    //ste.codepage = 1200;       // Unicode is codepage 1200
    //SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)pAboutInfo);

  #endif
    
    CenterDlgInParent(hwnd, NULL);
  }
  return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;


  case WM_PAINT:
    if (Globals.hIcon128) {
      RECT rt;
      GetWindowRect(hwnd, &rt);
      HDC hdc = GetWindowDC(hwnd);
      DrawIconEx(hdc, 22, 44, Globals.hIcon128, 128, 128, 0, NULL, DI_NORMAL);
      ReleaseDC(hwnd, hdc);
    }
    return false;


  case WM_NOTIFY:
  {
    LPNMHDR pnmhdr = (LPNMHDR)lParam;
    switch (pnmhdr->code)
    {
      case NM_CLICK:
      case NM_RETURN:
      {
        switch (pnmhdr->idFrom)
        {
        case IDC_WEBPAGE:
          ShellExecute(hwnd, L"open", L"https://www.rizonesoft.com", NULL, NULL, SW_SHOWNORMAL);
          break;

        default:
          break;
        }
      }
      break;

      case EN_LINK: // hyperlink from RichEdit Ctrl
      {
        ENLINK* penLink = (ENLINK *)lParam;
        if (penLink->msg == WM_LBUTTONDOWN) 
        {
          WCHAR hLink[256] = { L'\0' };
          TEXTRANGE txtRng;
          txtRng.chrg = penLink->chrg;
          txtRng.lpstrText = hLink;
          SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_GETTEXTRANGE, 0, (LPARAM)&txtRng);
          ShellExecute(hwnd, L"open", hLink, NULL, NULL, SW_SHOWNORMAL);
        }
      }
      break;
    }
  }
  break;

  case WM_SETCURSOR:
    {
      if ((LOWORD(lParam) == HTCLIENT) && 
          (GetDlgCtrlID((HWND)wParam) == IDC_RIZONEBMP))
      {
        SetCursor(LoadCursor(NULL, IDC_HAND));
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR)true);
        return true;
      }
    }
    break;

  case WM_COMMAND:

    switch (LOWORD(wParam))
    {
    case IDC_RIZONEBMP:
      ShellExecute(hwnd, L"open", _W(VERSION_WEBPAGEDISPLAY), NULL, NULL, SW_SHOWNORMAL);
      break;

    case IDC_COPYVERSTRG:
      {
        WCHAR wchBuf[128] = { L'\0' };
        WCHAR wchBuf2[128] = { L'\0' };
        WCHAR wchVerInfo[2048] = { L'\0' };

        StringCchCopy(wchVerInfo, COUNTOF(wchVerInfo), _W(_STRG(VERSION_FILEVERSION_LONG)));
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_COMPILER);

        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n");
        GetWinVersionString(wchBuf, COUNTOF(wchBuf));
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

        //GetLngString(IDS_MUI_TRANSL_AUTHOR, wchBuf, COUNTOF(wchBuf));
        StringCchCopy(wchBuf, COUNTOF(wchBuf), L"en-US");
        for (int lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
          if (MUI_LanguageDLLs[lng].bIsActive) {
            StringCchCopy(wchBuf, COUNTOF(wchBuf), MUI_LanguageDLLs[lng].szLocaleName);
            break;
          }
        }
        StringCchPrintf(wchBuf2, ARRAYSIZE(wchBuf2), L"\nLocale: %s (Codepage: '%s')", 
          wchBuf, g_Encodings[CPI_ANSI_DEFAULT].wchLabel);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf2);

        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_SCIVERSION);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_ONIGURUMA);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_UCHARDET);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_TINYEXPR);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_UTHASH);

        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n");

        SetClipboardTextW(Globals.hwndMain, wchVerInfo, StringCchLen(wchVerInfo,0));
      }
      break;

    case IDOK:
    case IDCANCEL:
      EndDialog(hwnd, IDOK);
      break;
    }
    return true;
  }
  return false;
}



//=============================================================================
//
//  RunDlgProc()
//
static INT_PTR CALLBACK RunDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg)
  {
    case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }
      // MakeBitmapButton(hwnd,IDC_SEARCHEXE,Globals.hInstance,IDB_OPEN);
      SendDlgItemMessage(hwnd, IDC_COMMANDLINE, EM_LIMITTEXT, MAX_PATH - 1, 0);
      SetDlgItemText(hwnd, IDC_COMMANDLINE, (LPCWSTR)lParam);
      SHAutoComplete(GetDlgItem(hwnd, IDC_COMMANDLINE), SHACF_FILESYSTEM);

      CenterDlgInParent(hwnd, NULL);
    }
    return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd, IDC_SEARCHEXE);
      return false;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {

        case IDC_SEARCHEXE:
        {
          WCHAR szArgs[MAX_PATH] = { L'\0' };
          WCHAR szArg2[MAX_PATH] = { L'\0' };
          WCHAR szFile[MAX_PATH] = { L'\0' };
          WCHAR szFilter[MAX_PATH] = { L'\0' };
          OPENFILENAME ofn;
          ZeroMemory(&ofn, sizeof(OPENFILENAME));

          GetDlgItemText(hwnd, IDC_COMMANDLINE, szArgs, COUNTOF(szArgs));
          ExpandEnvironmentStringsEx(szArgs, COUNTOF(szArgs));
          ExtractFirstArgument(szArgs, szFile, szArg2, MAX_PATH);

          GetLngString(IDS_MUI_FILTER_EXE, szFilter, COUNTOF(szFilter));
          PrepareFilterStr(szFilter);

          ofn.lStructSize = sizeof(OPENFILENAME);
          ofn.hwndOwner = hwnd;
          ofn.lpstrFilter = szFilter;
          ofn.lpstrFile = szFile;
          ofn.nMaxFile = COUNTOF(szFile);
          ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
            | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

          if (GetOpenFileName(&ofn)) {
            PathQuoteSpaces(szFile);
            if (StrIsNotEmpty(szArg2))
            {
              StringCchCat(szFile, COUNTOF(szFile), L" ");
              StringCchCat(szFile, COUNTOF(szFile), szArg2);
            }
            SetDlgItemText(hwnd, IDC_COMMANDLINE, szFile);
          }
          PostMessage(hwnd, WM_NEXTDLGCTL, 1, 0);
        }
        break;


        case IDC_COMMANDLINE:
        {
          bool bEnableOK = false;
          WCHAR args[MAX_PATH] = { L'\0' };

          if (GetDlgItemText(hwnd, IDC_COMMANDLINE, args, MAX_PATH)) {
            if (ExtractFirstArgument(args, args, NULL, MAX_PATH)) {
              if (StrIsNotEmpty(args)) {
                bEnableOK = true;
              }
            }
          }
          DialogEnableControl(hwnd, IDOK, bEnableOK);
        }
        break;


        case IDOK:
        {
          WCHAR arg1[MAX_PATH] = { L'\0' };
          WCHAR arg2[MAX_PATH] = { L'\0' };
          WCHAR wchDirectory[MAX_PATH] = { L'\0' };

          if (GetDlgItemText(hwnd, IDC_COMMANDLINE, arg1, MAX_PATH))
          {
            bool bQuickExit = false;

            ExpandEnvironmentStringsEx(arg1, COUNTOF(arg1));
            ExtractFirstArgument(arg1, arg1, arg2, MAX_PATH);

            if (StringCchCompareNI(arg1, COUNTOF(arg1), _W(SAPPNAME), CSTRLEN(_W(SAPPNAME))) == 0 ||
              StringCchCompareNI(arg1, COUNTOF(arg1), L"notepad3.exe", CSTRLEN(L"notepad3.exe")) == 0) {
              GetModuleFileName(NULL, arg1, COUNTOF(arg1));
              bQuickExit = true;
            }

            if (StrIsNotEmpty(Globals.CurrentFile)) {
              StringCchCopy(wchDirectory, COUNTOF(wchDirectory), Globals.CurrentFile);
              PathCchRemoveFileSpec(wchDirectory, COUNTOF(wchDirectory));
            }

            SHELLEXECUTEINFO sei;
            ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
            sei.cbSize = sizeof(SHELLEXECUTEINFO);
            sei.fMask = 0;
            sei.hwnd = hwnd;
            sei.lpVerb = NULL;
            sei.lpFile = arg1;
            sei.lpParameters = arg2;
            sei.lpDirectory = wchDirectory;
            sei.nShow = SW_SHOWNORMAL;

            if (bQuickExit) {
              sei.fMask |= SEE_MASK_NOZONECHECKS;
              EndDialog(hwnd, IDOK);
              ShellExecuteEx(&sei);
            }

            else {
              if (ShellExecuteEx(&sei))
                EndDialog(hwnd, IDOK);

              else
                PostMessage(hwnd, WM_NEXTDLGCTL,
                (WPARAM)(GetDlgItem(hwnd, IDC_COMMANDLINE)), 1);
            }
          }
        }
        break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

      }

      return true;

  }

  return false;

}


//=============================================================================
//
//  RunDlg()
//
INT_PTR RunDlg(HWND hwnd,LPCWSTR lpstrDefault)
{
  return ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_RUN), hwnd, RunDlgProc, (LPARAM)lpstrDefault);
}


//=============================================================================
//
//  OpenWithDlgProc()
//
static INT_PTR CALLBACK OpenWithDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }
        ResizeDlg_Init(hwnd,Settings.OpenWithDlgSizeX,Settings.OpenWithDlgSizeY,IDC_RESIZEGRIP);

        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_OPENWITHDIR),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,&lvc);
        DirList_Init(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL);
        DirList_Fill(GetDlgItem(hwnd,IDC_OPENWITHDIR),Settings.OpenWithDir,DL_ALLOBJECTS,NULL,false,Flags.NoFadeHidden,DS_NAME,false);
        DirList_StartIconThread(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        ListView_SetItemState(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETOPENWITHDIR,Globals.hInstance,IDB_OPEN);

        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_DESTROY:
      DirList_Destroy(GetDlgItem(hwnd,IDC_OPENWITHDIR));
      DeleteBitmapButton(hwnd,IDC_GETOPENWITHDIR);

      ResizeDlg_Destroy(hwnd,&Settings.OpenWithDlgSizeX,&Settings.OpenWithDlgSizeY);
      return false;


    case WM_SIZE:
      {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        hdwp = BeginDeferWindowPos(6);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDOK,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDCANCEL,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_OPENWITHDIR,dx,dy,SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_GETOPENWITHDIR,0,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_OPENWITHDESCR,0,dy,SWP_NOSIZE);
        EndDeferWindowPos(hdwp);

        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVSCW_AUTOSIZE_USEHEADER);
      }
      return true;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd,lParam);
      return true;


    case WM_NOTIFY:
      {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_OPENWITHDIR)
        {
          switch(pnmh->code)
          {
            case LVN_GETDISPINFO:
              DirList_GetDispInfo(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam,Flags.NoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;
                DialogEnableControl(hwnd,IDOK,(pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_OPENWITHDIR))) {
                SendWMCommand(hwnd, IDOK);
              }
              break;
          }
        }
      }
      return true;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_GETOPENWITHDIR:
          {
            if (GetDirectory(hwnd,IDS_MUI_OPENWITH,Settings.OpenWithDir,Settings.OpenWithDir,true))
            {
              DirList_Fill(GetDlgItem(hwnd,IDC_OPENWITHDIR),Settings.OpenWithDir,DL_ALLOBJECTS,NULL,false,Flags.NoFadeHidden,DS_NAME,false);
              DirList_StartIconThread(GetDlgItem(hwnd,IDC_OPENWITHDIR));
              ListView_EnsureVisible(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,false);
              ListView_SetItemState(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);
            }
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_OPENWITHDIR)),1);
          }
          break;


        case IDOK: {
            LPDLITEM lpdli = (LPDLITEM)GetWindowLongPtr(hwnd,DWLP_USER);
            lpdli->mask = DLI_FILENAME | DLI_TYPE;
            lpdli->ntype = DLE_NONE;
            DirList_GetItem(GetDlgItem(hwnd,IDC_OPENWITHDIR),(-1),lpdli);

            if (lpdli->ntype != DLE_NONE)
              EndDialog(hwnd,IDOK);
            else
              SimpleBeep();
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return true;

  }

  return false;

}


//=============================================================================
//
//  OpenWithDlg()
//
bool OpenWithDlg(HWND hwnd,LPCWSTR lpstrFile)
{
  bool result = false;

  DLITEM dliOpenWith;
  dliOpenWith.mask = DLI_FILENAME;

  if (IDOK == ThemedDialogBoxParam(Globals.hLngResContainer,MAKEINTRESOURCE(IDD_MUI_OPENWITH),
                             hwnd,OpenWithDlgProc,(LPARAM)&dliOpenWith))
  {
    WCHAR szParam[MAX_PATH] = { L'\0' };
    WCHAR wchDirectory[MAX_PATH] = { L'\0' };

    if (StrIsNotEmpty(Globals.CurrentFile)) {
      StringCchCopy(wchDirectory,COUNTOF(wchDirectory),Globals.CurrentFile);
      PathCchRemoveFileSpec(wchDirectory, COUNTOF(wchDirectory));
    }

    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = 0;
    sei.hwnd = hwnd;
    sei.lpVerb = NULL;
    sei.lpFile = dliOpenWith.szFileName;
    sei.lpParameters = szParam;
    sei.lpDirectory = wchDirectory;
    sei.nShow = SW_SHOWNORMAL;

    // resolve links and get short path name
    if (!(PathIsLnkFile(lpstrFile) && PathGetLnkPath(lpstrFile,szParam,COUNTOF(szParam))))
      StringCchCopy(szParam,COUNTOF(szParam),lpstrFile);
    //GetShortPathName(szParam,szParam,sizeof(WCHAR)*COUNTOF(szParam));
    PathQuoteSpaces(szParam);
    result = ShellExecuteEx(&sei);
  }

  return result;

}


//=============================================================================
//
//  FavoritesDlgProc()
//
static INT_PTR CALLBACK FavoritesDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        ResizeDlg_Init(hwnd,Settings.FavoritesDlgSizeX,Settings.FavoritesDlgSizeY,IDC_RESIZEGRIP);

        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_FAVORITESDIR));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_FAVORITESDIR),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,&lvc);
        DirList_Init(GetDlgItem(hwnd,IDC_FAVORITESDIR),NULL);
        DirList_Fill(GetDlgItem(hwnd,IDC_FAVORITESDIR),Settings.FavoritesDir,DL_ALLOBJECTS,NULL,false,Flags.NoFadeHidden,DS_NAME,false);
        DirList_StartIconThread(GetDlgItem(hwnd,IDC_FAVORITESDIR));
        ListView_SetItemState(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETFAVORITESDIR,Globals.hInstance,IDB_OPEN);

        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_DESTROY:
      DirList_Destroy(GetDlgItem(hwnd,IDC_FAVORITESDIR));
      DeleteBitmapButton(hwnd,IDC_GETFAVORITESDIR);

      ResizeDlg_Destroy(hwnd,&Settings.FavoritesDlgSizeX,&Settings.FavoritesDlgSizeY);
      return false;


    case WM_SIZE:
      {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        hdwp = BeginDeferWindowPos(6);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDOK,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDCANCEL,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_FAVORITESDIR,dx,dy,SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_GETFAVORITESDIR,0,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_FAVORITESDESCR,0,dy,SWP_NOSIZE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,LVSCW_AUTOSIZE_USEHEADER);
      }
      return true;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd,lParam);
      return true;


    case WM_NOTIFY:
      {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_FAVORITESDIR)
        {
          switch(pnmh->code)
          {
            case LVN_GETDISPINFO:
              DirList_GetDispInfo(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam,Flags.NoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd,IDC_FAVORITESDIR),lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;
                DialogEnableControl(hwnd,IDOK,(pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FAVORITESDIR))) {
                SendWMCommand(hwnd, IDOK);
              }
              break;
          }
        }
      }
      return true;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_GETFAVORITESDIR:
          {
            if (GetDirectory(hwnd,IDS_MUI_FAVORITES,Settings.FavoritesDir,Settings.FavoritesDir,true))
            {
              DirList_Fill(GetDlgItem(hwnd,IDC_FAVORITESDIR),Settings.FavoritesDir,DL_ALLOBJECTS,NULL,false,Flags.NoFadeHidden,DS_NAME,false);
              DirList_StartIconThread(GetDlgItem(hwnd,IDC_FAVORITESDIR));
              ListView_EnsureVisible(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,false);
              ListView_SetItemState(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);
            }
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_FAVORITESDIR)),1);
          }
          break;


        case IDOK: {
            LPDLITEM lpdli = (LPDLITEM)GetWindowLongPtr(hwnd,DWLP_USER);
            lpdli->mask = DLI_FILENAME | DLI_TYPE;
            lpdli->ntype = DLE_NONE;
            DirList_GetItem(GetDlgItem(hwnd,IDC_FAVORITESDIR),(-1),lpdli);

            if (lpdli->ntype != DLE_NONE)
              EndDialog(hwnd,IDOK);
            else
              SimpleBeep();
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return true;

  }

  return false;

}


//=============================================================================
//
//  FavoritesDlg()
//
bool FavoritesDlg(HWND hwnd,LPWSTR lpstrFile)
{

  DLITEM dliFavorite;
  ZeroMemory(&dliFavorite, sizeof(DLITEM));
  dliFavorite.mask = DLI_FILENAME;

  if (IDOK == ThemedDialogBoxParam(Globals.hLngResContainer,MAKEINTRESOURCE(IDD_MUI_FAVORITES),
                             hwnd,FavoritesDlgProc,(LPARAM)&dliFavorite))
  {
    StringCchCopyN(lpstrFile,MAX_PATH,dliFavorite.szFileName,MAX_PATH);
    return true;
  }
  return false;
}


//=============================================================================
//
//  AddToFavDlgProc()
//
//  Controls: IDC_ADDFAV_FILES Edit
//
static INT_PTR CALLBACK AddToFavDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) {

  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

      ResizeDlg_InitX(hwnd, Settings.AddToFavDlgSizeX, IDC_RESIZEGRIP);

      LPCWSTR const pszName = (LPCWSTR)lParam;
      SendDlgItemMessage(hwnd, IDC_ADDFAV_FILES, EM_LIMITTEXT, MAX_PATH - 1, 0);
      SetDlgItemText(hwnd, IDC_ADDFAV_FILES, pszName);

      CenterDlgInParent(hwnd, NULL);
    }
    return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    break;


  case WM_DESTROY:
    ResizeDlg_Destroy(hwnd, &Settings.AddToFavDlgSizeX, NULL);
    return FALSE;


  case WM_SIZE: {
    int dx;

    ResizeDlg_Size(hwnd, lParam, &dx, NULL);
    HDWP hdwp = BeginDeferWindowPos(5);
    hdwp = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP, dx, 0, SWP_NOSIZE);
    hdwp = DeferCtlPos(hdwp, hwnd, IDOK, dx, 0, SWP_NOSIZE);
    hdwp = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, 0, SWP_NOSIZE);
    hdwp = DeferCtlPos(hdwp, hwnd, IDC_FAVORITESDESCR, dx, 0, SWP_NOMOVE);
    hdwp = DeferCtlPos(hdwp, hwnd, IDC_ADDFAV_FILES, dx, 0, SWP_NOMOVE);
    EndDeferWindowPos(hdwp);
    InvalidateRect(GetDlgItem(hwnd, IDC_FAVORITESDESCR), NULL, TRUE);
  }
  return TRUE;


  case WM_GETMINMAXINFO:
    ResizeDlg_GetMinMaxInfo(hwnd, lParam);
    return TRUE;


  case WM_COMMAND:
    switch (LOWORD(wParam)) 
    {
    case IDC_ADDFAV_FILES:
      DialogEnableControl(hwnd, IDOK, GetWindowTextLength(GetDlgItem(hwnd, IDC_ADDFAV_FILES)));
      break;

    case IDOK:
      {
        LPWSTR pszName = (LPWSTR)GetWindowLongPtr(hwnd, DWLP_USER);
        GetDlgItemText(hwnd, IDC_ADDFAV_FILES, pszName, MAX_PATH - 1);
        EndDialog(hwnd, IDOK);
      }
      break;

    case IDCANCEL:
      EndDialog(hwnd, IDCANCEL);
      break;
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  AddToFavDlg()
//
bool AddToFavDlg(HWND hwnd,LPCWSTR lpszName,LPCWSTR lpszTarget)
{

  INT_PTR iResult;

  WCHAR pszName[MAX_PATH] = { L'\0' };
  StringCchCopy(pszName,COUNTOF(pszName),lpszName);

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_ADDTOFAV),
              hwnd,
              AddToFavDlgProc,(LPARAM)pszName);

  if (iResult == IDOK)
  {
    if (!PathCreateFavLnk(pszName,lpszTarget,Settings.FavoritesDir)) {
      InfoBoxLng(MB_ICONWARNING,NULL,IDS_MUI_FAV_FAILURE);
      return false;
    }
    InfoBoxLng(MB_ICONINFORMATION, NULL, IDS_MUI_FAV_SUCCESS);
    return true;
  }
  return false;
}


//=============================================================================
//
//  FileMRUDlgProc()
//
//
typedef struct tagIconThreadInfo
{
  HWND hwnd;                 // HWND of ListView Control
  HANDLE hThread;            // Thread Handle
  HANDLE hExitThread;        // Flag is set when Icon Thread should terminate
  HANDLE hTerminatedThread;  // Flag is set when Icon Thread has terminated

} ICONTHREADINFO, *LPICONTHREADINFO;

DWORD WINAPI FileMRUIconThread(LPVOID lpParam) {

  WCHAR tch[MAX_PATH] = { L'\0' };
  DWORD dwFlags = SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED;

  LPICONTHREADINFO lpit = (LPICONTHREADINFO)lpParam;
  ResetEvent(lpit->hTerminatedThread);

  HWND hwnd = lpit->hwnd;
  int iMaxItem = ListView_GetItemCount(hwnd);

  (void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);

  int iItem = 0;
  while (iItem < iMaxItem && WaitForSingleObject(lpit->hExitThread,0) != WAIT_OBJECT_0) {

    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(LV_ITEM));

    lvi.mask = LVIF_TEXT;
    lvi.pszText = tch;
    lvi.cchTextMax = COUNTOF(tch);
    lvi.iItem = iItem;

    SHFILEINFO shfi;
    ZeroMemory(&shfi, sizeof(SHFILEINFO));

    if (ListView_GetItem(hwnd,&lvi)) 
    {
      DWORD dwAttr = 0;
      if (PathIsUNC(tch) || !PathFileExists(tch)) {
        dwFlags |= SHGFI_USEFILEATTRIBUTES;
        dwAttr = FILE_ATTRIBUTE_NORMAL;
        shfi.dwAttributes = 0;
        SHGetFileInfo(PathFindFileName(tch),dwAttr,&shfi,sizeof(SHFILEINFO),dwFlags);
      }
      else {
        shfi.dwAttributes = SFGAO_LINK | SFGAO_SHARE;
        SHGetFileInfo(tch,dwAttr,&shfi,sizeof(SHFILEINFO),dwFlags);
      }

      lvi.mask = LVIF_IMAGE;
      lvi.iImage = shfi.iIcon;
      lvi.stateMask = 0;
      lvi.state = 0;

      if (shfi.dwAttributes & SFGAO_LINK) {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_OVERLAYMASK;
        lvi.state |= INDEXTOOVERLAYMASK(2);
      }

      if (shfi.dwAttributes & SFGAO_SHARE) {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_OVERLAYMASK;
        lvi.state |= INDEXTOOVERLAYMASK(1);
      }

      if (PathIsUNC(tch))
        dwAttr = FILE_ATTRIBUTE_NORMAL;
      else
        dwAttr = GetFileAttributes(tch);

      if (!Flags.NoFadeHidden &&
          dwAttr != INVALID_FILE_ATTRIBUTES &&
          dwAttr & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
        lvi.mask |= LVIF_STATE;
        lvi.stateMask |= LVIS_CUT;
        lvi.state |= LVIS_CUT;
      }

      lvi.iSubItem = 0;
      ListView_SetItem(hwnd,&lvi);
    }
    iItem++;
  }

  CoUninitialize();

  SetEvent(lpit->hTerminatedThread);
  lpit->hThread = NULL;

  ExitThread(0);
  //return(0);
}

#define IDC_FILEMRU_UPDATE_VIEW (WM_USER+1)

static INT_PTR CALLBACK FileMRUDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        SHFILEINFO shfi;
        ZeroMemory(&shfi, sizeof(SHFILEINFO));
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        LPICONTHREADINFO lpit = (LPICONTHREADINFO)AllocMem(sizeof(ICONTHREADINFO),HEAP_ZERO_MEMORY);
        if (lpit) {
          SetProp(hwnd, L"it", (HANDLE)lpit);
          lpit->hwnd = GetDlgItem(hwnd, IDC_FILEMRU);
          lpit->hThread = NULL;
          lpit->hExitThread = CreateEvent(NULL, true, false, NULL);
          lpit->hTerminatedThread = CreateEvent(NULL, true, true, NULL);
        }
        ResizeDlg_Init(hwnd,Settings.FileMRUDlgSizeX,Settings.FileMRUDlgSizeY,IDC_RESIZEGRIP);

        ListView_SetImageList(GetDlgItem(hwnd,IDC_FILEMRU),
          (HIMAGELIST)SHGetFileInfo(L"C:\\",FILE_ATTRIBUTE_DIRECTORY,
            &shfi,sizeof(SHFILEINFO),SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
          LVSIL_SMALL);

        ListView_SetImageList(GetDlgItem(hwnd,IDC_FILEMRU),
          (HIMAGELIST)SHGetFileInfo(L"C:\\",FILE_ATTRIBUTE_DIRECTORY,
            &shfi,sizeof(SHFILEINFO),SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
          LVSIL_NORMAL);

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_FILEMRU));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_FILEMRU),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_FILEMRU),0,&lvc);

        // Update view
        SendWMCommand(hwnd, IDC_FILEMRU_UPDATE_VIEW);

        CheckDlgButton(hwnd, IDC_SAVEMRU, SetBtn(Settings.SaveRecentFiles));
        CheckDlgButton(hwnd, IDC_PRESERVECARET, SetBtn(Settings.PreserveCaretPos));
        CheckDlgButton(hwnd, IDC_REMEMBERSEARCHPATTERN, SetBtn(Settings.SaveFindReplace));

        //if (!Settings.SaveRecentFiles) {
        //  DialogEnableWindow(hwnd,IDC_PRESERVECARET, false);
        //  DialogEnableWindow(hwnd,IDC_REMEMBERSEARCHPATTERN, false);
        //}

        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_DESTROY:
      {
        LPICONTHREADINFO lpit = (LPVOID)GetProp(hwnd,L"it");
        SetEvent(lpit->hExitThread);
        while (WaitForSingleObject(lpit->hTerminatedThread,0) != WAIT_OBJECT_0) {
          MSG msg;
          if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
        }
        CloseHandle(lpit->hExitThread);
        CloseHandle(lpit->hTerminatedThread);
        lpit->hThread = NULL;
        RemoveProp(hwnd,L"it");
        FreeMem(lpit);

        Settings.SaveFindReplace = IsButtonChecked(hwnd, IDC_REMEMBERSEARCHPATTERN);
        Settings.PreserveCaretPos = IsButtonChecked(hwnd, IDC_PRESERVECARET);
        Settings.SaveRecentFiles  = IsButtonChecked(hwnd, IDC_SAVEMRU);

        ResizeDlg_Destroy(hwnd,&Settings.FileMRUDlgSizeX,&Settings.FileMRUDlgSizeY);
      }
      return false;


    case WM_SIZE:
      {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        hdwp = BeginDeferWindowPos(5);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDOK,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDCANCEL,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_REMOVE,dx,dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_FILEMRU,dx,dy,SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_SAVEMRU,0,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_PRESERVECARET, 0, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_REMEMBERSEARCHPATTERN, 0, dy, SWP_NOSIZE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_FILEMRU),0,LVSCW_AUTOSIZE_USEHEADER);
      }
      return true;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd,lParam);
      return true;


    case WM_NOTIFY: {
      if (((LPNMHDR)(lParam))->idFrom == IDC_FILEMRU) {

      switch (((LPNMHDR)(lParam))->code) {

        case NM_DBLCLK:
          SendWMCommand(hwnd, IDOK);
          break;


        case LVN_GETDISPINFO: {
            /*
            LV_DISPINFO *lpdi = (LPVOID)lParam;

            if (lpdi->item.mask & LVIF_IMAGE) {

              WCHAR tch[MAX_PATH] = { L'\0' };
              LV_ITEM lvi;
              SHFILEINFO shfi;
              DWORD dwFlags = SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED;
              DWORD dwAttr  = 0;

              ZeroMemory(&lvi,sizeof(LV_ITEM));

              lvi.mask = LVIF_TEXT;
              lvi.pszText = tch;
              lvi.cchTextMax = COUNTOF(tch);
              lvi.iItem = lpdi->item.iItem;

              ListView_GetItem(GetDlgItem(hwnd,IDC_FILEMRU),&lvi);

              if (!PathFileExists(tch)) {
                dwFlags |= SHGFI_USEFILEATTRIBUTES;
                dwAttr = FILE_ATTRIBUTE_NORMAL;
                shfi.dwAttributes = 0;
                SHGetFileInfo(PathFindFileName(tch),dwAttr,&shfi,sizeof(SHFILEINFO),dwFlags);
              }

              else {
                shfi.dwAttributes = SFGAO_LINK | SFGAO_SHARE;
                SHGetFileInfo(tch,dwAttr,&shfi,sizeof(SHFILEINFO),dwFlags);
              }

              lpdi->item.iImage = shfi.iIcon;
              lpdi->item.mask |= LVIF_DI_SETITEM;

              lpdi->item.stateMask = 0;
              lpdi->item.state = 0;

              if (shfi.dwAttributes & SFGAO_LINK) {
                lpdi->item.mask |= LVIF_STATE;
                lpdi->item.stateMask |= LVIS_OVERLAYMASK;
                lpdi->item.state |= INDEXTOOVERLAYMASK(2);
              }

              if (shfi.dwAttributes & SFGAO_SHARE) {
                lpdi->item.mask |= LVIF_STATE;
                lpdi->item.stateMask |= LVIS_OVERLAYMASK;
                lpdi->item.state |= INDEXTOOVERLAYMASK(1);
              }

              dwAttr = GetFileAttributes(tch);

              if (!Flags.NoFadeHidden &&
                  dwAttr != INVALID_FILE_ATTRIBUTES &&
                  dwAttr & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
                lpdi->item.mask |= LVIF_STATE;
                lpdi->item.stateMask |= LVIS_CUT;
                lpdi->item.state |= LVIS_CUT;
              }
            }
            */
          }
          break;


        case LVN_ITEMCHANGED:
        case LVN_DELETEITEM:
            {
              UINT cnt = ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FILEMRU));
              DialogEnableControl(hwnd, IDOK, (cnt > 0));
              DialogEnableControl(hwnd, IDC_REMOVE, (cnt > 0));
            }
            break;
          }
        }
      }

      return true;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {
        case IDC_FILEMRU_UPDATE_VIEW:
          {
            int i;
            WCHAR tch[MAX_PATH] = { L'\0' };
            LV_ITEM lvi;
            SHFILEINFO shfi;
            ZeroMemory(&shfi, sizeof(SHFILEINFO));

            DWORD dwtid;
            LPICONTHREADINFO lpit = (LPVOID)GetProp(hwnd,L"it");

            SetEvent(lpit->hExitThread);
            while (WaitForSingleObject(lpit->hTerminatedThread,0) != WAIT_OBJECT_0) {
              MSG msg;
              if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
              }
            }
            ResetEvent(lpit->hExitThread);
            SetEvent(lpit->hTerminatedThread);
            lpit->hThread = NULL;

            ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_FILEMRU));

            ZeroMemory(&lvi,sizeof(LV_ITEM));
            lvi.mask = LVIF_TEXT | LVIF_IMAGE;

            SHGetFileInfo(L"Icon",FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),
              SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);

            lvi.iImage = shfi.iIcon;

            for (i = 0; i < MRU_Count(Globals.pFileMRU); i++) {
              MRU_Enum(Globals.pFileMRU,i,tch,COUNTOF(tch));
              PathAbsoluteFromApp(tch,NULL,0,true);
              //  SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_ADDSTRING,0,(LPARAM)tch); }
              //  SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_SETCARETINDEX,0,false);
              lvi.iItem = i;
              lvi.pszText = tch;
              ListView_InsertItem(GetDlgItem(hwnd,IDC_FILEMRU),&lvi);
            }

            UINT cnt = ListView_GetItemCount(GetDlgItem(hwnd, IDC_FILEMRU));
            if (cnt > 0) {
              UINT idx = ListView_GetTopIndex(GetDlgItem(hwnd, IDC_FILEMRU));
              ListView_SetItemState(GetDlgItem(hwnd, IDC_FILEMRU), idx, LVIS_FOCUSED, LVIS_FOCUSED);
              ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_FILEMRU), idx, LVSCW_AUTOSIZE_USEHEADER);
              ListView_SetItemState(GetDlgItem(hwnd, IDC_FILEMRU), idx, LVIS_SELECTED, LVIS_SELECTED);
            }

            lpit->hThread = CreateThread(NULL,0,FileMRUIconThread,(LPVOID)lpit,0,&dwtid);
          }
          break;

        case IDC_FILEMRU:
          break;

        case IDOK:
        case IDC_REMOVE:
          {
            WCHAR tchFileName[MAX_PATH] = { L'\0' };
            //int  iItem;

            //if ((iItem = SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_GETCURSEL,0,0)) != LB_ERR)

            UINT cnt = ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FILEMRU));
            if (cnt > 0)
            {
              //SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_GETTEXT,(WPARAM)iItem,(LPARAM)tch);
              LV_ITEM lvi;
              ZeroMemory(&lvi,sizeof(LV_ITEM));

              lvi.mask = LVIF_TEXT;
              lvi.pszText = tchFileName;
              lvi.cchTextMax = COUNTOF(tchFileName);
              lvi.iItem = ListView_GetNextItem(GetDlgItem(hwnd,IDC_FILEMRU),-1,LVNI_ALL | LVNI_SELECTED);

              ListView_GetItem(GetDlgItem(hwnd,IDC_FILEMRU),&lvi);

              PathUnquoteSpaces(tchFileName);

              if (!PathFileExists(tchFileName) || (LOWORD(wParam) == IDC_REMOVE)) {

                // don't remove myself
                int iCur = 0;
                if (!MRU_FindFile(Globals.pFileMRU, Globals.CurrentFile, &iCur)) {
                  iCur = -1;
                }

                // Ask...
                INT_PTR const answer = (LOWORD(wParam) == IDOK) ? 
                  InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_ERR_MRUDLG) 
                  : ((iCur == lvi.iItem) ? IDNO : IDYES);

                if ((IDOK == answer) || (IDYES == answer)) {

                  MRU_Delete(Globals.pFileMRU,lvi.iItem);

                  //SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_DELETESTRING,(WPARAM)iItem,0);
                  //ListView_DeleteItem(GetDlgItem(hwnd,IDC_FILEMRU),lvi.iItem);
                  // must use IDM_VIEW_REFRESH, index might change...
                  SendWMCommand(hwnd, IDC_FILEMRU_UPDATE_VIEW);

                  //DialogEnableWindow(hwnd,IDOK,
                  //  (LB_ERR != SendDlgItemMessage(hwnd,IDC_GOTO,LB_GETCURSEL,0,0)));

                  cnt = ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FILEMRU));
                  DialogEnableControl(hwnd, IDOK, (cnt > 0));
                  DialogEnableControl(hwnd, IDC_REMOVE, (cnt > 0));
                }
              }

              else {
                StringCchCopy((LPWSTR)GetWindowLongPtr(hwnd,DWLP_USER),MAX_PATH,tchFileName);
                EndDialog(hwnd,IDOK);
              }
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return true;

  }

  return false;

}


//=============================================================================
//
//  FileMRUDlg()
//
//
bool FileMRUDlg(HWND hwnd,LPWSTR lpstrFile)
{
  if (IDOK == ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_FILEMRU),
                                   hwnd, FileMRUDlgProc, (LPARAM)lpstrFile)) {
    return true;
  }
  return false;
}


//=============================================================================
//
//  ChangeNotifyDlgProc()
//
//  Controls: 100 Radio Button (None)
//            101 Radio Button (Display Message)
//            102 Radio Button (Auto-Reload)
//            103 Check Box    (Reset on New)
//

static INT_PTR CALLBACK ChangeNotifyDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) {
  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

      CheckRadioButton(hwnd, 100, 102, 100 + Settings.FileWatchingMode);
      if (Settings.ResetFileWatching) {
        CheckDlgButton(hwnd, 103, BST_CHECKED);
      }
      CenterDlgInParent(hwnd, NULL);
    }
    return true;

  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (IsButtonChecked(hwnd, 100)) {
        Settings.FileWatchingMode = FWM_DONT_CARE;
      }
      else if (IsButtonChecked(hwnd, 101)) {
        Settings.FileWatchingMode = FWM_MSGBOX;
      }
      else {
        Settings.FileWatchingMode = FWM_AUTORELOAD;
      }
      if (!FileWatching.MonitoringLog) { FileWatching.FileWatchingMode = Settings.FileWatchingMode; }

      Settings.ResetFileWatching = IsButtonChecked(hwnd, 103);

      if (!FileWatching.MonitoringLog) { FileWatching.ResetFileWatching = Settings.ResetFileWatching; }

      if (FileWatching.MonitoringLog) { PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL); }

      EndDialog(hwnd, IDOK);
      break;

    case IDCANCEL:
      EndDialog(hwnd, IDCANCEL);
      break;
    }
    return true;
  }

  return false;
}


//=============================================================================
//
//  ChangeNotifyDlg()
//
bool ChangeNotifyDlg(HWND hwnd)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCEW(IDD_MUI_CHANGENOTIFY),
              hwnd,
              ChangeNotifyDlgProc,
              0);

  return (iResult == IDOK) ? true : false;

}


//=============================================================================
//
//  ColumnWrapDlgProc()
//
//  Controls: Edit IDC_COLUMNWRAP
//
static INT_PTR CALLBACK ColumnWrapDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) {
  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

      UINT const uiNumber = *((UINT*)lParam);
      SetDlgItemInt(hwnd, IDC_COLUMNWRAP, uiNumber, false);
      SendDlgItemMessage(hwnd, IDC_COLUMNWRAP, EM_LIMITTEXT, 15, 0);
      CenterDlgInParent(hwnd, NULL);
    }
    return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;


  case WM_COMMAND:

    switch (LOWORD(wParam)) {

    case IDOK:
      {
        BOOL fTranslated;
        UINT const iNewNumber = GetDlgItemInt(hwnd, IDC_COLUMNWRAP, &fTranslated, FALSE);
        if (fTranslated) {
          UINT* piNumber = (UINT*)GetWindowLongPtr(hwnd, DWLP_USER);
          *piNumber = iNewNumber;

          EndDialog(hwnd, IDOK);
        }
        else
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_COLUMNWRAP)), 1);
      }
      break;


    case IDCANCEL:
      EndDialog(hwnd, IDCANCEL);
      break;

    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  ColumnWrapDlg()
//
bool ColumnWrapDlg(HWND hwnd,UINT uidDlg, UINT *iNumber)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCE(uidDlg),
              hwnd,
              ColumnWrapDlgProc,(LPARAM)iNumber);

  return (iResult == IDOK) ? true : false;

}


//=============================================================================
//
//  WordWrapSettingsDlgProc()
//
//  Controls: 100 Combo
//            101 Combo
//            102 Combo
//            103 Combo
//            200 Text
//            201 Text
//            202 Text
//            203 Text
//
static INT_PTR CALLBACK WordWrapSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) {

  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

      WCHAR tch[512];
      for (int i = 0; i < 4; i++) {
        GetDlgItemText(hwnd, 200 + i, tch, COUNTOF(tch));
        StringCchCat(tch, COUNTOF(tch), L"|");
        WCHAR* p1 = tch;
        WCHAR* p2 = StrChr(p1, L'|');
        while (p2) {
          *p2++ = L'\0';
          if (*p1)
            SendDlgItemMessage(hwnd, 100 + i, CB_ADDSTRING, 0, (LPARAM)p1);
          p1 = p2;
          p2 = StrChr(p1, L'|');
        }
        SendDlgItemMessage(hwnd, 100 + i, CB_SETEXTENDEDUI, true, 0);
      }
      SendDlgItemMessage(hwnd, 100, CB_SETCURSEL, (WPARAM)Settings.WordWrapIndent, 0);
      SendDlgItemMessage(hwnd, 101, CB_SETCURSEL, (WPARAM)(Settings.ShowWordWrapSymbols ? Settings.WordWrapSymbols % 10 : 0), 0);
      SendDlgItemMessage(hwnd, 102, CB_SETCURSEL, (WPARAM)(Settings.ShowWordWrapSymbols ? ((Settings.WordWrapSymbols % 100) - (Settings.WordWrapSymbols % 10)) / 10 : 0), 0);
      SendDlgItemMessage(hwnd, 103, CB_SETCURSEL, (WPARAM)Settings.WordWrapMode, 0);

      CenterDlgInParent(hwnd, NULL);
    }
    return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;


  case WM_COMMAND:

    switch (LOWORD(wParam)) {

    case IDOK:
      {
        int iSel = (int)SendDlgItemMessage(hwnd, 100, CB_GETCURSEL, 0, 0);
        Settings.WordWrapIndent = iSel;

        Settings.ShowWordWrapSymbols = false;
        iSel = (int)SendDlgItemMessage(hwnd, 101, CB_GETCURSEL, 0, 0);
        int iSel2 = (int)SendDlgItemMessage(hwnd, 102, CB_GETCURSEL, 0, 0);
        if (iSel > 0 || iSel2 > 0) {
          Settings.ShowWordWrapSymbols = true;
          Settings.WordWrapSymbols = iSel + iSel2 * 10;
        }

        iSel = (int)SendDlgItemMessage(hwnd, 103, CB_GETCURSEL, 0, 0);
        Settings.WordWrapMode = iSel;

        EndDialog(hwnd, IDOK);
      }
      break;


    case IDCANCEL:
      EndDialog(hwnd, IDCANCEL);
      break;

    }

    return true;

  }
  return false;
}


//=============================================================================
//
//  WordWrapSettingsDlg()
//
bool WordWrapSettingsDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCE(uidDlg),
              hwnd,
              WordWrapSettingsDlgProc,(LPARAM)iNumber);

  return (iResult == IDOK) ? true : false;

}


//=============================================================================
//
//  LongLineSettingsDlgProc()
//
//  Controls: 100 Edit
//            101 Radio1
//            102 Radio2
//
static INT_PTR CALLBACK LongLineSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) {

  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

      // TODO: @@@  set GUI IDS for hard coded numbers
      UINT const iNumber = *((UINT*)lParam);
      SetDlgItemInt(hwnd, 100, iNumber, false);
      SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, 15, 0);

      if (Settings.LongLineMode == EDGE_BACKGROUND) {
        CheckRadioButton(hwnd, 101, 102, 102);
      }
      else {
        CheckRadioButton(hwnd, 101, 102, 101);
      }
      CenterDlgInParent(hwnd, NULL);

    }
    return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;


  case WM_COMMAND:

    switch (LOWORD(wParam)) {

    case IDOK:
      {
        BOOL fTranslated;
        UINT const iNewNumber = GetDlgItemInt(hwnd, 100, &fTranslated, FALSE);

        if (fTranslated) {
          UINT* piNumber = (UINT*)GetWindowLongPtr(hwnd, DWLP_USER);
          *piNumber = iNewNumber;
          Settings.LongLineMode = IsButtonChecked(hwnd, 101) ? EDGE_LINE : EDGE_BACKGROUND;
          EndDialog(hwnd, IDOK);
        }
        else {
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, 100)), 1);
        }
      }
      break;


    case IDCANCEL:
      EndDialog(hwnd, IDCANCEL);
      break;

    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  LongLineSettingsDlg()
//
bool LongLineSettingsDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCE(uidDlg),
              hwnd,
              LongLineSettingsDlgProc,(LPARAM)iNumber);

  return (iResult == IDOK) ? true : false;

}


//=============================================================================
//
//  TabSettingsDlgProc()
//
//  Controls: 100 Edit
//            101 Edit
//            102 Check
//            103 Check
//            104 Check
//

static INT_PTR CALLBACK TabSettingsDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        SetDlgItemInt(hwnd,IDC_TAB_WIDTH,Globals.fvCurFile.iTabWidth,false);
        SendDlgItemMessage(hwnd,IDC_TAB_WIDTH,EM_LIMITTEXT,15,0);

        SetDlgItemInt(hwnd,IDC_INDENT_DEPTH, Globals.fvCurFile.iIndentWidth,false);
        SendDlgItemMessage(hwnd,IDC_INDENT_DEPTH,EM_LIMITTEXT,15,0);

        CheckDlgButton(hwnd,IDC_TAB_AS_SPC, SetBtn(Globals.fvCurFile.bTabsAsSpaces));
        CheckDlgButton(hwnd,IDC_TAB_INDENTS, SetBtn(Globals.fvCurFile.bTabIndents));
        CheckDlgButton(hwnd,IDC_BACKTAB_INDENTS, SetBtn(Settings.BackspaceUnindents));
        CheckDlgButton(hwnd,IDC_WARN_INCONSISTENT_INDENTS, SetBtn(Settings.WarnInconsistentIndents));
        CheckDlgButton(hwnd,IDC_AUTO_DETECT_INDENTS, SetBtn(Settings.AutoDetectIndentSettings));

        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {
        case IDOK: 
          {
            BOOL fTranslated1, fTranslated2;
            int const _iNewTabWidth = GetDlgItemInt(hwnd, IDC_TAB_WIDTH, &fTranslated1, FALSE);
            int const _iNewIndentWidth = GetDlgItemInt(hwnd, IDC_INDENT_DEPTH, &fTranslated2, FALSE);

            if (fTranslated1 && fTranslated2) 
            {
              Settings.TabWidth = _iNewTabWidth;
              Globals.fvCurFile.iTabWidth = _iNewTabWidth;

              Settings.IndentWidth = _iNewIndentWidth;
              Globals.fvCurFile.iIndentWidth = _iNewIndentWidth;

              bool const _bTabsAsSpaces = IsButtonChecked(hwnd, IDC_TAB_AS_SPC);
              Settings.TabsAsSpaces = _bTabsAsSpaces;
              Globals.fvCurFile.bTabsAsSpaces = _bTabsAsSpaces;

              bool const _bTabIndents = IsButtonChecked(hwnd, IDC_TAB_INDENTS);
              Settings.TabIndents = _bTabIndents;
              Globals.fvCurFile.bTabIndents = _bTabIndents;

              Settings.BackspaceUnindents = IsButtonChecked(hwnd, IDC_BACKTAB_INDENTS);
              Settings.WarnInconsistentIndents = IsButtonChecked(hwnd, IDC_WARN_INCONSISTENT_INDENTS);
              Settings.AutoDetectIndentSettings = IsButtonChecked(hwnd, IDC_AUTO_DETECT_INDENTS);
              EndDialog(hwnd, IDOK);
            }
            else {
              PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, (fTranslated1) ? IDC_INDENT_DEPTH : IDC_TAB_WIDTH)), 1);
            }
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

        default:
          break;
      }
      return true;
  }
  return false;
}


//=============================================================================
//
//  TabSettingsDlg()
//
bool TabSettingsDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCE(uidDlg),
              hwnd,
              TabSettingsDlgProc,(LPARAM)iNumber);

  return (iResult == IDOK) ? true : false;

}


//=============================================================================
//
//  SelectDefEncodingDlgProc()
//
//
typedef struct encodedlg {
  bool       bRecodeOnly;
  cpi_enc_t  idEncoding;
  int        cxDlg;
  int        cyDlg;
} 
ENCODEDLG, *PENCODEDLG;

static INT_PTR CALLBACK SelectDefEncodingDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static cpi_enc_t s_iEnc;
  static bool s_bUseAsFallback;
  static bool s_bLoadASCIIasUTF8;

  switch (umsg)
  {
  case WM_INITDIALOG:
  {
    SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
    if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

    PENCODEDLG const pdd = (PENCODEDLG)lParam;
    HBITMAP hbmp = LoadImage(Globals.hInstance, MAKEINTRESOURCE(IDB_ENCODING), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hbmp = ResizeImageForCurrentDPI(hbmp);

    HIMAGELIST himl = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
    ImageList_AddMasked(himl, hbmp, CLR_DEFAULT);
    DeleteObject(hbmp);
    SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CBEM_SETIMAGELIST, 0, (LPARAM)himl);
    SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CB_SETEXTENDEDUI, true, 0);

    Encoding_AddToComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), pdd->idEncoding, 0);

    Encoding_GetFromComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), &s_iEnc);
    s_bLoadASCIIasUTF8 = Settings.LoadASCIIasUTF8;
    s_bUseAsFallback = Encoding_IsASCII(s_iEnc) ? Settings.UseDefaultForFileEncoding : false;

    CheckDlgButton(hwnd, IDC_USEASREADINGFALLBACK, SetBtn(s_bUseAsFallback));
    CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(s_bLoadASCIIasUTF8));
    CheckDlgButton(hwnd, IDC_RELIABLE_DETECTION_RES, SetBtn(Settings.UseReliableCEDonly));
    CheckDlgButton(hwnd, IDC_NFOASOEM, SetBtn(Settings.LoadNFOasOEM));
    CheckDlgButton(hwnd, IDC_ENCODINGFROMFILEVARS, SetBtn(!Settings.NoEncodingTags));
    CheckDlgButton(hwnd, IDC_NOUNICODEDETECTION, SetBtn(!Settings.SkipUnicodeDetection));
    CheckDlgButton(hwnd, IDC_NOANSICPDETECTION, SetBtn(!Settings.SkipANSICodePageDetection));

    DialogEnableControl(hwnd, IDC_USEASREADINGFALLBACK, Encoding_IsASCII(s_iEnc));

    CenterDlgInParent(hwnd, NULL);
  }
  return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
    return true;


  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_ASCIIASUTF8:
      if (s_iEnc != CPI_UTF8) {
        s_bLoadASCIIasUTF8 = IsButtonChecked(hwnd, IDC_ASCIIASUTF8);
      }
      break;

    case IDC_USEASREADINGFALLBACK:
      if (s_iEnc != CPI_ANSI_DEFAULT) {
        s_bUseAsFallback = IsButtonChecked(hwnd, IDC_USEASREADINGFALLBACK);
      }
      if (s_iEnc == CPI_UTF8) {
        if (s_bUseAsFallback) {
          CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(true));
          DialogEnableControl(hwnd, IDC_ASCIIASUTF8, false);
        }
        else
        {
          DialogEnableControl(hwnd, IDC_ASCIIASUTF8, true);
          CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(s_bLoadASCIIasUTF8));
        }
      }
      break;

    case IDC_ENCODINGLIST:
    {
      Encoding_GetFromComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), &s_iEnc);

      s_bUseAsFallback = Encoding_IsASCII(s_iEnc) ? Settings.UseDefaultForFileEncoding : false;

      if (s_iEnc == CPI_UTF8) {
        if (s_bUseAsFallback) {
          DialogEnableControl(hwnd, IDC_ASCIIASUTF8, false);
          CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(true));
        }
        DialogEnableControl(hwnd, IDC_USEASREADINGFALLBACK, Encoding_IsASCII(s_iEnc));
        CheckDlgButton(hwnd, IDC_USEASREADINGFALLBACK, SetBtn(s_bUseAsFallback));
      }
      else if (s_iEnc == CPI_ANSI_DEFAULT) {
        DialogEnableControl(hwnd, IDC_ASCIIASUTF8, true);
        CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(s_bLoadASCIIasUTF8));
        s_bUseAsFallback = true;
        DialogEnableControl(hwnd, IDC_USEASREADINGFALLBACK, false);
        CheckDlgButton(hwnd, IDC_USEASREADINGFALLBACK, SetBtn(s_bUseAsFallback));
      }
      else {
        s_bUseAsFallback = Encoding_IsASCII(s_iEnc) ? Settings.UseDefaultForFileEncoding : false;
        DialogEnableControl(hwnd, IDC_ASCIIASUTF8, true);
        CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(s_bLoadASCIIasUTF8));
        DialogEnableControl(hwnd, IDC_USEASREADINGFALLBACK, Encoding_IsASCII(s_iEnc));
        CheckDlgButton(hwnd, IDC_USEASREADINGFALLBACK, SetBtn(s_bUseAsFallback));
      }
    }
    break;

    case IDOK: {
      PENCODEDLG pdd = (PENCODEDLG)GetWindowLongPtr(hwnd, DWLP_USER);
      if (Encoding_GetFromComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), &pdd->idEncoding)) {
        if (pdd->idEncoding < 0) {
          InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_ENCODINGNA);
          EndDialog(hwnd, IDCANCEL);
        }
        else {
          Settings.UseDefaultForFileEncoding = IsButtonChecked(hwnd, IDC_USEASREADINGFALLBACK);
          Settings.LoadASCIIasUTF8 = IsButtonChecked(hwnd, IDC_ASCIIASUTF8);
          Settings.UseReliableCEDonly = IsButtonChecked(hwnd, IDC_RELIABLE_DETECTION_RES);
          Settings.LoadNFOasOEM = IsButtonChecked(hwnd, IDC_NFOASOEM);
          Settings.NoEncodingTags = !IsButtonChecked(hwnd, IDC_ENCODINGFROMFILEVARS);
          Settings.SkipUnicodeDetection = !IsButtonChecked(hwnd, IDC_NOUNICODEDETECTION);
          Settings.SkipANSICodePageDetection = !IsButtonChecked(hwnd, IDC_NOANSICPDETECTION);
          EndDialog(hwnd, IDOK);
        }
      }
      else {
        PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_ENCODINGLIST)), 1);
      }
    }
    break;

    case IDCANCEL:
      EndDialog(hwnd, IDCANCEL);
      break;
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  SelectDefEncodingDlg()
//
bool SelectDefEncodingDlg(HWND hwnd, cpi_enc_t* pidREncoding)
{

  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = false;
  dd.idEncoding = *pidREncoding;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_DEFENCODING),
              hwnd,
              SelectDefEncodingDlgProc,
              (LPARAM)&dd);

  if (iResult == IDOK) {
    *pidREncoding = dd.idEncoding;
    return true;
  }
  return false;
}


//=============================================================================
//
//  SelectEncodingDlgProc()
//
//
static INT_PTR CALLBACK SelectEncodingDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static HWND hwndLV;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        PENCODEDLG const pdd = (PENCODEDLG)lParam;
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };
        ResizeDlg_Init(hwnd,pdd->cxDlg,pdd->cyDlg,IDC_RESIZEGRIP);

        hwndLV = GetDlgItem(hwnd,IDC_ENCODINGLIST);

        HBITMAP hbmp = LoadImage(Globals.hInstance,MAKEINTRESOURCE(IDB_ENCODING),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
        hbmp = ResizeImageForCurrentDPI(hbmp);

        HIMAGELIST himl = ImageList_Create(16,16,ILC_COLOR32|ILC_MASK,0,0);
        ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
        DeleteObject(hbmp);
        ListView_SetImageList(GetDlgItem(hwnd,IDC_ENCODINGLIST),himl,LVSIL_SMALL);

        //SetExplorerTheme(hwndLV);
        ListView_SetExtendedListViewStyle(hwndLV,/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV,0,&lvc);

        Encoding_AddToListView(hwndLV,pdd->idEncoding,pdd->bRecodeOnly);

        ListView_SetColumnWidth(hwndLV,0,LVSCW_AUTOSIZE_USEHEADER);

        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_DESTROY: 
      {
        PENCODEDLG pdd = (PENCODEDLG)GetWindowLongPtr(hwnd, DWLP_USER);
        ResizeDlg_Destroy(hwnd, &pdd->cxDlg, &pdd->cyDlg);
      }
      return false;


    case WM_SIZE:
      {
        int dx, dy;
        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        HDWP hdwp = BeginDeferWindowPos(4);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDOK,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDCANCEL,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_ENCODINGLIST,dx,dy,SWP_NOMOVE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_ENCODINGLIST),0,LVSCW_AUTOSIZE_USEHEADER);
      }
      return true;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd,lParam);
      return true;


    case WM_NOTIFY: {
        if (((LPNMHDR)(lParam))->idFrom == IDC_ENCODINGLIST) {

        switch (((LPNMHDR)(lParam))->code) {

          case NM_DBLCLK:
            SendWMCommand(hwnd, IDOK);
            break;

          case LVN_ITEMCHANGED:
          case LVN_DELETEITEM: {
              int i = ListView_GetNextItem(hwndLV,-1,LVNI_ALL | LVNI_SELECTED);
              DialogEnableControl(hwnd,IDOK,i != -1);
            }
            break;
          }
        }
      }
      return true;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDOK:
        {
          PENCODEDLG pdd = (PENCODEDLG)GetWindowLongPtr(hwnd, DWLP_USER);
          if (Encoding_GetFromListView(hwndLV, &pdd->idEncoding)) {
            if (pdd->idEncoding < 0) {
              InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_ENCODINGNA);
              EndDialog(hwnd, IDCANCEL);
            }
            else {
              EndDialog(hwnd, IDOK);
            }
          }
          else {
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_ENCODINGLIST)), 1);
          }
        }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return true;

  }

  return false;

}


//=============================================================================
//
//  SelectEncodingDlg()
//
bool SelectEncodingDlg(HWND hwnd, cpi_enc_t* pidREncoding)
{

  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = false;
  dd.idEncoding = *pidREncoding;
  dd.cxDlg = Settings.EncodingDlgSizeX;
  dd.cyDlg = Settings.EncodingDlgSizeY;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_ENCODING),
              hwnd,
              SelectEncodingDlgProc,
              (LPARAM)&dd);

  Settings.EncodingDlgSizeX = dd.cxDlg;
  Settings.EncodingDlgSizeY = dd.cyDlg;

  if (iResult == IDOK) {
    *pidREncoding = dd.idEncoding;
    return true;
  }
  return false;
}


//=============================================================================
//
//  RecodeDlg()
//
bool RecodeDlg(HWND hwnd, cpi_enc_t* pidREncoding)
{

  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = true;
  dd.idEncoding = *pidREncoding;
  dd.cxDlg = Settings.RecodeDlgSizeX;
  dd.cyDlg = Settings.RecodeDlgSizeY;

  iResult = ThemedDialogBoxParam(
              Globals.hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_RECODE),
              hwnd,
              SelectEncodingDlgProc,
              (LPARAM)&dd);

  Settings.RecodeDlgSizeX = dd.cxDlg;
  Settings.RecodeDlgSizeY = dd.cyDlg;

  if (iResult == IDOK) {
    *pidREncoding = dd.idEncoding;
    return true;
  }
  return false;
}


//=============================================================================
//
//  SelectDefLineEndingDlgProc()
//
//
static INT_PTR CALLBACK SelectDefLineEndingDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        int const iOption = *((int*)lParam);

        // Load options
        WCHAR wch[256] = { L'\0' };
        for (int i = 0; i < 3; i++) {
          GetLngString(IDS_EOL_WIN+i,wch,COUNTOF(wch));
          SendDlgItemMessage(hwnd, IDC_EOLMODELIST,CB_ADDSTRING,0,(LPARAM)wch);
        }

        SendDlgItemMessage(hwnd, IDC_EOLMODELIST,CB_SETCURSEL,iOption,0);
        SendDlgItemMessage(hwnd, IDC_EOLMODELIST,CB_SETEXTENDEDUI,true,0);

        CheckDlgButton(hwnd,IDC_WARN_INCONSISTENT_EOLS, SetBtn(Settings.WarnInconsistEOLs));
        CheckDlgButton(hwnd,IDC_CONSISTENT_EOLS, SetBtn(Settings.FixLineEndings));
        CheckDlgButton(hwnd,IDC_AUTOSTRIPBLANKS, SetBtn(Settings.FixTrailingBlanks));

        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            int* piOption = (int*)GetWindowLongPtr(hwnd, DWLP_USER);
            *piOption = (int)SendDlgItemMessage(hwnd,IDC_EOLMODELIST,CB_GETCURSEL,0,0);
            Settings.WarnInconsistEOLs = IsButtonChecked(hwnd,IDC_WARN_INCONSISTENT_EOLS);
            Settings.FixLineEndings = IsButtonChecked(hwnd,IDC_CONSISTENT_EOLS);
            Settings.FixTrailingBlanks = IsButtonChecked(hwnd,IDC_AUTOSTRIPBLANKS);
            EndDialog(hwnd,IDOK);
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;
      }
      return true;
  }
  return false;
}


//=============================================================================
//
//  SelectDefLineEndingDlg()
//
bool SelectDefLineEndingDlg(HWND hwnd, LPARAM piOption)
{
  INT_PTR const iResult = ThemedDialogBoxParam(Globals.hLngResContainer,
                                               MAKEINTRESOURCE(IDD_MUI_DEFEOLMODE),
                                               hwnd,
                                               SelectDefLineEndingDlgProc,
                                               piOption);

  return (iResult == IDOK);
}



//=============================================================================
//
//  WarnLineEndingDlgProc()
//
//
static INT_PTR CALLBACK WarnLineEndingDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) 
{
  switch (umsg) 
  {
  case WM_INITDIALOG: 
  {
    SetWindowLongPtr(hwnd, DWLP_USER, lParam);
    if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

    const EditFileIOStatus* const fioStatus = (EditFileIOStatus*)lParam;
    int const iEOLMode = fioStatus->iEOLMode;

    // Load options
    WCHAR wch[128];
    for (int i = 0; i < 3; i++) {
      GetLngString(IDS_MUI_EOLMODENAME_CRLF + i, wch, COUNTOF(wch));
      SendDlgItemMessage(hwnd, IDC_EOLMODELIST, CB_ADDSTRING, 0, (LPARAM)wch);
    }

    SendDlgItemMessage(hwnd, IDC_EOLMODELIST, CB_SETCURSEL, iEOLMode, 0);
    SendDlgItemMessage(hwnd, IDC_EOLMODELIST, CB_SETEXTENDEDUI, TRUE, 0);

    WCHAR tchFmt[128];
    for (int i = 0; i < 3; ++i) {
      WCHAR tchLn[32];
      StringCchPrintf(tchLn, COUNTOF(tchLn), DOCPOSFMTW, fioStatus->eolCount[i]);
      FormatNumberStr(tchLn, COUNTOF(tchLn), 0);
      GetDlgItemText(hwnd, IDC_EOL_SUM_CRLF + i, tchFmt, COUNTOF(tchFmt));
      StringCchPrintf(wch, COUNTOF(wch), tchFmt, tchLn);
      SetDlgItemText(hwnd, IDC_EOL_SUM_CRLF + i, wch);
    }

    CheckDlgButton(hwnd, IDC_WARN_INCONSISTENT_EOLS, SetBtn(Settings.WarnInconsistEOLs));
    CenterDlgInParent(hwnd, NULL);

    AttentionBeep(MB_ICONEXCLAMATION);
  }
  return true;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK: 
    case IDCANCEL:
      {
        EditFileIOStatus* status = (EditFileIOStatus*)GetWindowLongPtr(hwnd, DWLP_USER);
        const int iEOLMode = (int)SendDlgItemMessage(hwnd, IDC_EOLMODELIST, CB_GETCURSEL, 0, 0);
        status->iEOLMode = iEOLMode;
        Settings.WarnInconsistEOLs = IsButtonChecked(hwnd, IDC_WARN_INCONSISTENT_EOLS);
        EndDialog(hwnd, LOWORD(wParam));
      }
      break;
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  WarnLineEndingDlg()
//
bool WarnLineEndingDlg(HWND hwnd, EditFileIOStatus* fioStatus) 
{
  const INT_PTR iResult = ThemedDialogBoxParam(Globals.hLngResContainer, 
                                               MAKEINTRESOURCE(IDD_MUI_WARNLINEENDS), 
                                               hwnd, 
                                               WarnLineEndingDlgProc, 
                                               (LPARAM)fioStatus);
  return (iResult == IDOK);
}


//=============================================================================
//
//  WarnIndentationDlgProc()
//
//
static INT_PTR CALLBACK WarnIndentationDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) 
  {
  case WM_INITDIALOG: 
  {
    SetWindowLongPtr(hwnd, DWLP_USER, lParam);
    if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

    const EditFileIOStatus* const fioStatus = (EditFileIOStatus*)lParam;

    WCHAR wch[128];
    WCHAR tchFmt[128];
    WCHAR tchCnt[32];

    GetDlgItemText(hwnd, IDC_INDENT_WIDTH_TAB, tchFmt, COUNTOF(tchFmt));
    StringCchPrintf(wch, COUNTOF(wch), tchFmt, Globals.fvCurFile.iTabWidth);
    SetDlgItemText(hwnd, IDC_INDENT_WIDTH_TAB, wch);

    GetDlgItemText(hwnd, IDC_INDENT_WIDTH_SPC, tchFmt, COUNTOF(tchFmt));
    StringCchPrintf(wch, COUNTOF(wch), tchFmt, Globals.fvCurFile.iIndentWidth);
    SetDlgItemText(hwnd, IDC_INDENT_WIDTH_SPC, wch);

    StringCchPrintf(tchCnt, COUNTOF(tchCnt), DOCPOSFMTW, fioStatus->indentCount[I_TAB_LN]);
    FormatNumberStr(tchCnt, COUNTOF(tchCnt), 0);
    GetDlgItemText(hwnd, IDC_INDENT_SUM_TAB, tchFmt, COUNTOF(tchFmt));
    StringCchPrintf(wch, COUNTOF(wch), tchFmt, tchCnt);
    SetDlgItemText(hwnd, IDC_INDENT_SUM_TAB, wch);

    StringCchPrintf(tchCnt, COUNTOF(tchCnt), DOCPOSFMTW, fioStatus->indentCount[I_SPC_LN]);
    FormatNumberStr(tchCnt, COUNTOF(tchCnt), 0);
    GetDlgItemText(hwnd, IDC_INDENT_SUM_SPC, tchFmt, COUNTOF(tchFmt));
    StringCchPrintf(wch, COUNTOF(wch), tchFmt, tchCnt);
    SetDlgItemText(hwnd, IDC_INDENT_SUM_SPC, wch);

    StringCchPrintf(tchCnt, COUNTOF(tchCnt), DOCPOSFMTW, fioStatus->indentCount[I_MIX_LN]);
    FormatNumberStr(tchCnt, COUNTOF(tchCnt), 0);
    GetDlgItemText(hwnd, IDC_INDENT_SUM_MIX, tchFmt, COUNTOF(tchFmt));
    StringCchPrintf(wch, COUNTOF(wch), tchFmt, tchCnt);
    SetDlgItemText(hwnd, IDC_INDENT_SUM_MIX, wch);

    StringCchPrintf(tchCnt, COUNTOF(tchCnt), DOCPOSFMTW, fioStatus->indentCount[I_TAB_MOD_X]);
    FormatNumberStr(tchCnt, COUNTOF(tchCnt), 0);
    GetDlgItemText(hwnd, IDC_INDENT_TAB_MODX, tchFmt, COUNTOF(tchFmt));
    StringCchPrintf(wch, COUNTOF(wch), tchFmt, tchCnt);
    SetDlgItemText(hwnd, IDC_INDENT_TAB_MODX, wch);

    StringCchPrintf(tchCnt, COUNTOF(tchCnt), DOCPOSFMTW, fioStatus->indentCount[I_SPC_MOD_X]);
    FormatNumberStr(tchCnt, COUNTOF(tchCnt), 0);
    GetDlgItemText(hwnd, IDC_INDENT_SPC_MODX, tchFmt, COUNTOF(tchFmt));
    StringCchPrintf(wch, COUNTOF(wch), tchFmt, tchCnt);
    SetDlgItemText(hwnd, IDC_INDENT_SPC_MODX, wch);

    CheckDlgButton(hwnd, Globals.fvCurFile.bTabsAsSpaces ? IDC_INDENT_BY_SPCS : IDC_INDENT_BY_TABS, true);
    CheckDlgButton(hwnd, IDC_WARN_INCONSISTENT_INDENTS, SetBtn(Settings.WarnInconsistentIndents));
    CenterDlgInParent(hwnd, NULL);

    AttentionBeep(MB_ICONEXCLAMATION);
  }
  return true;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK: 
      {
        EditFileIOStatus* fioStatus = (EditFileIOStatus*)GetWindowLongPtr(hwnd, DWLP_USER);
        fioStatus->iGlobalIndent = IsButtonChecked(hwnd, IDC_INDENT_BY_TABS) ? I_TAB_LN : I_SPC_LN;
        Settings.WarnInconsistentIndents = IsButtonChecked(hwnd, IDC_WARN_INCONSISTENT_INDENTS);
        EndDialog(hwnd, IDOK);
      }
      break;

    case IDCANCEL: 
      {
        EditFileIOStatus* fioStatus = (EditFileIOStatus*)GetWindowLongPtr(hwnd, DWLP_USER);
        fioStatus->iGlobalIndent = I_MIX_LN;
        Settings.WarnInconsistentIndents = IsButtonChecked(hwnd, IDC_WARN_INCONSISTENT_INDENTS);
        EndDialog(hwnd, IDCANCEL);
      }
      break;
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  WarnIndentationDlg()
//
bool WarnIndentationDlg(HWND hwnd, EditFileIOStatus* fioStatus)
{
  const INT_PTR iResult = ThemedDialogBoxParam(Globals.hLngResContainer,
                                               MAKEINTRESOURCE(IDD_MUI_WARNINDENTATION),
                                               hwnd,
                                               WarnIndentationDlgProc,
                                               (LPARAM)fioStatus);
  return (iResult == IDOK);
}



//=============================================================================
//
//  GetMonitorInfoFromRect()
//
bool GetMonitorInfoFromRect(const RECT* rc, MONITORINFO* hMonitorInfo)
{
  bool result = false;
  if (hMonitorInfo) {
    HMONITOR const hMonitor = MonitorFromRect(rc, MONITOR_DEFAULTTONEAREST);
    ZeroMemory(hMonitorInfo, sizeof(MONITORINFO));
    hMonitorInfo->cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfo(hMonitor, hMonitorInfo)) {
      RECT _rc = { 0, 0, 0, 0 };
      if (SystemParametersInfo(SPI_GETWORKAREA, 0, &_rc, 0) != 0) {
        hMonitorInfo->rcWork = _rc;
        SetRect(&(hMonitorInfo->rcMonitor), 0, 0, _rc.right, _rc.bottom);
        result = true;
      }
    }
    else
      result = true;
  }
  return result;
}
// ----------------------------------------------------------------------------



//=============================================================================
//
//  WinInfoToScreen()
//
void WinInfoToScreen(WININFO* pWinInfo)
{
  if (pWinInfo) {
    MONITORINFO mi;
    RECT rc = RectFromWinInfo(pWinInfo);
    if (GetMonitorInfoFromRect(&rc, &mi)) {
      WININFO winfo = *pWinInfo;
      winfo.x += (mi.rcWork.left - mi.rcMonitor.left);
      winfo.y += (mi.rcWork.top - mi.rcMonitor.top);
      *pWinInfo = winfo;
    }
  }
}


//=============================================================================
//
//  GetMyWindowPlacement()
//
WININFO GetMyWindowPlacement(HWND hwnd, MONITORINFO* hMonitorInfo)
{
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(hwnd, &wndpl);

  // corrections in case of aero snapped position
  if (SW_NORMAL == wndpl.showCmd) {
    RECT rc;
    GetWindowRect(hwnd, &rc);
    MONITORINFO mi;
    GetMonitorInfoFromRect(&rc, &mi);
    LONG const width = rc.right - rc.left;
    LONG const height = rc.bottom - rc.top;
    rc.left -= (mi.rcWork.left - mi.rcMonitor.left);
    rc.right = rc.left + width;
    rc.top -= (mi.rcWork.top - mi.rcMonitor.top);
    rc.bottom = rc.top + height;
    wndpl.rcNormalPosition = rc;
  }

  WININFO wi;
  wi.x = wndpl.rcNormalPosition.left;
  wi.y = wndpl.rcNormalPosition.top;
  wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
  wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
  wi.max = IsZoomed(hwnd) || (wndpl.flags & WPF_RESTORETOMAXIMIZED);
  wi.zoom = SciCall_GetZoom();

  // set monitor info too
  GetMonitorInfoFromRect(&(wndpl.rcNormalPosition), hMonitorInfo);

  return wi;
}



//=============================================================================
//
//  FitIntoMonitorGeometry()
//
void FitIntoMonitorGeometry(RECT* pRect, WININFO* pWinInfo, SCREEN_MODE mode)
{
  MONITORINFO mi;
  GetMonitorInfoFromRect(pRect, &mi);

  if (mode == SCR_FULL_SCREEN) {
    SetRect(pRect, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
    // monitor coord -> screen coord
    pWinInfo->x = mi.rcMonitor.left - (mi.rcWork.left - mi.rcMonitor.left);
    pWinInfo->y = mi.rcMonitor.top - (mi.rcWork.top - mi.rcMonitor.top);
    pWinInfo->cx = (mi.rcMonitor.right - mi.rcMonitor.left);
    pWinInfo->cy = (mi.rcMonitor.bottom - mi.rcMonitor.top);
    pWinInfo->max = true;
  }
  else {
    WININFO wi = *pWinInfo;
    WinInfoToScreen(&wi);
    // fit into area
    if (wi.x < mi.rcWork.left) { wi.x = mi.rcWork.left; }
    if (wi.y < mi.rcWork.top) { wi.y = mi.rcWork.top; }
    if ((wi.x + wi.cx) > mi.rcWork.right) {
      wi.x -= (wi.x + wi.cx - mi.rcWork.right);
      if (wi.x < mi.rcWork.left) { wi.x = mi.rcWork.left; }
      if ((wi.x + wi.cx) > mi.rcWork.right) { wi.cx = mi.rcWork.right - wi.x; }
    }
    if ((wi.y + wi.cy) > mi.rcWork.bottom) {
      wi.y -= (wi.y + wi.cy - mi.rcWork.bottom);
      if (wi.y < mi.rcWork.top) { wi.y = mi.rcWork.top; }
      if ((wi.y + wi.cy) > mi.rcWork.bottom) { wi.cy = mi.rcWork.bottom - wi.y; }
    }
    SetRect(pRect, wi.x, wi.y, wi.x + wi.cx, wi.y + wi.cy);
    // monitor coord -> work area coord
    pWinInfo->x = wi.x - (mi.rcWork.left - mi.rcMonitor.left);
    pWinInfo->y = wi.y - (mi.rcWork.top - mi.rcMonitor.top);
    pWinInfo->cx = wi.cx;
    pWinInfo->cy = wi.cy;
    //pWinInfo->max = true;
  }
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  WindowPlacementFromInfo()
//
//
WINDOWPLACEMENT WindowPlacementFromInfo(HWND hwnd, const WININFO* pWinInfo, SCREEN_MODE mode)
{
  WINDOWPLACEMENT wndpl;
  ZeroMemory(&wndpl, sizeof(WINDOWPLACEMENT));
  wndpl.length = sizeof(WINDOWPLACEMENT);
  wndpl.flags = WPF_ASYNCWINDOWPLACEMENT;

  WININFO winfo = INIT_WININFO;
  if (pWinInfo) {
    RECT rc = RectFromWinInfo(pWinInfo);
    winfo = *pWinInfo;
    FitIntoMonitorGeometry(&rc, &winfo, mode);
    if (pWinInfo->max) { wndpl.flags &= WPF_RESTORETOMAXIMIZED; }
    wndpl.showCmd = SW_RESTORE;
  }
  else {
    RECT rc; 
    if (hwnd)
      GetWindowRect(hwnd, &rc);
    else
      GetWindowRect(GetDesktopWindow(), &rc);

    FitIntoMonitorGeometry(&rc, &winfo, mode);

    wndpl.showCmd = SW_SHOW;
  }
  wndpl.rcNormalPosition = RectFromWinInfo(&winfo);
  return wndpl;
}


//=============================================================================
//
//  DialogNewWindow()
//
//
void DialogNewWindow(HWND hwnd, bool bSaveOnRunTools, bool bSetCurFile)
{
  if (bSaveOnRunTools && !FileSave(false, true, false, false, Flags.bPreserveFileModTime)) { return; }

  WCHAR szModuleName[MAX_PATH] = { L'\0' };
  GetModuleFileName(NULL, szModuleName, COUNTOF(szModuleName));
  PathCanonicalizeEx(szModuleName, COUNTOF(szModuleName));

  WCHAR tch[64] = { L'\0' };
  WCHAR szParameters[2 * MAX_PATH + 64] = { L'\0' };
  StringCchPrintf(tch, COUNTOF(tch), L"\"-appid=%s\"", Settings2.AppUserModelID);
  StringCchCopy(szParameters, COUNTOF(szParameters), tch);

  StringCchPrintf(tch, COUNTOF(tch), L"\" -sysmru=%i\"", (Flags.ShellUseSystemMRU ? 1 : 0));
  StringCchCat(szParameters, COUNTOF(szParameters), tch);

  StringCchCat(szParameters, COUNTOF(szParameters), L" -f");
  if (StrIsNotEmpty(Globals.IniFile)) {
    StringCchCat(szParameters, COUNTOF(szParameters), L" \"");
    StringCchCat(szParameters, COUNTOF(szParameters), Globals.IniFile);
    StringCchCat(szParameters, COUNTOF(szParameters), L" \"");
  }
  else
    StringCchCat(szParameters, COUNTOF(szParameters), L"0");

  StringCchCat(szParameters, COUNTOF(szParameters), L" -n");

  MONITORINFO mi;
  WININFO wi = GetMyWindowPlacement(hwnd, &mi);
  // offset new window position +10/+10
  wi.x += 10;
  wi.y += 10;
  // check if window fits monitor
  if ((wi.x + wi.cx) > mi.rcWork.right || (wi.y + wi.cy) > mi.rcWork.bottom) {
    wi.x = mi.rcMonitor.left;
    wi.y = mi.rcMonitor.top;
  }
  wi.max = IsZoomed(hwnd);

  StringCchPrintf(tch, COUNTOF(tch), L" -pos %i,%i,%i,%i,%i", wi.x, wi.y, wi.cx, wi.cy, wi.max);
  StringCchCat(szParameters, COUNTOF(szParameters), tch);

  if (bSetCurFile && StrIsNotEmpty(Globals.CurrentFile))
  {
    WCHAR szFileName[MAX_PATH] = { L'\0' };
    StringCchCopy(szFileName, COUNTOF(szFileName), Globals.CurrentFile);
    PathQuoteSpaces(szFileName);
    StringCchCat(szParameters, COUNTOF(szParameters), L" ");
    StringCchCat(szParameters, COUNTOF(szParameters), szFileName);
  }

  SHELLEXECUTEINFO sei;
  ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOZONECHECKS;
  sei.hwnd = hwnd;
  sei.lpVerb = NULL;
  sei.lpFile = szModuleName;
  sei.lpParameters = szParameters;
  sei.lpDirectory = Globals.WorkingDirectory;
  sei.nShow = SW_SHOWNORMAL;
  ShellExecuteEx(&sei);
}


//=============================================================================
//
//  DialogFileBrowse()
//
//
void DialogFileBrowse(HWND hwnd)
{
  WCHAR tchParam[MAX_PATH] = L"";
  WCHAR tchExeFile[MAX_PATH] = L"";
  WCHAR tchTemp[MAX_PATH];

  StringCchCopyW(tchTemp, COUNTOF(tchTemp), Settings2.FileBrowserPath);

  if (StrIsNotEmpty(Settings2.FileBrowserPath))
  {
    ExtractFirstArgument(tchTemp, tchExeFile, tchParam, COUNTOF(tchTemp));
  }
  if (StrIsEmpty(tchExeFile)) {
    StringCchCopy(tchExeFile, COUNTOF(tchExeFile), Constants.FileBrowserMiniPath);
  }
  if (PathIsRelative(tchExeFile)) {
    GetModuleFileName(NULL, tchTemp, COUNTOF(tchTemp));
    PathCchRemoveFileSpec(tchTemp, COUNTOF(tchTemp));
    PathAppend(tchTemp, tchExeFile);
    if (PathFileExists(tchTemp)) {
      StringCchCopy(tchExeFile, COUNTOF(tchExeFile), tchTemp);
    }
  }
  if (StrIsNotEmpty(tchParam) && StrIsNotEmpty(Globals.CurrentFile)) {
    StringCchCat(tchParam, COUNTOF(tchParam), L" ");
  }
  if (StrIsNotEmpty(Globals.CurrentFile)) {
    StringCchCopy(tchTemp, COUNTOF(tchTemp), Globals.CurrentFile);
    PathQuoteSpaces(tchTemp);
    StringCchCat(tchParam, COUNTOF(tchParam), tchTemp);
  }

  SHELLEXECUTEINFO sei;
  ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOZONECHECKS;
  sei.hwnd = hwnd;
  sei.lpVerb = NULL;
  sei.lpFile = tchExeFile;
  sei.lpParameters = tchParam;
  sei.lpDirectory = NULL;
  sei.nShow = SW_SHOWNORMAL;
  ShellExecuteEx(&sei);

  if ((INT_PTR)sei.hInstApp < 32) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_BROWSE);
  }
}


//=============================================================================
//
//  DialogAdminExe()
//
//

void DialogAdminExe(HWND hwnd, bool bExecInstaller)
{
  WCHAR tchExe[MAX_PATH];

  StringCchCopyW(tchExe, COUNTOF(tchExe), Settings2.AdministrationTool);
  if (bExecInstaller && StrIsEmpty(tchExe)) { return; }

  WCHAR tchExePath[MAX_PATH];
  if (!SearchPath(NULL, tchExe, L".exe", COUNTOF(tchExePath), tchExePath, NULL)) {
    // try Notepad3's dir path
    GetModuleFileName(NULL, tchExePath, COUNTOF(tchExePath));
    PathCchRemoveFileSpec(tchExePath, COUNTOF(tchExePath));
    PathCchAppend(tchExePath, COUNTOF(tchExePath), tchExe);
  }

  SHELLEXECUTEINFO sei;
  ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOZONECHECKS;
  sei.hwnd = hwnd;
  sei.lpVerb = NULL;
  sei.lpFile = tchExePath;
  sei.lpParameters = NULL; // tchParam;
  sei.lpDirectory = Globals.WorkingDirectory;
  sei.nShow = SW_SHOWNORMAL;

  if (bExecInstaller) {
    ShellExecuteEx(&sei);
    if ((INT_PTR)sei.hInstApp < 32)
    {
      INT_PTR const answer = InfoBoxLng(MB_OKCANCEL, L"NoAdminTool", IDS_MUI_ERR_ADMINEXE);
      if ((IDOK == answer) || (IDYES == answer))
      {
        sei.lpFile = VERSION_UPDATE_CHECK;
        ShellExecuteEx(&sei);
      }
    }
  }
  else {
    sei.lpFile = VERSION_UPDATE_CHECK;
    ShellExecuteEx(&sei);
  }
}

// ============================================================================
// some Helpers
// ============================================================================


//=============================================================================
//
//  SetWindowTitle()
//
bool bFreezeAppTitle = false;

static const WCHAR *pszSep = L" - ";
static const WCHAR *pszMod = L"* ";
static WCHAR szCachedFile[MAX_PATH] = { L'\0' };
static WCHAR szCachedDisplayName[MAX_PATH] = { L'\0' };
static WCHAR szAdditionalTitleInfo[MAX_PATH] = { L'\0' };

bool SetWindowTitle(HWND hwnd, UINT uIDAppName, bool bIsElevated, UINT uIDUntitled,
  LPCWSTR lpszFile, int iFormat, bool bModified,
  UINT uIDReadOnly, bool bReadOnly, LPCWSTR lpszExcerpt)
{
  if (bFreezeAppTitle)
    return false;

  WCHAR szAppName[SMALL_BUFFER] = { L'\0' };
  WCHAR szUntitled[SMALL_BUFFER] = { L'\0' };
  if (!GetLngString(uIDAppName, szAppName, COUNTOF(szAppName)) ||
    !GetLngString(uIDUntitled, szUntitled, COUNTOF(szUntitled))) {
    return false;
  }
  if (bIsElevated) {
    WCHAR szElevatedAppName[SMALL_BUFFER] = { L'\0' };
    FormatLngStringW(szElevatedAppName, COUNTOF(szElevatedAppName), IDS_MUI_APPTITLE_ELEVATED, szAppName);
    StringCchCopyN(szAppName, COUNTOF(szAppName), szElevatedAppName, COUNTOF(szElevatedAppName));
  }

  WCHAR szTitle[MIDSZ_BUFFER] = { L'\0' };
  
  if (bModified) {
    StringCchCat(szTitle, COUNTOF(szTitle), pszMod);
  }
  if (StrIsNotEmpty(lpszExcerpt)) {
    WCHAR szExcrptFmt[32] = { L'\0' };
    WCHAR szExcrptQuot[SMALL_BUFFER] = { L'\0' };
    GetLngString(IDS_MUI_TITLEEXCERPT, szExcrptFmt, COUNTOF(szExcrptFmt));
    StringCchPrintf(szExcrptQuot, COUNTOF(szExcrptQuot), szExcrptFmt, lpszExcerpt);
    StringCchCat(szTitle, COUNTOF(szTitle), szExcrptQuot);
  }
  else if (StrIsNotEmpty(lpszFile))
  {
    if ((iFormat < 2) && !PathIsRoot(lpszFile))
    {
      if (StringCchCompareN(szCachedFile, COUNTOF(szCachedFile), lpszFile, MAX_PATH) != 0)
      {
        StringCchCopy(szCachedFile, COUNTOF(szCachedFile), lpszFile);
        PathGetDisplayName(szCachedDisplayName, COUNTOF(szCachedDisplayName), szCachedFile);
      }
      StringCchCat(szTitle, COUNTOF(szTitle), szCachedDisplayName);
      if (iFormat == 1) {
        WCHAR tchPath[MAX_PATH] = { L'\0' };
        StringCchCopy(tchPath, COUNTOF(tchPath), lpszFile);
        PathCchRemoveFileSpec(tchPath, COUNTOF(tchPath));
        StringCchCat(szTitle, COUNTOF(szTitle), L" [");
        StringCchCat(szTitle, COUNTOF(szTitle), tchPath);
        StringCchCat(szTitle, COUNTOF(szTitle), L"]");
      }
    }
    else
      StringCchCat(szTitle, COUNTOF(szTitle), lpszFile);
  }
  else {
    StringCchCopy(szCachedFile, COUNTOF(szCachedFile), L"");
    StringCchCopy(szCachedDisplayName, COUNTOF(szCachedDisplayName), L"");
    StringCchCat(szTitle, COUNTOF(szTitle), szUntitled);
  }

  WCHAR szReadOnly[32] = { L'\0' };
  if (bReadOnly && GetLngString(uIDReadOnly, szReadOnly, COUNTOF(szReadOnly)))
  {
    StringCchCat(szTitle, COUNTOF(szTitle), L" ");
    StringCchCat(szTitle, COUNTOF(szTitle), szReadOnly);
  }

  StringCchCat(szTitle, COUNTOF(szTitle), pszSep);
  StringCchCat(szTitle, COUNTOF(szTitle), szAppName);

  // UCHARDET
  if (StrIsNotEmpty(szAdditionalTitleInfo)) {
    StringCchCat(szTitle, COUNTOF(szTitle), pszSep);
    StringCchCat(szTitle, COUNTOF(szTitle), szAdditionalTitleInfo);
  }

  return SetWindowText(hwnd, szTitle);

}

void SetAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo)
{
  StringCchCopy(szAdditionalTitleInfo, COUNTOF(szAdditionalTitleInfo), lpszAddTitleInfo);
}

void AppendAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo)
{
  StringCchCat(szAdditionalTitleInfo, COUNTOF(szAdditionalTitleInfo), lpszAddTitleInfo);
}


//=============================================================================
//
//  SetWindowTransparentMode()
//
void SetWindowTransparentMode(HWND hwnd, bool bTransparentMode, int iOpacityLevel)
{
  if (bTransparentMode) {
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    // get opacity level from registry
    BYTE const bAlpha = (BYTE)MulDiv(iOpacityLevel, 255, 100);
    SetLayeredWindowAttributes(hwnd, 0, bAlpha, LWA_ALPHA);
    return;
  }
  SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
}


//=============================================================================
//
//  CenterDlgInParent()
//
void CenterDlgInParent(HWND hDlg, HWND hDlgParent /* = NULL */)
{
  if (!hDlg) { return; }

  RECT rcDlg;
  GetWindowRect(hDlg, &rcDlg);

  HWND const hParent = hDlgParent ? hDlgParent : GetParent(hDlg);
  RECT rcParent;
  BOOL const bFoundRect = hParent ? GetWindowRect(hParent, &rcParent) : GetWindowRect(GetDesktopWindow(), &rcParent);

  if (bFoundRect) 
  {
    HMONITOR const hMonitor = MonitorFromRect(&rcParent, MONITOR_DEFAULTTONEAREST);

    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &mi);

    int const xMin = mi.rcWork.left;
    int const yMin = mi.rcWork.top;

    int const xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
    int const yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

    int x;
    if ((rcParent.right - rcParent.left) - (rcDlg.right - rcDlg.left) > 20)
      x = rcParent.left + (((rcParent.right - rcParent.left) - (rcDlg.right - rcDlg.left)) / 2);
    else
      x = rcParent.left + 70;

    int y;
    if ((rcParent.bottom - rcParent.top) - (rcDlg.bottom - rcDlg.top) > 20)
      y = rcParent.top + (((rcParent.bottom - rcParent.top) - (rcDlg.bottom - rcDlg.top)) / 2);
    else
      y = rcParent.top + 60;

    SetWindowPos(hDlg, NULL, clampi(x, xMin, xMax), clampi(y, yMin, yMax), 0, 0, SWP_NOZORDER | SWP_NOSIZE);

  }
  //~SnapToDefaultButton(hDlg);
}


//=============================================================================
//
//  GetDlgPos()
//
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg)
{

  RECT rcDlg;
  GetWindowRect(hDlg, &rcDlg);

  HWND const hParent = GetParent(hDlg);
  RECT rcParent;
  GetWindowRect(hParent, &rcParent);

  // return positions relative to parent window
  *xDlg = (rcDlg.left - rcParent.left);
  *yDlg = (rcDlg.top - rcParent.top);

}


//=============================================================================
//
//  SetDlgPos()
//
void SetDlgPos(HWND hDlg, int xDlg, int yDlg)
{
  RECT rcDlg;
  GetWindowRect(hDlg, &rcDlg);

  HWND const hParent = GetParent(hDlg);
  RECT rcParent;
  GetWindowRect(hParent, &rcParent);

  HMONITOR const hMonitor = MonitorFromRect(&rcParent, MONITOR_DEFAULTTONEAREST);

  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);

  int const xMin = mi.rcWork.left;
  int const yMin = mi.rcWork.top;

  int const xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
  int const yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

  // desired positions relative to parent window
  int const x = rcParent.left + xDlg;
  int const y = rcParent.top + yDlg;

  SetWindowPos(hDlg, NULL, clampi(x, xMin, xMax), clampi(y, yMin, yMax), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}


//=============================================================================
//
// Resize Dialog Helpers()
//
#define RESIZEDLG_PROP_KEY	L"ResizeDlg"
typedef struct _resizeDlg {
  int direction;
  int cxClient;
  int cyClient;
  int mmiPtMinX;
  int mmiPtMinY;
  int mmiPtMaxX;	// only Y direction
  int mmiPtMaxY;	// only X direction
  int attrs[MAX_RESIZEDLG_ATTR_COUNT];
} RESIZEDLG, * PRESIZEDLG;

typedef const RESIZEDLG* LPCRESIZEDLG;

void ResizeDlg_InitEx(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int iDirection) 
{
  RESIZEDLG* pm = (RESIZEDLG*)AllocMem(sizeof(RESIZEDLG), HEAP_ZERO_MEMORY);
  pm->direction = iDirection;

  RECT rc;
  GetClientRect(hwnd, &rc);
  pm->cxClient = rc.right - rc.left;
  pm->cyClient = rc.bottom - rc.top;

  AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE) | WS_THICKFRAME, FALSE, 0);
  pm->mmiPtMinX = rc.right - rc.left;
  pm->mmiPtMinY = rc.bottom - rc.top;
  // only one direction
  switch (iDirection) {
  case ResizeDlgDirection_OnlyX:
    pm->mmiPtMaxY = pm->mmiPtMinY;
    break;

  case ResizeDlgDirection_OnlyY:
    pm->mmiPtMaxX = pm->mmiPtMinX;
    break;
  }

  cxFrame = max_i(cxFrame, pm->mmiPtMinX);
  cyFrame = max_i(cyFrame, pm->mmiPtMinY);

  SetProp(hwnd, RESIZEDLG_PROP_KEY, (HANDLE)pm);

  SetWindowPos(hwnd, NULL, rc.left, rc.top, cxFrame, cyFrame, SWP_NOZORDER);

  SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_THICKFRAME);
  SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

  WCHAR wch[64];
  GetMenuString(GetSystemMenu(GetParent(hwnd), FALSE), SC_SIZE, wch, COUNTOF(wch), MF_BYCOMMAND);
  InsertMenu(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_STRING | MF_ENABLED, SC_SIZE, wch);
  InsertMenu(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);

  HWND hwndCtl = GetDlgItem(hwnd, nIdGrip);
  SetWindowLongPtr(hwndCtl, GWL_STYLE, GetWindowLongPtr(hwndCtl, GWL_STYLE) | SBS_SIZEGRIP | WS_CLIPSIBLINGS);
  /// TODO: per-window DPI
  const int cGrip = GetSystemMetricsEx(SM_CXHTHUMB);
  SetWindowPos(hwndCtl, NULL, pm->cxClient - cGrip, pm->cyClient - cGrip, cGrip, cGrip, SWP_NOZORDER);
}

void ResizeDlg_Destroy(HWND hwnd, int* cxFrame, int* cyFrame) {
  PRESIZEDLG pm = (PRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);

  RECT rc;
  GetWindowRect(hwnd, &rc);
  if (cxFrame) {
    *cxFrame = rc.right - rc.left;
  }
  if (cyFrame) {
    *cyFrame = rc.bottom - rc.top;
  }

  RemoveProp(hwnd, RESIZEDLG_PROP_KEY);
  FreeMem(pm);
}

void ResizeDlg_Size(HWND hwnd, LPARAM lParam, int* cx, int* cy) {
  PRESIZEDLG pm = (PRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
  const int cxClient = LOWORD(lParam);
  const int cyClient = HIWORD(lParam);
  if (cx) {
    *cx = cxClient - pm->cxClient;
  }
  if (cy) {
    *cy = cyClient - pm->cyClient;
  }
  pm->cxClient = cxClient;
  pm->cyClient = cyClient;
}

void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam) {
  LPCRESIZEDLG pm = (LPCRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);

  LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
  lpmmi->ptMinTrackSize.x = pm->mmiPtMinX;
  lpmmi->ptMinTrackSize.y = pm->mmiPtMinY;

  // only one direction
  switch (pm->direction) {
  case ResizeDlgDirection_OnlyX:
    lpmmi->ptMaxTrackSize.y = pm->mmiPtMaxY;
    break;

  case ResizeDlgDirection_OnlyY:
    lpmmi->ptMaxTrackSize.x = pm->mmiPtMaxX;
    break;
  }
}

void ResizeDlg_SetAttr(HWND hwnd, int index, int value) {
  if (index < MAX_RESIZEDLG_ATTR_COUNT) {
    PRESIZEDLG pm = (PRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
    pm->attrs[index] = value;
  }
}

int ResizeDlg_GetAttr(HWND hwnd, int index) {
  if (index < MAX_RESIZEDLG_ATTR_COUNT) {
    const LPCRESIZEDLG pm = (LPCRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
    return pm->attrs[index];
  }

  return 0;
}

static inline int GetDlgCtlHeight(HWND hwndDlg, int nCtlId) {
  RECT rc;
  GetWindowRect(GetDlgItem(hwndDlg, nCtlId), &rc);
  const int height = rc.bottom - rc.top;
  return height;
}

void ResizeDlgCtl(HWND hwndDlg, int nCtlId, int dx, int dy) {
  HWND hwndCtl = GetDlgItem(hwndDlg, nCtlId);
  RECT rc;
  GetWindowRect(hwndCtl, &rc);
  MapWindowPoints(NULL, hwndDlg, (LPPOINT)& rc, 2);
  SetWindowPos(hwndCtl, NULL, 0, 0, rc.right - rc.left + dx, rc.bottom - rc.top + dy, SWP_NOZORDER | SWP_NOMOVE);
  InvalidateRect(hwndCtl, NULL, TRUE);
}


HDWP DeferCtlPos(HDWP hdwp, HWND hwndDlg, int nCtlId, int dx, int dy, UINT uFlags) {
  HWND hwndCtl = GetDlgItem(hwndDlg, nCtlId);
  RECT rc;
  GetWindowRect(hwndCtl, &rc);
  MapWindowPoints(NULL, hwndDlg, (LPPOINT)& rc, 2);
  if (uFlags & SWP_NOSIZE) {
    return DeferWindowPos(hdwp, hwndCtl, NULL, rc.left + dx, rc.top + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }
  return DeferWindowPos(hdwp, hwndCtl, NULL, 0, 0, rc.right - rc.left + dx, rc.bottom - rc.top + dy, SWP_NOZORDER | SWP_NOMOVE);
}


//=============================================================================
//
//  MakeBitmapButton()
//
void MakeBitmapButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, WORD uBmpId)
{
  HWND const hwndCtl = GetDlgItem(hwnd, nCtlId);
  HBITMAP hBmp = LoadImage(hInstance, MAKEINTRESOURCE(uBmpId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  hBmp = ResizeImageForCurrentDPI(hBmp);
  BITMAP bmp;
  GetObject(hBmp, sizeof(BITMAP), &bmp);
  BUTTON_IMAGELIST bi;
  bi.himl = ImageList_Create(bmp.bmWidth, bmp.bmHeight, ILC_COLOR32 | ILC_MASK, 1, 0);
  ImageList_AddMasked(bi.himl, hBmp, CLR_DEFAULT);
  DeleteObject(hBmp);
  SetRect(&bi.margin, 0, 0, 0, 0);
  bi.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
  SendMessage(hwndCtl, BCM_SETIMAGELIST, 0, (LPARAM)&bi);
}



//=============================================================================
//
//  MakeColorPickButton()
//
void MakeColorPickButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, COLORREF crColor)
{
  HWND const hwndCtl = GetDlgItem(hwnd, nCtlId);
  HIMAGELIST himlOld = NULL;
  COLORMAP colormap[2];

  BUTTON_IMAGELIST bi;
  if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)&bi)) {
    himlOld = bi.himl;
  }
  if (IsWindowEnabled(hwndCtl) && crColor != ((COLORREF)-1)) {
    colormap[0].from = RGB(0x00, 0x00, 0x00);
    colormap[0].to = GetSysColor(COLOR_3DSHADOW);
  }
  else {
    colormap[0].from = RGB(0x00, 0x00, 0x00);
    colormap[0].to = RGB(0xFF, 0xFF, 0xFF);
  }

  if (IsWindowEnabled(hwndCtl) && (crColor != (COLORREF)-1)) {

    if (crColor == RGB(0xFF, 0xFF, 0xFF)) {
      crColor = RGB(0xFF, 0xFF, 0xFE);
    }
    colormap[1].from = RGB(0xFF, 0xFF, 0xFF);
    colormap[1].to = crColor;
  }
  else {
    colormap[1].from = RGB(0xFF, 0xFF, 0xFF);
    colormap[1].to = RGB(0xFF, 0xFF, 0xFF);
  }

  HBITMAP hBmp = CreateMappedBitmap(hInstance, IDB_PICK, 0, colormap, 2);

  bi.himl = ImageList_Create(10, 10, ILC_COLORDDB | ILC_MASK, 1, 0);
  ImageList_AddMasked(bi.himl, hBmp, RGB(0xFF, 0xFF, 0xFF));
  DeleteObject(hBmp);

  SetRect(&bi.margin, 0, 0, 4, 0);
  bi.uAlign = BUTTON_IMAGELIST_ALIGN_RIGHT;

  SendMessage(hwndCtl, BCM_SETIMAGELIST, 0, (LPARAM)&bi);
  InvalidateRect(hwndCtl, NULL, true);

  if (himlOld) {
    ImageList_Destroy(himlOld);
  }
}


//=============================================================================
//
//  DeleteBitmapButton()
//
void DeleteBitmapButton(HWND hwnd, int nCtlId)
{
  HWND const hwndCtl = GetDlgItem(hwnd, nCtlId);
  BUTTON_IMAGELIST bi;
  if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)& bi)) {
    ImageList_Destroy(bi.himl);
  }
}


//=============================================================================
//
//  SendWMSize()
//
LRESULT SendWMSize(HWND hwnd, RECT* rc)
{
  if (!rc) {
    RECT _rc;
    GetClientRect(hwnd, &_rc);
    return SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(_rc.right, _rc.bottom));
  }
  return SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc->right, rc->bottom));
}


//=============================================================================
//
//  StatusSetText()
//
void StatusSetText(HWND hwnd, UINT nPart, LPCWSTR lpszText)
{
  if (lpszText) {
    UINT const uFlags = (nPart == (UINT)STATUS_HELP) ? nPart | SBT_NOBORDERS : nPart;
    SendMessage(hwnd, SB_SETTEXT, uFlags, (LPARAM)lpszText);
  }
}


//=============================================================================
//
//  StatusSetTextID()
//
bool StatusSetTextID(HWND hwnd, UINT nPart, UINT uID)
{

  WCHAR szText[256] = { L'\0' };
  UINT const uFlags = (nPart == STATUS_HELP) ? nPart | SBT_NOBORDERS : nPart;

  if (!uID)
  {
    SendMessage(hwnd, SB_SETTEXT, uFlags, 0);
    return true;
  }

  if (!GetLngString(uID, szText, 256))
    return false;

  return (bool)SendMessage(hwnd, SB_SETTEXT, uFlags, (LPARAM)szText);

}


//=============================================================================
//
//  Toolbar_Get/SetButtons()
//
int Toolbar_GetButtons(HANDLE hwnd, int cmdBase, LPWSTR lpszButtons, int cchButtons)
{
  WCHAR tchButtons[512] = { L'\0' };
  WCHAR tchItem[32] = { L'\0' };

  StringCchCopy(tchButtons, COUNTOF(tchButtons), L"");
  int const c = min_i(50, (int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0));

  for (int i = 0; i < c; i++) {
    TBBUTTON tbb;
    SendMessage(hwnd, TB_GETBUTTON, (WPARAM)i, (LPARAM)&tbb);
    StringCchPrintf(tchItem, COUNTOF(tchItem), L"%i ",
      (tbb.idCommand == 0) ? 0 : tbb.idCommand - cmdBase + 1);
    StringCchCat(tchButtons, COUNTOF(tchButtons), tchItem);
  }
  TrimSpcW(tchButtons);
  StringCchCopyN(lpszButtons, cchButtons, tchButtons, COUNTOF(tchButtons));
  return(c);
}

int Toolbar_SetButtons(HANDLE hwnd, int cmdBase, LPCWSTR lpszButtons, LPCTBBUTTON ptbb, int ctbb)
{
  WCHAR tchButtons[MIDSZ_BUFFER];

  ZeroMemory(tchButtons, COUNTOF(tchButtons) * sizeof(tchButtons[0]));
  StringCchCopyN(tchButtons, COUNTOF(tchButtons), lpszButtons, COUNTOF(tchButtons) - 2);
  TrimSpcW(tchButtons);
  WCHAR *p = StrStr(tchButtons, L"  ");
  while (p) {
    MoveMemory((WCHAR*)p, (WCHAR*)p + 1, (StringCchLen(p,0) + 1) * sizeof(WCHAR));
    p = StrStr(tchButtons, L"  ");  // next
  }
  int const c = (int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0);
  for (int i = 0; i < c; i++) {
    SendMessage(hwnd, TB_DELETEBUTTON, 0, 0);
  }
  for (int i = 0; i < COUNTOF(tchButtons); i++) {
    if (tchButtons[i] == L' ') tchButtons[i] = 0;
  }
  p = tchButtons;
  while (*p) {
    int iCmd;
    if (swscanf_s(p, L"%i", &iCmd) == 1) {
     iCmd = (iCmd == 0) ? 0 : iCmd + cmdBase - 1;
      for (int i = 0; i < ctbb; i++) {
        if (ptbb[i].idCommand == iCmd) {
          SendMessage(hwnd, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&ptbb[i]);
          break;
        }
      }
    }
    p = StrEnd(p,0) + 1;
  }
  return((int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0));
}


/*

Themed Dialogs
Modify dialog templates to use current theme font
Based on code of MFC helper class CDialogTemplate

*/

bool GetThemedDialogFont(LPWSTR lpFaceName, WORD* wSize)
{
  bool bSucceed = false;
  DPI_T const ppi = GetCurrentPPI(NULL);

  HTHEME hTheme = OpenThemeData(NULL, L"WINDOWSTYLE;WINDOW");
  if (hTheme) {
    LOGFONT lf;
    if (S_OK == GetThemeSysFont(hTheme,/*TMT_MSGBOXFONT*/805, &lf)) {
      if (lf.lfHeight < 0) {
        lf.lfHeight = -lf.lfHeight;
      }
      *wSize = (WORD)MulDiv(lf.lfHeight, 72, ppi.y);
      if (*wSize == 0) { *wSize = 8; }
      StringCchCopyN(lpFaceName, LF_FACESIZE, lf.lfFaceName, LF_FACESIZE);
      bSucceed = true;
    }
    CloseThemeData(hTheme);
  }
  return(bSucceed);
}


inline bool DialogTemplate_IsDialogEx(const DLGTEMPLATE* pTemplate) {
  return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}

inline bool DialogTemplate_HasFont(const DLGTEMPLATE* pTemplate) {
  return (DS_SETFONT &
    (DialogTemplate_IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->style : pTemplate->style));
}

inline size_t DialogTemplate_FontAttrSize(bool bDialogEx) {
  return (sizeof(WORD) * (bDialogEx ? 3 : 1));
}


inline BYTE* DialogTemplate_GetFontSizeField(const DLGTEMPLATE* pTemplate) {

  bool bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
  WORD* pw;

  if (bDialogEx)
    pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
  else
    pw = (WORD*)(pTemplate + 1);

  if (*pw == (WORD)-1)
    pw += 2;
  else
    while (*pw++);

  if (*pw == (WORD)-1)
    pw += 2;
  else
    while (*pw++);

  while (*pw++);

  return (BYTE*)pw;
}

DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lpDialogTemplateID, HINSTANCE hInstance) 
{
  DLGTEMPLATE* pTemplate = NULL;

  HRSRC hRsrc = FindResource(hInstance, lpDialogTemplateID, RT_DIALOG);
  if (hRsrc == NULL) { return NULL; }

  HGLOBAL hRsrcMem = LoadResource(hInstance, hRsrc);
  if (hRsrcMem) {
    DLGTEMPLATE* pRsrcMem = (DLGTEMPLATE*)LockResource(hRsrcMem);
    size_t dwTemplateSize = (size_t)SizeofResource(hInstance, hRsrc);
    if ((dwTemplateSize == 0) || (pTemplate = AllocMem(dwTemplateSize + LF_FACESIZE * 2, HEAP_ZERO_MEMORY)) == NULL) {
      UnlockResource(hRsrcMem);
      FreeResource(hRsrcMem);
      return NULL;
    }
    CopyMemory((BYTE*)pTemplate, pRsrcMem, dwTemplateSize);
    UnlockResource(hRsrcMem);
    FreeResource(hRsrcMem);

    WCHAR wchFaceName[LF_FACESIZE] = { L'\0' };
    WORD wFontSize = 0;
    if (!GetThemedDialogFont(wchFaceName, &wFontSize)) {
      return(pTemplate);
    }
    bool bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
    bool bHasFont = DialogTemplate_HasFont(pTemplate);
    size_t cbFontAttr = DialogTemplate_FontAttrSize(bDialogEx);

    if (bDialogEx)
      ((DLGTEMPLATEEX*)pTemplate)->style |= DS_SHELLFONT;
    else
      pTemplate->style |= DS_SHELLFONT;

    size_t cbNew = cbFontAttr + ((StringCchLenW(wchFaceName, COUNTOF(wchFaceName)) + 1) * sizeof(WCHAR));
    BYTE* pbNew = (BYTE*)wchFaceName;

    BYTE* pb = DialogTemplate_GetFontSizeField(pTemplate);
    size_t cbOld = (bHasFont ? cbFontAttr + 2 * (StringCchLen((WCHAR*)(pb + cbFontAttr), 0) + 1) : 0);

    BYTE* pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~(DWORD_PTR)3);
    BYTE* pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~(DWORD_PTR)3);

    WORD nCtrl = (bDialogEx ? ((DLGTEMPLATEEX*)pTemplate)->cDlgItems : pTemplate->cdit);

    if (cbNew != cbOld && nCtrl > 0)
      MoveMemory(pNewControls, pOldControls, (size_t)(dwTemplateSize - (pOldControls - (BYTE*)pTemplate)));

    *(WORD*)pb = wFontSize;
    MoveMemory(pb + cbFontAttr, pbNew, (size_t)(cbNew - cbFontAttr));
  }
  return(pTemplate);
}

INT_PTR ThemedDialogBoxParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent,
                             DLGPROC lpDialogFunc, LPARAM dwInitParam) 
{
  DLGTEMPLATE* pDlgTemplate = LoadThemedDialogTemplate(lpTemplate, hInstance);
  INT_PTR ret = (INT_PTR)NULL;
  if (pDlgTemplate) {
    ret = DialogBoxIndirectParam(hInstance, pDlgTemplate, hWndParent, lpDialogFunc, dwInitParam);
    FreeMem(pDlgTemplate);
  }
  return ret;
}

HWND CreateThemedDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent,
                             DLGPROC lpDialogFunc, LPARAM dwInitParam) 
{
  DLGTEMPLATE* pDlgTemplate = LoadThemedDialogTemplate(lpTemplate, hInstance);
  HWND hwnd = INVALID_HANDLE_VALUE;
  if (pDlgTemplate) {
    hwnd = CreateDialogIndirectParam(hInstance, pDlgTemplate, hWndParent, lpDialogFunc, dwInitParam);
    FreeMem(pDlgTemplate);
  }
  return hwnd;
}

//  End of Dialogs.c
