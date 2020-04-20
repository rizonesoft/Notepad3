// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* MiniPath - Notepad3 Explorer Plugin                                         *
*                                                                             *
* Dialogs.c                                                                   *
*   dialog boxes implementation                                               *
*   Based on code from metapath, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601
#endif
#define _WIN32_IE 0x601
#define OEMRESOURCE  // use OBM_ resource constants
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <strsafe.h>
#include "minipath.h"
#include "dlapi.h"
#include "config.h"
#include "dialogs.h"
#include "resource.h"

#include "version.h"


//=============================================================================
//
//  ErrorMessage()
//
//  L"Title\nMessage Text"
//
extern HWND    hwndMain;
extern HICON   g_hDlgIconSmall;
extern LANGID  g_iPrefLANGID;


//=============================================================================
//
// BFFCallBack()
//
int CALLBACK BFFCallBack(HWND hwnd,UINT umsg,LPARAM lParam,LPARAM lpData)
{
  if (umsg == BFFM_INITIALIZED)
    SendMessage(hwnd,BFFM_SETSELECTION,TRUE,lpData);

  UNUSED(lParam);

  return(0);
}


//=============================================================================
//
// GetDirectory()
//
BOOL GetDirectory(HWND hwndParent,int iTitle,LPWSTR pszFolder,LPCWSTR pszBase,BOOL bNewDialogStyle)
{

  BROWSEINFO bi;
  LPITEMIDLIST pidl;
  WCHAR szTitle[256] = { L'\0' };
  WCHAR szBase[MAX_PATH] = { L'\0' };
  BOOL fOk = FALSE;

  lstrcpy(szTitle, L"");
  GetLngString(iTitle,szTitle,COUNTOF(szTitle));

  if (!pszBase || !*pszBase)
    GetCurrentDirectory(MAX_PATH,szBase);
  else
    lstrcpy(szBase,pszBase);

  bi.hwndOwner = hwndParent;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = pszFolder;
  bi.lpszTitle = szTitle;
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  bi.lpfn = &BFFCallBack;
  bi.lParam = (LPARAM)szBase;
  bi.iImage = 0;

  pidl = SHBrowseForFolder(&bi);
  if (pidl) {
    SHGetPathFromIDList(pidl,pszFolder);
    CoTaskMemFree(pidl);
    fOk = TRUE;
  }

  UNUSED(bNewDialogStyle);

  return fOk;
}


//=============================================================================
//
// GetDirectory2()
//
BOOL GetDirectory2(HWND hwndParent,int iTitle,LPWSTR pszFolder,int iBase)
{

  BROWSEINFO bi;
  LPITEMIDLIST pidl,pidlRoot;
  WCHAR szTitle[256] = { L'\0' };
  BOOL fOk = FALSE;

  lstrcpy(szTitle,L"");
  GetLngString(iTitle,szTitle,COUNTOF(szTitle));

  if (NOERROR != SHGetSpecialFolderLocation(hwndParent,iBase,&pidlRoot)) {
    CoTaskMemFree(pidlRoot);
    return FALSE;
  }

  bi.hwndOwner = hwndParent;
  bi.pidlRoot = pidlRoot;
  bi.pszDisplayName = pszFolder;
  bi.lpszTitle = szTitle;
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  bi.lpfn = NULL;
  bi.lParam = (LPARAM)0;
  bi.iImage = 0;

  pidl = SHBrowseForFolder(&bi);
  if (pidl) {
    SHGetPathFromIDList(pidl,pszFolder);
    CoTaskMemFree(pidl);
    fOk = TRUE;
  }
  CoTaskMemFree(pidlRoot);

  return fOk;
}


//=============================================================================
//
//  RunDlgProc()
//
//
extern HWND hwndDirList;

INT_PTR CALLBACK RunDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

        MakeBitmapButton(hwnd,IDC_SEARCHEXE,g_hInstance,IDB_OPEN);

        DLITEM dli;
        ZeroMemory(&dli, sizeof(DLITEM));
        dli.mask = DLI_FILENAME;
        if (DirList_GetItem(hwndDirList,-1,&dli) != -1)
        {
          LPWSTR psz = GetFilenameStr(dli.szFileName);
          QuotateFilenameStr(psz);
          SetDlgItemText(hwnd,IDC_COMMANDLINE,psz);
        }
        SendDlgItemMessage(hwnd,IDC_COMMANDLINE,EM_LIMITTEXT,MAX_PATH - 1,0);

        SHAutoComplete(GetDlgItem(hwnd,IDC_COMMANDLINE),SHACF_FILESYSTEM);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd,IDC_SEARCHEXE);
      return FALSE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_SEARCHEXE:
          {
            WCHAR szArgs[MAX_PATH];
            WCHAR szArg2[MAX_PATH];
            WCHAR szFile[MAX_PATH * 2];
            WCHAR szTitle[32];
            WCHAR szFilter[256];
            OPENFILENAME ofn;
            ZeroMemory(&ofn,sizeof(OPENFILENAME));

            GetDlgItemText(hwnd,IDC_COMMANDLINE,szArgs,COUNTOF(szArgs));
            ExpandEnvironmentStringsEx(szArgs,COUNTOF(szArgs));
            ExtractFirstArgument(szArgs,szFile,szArg2);

            GetLngString(IDS_SEARCHEXE,szTitle,COUNTOF(szTitle));
            GetLngString(IDS_FILTER_EXE,szFilter,COUNTOF(szFilter));
            PrepareFilterStr(szFilter);

            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = szFilter;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = COUNTOF(szFile);
            ofn.lpstrTitle = szTitle;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
                      | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

            if (GetOpenFileName(&ofn))
            {
              QuotateFilenameStr(szFile);

              if (StrIsNotEmpty(szArg2)) {
                lstrcat(szFile,L" ");
                lstrcat(szFile,szArg2);
              }
              SetDlgItemText(hwnd,IDC_COMMANDLINE,szFile);
            }

            PostMessage(hwnd,WM_NEXTDLGCTL,1,0);
          }
          break;


        case IDC_COMMANDLINE:
          {
            BOOL bEnableOK = FALSE;
            WCHAR args[MAX_PATH];

            if (GetDlgItemText(hwnd,IDC_COMMANDLINE,args,MAX_PATH))
              if (ExtractFirstArgument(args,args,NULL))
                if (StrIsNotEmpty(args))
                  bEnableOK = TRUE;

            EnableWindow(GetDlgItem(hwnd,IDOK),bEnableOK);
          }
          break;


        case IDOK:
          {
            WCHAR arg1[MAX_PATH];
            WCHAR arg2[MAX_PATH];

            if (GetDlgItemText(hwnd,IDC_COMMANDLINE,arg1,MAX_PATH))
            {

              if (*arg1 == L'/')
              {
                EndDialog(hwnd,IDOK);

                // Call DisplayPath() when dialog ended
                ExtractFirstArgument(&arg1[1],arg1,arg2);
                DisplayPath(arg1,IDS_ERR_CMDLINE);
              }

              else
              {
                ExpandEnvironmentStringsEx(arg1,COUNTOF(arg1));
                ExtractFirstArgument(arg1,arg1,arg2);

                SHELLEXECUTEINFO sei = { 0 };
                sei.cbSize = sizeof(SHELLEXECUTEINFO);
                sei.fMask = 0;
                sei.hwnd = hwnd;
                sei.lpVerb = NULL;
                sei.lpFile = arg1;
                sei.lpParameters = arg2;
                sei.lpDirectory = Settings.szCurDir;
                sei.nShow = SW_SHOWNORMAL;

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

      return TRUE;

  }

  UNUSED(lParam);

  return FALSE;
}


//=============================================================================
//
//  RunDlg()
//
//
INT_PTR RunDlg(HWND hwnd)
{
  return ThemedDialogBox(g_hLngResContainer,MAKEINTRESOURCE(IDD_RUN),hwnd,RunDlgProc);
}


//=============================================================================
//
//  GotoDlgProc()
//
//

extern HISTORY g_mHistory;

INT_PTR CALLBACK GotoDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        WCHAR tch[64];

        RECT rc;
        GetClientRect(hwnd,&rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc,GetWindowLong(hwnd,GWL_STYLE)|WS_THICKFRAME,FALSE,0);
        mmiPtMinX = rc.right-rc.left;
        mmiPtMaxY = rc.bottom-rc.top;

        if (Settings.cxGotoDlg < (rc.right-rc.left))
          Settings.cxGotoDlg = rc.right-rc.left;
        SetWindowPos(hwnd,NULL,rc.left,rc.top, Settings.cxGotoDlg,rc.bottom-rc.top,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
        SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

        GetMenuString(GetSystemMenu(GetParent(hwnd),FALSE),SC_SIZE,tch,COUNTOF(tch),MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,tch);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

        SetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP),GWL_STYLE,
          GetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);

        int cGrip = GetSystemMetrics(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,cxClient-cGrip,
                     cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);

        SendDlgItemMessage(hwnd,IDC_GOTO,CB_LIMITTEXT,MAX_PATH-1,0);
        SendDlgItemMessage(hwnd,IDC_GOTO,CB_SETEXTENDEDUI,TRUE,0);

        for (int i = 0; i < HISTORY_ITEMS; i++) {
          if (g_mHistory.psz[i]) {
            int iItem = (int)SendDlgItemMessage(hwnd,IDC_GOTO,
                          CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)g_mHistory.psz[i]);
            if (iItem != LB_ERR)
              SendDlgItemMessage(hwnd,IDC_GOTO,CB_DELETESTRING,iItem,0);
            SendDlgItemMessage(hwnd,IDC_GOTO,CB_INSERTSTRING,0,
                               (LPARAM)g_mHistory.psz[i]);
          }
        }

        COMBOBOXINFO cbi;
        cbi.cbSize = sizeof(COMBOBOXINFO);
        if (GetComboBoxInfo(GetDlgItem(hwnd,IDC_GOTO),&cbi))
          SHAutoComplete(cbi.hwndItem,SHACF_FILESYSTEM);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      {
        RECT rc;
        GetWindowRect(hwnd,&rc);
        Settings.cxGotoDlg = rc.right-rc.left;
      }
      return FALSE;


    case WM_SIZE:
      {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd,IDC_RESIZEGRIP),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDOK),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDOK),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDOK),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDCANCEL),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDCANCEL),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDCANCEL),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_GOTO),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_GOTO),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOMOVE);
        InvalidateRect(GetDlgItem(hwnd,IDC_GOTO),NULL,TRUE);
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        lpmmi->ptMaxTrackSize.y = mmiPtMaxY;
      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_GOTO:
          EnableWindow(GetDlgItem(hwnd,IDOK),
            (GetWindowTextLength(GetDlgItem(hwnd,IDC_GOTO)) ||
             CB_ERR != SendDlgItemMessage(hwnd,IDC_GOTO,CB_GETCURSEL,0,0)));

          if (HIWORD(wParam) == CBN_CLOSEUP) {
            LONG lSelEnd = 0;
            SendDlgItemMessage(hwnd,IDC_GOTO,CB_GETEDITSEL,0,(LPARAM)&lSelEnd);
            SendDlgItemMessage(hwnd,IDC_GOTO,CB_SETEDITSEL,0,MAKELPARAM(lSelEnd,lSelEnd));
          }
          break;


        case IDOK:
          {
            WCHAR tch[MAX_PATH];

            if (GetDlgItemText(hwnd,IDC_GOTO,tch,MAX_PATH))
            {
              EndDialog(hwnd,IDOK);
              PathUnquoteSpaces(tch);
              DisplayPath(tch,IDS_ERR_CMDLINE);
            }

            else
              EnableWindow(GetDlgItem(hwnd,IDOK),
                (GetWindowTextLength(GetDlgItem(hwnd,IDC_GOTO)) ||
                CB_ERR != SendDlgItemMessage(hwnd,IDC_GOTO,CB_GETCURSEL,0,0)));
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
//  GotoDlg()
//
//
INT_PTR GotoDlg(HWND hwnd)
{
  return ThemedDialogBox(g_hLngResContainer,MAKEINTRESOURCE(IDD_GOTO),hwnd,GotoDlgProc);
}


//=============================================================================
//
//  AboutDlgProc()
//
INT_PTR CALLBACK AboutDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static HFONT hFontTitle;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        WCHAR wch[256];
        LOGFONT lf;

        if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

        SetDlgItemText(hwnd,IDC_VERSION,VERSION_FILEVERSION_LONG);
        SetDlgItemText(hwnd,IDC_COPYRIGHT,VERSION_LEGALCOPYRIGHT_LONG);
        SetDlgItemText(hwnd,IDC_AUTHORNAME,VERSION_AUTHORNAME);

        if (hFontTitle) { DeleteObject(hFontTitle); }

        if (NULL == (hFontTitle = (HFONT)SendDlgItemMessage(hwnd, IDC_VERSION, WM_GETFONT, 0, 0))) {
          hFontTitle = GetStockObject(DEFAULT_GUI_FONT);
        }
        GetObject(hFontTitle,sizeof(LOGFONT),&lf);
        lf.lfWeight = FW_BOLD;
        lf.lfWidth = 8;
        lf.lfHeight = 22;
        hFontTitle = CreateFontIndirect(&lf);
        SendDlgItemMessage(hwnd,IDC_VERSION,WM_SETFONT,(WPARAM)hFontTitle,TRUE);

        if (GetDlgItem(hwnd,IDC_WEBPAGE) == NULL) {
          SetDlgItemText(hwnd,IDC_WEBPAGE2,VERSION_WEBPAGEDISPLAY);
          ShowWindow(GetDlgItem(hwnd,IDC_WEBPAGE2),SW_SHOWNORMAL);
        }
        else {
          wsprintf(wch,L"<A>%s</A>",VERSION_WEBPAGEDISPLAY);
          SetDlgItemText(hwnd,IDC_WEBPAGE,wch);
        }

        if (GetDlgItem(hwnd,IDC_EMAIL) == NULL) {
          SetDlgItemText(hwnd,IDC_EMAIL2,VERSION_FBWEBPAGEDISPLAY);
          ShowWindow(GetDlgItem(hwnd,IDC_EMAIL2),SW_SHOWNORMAL);
        }
        else {
          wsprintf(wch,L"<A>%s</A>", VERSION_FBWEBPAGEDISPLAY);
          SetDlgItemText(hwnd,IDC_EMAIL,wch);
        }

        GetLngString(IDS_MUI_TRANSL_AUTHOR, wch, COUNTOF(wch));
        SetDlgItemText(hwnd, IDC_TRANSL_AUTH, wch);

        CenterDlgInParent(hwnd);
      }
      return TRUE;



    case WM_NOTIFY:
      {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code) {

          case NM_CLICK:
          case NM_RETURN:
            {
              if (pnmhdr->idFrom == IDC_WEBPAGE) {
                ShellExecute(hwnd,L"open",L"http://www.flos-freeware.ch",NULL,NULL,SW_SHOWNORMAL);
              }
              else if (pnmhdr->idFrom == IDC_EMAIL) {
                ShellExecute(hwnd,L"open",L"mailto:florian.balmer@gmail.com",NULL,NULL,SW_SHOWNORMAL);
              }
            }
            break;
        }
      }
      break;

    case WM_COMMAND:

      switch(LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          EndDialog(hwnd,IDOK);
          break;
      }
      return TRUE;
  }
  return FALSE;
}


