// sktoolslib - common files for SK tools

// Copyright (C) 2013-2015, 2017, 2020-2021 - Stefan Kueng

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

// Useful path info is found here:
// http://msdn.microsoft.com/en-us/library/aa365247.aspx#fully_qualified_vs._relative_paths

#include "stdafx.h"
#include "PathUtils.h"
#include "StringUtils.h"
#include <memory>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <Shlwapi.h>
#include <Shldisp.h>
#include <Shlobj.h>
#include <comutil.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "comsuppw.lib")

// New code should probably use filesystem V3 when it is standard.

namespace
{
// These variables are not exposed as any path name handling probably
// should be a function in here rather than be manipulating strings directly / inline.
const wchar_t thisOsPathSeparator  = L'\\';
const wchar_t otherOsPathSeparator = L'/';
const wchar_t DeviceSeparator      = L':';

// Check if the character given is either type of folder separator.
// if we want to remove support for "other"separators we can just
// change this function and force callers to use NormalizeFolderSeparators on
// filenames first at first point of entry into a program.
inline bool IsFolderSeparator(wchar_t c)
{
    return (c == thisOsPathSeparator || c == otherOsPathSeparator);
}

} // namespace

std::wstring CPathUtils::GetLongPathname(const std::wstring& path)
{
    if (path.empty())
        return path;
    wchar_t      pathBufCanonicalized[MAX_PATH]; // MAX_PATH ok.
    DWORD        ret  = 0;
    std::wstring sRet = path;
    if (!PathIsURL(path.c_str()) && PathIsRelative(path.c_str()))
    {
        ret = GetFullPathName(path.c_str(), 0, nullptr, nullptr);
        if (ret)
        {
            auto pathBuf = std::make_unique<wchar_t[]>(ret + 1);
            if ((ret = GetFullPathName(path.c_str(), ret, pathBuf.get(), nullptr)) != 0)
            {
                sRet = std::wstring(pathBuf.get(), ret);
            }
        }
    }
    else if (PathCanonicalize(pathBufCanonicalized, path.c_str()))
    {
        ret          = ::GetLongPathName(pathBufCanonicalized, nullptr, 0);
        auto pathBuf = std::make_unique<wchar_t[]>(ret + 2);
        ret          = ::GetLongPathName(pathBufCanonicalized, pathBuf.get(), ret + 1);
        // GetFullPathName() sometimes returns the full path with the wrong
        // case. This is not a problem on Windows since its filesystem is
        // case-insensitive. But for SVN that's a problem if the wrong case
        // is inside a working copy: the svn wc database is case sensitive.
        // To fix the casing of the path, we use a trick:
        // convert the path to its short form, then back to its long form.
        // That will fix the wrong casing of the path.
        int shortRet = ::GetShortPathName(pathBuf.get(), nullptr, 0);
        if (shortRet)
        {
            auto shortPath = std::make_unique<wchar_t[]>(shortRet + 2);
            if (::GetShortPathName(pathBuf.get(), shortPath.get(), shortRet + 1))
            {
                int ret2 = ::GetLongPathName(shortPath.get(), pathBuf.get(), ret + 1);
                if (ret2)
                    sRet = std::wstring(pathBuf.get(), ret2);
            }
        }
    }
    else
    {
        ret          = ::GetLongPathName(path.c_str(), nullptr, 0);
        auto pathBuf = std::make_unique<wchar_t[]>(ret + 2);
        ret          = ::GetLongPathName(path.c_str(), pathBuf.get(), ret + 1);
        sRet         = std::wstring(pathBuf.get(), ret);
        // fix the wrong casing of the path. See above for details.
        int shortRet = ::GetShortPathName(pathBuf.get(), nullptr, 0);
        if (shortRet)
        {
            auto shortPath = std::make_unique<wchar_t[]>(shortRet + 2);
            if (::GetShortPathName(pathBuf.get(), shortPath.get(), shortRet + 1))
            {
                int ret2 = ::GetLongPathName(shortPath.get(), pathBuf.get(), ret + 1);
                if (ret2)
                    sRet = std::wstring(pathBuf.get(), ret2);
            }
        }
    }
    if (ret == 0)
        return path;
    return sRet;
}

