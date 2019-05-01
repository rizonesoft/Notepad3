/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Styles.c                                                                    *
*   Scintilla Style Management                                                *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*   Mostly taken from SciTE, (c) Neil Hodgson                                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2019   *
*                                                 http://www.rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#include "Helpers.h"

#include <assert.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <stdio.h>

#include "SciLexer.h"
#include "SciXLexer.h"

#include "Notepad3.h"
#include "Helpers.h"
#include "Edit.h"
#include "Dialogs.h"
#include "resource.h"
#include "Encoding.h"
#include "MuiLanguage.h"

#include "SciCall.h"

#include "Styles.h"

extern const int g_FontQuality[4];

bool ChooseFontDirectWrite(HWND hwnd, const WCHAR* localeName, DPI_T dpi, LPCHOOSEFONT lpCF);

// ----------------------------------------------------------------------------

// This array holds all the lexers...
// Don't forget to change the number of the lexer for HTML and XML
// in Notepad2.c ParseCommandLine() if you change this array!
static PEDITLEXER g_pLexArray[NUMLEXERS] =
{
  &lexStandard,      // Default Text
  &lexStandard2nd,   // 2nd Default Text
  &lexANSI,          // ANSI Files
  &lexCONF,          // Apache Config Files
  &lexASM,           // Assembly Script
  &lexAHK,           // AutoHotkey Script
  &lexAHKL,          // AutoHotkey L Script
  &lexAU3,           // AutoIt3 Script
  &lexAVS,           // AviSynth Script
  &lexAwk,           // Awk Script
  &lexBAT,           // Batch Files
  &lexCS,            // C# Source Code
  &lexCPP,           // C/C++ Source Code
  &lexCmake,         // Cmake Script
  &lexCOFFEESCRIPT,  // Coffeescript
  &lexPROPS,         // Configuration Files
  &lexCSS,           // CSS Style Sheets
  &lexD,             // D Source Code
  &lexDIFF,          // Diff Files
  &lexGo,            // Go Source Code
  &lexINNO,          // Inno Setup Script
  &lexJAVA,          // Java Source Code
  &lexJS,            // JavaScript
  &lexJSON,          // JSON
  &lexLATEX,         // LaTeX Files
  &lexLUA,           // Lua Script
  &lexMAK,           // Makefiles
  &lexMARKDOWN,      // Markdown
  &lexMATLAB,        // MATLAB
  &lexNim,           // Nim(rod)
  &lexNSIS,          // NSIS Script
  &lexPAS,           // Pascal Source Code
  &lexPL,            // Perl Script
  &lexPS,            // PowerShell Script
  &lexPY,            // Python Script
  &lexRegistry,      // Registry Files
  &lexRC,            // Resource Script
  &lexR,             // R Statistics Code
  &lexRUBY,          // Ruby Script
  &lexRust,          // Rust Script
  &lexBASH,          // Shell Script
  &lexSQL,           // SQL Query
  &lexTCL,           // Tcl Script
  &lexVBS,           // VBScript
  &lexVHDL,          // VHDL
  &lexVB,            // Visual Basic
  &lexHTML,          // Web Source Code
  &lexXML,           // XML Document
  &lexYAML           // YAML
};


// Currently used lexer
static int s_iDefaultLexer = 0;
static PEDITLEXER s_pLexCurrent = &lexStandard;

const COLORREF s_colorDefault[16] = 
{
  RGB(0x00, 0x00, 0x00),
  RGB(0x0A, 0x24, 0x6A),
  RGB(0x3A, 0x6E, 0xA5),
  RGB(0x00, 0x3C, 0xE6),
  RGB(0x00, 0x66, 0x33),
  RGB(0x60, 0x80, 0x20),
  RGB(0x64, 0x80, 0x00),
  RGB(0xA4, 0x60, 0x00),
  RGB(0xFF, 0xFF, 0xFF),
  RGB(0xFF, 0xFF, 0xE2),
  RGB(0xFF, 0xF1, 0xA8),
  RGB(0xFF, 0xC0, 0x00),
  RGB(0xFF, 0x40, 0x00),
  RGB(0xC8, 0x00, 0x00),
  RGB(0xB0, 0x00, 0xB0),
  RGB(0xB2, 0x8B, 0x40)
};

static COLORREF s_colorCustom[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static bool s_bAutoSelect = true;

#define STYLESELECTDLG_X 304
#define STYLESELECTDLG_Y 344
static int  s_cxStyleSelectDlg = STYLESELECTDLG_X;
static int  s_cyStyleSelectDlg = STYLESELECTDLG_Y;


//=============================================================================

typedef struct _themeFiles
{
  UINT    rid;
  WCHAR   szName[80];
  WCHAR   szFilePath[MAX_PATH];

} THEMEFILES, * PTHEMEFILES;

static THEMEFILES Theme_Files[] =
{
  { 0, L"Default", L"" },
  { 0, L"Standard", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" },
  { 0, L"", L"" }
};
unsigned ThemeItems_CountOf() { return COUNTOF(Theme_Files); }
static unsigned s_idxSelectedTheme = 1;  // Default(0), Standard(1)

const WCHAR* const STYLING_THEME_NAME = L"ThemeFileName";

static void _FillThemesMenuTable()
{
  Theme_Files[0].rid = IDM_THEMES_DEFAULT;    // factory default
  Theme_Files[1].rid = IDM_THEMES_FILE_ITEM;  // NP3.ini settings
  // names are filled by Style_InsertThemesMenu()
  StringCchCopy(Theme_Files[1].szFilePath, COUNTOF(Theme_Files[1].szFilePath), Globals.IniFile);

  unsigned iTheme = 1; // Standard

  WCHAR tchThemeDir[MAX_PATH] = { L'\0' };
  // find "themes" sub-dir (side-by-side to Notepad3.ini)
  if (StrIsNotEmpty(Globals.IniFile)) {
    StringCchCopy(tchThemeDir, COUNTOF(tchThemeDir), Globals.IniFile);
  }
  else if (StrIsNotEmpty(Globals.IniFileDefault)) {
    StringCchCopy(tchThemeDir, COUNTOF(tchThemeDir), Globals.IniFileDefault);
  }
  if (StrIsNotEmpty(tchThemeDir)) {
    PathCchRemoveFileSpec(tchThemeDir, COUNTOF(tchThemeDir));
    PathCchAppend(tchThemeDir, COUNTOF(tchThemeDir), L"themes");
  }

  if (PathIsDirectory(tchThemeDir))
  {
    WCHAR tchThemePath[MAX_PATH] = { L'\0' };
    StringCchCopy(tchThemePath, COUNTOF(tchThemePath), tchThemeDir);
    PathCchAppend(tchThemePath, COUNTOF(tchThemePath), L"*.ini");

    WIN32_FIND_DATA FindFileData;
    ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATA));
    HANDLE hFindFile = FindFirstFile(tchThemePath, &FindFileData);
    if (hFindFile != INVALID_HANDLE_VALUE)
    {
      // ---  fill table by directory entries  ---
      for (iTheme = 2; iTheme < ThemeItems_CountOf(); ++iTheme)
      {
        Theme_Files[iTheme].rid = (iTheme + IDM_THEMES_DEFAULT);

        StringCchCopy(tchThemePath, COUNTOF(tchThemePath), PathFindFileName(FindFileData.cFileName));
        PathRemoveExtension(tchThemePath);
        StringCchCopy(Theme_Files[iTheme].szName, COUNTOF(Theme_Files[iTheme].szName), tchThemePath);

        StringCchCopy(tchThemePath, COUNTOF(tchThemePath), tchThemeDir);
        PathCchAppend(tchThemePath, COUNTOF(tchThemePath), FindFileData.cFileName);
        StringCchCopy(Theme_Files[iTheme].szFilePath, COUNTOF(Theme_Files[iTheme].szFilePath), tchThemePath);

        if (!FindNextFile(hFindFile, &FindFileData)) { break; }
      }
    }
    FindClose(hFindFile);
  }

  for (++iTheme; iTheme < ThemeItems_CountOf(); ++iTheme) 
  {
    Theme_Files[iTheme].rid = 0;   // no themes available
    Theme_Files[iTheme].szName[0] = L'\0';
    Theme_Files[iTheme].szFilePath[0] = L'\0';
  }
}



//=============================================================================
//
//  Style_SetIniFile()
//
void Style_SetIniFile(LPCWSTR szIniFile)
{
  StringCchCopy(Theme_Files[1].szFilePath, COUNTOF(Theme_Files[1].szFilePath), szIniFile);
  _FillThemesMenuTable();
}


//=============================================================================
//
//  Style_InsertThemesMenu()
//
bool Style_InsertThemesMenu(HMENU hMenuBar)
{
  HMENU hmenuThemes = CreatePopupMenu();
  int const pos = GetMenuItemCount(hMenuBar) - 1;

  GetLngString(Theme_Files[0].rid, Theme_Files[0].szName, COUNTOF(Theme_Files[0].szName));
  GetLngString(Theme_Files[1].rid, Theme_Files[1].szName, COUNTOF(Theme_Files[1].szName));

  for (unsigned i = 0; i < ThemeItems_CountOf(); ++i)
  {
    if (i == 2) {
      AppendMenu(hmenuThemes, MF_SEPARATOR, 0, 0);
    }
    if (Theme_Files[i].rid > 0) {
      AppendMenu(hmenuThemes, MF_ENABLED | MF_STRING, Theme_Files[i].rid, Theme_Files[i].szName);
    }
    else {
      break; // done
    }
  }

  // --- insert ---
  WCHAR wchMenuItemStrg[80] = { L'\0' };
  GetLngString(IDS_MUI_MENU_THEMES, wchMenuItemStrg, COUNTOF(wchMenuItemStrg));

  bool const res = InsertMenu(hMenuBar, pos, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)hmenuThemes, wchMenuItemStrg);

  CheckCmd(hMenuBar, Theme_Files[s_idxSelectedTheme].rid, true);

  if (StrIsEmpty(Theme_Files[s_idxSelectedTheme].szFilePath)) 
  {
    EnableCmd(hMenuBar, Theme_Files[s_idxSelectedTheme].rid, false);
  }

  return res;
}


//=============================================================================
//
//  Style_DynamicThemesMenuCmd() - Handles IDS_MUI_MENU_THEMES messages
//
//
void Style_DynamicThemesMenuCmd(int cmd, bool bEnableSaveSettings)
{
  unsigned const iThemeIdx = (unsigned)(cmd - IDM_THEMES_DEFAULT); // consecutive IDs

  if ((iThemeIdx < 0) || (iThemeIdx >= ThemeItems_CountOf())) {
    return;
  }
  if (iThemeIdx == s_idxSelectedTheme) { return; }

  CheckCmd(Globals.hMainMenu, Theme_Files[s_idxSelectedTheme].rid, false);

  if (Settings.SaveSettings) {
    if (s_idxSelectedTheme == 0) {
      // nothing to do: internal defaults
    }
    else if (s_idxSelectedTheme == 1) {
      if (bEnableSaveSettings) {
        CreateIniFile();
        if (StrIsNotEmpty(Globals.IniFile)) {
          Style_ExportToFile(Globals.IniFile, false);
        }
      }
    }
    else if (PathFileExists(Theme_Files[s_idxSelectedTheme].szFilePath))
    {
      bool const bIndependentFromStandardSettings = true;
      Style_ExportToFile(Theme_Files[s_idxSelectedTheme].szFilePath, bIndependentFromStandardSettings);
    }
  }

  s_idxSelectedTheme = iThemeIdx;

  bool result = true;
  if ((s_idxSelectedTheme > 1) && PathFileExists(Theme_Files[s_idxSelectedTheme].szFilePath))
  {
    result = Style_ImportFromFile(Theme_Files[s_idxSelectedTheme].szFilePath);
  }
  else if (s_idxSelectedTheme == 1) {
    result = Style_ImportFromFile(Globals.IniFile);
  }
  else {
    result = Style_ImportFromFile(L"");
  }

  if (result) {
    Style_ResetCurrentLexer(Globals.hwndEdit);
    SendWMSize(Globals.hwndMain, NULL);
    UpdateUI();
    UpdateAllBars(true);
  }

  CheckCmd(Globals.hMainMenu, Theme_Files[s_idxSelectedTheme].rid, true);
}






//=============================================================================
//
//  IsLexerStandard()
//

bool  IsLexerStandard(PEDITLEXER pLexer)
{
  return ( pLexer && ((pLexer == &lexStandard) || (pLexer == &lexStandard2nd)) );
}

PEDITLEXER  GetCurrentStdLexer()
{
  return (Style_GetUse2ndDefault() ? &lexStandard2nd : &lexStandard);
}

bool  IsStyleStandardDefault(PEDITSTYLE pStyle)
{
  return (pStyle && ((pStyle->rid == IDS_LEX_STD_STYLE) || (pStyle->rid == IDS_LEX_2ND_STYLE)));
}

bool  IsStyleSchemeDefault(PEDITSTYLE pStyle)
{
  return (pStyle && (pStyle->rid == IDS_LEX_STR_63126));
}

PEDITLEXER  GetDefaultLexer()
{
  return g_pLexArray[s_iDefaultLexer];
}



//=============================================================================
//
//  IsLexerStandard()
//
bool Style_IsCurLexerStandard()
{
  return IsLexerStandard(s_pLexCurrent);
}


//=============================================================================
//
//  Style_GetBaseFontSize()
//

static float  _SetBaseFontSize(float fSize)
{
  static float fBaseFontSize = 10.0f;

  if (fSize >= 0.0f) {
    fBaseFontSize = Round10th(fSize);
  }
  return fBaseFontSize;
}

//=============================================================================
//
//  Style_GetBaseFontSize()
//
float Style_GetBaseFontSize()
{
  return _SetBaseFontSize(-1.0);
}



//=============================================================================
//
//  Style_RgbAlpha()
//
int  Style_RgbAlpha(int rgbFore, int rgbBack, int alpha)
{
  return (int)RGB(\
    (0xFF - alpha) * (int)GetRValue(rgbBack) / 0xFF + alpha * (int)GetRValue(rgbFore) / 0xFF, \
                  (0xFF - alpha) * (int)GetGValue(rgbBack) / 0xFF + alpha * (int)GetGValue(rgbFore) / 0xFF, \
                  (0xFF - alpha) * (int)GetBValue(rgbBack) / 0xFF + alpha * (int)GetBValue(rgbFore) / 0xFF);
}


//=============================================================================
//
//  _SetCurrentFontSize(), _GetCurrentFontSize()
//
static float  _SetCurrentFontSize(float fSize)
{
  static float fCurrentFontSize = 10.0f;

  if (signbit(fSize) == 0) {
    float const fSizeR10th = Round10th(fSize);
    fCurrentFontSize = (0.5f < fSizeR10th) ? fSizeR10th :  0.5f;
  }
  return fCurrentFontSize;
}

float Style_GetCurrentFontSize()
{
  return _SetCurrentFontSize(-1.0f);
}


//=============================================================================
//
//  Style_Load()
//
void Style_Load()
{
  float const fBFS = GetBaseFontSize(Globals.hwndMain);
  _SetBaseFontSize(fBFS);
  _SetCurrentFontSize(fBFS);

  for (int i = 0; i < 16; ++i) {
    s_colorCustom[i] = s_colorDefault[i];
  }

  // 2nd Default Style has same filename extension list as (1st) Default Style
  StringCchCopyW(lexStandard2nd.szExtensions, COUNTOF(lexStandard2nd.szExtensions), lexStandard.szExtensions);

  _FillThemesMenuTable();

  // get theme name from settings2
  WCHAR wchThemeName[80];
  IniGetString(L"Styles", STYLING_THEME_NAME, L"", wchThemeName, COUNTOF(wchThemeName));

  unsigned iTheme = 1;
  if (StrIsNotEmpty(wchThemeName)) {
    for (; iTheme < ThemeItems_CountOf(); ++iTheme)
    {
      if (StringCchCompareXI(wchThemeName, Theme_Files[iTheme].szName) == 0) { break; }
    }
  }
  s_idxSelectedTheme = (iTheme < ThemeItems_CountOf()) ? iTheme : 1;

  Style_ImportFromFile(Theme_Files[s_idxSelectedTheme].szFilePath);
}


//=============================================================================
//
//  Style_Import()
//
bool Style_Import(HWND hwnd)
{
  WCHAR szFile[MAX_PATH] = { L'\0' };
  WCHAR szFilter[MAX_PATH] = { L'\0' };
  OPENFILENAME ofn;

  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  GetLngString(IDS_MUI_FILTER_INI, szFilter, COUNTOF(szFilter));
  PrepareFilterStr(szFilter);

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrDefExt = L"ini";
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
    | OFN_PATHMUSTEXIST | OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/;

  bool result = false;

  if (GetOpenFileName(&ofn))
  {
    result = Style_ImportFromFile(szFile);
  }
  return result;
}


//=============================================================================
//
//  Style_ImportFromFile()
//
bool Style_ImportFromFile(const WCHAR* szFile)
{
  bool bResetToDefault = (!szFile || szFile[0] == L'\0') ? true : false;

  bool result = false;

  size_t const len = NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER * 100;
  WCHAR* pIniSection = AllocMem(len * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pIniSection) { ZeroMemory(pIniSection, len * sizeof(WCHAR)); }

  if (pIniSection) {

    if (!bResetToDefault) 
    {
      WCHAR tch[32] = { L'\0' };

      GetPrivateProfileSection(L"Custom Colors", pIniSection, (int)len, szFile);

      for (int i = 0; i < 16; i++) {
        WCHAR wch[32] = { L'\0' };
        StringCchPrintf(tch, COUNTOF(tch), L"%02i", i + 1);
        int itok = 0;
        if (IniSectionGetString(pIniSection, tch, L"", wch, COUNTOF(wch))) {
          if (wch[0] == L'#') {
            unsigned int irgb;
            itok = swscanf_s(CharNext(wch), L"%x", &irgb);
            if (itok == 1) {
              s_colorCustom[i] = RGB((irgb & 0xFF0000) >> 16, (irgb & 0xFF00) >> 8, irgb & 0xFF);
            }
          }
        }
        if (itok != 1) {
          s_colorCustom[i] = s_colorDefault[i];
        }
      }
    }

    // Styles
    if (!bResetToDefault) {
      GetPrivateProfileSection(L"Styles", pIniSection, (int)len, szFile);
    }

    // 2nd default
    Style_SetUse2ndDefault(IniSectionGetBool(pIniSection, L"Use2ndDefaultStyle", false));

    // default scheme
    s_iDefaultLexer = clampi(IniSectionGetInt(pIniSection, L"DefaultScheme", 0), 0, COUNTOF(g_pLexArray) - 1);

    // auto select
    s_bAutoSelect = IniSectionGetBool(pIniSection, L"AutoSelect", true);

    // scheme select dlg dimensions
    s_cxStyleSelectDlg = clampi(IniSectionGetInt(pIniSection, L"SelectDlgSizeX", STYLESELECTDLG_X), 0, 8192);
    s_cyStyleSelectDlg = clampi(IniSectionGetInt(pIniSection, L"SelectDlgSizeY", STYLESELECTDLG_Y), 0, 8192);


    // Lexer 
    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++) {

      if (!bResetToDefault) {
        GetPrivateProfileSection(g_pLexArray[iLexer]->pszName, pIniSection, (int)len, szFile);
      }

      IniSectionGetString(pIniSection, L"FileNameExtensions", g_pLexArray[iLexer]->pszDefExt,
        g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions));

      unsigned i = 0;
      while (g_pLexArray[iLexer]->Styles[i].iStyle != -1)
      {
        IniSectionGetString(pIniSection, g_pLexArray[iLexer]->Styles[i].pszName,
          g_pLexArray[iLexer]->Styles[i].pszDefault,
          g_pLexArray[iLexer]->Styles[i].szValue,
          COUNTOF(g_pLexArray[iLexer]->Styles[i].szValue));
        ++i;
      }
    }
    FreeMem(pIniSection);
    result = true;
  }
  return result;
}


//=============================================================================
//
//  Style_Save()
//
void Style_Save()
{
  Style_ExportToFile(Theme_Files[s_idxSelectedTheme].szFilePath, false);
  IniSetString(L"Styles", STYLING_THEME_NAME, (s_idxSelectedTheme > 1) ? Theme_Files[s_idxSelectedTheme].szName : NULL);
}


