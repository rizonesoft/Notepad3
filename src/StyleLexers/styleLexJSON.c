#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_JSON =
{
    "Infinity NaN false null true",
    "@base @container @context @direction @graph @id @import @included @index @json @language @list @nest @none "
    "@prefix @propagate @protected @reverse @set @type @value @version @vocab",
    NULL,
};


EDITLEXER lexJSON =
{
    SCLEX_JSON, "json", IDS_LEX_JSON, L"JSON", L"json; har; ipynb; wxcp; jshintrc; eslintrc; babelrc; prettierrc; stylelintrc; jsonld; jsonc; arcconfig; arclint; jscop", L"",
    &KeyWords_JSON, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_JSON_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_JSON_LINECOMMENT,SCE_JSON_BLOCKCOMMENT,0,0)}, IDS_LEX_STR_63127, L"Comment", L"fore:#646464", L"" },
        { {SCE_JSON_KEYWORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#957000", L"" },
        { {SCE_JSON_LDKEYWORD}, IDS_LEX_STR_63365, L"LD Keyword", L"bold; fore:#A61D04", L"" },
        { {MULTI_STYLE(SCE_JSON_STRING,SCE_JSON_STRINGEOL,0,0)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        //{ {SCE_C_REGEX}, IDS_LEX_STR_63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_JSON_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {SCE_JSON_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_JSON_PROPERTYNAME}, IDS_LEX_STR_63364, L"Property Name", L"fore:#002697", L"" },
        { {SCE_JSON_ESCAPESEQUENCE}, IDS_LEX_STR_63366, L"ESC Sequence", L"fore:#0B982E", L"" },
        { {SCE_JSON_ERROR}, IDS_LEX_STR_63252, L"Parsing Error", L"fore:#FFFF00; back:#A00000; eolfilled", L"" },
        EDITLEXER_SENTINEL
    }
};

/*
# String
style.json.2=fore:#7F0000
# Unclosed string       SCE_JSON_STRINGEOL
style.json.3=fore:#FFFFFF,back:#FF0000,eolfilled
# Property name         SCE_JSON_PROPERTYNAME
style.json.4=fore:#880AE8
# Escape sequence       SCE_JSON_ESCAPESEQUENCE
style.json.5=fore:#0B982E
# Line comment          SCE_JSON_LINECOMMENT
style.json.6=fore:#05BBAE,italic
# Block comment         SCE_JSON_BLOCKCOMMENT
style.json.7=$(style.json.6)
# Operator              SCE_JSON_OPERATOR
style.json.8=fore:#18644A
# URL/IRI               SCE_JSON_URI
style.json.9=fore:#0000FF
# JSON-LD compact IRI   SCE_JSON_COMPACTIRI
style.json.10=fore:#D137C1
# JSON keyword          SCE_JSON_KEYWORD
style.json.11=fore:#0BCEA7,bold
# JSON-LD keyword       SCE_JSON_LDKEYWORD
style.json.12=fore:#EC2806
# Parsing error         SCE_JSON_ERROR
style.json.13=fore:#FFFFFF,back:#FF0000
*/
