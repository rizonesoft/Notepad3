#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#include <strsafe.h>
#include "..\language\common_res.h"
#include "..\src\TypeDefs.h"
extern "C" {
#include "..\src\DynStrg.h"
}
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Production DynStrg and extracted settings/commands; INI, editor, paths, saves and launches are stubbed.
struct TestPath {
    std::wstring value;
};
constexpr size_t PATHLONG_MAX_CCH = 0x8000;
#define COUNTOF(value) _countof(value)

PATHS_T Paths{};
SETTINGS_T Settings{};
SETTINGS2_T Settings2{};
GLOBALS_T Globals{};
std::wstring selection;
std::wstring savedPath;
bool saveResult = true;
bool fileExists = true;
bool launchResult = true;
HRESULT conversionError = S_OK;
HRESULT associationError = S_OK;
bool associationFailureOnRead = false;
bool browserQueriesOnly = true;
int associationQueries = 0;
bool bufferGrowthFails = false;
size_t writableCapacity = 0;
int oversizedBufferCalls = 0;
int conversionCalls = 0;
std::wstring defaultBrowserClass;
int saves = 0;
DWORD reportedError = 0;
int reportedErrors = 0;
bool enabled[CMD_WEBACTION3 + 1]{};
std::map<std::wstring, std::wstring> iniValues;
struct Launch {
    std::wstring file;
    std::wstring arguments;
    std::wstring directory;
    std::wstring className;
    ULONG mask;
};
std::vector<Launch> launches;

bool StrIsNotEmpty(LPCWSTR value) { return value && *value; }
LPWSTR TestStrgWriteAccessBuf(HSTRINGW value, size_t count) {
    LPWSTR buffer = bufferGrowthFails ? nullptr : StrgWriteAccessBuf(value, count);
    writableCapacity = buffer ? StrgGetAllocLength(value) : 0;
    return buffer;
}
#define StrgWriteAccessBuf TestStrgWriteAccessBuf
void ExpandEnvironmentStrgs(HSTRINGW value, bool) {
    const DWORD count = ExpandEnvironmentStringsW(StrgGet(value), nullptr, 0);
    std::vector<WCHAR> buffer(count);
    ExpandEnvironmentStringsW(StrgGet(value), buffer.data(), count);
    StrgReset(value, buffer.data());
}
std::wstring& PathValue(HPATHL value) { return reinterpret_cast<TestPath*>(value)->value; }
LPCWSTR Path_Get(HPATHL value) { return PathValue(value).c_str(); }
bool Path_IsEmpty(HPATHL value) { return PathValue(value).empty(); }
bool Path_IsNotEmpty(HPATHL value) { return !Path_IsEmpty(value); }
size_t Path_GetLength(HPATHL value) { return PathValue(value).size(); }
bool Path_IsExistingFile(HPATHL) { return fileExists; }
HPATHL Path_Copy(HPATHL value) { return reinterpret_cast<HPATHL>(new TestPath{ PathValue(value) }); }
void Path_Release(HPATHL value) { delete reinterpret_cast<TestPath*>(value); }
void Path_RemoveFileSpec(HPATHL value) {
    std::wstring& path = PathValue(value);
    const size_t at = path.find_last_of(L'\\');
    path.resize(at == 2 ? 3 : at);
}
size_t EditGetSelectedText(LPWSTR buffer, size_t count) {
    if (selection.size() >= count) { buffer[0] = L'\0'; return 0; }
    wcscpy_s(buffer, count, selection.c_str());
    return selection.size() + 1;
}
bool FileSave(int) {
    ++saves;
    if (saveResult && !savedPath.empty()) { PathValue(Paths.CurrentFile) = savedPath; }
    return saveResult;
}
DWORD InfoBoxLastError(LPCWSTR, DWORD error) {
    ++reportedErrors;
    reportedError = error ? error : GetLastError();
    return reportedError;
}
BOOL TestShellExecuteExW(SHELLEXECUTEINFOW* info) {
    launches.push_back({ info->lpFile, info->lpParameters ? info->lpParameters : L"",
                        info->lpDirectory ? info->lpDirectory : L"",
                        info->lpClass ? info->lpClass : L"", info->fMask });
    if (!launchResult) { SetLastError(ERROR_FILE_NOT_FOUND); }
    return launchResult;
}
#define ShellExecuteExW TestShellExecuteExW
HRESULT TestUrlCreateFromPathW(PCWSTR path, PWSTR url, DWORD* count, DWORD flags) {
    ++conversionCalls;
    if (*count > writableCapacity) { ++oversizedBufferCalls; return E_OUTOFMEMORY; }
    return FAILED(conversionError) ? conversionError : UrlCreateFromPathW(path, url, count, flags);
}
#define UrlCreateFromPathW TestUrlCreateFromPathW
HRESULT TestAssocQueryStringW(ASSOCF flags, ASSOCSTR kind, LPCWSTR association,
                             LPCWSTR extra, LPWSTR output, DWORD* count) {
    ++associationQueries;
    browserQueriesOnly = browserQueriesOnly && (flags & ASSOCF_IS_PROTOCOL) != 0
        && kind == ASSOCSTR_PROGID && std::wstring(association) == L"https" && !extra;
    if (FAILED(associationError) && (!associationFailureOnRead || output)) {
        return associationError;
    }
    const DWORD required = static_cast<DWORD>(defaultBrowserClass.size() + 1);
    if (!output) { *count = required; return S_FALSE; }
    if (*count > writableCapacity) { ++oversizedBufferCalls; return E_OUTOFMEMORY; }
    if (*count < required) { *count = required; return E_POINTER; }
    wcscpy_s(output, *count, defaultBrowserClass.c_str());
    *count = required;
    return S_OK;
}
#define AssocQueryStringW TestAssocQueryStringW
void EnableCmd(HMENU menu, int id, bool state) {
    enabled[id] = state;
    EnableMenuItem(menu, id, MF_BYCOMMAND | (state ? MF_ENABLED : MF_GRAYED));
}
void IniSectionGetStringNoQuotes(int, LPCWSTR key, LPCWSTR fallback, LPWSTR output, size_t count) {
    const auto value = iniValues.find(key);
    wcscpy_s(output, count, value == iniValues.end() ? fallback : value->second.c_str());
}
void LoadWebTemplateSettings() {
    const int IniSecSettings2 = 0;
    WCHAR pPathBuffer[PATHLONG_MAX_CCH]{};
#include "WebTemplateSettings.inc"
}

