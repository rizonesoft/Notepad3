// Scintilla source code edit control
/** @file PlatWin.cxx
 ** Implementation of platform facilities on Windows.
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cmath>
#include <climits>

#include <vector>
#include <map>
#include <algorithm>
#include <memory>

// Want to use std::min and std::max so don't want Windows.h version of min and max
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#undef WINVER
#define WINVER 0x0500
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <windowsx.h>

#if !defined(DISABLE_D2D)
#define USE_D2D 1
#endif

#if defined(USE_D2D)
#include <d2d1.h>
#include <dwrite.h>
#endif

#include "Platform.h"
#include "StringCopy.h"
#include "XPM.h"
#include "UniConversion.h"
#include "DBCS.h"
#include "FontQuality.h"

#include "PlatWin.h"

#ifndef SPI_GETFONTSMOOTHINGCONTRAST
#define SPI_GETFONTSMOOTHINGCONTRAST	0x200C
#endif

#ifndef LOAD_LIBRARY_SEARCH_SYSTEM32
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800
#endif

namespace Scintilla {

UINT CodePageFromCharSet(DWORD characterSet, UINT documentCodePage);

RECT RectFromPRectangle(PRectangle prc) {
	RECT rc = {static_cast<LONG>(prc.left), static_cast<LONG>(prc.top),
		static_cast<LONG>(prc.right), static_cast<LONG>(prc.bottom)};
	return rc;
}

#if defined(USE_D2D)
IDWriteFactory *pIDWriteFactory = 0;
ID2D1Factory *pD2DFactory = 0;
IDWriteRenderingParams *defaultRenderingParams = 0;
IDWriteRenderingParams *customClearTypeRenderingParams = 0;

static HMODULE hDLLD2D = NULL;
static HMODULE hDLLDWrite = NULL;

bool LoadD2D() {
	static bool triedLoadingD2D = false;
	if (!triedLoadingD2D) {
		DWORD loadLibraryFlags = 0;
		HMODULE kernel32 = ::GetModuleHandle(TEXT("kernel32.dll"));
		if (kernel32) {
			if (::GetProcAddress(kernel32, "SetDefaultDllDirectories")) {
				// Availability of SetDefaultDllDirectories implies Windows 8+ or
				// that KB2533623 has been installed so LoadLibraryEx can be called
				// with LOAD_LIBRARY_SEARCH_SYSTEM32.
				loadLibraryFlags = LOAD_LIBRARY_SEARCH_SYSTEM32;
			}
		}

		typedef HRESULT (WINAPI *D2D1CFSig)(D2D1_FACTORY_TYPE factoryType, REFIID riid,
			CONST D2D1_FACTORY_OPTIONS *pFactoryOptions, IUnknown **factory);
		typedef HRESULT (WINAPI *DWriteCFSig)(DWRITE_FACTORY_TYPE factoryType, REFIID iid,
			IUnknown **factory);

		hDLLD2D = ::LoadLibraryEx(TEXT("D2D1.DLL"), 0, loadLibraryFlags);
		if (hDLLD2D) {
			D2D1CFSig fnD2DCF = reinterpret_cast<D2D1CFSig>(::GetProcAddress(hDLLD2D, "D2D1CreateFactory"));
			if (fnD2DCF) {
				// A single threaded factory as Scintilla always draw on the GUI thread
				fnD2DCF(D2D1_FACTORY_TYPE_SINGLE_THREADED,
					__uuidof(ID2D1Factory),
					0,
					reinterpret_cast<IUnknown**>(&pD2DFactory));
			}
		}
		hDLLDWrite = ::LoadLibraryEx(TEXT("DWRITE.DLL"), 0, loadLibraryFlags);
		if (hDLLDWrite) {
			DWriteCFSig fnDWCF = reinterpret_cast<DWriteCFSig>(::GetProcAddress(hDLLDWrite, "DWriteCreateFactory"));
			if (fnDWCF) {
				fnDWCF(DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(IDWriteFactory),
					reinterpret_cast<IUnknown**>(&pIDWriteFactory));
			}
		}

		if (pIDWriteFactory) {
			const HRESULT hr = pIDWriteFactory->CreateRenderingParams(&defaultRenderingParams);
			if (SUCCEEDED(hr)) {
				unsigned int clearTypeContrast;
				if (::SystemParametersInfo(SPI_GETFONTSMOOTHINGCONTRAST, 0, &clearTypeContrast, 0)) {

					FLOAT gamma;
					if (clearTypeContrast >= 1000 && clearTypeContrast <= 2200)
						gamma = static_cast<FLOAT>(clearTypeContrast) / 1000.0f;
					else
						gamma = defaultRenderingParams->GetGamma();

					pIDWriteFactory->CreateCustomRenderingParams(gamma, defaultRenderingParams->GetEnhancedContrast(), defaultRenderingParams->GetClearTypeLevel(),
						defaultRenderingParams->GetPixelGeometry(), defaultRenderingParams->GetRenderingMode(), &customClearTypeRenderingParams);
				}
			}
		}

	}
	triedLoadingD2D = true;
	return pIDWriteFactory && pD2DFactory;
}
#endif

struct FormatAndMetrics {
	int technology;
	HFONT hfont;
#if defined(USE_D2D)
	IDWriteTextFormat *pTextFormat;
#endif
	int extraFontFlag;
	int characterSet;
	FLOAT yAscent;
	FLOAT yDescent;
	FLOAT yInternalLeading;
	FormatAndMetrics(HFONT hfont_, int extraFontFlag_, int characterSet_) :
		technology(SCWIN_TECH_GDI), hfont(hfont_),
#if defined(USE_D2D)
		pTextFormat(0),
#endif
		extraFontFlag(extraFontFlag_), characterSet(characterSet_), yAscent(2), yDescent(1), yInternalLeading(0) {
	}
#if defined(USE_D2D)
	FormatAndMetrics(IDWriteTextFormat *pTextFormat_,
	        int extraFontFlag_,
	        int characterSet_,
	        FLOAT yAscent_,
	        FLOAT yDescent_,
	        FLOAT yInternalLeading_) :
		technology(SCWIN_TECH_DIRECTWRITE),
		hfont(0),
		pTextFormat(pTextFormat_),
		extraFontFlag(extraFontFlag_),
		characterSet(characterSet_),
		yAscent(yAscent_),
		yDescent(yDescent_),
		yInternalLeading(yInternalLeading_) {
	}
#endif
	FormatAndMetrics(const FormatAndMetrics &) = delete;
	FormatAndMetrics(FormatAndMetrics &&) = delete;
	FormatAndMetrics &operator=(const FormatAndMetrics &) = delete;
	FormatAndMetrics &operator=(FormatAndMetrics &&) = delete;

	~FormatAndMetrics() {
		if (hfont)
			::DeleteObject(hfont);
#if defined(USE_D2D)
		if (pTextFormat)
			pTextFormat->Release();
		pTextFormat = 0;
#endif
		extraFontFlag = 0;
		characterSet = 0;
		yAscent = 2;
		yDescent = 1;
		yInternalLeading = 0;
	}
	HFONT HFont();
};

HFONT FormatAndMetrics::HFont() {
	LOGFONTW lf = {};
#if defined(USE_D2D)
	if (technology == SCWIN_TECH_GDI) {
		if (0 == ::GetObjectW(hfont, sizeof(lf), &lf)) {
			return 0;
		}
	} else {
		const HRESULT hr = pTextFormat->GetFontFamilyName(lf.lfFaceName, LF_FACESIZE);
		if (!SUCCEEDED(hr)) {
			return 0;
		}
		lf.lfWeight = pTextFormat->GetFontWeight();
		lf.lfItalic = pTextFormat->GetFontStyle() == DWRITE_FONT_STYLE_ITALIC;
		lf.lfHeight = -static_cast<int>(pTextFormat->GetFontSize());
	}
#else
	if (0 == ::GetObjectW(hfont, sizeof(lf), &lf)) {
		return 0;
	}
#endif
	return ::CreateFontIndirectW(&lf);
}

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

namespace {

void *PointerFromWindow(HWND hWnd) {
	return reinterpret_cast<void *>(::GetWindowLongPtr(hWnd, 0));
}

void SetWindowPointer(HWND hWnd, void *ptr) {
	::SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(ptr));
}

CRITICAL_SECTION crPlatformLock;
HINSTANCE hinstPlatformRes = 0;

HCURSOR reverseArrowCursor = NULL;

FormatAndMetrics *FamFromFontID(void *fid) {
	return static_cast<FormatAndMetrics *>(fid);
}

BYTE Win32MapFontQuality(int extraFontFlag) {
	switch (extraFontFlag & SC_EFF_QUALITY_MASK) {

		case SC_EFF_QUALITY_NON_ANTIALIASED:
			return NONANTIALIASED_QUALITY;

		case SC_EFF_QUALITY_ANTIALIASED:
			return ANTIALIASED_QUALITY;

		case SC_EFF_QUALITY_LCD_OPTIMIZED:
			return CLEARTYPE_QUALITY;

		default:
			return SC_EFF_QUALITY_DEFAULT;
	}
}

#if defined(USE_D2D)
D2D1_TEXT_ANTIALIAS_MODE DWriteMapFontQuality(int extraFontFlag) {
	switch (extraFontFlag & SC_EFF_QUALITY_MASK) {

		case SC_EFF_QUALITY_NON_ANTIALIASED:
			return D2D1_TEXT_ANTIALIAS_MODE_ALIASED;

		case SC_EFF_QUALITY_ANTIALIASED:
			return D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE;

		case SC_EFF_QUALITY_LCD_OPTIMIZED:
			return D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;

		default:
			return D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
	}
}
#endif

void SetLogFont(LOGFONTW &lf, const char *faceName, int characterSet, float size, int weight, bool italic, int extraFontFlag) {
	lf = LOGFONTW();
	// The negative is to allow for leading
	lf.lfHeight = -(abs(lround(size)));
	lf.lfWeight = weight;
	lf.lfItalic = italic ? 1 : 0;
	lf.lfCharSet = static_cast<BYTE>(characterSet);
	lf.lfQuality = Win32MapFontQuality(extraFontFlag);
	UTF16FromUTF8(faceName, strlen(faceName)+1, lf.lfFaceName, LF_FACESIZE);
}

/**
 * Create a hash from the parameters for a font to allow easy checking for identity.
 * If one font is the same as another, its hash will be the same, but if the hash is the
 * same then they may still be different.
 */
int HashFont(const FontParameters &fp) noexcept {
	return
		static_cast<int>(fp.size) ^
		(fp.characterSet << 10) ^
		((fp.extraFontFlag & SC_EFF_QUALITY_MASK) << 9) ^
		((fp.weight/100) << 12) ^
		(fp.italic ? 0x20000000 : 0) ^
		(fp.technology << 15) ^
		fp.faceName[0];
}

}

class FontCached : Font {
	FontCached *next;
	int usage;
	float size;
	LOGFONTW lf;
	int technology;
	int hash;
	explicit FontCached(const FontParameters &fp);
	bool SameAs(const FontParameters &fp);
	void Release() override;

	static FontCached *first;
public:
	~FontCached() override {}
	static FontID FindOrCreate(const FontParameters &fp);
	static void ReleaseId(FontID fid_);
};

FontCached *FontCached::first = 0;

FontCached::FontCached(const FontParameters &fp) :
	next(0), usage(0), size(1.0), hash(0) {
	SetLogFont(lf, fp.faceName, fp.characterSet, fp.size, fp.weight, fp.italic, fp.extraFontFlag);
	technology = fp.technology;
	hash = HashFont(fp);
	fid = 0;
	if (technology == SCWIN_TECH_GDI) {
		HFONT hfont = ::CreateFontIndirectW(&lf);
		fid = new FormatAndMetrics(hfont, fp.extraFontFlag, fp.characterSet);
	} else {
#if defined(USE_D2D)
		IDWriteTextFormat *pTextFormat;
		const int faceSize = 200;
		WCHAR wszFace[faceSize];
		UTF16FromUTF8(fp.faceName, strlen(fp.faceName)+1, wszFace, faceSize);
		const FLOAT fHeight = fp.size;
		const DWRITE_FONT_STYLE style = fp.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
		HRESULT hr = pIDWriteFactory->CreateTextFormat(wszFace, NULL,
			static_cast<DWRITE_FONT_WEIGHT>(fp.weight),
			style,
			DWRITE_FONT_STRETCH_NORMAL, fHeight, L"en-us", &pTextFormat);
		if (SUCCEEDED(hr)) {
			pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

			FLOAT yAscent = 1.0f;
			FLOAT yDescent = 1.0f;
			FLOAT yInternalLeading = 0.0f;
			IDWriteTextLayout *pTextLayout = 0;
			hr = pIDWriteFactory->CreateTextLayout(L"X", 1, pTextFormat,
					100.0f, 100.0f, &pTextLayout);
			if (SUCCEEDED(hr)) {
				const int maxLines = 2;
				DWRITE_LINE_METRICS lineMetrics[maxLines]{};
				UINT32 lineCount = 0;
				hr = pTextLayout->GetLineMetrics(lineMetrics, maxLines, &lineCount);
				if (SUCCEEDED(hr)) {
					yAscent = lineMetrics[0].baseline;
					yDescent = lineMetrics[0].height - lineMetrics[0].baseline;

					FLOAT emHeight;
					hr = pTextLayout->GetFontSize(0, &emHeight);
					if (SUCCEEDED(hr)) {
						yInternalLeading = lineMetrics[0].height - emHeight;
					}
				}
				pTextLayout->Release();
				pTextFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, lineMetrics[0].height, lineMetrics[0].baseline);
			}
			fid = new FormatAndMetrics(pTextFormat, fp.extraFontFlag, fp.characterSet, yAscent, yDescent, yInternalLeading);
		}
#endif
	}
	usage = 1;
}

