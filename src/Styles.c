// encoding: UTF-8
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
*                                                  (c) Rizonesoft 2008-2020   *
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

#include "Edit.h"
#include "Dialogs.h"
#include "resource.h"
#include "Encoding.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"
#include "DarkMode/DarkMode.h"

#include "SciCall.h"

#include "Styles.h"

extern COLORREF  g_colorCustom[16];

// removed from project, not MUI language compatible with ChooseFont()
//~bool ChooseFontDirectWrite(HWND hwnd, const WCHAR* localeName, DPI_T dpi, LPCHOOSEFONT lpCF);

// ----------------------------------------------------------------------------

// This array holds all the lexers...
// Don't forget to change the number of the lexer for HTML and XML
// in Notepad2.c ParseCommandLine() if you change this array!
static PEDITLEXER g_pLexArray[NUMLEXERS] =
{
  &lexStandard,      // Default Text
  &lexStandard2nd,   // 2nd Default Text
  &lexTEXT,          // Pure Text Files (Constants.StdDefaultLexerID = 2)
  &lexANSI,          // ANSI Files
  &lexCONF,          // Apache Config Files
  &lexASM,           // Assembly Script
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
  &lexCSV,           // CSV Prism Color Lexer
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
  &lexTOML,          // TOML Config Script
  &lexVBS,           // VBScript
  &lexVHDL,          // VHDL
  &lexVB,            // Visual Basic
  &lexHTML,          // Web Source Code
  &lexXML,           // XML Document
  &lexYAML           // YAML
};


// Currently used lexer
static int s_iDefaultLexer = 2; // (Constants.StdDefaultLexerID) Pure Text Files
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

static bool s_bAutoSelect = true;

#define STYLESELECTDLG_X 305
#define STYLESELECTDLG_Y 344
static int  s_cxStyleSelectDlg = STYLESELECTDLG_X;
static int  s_cyStyleSelectDlg = STYLESELECTDLG_Y;


//=============================================================================

THEMEFILES Theme_Files[] =
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

static void _FillThemesMenuTable()
{
  Theme_Files[0].rid = IDM_THEMES_DEFAULT;    // factory default
  Theme_Files[1].rid = IDM_THEMES_FILE_ITEM;  // NP3.ini settings

  unsigned iTheme = 1; // Standard

  WCHAR tchThemeDir[MAX_PATH] = { L'\0' };
  // find "themes" sub-dir (side-by-side to Notepad3.ini)
  if (StrIsNotEmpty(Globals.IniFile)) {
    StringCchCopy(tchThemeDir, COUNTOF(tchThemeDir), Globals.IniFile);
    // names are filled by Style_InsertThemesMenu()
    StringCchCopy(Theme_Files[iTheme].szFilePath, COUNTOF(Theme_Files[iTheme].szFilePath), Globals.IniFile);
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
      FindClose(hFindFile);
    }
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
//  Style_InsertThemesMenu()
//
static HMENU s_hmenuThemes = NULL;

bool Style_InsertThemesMenu(HMENU hMenuBar)
{
  if (s_hmenuThemes) { DestroyMenu(s_hmenuThemes); }
  s_hmenuThemes = CreatePopupMenu();
  //int const pos = GetMenuItemCount(hMenuBar) - 2;

  GetLngString(Theme_Files[0].rid, Theme_Files[0].szName, COUNTOF(Theme_Files[0].szName));
  GetLngString(Theme_Files[1].rid, Theme_Files[1].szName, COUNTOF(Theme_Files[1].szName));

  for (unsigned i = 0; i < ThemeItems_CountOf(); ++i)
  {
    if (i == 2) {
      AppendMenu(s_hmenuThemes, MF_SEPARATOR, 0, 0);
    }
    if (Theme_Files[i].rid > 0) {
      AppendMenu(s_hmenuThemes, MF_ENABLED | MF_STRING, Theme_Files[i].rid, Theme_Files[i].szName);
    }
    else {
      break; // done
    }
  }

  // --- insert ---
  WCHAR wchMenuItemStrg[128] = { L'\0' };
  GetLngString(IDS_MUI_MENU_THEMES, wchMenuItemStrg, COUNTOF(wchMenuItemStrg));

  //bool const res = InsertMenu(hMenuBar, pos, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)s_hmenuThemes, wchMenuItemStrg);
  bool const res = InsertMenu(hMenuBar, IDM_VIEW_SCHEMECONFIG, MF_BYCOMMAND | MF_POPUP | MF_STRING, (UINT_PTR)s_hmenuThemes, wchMenuItemStrg);

  CheckCmd(hMenuBar, Theme_Files[Globals.idxSelectedTheme].rid, true);

  if (StrIsEmpty(Theme_Files[Globals.idxSelectedTheme].szFilePath))
  {
    EnableCmd(hMenuBar, Theme_Files[Globals.idxSelectedTheme].rid, false);
  }

  return res;
}


//=============================================================================
//
//  Style_DynamicThemesMenuCmd() - Handles IDS_MUI_MENU_THEMES messages
//
//
static void _EnableSchemeConfig(const bool bEnable)
{
  EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_SCHEMECONFIG, bEnable);
  EnableTool(Globals.hwndToolbar, IDT_VIEW_SCHEMECONFIG, bEnable);
}

void Style_DynamicThemesMenuCmd(int cmd)
{
  unsigned const iThemeIdx = (unsigned)(cmd - IDM_THEMES_DEFAULT); // consecutive IDs
  if (iThemeIdx >= ThemeItems_CountOf()) {
    return;
  }
  if (iThemeIdx == Globals.idxSelectedTheme) { return; }

  CheckCmd(Globals.hMainMenu, Theme_Files[Globals.idxSelectedTheme].rid, false);

  if (Settings.SaveSettings) {
    if (Globals.idxSelectedTheme == 0) {
      // internal defaults
    }
    else if (Globals.idxSelectedTheme == 1) {
      if (!Flags.bSettingsFileSoftLocked) {
        Globals.bCanSaveIniFile = CreateIniFile(Globals.IniFile, NULL);
        if (Globals.bCanSaveIniFile) {
          Style_ExportToFile(Globals.IniFile, Globals.bIniFileFromScratch);
        }
      }
    }
    else if (PathIsExistingFile(Theme_Files[Globals.idxSelectedTheme].szFilePath))
    {
      Style_ExportToFile(Theme_Files[Globals.idxSelectedTheme].szFilePath, false);
    }
  }

  Globals.idxSelectedTheme = iThemeIdx;
  StringCchCopy(Globals.SelectedThemeName, COUNTOF(Globals.SelectedThemeName), Theme_Files[Globals.idxSelectedTheme].szName);

  bool result = true;
  if ((Globals.idxSelectedTheme > 1) && PathIsExistingFile(Theme_Files[Globals.idxSelectedTheme].szFilePath))
  {
    result = Style_ImportFromFile(Theme_Files[Globals.idxSelectedTheme].szFilePath);
  }
  else if (Globals.idxSelectedTheme == 1) {
    result = Style_ImportFromFile(Globals.IniFile);
  }
  else {
    result = Style_ImportFromFile(L"");
  }

  if (result) {
    Style_ResetCurrentLexer(Globals.hwndEdit);
    SendWMSize(Globals.hwndMain, NULL);
    _EnableSchemeConfig(Globals.idxSelectedTheme != 0);
    UpdateUI();
    UpdateToolbar();
    UpdateStatusbar(true);
    UpdateMarginWidth();
  }

  CheckCmd(Globals.hMainMenu, Theme_Files[Globals.idxSelectedTheme].rid, true);
}






//=============================================================================
//
//  IsLexerStandard()
//

inline bool  IsLexerStandard(PEDITLEXER pLexer)
{
  return ( pLexer && ((pLexer == &lexStandard) || (pLexer == &lexStandard2nd)) );
}

inline PEDITLEXER  GetCurrentStdLexer()
{
  return (Style_GetUse2ndDefault() ? &lexStandard2nd : &lexStandard);
}

inline bool  IsStyleStandardDefault(PEDITSTYLE pStyle)
{
  return (pStyle && ((pStyle->rid == IDS_LEX_STD_STYLE) || (pStyle->rid == IDS_LEX_2ND_STYLE)));
}

inline bool  IsStyleSchemeDefault(PEDITSTYLE pStyle)
{
  return (pStyle && (pStyle->rid == IDS_LEX_STR_63126));
}

inline PEDITLEXER  GetDefaultLexer()
{
  return g_pLexArray[s_iDefaultLexer];
}

inline PEDITLEXER  GetLargeFileLexer()
{
  return &lexTEXT;
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
  static float fBaseFontSize = 11.0f;

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
  alpha = clampi(alpha, SC_ALPHA_TRANSPARENT, SC_ALPHA_OPAQUE);

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
  _SetBaseFontSize((float)Globals.InitialFontSize);
  _SetCurrentFontSize((float)Globals.InitialFontSize);

  for (int i = 0; i < 16; ++i) {
    g_colorCustom[i] = s_colorDefault[i];
  }

  _FillThemesMenuTable();

  unsigned iTheme = 1;
  if (StrIsNotEmpty(Globals.SelectedThemeName)) {
    for (; iTheme < ThemeItems_CountOf(); ++iTheme)
    {
      if (StringCchCompareXI(Globals.SelectedThemeName, Theme_Files[iTheme].szName) == 0) { break; }
    }
  }
  Globals.idxSelectedTheme = (iTheme < ThemeItems_CountOf()) ? iTheme : 1;
  StringCchCopy(Globals.SelectedThemeName, COUNTOF(Globals.SelectedThemeName), Theme_Files[Globals.idxSelectedTheme].szName);

  Style_ImportFromFile(Theme_Files[Globals.idxSelectedTheme].szFilePath);
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
  bool bFactoryReset = StrIsEmpty(szFile) ? true : false;

  bool bIsStdIniFile = false;

  if (!bFactoryReset) {
    WCHAR szFilePathNorm[MAX_PATH] = { L'\0' };
    StringCchCopy(szFilePathNorm, COUNTOF(szFilePathNorm), szFile);
    NormalizePathEx(szFilePathNorm, COUNTOF(szFilePathNorm), true, false);
    if (StringCchCompareXI(szFilePathNorm, Globals.IniFile) == 0) {
      bIsStdIniFile = true;
    }
  }

  bool const result = bIsStdIniFile ? OpenSettingsFile(&bIsStdIniFile) :
                     (bFactoryReset ? ResetIniFileCache() : LoadIniFileCache(szFile));

  if (result) {
    if (bFactoryReset)
    {
      for (int i = 0; i < 16; i++) {
        g_colorCustom[i] = s_colorDefault[i];
      }
    }
    else {
      const WCHAR* const CustomColors_Section = L"Custom Colors";

      WCHAR tch[32] = { L'\0' };
      for (int i = 0; i < 16; i++) {
        WCHAR wch[32] = { L'\0' };
        StringCchPrintf(tch, COUNTOF(tch), L"%02i", i + 1);
        int itok = 0;
        if (IniSectionGetString(CustomColors_Section, tch, L"", wch, COUNTOF(wch))) {
          if (wch[0] == L'#') {
            unsigned int irgb;
            itok = swscanf_s(CharNext(wch), L"%x", &irgb);
            if (itok == 1) {
              g_colorCustom[i] = RGB((irgb & 0xFF0000) >> 16, (irgb & 0xFF00) >> 8, irgb & 0xFF);
            }
          }
        }
        if (itok != 1) {
          g_colorCustom[i] = s_colorDefault[i];
        }
      }
    }

    // Styles
    const WCHAR* const IniSecStyles = Constants.Styles_Section;

    // 2nd default
    Style_SetUse2ndDefault(IniSectionGetBool(IniSecStyles, L"Use2ndDefaultStyle", false));

    // default scheme
    s_iDefaultLexer = clampi(IniSectionGetInt(IniSecStyles, L"DefaultScheme", Constants.StdDefaultLexerID), 0, COUNTOF(g_pLexArray) - 1);

    // auto select
    s_bAutoSelect = IniSectionGetBool(IniSecStyles, L"AutoSelect", true);

    // scheme select dlg dimensions
    s_cxStyleSelectDlg = clampi(IniSectionGetInt(IniSecStyles, L"SelectDlgSizeX", STYLESELECTDLG_X), 0, 8192);
    s_cyStyleSelectDlg = clampi(IniSectionGetInt(IniSecStyles, L"SelectDlgSizeY", STYLESELECTDLG_Y), 0, 8192);

    // Lexer 
    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++) {

      LPCWSTR Lexer_Section = g_pLexArray[iLexer]->pszName;

      if ((Globals.iCfgVersionRead < CFG_VER_0004) && (iLexer < 2))
      {
        Lexer_Section = (iLexer == 0) ? L"Default Text" : L"2nd Default Text";
      }

      if (bIsStdIniFile) {
        IniSectionGetString(Lexer_Section, L"FileNameExtensions", g_pLexArray[iLexer]->pszDefExt,
          g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions));
      }
      // don't allow empty extensions settings => use default ext
      if (StrIsEmpty(g_pLexArray[iLexer]->szExtensions)) {
        StringCchCopy(g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions), g_pLexArray[iLexer]->pszDefExt);
      }

      unsigned i = 0;
      while (g_pLexArray[iLexer]->Styles[i].iStyle != -1)
      {
        IniSectionGetString(Lexer_Section, g_pLexArray[iLexer]->Styles[i].pszName,
          g_pLexArray[iLexer]->Styles[i].pszDefault,
          g_pLexArray[iLexer]->Styles[i].szValue,
          COUNTOF(g_pLexArray[iLexer]->Styles[i].szValue));
        ++i;
      }

      if (bIsStdIniFile && (Globals.iCfgVersionRead < CFG_VER_0004))
      {
        // handling "Text Files" lexer
        if (StringCchCompareXI(L"Text Files", g_pLexArray[iLexer]->pszName) == 0)
        {
          if (StrIsNotEmpty(g_pLexArray[0]->szExtensions)) {
            StringCchCopy(g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions), g_pLexArray[0]->szExtensions);
            StrTrim(g_pLexArray[iLexer]->szExtensions, L"; ");
          }
          lexStandard.szExtensions[0] = L'\0';
          lexStandard2nd.szExtensions[0] = L'\0';
          // copy default style
          StringCchCopy(g_pLexArray[iLexer]->Styles[0].szValue, COUNTOF(g_pLexArray[iLexer]->Styles[0].szValue), g_pLexArray[0]->Styles[0].szValue);
        }
      }
    }

    CloseSettingsFile(false, bIsStdIniFile);
  }
  return result;
}


//=============================================================================
//
//  Style_SaveSettings()
//
void Style_SaveSettings(bool bForceSaveSettings)
{
  if (Settings.SaveSettings || bForceSaveSettings)
  {
    Style_ExportToFile(Theme_Files[Globals.idxSelectedTheme].szFilePath, Globals.bIniFileFromScratch);
  }
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

  bool ok = false;
  if (GetSaveFileName(&ofn)) 
  {
    ok = Style_ExportToFile(szFile, true);
    if (!ok) {
      InfoBoxLng(MB_ICONERROR, NULL, IDS_MUI_EXPORT_FAIL, PathFindFileName(szFile));
    }
  }
  return ok;
}


//=============================================================================
//
//  Style_ToIniSection()
//

#define SAVE_STYLE_IF_NOT_EQ_DEFAULT(TYPE, VARNAME, VALUE, DEFAULT)      \
  if ((VALUE) != (DEFAULT)) {                                            \
    IniSectionSet##TYPE(IniSecStyles, _W(_STRG(VARNAME)), (VALUE));      \
  } else {                                                               \
    IniSectionDelete(IniSecStyles, _W(_STRG(VARNAME)), false);           \
  }

