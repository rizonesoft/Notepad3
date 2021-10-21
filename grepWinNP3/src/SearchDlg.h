// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2021 - Stefan Kueng

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
#include "EditDoubleClick.h"
#include "StringUtils.h"
#include <string>
#include <vector>
#include <set>
#include <mutex>
#ifdef NP3_ALLOW_UPDATE
#include <thread>
#endif

#include <wrl.h>
using namespace Microsoft::WRL;

#define SEARCH_START         (WM_APP+1)
#define SEARCH_PROGRESS      (WM_APP+2)
#define SEARCH_END           (WM_APP+3)
#define WM_GREPWIN_THREADEND (WM_APP+4)

#define ID_ABOUTBOX         0x0010
#define ID_CLONE            0x0011

#define ID_STAY_ON_TOP      0x0022

#define ALPHA_OPAQUE         (255)

enum class ExecuteAction
{
    None,
    Search,
    Replace,
    Capture
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
    bool bWholeWords;
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
    ~CSearchDlg() override;

    DWORD SearchThread();
    DWORD EvaluationThread();

    void  SetSearchPath(const std::wstring& path);
    void  SetFileMask(const std::wstring& mask, bool reg);
    void  SetPreset(const std::wstring& preset);

    inline void  SetSearchString(const std::wstring& search) { m_searchString = search; }
    inline void  SetDirExcludeRegexMask(const std::wstring& mask) { m_excludeDirsPatternRegex = mask; }
    inline void  SetReplaceWith(const std::wstring& replace) { m_replaceString = replace; }
    inline void  SetUseRegex(bool reg) { m_bUseRegex = reg; }

    inline void  SetCaseSensitive(bool bSet) { m_bCaseSensitiveC = true; m_bCaseSensitive = bSet; }
    inline void  SetMatchesNewline(bool bSet) { m_bDotMatchesNewlineC = true; m_bDotMatchesNewline = bSet; }
    inline void  SetCreateBackups(bool bSet) { m_bCreateBackupC = true; m_bCreateBackup = bSet; m_bConfirmationOnReplace = false; }
    inline void  SetCreateBackupsInFolders(bool bSet) { m_bCreateBackupInFoldersC = true; m_bCreateBackupInFolders = bSet; SetCreateBackups(bSet); }
    inline void  SetWholeWords(bool bSet) { m_bWholeWordsC = true; m_bWholeWords = bSet; }
    inline void  SetUTF8(bool bSet) { m_bUTF8C = true; m_bUTF8 = bSet; m_bForceBinary = false; }
    inline void  SetBinary(bool bSet) { m_bUTF8C = true; m_bForceBinary = bSet; m_bUTF8 = false; }
    inline void  SetSize(uint64_t size, int cmp) { m_bSizeC = true; m_lSize = size; m_sizeCmp = cmp; m_bAllSize = (size == (uint64_t)0); }
    inline void  SetIncludeSystem(bool bSet) { m_bIncludeSystemC = true; m_bIncludeSystem = bSet; }
    inline void  SetIncludeHidden(bool bSet) { m_bIncludeHiddenC = true; m_bIncludeHidden = bSet; }
    inline void  SetIncludeSubfolders(bool bSet) { m_bIncludeSubfoldersC = true; m_bIncludeSubfolders = bSet; }
    inline void  SetIncludeBinary(bool bSet) { m_bIncludeBinaryC = true; m_bIncludeBinary = bSet; }
    inline void  SetDateLimit(int datelimit, FILETIME t1, FILETIME t2) { m_bDateLimitC = true; m_dateLimit = datelimit; m_date1 = t1; m_date2 = t2; }
    inline void  SetNoSaveSettings(bool nosave) { m_bNoSaveSettings = nosave; }

    inline void  SetExecute(ExecuteAction execute) { m_executeImmediately = execute; }
    inline void  SetEndDialog() { m_endDialog = true; }
    inline void  SetShowContent() { m_showContent = true; m_showContentSet = true; }

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
    void                    FormatDate(wchar_t date_native[], const FILETIME& filetime, bool force_short_fmt) const;
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
    bool                    CloneWindow();
    std::wstring            ExpandString(const std::wstring& replaceString) const;

private:
    static bool NameCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool SizeCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool MatchesCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool PathCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool EncodingCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool ModifiedTimeCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool ExtCompareAsc(const CSearchInfo& entry1, const CSearchInfo& entry2);

    static bool NameCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool SizeCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool MatchesCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool PathCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool EncodingCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool ModifiedTimeCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);
    static bool ExtCompareDesc(const CSearchInfo& entry1, const CSearchInfo& entry2);

