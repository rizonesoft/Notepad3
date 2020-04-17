// sktoolslib - common files for SK tools

// Copyright (C) 2013, 2016 Stefan Kueng

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
#include <string>

/**
 * Loads a string from the application resources.
 */

inline std::wstring LoadResourceWString(HINSTANCE hInstance, UINT uID)
{
    const wchar_t* p = nullptr;
    int len = ::LoadStringW(hInstance, uID, reinterpret_cast<LPWSTR>(&p), 0);
    return (len > 0) ? std::wstring(p, static_cast<size_t>(len)) : std::wstring();
}

class ResString
{
public:
    // hInst is necessary to support multiple languages with resource dlls
    inline ResString (HINSTANCE hInst, int resId)
        : str(LoadResourceWString(hInst, resId))
    {
    }
    // Handy for when used with printf etc and to avoid explicit casts.
    inline const TCHAR* c_str() const { return str.c_str(); }
    inline operator const TCHAR*const () const { return str.c_str(); }
    inline operator const std::wstring& () const { return str; }
private:
    const std::wstring str;
};
