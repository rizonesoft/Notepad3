// sktoolslib - common files for SK tools

// Copyright (C) 2012 - Stefan Kueng

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

class CPidl
{
public:
    /// returns the next ID in the list
    static LPITEMIDLIST         GetNextItemID(LPCITEMIDLIST pidl);
    /// returns the size of the pidl
    static UINT                 GetSize(LPCITEMIDLIST pidl);
    /// creates a copy of a pidl. The returned pidl must be freed with CoTaskMemFree
    static LPITEMIDLIST         MakeCopy(LPCITEMIDLIST pidl);
    /// removes the child part from a pidl
    static BOOL                 GetParentID(LPITEMIDLIST pidl);
    /// appends two pidls. The returned pidl must be freed with CoTaskMemFree
    static LPITEMIDLIST         Append(LPITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd);

};
