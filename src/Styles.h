/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Styles.h                                                                    *
*   Scintilla Style Management                                                *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_STYLES_H_
#define _NP3_STYLES_H_

#define BUFSIZE_STYLE_VALUE 256
#define BUFZIZE_STYLE_EXTENTIONS 512

#define MARGIN_SCI_LINENUM 0
#define MARGIN_SCI_BOOKMRK 1
#define MARGIN_SCI_FOLDING 2


typedef struct _editstyle
{
  #pragma warning(disable : 4201)  // MS's Non-Std: Struktur/Union ohne Namen
  union
  {
    INT32 iStyle;
    UINT8 iStyle8[4];
  };
  int rid;
  WCHAR* pszName;
  WCHAR* pszDefault;
  WCHAR  szValue[BUFSIZE_STYLE_VALUE];

} EDITSTYLE, *PEDITSTYLE;


typedef struct _keywordlist
{
  char *pszKeyWords[KEYWORDSET_MAX + 1];

} KEYWORDLIST, *PKEYWORDLIST;

#pragma warning(disable : 4200)  // MS's Non-Std: Null-Array in Struktur/Union
typedef struct _editlexer
{
  int iLexer;
  int rid;
  WCHAR* pszName;
  WCHAR* pszDefExt;
  WCHAR  szExtensions[BUFZIZE_STYLE_EXTENTIONS];
  PKEYWORDLIST pKeyWords;
  EDITSTYLE    Styles[];

} EDITLEXER, *PEDITLEXER;


// Number of Lexers in pLexArray
#define NUMLEXERS 46
#define AVG_NUM_OF_STYLES_PER_LEXER 20


void   Style_Load();
void   Style_Save();
BOOL   Style_Import(HWND);
BOOL   Style_Export(HWND);
void   Style_SetLexer(HWND,PEDITLEXER);
void   Style_SetUrlHotSpot(HWND, BOOL);
void   Style_SetLongLineColors(HWND);
void   Style_SetCurrentLineBackground(HWND, BOOL);
void   Style_SetFolding(HWND, BOOL);
void   Style_SetBookmark(HWND, BOOL);
void   Style_SetMargin(HWND, int, LPCWSTR);
void   Style_SetLexerFromFile(HWND,LPCWSTR);
void   Style_SetLexerFromName(HWND,LPCWSTR,LPCWSTR);
void   Style_ResetCurrentLexer(HWND);
void   Style_SetDefaultLexer(HWND);
void   Style_SetHTMLLexer(HWND);
void   Style_SetXMLLexer(HWND);
void   Style_SetLexerFromID(HWND,int);
void   Style_SetDefaultFont(HWND,BOOL);
void   Style_ToggleUse2ndDefault(HWND);
BOOL   Style_GetUse2ndDefault();
BOOL   Style_SetUse2ndDefault(int); // in:BOOL
int    Style_GetBaseFontSize(HWND);
int    Style_SetBaseFontSize(HWND,int);
int    Style_GetCurrentFontSize(HWND);
int    Style_SetCurrentFontSize(HWND, int);
void   Style_SetIndentGuides(HWND,BOOL);
BOOL   Style_GetOpenDlgFilterStr(LPWSTR,int);
BOOL   Style_StrGetFont(LPCWSTR,LPWSTR,int);
BOOL   Style_StrGetFontQuality(LPCWSTR,LPWSTR,int);
BOOL   Style_StrGetCharSet(LPCWSTR,int*);
BOOL   Style_StrGetSize(LPCWSTR,int*);
BOOL   Style_StrGetSizeStr(LPCWSTR,LPWSTR,int);
BOOL   Style_StrGetColor(BOOL,LPCWSTR, COLORREF*);
BOOL   Style_StrGetCase(LPCWSTR,int*);
BOOL   Style_StrGetAlpha(LPCWSTR,int*,BOOL);
BOOL   Style_GetIndicatorType(LPWSTR,int,int*);
void   Style_CopyStyles_IfNotDefined(LPWSTR,LPWSTR,int,BOOL,BOOL);
BOOL   Style_SelectFont(HWND,LPWSTR,int,LPCWSTR,LPCWSTR,BOOL,BOOL,BOOL,BOOL);
BOOL   Style_SelectColor(HWND,BOOL,LPWSTR,int,BOOL);
void   Style_SetStyles(HWND,int,LPCWSTR);
void   Style_SetFontQuality(HWND,LPCWSTR);
void   Style_GetCurrentLexerName(LPWSTR,int);
int    Style_GetLexerIconId(PEDITLEXER);
BOOL   Style_HasLexerForExt(LPCWSTR);
HTREEITEM Style_AddLexerToTreeView(HWND,PEDITLEXER);
INT_PTR CALLBACK Styles_ConfigDlgProc(HWND,UINT,WPARAM,LPARAM);
void   Style_ConfigDlg(HWND);
INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND,UINT,WPARAM,LPARAM);
void   Style_SelectLexerDlg(HWND);
int    Style_GetHotspotStyleID();
int    Style_StrGetWeightValue(LPCWSTR,int*);
void   Style_AppendWeightStr(LPWSTR, int, int);

#endif //_NP3_STYLES_H_

// End of Style.h
