#include "StyleLexers.h"

KEYWORDLIST KeyWords_JS = {
"abstract boolean break byte case catch char class const continue debugger default delete do "
"double else enum export extends false final finally float for function goto if implements "
"import in instanceof int interface let long native new null package private protected public "
"return short static super switch synchronized this throw throws transient true try typeof var "
"void volatile while with",
"", "", "", "", "", "", "", "" };


EDITLEXER lexJS = { 
SCLEX_CPP, IDS_LEX_J_SCR, L"JavaScript", L"js; jse; jsm; as", L"", 
&KeyWords_JS, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_C_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_C_COMMENT, IDS_LEX_STR_63127, L"Comment", L"fore:#646464", L"" },
    { SCE_C_WORD, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#A46000", L"" },
    { SCE_C_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    { MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM), IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
    { SCE_C_REGEX, IDS_LEX_STR_63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
    { SCE_C_NUMBER, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
    { SCE_C_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
    //{ SCE_C_UUID, L"UUID", L"", L"" },
    //{ SCE_C_PREPROCESSOR, L"Preprocessor", L"fore:#FF8000", L"" },
    //{ SCE_C_WORD2, L"Word 2", L"", L"" },
    //{ SCE_C_GLOBALCLASS, L"Global Class", L"", L"" },
    EDITLEXER_SENTINEL } };

