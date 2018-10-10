#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static __int64 LexFunction(LexFunctionType type, int value)
{
  LEX_FUNCTION_BODY(type, value);
  return 0LL;
};

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_AHKL = {
// Directives
"#allowsamelinecomments #clipboardtimeout #commentflag #errorstdout #escapechar #hotkeyinterval "
"#hotkeymodifiertimeout #hotstring #if #iftimeout #ifwinactive #ifwinexist #include #includeagain "
"#installkeybdhook #installmousehook #keyhistory #ltrim #maxhotkeysperinterval #maxmem #maxthreads "
"#maxthreadsbuffer #maxthreadsperhotkey #menumaskkey #noenv #notrayicon #persistent #singleinstance "
"#usehook #warn #winactivateforce",
// Commands
"autotrim blockinput clipwait control controlclick controlfocus controlget controlgetfocus "
"controlgetpos controlgettext controlmove controlsend controlsendraw controlsettext coordmode "
"critical detecthiddentext detecthiddenwindows drive driveget drivespacefree edit endrepeat envadd "
"envdiv envget envmult envset envsub envupdate fileappend filecopy filecopydir filecreatedir "
"filecreateshortcut filedelete filegetattrib filegetshortcut filegetsize filegettime filegetversion "
"fileinstall filemove filemovedir fileread filereadline filerecycle filerecycleempty fileremovedir "
"fileselectfile fileselectfolder filesetattrib filesettime formattime getkeystate groupactivate "
"groupadd groupclose groupdeactivate gui guicontrol guicontrolget hideautoitwin hotkey if ifequal "
"ifexist ifgreater ifgreaterorequal ifinstring ifless iflessorequal ifmsgbox ifnotequal ifnotexist "
"ifnotinstring ifwinactive ifwinexist ifwinnotactive ifwinnotexist imagesearch inidelete iniread "
"iniwrite input inputbox keyhistory keywait listhotkeys listlines listvars menu mouseclick "
"mouseclickdrag mousegetpos mousemove msgbox outputdebug pixelgetcolor pixelsearch postmessage "
"process progress random regdelete regread regwrite reload run runas runwait send sendevent "
"sendinput sendmessage sendmode sendplay sendraw setbatchlines setcapslockstate setcontroldelay "
"setdefaultmousespeed setenv setformat setkeydelay setmousedelay setnumlockstate setscrolllockstate "
"setstorecapslockmode settitlematchmode setwindelay setworkingdir shutdown sort soundbeep soundget "
"soundgetwavevolume soundplay soundset soundsetwavevolume splashimage splashtextoff splashtexton "
"splitpath statusbargettext statusbarwait stringcasesense stringgetpos stringleft stringlen "
"stringlower stringmid stringreplace stringright stringsplit stringtrimleft stringtrimright "
"stringupper sysget thread tooltip transform traytip urldownloadtofile winactivate winactivatebottom "
"winclose winget wingetactivestats wingetactivetitle wingetclass wingetpos wingettext wingettitle "
"winhide winkill winmaximize winmenuselectitem winminimize winminimizeall winminimizeallundo winmove "
"winrestore winset winsettitle winshow winwait winwaitactive winwaitclose winwaitnotactive "
"fileencoding",
// Command Parameters
"ltrim rtrim join ahk_id ahk_pid ahk_class ahk_group processname minmax controllist statuscd "
"filesystem setlabel alwaysontop mainwindow nomainwindow useerrorlevel altsubmit hscroll vscroll "
"imagelist wantctrla wantf2 vis visfirst wantreturn backgroundtrans minimizebox maximizebox "
"sysmenu toolwindow exstyle check3 checkedgray readonly notab lastfound lastfoundexist alttab "
"shiftalttab alttabmenu alttabandmenu alttabmenudismiss controllisthwnd hwnd deref pow bitnot "
"bitand bitor bitxor bitshiftleft bitshiftright sendandmouse mousemove mousemoveoff "
"hkey_local_machine hkey_users hkey_current_user hkey_classes_root hkey_current_config hklm hku "
"hkcu hkcr hkcc reg_sz reg_expand_sz reg_multi_sz reg_dword reg_qword reg_binary reg_link "
"reg_resource_list reg_full_resource_descriptor caret reg_resource_requirements_list "
"reg_dword_big_endian regex pixel mouse screen relative rgb low belownormal normal abovenormal "
"high realtime between contains in is integer float number digit xdigit alpha upper lower alnum "
"time date not or and topmost top bottom transparent transcolor redraw region id idlast count "
"list capacity eject lock unlock label serial type status seconds minutes hours days read parse "
"logoff close error single shutdown menu exit reload tray add rename check uncheck togglecheck "
"enable disable toggleenable default nodefault standard nostandard color delete deleteall icon "
"noicon tip click show edit progress hotkey text picture pic groupbox button checkbox radio "
"dropdownlist ddl combobox statusbar treeview listbox listview datetime monthcal updown slider "
"tab tab2 iconsmall tile report sortdesc nosort nosorthdr grid hdr autosize range xm ym ys xs xp "
"yp font resize owner submit nohide minimize maximize restore noactivate na cancel destroy "
"center margin owndialogs guiescape guiclose guisize guicontextmenu guidropfiles tabstop section "
"wrap border top bottom buttons expand first lines number uppercase lowercase limit password "
"multi group background bold italic strike underline norm theme caption delimiter flash style "
"checked password hidden left right center section move focus hide choose choosestring text pos "
"enabled disabled visible notimers interrupt priority waitclose unicode tocodepage fromcodepage "
"yes no ok cancel abort retry ignore force on off all send wanttab monitorcount monitorprimary "
"monitorname monitorworkarea pid base useunsetlocal useunsetglobal localsameasglobal str astr wstr "
"int64 int short char uint64 uint ushort uchar float double int64p intp shortp charp uint64p uintp "
"ushortp ucharp floatp doublep ptr",
// Control Flow
"break continue else exit exitapp gosub goto loop onexit pause repeat return settimer sleep "
"suspend static global local byref while until for",
// Built-in Functions
"abs acos asc asin atan ceil chr cos dllcall exp fileexist floor getkeystate numget numput "
"registercallback il_add il_create il_destroy instr islabel isfunc ln log lv_add lv_delete "
"lv_deletecol lv_getcount lv_getnext lv_gettext lv_insert lv_insertcol lv_modify lv_modifycol "
"lv_setimagelist mod onmessage round regexmatch regexreplace sb_seticon sb_setparts sb_settext "
"sin sqrt strlen substr tan tv_add tv_delete tv_getchild tv_getcount tv_getnext tv_get tv_getparent "
"tv_getprev tv_getselection tv_gettext tv_modify varsetcapacity winactive winexist trim ltrim rtrim "
"fileopen strget strput object isobject objinsert objremove objminindex objmaxindex objsetcapacity "
"objgetcapacity objgetaddress objnewenum objaddref objrelease objclone _insert _remove _minindex "
"_maxindex _setcapacity _getcapacity _getaddress _newenum _addref _release _clone comobjcreate "
"comobjget comobjconnect comobjerror comobjactive comobjenwrap comobjunwrap comobjparameter "
"comobjmissing comobjtype comobjvalue comobjarray",
// Built-in Variables
"a_ahkpath a_ahkversion a_appdata a_appdatacommon a_autotrim a_batchlines a_caretx a_carety "
"a_computername a_controldelay a_cursor a_dd a_ddd a_dddd a_defaultmousespeed a_desktop "
"a_desktopcommon a_detecthiddentext a_detecthiddenwindows a_endchar a_eventinfo a_exitreason "
"a_formatfloat a_formatinteger a_gui a_guievent a_guicontrol a_guicontrolevent a_guiheight "
"a_guiwidth a_guix a_guiy a_hour a_iconfile a_iconhidden a_iconnumber a_icontip a_index a_ipaddress1 "
"a_ipaddress2 a_ipaddress3 a_ipaddress4 a_isadmin a_iscompiled a_issuspended a_keydelay a_language "
"a_lasterror a_linefile a_linenumber a_loopfield a_loopfileattrib a_loopfiledir a_loopfileext "
"a_loopfilefullpath a_loopfilelongpath a_loopfilename a_loopfileshortname a_loopfileshortpath "
"a_loopfilesize a_loopfilesizekb a_loopfilesizemb a_loopfiletimeaccessed a_loopfiletimecreated "
"a_loopfiletimemodified a_loopreadline a_loopregkey a_loopregname a_loopregsubkey "
"a_loopregtimemodified a_loopregtype a_mday a_min a_mm a_mmm a_mmmm a_mon a_mousedelay a_msec "
"a_mydocuments a_now a_nowutc a_numbatchlines a_ostype a_osversion a_priorhotkey a_programfiles "
"a_programs a_programscommon a_screenheight a_screenwidth a_scriptdir a_scriptfullpath a_scriptname "
"a_sec a_space a_startmenu a_startmenucommon a_startup a_startupcommon a_stringcasesense a_tab a_temp "
"a_thishotkey a_thismenu a_thismenuitem a_thismenuitempos a_tickcount a_timeidle a_timeidlephysical "
"a_timesincepriorhotkey a_timesincethishotkey a_titlematchmode a_titlematchmodespeed a_username "
"a_wday a_windelay a_windir a_workingdir a_yday a_year a_yweek a_yyyy clipboard clipboardall comspec "
"programfiles a_thisfunc a_thislabel a_ispaused a_iscritical a_isunicode a_ptrsize errorlevel "
"true false",
// Keyboard & Mouse Keys
"shift lshift rshift alt lalt ralt control lcontrol rcontrol ctrl lctrl rctrl lwin rwin appskey "
"altdown altup shiftdown shiftup ctrldown ctrlup lwindown lwinup rwindown rwinup lbutton rbutton "
"mbutton wheelup wheeldown xbutton1 xbutton2 joy1 joy2 joy3 joy4 joy5 joy6 joy7 joy8 joy9 joy10 joy11 "
"joy12 joy13 joy14 joy15 joy16 joy17 joy18 joy19 joy20 joy21 joy22 joy23 joy24 joy25 joy26 joy27 "
"joy28 joy29 joy30 joy31 joy32 joyx joyy joyz joyr joyu joyv joypov joyname joybuttons joyaxes "
"joyinfo space tab enter escape esc backspace bs delete del insert ins pgup pgdn home end up down "
"left right printscreen ctrlbreak pause scrolllock capslock numlock numpad0 numpad1 numpad2 numpad3 "
"numpad4 numpad5 numpad6 numpad7 numpad8 numpad9 numpadmult numpadadd numpadsub numpaddiv numpaddot "
"numpaddel numpadins numpadclear numpadup numpaddown numpadleft numpadright numpadhome numpadend "
"numpadpgup numpadpgdn numpadenter f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 f16 f17 f18 f19 "
"f20 f21 f22 f23 f24 browser_back browser_forward browser_refresh browser_stop browser_search "
"browser_favorites browser_home volume_mute volume_down volume_up media_next media_prev media_stop "
"media_play_pause launch_mail launch_media launch_app1 launch_app2 blind click raw wheelleft "
"wheelright",
// User Defined 1
"",
// User Defined 2
"" };