void Style_ToIniSection(bool bForceAll, bool bIsStdIniFile)
{
  // Custom colors
  const WCHAR* const CustomColors_Section = L"Custom Colors";

  for (int i = 0; i < 16; i++) {
    WCHAR tch[32] = { L'\0' };
    StringCchPrintf(tch, COUNTOF(tch), L"%02i", i + 1);
    if ((g_colorCustom[i] != s_colorDefault[i]) || (bForceAll && !Globals.bIniFileFromScratch))
    {
      WCHAR wch[32] = { L'\0' };
      StringCchPrintf(wch, COUNTOF(wch), L"#%02X%02X%02X",
        (int)GetRValue(g_colorCustom[i]), (int)GetGValue(g_colorCustom[i]), (int)GetBValue(g_colorCustom[i]));

      IniSectionSetString(CustomColors_Section, tch, wch);
    }
    else {
      IniSectionDelete(CustomColors_Section, tch, false);
    }
  }

  const WCHAR* const IniSecStyles = Constants.Styles_Section;

  // auto select
  bool const bUse2ndSty = Style_GetUse2ndDefault();
  SAVE_STYLE_IF_NOT_EQ_DEFAULT(Bool, Use2ndDefaultStyle, bUse2ndSty, false);

  // default scheme
  SAVE_STYLE_IF_NOT_EQ_DEFAULT(Int, DefaultScheme, s_iDefaultLexer, Constants.StdDefaultLexerID);

  // auto select
  SAVE_STYLE_IF_NOT_EQ_DEFAULT(Bool, AutoSelect, s_bAutoSelect, true);

  // scheme select dlg dimensions
  SAVE_STYLE_IF_NOT_EQ_DEFAULT(Int, SelectDlgSizeX, s_cxStyleSelectDlg, STYLESELECTDLG_X);
  SAVE_STYLE_IF_NOT_EQ_DEFAULT(Int, SelectDlgSizeY, s_cyStyleSelectDlg, STYLESELECTDLG_Y);

  // create canonical order of lexer sections 
  if (bForceAll || Globals.bIniFileFromScratch) {
    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++)
    {
      IniSectionClear(g_pLexArray[iLexer]->pszName, true);
    }
    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++)
    {
      IniSectionSetString(g_pLexArray[iLexer]->pszName, NULL, NULL);
    }
    bForceAll = !Globals.bIniFileFromScratch;
  }
  // ----------------------------------------------------------------

  WCHAR szTmpStyle[BUFSIZE_STYLE_VALUE];

  for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer)
  {
    LPCWSTR const Lexer_Section = g_pLexArray[iLexer]->pszName;

    if (bForceAll || (bIsStdIniFile && (StringCchCompareXI(g_pLexArray[iLexer]->szExtensions, g_pLexArray[iLexer]->pszDefExt) != 0)))
    {
      IniSectionSetString(Lexer_Section, L"FileNameExtensions", g_pLexArray[iLexer]->szExtensions);
    }
    else {
      IniSectionDelete(Lexer_Section, L"FileNameExtensions", false);
    }

    unsigned i = 0;
    while (g_pLexArray[iLexer]->Styles[i].iStyle != -1)
    {
      // normalize defaults
      szTmpStyle[0] = L'\0'; // clear
      Style_CopyStyles_IfNotDefined(g_pLexArray[iLexer]->Styles[i].pszDefault, szTmpStyle, COUNTOF(szTmpStyle), true);

      if (bForceAll || (StringCchCompareX(g_pLexArray[iLexer]->Styles[i].szValue, szTmpStyle) != 0))
      {
        // normalize value
        szTmpStyle[0] = L'\0'; // clear
        Style_CopyStyles_IfNotDefined(g_pLexArray[iLexer]->Styles[i].szValue, szTmpStyle, COUNTOF(szTmpStyle), true);
        IniSectionSetString(Lexer_Section, g_pLexArray[iLexer]->Styles[i].pszName, szTmpStyle);
      }
      else {
        IniSectionDelete(Lexer_Section, g_pLexArray[iLexer]->Styles[i].pszName, false);
      }
      ++i;
    }
  }

  // cleanup old (< v4) stuff 
  IniSectionDelete(L"Default Text", NULL, true);
  IniSectionDelete(L"2nd Default Text", NULL, true);
}


//=============================================================================
//
//  Style_ExportToFile()
//
bool Style_ExportToFile(const WCHAR* szFile, bool bForceAll)
{
  if (StrIsEmpty(szFile)) {
    if (Globals.idxSelectedTheme != 0) {
      InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SETTINGSNOTSAVED);
    }
    return false;
  }

  WCHAR szFilePathNorm[MAX_PATH] = { L'\0' };
  StringCchCopy(szFilePathNorm, COUNTOF(szFilePathNorm), szFile);
  NormalizePathEx(szFilePathNorm, COUNTOF(szFilePathNorm), true, false);

  bool ok = false;
  bool bIsStdIniFile = false;

  if (StringCchCompareXI(szFilePathNorm, Globals.IniFile) == 0) {
    bool bOpendByMe = false;
    bIsStdIniFile = true;
    if (OpenSettingsFile(&bOpendByMe)) {
      Style_ToIniSection(bForceAll, bIsStdIniFile);
      ok = CloseSettingsFile(true, bOpendByMe);
    }
  }
  else {
    if (StrIsNotEmpty(szFilePathNorm))
    {
      if (!PathIsExistingFile(szFilePathNorm)) {
        HANDLE hFile = CreateFile(szFilePathNorm,
          GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
          CloseHandle(hFile); // done
        }
      }
      if (LoadIniFileCache(szFilePathNorm)) {
        Style_ToIniSection(bForceAll, bIsStdIniFile);
        ok = SaveIniFileCache(szFilePathNorm);
        ResetIniFileCache();
      }
    }
  }
  return ok;
}


//=============================================================================
//
//  Style_SetFoldingAvailability()
//
void Style_SetFoldingAvailability(PEDITLEXER pLexer)
{
  switch (pLexer->lexerID)
  {
    case SCLEX_NULL:
    case SCLEX_CONTAINER:
    case SCLEX_BATCH:
    case SCLEX_CONF:
    case SCLEX_MAKEFILE:
    case SCLEX_MARKDOWN:
      FocusedView.CodeFoldingAvailable = false;
      break;
    default:
      FocusedView.CodeFoldingAvailable = true;
      break;
  }
}


//=============================================================================
//
//  Style_SetFoldingProperties()
//
void Style_SetFoldingProperties(bool active)
{
  if (active) {
    SciCall_SetProperty("fold", "1");
    SciCall_SetProperty("fold.comment", "1");
    SciCall_SetProperty("fold.compact", "0");
    SciCall_SetProperty("fold.foldsyntaxbased", "1");

    SciCall_SetProperty("fold.html", "1");
    SciCall_SetProperty("fold.preprocessor", "1");
    SciCall_SetProperty("fold.cpp.comment.explicit", "0");
  }
  else {
    SciCall_SetProperty("fold", "0");
  }
}


//=============================================================================
//
//  Style_SetFoldingFocusedView()
//
void Style_SetFoldingFocusedView()
{
  SciCall_SetProperty("fold", "0"); // disable folding by lexers
  SciCall_SetProperty("fold.comment", "1");
  SciCall_SetProperty("fold.compact", "0");
}


//=============================================================================
//
//  void Style_SetLexerSpecificProperties()
//
void Style_SetLexerSpecificProperties(const int lexerId)
{
  switch (lexerId)
  {
    case SCLEX_CPP:
      SciCall_SetProperty("styling.within.preprocessor", "1");
      SciCall_SetProperty("lexer.cpp.track.preprocessor", "0");
      SciCall_SetProperty("lexer.cpp.update.preprocessor", "0");
      break;

    case SCLEX_PASCAL:
      SciCall_SetProperty("lexer.pascal.smart.highlighting", "1");
      break;

    case SCLEX_SQL:
      SciCall_SetProperty("sql.backslash.escapes", "1");
      SciCall_SetProperty("lexer.sql.backticks.identifier", "1");
      SciCall_SetProperty("lexer.sql.numbersign.comment", Settings2.LexerSQLNumberSignAsComment ? "1" : "0");
      break;

    case SCLEX_NSIS:
      SciCall_SetProperty("nsis.ignorecase", "1");
      break;

    case SCLEX_CSS:
      SciCall_SetProperty("lexer.css.scss.language", "1");
      SciCall_SetProperty("lexer.css.less.language", "1");
      break;

    case SCLEX_JSON:
      SciCall_SetProperty("json.allow.comments", "1");
      SciCall_SetProperty("json.escape.sequence", "1");
      break;

    case SCLEX_PYTHON:
      SciCall_SetProperty("tab.timmy.whinge.level", "1");
      break;

    case SCLEX_XML:
      SciCall_SetProperty("lexer.xml.allow.scripts", "1");
      break;

    default:
      break;
  }
}


//=============================================================================
//
//  Style_StrGetAttributeEx()
//
// zufuliu: parse a style attribute separated by ';'
// e.g.: 'bold', 'bold;', '; bold' and '; bold;'
//
static bool Style_StrGetAttributeEx(LPCWSTR lpszStyle, LPCWSTR key, const size_t keyLen)
{
  LPCWSTR p = StrStrI(lpszStyle, key);
  while (p) {
    WCHAR chPrev = (p == lpszStyle) ? L';' : p[-1];
    if (chPrev == L' ') {
      LPCWSTR t = p - 2;
      while ((t > lpszStyle) && (*t == L' ')) { --t; }
      chPrev = (t <= lpszStyle) ? L';' : *t;
    }
    p += keyLen;
    if (chPrev == L';') {
      while (*p == L' ') {
        ++p;
      }
      if (*p == L'\0' || *p == L';') {
        return true;
      }
    }
    p = StrStrI(p, key);
  }
  return false;
}

#define Style_StrGetAttribute(lpszStyle, name)  Style_StrGetAttributeEx((lpszStyle), (name), StringCchLen((name),0))

// font weight
#define Style_StrGetAttrThin(lpszStyle)         Style_StrGetAttribute((lpszStyle), L"thin")
#define Style_StrGetAttrExtraLight(lpszStyle)   Style_StrGetAttribute((lpszStyle), L"extralight")
#define Style_StrGetAttrLight(lpszStyle)        Style_StrGetAttribute((lpszStyle), L"light")
#define Style_StrGetAttrNormal(lpszStyle)       Style_StrGetAttribute((lpszStyle), L"normal")
#define Style_StrGetAttrMedium(lpszStyle)       Style_StrGetAttribute((lpszStyle), L"medium")
#define Style_StrGetAttrSemiBold(lpszStyle)     Style_StrGetAttribute((lpszStyle), L"semibold")
#define Style_StrGetAttrBold(lpszStyle)         Style_StrGetAttribute((lpszStyle), L"bold")
#define Style_StrGetAttrExtraBold(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"extrabold")
#define Style_StrGetAttrHeavy(lpszStyle)        Style_StrGetAttribute((lpszStyle), L"heavy")

#if 0
// font stretch
#define Style_StrGetAttrUltraCondensed(lpszStyle)   Style_StrGetAttribute((lpszStyle), L"ultracondensed")
#define Style_StrGetAttrExtraCondensed(lpszStyle)   Style_StrGetAttribute((lpszStyle), L"extracondensed")
#define Style_StrGetAttrCondensed(lpszStyle)        Style_StrGetAttribute((lpszStyle), L"condensed")
#define Style_StrGetAttrSemiCondensed(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"semicondensed")
#define Style_StrGetAttrNormalStretch(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"normalstretch")
#define Style_StrGetAttrMediumStretch(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"mediumstretch")
#define Style_StrGetAttrSemiExpanded(lpszStyle)     Style_StrGetAttribute((lpszStyle), L"semiexpanded")
#define Style_StrGetAttrExpanded(lpszStyle)         Style_StrGetAttribute((lpszStyle), L"expanded")
#define Style_StrGetAttrExtraExpanded(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"extraexpanded")
#define Style_StrGetAttrUltraExpanded(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"ultraexpanded")
#endif

//// font quality
//#define Style_StrGetAttrNone(lpszStyle)         Style_StrGetAttribute((lpszStyle), L"none")
//#define Style_StrGetAttrStdType(lpszStyle)      Style_StrGetAttribute((lpszStyle), L"standard")
//#define Style_StrGetAttrClearType(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"cleartype")

// font effects
#define Style_StrGetAttrItalic(lpszStyle)       Style_StrGetAttribute((lpszStyle), L"italic")
#define Style_StrGetAttrUnderline(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"underline")
#define Style_StrGetAttrStrikeOut(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"strikeout")
#define Style_StrGetAttrEOLFilled(lpszStyle)    Style_StrGetAttribute((lpszStyle), L"eolfilled")

// caret style
#define Style_StrGetAttrOvrBlck(lpszStyle)      Style_StrGetAttribute((lpszStyle), L"ovrblck")
#define Style_StrGetAttrBlock(lpszStyle)        Style_StrGetAttribute((lpszStyle), L"block")
#define Style_StrGetAttrNoBlink(lpszStyle)      Style_StrGetAttribute((lpszStyle), L"noblink")


