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
*                                                  (c) Rizonesoft 2008-2024   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#include "Helpers.h"

#include <windowsx.h>
#include <commctrl.h>
#include <process.h>
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

#include "VersionEx.h"
#include "PathLib.h"
#include "Edit.h"
#include "Dlapi.h"
#include "Encoding.h"
#include "Styles.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"
#include "DarkMode/DarkMode.h"
#include "tinyexpr/tinyexpr.h"
//#include "tinyexprcpp/tinyexpr_cif.h"
#include "Resample.h"
#include "PathLib.h"

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
    if (nCode == HCBT_CREATEWND) {
        HWND const hThisWnd = (HWND)wParam;
        if (hThisWnd) {

            SetDialogIconNP3(hThisWnd);
            InitWindowCommon(hThisWnd, true);

            // get window handles
            LPCREATESTRUCT const pCreateStruct = ((LPCBT_CREATEWND)lParam)->lpcs;
            HWND const hParentWnd = pCreateStruct->hwndParent ? pCreateStruct->hwndParent : GetParentOrDesktop(hThisWnd);

            if (hParentWnd) {

                // set new coordinates
                RECT rcDlg = { 0, 0, 0, 0 };
                rcDlg.left = pCreateStruct->x;
                rcDlg.top = pCreateStruct->y;
                rcDlg.right = pCreateStruct->x + pCreateStruct->cx;
                rcDlg.bottom = pCreateStruct->y + pCreateStruct->cy;

                POINT const ptTopLeft = GetCenterOfDlgInParent(&rcDlg, hParentWnd);

                pCreateStruct->x = ptTopLeft.x;
                pCreateStruct->y = ptTopLeft.y;
            }

            // we are done
            if (s_hCBThook) {
                UnhookWindowsHookEx(s_hCBThook);
                s_hCBThook = NULL;
            }
        } else if (s_hCBThook) {
            // continue with any possible chained hooks
            return CallNextHookEx(s_hCBThook, nCode, wParam, lParam);
        }
    }
    return (LRESULT)0;
}
// -----------------------------------------------------------------------------


int MessageBoxLng(UINT uType, UINT uidMsg, ...)
{
    HSTRINGW    hfmt_str = StrgCreate(NULL);
    LPWSTR const fmt_buf = StrgWriteAccessBuf(hfmt_str, XXXL_BUFFER);
    if (!GetLngString(uidMsg, fmt_buf, (int)StrgGetAllocLength(hfmt_str))) {
        StrgDestroy(hfmt_str);
        return -1;
    }
    StrgSanitize(hfmt_str);

    HSTRINGW    htxt_str = StrgCreate(NULL);
    const PUINT_PTR argp = (PUINT_PTR)&uidMsg + 1;
    bool const bHasArgs = (argp && *argp);
    if (bHasArgs) {
        LPWSTR const txt_buf = StrgWriteAccessBuf(htxt_str, XXXL_BUFFER);
        StringCchVPrintfW(txt_buf, StrgGetAllocLength(htxt_str), StrgGet(hfmt_str), (LPVOID)argp);
        StrgSanitize(htxt_str);
    }

    uType |= MB_SETFOREGROUND;  //~ MB_TOPMOST
    if (Settings.DialogsLayoutRTL) {
        uType |= MB_RTLREADING;
    }

    // center message box to focus or main
    HWND const focus = GetFocus();
    HWND const hwnd  = focus ? focus : Globals.hwndMain;
    s_hCBThook       = SetWindowsHookEx(WH_CBT, &SetPosRelatedToParent_Hook, 0, GetCurrentThreadId());

    int const res = MessageBoxEx(hwnd, bHasArgs ? StrgGet(htxt_str) : StrgGet(hfmt_str), 
                                 _W(SAPPNAME), uType, GetLangIdByLocaleName(Globals.CurrentLngLocaleName));

    StrgDestroy(htxt_str);
    StrgDestroy(hfmt_str);

    return res;
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
        GetLangIdByLocaleName(Globals.CurrentLngLocaleName),
        (LPWSTR)&lpMsgBuf,
        0, NULL);

    if (lpMsgBuf) {
        // Display the error message and exit the process
        size_t const len = StringCchLen((LPCWSTR)lpMsgBuf, 0) + StringCchLen(lpszMessage, 0) + 160;
        LPWSTR const lpDisplayBuf = (LPWSTR)AllocMem(len * sizeof(WCHAR), HEAP_ZERO_MEMORY);

        if (lpDisplayBuf) {

            WCHAR msgFormat[128] = { L'\0' };
            GetLngString(IDS_MUI_ERR_DLG_FORMAT, msgFormat, COUNTOF(msgFormat));
            StringCchPrintf(lpDisplayBuf, len, msgFormat, lpszMessage, (LPCWSTR)lpMsgBuf, dwErrID);
            // center message box to main
            HWND const focus = GetFocus();
            HWND const hwnd = focus ? focus : Globals.hwndMain;
            s_hCBThook = SetWindowsHookEx(WH_CBT, &SetPosRelatedToParent_Hook, 0, GetCurrentThreadId());

            UINT uType = MB_ICONERROR | MB_TOPMOST | (Settings.DialogsLayoutRTL ? MB_RTLREADING : 0);
            MessageBoxEx(hwnd, lpDisplayBuf, _W(SAPPNAME) L" - ERROR", uType, GetLangIdByLocaleName(Globals.CurrentLngLocaleName));

            FreeMem(lpDisplayBuf);
        }
        LocalFree(lpMsgBuf); // LocalAlloc()
        lpMsgBuf = NULL;
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
    static HBITMAP hIconBmp = NULL;
    static HICON   hBoxIcon = NULL;
    static UINT dpi = USER_DEFAULT_SCREEN_DPI;

    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        LPINFOBOXLNG const lpMsgBox = (LPINFOBOXLNG)lParam;

        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            for (int btn = IDOK; btn <= IDCONTINUE; ++btn) {
                HWND const hBtn = GetDlgItem(hwnd, btn);
                if (hBtn) {
                    SetExplorerTheme(hBtn);
                }
            }
            SetWindowTheme(GetDlgItem(hwnd, IDC_INFOBOXCHECK), L"", L"");
        }
#endif

        switch (lpMsgBox->uType & MB_ICONMASK) {
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

        dpi = Scintilla_GetWindowDPI(hwnd);
        int const scxb = Scintilla_GetSystemMetricsForDpi(SM_CXICON, dpi);
        int const scyb = Scintilla_GetSystemMetricsForDpi(SM_CYICON, dpi);

        hIconBmp = ResampleIconToBitmap(hwnd, hIconBmp, hBoxIcon, scxb, scyb);
        if (hIconBmp) {
            SetBitmapControl(hwnd, IDC_INFOBOXICON, hIconBmp);
        }

        //UINT const tabStopDist[3] = { 4, 4, 8 };
        //SendMessage(GetDlgItem(hwnd, IDC_INFOBOXTEXT), EM_SETTABSTOPS, 3, (LPARAM)tabStopDist);

#if 0
        // Resize dialog to fit text - TODO: move buttons dynamically too
        HWND const hWndText = GetDlgItem(hwnd, IDC_INFOBOXTEXT);
        RECT       rectText = { 0 };
        GetWindowRectEx(hWndText, &rectText);

        RECT rectNew = { 0 };
        rectNew.left = 0;
        rectNew.top = 0;
        rectNew.right = rectText.right - rectText.left; // max as specified
        rectNew.bottom = rectNew.right; // max quadratic size

        HDC  hdc = GetDC(hWndText);
        DrawText(hdc, lpMsgBox->lpstrMessage, -1, &rectNew, DT_CALCRECT | DT_WORDBREAK); // calc size
        ReleaseDC(hWndText, hdc);

        // Change size of text field
        SetWindowPos(hWndText, NULL, 0, 0, rectNew.right - rectNew.left, rectNew.bottom - rectNew.top, SWP_NOMOVE | SWP_NOZORDER);
        //GetWindowRect(hWndText, &rectText);

        int const width = (rectNew.right - rectNew.left) + 100;
        int const height = (rectNew.bottom - rectNew.top) + 200;
        SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
#endif

        SetDlgItemText(hwnd, IDC_INFOBOXTEXT, lpMsgBox->lpstrMessage);

        if (lpMsgBox->bDisableCheckBox) {
            DialogEnableControl(hwnd, IDC_INFOBOXCHECK, false);
            DialogHideControl(hwnd, IDC_INFOBOXCHECK, true);
        }

        FreeMem(lpMsgBox->lpstrMessage);
        lpMsgBox->lpstrMessage = NULL;

        CenterDlgInParent(hwnd, NULL, true);
        AttentionBeep(lpMsgBox->uType);
    }
    return TRUE;


    case WM_DPICHANGED: {
        dpi = LOWORD(wParam);
        int const scxb = Scintilla_GetSystemMetricsForDpi(SM_CXICON, dpi);
        int const scyb = Scintilla_GetSystemMetricsForDpi(SM_CYICON, dpi);
        hIconBmp = ResampleIconToBitmap(hwnd, hIconBmp, hBoxIcon, scxb, scyb);
        if (hIconBmp) {
            SetBitmapControl(hwnd, IDC_INFOBOXICON, hIconBmp);
        }
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
    }
    return TRUE;


    case WM_CLOSE:
        EndDialog(hwnd, LOWORD(IDCLOSE));
        break;

    case WM_DESTROY:
        if (hIconBmp) {
            DeleteObject(hIconBmp);
        }
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            for (int btn = IDOK; btn <= IDCONTINUE; ++btn) {
                HWND const hBtn = GetDlgItem(hwnd, btn);
                if (hBtn) {
                    AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                    SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
                }
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_COMMAND: {
        LPINFOBOXLNG const lpMsgBox = (LPINFOBOXLNG)GetWindowLongPtr(hwnd, DWLP_USER);
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDYES:
        case IDRETRY:
        case IDIGNORE:
        case IDTRYAGAIN:
        case IDCONTINUE:
            if (IsButtonChecked(hwnd, IDC_INFOBOXCHECK) && StrIsNotEmpty(lpMsgBox->lpstrSetting) && Globals.bCanSaveIniFile) {
                IniFileSetLong(Paths.IniFile, Constants.SectionSuppressedMessages, lpMsgBox->lpstrSetting, LOWORD(wParam));
            }
            //[FallThrough]
        case IDNO:
        case IDABORT:
        case IDCLOSE:
        case IDCANCEL:
            EndDialog(hwnd, LOWORD(wParam));
            break;

        case IDC_INFOBOXCHECK: {
            bool const isChecked = IsButtonChecked(hwnd, IDC_INFOBOXCHECK);
            DialogEnableControl(hwnd, IDNO, !isChecked);
            DialogEnableControl(hwnd, IDABORT, !isChecked);
            DialogEnableControl(hwnd, IDIGNORE, !isChecked);
            DialogEnableControl(hwnd, IDCONTINUE, !isChecked);
            DialogEnableControl(hwnd, IDCLOSE, !isChecked);
            DialogEnableControl(hwnd, IDCANCEL, !isChecked);
            SendMessage(hwnd, WM_NEXTDLGCTL, 0, FALSE);
        }
        break;

        default:
            break;
        }
    }
    return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  InfoBoxLng()
//
//

LONG InfoBoxLng(UINT uType, LPCWSTR lpstrSetting, UINT uidMsg, ...)
{
    int const iMode = StrIsEmpty(lpstrSetting) ? 0 : IniFileGetLong(Paths.IniFile, Constants.SectionSuppressedMessages, lpstrSetting, 0);

    if (Settings.DialogsLayoutRTL) {
        uType |= MB_RTLREADING;
    }

    switch (iMode) {
    case IDOK:
    case IDYES:
    case IDCONTINUE:
        return MAKELONG(iMode, iMode);

    case 0:
    // no entry found
    case -1:
        // disable "Don't display again" check-box
        break;

    default:
        if (Globals.bCanSaveIniFile) {
            IniFileDelete(Paths.IniFile, Constants.SectionSuppressedMessages, lpstrSetting, false);
        }
        break;
    }

    WCHAR wchMessage[LARGE_BUFFER];
    if (!GetLngString(uidMsg, wchMessage, COUNTOF(wchMessage))) {
        return MAKELONG(0, iMode);
    }

    INFOBOXLNG msgBox = { 0 };
    msgBox.uType = uType;
    msgBox.lpstrMessage = AllocMem((COUNTOF(wchMessage)+1) * sizeof(WCHAR), HEAP_ZERO_MEMORY);

    const PUINT_PTR argp = (PUINT_PTR)& uidMsg + 1;
    if (argp && *argp) {
        StringCchVPrintfW(msgBox.lpstrMessage, COUNTOF(wchMessage), wchMessage, (LPVOID)argp);
    } else {
        StringCchCopy(msgBox.lpstrMessage, COUNTOF(wchMessage), wchMessage);
    }

    bool bLastError = false;
    switch (uidMsg) {
    case IDS_MUI_ERR_LOADFILE:
    case IDS_MUI_ERR_SAVEFILE:
    case IDS_MUI_CREATEINI_FAIL:
    case IDS_MUI_WRITEINI_FAIL:
    case IDS_MUI_EXPORT_FAIL:
    case IDS_MUI_ERR_ELEVATED_RIGHTS:
    case IDS_MUI_FILELOCK_ERROR:
        bLastError = true;
        break;
    default:
        //bLastError = false;
        break;
    }

    if (bLastError) {
        LPVOID lpMsgBuf = NULL;
        if (Globals.dwLastError != ERROR_SUCCESS) {
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                Globals.dwLastError,
                GetLangIdByLocaleName(Globals.CurrentLngLocaleName),
                (LPWSTR)&lpMsgBuf, 0,
                NULL);

            Globals.dwLastError = ERROR_SUCCESS; // reset;
        }

        if (lpMsgBuf) {
            StringCchCat(msgBox.lpstrMessage, COUNTOF(wchMessage), L"\n\n");
            StringCchCat(msgBox.lpstrMessage, COUNTOF(wchMessage), lpMsgBuf);
            LocalFree(lpMsgBuf);
            lpMsgBuf = NULL;
        }

        WCHAR wcht = *CharPrev(msgBox.lpstrMessage, StrEnd(msgBox.lpstrMessage, COUNTOF(wchMessage)));
        if (IsCharAlphaNumeric(wcht) || wcht == '"' || wcht == '\'') {
            StringCchCat(msgBox.lpstrMessage, COUNTOF(wchMessage), L".");
        }
    }

    msgBox.lpstrSetting = (LPWSTR)lpstrSetting;
    msgBox.bDisableCheckBox = (!Globals.bCanSaveIniFile || StrIsEmpty(lpstrSetting) || (iMode < 0)) ? true : false;

    int idDlg;
    switch (uType & MB_TYPEMASK) {

    case MB_OK:    // one push button : OK. This is the default.
        idDlg = IDD_MUI_INFOBOX;
        break;

    case MB_YESNO: // contains two push buttons : Yes and No.
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
        idDlg = IDD_MUI_INFOBOX6;
        break;
    // Use this message box type instead of MB_ABORTRETRYIGNORE.
    case MB_CANCELTRYCONTINUE:  // three push buttons : Cancel, Try Again, Continue.
        idDlg = IDD_MUI_INFOBOX7;
        break;

    case MB_FILECHANGEDNOTIFY:
        idDlg = IDD_MUI_INFOBOX_FILECHANGED;
        break;
    
    default:
        idDlg = IDD_MUI_INFOBOX;
        break;
    }

    HWND focus = GetFocus();
    HWND hwnd = focus ? focus : Globals.hwndMain;

    INT_PTR const answer = ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(idDlg), hwnd, _InfoBoxLngDlgProc, (LPARAM)&msgBox);
    if (msgBox.lpstrMessage) {
        FreeMem(msgBox.lpstrMessage);
    }
    return MAKELONG(answer, iMode);
}

/*

  MinimizeToTray - Copyright 2000 Matthew Ellis <m.t.ellis@bigfoot.com>

  Changes made by flo:
   - Commented out: #include "stdafx.h"
   - Moved variable declaration: APPBARDATA appBarData;

*/

// MinimizeToTray
//
// A couple of routines to show how to make it produce a custom caption
// animation to make it look like we are minimizing to and maximizing
// from the system tray
//
// These routines are public domain, but it would be nice if you dropped
// me a line if you use them!
//
// 1.0 29.06.2000 Initial version
// 1.1 01.07.2000 The window retains it's place in the Z-order of windows
//     when minimized/hidden. This means that when restored/shown, it doen't
//     always appear as the foreground window unless we call SetForegroundWindow
//
// Copyright 2000 Matthew Ellis <m.t.ellis@bigfoot.com>
/*#include "stdafx.h"*/

// Odd. VC++6 winuser.h has IDANI_CAPTION defined (as well as IDANI_OPEN and
// IDANI_CLOSE), but the Platform SDK only has IDANI_OPEN...

// I don't know what IDANI_OPEN or IDANI_CLOSE do. Trying them in this code
// produces nothing. Perhaps they were intended for window opening and closing
// like the MAC provides...
#ifndef IDANI_OPEN
#define IDANI_OPEN 1
#endif
#ifndef IDANI_CLOSE
#define IDANI_CLOSE 2
#endif
#ifndef IDANI_CAPTION
#define IDANI_CAPTION 3
#endif

#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30

// Returns the rect of where we think the system tray is. This will work for
// all current versions of the shell. If explorer isn't running, we try our
// best to work with a 3rd party shell. If we still can't find anything, we
// return a rect in the lower right hand corner of the screen
static bool GetTrayWndRect(LPRECT lpTrayRect) {
    APPBARDATA appBarData;
    // First, we'll use a quick hack method. We know that the taskbar is a window
    // of class Shell_TrayWnd, and the status tray is a child of this of class
    // TrayNotifyWnd. This provides us a window rect to minimize to. Note, however,
    // that this is not guaranteed to work on future versions of the shell. If we
    // use this method, make sure we have a backup!
    HWND hShellTrayWnd = FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);
    if (hShellTrayWnd) {
        HWND hTrayNotifyWnd = FindWindowEx(hShellTrayWnd, NULL, TEXT("TrayNotifyWnd"), NULL);
        if (hTrayNotifyWnd) {
            GetWindowRect(hTrayNotifyWnd, lpTrayRect);
            return true;
        }
    }

    // OK, we failed to get the rect from the quick hack. Either explorer isn't
    // running or it's a new version of the shell with the window class names
    // changed (how dare Microsoft change these undocumented class names!) So, we
    // try to find out what side of the screen the taskbar is connected to. We
    // know that the system tray is either on the right or the bottom of the
    // taskbar, so we can make a good guess at where to minimize to
    /*APPBARDATA appBarData;*/
    appBarData.cbSize = sizeof(appBarData);
    if (SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData)) {
        // We know the edge the taskbar is connected to, so guess the rect of the
        // system tray. Use various fudge factor to make it look good
        switch (appBarData.uEdge) {
        case ABE_LEFT:
        case ABE_RIGHT:
            // We want to minimize to the bottom of the taskbar
            lpTrayRect->top = appBarData.rc.bottom - 100;
            lpTrayRect->bottom = appBarData.rc.bottom - 16;
            lpTrayRect->left = appBarData.rc.left;
            lpTrayRect->right = appBarData.rc.right;
            break;

        case ABE_TOP:
        case ABE_BOTTOM:
            // We want to minimize to the right of the taskbar
            lpTrayRect->top = appBarData.rc.top;
            lpTrayRect->bottom = appBarData.rc.bottom;
            lpTrayRect->left = appBarData.rc.right - 100;
            lpTrayRect->right = appBarData.rc.right - 16;
            break;
        }

        return true;
    }

    // Blimey, we really aren't in luck. It's possible that a third party shell
    // is running instead of explorer. This shell might provide support for the
    // system tray, by providing a Shell_TrayWnd window (which receives the
    // messages for the icons) So, look for a Shell_TrayWnd window and work out
    // the rect from that. Remember that explorer's taskbar is the Shell_TrayWnd,
    // and stretches either the width or the height of the screen. We can't rely
    // on the 3rd party shell's Shell_TrayWnd doing the same, in fact, we can't
    // rely on it being any size. The best we can do is just blindly use the
    // window rect, perhaps limiting the width and height to, say 150 square.
    // Note that if the 3rd party shell supports the same configuraion as
    // explorer (the icons hosted in NotifyTrayWnd, which is a child window of
    // Shell_TrayWnd), we would already have caught it above
    hShellTrayWnd = FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);
    if (hShellTrayWnd) {
        GetWindowRect(hShellTrayWnd, lpTrayRect);
        if (lpTrayRect->right - lpTrayRect->left > DEFAULT_RECT_WIDTH) {
            lpTrayRect->left = lpTrayRect->right - DEFAULT_RECT_WIDTH;
        }
        if (lpTrayRect->bottom - lpTrayRect->top > DEFAULT_RECT_HEIGHT) {
            lpTrayRect->top = lpTrayRect->bottom - DEFAULT_RECT_HEIGHT;
        }
        return true;
    }

    // OK. Haven't found a thing. Provide a default rect based on the current work area
    SystemParametersInfo(SPI_GETWORKAREA, 0, lpTrayRect, 0);
    lpTrayRect->left = lpTrayRect->right - DEFAULT_RECT_WIDTH;
    lpTrayRect->top = lpTrayRect->bottom - DEFAULT_RECT_HEIGHT;
    return false;
}


// Check to see if the system animation has been enabled/disabled
static bool IsSystemDrawAnimation()
{
    ANIMATIONINFO ai = { sizeof(ANIMATIONINFO), 0 };
    SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);
    return ai.iMinAnimate;
}

bool HasDrawAnimation() {
    return IsSystemDrawAnimation() && Settings2.DrawAnimatedWindow;
}

void MinimizeWndToTray(HWND hWnd) {

    if (HasDrawAnimation()) {
        // Get the rect of the window. It is safe to use the rect of the whole
        // window - DrawAnimatedRects will only draw the caption
        RECT rcFrom;
        GetWindowRect(hWnd, &rcFrom);
        RECT rcTo;
        GetTrayWndRect(&rcTo);
        // Get the system to draw our animation for us
        DrawAnimatedRects(hWnd, IDANI_CAPTION, &rcFrom, &rcTo);
    }

    // Add the tray icon. If we add it before the call to DrawAnimatedRects,
    // the taskbar gets erased, but doesn't get redrawn until DAR finishes.
    // This looks untidy, so call the functions in this order
    ShowNotifyIcon(hWnd, true);
    SetNotifyIconTitle(hWnd);

    // Hide the window
    ShowWindow(hWnd, SW_HIDE);

    Globals.bMinimizedToTray = true;
}

void MinimizeWndToTaskbar(HWND hWnd)
{
    if (!Settings2.DrawAnimatedWindow) {
        ShowWindow(hWnd, SW_HIDE); // hide first, before minimize
    }
    ShowWindow(hWnd, SW_MINIMIZE);
    ShowWindow(hWnd, SW_SHOW);  // show taskbar icon
}

