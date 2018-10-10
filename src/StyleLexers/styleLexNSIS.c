#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static __int64 LexFunction(LexFunctionType type, int value)
{
  LEX_FUNCTION_BODY(type, value);
  return 0LL;
};

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_NSIS = {
"!addincludedir !addplugindir !appendfile !cd !define !delfile !echo !else !endif !error "
"!execute !finalize !getdllversion !if !ifdef !ifmacrodef !ifmacrondef !ifndef !include !insertmacro !macro "
"!macroend !macroundef !makensis !packhdr !searchparse !searchreplace !system !tempfile !undef !verbose !warning "
".onguiend .onguiinit .oninit .oninstfailed .oninstsuccess .onmouseoversection .onrebootfailed .onselchange "
".onuserabort .onverifyinstdir un.onguiend un.onguiinit un.oninit un.onrebootfailed un.onuninstfailed un.onuninstsuccess "
"un.onuserabort abort addbrandingimage addsize allowrootdirinstall allowskipfiles autoclosewindow "
"bannertrimpath bgfont bggradient brandingtext bringtofront call callinstdll caption changeui checkbitmap "
"clearerrors completedtext componenttext copyfiles crccheck createdirectory createfont createshortcut "
"delete deleteinisec deleteinistr deleteregkey deleteregvalue detailprint detailsbuttontext dirstate dirtext "
"dirvar dirverify enablewindow enumregkey enumregvalue exch exec execshell execwait expandenvstrings "
"file filebufsize fileclose fileerrortext fileexists fileopen fileread filereadbyte filereadutf16le filereadword "
"fileseek filewrite filewritebyte filewriteutf16le filewriteword findclose findfirst findnext findproc "
"findwindow flushini getcurinsttype getcurrentaddress getdlgitem getdllversion getdllversionlocal "
"geterrorlevel getfiletime getfiletimelocal getfontname getfontnamelocal getfontversion getfontversionlocal "
"getfullpathname getfunctionaddress getinstdirerror getlabeladdress gettempfilename goto hidewindow icon "
"ifabort iferrors iffileexists ifrebootflag ifsilent initpluginsdir installbuttontext installcolors installdir "
"installdirregkey instprogressflags insttype insttypegettext insttypesettext intcmp intcmpu intfmt intop "
"iswindow langstring licensebkcolor licensedata licenseforceselection licenselangstring licensetext "
"loadlanguagefile lockwindow logset logtext manifestsupportedos messagebox miscbuttontext name nop outfile page "
"pagecallbacks pop push quit readenvstr readinistr readregdword readregstr reboot regdll rename requestexecutionlevel "
"reservefile return rmdir searchpath sectiongetflags sectiongetinsttypes sectiongetsize sectiongettext sectionin "
"sectionsetflags sectionsetinsttypes sectionsetsize sectionsettext sendmessage setautoclose setbrandingimage "
"setcompress setcompressionlevel setcompressor setcompressordictsize setctlcolors setcurinsttype "
"setdatablockoptimize setdatesave setdetailsprint setdetailsview seterrorlevel seterrors setfileattributes "
"setfont setoutpath setoverwrite setpluginunload setrebootflag setregview setshellvarcontext setsilent "
"showinstdetails showuninstdetails showwindow silentinstall silentuninstall sleep spacetexts strcmp strcmps "
"strcpy strlen subcaption unicode uninstallbuttontext uninstallcaption uninstallicon uninstallsubcaption uninstalltext "
"uninstpage unregdll var viaddversionkey vifileversion viproductversion windowicon writeinistr writeregbin "
"writeregdword writeregexpandstr writeregstr writeuninstaller xpstyle",
"${nsisdir} $0 $1 $2 $3 $4 $5 $6 $7 $8 $9 $r0 $r1 $r2 $r3 $r4 $r5 $r6 $r7 $r8 $r9 $instdir $outdir $cmdline "
"$language $programfiles $programfiles32 $programfiles64 $commonfiles $commonfiles32 $commonfiles64 "
"$desktop $exedir $exefile $exepath $windir $sysdir $temp $startmenu $smprograms $smstartup $quicklaunch "
"$documents $sendto $recent $favorites $music $pictures $videos $nethood $fonts $templates $appdata "
"$localappdata $printhood $internet_cache $cookies $history $profile $admintools $resources $resources_localized "
"$cdburn_area $hwndparent $pluginsdir ${__date__} ${__file__} ${__function__} ${__global__} ${__line__} "
"${__pageex__} ${__section__} ${__time__} ${__timestamp__} ${__uninstall__}",
"alt charset colored control cur date end global ignorecase leave shift smooth utcdate sw_hide sw_showmaximized "
"sw_showminimized sw_shownormal archive auto oname rebootok nonfatal ifempty nounload filesonly short mb_ok "
"mb_okcancel mb_abortretryignore mb_retrycancel mb_yesno mb_yesnocancel mb_iconexclamation mb_iconinformation "
"mb_iconquestion mb_iconstop mb_usericon mb_topmost mb_setforeground mb_right mb_rtlreading mb_defbutton1 "
"mb_defbutton2 mb_defbutton3 mb_defbutton4 idabort idcancel idignore idno idok idretry idyes sd current all "
"timeout imgid resizetofit listonly textonly both branding hkcr hkey_classes_root hklm hkey_local_machine hkcu "
"hkey_current_user hku hkey_users hkcc hkey_current_config hkdd hkey_dyn_data hkpd hkey_performance_data shctx "
"shell_context left right top bottom true false on off italic underline strike trimleft trimright trimcenter "
"idd_license idd_dir idd_selcom idd_inst idd_instfiles idd_uninst idd_verify force windows nocustom customstring "
"componentsonlyoncustom gray none user highest admin lang hide show nevershow normal silent silentlog solid final "
"zlib bzip2 lzma try ifnewer ifdiff lastused manual alwaysoff normal file_attribute_normal file_attribute_archive "
"hidden file_attribute_hidden offline file_attribute_offline readonly file_attribute_readonly system "
"file_attribute_system temporary file_attribute_temporary custom license components directory instfiles "
"uninstconfirm 32 64 enablecancel noworkingdir plugin rawnl winvista win7 win8 win8.1 win10",
"", "", "", "", "", "" };