#include "WebTemplateHelpers.inc"

void RunAction(int iLoWParam = CMD_WEBACTION1) {
    switch (iLoWParam) {
#include "WebTemplateCommands.inc"
    }
}
void UpdateMenuOn(HMENU hmenu, bool se, bool mrs, bool bPosInSel, bool bIsHLink) {
#include "WebTemplateMenu.inc"
}
void UpdateMenu(bool se, bool mrs, bool bPosInSel, bool bIsHLink) {
    UpdateMenuOn(Globals.hMainMenu, se, mrs, bPosInSel, bIsHLink);
}
HMENU CreateActionMenu(bool mainMenu) {
    HMENU actions = CreatePopupMenu();
    AppendMenuW(actions, MF_STRING, CMD_WEBACTION1,
        mainMenu ? L"Web Template &1\tCtrl+Shift+1" : L"Open Web Action 1");
    AppendMenuW(actions, MF_STRING, CMD_WEBACTION2,
        mainMenu ? L"Web Template &2\tCtrl+Shift+2" : L"Open Web Action 2");
    AppendMenuW(actions, MF_STRING, CMD_WEBACTION3,
        mainMenu ? L"Preview current file &3\tCtrl+Shift+3" : L"Preview current file");
    if (!mainMenu) { return actions; }
    HMENU file = CreatePopupMenu();
    HMENU root = CreateMenu();
    AppendMenuW(file, MF_POPUP, reinterpret_cast<UINT_PTR>(actions), L"Launch");
    AppendMenuW(root, MF_POPUP, reinterpret_cast<UINT_PTR>(file), L"File");
    return root;
}
std::wstring MenuText(HMENU menu, UINT id) {
    WCHAR text[SMALL_BUFFER]{};
    GetMenuStringW(menu, id, text, COUNTOF(text), MF_BYCOMMAND);
    return text;
}