//=============================================================================
//
//  GeneralPageProc
//
//
extern WCHAR g_wchIniFile[MAX_PATH];

INT_PTR CALLBACK GeneralPageProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch (umsg)
  {

    case WM_INITDIALOG:

      if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

      if (StrIsNotEmpty(g_wchIniFile)) {
        if (Settings.bSaveSettings)
          CheckDlgButton(hwnd,IDC_SAVESETTINGS,BST_CHECKED);
      }
      else
        EnableWindow(GetDlgItem(hwnd,IDC_SAVESETTINGS),FALSE);

      if (Settings.bSingleClick)
        CheckDlgButton(hwnd,IDC_SINGLECLICK,BST_CHECKED);

      if (Settings.bTrackSelect)
        CheckDlgButton(hwnd,IDC_TRACKSELECT,BST_CHECKED);

      if (Settings.bFullRowSelect)
        CheckDlgButton(hwnd,IDC_FULLROWSELECT,BST_CHECKED);

      if (Settings.bFocusEdit)
        CheckDlgButton(hwnd,IDC_FOCUSEDIT,BST_CHECKED);

      if (Settings.bAlwaysOnTop)
        CheckDlgButton(hwnd,IDC_ALWAYSONTOP,BST_CHECKED);

      if (Settings.bMinimizeToTray)
        CheckDlgButton(hwnd,IDC_MINIMIZETOTRAY,BST_CHECKED);

      if (IniFileGetInt(g_wchIniFile, L"Settings2", L"ReuseWindow", 1)) {
        CheckDlgButton(hwnd, IDC_REUSEWINDOW, BST_CHECKED);
      }
      return TRUE;


    case WM_NOTIFY:

      switch (((LPNMHDR)lParam)->code)
      {

        case PSN_APPLY:

          if (IsWindowEnabled(GetDlgItem(hwnd,IDC_SAVESETTINGS))) {
            if (IsDlgButtonChecked(hwnd,IDC_SAVESETTINGS))
              Settings.bSaveSettings = 1;
            else
              Settings.bSaveSettings = 0;
          }

          if (IsDlgButtonChecked(hwnd,IDC_SINGLECLICK))
            Settings.bSingleClick = 1;
          else
            Settings.bSingleClick = 0;

          if (IsDlgButtonChecked(hwnd,IDC_TRACKSELECT))
            Settings.bTrackSelect = 1;
          else
            Settings.bTrackSelect = 0;

          if (IsDlgButtonChecked(hwnd,IDC_FULLROWSELECT))
            Settings.bFullRowSelect = 1;
          else
            Settings.bFullRowSelect  = 0;

          if (IsDlgButtonChecked(hwnd,IDC_FOCUSEDIT))
            Settings.bFocusEdit = 1;
          else
            Settings.bFocusEdit = 0;

          if (IsDlgButtonChecked(hwnd,IDC_ALWAYSONTOP))
            Settings.bAlwaysOnTop = 1;
          else
            Settings.bAlwaysOnTop = 0;

          if (IsDlgButtonChecked(hwnd,IDC_MINIMIZETOTRAY))
            Settings.bMinimizeToTray = 1;
          else
            Settings.bMinimizeToTray = 0;

          int const rw = IsDlgButtonChecked(hwnd, IDC_REUSEWINDOW) ? 1 : 0;
          IniFileSetInt(g_wchIniFile, L"Settings2", L"ReuseWindow", rw);

          SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);

          return TRUE;

      }

  }

  UNUSED(wParam);

  return FALSE;
}


//=============================================================================
//
//  AdvancedPageProc
//
//
INT_PTR CALLBACK AdvancedPageProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch (umsg)
  {

    case WM_INITDIALOG:

      if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

      if (Settings.bClearReadOnly)
        CheckDlgButton(hwnd,IDC_CLEARREADONLY,BST_CHECKED);

      if (Settings.bRenameOnCollision)
        CheckDlgButton(hwnd,IDC_RENAMEONCOLLISION,BST_CHECKED);

      if (Settings.fUseRecycleBin)
        CheckDlgButton(hwnd,IDC_USERECYCLEBIN,BST_CHECKED);

      if (Settings.fNoConfirmDelete)
        CheckDlgButton(hwnd,IDC_NOCONFIRMDELETE,BST_CHECKED);

      if (Settings.iStartupDir)
      {
        CheckDlgButton(hwnd,IDC_STARTUPDIR,BST_CHECKED);
        if (Settings.iStartupDir == 1)
          CheckRadioButton(hwnd,IDC_GOTOMRU,IDC_GOTOFAV,IDC_GOTOMRU);
        else
          CheckRadioButton(hwnd,IDC_GOTOMRU,IDC_GOTOFAV,IDC_GOTOFAV);
      }
      else
      {
        CheckRadioButton(hwnd,IDC_GOTOMRU,IDC_GOTOFAV,IDC_GOTOMRU);
        EnableWindow(GetDlgItem(hwnd,IDC_GOTOMRU),FALSE);
        EnableWindow(GetDlgItem(hwnd,IDC_GOTOFAV),FALSE);
      }

      if (Settings.iEscFunction)
      {
        CheckDlgButton(hwnd,IDC_ESCFUNCTION,BST_CHECKED);
        if (Settings.iEscFunction == 1)
          CheckRadioButton(hwnd,IDC_ESCMIN,IDC_ESCEXIT,IDC_ESCMIN);
        else
          CheckRadioButton(hwnd,IDC_ESCMIN,IDC_ESCEXIT,IDC_ESCEXIT);
      }
      else
      {
        CheckRadioButton(hwnd,IDC_ESCMIN,IDC_ESCEXIT,IDC_ESCMIN);
        EnableWindow(GetDlgItem(hwnd,IDC_ESCMIN),FALSE);
        EnableWindow(GetDlgItem(hwnd,IDC_ESCEXIT),FALSE);
      }

      return TRUE;


    case WM_COMMAND:

      if (LOWORD(wParam) == IDC_STARTUPDIR)
      {
        if (IsDlgButtonChecked(hwnd,IDC_STARTUPDIR))
        {
          EnableWindow(GetDlgItem(hwnd,IDC_GOTOMRU),TRUE);
          EnableWindow(GetDlgItem(hwnd,IDC_GOTOFAV),TRUE);
        }
        else
        {
          EnableWindow(GetDlgItem(hwnd,IDC_GOTOMRU),FALSE);
          EnableWindow(GetDlgItem(hwnd,IDC_GOTOFAV),FALSE);
        }
      }

      else if (LOWORD(wParam) == IDC_ESCFUNCTION)
      {
        if (IsDlgButtonChecked(hwnd,IDC_ESCFUNCTION))
        {
          EnableWindow(GetDlgItem(hwnd,IDC_ESCMIN),TRUE);
          EnableWindow(GetDlgItem(hwnd,IDC_ESCEXIT),TRUE);
        }
        else
        {
          EnableWindow(GetDlgItem(hwnd,IDC_ESCMIN),FALSE);
          EnableWindow(GetDlgItem(hwnd,IDC_ESCEXIT),FALSE);
        }
      }

      return TRUE;


    case WM_NOTIFY:

      switch (((LPNMHDR)lParam)->code)
      {

        case PSN_APPLY:

          if (IsDlgButtonChecked(hwnd,IDC_CLEARREADONLY))
            Settings.bClearReadOnly = 1;
          else
            Settings.bClearReadOnly = 0;

          if (IsDlgButtonChecked(hwnd,IDC_RENAMEONCOLLISION))
            Settings.bRenameOnCollision = 1;
          else
            Settings.bRenameOnCollision = 0;

          if (IsDlgButtonChecked(hwnd,IDC_USERECYCLEBIN))
            Settings.fUseRecycleBin = 1;
          else
            Settings.fUseRecycleBin = 0;

          if (IsDlgButtonChecked(hwnd,IDC_NOCONFIRMDELETE))
            Settings.fNoConfirmDelete = 1;
          else
            Settings.fNoConfirmDelete = 0;

          if (IsDlgButtonChecked(hwnd,IDC_STARTUPDIR)) {
            if (IsDlgButtonChecked(hwnd,IDC_GOTOMRU))
              Settings.iStartupDir = 1;
            else
              Settings.iStartupDir = 2;
          }
          else
            Settings.iStartupDir = 0;

          if (IsDlgButtonChecked(hwnd,IDC_ESCFUNCTION)) {
            if (IsDlgButtonChecked(hwnd,IDC_ESCMIN))
              Settings.iEscFunction = 1;
            else
              Settings.iEscFunction = 2;
          }
          else
            Settings.iEscFunction = 0;

          SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);

          return TRUE;

      }

  }

  return FALSE;

}


