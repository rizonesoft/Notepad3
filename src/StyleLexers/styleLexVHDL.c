#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_VHDL =
{
    "access after alias all architecture array assert attribute begin block body buffer bus case component "
    "configuration constant disconnect downto else elsif end entity exit file for function generate generic "
    "group guarded if impure in inertial inout is label library linkage literal loop map new next null of on "
    "open others out package port postponed procedure process pure range record register reject report return "
    "select severity shared signal subtype then to transport type unaffected units until use variable wait "
    "when while with",
    "abs and mod nand nor not or rem rol ror sla sll sra srl xnor xor",
    "active ascending base delayed driving driving_value endfile event falling_edge high image instance_name, "
    "is_x last_active last_event last_value left leftof length low now path_name pos pred quiet range read "
    "readline resize resolved reverse_range right rightof rising_edge rotate_left rotate_right shift_left "
    "shift_right simple_name stable std_match succ to_01 to_UX01 to_bit to_bitvector to_integer to_signed "
    "to_stdlogicvector to_stdulogic to_stdulogicvector to_unsigned to_x01 to_x01z transaction val value "
    "write writeline",
    "ieee math_complex math_real numeric_bit numeric_std standard std std_logic_1164 std_logic_arith "
    "std_logic_misc std_logic_signed std_logic_textio std_logic_unsigned textio vital_primitives "
    "vital_timing work",
    "UX01 UX01Z X01 X01Z bit bit_vector boolean character delay_length file_open_kind file_open_status integer "
    "line natural positive real severity_level side signed std_logic std_logic_vector std_ulogic "
    "std_ulogic_vector string text time unsigned width",
    NULL,
};

EDITLEXER lexVHDL =
{
    SCLEX_VHDL, "vhdl", IDS_LEX_VHDL, L"VHDL", L"vhdl; vhd", L"",
    &KeyWords_VHDL, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_VHDL_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_VHDL_COMMENTLINEBANG, SCE_VHDL_COMMENT, SCE_VHDL_BLOCK_COMMENT, 0)}, IDS_LEX_STR_63127, L"Comment", L"fore:#008800", L"" },
        { {SCE_VHDL_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {MULTI_STYLE(SCE_VHDL_STRING, SCE_VHDL_STRINGEOL, 0, 0)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_VHDL_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_VHDL_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {SCE_VHDL_KEYWORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0A246A", L"" },
        { {SCE_VHDL_STDOPERATOR}, IDS_LEX_STR_63336, L"Standard Operator", L"bold; fore:#0A246A", L"" },
        { {SCE_VHDL_ATTRIBUTE}, IDS_LEX_STR_63337, L"Attribute", L"", L"" },
        { {SCE_VHDL_STDFUNCTION}, IDS_LEX_STR_63338, L"Standard Function", L"", L"" },
        { {SCE_VHDL_STDPACKAGE}, IDS_LEX_STR_63339, L"Standard Package", L"", L"" },
        { {SCE_VHDL_STDTYPE}, IDS_LEX_STR_63340, L"Standard Type", L"fore:#FF8000", L"" },
        EDITLEXER_SENTINEL
    }
};
