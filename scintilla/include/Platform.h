// Scintilla source code edit control
/** @file Platform.h
 ** Interface to platform facilities. Also includes some basic utilities.
 ** Implemented in PlatGTK.cxx for GTK/Linux, PlatWin.cxx for Windows, and PlatWX.cxx for wxWindows.
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

typedef float XYPOSITION;
typedef double XYACCUMULATOR;

// Underlying the implementation of the platform classes are platform specific types.
// Sometimes these need to be passed around by client code so they are defined here

typedef void *FontID;
typedef void *SurfaceID;
typedef void *WindowID;
typedef void *MenuID;
typedef void *TickerID;
typedef void *Function;
typedef void *IdlerID;

/**
 * A geometric point class.
 * Point is similar to the Win32 POINT and GTK GdkPoint types.
 */
class Point {
public:
	XYPOSITION x;
	XYPOSITION y;

	constexpr explicit Point(XYPOSITION x_ = 0, XYPOSITION y_ = 0) noexcept : x(x_), y(y_) {}

	static constexpr Point FromInts(int x_, int y_) noexcept {
		return Point(static_cast<XYPOSITION>(x_), static_cast<XYPOSITION>(y_));
	}

	bool operator!=(Point other) const noexcept {
		return (x != other.x) || (y != other.y);
	}

	Point operator+(Point other) const noexcept {
		return Point(x + other.x, y + other.y);
	}

	Point operator-(Point other) const noexcept {
		return Point(x - other.x, y - other.y);
	}

	// Other automatically defined methods (assignment, copy constructor, destructor) are fine
};

/**
 * A geometric rectangle class.
 * PRectangle is similar to Win32 RECT.
 * PRectangles contain their top and left sides, but not their right and bottom sides.
 */
class PRectangle {
public:
	XYPOSITION left;
	XYPOSITION top;
	XYPOSITION right;
	XYPOSITION bottom;

	constexpr explicit PRectangle(XYPOSITION left_ = 0, XYPOSITION top_ = 0, XYPOSITION right_ = 0, XYPOSITION bottom_ = 0) noexcept :
		left(left_), top(top_), right(right_), bottom(bottom_) {}

	static constexpr PRectangle FromInts(int left_, int top_, int right_, int bottom_) noexcept {
		return PRectangle(static_cast<XYPOSITION>(left_), static_cast<XYPOSITION>(top_),
			static_cast<XYPOSITION>(right_), static_cast<XYPOSITION>(bottom_));
	}

	// Other automatically defined methods (assignment, copy constructor, destructor) are fine

	bool operator==(const PRectangle &rc) const noexcept {
		return (rc.left == left) && (rc.right == right) &&
			(rc.top == top) && (rc.bottom == bottom);
	}
	bool Contains(Point pt) const noexcept {
		return (pt.x >= left) && (pt.x <= right) &&
			(pt.y >= top) && (pt.y <= bottom);
	}
	bool ContainsWholePixel(Point pt) const noexcept {
		// Does the rectangle contain all of the pixel to left/below the point
		return (pt.x >= left) && ((pt.x + 1) <= right) &&
			(pt.y >= top) && ((pt.y + 1) <= bottom);
	}
	bool Contains(PRectangle rc) const noexcept {
		return (rc.left >= left) && (rc.right <= right) &&
			(rc.top >= top) && (rc.bottom <= bottom);
	}
	bool Intersects(PRectangle other) const noexcept {
		return (right > other.left) && (left < other.right) &&
			(bottom > other.top) && (top < other.bottom);
	}
	void Move(XYPOSITION xDelta, XYPOSITION yDelta) noexcept {
		left += xDelta;
		top += yDelta;
		right += xDelta;
		bottom += yDelta;
	}
	constexpr PRectangle Inflate(XYPOSITION xDelta, XYPOSITION yDelta) const noexcept {
		return PRectangle(left - xDelta, top - yDelta, right + xDelta, bottom + yDelta);
	}
	constexpr PRectangle Inflate(int xDelta, int yDelta) const noexcept {
		return PRectangle(left - xDelta, top - yDelta, right + xDelta, bottom + yDelta);
	}
	constexpr PRectangle Deflate(XYPOSITION xDelta, XYPOSITION yDelta) const noexcept {
		return Inflate(-xDelta, -yDelta);
	}
	constexpr PRectangle Deflate(int xDelta, int yDelta) const noexcept {
		return Inflate(-xDelta, -yDelta);
	}
	XYPOSITION Width() const noexcept {
		return right - left;
	}
	XYPOSITION Height() const noexcept {
		return bottom - top;
	}
	bool Empty() const noexcept {
		return (Height() <= 0) || (Width() <= 0);
	}
};

