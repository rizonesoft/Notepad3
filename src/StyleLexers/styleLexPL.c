#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_PL =
{
    "AUTOLOAD BEGIN CHECK CORE DESTROY END EQ GE GT INIT LE LT NE NULL UNITCHECK __DATA__ __END__ __FILE__ "
    "__LINE__ __PACKAGE__ abs accept alarm and atan2 bind binmode bless break caller chdir chmod chomp chop "
    "chown chr chroot close closedir cmp connect continue cos crypt dbmclose dbmopen default defined delete "
    "die do dump each else elsif endgrent endhostent endnetent endprotoent endpwent endservent eof eq eval "
    "exec exists exit exp fcntl fileno flock for foreach fork format formline ge getc getgrent getgrgid "
    "getgrnam gethostbyaddr gethostbyname gethostent getlogin getnetbyaddr getnetbyname getnetent getpeername "
    "getpgrp getppid getpriority getprotobyname getprotobynumber getprotoent getpwent getpwnam getpwuid "
    "getservbyname getservbyport getservent getsockname getsockopt given glob gmtime goto grep gt hex if index "
    "int ioctl join keys kill last lc lcfirst le length link listen local localtime lock log lstat lt map "
    "mkdir msgctl msgget msgrcv msgsnd my ne next no not oct open opendir or ord our pack package pipe pop pos "
    "print printf prototype push qu quotemeta rand read readdir readline readlink readpipe recv redo ref "
    "rename require reset return reverse rewinddir rindex rmdir say scalar seek seekdir select semctl semget "
    "semop send setgrent sethostent setnetent setpgrp setpriority setprotoent setpwent setservent setsockopt "
    "shift shmctl shmget shmread shmwrite shutdown sin sleep socket socketpair sort splice split sprintf sqrt "
    "srand stat state study sub substr symlink syscall sysopen sysread sysseek system syswrite tell telldir "
    "tie tied time times truncate uc ucfirst umask undef unless unlink unpack unshift untie until use utime "
    "values vec wait waitpid wantarray warn when while write xor",
    NULL,
};


EDITLEXER lexPL =
{
    SCLEX_PERL, "perl", IDS_LEX_PERL_SCR, L"Perl Script", L"pl; pm; cgi; pod; plx; stp", L"",
    &KeyWords_PL, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_PL_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_PL_COMMENTLINE}, IDS_LEX_STR_Comment, L"Comment", L"fore:#646464", L"" },
        { {SCE_PL_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#804000", L"" },
        { {SCE_PL_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_PL_STRING}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008000", L"" },
        { {SCE_PL_CHARACTER}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#008000", L"" },
        { {SCE_PL_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_PL_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"bold", L"" },
        { {SCE_PL_SCALAR}, IDS_LEX_STR_63215, L"Scalar $var", L"fore:#0A246A", L"" },
        { {SCE_PL_ARRAY}, IDS_LEX_STR_63216, L"Array @var", L"fore:#003CE6", L"" },
        { {SCE_PL_HASH}, IDS_LEX_STR_63217, L"Hash %var", L"fore:#B000B0", L"" },
        { {SCE_PL_SYMBOLTABLE}, IDS_LEX_STR_63218, L"Symbol Table *var", L"fore:#3A6EA5", L"" },
        { {SCE_PL_REGEX}, IDS_LEX_STR_63219, L"Regex /re/ or m{re}", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_PL_REGSUBST}, IDS_LEX_STR_63220, L"Substitution s/re/ore/", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_PL_BACKTICKS}, IDS_LEX_STR_63221, L"Back Ticks", L"fore:#E24000; back:#FFF1A8", L"" },
        { {SCE_PL_HERE_DELIM}, IDS_LEX_STR_63223, L"Here-Doc (Delimiter)", L"fore:#648000", L"" },
        { {SCE_PL_HERE_Q}, IDS_LEX_STR_63224, L"Here-Doc (Single Quoted, q)", L"fore:#648000", L"" },
        { {SCE_PL_HERE_QQ}, IDS_LEX_STR_63225, L"Here-Doc (Double Quoted, qq)", L"fore:#648000", L"" },
        { {SCE_PL_HERE_QX}, IDS_LEX_STR_63226, L"Here-Doc (Back Ticks, qx)", L"fore:#E24000; back:#FFF1A8", L"" },
        { {SCE_PL_STRING_Q}, IDS_LEX_STR_63227, L"Single Quoted String (Generic, q)", L"fore:#008000", L"" },
        { {SCE_PL_STRING_QQ}, IDS_LEX_STR_63228, L"Double Quoted String (qq)", L"fore:#008000", L"" },
        { {SCE_PL_STRING_QX}, IDS_LEX_STR_63229, L"Back Ticks (qx)", L"fore:#E24000; back:#FFF1A8", L"" },
        { {SCE_PL_STRING_QR}, IDS_LEX_STR_63230, L"Regex (qr)", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_PL_STRING_QW}, IDS_LEX_STR_63231, L"Array (qw)", L"fore:#003CE6", L"" },
        { {SCE_PL_SUB_PROTOTYPE}, IDS_LEX_STR_Prototyp, L"Prototype", L"fore:#800080; back:#FFE2FF", L"" },
        { {SCE_PL_FORMAT_IDENT}, IDS_LEX_STR_63254, L"Format Identifier", L"bold; fore:#648000; back:#FFF1A8", L"" },
        { {SCE_PL_FORMAT}, IDS_LEX_STR_63255, L"Format Body", L"fore:#648000; back:#FFF1A8", L"" },
        { {SCE_PL_POD}, IDS_LEX_STR_63213, L"POD (Common)", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
        { {SCE_PL_POD_VERB}, IDS_LEX_STR_63214, L"POD (Verbatim)", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
        { {SCE_PL_DATASECTION}, IDS_LEX_STR_63222, L"Data Section", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
        { {SCE_PL_ERROR}, IDS_LEX_STR_63252, L"Parsing Error", L"fore:#C80000; back:#FFFF80", L"" },
        //{ {SCE_PL_PUNCTUATION}, L"Symbols / Punctuation (not used)", L"", L"" },
        //{ {SCE_PL_PREPROCESSOR}, L"Preprocessor (not used)", L"", L"" },
        //{ {SCE_PL_LONGQUOTE}, L"Long Quote (qq, qr, qw, qx) (not used)", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
