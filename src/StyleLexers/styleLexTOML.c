#include "StyleLexers.h"

// ----------------------------------------------------------------------------

//KEYWORDLIST KeyWords_TOML = EMPTY_KEYWORDLIST;
KEYWORDLIST KeyWords_TOML = {
  "Keyword",
  "", "", "", "", "", "", "", "" };


EDITLEXER lexTOML = {
SCLEX_TOML, IDS_LEX_TOML_CFG, L"TOML Config", L"toml", L"",
&KeyWords_TOML,{
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ {SCE_TOML_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { {SCE_TOML_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { {SCE_TOML_KEY}, IDS_LEX_STR_63128, L"Key", L"bold; fore:#5E8F60", L"" },
    { {SCE_TOML_SECTION}, IDS_LEX_STR_63232, L"Section", L"bold; fore:#000000; back:#FF8040; eolfilled", L"" },
    { {SCE_TOML_ASSIGNMENT}, IDS_LEX_STR_63233, L"Assignment", L"fore:#FFA500", L"" },
    { {SCE_TOML_DEFVAL}, IDS_LEX_STR_63234, L"Default Value", L"fore:#00FF00", L"" },
    { {SCE_TOML_DATATYPE}, IDS_LEX_STR_63234, L"Datatype", L"fore:#0000FF", L"" },
    { {SCE_TOML_TYPEERROR}, IDS_LEX_STR_63234, L"Type Error", L"fore:#FF0000", L"" },
    EDITLEXER_SENTINEL } };
