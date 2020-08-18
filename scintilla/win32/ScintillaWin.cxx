// Scintilla source code edit control
/** @file ScintillaWin.cxx
 ** Windows specific subclass of ScintillaBase.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <climits>

#include <stdexcept>
#include <new>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <chrono>
//#include <mutex>

// Want to use std::min and std::max so don't want Windows.h version of min and max
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#ifndef WINVER
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif

#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <windowsx.h>
#include <zmouse.h>
#include <ole2.h>

#if !defined(DISABLE_D2D)
#define USE_D2D 1
#endif

#if defined(USE_D2D)
#include <d2d1.h>
#include <dwrite.h>
#endif

#include <shlwapi.h>
#include <shlobj.h>
#include <shellapi.h>
#include <VersionHelpers.h>

#define DebugDragAndDropDataFormat		0
#define MaxDragAndDropDataFormatCount	6
/*
CF_VSSTGPROJECTITEMS, CF_VSREFPROJECTITEMS
https://docs.microsoft.com/en-us/visualstudio/extensibility/ux-guidelines/application-patterns-for-visual-studio
*/
#define EnableDrop_VisualStudioProjectItem		1
/*
Chromium Web Custom MIME Data Format
Used by VSCode, Atom etc.
*/
#define Enable_ChromiumWebCustomMIMEDataFormat	0

#include "Platform.h"

#include "ILoader.h"
#include "ILexer.h"
#include "Scintilla.h"

//~#include "CharacterCategory.h"
#include "Position.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "CaseConvert.h"
#include "UniConversion.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "ElapsedPeriod.h"

#include "AutoComplete.h"
#include "ScintillaBase.h"

#include "PlatWin.h"
#include "HanjaDic.h"
#include "ScintillaWin.h"

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

#ifndef WM_UNICHAR
#define WM_UNICHAR                      0x0109
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif
#ifndef WM_DPICHANGED_AFTERPARENT
#define WM_DPICHANGED_AFTERPARENT 0x02E3
#endif

#ifndef UNICODE_NOCHAR
#define UNICODE_NOCHAR                  0xFFFF
#endif

#ifndef IS_HIGH_SURROGATE
#define IS_HIGH_SURROGATE(x)            ((x) >= SURROGATE_LEAD_FIRST && (x) <= SURROGATE_LEAD_LAST)
#endif

#ifndef IS_LOW_SURROGATE
#define IS_LOW_SURROGATE(x)             ((x) >= SURROGATE_TRAIL_FIRST && (x) <= SURROGATE_TRAIL_LAST)
#endif

#ifndef MK_ALT
#define MK_ALT 32
#endif

// Two idle messages SC_WIN_IDLE and SC_WORK_IDLE.

// SC_WIN_IDLE is low priority so should occur after the next WM_PAINT
// It is for lengthy actions like wrapping and background styling
constexpr UINT SC_WIN_IDLE = 5001;
// SC_WORK_IDLE is high priority and should occur before the next WM_PAINT
// It is for shorter actions like restyling the text just inserted
// and delivering SCN_UPDATEUI
constexpr UINT SC_WORK_IDLE = 5002;

#define SC_INDICATOR_INPUT INDICATOR_IME
#define SC_INDICATOR_TARGET (INDICATOR_IME + 1)
#define SC_INDICATOR_CONVERTED (INDICATOR_IME + 2)
#define SC_INDICATOR_UNKNOWN INDICATOR_IME_MAX

#if _WIN32_WINNT < _WIN32_WINNT_WIN8
DWORD	kSystemLibraryLoadFlags = 0;
using SetCoalescableTimerSig = UINT_PTR (WINAPI *)(HWND hwnd, UINT_PTR nIDEvent,
	UINT uElapse, TIMERPROC lpTimerFunc, ULONG uToleranceDelay);
#endif

using namespace Scintilla;

namespace {

const TCHAR *callClassName = L"CallTip";

inline void SetWindowID(HWND hWnd, int identifier) noexcept {
	::SetWindowLongPtr(hWnd, GWLP_ID, identifier);
}

constexpr Point PointFromLParam(sptr_t lpoint) noexcept {
	return Point::FromInts(GET_X_LPARAM(lpoint), GET_Y_LPARAM(lpoint));
}

inline bool KeyboardIsKeyDown(int key) noexcept {
    // the return value is a SHORT (16 bits), not a 32 bit value
    // (in other words, 0x80000000 is not a valid bit mask)
	return (::GetKeyState(key) & 0x8000) != 0;
}

constexpr bool KeyboardIsNumericKeypadFunction(uptr_t wParam, sptr_t lParam) noexcept {
	// Bit 24 is the extended keyboard flag and the numeric keypad is non-extended
	if ((lParam & (1 << 24)) != 0) {
		// Not from the numeric keypad
		return false;
	}

	switch (wParam) {
	case VK_INSERT:	// 0
	case VK_END:	// 1
	case VK_DOWN:	// 2
	case VK_NEXT:	// 3
	case VK_LEFT:	// 4
	case VK_CLEAR:	// 5
	case VK_RIGHT:	// 6
	case VK_HOME:	// 7
	case VK_UP:		// 8
	case VK_PRIOR:	// 9
		return true;
	default:
		return false;
	}
}

inline CLIPFORMAT GetClipboardFormat(LPCWSTR name) noexcept {
	return static_cast<CLIPFORMAT>(::RegisterClipboardFormat(name));
}

#if 0
inline void LazyGetClipboardFormat(UINT &fmt, LPCWSTR name) noexcept {
	if (fmt == 0) {
		fmt = ::RegisterClipboardFormat(name);
	}
}
#endif

}

class ScintillaWin; 	// Forward declaration for COM interface subobjects

/**
 */
class FormatEnumerator : public IEnumFORMATETC {
	ULONG ref;
	ULONG pos;
	std::vector<CLIPFORMAT> formats;

public:
	FormatEnumerator(ULONG pos_, const CLIPFORMAT formats_[], size_t formatsLen_);
	virtual ~FormatEnumerator() = default;

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv) noexcept override;
	STDMETHODIMP_(ULONG)AddRef() noexcept override;
	STDMETHODIMP_(ULONG)Release() noexcept override;

	// IEnumFORMATETC
	STDMETHODIMP Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched) noexcept override;
	STDMETHODIMP Skip(ULONG celt) noexcept override;
	STDMETHODIMP Reset() noexcept override;
	STDMETHODIMP Clone(IEnumFORMATETC **ppenum) override;
};

/**
 */
class DropSource : public IDropSource {
public:
	ScintillaWin *sci = nullptr;
	DropSource() noexcept = default;
	virtual ~DropSource() = default;

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv) noexcept override;
	STDMETHODIMP_(ULONG)AddRef() noexcept override;
	STDMETHODIMP_(ULONG)Release() noexcept override;

	// IDropSource
	STDMETHODIMP QueryContinueDrag(BOOL fEsc, DWORD grfKeyState) noexcept override;
	STDMETHODIMP GiveFeedback(DWORD) noexcept override;
};

/**
 */
class DataObject : public IDataObject {
public:
	ScintillaWin *sci = nullptr;
	DataObject() noexcept = default;
	virtual ~DataObject() = default;

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv) noexcept override;
	STDMETHODIMP_(ULONG)AddRef() noexcept override;
	STDMETHODIMP_(ULONG)Release() noexcept override;

	// IDataObject
	STDMETHODIMP GetData(FORMATETC *pFEIn, STGMEDIUM *pSTM) override;
	STDMETHODIMP GetDataHere(FORMATETC *, STGMEDIUM *) noexcept override;
	STDMETHODIMP QueryGetData(FORMATETC *pFE) noexcept override;
	STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *, FORMATETC *pFEOut) noexcept override;
	STDMETHODIMP SetData(FORMATETC *, STGMEDIUM *, BOOL) noexcept override;
	STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnum) override;
	STDMETHODIMP DAdvise(FORMATETC *, DWORD, IAdviseSink *, PDWORD) noexcept override;
	STDMETHODIMP DUnadvise(DWORD) noexcept override;
	STDMETHODIMP EnumDAdvise(IEnumSTATDATA **) noexcept override;
};

/**
 */
class DropTarget : public IDropTarget {
public:
	ScintillaWin *sci = nullptr;
	DropTarget() noexcept = default;
	virtual ~DropTarget() = default;

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv) noexcept override;
	STDMETHODIMP_(ULONG)AddRef() noexcept override;
	STDMETHODIMP_(ULONG)Release() noexcept override;

	// IDropTarget
	STDMETHODIMP DragEnter(LPDATAOBJECT pIDataSource, DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) override;
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) override;
	STDMETHODIMP DragLeave() override;
	STDMETHODIMP Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) override;
};

namespace {

// InputLanguage() and SetCandidateWindowPos() are based on Chromium's IMM32Manager and InputMethodWinImm32.
// https://github.com/chromium/chromium/blob/master/ui/base/ime/win/imm32_manager.cc
// See License.txt or https://github.com/chromium/chromium/blob/master/LICENSE for license details.

// See Chromium's IMM32Manager::SetInputLanguage()
LANGID InputLanguage() noexcept {
	// Retrieve the current input language from the system's keyboard layout.
	// Using GetKeyboardLayoutName instead of GetKeyboardLayout, because
	// the language from GetKeyboardLayout is the language under where the
	// keyboard layout is installed. And the language from GetKeyboardLayoutName
	// indicates the language of the keyboard layout itself.
	// See crbug.com/344834.
	LANGID inputLang;
	WCHAR keyboard_layout[KL_NAMELENGTH];
	if (::GetKeyboardLayoutNameW(keyboard_layout)) {
		inputLang = static_cast<LANGID>(wcstol(&keyboard_layout[KL_NAMELENGTH >> 1], nullptr, 16));
	} else {
		/// TODO: Fallback to en-US?
		HKL inputLocale = ::GetKeyboardLayout(0);
		inputLang = LOWORD(inputLocale);
	}
	//Platform::DebugPrintf("InputLanguage(): %04X\n", inputLang);
	return inputLang;
}

class IMContext {
	HWND hwnd;
public:
	HIMC hIMC;
	explicit IMContext(HWND hwnd_) noexcept :
		hwnd(hwnd_), hIMC(::ImmGetContext(hwnd_)) {}
	// Deleted so IMContext objects can not be copied.
	IMContext(const IMContext &) = delete;
	IMContext(IMContext &&) = delete;
	IMContext &operator=(const IMContext &) = delete;
	IMContext &operator=(IMContext &&) = delete;
	~IMContext() {
		if (hIMC)
			::ImmReleaseContext(hwnd, hIMC);
	}

	operator bool() const noexcept {
		return hIMC != nullptr;
	}

	LONG GetImeCaretPos() const noexcept {
		return ImmGetCompositionStringW(hIMC, GCS_CURSORPOS, nullptr, 0);
	}

	std::vector<BYTE> GetImeAttributes() const {
		const LONG attrLen = ::ImmGetCompositionStringW(hIMC, GCS_COMPATTR, nullptr, 0);
		std::vector<BYTE> attr(attrLen, 0);
		::ImmGetCompositionStringW(hIMC, GCS_COMPATTR, attr.data(), static_cast<DWORD>(attr.size()));
		return attr;
	}

	LONG HasCompositionString(DWORD dwIndex) const noexcept {
		return hIMC ? ::ImmGetCompositionStringW(hIMC, dwIndex, nullptr, 0) : 0;
	}

	std::wstring GetCompositionString(DWORD dwIndex) const {
		const LONG byteLen = ::ImmGetCompositionStringW(hIMC, dwIndex, nullptr, 0);
		std::wstring wcs(byteLen / sizeof(wchar_t), 0);
		::ImmGetCompositionStringW(hIMC, dwIndex, wcs.data(), byteLen);
		return wcs;
	}
};

class GlobalMemory;

class ReverseArrowCursor {
	DPI_T dpi = { USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI };
	HCURSOR cursor {};

public:
	ReverseArrowCursor() noexcept = default;
	// Deleted so ReverseArrowCursor objects can not be copied.
	ReverseArrowCursor(const ReverseArrowCursor &) = delete;
	ReverseArrowCursor(ReverseArrowCursor &&) = delete;
	ReverseArrowCursor &operator=(const ReverseArrowCursor &) = delete;
	ReverseArrowCursor &operator=(ReverseArrowCursor &&) = delete;
	~ReverseArrowCursor() {
		if (cursor) {
			::DestroyCursor(cursor);
		}
	}

	HCURSOR Load(DPI_T dpi_) noexcept {
		if (cursor)	 {
			if ((dpi.x == dpi_.x) && (dpi.y == dpi_.y)) {
				return cursor;
			}
			::DestroyCursor(cursor);
		}

		dpi = dpi_;
		cursor = LoadReverseArrowCursor(dpi_);
		return cursor ? cursor : ::LoadCursor({}, IDC_ARROW);
	}
};

}

/**
 */
class ScintillaWin :
	public ScintillaBase {

	bool lastKeyDownConsumed;
	wchar_t lastHighSurrogateChar;

	bool capturedMouse;
	bool trackedMouseLeave;
#if _WIN32_WINNT < _WIN32_WINNT_WIN8
	SetCoalescableTimerSig SetCoalescableTimerFn;
#endif

	unsigned int linesPerScroll;	///< Intellimouse support
	int wheelDelta; ///< Wheel delta from roll

	DPI_T dpi = { USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI };
	ReverseArrowCursor reverseArrowCursor;

	HRGN hRgnUpdate;

	bool hasOKText;

	CLIPFORMAT cfColumnSelect;
	UINT cfBorlandIDEBlockType;
	UINT cfLineSelect;
	UINT cfVSLineTag;

#if EnableDrop_VisualStudioProjectItem
	CLIPFORMAT cfVSStgProjectItem;
	CLIPFORMAT cfVSRefProjectItem;
#endif
#if Enable_ChromiumWebCustomMIMEDataFormat
	CLIPFORMAT cfChromiumCustomMIME;
#endif

	// supported drag & drop format
	CLIPFORMAT dropFormat[MaxDragAndDropDataFormatCount];
	UINT dropFormatCount;

	//HRESULT hrOle;
	DropSource ds;
	DataObject dob;
	DropTarget dt;

	static HINSTANCE hInstance;
	static ATOM scintillaClassAtom;
	static ATOM callClassAtom;

	// The current input Language ID.
	LANGID inputLang;

#if defined(USE_D2D)
	ID2D1RenderTarget *pRenderTarget;
	bool renderTargetValid;
#endif

	explicit ScintillaWin(HWND hwnd);
	// virtual ~ScintillaWin() in public section

	void Init() noexcept;
	void Finalise() noexcept override;
#if defined(USE_D2D)
	void EnsureRenderTarget(HDC hdc) noexcept;
	void DropRenderTarget() noexcept;
#endif
	HWND MainHWND() const noexcept;
#if DebugDragAndDropDataFormat
	void EnumDataSourceFormat(const char *tag, LPDATAOBJECT pIDataSource);
	void EnumAllClipboardFormat(const char *tag);
#else
	#define EnumDataSourceFormat(tag, pIDataSource)
	#define EnumAllClipboardFormat(tag)
#endif

	static sptr_t DirectFunction(
		sptr_t ptr, UINT iMessage, uptr_t wParam, sptr_t lParam);
	static LRESULT CALLBACK SWndProc(
		HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK CTWndProc(
		HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	enum : UINT_PTR {
		invalidTimerID, standardTimerID, idleTimerID, fineTimerStart
	};

	void DisplayCursor(Window::Cursor c) noexcept override;
	bool DragThreshold(Point ptStart, Point ptNow) noexcept override;
	void StartDrag() override;
	static int MouseModifiers(uptr_t wParam) noexcept;

	Sci::Position TargetAsUTF8(char *text) const;
	Sci::Position EncodedFromUTF8(const char *utf8, char *encoded) const;

	bool PaintDC(HDC hdc);
	sptr_t WndPaint();

	sptr_t HandleCompositionWindowed(uptr_t wParam, sptr_t lParam);
	sptr_t HandleCompositionInline(uptr_t wParam, sptr_t lParam);
	static bool KoreanIME() noexcept;
// >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
	bool IsIMEOpen();
	DWORD GetIMEInputMode();
// <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<
	void MoveImeCarets(Sci::Position offset) noexcept;
	void DrawImeIndicator(int indicator, Sci::Position len);
	void SetCandidateWindowPos();
	void SelectionToHangul();
	void EscapeHanja();
	void ToggleHanja();
	void AddWString(std::wstring_view wsv, CharacterSource charSource);

	UINT CodePageOfDocument() const noexcept;
	bool ValidCodePage(int codePage) const noexcept override;
	std::string EncodeWString(std::wstring_view wsv);
	sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) noexcept override;
	void IdleWork() override;
	void QueueIdleWork(WorkNeeded::workItems items, Sci::Position upTo) noexcept override;
	bool SetIdle(bool on) noexcept override;
	UINT_PTR timers[tickDwell + 1]{};
	bool FineTickerRunning(TickReason reason) noexcept override;
	void FineTickerStart(TickReason reason, int millis, int tolerance) noexcept override;
	void FineTickerCancel(TickReason reason) noexcept override;
	void SetMouseCapture(bool on) noexcept override;
	bool HaveMouseCapture() noexcept override;
	void SetTrackMouseLeaveEvent(bool on) noexcept;
	bool PaintContains(PRectangle rc) const noexcept override;
	void ScrollText(Sci::Line linesToMove) override;
	void NotifyCaretMove() noexcept override;
	void UpdateSystemCaret() override;
	void SetVerticalScrollPos() override;
	void SetHorizontalScrollPos() override;
	bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;
	void NotifyChange() noexcept override;
	void NotifyFocus(bool focus) override;
	void SetCtrlID(int identifier) noexcept override;
	int GetCtrlID() const noexcept override;
	void NotifyParent(SCNotification scn) noexcept override;
	void NotifyDoubleClick(Point pt, int modifiers) override;
	void NotifyURIDropped(const char *list) noexcept;
	CaseFolder *CaseFolderForEncoding() override;
	std::string CaseMapString(const std::string &s, int caseMapping) override;
	void Copy(bool asBinary) override;
	bool CanPaste() noexcept override;
	void Paste(bool asBinary) override;
	void CreateCallTipWindow(PRectangle rc) noexcept override;
#if SCI_EnablePopupMenu
	void AddToPopUp(const char *label, int cmd = 0, bool enabled = true) noexcept override;
#endif
	void ClaimSelection() noexcept override;

	// DBCS
	void ImeStartComposition();
	void ImeEndComposition();
	LRESULT ImeOnReconvert(LPARAM lParam);

	enum class CopyEncoding {
		Unicode,	// used in Copy & Paste, Drag & Drop
		Ansi,		// used in Drag & Drop for CF_TEXT
		Binary,		// used in Copy & Paste for asBinary
	};

	void GetIntelliMouseParameters() noexcept;
	void CopyToGlobal(GlobalMemory &gmUnicode, const SelectionText &selectedText, CopyEncoding encoding);
	void CopyToClipboard(const SelectionText &selectedText) override;
	void ScrollMessage(WPARAM wParam);
	void HorizontalScrollMessage(WPARAM wParam);
	void FullPaint();
	void FullPaintDC(HDC hdc);
	bool IsCompatibleDC(HDC hOtherDC) const noexcept;
	DWORD EffectFromState(DWORD grfKeyState) const noexcept;

	int SetScrollInfo(int nBar, LPCSCROLLINFO lpsi, BOOL bRedraw) const noexcept;
	bool GetScrollInfo(int nBar, LPSCROLLINFO lpsi) const noexcept;
	void ChangeScrollPos(int barType, Sci::Position pos);
	sptr_t GetTextLength() const noexcept;
	sptr_t GetText(uptr_t wParam, sptr_t lParam) const;
	Window::Cursor SCICALL ContextCursor(Point pt);
#if SCI_EnablePopupMenu
	sptr_t ShowContextMenu(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
#endif
	void SizeWindow();
	sptr_t MouseMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	sptr_t KeyMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	sptr_t FocusMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	sptr_t IMEMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	sptr_t EditMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	sptr_t IdleMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	sptr_t SciMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam);

public:
	virtual ~ScintillaWin() override;

	// Deleted so ScintillaWin objects can not be copied.
	ScintillaWin(const ScintillaWin &) = delete;
	ScintillaWin(ScintillaWin &&) = delete;
	ScintillaWin &operator=(const ScintillaWin &) = delete;
	ScintillaWin &operator=(ScintillaWin &&) = delete;
	// Public for benefit of Scintilla_DirectFunction
	sptr_t WndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) override;

	/// Implement IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv) noexcept;
	STDMETHODIMP_(ULONG)AddRef() noexcept;
	STDMETHODIMP_(ULONG)Release() noexcept;

	/// Implement IDropTarget
	STDMETHODIMP DragEnter(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
		POINTL pt, PDWORD pdwEffect);
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, PDWORD pdwEffect);
	STDMETHODIMP DragLeave();
	STDMETHODIMP Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
		POINTL pt, PDWORD pdwEffect);

	/// Implement important part of IDataObject
	STDMETHODIMP GetData(const FORMATETC *pFEIn, STGMEDIUM *pSTM);

	static BOOL CALLBACK PrepareOnce(PINIT_ONCE initOnce, PVOID parameter, PVOID *lpContext) noexcept;
	static bool Register(HINSTANCE hInstance_) noexcept;
	static bool Unregister() noexcept;

	friend class DropSource;
	friend class DataObject;
	friend class DropTarget;
	bool DragIsRectangularOK(UINT fmt) const noexcept {
		return drag.rectangular && (fmt == cfColumnSelect);
	}

