#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_SQL = {
"abort accessible action add after all alter analyze and as asc asensitive attach autoincrement "
"before begin between bigint binary bit blob both by call cascade case cast change char character "
"check collate column commit condition conflict constraint continue convert create cross current_date "
"current_time current_timestamp current_user cursor database databases date day_hour day_microsecond "
"day_minute day_second dec decimal declare default deferrable deferred delayed delete desc describe "
"detach deterministic distinct distinctrow div double drop dual each else elseif enclosed end enum "
"escape escaped except exclusive exists exit explain fail false fetch float float4 float8 for force "
"foreign from full fulltext glob grant group having high_priority hour_microsecond hour_minute "
"hour_second if ignore immediate in index infile initially inner inout insensitive insert instead int "
"int1 int2 int3 int4 int8 integer intersect interval into is isnull iterate join key keys kill "
"leading leave left like limit linear lines load localtime localtimestamp lock long longblob longtext "
"loop low_priority master_ssl_verify_server_cert match merge mediumblob mediumint mediumtext middleint "
"minute_microsecond minute_second mod modifies natural no no_write_to_binlog not notnull null numeric "
"of offset on optimize option optionally or order out outer outfile plan pragma precision primary "
"procedure purge query raise range read read_only read_write reads real references regexp reindex "
"release rename repeat replace require restrict return revoke right rlike rollback row rowid schema "
"schemas second_microsecond select sensitive separator set show smallint spatial specific sql "
"sql_big_result sql_calc_found_rows sql_small_result sqlexception sqlstate sqlwarning ssl starting "
"straight_join table temp temporary terminated text then time timestamp tinyblob tinyint tinytext to "
"trailing transaction trigger true undo union unique unlock unsigned update usage use using utc_date "
"utc_time utc_timestamp vacuum values varbinary varchar varcharacter varying view virtual when where "
"while with write xor year_month zerofill",
"", "", "", "", "", "", "", "" };


EDITLEXER lexSQL = { 
SCLEX_SQL, IDS_LEX_SQL, L"SQL Query", L"sql", L"", 
&KeyWords_SQL, {
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ {SCE_SQL_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { {SCE_SQL_COMMENT}, IDS_LEX_STR_63127, L"Comment", L"fore:#505050", L"" },
    { {SCE_SQL_WORD}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#800080", L"" },
    { {MULTI_STYLE(SCE_SQL_STRING,SCE_SQL_CHARACTER,0,0)}, IDS_LEX_STR_63131, L"String", L"fore:#008000; back:#FFF1A8", L"" },
    { {SCE_SQL_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"fore:#800080", L"" },
    { {SCE_SQL_QUOTEDIDENTIFIER}, IDS_LEX_STR_63243, L"Quoted Identifier", L"fore:#800080; back:#FFCCFF", L"" },
    { {SCE_SQL_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
    { {SCE_SQL_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"bold; fore:#800080", L"" },
    EDITLEXER_SENTINEL } };