//=============================================================================
//
//  Style_SetLexer()
//
void Style_SetLexer(HWND hwnd, PEDITLEXER pLexNew)
{
  // Select standard if NULL is specified
  if (!pLexNew) {
    pLexNew = Flags.bLargeFileLoaded ? GetLargeFileLexer() : GetDefaultLexer();
    if (IsLexerStandard(pLexNew)) {
      pLexNew = GetCurrentStdLexer();
    }
  }
  bool const bFocusedView = FocusedView.HideNonMatchedLines;
  if (bFocusedView) { EditToggleView(Globals.hwndEdit); }

  _IGNORE_NOTIFY_CHANGE_;

  // ! dont check for (pLexNew == s_pLexCurrent) <= "reapply current lexer"
  // assert(pLexNew != s_pLexCurrent);

  // first set standard lexer's default values
  const PEDITLEXER pCurrentStandard = (IsLexerStandard(pLexNew)) ? pLexNew : GetCurrentStdLexer();

  Style_SetUse2ndDefault(pCurrentStandard == &lexStandard2nd); // sync if forced

  // Lexer 
  SendMessage(hwnd, SCI_SETLEXER, pLexNew->lexerID, 0);

  // Lexer very specific styles
  Style_SetLexerSpecificProperties(pLexNew->lexerID);
  
  // Code folding
  Style_SetFoldingAvailability(pLexNew);
  Style_SetFoldingProperties(FocusedView.CodeFoldingAvailable);


  // Add KeyWord Lists
  for (int i = 0;  i <= KEYWORDSET_MAX;  ++i)
  {
    const char* pKeyWordList = pLexNew->pKeyWords->pszKeyWords[i];
    SciCall_SetKeywords(i, (pKeyWordList ? pKeyWordList : ""));
  }

  // --------------------------------------------------------------------------

  // Clear
  SciCall_ClearDocumentStyle();

  // Default Values are always set
  SciCall_StyleResetDefault();

  // Constants
  SciCall_StyleSetVisible(STYLE_DEFAULT, true);

  //~Style_SetACPfromCharSet(hwnd);

  // ---  apply/init  default style  ---
  _SetBaseFontSize((float)Globals.InitialFontSize);
  _SetCurrentFontSize((float)Globals.InitialFontSize);
  const WCHAR* const wchStandardStyleStrg = pCurrentStandard->Styles[STY_DEFAULT].szValue;
  Style_SetStyles(hwnd, STYLE_DEFAULT, wchStandardStyleStrg, true);

  // ---  apply current scheme specific settings to default style  ---

  const WCHAR* const wchNewLexerStyleStrg = pLexNew->Styles[STY_DEFAULT].szValue;

  if (IsLexerStandard(pLexNew))
  {
    // styles are already set
    EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CURRENTSCHEME, true);
  }
  else {
    // merge lexer default styles
    Style_SetStyles(hwnd, STYLE_DEFAULT, wchNewLexerStyleStrg, false);
    EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CURRENTSCHEME, !IsWindow(Globals.hwndDlgCustomizeSchemes));
  }

  // Broadcast STYLE_DEFAULT as base style to all other styles
  SendMessage(hwnd, SCI_STYLECLEARALL, 0, 0);

  // --------------------------------------------------------------------------
  
  Style_SetMargin(hwnd, pCurrentStandard->Styles[STY_MARGIN].iStyle,
                  pCurrentStandard->Styles[STY_MARGIN].szValue); // margin (line number, bookmarks, folding) style

  int iValue;
  COLORREF dColor;
  WCHAR wchSpecificStyle[128] = { L'\0' };

  if (Settings2.UseOldStyleBraceMatching) {
    Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_OK].iStyle,
      pCurrentStandard->Styles[STY_BRACE_OK].szValue, false); // brace light
  }
  else {
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_BRACE_OK].szValue, FOREGROUND_LAYER, &dColor))
      SciCall_IndicSetFore(INDIC_NP3_MATCH_BRACE, dColor);
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_OK].szValue, &iValue, true))
      SciCall_IndicSetAlpha(INDIC_NP3_MATCH_BRACE, iValue);
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_OK].szValue, &iValue, false))
      SciCall_IndicSetOutlineAlpha(INDIC_NP3_MATCH_BRACE, iValue);

    iValue = -1; // need for retrieval
    if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_BRACE_OK].szValue, 0, &iValue)) {
      // got default, get string
      StringCchCatW(pCurrentStandard->Styles[STY_BRACE_OK].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
      Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &iValue);
      StringCchCatW(pCurrentStandard->Styles[STY_BRACE_OK].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchSpecificStyle);
    }
    SciCall_IndicSetStyle(INDIC_NP3_MATCH_BRACE, iValue);
  }
  if (Settings2.UseOldStyleBraceMatching) {
    Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_BAD].iStyle,
      pCurrentStandard->Styles[STY_BRACE_BAD].szValue, false); // brace bad
  }
  else {
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, FOREGROUND_LAYER, &dColor))
      SciCall_IndicSetFore(INDIC_NP3_BAD_BRACE, dColor);
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, &iValue, true))
      SciCall_IndicSetAlpha(INDIC_NP3_BAD_BRACE, iValue);
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, &iValue, false))
      SciCall_IndicSetOutlineAlpha(INDIC_NP3_BAD_BRACE, iValue);

    iValue = -1; // need for retrieval
    if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, 0, &iValue)) {
      // got default, get string
      StringCchCatW(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
      Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &iValue);
      StringCchCatW(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchSpecificStyle);
    }
    SciCall_IndicSetStyle(INDIC_NP3_BAD_BRACE, iValue);
  }

  // Occurrences Marker
  if (!Style_StrGetColor(pCurrentStandard->Styles[STY_MARK_OCC].szValue, FOREGROUND_LAYER, &dColor))
  {
    dColor = GetSysColor(COLOR_HIGHLIGHT);

    WCHAR sty[32] = { L'\0' };
    StringCchPrintf(sty, COUNTOF(sty), L"; fore:#%02X%02X%02X", (int)GetRValue(dColor), (int)GetGValue(dColor), (int)GetBValue(dColor));
    StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), sty);
  }
  SendMessage(hwnd, SCI_INDICSETFORE, INDIC_NP3_MARK_OCCURANCE, dColor);

  if (!Style_StrGetAlpha(pCurrentStandard->Styles[STY_MARK_OCC].szValue, &iValue, true)) {
    iValue = 60; // force
    StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; alpha:60");
  }   
  SendMessage(hwnd, SCI_INDICSETALPHA, INDIC_NP3_MARK_OCCURANCE, iValue);

  if (!Style_StrGetAlpha(pCurrentStandard->Styles[STY_MARK_OCC].szValue, &iValue, false)) {
    iValue = 60; // force
    StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; alpha2:60");
  }
  SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, INDIC_NP3_MARK_OCCURANCE, iValue);

  iValue = -1; // need for retrieval
  if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_MARK_OCC].szValue, 0, &iValue)) {
    // got default, get string
    StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
    Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &iValue);
    StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchSpecificStyle);
  }
  SendMessage(hwnd, SCI_INDICSETSTYLE, INDIC_NP3_MARK_OCCURANCE, iValue);

  // --------------------------------------------------------------
  // COLOR definitions (INDIC_NP3_COLOR_DEF) are not configurable 
  // --------------------------------------------------------------

  // Unicode-Point Indicator (Hover)
  //SciCall_IndicSetFore(INDIC_NP3_UNICODE_POINT, RGB(0x00, 0x00, 0xF0));
  SciCall_IndicSetStyle (INDIC_NP3_UNICODE_POINT, INDIC_COMPOSITIONTHIN); // simple underline

  if (Style_StrGetColor(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, FOREGROUND_LAYER, &dColor))
    SciCall_IndicSetHoverFore(INDIC_NP3_UNICODE_POINT, dColor);
  if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, &iValue, true))
    SciCall_IndicSetAlpha(INDIC_NP3_UNICODE_POINT, iValue);
  if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, &iValue, false))
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_UNICODE_POINT, iValue);
  
  iValue = -1; // need for retrieval
  if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, 0, &iValue)) {
    // got default, get string
    StringCchCatW(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
    Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &iValue);
    StringCchCatW(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchSpecificStyle);
  }
  SciCall_IndicSetHoverStyle(INDIC_NP3_UNICODE_POINT, iValue);

  // Multi Edit Indicator
  if (Style_StrGetColor(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, FOREGROUND_LAYER, &dColor))
    SciCall_IndicSetFore(INDIC_NP3_MULTI_EDIT, dColor);
  if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, &iValue, true))
    SciCall_IndicSetAlpha(INDIC_NP3_MULTI_EDIT, iValue);
  if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, &iValue, false))
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_MULTI_EDIT, iValue);

  iValue = -1; // need for retrieval
  if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, 0, &iValue)) {
    // got default, get string
    StringCchCatW(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
    Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &iValue);
    StringCchCatW(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchSpecificStyle);
  }
  SciCall_IndicSetStyle(INDIC_NP3_MULTI_EDIT, iValue);


  // Inline-IME Color
  #define _SC_INDIC_IME_INPUT     (INDIC_IME + 0)
  #define _SC_INDIC_IME_TARGET    (INDIC_IME + 1)
  #define _SC_INDIC_IME_CONVERTED (INDIC_IME + 2)
  #define _SC_INDIC_IME_UNKNOWN    INDIC_IME_MAX

  COLORREF rgb = RGB(0xFF, 0xA0, 0x00);
  Style_StrGetColor(pCurrentStandard->Styles[STY_IME_COLOR].szValue, FOREGROUND_LAYER, &rgb); // IME foregr
  SciCall_IndicSetFore(_SC_INDIC_IME_INPUT, rgb);
  SciCall_IndicSetFore(_SC_INDIC_IME_TARGET, rgb);
  SciCall_IndicSetFore(_SC_INDIC_IME_CONVERTED, rgb);
  SciCall_IndicSetFore(_SC_INDIC_IME_UNKNOWN, rgb);

  if (pLexNew != &lexANSI) 
  {
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

  rgb = RGB(0xC0, 0xC0, 0xC0);
  Style_StrGetColor(pCurrentStandard->Styles[STY_SEL_TXT].szValue, BACKGROUND_LAYER, &rgb); // selection back
  SendMessage(hwnd, SCI_SETSELBACK, true, rgb);
  SendMessage(hwnd, SCI_SETADDITIONALSELBACK, rgb, 0);

  if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_SEL_TXT].szValue, &iValue, true)) { // selection alpha
    SendMessage(hwnd, SCI_SETSELALPHA, iValue, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELALPHA, iValue*2/3, 0);
  }
  else {
    SendMessage(hwnd, SCI_SETSELALPHA, SC_ALPHA_NOALPHA, 0);
    SendMessage(hwnd, SCI_SETADDITIONALSELALPHA, SC_ALPHA_OPAQUE*2/3, 0);
  }

  if (Style_StrGetAttrEOLFilled(pCurrentStandard->Styles[STY_SEL_TXT].szValue)) // selection eolfilled
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
  Style_SetBookmark(hwnd, Settings.ShowBookmarkMargin);

  // Hyperlink (URL) indicators
  Style_SetUrlHotSpot(hwnd);

  // caret style and width
  int const ovrstrk_mode = (StrStr(pCurrentStandard->Styles[STY_CARET].szValue, L"ovrblck")) ? 
    CARETSTYLE_OVERSTRIKE_BLOCK : CARETSTYLE_OVERSTRIKE_BAR;

  if (StrStr(pCurrentStandard->Styles[STY_CARET].szValue, L"block")) {
    StringCchCat(wchSpecificStyle, COUNTOF(wchSpecificStyle), L"; block");
    SendMessage(hwnd, SCI_SETCARETSTYLE, (CARETSTYLE_BLOCK | ovrstrk_mode), 0);
  }
  else {
    SendMessage(hwnd, SCI_SETCARETSTYLE, (CARETSTYLE_LINE | ovrstrk_mode), 0);

    iValue = 1;
    fValue = 1.0f;  // default caret width
    WCHAR wch[32] = { L'\0' };
    if (Style_StrGetSize(pCurrentStandard->Styles[STY_CARET].szValue, &fValue)) {
      iValue = clampi(float2int(fValue), 1, 3); // don't allow invisible 0
    }
    SendMessage(hwnd, SCI_SETCARETWIDTH, iValue, 0);

    if (iValue != 1) {
      StringCchPrintf(wch, COUNTOF(wch), L"; size:%i", iValue);
    }
    StringCchCat(wchSpecificStyle, COUNTOF(wchSpecificStyle), wch);
  }
  if (CARETSTYLE_OVERSTRIKE_BLOCK == ovrstrk_mode) {
    StringCchCat(wchSpecificStyle, COUNTOF(wchSpecificStyle), L"; ovrblck");
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

  int edgeColumns[MIDSZ_BUFFER] = { 0 };
  size_t const cnt = ReadVectorFromString(Globals.fvCurFile.wchMultiEdgeLines, edgeColumns, COUNTOF(edgeColumns), 0, LONG_LINES_MARKER_LIMIT, 0, true);
  Style_SetMultiEdgeLine(edgeColumns, cnt);

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

  // Lexer reserved indicator styles
  switch (s_pLexCurrent->lexerID)
  {
    case SCLEX_PYTHON:
      SendMessage(hwnd, SCI_INDICSETSTYLE, 1, INDIC_BOX);
      SendMessage(hwnd, SCI_INDICSETFORE, 1, (LPARAM)RGB(0xBF, 0, 0)); // (light red)
      //SendMessage(hwnd, SCI_INDICSETALPHA, 1, 40);                  
      //SendMessage(hwnd, SCI_INDICSETOUTLINEALPHA, 1, 100);
      break;

    default:
      //SendMessage(hwnd, SCI_INDICSETSTYLE, 0, INDIC_SQUIGGLE);
      //SendMessage(hwnd, SCI_INDICSETSTYLE, 1, INDIC_TT);
      //SendMessage(hwnd, SCI_INDICSETSTYLE, 2, INDIC_PLAIN);
      //SendMessage(hwnd, SCI_INDICSETFORE, 0, (LPARAM)RGB(0, 0x7f, 0)); // (dark green)
      //SendMessage(hwnd, SCI_INDICSETFORE, 1, (LPARAM)RGB(0, 0, 0xff)); // (light blue)
      //SendMessage(hwnd, SCI_INDICSETFORE, 2, (LPARAM)RGB(0xff, 0, 0)); // (light red)
      //for (int sty = 3; sty < INDIC_CONTAINER; ++sty) {
      //  SendMessage(hwnd, SCI_INDICSETSTYLE, sty, INDIC_ROUNDBOX);
      //}
      break;
  }

  Style_SetInvisible(hwnd, false); // set fixed invisible style

  _OBSERVE_NOTIFY_CHANGE_;

	SciCall_SetLayoutCache(SC_CACHE_PAGE); //~SC_CACHE_DOCUMENT ~ memory consumption !
  SciCall_SetPositionCache(SciCall_GetPositionCache()); // clear - default=1024

  //~Sci_LexerStyleAll();
  SciCall_StartStyling(0);

  // apply lexer styles
  if (Flags.bLargeFileLoaded)
  {
    SciCall_SetIdleStyling(SC_IDLESTYLING_ALL);
    EditUpdateVisibleIndicators();
  }
  else {
    SciCall_SetIdleStyling(SC_IDLESTYLING_NONE);
    EditDoStyling(0, -1);
    EditUpdateIndicators(0, -1, false);
  }
 
  if (bFocusedView) { EditToggleView(Globals.hwndEdit); }

  UpdateToolbar();
  UpdateStatusbar(true);
  UpdateMarginWidth();
}


//=============================================================================
//
//  Style_SetUrlHotSpot()
//
void Style_SetUrlHotSpot(HWND hwnd)
{
  UNUSED(hwnd);

  WCHAR* lpszStyleHotSpot = GetCurrentStdLexer()->Styles[STY_URL_HOTSPOT].szValue;
  int const cCount = COUNTOF(GetCurrentStdLexer()->Styles[STY_URL_HOTSPOT].szValue);

  int indicHoverStyle = -1; // need for retrieval
  if (!Style_GetIndicatorType(lpszStyleHotSpot, cCount, &indicHoverStyle)) {
    // got default, get string
    WCHAR wchSpecificStyle[80] = { L'\0' };
    StringCchCatW(lpszStyleHotSpot, cCount, L"; ");
    Style_GetIndicatorType(wchSpecificStyle, COUNTOF(wchSpecificStyle), &indicHoverStyle);
    StringCchCatW(lpszStyleHotSpot, cCount, wchSpecificStyle);
  }
 
  COLORREF activeFG = RGB(0x00, 0x00, 0xFF);
  Style_StrGetColor(lpszStyleHotSpot, FOREGROUND_LAYER, &activeFG);
  COLORREF inactiveFG = RGB(0x00, 0x00, 0xBF);
  if (!Style_StrGetColor(lpszStyleHotSpot, BACKGROUND_LAYER, &inactiveFG)) {
    inactiveFG = (COLORREF)((activeFG * 75 + 50) / 100);
  }
  
  int iValue = 40;
  if (Style_StrGetAlpha(lpszStyleHotSpot, &iValue, true)) {
    SciCall_IndicSetAlpha(INDIC_NP3_HYPERLINK_U, iValue);
  }
  iValue = 80;
  if (Style_StrGetAlpha(lpszStyleHotSpot, &iValue, false)) {
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_HYPERLINK_U, iValue);
  }

  // normal (fix)
  SciCall_IndicSetStyle(INDIC_NP3_HYPERLINK, INDIC_TEXTFORE);
  SciCall_IndicSetFore(INDIC_NP3_HYPERLINK, inactiveFG);
  SciCall_IndicSetStyle(INDIC_NP3_HYPERLINK_U, INDIC_PLAIN);
  SciCall_IndicSetFore(INDIC_NP3_HYPERLINK_U, inactiveFG);
  // hover (stylish)
  SciCall_IndicSetHoverStyle(INDIC_NP3_HYPERLINK, INDIC_TEXTFORE);
  SciCall_IndicSetHoverFore(INDIC_NP3_HYPERLINK, activeFG);
  SciCall_IndicSetHoverStyle(INDIC_NP3_HYPERLINK_U, indicHoverStyle);
  SciCall_IndicSetHoverFore(INDIC_NP3_HYPERLINK_U, activeFG);

  // style for hotspot 
  //~SciCall_StyleSetHotspot(_STYLE_GETSTYLEID(STY_URL_HOTSPOT), true);
  //~SciCall_SetHotspotActiveUnderline(false);
  //~SciCall_SetHotspotSigleLine(true);
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
//  Style_SetReadonly()
//
void Style_SetReadonly(HWND hwnd, bool bReadonly)
{
  SendMessage(hwnd, SCI_STYLESETCHANGEABLE, (WPARAM)Style_GetReadonlyStyleID(), (LPARAM)!bReadonly);
}