//=============================================================================
//
//  ItemsPageProc
//
//

INT_PTR CALLBACK ItemsPageProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static BOOL m_bDefCrNoFilt;
  static BOOL m_bDefCrFilter;

  static COLORREF m_crNoFilt;
  static COLORREF m_crFilter;

  static HBRUSH m_hbrNoFilt;
  static HBRUSH m_hbrFilter;

  CHOOSECOLOR cc;

  switch (umsg)
  {

    case WM_INITDIALOG:

      if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

      m_bDefCrNoFilt = Settings.bDefCrNoFilt;
      m_bDefCrFilter = Settings.bDefCrFilter;

      m_crNoFilt = Settings.crNoFilt;
      m_crFilter = Settings.crFilter;

      m_hbrNoFilt = CreateSolidBrush(m_crNoFilt);
      m_hbrFilter = CreateSolidBrush(m_crFilter);

      if (m_bDefCrNoFilt) {
        CheckRadioButton(hwnd,IDC_COLOR_DEF1,IDC_COLOR_CUST1,IDC_COLOR_DEF1);
        EnableWindow(GetDlgItem(hwnd,IDC_COLOR_PICK1),FALSE);
      }
      else
        CheckRadioButton(hwnd,IDC_COLOR_DEF1,IDC_COLOR_CUST1,IDC_COLOR_CUST1);

      if (m_bDefCrFilter) {
        CheckRadioButton(hwnd,IDC_COLOR_DEF2,IDC_COLOR_CUST2,IDC_COLOR_DEF2);
        EnableWindow(GetDlgItem(hwnd,IDC_COLOR_PICK2),FALSE);
      }
      else
        CheckRadioButton(hwnd,IDC_COLOR_DEF2,IDC_COLOR_CUST2,IDC_COLOR_CUST2);

      return TRUE;


    case WM_DESTROY:

      DeleteObject(m_hbrNoFilt);
      DeleteObject(m_hbrFilter);

      return FALSE;


    case WM_COMMAND:

      switch (LOWORD(wParam))
      {
        case IDC_COLOR_DEF1:
        case IDC_COLOR_CUST1:
          if (IsDlgButtonChecked(hwnd,IDC_COLOR_CUST1)) {
            m_bDefCrNoFilt = FALSE;
            EnableWindow(GetDlgItem(hwnd,IDC_COLOR_PICK1),TRUE);
          }

          else {
            m_bDefCrNoFilt = TRUE;
            EnableWindow(GetDlgItem(hwnd,IDC_COLOR_PICK1),FALSE);
          }

          InvalidateRect(GetDlgItem(hwnd,IDC_COLOR_SAMP1),NULL,TRUE);
          break;

        case IDC_COLOR_DEF2:
        case IDC_COLOR_CUST2:
          if (IsDlgButtonChecked(hwnd,IDC_COLOR_CUST2)) {
            m_bDefCrFilter = FALSE;
            EnableWindow(GetDlgItem(hwnd,IDC_COLOR_PICK2),TRUE);
          }

          else {
            m_bDefCrFilter = TRUE;
            EnableWindow(GetDlgItem(hwnd,IDC_COLOR_PICK2),FALSE);
          }

          InvalidateRect(GetDlgItem(hwnd,IDC_COLOR_SAMP2),NULL,TRUE);
          break;

        case IDC_COLOR_PICK1:
          ZeroMemory(&cc,sizeof(CHOOSECOLOR));

          cc.lStructSize = sizeof(CHOOSECOLOR);
          cc.hwndOwner = hwnd;
          cc.rgbResult = m_crNoFilt;
          cc.lpCustColors = Settings.crCustom;
          cc.Flags = CC_RGBINIT | CC_SOLIDCOLOR;

          if (ChooseColor(&cc)) {
            DeleteObject(m_hbrNoFilt);
            m_crNoFilt = cc.rgbResult;
            m_hbrNoFilt = CreateSolidBrush(m_crNoFilt);
          }

          InvalidateRect(GetDlgItem(hwnd,IDC_COLOR_SAMP1),NULL,TRUE);
          break;

        case IDC_COLOR_PICK2:
          ZeroMemory(&cc,sizeof(CHOOSECOLOR));

          cc.lStructSize = sizeof(CHOOSECOLOR);
          cc.hwndOwner = hwnd;
          cc.rgbResult = m_crFilter;
          cc.lpCustColors = Settings.crCustom;
          cc.Flags = CC_RGBINIT | CC_SOLIDCOLOR;

          if (ChooseColor(&cc)) {
            DeleteObject(m_hbrFilter);
            m_crFilter = cc.rgbResult;
            m_hbrFilter = CreateSolidBrush(m_crFilter);
          }

          InvalidateRect(GetDlgItem(hwnd,IDC_COLOR_SAMP2),NULL,TRUE);
          break;
      }

      return TRUE;


    case WM_NOTIFY:

      switch (((LPNMHDR)lParam)->code)
      {

        case PSN_APPLY:

          Settings.bDefCrNoFilt = m_bDefCrNoFilt;
          Settings.bDefCrFilter = m_bDefCrFilter;

          Settings.crNoFilt = m_crNoFilt;
          Settings.crFilter = m_crFilter;

          SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);

          return TRUE;

      }


    case WM_CTLCOLORSTATIC:

      if (!m_bDefCrNoFilt && GetDlgCtrlID((HWND)lParam) == IDC_COLOR_SAMP1)
        return(LRESULT)m_hbrNoFilt;

      else if (!m_bDefCrFilter && GetDlgCtrlID((HWND)lParam) == IDC_COLOR_SAMP2)
        return(LRESULT)m_hbrFilter;

      else
        return FALSE;

  }

  return FALSE;

}


//=============================================================================
//
//  ProgPageProc
//
//
INT_PTR CALLBACK ProgPageProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch (umsg)
  {

    case WM_INITDIALOG: {

        WCHAR tch[MAX_PATH];

        if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

        MakeBitmapButton(hwnd,IDC_BROWSE_Q,g_hInstance,IDB_OPEN);
        MakeBitmapButton(hwnd,IDC_BROWSE_F,g_hInstance,IDB_OPEN);

        lstrcpy(tch, Settings.szQuickview);
        PathQuoteSpaces(tch);
        if (StrIsNotEmpty(Settings.szQuickviewParams)) {
          StrCatBuff(tch,L" ",COUNTOF(tch));
          StrCatBuff(tch, Settings.szQuickviewParams,COUNTOF(tch));
        }
        SendDlgItemMessage(hwnd,IDC_QUICKVIEW,EM_LIMITTEXT,MAX_PATH - 2,0);
        SetDlgItemText(hwnd,IDC_QUICKVIEW,tch);
        SHAutoComplete(GetDlgItem(hwnd,IDC_QUICKVIEW),SHACF_FILESYSTEM);

        SendDlgItemMessage(hwnd,IDC_FAVORITES,EM_LIMITTEXT,MAX_PATH - 2,0);
        SetDlgItemText(hwnd,IDC_FAVORITES, Settings.g_tchFavoritesDir);
        SHAutoComplete(GetDlgItem(hwnd,IDC_FAVORITES),SHACF_FILESYSTEM);
      }
      return TRUE;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd,IDC_BROWSE_Q);
      DeleteBitmapButton(hwnd,IDC_BROWSE_F);
      return FALSE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_BROWSE_Q:
          {
            WCHAR tchBuf[MAX_PATH];
            WCHAR szFile[MAX_PATH];
            WCHAR szParams[MAX_PATH];
            WCHAR szTitle[32];
            WCHAR szFilter[256];
            OPENFILENAME ofn;

            ZeroMemory(&ofn,sizeof(OPENFILENAME));

            GetDlgItemText(hwnd,IDC_QUICKVIEW,tchBuf,COUNTOF(tchBuf));
            ExtractFirstArgument(tchBuf,szFile,szParams);

            GetLngString(IDS_GETQUICKVIEWER,szTitle,COUNTOF(szTitle));
            GetLngString(IDS_FILTER_EXE,szFilter,COUNTOF(szFilter));
            PrepareFilterStr(szFilter);

            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = szFilter;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = COUNTOF(szFile);
            ofn.lpstrTitle = szTitle;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
                      | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

            if (GetOpenFileName(&ofn)) {
              StrCpyN(tchBuf,szFile,COUNTOF(tchBuf));
              PathQuoteSpaces(tchBuf);
              if (StrIsNotEmpty(szParams)) {
                StrCatBuff(tchBuf,L" ",COUNTOF(tchBuf));
                StrCatBuff(tchBuf,szParams,COUNTOF(tchBuf));
              }
              SetDlgItemText(hwnd,IDC_QUICKVIEW,tchBuf);
            }

            PostMessage(hwnd,WM_NEXTDLGCTL,1,0);
          }
          break;

        case IDC_BROWSE_F:
          {
            WCHAR tch[MAX_PATH];

            GetDlgItemText(hwnd,IDC_FAVORITES,tch,COUNTOF(tch));
            StrTrim(tch,L" \"");

            if (GetDirectory(hwnd, IDS_FAVORITES, tch, tch, FALSE)) {
              SetDlgItemText(hwnd, IDC_FAVORITES, tch);
            }
            PostMessage(hwnd,WM_NEXTDLGCTL,1,0);
          }
          break;

      }

      return TRUE;


    case WM_NOTIFY:

      switch (((LPNMHDR)lParam)->code)
      {

        case PSN_APPLY: {

            WCHAR tch[MAX_PATH] = L"";

            if (!GetDlgItemText(hwnd,IDC_QUICKVIEW,tch,MAX_PATH)) {

              GetSystemDirectory(Settings.szQuickview,MAX_PATH);
              PathAddBackslash(Settings.szQuickview);
              lstrcat(Settings.szQuickview,L"Viewers\\Quikview.exe");
              PathQuoteSpaces(Settings.szQuickview);
              lstrcpy(Settings.szQuickviewParams,L"");
            }
            else
              ExtractFirstArgument(tch, Settings.szQuickview, Settings.szQuickviewParams);

            lstrcpy(tch, Settings.g_tchFavoritesDir);
            if (!GetDlgItemText(hwnd, IDC_FAVORITES, Settings.g_tchFavoritesDir, MAX_PATH)) {
              GetDefaultFavoritesDir(Settings.g_tchFavoritesDir, COUNTOF(Settings.g_tchFavoritesDir));
            }
            else
              StrTrim(Settings.g_tchFavoritesDir,L" \"");

            if (lstrcmpi(tch, Settings.g_tchFavoritesDir) != 0) { Settings.bNP3sFavoritesSettings = FALSE; }

            SetWindowLongPtr(hwnd,DWLP_MSGRESULT,PSNRET_NOERROR);
            
          }
          return TRUE;
      }
  }
  return FALSE;
}


