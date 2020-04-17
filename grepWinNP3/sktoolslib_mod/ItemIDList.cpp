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
#include "ItemIDList.h"


ItemIDList::ItemIDList(LPCITEMIDLIST item, LPCITEMIDLIST parent)
    : item_(item)
    , parent_(parent)
    , count_(-1)
{
}

ItemIDList::~ItemIDList()
{
}

int ItemIDList::size() const
{
    if (count_ == -1)
    {
        count_ = 0;
        if (item_)
        {
            LPCSHITEMID ptr = &item_->mkid;
            while (ptr != 0 && ptr->cb != 0)
            {
                ++count_;
                LPBYTE byte = (LPBYTE) ptr;
                byte += ptr->cb;
                ptr = (LPCSHITEMID) byte;
            }
        }
    }
    return count_;
}

LPCSHITEMID ItemIDList::get(int index) const
{
    int count = 0;

    if (item_ == NULL)
        return NULL;
    LPCSHITEMID ptr = &item_->mkid;
    if (ptr == NULL)
        return NULL;
    while (ptr->cb != 0)
    {
        if (count == index)
            break;

        ++count;
        LPBYTE byte = (LPBYTE) ptr;
        byte += ptr->cb;
        ptr = (LPCSHITEMID) byte;
    }
    return ptr;

}

tstring ItemIDList::toString()
{
    IShellFolder *shellFolder = NULL;
    IShellFolder *parentFolder = NULL;
    STRRET name;
    TCHAR * szDisplayName = NULL;
    tstring ret;
    HRESULT hr;

    hr = ::SHGetDesktopFolder(&shellFolder);
    if (!SUCCEEDED(hr))
        return ret;
    if (parent_)
    {
        hr = shellFolder->BindToObject(parent_, 0, IID_IShellFolder, (void**) &parentFolder);
        if (!SUCCEEDED(hr))
            parentFolder = shellFolder;
    }
    else
    {
        parentFolder = shellFolder;
    }

    if ((parentFolder != 0) && (item_ != 0))
    {
        hr = parentFolder->GetDisplayNameOf(item_, SHGDN_NORMAL | SHGDN_FORPARSING, &name);
        if (!SUCCEEDED(hr))
        {
            parentFolder->Release();
            return ret;
        }
        hr = StrRetToStr (&name, item_, &szDisplayName);
        if (!SUCCEEDED(hr))
            return ret;
    }
    parentFolder->Release();
    if (szDisplayName == NULL)
    {
        CoTaskMemFree(szDisplayName);
        return ret;         //to avoid a crash!
    }
    ret = szDisplayName;
    CoTaskMemFree(szDisplayName);
    return ret;
}

LPCITEMIDLIST ItemIDList::operator& ()
{
    return item_;
}