//=============================================================================
//
//  Style_SetMultiEdgeLine()
//
void Style_SetMultiEdgeLine(const int colVec[], const size_t count)
{
  COLORREF rgb;

  int const iLongLineLimit = (count > 0) ? colVec[count - 1] : Settings.LongLinesLimit;
  int const mLongLineMode = (count > 1) ? EDGE_MULTILINE : Settings.LongLineMode;

  Settings.LongLinesLimit = iLongLineLimit; // normalize
  Globals.iWrapCol = iLongLineLimit;  // long line limit should be explicit wrap column too

  if (mLongLineMode == EDGE_BACKGROUND) {
    if (!Style_StrGetColor(GetCurrentStdLexer()->Styles[STY_LONG_LN_MRK].szValue, BACKGROUND_LAYER, &rgb)) { // edge back
      rgb = GetSysColor(COLOR_3DSHADOW);
    }
  }
  else {
    if (!Style_StrGetColor(GetCurrentStdLexer()->Styles[STY_LONG_LN_MRK].szValue, FOREGROUND_LAYER, &rgb)) { // edge fore
      rgb = GetSysColor(COLOR_3DLIGHT);
    }
  }
  if (Settings.MarkLongLines) 
  {
    switch (mLongLineMode) {
      case EDGE_LINE:
      case EDGE_BACKGROUND:
        SciCall_SetEdgeMode(mLongLineMode);
        SciCall_SetEdgeColour(rgb);
        SciCall_SetEdgeColumn(iLongLineLimit);
        break;
      case EDGE_MULTILINE:
        SciCall_SetEdgeMode(mLongLineMode);
        SciCall_MultiEdgeClearAll();
        for (size_t i = 0; i < count; ++i) {
          SciCall_MultiEdgeAddLine(colVec[i], rgb);
        }
        break;
      default:
        SciCall_SetEdgeMode(EDGE_NONE);
        break;
    }
  }
  else {
    SciCall_SetEdgeMode(EDGE_NONE);
  }
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

    int alpha = SC_ALPHA_TRANSPARENT;
    if (!Style_StrGetAlpha(GetCurrentStdLexer()->Styles[STY_CUR_LN].szValue, &alpha, true)) {
      alpha = SC_ALPHA_NOALPHA;
    }

    if (!backgrColor) {
      int iFrameSize = 0;
      if (!Style_StrGetSizeInt(szValue, &iFrameSize)) {
        iFrameSize = 2;
      }
      iFrameSize = max_i(1, ScaleIntToDPI_Y(hwnd, iFrameSize));
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
static int  _GetMarkerMarginWidth(HWND hwnd)
{
  float fSize = Style_GetBaseFontSize();
  Style_StrGetSize(GetCurrentStdLexer()->Styles[STY_MARGIN].szValue, &fSize);     // relative to LineNumber
  Style_StrGetSize(GetCurrentStdLexer()->Styles[STY_BOOK_MARK].szValue, &fSize);  // settings
  float const zoomPercent = (float)SciCall_GetZoom();
  return ScaleFloatToDPI_X(hwnd, (fSize * zoomPercent) / 100.0f);
}

//=============================================================================
//
//  Style_SetFolding()
//
void Style_SetFolding(HWND hwnd, bool bShowMargin)
{
  SciCall_SetMarginWidthN(MARGIN_SCI_FOLDING, (bShowMargin ? _GetMarkerMarginWidth(hwnd) : 0));
}

//=============================================================================
//
//  Style_SetBookmark()
//
void Style_SetBookmark(HWND hwnd, bool bShowMargin)
{
  SciCall_SetMarginWidthN(MARGIN_SCI_BOOKMRK, (bShowMargin ? _GetMarkerMarginWidth(hwnd) + 4 : 0));
}


//=============================================================================
//
//  Style_SetMargin()
//
void Style_SetMargin(HWND hwnd, int iStyle, LPCWSTR lpszStyle)
{
  if (iStyle == STYLE_LINENUMBER) {
    Style_SetStyles(hwnd, iStyle, lpszStyle, false);   // line numbers
    SciCall_SetMarginSensitiveN(MARGIN_SCI_LINENUM, false); // allow selection drag
  }

  COLORREF clrFore = SciCall_StyleGetFore(STYLE_LINENUMBER);
  Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &clrFore);

  COLORREF clrBack = SciCall_StyleGetBack(STYLE_LINENUMBER);
  Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &clrBack);
  //~SciCall_SetMarginBackN(MARGIN_SCI_LINENUM, clrBack);

  // CallTips
  SciCall_CallTipSetFore(clrFore);
  SciCall_CallTipSetBack(clrBack);

  // ---  Bookmarks  ---
  COLORREF bmkFore = clrFore;
  COLORREF bmkBack = clrBack;

  const WCHAR* const wchBookMarkStyleStrg = GetCurrentStdLexer()->Styles[STY_BOOK_MARK].szValue;

  Style_StrGetColor(wchBookMarkStyleStrg, FOREGROUND_LAYER, &bmkFore);
  Style_StrGetColor(wchBookMarkStyleStrg, BACKGROUND_LAYER, &bmkBack);

  // adjust background color by alpha in case of show margin
  int alpha = 20;
  Style_StrGetAlpha(wchBookMarkStyleStrg, &alpha, true);

  COLORREF bckgrnd = clrBack;
  Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &bckgrnd);
  bmkBack = Style_RgbAlpha(bmkBack, bckgrnd, alpha);

  SciCall_MarkerDefine(MARKER_NP3_BOOKMARK, SC_MARK_VERTICALBOOKMARK); // SC_MARK_BOOKMARK/SC_MARK_SHORTARROW
  SciCall_MarkerSetFore(MARKER_NP3_BOOKMARK, bmkFore);
  SciCall_MarkerSetBack(MARKER_NP3_BOOKMARK, bmkBack);
  SciCall_MarkerSetAlpha(MARKER_NP3_BOOKMARK, alpha); // no margin or SC_MARK_BACKGROUND or SC_MARK_UNDERLINE

  // occurrence bookmarker
  bool const visible = Settings.MarkOccurrencesBookmark;
  SciCall_MarkerDefine(MARKER_NP3_OCCURRENCE, visible ? SC_MARK_ARROWS : SC_MARK_BACKGROUND);
  SciCall_MarkerSetFore(MARKER_NP3_OCCURRENCE, CalcContrastColor(clrBack, 100));
  SciCall_MarkerSetAlpha(MARKER_NP3_OCCURRENCE, SC_ALPHA_TRANSPARENT);

  // ---  WordBookMarks  ---
  COLORREF color;
  for (int m = MARKER_NP3_1; m < MARKER_NP3_BOOKMARK; ++m)
  {
    SciCall_MarkerDefine(m, (Settings.FocusViewMarkerMode & FVMM_LN_BACKGR) ? SC_MARK_BACKGROUND : SC_MARK_BOOKMARK);
    Style_StrGetColor(WordBookMarks[m], BACKGROUND_LAYER, &color);
    SciCall_MarkerSetFore(m, color);
    SciCall_MarkerSetBack(m, color);
    SciCall_MarkerSetAlpha(m, alpha); // no margin or SC_MARK_BACKGROUND or SC_MARK_UNDERLINE
  }

  SciCall_SetMarginBackN(MARGIN_SCI_BOOKMRK, clrBack);
  SciCall_SetMarginSensitiveN(MARGIN_SCI_BOOKMRK, true);
  SciCall_SetMarginCursorN(MARGIN_SCI_BOOKMRK, SC_NP3_CURSORHAND);


  // ---  Code folding  ---

  COLORREF fldHiLight = clrFore;
  const WCHAR* wchHighlightStyleStrg = GetCurrentStdLexer()->Styles[STY_SEL_TXT].szValue;
  Style_StrGetColor(wchHighlightStyleStrg, FOREGROUND_LAYER, &fldHiLight);

  SciCall_SetMarginTypeN(MARGIN_SCI_FOLDING, SC_MARGIN_COLOUR);
  SciCall_SetMarginMaskN(MARGIN_SCI_FOLDING, SC_MASK_FOLDERS);
  SciCall_SetMarginBackN(MARGIN_SCI_FOLDING, clrBack);
  SciCall_SetMarginSensitiveN(MARGIN_SCI_FOLDING, true);

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

  static const int FoldMarkerID[] =
  {
    SC_MARKNUM_FOLDEROPEN,
    SC_MARKNUM_FOLDER,
    SC_MARKNUM_FOLDERSUB,
    SC_MARKNUM_FOLDERTAIL,
    SC_MARKNUM_FOLDEREND,
    SC_MARKNUM_FOLDEROPENMID,
    SC_MARKNUM_FOLDERMIDTAIL
  };

  for (int i = 0; i < COUNTOF(FoldMarkerID); ++i) {
    SciCall_MarkerSetFore(FoldMarkerID[i], bmkBack); // (!)
    SciCall_MarkerSetBack(FoldMarkerID[i], bmkFore); // (!)
    SciCall_MarkerSetBackSelected(FoldMarkerID[i], fldHiLight);
  }
  SciCall_MarkerEnableHighlight(true); // highlight folding block

  SciCall_SetFoldMarginColour(true, clrBack);    // background
  SciCall_SetFoldMarginHiColour(true, clrBack);  // (!)
  //SciCall_FoldDisplayTextSetStyle(SC_FOLDDISPLAYTEXT_HIDDEN);

  int fldStyleLn = 0;
  Style_StrGetCharSet(wchBookMarkStyleStrg, &fldStyleLn);

  int const _debug_flags = 0; 
  //int const _debug_flags = (SC_FOLDFLAG_LEVELNUMBERS | SC_FOLDFLAG_LINESTATE); // !extend margin width

  switch (fldStyleLn)
  {
  case 1:
    SciCall_SetFoldFlags(SC_FOLDFLAG_LINEBEFORE_CONTRACTED | _debug_flags);
    break;
  case 2:
    SciCall_SetFoldFlags(SC_FOLDFLAG_LINEBEFORE_CONTRACTED | SC_FOLDFLAG_LINEAFTER_CONTRACTED | _debug_flags);
    break;
  default:
    SciCall_SetFoldFlags(SC_FOLDFLAG_LINEAFTER_CONTRACTED | _debug_flags);
    break;
  }

  // set width
  Style_SetBookmark(hwnd, Settings.ShowBookmarkMargin);
  Style_SetFolding(hwnd, (FocusedView.CodeFoldingAvailable && FocusedView.ShowCodeFolding));
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
  return NULL;
}


//=============================================================================
//
//  Style_MatchLexer()
//
PEDITLEXER Style_MatchLexer(LPCWSTR lpszMatch, bool bCheckNames) 
{
  if (bCheckNames) 
  {
    int const cch = (int)StringCchLenW(lpszMatch, 0);
    if (cch >= 3) {
      for (int iLex = 0; iLex < COUNTOF(g_pLexArray); ++iLex) 
      {
        if (StrCmpNI(g_pLexArray[iLex]->pszName, lpszMatch, cch) == 0) 
        {
          return(g_pLexArray[iLex]);
        }
      }
    }
  }
  else if (StrIsNotEmpty(lpszMatch)) 
  {
    for (int iLex = 0; iLex < COUNTOF(g_pLexArray); ++iLex)
    {
      if (Style_StrGetAttribute(g_pLexArray[iLex]->szExtensions, lpszMatch)) 
      {
        return g_pLexArray[iLex];
      }
    }
  }
  return NULL;
}


//=============================================================================
//
//  Style_RegExMatchLexer()
//
PEDITLEXER Style_RegExMatchLexer(LPCWSTR lpszFileName)
{
  if (StrIsNotEmpty(lpszFileName))
  {
    char chFilePath[MAX_PATH << 1] = { '\0' };
    WideCharToMultiByteEx(CP_UTF8, 0, lpszFileName, -1, chFilePath, COUNTOF(chFilePath), NULL, NULL);

    for (int iLex = 0; iLex < COUNTOF(g_pLexArray); ++iLex)
    {
      const WCHAR* p = g_pLexArray[iLex]->szExtensions;
      do {
        const WCHAR* f = StrChr(p, L'\\');
        const WCHAR* e = f;
        if (f) {
          e = StrChr(f, L';');
          if (!e) {
            e = f + StringCchLen(f, 0);
          }
          ++f; // exclude '\'
          char regexpat[MAX_PATH] = { '\0' };
          WideCharToMultiByteEx(CP_UTF8, 0, f, (int)(e-f), regexpat, COUNTOF(regexpat), NULL, NULL);

          if (OnigRegExFind(regexpat, chFilePath, false, SciCall_GetEOLMode()) >= 0) {
            return g_pLexArray[iLex];
          }
        }
        p = e;
      } while (p != NULL);
    }
  }
  return NULL;
}
  

//=============================================================================
//
//  Style_HasLexerForExt()
//
bool Style_HasLexerForExt(LPCWSTR lpszFile)
{
  bool bFound = false;
  LPCWSTR lpszExt = PathFindExtension(lpszFile);
  if (StrIsNotEmpty(lpszExt)) {
    if (*lpszExt == L'.') ++lpszExt;
    if (lpszExt && Style_MatchLexer(lpszExt, false))
    {
      bFound = true;
    }
  }
  if (!bFound && StrIsNotEmpty(lpszFile)) 
  {
    bFound = Style_RegExMatchLexer(PathFindFileName(lpszFile));
  }
  return bFound;
}


