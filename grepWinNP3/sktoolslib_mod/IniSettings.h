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
#include "SimpleIni.h"

class CIniSettings
{
private:
    CIniSettings(void);
    ~CIniSettings(void);
public:
    static CIniSettings&    Instance();

    void                    SetIniPath(const std::wstring& p);
    std::wstring            GetIniPath() const { return m_iniPath; }
    void                    Save();
    void                    Reload();

    LPCWSTR                 GetString(LPCWSTR section, LPCWSTR key, LPCWSTR default = nullptr);
    void                    SetString(LPCWSTR section, LPCWSTR key, LPCWSTR value);
    __int64                 GetInt64(LPCWSTR section, LPCWSTR key, __int64 default);
    void                    SetInt64(LPCWSTR section, LPCWSTR key, __int64 value);
    void                    Delete(LPCWSTR section, LPCWSTR key);
    void                    RestoreWindowPos(LPCWSTR windowname, HWND hWnd, UINT showCmd);
    void                    SaveWindowPos(LPCWSTR windowname, HWND hWnd);

private:
    CSimpleIni              m_IniFile;

    std::wstring            m_iniPath;
};

