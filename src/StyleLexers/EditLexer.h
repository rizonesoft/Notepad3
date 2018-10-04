#ifndef _EDIT_LEXER_H_
#define _EDIT_LEXER_H_


#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>

#include "Scintilla.h"

// -----------------------------------------------------------------------------

#define INITIAL_BASE_FONT_SIZE (11.0f)

#define BUFZIZE_STYLE_EXTENTIONS 512
#define BUFSIZE_STYLE_VALUE 256

// -----------------------------------------------------------------------------

typedef enum
{
  FCT_SETTING_CHANGE, // value -b: rest style bit, 0: get bit-set, +b: set style bit
  FCT_PASS
}
LexFunctionType;

typedef __int64 (*LexFunctionPtr_t)(LexFunctionType type, int value);


// -----------------------------------------------------------------------------

typedef struct _editstyle
{
#pragma warning(disable : 4201)  // MS's Non-Std: Structure/Union w/o name
  union
  {
    INT32 iStyle;
    UINT8 iStyle8[4];
  };
  int rid;
  LPCWSTR pszName;
  LPCWSTR pszDefault;
  WCHAR   szValue[BUFSIZE_STYLE_VALUE];

} EDITSTYLE, *PEDITSTYLE;


typedef struct _keywordlist
{
  char* pszKeyWords[KEYWORDSET_MAX + 1];

} KEYWORDLIST, *PKEYWORDLIST;


#pragma warning(disable : 4200)  // MS's Non-Std: Null-Array in Structure/Union
typedef struct _editlexer
{
  int lexerID;
  int resID;
  LPCWSTR pszName;
  LPCWSTR pszDefExt;
  WCHAR  szExtensions[BUFZIZE_STYLE_EXTENTIONS];
  LexFunctionPtr_t pFctPtr;
  PKEYWORDLIST pKeyWords;
  EDITSTYLE    Styles[];

} EDITLEXER, *PEDITLEXER;

// -----------------------------------------------------------------------------

// Default Style (styleLexStandard.c) Indices
typedef enum {
  STY_DEFAULT = 0,
  STY_MARGIN = 1,
  STY_BRACE_OK = 2,
  STY_BRACE_BAD = 3,
  STY_CTRL_CHR = 4,
  STY_INDENT_GUIDE = 5,
  STY_SEL_TXT = 6,
  STY_WHITESPACE = 7,
  STY_CUR_LN_BCK = 8,
  STY_CARET = 9,
  STY_LONG_LN_MRK = 10,
  STY_X_LN_SPACE = 11,
  STY_BOOK_MARK = 12,
  STY_MARK_OCC = 13,
  STY_URL_HOTSPOT = 14,
  STY_IME_COLOR = 15,
  STY_INVISIBLE = 16,
  STY_READONLY = 17

  // MAX = 127
}
LexDefaultStyles;

#define _STYLE_GETSTYLEID(ID) ((int)(STYLE_MAX - ID))
#define Style_GetHotspotStyleID()   _STYLE_GETSTYLEID(STY_URL_HOTSPOT)
#define Style_GetInvisibleStyleID() _STYLE_GETSTYLEID(STY_INVISIBLE)
#define Style_GetReadonlyStyleID()  _STYLE_GETSTYLEID(STY_READONLY)


// -----------------------------------------------------------------------------

extern EDITLEXER lexStandard;      // Default Text
extern EDITLEXER lexStandard2nd;   // 2nd Default Text
extern EDITLEXER lexANSI;          // ANSI Files
extern EDITLEXER lexCONF;          // Apache Config Files
extern EDITLEXER lexASM;           // Assembly Script
extern EDITLEXER lexAHK;           // AutoHotkey Script
extern EDITLEXER lexAHKL;          // AutoHotkey L Script
extern EDITLEXER lexAU3;           // AutoIt3 Script
extern EDITLEXER lexAVS;           // AviSynth Script
extern EDITLEXER lexAwk;           // Awk Script
extern EDITLEXER lexBAT;           // Batch Files
extern EDITLEXER lexCS;            // C# Source Code
extern EDITLEXER lexCPP;           // C/C++ Source Code
extern EDITLEXER lexCmake;         // Cmake Script
extern EDITLEXER lexCOFFEESCRIPT;  // Coffeescript
extern EDITLEXER lexPROPS;         // Configuration Files
extern EDITLEXER lexCSS;           // CSS Style Sheets
extern EDITLEXER lexD;             // D Source Code
extern EDITLEXER lexDIFF;          // Diff Files
extern EDITLEXER lexGo;            // Go Source Code
extern EDITLEXER lexINNO;          // Inno Setup Script
extern EDITLEXER lexJAVA;          // Java Source Code
extern EDITLEXER lexJS;            // JavaScript
extern EDITLEXER lexJSON;          // JSON
extern EDITLEXER lexLATEX;         // LaTeX Files
extern EDITLEXER lexLUA;           // Lua Script
extern EDITLEXER lexMAK;           // Makefiles
extern EDITLEXER lexMARKDOWN;      // Markdown
extern EDITLEXER lexMATLAB;        // MATLAB
extern EDITLEXER lexNimrod;        // Nim(rod)
extern EDITLEXER lexNSIS;          // NSIS Script
extern EDITLEXER lexPAS;           // Pascal Source Code
extern EDITLEXER lexPL;            // Perl Script
extern EDITLEXER lexPS;            // PowerShell Script
extern EDITLEXER lexPY;            // Python Script
extern EDITLEXER lexRegistry;      // Registry Files
extern EDITLEXER lexRC;            // Resource Script
extern EDITLEXER lexR;             // R Statistics Code
extern EDITLEXER lexRUBY;          // Ruby Script
extern EDITLEXER lexRust;          // Rust Script
extern EDITLEXER lexBASH;          // Shell Script
extern EDITLEXER lexSQL;           // SQL Query
extern EDITLEXER lexTCL;           // Tcl Script
extern EDITLEXER lexVBS;           // VBScript
extern EDITLEXER lexVHDL;          // VHDL
extern EDITLEXER lexVB;            // Visual Basic
extern EDITLEXER lexHTML;          // Web Source Code
extern EDITLEXER lexXML;           // XML Document
extern EDITLEXER lexYAML;          // YAML

// -----------------------------------------------------------------------------

#endif // _EDIT_LEXER_H_
