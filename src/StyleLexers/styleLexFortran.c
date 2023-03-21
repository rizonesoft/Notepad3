#include "StyleLexers.h"

// ----------------------------------------------------------------------------

// https://en.wikibooks.org/wiki/Fortran
// https://releases.llvm.org/11.0.0/tools/flang/docs/Intrinsics.html
// https://fortranwiki.org/fortran/show/Keywords
// Fortran Standard 2018

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_Fortran =
{
    // --- Primary keywords and identifiers (Keyword) ---
    "abstract access action advance all allocatable allocate apostrophe assign assignment associate asynchronous "
    "backspace bind blank block blockdata call case change character class close codimension common complex concurrent "
    "contains contiguous continue critical cycle "
    "data deallocate decimal delim default deferred dimension direct do dowhile double doubleprecision "
    "elemental else elseif elsewhere encoding end endassociate endblock endblockdata endcritical enddo endenum endfile "
    "endforall endfunction endif endinterface endmodule endprogram endprocedure endselect endsubroutine endsubmodule "
    "endteam endtype endwhere entry enum enumerator eor equivalence err error errmsg event exist exit extends external "
    "fail file final flush fmt forall form format formatted function generic go goto "
    "id if im image images implicit import impure in include inout integer inquire intent interface intrinsic iomsg "
    "iolength iostat kind len lock logical memory module name named namelist nextrec newunit nml none "
    "non_intrinsic non_overridable nopass nullify number only open opened operator optional out "
    "pad parameter pass pause pending pointer pos post position precision print private procedure program protected public pure "
    "quote rank re readwrite read real rec recl recursive result return rewind round "
    "save select selectcase selecttype selectrank sequence sequential sign size stat status stop stream subroutine submodule "
    "sync target team then to type unformatted unit unlock use "
    "value volatile wait where while write",

    // --- Intrinsic functions (Standard Function) - Fortran Standard 2018 ---
    "abs achar acos acosh adjustl adjustr aimag aint all allocated anint any asin asinh associated atan atan2 atanh "
    "atomic_add atomic_and atomic_cas atomic_define atomic_fetch_add atomic_fetch_and atomic_fetch_or atomic_fetch_xor atomic_or "
    "atomic_ref atomic_xor bessel_j0 bessel_j1 bessel_jn bessel_y0 bessel_y1 bessel_yn bge bgt bit_size ble blt btest "
    "ceiling char cmplx co_broadcast co_max co_min co_reduce co_sum command_argument_count conjg cos cosh coshape count cpu_time "
    "cshift date_and_time dble digits dim dot_product dprod dshiftl dshiftr eoshift epsilon erf erfc erfc_scaled event_query "
    "execute_command_line exp exponent extends_type_of failed_images findloc floor fraction gamma "
    "get_command get_command_argument get_environment_variable get_team "
    "huge hypot iachar iall iand iany ibclr ibits ibset ichar ieor image_index image_status index int ior iparity ishft ishftc "
    "is_contiguous is_iostat_end is_iostat_eor kind lbound lcobound leadz len len_trim lge lgt lle llt log log_gamma log10 "
    "logical maskl maskr matmul max maxexponent maxloc maxval merge merge_bits min minexponent minloc minval mod modulo "
    "move_alloc mvbits nearest new_line nint norm2 not null num_images out_of_range pack parity popcnt poppar precision "
    "present product radix random_init random_number random_seed range rank real reduce repeat reshape rrspacing "
    "same_type_as scale scan selected_char_kind selected_int_kind selected_real_kind set_exponent "
    "shape shift shifta shiftl shiftr sign sin sinh size spacing spread sqrt stopped_images storage_size sum "
    "system_clock tan tanh team_number this_image tiny trailz transfer transpose trim ubound ucobound unpack verify "
    // Unrestricted specific names for standard intrinsic procedures
    "alog alog10 amod anint asin cabs ccos cexp clog csin csqrt dabs dacos dasin datan datan2 dcos dcosh ddim dexp "
    "dint dlog dlog10 dmod dnint dsign dsin dsinh dsqrt dtan dtanh iabs idim idnint isign "
    // Restricted specific names for standard intrinsic procedures
    "amax0 amax1 amin0 amin1 dmax1 dmin1 float idint ifix max0 max1 min0 min1 sngl",
 
    // --- Extended and user defined functions (Function) ---
    // Intrinsic procedures for the IEEE_ARITHMETIC module
    "ieee_class ieee_copy_sign ieee_fma ieee_get_rounding_mode ieee_get_underflow_mode ieee_int ieee_is_finite "
    "ieee_nan ieee_is_negative ieee_is_normal ieee_logb ieee_max_num ieee_max_num_mag ieee_min_num ieee_min_num_mag "
    "ieee_next_after ieee_next_down ieee_next_up ieee_quiet_eq ieee_quiet_ge ieee_quiet_gt ieee_quiet_le ieee_quiet_lt "
    "ieee_quiet_ne ieee_real ieee_rem ieee_rint ieee_scalb ieee_selected_real_kind ieee_set_rounding_mode "
    "ieee_set_underflow_mode ieee_signaling_eq ieee_signaling_ge ieee_singaling_gt ieee_singaling_le ieee_singaling_lt "
    "ieee_singaling_ne ieee_signbit ieee_support_datatype ieee_support_denormal ieee_support_divide ieee_support_inf "
    "ieee_support_io ieee_support_rounding ieee_support_sqrt ieee_support_subnormal ieee_support_standard "
    "ieee_support_underflow_control ieee_unordered ieee_value "
    // Intrinsic procedures for the IEEE_EXCEPTIONS module
    "ieee_get_flag ieee_get_halting_mode ieee_get_modes ieee_get_status ieee_set_halting_mode ieee_set_modes ieee_set_status "
    "ieee_support_flag ieee_support_halting_mode "
    // Intrinsic procedures for the ISO_FORTRAN_ENV module
    "compiler_options compiler_version "
    // Intrinsic procedures for the ISO_C_BINDING module
    "c_associated c_f_pointer c_f_procpointer c_funloc c_loc c_sizeof",

    NULL,
};