std::wstring CPathUtils::AdjustForMaxPath(const std::wstring& path)
{
    if (path.size() < 248) // 248 instead of MAX_PATH because 248 is the limit for directories
        return path;
    if (path.substr(0, 4).compare(L"\\\\?\\") == 0)
        return path;
    return L"\\\\?\\" + path;
}

// Return the parent directory for a given path.
// Note if the path is just a device like "c:"
// or a device and a root like "c:\"
// or a server name like "\\my_server"
// then there is no parent, so "" is returned.

std::wstring CPathUtils::GetParentDirectory(const std::wstring& path)
{
    static std::wstring noParent;
    size_t              pathLen = path.length();
    size_t              pos;

    for (pos = pathLen; pos > 0;)
    {
        --pos;
        if (IsFolderSeparator(path[pos]))
        {
            size_t fileNameLen = pathLen - (pos + 1);
            // If the path in it's entirety is just a root, i.e. "\", it has no parent.
            if (pos == 0 && fileNameLen == 0)
                return noParent;
            // If the path in it's entirety is server name, i.e. "\\x", it has no parent.
            if (pos == 1 && IsFolderSeparator(path[0]) && IsFolderSeparator(path[1]) && fileNameLen > 0)
                return noParent;
            // If the parent begins with a device and root, i.e. "?:\" then
            // include both in the parent.
            if (pos == 2 && path[pos - 1] == DeviceSeparator)
            {
                // If the path is just a device i.e. not followed by a filename, it has no parent.
                if (fileNameLen == 0)
                    return noParent;
                ++pos;
            }
            // In summary, return everything before the last "\" of a filename unless the
            // whole path given is:
            // a server name, a device name, a root directory, or
            // a device followed by a root directory, in which case return "".
            std::wstring parent = path.substr(0, pos);
            return parent;
        }
    }
    // The path doesn't have a directory separator, we must be looking at either:
    // 1. just a name, like "apple"
    // 2. just a device, like "c:"
    // 3. a device followed by a name "c:apple"

    // 1. and 2. specifically have no parent,
    // For 3. the parent is the device including the separator.
    // We'll return just the separator if that's all there is.
    // It's an odd corner case but allow it through so the caller
    // yields an error if it uses it concatenated with another name rather
    // than something that might work.
    pos = path.find_first_of(DeviceSeparator);
    if (pos != std::wstring::npos)
    {
        // A device followed by a path. The device is the parent.
        std::wstring parent = path.substr(0, pos + 1);
        return parent;
    }
    return noParent;
}

// Finds the last "." after the last path separator and returns
// everything after it, NOT including the ".".
// Handles leading folders with dots.
// Example, if given: "c:\product version 1.0\test.txt"
// returns:          "txt"
std::wstring CPathUtils::GetFileExtension(const std::wstring& path)
{
    // Find the last dot after the first path separator as
    // folders can have dots in them too.
    // Start at the last character and work back stopping at the
    // first . or path separator. If we find a dot take the rest
    // after it as the extension.
    for (size_t i = path.length(); i > 0;)
    {
        --i;
        if (IsFolderSeparator(path[i]))
            break;
        if (path[i] == L'.')
        {
            std::wstring ext = path.substr(i + 1);
            return ext;
        }
    }
    return std::wstring();
}

// Finds the first "." after the last path separator and returns
// everything after it, NOT including the ".".
// Handles leading folders with dots.
// Example, if given: "c:\product version 1.0\test.aspx.cs"
// returns:          "aspx.cs"
std::wstring CPathUtils::GetLongFileExtension(const std::wstring& path)
{
    // Find the last dot after the first path separator as
    // folders can have dots in them too.
    // Start at the last character and work back stopping at the
    // first . or path separator. If we find a dot take the rest
    // after it as the extension.
    size_t foundPos = static_cast<size_t>(-1);
    bool   found    = false;
    for (size_t i = path.length(); i > 0;)
    {
        --i;
        if (IsFolderSeparator(path[i]))
            break;
        if (path[i] == L'.')
        {
            foundPos = i;
            found    = true;
        }
    }
    if (found && foundPos > 0)
    {
        std::wstring ext = path.substr(foundPos + 1);
        return ext;
    }
    return std::wstring();
}

