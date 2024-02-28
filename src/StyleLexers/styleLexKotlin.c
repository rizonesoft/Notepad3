#include "StyleLexers.h"

// ----------------------------------------------------------------------------

//KEYWORDLIST KeyWords_Kotlin = EMPTY_KEYWORDLIST;
KEYWORDLIST KeyWords_Kotlin =
{
    // 0 Keywords
    "abstract actual annotation as break by catch class companion const constructor continue crossinline "
    "data delegate do dynamic else enum expect external false field file final finally for fun get "
    "if import in infix init inline inner interface internal is it lateinit noinline null object open operator out override "
    "package param private property protected public receiver reified return sealed set setparam super suspend "
    "tailrec this throw true try typealias typeof val value var vararg when where while "

    , // 1 Java class
    "AbstractCollection AbstractList AbstractMap AbstractQueue AbstractSequentialList AbstractSet ActionBar Activity "
    "AdapterView AlertDialog Application Array ArrayAdapter ArrayBlockingQueue ArrayDeque ArrayList Arrays "
    "AtomicBoolean AtomicInteger AtomicLong AtomicReference AudioFormat AudioManager AudioRecord AudioTrack "
    "Base64 BaseAdapter BigDecimal BigInteger Binder BitSet Bitmap Boolean "
    "Buffer BufferedInputStream BufferedOutputStream BufferedReader BufferedWriter Build Bundle Button "
    "Byte ByteArrayInputStream ByteArrayOutputStream ByteBuffer ByteOrder "
    "Calendar Canvas CharArrayReader CharArrayWriter CharBuffer Character Charset CheckBox ChoiceFormat Class ClassLoader "
    "Collections Collectors Color "
    "ConcurrentHashMap ConcurrentLinkedDeque ConcurrentLinkedQueue Console Constructor Context ContextWrapper Copy "
    "CountDownLatch Currency "
    "DataInputStream DataOutputStream DatagramPacket DatagramSocket Date DateFormat DecimalFormat DeflaterOutputStream "
    "Dialog Dictionary Display Double DoubleBuffer Drawable "
    "EOFException EditText Enum EnumMap EnumSet Environment Error EventObject Exception "
    "Field File FileDescriptor FileInputStream FileOutputStream FilePermission FileReader FileSystem FileWriter "
    "FilterInputStream FilterOutputStream FilterReader FilterWriter Float FloatBuffer Format Formatter "
    "GZIPInputStream GZIPOutputStream Gradle GregorianCalendar GridView "
    "Handler HashMap HashSet Hashtable HttpClient HttpCookie HttpRequest HttpURLConnection "
    "IOError IOException Image ImageButton ImageView Inet4Address Inet6Address InetAddress InetSocketAddress "
    "InflaterInputStream InputStream InputStreamReader IntBuffer Integer Intent IntentFilter "
    "JarEntry JarException JarFile JarInputStream JarOutputStream JavaCompile KeyEvent "
    "LayoutInflater LinearLayout LinkedHashMap LinkedHashSet LinkedList ListView Locale Long LongBuffer Looper "
    "MappedByteBuffer Matcher Math Matrix Message MessageFormat Method Modifier Module MotionEvent MulticastSocket "
    "Notification Number NumberFormat Object ObjectInputStream ObjectOutputStream Optional OutputStream OutputStreamWriter "
    "Package Paint Parcel Pattern PendingIntent PhantomReference PipedInputStream PipedOutputStream PipedReader PipedWriter "
    "Point PointF PrintStream PrintWriter PriorityQueue Process ProcessBuilder ProgressBar Project Properties "
    "RadioButton RadioGroup Random Reader Record Rect RectF Reference ReferenceQueue Region RelativeLayout RemoteException "
    "Runtime RuntimeException "
    "Scanner Script ScrollView SearchView SecurityManager SeekBar Semaphore ServerSocket Service ServiceLoader Settings "
    "Short ShortBuffer SimpleDateFormat Socket SocketAddress SoftReference SourceSet Spinner "
    "Stack StackView String StringBuffer StringBuilder StringJoiner StringReader StringTokenizer StringWriter System "
    "TableLayout Task TextView Thread ThreadGroup ThreadLocal ThreadPoolExecutor Throwable TimeZone Timer TimerTask "
    "Toast ToggleButton TreeMap TreeSet "
    "URI URL URLConnection URLDecoder URLEncoder UUID Vector View ViewGroup Void WeakHashMap WeakReference Window Writer "
    "ZipEntry ZipException ZipFile ZipInputStream ZipOutputStream "

    , // 2 class
    "AbstractIterator AbstractMutableCollection AbstractMutableList AbstractMutableMap AbstractMutableSet Any "
    "BooleanArray ByteArray Char CharArray DoubleArray FloatArray IndexedValue Int IntArray LongArray MatchGroup Nothing "
    "Pair Regex Result ShortArray Triple UByte UByteArray UInt UIntArray ULong ULongArray UShort UShortArray Unit "

    , // 3 Java interface
    "Adapter Annotation Appendable AutoCloseable BaseStream BlockingDeque BlockingQueue ByteChannel "
    "Callable Channel CharSequence Cloneable Closeable Collection Collector Comparable Comparator ConcurrentMap Condition "
    "DataInput DataOutput Deque DoubleStream Enumeration EventListener Executor Flushable Formattable Function Future "
    "HttpResponse IBinder IInterface IntStream Iterable Iterator List ListAdapter ListIterator Lock LongStream "
    "Map MatchResult Menu MenuItem NavigableMap NavigableSet ObjectInput ObjectOutput OnClickListener "
    "Parcelable Path Predicate Queue RandomAccess ReadWriteLock Readable Runnable "
    "Serializable Set SortedMap SortedSet Spliterator Stream WebSocket "

    , // 4 interface
    "Grouping Lazy "
    "MatchGroupCollection "
    "MutableCollection MutableIterable MutableIterator MutableList MutableListIterator MutableMap MutableSet "

    , // 5 enumeration
    "AnnotationRetention AnnotationTarget DeprecationLevel ElementType LazyThreadSafetyMode RegexOption RetentionPolicy "
    "TimeUnit "

    , // 6 annotation
    "Basic Column Delegate DelegatesTo Deprecated Documented Entity FunctionalInterface Generated Id Inherited "
    "ManagedBean Metadata MustBeDocumented Native NonEmpty NonNull OrderBy OrderColumn Override "
    "PostConstruct PreDestroy Priority Readonly Repeatable ReplaceWith Resource Resources Retention "
    "SafeVarargs Serial Suppress SuppressWarnings Table Target Transient Version "

    , // 7 function
    "assert check error print println readLine require "

    , // 8 KDoc
    "author constructor exception param property receiver return sample see since suppress throws "

    , NULL
};


