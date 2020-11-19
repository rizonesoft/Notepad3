#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_JAVA =
{
    "@Deprecated @Documented @FlaskyTest @Inherited @JavascriptInterface @LargeTest @MediumTest @Override "
    "@Retention @SmallTest @Smoke @Supress @SupressLint @SupressWarnings @Target @TargetApi @TestTarget "
    "@TestTargetClass @UiThreadTest @interface abstract assert boolean break byte case catch char class const "
    "continue default do double else enum extends final finally float for future generic goto if implements "
    "import inner instanceof int interface long native new null outer package private protected public rest "
    "return short static super switch synchronized this throw throws transient try var void volatile while",
    NULL,
};


EDITLEXER lexJAVA =
{
    SCLEX_CPP, "cpp", IDS_LEX_JAVA_SRC, L"Java Source Code", L"java; jad; aidl; bsh", L"",
    &KeyWords_JAVA, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_C_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {SCE_C_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#646464", L"" },
        { {SCE_C_WORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#A46000", L"" },
        { {SCE_C_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_C_REGEX}, IDS_LEX_STR_63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_C_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {SCE_C_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
        //{ {SCE_C_UUID}, L"UUID", L"", L"" },
        //{ {SCE_C_PREPROCESSOR}, L"Preprocessor", L"fore:#FF8000", L"" },
        //{ {SCE_C_WORD}, L"Word 2", L"", L"" },
        //{ {SCE_C_GLOBALCLASS}, L"Global Class", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
