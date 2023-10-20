#pragma once

#include "TypeDefs.h"
#include "Scintilla.h"

// -----------------------------------------------------------------------------

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
    const char* const pszKeyWords[KEYWORDSET_MAX + 1];

} KEYWORDLIST, *PKEYWORDLIST;


#pragma warning(disable : 4200)  // MS's Non-Std: Null-Array in Structure/Union
typedef struct _editlexer
{
    int          lexerID;      // Scintilla/Lexilla ID
    LPCSTR       lexerName;    // Lexilla Name (case sensitive)
    int          resID;        // language resource
    LPCWSTR      pszName;      // config/settings section
    LPCWSTR      pszDefExt;    // default file name ext (4 reset)
    WCHAR        szExtensions[STYLE_EXTENTIONS_BUFFER];
    PKEYWORDLIST pKeyWords;
    EDITSTYLE    Styles[];     // must be last

} EDITLEXER, *PEDITLEXER;

// -----------------------------------------------------------------------------

// Default Style (styleLexStandard.c) Indices
typedef enum LexDefaultStyles
{
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
    STY_UNICODE_HOTSPOT = 15,
    STY_MULTI_EDIT = 16,
    STY_CHGHIST_MODIFIED = 17,
    STY_CHGHIST_SAVED = 18,
    STY_CHGHIST_REV_TO_MOD = 19,
    STY_CHGHIST_REV_TO_ORG = 20,
    STY_IME_COLOR = 21,

    STY_INVISIBLE = 22,
    STY_READONLY = 23

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
extern EDITLEXER lexCSV;           // CSV Prism Color Lexer
extern EDITLEXER lexD;             // D Source Code
extern EDITLEXER lexDart;          // Dart Source Code
extern EDITLEXER lexDIFF;          // Diff Files
extern EDITLEXER lexFortran;       // Fortran F90+
//~extern EDITLEXER lexF77;           // Fortran F77
extern EDITLEXER lexGo;            // Go Source Code
extern EDITLEXER lexINNO;          // Inno Setup Script
extern EDITLEXER lexJAVA;          // Java Source Code
extern EDITLEXER lexJS;            // JavaScript
extern EDITLEXER lexJSON;          // JSON
extern EDITLEXER lexJulia;         // Julia
extern EDITLEXER lexKiX;           // KiX
extern EDITLEXER lexKotlin;        // Kotlin
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
extern EDITLEXER lexSysVerilog;    // SystemVerilog HDVL
extern EDITLEXER lexTCL;           // Tcl Script
extern EDITLEXER lexTOML;          // TOML Config Script
extern EDITLEXER lexVBS;           // VBScript
extern EDITLEXER lexVerilog;       // Verilog HDL
extern EDITLEXER lexVHDL;          // VHDL
extern EDITLEXER lexVB;            // Visual Basic
extern EDITLEXER lexHTML;          // Web Source Code
extern EDITLEXER lexXML;           // XML Document
extern EDITLEXER lexYAML;          // YAML


// -----------------------------------------------------------------------------
// common defines
// -----------------------------------------------------------------------------

#define NP3_LEXER_JS_KEYWORD_LIST \
"abstract as async await boolean break byte case catch char class const continue debugger default delete do double "\
"each else enum export extends false final finally float for from function get goto if implements import in instanceof int "\
"interface let long native new null of package private protected public return set short static super switch "\
"synchronized this throw throws transient true try typeof var void volatile while with yield"


#define NP3_LEXER_VB_KEYWORD_LIST \
"addhandler addressof alias and andalso ansi any as assembly attribute auto begin boolean byref byte byval "\
"call case catch cbool cbyte cchar cdate cdbl cdec char cint class clng cobj compare const continue cshort csng cstr ctype currency "\
"date decimal declare default delegate dim directcast do double each else elseif empty end enum eqv erase error event "\
"exit explicit externalsource false finally for friend function get gettype global gosub goto handles if "\
"imp implement implements imports in inherits integer interface is let lib like load long loop lset me mid mod module mustinherit "\
"mustoverride mybase myclass namespace new next not nothing notinheritable notoverridable null object on option "\
"optional or orelse overloads overridable overrides paramarray preserve private property protected public "\
"raiseevent randomize readonly redim rem removehandler resume return rset select set shadows shared short "\
"single static step stop strict string structure sub synclock then throw to true try type typeof unicode unload until "\
"variant wend when while with withevents writeonly xor"

// -----------------------------------------------------------------------------

void Lexer_GetStreamCommentStrgs(LPWSTR beg_out, LPWSTR end_out, size_t maxlen);
bool Lexer_GetLineCommentStrg(LPWSTR pre_out, size_t maxlen);
void Lexer_SetFoldingAvailability(PEDITLEXER pLexer);
void Lexer_SetFoldingProperties(bool active);
void Lexer_SetFoldingFocusedView();
void Lexer_SetLexerSpecificProperties(const int lexerId);

// -----------------------------------------------------------------------------