private:
	// For use in creating a system caret
	bool HasCaretSizeChanged() const noexcept;
	BOOL CreateSystemCaret();
	BOOL DestroySystemCaret() noexcept;
	HBITMAP sysCaretBitmap;
	int sysCaretWidth;
	int sysCaretHeight;
	bool styleIdleInQueue;
};

HINSTANCE ScintillaWin::hInstance {};
ATOM ScintillaWin::scintillaClassAtom = 0;
ATOM ScintillaWin::callClassAtom = 0;

ScintillaWin::ScintillaWin(HWND hwnd) {

	lastKeyDownConsumed = false;
	lastHighSurrogateChar = 0;

	capturedMouse = false;
	trackedMouseLeave = false;
#if _WIN32_WINNT < _WIN32_WINNT_WIN8
	SetCoalescableTimerFn = nullptr;
#endif

	linesPerScroll = 0;
	wheelDelta = 0;   // Wheel delta from roll

	dpi = GetWindowDPI(hwnd);

	hRgnUpdate = {};

	hasOKText = false;

	// There does not seem to be a real standard for indicating that the clipboard
	// contains a rectangular selection, so copy Developer Studio and Borland Delphi.
	cfColumnSelect = GetClipboardFormat(L"MSDEVColumnSelect");
	cfBorlandIDEBlockType = ::RegisterClipboardFormat(L"Borland IDE Block Type");

	// Likewise for line-copy (copies a full line when no text is selected)
	cfLineSelect = ::RegisterClipboardFormat(L"MSDEVLineSelect");
	cfVSLineTag = ::RegisterClipboardFormat(L"VisualStudioEditorOperationsLineCutCopyClipboardTag");

#if EnableDrop_VisualStudioProjectItem
	cfVSStgProjectItem = GetClipboardFormat(L"CF_VSSTGPROJECTITEMS");
	cfVSRefProjectItem = GetClipboardFormat(L"CF_VSREFPROJECTITEMS");
#endif
#if Enable_ChromiumWebCustomMIMEDataFormat
	cfChromiumCustomMIME = GetClipboardFormat(L"Chromium Web Custom MIME Data Format");
#endif

	UINT index = 0;
	dropFormat[index++] = CF_HDROP;
#if EnableDrop_VisualStudioProjectItem
	dropFormat[index++] = cfVSStgProjectItem;
	dropFormat[index++] = cfVSRefProjectItem;
#endif
#if Enable_ChromiumWebCustomMIMEDataFormat
	dropFormat[index++] = cfChromiumCustomMIME;
#endif
	// text format comes last
	dropFormat[index++] = CF_UNICODETEXT;
	dropFormat[index++] = CF_TEXT;
	dropFormatCount = index;

	//hrOle = E_FAIL;
	wMain = hwnd;

	dob.sci = this;
	ds.sci = this;
	dt.sci = this;

	sysCaretBitmap = {};
	sysCaretWidth = 0;
	sysCaretHeight = 0;
	inputLang = LANG_USER_DEFAULT;

	styleIdleInQueue = false;

#if defined(USE_D2D)
	pRenderTarget = nullptr;
	renderTargetValid = true;
#endif

	caret.period = ::GetCaretBlinkTime();
	if (caret.period < 0)
		caret.period = 0;

	Init();
}

ScintillaWin::~ScintillaWin() { Finalise(); }

void ScintillaWin::Init() noexcept {
	// Initialize COM.  If the app has already done this it will have
	// no effect.  If the app hasn't, we really shouldn't ask them to call
	// it just so this internal feature works.
	//hrOle = ::OleInitialize(nullptr);

	// Find SetCoalescableTimer which is only available from Windows 8+
#if _WIN32_WINNT < _WIN32_WINNT_WIN8
	HMODULE user32 = ::GetModuleHandleW(L"user32.dll");
	SetCoalescableTimerFn = DLLFunction<SetCoalescableTimerSig>(user32, "SetCoalescableTimer");
#endif

	vs.indicators[SC_INDICATOR_UNKNOWN] = Indicator(INDIC_HIDDEN, ColourDesired(0, 0, 0xff));
	vs.indicators[SC_INDICATOR_INPUT] = Indicator(INDIC_DOTS, ColourDesired(0, 0, 0xff));
	vs.indicators[SC_INDICATOR_CONVERTED] = Indicator(INDIC_COMPOSITIONTHICK, ColourDesired(0, 0, 0xff));
	vs.indicators[SC_INDICATOR_TARGET] = Indicator(INDIC_STRAIGHTBOX, ColourDesired(0, 0, 0xff));
}

void ScintillaWin::Finalise() noexcept {
	ScintillaBase::Finalise();
	for (TickReason tr = tickCaret; tr <= tickDwell; tr = static_cast<TickReason>(tr + 1)) {
		FineTickerCancel(tr);
	}
	SetIdle(false);
#if defined(USE_D2D)
	DropRenderTarget();
#endif
	::RevokeDragDrop(MainHWND());
	//if (SUCCEEDED(hrOle)) {
	//	::OleUninitialize();
	//}
}

#if defined(USE_D2D)

void ScintillaWin::EnsureRenderTarget(HDC hdc) noexcept {
	if (!renderTargetValid) {
		DropRenderTarget();
		renderTargetValid = true;
	}
	if (pD2DFactory && !pRenderTarget) {
		HWND hw = MainHWND();
		RECT rc;
		GetClientRect(hw, &rc);

		const D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// Create a Direct2D render target.
#if 1
		D2D1_RENDER_TARGET_PROPERTIES drtp;
		drtp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
		drtp.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
		drtp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_UNKNOWN;
		drtp.dpiX = 96.0;
		drtp.dpiY = 96.0;
		drtp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
		drtp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

		if (technology == SC_TECHNOLOGY_DIRECTWRITEDC) {
			// Explicit pixel format needed.
			drtp.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_IGNORE);

			ID2D1DCRenderTarget *pDCRT = nullptr;
			const HRESULT hr = pD2DFactory->CreateDCRenderTarget(&drtp, &pDCRT);
			if (SUCCEEDED(hr)) {
				pRenderTarget = pDCRT;
			} else {
				//Platform::DebugPrintf("Failed CreateDCRenderTarget 0x%lx\n", hr);
				ReleaseUnknown(pDCRT);
				pRenderTarget = nullptr;
			}

		} else {
			D2D1_HWND_RENDER_TARGET_PROPERTIES dhrtp;
			dhrtp.hwnd = hw;
			dhrtp.pixelSize = size;
			dhrtp.presentOptions = (technology == SC_TECHNOLOGY_DIRECTWRITERETAIN) ?
				D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS : D2D1_PRESENT_OPTIONS_NONE;

			ID2D1HwndRenderTarget *pHwndRenderTarget = nullptr;
			const HRESULT hr = pD2DFactory->CreateHwndRenderTarget(drtp, dhrtp, &pHwndRenderTarget);
			if (SUCCEEDED(hr)) {
				pRenderTarget = pHwndRenderTarget;
			} else {
				//Platform::DebugPrintf("Failed CreateHwndRenderTarget 0x%lx\n", hr);
				ReleaseUnknown(pHwndRenderTarget);
				pRenderTarget = nullptr;
			}
		}
#else
		pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
				96.0f, 96.0f, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT),
			D2D1::HwndRenderTargetProperties(hw, size),
			&pRenderTarget);
#endif
		// Pixmaps were created to be compatible with previous render target so
		// need to be recreated.
		DropGraphics(false);
	}

	if ((technology == SC_TECHNOLOGY_DIRECTWRITEDC) && pRenderTarget) {
		RECT rcWindow;
		GetClientRect(MainHWND(), &rcWindow);
		const HRESULT hr = static_cast<ID2D1DCRenderTarget*>(pRenderTarget)->BindDC(hdc, &rcWindow);
		if (FAILED(hr)) {
			//Platform::DebugPrintf("BindDC failed 0x%lx\n", hr);
			DropRenderTarget();
		}
	}
}

void ScintillaWin::DropRenderTarget() noexcept {
	ReleaseUnknown(pRenderTarget);
}

#endif

HWND ScintillaWin::MainHWND() const noexcept {
	return HwndFromWindow(wMain);
}

void ScintillaWin::DisplayCursor(Window::Cursor c) noexcept {
	if (cursorMode != SC_CURSORNORMAL) {
		c = static_cast<Window::Cursor>(cursorMode);
	}
	if (c == Window::Cursor::cursorReverseArrow) {
		::SetCursor(reverseArrowCursor.Load(dpi));
	} else {
		wMain.SetCursor(c);
	}
}

bool ScintillaWin::DragThreshold(Point ptStart, Point ptNow) noexcept {
	const Point ptDifference = ptStart - ptNow;
	const XYPOSITION xMove = std::trunc(std::abs(ptDifference.x));
	const XYPOSITION yMove = std::trunc(std::abs(ptDifference.y));
	return (xMove > SystemMetricsForDpi(SM_CXDRAG, dpi.x)) ||
		   (yMove > SystemMetricsForDpi(SM_CYDRAG, dpi.y));
}

