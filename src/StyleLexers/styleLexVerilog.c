#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static KEYWORDLIST KeyWords_Verilog = {

    // 0 Primary keywords and identifiers
    "always and assign automatic "
    "begin buf bufif0 bufif1 "
    "case casex casez cell cmos config "
    "deassign default defparam design disable "
    "edge else end endcase endconfig endfunction endgenerate endmodule endprimitive endspecify endtable endtask event "
    "for force forever fork function "
    "generate genvar "
    "highz0 highz1 "
    "if ifnone incdir include initial inout input instance integer "
    "join "
    "large liblist library localparam "
    "macromodule medium module "
    "nand negedge nmos nor noshowcancelled not notif0 notif1 "
    "or output "
    "parameter pmos posedge primitive pull0 pull1 pulldown pullup pulsestyle_ondetect pulsestyle_onevent "
    "rcmos real realtime reg release repeat rnmos rpmos rtran rtranif0 rtranif1 "
    "scalared showcancelled signed small specify specparam strong0 strong1 supply0 supply1 "
    "table task time tran tranif0 tranif1 tri tri0 tri1 triand trior trireg "
    "unsigned use uwire "
    "vectored "
    "wait wand weak0 weak1 while wire wor "
    "xnor xor",

    // 1 Secondary keywords and identifiers
    "",

    // 2 System Tasks
    "$async$and$array $async$and$plane $async$nand$array $async$nand$plane $async$nor$array $async$nor$plane $async$or$array $async$or$plane "
    "$bitstoreal "
    "$countdrivers "
    "$display $displayb $displayh $displayo "
    "$dist_chi_square $dist_erlang $dist_exponential $dist_normal $dist_poisson $dist_t $dist_uniform "
    "$dumpall $dumpfile $dumpflush $dumplimit $dumpoff $dumpon $dumpportsall $dumpportsflush $dumpportslimit $dumpportsoff $dumpportson $dumpvars "
    "$fclose $fdisplayh $fdisplay $fdisplayf $fdisplayb $feof $ferror $fflush $fgetc $fgets $finish $fmonitorb $fmonitor $fmonitorf $fmonitorh $fopen $fread $fscanf $fseek $fsscanf $fstrobe $fstrobebb $fstrobef $fstrobeh $ftel $fullskew $fwriteb $fwritef $fwriteh $fwrite "
    "$getpattern "
    "$history $hold "
    "$incsave $input $itor "
    "$key "
    "$list $log "
    "$monitorb $monitorh $monitoroff $monitoron $monitor $monitoro "
    "$nochange $nokey $nolog "
    "$period $printtimescale "
    "$q_add $q_exam $q_full $q_initialize $q_remove "
    "$random $readmemb $readmemh $readmemh $realtime $realtobits $recovery $recrem $removal $reset_count $reset $reset_value $restart $rewind $rtoi "
    "$save $scale $scope $sdf_annotate $setup $setuphold $sformat $showscopes $showvariables $showvars $signed $skew $sreadmemb $sreadmemh $stime $stop $strobeb $strobe $strobeh $strobeo $swriteb $swriteh $swriteo $swrite $sync$and$array $sync$and$plane $sync$nand$array $sync$nand$plane $sync$nor$array $sync$nor$plane $sync$or$array $sync$or$plane "
    "$test$plusargs $time $timeformat $timeskew "
    "$ungetc $unsigned "
    "$value$plusargs "
    "$width $writeb $writeh $write $writeo",

    // 3 User defined tasks and identifiers
    "",

    // 4 Documentation comment keywords
    "synopsys parallel_case infer_mux TODO",

    // 5 Preprocessor definitions
    "ifdef ifndef else endif",

    NULL,
};


