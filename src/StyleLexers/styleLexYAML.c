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

KEYWORDLIST KeyWords_YAML = { "y n yes no on off true false", "", "", "", "", "", "", "", "" };

EDITLEXER lexYAML = { 
SCLEX_YAML, IDS_LEX_YAML, L"YAML", L"yaml; yml", L"", 
&LexFunction, // static
&KeyWords_YAML, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_YAML_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_YAML_COMMENT, IDS_LEX_STR_63127, L"Comment", L"fore:#008800", L"" },
    { SCE_YAML_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"bold; fore:#0A246A", L"" },
    { SCE_YAML_KEYWORD, IDS_LEX_STR_63128, L"Keyword", L"fore:#880088", L"" },
    { SCE_YAML_NUMBER, IDS_LEX_STR_63130, L"Number", L"fore:#FF8000", L"" },
    { SCE_YAML_REFERENCE, IDS_LEX_STR_63333, L"Reference", L"fore:#008888", L"" },
    { SCE_YAML_DOCUMENT, IDS_LEX_STR_63334, L"Document", L"fore:#FFFFFF; bold; back:#000088; eolfilled", L"" },
    { SCE_YAML_TEXT, IDS_LEX_STR_63335, L"Text", L"fore:#404040", L"" },
    { SCE_YAML_ERROR, IDS_LEX_STR_63261, L"Error", L"fore:#FFFFFF; bold; italic; back:#FF0000; eolfilled", L"" },
    { SCE_YAML_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"fore:#333366", L"" },
    EDITLEXER_SENTINEL } };
