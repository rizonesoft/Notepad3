// Scintilla source code edit control
/** @file Style.h
 ** Defines the font and colour style for a class of text.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef STYLE_H
#define STYLE_H

namespace Scintilla {

struct FontSpecification {
	const char *fontName;
	int weight;
	bool italic;
	int size;
	int characterSet;
	int extraFontFlag;
	FontSpecification() noexcept :
		fontName(nullptr),
		weight(SC_WEIGHT_NORMAL),
		italic(false),
		size(10 * SC_FONT_SIZE_MULTIPLIER),
		characterSet(0),
		extraFontFlag(0) {
	}
	bool operator==(const FontSpecification &other) const noexcept;
	bool operator<(const FontSpecification &other) const noexcept;
};

struct FontMeasurements {
	unsigned int ascent;
	unsigned int descent;
	XYPOSITION capitalHeight;	// Top of capital letter to baseline: ascent - internal leading
	XYPOSITION aveCharWidth;
	XYPOSITION spaceWidth;
	int sizeZoomed;
	FontMeasurements() noexcept;
	void ClearMeasurements() noexcept;
};

/**
 */
class Style : public FontSpecification, public FontMeasurements {
public:
	ColourDesired fore;
	ColourDesired back;
	bool eolFilled;
	bool underline;
// >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
	bool strike;
// <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<
	enum class CaseForce {mixed, upper, lower, camel};
	CaseForce caseForce;
	bool visible;
	bool changeable;
	bool hotspot;

	std::shared_ptr<Font> font;

	Style();
	Style(const Style &source) noexcept;
	Style(Style &&) noexcept = default;
	~Style();
	Style &operator=(const Style &source) noexcept;
	Style &operator=(Style &&) = delete;
	void Clear(ColourDesired fore_, ColourDesired back_,
	           int size_,
	           const char *fontName_, int characterSet_,
	           int weight_, bool italic_, bool eolFilled_,
	           // >>>>>>>>>>>>>>>   BEG NON STD SCI PATCH   >>>>>>>>>>>>>>>
	           bool underline_, bool strike_, CaseForce caseForce_,
	           // <<<<<<<<<<<<<<<   END NON STD SCI PATCH   <<<<<<<<<<<<<<<
	           bool visible_, bool changeable_, bool hotspot_) noexcept;
	void ClearTo(const Style &source) noexcept;
	void Copy(std::shared_ptr<Font> font_, const FontMeasurements &fm_) noexcept;
	bool IsProtected() const noexcept { return !(changeable && visible);}
};

}

#endif
