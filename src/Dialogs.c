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
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#include "Helpers.h"

#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <shellscalingapi.h>

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
#include "Styles.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"
#include "Resample.h"

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


#ifndef TMT_MSGBOXFONT
#define TMT_MSGBOXFONT 805
#endif

//=============================================================================
//
//  MessageBoxLng()
//
static HHOOK s_hCBThook = NULL;

static LRESULT CALLBACK SetPosRelatedToParent_Hook(INT nCode, WPARAM wParam, LPARAM lParam)
{
  // notification that a window is about to be activated  
  if (nCode == HCBT_CREATEWND)
  {
    HWND const hThisWnd = (HWND)wParam;
    if (hThisWnd) {
      SetDialogIconNP3(hThisWnd);

      // get window handles
      LPCREATESTRUCT const pCreateStructure = ((LPCBT_CREATEWND)lParam)->lpcs;
      HWND const           hParentWnd       = pCreateStructure->hwndParent; // GetParent(hThisWnd);

      if (hParentWnd) {

        WININFO const winInfo = GetMyWindowPlacement(hParentWnd, NULL);
        RECT rcParent;
        rcParent.left = winInfo.x;
        rcParent.top = winInfo.y;
        rcParent.right = winInfo.x + winInfo.cx;
        rcParent.bottom = winInfo.y + winInfo.cy;

        // set new coordinates
        RECT rcDlg;
        rcDlg.left   = pCreateStructure->x;
        rcDlg.top    = pCreateStructure->y;
        rcDlg.right  = pCreateStructure->x + pCreateStructure->cx;
        rcDlg.bottom = pCreateStructure->y + pCreateStructure->cy;

        POINT const ptTL = GetCenterOfDlgInParent(&rcDlg, &rcParent);

        pCreateStructure->x = ptTL.x;
        pCreateStructure->y = ptTL.y;
      }

      // we are done
      if (s_hCBThook) {
        UnhookWindowsHookEx(s_hCBThook);
        s_hCBThook = NULL;
      }
    }
    else if (s_hCBThook) {
      // continue with any possible chained hooks
      return CallNextHookEx(s_hCBThook, nCode, wParam, lParam);
    }
  }
  return (LRESULT)0;
}
// -----------------------------------------------------------------------------


int MessageBoxLng(UINT uType, UINT uidMsg, ...)
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

  uType |= MB_SETFOREGROUND;  //~ MB_TOPMOST
  if (Settings.DialogsLayoutRTL) {
    uType |= MB_RTLREADING;
  }

  // center message box to focus or main
  HWND const focus = GetFocus();
  HWND const hwnd  = focus ? focus : Globals.hwndMain;
  s_hCBThook       = SetWindowsHookEx(WH_CBT, &SetPosRelatedToParent_Hook, 0, GetCurrentThreadId());

  return MessageBoxEx(hwnd, szText, szTitle, uType, Globals.iPrefLANGID);
}


//=============================================================================
//
//  MsgBoxLastError()
//
DWORD MsgBoxLastError(LPCWSTR lpszMessage, DWORD dwErrID)
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
    (LPWSTR)&lpMsgBuf,
    0, NULL);

  if (lpMsgBuf) {
    // Display the error message and exit the process
    size_t const len = StringCchLen((LPCWSTR)lpMsgBuf, 0) + StringCchLen(lpszMessage, 0) + 160;
    LPWSTR lpDisplayBuf = (LPWSTR)AllocMem(len * sizeof(WCHAR), HEAP_ZERO_MEMORY);

    if (lpDisplayBuf) {
      WCHAR msgFormat[128] = { L'\0' };
      GetLngString(IDS_MUI_ERR_DLG_FORMAT, msgFormat, COUNTOF(msgFormat));
      StringCchPrintf(lpDisplayBuf, len, msgFormat, lpszMessage, (LPCWSTR)lpMsgBuf, dwErrID);
      // center message box to main
      HWND const focus = GetFocus();
      HWND const hwnd = focus ? focus : Globals.hwndMain;
      s_hCBThook = SetWindowsHookEx(WH_CBT, &SetPosRelatedToParent_Hook, 0, GetCurrentThreadId());
      
      UINT uType = MB_ICONERROR | MB_TOPMOST | (Settings.DialogsLayoutRTL ? MB_RTLREADING : 0);
      MessageBoxEx(hwnd, lpDisplayBuf, _W(SAPPNAME) L" - ERROR", uType, Globals.iPrefLANGID);

      FreeMem(lpDisplayBuf);
    }
    LocalFree(lpMsgBuf); // LocalAlloc()
  }
  return dwErrID;
}


DWORD DbgMsgBoxLastError(LPCWSTR lpszMessage, DWORD dwErrID)
{
#ifdef _DEBUG
  if (!dwErrID) {
    dwErrID = GetLastError();
  }
  return MsgBoxLastError(lpszMessage, dwErrID);
#else
  UNUSED(lpszMessage);
  return dwErrID;
#endif
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
  static HBITMAP hIconBmp = NULL;
  static HICON   hBoxIcon = NULL;
  static DPI_T dpi = {USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI};
  
  switch (umsg)
  {
  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      SetDialogIconNP3(hwnd);

      LPINFOBOXLNG const lpMsgBox = (LPINFOBOXLNG)lParam;

      SetWindowLayoutRTL(hwnd, (lpMsgBox->uType & MB_RTLREADING));

      dpi = Scintilla_GetWindowDPI(hwnd);

      int const scxb = ScaleIntByDPI(GetSystemMetrics(SM_CXICON), dpi.x);
      int const scyb = ScaleIntByDPI(GetSystemMetrics(SM_CYICON), dpi.y);

      switch (lpMsgBox->uType & MB_ICONMASK)
      {
      case MB_ICONQUESTION:
        hBoxIcon = Globals.hIconMsgQuest;
        break;
      case MB_ICONWARNING:  // = MB_ICONEXCLAMATION
        hBoxIcon = Globals.hIconMsgWarn;
        break;
      case MB_ICONERROR:  // = MB_ICONSTOP, MB_ICONHAND
        hBoxIcon = Globals.hIconMsgError;
        break;
      case MB_ICONSHIELD:
        hBoxIcon = Globals.hIconMsgShield;
        break;
      case MB_USERICON:
        hBoxIcon = Globals.hIconMsgUser;
        break;
      case MB_ICONINFORMATION:  // = MB_ICONASTERISK
      default:
        hBoxIcon = Globals.hIconMsgInfo;
        break;
      }
      hIconBmp = ResampleIconToBitmap(hwnd, hBoxIcon, scxb, scyb);
      if (hIconBmp) { 
        SetBitmapControl(hwnd, IDC_INFOBOXICON, hIconBmp);
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
    return !0;


  case WM_DPICHANGED:
    {
      dpi.x = LOWORD(wParam);
      dpi.y = HIWORD(wParam);
      int const scxb = ScaleIntByDPI(GetSystemMetrics(SM_CXICON), dpi.x);
      int const scyb = ScaleIntByDPI(GetSystemMetrics(SM_CYICON), dpi.y);
      hIconBmp       = ResampleIconToBitmap(hwnd, hBoxIcon, scxb, scyb);
      if (hIconBmp) {
        SetBitmapControl(hwnd, IDC_INFOBOXICON, hIconBmp);
      }
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
    }
    return !0;

  case WM_DESTROY:
    if (hIconBmp) {
      DeleteObject(hIconBmp);
    }
    return !0;

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
        if (IsButtonChecked(hwnd, IDC_INFOBOXCHECK) && StrIsNotEmpty(lpMsgBox->lpstrSetting) && Globals.bCanSaveIniFile) {
          IniFileSetInt(Globals.IniFile, Constants.SectionSuppressedMessages, lpMsgBox->lpstrSetting, LOWORD(wParam));
        }
      case IDNO:
      case IDABORT:
      case IDCLOSE:
      case IDCANCEL:
        EndDialog(hwnd, LOWORD(wParam));
        break;

      case IDC_INFOBOXCHECK:
        DialogEnableControl(hwnd, IDNO, !IsButtonChecked(hwnd, IDC_INFOBOXCHECK));
        DialogEnableControl(hwnd, IDABORT, !IsButtonChecked(hwnd, IDC_INFOBOXCHECK));
        DialogEnableControl(hwnd, IDCLOSE, !IsButtonChecked(hwnd, IDC_INFOBOXCHECK));
        DialogEnableControl(hwnd, IDCANCEL, !IsButtonChecked(hwnd, IDC_INFOBOXCHECK));
        break;

      default:
        break;
      }
    }
    return !0;
  }
  return 0;
}


//=============================================================================
//
//  InfoBoxLng()
//
//

