#include "StyleLexers.h"

KEYWORDLIST KeyWords_JSON = {
"false true null",
"@id @context @type @value @language @container @list @set @reverse @index @base @vocab @graph",
"", "", "", "", "", "", "" };


EDITLEXER lexJSON = { 
SCLEX_JSON, IDS_LEX_JSON, L"JSON", L"json; eslintrc; jshintrc; jsonld", L"", 
&KeyWords_JSON, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_C_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_C_COMMENT, IDS_LEX_STR_63127, L"Comment", L"fore:#646464", L"" },
    { SCE_C_WORD, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#A46000", L"" },
    { SCE_C_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    { SCE_JSON_STRING, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
    { SCE_C_REGEX, IDS_LEX_STR_63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
    { SCE_JSON_NUMBER, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
    { SCE_C_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
    EDITLEXER_SENTINEL } };

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