static KEYWORDLIST KeyWords_SysVerilog = {

    // 0 Primary keywords and identifiers
    "alias always always_comb always_ff always_latch and assert assign assume "
    "automatic before begin bind bins binsof bit break buf bufif0 bufif1 byte case "
    "casex casez cell chandle class clocking cmos config const constraint context "
    "continue cover covergroup coverpoint cross deassign default defparam design "
    "disable dist do edge else end endcase endclass endclocking endconfig "
    "endfunction endgenerate endgroup endinterface endmodule endpackage "
    "endprimitive endprogram endproperty endspecify endsequence endtable endtask "
    "enum event expect export extends extern final first_match for force foreach "
    "forever fork forkjoin function generate genvar highz0 highz1 if iff ifnone "
    "ignore_bins illegal_bins import incdir include initial inout input inside "
    "instance int integer interface intersect join join_any join_none large liblist "
    "library local localparam logic longint macromodule matches medium modport "
    "module nand negedge new nmos nor noshowcancelled not notif0 notif1 null or "
    "output package packed parameter pmos posedge primitive priority program "
    "property protected pull0 pull1 pulldown pullup pulsestyle_onevent "
    "pulsestyle_ondetect pure rand randc randcase randsequence rcmos real realtime "
    "ref reg release repeat return rnmos rpmos rtran rtranif0 rtranif1 scalared "
    "sequence shortint shortreal showcancelled signed small solve specify specparam "
    "static string strong0 strong1 struct super supply0 supply1 table tagged task "
    "this throughout time timeprecision timeunit tran tranif0 tranif1 tri tri0 tri1 "
    "triand trior trireg type typedef union unique unsigned use uwire var vectored "
    "virtual void wait wait_order wand weak0 weak1 while wildcard wire with within "
    "wor xnor xor",

    // 1 Secondary keywords and identifiers
    "",

    // 2 System Tasks
    "$acos $acosh $asin $asinh $assertfailoff $assertfailon $assertkill "
    "$assertnonvacuouson $assertoff $asserton $assertpassoff $assertpasson "
    "$assertvacuousoff $async$and$array $async$and$plane $async$nand$array "
    "$async$nand$plane $async$nor$array $async$nor$plane $async$or$array "
    "$async$or$plane $atan $atan2 $atanh $bits $bitstoreal $bitstoshortreal $cast "
    "$ceil $changed $changed_gclk $changing_gclk $clog2 $cos $cosh $countdrivers "
    "$countones $coverage_control $coverage_get $coverage_get_max $coverage_merge "
    "$coverage_save $dimensions $display $displayb $displayh $displayo "
    "$dist_chi_square $dist_erlang $dist_exponential $dist_normal $dist_poisson "
    "$dist_t $dist_uniform $dumpall $dumpfile $dumpflush $dumplimit $dumpoff "
    "$dumpon $dumpports $dumpportsall $dumpportsflush $dumpportslimit $dumpportsoff "
    "$dumpportson $dumpvars $error $exit $exp $falling_gclk $fatal $fclose "
    "$fdisplay $fdisplayb $fdisplayf $fdisplayh $fdisplayo $fell $fell_gclk $feof "
    "$ferror $fflush $fgetc $fgets $finish $floor $fmonitor $fmonitorb $fmonitorf "
    "$fmonitorh $fmonitoro $fopen $fread $fscanf $fseek $fsscanf $fstrobe $fstrobeb "
    "$fstrobebb $fstrobef $fstrobeh $fstrobeo $ftel $ftell $fullskew $future_gclk "
    "$fwrite $fwriteb $fwritef $fwriteh $fwriteo $get_coverage $getpattern $high "
    "$history $hold $hypot $increment $incsave $info $input $isunbounded $isunknown "
    "$itor $key $left $list $ln $load_coverage_db $log $log10 $low $monitor "
    "$monitorb $monitorh $monitoro $monitoroff $monitoron $nochange $nokey $nolog "
    "$onehot $onehot0 $past $past_gclk $period $pow $printtimescale $q_add $q_exam "
    "$q_full $q_initialize $q_remove $random $readmemb $readmemh $realtime "
    "$realtobits $recovery $recrem $removal $reset $reset_count $reset_value "
    "$restart $rewind $right $rising_gclk $root $rose $rose_gclk $rtoi $sampled "
    "$save $scale $scope $sdf_annotate $set_coverage_db_name $setup $setuphold "
    "$sformat $sformatf $shortrealtobits $showscopes $showvariables $showvars "
    "$signed $sin $sinh $size $skew $sqrt $sreadmemb $sreadmemh $sscanf $stable "
    "$stable_gclk $steady_gclk $stime $stop $strobe $strobeb $strobeh $strobeo "
    "$swrite $swriteb $swriteh $swriteo $sync$and$array $sync$and$plane "
    "$sync$nand$array $sync$nand$plane $sync$nor$array $sync$nor$plane "
    "$sync$or$array $sync$or$plane $system $tan $tanh $test$plusargs $time "
    "$timeformat $timeskew $typename $ungetc $unit $unpacked_dimensions $unsigned "
    "$urandom $urandom_range $value$plusargs $warning $width $write $writeb $writeh "
    "$writememb $writememh $writeo",

    // 3 User defined tasks and identifiers
    "PATHPULSE$ STDERR STDIN STDOUT accept_on and assert assume atobin atohex atoi atooct atoreal await bintoa "
    "compare constraint_mode cover delete exists expect "
    "find find_first find_first_index find_index find_last find_last_index first first_match "
    "get get_coverage get_inst_coverage get_randstate getc hextoa icompare index insert itoa kill last len "
    "mailbox max min name new next num octtoa or "
    "peek pop_back pop_front post_randomize pre_randomize prev process product property "
    "push_back push_front put putc "
    "rand_mode randomize realtoa reject_on resume reverse rsort "
    "sample self semaphore sequence set_inst_name set_randstate shuffle size sort srandom start status std stop "
    "substr sum suspend sync_accept_on sync_reject_on "
    "tolower toupper try_get try_peek try_put unique unique_index wait_order xor",

    // 4 Documentation comment keywords
    "synopsys parallel_case infer_mux TODO",

    // 5 Preprocessor definitions
    "ifdef ifndef else endif",

    NULL,
};


