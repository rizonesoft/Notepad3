// sktoolslib - common files for SK tools

// Copyright (C) 2016-2017, 2020-2021 Stefan Kueng

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
#include "ScrollTool.h"

CScrollTool::CScrollTool(HINSTANCE hInst)
    : CWindow(hInst)
    , m_bInitCalled(false)
    , m_bRightAligned(false)
{
    SecureZeroMemory(&ti, sizeof(ti));
}

CScrollTool::~CScrollTool()
{
}

bool CScrollTool::Init(bool bRightAligned /* = false */)
{
    if (!m_bInitCalled)
    {
        // the tooltips class is already registered by Windows
        bRegisterWindowCalled = true;
        // create the tooltip window
        if (!CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, nullptr, nullptr, TOOLTIPS_CLASSW))
        {
            return false;
        }
        //m_hwnd = CreateWindowEx(0, TOOLTIPS_CLASSW, nullptr, TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hParent, 0, hResource, (void *)this);

        wchar_t space[] = L" ";
        ti.cbSize       = sizeof(TOOLINFO);
        ti.uFlags       = TTF_TRACK;
        ti.hwnd         = nullptr;
        ti.hinst        = nullptr;
        ti.uId          = 0;
        ti.lpszText     = space;

        // ToolTip control will cover the whole window
        ti.rect.left   = 0;
        ti.rect.top    = 0;
        ti.rect.right  = 0;
        ti.rect.bottom = 0;

        POINT point;
        ::GetCursorPos(&point);

        SendMessage(*this, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
        SendMessage(*this, TTM_TRACKPOSITION, 0, static_cast<LPARAM>(MAKELONG(point.x, point.y)));
        SendMessage(*this, TTM_TRACKACTIVATE, true, reinterpret_cast<LPARAM>(&ti));

        m_bRightAligned = bRightAligned;
        m_bInitCalled   = true;
    }
    return true;
}

void CScrollTool::SetText(LPPOINT pos, const wchar_t* fmt, ...)
{
    if (!m_bInitCalled)
    {
        assert(0);
        return;
    }

    std::wstring s;
    va_list      marker;

    va_start(marker, fmt);
    // Get formatted string length adding one for the NUL
    auto len = _vscwprintf(fmt, marker);
    if (len > 0)
    {
        s.resize(len + 1);
        _vsnwprintf_s(&s[0], s.size(), len, fmt, marker);
        s.resize(len);
    }
    va_end(marker);

    SIZE textSize = {0};
    if (m_bRightAligned)
    {
        auto hDC = GetDC(*this);
        GetTextExtentPoint32(hDC, s.c_str(), static_cast<int>(s.length()), &textSize);
        ReleaseDC(*this, hDC);
    }

    ti.lpszText = const_cast<LPWSTR>(s.c_str());
    SendMessage(*this, TTM_UPDATETIPTEXT, 0, reinterpret_cast<LPARAM>(&ti));
    SendMessage(*this, TTM_TRACKPOSITION, 0, MAKELONG(pos->x - textSize.cx, pos->y));
}

void CScrollTool::Clear()
{
    if (m_bInitCalled)
    {
        SendMessage(*this, TTM_DELTOOL, 0, reinterpret_cast<LPARAM>(&ti));
        DestroyWindow(*this);
    }
    m_bInitCalled = false;
}

LONG CScrollTool::GetTextWidth(LPCWSTR szText)
{
    SIZE textsize = {0};
    auto hDC      = GetDC(*this);
    GetTextExtentPoint32(hDC, szText, static_cast<int>(wcslen(szText)), &textsize);
    ReleaseDC(*this, hDC);
    return textsize.cx;
}

LRESULT CScrollTool::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