bool FontCached::SameAs(const FontParameters &fp) {
	if (
		(size == fp.size) &&
		(lf.lfWeight == fp.weight) &&
		(lf.lfItalic == (fp.italic ? 1 : 0)) &&
		(lf.lfCharSet == fp.characterSet) &&
		(lf.lfQuality == Win32MapFontQuality(fp.extraFontFlag)) &&
		(technology == fp.technology)) {
			wchar_t wszFace[LF_FACESIZE];
			UTF16FromUTF8(fp.faceName, strlen(fp.faceName)+1, wszFace, LF_FACESIZE);
			return 0 == wcscmp(lf.lfFaceName,wszFace);
	}
	return false;
}

void FontCached::Release() {
	delete FamFromFontID(fid);
	fid = nullptr;
}

FontID FontCached::FindOrCreate(const FontParameters &fp) {
	FontID ret = 0;
	::EnterCriticalSection(&crPlatformLock);
	const int hashFind = HashFont(fp);
	for (FontCached *cur=first; cur; cur=cur->next) {
		if ((cur->hash == hashFind) &&
			cur->SameAs(fp)) {
			cur->usage++;
			ret = cur->fid;
		}
	}
	if (ret == 0) {
		FontCached *fc = new FontCached(fp);
		fc->next = first;
		first = fc;
		ret = fc->fid;
	}
	::LeaveCriticalSection(&crPlatformLock);
	return ret;
}

void FontCached::ReleaseId(FontID fid_) {
	::EnterCriticalSection(&crPlatformLock);
	FontCached **pcur=&first;
	for (FontCached *cur=first; cur; cur=cur->next) {
		if (cur->fid == fid_) {
			cur->usage--;
			if (cur->usage == 0) {
				*pcur = cur->next;
				cur->Release();
				cur->next = 0;
				delete cur;
			}
			break;
		}
		pcur=&cur->next;
	}
	::LeaveCriticalSection(&crPlatformLock);
}

Font::Font() {
	fid = 0;
}

Font::~Font() {
}

#define FONTS_CACHED

void Font::Create(const FontParameters &fp) {
	Release();
	if (fp.faceName)
		fid = FontCached::FindOrCreate(fp);
}

void Font::Release() {
	if (fid)
		FontCached::ReleaseId(fid);
	fid = 0;
}

// Buffer to hold strings and string position arrays without always allocating on heap.
// May sometimes have string too long to allocate on stack. So use a fixed stack-allocated buffer
// when less than safe size otherwise allocate on heap and free automatically.
template<typename T, int lengthStandard>
class VarBuffer {
	T bufferStandard[lengthStandard];
public:
	T *buffer;
	explicit VarBuffer(size_t length) : buffer(nullptr) {
		if (length > lengthStandard) {
			buffer = new T[length];
		} else {
			buffer = bufferStandard;
		}
	}
	// Deleted so VarBuffer objects can not be copied.
	VarBuffer(const VarBuffer &) = delete;
	VarBuffer(VarBuffer &&) = delete;
	VarBuffer &operator=(const VarBuffer &) = delete;
	VarBuffer &operator=(VarBuffer &&) = delete;

	~VarBuffer() {
		if (buffer != bufferStandard) {
			delete []buffer;
			buffer = 0;
		}
	}
};

const int stackBufferLength = 1000;
class TextWide : public VarBuffer<wchar_t, stackBufferLength> {
public:
	int tlen;	// Using int instead of size_t as most Win32 APIs take int.
	TextWide(const char *s, int len, bool unicodeMode, int codePage=0) :
		VarBuffer<wchar_t, stackBufferLength>(len) {
		if (unicodeMode) {
			tlen = static_cast<int>(UTF16FromUTF8(s, len, buffer, len));
		} else {
			// Support Asian string display in 9x English
			tlen = ::MultiByteToWideChar(codePage, 0, s, len, buffer, len);
		}
	}
};
typedef VarBuffer<XYPOSITION, stackBufferLength> TextPositions;

class SurfaceGDI : public Surface {
	bool unicodeMode;
	HDC hdc;
	bool hdcOwned;
	HPEN pen;
	HPEN penOld;
	HBRUSH brush;
	HBRUSH brushOld;
	HFONT font;
	HFONT fontOld;
	HBITMAP bitmap;
	HBITMAP bitmapOld;
	int maxWidthMeasure;
	int maxLenText;

	int codePage;

	void BrushColor(ColourDesired back);
	void SetFont(Font &font_);
	void Clear();

public:
	SurfaceGDI();
	// Deleted so SurfaceGDI objects can not be copied.
	SurfaceGDI(const SurfaceGDI &) = delete;
	SurfaceGDI(SurfaceGDI &&) = delete;
	SurfaceGDI &operator=(const SurfaceGDI &) = delete;
	SurfaceGDI &operator=(SurfaceGDI &&) = delete;

	~SurfaceGDI() override;

	void Init(WindowID wid) override;
	void Init(SurfaceID sid, WindowID wid) override;
	void InitPixMap(int width, int height, Surface *surface_, WindowID wid) override;

	void Release() override;
	bool Initialised() override;
	void PenColour(ColourDesired fore) override;
	int LogPixelsY() override;
	int DeviceHeightFont(int points) override;
	void MoveTo(int x_, int y_) override;
	void LineTo(int x_, int y_) override;
	void Polygon(Point *pts, int npts, ColourDesired fore, ColourDesired back) override;
	void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) override;
	void FillRectangle(PRectangle rc, ColourDesired back) override;
	void FillRectangle(PRectangle rc, Surface &surfacePattern) override;
	void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) override;
	void AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
		ColourDesired outline, int alphaOutline, int flags) override;
	void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) override;
	void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) override;
	void Copy(PRectangle rc, Point from, Surface &surfaceSource) override;

	void DrawTextCommon(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, UINT fuOptions);
	void DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore, ColourDesired back) override;
	void DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore, ColourDesired back) override;
	void DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore) override;
	void MeasureWidths(Font &font_, const char *s, int len, XYPOSITION *positions) override;
	XYPOSITION WidthText(Font &font_, const char *s, int len) override;
	XYPOSITION WidthChar(Font &font_, char ch) override;
	XYPOSITION Ascent(Font &font_) override;
	XYPOSITION Descent(Font &font_) override;
	XYPOSITION InternalLeading(Font &font_) override;
	XYPOSITION Height(Font &font_) override;
	XYPOSITION AverageCharWidth(Font &font_) override;

	void SetClip(PRectangle rc) override;
	void FlushCachedState() override;

	void SetUnicodeMode(bool unicodeMode_) override;
	void SetDBCSMode(int codePage_) override;
};

SurfaceGDI::SurfaceGDI() :
	unicodeMode(false),
	hdc(0), 	hdcOwned(false),
	pen(0), 	penOld(0),
	brush(0), brushOld(0),
	font(0), 	fontOld(0),
	bitmap(0), bitmapOld(0) {
	maxWidthMeasure = INT_MAX;
	// There appears to be a 16 bit string length limit in GDI on NT.
	maxLenText = 65535;

	codePage = 0;
}

SurfaceGDI::~SurfaceGDI() {
	Clear();
}

void SurfaceGDI::Clear() {
	if (penOld) {
		::SelectObject(hdc, penOld);
		::DeleteObject(pen);
		penOld = 0;
	}
	pen = 0;
	if (brushOld) {
		::SelectObject(hdc, brushOld);
		::DeleteObject(brush);
		brushOld = 0;
	}
	brush = 0;
	if (fontOld) {
		// Fonts are not deleted as they are owned by a Font object
		::SelectObject(hdc, fontOld);
		fontOld = 0;
	}
	font = 0;
	if (bitmapOld) {
		::SelectObject(hdc, bitmapOld);
		::DeleteObject(bitmap);
		bitmapOld = 0;
	}
	bitmap = 0;
	if (hdcOwned) {
		::DeleteDC(hdc);
		hdc = 0;
		hdcOwned = false;
	}
}

void SurfaceGDI::Release() {
	Clear();
}

bool SurfaceGDI::Initialised() {
	return hdc != 0;
}

void SurfaceGDI::Init(WindowID) {
	Release();
	hdc = ::CreateCompatibleDC(NULL);
	hdcOwned = true;
	::SetTextAlign(hdc, TA_BASELINE);
}

void SurfaceGDI::Init(SurfaceID sid, WindowID) {
	Release();
	hdc = static_cast<HDC>(sid);
	::SetTextAlign(hdc, TA_BASELINE);
}

void SurfaceGDI::InitPixMap(int width, int height, Surface *surface_, WindowID) {
	Release();
	SurfaceGDI *psurfOther = static_cast<SurfaceGDI *>(surface_);
	hdc = ::CreateCompatibleDC(psurfOther->hdc);
	hdcOwned = true;
	bitmap = ::CreateCompatibleBitmap(psurfOther->hdc, width, height);
	bitmapOld = static_cast<HBITMAP>(::SelectObject(hdc, bitmap));
	::SetTextAlign(hdc, TA_BASELINE);
	SetUnicodeMode(psurfOther->unicodeMode);
	SetDBCSMode(psurfOther->codePage);
}

void SurfaceGDI::PenColour(ColourDesired fore) {
	if (pen) {
		::SelectObject(hdc, penOld);
		::DeleteObject(pen);
		pen = 0;
		penOld = 0;
	}
	pen = ::CreatePen(0,1,fore.AsLong());
	penOld = static_cast<HPEN>(::SelectObject(hdc, pen));
}

void SurfaceGDI::BrushColor(ColourDesired back) {
	if (brush) {
		::SelectObject(hdc, brushOld);
		::DeleteObject(brush);
		brush = 0;
		brushOld = 0;
	}
	// Only ever want pure, non-dithered brushes
	const ColourDesired colourNearest = ColourDesired(::GetNearestColor(hdc, back.AsLong()));
	brush = ::CreateSolidBrush(colourNearest.AsLong());
	brushOld = SelectBrush(hdc, brush);
}
void SurfaceGDI::SetFont(Font &font_) {
	if (font_.GetID() != font) {
		const FormatAndMetrics *pfm = FamFromFontID(font_.GetID());
		PLATFORM_ASSERT(pfm->technology == SCWIN_TECH_GDI);
		if (fontOld) {
			::SelectObject(hdc, pfm->hfont);
		} else {
			fontOld = static_cast<HFONT>(::SelectObject(hdc, pfm->hfont));
		}
		font = pfm->hfont;
	}
}

int SurfaceGDI::LogPixelsY() {
	return ::GetDeviceCaps(hdc, LOGPIXELSY);
}

int SurfaceGDI::DeviceHeightFont(int points) {
	return ::MulDiv(points, LogPixelsY(), 72);
}

void SurfaceGDI::MoveTo(int x_, int y_) {
	::MoveToEx(hdc, x_, y_, 0);
}

void SurfaceGDI::LineTo(int x_, int y_) {
	::LineTo(hdc, x_, y_);
}

void SurfaceGDI::Polygon(Point *pts, int npts, ColourDesired fore, ColourDesired back) {
	PenColour(fore);
	BrushColor(back);
	std::vector<POINT> outline;
	for (int i=0; i<npts; i++) {
		POINT pt = {static_cast<LONG>(pts[i].x), static_cast<LONG>(pts[i].y)};
		outline.push_back(pt);
	}
	::Polygon(hdc, &outline[0], npts);
}

void SurfaceGDI::RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) {
	PenColour(fore);
	BrushColor(back);
	const RECT rcw = RectFromPRectangle(rc);
	::Rectangle(hdc, rcw.left, rcw.top, rcw.right, rcw.bottom);
}

void SurfaceGDI::FillRectangle(PRectangle rc, ColourDesired back) {
	// Using ExtTextOut rather than a FillRect ensures that no dithering occurs.
	// There is no need to allocate a brush either.
	const RECT rcw = RectFromPRectangle(rc);
	::SetBkColor(hdc, back.AsLong());
	::ExtTextOut(hdc, rcw.left, rcw.top, ETO_OPAQUE, &rcw, TEXT(""), 0, NULL);
}

void SurfaceGDI::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	HBRUSH br;
	if (static_cast<SurfaceGDI &>(surfacePattern).bitmap)
		br = ::CreatePatternBrush(static_cast<SurfaceGDI &>(surfacePattern).bitmap);
	else	// Something is wrong so display in red
		br = ::CreateSolidBrush(RGB(0xff, 0, 0));
	const RECT rcw = RectFromPRectangle(rc);
	::FillRect(hdc, &rcw, br);
	::DeleteObject(br);
}

void SurfaceGDI::RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) {
	PenColour(fore);
	BrushColor(back);
	const RECT rcw = RectFromPRectangle(rc);
	::RoundRect(hdc,
		rcw.left + 1, rcw.top,
		rcw.right - 1, rcw.bottom,
		8, 8);
}

namespace {

// Plot a point into a DWORD buffer symmetrically to all 4 quadrants
void AllFour(DWORD *pixels, int width, int height, int x, int y, DWORD val) {
	pixels[y*width+x] = val;
	pixels[y*width+width-1-x] = val;
	pixels[(height-1-y)*width+x] = val;
	pixels[(height-1-y)*width+width-1-x] = val;
}

DWORD dwordFromBGRA(byte b, byte g, byte r, byte a) {
	union {
		byte pixVal[4];
		DWORD val;
	} converter;
	converter.pixVal[0] = b;
	converter.pixVal[1] = g;
	converter.pixVal[2] = r;
	converter.pixVal[3] = a;
	return converter.val;
}

DWORD dwordMultiplied(ColourDesired colour, unsigned int alpha) {
	return dwordFromBGRA(
		static_cast<byte>(colour.GetBlue() * alpha / 255),
		static_cast<byte>(colour.GetGreen() * alpha / 255),
		static_cast<byte>(colour.GetRed() * alpha / 255),
		static_cast<byte>(alpha));
}

}

