// encoding: UTF-8
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "ChooseFont.h"
#include "FontEnumeration.h"


/******************************************************************
*                                                                 *
* GetLocalizedName                                                *
*                                                                 *
* Select a name from a list of localized names and use a simple   *
* fallback scheme if the desired locale isn't supported.           *
*                                                                 *
******************************************************************/

HRESULT GetLocalizedName(IDWriteLocalizedStrings* names, const WCHAR* locale, OUT std::wstring& familyName)
{
    HRESULT hr = S_OK;

    UINT32 nameIndex;
    BOOL   nameExists;
    UINT32 nameLength = 0;

    try
    {
        hr = names->FindLocaleName(locale, &nameIndex, &nameExists);

        // If there is no name with the desired locale fallback to US English.
        if (SUCCEEDED(hr))
        {
            if (!nameExists) {
              hr = names->FindLocaleName(L"en-US", &nameIndex, &nameExists);
            }
            // If the name still doesn't exist just take the first one.
            if (!nameExists) { 
              nameIndex = 0; 
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = names->GetStringLength(nameIndex, &nameLength);
        }

        if (SUCCEEDED(hr))
        {
            familyName.resize(static_cast<size_t>(nameLength + 1));
            hr = names->GetString(nameIndex, &familyName[0], static_cast<UINT32>(familyName.size()));
        }
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }
    return hr;
}


/******************************************************************
*                                                                 *
* GetFontFamilyName                                               *
*                                                                 *
* Get the family name (e.g. "Arial" in "Arial Bold") for a font   *
* family.                                                         *
*                                                                 *
******************************************************************/

HRESULT GetFontFamilyName(IDWriteFontFamily* fontFamily, const WCHAR* locale, OUT std::wstring& familyName)
{
    IDWriteLocalizedStrings* familyNames = nullptr;

    HRESULT hr = fontFamily->GetFamilyNames(&familyNames);

    if (SUCCEEDED(hr))
    {
        hr = GetLocalizedName(familyNames, locale, familyName);
    }

    SafeRelease(&familyNames);

    return hr;
}


/******************************************************************
*                                                                 *
* GetFontFaceName                                                 *
*                                                                 *
* Get the face name (e.g. "Bold" in "Arial Bold") for a font.     *
*                                                                 *
******************************************************************/

HRESULT GetFontFaceName(IDWriteFont* font, const WCHAR* locale, OUT std::wstring& faceName)
{
    IDWriteLocalizedStrings* faceNames = nullptr;

    HRESULT hr = font->GetFaceNames(&faceNames);

    if (SUCCEEDED(hr))
    {
        hr = GetLocalizedName(faceNames, locale, faceName);
    }

    SafeRelease(&faceNames);

    return hr;
}


/******************************************************************
*                                                                 *
* GetFontFamily                                                   *
*                                                                 *
* Get the font family object given a family name.                 *
*                                                                 *
******************************************************************/

HRESULT GetFontFamily(IDWriteFontCollection* fontCollection, const WCHAR* fontFamilyName, OUT IDWriteFontFamily** fontFamily)
{
    HRESULT hr;

    UINT32  familyIndex;
    BOOL    familyExists;

    hr = fontCollection->FindFamilyName(fontFamilyName, &familyIndex, &familyExists);
    if (!familyExists)
        return DWRITE_E_NOFONT;

    if (SUCCEEDED(hr))
    {
        hr = fontCollection->GetFontFamily(familyIndex, fontFamily);
    }

    return hr;
}


/******************************************************************
*                                                                 *
* GetFonts                                                        *
*                                                                 *
* Return all of the fonts (e.g. Arial Regular, Arial Bold,        *
* Arial Narrow, etc.) in a font family (e.g. Arial)               *
*                                                                 *
******************************************************************/

HRESULT GetFonts(IDWriteFontCollection* fontCollection, const WCHAR* fontFamilyName, IN OUT std::vector<IDWriteFont*>& fonts)
{
    IDWriteFontFamily* fontFamily = nullptr;
    IDWriteFont*       font       = nullptr;

    HRESULT hr = GetFontFamily(fontCollection, fontFamilyName, &fontFamily);

    if (SUCCEEDED(hr))
    {
        UINT32 fontCount = fontFamily->GetFontCount();

        // Read font variant in the family.
        try
        {
            for (UINT32 i = 0; i != fontCount; ++i)
            {
                hr = fontFamily->GetFont(i, &font);
                if (FAILED(hr))
                    break;

                fonts.push_back(font);
                SafeDetach(&font);
            }
        }
        catch (...)
        {
            hr = ExceptionToHResult();
        }
    }

    SafeRelease(&font);
    SafeRelease(&fontFamily);

    return hr;
}


/******************************************************************
*                                                                 *
* GetFontFaceInfo                                                 *
*                                                                 *
* Get information about the font faces supported by a font family *
*                                                                 *
******************************************************************/

HRESULT GetFontFaceInfo(const std::vector<IDWriteFont*>& fonts, const WCHAR* locale, IN OUT std::vector<FontFaceInfo>& info)
{
    HRESULT hr = S_OK;

    // If there are no fonts in this family just returned standard harcoded choices

    try
    {
        if (fonts.empty())
        {
            info.emplace_back(L"Regular", DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL);
            info.emplace_back(L"Bold", DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL);
            info.emplace_back(L"Italic", DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL);
            info.emplace_back(L"Oblique", DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL);
            info.emplace_back(L"Bold Italic", DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL);
            info.emplace_back(L"Bold Oblique", DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL);
            return S_OK;
        }

        for (size_t i = 0; i != fonts.size(); ++i)
        {
            std::wstring faceName;
            hr = GetFontFaceName(fonts[i], locale, faceName);
            if (FAILED(hr))
                break;

            info.emplace_back(faceName.c_str(),
                                fonts[i]->GetWeight(),
                                fonts[i]->GetStyle(),
                                fonts[i]->GetStretch());
        }
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    return hr;
}


/******************************************************************
*                                                                 *
* GetFontFamilies                                                 *
*                                                                 *
* Get all the font families (e.g. Arial, Cambria, Tahoma, etc.)   *
* in a font collection.                                           *
*                                                                 *
******************************************************************/

HRESULT GetFontFamilyNames(IDWriteFontCollection* fontCollection, const WCHAR* localeName, IN OUT std::vector< std::wstring >& fontFamilyNames)
{
    HRESULT hr = S_OK;

    std::wstring familyName;
    
    UINT32  familyCount = fontCollection->GetFontFamilyCount();

    try
    {
        for (UINT32 i = 0; i != familyCount; ++i)
        {
            IDWriteFontFamily* fontFamily = nullptr;
            hr = fontCollection->GetFontFamily(i, &fontFamily);

            if (SUCCEEDED(hr))
            {
                hr = GetFontFamilyName(fontFamily, localeName, familyName);
            }
            SafeRelease(&fontFamily);

            if (FAILED(hr))
                break;

            fontFamilyNames.push_back(familyName);
        }
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    return hr;
}


/******************************************************************
*                                                                 *
* GetFontFamilyNameFromFormat                                     *
*                                                                 *
* Return the font family name from an IDWriteTextFormat object.   *
*                                                                 *
******************************************************************/

HRESULT GetFontFamilyNameFromFormat(IDWriteTextFormat* textFormat, OUT std::wstring& fontFamilyName)
{
    HRESULT hr = S_OK;

    UINT32 familyNameLength = textFormat->GetFontFamilyNameLength() + 1;

    try
    {
        fontFamilyName.resize(familyNameLength);

        hr = textFormat->GetFontFamilyName(&fontFamilyName[0], familyNameLength);
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    return hr;
}


/******************************************************************
*                                                                 *
* GetBestFontAttributes                                           *
*                                                                 *
* Find the font that best matches the desired font attributes     *
* and return the actual attributes.                               *
*                                                                 *
******************************************************************/

ULONG GetBestFontAttributes(IDWriteFontCollection* fontCollection, const WCHAR* fontFamilyName, const FontFaceInfo& desiredAttributes)
{
    DWRITE_FONT_WEIGHT  fontWeight  = desiredAttributes.fontWeight;
    DWRITE_FONT_STYLE   fontStyle   = desiredAttributes.fontStyle;
    DWRITE_FONT_STRETCH fontStretch = desiredAttributes.fontStretch;

    IDWriteFontFamily*  fontFamily = nullptr;
    IDWriteFont*        font = nullptr;

    if (SUCCEEDED(GetFontFamily(fontCollection, fontFamilyName, &fontFamily)))
    {
        if (SUCCEEDED(fontFamily->GetFirstMatchingFont(fontWeight, fontStretch, fontStyle, &font)))
        {
            fontWeight = font->GetWeight();
            fontStyle = font->GetStyle();
            fontStretch = font->GetStretch();
        }
    }

    SafeRelease(&font);
    SafeRelease(&fontFamily);

    FontFaceInfo fontFaceInfo(L"", fontWeight, fontStyle, fontStretch);

    return fontFaceInfo.PackedFontAttributes();
}