//=============================================================================
//
//  OptionsPropSheet
//
//
extern HWND hwndStatus;

INT_PTR OptionsPropSheet(HWND hwnd,HINSTANCE hInstance)
{

  PROPSHEETPAGE   psp[4];
  ZeroMemory(psp,sizeof(PROPSHEETPAGE)*4);

  psp[0].dwSize      = sizeof(PROPSHEETPAGE);
  psp[0].dwFlags     = PSP_DLGINDIRECT;
  psp[0].hInstance   = hInstance;
  psp[0].pResource   = LoadThemedDialogTemplate(MAKEINTRESOURCE(IDPP_GENERAL),hInstance);
  psp[0].pfnDlgProc  = GeneralPageProc;

  psp[1].dwSize      = sizeof(PROPSHEETPAGE);
  psp[1].dwFlags     = PSP_DLGINDIRECT;
  psp[1].hInstance   = hInstance;
  psp[1].pResource   = LoadThemedDialogTemplate(MAKEINTRESOURCE(IDPP_ADVANCED),hInstance);
  psp[1].pfnDlgProc  = AdvancedPageProc;

  psp[2].dwSize      = sizeof(PROPSHEETPAGE);
  psp[2].dwFlags     = PSP_DLGINDIRECT;
  psp[2].hInstance   = hInstance;
  psp[2].pResource   = LoadThemedDialogTemplate(MAKEINTRESOURCE(IDPP_ITEMS),hInstance);
  psp[2].pfnDlgProc  = ItemsPageProc;

  psp[3].dwSize      = sizeof(PROPSHEETPAGE);
  psp[3].dwFlags     = PSP_DLGINDIRECT;
  psp[3].hInstance   = hInstance;
  psp[3].pResource   = LoadThemedDialogTemplate(MAKEINTRESOURCE(IDPP_PROG),hInstance);
  psp[3].pfnDlgProc  = ProgPageProc;

  PROPSHEETHEADER psh;
  ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
  psh.dwSize      = sizeof(PROPSHEETHEADER);
  psh.dwFlags     = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_PROPTITLE | PSH_USEHICON;
  psh.hwndParent  = hwnd;
  psh.hInstance   = hInstance;
  psh.hIcon       = g_hDlgIconSmall;
  psh.pszCaption  = L"MiniPath";
  psh.nPages      = 4;
  psh.nStartPage  = 0;
  psh.ppsp        = psp;

  INT_PTR nResult = PropertySheet(&psh);

  if (psp[0].pResource) LocalFree((HLOCAL)psp[0].pResource);
  if (psp[1].pResource) LocalFree((HLOCAL)psp[1].pResource);
  if (psp[2].pResource) LocalFree((HLOCAL)psp[2].pResource);
  if (psp[3].pResource) LocalFree((HLOCAL)psp[3].pResource);

  // Apply the results
  if (nResult)
  {
    if (Settings.bAlwaysOnTop)
      SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
    else
      SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

    if (Settings.bTrackSelect)
      ListView_SetExtendedListViewStyleEx(hwndDirList,
        LVS_EX_TRACKSELECT|LVS_EX_ONECLICKACTIVATE,
        LVS_EX_TRACKSELECT|LVS_EX_ONECLICKACTIVATE);
    else
      ListView_SetExtendedListViewStyleEx(hwndDirList,
        LVS_EX_TRACKSELECT|LVS_EX_ONECLICKACTIVATE,0);

    if (Settings.bFullRowSelect) {
      ListView_SetExtendedListViewStyleEx(hwndDirList,
        LVS_EX_FULLROWSELECT,
        LVS_EX_FULLROWSELECT);
      if (IsVista())
        SetTheme(hwndDirList,L"Explorer");
    }
    else {
      ListView_SetExtendedListViewStyleEx(hwndDirList,
        LVS_EX_FULLROWSELECT,0);
      if (IsVista())
        SetTheme(hwndDirList,L"Listview");
    }

    if (lstrcmp(Settings.tchFilter,L"*.*") || Settings.bNegFilter) {
      ListView_SetTextColor(hwndDirList,(Settings.bDefCrFilter) ? GetSysColor(COLOR_WINDOWTEXT) : Settings.crFilter);
      ListView_RedrawItems(hwndDirList,0,ListView_GetItemCount(hwndDirList)-1);
    }
    else {
      ListView_SetTextColor(hwndDirList,(Settings.bDefCrNoFilt) ? GetSysColor(COLOR_WINDOWTEXT) : Settings.crNoFilt);
      ListView_RedrawItems(hwndDirList,0,ListView_GetItemCount(hwndDirList)-1);
    }

  }
  return(nResult);
}


//=============================================================================
//
//  GetFilterDlgProc()
//
//

static HWND  s_hWnd = NULL;
static HMENU s_hMenu = NULL;
static DWORD s_dwIndex = 0;
static DWORD s_dwCheck = 0xFFFF; // index of current filter
static WCHAR s_szTypedFilter[512];


static void CALLBACK _AppendFilterMenu(LPCWSTR pszFilterName, LPCWSTR pszFilterValue)
{
  AppendMenu(s_hMenu, MF_ENABLED | MF_STRING, 1234 + s_dwIndex, pszFilterName);

  // Find description for current filter

  if ((!lstrcmpi(pszFilterValue,           s_szTypedFilter) && IsDlgButtonChecked(s_hWnd, IDC_NEGFILTER) != BST_CHECKED) ||
      (!lstrcmpi(CharNext(pszFilterValue), s_szTypedFilter) && IsDlgButtonChecked(s_hWnd, IDC_NEGFILTER) == BST_CHECKED && *pszFilterValue == L'-')) 
  {
    s_dwCheck = s_dwIndex;
  }
  s_dwIndex++;
}


INT_PTR CALLBACK GetFilterDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

        MakeBitmapButton(hwnd,IDC_BROWSEFILTER,NULL,OBM_COMBO);

        SendDlgItemMessage(hwnd,IDC_FILTER,EM_LIMITTEXT,COUNTOF(Settings.tchFilter)-1,0);
        SetDlgItemText(hwnd,IDC_FILTER, Settings.tchFilter);

        CheckDlgButton(hwnd,IDC_NEGFILTER, DlgBtnChk(Settings.bNegFilter));

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd,IDC_BROWSEFILTER);
      return FALSE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {
        case IDC_BROWSEFILTER:
          {
            s_hWnd = hwnd;
            s_dwIndex = 0;
            s_dwCheck = 0xFFFF; // index of current filter
            s_hMenu = CreatePopupMenu();
            GetDlgItemText(hwnd, IDC_FILTER, s_szTypedFilter, COUNTOF(s_szTypedFilter));

            IniFileIterateSection(g_wchIniFile, L"Filters", _AppendFilterMenu);

            if (s_dwCheck != 0xFFFF) { // check description for current filter
              CheckMenuRadioItem(s_hMenu, 0, s_dwIndex, s_dwCheck, MF_BYPOSITION);
            }
            if (s_dwIndex > 0) // at least 1 item exists
            {
              RECT rc;
              GetWindowRect(GetDlgItem(hwnd,IDC_BROWSEFILTER),&rc);
              //MapWindowPoints(hwnd,NULL,(POINT*)&rc,2);
              // Seems that TrackPopupMenuEx() works with client coords...?

              DWORD const dwCmd = TrackPopupMenuEx(s_hMenu,
                               TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
                               rc.left+1,rc.bottom+1,hwnd,NULL);

              if (dwCmd > 0)
              {
                WCHAR tchName[256];
                WCHAR tchValue[256];
                GetMenuString(s_hMenu,dwCmd,tchName,COUNTOF(tchName),MF_BYCOMMAND);

                if (IniFileGetString(g_wchIniFile, L"Filters", tchName, L"", tchValue, COUNTOF(tchValue)))
                {
                  if (tchValue[0] == L'-') // Negative Filter
                  {
                    if (tchValue[1]) {
                      SetDlgItemText(hwnd,IDC_FILTER,&tchValue[1]);
                      CheckDlgButton(hwnd,IDC_NEGFILTER,BST_CHECKED);
                    }
                    else {
                      MessageBeep(0);
                    }
                  }
                  else {
                    SetDlgItemText(hwnd,IDC_FILTER,tchValue);
                    CheckDlgButton(hwnd,IDC_NEGFILTER,BST_UNCHECKED);
                  }
                }
                else {
                  MessageBeep(0);
                }
              }
            }
            else
              ErrorMessage(0,IDS_ERR_FILTER);

            DestroyMenu(s_hMenu);
            s_hMenu = NULL;

            PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDC_FILTER)),1);
          }
          break;


        case IDOK:
          if (GetDlgItemText(hwnd,IDC_FILTER, Settings.tchFilter,COUNTOF(Settings.tchFilter)-1)) {
            Settings.bNegFilter = IsDlgButtonChecked(hwnd,IDC_NEGFILTER)?TRUE:FALSE;
          }
          else {
            lstrcpy(Settings.tchFilter,L"*.*");
            Settings.bNegFilter = FALSE;
          }
          EndDialog(hwnd,IDOK);
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  UNUSED(lParam);

  return FALSE;
}


