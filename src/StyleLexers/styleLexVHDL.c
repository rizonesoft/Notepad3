#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static KEYWORDLIST KeyWords_VHDL = {

    // 0 Keywords
    "access after alias all architecture array assert assume assume_guarantee attribute begin block body buffer bus case component "
    "configuration constant context cover default disconnect downto else elsif end entity exit fairness file for force function generate generic "
    "group guarded if impure in inertial inout is label library linkage literal loop map new next null of on "
    "open others out package parameter port postponed private procedure process property protect protected pure "
    "range record register reject release report restrict restrict_guarantee return "
    "select sequence severity shared signal strong subtype then to transport type unaffected units until use variable view vmode vpkg vprop vunit wait "
    "when while with",

    // 1 Operators
    "abs and mod nand nor not or rem rol ror sla sll sra srl xnor xor",

    // 2 Attributes
    "active ascending base converse delayed designated_subtype driving driving_value endfile element event falling_edge foreign high image index instance_name, "
    "is_x last_active last_event last_value left leftof length low now path_name pos pred quiet range read record "
    "readline reflect resize resolved reverse_range right rightof rising_edge rotate_left rotate_right shift_left "
    "shift_right signal simple_name stable std_match subtype succ to_01 to_UX01 to_bit to_bitvector to_integer to_signed "
    "to_stdlogicvector to_stdulogic to_stdulogicvector to_unsigned to_x01 to_x01z transaction val value "
    "write writeline",

    // 3 Functions
    "add add_carry arccos arccosh arcsin arcsinh arctan arctanh arg "
    "binary_read binary_write bitstoreal bread break_number bwrite "
    "cbrt ceil classfp cmplx complex_to_polar conj copysign cos cosh "
    "deallocate dir_close dir_createdir dir_deletedir dir_deletefile dir_itemexists dir_itemisdir dir_itemisfile "
    "dir_open dir_workingdir divide dividebyp2 "
    "endfile epoch epoch eq exp "
    "falling_edge file_close file_line file_mode file_name file_open file_path file_rewind file_seek file_size "
    "file_tell file_truncate find_leftmost find_rightmost finish finish finite floor flush "
    "from_binary_string from_bstring from_hex_string from_hstring from_octal_string from_ostring from_string "
    "ge get_call_path get_principal_value getenv gmtime gmtime gt hex_read hex_write hread hwrite "
    "is_negative is_x isnan justify le localtime localtime log log10 log2 logb lt "
    "mac maximum minimum modulo multiply nanfp ne neg_inffp neg_zerofp nextafter normalize now "
    "octal_read octal_write oread owrite polar_to_complex pos_inffp qnanfp "
    "read readline realmax realmin realtobits reciprocal remainder resize resolution_limit resolved rising_edge "
    "rotate_left rotate_right round "
    "saturate scalb seconds_to_time sfix_high sfix_low sfixed_high sfixed_low shift_left shift_right "
    "sign sin sinh sqrt sread std_match stop stop string_read string_write subtract swrite "
    "tan tanh tee time_to_seconds to_01 to_binary_string to_bit to_bit_vector to_bitvector to_bstring to_bv "
    "to_float to_float128 to_float32 to_float64 to_hex_string to_hstring to_integer to_octal_string to_ostring "
    "to_real to_sfix to_sfixed to_signed to_slv "
    "to_std_logic_vector to_std_ulogic_vector to_stdlogicvector to_stdulogic to_stdulogicvector to_string to_sulv "
    "to_ufix to_ufixed to_unsigned to_ux01 to_x01 to_x01z tool_edition tool_name tool_type tool_vendor tool_version "
    "trunc "
    "ufix_high ufix_low ufixed_high ufixed_low uniform unordered vhdl_version write writeline zerofp",

    // 4 Packages
    "env fixed_float_types fixed_generic_pkg fixed_pkg float_generic_pkg float_pkg ieee math_complex math_real "
    "numeric_bit numeric_bit_unsigned numeric_std reflection standard std std_logic_1164 std_logic_arith std_logic_misc "
    "std_logic_signed, std_logic_textio std_logic_unsigned textio vital_primitives vital_timing work",

    // 5 Types
    "bit bit_vector boolean boolean_vector "
    "call_path_element call_path_vector call_path_vector_ptr character complex complex_polar "
    "dayofweek delay_length dir_create_status dir_delete_status dir_open_status direction directory directory_items "
    "file_delete_status file_open_kind file_open_state file_open_status file_origin_kind "
    "fixed_overflow_style_type fixed_round_style_type float float128 float32 float64 "
    "integer integer_vector line line_vector natural positive positive_real principal_value real real_vector round_type "
    "severity_level sfixed side signed std_logic std_logic_vector std_ulogic std_ulogic_vector string "
    "text time time_record time_vector "
    "u_float u_float128 u_float32 u_float64 u_sfixed u_signed u_ufixed u_unsigned ufixed "
    "unresolved_float unresolved_float128 unresolved_float32 unresolved_float64 unresolved_sfixed unresolved_signed "
    "unresolved_ufixed unresolved_unsigned unsigned ux01 ux01z "
    "valid_fpstate width x01 x01z",

    // 6 User (Constants)
    "append_mode ascending descending dir_separator error "
    "failure false "
    "file_origin_begin file_origin_current file_origin_end fixed_round fixed_saturate fixed_truncate fixed_wrap "
    "fphdlsynth_or_real friday "
    "input isx left left_index "
    "math_1_over_e math_1_over_pi math_1_over_sqrt_2 math_2_pi math_3_pi_over_2 math_cbase_1 math_cbase_j math_czero "
    "math_deg_to_rad math_e math_log10_of_e math_log2_of_e math_log_of_10 math_log_of_2 "
    "math_pi math_pi_over_2 math_pi_over_3 math_pi_over_4 math_rad_to_deg math_sqrt_2 math_sqrt_pi mode_error monday "
    "name_error nan neg_denormal neg_inf neg_normal neg_zero note open_ok output pos_denormal pos_inf pos_normal pos_zero "
    "quiet_nan read_mode read_write_mode resolved right right_index round_inf round_nearest round_neginf round_zero "
    "saturday state_closed state_open status_access_denied status_error status_item_exists "
    "status_no_directory status_no_file status_not_empty status_not_found status_ok sunday "
    "thursday true tuesday warning wednesday write_mode",

    NULL,
};



