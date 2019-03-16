#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_BASH = {
"alias ar asa awk banner basename bash bc bdiff break bunzip2 bzip2 cal calendar case cat "
"cc cd chmod cksum clear cmp col comm compress continue cp cpio crypt csplit ctags cut date "
"dc dd declare deroff dev df diff diff3 dircmp dirname do done du echo ed egrep elif else "
"env esac eval ex exec exit expand export expr false fc fgrep fi file find fmt fold for function "
"functions getconf getopt getopts grep gres hash head help history iconv id if in integer "
"jobs join kill local lc let line ln logname look ls m4 mail mailx make man mkdir more mt mv "
"newgrp nl nm nohup ntps od pack paste patch pathchk pax pcat perl pg pr print printf ps pwd "
"read readonly red return rev rm rmdir sed select set sh shift size sleep sort spell split "
"start stop strings strip stty sum suspend sync tail tar tee test then time times touch tr "
"trap true tsort tty type typeset ulimit umask unalias uname uncompress unexpand uniq unpack "
"unset until uudecode uuencode vi vim vpax wait wc whence which while who wpaste wstart xargs "
"zcat chgrp chown chroot dir dircolors factor groups hostid install link md5sum mkfifo mknod "
"nice pinky printenv ptx readlink seq sha1sum shred stat su tac unlink users vdir whoami yes",
"", "", "", "", "", "", "", "" };


EDITLEXER lexBASH = { 
SCLEX_BASH, IDS_LEX_SHELL_SCR, L"Shell Script", L"sh", L"", 
&KeyWords_BASH, {
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ {SCE_SH_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { {SCE_SH_ERROR}, IDS_LEX_STR_63261, L"Error", L"", L"" },
    { {SCE_SH_COMMENTLINE}, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { {SCE_SH_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#008080", L"" },
    { {SCE_SH_WORD}, IDS_LEX_STR_63128, L"Keyword", L"fore:#0000FF", L"" },
    { {SCE_SH_STRING}, IDS_LEX_STR_63211, L"String Double Quoted", L"fore:#008080", L"" },
    { {SCE_SH_CHARACTER}, IDS_LEX_STR_63212, L"String Single Quoted", L"fore:#800080", L"" },
    { {SCE_SH_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"", L"" },
    { {SCE_SH_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    { {SCE_SH_SCALAR}, IDS_LEX_STR_63268, L"Scalar", L"fore:#808000", L"" },
    { {SCE_SH_PARAM}, IDS_LEX_STR_63269, L"Parameter Expansion", L"fore:#808000; back:#FFFF99", L"" },
    { {SCE_SH_BACKTICKS}, IDS_LEX_STR_63270, L"Back Ticks", L"fore:#FF0080", L"" },
    { {SCE_SH_HERE_DELIM}, IDS_LEX_STR_63271, L"Here-Doc (Delimiter)", L"", L"" },
    { {SCE_SH_HERE_Q}, IDS_LEX_STR_63272, L"Here-Doc (Single Quoted, q)", L"fore:#008080", L"" },
    EDITLEXER_SENTINEL } };