INT_PTR InfoBoxLng(UINT uType, LPCWSTR lpstrSetting, UINT uidMsg, ...)
{
  int const iMode = StrIsEmpty(lpstrSetting) ? 0 : IniFileGetInt(Globals.IniFile, Constants.SectionSuppressedMessages, lpstrSetting, 0);
  
  if (Settings.DialogsLayoutRTL) {
    uType |= MB_RTLREADING;
  }

  switch (iMode) 
  {
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
      if (Globals.bCanSaveIniFile) {
        IniFileDelete(Globals.IniFile, Constants.SectionSuppressedMessages, lpstrSetting, false);
      }
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
  msgBox.bDisableCheckBox = (!Globals.bCanSaveIniFile || StrIsEmpty(lpstrSetting) || (iMode < 0)) ? true : false;

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
  mbp.lpszIcon = MAKEINTRESOURCE(IDR_MAINWND);
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
      SetDialogIconNP3(hwnd);
      
      SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

      WCHAR szText[4096] = { L'\0' };
      GetLngString(IDS_MUI_CMDLINEHELP, szText, COUNTOF(szText));
      SetDlgItemText(hwnd, IDC_CMDLINEHELP, szText);
      CenterDlgInParent(hwnd, NULL);
    }
    return true;

  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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


//=============================================================================
//
//  AboutDlgProc()
//
INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static HFONT hVersionFont = NULL;
  static char pAboutResource[8192] = { '\0' };
  static char* pAboutInfo = NULL;
  static DPI_T dpi = { 0, 0 };

  switch (umsg)
  {
  case WM_INITDIALOG:
  {
    SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
    SetDialogIconNP3(hwnd);

    //~SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

    dpi = Scintilla_GetWindowDPI(hwnd);

    SetDlgItemText(hwnd, IDC_VERSION, _W(_STRG(VERSION_FILEVERSION_LONG)));
    SetDlgItemText(hwnd, IDC_SCI_VERSION, VERSION_SCIVERSION L", ID='" _W(_STRG(VERSION_COMMIT_ID)) L"'");
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
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SHOWSCROLLBAR, SB_VERT, TRUE);
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETZOOM, 1, 1); //, 0, 0); // OFF

    DWORD styleFlags = SES_EXTENDBACKCOLOR; // | SES_HYPERLINKTOOLTIPS;
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETEDITSTYLE, (WPARAM)styleFlags, (LPARAM)styleFlags);
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_AUTOURLDETECT, (WPARAM)1, (LPARAM)0);

    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETEVENTMASK, 0, (LPARAM)(ENM_LINK)); // link click

    //~if (StrIsEmptyA(pAboutResource)) { ~ maybe language resource changed, so reload
    char pAboutRes[4096];
    StringCchCopyA(pAboutResource, COUNTOF(pAboutResource), "");
    GetLngStringA(IDS_MUI_ABOUT_RTF_0, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
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
    //~}

    CenterDlgInParent(hwnd, NULL);


    HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, IDC_SCI_VERSION, WM_GETFONT, 0, 0);
    if (hFont) {
      LOGFONT lf;
      GetObject(hFont, sizeof(LOGFONT), &lf);
      lf.lfHeight = MulDiv(lf.lfHeight, 3, 2);
      lf.lfWeight = FW_BOLD;
      //lf.lfUnderline = true;
      if (hVersionFont) {
        DeleteObject(hVersionFont);
      }
      hVersionFont = CreateFontIndirectW(&lf);
      SendDlgItemMessageW(hwnd, IDC_VERSION, WM_SETFONT, (WPARAM)hVersionFont, true);
    }

    // render rich-edit control text again
    if (!StrIsEmptyA(pAboutResource)) {
      pAboutInfo              = pAboutResource;
      EDITSTREAM editStreamIn = {(DWORD_PTR)&pAboutInfo, 0, _LoadRtfCallback};
      SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, SF_RTF, (LPARAM)&editStreamIn);
    }
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SHOWSCROLLBAR, SB_HORZ, (LPARAM)(dpi.y > USER_DEFAULT_SCREEN_DPI));

    // RichEdit-Ctrl DPI-BUG: it initially uses the DPI setting of 
    // the main(1) screen instead it's current parent window screen DPI
    DPI_T const dpiPrime = Scintilla_GetWindowDPI(NULL);
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETZOOM, (WPARAM)dpi.y, (LPARAM)dpiPrime.y);

    int const width  = ScaleIntByDPI(136, dpi.x);
    int const height = ScaleIntByDPI(41, dpi.y);
    HBITMAP   hBmp   = LoadImage(Globals.hInstance, MAKEINTRESOURCE(IDR_RIZBITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    SetBitmapControlResample(hwnd, IDC_RIZONEBMP, hBmp, width, height);
    DeleteObject(hBmp);
  }
  break;

  case WM_DESTROY:
    if (hVersionFont) { DeleteObject(hVersionFont); }
    break;

  case WM_DPICHANGED:
    {
      dpi.x = LOWORD(wParam);
      dpi.y = HIWORD(wParam);

      // render rich-edit control text again
      if (!StrIsEmptyA(pAboutResource)) {
        pAboutInfo              = pAboutResource;
        EDITSTREAM editStreamIn = {(DWORD_PTR)&pAboutInfo, 0, _LoadRtfCallback};
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, SF_RTF, (LPARAM)&editStreamIn);
      }
      SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SHOWSCROLLBAR, SB_HORZ, (LPARAM)(dpi.y > USER_DEFAULT_SCREEN_DPI));
      //~SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETZOOM, (WPARAM)dpi.y, (LPARAM)USER_DEFAULT_SCREEN_DPI);

      int const width  = ScaleIntByDPI(136, dpi.x);
      int const height = ScaleIntByDPI(41, dpi.y);
      HBITMAP   hBmp   = LoadImage(Globals.hInstance, MAKEINTRESOURCE(IDR_RIZBITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
      SetBitmapControlResample(hwnd, IDC_RIZONEBMP, hBmp, width, height);
      DeleteObject(hBmp);

      HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, IDC_SCI_VERSION, WM_GETFONT, 0, 0);
      if (hFont) {
        LOGFONT lf;
        GetObject(hFont, sizeof(LOGFONT), &lf);
        lf.lfHeight = MulDiv(lf.lfHeight, 3, 2);
        lf.lfWeight = FW_BOLD;
        //lf.lfUnderline = true;
        if (hVersionFont) {
          DeleteObject(hVersionFont);
        }
        hVersionFont = CreateFontIndirectW(&lf);
        SendDlgItemMessageW(hwnd, IDC_VERSION, WM_SETFONT, (WPARAM)hVersionFont, true);
      }

      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
    }
    break;

  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC const hdc = GetDC(hwnd);  // ClientArea
      if (hdc) {
        BeginPaint(hwnd, &ps);
        SetMapMode(hdc, MM_TEXT);

        int const   iconSize  = 128;
        int const   dpiWidth  = ScaleIntByDPI(iconSize, dpi.x);
        int const   dpiHeight = ScaleIntByDPI(iconSize, dpi.y);
        HICON const hicon     = (dpiHeight > 128) ? Globals.hDlgIcon256 : Globals.hDlgIcon128;
        if (hicon) {
          //RECT rc = {0};
          //MapWindowPoints(GetDlgItem(hwnd, IDC_INFO_GROUPBOX), hwnd, (LPPOINT)&rc, 2);
          DrawIconEx(hdc, ScaleIntByDPI(10, dpi.x), ScaleIntByDPI(10, dpi.x), hicon, dpiWidth, dpiHeight, 0, NULL, DI_NORMAL);
        }

        ReleaseDC(hwnd, hdc);
        EndPaint(hwnd, &ps);
      }
    }
    return 0;


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

        int ResX, ResY;
        GetCurrentMonitorResolution(Globals.hwndMain, &ResX, &ResY);

        // --------------------------------------------------------------------

        StringCchCopy(wchVerInfo, COUNTOF(wchVerInfo), _W(_STRG(VERSION_FILEVERSION_LONG)) L" (" _W(_STRG(VERSION_COMMIT_ID)) L")");
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_COMPILER);

        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n");
        GetWinVersionString(wchBuf, COUNTOF(wchBuf));
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_SCIVERSION);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_ONIGURUMA);

        StringCchCopy(wchBuf, COUNTOF(wchBuf), L"en-US");
        for (int lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
          if (MUI_LanguageDLLs[lng].bIsActive) {
            StringCchCopy(wchBuf, COUNTOF(wchBuf), MUI_LanguageDLLs[lng].szLocaleName);
            break;
          }
        }
        StringCchPrintf(wchBuf2, ARRAYSIZE(wchBuf2), L"\n- Locale -> %s (CP:'%s')", 
          wchBuf, g_Encodings[CPI_ANSI_DEFAULT].wchLabel);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf2);

        StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Current Encoding -> '%s'", Encoding_GetLabel(Encoding_GetCurrent()));
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

        StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Screen-Resolution -> %i x %i [pix]", ResX, ResY);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

        StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Display-DPI -> %i x %i  (Scale: %i%%).", dpi.x, dpi.y, ScaleIntToDPI_X(hwnd, 100));
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

        StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Rendering-Technology -> '%s'", Settings.RenderingTechnology ? L"DIRECT-WRITE" : L"GDI");
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

        StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Zoom -> %i%%.", SciCall_GetZoom());
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), (IsProcessElevated() ?
                                                       L"\n- Process is elevated." : 
                                                       L"\n- Process is not elevated"));
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), (IsUserInAdminGroup() ?
                                                       L"\n- User is in Admin-Group." :
                                                       L"\n- User is not in Admin-Group"));

        Style_GetLexerDisplayName(Style_GetCurrentLexerPtr(), wchBuf, COUNTOF(wchBuf));
        StringCchPrintf(wchBuf2, ARRAYSIZE(wchBuf2), L"\n- Current Lexer -> '%s'", wchBuf);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf2);

        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n");

        // --------------------------------------------------------------------

        SetClipboardTextW(Globals.hwndMain, wchVerInfo, StringCchLen(wchVerInfo,0));
      }
      break;

    case IDOK:
    case IDCANCEL:
      EndDialog(hwnd, IDOK);
      break;
    }
    return !0;
  }
  return 0;
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
      SetDialogIconNP3(hwnd);

      SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

      // MakeBitmapButton(hwnd,IDC_SEARCHEXE,IDB_OPEN, -1, -1);
      SendDlgItemMessage(hwnd, IDC_COMMANDLINE, EM_LIMITTEXT, MAX_PATH - 1, 0);
      SetDlgItemText(hwnd, IDC_COMMANDLINE, (LPCWSTR)lParam);
      SHAutoComplete(GetDlgItem(hwnd, IDC_COMMANDLINE), SHACF_FILESYSTEM);

      CenterDlgInParent(hwnd, NULL);
    }
    return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
              StringCchCompareNI(arg1, COUNTOF(arg1), L"Notepad3.exe", CSTRLEN(L"Notepad3.exe")) == 0) {
              GetModuleFileName(NULL, arg1, COUNTOF(arg1));
              PathCanonicalizeEx(arg1, COUNTOF(arg1));
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
  static HWND hwndLV = NULL;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

        ResizeDlg_Init(hwnd, Settings.OpenWithDlgSizeX, Settings.OpenWithDlgSizeY, IDC_RESIZEGRIP);

        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        hwndLV = GetDlgItem(hwnd, IDC_OPENWITHDIR);
        InitWindowCommon(hwndLV, false);
        ListView_SetExtendedListViewStyle(hwndLV, /*LVS_EX_FULLROWSELECT|*/ LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV, 0, &lvc);
        DirList_Init(hwndLV, NULL);
        DirList_Fill(hwndLV, Settings.OpenWithDir, DL_ALLOBJECTS, NULL, false, Flags.NoFadeHidden, DS_NAME, false);
        DirList_StartIconThread(hwndLV);
        ListView_SetItemState(hwndLV, 0, LVIS_FOCUSED, LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETOPENWITHDIR,IDB_OPEN, -1, -1);

        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
      return true;


    case WM_DESTROY:
      DirList_Destroy(hwndLV);
      hwndLV = NULL;
      DeleteBitmapButton(hwnd,IDC_GETOPENWITHDIR);
      ResizeDlg_Destroy(hwnd,&Settings.OpenWithDlgSizeX,&Settings.OpenWithDlgSizeY);
      return false;


    case WM_SIZE:
      {
        int dx, dy;
        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        HDWP hdwp;
        hdwp = BeginDeferWindowPos(6);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDOK,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDCANCEL,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_OPENWITHDIR,dx,dy,SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_GETOPENWITHDIR,0,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_OPENWITHDESCR,0,dy,SWP_NOSIZE);
        EndDeferWindowPos(hdwp);

        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);
      }
      return !0;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd,lParam);
      return !0;


    case WM_NOTIFY:
      {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_OPENWITHDIR)
        {
          switch(pnmh->code)
          {
            case LVN_GETDISPINFO:
              DirList_GetDispInfo(hwndLV, lParam, Flags.NoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(hwndLV, lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;
                DialogEnableControl(hwnd,IDOK,(pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
                if (ListView_GetSelectedCount(hwndLV)) {
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
              DirList_Fill(hwndLV, Settings.OpenWithDir, DL_ALLOBJECTS, NULL, false, Flags.NoFadeHidden, DS_NAME, false);
              DirList_StartIconThread(hwndLV);
              ListView_EnsureVisible(hwndLV, 0, false);
              ListView_SetItemState(hwndLV, 0, LVIS_FOCUSED, LVIS_FOCUSED);
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(hwndLV), 1);
          }
          break;


        case IDOK: {
            LPDLITEM lpdli = (LPDLITEM)GetWindowLongPtr(hwnd,DWLP_USER);
            lpdli->mask = DLI_FILENAME | DLI_TYPE;
            lpdli->ntype = DLE_NONE;
            DirList_GetItem(hwndLV, (-1), lpdli);

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
  static HWND hwndLV = NULL;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

        ResizeDlg_Init(hwnd, Settings.FavoritesDlgSizeX, Settings.FavoritesDlgSizeY, IDC_RESIZEGRIP);

        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

		    hwndLV = GetDlgItem(hwnd, IDC_FAVORITESDIR);
        InitWindowCommon(hwndLV, false);

        ListView_SetExtendedListViewStyle(hwndLV,/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV,0,&lvc);
        DirList_Init(hwndLV,NULL);
        DirList_Fill(hwndLV,Settings.FavoritesDir,DL_ALLOBJECTS,NULL,false,Flags.NoFadeHidden,DS_NAME,false);
        DirList_StartIconThread(hwndLV);
        ListView_SetItemState(hwndLV,0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETFAVORITESDIR,IDB_OPEN, -1, -1);

        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
      return true;


    case WM_DESTROY:
      DirList_Destroy(hwndLV);
      hwndLV = NULL;
      DeleteBitmapButton(hwnd,IDC_GETFAVORITESDIR);
      ResizeDlg_Destroy(hwnd,&Settings.FavoritesDlgSizeX,&Settings.FavoritesDlgSizeY);
      return false;


    case WM_SIZE:
      {
        int dx, dy;
        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        HDWP hdwp;
        hdwp = BeginDeferWindowPos(6);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDOK,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDCANCEL,dx,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_FAVORITESDIR,dx,dy,SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_GETFAVORITESDIR,0,dy,SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_FAVORITESDESCR,0,dy,SWP_NOSIZE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(hwndLV,0,LVSCW_AUTOSIZE_USEHEADER);
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
              DirList_GetDispInfo(hwndLV, lParam, Flags.NoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(hwndLV, lParam);
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
              DirList_Fill(hwndLV,Settings.FavoritesDir,DL_ALLOBJECTS,NULL,false,Flags.NoFadeHidden,DS_NAME,false);
              DirList_StartIconThread(hwndLV);
              ListView_EnsureVisible(hwndLV,0,false);
              ListView_SetItemState(hwndLV,0,LVIS_FOCUSED,LVIS_FOCUSED);
            }
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(hwndLV),1);
          }
          break;


        case IDOK: {
            LPDLITEM lpdli = (LPDLITEM)GetWindowLongPtr(hwnd,DWLP_USER);
            lpdli->mask = DLI_FILENAME | DLI_TYPE;
            lpdli->ntype = DLE_NONE;
            DirList_GetItem(hwndLV,(-1),lpdli);

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
      SetDialogIconNP3(hwnd);

      SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

      ResizeDlg_InitX(hwnd, Settings.AddToFavDlgSizeX, IDC_RESIZEGRIP);

      LPCWSTR const pszName = (LPCWSTR)lParam;
      SendDlgItemMessage(hwnd, IDC_ADDFAV_FILES, EM_LIMITTEXT, MAX_PATH - 1, 0);
      SetDlgItemText(hwnd, IDC_ADDFAV_FILES, pszName);

      CenterDlgInParent(hwnd, NULL);
    }
    return !0;


  case WM_DESTROY:
    ResizeDlg_Destroy(hwnd, &Settings.AddToFavDlgSizeX, NULL);
    return FALSE;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
    break;


  case WM_SIZE:
  {
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
    return !0;


  case WM_GETMINMAXINFO:
    ResizeDlg_GetMinMaxInfo(hwnd, lParam);
    return !0;


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
    return !0;
  }
  return 0;
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
      if (PathIsUNC(tch) || !PathIsExistingFile(tch)) {
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

static INT_PTR CALLBACK FileMRUDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static HWND hwndIL = NULL;

  switch (umsg) {
    case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      SetDialogIconNP3(hwnd);

      SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

      // sync with other instances
      if (Settings.SaveRecentFiles && Globals.bCanSaveIniFile) {
        if (MRU_MergeSave(Globals.pFileMRU, true, Flags.RelativeFileMRU, Flags.PortableMyDocs)) {
          MRU_Load(Globals.pFileMRU, true);
        }
      }

		  hwndIL = GetDlgItem(hwnd, IDC_FILEMRU);
      InitWindowCommon(hwndIL, false);

      SHFILEINFO shfi;
      ZeroMemory(&shfi, sizeof(SHFILEINFO));
      LVCOLUMN lvc = {LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0};

      LPICONTHREADINFO lpit = (LPICONTHREADINFO)AllocMem(sizeof(ICONTHREADINFO), HEAP_ZERO_MEMORY);
      if (lpit) {
        SetProp(hwnd, L"it", (HANDLE)lpit);
        lpit->hwnd              = hwndIL;
        lpit->hThread           = NULL;
        lpit->hExitThread       = CreateEvent(NULL, true, false, NULL);
        lpit->hTerminatedThread = CreateEvent(NULL, true, true, NULL);
      }
      ResizeDlg_Init(hwnd, Settings.FileMRUDlgSizeX, Settings.FileMRUDlgSizeY, IDC_RESIZEGRIP);


      ListView_SetImageList(hwndIL,
                            (HIMAGELIST)SHGetFileInfo(L"C:\\", FILE_ATTRIBUTE_DIRECTORY,
                                                      &shfi, sizeof(SHFILEINFO), SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
                            LVSIL_SMALL);

      ListView_SetImageList(hwndIL,
                            (HIMAGELIST)SHGetFileInfo(L"C:\\", FILE_ATTRIBUTE_DIRECTORY,
                                                      &shfi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
                            LVSIL_NORMAL);

      //SetExplorerTheme(GetDlgItem(hwnd,IDC_FILEMRU));
      ListView_SetExtendedListViewStyle(hwndIL, /*LVS_EX_FULLROWSELECT|*/ LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
      ListView_InsertColumn(hwndIL, 0, &lvc);

      // Update view
      SendWMCommand(hwnd, IDC_FILEMRU_UPDATE_VIEW);

      CheckDlgButton(hwnd, IDC_SAVEMRU, SetBtn(Settings.SaveRecentFiles));
      CheckDlgButton(hwnd, IDC_PRESERVECARET, SetBtn(Settings.PreserveCaretPos));
      CheckDlgButton(hwnd, IDC_REMEMBERSEARCHPATTERN, SetBtn(Settings.SaveFindReplace));

      DialogEnableControl(hwnd, IDC_PRESERVECARET, Settings.SaveRecentFiles);

      CenterDlgInParent(hwnd, NULL);
    }
      return !0;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
      return !0;

    case WM_DESTROY:
    {
      LPICONTHREADINFO lpit = (LPVOID)GetProp(hwnd, L"it");
      SetEvent(lpit->hExitThread);
      while (WaitForSingleObject(lpit->hTerminatedThread, 0) != WAIT_OBJECT_0) {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      CloseHandle(lpit->hExitThread);
      CloseHandle(lpit->hTerminatedThread);
      lpit->hThread = NULL;
      RemoveProp(hwnd, L"it");
      FreeMem(lpit);

      if (Settings.SaveRecentFiles) {
        MRU_Save(Globals.pFileMRU); // last instance on save wins
      }

      Settings.SaveRecentFiles  = IsButtonChecked(hwnd, IDC_SAVEMRU);
      Settings.SaveFindReplace  = IsButtonChecked(hwnd, IDC_REMEMBERSEARCHPATTERN);
      Settings.PreserveCaretPos = IsButtonChecked(hwnd, IDC_PRESERVECARET);

      ResizeDlg_Destroy(hwnd, &Settings.FileMRUDlgSizeX, &Settings.FileMRUDlgSizeY);
    }
      return 0;

    case WM_SIZE:
    {
      int dx, dy;
      ResizeDlg_Size(hwnd, lParam, &dx, &dy);
      HDWP hdwp = BeginDeferWindowPos(8);
      hdwp      = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP, dx, dy, SWP_NOSIZE);
      hdwp      = DeferCtlPos(hdwp, hwnd, IDOK, dx, dy, SWP_NOSIZE);
      hdwp      = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, dy, SWP_NOSIZE);
      hdwp      = DeferCtlPos(hdwp, hwnd, IDC_REMOVE, dx, dy, SWP_NOSIZE);
      hdwp      = DeferCtlPos(hdwp, hwnd, IDC_FILEMRU, dx, dy, SWP_NOMOVE);
      hdwp      = DeferCtlPos(hdwp, hwnd, IDC_SAVEMRU, 0, dy, SWP_NOSIZE);
      hdwp      = DeferCtlPos(hdwp, hwnd, IDC_PRESERVECARET, 0, dy, SWP_NOSIZE);
      hdwp      = DeferCtlPos(hdwp, hwnd, IDC_REMEMBERSEARCHPATTERN, 0, dy, SWP_NOSIZE);
      EndDeferWindowPos(hdwp);
      ListView_SetColumnWidth(hwndIL, 0, LVSCW_AUTOSIZE_USEHEADER);
    }
      return !0;

    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd, lParam);
      return !0;

    case WM_NOTIFY:
    {
      switch (wParam)
      {
        case IDC_REMOVE:
          switch (((LPNMHDR)lParam)->code) {
            case BCN_DROPDOWN:
            {
              const NMBCDROPDOWN* pDropDown = (NMBCDROPDOWN*)lParam;
              // Get screen coordinates of the button.
              POINT pt;
              pt.x = pDropDown->rcButton.left;
              pt.y = pDropDown->rcButton.bottom;
              ClientToScreen(pDropDown->hdr.hwndFrom, &pt);
              // Create a menu and add items.
              HMENU hSplitMenu = CreatePopupMenu();
              if (!hSplitMenu)
                break;
              if (pDropDown->hdr.hwndFrom == GetDlgItem(hwnd, IDC_REMOVE)) {
                WCHAR szMenu[80] = {L'\0'};
                GetLngString(IDS_CLEAR_ALL, szMenu, COUNTOF(szMenu));
                AppendMenu(hSplitMenu, MF_STRING, IDC_CLEAR_LIST, szMenu);
              }

              // Display the menu.
              TrackPopupMenu(hSplitMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwnd, NULL);
              DestroyMenu(hSplitMenu);
              return !0;
            } 
            break;

            default:
              break;
          }
          break;

        case IDC_FILEMRU:
          if (((LPNMHDR)(lParam))->idFrom == IDC_FILEMRU)
          {
            switch (((LPNMHDR)(lParam))->code)
            {
              case NM_DBLCLK:
                SendWMCommand(hwnd, IDOK);
                break;

              case LVN_GETDISPINFO:
              {
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

                  if (!PathIsExistingFile(tch)) {
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
                UINT const cnt = ListView_GetSelectedCount(hwndIL);
                DialogEnableControl(hwnd, IDOK, (cnt > 0));
                // can't discard current file (myself)
                int cur = 0;
                if (!MRU_FindFile(Globals.pFileMRU, Globals.CurrentFile, &cur)) {
                  cur = -1;
                }
                int const item = ListView_GetNextItem(hwndIL, -1, LVNI_ALL | LVNI_SELECTED);
                DialogEnableControl(hwnd, IDC_REMOVE, (cnt > 0) && (cur != item));
              }
              break;
            }
          }
          break;
          
        default:
          break;
      }
    }
    return !0;

    case WM_COMMAND:

      switch (LOWORD(wParam))
      {
        case IDC_FILEMRU_UPDATE_VIEW:
        {
          LPICONTHREADINFO lpit = (LPVOID)GetProp(hwnd, L"it");
          SetEvent(lpit->hExitThread);
          while (WaitForSingleObject(lpit->hTerminatedThread, 0) != WAIT_OBJECT_0) {
            MSG msg;
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
              TranslateMessage(&msg);
              DispatchMessage(&msg);
            }
          }
          ResetEvent(lpit->hExitThread);
          SetEvent(lpit->hTerminatedThread);
          lpit->hThread = NULL;

          ListView_DeleteAllItems(hwndIL);

          LV_ITEM lvi;
          ZeroMemory(&lvi, sizeof(LV_ITEM));
          lvi.mask = LVIF_TEXT | LVIF_IMAGE;

          SHFILEINFO shfi;
          ZeroMemory(&shfi, sizeof(SHFILEINFO));
          SHGetFileInfo(L"Icon", FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO),
                        SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);

          lvi.iImage = shfi.iIcon;

          WCHAR tch[MAX_PATH] = { L'\0' };
          for (int i = 0; i < MRU_Count(Globals.pFileMRU); i++) {
            MRU_Enum(Globals.pFileMRU, i, tch, COUNTOF(tch));
            PathAbsoluteFromApp(tch, NULL, 0, true);
            //  SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_ADDSTRING,0,(LPARAM)tch); }
            //  SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_SETCARETINDEX,0,false);
            lvi.iItem   = i;
            lvi.pszText = tch;
            ListView_InsertItem(hwndIL, &lvi);
          }

          UINT const cnt = ListView_GetItemCount(hwndIL);
          if (cnt > 0) {
            UINT idx = ListView_GetTopIndex(hwndIL);
            ListView_SetColumnWidth(hwndIL, idx, LVSCW_AUTOSIZE_USEHEADER);
            ListView_SetItemState(hwndIL, ((cnt > 1) ? idx + 1 : idx), LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            //int cur = 0;
            //if (!MRU_FindFile(Globals.pFileMRU, Globals.CurrentFile, &cur)) { cur = -1; }
            //int const item = ListView_GetNextItem(hwndIL, -1, LVNI_ALL | LVNI_SELECTED);
            //if ((cur == item) && (cnt > 1)) {
            //  ListView_SetItemState(hwndIL, idx + 1, LVIS_SELECTED, LVIS_SELECTED);
            //}
          }

          DWORD dwtid;
          lpit->hThread = CreateThread(NULL, 0, FileMRUIconThread, (LPVOID)lpit, 0, &dwtid);

          DialogEnableControl(hwnd, IDOK, (cnt > 0));
          DialogEnableControl(hwnd, IDC_REMOVE, (cnt > 0));
        } 
        break;

        case IDC_FILEMRU:
          break;

        case IDC_SAVEMRU:
        {
          bool const bSaveMRU = IsButtonChecked(hwnd, IDC_SAVEMRU);
          DialogEnableControl(hwnd, IDC_PRESERVECARET, bSaveMRU);
        } break;

        case IDOK:
        case IDC_REMOVE:
        {
          WCHAR tchFileName[MAX_PATH] = {L'\0'};

          if (ListView_GetSelectedCount(hwndIL)) {

            LV_ITEM lvi;
            ZeroMemory(&lvi, sizeof(LV_ITEM));
            lvi.mask = LVIF_TEXT;
            lvi.pszText = tchFileName;
            lvi.cchTextMax = COUNTOF(tchFileName);
            lvi.iItem = ListView_GetNextItem(hwndIL, -1, LVNI_ALL | LVNI_SELECTED);

            ListView_GetItem(hwndIL, &lvi);

            PathUnquoteSpaces(tchFileName);

            if (!PathIsExistingFile(tchFileName) || (LOWORD(wParam) == IDC_REMOVE)) {
              // don't remove myself
              int iCur = 0;
              if (!MRU_FindFile(Globals.pFileMRU, Globals.CurrentFile, &iCur)) {
                iCur = -1;
              }

              // Ask...
              INT_PTR const answer = (LOWORD(wParam) == IDOK) ? InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_ERR_MRUDLG)
                                                              : ((iCur == lvi.iItem) ? IDNO : IDYES);

              if ((IDOK == answer) || (IDYES == answer))
              {
                MRU_Delete(Globals.pFileMRU, lvi.iItem);
                //SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_DELETESTRING,(WPARAM)iItem,0);
                //ListView_DeleteItem(GetDlgItem(hwnd,IDC_FILEMRU),lvi.iItem);
                //DialogEnableWindow(hwnd,IDOK,
                //  (LB_ERR != SendDlgItemMessage(hwnd,IDC_GOTO,LB_GETCURSEL,0,0)));
              }
            } 
            else { // file to load
              StringCchCopy((LPWSTR)GetWindowLongPtr(hwnd, DWLP_USER), MAX_PATH, tchFileName);
              EndDialog(hwnd, IDOK);
            }

            // must use IDM_VIEW_REFRESH, index might change...
            SendWMCommand(hwnd, IDC_FILEMRU_UPDATE_VIEW);
          }

          if (Settings.SaveRecentFiles && Globals.bCanSaveIniFile) {
            MRU_MergeSave(Globals.pFileMRU, true, Flags.RelativeFileMRU, Flags.PortableMyDocs);
          }

        } break;

        case IDC_CLEAR_LIST:
          ListView_DeleteAllItems(hwndIL);
          MRU_Empty(Globals.pFileMRU, StrIsNotEmpty(Globals.CurrentFile));
          if (Globals.bCanSaveIniFile) {
            MRU_Save(Globals.pFileMRU);
          }
          SendWMCommand(hwnd, IDC_FILEMRU_UPDATE_VIEW);
          break;

        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
      }
      return !0;
  }
  return 0;
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
      SetDialogIconNP3(hwnd);

      SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

      CheckRadioButton(hwnd, 100, 102, 100 + Settings.FileWatchingMode);
      if (Settings.ResetFileWatching) {
        CheckDlgButton(hwnd, 103, BST_CHECKED);
      }
      CenterDlgInParent(hwnd, NULL);
    }
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
          if (!FileWatching.MonitoringLog) {
            FileWatching.FileWatchingMode = Settings.FileWatchingMode;
          }

          Settings.ResetFileWatching = IsButtonChecked(hwnd, 103);

          if (!FileWatching.MonitoringLog) {
            FileWatching.ResetFileWatching = Settings.ResetFileWatching;
          }

          if (FileWatching.MonitoringLog) {
            PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
          }

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
      SetDialogIconNP3(hwnd);

      SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

      UINT const uiNumber = *((UINT*)lParam);
      SetDlgItemInt(hwnd, IDC_COLUMNWRAP, uiNumber, false);
      SendDlgItemMessage(hwnd, IDC_COLUMNWRAP, EM_LIMITTEXT, 15, 0);
      CenterDlgInParent(hwnd, NULL);
    }
    return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
      SetDialogIconNP3(hwnd);

      SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

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
    UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
//  MIDSZ_BUFFER
//
static INT_PTR CALLBACK LongLineSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) {

  case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      SetDialogIconNP3(hwnd);

      SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

      LPWSTR pszColumnList = (LPWSTR)lParam;
      SetDlgItemText(hwnd, IDC_MULTIEDGELINE, pszColumnList);
      SendDlgItemMessage(hwnd, IDC_MULTIEDGELINE, EM_LIMITTEXT, MIDSZ_BUFFER, 0);

      BOOL fTranslated;
      /*UINT const iCol = */ GetDlgItemInt(hwnd, IDC_MULTIEDGELINE, &fTranslated, FALSE);
      if (fTranslated) {
        switch (Settings.LongLineMode) {
          case EDGE_BACKGROUND:
            CheckRadioButton(hwnd, IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR, IDC_BACKGRDCOLOR);
            break;
          default:
            CheckRadioButton(hwnd, IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR, IDC_SHOWEDGELINE);
            break;
        }
      }
      else {
        CheckRadioButton(hwnd, IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR, IDC_SHOWEDGELINE);
        DialogEnableControl(hwnd, IDC_SHOWEDGELINE, false);
        DialogEnableControl(hwnd, IDC_BACKGRDCOLOR, false);
      }
      CenterDlgInParent(hwnd, NULL);
    }
    return true;


  case WM_DPICHANGED:
    UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
    return true;


  case WM_COMMAND:

    switch (LOWORD(wParam)) {

    case IDC_MULTIEDGELINE:
      {
        BOOL fTranslated;
        /*UINT const iCol = */ GetDlgItemInt(hwnd, IDC_MULTIEDGELINE, &fTranslated, FALSE);
        if (fTranslated) {
          DialogEnableControl(hwnd, IDC_SHOWEDGELINE, true);
          DialogEnableControl(hwnd, IDC_BACKGRDCOLOR, true);
          CheckRadioButton(hwnd, IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR,
            (Settings.LongLineMode == EDGE_LINE) ? IDC_SHOWEDGELINE : IDC_BACKGRDCOLOR);
        }
        else {
          DialogEnableControl(hwnd, IDC_SHOWEDGELINE, false);
          DialogEnableControl(hwnd, IDC_BACKGRDCOLOR, false);
          CheckRadioButton(hwnd, IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR, IDC_SHOWEDGELINE);
        }
      }
      break;

    case IDC_SHOWEDGELINE:
    case IDC_BACKGRDCOLOR:
      if (IsDialogItemEnabled(hwnd, IDC_SHOWEDGELINE))
      {
        Settings.LongLineMode = IsButtonChecked(hwnd, IDC_SHOWEDGELINE) ? EDGE_LINE : EDGE_BACKGROUND;
      }
      break;

    case IDOK:
      {
        WCHAR wchColumnList[MIDSZ_BUFFER];
        GetDlgItemText(hwnd, IDC_MULTIEDGELINE, wchColumnList, MIDSZ_BUFFER);

        bool const bOkay = true; // TODO: parse list OK
        if (bOkay)
        {
          LPWSTR pszColumnList = (LPWSTR)GetWindowLongPtr(hwnd, DWLP_USER);
          StringCchCopy(pszColumnList, MIDSZ_BUFFER, wchColumnList);
          Settings.LongLineMode = IsButtonChecked(hwnd, IDC_SHOWEDGELINE) ? EDGE_LINE : EDGE_BACKGROUND;
          EndDialog(hwnd, IDOK);
        }
        else {
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_MULTIEDGELINE)), 1);
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
bool LongLineSettingsDlg(HWND hwnd,UINT uidDlg, LPWSTR pColList)
{
  INT_PTR const iResult = ThemedDialogBoxParam(
                              Globals.hLngResContainer,
                              MAKEINTRESOURCE(uidDlg),
                              hwnd,
                              LongLineSettingsDlgProc, (LPARAM)pColList);

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
        SetDialogIconNP3(hwnd);

        SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

        SetDlgItemInt(hwnd, IDC_TAB_WIDTH, Globals.fvCurFile.iTabWidth, false);
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
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
        SetDialogIconNP3(hwnd);

        SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

        PENCODEDLG const pdd  = (PENCODEDLG)lParam;
        HBITMAP hbmp = LoadImage(Globals.hInstance, MAKEINTRESOURCE(IDB_ENCODING), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        hbmp = ResampleImageBitmap(hwnd, hbmp, -1, -1);

        HIMAGELIST himl = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
        ImageList_AddMasked(himl, hbmp, CLR_DEFAULT);
        DeleteObject(hbmp);
        SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CBEM_SETIMAGELIST, 0, (LPARAM)himl);
        SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CB_SETEXTENDEDUI, true, 0);

        Encoding_AddToComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), pdd->idEncoding, 0);

        Encoding_GetFromComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), &s_iEnc);
        s_bLoadASCIIasUTF8 = Settings.LoadASCIIasUTF8;
        s_bUseAsFallback = Encoding_IsASCII(s_iEnc) ? Settings.UseDefaultForFileEncoding : false;

        DialogEnableControl(hwnd, IDC_USEASREADINGFALLBACK, Encoding_IsASCII(s_iEnc));
        CheckDlgButton(hwnd, IDC_USEASREADINGFALLBACK, SetBtn(s_bUseAsFallback));

        CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(s_bLoadASCIIasUTF8));
        CheckDlgButton(hwnd, IDC_RELIABLE_DETECTION_RES, SetBtn(Settings.UseReliableCEDonly));
        CheckDlgButton(hwnd, IDC_NFOASOEM, SetBtn(Settings.LoadNFOasOEM));
        CheckDlgButton(hwnd, IDC_ENCODINGFROMFILEVARS, SetBtn(!Settings.NoEncodingTags));
        CheckDlgButton(hwnd, IDC_NOUNICODEDETECTION, SetBtn(!Settings.SkipUnicodeDetection));
        CheckDlgButton(hwnd, IDC_NOANSICPDETECTION, SetBtn(!Settings.SkipANSICodePageDetection));


        CenterDlgInParent(hwnd, NULL);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
      return true;


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_ENCODINGLIST:
        case IDC_USEASREADINGFALLBACK:
        case IDC_ASCIIASUTF8:
          {
            Encoding_GetFromComboboxEx(GetDlgItem(hwnd, IDC_ENCODINGLIST), &s_iEnc);

            s_bUseAsFallback = Encoding_IsASCII(s_iEnc) ? IsButtonChecked(hwnd, IDC_USEASREADINGFALLBACK) : false;
            s_bLoadASCIIasUTF8 = IsButtonChecked(hwnd, IDC_ASCIIASUTF8);

            DialogEnableControl(hwnd, IDC_USEASREADINGFALLBACK, Encoding_IsASCII(s_iEnc));
            CheckDlgButton(hwnd, IDC_USEASREADINGFALLBACK, SetBtn(s_bUseAsFallback));

            DialogEnableControl(hwnd, IDC_ASCIIASUTF8, true);
            CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(s_bLoadASCIIasUTF8));

            if (s_iEnc == CPI_UTF8) {
              if (s_bUseAsFallback) {
                s_bLoadASCIIasUTF8 = true;
                DialogEnableControl(hwnd, IDC_ASCIIASUTF8, false);
                CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(s_bLoadASCIIasUTF8));
              }
            }
            else if (s_iEnc == CPI_ANSI_DEFAULT) {
              if (s_bUseAsFallback) {
                s_bLoadASCIIasUTF8 = false;
                DialogEnableControl(hwnd, IDC_ASCIIASUTF8, false);
                CheckDlgButton(hwnd, IDC_ASCIIASUTF8, SetBtn(s_bLoadASCIIasUTF8));
              }
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
  static HWND hwndLV = NULL;
  static HIMAGELIST himl   = NULL;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

        PENCODEDLG const pdd = (PENCODEDLG)lParam;
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };
        ResizeDlg_Init(hwnd, pdd->cxDlg, pdd->cyDlg, IDC_RESIZEGRIP);

        hwndLV = GetDlgItem(hwnd, IDC_ENCODINGLIST);
        InitWindowCommon(hwndLV, false);

        HBITMAP hbmp = LoadImage(Globals.hInstance,MAKEINTRESOURCE(IDB_ENCODING),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
        hbmp = ResampleImageBitmap(hwnd, hbmp, -1, -1);

        himl = ImageList_Create(16,16,ILC_COLOR32|ILC_MASK,0,0);
        ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
        DeleteObject(hbmp);
        ListView_SetImageList(hwndLV, himl, LVSIL_SMALL);

        ListView_SetExtendedListViewStyle(hwndLV,/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV,0,&lvc);

        Encoding_AddToListView(hwndLV,pdd->idEncoding,pdd->bRecodeOnly);

        ListView_SetColumnWidth(hwndLV,0,LVSCW_AUTOSIZE_USEHEADER);

        CenterDlgInParent(hwnd, NULL);
      }
      return !0;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
      return !0;


    case WM_DESTROY: 
      {
        ImageList_Destroy(himl);
        himl = NULL;
        PENCODEDLG pdd = (PENCODEDLG)GetWindowLongPtr(hwnd, DWLP_USER);
        ResizeDlg_Destroy(hwnd, &pdd->cxDlg, &pdd->cyDlg);
      }
      return 0;


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
        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);
      }
      return !0;


    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd,lParam);
      return !0;


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
      return !0;


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
              PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hwndLV, 1);
            }
          }
          break;

        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

        default:
          break;
      }
      return !0;
  }
  return 0;
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
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

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
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
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
    SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
    SetDialogIconNP3(hwnd);

    SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

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
    SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
    SetDialogIconNP3(hwnd);

    SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);

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
  WINDOWPLACEMENT wndpl = {0};
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
    RECT rc = {0}; 
    if (hwnd) {
      GetWindowRect(hwnd, &rc);
    }
    else {
      GetWindowRect(GetDesktopWindow(), &rc);
    }
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
void DialogNewWindow(HWND hwnd, bool bSaveOnRunTools, LPCWSTR lpcwFilePath)
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
    StringCchCat(szParameters, COUNTOF(szParameters), L"\"");
  }
  else {
    StringCchCat(szParameters, COUNTOF(szParameters), L"0");
  }
  StringCchCat(szParameters, COUNTOF(szParameters), L" -n");

  MONITORINFO mi;
  WININFO wi = GetMyWindowPlacement(hwnd, &mi);
  //~ offset new window position +10/+10
  //~wi.x += 10;
  //~wi.y += 10;
  //~// check if window fits monitor
  //~if ((wi.x + wi.cx) > mi.rcWork.right || (wi.y + wi.cy) > mi.rcWork.bottom) {
  //~  wi.x = mi.rcMonitor.left;
  //~  wi.y = mi.rcMonitor.top;
  //~}
  //~wi.max = IsZoomed(hwnd);

  StringCchPrintf(tch, COUNTOF(tch), L" -pos %i,%i,%i,%i,%i", wi.x, wi.y, wi.cx, wi.cy, wi.max);
  StringCchCat(szParameters, COUNTOF(szParameters), tch);

  if (StrIsNotEmpty(lpcwFilePath))
  {
    WCHAR szFileName[MAX_PATH] = { L'\0' };
    StringCchCopy(szFileName, COUNTOF(szFileName), lpcwFilePath);
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
  WCHAR tchTemp[MAX_PATH] = { L'\0' };
  WCHAR tchParam[MAX_PATH] = { L'\0' };
  WCHAR tchExeFile[MAX_PATH] = { L'\0' };

  if (StrIsNotEmpty(Settings2.FileBrowserPath)) {
    ExtractFirstArgument(Settings2.FileBrowserPath, tchExeFile, tchParam, COUNTOF(tchExeFile));
    ExpandEnvironmentStringsEx(tchExeFile, COUNTOF(tchExeFile));
  }
  if (StrStrI(tchExeFile, L"explorer.exe") && StrIsEmpty(tchParam)) {
    SendWMCommand(hwnd, IDM_FILE_EXPLORE_DIR);
    return;
  }
  if (StrIsEmpty(tchExeFile)) {
    StringCchCopy(tchExeFile, COUNTOF(tchExeFile), Constants.FileBrowserMiniPath);
  }
  if (PathIsRelative(tchExeFile)) {
    PathGetAppDirectory(tchTemp, COUNTOF(tchTemp));
    PathAppend(tchTemp, tchExeFile);
    if (PathIsExistingFile(tchTemp)) {
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
//  DialogGrepWin() - Prerequisites
//
//

typedef struct _grepwin_ini
{
  const WCHAR* const key;
  const WCHAR* const val;
} 
grepWin_t;

static grepWin_t grepWinIniSettings[13] = 
{
  { L"onlyone",           L"1" },
  { L"AllSize",           L"1" },
  { L"Size",              L"2000" },
  { L"CaseSensitive",     L"0" },
  { L"CreateBackup",      L"1" },
  { L"DateLimit",         L"0" },
  { L"IncludeBinary",     L"0" },
  { L"IncludeHidden",     L"1" },
  { L"IncludeSubfolders", L"1" },
  { L"IncludeSystem",     L"1" },
  { L"UseFileMatchRegex", L"0" },
  { L"UseRegex",          L"0" },
  { L"UTF8",              L"1" }
};

//=============================================================================
//
//  DialogGrepWin()
//
//
void DialogGrepWin(HWND hwnd, LPCWSTR searchPattern)
{
  WCHAR tchTemp[MAX_PATH] = { L'\0' };
  WCHAR tchNotepad3Path[MAX_PATH] = { L'\0' };
  WCHAR tchExeFile[MAX_PATH] = { L'\0' };
  WCHAR tchOptions[MAX_PATH] = { L'\0' };

  GetModuleFileName(NULL, tchNotepad3Path, COUNTOF(tchNotepad3Path));
  PathCanonicalizeEx(tchNotepad3Path, COUNTOF(tchNotepad3Path));

  // find grepWin executable (side-by-side .ini file)
  if (StrIsNotEmpty(Settings2.GrepWinPath)) {
    ExtractFirstArgument(Settings2.GrepWinPath, tchExeFile, tchOptions, COUNTOF(tchExeFile));
    ExpandEnvironmentStringsEx(tchExeFile, COUNTOF(tchExeFile));
  }
  if (StrIsEmpty(tchExeFile)) {
    StringCchCopy(tchExeFile, COUNTOF(tchExeFile), Constants.FileSearchGrepWin);
  }
  if (PathIsRelative(tchExeFile)) {
    StringCchCopy(tchTemp, COUNTOF(tchTemp), tchNotepad3Path);
    PathCchRemoveFileSpec(tchTemp, COUNTOF(tchTemp));
    PathAppend(tchTemp, tchExeFile);
    if (PathIsExistingFile(tchTemp)) {
      StringCchCopy(tchExeFile, COUNTOF(tchExeFile), tchTemp);
    }
  }

  // working (grepWinNP3.ini) directory
  WCHAR tchGrepWinDir[MAX_PATH] = { L'\0' };
  WCHAR tchIniFilePath[MAX_PATH] = { L'\0' };

  if (PathIsExistingFile(tchExeFile))
  {
    StringCchCopy(tchGrepWinDir, COUNTOF(tchGrepWinDir), tchExeFile);
    PathCchRemoveFileSpec(tchGrepWinDir, COUNTOF(tchGrepWinDir));
    // relative Notepad3 path (for grepWin's EditorCmd)
    if (PathRelativePathTo(tchTemp, tchGrepWinDir, FILE_ATTRIBUTE_DIRECTORY, tchNotepad3Path, FILE_ATTRIBUTE_NORMAL)) {
      StringCchCopy(tchNotepad3Path, COUNTOF(tchNotepad3Path), tchTemp);
    }

    // grepWin INI-File
    const WCHAR* const gwIniFileName = L"grepWinNP3.ini";
    StringCchCopy(tchIniFilePath, COUNTOF(tchIniFilePath), StrIsNotEmpty(Globals.IniFile) ? Globals.IniFile : Globals.IniFileDefault);

    PathRemoveFileSpec(tchIniFilePath);
    PathAppend(tchIniFilePath, gwIniFileName);
    if (PathIsRelative(tchIniFilePath)) {
      StringCchCopy(tchIniFilePath, COUNTOF(tchIniFilePath), tchGrepWinDir);
      PathAppend(tchIniFilePath, gwIniFileName);
    }

    if (CreateIniFile(tchIniFilePath, NULL) && LoadIniFileCache(tchIniFilePath))
    {
      // preserve [global] user settings from last call
      const WCHAR* const globalSection = L"global";

      // get grepWin language
      int lngIdx = -1;
      for (int i = 0; i < grepWinLang_CountOf(); ++i) {
        if (grepWinLangResName[i].lngid == Globals.iPrefLANGID) {
          lngIdx = i;
          break;
        }
      }

      WCHAR value[HUGE_BUFFER];
      for (int i = 0; i < COUNTOF(grepWinIniSettings); ++i) {
        IniSectionGetString(globalSection, grepWinIniSettings[i].key, grepWinIniSettings[i].val, value, COUNTOF(value));
        IniSectionSetString(globalSection, grepWinIniSettings[i].key, value);
      }

      if (lngIdx >= 0) {
        IniSectionGetString(globalSection, L"languagefile", grepWinLangResName[lngIdx].filename, tchTemp, COUNTOF(tchTemp));
        IniSectionSetString(globalSection, L"languagefile", tchTemp);
      } else {
        IniSectionGetString(globalSection, L"languagefile", L"", tchTemp, COUNTOF(tchTemp));
        if (StrIsEmpty(tchTemp)) {
          IniSectionDelete(globalSection, L"languagefile", false);
        }
      }

      StringCchPrintf(tchTemp, COUNTOF(tchTemp), L"%s /%%mode%% \"%%pattern%%\" /g %%line%% - %%path%%", tchNotepad3Path);
      IniSectionSetString(globalSection, L"editorcmd", tchTemp);

      // [settings]
      const WCHAR *const settingsSection = L"settings";

      bool const bEscClose = IniSectionGetBool(settingsSection, L"escclose", (Settings.EscFunction == 2));
      IniSectionSetBool(settingsSection, L"escclose", bEscClose);
      bool const bBackupInFolder = IniSectionGetBool(settingsSection, L"backupinfolder", true);
      IniSectionSetBool(settingsSection, L"backupinfolder", bBackupInFolder);

      // [export]
      const WCHAR *const exportSection = L"export";
      bool const bExpPaths = IniSectionGetBool(exportSection, L"paths", true);
      IniSectionSetBool(exportSection, L"paths", bExpPaths);
      bool const bExpLnNums = IniSectionGetBool(exportSection, L"linenumbers", true);
      IniSectionSetBool(exportSection, L"linenumbers", bExpLnNums);
      bool const bExpContent = IniSectionGetBool(exportSection, L"linecontent", true);
      IniSectionSetBool(exportSection, L"linecontent", bExpContent);


      // search directory
      WCHAR tchSearchDir[MAX_PATH] = { L'\0' };
      if (StrIsNotEmpty(Globals.CurrentFile)) {
        StringCchCopy(tchSearchDir, COUNTOF(tchSearchDir), Globals.CurrentFile);
        PathCchRemoveFileSpec(tchSearchDir, COUNTOF(tchSearchDir));
      }
      else {
        StringCchCopy(tchSearchDir, COUNTOF(tchSearchDir), Globals.WorkingDirectory);
      }
      IniSectionSetString(globalSection, L"searchpath", tchSearchDir);

      // search pattern
      IniSectionSetString(globalSection, L"searchfor", searchPattern);

      SaveIniFileCache(tchIniFilePath);
      ResetIniFileCache();
    }
  }

  // grepWin arguments
  WCHAR tchParams[2*MAX_PATH] = { L'\0' };
  
  if (PathIsExistingFile(tchIniFilePath)) {
    // relative grepWinNP3.ini path (for shorter cmdline)
    if (PathRelativePathTo(tchTemp, tchGrepWinDir, FILE_ATTRIBUTE_DIRECTORY, tchIniFilePath, FILE_ATTRIBUTE_NORMAL)) {
      StringCchCopy(tchIniFilePath, COUNTOF(tchIniFilePath), tchTemp);
    }
    StringCchPrintf(tchParams, COUNTOF(tchParams), L"/portable /content %s /inipath:\"%s\"", tchOptions, tchIniFilePath);
  }
  else {
    StringCchPrintf(tchParams, COUNTOF(tchParams), L"/portable /content %s", tchOptions);
  }
  //if (StrIsNotEmpty(searchPattern)) {
  //  SetClipboardTextW(hwnd, searchPattern, StringCchLen(searchPattern, 0));
  //}

  SHELLEXECUTEINFO sei;
  ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOZONECHECKS;
  sei.hwnd = hwnd;
  sei.lpVerb = NULL;
  sei.lpFile = tchExeFile;
  sei.lpParameters = tchParams;
  sei.lpDirectory = tchGrepWinDir;
  sei.nShow = SW_SHOWNORMAL;
  ShellExecuteEx(&sei);

  if ((INT_PTR)sei.hInstApp < 32) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_GREPWIN);
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
    PathGetAppDirectory(tchExePath, COUNTOF(tchExePath));
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
  if (bFreezeAppTitle) {
    return false;
  }
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
  const DWORD exStyle = GetWindowExStyle(hwnd);
  if (bTransparentMode) {
    SetWindowExStyle(hwnd, exStyle | WS_EX_LAYERED);
    BYTE const bAlpha = (BYTE)MulDiv(iOpacityLevel, 255, 100);
    SetLayeredWindowAttributes(hwnd, 0, bAlpha, LWA_ALPHA);
  }
  else {
    SetWindowExStyle(hwnd, exStyle & ~WS_EX_LAYERED);
  }
}


//=============================================================================
//
//  SetWindowLayoutRTL()
//
void SetWindowLayoutRTL(HWND hwnd, bool bRTL)
{
  DWORD const exStyle = GetWindowExStyle(hwnd);
  if (bRTL) {
    SetWindowExStyle(hwnd, exStyle | WS_EX_LAYOUTRTL);
  }
  else {
    SetWindowExStyle(hwnd, exStyle & ~WS_EX_LAYOUTRTL);
  }
}


//=============================================================================
//
//  SetWindowReadingRTL()
//
void SetWindowReadingRTL(HWND hwnd, bool bRTL)
{
  DWORD const exStyle = GetWindowExStyle(hwnd);
  if (bRTL) {
    SetWindowExStyle(hwnd, exStyle | WS_EX_RTLREADING);
  }
  else {
    SetWindowExStyle(hwnd, exStyle & ~WS_EX_RTLREADING);
  }
}


//=============================================================================
//
//  GetCenterOfDlgInParent()
//
POINT GetCenterOfDlgInParent(const RECT* rcDlg, const RECT* rcParent)
{
  HMONITOR const hMonitor = MonitorFromRect(rcParent, MONITOR_DEFAULTTONEAREST);
  MONITORINFO mi;  mi.cbSize = sizeof(MONITORINFO);  GetMonitorInfo(hMonitor, &mi);

  int const xMin = mi.rcWork.left;
  int const xMax = (mi.rcWork.right) - (rcDlg->right - rcDlg->left);
  int const yMin = mi.rcWork.top;
  int const yMax = (mi.rcWork.bottom) - (rcDlg->bottom - rcDlg->top);

  int const x = rcParent->left + max_i(20, ((rcParent->right - rcParent->left) - (rcDlg->right - rcDlg->left)) / 2);
  int const y = rcParent->top + max_i(20, ((rcParent->bottom - rcParent->top) - (rcDlg->bottom - rcDlg->top)) / 2);

  POINT ptRet;
  ptRet.x = clampi(x, xMin, xMax);
  ptRet.y = clampi(y, yMin, yMax);
  return ptRet;
}


//=============================================================================
//
//  GetParentOrDesktop()
//
HWND GetParentOrDesktop(HWND hDlg)
{
  HWND const hParent = GetParent(hDlg);
  return hParent ? hParent : GetDesktopWindow();
}


//=============================================================================
//
//  CenterDlgInParent()
//
void CenterDlgInParent(HWND hDlg, HWND hDlgParent)
{
  if (!hDlg) { return; }
  
  HWND const hParentWnd = hDlgParent ? hDlgParent : GetParentOrDesktop(hDlg);

  RECT rcDlg = {0};
  GetWindowRect(hDlg, &rcDlg);
  
  WININFO const winInfo = GetMyWindowPlacement(hParentWnd, NULL);
  RECT rcParent = {0};
  rcParent.left   = winInfo.x;
  rcParent.top    = winInfo.y;
  rcParent.right  = winInfo.x + winInfo.cx;
  rcParent.bottom = winInfo.y + winInfo.cy;

  POINT const ptTopLeft = GetCenterOfDlgInParent(&rcDlg, &rcParent);
  SetWindowPos(hDlg, NULL, ptTopLeft.x, ptTopLeft.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  //~SnapToDefaultButton(hDlg);

  //~DPI_T const dpi = Scintilla_GetWindowDPI(hDlg);
  //~PostMessage(hDlg, WM_DPICHANGED, MAKEWPARAM(dpi.x, dpi.y), 0);
}


//=============================================================================
//
//  GetDlgPos()
//
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg)
{
  if (!hDlg) { return; }

  DPI_T const dpi = Scintilla_GetWindowDPI(hDlg); 

  RECT rcDlg;
  GetWindowRect(hDlg, &rcDlg);

  HWND const hParent = GetParent(hDlg);
  RECT rcParent;
  GetWindowRect(hParent, &rcParent);

  // return positions relative to parent window (normalized DPI)
  if (xDlg)
  {
      *xDlg = MulDiv((rcDlg.left - rcParent.left), USER_DEFAULT_SCREEN_DPI, (dpi.x ? dpi.x : USER_DEFAULT_SCREEN_DPI));
  }
  if (yDlg)
  {
      *yDlg = MulDiv((rcDlg.top - rcParent.top), USER_DEFAULT_SCREEN_DPI, (dpi.y ? dpi.y : USER_DEFAULT_SCREEN_DPI));
  }
}


//=============================================================================
//
//  SetDlgPos()
//
void SetDlgPos(HWND hDlg, int xDlg, int yDlg)
{
  if (!hDlg) { return; }

  DPI_T const dpi = Scintilla_GetWindowDPI(hDlg); 

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

  // desired positions relative to parent window (normalized DPI)
  int const x = rcParent.left + MulDiv(xDlg, dpi.x, USER_DEFAULT_SCREEN_DPI);
  int const y = rcParent.top + MulDiv(yDlg, dpi.y, USER_DEFAULT_SCREEN_DPI);

  SetWindowPos(hDlg, NULL, clampi(x, xMin, xMax), clampi(y, yMin, yMax), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}


//=============================================================================
//
// Resize Dialog Helpers()
//
#define RESIZEDLG_PROP_KEY	L"ResizeDlg"
#define MAX_RESIZEDLG_ATTR_COUNT	2
// temporary fix for moving dialog to monitor with different DPI
// TODO: all dimensions no longer valid after window DPI changed.
#define NP3_ENABLE_RESIZEDLG_TEMP_FIX	1

typedef struct _resizeDlg {
  int direction;
  DPI_T dpi;
  int cxClient;
  int cyClient;
  int mmiPtMinX;
  int mmiPtMinY;
  int mmiPtMaxX;	// only Y direction
  int mmiPtMaxY;	// only X direction
  int attrs[MAX_RESIZEDLG_ATTR_COUNT];
} RESIZEDLG, * PRESIZEDLG;

typedef const RESIZEDLG* LPCRESIZEDLG;

void ResizeDlg_InitEx(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, RSZ_DLG_DIR iDirection)
{
  RESIZEDLG* const pm = (RESIZEDLG*)AllocMem(sizeof(RESIZEDLG), HEAP_ZERO_MEMORY);
  pm->direction = iDirection;
  pm->dpi = Scintilla_GetWindowDPI(hwnd);

  RECT rc;
  GetClientRect(hwnd, &rc);
  pm->cxClient = rc.right - rc.left;
  pm->cyClient = rc.bottom - rc.top;

 	const DWORD style = (pm->direction < 0) ? (GetWindowStyle(hwnd) & ~WS_THICKFRAME) : (GetWindowStyle(hwnd) | WS_THICKFRAME);
  
  WRCT_T wrc;
  wrc.left = rc.left; wrc.top = rc.top; wrc.right = rc.right;	wrc.bottom = rc.bottom;
	Scintilla_AdjustWindowRectForDpi(&wrc, style, 0, pm->dpi);
  rc.left = wrc.left; rc.top = wrc.top; rc.right = wrc.right;	rc.bottom = wrc.bottom;

  pm->mmiPtMinX = rc.right - rc.left;
  pm->mmiPtMinY = rc.bottom - rc.top;
  
  // only one direction
  switch (iDirection) {
  case RSZ_ONLY_X:
    pm->mmiPtMaxY = pm->mmiPtMinY;
    break;
  case RSZ_ONLY_Y:
    pm->mmiPtMaxX = pm->mmiPtMinX;
    break;
  }

  cxFrame = max_i(cxFrame, pm->mmiPtMinX);
  cyFrame = max_i(cyFrame, pm->mmiPtMinY);

  SetProp(hwnd, RESIZEDLG_PROP_KEY, (HANDLE)pm);

  SetWindowPos(hwnd, NULL, rc.left, rc.top, cxFrame, cyFrame, SWP_NOZORDER);

  SetWindowStyle(hwnd, style);
  SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

  WCHAR wch[MAX_PATH];
  GetMenuString(GetSystemMenu(GetParent(hwnd), FALSE), SC_SIZE, wch, COUNTOF(wch), MF_BYCOMMAND);
  InsertMenu(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_STRING | MF_ENABLED, SC_SIZE, wch);
  InsertMenu(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);

  if (pm->direction >= 0) {
    HWND const hwndCtl = GetDlgItem(hwnd, nIdGrip);
    if (hwndCtl) {
      SetWindowStyle(hwndCtl, GetWindowStyle(hwndCtl) | SBS_SIZEGRIP | WS_CLIPSIBLINGS);
      int const cGrip = Scintilla_GetSystemMetricsForDpi(SM_CXHTHUMB, pm->dpi);
      SetWindowPos(hwndCtl, NULL, pm->cxClient - cGrip, pm->cyClient - cGrip, cGrip, cGrip, SWP_NOZORDER);
    }
  }
}


void ResizeDlg_Destroy(HWND hwnd, int* cxFrame, int* cyFrame)
{
  PRESIZEDLG const pm = (PRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);

  RECT rc;
  GetWindowRect(hwnd, &rc);
  if (cxFrame) {
    *cxFrame = (rc.right - rc.left);
  }
  if (cyFrame) {
    *cyFrame = (rc.bottom - rc.top);
  }
  RemoveProp(hwnd, RESIZEDLG_PROP_KEY);
  FreeMem(pm);
}

void ResizeDlg_Size(HWND hwnd, LPARAM lParam, int* cx, int* cy)
{
  PRESIZEDLG pm = (PRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
  const int cxClient = LOWORD(lParam);
  const int cyClient = HIWORD(lParam);
#if NP3_ENABLE_RESIZEDLG_TEMP_FIX
  const DPI_T dpi = Scintilla_GetWindowDPI(hwnd);
  const DPI_T old = pm->dpi;
  if (cx) {
    *cx = cxClient - MulDiv(pm->cxClient, dpi.x, old.x);
  }
  if (cy) {
    *cy = cyClient - MulDiv(pm->cyClient, dpi.y, old.y);
  }
  // store in original DPI.
  pm->cxClient = MulDiv(cxClient, old.x, dpi.x);
  pm->cyClient = MulDiv(cyClient, old.y, dpi.y);
#else
  if (cx) {
    *cx = cxClient - pm->cxClient;
  }
  if (cy) {
    *cy = cyClient - pm->cyClient;
  }
  pm->cxClient = cxClient;
  pm->cyClient = cyClient;
#endif
}

void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam)
{
  LPCRESIZEDLG pm = (LPCRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
  LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
#if NP3_ENABLE_RESIZEDLG_TEMP_FIX
  DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);
  DPI_T const old = pm->dpi;

  lpmmi->ptMinTrackSize.x = MulDiv(pm->mmiPtMinX, dpi.x, old.x);
  lpmmi->ptMinTrackSize.y = MulDiv(pm->mmiPtMinY, dpi.y, old.y);

  // only one direction
  switch (pm->direction) {
  case RSZ_ONLY_X:
    lpmmi->ptMaxTrackSize.y = MulDiv(pm->mmiPtMaxY, dpi.x, old.x);
    break;

  case RSZ_ONLY_Y:
    lpmmi->ptMaxTrackSize.x = MulDiv(pm->mmiPtMaxX, dpi.y, old.y);
    break;
  }
#else
  lpmmi->ptMinTrackSize.x = pm->mmiPtMinX;
  lpmmi->ptMinTrackSize.y = pm->mmiPtMinY;

  // only one direction
  switch (pm->direction) {
  case RSZ_ONLY_X:
    lpmmi->ptMaxTrackSize.y = pm->mmiPtMaxY;
    break;

  case RSZ_ONLY_Y:
    lpmmi->ptMaxTrackSize.x = pm->mmiPtMaxX;
    break;
  }
#endif
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

void ResizeDlg_InitY2Ex(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int iDirection, int nCtlId1, int nCtlId2) {
  const int hMin1 = GetDlgCtrlHeight(hwnd, nCtlId1);
  const int hMin2 = GetDlgCtrlHeight(hwnd, nCtlId2);
  ResizeDlg_InitEx(hwnd, cxFrame, cyFrame, nIdGrip, iDirection);
  PRESIZEDLG pm = (PRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
  pm->attrs[0] = hMin1;
  pm->attrs[1] = hMin2;
}

int ResizeDlg_CalcDeltaY2(HWND hwnd, int dy, int cy, int nCtlId1, int nCtlId2) {
  if (dy == 0) {
    return 0;
  }
  if (dy > 0) {
    return MulDiv(dy, cy, 100);
  }
  const LPCRESIZEDLG pm = (LPCRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
#if NP3_ENABLE_RESIZEDLG_TEMP_FIX
  DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);
  int const hMinX = MulDiv(pm->attrs[0], dpi.x, pm->dpi.x);
  int const hMinY = MulDiv(pm->attrs[1], dpi.y, pm->dpi.y);
#else
  int const hMinX = pm->attrs[0];
  int const hMinY = pm->attrs[1];
#endif
  int const h1 = GetDlgCtrlHeight(hwnd, nCtlId1);
  int const h2 = GetDlgCtrlHeight(hwnd, nCtlId2);
  // cy + h1 >= hMin1			cy >= hMin1 - h1
  // dy - cy + h2 >= hMin2	cy <= dy + h2 - hMin2
  int const cyMin = hMinX - h1;
  int const cyMax = dy + h2 - hMinY;
  cy = dy - MulDiv(dy, 100 - cy, 100);
  cy = clampi(cy, cyMin, cyMax);
  return cy;
}


HDWP DeferCtlPos(HDWP hdwp, HWND hwndDlg, int nCtlId, int dx, int dy, UINT uFlags) {
  HWND const hwndCtl = GetDlgItem(hwndDlg, nCtlId);
  RECT rc;
  GetWindowRect(hwndCtl, &rc);
  MapWindowPoints(NULL, hwndDlg, (LPPOINT)& rc, 2);
  if (uFlags & SWP_NOSIZE) {
    return DeferWindowPos(hdwp, hwndCtl, NULL, rc.left + dx, rc.top + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }
  return DeferWindowPos(hdwp, hwndCtl, NULL, 0, 0, rc.right - rc.left + dx, rc.bottom - rc.top + dy, SWP_NOZORDER | SWP_NOMOVE);
}


void ResizeDlgCtl(HWND hwndDlg, int nCtlId, int dx, int dy) {
  HWND const hwndCtl = GetDlgItem(hwndDlg, nCtlId);
  RECT rc;
  GetWindowRect(hwndCtl, &rc);
  MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rc, 2);
  SetWindowPos(hwndCtl, NULL, 0, 0, rc.right - rc.left + dx, rc.bottom - rc.top + dy, SWP_NOZORDER | SWP_NOMOVE);
  InvalidateRect(hwndCtl, NULL, TRUE);
}


//=============================================================================
//
//  SetBitmapControl()
//
void SetBitmapControl(HWND hwnd, int nCtrlId, HBITMAP hBmp)
{
  HBITMAP hBmpOld = (HBITMAP)SendDlgItemMessage(hwnd, nCtrlId, STM_GETIMAGE, IMAGE_BITMAP, 0);
  if (hBmpOld) {
    DeleteObject(hBmpOld);
  }
  SendDlgItemMessage(hwnd, nCtrlId, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
}


//=============================================================================
//
//  SetBitmapControlResample()
//  if width|height <= 0 : scale bitmap to current dpi
//
void SetBitmapControlResample(HWND hwnd, int nCtrlId, HBITMAP hBmp, int width, int height)
{
  if ((width ==  0) || (height == 0))
  {
    width  = GetDlgCtrlWidth(hwnd, nCtrlId);
    height = GetDlgCtrlHeight(hwnd, nCtrlId);
  }
  hBmp = ResampleImageBitmap(hwnd, hBmp, width, height);

  SetBitmapControl(hwnd, nCtrlId, hBmp);
}


//=============================================================================
//
//  MakeBitmapButton()
//  if width|height <= 0 : scale bitmap to current dpi
//
void MakeBitmapButton(HWND hwnd, int nCtrlId, WORD uBmpId, int width, int height)
{
  HWND const hwndCtrl = GetDlgItem(hwnd, nCtrlId);
  if ((width == 0) || (height == 0)) {
    width  = GetDlgCtrlWidth(hwnd, nCtrlId);
    height = GetDlgCtrlHeight(hwnd, nCtrlId);
  }
  HBITMAP hBmp = LoadImage(Globals.hInstance, MAKEINTRESOURCE(uBmpId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  hBmp         = ResampleImageBitmap(hwnd, hBmp, width, height);

  BITMAP bmp;
  GetObject(hBmp, sizeof(BITMAP), &bmp);
  BUTTON_IMAGELIST bi;
  bi.himl = ImageList_Create(bmp.bmWidth, bmp.bmHeight, ILC_COLOR32 | ILC_MASK, 1, 0);
  ImageList_AddMasked(bi.himl, hBmp, CLR_DEFAULT);
  
  DeleteObject(hBmp);
  
  SetRect(&bi.margin, 0, 0, 0, 0);
  bi.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
  SendMessage(hwndCtrl, BCM_SETIMAGELIST, 0, (LPARAM)&bi);
}


//=============================================================================
//
//  MakeColorPickButton()
//
void MakeColorPickButton(HWND hwnd, int nCtrlId, HINSTANCE hInstance, COLORREF crColor)
{
  HWND const hwndCtl = GetDlgItem(hwnd, nCtrlId);
  HIMAGELIST himlOld = NULL;
  COLORMAP colormap[2];

  BUTTON_IMAGELIST bi;
  if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)&bi)) {
    himlOld = bi.himl;
  }
  if (IsWindowEnabled(hwndCtl) && (crColor != COLORREF_MAX)) {
    colormap[0].from = RGB(0x00, 0x00, 0x00);
    colormap[0].to = GetSysColor(COLOR_3DSHADOW);
  }
  else {
    colormap[0].from = RGB(0x00, 0x00, 0x00);
    colormap[0].to = RGB(0xFF, 0xFF, 0xFF);
  }

  if (IsWindowEnabled(hwndCtl) && (crColor != COLORREF_MAX)) {

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
  InvalidateRect(hwndCtl, NULL, TRUE);

  if (himlOld) {
    ImageList_Destroy(himlOld);
  }
}


//=============================================================================
//
//  DeleteBitmapButton()
//
void DeleteBitmapButton(HWND hwnd, int nCtrlId)
{
  HWND const hwndCtl = GetDlgItem(hwnd, nCtrlId);
  BUTTON_IMAGELIST bi;
  if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)& bi)) {
    ImageList_Destroy(bi.himl);
  }
}


//=============================================================================
//
//  StatusSetText()
//
void StatusSetText(HWND hwnd, UINT nPart, LPCWSTR lpszText)
{
  if (lpszText) {
    UINT const uFlags = (nPart == (UINT)STATUS_HELP) ? nPart | SBT_NOBORDERS : nPart;
    StatusSetSimple(hwnd, (nPart == (UINT)STATUS_HELP));
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
  StatusSetSimple(hwnd, (nPart == (UINT)STATUS_HELP));

  if (!uID) {
    SendMessage(hwnd, SB_SETTEXT, uFlags, 0);
    return true;
  }
  if (!GetLngString(uID, szText, 256)) { return false; }

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
  int const cnt = min_i(50, (int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0));

  for (int i = 0; i < cnt; i++) {
    TBBUTTON tbb;
    SendMessage(hwnd, TB_GETBUTTON, (WPARAM)i, (LPARAM)&tbb);
    StringCchPrintf(tchItem, COUNTOF(tchItem), L"%i ",
      (tbb.idCommand == 0) ? 0 : tbb.idCommand - cmdBase + 1);
    StringCchCat(tchButtons, COUNTOF(tchButtons), tchItem);
  }
  TrimSpcW(tchButtons);
  StringCchCopyN(lpszButtons, cchButtons, tchButtons, COUNTOF(tchButtons));
  return cnt;
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
    //if (swscanf_s(p, L"%i", &iCmd) == 1) {
    if (StrToIntEx(p, STIF_DEFAULT, &iCmd)) {
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

//=============================================================================
//
//  GetCurrentPPI()
//  (font size) points per inch
//
DPI_T GetCurrentPPI(HWND hwnd) {
  HDC const hDC = GetDC(hwnd);
  DPI_T ppi;
  ppi.x = max_u(GetDeviceCaps(hDC, LOGPIXELSX), USER_DEFAULT_SCREEN_DPI);
  ppi.y = max_u(GetDeviceCaps(hDC, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI);
  ReleaseDC(hwnd, hDC);
  return ppi;
}


/*

Themed Dialogs
Modify dialog templates to use current theme font
Based on code of MFC helper class CDialogTemplate

*/
static inline bool IsChineseTraditionalSubLang(LANGID subLang)
{
  return subLang == SUBLANG_CHINESE_TRADITIONAL || subLang == SUBLANG_CHINESE_HONGKONG || subLang == SUBLANG_CHINESE_MACAU;
}

bool GetLocaleDefaultUIFont(LANGID lang, LPWSTR lpFaceName, WORD* wSize)
{
  LPCWSTR font;
  LANGID const subLang = SUBLANGID(lang);
  switch (PRIMARYLANGID(lang)) {
    default:
    case LANG_ENGLISH:
      font   = L"Segoe UI";
      *wSize = 9;
      break;
    case LANG_CHINESE:
      font   = IsChineseTraditionalSubLang(subLang) ? L"Microsoft JhengHei UI" : L"Microsoft YaHei UI";
      *wSize = 9;
      break;
    case LANG_JAPANESE:
      font   = L"Yu Gothic UI";
      *wSize = 9;
      break;
    case LANG_KOREAN:
      font   = L"Malgun Gothic";
      *wSize = 9;
      break;
  }
  bool const isAvail = IsFontAvailable(font);
  if (isAvail) {
    StringCchCopy(lpFaceName, LF_FACESIZE, font);
  }
  return isAvail;
}


bool GetThemedDialogFont(LPWSTR lpFaceName, WORD* wSize)
{
  bool bSucceed = GetLocaleDefaultUIFont(Globals.iPrefLANGID, lpFaceName, wSize);

  if (!bSucceed)
  {
    if (IsAppThemed()) {
      unsigned const iLogPixelsY = GetCurrentPPI(NULL).y - DIALOG_FONT_SIZE_INCR;

      HTHEME hTheme = OpenThemeData(NULL, L"WINDOWSTYLE;WINDOW");
      if (hTheme) {
        LOGFONT lf;
        if (S_OK == GetThemeSysFont(hTheme, TMT_MSGBOXFONT, &lf)) {
          if (lf.lfHeight < 0) {
            lf.lfHeight = -lf.lfHeight;
          }
          *wSize = (WORD)MulDiv(lf.lfHeight, 72, iLogPixelsY);
          if (*wSize < 9) { *wSize = 9; }
          StringCchCopy(lpFaceName, LF_FACESIZE, lf.lfFaceName);
          bSucceed = true;
        }
        CloseThemeData(hTheme);
      }
    }

    if (!bSucceed) {
      unsigned const iLogPixelsY = GetCurrentPPI(NULL).y - DIALOG_FONT_SIZE_INCR;

      NONCLIENTMETRICS ncm;
      ZeroMemory(&ncm, sizeof(ncm));
      ncm.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(ncm.iPaddedBorderWidth);
      if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0)) {
        if (ncm.lfMessageFont.lfHeight < 0) {
          ncm.lfMessageFont.lfHeight = -ncm.lfMessageFont.lfHeight;
        }
        *wSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight, 72, iLogPixelsY);
        if (*wSize < 9) { *wSize = 9; }
        StringCchCopy(lpFaceName, LF_FACESIZE, ncm.lfMessageFont.lfFaceName);
        bSucceed = true;
      }
    }
  }

  return bSucceed;
}


static inline bool DialogTemplate_IsDialogEx(const DLGTEMPLATE* pTemplate) {
  return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}

static inline bool DialogTemplate_HasFont(const DLGTEMPLATE* pTemplate)
{
  return (DS_SETFONT &
    (DialogTemplate_IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->style : pTemplate->style));
}

static inline size_t DialogTemplate_FontAttrSize(bool bDialogEx)
{
  return (sizeof(WORD) * (bDialogEx ? 3 : 1));
}


static inline BYTE* DialogTemplate_GetFontSizeField(const DLGTEMPLATE* pTemplate)
{

  bool bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
  WORD* pw;

  if (bDialogEx)
    pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
  else
    pw = (WORD*)(pTemplate + 1);

  if (*pw == WORD_MAX)
    pw += 2;
  else
    while (*pw++){}

  if (*pw == WORD_MAX)
    pw += 2;
  else
    while (*pw++){}

  while (*pw++){}

  return (BYTE*)pw;
}


DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lpDialogTemplateID, HINSTANCE hInstance)
{
  HRSRC const hRsrc = FindResource(hInstance, lpDialogTemplateID, RT_DIALOG);
  if (!hRsrc) { return NULL; }

  HGLOBAL const hRsrcMem = LoadResource(hInstance, hRsrc);
  DLGTEMPLATE* const pRsrcMem = hRsrcMem ? (DLGTEMPLATE*) LockResource(hRsrcMem) : NULL;
  if (!pRsrcMem) { return NULL; }

  size_t const  dwTemplateSize = (size_t)SizeofResource(hInstance, hRsrc);
  DLGTEMPLATE* const pTemplate = dwTemplateSize ? (DLGTEMPLATE*)AllocMem(dwTemplateSize + LF_FACESIZE * 2, HEAP_ZERO_MEMORY) : NULL;

  if (!pTemplate) {
    UnlockResource(hRsrcMem);
    FreeResource(hRsrcMem);
    return NULL;
  }

  CopyMemory((BYTE*)pTemplate, pRsrcMem, dwTemplateSize);
  UnlockResource(hRsrcMem);
  FreeResource(hRsrcMem);

  WCHAR wchFaceName[LF_FACESIZE] = {L'\0'};
  WORD  wFontSize = 0;
  if (!GetThemedDialogFont(wchFaceName, &wFontSize)) {
    return (pTemplate);
  }

  bool const bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
  bool const bHasFont = DialogTemplate_HasFont(pTemplate);
  size_t const cbFontAttr = DialogTemplate_FontAttrSize(bDialogEx);

  if (bDialogEx) {
    ((DLGTEMPLATEEX*)pTemplate)->style |= DS_SHELLFONT;
  }
  else {
    pTemplate->style |= DS_SHELLFONT;
  }

  size_t const cbNew = cbFontAttr + ((StringCchLenW(wchFaceName, COUNTOF(wchFaceName)) + 1) * sizeof(WCHAR));
  BYTE* const pbNew = (BYTE*)wchFaceName;

  BYTE* pb = DialogTemplate_GetFontSizeField(pTemplate);
  size_t const cbOld = (bHasFont ? cbFontAttr + 2 * (StringCchLen((WCHAR*)(pb + cbFontAttr), 0) + 1) : 0);

  BYTE* const pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~(DWORD_PTR)3);
  BYTE* const pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~(DWORD_PTR)3);

  WORD const nCtrl = (bDialogEx ? ((DLGTEMPLATEEX*)pTemplate)->cDlgItems : pTemplate->cdit);

  if (cbNew != cbOld && nCtrl > 0) {
    MoveMemory(pNewControls, pOldControls, (dwTemplateSize - (pOldControls - (BYTE*)pTemplate)));
  }

  *(WORD*)pb = wFontSize;
  MoveMemory(pb + cbFontAttr, pbNew, (size_t)(cbNew - cbFontAttr));

  return pTemplate;
}


INT_PTR ThemedDialogBoxParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent,
                             DLGPROC lpDialogFunc, LPARAM dwInitParam) 
{
  DLGTEMPLATE* const pDlgTemplate = LoadThemedDialogTemplate(lpTemplate, hInstance);
  INT_PTR const ret = DialogBoxIndirectParam(hInstance, pDlgTemplate, hWndParent, lpDialogFunc, dwInitParam);
  if (pDlgTemplate) {
    FreeMem(pDlgTemplate);
  }
  return ret;
}


