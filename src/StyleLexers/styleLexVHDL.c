#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static __int64 LexFunction(LexFunctionType type, int value)
{
  LEX_FUNCTION_BODY(type, value);
  return 0LL;
};

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_VHDL = {
"access after alias all architecture array assert attribute begin block body buffer bus case component configuration "
"constant disconnect downto else elsif end entity exit file for function generate generic group guarded if impure in "
"inertial inout is label library linkage literal loop map new next null of on open others out package port postponed "
"procedure process pure range record register reject report return select severity shared signal subtype then "
"to transport type unaffected units until use variable wait when while with",
"abs and mod nand nor not or rem rol ror sla sll sra srl xnor xor",
"left right low high ascending image value pos val succ pred leftof rightof base range reverse_range length delayed stable "
"quiet transaction event active last_event last_active last_value driving driving_value simple_name path_name instance_name",
"now readline read writeline write endfile resolved to_bit to_bitvector to_stdulogic to_stdlogicvector to_stdulogicvector "
"to_x01 to_x01z to_UX01 rising_edge falling_edge is_x shift_left shift_right rotate_left rotate_right resize to_integer "
"to_unsigned to_signed std_match to_01",
"std ieee work standard textio std_logic_1164 std_logic_arith std_logic_misc std_logic_signed std_logic_textio std_logic_unsigned "
"numeric_bit numeric_std math_complex math_real vital_primitives vital_timing",
"boolean bit character severity_level integer real time delay_length natural positive string bit_vector file_open_kind "
"file_open_status line text side width std_ulogic std_ulogic_vector std_logic std_logic_vector X01 X01Z UX01 UX01Z unsigned signed",
"", "", "" };

EDITLEXER lexVHDL = { 
SCLEX_VHDL, IDS_LEX_VHDL, L"VHDL", L"vhdl; vhd", L"", 
&LexFunction, // static
&KeyWords_VHDL, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_VHDL_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { MULTI_STYLE(SCE_VHDL_COMMENTLINEBANG, SCE_VHDL_COMMENT, SCE_VHDL_BLOCK_COMMENT, 0), IDS_LEX_STR_63127, L"Comment", L"fore:#008800", L"" },
    { SCE_VHDL_NUMBER, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
    { MULTI_STYLE(SCE_VHDL_STRING, SCE_VHDL_STRINGEOL, 0, 0), IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
    { SCE_VHDL_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
    { SCE_VHDL_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    { SCE_VHDL_KEYWORD, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0A246A", L"" },
    { SCE_VHDL_STDOPERATOR, IDS_LEX_STR_63336, L"Standard Operator", L"bold; fore:#0A246A", L"" },
    { SCE_VHDL_ATTRIBUTE, IDS_LEX_STR_63337, L"Attribute", L"", L"" },
    { SCE_VHDL_STDFUNCTION, IDS_LEX_STR_63338, L"Standard Function", L"", L"" },
    { SCE_VHDL_STDPACKAGE, IDS_LEX_STR_63339, L"Standard Package", L"", L"" },
    { SCE_VHDL_STDTYPE, IDS_LEX_STR_63340, L"Standard Type", L"fore:#FF8000", L"" },
    EDITLEXER_SENTINEL } };
