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
*                                                  (c) Rizonesoft 2008-2022   *
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

#include "Lexilla.h"
#include "lexers_x/SciXLexer.h"

#include "uthash/utarray.h"

#include "PathLib.h"
#include "Edit.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Config/Config.h"
#include "DarkMode/DarkMode.h"

#include "SciCall.h"

#include "Styles.h"

extern COLORREF  g_colorCustom[16];

// removed from project, not MUI language compatible with ChooseFont()
//~bool ChooseFontDirectWrite(HWND hwnd, const WCHAR* localeName, UINT dpi, LPCHOOSEFONT lpCF);

// ----------------------------------------------------------------------------

// This array holds all the lexers...
// Don't forget to change the number of the lexer for HTML and XML
// in Notepad2.c ParseCommandLine() if you change this array!
static PEDITLEXER g_pLexArray[] = {
    &lexStandard,      // Default Text
    &lexStandard2nd,   // 2nd Default Text
    &lexTEXT,          // Pure Text Files (Constants.StdDefaultLexerID = 2)
    &lexANSI,          // ANSI Files (ASCII Art)
    &lexCONF,          // Apache Config Files
    &lexASM,           // Assembly Script
    &lexAHK,           // AutoHotkey Script
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
    &lexDart,          // Dart Source Code
    &lexDIFF,          // Diff Files
    &lexFortran,       // Fortran F90+
    //&lexF77,           // Fortran F77
    &lexGo,            // Go Source Code
    &lexINNO,          // Inno Setup Script
    &lexJAVA,          // Java Source Code
    &lexJS,            // JavaScript
    &lexJSON,          // JSON
    &lexJulia,         // Julia
    &lexKiX,           // KiX
    &lexKotlin,        // Kotlin
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
    &lexSysVerilog,    // SystemVerilog HDVL
    &lexTCL,           // Tcl Script
    &lexTOML,          // TOML Config Script
    &lexVBS,           // VBScript
    &lexVerilog,       // Verilog HDL
    &lexVHDL,          // VHDL
    &lexVB,            // Visual Basic
    &lexHTML,          // Web Source Code
    &lexXML,           // XML Document
    &lexYAML,          // YAML
};

int Style_NumOfLexers() {
    return COUNTOF(g_pLexArray);
}


// Currently used lexer
static PEDITLEXER s_pLexCurrent = &lexStandard;
static int s_iDefaultLexer = 2; // (Constants.StdDefaultLexerID) Pure Text Files


const COLORREF s_colorLightDefault[16] = {
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

const COLORREF s_colorDarkDefault[16] = {
    RGB(0xDE, 0xDE, 0xDE),
    RGB(0xB4, 0xE1, 0xF5),
    RGB(0xA1, 0xC3, 0xD3),
    RGB(0x89, 0xAB, 0xEC),
    RGB(0x71, 0xF8, 0xAD),
    RGB(0xB1, 0xD7, 0x65),
    RGB(0xD8, 0xF7, 0x66),
    RGB(0xF6, 0xB0, 0x5A),
    RGB(0x14, 0x14, 0x14),
    RGB(0x27, 0x27, 0x02),
    RGB(0x54, 0x46, 0x04),
    RGB(0xF2, 0xB5, 0x0D),
    RGB(0xF2, 0x46, 0x0D),
    RGB(0xF5, 0x3C, 0x3D),
    RGB(0xF6, 0x51, 0xF6),
    RGB(0xBE, 0x94, 0x4E)
};


static bool s_bAutoSelect = true;

#define STYLESELECTDLG_X 305
#define STYLESELECTDLG_Y 344
static int  s_cxStyleSelectDlg = STYLESELECTDLG_X;
static int  s_cyStyleSelectDlg = STYLESELECTDLG_Y;

//=============================================================================

// Font Weights
typedef struct _fntwght {
    LPCWSTR const wname;
    int const weight;
} FONTWEIGHT_T;

static const FONTWEIGHT_T FontWeights[21] = {
    { L"thin",        FW_THIN },       //  0
    { L"semithin",    150 },           //  1
    { L"extralight",  FW_EXTRALIGHT }, //  2
    { L"lighter",     250 },           //  3
    { L"light",       FW_LIGHT },      //  4
    { L"book",        350 },           //  5
    { L"text",        375 },           //  6
    { L"regular",     FW_REGULAR },    //  7
    { L"thick",       425 },           //  8
    { L"retina",      450 },           //  9
    { L"medium",      FW_MEDIUM },     // 10
    { L"extramedium", 550 },           // 11
    { L"semibold",    FW_SEMIBOLD },   // 12
    { L"dark",        650 },           // 13
    { L"bold",        FW_BOLD },       // 14
    { L"bolder",      750 },           // 15
    { L"extrabold",   FW_EXTRABOLD },  // 16
    { L"semiheavy",   850 },           // 17
    { L"heavy",       FW_HEAVY },      // 18
    { L"extrablack",  950 },           // 19
    { L"ultradark",   1000 },          // 20
};

typedef enum {
    FW_IDX_THIN = 0,
    FW_IDX_SEMITHIN,
    FW_IDX_EXTRALIGHT,
    FW_IDX_LIGHTER,
    FW_IDX_LIGHT,
    FW_IDX_BOOK,
    FW_IDX_TEXT,
    FW_IDX_REGULAR,
    FW_IDX_THICK,
    FW_IDX_RETINA,
    FW_IDX_MEDIUM,
    FW_IDX_EXTRAMEDIUM,
    FW_IDX_SEMIBOLD,
    FW_IDX_DARK,
    FW_IDX_BOLD,
    FW_IDX_BOLDER,
    FW_IDX_EXTRABOLD,
    FW_IDX_SEMIHEAVY,
    FW_IDX_HEAVY,
    FW_IDX_EXTRABLACK,
    FW_IDX_ULTRADARK
} FW_IDX;

//// font quality
//#define Style_StrHasAttrNone(lpszStyle)         Style_StrHasAttribute((lpszStyle), L"none")
//#define Style_StrHasAttrStdType(lpszStyle)      Style_StrHasAttribute((lpszStyle), L"standard")
//#define Style_StrHasAttrClearType(lpszStyle)    Style_StrHasAttribute((lpszStyle), L"cleartype")

// font effects
static const WCHAR *const FontEffects[] = { L"italic", L"underline", L"strikeout", L"eolfilled" };
typedef enum { FE_ITALIC = 0, FE_UNDERLINE, FE_STRIKEOUT, FE_EOLFILLED } FE_IDX;

// caret style
static const WCHAR *const CaretStyle[] = { L"ovrblck", L"block", L"noblink", };
typedef enum { CS_OVRBLCK = 0, CS_BLOCK, CS_NOBLINK, } CS_IDX;

typedef struct _fntqual {
    LPCWSTR const qname;
    int const sci_value;
    int const win_value;
} FONTQUALITY_T;

static const FONTQUALITY_T FontQuality[4] = {
    { L"standard", SC_EFF_QUALITY_DEFAULT, DEFAULT_QUALITY },
    { L"aliased", SC_EFF_QUALITY_NON_ANTIALIASED, NONANTIALIASED_QUALITY },
    { L"antialiased", SC_EFF_QUALITY_ANTIALIASED, ANTIALIASED_QUALITY },
    { L"cleartype", SC_EFF_QUALITY_LCD_OPTIMIZED, CLEARTYPE_QUALITY }
};
typedef enum { FQ_STANDARD = 0, FQ_ALIASED, FQ_ANTIALIASED, FQ_CLEARTYPE } FQ_IDX;

static inline int MapSciToWinFontQuality(const int sciFQ)
{
    for (int i = 0; i < COUNTOF(FontQuality); ++i) {
        if (FontQuality[i].sci_value == sciFQ) {
            return FontQuality[i].win_value;
        }
    }
    // default should be CLEARTYPE_QUALITY
    return CLEARTYPE_QUALITY; //~DEFAULT_QUALITY;
}

#if 0
static inline int MapFQNameToSciFontQuality(LPCWSTR fqName) {
    for (int i = 0; i < COUNTOF(FontQuality); ++i) {
        if (StringCchCompareXI(fqName, FontQuality[i].qname) == 0) {
            return FontQuality[i].sci_value;
        }
    }
    // default should be SC_EFF_QUALITY_LCD_OPTIMIZED
    return SC_EFF_QUALITY_LCD_OPTIMIZED; //~SC_EFF_QUALITY_DEFAULT;
}
#endif

//=============================================================================

// ensure to be consistent with 
static WCHAR* IndicatorTypes[23] = {
    L"indic_plain",              //  0 INDIC_PLAIN
    L"indic_squiggle",           //  1 INDIC_SQUIGGLE
    L"indic_tt",                 //  2 INDIC_TT
    L"indic_diagonal",           //  3 INDIC_DIAGONAL
    L"indic_strike",             //  4 INDIC_STRIKE
    L"indic_hidden",             //  5 INDIC_HIDDEN
    L"indic_box",                //  6 INDIC_BOX
    L"indic_roundbox",           //  7 INDIC_ROUNDBOX
    L"indic_straightbox",        //  8 INDIC_STRAIGHTBOX
    L"indic_dash",               //  9 INDIC_DASH
    L"indic_dots",               // 10 INDIC_DOTS
    L"indic_squigglelow",        // 11 INDIC_SQUIGGLELOW
    L"indic_dotbox",             // 12 INDIC_DOTBOX
    L"indic_squigglepixmap",     // 13 INDIC_SQUIGGLEPIXMAP
    L"indic_compositionthick",   // 14 INDIC_COMPOSITIONTHICK
    L"indic_compositionthin",    // 15 INDIC_COMPOSITIONTHIN
    L"indic_fullbox",            // 16 INDIC_FULLBOX
    L"indic_textfore",           // 17 INDIC_TEXTFORE 
    L"indic_point",              // 18 INDIC_POINT
    L"indic_pointcharacter",     // 19 INDIC_POINTCHARACTER
    L"indic_gradient",           // 20 INDIC_GRADIENT
    L"indic_gradientcentre",     // 21 INDIC_GRADIENTCENTRE
    L"indic_point_top"           // 22 INDIC_POINT_TOP
};

static inline bool HasIndicStyleStrokeWidth(const int indicStyle) {
    switch (indicStyle) {
    case INDIC_PLAIN:
    case INDIC_SQUIGGLE:
    case INDIC_TT:
    case INDIC_DIAGONAL:
    case INDIC_STRIKE:
    case INDIC_BOX:
    case INDIC_ROUNDBOX:
    case INDIC_STRAIGHTBOX:
    case INDIC_DASH:
    case INDIC_DOTS:
    case INDIC_SQUIGGLELOW:
    case INDIC_FULLBOX:
        return true;
    default:
        break;
    }
    return false;
}


//=============================================================================

THEMEFILES Theme_Files[] = {
    { 0, L"Standard Config", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL },
    { 0, L"", NULL }
};

unsigned ThemeItems_CountOf()
{
    return COUNTOF(Theme_Files);
}

void ThemesItems_Init()
{
    for (unsigned i = 0; i < ThemeItems_CountOf(); ++i) {
        Theme_Files[i].rid = 0;
        Theme_Files[i].szName[0] = L'\0';
        if (Theme_Files[i].hStyleFilePath == NULL) {
            Theme_Files[i].hStyleFilePath = Path_Allocate(L"");
        }
    }
}

void ThemesItems_Release()
{
    for (unsigned i = 0; i < ThemeItems_CountOf(); ++i) {
        if (Theme_Files[i].hStyleFilePath != NULL) {
            Path_Release(Theme_Files[i].hStyleFilePath);
        }
    }
}

unsigned ThemesItems_MaxIndex()
{
    for (unsigned i = 1; i < ThemeItems_CountOf(); ++i) {
        if (Theme_Files[i].rid == 0) {
            return (i - 1);
        }
    }
    return (ThemeItems_CountOf() - 1);
}


static void _FillThemesMenuTable()
{
    HPATHL hThemesDir = Path_Copy(Paths.IniFile);

    // NP3.ini settings

    Theme_Files[0].rid = IDM_THEMES_STD_CFG;
    GetLngString(IDM_THEMES_STD_CFG, Theme_Files[0].szName, COUNTOF(Theme_Files[0].szName));
    if (Path_IsNotEmpty(hThemesDir)) {
        Path_Reset(Theme_Files[0].hStyleFilePath, Path_Get(hThemesDir));
    }
    Globals.uCurrentThemeIndex = 0;

    unsigned iTheme = 1; // other themes

    if (Path_IsEmpty(hThemesDir)) {
        Path_Reset(hThemesDir, Path_Get(Paths.IniFileDefault));
    }
    if (Path_IsNotEmpty(hThemesDir)) {
        Path_RemoveFileSpec(hThemesDir);
        Path_Append(hThemesDir, L"themes");
    }

    /// names are filled by Style_InsertThemesMenu()

    if (Path_IsExistingDirectory(hThemesDir)) {

        HPATHL hThemePath = Path_Copy(hThemesDir);
        Path_Append(hThemePath, L"*.ini");

        WIN32_FIND_DATA FindFileData;
        ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATA));
        HANDLE hFindFile = FindFirstFileW(Path_Get(hThemePath), &FindFileData);

        // ---  fill table by directory entries  ---

        if (IS_VALID_HANDLE(hFindFile)) {
            
            WCHAR wchFileName[SMALL_BUFFER] = { L'\0' };

            for (iTheme = 1; iTheme < ThemeItems_CountOf(); ++iTheme) {

                Theme_Files[iTheme].rid = (iTheme + IDM_THEMES_STD_CFG);

                StringCchCopy(wchFileName, COUNTOF(wchFileName), PathFindFileNameW(FindFileData.cFileName));
                PathRemoveExtensionW(wchFileName);
                StringCchCopy(Theme_Files[iTheme].szName, COUNTOF(Theme_Files[iTheme].szName), wchFileName);

                if (StringCchCompareXI(Theme_Files[iTheme].szName, Settings.CurrentThemeName) == 0) {
                    Globals.uCurrentThemeIndex = iTheme;
                }

                Path_Reset(hThemePath, Path_Get(hThemesDir));
                Path_Append(hThemePath, FindFileData.cFileName);
                Path_Swap(Theme_Files[iTheme].hStyleFilePath, hThemePath);

                if (!FindNextFileW(hFindFile, &FindFileData)) {
                    break;
                }
            }
            FindClose(hFindFile);
        }
        Path_Release(hThemePath);
    }

    Path_Release(hThemesDir);
}

//=============================================================================

static inline void AppendStyle(LPWSTR lpszStyleDest, size_t cchSizeDest, LPCWSTR lpszStyleSrc) {
    StringCchCat(lpszStyleDest, cchSizeDest, L"; ");
    StringCchCat(lpszStyleDest, cchSizeDest, lpszStyleSrc);
}

//=============================================================================

void Style_InitFileExtensions()
{
    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++) {
        if (StrIsEmpty(g_pLexArray[iLexer]->szExtensions)) {
            StringCchCopy(g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions), g_pLexArray[iLexer]->pszDefExt);
        }
    }
}

//=============================================================================
//
//  Style_InsertThemesMenu()
//
static HMENU s_hmenuThemes = NULL;

bool Style_InsertThemesMenu(HMENU hMenuBar)
{
    if (s_hmenuThemes) {
        DestroyMenu(s_hmenuThemes);
    }
    s_hmenuThemes = CreatePopupMenu();
    //int const pos = GetMenuItemCount(hMenuBar) - 2;

    WCHAR tchThemeName[SMALL_BUFFER] = { L'\0' };
    GetLngString(IDM_THEMES_FACTORY_RESET, tchThemeName, COUNTOF(tchThemeName));
    GetLngString(IDM_THEMES_STD_CFG, Theme_Files[0].szName, COUNTOF(Theme_Files[0].szName));
    AppendMenu(s_hmenuThemes, MF_ENABLED | MF_STRING, IDM_THEMES_FACTORY_RESET, tchThemeName);
    AppendMenu(s_hmenuThemes, MF_SEPARATOR, 0, 0);

    UINT iMaxRID = 0;
    for (unsigned i = 0; i < ThemeItems_CountOf(); ++i) {
        if (Theme_Files[i].rid > 0) {
            iMaxRID = Theme_Files[i].rid;
            AppendMenu(s_hmenuThemes, MF_ENABLED | MF_STRING, iMaxRID, Theme_Files[i].szName);
        } else {
            break; // done
        }
    }

    // --- insert ---
    WCHAR wchMenuItemStrg[128] = { L'\0' };
    GetLngString(IDS_MUI_MENU_THEMES, wchMenuItemStrg, COUNTOF(wchMenuItemStrg));

    //bool const res = InsertMenu(hMenuBar, pos, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)s_hmenuThemes, wchMenuItemStrg);
    bool const res = InsertMenu(hMenuBar, IDM_VIEW_SCHEMECONFIG, MF_BYCOMMAND | MF_POPUP | MF_STRING, (UINT_PTR)s_hmenuThemes, wchMenuItemStrg);

    unsigned const iTheme = Globals.uCurrentThemeIndex;

    CheckMenuRadioItem(hMenuBar, IDM_THEMES_STD_CFG, iMaxRID, IDM_THEMES_STD_CFG + iTheme, MF_BYCOMMAND);

    if (Path_IsEmpty(Theme_Files[iTheme].hStyleFilePath)) {
        EnableCmd(hMenuBar, Theme_Files[iTheme].rid, false);
    }

    return res;
}


//=============================================================================
//
//  Style_DynamicThemesMenuCmd() - Handles IDS_MUI_MENU_THEMES messages
//
// 
bool Style_DynamicThemesMenuCmd(int cmd)
{
    int const iThemeIdx = clampi(cmd - IDM_THEMES_STD_CFG, -1, ThemeItems_CountOf() - 1);  // consecutive IDs, -1 for factory reset

    if (iThemeIdx == (int)Globals.uCurrentThemeIndex) {
        return true;
    }

    if (Settings.SaveSettings) {
        if (Globals.uCurrentThemeIndex == 0) {
            if (!Flags.bSettingsFileSoftLocked) {
                Globals.bCanSaveIniFile = CreateIniFile(Paths.IniFile, NULL);
                if (Globals.bCanSaveIniFile) {
                    Style_ExportToFile(Paths.IniFile, false);
                }
            }
        } else if (Path_IsExistingFile(Theme_Files[Globals.uCurrentThemeIndex].hStyleFilePath)) {
            Style_ExportToFile(Theme_Files[Globals.uCurrentThemeIndex].hStyleFilePath, true);
        }
    }

    ResetIniFileCache();
    bool const result = Style_ImportTheme(iThemeIdx); // -1: factory reset

    if (result) {

        Globals.uCurrentThemeIndex = clampi(iThemeIdx, 0, (int)ThemeItems_CountOf() - 1);
        StringCchCopy(Settings.CurrentThemeName, COUNTOF(Settings.CurrentThemeName), Theme_Files[Globals.uCurrentThemeIndex].szName);

        CheckMenuRadioItem(Globals.hMainMenu, IDM_THEMES_STD_CFG, Theme_Files[ThemesItems_MaxIndex()].rid, 
                                              IDM_THEMES_STD_CFG + Globals.uCurrentThemeIndex, MF_BYCOMMAND);

        if (IsWindow(Globals.hwndDlgCustomizeSchemes)) {
            SendMessage(Globals.hwndDlgCustomizeSchemes, WM_THEMECHANGED, 0, 0);
        } else {
            Style_ResetCurrentLexer(Globals.hwndEdit);
        }
        UpdateMargins(true);
        UpdateUI(Globals.hwndMain);
    }
    return result;
}


//=============================================================================
//
//  IsLexerStandard()
//

inline bool IsLexerStandard(PEDITLEXER pLexer)
{
    return (pLexer && ((pLexer == &lexStandard) || (pLexer == &lexStandard2nd)));
}

inline PEDITLEXER GetCurrentStdLexer()
{
    return (Style_GetUse2ndDefault() ? &lexStandard2nd : &lexStandard);
}

inline bool  IsStyleStandardDefault(PEDITSTYLE pStyle)
{
    return (pStyle && ((pStyle->rid == IDS_LEX_STD_STYLE) || (pStyle->rid == IDS_LEX_2ND_STYLE)));
}