HWND CreateThemedDialogParam(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent,
                             DLGPROC lpDialogFunc, LPARAM dwInitParam) 
{
  DLGTEMPLATE* const pDlgTemplate = LoadThemedDialogTemplate(lpTemplate, hInstance);
  HWND const hwnd = CreateDialogIndirectParam(hInstance, pDlgTemplate, hWndParent, lpDialogFunc, dwInitParam);
  if (pDlgTemplate) {
    FreeMem(pDlgTemplate);
  }
  return hwnd;
}


//=============================================================================
//
//  _GetIconInfo()
//
static void _GetIconInfo(HICON hIcon, int* width, int* height, WORD* bitsPerPix)
{
  ICONINFO info = {0};
  if (!GetIconInfo(hIcon, &info)) {
    return;
  }
   if (info.hbmColor) {
    BITMAP bmp = {0};
    if (GetObject(info.hbmColor, sizeof(bmp), &bmp) > 0) {
      if (width) { *width = (int)bmp.bmWidth; }
      if (height) { *height = (int)bmp.bmHeight; }
      if (bitsPerPix) { *bitsPerPix = bmp.bmBitsPixel; }
    }
  }
  else if (info.hbmMask) {
    // Icon has no color plane, image data stored in mask
    BITMAP bmp = {0};
    if (GetObject(info.hbmMask, sizeof(bmp), &bmp) > 0) {
      if (width) { *width = (int)bmp.bmWidth; }
      if (height) { *height = (int)(bmp.bmHeight > 1); }
      if (bitsPerPix) { *bitsPerPix = 1; }
    }
  }
  if (info.hbmColor) {
    DeleteObject(info.hbmColor);
  }
  if (info.hbmMask) {
    DeleteObject(info.hbmMask);
  }
}


