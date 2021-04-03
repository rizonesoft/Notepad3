// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2020-2021 - Stefan Kueng

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
    enum class RetVal
    {
        Cancel = 0, ///< the user has pressed cancel
        Nopath,     ///< no folder was selected
        Ok          ///< everything went just fine
    };

public:
    //constructor / deconstructor
    CBrowseFolder();
    ~CBrowseFolder();

public:
    DWORD m_style; ///< styles of the dialog.
    /**
     * Sets the info text of the dialog. Call this method before calling Show().
     */
    void SetInfo(LPCWSTR title);
    /*
     * Sets the text to show for the checkbox. If this method is not called,
     * then no checkbox is added.
     */
    void SetCheckBoxText(LPCWSTR checkText) const;
    void SetCheckBoxText2(LPCWSTR checkText) const;
    /**
     * Shows the Dialog.
     * \param parent [in] window handle of the parent window.
     * \param path [out] the path to the folder which the user has selected
     * \return one of CANCEL, NOPATH or OK
     */
    CBrowseFolder::RetVal Show(HWND parent, std::wstring& path, const std::wstring& sDefaultPath = std::wstring());
    CBrowseFolder::RetVal Show(HWND parent, LPWSTR path, size_t pathLen, LPCWSTR szDefaultPath = nullptr);

    /**
     * If this is set to true, then the second checkbox gets disabled as soon as the first
     * checkbox is checked. If the first checkbox is unchecked, then the second checkbox is enabled
     * again.
     */
    static void DisableCheckBox2WhenCheckbox1IsEnabled(bool bSet = true) { m_disableCheckbox2WhenCheckbox1IsChecked = bSet; }

    static BOOL m_bCheck; ///< state of the checkbox on closing the dialog
    static BOOL m_bCheck2;
    wchar_t     m_title[200];

protected:
    static void SetFont(HWND hwnd, LPCWSTR fontName, int fontSize);

    static int CALLBACK     BrowseCallBackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
    static LRESULT APIENTRY CheckBoxSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT APIENTRY CheckBoxSubclassProc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static WNDPROC      m_cbProc;
    static HWND         m_checkBox;
    static HWND         m_checkBox2;
    static HWND         m_listView;
    static std::wstring m_sDefaultPath;
    wchar_t             m_displayName[200];
    LPITEMIDLIST        m_root;
    static wchar_t      m_checkText[200];
    static wchar_t      m_checkText2[200];
    static bool         m_disableCheckbox2WhenCheckbox1IsChecked;
};
