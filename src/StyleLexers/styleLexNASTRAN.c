#include "StyleLexers.h"

// The dictionary aids completion; unknown/custom cards still receive card styling.
KEYWORDLIST KeyWords_NASTRAN = {
    "accel accel1 aecomp aefact aelist aelink aeparm aesurf aero aeros aset aset1 "
    "bconp bctset bctpara bsurf caero1 caero2 cbar cbeam cbend cbush cbush1d "
    "cdamp1 cdamp2 cdamp3 cdamp4 celas1 celas2 celas3 celas4 chexa cihex1 conm1 conm2 "
    "conrod cord1c cord1r cord1s cord2c cord2r cord2s cpenta cpyram cquad4 cquad8 "
    "cquadr crod cshear ctetra ctria3 ctria6 ctriar ctube darea dconstr ddval delay "
    "desvar dlink dload dresp1 dresp2 dvprel1 dvprel2 eigb eigc eigp eigrl eigr "
    "force force1 force2 freq freq1 freq2 freq3 freq4 freq5 grav grid gridb mpc "
    "include load mat1 mat2 mat3 mat4 mat5 mat8 mat9 mat10 mathp mats1 matt1 matt8 "
    "moment moment1 moment2 mpcadd nlpci nlparm omit omit1 param pbar pbarl pbeam "
    "pbeaml pbend pbush pcomp pcompg pconv pdamp pelas pelast phbdy pload pload1 "
    "pload2 pload4 ploadx1 plotel pmass prod pshear pshell psolid ptube qbdy1 "
    "qbdy2 qbdy3 qvol randps rbar rbar1 rbe1 rbe2 rbe3 rforce rload1 rload2 "
    "set1 set3 spc spc1 spcadd spcd sload spoint suport suport1 tabdmp1 tabled1 "
    "tabled2 tabled3 tabled4 tablem1 tables1 temp tempd tic tload1 tload2 trim "
    "tstep tstepnl uset uset1",
    "all alter analysis assign autospc begin both bulk by cend center complex "
    "cubic damping decomp diag displacement echo enddata except exclude force "
    "frequency global id include label linear load max min method mpc nastran "
    "nlparm none output param plot post print punch real resvec seall set sol "
    "sort1 sort2 spc strain stress subcase subtitle super temperature thru time title vector",
    NULL,
};

EDITLEXER lexNASTRAN = {
    SCLEX_NASTRAN, "nastran", IDS_LEX_NASTRAN, L"NASTRAN", L"bdf; nas", L"",
    &KeyWords_NASTRAN, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_NASTRAN_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#38702A", L"" },
        { {SCE_NASTRAN_CARD}, IDS_LEX_STR_Cmd, L"Card", L"bold; fore:#18579A", L"" },
        { {SCE_NASTRAN_SECTION}, IDS_LEX_STR_Section, L"Section", L"bold; fore:#A04000", L"" },
        { {SCE_NASTRAN_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#6040A0", L"" },
        { {SCE_NASTRAN_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#A04000", L"" },
        { {SCE_NASTRAN_STRING}, IDS_LEX_STR_String, L"String", L"fore:#804090", L"" },
        { {SCE_NASTRAN_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#9A3050", L"" },
        { {SCE_NASTRAN_CONTINUATION}, IDS_LEX_STR_63376, L"Continuation", L"fore:#006565", L"" },
        { {SCE_NASTRAN_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