//=============================================================================
//
//  ConvertIconToBitmap()
//  cx/cy = 0  =>  use resource width/height
//
HBITMAP ConvertIconToBitmap(const HICON hIcon, int cx, int cy)
{
  int wdc = cx;
  int hdc = cy;
  if ((cx <= 0) || (cy <= 0)) {
    _GetIconInfo(hIcon, &wdc, &hdc, NULL);
    cx = cy = 0;
  }
 
  HDC     const hScreenDC = GetDC(NULL);
  HBITMAP const hbmpTmp   = CreateCompatibleBitmap(hScreenDC, wdc, hdc);
  HDC     const hMemDC    = CreateCompatibleDC(hScreenDC);
  HBITMAP const hOldBmp   = SelectObject(hMemDC, hbmpTmp); // assign
  DrawIconEx(hMemDC, 0, 0, hIcon, wdc, hdc, 0, NULL, DI_NORMAL /*&~DI_DEFAULTSIZE*/);
  SelectObject(hMemDC, hOldBmp); // restore

  UINT    const copyFlags = LR_COPYDELETEORG | LR_COPYRETURNORG | LR_DEFAULTSIZE | LR_CREATEDIBSECTION;
  HBITMAP const hDibBmp   = (HBITMAP)CopyImage((HANDLE)hbmpTmp, IMAGE_BITMAP, cx, cy, copyFlags);

  DeleteDC(hMemDC);
  ReleaseDC(NULL, hScreenDC);
  return hDibBmp;
}