void ScintillaWin::StartDrag() {
	inDragDrop = ddDragging;
	DWORD dwEffect = 0;
	dropWentOutside = true;
	IDataObject *pDataObject = reinterpret_cast<IDataObject *>(&dob);
	IDropSource *pDropSource = reinterpret_cast<IDropSource *>(&ds);
	//Platform::DebugPrintf("About to DoDragDrop %p %p\n", pDataObject, pDropSource);
	const HRESULT hr = ::DoDragDrop(
		pDataObject,
		pDropSource,
		DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
	//Platform::DebugPrintf("DoDragDrop = %lx\n", hr);
	if (SUCCEEDED(hr)) {
		if ((hr == DRAGDROP_S_DROP) && (dwEffect == DROPEFFECT_MOVE) && dropWentOutside) {
			// Remove dragged out text
			ClearSelection();
		}
	}
	inDragDrop = ddNone;
	SetDragPosition(SelectionPosition(Sci::invalidPosition));
}

int ScintillaWin::MouseModifiers(uptr_t wParam) noexcept {
	return ModifierFlags((wParam & MK_SHIFT) != 0,
		(wParam & MK_CONTROL) != 0,
		KeyboardIsKeyDown(VK_MENU));
}

namespace {

int InputCodePage() noexcept {
	HKL const inputLocale = ::GetKeyboardLayout(0);
	const LANGID inputLang = LOWORD(inputLocale);
	WCHAR sCodePage[32];
	const int res = ::GetLocaleInfo(MAKELCID(inputLang, SORT_DEFAULT),
		LOCALE_IDEFAULTANSICODEPAGE, sCodePage, sizeof(sCodePage) / sizeof(WCHAR));
	if (!res)
		return 0;
	return static_cast<int>(wcstol(sCodePage, nullptr, 32));
}

/** Map the key codes to their equivalent SCK_ form. */
int KeyTranslate(int keyIn) noexcept {
	//PLATFORM_ASSERT(!keyIn);
	switch (keyIn) {
	case VK_DOWN:		return SCK_DOWN;
	case VK_UP:			return SCK_UP;
	case VK_LEFT:		return SCK_LEFT;
	case VK_RIGHT:		return SCK_RIGHT;
	case VK_HOME:		return SCK_HOME;
	case VK_END:		return SCK_END;
	case VK_PRIOR:		return SCK_PRIOR;
	case VK_NEXT:		return SCK_NEXT;
	case VK_DELETE:		return SCK_DELETE;
	case VK_INSERT:		return SCK_INSERT;
	case VK_ESCAPE:		return SCK_ESCAPE;
	case VK_BACK:		return SCK_BACK;
	case VK_TAB:		return SCK_TAB;
	case VK_RETURN:		return SCK_RETURN;
	case VK_ADD:		return SCK_ADD;
	case VK_SUBTRACT:	return SCK_SUBTRACT;
	case VK_DIVIDE:		return SCK_DIVIDE;
	case VK_LWIN:		return SCK_WIN;
	case VK_RWIN:		return SCK_RWIN;
	case VK_APPS:		return SCK_MENU;
	case VK_OEM_2:		return '/';
	case VK_OEM_3:		return '`';
	case VK_OEM_4:		return '[';
	case VK_OEM_5:		return '\\';
	case VK_OEM_6:		return ']';
	default:			return keyIn;
	}
}

bool BoundsContains(PRectangle rcBounds, HRGN hRgnBounds, PRectangle rcCheck) noexcept {
	bool contains = true;
	if (!rcCheck.Empty()) {
		if (!rcBounds.Contains(rcCheck)) {
			contains = false;
		} else if (hRgnBounds) {
			// In bounding rectangle so check more accurately using region
			const RECT rcw = RectFromPRectangle(rcCheck);
			HRGN hRgnCheck = ::CreateRectRgnIndirect(&rcw);
			if (hRgnCheck) {
				HRGN hRgnDifference = ::CreateRectRgn(0, 0, 0, 0);
				if (hRgnDifference) {
					const int combination = ::CombineRgn(hRgnDifference, hRgnCheck, hRgnBounds, RGN_DIFF);
					if (combination != NULLREGION) {
						contains = false;
					}
					::DeleteRgn(hRgnDifference);
				}
				::DeleteRgn(hRgnCheck);
			}
		}
	}
	return contains;
}

// Simplify calling WideCharToMultiByte and MultiByteToWideChar by providing default parameters and using string view.

inline int MultiByteFromWideChar(UINT codePage, std::wstring_view wsv, LPSTR lpMultiByteStr, ptrdiff_t cbMultiByte) noexcept {
	return ::WideCharToMultiByte(codePage, 0, wsv.data(), static_cast<int>(wsv.length()), lpMultiByteStr, static_cast<int>(cbMultiByte), nullptr, nullptr);
}

inline int MultiByteLenFromWideChar(UINT codePage, std::wstring_view wsv) noexcept {
	return MultiByteFromWideChar(codePage, wsv, nullptr, 0);
}

inline int WideCharFromMultiByte(UINT codePage, std::string_view sv, LPWSTR lpWideCharStr, ptrdiff_t cchWideChar) noexcept {
	return ::MultiByteToWideChar(codePage, 0, sv.data(), static_cast<int>(sv.length()), lpWideCharStr, static_cast<int>(cchWideChar));
}

inline int WideCharLenFromMultiByte(UINT codePage, std::string_view sv) noexcept {
	return WideCharFromMultiByte(codePage, sv, nullptr, 0);
}

std::string StringEncode(const std::wstring_view wsv, int codePage) {
	const int cchMulti = wsv.length() ? MultiByteLenFromWideChar(codePage, wsv) : 0;
	std::string sMulti(cchMulti, 0);
	if (cchMulti) {
		MultiByteFromWideChar(codePage, wsv, sMulti.data(), cchMulti);
	}
	return sMulti;
}

std::wstring StringDecode(const std::string_view sv, int codePage) {
	const int cchWide = sv.length() ? WideCharLenFromMultiByte(codePage, sv) : 0;
	std::wstring sWide(cchWide, 0);
	if (cchWide) {
		WideCharFromMultiByte(codePage, sv, sWide.data(), cchWide);
	}
	return sWide;
}

std::wstring StringMapCase(const std::wstring_view wsv, DWORD mapFlags) {
	const int charsConverted = ::LCMapStringW(LOCALE_USER_DEFAULT, mapFlags,
		wsv.data(), static_cast<int>(wsv.length()), nullptr, 0);
	std::wstring wsConverted(charsConverted, 0);
	if (charsConverted) {
		::LCMapStringW(LOCALE_USER_DEFAULT, mapFlags,
			wsv.data(), static_cast<int>(wsv.length()), wsConverted.data(), charsConverted);
	}
	return wsConverted;
}

}

// Returns the target converted to UTF8.
// Return the length in bytes.
Sci::Position ScintillaWin::TargetAsUTF8(char *text) const {
	const Sci::Position targetLength = targetRange.Length();
	if (IsUnicodeMode()) {
		if (text) {
			pdoc->GetCharRange(text, targetRange.start.Position(), targetLength);
		}
	} else {
		// Need to convert
		const std::string s = RangeText(targetRange.start.Position(), targetRange.end.Position());
		const std::wstring characters = StringDecode(s, CodePageOfDocument());
		const int utf8Len = MultiByteLenFromWideChar(CP_UTF8, characters);
		if (text) {
			MultiByteFromWideChar(CP_UTF8, characters, text, utf8Len);
			text[utf8Len] = '\0';
		}
		return utf8Len;
	}
	return targetLength;
}

// Translates a nul terminated UTF8 string into the document encoding.
// Return the length of the result in bytes.
Sci::Position ScintillaWin::EncodedFromUTF8(const char *utf8, char *encoded) const {
	const Sci::Position inputLength = (lengthForEncode >= 0) ? lengthForEncode : strlen(utf8);
	if (IsUnicodeMode()) {
		if (encoded) {
			memcpy(encoded, utf8, inputLength);
		}
		return inputLength;
	} else {
		// Need to convert
		const std::string_view utf8Input(utf8, inputLength);
		const int charsLen = WideCharLenFromMultiByte(CP_UTF8, utf8Input);
		std::wstring characters(charsLen, L'\0');
		WideCharFromMultiByte(CP_UTF8, utf8Input, characters.data(), charsLen);

		const UINT codePage = CodePageOfDocument();
		const int encodedLen = MultiByteLenFromWideChar(codePage, characters);
		if (encoded) {
			MultiByteFromWideChar(codePage, characters, encoded, encodedLen);
			encoded[encodedLen] = '\0';
		}
		return encodedLen;
	}
}

bool ScintillaWin::PaintDC(HDC hdc) {
	if (technology == SC_TECHNOLOGY_DEFAULT) {
		AutoSurface surfaceWindow(hdc, this);
		if (surfaceWindow) {
			Paint(surfaceWindow, rcPaint);
			surfaceWindow->Release();
		}
	} else {
#if defined(USE_D2D)
		EnsureRenderTarget(hdc);
		if (pRenderTarget) {
			AutoSurface surfaceWindow(pRenderTarget, this);
			if (surfaceWindow) {
				pRenderTarget->BeginDraw();
				Paint(surfaceWindow, rcPaint);
				surfaceWindow->Release();
				const HRESULT hr = pRenderTarget->EndDraw();
				if (hr == static_cast<HRESULT>(D2DERR_RECREATE_TARGET)) {
					DropRenderTarget();
					return false;
				}
			}
		}
#endif
	}

	return true;
}

sptr_t ScintillaWin::WndPaint() {
	//ElapsedPeriod ep;
	
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	if (paintState != notPainting) { return 0; } // prevent recursion loop
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	// Redirect assertions to debug output and save current state
	//const bool assertsPopup = Platform::ShowAssertionPopUps(false);
	paintState = painting;
	PAINTSTRUCT ps = {};

	// Removed since this interferes with reporting other assertions as it occurs repeatedly
	//PLATFORM_ASSERT(hRgnUpdate == nullptr);
	hRgnUpdate = ::CreateRectRgn(0, 0, 0, 0);
	::GetUpdateRgn(MainHWND(), hRgnUpdate, FALSE);
	::BeginPaint(MainHWND(), &ps);
	rcPaint = PRectangle::FromInts(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
	const PRectangle rcClient = GetClientRectangle();
	paintingAllText = BoundsContains(rcPaint, hRgnUpdate, rcClient);
	if (!PaintDC(ps.hdc)) {
		paintState = paintAbandoned;
	}
	if (hRgnUpdate) {
		::DeleteRgn(hRgnUpdate);
		hRgnUpdate = {};
	}

	::EndPaint(MainHWND(), &ps);
	if (paintState == paintAbandoned) {
		// Painting area was insufficient to cover new styling or brace highlight positions
		FullPaint();
		::ValidateRect(MainHWND(), nullptr);
	}
	paintState = notPainting;

	// Restore debug output state
	//Platform::ShowAssertionPopUps(assertsPopup);

	//Platform::DebugPrintf("Paint took %g\n", ep.Duration());
	return 0;
}

sptr_t ScintillaWin::HandleCompositionWindowed(uptr_t wParam, sptr_t lParam) {
	if (lParam & GCS_RESULTSTR) {
		IMContext imc(MainHWND());
		if (imc.hIMC) {
			AddWString(imc.GetCompositionString(GCS_RESULTSTR), CharacterSource::imeResult);

			// Set new position after converted
			const Point pos = PointMainCaret();
			COMPOSITIONFORM CompForm;
			CompForm.dwStyle = CFS_POINT;
			CompForm.ptCurrentPos = POINTFromPoint(pos);
			::ImmSetCompositionWindow(imc.hIMC, &CompForm);
		}
		return 0;
	}
	return ::DefWindowProc(MainHWND(), WM_IME_COMPOSITION, wParam, lParam);
}

bool ScintillaWin::KoreanIME() noexcept {
	const int codePage = InputCodePage();
	return codePage == 949 || codePage == 1361;
}

// >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
bool ScintillaWin::IsIMEOpen() {
	IMContext imc(MainHWND());
	if (imc.hIMC) {
		if (ImmGetOpenStatus(imc.hIMC)) {
			return true;
		}
	}
	return false;
}

DWORD ScintillaWin::GetIMEInputMode() {
	IMContext imc(MainHWND());
	if (imc.hIMC && ImmGetOpenStatus(imc.hIMC)) {
		DWORD dwConversion = IME_CMODE_ALPHANUMERIC, dwSentence = IME_SMODE_NONE;
		if (ImmGetConversionStatus(imc.hIMC, &dwConversion, &dwSentence)) {
			return dwConversion;
		}
	}
	return 0;
}
// <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<

void ScintillaWin::MoveImeCarets(Sci::Position offset) noexcept {
	// Move carets relatively by bytes.
	for (size_t r = 0; r < sel.Count(); r++) {
		const Sci::Position positionInsert = sel.Range(r).Start().Position();
		sel.Range(r).caret.SetPosition(positionInsert + offset);
		sel.Range(r).anchor.SetPosition(positionInsert + offset);
	}
}

void ScintillaWin::DrawImeIndicator(int indicator, Sci::Position len) {
	// Emulate the visual style of IME characters with indicators.
	// Draw an indicator on the character before caret by the character bytes of len
	// so it should be called after InsertCharacter().
	// It does not affect caret positions.
	if (indicator < 8 || indicator > INDICATOR_MAX) {
		return;
	}
	pdoc->DecorationSetCurrentIndicator(indicator);
	for (size_t r = 0; r < sel.Count(); r++) {
		const Sci::Position positionInsert = sel.Range(r).Start().Position();
		pdoc->DecorationFillRange(positionInsert - len, 1, len);
	}
}

void ScintillaWin::SetCandidateWindowPos() {
	IMContext imc(MainHWND());
	if (imc.hIMC) {
		const Point pos = PointMainCaret();
		const PRectangle rcClient = GetTextRectangle();
		CANDIDATEFORM CandForm{};
		CandForm.dwIndex = 0;
		CandForm.dwStyle = CFS_EXCLUDE;
		CandForm.ptCurrentPos.x = static_cast<int>(pos.x);
		CandForm.ptCurrentPos.y = static_cast<int>(pos.y + std::max(4, vs.lineHeight/4));
		// Exclude the area of the whole caret line
		CandForm.rcArea.top = static_cast<int>(pos.y);
		CandForm.rcArea.bottom = static_cast<int>(pos.y + vs.lineHeight);
		CandForm.rcArea.left = static_cast<int>(rcClient.left);
		CandForm.rcArea.right = static_cast<int>(rcClient.right);
		::ImmSetCandidateWindow(imc.hIMC, &CandForm);
	}
}

void ScintillaWin::SelectionToHangul() {
	// Convert every hanja to hangul within the main range.
	const Sci::Position selStart = sel.RangeMain().Start().Position();
	const Sci::Position documentStrLen = sel.RangeMain().Length();
	const Sci::Position selEnd = selStart + documentStrLen;
	const Sci::Position utf16Len = pdoc->CountUTF16(selStart, selEnd);

	if (utf16Len > 0) {
		std::string documentStr(documentStrLen, '\0');
		pdoc->GetCharRange(documentStr.data(), selStart, documentStrLen);
		const UINT codePage = CodePageOfDocument();

		std::wstring uniStr = StringDecode(documentStr, codePage);
		const int converted = HanjaDict::GetHangulOfHanja(uniStr.data());
		documentStr = StringEncode(uniStr, codePage);

		if (converted > 0) {
			pdoc->BeginUndoAction();
			ClearSelection();
			InsertPaste(documentStr.data(), documentStr.size());
			pdoc->EndUndoAction();
		}
	}
}

void ScintillaWin::EscapeHanja() {
	// The candidate box pops up to user to select a hanja.
	// It comes into WM_IME_COMPOSITION with GCS_RESULTSTR.
	// The existing hangul or hanja is replaced with it.
	if (sel.Count() > 1) {
		return; // Do not allow multi carets.
	}
	const Sci::Position currentPos = CurrentPosition();
	const int oneCharLen = pdoc->LenChar(currentPos);

	if (oneCharLen < 2) {
		return; // No need to handle SBCS.
	}

	// ImmEscapeW() may overwrite uniChar[] with a null terminated string.
	// So enlarge it enough to Maximum 4 as in UTF-8.
	constexpr size_t safeLength = UTF8MaxBytes + 1;
	std::string oneChar(safeLength, '\0');
	pdoc->GetCharRange(oneChar.data(), currentPos, oneCharLen);

	std::wstring uniChar = StringDecode(oneChar, CodePageOfDocument());

	IMContext imc(MainHWND());
	if (imc.hIMC) {
		// Set the candidate box position since IME may show it.
		SetCandidateWindowPos();
		// IME_ESC_HANJA_MODE appears to receive the first character only.
		if (::ImmEscapeW(GetKeyboardLayout(0), imc.hIMC, IME_ESC_HANJA_MODE, uniChar.data())) {
			SetSelection(currentPos, currentPos + oneCharLen);
		}
	}
}

void ScintillaWin::ToggleHanja() {
	// If selection, convert every hanja to hangul within the main range.
	// If no selection, commit to IME.
	if (sel.Count() > 1) {
		return; // Do not allow multi carets.
	}

	if (sel.Empty()) {
		EscapeHanja();
	} else {
		SelectionToHangul();
	}
}

namespace {

// https://docs.microsoft.com/en-us/windows/desktop/Intl/composition-string
std::vector<int> MapImeIndicators(const std::vector<BYTE> &inputStyle) {
	std::vector<int> imeIndicator(inputStyle.size(), SC_INDICATOR_UNKNOWN);
	for (size_t i = 0; i < inputStyle.size(); i++) {
		switch (static_cast<int>(inputStyle.at(i))) {
		case ATTR_INPUT:
			imeIndicator[i] = SC_INDICATOR_INPUT;
			break;
		case ATTR_TARGET_NOTCONVERTED:
		case ATTR_TARGET_CONVERTED:
			imeIndicator[i] = SC_INDICATOR_TARGET;
			break;
		case ATTR_CONVERTED:
			imeIndicator[i] = SC_INDICATOR_CONVERTED;
			break;
		default:
			imeIndicator[i] = SC_INDICATOR_UNKNOWN;
			break;
		}
	}
	return imeIndicator;
}

}

void ScintillaWin::AddWString(std::wstring_view wsv, CharacterSource charSource) {
	if (wsv.empty())
		return;

	const UINT codePage = CodePageOfDocument();
	char inBufferCP[16];
	for (size_t i = 0; i < wsv.size(); ) {
		const size_t ucWidth = UTF16CharLength(wsv[i]);

		const int size = MultiByteFromWideChar(codePage, wsv.substr(i, ucWidth), inBufferCP, sizeof(inBufferCP) - 1);
		inBufferCP[size] = '\0';
		InsertCharacter(std::string_view(inBufferCP, size), charSource);
		i += ucWidth;
	}
}

sptr_t ScintillaWin::HandleCompositionInline(uptr_t, sptr_t lParam) {
	// Copy & paste by johnsonj with a lot of helps of Neil.
	// Great thanks for my foreruners, jiniya and BLUEnLIVE.

	IMContext imc(MainHWND());
	if (!imc.hIMC)
		return 0;
	if (pdoc->IsReadOnly() || SelectionContainsProtected()) {
		::ImmNotifyIME(imc.hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
		return 0;
	}

	bool initialCompose = false;
	if (pdoc->TentativeActive()) {
		pdoc->TentativeUndo();
	} else {
		// No tentative undo means start of this composition so
		// fill in any virtual spaces.
		initialCompose = true;
	}

	view.imeCaretBlockOverride = false;

	if (lParam & GCS_RESULTSTR) {
		AddWString(imc.GetCompositionString(GCS_RESULTSTR), CharacterSource::imeResult);
	}

	if (lParam & GCS_COMPSTR) {
		const std::wstring wcs = imc.GetCompositionString(GCS_COMPSTR);
		if (wcs.empty()) {
			ShowCaretAtCurrentPosition();
			return 0;
		}

		if (initialCompose) {
			ClearBeforeTentativeStart();
		}

		// Set candidate window left aligned to beginning of preedit string.
		SetCandidateWindowPos();
		pdoc->TentativeStart(); // TentativeActive from now on.

		std::vector<int> imeIndicator = MapImeIndicators(imc.GetImeAttributes());

		const UINT codePage = CodePageOfDocument();
		char inBufferCP[16];
		const std::wstring_view wsv = wcs;

		for (size_t i = 0; i < wsv.size(); ) {
			const size_t ucWidth = UTF16CharLength(wsv[i]);
			const int size = MultiByteFromWideChar(codePage, wsv.substr(i, ucWidth), inBufferCP, sizeof(inBufferCP) - 1);
			inBufferCP[size] = '\0';
			InsertCharacter(std::string_view(inBufferCP, size), CharacterSource::tentativeInput);

			DrawImeIndicator(imeIndicator[i], size);
			i += ucWidth;
		}

		// Move IME caret from current last position to imeCaretPos.
		const int imeEndToImeCaretU16 = imc.GetImeCaretPos() - static_cast<unsigned int>(wcs.size());
		const Sci::Position imeCaretPosDoc = pdoc->GetRelativePositionUTF16(CurrentPosition(), imeEndToImeCaretU16);

		MoveImeCarets(-CurrentPosition() + imeCaretPosDoc);

		if (std::find(imeIndicator.begin(), imeIndicator.end(), SC_INDICATOR_TARGET) != imeIndicator.end()) {
			// set candidate window left aligned to beginning of target string.
			SetCandidateWindowPos();
		}

		if (KoreanIME()) {
			view.imeCaretBlockOverride = true;
		}
	}
	EnsureCaretVisible();
	ShowCaretAtCurrentPosition();
	return 0;
}

namespace {

// Translate message IDs from WM_* and EM_* to SCI_* so can partly emulate Windows Edit control
unsigned int SciMessageFromEM(unsigned int iMessage) noexcept {
	switch (iMessage) {
	case EM_CANPASTE: return SCI_CANPASTE;
	case EM_CANREDO: return SCI_CANREDO;
	case EM_CANUNDO: return SCI_CANUNDO;
	case EM_EMPTYUNDOBUFFER: return SCI_EMPTYUNDOBUFFER;
	case EM_FINDTEXTEX: return SCI_FINDTEXT;
	case EM_FORMATRANGE: return SCI_FORMATRANGE;
	case EM_GETFIRSTVISIBLELINE: return SCI_GETFIRSTVISIBLELINE;
	case EM_GETLINE: return SCI_GETLINE;
	case EM_GETLINECOUNT: return SCI_GETLINECOUNT;
	case EM_GETSELTEXT: return SCI_GETSELTEXT;
	case EM_GETTEXTRANGE: return SCI_GETTEXTRANGE;
	case EM_HIDESELECTION: return SCI_HIDESELECTION;
	case EM_LINEINDEX: return SCI_POSITIONFROMLINE;
	case EM_LINESCROLL: return SCI_LINESCROLL;
	case EM_REDO: return SCI_REDO;
	case EM_UNDO: return SCI_UNDO;
	case EM_SCROLL: return WM_VSCROLL;
	case EM_REPLACESEL: return SCI_REPLACESEL;
	case EM_SCROLLCARET: return SCI_SCROLLCARET;
	case EM_SETREADONLY: return SCI_SETREADONLY;
	case WM_CLEAR: return SCI_CLEAR;
	case WM_COPY: return SCI_COPY;
	case WM_CUT: return SCI_CUT;
	case WM_SETTEXT: return SCI_SETTEXT;
	case WM_PASTE: return SCI_PASTE;
	case WM_UNDO: return SCI_UNDO;
	}
	return iMessage;
}

}

namespace Scintilla {

UINT CodePageFromCharSet(DWORD characterSet, UINT documentCodePage) noexcept {
	// UTF-8 and DBCS ANSI code pages
	if (documentCodePage) {
		return SC_CP_UTF8; // we only use UTF-8
	}
	// SBCS code pages: zero / CP_ACP
	switch (characterSet) {
	case SC_CHARSET_ANSI: return 1252;
	//case SC_CHARSET_DEFAULT: return documentCodePage ? documentCodePage : 1252;  // SCI orig
	case SC_CHARSET_DEFAULT: return documentCodePage;
	case SC_CHARSET_BALTIC: return 1257;
	case SC_CHARSET_CHINESEBIG5: return 950;
	case SC_CHARSET_EASTEUROPE: return 1250;
	case SC_CHARSET_GB2312: return 936;
	case SC_CHARSET_GREEK: return 1253;
	case SC_CHARSET_HANGUL: return 949;
	case SC_CHARSET_MAC: return 10000;
	case SC_CHARSET_OEM: return 437;
	case SC_CHARSET_RUSSIAN: return 1251;
	case SC_CHARSET_SHIFTJIS: return 932;
	case SC_CHARSET_TURKISH: return 1254;
	case SC_CHARSET_JOHAB: return 1361;
	case SC_CHARSET_HEBREW: return 1255;
	case SC_CHARSET_ARABIC: return 1256;
	case SC_CHARSET_VIETNAMESE: return 1258;
	case SC_CHARSET_THAI: return 874;
	case SC_CHARSET_8859_15: return 28605;
		// Not supported
	case SC_CHARSET_CYRILLIC: return documentCodePage;
	case SC_CHARSET_SYMBOL: return documentCodePage;
	}
	return documentCodePage;
}

}

UINT ScintillaWin::CodePageOfDocument() const noexcept {
	return CodePageFromCharSet(vs.styles[STYLE_DEFAULT].characterSet, pdoc->dbcsCodePage);
}

std::string ScintillaWin::EncodeWString(std::wstring_view wsv) {
	if (IsUnicodeMode()) {
		const size_t len = UTF8Length(wsv);
		std::string putf(len, 0);
		UTF8FromUTF16(wsv, putf.data(), len);
		return putf;
	} else {
		// Not in Unicode mode so convert from Unicode to current Scintilla code page
		return StringEncode(wsv, CodePageOfDocument());
	}
}

sptr_t ScintillaWin::GetTextLength() const noexcept {
	return pdoc->CountUTF16(0, pdoc->Length());
}

sptr_t ScintillaWin::GetText(uptr_t wParam, sptr_t lParam) const {
	if (lParam == 0) {
		return pdoc->CountUTF16(0, pdoc->Length());
	}
	if (wParam == 0) {
		return 0;
	}
	wchar_t *ptr = static_cast<wchar_t *>(PtrFromSPtr(lParam));
	if (pdoc->Length() == 0) {
		*ptr = L'\0';
		return 0;
	}
	const Sci::Position lengthWanted = wParam - 1;
	Sci::Position sizeRequestedRange = pdoc->GetRelativePositionUTF16(0, lengthWanted);
	if (sizeRequestedRange < 0) {
		// Requested more text than there is in the document.
		sizeRequestedRange = pdoc->Length();
	}
	std::string docBytes(sizeRequestedRange, '\0');
	pdoc->GetCharRange(&docBytes[0], 0, sizeRequestedRange);
	if (IsUnicodeMode()) {
		const size_t uLen = UTF16FromUTF8(docBytes, ptr, lengthWanted);
		ptr[uLen] = L'\0';
		return uLen;
	} else {
		// Not Unicode mode
		// Convert to Unicode using the current Scintilla code page
		const UINT cpSrc = CodePageOfDocument();
		int lengthUTF16 = WideCharLenFromMultiByte(cpSrc, docBytes);
		if (lengthUTF16 > lengthWanted)
			lengthUTF16 = static_cast<int>(lengthWanted);
		WideCharFromMultiByte(cpSrc, docBytes, ptr, lengthUTF16);
		ptr[lengthUTF16] = L'\0';
		return lengthUTF16;
	}
}

Window::Cursor ScintillaWin::ContextCursor(Point pt) {
	if (inDragDrop == ddDragging) {
		return Window::Cursor::cursorUp;
	} else {
		// Display regular (drag) cursor over selection
		if (PointInSelMargin(pt)) {
			return GetMarginCursor(pt);
		} else if (!SelectionEmpty() && PointInSelection(pt)) {
			return Window::Cursor::cursorArrow;
		} else if (PointIsHotspot(pt)) {
			return Window::Cursor::cursorHand;
		} else if (hoverIndicatorPos != Sci::invalidPosition) {
			const Sci::Position pos = PositionFromLocation(pt, true, true);
			if (pos != Sci::invalidPosition) {
				return Window::Cursor::cursorHand;
			}
		}
	}
	return Window::Cursor::cursorText;
}

#if SCI_EnablePopupMenu
sptr_t ScintillaWin::ShowContextMenu(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	Point pt = PointFromLParam(lParam);
	POINT rpt = POINTFromPoint(pt);
	::ScreenToClient(MainHWND(), &rpt);
	const Point ptClient = PointFromPOINT(rpt);
	if (ShouldDisplayPopup(ptClient)) {
		if ((pt.x == -1) && (pt.y == -1)) {
			// Caused by keyboard so display menu near caret
			pt = PointMainCaret();
			POINT spt = POINTFromPoint(pt);
			::ClientToScreen(MainHWND(), &spt);
			pt = PointFromPOINT(spt);
		}
		ContextMenu(pt);
		return 0;
	}
	return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
}
#endif

void ScintillaWin::SizeWindow() {
#if defined(USE_D2D)
	if (paintState == notPainting) {
		DropRenderTarget();
	} else {
		renderTargetValid = false;
	}
#endif
	//Platform::DebugPrintf("Scintilla WM_SIZE %d %d\n", LOWORD(lParam), HIWORD(lParam));
	ChangeSize();
}

sptr_t ScintillaWin::MouseMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	switch (iMessage) {
	case WM_LBUTTONDOWN: {
			// For IME, set the composition string as the result string.
			IMContext imc(MainHWND());
			if (imc.hIMC) {
				::ImmNotifyIME(imc.hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
			}
			//
			//Platform::DebugPrintf("Buttdown %d %x %x %x %x %x\n",iMessage, wParam, lParam,
			//	KeyboardIsKeyDown(VK_SHIFT),
			//	KeyboardIsKeyDown(VK_CONTROL),
			//	KeyboardIsKeyDown(VK_MENU));
			::SetFocus(MainHWND());
			ButtonDownWithModifiers(PointFromLParam(lParam), ::GetMessageTime(),
						MouseModifiers(wParam));
		}
		break;

	case WM_LBUTTONUP:
		ButtonUpWithModifiers(PointFromLParam(lParam),
				      ::GetMessageTime(), MouseModifiers(wParam));
		break;

	case WM_RBUTTONDOWN: {
		::SetFocus(MainHWND());
		const Point pt = PointFromLParam(lParam);
		if (!PointInSelection(pt)) {
			CancelModes();
			SetEmptySelection(PositionFromLocation(PointFromLParam(lParam)));
		}

			RightButtonDownWithModifiers(pt, ::GetMessageTime(), MouseModifiers(wParam));
		}
		break;

	// >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
	case WM_MBUTTONDOWN:
		// send to main window
		::SetFocus(MainHWND());
		//::DefWindowProc(MainHWND(), iMessage, wParam, lParam); // does not propagate - filter msg ?
		::SendMessage(GetParent(MainHWND()), WM_MBUTTONDOWN, wParam, lParam);
		break;
	// <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<

	case WM_MOUSEMOVE: {
		const Point pt = PointFromLParam(lParam);

		// Windows might send WM_MOUSEMOVE even though the mouse has not been moved:
		// http://blogs.msdn.com/b/oldnewthing/archive/2003/10/01/55108.aspx
		if (ptMouseLast != pt) {
			SetTrackMouseLeaveEvent(true);
			ButtonMoveWithModifiers(pt, ::GetMessageTime(), MouseModifiers(wParam));
		}
	}
	break;

	case WM_MOUSELEAVE:
		SetTrackMouseLeaveEvent(false);
		MouseLeave();
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_MOUSEWHEEL:
		if (!mouseWheelCaptures) {
			// if the mouse wheel is not captured, test if the mouse
			// pointer is over the editor window and if not, don't
			// handle the message but pass it on.
			RECT rc;
			GetWindowRect(MainHWND(), &rc);
			const POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (!PtInRect(&rc, pt)) {
				return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
			}
		}

		// if autocomplete list active then send mousewheel message to it
		if (ac.Active()) {
			HWND hWnd = HwndFromWindow(*(ac.lb));
			::SendMessage(hWnd, iMessage, wParam, lParam);
			break;
		}

		// Don't handle datazoom.
		// (A good idea for datazoom would be to "fold" or "unfold" details.
		// i.e. if datazoomed out only class structures are visible, when datazooming in the control
		// structures appear, then eventually the individual statements...)
		if (wParam & (MK_SHIFT | MK_RBUTTON)) {
			// send to client
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
		}

		// Either SCROLL or ZOOM. We handle the wheel steppings calculation
		wheelDelta -= GET_WHEEL_DELTA_WPARAM(wParam);
		if (std::abs(wheelDelta) >= WHEEL_DELTA && linesPerScroll > 0) {
			Sci::Line linesToScroll = linesPerScroll;
			if (linesPerScroll == WHEEL_PAGESCROLL) {
				linesToScroll = LinesOnScreen() - 1;
			}
			if (linesToScroll == 0) {
				linesToScroll = 1;
			}
			linesToScroll *= (wheelDelta / WHEEL_DELTA);
			if (wheelDelta >= 0) {
				wheelDelta = wheelDelta % WHEEL_DELTA;
			} else {
				wheelDelta = -(-wheelDelta % WHEEL_DELTA);
			}

			if (wParam & MK_CONTROL) {
				// Zoom! We play with the font sizes in the styles.
				// Number of steps/line is ignored, we just care if sizing up or down
				if (linesToScroll < 0) {
					KeyCommand(SCI_ZOOMIN);
				} else {
					KeyCommand(SCI_ZOOMOUT);
				}
				// >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
				// send to main window too !
				::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
				// <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<
			} else {
				// Scroll
				ScrollTo(topLine + linesToScroll);
			}
		}
		return 0;
	}
	return 0;
}

sptr_t ScintillaWin::KeyMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	switch (iMessage) {

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN: {
		// Platform::DebugPrintf("Keydown %c %c%c%c%c %x %x\n",
		// iMessage == WM_KEYDOWN ? 'K' : 'S',
		// (lParam & (1 << 24)) ? 'E' : '-',
		// KeyboardIsKeyDown(VK_SHIFT) ? 'S' : '-',
		// KeyboardIsKeyDown(VK_CONTROL) ? 'C' : '-',
		// KeyboardIsKeyDown(VK_MENU) ? 'A' : '-',
		// wParam, lParam);
		lastKeyDownConsumed = false;
		const bool altDown = KeyboardIsKeyDown(VK_MENU);
		if (altDown && KeyboardIsNumericKeypadFunction(wParam, lParam)) {
			// Don't interpret these as they may be characters entered by number.
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
		}
		const int ret = KeyDownWithModifiers(KeyTranslate(static_cast<int>(wParam)),
							ModifierFlags(KeyboardIsKeyDown(VK_SHIFT), KeyboardIsKeyDown(VK_CONTROL), altDown),
							&lastKeyDownConsumed);
		if (!ret && !lastKeyDownConsumed) {
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
		}
		break;
	}

	case WM_KEYUP:
		//Platform::DebugPrintf("S keyup %d %x %x\n",iMessage, wParam, lParam);
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_CHAR:
		if (!lastKeyDownConsumed) {
			wchar_t wcs[3] = { static_cast<wchar_t>(wParam), 0 };
			unsigned int wclen = 1;
			if (IS_HIGH_SURROGATE(wcs[0])) {
				// If this is a high surrogate character, we need a second one
				lastHighSurrogateChar = wcs[0];
				return 0;
			} else if (IS_LOW_SURROGATE(wcs[0])) {
				wcs[1] = wcs[0];
				wcs[0] = lastHighSurrogateChar;
				lastHighSurrogateChar = 0;
				wclen = 2;
			}
			AddWString(std::wstring_view(wcs, wclen), CharacterSource::directInput);
		}
		return 0;

	case WM_UNICHAR:
		if (wParam == UNICODE_NOCHAR) {
			return TRUE;
		} else if (lastKeyDownConsumed) {
			return 1;
		} else {
			wchar_t wcs[3] = { 0 };
			const size_t wclen = UTF16FromUTF32Character(static_cast<unsigned int>(wParam), wcs);
			AddWString(std::wstring_view(wcs, wclen), CharacterSource::directInput);
			return FALSE;
		}
	}

	return 0;
}

sptr_t ScintillaWin::FocusMessage(unsigned int iMessage, uptr_t wParam, sptr_t) {
	switch (iMessage) {
	case WM_KILLFOCUS: {
		HWND wOther = reinterpret_cast<HWND>(wParam);
		HWND wThis = MainHWND();
		HWND wCT = HwndFromWindow(ct.wCallTip);
		if (!wParam ||
			!(::IsChild(wThis, wOther) || (wOther == wCT))) {
			SetFocusState(false);
			DestroySystemCaret();
		}
		// Explicitly complete any IME composition
		IMContext imc(MainHWND());
		if (imc.hIMC) {
			::ImmNotifyIME(imc.hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
		}
		break;
	}

	case WM_SETFOCUS:
		SetFocusState(true);
		DestroySystemCaret();
		CreateSystemCaret();
		break;
	}
	return 0;
}

sptr_t ScintillaWin::IMEMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	switch (iMessage) {

	case WM_INPUTLANGCHANGE:
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_INPUTLANGCHANGEREQUEST:
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_IME_KEYDOWN: {
		if (wParam == VK_HANJA) {
			ToggleHanja();
		}
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
	}

	case WM_IME_REQUEST: {
		if (wParam == IMR_RECONVERTSTRING) {
			return ImeOnReconvert(lParam);
		}
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
	}

	case WM_IME_STARTCOMPOSITION:
		if (KoreanIME() || imeInteraction == imeInline) {
			return 0;
		} else {
			ImeStartComposition();
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
		}

	case WM_IME_ENDCOMPOSITION:
		ImeEndComposition();
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_IME_COMPOSITION:
		if (KoreanIME() || imeInteraction == imeInline) {
			return HandleCompositionInline(wParam, lParam);
		} else {
			return HandleCompositionWindowed(wParam, lParam);
		}

	case WM_IME_SETCONTEXT:
		if (KoreanIME() || imeInteraction == imeInline) {
			if (wParam) {
				LPARAM NoImeWin = lParam;
				NoImeWin = NoImeWin & (~ISC_SHOWUICOMPOSITIONWINDOW);
				return ::DefWindowProc(MainHWND(), iMessage, wParam, NoImeWin);
			}
		}
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	// >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
	case WM_IME_NOTIFY:
		if (wParam == IMN_SETOPENSTATUS) {
			imeIsOpen = IsIMEOpen();
		} 
		if (wParam == IMN_SETOPENSTATUS || wParam == IMN_SETCONVERSIONMODE) {
			imeIsInModeCJK = (GetIMEInputMode() != IME_CMODE_ALPHANUMERIC);
		}
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
	// <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<

}
	return 0;
}

sptr_t ScintillaWin::EditMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	switch (iMessage) {

	case EM_FINDTEXT:
		if (lParam == 0) {
			return -1;
		} else {
			const FINDTEXTA *pFT = reinterpret_cast<const FINDTEXTA *>(lParam);
			Sci_TextToFind tt = { { pFT->chrg.cpMin, pFT->chrg.cpMax }, pFT->lpstrText, {} };
			return ScintillaBase::WndProc(SCI_FINDTEXT, wParam, reinterpret_cast<sptr_t>(&tt));
		}

	case EM_FINDTEXTEX:
		if (lParam == 0) {
			return -1;
		} else {
			FINDTEXTEXA *pFT = reinterpret_cast<FINDTEXTEXA *>(lParam);
			Sci_TextToFind tt = { { pFT->chrg.cpMin, pFT->chrg.cpMax }, pFT->lpstrText, {} };
			const Sci::Position pos =ScintillaBase::WndProc(SCI_FINDTEXT, wParam, reinterpret_cast<sptr_t>(&tt));
			pFT->chrgText.cpMin = (pos == -1)? -1 : static_cast<LONG>(tt.chrgText.cpMin);
			pFT->chrgText.cpMax = (pos == -1)? -1 : static_cast<LONG>(tt.chrgText.cpMax);
			return pos;
		}

	case EM_FORMATRANGE:
		if (lParam) {
			const FORMATRANGE *pFR = reinterpret_cast<const FORMATRANGE *>(lParam);
			const Sci_RangeToFormat fr = { pFR->hdcTarget, pFR->hdc,
				{ pFR->rc.left, pFR->rc.top, pFR->rc.right, pFR->rc.bottom },
				{ pFR->rcPage.left, pFR->rcPage.top, pFR->rcPage.right, pFR->rcPage.bottom },
				{ pFR->chrg.cpMin, pFR->chrg.cpMax },
			};
			return ScintillaBase::WndProc(SCI_FORMATRANGE, wParam, reinterpret_cast<sptr_t>(&fr));
		}
		break;

	case EM_GETTEXTRANGE:
		if (lParam) {
			TEXTRANGEA *pTR = reinterpret_cast<TEXTRANGEA *>(lParam);
			Sci_TextRange tr = { { pTR->chrg.cpMin, pTR->chrg.cpMax }, pTR->lpstrText };
			return ScintillaBase::WndProc(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr));
		}
		break;

	case EM_LINEFROMCHAR:
		if (static_cast<Sci::Position>(wParam) < 0) {
			wParam = SelectionStart().Position();
		}
		return pdoc->LineFromPosition(wParam);

	case EM_EXLINEFROMCHAR:
		return pdoc->LineFromPosition(lParam);

	case EM_GETSEL:
		if (wParam) {
			*reinterpret_cast<DWORD *>(wParam) = static_cast<DWORD>(SelectionStart().Position());
		}
		if (lParam) {
			*reinterpret_cast<DWORD *>(lParam) = static_cast<DWORD>(SelectionEnd().Position());
		}
		return MAKELRESULT(SelectionStart().Position(), SelectionEnd().Position());

	case EM_EXGETSEL: {
		if (lParam == 0) {
			return 0;
		}
		CHARRANGE *pCR = reinterpret_cast<CHARRANGE *>(lParam);
		pCR->cpMin = static_cast<LONG>(SelectionStart().Position());
		pCR->cpMax = static_cast<LONG>(SelectionEnd().Position());
	}
	break;

	case EM_SETSEL: {
		Sci::Position nStart = wParam;
		Sci::Position nEnd = lParam;
		if (nStart == 0 && nEnd == -1) {
			nEnd = pdoc->Length();
		}
		if (nStart == -1) {
			nStart = nEnd;	// Remove selection
		}
		SetSelection(nEnd, nStart);
		EnsureCaretVisible();
	}
	break;

	case EM_EXSETSEL: {
		if (lParam == 0) {
			return 0;
		}
		const CHARRANGE *pCR = reinterpret_cast<const CHARRANGE *>(lParam);
		sel.selType = Selection::selStream;
		if (pCR->cpMin == 0 && pCR->cpMax == -1) {
			SetSelection(pCR->cpMin, pdoc->Length());
		} else {
			SetSelection(pCR->cpMin, pCR->cpMax);
		}
		EnsureCaretVisible();
		return pdoc->LineFromPosition(SelectionStart().Position());
	}

	case EM_LINELENGTH:
		return ScintillaBase::WndProc(SCI_LINELENGTH, pdoc->LineFromPosition(wParam), lParam);

	case EM_POSFROMCHAR:
		if (wParam) {
			const Point pt = LocationFromPosition(lParam);
			POINTL *ptw = reinterpret_cast<POINTL *>(wParam);
			ptw->x = static_cast<LONG>(pt.x - vs.textStart + vs.fixedColumnWidth); // SCI_POINTXFROMPOSITION
			ptw->y = static_cast<LONG>(pt.y);
		}
		break;

	case EM_GETZOOM:
		if (wParam && lParam) {
			*reinterpret_cast<int *>(wParam) = 16*vs.zoomLevel/25;
			*reinterpret_cast<int *>(lParam) = 64;
			return TRUE;
		}
		break;

	case EM_SETZOOM: {
		int level = 0;
		if (wParam == 0 && lParam == 0) {
			level = 100;
		} else if (wParam != 0 && lParam > 0) {
			level = static_cast<int>(wParam/lParam);
		}
		if (level != 0) {
			ScintillaBase::WndProc(SCI_SETZOOM, level, 0);
			return TRUE;
		}
	}
	break;

	}
	return 0;
}

