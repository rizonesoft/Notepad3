// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* TabManager.c                                                                *
*   Multi-Tab Document Manager Implementation                                 *
*   Uses Scintilla's native multi-document API:                              *
*     SCI_CREATEDOCUMENT / SCI_SETDOCPOINTER / SCI_RELEASEDOCUMENT           *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#include "TypeDefs.h"
#include "Notepad3.h"
#include "TabManager.h"
#include "SciCall.h"
#include "Helpers.h"
#include "PathLib.h"

#include <commctrl.h>

//=============================================================================
// Globals
//=============================================================================

HWND      g_hwndTabControl = NULL;
TabDoc*   g_pActiveTab = NULL;
int       g_nTabCount = 0;
TabDoc    g_Tabs[MAX_TABS];

//=============================================================================
// Internal Helpers
//=============================================================================

static void _BuildTabText(TabDoc* pTab, int nUntitledIndex)
{
    if (!pTab) return;

    if (Path_IsNotEmpty(pTab->hFilePath)) {
        HPATHL hName = Path_GetFileNameCopy(pTab->hFilePath);
        const WCHAR* pName = hName ? Path_Get(hName) : NULL;
        StringCchPrintf(pTab->wchTabText, COUNTOF(pTab->wchTabText),
                        L"%s%s", pTab->bModified ? L"* " : L"", pName ? pName : L"Untitled");
        Path_Release(hName);
    } else {
        StringCchPrintf(pTab->wchTabText, COUNTOF(pTab->wchTabText),
                        L"%sUntitled %i", pTab->bModified ? L"* " : L"", nUntitledIndex);
    }
}

static void _RefreshTabControlItem(int nIndex)
{
    if (!g_hwndTabControl || nIndex < 0 || nIndex >= g_nTabCount) return;

    TCITEMW tci = { 0 };
    tci.mask = TCIF_TEXT;
    tci.pszText = g_Tabs[nIndex].wchTabText;
    tci.cchTextMax = (int)COUNTOF(g_Tabs[nIndex].wchTabText);
    TabCtrl_SetItemW(g_hwndTabControl, nIndex, &tci);
}

//=============================================================================
// Initialization / Cleanup
//=============================================================================

void TabManager_Init(HWND hwndMain, HINSTANCE hInstance)
{
    g_hwndTabControl = CreateWindowEx(
        0,
        WC_TABCONTROL,
        NULL,
        (WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_SINGLELINE | TCS_RIGHTJUSTIFY | TCS_TABS) & ~WS_VISIBLE,
        0, 0, 0, 0,
        hwndMain,
        (HMENU)(INT_PTR)IDC_TAB,
        hInstance,
        NULL);

    if (!g_hwndTabControl) {
        return;
    }

    InitWindowCommon(g_hwndTabControl, true);

#ifdef D_NP3_WIN10_DARK_MODE
    if (IsDarkModeSupported()) {
        AllowDarkModeForWindowEx(g_hwndTabControl, CheckDarkModeEnabled());
    }
#endif

    for (int i = 0; i < MAX_TABS; i++) {
        g_Tabs[i].pDocPointer = NULL;
        g_Tabs[i].hFilePath = NULL;
        g_Tabs[i].bModified = false;
        g_Tabs[i].bReadOnly = false;
        g_Tabs[i].wchTabText[0] = L'\0';
        memset(&g_Tabs[i].fvFileVars, 0, sizeof(FILEVARS));
        g_Tabs[i].iEncoding = Settings.DefaultEncoding;
        g_Tabs[i].iEOLMode = Settings.DefaultEOLMode;
    }

    g_pActiveTab = NULL;
    g_nTabCount = 0;
}

void TabManager_Cleanup(HWND hwndMain)
{
    UNREFERENCED_PARAMETER(hwndMain);

    for (int i = 0; i < g_nTabCount; i++) {
        if (g_Tabs[i].pDocPointer && Globals.hwndEdit) {
            if (g_pActiveTab != &g_Tabs[i]) {
                SendMessage(Globals.hwndEdit, SCI_SETDOCPOINTER, 0, (LPARAM)g_Tabs[i].pDocPointer);
            }
            SendMessage(Globals.hwndEdit, SCI_RELEASEDOCUMENT, 0, (LPARAM)g_Tabs[i].pDocPointer);
        }
        if (g_Tabs[i].hFilePath) {
            Path_Release(g_Tabs[i].hFilePath);
        }
    }

    g_nTabCount = 0;
    g_pActiveTab = NULL;

    if (g_hwndTabControl) {
        DestroyWindow(g_hwndTabControl);
        g_hwndTabControl = NULL;
    }
}

