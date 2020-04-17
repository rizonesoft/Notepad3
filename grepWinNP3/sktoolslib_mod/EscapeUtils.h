// sktoolslib - common files for SK tools

// Copyright (C) 2013 - Stefan Kueng

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
#include <algorithm>
#include <functional>
#include <memory>


class CEscapeUtils
{
public:
    /**
     * Returns false if calling \ref Unescape is not necessary.
     */
    static bool ContainsEscapedChars(const char * psz, size_t length);

    /**
     * Replaces escaped sequences with the corresponding characters in a string.
     * \return Position of the terminating \0 char.
     */
    static char* Unescape(char * psz);

    /**
     * Replaces non-URI chars with the corresponding escape sequences.
     */
    static std::string EscapeString(const std::string& str);
    static std::wstring EscapeString(const std::wstring& str);

    /**
     * Replaces escaped sequences with the corresponding characters in a string.
     */
    static std::string StringUnescape(const std::string& path);
    static std::wstring StringUnescape(const std::wstring& path);

    static bool DoesPercentNeedEscaping(LPCSTR str);
};