//=============================================================================
//
//  Style_SetLexerFromFile()
//
bool Style_SetLexerFromFile(HWND hwnd,LPCWSTR lpszFile)
{
  if (Flags.bLargeFileLoaded) {
    Style_SetDefaultLexer(hwnd);
    return true;
  }

  LPCWSTR lpszExt = PathFindExtension(lpszFile);
  bool  bFound = false;
  PEDITLEXER pLexNew = NULL;
  PEDITLEXER pLexSniffed = NULL;

  if ((Globals.fvCurFile.mask & FV_MODE) && Globals.fvCurFile.chMode[0]) {

    PEDITLEXER pLexMode;
    WCHAR wchMode[MICRO_BUFFER] = { L'\0' };
    MultiByteToWideCharEx(Encoding_SciCP, 0, Globals.fvCurFile.chMode, -1, wchMode, MICRO_BUFFER);

    if (!Flags.NoCGIGuess && (StringCchCompareNI(wchMode,COUNTOF(wchMode),L"cgi", CSTRLEN(L"cgi")) == 0 ||
                              StringCchCompareNI(wchMode,COUNTOF(wchMode),L"fcgi", CSTRLEN(L"fcgi")) == 0)) {
      char tchText[256] = { '\0' };
      SciCall_GetText(COUNTOF(tchText), tchText);
      StrTrimA(tchText," \t\n\r");
      pLexSniffed = Style_SniffShebang(tchText);
      if (pLexSniffed) {
        if ((Encoding_GetCurrent() != Globals.DOSEncoding) || !IsLexerStandard(pLexSniffed) || (
          (StringCchCompareXI(lpszExt,L"nfo") == 0) && (StringCchCompareXI(lpszExt,L"diz") == 0))) {
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

  LPCWSTR lpszFileName = PathFindFileName(lpszFile);

  // check for filename regex match
  if (!bFound && s_bAutoSelect && StrIsNotEmpty(lpszFile)) {
    pLexSniffed = Style_RegExMatchLexer(lpszFileName);
    if (pLexSniffed) {
      pLexNew = pLexSniffed;
      bFound = true;
    }
  }

  if (!bFound && s_bAutoSelect && (StrIsNotEmpty(lpszFile) && *lpszExt)) 
  {
    if (*lpszExt == L'.') { ++lpszExt; }

    if (!Flags.NoCGIGuess && (StringCchCompareXI(lpszExt,L"cgi") == 0 || StringCchCompareXI(lpszExt,L"fcgi") == 0)) {
      char tchText[256] = { '\0' };
      SciCall_GetText(COUNTOF(tchText), tchText);
      StrTrimA(tchText," \t\n\r");
      pLexSniffed = Style_SniffShebang(tchText);
      if (pLexSniffed) {
        pLexNew = pLexSniffed;
        bFound = true;
      }
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

  if (!bFound && s_bAutoSelect && (!Flags.NoHTMLGuess || !Flags.NoCGIGuess)) {
    char tchText[512] = { '\0' };
    SciCall_GetText(COUNTOF(tchText), tchText);
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

  if (!bFound && (Encoding_GetCurrent() == Globals.DOSEncoding)) {
    pLexNew = &lexANSI;
  }
  
  // Apply the new lexer
  if (IsLexerStandard(pLexNew)) 
  {
    Style_SetDefaultLexer(hwnd);
  }
  else {
    Style_SetLexer(hwnd, pLexNew);
  }
  return bFound;
}


//=============================================================================
//
//  Style_MaybeBinaryFile()
//
bool Style_MaybeBinaryFile(HWND hwnd, LPCWSTR lpszFile) 
{
  UNUSED(hwnd);
#if 0
  UNUSED(lpszFile);
#else
  unsigned char buf[5] = { '\0' }; // magic
  SciCall_GetText(COUNTOF(buf), (char*)buf);
  UINT const magic2 = (buf[0] << 8) | buf[1];
  if (magic2 == 0x4D5AU ||  // PE: MZ
    magic2 == 0x504BU ||    // ZIP: PK
    magic2 == 0x377AU ||    // 7z: 7z
    magic2 == 0x424DU ||    // BMP: BM
    magic2 == 0xFFD8U       // JPEG
    ) {
    return true;
  }
  UINT const magic = (magic2 << 16) | (buf[2] << 8) | buf[3];
  if (magic == 0x0000FEFFU ||  // UTF32-BE
    magic == 0xFFFE0000U ||    // UTF32-LE
    magic == 0x89504E47U ||    // PNG: 0x89+PNG
    magic == 0x47494638U ||    // GIF: GIF89a
    magic == 0x25504446U ||    // PDF: %PDF-{version}
    magic == 0x52617221U ||    // RAR: Rar!
    magic == 0x7F454C46U ||    // ELF: 0x7F+ELF
    magic == 0x213C6172U ||    // .lib, .a: !<arch>\n
    magic == 0xFD377A58U ||    // xz: 0xFD+7zXZ
    magic == 0xCAFEBABEU       // Java class
    ) {
    return true;
  }
  const WCHAR* const binaryExt = L"|bin|exe|cur|ico|iso|img|lib|mdb|obj|pak|pdb|pyc|pyd|tar|"; // keep '|' at end
  size_t const _min = 5ULL;  size_t const _max = 6ULL;

  WCHAR lpszExt[16] = { L'\0' };
  StringCchCopy(lpszExt, COUNTOF(lpszExt), L"|");
  StringCchCopy(lpszExt, COUNTOF(lpszExt), PathFindExtension(lpszFile));
  StringCchCat(lpszExt, COUNTOF(lpszExt), L"|");
  size_t const len = StringCchLen(lpszExt,MAX_PATH);
  if (len < _min || len > _max) {
    if (StrStrI(binaryExt, lpszExt)) { return true; }
  }
#endif
  return false;
}


//=============================================================================
//
//  Style_SetLexerFromName()
//
void Style_SetLexerFromName(HWND hwnd, LPCWSTR lpszFile, LPCWSTR lpszName)
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
                       IsStyleStandardDefault(pLexerDefStyle), IsStyleSchemeDefault(pLexerDefStyle), true, true))
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
      case SCLEX_NIM:
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
  if (StrIsEmpty(Settings2.FileDlgFilters)) {
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
//  Style_StrGetFontName()
//
bool Style_StrGetFontName(LPCWSTR lpszStyle, LPWSTR lpszFont, int cchFont)
{
  WCHAR *p = StrStr(lpszStyle, L"font:");
  if (p) {
    p += CSTRLEN(L"font:");
    while (*p == L' ') { ++p; }
    StringCchCopyN(lpszFont, cchFont, p, cchFont);
    if ((p = StrChr(lpszFont, L';')) != NULL) {
      *p = L'\0';
    }
    TrimSpcW(lpszFont);
    if (StringCchCompareXI(lpszFont, L"Default") == 0) {
      StringCchCopyN(lpszFont, cchFont, IsFontAvailable(L"Consolas") ? L"Consolas" : L"Lucida Console", cchFont);
    }
    return true;
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetFontStyle()
//
bool Style_StrGetFontStyle(LPCWSTR lpszStyle, LPWSTR lpszFontStyle, int cchFontStyle)
{
  WCHAR* p = StrStr(lpszStyle, L"fstyle:");
  if (p) {
    p += CSTRLEN(L"fstyle:");
    while (*p == L' ') { ++p; }
    StringCchCopyN(lpszFontStyle, cchFontStyle, p, cchFontStyle);
    if ((p = StrChr(lpszFontStyle, L';')) != NULL) {
      *p = L'\0';
    }
    TrimSpcW(lpszFontStyle);
    return true;
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetFontQuality()
//
bool Style_StrGetFontQuality(LPCWSTR lpszStyle, LPWSTR lpszQuality, int cchQuality)
{
  WCHAR *p = StrStr(lpszStyle, L"smoothing:");
  if (p) {
    p += CSTRLEN(L"smoothing:");
    while (*p == L' ') { ++p; }
    StringCchCopyN(lpszQuality, cchQuality, p, cchQuality);
    if ((p = StrChr(lpszQuality, L';')) != NULL) {
      *p = L'\0';
    }
    TrimSpcW(lpszQuality);

    if (StringCchCompareX(lpszQuality, L"none") == 0 ||
        StringCchCompareX(lpszQuality, L"standard") == 0 ||
        StringCchCompareX(lpszQuality, L"cleartype") == 0 ||
        StringCchCompareX(lpszQuality, L"default") == 0)
    {
      return true;
    }
  }
  StringCchCopy(lpszQuality, cchQuality, L"");
  return false;
}


//=============================================================================
//
//  Style_StrGetCharSet()
//
bool Style_StrGetCharSet(LPCWSTR lpszStyle, int* i)
{
  WCHAR *p = StrStr(lpszStyle, L"charset:");
  if (p) {
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
  WCHAR *p = StrStr(lpszStyle, L"size:");
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
  WCHAR *p = StrStr(lpszStyle, L"size:");
  if (p) {
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
    TrimSpcW(tch);
    
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
  WCHAR *p = StrStr(lpszStyle, L"size:");
  if (p)
  {
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch, COUNTOF(tch), (p + CSTRLEN(L"size:")));
    p = StrChr(tch, L';');
    if (p) { *p = L'\0'; }
    TrimSpcW(tch);

    float fValue = 0.0f;
    if (Char2FloatW(tch, &fValue)) {
      WCHAR wchFloatVal[64];
      fValue = (float)fabs(fValue);
      bool const isZero = (fValue == 0.0f);
      Float2String(fValue, wchFloatVal, COUNTOF(wchFloatVal));

      if (tch[0] == L'+') {
        if (!isZero) {
          StringCchPrintf(lpszSize, cchSize, L"+%s", wchFloatVal);
        }
        else {
          return false;
        }
      }
      else if (tch[0] == L'-') {
        if (!isZero) {
          StringCchPrintf(lpszSize, cchSize, L"-%s", wchFloatVal);
        }
        else {
          return false;
        }
      }
      else {
        StringCchPrintf(lpszSize, cchSize, L"%s", wchFloatVal);
      }
      return true;
    }
  }
  return false;
}


//=============================================================================
//
//  Style_StrGetWeightValue()
//
bool Style_StrGetWeightValue(LPCWSTR lpszWeight, int* weight)
{
  int iFontWeight = -1;
  
  if (Style_StrGetAttrThin(lpszWeight))
    iFontWeight = FW_THIN;
  else if (Style_StrGetAttrExtraLight(lpszWeight))
    iFontWeight = FW_EXTRALIGHT;
  else if (Style_StrGetAttrLight(lpszWeight))
    iFontWeight = FW_LIGHT;
  else if (Style_StrGetAttrNormal(lpszWeight))
    iFontWeight = FW_NORMAL;
  else if (Style_StrGetAttrMedium(lpszWeight))
    iFontWeight = FW_MEDIUM;
  else if (Style_StrGetAttrSemiBold(lpszWeight))
    iFontWeight = FW_SEMIBOLD;
  else if (Style_StrGetAttrBold(lpszWeight))
    iFontWeight = FW_BOLD;
  else if (Style_StrGetAttrExtraBold(lpszWeight))
    iFontWeight = FW_EXTRABOLD;
  else if (Style_StrGetAttrHeavy(lpszWeight))
    iFontWeight = FW_HEAVY;

  if (iFontWeight >= 0) {
    *weight = iFontWeight;
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
    //~StringCchCat(lpszWeight, cchSize, L"; normal");
    //StringCchCat(lpszWeight, cchSize, L""); // std
  }
  else if (fontWeight <= FW_MEDIUM) {
    //~StringCchCat(lpszWeight, cchSize, L"; medium");
    //StringCchCat(lpszWeight, cchSize, L""); // std
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

#if 0
//=============================================================================
//
//  Style_StrGetStretchValue()
//
bool Style_StrGetStretchValue(LPCWSTR lpszStretch, int* stretch)
{
  int iFontStretch = FONT_STRETCH_UNDEFINED;

  if (Style_StrGetAttrUltraCondensed(lpszStretch))
    iFontStretch = FONT_STRETCH_ULTRA_CONDENSED;
  else if (Style_StrGetAttrExtraCondensed(lpszStretch))
    iFontStretch = FONT_STRETCH_EXTRA_CONDENSED;
  else if (Style_StrGetAttrCondensed(lpszStretch))
    iFontStretch = FONT_STRETCH_CONDENSED;
  else if (Style_StrGetAttrSemiCondensed(lpszStretch))
    iFontStretch = FONT_STRETCH_SEMI_CONDENSED;
  else if (Style_StrGetAttrNormalStretch(lpszStretch))
    iFontStretch = FONT_STRETCH_NORMAL;
  else if (Style_StrGetAttrMediumStretch(lpszStretch))
    iFontStretch = FONT_STRETCH_MEDIUM;
  else if (Style_StrGetAttrSemiExpanded(lpszStretch))
    iFontStretch = FONT_STRETCH_SEMI_EXPANDED;
  else if (Style_StrGetAttrExpanded(lpszStretch))
    iFontStretch = FONT_STRETCH_EXPANDED;
  else if (Style_StrGetAttrExtraExpanded(lpszStretch))
    iFontStretch = FONT_STRETCH_EXTRA_EXPANDED;
  else if (Style_StrGetAttrUltraExpanded(lpszStretch))
    iFontStretch = FONT_STRETCH_ULTRA_EXPANDED;

  if (iFontStretch > FONT_STRETCH_UNDEFINED) {
    *stretch = iFontStretch;
  }
  return ((iFontStretch <= FONT_STRETCH_UNDEFINED) ? false : true);
}

//=============================================================================
//
//  Style_AppendStretchStr()
//
void Style_AppendStretchStr(LPWSTR lpszStretch, int cchSize, int fontStretch)
{
  if (fontStretch <= FONT_STRETCH_ULTRA_CONDENSED) {
    StringCchCat(lpszStretch, cchSize, L"; ultracondensed");
  }
  else if (fontStretch <= FONT_STRETCH_EXTRA_CONDENSED) {
    StringCchCat(lpszStretch, cchSize, L"; extracondensed");
  }
  else if (fontStretch <= FONT_STRETCH_CONDENSED) {
    StringCchCat(lpszStretch, cchSize, L"; condensed");
  }
  else if (fontStretch <= FONT_STRETCH_SEMI_CONDENSED) {
    StringCchCat(lpszStretch, cchSize, L"; semicondensed");
  }
  else if (fontStretch <= FONT_STRETCH_NORMAL) {
    //~StringCchCat(lpszStretch, cchSize, L"; normalstretch");
    //StringCchCat(lpszStretch, cchSize, L""); // std
  }
  else if (fontStretch <= FONT_STRETCH_MEDIUM) {
    //~StringCchCat(lpszStretch, cchSize, L"; mediumstretch");
    //StringCchCat(lpszStretch, cchSize, L""); // std
  }
  else if (fontStretch <= FONT_STRETCH_SEMI_EXPANDED) {
    StringCchCat(lpszStretch, cchSize, L"; semiexpanded");
  }
  else if (fontStretch <= FONT_STRETCH_EXPANDED) {
    StringCchCat(lpszStretch, cchSize, L"; expanded");
  }
  else if (fontStretch <= FONT_STRETCH_EXTRA_EXPANDED) {
    StringCchCat(lpszStretch, cchSize, L"; extraexpanded");
  }
  else { // (fontStretch >= FONT_STRETCH_ULTRA_EXPANDED)
    StringCchCat(lpszStretch, cchSize, L"; ultraexpanded");
  }
}
#endif

//=============================================================================
//
//  Style_StrGetColor()
//
bool Style_StrGetColor(LPCWSTR lpszStyle, COLOR_LAYER layer, COLORREF* rgb)
{
  const WCHAR* const pItem = (layer == FOREGROUND_LAYER) ? L"fore:" : L"back:";

  WCHAR *p = StrStr(lpszStyle, pItem);
  if (p) {
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch, COUNTOF(tch), p + StringCchLenW(pItem,0));
    if (tch[0] == L'#')
      tch[0] = L' ';
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimSpcW(tch);
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
bool Style_StrGetAlpha(LPCWSTR lpszStyle, int* iOutValue, bool bAlpha1st) 
{
  const WCHAR* strAlpha = bAlpha1st ? L"alpha:" : L"alpha2:";

  WCHAR* p = StrStr(lpszStyle, strAlpha);
  if (p) {
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    StringCchCopy(tch, COUNTOF(tch), p + StringCchLenW(strAlpha,0));
    p = StrChr(tch, L';');
    if (p)
      *p = L'\0';
    TrimSpcW(tch);
    int iValue = 0;
    if (StrToIntEx(tch, STIF_DEFAULT, &iValue)) {
      *iOutValue = Sci_ClampAlpha(iValue);
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
//  WCHAR *p = StrStr(lpszStyle, lpszProperty);
//  if (p) {
//    StringCchCopy(tch, COUNTOF(tch), (p + StringCchLenW(lpszProperty,0)));
//    p = StrChr(tch, L';');
//    if (p)
//      *p = L'\0';
//    TrimStringW(tch);
//    //if (1 == swscanf_s(tch, L"%i", val)) { return true; }
//    if (StrToIntEx(tch, STIF_DEFAULT, val)) { return true; }
//  }
//  return false;
//}


//=============================================================================
//
//  Style_StrGetCase()
//
bool Style_StrGetCase(LPCWSTR lpszStyle, int* i)
{
  WCHAR *p = StrStr(lpszStyle, L"case:");
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

static WCHAR* IndicatorTypes[] = {
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
      if (Style_StrGetAttribute(lpszStyle, IndicatorTypes[i])) {
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
void Style_CopyStyles_IfNotDefined(LPCWSTR lpszStyleSrc, LPWSTR lpszStyleDest, int cchSizeDest, bool bCopyFont)
{
  int  iValue;
  COLORREF dColor;
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
  WCHAR szTmpStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

  // ---------   Font settings   ---------
  if (bCopyFont)
  {
    if (!StrStr(lpszStyleDest, L"font:")) {
      if (Style_StrGetFontName(lpszStyleSrc, tch, COUNTOF(tch))) {
        WCHAR wchDefFontName[BUFSIZE_STYLE_VALUE] = { L'\0' };
        Style_StrGetFontName(L"font:Default", wchDefFontName, COUNTOF(wchDefFontName));
        if ((StringCchCompareXI(tch, L"Default") == 0) || (StringCchCompareXI(tch, wchDefFontName) == 0)) {
          StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; font:Default");
        }
        else {
          StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; font:");
          StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
        }
      }
    }

    // ---------  Font Style  ---------
    if (!StrStr(lpszStyleDest, L"fstyle:")) {
      if (Style_StrGetFontStyle(lpszStyleSrc, tch, COUNTOF(tch))) {
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; fstyle:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
      }
    }

    // ---------  Size  ---------
    if (!StrStr(lpszStyleDest, L"size:")) {
      if (Style_StrGetSizeStr(lpszStyleSrc, tch, COUNTOF(tch))) {
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; size:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
      }
    }

    if (Style_StrGetAttrThin(lpszStyleSrc) && !Style_StrGetAttrThin(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; thin");
    else if (Style_StrGetAttrExtraLight(lpszStyleSrc) && !Style_StrGetAttrExtraLight(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; extralight");
    else if (Style_StrGetAttrLight(lpszStyleSrc) && !Style_StrGetAttrLight(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; light");
    else if (Style_StrGetAttrNormal(lpszStyleSrc) && !Style_StrGetAttrNormal(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; normal");
    else if (Style_StrGetAttrMedium(lpszStyleSrc) && !Style_StrGetAttrMedium(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; medium");
    else if (Style_StrGetAttrSemiBold(lpszStyleSrc) && !Style_StrGetAttrSemiBold(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; semibold");
    else if (Style_StrGetAttrBold(lpszStyleSrc) && !Style_StrGetAttrBold(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; bold");
    else if (Style_StrGetAttrExtraBold(lpszStyleSrc) && !Style_StrGetAttrExtraBold(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; extrabold");
    else if (Style_StrGetAttrHeavy(lpszStyleSrc) && !Style_StrGetAttrHeavy(lpszStyleDest))
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; heavy");
    //else
    //  StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; regular");  // -> default

    if (Style_StrGetAttrItalic(lpszStyleSrc) && !Style_StrGetAttrItalic(lpszStyleDest)) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; italic");
    }

    // ---------  Effects  ---------
    if (Style_StrGetAttrUnderline(lpszStyleSrc) && !Style_StrGetAttrUnderline(lpszStyleDest)) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; underline");
    }

    if (Style_StrGetAttrStrikeOut(lpszStyleSrc) && !Style_StrGetAttrStrikeOut(lpszStyleDest)) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; strikeout");
    }

    if (StrStr(lpszStyleSrc, L"fore:") && !StrStr(lpszStyleDest, L"fore:")) { // foreground
      if (Style_StrGetColor(lpszStyleSrc, FOREGROUND_LAYER, &dColor)) {
        StringCchPrintf(tch, COUNTOF(tch), L"; fore:#%02X%02X%02X",
          (int)GetRValue(dColor), (int)GetGValue(dColor), (int)GetBValue(dColor));
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
      }
    }

    if (StrStr(lpszStyleSrc, L"back:") && !StrStr(lpszStyleDest, L"back:")) { // background
      if (Style_StrGetColor(lpszStyleSrc, BACKGROUND_LAYER, &dColor)) {
        StringCchPrintf(tch, COUNTOF(tch), L"; back:#%02X%02X%02X",
          (int)GetRValue(dColor), (int)GetGValue(dColor), (int)GetBValue(dColor));
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
      }
    }

  }

#if 0
  // not set by ChooseFont() dialog
  if (Style_StrGetAttrUltraCondensed(lpszStyleSrc) && !Style_StrGetAttrUltraCondensed(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; ultracondensed");
  else if (Style_StrGetAttrExtraCondensed(lpszStyleSrc) && !Style_StrGetAttrExtraCondensed(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; extracondensed");
  else if (Style_StrGetAttrCondensed(lpszStyleSrc) && !Style_StrGetAttrCondensed(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; condensed");
  else if (Style_StrGetAttrSemiCondensed(lpszStyleSrc) && !Style_StrGetAttrSemiCondensed(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; semicondensed");
  else if (Style_StrGetAttrNormalStretch(lpszStyleSrc) && !Style_StrGetAttrNormalStretch(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; normalstretch");
  else if (Style_StrGetAttrMediumStretch(lpszStyleSrc) && !Style_StrGetAttrMediumStretch(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; mediumstretch");
  else if (Style_StrGetAttrSemiExpanded(lpszStyleSrc) && !Style_StrGetAttrSemiExpanded(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; semiexpanded");
  else if (Style_StrGetAttrExpanded(lpszStyleSrc) && !Style_StrGetAttrExpanded(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; expanded");
  else if (Style_StrGetAttrExtraExpanded(lpszStyleSrc) && !Style_StrGetAttrExtraExpanded(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; extraexpanded");
  else if (Style_StrGetAttrUltraExpanded(lpszStyleSrc) && !Style_StrGetAttrUltraExpanded(lpszStyleDest))
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; ultraexpanded");
#endif

  // ---------  Special Styles  ---------

  if (!StrStr(lpszStyleDest, L"charset:")) {
    if (Style_StrGetCharSet(lpszStyleSrc, &iValue)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; charset:%i", iValue);
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }

  if (!StrStr(lpszStyleDest, L"smoothing:")) {
    if (Style_StrGetFontQuality(lpszStyleSrc, tch, COUNTOF(tch))) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; smoothing:");
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }

  if (Style_StrGetAttrEOLFilled(lpszStyleSrc) && !Style_StrGetAttrEOLFilled(lpszStyleDest)) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; eolfilled");
  }

  if (Style_StrGetCase(lpszStyleSrc, &iValue) && !StrStr(lpszStyleDest, L"case:")) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; case:");
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), (iValue == SC_CASE_UPPER) ? L"U" : L"L");
  }

  if (!StrStr(lpszStyleDest, L"alpha:")) {
    if (Style_StrGetAlpha(lpszStyleSrc, &iValue, true)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; alpha:%i", iValue);
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }
  if (!StrStr(lpszStyleDest, L"alpha2:")) {
    if (Style_StrGetAlpha(lpszStyleSrc, &iValue, false)) {
      StringCchPrintf(tch, COUNTOF(tch), L"; alpha2:%i", iValue);
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }

  //const WCHAR* wchProperty = L"property:";
  //if (!StrStr(lpszStyleDest, wchProperty)) {
  //  if (Style_StrGetPropertyValue(lpszStyleSrc, wchProperty, &iValue)) {
  //    StringCchPrintf(tch, COUNTOF(tch), L"; %s%i", wchProperty, iValue);
  //    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
  //  }
  //}

  // --------   indicator type   --------
  if (!StrStr(lpszStyleDest, L"indic_")) {
    iValue = -1;
    StringCchCopy(tch, COUNTOF(tch), lpszStyleSrc);
    if (Style_GetIndicatorType(tch, 0, &iValue)) {
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; ");
      Style_GetIndicatorType(tch, COUNTOF(tch), &iValue);
      StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
  }

  // --------   other style settings   --------
  if (Style_StrGetAttrOvrBlck(lpszStyleSrc) && !Style_StrGetAttrOvrBlck(lpszStyleDest)) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; ovrblck");
  }

  if (Style_StrGetAttrBlock(lpszStyleSrc) && !Style_StrGetAttrBlock(lpszStyleDest)) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; block");
  }

  if (Style_StrGetAttrNoBlink(lpszStyleSrc) && !Style_StrGetAttrNoBlink(lpszStyleDest)) {
    StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), L"; noblink");
  }

  StrTrim(szTmpStyle, L" ;");
  StringCchCat(lpszStyleDest, cchSizeDest, szTmpStyle);
}

//=============================================================================

/// Callback to set the font dialog's title
static  WCHAR FontSelTitle[128];

static INT_PTR CALLBACK Style_FontDialogHook(
  HWND hdlg,      // handle to the dialog box window
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
)
{
  UNUSED(wParam);
  switch (uiMsg)
  {
    case WM_INITDIALOG:
    {
      if (Globals.hDlgIconSmall) { SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM)Globals.hDlgIconSmall); }

      InitWindowCommon(hdlg, true);

#ifdef D_NP3_WIN10_DARK_MODE

      #define IDCS_STRKOUT 0x410
      #define IDCS_UNDERLN 0x411
      #define IDCS_EFFECTS 0x430
      #define IDCS_SAMPLE  0x431
      #define IDCS_EXMPBOX 0x444
      #define IDCS_EXMPTXT 0x445

      if (UseDarkMode()) {
        SetExplorerTheme(GetDlgItem(hdlg, IDOK));
        SetExplorerTheme(GetDlgItem(hdlg, IDCANCEL));
        //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
        int const ctl[] = { IDCS_STRKOUT, IDCS_UNDERLN, IDCS_EFFECTS, IDCS_SAMPLE, IDCS_EXMPBOX, IDCS_EXMPTXT, IDC_STATIC };
        for (int i = 0; i < COUNTOF(ctl); ++i) {
          SetWindowTheme(GetDlgItem(hdlg, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
        }
      }
#endif

      const CHOOSEFONT* const pChooseFont = ((CHOOSEFONT*)lParam);

      if (pChooseFont->lCustData) {
        SetWindowText(hdlg, (WCHAR*)pChooseFont->lCustData);
      }
      //~else {
      //~  // HACK: to get the full font name instead of font family name
      //~  // [see: ChooseFontDirectWrite() PostProcessing]
      //~  SendMessage(hdlg, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)pChooseFont->lpLogFont);
      //~  PostMessage(hdlg, WM_CLOSE, 0, 0);
      //~}
    }
    break;

#ifdef D_NP3_WIN10_DARK_MODE

    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC:
      if (UseDarkMode()) {
        return SetDarkModeCtlColors((HDC)wParam);
      }
      break;

    case WM_SETTINGCHANGE:
      if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
        SendMessage(hdlg, WM_THEMECHANGED, 0, 0);
      }
      break;

    case WM_THEMECHANGED:
      if (IsDarkModeSupported()) {
        bool const darkModeEnabled = CheckDarkModeEnabled();
        AllowDarkModeForWindow(hdlg, darkModeEnabled);
        RefreshTitleBarThemeColor(hdlg);

        int const buttons[] = { IDOK, IDCANCEL };
        for (int id = 0; id < COUNTOF(buttons); ++id) {
          HWND const hBtn = GetDlgItem(hdlg, buttons[id]);
          AllowDarkModeForWindow(hBtn, darkModeEnabled);
          SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
        }
        UpdateWindow(hdlg);
      }
      break;

#endif

    default:
      break;
  }
  return FALSE;	// Allow the default handler a chance to process
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

  WCHAR wchDefaultFontName[LF_FULLFACESIZE] = { L'\0' };
  Style_StrGetFontName(L"font:Default", wchDefaultFontName, COUNTOF(wchDefaultFontName));

  WCHAR wchFontName[LF_FULLFACESIZE] = { L'\0' };
  if (!Style_StrGetFontName(lpszStyle, wchFontName, COUNTOF(wchFontName))) 
  {
    if (!Style_StrGetFontName(GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue, wchFontName, COUNTOF(wchFontName)))
    {
      Style_StrGetFontName(L"font:Default", wchFontName, COUNTOF(wchFontName));
    }
  }

  // font style 
  WCHAR szStyleStrg[LF_FULLFACESIZE] = { L'\0' };
  Style_StrGetFontStyle(lpszStyle, szStyleStrg, COUNTOF(szStyleStrg));

  int iCharSet = SC_CHARSET_DEFAULT;
  Style_StrGetCharSet(lpszStyle, &iCharSet);
    
  // is "size:" definition relative ?
  bool const bRelFontSize = (!StrStr(lpszStyle, L"size:") || StrStr(lpszStyle, L"size:+") || StrStr(lpszStyle, L"size:-"));

  float const fBaseFontSize = (bGlobalDefaultStyle ? (float)Globals.InitialFontSize : 
                                                     (bCurrentDefaultStyle ? Style_GetBaseFontSize() : Style_GetCurrentFontSize()));

  // Font Height
  float fFontSize;
  if (!Style_StrGetSize(lpszStyle, &fFontSize)) {
    fFontSize = fBaseFontSize;
  }
  HDC const hdc = GetDC(hwnd);
  int const iFontHeight = PointSizeToFontHeight(fFontSize, hdc);
  ReleaseDC(hwnd, hdc);

  // Font Weight
  int  iFontWeight;
  if (!Style_StrGetWeightValue(lpszStyle, &iFontWeight)) {
    iFontWeight = FW_NORMAL;
  }

#if 0
  int iFontStretch = FONT_STRETCH_NORMAL;
  if (!Style_StrGetStretchValue(lpszStyle, &iFontWeight)) {
    iFontStretch = FONT_STRETCH_NORMAL;
  }
#endif

  bool bIsItalic = Style_StrGetAttrItalic(lpszStyle);
  bool bIsUnderline = Style_StrGetAttrUnderline(lpszStyle);
  bool bIsStrikeout = Style_StrGetAttrStrikeOut(lpszStyle);

  int iQuality = Settings2.SciFontQuality;
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
  StringCchCopyN(lf.lfFaceName, LF_FACESIZE, wchFontName, COUNTOF(wchFontName));
  lf.lfCharSet = (BYTE)iCharSet;
  lf.lfHeight = iFontHeight;
  lf.lfWidth = 0; // iFontStretch
  lf.lfWeight = iFontWeight;
  lf.lfItalic = (BYTE)(BOOL)bIsItalic;
  lf.lfUnderline = (BYTE)(BOOL)bIsUnderline;
  lf.lfStrikeOut = (BYTE)(BOOL)bIsStrikeout;
  lf.lfQuality = (BYTE)iQuality;
  //~lf.lfClipPrecision = (BYTE)CLIP_DEFAULT_PRECIS;
  //~lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;

  COLORREF color = 0L;
  Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &color);

  // Init cf
  CHOOSEFONT cf;
  ZeroMemory(&cf, sizeof(CHOOSEFONT));
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hwndOwner = hwnd;
  cf.hInstance = Globals.hInstance; // ChooseFontDirectWrite
  cf.rgbColors = color;
  cf.lpLogFont = &lf;
  cf.iPointSize = (INT)float2int(fFontSize * 10.0f);
  cf.nFontType = SCREEN_FONTTYPE;
  cf.lpszStyle = szStyleStrg;

  cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_USESTYLE | CF_SCREENFONTS | CF_ENABLEHOOK;  //~ CF_NOSCRIPTSEL | CF_SCALABLEONLY | CF_FORCEFONTEXIST

  // CF_LIMITSIZE
  //cf.nSizeMin = 4;
  //cf.nSizeMax = 128;

  // custom hook for title bar
  cf.lpfnHook = (LPCFHOOKPROC)Style_FontDialogHook;	// Register the callback
  cf.lCustData = (LPARAM)FontSelTitle;

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

  if (bWithEffects) {
    cf.Flags |= CF_EFFECTS;
  }

  if (IsKeyDown(VK_SHIFT)) {
    cf.Flags |= CF_FIXEDPITCHONLY;
  }

  // ---  open systems Font Selection dialog  ---
#if 1
  if (!ChooseFont(&cf) || StrIsEmpty(lf.lfFaceName)) { return false; }
#else 
  if (Settings.RenderingTechnology > 0) {
    DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);
    const WCHAR* const localName = Settings2.PreferredLanguageLocaleName;
    if (!ChooseFontDirectWrite(Globals.hwndMain, localName, dpi, &cf) || StrIsEmpty(lf.lfFaceName)) { return false; }
    // HACK: to get the full font name instead of font family name
    // [see: Style_FontDialogHook() WM_INITDIALOG]
    cf.lCustData = (LPARAM)NULL;
    ChooseFont(&cf);
  }
  else {
    if (!ChooseFont(&cf) || StrIsEmpty(lf.lfFaceName)) { return false; }
  }
#endif

  // ---  map back to lpszStyle  ---

  WCHAR szNewStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

  if (StrStr(lpszStyle, L"font:")) {
    StringCchCopy(szNewStyle, COUNTOF(szNewStyle), L"font:");
    if (StringCchCompareX(lf.lfFaceName, wchDefaultFontName) == 0) {
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"Default");
    }
    else {
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), lf.lfFaceName);
    }
  }
  else // no font in source specified, 
  {
    if (lstrcmpW(lf.lfFaceName, wchFontName) != 0) {
      StringCchCopy(szNewStyle, COUNTOF(szNewStyle), L"font:");
      if (StringCchCompareX(lf.lfFaceName, wchDefaultFontName) == 0) {
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"Default");
      }
      else {
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), lf.lfFaceName);
      }
    }
  }

  if (StrIsNotEmpty(cf.lpszStyle))
  {
    if (StrStr(lpszStyle, L"fstyle:")) {
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; fstyle:");
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), cf.lpszStyle);
    }
    else  // no font style in source specified, 
    {
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; fstyle:");
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), cf.lpszStyle);
    }
  }

  if (lf.lfWeight == iFontWeight) {
    WCHAR check[64] = { L'\0' };
    Style_AppendWeightStr(check, COUNTOF(check), lf.lfWeight);
    StrTrimW(check, L" ;");
    if (Style_StrGetAttribute(lpszStyle, check)) {
      Style_AppendWeightStr(szNewStyle, COUNTOF(szNewStyle), lf.lfWeight);
    }
  }
  else {
    Style_AppendWeightStr(szNewStyle, COUNTOF(szNewStyle), lf.lfWeight);
  }

#if 0
  // font stretch 
  if (lf.lfWidth == 0) {
    WCHAR check[64] = { L'\0' };
    Style_AppendStretchStr(check, COUNTOF(check), /*lf.lfWidth*/ iFontStretch);
    StrTrimW(check, L" ;");
    if (Style_StrGetAttribute(lpszStyle, check)) {
      Style_AppendStretchStr(szNewStyle, COUNTOF(szNewStyle), /*lf.lfWidth*/ iFontStretch);
    }
  }
  else {
    Style_AppendStretchStr(szNewStyle, COUNTOF(szNewStyle), /*lf.lfWidth*/ FONT_STRETCH_NORMAL);
  }
#endif

  float fNewFontSize = (float)(cf.iPointSize) / 10.0f;
  WCHAR newSize[64] = { L'\0' };

  if (bRelFontSize)
  {
    float fNewRelSize = Round10th(fNewFontSize - fBaseFontSize);

    if (fNewRelSize >= 0.0) {
      if (HasNonZeroFraction(fNewRelSize)) {
        StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:+%.3G", fNewRelSize);
      }
      else {
        int const iRelSize = float2int(fNewRelSize);
        if (iRelSize) {
          StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:+%i", iRelSize);
        }
      }
    }
    else {
      if (HasNonZeroFraction(fNewRelSize)) {
        StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:-%.3G", (0.0f - fNewRelSize));
      }
      else {
        int const iRelSize = float2int(0.0f - fNewRelSize);
        if (iRelSize) {
          StringCchPrintfW(newSize, COUNTOF(newSize), L"; size:-%i", iRelSize);
        }
      }
    }
  }
  else {
    fFontSize = Round10th(fFontSize);
    fNewFontSize = Round10th(fNewFontSize);

    if (fNewFontSize == fFontSize) {
      if (StrStr(lpszStyle, L"size:")) {
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

  
  if (bGlobalDefaultStyle && (lf.lfCharSet != DEFAULT_CHARSET) && (lf.lfCharSet != ANSI_CHARSET))
  {
    WCHAR chset[32] = { L'\0' };
    if (lf.lfCharSet == iCharSet) {
      if (StrStr(lpszStyle, L"charset:"))
      {
        StringCchPrintf(chset, COUNTOF(chset), L"; charset:%i", GdiCharsetToSci(lf.lfCharSet));
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), chset);
      }
    }
    else {
      StringCchPrintf(chset, COUNTOF(chset), L"; charset:%i", GdiCharsetToSci(lf.lfCharSet));
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), chset);
    }
  }

  if (lf.lfItalic) {
    if (bIsItalic) {
      if (Style_StrGetAttrItalic(lpszStyle)) {
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; italic");
      }
    }
    else {
      StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; italic");
    }
  }

  if (bWithEffects)
  {
    if (lf.lfUnderline) {
      if (bIsUnderline) {
        if (Style_StrGetAttrUnderline(lpszStyle)) {
          StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; underline");
        }
      }
      else {
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), L"; underline");
      }
    }

    if (lf.lfStrikeOut) {
      if (bIsStrikeout) {
        if (Style_StrGetAttrStrikeOut(lpszStyle)) {
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
      if (StrStr(lpszStyle, L"fore:")) {
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
    Style_CopyStyles_IfNotDefined(lpszStyle, szNewStyle, COUNTOF(szNewStyle), false);
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
  WCHAR szNewStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };
  COLORREF dRGBResult;
  COLORREF dColor;
  WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };

  dRGBResult = (bForeGround) ? GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_WINDOW);
  Style_StrGetColor(lpszStyle, (bForeGround ? FOREGROUND_LAYER : BACKGROUND_LAYER), &dRGBResult);

  CHOOSECOLOR cc;
  ZeroMemory(&cc, sizeof(CHOOSECOLOR));
  cc.lStructSize = sizeof(CHOOSECOLOR);
  cc.hwndOwner = hwnd;
  cc.rgbResult = dRGBResult;
  cc.lpCustColors = g_colorCustom;
  cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;
  if (!ChooseColor(&cc)) { return false; }
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
    Style_CopyStyles_IfNotDefined(lpszStyle, szNewStyle, COUNTOF(szNewStyle), true);
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
  char chFontName[80] = { '\0' };
  WCHAR wchFontName[80] = { L'\0' };
  if (Style_StrGetFontName(lpszStyle, wchFontName, COUNTOF(wchFontName))) {
    if (StrIsNotEmpty(wchFontName)) {
      WideCharToMultiByteEx(Encoding_SciCP, 0, wchFontName, -1, chFontName, COUNTOF(chFontName), NULL, NULL);
      SendMessage(hwnd, SCI_STYLESETFONT, iStyle, (LPARAM)chFontName);
    }
  }
  else if (bInitDefault) {
    Style_StrGetFontName(L"font:Default", wchFontName, COUNTOF(wchFontName));
    WideCharToMultiByteEx(Encoding_SciCP, 0, wchFontName, -1, chFontName, COUNTOF(chFontName), NULL, NULL);
    SendMessage(hwnd, SCI_STYLESETFONT, iStyle, (LPARAM)chFontName);
  }

  // Font Quality
  // default should be SC_EFF_QUALITY_LCD_OPTIMIZED
  WPARAM wQuality = (WPARAM)Settings2.SciFontQuality;
  if (Style_StrGetFontQuality(lpszStyle, tch, COUNTOF(tch)))
  {
    if (StringCchCompareN(tch, COUNTOF(tch), L"none", COUNTOF(L"none")) == 0)
      wQuality = SC_EFF_QUALITY_NON_ANTIALIASED;
    else if (StringCchCompareN(tch, COUNTOF(tch), L"standard", COUNTOF(L"standard")) == 0)
      wQuality = SC_EFF_QUALITY_ANTIALIASED;
    else if (StringCchCompareN(tch, COUNTOF(tch), L"cleartype", COUNTOF(L"cleartype")) == 0)
      wQuality = SC_EFF_QUALITY_LCD_OPTIMIZED;
    else if (StringCchCompareN(tch, COUNTOF(tch), L"default", COUNTOF(L"default")) == 0)
      wQuality = SC_EFF_QUALITY_DEFAULT;
    SendMessage(hwnd, SCI_SETFONTQUALITY, wQuality, 0);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_SETFONTQUALITY, wQuality, 0);
  }

  // Size values are relative to BaseFontSize/CurrentFontSize
  float fBaseFontSize = Style_GetCurrentFontSize();
  if (Style_StrGetSize(lpszStyle, &fBaseFontSize)) {
    if (iStyle == STYLE_DEFAULT) {
      if (bInitDefault) {
        _SetBaseFontSize(fBaseFontSize);
      }
      _SetCurrentFontSize(fBaseFontSize);
    }
    SendMessage(hwnd, SCI_STYLESETSIZEFRACTIONAL, iStyle, float2int(fBaseFontSize * SC_FONT_SIZE_MULTIPLIER));
  }
  else if (bInitDefault) {
    _SetBaseFontSize(fBaseFontSize);
    SendMessage(hwnd, SCI_STYLESETSIZEFRACTIONAL, STYLE_DEFAULT, float2int(fBaseFontSize * SC_FONT_SIZE_MULTIPLIER));
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

#if 0
  // Stretch
  if (Style_StrGetStretchValue(lpszStyle, &iValue)) {
    SendMessage(hwnd, SCI_STYLESETSTRETCH, iStyle, (LPARAM)iValue);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETSTRETCH, iStyle, (LPARAM)FONT_STRETCH_NORMAL);
  }
#endif

  // Italic
  if (Style_StrGetAttrItalic(lpszStyle)) {
    SendMessage(hwnd, SCI_STYLESETITALIC, iStyle, (LPARAM)true);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETITALIC, iStyle, (LPARAM)false);
  }
  // Underline
  if (Style_StrGetAttrUnderline(lpszStyle)) {
    SendMessage(hwnd, SCI_STYLESETUNDERLINE, iStyle, (LPARAM)true);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETUNDERLINE, iStyle, (LPARAM)false);
  }
  // StrikeOut
  if (Style_StrGetAttrStrikeOut(lpszStyle)) {
    SendMessage(hwnd, SCI_STYLESETSTRIKE, iStyle, (LPARAM)true);
  }
  else if (bInitDefault) {
    SendMessage(hwnd, SCI_STYLESETSTRIKE, iStyle, (LPARAM)false);
  }
  // EOL Filled
  if (Style_StrGetAttrEOLFilled(lpszStyle)) {
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
  Style_CopyStyles_IfNotDefined(szBuf, szBufNorm, COUNTOF(szBufNorm), true);

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


INT_PTR CALLBACK Style_CustomizeSchemesDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    static HWND       hwndTV;
    static HFONT      hFontTitle = NULL;
    static bool       fDragging;
    static PEDITLEXER pCurrentLexer = NULL;
    static PEDITSTYLE pCurrentStyle = NULL;
    static int        iCurStyleIdx  = -1;
    static HBRUSH     hbrFore;
    static HBRUSH     hbrBack;
    static bool       bIsStyleSelected = false;
    static bool       bWarnedNoIniFile = false;
    static WCHAR*     Style_StylesBackup[NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER];

    static WCHAR tchTmpBuffer[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = {L'\0'};

    switch (umsg)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
            SetDialogIconNP3(hwnd);

            InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
            if (UseDarkMode()) {
              SetExplorerTheme(GetDlgItem(hwnd, IDOK));
              SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_STYLEFORE));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_STYLEBACK));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_STYLEFONT));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_PREVIEW));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_STYLEDEFAULT));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_PREVSTYLE));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_NEXTSTYLE));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_IMPORT));
              SetExplorerTheme(GetDlgItem(hwnd, IDC_EXPORT));
              //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
              // remove themes to colorize static controls correctly
              SetWindowTheme(GetDlgItem(hwnd, IDC_STATIC), L"", L"");
              SetWindowTheme(GetDlgItem(hwnd, IDC_INFO_GROUPBOX), L"", L"");
            }
#endif
            DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);

            GetLngString(IDS_MUI_STYLEEDIT_HELP, tchTmpBuffer, COUNTOF(tchTmpBuffer));
            SetDlgItemText(hwnd, IDC_STYLEEDIT_HELP, tchTmpBuffer);

            // Backup Styles
            ZeroMemory(&Style_StylesBackup, NUMLEXERS * AVG_NUM_OF_STYLES_PER_LEXER * sizeof(WCHAR*));
            int cnt = 0;
            for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer)
            {
                Style_StylesBackup[cnt++] = StrDup(g_pLexArray[iLexer]->szExtensions);
                int i                     = 0;
                while (g_pLexArray[iLexer]->Styles[i].iStyle != -1)
                {
                    Style_StylesBackup[cnt++] = StrDup(g_pLexArray[iLexer]->Styles[i].szValue);
                    ++i;
                }
            }

            hwndTV    = GetDlgItem(hwnd, IDC_STYLELIST);
            fDragging = false;

            SHFILEINFO shfi;
            ZeroMemory(&shfi, sizeof(SHFILEINFO));

	          InitWindowCommon(hwndTV, true);
            InitTreeView(hwndTV);

            TreeView_SetExtendedStyle(hwndTV, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);

            UINT const flagIconSize = (dpi.y >= LargeIconDPI()) ? SHGFI_LARGEICON : SHGFI_SMALLICON;
            TreeView_SetImageList(hwndTV,
                                  (HIMAGELIST)SHGetFileInfoW(L"C:\\", FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO),
                                                             flagIconSize | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
                                  TVSIL_NORMAL);

            // findlexer
            int found = -1;
            for (int i = 0; i < COUNTOF(g_pLexArray); ++i)
            {
                if (g_pLexArray[i] == s_pLexCurrent)
                {
                    found = i;
                    break;
                }
            }

            // Build lexer tree view
            HTREEITEM hCurrentTVLex = NULL;
            for (int i = 0; i < COUNTOF(g_pLexArray); i++)
            {
                if (i == found)
                    hCurrentTVLex = Style_AddLexerToTreeView(hwndTV, g_pLexArray[i]);
                else
                    Style_AddLexerToTreeView(hwndTV, g_pLexArray[i]);
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
            iCurStyleIdx  = STY_DEFAULT;

            SendDlgItemMessage(hwnd, IDC_STYLEEDIT, EM_LIMITTEXT, max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS) - 1, 0);

            MakeBitmapButton(hwnd, IDC_PREVSTYLE, IDB_PREV, -1, -1);
            MakeBitmapButton(hwnd, IDC_NEXTSTYLE, IDB_NEXT, -1, -1);

            if (Settings.CustomSchemesDlgPosX == CW_USEDEFAULT || Settings.CustomSchemesDlgPosY == CW_USEDEFAULT)
            {
                CenterDlgInParent(hwnd, NULL);
            }
            else
            {
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

            // Set title font
            HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, IDC_STYLELABEL, WM_GETFONT, 0, 0);
            if (hFont) {
              LOGFONT lf;
              GetObject(hFont, sizeof(LOGFONT), &lf);
              lf.lfHeight = MulDiv(lf.lfHeight, 3, 2);
              lf.lfWeight = FW_BOLD;
              //lf.lfUnderline = true;
              if (hFontTitle) {
                DeleteObject(hFontTitle);
              }
              hFontTitle = CreateFontIndirectW(&lf);
              SendDlgItemMessageW(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)hFontTitle, true);
            }
        }
        return !0;

        case WM_DPICHANGED:
        {
          DPI_T dpi;
          dpi.x = LOWORD(wParam);
          dpi.y = HIWORD(wParam);

          SHFILEINFO shfi;
          ZeroMemory(&shfi, sizeof(SHFILEINFO));
          UINT const flagIconSize = (dpi.y >= LargeIconDPI()) ? SHGFI_LARGEICON : SHGFI_SMALLICON;
          TreeView_SetImageList(hwndTV,
                                (HIMAGELIST)SHGetFileInfoW(L"C:\\", FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO),
                                                           flagIconSize | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
                                TVSIL_NORMAL);

          // Set title font
          HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, IDC_STYLELABEL, WM_GETFONT, 0, 0);
          if (hFont) {
            LOGFONT lf;
            GetObject(hFont, sizeof(LOGFONT), &lf);
            lf.lfHeight = MulDiv(lf.lfHeight, 3, 2);
            lf.lfWeight = FW_BOLD;
            //lf.lfUnderline = true;
            if (hFontTitle) {
              DeleteObject(hFontTitle);
            }
            hFontTitle = CreateFontIndirectW(&lf);
            SendDlgItemMessageW(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)hFontTitle, true);
          }

          MakeBitmapButton(hwnd, IDC_PREVSTYLE, IDB_PREV, -1, -1);
          MakeBitmapButton(hwnd, IDC_NEXTSTYLE, IDB_NEXT, -1, -1);

          UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
        }
        return !0;

        case WM_PAINT:
        {
          PAINTSTRUCT ps;
          HDC const   hdc = GetDC(hwnd); // ClientArea
          if (hdc) {
            BeginPaint(hwnd, &ps);

            DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);

            int const   iconSize  = 64;
            int const   dpiWidth  = ScaleIntByDPI(iconSize, dpi.x);
            int const   dpiHeight = ScaleIntByDPI(iconSize, dpi.y);
            HICON const hicon     = (dpiHeight > 128) ? Globals.hDlgIconPrefs256 : ((dpiHeight > 64) ? Globals.hDlgIconPrefs128 : Globals.hDlgIconPrefs64);
            if (hicon)
            {
              RECT rc = {0};
              MapWindowPoints(GetDlgItem(hwnd, IDC_INFO_GROUPBOX), hwnd, (LPPOINT)&rc, 2);
              DrawIconEx(hdc, rc.left + ScaleIntByDPI(10, dpi.x), rc.top + ScaleIntByDPI(20, dpi.y), hicon, dpiWidth, dpiHeight, 0, NULL, DI_NORMAL);
            }

            ReleaseDC(hwnd, hdc);
            EndPaint(hwnd, &ps);
          }
        }
        return 0;

#ifdef D_NP3_WIN10_DARK_MODE

        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSTATIC:
          if (UseDarkMode()) {
            return SetDarkModeCtlColors((HDC)wParam);
          }
          break;

        case WM_SETTINGCHANGE:
          if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
          }
          break;

        case WM_THEMECHANGED:
          if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindow(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);
            int const buttons[] = { IDOK, IDCANCEL, IDC_STYLEFORE, IDC_STYLEBACK, IDC_STYLEFONT, IDC_PREVIEW,
                                    IDC_STYLEDEFAULT, IDC_PREVSTYLE, IDC_NEXTSTYLE, IDC_IMPORT, IDC_EXPORT };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
              HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
              AllowDarkModeForWindow(hBtn, darkModeEnabled);
              SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            SendMessage(hwndTV, WM_THEMECHANGED, 0, 0);

            UpdateWindow(hwnd);
          }
          break;
#endif

        case WM_ENABLE:
          // modal child dialog should disable main window too
          EnableWindow(Globals.hwndMain, (BOOL)wParam);
          return !0;

        case WM_ACTIVATE:
            DialogEnableControl(hwnd, IDC_PREVIEW, ((pCurrentLexer == s_pLexCurrent) || (pCurrentLexer == GetCurrentStdLexer())));
            return !0;

        case WM_DESTROY:
            {
                DeleteBitmapButton(hwnd, IDC_STYLEFORE);
                DeleteBitmapButton(hwnd, IDC_STYLEBACK);
                DeleteBitmapButton(hwnd, IDC_PREVSTYLE);
                DeleteBitmapButton(hwnd, IDC_NEXTSTYLE);

                // free old backup
                int cnt = 0;
                for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer)
                {
                    if (Style_StylesBackup[cnt])
                    {
                        LocalFree(Style_StylesBackup[cnt]); // StrDup()
                        Style_StylesBackup[cnt] = NULL;
                    }
                    ++cnt;
                    int i = 0;
                    while (g_pLexArray[iLexer]->Styles[i].iStyle != -1)
                    {
                        if (Style_StylesBackup[cnt])
                        {
                            LocalFree(Style_StylesBackup[cnt]); // StrDup()
                            Style_StylesBackup[cnt] = NULL;
                        }
                        ++cnt;
                        ++i;
                    }
                }
                if (hFontTitle) {
                  DeleteObject(hFontTitle);
                  hFontTitle = NULL;
                }
                pCurrentLexer = NULL;
                pCurrentStyle = NULL;
                iCurStyleIdx  = -1;
            }
            return false;

        case WM_SYSCOMMAND:
            if (wParam == IDS_MUI_SAVEPOS)
            {
                PostWMCommand(hwnd, IDACC_SAVEPOS);
                return !0;
            }
            else if (wParam == IDS_MUI_RESETPOS)
            {
                PostWMCommand(hwnd, IDACC_RESETPOS);
                return !0;
            }
            else
                return 0;

        case WM_NOTIFY:

            if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST)
            {
                LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;

                switch (lpnmtv->hdr.code)
                {
                    case TVN_SELCHANGED:
                    {
                        if (pCurrentLexer && pCurrentStyle)
                        {
                            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
                        }

                        WCHAR name[80]   = {L'\0'};
                        WCHAR label[128] = {L'\0'};

                        //DialogEnableWindow(hwnd, IDC_STYLEEDIT, true);
                        //DialogEnableWindow(hwnd, IDC_STYLEFONT, true);
                        //DialogEnableWindow(hwnd, IDC_STYLEFORE, true);
                        //DialogEnableWindow(hwnd, IDC_STYLEBACK, true);
                        //DialogEnableWindow(hwnd, IDC_STYLEDEFAULT, true);

                        // a lexer has been selected
                        if (!TreeView_GetParent(hwndTV, lpnmtv->itemNew.hItem))
                        {
                            pCurrentLexer = (PEDITLEXER)lpnmtv->itemNew.lParam;

                            if (pCurrentLexer)
                            {
                                bIsStyleSelected = false;
                                GetLngString(IDS_MUI_ASSOCIATED_EXT, label, COUNTOF(label));
                                SetDlgItemText(hwnd, IDC_STYLELABEL_ROOT, label);
                                DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, true);
                                SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->szExtensions);
                                DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, true);

                                if (IsLexerStandard(pCurrentLexer))
                                {
                                    pCurrentStyle = &(pCurrentLexer->Styles[STY_DEFAULT]);
                                    iCurStyleIdx  = STY_DEFAULT;

                                    if (pCurrentStyle->rid == IDS_LEX_STD_STYLE)
                                    {
                                        GetLngString(IDS_MUI_STY_BASESTD, label, COUNTOF(label));
                                    }
                                    else
                                    {
                                        GetLngString(IDS_MUI_STY_BASE2ND, label, COUNTOF(label));
                                        DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, false);
                                    }
                                    DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, false);
                                }
                                else
                                {
                                    pCurrentStyle = &(pCurrentLexer->Styles[STY_DEFAULT]);
                                    iCurStyleIdx  = STY_DEFAULT;
                                    GetLngString(pCurrentLexer->resID, name, COUNTOF(name));
                                    FormatLngStringW(label, COUNTOF(label), IDS_MUI_STY_LEXDEF, name);
                                    DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, true);
                                }
                                SetDlgItemText(hwnd, IDC_STYLELABEL, label);
                                SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
                            }
                            else
                            {
                                SetDlgItemText(hwnd, IDC_STYLELABEL_ROOT, L"");
                                DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, false);
                                SetDlgItemText(hwnd, IDC_STYLELABEL, L"");
                                DialogEnableControl(hwnd, IDC_STYLEEDIT, false);
                            }
                            DialogEnableControl(hwnd, IDC_PREVIEW, ((pCurrentLexer == s_pLexCurrent) || (pCurrentLexer == GetCurrentStdLexer())));
                        }
                        // a style has been selected
                        else
                        {
                            if (pCurrentLexer)
                            {
                                if (IsLexerStandard(pCurrentLexer))
                                {
                                    if (pCurrentLexer->Styles[STY_DEFAULT].rid == IDS_LEX_STD_STYLE)
                                        GetLngString(IDS_MUI_STY_BASESTD, label, COUNTOF(label));
                                    else
                                        GetLngString(IDS_MUI_STY_BASE2ND, label, COUNTOF(label));
                                }
                                else
                                {
                                    GetLngString(pCurrentLexer->resID, name, COUNTOF(name));
                                    FormatLngStringW(label, COUNTOF(label), IDS_MUI_STY_LEXDEF, name);
                                }
                                SetDlgItemText(hwnd, IDC_STYLELABEL_ROOT, label);

                                SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->Styles[STY_DEFAULT].szValue);
                                DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, false);

                                pCurrentStyle = (PEDITSTYLE)lpnmtv->itemNew.lParam;
                                iCurStyleIdx  = -1;
                                int i         = 0;
                                while (pCurrentLexer->Styles[i].iStyle != -1)
                                {
                                    if (pCurrentLexer->Styles[i].rid == pCurrentStyle->rid)
                                    {
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
                                DialogEnableControl(hwnd, IDC_STYLEEDIT, false);
                            }
                        }
                    }
                    break;

                    case TVN_BEGINDRAG:
                    {
                        TreeView_Select(hwndTV, lpnmtv->itemNew.hItem, TVGN_CARET);

                        if (bIsStyleSelected)
                            DestroyCursor(SetCursor(LoadCursor(Globals.hInstance, MAKEINTRESOURCE(IDC_COPY))));
                        else
                            DestroyCursor(SetCursor(LoadCursor(NULL, IDC_NO)));

                        SetCapture(hwnd);
                        fDragging = true;
                    }
                }
            }
            break;

        case WM_MOUSEMOVE:
        {
            HTREEITEM     htiTarget;
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
                MapWindowPoints(hwnd, hwndTV, &tvht.pt, 1);

                if ((htiTarget = TreeView_HitTest(hwndTV, &tvht)) != NULL &&
                    TreeView_GetParent(hwndTV, htiTarget) != NULL)
                {
                    TreeView_SelectDropTarget(hwndTV, htiTarget);
                    //TreeView_Expand(hwndTV,htiTarget,TVE_EXPAND);
                    TreeView_EnsureVisible(hwndTV, htiTarget);
                }
                else
                    TreeView_SelectDropTarget(hwndTV, NULL);

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
                    WCHAR tchCopy[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = {L'\0'};
                    TreeView_SelectDropTarget(hwndTV, NULL);
                    GetDlgItemText(hwnd, IDC_STYLEEDIT, tchCopy, COUNTOF(tchCopy));
                    TreeView_Select(hwndTV, htiTarget, TVGN_CARET);

                    // after select, this is new current item
                    SetDlgItemText(hwnd, IDC_STYLEEDIT, tchCopy);
                    _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
                }
                ReleaseCapture();
                DestroyCursor(SetCursor(LoadCursor(NULL, IDC_ARROW)));
                fDragging = false;
            }
        }
        break;

        case WM_CANCELMODE:
        {
            if (fDragging)
            {
                //ImageList_EndDrag();
                TreeView_SelectDropTarget(hwndTV, NULL);
                ReleaseCapture();
                DestroyCursor(SetCursor(LoadCursor(NULL, IDC_ARROW)));
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
                    for (int i = 0; i < COUNTOF(g_pLexArray); ++i)
                    {
                        if (g_pLexArray[i] == s_pLexCurrent)
                        {
                            break;
                        }
                        hCurrentTVLex = TreeView_GetNextSibling(hwndTV, hCurrentTVLex); // next
                    }
                    if (s_pLexCurrent == pCurrentLexer)
                        break; // no change

                    // collaps current node
                    HTREEITEM hSel = TreeView_GetSelection(hwndTV);
                    if (hSel)
                    {
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
                    iCurStyleIdx  = STY_DEFAULT;

                    PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
                }
                break;

                case IDC_STYLEFORE:
                    if (pCurrentStyle)
                    {
                        WCHAR tch[BUFSIZE_STYLE_VALUE] = {L'\0'};
                        GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
                        if (Style_SelectColor(hwnd, true, tch, COUNTOF(tch), true))
                        {
                            SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
                        }
                    }
                    PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
                    break;

                case IDC_STYLEBACK:
                    if (pCurrentStyle)
                    {
                        WCHAR tch[BUFSIZE_STYLE_VALUE] = {L'\0'};
                        GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
                        if (Style_SelectColor(hwnd, false, tch, COUNTOF(tch), true))
                        {
                            SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
                        }
                    }
                    PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
                    break;

                case IDC_STYLEFONT:
                    if (pCurrentStyle)
                    {
                        WCHAR lexerName[BUFSIZE_STYLE_VALUE] = {L'\0'};
                        WCHAR styleName[BUFSIZE_STYLE_VALUE] = {L'\0'};
                        GetDlgItemText(hwnd, IDC_STYLEEDIT, tchTmpBuffer, COUNTOF(tchTmpBuffer));
                        GetLngString(pCurrentLexer->resID, lexerName, COUNTOF(lexerName));
                        GetLngString(pCurrentStyle->rid, styleName, COUNTOF(styleName));
                        if (Style_SelectFont(hwnd, tchTmpBuffer, COUNTOF(tchTmpBuffer), lexerName, styleName,
                                             IsStyleStandardDefault(pCurrentStyle), IsStyleSchemeDefault(pCurrentStyle), true, true))
                        {
                            SetDlgItemText(hwnd, IDC_STYLEEDIT, tchTmpBuffer);
                        }
                    }
                    PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
                    break;

                case IDC_STYLEDEFAULT:
                {
                    SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->pszDefault);
                    if (!bIsStyleSelected)
                    {
                        SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->pszDefExt);
                    }
                    _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
                    PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
                }
                break;

                case IDC_STYLEEDIT:
                {
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        WCHAR tch[max(BUFSIZE_STYLE_VALUE, BUFZIZE_STYLE_EXTENTIONS)] = {L'\0'};

                        GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));

                        COLORREF cr = COLORREF_MAX; // SciCall_StyleGetFore(STYLE_DEFAULT);
                        Style_StrGetColor(tch, FOREGROUND_LAYER, &cr);
                        MakeColorPickButton(hwnd, IDC_STYLEFORE, Globals.hInstance, cr);

                        cr = COLORREF_MAX; // SciCall_StyleGetBack(STYLE_DEFAULT);
                        Style_StrGetColor(tch, BACKGROUND_LAYER, &cr);
                        MakeColorPickButton(hwnd, IDC_STYLEBACK, Globals.hInstance, cr);
                    }
                }
                break;

                case IDC_IMPORT:
                {
                    hwndTV = GetDlgItem(hwnd, IDC_STYLELIST);

                    if (Style_Import(hwnd))
                    {
                        SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
                        if (!bIsStyleSelected)
                        {
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
                    if (hSel)
                    {
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
                    if (hSel)
                    {
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

                    if ((!bWarnedNoIniFile && StrIsEmpty(Theme_Files[Globals.idxSelectedTheme].szFilePath)) && (Globals.idxSelectedTheme > 0))
                    {
                        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SETTINGSNOTSAVED);
                        bWarnedNoIniFile = true;
                    }
                    //EndDialog(hwnd,IDOK);
                    DestroyWindow(hwnd);
                }
                break;

                case IDCANCEL:
                    if (fDragging)
                    {
                        SendMessage(hwnd, WM_CANCELMODE, 0, 0);
                    }
                    else
                    {
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
                                Style_CopyStyles_IfNotDefined(Style_StylesBackup[cnt], tchTmpBuffer, COUNTOF(tchTmpBuffer), true);
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
                    PostWMCommand(hwnd, IDC_SETCURLEXERTV);
                    break;

                case IDACC_PREVIEW:
                    PostWMCommand(hwnd, IDC_PREVIEW);
                    break;

                case IDACC_SAVEPOS:
                    GetDlgPos(hwnd, &Settings.CustomSchemesDlgPosX, &Settings.CustomSchemesDlgPosY);
                    break;

                case IDACC_RESETPOS:
                    CenterDlgInParent(hwnd, NULL);
                    Settings.CustomSchemesDlgPosX = Settings.CustomSchemesDlgPosY = CW_USEDEFAULT;
                    break;

                default:
                    // return false???
                    break;

            } // switch()
        }     // WM_COMMAND
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
                                      (LPARAM)hwnd);
  if (hDlg != INVALID_HANDLE_VALUE) {
    ShowWindow(hDlg, SW_SHOW);
  }
  return hDlg;
}