//=============================================================================
//
//  GetFilterDlg()
//
//
BOOL GetFilterDlg(HWND hwnd)
{

  WCHAR tchOldFilter[DL_FILTER_BUFSIZE];
  BOOL bOldNegFilter;

  lstrcpy(tchOldFilter, Settings.tchFilter);
  bOldNegFilter = Settings.bNegFilter;

  if (IDOK == ThemedDialogBox(g_hLngResContainer,MAKEINTRESOURCE(IDD_FILTER),hwnd,GetFilterDlgProc))
  {
    if (!lstrcmpi(Settings.tchFilter,tchOldFilter) && (bOldNegFilter == Settings.bNegFilter))
      return(FALSE); // Old and new filters are identical
    else
      return(TRUE);
  }

  return(FALSE);

}


// Data structure used in file operation dialogs
typedef struct tagFILEOPDLGDATA
{
  WCHAR szSource[MAX_PATH];
  WCHAR szDestination[MAX_PATH];
  UINT wFunc;
} FILEOPDLGDATA, *LPFILEOPDLGDATA;


//=============================================================================
//
//  RenameFileDlgProc()
//
//
INT_PTR CALLBACK RenameFileDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    LPFILEOPDLGDATA lpfod;

    case WM_INITDIALOG:
      lpfod = (LPFILEOPDLGDATA)lParam;
      SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)lpfod);

      if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

      SetDlgItemText(hwnd,IDC_OLDNAME,lpfod->szSource);
      SetDlgItemText(hwnd,IDC_NEWNAME,lpfod->szSource);
      SendDlgItemMessage(hwnd,IDC_NEWNAME,EM_LIMITTEXT,MAX_PATH-1,0);
      SendDlgItemMessage(hwnd,IDC_NEWNAME,EM_SETMODIFY,0,0);

      CenterDlgInParent(hwnd);
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_NEWNAME:
            EnableWindow(GetDlgItem(hwnd,IDOK),
              GetWindowTextLength(GetDlgItem(hwnd,IDC_NEWNAME)));
          break;


        case IDOK:
          if (!SendDlgItemMessage(hwnd,IDC_NEWNAME,EM_GETMODIFY,0,0)) {
            EndDialog(hwnd,IDCANCEL);
          }
          else {
            lpfod = (LPFILEOPDLGDATA)(LONG_PTR)GetWindowLongPtr(hwnd,DWLP_USER);
            GetDlgItemText(hwnd,IDC_NEWNAME,lpfod->szDestination,
              COUNTOF(lpfod->szDestination)-1);
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
//  RenameFileDlg()
//
//
BOOL RenameFileDlg(HWND hwnd)
{

  DLITEM dli;
  FILEOPDLGDATA fod;
  SHFILEOPSTRUCT shfos;
  WCHAR szFullDestination[MAX_PATH];
  WCHAR tchSource[MAX_PATH+4];
  WCHAR tchDestination[MAX_PATH+4];

  dli.mask = DLI_FILENAME;
  if (DirList_GetItem(hwndDirList,-1,&dli) != -1)
    lstrcpy(fod.szSource,GetFilenameStr(dli.szFileName));

  else
   return FALSE;

  if (IDOK == ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_RENAME),hwnd,RenameFileDlgProc,(LPARAM)&fod))
  {
    ZeroMemory(&shfos,sizeof(SHFILEOPSTRUCT));
    shfos.hwnd = hwnd;
    shfos.wFunc = FO_RENAME;
    shfos.pFrom = tchSource;
    shfos.pTo = tchDestination;
    shfos.fFlags = FOF_ALLOWUNDO;

    // Generate fully qualified destination
    lstrcpy(szFullDestination,dli.szFileName);
    *GetFilenameStr(szFullDestination) = 0;
    lstrcat(szFullDestination,fod.szDestination);

    // Double null terminated strings are essential!!!
    ZeroMemory(tchSource,sizeof(WCHAR)*COUNTOF(tchSource));
    ZeroMemory(tchDestination,sizeof(WCHAR)*COUNTOF(tchDestination));
    lstrcpy(tchSource,dli.szFileName);
    lstrcpy(tchDestination,szFullDestination);

    if (SHFileOperation(&shfos) == 0) // success, select renamed item
    {
      SHFILEINFO shfi;
      // refresh directory view
      SendMessage(hwnd,WM_COMMAND,MAKELONG(IDM_VIEW_UPDATE,1),0);
      // get new display name
      SHGetFileInfo(tchDestination,0,&shfi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
      DirList_SelectItem(hwndDirList,shfi.szDisplayName,tchDestination);
    }

    return TRUE;

  }

  else
    return FALSE;

}


//=============================================================================
//
//  CopyMoveDlgProc()
//
//
INT_PTR CALLBACK CopyMoveDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  switch(umsg)
  {

    LPFILEOPDLGDATA lpfod;

    case WM_INITDIALOG:
      {
        RECT rc;
        WCHAR tch[64];
        int cGrip;
        //FARPROC fp;
        COMBOBOXINFO cbi;

        GetClientRect(hwnd,&rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc,GetWindowLong(hwnd,GWL_STYLE)|WS_THICKFRAME,FALSE,0);
        mmiPtMinX = rc.right-rc.left;
        mmiPtMaxY = rc.bottom-rc.top;

        if (Settings.cxCopyMoveDlg < (rc.right-rc.left))
          Settings.cxCopyMoveDlg = rc.right-rc.left;
        SetWindowPos(hwnd,NULL,rc.left,rc.top, Settings.cxCopyMoveDlg,rc.bottom-rc.top,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
        SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

        GetMenuString(GetSystemMenu(GetParent(hwnd),FALSE),SC_SIZE,tch,COUNTOF(tch),MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,tch);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

        SetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP2),GWL_STYLE,
          GetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP2),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);

        cGrip = GetSystemMetrics(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP2),NULL,cxClient-cGrip,
                     cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);

        MakeBitmapButton(hwnd,IDC_BROWSEDESTINATION,g_hInstance,IDB_OPEN);

        lpfod = (LPFILEOPDLGDATA)lParam;
        SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)lpfod);

        MRU_LoadToCombobox(GetDlgItem(hwnd,IDC_DESTINATION),L"Copy/Move MRU");
        SendDlgItemMessage(hwnd,IDC_DESTINATION,CB_SETCURSEL,0,0);

        SetDlgItemText(hwnd,IDC_SOURCE,lpfod->szSource);
        SendDlgItemMessage(hwnd,IDC_DESTINATION,CB_LIMITTEXT,MAX_PATH-1,0);

        SendDlgItemMessage(hwnd,IDC_DESTINATION,CB_SETEXTENDEDUI,TRUE,0);

        if (lpfod->wFunc == FO_COPY)
          CheckRadioButton(hwnd,IDC_FUNCCOPY,IDC_FUNCMOVE,IDC_FUNCCOPY);
        else
          CheckRadioButton(hwnd,IDC_FUNCCOPY,IDC_FUNCMOVE,IDC_FUNCMOVE);

        cbi.cbSize = sizeof(COMBOBOXINFO);
        if (GetComboBoxInfo(GetDlgItem(hwnd,IDC_DESTINATION),&cbi))
          SHAutoComplete(cbi.hwndItem,SHACF_FILESYSTEM);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      {
        RECT rc;
        GetWindowRect(hwnd,&rc);
        Settings.cxCopyMoveDlg = rc.right-rc.left;

        DeleteBitmapButton(hwnd,IDC_BROWSEDESTINATION);
      }
      return FALSE;


    case WM_SIZE:
      {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd,IDC_RESIZEGRIP2),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP2),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_RESIZEGRIP2),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDOK),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDOK),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDOK),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDCANCEL),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDCANCEL),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDCANCEL),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_SOURCE),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_SOURCE),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOMOVE);
        InvalidateRect(GetDlgItem(hwnd,IDC_SOURCE),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_DESTINATION),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_DESTINATION),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOMOVE);
        InvalidateRect(GetDlgItem(hwnd,IDC_DESTINATION),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_BROWSEDESTINATION),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_BROWSEDESTINATION),NULL,rc.left+dxClient,rc.top,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_BROWSEDESTINATION),NULL,TRUE);
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        lpmmi->ptMaxTrackSize.y = mmiPtMaxY;
      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_DESTINATION:
            EnableWindow(GetDlgItem(hwnd,IDOK),
              (GetWindowTextLength(GetDlgItem(hwnd,IDC_DESTINATION)) ||
               CB_ERR != SendDlgItemMessage(hwnd,IDC_DESTINATION,CB_GETCURSEL,0,0)));
          break;


        case IDC_BROWSEDESTINATION:
          {
            WCHAR tch[MAX_PATH];
            GetDlgItemText(hwnd,IDC_DESTINATION,tch,COUNTOF(tch));
            ExpandEnvironmentStringsEx(tch,COUNTOF(tch));
            if (GetDirectory(hwnd,IDS_COPYMOVE,tch,tch,TRUE))
              SetDlgItemText(hwnd,IDC_DESTINATION,tch);
            PostMessage(hwnd,WM_NEXTDLGCTL,1,0);
          }
          break;


        case IDOK:

/*text*/  lpfod = (LPFILEOPDLGDATA)GetWindowLongPtr(hwnd,DWLP_USER);
          if (GetDlgItemText(hwnd,IDC_DESTINATION,lpfod->szDestination,
            COUNTOF(lpfod->szDestination)-1)) {

  /*wfunc*/ if (IsDlgButtonChecked(hwnd,IDC_FUNCCOPY))
              lpfod->wFunc = FO_COPY;
            else
              lpfod->wFunc = FO_MOVE;

            EndDialog(hwnd,IDOK);
          }

          else
            EnableWindow(GetDlgItem(hwnd,IDOK),
              (GetWindowTextLength(GetDlgItem(hwnd,IDC_DESTINATION)) ||
               CB_ERR != SendDlgItemMessage(hwnd,IDC_DESTINATION,CB_GETCURSEL,0,0)));

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
//  CopyMoveDlg()
//
//
BOOL CopyMoveDlg(HWND hwnd,UINT *wFunc)
{

  DLITEM dli;
  FILEOPDLGDATA fod;
  SHFILEOPSTRUCT shfos;

  WCHAR tchSource[MAX_PATH+4];
  WCHAR tchDestination[MAX_PATH+4];

  fod.wFunc = *wFunc;

  dli.mask = DLI_FILENAME;
  if (DirList_GetItem(hwndDirList, -1, &dli) != -1) {
    lstrcpy(fod.szSource, GetFilenameStr(dli.szFileName));
  }
  else
   return FALSE;

  if (IDOK == ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_COPYMOVE),hwnd,CopyMoveDlgProc,(LPARAM)&fod))
  {
    ZeroMemory(&shfos,sizeof(SHFILEOPSTRUCT));
    shfos.hwnd = hwnd;
    shfos.wFunc = fod.wFunc;
    shfos.pFrom = tchSource;
    shfos.pTo = tchDestination;
    shfos.fFlags = FOF_NO_CONNECTED_ELEMENTS | FOF_ALLOWUNDO;
    if (shfos.wFunc == FO_COPY && Settings.bRenameOnCollision)
      shfos.fFlags |= FOF_RENAMEONCOLLISION;

    // Save item
    MRU_AddOneItem(L"Copy/Move MRU",fod.szDestination);
    ExpandEnvironmentStringsEx(fod.szDestination,COUNTOF(fod.szDestination));

    // Double null terminated strings are essential!!!
    ZeroMemory(tchSource,sizeof(WCHAR)*COUNTOF(tchSource));
    ZeroMemory(tchDestination,sizeof(WCHAR)*COUNTOF(tchDestination));
    lstrcpy(tchSource,dli.szFileName);
    lstrcpy(tchDestination,fod.szDestination);

    // tchDestination is always assumed to be a directory
    // if it doesn't exist, the file name of tchSource is added
    if (PathIsRelative(tchDestination)) {
      WCHAR wszDir[MAX_PATH];
      GetCurrentDirectory(COUNTOF(wszDir),wszDir);
      PathAppend(wszDir,tchDestination);
      lstrcpy(tchDestination,wszDir);
    }

    if (!PathIsDirectory(tchDestination))
      PathAppend(tchDestination,PathFindFileName(dli.szFileName));

    if (SHFileOperation(&shfos) == 0) // success
    {
      if (Settings.bClearReadOnly)
      {
        DWORD dwFileAttributes = GetFileAttributes(tchDestination);
        if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        {
          dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
          SetFileAttributes(tchDestination,dwFileAttributes);
          // this should work after the successful file operation...
        }
      }
    }

    *wFunc = fod.wFunc; // save state for next call

    return TRUE;
  }

  else
    return FALSE;

}

extern int flagNoFadeHidden;

INT_PTR CALLBACK OpenWithDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        RECT rc;
        WCHAR tch[MAX_PATH];
        int cGrip;
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        GetClientRect(hwnd,&rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc,GetWindowLong(hwnd,GWL_STYLE)|WS_THICKFRAME,FALSE,0);
        mmiPtMinX = rc.right-rc.left;
        mmiPtMaxY = rc.bottom-rc.top;

        if (Settings.cxOpenWithDlg < (rc.right-rc.left))
          Settings.cxOpenWithDlg = rc.right-rc.left;
        if (Settings.cyOpenWithDlg < (rc.bottom-rc.top))
          Settings.cyOpenWithDlg = rc.bottom-rc.top;
        SetWindowPos(hwnd,NULL,rc.left,rc.top, Settings.cxOpenWithDlg, Settings.cyOpenWithDlg,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
        SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

        GetMenuString(GetSystemMenu(GetParent(hwnd),FALSE),SC_SIZE,tch,COUNTOF(tch),MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,tch);
        InsertMenu(GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

        SetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP3),GWL_STYLE,
          GetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP3),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);

        cGrip = GetSystemMetrics(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,cxClient-cGrip,
                     cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)lParam);

        //SetExplorerTheme(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        ListView_SetExtendedListViewStyle(GetDlgItem(hwnd,IDC_OPENWITHDIR),/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,&lvc);
        DirList_Init(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL);
        DirList_Fill(GetDlgItem(hwnd,IDC_OPENWITHDIR), Settings.tchOpenWithDir,DL_ALLOBJECTS,NULL,FALSE, flagNoFadeHidden,DS_NAME,FALSE);
        DirList_StartIconThread(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        ListView_SetItemState(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVIS_FOCUSED,LVIS_FOCUSED);

        MakeBitmapButton(hwnd,IDC_GETOPENWITHDIR,g_hInstance,IDB_OPEN);

        CenterDlgInParent(hwnd);
      }
      return TRUE;


    case WM_DESTROY:
      {
        RECT rc;

        DirList_Destroy(GetDlgItem(hwnd,IDC_OPENWITHDIR));
        DeleteBitmapButton(hwnd,IDC_GETOPENWITHDIR);

        GetWindowRect(hwnd,&rc);
        Settings.cxOpenWithDlg = rc.right-rc.left;
        Settings.cyOpenWithDlg = rc.bottom-rc.top;
      }
      return FALSE;


    case WM_SIZE:
      {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd,IDC_RESIZEGRIP3),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_RESIZEGRIP3),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDOK),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDOK),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDOK),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDCANCEL),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDCANCEL),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDCANCEL),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_OPENWITHDIR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top+dyClient,SWP_NOZORDER|SWP_NOMOVE);
        InvalidateRect(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_GETOPENWITHDIR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_GETOPENWITHDIR),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,LVSCW_AUTOSIZE_USEHEADER);
        InvalidateRect(GetDlgItem(hwnd,IDC_OPENWITHDIR),NULL,TRUE);

        GetWindowRect(GetDlgItem(hwnd,IDC_OPENWITHDESCR),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_OPENWITHDESCR),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_OPENWITHDESCR),NULL,TRUE);
      }
      return TRUE;


    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        //lpmmi->ptMaxTrackSize.y = mmiPtMaxY;
      }
      return TRUE;


    case WM_NOTIFY:
      {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_OPENWITHDIR)
        {
          switch(pnmh->code)
          {
            case LVN_GETDISPINFO:
              DirList_GetDispInfo(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam,flagNoFadeHidden);
              break;

            case LVN_DELETEITEM:
              DirList_DeleteItem(GetDlgItem(hwnd,IDC_OPENWITHDIR),lParam);
              break;

            case LVN_ITEMCHANGED: {
                NM_LISTVIEW *pnmlv = (NM_LISTVIEW*)lParam;
                EnableWindow(GetDlgItem(hwnd,IDOK),(pnmlv->uNewState & LVIS_SELECTED));
              }
              break;

            case NM_DBLCLK:
              if (ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_OPENWITHDIR)))
                SendMessage(hwnd,WM_COMMAND,MAKELONG(IDOK,1),0);
              break;
          }
        }
      }
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_GETOPENWITHDIR:
          {
            if (GetDirectory(hwnd,IDS_OPENWITH,Settings.tchOpenWithDir, Settings.tchOpenWithDir,TRUE))
            {
              DirList_Fill(GetDlgItem(hwnd,IDC_OPENWITHDIR), Settings.tchOpenWithDir,DL_ALLOBJECTS,NULL,FALSE,flagNoFadeHidden,DS_NAME,FALSE);
              DirList_StartIconThread(GetDlgItem(hwnd,IDC_OPENWITHDIR));
              ListView_EnsureVisible(GetDlgItem(hwnd,IDC_OPENWITHDIR),0,FALSE);
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

      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  OpenWithDlg()
//
//
BOOL OpenWithDlg(HWND hwnd,LPDLITEM lpdliParam)
{

  DLITEM dliOpenWith;
  dliOpenWith.mask = DLI_FILENAME;

  if (IDOK == ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_OPENWITH),hwnd,OpenWithDlgProc,(LPARAM)&dliOpenWith))
  {

    WCHAR szDestination[MAX_PATH+4];
    ZeroMemory(szDestination,sizeof(WCHAR)*COUNTOF(szDestination));

    if (PathIsLnkToDirectory(dliOpenWith.szFileName,szDestination,
                             sizeof(WCHAR)*COUNTOF(szDestination)))
    {
      SHFILEOPSTRUCT shfos;

      WCHAR szSource[MAX_PATH+4];
      ZeroMemory(szSource,sizeof(WCHAR)*COUNTOF(szSource));
      lstrcpy(szSource,lpdliParam->szFileName);

      PathAppend(szDestination,PathFindFileName(szSource));

      ZeroMemory(&shfos,sizeof(SHFILEOPSTRUCT));
      shfos.hwnd = hwnd;
      shfos.wFunc = FO_COPY;
      shfos.pFrom = szSource;
      shfos.pTo = szDestination;
      shfos.fFlags = FOF_ALLOWUNDO;

      if (SHFileOperation(&shfos) == 0) // success
      {
        if (Settings.bClearReadOnly)
        {
          DWORD dwFileAttributes = GetFileAttributes(szDestination);
          if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
          {
            dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
            SetFileAttributes(szDestination,dwFileAttributes);
            // this should work after the successful file operation...
          }
        }
      }

      return(TRUE);
    }

    else
    {
      WCHAR szParam[MAX_PATH];

      SHELLEXECUTEINFO sei = { 0 };
      sei.cbSize = sizeof(SHELLEXECUTEINFO);
      sei.fMask = 0;
      sei.hwnd = hwnd;
      sei.lpVerb = NULL;
      sei.lpFile = dliOpenWith.szFileName;
      sei.lpParameters = szParam;
      sei.lpDirectory = Settings.szCurDir;
      sei.nShow = SW_SHOWNORMAL;

      // resolve links and get short path name
      if (!(PathIsLnkFile(lpdliParam->szFileName) &&
            PathGetLnkPath(lpdliParam->szFileName,szParam,COUNTOF(szParam))))
        lstrcpy(szParam,lpdliParam->szFileName);

      GetShortPathName(szParam,szParam,sizeof(WCHAR)*COUNTOF(szParam));

      ShellExecuteEx(&sei);

      return(TRUE);
    }
  }

  return(FALSE);

}