//=============================================================================
//
//  ResampleIconToBitmap()
//
HBITMAP ResampleIconToBitmap(HWND hwnd, const HICON hIcon, const int cx, const int cy)
{
  //~return ConvertIconToBitmap(hwnd, hIcon, cx, cy);
  HBITMAP const hBmp = ConvertIconToBitmap(hIcon, 0, 0);
  return ResampleImageBitmap(hwnd, hBmp, cx, cy);
}

//=============================================================================
//
//  SetUACIcon()
//
void SetUACIcon(HWND hwnd, const HMENU hMenu, const UINT nItem)
{
  static bool bInitialized = false;
  if (bInitialized) { return; }

  DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);

  int const cx = ScaleIntByDPI(GetSystemMetrics(SM_CXSMICON), dpi.x);
  int const cy = ScaleIntByDPI(GetSystemMetrics(SM_CYSMICON), dpi.y);

  if (Globals.hIconMsgShield)
  {
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_BITMAP;
    mii.hbmpItem = ConvertIconToBitmap(Globals.hIconMsgShield, cx, cy);
    SetMenuItemInfo(hMenu, nItem, FALSE, &mii);
  }
  bInitialized = true;
}


//=============================================================================
//
//  UpdateWindowLayoutForDPI()
//
inline WRCT_T _ConvWinRectW(const RECT* pRC)
{
  WRCT_T wrc;
  wrc.left   = pRC->left;
  wrc.top    = pRC->top;
  wrc.right  = pRC->right;
  wrc.bottom = pRC->bottom;
  return wrc;
}