//=============================================================================
//
//  Style_Export()
//
bool Style_Export(HWND hwnd)
{
  WCHAR szFile[MAX_PATH] = { L'\0' };
  WCHAR szFilter[256] = { L'\0' };
  OPENFILENAME ofn;

  ZeroMemory(&ofn,sizeof(OPENFILENAME));
  GetLngString(IDS_MUI_FILTER_INI,szFilter,COUNTOF(szFilter));
  PrepareFilterStr(szFilter);

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrFile = szFile;
  ofn.lpstrDefExt = L"ini";
  ofn.nMaxFile = COUNTOF(szFile);
  ofn.Flags = /*OFN_FILEMUSTEXIST |*/ OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
            | OFN_PATHMUSTEXIST | OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/ | OFN_OVERWRITEPROMPT;

  Globals.dwLastError = ERROR_SUCCESS;

  if (GetSaveFileName(&ofn)) 
  {
    Globals.dwLastError = Style_ExportToFile(szFile, true);

    if (Globals.dwLastError != ERROR_SUCCESS) {
      InfoBoxLng(MB_ICONERROR, NULL, IDS_MUI_EXPORT_FAIL, szFile);
    }
  }
  return (Globals.dwLastError == ERROR_SUCCESS);
}


//=============================================================================
//
//  Style_ExportToFile()
//
DWORD Style_ExportToFile(const WCHAR* szFile, bool bForceAll)
{

  if (StrIsEmpty(szFile)) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SETTINGSNOTSAVED);
    return false;
  }

  size_t const len = NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER * 100;
  WCHAR* pIniSection = AllocMem(len * sizeof(WCHAR), HEAP_ZERO_MEMORY);
  if (pIniSection) { ZeroMemory(pIniSection, len * sizeof(WCHAR)); }

  DWORD dwError = ERROR_NOT_ENOUGH_MEMORY;

  if (pIniSection) {

    // Custom colors
    for (int i = 0; i < 16; i++) {
      if (bForceAll || (s_colorCustom[i] != s_colorDefault[i])) {
        WCHAR tch[32] = { L'\0' };
        WCHAR wch[32] = { L'\0' };
        StringCchPrintf(tch, COUNTOF(tch), L"%02i", i + 1);
        StringCchPrintf(wch, COUNTOF(wch), L"#%02X%02X%02X",
          (int)GetRValue(s_colorCustom[i]), (int)GetGValue(s_colorCustom[i]), (int)GetBValue(s_colorCustom[i]));
        IniSectionSetString(pIniSection, tch, wch);
      }
    }
    if (!WritePrivateProfileSection(L"Custom Colors", pIniSection, szFile)) {
      dwError = GetLastError();
    }
    ZeroMemory(pIniSection, len * sizeof(WCHAR));


    // auto select
    bool const bUse2ndSty = Style_GetUse2ndDefault();
    if (bUse2ndSty) {
      IniSectionSetBool(pIniSection, L"Use2ndDefaultStyle", bUse2ndSty);
    }
    // default scheme
    if (s_iDefaultLexer != 0) {
      IniSectionSetInt(pIniSection, L"DefaultScheme", s_iDefaultLexer);
    }
    // auto select
    if (!s_bAutoSelect) {
      IniSectionSetInt(pIniSection, L"AutoSelect", s_bAutoSelect);
    }

    // scheme select dlg dimensions
    if (s_cxStyleSelectDlg != STYLESELECTDLG_X) {
      IniSectionSetInt(pIniSection, L"SelectDlgSizeX", s_cxStyleSelectDlg);
    }
    if (s_cyStyleSelectDlg != STYLESELECTDLG_Y) {
      IniSectionSetInt(pIniSection, L"SelectDlgSizeY", s_cyStyleSelectDlg);
    }
    if (!WritePrivateProfileSection(L"Styles", pIniSection, szFile)) {
      dwError = GetLastError();
    }
    ZeroMemory(pIniSection, len * sizeof(WCHAR));


    // create canonical order of lexer sections
    if (bForceAll) {
      for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++)
      {
        WritePrivateProfileSection(g_pLexArray[iLexer]->pszName, L"\0", szFile);
      }
    }

    WCHAR szTmpStyle[BUFSIZE_STYLE_VALUE];

    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer)
    {
      if (bForceAll || (StringCchCompareXI(g_pLexArray[iLexer]->szExtensions, g_pLexArray[iLexer]->pszDefExt) != 0))
      {
        IniSectionSetString(pIniSection, L"FileNameExtensions", g_pLexArray[iLexer]->szExtensions);
      }

      unsigned i = 0;
      while (g_pLexArray[iLexer]->Styles[i].iStyle != -1)
      {
        // normalize defaults
        szTmpStyle[0] = L'\0'; // clear
        Style_CopyStyles_IfNotDefined(g_pLexArray[iLexer]->Styles[i].pszDefault, szTmpStyle, COUNTOF(szTmpStyle), true, true);

        if (bForceAll || (StringCchCompareXI(g_pLexArray[iLexer]->Styles[i].szValue, szTmpStyle) != 0))
        {
          // normalize value
          szTmpStyle[0] = L'\0'; // clear
          Style_CopyStyles_IfNotDefined(g_pLexArray[iLexer]->Styles[i].szValue, szTmpStyle, COUNTOF(szTmpStyle), true, true);
          IniSectionSetString(pIniSection, g_pLexArray[iLexer]->Styles[i].pszName, szTmpStyle);
        }
        ++i;
      }

      dwError = ERROR_SUCCESS;
      if (!WritePrivateProfileSection(g_pLexArray[iLexer]->pszName, pIniSection, szFile)) {
        dwError = GetLastError();
      }
      ZeroMemory(pIniSection, len * sizeof(WCHAR));
    }
    FreeMem(pIniSection);
  }
  return dwError;
}