EDITLEXER lexFortran =
{
    SCLEX_FORTRAN, "fortran", IDS_LEX_FORTRAN, L"Fortran Source Code", L"f; for; ftn; fpp; f90; f95; f03; f08; f15; f2k; hf", L"",
    &KeyWords_Fortran, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_F_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_F_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_F_PREPROCESSOR}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF8000", L"" },
        { {SCE_F_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"italic; fore:#808080", L"" },
        { {SCE_F_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#1242BE", L"" },
        { {SCE_F_WORD2}, IDS_LEX_STR_Function, L"Function", L"bold; italic; fore:#8000FF", L"" },
        { {SCE_F_WORD3}, IDS_LEX_STR_63305, L"User-Defined Function", L"bold; italic; fore:#800080", L"" },
        { {SCE_F_STRING1}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#008800", L"" },
        { {SCE_F_STRING2}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008800", L"" },
        { {SCE_F_STRINGEOL}, IDS_LEX_STR_StrgEOL, L"String EOL", L"fore:#008000", L"" },
        { {SCE_F_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#BD3000", L"" },
        { {MULTI_STYLE(SCE_F_OPERATOR,SCE_F_OPERATOR2,0,0)}, IDS_LEX_STR_Operator, L"Operator", L"bold; fore:#A60053", L"" },
        { {SCE_F_LABEL}, IDS_LEX_STR_Label, L"Label", L"fore:#C10061", L"" },
        { {SCE_F_CONTINUATION}, IDS_LEX_STR_63376, L"Continuation", L"fore:#00DDDD", L"" },
        EDITLEXER_SENTINEL
    }
};


#if 0

KEYWORDLIST KeyWords_F77 = {
    // Primary keywords and identifiers
    "access action advance allocatable allocate apostrophe assign assignment associate asynchronous "
    "backspace bind blank blockdata call case character class close common complex contains continue cycle "
    "data deallocate decimal delim default dimension direct do dowhile double doubleprecision "
    "else elseif elsewhere encoding end endassociate endblockdata enddo endfile endforall endfunction "
    "endif endinterface endmodule endprogram endselect endsubroutine endtype endwhere entry eor "
    "equivalence err errmsg exist exit external file flush fmt forall form format formatted function "
    "go goto id if implicit in include inout integer inquire intent interface intrinsic iomsg iolength iostat "
    "kind len logical module name named namelist nextrec nml none nullify number only open opened operator optional out "
    "pad parameter pass pause pending pointer pos position precision print private program protected public "
    "quote readwrite read real rec recl recursive result return rewind save select selectcase selecttype sequential "
    "sign size stat status stop stream subroutine target then to type unformatted unit use value volatile wait where while write",

    // Intrinsic functions
    "abs acos aimag aint anint asin atan atan2 char cmplx cmplx conjg cos cosh "
    "dble dim dprod exp ichar index int len lge lgt lle llt log log10 "
    "max min mod nint real real sign sin sinh sqrt tan tanh",

    // Extended and user defined functions
    "",

    NULL,
};


EDITLEXER lexF77 = {
    SCLEX_F77, "f77", IDS_LEX_F77, L"F77", L"f; for", L"",
    &KeyWords_F77, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_F_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_F_PREPROCESSOR}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF8000", L"" },
        { {SCE_F_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#880000", L"" },
        { {SCE_F_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#0A246A", L"" },
        { {MULTI_STYLE(SCE_F_WORD2,SCE_F_WORD3,0,0)}, IDS_LEX_STR_Keyword2nd, L"Keyword 2nd", L"bold; italic; fore:#3C6CDD", L"" },
        { {SCE_F_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_F_STRINGEOL}, IDS_LEX_STR_String, L"String EOL", L"fore:#008000", L"" },
        { {SCE_F_STRING1}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#008800", L"" },
        { {SCE_F_STRING2}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008800", L"" },
        { {SCE_F_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF4000", L"" },
        { {MULTI_STYLE(SCE_F_OPERATOR,SCE_F_OPERATOR2,0,0)}, IDS_LEX_STR_Operator, L"Operator", L"bold; fore:#666600", L"" },
        { {SCE_F_LABEL}, IDS_LEX_STR_Label, L"Label", L"fore:#0000DD", L"" },
        { {SCE_F_CONTINUATION}, IDS_LEX_STR_63376, L"Continuation", L"fore:#00DDDD", L"" },
        EDITLEXER_SENTINEL
    }
};

#endif

