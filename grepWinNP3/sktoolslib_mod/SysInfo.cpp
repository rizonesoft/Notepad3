// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2017 - Stefan Kueng

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
#include "SysInfo.h"

SysInfo::SysInfo(void)
    : isElevated(false)
    , isUACEnabled(false)
{
#ifndef NTDDI_WINBLUE
    SecureZeroMemory(&inf, sizeof(OSVERSIONINFOEX));
    inf.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&inf);
    if (IsVistaOrLater())
#else
    if (IsWindowsVistaOrGreater())
#endif
    {
        HANDLE hToken = nullptr;
        if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            TOKEN_ELEVATION te = {0};
            DWORD dwReturnLength = 0;

            if (::GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &dwReturnLength))
            {
                isElevated = (te.TokenIsElevated != 0);
            }
            TOKEN_ELEVATION_TYPE tet = TokenElevationTypeDefault;
            if (::GetTokenInformation(hToken, TokenElevationType, &tet, sizeof(tet), &dwReturnLength))
            {
                isUACEnabled = tet != TokenElevationTypeDefault;
            }
            ::CloseHandle( hToken );
        }
    }
    else
    {
        // on XP, assume we're elevated...
        isElevated = true;
    }
}

SysInfo::~SysInfo(void)
{
}

const SysInfo& SysInfo::Instance()
{
    static SysInfo instance;
    return instance;
}
