#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_CPP =
{
// Primary keywords
    "_Alignas _Alignof _Atomic _Bool _Complex _Generic _Imaginary _Noreturn _Static_assert _Thread_local alignas "
    "alignof auto bool break case catch char char16_t char32_t class const const_cast constexpr continue "
    "decltype default defined delete do double dynamic_cast else enum explicit export extern false float for "
    "friend goto if inline int long mutable naked namespace new noexcept noreturn nullptr operator private "
    "protected public register reinterpret_cast restrict return short signed sizeof static static_assert "
    "static_cast struct switch template this thread_local throw true try typedef typeid typename union "
    "unsigned using virtual void volatile wchar_t while",
// Secondary keywords
    "_Pragma __DATE__ __FILE__ __LINE__ __STDCPP_DEFAULT_NEW_ALIGNMENT__ __STDCPP_STRICT_POINTER_SAFETY__ "
    "__STDCPP_THREADS__ __STDC_HOSTED__ __STDC_ISO_10646__ __STDC_MB_MIGHT_NEQ_WC__ __STDC_UTF_16__ "
    "__STDC_UTF_32__ __STDC_VERSION__ __STDC__ __TIME__ __VA_ARGS__ __VA_OPT__ __abstract __alignof __asm "
    "__assume __based __box __cdecl __cplusplus __declspec __delegate __event __except __except__try "
    "__fastcall __finally __gc __has_include __hook __identifier __if_exists __if_not_exists __inline "
    "__interface __leave __multiple_inheritance __nogc __noop __pin __property __raise __sealed "
    "__single_inheritance __stdcall __super __try __try_cast __unhook __uuidof __value __virtual_inheritance "
    "asm final override",
// Documentation comment keywords
    "",
// Global classes and typedefs
    "__int16 __int32 __int64 __int8 __m128 __m128d __m128i __m64 __wchar_t complex imaginary int16_t int32_t "
    "int64_t int8_t intmax_t intptr_t ptrdiff_t size_t uint16_t uint32_t uint64_t uint8_t uintmax_t uintptr_t",
// Preprocessor definitions
    "DEBUG NDEBUG UNICODE _DEBUG _MSC_VER _UNICODE",
// Task marker and error marker keywords
    NULL,
};

EDITLEXER lexCPP =
{
    SCLEX_CPP, "cpp", IDS_LEX_CPP_SRC, L"C/C++ Source Code", L"c; cpp; cxx; cc; h; hpp; hxx; hh; mm; idl; midl; inl; odl; xpm; pch", L"",
    &KeyWords_CPP, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_C_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {SCE_C_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {SCE_C_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
        { {SCE_C_WORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#0A246A", L"" },
        { {SCE_C_WORD2}, IDS_LEX_STR_63260, L"Keyword 2nd", L"bold; italic; fore:#3C6CDD", L"" },
        { {SCE_C_GLOBALCLASS}, IDS_LEX_STR_63258, L"Typedefs/Classes", L"bold; italic; fore:#800000", L"" },
        { {MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_C_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {SCE_C_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_C_PREPROCESSOR,SCE_C_PREPROCESSORCOMMENT,SCE_C_PREPROCESSORCOMMENTDOC,0)}, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#FF8000", L"" },
        //{ {SCE_C_UUID}, L"UUID", L"", L"" },
        //{ {SCE_C_REGEX}, L"Regex", L"", L"" },
        EDITLEXER_SENTINEL
    }
};

