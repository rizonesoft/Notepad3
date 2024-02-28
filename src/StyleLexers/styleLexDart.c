#include "StyleLexers.h"

// ----------------------------------------------------------------------------

//KEYWORDLIST KeyWords_Dart = EMPTY_KEYWORDLIST;
KEYWORDLIST KeyWords_Dart =
{
    // 0 keywords
    "abstract as assert async await base break case catch class const continue covariant default deferred do "
    "else enum export extends extension external factory false final finally for get hide "
    "if implements import in interface is late library mixin native new null of on operator part required rethrow return "
    "sealed set show static super switch sync this throw true try typedef var when while with yield "

    , // 1 types
    "Function Never bool double dynamic int num void "

    , // 2 class
    "BigInt Comparable Comparator Completer DateTime Deprecated Directory DoubleLinkedQueue Duration "
    "Enum Error Exception Expando File FileLock FileMode FileStat FileSystemEntity FileSystemEvent Future FutureOr "
    "HashMap HashSet IOException Invocation Iterable IterableBase IterableMixin Iterator "
    "LinkedHashMap LinkedHashSet LinkedList LinkedListEntry List ListBase ListMixin ListQueue "
    "Map MapBase MapEntry MapMixin MapView Match Null OSError Object Pattern Platform Point Process Queue "
    "Random RawSocket RawSocketEvent Record Rectangle RegExp RegExpMatch RuneIterator Runes "
    "ServerSocket Set SetBase SetMixin Sink Socket SocketException SplayTreeMap SplayTreeSet "
    "StackTrace Stopwatch Stream String StringBuffer StringSink Symbol SystemHash "
    "Timer Type Uri UriData WeakReference "

    , // 3 enumeration
    ""

    , // 4 metadata
    "Deprecated Since deprecated override patch pragma "

    , // 5 function
    "identical identityHashCode main print "

    , NULL
};


EDITLEXER lexDart =
{
    SCLEX_DART, "Dart", IDS_LEX_DART_SRC, L"Dart Source Code", L"dart", L"",
    &KeyWords_Dart,{
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_DART_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_DART_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#0000FF", L"" },
        { {SCE_DART_WORD2}, IDS_LEX_STR_Keyword2nd, L"Keyword 2nd", L"fore:#0000FF", L"" },
        { {SCE_DART_METADATA}, IDS_LEX_STR_63383, L"Meta-Data", L"fore:#FF8000", L"" },
        { {SCE_DART_CLASS}, IDS_LEX_STR_63258, L"Class", L"fore:#0080FF", L"" },
        { {SCE_DART_ENUM}, IDS_LEX_STR_Enum, L"Enumeration", L"fore:#FF8000", L"" },
        { {SCE_DART_FUNCTION}, IDS_LEX_STR_63277, L"Function", L"fore:#A46000", L"" },
        { {MULTI_STYLE(SCE_DART_COMMENTBLOCK, SCE_DART_COMMENTLINE, 0, 0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#608060", L"" },
        { {MULTI_STYLE(SCE_DART_COMMENTBLOCKDOC, SCE_DART_COMMENTLINEDOC, 0, 0)}, IDS_LEX_STR_63259, L"Comment Doc", L"fore:#408040", L"" },
        { {SCE_DART_TASKMARKER}, IDS_LEX_STR_63373, L"Task Marker", L"bold; fore:#408080" },
        { {MULTI_STYLE(SCE_DART_STRING_SQ, SCE_DART_STRING_DQ, 0, 0)}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_DART_TRIPLE_STRING_SQ, SCE_DART_TRIPLE_STRING_DQ, 0, 0)}, IDS_LEX_STR_63385, L"TriQ-String", L"fore:#F08000", L"" },
        { {MULTI_STYLE(SCE_DART_RAWSTRING_SQ, SCE_DART_RAWSTRING_DQ, SCE_DART_TRIPLE_RAWSTRING_SQ, SCE_DART_TRIPLE_RAWSTRING_DQ)}, IDS_LEX_STR_VerbStrg, L"Verbatim String", L"fore:#F08000", L"" },
        { {SCE_DART_ESCAPECHAR}, IDS_LEX_STR_63366, L"ESC Sequence", L"fore:#0080C0", L"" },
        { {SCE_DART_LABEL}, IDS_LEX_STR_Label, L"Label", L"fore:#7C5AF3", L"" },
        { {SCE_DART_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_DART_VARIABLE}, IDS_LEX_STR_Var, L"Variable", L"fore:#9E4D2A", L"" },
        { {MULTI_STYLE(SCE_DART_OPERATOR, SCE_DART_OPERATOR2, 0, 0)}, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_DART_SYMBOL_IDENTIFIER}, IDS_LEX_STR_63386, L"Symbol Identifier", L"fore:#7C5AF3", L"" },
        { {SCE_DART_SYMBOL_OPERATOR}, IDS_LEX_STR_63387, L"Symbol Operator", L"fore:#7C5AF3", L"" },
        EDITLEXER_SENTINEL
    }
};