void SurfaceGDI::AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
		ColourDesired outline, int alphaOutline, int /* flags*/ ) {
	const RECT rcw = RectFromPRectangle(rc);
	if (rc.Width() > 0) {
		HDC hMemDC = ::CreateCompatibleDC(hdc);
		const int width = rcw.right - rcw.left;
		const int height = rcw.bottom - rcw.top;
		// Ensure not distorted too much by corners when small
		cornerSize = std::min(cornerSize, (std::min(width, height) / 2) - 2);
		const BITMAPINFO bpih = {{sizeof(BITMAPINFOHEADER), width, height, 1, 32, BI_RGB, 0, 0, 0, 0, 0},
			{{0, 0, 0, 0}}};
		void *image = nullptr;
		HBITMAP hbmMem = CreateDIBSection(hMemDC, &bpih,
			DIB_RGB_COLORS, &image, NULL, 0);

		if (hbmMem) {
			HBITMAP hbmOld = SelectBitmap(hMemDC, hbmMem);

			const DWORD valEmpty = dwordFromBGRA(0,0,0,0);
			const DWORD valFill = dwordMultiplied(fill, alphaFill);
			const DWORD valOutline = dwordMultiplied(outline, alphaOutline);

			DWORD *pixels = static_cast<DWORD *>(image);
			for (int y=0; y<height; y++) {
				for (int x=0; x<width; x++) {
					if ((x==0) || (x==width-1) || (y == 0) || (y == height-1)) {
						pixels[y*width+x] = valOutline;
					} else {
						pixels[y*width+x] = valFill;
					}
				}
			}
			for (int c=0; c<cornerSize; c++) {
				for (int x=0; x<c+1; x++) {
					AllFour(pixels, width, height, x, c-x, valEmpty);
				}
			}
			for (int x=1; x<cornerSize; x++) {
				AllFour(pixels, width, height, x, cornerSize-x, valOutline);
			}

			const BLENDFUNCTION merge = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

			AlphaBlend(hdc, rcw.left, rcw.top, width, height, hMemDC, 0, 0, width, height, merge);

			SelectBitmap(hMemDC, hbmOld);
			::DeleteObject(hbmMem);
		}
		::DeleteDC(hMemDC);
	} else {
		BrushColor(outline);
		FrameRect(hdc, &rcw, brush);
	}
}

void SurfaceGDI::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) {
	if (rc.Width() > 0) {
		HDC hMemDC = ::CreateCompatibleDC(hdc);
		if (rc.Width() > width)
			rc.left += floor((rc.Width() - width) / 2);
		rc.right = rc.left + width;
		if (rc.Height() > height)
			rc.top += floor((rc.Height() - height) / 2);
		rc.bottom = rc.top + height;

		const BITMAPINFO bpih = {{sizeof(BITMAPINFOHEADER), width, height, 1, 32, BI_RGB, 0, 0, 0, 0, 0},
			{{0, 0, 0, 0}}};
		void *image = nullptr;
		HBITMAP hbmMem = CreateDIBSection(hMemDC, &bpih,
			DIB_RGB_COLORS, &image, NULL, 0);
		if (hbmMem) {
			HBITMAP hbmOld = SelectBitmap(hMemDC, hbmMem);

			for (int y=height-1; y>=0; y--) {
				for (int x=0; x<width; x++) {
					unsigned char *pixel = static_cast<unsigned char *>(image) + (y*width+x) * 4;
					const unsigned char alpha = pixelsImage[3];
					// Input is RGBA, output is BGRA with premultiplied alpha
					pixel[2] = static_cast<unsigned char>((*pixelsImage++) * alpha / 255);
					pixel[1] = static_cast<unsigned char>((*pixelsImage++) * alpha / 255);
					pixel[0] = static_cast<unsigned char>((*pixelsImage++) * alpha / 255);
					pixel[3] = *pixelsImage++;
				}
			}

			const BLENDFUNCTION merge = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

			AlphaBlend(hdc, static_cast<int>(rc.left), static_cast<int>(rc.top),
				static_cast<int>(rc.Width()), static_cast<int>(rc.Height()), hMemDC, 0, 0, width, height, merge);

			SelectBitmap(hMemDC, hbmOld);
			::DeleteObject(hbmMem);
		}
		::DeleteDC(hMemDC);

	}
}

void SurfaceGDI::Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) {
	PenColour(fore);
	BrushColor(back);
	const RECT rcw = RectFromPRectangle(rc);
	::Ellipse(hdc, rcw.left, rcw.top, rcw.right, rcw.bottom);
}

void SurfaceGDI::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
	::BitBlt(hdc,
		static_cast<int>(rc.left), static_cast<int>(rc.top),
		static_cast<int>(rc.Width()), static_cast<int>(rc.Height()),
		static_cast<SurfaceGDI &>(surfaceSource).hdc,
		static_cast<int>(from.x), static_cast<int>(from.y), SRCCOPY);
}

typedef VarBuffer<int, stackBufferLength> TextPositionsI;

void SurfaceGDI::DrawTextCommon(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, UINT fuOptions) {
	SetFont(font_);
	const RECT rcw = RectFromPRectangle(rc);
	const int x = static_cast<int>(rc.left);
	const int yBaseInt = static_cast<int>(ybase);

	if (unicodeMode) {
		const TextWide tbuf(s, len, unicodeMode, codePage);
		::ExtTextOutW(hdc, x, yBaseInt, fuOptions, &rcw, tbuf.buffer, tbuf.tlen, NULL);
	} else {
		::ExtTextOutA(hdc, x, yBaseInt, fuOptions, &rcw, s, len, NULL);
	}
}

void SurfaceGDI::DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourDesired fore, ColourDesired back) {
	::SetTextColor(hdc, fore.AsLong());
	::SetBkColor(hdc, back.AsLong());
	DrawTextCommon(rc, font_, ybase, s, len, ETO_OPAQUE);
}

void SurfaceGDI::DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourDesired fore, ColourDesired back) {
	::SetTextColor(hdc, fore.AsLong());
	::SetBkColor(hdc, back.AsLong());
	DrawTextCommon(rc, font_, ybase, s, len, ETO_OPAQUE | ETO_CLIPPED);
}

void SurfaceGDI::DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourDesired fore) {
	// Avoid drawing spaces in transparent mode
	for (int i=0; i<len; i++) {
		if (s[i] != ' ') {
			::SetTextColor(hdc, fore.AsLong());
			::SetBkMode(hdc, TRANSPARENT);
			DrawTextCommon(rc, font_, ybase, s, len, 0);
			::SetBkMode(hdc, OPAQUE);
			return;
		}
	}
}

XYPOSITION SurfaceGDI::WidthText(Font &font_, const char *s, int len) {
	SetFont(font_);
	SIZE sz={0,0};
	if (!unicodeMode) {
		::GetTextExtentPoint32A(hdc, s, std::min(len, maxLenText), &sz);
	} else {
		const TextWide tbuf(s, len, unicodeMode, codePage);
		::GetTextExtentPoint32W(hdc, tbuf.buffer, tbuf.tlen, &sz);
	}
	return static_cast<XYPOSITION>(sz.cx);
}

void SurfaceGDI::MeasureWidths(Font &font_, const char *s, int len, XYPOSITION *positions) {
	// Zero positions to avoid random behaviour on failure.
	std::fill(positions, positions + len, 0.0f);
	SetFont(font_);
	SIZE sz={0,0};
	int fit = 0;
	int i = 0;
	if (unicodeMode) {
		const TextWide tbuf(s, len, unicodeMode, codePage);
		TextPositionsI poses(tbuf.tlen);
		if (!::GetTextExtentExPointW(hdc, tbuf.buffer, tbuf.tlen, maxWidthMeasure, &fit, poses.buffer, &sz)) {
			// Failure
			return;
		}
		// Map the widths given for UTF-16 characters back onto the UTF-8 input string
		for (int ui = 0; ui < fit; ui++) {
			const unsigned char uch = s[i];
			const unsigned int byteCount = UTF8BytesOfLead[uch];
			if (byteCount == 4) {	// Non-BMP
				ui++;
			}
			for (unsigned int bytePos=0; (bytePos<byteCount) && (i<len); bytePos++) {
				positions[i++] = static_cast<XYPOSITION>(poses.buffer[ui]);
			}
		}
	} else {
		TextPositionsI poses(len);
		if (!::GetTextExtentExPointA(hdc, s, len, maxWidthMeasure, &fit, poses.buffer, &sz)) {
			// Eeek - a NULL DC or other foolishness could cause this.
			return;
		}
		while (i<fit) {
			positions[i] = static_cast<XYPOSITION>(poses.buffer[i]);
			i++;
		}
	}
	// If any positions not filled in then use the last position for them
	const XYPOSITION lastPos = (fit > 0) ? positions[fit - 1] : 0.0f;
	std::fill(positions+i, positions + len, lastPos);
}

XYPOSITION SurfaceGDI::WidthChar(Font &font_, char ch) {
	SetFont(font_);
	SIZE sz;
	::GetTextExtentPoint32A(hdc, &ch, 1, &sz);
	return static_cast<XYPOSITION>(sz.cx);
}

XYPOSITION SurfaceGDI::Ascent(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return static_cast<XYPOSITION>(tm.tmAscent);
}

XYPOSITION SurfaceGDI::Descent(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return static_cast<XYPOSITION>(tm.tmDescent);
}

XYPOSITION SurfaceGDI::InternalLeading(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return static_cast<XYPOSITION>(tm.tmInternalLeading);
}

XYPOSITION SurfaceGDI::Height(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return static_cast<XYPOSITION>(tm.tmHeight);
}

XYPOSITION SurfaceGDI::AverageCharWidth(Font &font_) {
	SetFont(font_);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	return static_cast<XYPOSITION>(tm.tmAveCharWidth);
}

void SurfaceGDI::SetClip(PRectangle rc) {
	::IntersectClipRect(hdc, static_cast<int>(rc.left), static_cast<int>(rc.top),
		static_cast<int>(rc.right), static_cast<int>(rc.bottom));
}

void SurfaceGDI::FlushCachedState() {
	pen = 0;
	brush = 0;
	font = 0;
}

void SurfaceGDI::SetUnicodeMode(bool unicodeMode_) {
	unicodeMode=unicodeMode_;
}

void SurfaceGDI::SetDBCSMode(int codePage_) {
	// No action on window as automatically handled by system.
	codePage = codePage_;
}

#if defined(USE_D2D)

class SurfaceD2D : public Surface {
	bool unicodeMode;
	int x, y;

	int codePage;
	int codePageText;

	ID2D1RenderTarget *pRenderTarget;
	bool ownRenderTarget;
	int clipsActive;

	IDWriteTextFormat *pTextFormat;
	FLOAT yAscent;
	FLOAT yDescent;
	FLOAT yInternalLeading;

	ID2D1SolidColorBrush *pBrush;

	int logPixelsY;
	float dpiScaleX;
	float dpiScaleY;

	void Clear();
	void SetFont(Font &font_);

public:
	SurfaceD2D();
	// Deleted so SurfaceD2D objects can not be copied.
	SurfaceD2D(const SurfaceD2D &) = delete;
	SurfaceD2D(SurfaceD2D &&) = delete;
	SurfaceD2D &operator=(const SurfaceD2D &) = delete;
	SurfaceD2D &operator=(SurfaceD2D &&) = delete;
	virtual ~SurfaceD2D() override;

	void SetScale();
	void Init(WindowID wid) override;
	void Init(SurfaceID sid, WindowID wid) override;
	void InitPixMap(int width, int height, Surface *surface_, WindowID wid) override;

	void Release() override;
	bool Initialised() override;

	HRESULT FlushDrawing();

	void PenColour(ColourDesired fore) override;
	void D2DPenColour(ColourDesired fore, int alpha=255);
	int LogPixelsY() override;
	int DeviceHeightFont(int points) override;
	void MoveTo(int x_, int y_) override;
	void LineTo(int x_, int y_) override;
	void Polygon(Point *pts, int npts, ColourDesired fore, ColourDesired back) override;
	void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) override;
	void FillRectangle(PRectangle rc, ColourDesired back) override;
	void FillRectangle(PRectangle rc, Surface &surfacePattern) override;
	void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) override;
	void AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
		ColourDesired outline, int alphaOutline, int flags) override;
	void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) override;
	void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) override;
	void Copy(PRectangle rc, Point from, Surface &surfaceSource) override;

	void DrawTextCommon(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, UINT fuOptions);
	void DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore, ColourDesired back) override;
	void DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore, ColourDesired back) override;
	void DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore) override;
	void MeasureWidths(Font &font_, const char *s, int len, XYPOSITION *positions) override;
	XYPOSITION WidthText(Font &font_, const char *s, int len) override;
	XYPOSITION WidthChar(Font &font_, char ch) override;
	XYPOSITION Ascent(Font &font_) override;
	XYPOSITION Descent(Font &font_) override;
	XYPOSITION InternalLeading(Font &font_) override;
	XYPOSITION Height(Font &font_) override;
	XYPOSITION AverageCharWidth(Font &font_) override;

	void SetClip(PRectangle rc) override;
	void FlushCachedState() override;

	void SetUnicodeMode(bool unicodeMode_) override;
	void SetDBCSMode(int codePage_) override;
};

