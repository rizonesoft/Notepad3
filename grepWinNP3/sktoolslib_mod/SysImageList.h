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

#include <string>
#include <shellapi.h>
#include <Commctrl.h>

#pragma comment(lib, "shell32.lib")

/**
 * Provides simplified access to the system icons. Only small system icons
 * are supported.
 *
 * \note This class is implemented as a singleton.
 * The singleton instance is created when first accessed. See SYS_IMAGE_LIST() function
 * easy access of the singleton instance. All
 */
class CSysImageList
{
// Singleton constructor and destructor (private)
private:
    CSysImageList();
    ~CSysImageList();

// Singleton specific operations
public:
    /**
     * Returns a reference to the one and only instance of this class.
     */
    static CSysImageList& GetInstance();
    /**
     * Frees all allocated resources (if necessary). Don't call this
     * function when the image list is currently bound to any control!
     */
    static void Cleanup();

// Operations
public:
    /**
     * Returns the icon index for a directory.
     */
    int GetDirIconIndex() const;
    /**
     * Returns the icon index for a directory that's open (e.g. for a tree control)
     */
    int GetDirOpenIconIndex() const;
    /**
     * Returns the icon index for a file which has no special icon associated.
     */
    int GetDefaultIconIndex() const;
    /**
     * Returns the icon index for the specified \a file. Only the file extension
     * is used to determine the file's icon.
     */
    int GetFileIconIndex(const std::wstring& file) const;

    operator HIMAGELIST() const;

private:
    static CSysImageList *              instance;

    HIMAGELIST                          m_hSystemImageList;
};


/**
 * \relates CSysImageList
 * Singleton access for CSysImageList.
 */
inline CSysImageList& SYS_IMAGE_LIST() { return CSysImageList::GetInstance(); }
