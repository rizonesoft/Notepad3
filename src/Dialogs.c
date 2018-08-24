/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Dialogs.c                                                                   *
*   Notepad3 dialog boxes implementation                                      *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
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
#include <shellapi.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <commdlg.h>
#include <string.h>

#pragma warning( push )
#pragma warning( disable : 4201) // union/struct w/o name
#include <richedit.h>
#pragma warning( pop ) 

#include "scintilla.h"
#include "notepad3.h"
#include "edit.h"
#include "dlapi.h"
#include "resource.h"
#include "version.h"
#include "helpers.h"
#include "encoding.h"
#include "SciCall.h"

#include "dialogs.h"


extern HWND      g_hwndMain;
extern HINSTANCE g_hInstance;
extern HMODULE   g_hLngResContainer;
extern LANGID    g_iPrefLngLocID;
extern HICON     g_hDlgIcon;

extern WCHAR g_wchWorkingDirectory[];
extern WCHAR g_wchCurFile[];
extern WCHAR g_wchAppUserModelID[];

extern DWORD dwLastIOError;
extern bool bUseDefaultForFileEncoding;
extern bool bSkipUnicodeDetection;
extern bool bSkipANSICodePageDetection;
extern bool bLoadASCIIasUTF8;
extern bool bLoadNFOasOEM;
extern bool bNoEncodingTags;
extern bool bFixLineEndings;
extern bool bAutoStripBlanks;

extern int g_flagNoFileVariables;
extern int g_flagUseSystemMRU;



//=============================================================================
//
//  MsgBoxLng()
//
static HHOOK hhkMsgBox = NULL;

static LRESULT CALLBACK _MsgBoxProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
  HWND  hParentWnd, hChildWnd;    // msgbox is "child"
  RECT  rParent, rChild, rDesktop;

  // notification that a window is about to be activated  
  if (nCode == HCBT_ACTIVATE) {
    // set window handles
    hParentWnd = GetForegroundWindow();
    hChildWnd = (HWND)wParam; // window handle is wParam

    if ((hParentWnd != NULL) && (hChildWnd != NULL) &&
        (GetWindowRect(GetDesktopWindow(), &rDesktop) != 0) &&
        (GetWindowRect(hParentWnd, &rParent) != 0) &&
        (GetWindowRect(hChildWnd, &rChild) != 0)) {
      
      CenterDlgInParent(hChildWnd);
    }
    PostMessage(hChildWnd, WM_SETFOCUS, 0, 0);

    // exit _MsgBoxProc hook
    UnhookWindowsHookEx(hhkMsgBox);
  }
  else // otherwise, continue with any possible chained hooks
  {
    CallNextHookEx(hhkMsgBox, nCode, wParam, lParam);
  }
  return 0;
}
// -----------------------------------------------------------------------------


int MsgBoxLng(int iType, UINT uIdMsg, ...)
{
  WCHAR szText[HUGE_BUFFER] = { L'\0' };
  WCHAR szBuf[HUGE_BUFFER] = { L'\0' };
  WCHAR szTitle[64] = { L'\0' };

  if (!GetLngString(uIdMsg, szBuf, COUNTOF(szBuf)))
    return(0);

  StringCchVPrintfW(szText, COUNTOF(szText), szBuf, (LPVOID)((PUINT_PTR)&uIdMsg + 1));

  if (uIdMsg == IDS_MUI_ERR_LOADFILE || uIdMsg == IDS_MUI_ERR_SAVEFILE ||
    uIdMsg == IDS_MUI_CREATEINI_FAIL || uIdMsg == IDS_MUI_WRITEINI_FAIL ||
    uIdMsg == IDS_MUI_EXPORT_FAIL) {
    LPVOID lpMsgBuf = NULL;
    WCHAR wcht;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dwLastIOError,
      g_iPrefLngLocID,
      (LPTSTR)&lpMsgBuf,
      0,
      NULL);
    if (lpMsgBuf) {
      StrTrim(lpMsgBuf, L" \a\b\f\n\r\t\v");
      StringCchCat(szText, COUNTOF(szText), L"\n");
      StringCchCat(szText, COUNTOF(szText), lpMsgBuf);
      LocalFree(lpMsgBuf);
    }
    wcht = *CharPrev(szText, StrEnd(szText));
    if (IsCharAlphaNumeric(wcht) || wcht == '"' || wcht == '\'')
      StringCchCat(szText, COUNTOF(szText), L".");
  }

  GetLngString(IDS_MUI_APPTITLE, szTitle, COUNTOF(szTitle));

  int iIcon = MB_ICONHAND;
  switch (iType) {
  case MBINFO: iIcon = MB_ICONINFORMATION | MB_OK; break;
  case MBWARN: iIcon = MB_ICONWARNING | MB_OK; break;
  case MBYESNO: iIcon = MB_ICONQUESTION | MB_YESNO; break;
  case MBYESNOCANCEL: iIcon = MB_ICONINFORMATION | MB_YESNOCANCEL; break;
  case MBYESNOWARN: iIcon = MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1; break;
  case MBOKCANCEL: iIcon = MB_ICONEXCLAMATION | MB_OKCANCEL; break;
  case MBRETRYCANCEL: iIcon = MB_ICONQUESTION | MB_RETRYCANCEL; break;
  default: iIcon = MB_ICONSTOP | MB_OK; break;
  }
  iIcon |= (MB_TOPMOST | MB_SETFOREGROUND);

  // center message box to main
  HWND focus = GetFocus();
  HWND hwnd = focus ? focus : g_hwndMain;
  hhkMsgBox = SetWindowsHookEx(WH_CBT, &_MsgBoxProc, 0, GetCurrentThreadId());

  //return  MessageBox(hwnd, szText, szTitle, iIcon);
  //return  MessageBoxEx(hwnd, szText, szTitle, iIcon, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
  return  MessageBoxEx(hwnd, szText, szTitle, iIcon, g_iPrefLngLocID);
}


//=============================================================================
//
//  InfoBoxDlgProc()
//
//
typedef struct _infobox {
  LPWSTR lpstrMessage;
  LPWSTR lpstrSetting;
  bool   bDisableCheckBox;
} INFOBOX, *LPINFOBOX;

INT_PTR CALLBACK InfoBoxDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  LPINFOBOX lpib;

  switch (umsg)
  {
  case WM_INITDIALOG:
    {
      if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }
      lpib = (LPINFOBOX)lParam;
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
      SendDlgItemMessage(hwnd, IDC_INFOBOXICON, STM_SETICON, (WPARAM)LoadIcon(NULL, IDI_EXCLAMATION), 0);
      SetDlgItemText(hwnd, IDC_INFOBOXTEXT, lpib->lpstrMessage);
      if (lpib->bDisableCheckBox)
        DialogEnableWindow(hwnd, IDC_INFOBOXCHECK, false);
      LocalFree(lpib->lpstrMessage);
      CenterDlgInParent(hwnd);
    }
    return true;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
    case IDYES:
    case IDNO:
      lpib = (LPINFOBOX)GetWindowLongPtr(hwnd, DWLP_USER);
      if (IsDlgButtonChecked(hwnd, IDC_INFOBOXCHECK))
        IniSetInt(L"Suppressed Messages", lpib->lpstrSetting, 1);
      EndDialog(hwnd, LOWORD(wParam));
      break;
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  InfoBoxLng()
//
//
extern WCHAR g_wchIniFile[MAX_PATH];

INT_PTR InfoBoxLng(int iType, LPCWSTR lpstrSetting, int uidMessage, ...)
{
  int iMode = IniGetInt(L"Suppressed Messages", lpstrSetting, 0);

  if (StrIsNotEmpty(lpstrSetting) && iMode == 1) {
    return (iType == MBYESNO) ? IDYES : IDOK;
  }
  WCHAR wchFormat[LARGE_BUFFER];
  if (!GetLngString(uidMessage, wchFormat, COUNTOF(wchFormat)))
    return(-1);

  INFOBOX ib;
  ib.lpstrMessage = LocalAlloc(LPTR, HUGE_BUFFER * sizeof(WCHAR));
  StringCchVPrintfW(ib.lpstrMessage, HUGE_BUFFER, wchFormat, (LPVOID)((PUINT_PTR)&uidMessage + 1));
  ib.lpstrSetting = (LPWSTR)lpstrSetting;
  ib.bDisableCheckBox = (StrIsEmpty(g_wchIniFile) || StrIsEmpty(lpstrSetting) || iMode == 2) ? true : false;

  int idDlg;
  switch (iType) {
  case MBYESNO:
    idDlg = IDD_MUI_INFOBOX2;
    break;
  case MBOKCANCEL:
    idDlg = IDD_MUI_INFOBOX3;
    break;
  default:
    idDlg = IDD_MUI_INFOBOX;
    break;
  }

  MessageBeep(MB_ICONEXCLAMATION);

  HWND focus = GetFocus();
  HWND hwnd = focus ? focus : g_hwndMain;

  return ThemedDialogBoxParam(g_hLngResContainer, MAKEINTRESOURCE(idDlg), hwnd, InfoBoxDlgProc, (LPARAM)&ib);
}


