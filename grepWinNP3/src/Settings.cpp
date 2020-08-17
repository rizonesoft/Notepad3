// grepWin - regex search and replace for Windows

// Copyright (C) 2012-2013, 2016-2020 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "strsafe.h"
#include "resource.h"
#include "maxpath.h"
#include "Settings.h"
#include "BrowseFolder.h"
#include "DirFileEnum.h"
#include "Theme.h"
#include "DarkModeHelper.h"
#include <Commdlg.h>
#include <thread>

inline bool PathIsExistingFile(LPCWSTR pszPath) { return (PathFileExists(pszPath) && !PathIsDirectory(pszPath)); }



//=============================================================================
//
//  IsFontAvailable()
//  Test if a certain font is installed on the system
//
static int CALLBACK EnumFontsProc(CONST LOGFONT* plf, CONST TEXTMETRIC* ptm, DWORD FontType, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(plf);
    UNREFERENCED_PARAMETER(ptm);
    UNREFERENCED_PARAMETER(FontType);
    *((PBOOL)lParam) = true;
    return 0;
}

static inline bool IsFontAvailable(LPCWSTR lpszFontName)
{
    BOOL      fFound = FALSE;
    HDC const hDC    = GetDC(NULL);
    EnumFonts(hDC, lpszFontName, EnumFontsProc, (LPARAM)&fFound);
    ReleaseDC(NULL, hDC);
    return fFound;
}

bool GetLocaleDefaultUIFont(std::wstring langFileName, LPWSTR lpFaceName, WORD& wSize)
{
    LPCWSTR font = L"Segoe UI";
    wSize        = 9;

    std::transform(langFileName.begin(), langFileName.end(), langFileName.begin(), ::towlower);

    if (langFileName.find(L"[zh-cn]") != std::wstring::npos)
    {
        font = L"Microsoft JhengHei UI";
        //wSize = 9;
    }
    else if (langFileName.find(L"[zh-tw]") != std::wstring::npos)
    {
        font = L"Microsoft YaHei UI";
        //wSize = 9;
    }
    else if (langFileName.find(L"[ja-jp]") != std::wstring::npos)
    {
        font  = L"Yu Gothic UI";
        //wSize = 9;
    }
    else if (langFileName.find(L"[ko-kr]") != std::wstring::npos)
    {
        font  = L"Malgun Gothic";
        //wSize = 9;
    }

    bool const isAvail = IsFontAvailable(font);
    if (isAvail)
    {
        StringCchCopy(lpFaceName, LF_FACESIZE, font);
    }
    return isAvail;
}
//=============================================================================


CSettingsDlg::CSettingsDlg(HWND hParent)
    : m_hParent(hParent)
    , m_regEditorCmd(_T("Software\\grepWinNP3\\editorcmd"))
    , m_regEsc(_T("Software\\grepWinNP3\\escclose"), FALSE)
    , m_themeCallbackId(0)
{
}

CSettingsDlg::~CSettingsDlg()
{
}

const wchar_t* const defaultLang  = L"English (United States) [en-US]";
const wchar_t* const stdEditorCmd = _T(".\\Notepad3.exe /%mode% \"%pattern%\" /g %line% - %path%");

