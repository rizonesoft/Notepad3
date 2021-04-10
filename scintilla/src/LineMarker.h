// Scintilla source code edit control
/** @file LineMarker.h
 ** Defines the look of a line marker in the margin .
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef LINEMARKER_H
#define LINEMARKER_H

namespace Scintilla {

class XPM;
class RGBAImage;

typedef void (*DrawLineMarkerFn)(Surface *surface, const PRectangle &rcWhole, const Font *fontForCharacter, int tFold, int marginStyle, const void *lineMarker);

/**
 */
class LineMarker {
public:
	enum class FoldPart { undefined, head, body, tail, headWithTail };

	int markType = SC_MARK_CIRCLE;
	ColourAlpha fore = ColourAlpha(0, 0, 0);
	ColourAlpha back = ColourAlpha(0xff, 0xff, 0xff);
	ColourAlpha backSelected = ColourAlpha(0xff, 0x00, 0x00);
	int alpha = SC_ALPHA_NOALPHA;
	XYPOSITION strokeWidth = 1.0f;
	std::unique_ptr<XPM> pxpm;
	std::unique_ptr<RGBAImage> image;
	/** Some platforms, notably PLAT_CURSES, do not support Scintilla's native
	 * Draw function for drawing line markers. Allow those platforms to override
	 * it instead of creating a new method(s) in the Surface class that existing
	 * platforms must implement as empty. */
	DrawLineMarkerFn customDraw = nullptr;

	LineMarker() noexcept = default;
	LineMarker(const LineMarker &other);
	LineMarker(LineMarker &&) noexcept = default;
	LineMarker &operator=(const LineMarker& other);
	LineMarker &operator=(LineMarker&&) noexcept = default;
	virtual ~LineMarker() = default;

	void SetXPM(const char *textForm);
	void SetXPM(const char *const *linesForm);
	void SetRGBAImage(Point sizeRGBAImage, float scale, const unsigned char *pixelsRGBAImage);
	void AlignedPolygon(Surface *surface, const Point *pts, size_t npts) const;
	void Draw(Surface *surface, const PRectangle &rcWhole, const Font *fontForCharacter, FoldPart part, int marginStyle) const;
	void DrawFoldingMark(Surface *surface, const PRectangle &rcWhole, FoldPart part) const;
};

}

#endif
