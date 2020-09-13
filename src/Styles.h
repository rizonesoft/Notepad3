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
*                                                  (c) Rizonesoft 2008-2020   *
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
#define MARGIN_SCI_FOLDING 2

// Number of Lexers in pLexArray
#define NUMLEXERS 51
#define AVG_NUM_OF_STYLES_PER_LEXER 20

void   Style_Init();
bool   Style_Import(HWND hwnd);
bool   Style_ImportTheme(const unsigned iThemeIdx);
bool   Style_ImportFromFile(const WCHAR* szFile);
void   Style_SaveSettings(bool bForceSaveSettings);
bool   Style_Export(HWND hwnd);
void   Style_ToIniSection(bool bForceAll, bool bIsStdIniFile);
bool   Style_ExportToFile(const WCHAR* szFile, bool bForceAll);

unsigned ThemeItems_CountOf();
bool   Style_InsertThemesMenu(HMENU hMenuBar);
void   Style_DynamicThemesMenuCmd(int cmd, unsigned iCurThemeIdx);

float  Style_GetCurrentFontSize();
void   Style_SetFoldingAvailability(PEDITLEXER pLexer);
void   Style_SetFoldingProperties(bool active);
void   Style_SetFoldingFocusedView();
void   Style_SetLexer(HWND hwnd,PEDITLEXER pLexNew);
void   Style_SetUrlHotSpot(HWND hwnd);
void   Style_SetInvisible(HWND hwnd, bool);
void   Style_SetReadonly(HWND hwnd, bool);
void   Style_HighlightCurrentLine(HWND hwnd, int);
void   Style_SetFolding(HWND hwnd, bool bShowMargin);
void   Style_SetBookmark(HWND hwnd, bool bShowMargin);
void   Style_SetMargin(HWND hwnd, int iStyle, LPCWSTR lpszStyle);
bool   Style_SetLexerFromFile(HWND hwnd,LPCWSTR lpszFile);
bool   Style_MaybeBinaryFile(HWND hwnd, LPCWSTR lpszFile);
void   Style_SetLexerFromName(HWND hwnd,LPCWSTR lpszFile,LPCWSTR lpszName);
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
void   Style_SetExtraLineSpace(HWND hwnd, LPWSTR lpszStyle, int cch);
bool   Style_GetOpenDlgFilterStr(LPWSTR lpszFilter,int cchFilter);
bool   Style_StrGetFontName(LPCWSTR lpszStyle,LPWSTR lpszFont,int cchFont);
bool   Style_StrGetFontStyle(LPCWSTR lpszStyle,LPWSTR lpszFontStyle,int cchFontStyle);
bool   Style_StrGetFontQuality(LPCWSTR lpszStyle,LPWSTR lpszQuality,int cchQuality);
bool   Style_StrGetCharSet(LPCWSTR lpszStyle,int* i);
bool   Style_StrGetSizeInt(LPCWSTR lpszStyle, int* i);
bool   Style_StrGetSize(LPCWSTR lpszStyle,float* f);
bool   Style_StrGetSizeStr(LPCWSTR lpszStyle,LPWSTR lpszSize,int cchSize);
bool   Style_StrGetColor(LPCWSTR lpszStyle, COLOR_LAYER layer, COLORREF *rgb, bool useDefault);
bool   Style_StrGetCase(LPCWSTR lpszStyle,int* i);
bool   Style_StrGetAlpha(LPCWSTR lpszStyle, int* iOutValue, bool bAlpha1st);
bool   Style_GetIndicatorType(LPWSTR lpszStyle,int cchSize,int* idx);
void   Style_CopyStyles_IfNotDefined(LPCWSTR lpszStyleSrc,LPWSTR lpszStyleDest,int cchSizeDest,bool);
bool   Style_SelectFont(HWND hwnd,LPWSTR lpszStyle,int cchStyle,LPCWSTR sLexerName,LPCWSTR sStyleName,bool,bool,bool,bool);
bool   Style_SelectColor(HWND hwnd,bool,LPWSTR lpszStyle,int cchStyle,bool);
void   Style_SetStyles(HWND hwnd,int iStyle,LPCWSTR lpszStyle,bool);
bool   Style_IsCurLexerStandard();
float  Style_GetBaseFontSize();
void   Style_SetMultiEdgeLine(const int colVec[], const size_t count);
PEDITLEXER Style_GetCurrentLexerPtr();
int    Style_GetCurrentLexerRID();
void   Style_GetLexerDisplayName(PEDITLEXER pLexer, LPWSTR lpszName, int cchName);
void   Style_GetStyleDisplayName(PEDITSTYLE pStyle, LPWSTR lpszName, int cchName);
int    Style_GetLexerIconId(PEDITLEXER plex);
bool   Style_HasLexerForExt(LPCWSTR lpszFile);
HTREEITEM Style_AddLexerToTreeView(HWND hwnd,PEDITLEXER plex);
INT_PTR CALLBACK Styles_ConfigDlgProc(HWND,UINT,WPARAM,LPARAM);
HWND   Style_CustomizeSchemesDlg(HWND hwnd);
INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND hwnd,UINT umsg,WPARAM wParam,LPARAM lParam);
void   Style_SelectLexerDlg(HWND hwnd);
bool   Style_StrGetWeightValue(LPCWSTR lpszWeight,int* weight);
void   Style_AppendWeightStr(LPWSTR lpszWeight, int cchSize, int fontWeight);


inline void Style_PrintfCchColor(LPWSTR buffer, const size_t cch, LPCWSTR prefix, COLOR_LAYER layer, COLORREF color) {
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