SurfaceD2D::SurfaceD2D() :
	unicodeMode(false),
	x(0), y(0) {

	codePage = 0;
	codePageText = 0;

	pRenderTarget = NULL;
	ownRenderTarget = false;
	clipsActive = 0;

	// From selected font
	pTextFormat = NULL;
	yAscent = 2;
	yDescent = 1;
	yInternalLeading = 0;

	pBrush = NULL;

	logPixelsY = 72;
	dpiScaleX = 1.0;
	dpiScaleY = 1.0;
}

SurfaceD2D::~SurfaceD2D() {
	Clear();
}

void SurfaceD2D::Clear() {
	if (pBrush) {
		pBrush->Release();
		pBrush = 0;
	}
	if (pRenderTarget) {
		while (clipsActive) {
			pRenderTarget->PopAxisAlignedClip();
			clipsActive--;
		}
		if (ownRenderTarget) {
			pRenderTarget->Release();
		}
		pRenderTarget = 0;
	}
}

void SurfaceD2D::Release() {
	Clear();
}

void SurfaceD2D::SetScale() {
	HDC hdcMeasure = ::CreateCompatibleDC(NULL);
	logPixelsY = ::GetDeviceCaps(hdcMeasure, LOGPIXELSY);
	dpiScaleX = ::GetDeviceCaps(hdcMeasure, LOGPIXELSX) / 96.0f;
	dpiScaleY = logPixelsY / 96.0f;
	::DeleteDC(hdcMeasure);
}

bool SurfaceD2D::Initialised() {
	return pRenderTarget != 0;
}

HRESULT SurfaceD2D::FlushDrawing() {
	return pRenderTarget->Flush();
}

void SurfaceD2D::Init(WindowID /* wid */) {
	Release();
	SetScale();
}

void SurfaceD2D::Init(SurfaceID sid, WindowID) {
	Release();
	SetScale();
	pRenderTarget = static_cast<ID2D1RenderTarget *>(sid);
}

void SurfaceD2D::InitPixMap(int width, int height, Surface *surface_, WindowID) {
	Release();
	SetScale();
	SurfaceD2D *psurfOther = static_cast<SurfaceD2D *>(surface_);
	ID2D1BitmapRenderTarget *pCompatibleRenderTarget = NULL;
	const D2D1_SIZE_F desiredSize = D2D1::SizeF(static_cast<float>(width), static_cast<float>(height));
	D2D1_PIXEL_FORMAT desiredFormat;
#ifdef __MINGW32__
	desiredFormat.format = DXGI_FORMAT_UNKNOWN;
#else
	desiredFormat = psurfOther->pRenderTarget->GetPixelFormat();
#endif
	desiredFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	const HRESULT hr = psurfOther->pRenderTarget->CreateCompatibleRenderTarget(
		&desiredSize, NULL, &desiredFormat, D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE, &pCompatibleRenderTarget);
	if (SUCCEEDED(hr)) {
		pRenderTarget = pCompatibleRenderTarget;
		pRenderTarget->BeginDraw();
		ownRenderTarget = true;
	}
	SetUnicodeMode(psurfOther->unicodeMode);
	SetDBCSMode(psurfOther->codePage);
}

void SurfaceD2D::PenColour(ColourDesired fore) {
	D2DPenColour(fore);
}

void SurfaceD2D::D2DPenColour(ColourDesired fore, int alpha) {
	if (pRenderTarget) {
		D2D_COLOR_F col;
		col.r = (fore.AsLong() & 0xff) / 255.0f;
		col.g = ((fore.AsLong() & 0xff00) >> 8) / 255.0f;
		col.b = (fore.AsLong() >> 16) / 255.0f;
		col.a = alpha / 255.0f;
		if (pBrush) {
			pBrush->SetColor(col);
		} else {
			const HRESULT hr = pRenderTarget->CreateSolidColorBrush(col, &pBrush);
			if (!SUCCEEDED(hr) && pBrush) {
				pBrush->Release();
				pBrush = 0;
			}
		}
	}
}

void SurfaceD2D::SetFont(Font &font_) {
	const FormatAndMetrics *pfm = FamFromFontID(font_.GetID());
	PLATFORM_ASSERT(pfm->technology == SCWIN_TECH_DIRECTWRITE);
	pTextFormat = pfm->pTextFormat;
	yAscent = pfm->yAscent;
	yDescent = pfm->yDescent;
	yInternalLeading = pfm->yInternalLeading;
	codePageText = codePage;
	if (pfm->characterSet) {
		codePageText = Scintilla::CodePageFromCharSet(pfm->characterSet, codePage);
	}
	if (pRenderTarget) {
		D2D1_TEXT_ANTIALIAS_MODE aaMode;
		aaMode = DWriteMapFontQuality(pfm->extraFontFlag);

		if (aaMode == D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE && customClearTypeRenderingParams)
			pRenderTarget->SetTextRenderingParams(customClearTypeRenderingParams);
		else if (defaultRenderingParams)
			pRenderTarget->SetTextRenderingParams(defaultRenderingParams);

		pRenderTarget->SetTextAntialiasMode(aaMode);
	}
}

int SurfaceD2D::LogPixelsY() {
	return logPixelsY;
}

int SurfaceD2D::DeviceHeightFont(int points) {
	return ::MulDiv(points, LogPixelsY(), 72);
}

void SurfaceD2D::MoveTo(int x_, int y_) {
	x = x_;
	y = y_;
}

static int Delta(int difference) {
	if (difference < 0)
		return -1;
	else if (difference > 0)
		return 1;
	else
		return 0;
}

void SurfaceD2D::LineTo(int x_, int y_) {
	if (pRenderTarget) {
		const int xDiff = x_ - x;
		const int xDelta = Delta(xDiff);
		const int yDiff = y_ - y;
		const int yDelta = Delta(yDiff);
		if ((xDiff == 0) || (yDiff == 0)) {
			// Horizontal or vertical lines can be more precisely drawn as a filled rectangle
			const int xEnd = x_ - xDelta;
			const int left = std::min(x, xEnd);
			const int width = abs(x - xEnd) + 1;
			const int yEnd = y_ - yDelta;
			const int top = std::min(y, yEnd);
			const int height = abs(y - yEnd) + 1;
			const D2D1_RECT_F rectangle1 = D2D1::RectF(static_cast<float>(left), static_cast<float>(top),
				static_cast<float>(left+width), static_cast<float>(top+height));
			pRenderTarget->FillRectangle(&rectangle1, pBrush);
		} else if ((abs(xDiff) == abs(yDiff))) {
			// 45 degree slope
			pRenderTarget->DrawLine(D2D1::Point2F(x + 0.5f, y + 0.5f),
				D2D1::Point2F(x_ + 0.5f - xDelta, y_ + 0.5f - yDelta), pBrush);
		} else {
			// Line has a different slope so difficult to avoid last pixel
			pRenderTarget->DrawLine(D2D1::Point2F(x + 0.5f, y + 0.5f),
				D2D1::Point2F(x_ + 0.5f, y_ + 0.5f), pBrush);
		}
		x = x_;
		y = y_;
	}
}

void SurfaceD2D::Polygon(Point *pts, int npts, ColourDesired fore, ColourDesired back) {
	if (pRenderTarget) {
		ID2D1Factory *pFactory = 0;
		pRenderTarget->GetFactory(&pFactory);
		ID2D1PathGeometry *geometry=0;
		HRESULT hr = pFactory->CreatePathGeometry(&geometry);
		if (SUCCEEDED(hr)) {
			ID2D1GeometrySink *sink = 0;
			hr = geometry->Open(&sink);
			if (SUCCEEDED(hr)) {
				sink->BeginFigure(D2D1::Point2F(pts[0].x + 0.5f, pts[0].y + 0.5f), D2D1_FIGURE_BEGIN_FILLED);
				for (int i=1; i<npts; i++) {
					sink->AddLine(D2D1::Point2F(pts[i].x + 0.5f, pts[i].y + 0.5f));
				}
				sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				sink->Close();
				sink->Release();

				D2DPenColour(back);
				pRenderTarget->FillGeometry(geometry,pBrush);
				D2DPenColour(fore);
				pRenderTarget->DrawGeometry(geometry,pBrush);
			}

			geometry->Release();
		}
	}
}

void SurfaceD2D::RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) {
	if (pRenderTarget) {
		const D2D1_RECT_F rectangle1 = D2D1::RectF(round(rc.left) + 0.5f, rc.top+0.5f, round(rc.right) - 0.5f, rc.bottom-0.5f);
		D2DPenColour(back);
		pRenderTarget->FillRectangle(&rectangle1, pBrush);
		D2DPenColour(fore);
		pRenderTarget->DrawRectangle(&rectangle1, pBrush);
	}
}

void SurfaceD2D::FillRectangle(PRectangle rc, ColourDesired back) {
	if (pRenderTarget) {
		D2DPenColour(back);
        const D2D1_RECT_F rectangle1 = D2D1::RectF(round(rc.left), rc.top, round(rc.right), rc.bottom);
        pRenderTarget->FillRectangle(&rectangle1, pBrush);
	}
}

void SurfaceD2D::FillRectangle(PRectangle rc, Surface &surfacePattern) {
	SurfaceD2D &surfOther = static_cast<SurfaceD2D &>(surfacePattern);
	surfOther.FlushDrawing();
	ID2D1Bitmap *pBitmap = NULL;
	ID2D1BitmapRenderTarget *pCompatibleRenderTarget = reinterpret_cast<ID2D1BitmapRenderTarget *>(
		surfOther.pRenderTarget);
	HRESULT hr = pCompatibleRenderTarget->GetBitmap(&pBitmap);
	if (SUCCEEDED(hr)) {
		ID2D1BitmapBrush *pBitmapBrush = NULL;
		const D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
	        D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP,
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		// Create the bitmap brush.
		hr = pRenderTarget->CreateBitmapBrush(pBitmap, brushProperties, &pBitmapBrush);
		pBitmap->Release();
		if (SUCCEEDED(hr)) {
			pRenderTarget->FillRectangle(
				D2D1::RectF(rc.left, rc.top, rc.right, rc.bottom),
				pBitmapBrush);
			pBitmapBrush->Release();
		}
	}
}

void SurfaceD2D::RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) {
	if (pRenderTarget) {
		D2D1_ROUNDED_RECT roundedRectFill = {
			D2D1::RectF(rc.left+1.0f, rc.top+1.0f, rc.right-1.0f, rc.bottom-1.0f),
			4, 4};
		D2DPenColour(back);
		pRenderTarget->FillRoundedRectangle(roundedRectFill, pBrush);

		D2D1_ROUNDED_RECT roundedRect = {
			D2D1::RectF(rc.left + 0.5f, rc.top+0.5f, rc.right - 0.5f, rc.bottom-0.5f),
			4, 4};
		D2DPenColour(fore);
		pRenderTarget->DrawRoundedRectangle(roundedRect, pBrush);
	}
}

void SurfaceD2D::AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
		ColourDesired outline, int alphaOutline, int /* flags*/ ) {
	if (pRenderTarget) {
		if (cornerSize == 0) {
			// When corner size is zero, draw square rectangle to prevent blurry pixels at corners
			const D2D1_RECT_F rectFill = D2D1::RectF(round(rc.left) + 1.0f, rc.top + 1.0f, round(rc.right) - 1.0f, rc.bottom - 1.0f);
			D2DPenColour(fill, alphaFill);
			pRenderTarget->FillRectangle(rectFill, pBrush);

			const D2D1_RECT_F rectOutline = D2D1::RectF(round(rc.left) + 0.5f, rc.top + 0.5f, round(rc.right) - 0.5f, rc.bottom - 0.5f);
			D2DPenColour(outline, alphaOutline);
			pRenderTarget->DrawRectangle(rectOutline, pBrush);
		} else {
			const float cornerSizeF = static_cast<float>(cornerSize);
			D2D1_ROUNDED_RECT roundedRectFill = {
				D2D1::RectF(round(rc.left) + 1.0f, rc.top + 1.0f, round(rc.right) - 1.0f, rc.bottom - 1.0f),
				cornerSizeF, cornerSizeF};
			D2DPenColour(fill, alphaFill);
			pRenderTarget->FillRoundedRectangle(roundedRectFill, pBrush);

			D2D1_ROUNDED_RECT roundedRect = {
				D2D1::RectF(round(rc.left) + 0.5f, rc.top + 0.5f, round(rc.right) - 0.5f, rc.bottom - 0.5f),
				cornerSizeF, cornerSizeF};
			D2DPenColour(outline, alphaOutline);
			pRenderTarget->DrawRoundedRectangle(roundedRect, pBrush);
		}
	}
}

