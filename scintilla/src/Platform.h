// Scintilla source code edit control
/** @file Platform.h
 ** Interface to platform facilities. Also includes some basic utilities.
 ** Implemented in PlatGTK.cxx for GTK+/Linux, PlatWin.cxx for Windows, and PlatWX.cxx for wxWindows.
 **/
// Copyright 1998-2009 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#pragma once

// PLAT_GTK = GTK+ on Linux or Win32
// PLAT_GTK_WIN32 is defined additionally when running PLAT_GTK under Win32
// PLAT_WIN = Win32 API on Win32 OS
// PLAT_WX is wxWindows on any supported platform
// PLAT_TK = Tcl/TK on Linux or Win32

#define PLAT_GTK 0
#define PLAT_GTK_WIN32 0
#define PLAT_GTK_MACOSX 0
#define PLAT_MACOSX 0
#define PLAT_WIN 0
#define PLAT_WX  0
#define PLAT_QT 0
#define PLAT_FOX 0
#define PLAT_CURSES 0
#define PLAT_TK 0
#define PLAT_HAIKU 0

#if defined(FOX)
#undef PLAT_FOX
#define PLAT_FOX 1

#elif defined(__WX__)
#undef PLAT_WX
#define PLAT_WX  1

#elif defined(CURSES)
#undef PLAT_CURSES
#define PLAT_CURSES 1

#elif defined(__HAIKU__)
#undef PLAT_HAIKU
#define PLAT_HAIKU 1

#elif defined(SCINTILLA_QT)
#undef PLAT_QT
#define PLAT_QT 1

#elif defined(TK)
#undef PLAT_TK
#define PLAT_TK 1

#elif defined(GTK)
#undef PLAT_GTK
#define PLAT_GTK 1

#if defined(__WIN32__) || defined(_MSC_VER)
#undef PLAT_GTK_WIN32
#define PLAT_GTK_WIN32 1
#endif

#if defined(__APPLE__)
#undef PLAT_GTK_MACOSX
#define PLAT_GTK_MACOSX 1
#endif

#elif defined(__APPLE__)

#undef PLAT_MACOSX
#define PLAT_MACOSX 1

#else
#undef PLAT_WIN
#define PLAT_WIN 1

#endif

// use __vectorcall to pass float/double arguments such as Point and PRectangle.
#if defined(_WIN64) && defined(NDEBUG)
	#if defined(_MSC_BUILD)
		#define SCICALL __vectorcall
	#elif defined(__INTEL_COMPILER_BUILD_DATE)
		//#define SCICALL __regcall
		#define SCICALL
	#else
		#define SCICALL
	#endif
#else
	#define SCICALL
#endif


// >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
#include <memory>
#include <vector>
#include <string_view>
// <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<

namespace Scintilla {

// official Scintilla use dynamic_cast, which requires RTTI.
#ifdef NDEBUG
#define USE_RTTI	0
#else
#define USE_RTTI	1
#endif

template<typename DerivedPointer, class Base>
inline DerivedPointer down_cast(Base* ptr) noexcept {
#if USE_RTTI
	return dynamic_cast<DerivedPointer>(ptr);
#else
	return static_cast<DerivedPointer>(ptr);
#endif
}

typedef double XYACCUMULATOR;

// Underlying the implementation of the platform classes are platform specific types.
// Sometimes these need to be passed around by client code so they are defined here

typedef void *SurfaceID;
typedef void *WindowID;
typedef void *MenuID;
typedef void *TickerID;
typedef void *Function;
typedef void *IdlerID;


/**
/**
 * Font management.
 */

constexpr const char *localeNameDefault = "en-us";

struct FontParameters {
	const char *faceName;
	XYPOSITION size;
	int weight;
	int stretch;
	bool italic;
	int extraFontFlag;
	int technology;
	int characterSet;
	const char *localeName;

	constexpr FontParameters(
		const char *faceName_,
		XYPOSITION size_=10,
		int weight_=400,
		int stretch_=5,
		bool italic_=false,
		int extraFontFlag_=0,
		int technology_=0,
		int characterSet_=0,
		const char *localeName_=localeNameDefault) noexcept :

		faceName(faceName_),
		size(size_),
		weight(weight_),
		stretch(stretch_),
		italic(italic_),
		extraFontFlag(extraFontFlag_),
		technology(technology_),
		characterSet(characterSet_),
		localeName(localeName_)
	{
	}

};

class Font {
public:
	Font() noexcept = default;
	// Deleted so Font objects can not be copied
	Font(const Font &) = delete;
	Font(Font &&) = delete;
	Font &operator=(const Font &) = delete;
	Font &operator=(Font &&) = delete;
	virtual ~Font() noexcept = default;