sptr_t ScintillaWin::IdleMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	switch (iMessage) {
	case SC_WIN_IDLE:
		// wParam=dwTickCountInitial, or 0 to initialize.  lParam=bSkipUserInputTest
		if (idler.state) {
			if (lParam || (WAIT_TIMEOUT == MsgWaitForMultipleObjects(0, nullptr, 0, 0, QS_INPUT | QS_HOTKEY))) {
				if (Idle()) {
					// User input was given priority above, but all events do get a turn.  Other
					// messages, notifications, etc. will get interleaved with the idle messages.

					// However, some things like WM_PAINT are a lower priority, and will not fire
					// when there's a message posted.  So, several times a second, we stop and let
					// the low priority events have a turn (after which the timer will fire again).

					// Suppress a warning from Code Analysis that the GetTickCount function
					// wraps after 49 days. The WM_TIMER will kick off another SC_WIN_IDLE
					// after the wrap.

					const DWORD dwCurrent = GetTickCount();
					const DWORD dwStart = wParam ? static_cast<DWORD>(wParam) : dwCurrent;
					constexpr DWORD maxWorkTime = 50;

					if (dwCurrent >= dwStart && dwCurrent > maxWorkTime &&dwCurrent - maxWorkTime < dwStart) {
						PostMessage(MainHWND(), SC_WIN_IDLE, dwStart, 0);
					}
				} else {
					SetIdle(false);
				}
			}
		}
		break;

	case SC_WORK_IDLE:
		IdleWork();
		break;
	}
	return 0;
}

