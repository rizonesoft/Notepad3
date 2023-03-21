#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_Rust =
{
// Primary keywords and identifiers
    "as be break const continue crate else enum extern false fn for if impl in let loop match mod mut once pub "
    "ref return self static struct super trait true type unsafe use while",
// Built in types
    "bool char f32 f64 i16 i32 i64 i8 int str u16 u32 u64 u8 uint",
// Other keywords
    "abstract alignof become box do final macro offsetof override priv proc pure sizeof typeof unsized "
    "virtual yield",
// Keywords 4
    "",
// Keywords 5
    "",
// Keywords 6
    "",
// Keywords 7
    "",
    NULL,
};


EDITLEXER lexRust =
{
    SCLEX_RUST, "rust", IDS_LEX_RUST_SRC, L"Rust Source Code", L"rs; rust", L"",
    &KeyWords_Rust,{
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_RUST_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_RUST_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_RUST_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#248112", L"" },
        { {SCE_RUST_WORD2}, IDS_LEX_STR_63343, L"Build-In Type", L"fore:#A9003D", L"" },
        { {SCE_RUST_WORD3}, IDS_LEX_STR_63345, L"Other Keyword", L"italic; fore:#248112", L"" },
        //{ {SCE_RUST_WORD4}, IDS_LEX_STR_Keyword, L"Keyword 4", L"bold; fore:#0A246A", L"" },
        //{ {SCE_RUST_WORD5}, IDS_LEX_STR_Keyword, L"Keyword 5", L"bold; fore:#0A246A", L"" },
        //{ {SCE_RUST_WORD6}, IDS_LEX_STR_Keyword, L"Keyword 6", L"bold; fore:#0A246A", L"" },
        //{ {SCE_RUST_WORD}, IDS_LEX_STR_Keyword, L"Keyword 7", L"bold; fore:#0A246A", L"" },
        { {SCE_RUST_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#666666", L"" },
        { {MULTI_STYLE(SCE_RUST_COMMENTBLOCK,SCE_RUST_COMMENTLINE,SCE_RUST_COMMENTBLOCKDOC,SCE_RUST_COMMENTLINEDOC)}, IDS_LEX_STR_Comment, L"Comment", L"italic; fore:#488080", L"" },
        { {MULTI_STYLE(SCE_RUST_STRING,SCE_RUST_STRINGR,SCE_RUST_CHARACTER,0)}, IDS_LEX_STR_String, L"String", L"fore:#B31C1B", L"" },
        { {SCE_RUST_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#666666", L"" },
        { {SCE_RUST_MACRO}, IDS_LEX_STR_63280, L"Macro Definition", L"fore:#0A246A", L"" },
        { {SCE_RUST_LIFETIME}, IDS_LEX_STR_63346, L"Rust Lifetime", L"fore:#B000B0", L"" },
        { {SCE_RUST_LEXERROR}, IDS_LEX_STR_63252, L"Parsing Error", L"fore:#F0F0F0; back:#F00000", L"" },
        { {MULTI_STYLE(SCE_RUST_BYTESTRING,SCE_RUST_BYTESTRINGR,SCE_RUST_BYTECHARACTER,0)}, IDS_LEX_STR_63344, L"Byte String", L"fore:#C0C0C0", L"" },
        EDITLEXER_SENTINEL
    }
};
