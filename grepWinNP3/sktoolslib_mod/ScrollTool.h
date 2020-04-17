// sktoolslib - common files for SK tools

// Copyright (C) 2016 Stefan Kueng

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
#include "BaseWindow.h"

/**
 * Helper class to show a tooltip for e.g. a scrollbar while it is dragged.
 * Example use:
 * In the OnVScroll() message handler
 * \code
 * switch (nSBCode)
 * {
 *   case SB_THUMBPOSITION:
 *    m_ScrollTool.Clear();
 *    break;
 *   case SB_THUMBTRACK:
 *    m_ScrollTool.Init();
 *    m_ScrollTool.SetText(&thumbpoint, L"Line: %*ld", maxchars, nTrackPos);
 *    break;
 * }
 *
 */
class CScrollTool : public CWindow
{
public:
    CScrollTool(HINSTANCE hInst);
    virtual ~CScrollTool();

public:
    /**
     * Initializes the tooltip control.
     * \param pos the position in screen coordinates where the tooltip should be shown
     * \param bRightAligned if set to true, the tooltip is right aligned with pos,
     *        depending on the text width shown in the tooltip
     */
    bool Init(bool bRightAligned = false);
    /**
     * Sets the text which should be shown in the tooltip.
     * \param pos the position in screen coordinates where the tooltip should be shown
     * \fmt a format string
     */
    void SetText(LPPOINT pos, const TCHAR * fmt, ...);
    /**
     * Removes the tooltip control.
     */
    void Clear();
    /**
     * Returns the width of \c szText in pixels for the tooltip control
     */
    LONG GetTextWidth(LPCTSTR szText);

protected:
    LRESULT CALLBACK WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    TOOLINFO ti;
    bool m_bInitCalled;
    bool m_bRightAligned;
};
