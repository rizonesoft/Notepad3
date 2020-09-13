// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2020 - Stefan Kueng

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
#pragma once
#include "BaseDialog.h"
#include "SearchInfo.h"
#include "BookmarksDlg.h"
#include "DlgResizer.h"
#include "FileDropTarget.h"
#include "AutoComplete.h"
#include "Registry.h"
#include "hyperlink.h"
#include "EditDoubleClick.h"
#include "StringUtils.h"
#include <string>
#include <vector>
#include <set>
#include <mutex>
#ifdef NP3_ALLOW_UPDATE
#include <thread>
#endif


#define SEARCH_START         (WM_APP+1)
#define SEARCH_PROGRESS      (WM_APP+2)
#define SEARCH_END           (WM_APP+3)
#define WM_GREPWIN_THREADEND (WM_APP+4)

#define ID_ABOUTBOX          0x0010

enum ExecuteAction
{
    None,
    Search,
    Replace
};

typedef struct _SearchFlags_t
{
    bool bSearchAlways;
    bool bUTF8;
    bool bForceBinary;
    bool bIncludeBinary;
    bool bUseRegex;
    bool bCaseSensitive;
    bool bDotMatchesNewline;
    bool bCreateBackup;
    bool bBackupInFolder;
    bool bReplace;
    bool bCaptureSearch;

} SearchFlags_t;

#define GREPWINNP3_CPYDAT 4711
typedef struct _CopyData_t
{
    wchar_t searchFor[1024];
    wchar_t searchPath[MAX_PATH << 8];

} CopyData_t;


/**
 * search dialog.
 */
class CSearchDlg : public CDialog
{
public:
    CSearchDlg(HWND hParent);
    ~CSearchDlg();

    DWORD                   SearchThread();
    DWORD                   EvaluationThread();
    void                    SetSearchPath(const std::wstring& path) {m_searchpath = path; SearchReplace(m_searchpath, L"/", L"\\"); }
    void                    SetSearchString(const std::wstring& search) {m_searchString = search;}
    void                    SetFileMask(const std::wstring& mask, bool reg) {m_patternregex = mask; m_bUseRegexForPaths = reg;}
    void                    SetDirExcludeRegexMask(const std::wstring& mask) {m_excludedirspatternregex = mask;}
    void                    SetReplaceWith(const std::wstring& replace) { m_replaceString = replace; }
    void                    SetUseRegex(bool reg) { m_bUseRegex = reg; }
    void                    SetPreset(const std::wstring& preset);

    void                    SetCaseSensitive(bool bSet) {m_bCaseSensitiveC = true; m_bCaseSensitive = bSet;}
    void                    SetMatchesNewline(bool bSet) {m_bDotMatchesNewlineC = true; m_bDotMatchesNewline = bSet;}
    void                    SetCreateBackups(bool bSet) { m_bCreateBackupC = true; m_bCreateBackup = bSet; m_bConfirmationOnReplace = false; }
    void                    SetCreateBackupsInFolders(bool bSet) { m_bCreateBackupInFoldersC = true; m_bCreateBackupInFolders = bSet; SetCreateBackups(bSet); }
    void                    SetUTF8(bool bSet) { m_bUTF8C = true; m_bUTF8 = bSet; m_bForceBinary = false; }
    void                    SetBinary(bool bSet) { m_bUTF8C = true; m_bForceBinary = bSet; m_bUTF8 = false; }
    void                    SetSize(uint64_t size, int cmp) {m_bSizeC = true; m_lSize = size; m_sizeCmp = cmp; m_bAllSize = (size == (uint64_t)-1);}
    void                    SetIncludeSystem(bool bSet) {m_bIncludeSystemC = true; m_bIncludeSystem = bSet;}
    void                    SetIncludeHidden(bool bSet) {m_bIncludeHiddenC = true; m_bIncludeHidden = bSet;}
    void                    SetIncludeSubfolders(bool bSet) {m_bIncludeSubfoldersC = true; m_bIncludeSubfolders = bSet;}
    void                    SetIncludeBinary(bool bSet) {m_bIncludeBinaryC = true; m_bIncludeBinary = bSet;}
    void                    SetDateLimit(int datelimit, FILETIME t1, FILETIME t2) { m_bDateLimitC = true; m_DateLimit = datelimit; m_Date1 = t1; m_Date2 = t2; }

    void                    SetExecute(ExecuteAction execute) {m_ExecuteImmediately = execute;}
    void                    SetEndDialog() { m_endDialog = true; }
    void                    SetShowContent() { m_showContent = true; m_showContentSet = true; }
protected:
    LRESULT CALLBACK        DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT                 DoCommand(int id, int msg);
    bool                    PreTranslateMessage(MSG* pMsg) override;

    static int              SearchFile(std::shared_ptr<CSearchInfo> sinfoPtr, const std::wstring& searchRoot, const SearchFlags_t searchFlags,
                                       const std::wstring& searchString, const std::wstring& searchStringUtf16le, const std::wstring& replaceString);