	static std::shared_ptr<Font> Allocate(const FontParameters &fp);
};

class IScreenLine {
public:
	virtual std::string_view Text() const noexcept = 0;
	virtual size_t Length() const noexcept = 0;
	virtual size_t RepresentationCount() const = 0;
	virtual XYPOSITION Width() const noexcept = 0;
	virtual XYPOSITION Height() const noexcept = 0;
	virtual XYPOSITION TabWidth() const noexcept = 0;
	virtual XYPOSITION TabWidthMinimumPixels() const noexcept = 0;
	virtual const Font *FontOfPosition(size_t position) const noexcept = 0;
	virtual XYPOSITION RepresentationWidth(size_t position) const noexcept = 0;
	virtual XYPOSITION TabPositionAfter(XYPOSITION xPosition) const noexcept = 0;
};

class IScreenLineLayout {
public:
	virtual ~IScreenLineLayout() = default;
	virtual size_t PositionFromX(XYPOSITION xDistance, bool charPosition) = 0;
	virtual XYPOSITION XFromPosition(size_t caretPosition) noexcept = 0;
	virtual std::vector<Interval> FindRangeIntervals(size_t start, size_t end) = 0;
};

/**
 * Parameters for surfaces.
 */
struct SurfaceMode {
	int codePage = 0;
	bool bidiR2L = false;
	SurfaceMode() = default;
	explicit SurfaceMode(int codePage_, bool bidiR2L_) noexcept : codePage(codePage_), bidiR2L(bidiR2L_) {
	}
};

/**
 * A surface abstracts a place to draw.
 */
class Surface {
public:
	Surface() noexcept = default;
	Surface(const Surface &) = delete;
	Surface(Surface &&) = delete;
	Surface &operator=(const Surface &) = delete;
	Surface &operator=(Surface &&) = delete;
	virtual ~Surface() noexcept = default;
	static std::unique_ptr<Surface> Allocate(int technology);

	virtual void Init(WindowID wid)=0;
	virtual void Init(SurfaceID sid, WindowID wid)=0;
	virtual std::unique_ptr<Surface> AllocatePixMap(int width, int height)=0;

	virtual void SetMode(SurfaceMode mode)=0;

	enum class Ends {
		semiCircles = 0x0,
		leftFlat = 0x1,
		leftAngle = 0x2,
		rightFlat = 0x10,
		rightAngle = 0x20,
	};

	virtual void Release() noexcept = 0;
	virtual int Supports(int feature) noexcept=0;
	virtual bool Initialised() const noexcept = 0;
	virtual void PenColour(ColourDesired fore) = 0;
	virtual int LogPixelsY() const noexcept = 0;
	virtual int DeviceHeightFont(int points) const noexcept = 0;
	virtual void SCICALL MoveTo(int x_, int y_) noexcept = 0;
	virtual void SCICALL LineTo(int x_, int y_) noexcept = 0;
	virtual void SCICALL Polygon(const Point *pts, size_t npts, ColourDesired fore, ColourDesired back) = 0;
	virtual void SCICALL RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) = 0;
	virtual void SCICALL FillRectangle(PRectangle rc, ColourDesired back) = 0;
	virtual void SCICALL FillRectangle(PRectangle rc, Surface &surfacePattern) = 0;
	virtual void SCICALL RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) = 0;
	virtual void SCICALL AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
		ColourDesired outline, int alphaOutline, int flags) = 0;
	enum class GradientOptions {
		leftToRight, topToBottom
	};
	virtual void SCICALL GradientRectangle(PRectangle rc, const std::vector<ColourStop> &stops, GradientOptions options) = 0;
	virtual void SCICALL DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) = 0;
	virtual void SCICALL Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) = 0;
	virtual void SCICALL Copy(PRectangle rc, Point from, Surface &surfaceSource) = 0;

	virtual std::unique_ptr<IScreenLineLayout> Layout(const IScreenLine *screenLine) = 0;

	virtual void SCICALL DrawTextNoClip(PRectangle rc, const Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore, ColourDesired back) = 0;
	virtual void SCICALL DrawTextClipped(PRectangle rc, const Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore, ColourDesired back) = 0;
	virtual void SCICALL DrawTextTransparent(PRectangle rc, const Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore) = 0;
	virtual void SCICALL MeasureWidths(const Font &font_, std::string_view text, XYPOSITION *positions) = 0;
	virtual XYPOSITION WidthText(const Font &font_, std::string_view text) = 0;
	virtual XYPOSITION Ascent(const Font &font_) noexcept = 0;
	virtual XYPOSITION Descent(const Font &font_) noexcept = 0;
	virtual XYPOSITION InternalLeading(const Font &font_) noexcept = 0;
	virtual XYPOSITION Height(const Font &font_) noexcept = 0;
	virtual XYPOSITION AverageCharWidth(const Font &font_) = 0;

	virtual void SCICALL SetClip(PRectangle rc) noexcept = 0;
	virtual void PopClip()=0;
	virtual void FlushCachedState() noexcept = 0;
	virtual void FlushDrawing()=0;

};