void RestoreWndFromTray(HWND hWnd) {

    if (HasDrawAnimation()) {
        // Get the rect of the tray and the window. Note that the window rect
        // is still valid even though the window is hidden
        RECT rcFrom;
        GetTrayWndRect(&rcFrom);
        RECT rcTo;
        GetWindowRect(hWnd, &rcTo);
        // needed, if minimized: WININFO wi = GetMyWindowPlacement(hWnd, NULL, 0, false); RectFromWinInfo(&wi, &rcTo);
        // Get the system to draw our animation for us
        DrawAnimatedRects(hWnd, IDANI_CAPTION, &rcFrom, &rcTo);
    }

    // Show the window, and make sure we're the foreground window
    ShowWindow(hWnd, SW_SHOW);

    SetActiveWindow(hWnd);
    SetForegroundWindow(hWnd);

    // Remove the tray icon. As described above, remove the icon after the
    // call to DrawAnimatedRects, or the taskbar will not refresh itself
    // properly until DAR finished
    ShowNotifyIcon(hWnd, false);

    Globals.bMinimizedToTray = false;
}

void RestoreWndFromTaskbar(HWND hWnd)
{
    if (!Settings2.DrawAnimatedWindow) {
        ShowWindow(hWnd, SW_HIDE); // hide first, before restore
    }
    ShowWindow(hWnd, SW_RESTORE);
    ShowWindow(hWnd, SW_SHOW);

    SetActiveWindow(hWnd);
    SetForegroundWindow(hWnd);
}


//=============================================================================
//
//  DisplayCmdLineHelp()
//
#if 0
void DisplayCmdLineHelp(HWND hwnd)
{
    WCHAR szText[2048] = { L'\0' };
    GetLngString(IDS_MUI_CMDLINEHELP,szText,COUNTOF(szText));

    MSGBOXPARAMS mbp = { 0 };
    mbp.cbSize = sizeof(MSGBOXPARAMS);
    mbp.hwndOwner = hwnd;
    mbp.hInstance = Globals.hInstance;
    mbp.lpszText = szText;
    mbp.lpszCaption = _W(SAPPNAME);
    mbp.dwStyle = MB_OK | MB_USERICON | MB_SETFOREGROUND;
    mbp.lpszIcon = MAKEINTRESOURCE(IDR_MAINWND);
    mbp.dwContextHelpId = 0;
    mbp.lpfnMsgBoxCallback = NULL;
    mbp.dwLanguageId = GetLangIdByLocaleName(Globals.CurrentLngLocaleName);

    hhkMsgBox = SetWindowsHookEx(WH_CBT, &_MsgBoxProc, 0, GetCurrentThreadId());

    MessageBoxIndirect(&mbp);
    //MsgBoxLng(MBINFO, IDS_MUI_CMDLINEHELP);
}
#else

static INT_PTR CALLBACK CmdLineHelpProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
        }
#endif

        WCHAR szText[4096] = { L'\0' };
        GetLngString(IDS_MUI_CMDLINEHELP, szText, COUNTOF(szText));
        SetDlgItemText(hwnd, IDC_CMDLINEHELP, szText);
        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;

    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;

#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            for (int btn = IDOK; btn <= IDCONTINUE; ++btn) {
                HWND const hBtn = GetDlgItem(hwnd, btn);
                if (hBtn) {
                    AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                    SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
                }
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
        case IDYES:
        case IDNO:
            EndDialog(hwnd, LOWORD(wParam));
            break;
        }
        return TRUE;

    default:
        break;
    }
    return FALSE;
}

INT_PTR DisplayCmdLineHelp(HWND hwnd)
{
    return ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_CMDLINEHELP), hwnd, CmdLineHelpProc, (LPARAM)L"");
}

#endif


/*
//=============================================================================
//
//  _LoadStringEx()
//
static DWORD _LoadStringEx(UINT nResId, LPCTSTR pszRsType, LPSTR strOut)
{
  LPTSTR pszResId = MAKEINTRESOURCE(nResId);

  if (Globals.hInstance == NULL)
    return FALSEL;

  HRSRC hRsrc = FindResource(Globals.hInstance, pszResId, pszRsType);

  if (hRsrc == NULL) {
    return FALSEL;
  }

  HGLOBAL hGlobal = LoadResource(Globals.hInstance, hRsrc);

  if (hGlobal == NULL) {
    return FALSEL;
  }

  const BYTE* pData = (const BYTE*)LockResource(hGlobal);

  if (pData == NULL) {
    FreeResource(hGlobal);
    return FALSEL;
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

    if (len < cb) {
        *pcb = len;
        memcpy_s(pbBuff, cb, (LPCSTR)*pstr, *pcb);
        *pstr += len;
        //*pstr = '\0';
    } else {
        *pcb = cb;
        memcpy_s(pbBuff, cb, (LPCSTR)*pstr, *pcb);
        *pstr += cb;
    }
    return FALSE;
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
    static UINT dpi = USER_DEFAULT_SCREEN_DPI;
    static HBRUSH hbrBkgnd = NULL;

    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        //~InitWindowCommon(hwnd, true);
        //~SetWindowLayoutRTL(hwnd, Settings.DialogsLayoutRTL);
        SetExplorerTheme(hwnd);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_COPYVERSTRG));
        }
#endif

        dpi = Scintilla_GetWindowDPI(hwnd);

        SetDlgItemText(hwnd, IDC_VERSION, _W(_STRG(VERSION_FILEVERSION_LONG)));
        SetDlgItemText(hwnd, IDC_SCI_VERSION, VERSION_SCIVERSION L", " VERSION_LXIVERSION L", ID='" _W(_STRG(VERSION_COMMIT_ID)) L"'");
        SetDlgItemText(hwnd, IDC_COPYRIGHT, _W(VERSION_LEGALCOPYRIGHT));
        SetDlgItemText(hwnd, IDC_AUTHORNAME, _W(VERSION_AUTHORNAME));
        SetDlgItemText(hwnd, IDC_COMPILER, VERSION_COMPILER);

        WCHAR wch[256] = { L'\0' };
        if (GetDlgItem(hwnd, IDC_WEBPAGE) == NULL) {
            SetDlgItemText(hwnd, IDC_WEBPAGE2, _W(VERSION_WEBPAGEDISPLAY));
            ShowWindow(GetDlgItem(hwnd, IDC_WEBPAGE2), SW_SHOWNORMAL);
        } else {
            StringCchPrintf(wch, COUNTOF(wch), L"<A>%s</A>", _W(VERSION_WEBPAGEDISPLAY));
            SetDlgItemText(hwnd, IDC_WEBPAGE, wch);
        }
        GetLngString(IDS_MUI_TRANSL_AUTHOR, wch, COUNTOF(wch));
        SetDlgItemText(hwnd, IDC_TRANSL_AUTH, wch);

        // --- Rich Edit Control ---
        //SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETBKGNDCOLOR, 0, (LPARAM)GetBackgroundColor(hwnd));
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETBKGNDCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SHOWSCROLLBAR, SB_VERT, TRUE);
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETZOOM, 1, 1); //, 0, 0); // OFF

        DWORD styleFlags = SES_EXTENDBACKCOLOR; // | SES_HYPERLINKTOOLTIPS;
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETEDITSTYLE, (WPARAM)styleFlags, (LPARAM)styleFlags);
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_AUTOURLDETECT,
            (WPARAM)(AURL_ENABLEURL|AURL_ENABLEEAURLS|AURL_ENABLETELNO), (LPARAM)("http:https:"));

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

        CenterDlgInParent(hwnd, NULL, false);

        HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, IDC_SCI_VERSION, WM_GETFONT, 0, 0);
        if (hFont) {
            LOGFONT lf = { 0 };
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
            if (UseDarkMode()) {
                SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0x80, 0x80, 0x80));
            }
            SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, SF_RTF, (LPARAM)&editStreamIn);
        }
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SHOWSCROLLBAR, SB_HORZ, (LPARAM)(dpi > USER_DEFAULT_SCREEN_DPI));

        // RichEdit-Ctrl DPI-BUG: it initially uses the DPI setting of
        // the main(1) screen instead it's current parent window screen DPI
        UINT const dpiPrime = Scintilla_GetWindowDPI(NULL);
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETZOOM, (WPARAM)dpi, (LPARAM)dpiPrime);

        int const width  = ScaleIntByDPI(136, dpi);
        int const height = ScaleIntByDPI(41, dpi);
        HBITMAP   hBmp   = LoadImage(Globals.hInstance, MAKEINTRESOURCE(IDR_RIZBITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        SetBitmapControlResample(hwnd, IDC_RIZONEBMP, hBmp, width, height);
        DeleteObject(hBmp);
    }
    break;

    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;

    case WM_DESTROY:
        if (hVersionFont) {
            DeleteObject(hVersionFont);
            hVersionFont = NULL;
        }
        break;

    case WM_DPICHANGED: {
        dpi = LOWORD(wParam);
        //dpi.y = HIWORD(wParam);

        // render rich-edit control text again
        if (!StrIsEmptyA(pAboutResource)) {
            pAboutInfo              = pAboutResource;
            EDITSTREAM editStreamIn = {(DWORD_PTR)&pAboutInfo, 0, _LoadRtfCallback};
            if (UseDarkMode()) {
                SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(0xA0,0xA0,0xA0));
            }
            SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_STREAMIN, SF_RTF, (LPARAM)&editStreamIn);
        }
        SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SHOWSCROLLBAR, SB_HORZ, (LPARAM)(dpi > USER_DEFAULT_SCREEN_DPI));
        //~SendDlgItemMessage(hwnd, IDC_RICHEDITABOUT, EM_SETZOOM, (WPARAM)dpi.y, (LPARAM)USER_DEFAULT_SCREEN_DPI);

        int const width  = ScaleIntByDPI(136, dpi);
        int const height = ScaleIntByDPI(41, dpi);
        HBITMAP   hBmp   = LoadImage(Globals.hInstance, MAKEINTRESOURCE(IDR_RIZBITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        SetBitmapControlResample(hwnd, IDC_RIZONEBMP, hBmp, width, height);
        DeleteObject(hBmp);

        HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, IDC_SCI_VERSION, WM_GETFONT, 0, 0);
        if (hFont) {
            LOGFONT lf = { 0 };
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

        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
    }
    break;

#ifdef D_NP3_WIN10_DARK_MODE

        //case WM_ERASEBKGND:
        //  if (UseDarkMode()) {
        //    HDC const hdc = (HDC)wParam;
        //    SelectObject((HDC)wParam, Globals.hbrDarkModeBkgBrush);
        //    RECT rc;
        //    GetClientRect(hwnd, &rc);
        //    SetMapMode(hdc, MM_ANISOTROPIC);
        //    SetWindowExtEx(hdc, 100, 100, NULL);
        //    SetViewportExtEx(hdc, rc.right, rc.bottom, NULL);
        //    FillRect(hdc, &rc, Globals.hbrDarkModeBkgBrush);
        //  }
        //  return TRUE;

CASE_WM_CTLCOLOR_SET:
    return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
    break;


    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;


    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDC_COPYVERSTRG };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC const hdc = GetDC(hwnd);  // ClientArea
        if (hdc) {
            BeginPaint(hwnd, &ps);
            SetMapMode(hdc, MM_TEXT);

            int const   iconSize  = 128;
            int const   dpiWidth  = ScaleIntByDPI(iconSize, dpi);
            int const   dpiHeight = ScaleIntByDPI(iconSize, dpi);
            HICON const hicon     = (dpiHeight > 128) ? Globals.hDlgIcon256 : Globals.hDlgIcon128;
            if (hicon) {
                //RECT rc = {0};
                //MapWindowPoints(GetDlgItem(hwnd, IDC_INFO_GROUPBOX), hwnd, (LPPOINT)&rc, 2);
                DrawIconEx(hdc, ScaleIntByDPI(10, dpi), ScaleIntByDPI(10, dpi), hicon, dpiWidth, dpiHeight, 0, NULL, DI_NORMAL);
            }

            ReleaseDC(hwnd, hdc);
            EndPaint(hwnd, &ps);
        }
    }
    return FALSE;


    case WM_NOTIFY: {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code) {
        case NM_CLICK:
        case NM_RETURN: {
            switch (pnmhdr->idFrom) {
            case IDC_WEBPAGE:
                ShellExecute(hwnd, L"open", L"https://www.rizonesoft.com", NULL, NULL, SW_SHOWNORMAL);
                break;

            default:
                break;
            }
        }
        break;

        case EN_LINK: { // hyperlink from RichEdit Ctrl
            ENLINK* penLink = (ENLINK *)lParam;
            if (penLink->msg == WM_LBUTTONDOWN) {
                WCHAR hLink[256] = { L'\0' };
                TEXTRANGE txtRng = { 0 };
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

    case WM_SETCURSOR: {
        if ((LOWORD(lParam) == HTCLIENT) &&
                (GetDlgCtrlID((HWND)wParam) == IDC_RIZONEBMP)) {
            SetCursor(LoadCursor(NULL, IDC_HAND));
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR)true);
            return TRUE;
        }
    }
    break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
        case IDC_RIZONEBMP:
            ShellExecute(hwnd, L"open", _W(VERSION_WEBPAGEDISPLAY), NULL, NULL, SW_SHOWNORMAL);
            break;

        case IDC_COPYVERSTRG: {
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

            bool const bDarkModeSupported = IsDarkModeSupported();
            bool const bIsThemeEnabled = ShouldAppsUseDarkModeEx();
            StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\nWindows Colors 'Dark-Mode' Theme is %s.", 
                bDarkModeSupported ? (bIsThemeEnabled ?  L"SUPPORTED and SELECTED" : L"SUPPORTED but NOT SELECTED") : L"NO SUPPORTED");
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_SCIVERSION);
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_LXIVERSION);
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n" VERSION_ONIGURUMA);

            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), (IsProcessElevated() ? L"\n- Process is elevated." : L"\n- Process is not elevated"));
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), (IsUserInAdminGroup() ? L"\n- User is in Admin-Group." : L"\n- User is not in Admin-Group"));

            StringCchCopy(wchBuf, COUNTOF(wchBuf), MUI_BASE_LNG_ID);
            #if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)
                for (unsigned lng = 0; lng < MuiLanguages_CountOf(); ++lng) {
                if (IsMUILanguageActive(lng)) {
                    StringCchCopy(wchBuf, COUNTOF(wchBuf), GetMUILocaleNameByIndex(lng));
                    break;
                }
            }
            #endif

            StringCchPrintf(wchBuf2, ARRAYSIZE(wchBuf2), L"\n- Locale -> %s (CP:'%s')",
                            wchBuf, g_Encodings[CPI_ANSI_DEFAULT].wchLabel);
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf2);

            StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Current Encoding -> '%s'", Encoding_GetLabel(Encoding_GetCurrent()));
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

            if (bDarkModeSupported && bIsThemeEnabled) {
                StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Dark-Mode enabled -> %s", CheckDarkModeEnabled() ? L"YES" : L"NO");
                StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);
            }

            StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Screen-Resolution -> %i x %i [pix]", ResX, ResY);
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

            StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Display-DPI -> %i x %i  (Scale: %i%%).", dpi, dpi, ScaleIntToDPI(hwnd, 100));
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

            StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Rendering-Technology -> '%s'", Settings.RenderingTechnology ? L"DIRECT-WRITE" : L"GDI");
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

            StringCchPrintf(wchBuf, COUNTOF(wchBuf), L"\n- Zoom -> %i%%.", SciCall_GetZoom());
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf);

            Style_GetLexerDisplayName(Style_GetCurrentLexerPtr(), wchBuf, COUNTOF(wchBuf));
            StringCchPrintf(wchBuf2, ARRAYSIZE(wchBuf2), L"\n- Current Lexer -> '%s'", wchBuf);
            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), wchBuf2);

            StringCchCat(wchVerInfo, COUNTOF(wchVerInfo), L"\n");

            // --------------------------------------------------------------------

            SetClipboardText(Globals.hwndMain, wchVerInfo, StringCchLen(wchVerInfo,0));
        }
        break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd, IDOK);
            break;
        }
        return TRUE;
    }
    return FALSE;
}



//=============================================================================
//
//  RunDlgProc()
//
static INT_PTR CALLBACK RunDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_SEARCHEXE));
        }
#endif

        // MakeBitmapButton(hwnd,IDC_SEARCHEXE,IDB_OPEN, -1, -1);
        SendDlgItemMessage(hwnd, IDC_COMMANDLINE, EM_LIMITTEXT, CMDLN_LENGTH_LIMIT - 1, 0);
        SetDlgItemText(hwnd, IDC_COMMANDLINE, (LPCWSTR)lParam);
        SHAutoComplete(GetDlgItem(hwnd, IDC_COMMANDLINE), SHACF_FILESYSTEM);

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;

    case WM_DESTROY:
        DeleteBitmapButton(hwnd, IDC_SEARCHEXE);
        return FALSE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL, IDC_SEARCHEXE };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDC_SEARCHEXE: {

            HPATHL hfile_pth = Path_Allocate(NULL);
            LPWSTR const file_buf = Path_WriteAccessBuf(hfile_pth, CMDLN_LENGTH_LIMIT);

            HSTRINGW hargs_str = StrgCreate(NULL);
            LPWSTR const  args_buf = StrgWriteAccessBuf(hargs_str, CMDLN_LENGTH_LIMIT);
            HSTRINGW hargs2_str = StrgCreate(NULL);
            LPWSTR const args2_buf = StrgWriteAccessBuf(hargs2_str, StrgGetAllocLength(hargs_str));

            HSTRINGW hflt_str = StrgCreate(NULL);
            LPWSTR const flt_buf = StrgWriteAccessBuf(hflt_str, EXTENTIONS_FILTER_BUFFER);

            GetDlgItemText(hwnd, IDC_COMMANDLINE, args_buf, (int)StrgGetAllocLength(hargs_str));
            StrgSanitize(hargs_str);
            ExpandEnvironmentStrgs(hargs_str, false);

            ExtractFirstArgument(args_buf, file_buf, args2_buf, CMDLN_LENGTH_LIMIT);
            Path_Sanitize(hfile_pth);
            StrgSanitize(hargs2_str);

            GetLngString(IDS_MUI_FILTER_EXE, flt_buf, (int)StrgGetAllocLength(hflt_str));
            StrgSanitize(hflt_str);

            PrepareFilterStr(flt_buf);

            OPENFILENAME ofn = { 0 };
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = StrgGet(hflt_str);
            ofn.lpstrFile = file_buf;
            ofn.nMaxFile = (DWORD)Path_GetBufCount(hfile_pth);
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
                        | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

            if (GetOpenFileName(&ofn)) {
                Path_Sanitize(hfile_pth);
                Path_QuoteSpaces(hfile_pth, true);
                StrgReset(hargs_str, Path_Get(hfile_pth));
                if (StrgIsNotEmpty(hargs2_str)) {
                    StrgCat(hargs_str, L" ");
                    StrgCat(hargs_str, StrgGet(hargs2_str));
                }
                SetDlgItemText(hwnd, IDC_COMMANDLINE, StrgGet(hargs_str));
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, 1, 0);

            StrgDestroy(hflt_str);
            StrgDestroy(hargs2_str);
            StrgDestroy(hargs_str);
            Path_Release(hfile_pth);
        }
        break;


        case IDC_COMMANDLINE: {
            bool bEnableOK = false;
            HSTRINGW       hargs_str = StrgCreate(NULL);
            wchar_t* const args_buf = StrgWriteAccessBuf(hargs_str, CMDLN_LENGTH_LIMIT);

            if (GetDlgItemText(hwnd, IDC_COMMANDLINE, args_buf, (int)StrgGetAllocLength(hargs_str))) {
                StrgSanitize(hargs_str);
                if (ExtractFirstArgument(args_buf, args_buf, NULL, (int)StrgGetAllocLength(hargs_str))) {
                    StrgSanitize(hargs_str);
                    if (StrgIsNotEmpty(hargs_str)) {
                        bEnableOK = true;
                    }
                }
            }
            DialogEnableControl(hwnd, IDOK, bEnableOK);
            StrgDestroy(hargs_str);
        }
        break;


        case IDOK: {
            HPATHL         hfile_pth = Path_Allocate(NULL);
            wchar_t* const file_buf = Path_WriteAccessBuf(hfile_pth, CMDLN_LENGTH_LIMIT);
            HSTRINGW       hargs_str = StrgCreate(NULL);
            wchar_t* const args_buf = StrgWriteAccessBuf(hargs_str, CMDLN_LENGTH_LIMIT);

            if (GetDlgItemText(hwnd, IDC_COMMANDLINE, file_buf, (int)Path_GetBufCount(hfile_pth))) {
                Path_Sanitize(hfile_pth);

                bool bQuickExit = false;

                Path_ExpandEnvStrings(hfile_pth);
                ExtractFirstArgument(file_buf, file_buf, args_buf, (int)Path_GetBufCount(hfile_pth));
                Path_Sanitize(hfile_pth);
                StrgSanitize(hargs_str);

                if (StringCchCompareXI(file_buf, _W(SAPPNAME)) == 0 ||
                    StringCchCompareXI(file_buf, _W(SAPPNAME) L".exe") == 0) {
                    Path_GetModuleFilePath(hfile_pth);
                    bQuickExit = true;
                }

                HPATHL pthDirectory = NULL;
                if (Path_IsNotEmpty(Paths.CurrentFile)) {
                    pthDirectory = Path_Copy(Paths.CurrentFile);
                    Path_RemoveFileSpec(pthDirectory);
                }

                SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
                sei.fMask = SEE_MASK_DEFAULT;
                sei.hwnd = hwnd;
                sei.lpVerb = NULL;
                sei.lpFile = Path_Get(hfile_pth);
                sei.lpParameters = StrgGet(hargs_str);
                sei.lpDirectory = Path_Get(pthDirectory);
                sei.nShow = SW_SHOWNORMAL;
                if (bQuickExit) {
                    sei.fMask |= SEE_MASK_NOZONECHECKS;
                    EndDialog(hwnd, IDOK);
                    ShellExecuteExW(&sei);
                }
                else {
                    if (ShellExecuteExW(&sei)) {
                        EndDialog(hwnd, IDOK);
                    }
                    else {
                        PostMessage(hwnd, WM_NEXTDLGCTL,
                            (WPARAM)(GetDlgItem(hwnd, IDC_COMMANDLINE)), 1);
                    }
                }
                Path_Release(pthDirectory);
            }
            StrgDestroy(hargs_str);
            Path_Release(hfile_pth);
        }
        break;


        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        }

        return TRUE;

    }

    return FALSE;

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
    static HPATHL hFilePath = NULL;

    switch(umsg) {
    case WM_INITDIALOG: {

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

        hFilePath = Path_Allocate(NULL);
        Path_WriteAccessBuf(hFilePath, PATHLONG_MAX_CCH); // reserve buffer

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_GETOPENWITHDIR));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
        }
#endif

        ResizeDlg_Init(hwnd, Settings.OpenWithDlgSizeX, Settings.OpenWithDlgSizeY, IDC_RESIZEGRIP);

        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        hwndLV = GetDlgItem(hwnd, IDC_OPENWITHDIR);
        InitWindowCommon(hwndLV, true);
        InitListView(hwndLV); // DarkMode

        ListView_SetExtendedListViewStyle(hwndLV, /*LVS_EX_FULLROWSELECT|*/ LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV, 0, &lvc);
        DirList_Init(hwndLV, NULL, hFilePath);
        DirList_Fill(hwndLV, Path_Get(Settings.OpenWithDir), DL_ALLOBJECTS, NULL, false, Flags.NoFadeHidden, DS_NAME, false);
        DirList_StartIconThread(hwndLV);
        ListView_SetItemState(hwndLV, 0, LVIS_FOCUSED, LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETOPENWITHDIR,IDB_OPEN, -1, -1);

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;

    case WM_DESTROY:
        DirList_Destroy(hwndLV);
        hwndLV = NULL;
        DeleteBitmapButton(hwnd,IDC_GETOPENWITHDIR);
        Path_Release(hFilePath);
        ResizeDlg_Destroy(hwnd,&Settings.OpenWithDlgSizeX,&Settings.OpenWithDlgSizeY);
        return FALSE;


    case WM_SIZE: {
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
    return TRUE;


    case WM_GETMINMAXINFO:
        ResizeDlg_GetMinMaxInfo(hwnd, lParam);
        return TRUE;

#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL, IDC_RESIZEGRIP };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            SendMessage(hwndLV, WM_THEMECHANGED, 0, 0);

            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_OPENWITHDIR) {
            switch(pnmh->code) {
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
    return TRUE;


    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDC_GETOPENWITHDIR: {
            WCHAR szTitle[MIDSZ_BUFFER] = { L'\0' };
            GetLngString(IDS_MUI_OPENWITH, szTitle, COUNTOF(szTitle));
            if (Path_BrowseDirectory(hwnd, szTitle, Settings.OpenWithDir, Settings.OpenWithDir, true)) {
                DirList_Fill(hwndLV, Path_Get(Settings.OpenWithDir), DL_ALLOBJECTS, NULL, false, Flags.NoFadeHidden, DS_NAME, false);
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

            if (lpdli->ntype != DLE_NONE) {
                EndDialog(hwnd,IDOK);
            } else {
                SimpleBeep();
            }
        }
        break;

        case IDCANCEL:
            EndDialog(hwnd,IDCANCEL);
            break;

        }
        return TRUE;

    }
    return FALSE;

}


