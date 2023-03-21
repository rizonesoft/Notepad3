#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_NSIS =
{
    "!addincludedir !addplugindir !appendfile !cd !define !delfile !echo !else !endif !error !execute !finalize "
    "!getdllversion !if !ifdef !ifmacrodef !ifmacrondef !ifndef !include !insertmacro !macro !macroend "
    "!macroundef !makensis !packhdr !searchparse !searchreplace !system !tempfile !undef !verbose !warning "
    ".onguiend .onguiinit .oninit .oninstfailed .oninstsuccess .onmouseoversection .onrebootfailed "
    ".onselchange .onuserabort .onverifyinstdir abort addbrandingimage addsize allowrootdirinstall "
    "allowskipfiles autoclosewindow bannertrimpath bgfont bggradient brandingtext bringtofront call "
    "callinstdll caption changeui checkbitmap clearerrors completedtext componenttext copyfiles crccheck "
    "createdirectory createfont createshortcut delete deleteinisec deleteinistr deleteregkey deleteregvalue "
    "detailprint detailsbuttontext dirstate dirtext dirvar dirverify enablewindow enumregkey enumregvalue exch "
    "exec execshell execwait expandenvstrings file filebufsize fileclose fileerrortext fileexists fileopen "
    "fileread filereadbyte filereadutf16le filereadword fileseek filewrite filewritebyte filewriteutf16le "
    "filewriteword findclose findfirst findnext findproc findwindow flushini getcurinsttype getcurrentaddress "
    "getdlgitem getdllversion getdllversionlocal geterrorlevel getfiletime getfiletimelocal getfontname "
    "getfontnamelocal getfontversion getfontversionlocal getfullpathname getfunctionaddress getinstdirerror "
    "getlabeladdress gettempfilename goto hidewindow icon ifabort iferrors iffileexists ifrebootflag ifsilent "
    "initpluginsdir installbuttontext installcolors installdir installdirregkey instprogressflags insttype "
    "insttypegettext insttypesettext intcmp intcmpu intfmt intop iswindow langstring licensebkcolor "
    "licensedata licenseforceselection licenselangstring licensetext loadlanguagefile lockwindow logset "
    "logtext manifestsupportedos messagebox miscbuttontext name nop outfile page pagecallbacks pop push quit "
    "readenvstr readinistr readregdword readregstr reboot regdll rename requestexecutionlevel reservefile "
    "return rmdir searchpath sectiongetflags sectiongetinsttypes sectiongetsize sectiongettext sectionin "
    "sectionsetflags sectionsetinsttypes sectionsetsize sectionsettext sendmessage setautoclose "
    "setbrandingimage setcompress setcompressionlevel setcompressor setcompressordictsize setctlcolors "
    "setcurinsttype setdatablockoptimize setdatesave setdetailsprint setdetailsview seterrorlevel seterrors "
    "setfileattributes setfont setoutpath setoverwrite setpluginunload setrebootflag setregview "
    "setshellvarcontext setsilent showinstdetails showuninstdetails showwindow silentinstall silentuninstall "
    "sleep spacetexts strcmp strcmps strcpy strlen subcaption un.onguiend un.onguiinit un.oninit "
    "un.onrebootfailed un.onuninstfailed un.onuninstsuccess un.onuserabort unicode uninstallbuttontext "
    "uninstallcaption uninstallicon uninstallsubcaption uninstalltext uninstpage unregdll var viaddversionkey "
    "vifileversion viproductversion windowicon writeinistr writeregbin writeregdword writeregexpandstr "
    "writeregstr writeuninstaller xpstyle",
    "$0 $1 $2 $3 $4 $5 $6 $7 $8 $9 $admintools $appdata $cdburn_area $cmdline $commonfiles $commonfiles32 "
    "$commonfiles64 $cookies $desktop $documents $exedir $exefile $exepath $favorites $fonts $history "
    "$hwndparent $instdir $internet_cache $language $localappdata $music $nethood $outdir $pictures "
    "$pluginsdir $printhood $profile $programfiles $programfiles32 $programfiles64 $quicklaunch $r0 $r1 $r2 "
    "$r3 $r4 $r5 $r6 $r7 $r8 $r9 $recent $resources $resources_localized $sendto $smprograms $smstartup "
    "$startmenu $sysdir $temp $templates $videos $windir ${__date__} ${__file__} ${__function__} ${__global__} "
    "${__line__} ${__pageex__} ${__section__} ${__time__} ${__timestamp__} ${__uninstall__} ${nsisdir}",
    "32 64 admin all alt alwaysoff archive auto both bottom branding bzip2 charset colored components "
    "componentsonlyoncustom control cur current custom customstring date directory enablecancel end false "
    "file_attribute_archive file_attribute_hidden file_attribute_normal file_attribute_offline "
    "file_attribute_readonly file_attribute_system file_attribute_temporary filesonly final force global gray "
    "hidden hide highest hkcc hkcr hkcu hkdd hkey_classes_root hkey_current_config hkey_current_user "
    "hkey_dyn_data hkey_local_machine hkey_performance_data hkey_users hklm hkpd hku idabort idcancel idd_dir "
    "idd_inst idd_instfiles idd_license idd_selcom idd_uninst idd_verify idignore idno idok idretry idyes "
    "ifdiff ifempty ifnewer ignorecase imgid instfiles italic lang lastused leave left license listonly lzma "
    "manual mb_abortretryignore mb_defbutton1 mb_defbutton2 mb_defbutton3 mb_defbutton4 mb_iconexclamation "
    "mb_iconinformation mb_iconquestion mb_iconstop mb_ok mb_okcancel mb_retrycancel mb_right mb_rtlreading "
    "mb_setforeground mb_topmost mb_usericon mb_yesno mb_yesnocancel nevershow nocustom none nonfatal normal "
    "normal nounload noworkingdir off offline on oname plugin rawnl readonly rebootok resizetofit right sd "
    "shctx shell_context shift short show silent silentlog smooth solid strike sw_hide sw_showmaximized "
    "sw_showminimized sw_shownormal system temporary textonly timeout top trimcenter trimleft trimright true "
    "try underline uninstconfirm user utcdate win10 win7 win8 win8.1 windows winvista zlib",
    NULL,
};


