#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_SQL =
{
// Keywords (keywords1)
    "abort accessible account action active add admin after against aggregate algorithm all allocate alter "
    "always analyse analyze and any apply as asc ascii asensitive at attach audit authorization authors "
    "auto_increment autoextend_size autoincrement avg avg_row_length backup before begin between binlog block "
    "both break btree buckets by cache call cascade cascaded case cast catalog_name chain change changed "
    "channel charset check checksum cipher class_origin client clone close cluster coalesce code collate "
    "collation column column_format column_name columns comment commit committed compact completion component "
    "compressed compression concurrent condition conflict connect connection consistent constraint "
    "constraint_catalog constraint_name constraint_schema contains context continue contributors convert cpu "
    "create cross cube cume_dist current current_date current_path current_time current_timestamp current_user "
    "cursor cursor_name data database databases datafile day_hour day_microsecond day_minute day_second "
    "deallocate declare default default_auth deferrable deferred definer definition delay_key_write delayed "
    "delete delimiter dense_rank des_key_file desc describe description descriptor detach deterministic "
    "diagnostics directory disable discard disconnect disk distinct distinctrow div do drop dual dumpfile "
    "duplicate dynamic each else elseif empty enable enclosed encryption end ends engine engines error errors "
    "escape escaped event events every except exception exchange exclude exclusive exec execute exists exit "
    "expansion expire explain export extended extent_size external fail false fast faults fetch fields file "
    "file_block_size filter first first_value flush following follows for force foreign format found "
    "frac_second free from full fulltext function general generated get get_format get_master_public_key glob "
    "global go goto grant grants group group_replication grouping groups handler hash having help "
    "high_priority histogram history hold host hosts hour_microsecond hour_minute hour_second identified "
    "identity if ignore ignore_server_ids immediate import in inactive index indexed indexes infile "
    "initial_size initially inner innobase innodb inout insensitive insert insert_method install instance "
    "instead intersect interval into invoker io io_after_gtids io_before_gtids io_thread ipc is isnull "
    "isolation issuer iterate join json_table key key_block_size keys kill lag language last last_value lead "
    "leading leave leaves left less level like limit linear lines list load local localtime localtimestamp "
    "locator lock locked locks logfile logs loop low_priority master master_auto_position master_bind "
    "master_connect_retry master_delay master_heartbeat_period master_host master_log_file master_log_pos "
    "master_password master_port master_public_key_path master_retry_count master_server_id master_ssl "
    "master_ssl_ca master_ssl_capath master_ssl_cert master_ssl_cipher master_ssl_crl master_ssl_crlpath "
    "master_ssl_key master_ssl_verify_server_cert master_tls_version master_user match "
    "max_connections_per_hour max_queries_per_hour max_rows max_size max_statement_time max_updates_per_hour "
    "max_user_connections maxvalue medium memory merge message_text microsecond middleint migrate min_rows "
    "minute_microsecond minute_second mod mode modifies modify mutex mysql_errno name names national natural "
    "ndb ndbcluster nested never new next no no_wait no_write_to_binlog nocase nodegroup nonblocking none not "
    "notnull nowait nth_value ntile null nullif nulls number of offset old old_password on one one_shot only "
    "open optimize optimizer_costs option optional optionally options or order ordinality organization others "
    "out outer outfile over owner pack_keys page parse_gcol_expr parser partial partition partitioning "
    "partitions password path percent_rank persist persist_only phase plan plugin plugin_dir plugins port "
    "pragma precedes preceding precision prepare preserve prev primary prior privileges procedure process "
    "processlist profile profiles proxy public purge quarter query quick raise range rank read read_only "
    "read_write reads rebuild recover recursive redo_buffer_size redofile redundant reference references "
    "regexp reindex relay relay_log_file relay_log_pos relay_thread relaylog release reload remote remove "
    "rename reorganize repair repeat repeatable replace replicate_do_db replicate_do_table replicate_ignore_db "
    "replicate_ignore_table replicate_rewrite_db replicate_wild_do_table replicate_wild_ignore_table "
    "replication require reset resignal resource respect restart restore restrict result resume return "
    "returned_sqlstate returning returns reuse reverse revoke right rlike role rollback rollup rotate routine "
    "row row_count row_format row_number rowid rows rtree rtrim savepoint schedule schema schema_name schemas "
    "second_microsecond secondary_engine secondary_load secondary_unload security select sensitive separator "
    "sequence serializable server session session_user set share show shutdown signal simple size skip slave "
    "slow snapshot socket some soname sounds source spatial specific sql sql_after_gtids sql_after_mts_gaps "
    "sql_before_gtids sql_big_result sql_buffer_result sql_cache sql_calc_found_rows sql_no_cache "
    "sql_small_result sql_thread sql_tsi_day sql_tsi_frac_second sql_tsi_hour sql_tsi_minute sql_tsi_month "
    "sql_tsi_quarter sql_tsi_second sql_tsi_week sql_tsi_year sqlexception sqlstate sqlwarning srid ssl "
    "stacked start starting starts static stats_auto_recalc stats_persistent stats_sample_pages status stop "
    "storage stored straight_join string subclass_origin subject subpartition subpartitions super suspend "
    "swaps switches synonym system system_user table table_checksum table_name tables tablespace temp "
    "temporary temptable terminated than then thread_priority ties timestampadd timestampdiff to trailing "
    "transaction trigger triggers true truncate type types unbounded uncommitted undefined undo "
    "undo_buffer_size undofile unicode uninstall union unique unknown unlock until update upgrade usage use "
    "use_frm user user_resources using utc_date utc_time utc_timestamp vacuum validation value values "
    "variables vcpu view virtual visible wait warnings week weight_string when whenever where while window "
    "with within without work wrapper write x509 xa xid xml xor year_month zerofill",
// Database Objects (keywords2)
    "",
// PLDoc (kw_pldoc)
    "",
// SQL*Plus (kw_sqlplus)
    "",
// User Keywords 1 (kw_user1)
    "bigint binary bit blob bool boolean byte char character clob date datetime day dec decimal double enum "
    "fixed float float4 float8 hour int int1 int2 int3 int4 int8 integer json long mediumint minute month "
    "native nchar numeric nvarchar real second serial signed smallint text time timestamp tinyint unsigned "
    "varbinary varchar varcharacter varying year",
// (MySQL) (kw_user2)
    "geomcollection geometry geometrycollection linestring longblob longtext mediumblob mediumtext "
    "multilinestring multipoint multipolygon point polygon tinyblob tinytext",
// User Keywords 3
    ""
// User Keywords 4
    "",
    NULL,
};


EDITLEXER lexSQL =
{
    SCLEX_SQL, "sql", IDS_LEX_SQL, L"SQL Query", L"sql; mysql; hsql; pkb", L"",
    &KeyWords_SQL, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_SQL_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_SQL_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008080", L"" },
        { {SCE_SQL_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#3E3EFF", L"" },
        { {SCE_SQL_USER1}, IDS_LEX_STR_63286, L"Value Type", L"bold; fore:#000080", L"" },
        { {MULTI_STYLE(SCE_SQL_STRING,SCE_SQL_CHARACTER,0,0)}, IDS_LEX_STR_String, L"String", L"fore:#808080", L"" },
        { {SCE_SQL_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"fore:#000000", L"" },
        { {SCE_SQL_QUOTEDIDENTIFIER}, IDS_LEX_STR_63243, L"Quoted Identifier", L"fore:#000080; back:#FFFFE2", L"" },
        { {SCE_SQL_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#A200A2", L"" },
        { {SCE_SQL_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"bold; fore:#FF8000", L"" },
        EDITLEXER_SENTINEL
    }
};
