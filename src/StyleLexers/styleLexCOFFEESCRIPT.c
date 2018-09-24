#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static __int64 LexFunction(LexFunctionType type, int value)
{
  static __int64 iStyleChanged = 0LL;

  switch (type)
  {
  case FCT_SETTING_CHANGE:
    if (value == 0) {
      return iStyleChanged;
    }
    else if (value > 0) {
      iStyleChanged |= (((__int64)1) << value);
    }
    else {  // value < 0
      iStyleChanged &= ~(((__int64)1) << (0 - value));
    }
    break;

  default:
    break;
  }
  return (__int64)0;
};

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_COFFEESCRIPT = EMPTY_KEYWORDLIST;


EDITLEXER lexCOFFEESCRIPT = { 
SCLEX_COFFEESCRIPT, IDS_LEX_COFFEE_SCR, L"Coffeescript", L"coffee; Cakefile", L"", 
&LexFunction, // static
&KeyWords_COFFEESCRIPT, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_COFFEESCRIPT_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { MULTI_STYLE(SCE_COFFEESCRIPT_COMMENT,SCE_COFFEESCRIPT_COMMENTLINE,SCE_COFFEESCRIPT_COMMENTDOC,SCE_COFFEESCRIPT_COMMENTBLOCK), IDS_LEX_STR_63127, L"Comment", L"fore:#646464", L"" },
    { MULTI_STYLE(SCE_COFFEESCRIPT_STRING,SCE_COFFEESCRIPT_STRINGEOL,SCE_COFFEESCRIPT_STRINGRAW,0), IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
    { SCE_COFFEESCRIPT_PREPROCESSOR, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#FF8000", L"" },
    { SCE_COFFEESCRIPT_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"bold; fore:#0A246A", L"" },
    { SCE_COFFEESCRIPT_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
    { SCE_COFFEESCRIPT_NUMBER, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
    //{ SCE_COFFEESCRIPT_CHARACTER, IDS_LEX_STR_63376, L"Character", L"", L"" },
    { MULTI_STYLE(SCE_COFFEESCRIPT_REGEX,SCE_COFFEESCRIPT_VERBOSE_REGEX,SCE_COFFEESCRIPT_VERBOSE_REGEX_COMMENT,0), IDS_LEX_STR_63315, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
    { SCE_COFFEESCRIPT_GLOBALCLASS, IDS_LEX_STR_63304, L"Global Class", L"", L"" },
    //{ MULTI_STYLE(SCE_COFFEESCRIPT_COMMENTLINEDOC,SCE_COFFEESCRIPT_COMMENTDOCKEYWORD,SCE_COFFEESCRIPT_COMMENTDOCKEYWORDERROR,0), IDS_LEX_STR_63379, L"Comment line", L"fore:#646464", L"" },
    { MULTI_STYLE(SCE_COFFEESCRIPT_WORD,SCE_COFFEESCRIPT_WORD2,0,0), IDS_LEX_STR_63341, L"Word", L"", L"" },
    { MULTI_STYLE(SCE_COFFEESCRIPT_VERBATIM,SCE_COFFEESCRIPT_TRIPLEVERBATIM,0,0), IDS_LEX_STR_63342, L"Verbatim", L"", L"" },
    EDITLEXER_SENTINEL } };
