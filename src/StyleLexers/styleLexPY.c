#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_PY =
{
    // Keywords
    "False None True and as assert break class continue def del elif else except exec finally for from global if "
    "import in is lambda nonlocal not or pass print raise return try while with yield",
    
    // Highlighted identifiers
    "__main__ _dummy_thread _thread abc aifc argparse array ast asynchat asyncio asyncore atexit audioop "
    "base64 bdb binascii binhex bisect builtins bz2 calendar cgi cgitb chunk cmath cmd code codecs "
    "codeop collections colorsys compileall concurrent configparser contextlib copy copyreg crypt csv "
    "ctypes curses datetime dbm decimal difflib dis distutils dummy_threading email ensurepip enum "
    "errno faulthandler fcntl filecmp fileinput fnmatch formatter fpectl fractions ftplib functools gc getopt "
    "getpass gettext glob grp gzip hashlib heapq hmac html http http imaplib imghdr importlib inspect io "
    "ipaddress itertools json keyword linecache locale logging lzma macpath mailbox mailcap marshal math "
    "mimetypes mmap modulefinder msilib msvcrt multiprocessing netrc nis nntplib numbers operator "
    "os os ossaudiodev parser pathlib pdb pickle pickletools pipes pkgutil platform plistlib poplib posix "
    "pprint pty pwd py_compile pyclbr queue quopri random re readline reprlib resource rlcompleter runpy "
    "sched select selectors shelve shlex shutil signal site smtpd smtplib sndhdr socket socketserver spwd "
    "sqlite3 ssl stat statistics string stringprep struct subprocess sunau symbol symtable sys sysconfig "
    "syslog tabnanny tarfile telnetlib tempfile termios textwrap threading time timeit tkinter token "
    "tokenize trace traceback tracemalloc tty turtle types unicodedata unittest urllib uu uuid venv warnings "
    "wave weakref webbrowser winreg winsound wsgiref xdrlib xml xmlrpc zipfile zipimport zlib",
    
    NULL,
};


EDITLEXER lexPY =
{
    SCLEX_PYTHON, "python", IDS_LEX_PYTHON, L"Python Script", L"py; pyw; pyx; pxd; pxi; boo; empy; cobra; gs", L"",
    &KeyWords_PY, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_P_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_P_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#007F00", L"" },
        { {SCE_P_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#00007F", L"" },
        { {SCE_P_WORD2}, IDS_LEX_STR_Keyword2nd, L"Keyword 2nd", L"fore:#000088", L"" },
        { {SCE_P_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_P_ATTRIBUTE}, IDS_LEX_STR_Attrib, L"Attribute", L"fore:#008E8E", L"" },
        { {MULTI_STYLE(SCE_P_CHARACTER,SCE_P_FCHARACTER,0,0)}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#008800", L"" },
        { {MULTI_STYLE(SCE_P_STRING,SCE_P_FSTRING,SCE_P_STRINGEOL,0)}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008800", L"" },
        { {MULTI_STYLE(SCE_P_TRIPLE, SCE_P_FTRIPLE,0,0)}, IDS_LEX_STR_63245, L"String Triple Single Quotes", L"fore:#88B634", L"" },
        { {MULTI_STYLE(SCE_P_TRIPLEDOUBLE,SCE_P_FTRIPLEDOUBLE,0,0)}, IDS_LEX_STR_63244, L"String Triple Double Quotes", L"fore:#88B634", L"" },
        { {SCE_P_DECORATOR}, IDS_LEX_STR_Decor, L"Decorator", L"fore:#F2B600", L"" },
        { {SCE_P_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF4000", L"" },
        { {SCE_P_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"bold; fore:#666600", L"" },
        { {SCE_P_DEFNAME}, IDS_LEX_STR_FctName, L"Function Name", L"fore:#660066", L"" },
        { {SCE_P_CLASSNAME}, IDS_LEX_STR_ClassName, L"Class Name", L"fore:#910048", L"" },
        EDITLEXER_SENTINEL
    }
};