//=============================================================================
//
//  OpenWithDlg()
//
bool OpenWithDlg(HWND hwnd, LPCWSTR lpstrFile)
{
    bool result = false;

    DLITEM dliOpenWith = { 0 };
    dliOpenWith.mask = DLI_FILENAME;

    HPATHL hpthFileName = Path_Allocate(lpstrFile);
    dliOpenWith.pthFileName = Path_WriteAccessBuf(hpthFileName, PATHLONG_MAX_CCH);

    WCHAR chDispayName[MAX_PATH_EXPLICIT>>1] = { L'\0' };
    Path_GetDisplayName(chDispayName, COUNTOF(chDispayName), hpthFileName, NULL, true);
    dliOpenWith.strDisplayName = chDispayName;

    if (IsYesOkay(ThemedDialogBoxParam(Globals.hLngResContainer,MAKEINTRESOURCE(IDD_MUI_OPENWITH),
                                       hwnd,OpenWithDlgProc,(LPARAM)&dliOpenWith))) {

        Path_Sanitize(hpthFileName);
        if (Path_IsLnkFile(hpthFileName)) {
            Path_GetLnkPath(hpthFileName, hpthFileName);
        }

        HPATHL hpthDirectory = NULL;
        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            hpthDirectory = Path_Copy(Paths.CurrentFile);
            Path_RemoveFileSpec(hpthDirectory);
        }
        //else {
        //    pthDirectory = Path_Allocate(NULL);
        //}

        HPATHL hpthParams = Path_Allocate(lpstrFile);
        // resolve links and get short path name
        if (Path_IsLnkFile(hpthParams)) {
            Path_GetLnkPath(hpthParams, hpthParams);
        }
        Path_Sanitize(hpthParams);
        Path_QuoteSpaces(hpthParams, true);

        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
        sei.fMask = SEE_MASK_DEFAULT;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = Path_Get(hpthFileName);
        sei.lpParameters = Path_Get(hpthParams);
        sei.lpDirectory = Path_Get(hpthDirectory);
        sei.nShow = SW_SHOWNORMAL;
        result = ShellExecuteExW(&sei);

        Path_Release(hpthDirectory);
        Path_Release(hpthParams);
    }

    Path_Release(hpthFileName);
    return result;
}


//=============================================================================
//
//  FavoritesDlgProc()
//
static INT_PTR CALLBACK FavoritesDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
    static HWND hwndLV = NULL;
    static HPATHL hFilePath = NULL;

    switch(umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

        hFilePath = Path_Allocate(NULL);
        Path_WriteAccessBuf(hFilePath, PATHLONG_MAX_CCH); // reserve buffer

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_GETFAVORITESDIR));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
        }
#endif

        ResizeDlg_Init(hwnd, Settings.FavoritesDlgSizeX, Settings.FavoritesDlgSizeY, IDC_RESIZEGRIP);

        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        hwndLV = GetDlgItem(hwnd, IDC_FAVORITESDIR);
        InitWindowCommon(hwndLV, true);
        InitListView(hwndLV); // DarkMode

        ListView_SetExtendedListViewStyle(hwndLV,/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV,0,&lvc);
        DirList_Init(hwndLV, NULL, hFilePath);
        DirList_Fill(hwndLV,Path_Get(Settings.FavoritesDir),DL_ALLOBJECTS,NULL,false,Flags.NoFadeHidden,DS_NAME,false);
        DirList_StartIconThread(hwndLV);
        ListView_SetItemState(hwndLV,0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETFAVORITESDIR,IDB_OPEN, -1, -1);

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;


    case WM_DESTROY:
        DirList_Destroy(hwndLV);
        hwndLV = NULL;
        DeleteBitmapButton(hwnd,IDC_GETFAVORITESDIR);
        Path_Release(hFilePath);
        ResizeDlg_Destroy(hwnd,&Settings.FavoritesDlgSizeX,&Settings.FavoritesDlgSizeY);
        return FALSE;


    case WM_SIZE: {
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
    return TRUE;


    case WM_GETMINMAXINFO:
        ResizeDlg_GetMinMaxInfo(hwnd,lParam);
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL, IDC_RESIZEGRIP };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            SendMessage(hwndLV, WM_THEMECHANGED, 0, 0);

            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_FAVORITESDIR) {
            switch(pnmh->code) {
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
    return TRUE;


    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDC_GETFAVORITESDIR: {
            WCHAR szTitle[SMALL_BUFFER] = { L'\0' };
            GetLngString(IDS_MUI_FAVORITES, szTitle, COUNTOF(szTitle));
            if (Path_BrowseDirectory(hwnd, szTitle, Settings.FavoritesDir, Settings.FavoritesDir, true)) {
                DirList_Fill(hwndLV,Path_Get(Settings.FavoritesDir),DL_ALLOBJECTS,NULL,false,Flags.NoFadeHidden,DS_NAME,false);
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

            if (lpdli->ntype != DLE_NONE) {
                EndDialog(hwnd,IDOK);
            } else {
                SimpleBeep();
            }
        }
        break;

        case IDCANCEL:
            EndDialog(hwnd,IDCANCEL);
            break;

        }

        return TRUE;

    }

    return FALSE;

}


//=============================================================================
//
//  FavoritesDlg()
//
bool FavoritesDlg(HWND hwnd, HPATHL hpath_in_out)
{
    DLITEM dliFavorite = { 0 };
    dliFavorite.mask = DLI_FILENAME;

    HPATHL hpthFileName = Path_Allocate(NULL);
    dliFavorite.pthFileName = Path_WriteAccessBuf(hpthFileName, PATHLONG_MAX_CCH);

    HSTRINGW hstrDisplayName = StrgCreate(NULL);
    dliFavorite.strDisplayName = StrgWriteAccessBuf(hstrDisplayName, INTERNET_MAX_URL_LENGTH);

    bool res = false;
    if (IsYesOkay(ThemedDialogBoxParam(Globals.hLngResContainer,MAKEINTRESOURCE(IDD_MUI_FAVORITES),
                                       hwnd,FavoritesDlgProc,(LPARAM)&dliFavorite))) {
        Path_Sanitize(hpthFileName);
        Path_Swap(hpath_in_out, hpthFileName);
        res = true;
    }

    StrgDestroy(hstrDisplayName);
    Path_Release(hpthFileName);
    return res;
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

    case WM_INITDIALOG: {

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        
        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_ADDFAV_FILES));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
        }
#endif

        ResizeDlg_InitX(hwnd, Settings.AddToFavDlgSizeX, IDC_RESIZEGRIP);

        LPCWSTR wchNamePth = (LPCWSTR)GetWindowLongPtr(hwnd, DWLP_USER);

        SendDlgItemMessage(hwnd, IDC_ADDFAV_FILES, EM_LIMITTEXT, INTERNET_MAX_URL_LENGTH, 0); // max
        SetDlgItemTextW(hwnd, IDC_ADDFAV_FILES, wchNamePth);

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;


    case WM_DESTROY:
        ResizeDlg_Destroy(hwnd, &Settings.AddToFavDlgSizeX, NULL);
        return FALSE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        break;


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


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL, IDC_RESIZEGRIP };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_ADDFAV_FILES:
            DialogEnableControl(hwnd, IDOK, GetWindowTextLength(GetDlgItem(hwnd, IDC_ADDFAV_FILES)));
            break;

        case IDOK: {
            LPWSTR wchNamePth = (LPWSTR)GetWindowLongPtr(hwnd, DWLP_USER);
            GetDlgItemText(hwnd, IDC_ADDFAV_FILES, wchNamePth, INTERNET_MAX_URL_LENGTH);
            EndDialog(hwnd, IDOK);
        }
        break;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  AddToFavDlg()
