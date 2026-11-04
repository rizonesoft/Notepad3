#include <windows.h>
#include <shlobj.h>
#include <strsafe.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <tuple>

#pragma comment(lib, "shell32.lib")

// Exercise the production Path_CreateDirectoryEx and its three result-checking callers
// (extracted from the sources) against the real file system. Only the PathLib/string
// handle layer and UI hooks are stubbed; SHCreateDirectoryExW is forwarded to the real
// API unless a raw Win32 status is injected for failures that cannot be reproduced reliably.

struct TestPath {
    std::wstring value;
};
using HPATHL = TestPath*;
using HSTRINGW = std::wstring*;
#define PTHAPI __stdcall
#define COUNTOF(ar) ARRAYSIZE(ar)
#define MAX_PATH_EXPLICIT MAX_PATH
#define IS_VALID_HANDLE(HNDL) ((HNDL) && ((HNDL) != INVALID_HANDLE_VALUE))
#define PathGet(HPTH) ((HPTH)->value.c_str())

static int g_inject = -1; // >= 0: status returned by SHCreateDirectoryExW without touching the file system
static int g_createFileCalls = 0;
static int g_infoBoxCalls = 0;
static std::wstring g_snapshotDir;

static int WINAPI TestSHCreateDirectoryExW(HWND hwnd, LPCWSTR path, const SECURITY_ATTRIBUTES* sa)
{
    if (g_inject >= 0) {
        return g_inject;
    }
    return ::SHCreateDirectoryExW(hwnd, path, sa);
}
#define SHCreateDirectoryExW TestSHCreateDirectoryExW

