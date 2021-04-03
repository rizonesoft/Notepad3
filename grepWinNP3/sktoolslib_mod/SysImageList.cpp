// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2017, 2020-2021 - Stefan Kueng

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
#include "SysImageList.h"
#include <shellapi.h>

// Singleton constructor and destructor (private)

CSysImageList* CSysImageList::instance = nullptr;

CSysImageList::CSysImageList()
{
    m_hSystemImageList = nullptr;
    SHFILEINFO ssfi    = {nullptr};
    wchar_t    winDir[MAX_PATH];
    GetWindowsDirectory(winDir, _countof(winDir)); // MAX_PATH ok.
    m_hSystemImageList =
        reinterpret_cast<HIMAGELIST>(SHGetFileInfo(
            winDir,
            0,
            &ssfi, sizeof ssfi,
            SHGFI_SYSICONINDEX | SHGFI_SMALLICON));
}

CSysImageList::~CSysImageList()
{
}

// Singleton specific operations

CSysImageList& CSysImageList::GetInstance()
{
    if (instance == nullptr)
        instance = new CSysImageList;
    return *instance;
}

void CSysImageList::Cleanup()
{
    delete instance;
    instance = nullptr;
}

// Operations

int CSysImageList::GetDirIconIndex()
{
    SHFILEINFO sfi;
    SecureZeroMemory(&sfi, sizeof sfi);

    SHGetFileInfo(
        L"Doesn't matter",
        FILE_ATTRIBUTE_DIRECTORY,
        &sfi, sizeof sfi,
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

    return sfi.iIcon;
}

int CSysImageList::GetDirOpenIconIndex()
{
    SHFILEINFO sfi;
    SecureZeroMemory(&sfi, sizeof sfi);

    SHGetFileInfo(
        L"Doesn't matter",
        FILE_ATTRIBUTE_DIRECTORY,
        &sfi, sizeof sfi,
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES | SHGFI_OPENICON);

    return sfi.iIcon;
}

int CSysImageList::GetDefaultIconIndex()
{
    SHFILEINFO sfi;
    SecureZeroMemory(&sfi, sizeof sfi);

    SHGetFileInfo(
        L"",
        FILE_ATTRIBUTE_NORMAL,
        &sfi, sizeof sfi,
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

    return sfi.iIcon;
}

int CSysImageList::GetFileIconIndex(const std::wstring& file)
{
    SHFILEINFO sfi;
    SecureZeroMemory(&sfi, sizeof sfi);

    SHGetFileInfo(
        file.c_str(),
        FILE_ATTRIBUTE_NORMAL,
        &sfi, sizeof sfi,
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

    return sfi.iIcon;
}

CSysImageList::operator HIMAGELIST() const
{
    return m_hSystemImageList;
}
