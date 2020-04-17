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

#include "stdafx.h"
#include "Pidl.h"


LPITEMIDLIST CPidl::GetNextItemID(LPCITEMIDLIST pidl)
{
    // Check for valid pidl.
    if (pidl == NULL)
        return NULL;

    // Get the size of the specified item identifier.
    int cb = pidl->mkid.cb;

    // If the size is zero, it is the end of the list.
    if (cb == 0)
        return NULL;

    // Add cb to pidl (casting to increment by bytes).
    pidl = (LPITEMIDLIST) (((LPBYTE) pidl) + cb);

    // Return NULL if it is null-terminating, or a pidl otherwise.
    return (pidl->mkid.cb == 0) ? NULL : (LPITEMIDLIST) pidl;
}

UINT CPidl::GetSize(LPCITEMIDLIST pidl)
{
    UINT cbTotal = 0;
    if (pidl)
    {
        cbTotal += sizeof(pidl->mkid.cb);    // Terminating null character
        while (pidl)
        {
            cbTotal += pidl->mkid.cb;
            pidl = GetNextItemID(pidl);
        }
    }
    return cbTotal;
}

LPITEMIDLIST CPidl::MakeCopy(LPCITEMIDLIST pidl)
{
    UINT cb = 0;

    // Calculate size of list.
    cb = GetSize(pidl);

    LPITEMIDLIST pidlRet = (LPITEMIDLIST)CoTaskMemAlloc(cb);
    if (pidlRet)
        CopyMemory(pidlRet, pidl, cb);
    return pidlRet;
}

BOOL CPidl::GetParentID(LPITEMIDLIST pidl)
{
    BOOL fRemoved = FALSE;

    // Make sure it's a valid PIDL.
    if (pidl == NULL)
        return(FALSE);

    if (pidl->mkid.cb)
    {
        LPITEMIDLIST pidlNext = pidl;
        while (pidlNext)
        {
            pidl = pidlNext;
            pidlNext = GetNextItemID(pidl);
        }
        // Remove the last one, insert terminating null character.
        pidl->mkid.cb = 0;
        fRemoved = TRUE;
    }

    return fRemoved;
}

LPITEMIDLIST CPidl::Append(LPITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd)
{
    if (pidlBase == NULL)
        return NULL;
    if (pidlAdd == NULL)
        return pidlBase;

    LPITEMIDLIST pidlNew;

    UINT cb1 = GetSize(pidlBase) - sizeof(pidlBase->mkid.cb);
    UINT cb2 = GetSize(pidlAdd);

    pidlNew = (LPITEMIDLIST)CoTaskMemAlloc(cb1 + cb2);
    if (pidlNew)
    {
        CopyMemory(pidlNew, pidlBase, cb1);
        CopyMemory(((LPSTR)pidlNew) + cb1, pidlAdd, cb2);
    }
    return pidlNew;
}