inline bool  IsStyleSchemeDefault(PEDITSTYLE pStyle)
{
    return (pStyle && (pStyle->rid == IDS_LEX_STR_Default));
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
float Style_GetBaseFontSize()
{
    LPCWSTR const lpszStyle = GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue;
    float         fFontSize = GLOBAL_INITIAL_FONTSIZE;
    Style_StrGetSizeFloatEx(lpszStyle, &fFontSize);
    return max_f(0.5f, fFontSize);
}


//=============================================================================
//
//  Style_GetBaseFontSize()
//
float Style_GetCurrentLexerFontSize()
{
    float fFontSize = Style_GetBaseFontSize();
    if (!IsLexerStandard(Style_GetCurrentLexerPtr())) {
        LPCWSTR const lpszStyle = Style_GetCurrentLexerPtr()->Styles[STY_DEFAULT].szValue;
        Style_StrGetSizeFloatEx(lpszStyle, &fFontSize);
    }
    return max_f(0.5f, fFontSize);
}


//=============================================================================
//
//  Style_RgbAlpha() - Simulate Translucency
//
int Style_RgbAlpha(int rgbFore, int rgbBack, int alpha)
{
    alpha = clampi(alpha, SC_ALPHA_TRANSPARENT, SC_ALPHA_OPAQUE);
    return (int)RGB(
               (0xFF - alpha) * (int)GetRValue(rgbBack) / 0xFF + alpha * (int)GetRValue(rgbFore) / 0xFF,
               (0xFF - alpha) * (int)GetGValue(rgbBack) / 0xFF + alpha * (int)GetGValue(rgbFore) / 0xFF,
               (0xFF - alpha) * (int)GetBValue(rgbBack) / 0xFF + alpha * (int)GetBValue(rgbFore) / 0xFF);
}


//=============================================================================
//
//  Style_Import()
//
bool Style_Import(HWND hwnd)
{
    HPATHL         hfile_pth = Path_Allocate(NULL);
    wchar_t* const file_buf = Path_WriteAccessBuf(hfile_pth, CMDLN_LENGTH_LIMIT);

    HSTRINGW       hflt_str = StrgCreate(NULL);
    wchar_t* const flt_buf = StrgWriteAccessBuf(hflt_str, EXTENTIONS_FILTER_BUFFER);

    GetLngString(IDS_MUI_FILTER_INI, flt_buf, (int)StrgGetAllocLength(hflt_str));
    StrgSanitize(hflt_str);

    PrepareFilterStr(flt_buf);

    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = StrgGet(hflt_str);
    ofn.lpstrFile = file_buf;
    ofn.lpstrDefExt = L"ini";
    ofn.nMaxFile = (DWORD)Path_GetBufCount(hfile_pth);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
                | OFN_PATHMUSTEXIST | OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/;

    bool result = false;

    if (GetOpenFileNameW(&ofn)) {
        Path_Sanitize(hfile_pth);
        result = Style_ImportFromFile(hfile_pth);
    }

    StrgDestroy(hflt_str);
    Path_Release(hfile_pth);
    return result;
}


//=============================================================================
//
//   _LoadLexerFileExtensions()
//
static void _LoadLexerFileExtensions()
{
    if (OpenSettingsFile(__func__)) {

        for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++) {

            LPCWSTR Lexer_Section = g_pLexArray[iLexer]->pszName;

            if ((Globals.iCfgVersionRead < CFG_VER_0004) && (iLexer < 2)) {
                Lexer_Section = (iLexer == 0) ? L"Default Text" : L"2nd Default Text";
            }

            IniSectionGetString(Lexer_Section, L"FileNameExtensions", g_pLexArray[iLexer]->pszDefExt,
                g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions));

            // don't allow empty extensions settings => use default ext
            if (StrIsEmpty(g_pLexArray[iLexer]->szExtensions)) {
                StringCchCopy(g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions), g_pLexArray[iLexer]->pszDefExt);
            }

            if (Globals.iCfgVersionRead < CFG_VER_0004) {
                // handling "Text Files" lexer
                if (StringCchCompareXI(L"Text Files", g_pLexArray[iLexer]->pszName) == 0) {
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

        CloseSettingsFile(__func__, false); // read only
    }
}


//=============================================================================
//
//  Style_Prerequisites()
//
void Style_Prerequisites() {

    //_SetBaseFontSize(GLOBAL_INITIAL_FONTSIZE);
    //_SetCurrentFontSize(GLOBAL_INITIAL_FONTSIZE);

    for (int i = 0; i < 16; ++i) {
        g_colorCustom[i] = (UseDarkMode() ? s_colorDarkDefault[i] : s_colorLightDefault[i]);
    }

    _FillThemesMenuTable();
    _LoadLexerFileExtensions();

    ///~ Style_ImportFromFile(Paths.IniFile); ~ done later
}


//=============================================================================
//
//  _DefaultsToTmpCache()
//
static void _DefaultsToTmpCache() {

    ResetTmpCache();

    if (UseDarkMode()) {

        WCHAR wchDefaultStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

        CopyToTmpCache(Globals.pStdDarkModeIniStyles);

        // in case of "pStdDarkModeIniStyles" is incomplete (new Lexer, etc.)
        for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {
            LPCWSTR const Lexer_Section = g_pLexArray[iLexer]->pszName;
            unsigned i = 0;
            while (g_pLexArray[iLexer]->Styles[i].iStyle != -1) {
                LPCWSTR const pszKeyName = g_pLexArray[iLexer]->Styles[i].pszName;
                LPCWSTR const pszDefault = g_pLexArray[iLexer]->Styles[i].pszDefault;
                wchDefaultStyle[0] = L'\0'; // empty
                TmpCacheGetString(Lexer_Section, pszKeyName, L"", wchDefaultStyle, COUNTOF(wchDefaultStyle));
                StrTrim(wchDefaultStyle, L" ;");
                if (StrIsEmpty(wchDefaultStyle) && StrIsNotEmpty(pszDefault)) {
                    TmpCacheSetString(Lexer_Section, pszKeyName, pszDefault);
                }
                ++i;
            }
        }

    } else {

        for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {
            LPCWSTR const Lexer_Section = g_pLexArray[iLexer]->pszName;
            unsigned i = 0;
            while (g_pLexArray[iLexer]->Styles[i].iStyle != -1) {
                LPCWSTR const pszKeyName = g_pLexArray[iLexer]->Styles[i].pszName;
                LPCWSTR const pszDefault = g_pLexArray[iLexer]->Styles[i].pszDefault;
                TmpCacheSetString(Lexer_Section, pszKeyName, pszDefault);
                ++i;
            }
        }
    }
}

static bool _CopyTmpCacheToIniFileCache() {

    WCHAR wchStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {
        LPCWSTR const Lexer_Section = g_pLexArray[iLexer]->pszName;
        unsigned i = 0;
        while (g_pLexArray[iLexer]->Styles[i].iStyle != -1) {
            LPCWSTR const pszKeyName = g_pLexArray[iLexer]->Styles[i].pszName;
            wchStyle[0] = L'\0';
            TmpCacheGetString(Lexer_Section, pszKeyName, L"", wchStyle, COUNTOF(wchStyle));
            IniSectionSetString(Lexer_Section, pszKeyName, wchStyle);
            ++i;
        }
    }
    return true;
}


//=============================================================================
//
//  _ReadFromIniCache()
//
static void _ReadFromIniCache() {

    for (int i = 0; i < 16; i++) {
        g_colorCustom[i] = (UseDarkMode() ? s_colorDarkDefault[i] : s_colorLightDefault[i]); // reset
    }
    const WCHAR *const CustomColors_Section = L"Custom Colors";
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
            g_colorCustom[i] = (UseDarkMode() ? s_colorDarkDefault[i] : s_colorLightDefault[i]);
        }
    }

    // Styles
    const WCHAR *const IniSecStyles = Constants.Styles_Section;

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
    WCHAR wchDefaultStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

    _DefaultsToTmpCache();

    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++) {

        LPCWSTR Lexer_Section = g_pLexArray[iLexer]->pszName;

        if ((Globals.iCfgVersionRead < CFG_VER_0004) && (iLexer < 2)) {
            Lexer_Section = (iLexer == 0) ? L"Default Text" : L"2nd Default Text";
        }

        unsigned i = 0;
        while (g_pLexArray[iLexer]->Styles[i].iStyle != -1) {

            LPCWSTR const pszKeyName = g_pLexArray[iLexer]->Styles[i].pszName;

            wchDefaultStyle[0] = L'\0';
            TmpCacheGetString(Lexer_Section, pszKeyName, L"", wchDefaultStyle, COUNTOF(wchDefaultStyle));
            IniSectionGetString(Lexer_Section, pszKeyName, wchDefaultStyle,
                g_pLexArray[iLexer]->Styles[i].szValue,
                COUNTOF(g_pLexArray[iLexer]->Styles[i].szValue));
            ++i;
        }
    }

    ResetTmpCache();
}


//=============================================================================
//
//  Style_ImportFromFile()
//
bool Style_ImportFromFile(const HPATHL hpath)
{
    bool const bHaveFileResource = Path_IsNotEmpty(hpath);
    bool const bIsStdIniFile = bHaveFileResource ? (Path_StrgComparePath(hpath, Paths.IniFile, Paths.ModuleDirectory) == 0) : false;

    bool result = bIsStdIniFile ? OpenSettingsFile(__func__) : (bHaveFileResource ? LoadIniFileCache(hpath) : true);
    if (result) {
        _ReadFromIniCache();
        result = bIsStdIniFile ? !CloseSettingsFile(__func__, false) /* (!)import only */ : (bHaveFileResource ? ResetIniFileCache() : true);
    }
    return result;
}


//=============================================================================
//
//  Style_ImportTheme()
//
bool Style_ImportTheme(const int iThemeIdx) {

    switch (iThemeIdx) {
    case -1:
        return Style_ImportFromFile(NULL);
    default:
        if ((iThemeIdx >= 0) && (iThemeIdx < (int)ThemeItems_CountOf()) && Path_IsExistingFile(Theme_Files[iThemeIdx].hStyleFilePath)) {
            return Style_ImportFromFile(Theme_Files[iThemeIdx].hStyleFilePath);
        }
        break;
    }
    return false;
}


//=============================================================================
//
//  Style_SaveSettings()
//
void Style_SaveSettings(bool bForceSaveSettings)
{
    if (Settings.SaveSettings || bForceSaveSettings) {
        Style_ExportToFile(Theme_Files[Globals.uCurrentThemeIndex].hStyleFilePath, false);
    }
}


//=============================================================================
//
//  Style_Export()
//
bool Style_Export(HWND hwnd)
{
    HPATHL         hfile_pth = Path_Allocate(NULL);
    wchar_t* const file_buf = Path_WriteAccessBuf(hfile_pth, CMDLN_LENGTH_LIMIT);

    HSTRINGW       hflt_str = StrgCreate(NULL);
    wchar_t* const flt_buf = StrgWriteAccessBuf(hflt_str, EXTENTIONS_FILTER_BUFFER);

    GetLngString(IDS_MUI_FILTER_INI, flt_buf, (int)StrgGetAllocLength(hflt_str));
    StrgSanitize(hflt_str);

    PrepareFilterStr(flt_buf);

    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = StrgGet(hflt_str);
    ofn.lpstrFile = file_buf;
    ofn.lpstrDefExt = L"ini";
    ofn.nMaxFile = (DWORD)Path_GetBufCount(hfile_pth);
    ofn.Flags = /*OFN_FILEMUSTEXIST |*/ OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT
                | OFN_PATHMUSTEXIST | OFN_SHAREAWARE /*| OFN_NODEREFERENCELINKS*/ | OFN_OVERWRITEPROMPT;

    bool result = false;

    if (GetSaveFileNameW(&ofn)) {
        Path_Sanitize(hfile_pth);
        result = Style_ExportToFile(hfile_pth, true);
        if (!result) {
            InfoBoxLng(MB_ICONERROR, NULL, IDS_MUI_EXPORT_FAIL, Path_FindFileName(hfile_pth));
        }
    }

    StrgDestroy(hflt_str);
    Path_Release(hfile_pth);
    return result;
}



//=============================================================================
//
//  Style_FileExtToIniSection()
//
void Style_FileExtToIniSection(bool bForceAll)
{
    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {

        LPCWSTR const Lexer_Section = g_pLexArray[iLexer]->pszName;

        if (bForceAll || (StringCchCompareXI(g_pLexArray[iLexer]->szExtensions, g_pLexArray[iLexer]->pszDefExt) != 0)) {
            IniSectionSetString(Lexer_Section, L"FileNameExtensions", g_pLexArray[iLexer]->szExtensions);
        } else {
            IniSectionDelete(Lexer_Section, L"FileNameExtensions", false);
        }
    }
}


//=============================================================================
//
//  Style_CanonicalSectionToIniCache()
//  create canonical order of lexer sections
//
void Style_CanonicalSectionToIniCache() {
    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++) {
        IniSectionClear(g_pLexArray[iLexer]->pszName, true);
    }
    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); iLexer++) {
        IniSectionSetString(g_pLexArray[iLexer]->pszName, NULL, NULL);
    }
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

bool Style_ToIniSection(bool bForceAll)
{
    // Custom colors
    const WCHAR* const CustomColors_Section = L"Custom Colors";

    for (int i = 0; i < 16; i++) {
        WCHAR tch[32] = { L'\0' };
        StringCchPrintf(tch, COUNTOF(tch), L"%02i", i + 1);
        if ((g_colorCustom[i] != (UseDarkMode() ? s_colorDarkDefault[i] : s_colorLightDefault[i])) || bForceAll) {
            WCHAR wch[32] = { L'\0' };
            ColorToHtmlCode(g_colorCustom[i], wch, COUNTOF(wch));
            IniSectionSetString(CustomColors_Section, tch, wch);
        } else {
            IniSectionDelete(CustomColors_Section, tch, false);
        }
    }

    const WCHAR* const IniSecStyles = Constants.Styles_Section;

    // auto select
    SAVE_STYLE_IF_NOT_EQ_DEFAULT(Bool, Use2ndDefaultStyle, Style_GetUse2ndDefault(), false);

    // default scheme
    SAVE_STYLE_IF_NOT_EQ_DEFAULT(Int, DefaultScheme, s_iDefaultLexer, Constants.StdDefaultLexerID);

    // auto select
    SAVE_STYLE_IF_NOT_EQ_DEFAULT(Bool, AutoSelect, s_bAutoSelect, true);

    // scheme select dlg dimensions
    SAVE_STYLE_IF_NOT_EQ_DEFAULT(Int, SelectDlgSizeX, s_cxStyleSelectDlg, STYLESELECTDLG_X);
    SAVE_STYLE_IF_NOT_EQ_DEFAULT(Int, SelectDlgSizeY, s_cyStyleSelectDlg, STYLESELECTDLG_Y);

    if (bForceAll || Globals.bIniFileFromScratch) {
        Style_CanonicalSectionToIniCache();
    }

    WCHAR wchCurrentStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };
    WCHAR wchDefaultStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

    // prepare tmp cache with defaults
    _DefaultsToTmpCache();

    for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {

        LPCWSTR const Lexer_Section = g_pLexArray[iLexer]->pszName;

        unsigned idx = 0;
        while (g_pLexArray[iLexer]->Styles[idx].iStyle != -1) {

            LPCWSTR const pszKeyName = g_pLexArray[iLexer]->Styles[idx].pszName;

            // normalize values for comparison
            wchCurrentStyle[0] = L'\0';
            TmpCacheGetString(Lexer_Section, pszKeyName, L"", wchCurrentStyle, COUNTOF(wchCurrentStyle));
            wchDefaultStyle[0] = L'\0';
            Style_CopyStyles_IfNotDefined(wchCurrentStyle, wchDefaultStyle, COUNTOF(wchDefaultStyle));

            wchCurrentStyle[0] = L'\0';
            LPCWSTR const pszValue = g_pLexArray[iLexer]->Styles[idx].szValue;
            Style_CopyStyles_IfNotDefined(pszValue, wchCurrentStyle, COUNTOF(wchCurrentStyle));

            if (bForceAll || (StringCchCompareX(wchCurrentStyle, wchDefaultStyle) != 0)) {
                if (StrIsNotEmpty(wchCurrentStyle)) {
                    IniSectionSetString(Lexer_Section, pszKeyName, wchCurrentStyle);
                } else {
                    IniSectionDelete(Lexer_Section, pszKeyName, false);
                }
            } else {
                IniSectionDelete(Lexer_Section, pszKeyName, false);
            }
            ++idx;
        }
    }

    ResetTmpCache();

    // cleanup old (< v4) stuff
    IniSectionDelete(L"Default Text", NULL, true);
    IniSectionDelete(L"2nd Default Text", NULL, true);

    return true;
}


//=============================================================================
//
//  Style_ExportToFile()
//
bool Style_ExportToFile(const HPATHL hpath, bool bForceAll)
{
    if (Path_IsEmpty(hpath)) {
        if (Globals.uCurrentThemeIndex != 0) {
            InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SETTINGSNOTSAVED);
        }
        return false;
    }

    bool const bIsStdIniFile = (Path_StrgComparePath(hpath, Paths.IniFile, Paths.ModuleDirectory) == 0);

    // special handling of standard .ini-file
    bool ok = false;
    if (bIsStdIniFile) {
        if (OpenSettingsFile(__func__)) {
            Style_ToIniSection(bForceAll);
            Style_FileExtToIniSection(bForceAll);
            ok = CloseSettingsFile(__func__, true);
        }
    } else {
        HPATHL hpth_tmp = Path_Copy(hpath);
        Path_NormalizeEx(hpth_tmp, Paths.WorkingDirectory, true, false);
        if (Path_IsNotEmpty(hpth_tmp)) {
            if (!Path_IsExistingFile(hpth_tmp)) {
                HANDLE hFile = CreateFile(Path_Get(hpth_tmp),
                                          GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (IS_VALID_HANDLE(hFile)) {
                    CloseHandle(hFile); // done
                }
            }
            ResetIniFileCache();
            if (LoadIniFileCache(hpth_tmp)) {
                Style_ToIniSection(bForceAll);
                Style_FileExtToIniSection(bForceAll);
                ok = SaveIniFileCache(hpth_tmp);
                ResetIniFileCache();
            }
        }
        Path_Release(hpth_tmp);
    }
    return ok;
}


//=============================================================================
//
//  Style_StrHasAttribute()
//
// zufuliu: parse a style attribute separated by ';'
// e.g.: 'bold', ' bold;', '; bold ' and '; bold;'
//
static bool Style_StrHasAttributeEx(LPCWSTR lpszStyle, LPCWSTR key, const size_t keyLen)
{
    LPCWSTR p = StrStrI(lpszStyle, key);
    while (p) {
        WCHAR chPrev = (p == lpszStyle) ? L';' : p[-1];
        if (chPrev == L' ') {
            LPCWSTR t = p - 2;
            while ((t > lpszStyle) && (*t == L' ')) {
                --t;
            }
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

static __forceinline bool Style_StrHasAttribute(LPCWSTR lpszStyle, LPCWSTR name) {
    return Style_StrHasAttributeEx(lpszStyle, name, StringCchLen(name, 0));
}


//=============================================================================
//
//  Style_SetLexer()
//
void Style_SetLexer(HWND hwnd, PEDITLEXER pLexNew)
{
    // Select standard if NULL is specified
    if (!pLexNew) {
        pLexNew = Flags.bHugeFileLoadState ? GetLargeFileLexer() : GetDefaultLexer();
        if (IsLexerStandard(pLexNew)) {
            pLexNew = GetCurrentStdLexer();
        }
    }
    bool const bFocusedView = FocusedView.HideNonMatchedLines;
    if (bFocusedView) {
        EditToggleView(Globals.hwndEdit);
    }

    //~ ! dont check for (pLexNew == s_pLexCurrent) <= "reapply current lexer"
    //~ assert(pLexNew != s_pLexCurrent);

    BeginWaitCursorUID(Flags.bHugeFileLoadState, IDS_MUI_SB_LEXER_STYLING);

    int const idleStylingMode = SciCall_GetIdleStyling();
    SciCall_SetIdleStyling(SC_IDLESTYLING_ALL);

    // first set standard lexer's default values
    const PEDITLEXER pCurrentStandard = (IsLexerStandard(pLexNew)) ? pLexNew : GetCurrentStdLexer();

    // Set Lexer
    SciCall_SetILexer(CreateLexer(pLexNew->lexerName));

#ifdef _DEBUG
    int const iNewLexer = SciCall_GetLexer();
    if ((pLexNew->lexerID > SCLEX_NULL) && (iNewLexer != pLexNew->lexerID)) {
        WCHAR msg[256] = { L'\0' };
        StringCchPrintf(msg, COUNTOF(msg), L"Failed to set desired Lexer (#%i), got Lexer #%i!", pLexNew->lexerID, iNewLexer);
        MsgBoxLastError(msg, ERROR_DLL_INIT_FAILED);
    }
#endif

    // Lexer very specific styles
    Lexer_SetLexerSpecificProperties(pLexNew->lexerID);

    // Code folding
    Lexer_SetFoldingAvailability(pLexNew);
    Lexer_SetFoldingProperties(FocusedView.CodeFoldingAvailable);

    // Add KeyWord Lists
    for (int i = 0; i <= KEYWORDSET_MAX; ++i) {
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

    // ---  apply current scheme specific settings to default style  ---
    WCHAR wchStylesBuffer[BUFSIZE_STYLE_VALUE] = { L'\0' };
    // set common defaults
    StringCchCopy(wchStylesBuffer, COUNTOF(wchStylesBuffer), pLexNew->Styles[STY_DEFAULT].szValue);
    // merge lexer default styles
    Style_CopyStyles_IfNotDefined(pCurrentStandard->Styles[STY_DEFAULT].szValue, wchStylesBuffer, COUNTOF(wchStylesBuffer));

    // apply default settings
    float fBaseFontSize = IsLexerStandard(pLexNew) ? GLOBAL_INITIAL_FONTSIZE : Style_GetBaseFontSize();
    Style_SetStyles(hwnd, STYLE_DEFAULT, wchStylesBuffer, fBaseFontSize);
    Style_StrGetSizeFloatEx(wchStylesBuffer, &fBaseFontSize); // get scheme base font size

    // Broadcast STYLE_DEFAULT as base style to all other styles
    SciCall_StyleClearAll();

    if (IsLexerStandard(pLexNew)) {
        EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CURRENTSCHEME, true);
    }
    else {
        EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_CURRENTSCHEME, !IsWindow(Globals.hwndDlgCustomizeSchemes));
    }

    // --------------------------------------------------------------------------

    bool     bFlag;
    int      iValue;
    COLORREF dColor;
    WCHAR    wch[64] = { L'\0' };
    wchStylesBuffer[0] = L'\0';

    // margin (line number, bookmarks, folding) style
    Style_SetMargin(hwnd, pCurrentStandard->Styles[STY_MARGIN].szValue);

    if (Settings2.UseOldStyleBraceMatching) {
        Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_OK].iStyle,
            pCurrentStandard->Styles[STY_BRACE_OK].szValue, fBaseFontSize); // brace light
    }
    else {
        if (Style_StrGetColor(pCurrentStandard->Styles[STY_BRACE_OK].szValue, FOREGROUND_LAYER, &dColor, NULL, false)) {
            SciCall_IndicSetFore(INDIC_NP3_MATCH_BRACE, dColor);
        }
        if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_OK].szValue, &iValue, SC_ALPHA_OPAQUE, true)) {
            SciCall_IndicSetAlpha(INDIC_NP3_MATCH_BRACE, iValue);
        }
        if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_OK].szValue, &iValue, SC_ALPHA_OPAQUE, false)) {
            SciCall_IndicSetOutlineAlpha(INDIC_NP3_MATCH_BRACE, iValue);
        }

        iValue = -1; // need for retrieval
        if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_BRACE_OK].szValue, 0, &iValue)) {
            // got default, get string
            StringCchCatW(pCurrentStandard->Styles[STY_BRACE_OK].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
            Style_GetIndicatorType(wchStylesBuffer, COUNTOF(wchStylesBuffer), &iValue);
            StringCchCatW(pCurrentStandard->Styles[STY_BRACE_OK].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchStylesBuffer);
        }

        SciCall_IndicSetStyle(INDIC_NP3_MATCH_BRACE, iValue);

        if (Style_StrGetStrokeWidth(hwnd, INDIC_NP3_MATCH_BRACE, pCurrentStandard->Styles[STY_BRACE_OK].szValue, &iValue)) {
            SciCall_IndicSetStrokeWidth(INDIC_NP3_MATCH_BRACE, iValue);
        }
    }
    if (Settings2.UseOldStyleBraceMatching) {
        Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_BAD].iStyle,
            pCurrentStandard->Styles[STY_BRACE_BAD].szValue, fBaseFontSize); // brace bad
    }
    else {
        if (Style_StrGetColor(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, FOREGROUND_LAYER, &dColor, NULL, false)) {
            SciCall_IndicSetFore(INDIC_NP3_BAD_BRACE, dColor);
        }
        if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, &iValue, SC_ALPHA_OPAQUE, true)) {
            SciCall_IndicSetAlpha(INDIC_NP3_BAD_BRACE, iValue);
        }
        if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, &iValue, SC_ALPHA_OPAQUE, false)) {
            SciCall_IndicSetOutlineAlpha(INDIC_NP3_BAD_BRACE, iValue);
        }

        iValue = -1; // need for retrieval
        if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, 0, &iValue)) {
            // got default, get string
            StringCchCatW(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
            Style_GetIndicatorType(wchStylesBuffer, COUNTOF(wchStylesBuffer), &iValue);
            StringCchCatW(pCurrentStandard->Styles[STY_BRACE_BAD].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchStylesBuffer);
        }

        SciCall_IndicSetStyle(INDIC_NP3_BAD_BRACE, iValue);

        if (Style_StrGetStrokeWidth(hwnd, INDIC_NP3_BAD_BRACE, pCurrentStandard->Styles[STY_BRACE_BAD].szValue, &iValue)) {
            SciCall_IndicSetStrokeWidth(INDIC_NP3_BAD_BRACE, iValue);
        }
    }

    // Occurrences Marker
    if (!Style_StrGetColor(pCurrentStandard->Styles[STY_MARK_OCC].szValue, FOREGROUND_LAYER, &dColor, NULL, false)) {
        dColor = GetSysColor(COLOR_HIGHLIGHT);
        WCHAR sty[32] = { L'\0' };
        Style_PrintfCchColor(sty, COUNTOF(sty), L"; ", FOREGROUND_LAYER, dColor);
        StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), sty);
    }
    SciCall_IndicSetFore(INDIC_NP3_MARK_OCCURANCE, dColor);

    if (!Style_StrGetAlpha(pCurrentStandard->Styles[STY_MARK_OCC].szValue, &iValue, SC_ALPHA_OPAQUE, true)) {
        iValue = 60; // force
        StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; alpha:60");
    }
    SciCall_IndicSetAlpha(INDIC_NP3_MARK_OCCURANCE, iValue);

    if (!Style_StrGetAlpha(pCurrentStandard->Styles[STY_MARK_OCC].szValue, &iValue, SC_ALPHA_OPAQUE, false)) {
        iValue = 60; // force
        StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; alpha2:60");
    }
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_MARK_OCCURANCE, iValue);

    iValue = -1; // need for retrieval
    if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_MARK_OCC].szValue, 0, &iValue)) {
        // got default, get string
        StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
        Style_GetIndicatorType(wchStylesBuffer, COUNTOF(wchStylesBuffer), &iValue);
        StringCchCat(pCurrentStandard->Styles[STY_MARK_OCC].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchStylesBuffer);
    }

    SciCall_IndicSetStyle(INDIC_NP3_MARK_OCCURANCE, iValue);

    if (Style_StrGetStrokeWidth(hwnd, INDIC_NP3_MARK_OCCURANCE, pCurrentStandard->Styles[STY_MARK_OCC].szValue, &iValue)) {
        SciCall_IndicSetStrokeWidth(INDIC_NP3_MARK_OCCURANCE, iValue);
    }

    // --------------------------------------------------------------
    // COLOR definitions (INDIC_NP3_COLOR_DEF) are not configurable
    // --------------------------------------------------------------

    // Unicode-Point Indicator (Hover)
    //SciCall_IndicSetFore(INDIC_NP3_UNICODE_POINT, RGB(0x00, 0x00, 0xF0));
    SciCall_IndicSetStyle(INDIC_NP3_UNICODE_POINT, INDIC_COMPOSITIONTHIN); // simple underline

    if (Style_StrGetStrokeWidth(hwnd, INDIC_NP3_UNICODE_POINT, pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, &iValue)) {
        SciCall_IndicSetStrokeWidth(INDIC_NP3_UNICODE_POINT, iValue);
    }

    if (Style_StrGetColor(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, FOREGROUND_LAYER, &dColor, NULL, false)) {
        SciCall_IndicSetHoverFore(INDIC_NP3_UNICODE_POINT, dColor);
    }
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, &iValue, SC_ALPHA_OPAQUE, true)) {
        SciCall_IndicSetAlpha(INDIC_NP3_UNICODE_POINT, iValue);
    }
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, &iValue, SC_ALPHA_OPAQUE, false)) {
        SciCall_IndicSetOutlineAlpha(INDIC_NP3_UNICODE_POINT, iValue);
    }

    iValue = -1; // need for retrieval
    if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, 0, &iValue)) {
        // got default, get string
        StringCchCatW(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
        Style_GetIndicatorType(wchStylesBuffer, COUNTOF(wchStylesBuffer), &iValue);
        StringCchCatW(pCurrentStandard->Styles[STY_UNICODE_HOTSPOT].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchStylesBuffer);
    }
    SciCall_IndicSetHoverStyle(INDIC_NP3_UNICODE_POINT, iValue);

    // Multi Edit Indicator
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, FOREGROUND_LAYER, &dColor, NULL, false)) {
        SciCall_IndicSetFore(INDIC_NP3_MULTI_EDIT, dColor);
    }
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, &iValue, SC_ALPHA_OPAQUE, true)) {
        SciCall_IndicSetAlpha(INDIC_NP3_MULTI_EDIT, iValue);
    }
    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, &iValue, SC_ALPHA_OPAQUE, false)) {
        SciCall_IndicSetOutlineAlpha(INDIC_NP3_MULTI_EDIT, iValue);
    }

    iValue = -1; // need for retrieval
    if (!Style_GetIndicatorType(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, 0, &iValue)) {
        // got default, get string
        StringCchCatW(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), L"; ");
        Style_GetIndicatorType(wchStylesBuffer, COUNTOF(wchStylesBuffer), &iValue);
        StringCchCatW(pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, COUNTOF(pCurrentStandard->Styles[0].szValue), wchStylesBuffer);
    }

    SciCall_IndicSetStyle(INDIC_NP3_MULTI_EDIT, iValue);

    if (Style_StrGetStrokeWidth(hwnd, INDIC_NP3_MULTI_EDIT, pCurrentStandard->Styles[STY_MULTI_EDIT].szValue, &iValue)) {
        SciCall_IndicSetStrokeWidth(INDIC_NP3_MULTI_EDIT, iValue);
    }

