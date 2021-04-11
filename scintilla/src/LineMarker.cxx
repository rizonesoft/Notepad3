// Scintilla source code edit control
/** @file LineMarker.cxx
 ** Defines the look of a line marker in the margin.
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cstring>
#include <cmath>

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <iterator>
#include <memory>

#include "Debugging.h"
#include "Geometry.h"
#include "Platform.h"

#include "Scintilla.h"

#include "XPM.h"
#include "LineMarker.h"
#include "UniConversion.h"

using namespace Scintilla;

LineMarker::LineMarker(const LineMarker &other) {
	// Defined to avoid pxpm and image being blindly copied, not as a complete copy constructor.
	markType = other.markType;
	fore = other.fore;
	back = other.back;
	backSelected = other.backSelected;
	strokeWidth = other.strokeWidth;
	alpha = other.alpha;
	if (other.pxpm)
		pxpm = std::make_unique<XPM>(*other.pxpm);
	else
		pxpm = nullptr;
	if (other.image)
		image = std::make_unique<RGBAImage>(*other.image);
	else
		image = nullptr;
	customDraw = other.customDraw;
}

LineMarker &LineMarker::operator=(const LineMarker &other) {
	// Defined to avoid pxpm and image being blindly copied, not as a complete assignment operator.
	if (this != &other) {
		markType = other.markType;
		fore = other.fore;
		back = other.back;
		backSelected = other.backSelected;
		strokeWidth = other.strokeWidth;
		alpha = other.alpha;
		if (other.pxpm)
			pxpm = std::make_unique<XPM>(*other.pxpm);
		else
			pxpm = nullptr;
		if (other.image)
			image = std::make_unique<RGBAImage>(*other.image);
		else
			image = nullptr;
		customDraw = other.customDraw;
	}
	return *this;
}

void LineMarker::SetXPM(const char *textForm) {
	pxpm = std::make_unique<XPM>(textForm);
	markType = SC_MARK_PIXMAP;
}

void LineMarker::SetXPM(const char *const *linesForm) {
	pxpm = std::make_unique<XPM>(linesForm);
	markType = SC_MARK_PIXMAP;
}

void LineMarker::SetRGBAImage(Point sizeRGBAImage, float scale, const unsigned char *pixelsRGBAImage) {
	image = std::make_unique<RGBAImage>(static_cast<int>(sizeRGBAImage.x), static_cast<int>(sizeRGBAImage.y), scale, pixelsRGBAImage);
	markType = SC_MARK_RGBAIMAGE;
}

namespace {

enum class Expansion { Minus, Plus };
enum class Shape { Square, Circle };

void DrawSymbol(Surface *surface, Shape shape, Expansion expansion, PRectangle rcSymbol, XYPOSITION widthStroke,
	ColourAlpha colourFill, ColourAlpha colourFrame, ColourAlpha colourFrameRight, ColourAlpha colourExpansion) {

	const FillStroke fillStroke(colourFill, colourFrame, widthStroke);
	const PRectangle rcSymbolLeft = Side(rcSymbol, Edge::left, (rcSymbol.Width() + widthStroke) / 2.0f);
	surface->SetClip(rcSymbolLeft);
	if (shape == Shape::Square) {
		// Hollowed square
		surface->RectangleDraw(rcSymbol, fillStroke);
	} else {
		surface->Ellipse(rcSymbol, fillStroke);
	}
	surface->PopClip();

	const FillStroke fillStrokeRight(colourFill, colourFrameRight, widthStroke);
	const PRectangle rcSymbolRight = Side(rcSymbol, Edge::right, (rcSymbol.Width() - widthStroke) / 2.0f);
	surface->SetClip(rcSymbolRight);
	if (shape == Shape::Square) {
		surface->RectangleDraw(rcSymbol, fillStrokeRight);
	} else {
		surface->Ellipse(rcSymbol, fillStrokeRight);
	}
	surface->PopClip();

	const PRectangle rcPlusMinus = rcSymbol.Inset(widthStroke + 1.0f);
	const XYPOSITION armWidth = (rcPlusMinus.Width() - widthStroke) / 2.0f;
	const XYPOSITION top = rcPlusMinus.top + armWidth;
	const PRectangle rcH = PRectangle(
		rcPlusMinus.left, top,
		rcPlusMinus.right, top + widthStroke);
	surface->FillRectangle(rcH, colourExpansion);
	if (expansion == Expansion::Plus) {
		const XYPOSITION left = rcPlusMinus.left + armWidth;
		const PRectangle rcV = PRectangle(
			left, rcPlusMinus.top,
			left + widthStroke, rcPlusMinus.bottom);
		surface->FillRectangle(rcV, colourExpansion);
	}
}

void DrawTail(Surface *surface, XYPOSITION leftLine, XYPOSITION rightTail, XYPOSITION centreY, XYPOSITION widthSymbolStroke, ColourAlpha fill) {
	const XYPOSITION slopeLength = 2.0f + widthSymbolStroke;
	const XYPOSITION strokeTop = centreY + slopeLength;
	const XYPOSITION halfWidth = widthSymbolStroke / 2.0f;
	const XYPOSITION strokeMiddle = strokeTop + halfWidth;
	Point lines[] = {
		// Stick
		Point(rightTail, strokeMiddle),
		Point(leftLine + halfWidth + slopeLength, strokeMiddle),
		// Slope
		Point(leftLine + widthSymbolStroke / 2.0f, centreY + halfWidth),
	};
	surface->PolyLine(lines, std::size(lines), Stroke(fill, widthSymbolStroke));
}

}

void LineMarker::DrawFoldingMark(Surface *surface, const PRectangle &rcWhole, FoldPart part) const {
	// Assume: edges of rcWhole are integers.
	// Code can only really handle integer strokeWidth.

	ColourAlpha colourHead = back;
	ColourAlpha colourBody = back;
	ColourAlpha colourTail = back;

	switch (part) {
	case FoldPart::head:
	case FoldPart::headWithTail:
		colourHead = backSelected;
		colourTail = backSelected;
		break;
	case FoldPart::body:
		colourHead = backSelected;
		colourBody = backSelected;
		break;
	case FoldPart::tail:
		colourBody = backSelected;
		colourTail = backSelected;
		break;
	default:
		// LineMarker::undefined
		break;
	}

	const int pixelDivisions = surface->PixelDivisions();

	// Folding symbols should have equal height and width to be either a circle or square.
	// So find the minimum of width and height.
	const XYPOSITION minDimension = std::floor(std::min(rcWhole.Width(), rcWhole.Height() - 2)) - 1;

	// If strokeWidth would take up too much of area reduce to reasonable width.
	const XYPOSITION widthStroke = PixelAlignFloor(std::min(strokeWidth, minDimension / 5.0f), pixelDivisions);

	// To centre +/-, odd strokeWidth -> odd symbol width, even -> even
	const XYPOSITION widthSymbol =
		((std::lround(minDimension * pixelDivisions) % 2) == (std::lround(widthStroke * pixelDivisions) % 2)) ?
		minDimension : minDimension - 1.0f / pixelDivisions;

	const Point centre = PixelAlign(rcWhole.Centre(), pixelDivisions);

	// Folder symbols and lines follow some rules to join up, fit the pixel grid,
	// and avoid over-painting.

	const XYPOSITION halfSymbol = std::round(widthSymbol / 2);
	const Point topLeft(centre.x - halfSymbol, centre.y - halfSymbol);
	const PRectangle rcSymbol(
		topLeft.x, topLeft.y,
		topLeft.x + widthSymbol, topLeft.y + widthSymbol);
	const XYPOSITION leftLine = rcSymbol.Centre().x - widthStroke / 2.0f;
	const XYPOSITION rightLine = leftLine + widthStroke;

	// This is the vertical line through the whole area which is subdivided
	// when there is a symbol on the line or the colour changes for highlighting.
	const PRectangle rcVLine(leftLine, rcWhole.top, rightLine, rcWhole.bottom);

	// Portions of rcVLine above and below the symbol.
	const PRectangle rcAboveSymbol = Clamp(rcVLine, Edge::bottom, rcSymbol.top);
	const PRectangle rcBelowSymbol = Clamp(rcVLine, Edge::top, rcSymbol.bottom);

	// Projection to right.
	const PRectangle rcStick(
		rcVLine.right, centre.y + 1.0f - widthStroke,
		rcWhole.right - 1, centre.y + 1.0f);

	switch (markType) {

	case SC_MARK_VLINE:
		surface->FillRectangle(rcVLine, colourBody);
		break;

	case SC_MARK_LCORNER:
		surface->FillRectangle(Clamp(rcVLine, Edge::bottom, centre.y + 1.0f), colourTail);
		surface->FillRectangle(rcStick, colourTail);
		break;

	case SC_MARK_TCORNER:
		surface->FillRectangle(Clamp(rcVLine, Edge::bottom, centre.y + 1.0f), colourBody);
		surface->FillRectangle(Clamp(rcVLine, Edge::top, centre.y + 1.0f), colourHead);
		surface->FillRectangle(rcStick, colourTail);
		break;

	// CORNERCURVE cases divide slightly lower than CORNER to accommodate the curve
	case SC_MARK_LCORNERCURVE:
		surface->FillRectangle(Clamp(rcVLine, Edge::bottom, centre.y), colourTail);
		DrawTail(surface, leftLine, rcWhole.right - 1.0f, centre.y - widthStroke,
			widthStroke, colourTail);
		break;

	case SC_MARK_TCORNERCURVE:
		surface->FillRectangle(Clamp(rcVLine, Edge::bottom, centre.y), colourBody);
		surface->FillRectangle(Clamp(rcVLine, Edge::top, centre.y), colourHead);
		DrawTail(surface, leftLine, rcWhole.right - 1.0f, centre.y - widthStroke,
			widthStroke, colourTail);
		break;

	case SC_MARK_BOXPLUS:
		DrawSymbol(surface, Shape::Square, Expansion::Plus, rcSymbol, widthStroke,
			fore, colourHead, colourHead, colourTail);
		break;

	case SC_MARK_BOXPLUSCONNECTED: {
			const ColourAlpha colourBelow = (part == FoldPart::headWithTail) ? colourTail : colourBody;
			surface->FillRectangle(rcBelowSymbol, colourBelow);
			surface->FillRectangle(rcAboveSymbol, colourBody);

			const ColourAlpha colourRight = (part == FoldPart::body) ? colourTail : colourHead;
			DrawSymbol(surface, Shape::Square, Expansion::Plus, rcSymbol, widthStroke,
				fore, colourHead, colourRight, colourTail);
		}
		break;

	case SC_MARK_BOXMINUS:
		surface->FillRectangle(rcBelowSymbol, colourHead);
		DrawSymbol(surface, Shape::Square, Expansion::Minus, rcSymbol, widthStroke,
			fore, colourHead, colourHead, colourTail);
		break;

	case SC_MARK_BOXMINUSCONNECTED: {
			surface->FillRectangle(rcBelowSymbol, colourHead);
			surface->FillRectangle(rcAboveSymbol, colourBody);

			const ColourAlpha colourRight = (part == FoldPart::body) ? colourTail : colourHead;
			DrawSymbol(surface, Shape::Square, Expansion::Minus, rcSymbol, widthStroke,
				fore, colourHead, colourRight, colourTail);
		}
		break;

	case SC_MARK_CIRCLEPLUS:
		DrawSymbol(surface, Shape::Circle, Expansion::Plus, rcSymbol, widthStroke,
			fore, colourHead, colourHead, colourTail);
		break;

	case SC_MARK_CIRCLEPLUSCONNECTED: {
			const ColourAlpha colourBelow = (part == FoldPart::headWithTail) ? colourTail : colourBody;
			surface->FillRectangle(rcBelowSymbol, colourBelow);
			surface->FillRectangle(rcAboveSymbol, colourBody);

			const ColourAlpha colourRight = (part == FoldPart::body) ? colourTail : colourHead;
			DrawSymbol(surface, Shape::Circle, Expansion::Plus, rcSymbol, widthStroke,
				fore, colourHead, colourRight, colourTail);
		}
		break;

	case SC_MARK_CIRCLEMINUS:
		surface->FillRectangle(rcBelowSymbol, colourHead);
		DrawSymbol(surface, Shape::Circle, Expansion::Minus, rcSymbol, widthStroke,
			fore, colourHead, colourHead, colourTail);
		break;

	case SC_MARK_CIRCLEMINUSCONNECTED: {
			surface->FillRectangle(rcBelowSymbol, colourHead);
			surface->FillRectangle(rcAboveSymbol, colourBody);
			const ColourAlpha colourRight = (part == FoldPart::body) ? colourTail : colourHead;
			DrawSymbol(surface, Shape::Circle, Expansion::Minus, rcSymbol, widthStroke,
				fore, colourHead, colourRight, colourTail);
		}
		break;

	}
}

void LineMarker::AlignedPolygon(Surface *surface, const Point *pts, size_t npts) const {
	const XYPOSITION move = strokeWidth / 2.0;
	std::vector<Point> points;
	std::transform(pts, pts + npts, std::back_inserter(points), [=](Point pt)->Point {
		return Point(pt.x + move, pt.y + move);
	});
	surface->Polygon(points.data(), std::size(points), FillStroke(back, fore, strokeWidth));
}

void LineMarker::Draw(Surface *surface, const PRectangle &rcWhole, const Font *fontForCharacter, FoldPart part, int marginStyle) const {
	// This is to satisfy the changed API - eventually the stroke width will be exposed to clients

	if (customDraw) {
		customDraw(surface, rcWhole, fontForCharacter, static_cast<int>(part), marginStyle, this);
		return;
	}

	if ((markType == SC_MARK_PIXMAP) && (pxpm)) {
		pxpm->Draw(surface, rcWhole);
		return;
	}
	if ((markType == SC_MARK_RGBAIMAGE) && (image)) {
		// Make rectangle just large enough to fit image centred on centre of rcWhole
		PRectangle rcImage;
		rcImage.top = ((rcWhole.top + rcWhole.bottom) - image->GetScaledHeight()) / 2;
		rcImage.bottom = rcImage.top + image->GetScaledHeight();
		rcImage.left = ((rcWhole.left + rcWhole.right) - image->GetScaledWidth()) / 2;
		rcImage.right = rcImage.left + image->GetScaledWidth();
		surface->DrawRGBAImage(rcImage, image->GetWidth(), image->GetHeight(), image->Pixels());
		return;
	}

	if ((markType >= SC_MARK_VLINE) && markType <= (SC_MARK_CIRCLEMINUSCONNECTED)) {
		DrawFoldingMark(surface, rcWhole, part);
		return;
	}

	// Restrict most shapes a bit
	const PRectangle rc(rcWhole.left, rcWhole.top + 1, rcWhole.right, rcWhole.bottom - 1);
	// Ensure does not go beyond edge
	const XYPOSITION minDim = std::min(rcWhole.Width(), rcWhole.Height() - 2) - 1;

	const Point centre = rcWhole.Centre();
	XYPOSITION centreX = std::floor(centre.x);
	const XYPOSITION centreY = std::floor(centre.y);
	const XYPOSITION dimOn2 = std::floor(minDim / 2);
	const XYPOSITION dimOn4 = std::floor(minDim / 4);
	const XYPOSITION armSize = dimOn2 - 2;
	if (marginStyle == SC_MARGIN_NUMBER || marginStyle == SC_MARGIN_TEXT || marginStyle == SC_MARGIN_RTEXT) {
		// On textual margins move marker to the left to try to avoid overlapping the text
		centreX = rcWhole.left + dimOn2 + 1;
	}

	switch (markType) {
	case SC_MARK_ROUNDRECT: {
			PRectangle rcRounded = rc;
			rcRounded.left = rc.left + 1;
			rcRounded.right = rc.right - 1;
			surface->RoundedRectangle(rcRounded, FillStroke(back, fore, strokeWidth));
		}
		break;

	case SC_MARK_CIRCLE: {
			const PRectangle rcCircle = PRectangle(
							    centreX - dimOn2,
							    centreY - dimOn2,
							    centreX + dimOn2,
							    centreY + dimOn2);
			surface->Ellipse(rcCircle, FillStroke(back, fore, strokeWidth));
		}
		break;

	case SC_MARK_ARROW: {
			Point pts[] = {
				Point(centreX - dimOn4, centreY - dimOn2),
				Point(centreX - dimOn4, centreY + dimOn2),
				Point(centreX + dimOn2 - dimOn4, centreY),
			};
			AlignedPolygon(surface, pts, std::size(pts));
		}
		break;

	case SC_MARK_ARROWDOWN: {
			Point pts[] = {
				Point(centreX - dimOn2, centreY - dimOn4),
				Point(centreX + dimOn2, centreY - dimOn4),
				Point(centreX, centreY + dimOn2 - dimOn4),
			};
			AlignedPolygon(surface, pts, std::size(pts));
		}
		break;

	case SC_MARK_PLUS: {
			Point pts[] = {
				Point(centreX - armSize, centreY - 1),
				Point(centreX - 1, centreY - 1),
				Point(centreX - 1, centreY - armSize),
				Point(centreX + 1, centreY - armSize),
				Point(centreX + 1, centreY - 1),
				Point(centreX + armSize, centreY - 1),
				Point(centreX + armSize, centreY + 1),
				Point(centreX + 1, centreY + 1),
				Point(centreX + 1, centreY + armSize),
				Point(centreX - 1, centreY + armSize),
				Point(centreX - 1, centreY + 1),
				Point(centreX - armSize, centreY + 1),
			};
			AlignedPolygon(surface, pts, std::size(pts));
		}
		break;

	case SC_MARK_MINUS: {
			Point pts[] = {
				Point(centreX - armSize, centreY - 1),
				Point(centreX + armSize, centreY - 1),
				Point(centreX + armSize, centreY + 1),
				Point(centreX - armSize, centreY + 1),
			};
			AlignedPolygon(surface, pts, std::size(pts));
		}
		break;

	case SC_MARK_SMALLRECT: {
			PRectangle rcSmall;
			rcSmall.left = rc.left + 1;
			rcSmall.top = rc.top + 2;
			rcSmall.right = rc.right - 1;
			rcSmall.bottom = rc.bottom - 2;
			surface->RectangleDraw(rcSmall, FillStroke(back, fore, strokeWidth));
		}
		break;

	case SC_MARK_EMPTY:
	case SC_MARK_BACKGROUND:
	case SC_MARK_UNDERLINE:
	case SC_MARK_AVAILABLE:
		// An invisible marker so don't draw anything
		break;

	case SC_MARK_DOTDOTDOT: {
			XYPOSITION right = static_cast<XYPOSITION>(centreX - 6);
			for (int b = 0; b < 3; b++) {
				const PRectangle rcBlob(right, rc.bottom - 4, right + 2, rc.bottom - 2);
				surface->FillRectangle(rcBlob, fore);
				right += 5.0f;
			}
		}
		break;

	case SC_MARK_ARROWS: {
			XYPOSITION right = centreX - 4.0f + strokeWidth / 2.0f;
			const XYPOSITION midY = centreY + strokeWidth / 2.0f;
			const XYPOSITION armLength = std::round(dimOn2 - strokeWidth);
			for (int b = 0; b < 3; b++) {
				const Point pts[] = {
					Point(right - armLength, midY - armLength),
					Point(right, midY),
					Point(right - armLength, midY + armLength)
				};
				surface->PolyLine(pts, std::size(pts), Stroke(fore, strokeWidth));
				right += strokeWidth + 3.0f;
			}
		}
		break;

	case SC_MARK_SHORTARROW: {
			Point pts[] = {
				Point(centreX, centreY + dimOn2),
				Point(centreX + dimOn2, centreY),
				Point(centreX, centreY - dimOn2),
				Point(centreX, centreY - dimOn4),
				Point(centreX - dimOn4, centreY - dimOn4),
				Point(centreX - dimOn4, centreY + dimOn4),
				Point(centreX, centreY + dimOn4),
				Point(centreX, centreY + dimOn2),
			};
			AlignedPolygon(surface, pts, std::size(pts));
		}
		break;

	case SC_MARK_FULLRECT:
		surface->FillRectangle(rcWhole, back);
		break;

	case SC_MARK_LEFTRECT: {
			PRectangle rcLeft = rcWhole;
			rcLeft.right = rcLeft.left + 4;
			surface->FillRectangle(rcLeft, back);
		}
		break;

	case SC_MARK_BOOKMARK: {
			const XYPOSITION halfHeight = std::floor(minDim / 3);
			Point pts[] = {
				Point(rcWhole.left, centreY - halfHeight),
				Point(rcWhole.right - strokeWidth - 2, centreY - halfHeight),
				Point(rcWhole.right - strokeWidth - 2 - halfHeight, centreY),
				Point(rcWhole.right - strokeWidth - 2, centreY + halfHeight),
				Point(rcWhole.left, centreY + halfHeight),
			};
			AlignedPolygon(surface, pts, std::size(pts));
		}
		break;

	case SC_MARK_VERTICALBOOKMARK: {
			const XYPOSITION halfWidth = std::floor(minDim / 3);
			Point pts[] = {
				Point(centreX - halfWidth, centreY - dimOn2),
				Point(centreX + halfWidth, centreY - dimOn2),
				Point(centreX + halfWidth, centreY + dimOn2),
				Point(centreX, centreY + dimOn2 - halfWidth),
				Point(centreX - halfWidth, centreY + dimOn2),
			};
			AlignedPolygon(surface, pts, std::size(pts));
		}
		break;

	default:
		if (markType >= SC_MARK_CHARACTER) {
			char character[UTF8MaxBytes + 1];
			UTF8FromUTF32Character(markType - SC_MARK_CHARACTER, character);
			const XYPOSITION width = surface->WidthTextUTF8(fontForCharacter, character);
			PRectangle rcText = rc;
			rcText.left += (rc.Width() - width) / 2;
			rcText.right = rcText.left + width;
			surface->DrawTextNoClipUTF8(rcText, fontForCharacter, rcText.bottom - 2,
						 character, fore, back);
		} else {
			// treat as SC_MARK_FULLRECT
			surface->FillRectangle(rcWhole, back);
		}
		break;
	}
}
