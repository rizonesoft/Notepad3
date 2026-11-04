#include <windows.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

// Exercise the production directory-selection function with an in-memory path layer.
struct TestPath {
    std::wstring value;
    std::vector<WCHAR> buffer;
};
using HPATHL = TestPath*;
constexpr size_t PATHLONG_MAX_CCH = 32768;
struct {
    HPATHL CurrentFile;
    HPATHL WorkingDirectory;
    HPATHL ModuleDirectory;
} Paths;
struct {
    HPATHL DefaultDirectoryOverride;
    bool SaveRecentFiles;
} Settings;
struct { HPATHL DefaultDirectory; } Settings2;
struct { std::vector<std::wstring>* pFileMRU; } Globals;
static int FOLDERID_Documents;
static std::vector<std::wstring> directories;

static bool Path_IsEmpty(HPATHL p) { return p->value.empty(); }
static bool Path_IsNotEmpty(HPATHL p) { return !Path_IsEmpty(p); }
static LPCWSTR Path_Get(HPATHL p) { return p->value.c_str(); }
static size_t Path_GetLength(HPATHL p) { return p->value.size(); }
static void Path_Reset(HPATHL p, LPCWSTR value) { p->value = value; }
static void Path_Empty(HPATHL p, bool) { p->value.clear(); }
static void Path_RemoveFileSpec(HPATHL p) {
    p->value = std::filesystem::path(p->value).parent_path().wstring();
    while (!p->value.empty() && p->value.back() == L'\\') {
        p->value.pop_back();
    }
}
static bool Path_IsRelative(HPATHL p) {
    return std::filesystem::path(p->value).is_relative();
}
static void Path_CanonicalizeEx(HPATHL p, HPATHL base) {
    if (Path_IsRelative(p)) {
        p->value = (std::filesystem::path(base->value) / p->value).wstring();
    }
    p->value = std::filesystem::path(p->value).lexically_normal().wstring();
}
static LPWSTR Path_WriteAccessBuf(HPATHL p, size_t count) {
    p->buffer.assign(count, L'\0');
    return p->buffer.data();
}
static size_t Path_GetBufCount(HPATHL p) { return p->buffer.size(); }
static void Path_Sanitize(HPATHL p) { p->value = p->buffer.data(); }
static void Path_UnQuoteSpaces(HPATHL p) {
    if (p->value.size() >= 2 && p->value.front() == L'"' && p->value.back() == L'"') {
        p->value = p->value.substr(1, p->value.size() - 2);
    }
}
static void Path_AbsoluteFromApp(HPATHL p, bool) {
    Path_CanonicalizeEx(p, Paths.ModuleDirectory);
}
static bool Path_IsExistingDirectory(HPATHL p) {
    for (const auto& dir : directories) {
        if (p->value == dir) { return true; }
    }
    return false;
}
static bool Path_IsExistingFile(HPATHL) { return false; }
static bool Path_IsDirectoryWritable(HPATHL p) { return Path_IsExistingDirectory(p); }
static void Path_GetKnownFolder(const int*, HPATHL p) { p->value = L"C:\\Documents"; }
static int MRU_Count(std::vector<std::wstring>* mru) { return static_cast<int>(mru->size()); }
static int MRU_Enum(std::vector<std::wstring>* mru, int index, LPWSTR buf, int count) {
    const auto& value = mru->at(index);
    wcscpy_s(buf, static_cast<size_t>(count), value.c_str());
    return static_cast<int>(value.size());
}

#include "OpenDialogDirectory.inc"

int wmain() {
    TestPath current{}, working{ L"C:\\Working", {} }, module{ L"C:\\App", {} };
    TestPath overrideDir{}, defaultDir{}, output{};
    std::vector<std::wstring> mru;
    Paths = { &current, &working, &module };
    Settings = { &overrideDir, true };
    Settings2 = { &defaultDir };
    Globals = { &mru };
    directories = { L"C:\\Working", L"C:\\Recent", L"C:\\Current",
                    L"C:\\Override", L"C:\\Default", L"C:\\Explicit", L"C:\\App\\Recent", L"C:\\" };
    int failures = 0;
    const auto check = [&](const char* name, LPCWSTR expected, bool open = true) {
        if (open) {
            _ResolveOpenInitialDir(&output);
        }
        else {
            _CanonicalizeInitialDir(&output, true);
        }
        if (output.value != expected) {
            std::cerr << "FAIL: " << name << '\n';
            ++failures;
        }
        output.value.clear();
    };
    check("empty history", L"C:\\Working");
    mru = { L"C:\\Recent\\deleted.txt" };
    check("existing parent of missing recent file", L"C:\\Recent");
    mru = { L"C:\\file.txt" };
    check("drive root is preserved", L"C:\\");
    mru = { L"" };
    check("empty history entry", L"C:\\Working");
    mru = { L"C:\\Recent\\deleted.txt" };
    Settings.SaveRecentFiles = false;
    check("history disabled", L"C:\\Working");
    Settings.SaveRecentFiles = true;
    check("save does not use history", L"C:\\Working", false);
    mru = { L"C:\\Missing\\file.txt", L"C:\\Recent\\older.txt" };
    check("missing newest directory skips older entries", L"C:\\Working");
    working.value = L"Recent";
    check("relative working directory uses shared fallback", L"C:\\App\\Recent");
    working.value = L"C:\\Working";
    mru = { L"\"Recent\\file.txt\"" };
    check("quoted relative history", L"C:\\App\\Recent");
    defaultDir.value = L"C:\\Default";
    check("configured default precedes history", L"C:\\Default");
    overrideDir.value = L"C:\\Override";
    check("menu default precedes configured default", L"C:\\Override");
    current.value = L"C:\\Current\\file.txt";
    check("current file precedes defaults", L"C:\\Current");
    output.value = L"C:\\Explicit";
    check("explicit directory precedes current file", L"C:\\Explicit");
    current.value.clear();
    overrideDir.value.clear();
    defaultDir.value.clear();
    mru.clear();
    working.value = L"C:\\Missing";
    check("invalid working directory falls back to Documents", L"C:\\Documents");
    if (failures) { return 1; }
    std::cout << "All 14 open-directory cases passed.\n";
    return 0;
}