// Inline-IME Color
#define _SC_INDIC_IME_INPUT (INDIC_IME + 0)
#define _SC_INDIC_IME_TARGET (INDIC_IME + 1)
#define _SC_INDIC_IME_CONVERTED (INDIC_IME + 2)
#define _SC_INDIC_IME_UNKNOWN INDIC_IME_MAX

    COLORREF rgb = RGB(0xFF, 0xA0, 0x00);
    COLORREF rgbWrt = RGB(0xFF, 0xA0, 0x00);
    Style_StrGetColor(pCurrentStandard->Styles[STY_IME_COLOR].szValue, FOREGROUND_LAYER, &rgb, NULL, true); // IME foregr
    SciCall_IndicSetFore(_SC_INDIC_IME_INPUT, rgb);
    SciCall_IndicSetFore(_SC_INDIC_IME_TARGET, rgb);
    SciCall_IndicSetFore(_SC_INDIC_IME_CONVERTED, rgb);
    SciCall_IndicSetFore(_SC_INDIC_IME_UNKNOWN, rgb);

    if (pLexNew != &lexANSI) {
        Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_CTRL_CHR].iStyle, pCurrentStandard->Styles[STY_CTRL_CHR].szValue, fBaseFontSize); // control char
    }
    Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_INDENT_GUIDE].iStyle, pCurrentStandard->Styles[STY_INDENT_GUIDE].szValue, fBaseFontSize); // indent guide

    if (Style_StrGetColor(pCurrentStandard->Styles[STY_SEL_TXT].szValue, FOREGROUND_LAYER, &rgb, NULL, false)) { // selection fore
        SciCall_SetElementColour(SC_ELEMENT_SELECTION_TEXT, RGB2RGBAREF(rgb));
        SciCall_SetElementColour(SC_ELEMENT_SELECTION_ADDITIONAL_TEXT, RGB2RGBAREF(rgb));
    }
    else {
        SciCall_ResetElementColour(SC_ELEMENT_SELECTION_TEXT);
        SciCall_ResetElementColour(SC_ELEMENT_SELECTION_ADDITIONAL_TEXT);
    }

    rgb = RGB(0xC0, 0xC0, 0xC0);
    SciCall_SetSelectionLayer(SC_LAYER_UNDER_TEXT); // selection back
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_SEL_TXT].szValue, BACKGROUND_LAYER, &rgb, NULL, true)) {
        Style_StrGetAlpha(pCurrentStandard->Styles[STY_SEL_TXT].szValue, &iValue, SC_ALPHA_OPAQUE, true);
        SciCall_SetElementColour(SC_ELEMENT_SELECTION_BACK, AxRGB(iValue, rgb));
        SciCall_SetElementColour(SC_ELEMENT_SELECTION_ADDITIONAL_BACK, AxRGB(iValue * 2 / 3, rgb));
    }
    else {
        SciCall_ResetElementColour(SC_ELEMENT_SELECTION_BACK);
        SciCall_ResetElementColour(SC_ELEMENT_SELECTION_ADDITIONAL_BACK);
    }

    // AutoCompletion List
    SciCall_SetElementColour(SC_ELEMENT_LIST, RGB2RGBAREF(GetModeTextColor(UseDarkMode())));
    SciCall_SetElementColour(SC_ELEMENT_LIST_BACK, RGB2RGBAREF(GetModeBkColor(UseDarkMode())));
    //SciCall_SetElementColour(SC_ELEMENT_LIST_SELECTED, SciCall_GetElementBaseColour(SC_ELEMENT_LIST_SELECTED));
    //SciCall_SetElementColour(SC_ELEMENT_LIST_SELECTED_BACK, SciCall_GetElementBaseColour(SC_ELEMENT_LIST_SELECTED_BACK));


    // selection eol filled
    bFlag = Style_StrHasAttribute(pCurrentStandard->Styles[STY_SEL_TXT].szValue, FontEffects[FE_EOLFILLED]);
    SciCall_SetSelEOLFilled(bFlag);


    // Nonprinting characters
    if (SciCall_GetTechnology() == SC_TECHNOLOGY_DEFAULT) {
        SciCall_ClearAllRepresentations();
    }
    else {
        SciCall_SetRepresentation("\r", "\xE2\x86\x90");
        SciCall_SetRepresentationAppearance("\r", SC_REPRESENTATION_COLOUR);
        SciCall_SetRepresentation("\n", "\xE2\x86\x93");
        SciCall_SetRepresentationAppearance("\n", SC_REPRESENTATION_COLOUR);
        SciCall_SetRepresentation("\r\n", "\xE2\x86\xB2"); // "\xE2\xAE\x92"
        SciCall_SetRepresentationAppearance("\r\n", SC_REPRESENTATION_COLOUR);
    }

    // whitespace dot size
    wchStylesBuffer[0] = L'\0'; // empty

    iValue = 2; // default whitespace size
    if (Style_StrGetSizeInt(pCurrentStandard->Styles[STY_WHITESPACE].szValue, &iValue)) {
        iValue = clampi(iValue, 1, 12);
        StringCchPrintf(wchStylesBuffer, COUNTOF(wchStylesBuffer), L"size:%i", iValue);
    }
    //@@@SciCall_SetWhiteSpaceSize(MulDiv(iValue, SciCall_GetZoom(), 100)); // needs update on zoom
    SciCall_SetWhiteSpaceSize(iValue);

    // whitespace colors
    rgb = RGB(0, 0, 0);
    rgbWrt = rgb;
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_WHITESPACE].szValue, FOREGROUND_LAYER, &rgb, &rgbWrt, false)) {
        Style_PrintfCchColor(wch, COUNTOF(wch), L"; ", FOREGROUND_LAYER, rgbWrt);
        StringCchCat(wchStylesBuffer, COUNTOF(wchStylesBuffer), wch);
        if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_WHITESPACE].szValue, &iValue, SC_ALPHA_OPAQUE, true)) {
            StringCchPrintf(wch, COUNTOF(wch), L"; alpha:%i", iValue);
            StringCchCat(wchStylesBuffer, COUNTOF(wchStylesBuffer), wch);
        }
        SciCall_SetElementColour(SC_ELEMENT_WHITE_SPACE, AxRGB(iValue, rgb));
        SciCall_SetRepresentationColour("\r", AxRGB(iValue, rgb));
        SciCall_SetRepresentationColour("\n", AxRGB(iValue, rgb));
        SciCall_SetRepresentationColour("\r\n", AxRGB(iValue, rgb));
    }
    else {
        SciCall_ResetElementColour(SC_ELEMENT_WHITE_SPACE);
        SciCall_SetRepresentationColour("\r", SciCall_GetElementColour(SC_ELEMENT_WHITE_SPACE));
        SciCall_SetRepresentationColour("\n", SciCall_GetElementColour(SC_ELEMENT_WHITE_SPACE));
        SciCall_SetRepresentationColour("\r\n", SciCall_GetElementColour(SC_ELEMENT_WHITE_SPACE));
    }

    rgb = RGB(0, 0, 0);
    rgbWrt = rgb;
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_WHITESPACE].szValue, BACKGROUND_LAYER, &rgb, &rgbWrt, true)) {
        Style_PrintfCchColor(wch, COUNTOF(wch), L"; ", FOREGROUND_LAYER, rgbWrt);
        StringCchCat(wchStylesBuffer, COUNTOF(wchStylesBuffer), wch);

        //~ always opaque, no translucency possible in Win32
        //~if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_WHITESPACE].szValue, &iValue, SC_ALPHA_OPAQUE, false)) {
        //~    StringCchPrintf(wch, COUNTOF(wch), L"; alpha2:%i", iValue);
        //~    StringCchCat(wchStylesBuffer, COUNTOF(wchStylesBuffer), wch);
        //~}
        SciCall_SetElementColour(SC_ELEMENT_WHITE_SPACE_BACK, RGB2RGBAREF(rgb));
    }
    else {
        SciCall_ResetElementColour(SC_ELEMENT_WHITE_SPACE_BACK);
    }

    StrTrim(wchStylesBuffer, L" ;");
    StringCchCopy(pCurrentStandard->Styles[STY_WHITESPACE].szValue,
        COUNTOF(pCurrentStandard->Styles[STY_WHITESPACE].szValue), wchStylesBuffer);


    // current line background
    Style_HighlightCurrentLine(hwnd, Settings.HighlightCurrentLine);

    // Hyperlink (URL) indicators
    Style_SetUrlHotSpot(hwnd);

    // caret style and width
    wchStylesBuffer[0] = L'\0';
    int const ovrstrk_mode = (StrStr(pCurrentStandard->Styles[STY_CARET].szValue, L"ovrblck")) ? CARETSTYLE_OVERSTRIKE_BLOCK : CARETSTYLE_OVERSTRIKE_BAR;

    if (StrStr(pCurrentStandard->Styles[STY_CARET].szValue, L"block")) {
        StringCchCat(wchStylesBuffer, COUNTOF(wchStylesBuffer), L"; block");
        SciCall_SetCaretStyle(CARETSTYLE_BLOCK | ovrstrk_mode);
    } else {
        SciCall_SetCaretStyle(CARETSTYLE_LINE | ovrstrk_mode);

        iValue = 1; // don't allow invisible 0
        if (Style_StrGetSizeInt(pCurrentStandard->Styles[STY_CARET].szValue, &iValue)) {
            iValue = clampi(iValue, 1, 20);
            if (iValue != 1) {
                StringCchPrintf(wch, COUNTOF(wch), L"; size:%i", iValue);
                StringCchCat(wchStylesBuffer, COUNTOF(wchStylesBuffer), wch);
            }
        }
        SciCall_SetCaretWidth(iValue);
    }

    if (CARETSTYLE_OVERSTRIKE_BLOCK == ovrstrk_mode) {
        StringCchCat(wchStylesBuffer, COUNTOF(wchStylesBuffer), L"; ovrblck");
    }

    if (StrStr(pCurrentStandard->Styles[STY_CARET].szValue,L"noblink")) {
        SciCall_SetCaretPeriod(0);
        SciCall_SetAdditionalCaretsBlink(false);
        StringCchCat(wchStylesBuffer,COUNTOF(wchStylesBuffer),L"; noblink");
    } else {
        const UINT uCaretBlinkTime = GetCaretBlinkTime();
        SciCall_SetCaretPeriod(uCaretBlinkTime);
        SciCall_SetAdditionalCaretsBlink(uCaretBlinkTime != 0);
    }
    // caret fore
    rgb = GetModeTextColor(UseDarkMode());
    rgbWrt = rgb;
    if (Style_StrGetColor(pCurrentStandard->Styles[STY_CARET].szValue, FOREGROUND_LAYER, &rgb, &rgbWrt, false)) {
        Style_PrintfCchColor(wch, COUNTOF(wch), L"; ", FOREGROUND_LAYER, rgbWrt);
        StringCchCat(wchStylesBuffer,COUNTOF(wchStylesBuffer),wch);
    }
    if (!VerifyContrast(rgb, SciCall_StyleGetBack(0))) {
        rgb = SciCall_StyleGetFore(0);
    }

    if (Style_StrGetAlpha(pCurrentStandard->Styles[STY_CARET].szValue, &iValue, SC_ALPHA_OPAQUE, true)) {
        iValue = clampi(iValue, 20, SC_ALPHA_OPAQUE); // no full transparency
        StringCchPrintf(wch, COUNTOF(wch), L"; alpha:%i", iValue);
        StringCchCat(wchStylesBuffer, COUNTOF(wchStylesBuffer), wch);
    }
    SciCall_SetElementColour(SC_ELEMENT_CARET, AxRGB(iValue, rgb));
    SciCall_SetElementColour(SC_ELEMENT_CARET_ADDITIONAL, AxRGB(iValue, RGB(220, 0, 0)));

    StrTrim(wchStylesBuffer, L" ;");
    StringCchCopy(pCurrentStandard->Styles[STY_CARET].szValue,
                  COUNTOF(pCurrentStandard->Styles[STY_CARET].szValue),wchStylesBuffer);

    int edgeColumns[MIDSZ_BUFFER] = { 0 };
    size_t const cnt = ReadVectorFromString(Globals.fvCurFile.wchMultiEdgeLines, edgeColumns, COUNTOF(edgeColumns), 0, LONG_LINES_MARKER_LIMIT, 0, true);
    Style_SetMultiEdgeLine(edgeColumns, cnt);

        
    int iLnSpc = 0;
    if (Style_StrGetSizeIntEx(pCurrentStandard->Styles[STY_X_LN_SPACE].szValue, &iLnSpc)) {
        Style_SetExtraLineSpace(iLnSpc);
    }
    else {
        Style_SetExtraLineSpace(0);
    }

    if (SciCall_GetIndentationGuides() != SC_IV_NONE) {
        Style_SetIndentGuides(hwnd, true);
    }

    // (!) here: global define current lexer (used in subsequent calls)
    // --------------------------------------------------------------------
    s_pLexCurrent = pLexNew;
    // --------------------------------------------------------------------

    if (s_pLexCurrent == &lexANSI) { // special ANSI-Art style

        // margin (line number, bookmarks, folding) style
        StringCchCopy(wchStylesBuffer, COUNTOF(wchStylesBuffer), s_pLexCurrent->Styles[STY_MARGIN].szValue);
        Style_CopyStyles_IfNotDefined(pCurrentStandard->Styles[STY_MARGIN].szValue, wchStylesBuffer, COUNTOF(wchStylesBuffer));
        Style_SetMargin(hwnd, wchStylesBuffer);

        if (Settings2.UseOldStyleBraceMatching) {
            Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_OK].iStyle,
                pCurrentStandard->Styles[STY_BRACE_OK].szValue, fBaseFontSize);

            Style_SetStyles(hwnd, pCurrentStandard->Styles[STY_BRACE_BAD].iStyle,
                pCurrentStandard->Styles[STY_BRACE_BAD].szValue, fBaseFontSize);
        }

        iLnSpc = 0; // do not inherit from base
        if (Style_StrGetSizeIntEx(s_pLexCurrent->Styles[4].szValue, &iLnSpc)) {
            Style_SetExtraLineSpace(iLnSpc);
        }
    }
    else if (s_pLexCurrent == &lexTEXT) {
    
        // margin (line number, bookmarks, folding) style
        StringCchCopy(wchStylesBuffer, COUNTOF(wchStylesBuffer), s_pLexCurrent->Styles[STY_MARGIN].szValue);
        Style_CopyStyles_IfNotDefined(pCurrentStandard->Styles[STY_MARGIN].szValue, wchStylesBuffer, COUNTOF(wchStylesBuffer));
        Style_SetMargin(hwnd, wchStylesBuffer);

        iLnSpc = (SciCall_GetExtraAscent() + SciCall_GetExtraDescent()) >> 1; // inherit from base
        if (Style_StrGetSizeIntEx(s_pLexCurrent->Styles[2].szValue, &iLnSpc)) {
            Style_SetExtraLineSpace(iLnSpc);
        }

    } else if (s_pLexCurrent->lexerID != SCLEX_NULL) {

        Style_FillRelatedStyles(hwnd, s_pLexCurrent);
    }

    // Lexer reserved indicator styles
    switch (s_pLexCurrent->lexerID) {
    case SCLEX_PYTHON:
        SciCall_IndicSetStyle(1, INDIC_BOX);
        SciCall_IndicSetFore(1, RGB(0xBF, 0, 0)); // (light red)
        //SciCall_IndicSetAlpha(1, 40);
        //SciCall_IndicSetOutlineAlpha(1, 100);
        break;

    default:
        //SciCall_IndicSetStyle(0, INDIC_SQUIGGLE);
        //SciCall_IndicSetStyle(1, INDIC_TT);
        //SciCall_IndicSetStyle(2, INDIC_PLAIN);
        //SciCall_IndicSetFore(0, RGB(0, 0x7F, 0)); // (dark green)
        //SciCall_IndicSetFore(1, RGB(0, 0, 0xFF)); // (light blue)
        //SciCall_IndicSetFore(2, RGB(0xBF, 0, 0)); // (light red)
        //for (int sty = 3; sty < INDIC_CONTAINER; ++sty) {
        //    SciCall_IndicSetStyle(sty, INDIC_ROUNDBOX);
        //}
        break;
    }

    Style_SetInvisible(hwnd, false); // set fixed invisible style

    SciCall_SetLayoutCache(SC_CACHE_PAGE); //~SC_CACHE_DOCUMENT ~ memory consumption !
    SciCall_SetPositionCache(SciCall_GetPositionCache()); // clear - default=1024

    SciCall_StartStyling(0);
    //~SciCall_Colourise(0, -1);

    // apply lexer styles
    if (Flags.bHugeFileLoadState) {
        EditUpdateVisibleIndicators();
    } else {
        EditUpdateIndicators(0, -1, false);
    }

    Style_UpdateAllMargins(hwnd, true);

    SciCall_SetIdleStyling(Flags.bHugeFileLoadState ? SC_IDLESTYLING_TOVISIBLE : idleStylingMode);

    if (bFocusedView) {
        EditToggleView(hwnd);
    }

    Sci_ScrollSelectionToView();

    EndWaitCursor();
}


