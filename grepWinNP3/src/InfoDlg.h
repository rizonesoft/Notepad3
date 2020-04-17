// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008 - Stefan Kueng

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
#if !defined(AFX_HTMLABOUT_H__13757693_FCB3_11D3_AB6F_0050BAD05CD9__INCLUDED_)
#define AFX_HTMLABOUT_H__13757693_FCB3_11D3_AB6F_0050BAD05CD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CInfoDlg
{
public:
    CInfoDlg();
    virtual ~CInfoDlg();
public:
    static BOOL ShowDialog(UINT idAboutHTMLID, HINSTANCE hInstance);

};

#endif // !defined(AFX_HTMLABOUT_H__13757693_FCB3_11D3_AB6F_0050BAD05CD9__INCLUDED_)
