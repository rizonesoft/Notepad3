// Scintilla source code edit control
/** @file Indicator.cxx
 ** Defines the style of indicators which are text decorations such as underlining.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <cmath>

#include <stdexcept>
#include <string_view>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <memory>

#include "Debugging.h"
#include "Geometry.h"
#include "Platform.h"

#include "Scintilla.h"
#include "Indicator.h"
#include "XPM.h"

using namespace Scintilla;

void Indicator::Draw(Surface *surface, const PRectangle &rc, const PRectangle &rcLine, const PRectangle &rcCharacter, State state, int value) const {
	StyleAndColour sacDraw = sacNormal;
	if (Flags() & SC_INDICFLAG_VALUEFORE) {
		sacDraw.fore = ColourDesired(value & SC_INDICVALUEMASK);
	}
	if (state == State::hover) {
		sacDraw = sacHover;
	}

	const int pixelDivisions = surface->PixelDivisions();

	const XYPOSITION halfWidth = strokeWidth / 2.0f;

	const PRectangle rcAligned(PixelAlignOutside(rc, pixelDivisions));
	PRectangle rcFullHeightAligned = PixelAlignOutside(rcLine, pixelDivisions);
	rcFullHeightAligned.left = rcAligned.left;
	rcFullHeightAligned.right = rcAligned.right;

	const XYPOSITION ymid = PixelAlign(rc.Centre().y, pixelDivisions);

	// This is a reasonable clip for indicators beneath text like underlines 
	PRectangle rcClip = rcAligned;
	rcClip.bottom = rcFullHeightAligned.bottom;

	switch (sacDraw.style) {
	case INDIC_SQUIGGLE: {
			surface->SetClip(rcClip);
			XYPOSITION x = rcAligned.left + halfWidth;
			const XYPOSITION top = rcAligned.top + halfWidth;
			const XYPOSITION xLast = rcAligned.right + halfWidth;
			XYPOSITION y = 0;
			std::vector<Point> pts;
			const XYPOSITION pitch = 1 + strokeWidth;
			pts.emplace_back(x, top + y);
			while (x < xLast) {
				x += pitch;
				y = pitch - y;
				pts.emplace_back(x, top + y);
				}
			surface->PolyLine(pts.data(), std::size(pts), Stroke(sacDraw.fore, strokeWidth));
			surface->PopClip();
		}
		break;

	case INDIC_SQUIGGLEPIXMAP: {
			const PRectangle rcSquiggle = PixelAlign(rc, 1);

			const int width = std::min(4000, static_cast<int>(rcSquiggle.Width()));
			RGBAImage image(width, 3, 1.0, nullptr);
			enum { alphaFull = 0xff, alphaSide = 0x2f, alphaSide2=0x5f };
			for (int x = 0; x < width; x++) {
				if (x%2) {
					// Two halfway columns have a full pixel in middle flanked by light pixels
					image.SetPixel(x, 0, sacDraw.fore, alphaSide);
					image.SetPixel(x, 1, sacDraw.fore, alphaFull);
					image.SetPixel(x, 2, sacDraw.fore, alphaSide);
				} else {
					// Extreme columns have a full pixel at bottom or top and a mid-tone pixel in centre
					image.SetPixel(x, (x % 4) ? 0 : 2, sacDraw.fore, alphaFull);
					image.SetPixel(x, 1, sacDraw.fore, alphaSide2);
				}
			}
			surface->DrawRGBAImage(rcSquiggle, image.GetWidth(), image.GetHeight(), image.Pixels());
		}
		break;

	case INDIC_SQUIGGLELOW: {
			std::vector<Point> pts;
			const XYPOSITION top = rcAligned.top + halfWidth;
			int y = 0;
			XYPOSITION x = std::round(rcAligned.left) + halfWidth;
			pts.emplace_back(x, top + y);
			const XYPOSITION pitch = 2 + strokeWidth;
			x += pitch;
			while (x < rcAligned.right) {
				pts.emplace_back(x - 1, top + y);
				y = 1 - y;
				pts.emplace_back(x, top + y);
				x += pitch;
			}
			pts.emplace_back(rcAligned.right, top + y);
			surface->PolyLine(pts.data(), std::size(pts), Stroke(sacDraw.fore, strokeWidth));
		}
		break;

	case INDIC_TT: {
			surface->SetClip(rcClip);
			const XYPOSITION yLine = ymid;
			XYPOSITION x = rcAligned.left + 5.0f;
			const XYPOSITION pitch = 4 + strokeWidth;
			while (x < rc.right + pitch) {
				const PRectangle line(x-pitch, yLine, x, yLine + strokeWidth);
				surface->FillRectangle(line, sacDraw.fore);
				const PRectangle tail(x - 2 - strokeWidth, yLine + strokeWidth, x - 2, yLine + strokeWidth * 2);
				surface->FillRectangle(tail, sacDraw.fore);
				x++;
				x += pitch;
			}
			surface->PopClip();
		}
		break;

	case INDIC_DIAGONAL: {
			surface->SetClip(rcClip);
			XYPOSITION x = rcAligned.left + halfWidth;
			const XYPOSITION top = rcAligned.top + halfWidth;
			const XYPOSITION pitch = 3 + strokeWidth;
			while (x < rc.right) {
				const XYPOSITION endX = x+3;
				const XYPOSITION endY = top - 1;
				surface->LineDraw(Point(x, top + 2), Point(endX, endY), Stroke(sacDraw.fore, strokeWidth));
				x += pitch;
			}
			surface->PopClip();
		}
		break;

	case INDIC_STRIKE: {
			const XYPOSITION yStrike = std::round(rcLine.Centre().y);
			const PRectangle rcStrike(
				rcAligned.left, yStrike, rcAligned.right, yStrike + strokeWidth);
			surface->FillRectangle(rcStrike, sacDraw.fore);
		}
		break;

	case INDIC_HIDDEN:
	case INDIC_TEXTFORE:
		// Draw nothing
		break;

	case INDIC_BOX: {
			PRectangle rcBox = rcFullHeightAligned;
			rcBox.top = rcBox.top + 1.0f;
			rcBox.bottom = ymid + 1.0f;
			surface->RectangleFrame(rcBox, Stroke(ColourAlpha(sacDraw.fore, outlineAlpha), strokeWidth));
		}
		break;

	case INDIC_ROUNDBOX:
	case INDIC_STRAIGHTBOX:
	case INDIC_FULLBOX: {
			PRectangle rcBox = rcFullHeightAligned;
			if (sacDraw.style != INDIC_FULLBOX)
				rcBox.top = rcBox.top + 1;
			surface->AlphaRectangle(rcBox, (sacDraw.style == INDIC_ROUNDBOX) ? 1.0f : 0.0f,
						FillStroke(ColourAlpha(sacDraw.fore, fillAlpha), ColourAlpha(sacDraw.fore, outlineAlpha), strokeWidth));
		}
		break;

	case INDIC_GRADIENT:
	case INDIC_GRADIENTCENTRE: {
			PRectangle rcBox = rcFullHeightAligned;
			rcBox.top = rcBox.top + 1;
			const Surface::GradientOptions options = Surface::GradientOptions::topToBottom;
			const ColourAlpha start(sacDraw.fore, fillAlpha);
			const ColourAlpha end(sacDraw.fore, 0);
			std::vector<ColourStop> stops;
			switch (sacDraw.style) {
			case INDIC_GRADIENT:
				stops.emplace_back(0.0f, start);
				stops.emplace_back(1.0f, end);
				break;
			case INDIC_GRADIENTCENTRE:
				stops.emplace_back(0.0f, end);
				stops.emplace_back(0.5f, start);
				stops.emplace_back(1.0f, end);
				break;
			}
			surface->GradientRectangle(rcBox, stops, options);
		}
		break;

	case INDIC_DOTBOX: {
			PRectangle rcBox = rcFullHeightAligned;
			rcBox.top = rcBox.top + 1;
			// Cap width at 4000 to avoid large allocations when mistakes made
			const int width = std::min(static_cast<int>(rcBox.Width()), 4000);
			const int height = static_cast<int>(rcBox.Height());
			RGBAImage image(width, height, 1.0, nullptr);
			// Draw horizontal lines top and bottom
			for (int x=0; x<width; x++) {
				for (int y = 0; y< height; y += height - 1) {
					image.SetPixel(x, y, sacDraw.fore, ((x + y) % 2) ? outlineAlpha : fillAlpha);
				}
			}
			// Draw vertical lines left and right
			for (int y = 1; y<height; y++) {
				for (int x=0; x<width; x += width-1) {
					image.SetPixel(x, y, sacDraw.fore, ((x + y) % 2) ? outlineAlpha : fillAlpha);
				}
			}
			surface->DrawRGBAImage(rcBox, image.GetWidth(), image.GetHeight(), image.Pixels());
		}
		break;

	case INDIC_DASH: {
			XYPOSITION x = std::floor(rc.left);
			const XYPOSITION widthDash = 3 + std::round(strokeWidth);
			while (x < rc.right) {
				const PRectangle rcDash = PRectangle(x, ymid,
					x + widthDash, ymid + std::round(strokeWidth));
				surface->FillRectangle(rcDash, sacDraw.fore);
				x += 3 + widthDash;
			}
		}
		break;

	case INDIC_DOTS: {
			const XYPOSITION widthDot = std::round(strokeWidth);
			XYPOSITION x = std::floor(rc.left);
			while (x < rc.right) {
				const PRectangle rcDot = PRectangle(x, ymid, 
					x + widthDot, ymid + widthDot);
				surface->FillRectangle(rcDot, sacDraw.fore);
				x += widthDot * 2;
			}
		}
		break;

	case INDIC_COMPOSITIONTHICK: {
			const PRectangle rcComposition(rc.left+1, rcLine.bottom-2, rc.right-1, rcLine.bottom);
			surface->FillRectangle(rcComposition, sacDraw.fore);
		}
		break;

	case INDIC_COMPOSITIONTHIN: {
			const PRectangle rcComposition(rc.left+1, rcLine.bottom-2, rc.right-1, rcLine.bottom-1);
			surface->FillRectangle(rcComposition, sacDraw.fore);
		}
		break;

	case INDIC_POINT:
	case INDIC_POINTCHARACTER:
		if (rcCharacter.Width() >= 0.1) {
			const XYPOSITION pixelHeight = std::floor(rc.Height() - 1.0f);	// 1 pixel onto next line if multiphase
			const XYPOSITION x = (sacDraw.style == INDIC_POINT) ? (rcCharacter.left) : ((rcCharacter.right + rcCharacter.left) / 2);
			// 0.5f is to hit midpoint of pixels:
			const XYPOSITION ix = std::round(x) + 0.5f;
			const XYPOSITION iy = std::floor(rc.top + 1.0f) + 0.5f;
			const Point pts[] = {
				Point(ix - pixelHeight, iy + pixelHeight),	// Left
				Point(ix + pixelHeight, iy + pixelHeight),	// Right
				Point(ix, iy)								// Top
			};
			surface->Polygon(pts, std::size(pts), FillStroke(sacDraw.fore));
		}
		break;

	default:
		// Either INDIC_PLAIN or unknown
		surface->FillRectangle(PRectangle(rcAligned.left, ymid,
			rcAligned.right, ymid + std::round(strokeWidth)), sacDraw.fore);
	}
}

void Indicator::SetFlags(int attributes_) noexcept {
	attributes = attributes_;
}
