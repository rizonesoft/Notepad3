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
#include <string.h>
#include <tchar.h>
#include <malloc.h>
#include <crtdbg.h>
#include "ResourceTextFile.h"

CResourceTextFile::CResourceTextFile()
    : m_pszText(NULL)
    , m_eBomAction(NoBomAction)
    , m_eConvertAction(NoConvertAction)
{
}

CResourceTextFile::CResourceTextFile(const CResourceTextFile& rf)
{
    if (rf.m_bDoNotDeleteBuffer)
    {
        // buffer is allocated externally or has been detached
        m_pszText = rf.m_pszText;
    }
    else
    {
        m_pszText = new TCHAR [rf.m_nBufLen + 2];
        memset(m_pszText, 0, (rf.m_nBufLen+2)*sizeof(TCHAR));
        _tcsncpy_s(m_pszText, rf.m_nBufLen + 2, rf.m_pszText, rf.m_nBufLen);
    }

    m_nBufLen = rf.m_nBufLen;
    m_nPosition = 0;
    m_bIsOpen = rf.m_bIsOpen;
    m_bText = rf.m_bText;
    m_bDoNotDeleteBuffer = rf.m_bDoNotDeleteBuffer;
    m_eConvertAction = rf.m_eConvertAction;
    m_eBomAction = rf.m_eBomAction;
}

CResourceTextFile::~CResourceTextFile()
{
    Close();
}

BOOL CResourceTextFile::Open(HINSTANCE hInstance,
                             LPCTSTR lpszResId,
                             LPCTSTR lpszResType /*= _T("TEXT")*/,
                             ConvertAction eConvertAction /*= NoConvertAction*/,
                             BomAction eBomAction /*= NoBomAction*/)
{
    BOOL rc = FALSE;

    Close();

    _ASSERTE(lpszResId);
    _ASSERTE(lpszResType);

    m_eConvertAction = eConvertAction;
    m_eBomAction = eBomAction;

    if (lpszResId && lpszResType)
    {
        rc = CResourceFile::Open(hInstance, lpszResId, lpszResType);

        if (rc)
        {
            TCHAR *cp = (TCHAR *) GetByteBuffer();
            DWORD dwSize = (DWORD) GetLength();

            rc = SetTextBuffer(cp, dwSize,
                    eConvertAction, eBomAction);

            if (rc)
            {
                m_bText = TRUE;
            }
            else
            {
                Close();
            }
        }
    }

    return rc;
}

void CResourceTextFile::Close()
{
    CResourceFile::Close();

    if (m_pszText && !m_bDoNotDeleteBuffer)
        delete [] m_pszText;
    m_pszText = NULL;
}

TCHAR * CResourceTextFile::DetachTextBuffer()
{
    TCHAR *cp = NULL;

    if (m_bIsOpen && m_bText)
    {
        m_bDoNotDeleteBuffer = TRUE;
        cp = m_pszText;
    }

    return cp;
}

TCHAR * CResourceTextFile::DuplicateTextBuffer()
{
    TCHAR *dup = NULL;

    if (IsOpen() && m_bText)
        dup = _tcsdup(m_pszText);

    return dup;
}

