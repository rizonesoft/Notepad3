// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2017, 2020-2021 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "stdafx.h"
#include "GDIHelpers.h"
#include <cassert>

enum
{
    AlphaShift = 24,
    RedShift   = 16,
    GreenShift = 8,
    BlueShift  = 0
};

COLORREF GDIHelpers::Darker(COLORREF crBase, float fFactor)
{
    assert(fFactor <= 1.0f && fFactor > 0.0f);

    fFactor = min(fFactor, 1.0f);
    fFactor = max(fFactor, 0.0f);

    const BYTE bRed   = GetRValue(crBase);
    const BYTE bBlue  = GetBValue(crBase);
    const BYTE bGreen = GetGValue(crBase);

    const BYTE bRedShadow   = static_cast<BYTE>(bRed * fFactor);
    const BYTE bBlueShadow  = static_cast<BYTE>(bBlue * fFactor);
    const BYTE bGreenShadow = static_cast<BYTE>(bGreen * fFactor);

    return RGB(bRedShadow, bGreenShadow, bBlueShadow);
}

COLORREF GDIHelpers::Lighter(COLORREF crBase, float fFactor)
{
    assert(fFactor > 1.0f);

    fFactor = max(fFactor, 1.0f);

    const BYTE bRed   = GetRValue(crBase);
    const BYTE bBlue  = GetBValue(crBase);
    const BYTE bGreen = GetGValue(crBase);

    const BYTE bRedHilite   = static_cast<BYTE>(min((int)(bRed * fFactor), 255));
    const BYTE bBlueHilite  = static_cast<BYTE>(min((int)(bBlue * fFactor), 255));
    const BYTE bGreenHilite = static_cast<BYTE>(min((int)(bGreen * fFactor), 255));

    return RGB(bRedHilite, bGreenHilite, bBlueHilite);
}

void GDIHelpers::FillSolidRect(HDC hDC, int left, int top, int right, int bottom, COLORREF clr)
{
    ::SetBkColor(hDC, clr);
    RECT rect;
    rect.left   = left;
    rect.top    = top;
    rect.right  = right;
    rect.bottom = bottom;
    ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr);
}

void GDIHelpers::FillSolidRect(HDC hDC, const RECT* rc, COLORREF clr)
{
    ::SetBkColor(hDC, clr);
    ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, rc, nullptr, 0, nullptr);
}

Gdiplus::ARGB GDIHelpers::MakeARGB(IN BYTE a, IN BYTE r, IN BYTE g, IN BYTE b)
{
    return ((static_cast<Gdiplus::ARGB>(b) << BlueShift) |
            (static_cast<Gdiplus::ARGB>(g) << GreenShift) |
            (static_cast<Gdiplus::ARGB>(r) << RedShift) |
            (static_cast<Gdiplus::ARGB>(a) << AlphaShift));
}

COLORREF GDIHelpers::InterpolateColors(COLORREF c1, COLORREF c2, double fraction)
{
    assert(fraction >= 0.0 && fraction <= 1.0);
    int  r1  = static_cast<int>(GetRValue(c1));
    int  g1  = static_cast<int>(GetGValue(c1));
    int  b1  = static_cast<int>(GetBValue(c1));
    int  r2  = static_cast<int>(GetRValue(c2));
    int  g2  = static_cast<int>(GetGValue(c2));
    int  b2  = static_cast<int>(GetBValue(c2));
    auto clr = RGB((r2 - r1) * fraction + r1, (g2 - g1) * fraction + g1, (b2 - b1) * fraction + b1);

    return clr;
}

void GDIHelpers::RGBToHSB(COLORREF rgb, BYTE& hue, BYTE& saturation, BYTE& brightness)
{
    BYTE   r      = GetRValue(rgb);
    BYTE   g      = GetGValue(rgb);
    BYTE   b      = GetBValue(rgb);
    BYTE   minRGB = min(min(r, g), b);
    BYTE   maxRGB = max(max(r, g), b);
    BYTE   delta  = maxRGB - minRGB;
    double l      = maxRGB;
    double s      = 0.0;
    double h      = 0.0;
    if (maxRGB == 0)
    {
        hue        = 0;
        saturation = 0;
        brightness = 0;
        return;
    }
    if (maxRGB)
        s = (255.0 * delta) / maxRGB;

    if (static_cast<BYTE>(s) != 0)
    {
        if (r == maxRGB)
            h = 0 + 43 * static_cast<double>(g - b) / delta;
        else if (g == maxRGB)
            h = 85 + 43 * static_cast<double>(b - r) / delta;
        else if (b == maxRGB)
            h = 171 + 43 * static_cast<double>(r - g) / delta;
    }
    else
        h = 0.0;

    hue        = static_cast<BYTE>(h);
    saturation = static_cast<BYTE>(s);
    brightness = static_cast<BYTE>(l);
}

