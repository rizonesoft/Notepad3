// sktoolslib - common files for SK tools

// Copyright (C) 2013 - Stefan Kueng

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
#include <map>

#define TranslatedString(a,b) CLanguage::Instance().GetTranslatedString(ResString(a,b))

/**
 * Helps with showing a translated UI
 */
class CLanguage
{
private:
    CLanguage() {}
    ~CLanguage() {}
public:
    static CLanguage& Instance();

    /// Loads a po-File containing the original and translated strings
    /// Use ResText to extract the string resources and generate a template po file
    bool LoadFile(const std::wstring& path);

    /// returns a translated string for the original string
    std::wstring GetTranslatedString(const std::wstring& s);
    /// Translates a whole window/dialog and all its controls
    void TranslateWindow(HWND hWnd);
    /// Translates a menu and all submenus
    void TranslateMenu(HMENU hMenu);

private:
    static BOOL CALLBACK TranslateWindowProc(HWND hwnd, LPARAM lParam);
    static std::wstring GetTranslatedString(const std::wstring& s, std::map<std::wstring, std::wstring>* pLangMap);

private:
    std::map<std::wstring, std::wstring>    langmap;
};