//=============================================================================
//
//  Style_SetLexer()
//
void Style_SetLexer(HWND hwnd, PEDITLEXER pLexNew) 
{
  int iValue;
  COLORREF rgb;
  COLORREF dColor;

  WCHAR wchSpecificStyle[80] = { L'\0' };

  // Select standard if NULL is specified
  if (!pLexNew) {
    pLexNew = GetDefaultLexer();
    if (IsLexerStandard(pLexNew)) {
      pLexNew = GetCurrentStdLexer();
    }
  }
  const WCHAR* const wchNewLexerStyleStrg = pLexNew->Styles[STY_DEFAULT].szValue;

  // first set standard lexer's default values
  if (IsLexerStandard(pLexNew)) {
    s_pLexCurrent = pLexNew;
    Style_SetUse2ndDefault(s_pLexCurrent == &lexStandard2nd); // sync if forced
  }
  else {
    s_pLexCurrent = GetCurrentStdLexer();
  }

  // Lexer 
  SendMessage(hwnd, SCI_SETLEXER, pLexNew->lexerID, 0);

  // Lexer very specific styles
  if (pLexNew->lexerID == SCLEX_XML)
    SciCall_SetProperty("lexer.xml.allow.scripts", "1");
  if (pLexNew->lexerID == SCLEX_CPP) {
    SciCall_SetProperty("styling.within.preprocessor", "1");
    SciCall_SetProperty("lexer.cpp.track.preprocessor", "0");
    SciCall_SetProperty("lexer.cpp.update.preprocessor", "0");
  }
  else if (pLexNew->lexerID == SCLEX_PASCAL)
    SciCall_SetProperty("lexer.pascal.smart.highlighting", "1");
  else if (pLexNew->lexerID == SCLEX_SQL) {
    SciCall_SetProperty("sql.backslash.escapes", "1");
    SciCall_SetProperty("lexer.sql.backticks.identifier", "1");
    SciCall_SetProperty("lexer.sql.numbersign.comment", "1");
  }
  else if (pLexNew->lexerID == SCLEX_NSIS)
    SciCall_SetProperty("nsis.ignorecase", "1");
  else if (pLexNew->lexerID == SCLEX_CSS) {
    SciCall_SetProperty("lexer.css.scss.language", "1");
    SciCall_SetProperty("lexer.css.less.language", "1");
  }
  else if (pLexNew->lexerID == SCLEX_JSON) {
    SciCall_SetProperty("json.allow.comments", "1");
    SciCall_SetProperty("json.escape.sequence", "1");
  }

  // Code folding
  switch (pLexNew->lexerID)
  {
  case SCLEX_NULL:
  case SCLEX_CONTAINER:
  case SCLEX_BATCH:
  case SCLEX_CONF:
  case SCLEX_MAKEFILE:
  case SCLEX_MARKDOWN:
    Globals.bCodeFoldingAvailable = false;
    SciCall_SetProperty("fold", "0");
    break;
  default:
    Globals.bCodeFoldingAvailable = true;
    SciCall_SetProperty("fold", "1");
    SciCall_SetProperty("fold.compact", "0");
    SciCall_SetProperty("fold.comment", "1");
    SciCall_SetProperty("fold.html", "1");
    SciCall_SetProperty("fold.preprocessor", "1");
    SciCall_SetProperty("fold.cpp.comment.explicit", "0");
    break;
  }

  // Add KeyWord Lists
  for (int i = 0; i < (KEYWORDSET_MAX + 1); i++) {
    SciCall_SetKeywords(i, pLexNew->pKeyWords->pszKeyWords[i]);
  }

  // --------------------------------------------------------------------------

  // Clear
  SendMessage(hwnd, SCI_CLEARDOCUMENTSTYLE, 0, 0);

  // Default Values are always set
  SendMessage(hwnd, SCI_STYLERESETDEFAULT, 0, 0);


  // constants
  SendMessage(hwnd, SCI_STYLESETVISIBLE, STYLE_DEFAULT, (LPARAM)true);
  SendMessage(hwnd, SCI_STYLESETHOTSPOT, STYLE_DEFAULT, (LPARAM)false);       // default hotspot off
                                                                              // Auto-select codepage according to charset
  //~Style_SetACPfromCharSet(hwnd);

  // ---  apply/init  default style  ---
  float const fBFS = GetBaseFontSize(Globals.hwndMain);
  _SetBaseFontSize(fBFS);
  _SetCurrentFontSize(fBFS);
  const WCHAR* const wchStandardStyleStrg = s_pLexCurrent->Styles[STY_DEFAULT].szValue;
  Style_SetStyles(hwnd, STYLE_DEFAULT, wchStandardStyleStrg, true);

  // ---  apply current scheme specific settings to default style  ---
  if (IsLexerStandard(pLexNew))
  {
    // styles ar already set
    EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CURRENTSCHEME, false);
  }
  else {
    // merge lexer default styles
    Style_SetStyles(hwnd, STYLE_DEFAULT, wchNewLexerStyleStrg, false);

    EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CURRENTSCHEME, true && !IsWindow(Globals.hwndDlgCustomizeSchemes));
  }

  // Broadcast STYLE_DEFAULT as base style to all other styles
  SendMessage(hwnd, SCI_STYLECLEARALL, 0, 0);

  // --------------------------------------------------------------------------
  
  const PEDITLEXER pCurrentStandard = s_pLexCurrent;
  
  // --------------------------------------------------------------------------

  Style_SetMargin(hwnd, pCurrentStandard->Styles[STY_MARGIN].iStyle,
                  pCurrentStandard->Styles[STY_MARGIN].szValue); // margin (line number, bookmarks, folding) style

  if (Settings2.UseOldStyleBraceMatching) {
    Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_OK].iStyle,
      pCurrentStandard->Styles[STY_BRACE_OK].szValue, false); // brace light
  }
  else {
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_BRACE_OK].szValue, FOREGROUND_LAYER, &dColor))
      SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_MATCH_BRACE, dColor);
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_OK].szValue, &iValue, true))
      SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_MATCH_BRACE, iValue);
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_OK].szValue, &iValue, false))
      SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MATCH_BRACE, iValue);

    iValue = -1; // need for retrieval
    if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_BRACE_OK].szValue, 0, &iValue)) {
      // got default, get string
      StringCchCatW(pCurrentStandard->Styles[STY_BRACE_OK].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
      Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &iValue);
      StringCchCatW(pCurrentStandard->Styles[STY_BRACE_OK].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchSpecificStyle);
    }
    SendMessage(hwnd, SCI_INDICSETSTYLE, INDIC_NP3_MATCH_BRACE, iValue);
  }
  if (Settings2.UseOldStyleBraceMatching) {
    Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_BAD].iStyle,
      pCurrentStandard->Styles[STY_BRACE_BAD].szValue, false); // brace bad
  }
  else {
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, FOREGROUND_LAYER, &dColor))
      SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_BAD_BRACE, dColor);
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, &iValue, true))
      SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_BAD_BRACE, iValue);
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, &iValue, false))
      SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_BAD_BRACE, iValue);

    iValue = -1; // need for retrieval
    if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, 0, &iValue)) {
      // got default, get string
      StringCchCatW(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
      Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &iValue);
      StringCchCatW(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchSpecificStyle);
    }
    SendMessage(hwnd, SCI_INDICSETSTYLE, INDIC_NP3_BAD_BRACE, iValue);
  }

  // Occurrences Marker
  if (!Style_StrGetColor(pCurrentStandard->Styles[STY_MARK_OCC].szValue, FOREGROUND_LAYER, &dColor))
  {
    switch (Settings.MarkOccurrences) {
    case 1:
      dColor = RGB(0xFF, 0x00, 0x00);
      break;
    case 2:
      dColor = RGB(0x00, 0xFF, 0x00);
      break;
    case 3:
      dColor = RGB(0x00, 0xFF, 0x00);
      break;
    default:
      dColor = GetSysColor(COLOR_HIGHLIGHT);
      break;
    }
    WCHAR sty[32] = { L'\0' };
    StringCchPrintf(sty, COUNTOF(sty), L"fore:#%02X%02X%02X", (int)GetRValue(dColor), (int)GetGValue(dColor), (int)GetBValue(dColor));
    StringCchCopy(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), sty);
  }
  SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_MARK_OCCURANCE, dColor);

  if (!Style_StrGetAlpha(pCurrentStandard->Styles[STY_MARK_OCC].szValue, &iValue, true)) {
    iValue = 100; // force
    StringCchCatW(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; alpha:100");
  }   
  SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_MARK_OCCURANCE, iValue);

  if (!Style_StrGetAlpha(pCurrentStandard->Styles[STY_MARK_OCC].szValue, &iValue, false)) {
    iValue = 100; // force
    StringCchCatW(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; alpha2:100");
  }
  SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MARK_OCCURANCE, iValue);

  iValue = -1; // need for retrieval
  if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_MARK_OCC].szValue, 0, &iValue)) {
    // got default, get string
    StringCchCatW(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
    Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &iValue);
    StringCchCatW(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchSpecificStyle);
  }
  SendMessage(hwnd, SCI_INDICSETSTYLE, INDIC_NP3_MARK_OCCURANCE, iValue);


  // Inline-IME Color
  #define _SC_INDIC_IME_INPUT     (INDIC_IME + 0)
  #define _SC_INDIC_IME_TARGET    (INDIC_IME + 1)
  #define _SC_INDIC_IME_CONVERTED (INDIC_IME + 2)
  #define _SC_INDIC_IME_UNKNOWN    INDIC_IME_MAX

  if (Style_StrGetColor(pCurrentStandard->Styles[STY_IME_COLOR].szValue, FOREGROUND_LAYER, &dColor)) { // IME foregr
    SendMessage(hwnd, SCI_INDICSETFORE, _SC_INDIC_IME_INPUT, dColor);
    SendMessage(hwnd, SCI_INDICSETFORE, _SC_INDIC_IME_TARGET, dColor);
    SendMessage(hwnd, SCI_INDICSETFORE, _SC_INDIC_IME_CONVERTED, dColor);
    SendMessage(hwnd, SCI_INDICSETFORE, _SC_INDIC_IME_UNKNOWN, dColor);
  }

  // More default values...

  if (pLexNew != &lexANSI) {
    Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_CTRL_CHR].iStyle, pCurrentStandard->Styles[STY_CTRL_CHR].szValue, false); // control char
  }
  Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_INDENT_GUIDE].iStyle, pCurrentStandard->Styles[STY_INDENT_GUIDE].szValue, false); // indent guide

  if (Style_StrGetColor(pCurrentStandard->Styles[STY_SEL_TXT].szValue, FOREGROUND_LAYER, &rgb)) { // selection fore
    SendMessage(hwnd, SCI_SETSELFORE, true, rgb);
    SendMessage(hwnd, SCI_SETADDITIONALSELFORE, rgb, 0);
  }
  else {
    SendMessage(hwnd, SCI_SETSELFORE, 0, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELFORE, 0, 0);
  }

  if (Style_StrGetColor(pCurrentStandard->Styles[STY_SEL_TXT].szValue, BACKGROUND_LAYER , &dColor)) { // selection back
    SendMessage(hwnd, SCI_SETSELBACK, true, dColor);
    SendMessage(hwnd, SCI_SETADDITIONALSELBACK, dColor, 0);
  }
  else {
    SendMessage(hwnd, SCI_SETSELBACK, true, RGB(0xC0, 0xC0, 0xC0)); // use a default value...
    SendMessage(hwnd, SCI_SETADDITIONALSELBACK, RGB(0xC0, 0xC0, 0xC0), 0);
  }

  if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_SEL_TXT].szValue, &iValue, true)) { // selection alpha
    SendMessage(hwnd, SCI_SETSELALPHA, iValue, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELALPHA, iValue, 0);
  }
  else {
    SendMessage(hwnd, SCI_SETSELALPHA, SC_ALPHA_NOALPHA, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELALPHA, SC_ALPHA_NOALPHA, 0);
  }

  if (StrStrI(pCurrentStandard->Styles[STY_SEL_TXT].szValue, L"eolfilled")) // selection eolfilled
    SendMessage(hwnd, SCI_SETSELEOLFILLED, 1, 0);
  else
    SendMessage(hwnd, SCI_SETSELEOLFILLED, 0, 0);

  if (Style_StrGetColor(pCurrentStandard->Styles[STY_WHITESPACE].szValue, FOREGROUND_LAYER, &rgb)) // whitespace fore
    SendMessage(hwnd, SCI_SETWHITESPACEFORE, true, rgb);
  else
    SendMessage(hwnd, SCI_SETWHITESPACEFORE, 0, 0);

  if (Style_StrGetColor(pCurrentStandard->Styles[STY_WHITESPACE].szValue, BACKGROUND_LAYER, &rgb)) // whitespace back
    SendMessage(hwnd, SCI_SETWHITESPACEBACK, true, rgb);
  else
    SendMessage(hwnd, SCI_SETWHITESPACEBACK, 0, 0);    // use a default value...

  // whitespace dot size
  iValue = 1;
  float fValue = 1.0;
  if (Style_StrGetSize(pCurrentStandard->Styles[STY_WHITESPACE].szValue, &fValue)) 
  {
    iValue = clampi(float2int(fValue), 0, 12);

    WCHAR tch[32] = { L'\0' };
    WCHAR wchStyle[BUFSIZE_STYLE_VALUE];
    StringCchCopyN(wchStyle, COUNTOF(wchStyle), pCurrentStandard->Styles[STY_WHITESPACE].szValue, 
                   COUNTOF(pCurrentStandard->Styles[STY_WHITESPACE].szValue));

    StringCchPrintf(pCurrentStandard->Styles[STY_WHITESPACE].szValue, 
                    COUNTOF(pCurrentStandard->Styles[STY_WHITESPACE].szValue), L"size:%i", iValue);

    if (Style_StrGetColor(wchStyle, FOREGROUND_LAYER, &rgb)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; fore:#%02X%02X%02X",
        (int)GetRValue(rgb),
        (int)GetGValue(rgb),
        (int)GetBValue(rgb));
      StringCchCat(pCurrentStandard->Styles[STY_WHITESPACE].szValue, 
                   COUNTOF(pCurrentStandard->Styles[STY_WHITESPACE].szValue), tch);
    }

    if (Style_StrGetColor(wchStyle, BACKGROUND_LAYER, &rgb)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; back:#%02X%02X%02X",
        (int)GetRValue(rgb),
        (int)GetGValue(rgb),
        (int)GetBValue(rgb));
      StringCchCat(pCurrentStandard->Styles[STY_WHITESPACE].szValue, 
                   COUNTOF(pCurrentStandard->Styles[STY_WHITESPACE].szValue), tch);
    }
  }
  SendMessage(hwnd, SCI_SETWHITESPACESIZE, iValue, 0);

  // current line background
  Style_HighlightCurrentLine(hwnd, Settings.HighlightCurrentLine);

  // bookmark line or marker
  Style_SetBookmark(hwnd, Settings.ShowSelectionMargin);

  // caret style and width

  int ovr_mask = CARETSTYLE_OVERSTRIKE_BLOCK;
  if (StrStr(pCurrentStandard->Styles[STY_CARET].szValue, L"ovrbar")) {
    ovr_mask = CARETSTYLE_OVERSTRIKE_BAR;
  }

  if (StrStr(pCurrentStandard->Styles[STY_CARET].szValue, L"block")) {
    StringCchCat(wchSpecificStyle, COUNTOF(wchSpecificStyle), L"; block");
    SendMessage(hwnd, SCI_SETCARETSTYLE, (CARETSTYLE_BLOCK | ovr_mask), 0);
  }
  else {
    SendMessage(hwnd, SCI_SETCARETSTYLE, (CARETSTYLE_LINE | ovr_mask), 0);

    iValue = 1;
    fValue = 1.0f;  // default caret width
    WCHAR wch[32] = { L'\0' };
    if (Style_StrGetSize(pCurrentStandard->Styles[STY_CARET].szValue, &fValue)) {
      iValue = clampi(float2int(fValue), 1, 3); // don't allow invisible 0
    }
    SendMessage(hwnd, SCI_SETCARETWIDTH, iValue, 0);

    StringCchPrintf(wch, COUNTOF(wch), L"; size:%i", iValue);
    StringCchCat(wchSpecificStyle, COUNTOF(wchSpecificStyle), wch);

    if (CARETSTYLE_OVERSTRIKE_BAR == ovr_mask) {
      StringCchCat(wchSpecificStyle, COUNTOF(wchSpecificStyle), L"; ovrbar");
    }
  }
  if (StrStr(pCurrentStandard->Styles[STY_CARET].szValue,L"noblink")) {
    SendMessage(hwnd,SCI_SETCARETPERIOD,(WPARAM)0,0);
    SendMessage(hwnd, SCI_SETADDITIONALCARETSBLINK, false, 0);
    StringCchCat(wchSpecificStyle,COUNTOF(wchSpecificStyle),L"; noblink");
  }
  else {
    const UINT uCaretBlinkTime = GetCaretBlinkTime();
    SendMessage(hwnd, SCI_SETCARETPERIOD, (WPARAM)uCaretBlinkTime, 0);
    SendMessage(hwnd, SCI_SETADDITIONALCARETSBLINK, ((uCaretBlinkTime != 0) ? true : false), 0);
  }
  // caret fore
  if (!Style_StrGetColor(pCurrentStandard->Styles[STY_CARET].szValue, FOREGROUND_LAYER, &rgb)) {
    rgb = GetSysColor(COLOR_WINDOWTEXT);
  }
  else {
    WCHAR wch[32] = { L'\0' };
    StringCchPrintf(wch,COUNTOF(wch),L"; fore:#%02X%02X%02X",
      (int)GetRValue(rgb),
      (int)GetGValue(rgb),
      (int)GetBValue(rgb));

    StringCchCat(wchSpecificStyle,COUNTOF(wchSpecificStyle),wch);
  }
  if (!VerifyContrast(rgb, (COLORREF)SendMessage(hwnd, SCI_STYLEGETBACK, 0, 0))) {
    rgb = (int)SendMessage(hwnd, SCI_STYLEGETFORE, 0, 0);
  }
  SendMessage(hwnd,SCI_SETCARETFORE,rgb,0);
  SendMessage(hwnd,SCI_SETADDITIONALCARETFORE,rgb,0);


  StrTrimW(wchSpecificStyle, L" ;");
  StringCchCopy(pCurrentStandard->Styles[STY_CARET].szValue,
                COUNTOF(pCurrentStandard->Styles[STY_CARET].szValue),wchSpecificStyle);

  if (SendMessage(hwnd,SCI_GETEDGEMODE,0,0) == EDGE_LINE) {
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_LONG_LN_MRK].szValue, FOREGROUND_LAYER, &rgb)) // edge fore
      SendMessage(hwnd,SCI_SETEDGECOLOUR,rgb,0);
    else
      SendMessage(hwnd,SCI_SETEDGECOLOUR,GetSysColor(COLOR_3DLIGHT),0);
  }
  else {
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_LONG_LN_MRK].szValue, BACKGROUND_LAYER, &rgb)) // edge back
      SendMessage(hwnd,SCI_SETEDGECOLOUR,rgb,0);
    else
      SendMessage(hwnd,SCI_SETEDGECOLOUR,GetSysColor(COLOR_3DLIGHT),0);
  }

  Style_SetExtraLineSpace(hwnd, pCurrentStandard->Styles[STY_X_LN_SPACE].szValue, 
                          COUNTOF(pCurrentStandard->Styles[STY_X_LN_SPACE].szValue));

  if (SendMessage(hwnd, SCI_GETINDENTATIONGUIDES, 0, 0) != SC_IV_NONE) {
    Style_SetIndentGuides(hwnd, true);
  }

  // here: global define current lexer (used in subsequent calls)
  s_pLexCurrent = pLexNew;

  if (s_pLexCurrent == &lexANSI) { // special ANSI-Art style

    Style_SetMargin(hwnd, s_pLexCurrent->Styles[STY_MARGIN].iStyle,
                    s_pLexCurrent->Styles[STY_MARGIN].szValue); // margin (line number, bookmarks, folding) style

    if (Settings2.UseOldStyleBraceMatching) {
      Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_OK].iStyle,
                      pCurrentStandard->Styles[STY_BRACE_OK].szValue, false);

      Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_BAD].iStyle,
                      pCurrentStandard->Styles[STY_BRACE_BAD].szValue, false);
    }

    // (SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT) at pos STY_CTRL_CHR(4) ) 
    Style_SetExtraLineSpace(hwnd, s_pLexCurrent->Styles[STY_CTRL_CHR].szValue,
                            COUNTOF(s_pLexCurrent->Styles[STY_CTRL_CHR].szValue));

  }
  else if (s_pLexCurrent->lexerID != SCLEX_NULL)
  {
    // -----------------------------------------------
    int i = 1; // don't re-apply lexer's default style
    // -----------------------------------------------
    while (s_pLexCurrent->Styles[i].iStyle != -1) 
    {
      // apply MULTI_STYLE() MACRO
      for (int j = 0; j < 4 && (s_pLexCurrent->Styles[i].iStyle8[j] != 0 || j == 0); ++j) {
        Style_SetStyles(hwnd, s_pLexCurrent->Styles[i].iStyle8[j], s_pLexCurrent->Styles[i].szValue, false);
      }

      if (s_pLexCurrent->lexerID == SCLEX_HTML && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_HPHP_DEFAULT) {
        int iRelated[] = { SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE, SCE_HPHP_WORD, SCE_HPHP_HSTRING, SCE_HPHP_SIMPLESTRING, SCE_HPHP_NUMBER,
                           SCE_HPHP_OPERATOR, SCE_HPHP_VARIABLE, SCE_HPHP_HSTRING_VARIABLE, SCE_HPHP_COMPLEX_VARIABLE };
        for (int j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],s_pLexCurrent->Styles[i].szValue, false);
      }

      if (s_pLexCurrent->lexerID == SCLEX_HTML && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_HJ_DEFAULT) {
        int iRelated[] = { SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC, SCE_HJ_KEYWORD, SCE_HJ_WORD, SCE_HJ_DOUBLESTRING,
                           SCE_HJ_SINGLESTRING, SCE_HJ_STRINGEOL, SCE_HJ_REGEX, SCE_HJ_NUMBER, SCE_HJ_SYMBOLS };
        for (int j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],s_pLexCurrent->Styles[i].szValue, false);
      }

      if (s_pLexCurrent->lexerID == SCLEX_HTML && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_HJA_DEFAULT) {
        int iRelated[] = { SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE, SCE_HJA_COMMENTDOC, SCE_HJA_KEYWORD, SCE_HJA_WORD, SCE_HJA_DOUBLESTRING,
                           SCE_HJA_SINGLESTRING, SCE_HJA_STRINGEOL, SCE_HJA_REGEX, SCE_HJA_NUMBER, SCE_HJA_SYMBOLS };
        for (int j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],s_pLexCurrent->Styles[i].szValue, false);
      }

      if (s_pLexCurrent->lexerID == SCLEX_HTML && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_HB_DEFAULT) {
        int iRelated[] = { SCE_HB_COMMENTLINE, SCE_HB_WORD, SCE_HB_IDENTIFIER, SCE_HB_STRING, SCE_HB_STRINGEOL, SCE_HB_NUMBER };
        for (int j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],s_pLexCurrent->Styles[i].szValue, false);
      }

      if (s_pLexCurrent->lexerID == SCLEX_HTML && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_HBA_DEFAULT) {
        int iRelated[] = { SCE_HBA_COMMENTLINE, SCE_HBA_WORD, SCE_HBA_IDENTIFIER, SCE_HBA_STRING, SCE_HBA_STRINGEOL, SCE_HBA_NUMBER };
        for (int j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],s_pLexCurrent->Styles[i].szValue, false);
      }

      if ((s_pLexCurrent->lexerID == SCLEX_HTML || s_pLexCurrent->lexerID == SCLEX_XML) && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_H_SGML_DEFAULT) {
        int iRelated[] = { SCE_H_SGML_COMMAND, SCE_H_SGML_1ST_PARAM, SCE_H_SGML_DOUBLESTRING, SCE_H_SGML_SIMPLESTRING, SCE_H_SGML_ERROR,
                           SCE_H_SGML_SPECIAL, SCE_H_SGML_ENTITY, SCE_H_SGML_COMMENT, SCE_H_SGML_1ST_PARAM_COMMENT, SCE_H_SGML_BLOCK_DEFAULT };
        for (int j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],s_pLexCurrent->Styles[i].szValue, false);
      }

      if ((s_pLexCurrent->lexerID == SCLEX_HTML || s_pLexCurrent->lexerID == SCLEX_XML) && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_H_CDATA) {
        int iRelated[] = { SCE_HP_START, SCE_HP_DEFAULT, SCE_HP_COMMENTLINE, SCE_HP_NUMBER, SCE_HP_STRING,
                           SCE_HP_CHARACTER, SCE_HP_WORD, SCE_HP_TRIPLE, SCE_HP_TRIPLEDOUBLE, SCE_HP_CLASSNAME,
                           SCE_HP_DEFNAME, SCE_HP_OPERATOR, SCE_HP_IDENTIFIER, SCE_HPA_START, SCE_HPA_DEFAULT,
                           SCE_HPA_COMMENTLINE, SCE_HPA_NUMBER, SCE_HPA_STRING, SCE_HPA_CHARACTER, SCE_HPA_WORD,
                           SCE_HPA_TRIPLE, SCE_HPA_TRIPLEDOUBLE, SCE_HPA_CLASSNAME, SCE_HPA_DEFNAME, SCE_HPA_OPERATOR,
                           SCE_HPA_IDENTIFIER };
        for (int j = 0; j < COUNTOF(iRelated); j++)
          Style_SetStyles(hwnd,iRelated[j],s_pLexCurrent->Styles[i].szValue, false);
      }

      if (s_pLexCurrent->lexerID == SCLEX_XML && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_H_CDATA) {
        int iRelated[] = { SCE_H_SCRIPT, SCE_H_ASP, SCE_H_ASPAT, SCE_H_QUESTION,
                           SCE_HPHP_DEFAULT, SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE, SCE_HPHP_WORD, SCE_HPHP_HSTRING,
                           SCE_HPHP_SIMPLESTRING, SCE_HPHP_NUMBER, SCE_HPHP_OPERATOR, SCE_HPHP_VARIABLE,
                           SCE_HPHP_HSTRING_VARIABLE, SCE_HPHP_COMPLEX_VARIABLE, SCE_HJ_START, SCE_HJ_DEFAULT,
                           SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC, SCE_HJ_KEYWORD, SCE_HJ_WORD,
                           SCE_HJ_DOUBLESTRING, SCE_HJ_SINGLESTRING, SCE_HJ_STRINGEOL, SCE_HJ_REGEX, SCE_HJ_NUMBER,
                           SCE_HJ_SYMBOLS, SCE_HJA_START, SCE_HJA_DEFAULT, SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE,
                           SCE_HJA_COMMENTDOC, SCE_HJA_KEYWORD, SCE_HJA_WORD, SCE_HJA_DOUBLESTRING, SCE_HJA_SINGLESTRING,
                           SCE_HJA_STRINGEOL, SCE_HJA_REGEX, SCE_HJA_NUMBER, SCE_HJA_SYMBOLS, SCE_HB_START, SCE_HB_DEFAULT,
                           SCE_HB_COMMENTLINE, SCE_HB_WORD, SCE_HB_IDENTIFIER, SCE_HB_STRING, SCE_HB_STRINGEOL,
                           SCE_HB_NUMBER, SCE_HBA_START, SCE_HBA_DEFAULT, SCE_HBA_COMMENTLINE, SCE_HBA_WORD,
                           SCE_HBA_IDENTIFIER, SCE_HBA_STRING, SCE_HBA_STRINGEOL, SCE_HBA_NUMBER, SCE_HP_START,
                           SCE_HP_DEFAULT, SCE_HP_COMMENTLINE, SCE_HP_NUMBER, SCE_HP_STRING, SCE_HP_CHARACTER, SCE_HP_WORD,
                           SCE_HP_TRIPLE, SCE_HP_TRIPLEDOUBLE, SCE_HP_CLASSNAME, SCE_HP_DEFNAME, SCE_HP_OPERATOR,
                           SCE_HP_IDENTIFIER, SCE_HPA_START, SCE_HPA_DEFAULT, SCE_HPA_COMMENTLINE, SCE_HPA_NUMBER,
                           SCE_HPA_STRING, SCE_HPA_CHARACTER, SCE_HPA_WORD, SCE_HPA_TRIPLE, SCE_HPA_TRIPLEDOUBLE,
                           SCE_HPA_CLASSNAME, SCE_HPA_DEFNAME, SCE_HPA_OPERATOR, SCE_HPA_IDENTIFIER };

        for (int j = 0; j < COUNTOF(iRelated); j++) {
          Style_SetStyles(hwnd, iRelated[j], s_pLexCurrent->Styles[i].szValue, false);
        }
      }

      if (s_pLexCurrent->lexerID == SCLEX_CPP && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_C_COMMENT) {
        int iRelated[] = { SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR };
        for (int j = 0; j < COUNTOF(iRelated); j++) {
          Style_SetStyles(hwnd, iRelated[j], s_pLexCurrent->Styles[i].szValue, false);
        }
      }

      if (s_pLexCurrent->lexerID == SCLEX_SQL && s_pLexCurrent->Styles[i].iStyle8[0] == SCE_SQL_COMMENT) {
        int iRelated[] = { SCE_SQL_COMMENTLINE, SCE_SQL_COMMENTDOC, SCE_SQL_COMMENTLINEDOC, SCE_SQL_COMMENTDOCKEYWORD, SCE_SQL_COMMENTDOCKEYWORDERROR };
        for (int j = 0; j < COUNTOF(iRelated); j++) {
          Style_SetStyles(hwnd, iRelated[j], s_pLexCurrent->Styles[i].szValue, false);
        }
      }
      ++i;
    }
  }

  Style_SetInvisible(hwnd, false); // set fixed invisible style
  Style_SetUrlHotSpot(hwnd, Settings.HyperlinkHotspot);

  // apply lexer styles
  Sci_ApplyLexerStyle(0, -1);
  EditUpdateUrlHotspots(hwnd, 0, -1, Settings.HyperlinkHotspot);
  EditFinalizeStyling(-1);
  
  UpdateAllBars(false);
}



//=============================================================================
//
//  Style_SetUrlHotSpot()
//
void Style_SetUrlHotSpot(HWND hwnd, bool bHotSpot)
{
  int const cHotSpotStyleID = Style_GetHotspotStyleID();

  if (bHotSpot)
  {
    const WCHAR* const lpszStyleDefault = GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue;
    const WCHAR* const lpszStyleHotSpot = GetCurrentStdLexer()->Styles[STY_URL_HOTSPOT].szValue;

    SendMessage(hwnd, SCI_STYLESETHOTSPOT, cHotSpotStyleID, (LPARAM)true);
    SendMessage(hwnd, SCI_SETHOTSPOTSINGLELINE, false, 0);

    // Font
    Style_SetStyles(hwnd, cHotSpotStyleID, lpszStyleHotSpot, false);

    //if (StrStrI(lpszStyleHotSpot, L"underline") != NULL)
    //  SendMessage(hwnd, SCI_SETHOTSPOTACTIVEUNDERLINE, true, 0);
    //else
    //  SendMessage(hwnd, SCI_SETHOTSPOTACTIVEUNDERLINE, false, 0);
    SendMessage(hwnd, SCI_SETHOTSPOTACTIVEUNDERLINE, true, 0);

    COLORREF rgb = 0;

    if (!Style_StrGetColor(lpszStyleHotSpot, FOREGROUND_LAYER, &rgb)) 
    {
      Style_StrGetColor(L"italic; fore:#0000FF", FOREGROUND_LAYER, &rgb);
    }
    COLORREF inactiveFG = (COLORREF)((rgb * 75 + 50) / 100);
    SendMessage(hwnd, SCI_STYLESETFORE, cHotSpotStyleID, (LPARAM)inactiveFG);
    SendMessage(hwnd, SCI_SETHOTSPOTACTIVEFORE, true, (LPARAM)rgb);


    if (!Style_StrGetColor(lpszStyleHotSpot, BACKGROUND_LAYER, &rgb))
    {
      if (!Style_StrGetColor(lpszStyleDefault, BACKGROUND_LAYER, &rgb))
      {
        rgb = GetSysColor(COLOR_WINDOW);
      }
    }
    SendMessage(hwnd, SCI_STYLESETBACK, cHotSpotStyleID, (LPARAM)rgb);
    SendMessage(hwnd, SCI_SETHOTSPOTACTIVEBACK, true, (LPARAM)rgb);
  }
  else {
    Style_SetStyles(hwnd, cHotSpotStyleID, L"", false); // uses Styles[STY_DEFAULT]
    SendMessage(hwnd, SCI_STYLESETHOTSPOT, cHotSpotStyleID, (LPARAM)false);
  }

}


//=============================================================================
//
//  Style_SetInvisible()
//
void Style_SetInvisible(HWND hwnd, bool bInvisible)
{
  //SendMessage(hwnd, SCI_FOLDDISPLAYTEXTSETSTYLE, (WPARAM)SC_FOLDDISPLAYTEXT_BOXED, 0);
  //SciCall_MarkerDefine(MARKER_NP3_OCCUR_LINE, SC_MARK_EMPTY);  // occurrences marker
  if (bInvisible) {
    SendMessage(hwnd, SCI_STYLESETVISIBLE, (WPARAM)Style_GetInvisibleStyleID(), (LPARAM)!bInvisible);
  }
}


//=============================================================================
//
//  Style_SetInvisible()
//
void Style_SetReadonly(HWND hwnd, bool bReadonly)
{
  SendMessage(hwnd, SCI_STYLESETCHANGEABLE, (WPARAM)Style_GetReadonlyStyleID(), (LPARAM)!bReadonly);
}


//=============================================================================
//
//  Style_SetLongLineColors()
//
void Style_SetLongLineColors(HWND hwnd)
{
  COLORREF rgb;

  if (SendMessage(hwnd,SCI_GETEDGEMODE,0,0) == EDGE_LINE) 
  {
    if (!Style_StrGetColor(GetCurrentStdLexer()->Styles[STY_LONG_LN_MRK].szValue, FOREGROUND_LAYER, &rgb)) { // edge fore
      rgb = GetSysColor(COLOR_3DLIGHT);
    }
  }
  else {
    if (!Style_StrGetColor(GetCurrentStdLexer()->Styles[STY_LONG_LN_MRK].szValue, BACKGROUND_LAYER, &rgb)) { // edge back
      rgb = GetSysColor(COLOR_3DSHADOW);
    }
  }
  SendMessage(hwnd, SCI_SETEDGECOLOUR, rgb, 0);
}