//
bool AddToFavDlg(HWND hwnd, const HPATHL hTargetPth)
{
    WCHAR szDisplayName[INTERNET_MAX_URL_LENGTH] = { L'\0' };
    Path_GetDisplayName(szDisplayName, COUNTOF(szDisplayName), hTargetPth, NULL, true);

    INT_PTR const iResult = ThemedDialogBoxParam(
        Globals.hLngResContainer,
        MAKEINTRESOURCE(IDD_MUI_ADDTOFAV),
        hwnd,
        AddToFavDlgProc, (LPARAM)szDisplayName);

    if (IsYesOkay(iResult)) {
        StringCchCat(szDisplayName, COUNTOF(szDisplayName), L".lnk");
        if (!Path_CreateFavLnk(szDisplayName, hTargetPth, Settings.FavoritesDir)) {
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
unsigned int WINAPI FileMRUIconThread(LPVOID lpParam)
{
    BackgroundWorker *worker = (BackgroundWorker *)lpParam;

    (void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);

    DWORD dwFlags = SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED;

    HWND hwnd = worker->hwnd;
    int iMaxItem = ListView_GetItemCount(hwnd);

    int iItem = 0;
    while (iItem < iMaxItem && BackgroundWorker_Continue(worker)) {

        LV_ITEM lvi = { 0 };
        lvi.mask = LVIF_TEXT;
        lvi.pszText = Path_WriteAccessBuf(worker->hFilePath, 0);
        lvi.cchTextMax = (int)Path_GetBufCount(worker->hFilePath);
        lvi.iItem = iItem;

        SHFILEINFO shfi = { 0 };

        if (ListView_GetItem(hwnd,&lvi)) {
            Path_Sanitize(worker->hFilePath);
            DWORD dwAttr = 0;
            if (Path_IsValidUNC(worker->hFilePath) || !Path_IsExistingFile(worker->hFilePath)) {
                dwFlags |= SHGFI_USEFILEATTRIBUTES;
                dwAttr = FILE_ATTRIBUTE_NORMAL;
                shfi.dwAttributes = 0;
                SHGetFileInfoW(Path_FindFileName(worker->hFilePath), dwAttr, &shfi, sizeof(SHFILEINFO), dwFlags);
            } else {
                shfi.dwAttributes = SFGAO_LINK | SFGAO_SHARE;
                SHGetFileInfoW(Path_Get(worker->hFilePath), dwAttr, &shfi, sizeof(SHFILEINFO), dwFlags);
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

            if (Path_IsValidUNC(worker->hFilePath)) {
                dwAttr = FILE_ATTRIBUTE_NORMAL;
            } else {
                dwAttr = GetFileAttributesW(Path_Get(worker->hFilePath));
            }

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
    BackgroundWorker_End(worker, 0);
    return 0;
}


static INT_PTR CALLBACK FileMRUDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndLV = NULL;
    static HPATHL hFilePath = NULL;
    
    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

        hFilePath = Path_Allocate(NULL);
        Path_WriteAccessBuf(hFilePath, PATHLONG_MAX_CCH); // reserve buffer

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_REMOVE));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { IDC_SAVEMRU, IDC_PRESERVECARET, IDC_REMEMBERSEARCHPATTERN, IDC_AUTOLOAD_MRU_FILE, IDC_STATIC };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif

        // sync with other instances
        if (Settings.SaveRecentFiles && Globals.bCanSaveIniFile) {
            if (MRU_MergeSave(Globals.pFileMRU, true, Flags.RelativeFileMRU, Flags.PortableMyDocs)) {
                MRU_Load(Globals.pFileMRU, true);
            }
        }

        hwndLV = GetDlgItem(hwnd, IDC_FILEMRU);
        InitWindowCommon(hwndLV, true);
        InitListView(hwndLV); // DarkMode

        BackgroundWorker *worker = (BackgroundWorker *)GlobalAlloc(GPTR, sizeof(BackgroundWorker));
        SetProp(hwnd, L"it", (HANDLE)worker);
        BackgroundWorker_Init(worker, hwndLV, hFilePath);

        ResizeDlg_Init(hwnd, Settings.FileMRUDlgSizeX, Settings.FileMRUDlgSizeY, IDC_RESIZEGRIP);

        SHFILEINFO shfi = { 0 };
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        ListView_SetImageList(hwndLV,
                              (HIMAGELIST)SHGetFileInfo(L"C:\\", FILE_ATTRIBUTE_DIRECTORY,
                                      &shfi, sizeof(SHFILEINFO), SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
                              LVSIL_SMALL);

        ListView_SetImageList(hwndLV,
                              (HIMAGELIST)SHGetFileInfo(L"C:\\", FILE_ATTRIBUTE_DIRECTORY,
                                      &shfi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
                              LVSIL_NORMAL);

        ListView_SetExtendedListViewStyle(hwndLV, /*LVS_EX_FULLROWSELECT|*/ LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV, 0, &lvc);

        // Update view
        SendWMCommand(hwnd, IDC_FILEMRU_UPDATE_VIEW);

        CheckDlgButton(hwnd, IDC_SAVEMRU, SetBtn(Settings.SaveRecentFiles));
        CheckDlgButton(hwnd, IDC_PRESERVECARET, SetBtn(Settings.PreserveCaretPos));
        CheckDlgButton(hwnd, IDC_REMEMBERSEARCHPATTERN, SetBtn(Settings.SaveFindReplace));
        CheckDlgButton(hwnd, IDC_AUTOLOAD_MRU_FILE, SetBtn(Settings.AutoLoadMRUFile));

        DialogEnableControl(hwnd, IDC_PRESERVECARET, Settings.SaveRecentFiles);

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;

    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;

    case WM_DESTROY: {
        BackgroundWorker *worker = (BackgroundWorker *)GetProp(hwnd, L"it");
        BackgroundWorker_Destroy(worker);
        RemoveProp(hwnd, L"it");
        GlobalFree(worker);

        if (Settings.SaveRecentFiles) {
            MRU_Save(Globals.pFileMRU); // last instance on save wins
        }

        Settings.SaveRecentFiles  = IsButtonChecked(hwnd, IDC_SAVEMRU);
        Settings.PreserveCaretPos = IsButtonChecked(hwnd, IDC_PRESERVECARET);
        Settings.SaveFindReplace  = IsButtonChecked(hwnd, IDC_REMEMBERSEARCHPATTERN);
        Settings.AutoLoadMRUFile  = IsButtonChecked(hwnd, IDC_AUTOLOAD_MRU_FILE);

        ResizeDlg_Destroy(hwnd, &Settings.FileMRUDlgSizeX, &Settings.FileMRUDlgSizeY);

        Path_Release(hFilePath);
    }
    return FALSE;

    case WM_SIZE: {
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
        hdwp      = DeferCtlPos(hdwp, hwnd, IDC_AUTOLOAD_MRU_FILE, 0, dy, SWP_NOSIZE);
        EndDeferWindowPos(hdwp);
        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);
    }
    return TRUE;

    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;

    case WM_GETMINMAXINFO:
        ResizeDlg_GetMinMaxInfo(hwnd, lParam);
        return TRUE;

#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL, IDC_REMOVE, IDC_RESIZEGRIP };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            SendMessage(hwndLV, WM_THEMECHANGED, 0, 0);

            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_NOTIFY: {
        switch (wParam) {
        case IDC_REMOVE:
            switch (((LPNMHDR)lParam)->code) {
            case BCN_DROPDOWN: {
                const NMBCDROPDOWN* pDropDown = (NMBCDROPDOWN*)lParam;
                // Get screen coordinates of the button.
                POINT pt = { 0, 0 };
                pt.x = pDropDown->rcButton.left;
                pt.y = pDropDown->rcButton.bottom;
                ClientToScreen(pDropDown->hdr.hwndFrom, &pt);
                // Create a menu and add items.
                HMENU hSplitMenu = CreatePopupMenu();
                if (!hSplitMenu) {
                    break;
                }
                if (pDropDown->hdr.hwndFrom == GetDlgItem(hwnd, IDC_REMOVE)) {
                    WCHAR szMenu[80] = {L'\0'};
                    GetLngString(IDS_CLEAR_ALL, szMenu, COUNTOF(szMenu));
                    AppendMenu(hSplitMenu, MF_STRING, IDC_CLEAR_LIST, szMenu);
                }

                // Display the menu.
                TrackPopupMenu(hSplitMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hSplitMenu);
                return TRUE;
            }
            break;

            default:
                break;
            }
            break;

        case IDC_FILEMRU:
            if (((LPNMHDR)(lParam))->idFrom == IDC_FILEMRU) {
                switch (((LPNMHDR)(lParam))->code) {
                case NM_DBLCLK:
                    SendWMCommand(hwnd, IDOK);
                    break;

                case LVN_GETDISPINFO: {
                    // done by BackgroundWorker FileMRUIconThread()
                }
                break;

                case LVN_ITEMCHANGED:
                case LVN_DELETEITEM: {
                    UINT const cnt = ListView_GetSelectedCount(hwndLV);
                    DialogEnableControl(hwnd, IDOK, (cnt > 0));
                    // can't discard current file (its myself)
                    int cur = 0;
                    if (!MRU_FindPath(Globals.pFileMRU, Paths.CurrentFile, &cur)) {
                        cur = -1;
                    }
                    int const item = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
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
    return TRUE;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
        case IDC_FILEMRU_UPDATE_VIEW: {

            BackgroundWorker* worker = (BackgroundWorker*)GetProp(hwnd, L"it");
            BackgroundWorker_Cancel(worker);

            ListView_DeleteAllItems(hwndLV);

            LV_ITEM lvi = { 0 };
            lvi.mask = LVIF_TEXT | LVIF_IMAGE;

            SHFILEINFO shfi = { 0 };
            SHGetFileInfo(L"Icon", FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO),
                          SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
            lvi.iImage = shfi.iIcon;

            LPWSTR const szFileBuf = Path_WriteAccessBuf(hFilePath, 0);
            int const    cchFileBuf = (int)Path_GetBufCount(hFilePath);

            for (int i = 0; i < MRU_Count(Globals.pFileMRU); i++) {
                MRU_Enum(Globals.pFileMRU, i, szFileBuf, cchFileBuf);
                Path_Sanitize(hFilePath);
                //  SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_ADDSTRING,0,(LPARAM)tch); }
                //  SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_SETCARETINDEX,0,false);
                lvi.iItem   = i;
                lvi.pszText = Path_WriteAccessBuf(hFilePath, 0);
                lvi.cchTextMax = (int)Path_GetBufCount(hFilePath);
                ListView_InsertItem(hwndLV, &lvi);
            }

            UINT const cnt = ListView_GetItemCount(hwndLV);
            if (cnt > 0) {
                UINT idx = ListView_GetTopIndex(hwndLV);
                ListView_SetColumnWidth(hwndLV, idx, LVSCW_AUTOSIZE_USEHEADER);
                ListView_SetItemState(hwndLV, ((cnt > 1) ? idx + 1 : idx), LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
                //int cur = 0;
                //if (!MRU_FindPath(Globals.pFileMRU, Paths.CurrentFile, &cur)) { cur = -1; }
                //int const item = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
                //if ((cur == item) && (cnt > 1)) {
                //  ListView_SetItemState(hwndLV, idx + 1, LVIS_SELECTED, LVIS_SELECTED);
                //}
            }
            DialogEnableControl(hwnd, IDOK, (cnt > 0));
            DialogEnableControl(hwnd, IDC_REMOVE, (cnt > 0));

            BackgroundWorker_Start(worker, FileMRUIconThread, worker);
        }
        break;

        case IDC_FILEMRU:
            break;

        case IDC_SAVEMRU: {
            bool const bSaveMRU = IsButtonChecked(hwnd, IDC_SAVEMRU);
            DialogEnableControl(hwnd, IDC_PRESERVECARET, bSaveMRU);
        }
        break;

        case IDOK:
        case IDC_REMOVE: {

            if (ListView_GetSelectedCount(hwndLV)) {

                LV_ITEM lvi = { 0 };
                lvi.mask = LVIF_TEXT;
                lvi.pszText = Path_WriteAccessBuf(hFilePath, 0);
                lvi.cchTextMax = (int)Path_GetBufCount(hFilePath);
                lvi.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);

                ListView_GetItem(hwndLV, &lvi);

                Path_UnQuoteSpaces(hFilePath);
                Path_AbsoluteFromApp(hFilePath, true);

                if (!Path_IsExistingFile(hFilePath) || (LOWORD(wParam) == IDC_REMOVE)) {
                    // don't remove myself
                    int iCur = 0;
                    if (!MRU_FindPath(Globals.pFileMRU, Paths.CurrentFile, &iCur)) {
                        iCur = -1;
                    }

                    // Ask...
                    LONG const answer = IsYesOkay(wParam) ? InfoBoxLng(MB_YESNO | MB_ICONWARNING, NULL, IDS_MUI_ERR_MRUDLG)
                                        : ((iCur == lvi.iItem) ? IDNO : IDYES);

                    if (IsYesOkay(answer)) {
                        MRU_Delete(Globals.pFileMRU, lvi.iItem);
                        //SendDlgItemMessage(hwnd,IDC_FILEMRU,LB_DELETESTRING,(WPARAM)iItem,0);
                        //ListView_DeleteItem(GetDlgItem(hwnd,IDC_FILEMRU),lvi.iItem);
                        //DialogEnableWindow(hwnd,IDOK,
                        //  (LB_ERR != SendDlgItemMessage(hwnd,IDC_GOTO,LB_GETCURSEL,0,0)));
                    }
                } else { // file to load
                    HPATHL hFilePathOut = (HPATHL)GetWindowLongPtr(hwnd, DWLP_USER);
                    Path_Reset(hFilePathOut, Path_Get(hFilePath)); // (!) no Path_Swap() here
                    EndDialog(hwnd, IDOK);
                }

                // must use IDM_VIEW_REFRESH, index might change...
                SendWMCommand(hwnd, IDC_FILEMRU_UPDATE_VIEW);
            }
        }
        break;

        case IDC_CLEAR_LIST:
            ListView_DeleteAllItems(hwndLV);
            MRU_Empty(Globals.pFileMRU, Path_IsNotEmpty(Paths.CurrentFile), Globals.bCanSaveIniFile);
            SendWMCommand(hwnd, IDC_FILEMRU_UPDATE_VIEW);
            break;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  FileMRUDlg()
//
//
bool FileMRUDlg(HWND hwnd, HPATHL hFilePath_out)
{
    return (IsYesOkay(ThemedDialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_FILEMRU),
                      hwnd, FileMRUDlgProc, (LPARAM)hFilePath_out)));
}


//=============================================================================
//
//  ChangeNotifyDlgProc()
//
//  Controls: IDC_RADIO_BTN_A Radio Button (None)
//            IDC_RADIO_BTN_B Radio Button (Indicator Silent)
//            IDC_RADIO_BTN_C Radio Button (Display Message)
//            IDC_RADIO_BTN_D Radio Button (Auto-Reload)
//            IDC_RADIO_BTN_E Radio Button (Exclusive Lock)
//            IDC_CHECK_BOX_A Check Box    (Reset on New)
//            IDC_CHECK_BOX_B Check Box    (Monitoring Log)
//

static INT_PTR CALLBACK ChangeNotifyDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    static FILE_WATCHING_MODE s_FWM = FWM_NO_INIT;

    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { IDC_RADIO_BTN_A, IDC_RADIO_BTN_B, IDC_RADIO_BTN_C, IDC_RADIO_BTN_D, IDC_RADIO_BTN_E, IDC_CHECK_BOX_A, IDC_CHECK_BOX_B, -1 };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif
        if (s_FWM == FWM_NO_INIT) {
            s_FWM = Settings.FileWatchingMode;
        }
        CheckDlgButton(hwnd, IDC_CHECK_BOX_A, SetBtn(Settings.ResetFileWatching));
        CheckDlgButton(hwnd, IDC_CHECK_BOX_B, SetBtn(FileWatching.MonitoringLog));

        if (FileWatching.MonitoringLog) {
            CheckRadioButton(hwnd, IDC_RADIO_BTN_A, IDC_RADIO_BTN_E, IDC_RADIO_BTN_C);
            EnableItem(hwnd, IDC_RADIO_BTN_A, FALSE);
            EnableItem(hwnd, IDC_RADIO_BTN_B, FALSE);
            EnableItem(hwnd, IDC_RADIO_BTN_C, FALSE);
            EnableItem(hwnd, IDC_RADIO_BTN_D, FALSE);
            EnableItem(hwnd, IDC_RADIO_BTN_E, FALSE);
            EnableItem(hwnd, IDC_CHECK_BOX_A, FALSE);
        } else {
            s_FWM = FileWatching.FileWatchingMode;
            CheckRadioButton(hwnd, IDC_RADIO_BTN_A, IDC_RADIO_BTN_E, IDC_RADIO_BTN_A + s_FWM);
        }
        
        SetDlgItemInt(hwnd, IDC_FILE_CHECK_INTERVAL, (UINT)FileWatching.FileCheckInterval, FALSE);

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;

    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;

#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL, IDC_RADIO_BTN_A, IDC_RADIO_BTN_B, IDC_RADIO_BTN_C, IDC_RADIO_BTN_D, IDC_RADIO_BTN_E, IDC_CHECK_BOX_A, IDC_CHECK_BOX_B };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_CHECK_BOX_A:
            if (!IsButtonChecked(hwnd, IDC_CHECK_BOX_A)) {
                CheckRadioButton(hwnd, IDC_RADIO_BTN_A, IDC_RADIO_BTN_E, IDC_RADIO_BTN_A + s_FWM);
            }
            break;


        case IDC_CHECK_BOX_B:
            FileWatching.MonitoringLog = IsButtonChecked(hwnd, IDC_CHECK_BOX_B);
            if (FileWatching.MonitoringLog) {
                CheckRadioButton(hwnd, IDC_RADIO_BTN_A, IDC_RADIO_BTN_E, IDC_RADIO_BTN_C);
                EnableItem(hwnd, IDC_RADIO_BTN_A, FALSE);
                EnableItem(hwnd, IDC_RADIO_BTN_B, FALSE);
                EnableItem(hwnd, IDC_RADIO_BTN_C, FALSE);
                EnableItem(hwnd, IDC_RADIO_BTN_D, FALSE);
                EnableItem(hwnd, IDC_RADIO_BTN_E, FALSE);
                EnableItem(hwnd, IDC_CHECK_BOX_A, FALSE);
            } else {
                CheckRadioButton(hwnd, IDC_RADIO_BTN_A, IDC_RADIO_BTN_E, IDC_RADIO_BTN_A + s_FWM);
                EnableItem(hwnd, IDC_RADIO_BTN_A, TRUE);
                EnableItem(hwnd, IDC_RADIO_BTN_B, TRUE);
                EnableItem(hwnd, IDC_RADIO_BTN_C, TRUE);
                EnableItem(hwnd, IDC_RADIO_BTN_D, TRUE);
                EnableItem(hwnd, IDC_RADIO_BTN_E, TRUE);
                EnableItem(hwnd, IDC_CHECK_BOX_A, TRUE);
            }
            break;


        case IDC_FILE_CHECK_INTERVAL: {
            BOOL       bTranslated = FALSE;
            UINT const fChkIntv = GetDlgItemInt(hwnd, IDC_FILE_CHECK_INTERVAL, &bTranslated, FALSE);
            if (bTranslated) {
                LONG64 const clampedFCIValue = clampll((LONG64)fChkIntv, 0, MAX_FC_POLL_INTERVAL);
                if (clampedFCIValue != fChkIntv) {
                    SetDlgItemInt(hwnd, IDC_FILE_CHECK_INTERVAL, (UINT)clampedFCIValue, FALSE);
                }
            }
            else {
                SetDlgItemInt(hwnd, IDC_FILE_CHECK_INTERVAL, (UINT)Settings2.FileCheckInterval, FALSE);
            }}
            break;


        case IDOK: {
                if (IsButtonChecked(hwnd, IDC_RADIO_BTN_A)) {
                    s_FWM = FWM_DONT_CARE;
                }
                else if (IsButtonChecked(hwnd, IDC_RADIO_BTN_B)) {
                    s_FWM = FWM_INDICATORSILENT;
                }
                else if (IsButtonChecked(hwnd, IDC_RADIO_BTN_C)) {
                    s_FWM = FWM_MSGBOX;
                }
                else if (IsButtonChecked(hwnd, IDC_RADIO_BTN_D)) {
                    s_FWM = FWM_AUTORELOAD;
                }
                else if (IsButtonChecked(hwnd, IDC_RADIO_BTN_E)) {
                    s_FWM = FWM_EXCLUSIVELOCK;
                }

                Settings.ResetFileWatching = IsButtonChecked(hwnd, IDC_CHECK_BOX_A);

                if (!FileWatching.MonitoringLog) {
                    FileWatching.FileWatchingMode = s_FWM;
                }
                if (!Settings.ResetFileWatching) {
                    Settings.FileWatchingMode = s_FWM;
                }

                BOOL       bTranslated = FALSE;
                UINT const fChkIntv = GetDlgItemInt(hwnd, IDC_FILE_CHECK_INTERVAL, &bTranslated, FALSE);
                if (bTranslated) {
                    LONG64 const newFCIValue = clampll((LONG64)fChkIntv, MIN_FC_POLL_INTERVAL, MAX_FC_POLL_INTERVAL);
                    if (newFCIValue != Settings2.FileCheckInterval) {
                        FileWatching.FileCheckInterval = Settings2.FileCheckInterval = newFCIValue;
                        if (Globals.bCanSaveIniFile) {
                            IniFileSetLong(Paths.IniFile, Constants.Settings2_Section, L"FileCheckInterval", (long)newFCIValue);
                        }
                    }
                }

                if (FileWatching.MonitoringLog) {
                    FileWatching.MonitoringLog = false; // will be toggled in IDM_VIEW_CHASING_DOCTAIL
                    PostWMCommand(Globals.hwndMain, IDM_VIEW_CHASING_DOCTAIL);
                }

                EndDialog(hwnd, IDOK);
            }
            break;


        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        }
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  ChangeNotifyDlg()
//
bool ChangeNotifyDlg(HWND hwnd)
{
    INT_PTR const iResult = ThemedDialogBoxParam(
                              Globals.hLngResContainer,
                              MAKEINTRESOURCEW(IDD_MUI_CHANGENOTIFY),
                              hwnd,
                              ChangeNotifyDlgProc,
                              0);

    return IsYesOkay(iResult);

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
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
        }
#endif

        UINT const uiNumber = *((UINT*)lParam);
        SetDlgItemInt(hwnd, IDC_COLUMNWRAP, uiNumber, false);
        SendDlgItemMessage(hwnd, IDC_COLUMNWRAP, EM_LIMITTEXT, 15, 0);
        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDOK: {
            BOOL fTranslated;
            UINT const iNewNumber = GetDlgItemInt(hwnd, IDC_COLUMNWRAP, &fTranslated, FALSE);
            if (fTranslated) {
                UINT* piNumber = (UINT*)GetWindowLongPtr(hwnd, DWLP_USER);
                *piNumber = iNewNumber;

                EndDialog(hwnd, IDOK);
            } else {
                PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_COLUMNWRAP)), 1);
            }
        }
        break;


        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        }
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  ColumnWrapDlg()
//
bool ColumnWrapDlg(HWND hwnd,UINT uidDlg, UINT *iNumber)
{
    INT_PTR const iResult = ThemedDialogBoxParam(
                              Globals.hLngResContainer,
                              MAKEINTRESOURCE(uidDlg),
                              hwnd,
                              ColumnWrapDlgProc,(LPARAM)iNumber);

    return IsYesOkay(iResult);

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

    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { 100, 101, 102, 103, -1 };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif

        WCHAR tch[512];
        for (int i = 0; i < 4; i++) {
            GetDlgItemText(hwnd, 200 + i, tch, COUNTOF(tch));
            StringCchCat(tch, COUNTOF(tch), L"|");
            WCHAR* p1 = tch;
            WCHAR* p2 = StrChr(p1, L'|');
            while (p2) {
                *p2++ = L'\0';
                if (*p1) {
                    SendDlgItemMessage(hwnd, 100 + i, CB_ADDSTRING, 0, (LPARAM)p1);
                }
                p1 = p2;
                p2 = StrChr(p1, L'|');
            }
            SendDlgItemMessage(hwnd, 100 + i, CB_SETEXTENDEDUI, true, 0);
        }
        SendDlgItemMessage(hwnd, 100, CB_SETCURSEL, (WPARAM)Settings.WordWrapIndent, 0);
        SendDlgItemMessage(hwnd, 101, CB_SETCURSEL, (WPARAM)(Settings.ShowWordWrapSymbols ? Settings.WordWrapSymbols % 10 : 0), 0);
        SendDlgItemMessage(hwnd, 102, CB_SETCURSEL, (WPARAM)(Settings.ShowWordWrapSymbols ? ((Settings.WordWrapSymbols % 100) - (Settings.WordWrapSymbols % 10)) / 10 : 0), 0);
        SendDlgItemMessage(hwnd, 103, CB_SETCURSEL, (WPARAM)Settings.WordWrapMode, 0);

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDOK: {
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
        return TRUE;

    }
    return FALSE;
}


//=============================================================================
//
//  WordWrapSettingsDlg()
//
bool WordWrapSettingsDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{
    INT_PTR const iResult = ThemedDialogBoxParam(
                              Globals.hLngResContainer,
                              MAKEINTRESOURCE(uidDlg),
                              hwnd,
                              WordWrapSettingsDlgProc,(LPARAM)iNumber);

    return IsYesOkay(iResult);
}


//=============================================================================
//
//  LongLineSettingsDlgProc()
//  MIDSZ_BUFFER
//
static INT_PTR CALLBACK LongLineSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    switch (umsg) {

    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR, IDC_STATIC };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif

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
        } else {
            CheckRadioButton(hwnd, IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR, IDC_SHOWEDGELINE);
            DialogEnableControl(hwnd, IDC_SHOWEDGELINE, false);
            DialogEnableControl(hwnd, IDC_BACKGRDCOLOR, false);
        }
        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDC_MULTIEDGELINE: {
            BOOL fTranslated;
            /*UINT const iCol = */ GetDlgItemInt(hwnd, IDC_MULTIEDGELINE, &fTranslated, FALSE);
            if (fTranslated) {
                DialogEnableControl(hwnd, IDC_SHOWEDGELINE, true);
                DialogEnableControl(hwnd, IDC_BACKGRDCOLOR, true);
                CheckRadioButton(hwnd, IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR,
                                 (Settings.LongLineMode == EDGE_LINE) ? IDC_SHOWEDGELINE : IDC_BACKGRDCOLOR);
            } else {
                DialogEnableControl(hwnd, IDC_SHOWEDGELINE, false);
                DialogEnableControl(hwnd, IDC_BACKGRDCOLOR, false);
                CheckRadioButton(hwnd, IDC_SHOWEDGELINE, IDC_BACKGRDCOLOR, IDC_SHOWEDGELINE);
            }
        }
        break;

        case IDC_SHOWEDGELINE:
        case IDC_BACKGRDCOLOR:
            if (IsDialogControlEnabled(hwnd, IDC_SHOWEDGELINE)) {
                Settings.LongLineMode = IsButtonChecked(hwnd, IDC_SHOWEDGELINE) ? EDGE_LINE : EDGE_BACKGROUND;
            }
            break;

        case IDOK: {
            WCHAR wchColumnList[MIDSZ_BUFFER];
            GetDlgItemText(hwnd, IDC_MULTIEDGELINE, wchColumnList, MIDSZ_BUFFER);

            bool const bOkay = true; // TODO: parse list OK
            if (bOkay) {
                LPWSTR pszColumnList = (LPWSTR)GetWindowLongPtr(hwnd, DWLP_USER);
                StringCchCopy(pszColumnList, MIDSZ_BUFFER, wchColumnList);
                Settings.LongLineMode = IsButtonChecked(hwnd, IDC_SHOWEDGELINE) ? EDGE_LINE : EDGE_BACKGROUND;
                EndDialog(hwnd, IDOK);
            } else {
                PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_MULTIEDGELINE)), 1);
            }
        }
        break;


        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        }
        return TRUE;
    }
    return FALSE;
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

    return IsYesOkay(iResult);
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
    switch(umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { IDC_TAB_AS_SPC, IDC_TAB_INDENTS, IDC_BACKTAB_INDENTS,
                                IDC_WARN_INCONSISTENT_INDENTS, IDC_AUTO_DETECT_INDENTS, IDC_STATIC, IDC_STATIC2, IDC_STATIC3
                              };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif

        SetDlgItemInt(hwnd, IDC_TAB_WIDTH, Globals.fvCurFile.iTabWidth, false);
        SendDlgItemMessage(hwnd,IDC_TAB_WIDTH,EM_LIMITTEXT,15,0);

        SetDlgItemInt(hwnd,IDC_INDENT_DEPTH, Globals.fvCurFile.iIndentWidth,false);
        SendDlgItemMessage(hwnd,IDC_INDENT_DEPTH,EM_LIMITTEXT,15,0);

        CheckDlgButton(hwnd,IDC_TAB_AS_SPC, SetBtn(Globals.fvCurFile.bTabsAsSpaces));
        CheckDlgButton(hwnd,IDC_TAB_INDENTS, SetBtn(Globals.fvCurFile.bTabIndents));
        CheckDlgButton(hwnd,IDC_BACKTAB_INDENTS, SetBtn(Settings.BackspaceUnindents));
        CheckDlgButton(hwnd,IDC_WARN_INCONSISTENT_INDENTS, SetBtn(Settings.WarnInconsistentIndents));
        CheckDlgButton(hwnd,IDC_AUTO_DETECT_INDENTS, SetBtn(Settings.AutoDetectIndentSettings));

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_COMMAND:

        switch(LOWORD(wParam)) {
        case IDOK: {
            BOOL fTranslated1, fTranslated2;
            int const _iNewTabWidth = GetDlgItemInt(hwnd, IDC_TAB_WIDTH, &fTranslated1, FALSE);
            int const _iNewIndentWidth = GetDlgItemInt(hwnd, IDC_INDENT_DEPTH, &fTranslated2, FALSE);

            if (fTranslated1 && fTranslated2) {
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
            } else {
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
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  TabSettingsDlg()
//
bool TabSettingsDlg(HWND hwnd,UINT uidDlg,int *iNumber)
{
    INT_PTR const iResult = ThemedDialogBoxParam(
                              Globals.hLngResContainer,
                              MAKEINTRESOURCE(uidDlg),
                              hwnd,
                              TabSettingsDlgProc,(LPARAM)iNumber);

    return IsYesOkay(iResult);
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

    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //~SetExplorerTheme(GetDlgItem(hwnd, IDC_ENCODINGLIST)); ~ OWNERDRAWN -> WM_DRAWITEM
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { IDC_ENCODINGLIST, IDC_USEASREADINGFALLBACK, IDC_ASCIIASUTF8, IDC_RELIABLE_DETECTION_RES,
                                IDC_NFOASOEM, IDC_ENCODINGFROMFILEVARS, IDC_NOUNICODEDETECTION, IDC_NOANSICPDETECTION, IDC_STATIC, IDC_STATIC2
                              };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif

        PENCODEDLG const pdd  = (PENCODEDLG)lParam;
        HBITMAP hbmp = LoadImage(Globals.hInstance, MAKEINTRESOURCE(IDB_ENCODING), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        hbmp = ResampleImageBitmap(hwnd, hbmp, -1, -1);

        HIMAGELIST himl = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
        ImageList_AddMasked(himl, hbmp, CLR_DEFAULT);
        DeleteObject(hbmp);
        SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CBEM_SETIMAGELIST, 0, (LPARAM)himl);
        SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CB_SETEXTENDEDUI, true, 0);
        //SendDlgItemMessage(hwnd, IDC_ENCODINGLIST, CBEM_SETEXTENDEDSTYLE, 0, CBES_EX_TEXTENDELLIPSIS);

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

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

    case WM_DRAWITEM: {
        /// TODO: migrate: currently "ComboBoxEx32" control is used, instead of COMBOBOX control
        /// "ComboBoxEx32" does not support WM_DRAWITEM (OwnerDrawn)
        /// see https://docs.microsoft.com/en-us/windows/win32/controls/comboboxex-control-reference
        /// vs
        /// https://docs.microsoft.com/en-us/windows/win32/controls/create-an-owner-drawn-combo-box
        ///
        if (LOWORD(wParam) == IDC_ENCODINGLIST) {
            const DRAWITEMSTRUCT *const pDIS = (const DRAWITEMSTRUCT *const)lParam;
            //HWND const hWndItem = pDIS->hwndItem;
            HDC const hdc = pDIS->hDC;
            //RECT const rc = pDIS->rcItem;
            SetModeBkColor(hdc, UseDarkMode());
            SetModeTextColor(hdc, UseDarkMode());
        }
    }
    break;

#endif


    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ENCODINGLIST:
        case IDC_USEASREADINGFALLBACK:
        case IDC_ASCIIASUTF8: {
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
            } else if (s_iEnc == CPI_ANSI_DEFAULT) {
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
                } else {
                    Settings.UseDefaultForFileEncoding = IsButtonChecked(hwnd, IDC_USEASREADINGFALLBACK);
                    Settings.LoadASCIIasUTF8 = IsButtonChecked(hwnd, IDC_ASCIIASUTF8);
                    Settings.UseReliableCEDonly = IsButtonChecked(hwnd, IDC_RELIABLE_DETECTION_RES);
                    Settings.LoadNFOasOEM = IsButtonChecked(hwnd, IDC_NFOASOEM);
                    Settings.NoEncodingTags = !IsButtonChecked(hwnd, IDC_ENCODINGFROMFILEVARS);
                    Settings.SkipUnicodeDetection = !IsButtonChecked(hwnd, IDC_NOUNICODEDETECTION);
                    Settings.SkipANSICodePageDetection = !IsButtonChecked(hwnd, IDC_NOANSICPDETECTION);
                    EndDialog(hwnd, IDOK);
                }
            } else {
                PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_ENCODINGLIST)), 1);
            }
        }
        break;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  SelectDefEncodingDlg()
//
bool SelectDefEncodingDlg(HWND hwnd, cpi_enc_t* pidREncoding)
{
    ENCODEDLG dd = { 0 };
    dd.bRecodeOnly = false;
    dd.idEncoding = *pidREncoding;

    INT_PTR const iResult = ThemedDialogBoxParam(
                              Globals.hLngResContainer,
                              MAKEINTRESOURCE(IDD_MUI_DEFENCODING),
                              hwnd,
                              SelectDefEncodingDlgProc,
                              (LPARAM)&dd);

    if (IsYesOkay(iResult)) {
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

    switch(umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
        }
#endif

        PENCODEDLG const pdd = (PENCODEDLG)lParam;
        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };
        ResizeDlg_Init(hwnd, pdd->cxDlg, pdd->cyDlg, IDC_RESIZEGRIP);

        hwndLV = GetDlgItem(hwnd, IDC_ENCODINGLIST);
        InitWindowCommon(hwndLV, true);
        InitListView(hwndLV);

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

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;


    case WM_DESTROY: {
        ImageList_Destroy(himl);
        himl = NULL;
        PENCODEDLG pdd = (PENCODEDLG)GetWindowLongPtr(hwnd, DWLP_USER);
        ResizeDlg_Destroy(hwnd, &pdd->cxDlg, &pdd->cyDlg);
    }
    return FALSE;


    case WM_SIZE: {
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
    return TRUE;


    case WM_GETMINMAXINFO:
        ResizeDlg_GetMinMaxInfo(hwnd,lParam);
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL, IDC_RESIZEGRIP };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            SendMessage(hwndLV, WM_THEMECHANGED, 0, 0);

            UpdateWindowEx(hwnd);
        }
        break;

