// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------
#pragma once

/******************************************************************
*                                                                 *
* FontFaceInfo                                                    *
*                                                                 *
* Helper structure that wraps up the common font attributes into  *
* single object.                                                  *
*                                                                 *
******************************************************************/

struct FontFaceInfo
{
    WCHAR               fontFaceName[100];
    DWRITE_FONT_WEIGHT  fontWeight;
    DWRITE_FONT_STYLE   fontStyle;
    DWRITE_FONT_STRETCH fontStretch;

    FontFaceInfo(
        const WCHAR* fontFaceName,
        DWRITE_FONT_WEIGHT   fontWeight, 
        DWRITE_FONT_STYLE    fontStyle, 
        DWRITE_FONT_STRETCH  fontStretch)
        :   fontWeight(fontWeight),
            fontStyle(fontStyle),
            fontStretch(fontStretch)
    {
        StringCchCopy(this->fontFaceName, ARRAYSIZE(this->fontFaceName), fontFaceName);
    }

    FontFaceInfo(
        const WCHAR* fontFaceName,
        ULONG packedAttributes)
        :   fontWeight((DWRITE_FONT_WEIGHT) LOWORD(packedAttributes)),
            fontStyle((DWRITE_FONT_STYLE) LOBYTE(HIWORD(packedAttributes))),
            fontStretch((DWRITE_FONT_STRETCH) HIBYTE(HIWORD(packedAttributes)))
    {
        StringCchCopy(this->fontFaceName, ARRAYSIZE(this->fontFaceName), fontFaceName);
    }

    ULONG PackedFontAttributes()
    {
        return MAKELONG(fontWeight, MAKEWORD(fontStyle, fontStretch));
    }
};


/******************************************************************
*                                                                 *
* Support routines used by the UI code in ChooseFont.cpp to do    *
* the actual enumeration and font information retrieval           *
*                                                                 *
******************************************************************/

HRESULT GetFontFamilyName(IDWriteFontFamily* fontFamily, const WCHAR* locale, OUT std::wstring& familyName);
HRESULT GetFontFamilyNames(IDWriteFontCollection* fontCollection, const WCHAR* localeName, IN OUT std::vector< std::wstring >& fontFamilyNames);
HRESULT GetFonts(IDWriteFontCollection* fontCollection, const WCHAR* fontFamilyName, IN OUT std::vector<IDWriteFont*>& fonts);
HRESULT GetFontFaceInfo(const std::vector<IDWriteFont*>& fonts, const WCHAR* locale, IN OUT std::vector<FontFaceInfo>& info);
HRESULT GetFontFamilyNameFromFormat(IDWriteTextFormat* textFormat, OUT std::wstring& fontFamilyName);
HRESULT GetFontFamily(IDWriteFontCollection* fontCollection, const WCHAR* fontFamilyName, OUT IDWriteFontFamily** fontFamily);
ULONG   GetBestFontAttributes(IDWriteFontCollection* fontCollection, const WCHAR* fontFamilyName, const FontFaceInfo& desiredAttributes);

