#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_CONF =
{
    "acceptfilter acceptmutex acceptpathinfo accessconfig accessfilename action addalt addaltbyencoding "
    "addaltbytype addcharset adddefaultcharset adddescription addencoding addhandler addicon addiconbyencoding "
    "addiconbytype addinputfilter addlanguage addmodule addmoduleinfo addoutputfilter addoutputfilterbytype "
    "addtype agentlog alias aliasmatch all allow allowconnect allowencodedslashes allowmethods allowoverride "
    "allowoverridelist anonymous anonymous_authoritative anonymous_logemail anonymous_mustgiveemail "
    "anonymous_nouserid anonymous_verifyemail assignuserid asyncrequestworkerfactor authauthoritative "
    "authbasicauthoritative authbasicfake authbasicprovider authbasicusedigestalgorithm authdbauthoritative "
    "authdbduserpwquery authdbduserrealmquery authdbgroupfile authdbmauthoritative authdbmgroupfile "
    "authdbmtype authdbmuserfile authdbuserfile authdigestalgorithm authdigestdomain authdigestfile "
    "authdigestgroupfile authdigestnccheck authdigestnonceformat authdigestnoncelifetime authdigestprovider "
    "authdigestqop authdigestshmemsize authformauthoritative authformbody authformdisablenostore "
    "authformfakebasicauth authformlocation authformloginrequiredlocation authformloginsuccesslocation "
    "authformlogoutlocation authformmethod authformmimetype authformpassword authformprovider "
    "authformsitepassphrase authformsize authformusername authgroupfile authldapauthoritative "
    "authldapauthorizeprefix authldapbindauthoritative authldapbinddn authldapbindpassword "
    "authldapcharsetconfig authldapcompareasuser authldapcomparednonserver authldapdereferencealiases "
    "authldapenabled authldapfrontpagehack authldapgroupattribute authldapgroupattributeisdn "
    "authldapinitialbindasuser authldapinitialbindpattern authldapmaxsubgroupdepth authldapremoteuserattribute "
    "authldapremoteuserisdn authldapsearchasuser authldapsubgroupattribute authldapsubgroupclass authldapurl "
    "authmerging authname authncachecontext authncacheenable authncacheprovidefor authncachesocache "
    "authncachetimeout authnprovideralias authnzfcgicheckauthnprovider authnzfcgidefineprovider authtype "
    "authuserfile authzdbdlogintoreferer authzdbdquery authzdbdredirectquery authzdbmtype "
    "authzsendforbiddenonfailure balancergrowth balancerinherit balancermember balancerpersist bindaddress "
    "browsermatch browsermatchnocase bs2000account bufferedlogs buffersize cachedefaultexpire "
    "cachedetailheader cachedirlength cachedirlevels cachedisable cacheenable cacheexpirycheck cachefile "
    "cacheforcecompletion cachegcclean cachegcdaily cachegcinterval cachegcmemusage cachegcunused cacheheader "
    "cacheignorecachecontrol cacheignoreheaders cacheignorenolastmod cacheignorequerystring "
    "cacheignoreurlsessionidentifiers cachekeybaseurl cachelastmodifiedfactor cachelock cachelockmaxage "
    "cachelockpath cachemaxexpire cachemaxfilesize cacheminexpire cacheminfilesize cachenegotiateddocs "
    "cachequickhandler cachereadsize cachereadtime cacheroot cachesize cachesocache cachesocachemaxsize "
    "cachesocachemaxtime cachesocachemintime cachesocachereadsize cachesocachereadtime cachestaleonerror "
    "cachestoreexpired cachestorenostore cachestoreprivate cachetimemargin cgidscripttimeout cgimapextension "
    "cgipassauth cgivar charsetdefault charsetoptions charsetsourceenc checkcaseonly checkspelling "
    "childperuserid chrootdir clearmodulelist contentdigest cookiedomain cookieexpires cookielog cookiename "
    "cookiestyle cookietracking coredumpdirectory customlog dav davdepthinfinity davgenericlockdb davlockdb "
    "davmintimeout dbdexptime dbdinitsql dbdkeep dbdmax dbdmin dbdparams dbdpersist dbdpreparesql dbdriver "
    "defaulticon defaultlanguage defaultruntimedir defaulttype define deflatebuffersize "
    "deflatecompressionlevel deflatefilternote deflateinflatelimitrequestbody deflateinflateratioburst "
    "deflateinflateratiolimit deflatememlevel deflatewindowsize deny directory directorycheckhandler "
    "directoryindex directoryindexredirect directorymatch directoryslash documentroot dtraceprivileges "
    "dumpioinput dumpiooutput else elseif enableexceptionhook enablemmap enablesendfile error errordocument "
    "errorlog errorlogformat example expiresactive expiresbytype expiresdefault extendedstatus extfilterdefine "
    "extfilteroptions fallbackresource fancyindexing fileetag files filesmatch filterchain filterdeclare "
    "filterprotocol filterprovider filtertrace forcelanguagepriority forcetype forensiclog from globallog "
    "gracefulshutdowntimeout group h2direct h2maxsessionstreams h2maxworkeridleseconds h2maxworkers "
    "h2minworkers h2moderntlsonly h2push h2pushdiarysize h2pushpriority h2serializeheaders h2sessionextrafiles "
    "h2streammaxmemsize h2tlscooldownsecs h2tlswarmupsize h2upgrade h2windowsize header headername "
    "heartbeataddress heartbeatlisten heartbeatmaxservers heartbeatstorage hostnamelookups identitycheck "
    "identitychecktimeout if ifdefine ifmodule ifversion imapbase imapdefault imapmenu include includeoptional "
    "indexheadinsert indexignore indexignorereset indexoptions indexorderdefault indexstylesheet inputsed "
    "isapiappendlogtoerrors isapiappendlogtoquery isapicachefile isapifakeasync isapilognotsupported "
    "isapireadaheadbuffer keepalive keepalivetimeout keptbodysize languagepriority ldapcacheentries "
    "ldapcachettl ldapconnectionpoolttl ldapconnectiontimeout ldaplibrarydebug ldapopcacheentries "
    "ldapopcachettl ldapreferralhoplimit ldapreferrals ldapretries ldapretrydelay ldapsharedcachefile "
    "ldapsharedcachesize ldaptimeout ldaptrustedca ldaptrustedcatype ldaptrustedclientcert "
    "ldaptrustedglobalcert ldaptrustedmode ldapverifyservercert limit limitexcept limitinternalrecursion "
    "limitrequestbody limitrequestfields limitrequestfieldsize limitrequestline limitxmlrequestbody listen "
    "listenbacklog listencoresbucketsratio loadfile loadmodule location locationmatch lockfile logformat "
    "logiotrackttfb loglevel logmessage luaauthzprovider luacodecache luahookaccesschecker luahookauthchecker "
    "luahookcheckuserid luahookfixups luahookinsertfilter luahooklog luahookmaptostorage luahooktranslatename "
    "luahooktypechecker luainherit luainputfilter luamaphandler luaoutputfilter luapackagecpath luapackagepath "
    "luaquickhandler luaroot luascope macro maxclients maxconnectionsperchild maxkeepaliverequests maxmemfree "
    "maxrangeoverlaps maxrangereversals maxranges maxrequestsperchild maxrequestsperthread maxrequestworkers "
    "maxspareservers maxsparethreads maxthreads maxthreadsperchild mcachemaxobjectcount mcachemaxobjectsize "
    "mcachemaxstreamingbuffer mcacheminobjectsize mcacheremovalalgorithm mcachesize memcacheconnttl "
    "mergetrailers metadir metafiles metasuffix mimemagicfile minspareservers minsparethreads mmapfile "
    "modemstandard modmimeusepathinfo multiviewsmatch mutex namevirtualhost nocache noproxy numservers "
    "nwssltrustedcerts nwsslupgradeable options order outputsed passenv pidfile port privilegesmode protocol "
    "protocolecho protocols protocolshonororder proxy proxyaddheaders proxybadheader proxyblock proxydomain "
    "proxyerroroverride proxyexpressdbmfile proxyexpressdbmtype proxyexpressenable proxyftpdircharset "
    "proxyftpescapewildcards proxyftplistonwildcard proxyhcexpr proxyhctemplate proxyhctpsize proxyhtmlbufsize "
    "proxyhtmlcharsetout proxyhtmldoctype proxyhtmlenable proxyhtmlevents proxyhtmlextended proxyhtmlfixups "
    "proxyhtmlinterp proxyhtmllinks proxyhtmlmeta proxyhtmlstripcomments proxyhtmlurlmap proxyiobuffersize "
    "proxymatch proxymaxforwards proxypass proxypassinherit proxypassinterpolateenv proxypassmatch "
    "proxypassreverse proxypassreversecookiedomain proxypassreversecookiepath proxypreservehost "
    "proxyreceivebuffersize proxyremote proxyremotematch proxyrequests proxyscgiinternalredirect "
    "proxyscgisendfile proxyset proxysourceaddress proxystatus proxytimeout proxyvia qsc qualifyredirecturl "
    "readmename receivebuffersize redirect redirectmatch redirectpermanent redirecttemp refererignore "
    "refererlog reflectorheader remoteipheader remoteipinternalproxy remoteipinternalproxylist "
    "remoteipproxiesheader remoteiptrustedproxy remoteiptrustedproxylist removecharset removeencoding "
    "removehandler removeinputfilter removelanguage removeoutputfilter removetype requestheader "
    "requestreadtimeout require requireall requireany requirenone resourceconfig rewritebase rewritecond "
    "rewriteengine rewritelock rewritelog rewriteloglevel rewritemap rewriteoptions rewriterule rlimitcpu "
    "rlimitmem rlimitnproc satisfy scoreboardfile script scriptalias scriptaliasmatch scriptinterpretersource "
    "scriptlog scriptlogbuffer scriptloglength scriptsock securelisten seerequesttail sendbuffersize "
    "serveradmin serveralias serverlimit servername serverpath serverroot serversignature servertokens "
    "servertype session sessioncookiename sessioncookiename2 sessioncookieremove sessioncryptocipher "
    "sessioncryptodriver sessioncryptopassphrase sessioncryptopassphrasefile sessiondbdcookiename "
    "sessiondbdcookiename2 sessiondbdcookieremove sessiondbddeletelabel sessiondbdinsertlabel "
    "sessiondbdperuser sessiondbdselectlabel sessiondbdupdatelabel sessionenv sessionexclude sessionheader "
    "sessioninclude sessionmaxage setenv setenvif setenvifexpr setenvifnocase sethandler setinputfilter "
    "setoutputfilter singlelisten ssiendtag ssierrormsg ssietag ssilastmodified ssilegacyexprparser "
    "ssistarttag ssitimeformat ssiundefinedecho sslcacertificatefile sslcacertificatepath sslcadnrequestfile "
    "sslcadnrequestpath sslcarevocationcheck sslcarevocationfile sslcarevocationpath sslcertificatechainfile "
    "sslcertificatefile sslcertificatekeyfile sslciphersuite sslcompression sslcryptodevice sslengine sslfips "
    "sslhonorcipherorder sslinsecurerenegotiation sslmutex sslocspdefaultresponder sslocspenable "
    "sslocspoverrideresponder sslocspproxyurl sslocsprespondertimeout sslocspresponsemaxage "
    "sslocspresponsetimeskew sslocspuserequestnonce sslopensslconfcmd ssloptions sslpassphrasedialog "
    "sslprotocol sslproxycacertificatefile sslproxycacertificatepath sslproxycarevocationcheck "
    "sslproxycarevocationfile sslproxycarevocationpath sslproxycheckpeercn sslproxycheckpeerexpire "
    "sslproxycheckpeername sslproxyciphersuite sslproxyengine sslproxymachinecertificatechainfile "
    "sslproxymachinecertificatefile sslproxymachinecertificatepath sslproxyprotocol sslproxyverify "
    "sslproxyverifydepth sslrandomseed sslrenegbuffersize sslrequire sslrequiressl sslsessioncache "
    "sslsessioncachetimeout sslsessionticketkeyfile sslsessiontickets sslsrpunknownuserseed sslsrpverifierfile "
    "sslstaplingcache sslstaplingerrorcachetimeout sslstaplingfaketrylater sslstaplingforceurl "
    "sslstaplingrespondertimeout sslstaplingresponsemaxage sslstaplingresponsetimeskew "
    "sslstaplingreturnrespondererrors sslstaplingstandardcachetimeout sslstrictsnivhostcheck sslusername "
    "sslusestapling sslverifyclient sslverifydepth startservers startthreads substitute "
    "substituteinheritbefore substitutemaxlinelength suexec suexecusergroup threadlimit threadsperchild "
    "threadstacksize timeout traceenable transferlog typesconfig undefine undefmacro unsetenv use "
    "usecanonicalname usecanonicalphysicalport user userdir vhostcgimode vhostcgiprivs vhostgroup vhostprivs "
    "vhostsecure vhostuser virtualdocumentroot virtualdocumentrootip virtualhost virtualscriptalias "
    "virtualscriptaliasip watchdoginterval win32disableacceptex xbithack xml2encalias xml2encdefault "
    "xml2startparse",
    "",
// "downgrade-1.0 followsymlinks force-response-1.0 includes indexes inetd nokeepalive none off on "
// "standalone x-compress x-gzip",
    NULL,
};