EDITLEXER lexAHKL = { 
SCLEX_AHKL, IDS_LEX_AHKL, L"AutoHotkey_L Script", L"ahkl; ahk; ia; scriptlet", L"", 
&LexFunction, // static
&KeyWords_AHKL, {
    { {STYLE_DEFAULT}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ {SCE_AHK_NEUTRAL}, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { {SCE_AHKL_IDENTIFIER}, IDS_LEX_STR_63129, L"Identifier", L"", L"" },
    { {MULTI_STYLE(SCE_AHKL_COMMENTDOC,SCE_AHKL_COMMENTLINE,SCE_AHKL_COMMENTBLOCK,SCE_AHKL_COMMENTKEYWORD)}, IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { {MULTI_STYLE(SCE_AHKL_STRING,SCE_AHKL_STRINGOPTS,SCE_AHKL_STRINGBLOCK,SCE_AHKL_STRINGCOMMENT)}, IDS_LEX_STR_63131, L"String", L"fore:#404040", L"" },
    { {SCE_AHKL_LABEL}, IDS_LEX_STR_63235, L"Label", L"fore:#0000DD", L"" },
    { {SCE_AHKL_HOTKEY}, IDS_LEX_STR_63349, L"HotKey", L"fore:#00AADD", L"" },
    { {SCE_AHKL_HOTSTRING}, IDS_LEX_STR_63350, L"HotString", L"fore:#00BBBB", L"" },
    { {SCE_AHKL_HOTSTRINGOPT}, IDS_LEX_STR_63351, L"KeyHotstringOption", L"fore:#990099", L"" },
    { {SCE_AHKL_HEXNUMBER}, IDS_LEX_STR_63287, L"Hex Number", L"fore:#880088", L"" },
    { {SCE_AHKL_DECNUMBER}, IDS_LEX_STR_63130, L"Number", L"fore:#FF9000", L"" },
    { {SCE_AHKL_VAR}, IDS_LEX_STR_63249, L"Variable", L"fore:#FF9000", L"" },
    { {SCE_AHKL_VARREF}, IDS_LEX_STR_63309, L"Variable Dereferencing", L"fore:#990055", L"" },
    { {SCE_AHKL_OBJECT}, IDS_LEX_STR_63347, L"Object", L"fore:#008888", L"" },
    { {SCE_AHKL_USERFUNCTION}, IDS_LEX_STR_63305, L"User-Defined Function", L"fore:#0000DD", L"" },
    { {SCE_AHKL_DIRECTIVE}, IDS_LEX_STR_63203, L"Directive", L"fore:#4A0000; italic", L"" },
    { {SCE_AHKL_COMMAND}, IDS_LEX_STR_63236, L"Command", L"fore:#0000DD; bold", L"" },
    { {SCE_AHKL_PARAM}, IDS_LEX_STR_63281, L"Parameter", L"fore:#0085DD", L"" },   
    { {SCE_AHKL_CONTROLFLOW}, IDS_LEX_STR_63310, L"Flow of Control", L"fore:#0000DD;", L"" },
    { {SCE_AHKL_BUILTINFUNCTION}, IDS_LEX_STR_63277, L"Function", L"fore:#DD00DD", L"" },
    { {SCE_AHKL_BUILTINVAR}, IDS_LEX_STR_63312, L"Built-In Variables", L"fore:#EE3010; bold", L"" },
    { {SCE_AHKL_KEY}, IDS_LEX_STR_63348, L"Key", L"fore:#A2A2A2", L"" },
    //{ {SCE_AHKL_USERDEFINED}, IDS_LEX_STR_63106, L"User Defined", L"fore:#800020", L"" },
    //{ {SCE_AHKL_USERDEFINED}, IDS_LEX_STR_63106, L"User Defined", L"fore:#800020", L"" },
    { {SCE_AHKL_ESCAPESEQ}, IDS_LEX_STR_63306, L"Escape", L"fore:#660000; italic", L"" },
    { {SCE_AHKL_ERROR}, IDS_LEX_STR_63261, L"Error", L"back:#FF0000", L"" },
    EDITLEXER_SENTINEL } };
