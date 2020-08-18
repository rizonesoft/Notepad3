// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2010-2020 - Stefan Kueng

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
#include "SearchDlg.h"
#include "AboutDlg.h"
#include "CmdLineParser.h"
#include "Registry.h"
#include "Language.h"
#include "StringUtils.h"
#include "PathUtils.h"
#pragma warning(push)
#pragma warning(disable : 4458) // declaration of 'xxx' hides class member
#include <gdiplus.h>
#include "WinUser.h"
#pragma warning(pop)

// Global Variables:
HINSTANCE g_hInst;            // current instance
HICON     g_hDlgIcon128;

bool bPortable = false;
CSimpleIni g_iniFile;
HANDLE     hInitProtection = nullptr;

ULONGLONG g_startTime = GetTickCount64();
UINT      GREPWIN_STARTUPMSG = RegisterWindowMessage(L"grepWinNP3_StartupMessage");

static std::wstring SanitizeSearchPaths(const std::wstring& searchpath)
{
    std::vector<std::wstring> container;
    stringtok(container, searchpath, true);
    std::wstring sResult;
    for (auto path : container)
    {
        if (!sResult.empty())
            sResult += L"|";
        size_t endpos = path.find_last_not_of(L" \"\t");
        if (std::wstring::npos != endpos)
        {
            path = path.substr(0, endpos + 1);
        }
        size_t startpos = path.find_first_not_of(L" \"\t");
        if ((startpos > 0) && (std::wstring::npos != startpos))
        {
            path = path.substr(startpos);
        }
        sResult += CPathUtils::GetLongPathname(path);
    }
    return sResult;
}