//=============================================================================
//
//  DisplayCmdLineHelp()
//
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
  mbp.hInstance = g_hInstance;
  mbp.lpszText = szText;
  mbp.lpszCaption = szTitle;
  mbp.dwStyle = MB_OK | MB_USERICON | MB_SETFOREGROUND;
  mbp.lpszIcon = MAKEINTRESOURCE(IDR_MAINWND48);
  mbp.dwContextHelpId = 0;
  mbp.lpfnMsgBoxCallback = NULL;
  mbp.dwLanguageId = g_iPrefLngLocID;

  hhkMsgBox = SetWindowsHookEx(WH_CBT, &_MsgBoxProc, 0, GetCurrentThreadId());

  MessageBoxIndirect(&mbp);
  //MsgBoxLng(MBINFO, IDS_MUI_CMDLINEHELP);
}


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

  if (g_hInstance == NULL)
    return 0L;

  HRSRC hRsrc = FindResource(g_hInstance, pszResId, pszRsType);

  if (hRsrc == NULL) {
    return 0L;
  }

  HGLOBAL hGlobal = LoadResource(g_hInstance, hRsrc);

  if (hGlobal == NULL) {
    return 0L;
  }

  const BYTE* pData = (const BYTE*)LockResource(hGlobal);

  if (pData == NULL) {
    FreeResource(hGlobal);
    return 0L;
  }

  DWORD dwSize = SizeofResource(g_hInstance, hRsrc);

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
  LONG len = (LONG)strlen(*pstr);

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


static char pAboutResource[8192] = { '\0' };
static char* pAboutInfo;


//=============================================================================
//
//  AboutDlgProc()
//
INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  WCHAR wch[256] = { L'\0' };
  static HFONT hFontTitle;
  static HICON hIcon = NULL;

  switch (umsg)
  {
  case WM_INITDIALOG:
  {
    {
      if (!hIcon) {
        hIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDR_MAINWND), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
      }

      SetDlgItemText(hwnd, IDC_VERSION, L"" VERSION_FILEVERSION_LONG);

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
    }

    SetDlgItemText(hwnd, IDC_SCI_VERSION, VERSION_SCIVERSION);
    SetDlgItemText(hwnd, IDC_COPYRIGHT, VERSION_LEGALCOPYRIGHT);
    SetDlgItemText(hwnd, IDC_AUTHORNAME, VERSION_AUTHORNAME);
    SetDlgItemText(hwnd, IDC_COMPILER, VERSION_COMPILER);

    if (GetDlgItem(hwnd, IDC_WEBPAGE) == NULL) {
      SetDlgItemText(hwnd, IDC_WEBPAGE2, VERSION_WEBPAGEDISPLAY);
      ShowWindow(GetDlgItem(hwnd, IDC_WEBPAGE2), SW_SHOWNORMAL);
    }
    else {
      StringCchPrintf(wch, COUNTOF(wch), L"<A>%s</A>", VERSION_WEBPAGEDISPLAY);
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

    char pAboutRes[4000];
    GetLngStringA(IDS_MUI_ABOUT_RTF_1, pAboutRes, COUNTOF(pAboutRes));
    StringCchCopyA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);
    GetLngStringA(IDS_MUI_ABOUT_RTF_2, pAboutRes, COUNTOF(pAboutRes));
    StringCchCatA(pAboutResource, COUNTOF(pAboutResource), pAboutRes);

    EDITSTREAM editStreamIn = { (DWORD_PTR)&pAboutInfo, 0, _LoadRtfCallback };
    pAboutInfo = pAboutResource;
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, SF_RTF, (LPARAM)&editStreamIn);

    
    //DWORD dwSize = _LoadStringEx(IDR_ABOUTINFO_RTF, L"RTF", NULL);
    //if (dwSize != 0) {
    //  char* pchBuffer = LocalAlloc(LPTR, dwSize + 1);
    //  pAboutInfo = pchBuffer;
    //  _LoadStringEx(IDR_ABOUTINFO_RTF, L"RTF", pAboutInfo);
    //  SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, SF_RTF, (LPARAM)&editStreamIn);
    //  LocalFree(pchBuffer);
    //}
    //else {
    //  pAboutInfo = chErrMsg;
    //  SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, SF_RTF, (LPARAM)&editStreamIn);
    //}
    
  #else
    PARAFORMAT2 pf2;
    ZeroMemory(&pf2, sizeof(PARAFORMAT2));
    pf2.cbSize = (UINT)sizeof(PARAFORMAT2);
    pf2.dwMask = (PFM_SPACEBEFORE | PFM_SPACEAFTER | PFM_LINESPACING);
    pf2.dySpaceBefore = 48;     // paragraph
    pf2.dySpaceAfter = 48;      // paragraph
    pf2.dyLineSpacing = 24;     // [twips]
    pf2.bLineSpacingRule = 5;   // 5: dyLineSpacing/20 is the spacing, in lines, from one line to the next.
    SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
    SetDlgItemText(hwnd, IDC_RICHEDITABOUT, ABOUT_INFO_PLAIN);
  #endif

    CenterDlgInParent(hwnd);
  }
  return true;


  case WM_PAINT:
    if (hIcon) {
      RECT rt;
      GetWindowRect(hwnd, &rt);
      HDC hdc = GetWindowDC(hwnd);
      DrawIconEx(hdc, 16, 32, hIcon, 128, 128, 0, NULL, DI_NORMAL);
      ReleaseDC(hwnd, hdc);
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
          WCHAR hLink[256];
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
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, true);
        return true;
      }
    }
    break;

  case WM_COMMAND:

    switch (LOWORD(wParam))
    {
    case IDC_RIZONEBMP:
      ShellExecute(hwnd, L"open", L"https://www.rizonesoft.com", NULL, NULL, SW_SHOWNORMAL);
      break;

    case IDC_COPYVERSTRG:
      {
        WCHAR wchVerInfo[1024] = { L'\0' };
        WCHAR wchAuthInfo[128] = { L'\0' };
        StringCchCopy(wchVerInfo, COUNTOF(wchVerInfo), L"" VERSION_FILEVERSION_LONG);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_SCIVERSION);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_COMPILER);
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n");
        GetLngString(IDS_MUI_TRANSL_AUTHOR, wchAuthInfo, COUNTOF(wchAuthInfo));
        StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchAuthInfo);
        SetClipboardTextW(g_hwndMain, wchVerInfo);
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
INT_PTR CALLBACK RunDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }
        // MakeBitmapButton(hwnd,IDC_SEARCHEXE,g_hInstance,IDB_OPEN);
        SendDlgItemMessage(hwnd,IDC_COMMANDLINE,EM_LIMITTEXT,MAX_PATH - 1,0);
        SetDlgItemText(hwnd,IDC_COMMANDLINE,(LPCWSTR)lParam);
        SHAutoComplete(GetDlgItem(hwnd,IDC_COMMANDLINE),SHACF_FILESYSTEM);

        CenterDlgInParent(hwnd);
      }
      return true;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd,IDC_SEARCHEXE);
      return false;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_SEARCHEXE:
          {
            WCHAR szArgs[MAX_PATH] = { L'\0' };
            WCHAR szArg2[MAX_PATH] = { L'\0' };
            WCHAR szFile[MAX_PATH * 2] = { L'\0' };
            WCHAR szFilter[256] = { L'\0' };
            OPENFILENAME ofn;
            ZeroMemory(&ofn,sizeof(OPENFILENAME));

            GetDlgItemText(hwnd,IDC_COMMANDLINE,szArgs,COUNTOF(szArgs));
            ExpandEnvironmentStringsEx(szArgs,COUNTOF(szArgs));
            ExtractFirstArgument(szArgs,szFile,szArg2,MAX_PATH);

            GetLngString(IDS_MUI_FILTER_EXE,szFilter,COUNTOF(szFilter));
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
              if (StringCchLen(szArg2,COUNTOF(szArg2)))
              {
                StringCchCat(szFile,COUNTOF(szFile),L" ");
                StringCchCat(szFile,COUNTOF(szFile),szArg2);
              }
              SetDlgItemText(hwnd,IDC_COMMANDLINE,szFile);
            }

            PostMessage(hwnd,WM_NEXTDLGCTL,1,0);
          }
          break;


        case IDC_COMMANDLINE:
          {
            bool bEnableOK = false;
            WCHAR args[MAX_PATH] = { L'\0' };

            if (GetDlgItemText(hwnd,IDC_COMMANDLINE,args,MAX_PATH))
              if (ExtractFirstArgument(args,args,NULL,MAX_PATH))
                if (StringCchLenW(args,COUNTOF(args)))
                  bEnableOK = true;

            DialogEnableWindow(hwnd,IDOK,bEnableOK);
          }
          break;


        case IDOK:
          {
            WCHAR arg1[MAX_PATH] = { L'\0' };
            WCHAR arg2[MAX_PATH] = { L'\0' };
            WCHAR wchDirectory[MAX_PATH] = { L'\0' };

            if (GetDlgItemText(hwnd,IDC_COMMANDLINE,arg1,MAX_PATH))
            {
              bool bQuickExit = false;

              ExpandEnvironmentStringsEx(arg1,COUNTOF(arg1));
              ExtractFirstArgument(arg1,arg1,arg2,MAX_PATH);

              if (StringCchCompareIN(arg1,COUNTOF(arg1), TSTRG(APPNAME),-1) == 0 ||
                  StringCchCompareIN(arg1,COUNTOF(arg1),L"notepad3.exe",-1) == 0) {
                GetModuleFileName(NULL,arg1,COUNTOF(arg1));
                bQuickExit = true;
              }

              if (StringCchLenW(g_wchCurFile, FILE_ARG_BUF)) {
                StringCchCopy(wchDirectory,COUNTOF(wchDirectory),g_wchCurFile);
                PathRemoveFileSpec(wchDirectory);
              }

              SHELLEXECUTEINFO sei;
              ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
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
                EndDialog(hwnd,IDOK);
                ShellExecuteEx(&sei);
              }

              else {
                if (ShellExecuteEx(&sei))
                  EndDialog(hwnd,IDOK);

                else
                  PostMessage(hwnd,WM_NEXTDLGCTL,
                    (WPARAM)(GetDlgItem(hwnd,IDC_COMMANDLINE)),1);
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
//  RunDlg()
//
void RunDlg(HWND hwnd,LPCWSTR lpstrDefault)
{
  ThemedDialogBoxParam(g_hLngResContainer, MAKEINTRESOURCE(IDD_MUI_RUN), hwnd, RunDlgProc, (LPARAM)lpstrDefault);
}


//=============================================================================
//
//  OpenWithDlgProc()
//
extern WCHAR g_tchOpenWithDir[MAX_PATH];
extern int  g_flagNoFadeHidden;

extern int cxOpenWithDlg;
extern int cyOpenWithDlg;

INT_PTR CALLBACK OpenWithDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }

        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        ResizeDlg_Init(hwnd,cxOpenWithDlg,cyOpenWithDlg,IDC_RESIZEGRIP3);

        SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)lParam);

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_OPENWITHDIR),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,&lvc);
        DirList_Init(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL);
        DirList_Fill(GetDlgItem(hwnd,IDC_OPENWITHDIR),g_tchOpenWithDir,DL_ALLOBJECTS,NULL,false,g_flagNoFadeHidden,DS_NAME,false);
        DirList_StartIconThread(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        ListView_SetItemState(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETOPENWITHDIR,g_hInstance,IDB_OPEN);

        CenterDlgInParent(hwnd);
      }
      return true;


    case WM_DESTROY:
      DirList_Destroy(GetDlgItem(hwnd,IDC_OPENWITHDIR));
      DeleteBitmapButton(hwnd,IDC_GETOPENWITHDIR);

      ResizeDlg_Destroy(hwnd,&cxOpenWithDlg,&cyOpenWithDlg);
      return false;


    case WM_SIZE:
      {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        hdwp = BeginDeferWindowPos(6);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP3,dx,dy,SWP_NOSIZE);
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
              DirList_GetDispInfo(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam,g_flagNoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;
                DialogEnableWindow(hwnd,IDOK,(pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_OPENWITHDIR)))
                SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
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
            if (GetDirectory(hwnd,IDS_MUI_OPENWITH,g_tchOpenWithDir,g_tchOpenWithDir,true))
            {
              DirList_Fill(GetDlgItem(hwnd,IDC_OPENWITHDIR),g_tchOpenWithDir,DL_ALLOBJECTS,NULL,false,g_flagNoFadeHidden,DS_NAME,false);
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
              MessageBeep(0);
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

  if (IDOK == ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_MUI_OPENWITH),
                             hwnd,OpenWithDlgProc,(LPARAM)&dliOpenWith))
  {
    WCHAR szParam[MAX_PATH] = { L'\0' };
    WCHAR wchDirectory[MAX_PATH] = { L'\0' };

    if (StringCchLenW(g_wchCurFile, FILE_ARG_BUF)) {
      StringCchCopy(wchDirectory,COUNTOF(wchDirectory),g_wchCurFile);
      PathRemoveFileSpec(wchDirectory);
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
extern WCHAR g_tchFavoritesDir[MAX_PATH];

extern int cxFavoritesDlg;
extern int cyFavoritesDlg;

INT_PTR CALLBACK FavoritesDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }

        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        ResizeDlg_Init(hwnd,cxFavoritesDlg,cyFavoritesDlg,IDC_RESIZEGRIP3);

        SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)lParam);

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_FAVORITESDIR));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_FAVORITESDIR),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,&lvc);
        DirList_Init(GetDlgItem(hwnd,IDC_FAVORITESDIR),NULL);
        DirList_Fill(GetDlgItem(hwnd,IDC_FAVORITESDIR),g_tchFavoritesDir,DL_ALLOBJECTS,NULL,false,g_flagNoFadeHidden,DS_NAME,false);
        DirList_StartIconThread(GetDlgItem(hwnd,IDC_FAVORITESDIR));
        ListView_SetItemState(GetDlgItem(hwnd,IDC_FAVORITESDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETFAVORITESDIR,g_hInstance,IDB_OPEN);

        CenterDlgInParent(hwnd);
      }
      return true;


    case WM_DESTROY:
      DirList_Destroy(GetDlgItem(hwnd,IDC_FAVORITESDIR));
      DeleteBitmapButton(hwnd,IDC_GETFAVORITESDIR);

      ResizeDlg_Destroy(hwnd,&cxFavoritesDlg,&cyFavoritesDlg);
      return false;


    case WM_SIZE:
      {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        hdwp = BeginDeferWindowPos(6);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP3,dx,dy,SWP_NOSIZE);
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
              DirList_GetDispInfo(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam,g_flagNoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd,IDC_FAVORITESDIR),lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;
                DialogEnableWindow(hwnd,IDOK,(pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_FAVORITESDIR)))
                SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
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
            if (GetDirectory(hwnd,IDS_MUI_FAVORITES,g_tchFavoritesDir,g_tchFavoritesDir,true))
            {
              DirList_Fill(GetDlgItem(hwnd,IDC_FAVORITESDIR),g_tchFavoritesDir,DL_ALLOBJECTS,NULL,false,g_flagNoFadeHidden,DS_NAME,false);
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
              MessageBeep(0);
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

  if (IDOK == ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_MUI_FAVORITES),
                             hwnd,FavoritesDlgProc,(LPARAM)&dliFavorite))
  {
    StringCchCopyN(lpstrFile,MAX_PATH,dliFavorite.szFileName,MAX_PATH);
    return(true);
  }
  return(false);
}