void TabManager_RegisterInitialTab(HWND hwndMain)
{
    UNREFERENCED_PARAMETER(hwndMain);

    if (!Globals.hwndEdit || !g_hwndTabControl) return;

    void* pDoc = (void*)SendMessage(Globals.hwndEdit, SCI_GETDOCPOINTER, 0, 0);
    if (!pDoc) return;

    int nIndex = g_nTabCount;
    TabDoc* pTab = &g_Tabs[nIndex];

    pTab->pDocPointer = pDoc;
    pTab->bModified = false;
    pTab->bReadOnly = false;
    pTab->fvFileVars = Globals.fvCurFile;
    pTab->iEncoding = Encoding_Current(-1);
    pTab->iEOLMode = Globals.fvCurFile.chMode[0] ? Globals.fvCurFile.chMode[0] : Settings.DefaultEOLMode;

    if (Path_IsNotEmpty(Paths.CurrentFile)) {
        pTab->hFilePath = Path_Copy(Paths.CurrentFile);
    } else {
        pTab->hFilePath = NULL;
    }

    _BuildTabText(pTab, nIndex + 1);

    TCITEMW tci = { 0 };
    tci.mask = TCIF_TEXT | TCIF_PARAM;
    tci.pszText = pTab->wchTabText;
    tci.cchTextMax = (int)COUNTOF(pTab->wchTabText);
    tci.lParam = (LPARAM)pTab;
    TabCtrl_InsertItemW(g_hwndTabControl, nIndex, &tci);

    g_nTabCount++;
    g_pActiveTab = pTab;

    ShowWindow(g_hwndTabControl, SW_SHOW);
}

//=============================================================================
// Tab Operations
//=============================================================================

TabDoc* TabManager_AddNewTab(HWND hwndMain)
{
    if (!Globals.hwndEdit || !g_hwndTabControl || g_nTabCount >= MAX_TABS) {
        return NULL;
    }

    void* pNewDoc = (void*)SendMessage(Globals.hwndEdit, SCI_CREATEDOCUMENT, 0, SC_DOCUMENTOPTION_DEFAULT);
    if (!pNewDoc) return NULL;

    int nIndex = g_nTabCount;
    TabDoc* pTab = &g_Tabs[nIndex];

    pTab->pDocPointer = pNewDoc;
    pTab->bModified = false;
    pTab->bReadOnly = false;
    pTab->fvFileVars = Globals.fvCurFile;
    pTab->hFilePath = NULL;
    pTab->iEncoding = Settings.DefaultEncoding;
    pTab->iEOLMode = Settings.DefaultEOLMode;

    _BuildTabText(pTab, nIndex + 1);

    TCITEMW tci = { 0 };
    tci.mask = TCIF_TEXT | TCIF_PARAM;
    tci.pszText = pTab->wchTabText;
    tci.cchTextMax = (int)COUNTOF(pTab->wchTabText);
    tci.lParam = (LPARAM)pTab;
    TabCtrl_InsertItemW(g_hwndTabControl, nIndex, &tci);

    g_nTabCount++;

    TabManager_SwitchToTab(hwndMain, nIndex);

    ShowWindow(g_hwndTabControl, SW_SHOW);

    return pTab;
}

bool TabManager_CloseTab(int nIndex)
{
    if (!g_hwndTabControl || nIndex < 0 || nIndex >= g_nTabCount) {
        return false;
    }

    TabDoc* pTab = &g_Tabs[nIndex];

    if (pTab->pDocPointer && Globals.hwndEdit) {
        SendMessage(Globals.hwndEdit, SCI_RELEASEDOCUMENT, 0, (LPARAM)pTab->pDocPointer);
    }

    if (pTab->hFilePath) {
        Path_Release(pTab->hFilePath);
    }

    TabCtrl_DeleteItem(g_hwndTabControl, nIndex);

    for (int i = nIndex; i < g_nTabCount - 1; i++) {
        g_Tabs[i] = g_Tabs[i + 1];
        TCITEMW tci = { 0 };
        tci.mask = TCIF_PARAM;
        tci.lParam = (LPARAM)&g_Tabs[i];
        TabCtrl_SetItemW(g_hwndTabControl, i, &tci);
    }

    TabDoc empty = { 0 };
    g_Tabs[g_nTabCount - 1] = empty;

    g_nTabCount--;

    if (g_nTabCount == 0) {
        return true;
    } else {
        int nSwitchIndex = nIndex;
        if (nSwitchIndex >= g_nTabCount) {
            nSwitchIndex = g_nTabCount - 1;
        }
        TabManager_SwitchToTab(Globals.hwndMain, nSwitchIndex);
    }

    return true;
}

