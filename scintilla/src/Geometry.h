// Scintilla source code edit control
/** @file Geometry.h
 ** Classes and functions for geometric and colour calculations.
 **/
// Copyright 2020 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef GEOMETRY_H
#define GEOMETRY_H

namespace Scintilla {

typedef double XYPOSITION;
typedef double XYACCUMULATOR;

// Test if an enum class value has the bit flag(s) of test set.
template <typename T>
constexpr bool FlagSet(T value, T test) {
	return (static_cast<int>(value) & static_cast<int>(test)) == static_cast<int>(test);
}

/**
 * A geometric point class.
 * Point is similar to the Win32 POINT and GTK+ GdkPoint types.
 */
class Point {
public:
	XYPOSITION x;
	XYPOSITION y;

	constexpr explicit Point(XYPOSITION x_=0, XYPOSITION y_=0) noexcept : x(x_), y(y_) {
	}

	static constexpr Point FromInts(int x_, int y_) noexcept {
		return Point(static_cast<XYPOSITION>(x_), static_cast<XYPOSITION>(y_));
	}

	constexpr bool operator==(Point other) const noexcept {
		return (x == other.x) && (y == other.y);
	}

	constexpr bool operator!=(Point other) const noexcept {
		return (x != other.x) || (y != other.y);
	}

	constexpr Point operator+(Point other) const noexcept {
		return Point(x + other.x, y + other.y);
	}

	constexpr Point operator-(Point other) const noexcept {
		return Point(x - other.x, y - other.y);
	}

	// Other automatically defined methods (assignment, copy constructor, destructor) are fine
};


/**
 * A geometric interval class.
 */
class Interval {
public:
	XYPOSITION left;
	XYPOSITION right;
	constexpr bool operator==(const Interval &other) const noexcept {
		return (left == other.left) && (right == other.right);
	}
	constexpr XYPOSITION Width() const noexcept { return right - left; }
	constexpr bool Empty() const noexcept {
		return Width() <= 0;
	}
	constexpr bool Intersects(Interval other) const noexcept {
		return (right > other.left) && (left < other.right);
	}
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

	constexpr explicit PRectangle(XYPOSITION left_=0, XYPOSITION top_=0, XYPOSITION right_=0, XYPOSITION bottom_ = 0) noexcept :
		left(left_), top(top_), right(right_), bottom(bottom_) {
	}

	static constexpr PRectangle FromInts(int left_, int top_, int right_, int bottom_) noexcept {
		return PRectangle(static_cast<XYPOSITION>(left_), static_cast<XYPOSITION>(top_),
			static_cast<XYPOSITION>(right_), static_cast<XYPOSITION>(bottom_));
	}

	// Other automatically defined methods (assignment, copy constructor, destructor) are fine

	constexpr bool operator==(const PRectangle &rc) const noexcept {
		return (rc.left == left) && (rc.right == right) &&
			(rc.top == top) && (rc.bottom == bottom);
	}
	constexpr bool Contains(Point pt) const noexcept {
		return (pt.x >= left) && (pt.x <= right) &&
			(pt.y >= top) && (pt.y <= bottom);
	}
	constexpr bool ContainsWholePixel(Point pt) const noexcept {
		// Does the rectangle contain all of the pixel to left/below the point
		return (pt.x >= left) && ((pt.x+1) <= right) &&
			(pt.y >= top) && ((pt.y+1) <= bottom);
	}
	constexpr bool Contains(PRectangle rc) const noexcept {
		return (rc.left >= left) && (rc.right <= right) &&
			(rc.top >= top) && (rc.bottom <= bottom);
	}
	constexpr bool Intersects(PRectangle other) const noexcept {
		return (right > other.left) && (left < other.right) &&
			(bottom > other.top) && (top < other.bottom);
	}
	void Move(XYPOSITION xDelta, XYPOSITION yDelta) noexcept {
		left += xDelta;
		top += yDelta;
		right += xDelta;
		bottom += yDelta;
	}

	constexpr PRectangle Inset(XYPOSITION delta) const noexcept {
		return PRectangle(left + delta, top + delta, right - delta, bottom - delta);
	}

	constexpr Point Centre() const noexcept {
		return Point((left + right) / 2, (top + bottom) / 2);
	}

	constexpr XYPOSITION Width() const noexcept { return right - left; }
	constexpr XYPOSITION Height() const noexcept { return bottom - top; }
	constexpr bool Empty() const noexcept {
		return (Height() <= 0) || (Width() <= 0);
	}
};

enum class Edge { left, top, bottom, right };

PRectangle Clamp(PRectangle rc, Edge edge, XYPOSITION position) noexcept;
PRectangle Side(PRectangle rc, Edge edge, XYPOSITION size) noexcept;

Interval Intersection(Interval a, Interval b) noexcept;
PRectangle Intersection(PRectangle rc, Interval horizontalBounds) noexcept;
Interval HorizontalBounds(PRectangle rc) noexcept;

XYPOSITION PixelAlign(XYPOSITION xy, int pixelDivisions) noexcept;
XYPOSITION PixelAlignFloor(XYPOSITION xy, int pixelDivisions) noexcept;

Point PixelAlign(const Point &pt, int pixelDivisions) noexcept;

PRectangle PixelAlign(const PRectangle &rc, int pixelDivisions) noexcept;
PRectangle PixelAlignOutside(const PRectangle &rc, int pixelDivisions) noexcept;

/**
 * Holds an RGB colour with 8 bits for each component.
 */
constexpr const float componentMaximum = 255.0f;
class ColourDesired {
	int co;
public:
	constexpr explicit ColourDesired(int co_=0) noexcept : co(co_) {
	}