//=============================================================================
//
//  Style_FillRelatedStyles()
//
void Style_FillRelatedStyles(HWND hwnd, const PEDITLEXER pLexer) {

    //bool const bIsLexerStd = IsLexerStandard(pLexer);

    float fBaseFontSize = IsLexerStandard(pLexer) ? GLOBAL_INITIAL_FONTSIZE : Style_GetBaseFontSize();
    Style_StrGetSizeFloatEx(pLexer->Styles[STY_DEFAULT].szValue, &fBaseFontSize);

    // -----------------------------------------------
    int i = 1; // don't re-apply lexer's default style
    // -----------------------------------------------
    while (pLexer->Styles[i].iStyle != -1) {

        // apply MULTI_STYLE() MACRO
        for (int j = 0; j < 4 && (pLexer->Styles[i].iStyle8[j] != 0 || j == 0); ++j) {
            Style_SetStyles(hwnd, pLexer->Styles[i].iStyle8[j], pLexer->Styles[i].szValue, fBaseFontSize);
        }

        if (pLexer->lexerID == SCLEX_HTML && pLexer->Styles[i].iStyle8[0] == SCE_HPHP_DEFAULT) {
            int iRelated[] = { SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE, SCE_HPHP_WORD, SCE_HPHP_HSTRING, SCE_HPHP_SIMPLESTRING, SCE_HPHP_NUMBER,
                SCE_HPHP_OPERATOR, SCE_HPHP_VARIABLE, SCE_HPHP_HSTRING_VARIABLE, SCE_HPHP_COMPLEX_VARIABLE };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if (pLexer->lexerID == SCLEX_HTML && pLexer->Styles[i].iStyle8[0] == SCE_HJ_DEFAULT) {
            int iRelated[] = { SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC, SCE_HJ_KEYWORD, SCE_HJ_WORD, SCE_HJ_DOUBLESTRING,
                SCE_HJ_SINGLESTRING, SCE_HJ_STRINGEOL, SCE_HJ_REGEX, SCE_HJ_NUMBER, SCE_HJ_SYMBOLS };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if (pLexer->lexerID == SCLEX_HTML && pLexer->Styles[i].iStyle8[0] == SCE_HJA_DEFAULT) {
            int iRelated[] = { SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE, SCE_HJA_COMMENTDOC, SCE_HJA_KEYWORD, SCE_HJA_WORD, SCE_HJA_DOUBLESTRING,
                SCE_HJA_SINGLESTRING, SCE_HJA_STRINGEOL, SCE_HJA_REGEX, SCE_HJA_NUMBER, SCE_HJA_SYMBOLS };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if (pLexer->lexerID == SCLEX_HTML && pLexer->Styles[i].iStyle8[0] == SCE_HB_DEFAULT) {
            int iRelated[] = { SCE_HB_COMMENTLINE, SCE_HB_WORD, SCE_HB_IDENTIFIER, SCE_HB_STRING, SCE_HB_STRINGEOL, SCE_HB_NUMBER };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if (pLexer->lexerID == SCLEX_HTML && pLexer->Styles[i].iStyle8[0] == SCE_HBA_DEFAULT) {
            int iRelated[] = { SCE_HBA_COMMENTLINE, SCE_HBA_WORD, SCE_HBA_IDENTIFIER, SCE_HBA_STRING, SCE_HBA_STRINGEOL, SCE_HBA_NUMBER };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if ((pLexer->lexerID == SCLEX_HTML || pLexer->lexerID == SCLEX_XML) && pLexer->Styles[i].iStyle8[0] == SCE_H_SGML_DEFAULT) {
            int iRelated[] = { SCE_H_SGML_COMMAND, SCE_H_SGML_1ST_PARAM, SCE_H_SGML_DOUBLESTRING, SCE_H_SGML_SIMPLESTRING, SCE_H_SGML_ERROR,
                SCE_H_SGML_SPECIAL, SCE_H_SGML_ENTITY, SCE_H_SGML_COMMENT, SCE_H_SGML_1ST_PARAM_COMMENT, SCE_H_SGML_BLOCK_DEFAULT };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if ((pLexer->lexerID == SCLEX_HTML || pLexer->lexerID == SCLEX_XML) && pLexer->Styles[i].iStyle8[0] == SCE_H_CDATA) {
            int iRelated[] = { SCE_HP_START, SCE_HP_DEFAULT, SCE_HP_COMMENTLINE, SCE_HP_NUMBER, SCE_HP_STRING,
                SCE_HP_CHARACTER, SCE_HP_WORD, SCE_HP_TRIPLE, SCE_HP_TRIPLEDOUBLE, SCE_HP_CLASSNAME,
                SCE_HP_DEFNAME, SCE_HP_OPERATOR, SCE_HP_IDENTIFIER, SCE_HPA_START, SCE_HPA_DEFAULT,
                SCE_HPA_COMMENTLINE, SCE_HPA_NUMBER, SCE_HPA_STRING, SCE_HPA_CHARACTER, SCE_HPA_WORD,
                SCE_HPA_TRIPLE, SCE_HPA_TRIPLEDOUBLE, SCE_HPA_CLASSNAME, SCE_HPA_DEFNAME, SCE_HPA_OPERATOR,
                SCE_HPA_IDENTIFIER };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if (pLexer->lexerID == SCLEX_XML && pLexer->Styles[i].iStyle8[0] == SCE_H_CDATA) {
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
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if (pLexer->lexerID == SCLEX_CPP && pLexer->Styles[i].iStyle8[0] == SCE_C_STRING) {
            int iRelated[] = { SCE_C_CHARACTER, SCE_C_STRINGEOL, SCE_C_VERBATIM, SCE_C_STRINGRAW, SCE_C_HASHQUOTEDSTRING };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }

        if (pLexer->lexerID == SCLEX_SQL && pLexer->Styles[i].iStyle8[0] == SCE_SQL_COMMENT) {
            int iRelated[] = { SCE_SQL_COMMENTLINE, SCE_SQL_COMMENTDOC, SCE_SQL_COMMENTLINEDOC, SCE_SQL_COMMENTDOCKEYWORD, SCE_SQL_COMMENTDOCKEYWORDERROR };
            for (int j = 0; j < COUNTOF(iRelated); j++) {
                Style_SetStyles(hwnd, iRelated[j], pLexer->Styles[i].szValue, fBaseFontSize);
            }
        }
        ++i;
    }
}

//=============================================================================
//
//  Style_SetUrlHotSpot()
//
void Style_SetUrlHotSpot(HWND hwnd)
{
    UNREFERENCED_PARAMETER(hwnd);

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

    COLORREF activeFG = RGB(0x00, 0x00, 0xE0);
    Style_StrGetColor(lpszStyleHotSpot, FOREGROUND_LAYER, &activeFG, NULL, false);

    COLORREF inactiveFG = RGB(0x00, 0x60, 0xB0);
    Style_StrGetColor(lpszStyleHotSpot, BACKGROUND_LAYER, &inactiveFG, NULL, false);

    int iValue;
    Style_StrGetAlpha(lpszStyleHotSpot, &iValue, SC_ALPHA_OPAQUE, true);
    SciCall_IndicSetAlpha(INDIC_NP3_HYPERLINK_U, iValue);

    Style_StrGetAlpha(lpszStyleHotSpot, &iValue, SC_ALPHA_OPAQUE, false); // alpha2:
    SciCall_IndicSetOutlineAlpha(INDIC_NP3_HYPERLINK_U, iValue);

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
    UNREFERENCED_PARAMETER(hwnd);
    //SendMessage(hwnd, SCI_FOLDDISPLAYTEXTSETSTYLE, (WPARAM)SC_FOLDDISPLAYTEXT_BOXED, 0);
    //SciCall_MarkerDefine(MARKER_NP3_OCCUR_LINE, SC_MARK_EMPTY);  // occurrences marker
    if (bInvisible) {
        SciCall_StyleSetVisible(Style_GetInvisibleStyleID(), !bInvisible);
    }
}

#if 0
//=============================================================================
//
//  Style_SetReadonly()
//
void Style_SetReadonly(HWND hwnd, bool bReadonly)
{
    UNREFERENCED_PARAMETER(hwnd);
    SciCall_StyleSetChangeable(Style_GetReadonlyStyleID(), !bReadonly);
}
#endif

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
        if (!Style_StrGetColor(GetCurrentStdLexer()->Styles[STY_LONG_LN_MRK].szValue, BACKGROUND_LAYER, &rgb, NULL, false)) { // edge back
            rgb = GetSysColor(COLOR_3DSHADOW);
        }
    } else {
        if (!Style_StrGetColor(GetCurrentStdLexer()->Styles[STY_LONG_LN_MRK].szValue, FOREGROUND_LAYER, &rgb, NULL, false)) { // edge fore
            rgb = GetSysColor(COLOR_3DLIGHT);
        }
    }
    if (Settings.MarkLongLines) {
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
    } else {
        SciCall_SetEdgeMode(EDGE_NONE);
    }
}


//=============================================================================
//
// Style_HighlightCurrentLine()
//
void Style_HighlightCurrentLine(HWND hwnd, int iHiLitCurLn)
{
    SciCall_SetCaretLineFrame(0);
    SciCall_SetCaretLineVisibleAlways(false);

    bool const backgrColor = (iHiLitCurLn == 1);
    LPCWSTR szValue = GetCurrentStdLexer()->Styles[STY_CUR_LN].szValue;

    COLORREF rgb;
    if (!Style_StrGetColor(szValue, (backgrColor ? BACKGROUND_LAYER : FOREGROUND_LAYER), &rgb, NULL, false)) {
        rgb = (backgrColor ? RGB(0xFF, 0xFF, 0x00) : RGB(0xC2, 0xC0, 0xC3));
    }

    int alpha = SC_ALPHA_TRANSPARENT; // full translucent
    if (iHiLitCurLn > 0) {
        Style_StrGetAlpha(GetCurrentStdLexer()->Styles[STY_CUR_LN].szValue, &alpha, 80, true);
        if (!backgrColor) {
            int iFrameSize = 0;
            if (!Style_StrGetSizeInt(szValue, &iFrameSize)) {
                iFrameSize = 2;
            }
            iFrameSize = max_i(1, ScaleIntToDPI(hwnd, iFrameSize));
            SciCall_SetCaretLineFrame(iFrameSize);
        }
    }

    SciCall_SetCaretLineLayer(SC_LAYER_UNDER_TEXT);
    SciCall_SetElementColour(SC_ELEMENT_CARET_LINE_BACK, AxRGB(alpha, rgb));
    SciCall_SetCaretLineVisibleAlways(iHiLitCurLn > 0);
}


//=============================================================================
//
//  Style_UpdateLineNumberMargin()
//
void Style_UpdateLineNumberMargin(const bool bForce)
{
    static bool  bShowLnNums = false;
    static DocLn prevLineCount = -1LL;

    DocLn const currLineCount = SciCall_GetLineCount();

    if (!bForce && (currLineCount == prevLineCount) && (bShowLnNums == Settings.ShowLineNumbers)) {
        return;
    }

    if (Settings.ShowLineNumbers) {
        static char chLines[32] = { '\0' };
        StringCchPrintfA(chLines, COUNTOF(chLines), "_%td", (size_t)currLineCount);
        int const iLineMarginWidthFit = SciCall_TextWidth(STYLE_LINENUMBER, chLines);
        int const iLineMarginWidthNow = SciCall_GetMarginWidthN(MARGIN_SCI_LINENUM);
        if (iLineMarginWidthNow != iLineMarginWidthFit) {
            SciCall_SetMarginWidthN(MARGIN_SCI_LINENUM, iLineMarginWidthFit);
        }
    }
    else {
        SciCall_SetMarginWidthN(MARGIN_SCI_LINENUM, 0);
    }

    bShowLnNums = Settings.ShowLineNumbers;
    prevLineCount = currLineCount;
}


//=============================================================================
//
//  _GetMarkerMarginWidth()
//
static int _GetMarkerMarginWidth(HWND hwnd, LPCWSTR styleStrg, const float fScale)
{
    UNREFERENCED_PARAMETER(hwnd);
    float fSize = (float)SciCall_TextWidth(STYLE_LINENUMBER, "__"); // 2x underscore
    Style_StrGetSizeFloatEx(styleStrg, &fSize);
    return lroundf(fSize * fScale);
}


//=============================================================================
//
//  Style_UpdateBookmarkMargin()
//
void Style_UpdateBookmarkMargin(HWND hwnd)
{
    int const size = _GetMarkerMarginWidth(hwnd, GetCurrentStdLexer()->Styles[STY_BOOK_MARK].szValue, 1.0f);
    SciCall_SetMarginWidthN(MARGIN_SCI_BOOKMRK, (Settings.ShowBookmarkMargin ? size : 0));
}


//=============================================================================
//
//  Style_UpdateChangeHistoryMargin()
//
void Style_UpdateChangeHistoryMargin(HWND hwnd)
{
    int const  size = _GetMarkerMarginWidth(hwnd, GetCurrentStdLexer()->Styles[STY_BOOK_MARK].szValue, 1.0f);
    bool const bShowMargin = (Settings.ChangeHistoryMode & SC_CHANGE_HISTORY_MARKERS);
    SciCall_SetMarginWidthN(MARGIN_SCI_CHGHIST, (bShowMargin ? size : 0));
}


//=============================================================================
//
//  Style_UpdateFoldingMargin()
//
void Style_UpdateFoldingMargin(HWND hwnd, bool bShowMargin)
{
    int const size = _GetMarkerMarginWidth(hwnd, GetCurrentStdLexer()->Styles[STY_BOOK_MARK].szValue, 0.8f);
    SciCall_SetMarginWidthN(MARGIN_SCI_FOLDING, (bShowMargin ? size : 0));
}


//=============================================================================
//
//  UpdateMargins()
//
//
void Style_UpdateAllMargins(HWND hwnd, const bool bForce)
{
    Style_UpdateLineNumberMargin(bForce);
    if (bForce) {
        Style_UpdateBookmarkMargin(hwnd);
        Style_UpdateChangeHistoryMargin(hwnd);
        Style_UpdateFoldingMargin(hwnd, (FocusedView.CodeFoldingAvailable && FocusedView.ShowCodeFolding));
    }
}


//=============================================================================
//
//  Style_SetMargin()
//
void Style_SetMargin(HWND hwnd, LPCWSTR lpszStyle) /// iStyle == STYLE_LINENUMBER
{
    Style_SetStyles(hwnd, STYLE_LINENUMBER, lpszStyle, Style_GetBaseFontSize()); // line numbers

    int      alpha;
    COLORREF colorRead;

    // background
    if (!Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &colorRead, NULL, false)) {
        colorRead = GetModeBtnfaceColor(UseDarkMode());
    }
    COLORREF const clrMarginBack = colorRead; // (=clrLineNumBack)

    // foreground
    if (!Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &colorRead, NULL, false)) {
        colorRead = GetModeTextColor(UseDarkMode());
    }
    Style_StrGetAlpha(lpszStyle, &alpha, SC_ALPHA_OPAQUE, true);
    COLORREF const clrLineNumFore = Style_RgbAlpha(colorRead, clrMarginBack, alpha);

    // ---  Line Numbers  ---
    SciCall_StyleSetFore(STYLE_LINENUMBER, clrLineNumFore);
    SciCall_StyleSetBack(STYLE_LINENUMBER, clrMarginBack);
    SciCall_SetMarginBackN(MARGIN_SCI_LINENUM, clrMarginBack);
    SciCall_SetMarginSensitiveN(MARGIN_SCI_LINENUM, false); /// (!) false: allow selection drag


    // ---  CallTips  ---
    SciCall_CallTipSetBack(clrMarginBack);
    SciCall_CallTipSetFore(RGB(0x80, 0x80, 0x80));
    SciCall_CallTipSetForeHlt(clrLineNumFore);


    // ---  Bookmarks  ---
    LPCWSTR const wchBookMarkStyleStrg = GetCurrentStdLexer()->Styles[STY_BOOK_MARK].szValue;

    colorRead = clrLineNumFore; // bookmark
    Style_StrGetColor(wchBookMarkStyleStrg, FOREGROUND_LAYER, &colorRead, NULL, false);
    COLORREF const clrBookMarkFore = colorRead;

    Style_StrGetAlpha(wchBookMarkStyleStrg, &alpha, SC_ALPHA_OPAQUE, true);
    int const bookmarkAlpha = alpha;

    colorRead = clrMarginBack; // folding signs
    // document background as default:
    Style_StrGetColor(GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue, BACKGROUND_LAYER, &colorRead, NULL, true);
    // if defined, use bookmark background color
    Style_StrGetColor(wchBookMarkStyleStrg, BACKGROUND_LAYER, &colorRead, NULL, false);
    COLORREF const clrFoldMarginBack = colorRead;

    int strokeWidth = FW_DONTCARE;
    if (!Style_StrGetWeightValue(lpszStyle, &strokeWidth)) {
        strokeWidth = FontWeights[FW_IDX_REGULAR].weight;
    }
    strokeWidth >>= 2;

    SciCall_MarkerDefine(MARKER_NP3_BOOKMARK, SC_MARK_VERTICALBOOKMARK); // SC_MARK_BOOKMARK/SC_MARK_SHORTARROW
    SciCall_MarkerSetStrokeWidth(MARKER_NP3_BOOKMARK, strokeWidth);
    SciCall_MarkerSetAlpha(MARKER_NP3_BOOKMARK, bookmarkAlpha);                                  // if drawn in content area
    SciCall_MarkerSetForeTranslucent(MARKER_NP3_BOOKMARK, AxRGB(bookmarkAlpha, clrLineNumFore)); //~clrBookMarkFore
    //~SciCall_MarkerSetBack(MARKER_NP3_BOOKMARK, Style_RgbAlpha(clrBookMarkFore, clrMarginBack, bookmarkAlpha));
    SciCall_MarkerSetBackTranslucent(MARKER_NP3_BOOKMARK, AxRGB(bookmarkAlpha, clrBookMarkFore));

    // occurrence bookmarker
    bool const visible = Settings.MarkOccurrencesBookmark;
    //SciCall_MarkerDefine(MARKER_NP3_OCCURRENCE, visible ? SC_MARK_ARROWS : SC_MARK_BACKGROUND);
    SciCall_MarkerDefine(MARKER_NP3_OCCURRENCE, visible ? SC_MARK_ARROWS : SC_MARK_EMPTY);
    SciCall_MarkerSetStrokeWidth(MARKER_NP3_OCCURRENCE, strokeWidth);
    SciCall_MarkerSetForeTranslucent(MARKER_NP3_OCCURRENCE, RGB2RGBAREF(CalcContrastColor(clrMarginBack, 100)));
    SciCall_MarkerSetBackTranslucent(MARKER_NP3_OCCURRENCE, AxRGB(SC_ALPHA_TRANSPARENT, clrMarginBack));
    //~SciCall_MarkerSetForeSelected(MARKER_NP3_OCCURRENCE, RGB(0,0,220));

    // ---  WordBookMarks  ---
    COLORREF color;
    for (int m = MARKER_NP3_1; m < MARKER_NP3_BOOKMARK; ++m) {
        SciCall_MarkerDefine(m, (Settings.FocusViewMarkerMode & FVMM_LN_BACKGR) ? SC_MARK_BACKGROUND : SC_MARK_BOOKMARK);
        Style_StrGetColor(WordBookMarks[m], BACKGROUND_LAYER, &color, NULL, true);
        SciCall_MarkerSetAlpha(m, bookmarkAlpha); // if drawn in content area
        SciCall_MarkerSetForeTranslucent(m, RGB2RGBAREF(color));
        SciCall_MarkerSetBackTranslucent(m, AxRGB(bookmarkAlpha, color)); // 'alpha' no meaning for SC_MARK_BACKGROUND
    }

    SciCall_SetMarginBackN(MARGIN_SCI_BOOKMRK, clrMarginBack);
    SciCall_SetMarginSensitiveN(MARGIN_SCI_BOOKMRK, true);
    SciCall_SetMarginCursorN(MARGIN_SCI_BOOKMRK, SC_NP3_CURSORHAND);

    // --- Change History ---

    SciCall_SetMarginBackN(MARGIN_SCI_CHGHIST, clrMarginBack);
    SciCall_SetMarginSensitiveN(MARGIN_SCI_CHGHIST, true);

    const WCHAR* const wchChgHistMrkModifiedStyleStrg = GetCurrentStdLexer()->Styles[STY_CHGHIST_MODIFIED].szValue;
    colorRead = clrLineNumFore;
    if (Style_StrGetColor(wchChgHistMrkModifiedStyleStrg, FOREGROUND_LAYER, &colorRead, NULL, false)) {
        SciCall_MarkerSetFore(SC_MARKNUM_HISTORY_MODIFIED, colorRead);
    }
    colorRead = clrMarginBack;
    Style_StrGetColor(wchChgHistMrkModifiedStyleStrg, BACKGROUND_LAYER, &colorRead, NULL, false);
    // also if not defined, use margin backgr
    SciCall_MarkerSetBack(SC_MARKNUM_HISTORY_MODIFIED, colorRead);

    // TODO: alpha/translucent/layer in print mode ?
    //Style_StrGetAlpha(wchChgHistMrkModifiedStyleStrg, &alpha, SC_ALPHA_OPAQUE, true);
    //SciCall_MarkerSetAlpha(SC_MARKNUM_HISTORY_MODIFIED, alpha);
    // COLORREF const rgbAlpha = Style_RgbAlpha(colorRead, clrMarginBack, alpha);
 
    const WCHAR* const wchChgHistMrkSavedStyleStrg = GetCurrentStdLexer()->Styles[STY_CHGHIST_SAVED].szValue;
    colorRead = clrLineNumFore;
    if (Style_StrGetColor(wchChgHistMrkSavedStyleStrg, FOREGROUND_LAYER, &colorRead, NULL, false)) {
        SciCall_MarkerSetFore(SC_MARKNUM_HISTORY_SAVED, colorRead);
    }
    colorRead = clrMarginBack;
    if (Style_StrGetColor(wchChgHistMrkSavedStyleStrg, BACKGROUND_LAYER, &colorRead, NULL, false)) {
        SciCall_MarkerSetBack(SC_MARKNUM_HISTORY_SAVED, colorRead);
    }
    //SciCall_MarkerSetStrokeWidth(SC_MARKNUM_HISTORY_SAVED, strokeWidth);

    const WCHAR* const wchChgHistMrkRev2OrgStyleStrg = GetCurrentStdLexer()->Styles[STY_CHGHIST_REV_TO_ORG].szValue;
    colorRead = clrLineNumFore;
    if (Style_StrGetColor(wchChgHistMrkRev2OrgStyleStrg, FOREGROUND_LAYER, &colorRead, NULL, false)) {
        SciCall_MarkerSetFore(SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN, colorRead);
    }
    colorRead = clrMarginBack;
    if (Style_StrGetColor(wchChgHistMrkRev2OrgStyleStrg, BACKGROUND_LAYER, &colorRead, NULL, false)) {
        SciCall_MarkerSetBack(SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN, colorRead);
    }
    //SciCall_MarkerSetStrokeWidth(SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN, strokeWidth);

    const WCHAR* const wchChgHistMrkRev2ModStyleStrg = GetCurrentStdLexer()->Styles[STY_CHGHIST_REV_TO_MOD].szValue;
    colorRead = clrLineNumFore;
    if (Style_StrGetColor(wchChgHistMrkRev2ModStyleStrg, FOREGROUND_LAYER, &colorRead, NULL, false)) {
        SciCall_MarkerSetFore(SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED, colorRead);
    }
    colorRead = clrMarginBack;
    if (Style_StrGetColor(wchChgHistMrkRev2ModStyleStrg, BACKGROUND_LAYER, &colorRead, NULL, false)) {
        SciCall_MarkerSetBack(SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED, colorRead);
    }
    //SciCall_MarkerSetStrokeWidth(SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED, strokeWidth);

    // ---  Code folding  ---
    
    SciCall_SetMarginSensitiveN(MARGIN_SCI_FOLDING, true);

    int fldStyleMrk = SC_CASE_LOWER;
    Style_StrGetCase(wchBookMarkStyleStrg, &fldStyleMrk);
    if (fldStyleMrk == SC_CASE_UPPER) { // circle style
        SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPEN, SC_MARK_CIRCLEMINUS);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDEREND, SC_MARK_CIRCLEPLUSCONNECTED);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
    } else { // box style
        SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
        SciCall_MarkerDefine(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
    }
    static const int FoldMarkerID[] = {
        SC_MARKNUM_FOLDEROPEN,
        SC_MARKNUM_FOLDER,
        SC_MARKNUM_FOLDERSUB,
        SC_MARKNUM_FOLDERTAIL,
        SC_MARKNUM_FOLDEREND,
        SC_MARKNUM_FOLDEROPENMID,
        SC_MARKNUM_FOLDERMIDTAIL
    };

    colorRead = clrLineNumFore;
    const WCHAR* wchHighlightStyleStrg = GetCurrentStdLexer()->Styles[STY_SEL_TXT].szValue;
    Style_StrGetColor(wchHighlightStyleStrg, FOREGROUND_LAYER, &colorRead, NULL, true);
    COLORREF const fldHiLight = colorRead;

    for (int i = 0; i < COUNTOF(FoldMarkerID); ++i) {
        SciCall_MarkerSetForeTranslucent(FoldMarkerID[i], RGB2RGBAREF(clrFoldMarginBack)); // (!)
        SciCall_MarkerSetBackTranslucent(FoldMarkerID[i], RGB2RGBAREF(clrLineNumFore)); // (!) //~clrBookMarkForeAlpha
        SciCall_MarkerSetBackSelected(FoldMarkerID[i], fldHiLight);
        SciCall_MarkerSetStrokeWidth(FoldMarkerID[i], strokeWidth);
    }
    SciCall_MarkerEnableHighlight(true); // highlight folding block

    // background 
    //~SciCall_SetMarginBackN(MARGIN_SCI_FOLDING, clrMarginBack); // no effect
    SciCall_SetFoldMarginColour(true, clrFoldMarginBack);     // background
    SciCall_SetFoldMarginHiColour(true, clrFoldMarginBack); // (!)


    int const _debug_flags = 0;
    //int const _debug_flags = (SC_FOLDFLAG_LEVELNUMBERS | SC_FOLDFLAG_LINESTATE); // !extend margin width

    int fldStyleLn = 0;
    Style_StrGetCharSet(wchBookMarkStyleStrg, &fldStyleLn);
    switch (fldStyleLn) {
    case 1:
        SciCall_SetFoldFlags(SC_FOLDFLAG_LINEBEFORE_CONTRACTED | _debug_flags);
        break;
    case 2:
        SciCall_SetFoldFlags(SC_FOLDFLAG_LINEBEFORE_CONTRACTED | SC_FOLDFLAG_LINEAFTER_CONTRACTED | _debug_flags);
        break;
    case 3:
        SciCall_SetFoldFlags(SC_FOLDFLAG_NONE | _debug_flags);
        SciCall_SetDefaultFoldDisplayText(" \xE2\x80\xA6 ");
        SciCall_FoldDisplayTextSetStyle(SC_FOLDDISPLAYTEXT_BOXED);
        break;
    default:
        SciCall_SetFoldFlags(SC_FOLDFLAG_LINEAFTER_CONTRACTED | _debug_flags);
        break;
    }

    // set width
    Style_UpdateAllMargins(hwnd, true);
}


//=============================================================================
//
//  Style_SniffShebang()
//
PEDITLEXER  Style_SniffShebang(char* pchText)
{
    if (StrCmpNA(pchText,"#!",2) == 0) {
        char *pch = pchText + 2;
        while (*pch == ' ' || *pch == '\t') {
            pch++;
        }
        while (*pch && *pch != ' ' && *pch != '\t' && *pch != '\r' && *pch != '\n') {
            pch++;
        }
        if ((pch - pchText) >= 3 && StrCmpNA(pch-3,"env",3) == 0) {
            while (*pch == ' ') {
                pch++;
            }
            while (*pch && *pch != ' ' && *pch != '\t' && *pch != '\r' && *pch != '\n') {
                pch++;
            }
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
    if (bCheckNames) {
        int const cch = (int)StringCchLen(lpszMatch, 0);
        if (cch >= 3) {
            for (int iLex = 0; iLex < COUNTOF(g_pLexArray); ++iLex) {
                if (StrCmpNI(g_pLexArray[iLex]->pszName, lpszMatch, cch) == 0) {
                    return (g_pLexArray[iLex]);
                }
            }
        }
    } else if (StrIsNotEmpty(lpszMatch)) {
        for (int iLex = 0; iLex < COUNTOF(g_pLexArray); ++iLex) {
            if (Style_StrHasAttribute(g_pLexArray[iLex]->szExtensions, lpszMatch)) {
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
    if (StrIsNotEmpty(lpszFileName)) {

        char chFilePath[XHUGE_BUFFER] = { '\0' };
        WideCharToMultiByteEx(CP_UTF8, 0, lpszFileName, -1, chFilePath, COUNTOF(chFilePath), NULL, NULL);

        for (int iLex = 0; iLex < COUNTOF(g_pLexArray); ++iLex) {
            const WCHAR *p = g_pLexArray[iLex]->szExtensions;
            do {
                const WCHAR* f = StrChr(p, L'\\');
                const WCHAR* e = f;
                if (f) {
                    e = StrChr(f, L';');
                    if (!e) {
                        e = f + StringCchLen(f, 0);
                    }
                    ++f; // exclude '\'
                    char regexpat[HUGE_BUFFER] = { '\0' };
                    WideCharToMultiByte(CP_UTF8, 0, f, (int)(e-f), regexpat, (int)COUNTOF(regexpat), NULL, NULL);

                    if (OnigRegExFind(regexpat, chFilePath, false, SciCall_GetEOLMode(), NULL) >= 0) {
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
bool Style_HasLexerForExt(const HPATHL hpath)
{
    bool bFound = false;
    LPCWSTR lpszExt = Path_FindExtension(hpath);
    if (StrIsNotEmpty(lpszExt)) {
        if (*lpszExt == L'.') {
            ++lpszExt;
        }
        if (lpszExt && Style_MatchLexer(lpszExt, false)) {
            bFound = true;
        }
    }
    if (!bFound && Path_IsNotEmpty(hpath)) {
        bFound = Style_RegExMatchLexer(Path_FindFileName(hpath));
    }
    return bFound;
}


//=============================================================================
//
//  Style_SetLexerFromFile()
//
bool Style_SetLexerFromFile(HWND hwnd, const HPATHL hpath)
{
    if (Flags.bHugeFileLoadState) {
        Style_SetDefaultLexer(hwnd);
        return true;
    }

    LPCWSTR    lpszExt = Path_FindExtension(hpath);
    bool       bFound = false;
    PEDITLEXER pLexNew = NULL;
    PEDITLEXER pLexSniffed = NULL;

    if ((Globals.fvCurFile.mask & FV_MODE) && Globals.fvCurFile.chMode[0]) {
        PEDITLEXER pLexMode;
        WCHAR wchMode[MICRO_BUFFER] = { L'\0' };
        MultiByteToWideCharEx(Encoding_SciCP, 0, Globals.fvCurFile.chMode, -1, wchMode, MICRO_BUFFER);

        if (!Flags.NoCGIGuess && (StringCchCompareNI(wchMode,COUNTOF(wchMode),L"cgi", CONSTSTRGLEN(L"cgi")) == 0 ||
                                  StringCchCompareNI(wchMode,COUNTOF(wchMode),L"fcgi", CONSTSTRGLEN(L"fcgi")) == 0)) {
            char tchText[256] = { '\0' };
            SciCall_GetText(COUNTOF(tchText) - 1, tchText);
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
            } else {
                pLexMode = Style_MatchLexer(wchMode, true);
                if (pLexMode) {
                    pLexNew = pLexMode;
                    bFound = true;
                }
            }
        }
    }

    LPCWSTR lpszFileName = Path_FindFileName(hpath);

    // check for filename regex match
    if (!bFound && s_bAutoSelect && Path_IsNotEmpty(hpath)) {
        pLexSniffed = Style_RegExMatchLexer(lpszFileName);
        if (pLexSniffed) {
            pLexNew = pLexSniffed;
            bFound = true;
        }
    }

    if (!bFound && s_bAutoSelect && (Path_IsNotEmpty(hpath) && *lpszExt)) {
        if (*lpszExt == L'.') {
            ++lpszExt;
        }

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
        SciCall_GetText(COUNTOF(tchText) - 1, tchText);
        StrTrimA(tchText," \t\n\r");
        if (!Flags.NoCGIGuess) {
            if (tchText[0] == '<') {
                if (StrStrIA(tchText, "<html")) {
                    pLexNew = &lexHTML;
                } else {
                    pLexNew = &lexXML;
                }
                bFound = true;
            } else {
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
    if (IsLexerStandard(pLexNew)) {
        Style_SetDefaultLexer(hwnd);
    } else {
        Style_SetLexer(hwnd, pLexNew);
    }

    return bFound;
}


//=============================================================================
//
//  Style_MaybeBinaryFile()
//
bool Style_MaybeBinaryFile(HWND hwnd, const HPATHL hpath)
{
    UNREFERENCED_PARAMETER(hwnd);
#if 0
    UNREFERENCED_PARAMETER(lpszFile);
#else
    unsigned char buf[5] = { '\0' }; // magic
    SciCall_GetText(COUNTOF(buf) - 1, (char*)buf);
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
    size_t const _min = 5ULL;
    size_t const _max = 6ULL;

    WCHAR lpszExt[32] = { L'\0' };
    StringCchCopyW(lpszExt, COUNTOF(lpszExt), L"|");
    StringCchCopyW(lpszExt, COUNTOF(lpszExt), Path_FindExtension(hpath));
    StringCchCat(lpszExt, COUNTOF(lpszExt), L"|");

    size_t const len = StringCchLen(lpszExt, COUNTOF(lpszExt));
    if (len < _min || len > _max) {
        if (StrStrIW(binaryExt, lpszExt)) {
            return true;
        }
    }
#endif
    return false;
}


//=============================================================================
//
//  Style_SetLexerFromName()
//
void Style_SetLexerFromName(HWND hwnd, const HPATHL hpath, LPCWSTR lpszName)
{
    PEDITLEXER pLexNew = Style_MatchLexer(lpszName, false);
    if (pLexNew) {
        Style_SetLexer(hwnd, pLexNew);
    } else {
        pLexNew = Style_MatchLexer(lpszName, true);
        if (pLexNew) {
            Style_SetLexer(hwnd, pLexNew);
        } else {
            Style_SetLexerFromFile(hwnd, hpath);
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
        Style_SetLexer(hwnd, g_pLexArray[id]);
    }
}


//=============================================================================
//
//  Style_ToggleUse2ndDefault()
//
void Style_ToggleUse2ndDefault(HWND hwnd)
{
    bool const use2ndDefStyle = Style_GetUse2ndDefault();
    Style_SetUse2ndDefault(!use2ndDefStyle); // swap
    if (IsLexerStandard(s_pLexCurrent)) {
        s_pLexCurrent = GetCurrentStdLexer(); // sync
    }
    Style_ResetCurrentLexer(Globals.hwndEdit);
    UNREFERENCED_PARAMETER(hwnd);
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

    DEFAULT_FONT_STYLES const defaultFontStyle = bGlobalDefault ? DFS_GLOBAL : DFS_CURR_LEXER;
    if (Style_SelectFont(hwnd, newStyle, COUNTOF(newStyle), lexerName, styleName, defaultFontStyle)) {
        // set new styles to current lexer's default text
        StringCchCopyW(pLexerDefStyle->szValue, COUNTOF(pLexerDefStyle->szValue), newStyle);
        Style_ResetCurrentLexer(Globals.hwndEdit);
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
    UNREFERENCED_PARAMETER(hwnd);
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
        } else {
            iIndentView = SC_IV_REAL;
        }
    }
    SciCall_SetIndentationGuides(iIndentView);
}


//=============================================================================
//
//  Style_SetExtraLineSpace()
//
void Style_SetExtraLineSpace(int iValue)
{
    int iAscent = 0, iDescent = 0;
    if ((iValue % 2) != 0) {
        iAscent++;
        iValue--;
    }
    iAscent += (iValue >> 1);
    iDescent += (iValue >> 1);
    SciCall_SetExtraAscent(iAscent);
    SciCall_SetExtraDescent(iDescent);
}


//=============================================================================
//
//  Style_GetFileFilterStr()
//
bool Style_GetFileFilterStr(LPWSTR lpszFilter, int cchFilter, LPWSTR lpszDefExt, int cchExt, bool bSaveAs)
{
    ZeroMemory(lpszFilter, cchFilter * sizeof(WCHAR));

    LPCWSTR curExt = Path_FindExtension(Paths.CurrentFile);
    if (StrIsNotEmpty(curExt)) {
        curExt += 1;
    }

    WCHAR filterAll[80] = { L'\0' };
    GetLngString(IDS_MUI_FILTER_ALL, filterAll, COUNTOF(filterAll));

    WCHAR  filterDef[EXTENTIONS_FILTER_BUFFER] = { L'\0' };
    WCHAR ext[64] = { L'\0' };
    WCHAR append[80] = { L'\0' };
    bool bCurExtIncl = false;
    LPWSTR p = Style_GetCurrentLexerPtr()->szExtensions;
    while (p) {
        LPWSTR q = StrChrW(p, L';');
        if (q) {
            StringCchCopyN(ext, COUNTOF(ext), p, (q - p));
            p = q + 1;
        } else {
            StringCchCopy(ext, COUNTOF(ext), p);
            p = q;
        }
        if (StrIsNotEmpty(ext)) {
            if (StringCchCompareXI(ext, curExt) == 0) {
                bCurExtIncl = true;
            }
            if (StrIsNotEmpty(append)) {
                StringCchCat(filterDef, COUNTOF(filterDef), L";");
            } else {
                StringCchCopy(lpszDefExt, cchExt, ext); // first bIsDefined ext is default
            }
            StringCchPrintf(append, COUNTOF(append), L"*.%s", ext);
            StringCchCat(filterDef, COUNTOF(filterDef), append);
        }
    }
    if (!bCurExtIncl && StrIsNotEmpty(curExt)) {
        StringCchPrintf(append, COUNTOF(append), L";*.%s", curExt);
        StringCchCat(filterDef, COUNTOF(filterDef), append);
    }

    if (!bSaveAs) {
        StringCchCat(lpszFilter, cchFilter, filterAll); // 1st for open dlg
    }

    if (StrIsNotEmpty(filterDef)) {
        WCHAR lexerNameLng[80];
        GetLngString(Style_GetCurrentLexerPtr()->resID, lexerNameLng, COUNTOF(lexerNameLng));
        StringCchCat(lpszFilter, cchFilter, lexerNameLng);
        StringCchCat(lpszFilter, cchFilter, L" (");
        StringCchCat(lpszFilter, cchFilter, filterDef);
        StringCchCat(lpszFilter, cchFilter, L")|");
        StringCchCat(lpszFilter, cchFilter, filterDef);
        StringCchCat(lpszFilter, cchFilter, L"|");
    }

    if (StrgIsNotEmpty(Settings2.FileDlgFilters)) {
        StringCchCat(lpszFilter, cchFilter, StrgGet(Settings2.FileDlgFilters));
        StringCchCat(lpszFilter, cchFilter, L"|");
    }

    if (bSaveAs) {
        StringCchCat(lpszFilter, cchFilter, filterAll); // last if save as dlg
    }

    PrepareFilterStr(lpszFilter);
    return true;
}

//=============================================================================

static inline void GetDefaultCodeFont(LPWSTR pwchFontName, int cchFont, int iStartIdx)
{
    pwchFontName[0] = L'\0';

    for (unsigned i = iStartIdx; i < COUNTOF(Settings2.CodeFontPrefPrioList); ++i) {
        LPCWSTR const fontName = Settings2.CodeFontPrefPrioList[i];
        if (IsFontAvailable(fontName)) {
            StringCchCopy(pwchFontName, cchFont, fontName);
            break;
        }
    }
    if (StrIsEmpty(pwchFontName)) {
        // use internal list
        for (unsigned i = iStartIdx; i < COUNTOF(g_CodeFontPrioList); ++i) {
            LPCWSTR const fontName = g_CodeFontPrioList[i];
            if (IsFontAvailable(fontName)) {
                StringCchCopy(pwchFontName, cchFont, fontName);
                break;
            }
        }
    }
    if (StrIsEmpty(pwchFontName)) {
        StringCchCopy(pwchFontName, cchFont, L"Courier New"); // fallback
    }
}


static inline void GetDefaultTextFont(LPWSTR pwchFontName, int cchFont, int iStartIdx)
{
    pwchFontName[0] = L'\0';

    for (unsigned i = iStartIdx; i < COUNTOF(Settings2.TextFontPrefPrioList); ++i) {
        LPCWSTR const fontName = Settings2.TextFontPrefPrioList[i];
        if (IsFontAvailable(fontName)) {
            StringCchCopy(pwchFontName, cchFont, fontName);
            break;
        }
    }
    if (StrIsEmpty(pwchFontName)) {
        // use internal list
        for (unsigned i = iStartIdx; i < COUNTOF(g_TextFontPrioList); ++i) {
            LPCWSTR const fontName = g_TextFontPrioList[i];
            if (IsFontAvailable(fontName)) {
                StringCchCopy(pwchFontName, cchFont, fontName);
                break;
            }
        }
    }
    if (StrIsEmpty(pwchFontName)) {
        WORD _wDTFSize = 9;
        GetThemedDialogFont(pwchFontName, &_wDTFSize);
    }
}


//=============================================================================
//
//  Style_StrGetFontName()
//
bool Style_StrGetFontName(LPCWSTR lpszStyle, LPWSTR lpszFont, int cchFont)
{
    WCHAR *p = StrStr(lpszStyle, L"font:");
    if (p) {
        p += CONSTSTRGLEN(L"font:");
        while (*p == L' ') {
            ++p;
        }
        StringCchCopy(lpszFont, cchFont, p);
        if ((p = StrChr(lpszFont, L';')) != NULL) {
            *p = L'\0';
        }
        TrimSpcW(lpszFont);

        if (StringCchCompareXI(lpszFont, L"Default") == 0) {

            GetDefaultCodeFont(lpszFont, cchFont, 0);

        }
        else if (StringCchStartsWithI(lpszFont, L"$Code")) {

            int idx = 0;
            Str2Int(&lpszFont[CONSTSTRGLEN(L"$Code")], &idx);
            GetDefaultCodeFont(lpszFont, cchFont, idx);

        }
        else if (StringCchStartsWithI(lpszFont, L"$Text")) {

            int idx = 0;
            Str2Int(&lpszFont[CONSTSTRGLEN(L"$Text")], &idx);
            GetDefaultTextFont(lpszFont, cchFont, idx);

        } else if (!IsFontAvailable(lpszFont)) {

            GetDefaultCodeFont(lpszFont, cchFont, 0);

        }
        return true; // font: defined
    }
    return false; // font: not defined
}


//=============================================================================
//
//  Style_StrGetFontQuality()
//
bool Style_StrGetFontQuality(LPCWSTR lpszStyle, LPWSTR lpszQuality, int cchQuality, int* iSciQuality_out)
{
    WCHAR szFontQuality[64] = { L'\0' };

    WCHAR *p = StrStr(lpszStyle, L"smoothing:");
    if (p) {
        p += CONSTSTRGLEN(L"smoothing:");
        while (*p == L' ') {
            ++p;
        }
        StringCchCopy(szFontQuality, COUNTOF(szFontQuality), p);
        if ((p = StrChr(szFontQuality, L';')) != NULL) {
            *p = L'\0';
        }
        TrimSpcW(szFontQuality);

        for (int i = 0; i < COUNTOF(FontQuality); ++i) {
            if (StringCchCompareX(szFontQuality, FontQuality[i].qname) == 0) {
                if (lpszQuality) {
                    StringCchCopy(lpszQuality, cchQuality, FontQuality[i].qname);
                }
                if (iSciQuality_out) {
                    *iSciQuality_out = FontQuality[i].sci_value;
                }
                return true;
            }
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
    WCHAR *p = StrStr(lpszStyle, L"charset:");
    if (p) {
        p += CONSTSTRGLEN(L"charset:");
        int iValue = 0;
        if (Str2Int(p, &iValue)) {
            *i = (int)max_l(SC_CHARSET_ANSI, iValue);
            return true;
        }
    }
    return false;
}


//=============================================================================
//
//  Style_StrGetSizeInt()
//
bool Style_StrGetSizeInt(LPCWSTR lpszStyle, int* i)
{
    if (i) {
        WCHAR* p = StrStr(lpszStyle, L"size:");
        if (p) {
            p += CONSTSTRGLEN(L"size:");
            return Str2Int(p, i);
        }
    }
    return false;
}


//=============================================================================
//
//  Style_StrGetSizeIntEx()
//
bool Style_StrGetSizeIntEx(LPCWSTR lpszStyle, int* i)
{
    if (i) {
        WCHAR* p = StrStr(lpszStyle, L"size:");
        if (p) {
            int   iSign = 0;
            WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
            StringCchCopy(tch, COUNTOF(tch), p + CONSTSTRGLEN(L"size:"));
            if (tch[0] == L'+') {
                iSign = 1;
                tch[0] = L' ';
            }
            else if (tch[0] == L'-') {
                iSign = -1;
                tch[0] = L' ';
            }
            p = StrChr(tch, L';');
            if (p) {
                *p = L'\0';
            }
            TrimSpcW(tch);

            int iValue = 0;
            if (Str2Int(tch, &iValue)) {
                if (iSign == 0) {
                    *i = iValue;
                }
                else {                         // iSign: relative value
                    iValue = (iSign * iValue); // can be negative
                    *i += iValue;
                }
                return true;
            }
        }
    }
    return false;
}


//=============================================================================
//
//  Style_StrGetSizeFloat()
//
bool Style_StrGetSizeFloat(LPCWSTR lpszStyle, float* f)
{
    WCHAR *p = StrStr(lpszStyle, L"size:");
    if (p) {
        p += CONSTSTRGLEN(L"size:");
        return Str2Float(p, f);
    }
    return false;
}


//=============================================================================
//
//  Style_StrGetSizeFloatEx()
//  Adds parsed value to given one if relative (+/-)
//
bool Style_StrGetSizeFloatEx(LPCWSTR lpszStyle, float* f)
{
    WCHAR *p = StrStr(lpszStyle, L"size:");
    if (p) {
        int fSign = 0;
        WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
        StringCchCopy(tch,COUNTOF(tch),p + CONSTSTRGLEN(L"size:"));
        if (tch[0] == L'+') {
            fSign = 1;
            tch[0] = L' ';
        } else if (tch[0] == L'-') {
            fSign = -1;
            tch[0] = L' ';
        }
        p = StrChr(tch, L';');
        if (p) {
            *p = L'\0';
        }
        TrimSpcW(tch);

        float fValue = 0.0;
        if (StrToFloatEx(tch, &fValue)) {
            if (fSign == 0) {
                *f = Round10th(fValue);
            }
            else { // fSign: relative value
                fValue = (fSign * fValue); // can be negative
                *f += Round10th(fValue);
            }
            return true;
        }
    }
    return false;
}



//=============================================================================
//
//  Style_StrGetSizeStr()
//
bool Style_StrGetSizeStr(LPCWSTR lpszStyle, LPWSTR lpszSize, int cchSize)
{
    WCHAR *p = StrStr(lpszStyle, L"size:");
    if (p) {
        WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
        StringCchCopy(tch, COUNTOF(tch), (p + CONSTSTRGLEN(L"size:")));
        p = StrChr(tch, L';');
        if (p) {
            *p = L'\0';
        }
        TrimSpcW(tch);
        // --- normalize ---
        float fValue = 0.0f;
        if (StrToFloatEx(tch, &fValue)) {
            WCHAR wchFloatVal[64];
            fValue = (float)fabs(fValue);
            bool const isZero = (fValue == 0.0f);
            FloatToStr(fValue, wchFloatVal, COUNTOF(wchFloatVal));

            if (tch[0] == L'+') {
                if (!isZero) {
                    StringCchPrintf(lpszSize, cchSize, L"+%s", wchFloatVal);
                } else {
                    return false;
                }
            } else if (tch[0] == L'-') {
                if (!isZero) {
                    StringCchPrintf(lpszSize, cchSize, L"-%s", wchFloatVal);
                } else {
                    return false;
                }
            } else {
                StringCchPrintf(lpszSize, cchSize, L"%s", wchFloatVal);
            }
            return true;
        }
    }
    return false;
}


//=============================================================================
//
//  Style_AppendSizeAttribute()
//
void Style_AppendSizeAttribute(LPWSTR lpszSize, int cchSize, const float fFontSize, const float fBaseFontSize)
{
    WCHAR tch[32] = { L'\0' };
    WCHAR newSize[64] = { L'\0' };

    if (fBaseFontSize > 0.0f) {

        float const fRelSize = (fFontSize - fBaseFontSize);

        if (fRelSize >= 0.0f) {
            FloatToStr(fRelSize, tch, COUNTOF(tch));
            StringCchPrintf(newSize, COUNTOF(newSize), L"; size:+%s", tch);
        } else {
            FloatToStr((0.0f - fRelSize), tch, COUNTOF(tch));
            StringCchPrintf(newSize, COUNTOF(newSize), L"; size:-%s", tch);
        }

    } else { // absolute size

        FloatToStr(fFontSize, tch, COUNTOF(tch));
        StringCchPrintf(newSize, COUNTOF(newSize), L"; size:%s", tch);
    }

    if (StrIsNotEmpty(newSize)) {
        AppendStyle(lpszSize, cchSize, newSize);
    }
}


//=============================================================================
//
//  Style_StrGetWeightValue()
//
bool Style_StrGetWeightValue(LPCWSTR lpszWeight, int* weight)
{
    int fontWeight = FW_DONTCARE;
    for (int i = FW_IDX_THIN; i <= FW_IDX_ULTRADARK; ++i) {
        if (Style_StrHasAttribute(lpszWeight, FontWeights[i].wname)) {
            fontWeight = FontWeights[i].weight;
            break;
        }
    }
    bool const bFoundFW = (fontWeight > FW_DONTCARE);
    if (bFoundFW) {
        *weight = fontWeight;
    }
    return bFoundFW;
}


//=============================================================================
//
//  Style_AppendWeightAttribute()
//
void Style_AppendWeightAttribute(LPWSTR lpszWeight, int cchSize, int fontWeight)
{
    const WCHAR * pFontWeight = NULL;
    int i;
    for (i = FW_IDX_THIN; i <= FW_IDX_ULTRADARK; ++i) {
        if (fontWeight <= FontWeights[i].weight) {
            pFontWeight = FontWeights[i].wname;
            break;
        }
    }
    if (pFontWeight && (i != FW_IDX_REGULAR)) {
        AppendStyle(lpszWeight, cchSize, pFontWeight);
    }
}


//=============================================================================
//
//  Style_StrGetColor()
//
bool Style_StrGetColor(LPCWSTR lpszStyle, COLOR_LAYER layer, COLORALPHAREF* rgba, COLORALPHAREF* rgbaOrig, bool useDefault)
{
    bool const bFGLayer = (layer == FOREGROUND_LAYER);
    //~COLORREF const     colorDefault = bFGLayer ? GetModeTextColor(UseDarkMode()) : GetModeBkColor(UseDarkMode());
    COLORALPHAREF const colorDefault = bFGLayer ? RGB2RGBAREF(SciCall_StyleGetFore(STYLE_DEFAULT)) : 
                                                  RGB2RGBAREF(SciCall_StyleGetBack(STYLE_DEFAULT)); //~ SCI maybe not initialized
    COLORALPHAREF       color = rgbaOrig ? *rgbaOrig : (rgba ? *rgba : colorDefault);
    bool                bIsDefined = false;
    const WCHAR* const  pItem = bFGLayer ? L"fore:" : L"back:";
    WCHAR*              p = StrStr(lpszStyle, pItem);
    if (p) {
        WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
        StringCchCopy(tch, COUNTOF(tch), p + StringCchLen(pItem, 0));
        if (tch[0] == L'#') {
            tch[0] = L' ';
        }
        p = StrChr(tch, L';');
        if (p) {
            *p = L'\0';
        }
        TrimSpcW(tch);

        unsigned __int32 iValue = 0;
        if (swscanf_s(tch, L"%x", &iValue) == 1) {
            color = ARGB((iValue & 0xFF000000) >> 24, (iValue & 0xFF0000) >> 16, (iValue & 0xFF00) >> 8, iValue & 0xFF);
            if (GetAValue(color) == 0) {
                color = RGB2RGBAREF(color); // alpha not defined: assuming opaque
            }
            bIsDefined = true;
        }
    }

    if (!bIsDefined && useDefault) {
        color = colorDefault;
        //~ don't: bIsDefined = true;
    }

    if (rgbaOrig) {
        *rgbaOrig = color;
    }
    if (bFGLayer && UseDarkMode()) {
        color = AxRGB(GetAValue(color), ContrastColor(ARGB_TO_COLREF(color), Settings2.DarkModeHiglightContrast));
    }
    if (rgba) {
        *rgba = color;
    }

    return bIsDefined;
}


//=============================================================================
//
//  Style_StrGetAlpha()
//
bool Style_StrGetAlpha(LPCWSTR lpszStyle, int* iOutValue, const int defAlpha, const bool bAlpha1st)
{
    const WCHAR* strAlpha = bAlpha1st ? L"alpha:" : L"alpha2:";

    WCHAR* p = StrStr(lpszStyle, strAlpha);
    if (p) {
        WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
        StringCchCopy(tch, COUNTOF(tch), p + StringCchLen(strAlpha,0));
        p = StrChr(tch, L';');
        if (p) {
            *p = L'\0';
        }
        TrimSpcW(tch);
        int iValue = 0;
        if (StrToIntEx(tch, STIF_DEFAULT, &iValue)) {
            *iOutValue = Sci_ClampAlpha(iValue);
            return true;
        }
    }
    *iOutValue = Sci_ClampAlpha(defAlpha);
    return false;
}


//=============================================================================
//
//  Style_StrGetStrokeWidth()
//
bool Style_StrGetStrokeWidth(HWND hwnd, int indicID, LPCWSTR lpszStyle, int *piStrokeWidth)
{
    if (HasIndicStyleStrokeWidth(SciCall_IndicGetStyle(indicID))) {
        int iStrkWdth = 1;
        if (Style_StrGetSizeInt(lpszStyle, &iStrkWdth)) {
            iStrkWdth = ScaleIntToDPI(hwnd, iStrkWdth * 100);
            *piStrokeWidth = clampi(iStrkWdth, 100, 64000);
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
//    StringCchCopy(tch, COUNTOF(tch), (p + StringCchLen(lpszProperty,0)));
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
        p += CONSTSTRGLEN(L"case:");
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
bool Style_GetIndicatorType(LPWSTR lpszStyle, int cchSize, int* idx)
{
    if (*idx < 0) { // retrieve indicator style from string
        for (int i = COUNTOF(IndicatorTypes) - 1;  0 <= i;  --i) {
            if (Style_StrHasAttribute(lpszStyle, IndicatorTypes[i])) {
                *idx = i;
                return true;
            }
        }
        *idx = INDIC_ROUNDBOX; // default
    } else { // get indicator string from index
        if (*idx < COUNTOF(IndicatorTypes)) {
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
void Style_CopyStyles_IfNotDefined(LPCWSTR lpszStyleSrc, LPWSTR lpszStyleDest, int cchSizeDest) 
{
    int iValue;
    COLORREF dColor;
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };
    WCHAR szTmpStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

    // ---------   Font settings   ---------

    bool bIsFontDefInDestination = false;

    if (Style_StrGetFontName(lpszStyleDest, tch, COUNTOF(tch))) {
        bIsFontDefInDestination = true;
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), L"font:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    } else if (Style_StrGetFontName(lpszStyleSrc, tch, COUNTOF(tch))) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), L"font:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }

    // ---------  Font Style  ---------

    const WCHAR *pFontWeight = NULL;
    for (int idx = FW_IDX_THIN; idx <= FW_IDX_ULTRADARK; ++idx) {
        if (Style_StrHasAttribute(lpszStyleDest, FontWeights[idx].wname)) {
            pFontWeight = FontWeights[idx].wname;
            break;
        }
    }
    if (!bIsFontDefInDestination && !pFontWeight) {
        for (int idx = FW_IDX_THIN; idx <= FW_IDX_ULTRADARK; ++idx) {
            if (Style_StrHasAttribute(lpszStyleSrc, FontWeights[idx].wname)) {
                pFontWeight = FontWeights[idx].wname;
                break;
            }
        }
    }
    if (pFontWeight) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), pFontWeight);
    }

    if (Style_StrHasAttribute(lpszStyleDest, FontEffects[FE_ITALIC])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), FontEffects[FE_ITALIC]);
    } else if (!bIsFontDefInDestination && Style_StrHasAttribute(lpszStyleSrc, FontEffects[FE_ITALIC])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), FontEffects[FE_ITALIC]);
    }

    // ---------  Size  ---------

    if (Style_StrGetSizeStr(lpszStyleDest, tch, COUNTOF(tch))) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), L"size:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    } else if (!bIsFontDefInDestination && Style_StrGetSizeStr(lpszStyleSrc, tch, COUNTOF(tch))) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), L"size:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }

    if (Style_StrHasAttribute(lpszStyleDest, FontEffects[FE_UNDERLINE])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), FontEffects[FE_UNDERLINE]);
    } else if (!bIsFontDefInDestination && Style_StrHasAttribute(lpszStyleSrc, FontEffects[FE_UNDERLINE])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), FontEffects[FE_UNDERLINE]);
    }

    if (Style_StrHasAttribute(lpszStyleDest, FontEffects[FE_STRIKEOUT])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), FontEffects[FE_STRIKEOUT]);
    } else if (!bIsFontDefInDestination && Style_StrHasAttribute(lpszStyleSrc, FontEffects[FE_STRIKEOUT])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), FontEffects[FE_STRIKEOUT]);
    }

    if (Style_StrGetCharSet(lpszStyleDest, &iValue)) {
        StringCchPrintf(tch, COUNTOF(tch), L"charset:%i", iValue);
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), tch);
    } else if (!bIsFontDefInDestination && Style_StrGetCharSet(lpszStyleSrc, &iValue)) {
        StringCchPrintf(tch, COUNTOF(tch), L"charset:%i", iValue);
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }

    // foreground color
    if (Style_StrGetColor(lpszStyleDest, FOREGROUND_LAYER, NULL, &dColor, false)) {
        Style_PrintfCchColor(tch, COUNTOF(tch), L"; ", FOREGROUND_LAYER, dColor);
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
    else if (Style_StrGetColor(lpszStyleSrc, FOREGROUND_LAYER, NULL, &dColor, false)) {
        Style_PrintfCchColor(tch, COUNTOF(tch), L"; ", FOREGROUND_LAYER, dColor);
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }

    // ########  attributes not defined by Font Selection Dialog  ########

    // background color
    if (Style_StrGetColor(lpszStyleDest, BACKGROUND_LAYER, NULL, &dColor, false)) {
        Style_PrintfCchColor(tch, COUNTOF(tch), L"; ", BACKGROUND_LAYER, dColor);
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
    else if (Style_StrGetColor(lpszStyleSrc, BACKGROUND_LAYER, NULL, &dColor, false)) {
        Style_PrintfCchColor(tch, COUNTOF(tch), L"; ", BACKGROUND_LAYER, dColor);
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }

    int iFontQuality = Settings2.SciFontQuality;
    if (Style_StrGetFontQuality(lpszStyleDest, tch, COUNTOF(tch), &iFontQuality)) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), L"smoothing:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    } else if (!bIsFontDefInDestination && Style_StrGetFontQuality(lpszStyleSrc, tch, COUNTOF(tch), &iFontQuality)) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), L"smoothing:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }

    // ---------  Special Styles  ---------

    if (Style_StrHasAttribute(lpszStyleDest, FontEffects[FE_EOLFILLED])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), FontEffects[FE_EOLFILLED]);
    } else if (Style_StrHasAttribute(lpszStyleSrc, FontEffects[FE_EOLFILLED])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), FontEffects[FE_EOLFILLED]);
    }

    if (Style_StrGetCase(lpszStyleDest, &iValue)) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), L"case:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), (iValue == SC_CASE_UPPER) ? L"U" : L"L");
    } else if (Style_StrGetCase(lpszStyleSrc, &iValue)) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), L"case:");
        StringCchCat(szTmpStyle, COUNTOF(szTmpStyle), (iValue == SC_CASE_UPPER) ? L"U" : L"L");
    }

    if (Style_StrGetAlpha(lpszStyleDest, &iValue, SC_ALPHA_OPAQUE, true)) {
        StringCchPrintf(tch, COUNTOF(tch), L"alpha:%i", iValue);
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
    else if (Style_StrGetAlpha(lpszStyleSrc, &iValue, SC_ALPHA_OPAQUE, true)) {
        StringCchPrintf(tch, COUNTOF(tch), L"alpha:%i", iValue);
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }

    if (Style_StrGetAlpha(lpszStyleDest, &iValue, SC_ALPHA_OPAQUE, false)) {
        StringCchPrintf(tch, COUNTOF(tch), L"alpha2:%i", iValue);
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }
    else if (Style_StrGetAlpha(lpszStyleSrc, &iValue, SC_ALPHA_OPAQUE, false)) {
        StringCchPrintf(tch, COUNTOF(tch), L"alpha2:%i", iValue);
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), tch);
    }

    // --------   Indicator Type   --------

    bool indic_found = false;
    iValue = -1; // find SCI index
    StringCchCopy(tch, COUNTOF(tch), lpszStyleDest);
    if (Style_GetIndicatorType(tch, 0, &iValue)) {
        Style_GetIndicatorType(tch, COUNTOF(tch), &iValue);
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), tch);
        indic_found = true;
    }
    if (!indic_found) {
        iValue = -1; // find SCI index
        StringCchCopy(tch, COUNTOF(tch), lpszStyleSrc);
        if (Style_GetIndicatorType(tch, 0, &iValue)) {
            Style_GetIndicatorType(tch, COUNTOF(tch), &iValue);
            AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), tch);
        }
    }

    // --------   other style settings   --------
    if (Style_StrHasAttribute(lpszStyleDest, CaretStyle[CS_OVRBLCK])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), CaretStyle[CS_OVRBLCK]);
    } else if (Style_StrHasAttribute(lpszStyleSrc, CaretStyle[CS_OVRBLCK])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), CaretStyle[CS_OVRBLCK]);
    }

    if (Style_StrHasAttribute(lpszStyleDest, CaretStyle[CS_BLOCK])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), CaretStyle[CS_BLOCK]);
    } else if (Style_StrHasAttribute(lpszStyleSrc, CaretStyle[CS_BLOCK])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), CaretStyle[CS_BLOCK]);
    }

    if (Style_StrHasAttribute(lpszStyleDest, CaretStyle[CS_NOBLINK])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), CaretStyle[CS_NOBLINK]);
    } else if (Style_StrHasAttribute(lpszStyleSrc, CaretStyle[CS_NOBLINK])) {
        AppendStyle(szTmpStyle, COUNTOF(szTmpStyle), CaretStyle[CS_NOBLINK]);
    }

    // cleanup
    StrTrim(szTmpStyle, L" ;");

    // replace destination by normalized string
    StringCchCopy(lpszStyleDest, cchSizeDest, szTmpStyle);
}

