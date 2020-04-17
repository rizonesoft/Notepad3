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

#include "stdafx.h"
#include "PreserveChdir.h"
#include "PathUtils.h"

PreserveChdir::PreserveChdir() : m_originalDir(CPathUtils::GetCWD())
{
}

PreserveChdir::~PreserveChdir()
{
    if (m_originalDir.empty()) // Nothing to do if failed to get the original dir.
        return;
    // _tchdir is an expensive function - don't call it unless we really have to.
    std::wstring currentDir = CPathUtils::GetCWD();
    // Restore the current directory if it doesn't match the original directory
    // or if we failed to get the current directory.
    if (currentDir.empty() || _tcsicmp(m_originalDir.c_str(), currentDir.c_str()) != 0)
        SetCurrentDirectory(m_originalDir.c_str());
}