void UpdateWindowLayoutForDPI(HWND hwnd, const RECT* prc, const DPI_T* pdpi)
{
  UINT const uWndFlags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED; //~ SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION
  
  if (prc) {
    SetWindowPos(hwnd, NULL, prc->left, prc->top, (prc->right - prc->left), (prc->bottom - prc->top), uWndFlags);
    return;
  }
 
  DPI_T const dpi = pdpi ? *pdpi : Scintilla_GetWindowDPI(hwnd);

  RECT rc;  GetWindowRect(hwnd, &rc);
  //~MapWindowPoints(NULL, hWnd, (LPPOINT)&rc, 2);
  WRCT_T wrc = _ConvWinRectW(prc);
  Scintilla_AdjustWindowRectForDpi(&wrc, uWndFlags, 0, dpi);
  SetWindowPos(hwnd, NULL, wrc.left, wrc.top, (wrc.right - wrc.left), (wrc.bottom - wrc.top), uWndFlags);
}

//=============================================================================
//
//  ResampleImageBitmap()  (resample_delete_orig)
//  if width|height <= 0 : scale bitmap to current dpi
//
HBITMAP ResampleImageBitmap(HWND hwnd, HBITMAP hbmp, int width, int height)
{
  if (hbmp) {
    BITMAP bmp;
    if (GetObject(hbmp, sizeof(BITMAP), &bmp)) {
      if ((width <= 0) || (height <= 0)) {
        DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);
        width  = ScaleIntByDPI(bmp.bmWidth, dpi.x);
        height = ScaleIntByDPI(bmp.bmHeight, dpi.y);
      }
      if (((LONG)width != bmp.bmWidth) || ((LONG)height != bmp.bmHeight)) {
#if TRUE      
        HDC const hdc   = GetDC(hwnd);
        HBITMAP   hCopy = CreateResampledBitmap(hdc, hbmp, width, height, BMP_RESAMPLE_FILTER);
        ReleaseDC(hwnd, hdc);
#else
        HBITMAP hCopy = CopyImage(hbmp, IMAGE_BITMAP, width, height, LR_CREATEDIBSECTION | LR_COPYRETURNORG | LR_COPYDELETEORG);
#endif
        if (hCopy && (hCopy != hbmp)) {
          DeleteObject(hbmp);
          hbmp = hCopy;
        }
      }
    }
  }
  return hbmp;
}