EDITLEXER lexNSIS = { 
SCLEX_NSIS, IDS_LEX_NSIS, L"NSIS Script", L"nsi; nsh", L"", 
&LexFunction, // static
&KeyWords_NSIS, {
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //,{ {SCE_NSIS_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { {MULTI_STYLE(SCE_NSIS_COMMENT,SCE_NSIS_COMMENTBOX,0,0)}, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { {MULTI_STYLE(SCE_NSIS_STRINGDQ,SCE_NSIS_STRINGLQ,SCE_NSIS_STRINGRQ,0)}, IDS_LEX_STR_63131, L"String", L"fore:#666666; back:#EEEEEE", L"" },
    { {SCE_NSIS_FUNCTION}, IDS_LEX_STR_63273, L"Function", L"fore:#0033CC", L"" },
    { {SCE_NSIS_VARIABLE}, IDS_LEX_STR_63249, L"Variable", L"fore:#CC3300", L"" },
    { {SCE_NSIS_STRINGVAR}, IDS_LEX_STR_63267, L"Variable within String", L"fore:#CC3300; back:#EEEEEE", L"" },
    { {SCE_NSIS_NUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF0000", L"" },
    { {SCE_NSIS_LABEL}, IDS_LEX_STR_63274, L"Constant", L"fore:#FF9900", L"" },
    { {SCE_NSIS_SECTIONDEF}, IDS_LEX_STR_63232, L"Section", L"fore:#0033CC", L"" },
    { {SCE_NSIS_SUBSECTIONDEF}, IDS_LEX_STR_63275, L"Sub Section", L"fore:#0033CC", L"" },
    { {SCE_NSIS_SECTIONGROUP}, IDS_LEX_STR_63276, L"Section Group", L"fore:#0033CC", L"" },
    { {SCE_NSIS_FUNCTIONDEF}, IDS_LEX_STR_63277, L"Function Definition", L"fore:#0033CC", L"" },
    { {SCE_NSIS_PAGEEX}, IDS_LEX_STR_63278, L"PageEx", L"fore:#0033CC", L"" },
    { {SCE_NSIS_IFDEFINEDEF}, IDS_LEX_STR_63279, L"If Definition", L"fore:#0033CC", L"" },
    { {SCE_NSIS_MACRODEF}, IDS_LEX_STR_63280, L"Macro Definition", L"fore:#0033CC", L"" },
    //{ {SCE_NSIS_USERDEFINED}, L"User Defined", L"", L"" },
    EDITLEXER_SENTINEL } };

