// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2015, 2017, 2020-2021 - Stefan Kueng

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

#include "Registry.h"
#include <cassert>
#include <io.h>
#include <fcntl.h>

// A little like a release build assert. Always evaluate expr.
// Does not abort in this variant.
#define APPVERIFY(expr)            CTraceToOutputDebugString::Instance().Verify(expr, __FUNCTION__, __LINE__, (const char*)nullptr)
#define APPVERIFYM(expr, msg, ...) CTraceToOutputDebugString::Instance().Verify(expr, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__)

class CTraceToOutputDebugString
{
public:
    static CTraceToOutputDebugString& Instance()
    {
        if (m_pInstance == nullptr)
            m_pInstance = new CTraceToOutputDebugString;
        return *m_pInstance;
    }

    static bool Active()
    {
        return Instance().m_bActive;
    }

    bool SetLogFile(PCWSTR path)
    {
        auto handle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle == INVALID_HANDLE_VALUE)
            return false;

        // Associates a C run-time file descriptor with a file HANDLE.
        auto fd = _open_osfhandle(reinterpret_cast<intptr_t>(handle), _O_BINARY | _O_TEXT);
        if (fd == -1)
        {
            CloseHandle(handle);
            return false;
        }

        // Associates a stream with a C run-time file descriptor.
        m_fi = _fdopen(fd, "w+");
        if (m_fi != nullptr)
        {
            setvbuf(m_fi, nullptr, _IONBF, 0);
            return true;
        }
        return false;
    }
    // Non Unicode output helper
    void operator()(PCSTR pszFormat, ...) const
    {
        if (m_bActive)
        {
            va_list ptr;
            va_start(ptr, pszFormat);
            TraceV(pszFormat, ptr);
            va_end(ptr);
        }
    }

    // Unicode output helper
    void operator()(PCWSTR pszFormat, ...) const
    {
        if (m_bActive)
        {
            va_list ptr;
            va_start(ptr, pszFormat);
            TraceV(pszFormat, ptr);
            va_end(ptr);
        }
    }

    void Verify(bool expr, const char* function, int line, const char* msg, ...) const
    {
        if (!expr)
        {
            OutputDebugStringA(function);
            OutputDebugStringA(", line ");
            OutputDebugStringA(std::to_string(line).c_str());
            OutputDebugStringA(" : ");
            if (m_fi)
            {
                fputs(function, m_fi);
                fputs(", line ", m_fi);
                fputs(std::to_string(line).c_str(), m_fi);
                fputs(" : ", m_fi);
            }
            if (msg)
            {
                va_list ptr;
                va_start(ptr, msg);
                TraceV(msg, ptr);
                va_end(ptr);
            }
            else
            {
                OutputDebugStringA("An unexpected error occurred");
                if (m_fi)
                    fputs("An unexpected error occurred", m_fi);
            }
            OutputDebugStringA("\n");
            if (m_fi)
                fputs("\n", m_fi);
            // Verification failures are bugs so draw attention to them while debugging.
            assert(false);
        }
    }

    void Verify(bool expr, const char* function, int line, const wchar_t* msg, ...) const
    {
        if (!expr)
        {
            OutputDebugStringA(function);
            OutputDebugStringA(", line ");
            OutputDebugStringA(std::to_string(line).c_str());
            OutputDebugStringA(" : ");
            if (m_fi)
            {
                fputs(function, m_fi);
                fputs(", line ", m_fi);
                fputs(std::to_string(line).c_str(), m_fi);
                fputs(" : ", m_fi);
            }

            if (msg)
            {
                va_list ptr;
                va_start(ptr, msg);
                TraceV(msg, ptr);
                va_end(ptr);
            }
            else
            {
                OutputDebugStringA("An unexpected error occurred");
                if (m_fi)
                    fputs("An unexpected error occurred", m_fi);
            }
            OutputDebugStringA("\n");
            if (m_fi)
                fputs("\n", m_fi);
            // Verification failures are bugs so draw attention to them while debugging.
            assert(false);
        }
    }

private:
    CTraceToOutputDebugString()
        : m_fi(nullptr)
    {
        m_lastTick = GetTickCount64();
        m_bActive  = !!CRegStdDWORD(DEBUGOUTPUTREGPATH, FALSE);
    }
    ~CTraceToOutputDebugString()
    {
        delete m_pInstance;
    }

    ULONGLONG                         m_lastTick;
    bool                              m_bActive;
    FILE*                             m_fi;
    static CTraceToOutputDebugString* m_pInstance;

    // Non Unicode output helper
    void TraceV(PCSTR pszFormat, va_list args) const
    {
        // Format the output buffer
        char szBuffer[1024];
        _vsnprintf_s(szBuffer, _countof(szBuffer), pszFormat, args);
        OutputDebugStringA(szBuffer);
        if (m_fi)
            fputs(szBuffer, m_fi);
    }

    // Unicode output helper
    void TraceV(PCWSTR pszFormat, va_list args) const
    {
        wchar_t szBuffer[1024];
        _vsnwprintf_s(szBuffer, _countof(szBuffer), pszFormat, args);
        OutputDebugStringW(szBuffer);
        if (m_fi)
            fputws(szBuffer, m_fi);
    }

    bool IsActive()
    {
#ifdef DEBUG
        return true;
#else
        if (GetTickCount64() - m_lastTick > 10000)
        {
            m_lastTick = GetTickCount64();
            m_bActive  = !!CRegStdDWORD(DEBUGOUTPUTREGPATH, FALSE);
        }
        return m_bActive;
#endif
    }
};

class ProfileTimer
{
public:
    ProfileTimer(LPCWSTR text)
        : info(text)
    {
        QueryPerformanceCounter(&startTime);
    }
    ~ProfileTimer()
    {
        LARGE_INTEGER endTime;
        QueryPerformanceCounter(&endTime);
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        LARGE_INTEGER milliseconds;
        milliseconds.QuadPart = endTime.QuadPart - startTime.QuadPart;
        milliseconds.QuadPart *= 1000;
        milliseconds.QuadPart /= frequency.QuadPart;
        CTraceToOutputDebugString::Instance()(L"%s : %lld ms\n", info.c_str(), milliseconds.QuadPart);
    }

private:
    LARGE_INTEGER startTime;
    std::wstring  info;
};
