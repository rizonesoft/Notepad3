#ifndef _EDIT_LEXER_H_
#define _EDIT_LEXER_H_


#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>

#include "Scintilla.h"

// -----------------------------------------------------------------------------

#define BUFZIZE_STYLE_EXTENTIONS 512
#define BUFSIZE_STYLE_VALUE 256


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
  STY_CUR_LN = 8,
  STY_CARET = 9,
  STY_LONG_LN_MRK = 10,
  STY_X_LN_SPACE = 11,
  STY_BOOK_MARK = 12,
  STY_MARK_OCC = 13,
  STY_URL_HOTSPOT = 14,
  STY_MULTI_EDIT = 15,
  STY_IME_COLOR = 17,

  STY_INVISIBLE = 17,
  STY_READONLY = 18

  // MAX = (127 - STYLE_LASTPREDEFINED)
  // -------^----- => char <-> int casting !!!
}
LexDefaultStyles;

#define _STYLE_GETSTYLEID(ID) ((STYLE_LASTPREDEFINED + 1) + ID)
#define Style_GetInvisibleStyleID() ((int)_STYLE_GETSTYLEID(STY_INVISIBLE))
#define Style_GetReadonlyStyleID()  ((int)_STYLE_GETSTYLEID(STY_READONLY))


// -----------------------------------------------------------------------------

extern EDITLEXER lexStandard;      // Default Text
extern EDITLEXER lexStandard2nd;   // 2nd Default Text
extern EDITLEXER lexTEXT;          // Pure Text Files
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
extern EDITLEXER lexNim;           // Nim(rod)
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