void SurfaceD2D::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) {
	if (pRenderTarget) {
		if (rc.Width() > width)
			rc.left += floor((rc.Width() - width) / 2);
		rc.right = rc.left + width;
		if (rc.Height() > height)
			rc.top += floor((rc.Height() - height) / 2);
		rc.bottom = rc.top + height;

		std::vector<unsigned char> image(height * width * 4);
		for (int yPixel=0; yPixel<height; yPixel++) {
			for (int xPixel = 0; xPixel<width; xPixel++) {
				unsigned char *pixel = &image[0] + (yPixel*width + xPixel) * 4;
				const unsigned char alpha = pixelsImage[3];
				// Input is RGBA, output is BGRA with premultiplied alpha
				pixel[2] = (*pixelsImage++) * alpha / 255;
				pixel[1] = (*pixelsImage++) * alpha / 255;
				pixel[0] = (*pixelsImage++) * alpha / 255;
				pixel[3] = *pixelsImage++;
			}
		}

		ID2D1Bitmap *bitmap = 0;
		const D2D1_SIZE_U size = D2D1::SizeU(width, height);
		D2D1_BITMAP_PROPERTIES props = {{DXGI_FORMAT_B8G8R8A8_UNORM,
		    D2D1_ALPHA_MODE_PREMULTIPLIED}, 72.0, 72.0};
		const HRESULT hr = pRenderTarget->CreateBitmap(size, &image[0],
                  width * 4, &props, &bitmap);
		if (SUCCEEDED(hr)) {
			D2D1_RECT_F rcDestination = {rc.left, rc.top, rc.right, rc.bottom};
			pRenderTarget->DrawBitmap(bitmap, rcDestination);
			bitmap->Release();
		}
	}
}

void SurfaceD2D::Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) {
	if (pRenderTarget) {
		const FLOAT radius = rc.Width() / 2.0f;
		D2D1_ELLIPSE ellipse = {
			D2D1::Point2F((rc.left + rc.right) / 2.0f, (rc.top + rc.bottom) / 2.0f),
			radius,radius};

		PenColour(back);
		pRenderTarget->FillEllipse(ellipse, pBrush);
		PenColour(fore);
		pRenderTarget->DrawEllipse(ellipse, pBrush);
	}
}

void SurfaceD2D::Copy(PRectangle rc, Point from, Surface &surfaceSource) {
	SurfaceD2D &surfOther = static_cast<SurfaceD2D &>(surfaceSource);
	surfOther.FlushDrawing();
	ID2D1BitmapRenderTarget *pCompatibleRenderTarget = reinterpret_cast<ID2D1BitmapRenderTarget *>(
		surfOther.pRenderTarget);
	ID2D1Bitmap *pBitmap = NULL;
	HRESULT hr = pCompatibleRenderTarget->GetBitmap(&pBitmap);
	if (SUCCEEDED(hr)) {
		D2D1_RECT_F rcDestination = {rc.left, rc.top, rc.right, rc.bottom};
		D2D1_RECT_F rcSource = {from.x, from.y, from.x + rc.Width(), from.y + rc.Height()};
		pRenderTarget->DrawBitmap(pBitmap, rcDestination, 1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, rcSource);
		hr = pRenderTarget->Flush();
		if (FAILED(hr)) {
			Platform::DebugPrintf("Failed Flush 0x%x\n", hr);
		}
		pBitmap->Release();
	}
}

void SurfaceD2D::DrawTextCommon(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, UINT fuOptions) {
	SetFont(font_);

	// Use Unicode calls
	const TextWide tbuf(s, len, unicodeMode, codePageText);
	if (pRenderTarget && pTextFormat && pBrush) {
		if (fuOptions & ETO_CLIPPED) {
			D2D1_RECT_F rcClip = {rc.left, rc.top, rc.right, rc.bottom};
			pRenderTarget->PushAxisAlignedClip(rcClip, D2D1_ANTIALIAS_MODE_ALIASED);
		}

		// Explicitly creating a text layout appears a little faster
		IDWriteTextLayout *pTextLayout;
		const HRESULT hr = pIDWriteFactory->CreateTextLayout(tbuf.buffer, tbuf.tlen, pTextFormat,
				rc.Width(), rc.Height(), &pTextLayout);
		if (SUCCEEDED(hr)) {
			D2D1_POINT_2F origin = {rc.left, ybase-yAscent};
			pRenderTarget->DrawTextLayout(origin, pTextLayout, pBrush, D2D1_DRAW_TEXT_OPTIONS_NONE);
			pTextLayout->Release();
		}

		if (fuOptions & ETO_CLIPPED) {
			pRenderTarget->PopAxisAlignedClip();
		}
	}
}

void SurfaceD2D::DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourDesired fore, ColourDesired back) {
	if (pRenderTarget) {
		FillRectangle(rc, back);
		D2DPenColour(fore);
		DrawTextCommon(rc, font_, ybase, s, len, ETO_OPAQUE);
	}
}

void SurfaceD2D::DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourDesired fore, ColourDesired back) {
	if (pRenderTarget) {
		FillRectangle(rc, back);
		D2DPenColour(fore);
		DrawTextCommon(rc, font_, ybase, s, len, ETO_OPAQUE | ETO_CLIPPED);
	}
}

void SurfaceD2D::DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len,
	ColourDesired fore) {
	// Avoid drawing spaces in transparent mode
	for (int i=0; i<len; i++) {
		if (s[i] != ' ') {
			if (pRenderTarget) {
				D2DPenColour(fore);
				DrawTextCommon(rc, font_, ybase, s, len, 0);
			}
			return;
		}
	}
}

XYPOSITION SurfaceD2D::WidthText(Font &font_, const char *s, int len) {
	FLOAT width = 1.0;
	SetFont(font_);
	const TextWide tbuf(s, len, unicodeMode, codePageText);
	if (pIDWriteFactory && pTextFormat) {
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		const HRESULT hr = pIDWriteFactory->CreateTextLayout(tbuf.buffer, tbuf.tlen, pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			DWRITE_TEXT_METRICS textMetrics;
			if (SUCCEEDED(pTextLayout->GetMetrics(&textMetrics)))
				width = textMetrics.widthIncludingTrailingWhitespace;
			pTextLayout->Release();
		}
	}
	return width;
}

void SurfaceD2D::MeasureWidths(Font &font_, const char *s, int len, XYPOSITION *positions) {
	SetFont(font_);
	if (!pIDWriteFactory || !pTextFormat) {
		// SetFont failed or no access to DirectWrite so give up.
		return;
	}
	const TextWide tbuf(s, len, unicodeMode, codePageText);
	TextPositions poses(tbuf.tlen);
	// Initialize poses for safety.
	std::fill(poses.buffer, poses.buffer + tbuf.tlen, 0.0f);
	// Create a layout
	IDWriteTextLayout *pTextLayout = 0;
	const HRESULT hrCreate = pIDWriteFactory->CreateTextLayout(tbuf.buffer, tbuf.tlen, pTextFormat, 10000.0, 1000.0, &pTextLayout);
	if (!SUCCEEDED(hrCreate)) {
		return;
	}
	const int clusters = 1000;
	DWRITE_CLUSTER_METRICS clusterMetrics[clusters];
	UINT32 count = 0;
	const HRESULT hrGetCluster = pTextLayout->GetClusterMetrics(clusterMetrics, clusters, &count);
	pTextLayout->Release();
	if (!SUCCEEDED(hrGetCluster)) {
		return;
	}
	// A cluster may be more than one WCHAR, such as for "ffi" which is a ligature in the Candara font
	FLOAT position = 0.0f;
	size_t ti=0;
	for (size_t ci=0; ci<count; ci++) {
		for (size_t inCluster=0; inCluster<clusterMetrics[ci].length; inCluster++) {
			poses.buffer[ti++] = position + clusterMetrics[ci].width * (inCluster + 1) / clusterMetrics[ci].length;
		}
		position += clusterMetrics[ci].width;
	}
	PLATFORM_ASSERT(ti == static_cast<size_t>(tbuf.tlen));
	if (unicodeMode) {
		// Map the widths given for UTF-16 characters back onto the UTF-8 input string
		int ui=0;
		int i=0;
		while (ui<tbuf.tlen) {
			const unsigned char uch = s[i];
			const unsigned int byteCount = UTF8BytesOfLead[uch];
			if (byteCount == 4) {	// Non-BMP
				ui++;
			}
			for (unsigned int bytePos=0; (bytePos<byteCount) && (i<len); bytePos++) {
				positions[i++] = poses.buffer[ui];
			}
			ui++;
		}
		XYPOSITION lastPos = 0.0f;
		if (i > 0)
			lastPos = positions[i-1];
		while (i<len) {
			positions[i++] = lastPos;
		}
	} else if (codePageText == 0) {

		// One char per position
		PLATFORM_ASSERT(len == tbuf.tlen);
		for (int kk=0; kk<tbuf.tlen; kk++) {
			positions[kk] = poses.buffer[kk];
		}

	} else {

		// May be one or two bytes per position
		int ui = 0;
		for (int i=0; i<len && ui<tbuf.tlen;) {
			positions[i] = poses.buffer[ui];
			if (DBCSIsLeadByte(codePageText, s[i])) {
				positions[i+1] = poses.buffer[ui];
				i += 2;
			} else {
				i++;
			}

			ui++;
		}
	}
}

XYPOSITION SurfaceD2D::WidthChar(Font &font_, char ch) {
	FLOAT width = 1.0;
	SetFont(font_);
	if (pIDWriteFactory && pTextFormat) {
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		const WCHAR wch = ch;
		const HRESULT hr = pIDWriteFactory->CreateTextLayout(&wch, 1, pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			DWRITE_TEXT_METRICS textMetrics;
			if (SUCCEEDED(pTextLayout->GetMetrics(&textMetrics)))
				width = textMetrics.widthIncludingTrailingWhitespace;
			pTextLayout->Release();
		}
	}
	return width;
}

XYPOSITION SurfaceD2D::Ascent(Font &font_) {
	SetFont(font_);
	return ceil(yAscent);
}

XYPOSITION SurfaceD2D::Descent(Font &font_) {
	SetFont(font_);
	return ceil(yDescent);
}

XYPOSITION SurfaceD2D::InternalLeading(Font &font_) {
	SetFont(font_);
	return floor(yInternalLeading);
}

XYPOSITION SurfaceD2D::Height(Font &font_) {
	return Ascent(font_) + Descent(font_);
}

