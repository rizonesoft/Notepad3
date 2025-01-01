// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Styles.h                                                                    *
*   Scintilla Style Management                                                *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2025   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_STYLES_H_
#define _NP3_STYLES_H_

#include "Scintilla.h"
#include "TypeDefs.h"
#include "StyleLexers/EditLexer.h"


#define MARGIN_SCI_LINENUM 0
#define MARGIN_SCI_BOOKMRK 1
#define MARGIN_SCI_CHGHIST 2
#define MARGIN_SCI_FOLDING 3
#define NUMBER_OF_MARGINS  4

#define MARGIN_MARK_HISTORY_MASK (                   \
    (1 << SC_MARKNUM_HISTORY_REVERTED_TO_ORIGIN) |   \
    (1 << SC_MARKNUM_HISTORY_SAVED) |                \
    (1 << SC_MARKNUM_HISTORY_MODIFIED) |             \
    (1 << SC_MARKNUM_HISTORY_REVERTED_TO_MODIFIED)   \
)


int    Style_NumOfLexers();  // Number of Lexers in pLexArray

void   Style_InitFileExtensions();
void   Style_Prerequisites();
bool   Style_Import(HWND hwnd);
bool   Style_ImportTheme(const int iThemeIdx);  // -1 => Factory Reset
bool   Style_ImportFromFile(const HPATHL hpath);
void   Style_SaveSettings(bool bForceSaveSettings);
bool   Style_Export(HWND hwnd);
void   Style_FileExtToIniSection(bool bForceAll);
void   Style_CanonicalSectionToIniCache();
bool   Style_ToIniSection(bool bForceAll);
bool   Style_ExportToFile(const HPATHL hpath, bool bForceAll);

unsigned ThemeItems_CountOf();
void     ThemesItems_Init();
void     ThemesItems_Release();
bool     Style_InsertThemesMenu(HMENU hMenuBar);
bool     Style_DynamicThemesMenuCmd(int cmd);