//=============================================================================
//
// Style_HighlightCurrentLine()
//
void Style_HighlightCurrentLine(HWND hwnd, int iHiLitCurLn)
{
  SendMessage(hwnd, SCI_SETCARETLINEFRAME, 0, 0);
  SendMessage(hwnd, SCI_SETCARETLINEVISIBLE, false, 0);
  SendMessage(hwnd, SCI_SETCARETLINEVISIBLEALWAYS, false, 0);

  if (iHiLitCurLn > 0)
  {
    bool const backgrColor = (iHiLitCurLn == 1);

    LPCWSTR szValue = GetCurrentStdLexer()->Styles[STY_CUR_LN].szValue;

    COLORREF rgb;
    if (!Style_StrGetColor(szValue, (backgrColor ? BACKGROUND_LAYER : FOREGROUND_LAYER),  &rgb)) {
      rgb = (backgrColor ? RGB(0xFF, 0xFF, 0x00) : RGB(0xC2, 0xC0, 0xC3));
    }

    int alpha = 0;
    if (!Style_StrGetAlpha(GetCurrentStdLexer()->Styles[STY_CUR_LN].szValue, &alpha, true)) {
      alpha = SC_ALPHA_NOALPHA;
    }

    if (!backgrColor) {
      int iFrameSize = 0;
      if (!Style_StrGetSizeInt(szValue, &iFrameSize)) {
        iFrameSize = 2;
      }
      iFrameSize = max_i(1, ScaleIntToCurrentDPI(iFrameSize));
      SendMessage(hwnd, SCI_SETCARETLINEFRAME, iFrameSize, 0);
    }

    SendMessage(hwnd, SCI_SETCARETLINEBACK, rgb, 0);
    SendMessage(hwnd, SCI_SETCARETLINEBACKALPHA, alpha, 0);
    SendMessage(hwnd, SCI_SETCARETLINEVISIBLEALWAYS, true, 0);
    SendMessage(hwnd, SCI_SETCARETLINEVISIBLE, true, 0);
  }
}


//=============================================================================
//
//  _GetMarkerMarginWidth()
//
static int  _GetMarkerMarginWidth()
{
  float fSize = Style_GetBaseFontSize();
  Style_StrGetSize(GetCurrentStdLexer()->Styles[STY_MARGIN].szValue, &fSize);     // relative to LineNumber
  Style_StrGetSize(GetCurrentStdLexer()->Styles[STY_BOOK_MARK].szValue, &fSize);  // settings
  float const zoomPercent = (float)SciCall_GetZoom();
  return ScaleToCurrentDPI((fSize * zoomPercent) / 100.0f);
}

//=============================================================================
//
//  Style_SetFolding()
//
void Style_SetFolding(HWND hwnd, bool bShowCodeFolding)
{
  UNUSED(hwnd);
  SciCall_SetMarginWidthN(MARGIN_SCI_FOLDING, (bShowCodeFolding ? _GetMarkerMarginWidth() : 0));
}

//=============================================================================
//
//  Style_SetBookmark()
//
void Style_SetBookmark(HWND hwnd, bool bShowSelMargin)
{
  UNUSED(hwnd);
  SciCall_SetMarginWidthN(MARGIN_SCI_BOOKMRK, (bShowSelMargin ? _GetMarkerMarginWidth() + 4 : 0));
}


//=============================================================================
//
//  Style_SetMargin()
//
void Style_SetMargin(HWND hwnd, int iStyle, LPCWSTR lpszStyle)
{
  if (iStyle == STYLE_LINENUMBER) {
    Style_SetStyles(hwnd, iStyle, lpszStyle, false);   // line numbers
  }

  COLORREF clrFore = SciCall_StyleGetFore(STYLE_LINENUMBER);
  Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &clrFore);

  COLORREF clrBack = SciCall_StyleGetBack(STYLE_LINENUMBER);
  Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &clrBack);
  
  //SciCall_SetMarginBackN(MARGIN_SCI_LINENUM, clrBack);

    // CallTips
  SciCall_CallTipSetFore(clrFore);
  SciCall_CallTipSetBack(clrBack);

  // ---  Bookmarks  ---
  COLORREF bmkFore = clrFore;
  COLORREF bmkBack = clrBack;

  const WCHAR* wchBookMarkStyleStrg = GetCurrentStdLexer()->Styles[STY_BOOK_MARK].szValue;

  Style_StrGetColor(wchBookMarkStyleStrg, FOREGROUND_LAYER, &bmkFore);
  Style_StrGetColor(wchBookMarkStyleStrg, BACKGROUND_LAYER, &bmkBack);

  // adjust background color by alpha in case of show margin
  int alpha = 20;
  Style_StrGetAlpha(wchBookMarkStyleStrg, &alpha, true);

  COLORREF bckgrnd = clrBack;
  Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &bckgrnd);
  bmkBack = Style_RgbAlpha(bmkBack, bckgrnd, min_i(0xFF, alpha));

  //SciCall_MarkerDefine(MARKER_NP3_BOOKMARK, SC_MARK_BOOKMARK);
  //SciCall_MarkerDefine(MARKER_NP3_BOOKMARK, SC_MARK_SHORTARROW);
  SciCall_MarkerDefine(MARKER_NP3_BOOKMARK, SC_MARK_VERTICALBOOKMARK);
  SciCall_MarkerSetFore(MARKER_NP3_BOOKMARK, bmkFore);
  SciCall_MarkerSetBack(MARKER_NP3_BOOKMARK, bmkBack);
  SciCall_MarkerSetAlpha(MARKER_NP3_BOOKMARK, alpha);
  SciCall_SetMarginBackN(MARGIN_SCI_BOOKMRK, clrBack);

  // ---  Code folding  ---

  COLORREF fldHiLight = clrFore;
  const WCHAR* wchHighlightStyleStrg = GetCurrentStdLexer()->Styles[STY_SEL_TXT].szValue;
  Style_StrGetColor(wchHighlightStyleStrg, FOREGROUND_LAYER, &fldHiLight);

  SciCall_SetMarginTypeN(MARGIN_SCI_FOLDING, SC_MARGIN_COLOUR);
  SciCall_SetMarginMaskN(MARGIN_SCI_FOLDING, SC_MASK_FOLDERS);
  SciCall_SetMarginSensitiveN(MARGIN_SCI_FOLDING, true);
  SciCall_SetMarginBackN(MARGIN_SCI_FOLDING, clrBack);

  int fldStyleMrk = SC_CASE_LOWER;
  Style_StrGetCase(wchBookMarkStyleStrg, &fldStyleMrk);
  if (fldStyleMrk == SC_CASE_UPPER) // circle style
  {
    SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPEN, SC_MARK_CIRCLEMINUS);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDEREND, SC_MARK_CIRCLEPLUSCONNECTED);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
  }
  else // box style
  {
    SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
    SciCall_MarkerDefine(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
  }

  static const int iMarkerIDs[7] =
  {
    SC_MARKNUM_FOLDEROPEN,
    SC_MARKNUM_FOLDER,
    SC_MARKNUM_FOLDERSUB,
    SC_MARKNUM_FOLDERTAIL,
    SC_MARKNUM_FOLDEREND,
    SC_MARKNUM_FOLDEROPENMID,
    SC_MARKNUM_FOLDERMIDTAIL
  };

  for (int i = 0; i < COUNTOF(iMarkerIDs); ++i) {
    SciCall_MarkerSetFore(iMarkerIDs[i], bmkBack); // (!)
    SciCall_MarkerSetBack(iMarkerIDs[i], bmkFore); // (!)
    SciCall_MarkerSetBackSelected(iMarkerIDs[i], fldHiLight);
  }
  SciCall_MarkerEnableHighlight(true);

  SciCall_SetFoldMarginColour(true, clrBack);    // background
  SciCall_SetFoldMarginHiColour(true, clrBack);  // (!)
  //SciCall_FoldDisplayTextSetStyle(SC_FOLDDISPLAYTEXT_HIDDEN);

  int fldStyleLn = 0;
  Style_StrGetCharSet(wchBookMarkStyleStrg, &fldStyleLn);
  switch (fldStyleLn)
  {
  case 1:
    SciCall_SetFoldFlags(SC_FOLDFLAG_LINEBEFORE_CONTRACTED);
    break;
  case 2:
    SciCall_SetFoldFlags(SC_FOLDFLAG_LINEBEFORE_CONTRACTED | SC_FOLDFLAG_LINEAFTER_CONTRACTED);
    break;
  default:
    SciCall_SetFoldFlags(SC_FOLDFLAG_LINEAFTER_CONTRACTED);
    break;
  }

  // set width
  Style_SetBookmark(hwnd, Settings.ShowSelectionMargin);
  Style_SetFolding(hwnd, (Globals.bCodeFoldingAvailable && Settings.ShowCodeFolding));
}


//=============================================================================
//
//  Style_SniffShebang()
//
PEDITLEXER  Style_SniffShebang(char* pchText)
{
  if (StrCmpNA(pchText,"#!",2) == 0) {
    char *pch = pchText + 2;
    while (*pch == ' ' || *pch == '\t')
      pch++;
    while (*pch && *pch != ' ' && *pch != '\t' && *pch != '\r' && *pch != '\n')
      pch++;
    if ((pch - pchText) >= 3 && StrCmpNA(pch-3,"env",3) == 0) {
      while (*pch == ' ')
        pch++;
      while (*pch && *pch != ' ' && *pch != '\t' && *pch != '\r' && *pch != '\n')
        pch++;
    }
    if ((pch - pchText) >= 3 && StrCmpNIA(pch - 3, "php", 3) == 0) {
      return(&lexHTML);
    }
    if ((pch - pchText) >= 4 && StrCmpNIA(pch - 4, "perl", 4) == 0) {
      return(&lexPL);
    }
    if ((pch - pchText) >= 6 && StrCmpNIA(pch - 6, "python", 6) == 0) {
      return(&lexPY);
    }
    if ((pch - pchText) >= 3 && StrCmpNA(pch - 3, "tcl", 3) == 0) {
      return(&lexTCL);
    }
    if ((pch - pchText) >= 4 && StrCmpNA(pch - 4, "wish", 4) == 0) {
      return(&lexTCL);
    }
    if ((pch - pchText) >= 5 && StrCmpNA(pch - 5, "tclsh", 5) == 0) {
      return(&lexTCL);
    }
    if ((pch - pchText) >= 2 && StrCmpNA(pch - 2, "sh", 2) == 0) {
      return(&lexBASH);
    }
    if ((pch - pchText) >= 4 && StrCmpNA(pch - 4, "ruby", 4) == 0) {
      return(&lexRUBY);
    }
    if ((pch - pchText) >= 4 && StrCmpNA(pch - 4, "node", 4) == 0) {
      return(&lexJS);
    }
  }
  return(NULL);
}


//=============================================================================
//
//  Style_MatchLexer()
//
PEDITLEXER Style_MatchLexer(LPCWSTR lpszMatch, bool bCheckNames) 
{
  int const cch = (int)StringCchLenW(lpszMatch, 0);

  if (!bCheckNames) 
  {
    for (int iLex = 0; iLex < COUNTOF(g_pLexArray); ++iLex) {
      LPCWSTR p1 = g_pLexArray[iLex]->szExtensions;
      do {
        LPCWSTR p2 = StrStrI(p1, lpszMatch);
        if (p2 == NULL) {
          break;
        }
        WCHAR const ch = (p2 == p1) ? L'\0' : p2[-1];
        p2 += cch;
        if ((ch == L';' || ch == ' ' || ch == L'\0') && (*p2 == L';' || *p2 == L' ' || *p2 == L'\0')) {
          return g_pLexArray[iLex];
        }
        p1 = StrChr(p2, L';');
      } while (p1 != NULL);
    }
  }
  else {
    if (cch >= 3) {
      for (int iLex = 0; iLex < COUNTOF(g_pLexArray); ++iLex) {
        if (StrCmpNI(g_pLexArray[iLex]->pszName, lpszMatch, cch) == 0)
          return(g_pLexArray[iLex]);
      }
    }
  }
  return NULL;
}


//=============================================================================
//
//  Style_HasLexerForExt()
//
bool Style_HasLexerForExt(LPCWSTR lpszExt)
{
  if (lpszExt && (*lpszExt == L'.')) ++lpszExt;
  return (lpszExt && Style_MatchLexer(lpszExt,false)) ? true : false;
}


//=============================================================================
//
//  Style_SetLexerFromFile()
//
void Style_SetLexerFromFile(HWND hwnd,LPCWSTR lpszFile)
{
  LPWSTR lpszExt = PathFindExtension(lpszFile);
  bool  bFound = false;
  PEDITLEXER pLexNew = NULL;
  PEDITLEXER pLexSniffed = NULL;

  if ((Globals.fvCurFile.mask & FV_MODE) && Globals.fvCurFile.tchMode[0]) {

    PEDITLEXER pLexMode;
    WCHAR wchMode[MICRO_BUFFER] = { L'\0' };
    MultiByteToWideChar(Encoding_SciCP, 0, Globals.fvCurFile.tchMode, -1, wchMode, MICRO_BUFFER);

    if (!Flags.NoCGIGuess && (StringCchCompareNI(wchMode,COUNTOF(wchMode),L"cgi", CSTRLEN(L"cgi")) == 0 ||
                         StringCchCompareNI(wchMode,COUNTOF(wchMode),L"fcgi", CSTRLEN(L"fcgi")) == 0)) {
      char tchText[256] = { L'\0' };
      SciCall_GetText(COUNTOF(tchText) - 1, tchText);
      StrTrimA(tchText," \t\n\r");
      pLexSniffed = Style_SniffShebang(tchText);
      if (pLexSniffed) {
        if ((Encoding_Current(CPI_GET) != Globals.DOSEncoding) || !IsLexerStandard(pLexSniffed) || (
          StringCchCompareXI(lpszExt,L"nfo") && StringCchCompareXI(lpszExt,L"diz"))) {
          // Although .nfo and .diz were removed from the default lexer's
          // default extensions list, they may still presist in the user's INI
          pLexNew = pLexSniffed;
          bFound = true;
        }
      }
    }

    if (!bFound) {
      pLexMode = Style_MatchLexer(wchMode, false);
      if (pLexMode) {
        pLexNew = pLexMode;
        bFound = true;
      }
      else {
        pLexMode = Style_MatchLexer(wchMode, true);
        if (pLexMode) {
          pLexNew = pLexMode;
          bFound = true;
        }
      }
    }
  }

  if (!bFound && s_bAutoSelect && /* s_bAutoSelect == false skips lexer search */
      (StrIsNotEmpty(lpszFile) && *lpszExt)) {

    if (*lpszExt == L'.') ++lpszExt;

    if (!Flags.NoCGIGuess && (StringCchCompareXI(lpszExt,L"cgi") == 0 || StringCchCompareXI(lpszExt,L"fcgi") == 0)) {
      char tchText[256] = { '\0' };
      SciCall_GetText(COUNTOF(tchText) - 1, tchText);
      StrTrimA(tchText," \t\n\r");
      pLexSniffed = Style_SniffShebang(tchText);
      if (pLexSniffed) {
        pLexNew = pLexSniffed;
        bFound = true;
      }
    }

    if (!bFound && StringCchCompareXI(PathFindFileName(lpszFile),L"cmakelists.txt") == 0) {
      pLexNew = &lexCmake;
      bFound = true;
    }

    // check associated extensions
    if (!bFound) {
      pLexSniffed = Style_MatchLexer(lpszExt, false);
      if (pLexSniffed) {
        pLexNew = pLexSniffed;
        bFound = true;
      }
    }
  }

  if (!bFound && s_bAutoSelect && lpszFile &&
      StringCchCompareXI(PathFindFileName(lpszFile),L"makefile") == 0) {
    pLexNew = &lexMAK;
    bFound = true;
  }

  if (!bFound && s_bAutoSelect && lpszFile &&
      StringCchCompareXI(PathFindFileName(lpszFile),L"rakefile") == 0) {
    pLexNew = &lexRUBY;
    bFound = true;
  }

  if (!bFound && s_bAutoSelect && lpszFile &&
      StringCchCompareXI(PathFindFileName(lpszFile),L"mozconfig") == 0) {
    pLexNew = &lexBASH;
    bFound = true;
  }

  if (!bFound && s_bAutoSelect && (!Flags.NoHTMLGuess || !Flags.NoCGIGuess)) {
    char tchText[512];
    SciCall_GetText(COUNTOF(tchText) - 1, tchText);
    StrTrimA(tchText," \t\n\r");
    if (!Flags.NoCGIGuess) {
      if (tchText[0] == '<') {
        if (StrStrIA(tchText, "<html"))
          pLexNew = &lexHTML;
        else
          pLexNew = &lexXML;
        bFound = true;
      }
      else {
        pLexSniffed = Style_SniffShebang(tchText);
        if (pLexSniffed) {
          pLexNew = pLexSniffed;
          bFound = true;
        }
      }
    }
  }

  if (!bFound && (Encoding_Current(CPI_GET) == Globals.DOSEncoding)) {
    pLexNew = &lexANSI;
  }
  // Apply the new lexer
  if (IsLexerStandard(pLexNew)) {
    Style_SetDefaultLexer(hwnd);
  }
  else {
    Style_SetLexer(hwnd, pLexNew);
  }
}


//=============================================================================
//
//  Style_SetLexerFromName()
//
void Style_SetLexerFromName(HWND hwnd,LPCWSTR lpszFile,LPCWSTR lpszName)
{
  PEDITLEXER pLexNew = Style_MatchLexer(lpszName, false);
  if (pLexNew) {
    Style_SetLexer(hwnd, pLexNew);
  }
  else {
    pLexNew = Style_MatchLexer(lpszName, true);
    if (pLexNew) {
      Style_SetLexer(hwnd, pLexNew);
    }
    else {
      Style_SetLexerFromFile(hwnd, lpszFile);
    }
  }
}


//=============================================================================
//
//  Style_ResetCurrentLexer()
//
void Style_ResetCurrentLexer(HWND hwnd)
{
  Style_SetLexer(hwnd, s_pLexCurrent);
}


//=============================================================================
//
//  Style_SetDefaultLexer()
//
void Style_SetDefaultLexer(HWND hwnd)
{
  Style_SetLexer(hwnd, NULL);
}


//=============================================================================
//
//  Style_SetHTMLLexer()
//
void Style_SetHTMLLexer(HWND hwnd)
{
  Style_SetLexer(hwnd,&lexHTML);
}


//=============================================================================
//
//  Style_SetXMLLexer()
//
void Style_SetXMLLexer(HWND hwnd)
{
  Style_SetLexer(hwnd,&lexXML);
}


//=============================================================================
//
//  Style_SetLexerFromID()
//
void Style_SetLexerFromID(HWND hwnd,int id)
{
  if (id >= 0 && id < COUNTOF(g_pLexArray)) {
    Style_SetLexer(hwnd,g_pLexArray[id]);
  }
}


//=============================================================================
//
//  Style_ToggleUse2ndDefault()
//
void Style_ToggleUse2ndDefault(HWND hwnd)
{
  bool const use2ndDefStyle = Style_GetUse2ndDefault();
  Style_SetUse2ndDefault(use2ndDefStyle ? false : true); // swap
  if (IsLexerStandard(s_pLexCurrent)) {
    s_pLexCurrent = Style_GetUse2ndDefault() ? &lexStandard2nd : &lexStandard; // sync
  }
  Style_ResetCurrentLexer(hwnd);
}


