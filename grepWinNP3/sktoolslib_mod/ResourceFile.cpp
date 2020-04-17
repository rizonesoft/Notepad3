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
#include <stdio.h>
#include <tchar.h>
#include <malloc.h>
#include <crtdbg.h>
#include "ResourceFile.h"

CResourceFile::CResourceFile()
  : m_pBytes(NULL)
  , m_bText(FALSE)
  , m_nBufLen(0)
  , m_nPosition(0)
  , m_bIsOpen(FALSE)
  , m_bDoNotDeleteBuffer(FALSE)
{
}

CResourceFile::CResourceFile(const CResourceFile& rf)
{
    if (rf.m_bDoNotDeleteBuffer)
    {
        // buffer is allocated externally or has been detached
        m_pBytes = rf.m_pBytes;
    }
    else
    {
        m_pBytes = new BYTE [rf.m_nBufLen + 2];
        memset(m_pBytes, 0, rf.m_nBufLen+2);
        memcpy(m_pBytes, rf.m_pBytes, rf.m_nBufLen);
    }

    m_nBufLen = rf.m_nBufLen;
    m_nPosition = 0;
    m_bIsOpen = rf.m_bIsOpen;
    m_bText = rf.m_bText;
    m_bDoNotDeleteBuffer = rf.m_bDoNotDeleteBuffer;
}

CResourceFile::~CResourceFile()
{
    Close();
}

BOOL CResourceFile::Open(HINSTANCE hInstance,
                         LPCTSTR lpszResId,
                         LPCTSTR lpszResType)
{
    BOOL rc = FALSE;

    Close();

    _ASSERTE(lpszResId);
    _ASSERTE(lpszResType);

    if (lpszResId && lpszResType)
    {
        TCHAR *pszRes = NULL;

        // is this a resource name string or an id?
        if (HIWORD(lpszResId) == 0)
        {
            // id
            pszRes = MAKEINTRESOURCE(LOWORD((UINT)(UINT_PTR)lpszResId));
        }
        else
        {
            // string
            pszRes = (TCHAR *)lpszResId;
        }

        HRSRC hrsrc = FindResource(hInstance, pszRes, lpszResType);
        _ASSERTE(hrsrc);

        if (hrsrc)
        {
            DWORD dwSize = SizeofResource(hInstance, hrsrc);    // in bytes

            HGLOBAL hglob = LoadResource(hInstance, hrsrc);
            _ASSERTE(hglob);

            if (hglob)
            {
                LPVOID lplock = LockResource(hglob);
                _ASSERTE(lplock);

                if (lplock)
                {
                    // save resource as byte buffer

                    m_pBytes = new BYTE [dwSize+16];
                    memset(m_pBytes, 0, dwSize+16);
                    m_nBufLen = (int) dwSize;
                    memcpy(m_pBytes, lplock, m_nBufLen);

                    m_nPosition = 0;
                    m_bIsOpen = TRUE;
                    m_bDoNotDeleteBuffer = FALSE;   // ok to delete the buffer
                    rc = TRUE;
                }
            }
        }
    }

    return rc;
}

void CResourceFile::Close()
{
    m_bIsOpen = FALSE;
    if (m_pBytes && !m_bDoNotDeleteBuffer)
        delete [] m_pBytes;
    m_pBytes = NULL;
    m_nBufLen = 0;
    m_nPosition = 0;
}

BYTE * CResourceFile::DetachByteBuffer()
{
    BYTE *p = NULL;

    if (m_bIsOpen && !m_bText)
    {
        m_bDoNotDeleteBuffer = TRUE;
        p = m_pBytes;
    }

    return p;
}

BYTE * CResourceFile::DuplicateByteBuffer()
{
    BYTE *dup = NULL;

    if (IsOpen() && !m_bText)
    {
        dup = (BYTE *) malloc(m_nBufLen+2);
        memset(dup, 0, m_nBufLen+2);
        memcpy(dup, m_pBytes, m_nBufLen);
    }

    return dup;
}

void CResourceFile::SetByteBuffer(BYTE * buf, DWORD len)
{
    _ASSERTE(buf);
    _ASSERTE(len != 0);
    _ASSERTE(m_pBytes == NULL);

    if (buf && (len > 0))
    {
        m_pBytes = buf;
        m_nBufLen = len;
        m_bText = FALSE;
        m_bDoNotDeleteBuffer = TRUE;    // do not delete this buffer
        m_bIsOpen = TRUE;
    }
}

size_t CResourceFile::Read(BYTE *buf, size_t nBufLen)
{
    size_t nOldPosition = m_nPosition;
    size_t nIndex = 0;
    if (buf)
        *buf = _T('\0');

    if (m_bIsOpen && m_pBytes && !m_bText)
    {
        while (!IsAtEOF())
        {
            BYTE b = m_pBytes[m_nPosition++];

            if (buf && (nIndex < nBufLen))
                buf[nIndex] = b;

            nIndex++;
        }
    }

    // if we were just getting buffer size, restore position
    if (!buf)
        m_nPosition = nOldPosition;

    return nIndex;
}

size_t CResourceFile::SeekToBegin()
{
    return Seek(0, SEEK_SET);
}

size_t CResourceFile::SeekToEnd()
{
    return Seek(0, SEEK_END);
}

size_t CResourceFile::Seek(size_t offset, size_t origin)
{
    size_t rc = (size_t)-1;

    if (IsOpen())
    {
        switch (origin)
        {
            default:
            case SEEK_SET:      // beginning of file
                if (offset <= m_nBufLen)
                {
                    m_nPosition = offset;
                    rc = m_nPosition;
                }
                break;

            case SEEK_CUR:      // current position of file pointer
                if ((m_nPosition + offset) <= m_nBufLen)
                {
                    m_nPosition += offset;
                    rc = m_nPosition;
                }
                break;

            case SEEK_END:      // end of file
                m_nPosition = m_nBufLen;
                rc = m_nPosition;
                break;
        }
    }

    return rc;
}