// Given "c:\folder\test.txt", yields "test.txt".
// Isn't tripped up by path names having both separators
// as can sometimes happen like c:\folder/test.txt
// Handles c:test.txt as can sometimes appear too.
std::wstring CPathUtils::GetFileName(const std::wstring& path)
{
    bool   gotSep = false;
    size_t sepPos = 0;

    for (size_t i = path.length(); i > 0;)
    {
        --i;
        if (IsFolderSeparator(path[i]) || path[i] == DeviceSeparator)
        {
            gotSep = true;
            sepPos = i;
            break;
        }
    }
    size_t       nameStart = gotSep ? sepPos + 1 : 0;
    size_t       nameLen   = path.length() - nameStart;
    std::wstring name      = path.substr(nameStart, nameLen);
    return name;
}

// Returns only the filename without extension, i.e. will not include a path.
std::wstring CPathUtils::GetFileNameWithoutExtension(const std::wstring& path)
{
    return RemoveExtension(GetFileName(path));
}

// Returns only the filename without extension, i.e. will not include a path.
std::wstring CPathUtils::GetFileNameWithoutLongExtension(const std::wstring& path)
{
    return RemoveLongExtension(GetFileName(path));
}

// Finds the last "." after the last path separator and returns
// everything before it.
// Does not include the dot. Handles leading folders with dots.
// Example, if given: "c:\product version 1.0\test.txt"
// returns:           "c:\product version 1.0\test"
std::wstring CPathUtils::RemoveExtension(const std::wstring& path)
{
    for (size_t i = path.length(); i > 0;)
    {
        --i;
        if (IsFolderSeparator(path[i]))
            break;
        if (path[i] == L'.')
            return path.substr(0, i);
    }
    return path;
}

// Finds the first "." after the last path separator and returns
// everything before it, NOT including the ".".
// Handles leading folders with dots.
// Example, if given: "c:\product version 1.0\test.aspx.cs"
// returns:          "aspx.cs"
std::wstring CPathUtils::RemoveLongExtension(const std::wstring& path)
{
    // Find the last dot after the first path separator as
    // folders can have dots in them too.
    // Start at the last character and work back stopping at the
    // first . or path separator. If we find a dot take the rest
    // after it as the extension.
    size_t foundPos = static_cast<size_t>(-1);
    bool   found    = false;
    for (size_t i = path.length(); i > 0;)
    {
        --i;
        if (IsFolderSeparator(path[i]))
            break;
        if (path[i] == L'.')
        {
            foundPos = i;
            found    = true;
        }
    }
    if (found && foundPos > 0)
    {
        std::wstring pathWithoutExt = path.substr(0, foundPos);
        return pathWithoutExt;
    }
    return path;
}

std::wstring CPathUtils::GetModulePath(HMODULE hMod /*= nullptr*/)
{
    DWORD                      len       = 0;
    DWORD                      bufferLen = MAX_PATH; // MAX_PATH is not the limit here!
    std::unique_ptr<wchar_t[]> path;
    do
    {
        bufferLen += MAX_PATH; // MAX_PATH is not the limit here!
        path = std::make_unique<wchar_t[]>(bufferLen);
        len  = GetModuleFileName(hMod, path.get(), bufferLen);
    } while (len == bufferLen);
    std::wstring sPath = path.get();
    return sPath;
}

std::wstring CPathUtils::GetModuleDir(HMODULE hMod /*= nullptr*/)
{
    return GetParentDirectory(GetModulePath(hMod));
}