//=============================================================================
//
//  NewDirDlgProc()
//
//
INT_PTR CALLBACK NewDirDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  switch(umsg)
  {

    LPFILEOPDLGDATA lpfod;

    case WM_INITDIALOG:
      lpfod = (LPFILEOPDLGDATA)lParam;
      SetWindowLongPtr(hwnd,DWLP_USER,(LONG_PTR)lpfod);

      if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

      SendDlgItemMessage(hwnd,IDC_NEWDIR,EM_LIMITTEXT,MAX_PATH-1,0);

      CenterDlgInParent(hwnd);
      return TRUE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_NEWDIR:
            EnableWindow(GetDlgItem(hwnd,IDOK),
              GetWindowTextLength(GetDlgItem(hwnd,IDC_NEWDIR)));
          break;


        case IDOK:
          lpfod = (LPFILEOPDLGDATA)GetWindowLongPtr(hwnd,DWLP_USER);
          GetDlgItemText(hwnd,IDC_NEWDIR,lpfod->szDestination,
            COUNTOF(lpfod->szDestination)-1);
          EndDialog(hwnd,IDOK);
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
//  NewDirDlg()
//
//
BOOL NewDirDlg(HWND hwnd,LPWSTR pszNewDir)
{

  FILEOPDLGDATA fod;

  if (IDOK == ThemedDialogBoxParam(g_hLngResContainer,MAKEINTRESOURCE(IDD_NEWDIR),hwnd,NewDirDlgProc,(LPARAM)&fod))
  {
    lstrcpy(pszNewDir,fod.szDestination);

    return TRUE;
  }

  else
    return FALSE;

}


//=============================================================================
//
//  FindWinDlgProc()
//
//  Find target window helper dialog
//
extern int flagPortableMyDocs;

INT_PTR CALLBACK FindWinDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static WCHAR *pTargetWndClassBuf;
  static HICON hIconCross1, hIconCross2;
  static HCURSOR hCursorCross;
  static BOOL bHasCapture;

  switch(umsg)
  {

    case WM_INITDIALOG:
      pTargetWndClassBuf = (WCHAR*)lParam;
      hIconCross1 = LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_CROSS1));
      hIconCross2 = LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_CROSS2));
      hCursorCross = LoadCursor(g_hInstance,MAKEINTRESOURCE(IDC_CROSSHAIR));

      if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

      CenterDlgInParent(hwnd);
      bHasCapture = FALSE;
      return TRUE;

    case WM_CANCELMODE:
      ReleaseCapture();
      bHasCapture = FALSE;
      break;

    case WM_LBUTTONDOWN: {
        POINT pt;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        if (GetDlgCtrlID((ChildWindowFromPoint(hwnd,pt))) == IDC_CROSSCURSOR) {
          SetCapture(hwnd);
          bHasCapture = TRUE;
          SetCursor(hCursorCross);
          SendDlgItemMessage(hwnd,IDC_CROSSCURSOR,STM_SETICON,(WPARAM)hIconCross2,0);
        }
      }
      break;

    case WM_LBUTTONUP:
      {
        WCHAR tch[256];

        SetCursor(LoadCursor(NULL,IDC_ARROW));
        SendDlgItemMessage(hwnd,IDC_CROSSCURSOR,STM_SETICON,(WPARAM)hIconCross1,0);
        ReleaseCapture();
        bHasCapture = FALSE;

        EnableWindow(GetDlgItem(hwnd,IDOK),GetDlgItemText(hwnd,IDC_WINCLASS,tch,COUNTOF(tch)));
        if (IsWindowEnabled(GetDlgItem(hwnd,IDOK)))
          PostMessage(hwnd,WM_NEXTDLGCTL,(WPARAM)(GetDlgItem(hwnd,IDOK)),1);

        //if (GetDlgItemText(hwnd,IDC_WINMODULE,tch,COUNTOF(tch)))
        //  SetDlgItemText(GetParent(hwnd),IDC_TARGETPATH,tch);

        //if (GetDlgItemText(hwnd,IDC_WINCLASS,tch,COUNTOF(tch)))
        //{
        //  lstrcpyn(pTargetWndClassBuf,tch,256);
        //  PostMessage(hwnd,WM_CLOSE,0,0);
        //}
      }
      break;

    case WM_MOUSEMOVE:
      {
        HWND hwndFind;
        POINT pt;
        WCHAR tch[256];

        if (bHasCapture) {

          GetCursorPos(&pt);
          hwndFind = WindowFromPoint(pt);

          while (GetWindowLongPtr(hwndFind,GWL_STYLE) & WS_CHILD)
            hwndFind = GetParent(hwndFind);

          if (hwndFind != hwnd) {

            GetWindowText(hwndFind,tch,COUNTOF(tch));
            SetDlgItemText(hwnd,IDC_WINTITLE,tch);
            GetClassName(hwndFind,tch,COUNTOF(tch));
            SetDlgItemText(hwnd,IDC_WINCLASS,tch);

            if (ExeNameFromWnd(hwndFind,tch,COUNTOF(tch)))
              SetDlgItemText(hwnd,IDC_WINMODULE,tch);
            else
              SetDlgItemText(hwnd,IDC_WINMODULE,L"");
          }

          else {
            SetDlgItemText(hwnd,IDC_WINTITLE,L"");
            SetDlgItemText(hwnd,IDC_WINCLASS,L"");
            SetDlgItemText(hwnd,IDC_WINMODULE,L"");
      } } }
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          if (bHasCapture) {
            ReleaseCapture();
            SendMessage(hwnd,WM_LBUTTONUP,0,0);
          }
          else
          {
            WCHAR tch[MAX_PATH] = L"";
            if (LOWORD(wParam) == IDOK) {
              if (GetDlgItemText(hwnd,IDC_WINMODULE,tch,COUNTOF(tch))) {
                PathRelativeToApp(tch,tch,COUNTOF(tch),TRUE,TRUE,flagPortableMyDocs);
                PathQuoteSpaces(tch);
                SetDlgItemText(GetParent(hwnd),IDC_TARGETPATH,tch);
              }

              if (GetDlgItemText(hwnd,IDC_WINCLASS,tch,COUNTOF(tch)))
                lstrcpyn(pTargetWndClassBuf,tch,256);
            }

            DestroyIcon(hIconCross1);
            DestroyIcon(hIconCross2);
            DestroyCursor(hCursorCross);
            EndDialog(hwnd,IDOK);
          }
          break;
      }
      return TRUE;

  }

  return FALSE;

}


//=============================================================================
//
//  FindTargetDlgProc()
//
//  Select MiniPath target application
//
extern UseTargetApp eUseTargetApplication;
extern TargetAppMode eTargetApplicationMode;

extern WCHAR szTargetApplication[MAX_PATH];
extern WCHAR szTargetApplicationParams[MAX_PATH];
extern WCHAR szTargetApplicationWndClass[MAX_PATH];
extern WCHAR szDDEMsg[256];
extern WCHAR szDDEApp[256];
extern WCHAR szDDETopic[256];


