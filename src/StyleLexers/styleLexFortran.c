#include "StyleLexers.h"

// ----------------------------------------------------------------------------

// https://en.wikibooks.org/wiki/Fortran
// https://releases.llvm.org/11.0.0/tools/flang/docs/Intrinsics.html

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_Fortran =
{
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
    "achar adjustl adjustr aimag aint alog alog10 amax0 amax1 amin0 amin1 amod anint associated atan2 baddress "
    "bessel_j0 bessel_j1 bessel_y0 bessel_y1 bge bgt bit_size ble blt btest cabs cachesize ccos ceiling cexp char "
    "clog command_argument_count compl conjg coshape cotan cotand cshift csin csqrt ctan dabs dacos dasin datan "
    "datan2 dble dcos dcosh ddim dexp dfloat digits dint dlog dlog10 dmax1 dmin1 dmod dnint dnum dprod dsign dsin "
    "dsinh dsqrt dtan dtanh eof eoshift epsilon eqv erf erfc erfc_scaled exponent extends_type_of failed_images "
    "float floor fp_class fraction gamma get_team hypot iabs iachar iaddr iarg iargc ibchng ibclr ibits ibset ichar "
    "idim idint idnint ifix ilen image_status index int int_ptr_kind int8 inum is_contiguous is_iostat_end "
    "is_iostat_eor isha ishc ishft ishftc ishl isign isnan ixor izext jint jnint jnum kind knint knum lbound "
    "lcobound leadz len len_trim lge lgt lle llt log_gamma log10 logical malloc maskl maskr max0 max1 maxexponent "
    "mclock merge min0 min1 minexponent nargs nearest neqv new_line nint not null numarg pack popcnt poppar "
    "precision present qcmplx qext qfloat qnum qreal radix ran ranf range rank real reduce repeat reshape rnum "
    "rrspacing same_type_as scale scan secnds selected_char_kind selected_int_kind selected_real_kind set_exponent "
    "shape shift shifta shiftl shiftr size sizeof sngl spacing spread stopped_images storage_size team_number "
    "this_image tiny trailz transpose trim ubound ucobound unpack verify",

    // Extended and user defined functions
    "",

    NULL,
};


EDITLEXER lexFortran =
{
    SCLEX_FORTRAN, "fortran", IDS_LEX_FORTRAN, L"Fortran Source Code", L"f; for; ftn; fpp; f90; f95; f03; f08; f15; f2k; hf", L"",
    &KeyWords_Fortran, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_F_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {SCE_F_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {SCE_F_PREPROCESSOR}, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#FF8000", L"" },
        { {SCE_F_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"italic; fore:#808080", L"" },
        { {SCE_F_WORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#1242BE", L"" },
        { {MULTI_STYLE(SCE_F_WORD2,SCE_F_WORD3,0,0)}, IDS_LEX_STR_63273, L"Function", L"bold; italic; fore:#8000FF", L"" },
        { {SCE_F_STRING1}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#008800", L"" },
        { {SCE_F_STRING2}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008800", L"" },
        { {SCE_F_STRINGEOL}, IDS_LEX_STR_63131, L"String EOL", L"fore:#008000", L"" },
        { {SCE_F_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#BD3000", L"" },
        { {MULTI_STYLE(SCE_F_OPERATOR,SCE_F_OPERATOR2,0,0)}, IDS_LEX_STR_63132, L"Operator", L"bold; fore:#A60053", L"" },
        { {SCE_F_LABEL}, IDS_LEX_STR_63235, L"Label", L"fore:#C10061", L"" },
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
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_F_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {SCE_F_PREPROCESSOR}, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#FF8000", L"" },
        { {SCE_F_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#880000", L"" },
        { {SCE_F_WORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0A246A", L"" },
        { {MULTI_STYLE(SCE_F_WORD2,SCE_F_WORD3,0,0)}, IDS_LEX_STR_63260, L"Keyword 2nd", L"bold; italic; fore:#3C6CDD", L"" },
        { {SCE_F_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {SCE_F_STRINGEOL}, IDS_LEX_STR_63131, L"String EOL", L"fore:#008000", L"" },
        { {SCE_F_STRING1}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#008800", L"" },
        { {SCE_F_STRING2}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008800", L"" },
        { {SCE_F_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF4000", L"" },
        { {MULTI_STYLE(SCE_F_OPERATOR,SCE_F_OPERATOR2,0,0)}, IDS_LEX_STR_63132, L"Operator", L"bold; fore:#666600", L"" },
        { {SCE_F_LABEL}, IDS_LEX_STR_63235, L"Label", L"fore:#0000DD", L"" },
        { {SCE_F_CONTINUATION}, IDS_LEX_STR_63376, L"Continuation", L"fore:#00DDDD", L"" },
        EDITLEXER_SENTINEL
    }
};

#endif