//=============================================================================
//
//  AddToFavDlgProc()
//
//  Controls: 100 Edit
//
INT_PTR CALLBACK AddToFavDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) {
    WCHAR *pszName;

  case WM_INITDIALOG:
    {
      pszName = (LPWSTR)lParam;
      SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pszName);

      if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }
      SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, MAX_PATH - 1, 0);
      SetDlgItemText(hwnd, 100, pszName);

      CenterDlgInParent(hwnd);
    }
    return true;


  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case 100:
      DialogEnableWindow(hwnd, IDOK, GetWindowTextLength(GetDlgItem(hwnd, 100)));
      break;

    case IDOK:
      pszName = (LPWSTR)GetWindowLongPtr(hwnd, DWLP_USER);
      GetDlgItemText(hwnd, 100, pszName,
                     MAX_PATH - 1);
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
//  AddToFavDlg()
//
bool AddToFavDlg(HWND hwnd,LPCWSTR lpszName,LPCWSTR lpszTarget)
{

  INT_PTR iResult;

  WCHAR pszName[MAX_PATH] = { L'\0' };
  StringCchCopy(pszName,COUNTOF(pszName),lpszName);

  iResult = ThemedDialogBoxParam(
              g_hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_ADDTOFAV),
              hwnd,
              AddToFavDlgProc,(LPARAM)pszName);

  if (iResult == IDOK)
  {
    if (!PathCreateFavLnk(pszName,lpszTarget,g_tchFavoritesDir)) {
      MsgBoxLng(MBWARN,IDS_MUI_FAV_FAILURE);
      return false;
    }
    else {
      MsgBoxLng(MBINFO,IDS_MUI_FAV_SUCCESS);
      return true;
    }
  }

  else
    return false;

}