XYPOSITION SurfaceD2D::AverageCharWidth(Font &font_) {
	FLOAT width = 1.0;
	SetFont(font_);
	if (pIDWriteFactory && pTextFormat) {
		// Create a layout
		IDWriteTextLayout *pTextLayout = 0;
		const WCHAR wszAllAlpha[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		const size_t lenAllAlpha = wcslen(wszAllAlpha);
		const HRESULT hr = pIDWriteFactory->CreateTextLayout(wszAllAlpha, static_cast<UINT32>(lenAllAlpha),
			pTextFormat, 1000.0, 1000.0, &pTextLayout);
		if (SUCCEEDED(hr)) {
			DWRITE_TEXT_METRICS textMetrics;
			if (SUCCEEDED(pTextLayout->GetMetrics(&textMetrics)))
				width = textMetrics.width / lenAllAlpha;
			pTextLayout->Release();
		}
	}
	return width;
}

void SurfaceD2D::SetClip(PRectangle rc) {
	if (pRenderTarget) {
		D2D1_RECT_F rcClip = {rc.left, rc.top, rc.right, rc.bottom};
		pRenderTarget->PushAxisAlignedClip(rcClip, D2D1_ANTIALIAS_MODE_ALIASED);
		clipsActive++;
	}
}

void SurfaceD2D::FlushCachedState() {
}

void SurfaceD2D::SetUnicodeMode(bool unicodeMode_) {
	unicodeMode=unicodeMode_;
}

void SurfaceD2D::SetDBCSMode(int codePage_) {
	// No action on window as automatically handled by system.
	codePage = codePage_;
}
#endif

Surface *Surface::Allocate(int technology) {
#if defined(USE_D2D)
	if (technology == SCWIN_TECH_GDI)
		return new SurfaceGDI;
	else
		return new SurfaceD2D;
#else
	return new SurfaceGDI;
#endif
}

namespace {

HWND HwndFromWindowID(WindowID wid) {
	return static_cast<HWND>(wid);
}

}

Window::~Window() {
}

void Window::Destroy() {
	if (wid)
		::DestroyWindow(HwndFromWindowID(wid));
	wid = nullptr;
}

PRectangle Window::GetPosition() {
	RECT rc;
	::GetWindowRect(HwndFromWindowID(wid), &rc);
	return PRectangle::FromInts(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::SetPosition(PRectangle rc) {
	::SetWindowPos(HwndFromWindowID(wid),
		0, static_cast<int>(rc.left), static_cast<int>(rc.top),
		static_cast<int>(rc.Width()), static_cast<int>(rc.Height()), SWP_NOZORDER | SWP_NOACTIVATE);
}

namespace {

static RECT RectFromMonitor(HMONITOR hMonitor) {
	MONITORINFO mi = {};
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(hMonitor, &mi)) {
		return mi.rcWork;
	}
	RECT rc = {0, 0, 0, 0};
	if (::SystemParametersInfoA(SPI_GETWORKAREA, 0, &rc, 0) == 0) {
		rc.left = 0;
		rc.top = 0;
		rc.right = 0;
		rc.bottom = 0;
	}
	return rc;
}

}

void Window::SetPositionRelative(PRectangle rc, Window relativeTo) {
	const LONG style = ::GetWindowLong(HwndFromWindowID(wid), GWL_STYLE);
	if (style & WS_POPUP) {
		POINT ptOther = {0, 0};
		::ClientToScreen(HwndFromWindowID(relativeTo.GetID()), &ptOther);
		rc.Move(static_cast<XYPOSITION>(ptOther.x), static_cast<XYPOSITION>(ptOther.y));

		const RECT rcMonitor = RectFromPRectangle(rc);

		HMONITOR hMonitor = MonitorFromRect(&rcMonitor, MONITOR_DEFAULTTONEAREST);
		// If hMonitor is NULL, that's just the main screen anyways.
		const RECT rcWork = RectFromMonitor(hMonitor);

		if (rcWork.left < rcWork.right) {
			// Now clamp our desired rectangle to fit inside the work area
			// This way, the menu will fit wholly on one screen. An improvement even
			// if you don't have a second monitor on the left... Menu's appears half on
			// one screen and half on the other are just U.G.L.Y.!
			if (rc.right > rcWork.right)
				rc.Move(rcWork.right - rc.right, 0);
			if (rc.bottom > rcWork.bottom)
				rc.Move(0, rcWork.bottom - rc.bottom);
			if (rc.left < rcWork.left)
				rc.Move(rcWork.left - rc.left, 0);
			if (rc.top < rcWork.top)
				rc.Move(0, rcWork.top - rc.top);
		}
	}
	SetPosition(rc);
}

PRectangle Window::GetClientPosition() {
	RECT rc={0,0,0,0};
	if (wid)
		::GetClientRect(HwndFromWindowID(wid), &rc);
	return PRectangle::FromInts(rc.left, rc.top, rc.right, rc.bottom);
}

void Window::Show(bool show) {
	if (show)
		::ShowWindow(HwndFromWindowID(wid), SW_SHOWNOACTIVATE);
	else
		::ShowWindow(HwndFromWindowID(wid), SW_HIDE);
}

void Window::InvalidateAll() {
	::InvalidateRect(HwndFromWindowID(wid), NULL, FALSE);
}

void Window::InvalidateRectangle(PRectangle rc) {
	const RECT rcw = RectFromPRectangle(rc);
	::InvalidateRect(HwndFromWindowID(wid), &rcw, FALSE);
}

void Window::SetFont(Font &font) {
	::SendMessage(HwndFromWindowID(wid), WM_SETFONT,
		reinterpret_cast<WPARAM>(font.GetID()), 0);
}

namespace {

void FlipBitmap(HBITMAP bitmap, int width, int height) {
	HDC hdc = ::CreateCompatibleDC(NULL);
	if (hdc != NULL) {
		HGDIOBJ prevBmp = ::SelectObject(hdc, bitmap);
		::StretchBlt(hdc, width - 1, 0, -width, height, hdc, 0, 0, width, height, SRCCOPY);
		::SelectObject(hdc, prevBmp);
		::DeleteDC(hdc);
	}
}

HCURSOR GetReverseArrowCursor() {
	if (reverseArrowCursor != NULL)
		return reverseArrowCursor;

	::EnterCriticalSection(&crPlatformLock);
	HCURSOR cursor = reverseArrowCursor;
	if (cursor == NULL) {
		cursor = ::LoadCursor(NULL, IDC_ARROW);
		ICONINFO info;
		if (::GetIconInfo(cursor, &info)) {
			BITMAP bmp;
			if (::GetObject(info.hbmMask, sizeof(bmp), &bmp)) {
				FlipBitmap(info.hbmMask, bmp.bmWidth, bmp.bmHeight);
				if (info.hbmColor != NULL)
					FlipBitmap(info.hbmColor, bmp.bmWidth, bmp.bmHeight);
				info.xHotspot = bmp.bmWidth - 1 - info.xHotspot;

				reverseArrowCursor = ::CreateIconIndirect(&info);
				if (reverseArrowCursor != NULL)
					cursor = reverseArrowCursor;
			}

			::DeleteObject(info.hbmMask);
			if (info.hbmColor != NULL)
				::DeleteObject(info.hbmColor);
		}
	}
	::LeaveCriticalSection(&crPlatformLock);
	return cursor;
}

}

void Window::SetCursor(Cursor curs) {
	switch (curs) {
	case cursorText:
		::SetCursor(::LoadCursor(NULL,IDC_IBEAM));
		break;
	case cursorUp:
		::SetCursor(::LoadCursor(NULL,IDC_UPARROW));
		break;
	case cursorWait:
		::SetCursor(::LoadCursor(NULL,IDC_WAIT));
		break;
	case cursorHoriz:
		::SetCursor(::LoadCursor(NULL,IDC_SIZEWE));
		break;
	case cursorVert:
		::SetCursor(::LoadCursor(NULL,IDC_SIZENS));
		break;
	case cursorHand:
		::SetCursor(::LoadCursor(NULL,IDC_HAND));
		break;
	case cursorReverseArrow:
		::SetCursor(GetReverseArrowCursor());
		break;
	case cursorArrow:
	case cursorInvalid:	// Should not occur, but just in case.
		::SetCursor(::LoadCursor(NULL,IDC_ARROW));
		break;
	}
}

/* Returns rectangle of monitor pt is on, both rect and pt are in Window's
   coordinates */
PRectangle Window::GetMonitorRect(Point pt) {
	const PRectangle rcPosition = GetPosition();
	POINT ptDesktop = {static_cast<LONG>(pt.x + rcPosition.left),
		static_cast<LONG>(pt.y + rcPosition.top)};
	HMONITOR hMonitor = MonitorFromPoint(ptDesktop, MONITOR_DEFAULTTONEAREST);

	const RECT rcWork = RectFromMonitor(hMonitor);
	if (rcWork.left < rcWork.right) {
		PRectangle rcMonitor(
			rcWork.left - rcPosition.left,
			rcWork.top - rcPosition.top,
			rcWork.right - rcPosition.left,
			rcWork.bottom - rcPosition.top);
		return rcMonitor;
	} else {
		return PRectangle();
	}
}

struct ListItemData {
	const char *text;
	int pixId;
};

class LineToItem {
	std::vector<char> words;

	std::vector<ListItemData> data;

public:
	void Clear() {
		words.clear();
		data.clear();
	}

	ListItemData Get(size_t index) const {
		if (index < data.size()) {
			return data[index];
		} else {
			ListItemData missing = {"", -1};
			return missing;
		}
	}
	int Count() const {
		return static_cast<int>(data.size());
	}

	void AllocItem(const char *text, int pixId) {
		ListItemData lid = { text, pixId };
		data.push_back(lid);
	}

	char *SetWords(const char *s) {
		words = std::vector<char>(s, s+strlen(s)+1);
		return &words[0];
	}
};

const TCHAR ListBoxX_ClassName[] = TEXT("ListBoxX");

ListBox::ListBox() {
}

ListBox::~ListBox() {
}

class ListBoxX : public ListBox {
	int lineHeight;
	FontID fontCopy;
	int technology;
	RGBAImageSet images;
	LineToItem lti;
	HWND lb;
	bool unicodeMode;
	int desiredVisibleRows;
	unsigned int maxItemCharacters;
	unsigned int aveCharWidth;
	Window *parent;
	int ctrlID;
	IListBoxDelegate *delegate;
	const char *widestItem;
	unsigned int maxCharWidth;
	WPARAM resizeHit;
	PRectangle rcPreSize;
	Point dragOffset;
	Point location;	// Caret location at which the list is opened
	int wheelDelta; // mouse wheel residue

	HWND GetHWND() const;
	void AppendListItem(const char *text, const char *numword);
	static void AdjustWindowRect(PRectangle *rc);
	int ItemHeight() const;
	int MinClientWidth() const;
	int TextOffset() const;
	POINT GetClientExtent() const;
	POINT MinTrackSize() const;
	POINT MaxTrackSize() const;
	void SetRedraw(bool on);
	void OnDoubleClick();
	void OnSelChange();
	void ResizeToCursor();
	void StartResize(WPARAM);
	LRESULT NcHitTest(WPARAM, LPARAM) const;
	void CentreItem(int n);
	void Paint(HDC);
	static LRESULT PASCAL ControlWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	static const Point ItemInset;	// Padding around whole item
	static const Point TextInset;	// Padding around text
	static const Point ImageInset;	// Padding around image

public:
	ListBoxX() : lineHeight(10), fontCopy(0), technology(0), lb(0), unicodeMode(false),
		desiredVisibleRows(9), maxItemCharacters(0), aveCharWidth(8),
		parent(NULL), ctrlID(0),
		delegate(nullptr),
		widestItem(NULL), maxCharWidth(1), resizeHit(0), wheelDelta(0) {
	}
	~ListBoxX() override {
		if (fontCopy) {
			::DeleteObject(fontCopy);
			fontCopy = 0;
		}
	}
	void SetFont(Font &font) override;
	void Create(Window &parent_, int ctrlID_, Point location_, int lineHeight_, bool unicodeMode_, int technology_) override;
	void SetAverageCharWidth(int width) override;
	void SetVisibleRows(int rows) override;
	int GetVisibleRows() const override;
	PRectangle GetDesiredRect() override;
	int CaretFromEdge() override;
	void Clear() override;
	void Append(char *s, int type = -1) override;
	int Length() override;
	void Select(int n) override;
	int GetSelection() override;
	int Find(const char *prefix) override;
	void GetValue(int n, char *value, int len) override;
	void RegisterImage(int type, const char *xpm_data) override;
	void RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage) override;
	void ClearRegisteredImages() override;
	void SetDelegate(IListBoxDelegate *lbDelegate) override;
	void SetList(const char *list, char separator, char typesep) override;
	void Draw(DRAWITEMSTRUCT *pDrawItem);
	LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static LRESULT PASCAL StaticWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
};

const Point ListBoxX::ItemInset(0, 0);
const Point ListBoxX::TextInset(2, 0);
const Point ListBoxX::ImageInset(1, 0);

ListBox *ListBox::Allocate() {
	ListBoxX *lb = new ListBoxX();
	return lb;
}

void ListBoxX::Create(Window &parent_, int ctrlID_, Point location_, int lineHeight_, bool unicodeMode_, int technology_) {
	parent = &parent_;
	ctrlID = ctrlID_;
	location = location_;
	lineHeight = lineHeight_;
	unicodeMode = unicodeMode_;
	technology = technology_;
	HWND hwndParent = HwndFromWindowID(parent->GetID());
	HINSTANCE hinstanceParent = GetWindowInstance(hwndParent);
	// Window created as popup so not clipped within parent client area
	wid = ::CreateWindowEx(
		WS_EX_WINDOWEDGE, ListBoxX_ClassName, TEXT(""),
		WS_POPUP | WS_THICKFRAME,
		100,100, 150,80, hwndParent,
		NULL,
		hinstanceParent,
		this);

	POINT locationw = {static_cast<LONG>(location.x), static_cast<LONG>(location.y)};
	::MapWindowPoints(hwndParent, NULL, &locationw, 1);
	location = Point::FromInts(locationw.x, locationw.y);
}

void ListBoxX::SetFont(Font &font) {
	if (font.GetID()) {
		if (fontCopy) {
			::DeleteObject(fontCopy);
			fontCopy = 0;
		}
		FormatAndMetrics *pfm = static_cast<FormatAndMetrics *>(font.GetID());
		fontCopy = pfm->HFont();
		::SendMessage(lb, WM_SETFONT, reinterpret_cast<WPARAM>(fontCopy), 0);
	}
}

void ListBoxX::SetAverageCharWidth(int width) {
	aveCharWidth = width;
}

void ListBoxX::SetVisibleRows(int rows) {
	desiredVisibleRows = rows;
}

int ListBoxX::GetVisibleRows() const {
	return desiredVisibleRows;
}

HWND ListBoxX::GetHWND() const {
	return HwndFromWindowID(GetID());
}

PRectangle ListBoxX::GetDesiredRect() {
	PRectangle rcDesired = GetPosition();

	int rows = Length();
	if ((rows == 0) || (rows > desiredVisibleRows))
		rows = desiredVisibleRows;
	rcDesired.bottom = rcDesired.top + ItemHeight() * rows;

	int width = MinClientWidth();
	HDC hdc = ::GetDC(lb);
	HFONT oldFont = SelectFont(hdc, fontCopy);
	SIZE textSize = {0, 0};
	int len = 0;
	if (widestItem) {
		len = static_cast<int>(strlen(widestItem));
		if (unicodeMode) {
			const TextWide tbuf(widestItem, len, unicodeMode);
			::GetTextExtentPoint32W(hdc, tbuf.buffer, tbuf.tlen, &textSize);
		} else {
			::GetTextExtentPoint32A(hdc, widestItem, len, &textSize);
		}
	}
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	maxCharWidth = tm.tmMaxCharWidth;
	SelectFont(hdc, oldFont);
	::ReleaseDC(lb, hdc);

	const int widthDesired = std::max(textSize.cx, (len + 1) * tm.tmAveCharWidth);
	if (width < widthDesired)
		width = widthDesired;

	rcDesired.right = rcDesired.left + TextOffset() + width + (TextInset.x * 2);
	if (Length() > rows)
		rcDesired.right += ::GetSystemMetrics(SM_CXVSCROLL);

	AdjustWindowRect(&rcDesired);
	return rcDesired;
}

int ListBoxX::TextOffset() const {
	const int pixWidth = images.GetWidth();
	return static_cast<int>(pixWidth == 0 ? ItemInset.x : ItemInset.x + pixWidth + (ImageInset.x * 2));
}

int ListBoxX::CaretFromEdge() {
	PRectangle rc;
	AdjustWindowRect(&rc);
	return TextOffset() + static_cast<int>(TextInset.x + (0 - rc.left) - 1);
}

void ListBoxX::Clear() {
	::SendMessage(lb, LB_RESETCONTENT, 0, 0);
	maxItemCharacters = 0;
	widestItem = NULL;
	lti.Clear();
}

void ListBoxX::Append(char *, int) {
	// This method is no longer called in Scintilla
	PLATFORM_ASSERT(false);
}

int ListBoxX::Length() {
	return lti.Count();
}

void ListBoxX::Select(int n) {
	// We are going to scroll to centre on the new selection and then select it, so disable
	// redraw to avoid flicker caused by a painting new selection twice in unselected and then
	// selected states
	SetRedraw(false);
	CentreItem(n);
	::SendMessage(lb, LB_SETCURSEL, n, 0);
	OnSelChange();
	SetRedraw(true);
}

int ListBoxX::GetSelection() {
	return static_cast<int>(::SendMessage(lb, LB_GETCURSEL, 0, 0));
}

// This is not actually called at present
int ListBoxX::Find(const char *) {
	return LB_ERR;
}

void ListBoxX::GetValue(int n, char *value, int len) {
	const ListItemData item = lti.Get(n);
	strncpy(value, item.text, len);
	value[len-1] = '\0';
}

void ListBoxX::RegisterImage(int type, const char *xpm_data) {
	XPM xpmImage(xpm_data);
	images.Add(type, new RGBAImage(xpmImage));
}

void ListBoxX::RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage) {
	images.Add(type, new RGBAImage(width, height, 1.0, pixelsImage));
}