EDITLEXER lexVHDL =
{
    SCLEX_VHDL, "vhdl", IDS_LEX_VHDL, L"VHDL", L"vhdl; vhd", L"",
    &KeyWords_VHDL, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_VHDL_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_VHDL_COMMENT, SCE_VHDL_COMMENTLINEBANG, SCE_VHDL_BLOCK_COMMENT, 0)}, IDS_LEX_STR_63127, L"Comment", L"fore:#008800", L"" },
        { {SCE_VHDL_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {MULTI_STYLE(SCE_VHDL_STRING, SCE_VHDL_STRINGEOL, 0, 0)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_VHDL_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_VHDL_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {SCE_VHDL_KEYWORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0000FF", L"" },
        { {SCE_VHDL_STDOPERATOR}, IDS_LEX_STR_63336, L"Standard Operator", L"bold; fore:#0A246A", L"" },
        { {SCE_VHDL_ATTRIBUTE}, IDS_LEX_STR_63337, L"Attribute", L"", L"" },
        { {SCE_VHDL_STDFUNCTION}, IDS_LEX_STR_63338, L"Standard Function", L"", L"" },
        { {SCE_VHDL_STDPACKAGE}, IDS_LEX_STR_63339, L"Standard Package", L"", L"" },
        { {SCE_VHDL_STDTYPE}, IDS_LEX_STR_63340, L"Standard Type", L"fore:#FF8000", L"" },
        { {SCE_VHDL_USERWORD}, IDS_LEX_STR_63274, L"Constant", L"fore:#A400A4", L"" },
        EDITLEXER_SENTINEL
    }
};