BOOL CResourceTextFile::SetTextBuffer(TCHAR * inbuf,
                                      DWORD len,
                                      ConvertAction eConvertAction /*= NoConvertAction*/,
                                      BomAction eBomAction /*= NoBomAction*/)
{
    BOOL rc = FALSE;

    _ASSERTE(inbuf);
    _ASSERTE(len != 0);
    _ASSERTE(m_pszText == NULL);

    if (inbuf && (len != 0))
    {
        m_bText = TRUE;

        m_eConvertAction = eConvertAction;
        m_eBomAction = eBomAction;

        DWORD dwSize = len;     // bytes

        // copy buffer to ensure it's null terminated
        BYTE * buf = new BYTE [dwSize+16];
        memset(buf, 0, dwSize+16);
        memcpy(buf, inbuf, dwSize);

        BOOL bFoundBom = (buf[0] == 0xFF) && (buf[1] == 0xFE);

        if (m_eConvertAction == ConvertToUnicode)
        {
            int wlen = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buf, -1, NULL, 0);
#ifndef _UNICODE
            wlen = wlen * sizeof(WCHAR);
#endif
            m_pszText = new TCHAR [wlen+16];
            memset(m_pszText, 0, (wlen+16)*sizeof(TCHAR));
            LPWSTR wp = (LPWSTR) m_pszText;
            if ((m_eBomAction == AddBom) && !bFoundBom)
            {
                // caller wants a BOM
                BYTE * p = (BYTE *)m_pszText;
                p[0] = 0xFF;
                p[1] = 0xFE;
                wp += 1;
            }
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buf, -1, wp, wlen+2);
            m_nBufLen = wcslen((WCHAR*)m_pszText);
        }
        else if (m_eConvertAction == ConvertToAnsi)
        {
            LPCWSTR wp = (LPCWSTR) buf;
            if (bFoundBom && (m_eBomAction == RemoveBom))
                wp++;   // skip over BOM
            int alen = WideCharToMultiByte(CP_ACP, 0, wp, -1,
                            NULL, 0, NULL, NULL);
            m_pszText = new TCHAR [alen+4];
            memset(m_pszText, 0, (alen+4)*sizeof(TCHAR));
            WideCharToMultiByte(CP_ACP, 0, wp, -1,
                (LPSTR)m_pszText, alen+1, NULL, NULL);
            m_nBufLen = strlen((LPCSTR)m_pszText);
        }
        else
        {
            // no conversion
            m_pszText = new TCHAR [(dwSize + 16)/sizeof(TCHAR)];
            TCHAR *cp = m_pszText;
            memset(m_pszText, 0, dwSize+8);
            int index = 0;
            if ((m_eBomAction == AddBom) && !bFoundBom)
            {
                BYTE bom[2] = { 0xFF, 0xFE };
                memcpy(cp, bom, 2);
                cp += 2;
            }
            else if ((m_eBomAction == RemoveBom) && bFoundBom)
            {
                index = 2;
            }
            memcpy(cp, &buf[index], dwSize);
            m_nBufLen = _tcslen(m_pszText);
        }

        m_nPosition = 0;
        m_bIsOpen = TRUE;
        m_bDoNotDeleteBuffer = FALSE;   // ok to delete the buffer
        delete [] buf;
        rc = TRUE;
    }

    return rc;
}

size_t CResourceTextFile::ReadLine(TCHAR *buf, size_t nBufLen)
{
    size_t nOldPosition = m_nPosition;
    size_t nIndex = 0;
    if (buf)
        *buf = _T('\0');

    if (m_bIsOpen && m_pszText && m_bText)
    {
        while (!IsAtEOF())
        {
            TCHAR c = m_pszText[m_nPosition++];

            if ((c == _T('\r')) || (c == _T('\n')))
            {
                if (!IsAtEOF())
                {
                    // check for \r\n pair
                    TCHAR prevc = c;
                    c = m_pszText[m_nPosition];
                    if (((prevc == _T('\r')) && (c == _T('\n'))) ||
                        ((prevc == _T('\n')) && (c == _T('\r'))))
                    {
                        m_nPosition++;
                    }
                }
                break;  // end of line
            }

            if (buf && (nIndex < nBufLen))
                buf[nIndex] = c;
            nIndex++;
        }
    }

    // add terminating nul always
    if (buf)
    {
        if (nIndex >= nBufLen)
        {
            // there is not enough room, so replace last char
            nIndex = nBufLen - 1;
            if (nBufLen == 0)
                nIndex = 0;
        }
        buf[nIndex] = _T('\0');
    }

    // if we were just getting buffer size, restore position
    if (!buf)
    {
        m_nPosition = nOldPosition;
    }

    return nIndex;
}