private:
    HWND          m_hParent;
    //volatile LONG m_dwThreadRunning;
    //volatile LONG m_cancelled;

    std::unique_ptr<CBookmarksDlg> m_bookmarksDlg;
    ComPtr<ITaskbarList3>          m_pTaskbarList;

    std::wstring                      m_searchPath;
    std::wstring                      m_searchString;
    std::wstring                      m_replaceString;
    std::vector<std::wstring>         m_patterns;
    std::wstring                      m_patternRegex;
    std::wstring                      m_excludeDirsPatternRegex;
    bool                              m_bUseRegex;
    bool                              m_bUseRegexForPaths;
    bool                              m_bAllSize;
    uint64_t                          m_lSize;
    int                               m_sizeCmp;
    bool                              m_bIncludeSystem;
    bool                              m_bIncludeSystemC;
    bool                              m_bIncludeHidden;
    bool                              m_bIncludeHiddenC;
    bool                              m_bIncludeSubfolders;
    bool                              m_bIncludeSubfoldersC;
    bool                              m_bIncludeBinary;
    bool                              m_bIncludeBinaryC;
    bool                              m_bCreateBackup;
    bool                              m_bCreateBackupC;
    bool                              m_bCreateBackupInFolders;
    bool                              m_bCreateBackupInFoldersC;
    bool                              m_bWholeWords;
    bool                              m_bWholeWordsC;
    bool                              m_bUTF8;
    bool                              m_bUTF8C;
    bool                              m_bForceBinary;
    bool                              m_bCaseSensitive;
    bool                              m_bCaseSensitiveC;
    bool                              m_bDotMatchesNewline;
    bool                              m_bDotMatchesNewlineC;
    bool                              m_bNotSearch;
    bool                              m_bCaptureSearch;
    bool                              m_bSizeC;
    bool                              m_endDialog;
    ExecuteAction                     m_executeImmediately;
    int                               m_dateLimit;
    bool                              m_bDateLimitC;
    FILETIME                          m_date1;
    FILETIME                          m_date2;
    bool                              m_bNoSaveSettings;
    bool                              m_bReplace;
    bool                              m_bConfirmationOnReplace;
    bool                              m_showContent;
    bool                              m_showContentSet;
    std::vector<CSearchInfo>          m_items;
    std::vector<std::tuple<int, int>> m_listItems;
    std::set<std::wstring>            m_backupAndTempFiles;
    int                               m_totalItems;
    int                               m_searchedItems;
    int                               m_totalMatches;
    int                               m_selectedItems;
    bool                              m_bAscending;
    std::wstring                      m_resultString;
    std::wstring                      m_toolTipReplaceString;

    bool m_bStayOnTop;
    BYTE m_OpacityNoFocus;

    CDlgResizer m_resizer;
    int         m_themeCallbackId;

    CFileDropTarget* m_pDropTarget;

    static UINT m_grepwinStartupmsg;

#ifdef NP3_ALLOW_UPDATE
    std::thread m_updateCheckThread;
#endif

    CAutoComplete m_autoCompleteFilePatterns;
    CAutoComplete m_autoCompleteExcludeDirsPatterns;
    CAutoComplete m_autoCompleteSearchPatterns;
    CAutoComplete m_autoCompleteReplacePatterns;
    CAutoComplete m_autoCompleteSearchPaths;

    CEditDoubleClick m_editFilePatterns;
    CEditDoubleClick m_editExcludeDirsPatterns;
    CEditDoubleClick m_editSearchPatterns;
    CEditDoubleClick m_editReplacePatterns;
    CEditDoubleClick m_editSearchPaths;

    CRegStdDWORD  m_regUseRegex;
    CRegStdDWORD  m_regAllSize;
    CRegStdString m_regSize;
    CRegStdDWORD  m_regSizeCombo;
    CRegStdDWORD  m_regIncludeSystem;
    CRegStdDWORD  m_regIncludeHidden;
    CRegStdDWORD  m_regIncludeSubfolders;
    CRegStdDWORD  m_regIncludeBinary;
    CRegStdDWORD  m_regCreateBackup;
    CRegStdDWORD  m_regWholeWords;
    CRegStdDWORD  m_regUTF8;
    CRegStdDWORD  m_regBinary;
    CRegStdDWORD  m_regCaseSensitive;
    CRegStdDWORD  m_regDotMatchesNewline;
    CRegStdDWORD  m_regUseRegexForPaths;
    CRegStdString m_regPattern;
    CRegStdString m_regExcludeDirsPattern;
    CRegStdString m_regSearchPath;
    CRegStdString m_regEditorCmd;
    CRegStdDWORD  m_regBackupInFolder;
    CRegStdDWORD  m_regDateLimit;
    CRegStdDWORD  m_regDate1Low;
    CRegStdDWORD  m_regDate1High;
    CRegStdDWORD  m_regDate2Low;
    CRegStdDWORD  m_regDate2High;
    CRegStdDWORD  m_regShowContent;

    CRegStdDWORD  m_regStayOnTop;
    CRegStdDWORD  m_regOpacityNoFocus;
};