// Append one path onto another such that "path" + "append" = "path\append"
// Aims to conform to C++ <filesystem> semantics.
// e.g: "c:" + "append" = "c:\append" not "c:append"
// note: "c:append" breaks many Windows APIs
std::wstring CPathUtils::Append(const std::wstring& path, const std::wstring& append)
{
    std::wstring newPath(path);
    size_t       pathLen   = path.length();
    size_t       appendLen = append.length();

    if (pathLen == 0)
        newPath += append;
    else if (IsFolderSeparator(path[pathLen - 1]))
        newPath += append;
    else if (appendLen > 0)
    {
        if (IsFolderSeparator(append[0]))
            newPath += append;
        else
        {
            newPath += thisOsPathSeparator;
            newPath += append;
        }
    }
    return newPath;
}

std::wstring CPathUtils::GetTempFilePath()
{
    DWORD len      = ::GetTempPath(0, nullptr);
    auto  tempPath = std::make_unique<wchar_t[]>(len + 1);
    auto  tempF    = std::make_unique<wchar_t[]>(len + 50);
    ::GetTempPath(len + 1, tempPath.get());
    std::wstring tempFile;
    ::GetTempFileName(tempPath.get(), TEXT("cm_"), 0, tempF.get());
    tempFile = std::wstring(tempF.get());
    //now create the tempfile, so that subsequent calls to GetTempFile() return
    //different filenames.
    HANDLE hFile = CreateFile(tempFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
    CloseHandle(hFile);
    return tempFile;
}

std::wstring CPathUtils::GetVersionFromFile(const std::wstring& path)
{
    struct Transarray
    {
        WORD wLanguageID;
        WORD wCharacterSet;
    };

    std::wstring strReturn;
    DWORD        dwReserved   = 0;
    DWORD        dwBufferSize = GetFileVersionInfoSize(const_cast<LPWSTR>(path.c_str()), &dwReserved);
    dwReserved                = 0;
    if (dwBufferSize > 0)
    {
        auto pBuffer = std::make_unique<char[]>(dwBufferSize);
        if (pBuffer)
        {
            UINT nInfoSize         = 0,
                 nFixedLength      = 0;
            LPCWSTR      lpVersion = nullptr;
            VOID*        lpFixedPointer;
            std::wstring strLangProductVersion;

            GetFileVersionInfo(path.c_str(),
                               dwReserved,
                               dwBufferSize,
                               pBuffer.get());

            // Check the current language
            VerQueryValue(pBuffer.get(),
                          L"\\VarFileInfo\\Translation",
                          &lpFixedPointer,
                          &nFixedLength);
            Transarray* lpTransArray = static_cast<Transarray*>(lpFixedPointer);

            strLangProductVersion = CStringUtils::Format(L"\\StringFileInfo\\%04x%04x\\ProductVersion",
                                                         lpTransArray[0].wLanguageID, lpTransArray[0].wCharacterSet);

            VerQueryValue(pBuffer.get(),
                          const_cast<LPWSTR>(strLangProductVersion.c_str()),
                          reinterpret_cast<LPVOID*>(const_cast<LPWSTR*>(&lpVersion)),
                          &nInfoSize);
            if (nInfoSize && lpVersion)
                strReturn = lpVersion;
        }
    }

    return strReturn;
}

std::wstring CPathUtils::GetAppDataPath(HMODULE hMod)
{
    PWSTR path = nullptr;
    if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &path) == S_OK)
    {
        std::wstring sPath = path;
        CoTaskMemFree(path);
        sPath += L"\\";
        sPath += CPathUtils::GetFileNameWithoutExtension(CPathUtils::GetModulePath(hMod));
        CreateDirectory(sPath.c_str(), nullptr);
        return sPath;
    }
    return CPathUtils::GetModuleDir(hMod);
}