#endif


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
    return TRUE;


    case WM_COMMAND:

        switch(LOWORD(wParam)) {
        case IDOK: {
            PENCODEDLG pdd = (PENCODEDLG)GetWindowLongPtr(hwnd, DWLP_USER);
            if (Encoding_GetFromListView(hwndLV, &pdd->idEncoding)) {
                if (pdd->idEncoding < 0) {
                    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_ENCODINGNA);
                    EndDialog(hwnd, IDCANCEL);
                } else {
                    EndDialog(hwnd, IDOK);
                }
            } else {
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
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  SelectEncodingDlg()
//
bool SelectEncodingDlg(HWND hwnd, cpi_enc_t* pidREncoding, bool bRecode)
{
    ENCODEDLG dd = { 0 };
    dd.bRecodeOnly = bRecode;
    dd.idEncoding = *pidREncoding;
    dd.cxDlg = bRecode ? Settings.RecodeDlgSizeX : Settings.EncodingDlgSizeX;
    dd.cyDlg = bRecode ? Settings.RecodeDlgSizeY : Settings.EncodingDlgSizeY;

    INT_PTR const iResult = ThemedDialogBoxParam(
                              Globals.hLngResContainer,
                              MAKEINTRESOURCE(bRecode ? IDD_MUI_RECODE : IDD_MUI_ENCODING),
                              hwnd,
                              SelectEncodingDlgProc,
                              (LPARAM)&dd);

    if (bRecode) {
        Settings.RecodeDlgSizeX = dd.cxDlg;
        Settings.RecodeDlgSizeY = dd.cyDlg;
    }
    else {
        Settings.EncodingDlgSizeX = dd.cxDlg;
        Settings.EncodingDlgSizeY = dd.cyDlg;
    }

    if (IsYesOkay(iResult)) {
        *pidREncoding = dd.idEncoding;
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  SelectDefLineEndingDlgProc()
//
//
static INT_PTR CALLBACK SelectDefLineEndingDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
    switch(umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { IDC_EOLMODELIST, IDC_WARN_INCONSISTENT_EOLS, IDC_CONSISTENT_EOLS, IDC_AUTOSTRIPBLANKS, IDC_STATIC };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif

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

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_COMMAND:
        switch(LOWORD(wParam)) {
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
        return TRUE;
    }
    return FALSE;
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

    return IsYesOkay(iResult);
}



//=============================================================================
//
//  WarnLineEndingDlgProc()
//
//
static INT_PTR CALLBACK WarnLineEndingDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            SetWindowTheme(GetDlgItem(hwnd, IDC_WARN_INCONSISTENT_EOLS), L"", L""); // remove theme for BS_AUTORADIOBUTTON
        }
#endif

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
        CenterDlgInParent(hwnd, NULL, false);

        AttentionBeep(MB_ICONEXCLAMATION);
    }
    return TRUE;

#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
    return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
    break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL: {
            EditFileIOStatus* status = (EditFileIOStatus*)GetWindowLongPtr(hwnd, DWLP_USER);
            const int iEOLMode = (int)SendDlgItemMessage(hwnd, IDC_EOLMODELIST, CB_GETCURSEL, 0, 0);
            status->iEOLMode = iEOLMode;
            Settings.WarnInconsistEOLs = IsButtonChecked(hwnd, IDC_WARN_INCONSISTENT_EOLS);
            EndDialog(hwnd, LOWORD(wParam));
        }
        break;
        }
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  WarnLineEndingDlg()
//
bool WarnLineEndingDlg(HWND hwnd, EditFileIOStatus* fioStatus)
{
    INT_PTR const iResult = ThemedDialogBoxParam(Globals.hLngResContainer,
                                MAKEINTRESOURCE(IDD_MUI_WARNLINEENDS),
                                hwnd,
                                WarnLineEndingDlgProc,
                                (LPARAM)fioStatus);
    return IsYesOkay(iResult);
}


//=============================================================================
//
//  WarnIndentationDlgProc()
//
static INT_PTR CALLBACK WarnIndentationDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    switch (umsg) {
    case WM_INITDIALOG: {

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { IDC_INDENT_BY_SPCS, IDC_INDENT_BY_TABS, IDC_WARN_INCONSISTENT_INDENTS,
                                IDC_STATIC, IDC_STATIC2
                              };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif

        const EditFileIOStatus* const fioStatus = (EditFileIOStatus*)lParam;

        WCHAR wch[128];
        WCHAR tchFmt[128];
        WCHAR tchCnt[64];

        StringCchPrintf(tchCnt, COUNTOF(tchCnt), L"%i", Globals.fvCurFile.iTabWidth);
        FormatNumberStr(tchCnt, COUNTOF(tchCnt), 0);
        GetDlgItemText(hwnd, IDC_INDENT_WIDTH_TAB, tchFmt, COUNTOF(tchFmt));
        StringCchPrintf(wch, COUNTOF(wch), tchFmt, tchCnt);
        SetDlgItemText(hwnd, IDC_INDENT_WIDTH_TAB, wch);

        StringCchPrintf(tchCnt, COUNTOF(tchCnt), L"%i", Globals.fvCurFile.iIndentWidth);
        FormatNumberStr(tchCnt, COUNTOF(tchCnt), 0);
        GetDlgItemText(hwnd, IDC_INDENT_WIDTH_SPC, tchFmt, COUNTOF(tchFmt));
        StringCchPrintf(wch, COUNTOF(wch), tchFmt, tchCnt);
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
        CenterDlgInParent(hwnd, NULL, false);

        AttentionBeep(MB_ICONEXCLAMATION);
    }
    return TRUE;

#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
    return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
    break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            EditFileIOStatus* fioStatus = (EditFileIOStatus*)GetWindowLongPtr(hwnd, DWLP_USER);
            fioStatus->iGlobalIndent = IsButtonChecked(hwnd, IDC_INDENT_BY_TABS) ? I_TAB_LN : I_SPC_LN;
            Settings.WarnInconsistentIndents = IsButtonChecked(hwnd, IDC_WARN_INCONSISTENT_INDENTS);
            EndDialog(hwnd, IDOK);
        }
        break;

        case IDCANCEL: {
            EditFileIOStatus* fioStatus = (EditFileIOStatus*)GetWindowLongPtr(hwnd, DWLP_USER);
            fioStatus->iGlobalIndent = I_MIX_LN;
            Settings.WarnInconsistentIndents = IsButtonChecked(hwnd, IDC_WARN_INCONSISTENT_INDENTS);
            EndDialog(hwnd, IDCANCEL);
        }
        break;
        }
        return TRUE;
    }
    return FALSE;
}


//=============================================================================
//
//  WarnIndentationDlg()
//
bool WarnIndentationDlg(HWND hwnd, EditFileIOStatus* fioStatus)
{
    INT_PTR const iResult = ThemedDialogBoxParam(Globals.hLngResContainer,
                                MAKEINTRESOURCE(IDD_MUI_WARNINDENTATION),
                                hwnd,
                                WarnIndentationDlgProc,
                                (LPARAM)fioStatus);
    return IsYesOkay(iResult);
}



//=============================================================================
//
//  AutoSaveBackupSettingsDlgProc()
//
static INT_PTR CALLBACK AutoSaveBackupSettingsDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (umsg) {
    case WM_INITDIALOG: {

        //SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hwnd, IDOK));
            SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
            SetExplorerTheme(GetDlgItem(hwnd, IDC_AS_BACKUP_OPENFOLDER));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { IDC_AUTOSAVE_ENABLE, IDC_AUTOSAVE_INTERVAL, IDC_AUTOSAVE_SUSPEND, IDC_AUTOSAVE_SHUTDOWN,
                IDC_AS_BACKUP_ENABLE, IDC_AS_BACKUP_AUTOSAVE, IDC_AS_BACKUP_SIDEBYSIDE, IDC_STATIC, IDC_STATIC2, IDC_STATIC3 };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
        }
#endif

        CheckDlgButton(hwnd, IDC_AUTOSAVE_ENABLE, SetBtn(Settings.AutoSaveOptions & ASB_Periodic));
        CheckDlgButton(hwnd, IDC_AUTOSAVE_SUSPEND, SetBtn(Settings.AutoSaveOptions & ASB_Suspend));
        CheckDlgButton(hwnd, IDC_AUTOSAVE_SHUTDOWN, SetBtn(Settings.AutoSaveOptions & ASB_Shutdown));

        DialogEnableControl(hwnd, IDC_STATIC2, IsButtonChecked(hwnd, IDC_AUTOSAVE_ENABLE));
        DialogEnableControl(hwnd, IDC_AUTOSAVE_INTERVAL, IsButtonChecked(hwnd, IDC_AUTOSAVE_ENABLE));

        CheckDlgButton(hwnd, IDC_AS_BACKUP_ENABLE, SetBtn(Settings.AutoSaveOptions & ASB_Backup));
        CheckDlgButton(hwnd, IDC_AS_BACKUP_AUTOSAVE, SetBtn(Settings.AutoSaveOptions & ASB_OnAutoSave));
        CheckDlgButton(hwnd, IDC_AS_BACKUP_SIDEBYSIDE, SetBtn(Settings.AutoSaveOptions & ASB_SideBySide));

        DialogEnableControl(hwnd, IDC_AS_BACKUP_AUTOSAVE, IsButtonChecked(hwnd, IDC_AS_BACKUP_ENABLE));
        DialogEnableControl(hwnd, IDC_AS_BACKUP_SIDEBYSIDE, IsButtonChecked(hwnd, IDC_AS_BACKUP_ENABLE));
        DialogEnableControl(hwnd, IDC_AS_BACKUP_OPENFOLDER, IsButtonChecked(hwnd, IDC_AS_BACKUP_ENABLE));

        WCHAR      wch[32];
        const UINT seconds = Settings.AutoSaveInterval / 1000;
        const UINT milliseconds = Settings.AutoSaveInterval % 1000;
        if (milliseconds) {
            StringCchPrintf(wch, COUNTOF(wch), L"%u.%03u", seconds, milliseconds);
        }
        else {
            StringCchPrintf(wch, COUNTOF(wch), L"%u", seconds);
        }
        SetDlgItemText(hwnd, IDC_AUTOSAVE_INTERVAL, wch);

        CenterDlgInParent(hwnd, NULL, false);
    }
    return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

    CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);

            int const buttons[] = { IDOK, IDCANCEL, IDC_AS_BACKUP_OPENFOLDER };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            AutoSaveBackupOptions options = ASB_None;
            options |= IsButtonChecked(hwnd, IDC_AUTOSAVE_ENABLE) ? ASB_Periodic : ASB_None;
            options |= IsButtonChecked(hwnd, IDC_AUTOSAVE_SUSPEND) ? ASB_Suspend : ASB_None;
            options |= IsButtonChecked(hwnd, IDC_AUTOSAVE_SHUTDOWN) ? ASB_Shutdown : ASB_None;

            options |= IsButtonChecked(hwnd, IDC_AS_BACKUP_ENABLE) ? ASB_Backup : ASB_None;
            options |= IsButtonChecked(hwnd, IDC_AS_BACKUP_SIDEBYSIDE) ? ASB_OnAutoSave : ASB_None;
            options |= IsButtonChecked(hwnd, IDC_AS_BACKUP_SIDEBYSIDE) ? ASB_SideBySide : ASB_None;

            Settings.AutoSaveOptions = options;

            char chInterval[32];
            GetDlgItemTextA(hwnd, IDC_AUTOSAVE_INTERVAL, chInterval, COUNTOF(chInterval));
            te_int_t iExprErr = true;
            float     interval = (float)te_interp(chInterval, &iExprErr);
            if (iExprErr) {
                WCHAR wch[32];
                GetDlgItemText(hwnd, IDC_AUTOSAVE_INTERVAL, wch, COUNTOF(wch));
                StrToFloatEx(wch, &interval);
            }
            Settings.AutoSaveInterval = clampi(f2int(interval * 1000.0f), 2000, USER_TIMER_MAXIMUM);
            EndDialog(hwnd, IDOK);
        } break;

        case IDC_AS_BACKUP_OPENFOLDER: {
            WCHAR szTitle[SMALL_BUFFER] = { L'\0' };
            GetLngString(IDS_MUI_FAVORITES, szTitle, COUNTOF(szTitle));
            HPATHL hdir_pth = Path_Allocate(NULL);
            if (Path_BrowseDirectory(hwnd, szTitle, hdir_pth, Paths.WorkingDirectory, true)) {
                InfoBoxLng(MB_ICONINFORMATION, NULL, IDS_MUI_LOADFILE, Path_Get(hdir_pth));
                // change dir
            }
            //if (GetFolderDlg(Globals.hwndMain, hdir_pth, Paths.WorkingDirectory)) {
            //    InfoBoxLng(MB_ICONINFORMATION, NULL, IDS_MUI_LOADFILE, Path_Get(hdir_pth));
            //}
            Path_Release(hdir_pth);
        } break;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDC_AUTOSAVE_ENABLE:
            DialogEnableControl(hwnd, IDC_STATIC2, IsButtonChecked(hwnd, IDC_AUTOSAVE_ENABLE));
            DialogEnableControl(hwnd, IDC_AUTOSAVE_INTERVAL, IsButtonChecked(hwnd, IDC_AUTOSAVE_ENABLE));
            break;

        case IDC_AS_BACKUP_ENABLE:
            DialogEnableControl(hwnd, IDC_AS_BACKUP_AUTOSAVE, IsButtonChecked(hwnd, IDC_AS_BACKUP_ENABLE));
            DialogEnableControl(hwnd, IDC_AS_BACKUP_SIDEBYSIDE, IsButtonChecked(hwnd, IDC_AS_BACKUP_ENABLE));
            DialogEnableControl(hwnd, IDC_AS_BACKUP_OPENFOLDER, IsButtonChecked(hwnd, IDC_AS_BACKUP_ENABLE));
            break;

        default:
            return FALSE;
        }
        return TRUE;

    }
    return FALSE;
}


bool AutoSaveBackupSettingsDlg(HWND hwnd)
{
    INT_PTR const iResult = ThemedDialogBoxParam(Globals.hLngResContainer,
                                MAKEINTRESOURCE(IDD_MUI_AUTOSAVE_BACKUP),
                                hwnd, 
                                AutoSaveBackupSettingsDlgProc,
                                0);

    return IsYesOkay(iResult);
}


//=============================================================================
//
//  RelAdjustRectForDPI()
//
void RelAdjustRectForDPI(LPRECT rc, const UINT oldDPI, const UINT newDPI) {
    if (oldDPI == newDPI) { return; }
    float const scale = (float)newDPI / (float)(oldDPI != 0 ? oldDPI : 1);
    LONG const oldWidth = (rc->right - rc->left);
    LONG const oldHeight = (rc->bottom - rc->top);
    LONG const newWidth = lroundf((float)oldWidth * scale);
    LONG const newHeight = lroundf((float)oldHeight * scale);
    rc->left -= (newWidth - oldWidth) >> 1;
    rc->right = rc->left + newWidth;
    rc->top -= (newHeight - oldHeight) >> 1;
    rc->bottom = rc->top + newHeight;
}


//=============================================================================
//
//  MapRectClientToWndCoords()
//
void MapRectClientToWndCoords(HWND hwnd, LPRECT rc)
{
    // map to screen (left-top as point)
    MapWindowPoints(hwnd, NULL, (POINT*)rc, 2);

    RECT scrc;
    GetWindowRect(hwnd, &scrc);

    // map to window coords by substracting the window coord origin in screen coords.
    OffsetRect(rc, -scrc.left, -scrc.top);
}


//=============================================================================
//
//  GetMonitorInfoFromRect()
//
bool GetMonitorInfoFromRect(const LPRECT rc, MONITORINFO *hMonitorInfo) {

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
        } else {
            result = true;
        }
    }
    return result;
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  WinInfoToScreenCoord()
//
void WinInfoToScreenCoord(WININFO *pWinInfo) {
    if (pWinInfo) {
        MONITORINFO mi = { sizeof(MONITORINFO) };
        RECT rc = { 0 };
        RectFromWinInfo(pWinInfo, &rc);
        if (GetMonitorInfoFromRect(&rc, &mi)) {
            pWinInfo->x += (mi.rcWork.left - mi.rcMonitor.left);
            pWinInfo->y += (mi.rcWork.top - mi.rcMonitor.top);
        }
    }
}


