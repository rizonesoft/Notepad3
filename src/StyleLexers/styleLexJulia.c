#include "StyleLexers.h"

// ----------------------------------------------------------------------------

//KEYWORDLIST KeyWords_Julia = EMPTY_KEYWORDLIST;
KEYWORDLIST KeyWords_Julia =
{
     // 0 Primary keywords and identifiers
    "abstract baremodule begin break catch const continue do else elseif end export finally for function global "
    "if import in isa let local macro module mutable primitive quote return struct try type using var where while "

    , // 1 Built in types
    "AbstractArray AbstractArrayStyle AbstractChannel AbstractChar AbstractDict AbstractDisplay AbstractFloat "
    "AbstractIrrational AbstractLock AbstractLogger AbstractMatrix AbstractRNG AbstractRange "
    "AbstractSet AbstractSparseArray AbstractSparseMatrix AbstractSparseVector AbstractString AbstractUnitRange "
    "AbstractVecOrMat AbstractVector AbstractWorkerPool Adjoint Anonymous Any ArgumentError Array ArrayStyle "
    "AssertionError AsyncCondition Atomic "
    "Base64DecodePipe Base64EncodePipe Bidiagonal BigFloat BigInt BitArray BitMatrix BitSet BitVector Bool BoundsError "
    "BroadcastStyle "
    "CFunction CapturedException CartesianIndex CartesianIndices Cchar Cdouble Cfloat Channel Char Cint Cintmax_t "
    "Clong Clonglong ClusterManager Cmd "
    "Colon Complex ComplexF16 ComplexF32 ComplexF64 CompositeException CompoundPeriod Condition ConsoleLogger Cptrdiff_t "
    "Cshort Csize_t Cssize_t Cstring Cuchar Cuint Cuintmax_t Culong Culonglong Cushort Cwchar_t Cwstring "
    "DataType Date DateFormat DateTime Day DefaultArrayStyle DenseArray DenseMatrix DenseVecOrMat DenseVector "
    "Diagonal Dict DimensionMismatch Dims DivideError DomainError "
    "EOFError Enum EnvDict ErrorException Event Exception ExponentialBackOff Expr Float16 Float32 Float64 Function Future "
    "Givens GlobalRef Hermitian Hour "
    "IO IOBuffer IOContext IOStream IPAddr IPv4 IPv6 IdDict ImmutableDict IndexCartesian IndexLinear IndexStyle InexactError "
    "InitError Instant Int Int128 Int16 Int32 Int64 Int8 Integer InterruptException InvalidStateException Irrational "
    "IteratorEltype IteratorSize "
    "KeyError LinRange LineNumberNode LinearIndices LoadError LogLevel LowerTriangular "
    "MIME Matrix MersenneTwister Method MethodError Microsecond Millisecond Minute Missing MissingException Module Month "
    "Mutex "
    "NTuple NamedTuple Nanosecond Nothing NullLogger Number OneTo OrdinalRange OutOfMemoryError OverflowError "
    "Pair Pairs PartialQuickSort Period PermutedDimsArray Pipe PipeBuffer PosDefException ProcessFailedException Ptr "
    "QR QRCompactWY QRPivoted QuoteNode "
    "Random RandomDevice Rational RawFD "
    "ReadOnlyMemoryError Real ReentrantLock Ref Regex RegexMatch RemoteChannel RemoteException RoundingMode "
    "Second SegmentationFault Semaphore Serialization Set SharedArray SharedMatrix SharedVector "
    "Signed SimpleLogger SingularException Some SparseMatrixCSC SparseVector SpinLock "
    "StackFrame StackOverflowError StackTrace Stateful StepRange StepRangeLen "
    "StridedArray StridedMatrix StridedVecOrMat StridedVector String StringIndexError SubArray SubString SubstitutionString "
    "SymTridiagonal Symbol Symmetric SystemError "
    "TCPSocket Task TextDisplay Time TimeType Timer TmStruct Transpose Tridiagonal Tuple Type TypeError TypeVar "
    "UDPSocket UInt UInt128 UInt16 UInt32 UInt64 UInt8 UTInstant "
    "UndefInitializer UndefKeywordError UndefRefError UndefVarError "
    "UniformScaling Union UnionAll UnitLowerTriangular UnitRange UnitUpperTriangular Unsigned UpperTriangular "
    "Val Vararg VecElement VecOrMat Vector VersionNumber WeakKeyDict WeakRef Week WorkerConfig WorkerPool Year "

    , // 2 Other keywords
    "allocated assert async boundscheck cfunction debug deprecate distributed dump "
    "elapsed enum error eval evalpoly everywhere fastmath fetch generated gensym goto "
    "inbounds inferred info inline isdefined label logmsg lower macroexpand macroexpand1 noinline nospecialize "
    "polly preserve printf profile propagate_inbounds pure show spawn spawnat specialize sprintf static sync "
    "task test test_broken test_deprecated test_logs test_nowarn test_skip test_throws test_warn testset threads "
    "time timed timev "
    "view views warn "

    , // 3 Raw string literals
    "raw "

    , NULL
};