//=============================================================================
//
//  Style_SelectLexerDlgProc()
//

static PEDITLEXER _s_selectedLexer = NULL;
static int _s_idefaultLexer = -1;

INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam)
{
  static int cxClient = 0;
  static int cyClient = 0;

  static HWND hwndLV = NULL;

  static int  iInternalDefault = 0;

  switch(umsg)
  {
    case WM_INITDIALOG:
      {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
        SetDialogIconNP3(hwnd);

        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
          SetExplorerTheme(GetDlgItem(hwnd, IDOK));
          SetExplorerTheme(GetDlgItem(hwnd, IDCANCEL));
          //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
          int const ctl[] = { IDC_DEFAULTSCHEME, IDC_AUTOSELECT, IDC_STATIC };
          for (int i = 0; i < COUNTOF(ctl); ++i) {
            SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
          }
        }
#endif

        DPI_T const dpi = Scintilla_GetWindowDPI(hwnd);

        hwndLV = GetDlgItem(hwnd,IDC_STYLELIST);
        InitWindowCommon(hwndLV, true);
        InitListView(hwndLV);

        SHFILEINFO shfi;
        ZeroMemory(&shfi, sizeof(SHFILEINFO));

        UINT const flagIconSize = (dpi.y >= LargeIconDPI()) ? SHGFI_LARGEICON : SHGFI_SMALLICON;
        ListView_SetImageList(hwndLV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\",FILE_ATTRIBUTE_DIRECTORY,
            &shfi, sizeof(SHFILEINFO), flagIconSize | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
          LVSIL_SMALL);

        ListView_SetImageList(hwndLV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\",FILE_ATTRIBUTE_DIRECTORY,
            &shfi,sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
          LVSIL_NORMAL);

        LVCOLUMN lvc = { LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT, 0, L"", -1, 0, 0, 0 };
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

          if (((PEDITLEXER)lvi.lParam)->resID == _s_selectedLexer->resID)
          {
            ListView_SetItemState(hwndLV,i,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
            ListView_EnsureVisible(hwndLV,i,false);
            CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, SetBtn(_s_idefaultLexer == i));
            break;
          }
        }

        iInternalDefault = _s_idefaultLexer;
        CheckDlgButton(hwnd,IDC_AUTOSELECT, SetBtn(s_bAutoSelect));

        CenterDlgInParent(hwnd, NULL);
      }
      return !0;


    case WM_DPICHANGED:
      {
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
  
        DPI_T dpi;
        dpi.x = LOWORD(wParam);
        dpi.y = HIWORD(wParam);

        SHFILEINFO shfi;
        ZeroMemory(&shfi, sizeof(SHFILEINFO));
        UINT const flagIconSize = (dpi.y >= LargeIconDPI()) ? SHGFI_LARGEICON : SHGFI_SMALLICON;
        ListView_SetImageList(hwndLV,
          (HIMAGELIST)SHGetFileInfo(L"C:\\", FILE_ATTRIBUTE_DIRECTORY,
            &shfi, sizeof(SHFILEINFO), flagIconSize | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
          LVSIL_SMALL);
      }
      return !0;


    case WM_DESTROY:
        return !0;


    case WM_SIZE:
      {
        ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_STYLELIST), 0, LVSCW_AUTOSIZE_USEHEADER);
      }
      return !0;


    case WM_GETMINMAXINFO:
      return !0;