/**
 * Holds an RGB colour with 8 bits for each component.
 */
constexpr float componentMaximum = 255.0F;
class ColourDesired {
	unsigned int co;
public:
	explicit ColourDesired(unsigned int co_ = 0) noexcept : co(co_) {}

	ColourDesired(unsigned int red, unsigned int green, unsigned int blue) noexcept :
		co(red | (green << 8) | (blue << 16)) {}

	bool operator==(const ColourDesired &other) const noexcept {
		return co == other.co;
	}

	unsigned int AsInteger() const noexcept {
		return co;
	}

	// Red, green and blue values as bytes 0..255
	unsigned char GetRed() const noexcept {
		return co & 0xff;
	}
	unsigned char GetGreen() const noexcept {
		return (co >> 8) & 0xff;
	}
	unsigned char GetBlue() const noexcept {
		return (co >> 16) & 0xff;
	}

	// Red, green and blue values as float 0..1.0
	float GetRedComponent() const noexcept {
		return GetRed() / componentMaximum;
	}
	float GetGreenComponent() const noexcept {
		return GetGreen() / componentMaximum;
	}
	float GetBlueComponent() const noexcept {
		return GetBlue() / componentMaximum;
	}
};

/**
* Holds an RGBA colour.
*/
class ColourAlpha : public ColourDesired {
public:
	explicit ColourAlpha(unsigned int co_ = 0) noexcept : ColourDesired(co_) {}

	ColourAlpha(unsigned int red, unsigned int green, unsigned int blue) noexcept :
		ColourDesired(red | (green << 8) | (blue << 16)) {}
	ColourAlpha(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha) noexcept :
		ColourDesired(red | (green << 8) | (blue << 16) | (alpha << 24)) {}

	ColourAlpha(ColourDesired cd, unsigned int alpha) noexcept :
		ColourDesired(cd.AsInteger() | (alpha << 24)) {}

	ColourDesired GetColour() const noexcept {
		return ColourDesired(AsInteger() & 0xffffff);
	}

	unsigned char GetAlpha() const noexcept {
		return (AsInteger() >> 24) & 0xff;
	}

	float GetAlphaComponent() const noexcept {
		return GetAlpha() / componentMaximum;
	}

	ColourAlpha MixedWith(ColourAlpha other) const noexcept {
		const unsigned int red = (GetRed() + other.GetRed()) / 2;
		const unsigned int green = (GetGreen() + other.GetGreen()) / 2;
		const unsigned int blue = (GetBlue() + other.GetBlue()) / 2;
		const unsigned int alpha = (GetAlpha() + other.GetAlpha()) / 2;
		return ColourAlpha(red, green, blue, alpha);
	}
};

/**
* Holds an element of a gradient with an RGBA colour and a relative position.
*/
class ColourStop {
public:
	float position;
	ColourAlpha colour;
	ColourStop(float position_, ColourAlpha colour_) noexcept :
		position(position_), colour(colour_) {}
};

/**
 * Font management.
 */

constexpr const char *defaultLocaleName = ""; // "en-us";

struct FontParameters {
	const char *faceName;
	float size;
	int weight;
	int stretch;
	bool italic;
	int extraFontFlag;
	int technology;
	int characterSet;
	const char *localeName;

	explicit FontParameters(
		const char *faceName_,
		float size_=10,
		int weight_=400,
		int stretch_=5,
		bool italic_=false,
		int extraFontFlag_=0,
		int technology_=0,
		int characterSet_=0,
		const char *localeName_=defaultLocaleName) noexcept :