void ListBoxX::ClearRegisteredImages() {
	images.Clear();
}

void ListBoxX::Draw(DRAWITEMSTRUCT *pDrawItem) {
	if ((pDrawItem->itemAction == ODA_SELECT) || (pDrawItem->itemAction == ODA_DRAWENTIRE)) {
		RECT rcBox = pDrawItem->rcItem;
		rcBox.left += TextOffset();
		if (pDrawItem->itemState & ODS_SELECTED) {
			RECT rcImage = pDrawItem->rcItem;
			rcImage.right = rcBox.left;
			// The image is not highlighted
			::FillRect(pDrawItem->hDC, &rcImage, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
			::FillRect(pDrawItem->hDC, &rcBox, reinterpret_cast<HBRUSH>(COLOR_HIGHLIGHT+1));
			::SetBkColor(pDrawItem->hDC, ::GetSysColor(COLOR_HIGHLIGHT));
			::SetTextColor(pDrawItem->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
		} else {
			::FillRect(pDrawItem->hDC, &pDrawItem->rcItem, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
			::SetBkColor(pDrawItem->hDC, ::GetSysColor(COLOR_WINDOW));
			::SetTextColor(pDrawItem->hDC, ::GetSysColor(COLOR_WINDOWTEXT));
		}

		const ListItemData item = lti.Get(pDrawItem->itemID);
		const int pixId = item.pixId;
		const char *text = item.text;
		const int len = static_cast<int>(strlen(text));

		RECT rcText = rcBox;
		::InsetRect(&rcText, static_cast<int>(TextInset.x), static_cast<int>(TextInset.y));

		if (unicodeMode) {
			const TextWide tbuf(text, len, unicodeMode);
			::DrawTextW(pDrawItem->hDC, tbuf.buffer, tbuf.tlen, &rcText, DT_NOPREFIX|DT_END_ELLIPSIS|DT_SINGLELINE|DT_NOCLIP);
		} else {
			::DrawTextA(pDrawItem->hDC, text, len, &rcText, DT_NOPREFIX|DT_END_ELLIPSIS|DT_SINGLELINE|DT_NOCLIP);
		}

		// Draw the image, if any
		const RGBAImage *pimage = images.Get(pixId);
		if (pimage) {
			std::unique_ptr<Surface> surfaceItem(Surface::Allocate(technology));
			if (technology == SCWIN_TECH_GDI) {
				surfaceItem->Init(pDrawItem->hDC, pDrawItem->hwndItem);
				const long left = pDrawItem->rcItem.left + static_cast<int>(ItemInset.x + ImageInset.x);
				const PRectangle rcImage = PRectangle::FromInts(left, pDrawItem->rcItem.top,
					left + images.GetWidth(), pDrawItem->rcItem.bottom);
				surfaceItem->DrawRGBAImage(rcImage,
					pimage->GetWidth(), pimage->GetHeight(), pimage->Pixels());
				::SetTextAlign(pDrawItem->hDC, TA_TOP);
			} else {
#if defined(USE_D2D)
				const D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
					D2D1_RENDER_TARGET_TYPE_DEFAULT,
					D2D1::PixelFormat(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						D2D1_ALPHA_MODE_IGNORE),
					0,
					0,
					D2D1_RENDER_TARGET_USAGE_NONE,
					D2D1_FEATURE_LEVEL_DEFAULT
					);
				ID2D1DCRenderTarget *pDCRT = 0;
				HRESULT hr = pD2DFactory->CreateDCRenderTarget(&props, &pDCRT);
				if (SUCCEEDED(hr)) {
					RECT rcWindow;
					GetClientRect(pDrawItem->hwndItem, &rcWindow);
					hr = pDCRT->BindDC(pDrawItem->hDC, &rcWindow);
					if (SUCCEEDED(hr)) {
						surfaceItem->Init(pDCRT, pDrawItem->hwndItem);
						pDCRT->BeginDraw();
						const long left = pDrawItem->rcItem.left + static_cast<long>(ItemInset.x + ImageInset.x);
						const PRectangle rcImage = PRectangle::FromInts(left, pDrawItem->rcItem.top,
							left + images.GetWidth(), pDrawItem->rcItem.bottom);
						surfaceItem->DrawRGBAImage(rcImage,
							pimage->GetWidth(), pimage->GetHeight(), pimage->Pixels());
						pDCRT->EndDraw();
						pDCRT->Release();
					}
				}
#endif
			}
		}
	}
}

void ListBoxX::AppendListItem(const char *text, const char *numword) {
	int pixId = -1;
	if (numword) {
		pixId = 0;
		char ch;
		while ((ch = *++numword) != '\0') {
			pixId = 10 * pixId + (ch - '0');
		}
	}

	lti.AllocItem(text, pixId);
	const unsigned int len = static_cast<unsigned int>(strlen(text));
	if (maxItemCharacters < len) {
		maxItemCharacters = len;
		widestItem = text;
	}
}

void ListBoxX::SetDelegate(IListBoxDelegate *lbDelegate) {
	delegate = lbDelegate;
}

void ListBoxX::SetList(const char *list, char separator, char typesep) {
	// Turn off redraw while populating the list - this has a significant effect, even if
	// the listbox is not visible.
	SetRedraw(false);
	Clear();
	const size_t size = strlen(list);
	char *words = lti.SetWords(list);
	char *startword = words;
	char *numword = NULL;
	for (size_t i=0; i < size; i++) {
		if (words[i] == separator) {
			words[i] = '\0';
			if (numword)
				*numword = '\0';
			AppendListItem(startword, numword);
			startword = words + i + 1;
			numword = NULL;
		} else if (words[i] == typesep) {
			numword = words + i;
		}
	}
	if (startword) {
		if (numword)
			*numword = '\0';
		AppendListItem(startword, numword);
	}

	// Finally populate the listbox itself with the correct number of items
	const int count = lti.Count();
	::SendMessage(lb, LB_INITSTORAGE, count, 0);
	for (int j=0; j<count; j++) {
		::SendMessage(lb, LB_ADDSTRING, 0, j+1);
	}
	SetRedraw(true);
}

void ListBoxX::AdjustWindowRect(PRectangle *rc) {
	RECT rcw = RectFromPRectangle(*rc);
	::AdjustWindowRectEx(&rcw, WS_THICKFRAME, false, WS_EX_WINDOWEDGE);
	*rc = PRectangle::FromInts(rcw.left, rcw.top, rcw.right, rcw.bottom);
}

int ListBoxX::ItemHeight() const {
	int itemHeight = lineHeight + (static_cast<int>(TextInset.y) * 2);
	const int pixHeight = images.GetHeight() + (static_cast<int>(ImageInset.y) * 2);
	if (itemHeight < pixHeight) {
		itemHeight = pixHeight;
	}
	return itemHeight;
}

int ListBoxX::MinClientWidth() const {
	return 12 * (aveCharWidth+aveCharWidth/3);
}

POINT ListBoxX::MinTrackSize() const {
	PRectangle rc = PRectangle::FromInts(0, 0, MinClientWidth(), ItemHeight());
	AdjustWindowRect(&rc);
	POINT ret = {static_cast<LONG>(rc.Width()), static_cast<LONG>(rc.Height())};
	return ret;
}

POINT ListBoxX::MaxTrackSize() const {
	PRectangle rc = PRectangle::FromInts(0, 0,
		std::max(static_cast<unsigned int>(MinClientWidth()),
		maxCharWidth * maxItemCharacters + static_cast<int>(TextInset.x) * 2 +
		 TextOffset() + ::GetSystemMetrics(SM_CXVSCROLL)),
		ItemHeight() * lti.Count());
	AdjustWindowRect(&rc);
	POINT ret = {static_cast<LONG>(rc.Width()), static_cast<LONG>(rc.Height())};
	return ret;
}

void ListBoxX::SetRedraw(bool on) {
	::SendMessage(lb, WM_SETREDRAW, on, 0);
	if (on)
		::InvalidateRect(lb, NULL, TRUE);
}

void ListBoxX::ResizeToCursor() {
	PRectangle rc = GetPosition();
	POINT ptw;
	::GetCursorPos(&ptw);
	Point pt = Point::FromInts(ptw.x, ptw.y);
	pt.x += dragOffset.x;
	pt.y += dragOffset.y;

	switch (resizeHit) {
		case HTLEFT:
			rc.left = pt.x;
			break;
		case HTRIGHT:
			rc.right = pt.x;
			break;
		case HTTOP:
			rc.top = pt.y;
			break;
		case HTTOPLEFT:
			rc.top = pt.y;
			rc.left = pt.x;
			break;
		case HTTOPRIGHT:
			rc.top = pt.y;
			rc.right = pt.x;
			break;
		case HTBOTTOM:
			rc.bottom = pt.y;
			break;
		case HTBOTTOMLEFT:
			rc.bottom = pt.y;
			rc.left = pt.x;
			break;
		case HTBOTTOMRIGHT:
			rc.bottom = pt.y;
			rc.right = pt.x;
			break;
	}

	const POINT ptMin = MinTrackSize();
	const POINT ptMax = MaxTrackSize();
	// We don't allow the left edge to move at present, but just in case
	rc.left = std::clamp(rc.left, rcPreSize.right - ptMax.x, rcPreSize.right - ptMin.x);
	rc.top = std::clamp(rc.top, rcPreSize.bottom - ptMax.y, rcPreSize.bottom - ptMin.y);
	rc.right = std::clamp(rc.right, rcPreSize.left + ptMin.x, rcPreSize.left + ptMax.x);
	rc.bottom = std::clamp(rc.bottom, rcPreSize.top + ptMin.y, rcPreSize.top + ptMax.y);

	SetPosition(rc);
}

void ListBoxX::StartResize(WPARAM hitCode) {
	rcPreSize = GetPosition();
	POINT cursorPos;
	::GetCursorPos(&cursorPos);

	switch (hitCode) {
		case HTRIGHT:
		case HTBOTTOM:
		case HTBOTTOMRIGHT:
			dragOffset.x = rcPreSize.right - cursorPos.x;
			dragOffset.y = rcPreSize.bottom - cursorPos.y;
			break;

		case HTTOPRIGHT:
			dragOffset.x = rcPreSize.right - cursorPos.x;
			dragOffset.y = rcPreSize.top - cursorPos.y;
			break;

		// Note that the current hit test code prevents the left edge cases ever firing
		// as we don't want the left edge to be moveable
		case HTLEFT:
		case HTTOP:
		case HTTOPLEFT:
			dragOffset.x = rcPreSize.left - cursorPos.x;
			dragOffset.y = rcPreSize.top - cursorPos.y;
			break;
		case HTBOTTOMLEFT:
			dragOffset.x = rcPreSize.left - cursorPos.x;
			dragOffset.y = rcPreSize.bottom - cursorPos.y;
			break;

		default:
			return;
	}

	::SetCapture(GetHWND());
	resizeHit = hitCode;
}

LRESULT ListBoxX::NcHitTest(WPARAM wParam, LPARAM lParam) const {
	Window win = *this;	// Copy HWND to avoid const problems
	const PRectangle rc = win.GetPosition();

	LRESULT hit = ::DefWindowProc(GetHWND(), WM_NCHITTEST, wParam, lParam);
	// There is an apparent bug in the DefWindowProc hit test code whereby it will
	// return HTTOPXXX if the window in question is shorter than the default
	// window caption height + frame, even if one is hovering over the bottom edge of
	// the frame, so workaround that here
	if (hit >= HTTOP && hit <= HTTOPRIGHT) {
		const int minHeight = GetSystemMetrics(SM_CYMINTRACK);
		const int yPos = GET_Y_LPARAM(lParam);
		if ((rc.Height() < minHeight) && (yPos > ((rc.top + rc.bottom)/2))) {
			hit += HTBOTTOM - HTTOP;
		}
	}

	// Nerver permit resizing that moves the left edge. Allow movement of top or bottom edge
	// depending on whether the list is above or below the caret
	switch (hit) {
		case HTLEFT:
		case HTTOPLEFT:
		case HTBOTTOMLEFT:
			hit = HTERROR;
			break;

		case HTTOP:
		case HTTOPRIGHT: {
				// Valid only if caret below list
				if (location.y < rc.top)
					hit = HTERROR;
			}
			break;

		case HTBOTTOM:
		case HTBOTTOMRIGHT: {
				// Valid only if caret above list
				if (rc.bottom <= location.y)
					hit = HTERROR;
			}
			break;
	}

	return hit;
}

void ListBoxX::OnDoubleClick() {
	if (delegate) {
		ListBoxEvent event(ListBoxEvent::EventType::doubleClick);
		delegate->ListNotify(&event);
	}
}

void ListBoxX::OnSelChange() {
	if (delegate) {
		ListBoxEvent event(ListBoxEvent::EventType::selectionChange);
		delegate->ListNotify(&event);
	}
}

POINT ListBoxX::GetClientExtent() const {
	RECT rc;
	::GetWindowRect(HwndFromWindowID(wid), &rc);
	POINT ret { rc.right - rc.left, rc.bottom - rc.top };
	return ret;
}

void ListBoxX::CentreItem(int n) {
	// If below mid point, scroll up to centre, but with more items below if uneven
	if (n >= 0) {
		const POINT extent = GetClientExtent();
		const int visible = extent.y/ItemHeight();
		if (visible < Length()) {
			const LRESULT top = ::SendMessage(lb, LB_GETTOPINDEX, 0, 0);
			const int half = (visible - 1) / 2;
			if (n > (top + half))
				::SendMessage(lb, LB_SETTOPINDEX, n - half , 0);
		}
	}
}

// Performs a double-buffered paint operation to avoid flicker
void ListBoxX::Paint(HDC hDC) {
	const POINT extent = GetClientExtent();
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, extent.x, extent.y);
	HDC bitmapDC = ::CreateCompatibleDC(hDC);
	HBITMAP hBitmapOld = SelectBitmap(bitmapDC, hBitmap);
	// The list background is mainly erased during painting, but can be a small
	// unpainted area when at the end of a non-integrally sized list with a
	// vertical scroll bar
	RECT rc = { 0, 0, extent.x, extent.y };
	::FillRect(bitmapDC, &rc, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
	// Paint the entire client area and vertical scrollbar
	::SendMessage(lb, WM_PRINT, reinterpret_cast<WPARAM>(bitmapDC), PRF_CLIENT|PRF_NONCLIENT);
	::BitBlt(hDC, 0, 0, extent.x, extent.y, bitmapDC, 0, 0, SRCCOPY);
	// Select a stock brush to prevent warnings from BoundsChecker
	::SelectObject(bitmapDC, GetStockFont(WHITE_BRUSH));
	SelectBitmap(bitmapDC, hBitmapOld);
	::DeleteDC(bitmapDC);
	::DeleteObject(hBitmap);
}

LRESULT PASCAL ListBoxX::ControlWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	try {
		ListBoxX *lbx = static_cast<ListBoxX *>(PointerFromWindow(::GetParent(hWnd)));
		switch (iMessage) {
		case WM_ERASEBKGND:
			return TRUE;

		case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hDC = ::BeginPaint(hWnd, &ps);
				if (lbx) {
					lbx->Paint(hDC);
				}
				::EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_MOUSEACTIVATE:
			// This prevents the view activating when the scrollbar is clicked
			return MA_NOACTIVATE;

		case WM_LBUTTONDOWN: {
				// We must take control of selection to prevent the ListBox activating
				// the popup
				const LRESULT lResult = ::SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
				const int item = LOWORD(lResult);
				if (HIWORD(lResult) == 0 && item >= 0) {
					::SendMessage(hWnd, LB_SETCURSEL, item, 0);
					if (lbx) {
						lbx->OnSelChange();
					}
				}
			}
			return 0;

		case WM_LBUTTONUP:
			return 0;

		case WM_LBUTTONDBLCLK: {
				if (lbx) {
					lbx->OnDoubleClick();
				}
			}
			return 0;

		case WM_MBUTTONDOWN:
			// disable the scroll wheel button click action
			return 0;
		}

		WNDPROC prevWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (prevWndProc) {
			return ::CallWindowProc(prevWndProc, hWnd, iMessage, wParam, lParam);
		} else {
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	} catch (...) {
	}
	return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
}

LRESULT ListBoxX::WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_CREATE: {
			HINSTANCE hinstanceParent = GetWindowInstance(HwndFromWindowID(parent->GetID()));
			// Note that LBS_NOINTEGRALHEIGHT is specified to fix cosmetic issue when resizing the list
			// but has useful side effect of speeding up list population significantly
			lb = ::CreateWindowEx(
				0, TEXT("listbox"), TEXT(""),
				WS_CHILD | WS_VSCROLL | WS_VISIBLE |
				LBS_OWNERDRAWFIXED | LBS_NODATA | LBS_NOINTEGRALHEIGHT,
				0, 0, 150,80, hWnd,
				reinterpret_cast<HMENU>(static_cast<ptrdiff_t>(ctrlID)),
				hinstanceParent,
				0);
			WNDPROC prevWndProc = SubclassWindow(lb, ControlWndProc);
			::SetWindowLongPtr(lb, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(prevWndProc));
		}
		break;

	case WM_SIZE:
		if (lb) {
			SetRedraw(false);
			::SetWindowPos(lb, 0, 0,0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
			// Ensure the selection remains visible
			CentreItem(GetSelection());
			SetRedraw(true);
		}
		break;

	case WM_PAINT: {
			PAINTSTRUCT ps;
			::BeginPaint(hWnd, &ps);
			::EndPaint(hWnd, &ps);
		}
		break;

	case WM_COMMAND:
		// This is not actually needed now - the registered double click action is used
		// directly to action a choice from the list.
		::SendMessage(HwndFromWindowID(parent->GetID()), iMessage, wParam, lParam);
		break;

	case WM_MEASUREITEM: {
			MEASUREITEMSTRUCT *pMeasureItem = reinterpret_cast<MEASUREITEMSTRUCT *>(lParam);
			pMeasureItem->itemHeight = ItemHeight();
		}
		break;

	case WM_DRAWITEM:
		Draw(reinterpret_cast<DRAWITEMSTRUCT *>(lParam));
		break;

	case WM_DESTROY:
		lb = 0;
		::SetWindowLong(hWnd, 0, 0);
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);

	case WM_ERASEBKGND:
		// To reduce flicker we can elide background erasure since this window is
		// completely covered by its child.
		return TRUE;

	case WM_GETMINMAXINFO: {
			MINMAXINFO *minMax = reinterpret_cast<MINMAXINFO*>(lParam);
			minMax->ptMaxTrackSize = MaxTrackSize();
			minMax->ptMinTrackSize = MinTrackSize();
		}
		break;

	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;

	case WM_NCHITTEST:
		return NcHitTest(wParam, lParam);

	case WM_NCLBUTTONDOWN:
		// We have to implement our own window resizing because the DefWindowProc
		// implementation insists on activating the resized window
		StartResize(wParam);
		return 0;

	case WM_MOUSEMOVE: {
			if (resizeHit == 0) {
				return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
			} else {
				ResizeToCursor();
			}
		}
		break;

	case WM_LBUTTONUP:
	case WM_CANCELMODE:
		if (resizeHit != 0) {
			resizeHit = 0;
			::ReleaseCapture();
		}
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	case WM_MOUSEWHEEL:
		wheelDelta -= GET_WHEEL_DELTA_WPARAM(wParam);
		if (abs(wheelDelta) >= WHEEL_DELTA) {
			const int nRows = GetVisibleRows();
			int linesToScroll = 1;
			if (nRows > 1) {
				linesToScroll = nRows - 1;
			}
			if (linesToScroll > 3) {
				linesToScroll = 3;
			}
			linesToScroll *= (wheelDelta / WHEEL_DELTA);
			LRESULT top = ::SendMessage(lb, LB_GETTOPINDEX, 0, 0) + linesToScroll;
			if (top < 0) {
				top = 0;
			}
			::SendMessage(lb, LB_SETTOPINDEX, top, 0);
			// update wheel delta residue
			if (wheelDelta >= 0)
				wheelDelta = wheelDelta % WHEEL_DELTA;
			else
				wheelDelta = - (-wheelDelta % WHEEL_DELTA);
		}
		break;

	default:
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	}

	return 0;
}

