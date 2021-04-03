// sktoolslib - common files for SK tools

// Copyright (C) 2014, 2017, 2020-2021 - Stefan Kueng

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
#include "TempFile.h"
#include "StringUtils.h"
#include "PathUtils.h"
#include "SmartHandle.h"

CTempFiles::CTempFiles()
{
}

CTempFiles::~CTempFiles()
{
    for (const auto& f : m_tempFileList)
        DeleteFile(f.c_str());
}

CTempFiles& CTempFiles::Instance()
{
    static CTempFiles instance;
    return instance;
}

std::wstring CTempFiles::ConstructTempPath(const std::wstring& path) const
{
    DWORD len      = ::GetTempPath(0, nullptr);
    auto  tempPath = std::make_unique<wchar_t[]>(len + 1);
    auto  tempF    = std::make_unique<wchar_t[]>(len + 50);
    ::GetTempPath(len + 1, tempPath.get());
    std::wstring tempFile;
    if (path.empty())
    {
        ::GetTempFileName(tempPath.get(), L"skt", 0, tempF.get());
        tempFile = std::wstring(tempF.get());
    }
    else
    {
        int i = 0;
        do
        {
            std::wstring filename = path;
            // the inner loop assures that the resulting path is < MAX_PATH
            // if that's not possible without reducing the 'filename' to less than 5 chars, use a path
            // that's longer than MAX_PATH (in that case, we can't really do much to avoid longer paths)
            std::wstring sExt = CPathUtils::GetFileExtension(path);
            do
            {
                std::wstring possibleTempFile = CStringUtils::Format(L"%s%s.svn%3.3x.tmp.%s", tempPath.get(), filename.c_str(), i, sExt.c_str());
                tempFile                      = possibleTempFile;
                filename                      = filename.substr(0, filename.size() - 1);
            } while ((filename.size() > 4) && (tempFile.size() >= MAX_PATH));
            i++;
        } while (PathFileExists(tempFile.c_str()));
    }

    // caller has to actually grab the file path

    return tempFile;
}

std::wstring CTempFiles::CreateTempPath(bool bRemoveAtEnd, const std::wstring& path, bool directory)
{
    bool succeeded = false;
    for (int retryCount = 0; retryCount < Max_Retries; ++retryCount)
    {
        std::wstring tempFile = ConstructTempPath(path);

        // now create the temp file / directory, so that subsequent calls to GetTempFile() return
        // different filenames.
        // Handle races, i.e. name collisions.

        if (directory)
        {
            DeleteFile(tempFile.c_str());
            if (CreateDirectory(tempFile.c_str(), nullptr) == FALSE)
            {
                if (GetLastError() != ERROR_ALREADY_EXISTS)
                    return std::wstring();
            }
            else
                succeeded = true;
        }
        else
        {
            CAutoFile hFile = CreateFile(tempFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
            if (!hFile)
            {
                if (GetLastError() != ERROR_ALREADY_EXISTS)
                    return std::wstring();
            }
            else
            {
                succeeded = true;
            }
        }

        // done?

        if (succeeded)
        {
            if (bRemoveAtEnd)
                m_tempFileList.insert(tempFile);

            return tempFile;
        }
    }

    // give up

    return std::wstring();
}

std::wstring CTempFiles::GetTempFilePath(bool bRemoveAtEnd, const std::wstring& path /*= std::wstring()*/)
{
    return CreateTempPath(bRemoveAtEnd, path, false);
}

std::wstring CTempFiles::GetTempFilePathString()
{
    return CreateTempPath(true, std::wstring(), false);
}

std::wstring CTempFiles::GetTempDirPath(bool bRemoveAtEnd, const std::wstring& path /* = CTSVNPath() */)
{
    return CreateTempPath(bRemoveAtEnd, path, true);
}