void TabManager_CloseAllOtherTabs(int nKeepIndex)
{
    if (nKeepIndex < 0 || nKeepIndex >= g_nTabCount) return;

    for (int i = g_nTabCount - 1; i >= 0; i--) {
        if (i != nKeepIndex) {
            TabManager_CloseTab(i);
        }
    }
}

void TabManager_CloseAllTabs(HWND hwndMain)
{
    while (g_nTabCount > 0) {
        TabManager_CloseTab(g_nTabCount - 1);
    }

    TabManager_Init(hwndMain, GetModuleHandle(NULL));
    TabManager_RegisterInitialTab(hwndMain);
}

void TabManager_SwitchToTab(HWND hwndMain, int nIndex)
{
    if (!Globals.hwndEdit || !g_hwndTabControl) return;
    if (nIndex < 0 || nIndex >= g_nTabCount) return;

    TabDoc* pNewTab = &g_Tabs[nIndex];

    SendMessage(Globals.hwndEdit, SCI_SETDOCPOINTER, 0, (LPARAM)pNewTab->pDocPointer);

    g_pActiveTab = pNewTab;

    if (pNewTab->hFilePath) {
        Path_Reset(Paths.CurrentFile, Path_Get(pNewTab->hFilePath));
    } else {
        Path_Empty(Paths.CurrentFile, false);
    }

    Encoding_Current(pNewTab->iEncoding);
    Globals.fvCurFile = pNewTab->fvFileVars;

    TabCtrl_SetCurSel(g_hwndTabControl, nIndex);

    if (hwndMain) {
        UpdateTitlebar(hwndMain);
        UpdateStatusbar(true);
        UpdateSaveSettingsCmds();
    }
}

//=============================================================================
// Tab Info / Queries
//=============================================================================

int TabManager_GetActiveTabIndex(void)
{
    if (!g_hwndTabControl) return -1;
    return TabCtrl_GetCurSel(g_hwndTabControl);
}

TabDoc* TabManager_GetTab(int nIndex)
{
    if (nIndex < 0 || nIndex >= g_nTabCount) return NULL;
    return &g_Tabs[nIndex];
}

//=============================================================================
// UI Helpers
//=============================================================================

void TabManager_UpdateTabText(int nIndex)
{
    TabDoc* pTab = TabManager_GetTab(nIndex);
    if (!pTab) return;

    int nUntitledCount = 0;
    for (int i = 0; i <= nIndex; i++) {
        if (Path_IsEmpty(g_Tabs[i].hFilePath)) {
            nUntitledCount++;
        }
    }
    _BuildTabText(pTab, nUntitledCount);
    _RefreshTabControlItem(nIndex);
}

void TabManager_UpdateActiveTabText(void)
{
    int nActive = TabManager_GetActiveTabIndex();
    if (nActive >= 0) {
        TabManager_UpdateTabText(nActive);
    }
}

LRESULT TabManager_HandleNotify(HWND hwndMain, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    LPNMHDR pnmh = (LPNMHDR)lParam;
    if (!pnmh) return 0;

    if ((UINT_PTR)(INT_PTR)pnmh->idFrom != (UINT_PTR)(INT_PTR)IDC_TAB) {
        return 0;
    }

    switch (pnmh->code) {
    case TCN_SELCHANGING:
        return FALSE;

    case TCN_SELCHANGE: {
        int nIndex = TabCtrl_GetCurSel(g_hwndTabControl);
        if (nIndex >= 0 && nIndex < g_nTabCount) {
            if (g_pActiveTab) {
                g_pActiveTab->fvFileVars = Globals.fvCurFile;
                g_pActiveTab->iEncoding = Encoding_Current(-1);
            }
            TabManager_SwitchToTab(hwndMain, nIndex);
        }
        return 0;
    }
    }

    return 0;
}