EDITLEXER lexVerilog =
{
    SCLEX_VERILOG, "verilog", // see LexVerilog.cxx: LexerModule lmVerilog
    IDS_LEX_VERILOG, L"Verilog HDL", L"v; vh", L"",
    &KeyWords_Verilog, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_V_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_V_COMMENT, SCE_V_COMMENTLINE, SCE_V_COMMENTLINEBANG, SCE_V_COMMENT_WORD)}, IDS_LEX_STR_63127, L"Comment", L"fore:#008800", L"" },
        { {SCE_V_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {MULTI_STYLE(SCE_V_WORD, SCE_V_WORD2, 0, 0)}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#A8007E", L"" },
        { {SCE_V_WORD3}, IDS_LEX_STR_SysTasks, L"System Tasks", L"fore:#0053A6", L"" },
        { {MULTI_STYLE(SCE_V_STRING, SCE_V_STRINGEOL, 0, 0)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_V_PREPROCESSOR}, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#FF8000", L"" },
        { {SCE_V_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#007070", L"" },
        { {SCE_V_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {SCE_V_USER}, IDS_LEX_STR_63305, L"User-Defined Function", L"fore:#2A00FF", L"" },
        { {SCE_V_INPUT}, IDS_LEX_STR_Input, L"Input", L"fore:#0231AC", L"" },
        { {SCE_V_OUTPUT}, IDS_LEX_STR_Output, L"Output", L"fore:#00007F", L"" },
        { {SCE_V_INOUT}, IDS_LEX_STR_InOut, L"InOut", L"fore:#0000FF", L"" },
        { {SCE_V_PORT_CONNECT}, IDS_LEX_STR_Port_Conn, L"Port Connection", L"bold; fore:#0A246A", L"" },
        EDITLEXER_SENTINEL
    }
};


EDITLEXER lexSysVerilog =
{
    SCLEX_SYSVERILOG, "sysverilog", // see LexVerilog.cxx: LexerModule lmVerilog
    IDS_LEX_SYSVERILOG, L"SystemVerilog HDVL", L"sv; svh", L"",
    &KeyWords_SysVerilog, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        //{ {SCE_V_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_V_COMMENT, SCE_V_COMMENTLINE, SCE_V_COMMENTLINEBANG, SCE_V_COMMENT_WORD)}, IDS_LEX_STR_63127, L"Comment", L"fore:#008800", L"" },
        { {SCE_V_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
        { {MULTI_STYLE(SCE_V_WORD, SCE_V_WORD2, 0, 0)}, IDS_LEX_STR_63128, L"Keyword", L"bold; fore:#A8007E", L"" },
        { {SCE_V_WORD3}, IDS_LEX_STR_SysTasks, L"System Tasks", L"fore:#0053A6", L"" },
        { {MULTI_STYLE(SCE_V_STRING, SCE_V_STRINGEOL, 0, 0)}, IDS_LEX_STR_63131, L"String", L"fore:#008000", L"" },
        { {SCE_V_PREPROCESSOR}, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#FF8000", L"" },
        { {SCE_V_OPERATOR}, IDS_LEX_STR_63132, L"Operator", L"fore:#007070", L"" },
        { {SCE_V_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
        { {SCE_V_USER}, IDS_LEX_STR_63305, L"User-Defined Function", L"fore:#2A00FF", L"" },
        { {SCE_V_INPUT}, IDS_LEX_STR_Input, L"Input", L"fore:#0231AC", L"" },
        { {SCE_V_OUTPUT}, IDS_LEX_STR_Output, L"Output", L"fore:#00007F", L"" },
        { {SCE_V_INOUT}, IDS_LEX_STR_InOut, L"InOut", L"fore:#0000FF", L"" },
        { {SCE_V_PORT_CONNECT}, IDS_LEX_STR_Port_Conn, L"Port Connection", L"bold; fore:#0A246A", L"" },
        EDITLEXER_SENTINEL
    }
};