//=============================================================================
//
//  FileMRUDlgProc()
//
//
extern LPMRULIST g_pFileMRU;
extern bool g_bSaveRecentFiles;
extern bool g_bPreserveCaretPos;
extern bool g_bSaveFindReplace;
extern int  cxFileMRUDlg;
extern int  cyFileMRUDlg;
extern int  g_flagNoFadeHidden;

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

  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);

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

    DWORD dwAttr = 0;

    if (ListView_GetItem(hwnd,&lvi)) {

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

      if (!g_flagNoFadeHidden &&
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

INT_PTR CALLBACK FileMRUDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SHFILEINFO shfi;
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }

        LPICONTHREADINFO lpit = (LPICONTHREADINFO)GlobalAlloc(GPTR,sizeof(ICONTHREADINFO));
        SetProp(hwnd,L"it",(HANDLE)lpit);
        lpit->hwnd = GetDlgItem(hwnd,IDC_FILEMRU);
        lpit->hThread = NULL;
        lpit->hExitThread = CreateEvent(NULL,true,false,NULL);
        lpit->hTerminatedThread = CreateEvent(NULL,true,true,NULL);

        SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)lParam);

        ResizeDlg_Init(hwnd,cxFileMRUDlg,cyFileMRUDlg,IDC_RESIZEGRIP);

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
        SendMessage(hwnd,WM_COMMAND,MAKELONG(0x00A0,1),0);

        CheckDlgButton(hwnd, IDC_SAVEMRU, DlgBtnChk(g_bSaveRecentFiles));
        CheckDlgButton(hwnd, IDC_PRESERVECARET, DlgBtnChk(g_bPreserveCaretPos));
        CheckDlgButton(hwnd, IDC_REMEMBERSEARCHPATTERN, DlgBtnChk(g_bSaveFindReplace));

        //if (!g_bSaveRecentFiles) {
        //  DialogEnableWindow(hwnd,IDC_PRESERVECARET, false);
        //  DialogEnableWindow(hwnd,IDC_REMEMBERSEARCHPATTERN, false);
        //}

        CenterDlgInParent(hwnd);
      }
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
        GlobalFree(lpit);

        g_bSaveFindReplace = (IsDlgButtonChecked(hwnd, IDC_REMEMBERSEARCHPATTERN)) ? true : false;
        g_bPreserveCaretPos = (IsDlgButtonChecked(hwnd, IDC_PRESERVECARET)) ? true : false;
        g_bSaveRecentFiles  = (IsDlgButtonChecked(hwnd, IDC_SAVEMRU)) ? true : false;

        ResizeDlg_Destroy(hwnd,&cxFileMRUDlg,&cyFileMRUDlg);
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
          SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
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

              if (!g_flagNoFadeHidden &&
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
              DialogEnableWindow(hwnd, IDOK, (cnt > 0));
              DialogEnableWindow(hwnd, IDC_REMOVE, (cnt > 0));
            }
            break;
          }
        }
      }

      return true;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case 0x00A0:
          {
            int i;
            WCHAR tch[MAX_PATH] = { L'\0' };
            LV_ITEM lvi;
            SHFILEINFO shfi;

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

            for (i = 0; i < MRU_Count(g_pFileMRU); i++) {
              MRU_Enum(g_pFileMRU,i,tch,COUNTOF(tch));
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
                if (!MRU_FindFile(g_pFileMRU, g_wchCurFile, &iCur)) {
                  iCur = -1;
                }

                // Ask...
                int answ = (LOWORD(wParam) == IDOK) ? MsgBoxLng(MBYESNOWARN, IDS_MUI_ERR_MRUDLG) 
                                                    : ((iCur == lvi.iItem) ? IDNO : IDYES);

                if (IDYES == answ) {

                  MRU_Delete(g_pFileMRU,lvi.iItem);
                  MRU_DeleteFileFromStore(g_pFileMRU,tchFileName);

                  //SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_DELETESTRING,(WPARAM)iItem,0);
                  //ListView_DeleteItem(GetDlgItem(hwnd,IDC_FILEMRU),lvi.iItem);
                  // must use IDM_VIEW_REFRESH, index might change...
                  SendMessage(hwnd,WM_COMMAND,MAKELONG(0x00A0,1),0);

                  //DialogEnableWindow(hwnd,IDOK,
                  //  (LB_ERR != SendDlgItemMessage(hwnd,IDC_GOTO,LB_GETCURSEL,0,0)));

                  cnt = ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_FILEMRU));
                  DialogEnableWindow(hwnd, IDOK, (cnt > 0));
                  DialogEnableWindow(hwnd, IDC_REMOVE, (cnt > 0));
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

  if (IDOK == ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_MUI_FILEMRU),
                hwnd,FileMRUDlgProc,(LPARAM)lpstrFile))
    return true;
  else
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
extern int g_iFileWatchingMode;
extern bool g_bResetFileWatching;
extern bool g_bChasingDocTail;

INT_PTR CALLBACK ChangeNotifyDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  switch (umsg) {
  case WM_INITDIALOG:
    {
      if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }
      CheckRadioButton(hwnd, 100, 102, 100 + g_iFileWatchingMode);
      if (g_bResetFileWatching)
        CheckDlgButton(hwnd, 103, BST_CHECKED);
      CenterDlgInParent(hwnd);
    }
    return true;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (IsDlgButtonChecked(hwnd, 100) == BST_CHECKED)
        g_iFileWatchingMode = 0;
      else if (IsDlgButtonChecked(hwnd, 101) == BST_CHECKED)
        g_iFileWatchingMode = 1;
      else
        g_iFileWatchingMode = 2;

      g_bResetFileWatching = (IsDlgButtonChecked(hwnd, 103) == BST_CHECKED) ? true : false;

      if (g_bChasingDocTail) { SendMessage(g_hwndMain, WM_COMMAND, MAKELONG(IDM_VIEW_CHASING_DOCTAIL, 1), 0); }

      EndDialog(hwnd, IDOK);
      break;

    case IDCANCEL:
      EndDialog(hwnd, IDCANCEL);
      break;
    }
    return true;
  }
  UNUSED(lParam);

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
              g_hLngResContainer,
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
INT_PTR CALLBACK ColumnWrapDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{

  static UINT *piNumber;

  switch (umsg) {
  case WM_INITDIALOG:
    {
      piNumber = (UINT*)lParam;
      if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }
      SetDlgItemInt(hwnd, IDC_COLUMNWRAP, *piNumber, false);
      SendDlgItemMessage(hwnd, IDC_COLUMNWRAP, EM_LIMITTEXT, 15, 0);
      CenterDlgInParent(hwnd);
    }
    return true;


  case WM_COMMAND:

    switch (LOWORD(wParam)) {

    case IDOK:
      {
        BOOL fTranslated;
        UINT iNewNumber = GetDlgItemInt(hwnd, IDC_COLUMNWRAP, &fTranslated, FALSE);
        if (fTranslated) {
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
              g_hLngResContainer,
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
extern int  iWordWrapMode;
extern int  iWordWrapIndent;
extern int  iWordWrapSymbols;
extern bool bShowWordWrapSymbols;

INT_PTR CALLBACK WordWrapSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  switch (umsg) {

  case WM_INITDIALOG:
    {
      if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }
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
      SendDlgItemMessage(hwnd, 100, CB_SETCURSEL, (WPARAM)iWordWrapIndent, 0);
      SendDlgItemMessage(hwnd, 101, CB_SETCURSEL, (WPARAM)(bShowWordWrapSymbols) ? iWordWrapSymbols % 10 : 0, 0);
      SendDlgItemMessage(hwnd, 102, CB_SETCURSEL, (WPARAM)(bShowWordWrapSymbols) ? ((iWordWrapSymbols % 100) - (iWordWrapSymbols % 10)) / 10 : 0, 0);
      SendDlgItemMessage(hwnd, 103, CB_SETCURSEL, (WPARAM)iWordWrapMode, 0);

      CenterDlgInParent(hwnd);
    }
    return true;


  case WM_COMMAND:

    switch (LOWORD(wParam)) {

    case IDOK:
      {
        int iSel = (int)SendDlgItemMessage(hwnd, 100, CB_GETCURSEL, 0, 0);
        iWordWrapIndent = iSel;

        bShowWordWrapSymbols = false;
        iSel = (int)SendDlgItemMessage(hwnd, 101, CB_GETCURSEL, 0, 0);
        int iSel2 = (int)SendDlgItemMessage(hwnd, 102, CB_GETCURSEL, 0, 0);
        if (iSel > 0 || iSel2 > 0) {
          bShowWordWrapSymbols = true;
          iWordWrapSymbols = iSel + iSel2 * 10;
        }

        iSel = (int)SendDlgItemMessage(hwnd, 103, CB_GETCURSEL, 0, 0);
        iWordWrapMode = iSel;

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
              g_hLngResContainer,
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
extern int iLongLineMode;

INT_PTR CALLBACK LongLineSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
  static int *piNumber;

  switch (umsg) {

  case WM_INITDIALOG:
    {
      if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }
      piNumber = (int*)lParam;
      SetDlgItemInt(hwnd, 100, *piNumber, false);
      SendDlgItemMessage(hwnd, 100, EM_LIMITTEXT, 15, 0);

      if (iLongLineMode == EDGE_LINE)
        CheckRadioButton(hwnd, 101, 102, 101);
      else
        CheckRadioButton(hwnd, 101, 102, 102);

      CenterDlgInParent(hwnd);

    }
    return true;


  case WM_COMMAND:

    switch (LOWORD(wParam)) {

    case IDOK:
      {

        BOOL fTranslated;

        UINT iNewNumber = GetDlgItemInt(hwnd, 100, &fTranslated, FALSE);

        if (fTranslated) {
          *piNumber = iNewNumber;

          iLongLineMode = (IsDlgButtonChecked(hwnd, 101)) ? EDGE_LINE : EDGE_BACKGROUND;

          EndDialog(hwnd, IDOK);
        }

        else
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, 100)), 1);

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
              g_hLngResContainer,
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
extern int  g_iTabWidth;
extern int  g_iIndentWidth;
extern bool g_bTabsAsSpaces;
extern bool g_bTabIndents;
extern bool bBackspaceUnindents;

INT_PTR CALLBACK TabSettingsDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }

        SetDlgItemInt(hwnd,100,g_iTabWidth,false);
        SendDlgItemMessage(hwnd,100,EM_LIMITTEXT,15,0);

        SetDlgItemInt(hwnd,101,g_iIndentWidth,false);
        SendDlgItemMessage(hwnd,101,EM_LIMITTEXT,15,0);

        if (g_bTabsAsSpaces)
          CheckDlgButton(hwnd,102,BST_CHECKED);

        if (g_bTabIndents)
          CheckDlgButton(hwnd,103,BST_CHECKED);

        if (bBackspaceUnindents)
          CheckDlgButton(hwnd,104,BST_CHECKED);

        CenterDlgInParent(hwnd);

      }
      return true;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDOK: {

          BOOL fTranslated1,fTranslated2;

          UINT iNewTabWidth = GetDlgItemInt(hwnd,100,&fTranslated1,FALSE);
          UINT iNewIndentWidth = GetDlgItemInt(hwnd,101,&fTranslated2,FALSE);

          if (fTranslated1 && fTranslated2)
          {
            g_iTabWidth = iNewTabWidth;
            g_iIndentWidth = iNewIndentWidth;

            g_bTabsAsSpaces = (IsDlgButtonChecked(hwnd,102)) ? true : false;

            g_bTabIndents = (IsDlgButtonChecked(hwnd,103)) ? true : false;

            bBackspaceUnindents = (IsDlgButtonChecked(hwnd,104)) ? true : false;

            EndDialog(hwnd,IDOK);
          }

          else
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,(fTranslated1) ? 101 : 100)),1);

          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return true;

  }

  UNUSED(lParam);

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
              g_hLngResContainer,
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
  bool bRecodeOnly;
  int  idEncoding;
  int  cxDlg;
  int  cyDlg;
} ENCODEDLG, *PENCODEDLG;