//=============================================================================
//
//  GetWindowRectEx()
//
bool GetWindowRectEx(HWND hwnd, LPRECT pRect) {

    bool bMainWndTray = false;
    if (Globals.hwndMain == hwnd) {
        bMainWndTray = Settings.MinimizeToTray && Globals.bMinimizedToTray;
    }
    bool const res = bMainWndTray ? GetTrayWndRect(pRect) : GetWindowRect(hwnd, pRect);

    WINDOWPLACEMENT wndpl = { sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(hwnd, &wndpl);

    switch (wndpl.showCmd) {
    case SW_HIDE:
    case SW_SHOWMINIMIZED:
        if (res) {
            POINT pt = { 0 };
            GetCursorPos(&pt);
            pRect->left = pRect->right = pt.x;
            pRect->top = pRect->bottom = pt.y;
            return res;
        }
        break;

    case SW_SHOWMAXIMIZED:
    case SW_MAX: {
        MONITORINFO mi = { sizeof(MONITORINFO) };
        GetMonitorInfoFromRect(pRect, &mi);
        *pRect = mi.rcWork;
    } break;

    default:
        break;
    }
    return res;
}


//=============================================================================
//
//  FitIntoMonitorGeometry()
//
void FitIntoMonitorGeometry(LPRECT pRect, WININFO *pWinInfo, SCREEN_MODE mode, bool bTopLeft) {

    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfoFromRect(pRect, &mi);

    if (mode == SCR_FULL_SCREEN) {
        SetRect(pRect, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
        // monitor coord -> screen coord
        pWinInfo->x = mi.rcMonitor.left - (mi.rcWork.left - mi.rcMonitor.left);
        pWinInfo->y = mi.rcMonitor.top - (mi.rcWork.top - mi.rcMonitor.top);
        pWinInfo->cx = (mi.rcMonitor.right - mi.rcMonitor.left);
        pWinInfo->cy = (mi.rcMonitor.bottom - mi.rcMonitor.top);
        pWinInfo->max = true;
        //~pWinInfo->dpi = Scintilla_GetWindowDPI(hwnd); // don't change
    } else {
        WININFO wi = *pWinInfo;
        WinInfoToScreenCoord(&wi);
        // fit into area
        if (wi.x < mi.rcWork.left) {
            wi.x = mi.rcWork.left;
        }
        if (wi.y < mi.rcWork.top) {
            wi.y = mi.rcWork.top;
        }
        if (bTopLeft && (((wi.x + wi.cx) > mi.rcWork.right) || 
                         ((wi.y + wi.cy) > mi.rcWork.bottom))) {
            wi.y = mi.rcWork.top;
            wi.x = mi.rcWork.left;
        } else {
            if ((wi.x + wi.cx) > mi.rcWork.right) {
                wi.x -= (wi.x + wi.cx - mi.rcWork.right);
                if (wi.x < mi.rcWork.left) {
                    wi.x = mi.rcWork.left;
                }
                if ((wi.x + wi.cx) > mi.rcWork.right) {
                    wi.cx = mi.rcWork.right - wi.x;
                }
            }
            if ((wi.y + wi.cy) > mi.rcWork.bottom) {
                wi.y -= (wi.y + wi.cy - mi.rcWork.bottom);
                if (wi.y < mi.rcWork.top) {
                    wi.y = mi.rcWork.top;
                }
                if ((wi.y + wi.cy) > mi.rcWork.bottom) {
                    wi.cy = mi.rcWork.bottom - wi.y;
                }
            }
        }
        RectFromWinInfo(&wi, pRect);
        // screen coord -> work area coord
        pWinInfo->x = wi.x - (mi.rcWork.left - mi.rcMonitor.left);
        pWinInfo->y = wi.y - (mi.rcWork.top - mi.rcMonitor.top);
        pWinInfo->cx = wi.cx;
        pWinInfo->cy = wi.cy;
        //~pWinInfo->dpi = Scintilla_GetWindowDPI(hwnd); // don't change
    }
}
// ----------------------------------------------------------------------------


//=============================================================================
//
//  GetMyWindowPlacement()
//
WININFO GetMyWindowPlacement(HWND hwnd, MONITORINFO *hMonitorInfo, const int offset, const bool bFullVisible) {
    RECT rc;
    GetWindowRect(hwnd, &rc);

    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfoFromRect(&rc, &mi);

    // set monitor info
    if (hMonitorInfo) {
        if (hMonitorInfo->cbSize == mi.cbSize) {
            *hMonitorInfo = mi;
        } else {
            GetMonitorInfoFromRect(&rc, hMonitorInfo);
        }
    }

    WINDOWPLACEMENT wndpl = { sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(hwnd, &wndpl);

    // corrections in case of aero snapped position
    if (SW_SHOWNORMAL == wndpl.showCmd) {
        LONG const width = rc.right - rc.left;
        LONG const height = rc.bottom - rc.top;
        rc.left -= (mi.rcWork.left - mi.rcMonitor.left);
        rc.right = rc.left + width;
        rc.top -= (mi.rcWork.top - mi.rcMonitor.top);
        rc.bottom = rc.top + height;
        wndpl.rcNormalPosition = rc;
    }

    WININFO wi = { 0 };
    wi.x = wndpl.rcNormalPosition.left + offset;
    wi.y = wndpl.rcNormalPosition.top + offset;
    wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
    wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
    wi.max = (hwnd ? IsZoomed(hwnd) : false) || (wndpl.flags & WPF_RESTORETOMAXIMIZED);
    wi.zoom = hwnd ? SciCall_GetZoom() : 100;
    wi.dpi = Scintilla_GetWindowDPI(hwnd);

    if (bFullVisible) {
        RECT rci;
        RectFromWinInfo(&wi, &rci);
        FitIntoMonitorGeometry(&rci, &wi, SCR_NORMAL, false);
    }
    return wi;
}


//=============================================================================
//
//  WindowPlacementFromInfo()
//
//
WINDOWPLACEMENT WindowPlacementFromInfo(HWND hwnd, const WININFO* pWinInfo, SCREEN_MODE mode, UINT nCmdShow)
{
    WINDOWPLACEMENT wndpl = {0};
    wndpl.length = sizeof(WINDOWPLACEMENT);
    wndpl.flags = WPF_ASYNCWINDOWPLACEMENT;

    WININFO winfo = INIT_WININFO;
    if (pWinInfo) {
        winfo = *pWinInfo;
        if (Settings2.LaunchInstanceFullVisible) {
            RECT rc = { 0 };
            RectFromWinInfo(pWinInfo, &rc);
            FitIntoMonitorGeometry(&rc, &winfo, mode, false);
        }
        if (pWinInfo->max) {
            wndpl.flags &= WPF_RESTORETOMAXIMIZED;
        }
    } else {
        RECT rc = { 0 };
        if (hwnd) {
            GetWindowRect(hwnd, &rc);
        }
        else {
            GetWindowRect(GetDesktopWindow(), &rc);
        }
        if (Settings2.LaunchInstanceFullVisible) {
            FitIntoMonitorGeometry(&rc, &winfo, mode, false);
        } else {
            WinInfoFromRect(&rc, &winfo);
        }
    }
    wndpl.showCmd = nCmdShow;
    RectFromWinInfo(&winfo, &(wndpl.rcNormalPosition));
    return wndpl;
}

//=============================================================================
//
//  SnapToWinInfoPos()
//  Aligns Notepad3 to the given window position on the screen
//
static bool s_bPrevFullScreenFlag = false;

void SnapToWinInfoPos(HWND hwnd, const WININFO winInfo, SCREEN_MODE mode, UINT nCmdShow)
{
    if (!hwnd) {
        return;
    }
    static bool            s_bPrevShowTitlebar = true;
    static bool            s_bPrevShowMenubar = true;
    static bool            s_bPrevShowToolbar = true;
    static bool            s_bPrevShowStatusbar = true;
    static bool            s_bPrevAlwaysOnTop = false;
    static WINDOWPLACEMENT s_wndplPrev = { 0 };
    s_wndplPrev.length = sizeof(WINDOWPLACEMENT);

    DWORD const dwRmvFScrStyle = WS_OVERLAPPEDWINDOW | WS_BORDER;

    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    RECT  rcCurrent;
    GetWindowRect(hwnd, &rcCurrent);

    if ((mode == SCR_NORMAL) || s_bPrevFullScreenFlag) {
        SetWindowLong(hwnd, GWL_STYLE, dwStyle | dwRmvFScrStyle);
        if (s_bPrevFullScreenFlag) {
            SetWindowPlacement(hwnd, &s_wndplPrev); // 1st set correct screen (DPI Aware)
            SetWindowPlacement(hwnd, &s_wndplPrev); // 2nd resize position to correct DPI settings
            Settings.ShowTitlebar = s_bPrevShowTitlebar;
            Settings.ShowMenubar = s_bPrevShowMenubar;
            Settings.ShowToolbar = s_bPrevShowToolbar;
            Settings.ShowStatusbar = s_bPrevShowStatusbar;
            Settings.AlwaysOnTop = s_bPrevAlwaysOnTop;
        }
        else {
            WINDOWPLACEMENT wndpl = WindowPlacementFromInfo(hwnd, &winInfo, mode, nCmdShow);
            if (HasDrawAnimation() && wndpl.showCmd) {
                DrawAnimatedRects(hwnd, IDANI_CAPTION, &rcCurrent, &wndpl.rcNormalPosition);
            } else {
                ShowWindow(hwnd, SW_HIDE);
            }
            SetWindowPlacement(hwnd, &wndpl); // 1st set correct screen (DPI Aware)
            RelAdjustRectForDPI(&wndpl.rcNormalPosition, winInfo.dpi, Scintilla_GetWindowDPI(hwnd));
            SetWindowPlacement(hwnd, &wndpl); // 2nd resize position to correct DPI settings
            ShowWindow(hwnd, wndpl.showCmd);
        }
        SetWindowPos(hwnd, Settings.AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        s_bPrevFullScreenFlag = false;
    }
    else { // full screen mode
        s_bPrevShowTitlebar = Settings.ShowTitlebar;
        s_bPrevShowMenubar = Settings.ShowMenubar;
        s_bPrevShowToolbar = Settings.ShowToolbar;
        s_bPrevShowStatusbar = Settings.ShowStatusbar;
        s_bPrevAlwaysOnTop = Settings.AlwaysOnTop;
        GetWindowPlacement(hwnd, &s_wndplPrev);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
        SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~dwRmvFScrStyle);
        WINDOWPLACEMENT const wndpl = WindowPlacementFromInfo(hwnd, NULL, mode, nCmdShow);
        if (HasDrawAnimation() && wndpl.showCmd) {
            DrawAnimatedRects(hwnd, IDANI_CAPTION, &rcCurrent, &wndpl.rcNormalPosition);
        } else {
            ShowWindow(hwnd, SW_HIDE);
        }
        SetWindowPlacement(hwnd, &wndpl);
        SetWindowPos(hwnd, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
            mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        ShowWindow(hwnd, wndpl.showCmd);
        Settings.ShowTitlebar = Settings.ShowMenubar = Settings.ShowToolbar = Settings.ShowStatusbar = false;
        Settings.AlwaysOnTop = true;
        s_bPrevFullScreenFlag = true;
    }
    SetMenu(hwnd, (Settings.ShowMenubar ? Globals.hMainMenu : NULL));
    DrawMenuBar(hwnd);
    ShowWindow(Globals.hwndRebar, (Settings.ShowToolbar ? SW_RESTORE : SW_HIDE));

    UpdateUI(hwnd);
}


//=============================================================================
//
//  RestorePrevScreenPos()
//
void RestorePrevScreenPos(HWND hwnd)
{
    if (hwnd == Globals.hwndMain) {
        if (s_bPrevFullScreenFlag) {
            SendWMCommand(hwnd, CMD_FULLSCRWINPOS);
        }
    }
}


//=============================================================================
//
//  DialogNewWindow()
//
void DialogNewWindow(HWND hwnd, bool bSaveBeforeOpen, const HPATHL hFilePath, WININFO* wi)
{
    if (bSaveBeforeOpen && !FileSave(FSF_Ask)) {
        return;
    }
    WCHAR wch[80] = { L'\0' };

    HPATHL hmod_pth = Path_Allocate(NULL);
    Path_GetModuleFilePath(hmod_pth);

    StringCchPrintf(wch, COUNTOF(wch), L"\"-appid=%s\"", Settings2.AppUserModelID);
    HSTRINGW hparam_str = StrgCreate(wch);
    StringCchPrintf(wch, COUNTOF(wch), L"\" -sysmru=%i\"", (Flags.ShellUseSystemMRU ? 1 : 0));
    StrgCat(hparam_str, wch);
    if (Path_IsNotEmpty(Paths.IniFile)) {
        HPATHL hini_path = Path_Copy(Paths.IniFile);
        Path_QuoteSpaces(hini_path, true);
        StrgCat(hparam_str, L" -f ");
        StrgCat(hparam_str, Path_Get(hini_path));
        Path_Release(hini_path);
    } else {
        StrgCat(hparam_str, L" -f0");
    }
    StrgCat(hparam_str, Flags.bSingleFileInstance ? L" -ns" : L" -n");

    WININFO const _wi = (Flags.bStickyWindowPosition ? g_IniWinInfo : 
                         (wi ? *wi : GetMyWindowPlacement(hwnd, NULL, Settings2.LaunchInstanceWndPosOffset, Settings2.LaunchInstanceFullVisible)));

    StringCchPrintf(wch, COUNTOF(wch), L" -pos " WINDOWPOS_STRGFORMAT, _wi.x, _wi.y, _wi.cx, _wi.cy, _wi.dpi, (int)_wi.max);
    StrgCat(hparam_str, wch);

    if (Path_IsNotEmpty(hFilePath)) {
        HPATHL hfile_pth = Path_Copy(hFilePath);
        Path_QuoteSpaces(hfile_pth, true);
        StrgCat(hparam_str, L" ");
        StrgCat(hparam_str, Path_Get(hfile_pth));
        Path_Release(hfile_pth);
    }

    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOZONECHECKS;
    sei.hwnd = hwnd;
    sei.lpVerb = NULL;
    sei.lpFile = Path_Get(hmod_pth);
    sei.lpParameters = StrgGet(hparam_str);
    sei.lpDirectory = Path_Get(Paths.WorkingDirectory);
    sei.nShow = SW_SHOWNORMAL;
    ShellExecuteExW(&sei);

    StrgDestroy(hparam_str);
    Path_Release(hmod_pth);
}


//=============================================================================
//
//  DialogFileBrowse()
//
//
void DialogFileBrowse(HWND hwnd)
{
    wchar_t* const param_buf = AllocMem((PATHLONG_MAX_CCH + 1) * sizeof(wchar_t), HEAP_ZERO_MEMORY);
    if (!param_buf) {
        return;
    }

    HPATHL hExeFile = Path_Allocate(NULL);
    wchar_t* const pth_buf = Path_WriteAccessBuf(hExeFile, PATHLONG_MAX_CCH);

    if (Path_IsNotEmpty(Settings2.FileBrowserPath)) {
        ExtractFirstArgument(Path_Get(Settings2.FileBrowserPath), pth_buf, param_buf, PATHLONG_MAX_CCH);
        Path_Sanitize(hExeFile);
        Path_ExpandEnvStrings(hExeFile);
    }

    if (StrStrIW(Path_Get(hExeFile), L"explorer.exe") && StrIsEmpty(param_buf)) {
        SendWMCommand(hwnd, IDM_FILE_EXPLORE_DIR);
        Path_Release(hExeFile);
        FreeMem(param_buf);
        return;
    }

    if (Path_IsEmpty(hExeFile)) {
        Path_Reset(hExeFile, Constants.FileBrowserMiniPath);
    }

    if (Path_IsRelative(hExeFile)) {

        HPATHL hTemp = Path_Copy(Paths.ModuleDirectory);
        Path_Append(hTemp, Path_Get(hExeFile));
        if (Path_IsExistingFile(hTemp)) {
            Path_Swap(hExeFile, hTemp);
        }
        Path_Release(hTemp);
    }

    if (Path_IsNotEmpty(Paths.CurrentFile)) {
        HPATHL pthTmp = Path_Copy(Paths.CurrentFile);
        Path_QuoteSpaces(pthTmp, true);
        StringCchCat(param_buf, PATHLONG_MAX_CCH, Path_Get(pthTmp));
        Path_Release(pthTmp);
    }

    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOZONECHECKS;
    sei.hwnd = hwnd;
    sei.lpVerb = NULL;
    sei.lpFile = Path_Get(hExeFile);
    sei.lpParameters = param_buf;
    sei.lpDirectory = NULL;
    sei.nShow = SW_SHOWNORMAL;
    ShellExecuteExW(&sei);

    if ((INT_PTR)sei.hInstApp < 32) {
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_BROWSE);
    }

    Path_Release(hExeFile);
    FreeMem(param_buf);
}


//=============================================================================
//
//  DialogGrepWin() - Prerequisites
//
//

typedef struct _grepwin_ini {
    const WCHAR* const key;
    const WCHAR* const val;
}
grepWin_t;

static grepWin_t grepWinIniSettings[] = {
    { L"onlyone",           L"1" },
    { L"AllSize",           L"0" },
    { L"Size",           L"2000" },
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
    HPATHL hGrepWinIniPath = Path_Copy(Paths.IniFile);  // side-by-side
    Path_CanonicalizeEx(hGrepWinIniPath, Paths.ModuleDirectory);

    HPATHL hExeFilePath = Path_Allocate(Path_Get(Settings2.GrepWinPath));
    Path_CanonicalizeEx(hExeFilePath, Paths.ModuleDirectory);
    wchar_t* const exe_pth_buf = Path_WriteAccessBuf(hExeFilePath, PATHLONG_MAX_CCH);

    HSTRINGW hstrOptions = StrgCreate(NULL);
    wchar_t* const options_buf = StrgWriteAccessBuf(hstrOptions, PATHLONG_MAX_CCH);

    // find grepWin executable
    if (Path_IsNotEmpty(hExeFilePath)) {
        ExtractFirstArgument(Path_Get(Settings2.GrepWinPath), exe_pth_buf, options_buf, PATHLONG_MAX_CCH);
        StrgSanitize(hstrOptions);
        Path_Sanitize(hExeFilePath);
    }
    if (Path_IsEmpty(hExeFilePath)) {
        Path_Reset(hExeFilePath, Constants.FileSearchGrepWin);
        Path_CanonicalizeEx(hExeFilePath, Paths.ModuleDirectory);
    }
    if (Path_IsRelative(hExeFilePath)) {
        Path_AbsoluteFromApp(hExeFilePath, false);
    }

    // working (grepWinNP3.ini) directory
    HPATHL hTemp = Path_Allocate(NULL);
    HPATHL hGrepWinDir = Path_Allocate(Path_Get(hExeFilePath));
    Path_RemoveFileSpec(hGrepWinDir);

    if (Path_IsExistingFile(hExeFilePath)) {

        // path to grepWin INI-File
        if (Path_IsEmpty(hGrepWinIniPath)) {
            Path_Reset(hGrepWinIniPath, Path_Get(Paths.IniFileDefault));
            Path_CanonicalizeEx(hGrepWinIniPath, Paths.ModuleDirectory);
        }
        Path_RemoveFileSpec(hGrepWinIniPath);

        LPCWSTR const wchIniFileName = L"grepWinNP3.ini";
        Path_Append(hGrepWinIniPath, wchIniFileName);
        if (Path_IsRelative(hGrepWinIniPath)) {
            Path_Reset(hGrepWinIniPath, Path_Get(hGrepWinDir));
            Path_Append(hGrepWinIniPath, wchIniFileName);
        }

        // create/modify grepWin INI-File
        ResetIniFileCache();
        if (CreateIniFile(hGrepWinIniPath, NULL) && LoadIniFileCache(hGrepWinIniPath)) {

            // preserve [global] user settings from last call
            const WCHAR* const globalSection = L"global";

            WCHAR value[HUGE_BUFFER];
            for (int i = 0; i < COUNTOF(grepWinIniSettings); ++i) {
                IniSectionGetString(globalSection, grepWinIniSettings[i].key, grepWinIniSettings[i].val, value, COUNTOF(value));
                IniSectionSetString(globalSection, grepWinIniSettings[i].key, value);
            }

            // get grepWin language
            int lngIdx = -1;
            for (unsigned i = 0; i < grepWinLang_CountOf(); ++i) {
                if (IsSameLocale(grepWinLangResName[i].localename, Globals.CurrentLngLocaleName)) {
                    lngIdx = i;
                    break;
                }
            }

            HPATHL hLngFilePath = Path_Allocate(NULL);
            LPWSTR wchLngPathBuf = Path_WriteAccessBuf(hLngFilePath, PATHLONG_MAX_CCH);

            if (lngIdx >= 0) {
                IniSectionGetString(globalSection, L"languagefile", grepWinLangResName[lngIdx].filename, wchLngPathBuf, Path_GetBufCount(hLngFilePath));
                IniSectionSetString(globalSection, L"languagefile", wchLngPathBuf);
            } else {
                IniSectionGetString(globalSection, L"languagefile", L"", wchLngPathBuf, Path_GetBufCount(hLngFilePath));
                if (Path_IsEmpty(hLngFilePath)) {
                    IniSectionDelete(globalSection, L"languagefile", false);
                }
            }

            bool const bDarkMode = UseDarkMode(); // <- override usr ~ IniSectionGetBool(globalSection, L"darkmode", UseDarkMode());
            IniSectionSetBool(globalSection, L"darkmode", bDarkMode);

            // Notepad3 path (for grepWin's EditorCmd)
            HPATHL hpath_np3 = Path_Allocate(NULL);
            Path_GetModuleFilePath(hpath_np3);

            StringCchPrintf(wchLngPathBuf, Path_GetBufCount(hLngFilePath), L"%s /%%mode%% \"%%pattern%%\" /g %%line%% - %%path%%", Path_Get(hpath_np3));
            IniSectionSetString(globalSection, L"editorcmd", wchLngPathBuf);

            Path_Release(hpath_np3);
            Path_Release(hLngFilePath);

            long const iOpacity = IniSectionGetLong(globalSection, L"OpacityNoFocus", Settings2.FindReplaceOpacityLevel);
            IniSectionSetLong(globalSection, L"OpacityNoFocus", iOpacity);

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
            HPATHL pthSearchDir = NULL;
            if (Path_IsNotEmpty(Paths.CurrentFile)) {
                pthSearchDir = Path_Copy(Paths.CurrentFile);
                Path_RemoveFileSpec(pthSearchDir);
            }
            else {
                pthSearchDir = Path_Copy(Paths.WorkingDirectory);
            }
            IniSectionSetString(globalSection, L"searchpath", Path_Get(pthSearchDir));
            Path_Release(pthSearchDir);

            // search pattern
            IniSectionSetString(globalSection, L"searchfor", searchPattern);

            SaveIniFileCache(hGrepWinIniPath);
            ResetIniFileCache();
        }
    }

    // grepWin arguments
    HSTRINGW hstrParams = StrgCreate(NULL);
    if (Path_IsExistingFile(hGrepWinIniPath)) {
        StrgFormat(hstrParams, L"/portable /content %s /inipath:\"%s\"", StrgGet(hstrOptions), Path_Get(hGrepWinIniPath));
    } else {
        StrgFormat(hstrParams, L"/portable /content %s", StrgGet(hstrOptions));
    }
    //if (StrIsNotEmpty(searchPattern)) {
    //  SetClipboardText(Globals.hwndMain, searchPattern, StringCchLen(searchPattern, 0));
    //}
    StrgDestroy(hstrOptions);

    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOZONECHECKS;
    sei.hwnd = hwnd;
    sei.lpVerb = NULL;
    sei.lpFile = Path_Get(hExeFilePath);
    sei.lpParameters = StrgGet(hstrParams);
    sei.lpDirectory = Path_Get(hGrepWinDir);
    sei.nShow = SW_SHOWNORMAL;
    ShellExecuteExW(&sei);

    if ((INT_PTR)sei.hInstApp < 32) {
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_ERR_GREPWIN);
    }

    StrgDestroy(hstrParams);

    Path_Release(hGrepWinIniPath);
    Path_Release(hGrepWinDir);
    Path_Release(hTemp);
    Path_Release(hExeFilePath);
}


//=============================================================================
//
//  DialogAdminExe()
//
//
void DialogAdminExe(HWND hwnd, bool bExecInstaller)
{
    if (bExecInstaller && Path_IsEmpty(Settings2.AdministrationTool)) {
        return;
    }

    HPATHL hexe_pth = Path_Allocate(NULL);
    wchar_t* const exe_buf = Path_WriteAccessBuf(hexe_pth, PATHLONG_MAX_CCH);
    if (!SearchPathW(NULL, Path_Get(Settings2.AdministrationTool), L".exe", PATHLONG_MAX_CCH, exe_buf, NULL)) {
        // try Notepad3's dir path
        Path_GetAppDirectory(hexe_pth);
        Path_Append(hexe_pth, Path_Get(Settings2.AdministrationTool));
    }
    Path_Sanitize(hexe_pth);

    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOZONECHECKS;
    sei.hwnd = hwnd;
    sei.lpVerb = NULL;
    sei.lpFile = Path_Get(hexe_pth);
    sei.lpParameters = NULL; // tchParam;
    sei.lpDirectory = Path_Get(Paths.WorkingDirectory);
    sei.nShow = SW_SHOWNORMAL;
    if (bExecInstaller) {
        ShellExecuteExW(&sei);
        if (IsYesOkay(InfoBoxLng(MB_OKCANCEL, L"NoAdminTool", IDS_MUI_ERR_ADMINEXE))) {
            sei.lpFile = VERSION_UPDATE_CHECK;
            ShellExecuteExW(&sei);
        }
    } else {
        sei.lpFile = VERSION_UPDATE_CHECK;
        ShellExecuteExW(&sei);
    }

    Path_Release(hexe_pth);
}

// ============================================================================
// some Helpers
// ============================================================================


//=============================================================================
//
//  SetWindowTitle()
//
bool s_bFreezeAppTitle = false; // extern visible

static HSTRINGW s_wchAdditionalTitleInfo = NULL;

void SetAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo) {
    if (!s_wchAdditionalTitleInfo) {
        s_wchAdditionalTitleInfo = StrgCreate(lpszAddTitleInfo);
    }
    else {
        StrgReset(s_wchAdditionalTitleInfo, lpszAddTitleInfo);
    }
}

void AppendAdditionalTitleInfo(LPCWSTR lpszAddTitleInfo) {
    StrgCat(s_wchAdditionalTitleInfo, lpszAddTitleInfo);
}

static WCHAR        s_szUntitled[80] = { L'\0' };
static const WCHAR* pszMod = DOCMODDIFYD;
static const WCHAR* pszSep = L" - ";

static WCHAR  s_wchCachedDisplayName[80] = { L'\0' };
static HPATHL s_pthCachedFilePath = NULL;

// ----------------------------------------------------------------------------

void SetWindowTitle(HWND hwnd, const HPATHL pthFilePath, const TITLEPROPS_T properties, LPCWSTR lpszExcerpt, bool forceRedraw)
{
    static TITLEPROPS_T s_properties = { 0 };
    static size_t       s_hashFileName = 0;
    static size_t       s_hashExcerpt = 0;

    if (s_bFreezeAppTitle) {
        return;
    }

    if (!forceRedraw) {
        if (s_properties.iFormat != properties.iFormat) {
            forceRedraw = true;
        }
        else if (s_properties.bPasteBoard != properties.bPasteBoard) {
            forceRedraw = true;
        }
        else if (s_properties.bIsElevated != properties.bIsElevated) {
            forceRedraw = true;
        }
        else if (s_properties.bModified != properties.bModified) {
            forceRedraw = true;
        }
        else if (s_properties.bFileLocked != properties.bFileLocked) {
            forceRedraw = true;
        }
        else if (s_properties.bFileChanged != properties.bFileChanged) {
            forceRedraw = true;
        }
        else if (s_properties.bFileDeleted != properties.bFileDeleted) {
            forceRedraw = true;
        }
        else if (s_properties.bReadOnly != properties.bReadOnly) {
            forceRedraw = true;
        }
        else {
            size_t const hashExcerpt = SimpleHash(lpszExcerpt);
            if (s_hashExcerpt != hashExcerpt) {
                forceRedraw = true;
            }
            else {
                size_t const hashFileName = SimpleHash(Path_Get(pthFilePath));
                if (s_hashFileName != hashFileName) {
                    forceRedraw = true;
                }
            }
        }
    }

    if (!forceRedraw) {
        return;
    }

    // save current state
    s_properties = properties;
    s_hashExcerpt = SimpleHash(lpszExcerpt);
    s_hashFileName = SimpleHash(Path_Get(pthFilePath));

    if (!s_pthCachedFilePath) {
        s_pthCachedFilePath = Path_Allocate(L"");
    }

    WCHAR szAppName[SMALL_BUFFER] = { L'\0' };
    if (properties.bPasteBoard) {
        FormatLngStringW(szAppName, COUNTOF(szAppName), IDS_MUI_APPTITLE_PASTEBOARD, _W(SAPPNAME));
    }
    else if (properties.bIsElevated) {
        WCHAR szElevatedAppName[SMALL_BUFFER] = { L'\0' };
        FormatLngStringW(szElevatedAppName, COUNTOF(szElevatedAppName), IDS_MUI_APPTITLE_ELEVATED, _W(SAPPNAME));
        StringCchCopy(szAppName, COUNTOF(szAppName), szElevatedAppName);
    }
    else {
        StringCchCopy(szAppName, COUNTOF(szAppName), _W(SAPPNAME));
    }

    if (StrIsEmpty(s_szUntitled)) {
        GetLngString(IDS_MUI_UNTITLED, s_szUntitled, COUNTOF(s_szUntitled));
    }

    WCHAR szTitle[MIDSZ_BUFFER] = { L'\0' };

    if (properties.bModified) {
        StringCchCat(szTitle, COUNTOF(szTitle), pszMod);
    }
    if (properties.bFileChanged) {
        if (properties.bFileDeleted) {
            StringCchCatN(szTitle, COUNTOF(szTitle), Settings2.FileDeletedIndicator, 3);
        }
        else {
            StringCchCatN(szTitle, COUNTOF(szTitle), Settings2.FileChangedIndicator, 3);
        }
        StringCchCat(szTitle, COUNTOF(szTitle), L" ");
    }
    if (StrIsNotEmpty(lpszExcerpt)) {
        WCHAR szExcrptFmt[32] = { L'\0' };
        WCHAR szExcrptQuot[SMALL_BUFFER] = { L'\0' };
        GetLngString(IDS_MUI_TITLEEXCERPT, szExcrptFmt, COUNTOF(szExcrptFmt));
        StringCchPrintf(szExcrptQuot, COUNTOF(szExcrptQuot), szExcrptFmt, lpszExcerpt);
        StringCchCat(szTitle, COUNTOF(szTitle), szExcrptQuot);
    }
    else if (Path_IsNotEmpty(pthFilePath)) {

        if (properties.iFormat < 2) {
            if (Path_StrgComparePath(s_pthCachedFilePath, pthFilePath, Paths.WorkingDirectory) != 0) {
                Path_Reset(s_pthCachedFilePath, Path_Get(pthFilePath));
                Path_GetDisplayName(s_wchCachedDisplayName, COUNTOF(s_wchCachedDisplayName), s_pthCachedFilePath, s_szUntitled, true);
            }
            StringCchCat(szTitle, COUNTOF(szTitle), Path_FindFileName(s_pthCachedFilePath));
            if (properties.iFormat == 1) {
                HPATHL hdir = Path_Copy(s_pthCachedFilePath);
                if (Path_IsNotEmpty(hdir)) {
                    Path_RemoveFileSpec(hdir);
                }
                StringCchCat(szTitle, COUNTOF(szTitle), L" [");
                StringCchCat(szTitle, COUNTOF(szTitle), Path_Get(hdir));
                StringCchCat(szTitle, COUNTOF(szTitle), L"]");
                Path_Release(hdir);
            }
        }
        else {
            StringCchCat(szTitle, COUNTOF(szTitle), Path_Get(pthFilePath));
        }
    }
    else {
        Path_Empty(s_pthCachedFilePath, false);
        s_wchCachedDisplayName[0] = L'\0';
        StringCchCat(szTitle, COUNTOF(szTitle), s_szUntitled);
    }

    WCHAR wchModeEx[64] = { L'\0' };
    if (properties.bFileLocked) {
        GetLngString(IDS_MUI_FILELOCKED, wchModeEx, COUNTOF(wchModeEx));
        StringCchCat(szTitle, COUNTOF(szTitle), L" ");
        StringCchCat(szTitle, COUNTOF(szTitle), wchModeEx);
    }
    if (properties.bReadOnly) {
        GetLngString(IDS_MUI_READONLY, wchModeEx, COUNTOF(wchModeEx));
        StringCchCat(szTitle, COUNTOF(szTitle), L" ");
        StringCchCat(szTitle, COUNTOF(szTitle), wchModeEx);
    }

    StringCchCat(szTitle, COUNTOF(szTitle), pszSep);
    StringCchCat(szTitle, COUNTOF(szTitle), szAppName);

    // UCHARDET
    if (StrgIsNotEmpty(s_wchAdditionalTitleInfo)) {
        StringCchCat(szTitle, COUNTOF(szTitle), pszSep);
        StringCchCat(szTitle, COUNTOF(szTitle), StrgGet(s_wchAdditionalTitleInfo));
    }

    SetWindowText(hwnd, szTitle);
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
    } else {
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
    } else {
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
    } else {
        SetWindowExStyle(hwnd, exStyle & ~WS_EX_RTLREADING);
    }
}