//=============================================================================
//
//  Style_SetDefaultFont()
//
void Style_SetDefaultFont(HWND hwnd, bool bGlobalDefault)
{
  WCHAR newStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR lexerName[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR styleName[BUFSIZE_STYLE_VALUE] = { L'\0' };

  PEDITLEXER const pLexer = bGlobalDefault ? GetCurrentStdLexer() : s_pLexCurrent;
  PEDITSTYLE const pLexerDefStyle = &(pLexer->Styles[STY_DEFAULT]);

  StringCchCopyW(newStyle, COUNTOF(newStyle), pLexer->Styles[STY_DEFAULT].szValue);

  GetLngString(pLexer->resID, lexerName, COUNTOF(lexerName));
  GetLngString(pLexer->Styles[STY_DEFAULT].rid, styleName, COUNTOF(styleName));

  if (Style_SelectFont(hwnd, newStyle, COUNTOF(newStyle), lexerName, styleName,
                       IsStyleStandardDefault(pLexerDefStyle), IsStyleSchemeDefault(pLexerDefStyle), false, true))
  {
    // set new styles to current lexer's default text
    StringCchCopyW(pLexerDefStyle->szValue, COUNTOF(pLexerDefStyle->szValue), newStyle);
    Style_ResetCurrentLexer(hwnd);
  }
}


//=============================================================================
//
//  Style_SetUse2ndDefault(), Style_GetUse2ndDefault()
//
static bool s_bUse2ndDefaultStyle = false;

void Style_SetUse2ndDefault(bool use2nd)
{
  s_bUse2ndDefaultStyle = use2nd;
}

bool Style_GetUse2ndDefault()
{
  return s_bUse2ndDefaultStyle;
}


//=============================================================================
//
//  Style_SetIndentGuides()
//

void Style_SetIndentGuides(HWND hwnd,bool bShow)
{
  UNUSED(hwnd);
  int iIndentView = SC_IV_NONE;
  if (bShow) {
    if (!Flags.SimpleIndentGuides) {
      switch (SciCall_GetLexer()) {
      case SCLEX_PYTHON:
      case SCLEX_NIMROD:
        iIndentView = SC_IV_LOOKFORWARD;
        break;
      default:
        iIndentView = SC_IV_LOOKBOTH;
        break;
      }
    }
    else
      iIndentView = SC_IV_REAL;
  }
  SciCall_SetIndentationGuides(iIndentView);
}


//=============================================================================
//
//  Style_SetExtraLineSpace()
//
void Style_SetExtraLineSpace(HWND hwnd, LPWSTR lpszStyle, int cch)
{
  float fValue = 0.0f;
  bool const  bHasLnSpaceDef = Style_StrGetSize(lpszStyle, &fValue);

  int iAscent = 0;
  int iDescent = 0;

  if (bHasLnSpaceDef) {
    int const iValue = float2int(fValue);
    const int iCurFontSizeDbl = float2int(Style_GetCurrentFontSize() * 2.0f);
    int iValAdj = clampi(iValue, (0 - iCurFontSizeDbl), 256 * iCurFontSizeDbl);
    if ((iValAdj != iValue) && (cch > 0)) {
      StringCchPrintf(lpszStyle, cch, L"size:%i", iValAdj);
    }
    if ((iValAdj % 2) != 0) {
      iAscent++;
      iValAdj--;
    }
    iAscent += (iValAdj >> 1);
    iDescent += (iValAdj >> 1);
  }
  SendMessage(hwnd, SCI_SETEXTRAASCENT, (WPARAM)iAscent, 0);
  SendMessage(hwnd, SCI_SETEXTRADESCENT, (WPARAM)iDescent, 0);
}


//=============================================================================
//
//  Style_GetFileOpenDlgFilter()
//

bool Style_GetOpenDlgFilterStr(LPWSTR lpszFilter,int cchFilter)
{
  if (StringCchLenW(Settings2.FileDlgFilters, COUNTOF(Settings2.FileDlgFilters)) == 0) {
    GetLngString(IDS_MUI_FILTER_ALL, lpszFilter, cchFilter);
  }
  else {
    StringCchCopyN(lpszFilter,cchFilter,Settings2.FileDlgFilters,cchFilter - 2);
    StringCchCat(lpszFilter,cchFilter,L"||");
  }
  PrepareFilterStr(lpszFilter);
  return true;
}


//=============================================================================
//
//  Style_StrGetFont()
//
bool Style_StrGetFont(LPCWSTR lpszStyle, LPWSTR lpszFont, int cchFont)
{
  WCHAR *p = StrStrI(lpszStyle, L"font:");
  if (p) {
    p += CSTRLEN(L"font:");
    while (*p == L' ') { ++p; }
    StringCchCopyN(lpszFont, cchFont, p, cchFont);
    if ((p = StrChr(lpszFont, L';')) != NULL) {
      *p = L'\0';
    }
    TrimStringW(lpszFont);
    if (StringCchCompareNI(lpszFont, cchFont, L"Default", CSTRLEN(L"Default")) == 0) {
      if (IsFontAvailable(L"Consolas")) {
        StringCchCopyN(lpszFont, cchFont, L"Consolas", cchFont);
      }
      else {
        StringCchCopyN(lpszFont, cchFont, L"Lucida Console", cchFont);
      }
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetFontQuality()
//
bool Style_StrGetFontQuality(LPCWSTR lpszStyle,LPWSTR lpszQuality,int cchQuality)
{
  WCHAR *p = StrStrI(lpszStyle, L"smoothing:");
  if (p)
  {
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch,COUNTOF(tch),p + CSTRLEN(L"smoothing:"));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimStringW(tch);
    if (StringCchCompareXI(tch,L"none") == 0 ||
        StringCchCompareXI(tch,L"standard") == 0 ||
        StringCchCompareXI(tch,L"cleartype") == 0 ||
        StringCchCompareXI(tch,L"default") == 0)
    {
      StringCchCopyN(lpszQuality,cchQuality,tch,COUNTOF(tch));
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetCharSet()
//
bool Style_StrGetCharSet(LPCWSTR lpszStyle, int* i)
{
  WCHAR *p = StrStrI(lpszStyle, L"charset:");
  if (p)
  {
    p += CSTRLEN(L"charset:");
    int iValue = 0;
    if (Char2IntW(p, &iValue)) {
      *i = max_i(SC_CHARSET_ANSI, iValue);
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetIntSizeInt()
//
bool Style_StrGetSizeInt(LPCWSTR lpszStyle, int* i)
{
  WCHAR *p = StrStrI(lpszStyle, L"size:");
  if (p)
  {
    p += CSTRLEN(L"size:");
    return Char2IntW(p, i);
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetSize()
//
bool Style_StrGetSize(LPCWSTR lpszStyle, float* f)
{
  WCHAR *p = StrStrI(lpszStyle, L"size:");
  if (p)
  {
    int fSign = 0;
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch,COUNTOF(tch),p + CSTRLEN(L"size:"));
    if (tch[0] == L'+')
    {
      fSign = 1;
      tch[0] = L' ';
    }
    else if (tch[0] == L'-')
    {
      fSign = -1;
      tch[0] = L' ';
    }
    p = StrChr(tch, L';');
    if (p) { *p = L'\0'; }
    TrimStringW(tch);
    
    float fValue = 0.0;
    if (Char2FloatW(tch, &fValue))
    {
      if (fSign != 0) {
        // relative size calculation
        float const base = *f; // base is input
        fValue = (base + (fSign * fValue)); // can be negative
      }
      *f = Round10th(fValue);
      return true;
    }
  }
  return false;
}



//=============================================================================
//
//  Style_StrGetSizeStr()
//
bool Style_StrGetSizeStr(LPCWSTR lpszStyle,LPWSTR lpszSize,int cchSize)
{
  WCHAR *p = StrStrI(lpszStyle, L"size:");
  if (p)
  {
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch, COUNTOF(tch), (p + CSTRLEN(L"size:")));
    p = StrChr(tch, L';');
    if (p) { *p = L'\0'; }
    TrimStringW(tch);

    float fValue = 0.0f;
    if (Char2FloatW(tch, &fValue)) {
      WCHAR wchFloatVal[64];
      fValue = (float)fabs(fValue);
      Float2String(fValue, wchFloatVal, COUNTOF(wchFloatVal));

      if (tch[0] == L'+')
        StringCchPrintf(lpszSize, cchSize, L"+%s", wchFloatVal);
      else if (tch[0] == L'-')
        StringCchPrintf(lpszSize, cchSize, L"-%s", wchFloatVal);
      else
        StringCchPrintf(lpszSize, cchSize, L"%s", wchFloatVal);

      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetWeightValue()
//
bool Style_StrGetWeightValue(LPCWSTR lpszWeight, int* i)
{
  int iFontWeight = -1;
  
  if (StrStrI(lpszWeight, L"thin"))
    iFontWeight = FW_THIN;
  else if (StrStrI(lpszWeight, L"extralight"))
    iFontWeight = FW_EXTRALIGHT;
  else if (StrStrI(lpszWeight, L"light"))
    iFontWeight = FW_LIGHT;
  else if (StrStrI(lpszWeight, L"normal"))
    iFontWeight = FW_NORMAL;
  else if (StrStrI(lpszWeight, L"medium"))
    iFontWeight = FW_MEDIUM;
  else if (StrStrI(lpszWeight, L"semibold"))
    iFontWeight = FW_SEMIBOLD;
  else if (StrStrI(lpszWeight, L"extrabold"))
    iFontWeight = FW_EXTRABOLD;
  else if (StrStrI(lpszWeight, L"bold"))     // here, cause bold is in semibold and extrabold too
    iFontWeight = FW_BOLD;
  else if (StrStrI(lpszWeight, L"heavy"))
    iFontWeight = FW_HEAVY;

  if (iFontWeight >= 0) {
    *i = iFontWeight;
  }
  return ((iFontWeight < 0) ? false : true);
}

//=============================================================================
//
//  Style_AppendWeightStr()
//
void Style_AppendWeightStr(LPWSTR lpszWeight, int cchSize, int fontWeight)
{
  if (fontWeight <= FW_THIN) {
    StringCchCat(lpszWeight, cchSize, L"; thin");
  }
  else if (fontWeight <= FW_EXTRALIGHT) {
    StringCchCat(lpszWeight, cchSize, L"; extralight");
  }
  else if (fontWeight <= FW_LIGHT) {
    StringCchCat(lpszWeight, cchSize, L"; light");
  }
  else if (fontWeight <= FW_NORMAL) {
    StringCchCat(lpszWeight, cchSize, L"; normal");
  }
  else if (fontWeight <= FW_MEDIUM) {
    StringCchCat(lpszWeight, cchSize, L"; medium");
  }
  else if (fontWeight <= FW_SEMIBOLD) {
    StringCchCat(lpszWeight, cchSize, L"; semibold");
  }
  else if (fontWeight <= FW_BOLD) {
    StringCchCat(lpszWeight, cchSize, L"; bold");
  }
  else if (fontWeight <= FW_EXTRABOLD) {
    StringCchCat(lpszWeight, cchSize, L"; extrabold");
  }
  else { // (fontWeight >= FW_HEAVY)
    StringCchCat(lpszWeight, cchSize, L"; heavy");
  }
}


//=============================================================================
//
//  Style_StrGetColor()
//
bool Style_StrGetColor(LPCWSTR lpszStyle, COLOR_LAYER layer, COLORREF* rgb)
{
  WCHAR *pItem = (layer == FOREGROUND_LAYER) ? L"fore:" : L"back:";

  WCHAR *p = StrStrI(lpszStyle, pItem);
  if (p)
  {
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch, COUNTOF(tch), p + StringCchLenW(pItem,0));
    if (tch[0] == L'#')
      tch[0] = L' ';
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimStringW(tch);
    unsigned int iValue = 0;
    int itok = swscanf_s(tch, L"%x", &iValue);
    if (itok == 1)
    {
      *rgb = RGB((iValue & 0xFF0000) >> 16, (iValue & 0xFF00) >> 8, iValue & 0xFF);
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetAlpha()
//
bool Style_StrGetAlpha(LPCWSTR lpszStyle, int* i, bool bAlpha1st) 
{
  const WCHAR* strAlpha = bAlpha1st ? L"alpha:" : L"alpha2:";

  WCHAR* p = StrStrI(lpszStyle, strAlpha);
  if (p) {
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch, COUNTOF(tch), p + StringCchLenW(strAlpha,0));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimStringW(tch);
    int iValue = 0;
    int itok = swscanf_s(tch, L"%i", &iValue);
    if (itok == 1) {
      *i = clampi(iValue, SC_ALPHA_TRANSPARENT, SC_ALPHA_OPAQUE);
      return true;
    }
  }
  return false;
}


////=============================================================================
////
////  Style_StrGetPropertyValue()
////
//bool Style_StrGetPropertyValue(LPCWSTR lpszStyle, LPCWSTR lpszProperty, int* val)
//{
//  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
//  WCHAR *p = StrStrI(lpszStyle, lpszProperty);
//  if (p) {
//    StringCchCopy(tch, COUNTOF(tch), (p + StringCchLenW(lpszProperty,0)));
//    p = StrChr(tch, L';');
//    if (p)
//      *p = L'\0';
//    TrimStringW(tch);
//    if (1 == swscanf_s(tch, L"%i", val)) { return true; }
//  }
//  return false;
//}


//=============================================================================
//
//  Style_StrGetCase()
//
bool Style_StrGetCase(LPCWSTR lpszStyle, int* i)
{
  WCHAR *p = StrStrI(lpszStyle, L"case:");
  if (p) {
    p += CSTRLEN(L"case:");
    p += StrSpn(p, L" ");
    switch (*p) {
    case L'u':
    case L'U':
      *i = SC_CASE_UPPER;
      return true;
    case L'l':
    case L'L':
      *i = SC_CASE_LOWER;
      return true;
    default:
      break;
    }
  }
  return false;
}


//=============================================================================
//
//  Style_GetIndicatorType()
//

static WCHAR* IndicatorTypes[22] = {
  L"indic_plain",
  L"indic_squiggle",
  L"indic_tt",
  L"indic_diagonal",
  L"indic_strike",
  L"indic_hidden",
  L"indic_box",
  L"indic_roundbox",
  L"indic_straightbox",
  L"indic_dash",
  L"indic_dots",
  L"indic_squigglelow",
  L"indic_dotbox",
  L"indic_squigglepixmap",
  L"indic_compositionthick",
  L"indic_compositionthin",
  L"indic_fullbox",
  L"indic_textfore",
  L"indic_point",
  L"indic_pointcharacter",
  L"indic_gradient",
  L"indic_gradientcentre"
};


bool Style_GetIndicatorType(LPWSTR lpszStyle, int cchSize, int* idx)
{
  if (*idx < 0) { // retrieve indicator style from string
    for (int i = COUNTOF(IndicatorTypes) - 1;  0 <= i;  --i) {
      if (StrStrI(lpszStyle, IndicatorTypes[i])) {
        *idx = i;
        return true;
      }
    }
    *idx = INDIC_ROUNDBOX; // default
  }
  else {  // get indicator string from index

    if (*idx < COUNTOF(IndicatorTypes)) 
    {
      StringCchCopy(lpszStyle, cchSize, IndicatorTypes[*idx]);
      return true;
    }
    StringCchCopy(lpszStyle, cchSize, IndicatorTypes[INDIC_ROUNDBOX]); // default
  }
  return false;
}


//=============================================================================
//
//  Style_CopyStyles_IfNotDefined()
//
void Style_CopyStyles_IfNotDefined(LPCWSTR lpszStyleSrc, LPWSTR lpszStyleDest, int cchSizeDest, bool bCopyFont, bool bWithEffects)
{
  int  iValue;
  COLORREF dColor;
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR wchDefFontName[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR szTmpStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

  // ---------   Font settings   ---------
  if (bCopyFont)
  {
    if (!StrStrI(lpszStyleDest, L"font:")) {
      if (Style_StrGetFont(lpszStyleSrc, tch, COUNTOF(tch))) {
        Style_StrGetFont(L"font:Default", wchDefFontName, COUNTOF(wchDefFontName));
        if (StringCchCompareXI(tch, wchDefFontName) == 0) {
          StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; font:Default");
        }
        else {
          StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; font:");
          StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
        }
      }
    }

    // ---------  Size  ---------
    if (!StrStrI(lpszStyleDest, L"size:")) {
      if (Style_StrGetSizeStr(lpszStyleSrc, tch, COUNTOF(tch))) {
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; size:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
      }
    }

    if (StrStrI(lpszStyleSrc, L"thin") && !StrStrI(lpszStyleDest, L"thin"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; thin");
    else if (StrStrI(lpszStyleSrc, L"extralight") && !StrStrI(lpszStyleDest, L"extralight"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; extralight");
    else if (StrStrI(lpszStyleSrc, L"light") && !StrStrI(lpszStyleDest, L"light"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; light");
    else if (StrStrI(lpszStyleSrc, L"normal") && !StrStrI(lpszStyleDest, L"normal"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; normal");
    else if (StrStrI(lpszStyleSrc, L"medium") && !StrStrI(lpszStyleDest, L"medium"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; medium");
    else if (StrStrI(lpszStyleSrc, L"semibold") && !StrStrI(lpszStyleDest, L"semibold"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; semibold");
    else if (StrStrI(lpszStyleSrc, L"extrabold") && !StrStrI(lpszStyleDest, L"extrabold"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; extrabold");
    else if (StrStrI(lpszStyleSrc, L"bold") && !StrStrI(lpszStyleDest, L"bold"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; bold");
    else if (StrStrI(lpszStyleSrc, L"heavy") && !StrStrI(lpszStyleDest, L"heavy"))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; heavy");
    //else
    //  StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; normal");

    if (StrStrI(lpszStyleSrc, L"italic") && !StrStrI(lpszStyleDest, L"italic")) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; italic");
    }
  }

  // ---------  Effects  ---------
  if (bWithEffects)
  {
    if (StrStrI(lpszStyleSrc, L"strikeout") && !StrStrI(lpszStyleDest, L"strikeout")) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; strikeout");
    }

    if (StrStrI(lpszStyleSrc, L"underline") && !StrStrI(lpszStyleDest, L"underline")) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; underline");
    }

    if (StrStrI(lpszStyleSrc, L"fore:") && !StrStrI(lpszStyleDest, L"fore:")) { // foreground
      if (Style_StrGetColor(lpszStyleSrc, FOREGROUND_LAYER, &dColor)) {
        StringCchPrintf(tch, COUNTOF(tch), L"; fore:#%02X%02X%02X",
          (int)GetRValue(dColor), (int)GetGValue(dColor), (int)GetBValue(dColor));
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
      }
    }

    if (StrStrI(lpszStyleSrc, L"back:") && !StrStrI(lpszStyleDest, L"back:")) { // background
      if (Style_StrGetColor(lpszStyleSrc, BACKGROUND_LAYER, &dColor)) {
        StringCchPrintf(tch, COUNTOF(tch), L"; back:#%02X%02X%02X",
          (int)GetRValue(dColor), (int)GetGValue(dColor), (int)GetBValue(dColor));
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
      }
    }
  }

  // ---------  Special Styles  ---------

  if (!StrStrI(lpszStyleDest, L"charset:")) {
    if (Style_StrGetCharSet(lpszStyleSrc, &iValue)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; charset:%i", iValue);
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }

  if (!StrStrI(lpszStyleDest, L"smoothing:")) {
    if (Style_StrGetFontQuality(lpszStyleSrc, tch, COUNTOF(tch))) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; smoothing:");
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }

  if (StrStrI(lpszStyleSrc, L"eolfilled") && !StrStrI(lpszStyleDest, L"eolfilled")) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; eolfilled");
  }

  if (Style_StrGetCase(lpszStyleSrc, &iValue) && !StrStrI(lpszStyleDest, L"case:")) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; case:");
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), (iValue == SC_CASE_UPPER) ? L"U" : L"L");
  }

  if (!StrStrI(lpszStyleDest, L"alpha:")) {
    if (Style_StrGetAlpha(lpszStyleSrc, &iValue, true)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; alpha:%i", iValue);
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }
  if (!StrStrI(lpszStyleDest, L"alpha2:")) {
    if (Style_StrGetAlpha(lpszStyleSrc, &iValue, false)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; alpha2:%i", iValue);
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }

  //const WCHAR* wchProperty = L"property:";
  //if (!StrStrI(lpszStyleDest, wchProperty)) {
  //  if (Style_StrGetPropertyValue(lpszStyleSrc, wchProperty, &iValue)) {
  //    StringCchPrintf(tch, COUNTOF(tch), L"; %s%i", wchProperty, iValue);
  //    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
  //  }
  //}

  // --------   indicator type   --------
  if (!StrStrI(lpszStyleDest, L"indic_")) {
    iValue = -1;
    StringCchCopy(tch, COUNTOF(tch), lpszStyleSrc);
    if (Style_GetIndicatorType(tch, 0, &iValue)) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; ");
      Style_GetIndicatorType(tch, COUNTOF(tch), &iValue);
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }

  // --------   other style settings   --------
  if (StrStrI(lpszStyleSrc, L"ovrbar") && !StrStrI(lpszStyleDest, L"ovrbar")) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; ovrbar");
  }

  if (StrStrI(lpszStyleSrc, L"block") && !StrStrI(lpszStyleDest, L"block")) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; block");
  }

  if (StrStrI(lpszStyleSrc, L"noblink") && !StrStrI(lpszStyleDest, L"noblink")) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; noblink");
  }

  StrTrim(szTmpStyle, L" ;");
  StringCchCat(lpszStyleDest, cchSizeDest, szTmpStyle);
}

//=============================================================================

/// Callback to set the font dialog's title
static  WCHAR FontSelTitle[128];

static UINT CALLBACK Style_FontDialogHook(
  HWND hdlg,      // handle to the dialog box window
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
)
{
  if (uiMsg == WM_INITDIALOG) {
    SetWindowText(hdlg, (WCHAR*)((CHOOSEFONT*)lParam)->lCustData);
  }
  UNUSED(wParam);
  return 0;	// Allow the default handler a chance to process
}



//=============================================================================
//
//  Style_SelectFont()
//
bool Style_SelectFont(HWND hwnd,LPWSTR lpszStyle,int cchStyle, LPCWSTR sLexerName, LPCWSTR sStyleName,
                      bool bGlobalDefaultStyle, bool bCurrentDefaultStyle, 
                      bool bWithEffects, bool bPreserveStyles)
{
  // Map lpszStyle to LOGFONT

  WCHAR wchFontName[64] = { L'\0' };
  if (!Style_StrGetFont(lpszStyle, wchFontName, COUNTOF(wchFontName))) 
  {
    if (!Style_StrGetFont(GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue, wchFontName, COUNTOF(wchFontName)))
    {
      Style_StrGetFont(L"font:Default", wchFontName, COUNTOF(wchFontName));
    }
  }

  int iCharSet = Globals.iDefaultCharSet;
  if (!Style_StrGetCharSet(lpszStyle, &iCharSet)) {
    iCharSet = Globals.iDefaultCharSet;
  }
    
  // is "size:" definition relative ?
  bool const bRelFontSize = (!StrStrI(lpszStyle, L"size:") || StrStrI(lpszStyle, L"size:+") || StrStrI(lpszStyle, L"size:-"));

  float const fBFS = GetBaseFontSize(Globals.hwndMain);
  float const fBaseFontSize = (bGlobalDefaultStyle ? fBFS : (bCurrentDefaultStyle ? Style_GetBaseFontSize() : Style_GetCurrentFontSize()));

  // Font Height

  int iFontHeight = 0;
  int iPointSize = 0;
  float fFontSize = fBaseFontSize;
  if (Style_StrGetSize(lpszStyle, &fFontSize)) {
    iPointSize = float2int(fFontSize * 10.0f);
    HDC hdc = GetDC(hwnd);
    iFontHeight = -MulDiv(float2int(fFontSize * SC_FONT_SIZE_MULTIPLIER), GetDeviceCaps(hdc, LOGPIXELSY), 72 * SC_FONT_SIZE_MULTIPLIER);
    ReleaseDC(hwnd,hdc);
  }
  else {
    iPointSize = float2int(fBaseFontSize * 10.0f);
    HDC hdc = GetDC(hwnd);
    iFontHeight = -MulDiv(float2int(fBaseFontSize * SC_FONT_SIZE_MULTIPLIER), GetDeviceCaps(hdc, LOGPIXELSY), 72 * SC_FONT_SIZE_MULTIPLIER);
    ReleaseDC(hwnd, hdc);
  }

  // Font Weight
  int  iFontWeight = FW_NORMAL;
  if (!Style_StrGetWeightValue(lpszStyle, &iFontWeight)) {
    iFontWeight = FW_NORMAL;
  }
  bool bIsItalic = (StrStrI(lpszStyle, L"italic")) ? true : false;
  bool bIsUnderline = (StrStrI(lpszStyle, L"underline")) ? true : false;
  bool bIsStrikeout = (StrStrI(lpszStyle, L"strikeout")) ? true : false;

  int iQuality = g_FontQuality[Settings2.SciFontQuality];
  switch (iQuality) {
  case SC_EFF_QUALITY_NON_ANTIALIASED:
    iQuality = NONANTIALIASED_QUALITY;
    break;
  case SC_EFF_QUALITY_ANTIALIASED:
    iQuality = ANTIALIASED_QUALITY;
    break;
  case SC_EFF_QUALITY_LCD_OPTIMIZED:
    iQuality = CLEARTYPE_QUALITY;
    break;
  default:
  case SC_EFF_QUALITY_DEFAULT:
    iQuality = DEFAULT_QUALITY;
    break;
  }

  // --------------------------------------------------------------------------

  LOGFONT lf;
  ZeroMemory(&lf, sizeof(LOGFONT));
  StringCchCopyN(lf.lfFaceName, COUNTOF(lf.lfFaceName), wchFontName, COUNTOF(wchFontName));
  lf.lfCharSet = (BYTE)iCharSet;
  lf.lfHeight = iFontHeight;
  lf.lfWeight = iFontWeight;
  lf.lfItalic = (BYTE)bIsItalic;
  lf.lfUnderline = (BYTE)bIsUnderline;
  lf.lfStrikeOut = (BYTE)bIsStrikeout;
  lf.lfQuality = (BYTE)iQuality;
  lf.lfClipPrecision = (BYTE)CLIP_DEFAULT_PRECIS;
  lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;

  COLORREF color = 0L;
  Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &color);

  // Init cf
  CHOOSEFONT cf;
  ZeroMemory(&cf, sizeof(CHOOSEFONT));
  //cf.nSizeMin = 4;
  //cf.nSizeMax = 128;
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hwndOwner = hwnd;
  cf.hInstance = Globals.hInstance; // ChooseFontDirectWrite
  cf.rgbColors = color;
  cf.lpLogFont = &lf;
  cf.iPointSize = iPointSize;
  cf.nFontType = ((iFontWeight <= FW_MEDIUM) ? REGULAR_FONTTYPE : BOLD_FONTTYPE);
  cf.nFontType |= (bIsItalic ? ITALIC_FONTTYPE : 0);
  cf.lpfnHook = (LPCFHOOKPROC)Style_FontDialogHook;	// Register the callback
  cf.lCustData = (LPARAM)FontSelTitle;
  //cf.Flags = CF_INITTOLOGFONTSTRUCT /*| CF_EFFECTS | CF_NOSCRIPTSEL*/ | CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_ENABLEHOOK;
  //cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_USESTYLE | CF_SCALABLEONLY | CF_FORCEFONTEXIST | CF_ENABLEHOOK;
  cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCALABLEONLY | CF_FORCEFONTEXIST | CF_ENABLEHOOK;

  if (bGlobalDefaultStyle) {
    if (bRelFontSize)
      FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), IDS_MUI_TITLE_RELBASE, sStyleName);
    else
      FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), IDS_MUI_TITLE_FIXBASE, sStyleName);
  }
  else if (bCurrentDefaultStyle) {
    if (bRelFontSize)
      FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), IDS_MUI_TITLE_RELCUR, sLexerName, sStyleName);
    else
      FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), IDS_MUI_TITLE_FIXCUR, sLexerName, sStyleName);
  }
  else {
    if (bRelFontSize)
      FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), IDS_MUI_TITLE_RELARB, sStyleName, sLexerName);
    else
      FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), IDS_MUI_TITLE_FIXARB, sStyleName, sLexerName);
  }

  if (bWithEffects)
    cf.Flags |= CF_EFFECTS;

  if (IsKeyDown(VK_SHIFT)) {
    cf.Flags |= CF_FIXEDPITCHONLY;
  }

  WCHAR szStyleStrg[80] = { L'\0' };
  if (cf.Flags & CF_USESTYLE) {
    cf.lpszStyle = szStyleStrg;
  }


  // ---  open systems Font Selection dialog  ---
  if (Settings.RenderingTechnology > 0) {
    if (!ChooseFontDirectWrite(Globals.hwndMain, Settings2.PreferredLanguageLocaleName, Globals.CurrentDPI, &cf) ||
        (lf.lfFaceName[0] == L'\0')) { 
      return false; 
    }
  }
  else {
    if (!ChooseFont(&cf) || (lf.lfFaceName[0] == L'\0')) { return false; }
  }

  // ---  map back to lpszStyle  ---

  WCHAR szNewStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

  if (StrStrI(lpszStyle, L"font:")) {
    StringCchCopy(szNewStyle, COUNTOF(szNewStyle), L"font:");
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), lf.lfFaceName);
  }
  else { // no font in source specified, 
    if (lstrcmpW(lf.lfFaceName, wchFontName) != 0) {
      StringCchCopy(szNewStyle, COUNTOF(szNewStyle), L"font:");
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), lf.lfFaceName);
    }
  }

  if (lf.lfWeight == iFontWeight) {
    WCHAR check[64] = { L'\0' };
    Style_AppendWeightStr(check, COUNTOF(check), lf.lfWeight);
    StrTrimW(check, L" ;");
    if (StrStrI(lpszStyle, check)) {
      Style_AppendWeightStr(szNewStyle, COUNTOF(szNewStyle), lf.lfWeight);
    }
  }
  else {
    Style_AppendWeightStr(szNewStyle, COUNTOF(szNewStyle), lf.lfWeight);
  }


  float fNewFontSize = (float)(cf.iPointSize) / 10.0f;
  WCHAR newSize[64] = { L'\0' };

  if (bRelFontSize)
  {
    float fNewRelSize = Round10th(fNewFontSize - fBaseFontSize);

    if (fNewRelSize >= 0.0) {
      if (HasNonZeroFraction(fNewRelSize))
        StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:+%.3G", fNewRelSize);
      else
        StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:+%i", float2int(fNewRelSize));
    }
    else {
      if (HasNonZeroFraction(fNewRelSize))
        StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:-%.3G", (0.0f - fNewRelSize));
      else 
        StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:-%i", float2int(0.0f - fNewRelSize));
    }
  }
  else {
    fFontSize = Round10th(fFontSize);
    fNewFontSize = Round10th(fNewFontSize);

    if (fNewFontSize == fFontSize) {
      if (StrStrI(lpszStyle, L"size:")) {
        if (HasNonZeroFraction(fNewFontSize))
          StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:%.3G", fNewFontSize);
        else
          StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:%i", float2int(fNewFontSize));
      }
    }
    else {
      if (HasNonZeroFraction(fNewFontSize))
        StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:%.3G", fNewFontSize);
      else
        StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:%i", float2int(fNewFontSize));
    }
  }
  StringCchCat(szNewStyle, COUNTOF(szNewStyle), newSize);

  
  if (bGlobalDefaultStyle &&
    (lf.lfCharSet != DEFAULT_CHARSET) &&
    (lf.lfCharSet != ANSI_CHARSET) &&
    (lf.lfCharSet != Globals.iDefaultCharSet)) {
    WCHAR chset[32] = { L'\0' };
    if (lf.lfCharSet == iCharSet) {
      if (StrStrI(lpszStyle, L"charset:"))
      {
        StringCchPrintf(chset, COUNTOF(chset), L"; charset:%i", lf.lfCharSet);
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), chset);
      }
    }
    else {
      StringCchPrintf(chset, COUNTOF(chset), L"; charset:%i", lf.lfCharSet);
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), chset);
    }
  }

  if (lf.lfItalic) {
    if (bIsItalic) {
      if (StrStrI(lpszStyle, L"italic")) {
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; italic");
      }
    }
    else {
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; italic");
    }
  }


  if (bWithEffects) {

    if (lf.lfUnderline) {
      if (bIsUnderline) {
        if (StrStrI(lpszStyle, L"underline")) {
          StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; underline");
        }
      }
      else {
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; underline");
      }
    }

    if (lf.lfStrikeOut) {
      if (bIsStrikeout) {
        if (StrStrI(lpszStyle, L"strikeout")) {
          StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; strikeout");
        }
      }
      else {
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; strikeout");
      }
    }

    // ---  save colors  ---
    WCHAR newColor[64] = { L'\0' };
    if (cf.rgbColors == color) {
      if (StrStrI(lpszStyle, L"fore:")) {
        StringCchPrintf(newColor, COUNTOF(newColor), L"; fore:#%02X%02X%02X",
          (int)GetRValue(cf.rgbColors),
                        (int)GetGValue(cf.rgbColors),
                        (int)GetBValue(cf.rgbColors));
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), newColor);
      }
    }
    else { // color changed
      StringCchPrintf(newColor, COUNTOF(newColor), L"; fore:#%02X%02X%02X",
        (int)GetRValue(cf.rgbColors),
                      (int)GetGValue(cf.rgbColors),
                      (int)GetBValue(cf.rgbColors));
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), newColor);
    }
    // copy background
    if (Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &color)) {
      StringCchPrintf(newColor, COUNTOF(newColor), L"; back:#%02X%02X%02X",
                      (int)GetRValue(color),
                      (int)GetGValue(color),
                      (int)GetBValue(color));
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), newColor);
    }
  }

  if (bPreserveStyles) {
    // copy all other styles
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; ");
    Style_CopyStyles_IfNotDefined(lpszStyle, szNewStyle, COUNTOF(szNewStyle), false, !bWithEffects);
  }

  StrTrim(szNewStyle, L" ;");
  StringCchCopyN(lpszStyle, cchStyle, szNewStyle, COUNTOF(szNewStyle));
  return true;
}