INT_PTR CALLBACK SelectDefEncodingDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static PENCODEDLG pdd;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        HBITMAP hbmp;
        HIMAGELIST himl;

        pdd = (PENCODEDLG)lParam;
        if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }

        hbmp = LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_ENCODING),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
        hbmp = ResizeImageForCurrentDPI(hbmp);

        himl = ImageList_Create(16,16,ILC_COLOR32|ILC_MASK,0,0);
        ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
        DeleteObject(hbmp);
        SendDlgItemMessage(hwnd,IDC_ENCODINGLIST,CBEM_SETIMAGELIST,0,(LPARAM)himl);
        SendDlgItemMessage(hwnd,IDC_ENCODINGLIST,CB_SETEXTENDEDUI,true,0);

        Encoding_AddToComboboxEx(GetDlgItem(hwnd,IDC_ENCODINGLIST),pdd->idEncoding,0);

        CheckDlgButton(hwnd, IDC_USEASREADINGFALLBACK, DlgBtnChk(bUseDefaultForFileEncoding));
        CheckDlgButton(hwnd,IDC_NOUNICODEDETECTION, DlgBtnChk(bSkipUnicodeDetection));
        CheckDlgButton(hwnd, IDC_NOANSICPDETECTION, DlgBtnChk(bSkipANSICodePageDetection));
        CheckDlgButton(hwnd,IDC_ASCIIASUTF8, DlgBtnChk(bLoadASCIIasUTF8));
        CheckDlgButton(hwnd,IDC_NFOASOEM, DlgBtnChk(bLoadNFOasOEM));
        CheckDlgButton(hwnd,IDC_ENCODINGFROMFILEVARS, DlgBtnChk(bNoEncodingTags));

        CenterDlgInParent(hwnd);
      }
      return true;


    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            if (Encoding_GetFromComboboxEx(GetDlgItem(hwnd,IDC_ENCODINGLIST),&pdd->idEncoding)) {
              if (pdd->idEncoding < 0) {
                MsgBoxLng(MBWARN,IDS_MUI_ERR_ENCODINGNA);
                EndDialog(hwnd,IDCANCEL);
              }
              else {
                bUseDefaultForFileEncoding = (IsDlgButtonChecked(hwnd, IDC_USEASREADINGFALLBACK) == BST_CHECKED);
                bSkipUnicodeDetection = (IsDlgButtonChecked(hwnd,IDC_NOUNICODEDETECTION) == BST_CHECKED);
                bSkipANSICodePageDetection = (IsDlgButtonChecked(hwnd, IDC_NOANSICPDETECTION) == BST_CHECKED);
                bLoadASCIIasUTF8 = (IsDlgButtonChecked(hwnd,IDC_ASCIIASUTF8) == BST_CHECKED);
                bLoadNFOasOEM = (IsDlgButtonChecked(hwnd,IDC_NFOASOEM) == BST_CHECKED);
                bNoEncodingTags = (IsDlgButtonChecked(hwnd,IDC_ENCODINGFROMFILEVARS) == BST_CHECKED);
                EndDialog(hwnd,IDOK);
              }
            }
            else
              PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_ENCODINGLIST)),1);
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
//  SelectDefEncodingDlg()
//
bool SelectDefEncodingDlg(HWND hwnd,int *pidREncoding)
{

  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = false;
  dd.idEncoding = *pidREncoding;

  iResult = ThemedDialogBoxParam(
              g_hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_DEFENCODING),
              hwnd,
              SelectDefEncodingDlgProc,
              (LPARAM)&dd);

  if (iResult == IDOK) {
    *pidREncoding = dd.idEncoding;
    return(true);
  }
  else
    return(false);

}


//=============================================================================
//
//  SelectEncodingDlgProc()
//
//
INT_PTR CALLBACK SelectEncodingDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static PENCODEDLG pdd;
  static HWND hwndLV;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };
        HBITMAP hbmp;
        HIMAGELIST himl;

        pdd = (PENCODEDLG)lParam;

        if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }

        ResizeDlg_Init(hwnd,pdd->cxDlg,pdd->cyDlg,IDC_RESIZEGRIP4);

        hwndLV = GetDlgItem(hwnd,IDC_ENCODINGLIST);

        hbmp = LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_ENCODING),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
        hbmp = ResizeImageForCurrentDPI(hbmp);

        himl = ImageList_Create(16,16,ILC_COLOR32|ILC_MASK,0,0);
        ImageList_AddMasked(himl,hbmp,CLR_DEFAULT);
        DeleteObject(hbmp);
        ListView_SetImageList(GetDlgItem(hwnd,IDC_ENCODINGLIST),himl,LVSIL_SMALL);

        //SetExplorerTheme(hwndLV);
        ListView_SetExtendedListViewStyle(hwndLV,/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV,0,&lvc);

        Encoding_AddToListView(hwndLV,pdd->idEncoding,pdd->bRecodeOnly);

        ListView_SetColumnWidth(hwndLV,0,LVSCW_AUTOSIZE_USEHEADER);

        CenterDlgInParent(hwnd);
      }
      return true;


    case WM_DESTROY:
      ResizeDlg_Destroy(hwnd,&pdd->cxDlg,&pdd->cyDlg);
      return false;


    case WM_SIZE:
      {
        int dx;
        int dy;
        HDWP hdwp;

        ResizeDlg_Size(hwnd,lParam,&dx,&dy);

        hdwp = BeginDeferWindowPos(4);
        hdwp = DeferCtlPos(hdwp,hwnd,IDC_RESIZEGRIP4,dx,dy,SWP_NOSIZE);
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
            SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
            break;

          case LVN_ITEMCHANGED:
          case LVN_DELETEITEM: {
              int i = ListView_GetNextItem(hwndLV,-1,LVNI_ALL | LVNI_SELECTED);
              DialogEnableWindow(hwnd,IDOK,i != -1);
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
          if (Encoding_GetFromListView(hwndLV,&pdd->idEncoding)) {
            if (pdd->idEncoding < 0) {
              MsgBoxLng(MBWARN,IDS_MUI_ERR_ENCODINGNA);
              EndDialog(hwnd,IDCANCEL);
            }
            else
              EndDialog(hwnd,IDOK);
          }
          else
            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_ENCODINGLIST)),1);
          
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
extern int cxEncodingDlg;
extern int cyEncodingDlg;

bool SelectEncodingDlg(HWND hwnd,int *pidREncoding)
{

  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = false;
  dd.idEncoding = *pidREncoding;
  dd.cxDlg = cxEncodingDlg;
  dd.cyDlg = cyEncodingDlg;

  iResult = ThemedDialogBoxParam(
              g_hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_ENCODING),
              hwnd,
              SelectEncodingDlgProc,
              (LPARAM)&dd);

  cxEncodingDlg = dd.cxDlg;
  cyEncodingDlg = dd.cyDlg;

  if (iResult == IDOK) {
    *pidREncoding = dd.idEncoding;
    return(true);
  }
  else
    return(false);

}


//=============================================================================
//
//  RecodeDlg()
//
extern int cxRecodeDlg;
extern int cyRecodeDlg;

bool RecodeDlg(HWND hwnd,int *pidREncoding)
{

  INT_PTR iResult;
  ENCODEDLG dd;

  dd.bRecodeOnly = true;
  dd.idEncoding = *pidREncoding;
  dd.cxDlg = cxRecodeDlg;
  dd.cyDlg = cyRecodeDlg;

  iResult = ThemedDialogBoxParam(
              g_hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_RECODE),
              hwnd,
              SelectEncodingDlgProc,
              (LPARAM)&dd);

  cxRecodeDlg = dd.cxDlg;
  cyRecodeDlg = dd.cyDlg;

  if (iResult == IDOK) {
    *pidREncoding = dd.idEncoding;
    return(true);
  }
  else
    return(false);

}


