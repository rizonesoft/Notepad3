#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_CPP =
{
// Primary keywords
    "alignas alignof asm audit auto axiom bitand bitor bool break case catch char class compl concept "
    "const const_cast consteval constexpr continue co_await co_return co_yield "
    "decltype default defined delete do double dynamic_cast else enum explicit export extern false final float for "
    "friend goto if import inline int long module mutable naked namespace new noexcept not not_eq noreturn nullptr "
    "operator or or_eq override private protected public "
    "register reinterpret_cast requires restrict return "
    "short signed sizeof static static_assert static_cast struct switch "
    "template this thread_local throw true try typedef typeid typename "
    "union unsigned using virtual void volatile while xor xor_eq",
// Secondary keywords
    "_Alignas _Alignof _Atomic _Bool _Complex _Generic _Imaginary _Noreturn _Static_assert _Thread_local "
    "_Pragma __DATE__ __FILE__ __LINE__ __STDCPP_DEFAULT_NEW_ALIGNMENT__ __STDCPP_STRICT_POINTER_SAFETY__ "
    "__STDCPP_THREADS__ __STDC_HOSTED__ __STDC_ISO_10646__ __STDC_MB_MIGHT_NEQ_WC__ __STDC_UTF_16__ "
    "__STDC_UTF_32__ __STDC_VERSION__ __STDC__ __TIME__ __VA_ARGS__ __VA_OPT__ __abstract __alignof __asm "
    "__assume __based __box __cdecl __cplusplus __declspec __delegate __event __except __except__try "
    "__fastcall __finally __gc __has_include __hook __identifier __if_exists __if_not_exists __inline "
    "__interface __leave __multiple_inheritance __nogc __noop __pin __property __raise __sealed "
    "__single_inheritance __stdcall __super __try __try_cast __unhook __uuidof __value __virtual_inheritance",
// Documentation comment keywords
    "addindex addtogroup anchor arg attention author b brief bug c class code copyright date def defgroup deprecated dontinclude "
    "e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception f$ f[f] file"
    "hideinitializer htmlinclude htmlonly if image include ingroup internal invariant interface latexonly li line link "
    "mainpage name namespace nosubgrouping note overload p page par param param[in] param[out] post pre "
    "ref relates remarks return retval sa section see showinitializer since skip skipline struct subsection "
    "test throw throws todo typedef union until var verbatim verbinclude version warning weakgroup",
// Global classes and typedefs
    "__int16 __int32 __int64 __int8 __m128 __m128d __m128i __m64 __wchar_t char8_t char16_t char32_t complex imaginary int16_t int32_t "
    "int64_t int8_t intmax_t intptr_t ptrdiff_t size_t uint16_t uint32_t uint64_t uint8_t uintmax_t uintptr_t wchar_t",
// Preprocessor definitions
    "DEBUG NDEBUG UNICODE _DEBUG _MSC_VER _UNICODE",
// Task marker and error marker keywords
    "BUG FIXME HACK NOTE TBD TODO UNDONE XXX @@@",
    NULL,
};

EDITLEXER lexCPP =
{
    SCLEX_CPP, "cpp", IDS_LEX_CPP_SRC, L"C/C++ Source Code", L"c; cpp; cxx; cc; h; hpp; hxx; hh; mm; idl; midl; inl; odl; xpm; pch", L"",
    &KeyWords_CPP, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_C_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_C_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_C_COMMENT,SCE_C_COMMENTLINE,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_C_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#0A246A", L"" },
        { {SCE_C_WORD2}, IDS_LEX_STR_Keyword2nd, L"Keyword 2nd", L"bold; italic; fore:#3C6CDD", L"" },
        { {SCE_C_GLOBALCLASS}, IDS_LEX_STR_63258, L"Typedefs/Classes", L"bold; italic; fore:#800000", L"" },
        { {SCE_C_STRING}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        //{ {SCE_C_REGEX}, IDS_LEX_STR_RegEx, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_C_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_C_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_C_PREPROCESSOR,SCE_C_PREPROCESSORCOMMENT,SCE_C_PREPROCESSORCOMMENTDOC,0)}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF8000", L"" },
        { {MULTI_STYLE(SCE_C_VERBATIM,SCE_C_TRIPLEVERBATIM,0,0)}, IDS_LEX_STR_VerbStrg, L"Verbatim", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_C_COMMENTDOC,SCE_C_COMMENTLINEDOC,0,0)}, IDS_LEX_STR_63259, L"Comment Doc", L"fore:#808080", L"" },
        { {SCE_C_COMMENTDOCKEYWORD}, IDS_LEX_STR_63371, L"Comment Doc Word", L"bold; fore:#808080", L"" },
        { {SCE_C_COMMENTDOCKEYWORDERROR}, IDS_LEX_STR_63374, L"Comment Doc Error", L"italic; fore:#800000", L"" },
        { {SCE_C_TASKMARKER}, IDS_LEX_STR_63373, L"Task Marker", L"bold; fore:#208080", L"" },
        //{ {SCE_C_UUID}, L"UUID", L"", L"" },
        //{ {SCE_C_USERLITERAL}, L"User Literal", L"", L"" },
        //{ {SCE_C_ESCAPESEQUENCE}, L"Esc Seq", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