//=============================================================================


//=============================================================================
//
//  Style_SelectFont()
//
bool Style_SelectFont(HWND hwnd, LPWSTR lpszStyle, int cchStyle, LPCWSTR sLexerName, 
                      LPCWSTR sStyleName, DEFAULT_FONT_STYLES styleType)
{

    // Map lpszStyle to LOGFONT
    WCHAR wchDefaultFontName[LF_FACESIZE] = { L'\0' };
    GetDefaultCodeFont(wchDefaultFontName, COUNTOF(wchDefaultFontName), 0);

    // current base style
    const WCHAR *const lpszBaseStyleDefinition = GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue;

    // current common default font name setting
    WCHAR wchCurrCommonFontName[LF_FACESIZE] = { L'\0' };
    if (!Style_StrGetFontName(lpszBaseStyleDefinition, wchCurrCommonFontName, COUNTOF(wchCurrCommonFontName))) {
        StringCchCopy(wchCurrCommonFontName, COUNTOF(wchCurrCommonFontName), wchDefaultFontName);
    }

    // specified font name
    WCHAR wchFontName[LF_FACESIZE] = { L'\0' };
    if (!Style_StrGetFontName(lpszStyle, wchFontName, COUNTOF(wchFontName))) {
        StringCchCopy(wchFontName, COUNTOF(wchFontName), wchCurrCommonFontName);
    }

    // font style
    // NOTE:  To globalists your application, you should specify the style by using
    // the lfWeight and lfItalic members of the LOGFONT structure pointed to by lpLogFont.
    // The style name may change depending on the system user interface language.
    DWORD const flagUseStyle = 0; // = CF_USESTYLE; ~ don't use

    // Font Weight
    int iBaseFontWeight = FontWeights[FW_IDX_REGULAR].weight;
    Style_StrGetWeightValue(lpszBaseStyleDefinition, &iBaseFontWeight);

    int iFontWeight = iBaseFontWeight;
    Style_StrGetWeightValue(lpszStyle, &iFontWeight);

    // Italic / Oblique
    bool const bIsItalic = Style_StrHasAttribute(lpszStyle, FontEffects[FE_ITALIC]);

    // ------------------------------------------------------------------------

    int iCharSet = SC_CHARSET_DEFAULT;
    Style_StrGetCharSet(lpszStyle, &iCharSet);

    // is "size:" definition relative ?
    bool const bRelFontSize = (StrStr(lpszStyle, L"size:+") || StrStr(lpszStyle, L"size:-"));

    // Font Height
    float fFontSize = GLOBAL_INITIAL_FONTSIZE;
    switch (styleType) {
    case DFS_GLOBAL:
        fFontSize = Style_GetBaseFontSize();
        break;
    case DFS_CURR_LEXER:
        fFontSize = Style_GetCurrentLexerFontSize();
        break;
    case DFS_GENERIC_USE:
    default:
        Style_StrGetSizeFloatEx(lpszStyle, &fFontSize);
        break;
    }

    HDC const hdc = GetDC(hwnd);
    int const iFontHeight = PointSizeToFontHeight(fFontSize, hdc);
    ReleaseDC(hwnd, hdc);

    int const iFontStretch = 0; // with calculated automatically
    bool const bIsUnderline = Style_StrHasAttribute(lpszStyle, FontEffects[FE_UNDERLINE]);
    bool const bIsStrikeout = Style_StrHasAttribute(lpszStyle, FontEffects[FE_STRIKEOUT]);

    COLORREF fgColor = 0L;
    Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &fgColor, NULL, true);

    // Font Quality
    WCHAR wchFontQuality[80] = { L'\0' };
    int   iFontQuality = Settings2.SciFontQuality;
    Style_StrGetFontQuality(lpszStyle, wchFontQuality, COUNTOF(wchFontQuality), &iFontQuality);

    // --------------------------------------------------------------------------

    LOGFONT lf = { 0 };
    lf.lfCharSet = (BYTE)iCharSet;
    lf.lfHeight = iFontHeight;
    lf.lfWidth = iFontStretch;
    lf.lfWeight = iFontWeight;
    lf.lfItalic = (BYTE)(BOOL)bIsItalic;
    lf.lfUnderline = (BYTE)(BOOL)bIsUnderline;
    lf.lfStrikeOut = (BYTE)(BOOL)bIsStrikeout;
    lf.lfQuality = (BYTE)MapSciToWinFontQuality(iFontQuality);
    //~lf.lfClipPrecision = (BYTE)CLIP_DEFAULT_PRECIS;
    //~lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;

    StringCchCopy(lf.lfFaceName, LF_FACESIZE, wchFontName); // (!) not LF_FULLFACESIZE

    // --------------------------------------------------------------------------

    // Init cf
    CHOOSEFONT cf = { sizeof(CHOOSEFONT) }; // cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = hwnd;
    cf.hInstance = Globals.hLngResContainer; // Globals.hInstance;

    cf.iPointSize = (INT)f2int(fFontSize * 10.0f);
    cf.rgbColors = fgColor;

    // --- FLAGS ---
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_FORCEFONTEXIST; // | CF_NOSCRIPTSEL

    // use logfont struct
    cf.Flags |= CF_INITTOLOGFONTSTRUCT;
    cf.lpLogFont = (LPLOGFONT)&lf;

    cf.Flags |= (SciCall_GetTechnology() != SC_TECHNOLOGY_DEFAULT) ? CF_SCALABLEONLY : 0;
    cf.Flags |= IsKeyDown(VK_SHIFT) ? CF_FIXEDPITCHONLY : 0;

    // font style (
    cf.Flags |= flagUseStyle; //~ CF_USESTYLE
    cf.lpszStyle = NULL;      //~flagUseStyle ? szStyleStrg : NULL;

    // Font size limits
    cf.Flags |= CF_LIMITSIZE;
    cf.nSizeMin = 4;
    cf.nSizeMax = 128;

    // custom hook for title bar
    cf.Flags |= CF_ENABLEHOOK;
    cf.lpfnHook = (LPCFHOOKPROC)FontDialogHookProc; // Register the callback
    cf.lCustData = (LPARAM)FontSelTitle;

    // Font.dlg resource template
    cf.Flags |= CF_ENABLETEMPLATE;
    cf.lpTemplateName = MAKEINTRESOURCEW(IDD_MUI_SYSFONT_WITHLINK);

    // ------------------------------------------------------------------------

    switch (styleType) {
    case DFS_GLOBAL:
        FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), bRelFontSize ? IDS_MUI_TITLE_RELBASE : IDS_MUI_TITLE_FIXBASE, sStyleName);
        break;
    case DFS_CURR_LEXER:
        FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), bRelFontSize ? IDS_MUI_TITLE_RELCUR : IDS_MUI_TITLE_FIXCUR, sLexerName, sStyleName);
        break;
    case DFS_GENERIC_USE:
    default:
        FormatLngStringW(FontSelTitle, COUNTOF(FontSelTitle), bRelFontSize ? IDS_MUI_TITLE_RELARB : IDS_MUI_TITLE_FIXARB, sStyleName, sLexerName);
        break;
    }

    // ------------------------------------------------------------------------