//=============================================================================
//
//  SelectDefLineEndingDlgProc()
//
//  Controls: 100 Combo
//            IDC_CONSISTENTEOLS
//            IDC_AUTOSTRIPBLANKS
//
INT_PTR CALLBACK SelectDefLineEndingDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static int *piOption;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        int i;
        WCHAR wch[256] = { L'\0' };

        piOption = (int*)lParam;

        if (g_hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon); }

        // Load options
        for (i = 0; i < 3; i++) {
          GetLngString(IDS_EOL_WIN+i,wch,COUNTOF(wch));
          SendDlgItemMessage(hwnd,100,CB_ADDSTRING,0,(LPARAM)wch);
        }

        SendDlgItemMessage(hwnd,100,CB_SETCURSEL,(WPARAM)*piOption,0);
        SendDlgItemMessage(hwnd,100,CB_SETEXTENDEDUI,true,0);

        CheckDlgButton(hwnd,IDC_CONSISTENTEOLS, DlgBtnChk(bFixLineEndings));
        CheckDlgButton(hwnd,IDC_AUTOSTRIPBLANKS, DlgBtnChk(bAutoStripBlanks));

        CenterDlgInParent(hwnd);
      }
      return true;


    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK: {
            *piOption = (int)SendDlgItemMessage(hwnd,100,CB_GETCURSEL,0,0);
            bFixLineEndings = (IsDlgButtonChecked(hwnd,IDC_CONSISTENTEOLS) == BST_CHECKED);
            bAutoStripBlanks = (IsDlgButtonChecked(hwnd,IDC_AUTOSTRIPBLANKS) == BST_CHECKED);
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
bool SelectDefLineEndingDlg(HWND hwnd,int *iOption)
{

  INT_PTR iResult;

  iResult = ThemedDialogBoxParam(
              g_hLngResContainer,
              MAKEINTRESOURCE(IDD_MUI_DEFEOLMODE),
              hwnd,
              SelectDefLineEndingDlgProc,
              (LPARAM)iOption);

  return (iResult == IDOK) ? true : false;

}



//=============================================================================
//
//  GetMyWindowPlacement()
//
//
WININFO GetMyWindowPlacement(HWND hwnd, MONITORINFO* hMonitorInfo)
{
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(hwnd, &wndpl);

  WININFO wi;
  wi.x = wndpl.rcNormalPosition.left;
  wi.y = wndpl.rcNormalPosition.top;
  wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
  wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
  wi.max = IsZoomed(hwnd) || (wndpl.flags & WPF_RESTORETOMAXIMIZED);
  wi.zoom = SciCall_GetZoom();

  if (hMonitorInfo)
  {
    HMONITOR hMonitor = MonitorFromRect(&wndpl.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
    hMonitorInfo->cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, hMonitorInfo);
  }
  return wi;
}



//=============================================================================
//
//  DialogNewWindow()
//
//
void DialogNewWindow(HWND hwnd, bool bSaveOnRunTools, bool bSetCurFile)
{
  WCHAR szModuleName[MAX_PATH] = { L'\0' };
  WCHAR szFileName[MAX_PATH] = { L'\0' };
  WCHAR szParameters[2 * MAX_PATH + 64] = { L'\0' };
  WCHAR tch[64] = { L'\0' };

  if (bSaveOnRunTools && !FileSave(false, true, false, false)) { return; }

  GetModuleFileName(NULL, szModuleName, COUNTOF(szModuleName));

  StringCchPrintf(tch, COUNTOF(tch), L"\"-appid=%s\"", g_wchAppUserModelID);
  StringCchCopy(szParameters, COUNTOF(szParameters), tch);

  StringCchPrintf(tch, COUNTOF(tch), L" \"-sysmru=%i\"", (g_flagUseSystemMRU == 2) ? 1 : 0);
  StringCchCat(szParameters, COUNTOF(szParameters), tch);

  StringCchCat(szParameters, COUNTOF(szParameters), L" -f");
  if (StringCchLenW(g_wchIniFile, COUNTOF(g_wchIniFile))) {
    StringCchCat(szParameters, COUNTOF(szParameters), L" \"");
    StringCchCat(szParameters, COUNTOF(szParameters), g_wchIniFile);
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

  if (bSetCurFile && StringCchLenW(g_wchCurFile, FILE_ARG_BUF)) 
  {
    StringCchCopy(szFileName, COUNTOF(szFileName), g_wchCurFile);
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
  sei.lpDirectory = g_wchWorkingDirectory;
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
  WCHAR tchParam[MAX_PATH+2] = L"";
  WCHAR tchExeFile[MAX_PATH+4];
  WCHAR tchTemp[MAX_PATH+2];

  if (IniGetString(L"Settings2", L"filebrowser.exe", L"", tchTemp, COUNTOF(tchTemp))) 
  {
    ExtractFirstArgument(tchTemp, tchExeFile, tchParam, MAX_PATH+2);
    if (PathIsRelative(tchExeFile)) {
      if (!SearchPath(NULL, tchExeFile, L".exe", COUNTOF(tchTemp), tchTemp, NULL)) {
        GetModuleFileName(NULL, tchTemp, COUNTOF(tchTemp));
        PathRemoveFileSpec(tchTemp);
        PathCchAppend(tchTemp, COUNTOF(tchTemp), tchExeFile);
        StringCchCopy(tchExeFile, COUNTOF(tchExeFile), tchTemp);
      }
    }
  }
  else {
    if (!SearchPath(NULL, L"minipath.exe", L".exe", COUNTOF(tchExeFile), tchExeFile, NULL)) {
      GetModuleFileName(NULL, tchExeFile, COUNTOF(tchExeFile));
      PathRemoveFileSpec(tchExeFile);
      PathCchAppend(tchExeFile, COUNTOF(tchExeFile), L"minipath.exe");
    }
  }

  if (StringCchLenW(tchParam, COUNTOF(tchParam)) && StringCchLenW(g_wchCurFile, FILE_ARG_BUF))
    StringCchCat(tchParam, COUNTOF(tchParam), L" ");

  if (StringCchLenW(g_wchCurFile, FILE_ARG_BUF)) {
    StringCchCopy(tchTemp, COUNTOF(tchTemp), g_wchCurFile);
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

  if ((INT_PTR)sei.hInstApp < 32)
    MsgBoxLng(MBWARN, IDS_MUI_ERR_BROWSE);

}


//=============================================================================
//
//  DialogAdminExe()
//
//
extern WCHAR g_tchAdministrationExe[];

void DialogAdminExe(HWND hwnd, bool bExecInstaller)
{
  WCHAR tchExe[MAX_PATH+2];

  StringCchCopyW(tchExe, COUNTOF(tchExe), g_tchAdministrationExe);
  if (bExecInstaller && !StringCchLenW(tchExe, COUNTOF(tchExe))) { return; }

  WCHAR tchExePath[MAX_PATH + 2];
  if (!SearchPath(NULL, tchExe, L".exe", COUNTOF(tchExePath), tchExePath, NULL)) {
    // try Notepad3's dir path
    GetModuleFileName(NULL, tchExePath, COUNTOF(tchExePath));
    PathRemoveFileSpec(tchExePath);
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
  sei.lpDirectory = g_wchWorkingDirectory;
  sei.nShow = SW_SHOWNORMAL;

  if (bExecInstaller) {
    ShellExecuteEx(&sei);
    if ((INT_PTR)sei.hInstApp < 32)
    {
      if (IDOK == InfoBoxLng(MBOKCANCEL, L"NoAdminTool", IDS_MUI_ERR_ADMINEXE))
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

bool SetWindowTitle(HWND hwnd, UINT uIDAppName, bool bIsElevated, UINT uIDUntitled,
  LPCWSTR lpszFile, int iFormat, bool bModified,
  UINT uIDReadOnly, bool bReadOnly, LPCWSTR lpszExcerpt)
{

  WCHAR szUntitled[MIDSZ_BUFFER] = { L'\0' };
  WCHAR szExcrptQuot[MIDSZ_BUFFER] = { L'\0' };
  WCHAR szExcrptFmt[32] = { L'\0' };
  WCHAR szAppName[MIDSZ_BUFFER] = { L'\0' };
  WCHAR szElevatedAppName[MIDSZ_BUFFER] = { L'\0' };
  WCHAR szReadOnly[32] = { L'\0' };
  WCHAR szTitle[LARGE_BUFFER] = { L'\0' };

  if (bFreezeAppTitle)
    return false;

  if (!GetLngString(uIDAppName, szAppName, COUNTOF(szAppName)) ||
    !GetLngString(uIDUntitled, szUntitled, COUNTOF(szUntitled))) {
    return false;
  }

  if (bIsElevated) {
    FormatLngStringW(szElevatedAppName, COUNTOF(szElevatedAppName), IDS_MUI_APPTITLE_ELEVATED, szAppName);
    StringCchCopyN(szAppName, COUNTOF(szAppName), szElevatedAppName, COUNTOF(szElevatedAppName));
  }

  if (bModified)
    StringCchCopy(szTitle, COUNTOF(szTitle), pszMod);
  else
    StringCchCopy(szTitle, COUNTOF(szTitle), L"");

  if (StrIsNotEmpty(lpszExcerpt)) {
    GetLngString(IDS_MUI_TITLEEXCERPT, szExcrptFmt, COUNTOF(szExcrptFmt));
    StringCchPrintf(szExcrptQuot, COUNTOF(szExcrptQuot), szExcrptFmt, lpszExcerpt);
    StringCchCat(szTitle, COUNTOF(szTitle), szExcrptQuot);
  }

  else if (StringCchLen(lpszFile, MAX_PATH))
  {
    if (iFormat < 2 && !PathIsRoot(lpszFile))
    {
      if (StringCchCompareN(szCachedFile, COUNTOF(szCachedFile), lpszFile, MAX_PATH) != 0) {
        SHFILEINFO shfi;
        StringCchCopy(szCachedFile, COUNTOF(szCachedFile), lpszFile);
        if (SHGetFileInfo2(lpszFile, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES))
          StringCchCopy(szCachedDisplayName, COUNTOF(szCachedDisplayName), shfi.szDisplayName);
        else
          StringCchCopy(szCachedDisplayName, COUNTOF(szCachedDisplayName), PathFindFileName(lpszFile));
      }
      StringCchCat(szTitle, COUNTOF(szTitle), szCachedDisplayName);
      if (iFormat == 1) {
        WCHAR tchPath[MAX_PATH] = { L'\0' };
        StringCchCopyN(tchPath, COUNTOF(tchPath), lpszFile, StringCchLen(lpszFile, MAX_PATH));
        PathRemoveFileSpec(tchPath);
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

  if (bReadOnly && GetLngString(uIDReadOnly, szReadOnly, COUNTOF(szReadOnly)))
  {
    StringCchCat(szTitle, COUNTOF(szTitle), L" ");
    StringCchCat(szTitle, COUNTOF(szTitle), szReadOnly);
  }

  StringCchCat(szTitle, COUNTOF(szTitle), pszSep);
  StringCchCat(szTitle, COUNTOF(szTitle), szAppName);

  return SetWindowText(hwnd, szTitle);

}


//=============================================================================
//
//  SetWindowTransparentMode()
//
void SetWindowTransparentMode(HWND hwnd, bool bTransparentMode)
{
  if (bTransparentMode) {
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    // get opacity level from registry
    int iAlphaPercent = IniGetInt(L"Settings2", L"OpacityLevel", 75);
    iAlphaPercent = clampi(iAlphaPercent, 0, 100);
    BYTE const bAlpha = (BYTE)MulDiv(iAlphaPercent, 255, 100);
    SetLayeredWindowAttributes(hwnd, 0, bAlpha, LWA_ALPHA);
  }
  else
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
}


//=============================================================================
//
//  CenterDlgInParent()
//
void CenterDlgInParent(HWND hDlg)
{
  RECT rcDlg;
  HWND hParent;
  RECT rcParent;
  MONITORINFO mi;
  HMONITOR hMonitor;

  int xMin, yMin, xMax, yMax, x, y;

  GetWindowRect(hDlg, &rcDlg);

  hParent = GetParent(hDlg);
  GetWindowRect(hParent, &rcParent);

  hMonitor = MonitorFromRect(&rcParent, MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);

  xMin = mi.rcWork.left;
  yMin = mi.rcWork.top;

  xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
  yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

  if ((rcParent.right - rcParent.left) - (rcDlg.right - rcDlg.left) > 20)
    x = rcParent.left + (((rcParent.right - rcParent.left) - (rcDlg.right - rcDlg.left)) / 2);
  else
    x = rcParent.left + 70;

  if ((rcParent.bottom - rcParent.top) - (rcDlg.bottom - rcDlg.top) > 20)
    y = rcParent.top + (((rcParent.bottom - rcParent.top) - (rcDlg.bottom - rcDlg.top)) / 2);
  else
    y = rcParent.top + 60;

  SetWindowPos(hDlg, NULL, max(xMin, min(xMax, x)), max(yMin, min(yMax, y)), 0, 0, SWP_NOZORDER | SWP_NOSIZE);

  //SnapToDefaultButton(hDlg);
}


//=============================================================================
//
//  GetDlgPos()
//
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg)
{

  RECT rcDlg;
  HWND hParent;
  RECT rcParent;

  GetWindowRect(hDlg, &rcDlg);

  hParent = GetParent(hDlg);
  GetWindowRect(hParent, &rcParent);

  // return positions relative to parent window
  *xDlg = rcDlg.left - rcParent.left;
  *yDlg = rcDlg.top - rcParent.top;

}


//=============================================================================
//
//  SetDlgPos()
//
void SetDlgPos(HWND hDlg, int xDlg, int yDlg)
{

  RECT rcDlg;
  HWND hParent;
  RECT rcParent;
  MONITORINFO mi;
  HMONITOR hMonitor;

  int xMin, yMin, xMax, yMax, x, y;

  GetWindowRect(hDlg, &rcDlg);

  hParent = GetParent(hDlg);
  GetWindowRect(hParent, &rcParent);

  hMonitor = MonitorFromRect(&rcParent, MONITOR_DEFAULTTONEAREST);
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);

  xMin = mi.rcWork.left;
  yMin = mi.rcWork.top;

  xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
  yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

  // desired positions relative to parent window
  x = rcParent.left + xDlg;
  y = rcParent.top + yDlg;

  SetWindowPos(hDlg, NULL, max(xMin, min(xMax, x)), max(yMin, min(yMax, y)), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

/*

... only if we are working with nonstandard dialog boxes ...

//=============================================================================
//
//  SnapToDefaultButton()
//
// Why doesn't the "Automatically move pointer to the default button in a dialog box"
// work for nonstandard dialog boxes, and how do I add it to my own nonstandard dialog boxes?
// https://blogs.msdn.microsoft.com/oldnewthing/20130826-00/?p=3413/
//
void SnapToDefaultButton(HWND hwndBox)
{
bool bSnapToDefButton = false;
if (SystemParametersInfo(SPI_GETSNAPTODEFBUTTON, 0, &bSnapToDefButton, 0) && bSnapToDefButton) {
// get child window at the top of the Z order.
// for all our MessageBoxs it's the OK or YES button or NULL.
HWND btn = GetWindow(hwndBox, GW_CHILD);
if (btn != NULL) {
WCHAR className[32] = L"";
GetClassName(btn, className, COUNTOF(className));
if (lstrcmpi(className, L"Button") == 0) {
RECT rect;
int x, y;
GetWindowRect(btn, &rect);
x = rect.left + (rect.right - rect.left) / 2;
y = rect.top + (rect.bottom - rect.top) / 2;
SetCursorPos(x, y);
}
}
}
}
*/


//=============================================================================
//
//  Resize Dialog Helpers()
//
typedef struct _resizedlg {
  int cxClient;
  int cyClient;
  int cxFrame;
  int cyFrame;
  int mmiPtMinX;
  int mmiPtMinY;
} RESIZEDLG, *PRESIZEDLG;


void ResizeDlg_Init(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip)
{
  RECT rc;
  WCHAR wch[64] = { L'\0' };
  int cGrip;
  RESIZEDLG *pm = LocalAlloc(LPTR, sizeof(RESIZEDLG));

  GetClientRect(hwnd, &rc);
  pm->cxClient = rc.right - rc.left;
  pm->cyClient = rc.bottom - rc.top;

  pm->cxFrame = cxFrame;
  pm->cyFrame = cyFrame;

  AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE) | WS_THICKFRAME, false, 0);
  pm->mmiPtMinX = rc.right - rc.left;
  pm->mmiPtMinY = rc.bottom - rc.top;

  if (pm->cxFrame < (rc.right - rc.left))
    pm->cxFrame = rc.right - rc.left;
  if (pm->cyFrame < (rc.bottom - rc.top))
    pm->cyFrame = rc.bottom - rc.top;

  SetProp(hwnd, L"ResizeDlg", (HANDLE)pm);

  SetWindowPos(hwnd, NULL, rc.left, rc.top, pm->cxFrame, pm->cyFrame, SWP_NOZORDER);

  SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_THICKFRAME);
  SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
  GetMenuString(GetSystemMenu(GetParent(hwnd), false), SC_SIZE, wch, COUNTOF(wch), MF_BYCOMMAND);
  InsertMenu(GetSystemMenu(hwnd, false), SC_CLOSE, MF_BYCOMMAND | MF_STRING | MF_ENABLED, SC_SIZE, wch);
  InsertMenu(GetSystemMenu(hwnd, false), SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);

  SetWindowLongPtr(GetDlgItem(hwnd, nIdGrip), GWL_STYLE,
    GetWindowLongPtr(GetDlgItem(hwnd, nIdGrip), GWL_STYLE) | SBS_SIZEGRIP | WS_CLIPSIBLINGS);
  cGrip = GetSystemMetrics(SM_CXHTHUMB);
  SetWindowPos(GetDlgItem(hwnd, nIdGrip), NULL, pm->cxClient - cGrip, pm->cyClient - cGrip, cGrip, cGrip, SWP_NOZORDER);
}

void ResizeDlg_Destroy(HWND hwnd, int *cxFrame, int *cyFrame)
{
  RECT rc;
  PRESIZEDLG pm = GetProp(hwnd, L"ResizeDlg");

  GetWindowRect(hwnd, &rc);
  *cxFrame = rc.right - rc.left;
  *cyFrame = rc.bottom - rc.top;

  RemoveProp(hwnd, L"ResizeDlg");
  LocalFree(pm);
}

void ResizeDlg_Size(HWND hwnd, LPARAM lParam, int *cx, int *cy)
{
  PRESIZEDLG pm = GetProp(hwnd, L"ResizeDlg");

  *cx = LOWORD(lParam) - pm->cxClient;
  *cy = HIWORD(lParam) - pm->cyClient;
  pm->cxClient = LOWORD(lParam);
  pm->cyClient = HIWORD(lParam);
}

void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam)
{
  PRESIZEDLG pm = GetProp(hwnd, L"ResizeDlg");

  LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
  lpmmi->ptMinTrackSize.x = pm->mmiPtMinX;
  lpmmi->ptMinTrackSize.y = pm->mmiPtMinY;
}

HDWP DeferCtlPos(HDWP hdwp, HWND hwndDlg, int nCtlId, int dx, int dy, UINT uFlags)
{
  RECT rc;
  HWND hwndCtl = GetDlgItem(hwndDlg, nCtlId);
  GetWindowRect(hwndCtl, &rc);
  MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rc, 2);
  if (uFlags & SWP_NOSIZE)
    return(DeferWindowPos(hdwp, hwndCtl, NULL, rc.left + dx, rc.top + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE));
  else
    return(DeferWindowPos(hdwp, hwndCtl, NULL, 0, 0, rc.right - rc.left + dx, rc.bottom - rc.top + dy, SWP_NOZORDER | SWP_NOMOVE));
}


//=============================================================================
//
//  MakeBitmapButton()
//
void MakeBitmapButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, UINT uBmpId)
{
  HWND hwndCtl = GetDlgItem(hwnd, nCtlId);
  BITMAP bmp;
  BUTTON_IMAGELIST bi;
  HBITMAP hBmp = LoadImage(hInstance, MAKEINTRESOURCE(uBmpId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  hBmp = ResizeImageForCurrentDPI(hBmp);
  GetObject(hBmp, sizeof(BITMAP), &bmp);
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
  HWND hwndCtl = GetDlgItem(hwnd, nCtlId);
  BUTTON_IMAGELIST bi;
  HIMAGELIST himlOld = NULL;
  HBITMAP hBmp;
  COLORMAP colormap[2];

  if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)&bi))
    himlOld = bi.himl;

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

  hBmp = CreateMappedBitmap(hInstance, IDB_PICK, 0, colormap, 2);

  bi.himl = ImageList_Create(10, 10, ILC_COLORDDB | ILC_MASK, 1, 0);
  ImageList_AddMasked(bi.himl, hBmp, RGB(0xFF, 0xFF, 0xFF));
  DeleteObject(hBmp);

  SetRect(&bi.margin, 0, 0, 4, 0);
  bi.uAlign = BUTTON_IMAGELIST_ALIGN_RIGHT;

  SendMessage(hwndCtl, BCM_SETIMAGELIST, 0, (LPARAM)&bi);
  InvalidateRect(hwndCtl, NULL, true);

  if (himlOld)
    ImageList_Destroy(himlOld);
}


//=============================================================================
//
//  DeleteBitmapButton()
//
void DeleteBitmapButton(HWND hwnd, int nCtlId)
{
  HWND hwndCtl = GetDlgItem(hwnd, nCtlId);
  BUTTON_IMAGELIST bi;
  if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)&bi))
    ImageList_Destroy(bi.himl);
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
    return (SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(_rc.right, _rc.bottom)));
  }
  return (SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc->right, rc->bottom)));
}