//=============================================================================
//
//  A2W: Convert Dialog Item Text form Unicode to UTF-8 and vice versa
//

UINT ComboBox_GetTextLengthEx(HWND hDlg, int nIDDlgItem)
{
    return (UINT)ComboBox_GetTextLength(GetDlgItem(hDlg, nIDDlgItem));
}

UINT ComboBox_GetCurSelEx(HWND hDlg, int nIDDlgItem) {
    return (UINT)ComboBox_GetCurSel(GetDlgItem(hDlg, nIDDlgItem));
}

int ComboBox_GetTextHW(HWND hDlg, int nIDDlgItem, HSTRINGW hstr)
{
    HWND const   hwndCtl = GetDlgItem(hDlg, nIDDlgItem);
    int const    idx = ComboBox_GetCurSel(hwndCtl);
    int const    len = ((idx >= 0) ? ComboBox_GetLBTextLen(hwndCtl, idx) : ComboBox_GetTextLength(hwndCtl)) + 1;
    LPWSTR const buf = StrgWriteAccessBuf(hstr, len);
    if (idx >= 0) {
        ComboBox_GetLBText(hwndCtl, idx, buf);
    }
    else {
        ComboBox_GetText(hwndCtl, buf, len);
    }
    StrgSanitize(hstr);
    return (int)StrgGetLength(hstr);
}

int ComboBox_GetTextW2MB(HWND hDlg, int nIDDlgItem, LPSTR lpString, size_t cch)
{
    HSTRINGW hstr = StrgCreate(NULL);
    ComboBox_GetTextHW(hDlg, nIDDlgItem, hstr);
    int const len = StrgGetAsUTF8(hstr, lpString, (int)cch);
    StrgDestroy(hstr);
    return len;
}

void ComboBox_SetTextW(HWND hDlg, int nIDDlgItem, LPCWSTR wstr)
{
    ComboBox_SetText(GetDlgItem(hDlg, nIDDlgItem), wstr ? wstr : L"");
}

void ComboBox_SetTextHW(HWND hDlg, int nIDDlgItem, const HSTRINGW hstr)
{
    ComboBox_SetText(GetDlgItem(hDlg, nIDDlgItem), StrgIsNotEmpty(hstr) ? StrgGet(hstr) : L"");
}

void ComboBox_SetTextMB2W(HWND hDlg, int nIDDlgItem, LPCSTR lpString)
{
    if (StrIsEmptyA(lpString)) {
        ComboBox_SetText(GetDlgItem(hDlg, nIDDlgItem), L"");
    }
    else {
        int const size = MultiByteToWideChar(Encoding_SciCP, 0, lpString, -1, NULL, 0) + 1;
        LPWSTR const buf = AllocMem(size * sizeof(WCHAR), HEAP_ZERO_MEMORY);
        MultiByteToWideChar(Encoding_SciCP, 0, lpString, -1, buf, size);
        ComboBox_SetText(GetDlgItem(hDlg, nIDDlgItem), buf);
        FreeMem(buf);
    }
}

#if 0
void ComboBox_AddStringMB2W(HWND hDlg, int nIDDlgItem, LPCSTR lpString)
{
    int const len = MultiByteToWideChar(CP_UTF8, 0, lpString, -1, NULL, 0) + 1;
    wchar_t* const buf = AllocMem(len * sizeof(wchar_t), HEAP_ZERO_MEMORY);
    MultiByteToWideChar(CP_UTF8, 0, lpString, -1, buf, len);
    ComboBox_AddString(GetDlgItem(hDlg, nIDDlgItem), buf);
    FreeMem(buf);
}
#endif

//=============================================================================
//
//  GetCenterOfDlgInParent()
//
POINT GetCenterOfDlgInParent(const RECT* rcDlg, const HWND hwndParent)
{
    RECT rcParent = { 0 };

    WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
    if (GetWindowPlacement(hwndParent, &wp)) {
        rcParent = wp.rcNormalPosition;
    }
    else {
        GetWindowRectEx(hwndParent, &rcParent);
    }

    HMONITOR const hMonitor = MonitorFromRect(&rcParent, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMonitor, &mi);
    int const xMin = mi.rcWork.left;
    int const xMax = (mi.rcWork.right) - (rcDlg->right - rcDlg->left);
    int const yMin = mi.rcWork.top;
    int const yMax = (mi.rcWork.bottom) - (rcDlg->bottom - rcDlg->top);

    int const x = rcParent.left + (((rcParent.right - rcParent.left) - (rcDlg->right - rcDlg->left)) >> 1);
    int const y = rcParent.top + (((rcParent.bottom - rcParent.top) - (rcDlg->bottom - rcDlg->top)) >> 1);

    POINT ptRet = { 0, 0 };
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
void CenterDlgInParent(HWND hDlg, HWND hDlgParent, bool bLock)
{
    if (!hDlg) { return; }

    RECT rcDlg = { 0 };
    GetWindowRect(hDlg, &rcDlg);

    HWND const hParentWnd = hDlgParent ? hDlgParent : GetParentOrDesktop(hDlg);

    POINT const ptTopLeft = GetCenterOfDlgInParent(&rcDlg, hParentWnd);

    SetWindowPos(hDlg, NULL, ptTopLeft.x, ptTopLeft.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    SetForegroundWindow(hDlg);
    if (bLock) {
        LockSetForegroundWindow(LSFW_LOCK);
    }
}


//=============================================================================
//
//  GetDlgPos()
//
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg)
{
    if (!hDlg) {
        return;
    }

    UINT const dpi = Scintilla_GetWindowDPI(hDlg);

    RECT rcDlg;
    GetWindowRect(hDlg, &rcDlg);

    HWND const hParent = GetParent(hDlg);
    RECT rcParent;
    GetWindowRect(hParent, &rcParent);

    // return positions relative to parent window (normalized DPI)
    if (xDlg) {
        *xDlg = MulDiv((rcDlg.left - rcParent.left), USER_DEFAULT_SCREEN_DPI, (dpi ? dpi : USER_DEFAULT_SCREEN_DPI));
    }
    if (yDlg) {
        *yDlg = MulDiv((rcDlg.top - rcParent.top), USER_DEFAULT_SCREEN_DPI, (dpi ? dpi : USER_DEFAULT_SCREEN_DPI));
    }
}


//=============================================================================
//
//  SetDlgPos()
//
void SetDlgPos(HWND hDlg, int xDlg, int yDlg)
{
    if (!hDlg) {
        return;
    }

    UINT const dpi = Scintilla_GetWindowDPI(hDlg);

    RECT rcDlg;
    GetWindowRect(hDlg, &rcDlg);

    HWND const hParent = GetParent(hDlg);
    RECT rcParent;
    GetWindowRect(hParent, &rcParent);

    HMONITOR const hMonitor = MonitorFromRect(&rcParent, MONITOR_DEFAULTTONEAREST);

    MONITORINFO mi = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMonitor, &mi);

    int const xMin = mi.rcWork.left;
    int const yMin = mi.rcWork.top;

    int const xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
    int const yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

    // desired positions relative to parent window (normalized DPI)
    int const x = rcParent.left + MulDiv(xDlg, dpi, USER_DEFAULT_SCREEN_DPI);
    int const y = rcParent.top + MulDiv(yDlg, dpi, USER_DEFAULT_SCREEN_DPI);

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
    UINT dpi;
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
    if (pm) {
        pm->direction = iDirection;
        pm->dpi = Scintilla_GetWindowDPI(hwnd);

        RECT rc;
        GetClientRect(hwnd, &rc);
        pm->cxClient = rc.right - rc.left;
        pm->cyClient = rc.bottom - rc.top;

        const DWORD style = (pm->direction < 0) ? (GetWindowStyle(hwnd) & ~WS_THICKFRAME) : (GetWindowStyle(hwnd) | WS_THICKFRAME);

        Scintilla_AdjustWindowRectForDpi((LPWRECT)&rc, style, 0, pm->dpi);

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

        WCHAR wch[MIDSZ_BUFFER];
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
    if (pm) {
        const int cxClient = LOWORD(lParam);
        const int cyClient = HIWORD(lParam);
#if NP3_ENABLE_RESIZEDLG_TEMP_FIX
        const UINT dpi = Scintilla_GetWindowDPI(hwnd);
        const UINT old = pm->dpi;
        if (cx) {
            *cx = cxClient - MulDiv(pm->cxClient, dpi, old);
        }
        if (cy) {
            *cy = cyClient - MulDiv(pm->cyClient, dpi, old);
        }
        // store in original DPI.
        pm->cxClient = MulDiv(cxClient, old, dpi);
        pm->cyClient = MulDiv(cyClient, old, dpi);
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
}

void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam)
{
    LPCRESIZEDLG pm = (LPCRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
    LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
#if NP3_ENABLE_RESIZEDLG_TEMP_FIX
    UINT const dpi = Scintilla_GetWindowDPI(hwnd);
    UINT const old = pm->dpi;

    lpmmi->ptMinTrackSize.x = MulDiv(pm->mmiPtMinX, dpi, old);
    lpmmi->ptMinTrackSize.y = MulDiv(pm->mmiPtMinY, dpi, old);

    // only one direction
    switch (pm->direction) {
    case RSZ_ONLY_X:
        lpmmi->ptMaxTrackSize.y = MulDiv(pm->mmiPtMaxY, dpi, old);
        break;

    case RSZ_ONLY_Y:
        lpmmi->ptMaxTrackSize.x = MulDiv(pm->mmiPtMaxX, dpi, old);
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

void ResizeDlg_SetAttr(HWND hwnd, int index, int value)
{
    if (index < MAX_RESIZEDLG_ATTR_COUNT) {
        PRESIZEDLG pm = (PRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
        pm->attrs[index] = value;
    }
}

int ResizeDlg_GetAttr(HWND hwnd, int index)
{
    if (index < MAX_RESIZEDLG_ATTR_COUNT) {
        const LPCRESIZEDLG pm = (LPCRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
        return pm->attrs[index];
    }
    return FALSE;
}

void ResizeDlg_InitY2Ex(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip, int iDirection, int nCtlId1, int nCtlId2)
{
    const int hMin1 = GetDlgCtrlHeight(hwnd, nCtlId1);
    const int hMin2 = GetDlgCtrlHeight(hwnd, nCtlId2);
    ResizeDlg_InitEx(hwnd, cxFrame, cyFrame, nIdGrip, iDirection);
    PRESIZEDLG pm = (PRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
    pm->attrs[0] = hMin1;
    pm->attrs[1] = hMin2;
}

int ResizeDlg_CalcDeltaY2(HWND hwnd, int dy, int cy, int nCtlId1, int nCtlId2)
{
    if (dy == 0) {
        return FALSE;
    }
    if (dy > 0) {
        return MulDiv(dy, cy, 100);
    }
    const LPCRESIZEDLG pm = (LPCRESIZEDLG)GetProp(hwnd, RESIZEDLG_PROP_KEY);
#if NP3_ENABLE_RESIZEDLG_TEMP_FIX
    UINT const dpi = Scintilla_GetWindowDPI(hwnd);
    int const hMinX = MulDiv(pm->attrs[0], dpi, pm->dpi);
    int const hMinY = MulDiv(pm->attrs[1], dpi, pm->dpi);
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


HDWP DeferCtlPos(HDWP hdwp, HWND hwndDlg, int nCtlId, int dx, int dy, UINT uFlags)
{
    HWND const hwndCtl = GetDlgItem(hwndDlg, nCtlId);
    RECT rc;
    GetWindowRect(hwndCtl, &rc);
    MapWindowPoints(NULL, hwndDlg, (LPPOINT)& rc, 2);
    if (uFlags & SWP_NOSIZE) {
        return DeferWindowPos(hdwp, hwndCtl, NULL, rc.left + dx, rc.top + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    return DeferWindowPos(hdwp, hwndCtl, NULL, 0, 0, rc.right - rc.left + dx, rc.bottom - rc.top + dy, SWP_NOZORDER | SWP_NOMOVE);
}


void ResizeDlgCtl(HWND hwndDlg, int nCtlId, int dx, int dy)
{
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
    if ((width ==  0) || (height == 0)) {
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

    BITMAP bmp = { 0 };
    GetObject(hBmp, sizeof(BITMAP), &bmp);
    BUTTON_IMAGELIST bi = { 0 };
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
    COLORMAP colormap[2] = { {0,0}, {0,0} };

    BUTTON_IMAGELIST bi = { 0 };
    if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)&bi)) {
        himlOld = bi.himl;
    }

    COLORREF const MASK_NONE = RGB(0x00, 0x00, 0x00);
    COLORREF const MASK_FULL = RGB(0xFF, 0xFF, 0xFF);
    COLORREF const CNTRST = RGB(0xFF, 0xFF, 0xFE);

    if (IsWindowEnabled(hwndCtl) && (crColor != COLORREF_MAX)) {
        colormap[0].from = MASK_NONE;
        colormap[0].to = GetSysColor(COLOR_3DSHADOW);
    } else {
        colormap[0].from = MASK_NONE;
        colormap[0].to = MASK_FULL;
    }

    if (IsWindowEnabled(hwndCtl) && (crColor != COLORREF_MAX)) {
        if (crColor == MASK_FULL) { crColor = CNTRST; }
        colormap[1].from = MASK_FULL;
        colormap[1].to = crColor;
    } else {
        colormap[1].from = MASK_FULL;
        colormap[1].to = MASK_FULL;
    }

    HBITMAP hBmp = CreateMappedBitmap(hInstance, IDB_PICK, 0, colormap, 2);

    BITMAP bmp = { 0 };
    GetObject(hBmp, sizeof(BITMAP), &bmp);
    bi.himl = ImageList_Create(bmp.bmWidth, bmp.bmHeight, ILC_COLORDDB | ILC_MASK, 1, 0);
    ImageList_AddMasked(bi.himl, hBmp, MASK_FULL);
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
    BUTTON_IMAGELIST bi = { 0 };
    if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)&bi)) {
        ImageList_Destroy(bi.himl);
    }
}


//=============================================================================
//
//  StatusOwnerDrawText()
//
void StatusSetText(HWND hwnd, BYTE nPart, LPCWSTR lpszText)
{
    if (lpszText) {
        bool const bSimplSB = (nPart == SB_SIMPLEID);
        if (bSimplSB) {
            int aSingleSect[1] = { -1 };
            SendMessage(hwnd, SB_SETPARTS, (WPARAM)1, (LPARAM)aSingleSect);
        }
        DWORD const wParam = (bSimplSB ? 0 : nPart) | SBT_OWNERDRAW;
        SendMessage(hwnd, SB_SETTEXT, (WPARAM)wParam, (LPARAM)lpszText);
    }
}

//=============================================================================
//
//  StatusSetTextID()
//
void StatusSetTextID(HWND hwnd, BYTE nPart, UINT uID)
{
    bool const bSimplSB = (nPart == SB_SIMPLEID);
    if (bSimplSB) {
        int aSingleSect[1] = { -1 };
        SendMessage(hwnd, SB_SETPARTS, (WPARAM)1, (LPARAM)aSingleSect);
    }
    DWORD const wParam = (bSimplSB ? 0 : nPart) | SBT_OWNERDRAW;
    if (!uID) {
        SendMessage(hwnd, SB_SETTEXT, (WPARAM)wParam, (LPARAM)L"");
    } else {
        WCHAR szText[MIDSZ_BUFFER] = { L'\0' };
        if (!GetLngString(uID, szText, COUNTOF(szText))) {
            SendMessage(hwnd, SB_SETTEXT, (WPARAM)wParam, (LPARAM)L"");
        } else {
            SendMessage(hwnd, SB_SETTEXT, (WPARAM)wParam, (LPARAM)szText);
        }
    }
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
        TBBUTTON tbb = { 0 };
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
        if (tchButtons[i] == L' ') {
            tchButtons[i] = 0;
        }
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
UINT GetCurrentPPI(HWND hwnd) {
    HDC const hDC = GetDC(hwnd);
    UINT ppi = 0;
    //ppi.x = max_u(GetDeviceCaps(hDC, LOGPIXELSX), USER_DEFAULT_SCREEN_DPI);
    ppi = max_u(GetDeviceCaps(hDC, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI);
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
    // deprecated:
    LANGID const langID = GetLangIdByLocaleName(Globals.CurrentLngLocaleName);
    bool bSucceed = GetLocaleDefaultUIFont(langID, lpFaceName, wSize);

    if (!bSucceed) {
        if (IsAppThemed()) {
            unsigned const iLogPixelsY = GetCurrentPPI(NULL) - DIALOG_FONT_SIZE_INCR;

            HTHEME hTheme = OpenThemeData(NULL, L"WINDOWSTYLE;WINDOW");
            if (hTheme) {
                LOGFONT lf;
                if (S_OK == GetThemeSysFont(hTheme, TMT_MSGBOXFONT, &lf)) {
                    if (lf.lfHeight < 0) {
                        lf.lfHeight = -lf.lfHeight;
                    }
                    *wSize = (WORD)MulDiv(lf.lfHeight, 72, iLogPixelsY);
                    if (*wSize < 9) {
                        *wSize = 9;
                    }
                    StringCchCopy(lpFaceName, LF_FACESIZE, lf.lfFaceName);
                    bSucceed = true;
                }
                CloseThemeData(hTheme);
            }
        }

        if (!bSucceed) {
            unsigned const iLogPixelsY = GetCurrentPPI(NULL) - DIALOG_FONT_SIZE_INCR;

            NONCLIENTMETRICS ncm = { 0 };
            ncm.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(ncm.iPaddedBorderWidth);
            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0)) {
                if (ncm.lfMessageFont.lfHeight < 0) {
                    ncm.lfMessageFont.lfHeight = -ncm.lfMessageFont.lfHeight;
                }
                *wSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight, 72, iLogPixelsY);
                if (*wSize < 9) {
                    *wSize = 9;
                }
                StringCchCopy(lpFaceName, LF_FACESIZE, ncm.lfMessageFont.lfFaceName);
                bSucceed = true;
            }
        }
    }

    return bSucceed;
}


static inline bool DialogTemplate_IsDialogEx(const DLGTEMPLATE* pTemplate)
{
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

    if (bDialogEx) {
        pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
    } else {
        pw = (WORD*)(pTemplate + 1);
    }

    if (*pw == WORD_MAX) {
        pw += 2;
    } else
        while (*pw++) {}

    if (*pw == WORD_MAX) {
        pw += 2;
    } else
        while (*pw++) {}

    while (*pw++) {}

    return (BYTE*)pw;
}


DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lpDialogTemplateID, HINSTANCE hInstance)
{
    HRSRC const hRsrc = FindResource(hInstance, lpDialogTemplateID, RT_DIALOG);
    if (!hRsrc) {
        return NULL;
    }

    HGLOBAL const hRsrcMem = LoadResource(hInstance, hRsrc);
    DLGTEMPLATE* const pRsrcMem = hRsrcMem ? (DLGTEMPLATE*) LockResource(hRsrcMem) : NULL;
    if (!pRsrcMem) {
        return NULL;
    }

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
    } else {
        pTemplate->style |= DS_SHELLFONT;
    }

    size_t const cbNew = cbFontAttr + ((StringCchLen(wchFaceName, COUNTOF(wchFaceName)) + 1) * sizeof(WCHAR));
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
            if (width) {
                *width = (int)bmp.bmWidth;
            }
            if (height) {
                *height = (int)bmp.bmHeight;
            }
            if (bitsPerPix) {
                *bitsPerPix = bmp.bmBitsPixel;
            }
        }
    } else if (info.hbmMask) {
        // Icon has no color plane, image data stored in mask
        BITMAP bmp = {0};
        if (GetObject(info.hbmMask, sizeof(bmp), &bmp) > 0) {
            if (width) {
                *width = (int)bmp.bmWidth;
            }
            if (height) {
                *height = (int)(bmp.bmHeight > 1);
            }
            if (bitsPerPix) {
                *bitsPerPix = 1;
            }
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
HBITMAP ResampleIconToBitmap(HWND hwnd, HBITMAP hOldBmp, const HICON hIcon, const int cx, const int cy)
{
    if (hOldBmp) {
        DeleteObject(hOldBmp);
    }
    //~return ConvertIconToBitmap(hwnd, hIcon, cx, cy);
    HBITMAP const hBmp = ConvertIconToBitmap(hIcon, 0, 0);
    return ResampleImageBitmap(hwnd, hBmp, cx, cy);
}

//=============================================================================
//
//  _MakeMenuIcon()
//
static MENUITEMINFO _MakeMenuIcon(const HICON hIcon, const UINT dpi)
{
    int const scx = Scintilla_GetSystemMetricsForDpi(SM_CXSMICON, dpi);
    int const scy = Scintilla_GetSystemMetricsForDpi(SM_CYSMICON, dpi);
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_BITMAP;
    mii.hbmpItem = ConvertIconToBitmap(hIcon, scx, scy);
    return mii;
}


//=============================================================================
//
//  SetUACIcon()
//
void SetUACIcon(HWND hwnd, const HMENU hMenu, const UINT nItem)
{
    static UINT dpi = 0; // (!) initially, to force first calculation 
    static MENUITEMINFO mii = { 0 };

    UINT const cur_dpi = Scintilla_GetWindowDPI(hwnd);

    if (dpi != cur_dpi) {
        if (mii.hbmpItem) {
            DeleteObject(mii.hbmpItem);
        }
        mii = _MakeMenuIcon(Globals.hIconMsgShield, cur_dpi);
        dpi = cur_dpi;
    }
    SetMenuItemInfo(hMenu, nItem, FALSE, &mii);
}


//=============================================================================
//
//  SetWinIcon()
//
void SetWinIcon(HWND hwnd, const HMENU hMenu, const UINT nItem)
{
    static UINT dpi = 0; // (!) initially, to force first calculation 
    static MENUITEMINFO mii = { 0 };

    UINT const cur_dpi = Scintilla_GetWindowDPI(hwnd);

    if (dpi != cur_dpi) {
        if (mii.hbmpItem) {
            DeleteObject(mii.hbmpItem);
        }
        mii = _MakeMenuIcon(Globals.hIconMsgWinCmd, cur_dpi);
        dpi = cur_dpi;
    }
    SetMenuItemInfo(hMenu, nItem, FALSE, &mii);
}


//=============================================================================
//
//  SetGrepWinIcon()
//
void SetGrepWinIcon(HWND hwnd, const HMENU hMenu, const UINT nItem)
{
    static UINT dpi = 0; // (!) initially, to force first calculation 
    static MENUITEMINFO mii = { 0 };

    UINT const cur_dpi = Scintilla_GetWindowDPI(hwnd);

    if (dpi != cur_dpi) {
        if (mii.hbmpItem) {
            DeleteObject(mii.hbmpItem);
        }
        mii = _MakeMenuIcon(Globals.hIconGrepWinNP3, cur_dpi);
        dpi = cur_dpi;
    }
    SetMenuItemInfo(hMenu, nItem, FALSE, &mii);
}


//=============================================================================
//
//  UpdateWindowLayoutForDPI()
//

#define USE_RECT_FOR_WIN_POS true

void UpdateWindowLayoutForDPI(HWND hwnd, const RECT *pNewRect, const UINT dpi)
{
    UINT const uWndFlags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED; //~ SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION

    if (pNewRect && (USE_RECT_FOR_WIN_POS || (dpi == 0))) {
        SetWindowPos(hwnd, NULL, pNewRect->left, pNewRect->top, 
            (pNewRect->right - pNewRect->left), (pNewRect->bottom - pNewRect->top), uWndFlags);
    } else {
        RECT rc = { 0 };
        GetWindowRect(hwnd, &rc);
        //~MapWindowPoints(NULL, hWnd, (LPPOINT)&rc, 2);
        UINT const _dpi = (dpi < (USER_DEFAULT_SCREEN_DPI >> 2)) ? Scintilla_GetWindowDPI(hwnd) : dpi;
        Scintilla_AdjustWindowRectForDpi((LPWRECT)&rc, uWndFlags, 0, _dpi);
        SetWindowPos(hwnd, NULL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), uWndFlags);
    }
    RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_INTERNALPAINT | RDW_ALLCHILDREN | RDW_UPDATENOW);
}


//=============================================================================
//
//  ResampleImageBitmap()  (resample_delete_orig)
//  if width|height <= 0 : scale bitmap to current dpi
//
HBITMAP ResampleImageBitmap(HWND hwnd, HBITMAP hbmp, int width, int height)
{
    if (hbmp) {
        BITMAP bmp = { 0 };
        if (GetObject(hbmp, sizeof(BITMAP), &bmp)) {
            if ((width <= 0) || (height <= 0)) {
                UINT const dpi = Scintilla_GetWindowDPI(hwnd);
                width  = ScaleIntByDPI(bmp.bmWidth, dpi);
                height = ScaleIntByDPI(bmp.bmHeight, dpi);
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

//=============================================================================
//
//  UpdateUI()
//
void UpdateUI(HWND hwnd)
{
    SendWMSize(hwnd, NULL);
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    //if (!IsWindows10OrGreater()) {
    //    PostMessage(hwnd, WM_NCACTIVATE, FALSE, -1); // (!)
    //    PostMessage(hwnd, WM_NCACTIVATE, TRUE, 0);
    //}
}


//=============================================================================
//
//  FontDialogHookProc()
//
WCHAR FontSelTitle[128];

INT_PTR CALLBACK FontDialogHookProc(
    HWND hdlg,     // handle to the dialog box window
    UINT uiMsg,    // message identifier
    WPARAM wParam, // message parameter
    LPARAM lParam  // message parameter
) {
    UNREFERENCED_PARAMETER(wParam);

    static UINT dpi = USER_DEFAULT_SCREEN_DPI;

    switch (uiMsg) {

    case WM_INITDIALOG: {

        if (Globals.hDlgIconSmall) {
            SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIconSmall);
        }

        InitWindowCommon(hdlg, true);

#ifdef D_NP3_WIN10_DARK_MODE

        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hdlg, IDOK));
            SetExplorerTheme(GetDlgItem(hdlg, IDCANCEL));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { grp1, grp2, chx1, chx2, cmb1, cmb2, cmb3, cmb4, cmb5, stc1, stc2, stc3, stc4, stc5, stc6, stc7 };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hdlg, ctl[i]), L"", L""); // remove theme
            }
        }
#endif
        //WCHAR buf[32] = { L'\0' };
        //SendMessage(GetDlgItem(hdlg, stc5), WM_GETTEXT, (WPARAM)30, (LPARAM)buf);
        //SendMessage(GetDlgItem(hdlg, stc5), WM_SETTEXT, 0, (LPARAM)buf);

        dpi = Scintilla_GetWindowDPI(hdlg);


        const CHOOSEFONT *const pChooseFont = ((CHOOSEFONT *)lParam);
        if (pChooseFont) {
            SendMessage(hdlg, WM_CHOOSEFONT_SETFLAGS, 0, (LPARAM)pChooseFont->Flags);
            if (pChooseFont->lCustData) {
                SetWindowText(hdlg, (WCHAR *)pChooseFont->lCustData);
            }
            const LOGFONT *const pLogFont = ((LOGFONT *)pChooseFont->lpLogFont);
            if (pLogFont) {
                // fill font name selector
                SendMessage(hdlg, WM_CHOOSEFONT_SETLOGFONT, 0, (LPARAM)pLogFont);
            }
        }
        //~else {
        //~  // HACK: to get the full font name instead of font family name
        //~  // [see: ChooseFontDirectWrite() PostProcessing]
        //~  SendMessage(hdlg, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)pChooseFont->lpLogFont);
        //~  PostMessage(hdlg, WM_CLOSE, 0, 0);
        //~}

        CenterDlgInParent(hdlg, NULL, false);

        PostMessage(hdlg, WM_THEMECHANGED, 0, 0);
    } break;

