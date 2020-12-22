#include "StyleLexers.h"

// ----------------------------------------------------------------------------

//KEYWORDLIST KeyWords_Kotlin = EMPTY_KEYWORDLIST;
KEYWORDLIST KeyWords_Kotlin =
{
      // 0 keywords
    "abstract actual annotation as break by catch class companion const constructor continue crossinline "
    "data delegate do dynamic else enum expect external false field file final finally for fun get "
    "if import in infix init inline inner interface internal is it lateinit noinline null object open operator out override "
    "package param private property protected public receiver reified return sealed set setparam super suspend "
    "tailrec this throw true try typealias typeof val var vararg when where while "

    , // 1 class
    "AbstractCollection AbstractIterator AbstractList "
    "AbstractMap AbstractMutableCollection AbstractMutableList AbstractMutableMap AbstractMutableSet AbstractSet Any "
    "Array ArrayList "
    "Boolean BooleanArray Byte ByteArray Char CharArray Double DoubleArray Enum Error Exception Float FloatArray "
    "HashMap HashSet IndexedValue Int IntArray LinkedHashMap LinkedHashSet Long LongArray MatchGroup Nothing Number Pair "
    "Random Regex Result RuntimeException Short ShortArray String StringBuilder Throwable Triple "
    "UByte UByteArray UInt UIntArray ULong ULongArray UShort UShortArray Unit "

    , // 2 interface
    "Annotation Appendable CharSequence Collection Comparable Comparator Function Grouping Iterable Iterator "
    "Lazy List ListIterator "
    "Map MatchGroupCollection MatchResult "
    "MutableCollection MutableIterable MutableIterator MutableList MutableListIterator MutableMap MutableSet "
    "RandomAccess Set "

    , // 3 enum
    "AnnotationRetention AnnotationTarget DeprecationLevel LazyThreadSafetyMode RegexOption "

    , // 4 annotation
    "Deprecated Metadata MustBeDocumented Repeatable ReplaceWith Retention Suppress Target "

    , // 5 function
    "assert check error print println readLine require "

    , // 6 KDoc
    "author constructor exception param property receiver return sample see since suppress throws "

    , NULL
};


EDITLEXER lexKotlin =
{
    SCLEX_KOTLIN, "Kotlin", IDS_LEX_KOTLIN_SRC, L"Kotlin Source Code", L"kt; kts; ktm", L"",
    &KeyWords_Kotlin, {
        { { STYLE_DEFAULT }, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_KOTLIN_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { { SCE_KOTLIN_WORD }, IDS_LEX_STR_63128, L"Keyword", L"fore:#0000FF", L"" },
	    { { SCE_KOTLIN_ANNOTATION }, IDS_LEX_STR_63370, L"Annotation", L"fore:#FF8000", L"" },
	    { { SCE_KOTLIN_CLASS }, IDS_LEX_STR_63258, L"Class", L"fore:#0080FF", L"" },
	    { { SCE_KOTLIN_INTERFACE }, IDS_LEX_STR_63369, L"Interface", L"bold; fore:#1E90FF", L"" },
	    { { SCE_KOTLIN_ENUM }, IDS_LEX_STR_63203, L"Enumeration", L"fore:#FF8000", L"" },
	    { { SCE_KOTLIN_FUNCTION }, IDS_LEX_STR_63277, L"Function", L"fore:#A46000", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_COMMENTBLOCK, SCE_KOTLIN_COMMENTLINE, 0, 0) }, IDS_LEX_STR_63127, L"Comment", L"fore:#608060", L"" },
	    { { SCE_KOTLIN_COMMENTDOCWORD }, IDS_LEX_STR_63371, L"Comment Doc Word", L"fore:#408080", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_COMMENTBLOCKDOC, SCE_KOTLIN_COMMENTLINEDOC, 0, 0) }, IDS_LEX_STR_63259, L"Comment Doc", L"fore:#408080", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_CHARACTER, SCE_KOTLIN_STRING, 0, 0) }, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_RAWSTRING, SCE_KOTLIN_RAWSTRINGSTART, SCE_KOTLIN_RAWSTRINGEND, 0) }, IDS_LEX_STR_63134, L"Verbatim String", L"fore:#F08000", L"" },
	    { { SCE_KOTLIN_ESCAPECHAR }, IDS_LEX_STR_63366, L"ESC Sequence", L"fore:#0080C0", L"" },
	    { { SCE_KOTLIN_BACKTICKS }, IDS_LEX_STR_63221, L"Back Ticks", L"fore:#9E4D2A", L"" },
	    { { SCE_KOTLIN_LABEL }, IDS_LEX_STR_63235, L"Label", L"fore:#7C5AF3", L"" },
	    { { SCE_KOTLIN_NUMBER }, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
	    { { SCE_KOTLIN_VARIABLE }, IDS_LEX_STR_63249, L"Variable", L"fore:#9E4D2A", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_OPERATOR, SCE_KOTLIN_OPERATOR2, 0, 0) }, IDS_LEX_STR_63132, L"Operator", L"fore:#B000B0", L"" },
        EDITLEXER_SENTINEL
    }
};