std::wstring CPathUtils::GetCWD()
{
    // Getting the CWD is a little more complicated than it seems.
    // The directory can change between asking for the name size
    // and obtaining the name value. So we need to handle that.
    // We also need to handle any error the first or second time we ask.

    for (;;)
    {
        // Returned length already includes + 1 fo null.
        auto estimatedLen = GetCurrentDirectory(0, nullptr);
        if (estimatedLen <= 0) // Error, can't recover.
            break;
        auto cwd       = std::make_unique<wchar_t[]>(estimatedLen);
        auto actualLen = GetCurrentDirectory(estimatedLen, cwd.get());
        if (actualLen <= 0) // Error Can't recover
            break;
        // Directory changed in mean time and got larger..
        if (actualLen <= estimatedLen)
            return std::wstring(cwd.get(), actualLen);
        // If we reach here, the directory has changed between us
        // asking for it's size and obtaining the value and the
        // the size has increased, so loop around to try again.
    }
    return std::wstring();
}

// Change the path separators to ones appropriate for this OS.
void CPathUtils::NormalizeFolderSeparators(std::wstring& path)
{
    std::replace(path.begin(), path.end(), otherOsPathSeparator, thisOsPathSeparator);
}

// Path names are case insensitive, using this function is clearer
// that the string involved is a path.
// In theory it can be case insensitive or not as needed for the OS too.
int CPathUtils::PathCompare(const std::wstring& path1, const std::wstring& path2)
{
    return _wcsicmp(path1.c_str(), path2.c_str());
}

int CPathUtils::PathCompareN(const std::wstring& path1, const std::wstring& path2, size_t limit)
{
    return _wcsnicmp(path1.c_str(), path2.c_str(), limit);
}

bool CPathUtils::Unzip2Folder(LPCWSTR lpZipFile, LPCWSTR lpFolder)
{
    IShellDispatch* pIsd;

    Folder* pZippedFile  = nullptr;
    Folder* pDestination = nullptr;

    long         filesCount   = 0;
    IDispatch*   pItem        = nullptr;
    FolderItems* pFilesInside = nullptr;

    VARIANT options, outFolder, inZipFile, item;
    HRESULT hr = S_OK;
    CoInitialize(nullptr);
    try
    {
        if (CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER, IID_IShellDispatch, reinterpret_cast<void**>(&pIsd)) != S_OK)
            return false;

        inZipFile.vt      = VT_BSTR;
        _bstr_t bstr      = lpZipFile; // back reading
        inZipFile.bstrVal = bstr.Detach();
        hr                = pIsd->NameSpace(inZipFile, &pZippedFile);
        if (FAILED(hr) || !pZippedFile)
        {
            pIsd->Release();
            return false;
        }

        outFolder.vt      = VT_BSTR;
        bstr              = lpFolder; // back reading
        outFolder.bstrVal = bstr.Detach();
        pIsd->NameSpace(outFolder, &pDestination);
        if (!pDestination)
        {
            pZippedFile->Release();
            pIsd->Release();
            return false;
        }

        pZippedFile->Items(&pFilesInside);
        if (!pFilesInside)
        {
            pDestination->Release();
            pZippedFile->Release();
            pIsd->Release();
            return false;
        }

        pFilesInside->get_Count(&filesCount);
        if (filesCount < 1)
        {
            pFilesInside->Release();
            pDestination->Release();
            pZippedFile->Release();
            pIsd->Release();
            return true;
        }

        pFilesInside->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(&pItem));

        item.vt       = VT_DISPATCH;
        item.pdispVal = pItem;

        options.vt   = VT_I4;
        options.lVal = 1024 | 512 | 16 | 4; //http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx

        bool retval = pDestination->CopyHere(item, options) == S_OK;

        pItem->Release();
        pItem = nullptr;
        pFilesInside->Release();
        pFilesInside = nullptr;
        pDestination->Release();
        pDestination = nullptr;
        pZippedFile->Release();
        pZippedFile = nullptr;
        pIsd->Release();
        pIsd = nullptr;

        return retval;
    }
    catch (std::exception&)
    {
        CoUninitialize();
    }
    return false;
}