#if TRUE

    // ---  open systems Font Selection dialog  ---
    if (!ChooseFont(&cf) || StrIsEmpty(lf.lfFaceName)) {
        return false;
    }

#else

    if (Settings.RenderingTechnology > 0) {
        UINT const dpi = Scintilla_GetWindowDPI(hwnd);
        const WCHAR *const localName = Settings2.PreferredLanguageLocaleName;
        if (!ChooseFontDirectWrite(Globals.hwndMain, localName, dpi, &cf) || StrIsEmpty(lf.lfFaceName)) {
            return false;
        }
        // HACK: to get the full font name instead of font family name
        // [see: Style_FontDialogHook() WM_INITDIALOG]
        cf.lCustData = (LPARAM)NULL;
        ChooseFont(&cf);
    } else {
        if (!ChooseFont(&cf) || StrIsEmpty(lf.lfFaceName)) {
            return false;
        }
    }

#endif

    // ---  map back to lpszStyle  ---

    WCHAR szNewStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };

    if (StringCchCompareX(lf.lfFaceName, wchDefaultFontName) == 0) {
        StringCchCopy(szNewStyle, COUNTOF(szNewStyle), L"font:$Code0");
    } else {
        StringCchPrintf(szNewStyle, COUNTOF(szNewStyle), L"font:%s", lf.lfFaceName);
    }

    if (lf.lfWeight != FontWeights[FW_IDX_REGULAR].weight) {
        Style_AppendWeightAttribute(szNewStyle, COUNTOF(szNewStyle), lf.lfWeight);
    }

    if (lf.lfItalic) {
        AppendStyle(szNewStyle, COUNTOF(szNewStyle), FontEffects[FE_ITALIC]);
    }

    float const fNewFontSize = ((float)(cf.iPointSize < 10 ? 10 : cf.iPointSize)) / 10.0f;

    switch (styleType) {
    case DFS_GLOBAL:
        Style_AppendSizeAttribute(szNewStyle, COUNTOF(szNewStyle), fNewFontSize, bRelFontSize ? GLOBAL_INITIAL_FONTSIZE : 0.0f);
        break;
    case DFS_CURR_LEXER:
        Style_AppendSizeAttribute(szNewStyle, COUNTOF(szNewStyle), fNewFontSize, bRelFontSize ? Style_GetBaseFontSize() : 0.0f);
        break;
    case DFS_GENERIC_USE:
    default:
        Style_AppendSizeAttribute(szNewStyle, COUNTOF(szNewStyle), fNewFontSize, bRelFontSize ? Style_GetCurrentLexerFontSize() : 0.0f);
        break;
    }

    if ((lf.lfCharSet != DEFAULT_CHARSET) && (lf.lfCharSet != ANSI_CHARSET)) {
        WCHAR chset[32] = { L'\0' };
        StringCchPrintf(chset, COUNTOF(chset), L"charset:%i", GdiCharsetToSci(lf.lfCharSet));
        AppendStyle(szNewStyle, COUNTOF(szNewStyle), chset);
    }

    if (lf.lfUnderline) {
        AppendStyle(szNewStyle, COUNTOF(szNewStyle), FontEffects[FE_UNDERLINE]);
    }

    if (lf.lfStrikeOut) {
        AppendStyle(szNewStyle, COUNTOF(szNewStyle), FontEffects[FE_STRIKEOUT]);
    }

    COLORREF fgColorDefault = 0L;
    Style_StrGetColor(L"", FOREGROUND_LAYER, &fgColorDefault, NULL, true);
    if (cf.rgbColors != fgColorDefault) {
        WCHAR fgColorStr[32] = { L'\0' };
        Style_PrintfCchColor(fgColorStr, COUNTOF(fgColorStr), L"; ", FOREGROUND_LAYER, cf.rgbColors);
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), fgColorStr);
    }

    // Font Quality
    if (StrIsNotEmpty(wchFontQuality)) {
        AppendStyle(szNewStyle, COUNTOF(szNewStyle), L"smoothing:");
        StringCchCat(szNewStyle, COUNTOF(szNewStyle), wchFontQuality);
    }

    // copy all other styles (incl. background color)
    Style_CopyStyles_IfNotDefined(lpszStyle, szNewStyle, COUNTOF(szNewStyle));

    StrTrim(szNewStyle, L" ;");
    StringCchCopy(lpszStyle, cchStyle, szNewStyle);
    return true;
}


