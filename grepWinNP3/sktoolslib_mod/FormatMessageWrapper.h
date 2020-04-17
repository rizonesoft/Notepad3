// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2017 - Stefan Kueng

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
 * A wrapper class for calling the FormatMessage() Win32 function and controlling
 * the lifetime of the allocated error message buffer.
 */

class CFormatMessageWrapper
{
private:
    LPTSTR buffer;
    DWORD result;
    void release();
    void obtainMessage() { obtainMessage(::GetLastError()); }
    void obtainMessage(DWORD errorCode);

public:
    CFormatMessageWrapper() : buffer(nullptr), result(0) { obtainMessage(); }
    CFormatMessageWrapper(DWORD lastError) : buffer(nullptr), result(0) { obtainMessage(lastError); }
    ~CFormatMessageWrapper() { release(); }
    operator LPCTSTR() const { return buffer; }
    operator bool() const { return result != 0; }
    bool operator!() const { return result == 0; }
    LPCTSTR c_str() const { return buffer; }
};

inline void CFormatMessageWrapper::obtainMessage(DWORD errorCode)
{
    // First of all release the buffer to make it possible to call this
    // method more than once on the same object.
    release();
    result = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           nullptr,
                           errorCode,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                           (LPTSTR)&buffer,
                           0,
                           nullptr
    );
}

inline void CFormatMessageWrapper::release()
{
    if (buffer != 0)
    {
        LocalFree(buffer);
        buffer = 0;
    }

    result = 0;
}