#ifdef D_NP3_WIN10_DARK_MODE

    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC:
      if (UseDarkMode()) {
        return SetDarkModeCtlColors((HDC)wParam);
      }
      break;

    case WM_SETTINGCHANGE:
      if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
        SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
      }
      break;

    case WM_THEMECHANGED:
      if (IsDarkModeSupported()) {
        bool const darkModeEnabled = CheckDarkModeEnabled();
        AllowDarkModeForWindow(hwnd, darkModeEnabled);
        RefreshTitleBarThemeColor(hwnd);
        int const buttons[] = { IDOK, IDCANCEL };
        for (int id = 0; id < COUNTOF(buttons); ++id) {
          HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
          AllowDarkModeForWindow(hBtn, darkModeEnabled);
          SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
        }
        SendMessage(hwndLV, WM_THEMECHANGED, 0, 0);

        UpdateWindow(hwnd);
      }
      break;

#endif


    case WM_NOTIFY: 
      {
        if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST) {

          switch (((LPNMHDR)(lParam))->code) {

          case NM_DBLCLK:
            SendWMCommand(hwnd, IDOK);
            break;

          case LVN_ITEMCHANGED:
          case LVN_DELETEITEM:
            {
              int i = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
              CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, SetBtn(iInternalDefault == i));
              DialogEnableControl(hwnd, IDC_DEFAULTSCHEME, i != -1);
              DialogEnableControl(hwnd, IDOK, i != -1);
            }
            break;
          }
        }
      }
      return !0;


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
              _s_selectedLexer = (PEDITLEXER)lvi.lParam;
              _s_idefaultLexer = iInternalDefault;
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
      return !0;
  }
  return 0;
}


//=============================================================================
//
//  Style_SelectLexerDlg()
//
void Style_SelectLexerDlg(HWND hwnd)
{
  _s_selectedLexer = s_pLexCurrent;
  _s_idefaultLexer = s_iDefaultLexer;

  if (IDOK == ThemedDialogBoxParam(Globals.hLngResContainer,
    MAKEINTRESOURCE(IDD_MUI_STYLESELECT),
    GetParent(hwnd), Style_SelectLexerDlgProc, 0))
  {
    s_iDefaultLexer = _s_idefaultLexer;
    Style_SetLexer(Globals.hwndEdit, _s_selectedLexer);
  }
}

// End of Styles.c