	constexpr ColourDesired(unsigned int red, unsigned int green, unsigned int blue) noexcept :
		co(red | (green << 8) | (blue << 16)) {
	}

	constexpr bool operator==(const ColourDesired &other) const noexcept {
		return co == other.co;
	}

	constexpr int AsInteger() const noexcept {
		return co;
	}

	// Red, green and blue values as bytes 0..255
	constexpr unsigned char GetRed() const noexcept {
		return co & 0xff;
	}
	constexpr unsigned char GetGreen() const noexcept {
		return (co >> 8) & 0xff;
	}
	constexpr unsigned char GetBlue() const noexcept {
		return (co >> 16) & 0xff;
	}

	// Red, green and blue values as float 0..1.0
	constexpr float GetRedComponent() const noexcept {
		return GetRed() / componentMaximum;
	}
	constexpr float GetGreenComponent() const noexcept {
		return GetGreen() / componentMaximum;
	}
	constexpr float GetBlueComponent() const noexcept {
		return GetBlue() / componentMaximum;
	}
};

/**
* Holds an RGBA colour.
*/
class ColourAlpha : public ColourDesired {
public:
	constexpr explicit ColourAlpha(int co_ = 0) noexcept : ColourDesired(co_) {
	}

	constexpr ColourAlpha(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha=0xff) noexcept :
		ColourDesired(red | (green << 8) | (blue << 16) | (alpha << 24)) {
	}

	constexpr ColourAlpha(ColourDesired cd, unsigned int alpha) noexcept :
		ColourDesired(cd.AsInteger() | (alpha << 24)) {
	}

	constexpr ColourAlpha(ColourDesired cd) noexcept :
		ColourDesired(cd.AsInteger() | (0xffu << 24)) {
	}

	constexpr ColourDesired GetColour() const noexcept {
		return ColourDesired(AsInteger() & 0xffffff);
	}

	constexpr unsigned char GetAlpha() const noexcept {
		return (AsInteger() >> 24) & 0xff;
	}

	constexpr float GetAlphaComponent() const noexcept {
		return GetAlpha() / componentMaximum;
	}

	constexpr bool IsOpaque() const noexcept {
		return GetAlpha() == 0xff;
	}

	constexpr ColourAlpha MixedWith(ColourAlpha other) const noexcept {
		const unsigned int red = (GetRed() + other.GetRed()) / 2;
		const unsigned int green = (GetGreen() + other.GetGreen()) / 2;
		const unsigned int blue = (GetBlue() + other.GetBlue()) / 2;
		const unsigned int alpha = (GetAlpha() + other.GetAlpha()) / 2;
		return ColourAlpha(red, green, blue, alpha);
	}
};

/**
* Holds an RGBA colour and stroke width to stroke a shape.
*/
class Stroke {
public:
	ColourAlpha colour;
	XYPOSITION width;
	constexpr Stroke(ColourAlpha colour_, XYPOSITION width_=1.0f) noexcept : 
		colour(colour_), width(width_) {
	}
	constexpr float WidthF() {
		return static_cast<float>(width);
	}
};

/**
* Holds an RGBA colour to fill a shape.
*/
class Fill {
public:
	ColourAlpha colour;
	constexpr Fill(ColourAlpha colour_) noexcept : 
		colour(colour_) {
	}
	constexpr Fill(ColourDesired colour_) noexcept :
		colour(colour_) {
	}
};

/**
* Holds a pair of RGBA colours and stroke width to fill and stroke a shape.
*/
class FillStroke {
public:
	Fill fill;
	Stroke stroke;
	constexpr FillStroke(ColourAlpha colourFill_, ColourAlpha colourStroke_, XYPOSITION widthStroke_=1.0f) noexcept : 
		fill(colourFill_), stroke(colourStroke_, widthStroke_) {
	}
	constexpr FillStroke(ColourAlpha colourBoth, XYPOSITION widthStroke_=1.0f) noexcept : 
		fill(colourBoth), stroke(colourBoth, widthStroke_) {
	}
};

/**
* Holds an element of a gradient with an RGBA colour and a relative position.
*/
class ColourStop {
public:
	XYPOSITION position;
	ColourAlpha colour;
	constexpr ColourStop(XYPOSITION position_, ColourAlpha colour_) noexcept :
		position(position_), colour(colour_) {
	}
};

}

#endif
