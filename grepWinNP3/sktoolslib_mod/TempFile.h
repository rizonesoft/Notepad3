// sktoolslib - common files for SK tools

// Copyright (C) 2014 - Stefan Kueng

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
#include <set>

/**
* This singleton class handles temporary files.
* All temp files are deleted at the end
*/
class CTempFiles
{
public:
    static CTempFiles& Instance();

    /**
     * Returns a path to a temporary file.
     * \param bRemoveAtEnd if true, the temp file is removed when this object
     *                     goes out of scope.
     * \param path         if set, the temp file will have the same file extension
     *                     as this path.
     */
    std::wstring            GetTempFilePath(bool bRemoveAtEnd, const std::wstring& path = std::wstring());
    std::wstring            GetTempFilePathString();

    /**
     * Returns a path to a temporary directory.
     * \param bRemoveAtEnd if true, the temp directory is removed when this object
     *                     goes out of scope.
     * \param path         if set, the temp directory will have the same file extension
     *                     as this path.
     */
    std::wstring            GetTempDirPath(bool bRemoveAtEnd, const std::wstring& path = std::wstring());


    void                    AddFileToRemove(const std::wstring& file) { m_TempFileList.insert(file); }

private:

    // try to allocate an unused temp file / dir at most MAX_RETRIES times

    enum {MAX_RETRIES = 100};

    // list of paths to delete when terminating the app

    std::set<std::wstring> m_TempFileList;

    // actual implementation

    std::wstring ConstructTempPath(const std::wstring& path) const;
    std::wstring CreateTempPath(bool bRemoveAtEnd, const std::wstring& path, bool directory);

    // construction / destruction

    CTempFiles(void);
    ~CTempFiles(void);
};