//=============================================================================
//
//  Style_SelectColor()
//
bool Style_SelectColor(HWND hwnd,bool bForeGround,LPWSTR lpszStyle,int cchStyle, bool bPreserveStyles)
{
  CHOOSECOLOR cc;
  WCHAR szNewStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };
  COLORREF dRGBResult;
  COLORREF dColor;
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };

  ZeroMemory(&cc,sizeof(CHOOSECOLOR));

  dRGBResult = (bForeGround) ? GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_WINDOW);
  Style_StrGetColor(lpszStyle, (bForeGround ? FOREGROUND_LAYER : BACKGROUND_LAYER), &dRGBResult);

  cc.lStructSize = sizeof(CHOOSECOLOR);
  cc.hwndOwner = hwnd;
  cc.rgbResult = dRGBResult;
  cc.lpCustColors = s_colorCustom;
  cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;

  if (!ChooseColor(&cc))
    return false;

  dRGBResult = cc.rgbResult;


  // Rebuild style string
  StringCchCopy(szNewStyle, COUNTOF(szNewStyle), L"");  // clear

  if (bForeGround)
  {
    StringCchPrintf(tch,COUNTOF(tch),L"; fore:#%02X%02X%02X",
      (int)GetRValue(dRGBResult),
      (int)GetGValue(dRGBResult),
      (int)GetBValue(dRGBResult));
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);

    if (Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &dColor))
    {
      StringCchPrintf(tch,COUNTOF(tch),L"; back:#%02X%02X%02X",
        (int)GetRValue(dColor),
        (int)GetGValue(dColor),
        (int)GetBValue(dColor));
      StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
    }
  }
  else // set background
  {
    if (Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &dColor))
    {
      StringCchPrintf(tch,COUNTOF(tch),L"; fore:#%02X%02X%02X; ",
        (int)GetRValue(dColor),
        (int)GetGValue(dColor),
        (int)GetBValue(dColor));
      StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
    }
    StringCchPrintf(tch,COUNTOF(tch),L"; back:#%02X%02X%02X",
      (int)GetRValue(dRGBResult),
      (int)GetGValue(dRGBResult),
      (int)GetBValue(dRGBResult));
    StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
  }

  if (bPreserveStyles) {
    // copy all other styles
    StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; ");
    Style_CopyStyles_IfNotDefined(lpszStyle, szNewStyle, COUNTOF(szNewStyle), true, false);
  }

  StrTrim(szNewStyle, L" ;");
  StringCchCopyN(lpszStyle,cchStyle,szNewStyle,cchStyle);
  return true;
}



//=============================================================================
//
//  Style_SetStyles()
//
void Style_SetStyles(HWND hwnd, int iStyle, LPCWSTR lpszStyle, bool bInitDefault)
{
  if (!bInitDefault && StrIsEmpty(lpszStyle)) { return; }

  int iValue = 0;
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };

  // reset horizontal scrollbar width
  SendMessage(hwnd, SCI_SETSCROLLWIDTH, 1, 0);

  // Font
  WCHAR wchFontName[80] = { L'\0' };
  char chFontName[80] = { '\0' };
  if (Style_StrGetFont(lpszStyle, wchFontName, COUNTOF(wchFontName))) {
    if (StringCchLenW(wchFontName, COUNTOF(wchFontName)) > 0) {
      WideCharToMultiByte(Encoding_SciCP, 0, wchFontName, -1, chFontName, COUNTOF(chFontName), NULL, NULL);
      SendMessage(hwnd, SCI_STYLESETFONT, iStyle, (LPARAM)chFontName);
    }
  }
  else if (bInitDefault) {
    Style_StrGetFont(L"font:Default", wchFontName, COUNTOF(wchFontName));
    WideCharToMultiByte(Encoding_SciCP, 0, wchFontName, -1, chFontName, COUNTOF(chFontName), NULL, NULL);
    SendMessage(hwnd, SCI_STYLESETFONT, iStyle, (LPARAM)chFontName);
  }
  
  // Font Quality
  if (Style_StrGetFontQuality(lpszStyle, tch, COUNTOF(tch)))
  {
    WPARAM wQuality = SC_EFF_QUALITY_DEFAULT;

    if (StringCchCompareNI(tch, COUNTOF(tch), L"none", COUNTOF(L"none")) == 0)
      wQuality = SC_EFF_QUALITY_NON_ANTIALIASED;
    else if (StringCchCompareNI(tch, COUNTOF(tch), L"standardtype", COUNTOF(L"standardtype")) == 0)
      wQuality = SC_EFF_QUALITY_ANTIALIASED;
    else if (StringCchCompareNI(tch, COUNTOF(tch), L"cleartype", COUNTOF(L"cleartype")) == 0)
      wQuality = SC_EFF_QUALITY_LCD_OPTIMIZED;

    SendMessage(hwnd, SCI_SETFONTQUALITY, wQuality, 0);
  }
  else if (bInitDefault) {
    WPARAM wQuality = (WPARAM)g_FontQuality[Settings2.SciFontQuality];
    if (wQuality == SC_EFF_QUALITY_DEFAULT) {
      // undefined, use general settings, except for special fonts
      if (StringCchCompareXI(wchFontName, L"Calibri") == 0 ||
          StringCchCompareXI(wchFontName, L"Cambria") == 0 ||
          StringCchCompareXI(wchFontName, L"Candara") == 0 ||
          StringCchCompareXI(wchFontName, L"Consolas") == 0 ||
          StringCchCompareXI(wchFontName, L"Constantia") == 0 ||
          StringCchCompareXI(wchFontName, L"Corbel") == 0 ||
          StringCchCompareXI(wchFontName, L"DejaVu Sans Mono") == 0 ||
          StringCchCompareXI(wchFontName, L"Segoe UI") == 0 ||
          StringCchCompareXI(wchFontName, L"Source Code Pro") == 0) 
      {
        wQuality = SC_EFF_QUALITY_LCD_OPTIMIZED;
      }
    }
    SendMessage(hwnd, SCI_SETFONTQUALITY, wQuality, 0);
  }

  // Size values are relative to BaseFontSize/CurrentFontSize
  float fBaseFontSize = Style_GetCurrentFontSize();

  if (Style_StrGetSize(lpszStyle, &fBaseFontSize)) {
    SendMessage(hwnd, SCI_STYLESETSIZEFRACTIONAL, iStyle, (LPARAM)ScaleFractionalFontSize(fBaseFontSize));
    if (iStyle == STYLE_DEFAULT) {
      if (bInitDefault) {
        _SetBaseFontSize(fBaseFontSize);
      }
      _SetCurrentFontSize(fBaseFontSize);
    }
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETSIZEFRACTIONAL, STYLE_DEFAULT, (LPARAM)ScaleFractionalFontSize(fBaseFontSize));
    _SetBaseFontSize(fBaseFontSize);
  }

  // Character Set
  if (Style_StrGetCharSet(lpszStyle, &iValue)) {
    SendMessage(hwnd, SCI_STYLESETCHARACTERSET, iStyle, (LPARAM)iValue);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETCHARACTERSET, iStyle, (LPARAM)SC_CHARSET_DEFAULT);
  }

  COLORREF dColor = 0L;
  // Fore
  if (Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &dColor)) {
    SendMessage(hwnd, SCI_STYLESETFORE, iStyle, (LPARAM)dColor);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETFORE, iStyle, (LPARAM)GetSysColor(COLOR_WINDOWTEXT));   // default text color
  }

  // Back
  if (Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &dColor)) {
    SendMessage(hwnd, SCI_STYLESETBACK, iStyle, (LPARAM)dColor);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETBACK, iStyle, (LPARAM)GetSysColor(COLOR_WINDOW));       // default window color
  }

  // Weight
  if (Style_StrGetWeightValue(lpszStyle, &iValue)) {
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)iValue);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETWEIGHT, iStyle, (LPARAM)SC_WEIGHT_NORMAL);
  }

  // Italic
  if (StrStrI(lpszStyle, L"italic")) {
    SendMessage(hwnd, SCI_STYLESETITALIC, iStyle, (LPARAM)true);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETITALIC, iStyle, (LPARAM)false);
  }

  // Underline
  if (StrStrI(lpszStyle, L"underline")) {
    SendMessage(hwnd, SCI_STYLESETUNDERLINE, iStyle, (LPARAM)true);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETUNDERLINE, iStyle, (LPARAM)false);
  }

  // StrikeOut does not exist in scintilla ???  / Hide instead (no good idea)
  //if (StrStrI(lpszStyle, L"strikeout")) {
  //  SendMessage(hwnd, SCI_STYLESETVISIBLE,iStyle,(LPARAM)false);
  //}
  //else if (bInitDefault) {
  //  SendMessage(hwnd, SCI_STYLESETVISIBLE,iStyle,(LPARAM)true);
  //}

  // EOL Filled
  if (StrStrI(lpszStyle, L"eolfilled")) {
    SendMessage(hwnd, SCI_STYLESETEOLFILLED, iStyle, (LPARAM)true);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETEOLFILLED, iStyle, (LPARAM)false);
  }

  // Case
  if (Style_StrGetCase(lpszStyle, &iValue)) {
    SendMessage(hwnd, SCI_STYLESETCASE, iStyle, (LPARAM)iValue);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETCASE, iStyle, (LPARAM)SC_CASE_MIXED);
  }
}


