#include "StyleLexers.h"

KEYWORDLIST KeyWords_INNO = {
"code components custommessages dirs files icons ini installdelete langoptions languages messages "
"registry run setup types tasks uninstalldelete uninstallrun _istool",
"allowcancelduringinstall allownetworkdrive allownoicons allowrootdirectory allowuncpath alwaysrestart "
"alwaysshowcomponentslist alwaysshowdironreadypage alwaysshowgrouponreadypage alwaysusepersonalgroup appcomments "
"appcontact appcopyright appenddefaultdirname appenddefaultgroupname appid appmodifypath appmutex appname apppublisher "
"apppublisherurl appreadmefile appsupportphone appsupporturl appupdatesurl appvername appversion architecturesallowed "
"architecturesinstallin64bitmode backcolor backcolor2 backcolordirection backsolid beveledlabel changesassociations "
"changesenvironment closeapplications closeapplicationsfilter compression compressionthreads copyrightfontname "
"copyrightfontsize createappdir createuninstallregkey defaultdirname defaultgroupname defaultuserinfoname "
"defaultuserinfoorg defaultuserinfoserial dialogfontname dialogfontsize direxistswarning disabledirpage "
"disablefinishedpage disableprogramgrouppage disablereadymemo disablereadypage disablestartupprompt "
"disablewelcomepage diskclustersize diskslicesize diskspanning enabledirdoesntexistwarning encryption "
"extradiskspacerequired flatcomponentslist infoafterfile infobeforefile internalcompresslevel languagedetectionmethod "
"languagecodepage languageid languagename licensefile lzmaalgorithm lzmablocksize lzmadictionarysize lzmamatchfinder "
"lzmanumblockthreads lzmanumfastbytes lzmauseseparateprocess mergeduplicatefiles minversion onlybelowversion "
"outputbasefilename outputdir outputmanifestfile password privilegesrequired reservebytes restartapplications "
"restartifneededbyrun righttoleft setupiconfile setuplogging setupmutex showcomponentsizes showlanguagedialog showtaskstreelines "
"showundisplayablelanguages signeduninstaller signeduninstallerdir signtool signtoolretrycount slicesperdisk solidcompression "
"sourcedir strongassemblyname timestamprounding timestampsinutc titlefontname titlefontsize touchdate touchtime uninstallable "
"uninstalldisplayicon uninstalldisplayname uninstallfilesdir uninstalldisplaysize uninstalllogmode uninstallrestartcomputer "
"updateuninstalllogappname usepreviousappdir usepreviousgroup usepreviouslanguage useprevioussetuptype useprevioustasks "
"verb versioninfoproductname useprevioususerinfo userinfopage usesetupldr versioninfocompany versioninfocopyright "
"versioninfodescription versioninfoproductversion versioninfotextversion versioninfoversion versioninfoproducttextversion "
"welcomefontname welcomefontsize windowshowcaption windowstartmaximized windowresizable windowvisible wizardimagealphaformat "
"wizardimagebackcolor wizardimagefile wizardimagestretch wizardsmallimagefile",
"appusermodelid afterinstall attribs beforeinstall check comment components copymode description destdir destname excludes "
"extradiskspacerequired filename flags fontinstall groupdescription hotkey infoafterfile infobeforefile iconfilename "
"iconindex key languages licensefile messagesfile minversion name onlybelowversion parameters permissions root runonceid "
"section source statusmsg string subkey tasks terminalservicesaware type types valuedata valuename valuetype workingdir",
"append define dim else emit elif endif endsub error expr file for if ifdef ifexist ifndef ifnexist include insert pragma "
"sub undef",
"and begin break case const continue do downto else end except finally for function "
"if not of or procedure repeat then to try type until uses var while with",
"", "", "", "" };


EDITLEXER lexINNO = { 
SCLEX_INNOSETUP, IDS_LEX_INNO, L"Inno Setup Script", L"iss; isl; islu", L"", 
&KeyWords_INNO, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_INNO_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { SCE_INNO_COMMENT, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { SCE_INNO_KEYWORD, IDS_LEX_STR_63128, L"Keyword", L"fore:#0000FF", L"" },
    { SCE_INNO_PARAMETER, IDS_LEX_STR_63281, L"Parameter", L"fore:#0000FF", L"" },
    { SCE_INNO_SECTION, IDS_LEX_STR_63232, L"Section", L"fore:#000080; bold", L"" },
    { SCE_INNO_PREPROC, IDS_LEX_STR_63133, L"Preprocessor", L"fore:#CC0000", L"" },
    { SCE_INNO_INLINE_EXPANSION, IDS_LEX_STR_63282, L"Inline Expansion", L"fore:#800080", L"" },
    { SCE_INNO_COMMENT_PASCAL, IDS_LEX_STR_63283, L"Pascal Comment", L"fore:#008000", L"" },
    { SCE_INNO_KEYWORD_PASCAL, IDS_LEX_STR_63284, L"Pascal Keyword", L"fore:#0000FF", L"" },
    { MULTI_STYLE(SCE_INNO_STRING_DOUBLE,SCE_INNO_STRING_SINGLE,0,0), IDS_LEX_STR_63131, L"String", L"", L"" },
    //{ SCE_INNO_IDENTIFIER, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    //{ SCE_INNO_KEYWORD_USER, L"User Defined", L"", L"" },
    { -1, 00000, L"", L"", L"" } } };