    bool                    InitResultList();
    void                    FillResultList();
    bool                    AddFoundEntry(const CSearchInfo * pInfo, bool bOnlyListControl = false);
    void                    ShowContextMenu(int x, int y);
    void                    DoListNotify(LPNMITEMACTIVATE lpNMItemActivate);
    void                    OpenFileAtListIndex(int listIndex);
    void                    UpdateInfoLabel();
    bool                    SaveSettings();
    void                    SaveWndPosition();
    void                    formatDate(wchar_t date_native[], const FILETIME& filetime, bool force_short_fmt);
    int                     CheckRegex();
    bool                    MatchPath(LPCTSTR pathbuf);
    void                    AutoSizeAllColumns();
    int                     GetSelectedListIndex(int index);
    bool                    FailedShowMessage(HRESULT hr);
#ifdef NP3_ALLOW_UPDATE
    void                    CheckForUpdates(bool force = false);
    void                    ShowUpdateAvailable();
    bool                    IsVersionNewer(const std::wstring& sVer);
#endif
private:
    static bool             NameCompareAsc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             SizeCompareAsc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             MatchesCompareAsc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             PathCompareAsc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             EncodingCompareAsc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             ModifiedTimeCompareAsc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             ExtCompareAsc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);

    static bool             NameCompareDesc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             SizeCompareDesc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             MatchesCompareDesc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             PathCompareDesc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             EncodingCompareDesc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             ModifiedTimeCompareDesc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);
    static bool             ExtCompareDesc(const CSearchInfo& Entry1, const CSearchInfo& Entry2);

private:
    HWND                    m_hParent;
    CBookmarksDlg *         m_pBookmarksDlg;

    std::wstring            m_searchpath;
    std::wstring            m_searchString;
    std::wstring            m_replaceString;
    std::vector<std::wstring> m_patterns;
    std::wstring            m_patternregex;
    std::wstring            m_excludedirspatternregex;
    bool                    m_bUseRegex;
    bool                    m_bUseRegexForPaths;
    bool                    m_bAllSize;
    uint64_t                m_lSize;
    int                     m_sizeCmp;
    bool                    m_bIncludeSystem;
    bool                    m_bIncludeSystemC;
    bool                    m_bIncludeHidden;
    bool                    m_bIncludeHiddenC;
    bool                    m_bIncludeSubfolders;
    bool                    m_bIncludeSubfoldersC;
    bool                    m_bIncludeBinary;
    bool                    m_bIncludeBinaryC;
    bool                    m_bCreateBackup;
    bool                    m_bCreateBackupC;
    bool                    m_bCreateBackupInFolders;
    bool                    m_bCreateBackupInFoldersC;
    bool                    m_bUTF8;
    bool                    m_bUTF8C;
    bool                    m_bForceBinary;
    bool                    m_bCaseSensitive;
    bool                    m_bCaseSensitiveC;
    bool                    m_bDotMatchesNewline;
    bool                    m_bDotMatchesNewlineC;
    bool                    m_bNOTSearch;
    bool                    m_bCaptureSearch;
    bool                    m_bSizeC;
    bool                    m_endDialog;
    ExecuteAction           m_ExecuteImmediately;
    int                     m_DateLimit;
    bool                    m_bDateLimitC;
    FILETIME                m_Date1;
    FILETIME                m_Date2;

    bool                    m_bReplace;
    bool                    m_bConfirmationOnReplace;
    bool                    m_showContent;
    bool                    m_showContentSet;

    std::vector<CSearchInfo> m_items;
    std::vector<std::tuple<int, int>> m_listItems;

    int                     m_totalitems;
    int                     m_searchedItems;
    int                     m_totalmatches;
    bool                    m_bAscending;
    std::wstring            m_resultString;

    CDlgResizer             m_resizer;
    CHyperLink              m_link;
    int                     m_themeCallbackId;

    CFileDropTarget *       m_pDropTarget;

    static UINT             GREPWIN_STARTUPMSG;

#ifdef NP3_ALLOW_UPDATE
   std::thread             m_updateCheckThread;
#endif

    CAutoComplete           m_AutoCompleteFilePatterns;
    CAutoComplete           m_AutoCompleteExcludeDirsPatterns;
    CAutoComplete           m_AutoCompleteSearchPatterns;
    CAutoComplete           m_AutoCompleteReplacePatterns;
    CAutoComplete           m_AutoCompleteSearchPaths;

    CEditDoubleClick        m_editFilePatterns;
    CEditDoubleClick        m_editExcludeDirsPatterns;
    CEditDoubleClick        m_editSearchPatterns;
    CEditDoubleClick        m_editReplacePatterns;
    CEditDoubleClick        m_editSearchPaths;

    CRegStdDWORD            m_regUseRegex;
    CRegStdDWORD            m_regAllSize;
    CRegStdString           m_regSize;
    CRegStdDWORD            m_regSizeCombo;
    CRegStdDWORD            m_regIncludeSystem;
    CRegStdDWORD            m_regIncludeHidden;
    CRegStdDWORD            m_regIncludeSubfolders;
    CRegStdDWORD            m_regIncludeBinary;
    CRegStdDWORD            m_regCreateBackup;
    CRegStdDWORD            m_regUTF8;
    CRegStdDWORD            m_regBinary;
    CRegStdDWORD            m_regCaseSensitive;
    CRegStdDWORD            m_regDotMatchesNewline;
    CRegStdDWORD            m_regUseRegexForPaths;
    CRegStdString           m_regPattern;
    CRegStdString           m_regExcludeDirsPattern;
    CRegStdString           m_regSearchPath;
    CRegStdString           m_regEditorCmd;
    CRegStdDWORD            m_regBackupInFolder;
    CRegStdDWORD            m_regDateLimit;
    CRegStdDWORD            m_regDate1Low;
    CRegStdDWORD            m_regDate1High;
    CRegStdDWORD            m_regDate2Low;
    CRegStdDWORD            m_regDate2High;
    CRegStdDWORD            m_regShowContent;
};