LRESULT CSettingsDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            m_themeCallbackId = CTheme::Instance().RegisterThemeChangeCallback(
                [this]() {
                    CTheme::Instance().SetThemeForDialog(*this, CTheme::Instance().IsDarkTheme());
                    CTheme::Instance().SetFontForDialog(*this, CTheme::Instance().GetDlgFontFaceName(), CTheme::Instance().GetDlgFontSize());
                });
            CTheme::Instance().SetThemeForDialog(*this, CTheme::Instance().IsDarkTheme());
            CTheme::Instance().SetFontForDialog(*this, CTheme::Instance().GetDlgFontFaceName(), CTheme::Instance().GetDlgFontSize());
            InitDialog(hwndDlg, IDI_GREPWIN);
            DarkModeHelper::Instance().AllowDarkModeForWindow(GetToolTipHWND(), CTheme::Instance().IsDarkTheme());
            SetWindowTheme(GetToolTipHWND(), L"Explorer", nullptr);

            CLanguage::Instance().TranslateWindow(*this);
            AddToolTip(IDC_ONLYONE, TranslatedString(hResource, IDS_ONLYONE_TT).c_str());

            std::wstring editorCmd = bPortable ? g_iniFile.GetValue(L"global", L"editorcmd", L"") : std::wstring(m_regEditorCmd);
            if (editorCmd.empty()) 
                editorCmd = stdEditorCmd;

            SetDlgItemText(hwndDlg, IDC_EDITORCMD, editorCmd.c_str());

            wchar_t moduledir[MAX_PATH] = {0};
            GetModuleFileName(nullptr, moduledir, MAX_PATH);
            PathRemoveFileSpec(moduledir);

            std::wstring path = moduledir;
            bool bRecurse = false;
            bool bIsDirectory = false;
            CRegStdString regLang(L"Software\\grepWinNP3\\languagefile");
            std::wstring  setLang = regLang;

            if (bPortable)
            {
                wchar_t absLngPath[MAX_PATH] = {0};
                StringCchCopy(absLngPath, MAX_PATH, g_iniFile.GetValue(L"global", L"languagefile", L""));
                if (absLngPath[0] == L'\0')
                {
                    StringCchCopy(absLngPath, MAX_PATH, defaultLang);
                    StringCchCat(absLngPath, MAX_PATH, L".lang");
                }
                if (PathIsRelative(absLngPath))
                {
                    wchar_t tmpPath[MAX_PATH] = {0};
                    StringCchCopy(tmpPath, MAX_PATH, moduledir);
                    PathAppend(tmpPath, absLngPath);
                    StringCchCopy(absLngPath, MAX_PATH, tmpPath);
                }
                
                // need to adapt file enumerator path
                if (PathIsExistingFile(absLngPath))
                {
                    StringCchCopy(moduledir, MAX_PATH, absLngPath);
                    PathRemoveFileSpec(moduledir);
                }
                else 
                    absLngPath[0] = L'\0'; // empty

                setLang = absLngPath;
                path    = moduledir;
            }
            
            // ordered map of language files
            std::wstring                         sPath;
            std::map<std::wstring, std::wstring> langFileMap;
            CDirFileEnum                         fileEnumerator(path.c_str());
            while (fileEnumerator.NextFile(sPath, &bIsDirectory, bRecurse))
            {
                size_t const dotpos = sPath.find_last_of('.');
                if (dotpos == std::wstring::npos)
                    continue;
                std::wstring const ext = sPath.substr(dotpos);
                if (ext.compare(L".lang"))
                    continue;
                size_t const keypos = max(0, dotpos - 7);
                std::wstring const lngKey = sPath.substr(keypos, (dotpos - keypos - 1));
                langFileMap.insert({ lngKey, sPath });
            }

            // clean map and get [en-US] first ...
            int langIndex = -1;
            for (auto it = langFileMap.cbegin();  it != langFileMap.cend() /* not hoisted */; /* no increment */)
            {
                size_t const slashpos = (it->second).find_last_of('\\');
                if (slashpos == std::wstring::npos)
                    it = langFileMap.erase(it);
                else
                {
                    sPath               = (it->second).substr(slashpos + 1);
                    size_t const dotpos = sPath.find_last_of('.');
                    sPath               = sPath.substr(0, dotpos);
                    if (sPath.compare(defaultLang) == 0)
                    {
                        if ((it->second).compare(setLang) == 0)
                          langIndex = 0;
                        m_langpaths.push_front(it->second);
                        SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)sPath.c_str());
                        it = langFileMap.erase(it);
                    }
                    else
                        ++it;
                }
            }

            // build combobox
            auto index = static_cast<int>(m_langpaths.size());
            for (const auto& lang : langFileMap)
            {
                size_t const slashpos = lang.second.find_last_of('\\');
                if (slashpos == std::wstring::npos)
                    continue;
                if ((langIndex < 0) && (lang.second.compare(setLang) == 0))
                    langIndex = index;
                sPath               = lang.second.substr(slashpos + 1);
                size_t const dotpos = sPath.find_last_of('.');
                sPath  = sPath.substr(0, dotpos);
                m_langpaths.push_back(lang.second);
                SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)sPath.c_str());
                ++index;
            }

            if (langIndex < 0) // configured language file not found
            {
                langIndex = 0;
                m_langpaths.push_front(L"");
                SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)0, (LPARAM)defaultLang);
            }

            auto const height = (int)SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_GETITEMHEIGHT, 0, NULL);
            SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_SETITEMHEIGHT, 0, (LPARAM)((int)(height*132)/100));

            SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_SETCURSEL, langIndex, 0);
            SendDlgItemMessage(hwndDlg, IDC_ESCKEY, BM_SETCHECK, bPortable ? g_iniFile.GetBoolValue(L"settings", L"escclose", false) : !!DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\escclose", FALSE)) ? BST_CHECKED : BST_UNCHECKED, 0);
            SendDlgItemMessage(hwndDlg, IDC_BACKUPINFOLDER, BM_SETCHECK, bPortable ? g_iniFile.GetBoolValue(L"settings", L"backupinfolder", false) : !!DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\backupinfolder", FALSE)) ? BST_CHECKED : BST_UNCHECKED, 0);
            SendDlgItemMessage(hwndDlg, IDC_NOWARNINGIFNOBACKUP, BM_SETCHECK, bPortable ? g_iniFile.GetBoolValue(L"settings", L"nowarnifnobackup", false) : !!DWORD(CRegStdDWORD(L"Software\\grepWin\\nowarnifnobackup", FALSE)) ? BST_CHECKED : BST_UNCHECKED, 0);
            SendDlgItemMessage(hwndDlg, IDC_ONLYONE, BM_SETCHECK, bPortable ? g_iniFile.GetBoolValue(L"global", L"onlyone", false) : !!DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\onlyone", FALSE)) ? BST_CHECKED : BST_UNCHECKED, 0);
#ifdef NP3_ALLOW_UPDATE
            SendDlgItemMessage(hwndDlg, IDC_DOUPDATECHECKS, BM_SETCHECK, bPortable ? g_iniFile.GetBoolValue(L"global", L"CheckForUpdates", false) : !!DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\CheckForUpdates", FALSE)) ? BST_CHECKED : BST_UNCHECKED, 0);
#endif
            SendDlgItemMessage(hwndDlg, IDC_DARKMODE, BM_SETCHECK, CTheme::Instance().IsDarkTheme() ? BST_CHECKED : BST_UNCHECKED, 0);
            EnableWindow(GetDlgItem(*this, IDC_DARKMODE), CTheme::Instance().IsDarkModeAllowed());
            SetDlgItemText(*this, IDC_NUMNULL, bPortable ? g_iniFile.GetValue(L"settings", L"nullbytes", L"0") : std::to_wstring(DWORD(CRegStdDWORD(L"Software\\grepWin\\nullbytes", 0))).c_str());

            DWORD const nMaxWorker = std::thread::hardware_concurrency() << 2;
            SendDlgItemMessage(hwndDlg, IDC_SPIN_MAXWORKER, UDM_SETRANGE, 0, MAKELPARAM(nMaxWorker, 1));
            DWORD const nWorker = max(min(bPortable ? g_iniFile.GetLongValue(L"global", L"MaxNumOfWorker", nMaxWorker >> 1) : DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\MaxNumOfWorker", nMaxWorker >> 1)), nMaxWorker), 1);
            wchar_t     number[32];
            StringCchPrintf(number, ARRAYSIZE(number), L"%i", nWorker);
            SendDlgItemMessage(hwndDlg, IDC_MAXNUMWORKER, WM_SETTEXT, 0, (WPARAM)number);

            AddToolTip(IDC_BACKUPINFOLDER, TranslatedString(hResource, IDS_BACKUPINFOLDER_TT).c_str());
            if (!CTheme::Instance().IsDarkModeAllowed())
                SetDlgItemText(*this, IDC_DARKMODEINFO, TranslatedString(hResource, IDS_DARKMODE_TT).c_str());

            m_resizer.Init(hwndDlg);
            m_resizer.UseSizeGrip(!CTheme::Instance().IsDarkTheme());
            m_resizer.AddControl(hwndDlg, IDC_EDITORGROUP, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_EDITORCMD, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_SEARCHPATHBROWSE, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC1, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC2, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC3, RESIZER_TOPLEFT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC4, RESIZER_TOPLEFT);
            m_resizer.AddControl(hwndDlg, IDC_NUMNULL, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_LANGUAGE, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_ESCKEY, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_BACKUPINFOLDER, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_NOWARNINGIFNOBACKUP, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_ONLYONE, RESIZER_TOPLEFTRIGHT);
#ifdef NP3_ALLOW_UPDATE
            m_resizer.AddControl(hwndDlg, IDC_DOUPDATECHECKS, RESIZER_TOPLEFTRIGHT);
#endif
            m_resizer.AddControl(hwndDlg, IDC_DARKMODE, RESIZER_TOPLEFT);
            m_resizer.AddControl(hwndDlg, IDC_DARKMODEINFO, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_TEXT_NUMOFWORKER, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_MAXNUMWORKER, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_SPIN_MAXWORKER, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);
        }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam), HIWORD(wParam));
    case WM_SIZE:
        {
            m_resizer.DoResize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_GETMINMAXINFO:
        {
            auto * mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = m_resizer.GetDlgRectScreen()->right;
            mmi->ptMinTrackSize.y = m_resizer.GetDlgRectScreen()->bottom;
            return 0;
        }
        break;
        case WM_CLOSE:
            CTheme::Instance().RemoveRegisteredCallback(m_themeCallbackId);
            break;
    default:
        return FALSE;
    }
    return FALSE;
}

LRESULT CSettingsDlg::DoCommand(int id, int /*msg*/)
{
    switch (id)
    {
    case IDC_RESETDEFAULT:
            SetDlgItemText(*this, IDC_EDITORCMD, stdEditorCmd);
        break;
    case IDOK:
        {
            auto buf = GetDlgItemText(IDC_EDITORCMD);
            if (bPortable)
                g_iniFile.SetValue(L"global", L"editorcmd", buf.get());
            else
                m_regEditorCmd = buf.get();
            auto               langIndex = static_cast<size_t>(SendDlgItemMessage(*this, IDC_LANGUAGE, CB_GETCURSEL, 0, 0));
            const std::wstring langpath  = (langIndex < m_langpaths.size()) ? m_langpaths[langIndex] : L"";
            if (bPortable)
            {
                WCHAR moduledir[MAX_PATH] = {L'\0'};
                GetModuleFileName(nullptr, moduledir, MAX_PATH);
                PathRemoveFileSpec(moduledir);

                WCHAR absLngPath[MAX_PATH] = {L'\0'};
                StringCchCopy(absLngPath, MAX_PATH, langpath.c_str());
                //~PathCanonicalize(tmp, absLngPath);
                if (PathIsExistingFile(absLngPath))
                {
                    WCHAR relLngPath[MAX_PATH] = {L'\0'};
                    if (PathRelativePathTo(relLngPath, moduledir, FILE_ATTRIBUTE_DIRECTORY, absLngPath, FILE_ATTRIBUTE_NORMAL))
                        g_iniFile.SetValue(L"global", L"languagefile", relLngPath);
                    else
                        g_iniFile.SetValue(L"global", L"languagefile", absLngPath);
                }
                else
                    g_iniFile.DeleteValue(L"global", L"languagefile", nullptr, false);
            }
            else
            {
                CRegStdString regLang(L"Software\\grepWinNP3\\languagefile");
                if (langIndex==0)
                    regLang.removeValue();
                else
                    regLang = langpath;
            }

            WORD fntSize = 9;
            WCHAR fontFaceName[LF_FACESIZE];
            if (GetLocaleDefaultUIFont(langpath, fontFaceName, fntSize)) {
                CTheme::Instance().SetDlgFontFaceName(fontFaceName, fntSize);
            }

            CLanguage::Instance().LoadFile(langpath);
            CLanguage::Instance().TranslateWindow(::GetParent(*this));

            wchar_t worker[32];
            SendDlgItemMessage(*this, IDC_MAXNUMWORKER, WM_GETTEXT, (LPARAM)32, (WPARAM)worker);
            long const nWorker = _wtol((wchar_t*)worker);
            std::wstring sNumNull = GetDlgItemText(IDC_NUMNULL).get();

            if (bPortable)
            {
                g_iniFile.SetBoolValue(L"settings", L"escclose", (IsDlgButtonChecked(*this, IDC_ESCKEY) == BST_CHECKED));
                g_iniFile.SetBoolValue(L"settings", L"backupinfolder", (IsDlgButtonChecked(*this, IDC_BACKUPINFOLDER) == BST_CHECKED));
                g_iniFile.SetBoolValue(L"settings", L"nowarnifnobackup", (IsDlgButtonChecked(*this, IDC_NOWARNINGIFNOBACKUP) == BST_CHECKED));
                g_iniFile.SetBoolValue(L"global", L"onlyone", (IsDlgButtonChecked(*this, IDC_ONLYONE) == BST_CHECKED));
#ifdef NP3_ALLOW_UPDATE
                g_iniFile.SetBoolValue(L"global", L"CheckForUpdates", (IsDlgButtonChecked(*this, IDC_DOUPDATECHECKS) == BST_CHECKED));
#endif
                g_iniFile.SetValue(L"settings", L"nullbytes", sNumNull.c_str());
                g_iniFile.SetLongValue(L"global", L"MaxNumOfWorker", nWorker);
            }
            else
            {
                CRegStdDWORD esc(L"Software\\grepWinNP3\\escclose", FALSE);
                esc = (IsDlgButtonChecked(*this, IDC_ESCKEY) == BST_CHECKED);
                CRegStdDWORD backup(L"Software\\grepWinNP3\\backupinfolder", FALSE);
                backup = (IsDlgButtonChecked(*this, IDC_BACKUPINFOLDER) == BST_CHECKED);
                CRegStdDWORD nowarn(L"Software\\grepWinNP3\\nowarnifnobackup", FALSE);
                nowarn = (IsDlgButtonChecked(*this, IDC_NOWARNINGIFNOBACKUP) == BST_CHECKED);
                CRegStdDWORD regOnlyOne(L"Software\\grepWinNP3\\onlyone", FALSE);
                regOnlyOne = (IsDlgButtonChecked(*this, IDC_ONLYONE) == BST_CHECKED);
#ifdef NP3_ALLOW_UPDATE
                CRegStdDWORD regCheckForUpdates(L"Software\\grepWinNP3\\CheckForUpdates", FALSE);
                regCheckForUpdates = (IsDlgButtonChecked(*this, IDC_DOUPDATECHECKS) == BST_CHECKED);
#endif
                CRegStdDWORD regNumNull(L"Software\\grepWinNP3\\nullbytes", FALSE);
                regNumNull = _wtoi(sNumNull.c_str());
                CRegStdDWORD regNumWorker(L"Software\\grepWinNP3\\MaxNumOfWorker", 1);
                regNumWorker = nWorker;
            }
            CTheme::Instance().SetDarkTheme(IsDlgButtonChecked(*this, IDC_DARKMODE) == BST_CHECKED);
        }
        // fall through
    case IDCANCEL:
        CTheme::Instance().RemoveRegisteredCallback(m_themeCallbackId);
        EndDialog(*this, id);
        break;
    case IDC_SEARCHPATHBROWSE:
        {
            OPENFILENAME ofn = {0};     // common dialog box structure
            TCHAR szFile[MAX_PATH] = {0};  // buffer for file name
            // Initialize OPENFILENAME
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = *this;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = _countof(szFile);
            std::wstring sTitle = TranslatedString(hResource, IDS_SELECTEDITOR);
            ofn.lpstrTitle = sTitle.c_str();
            ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_DONTADDTORECENT;
            auto sProgs = TranslatedString(hResource, IDS_PROGRAMS);
            auto sAllFiles = TranslatedString(hResource, IDS_ALLFILES);
            auto sFilter = sProgs;
            sFilter.append(L"\0*.exe;*.com\0", _countof(L"\0*.exe;*.com\0")-1);
            sFilter.append(sAllFiles);
            sFilter.append(L"\0*.*\0\0", _countof(L"\0*.*\0\0")-1);
            ofn.lpstrFilter = sFilter.c_str();
            ofn.nFilterIndex = 1;
            // Display the Open dialog box.
            if (GetOpenFileName(&ofn)==TRUE)
            {
                SetDlgItemText(*this, IDC_EDITORCMD, szFile);
            }
        }
        break;
    }
    return 1;
}
