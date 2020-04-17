// sktoolslib - common files for SK tools

// Copyright (C) 2016-2017 - Stefan Kueng

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

#pragma once
#pragma warning(push)
#pragma warning(disable: 4458) // declaration of 'xxx' hides class member
#include <gdiplus.h>
#pragma warning(pop)

class GDIHelpers
{
public:
    static COLORREF         Darker(COLORREF crBase, float fFactor);
    static COLORREF         Lighter(COLORREF crBase, float fFactor);

    static void             FillSolidRect(HDC hDC, int left, int top, int right, int bottom, COLORREF clr);
    static void             FillSolidRect(HDC hDC, const RECT* rc, COLORREF clr);
    static Gdiplus::ARGB    MakeARGB(IN BYTE a, IN BYTE r, IN BYTE g, IN BYTE b);

    static COLORREF         InterpolateColors(COLORREF c1, COLORREF c2, double fraction);
    static void             RGBToHSB(COLORREF rgb, BYTE& hue, BYTE& saturation, BYTE& brightness);
    static void             RGBtoHSL(COLORREF color, float& h, float& s, float& l);
    static COLORREF         HSLtoRGB(float h, float s, float l);
    static bool             HexStringToCOLORREF(const std::string& s, COLORREF* clr);
    static bool             HexStringToCOLORREF(const std::wstring& s, COLORREF* clr);
    static bool             ShortHexStringToCOLORREF(const std::string& s, COLORREF* clr);
    static bool             LongHexStringToCOLORREF(const std::string& s, COLORREF* clr);
};