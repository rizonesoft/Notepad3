#include "StyleLexers.h"

// ----------------------------------------------------------------------------

// https://en.wikibooks.org/wiki/Fortran

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
    "achar adjustl adjustr aimag aint all alog alog10 amax0 amax1 amin0 amin1 amod anint any atan2 bessel_j0 bessel_j1 bessel_jn bessel_y0 bessel_y1 "
    "bessel_yn bge bgt ble blt btest cabs ccos ceiling cexp char clog conjg count csin csqrt ctan dabs dacos dasin datan datan2 dble dcos dcosh ddim dexp "
    "dint dlog dlog10 dmax1 dmin1 dmod dnint dprod dsign dsin dsinh dsqrt dtan dtanh erf erfc erfc_scaled exponent findloc float floor fraction gamma "
    "hypot iabs iachar iall iany ibclr ibits ibset ichar idim idint idnint ifix image_status index int iparity is_iostat_end is_iostat_eor ishft ishftc "
    "isign leadz len len_trim lge lgt lle llt log_gamma log10 logical maskl maskr max0 max1 maxloc maxval merge min0 min1 minloc minval nearest nint "
    "norm2 not parity popcnt poppar product real rrspacing scale scan set_exponent shifta shiftl shiftr sngl spacing sum trailz verify",

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

