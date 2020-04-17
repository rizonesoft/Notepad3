// sktoolslib - common files for SK tools

// Copyright (C) 2018, 2020 - Stefan Kueng

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
#include <wrl.h>

/// Sets the application ID
HRESULT SetAppID(LPCTSTR appID);
/// creates an IShellLink object with the specified arguments
/// note: the shell link is created for the current process exe path
/// \param pszArguments the command line arguments to pass to this application
/// \param pszTitle the title of the link which appears in the jump list
/// \param iconIndex the index of the icon. Note this is NOT the icon ID but the number of the icon as it appears in the compiled resources
/// \param asAdmin if true, the link is executed with "runas"
HRESULT CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, int iconIndex, bool asAdmin, Microsoft::WRL::ComPtr<IShellLink> *ppsl);
/// creates an IShellLink that is a separator
HRESULT CreateSeparatorLink(Microsoft::WRL::ComPtr<IShellLink> *ppsl);
/// checks if an IShellItem is in an IObjectArray
bool IsItemInArray(IShellItem *psi, IObjectArray *poaRemoved);
/// removes the existing jump list
HRESULT DeleteJumpList(LPCTSTR appID);
/// sets the relaunch command for the specified window.
/// use this to specify command line parameters for the default jump list command which starts the pinned application
HRESULT SetRelaunchCommand(HWND hWnd, LPCWSTR appID, LPCWSTR commandLine, LPCWSTR dispName, LPCWSTR icon = nullptr);

