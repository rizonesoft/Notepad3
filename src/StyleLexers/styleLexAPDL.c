#include "StyleLexers.h"

// ANSYS Mechanical APDL. Keyword lists are lowercase: APDL is case-insensitive.
KEYWORDLIST KeyWords_APDL = {
    "/aux2 /aux3 /aux12 /aux15 /begin /exit /post1 /post26 /prep7 /solu finish",
    "a adrag agen aglue allsel alist amesh antype asel aslv asubst bflist bf bfe "
    "bfblock bfeblock blc4 block /block cdread cdwrite ce cm cmblock cmsel cp csys "
    "d da dblock ddele dk dl dlist "
    "e eblock egen elist emodif esel esize et etblock etlist f fblock fill finish flist k kgen "
    "klist ksel l lesize lgen llist lmesh lsel mat meshing mshape msize mp mpdata "
    "mptemp n nblock ngen nlist nplot nsel nsle nsmblock numcmp nummerge outres plnsol "
    "pldisp plnsum post1 prnsol *pread r real resume rlblock rmore save secblock sectype secdata set sf "
    "sfe sfeblock solve tb tbdata time title type v vdrag vgen vglue vlist vmesh vsel",
    "all and then eq ne lt le gt ge abgt ablt or xor node elem kp line area volu "
    "solid shell beam static modal transient full yes no on off new append",
    "abs acos asin atan atan2 cos cosh exp log log10 max min mod nint sin sinh "
    "sqrt tan tanh valchr chrval strcat strcomp strfind strleng strsub strupper "
    "strlower nx ny nz kx ky kz centrx centry centrz distnd distkp ndnext elnext",
    NULL,
};

EDITLEXER lexAPDL = {
    SCLEX_APDL, "apdl", IDS_LEX_APDL, L"ANSYS APDL", L"apdl; mac; cdb", L"",
    &KeyWords_APDL, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_APDL_COMMENT,SCE_APDL_COMMENTBLOCK,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#38702A", L"" },
        { {SCE_APDL_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#A04000", L"" },
        { {SCE_APDL_STRING}, IDS_LEX_STR_String, L"String", L"fore:#804090", L"" },
        { {SCE_APDL_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#9A3050", L"" },
        { {SCE_APDL_PROCESSOR}, IDS_LEX_STR_Processor, L"Processor", L"bold; fore:#A04000", L"" },
        { {SCE_APDL_COMMAND}, IDS_LEX_STR_Cmd, L"Command", L"fore:#18579A", L"" },
        { {SCE_APDL_SLASHCOMMAND}, IDS_LEX_STR_SlashCmd, L"Slash Command", L"fore:#006565", L"" },
        { {SCE_APDL_STARCOMMAND}, IDS_LEX_STR_StarCmd, L"Star Command", L"bold; fore:#6040A0", L"" },
        { {SCE_APDL_ARGUMENT}, IDS_LEX_STR_Argument, L"Argument", L"fore:#665700", L"" },
        { {SCE_APDL_FUNCTION}, IDS_LEX_STR_Function, L"Function", L"fore:#795300", L"" },
        { {SCE_APDL_WORD}, IDS_LEX_STR_Word, L"Word", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