LRESULT PASCAL ListBoxX::StaticWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	if (iMessage == WM_CREATE) {
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
		SetWindowPointer(hWnd, pCreate->lpCreateParams);
	}
	// Find C++ object associated with window.
	ListBoxX *lbx = static_cast<ListBoxX *>(PointerFromWindow(hWnd));
	if (lbx) {
		return lbx->WndProc(hWnd, iMessage, wParam, lParam);
	} else {
		return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
}

namespace {

bool ListBoxX_Register() {
	WNDCLASSEX wndclassc;
	wndclassc.cbSize = sizeof(wndclassc);
	// We need CS_HREDRAW and CS_VREDRAW because of the ellipsis that might be drawn for
	// truncated items in the list and the appearance/disappearance of the vertical scroll bar.
	// The list repaint is double-buffered to avoid the flicker this would otherwise cause.
	wndclassc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclassc.cbClsExtra = 0;
	wndclassc.cbWndExtra = sizeof(ListBoxX *);
	wndclassc.hInstance = hinstPlatformRes;
	wndclassc.hIcon = NULL;
	wndclassc.hbrBackground = NULL;
	wndclassc.lpszMenuName = NULL;
	wndclassc.lpfnWndProc = ListBoxX::StaticWndProc;
	wndclassc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wndclassc.lpszClassName = ListBoxX_ClassName;
	wndclassc.hIconSm = 0;

	return ::RegisterClassEx(&wndclassc) != 0;
}

bool ListBoxX_Unregister() {
	return ::UnregisterClass(ListBoxX_ClassName, hinstPlatformRes) != 0;
}

}

Menu::Menu() : mid(0) {
}

void Menu::CreatePopUp() {
	Destroy();
	mid = ::CreatePopupMenu();
}

void Menu::Destroy() {
	if (mid)
		::DestroyMenu(static_cast<HMENU>(mid));
	mid = 0;
}

void Menu::Show(Point pt, Window &w) {
	::TrackPopupMenu(static_cast<HMENU>(mid),
		TPM_RIGHTBUTTON, static_cast<int>(pt.x - 4), static_cast<int>(pt.y), 0,
		HwndFromWindowID(w.GetID()), NULL);
	Destroy();
}

class DynamicLibraryImpl : public DynamicLibrary {
protected:
	HMODULE h;
public:
	explicit DynamicLibraryImpl(const char *modulePath) {
		h = ::LoadLibraryA(modulePath);
	}

	~DynamicLibraryImpl() override {
		if (h != NULL)
			::FreeLibrary(h);
	}

	// Use GetProcAddress to get a pointer to the relevant function.
	Function FindFunction(const char *name) override {
		if (h != NULL) {
			// C++ standard doesn't like casts between function pointers and void pointers so use a union
			union {
				FARPROC fp;
				Function f;
			} fnConv;
			fnConv.fp = ::GetProcAddress(h, name);
			return fnConv.f;
		} else {
			return NULL;
		}
	}

	bool IsValid() override {
		return h != NULL;
	}
};

DynamicLibrary *DynamicLibrary::Load(const char *modulePath) {
	return static_cast<DynamicLibrary *>(new DynamicLibraryImpl(modulePath));
}

ColourDesired Platform::Chrome() {
	return ColourDesired(::GetSysColor(COLOR_3DFACE));
}

ColourDesired Platform::ChromeHighlight() {
	return ColourDesired(::GetSysColor(COLOR_3DHIGHLIGHT));
}

const char *Platform::DefaultFont() {
	return "Verdana";
}

int Platform::DefaultFontSize() {
	return 8;
}

unsigned int Platform::DoubleClickTime() {
	return ::GetDoubleClickTime();
}

void Platform::DebugDisplay(const char *s) {
	::OutputDebugStringA(s);
}

//#define TRACE

#ifdef TRACE
void Platform::DebugPrintf(const char *format, ...) {
	char buffer[2000];
	va_list pArguments;
	va_start(pArguments, format);
	vsprintf(buffer,format,pArguments);
	va_end(pArguments);
	Platform::DebugDisplay(buffer);
}
#else
void Platform::DebugPrintf(const char *, ...) {
}
#endif

static bool assertionPopUps = true;

bool Platform::ShowAssertionPopUps(bool assertionPopUps_) {
	const bool ret = assertionPopUps;
	assertionPopUps = assertionPopUps_;
	return ret;
}

void Platform::Assert(const char *c, const char *file, int line) {
	char buffer[2000];
	sprintf(buffer, "Assertion [%s] failed at %s %d%s", c, file, line, assertionPopUps ? "" : "\r\n");
	if (assertionPopUps) {
		const int idButton = ::MessageBoxA(0, buffer, "Assertion failure",
			MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);
		if (idButton == IDRETRY) {
			::DebugBreak();
		} else if (idButton == IDIGNORE) {
			// all OK
		} else {
			abort();
		}
	} else {
		Platform::DebugDisplay(buffer);
		::DebugBreak();
		abort();
	}
}

void Platform_Initialise(void *hInstance) {
	::InitializeCriticalSection(&crPlatformLock);
	hinstPlatformRes = static_cast<HINSTANCE>(hInstance);
	ListBoxX_Register();
}

void Platform_Finalise(bool fromDllMain) {
#if defined(USE_D2D)
	if (!fromDllMain) {
		if (defaultRenderingParams) {
			defaultRenderingParams->Release();
			defaultRenderingParams = 0;
		}
		if (customClearTypeRenderingParams) {
			customClearTypeRenderingParams->Release();
			customClearTypeRenderingParams = 0;
		}
		if (pIDWriteFactory) {
			pIDWriteFactory->Release();
			pIDWriteFactory = 0;
		}
		if (pD2DFactory) {
			pD2DFactory->Release();
			pD2DFactory = 0;
		}
		if (hDLLDWrite) {
			FreeLibrary(hDLLDWrite);
			hDLLDWrite = NULL;
		}
		if (hDLLD2D) {
			FreeLibrary(hDLLD2D);
			hDLLD2D = NULL;
		}
	}
#endif
	if (reverseArrowCursor != NULL)
		::DestroyCursor(reverseArrowCursor);
	ListBoxX_Unregister();
	::DeleteCriticalSection(&crPlatformLock);
}

}
