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
  int lexerID;
  int resID;
  WCHAR* pszName;
  WCHAR* pszDefExt;
  WCHAR  szExtensions[BUFZIZE_STYLE_EXTENTIONS];
  PKEYWORDLIST pKeyWords;
  EDITSTYLE    Styles[];

} EDITLEXER, *PEDITLEXER;


// Number of Lexers in pLexArray
#define NUMLEXERS 48
#define AVG_NUM_OF_STYLES_PER_LEXER 20


void   Style_Load();
void   Style_Save();
bool   Style_Import(HWND);
bool   Style_Export(HWND);
void   Style_SetLexer(HWND,PEDITLEXER);
void   Style_SetUrlHotSpot(HWND, bool);
void   Style_SetInvisible(HWND, bool);
void   Style_SetReadonly(HWND, bool);
void   Style_SetLongLineColors(HWND);
void   Style_SetCurrentLineBackground(HWND, bool);
void   Style_SetFolding(HWND, bool);
void   Style_SetBookmark(HWND, bool);
void   Style_SetMargin(HWND, int, LPCWSTR);
void   Style_SetLexerFromFile(HWND,LPCWSTR);
void   Style_SetLexerFromName(HWND,LPCWSTR,LPCWSTR);
void   Style_ResetCurrentLexer(HWND);
void   Style_SetDefaultLexer(HWND);
void   Style_SetHTMLLexer(HWND);
void   Style_SetXMLLexer(HWND);
void   Style_SetLexerFromID(HWND,int);
void   Style_SetDefaultFont(HWND,bool);
void   Style_ToggleUse2ndDefault(HWND);
bool   Style_GetUse2ndDefault();
void   Style_SetUse2ndDefault(bool);
void   Style_SetIndentGuides(HWND,bool);
void   Style_SetExtraLineSpace(HWND hwnd, LPWSTR lpszStyle, int size);
bool   Style_GetOpenDlgFilterStr(LPWSTR,int);
bool   Style_StrGetFont(LPCWSTR,LPWSTR,int);
bool   Style_StrGetFontQuality(LPCWSTR,LPWSTR,int);
bool   Style_StrGetCharSet(LPCWSTR,int*);
bool   Style_StrGetSize(LPCWSTR,float*);
bool   Style_StrGetSizeStr(LPCWSTR,LPWSTR,int);
bool   Style_StrGetColor(bool,LPCWSTR, COLORREF*);
bool   Style_StrGetCase(LPCWSTR,int*);
bool   Style_StrGetAlpha(LPCWSTR,int*,bool);
bool   Style_GetIndicatorType(LPWSTR,int,int*);
void   Style_CopyStyles_IfNotDefined(LPWSTR,LPWSTR,int,bool,bool);
bool   Style_SelectFont(HWND,LPWSTR,int,LPCWSTR,LPCWSTR,bool,bool,bool,bool);
bool   Style_SelectColor(HWND,bool,LPWSTR,int,bool);
void   Style_SetStyles(HWND,int,LPCWSTR,bool);
bool   Style_IsCurLexerStandard();
PEDITLEXER Style_GetCurrentLexerPtr();
int    Style_GetCurrentLexerRID();
void   Style_GetLexerDisplayName(PEDITLEXER pLexer, LPWSTR lpszName, int cchName);
void   Style_GetStyleDisplayName(PEDITSTYLE pStyle, LPWSTR lpszName, int cchName);
int    Style_GetLexerIconId(PEDITLEXER);
bool   Style_HasLexerForExt(LPCWSTR);
HTREEITEM Style_AddLexerToTreeView(HWND,PEDITLEXER);
INT_PTR CALLBACK Styles_ConfigDlgProc(HWND,UINT,WPARAM,LPARAM);
HWND   Style_CustomizeSchemesDlg(HWND);
INT_PTR CALLBACK Style_SelectLexerDlgProc(HWND,UINT,WPARAM,LPARAM);
void   Style_SelectLexerDlg(HWND);
int    Style_GetHotspotStyleID();
int    Style_GetInvisibleStyleID();
int    Style_GetReadonlyStyleID();
bool   Style_StrGetWeightValue(LPCWSTR,int*);
void   Style_AppendWeightStr(LPWSTR, int, int);

#endif //_NP3_STYLES_H_

// End of Style.h
