#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_INNO =
{
    "_istool code components custommessages dirs files icons ini installdelete langoptions languages messages "
    "registry run setup tasks types uninstalldelete uninstallrun",
    "allowcancelduringinstall allownetworkdrive allownoicons allowrootdirectory allowuncpath alwaysrestart "
    "alwaysshowcomponentslist alwaysshowdironreadypage alwaysshowgrouponreadypage alwaysusepersonalgroup "
    "appcomments appcontact appcopyright appenddefaultdirname appenddefaultgroupname appid appmodifypath appmutex "
    "appname apppublisher apppublisherurl appreadmefile appsupportphone appsupporturl appupdatesurl appvername "
    "appversion architecturesallowed architecturesinstallin64bitmode aslrcompatible backcolor backcolor2 "
    "backcolordirection backsolid beveledlabel changesassociations changesenvironment closeapplications "
    "closeapplicationsfilter compression compressionthreads copyrightfontname copyrightfontsize createappdir "
    "createuninstallregkey defaultdialogfontname defaultdirname defaultgroupname defaultuserinfoname "
    "defaultuserinfoorg defaultuserinfoserial depcompatible dialogfontname dialogfontsize direxistswarning "
    "disabledirpage disablefinishedpage disableprogramgrouppage disablereadymemo disablereadypage "
    "disablestartupprompt disablewelcomepage diskclustersize diskslicesize diskspanning enabledirdoesntexistwarning "
    "encryption extradiskspacerequired flatcomponentslist iconindex infoafterfile infobeforefile "
    "internalcompresslevel languagecodepage languagedetectionmethod languageid languagename licensefile "
    "lzmaalgorithm lzmablocksize lzmadictionarysize lzmamatchfinder lzmanumblockthreads lzmanumfastbytes "
    "lzmauseseparateprocess mergeduplicatefiles minversion missingrunonceidswarning onlybelowversion output "
    "outputbasefilename outputdir outputmanifestfile password privilegesrequired privilegesrequiredoverridesallowed "
    "reservebytes restartapplications restartifneededbyrun righttoleft setupiconfile setuplogging setupmutex "
    "showcomponentsizes showlanguagedialog showtaskstreelines showundisplayablelanguages signeduninstaller "
    "signeduninstallerdir signtool signtoolminimumtimebetween signtoolretrycount signtoolretrydelay "
    "signtoolrunminimized slicesperdisk solidcompression sourcedir strongassemblyname terminalservicesaware "
    "timestamprounding timestampsinutc titlefontname titlefontsize touchdate touchtime uninstallable "
    "uninstalldisplayicon uninstalldisplayname uninstalldisplaysize uninstallfilesdir uninstalllogmode "
    "uninstallrestartcomputer updateuninstalllogappname useduserareaswarning usepreviousappdir usepreviousgroup "
    "usepreviouslanguage usepreviousprivigeles useprevioussetuptype useprevioustasks useprevioususerinfo "
    "userinfopage usesetupldr verb versioninfocompany versioninfocopyright versioninfodescription "
    "versioninfooriginalfilename versioninfoproductname versioninfoproducttextversion versioninfoproductversion "
    "versioninfotextversion versioninfoversion welcomefontname welcomefontsize windowresizable windowshowcaption "
    "windowstartmaximized windowvisible wizardimagealphaformat wizardimagefile wizardimagestretch wizardresizable "
    "wizardsizepercent wizardsmallimagefile wizardstyle",
    "afterinstall appusermodelid attribs beforeinstall check comment components copymode description destdir "
    "destname excludes extradiskspacerequired filename flags fontinstall groupdescription hotkey iconfilename "
    "iconindex infoafterfile infobeforefile key languages licensefile messagesfile minversion name "
    "onlybelowversion parameters permissions root runonceid section source statusmsg string subkey tasks "
    "terminalservicesaware type types valuedata valuename valuetype workingdir",
    "append define dim elif else emit endif endsub error expr file for if ifdef ifexist ifndef ifnexist include "
    "insert pragma sub undef",
    "and begin break case const continue do downto else end except finally for function if not of or procedure "
    "repeat then to try type until uses var while with",
    NULL,
};


EDITLEXER lexINNO =
{
    SCLEX_INNOSETUP, "inno", IDS_LEX_INNO, L"Inno Setup Script", L"iss; isl; islu", L"",
    &KeyWords_INNO, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_INNO_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_INNO_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_INNO_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#0000FF", L"" },
        { {SCE_INNO_PARAMETER}, IDS_LEX_STR_Param, L"Parameter", L"fore:#0000FF", L"" },
        { {SCE_INNO_SECTION}, IDS_LEX_STR_Section, L"Section", L"bold; fore:#000080", L"" },
        { {SCE_INNO_PREPROC}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#CC0000", L"" },
        { {SCE_INNO_INLINE_EXPANSION}, IDS_LEX_STR_63282, L"Inline Expansion", L"fore:#800080", L"" },
        { {SCE_INNO_COMMENT_PASCAL}, IDS_LEX_STR_63283, L"Pascal Comment", L"fore:#008000", L"" },
        { {SCE_INNO_KEYWORD_PASCAL}, IDS_LEX_STR_63284, L"Pascal Keyword", L"fore:#0000FF", L"" },
        { {MULTI_STYLE(SCE_INNO_STRING_DOUBLE,SCE_INNO_STRING_SINGLE,0,0)}, IDS_LEX_STR_String, L"String", L"", L"" },
        //{ {SCE_INNO_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        //{ {SCE_INNO_KEYWORD_USER}, L"User Defined", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
