// sktoolslib - common files for SK tools

// Copyright (C) 2014, 2020 - Stefan Kueng

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

/**
 * Helper macro to execute code when the object goes out of scope. Very
 * useful to clean up in an exception safe way.
 *
 * Example:
 * \code
 * CoInitialize();
 * OnOutOfScope(CoUninitialize(););
 * // more code
 * // at the end of this scope, CoUninitialize(); is called
 **/

#define AUTOOUTOFSCOPE_TOKEN_PASTEx(x, y) x##y
#define AUTOOUTOFSCOPE_TOKEN_PASTE(x, y)  AUTOOUTOFSCOPE_TOKEN_PASTEx(x, y)

template <typename T>
class AutoOutOfScope
{
public:
    AutoOutOfScope(T& destructor)
        : m_destructor(destructor)
    {
    }
    ~AutoOutOfScope() { m_destructor(); }
    // no copies of this class, also to avoid compiler warnings
    AutoOutOfScope(const AutoOutOfScope&) = delete;
    AutoOutOfScope& operator=(const AutoOutOfScope& tmp) = delete;

private:
    T& m_destructor;
};

#define AUTOOUTOFSCOPE__INTERNAL(Destructor, counter)                                                                                                  \
    auto                                                                      AUTOOUTOFSCOPE_TOKEN_PASTE(auto_func_, counter) = [&]() { Destructor; }; \
    AutoOutOfScope<decltype(AUTOOUTOFSCOPE_TOKEN_PASTE(auto_func_, counter))> AUTOOUTOFSCOPE_TOKEN_PASTE(auto_, counter)(AUTOOUTOFSCOPE_TOKEN_PASTE(auto_func_, counter));

#define OnOutOfScope(Destructor) AUTOOUTOFSCOPE__INTERNAL(Destructor, __COUNTER__)
