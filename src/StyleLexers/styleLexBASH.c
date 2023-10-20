#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_BASH =
{
    "alias ar asa awk banner basename bash bc bdiff break bunzip2 bzip2 cal calendar case cat cc cd chgrp chmod "
    "chown chroot cksum clear cmp col comm compress continue cp cpio crypt csplit ctags cut date dc dd declare "
    "deroff dev df diff diff3 dir dircmp dircolors dirname do done du echo ed egrep elif else env esac eval ex "
    "exec exit expand export expr factor false fc fgrep fi file find fmt fold for function functions getconf "
    "getopt getopts grep gres groups hash head help history hostid iconv id if in install integer jobs join "
    "kill lc let line link ln local logname look ls m4 mail mailx make man md5sum mkdir mkfifo mknod more mt "
    "mv newgrp nice nl nm nohup ntps od pack paste patch pathchk pax pcat perl pg pinky pr print printenv "
    "printf ps ptx pwd read readlink readonly red return rev rm rmdir sed select seq set sh sha1sum shift "
    "shred size sleep sort spell split start stat stop strings strip stty su sum suspend sync tac tail tar tee "
    "test then time times touch tr trap true tsort tty type typeset ulimit umask unalias uname uncompress "
    "unexpand uniq unlink unpack unset until users uudecode uuencode vdir vi vim vpax wait wc whence which "
    "while who whoami wpaste wstart xargs yes zcat",
    NULL,
};


EDITLEXER lexBASH =
{
    SCLEX_BASH, "bash", IDS_LEX_SHELL_SCR, L"Shell Script", L"sh; csh; zsh; bash; tcsh; m4; in; \\^mozconfig$", L"",
    &KeyWords_BASH, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_SH_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_SH_ERROR}, IDS_LEX_STR_Error, L"Error", L"", L"" },
        { {SCE_SH_COMMENTLINE}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_SH_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#008080", L"" },
        { {SCE_SH_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#0000FF", L"" },
        { {SCE_SH_STRING}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008080", L"" },
        { {SCE_SH_CHARACTER}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#800080", L"" },
        { {SCE_SH_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"", L"" },
        { {SCE_SH_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_SH_SCALAR}, IDS_LEX_STR_Scalar, L"Scalar", L"fore:#808000", L"" },
        { {SCE_SH_PARAM}, IDS_LEX_STR_Param, L"Parameter Expansion", L"fore:#808000; back:#FFFF99", L"" },
        { {SCE_SH_BACKTICKS}, IDS_LEX_STR_63221, L"Back Ticks", L"fore:#FF0080", L"" },
        { {SCE_SH_HERE_DELIM}, IDS_LEX_STR_63223, L"Here-Doc (Delimiter)", L"", L"" },
        { {SCE_SH_HERE_Q}, IDS_LEX_STR_63224, L"Here-Doc (Single Quoted, q)", L"fore:#008080", L"" },
        EDITLEXER_SENTINEL
    }
};