static void RegisterContextMenu(bool bAdd)
{
    if (bAdd)
    {
        std::wstring sIconPath = CStringUtils::Format(L"%s,-%d", CPathUtils::GetLongPathname(CPathUtils::GetModulePath()).c_str(), IDI_GREPWIN);
        std::wstring sExePath = CStringUtils::Format(L"%s /searchpath:\"%%1\"", CPathUtils::GetLongPathname(CPathUtils::GetModulePath()).c_str());
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\shell\\grepWinNP3", nullptr, REG_SZ, L"search with grepWinNP3", sizeof(L"search with grepWinNP3") + 2);
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\shell\\grepWinNP3", L"Icon", REG_SZ, sIconPath.c_str(), DWORD((sIconPath.size() + 1) * sizeof(WCHAR)));
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\shell\\grepWinNP3\\Command", nullptr, REG_SZ, sExePath.c_str(), DWORD((sExePath.size() + 1) * sizeof(WCHAR)));
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\Background\\shell\\grepWinNP3", nullptr, REG_SZ, L"search with grepWinNP3", sizeof(L"search with grepWinNP3") + 2);
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\Background\\shell\\grepWinNP3", L"Icon", REG_SZ, sIconPath.c_str(), DWORD((sIconPath.size() + 1) * sizeof(WCHAR)));

        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Folder\\shell\\grepWinNP3", nullptr, REG_SZ, L"search with grepWinNP3", sizeof(L"search with grepWinNP3") + 2);
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Folder\\shell\\grepWinNP3", L"Icon", REG_SZ, sIconPath.c_str(), DWORD((sIconPath.size() + 1) * sizeof(WCHAR)));
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Folder\\shell\\grepWinNP3\\Command", nullptr, REG_SZ, sExePath.c_str(), DWORD((sExePath.size() + 1) * sizeof(WCHAR)));
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Drive\\shell\\grepWinNP3", nullptr, REG_SZ, L"search with grepWinNP3", sizeof(L"search with grepWinNP3") + 2);
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Drive\\shell\\grepWinNP3", L"Icon", REG_SZ, sIconPath.c_str(), DWORD((sIconPath.size() + 1) * sizeof(WCHAR)));
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Drive\\shell\\grepWinNP3\\Command", nullptr, REG_SZ, sExePath.c_str(), DWORD((sExePath.size() + 1) * sizeof(WCHAR)));
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\grepWinNP3", nullptr, REG_SZ, L"search with grepWinNP3", sizeof(L"search with grepWinNP3") + 2);
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\grepWinNP3", L"Icon", REG_SZ, sIconPath.c_str(), DWORD((sIconPath.size() + 1) * sizeof(WCHAR)));
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\grepWinNP3\\Command", nullptr, REG_SZ, sExePath.c_str(), DWORD((sExePath.size() + 1) * sizeof(WCHAR)));
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\grepWinNP3", L"MultiSelectModel", REG_SZ, L"Player", sizeof(L"Player") + 2);

        sExePath = CStringUtils::Format(L"%s /searchpath:\"%%V\"", CPathUtils::GetLongPathname(CPathUtils::GetModulePath()).c_str());
        SHSetValue(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\Background\\shell\\grepWinNP3\\Command", nullptr, REG_SZ, sExePath.c_str(), DWORD((sExePath.size() + 1) * sizeof(WCHAR)));
    }
    else
    {
        SHDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\shell\\grepWinNP3");
        SHDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\Directory\\Background\\shell\\grepWinNP3");
        SHDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\Folder\\shell\\grepWinNP3");
        SHDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\Drive\\shell\\grepWinNP3");
        SHDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\grepWinNP3");
    }
}



BOOL CALLBACK windowenumerator(__in  HWND hwnd,__in  LPARAM lParam)
{
    auto * pWnd = (HWND*)lParam;
    WCHAR buf[MAX_PATH] = {0};
    GetWindowText(hwnd, buf, _countof(buf));
    if (_wcsnicmp(buf, L"grepwinnp3 :", 12) == 0) {
        *pWnd = hwnd;
        if (SendMessage(hwnd, GREPWIN_STARTUPMSG, 1, 0))
        {
            // grepWin instance started moments ago, so use this one
            return FALSE;
        }
    }
    return TRUE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // uncomment the following lines for low-memory tests.
    // note: process needs to run elevated for this to work.
    //
    //auto job = CreateJobObject(NULL, NULL);
    //JOBOBJECT_EXTENDED_LIMIT_INFORMATION joblimit = { 0 };
    //joblimit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_WORKINGSET;
    //joblimit.JobMemoryLimit = 30 * 1024 * 1024;
    //joblimit.ProcessMemoryLimit = 30 * 1024 * 1024;
    //joblimit.PeakProcessMemoryUsed = 30 * 1024 * 1024;
    //joblimit.BasicLimitInformation.MaximumWorkingSetSize = 30 * 1024 * 1024;
    //joblimit.BasicLimitInformation.MinimumWorkingSetSize = 30 * 1024;
    //SetInformationJobObject(job, JobObjectExtendedLimitInformation, &joblimit, sizeof(joblimit));
    //AssignProcessToJobObject(job, GetCurrentProcess());

    SetDllDirectory(L"");
    // if multiple items are selected in explorer and grepWin is started for all of them,
    // explorer starts multiple grepWin instances at once. In case there's already a grepWin instance
    // running, sleep for a while to give that instance time to fully initialize
    HANDLE hReloadProtection = ::CreateMutex(nullptr, FALSE, L"{6473AA76-0EAE-4C96-8C99-AFDFEFFE42B5}");
    bool alreadyRunning = false;
    if ((!hReloadProtection) || (GetLastError() == ERROR_ALREADY_EXISTS))
    {
        // An instance of grepWin is already running
        alreadyRunning = true;
    }
    hInitProtection     = ::CreateMutex(NULL, FALSE, L"{6473AA76-0EAE-4C96-8C99-AFDFEFFE42B6}");
    bool initInProgress = false;
    if ((!hInitProtection) || (GetLastError() == ERROR_ALREADY_EXISTS))
    {
        // An instance of grepWin is initializing
        initInProgress = true;
    }

    g_hInst = hInstance;
    ::OleInitialize(nullptr);
    ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // we need some of the common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_LINK_CLASS | ICC_LISTVIEW_CLASSES | ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    g_hDlgIcon128 = NULL;
    LoadIconWithScaleDown(hInstance, MAKEINTRESOURCE(IDI_GREPWIN), 128, 128, &g_hDlgIcon128);

    HMODULE hRichEdt = LoadLibrary(L"Riched20.dll");

    CCmdLineParser parser(lpCmdLine);

    //MessageBox(NULL, L"", L"", MB_OK);

    if (parser.HasKey(L"register"))
    {
        RegisterContextMenu(true);
        return FALSE;
    }
    if ((parser.HasKey(L"unregister")) || (parser.HasKey(L"deregister")))
    {
        RegisterContextMenu(false);
        return FALSE;
    }

    bool bQuit = false;
    HWND hWnd = nullptr;
    int timeout = 20;
    // find already running grepWin windows
    if (alreadyRunning)
    {
        do
        {
            if (EnumWindows(windowenumerator, (LPARAM)&hWnd) != FALSE)
            {
                // long running grepWin Window found:
                // if a grepWin process is currently initializing,
                // wait a while and enumerate again
                while (initInProgress)
                {
                    CloseHandle(hInitProtection);
                    Sleep(100);
                    initInProgress = false;
                    hInitProtection = ::CreateMutex(NULL, FALSE, L"{6473AA76-0EAE-4C96-8C99-AFDFEFFE42B6}");
                    if ((!hInitProtection) || (GetLastError() == ERROR_ALREADY_EXISTS))
                    {
                        // An instance of grepWin is still initializing
                        initInProgress = true;
                    }
                }
                hWnd = nullptr;
                EnumWindows(windowenumerator, (LPARAM)&hWnd);
            }
            if (alreadyRunning && (hWnd == nullptr))
                Sleep(100);
            timeout--;
        } while ((hWnd == nullptr) && alreadyRunning && timeout);
    }
    auto modulename = CPathUtils::GetFileName(CPathUtils::GetModulePath(nullptr));
    bPortable       = ((_tcsstr(modulename.c_str(), L"portable")) || 
                       (_tcsstr(modulename.c_str(), L"NP3")) || 
                       (parser.HasKey(L"portable")));

    std::wstring iniPath = CPathUtils::GetModuleDir(nullptr);
    iniPath += L"\\grepWinNP3.ini";

    if (parser.HasVal(L"inipath"))
        iniPath = parser.GetVal(L"inipath");

    if (bPortable)
    {
        if (PathIsRelative(iniPath.c_str()))
        {
            WCHAR absPath[MAX_PATH] = {L'\0'};
            StringCchCopy(absPath, MAX_PATH, CPathUtils::GetModuleDir(nullptr).c_str());
            PathAppend(absPath, iniPath.c_str());
            iniPath = absPath;
        }
        g_iniFile.SetUnicode();
        g_iniFile.SetMultiLine();
        g_iniFile.SetSpaces(false);
        g_iniFile.LoadFile(iniPath.c_str());
    }

    if (hWnd)
    {
        bool bOnlyOne = bPortable ? g_iniFile.GetBoolValue(L"global", L"onlyone", L"false") : 
                                    !!DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\onlyone", 0));
        bool bCopyData = false;
        if (SendMessage(hWnd, GREPWIN_STARTUPMSG, 1, 0)) // check if grepWin was started moments ago
        {
            SendMessage(hWnd, GREPWIN_STARTUPMSG, 0, 0); // reset the timer
            bCopyData = true;
            bQuit     = true;
        }
        else if (bOnlyOne)
        {
            bCopyData = true;
            bQuit     = true;
        }
        if (bCopyData) {
            // grepWin was started just moments ago:
            // add the new path to the existing search path in that grepWin instance
            std::wstring searchfor = parser.HasVal(L"searchfor") ?
                                     parser.GetVal(L"searchfor") :
                                     (bPortable ? g_iniFile.GetValue(L"global", L"searchfor", L"") : L"");

            std::wstring spath     = parser.HasVal(L"searchpath") ? parser.GetVal(L"searchpath") : 
                (bPortable ? g_iniFile.GetValue(L"global", L"searchpath", L"") : L"");
            SearchReplace(spath, L"/", L"\\");
            spath = SanitizeSearchPaths(spath);

            CopyData_t data2copy = {0};
            StringCchCopyW(data2copy.searchFor, _countof(data2copy.searchFor), searchfor.c_str());
            StringCchCopyW(data2copy.searchPath, _countof(data2copy.searchPath), spath.c_str());

            COPYDATASTRUCT CopyData = {0};
            CopyData.dwData         = (DWORD)GREPWINNP3_CPYDAT;
            CopyData.lpData         = (LPVOID)&data2copy;
            CopyData.cbData         = (DWORD)sizeof(CopyData_t);
            SendMessage(hWnd, WM_COPYDATA, (WPARAM)(HWND)NULL, (LPARAM)&CopyData);
            SetForegroundWindow(hWnd); //set the window to front
        }
    }

    int ret = 0;
    if (!bQuit)
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR                    gdiplusToken;
        // Initialize GDI+.
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        if (bPortable)
        {
            std::wstring languagefile = g_iniFile.GetValue(L"global", L"languagefile", L"");
            if (PathIsRelative(languagefile.c_str()))
            {
                TCHAR wchAppPath[MAX_PATH] = {L'\0'};
                GetModuleFileName(nullptr, wchAppPath, MAX_PATH);
                PathRemoveFileSpec(wchAppPath);
                PathAppend(wchAppPath, languagefile.c_str());
                TCHAR wchLngPath[MAX_PATH] = {L'\0'};
                PathCanonicalize(wchLngPath, wchAppPath);
                CLanguage::Instance().LoadFile(wchLngPath);
            }
            else
                CLanguage::Instance().LoadFile(languagefile);
        }
        else
            CLanguage::Instance().LoadFile(std::wstring(CRegStdString(L"Software\\grepWinNP3\\languagefile")));

        if (parser.HasKey(L"about")||parser.HasKey(_T("?"))||parser.HasKey(L"help"))
        {
            if (hInitProtection)
                CloseHandle(hInitProtection);
            CAboutDlg aboutDlg(nullptr);
            ret= (int)aboutDlg.DoModal(hInstance, IDD_ABOUT, nullptr, NULL);
        }
        else
        {
            CSearchDlg   searchDlg(nullptr);

            std::wstring spath = parser.HasVal(L"searchpath") ? parser.GetVal(L"searchpath") : 
              (bPortable ? g_iniFile.GetValue(L"global", L"searchpath", L"") : L"");
            SearchReplace(spath, L"/", L"\\");
            spath = SanitizeSearchPaths(spath);
            searchDlg.SetSearchPath(spath);

            std::wstring searchfor = parser.HasVal(L"searchfor") ? parser.GetVal(L"searchfor") : 
              (bPortable ? g_iniFile.GetValue(L"global", L"searchfor", L"") : L"");
            searchDlg.SetSearchString(searchfor);

            if (parser.HasVal(L"filemaskregex"))
                searchDlg.SetFileMask(parser.GetVal(L"filemaskregex"), true);
            if (parser.HasVal(L"filemask"))
                searchDlg.SetFileMask(parser.GetVal(L"filemask"), false);
            if (parser.HasVal(L"direxcluderegex"))
                searchDlg.SetDirExcludeRegexMask(parser.GetVal(L"direxcluderegex"));
            else if (parser.HasVal(L"filemaskexclude"))
                searchDlg.SetDirExcludeRegexMask(parser.GetVal(L"filemaskexclude"));
            if (parser.HasVal(L"replacewith"))
                searchDlg.SetReplaceWith(parser.GetVal(L"replacewith"));
            if (parser.HasVal(L"preset"))
                searchDlg.SetPreset(parser.GetVal(L"preset"));

            if (parser.HasVal(L"i"))
                searchDlg.SetCaseSensitive(_tcsicmp(parser.GetVal(L"i"), L"yes")!=0);
            if (parser.HasVal(L"n"))
                searchDlg.SetMatchesNewline(_tcsicmp(parser.GetVal(L"n"), L"yes")==0);
            if (parser.HasVal(L"k"))
                searchDlg.SetCreateBackups(_tcsicmp(parser.GetVal(L"k"), L"yes")==0);
            if (parser.HasVal(L"utf8"))
                searchDlg.SetUTF8(_tcsicmp(parser.GetVal(L"utf8"), L"yes")==0);
            if (parser.HasVal(L"binary"))
                searchDlg.SetBinary(_tcsicmp(parser.GetVal(L"binary"), L"yes") == 0);
            if (parser.HasVal(L"size"))
            {
                int cmp = 0;
                if (parser.HasVal(L"sizecmp"))
                    cmp = parser.GetLongVal(L"sizecmp");
                searchDlg.SetSize(parser.GetLongVal(L"size"), cmp);
            }
            if (parser.HasVal(L"s"))
                searchDlg.SetIncludeSystem(_tcsicmp(parser.GetVal(L"s"), L"yes")==0);
            if (parser.HasVal(L"h"))
                searchDlg.SetIncludeHidden(_tcsicmp(parser.GetVal(L"h"), L"yes")==0);
            if (parser.HasVal(L"u"))
                searchDlg.SetIncludeSubfolders(_tcsicmp(parser.GetVal(L"u"), L"yes")==0);
            if (parser.HasVal(L"b"))
                searchDlg.SetIncludeBinary(_tcsicmp(parser.GetVal(L"b"), L"yes")==0);
            if (parser.HasVal(L"regex"))
                searchDlg.SetUseRegex(_tcsicmp(parser.GetVal(L"regex"), L"yes") == 0);
            else if(parser.HasVal(L"searchfor"))
                searchDlg.SetUseRegex(true);

            if (parser.HasKey(L"execute") || parser.HasKey(L"executesearch"))
                searchDlg.SetExecute(ExecuteAction::Search);
            if (parser.HasKey(L"executereplace"))
                searchDlg.SetExecute(ExecuteAction::Replace);
            if (parser.HasKey(L"closedialog"))
                searchDlg.SetEndDialog();
            if (parser.HasKey(L"content"))
                searchDlg.SetShowContent();
            if (parser.HasVal(L"datelimit") && parser.HasVal(L"date1"))
            {
                FILETIME date1 = { 0 };
                FILETIME date2 = { 0 };
                int year = 0;
                int month = 0;
                int day = 0;
                auto sDate1 = parser.GetVal(L"date1");
                swscanf_s(sDate1, L"%4d:%2d:%2d", &year, &month, &day);
                SYSTEMTIME sysTime = { 0 };
                sysTime.wYear = (WORD)year;
                sysTime.wMonth = (WORD)month;
                sysTime.wDay = (WORD)day;
                SystemTimeToFileTime(&sysTime, &date1);
                if (parser.HasVal(L"date2"))
                {
                    auto sDate2 = parser.GetVal(L"date2");
                    swscanf_s(sDate2, L"%4d:%2d:%2d", &year, &month, &day);
                    sysTime.wYear = (WORD)year;
                    sysTime.wMonth = (WORD)month;
                    sysTime.wDay = (WORD)day;
                    SystemTimeToFileTime(&sysTime, &date2);
                }
                searchDlg.SetDateLimit(parser.GetLongVal(L"datelimit"), date1, date2);
            }

            if (!parser.HasVal(L"searchpath"))
            {
                auto cmdLineSize = wcslen(lpCmdLine);
                auto cmdLinePath = std::make_unique<wchar_t[]>(cmdLineSize + 1);
                wcscpy_s(cmdLinePath.get(), cmdLineSize + 1, lpCmdLine);
                PathUnquoteSpaces(cmdLinePath.get());
                if (PathFileExists(cmdLinePath.get()))
                {
                    spath = cmdLinePath.get();
                    spath = SanitizeSearchPaths(spath);
                    searchDlg.SetSearchPath(spath);
                }
            }

            ret = (int)searchDlg.DoModal(hInstance, IDD_SEARCHDLG, NULL, IDR_SEARCHDLG);
        }
        if (bPortable)
            g_iniFile.SaveFile(iniPath.c_str(), true);
        
        Gdiplus::GdiplusShutdown(gdiplusToken);
    }

    ::CoUninitialize();
    ::OleUninitialize();
    FreeLibrary(hRichEdt);
    CloseHandle(hReloadProtection);
    return ret;
}