//=============================================================================
//
//  Style_GetCurrentLexerPtr()
//
PEDITLEXER Style_GetCurrentLexerPtr()
{
  return s_pLexCurrent;
}


//=============================================================================
//
//  Style_GetCurrentLexerRID()
//
int Style_GetCurrentLexerRID()
{
  return s_pLexCurrent->resID;
}


//=============================================================================
//
//  Style_GetLexerDisplayName()
//
void Style_GetLexerDisplayName(PEDITLEXER pLexer, LPWSTR lpszName, int cchName)
{
  if (!pLexer) {
    //pLexer = Style_GetUse2ndDefault() ? &lexStandard2nd : &lexStandard;
    pLexer = &lexStandard; // don't distinguish between STD/2ND
  }
  if (!GetLngString(pLexer->resID, lpszName, cchName)) {
    StringCchCopyW(lpszName, cchName, pLexer->pszName);
  }
}


//=============================================================================
//
//  Style_GetStyleDisplayName()
//
void Style_GetStyleDisplayName(PEDITSTYLE pStyle, LPWSTR lpszName, int cchName)
{
  if (pStyle) {
    if (!GetLngString(pStyle->rid, lpszName, cchName)) {
      StringCchCopyW(lpszName, cchName, pStyle->pszName);
    }
  }
}


//=============================================================================
//
//  Style_GetLexerIconId()
//
int Style_GetLexerIconId(PEDITLEXER plex)
{
  WCHAR pszFile[MAX_PATH + BUFZIZE_STYLE_EXTENTIONS];

  LPCWSTR pszExtensions;
  if (StrIsNotEmpty(plex->szExtensions)) {
    pszExtensions = plex->szExtensions;
  }
  else {
    pszExtensions = plex->pszDefExt;
  }
  StringCchCopy(pszFile,COUNTOF(pszFile),L"*.");
  StringCchCat(pszFile,COUNTOF(pszFile),pszExtensions);

  WCHAR *p = StrChr(pszFile, L';');
  if (p) { *p = L'\0'; }

  // check for ; at beginning
  if (StringCchLen(pszFile, COUNTOF(pszFile)) < 3) {
    StringCchCat(pszFile, COUNTOF(pszFile), L"txt");
  }
  SHFILEINFO shfi;
  ZeroMemory(&shfi,sizeof(SHFILEINFO));

  SHGetFileInfo(pszFile,FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(SHFILEINFO),
    SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);

  return (shfi.iIcon);
}


//=============================================================================
//
//  Style_AddLexerToTreeView()
//
HTREEITEM Style_AddLexerToTreeView(HWND hwnd,PEDITLEXER plex)
{
  WCHAR tch[MIDSZ_BUFFER] = { L'\0' };

  HTREEITEM hTreeNode;

  TVINSERTSTRUCT tvis;
  ZeroMemory(&tvis,sizeof(TVINSERTSTRUCT));

  tvis.hInsertAfter = TVI_LAST;

  tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

  if (GetLngString(plex->resID,tch,COUNTOF(tch)))
    tvis.item.pszText = tch;
  else
    tvis.item.pszText = (LPWSTR)plex->pszName;

  tvis.item.iImage = Style_GetLexerIconId(plex);
  tvis.item.iSelectedImage = tvis.item.iImage;
  tvis.item.lParam = (LPARAM)plex;

  hTreeNode = TreeView_InsertItem(hwnd,&tvis);

  tvis.hParent = hTreeNode;

  tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  //tvis.item.iImage = -1;
  //tvis.item.iSelectedImage = -1;

  int i = 1; // default style is handled separately
  while (plex->Styles[i].iStyle != -1) {

    if (GetLngString(plex->Styles[i].rid,tch,COUNTOF(tch)))
      tvis.item.pszText = tch;
    else
      tvis.item.pszText = (LPWSTR)plex->Styles[i].pszName;
    tvis.item.lParam = (LPARAM)(&plex->Styles[i]);
    TreeView_InsertItem(hwnd,&tvis);
    i++;
  }

  return hTreeNode;
}


//=============================================================================
//
//  Style_AddLexerToListView()
//
void Style_AddLexerToListView(HWND hwnd,PEDITLEXER plex)
{
  WCHAR tch[MIDSZ_BUFFER] = { L'\0' };
  LVITEM lvi;
  ZeroMemory(&lvi,sizeof(LVITEM));

  lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
  lvi.iItem = ListView_GetItemCount(hwnd);
  if (GetLngString(plex->resID,tch,COUNTOF(tch)))
    lvi.pszText = tch;
  else
    lvi.pszText = (LPWSTR)plex->pszName;
  lvi.iImage = Style_GetLexerIconId(plex);
  lvi.lParam = (LPARAM)plex;

  ListView_InsertItem(hwnd,&lvi);
}



//=============================================================================
//
//  Style_CustomizeSchemesDlgProc()
//

static bool  _ApplyDialogItemText(HWND hwnd, 
  PEDITLEXER pCurrentLexer, PEDITSTYLE pCurrentStyle, int iCurStyleIdx, bool bIsStyleSelected)
{
  UNUSED(iCurStyleIdx);

  bool bChgNfy = false;

  WCHAR szBuf[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = { L'\0' };
  GetDlgItemText(hwnd, IDC_STYLEEDIT, szBuf, COUNTOF(szBuf));
  // normalize
  WCHAR szBufNorm[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = { L'\0' };
  Style_CopyStyles_IfNotDefined(szBuf, szBufNorm, COUNTOF(szBufNorm), true, true);

  if (StringCchCompareXI(szBufNorm, pCurrentStyle->szValue) != 0) {
    StringCchCopy(pCurrentStyle->szValue, COUNTOF(pCurrentStyle->szValue), szBufNorm);
    bChgNfy = true;
  }
  if (!bIsStyleSelected)  // must be file extensions
  {
    if (!GetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, szBuf, COUNTOF(szBuf))) 
    {
      StringCchCopy(szBuf, COUNTOF(szBuf), pCurrentLexer->pszDefExt);
    }
    if (StringCchCompareXI(szBuf, pCurrentLexer->szExtensions) != 0) 
    {
      StringCchCopy(pCurrentLexer->szExtensions, COUNTOF(pCurrentLexer->szExtensions), szBuf);
      bChgNfy = true;
    }
  }
  if (bChgNfy && (IsLexerStandard(pCurrentLexer) || (pCurrentLexer == s_pLexCurrent))) 
  {
    Style_ResetCurrentLexer(Globals.hwndEdit);
  }
  return bChgNfy;
}


INT_PTR CALLBACK Style_CustomizeSchemesDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static HWND hwndTV;
  static bool fDragging;
  static PEDITLEXER pCurrentLexer = NULL;
  static PEDITSTYLE pCurrentStyle = NULL;
  static int iCurStyleIdx = -1;
  static HBRUSH hbrFore;
  static HBRUSH hbrBack;
  static bool bIsStyleSelected = false;
  static bool bWarnedNoIniFile = false;
  static WCHAR* Style_StylesBackup[NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER];

  static WCHAR tchTmpBuffer[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = { L'\0' };

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }

        ResizeDlg_Init(hwnd, Settings.CustomSchemesDlgSizeX, Settings.CustomSchemesDlgSizeY, IDC_RESIZEGRIP);

        GetLngString(IDS_MUI_STYLEEDIT_HELP, tchTmpBuffer, COUNTOF(tchTmpBuffer));
        SetDlgItemText(hwnd, IDC_STYLEEDIT_HELP, tchTmpBuffer);

        // Backup Styles
        ZeroMemory(&Style_StylesBackup, NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER * sizeof(WCHAR*));
        int cnt = 0;
        for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {
          Style_StylesBackup[cnt++] = StrDup(g_pLexArray[iLexer]->szExtensions);
          int i = 0;
          while (g_pLexArray[iLexer]->Styles[i].iStyle != -1) {
            Style_StylesBackup[cnt++] = StrDup(g_pLexArray[iLexer]->Styles[i].szValue);
            ++i;
          }
        }

        hwndTV = GetDlgItem(hwnd,IDC_STYLELIST);
        fDragging = false;

        SHFILEINFO shfi;
        ZeroMemory(&shfi, sizeof(SHFILEINFO));
        TreeView_SetImageList(hwndTV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\",FILE_ATTRIBUTE_DIRECTORY,&shfi,sizeof(SHFILEINFO),
            SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),TVSIL_NORMAL);

        // findlexer
        int found = -1;
        for (int i = 0; i < COUNTOF(g_pLexArray); ++i) {
          if (g_pLexArray[i] == s_pLexCurrent) {
            found = i;
            break;
          }
        }

        // Build lexer tree view
        HTREEITEM hCurrentTVLex = NULL;
        for (int i = 0; i < COUNTOF(g_pLexArray); i++)
        {
          if (i == found)
            hCurrentTVLex = Style_AddLexerToTreeView(hwndTV,g_pLexArray[i]);
          else
            Style_AddLexerToTreeView(hwndTV,g_pLexArray[i]);
        }
        if (!hCurrentTVLex) 
        {
          hCurrentTVLex = TreeView_GetRoot(hwndTV);
          if (Style_GetUse2ndDefault()) 
            hCurrentTVLex = TreeView_GetNextSibling(hwndTV, hCurrentTVLex);
        }
        TreeView_Select(hwndTV, hCurrentTVLex, TVGN_CARET);

        pCurrentLexer = (found >= 0) ? s_pLexCurrent : GetDefaultLexer();
        pCurrentStyle = &(pCurrentLexer->Styles[STY_DEFAULT]);
        iCurStyleIdx = STY_DEFAULT;

        SendDlgItemMessage(hwnd,IDC_STYLEEDIT,EM_LIMITTEXT, max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)-1,0);

        MakeBitmapButton(hwnd,IDC_PREVSTYLE,Globals.hInstance,IDB_PREV);
        MakeBitmapButton(hwnd,IDC_NEXTSTYLE,Globals.hInstance,IDB_NEXT);

        // Setup title font
        static HFONT hFontTitle = NULL;
        if (hFontTitle) {
          DeleteObject(hFontTitle);
        }
        if (NULL == (hFontTitle = (HFONT)SendDlgItemMessage(hwnd,IDC_TITLE,WM_GETFONT,0,0)))
          hFontTitle = GetStockObject(DEFAULT_GUI_FONT);

        LOGFONT lf;
        GetObject(hFontTitle,sizeof(LOGFONT),&lf);
        lf.lfHeight += lf.lfHeight / 5;
        lf.lfWeight = FW_BOLD;
        hFontTitle = CreateFontIndirect(&lf);
        SendDlgItemMessage(hwnd,IDC_TITLE,WM_SETFONT,(WPARAM)hFontTitle,true);

        if (Settings.CustomSchemesDlgPosX == CW_USEDEFAULT || Settings.CustomSchemesDlgPosY == CW_USEDEFAULT)
        {
          CenterDlgInParent(hwnd);
        }
        else {
          SetDlgPos(hwnd, Settings.CustomSchemesDlgPosX, Settings.CustomSchemesDlgPosY);
        }
        HMENU hmenu = GetSystemMenu(hwnd, false);
        GetLngString(IDS_MUI_PREVIEW, tchTmpBuffer, COUNTOF(tchTmpBuffer));
        InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_PREVIEW, tchTmpBuffer);
        InsertMenu(hmenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        GetLngString(IDS_MUI_SAVEPOS, tchTmpBuffer, COUNTOF(tchTmpBuffer));
        InsertMenu(hmenu, 2, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_SAVEPOS, tchTmpBuffer);
        GetLngString(IDS_MUI_RESETPOS, tchTmpBuffer, COUNTOF(tchTmpBuffer));
        InsertMenu(hmenu, 3, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_RESETPOS, tchTmpBuffer);
        InsertMenu(hmenu, 4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        bWarnedNoIniFile = false;
      }
      return true;

    case WM_ACTIVATE:
      DialogEnableWindow(hwnd, IDC_PREVIEW, ((pCurrentLexer == s_pLexCurrent) || (pCurrentLexer == GetCurrentStdLexer())));
      return true;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;

    case WM_DESTROY:
      {
        DeleteBitmapButton(hwnd, IDC_STYLEFORE);
        DeleteBitmapButton(hwnd, IDC_STYLEBACK);
        DeleteBitmapButton(hwnd, IDC_PREVSTYLE);
        DeleteBitmapButton(hwnd, IDC_NEXTSTYLE);
        ResizeDlg_Destroy(hwnd, &Settings.CustomSchemesDlgSizeX, &Settings.CustomSchemesDlgSizeY);

        // free old backup
        int cnt = 0;
        for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {
          if (Style_StylesBackup[cnt]) {
            LocalFree(Style_StylesBackup[cnt]);  // StrDup()
            Style_StylesBackup[cnt] = NULL;
          }
          ++cnt;
          int i = 0;
          while (g_pLexArray[iLexer]->Styles[i].iStyle != -1) {
            if (Style_StylesBackup[cnt]) {
              LocalFree(Style_StylesBackup[cnt]);  // StrDup()
              Style_StylesBackup[cnt] = NULL;
            }
            ++cnt;
            ++i;
          }
        }
        pCurrentLexer = NULL;
        pCurrentStyle = NULL;
        iCurStyleIdx = -1;
      }
      return false;


    case WM_SYSCOMMAND:
      if (wParam == IDS_MUI_SAVEPOS) {
        PostMessage(hwnd, WM_COMMAND, MAKELONG(IDACC_SAVEPOS, 0), 0);
        return true;
      }
      else if (wParam == IDS_MUI_RESETPOS) {
        PostMessage(hwnd, WM_COMMAND, MAKELONG(IDACC_RESETPOS, 0), 0);
        return true;
      }
      else
        return false;

    case WM_SIZE: 
      {
        int dx;
        int dy;
        ResizeDlg_Size(hwnd, lParam, &dx, &dy);
        HDWP hdwp = BeginDeferWindowPos(18);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_RESIZEGRIP, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDOK, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDCANCEL, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLELIST, 0, dy, SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_INFO_GROUPBOX, dx, 0, SWP_NOMOVE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLELABEL_ROOT, 0, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLEEDIT_ROOT, 0, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLELABEL, 0, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLEEDIT, 0, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLEFORE, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLEBACK, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLEFONT, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_PREVIEW, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_STYLEDEFAULT, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_PREVSTYLE, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_NEXTSTYLE, dx, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_IMPORT, 0, dy, SWP_NOSIZE);
        hdwp = DeferCtlPos(hdwp, hwnd, IDC_EXPORT, 0, dy, SWP_NOSIZE);
        EndDeferWindowPos(hdwp);
      }
      return TRUE;

    case WM_GETMINMAXINFO:
      ResizeDlg_GetMinMaxInfo(hwnd, lParam);
      return TRUE;


    case WM_NOTIFY:

      if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST)
      {
        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;

        switch (lpnmtv->hdr.code)
        {

          case TVN_SELCHANGED:
            {
              if (pCurrentLexer && pCurrentStyle) {
                _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
              }

              WCHAR name[80] = { L'\0' };
              WCHAR label[128] = { L'\0' };

              //DialogEnableWindow(hwnd, IDC_STYLEEDIT, true);
              //DialogEnableWindow(hwnd, IDC_STYLEFONT, true);
              //DialogEnableWindow(hwnd, IDC_STYLEFORE, true);
              //DialogEnableWindow(hwnd, IDC_STYLEBACK, true);
              //DialogEnableWindow(hwnd, IDC_STYLEDEFAULT, true);

              // a lexer has been selected
              if (!TreeView_GetParent(hwndTV,lpnmtv->itemNew.hItem))
              {
                pCurrentLexer = (PEDITLEXER)lpnmtv->itemNew.lParam;

                if (pCurrentLexer)
                {
                  bIsStyleSelected = false;
                  GetLngString(IDS_MUI_ASSOCIATED_EXT, label, COUNTOF(label));
                  SetDlgItemText(hwnd,IDC_STYLELABEL_ROOT, label);
                  DialogEnableWindow(hwnd,IDC_STYLEEDIT_ROOT,true);
                  SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->szExtensions);
                  DialogEnableWindow(hwnd, IDC_STYLEEDIT_ROOT, true);

                  if (IsLexerStandard(pCurrentLexer)) 
                  {
                    pCurrentStyle = &(pCurrentLexer->Styles[STY_DEFAULT]);
                    iCurStyleIdx = STY_DEFAULT;

                    if (pCurrentStyle->rid == IDS_LEX_STD_STYLE) {
                      GetLngString(IDS_MUI_STY_BASESTD, label, COUNTOF(label));
                    }
                    else {
                      GetLngString(IDS_MUI_STY_BASE2ND, label, COUNTOF(label));
                      DialogEnableWindow(hwnd, IDC_STYLEEDIT_ROOT, false);
                    }
                  }
                  else {
                    pCurrentStyle = &(pCurrentLexer->Styles[STY_DEFAULT]);
                    iCurStyleIdx = STY_DEFAULT;
                    GetLngString(pCurrentLexer->resID, name, COUNTOF(name));
                    FormatLngStringW(label, COUNTOF(label), IDS_MUI_STY_LEXDEF, name);
                  }
                  SetDlgItemText(hwnd, IDC_STYLELABEL, label);
                  SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
                }
                else
                {
                  SetDlgItemText(hwnd,IDC_STYLELABEL_ROOT,L"");
                  DialogEnableWindow(hwnd,IDC_STYLEEDIT_ROOT,false);
                  SetDlgItemText(hwnd, IDC_STYLELABEL, L"");
                  DialogEnableWindow(hwnd, IDC_STYLEEDIT, false);
                }
                DialogEnableWindow(hwnd, IDC_PREVIEW, ((pCurrentLexer == s_pLexCurrent) || (pCurrentLexer == GetCurrentStdLexer())));
              }
              // a style has been selected
              else
              {
                if (pCurrentLexer) {
                  if (IsLexerStandard(pCurrentLexer)) {
                    if (pCurrentLexer->Styles[STY_DEFAULT].rid == IDS_LEX_STD_STYLE)
                      GetLngString(IDS_MUI_STY_BASESTD, label, COUNTOF(label));
                    else
                      GetLngString(IDS_MUI_STY_BASE2ND, label, COUNTOF(label));
                  }
                  else {
                    GetLngString(pCurrentLexer->resID, name, COUNTOF(name));

                    FormatLngStringW(label, COUNTOF(label), IDS_MUI_STY_LEXDEF, name);
                  }
                  SetDlgItemText(hwnd, IDC_STYLELABEL_ROOT, label);

                  SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->Styles[STY_DEFAULT].szValue);
                  DialogEnableWindow(hwnd, IDC_STYLEEDIT_ROOT, false);

                  pCurrentStyle = (PEDITSTYLE)lpnmtv->itemNew.lParam;
                  iCurStyleIdx = -1;
                  int i = 0;
                  while (pCurrentLexer->Styles[i].iStyle != -1) {
                    if (pCurrentLexer->Styles[i].rid == pCurrentStyle->rid) {
                      iCurStyleIdx = i;
                      break;
                    }
                    ++i;
                  }
                  assert(iCurStyleIdx != -1);
                }
                if (pCurrentStyle)
                {
                  bIsStyleSelected = true;
                  GetLngString(pCurrentStyle->rid, name, COUNTOF(name));
                  FormatLngStringW(label, COUNTOF(label), IDS_MUI_STY_LEXSTYLE, name);
                  SetDlgItemText(hwnd, IDC_STYLELABEL, label);
                  SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
                }
                else
                {
                  iCurStyleIdx = -1;
                  SetDlgItemText(hwnd, IDC_STYLELABEL, L"");
                  DialogEnableWindow(hwnd, IDC_STYLEEDIT, false);
                }
              }
            }
            break;

          case TVN_BEGINDRAG:
            {
              TreeView_Select(hwndTV,lpnmtv->itemNew.hItem,TVGN_CARET);

              if (bIsStyleSelected)
                DestroyCursor(SetCursor(LoadCursor(Globals.hInstance,MAKEINTRESOURCE(IDC_COPY))));
              else
                DestroyCursor(SetCursor(LoadCursor(NULL,IDC_NO)));

              SetCapture(hwnd);
              fDragging = true;
            }

        }
      }
      break;


    case WM_MOUSEMOVE:
      {
        HTREEITEM htiTarget;
        TVHITTESTINFO tvht;

        if (fDragging && bIsStyleSelected)
        {
          LONG xCur = (LONG)(short)LOWORD(lParam);
          LONG yCur = (LONG)(short)HIWORD(lParam);

          //ImageList_DragMove(xCur,yCur);
          //ImageList_DragShowNolock(false);

          tvht.pt.x = xCur;
          tvht.pt.y = yCur;

          //ClientToScreen(hwnd,&tvht.pt);
          //ScreenToClient(hwndTV,&tvht.pt);
          MapWindowPoints(hwnd,hwndTV,&tvht.pt,1);

          if ((htiTarget = TreeView_HitTest(hwndTV,&tvht)) != NULL &&
               TreeView_GetParent(hwndTV,htiTarget) != NULL)
          {
            TreeView_SelectDropTarget(hwndTV,htiTarget);
            //TreeView_Expand(hwndTV,htiTarget,TVE_EXPAND);
            TreeView_EnsureVisible(hwndTV,htiTarget);
          }
          else
            TreeView_SelectDropTarget(hwndTV,NULL);

          //ImageList_DragShowNolock(true);
        }
      }
      break;


    case WM_LBUTTONUP:
      {
        if (fDragging && bIsStyleSelected)
        {
          //ImageList_EndDrag();
          HTREEITEM htiTarget = TreeView_GetDropHilight(hwndTV);
          if (htiTarget)
          {
            WCHAR tchCopy[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = { L'\0' };
            TreeView_SelectDropTarget(hwndTV,NULL);
            GetDlgItemText(hwnd,IDC_STYLEEDIT,tchCopy,COUNTOF(tchCopy));
            TreeView_Select(hwndTV,htiTarget,TVGN_CARET);

            // after select, this is new current item
            SetDlgItemText(hwnd,IDC_STYLEEDIT,tchCopy);
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
          }
          ReleaseCapture();
          DestroyCursor(SetCursor(LoadCursor(NULL,IDC_ARROW)));
          fDragging = false;
        }
      }
      break;


    case WM_CANCELMODE:
      {
        if (fDragging)
        {
          //ImageList_EndDrag();
          TreeView_SelectDropTarget(hwndTV,NULL);
          ReleaseCapture();
          DestroyCursor(SetCursor(LoadCursor(NULL,IDC_ARROW)));
          fDragging = false;
        }
      }
      break;


    case WM_COMMAND:
      {
        switch (LOWORD(wParam)) 
        {
        case IDC_SETCURLEXERTV:
          {
            // find current lexer's tree entry
            HTREEITEM hCurrentTVLex = TreeView_GetRoot(hwndTV);
            for (int i = 0; i < COUNTOF(g_pLexArray); ++i) {
              if (g_pLexArray[i] == s_pLexCurrent) {
                break;
              }
              hCurrentTVLex = TreeView_GetNextSibling(hwndTV, hCurrentTVLex); // next
            }
            if (s_pLexCurrent == pCurrentLexer)
              break; // no change

            // collaps current node
            HTREEITEM hSel = TreeView_GetSelection(hwndTV);
            if (hSel) {
              HTREEITEM hPar = TreeView_GetParent(hwndTV, hSel);
              TreeView_Expand(hwndTV, hSel, TVE_COLLAPSE);
              if (hPar)
                TreeView_Expand(hwndTV, hPar, TVE_COLLAPSE);
            }

            // set new lexer
            TreeView_Select(hwndTV, hCurrentTVLex, TVGN_CARET);
            TreeView_Expand(hwndTV, hCurrentTVLex, TVE_EXPAND);

            pCurrentLexer = s_pLexCurrent;
            pCurrentStyle = &(pCurrentLexer->Styles[STY_DEFAULT]);
            iCurStyleIdx = STY_DEFAULT;

            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          }
          break;
        

        case IDC_STYLEFORE:
          if (pCurrentStyle) {
            WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
            GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
            if (Style_SelectColor(hwnd, true, tch, COUNTOF(tch), true)) {
              SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
            }
          }
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;


        case IDC_STYLEBACK:
          if (pCurrentStyle) {
            WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
            GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
            if (Style_SelectColor(hwnd, false, tch, COUNTOF(tch), true)) {
              SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
            }
          }
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;


        case IDC_STYLEFONT:
          if (pCurrentStyle) {
            WCHAR lexerName[BUFSIZE_STYLE_VALUE] = { L'\0' };
            WCHAR styleName[BUFSIZE_STYLE_VALUE] = { L'\0' };
            GetDlgItemText(hwnd, IDC_STYLEEDIT, tchTmpBuffer, COUNTOF(tchTmpBuffer));
            GetLngString(pCurrentLexer->resID, lexerName, COUNTOF(lexerName));
            GetLngString(pCurrentStyle->rid, styleName, COUNTOF(styleName));
            if (Style_SelectFont(hwnd, tchTmpBuffer, COUNTOF(tchTmpBuffer), lexerName, styleName,
                                 IsStyleStandardDefault(pCurrentStyle), IsStyleSchemeDefault(pCurrentStyle), false, true)) {
              SetDlgItemText(hwnd, IDC_STYLEEDIT, tchTmpBuffer);
            }
          }
          PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          break;


        case IDC_STYLEDEFAULT:
          {
            SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->pszDefault);
            if (!bIsStyleSelected) {
              SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->pszDefExt);
            }
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          }
          break;


        case IDC_STYLEEDIT:
          {
            if (HIWORD(wParam) == EN_CHANGE) {
              WCHAR tch[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = { L'\0' };

              GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));

              COLORREF cr = (COLORREF)-1; // SciCall_StyleGetFore(STYLE_DEFAULT);
              Style_StrGetColor(tch, FOREGROUND_LAYER, &cr);
              MakeColorPickButton(hwnd, IDC_STYLEFORE, Globals.hInstance, cr);

              cr = (COLORREF)-1; // SciCall_StyleGetBack(STYLE_DEFAULT);
              Style_StrGetColor(tch, BACKGROUND_LAYER, &cr);
              MakeColorPickButton(hwnd, IDC_STYLEBACK, Globals.hInstance, cr);
            }
          }
          break;


        case IDC_IMPORT:
          {
            hwndTV = GetDlgItem(hwnd, IDC_STYLELIST);

            if (Style_Import(hwnd)) {
              SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
              if (!bIsStyleSelected) {
                SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->szExtensions);
              }
              TreeView_Select(hwndTV, TreeView_GetRoot(hwndTV), TVGN_CARET);
              Style_ResetCurrentLexer(Globals.hwndEdit);
            }
          }
          break;


        case IDC_EXPORT:
          {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
            Style_Export(hwnd);
          }
          break;


        case IDC_PREVIEW:
          {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
          }
          break;


        case IDC_PREVSTYLE:
          {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
            HTREEITEM hSel = TreeView_GetSelection(hwndTV);
            if (hSel) {
              HTREEITEM hPrev = TreeView_GetPrevVisible(hwndTV, hSel);
              if (hPrev)
                TreeView_Select(hwndTV, hPrev, TVGN_CARET);
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          }
          break;


        case IDC_NEXTSTYLE:
          {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
            HTREEITEM hSel = TreeView_GetSelection(hwndTV);
            if (hSel) {
              HTREEITEM hNext = TreeView_GetNextVisible(hwndTV, hSel);
              if (hNext)
                TreeView_Select(hwndTV, hNext, TVGN_CARET);
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
          }
          break;


        case IDOK:
          {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);

            if ((!bWarnedNoIniFile && StrIsEmpty(Theme_Files[s_idxSelectedTheme].szFilePath)) && (s_idxSelectedTheme > 0)) 
            {
              InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SETTINGSNOTSAVED);
              bWarnedNoIniFile = true;
            }
            //EndDialog(hwnd,IDOK);
            DestroyWindow(hwnd);
          }
          break;


        case IDCANCEL:
          if (fDragging) {
            SendMessage(hwnd, WM_CANCELMODE, 0, 0);
          }
          else {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);

            // Restore Styles from Backup
            int cnt = 0;
            for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) 
            {
              StringCchCopy(g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions), Style_StylesBackup[cnt]);

              ++cnt;
              int i = 0;
              while (g_pLexArray[iLexer]->Styles[i].iStyle != -1) 
              {
                // normalize
                tchTmpBuffer[0] = L'\0'; // clear
                Style_CopyStyles_IfNotDefined(Style_StylesBackup[cnt], tchTmpBuffer, COUNTOF(tchTmpBuffer), true, true);
                StringCchCopy(g_pLexArray[iLexer]->Styles[i].szValue, COUNTOF(g_pLexArray[iLexer]->Styles[i].szValue), tchTmpBuffer);
                ++cnt;
                ++i;
              }
            }
            Style_ResetCurrentLexer(Globals.hwndEdit);
            //EndDialog(hwnd,IDCANCEL);
            DestroyWindow(hwnd);
          }
          break;


        case IDACC_VIEWSCHEMECONFIG:
          PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_SETCURLEXERTV, 1), 0);
          break;

        case IDACC_PREVIEW:
          PostMessage(hwnd, WM_COMMAND, MAKELONG(IDC_PREVIEW, 1), 0);
          break;

        case IDACC_SAVEPOS:
          GetDlgPos(hwnd, &Settings.CustomSchemesDlgPosX, &Settings.CustomSchemesDlgPosY);
          break;

        case IDACC_RESETPOS:
          CenterDlgInParent(hwnd);
          Settings.CustomSchemesDlgPosX = Settings.CustomSchemesDlgPosY = CW_USEDEFAULT;
          break;


        default:
          // return false???
          break;

        } // switch()
      } // WM_COMMAND
      return true;
  }
  return false;
}