static HPATHL Path_Allocate(LPCWSTR s) { return new TestPath{ s ? s : L"" }; }
static void Path_Release(HPATHL p) { delete p; }
static HPATHL Path_Copy(const HPATHL p) { return new TestPath{ p->value }; }
static bool Path_IsNotEmpty(const HPATHL p) { return !p->value.empty(); }
static void Path_Append(HPATHL p, LPCWSTR s) {
    if (!p->value.empty() && p->value.back() != L'\\') {
        p->value += L'\\';
    }
    p->value += s;
}
static void Path_RemoveFileSpec(HPATHL p) {
    size_t const pos = p->value.find_last_of(L'\\');
    p->value.resize(pos == std::wstring::npos ? 0 : pos);
}
static LPCWSTR Path_FindFileName(const HPATHL p) {
    size_t const pos = p->value.find_last_of(L'\\');
    return p->value.c_str() + (pos == std::wstring::npos ? 0 : pos + 1);
}
static void PrependLongPathPrefix(HPATHL) {}
[[maybe_unused]] static bool Path_IsExistingDirectory(const HPATHL p) {
    DWORD const attr = GetFileAttributesW(p->value.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}
static bool Path_IsExistingFile(const HPATHL p) {
    DWORD const attr = GetFileAttributesW(p->value.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}
static HANDLE Path_CreateFile(const HPATHL p, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sa,
                              DWORD creation, DWORD attrs, HANDLE tmpl) {
    ++g_createFileCalls;
    return CreateFileW(p->value.c_str(), access, share, sa, creation, attrs, tmpl);
}
static void Path_GetDisplayName(LPWSTR buf, size_t cch, const HPATHL p, LPCWSTR, bool) {
    StringCchCopyW(buf, cch, p->value.c_str());
}
static HSTRINGW StrgCreate(LPCWSTR) { return new std::wstring(); }
static void StrgFormat(HSTRINGW s, LPCWSTR fmt, ...) { *s = fmt; }
static LPCWSTR StrgGet(HSTRINGW s) { return s->c_str(); }
static void StrgDestroy(HSTRINGW s) { delete s; }
static void InfoBoxLastError(LPCWSTR, int) { ++g_infoBoxCalls; }
static bool CanAccessPath(const HPATHL, DWORD) { return true; }
static bool _GetDropSnapshotDir(HPATHL out) { out->value = g_snapshotDir; return true; }

#include "PathLibCreateDirectory.inc"
#include "CreateIniFile.inc"
#include "BuildSnapshotPath.inc"

static bool RecreateDirectory(HPATHL hdir_path)
{
    bool bDirCreated = false;
#include "RecreateDirectory.inc"
    return bDirCreated;
}

namespace fs = std::filesystem;

int wmain()
{
    WCHAR tmp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, tmp);
    fs::path const root = fs::path(tmp) / (L"np3-createdir-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()));
    fs::create_directories(root);

    int failures = 0;
    auto check = [&](const char* name, bool ok) {
        if (!ok) {
            std::cerr << "FAIL: " << name << '\n';
            ++failures;
        }
    };

    fs::path const existingDir = root / L"existing";
    fs::path const conflictFile = root / L"conflict";
    fs::create_directories(existingDir);
    { HANDLE h = CreateFileW(conflictFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr); CloseHandle(h); }

    HRESULT const hrExists = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
    HRESULT const hrFileExists = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);

    // ---- Path_CreateDirectoryEx contract ----
    auto create = [&](const std::wstring& path, int inject = -1) {
        g_inject = inject;
        TestPath p{ path };
        HRESULT const hr = Path_CreateDirectoryEx(&p);
        g_inject = -1;
        return hr;
    };
    fs::path const newDir = root / L"new" / L"nested";
    check("created: S_OK", create(newDir.wstring()) == S_OK);
    check("created: directory exists", fs::is_directory(newDir));
    check("existing directory: ERROR_ALREADY_EXISTS", create(existingDir.wstring()) == hrExists);
    check("conflicting file: ERROR_FILE_EXISTS, failure", create(conflictFile.wstring()) == hrFileExists && FAILED(hrFileExists));
    check("path below a file: failure", FAILED(create((conflictFile / L"sub").wstring())));
    check("relative path: failure", create(L"relative\\path") == HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    check("illegal characters: failure", FAILED(create((root / L"a?b").wstring())));
    check("injected access denied", create(existingDir.wstring(), ERROR_ACCESS_DENIED) == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED));
    check("injected path not found", create(existingDir.wstring(), ERROR_PATH_NOT_FOUND) == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND));
    check("injected cancelled", create(existingDir.wstring(), ERROR_CANCELLED) == HRESULT_FROM_WIN32(ERROR_CANCELLED));
    check("injected FILE_EXISTS on a directory", create(existingDir.wstring(), ERROR_FILE_EXISTS) == hrExists);
    check("injected FILE_EXISTS on a file", create(conflictFile.wstring(), ERROR_FILE_EXISTS) == hrFileExists);
    check("injected ALREADY_EXISTS on a file", create(conflictFile.wstring(), ERROR_ALREADY_EXISTS) == hrFileExists);

    // ---- caller: _BuildSnapshotPath (Notepad3.c) ----
    auto snapshot = [&](const std::wstring& dir, int inject = -1) {
        g_snapshotDir = dir;
        g_inject = inject;
        TestPath orig{ L"C:\\x\\orig.txt" };
        HPATHL const snap = _BuildSnapshotPath(&orig, 0);
        g_inject = -1;
        bool const ok = (snap != nullptr);
        Path_Release(snap);
        return ok;
    };
    check("snapshot: new directory accepted", snapshot((root / L"snap-new").wstring()));
    check("snapshot: existing directory accepted", snapshot(existingDir.wstring()));
    check("snapshot: conflicting file rejected", !snapshot(conflictFile.wstring()));
    check("snapshot: path below a file rejected", !snapshot((conflictFile / L"sub").wstring()));
    check("snapshot: access denied rejected", !snapshot((root / L"snap-denied").wstring(), ERROR_ACCESS_DENIED));

    // ---- caller: CreateIniFile (Config.cpp) ----
    auto iniFile = [&](const fs::path& dir, int inject = -1) {
        g_inject = inject;
        g_createFileCalls = 0;
        g_infoBoxCalls = 0;
        TestPath ini{ (dir / L"Notepad3.ini").wstring() };
        DWORD size = 123;
        bool const ok = CreateIniFile(&ini, &size);
        g_inject = -1;
        return std::make_tuple(ok, size);
    };
    {
        auto [ok, size] = iniFile(root / L"ini-new");
        check("ini: new directory accepted", ok && size == 0 && fs::exists(root / L"ini-new" / L"Notepad3.ini"));
        auto [ok2, size2] = iniFile(existingDir);
        check("ini: existing directory accepted", ok2 && g_createFileCalls == 1);
        auto [ok3, size3] = iniFile(conflictFile);
        check("ini: conflicting file rejected before file creation", !ok3 && size3 == 0 && g_createFileCalls == 0 && g_infoBoxCalls == 0);
        auto [ok4, size4] = iniFile(root / L"ini-denied", ERROR_ACCESS_DENIED);
        check("ini: access denied rejected before file creation", !ok4 && size4 == 0 && g_createFileCalls == 0 && g_infoBoxCalls == 0);
    }

    // ---- caller: directory recreation (Notepad3.c) ----
    auto recreate = [&](const std::wstring& dir, int inject = -1) {
        g_inject = inject;
        TestPath p{ dir };
        bool const ok = RecreateDirectory(&p);
        g_inject = -1;
        return ok;
    };
    check("recreate: new directory accepted", recreate((root / L"re-new").wstring()));
    check("recreate: existing directory accepted", recreate(existingDir.wstring()));
    check("recreate: conflicting file rejected", !recreate(conflictFile.wstring()));
    check("recreate: access denied rejected", !recreate((root / L"re-denied").wstring(), ERROR_ACCESS_DENIED));

    std::error_code ec;
    fs::remove_all(root, ec);
    if (failures) {
        return 1;
    }
    std::cout << "All create-directory cases passed.\n";
    return 0;
}
