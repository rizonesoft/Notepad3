// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2020 - Stefan Kueng

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

#include <vector>



#define RESIZER_TOPLEFT                 0
#define RESIZER_TOPRIGHT                1
#define RESIZER_TOPLEFTRIGHT            2
#define RESIZER_TOPLEFTBOTTOMRIGHT      3
#define RESIZER_BOTTOMLEFT              4
#define RESIZER_BOTTOMRIGHT             5
#define RESIZER_BOTTOMLEFTRIGHT         6

struct ResizeCtrls
{
    HWND        hWnd;
    UINT        resizeType;
    RECT        origSize;
};

class CDlgResizer
{
public:
    CDlgResizer(void);
    ~CDlgResizer(void);

    void    Init(HWND hWndDlg);
    void    AddControl(HWND hWndDlg, UINT ctrlId, UINT resizeType);
    void    AdjustMinMaxSize();

    void    DoResize(int width, int height);

    RECT *  GetDlgRect() {return &m_dlgRect;}
    RECT *  GetDlgRectScreen() {return &m_dlgRectScreen;}
    void    UpdateGripPos();
    void UseSizeGrip(bool use) { m_useSizeGrip = use; }

private:
    void                        ShowSizeGrip(bool bShow = true);
    HWND                        m_hDlg;
    std::vector<ResizeCtrls>    m_controls;
    RECT                        m_dlgRect;
    RECT                        m_dlgRectScreen;
    SIZE                        m_sizeGrip;
    HWND                        m_wndGrip;
    bool                        m_useSizeGrip;
};
