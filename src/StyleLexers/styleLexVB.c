#include "StyleLexers.h"

KEYWORDLIST KeyWords_VB = {
"addhandler addressof alias and andalso ansi any as assembly auto boolean byref byte byval call "
"case catch cbool cbyte cchar cdate cdbl cdec char cint class clng cobj compare const cshort csng "
"cstr ctype date decimal declare default delegate dim directcast do double each else elseif end "
"enum erase error event exit explicit externalsource false finally for friend function get "
"gettype gosub goto handles if implements imports in inherits integer interface is let lib like "
"long loop me mid mod module mustinherit mustoverride mybase myclass namespace new next not "
"nothing notinheritable notoverridable object on option optional or orelse overloads overridable "
"overrides paramarray preserve private property protected public raiseevent randomize readonly "
"redim rem removehandler resume return select set shadows shared short single static step stop "
"strict string structure sub synclock then throw to true try typeof unicode until variant when "
"while with withevents writeonly xor",
"", "", "", "", "", "", "", "" };


EDITLEXER lexVB = { 
SCLEX_VB, IDS_LEX_VIS_BAS, L"Visual Basic", L"vb; bas; frm; cls; ctl; pag; dsr; dob", L"", 
&KeyWords_VB, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_B_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_B_COMMENT, IDS_LEX_STR_63127, L"Comment", L"fore:#808080", L"" },
    { SCE_B_KEYWORD, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#B000B0", L"" },
    { SCE_B_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    { MULTI_STYLE(SCE_B_STRING,SCE_B_STRINGEOL,0,0), IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
    { MULTI_STYLE(SCE_B_NUMBER,SCE_B_DATE,0,0), IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
    { SCE_B_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"", L"" },
    { SCE_B_PREPROCESSOR, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#FF9C00", L"" },
    //{ SCE_B_CONSTANT, L"Constant", L"", L"" },
    //{ SCE_B_KEYWORD2, L"Keyword 2", L"", L"" },
    //{ SCE_B_KEYWORD3, L"Keyword 3", L"", L"" },
    //{ SCE_B_KEYWORD4, L"Keyword 4", L"", L"" },
    //{ SCE_B_ASM, L"Inline Asm", L"fore:#FF8000", L"" },
    { -1, 00000, L"", L"", L"" } } };