float  Style_GetCurrentLexerFontSize();
void   Style_SetLexer(HWND hwnd,PEDITLEXER pLexNew);
void   Style_FillRelatedStyles(HWND hwnd, const PEDITLEXER pLexer);
void   Style_SetUrlHotSpot(HWND hwnd);
void   Style_SetInvisible(HWND hwnd, bool);
//void   Style_SetReadonly(HWND hwnd, bool);
void   Style_HighlightCurrentLine(HWND hwnd, int);
void   Style_UpdateLineNumberMargin(const bool bForce);
void   Style_UpdateBookmarkMargin(HWND hwnd);
void   Style_UpdateChangeHistoryMargin(HWND hwnd);
void   Style_UpdateFoldingMargin(HWND hwnd, bool bShowMargin);
void   Style_UpdateAllMargins(HWND hwnd, const bool bForce);
void   Style_SetMargin(HWND hwnd, LPCWSTR lpszStyle);
bool   Style_SetLexerFromFile(HWND hwnd,const HPATHL hpath);
bool   Style_MaybeBinaryFile(HWND hwnd, const HPATHL hpath);
void   Style_SetLexerFromName(HWND hwnd, const HPATHL hpath, LPCWSTR lpszName);
void   Style_ResetCurrentLexer(HWND hwnd);
void   Style_SetDefaultLexer(HWND hwnd);
void   Style_SetHTMLLexer(HWND hwnd);
void   Style_SetXMLLexer(HWND hwnd);
void   Style_SetLexerFromID(HWND hwnd,int id);
void   Style_SetDefaultFont(HWND hwnd,bool);
void   Style_ToggleUse2ndDefault(HWND hwnd);
bool   Style_GetUse2ndDefault();
void   Style_SetUse2ndDefault(bool);
void   Style_SetIndentGuides(HWND hwnd,bool);
void   Style_SetExtraLineSpace(int iValue);
bool   Style_GetFileFilterStr(LPWSTR lpszFilter, int cchFilter, LPWSTR lpszDefExt, int cchExt, bool bSaveAs);
bool   Style_StrGetFontName(LPCWSTR lpszStyle,LPWSTR lpszFont,int cchFont);
bool   Style_StrGetFontQuality(LPCWSTR lpszStyle, LPWSTR lpszQuality, int cchQuality, int* iSciQuality_out);
bool   Style_StrGetCharSet(LPCWSTR lpszStyle,int* i);
bool   Style_StrGetSizeInt(LPCWSTR lpszStyle, int* i);
bool   Style_StrGetSizeIntEx(LPCWSTR lpszStyle, int* i);
bool   Style_StrGetSizeFloat(LPCWSTR lpszStyle, float* f);
bool   Style_StrGetSizeFloatEx(LPCWSTR lpszStyle,float* f);
bool   Style_StrGetSizeStr(LPCWSTR lpszStyle,LPWSTR lpszSize,int cchSize);
void   Style_AppendSizeAttribute(LPWSTR lpszSize, int cchSize, const float fFontSize, const float fBaseFontSize);
bool   Style_StrGetWeightValue(LPCWSTR lpszWeight, int *weight);
void   Style_AppendWeightAttribute(LPWSTR lpszWeight, int cchSize, int fontWeight);
bool   Style_StrGetColor(LPCWSTR lpszStyle, COLOR_LAYER layer, COLORALPHAREF* rgba, COLORALPHAREF* rgbaOrig, bool useDefault);
bool   Style_StrGetStrokeWidth(HWND hwnd, int indicID, LPCWSTR lpszStyle, int *piStrokeWidth);
bool   Style_StrGetCase(LPCWSTR lpszStyle, int *i);
bool   Style_StrGetAlpha(LPCWSTR lpszStyle, int* iOutValue, const int defAlpha, const bool bAlpha1st);
bool   Style_GetIndicatorType(LPWSTR lpszStyle,int cchSize,int* idx);
void   Style_CopyStyles_IfNotDefined(LPCWSTR lpszStyleSrc, LPWSTR lpszStyleDest, int cchSizeDest);
bool   Style_SelectFont(HWND hwnd, LPWSTR lpszStyle, int cchStyle, LPCWSTR sLexerName, LPCWSTR sStyleName, DEFAULT_FONT_STYLES styleType);
bool   Style_SelectColor(HWND hwnd, bool, LPWSTR lpszStyle, int cchStyle, bool bPreserveStyles);
void   Style_SetStyles(HWND hwnd, const int iStyle, LPCWSTR lpszStyle, const float fBaseFontSize);
bool   Style_IsCurLexerStandard();
float  Style_GetBaseFontSize();
void   Style_SetMultiEdgeLine(const int colVec[], const size_t count);
PEDITLEXER Style_GetCurrentLexerPtr();
int    Style_GetCurrentLexerRID();
void   Style_GetLexerDisplayName(PEDITLEXER pLexer, LPWSTR lpszName, int cchName);
void   Style_GetStyleDisplayName(PEDITSTYLE pStyle, LPWSTR lpszName, int cchName);
int    Style_GetLexerIconId(PEDITLEXER plex);
bool   Style_HasLexerForExt(const HPATHL hpath);
HTREEITEM Style_AddLexerToTreeView(HWND hwnd,PEDITLEXER plex);
INT_PTR CALLBACK Styles_ConfigDlgProc(HWND,UINT,WPARAM,LPARAM);
HWND   Style_CustomizeSchemesDlg(HWND hwnd);
INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam);
void   Style_SelectLexerDlg(HWND hwnd);


inline void Style_PrintfCchColor(LPWSTR buffer, const size_t cch, LPCWSTR prefix, COLOR_LAYER layer, COLORREF color)
{
    if (layer == FOREGROUND_LAYER) {
        StringCchPrintf(buffer, cch, L"%sfore:#%02X%02X%02X", prefix, GetRValue(color), GetGValue(color), GetBValue(color));
    } else {
        StringCchPrintf(buffer, cch, L"%sback:#%02X%02X%02X", prefix, GetRValue(color), GetGValue(color), GetBValue(color));
    }
}

#if 0
bool   Style_StrGetStretchValue(LPCWSTR lpszWeight, int* stretch);
void   Style_AppendStretchStr(LPWSTR lpszWeight, int cchSize, int fontStretch);
#endif

#endif //_NP3_STYLES_H_

// End of Style.h