		faceName(faceName_),
		size(size_),
		weight(weight_),
		stretch(stretch_),
		italic(italic_),
		extraFontFlag(extraFontFlag_),
		technology(technology_),
		characterSet(characterSet_),
		localeName(localeName_) {}

};

class Font {
protected:
	FontID fid;
public:
	Font() noexcept;
	// Deleted so Font objects can not be copied
	Font(const Font &) = delete;
	Font(Font &&) = delete;
	Font &operator=(const Font &) = delete;
	Font &operator=(Font &&) = delete;
	virtual ~Font();

	virtual void Create(const FontParameters &fp);
	virtual void Release() noexcept;

	FontID GetID() const noexcept {
		return fid;
	}
	// Alias another font - caller guarantees not to Release
	void SetID(FontID fid_) noexcept {
		fid = fid_;
	}
	friend class Surface;
	friend class SurfaceImpl;
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

struct Interval {
	XYPOSITION left;
	XYPOSITION right;
};

class IScreenLineLayout {
public:
	virtual ~IScreenLineLayout() = default;
	virtual size_t PositionFromX(XYPOSITION xDistance, bool charPosition) = 0;
	virtual XYPOSITION XFromPosition(size_t caretPosition) noexcept = 0;
	virtual std::vector<Interval> FindRangeIntervals(size_t start, size_t end) = 0;
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
	virtual ~Surface() = default;
	static Surface *Allocate(int technology);

	virtual void Init(WindowID wid) noexcept = 0;
	virtual void Init(SurfaceID sid, WindowID wid, bool printing = false) noexcept = 0;
	virtual void InitPixMap(int width, int height, Surface *surface_, WindowID wid) noexcept = 0;

	virtual void Release() noexcept = 0;
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
	virtual void FlushCachedState() noexcept = 0;

	virtual void SetUnicodeMode(bool unicodeMode_) noexcept = 0;
	virtual void SetDBCSMode(int codePage) noexcept = 0;
	virtual void SetBidiR2L(bool bidiR2L_) noexcept = 0;
};

/**
 * Class to hide the details of window manipulation.
 * Does not own the window which will normally have a longer life than this object.
 */
class Window {
protected:
	WindowID wid;

public:
	Window() noexcept : wid(nullptr), cursorLast(Cursor::cursorInvalid) {}
	Window(const Window &source) = delete;
	Window(Window &&) = delete;
	Window &operator=(WindowID wid_) noexcept {
		wid = wid_;
		cursorLast = Cursor::cursorInvalid;
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
		cursorInvalid, cursorText, cursorArrow, cursorUp, cursorWait, cursorHoriz, cursorVert, cursorReverseArrow, cursorHand
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

/**
 * Platform class used to retrieve system wide parameters such as double click speed
 * and chrome colour. Not a creatable object, more of a module with several functions.
 */
class Platform {
public:
	Platform() noexcept = default;
	Platform(const Platform &) = delete;
	Platform(Platform &&) = delete;
	Platform &operator=(const Platform &) = delete;
	Platform &operator=(Platform &&) = delete;
	~Platform() = default;

	static ColourDesired Chrome() noexcept;
	static ColourDesired ChromeHighlight() noexcept;
	static const char *DefaultFont() noexcept;
	static int DefaultFontSize() noexcept;
	static unsigned int DoubleClickTime() noexcept;

	static void DebugDisplay(const char *s) noexcept;
	static void DebugPrintf(const char *format, ...) noexcept
#if defined(__GNUC__) || defined(__clang__)
	__attribute__((format(printf, 1, 2)))
#endif
	;
	static bool ShowAssertionPopUps(bool assertionPopUps_) noexcept;
	static void Assert(const char *c, const char *file, int line) noexcept CLANG_ANALYZER_NORETURN;
};

#ifdef  NDEBUG
#define PLATFORM_ASSERT(c) ((void)0)
#else
#define PLATFORM_ASSERT(c) ((c) ? (void)(0) : Scintilla::Platform::Assert(#c, __FILE__, __LINE__))
#endif

}
