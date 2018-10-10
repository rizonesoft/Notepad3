#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static __int64 LexFunction(LexFunctionType type, int value)
{
  LEX_FUNCTION_BODY(type, value);
  return 0LL;
};

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_PROPS = EMPTY_KEYWORDLIST;


EDITLEXER lexPROPS = { 
SCLEX_PROPERTIES, IDS_LEX_CONF, L"Configuration Files", L"ini; inf; cfg; properties; oem; sif; url; sed; theme", L"", 
&LexFunction, // static
&KeyWords_PROPS, {
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ {SCE_PROPS_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { {SCE_PROPS_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { {SCE_PROPS_SECTION}, IDS_LEX_STR_63232, L"Section", L"fore:#000000; back:#FF8040; bold; eolfilled", L"" },
    { {SCE_PROPS_ASSIGNMENT}, IDS_LEX_STR_63233, L"Assignment", L"fore:#FF0000", L"" },
    { {SCE_PROPS_DEFVAL}, IDS_LEX_STR_63234, L"Default Value", L"fore:#FF0000", L"" },
    EDITLEXER_SENTINEL } };