//=============================================================================
//
//  SendWMSize()
//
LRESULT SendWMSize(HWND hwnd, RECT* rc)
{
  if (rc) {
    return SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc->right, rc->bottom));
  }
  RECT wndrc;
  GetClientRect(hwnd, &wndrc);
  return SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(wndrc.right, wndrc.bottom));
}


#if FALSE
//=============================================================================
//
//  CreateAndSetFontDlgItemDPI()
//
HFONT CreateAndSetFontDlgItemDPI(HWND hdlg, const int idDlgItem, int fontSize, bool bold)
{
  NONCLIENTMETRICSW ncm = {0};
  ncm.cbSize            = sizeof(NONCLIENTMETRICSW);
  if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0)) {
    HDC const hdcSys = GetDC(NULL);
    DPI_T const dpiSys = Scintilla_GetWindowDPI(NULL);
    DPI_T const dpiDlg = Scintilla_GetWindowDPI(hdlg);
    if (fontSize <= 0) {
      fontSize = (ncm.lfMessageFont.lfHeight < 0) ? -ncm.lfMessageFont.lfHeight : ncm.lfMessageFont.lfHeight;
      if (fontSize == 0) {
        fontSize = 9;
      }
    }
    fontSize <<= 10; // precision
    fontSize = MulDiv(fontSize, USER_DEFAULT_SCREEN_DPI, dpiSys.y); // correction
    fontSize = ScaleIntByDPI(fontSize, dpiDlg.y);
    ncm.lfMessageFont.lfHeight = -(MulDiv(fontSize, GetDeviceCaps(hdcSys, LOGPIXELSY), 72) >> 10); 
    ncm.lfMessageFont.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    HFONT const hFont = CreateFontIndirectW(&ncm.lfMessageFont);
    if (idDlgItem > 0) {
      SendDlgItemMessageW(hdlg, idDlgItem, WM_SETFONT, (WPARAM)hFont, true);
    }
    ReleaseDC(hdlg, hdcSys);
    return hFont;
  }
  return NULL;
}
#endif