int wmain() {
    TestPath current{};
    HSTRINGW first = StrgCreate(nullptr), second = StrgCreate(nullptr), third = StrgCreate(nullptr);
    HSTRINGW app = StrgCreate(nullptr), arguments = StrgCreate(nullptr);
    Paths.CurrentFile = reinterpret_cast<HPATHL>(&current);
    Settings2.WebTemplate1 = first;
    Settings2.WebTemplate2 = second;
    Settings2.WebTemplate3 = third;
    Settings2.HyperlinkShellExURLWithApp = app;
    Settings2.HyperlinkShellExURLCmdLnArgs = arguments;
    Globals.hMainMenu = CreateActionMenu(true);
    const auto reset = [&] {
        current.value = L"C:\\Notes\\readme.md";
        StrgReset(first, L"https://example.test/?q=%s");
        StrgReset(second, L"https://example.test/second/%s");
        StrgReset(third, L"${FILE_URL}");
        StrgReset(app, L"");
        StrgReset(arguments, L"\"${URL}\"");
        Settings2.WebTmpl1MenuName[0] = Settings2.WebTmpl2MenuName[0] = Settings2.WebTmpl3MenuName[0] = L'\0';
        iniValues.clear();
        Settings.SaveBeforeRunningTools = false;
        selection = L"selected text";
        savedPath.clear();
        saveResult = fileExists = launchResult = true;
        conversionError = S_OK;
        associationError = S_OK;
        associationFailureOnRead = false;
        browserQueriesOnly = true;
        associationQueries = 0;
        bufferGrowthFails = false;
        writableCapacity = 0;
        oversizedBufferCalls = 0;
        conversionCalls = 0;
        defaultBrowserClass = L"NP3.TestHttpsBrowser";
        saves = 0;
        reportedError = 0;
        reportedErrors = 0;
        launches.clear();
    };
    int failures = 0;
    int checks = 0;
    const auto check = [&](const char* name, bool result) {
        ++checks;
        if (!result) { std::cerr << "FAIL: " << name << '\n'; ++failures; }
    };
    reset();
    RunAction();
    check("legacy selection URL and shell fallback", launches.size() == 1
        && launches[0].file == L"https://example.test/?q=selected text"
        && launches[0].directory == L"C:\\Notes" && launches[0].arguments.empty()
        && launches[0].className.empty() && associationQueries == 0
        && launches[0].mask == SEE_MASK_NOZONECHECKS);
    reset();
    selection = L"x";
    RunAction();
    check("legacy single-character selection", launches.size() == 1
        && launches[0].file == L"https://example.test/?q=x");
    reset();
    selection.clear();
    RunAction();
    check("legacy empty selection", launches.empty());
    for (const WCHAR delimiter : { L'\r', L'\n', L'\t' }) {
        reset();
        selection = std::wstring(L"first") + delimiter + L"second";
        RunAction();
        check("legacy selection delimiter truncation", launches.size() == 1
            && launches[0].file == L"https://example.test/?q=first");
    }
    reset();
    StrgReset(app, L"C:\\Browser\\browser.exe");
    StrgReset(arguments, L"--new-window \"${URL}\"");
    Settings.SaveBeforeRunningTools = true;
    RunAction(CMD_WEBACTION2);
    check("second slot and shared browser arguments", launches.size() == 1
        && launches[0].file == StrgGet(app)
        && launches[0].arguments == L"--new-window \"https://example.test/second/selected text\""
        && launches[0].mask == SEE_MASK_DEFAULT);
    check("legacy templates do not save", saves == 0);
    StrgReset(arguments, L"");
    launches.clear();
    RunAction();
    check("legacy empty arguments remain unchanged", launches.size() == 1
        && launches[0].arguments.empty() && reportedError == 0);
    StrgReset(arguments, L"--new-window");
    launches.clear();
    RunAction();
    check("legacy arguments without URL token remain unchanged", launches.size() == 1
        && launches[0].arguments == L"--new-window" && reportedError == 0 && saves == 0);
    UpdateMenu(false, false, true, false);
    check("legacy menu selection available", enabled[CMD_WEBACTION1] && enabled[CMD_WEBACTION2]);
    UpdateMenu(true, false, false, false);
    check("legacy menu selection required", !enabled[CMD_WEBACTION1] && !enabled[CMD_WEBACTION2]);
    UpdateMenu(false, false, true, true);
    check("legacy hyperlink context unavailable", !enabled[CMD_WEBACTION1] && !enabled[CMD_WEBACTION2]);

    const auto preview = [&] {
        reset();
        StrgReset(first, L"${FILE_URL}");
        StrgReset(app, L"C:\\Browser\\browser.exe");
        selection.clear();
    };
    preview();
    RunAction();
    check("file preview without selection", launches.size() == 1
        && launches[0].file == StrgGet(app)
        && launches[0].arguments == L"\"file:///C:/Notes/readme.md\""
        && launches[0].directory == L"C:\\Notes" && saves == 0
        && launches[0].className.empty() && associationQueries == 0);
    preview();
    StrgReset(arguments, L"");
    RunAction();
    check("empty browser arguments still pass the file URL", launches.size() == 1
        && launches[0].arguments == L"file:///C:/Notes/readme.md");
    preview();
    StrgReset(arguments, L"--new-window");
    Settings.SaveBeforeRunningTools = true;
    RunAction();
    check("browser arguments without URL token rejected before saving",
        launches.empty() && saves == 0 && reportedError == ERROR_INVALID_PARAMETER);
    preview();
    selection = L"unrelated selected text";
    RunAction();
    check("file preview ignores selection", launches.size() == 1
        && launches[0].arguments == L"\"file:///C:/Notes/readme.md\"");
    UpdateMenu(true, true, false, true);
    check("file menu ignores selection and hyperlink context", enabled[CMD_WEBACTION1] && !enabled[CMD_WEBACTION2]);
    StrgReset(first, L"https://example.test/?q=%s");
    StrgReset(second, L"${FILE_URL}");
    UpdateMenu(true, false, false, false);
    RunAction(CMD_WEBACTION2);
    check("second slot has independent file mode", !enabled[CMD_WEBACTION1] && enabled[CMD_WEBACTION2]
        && launches.size() == 2 && launches[1].arguments == L"\"file:///C:/Notes/readme.md\"");
    preview();
    current.value.clear();
    Settings.SaveBeforeRunningTools = true;
    UpdateMenu(true, false, false, false);
    RunAction();
    check("untitled is disabled and never saves or launches", !enabled[CMD_WEBACTION1] && saves == 0 && launches.empty());
    preview();
    StrgReset(app, L"");
    UpdateMenu(true, true, false, true);
    RunAction();
    check("default browser receives file URL without using its extension association",
        enabled[CMD_WEBACTION1] && launches.size() == 1 && associationQueries == 2 && browserQueriesOnly
        && launches[0].file == L"file:///C:/Notes/readme.md"
        && launches[0].arguments.empty() && launches[0].directory == L"C:\\Notes"
        && launches[0].className == defaultBrowserClass
        && (launches[0].mask & SEE_MASK_CLASSNAME) != 0);
    preview();
    StrgReset(app, L"");
    StrgReset(arguments, L"--ignored-explicit-browser-arguments");
    StrgReset(first, L"https://example.test/?q=%s");
    StrgReset(second, L"${FILE_URL}");
    current.value = L"C:\\Notes\\a b#100%.md";
    Settings.SaveBeforeRunningTools = true;
    RunAction(CMD_WEBACTION2);
    check("second slot defaults to browser and ignores explicit-app arguments",
        launches.size() == 1 && saves == 1 && reportedError == 0
        && launches[0].file == L"file:///C:/Notes/a%20b%23100%25.md"
        && launches[0].arguments.empty() && launches[0].className == defaultBrowserClass);
    preview();
    StrgReset(app, L"");
    current.value.clear();
    Settings.SaveBeforeRunningTools = true;
    UpdateMenu(true, false, false, false);
    RunAction();
    check("untitled default-browser preview is disabled without querying or saving",
        !enabled[CMD_WEBACTION1] && associationQueries == 0 && saves == 0 && launches.empty());
    for (const bool failOnRead : { false, true }) {
        preview();
        StrgReset(app, L"");
        associationError = HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);
        associationFailureOnRead = failOnRead;
        Settings.SaveBeforeRunningTools = true;
        RunAction();
        check("failed browser lookup reports error without save or extension fallback",
            launches.empty() && saves == 0 && reportedError == ERROR_NO_ASSOCIATION
            && associationQueries == (failOnRead ? 2 : 1));
    }
    preview();
    StrgReset(app, L"");
    associationError = E_POINTER;
    associationFailureOnRead = true;
    Settings.SaveBeforeRunningTools = true;
    RunAction();
    check("undersized browser-query buffer reports a buffer error, not invalid pointer",
        launches.empty() && saves == 0 && reportedError == ERROR_INSUFFICIENT_BUFFER
        && associationQueries == 2);
    reset();
    bufferGrowthFails = true;
    Settings.SaveBeforeRunningTools = true;
    RunAction(CMD_WEBACTION3);
    check("failed browser buffer allocation aborts before saving or calling Windows",
        launches.empty() && saves == 0 && associationQueries == 1 && oversizedBufferCalls == 0
        && reportedError == ERROR_OUTOFMEMORY && reportedErrors == 1);
    preview();
    Settings.SaveBeforeRunningTools = true;
    check("unallocated file-preview target reports failure before saving or conversion",
        !_PrepareWebTemplate(StrgGet(first), nullptr) && saves == 0 && conversionCalls == 0
        && reportedError == ERROR_OUTOFMEMORY && reportedErrors == 1);
    preview();
    Settings.SaveBeforeRunningTools = true;
    HSTRINGW freshTarget = StrgCreate(nullptr);
    const bool lazyTarget = freshTarget && !StrgGet(freshTarget);
    const bool preparedTarget = _PrepareWebTemplate(StrgGet(first), freshTarget);
    check("fresh production string initializes before allocation is checked",
        lazyTarget && preparedTarget && std::wstring(StrgGet(freshTarget)) == L"file:///C:/Notes/readme.md"
        && saves == 1 && conversionCalls == 1 && reportedErrors == 0);
    StrgDestroy(freshTarget);
    preview();
    StrgReset(app, L"");
    defaultBrowserClass.clear();
    RunAction();
    check("empty default browser class never uses file association",
        launches.empty() && reportedError == ERROR_NO_ASSOCIATION);
    preview();
    StrgReset(app, L"");
    launchResult = false;
    RunAction();
    check("default browser launch failure reports once without shell error UI or association retry",
        launches.size() == 1 && launches[0].className == defaultBrowserClass
        && reportedError == ERROR_FILE_NOT_FOUND && reportedErrors == 1
        && (launches[0].mask & SEE_MASK_FLAG_NO_UI) != 0);
    preview();
    associationError = HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);
    RunAction();
    check("explicit browser overrides missing default browser",
        launches.size() == 1 && launches[0].file == StrgGet(app) && associationQueries == 0
        && reportedError == 0);
    preview();
    StrgReset(first, L"");
    UpdateMenu(false, false, true, false);
    RunAction();
    check("empty template is disabled", !enabled[CMD_WEBACTION1] && launches.empty());
    preview();
    Settings.SaveBeforeRunningTools = true;
    RunAction();
    check("save accepted or declined previews disk version", saves == 1 && launches.size() == 1
        && launches[0].arguments == L"\"file:///C:/Notes/readme.md\"");
    preview();
    Settings.SaveBeforeRunningTools = true;
    savedPath = L"D:\\Moved\\new name.md";
    RunAction();
    check("path and working directory reacquired after save", saves == 1 && launches.size() == 1
        && launches[0].arguments == L"\"file:///D:/Moved/new%20name.md\""
        && launches[0].directory == L"D:\\Moved");
    preview();
    Settings.SaveBeforeRunningTools = true;
    saveResult = false;
    RunAction();
    check("save cancellation or failure aborts", saves == 1 && launches.empty());
    preview();
    fileExists = false;
    RunAction();
    check("missing disk file reports error without launch",
        launches.empty() && reportedError == ERROR_FILE_NOT_FOUND);
    preview();
    bufferGrowthFails = true;
    RunAction(CMD_WEBACTION3);
    check("failed URL buffer growth never passes its requested size to Windows",
        launches.empty() && conversionCalls == 0 && oversizedBufferCalls == 0
        && reportedError == ERROR_OUTOFMEMORY && reportedErrors == 1);

    for (const auto* path : { L"C:\\Notes\\a b#100%.md", L"C:\\Notes\\\u00E4\u4E2D.md",
                             L"\\\\server\\share\\a b#100%.md", L"C:\\Notes\\%TEMP%.md",
                             L"C:\\Notes\\${FILE_URL}-${URL}.md" }) {
        preview();
        current.value = path;
        RunAction();
        bool matches = false;
        if (launches.size() == 1 && launches[0].arguments.size() > 2) {
            const std::wstring url = launches[0].arguments.substr(1, launches[0].arguments.size() - 2);
            WCHAR decoded[INTERNET_MAX_URL_LENGTH]{};
            DWORD count = COUNTOF(decoded);
            matches = SUCCEEDED(PathCreateFromUrlW(url.c_str(), decoded, &count, 0))
                && current.value == decoded;
            if (current.value.find(L'#') != std::wstring::npos) {
                matches = matches && url.find(L"%23") != std::wstring::npos;
            }
            if (current.value.find(L'%') != std::wstring::npos) {
                matches = matches && url.find(L"%25") != std::wstring::npos;
            }
        }
        check("file URL preserves special characters and UNC identity", matches);
    }
    preview();
    StrgReset(first, L"${FILE_URL}#again=${FILE_URL}");
    RunAction();
    check("all file tokens replaced", launches.size() == 1
        && launches[0].arguments == L"\"file:///C:/Notes/readme.md#again=file:///C:/Notes/readme.md\"");
    preview();
    SetEnvironmentVariableW(L"NP3_WEBTEMPLATE_TEST", L"prefix%20/");
    StrgReset(first, L"%NP3_WEBTEMPLATE_TEST%${FILE_URL}");
    current.value = L"C:\\Notes\\%NP3_WEBTEMPLATE_TEST%.md";
    RunAction();
    check("environment expansion precedes literal URL insertion", launches.size() == 1
        && launches[0].arguments == L"\"prefix%20/file:///C:/Notes/%25NP3_WEBTEMPLATE_TEST%25.md\"");
    SetEnvironmentVariableW(L"NP3_WEBTEMPLATE_TEST", nullptr);
    preview();
    StrgReset(first, L"${FILE_URL}?selection=%s");
    Settings.SaveBeforeRunningTools = true;
    RunAction();
    check("mixed file and selection templates rejected before save",
        launches.empty() && saves == 0 && reportedError == ERROR_INVALID_PARAMETER);
    for (const auto& path : { std::wstring(L"\\\\?\\C:\\Notes\\readme.md"),
                             std::wstring(L"\\\\?\\UNC\\server\\share\\readme.md"),
                             std::wstring(L"C:\\") + std::wstring(INTERNET_MAX_URL_LENGTH, L'x') }) {
        preview();
        current.value = path;
        RunAction();
        check("unsupported path fails without truncated launch", launches.empty() && reportedError != 0);
    }
    preview();
    conversionError = E_POINTER;
    RunAction();
    check("conversion failure reports error without launch",
        launches.empty() && reportedError == ERROR_INSUFFICIENT_BUFFER);
    preview();
    conversionError = E_FAIL;
    RunAction();
    WCHAR errorMessage[512]{};
    check("generic conversion error remains displayable", launches.empty() && reportedError != 0
        && FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
            reportedError, 0, errorMessage, COUNTOF(errorMessage), nullptr) != 0);
    preview();
    launchResult = false;
    RunAction();
    check("explicit browser launch failure reports once without shell error UI or fallback",
        launches.size() == 1 && reportedError == ERROR_FILE_NOT_FOUND && reportedErrors == 1
        && (launches[0].mask & SEE_MASK_FLAG_NO_UI) != 0);

    preview();
    wcscpy_s(Settings2.WebTmpl1MenuName, L"Preview current file");
    wcscpy_s(Settings2.WebTmpl2MenuName, L"Search selected text");
    UpdateMenu(true, false, false, false);
    check("main Launch menu uses configured labels and shortcuts",
        MenuText(Globals.hMainMenu, CMD_WEBACTION1) == L"Preview current file\tCtrl+Shift+1"
        && MenuText(Globals.hMainMenu, CMD_WEBACTION2) == L"Search selected text\tCtrl+Shift+2");
    HMENU context = CreateActionMenu(false);
    EnableMenuItem(context, CMD_WEBACTION1, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(context, CMD_WEBACTION2, MF_BYCOMMAND | MF_GRAYED);
    UpdateMenuOn(context, true, true, false, true);
    check("context menu uses configured labels and shortcuts",
        MenuText(context, CMD_WEBACTION1) == L"Preview current file\tCtrl+Shift+1"
        && MenuText(context, CMD_WEBACTION2) == L"Search selected text\tCtrl+Shift+2");
    check("real popup enables file mode without selection and keeps selection mode disabled",
        GetMenuState(context, CMD_WEBACTION1, MF_BYCOMMAND) == MF_ENABLED
        && (GetMenuState(context, CMD_WEBACTION2, MF_BYCOMMAND) & MF_GRAYED) != 0);
    UpdateMenuOn(context, true, false, false, false);
    check("repeated popup initialization does not duplicate shortcuts",
        MenuText(context, CMD_WEBACTION1) == L"Preview current file\tCtrl+Shift+1");
    StrgReset(first, L"https://example.test/?q=%s");
    StrgReset(second, L"${FILE_URL}");
    UpdateMenuOn(context, true, false, false, false);
    check("second popup slot independently enables file mode without selection",
        (GetMenuState(context, CMD_WEBACTION1, MF_BYCOMMAND) & MF_GRAYED) != 0
        && GetMenuState(context, CMD_WEBACTION2, MF_BYCOMMAND) == MF_ENABLED);
    StrgReset(app, L"");
    UpdateMenuOn(context, true, false, false, false);
    check("popup enables default-browser file preview without selection or association queries",
        GetMenuState(context, CMD_WEBACTION2, MF_BYCOMMAND) == MF_ENABLED && associationQueries == 0);
    DestroyMenu(context);

    preview();
    HMENU originalMain = Globals.hMainMenu;
    Globals.hMainMenu = CreateActionMenu(true);
    ModifyMenuW(Globals.hMainMenu, CMD_WEBACTION1, MF_BYCOMMAND | MF_STRING,
        CMD_WEBACTION1, L"Webvorlage &1\tStrg+Umschalt+1");
    context = CreateActionMenu(false);
    ModifyMenuW(context, CMD_WEBACTION1, MF_BYCOMMAND | MF_STRING, CMD_WEBACTION1, L"Webaktion 1");
    UpdateMenu(true, false, false, false);
    UpdateMenuOn(context, true, false, false, false);
    check("default labels and localized shortcut suffixes are preserved",
        MenuText(Globals.hMainMenu, CMD_WEBACTION1) == L"Webvorlage &1\tStrg+Umschalt+1"
        && MenuText(context, CMD_WEBACTION1) == L"Webaktion 1\tStrg+Umschalt+1");
    DestroyMenu(context);
    DestroyMenu(Globals.hMainMenu);
    Globals.hMainMenu = originalMain;

    reset();
    LoadWebTemplateSettings();
    selection.clear();
    UpdateMenu(true, true, false, true);
    RunAction(CMD_WEBACTION3);
    check("third slot defaults to browser file preview without changing search availability",
        !enabled[CMD_WEBACTION1] && !enabled[CMD_WEBACTION2] && enabled[CMD_WEBACTION3]
        && launches.size() == 1 && launches[0].file == L"file:///C:/Notes/readme.md"
        && launches[0].className == defaultBrowserClass && launches[0].arguments.empty());
    launches.clear();
    selection = L"needle";
    RunAction(CMD_WEBACTION1);
    RunAction(CMD_WEBACTION2);
    check("configuration keeps the original first and second search defaults",
        launches.size() == 2 && launches[0].file == L"https://google.com/search?q=needle"
        && launches[1].file == L"https://en.wikipedia.org/w/index.php?search=needle" && saves == 0);
    reset();
    iniValues[L"WebTemplate1"] = L"${FILE_URL}";
    iniValues[L"WebTmpl1MenuName"] = L"Existing preview";
    iniValues[L"WebTemplate2"] = L"https://custom.test/%s";
    LoadWebTemplateSettings();
    selection.clear();
    RunAction(CMD_WEBACTION1);
    RunAction(CMD_WEBACTION3);
    check("existing first-slot preview is preserved rather than migrated",
        std::wstring(StrgGet(first)) == L"${FILE_URL}" && std::wstring(Settings2.WebTmpl1MenuName) == L"Existing preview"
        && std::wstring(StrgGet(second)) == L"https://custom.test/%s" && launches.size() == 2
        && launches[0].file == launches[1].file && launches[1].className == defaultBrowserClass);
    reset();
    iniValues[L"WebTemplate3"] = L"https://example.test/third/%s";
    LoadWebTemplateSettings();
    Settings.SaveBeforeRunningTools = true;
    selection = L"third";
    UpdateMenu(false, false, true, false);
    RunAction(CMD_WEBACTION3);
    check("third slot can be configured as a selection template without acquiring save behavior",
        enabled[CMD_WEBACTION3] && launches.size() == 1
        && launches[0].file == L"https://example.test/third/third" && saves == 0 && associationQueries == 0);
    reset();
    current.value.clear();
    Settings.SaveBeforeRunningTools = true;
    UpdateMenu(true, false, false, false);
    RunAction(CMD_WEBACTION3);
    check("untitled third-slot preview never saves, queries or launches",
        !enabled[CMD_WEBACTION3] && saves == 0 && associationQueries == 0 && launches.empty());
    reset();
    iniValues[L"WebTemplate3"] = L"";
    LoadWebTemplateSettings();
    UpdateMenu(false, false, true, false);
    RunAction(CMD_WEBACTION3);
    check("explicitly empty third template stays disabled",
        !enabled[CMD_WEBACTION3] && launches.empty() && associationQueries == 0);
    reset();
    StrgReset(app, L"C:\\Browser\\browser.exe");
    RunAction(CMD_WEBACTION3);
    check("third slot honors the shared explicit browser override",
        launches.size() == 1 && launches[0].file == StrgGet(app)
        && launches[0].arguments == L"\"file:///C:/Notes/readme.md\"" && associationQueries == 0);

    reset();
    LoadWebTemplateSettings();
    selection.clear();
    originalMain = Globals.hMainMenu;
    Globals.hMainMenu = LoadMenuW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDR_MUI_MAINMENU));
    context = LoadMenuW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDR_MUI_POPUPMENU));
    UpdateMenu(true, false, false, false);
    UpdateMenuOn(context, true, false, false, false);
    check("production main and context menus expose default preview with its shortcut",
        MenuText(Globals.hMainMenu, CMD_WEBACTION3) == L"Preview current file &3\tCtrl+Shift+3"
        && MenuText(context, CMD_WEBACTION3) == L"Preview current file\tCtrl+Shift+3"
        && GetMenuState(Globals.hMainMenu, CMD_WEBACTION3, MF_BYCOMMAND) == MF_ENABLED
        && GetMenuState(context, CMD_WEBACTION3, MF_BYCOMMAND) == MF_ENABLED);
    HACCEL accelerators = LoadAcceleratorsW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDR_MAINWND));
    const int acceleratorCount = CopyAcceleratorTableW(accelerators, nullptr, 0);
    std::vector<ACCEL> keys(acceleratorCount);
    CopyAcceleratorTableW(accelerators, keys.data(), acceleratorCount);
    int previewShortcuts = 0;
    for (const auto& key : keys) {
        if (key.key == L'3' && (key.fVirt & (FVIRTKEY | FCONTROL | FSHIFT | FALT)) == (FVIRTKEY | FCONTROL | FSHIFT)) {
            ++previewShortcuts;
            RunAction(key.cmd);
        }
    }
    check("production Ctrl+Shift+3 accelerator dispatches preview without a conflicting binding",
        previewShortcuts == 1 && launches.size() == 1 && launches[0].file == L"file:///C:/Notes/readme.md"
        && launches[0].className == defaultBrowserClass);
    iniValues[L"WebTmpl3MenuName"] = L"My Markdown preview";
    LoadWebTemplateSettings();
    UpdateMenu(true, false, false, false);
    UpdateMenuOn(context, true, false, false, false);
    check("third-slot configured name and shortcut reach both production menus",
        MenuText(Globals.hMainMenu, CMD_WEBACTION3) == L"My Markdown preview\tCtrl+Shift+3"
        && MenuText(context, CMD_WEBACTION3) == L"My Markdown preview\tCtrl+Shift+3");
    DestroyMenu(context);
    DestroyMenu(Globals.hMainMenu);
    Globals.hMainMenu = originalMain;
    DestroyMenu(Globals.hMainMenu);
    for (const auto handle : { first, second, third, app, arguments }) { StrgDestroy(handle); }
    std::cout << checks << " WebTemplate checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
