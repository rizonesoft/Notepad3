#include <windows.h>
#include <strsafe.h>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <initializer_list>

#define COUNTOF(a) static_cast<int>(sizeof(a) / sizeof((a)[0]))

#include "ZoomTooltipPositionTypes.inc"

using DocPos = ptrdiff_t;
using DocLn = ptrdiff_t;

static struct {
    HWND hwndEdit;
} Globals;
static struct {
    int ZoomTooltipTimeout = 3200;
    int ZoomTooltipPosition = ZOOM_TOOLTIP_UPPER_LEFT;
} Settings2;
static char editorInstance;
static UINT testDpi = 96;
static DocPos currentPos = 400;
static POINT caretPoint = { 100, 100 };
static bool caretLineVisible = true;
static POINT mouseScreen = {};
static bool cursorValid = true;
static int cursorQueries = 0;
static bool conversionValid = true;
static int xOffset = 120;
static int offsetChanges = 0;
static int tipShows = 0;
static int tipCancels = 0;
static DocPos shownPos = -1;
static int shownXOffset = 0;
static bool tipAbove = false;
static char shownText[64] = {};
static int delayCalls = 0;
static LONG64 scheduledDelay = 0;
static HWND simulatedTip = nullptr;
static bool hasIniValue = false;
static int iniValue = 0;
static RECT monitorWorkArea = { -32000, -32000, 32000, 32000 };
static bool monitorInfoValid = true;

static const char* SciCall_GetDirectPointer()
{
    return &editorInstance;
}

static long max_l(long x, long y)
{
    return x > y ? x : y;
}

static long min_l(long x, long y)
{
    return x < y ? x : y;
}

static int ScaleIntToDPI(HWND, int value) { return MulDiv(value, testDpi, USER_DEFAULT_SCREEN_DPI); }
static int SciCall_GetMarginLeft() { return 6; }
static int SciCall_GetMarginRight() { return 8; }
static int SciCall_GetMargins() { return 2; }
static int SciCall_GetMarginWidthN(int index) { return index == 0 ? 20 : 12; }
static DocPos SciCall_GetCurrentPos() { return currentPos; }
static DocLn SciCall_LineFromPosition(DocPos pos) { return pos / 10; }
static bool SciCall_GetLineVisible(DocLn) { return caretLineVisible; }
static int SciCall_PointXFromPosition(DocPos) { return caretPoint.x; }
static int SciCall_PointYFromPosition(DocPos) { return caretPoint.y; }
static DocLn SciCall_GetFirstVisibleLine() { return 10; }
static DocPos SciCall_PositionFromLine(DocLn line) { return line * 10; }
static int SciCall_GetXOffset() { return xOffset; }
static void SciCall_SetXOffset(int offset) { xOffset = offset; ++offsetChanges; }
static int NP3_GetZoomPercent() { return 125; }
static void SciCall_CallTipSetPosition(bool above) { tipAbove = above; }
static void SciCall_CallTipCancel() { ++tipCancels; }
#define Sci_CallTipCancelEx() { SciCall_CallTipCancel(); SciCall_CallTipSetPosition(false); }