//=============================================================================
//
//  Style_SelectColor()
//
bool Style_SelectColor(HWND hwnd,bool bForeGround,LPWSTR lpszStyle,int cchStyle, bool bPreserveStyles)
{
    WCHAR szNewStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };
    COLORREF dColor;
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };

    COLOR_LAYER const layer = (bForeGround ? FOREGROUND_LAYER : BACKGROUND_LAYER);
    COLORREF dRGBResult;
    Style_StrGetColor(lpszStyle, layer, &dRGBResult, NULL, true);

    CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };
    cc.hwndOwner = hwnd;
    cc.hInstance = (HWND)Globals.hLngResContainer; // Globals.hInstance;
    cc.rgbResult = dRGBResult;
    cc.lpCustColors = &g_colorCustom[0];
    cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_SOLIDCOLOR;

    // custom hook
    cc.Flags |= CC_ENABLEHOOK;
    cc.lpfnHook = (LPCCHOOKPROC)ColorDialogHookProc;
    cc.lCustData = (LPARAM)NULL;

    // Color.dlg resource template
    cc.Flags |= CC_ENABLETEMPLATE | CC_ENABLETEMPLATEHANDLE;
    cc.lpTemplateName = MAKEINTRESOURCEW(IDD_MUI_SYSCOLOR_DLG);

    if (!ChooseColor(&cc)) {
        return false;
    }
    dRGBResult = cc.rgbResult;

    // Rebuild style string
    szNewStyle[0] = L'\0'; // clear

    if (bForeGround) {

        Style_PrintfCchColor(tch, COUNTOF(tch), L"; ", FOREGROUND_LAYER, dRGBResult);
        StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);

        if (Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &dColor, NULL, false)) {
            Style_PrintfCchColor(tch, COUNTOF(tch), L"; ", BACKGROUND_LAYER, dColor);
            StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
        }

    } else { // set background

        if (Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &dColor, NULL, false)) {
            Style_PrintfCchColor(tch, COUNTOF(tch), L"; ", FOREGROUND_LAYER, dColor);
            StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
        }
        Style_PrintfCchColor(tch, COUNTOF(tch), L"; ", BACKGROUND_LAYER, dRGBResult);
        StringCchCat(szNewStyle,COUNTOF(szNewStyle),tch);
    }

    if (bPreserveStyles) {
        // copy all other styles
        Style_CopyStyles_IfNotDefined(lpszStyle, szNewStyle, COUNTOF(szNewStyle));
    }

    StrTrim(szNewStyle, L" ;");
    StringCchCopy(lpszStyle, cchStyle, szNewStyle);
    return true;
}



//=============================================================================
//
//  Style_SetStyles()
//
void Style_SetStyles(HWND hwnd, const int iStyle, LPCWSTR lpszStyle, const float fBaseFontSize)
{
    bool const bIsDefaultStyle = (iStyle == STYLE_DEFAULT);

    if (!bIsDefaultStyle && StrIsEmpty(lpszStyle)) {
        return;
    }

    int iValue = 0;
    WCHAR tch[BUFSIZE_STYLE_VALUE] = { L'\0' };

    // reset horizontal scrollbar width
    SciCall_SetScrollWidth(1);

    // === FONT === 

    // Font Face Name
    WCHAR wchFontName[LF_FACESIZE] = { L'\0' };
    if (!Style_StrGetFontName(lpszStyle, wchFontName, COUNTOF(wchFontName))) {
        if (bIsDefaultStyle) {
            //?Style_StrGetFontName(GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue, wchFontName, COUNTOF(wchFontName));
            GetDefaultCodeFont(wchFontName, COUNTOF(wchFontName), 0);
        }
    }
    if (StrIsNotEmpty(wchFontName)) {
        char chSetFontName[LF_FACESIZE<<2] = { '\0' };
        WideCharToMultiByte(CP_UTF8, 0, wchFontName, -1, chSetFontName, (int)COUNTOF(chSetFontName), NULL, NULL);
        SciCall_StyleSetFont(iStyle, chSetFontName);
    }

    // Font Weight
    if (Style_StrGetWeightValue(lpszStyle, &iValue)) {
        SciCall_StyleSetWeight(iStyle, iValue);
    } else if (bIsDefaultStyle) {
        SciCall_StyleSetWeight(iStyle, SC_WEIGHT_NORMAL);
    }

    // Italic
    SciCall_StyleSetItalic(iStyle, Style_StrHasAttribute(lpszStyle, FontEffects[FE_ITALIC]));

    // Font Quality
    int iFontQuality = Settings2.SciFontQuality;
    if (bIsDefaultStyle) {
        if (Style_StrGetFontQuality(lpszStyle, tch, COUNTOF(tch), &iFontQuality)) {
            SciCall_SetFontQuality(iFontQuality);
        } else {
            SciCall_SetFontQuality(Settings2.SciFontQuality);
        }
    }

    // Size values are relative to BaseFontSize/CurrentFontSize
    float fFontSize = fBaseFontSize;
    Style_StrGetSizeFloatEx(lpszStyle, &fFontSize);
    SendMessage(hwnd, SCI_STYLESETSIZEFRACTIONAL, iStyle, f2int(fFontSize * SC_FONT_SIZE_MULTIPLIER));

    char localeNameA[LOCALE_NAME_MAX_LENGTH] = "en-us\0";
#if defined(HAVE_DYN_LOAD_LIBS_MUI_LNGS)
    WideCharToMultiByte(CP_UTF8, 0, Settings2.PreferredLanguageLocaleName, -1, localeNameA, COUNTOF(localeNameA), NULL, NULL);
#else
    WideCharToMultiByte(CP_UTF8, 0, MUI_BASE_LNG_ID, -1, localeNameA, COUNTOF(localeNameA), NULL, NULL);
#endif
    SciCall_SetFontLocale(localeNameA);

    // Character Set
    if (Style_StrGetCharSet(lpszStyle, &iValue)) {
        SendMessage(hwnd, SCI_STYLESETCHARACTERSET, iStyle, (LPARAM)iValue);
    } else if (bIsDefaultStyle) {
        SendMessage(hwnd, SCI_STYLESETCHARACTERSET, iStyle, (LPARAM)SC_CHARSET_DEFAULT);
    }

    COLORREF dColor = 0L;
    // Foregr
    if (Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &dColor, NULL, false)) {
        SciCall_StyleSetFore(iStyle, dColor);
    } else if (bIsDefaultStyle) {
        SciCall_StyleSetFore(iStyle, GetModeTextColor(UseDarkMode()));
    } else { // fallback: SCI default
        Style_StrGetColor(lpszStyle, FOREGROUND_LAYER, &dColor, NULL, true);
        SciCall_StyleSetFore(iStyle, dColor);
    }

    // Backgr
    if (Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &dColor, NULL, false)) {
        SciCall_StyleSetBack(iStyle, dColor);
    } else if (bIsDefaultStyle) {
        SciCall_StyleSetBack(iStyle, GetModeBkColor(UseDarkMode()));
    } else { // fallback: SCI default
        Style_StrGetColor(lpszStyle, BACKGROUND_LAYER, &dColor, NULL, true);
        SciCall_StyleSetBack(iStyle, dColor);
    }

    // Underline
    if (Style_StrHasAttribute(lpszStyle, FontEffects[FE_UNDERLINE])) {
        SendMessage(hwnd, SCI_STYLESETUNDERLINE, iStyle, (LPARAM)true);
    } else if (bIsDefaultStyle) {
        SendMessage(hwnd, SCI_STYLESETUNDERLINE, iStyle, (LPARAM)false);
    }
    // StrikeOut
    if (Style_StrHasAttribute(lpszStyle, FontEffects[FE_STRIKEOUT])) {
        SendMessage(hwnd, SCI_STYLESETSTRIKE, iStyle, (LPARAM)true);
    } else if (bIsDefaultStyle) {
        SendMessage(hwnd, SCI_STYLESETSTRIKE, iStyle, (LPARAM)false);
    }
    // EOL Filled
    if (Style_StrHasAttribute(lpszStyle, FontEffects[FE_EOLFILLED])) {
        SendMessage(hwnd, SCI_STYLESETEOLFILLED, iStyle, (LPARAM)true);
    } else if (bIsDefaultStyle) {
        SendMessage(hwnd, SCI_STYLESETEOLFILLED, iStyle, (LPARAM)false);
    }
    // Case
    if (Style_StrGetCase(lpszStyle, &iValue)) {
        SendMessage(hwnd, SCI_STYLESETCASE, iStyle, (LPARAM)iValue);
    } else if (bIsDefaultStyle) {
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
    WCHAR pszFile[STYLE_EXTENTIONS_BUFFER << 1];

    LPCWSTR pszExtensions = StrIsNotEmpty(plex->szExtensions) ? plex->szExtensions : plex->pszDefExt;
    StringCchCopy(pszFile, COUNTOF(pszFile), L"*.");
    StringCchCat(pszFile, COUNTOF(pszFile), pszExtensions);

    WCHAR *p = StrChrW(pszFile, L';');
    if (p) {
        *p = L'\0';
    }

    // check for ; at beginning
    if (StringCchLen(pszFile, COUNTOF(pszFile)) < 3) {
        StringCchCat(pszFile, COUNTOF(pszFile), L"txt");
    }
    SHFILEINFO shfi = { 0 };
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

    TVINSERTSTRUCT tvis = { 0 };
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

    if (GetLngString(plex->resID,tch,COUNTOF(tch))) {
        tvis.item.pszText = tch;
    } else {
        tvis.item.pszText = (LPWSTR)plex->pszName;
    }

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

        if (GetLngString(plex->Styles[i].rid,tch,COUNTOF(tch))) {
            tvis.item.pszText = tch;
        } else {
            tvis.item.pszText = (LPWSTR)plex->Styles[i].pszName;
        }
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
    LVITEM lvi = { 0 };
    lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
    lvi.iItem = ListView_GetItemCount(hwnd);
    if (GetLngString(plex->resID,tch,COUNTOF(tch))) {
        lvi.pszText = tch;
    } else {
        lvi.pszText = (LPWSTR)plex->pszName;
    }
    lvi.iImage = Style_GetLexerIconId(plex);
    lvi.lParam = (LPARAM)plex;

    ListView_InsertItem(hwnd,&lvi);
}



//=============================================================================
//
//  Style_CustomizeSchemesDlgProc()
//

static bool  _ApplyDialogItemText(HWND hwnd,
                                  PEDITLEXER pDlgLexer, PEDITSTYLE pDlgStyle, int iDlgStyleIdx, bool bIsStyleSelected)
{
    UNREFERENCED_PARAMETER(iDlgStyleIdx);

    bool bChgNfy = false;

    WCHAR szBuf[max(BUFSIZE_STYLE_VALUE, STYLE_EXTENTIONS_BUFFER)] = { L'\0' };
    GetDlgItemText(hwnd, IDC_STYLEEDIT, szBuf, COUNTOF(szBuf));
    // normalize
    WCHAR szBufNorm[max(BUFSIZE_STYLE_VALUE, STYLE_EXTENTIONS_BUFFER)] = { L'\0' };
    Style_CopyStyles_IfNotDefined(szBuf, szBufNorm, COUNTOF(szBufNorm));

    if (StringCchCompareXI(szBufNorm, pDlgStyle->szValue) != 0) {
        StringCchCopy(pDlgStyle->szValue, COUNTOF(pDlgStyle->szValue), szBufNorm);
        bChgNfy = true;
    }
    if (!bIsStyleSelected) { // must be file extensions
        if (!GetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, szBuf, COUNTOF(szBuf))) {
            StringCchCopy(szBuf, COUNTOF(szBuf), pDlgLexer->pszDefExt);
        }
        if (StringCchCompareXI(szBuf, pDlgLexer->szExtensions) != 0) {
            StringCchCopy(pDlgLexer->szExtensions, COUNTOF(pDlgLexer->szExtensions), szBuf);
            bChgNfy = true;
        }
    }
    if (bChgNfy && (IsLexerStandard(pDlgLexer) || (pDlgLexer == s_pLexCurrent))) {
        Style_ResetCurrentLexer(Globals.hwndEdit);
    }
    return bChgNfy;
}


static WCHAR s_OrigTitle[64] = { L'\0' };
static WCHAR s_TitleTxt[128] = { L'\0' };

static void _UpdateTitleText(HWND hwnd)
{
    if (StrIsEmpty(s_OrigTitle)) {
        GetWindowText(hwnd, s_OrigTitle, COUNTOF(s_OrigTitle));
    }
    WCHAR scheme[96] = { L'\0' };
    StringCchCopy(scheme, COUNTOF(scheme), Theme_Files[Globals.uCurrentThemeIndex].szName);
    StrDelChr(scheme, L"&"); // rm hotkey mark
    PWCHAR const e = StrChr(scheme, L' ');
    if (e) {
        *e = L'\0';    // until 1st space
    }
    StringCchPrintf(s_TitleTxt, COUNTOF(s_TitleTxt), L"%s - %s", s_OrigTitle, scheme);
    SetWindowText(hwnd, s_TitleTxt);
}


static void _style_copy(void* _dst, const void* _src) {
    LPCWSTR const * src = (LPWSTR const *)_src;
    LPCWSTR * dst = (LPWSTR *)_dst;
    *dst = *src ? StrDup(*src) : NULL;
}

static void _style_dtor(void *_elt) {
    LPWSTR * const elt = (LPWSTR *)_elt;
    if (*elt) { LocalFree(*elt); }
}

static UT_icd _style_icd = { sizeof(LPWSTR), NULL, _style_copy, _style_dtor };

INT_PTR CALLBACK Style_CustomizeSchemesDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    static HWND       hwndTV;
    static HFONT      hFontTitle = NULL;
    static bool       fDragging;
    static PEDITLEXER pCurrentLexer = NULL;
    static PEDITSTYLE pCurrentStyle = NULL;
    static int        iCurStyleIdx  = -1;
    static HBRUSH     hbrFore = { 0 };
    static HBRUSH     hbrBack = { 0 };
    static bool       bIsStyleSelected = false;
    static bool       bWarnedNoIniFile = false;

    static WCHAR      tchTmpBuffer[max(BUFSIZE_STYLE_VALUE, STYLE_EXTENTIONS_BUFFER)] = {L'\0'};
    static UT_array  *pStylesBackup = NULL;

    switch (umsg) {
    case WM_INITDIALOG: {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);

        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

        _UpdateTitleText(hwnd);

        if (pStylesBackup) {
            utarray_free(pStylesBackup);
            pStylesBackup = NULL;
        }
        utarray_new(pStylesBackup, &_style_icd);

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
        UINT const dpi = Scintilla_GetWindowDPI(hwnd);

        GetLngString(IDS_MUI_STYLEEDIT_HELP, tchTmpBuffer, COUNTOF(tchTmpBuffer));
        SetDlgItemText(hwnd, IDC_STYLEEDIT_HELP, tchTmpBuffer);

        // Backup Styles
        for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {
            LPCWSTR const pExt = g_pLexArray[iLexer]->szExtensions;
            utarray_push_back(pStylesBackup, &pExt);
            int i = 0;
            while (g_pLexArray[iLexer]->Styles[i].iStyle != -1) {
                LPCWSTR const pVal = g_pLexArray[iLexer]->Styles[i++].szValue;
                utarray_push_back(pStylesBackup, &pVal);
            }
        }

        hwndTV    = GetDlgItem(hwnd, IDC_STYLELIST);
        fDragging = false;

        SHFILEINFO shfi = { 0 };

        InitWindowCommon(hwndTV, true);
        InitTreeView(hwndTV);

        TreeView_SetExtendedStyle(hwndTV, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);

        UINT const flagIconSize = (dpi >= LargeIconDPI()) ? SHGFI_LARGEICON : SHGFI_SMALLICON;
        TreeView_SetImageList(hwndTV,
                              (HIMAGELIST)SHGetFileInfoW(L"C:\\", FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO),
                                      flagIconSize | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
                              TVSIL_NORMAL);

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
        for (int i = 0; i < COUNTOF(g_pLexArray); i++) {
            if (i == found) {
                hCurrentTVLex = Style_AddLexerToTreeView(hwndTV, g_pLexArray[i]);
            } else {
                Style_AddLexerToTreeView(hwndTV, g_pLexArray[i]);
            }
        }
        if (!hCurrentTVLex) {
            hCurrentTVLex = TreeView_GetRoot(hwndTV);
            if (Style_GetUse2ndDefault()) {
                hCurrentTVLex = TreeView_GetNextSibling(hwndTV, hCurrentTVLex);
            }
        }
        TreeView_Select(hwndTV, hCurrentTVLex, TVGN_CARET);
        TreeView_Expand(hwndTV, hCurrentTVLex, TVE_EXPAND);

        pCurrentLexer = (found >= 0) ? s_pLexCurrent : GetDefaultLexer();
        pCurrentStyle = &(pCurrentLexer->Styles[STY_DEFAULT]);
        iCurStyleIdx  = STY_DEFAULT;

        SendDlgItemMessage(hwnd, IDC_STYLEEDIT, EM_LIMITTEXT, max(BUFSIZE_STYLE_VALUE, STYLE_EXTENTIONS_BUFFER) - 1, 0);

        MakeBitmapButton(hwnd, IDC_PREVSTYLE, IDB_PREV, -1, -1);
        MakeBitmapButton(hwnd, IDC_NEXTSTYLE, IDB_NEXT, -1, -1);

        if (Settings.CustomSchemesDlgPosX == CW_USEDEFAULT || Settings.CustomSchemesDlgPosY == CW_USEDEFAULT) {
            CenterDlgInParent(hwnd, NULL);
        } else {
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
            LOGFONT lf = { 0 };
            GetObject(hFont, sizeof(LOGFONT), &lf);
            lf.lfHeight = MulDiv(lf.lfHeight, 3, 2);
            lf.lfWeight = FW_BOLD;
            //lf.lfUnderline = true;
            if (hFontTitle) {
                DeleteObject(hFontTitle);
            }
            hFontTitle = CreateFontIndirectW(&lf);
            SendDlgItemMessageW(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)hFontTitle, true);
            SendDlgItemMessageW(hwnd, IDC_TITLE, WM_SETTEXT, 0, (LPARAM)s_TitleTxt);
        }
    }
    return TRUE;

    case WM_DPICHANGED: {
        UINT const dpi = LOWORD(wParam);

        SHFILEINFO shfi = { 0 };
        UINT const flagIconSize = (dpi >= LargeIconDPI()) ? SHGFI_LARGEICON : SHGFI_SMALLICON;
        TreeView_SetImageList(hwndTV,
                              (HIMAGELIST)SHGetFileInfoW(L"C:\\", FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO),
                                      flagIconSize | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
                              TVSIL_NORMAL);

        // Set title font
        HFONT const hFont = (HFONT)SendDlgItemMessage(hwnd, IDC_STYLELABEL, WM_GETFONT, 0, 0);
        if (hFont) {
            LOGFONT lf = { 0 };
            GetObject(hFont, sizeof(LOGFONT), &lf);
            lf.lfHeight = MulDiv(lf.lfHeight, 3, 2);
            lf.lfWeight = FW_BOLD;
            //lf.lfUnderline = true;
            if (hFontTitle) {
                DeleteObject(hFontTitle);
            }
            hFontTitle = CreateFontIndirectW(&lf);
            SendDlgItemMessageW(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)hFontTitle, true);
            SendDlgItemMessageW(hwnd, IDC_TITLE, WM_SETTEXT, 0, (LPARAM)s_TitleTxt);
        }

        MakeBitmapButton(hwnd, IDC_PREVSTYLE, IDB_PREV, -1, -1);
        MakeBitmapButton(hwnd, IDC_NEXTSTYLE, IDB_NEXT, -1, -1);

        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
    }
    return TRUE;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC const   hdc = GetDC(hwnd); // ClientArea
        if (hdc) {
            BeginPaint(hwnd, &ps);

            UINT const dpi = Scintilla_GetWindowDPI(hwnd);

            int const   iconSize  = 64;
            int const   dpiSize  = ScaleIntByDPI(iconSize, dpi);
            HICON const hicon = (dpiSize > 128) ? Globals.hDlgIconPrefs256 : ((dpiSize > 64) ? Globals.hDlgIconPrefs128 : Globals.hDlgIconPrefs64);
            if (hicon) {
                RECT rc = {0};
                MapWindowPoints(GetDlgItem(hwnd, IDC_INFO_GROUPBOX), hwnd, (LPPOINT)&rc, 2);
                DrawIconEx(hdc, rc.left + ScaleIntByDPI(10, dpi), rc.top + ScaleIntByDPI(20, dpi), hicon, dpiSize, dpiSize, 0, NULL, DI_NORMAL);
            }

            ReleaseDC(hwnd, hdc);
            EndPaint(hwnd, &ps);
        }
    }
    return 0;