sptr_t ScintillaWin::SciMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	switch (iMessage) {
	case SCI_GETDIRECTFUNCTION:
		return reinterpret_cast<sptr_t>(DirectFunction);

	case SCI_GETDIRECTPOINTER:
		return reinterpret_cast<sptr_t>(this);

	case SCI_GRABFOCUS:
		::SetFocus(MainHWND());
		break;

#ifdef INCLUDE_DEPRECATED_FEATURES
	case SCI_SETKEYSUNICODE:
		break;

	case SCI_GETKEYSUNICODE:
		return true;
#endif

	case SCI_SETTECHNOLOGY:
		if ((wParam == SC_TECHNOLOGY_DEFAULT) ||
			(wParam == SC_TECHNOLOGY_DIRECTWRITERETAIN) ||
			(wParam == SC_TECHNOLOGY_DIRECTWRITEDC) ||
			(wParam == SC_TECHNOLOGY_DIRECTWRITE)) {
			const int technologyNew = static_cast<int>(wParam);
			if (technology != technologyNew) {
				if (technologyNew > SC_TECHNOLOGY_DEFAULT) {
#if defined(USE_D2D)
					if (!LoadD2D())
						// Failed to load Direct2D or DirectWrite so no effect
						return 0;
#else
					return 0;
#endif
				} else {
					bidirectional = EditModel::Bidirectional::bidiDisabled;
				}
#if defined(USE_D2D)
				DropRenderTarget();
#endif
				technology = technologyNew;
				// Invalidate all cached information including layout.
				vs.fontsValid = false;
				DropGraphics(true);
				InvalidateStyleRedraw();
			}
		}
		break;

	case SCI_SETBIDIRECTIONAL:
		if (technology == SC_TECHNOLOGY_DEFAULT) {
			bidirectional = EditModel::Bidirectional::bidiDisabled;
		} else if (wParam <= SC_BIDIRECTIONAL_R2L) {
			bidirectional = static_cast<EditModel::Bidirectional>(wParam);
		}
		// Invalidate all cached information including layout.
		DropGraphics(true);
		InvalidateStyleRedraw();
		break;

	case SCI_TARGETASUTF8:
		return TargetAsUTF8(CharPtrFromSPtr(lParam));

	case SCI_ENCODEDFROMUTF8:
		return EncodedFromUTF8(ConstCharPtrFromUPtr(wParam),
			CharPtrFromSPtr(lParam));

	}
	return 0;
}

sptr_t ScintillaWin::WndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	try {
		//Platform::DebugPrintf("S M:%x WP:%x L:%x\n", iMessage, wParam, lParam);
		iMessage = SciMessageFromEM(iMessage);
		switch (iMessage) {

		case WM_CREATE:
			ctrlID = ::GetDlgCtrlID(HwndFromWindow(wMain));
			// Get Intellimouse scroll line parameters
			GetIntelliMouseParameters();
			::RegisterDragDrop(MainHWND(), reinterpret_cast<IDropTarget *>(&dt));
			break;

		case WM_COMMAND:
			Command(LOWORD(wParam));
			break;

		case WM_PAINT:
			return WndPaint();

		case WM_PRINTCLIENT: {
			HDC hdc = reinterpret_cast<HDC>(wParam);
			if (!IsCompatibleDC(hdc)) {
				return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
			}
			FullPaintDC(hdc);
		}
		break;

		case WM_VSCROLL:
			ScrollMessage(wParam);
			break;

		case WM_HSCROLL:
			HorizontalScrollMessage(wParam);
			break;

		case WM_SIZE:
			SizeWindow();
			break;

		case WM_TIMER:
			if (wParam == idleTimerID && idler.state) {
				SendMessage(MainHWND(), SC_WIN_IDLE, 0, 1);
			} else {
				TickFor(static_cast<TickReason>(wParam - fineTimerStart));
			}
			break;

		case SC_WIN_IDLE:
		case SC_WORK_IDLE:
			return IdleMessage(iMessage, wParam, lParam);

		case WM_GETMINMAXINFO:
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_MOUSELEAVE:
		case WM_MOUSEWHEEL:
			return MouseMessage(iMessage, wParam, lParam);

		case WM_SETCURSOR:
			if (LOWORD(lParam) == HTCLIENT) {
				POINT pt;
				if (::GetCursorPos(&pt)) {
					::ScreenToClient(MainHWND(), &pt);
					DisplayCursor(ContextCursor(PointFromPOINT(pt)));
				}
				return TRUE;
			}
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_UNICHAR:
			return KeyMessage(iMessage, wParam, lParam);

		case WM_SETTINGCHANGE:
			//Platform::DebugPrintf("Setting Changed\n");
			InvalidateStyleData();
			// Get Intellimouse scroll line parameters
			GetIntelliMouseParameters();
			break;

		case WM_GETDLGCODE:
			return DLGC_HASSETSEL | DLGC_WANTALLKEYS;

		case WM_KILLFOCUS:
		case WM_SETFOCUS:
			return FocusMessage(iMessage, wParam, lParam);

		case WM_SYSCOLORCHANGE:
			//Platform::DebugPrintf("Setting Changed\n");
			InvalidateStyleData();
			break;

		case WM_DPICHANGED:
			dpi.x = LOWORD(wParam);
			dpi.y = HIWORD(wParam);
			vs.fontsValid = false;
			InvalidateStyleRedraw();
			break;

		case WM_DPICHANGED_AFTERPARENT: {
			DPI_T const dpiNow = GetWindowDPI(MainHWND());
			if ((dpi.x != dpiNow.x) || (dpi.y != dpiNow.y)) {
				dpi = dpiNow;
				vs.fontsValid = false;
				InvalidateStyleRedraw();
			}
		}
		break;

		case WM_CONTEXTMENU:
#if SCI_EnablePopupMenu
			return ShowContextMenu(iMessage, wParam, lParam);
#else
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
#endif

		case WM_ERASEBKGND:
			return 1;   // Avoid any background erasure as whole window painted.

		case WM_CAPTURECHANGED:
			capturedMouse = false;
			return 0;

		// These are not handled in Scintilla and its faster to dispatch them here.
		// Also moves time out to here so profile doesn't count lots of empty message calls.

		case WM_MOVE:
		case WM_MOUSEACTIVATE:
		case WM_NCHITTEST:
		case WM_NCCALCSIZE:
		case WM_NCPAINT:
		case WM_NCMOUSEMOVE:
		case WM_NCLBUTTONDOWN:
		case WM_SYSCOMMAND:
		case WM_WINDOWPOSCHANGING:
		case WM_WINDOWPOSCHANGED:
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

		case WM_GETTEXTLENGTH:
			return GetTextLength();

		case WM_GETTEXT:
			return GetText(wParam, lParam);

		case WM_INPUTLANGCHANGE:
		case WM_INPUTLANGCHANGEREQUEST:
		case WM_IME_KEYDOWN:
		case WM_IME_REQUEST:
		case WM_IME_STARTCOMPOSITION:
		case WM_IME_ENDCOMPOSITION:
		case WM_IME_COMPOSITION:
		case WM_IME_SETCONTEXT:
		case WM_IME_NOTIFY:
			return IMEMessage(iMessage, wParam, lParam);

		case EM_LINEFROMCHAR:
		case EM_EXLINEFROMCHAR:
		case EM_FINDTEXT:
		case EM_FINDTEXTEX:
		case EM_FORMATRANGE:
		case EM_GETTEXTRANGE:
		case EM_GETSEL:
		case EM_EXGETSEL:
		case EM_SETSEL:
		case EM_EXSETSEL:
		case EM_LINELENGTH:
		case EM_POSFROMCHAR:
		case EM_GETZOOM:
		case EM_SETZOOM:
			return EditMessage(iMessage, wParam, lParam);

		case SCI_GETDIRECTFUNCTION:
		case SCI_GETDIRECTPOINTER:
		case SCI_GRABFOCUS:
#ifdef INCLUDE_DEPRECATED_FEATURES
		case SCI_SETKEYSUNICODE:
		case SCI_GETKEYSUNICODE:
#endif
		case SCI_SETTECHNOLOGY:
		case SCI_SETBIDIRECTIONAL:
		case SCI_TARGETASUTF8:
		case SCI_ENCODEDFROMUTF8:
			return SciMessage(iMessage, wParam, lParam);

		default:
			return ScintillaBase::WndProc(iMessage, wParam, lParam);
		}
	} catch (std::bad_alloc &) {
		errorStatus = SC_STATUS_BADALLOC;
	} catch (...) {
		errorStatus = SC_STATUS_FAILURE;
	}
	return 0;
}

bool ScintillaWin::ValidCodePage(int codePage) const noexcept {
	return codePage == 0 || codePage == SC_CP_UTF8 ||
		codePage == 932 || codePage == 936 || codePage == 949 ||
		codePage == 950 || codePage == 1361;
}

sptr_t ScintillaWin::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) noexcept {
	return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
}

bool ScintillaWin::FineTickerRunning(TickReason reason) noexcept {
	return timers[reason] != 0;
}

void ScintillaWin::FineTickerStart(TickReason reason, int millis, int tolerance) noexcept {
	FineTickerCancel(reason);
	const UINT_PTR eventID = fineTimerStart + static_cast<UINT_PTR>(reason);
#if _WIN32_WINNT < _WIN32_WINNT_WIN8
	if (SetCoalescableTimerFn && tolerance) {
		timers[reason] = SetCoalescableTimerFn(MainHWND(), eventID, millis, nullptr, tolerance);
	} else {
		timers[reason] = ::SetTimer(MainHWND(), eventID, millis, nullptr);
	}
#else
	if (tolerance) {
		timers[reason] = ::SetCoalescableTimer(MainHWND(), eventID, millis, nullptr, tolerance);
	} else {
		timers[reason] = ::SetTimer(MainHWND(), eventID, millis, nullptr);
	}
#endif
}

void ScintillaWin::FineTickerCancel(TickReason reason) noexcept {
	if (timers[reason]) {
		::KillTimer(MainHWND(), timers[reason]);
		timers[reason] = 0;
	}
}

bool ScintillaWin::SetIdle(bool on) noexcept {
	// On Win32 the Idler is implemented as a Timer on the Scintilla window.  This
	// takes advantage of the fact that WM_TIMER messages are very low priority,
	// and are only posted when the message queue is empty, i.e. during idle time.
	if (idler.state != on) {
		if (on) {
			idler.idlerID = ::SetTimer(MainHWND(), idleTimerID, 10, nullptr)
				? reinterpret_cast<IdlerID>(idleTimerID) : nullptr;
		} else {
			::KillTimer(MainHWND(), reinterpret_cast<uptr_t>(idler.idlerID));
			idler.idlerID = nullptr;
		}
		idler.state = idler.idlerID != nullptr;
	}
	return idler.state;
}

void ScintillaWin::IdleWork() {
	styleIdleInQueue = false;
	Editor::IdleWork();
}

void ScintillaWin::QueueIdleWork(WorkNeeded::workItems items, Sci::Position upTo) noexcept {
	Editor::QueueIdleWork(items, upTo);
	if (!styleIdleInQueue) {
		if (PostMessage(MainHWND(), SC_WORK_IDLE, 0, 0)) {
			styleIdleInQueue = true;
		}
	}
}

void ScintillaWin::SetMouseCapture(bool on) noexcept {
	if (mouseDownCaptures) {
		if (on) {
			::SetCapture(MainHWND());
		} else {
			::ReleaseCapture();
		}
	}
	capturedMouse = on;
}

bool ScintillaWin::HaveMouseCapture() noexcept {
	// Cannot just see if GetCapture is this window as the scroll bar also sets capture for the window
	return capturedMouse;
	//return capturedMouse && (::GetCapture() == MainHWND());
}

void ScintillaWin::SetTrackMouseLeaveEvent(bool on) noexcept {
	if (on && !trackedMouseLeave) {
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = MainHWND();
		tme.dwHoverTime = HOVER_DEFAULT;	// Unused but triggers Dr. Memory if not initialized
		TrackMouseEvent(&tme);
	}
	trackedMouseLeave = on;
}

bool ScintillaWin::PaintContains(PRectangle rc) const noexcept {
	if (paintState == painting) {
		return BoundsContains(rcPaint, hRgnUpdate, rc);
	}
	return true;
}

void ScintillaWin::ScrollText(Sci::Line /* linesToMove */) {
	//Platform::DebugPrintf("ScintillaWin::ScrollText %d\n", linesToMove);
	//::ScrollWindow(MainHWND(), 0,
	//	vs.lineHeight * linesToMove, 0, 0);
	//::UpdateWindow(MainHWND());
	Redraw();
	UpdateSystemCaret();
}

void ScintillaWin::NotifyCaretMove() noexcept {
	NotifyWinEvent(EVENT_OBJECT_LOCATIONCHANGE, MainHWND(), OBJID_CARET, CHILDID_SELF);
}

void ScintillaWin::UpdateSystemCaret() {
	if (hasFocus) {
		if (pdoc->TentativeActive()) {
			// ongoing inline mode IME composition, don't inform IME of system caret position.
			// fix candidate window for Google Japanese IME moved on typing on Win7.
			return;
		}
		if (HasCaretSizeChanged()) {
			DestroySystemCaret();
			CreateSystemCaret();
		}
		const Point pos = PointMainCaret();
		::SetCaretPos(static_cast<int>(pos.x), static_cast<int>(pos.y));
	}
}

int ScintillaWin::SetScrollInfo(int nBar, LPCSCROLLINFO lpsi, BOOL bRedraw) const noexcept {
	return ::SetScrollInfo(MainHWND(), nBar, lpsi, bRedraw);
}

bool ScintillaWin::GetScrollInfo(int nBar, LPSCROLLINFO lpsi) const noexcept {
	return ::GetScrollInfo(MainHWND(), nBar, lpsi) != 0;
}

// Change the scroll position but avoid repaint if changing to same value
void ScintillaWin::ChangeScrollPos(int barType, Sci::Position pos) {
	SCROLLINFO sci = {
		sizeof(sci), 0, 0, 0, 0, 0, 0
	};
	sci.fMask = SIF_POS;
	GetScrollInfo(barType, &sci);
	if (sci.nPos != pos) {
		DwellEnd(true);
		sci.nPos = static_cast<int>(pos);
		SetScrollInfo(barType, &sci, TRUE);
	}
}

void ScintillaWin::SetVerticalScrollPos() {
	ChangeScrollPos(SB_VERT, topLine);
}

void ScintillaWin::SetHorizontalScrollPos() {
	ChangeScrollPos(SB_HORZ, xOffset);
}

bool ScintillaWin::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) {
	bool modified = false;
	SCROLLINFO sci = {
		sizeof(sci), 0, 0, 0, 0, 0, 0
	};
	sci.fMask = SIF_PAGE | SIF_RANGE;
	GetScrollInfo(SB_VERT, &sci);
	const Sci::Line vertEndPreferred = nMax;
	if (!verticalScrollBarVisible)
		nPage = vertEndPreferred + 1;
	if ((sci.nMin != 0) ||
		(sci.nMax != vertEndPreferred) ||
		(sci.nPage != static_cast<unsigned int>(nPage)) ||
		(sci.nPos != 0)) {
		sci.fMask = SIF_PAGE | SIF_RANGE;
		sci.nMin = 0;
		sci.nMax = static_cast<int>(vertEndPreferred);
		sci.nPage = static_cast<UINT>(nPage);
		sci.nPos = 0;
		sci.nTrackPos = 1;
		SetScrollInfo(SB_VERT, &sci, TRUE);
		modified = true;
	}

	const PRectangle rcText = GetTextRectangle();
	int horizEndPreferred = scrollWidth;
	if (horizEndPreferred < 0)
		horizEndPreferred = 0;
	int pageWidth = static_cast<int>(rcText.Width());
	if (!horizontalScrollBarVisible || Wrapping())
		pageWidth = horizEndPreferred + 1;
	sci.fMask = SIF_PAGE | SIF_RANGE;
	GetScrollInfo(SB_HORZ, &sci);
	if ((sci.nMin != 0) ||
		(sci.nMax != horizEndPreferred) ||
		(sci.nPage != static_cast<unsigned int>(pageWidth)) ||
		(sci.nPos != 0)) {
		sci.fMask = SIF_PAGE | SIF_RANGE;
		sci.nMin = 0;
		sci.nMax = horizEndPreferred;
		sci.nPage = pageWidth;
		sci.nPos = 0;
		sci.nTrackPos = 1;
		SetScrollInfo(SB_HORZ, &sci, TRUE);
		modified = true;
		if (scrollWidth < pageWidth) {
			HorizontalScrollTo(0);
		}
	}
	return modified;
}

void ScintillaWin::NotifyChange() noexcept {
	::SendMessage(::GetParent(MainHWND()), WM_COMMAND,
		MAKEWPARAM(GetCtrlID(), SCEN_CHANGE),
		reinterpret_cast<LPARAM>(MainHWND()));
}

void ScintillaWin::NotifyFocus(bool focus) {
	if (commandEvents) {
		::SendMessage(::GetParent(MainHWND()), WM_COMMAND,
			MAKEWPARAM(GetCtrlID(), focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS),
			reinterpret_cast<LPARAM>(MainHWND()));
	}
	Editor::NotifyFocus(focus);
}

void ScintillaWin::SetCtrlID(int identifier) noexcept {
	::SetWindowID(HwndFromWindow(wMain), identifier);
}