EDITLEXER lexCONF =
{
    SCLEX_CONF, "conf", IDS_LEX_APC_CFG, L"Apache Config Files", L"conf; cfg; cnf; htaccess; prefs; iface; prop; po; te; \\^Kconfig$; \\^Doxyfile$", L"",
    &KeyWords_CONF, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_CONF_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {SCE_CONF_COMMENT}, IDS_LEX_STR_Comment, L"Comment", L"fore:#648000", L"" },
        { {SCE_CONF_STRING}, IDS_LEX_STR_String, L"String", L"fore:#B000B0", L"" },
        { {SCE_CONF_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF4000", L"" },
        { {SCE_CONF_DIRECTIVE}, IDS_LEX_STR_63203, L"Directive", L"fore:#003CE6", L"" },
        { {SCE_CONF_IP}, IDS_LEX_STR_IpAdr, L"IP Address", L"bold; fore:#FF4000", L"" },
// Not used by lexer  { {SCE_CONF_IDENTIFIER}, L"Identifier", L"", L"" },
// Lexer is buggy     { {SCE_CONF_OPERATOR}, L"Operator", L"", L"" },
// Lexer is buggy     { {SCE_CONF_PARAMETER}, L"Runtime Directive Parameter", L"", L"" },
// Lexer is buggy     { {SCE_CONF_EXTENSION}, L"Extension", L"", L"" },
        EDITLEXER_SENTINEL
    }
};

