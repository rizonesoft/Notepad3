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

#pragma once

#include <string>

/**
 * \ingroup Utils
 * A simple wrapper class for the SHBrowseForFolder API on XP.
 * On Vista and later, the IFileDialog is used with the FOS_PICKFOLDERS flag.
 */
class CBrowseFolder
{
public:
    enum retVal
    {
        CANCEL = 0,     ///< the user has pressed cancel
        NOPATH,         ///< no folder was selected
        OK              ///< everything went just fine
    };
public:
    //constructor / deconstructor
    CBrowseFolder(void);
    ~CBrowseFolder(void);
public:
    DWORD m_style;      ///< styles of the dialog.
    /**
     * Sets the info text of the dialog. Call this method before calling Show().
     */
    void SetInfo(LPCTSTR title);
    /*
     * Sets the text to show for the checkbox. If this method is not called,
     * then no checkbox is added.
     */
    void SetCheckBoxText(LPCTSTR checktext);
    void SetCheckBoxText2(LPCTSTR checktext);
    /**
     * Shows the Dialog.
     * \param parent [in] window handle of the parent window.
     * \param path [out] the path to the folder which the user has selected
     * \return one of CANCEL, NOPATH or OK
     */
    CBrowseFolder::retVal Show(HWND parent, std::wstring& path, const std::wstring& sDefaultPath = std::wstring());
    CBrowseFolder::retVal Show(HWND parent, LPTSTR path, size_t pathlen, LPCTSTR szDefaultPath = NULL);

    /**
     * If this is set to true, then the second checkbox gets disabled as soon as the first
     * checkbox is checked. If the first checkbox is unchecked, then the second checkbox is enabled
     * again.
     */
    void DisableCheckBox2WhenCheckbox1IsEnabled(bool bSet = true) {m_DisableCheckbox2WhenCheckbox1IsChecked = bSet;}

    static BOOL m_bCheck;       ///< state of the checkbox on closing the dialog
    static BOOL m_bCheck2;
    TCHAR m_title[200];
protected:
    static void SetFont(HWND hwnd,LPTSTR FontName,int FontSize);

    static int CALLBACK BrowseCallBackProc(HWND  hwnd,UINT  uMsg,LPARAM  lParam,LPARAM  lpData);
    static LRESULT APIENTRY CheckBoxSubclassProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    static LRESULT APIENTRY CheckBoxSubclassProc2(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

    static WNDPROC CBProc;
    static HWND checkbox;
    static HWND checkbox2;
    static HWND ListView;
    static std::wstring m_sDefaultPath;
    TCHAR m_displayName[200];
    LPITEMIDLIST m_root;
    static TCHAR m_CheckText[200];
    static TCHAR m_CheckText2[200];
    static bool m_DisableCheckbox2WhenCheckbox1IsChecked;
};
