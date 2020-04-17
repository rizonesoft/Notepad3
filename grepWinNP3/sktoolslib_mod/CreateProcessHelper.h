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
 * A helper class for invoking CreateProcess(). The lpProcessInformation
 * can point to an uninitialized struct - it's memset to all zeroes inside.
 */

class CCreateProcessHelper
{
public:
    static bool CreateProcess(LPCTSTR lpApplicationName,
                    LPTSTR lpCommandLine,
                    LPCTSTR lpCurrentDirectory,
                    LPPROCESS_INFORMATION lpProcessInformation,
                    bool hidden = false,
                    DWORD dwCreationFlags = CREATE_UNICODE_ENVIRONMENT);
    static bool CreateProcess(LPCTSTR lpApplicationName,
                    LPTSTR lpCommandLine,
                    LPPROCESS_INFORMATION lpProcessInformation,
                    bool hidden = false,
                    DWORD dwCreationFlags = CREATE_UNICODE_ENVIRONMENT);

    static bool CreateProcessDetached(LPCTSTR lpApplicationName,
                    LPTSTR lpCommandLine,
                    LPCTSTR lpCurrentDirectory,
                    bool hidden = false,
                    DWORD dwCreationFlags = CREATE_UNICODE_ENVIRONMENT);
    static bool CreateProcessDetached(LPCTSTR lpApplicationName,
                    LPTSTR lpCommandLine,
                    bool hidden = false,
                    DWORD dwCreationFlags = CREATE_UNICODE_ENVIRONMENT);
};

inline bool CCreateProcessHelper::CreateProcess(LPCTSTR applicationName,
    LPTSTR commandLine, LPCTSTR currentDirectory,
    LPPROCESS_INFORMATION processInfo,
    bool hidden,
    DWORD dwCreationFlags)
{
    STARTUPINFO startupInfo;
    SecureZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(STARTUPINFO);
    if (hidden)
    {
        startupInfo.dwFlags = STARTF_USESHOWWINDOW;
        startupInfo.wShowWindow = SW_HIDE;
    }

    SecureZeroMemory(processInfo, sizeof(PROCESS_INFORMATION));
    const BOOL result = ::CreateProcess( applicationName,
                    commandLine, nullptr, nullptr, FALSE, dwCreationFlags, 0, currentDirectory,
                    &startupInfo, processInfo );
    return result != 0;
}

inline bool CCreateProcessHelper::CreateProcess(LPCTSTR applicationName,
    LPTSTR commandLine, LPPROCESS_INFORMATION processInformation, bool hidden, DWORD dwCreationFlags)
{
    return CreateProcess( applicationName, commandLine, 0, processInformation, hidden, dwCreationFlags );
}

inline bool CCreateProcessHelper::CreateProcessDetached(LPCTSTR lpApplicationName,
    LPTSTR lpCommandLine, LPCTSTR lpCurrentDirectory, bool hidden, DWORD dwCreationFlags)
{
    PROCESS_INFORMATION process;
    if (!CreateProcess(lpApplicationName, lpCommandLine, lpCurrentDirectory, &process, hidden, dwCreationFlags))
        return false;

    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return true;
}

inline bool CCreateProcessHelper::CreateProcessDetached(LPCTSTR lpApplicationName,
    LPTSTR lpCommandLine, bool hidden, DWORD dwCreationFlags)
{
    return CreateProcessDetached(lpApplicationName, lpCommandLine, 0, hidden, dwCreationFlags);
}