#ifdef D_NP3_WIN10_DARK_MODE

    CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hdlg, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hdlg, darkModeEnabled);
            RefreshTitleBarThemeColor(hdlg);
            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hdlg, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
        }
        UpdateWindowEx(hdlg);
        break;

#endif

    case WM_DPICHANGED:
        dpi = LOWORD(wParam);
        //dpi.y = HIWORD(wParam);
        UpdateWindowLayoutForDPI(hdlg, (RECT*)lParam, LOWORD(wParam));
        int const ctl[] = { cmb1, cmb2, cmb3, cmb4, cmb5 };
        for (int i = 0; i < COUNTOF(ctl); ++i) {
            HFONT const hFont = (HFONT)SendMessage(GetDlgItem(hdlg, ctl[i]), WM_GETFONT, 0, 0);
            SendMessage(GetDlgItem(hdlg, ctl[i]), WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        }
        return TRUE;

    default:
        break;
    }
    return 0; // Allow the default handler a chance to process
}



//=============================================================================
//
//  ColorDialogHookProc()
//
INT_PTR CALLBACK ColorDialogHookProc(
    HWND hdlg,     // handle to the dialog box window
    UINT uiMsg,    // message identifier
    WPARAM wParam, // message parameter
    LPARAM lParam  // message parameter
) {
    UNREFERENCED_PARAMETER(wParam);

    static UINT dpi = USER_DEFAULT_SCREEN_DPI;

    switch (uiMsg) {

    case WM_INITDIALOG: {

        if (Globals.hDlgIconSmall) {
            SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIconSmall);
        }

        InitWindowCommon(hdlg, true);

#ifdef D_NP3_WIN10_DARK_MODE

        if (UseDarkMode()) {
            SetExplorerTheme(GetDlgItem(hdlg, IDOK));
            SetExplorerTheme(GetDlgItem(hdlg, IDCANCEL));
            SetExplorerTheme(GetDlgItem(hdlg, COLOR_ADD));
            SetExplorerTheme(GetDlgItem(hdlg, COLOR_MIX));
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
            int const ctl[] = { COLOR_RAINBOW, COLOR_LUMSCROLL, COLOR_CURRENT, IDC_STATIC };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hdlg, ctl[i]), L"", L""); // remove theme
            }
        }
#endif
        dpi = Scintilla_GetWindowDPI(hdlg);

        const CHOOSECOLOR *const pChooseColor = ((CHOOSECOLOR *)lParam);
        if (pChooseColor && pChooseColor->lCustData) {
            POINT const pt = *(POINT*)pChooseColor->lCustData;
            SetWindowPos(hdlg, NULL, pt.x, pt.y, 0, 0, (SWP_NOZORDER | SWP_NOSIZE | SWP_HIDEWINDOW) & ~SWP_SHOWWINDOW);
            WININFO wi = GetMyWindowPlacement(hdlg, NULL, 0, true);
            SetWindowPos(hdlg, NULL, wi.x, wi.y, wi.cx, wi.cy, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
            SetForegroundWindow(hdlg);
        } else {
            CenterDlgInParent(hdlg, NULL, false);
        }

        PostMessage(hdlg, WM_THEMECHANGED, 0, 0);
    } 
    break;


#ifdef D_NP3_WIN10_DARK_MODE

    CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hdlg, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hdlg, darkModeEnabled);
            RefreshTitleBarThemeColor(hdlg);
            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hdlg, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
        }
        UpdateWindowEx(hdlg);
        break;

#endif

    case WM_DPICHANGED:
        dpi = LOWORD(wParam);
        //dpi.y = HIWORD(wParam);
        UpdateWindowLayoutForDPI(hdlg, (RECT*)lParam, LOWORD(wParam));
        int const ctl[] = { COLOR_ADD, COLOR_MIX, IDOK, IDCANCEL };
        for (int i = 0; i < COUNTOF(ctl); ++i) {
            HFONT const hFont = (HFONT)SendMessage(GetDlgItem(hdlg, ctl[i]), WM_GETFONT, 0, 0);
            SendMessage(GetDlgItem(hdlg, ctl[i]), WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        }
        return TRUE;

    default:
        break;
    }
    return 0; // Allow the default handler a chance to process
}


//=============================================================================
//
//  CleanupDlgResources()
//
void CleanupDlgResources()
{
    if (s_wchAdditionalTitleInfo) {
        StrgDestroy(s_wchAdditionalTitleInfo);
    }
    if (s_pthCachedFilePath) {
        Path_Release(s_pthCachedFilePath);
    }
}


//=============================================================================
//
//  _CanonicalizeInitialDir()  TODO: use Path_NormalizeEx() here ?
//
static void _CanonicalizeInitialDir(HPATHL hpth_in_out)
{
    if (Path_IsEmpty(hpth_in_out)) {

        if (Path_IsNotEmpty(Paths.CurrentFile)) {
            Path_Reset(hpth_in_out, Path_Get(Paths.CurrentFile));
            Path_RemoveFileSpec(hpth_in_out);
        }
        else if (Path_IsNotEmpty(Settings2.DefaultDirectory)) {
            Path_Reset(hpth_in_out, Path_Get(Settings2.DefaultDirectory));
        }
        else {
            Path_Reset(hpth_in_out, Path_Get(Paths.WorkingDirectory));
        }
        Path_CanonicalizeEx(hpth_in_out, Paths.ModuleDirectory);
    }
    else { // Path_IsNotEmpty(hpth_in_out)

        if (Path_IsRelative(hpth_in_out)) {
            Path_AbsoluteFromApp(hpth_in_out, true);
            //~ already Path_CanonicalizeEx(hpth_in_out, Paths.ModuleDirectory);
        }
        else {
            Path_CanonicalizeEx(hpth_in_out, Paths.ModuleDirectory);
        }
        if (!Path_IsExistingDirectory(hpth_in_out)) {
            Path_RemoveFileSpec(hpth_in_out);
        }
    }
    // finally: directory exists ?
    if (!Path_IsExistingDirectory(hpth_in_out)) {
        Path_Empty(hpth_in_out, false);
    }
}


#if 0
// ============================================================================
//
//  GetFolderDlg()
//  lpstrInitialDir == NULL      : leave initial dir to Open File Explorer
//  lpstrInitialDir == ""[empty] : use a reasonable initial directory path
//
bool GetFolderDlg(HWND hwnd, HPATHL hdir_pth_io, const HPATHL hinidir_pth)
{
    if (!hdir_pth_io) {
        return false;
    }

    HPATHL hpth_dir = Path_Allocate(Path_Get(hinidir_pth));
    _CanonicalizeInitialDir(hpth_dir);

    DWORD dwAttributes = Path_GetFileAttributes(hpth_dir);
    if ((dwAttributes == INVALID_FILE_ATTRIBUTES) || !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        Path_RemoveFileSpec(hpth_dir);
    }
    //if (dwAttributes != INVALID_FILE_ATTRIBUTES) {
    //    // if File is root, open the volume instead of open My Computer and select the volume
    //    if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) && Path_IsRoot(hpth_dir)) {
    //        bSelect = false;
    //    }
    //    else {
    //        inidirBuf = Path_Get(hinidir_pth);
    //    }
    //}
    dwAttributes = Path_GetFileAttributes(hpth_dir);
    if ((dwAttributes == INVALID_FILE_ATTRIBUTES) || !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return false;
    }

    Path_Swap(hdir_pth_io, hpth_dir);
    Path_Release(hpth_dir);

    LPCWSTR directory = Path_WriteAccessBuf(hdir_pth_io, PATHLONG_MAX_CCH);

    HRESULT hr = S_FALSE;
    PIDLIST_ABSOLUTE pidl = ILCreateFromPath(directory);
    if (pidl) {
        PIDLIST_ABSOLUTE pidlEntry = !Path_IsEmpty(hinidir_pth) ? ILCreateFromPath(Path_Get(hinidir_pth)) : NULL;
        if (pidlEntry) {
            hr = SHOpenFolderAndSelectItems(pidl, 1, (PCUITEMID_CHILD_ARRAY)(&pidlEntry), 0);
            CoTaskMemFree((LPVOID)pidlEntry);
        }
        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
        sei.fMask = SEE_MASK_IDLIST | SEE_MASK_NOCLOSEPROCESS;
        sei.hwnd = hwnd;
        sei.lpVerb = L"explore";
        //~sei.lpVerb = L"open";
        sei.lpIDList = (void*)pidl;
        sei.lpDirectory = NULL; //Path_Get(hinidir_pth);
        sei.nShow = SW_SHOW;
        sei.hInstApp = Globals.hInstance;

        const BOOL result = ShellExecuteEx(&sei);
        if (sei.hProcess) {
            WaitForSingleObject(sei.hProcess, INFINITE);
            CloseHandle(sei.hProcess);
            hr = result ? S_OK : S_FALSE;
        }
        else {
            hr = S_FALSE;
        }

        CoTaskMemFree((LPVOID)pidl);
    }

    Path_Sanitize(hdir_pth_io);
    Path_FreeExtra(hdir_pth_io, MAX_PATH_EXPLICIT);

    if (hr == S_OK) {
        return true;
    }

#if 0
    if (path == NULL) {
        path = wchDirectory;
    }

    // open a new explorer window every time
    LPWSTR szParameters = (LPWSTR)NP2HeapAlloc((lstrlen(path) + 64) * sizeof(WCHAR));
    lstrcpy(szParameters, bSelect ? L"/select," : L"");
    lstrcat(szParameters, L"\"");
    lstrcat(szParameters, path);
    lstrcat(szParameters, L"\"");
    ShellExecute(hwnd, L"open", L"explorer", szParameters, NULL, SW_SHOW);
    NP2HeapFree(szParameters);
#endif
    return false;
}
#endif


// ============================================================================
//
//  OpenFileDlg()
//  lpstrInitialDir == NULL      : leave initial dir to Open File Explorer
//  lpstrInitialDir == ""[empty] : use a reasonable initial directory path
//
// TODO: Replace GetOpenFileNameW() by Common Item Dialog: IFileOpenDialog()
//       https://docs.microsoft.com/en-us/windows/win32/shell/common-file-dialog
//
bool OpenFileDlg(HWND hwnd, HPATHL hfile_pth_io, const HPATHL hinidir_pth)
{
    if (!hfile_pth_io) {
        return false;
    }
    if (!Path_IsEmpty(hinidir_pth)) {
        // clear output path, so that GetOpenFileName does not use the contents of szFile to initialize itself.
        Path_Empty(hfile_pth_io, false);
    }
    WCHAR szDefExt[64] = { L'\0' };
    WCHAR szFilter[EXTENTIONS_FILTER_BUFFER];
    Style_GetFileFilterStr(szFilter, COUNTOF(szFilter), szDefExt, COUNTOF(szDefExt), false);

    HPATHL hpth_dir = Path_Copy(hinidir_pth);
    _CanonicalizeInitialDir(hpth_dir);

    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    ofn.hwndOwner = hwnd;
    ofn.hInstance = Globals.hInstance;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL; // no preserved (static member) user-defined patten
    ofn.lpstrInitialDir = Path_IsNotEmpty(hpth_dir) ? Path_Get(hpth_dir) : NULL;
    ofn.lpstrFile = Path_WriteAccessBuf(hfile_pth_io, PATHLONG_MAX_CCH);
    ofn.nMaxFile = (DWORD)Path_GetBufCount(hfile_pth_io);
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | /* OFN_NOCHANGEDIR |*/
                OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST |
                OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/;
    ofn.lpstrDefExt = StrIsNotEmpty(szDefExt) ? szDefExt : Settings2.DefaultExtension;

    bool const res = GetOpenFileNameW(&ofn);

    Path_Sanitize(hfile_pth_io);

    Path_Release(hpth_dir);
    Path_FreeExtra(hfile_pth_io, MAX_PATH_EXPLICIT);

    return res;
}

// ============================================================================
//
//  SaveFileDlg()
//  lpstrInitialDir == NULL      : leave initial dir to Save File Explorer
//  lpstrInitialDir == ""[empty] : use a reasonable initial directory path
//
bool SaveFileDlg(HWND hwnd, HPATHL hfile_pth_io, const HPATHL hinidir_pth)
{
    if (!hfile_pth_io) {
        return false;
    }

    WCHAR szDefExt[64] = { L'\0' };
    WCHAR szFilter[EXTENTIONS_FILTER_BUFFER];
    Style_GetFileFilterStr(szFilter, COUNTOF(szFilter), szDefExt, COUNTOF(szDefExt), true);

    HPATHL hpth_dir = Path_Copy(hinidir_pth);
    _CanonicalizeInitialDir(hpth_dir);

    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    ofn.hwndOwner = hwnd;
    ofn.hInstance = Globals.hInstance;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL; // no preserved (static member) user-defined patten
    ofn.lpstrInitialDir = Path_IsNotEmpty(hpth_dir) ? Path_Get(hpth_dir) : NULL;
    ofn.lpstrFile = Path_WriteAccessBuf(hfile_pth_io, PATHLONG_MAX_CCH);
    ofn.nMaxFile = (DWORD)Path_GetBufCount(hfile_pth_io);
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | /*| OFN_NOCHANGEDIR*/
                /*OFN_NODEREFERENCELINKS |*/ OFN_OVERWRITEPROMPT |
                OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = StrIsNotEmpty(szDefExt) ? szDefExt : Settings2.DefaultExtension;

    bool const res = GetSaveFileNameW(&ofn);

    Path_Sanitize(hfile_pth_io);

    Path_Release(hpth_dir);
    Path_FreeExtra(hfile_pth_io, MAX_PATH_EXPLICIT);

    return res;
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
        UINT const dpiSys = Scintilla_GetWindowDPI(NULL);
        UINT const dpiDlg = Scintilla_GetWindowDPI(hdlg);
        if (fontSize <= 0) {
            fontSize = (ncm.lfMessageFont.lfHeight < 0) ? -ncm.lfMessageFont.lfHeight : ncm.lfMessageFont.lfHeight;
            if (fontSize == 0) {
                fontSize = 9;
            }
        }
        fontSize <<= 10; // precision
        fontSize = MulDiv(fontSize, USER_DEFAULT_SCREEN_DPI, dpiSys.y); // correction
        fontSize = ScaleIntByDPI(fontSize, dpiDlg);
        ncm.lfMessageFont.lfHeight = -(MulDiv(fontSize, GetDeviceCaps(hdcSys, LOGPIXELSY), 72) >> 10);
        ncm.lfMessageFont.lfWeight = bold ? FW_BOLD : FW_REGULAR;
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