//=============================================================================
//
//  Style_CustomizeSchemesDlg()
//
HWND Style_CustomizeSchemesDlg(HWND hwnd)
{
  HWND hDlg = CreateThemedDialogParam(Globals.hLngResContainer,
                                      MAKEINTRESOURCE(IDD_MUI_STYLECONFIG),
                                      GetParent(hwnd),
                                      Style_CustomizeSchemesDlgProc,
                                      (LPARAM)NULL);
  if (hDlg != INVALID_HANDLE_VALUE) {
    ShowWindow(hDlg, SW_SHOW);
  }
  return hDlg;
}


//=============================================================================
//
//  Style_SelectLexerDlgProc()
//
INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{

  static int cxClient;
  static int cyClient;
  static int mmiPtMaxY;
  static int mmiPtMinX;

  static HWND hwndLV;
  static int  iInternalDefault;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        if (Globals.hDlgIcon) { SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIcon); }
        
        LVCOLUMN lvc = { LVCF_FMT|LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };

        RECT rc;
        GetClientRect(hwnd,&rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;

        AdjustWindowRectEx(&rc,GetWindowLong(hwnd,GWL_STYLE)|WS_THICKFRAME,false,0);
        mmiPtMinX = rc.right-rc.left;
        mmiPtMaxY = rc.bottom-rc.top;

        if (s_cxStyleSelectDlg < (rc.right-rc.left))
          s_cxStyleSelectDlg = rc.right-rc.left;
        if (s_cyStyleSelectDlg < (rc.bottom-rc.top))
          s_cyStyleSelectDlg = rc.bottom-rc.top;
        SetWindowPos(hwnd,NULL,rc.left,rc.top,s_cxStyleSelectDlg,s_cyStyleSelectDlg,SWP_NOZORDER);

        SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_THICKFRAME);
        SetWindowPos(hwnd,NULL,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        WCHAR tch[MAX_PATH] = { L'\0' };
        GetMenuString(GetSystemMenu(GetParent(hwnd),false),SC_SIZE,tch,COUNTOF(tch),MF_BYCOMMAND);
        InsertMenu(GetSystemMenu(hwnd,false),SC_CLOSE,MF_BYCOMMAND|MF_STRING|MF_ENABLED,SC_SIZE,tch);
        InsertMenu(GetSystemMenu(hwnd,false),SC_CLOSE,MF_BYCOMMAND|MF_SEPARATOR,0,NULL);

        SetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP),GWL_STYLE,
          GetWindowLongPtr(GetDlgItem(hwnd,IDC_RESIZEGRIP),GWL_STYLE)|SBS_SIZEGRIP|WS_CLIPSIBLINGS);

        int cGrip = GetSystemMetricsEx(SM_CXHTHUMB);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,cxClient-cGrip,
                     cyClient-cGrip,cGrip,cGrip,SWP_NOZORDER);

        hwndLV = GetDlgItem(hwnd,IDC_STYLELIST);

        SHFILEINFO shfi;
        ZeroMemory(&shfi, sizeof(SHFILEINFO));
        ListView_SetImageList(hwndLV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\",FILE_ATTRIBUTE_DIRECTORY,
            &shfi,sizeof(SHFILEINFO),SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
          LVSIL_SMALL);

        ListView_SetImageList(hwndLV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\",FILE_ATTRIBUTE_DIRECTORY,
            &shfi,sizeof(SHFILEINFO),SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
          LVSIL_NORMAL);

        //SetExplorerTheme(hwndLV);
        ListView_SetExtendedListViewStyle(hwndLV,/*LVS_EX_FULLROWSELECT|*/LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
        ListView_InsertColumn(hwndLV,0,&lvc);

        // Add lexers
        for (int i = 0; i < COUNTOF(g_pLexArray); i++) {
          Style_AddLexerToListView(hwndLV, g_pLexArray[i]);
        }
        ListView_SetColumnWidth(hwndLV,0,LVSCW_AUTOSIZE_USEHEADER);

        // Select current lexer
        int lvItems = ListView_GetItemCount(hwndLV);
        LVITEM lvi;
        lvi.mask = LVIF_PARAM;
        for (int i = 0; i < lvItems; i++) {
          lvi.iItem = i;
          ListView_GetItem(hwndLV,&lvi);

          if (((PEDITLEXER)lvi.lParam)->resID == s_pLexCurrent->resID)
          {
            ListView_SetItemState(hwndLV,i,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
            ListView_EnsureVisible(hwndLV,i,false);
            CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, SetBtn(s_iDefaultLexer == i));
            break;
          }
        }

        iInternalDefault = s_iDefaultLexer;
        CheckDlgButton(hwnd,IDC_AUTOSELECT, SetBtn(s_bAutoSelect));

        CenterDlgInParent(hwnd);
      }
      return true;


    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, 0, 0, 0, 0);
      return true;


    case WM_DESTROY:
      {
        RECT rc;
        GetWindowRect(hwnd,&rc);
        s_cxStyleSelectDlg = rc.right-rc.left;
        s_cyStyleSelectDlg = rc.bottom-rc.top;
      }
      return false;


    case WM_SIZE:
      {
        RECT rc;

        int dxClient = LOWORD(lParam) - cxClient;
        int dyClient = HIWORD(lParam) - cyClient;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

        GetWindowRect(GetDlgItem(hwnd,IDC_RESIZEGRIP),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_RESIZEGRIP),NULL,true);

        GetWindowRect(GetDlgItem(hwnd,IDOK),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDOK),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDOK),NULL,true);

        GetWindowRect(GetDlgItem(hwnd,IDCANCEL),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDCANCEL),NULL,rc.left+dxClient,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDCANCEL),NULL,true);

        GetWindowRect(GetDlgItem(hwnd,IDC_STYLELIST),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_STYLELIST),NULL,0,0,rc.right-rc.left+dxClient,rc.bottom-rc.top+dyClient,SWP_NOZORDER|SWP_NOMOVE);
        ListView_SetColumnWidth(GetDlgItem(hwnd,IDC_STYLELIST),0,LVSCW_AUTOSIZE_USEHEADER);
        InvalidateRect(GetDlgItem(hwnd,IDC_STYLELIST),NULL,true);

        GetWindowRect(GetDlgItem(hwnd,IDC_AUTOSELECT),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_AUTOSELECT),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_AUTOSELECT),NULL,true);

        GetWindowRect(GetDlgItem(hwnd,IDC_DEFAULTSCHEME),&rc);
        MapWindowPoints(NULL,hwnd,(LPPOINT)&rc,2);
        SetWindowPos(GetDlgItem(hwnd,IDC_DEFAULTSCHEME),NULL,rc.left,rc.top+dyClient,0,0,SWP_NOZORDER|SWP_NOSIZE);
        InvalidateRect(GetDlgItem(hwnd,IDC_DEFAULTSCHEME),NULL,true);
      }
      return true;


    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = mmiPtMinX;
        lpmmi->ptMinTrackSize.y = mmiPtMaxY;
        //lpmmi->ptMaxTrackSize.y = mmiPtMaxY;
      }
      return true;


    case WM_NOTIFY: 
      {
        if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST) {

          switch (((LPNMHDR)(lParam))->code) {

          case NM_DBLCLK:
            SendMessage(hwnd, WM_COMMAND, MAKELONG(IDOK, 1), 0);
            break;

          case LVN_ITEMCHANGED:
          case LVN_DELETEITEM:
            {
              int i = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
              CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, SetBtn(iInternalDefault == i));
              DialogEnableWindow(hwnd, IDC_DEFAULTSCHEME, i != -1);
              DialogEnableWindow(hwnd, IDOK, i != -1);
            }
            break;
          }
        }
      }
      return true;


    case WM_COMMAND:
      {
        switch (LOWORD(wParam)) 
        {
        case IDC_DEFAULTSCHEME:
          if (IsButtonChecked(hwnd, IDC_DEFAULTSCHEME))
            iInternalDefault = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
          else
            iInternalDefault = 0;
          break;


        case IDOK:
          {
            LVITEM lvi;
            lvi.mask = LVIF_PARAM;
            lvi.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
            if (ListView_GetItem(hwndLV, &lvi)) {
              s_pLexCurrent = (PEDITLEXER)lvi.lParam;
              s_iDefaultLexer = iInternalDefault;
              s_bAutoSelect = IsButtonChecked(hwnd, IDC_AUTOSELECT);
              EndDialog(hwnd,IDOK);
            }
          }
          break;


        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;

        } // switch()
      } // WM_COMMAND 
      return true;
  }
  return false;
}


//=============================================================================
//
//  Style_SelectLexerDlg()
//
void Style_SelectLexerDlg(HWND hwnd)
{
  if (IDOK == ThemedDialogBoxParam(Globals.hLngResContainer,
                                   MAKEINTRESOURCE(IDD_MUI_STYLESELECT),
                                   GetParent(hwnd), Style_SelectLexerDlgProc, 0))

    Style_ResetCurrentLexer(Globals.hwndEdit);
}

// End of Styles.c