void GDIHelpers::RGBtoHSL(COLORREF color, float& h, float& s, float& l)
{
    const float rPercent = static_cast<float>(GetRValue(color)) / 255;
    const float gPercent = static_cast<float>(GetGValue(color)) / 255;
    const float bPercent = static_cast<float>(GetBValue(color)) / 255;

    float maxColor = 0;
    if ((rPercent >= gPercent) && (rPercent >= bPercent))
        maxColor = rPercent;
    else if ((gPercent >= rPercent) && (gPercent >= bPercent))
        maxColor = gPercent;
    else if ((bPercent >= rPercent) && (bPercent >= gPercent))
        maxColor = bPercent;

    float minColor = 0;
    if ((rPercent <= gPercent) && (rPercent <= bPercent))
        minColor = rPercent;
    else if ((gPercent <= rPercent) && (gPercent <= bPercent))
        minColor = gPercent;
    else if ((bPercent <= rPercent) && (bPercent <= gPercent))
        minColor = bPercent;

    float L = 0, S = 0, H = 0;

    L = (maxColor + minColor) / 2;

    if (maxColor == minColor)
    {
        S = 0;
        H = 0;
    }
    else
    {
        auto d = maxColor - minColor;
        if (L < .50)
            S = d / (maxColor + minColor);
        else
            S = d / ((2.0f - maxColor) - minColor);

        if (maxColor == rPercent)
            H = (gPercent - bPercent) / d;

        else if (maxColor == gPercent)
            H = 2.0f + (bPercent - rPercent) / d;

        else if (maxColor == bPercent)
            H = 4.0f + (rPercent - gPercent) / d;
    }
    H = H * 60;
    if (H < 0)
        H += 360;
    s = S * 100;
    l = L * 100;
    h = H;
}

static float hsLtoRGBSubfunction(float temp1, float temp2, float temp3)
{
    if ((temp3 * 6) < 1)
        return (temp2 + (temp1 - temp2) * 6 * temp3) * 100;
    else if ((temp3 * 2) < 1)
        return temp1 * 100;
    else if ((temp3 * 3) < 2)
        return (temp2 + (temp1 - temp2) * (.66666f - temp3) * 6) * 100;
    else
        return temp2 * 100;
}

COLORREF GDIHelpers::HSLtoRGB(float h, float s, float l)
{
    if (s == 0)
    {
        BYTE t = static_cast<BYTE>(l / 100 * 255);
        return RGB(t, t, t);
    }
    const float L     = l / 100;
    const float S     = s / 100;
    const float H     = h / 360;
    const float temp1 = (L < .50) ? L * (1 + S) : L + S - (L * S);
    const float temp2 = 2 * L - temp1;
    float       temp3 = 0;
    temp3             = H + .33333f;
    if (temp3 > 1)
        temp3 -= 1;
    const float pcr = hsLtoRGBSubfunction(temp1, temp2, temp3);
    temp3           = H;
    const float pcg = hsLtoRGBSubfunction(temp1, temp2, temp3);
    temp3           = H - .33333f;
    if (temp3 < 0)
        temp3 += 1;
    const float pcb = hsLtoRGBSubfunction(temp1, temp2, temp3);
    BYTE        r   = static_cast<BYTE>(pcr / 100 * 255);
    BYTE        g   = static_cast<BYTE>(pcg / 100 * 255);
    BYTE        b   = static_cast<BYTE>(pcb / 100 * 255);
    return RGB(r, g, b);
}

bool GDIHelpers::ShortHexStringToCOLORREF(const std::string& s, COLORREF* clr)
{
    if (s.length() != 3)
        return false;
    BYTE rgb[3]; // [0]=red, [1]=green, [2]=blue
    char dig[2];
    dig[1] = '\0';
    for (int i = 0; i < 3; ++i)
    {
        dig[0]   = s[i];
        BYTE& v  = rgb[i];
        char* ep = nullptr;
        errno    = 0;
        // Must convert all digits of string.
        v = static_cast<BYTE>(strtoul(dig, &ep, 16));
        if (errno == 0 && ep == &dig[1])
            v = v * 16 + v;
        else
        {
            *clr = 0;
            return false;
        }
    }
    auto color = RGB(rgb[0], rgb[1], rgb[2]);
    *clr       = color;
    return true;
}

bool GDIHelpers::HexStringToCOLORREF(const std::string& s, COLORREF* clr)
{
    if ((s.length() != 6) && (s.length() != 8))
        return false;
    char* ep        = nullptr;
    errno           = 0;
    unsigned long v = strtoul(s.c_str(), &ep, 16);
    // Must convert all digits of string.
    if (errno == 0 && ((ep == &s[6]) || (ep == &s[8])))
    {
        BYTE r = (v >> 16) & 0xFF;
        BYTE g = (v >> 8) & 0xFF;
        BYTE b = v & 0xFF;
        *clr   = RGB(r, g, b) | (v & 0xFF000000);
        return true;
    }
    *clr = RGB(0, 0, 0);
    return false;
}

bool GDIHelpers::LongHexStringToCOLORREF(const std::string& s, COLORREF* clr)
{
    if (s.length() != 8)
        return false;
    char* ep        = nullptr;
    errno           = 0;
    unsigned long v = strtoul(s.c_str(), &ep, 16);
    // Must convert all digits of string.
    if (errno == 0 && ep == &s[8])
    {
        BYTE b = (v >> 16) & 0xFF;
        BYTE g = (v >> 8) & 0xFF;
        BYTE r = v & 0xFF;
        *clr   = RGB(r, g, b) | (v & 0xFF000000);
        return true;
    }
    *clr = RGB(0, 0, 0);
    return false;
}

bool GDIHelpers::HexStringToCOLORREF(const std::wstring& s, COLORREF* clr)
{
    if (s.length() != 6)
        return false;
    wchar_t* ep     = nullptr;
    errno           = 0;
    unsigned long v = wcstoul(s.c_str(), &ep, 16);
    if (errno == 0 && ep == &s[6])
    {
        BYTE r = (v >> 16) & 0xFF;
        BYTE g = (v >> 8) & 0xFF;
        BYTE b = v & 0xFF;
        *clr   = RGB(r, g, b) | (v & 0xFF000000);
        return true;
    }
    *clr = RGB(0, 0, 0);
    return false;
}
