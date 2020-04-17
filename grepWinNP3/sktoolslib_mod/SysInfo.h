// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013 - Stefan Kueng

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
* \ingroup Utils
* This singleton class handles system information
*/

#ifdef NTDDI_WINBLUE
#include <VersionHelpers.h>
class SysInfo
{
private:
    SysInfo(void);
    ~SysInfo(void);
public:
    static const SysInfo& Instance();

    bool            IsElevated() const { return isElevated; }
    bool            IsUACEnabled() const { return isUACEnabled; }
private:
    bool            isElevated;
    bool            isUACEnabled;
};

#else

class SysInfo
{
private:
    SysInfo(void);
    ~SysInfo(void);
public:
    static const SysInfo& Instance();

    DWORD           GetFullVersion() const {return MAKEWORD(inf.dwMinorVersion, inf.dwMajorVersion);}
    bool            IsXP() const {return (GetFullVersion() < 0x0600);} // cover Win5.1 and 5.2 alike
    bool            IsVista() const {return (GetFullVersion() == 0x0600);}
    bool            IsVistaOrLater() const {return (GetFullVersion() >= 0x0600);}
    bool            IsWin7() const {return (GetFullVersion() == 0x0601);}
    bool            IsWin7OrLater() const {return (GetFullVersion() >= 0x0601);}
    bool            IsWin8() const {return (GetFullVersion() == 0x0602);}
    bool            IsWin8OrLater() const {return (GetFullVersion() >= 0x0602);}
    bool            IsElevated() const {return isElevated;}
    bool            IsUACEnabled() const {return isUACEnabled;}
private:
    OSVERSIONINFOEX inf;
    bool            isElevated;
    bool            isUACEnabled;
};

#endif