int ScintillaWin::GetCtrlID() const noexcept {
	return ::GetDlgCtrlID(HwndFromWindow(wMain));
}

void ScintillaWin::NotifyParent(SCNotification scn) noexcept {
	scn.nmhdr.hwndFrom = MainHWND();
	scn.nmhdr.idFrom = GetCtrlID();
	::SendMessage(::GetParent(MainHWND()), WM_NOTIFY,
		GetCtrlID(), reinterpret_cast<LPARAM>(&scn));
}

void ScintillaWin::NotifyDoubleClick(Point pt, int modifiers) {
	//Platform::DebugPrintf("ScintillaWin Double click 0\n");
	ScintillaBase::NotifyDoubleClick(pt, modifiers);
	// Send myself a WM_LBUTTONDBLCLK, so the container can handle it too.
	::SendMessage(::GetParent(MainHWND()),
		WM_LBUTTONDBLCLK,
		(modifiers & SCI_SHIFT) ? MK_SHIFT : 0,
		MAKELPARAM(pt.x, pt.y));
}

void ScintillaWin::NotifyURIDropped(const char *list) noexcept {
	SCNotification scn = {};
	scn.nmhdr.code = SCN_URIDROPPED;
	scn.text = list;

	NotifyParent(scn);
}

class CaseFolderDBCS : public CaseFolderTable {
	// Allocate the expandable storage here so that it does not need to be reallocated
	// for each call to Fold.
	std::vector<wchar_t> utf16Mixed;
	std::vector<wchar_t> utf16Folded;
	UINT cp;
public:
	explicit CaseFolderDBCS(UINT cp_) noexcept : cp(cp_) {
		StandardASCII();
	}
	size_t Fold(char *folded, size_t sizeFolded, const char *mixed, size_t lenMixed) override {
		if ((lenMixed == 1) && (sizeFolded > 0)) {
			folded[0] = mapping[static_cast<unsigned char>(mixed[0])];
			return 1;
		} else {
			if (lenMixed > utf16Mixed.size()) {
				utf16Mixed.resize(lenMixed + 8);
			}
			const size_t nUtf16Mixed = WideCharFromMultiByte(cp,
				std::string_view(mixed, lenMixed),
				utf16Mixed.data(),
				utf16Mixed.size());

			if (nUtf16Mixed == 0) {
				// Failed to convert -> bad input
				folded[0] = '\0';
				return 1;
			}

			size_t lenFlat = 0;
			for (size_t mixIndex = 0; mixIndex < nUtf16Mixed; mixIndex++) {
				if ((lenFlat + 20) > utf16Folded.size())
					utf16Folded.resize(lenFlat + 60);
				const char *foldedUTF8 = CaseConvert(utf16Mixed[mixIndex], CaseConversionFold);
				if (foldedUTF8) {
					// Maximum length of a case conversion is 6 bytes, 3 characters
					wchar_t wFolded[20];
					const size_t charsConverted = UTF16FromUTF8(std::string_view(foldedUTF8),
						wFolded, std::size(wFolded));
					for (size_t j = 0; j < charsConverted; j++) {
						utf16Folded[lenFlat++] = wFolded[j];
					}
				} else {
					utf16Folded[lenFlat++] = utf16Mixed[mixIndex];
				}
			}

			const std::wstring_view wsvFolded(utf16Folded.data(), lenFlat);
			const size_t lenOut = MultiByteLenFromWideChar(cp, wsvFolded);

			if (lenOut < sizeFolded) {
				MultiByteFromWideChar(cp, wsvFolded, folded, lenOut);
				return lenOut;
			} else {
				return 0;
			}
		}
	}
};

CaseFolder *ScintillaWin::CaseFolderForEncoding() {
	const UINT cpDest = CodePageOfDocument();
	if (cpDest == SC_CP_UTF8) {
		return new CaseFolderUnicode();
	} else {
		if (pdoc->dbcsCodePage == 0) {
			CaseFolderTable *pcf = new CaseFolderTable();
			pcf->StandardASCII();
			// Only for single byte encodings
			for (int i = 0x80; i < 0x100; i++) {
				char sCharacter[2] = "A";
				sCharacter[0] = static_cast<char>(i);
				wchar_t wCharacter[20];
				const unsigned int lengthUTF16 = WideCharFromMultiByte(cpDest, sCharacter,
					wCharacter, std::size(wCharacter));
				if (lengthUTF16 == 1) {
					const char *caseFolded = CaseConvert(wCharacter[0], CaseConversionFold);
					if (caseFolded) {
						wchar_t wLower[20];
						const size_t charsConverted = UTF16FromUTF8(std::string_view(caseFolded),
							wLower, std::size(wLower));
						if (charsConverted == 1) {
							char sCharacterLowered[20];
							const unsigned int lengthConverted = MultiByteFromWideChar(cpDest,
								std::wstring_view(wLower, charsConverted),
								sCharacterLowered, std::size(sCharacterLowered));
							if ((lengthConverted == 1) && (sCharacter[0] != sCharacterLowered[0])) {
								pcf->SetTranslation(sCharacter[0], sCharacterLowered[0]);
							}
						}
					}
				}
			}
			return pcf;
		} else {
			return new CaseFolderDBCS(cpDest);
		}
	}
}

std::string ScintillaWin::CaseMapString(const std::string &s, int caseMapping) {
	if (s.empty() || (caseMapping == cmSame))
		return s;

	const UINT cpDoc = CodePageOfDocument();
	if (cpDoc == SC_CP_UTF8) {
		return CaseConvertString(s, (caseMapping == cmUpper) ? CaseConversionUpper : CaseConversionLower);
	}

	// Change text to UTF-16
	const std::wstring wsText = StringDecode(s, cpDoc);

	const DWORD mapFlags = LCMAP_LINGUISTIC_CASING |
		((caseMapping == cmUpper) ? LCMAP_UPPERCASE : LCMAP_LOWERCASE);

	// Change case
	const std::wstring wsConverted = StringMapCase(wsText, mapFlags);

	// Change back to document encoding
	std::string sConverted = StringEncode(wsConverted, cpDoc);

	return sConverted;
}

void ScintillaWin::Copy(bool asBinary) {
	//Platform::DebugPrintf("Copy\n");
	if (!sel.Empty()) {
		SelectionText selectedText;
		selectedText.asBinary = asBinary;
		CopySelectionRange(&selectedText);
		CopyToClipboard(selectedText);
	}
}

bool ScintillaWin::CanPaste() noexcept {
	if (!Editor::CanPaste())
		return false;
	return ::IsClipboardFormatAvailable(CF_UNICODETEXT);
}

namespace {

class GlobalMemory {
	HGLOBAL hand {};
public:
	void *ptr {};
	GlobalMemory() noexcept = default;
	explicit GlobalMemory(HGLOBAL hand_) noexcept : hand(hand_) {
		if (hand) {
			ptr = ::GlobalLock(hand);
		}
	}
	// Deleted so GlobalMemory objects can not be copied.
	GlobalMemory(const GlobalMemory &) = delete;
	GlobalMemory(GlobalMemory &&) = delete;
	GlobalMemory &operator=(const GlobalMemory &) = delete;
	GlobalMemory &operator=(GlobalMemory &&) = delete;
	~GlobalMemory() {
		assert(!ptr);
		assert(!hand);
	}
	void Allocate(size_t bytes) noexcept {
		assert(!hand);
		hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
		if (hand) {
			ptr = ::GlobalLock(hand);
		}
	}
	HGLOBAL Unlock() noexcept {
		assert(ptr);
		HGLOBAL handCopy = hand;
		::GlobalUnlock(hand);
		ptr = nullptr;
		hand = nullptr;
		return handCopy;
	}
	void SetClip(UINT uFormat) noexcept {
		::SetClipboardData(uFormat, Unlock());
	}
	operator bool() const noexcept {
		return ptr != nullptr;
	}
	SIZE_T Size() noexcept {
		return ::GlobalSize(hand);
	}
};

// OpenClipboard may fail if another application has opened the clipboard.
// Try up to 8 times, with an initial delay of 1 ms and an exponential back off
// for a maximum total delay of 127 ms (1+2+4+8+16+32+64).
bool OpenClipboardRetry(HWND hwnd) noexcept {
	for (int attempt = 0; attempt < 8; attempt++) {
		if (attempt > 0) {
			::Sleep(1 << (attempt - 1));
		}
		if (::OpenClipboard(hwnd)) {
			return true;
		}
	}
	return false;
}

inline bool IsValidFormatEtc(const FORMATETC *pFE) noexcept {
	return pFE->ptd == nullptr
		&& (pFE->dwAspect & DVASPECT_CONTENT) != 0
		&& pFE->lindex == -1
		&& (pFE->tymed & TYMED_HGLOBAL) != 0;
}

inline bool SupportedFormat(const FORMATETC *pFE) noexcept {
	return (pFE->cfFormat == CF_UNICODETEXT || pFE->cfFormat == CF_TEXT)
		&& IsValidFormatEtc(pFE);
}

}

void ScintillaWin::Paste(bool asBinary) {
	if (!::OpenClipboardRetry(MainHWND())) {
		return;
	}

	UndoGroup ug(pdoc);
	//EnumAllClipboardFormat("Paste");
	const bool isLine = SelectionEmpty() &&
		(::IsClipboardFormatAvailable(cfLineSelect) || ::IsClipboardFormatAvailable(cfVSLineTag));
	ClearSelection(multiPasteMode == SC_MULTIPASTE_EACH);
	bool isRectangular = (::IsClipboardFormatAvailable(cfColumnSelect) != 0);

	if (!isRectangular) {
		// Evaluate "Borland IDE Block Type" explicitly
		GlobalMemory memBorlandSelection(::GetClipboardData(cfBorlandIDEBlockType));
		if (memBorlandSelection) {
			isRectangular = (memBorlandSelection.Size() == 1) && (static_cast<BYTE *>(memBorlandSelection.ptr)[0] == 0x02);
			memBorlandSelection.Unlock();
		}
	}

	const PasteShape pasteShape = isRectangular ? pasteRectangular : (isLine ? pasteLine : pasteStream);

	if (asBinary) {
		// get data with CF_TEXT, decode and verify length information
		if (!asBinary) {
			::CloseClipboard();
			Redraw();
			return;
		}
	}

	// Use CF_UNICODETEXT if available
	GlobalMemory memUSelection(::GetClipboardData(CF_UNICODETEXT));
	if (const wchar_t *uptr = static_cast<const wchar_t *>(memUSelection.ptr)) {
		const std::string putf = EncodeWString(uptr);
		InsertPasteShape(putf.c_str(), putf.length(), pasteShape);
		memUSelection.Unlock();
	}
	::CloseClipboard();
	Redraw();
}

void ScintillaWin::CreateCallTipWindow(PRectangle) noexcept {
	if (!ct.wCallTip.Created()) {
		HWND wnd = ::CreateWindow(callClassName, L"ACallTip",
			WS_POPUP, 100, 100, 150, 20,
			MainHWND(), nullptr,
			GetWindowInstance(MainHWND()),
			this);
		ct.wCallTip = wnd;
		ct.wDraw = wnd;
	}
}

#if SCI_EnablePopupMenu
void ScintillaWin::AddToPopUp(const char *label, int cmd, bool enabled) noexcept {
	HMENU hmenuPopup = static_cast<HMENU>(popup.GetID());
	if (!label[0])
		::AppendMenuA(hmenuPopup, MF_SEPARATOR, 0, "");
	else if (enabled)
		::AppendMenuA(hmenuPopup, MF_STRING, cmd, label);
	else
		::AppendMenuA(hmenuPopup, MF_STRING | MF_DISABLED | MF_GRAYED, cmd, label);
}
#endif

void ScintillaWin::ClaimSelection() noexcept {
	// Windows does not have a primary selection
}

/// Implement IUnknown
STDMETHODIMP FormatEnumerator::QueryInterface(REFIID riid, PVOID *ppv) noexcept {
	//Platform::DebugPrintf("EFE QI");
	*ppv = nullptr;
	if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC) {
		*ppv = this;
	} else {
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}
STDMETHODIMP_(ULONG)FormatEnumerator::AddRef() noexcept {
	return ++ref;
}
STDMETHODIMP_(ULONG)FormatEnumerator::Release() noexcept {
	const ULONG refs = --ref;
	if (refs == 0) {
		delete this;
	}
	return refs;
}

/// Implement IEnumFORMATETC
STDMETHODIMP FormatEnumerator::Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched) noexcept {
	if (!rgelt) return E_POINTER;
	ULONG putPos = 0;
	while ((pos < formats.size()) && (putPos < celt)) {
		rgelt->cfFormat = formats[pos];
		rgelt->ptd = nullptr;
		rgelt->dwAspect = DVASPECT_CONTENT;
		rgelt->lindex = -1;
		rgelt->tymed = TYMED_HGLOBAL;
		rgelt++;
		pos++;
		putPos++;
	}
	if (pceltFetched)
		*pceltFetched = putPos;
	return putPos ? S_OK : S_FALSE;
}
STDMETHODIMP FormatEnumerator::Skip(ULONG celt) noexcept {
	pos += celt;
	return S_OK;
}
STDMETHODIMP FormatEnumerator::Reset() noexcept {
	pos = 0;
	return S_OK;
}
STDMETHODIMP FormatEnumerator::Clone(IEnumFORMATETC **ppenum) {
	FormatEnumerator *pfe;
	try {
		pfe = new FormatEnumerator(pos, formats.data(), formats.size());
	} catch (...) {
		return E_OUTOFMEMORY;
	}
	return pfe->QueryInterface(IID_IEnumFORMATETC, reinterpret_cast<PVOID *>(ppenum));
}

FormatEnumerator::FormatEnumerator(ULONG pos_, const CLIPFORMAT formats_[], size_t formatsLen_) {
	ref = 0;   // First QI adds first reference...
	pos = pos_;
	formats.insert(formats.begin(), formats_, formats_ + formatsLen_);
}

