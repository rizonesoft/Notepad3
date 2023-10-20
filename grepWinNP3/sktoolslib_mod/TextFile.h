// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2017-2023 - Stefan Kueng

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
#include <vector>
#include <memory>

/**
 * handles text files.
 */
class CTextFile
{
public:
    CTextFile();
    ~CTextFile();

    enum UnicodeType
    {
        AutoType,
        Binary,
        Ansi,
        Unicode_Le,
        Unicode_Be,
        UTF8
    };

    /**
     * Loads a file from the specified \c path.
     */
    bool                Load(LPCWSTR path, UnicodeType& type, bool bUTF8, std::atomic_bool& bCancelled);

    /**
     * Saves the file contents to disk at \c path.
     */
    bool                Save(LPCWSTR path, bool keepFileDate) const;

    /**
     * modifies the contents of a file.
     * \param pBuf pointer to a buffer holding the new contents of the file.
     *             note: the buffer must be created with 'new BYTE[len]'
     * \param newLen length of the new file content in bytes
     * \note the old buffer is automatically freed.
     */
    bool                ContentsModified(std::unique_ptr<BYTE[]> pBuf, DWORD newLen);

    /**
     * Returns the line number from a given character position inside the file.
     */
    long                LineFromPosition(long pos) const;

    /**
     * Returns the line from a given line number
     */
    std::wstring        GetLineString(long lineNumber) const;

    /**
     * Returns the file content as a text string.
     * \note the text string can not be modified and is to be treated read-only.
     */
    const std::wstring& GetFileString() const { return textContent; }

    /**
     * Returns a pointer to the file contents. Call GetFileLength() to get
     * the size in number of bytes of this buffer.
     */
    LPVOID              GetFileContent() const { return pFileBuf.get(); }

    /**
     * Returns the size of the file in bytes
     */
    long                GetFileLength() const { return fileLen; }

    /**
     * Returns the encoding of the file
     */
    UnicodeType         GetEncoding() const { return encoding; }

    /**
     * Returns a string representation of the encoding
     */
    static std::wstring        GetEncodingString(UnicodeType type);

    /**
     * Returns the filename
     */
    const std::wstring& GetFileName() const { return filename; }

    /**
     * Returns the filename without the extension (if any)
     */
    std::wstring        GetFileNameWithoutExtension() const;

    /**
     * Returns the filename extension (if any)
     */
    std::wstring        GetFileNameExtension() const;

    /**
     * Replaces the file content.
     */
    void                SetFileContent(const std::wstring& content);

    bool                HasBOM() const { return hasBOM; }

    /**
     * Sets the number of null bytes that are allowed for
     * a file to still be considered text instead of binary
     * in the encoding detection. Default is 2.
     */
    void                SetNullbyteCountForBinary(int count) { nullByteCount = count; }

protected:
    /**
     * Tries to find out the encoding of the file (utf8, utf16, ansi)
     */
    UnicodeType CheckUnicodeType(BYTE* pBuffer, int cb) const;
    /**
     * Fills an array with line information to make it faster later
     * to get the line from a char position.
     */
    bool        CalculateLines(std::atomic_bool& bCancelled);

private:
    std::unique_ptr<BYTE[]> pFileBuf;
    DWORD                   fileLen;
    std::wstring            textContent;
    std::vector<size_t>     linePositions;
    UnicodeType             encoding;
    std::wstring            filename;
    bool                    hasBOM;
    int                     nullByteCount;
};