INT_PTR CALLBACK FindTargetDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static WCHAR szTargetWndClass[256];

  switch(umsg)
  {

    case WM_INITDIALOG:
      {
        HWND hwndToolTip;
        TOOLINFO ti;

        int i;
        WCHAR wch[MAX_PATH];

        // ToolTip for browse button
        hwndToolTip = CreateWindowEx(0,TOOLTIPS_CLASS,NULL,0,0,0,0,0,hwnd,NULL,g_hInstance,NULL);

        ZeroMemory(&ti,sizeof(TOOLINFO));
        ti.cbSize   = sizeof(TOOLINFO);
        ti.uFlags   = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hwnd     = hwnd;
        ti.uId      = (UINT_PTR)GetDlgItem(hwnd,IDC_BROWSE);
        ti.hinst    = g_hInstance;
        ti.lpszText = (LPWSTR)IDS_SEARCHEXE;

        if (!SendMessage(hwndToolTip,TTM_ADDTOOL,0,(LPARAM)&ti))
          DestroyWindow(hwndToolTip);

        //// ToolTip for find window button
        //hwndToolTip = CreateWindowEx(0,TOOLTIPS_CLASS,NULL,0,0,0,0,0,hwnd,
        //                             NULL,g_hInstance,NULL);

        //ZeroMemory(&ti,sizeof(TOOLINFO));
        //ti.cbSize   = sizeof(TOOLINFO);
        //ti.uFlags   = TTF_IDISHWND | TTF_SUBCLASS;
        //ti.hwnd     = hwnd;
        //ti.uId      = (UINT)GetDlgItem(hwnd,IDC_FINDWIN);
        //ti.hinst    = g_hInstance;
        //ti.lpszText = (LPWSTR)IDS_FINDWINTITLE;

        //if (!SendMessage(hwndToolTip,TTM_ADDTOOL,0,(LPARAM)&ti))
        //  DestroyWindow(hwndToolTip);

        if (g_hDlgIconSmall) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIconSmall); }

        // Bitmap für den Browse-Button
        MakeBitmapButton(hwnd,IDC_BROWSE,g_hInstance,IDB_OPEN);
        //MakeBitmapButton(hwnd,IDC_FINDWIN,g_hInstance,IDB_BROWSE);

        // initialize edit controls
        SendDlgItemMessage(hwnd,IDC_TARGETPATH,EM_LIMITTEXT,MAX_PATH - 1,0);
        SHAutoComplete(GetDlgItem(hwnd,IDC_TARGETPATH),SHACF_FILESYSTEM|SHACF_URLMRU);

        SendDlgItemMessage(hwnd,IDC_DDEMSG,EM_LIMITTEXT,128,0);
        SendDlgItemMessage(hwnd,IDC_DDEAPP,EM_LIMITTEXT,128,0);
        SendDlgItemMessage(hwnd,IDC_DDETOPIC,EM_LIMITTEXT,128,0);

        LoadTargetParamsOnce();

        if (eUseTargetApplication != UTA_UNDEFINED)
          CheckRadioButton(hwnd,IDC_LAUNCH,IDC_TARGET,IDC_TARGET);
        else
          CheckRadioButton(hwnd,IDC_LAUNCH,IDC_TARGET,IDC_LAUNCH);

        lstrcpy(wch, szTargetApplication);
        PathQuoteSpaces(wch);
        if (StrIsNotEmpty(szTargetApplicationParams)) {
          StrCatBuff(wch,L" ",COUNTOF(wch));
          StrCatBuff(wch, szTargetApplicationParams,COUNTOF(wch));
        }
        SetDlgItemText(hwnd,IDC_TARGETPATH,wch);

        if (eUseTargetApplication != UTA_UNDEFINED) {
          i = clampi(eTargetApplicationMode, TAM_ALWAYS_RUN, TAM_SEND_DDE_MSG);
          CheckRadioButton(hwnd,IDC_ALWAYSRUN,IDC_USEDDE,IDC_ALWAYSRUN + i);
        }

        lstrcpy(szTargetWndClass, szTargetApplicationWndClass);

        SetDlgItemText(hwnd,IDC_DDEMSG, szDDEMsg);
        SetDlgItemText(hwnd,IDC_DDEAPP, szDDEApp);
        SetDlgItemText(hwnd,IDC_DDETOPIC, szDDETopic);

        CenterDlgInParent(hwnd);

      }
      return TRUE;


    case WM_DESTROY:
      DeleteBitmapButton(hwnd,IDC_BROWSE);
      //DeleteBitmapButton(hwnd,IDC_FINDWIN);
      return FALSE;


    case WM_COMMAND:

      switch(LOWORD(wParam))
      {

        case IDC_BROWSE:
          {
            WCHAR tchBuf[MAX_PATH];
            WCHAR szFile[MAX_PATH];
            WCHAR szParams[MAX_PATH];
            WCHAR szTitle[32];
            WCHAR szFilter[256];

            OPENFILENAME ofn;
            ZeroMemory(&ofn,sizeof(OPENFILENAME));

            GetDlgItemText(hwnd,IDC_TARGETPATH,tchBuf,COUNTOF(tchBuf));
            ExtractFirstArgument(tchBuf,szFile,szParams);
            PathAbsoluteFromApp(szFile,szFile,COUNTOF(szFile),TRUE);

            // Strings laden
            GetLngString(IDS_SEARCHEXE,szTitle,COUNTOF(szTitle));
            GetLngString(IDS_FILTER_EXE,szFilter,COUNTOF(szFilter));
            PrepareFilterStr(szFilter);

            // ofn ausfüllen
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner   = hwnd;
            ofn.lpstrFilter = szFilter;
            ofn.lpstrFile   = szFile;
            ofn.nMaxFile    = COUNTOF(szFile);
            ofn.lpstrTitle  = szTitle;
            ofn.Flags       = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR |
                              OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_NODEREFERENCELINKS;

            // execute file open dlg
            if (GetOpenFileName(&ofn)) {
              StrCpyN(tchBuf,szFile,COUNTOF(tchBuf));
              PathRelativeToApp(tchBuf,tchBuf,COUNTOF(tchBuf),TRUE,TRUE,flagPortableMyDocs);
              PathQuoteSpaces(tchBuf);
              if (StrIsNotEmpty(szParams)) {
                StrCatBuff(tchBuf,L" ",COUNTOF(tchBuf));
                StrCatBuff(tchBuf,szParams,COUNTOF(tchBuf));
              }
              SetDlgItemText(hwnd,IDC_TARGETPATH,tchBuf);
            }

            // set focus to edit control
            PostMessage(hwnd,WM_NEXTDLGCTL,1,0);

            CheckRadioButton(hwnd,IDC_LAUNCH,IDC_TARGET,IDC_TARGET);
          }
          break;


        //case IDC_COMMANDLINE:
        //  {
        //    BOOL bEnableOK = FALSE;
        //    WCHAR tchArgs[MAX_PATH * 2];

        //    if (GetDlgItemText(hwnd,IDC_COMMANDLINE,tchArgs,COUNTOF(tchArgs)))
        //      if (ExtractFirstArgument(tchArgs,tchArgs,NULL))
        //        if (StrIsNotEmpty(tchArgs))
        //          bEnableOK = TRUE;

        //    // OK Button enablen und disablen
        //    EnableWindow(GetDlgItem(hwnd,IDOK),bEnableOK);
        //  }
        //  break;


        case IDC_LAUNCH:
          CheckRadioButton(hwnd,IDC_ALWAYSRUN,IDC_USEDDE,0);
          break;


        case IDC_TARGET:
          CheckRadioButton(hwnd,IDC_ALWAYSRUN,IDC_USEDDE,IDC_ALWAYSRUN);
          break;


        case IDC_TARGETPATH:
          if (HIWORD(wParam) == EN_SETFOCUS)
            CheckRadioButton(hwnd,IDC_LAUNCH,IDC_TARGET,IDC_TARGET);
          break;


        case IDC_ALWAYSRUN:
        case IDC_SENDDROPMSG:
        case IDC_USEDDE:
          CheckRadioButton(hwnd,IDC_LAUNCH,IDC_TARGET,IDC_TARGET);
          break;


        case IDC_DDEMSG:
        case IDC_DDEAPP:
        case IDC_DDETOPIC:
          if (HIWORD(wParam) == EN_SETFOCUS) {
            CheckRadioButton(hwnd,IDC_ALWAYSRUN,IDC_USEDDE,IDC_USEDDE);
            CheckRadioButton(hwnd,IDC_LAUNCH,IDC_TARGET,IDC_TARGET);
          }
          break;


        case IDC_FINDWIN:
          {
            ShowWindow(hwnd, SW_HIDE);
            ShowWindow(hwndMain, SW_HIDE);

            ThemedDialogBoxParam(g_hLngResContainer, MAKEINTRESOURCE(IDD_FINDWIN), hwnd, FindWinDlgProc, (LPARAM)szTargetWndClass);

            ShowWindow(hwndMain, SW_SHOWNORMAL);
            ShowWindow(hwnd, SW_SHOWNORMAL);

            CheckRadioButton(hwnd, IDC_ALWAYSRUN, IDC_USEDDE, IDC_SENDDROPMSG);
            CheckRadioButton(hwnd, IDC_LAUNCH, IDC_TARGET, IDC_TARGET);
          }
          return FALSE;


        case IDOK:
          {
            WCHAR tch[MAX_PATH];
            int i;

            // input validation
            if ((IsDlgButtonChecked(hwnd, IDC_TARGET) && GetDlgItemText(hwnd, IDC_TARGETPATH, tch, COUNTOF(tch)) == 0) ||
              (IsDlgButtonChecked(hwnd, IDC_SENDDROPMSG) && StrIsEmpty(szTargetWndClass)) ||
              (IsDlgButtonChecked(hwnd, IDC_USEDDE) &&
              (GetDlgItemText(hwnd, IDC_DDEMSG, tch, COUNTOF(tch)) == 0 ||
                GetDlgItemText(hwnd, IDC_DDEAPP, tch, COUNTOF(tch)) == 0 ||
                GetDlgItemText(hwnd, IDC_DDETOPIC, tch, COUNTOF(tch)) == 0)))

              ErrorMessage(1, IDS_ERR_INVALIDTARGET);

            else {

              __try {

                LoadIniFileCache(g_wchIniFile);

                const WCHAR* const TargetApp_Section = L"Target Application";

                i = (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_LAUNCH));
                eUseTargetApplication = ((i) ? UTA_UNDEFINED : UTA_LAUNCH_TARGET);
                IniSectionSetInt(TargetApp_Section, L"UseTargetApplication", eUseTargetApplication);

                if (eUseTargetApplication != UTA_UNDEFINED) {
                  GetDlgItemText(hwnd, IDC_TARGETPATH, tch, COUNTOF(tch));
                  ExtractFirstArgument(tch, szTargetApplication, szTargetApplicationParams);
                }
                else {
                  lstrcpy(szTargetApplication, L"");
                  lstrcpy(szTargetApplicationParams, L"");
                }
                IniSectionSetString(TargetApp_Section, L"TargetApplicationPath", szTargetApplication);
                IniSectionSetString(TargetApp_Section, L"TargetApplicationParams", szTargetApplicationParams);

                if (eUseTargetApplication == UTA_UNDEFINED) {
                  eTargetApplicationMode = TAM_ALWAYS_RUN;
                  IniSectionSetInt(TargetApp_Section, L"TargetApplicationMode", eTargetApplicationMode);
                }
                else {
                  if (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_ALWAYSRUN)) {
                    eTargetApplicationMode = TAM_ALWAYS_RUN;
                    IniSectionSetInt(TargetApp_Section, L"TargetApplicationMode", eTargetApplicationMode);
                  }
                  else if (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_SENDDROPMSG)) {
                    eTargetApplicationMode = TAM_SEND_DROP_MSG;
                    IniSectionSetInt(TargetApp_Section, L"TargetApplicationMode", eTargetApplicationMode);
                  }
                  else {
                    eTargetApplicationMode = TAM_SEND_DDE_MSG;
                    IniSectionSetInt(TargetApp_Section, L"TargetApplicationMode", eTargetApplicationMode);
                  }
                }

                if (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_SENDDROPMSG) && !i) {
                  lstrcpy(szTargetApplicationWndClass, szTargetWndClass);
                  IniSectionSetString(TargetApp_Section, L"TargetApplicationWndClass", szTargetApplicationWndClass);
                }
                else {
                  lstrcpy(szTargetApplicationWndClass, L"");
                  IniSectionSetString(TargetApp_Section, L"TargetApplicationWndClass", szTargetApplicationWndClass);
                }

                i = (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_USEDDE));
                if (i)
                  GetDlgItemText(hwnd, IDC_DDEMSG, szDDEMsg, COUNTOF(szDDEMsg));
                else
                  lstrcpy(szDDEMsg, L"");
                IniSectionSetString(TargetApp_Section, L"DDEMessage", szDDEMsg);

                if (i)
                  GetDlgItemText(hwnd, IDC_DDEAPP, szDDEApp, COUNTOF(szDDEApp));
                else
                  lstrcpy(szDDEApp, L"");
                IniSectionSetString(TargetApp_Section, L"DDEApplication", szDDEApp);

                if (i)
                  GetDlgItemText(hwnd, IDC_DDETOPIC, szDDETopic, COUNTOF(szDDETopic));
                else
                  lstrcpy(szDDETopic, L"");
                IniSectionSetString(TargetApp_Section, L"DDETopic", szDDETopic);

              }
              __finally {
                SaveIniFileCache(g_wchIniFile);
              }
              EndDialog(hwnd, IDOK);
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd,IDCANCEL);
          break;

      }

      return TRUE;

  }

  UNUSED(lParam);

  return FALSE;
}


//=============================================================================
//
//  ErrorMessage()
//
int ErrorMessage(int iLevel, UINT uIdMsg, ...)
{

  WCHAR szText[256 * 2] = { L'\0' };
  WCHAR szTitle[256 * 2] = { L'\0' };
  int iIcon;

  if (!GetLngString(uIdMsg, szText, COUNTOF(szText)))
    return(0);

  //int t = wvsprintf(szTitle,szText,(LPVOID)((PUINT_PTR)&uIdMsg + 1));
  int const t = clampi(vswprintf_s(szTitle, COUNTOF(szTitle), szText, (LPVOID)((PUINT_PTR)&uIdMsg + 1)), 0, 1023);
  szTitle[t] = L'\0';

  WCHAR* c = StrChr(szTitle, L'\n');
  if (c)
  {
    lstrcpy(szText, (c + 1));
    *c = '\0';
  }
  else
  {
    lstrcpy(szText, szTitle);
    lstrcpy(szTitle, L"");
  }

  iIcon = (iLevel > 1) ? MB_ICONEXCLAMATION : MB_ICONINFORMATION;

  HWND focus = GetFocus();
  HWND hwnd = focus ? focus : hwndMain;

  return MessageBoxEx(hwnd, szText, szTitle, MB_SETFOREGROUND | iIcon, g_iPrefLANGID);
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
    g_iPrefLANGID,
    (LPTSTR)&lpMsgBuf,
    0, NULL);

  if (lpMsgBuf) {
    // Display the error message and exit the process
    size_t const len = lstrlen((LPCWSTR)lpMsgBuf) + lstrlen(lpszMessage) + 80;

    LPWSTR lpDisplayBuf = LocalAlloc(LPTR, len * sizeof(WCHAR));

    if (lpDisplayBuf) {
      StringCchPrintf(lpDisplayBuf, len, L"Error: '%s' failed with error id %d:\n%s.\n",
        lpszMessage, dwErrID, (LPCWSTR)lpMsgBuf);

      // center message box to main
      HWND focus = GetFocus();
      HWND hwnd = focus ? focus : hwndMain;

      MessageBoxEx(hwnd, lpDisplayBuf, L"MiniPath - ERROR", MB_ICONERROR, g_iPrefLANGID);

      LocalFree(lpDisplayBuf);
    }
    LocalFree(lpMsgBuf); // LocalAlloc()
  }
  return dwErrID;
}


DWORD DbgMsgBoxLastError(LPCWSTR lpszMessage, DWORD dwErrID)
{
#ifdef _DEBUG
  return MsgBoxLastError(lpszMessage, dwErrID);
#else
  UNUSED(lpszMessage);
  return dwErrID;
#endif
}


// End of Dialogs.c