/// Implement IUnknown
STDMETHODIMP DropSource::QueryInterface(REFIID riid, PVOID *ppv) noexcept {
	return sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DropSource::AddRef() noexcept {
	return sci->AddRef();
}
STDMETHODIMP_(ULONG)DropSource::Release() noexcept {
	return sci->Release();
}

/// Implement IDropSource
STDMETHODIMP DropSource::QueryContinueDrag(BOOL fEsc, DWORD grfKeyState) noexcept {
	if (fEsc)
		return DRAGDROP_S_CANCEL;
	if (!(grfKeyState & MK_LBUTTON))
		return DRAGDROP_S_DROP;
	return S_OK;
}
STDMETHODIMP DropSource::GiveFeedback(DWORD) noexcept {
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

/// Implement IUnkown
STDMETHODIMP DataObject::QueryInterface(REFIID riid, PVOID *ppv) noexcept {
	//Platform::DebugPrintf("DO QI %p\n", this);
	return sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DataObject::AddRef() noexcept {
	return sci->AddRef();
}
STDMETHODIMP_(ULONG)DataObject::Release() noexcept {
	return sci->Release();
}

/// Implement IDataObject
STDMETHODIMP DataObject::GetData(FORMATETC *pFEIn, STGMEDIUM *pSTM) {
	return sci->GetData(pFEIn, pSTM);
}

STDMETHODIMP DataObject::GetDataHere(FORMATETC *, STGMEDIUM *) noexcept {
	//Platform::DebugPrintf("DOB GetDataHere\n");
	return E_NOTIMPL;
}

STDMETHODIMP DataObject::QueryGetData(FORMATETC *pFE) noexcept {
	if (sci->DragIsRectangularOK(pFE->cfFormat) && IsValidFormatEtc(pFE)) {
		return S_OK;
	}

	return SupportedFormat(pFE)? S_OK : S_FALSE;
}

STDMETHODIMP DataObject::GetCanonicalFormatEtc(FORMATETC *, FORMATETC *pFEOut) noexcept {
	//Platform::DebugPrintf("DOB GetCanon\n");
	pFEOut->cfFormat = CF_UNICODETEXT;
	pFEOut->ptd = nullptr;
	pFEOut->dwAspect = DVASPECT_CONTENT;
	pFEOut->lindex = -1;
	pFEOut->tymed = TYMED_HGLOBAL;
	return S_OK;
}

STDMETHODIMP DataObject::SetData(FORMATETC *, STGMEDIUM *, BOOL) noexcept {
	//Platform::DebugPrintf("DOB SetData\n");
	return E_FAIL;
}

STDMETHODIMP DataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnum) {
	try {
		//Platform::DebugPrintf("DOB EnumFormatEtc %lu\n", dwDirection);
		if (dwDirection != DATADIR_GET) {
			*ppEnum = nullptr;
			return E_FAIL;
		}

		const CLIPFORMAT formats[] = { CF_UNICODETEXT, CF_TEXT };
		FormatEnumerator *pfe = new FormatEnumerator(0, formats, std::size(formats));
		return pfe->QueryInterface(IID_IEnumFORMATETC, reinterpret_cast<PVOID *>(ppEnum));
	} catch (std::bad_alloc &) {
		sci->errorStatus = SC_STATUS_BADALLOC;
		return E_OUTOFMEMORY;
	} catch (...) {
		sci->errorStatus = SC_STATUS_FAILURE;
		return E_FAIL;
	}
}

STDMETHODIMP DataObject::DAdvise(FORMATETC *, DWORD, IAdviseSink *, PDWORD) noexcept {
	//Platform::DebugPrintf("DOB DAdvise\n");
	return E_FAIL;
}

STDMETHODIMP DataObject::DUnadvise(DWORD) noexcept {
	//Platform::DebugPrintf("DOB DUnadvise\n");
	return E_FAIL;
}

STDMETHODIMP DataObject::EnumDAdvise(IEnumSTATDATA **) noexcept {
	//Platform::DebugPrintf("DOB EnumDAdvise\n");
	return E_FAIL;
}

/// Implement IUnknown
STDMETHODIMP DropTarget::QueryInterface(REFIID riid, PVOID *ppv) noexcept {
	//Platform::DebugPrintf("DT QI %p\n", this);
	return sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DropTarget::AddRef() noexcept {
	return sci->AddRef();
}
STDMETHODIMP_(ULONG)DropTarget::Release() noexcept {
	return sci->Release();
}

/// Implement IDropTarget by forwarding to Scintilla
STDMETHODIMP DropTarget::DragEnter(LPDATAOBJECT pIDataSource, DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	return sci->DragEnter(pIDataSource, grfKeyState, pt, pdwEffect);
}
STDMETHODIMP DropTarget::DragOver(DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	return sci->DragOver(grfKeyState, pt, pdwEffect);
}
STDMETHODIMP DropTarget::DragLeave() {
	return sci->DragLeave();
}
STDMETHODIMP DropTarget::Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	return sci->Drop(pIDataSource, grfKeyState, pt, pdwEffect);
}

/**
 * DBCS: support Input Method Editor (IME).
 * Called when IME Window opened.
 */
void ScintillaWin::ImeStartComposition() {
	if (caret.active) {
		// Move IME Window to current caret position
		IMContext imc(MainHWND());
		const Point pos = PointMainCaret();
		COMPOSITIONFORM CompForm;
		CompForm.dwStyle = CFS_POINT;
		CompForm.ptCurrentPos = POINTFromPoint(pos);

		::ImmSetCompositionWindow(imc.hIMC, &CompForm);

		// Set font of IME window to same as surrounded text.
		if (stylesValid) {
			// Since the style creation code has been made platform independent,
			// The logfont for the IME is recreated here.
			const int styleHere = pdoc->StyleIndexAt(sel.MainCaret());
			LOGFONTW lf {};
			const int sizeZoomed = GetFontSizeZoomed(vs.styles[styleHere].size, vs.zoomLevel);
			// The negative is to allow for leading
			lf.lfHeight = -::MulDiv(sizeZoomed, dpi.y, 72 * SC_FONT_SIZE_MULTIPLIER);
			lf.lfWidth = 0; // vs.styles[styleHere].stretch ?
			lf.lfWeight = vs.styles[styleHere].weight;
			lf.lfItalic = vs.styles[styleHere].italic ? TRUE : FALSE;
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfFaceName[0] = L'\0';
			if (vs.styles[styleHere].fontName) {
				const char* fontName = vs.styles[styleHere].fontName;
				UTF16FromUTF8(std::string_view(fontName), lf.lfFaceName, LF_FACESIZE);
			}

			::ImmSetCompositionFontW(imc.hIMC, &lf);
		}
		// Caret is displayed in IME window. So, caret in Scintilla is useless.
		DropCaret();
	}
}

/** Called when IME Window closed.
* TODO: see Chromium's InputMethodWinImm32::OnImeEndComposition().
*/
void ScintillaWin::ImeEndComposition() {
	// clear IME composition state.
	view.imeCaretBlockOverride = false;
	pdoc->TentativeUndo();
	ShowCaretAtCurrentPosition();
}

LRESULT ScintillaWin::ImeOnReconvert(LPARAM lParam) {
	// Reconversion on windows limits within one line without eol.
	// Look around:   baseStart  <--  (|mainStart|  -- mainEnd)  --> baseEnd.
	const Sci::Position mainStart = sel.RangeMain().Start().Position();
	const Sci::Position mainEnd = sel.RangeMain().End().Position();
	const Sci::Line curLine = pdoc->SciLineFromPosition(mainStart);
	if (curLine != pdoc->LineFromPosition(mainEnd))
		return 0;
	const Sci::Position baseStart = pdoc->LineStart(curLine);
	const Sci::Position baseEnd = pdoc->LineEnd(curLine);
	if ((baseStart == baseEnd) || (mainEnd > baseEnd))
		return 0;

	const UINT codePage = CodePageOfDocument();
	const std::wstring rcFeed = StringDecode(RangeText(baseStart, baseEnd), codePage);
	const DWORD rcFeedLen = static_cast<DWORD>(rcFeed.length()) * sizeof(wchar_t);
	const DWORD rcSize = sizeof(RECONVERTSTRING) + rcFeedLen + sizeof(wchar_t);

	RECONVERTSTRING *rc = static_cast<RECONVERTSTRING *>(PtrFromSPtr(lParam));
	if (!rc)
		return rcSize; // Immediately be back with rcSize of memory block.

	wchar_t *rcFeedStart = reinterpret_cast<wchar_t*>(rc + 1);
	memcpy(rcFeedStart, rcFeed.data(), rcFeedLen);

	std::string rcCompString = RangeText(mainStart, mainEnd);
	std::wstring rcCompWstring = StringDecode(rcCompString, codePage);
	std::string rcCompStart = RangeText(baseStart, mainStart);
	std::wstring rcCompWstart = StringDecode(rcCompStart, codePage);

	// Map selection to dwCompStr.
	// No selection assumes current caret as rcCompString without length.
	rc->dwVersion = 0; // It should be absolutely 0.
	rc->dwStrLen = static_cast<DWORD>(rcFeed.length());
	rc->dwStrOffset = sizeof(RECONVERTSTRING);
	rc->dwCompStrLen = static_cast<DWORD>(rcCompWstring.length());
	rc->dwCompStrOffset = static_cast<DWORD>(rcCompWstart.length()) * sizeof(wchar_t);
	rc->dwTargetStrLen = rc->dwCompStrLen;
	rc->dwTargetStrOffset = rc->dwCompStrOffset;

	IMContext imc(MainHWND());
	if (!imc.hIMC)
		return 0;

	if (!::ImmSetCompositionStringW(imc.hIMC, SCS_QUERYRECONVERTSTRING, rc, rcSize, nullptr, 0))
		return 0;

	// No selection asks IME to fill target fields with its own value.
	const DWORD tgWlen = rc->dwTargetStrLen;
	const DWORD tgWstart = rc->dwTargetStrOffset / sizeof(wchar_t);

	std::string tgCompStart = StringEncode(rcFeed.substr(0, tgWstart), codePage);
	std::string tgComp = StringEncode(rcFeed.substr(tgWstart, tgWlen), codePage);

	// No selection needs to adjust reconvert start position for IME set.
	const Sci::Position adjust = tgCompStart.length() - rcCompStart.length();
	const Sci::Position docCompLen = tgComp.length();

	// Make place for next composition string to sit in.
	for (size_t r = 0; r < sel.Count(); r++) {
		const Sci::Position rBase = sel.Range(r).Start().Position();
		const Sci::Position docCompStart = rBase + adjust;

		if (inOverstrike) { // the docCompLen of bytes will be overstriked.
			sel.Range(r).caret.SetPosition(docCompStart);
			sel.Range(r).anchor.SetPosition(docCompStart);
		} else {
			// Ensure docCompStart+docCompLen be not beyond lineEnd.
			// since docCompLen by byte might break eol.
			const Sci::Position lineEnd = pdoc->LineEnd(pdoc->LineFromPosition(rBase));
			const Sci::Position overflow = (docCompStart + docCompLen) - lineEnd;
			if (overflow > 0) {
				pdoc->DeleteChars(docCompStart, docCompLen - overflow);
			} else {
				pdoc->DeleteChars(docCompStart, docCompLen);
			}
		}
	}
	// Immediately Target Input or candidate box choice with GCS_COMPSTR.
	return rcSize;
}

void ScintillaWin::GetIntelliMouseParameters() noexcept {
	// This retrieves the number of lines per scroll as configured in the Mouse Properties sheet in Control Panel
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &linesPerScroll, 0);
}

void ScintillaWin::CopyToGlobal(GlobalMemory &gmUnicode, const SelectionText &selectedText, CopyEncoding encoding) {
	const std::string_view svSelected(selectedText.Data(), selectedText.LengthWithTerminator());
	switch (encoding) {
	case CopyEncoding::Unicode:
		if (IsUnicodeMode()) {
			const size_t uchars = UTF16Length(svSelected);
			gmUnicode.Allocate(2 * uchars);
			if (gmUnicode) {
				UTF16FromUTF8(svSelected, static_cast<wchar_t *>(gmUnicode.ptr), uchars);
			}
		} else {
			// Not Unicode mode
			// Convert to Unicode using the current Scintilla code page
			const UINT cpSrc = CodePageFromCharSet(selectedText.characterSet, selectedText.codePage);
			const size_t uLen = WideCharLenFromMultiByte(cpSrc, svSelected);
			gmUnicode.Allocate(2 * uLen);
			if (gmUnicode) {
				WideCharFromMultiByte(cpSrc, svSelected, static_cast<wchar_t *>(gmUnicode.ptr), uLen);
			}
		}
		break;

	case CopyEncoding::Ansi: {
		std::string s;
		if (IsUnicodeMode()) {
			const std::wstring wsv = StringDecode(svSelected, CP_UTF8);
			s = StringEncode(wsv, CP_ACP);
		} else {
			// no need to convert selectedText to CP_ACP
			s = svSelected;
		}
		gmUnicode.Allocate(s.size() + 1);
		if (gmUnicode) {
			memcpy(gmUnicode.ptr, s.c_str(), s.size());
		}
	}
	break;

	case CopyEncoding::Binary:
		gmUnicode.Allocate(svSelected.size());
		if (gmUnicode) {
			memcpy(gmUnicode.ptr, svSelected.data(), svSelected.size() - 1);
		}
		break;
	}
}

void ScintillaWin::CopyToClipboard(const SelectionText &selectedText) {
	if (!::OpenClipboardRetry(MainHWND())) {
		return;
	}
	::EmptyClipboard();

	GlobalMemory uniText;
	CopyToGlobal(uniText, selectedText, selectedText.asBinary ? CopyEncoding::Binary : CopyEncoding::Unicode);

	if (uniText) {
		uniText.SetClip(selectedText.asBinary ? CF_TEXT : CF_UNICODETEXT);

		if (selectedText.asBinary) {
			// encode length information
		}
	}

	if (selectedText.rectangular) {
		::SetClipboardData(cfColumnSelect, nullptr);

		GlobalMemory borlandSelection;
		borlandSelection.Allocate(1);
		if (borlandSelection) {
			static_cast<BYTE *>(borlandSelection.ptr)[0] = 0x02;
			borlandSelection.SetClip(cfBorlandIDEBlockType);
		}
	}

	if (selectedText.lineCopy) {
		::SetClipboardData(cfLineSelect, nullptr);
		::SetClipboardData(cfVSLineTag, nullptr);
	}

	::CloseClipboard();

	// TODO: notify data loss
	//if (!selectedText.asBinary && ) {
	//}
}

void ScintillaWin::ScrollMessage(WPARAM wParam) {
	//DWORD dwStart = timeGetTime();
	//Platform::DebugPrintf("Scroll %x %d\n", wParam, lParam);

	SCROLLINFO sci = {};
	sci.cbSize = sizeof(sci);
	sci.fMask = SIF_ALL;

	GetScrollInfo(SB_VERT, &sci);

	//Platform::DebugPrintf("ScrollInfo %d mask=%x min=%d max=%d page=%d pos=%d track=%d\n", b, sci.fMask,
	//sci.nMin, sci.nMax, sci.nPage, sci.nPos, sci.nTrackPos);
	Sci::Line topLineNew = topLine;
	switch (LOWORD(wParam)) {
	case SB_LINEUP:
		topLineNew -= 1;
		break;
	case SB_LINEDOWN:
		topLineNew += 1;
		break;
	case SB_PAGEUP:
		topLineNew -= LinesToScroll(); break;
	case SB_PAGEDOWN: topLineNew += LinesToScroll(); break;
	case SB_TOP: topLineNew = 0; break;
	case SB_BOTTOM: topLineNew = MaxScrollPos(); break;
	case SB_THUMBPOSITION: topLineNew = sci.nTrackPos; break;
	case SB_THUMBTRACK: topLineNew = sci.nTrackPos; break;
	}
	ScrollTo(topLineNew);
}

void ScintillaWin::HorizontalScrollMessage(WPARAM wParam) {
	int xPos = xOffset;
	const PRectangle rcText = GetTextRectangle();
	const int pageWidth = static_cast<int>(rcText.Width() * 2 / 3);
	switch (LOWORD(wParam)) {
	case SB_LINEUP:
		xPos -= 20;
		break;
	case SB_LINEDOWN:	// May move past the logical end
		xPos += 20;
		break;
	case SB_PAGEUP:
		xPos -= pageWidth;
		break;
	case SB_PAGEDOWN:
		xPos += pageWidth;
		if (xPos > scrollWidth - rcText.Width()) {	// Hit the end exactly
			xPos = scrollWidth - static_cast<int>(rcText.Width());
		}
		break;
	case SB_TOP:
		xPos = 0;
		break;
	case SB_BOTTOM:
		xPos = scrollWidth;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK: {
		// Do NOT use wParam, its 16 bit and not enough for very long lines. Its still possible to overflow the 32 bit but you have to try harder =]
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_TRACKPOS;
		if (GetScrollInfo(SB_HORZ, &si)) {
			xPos = si.nTrackPos;
		}
	}
	break;
	}
	HorizontalScrollTo(xPos);
}

/**
 * Redraw all of text area.
 * This paint will not be abandoned.
 */
void ScintillaWin::FullPaint() {
	if ((technology == SC_TECHNOLOGY_DEFAULT) || (technology == SC_TECHNOLOGY_DIRECTWRITEDC)) {
		HDC hdc = ::GetDC(MainHWND());
		FullPaintDC(hdc);
		::ReleaseDC(MainHWND(), hdc);
	} else {
		FullPaintDC({});
	}
}

/**
 * Redraw all of text area on the specified DC.
 * This paint will not be abandoned.
 */
void ScintillaWin::FullPaintDC(HDC hdc) {
	paintState = painting;
	rcPaint = GetClientRectangle();
	paintingAllText = true;
	PaintDC(hdc);
	paintState = notPainting;
}

namespace {

inline bool CompareDevCap(HDC hdc, HDC hOtherDC, int nIndex) noexcept {
	return ::GetDeviceCaps(hdc, nIndex) == ::GetDeviceCaps(hOtherDC, nIndex);
}

}

bool ScintillaWin::IsCompatibleDC(HDC hOtherDC) const noexcept {
	HDC hdc = ::GetDC(MainHWND());
	const bool isCompatible =
		CompareDevCap(hdc, hOtherDC, TECHNOLOGY) &&
		CompareDevCap(hdc, hOtherDC, LOGPIXELSY) &&
		CompareDevCap(hdc, hOtherDC, LOGPIXELSX) &&
		CompareDevCap(hdc, hOtherDC, BITSPIXEL) &&
		CompareDevCap(hdc, hOtherDC, PLANES);
	::ReleaseDC(MainHWND(), hdc);
	return isCompatible;
}

// https://docs.microsoft.com/en-us/windows/desktop/api/oleidl/nf-oleidl-idroptarget-dragenter
DWORD ScintillaWin::EffectFromState(DWORD grfKeyState) const noexcept {
	// These are the Wordpad semantics.
	// DROPEFFECT_COPY not works for some applications like Github Atom.
	DWORD dwEffect = DROPEFFECT_MOVE;
#if 0
	if (inDragDrop == ddDragging)	// Internal defaults to move
		dwEffect = DROPEFFECT_MOVE;
	else
		dwEffect = DROPEFFECT_COPY;
	if ((grfKeyState & MK_CONTROL) && (grfKeyState & MK_SHIFT))
		dwEffect = DROPEFFECT_LINK;
	else
	if (grfKeyState & (MK_ALT | MK_SHIFT))
		dwEffect = DROPEFFECT_MOVE;
	else
#endif
	if (grfKeyState & MK_CONTROL)
		dwEffect = DROPEFFECT_COPY;
	return dwEffect;
}

/// Implement IUnknown
STDMETHODIMP ScintillaWin::QueryInterface(REFIID riid, PVOID *ppv) noexcept {
	*ppv = nullptr;
	if (riid == IID_IUnknown) {
		*ppv = &dt;
	} else if (riid == IID_IDropSource) {
		*ppv = &ds;
	} else if (riid == IID_IDropTarget) {
		*ppv = &dt;
	} else if (riid == IID_IDataObject) {
		*ppv = &dob;
	}
	if (!*ppv) {
		return E_NOINTERFACE;
	}
	return S_OK;
}

STDMETHODIMP_(ULONG) ScintillaWin::AddRef() noexcept {
	return 1;
}

STDMETHODIMP_(ULONG) ScintillaWin::Release() noexcept {
	return 1;
}

#if DebugDragAndDropDataFormat

namespace {

const char* GetStorageMediumType(DWORD tymed) noexcept {
	switch (tymed) {
	case TYMED_HGLOBAL:
		return "TYMED_HGLOBAL";
	case TYMED_FILE:
		return "TYMED_FILE";
	case TYMED_ISTREAM:
		return "TYMED_ISTREAM";
	case TYMED_ISTORAGE:
		return "TYMED_ISTORAGE";
	default:
		return "Unknown";
	}
}

const char* GetSourceFormatName(UINT fmt, char name[], int cchName) noexcept {
	const int len = GetClipboardFormatNameA(fmt, name, cchName);
	if (len <= 0) {
		switch (fmt) {
		case CF_TEXT:
			return "CF_TEXT";
		case CF_UNICODETEXT:
			return "CF_UNICODETEXT";
		case CF_HDROP:
			return "CF_HDROP";
		case CF_LOCALE:
			return "CF_LOCALE";
		case CF_OEMTEXT:
			return "CF_OEMTEXT";
		default:
			return "Unknown";
		}
	}

	return name;
}

}

// https://docs.microsoft.com/en-us/windows/desktop/api/objidl/nf-objidl-idataobject-enumformatetc
void ScintillaWin::EnumDataSourceFormat(const char *tag, LPDATAOBJECT pIDataSource) {
	IEnumFORMATETC *fmtEnum = nullptr;
	HRESULT hr = pIDataSource->EnumFormatEtc(DATADIR_GET, &fmtEnum);
	if (hr == S_OK && fmtEnum) {
		FORMATETC fetc[32] = {};
		ULONG fetched = 0;
		hr = fmtEnum->Next(sizeof(fetc) / sizeof(fetc[0]), fetc, &fetched);
		if (fetched > 0) {
			char name[1024];
			char buf[2048];
			for (ULONG i = 0; i < fetched; i++) {
				const CLIPFORMAT fmt = fetc[i].cfFormat;
				const DWORD tymed = fetc[i].tymed;
				const char *typeName = GetStorageMediumType(tymed);
				const char *fmtName = GetSourceFormatName(fmt, name, sizeof(name));
				const int len = sprintf(buf, "%s: fmt[%lu]=%u, 0x%04X; tymed=%lu, %s; name=%s\n",
					tag, i, fmt, fmt, tymed, typeName, fmtName);
				InsertCharacter(std::string_view(buf, len), CharacterSource::tentativeInput);
			}
		}
	}
	if (fmtEnum) {
		fmtEnum->Release();
	}
}

// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-enumclipboardformats
void ScintillaWin::EnumAllClipboardFormat(const char *tag) {
	UINT fmt = 0;
	unsigned int i = 0;
	char name[1024];
	char buf[2048];
	while ((fmt = ::EnumClipboardFormats(fmt)) != 0) {
		const char *fmtName = GetSourceFormatName(fmt, name, sizeof(name));
		const int len = sprintf(buf, "%s: fmt[%u]=%u, 0x%04X; name=%s\n",
			tag, i, fmt, fmt, fmtName);
		InsertCharacter(std::string_view(buf, len), CharacterSource::tentativeInput);
		i++;
	}
}

#endif

/// Implement IDropTarget
STDMETHODIMP ScintillaWin::DragEnter(LPDATAOBJECT pIDataSource, DWORD grfKeyState, POINTL, PDWORD pdwEffect) {
	if (!pIDataSource) {
		return E_POINTER;
	}

	hasOKText = false;
	try {
		//EnumDataSourceFormat("DragEnter", pIDataSource);

		for (UINT fmtIndex = 0; fmtIndex < dropFormatCount; fmtIndex++) {
			const CLIPFORMAT fmt = dropFormat[fmtIndex];
			FORMATETC fmtu = { fmt, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			const HRESULT hrHasUText = pIDataSource->QueryGetData(&fmtu);
			hasOKText = (hrHasUText == S_OK);
			if (hasOKText) {
				break;
			}
		}
	} catch (...) {
		errorStatus = SC_STATUS_FAILURE;
	}

	*pdwEffect = hasOKText? EffectFromState(grfKeyState) : DROPEFFECT_NONE;
	return S_OK;
}

STDMETHODIMP ScintillaWin::DragOver(DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	try {
		if (!hasOKText || pdoc->IsReadOnly()) {
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}

		*pdwEffect = EffectFromState(grfKeyState);

		// Update the cursor.
		POINT rpt = { pt.x, pt.y };
		::ScreenToClient(MainHWND(), &rpt);
		SetDragPosition(SPositionFromLocation(PointFromPOINT(rpt), false, false, UserVirtualSpace()));

		return S_OK;
	} catch (...) {
		errorStatus = SC_STATUS_FAILURE;
	}
	return E_FAIL;
}

STDMETHODIMP ScintillaWin::DragLeave() {
	try {
		SetDragPosition(SelectionPosition(Sci::invalidPosition));
		return S_OK;
	} catch (...) {
		errorStatus = SC_STATUS_FAILURE;
	}
	return E_FAIL;
}

STDMETHODIMP ScintillaWin::Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	try {
		*pdwEffect = EffectFromState(grfKeyState);

		if (!pIDataSource) {
			return E_POINTER;
		}

		SetDragPosition(SelectionPosition(Sci::invalidPosition));

		std::string putf;
		bool fileDrop = false;
		HRESULT hr = DV_E_FORMATETC;

		//EnumDataSourceFormat("Drop", pIDataSource);
		for (UINT fmtIndex = 0; fmtIndex < dropFormatCount; fmtIndex++) {
			const CLIPFORMAT fmt = dropFormat[fmtIndex];
			FORMATETC fmtu = { fmt, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM medium {};
			hr = pIDataSource->GetData(&fmtu, &medium);

			if (SUCCEEDED(hr) && medium.hGlobal) {
				// File Drop
				if (fmt == CF_HDROP
#if EnableDrop_VisualStudioProjectItem
					|| fmt == cfVSStgProjectItem || fmt == cfVSRefProjectItem
#endif
					) {
					WCHAR pathDropped[1024];
					HDROP hDrop = static_cast<HDROP>(medium.hGlobal);
					if (::DragQueryFileW(hDrop, 0, pathDropped, sizeof(pathDropped)/sizeof(WCHAR)) > 0) {
						WCHAR *p = pathDropped;
#if EnableDrop_VisualStudioProjectItem
						if (fmt == cfVSStgProjectItem || fmt == cfVSRefProjectItem) {
							// {UUID}|Solution\Project.[xx]proj|path
							WCHAR *t = StrRChrW(p, nullptr, L'|');
							if (t) {
								p = t + 1;
							}
						}
#endif
						putf = StringEncode(p, CP_UTF8);
						fileDrop = true;
					}
					// TODO: This seems not required, MSDN only says it need be called in WM_DROPFILES
					::DragFinish(hDrop);
				}
#if Enable_ChromiumWebCustomMIMEDataFormat
				else if (fmt == cfChromiumCustomMIME) {
					GlobalMemory memUDrop(medium.hGlobal);
					if (const wchar_t *uptr = static_cast<const wchar_t *>(memUDrop.ptr)) {
						const std::wstring_view wsv(uptr, memUDrop.Size() / 2);
						// parse file url: "resource":"file:///path"
						const size_t dataLen = UTF8Length(wsv);
					}
					memUDrop.Unlock();
				}
#endif
				// Unicode Text
				else if (fmt == CF_UNICODETEXT) {
					GlobalMemory memUDrop(medium.hGlobal);
					if (const wchar_t *uptr = static_cast<const wchar_t *>(memUDrop.ptr)) {
						putf = EncodeWString(uptr);
					}
					memUDrop.Unlock();
				}
				// ANSI Text
				else if (fmt == CF_TEXT) {
					GlobalMemory memDrop(medium.hGlobal);
					if (const char *ptr = static_cast<const char *>(memDrop.ptr)) {
						const std::string_view sv(ptr, strnlen(ptr, memDrop.Size()));
						// In Unicode mode, convert text to UTF-8
						if (IsUnicodeMode()) {
							const std::wstring wsv = StringDecode(sv, CP_ACP);
							putf = StringEncode(wsv, CP_UTF8);
						} else {
							// no need to convert ptr from CP_ACP to CodePageOfDocument().
							putf = sv;
						}
					}
					memDrop.Unlock();
				}
			}

			::ReleaseStgMedium(&medium);
			if (!putf.empty()) {
				break;
			}
		}

		if (!SUCCEEDED(hr)) {
			return hr;
		}
		if (putf.empty()) {
			return S_OK;
		}

		if (fileDrop) {
			NotifyURIDropped(putf.c_str());
		} else {
			FORMATETC fmtr = { cfColumnSelect, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			const bool isRectangular = (S_OK == pIDataSource->QueryGetData(&fmtr));

			POINT rpt = { pt.x, pt.y };
			::ScreenToClient(MainHWND(), &rpt);
			const SelectionPosition movePos = SPositionFromLocation(PointFromPOINT(rpt), false, false, UserVirtualSpace());

			DropAt(movePos, putf.c_str(), putf.size(), *pdwEffect == DROPEFFECT_MOVE, isRectangular);
		}

		return S_OK;
	} catch (...) {
		errorStatus = SC_STATUS_FAILURE;
	}
	return E_FAIL;
}

/// Implement important part of IDataObject
STDMETHODIMP ScintillaWin::GetData(const FORMATETC *pFEIn, STGMEDIUM *pSTM) {
	if (!SupportedFormat(pFEIn)) {
		//Platform::DebugPrintf("DOB GetData No %d %x %x fmt=%x\n", lenDrag, pFEIn, pSTM, pFEIn->cfFormat);
		return DATA_E_FORMATETC;
	}

	pSTM->tymed = TYMED_HGLOBAL;
	//Platform::DebugPrintf("DOB GetData OK %d %x %x\n", lenDrag, pFEIn, pSTM);

	GlobalMemory uniText;
	CopyToGlobal(uniText, drag, (pFEIn->cfFormat == CF_TEXT)? CopyEncoding::Ansi : CopyEncoding::Unicode);
	pSTM->hGlobal = uniText ? uniText.Unlock() : nullptr;
	pSTM->pUnkForRelease = nullptr;
	return S_OK;
}

BOOL CALLBACK ScintillaWin::PrepareOnce(PINIT_ONCE initOnce, PVOID parameter, PVOID *lpContext) noexcept
{
#if USE_WIN32_INIT_ONCE
	UNREFERENCED_PARAMETER(initOnce);
	UNREFERENCED_PARAMETER(parameter);
	UNREFERENCED_PARAMETER(lpContext);
#endif

	Platform_Initialise(hInstance);
	//~CharClassify::InitUnicodeData();

	// Register the CallTip class
	WNDCLASSEX wndclassc {};
	wndclassc.cbSize = sizeof(wndclassc);
	wndclassc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclassc.cbWndExtra = sizeof(ScintillaWin *);
	wndclassc.hInstance = hInstance;
	wndclassc.lpfnWndProc = ScintillaWin::CTWndProc;
	wndclassc.hCursor = ::LoadCursor({}, IDC_ARROW);
	wndclassc.lpszClassName = callClassName;

	callClassAtom = ::RegisterClassEx(&wndclassc);
	return TRUE;
}

bool ScintillaWin::Register(HINSTANCE hInstance_) noexcept {
	hInstance = hInstance_;

	// Register the Scintilla class
	// Register Scintilla as a wide character window
	WNDCLASSEXW wndclass {};
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = ScintillaWin::SWndProc;
	wndclass.cbWndExtra = sizeof(ScintillaWin *);
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = L"Scintilla";
	scintillaClassAtom = ::RegisterClassExW(&wndclass);

	const bool result = (0 != scintillaClassAtom);
	return result;
}

bool ScintillaWin::Unregister() noexcept {
	bool result = true;
	if (0 != scintillaClassAtom) {
		if (::UnregisterClass(MAKEINTATOM(scintillaClassAtom), hInstance) == 0) {
			result = false;
		}
		scintillaClassAtom = 0;
	}
	if (0 != callClassAtom) {
		if (::UnregisterClass(MAKEINTATOM(callClassAtom), hInstance) == 0) {
			result = false;
		}
		callClassAtom = 0;
	}
	return result;
}

bool ScintillaWin::HasCaretSizeChanged() const noexcept {
	if (
		((0 != vs.caretWidth) && (sysCaretWidth != vs.caretWidth))
		|| ((0 != vs.lineHeight) && (sysCaretHeight != vs.lineHeight))
		) {
		return true;
	}
	return false;
}

BOOL ScintillaWin::CreateSystemCaret() {
	sysCaretWidth = vs.caretWidth;
	if (0 == sysCaretWidth) {
		sysCaretWidth = 1;
	}
	sysCaretHeight = vs.lineHeight;
	const int bitmapSize = (((sysCaretWidth + 15) & ~15) >> 3) *
		sysCaretHeight;
	std::vector<BYTE> bits(bitmapSize);
	sysCaretBitmap = ::CreateBitmap(sysCaretWidth, sysCaretHeight, 1,
		1, bits.data());
	const BOOL retval = ::CreateCaret(
		MainHWND(), sysCaretBitmap,
		sysCaretWidth, sysCaretHeight);
	if (technology == SC_TECHNOLOGY_DEFAULT) {
		// System caret interferes with Direct2D drawing so only show it for GDI.
		::ShowCaret(MainHWND());
	}
	return retval;
}

BOOL ScintillaWin::DestroySystemCaret() noexcept {
	::HideCaret(MainHWND());
	const BOOL retval = ::DestroyCaret();
	if (sysCaretBitmap) {
		::DeleteObject(sysCaretBitmap);
		sysCaretBitmap = {};
	}
	return retval;
}

LRESULT CALLBACK ScintillaWin::CTWndProc(
	HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	// Find C++ object associated with window.
	ScintillaWin *sciThis = static_cast<ScintillaWin *>(PointerFromWindow(hWnd));
	try {
		// ctp will be zero if WM_CREATE not seen yet
		if (sciThis == nullptr) {
			if (iMessage == WM_CREATE) {
				// Associate CallTip object with window
				CREATESTRUCT *pCreate = static_cast<CREATESTRUCT *>(PtrFromSPtr(lParam));
				SetWindowPointer(hWnd, pCreate->lpCreateParams);
				return 0;
			} else {
				return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
			}
		} else {
			if (iMessage == WM_NCDESTROY) {
				SetWindowPointer(hWnd, nullptr);
				return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
			} else if (iMessage == WM_PAINT) {
				PAINTSTRUCT ps;
				::BeginPaint(hWnd, &ps);
				std::unique_ptr<Surface> surfaceWindow(Surface::Allocate(sciThis->technology));
#if defined(USE_D2D)
				ID2D1HwndRenderTarget *pCTRenderTarget = nullptr;
#endif
				RECT rc;
				GetClientRect(hWnd, &rc);
				// Create a Direct2D render target.
				if (sciThis->technology == SC_TECHNOLOGY_DEFAULT) {
					surfaceWindow->Init(ps.hdc, hWnd);
				} else {
#if defined(USE_D2D)
					D2D1_HWND_RENDER_TARGET_PROPERTIES dhrtp;
					dhrtp.hwnd = hWnd;
					dhrtp.pixelSize = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
					dhrtp.presentOptions = (sciThis->technology == SC_TECHNOLOGY_DIRECTWRITERETAIN) ?
						D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS : D2D1_PRESENT_OPTIONS_NONE;

					D2D1_RENDER_TARGET_PROPERTIES drtp;
					drtp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
					drtp.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
					drtp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_UNKNOWN;
					drtp.dpiX = 96.0;
					drtp.dpiY = 96.0;
					drtp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
					drtp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

					if (!SUCCEEDED(pD2DFactory->CreateHwndRenderTarget(drtp, dhrtp, &pCTRenderTarget))) {
						surfaceWindow->Release();
						::EndPaint(hWnd, &ps);
						ReleaseUnknown(pCTRenderTarget);
						return 0;
					}
					// If above SUCCEEDED, then pCTRenderTarget not nullptr
					assert(pCTRenderTarget);
					if (pCTRenderTarget) {
						surfaceWindow->Init(pCTRenderTarget, hWnd);
						pCTRenderTarget->BeginDraw();
					}
#endif
				}
				surfaceWindow->SetUnicodeMode(SC_CP_UTF8 == sciThis->ct.codePage);
				surfaceWindow->SetDBCSMode(sciThis->ct.codePage);
				surfaceWindow->SetBidiR2L(sciThis->BidirectionalR2L());
				sciThis->ct.PaintCT(surfaceWindow.get());
#if defined(USE_D2D)
				if (pCTRenderTarget)
					pCTRenderTarget->EndDraw();
#endif
				surfaceWindow->Release();
#if defined(USE_D2D)
				ReleaseUnknown(pCTRenderTarget);
#endif
				::EndPaint(hWnd, &ps);
				return 0;
			} else if ((iMessage == WM_NCLBUTTONDOWN) || (iMessage == WM_NCLBUTTONDBLCLK)) {
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ScreenToClient(hWnd, &pt);
				sciThis->ct.MouseClick(PointFromPOINT(pt));
				sciThis->CallTipClick();
				return 0;
			} else if (iMessage == WM_LBUTTONDOWN) {
				// This does not fire due to the hit test code
				sciThis->ct.MouseClick(PointFromLParam(lParam));
				sciThis->CallTipClick();
				return 0;
			} else if (iMessage == WM_SETCURSOR) {
				::SetCursor(::LoadCursor({}, IDC_ARROW));
				return 0;
			} else if (iMessage == WM_NCHITTEST) {
				return HTCAPTION;
			} else {
				return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
			}
		}
	} catch (...) {
		sciThis->errorStatus = SC_STATUS_FAILURE;
	}
	return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
}

sptr_t ScintillaWin::DirectFunction(
	sptr_t ptr, UINT iMessage, uptr_t wParam, sptr_t lParam) {
	PLATFORM_ASSERT(::GetCurrentThreadId() == ::GetWindowThreadProcessId(reinterpret_cast<ScintillaWin *>(ptr)->MainHWND(), nullptr));
	return reinterpret_cast<ScintillaWin *>(ptr)->WndProc(iMessage, wParam, lParam);
}

#ifdef SCINTILLA_DLL
namespace Scintilla {
	sptr_t DirectFunction(ScintillaWin* sci, UINT iMessage, uptr_t wParam, sptr_t lParam) {
		return sci->WndProc(iMessage, wParam, lParam);
	}
}
#else
extern "C"
sptr_t __stdcall Scintilla_DirectFunction(
	ScintillaWin* sci, UINT iMessage, uptr_t wParam, sptr_t lParam) {
	return sci->WndProc(iMessage, wParam, lParam);
}
#endif

LRESULT CALLBACK ScintillaWin::SWndProc(
	HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	//Platform::DebugPrintf("S W:%x M:%x WP:%x L:%x\n", hWnd, iMessage, wParam, lParam);

	// Find C++ object associated with window.
	ScintillaWin *sci = static_cast<ScintillaWin *>(PointerFromWindow(hWnd));
	// sci will be zero if WM_CREATE not seen yet
	if (sci == nullptr) {
		try {
			if (iMessage == WM_CREATE) {
				static INIT_ONCE once = INIT_ONCE_STATIC_INIT;
				::InitOnceExecuteOnce(&once, PrepareOnce, nullptr, nullptr);
				// Create C++ object associated with window
				sci = new ScintillaWin(hWnd);
				SetWindowPointer(hWnd, sci);
				return sci->WndProc(iMessage, wParam, lParam);
			}
		} catch (...) {
		}
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	} else {
		if (iMessage == WM_NCDESTROY) {
			try {
				//sci->Finalise();
				delete sci; // Finalise() in d'tor
			} catch (...) {
			}
			SetWindowPointer(hWnd, nullptr);
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
		} else {
			return sci->WndProc(iMessage, wParam, lParam);
		}
	}
}

namespace Scintilla {

// This function is externally visible so it can be called from container when building statically.
// Must be called once only.
extern "C" __declspec(dllexport)
int Scintilla_RegisterClasses(void *hInstance) {

#if _WIN32_WINNT < _WIN32_WINNT_WIN8
	// see LoadD2D() in PlatWin.cxx
	kSystemLibraryLoadFlags = (IsWindows8Point1OrGreater() || GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetDefaultDllDirectories")) ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0;
#endif

	Platform_Initialise(hInstance);

	const bool result = ScintillaWin::Register(static_cast<HINSTANCE>(hInstance));
	//@@@CharClassify::InitUnicodeData();
	
#ifdef SCI_LEXER
	Scintilla_LinkLexers();
#endif
	return result;
}

	int ResourcesRelease(bool fromDllMain) noexcept {
		const bool result = ScintillaWin::Unregister();
		Platform_Finalise(fromDllMain);
		return result;
	}


// This function is externally visible so it can be called from container when building statically.
extern "C" __declspec(dllexport)
int Scintilla_ReleaseResources(void) {
	return Scintilla::ResourcesRelease(false);
}

extern "C" __declspec(dllexport)
int Scintilla_InputCodePage(void) {
	return InputCodePage();
}

extern "C" __declspec(dllexport)
DPI_T Scintilla_GetWindowDPI(void* hwnd) {
	return GetWindowDPI(static_cast<HWND>(hwnd));
}

extern "C" __declspec(dllexport)
int Scintilla_GetSystemMetricsForDpi(int nIndex, DPI_T dpi) {
	return SystemMetricsForDpi(nIndex, dpi.y);
}

extern "C" __declspec(dllexport)
int Scintilla_AdjustWindowRectForDpi(WRCT_T* lpRect, unsigned long dwStyle, unsigned long dwExStyle, DPI_T dpi) {
	RECT rc;
	rc.left = lpRect->left; rc.top = lpRect->top; rc.right = lpRect->right;	rc.bottom = lpRect->bottom;
	BOOL const res =  AdjustWindowRectForDpi(&rc, dwStyle, dwExStyle, dpi.y);
	lpRect->left = rc.left; lpRect->top = rc.top; lpRect->right = rc.right;	lpRect->bottom = rc.bottom;
	return (int)res;
}

} // namespace Scintilla