static void SciCall_CallTipShow(DocPos pos, const char* text)
{
    ++tipShows;
    shownPos = pos;
    shownXOffset = xOffset;
    StringCchCopyA(shownText, COUNTOF(shownText), text);
    if (simulatedTip) {
        SetWindowPos(simulatedTip, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

static void _DelayClearCallTip(LONG64 delay)
{
    ++delayCalls;
    scheduledDelay = delay;
}

static BOOL TestGetCursorPos(LPPOINT point)
{
    ++cursorQueries;
    *point = mouseScreen;
    return cursorValid;
}

static BOOL TestScreenToClient(HWND hwnd, LPPOINT point)
{
    return conversionValid && ScreenToClient(hwnd, point);
}

static HMONITOR TestMonitorFromRect(const RECT*, DWORD)
{
    return reinterpret_cast<HMONITOR>(1);
}

static BOOL TestGetMonitorInfo(HMONITOR, LPMONITORINFO info)
{
    info->rcWork = monitorWorkArea;
    return monitorInfoValid;
}

static constexpr LPCWSTR IniSecSettings2 = L"Settings2";
static int IniSectionGetInt(LPCWSTR section, LPCWSTR key, int fallback)
{
    return hasIniValue && std::wcscmp(section, L"Settings2") == 0 && std::wcscmp(key, L"ZoomTooltipPosition") == 0
        ? iniValue : fallback;
}

#define GetCursorPos TestGetCursorPos
#define ScreenToClient TestScreenToClient
#define MonitorFromRect TestMonitorFromRect
#define GetMonitorInfoW TestGetMonitorInfo
#include "ZoomTooltipPosition.inc"
#undef GetMonitorInfoW
#undef MonitorFromRect
#undef ScreenToClient
#undef GetCursorPos

static int failures = 0;
static int checks = 0;

static void Check(bool condition, const char* message)
{
    ++checks;
    if (!condition) {
        std::printf("FAIL: %s\n", message);
        ++failures;
    }
}

static bool HasRect(HWND hwnd, LONG x, LONG y, LONG width, LONG height)
{
    RECT rc = {};
    return GetWindowRect(hwnd, &rc) && rc.left == x && rc.top == y
        && rc.right - rc.left == width && rc.bottom - rc.top == height;
}

static void ResetObservation()
{
    cursorQueries = 0;
    offsetChanges = 0;
    tipShows = 0;
    tipCancels = 0;
    delayCalls = 0;
    shownPos = -1;
}

static void CheckShown(DocPos pos, bool above, int expectedOffsetChanges, const char* message)
{
    Check(tipShows == 1 && tipCancels == 0 && shownPos == pos && tipAbove == above
        && xOffset == 120 && offsetChanges == expectedOffsetChanges && currentPos == 400
        && delayCalls == 1 && scheduledDelay == Settings2.ZoomTooltipTimeout, message);
}

static void SetMouseClient(LONG x, LONG y)
{
    mouseScreen = { x, y };
    ClientToScreen(Globals.hwndEdit, &mouseScreen);
}

int main()
{
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = DefWindowProcW;
    wc.cbWndExtra = sizeof(LONG_PTR);
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"CallTip";
    if (!RegisterClassExW(&wc)) {
        return 1;
    }

    HWND owner = CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 100, 100, 640, 480, nullptr, nullptr, wc.hInstance, nullptr);
    Globals.hwndEdit = CreateWindowExW(0, L"STATIC", L"", WS_CHILD, 0, 0, 640, 480, owner, nullptr, wc.hInstance, nullptr);
    HWND tip = CreateWindowExW(0, L"CallTip", L"", WS_POPUP, 0, 0, 96, 24, Globals.hwndEdit, nullptr, wc.hInstance, nullptr);
    HWND otherTip = CreateWindowExW(0, L"CallTip", L"", WS_POPUP, 10, 20, 96, 24, Globals.hwndEdit, nullptr, wc.hInstance, nullptr);
    HWND otherOwner = CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
    HWND foreignTip = CreateWindowExW(0, L"CallTip", L"", WS_POPUP, 30, 40, 96, 24, otherOwner, nullptr, wc.hInstance, nullptr);
    if (!owner || !Globals.hwndEdit || !tip || !otherTip || !otherOwner || !foreignTip) {
        return 1;
    }
    SetWindowLongPtrW(tip, 0, reinterpret_cast<LONG_PTR>(&editorInstance));
    SetWindowLongPtrW(foreignTip, 0, reinterpret_cast<LONG_PTR>(&editorInstance));

    Check(GetWindow(tip, GW_OWNER) == owner, "child editor's popup is owned by its top-level window");
    HWND const active = GetActiveWindow();
    HWND const focus = GetFocus();
    RECT bounds = { 150, 110, 730, 560 };
    _PositionZoomCallTip(&bounds, nullptr);
    Check(HasRect(tip, 630, 114, 96, 24), "positions measured popup at upper-right in screen coordinates");
    Check(HasRect(otherTip, 10, 20, 96, 24), "does not move another editor's call-tip");
    Check(HasRect(foreignTip, 30, 40, 96, 24), "does not move a call-tip with another owner");
    Check(!IsWindowVisible(tip), "positioning does not show a hidden popup");
    Check(GetActiveWindow() == active && GetFocus() == focus, "positioning does not activate or focus");

    bounds = { -500, 60, -450, 80 };
    _PositionZoomCallTip(&bounds, nullptr);
    Check(HasRect(tip, -500, 60, 96, 24), "oversized popup clamps to viewport left without resizing");
    DestroyWindow(tip);
    tip = CreateWindowExW(0, L"CallTip", L"", WS_POPUP, 0, 0, 120, 32, Globals.hwndEdit, nullptr, wc.hInstance, nullptr);
    SetWindowLongPtrW(tip, 0, reinterpret_cast<LONG_PTR>(&editorInstance));
    bounds = { 150, 110, 730, 560 };
    testDpi = 192;
    _PositionZoomCallTip(&bounds, nullptr);
    Check(HasRect(tip, 602, 118, 120, 32), "finds recreated popup and scales inset for new DPI and dimensions");
    testDpi = 96;

    monitorWorkArea = { 0, 0, 700, 500 };
    bounds = { 150, -20, 850, 560 };
    _PositionZoomCallTip(&bounds, nullptr);
    Check(HasRect(tip, 580, 0, 120, 32), "upper-right remains in monitor work area when editor extends off-screen");
    monitorInfoValid = false;
    _PositionZoomCallTip(&bounds, nullptr);
    Check(HasRect(tip, 726, -16, 120, 32), "failed monitor query retains viewport placement");
    monitorInfoValid = true;
    monitorWorkArea = { -32000, -32000, 32000, 32000 };

    LoadZoomTooltipPosition();
    Check(Settings2.ZoomTooltipPosition == ZOOM_TOOLTIP_UPPER_LEFT, "missing INI value preserves upper-left default");
    hasIniValue = true;
    for (int value : { -1, 0, 1, 2, 3, 4, INT_MAX }) {
        iniValue = value;
        LoadZoomTooltipPosition();
        Check(Settings2.ZoomTooltipPosition == (value >= 0 && value <= 3 ? value : 0), "loads supported modes and rejects invalid INI values");
    }

    RECT viewport = {};
    Check(_GetZoomCallTipViewport(&viewport) && viewport.left == 38 && viewport.right == 632
        && viewport.top == 0 && viewport.bottom == 480, "text viewport excludes all margins");
    Settings2.ZoomTooltipPosition = ZOOM_TOOLTIP_UPPER_LEFT;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "default retains legacy anchor, offset workaround, restored scrolling, and timer");
    Check(shownXOffset == -4 && std::strcmp(shownText, "Zoom: 125%") == 0, "legacy offset at display and percentage formatting are unchanged");

    Settings2.ZoomTooltipPosition = ZOOM_TOOLTIP_CARET;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(400, false, 0, "visible caret anchors below text without modifying horizontal scrolling");
    caretPoint.y = -1;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "caret vertically off-screen falls back to upper-left");
    caretPoint = { 37, 100 };
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "caret horizontally scrolled out of text area falls back");
    caretPoint = { 632, 100 };
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "caret at right clipping edge falls back");
    caretPoint = { 100, 480 };
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "caret below bottom clipping edge falls back");
    caretPoint = { 100, 100 };
    caretLineVisible = false;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "folded caret line falls back even if coordinate query looks visible");
    caretLineVisible = true;

    Settings2.ZoomTooltipPosition = ZOOM_TOOLTIP_MOUSE;
    simulatedTip = tip;
    POINT origin = { 0, 0 };
    ClientToScreen(Globals.hwndEdit, &origin);
    SetMouseClient(300, 240);
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(400, false, 0, "mouse mode measures popup without changing caret or horizontal scrolling");
    Check(HasRect(tip, origin.x + 304, origin.y + 244, 120, 32), "mouse mode uses sampled pixel coordinates with an inset");
    Check(cursorQueries == 1, "mouse positioning samples the pointer only once");
    caretPoint.x = -100;
    ResetObservation();
    ShowZoomCallTip();
    Check(HasRect(tip, origin.x + 304, origin.y + 244, 120, 32), "mouse mode includes virtual space beyond a horizontally scrolled line ending");
    caretPoint.y = -1;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(400, false, 0, "mouse pixel placement works even when caret and nearest text are off-screen");
    Check(HasRect(tip, origin.x + 304, origin.y + 244, 120, 32), "off-screen caret does not change pixel placement");
    caretPoint = { 100, 100 };
    caretLineVisible = false;
    ResetObservation();
    ShowZoomCallTip();
    Check(HasRect(tip, origin.x + 304, origin.y + 244, 120, 32), "mouse pixel placement does not depend on folded caret line visibility");
    caretLineVisible = true;
    currentPos = 0;
    SetMouseClient(500, 400);
    ResetObservation();
    ShowZoomCallTip();
    Check(shownPos == 0 && HasRect(tip, origin.x + 504, origin.y + 404, 120, 32), "empty document stays near pointer instead of snapping to position zero");
    currentPos = 400;
    SetMouseClient(630, 478);
    ResetObservation();
    ShowZoomCallTip();
    Check(HasRect(tip, origin.x + 506, origin.y + 442, 120, 32), "mouse popup flips left and above near viewport edges");
    SetMouseClient(300, 240);
    testDpi = 192;
    ResetObservation();
    ShowZoomCallTip();
    Check(HasRect(tip, origin.x + 308, origin.y + 248, 120, 32), "mouse offset scales with editor DPI");
    testDpi = 96;
    monitorWorkArea = { origin.x, origin.y, origin.x + 400, origin.y + 480 };
    SetMouseClient(398, 478);
    ResetObservation();
    ShowZoomCallTip();
    Check(HasRect(tip, origin.x + 274, origin.y + 442, 120, 32), "mouse popup flips before clamping to monitor work-area edges");
    monitorWorkArea = { -32000, -32000, 32000, 32000 };
    SetMouseClient(700, 240);
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(400, false, 0, "mouse outside editor falls back to visible caret");
    Check(HasRect(tip, 0, 0, 120, 32), "outside mouse does not trigger native mouse placement");
    SetMouseClient(10, 100);
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(400, false, 0, "mouse over gutter uses visible caret instead of gutter text mapping");
    caretPoint.y = -1;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "outside mouse and invisible caret fall back to upper-left");
    caretPoint.y = 100;
    SetMouseClient(300, 240);
    cursorValid = false;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(400, false, 0, "failed pointer query uses visible caret");
    cursorValid = true;
    conversionValid = false;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(400, false, 0, "failed coordinate conversion uses visible caret");
    conversionValid = true;

    Settings2.ZoomTooltipPosition = ZOOM_TOOLTIP_UPPER_RIGHT;
    simulatedTip = tip;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "upper-right reuses existing display and timeout lifecycle");
    Check(HasRect(tip, origin.x + 632 - 120 - 4, origin.y + 4, 120, 32), "upper-right translates text viewport to screen and uses actual popup dimensions");
    ResetObservation();
    ShowZoomCallTip();
    Check(HasRect(tip, origin.x + 632 - 120 - 4, origin.y + 4, 120, 32), "repeated zoom repositions after Scintilla resets popup placement");
    simulatedTip = nullptr;

    SetWindowPos(Globals.hwndEdit, nullptr, 0, 0, 20, 20, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    Settings2.ZoomTooltipPosition = ZOOM_TOOLTIP_CARET;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "empty text viewport safely uses legacy placement");

    for (int timeout : { 0, (_MQ_TIMER_CYCLE << 3) - 1 }) {
        Settings2.ZoomTooltipTimeout = timeout;
        ResetObservation();
        ShowZoomCallTip();
        Check(tipShows == 0 && tipCancels == 1 && !tipAbove && delayCalls == 0 && offsetChanges == 0,
            "disabled or below-threshold timeout only cancels and resets call-tip orientation");
    }
    Settings2.ZoomTooltipTimeout = _MQ_TIMER_CYCLE << 3;
    ResetObservation();
    ShowZoomCallTip();
    CheckShown(100, true, 2, "existing timer threshold still displays the indicator");

    DestroyWindow(tip);
    DestroyWindow(otherTip);
    DestroyWindow(foreignTip);
    DestroyWindow(otherOwner);
    DestroyWindow(owner);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    std::printf("Zoom tooltip position: %s (%d checks)\n", failures ? "FAILED" : "PASS", checks);
    return failures ? 1 : 0;
}