EDITLEXER lexKotlin =
{
    SCLEX_KOTLIN, "Kotlin", IDS_LEX_KOTLIN_SRC, L"Kotlin Source Code", L"kt; kts; ktm", L"",
    &KeyWords_Kotlin, {
        { { STYLE_DEFAULT }, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_KOTLIN_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { { SCE_KOTLIN_WORD }, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#0000FF", L"" },
	    { { SCE_KOTLIN_ANNOTATION }, IDS_LEX_STR_Annot, L"Annotation", L"fore:#FF8000", L"" },
	    { { SCE_KOTLIN_CLASS }, IDS_LEX_STR_63258, L"Class", L"fore:#0080FF", L"" },
	    { { SCE_KOTLIN_INTERFACE }, IDS_LEX_STR_Interface, L"Interface", L"bold; fore:#1E90FF", L"" },
	    { { SCE_KOTLIN_ENUM }, IDS_LEX_STR_Enum, L"Enumeration", L"fore:#FF8000", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_FUNCTION, SCE_KOTLIN_FUNCTION_DEFINITION, 0, 0) }, IDS_LEX_STR_63277, L"Function", L"fore:#A46000", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_COMMENTBLOCK, SCE_KOTLIN_COMMENTLINE, 0, 0) }, IDS_LEX_STR_Comment, L"Comment", L"fore:#608060", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_COMMENTBLOCKDOC, SCE_KOTLIN_COMMENTLINEDOC, 0, 0) }, IDS_LEX_STR_63259, L"Comment Doc", L"fore:#408080", L"" },
	    { { SCE_KOTLIN_COMMENTDOCWORD }, IDS_LEX_STR_63371, L"Comment Doc Word", L"fore:#408080", L"" },
      	{ { SCE_KOTLIN_TASKMARKER }, IDS_LEX_STR_63373, L"Task Marker", L"bold; fore:#208080" },
	    { { MULTI_STYLE(SCE_KOTLIN_CHARACTER, SCE_KOTLIN_STRING, 0, 0) }, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
	    { { SCE_KOTLIN_RAWSTRING }, IDS_LEX_STR_VerbStrg, L"Verbatim String", L"fore:#F08000", L"" },
	    { { SCE_KOTLIN_ESCAPECHAR }, IDS_LEX_STR_63366, L"ESC Sequence", L"fore:#0080C0", L"" },
	    { { SCE_KOTLIN_BACKTICKS }, IDS_LEX_STR_63221, L"Back Ticks", L"fore:#9E4D2A", L"" },
	    { { SCE_KOTLIN_LABEL }, IDS_LEX_STR_Label, L"Label", L"fore:#7C5AF3", L"" },
	    { { SCE_KOTLIN_NUMBER }, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
	    { { SCE_KOTLIN_VARIABLE }, IDS_LEX_STR_Var, L"Variable", L"fore:#9E4D2A", L"" },
	    { { MULTI_STYLE(SCE_KOTLIN_OPERATOR, SCE_KOTLIN_OPERATOR2, 0, 0) }, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        EDITLEXER_SENTINEL
    }
};