/**
 * Class to hide the details of window manipulation.
 * Does not own the window which will normally have a longer life than this object.
 */
class Window {
protected:
	WindowID wid;

public:
	Window() noexcept : wid(nullptr), cursorLast(Cursor::invalid) {}
	Window(const Window &source) = delete;
	Window(Window &&) = delete;
	Window &operator=(WindowID wid_) noexcept {
		wid = wid_;
		cursorLast = Cursor::invalid;
		return *this;
	}
	Window &operator=(const Window &) = delete;
	Window &operator=(Window &&) = delete;
	virtual ~Window();
	WindowID GetID() const noexcept {
		return wid;
	}
	bool Created() const noexcept {
		return wid != nullptr;
	}
	void Destroy() noexcept;
	PRectangle GetPosition() const noexcept;
	void SCICALL SetPosition(PRectangle rc) noexcept;
	void SCICALL SetPositionRelative(PRectangle rc, const Window *relativeTo) noexcept;
	PRectangle GetClientPosition() const noexcept;
	void Show(bool show = true) const noexcept;
	void InvalidateAll() noexcept;
	void SCICALL InvalidateRectangle(PRectangle rc) noexcept;
	virtual void SetFont(const Font &font) noexcept;
	enum class Cursor {
		invalid, text, arrow, up, wait, horiz, vert, reverseArrow, hand
	};
	void SetCursor(Cursor curs) noexcept;
	PRectangle SCICALL GetMonitorRect(Point pt) const noexcept;

private:
	Cursor cursorLast;
};

/**
 * Listbox management.
 */

// ScintillaBase implements IListBoxDelegate to receive ListBoxEvents from a ListBox

struct ListBoxEvent {
	enum class EventType {
		selectionChange, doubleClick
	} event;
	explicit ListBoxEvent(EventType event_) noexcept : event(event_) {}
};

class IListBoxDelegate {
public:
	virtual void ListNotify(ListBoxEvent *plbe) = 0;
};

struct ListOptions {
	std::optional<ColourAlpha> fore;
	std::optional<ColourAlpha> back;
	std::optional<ColourAlpha> foreSelected;
	std::optional<ColourAlpha> backSelected;
};

class ListBox : public Window {
public:
	ListBox() noexcept;
	~ListBox() override;
	static ListBox *Allocate();

	void SetFont(const Font &font) noexcept override = 0;
	virtual void SetColour(ColourDesired fore, ColourDesired back) noexcept = 0;
	virtual void SCICALL Create(Window &parent, int ctrlID, Point location, int lineHeight_, bool unicodeMode_, int technology_) noexcept = 0;
	virtual void SetAverageCharWidth(int width) noexcept = 0;
	virtual void SetVisibleRows(int rows) noexcept = 0;
	virtual int GetVisibleRows() const noexcept = 0;
	virtual PRectangle GetDesiredRect() = 0;
	virtual int CaretFromEdge() const = 0;
	virtual void Clear() noexcept = 0;
	virtual void Append(const char *s, int type = -1) const noexcept = 0;
	virtual int Length() const noexcept = 0;
	virtual void Select(int n) = 0;
	virtual int GetSelection() const noexcept = 0;
	virtual int Find(const char *prefix) const noexcept = 0;
	virtual void GetValue(int n, char *value, int len) const noexcept = 0;
	virtual void RegisterImage(int type, const char *xpm_data) = 0;
	virtual void RegisterRGBAImage(int type, int width, int height, const unsigned char *pixelsImage) = 0;
	virtual void ClearRegisteredImages() noexcept = 0;
	virtual void SetDelegate(IListBoxDelegate *lbDelegate) noexcept = 0;
	virtual void SetList(const char* list, char separator, char typesep) = 0;
};

/**
 * Menu management.
 */
class Menu {
	MenuID mid;
public:
	Menu() noexcept;
	MenuID GetID() const noexcept {
		return mid;
	}
	void CreatePopUp() noexcept;
	void Destroy() noexcept;
	void SCICALL Show(Point pt, const Window &w) noexcept;
};

/**
 * Dynamic Library (DLL/SO/...) loading
 */
class DynamicLibrary {
public:
	virtual ~DynamicLibrary() = default;

	/// @return Pointer to function "name", or NULL on failure.
	virtual Function FindFunction(const char *name) = 0;

	/// @return true if the library was loaded successfully.
	virtual bool IsValid() = 0;

	/// @return An instance of a DynamicLibrary subclass with "modulePath" loaded.
	static DynamicLibrary *Load(const char *modulePath);
};

#if defined(__clang__)
	#if __has_feature(attribute_analyzer_noreturn)
		#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
	#else
		#define CLANG_ANALYZER_NORETURN
	#endif
#else
	#define CLANG_ANALYZER_NORETURN
#endif

}