#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
    return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
    break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);
            int const buttons[] = { IDOK, IDCANCEL, IDC_STYLEFORE, IDC_STYLEBACK, IDC_STYLEFONT, IDC_PREVIEW,
                                    IDC_STYLEDEFAULT, IDC_PREVSTYLE, IDC_NEXTSTYLE, IDC_IMPORT, IDC_EXPORT
                                  };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            SendMessage(hwndTV, WM_THEMECHANGED, 0, 0);
            _UpdateTitleText(hwnd);
            SendDlgItemMessageW(hwnd, IDC_TITLE, WM_SETTEXT, 0, (LPARAM)s_TitleTxt); // scheme may have changed
            SendWMCommandEx(hwnd, IDC_STYLEEDIT, EN_CHANGE); // button color inlay
            UpdateWindowEx(hwnd);
        }
        break;
#endif

    case WM_ENABLE:
        // modal child dialog should disable main window too
        EnableWindow(Globals.hwndMain, (BOOL)wParam);
        return TRUE;

    case WM_ACTIVATE:
        DialogEnableControl(hwnd, IDC_PREVIEW, ((pCurrentLexer == s_pLexCurrent) || (pCurrentLexer == GetCurrentStdLexer())));
        return TRUE;

    case WM_DESTROY: {
        DeleteBitmapButton(hwnd, IDC_STYLEFORE);
        DeleteBitmapButton(hwnd, IDC_STYLEBACK);
        DeleteBitmapButton(hwnd, IDC_PREVSTYLE);
        DeleteBitmapButton(hwnd, IDC_NEXTSTYLE);

        // free old backup
        if (pStylesBackup) {
            utarray_free(pStylesBackup);
            pStylesBackup = NULL;
        }

        if (hFontTitle) {
            DeleteObject(hFontTitle);
            hFontTitle = NULL;
        }
        s_TitleTxt[0] = L'\0';
        s_OrigTitle[0] = L'\0';
        pCurrentLexer = NULL;
        pCurrentStyle = NULL;
        iCurStyleIdx  = -1;
    }
    return FALSE;

    case WM_SYSCOMMAND:
        if (wParam == IDS_MUI_SAVEPOS) {
            PostWMCommand(hwnd, IDACC_SAVEPOS);
            return TRUE;
        } else if (wParam == IDS_MUI_RESETPOS) {
            PostWMCommand(hwnd, IDACC_RESETPOS);
            return TRUE;
        } else {
            return FALSE;
        }

    case WM_NOTIFY:

        if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST) {
            LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;

            switch (lpnmtv->hdr.code) {
            case TVN_SELCHANGED: {
                if (pCurrentLexer && pCurrentStyle) {
                    _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
                }

                WCHAR name[80] = { L'\0' };
                WCHAR label[128] = { L'\0' };
                WCHAR styleBuf[BUFSIZE_STYLE_VALUE] = { L'\0' };

                //DialogEnableWindow(hwnd, IDC_STYLEEDIT, true);
                //DialogEnableWindow(hwnd, IDC_STYLEFONT, true);
                //DialogEnableWindow(hwnd, IDC_STYLEFORE, true);
                //DialogEnableWindow(hwnd, IDC_STYLEBACK, true);
                //DialogEnableWindow(hwnd, IDC_STYLEDEFAULT, true);

                // a lexer has been selected
                if (!TreeView_GetParent(hwndTV, lpnmtv->itemNew.hItem)) {
                    pCurrentLexer = (PEDITLEXER)lpnmtv->itemNew.lParam;

                    if (pCurrentLexer) {
                        bIsStyleSelected = false;
                        GetLngString(IDS_MUI_ASSOCIATED_EXT, label, COUNTOF(label));
                        SetDlgItemText(hwnd, IDC_STYLELABEL_ROOT, label);
                        DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, true);
                        SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->szExtensions);
                        DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, true);

                        iCurStyleIdx = STY_DEFAULT;
                        pCurrentStyle = &(pCurrentLexer->Styles[iCurStyleIdx]);

                        StringCchCopy(styleBuf, COUNTOF(styleBuf), pCurrentStyle->szValue);
                        Style_CopyStyles_IfNotDefined(GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue, styleBuf, COUNTOF(styleBuf));

                        if (IsLexerStandard(pCurrentLexer)) {
                            if (pCurrentStyle->rid == IDS_LEX_STD_STYLE) {
                                GetLngString(IDS_MUI_STY_BASESTD, label, COUNTOF(label));
                            } else {
                                GetLngString(IDS_MUI_STY_BASE2ND, label, COUNTOF(label));
                                DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, false);
                            }
                            DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, false);
                        } else {
                            GetLngString(pCurrentLexer->resID, name, COUNTOF(name));
                            FormatLngStringW(label, COUNTOF(label), IDS_MUI_STY_LEXDEF, name);
                            DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, true);
                        }
                        SetDlgItemText(hwnd, IDC_STYLELABEL, label);
                        SetDlgItemText(hwnd, IDC_STYLEEDIT, styleBuf);
                    } else {
                        SetDlgItemText(hwnd, IDC_STYLELABEL_ROOT, L"");
                        DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, false);
                        SetDlgItemText(hwnd, IDC_STYLELABEL, L"");
                        DialogEnableControl(hwnd, IDC_STYLEEDIT, false);
                    }
                    DialogEnableControl(hwnd, IDC_PREVIEW, ((pCurrentLexer == s_pLexCurrent) || (pCurrentLexer == GetCurrentStdLexer())));
                }
                // a style has been selected
                else {
                    if (pCurrentLexer) {
                        if (IsLexerStandard(pCurrentLexer)) {
                            if (pCurrentLexer->Styles[STY_DEFAULT].rid == IDS_LEX_STD_STYLE) {
                                GetLngString(IDS_MUI_STY_BASESTD, label, COUNTOF(label));
                            } else {
                                GetLngString(IDS_MUI_STY_BASE2ND, label, COUNTOF(label));
                            }
                        } else {
                            GetLngString(pCurrentLexer->resID, name, COUNTOF(name));
                            FormatLngStringW(label, COUNTOF(label), IDS_MUI_STY_LEXDEF, name);
                        }
                        SetDlgItemText(hwnd, IDC_STYLELABEL_ROOT, label);

                        StringCchCopy(styleBuf, COUNTOF(styleBuf), pCurrentLexer->Styles[STY_DEFAULT].szValue);
                        Style_CopyStyles_IfNotDefined(GetCurrentStdLexer()->Styles[STY_DEFAULT].szValue, styleBuf, COUNTOF(styleBuf));

                        SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, styleBuf);
                        DialogEnableControl(hwnd, IDC_STYLEEDIT_ROOT, false);

                        pCurrentStyle = (PEDITSTYLE)lpnmtv->itemNew.lParam;
                        iCurStyleIdx  = -1;
                        int i         = 0;
                        while (pCurrentLexer->Styles[i].iStyle != -1) {
                            if (pCurrentLexer->Styles[i].rid == pCurrentStyle->rid) {
                                iCurStyleIdx = i;
                                break;
                            }
                            ++i;
                        }
                        //assert(iCurStyleIdx != -1);
                    }
                    if (pCurrentStyle) {
                        bIsStyleSelected = true;
                        GetLngString(pCurrentStyle->rid, name, COUNTOF(name));
                        FormatLngStringW(label, COUNTOF(label), IDS_MUI_STY_LEXSTYLE, name);
                        SetDlgItemText(hwnd, IDC_STYLELABEL, label);
                        SetDlgItemText(hwnd, IDC_STYLEEDIT, pCurrentStyle->szValue);
                    } else {
                        iCurStyleIdx = -1;
                        SetDlgItemText(hwnd, IDC_STYLELABEL, L"");
                        DialogEnableControl(hwnd, IDC_STYLEEDIT, false);
                    }
                }
            }
            break;

            case TVN_BEGINDRAG: {
                TreeView_Select(hwndTV, lpnmtv->itemNew.hItem, TVGN_CARET);

                if (bIsStyleSelected) {
                    DestroyCursor(SetCursor(LoadCursor(Globals.hInstance, MAKEINTRESOURCE(IDC_COPY))));
                } else {
                    DestroyCursor(SetCursor(LoadCursor(NULL, IDC_NO)));
                }

                SetCapture(hwnd);
                fDragging = true;
            }
            }
        }
        break;

    case WM_MOUSEMOVE: {
        HTREEITEM htiTarget = { 0 };
        TVHITTESTINFO tvht = { 0 };
        if (fDragging && bIsStyleSelected) {
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
                    TreeView_GetParent(hwndTV, htiTarget) != NULL) {
                TreeView_SelectDropTarget(hwndTV, htiTarget);
                //TreeView_Expand(hwndTV,htiTarget,TVE_EXPAND);
                TreeView_EnsureVisible(hwndTV, htiTarget);
            } else {
                TreeView_SelectDropTarget(hwndTV, NULL);
            }

            //ImageList_DragShowNolock(true);
        }
    }
    break;

    case WM_LBUTTONUP: {
        if (fDragging && bIsStyleSelected) {
            //ImageList_EndDrag();
            HTREEITEM htiTarget = TreeView_GetDropHilight(hwndTV);
            if (htiTarget) {
                WCHAR tchCopy[max(BUFSIZE_STYLE_VALUE, STYLE_EXTENTIONS_BUFFER)] = {L'\0'};
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

    case WM_CANCELMODE: {
        if (fDragging) {
            //ImageList_EndDrag();
            TreeView_SelectDropTarget(hwndTV, NULL);
            ReleaseCapture();
            DestroyCursor(SetCursor(LoadCursor(NULL, IDC_ARROW)));
            fDragging = false;
        }
    }
    break;

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_SETCURLEXERTV: {
            // find current lexer's tree entry
            HTREEITEM hCurrentTVLex = TreeView_GetRoot(hwndTV);
            for (int i = 0; i < COUNTOF(g_pLexArray); ++i) {
                if (g_pLexArray[i] == s_pLexCurrent) {
                    break;
                }
                hCurrentTVLex = TreeView_GetNextSibling(hwndTV, hCurrentTVLex); // next
            }
            if (s_pLexCurrent == pCurrentLexer) {
                break;    // no change
            }

            // collaps current node
            HTREEITEM hSel = TreeView_GetSelection(hwndTV);
            if (hSel) {
                HTREEITEM hPar = TreeView_GetParent(hwndTV, hSel);
                TreeView_Expand(hwndTV, hSel, TVE_COLLAPSE);
                if (hPar) {
                    TreeView_Expand(hwndTV, hPar, TVE_COLLAPSE);
                }
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
            if (pCurrentStyle) {
                WCHAR tch[BUFSIZE_STYLE_VALUE] = {L'\0'};
                GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
                if (Style_SelectColor(hwnd, true, tch, COUNTOF(tch), true)) {
                    SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
                }
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
            break;

        case IDC_STYLEBACK:
            if (pCurrentStyle) {
                WCHAR tch[BUFSIZE_STYLE_VALUE] = {L'\0'};
                GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));
                if (Style_SelectColor(hwnd, false, tch, COUNTOF(tch), true)) {
                    SetDlgItemText(hwnd, IDC_STYLEEDIT, tch);
                }
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
            break;

        case IDC_STYLEFONT:
            if (pCurrentStyle) {
                WCHAR lexerName[BUFSIZE_STYLE_VALUE] = {L'\0'};
                WCHAR styleName[BUFSIZE_STYLE_VALUE] = {L'\0'};
                GetDlgItemText(hwnd, IDC_STYLEEDIT, tchTmpBuffer, COUNTOF(tchTmpBuffer));
                GetLngString(pCurrentLexer->resID, lexerName, COUNTOF(lexerName));
                GetLngString(pCurrentStyle->rid, styleName, COUNTOF(styleName));
                DEFAULT_FONT_STYLES const defaultFontStyle = IsStyleSchemeDefault(pCurrentStyle) ? DFS_CURR_LEXER :
                    (IsStyleStandardDefault(pCurrentStyle) ? DFS_GLOBAL : DFS_GENERIC_USE);
                if (Style_SelectFont(hwnd, tchTmpBuffer, COUNTOF(tchTmpBuffer), lexerName, styleName, defaultFontStyle)) {
                    SetDlgItemText(hwnd, IDC_STYLEEDIT, tchTmpBuffer);
                }
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
            break;

        case IDC_STYLEDEFAULT: {
            WCHAR wchDefaultStyle[BUFSIZE_STYLE_VALUE] = { L'\0' };
            _DefaultsToTmpCache();
            TmpCacheGetString(pCurrentLexer->pszName, pCurrentStyle->pszName, L"", wchDefaultStyle, COUNTOF(wchDefaultStyle));
            SetDlgItemText(hwnd, IDC_STYLEEDIT, wchDefaultStyle);
            if (!bIsStyleSelected) {
                SetDlgItemText(hwnd, IDC_STYLEEDIT_ROOT, pCurrentLexer->pszDefExt);
            }
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
            ResetTmpCache();
        }
        break;

        case IDC_STYLEEDIT: {
            if (HIWORD(wParam) == EN_CHANGE) {
                WCHAR tch[max(BUFSIZE_STYLE_VALUE, STYLE_EXTENTIONS_BUFFER)] = {L'\0'};

                GetDlgItemText(hwnd, IDC_STYLEEDIT, tch, COUNTOF(tch));

                COLORREF cr = COLORREF_MAX; // SciCall_StyleGetFore(STYLE_DEFAULT);
                Style_StrGetColor(tch, FOREGROUND_LAYER, &cr, NULL, true);
                MakeColorPickButton(hwnd, IDC_STYLEFORE, Globals.hInstance, cr);

                cr = COLORREF_MAX; // SciCall_StyleGetBack(STYLE_DEFAULT);
                Style_StrGetColor(tch, BACKGROUND_LAYER, &cr, NULL, true);
                MakeColorPickButton(hwnd, IDC_STYLEBACK, Globals.hInstance, cr);
            }
        }
        break;

        case IDC_IMPORT: {
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

        case IDC_EXPORT: {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
            Style_Export(hwnd);
        }
        break;

        case IDC_PREVIEW: {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
        }
        break;

        case IDC_PREVSTYLE: {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
            HTREEITEM hSel = TreeView_GetSelection(hwndTV);
            if (hSel) {
                HTREEITEM hPrev = TreeView_GetPrevVisible(hwndTV, hSel);
                if (hPrev) {
                    TreeView_Select(hwndTV, hPrev, TVGN_CARET);
                }
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
        }
        break;

        case IDC_NEXTSTYLE: {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);
            HTREEITEM hSel = TreeView_GetSelection(hwndTV);
            if (hSel) {
                HTREEITEM hNext = TreeView_GetNextVisible(hwndTV, hSel);
                if (hNext) {
                    TreeView_Select(hwndTV, hNext, TVGN_CARET);
                }
            }
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_STYLEEDIT)), 1);
        }
        break;

        case IDOK: {
            _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);

            unsigned const iTheme = Globals.uCurrentThemeIndex;
            if ((iTheme > 0) && (!bWarnedNoIniFile && Path_IsEmpty(Theme_Files[iTheme].hStyleFilePath))) {
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
            } else {
                _ApplyDialogItemText(hwnd, pCurrentLexer, pCurrentStyle, iCurStyleIdx, bIsStyleSelected);

                // Restore Styles from Backup
                LPWSTR * pStyle = (LPWSTR*)utarray_front(pStylesBackup);
                for (int iLexer = 0; iLexer < COUNTOF(g_pLexArray); ++iLexer) {
                    StringCchCopy(g_pLexArray[iLexer]->szExtensions, COUNTOF(g_pLexArray[iLexer]->szExtensions), *pStyle);
                    pStyle = (LPWSTR*)utarray_next(pStylesBackup, (void*)pStyle);
                    int i = 0;
                    while (pStyle && (g_pLexArray[iLexer]->Styles[i].iStyle != -1)) {
                        // normalize
                        tchTmpBuffer[0] = L'\0'; // clear
                        Style_CopyStyles_IfNotDefined(*pStyle, tchTmpBuffer, COUNTOF(tchTmpBuffer));
                        StringCchCopy(g_pLexArray[iLexer]->Styles[i].szValue, COUNTOF(g_pLexArray[iLexer]->Styles[i].szValue), tchTmpBuffer);
                        ++i;
                        pStyle = (LPWSTR*)utarray_next(pStylesBackup, (void*)pStyle);
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
    if (IS_VALID_HANDLE(hDlg)) {
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
    static int cxClient = 0;
    static int cyClient = 0;

    static HWND hwndLV = NULL;

    static int  iInternalDefault = 0;
    static PEDITLEXER* pSelectedLexer = NULL;


    switch(umsg) {
    case WM_INITDIALOG: {

        pSelectedLexer = (PEDITLEXER*)lParam;

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

        UINT const dpi = Scintilla_GetWindowDPI(hwnd);

        hwndLV = GetDlgItem(hwnd,IDC_STYLELIST);
        InitWindowCommon(hwndLV, true);
        InitListView(hwndLV);

        SHFILEINFO shfi = { 0 };

        UINT const flagIconSize = (dpi >= LargeIconDPI()) ? SHGFI_LARGEICON : SHGFI_SMALLICON;
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
        LVITEM lvi = { 0 };
        lvi.mask = LVIF_PARAM;
        for (int i = 0; i < lvItems; i++) {
            lvi.iItem = i;
            ListView_GetItem(hwndLV,&lvi);

            if (((PEDITLEXER)lvi.lParam)->resID == (*pSelectedLexer)->resID) {
                ListView_SetItemState(hwndLV,i,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
                ListView_EnsureVisible(hwndLV,i,false);
                CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, SetBtn(s_iDefaultLexer == i));
                break;
            }
        }

        iInternalDefault = s_iDefaultLexer;
        CheckDlgButton(hwnd,IDC_AUTOSELECT, SetBtn(s_bAutoSelect));

        CenterDlgInParent(hwnd, NULL);
    }
    return TRUE;


    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;


    case WM_DESTROY:
        return TRUE;


    case WM_SIZE: {
        ListView_SetColumnWidth(GetDlgItem(hwnd, IDC_STYLELIST), 0, LVSCW_AUTOSIZE_USEHEADER);
    }
    return TRUE;


    case WM_DPICHANGED: {
        UINT const dpi = LOWORD(wParam);
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, dpi);
        SHFILEINFO shfi = { 0 };
        UINT const flagIconSize = (dpi >= LargeIconDPI()) ? SHGFI_LARGEICON : SHGFI_SMALLICON;
        ListView_SetImageList(hwndLV,
            (HIMAGELIST)SHGetFileInfo(L"C:\\", FILE_ATTRIBUTE_DIRECTORY,
                &shfi, sizeof(SHFILEINFO), flagIconSize | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES),
            LVSIL_SMALL);
    }
        return TRUE;

  
    case WM_GETMINMAXINFO:
        return TRUE;


#ifdef D_NP3_WIN10_DARK_MODE

CASE_WM_CTLCOLOR_SET:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);
            int const buttons[] = { IDOK, IDCANCEL };
            for (int id = 0; id < COUNTOF(buttons); ++id) {
                HWND const hBtn = GetDlgItem(hwnd, buttons[id]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            SendMessage(hwndLV, WM_THEMECHANGED, 0, 0);
            UpdateWindowEx(hwnd);
        }
        break;

#endif


    case WM_NOTIFY: {
        if (((LPNMHDR)(lParam))->idFrom == IDC_STYLELIST) {

            switch (((LPNMHDR)(lParam))->code) {

            case NM_DBLCLK:
                SendWMCommand(hwnd, IDOK);
                break;

            case LVN_ITEMCHANGED:
            case LVN_DELETEITEM: {
                int i = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
                CheckDlgButton(hwnd, IDC_DEFAULTSCHEME, SetBtn(iInternalDefault == i));
                DialogEnableControl(hwnd, IDC_DEFAULTSCHEME, i != -1);
                DialogEnableControl(hwnd, IDOK, i != -1);
            }
            break;
            }
        }
    }
    return TRUE;


    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_DEFAULTSCHEME:
            if (IsButtonChecked(hwnd, IDC_DEFAULTSCHEME)) {
                iInternalDefault = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
            } else {
                iInternalDefault = 0;
            }
            break;


        case IDOK: {
            LVITEM lvi = {0};
            lvi.mask = LVIF_PARAM;
            lvi.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_ALL | LVNI_SELECTED);
            if (ListView_GetItem(hwndLV, &lvi)) {
                *pSelectedLexer = (PEDITLEXER)lvi.lParam;
                s_iDefaultLexer = iInternalDefault;
                s_bAutoSelect = IsButtonChecked(hwnd, IDC_AUTOSELECT);
                //@@@??? Flags.bHugeFileLoadState = false;  // user choice
                EndDialog(hwnd,IDOK);
            }
        }
        break;


        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        } // switch()
    } // WM_COMMAND
    return TRUE;
    }
    return 0;
}


//=============================================================================
//
//  Style_SelectLexerDlg()
//
void Style_SelectLexerDlg(HWND hwnd)
{
    PEDITLEXER selectedLexer = s_pLexCurrent;

    if (IsYesOkay(ThemedDialogBoxParam(Globals.hLngResContainer,
                  MAKEINTRESOURCE(IDD_MUI_STYLESELECT),
                  GetParent(hwnd), Style_SelectLexerDlgProc, (LPARAM)&selectedLexer))) {
        Style_SetLexer(Globals.hwndEdit, selectedLexer);
    }
}

// End of Styles.c
