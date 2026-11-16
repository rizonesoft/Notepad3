#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <cstdio>

#define COUNTOF(a) static_cast<int>(sizeof(a) / sizeof((a)[0]))
#define MIDSZ_BUFFER 256

struct EDITLEXER {
    int resID;
    LPCWSTR pszName;
};
using PEDITLEXER = EDITLEXER*;

static EDITLEXER lexStandard = { 0, L"Standard" };
static EDITLEXER lexStandard2nd = { 0, L"Standard2nd" };
static EDITLEXER lexTEXT = { 0, L"Text" };
static EDITLEXER lexZulu = { 1, L"Zulu" };
static EDITLEXER lexBravo = { 0, L"Bravo" };
static EDITLEXER lexBravo2 = { 0, L"bravo" };
static EDITLEXER lexAPDL = { 0, L"ANSYS APDL" };
static PEDITLEXER g_pLexArray[] = {
    &lexStandard, &lexStandard2nd, &lexTEXT, &lexZulu, &lexBravo, &lexBravo2, &lexAPDL
};

static bool GetLngString(int resID, LPWSTR name, int capacity)
{
    return resID == 1 && SUCCEEDED(StringCchCopyW(name, capacity, L"Alpha"));
}

#include "LexerDialogSorting.inc"

static int failures = 0;

static void Check(bool condition, const char* message)
{
    if (!condition) {
        std::printf("FAIL: %s\n", message);
        ++failures;
    }
}

int main()
{
    INITCOMMONCONTROLSEX controls = { sizeof(controls), ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES };
    if (!InitCommonControlsEx(&controls)) {
        return 1;
    }
    HWND parent = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 100, 100, nullptr, nullptr, nullptr, nullptr);
    HWND tree = CreateWindowExW(0, WC_TREEVIEWW, L"", WS_CHILD, 0, 0, 100, 100, parent, nullptr, nullptr, nullptr);
    HWND list = CreateWindowExW(0, WC_LISTVIEWW, L"", WS_CHILD | LVS_REPORT, 0, 0, 100, 100, parent, nullptr, nullptr, nullptr);
    if (!parent || !tree || !list) {
        if (parent) { DestroyWindow(parent); }
        return 1;
    }

    for (PEDITLEXER lexer : g_pLexArray) {
        WCHAR name[MIDSZ_BUFFER] = {};
        Style_GetLexerDisplayName(lexer, name, COUNTOF(name));
        TVINSERTSTRUCT root = {};
        root.hInsertAfter = TVI_LAST;
        root.item.mask = TVIF_PARAM | TVIF_TEXT;
        root.item.lParam = reinterpret_cast<LPARAM>(lexer);
        root.item.pszText = name;
        HTREEITEM node = TreeView_InsertItem(tree, &root);
        LPCWSTR styleNames[] = { L"Zulu style", L"Alpha style" };
        for (LPCWSTR style : styleNames) {
            TVINSERTSTRUCT child = {};
            child.hParent = node;
            child.hInsertAfter = TVI_LAST;
            child.item.mask = TVIF_TEXT;
            child.item.pszText = const_cast<LPWSTR>(style);
            TreeView_InsertItem(tree, &child);
        }
        LVITEM item = {};
        item.mask = LVIF_PARAM | LVIF_TEXT;
        item.iItem = ListView_GetItemCount(list);
        item.lParam = reinterpret_cast<LPARAM>(lexer);
        item.pszText = name;
        ListView_InsertItem(list, &item);
    }

    HTREEITEM current = _FindLexerTreeItem(tree, &lexAPDL);
    TreeView_SelectItem(tree, current);
    TV_SORTCB sort = { nullptr, _CompareLexersByDisplayName, 0 };
    Check(TreeView_SortChildrenCB(tree, &sort, FALSE), "tree sorting succeeds");
    Check(ListView_SortItems(list, _CompareLexersByDisplayName, 0), "list sorting succeeds");
    PEDITLEXER expected[] = {
        &lexStandard, &lexStandard2nd, &lexTEXT, &lexZulu, &lexAPDL, &lexBravo, &lexBravo2
    };
    HTREEITEM node = TreeView_GetRoot(tree);
    for (int i = 0; i < COUNTOF(expected); ++i) {
        Check(node == _FindLexerTreeItem(tree, expected[i]), "tree order pins first three and sorts localized names");
        Check(_GetListViewLexer(list, i) == expected[i], "list order matches tree order");
        WCHAR styleName[MIDSZ_BUFFER] = {};
        TVITEM child = {};
        child.mask = TVIF_TEXT;
        child.hItem = TreeView_GetChild(tree, node);
        child.pszText = styleName;
        child.cchTextMax = COUNTOF(styleName);
        TreeView_GetItem(tree, &child);
        Check(lstrcmpW(styleName, L"Zulu style") == 0, "style children retain their original order");
        node = TreeView_GetNextSibling(tree, node);
    }
    Check(TreeView_GetSelection(tree) == current, "tree preserves current selection");
    Check(_FindLexerTreeItem(tree, &lexAPDL) == current, "current-lexer lookup ignores sorted position");
    Check(_FindLexerTreeItem(tree, nullptr) == nullptr, "missing tree lexer returns null");
    Check(_GetListViewLexer(list, -1) == nullptr, "no list selection returns null");
    Check(_GetLexerIndex(_GetListViewLexer(list, 4)) == 6, "APDL default retains its original index, not display index");
    Check(g_pLexArray[6] == &lexAPDL && g_pLexArray[3] == &lexZulu, "sorting does not reorder registration array");
    DestroyWindow(parent);
    std::printf("Lexer sorting: %s\n", failures ? "FAILED" : "PASS");
    return failures ? 1 : 0;
}