bool CPathUtils::IsKnownExtension(const std::wstring& ext)
{
    // an extension is considered as 'known' if it's registered
    // in the registry with an associated application.
    if (ext.empty())
        return false; // no extension, assume 'not known'

    LPCWSTR      sExt = ext.c_str();
    std::wstring sDotExt;
    if (ext[0] != '.')
    {
        sDotExt = L"." + ext;
        sExt    = sDotExt.c_str();
    }
    HKEY hKey = nullptr;
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, sExt, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        // key exists
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool CPathUtils::PathIsChild(const std::wstring& parent, const std::wstring& child)
{
    std::wstring sParent = GetLongPathname(parent);
    std::wstring sChild  = GetLongPathname(child);
    if (sParent.size() >= sChild.size())
        return false;
    NormalizeFolderSeparators(sParent);
    NormalizeFolderSeparators(sChild);

    std::wstring sChildAsParent = sChild.substr(0, sParent.size());
    if (sChildAsParent.empty())
        return false;
    if (PathCompare(sParent, sChildAsParent) != 0)
        return false;

    if (IsFolderSeparator(*sParent.rbegin()))
    {
        if (!IsFolderSeparator(*sChildAsParent.rbegin()))
            return false;
    }
    else
    {
        if (!IsFolderSeparator(sChild[sParent.size()]))
            return false;
    }
    return true;
}

bool CPathUtils::IsPathRelative(const std::wstring& path)
{
    return PathIsRelative(path.c_str()) ? true : false;
}

bool CPathUtils::CreateRecursiveDirectory(const std::wstring& path)
{
    if (path.empty() || PathIsRoot(path.c_str()))
        return false;

    auto ret = CreateDirectory(path.c_str(), nullptr);
    if (ret == FALSE)
    {
        if (GetLastError() == ERROR_PATH_NOT_FOUND)
        {
            if (CPathUtils::CreateRecursiveDirectory(CPathUtils::GetParentDirectory(path)))
            {
                // some file systems (e.g. webdav mounted drives) take time until
                // a dir is properly created. So we try a few times with a wait in between
                // to create the sub dir after just having created the parent dir.
                int retryCount = 5;
                do
                {
                    ret = CreateDirectory(path.c_str(), nullptr);
                    if (ret == FALSE)
                        Sleep(50);
                } while (retryCount-- && (ret == FALSE));
            }
        }
    }
    return ret != FALSE;
}

// poor mans code tests
#ifdef _DEBUG
[[maybe_unused]] static class CPathTests
{
public:
    CPathTests()
    {
        assert(CPathUtils::AdjustForMaxPath(L"c:\\") == L"c:\\");
        assert(CPathUtils::AdjustForMaxPath(L"c:\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz") == L"\\\\?\\c:\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz\\abcdefghijklmnopqrstuvwxyz");
        assert(CPathUtils::GetParentDirectory(L"c:\\windows\\system32") == L"c:\\windows");
        assert(CPathUtils::GetParentDirectory(L"c:\\") == L"");
        assert(CPathUtils::GetParentDirectory(L"\\myserver") == L"");
        assert(CPathUtils::GetFileExtension(L"d:\\test.file.ext1.ext2") == L"ext2");
        assert(CPathUtils::GetLongFileExtension(L"d:\\test.file.ext1.ext2") == L"file.ext1.ext2");
        assert(!CPathUtils::PathIsChild(L"c:\\windows\\", L"c:\\windows"));
        assert(!CPathUtils::PathIsChild(L"c:\\windows\\", L"c:\\windows\\"));
        assert(CPathUtils::PathIsChild(L"c:\\windows\\", L"c:\\windows\\child\\"));
        assert(CPathUtils::PathIsChild(L"c:\\windows\\", L"c:\\windows\\child"));
        assert(CPathUtils::PathIsChild(L"c:\\windows", L"c:\\windows\\child"));
        assert(!CPathUtils::PathIsChild(L"c:\\windows", L"c:\\windowsnotachild"));
        assert(!CPathUtils::PathIsChild(L"c:\\windows\\", L"c:\\windowsnotachild"));
    }
} cPathTests;
#endif