EDITLEXER lexJulia =
{
    SCLEX_JULIA, "julia", IDS_LEX_JULIA_SCR, L"Julia Script", L"jl", L"",
    &KeyWords_Julia, {
        { { STYLE_DEFAULT }, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_JULIA_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { { SCE_JULIA_COMMENT }, IDS_LEX_STR_Comment, L"Comment", L"fore:#608060", L"" },
        { { SCE_JULIA_NUMBER }, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { { SCE_JULIA_KEYWORD1 }, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#0000FF", L"" },
        { { MULTI_STYLE(SCE_JULIA_KEYWORD2, SCE_JULIA_KEYWORD3, SCE_JULIA_KEYWORD4, 0) }, IDS_LEX_STR_Keyword2nd, L"Keyword 2nd", L"bold; fore:#8A008A", L"" },
        { { MULTI_STYLE(SCE_JULIA_CHAR, SCE_JULIA_STRING, 0, 0) }, IDS_LEX_STR_String, L"String", L"fore:#BB2F00", L"" },
        { { MULTI_STYLE(SCE_JULIA_OPERATOR, SCE_JULIA_BRACKET, 0, 0) }, IDS_LEX_STR_Operator, L"Operator", L"fore:#B53A00", L"" },
        { { SCE_JULIA_TYPEOPERATOR }, IDS_LEX_STR_TypeOp, L"Type Operator", L"fore:#950095", L"" },
        { { SCE_JULIA_IDENTIFIER }, IDS_LEX_STR_Identifier, L"Identifier", L"fore:#00007B", L"" },
        { { SCE_JULIA_SYMBOL }, IDS_LEX_STR_Symbol, L"Symbol", L"fore:#C0A030", L"" },
        { { SCE_JULIA_MACRO }, IDS_LEX_STR_63280, L"Macro Def", L"fore:#0080FF", L"" },
        { { SCE_JULIA_DOCSTRING }, IDS_LEX_STR_63259, L"Comment Doc", L"fore:#808080", L"" },
        { { MULTI_STYLE(SCE_JULIA_STRINGINTERP, SCE_JULIA_STRINGLITERAL, 0, 0) }, IDS_LEX_STR_LitStrg, L"Literal String", L"fore:#B000B0", L"" },
        { { MULTI_STYLE(SCE_JULIA_COMMAND, SCE_JULIA_COMMANDLITERAL, 0, 0) }, IDS_LEX_STR_Cmd, L"Command", L"bold; fore:#0000DD", L"" },
        { { SCE_JULIA_TYPEANNOT }, IDS_LEX_STR_Annot, L"Annotation", L"fore:#FF8000", L"" },
        { { SCE_JULIA_LEXERROR }, IDS_LEX_STR_63252, L"Parsing Error", L"fore:#FFFF00; back:#A00000; eolfilled", L"" },
        EDITLEXER_SENTINEL
    }
};