//=============================================================================
//
//  StatusSetText()
//
void StatusSetText(HWND hwnd, UINT nPart, LPCWSTR lpszText)
{
  if (lpszText) {
    UINT uFlags = (nPart == (UINT)STATUS_HELP) ? nPart | SBT_NOBORDERS : nPart;
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
  UINT uFlags = (nPart == STATUS_HELP) ? nPart | SBT_NOBORDERS : nPart;

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
int Toolbar_GetButtons(HWND hwnd, int cmdBase, LPWSTR lpszButtons, int cchButtons)
{
  WCHAR tchButtons[512] = { L'\0' };
  WCHAR tchItem[32] = { L'\0' };
  int i, c;
  TBBUTTON tbb;

  StringCchCopy(tchButtons, COUNTOF(tchButtons), L"");
  c = min(50, (int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0));

  for (i = 0; i < c; i++) {
    SendMessage(hwnd, TB_GETBUTTON, (WPARAM)i, (LPARAM)&tbb);
    StringCchPrintf(tchItem, COUNTOF(tchItem), L"%i ",
      (tbb.idCommand == 0) ? 0 : tbb.idCommand - cmdBase + 1);
    StringCchCat(tchButtons, COUNTOF(tchButtons), tchItem);
  }
  TrimString(tchButtons);
  StringCchCopyN(lpszButtons, cchButtons, tchButtons, COUNTOF(tchButtons));
  return(c);
}

int Toolbar_SetButtons(HWND hwnd, int cmdBase, LPCWSTR lpszButtons, LPCTBBUTTON ptbb, int ctbb)
{
  WCHAR tchButtons[MIDSZ_BUFFER];
  int i, c;
  int iCmd;

  ZeroMemory(tchButtons, COUNTOF(tchButtons) * sizeof(tchButtons[0]));
  StringCchCopyN(tchButtons, COUNTOF(tchButtons), lpszButtons, COUNTOF(tchButtons) - 2);
  TrimString(tchButtons);
  WCHAR *p = StrStr(tchButtons, L"  ");
  while (p) {
    MoveMemory((WCHAR*)p, (WCHAR*)p + 1, (lstrlen(p) + 1) * sizeof(WCHAR));
    p = StrStr(tchButtons, L"  ");  // next
  }
  c = (int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0);
  for (i = 0; i < c; i++)
    SendMessage(hwnd, TB_DELETEBUTTON, 0, 0);

  for (i = 0; i < COUNTOF(tchButtons); i++)
    if (tchButtons[i] == L' ') tchButtons[i] = 0;

  p = tchButtons;
  while (*p) {
    if (swscanf_s(p, L"%i", &iCmd) == 1) {
      iCmd = (iCmd == 0) ? 0 : iCmd + cmdBase - 1;
      for (i = 0; i < ctbb; i++) {
        if (ptbb[i].idCommand == iCmd) {
          SendMessage(hwnd, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&ptbb[i]);
          break;
        }
      }
    }
    p = StrEnd(p) + 1;
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
  UINT ppi = GetCurrentPPI(NULL);

  HTHEME hTheme = OpenThemeData(NULL, L"WINDOWSTYLE;WINDOW");
  if (hTheme) {
    LOGFONT lf;
    if (S_OK == GetThemeSysFont(hTheme,/*TMT_MSGBOXFONT*/805, &lf)) {
      if (lf.lfHeight < 0) {
        lf.lfHeight = -lf.lfHeight;
      }
      *wSize = (WORD)MulDiv(lf.lfHeight, 72, ppi);
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

inline int DialogTemplate_FontAttrSize(bool bDialogEx) {
  return (int)sizeof(WORD) * (bDialogEx ? 3 : 1);
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

DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lpDialogTemplateID, HINSTANCE hInstance) {

  HRSRC hRsrc;
  HGLOBAL hRsrcMem;
  DLGTEMPLATE *pRsrcMem;
  DLGTEMPLATE *pTemplate;
  UINT dwTemplateSize = 0;
  WCHAR wchFaceName[LF_FACESIZE];
  WORD wFontSize;
  bool bDialogEx;
  bool bHasFont;
  int cbFontAttr;
  int cbNew;
  int cbOld;
  BYTE* pbNew;
  BYTE* pb;
  BYTE* pOldControls;
  BYTE* pNewControls;
  WORD nCtrl;

  hRsrc = FindResource(hInstance, lpDialogTemplateID, RT_DIALOG);
  if (hRsrc == NULL)
    return(NULL);

  hRsrcMem = LoadResource(hInstance, hRsrc);
  pRsrcMem = (DLGTEMPLATE*)LockResource(hRsrcMem);
  dwTemplateSize = (UINT)SizeofResource(hInstance, hRsrc);

  if ((dwTemplateSize == 0) ||
    (pTemplate = LocalAlloc(LPTR, dwTemplateSize + LF_FACESIZE * 2)) == NULL) {
    UnlockResource(hRsrcMem);
    FreeResource(hRsrcMem);
    return(NULL);
  }

  CopyMemory((BYTE*)pTemplate, pRsrcMem, (size_t)dwTemplateSize);
  UnlockResource(hRsrcMem);
  FreeResource(hRsrcMem);

  if (!GetThemedDialogFont(wchFaceName, &wFontSize))
    return(pTemplate);

  bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
  bHasFont = DialogTemplate_HasFont(pTemplate);
  cbFontAttr = DialogTemplate_FontAttrSize(bDialogEx);

  if (bDialogEx)
    ((DLGTEMPLATEEX*)pTemplate)->style |= DS_SHELLFONT;
  else
    pTemplate->style |= DS_SHELLFONT;

  cbNew = cbFontAttr + (((int)StringCchLenW(wchFaceName, COUNTOF(wchFaceName)) + 1) * sizeof(WCHAR));
  pbNew = (BYTE*)wchFaceName;

  pb = DialogTemplate_GetFontSizeField(pTemplate);
  cbOld = (int)(bHasFont ? cbFontAttr + 2 * (lstrlen((WCHAR*)(pb + cbFontAttr)) + 1) : 0);

  pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~(DWORD_PTR)3);
  pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~(DWORD_PTR)3);

  nCtrl = bDialogEx ?
    (WORD)((DLGTEMPLATEEX*)pTemplate)->cDlgItems :
    (WORD)pTemplate->cdit;

  if (cbNew != cbOld && nCtrl > 0)
    MoveMemory(pNewControls, pOldControls, (size_t)(dwTemplateSize - (pOldControls - (BYTE*)pTemplate)));

  *(WORD*)pb = wFontSize;
  MoveMemory(pb + cbFontAttr, pbNew, (size_t)(cbNew - cbFontAttr));

  return(pTemplate);
}

INT_PTR ThemedDialogBoxParam(
  HINSTANCE hInstance,
  LPCTSTR lpTemplate,
  HWND hWndParent,
  DLGPROC lpDialogFunc,
  LPARAM dwInitParam) {

  INT_PTR ret;
  DLGTEMPLATE *pDlgTemplate;

  pDlgTemplate = LoadThemedDialogTemplate(lpTemplate, hInstance);
  ret = DialogBoxIndirectParam(hInstance, pDlgTemplate, hWndParent, lpDialogFunc, dwInitParam);
  if (pDlgTemplate)
    LocalFree(pDlgTemplate);

  return(ret);
}

HWND CreateThemedDialogParam(
  HINSTANCE hInstance,
  LPCTSTR lpTemplate,
  HWND hWndParent,
  DLGPROC lpDialogFunc,
  LPARAM dwInitParam) {

  HWND hwnd;
  DLGTEMPLATE *pDlgTemplate;

  pDlgTemplate = LoadThemedDialogTemplate(lpTemplate, hInstance);
  hwnd = CreateDialogIndirectParam(hInstance, pDlgTemplate, hWndParent, lpDialogFunc, dwInitParam);
  if (pDlgTemplate)
    LocalFree(pDlgTemplate);

  return(hwnd);
}



//  End of Dialogs.c