//=============================================================================

#if FALSE
void Handle_WM_PAINT(HWND hwnd)
{
  static HFONT hVersionFont = NULL;

  PAINTSTRUCT ps;

  // Get a paint DC for current window.
  // Paint DC contains the right scaling to match
  // the monitor DPI where the window is located.
  HDC hdc = BeginPaint(hwnd, &ps);

  RECT rect;
  GetClientRect(hwnd, &rect);

  UINT cx = (rect.right - rect.left);
  UINT cy = (rect.bottom - rect.top);

  // Create a compatible bitmap using paint DC.
  // Compatible bitmap will be properly scaled in size internally and
  // transparently to the app to match current monitor DPI where
  // the window is located.
  HBITMAP memBitmap = CreateCompatibleBitmap(hdc, cx, cy);

  // Create a compatible DC, even without a bitmap selected,
  // compatible DC will inherit the paint DC GDI scaling
  // matching the window monitor DPI.
  HDC memDC = CreateCompatibleDC(hdc);

  // Selecting GDI scaled compatible bitmap in the
  // GDI scaled compatible DC.
  HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

  // Setting some properties in the compatible GDI scaled DC.
  if (hVersionFont) {
    DeleteObject(hVersionFont);
  }
  hVersionFont = GetStockObject(DEFAULT_GUI_FONT);

  SetTextColor(memDC, GetSysColor(COLOR_INFOTEXT));
  SetBkMode(memDC, TRANSPARENT);
  SelectObject(memDC, hVersionFont);

  // Drawing content on the compatible GDI scaled DC.
  // If the monitor DPI was 150% or 200%, text internally will
  // be draw at next integral scaling value, in current example
  // 200%.
  DrawText(memDC, ctx.balloonText, -1, &rect,
           DT_NOCLIP | DT_LEFT | DT_NOPREFIX | DT_WORDBREAK);

  // Copying the content back from compatible DC to paint DC.
  // Since both compatible DC and paint DC are GDI scaled,
  // content is copied without any stretching thus preserving
  // the quality of the rendering.
  BitBlt(hdc, 0, 0, cx, cy, memDC, 0, 0, 0);

  // Cleanup.
  SelectObject(memDC, oldBitmap);
  DeleteObject(memBitmap);
  DeleteDC(memDC);

  // At this time the content is presented to the screen.
  // DWM (Desktop Window Manager) will scale down if required the
  // content to actual monitor DPI.
  // If the monitor DPI is already an integral one, for example 200%,
  // there would be no DWM down scaling.
  // If the monitor DPI is 150%, DWM will scale down rendered content
  // from 200% to 150%.
  // While not a perfect solution, it's better to scale-down content
  // instead of scaling-up since a lot of the details will be preserved
  // during scale-down.
  // The end result is that with GDI Scaling enabled, the content will
  // look less blurry on screen and in case of monitors with DPI setting
  // set to an integral value (200%, 300%) the vector based and text
  // content will be rendered natively at the monitor DPI looking crisp
  // on screen.

  EndPaint(hwnd, &ps);
}
#endif

//=============================================================================

//  End of Dialogs.c