EDITLEXER lexNSIS =
{
    SCLEX_NSIS, "nsis", IDS_LEX_NSIS, L"NSIS Script", L"nsi; nsh", L"",
    &KeyWords_NSIS, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //,{ {SCE_NSIS_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_NSIS_COMMENT,SCE_NSIS_COMMENTBOX,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {MULTI_STYLE(SCE_NSIS_STRINGDQ,SCE_NSIS_STRINGLQ,SCE_NSIS_STRINGRQ,0)}, IDS_LEX_STR_String, L"String", L"fore:#666666; back:#EEEEEE", L"" },
        { {SCE_NSIS_FUNCTION}, IDS_LEX_STR_Function, L"Function", L"fore:#0033CC", L"" },
        { {SCE_NSIS_VARIABLE}, IDS_LEX_STR_Var, L"Variable", L"fore:#CC3300", L"" },
        { {SCE_NSIS_STRINGVAR}, IDS_LEX_STR_63267, L"Variable within String", L"fore:#CC3300; back:#EEEEEE", L"" },
        { {SCE_NSIS_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_NSIS_LABEL}, IDS_LEX_STR_Const, L"Constant", L"fore:#FF9900", L"" },
        { {SCE_NSIS_SECTIONDEF}, IDS_LEX_STR_Section, L"Section", L"fore:#0033CC", L"" },
        { {SCE_NSIS_SUBSECTIONDEF}, IDS_LEX_STR_63275, L"Sub Section", L"fore:#0033CC", L"" },
        { {SCE_NSIS_SECTIONGROUP}, IDS_LEX_STR_63276, L"Section Group", L"fore:#0033CC", L"" },
        { {SCE_NSIS_FUNCTIONDEF}, IDS_LEX_STR_63277, L"Function Definition", L"fore:#0033CC", L"" },
        { {SCE_NSIS_PAGEEX}, IDS_LEX_STR_PageEx, L"PageEx", L"fore:#0033CC", L"" },
        { {SCE_NSIS_IFDEFINEDEF}, IDS_LEX_STR_63279, L"If Definition", L"fore:#0033CC", L"" },
        { {SCE_NSIS_MACRODEF}, IDS_LEX_STR_63280, L"Macro Definition", L"fore:#0033CC", L"" },
        //{ {SCE_NSIS_USERDEFINED}, L"User Defined", L"", L"" },
        EDITLEXER_SENTINEL
    }
};

