#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_AHKL = {
// Directives
"#AllowSameLineComments #ClipboardTimeout #CommentFlag #ErrorStdOut #EscapeChar #HotkeyInterval "
"#HotkeyModifierTimeout #Hotstring #If #IfTimeout #IfWinActive #IfWinExist #IfWinNotActive "
"#IfWinNotExist #Include #IncludeAgain #InputLevel #InstallKeybdHook #InstallMouseHook #KeyHistory "
"#LTrim #MaxHotkeysPerInterval #MaxMem #MaxThreads #MaxThreadsBuffer #MaxThreadsPerHotkey #MenuMaskKey "
"#NoEnv #NoTrayIcon #Persistent #SingleInstance #UseHook #Warn #WinActivateForce",
// Commands
"AutoTrim BlockInput Click ClipWait Control ControlClick ControlFocus ControlGet ControlGetFocus "
"ControlGetPos ControlGetText ControlMove ControlSend ControlSendRaw ControlSetText CoordMode Critical "
"DetectHiddenText DetectHiddenWindows Drive DriveGet DriveSpaceFree Edit EnvAdd EnvDiv EnvGet EnvMult "
"EnvSet EnvSub EnvUpdate FileAppend FileCopy FileCopyDir FileCreateDir FileCreateShortcut FileDelete "
"FileEncoding FileGetAttrib FileGetShortcut FileGetSize FileGetTime FileGetVersion FileInstall FileMove "
"FileMoveDir FileRead FileReadLine FileRecycle FileRecycleEmpty FileRemoveDir FileSelectFile "
"FileSelectFolder FileSetAttrib FileSetTime FormatTime GetKeyState GroupActivate GroupAdd GroupClose "
"GroupDeactivate Gui GuiControl GuiControlGet Hotkey IfEqual IfExist IfGreater IfGreaterOrEqual "
"IfInString IfLess IfLessOrEqual IfMsgBox IfNotEqual IfNotExist IfNotInString IfWinActive IfWinExist "
"IfWinNotActive IfWinNotExist ImageSearch IniDelete IniRead IniWrite Input InputBox KeyHistory KeyWait "
"ListHotkeys ListLines ListVars Menu MouseClick MouseClickDrag MouseGetPos MouseMove MsgBox OutputDebug "
"PixelGetColor PixelSearch PostMessage Process Progress Random RegDelete RegRead RegWrite Reload Run "
"RunAs RunWait Send SendEvent SendInput SendLevel SendMessage SendMode SendPlay SendRaw SetBatchLines "
"SetCapslockState SetControlDelay SetDefaultMouseSpeed SetEnv SetFormat SetKeyDelay SetMouseDelay "
"SetNumlockState SetRegView SetScrollLockState SetStoreCapslockMode SetTitleMatchMode SetWinDelay "
"SetWorkingDir Shutdown Sort SoundBeep SoundGet SoundGetWaveVolume SoundPlay SoundSet SoundSetWaveVolume "
"SplashImage SplashTextOff SplashTextOn SplitPath StatusBarGetText StatusBarWait StringCaseSense "
"StringGetPos StringLeft StringLen StringLower StringMid StringReplace StringRight StringSplit "
"tringTrimLeft StringTrimRight StringUpper SysGet Thread Throw ToolTip Transform TrayTip "
"URLDownloadToFile WinActivate WinActivateBottom WinClose WinGet WinGetActiveStats WinGetActiveTitle "
"WinGetClass WinGetPos WinGetText WinGetTitle WinHide WinKill WinMaximize WinMenuSelectItem WinMinimize "
"WinMinimizeAll WinMinimizeAllUndo WinMove WinRestore WinSet WinSetTitle WinShow WinWait WinWaitActive "
"WinWaitClose WinWaitNotActive",
// Command Parameters
"Pixel Mouse Screen Relative RGB Caret Menu LTrim RTrim Join Low BelowNormal Normal AboveNormal "
"High Realtime ahk_class ahk_exe ahk_id ahk_group ahk_pid Between Contains In Is Integer Float "
"Number Digit Xdigit Alpha Upper Lower Alnum Time Date not or and AlwaysOnTop Topmost Top Bottom "
"Transparent TransColor Redraw Region ID IDLast ProcessName MinMax ControlList Count List Capacity "
"StatusCD Eject Lock Unlock Label FileSystem Label SetLabel Serial Type Status "
"Seconds Minutes Hours Days Read Parse Logoff Close Error Single Tray Add Rename Check UnCheck "
"ToggleCheck Enable Disable ToggleEnable Default NoDefault Standard NoStandard Color Delete DeleteAll "
"Icon NoIcon Tip Click Show MainWindow NoMainWindow UseErrorLevel Text Picture Pic GroupBox Button "
"Checkbox Radio DropDownList DDL ComboBox ListBox ListView DateTime MonthCal Slider StatusBar Tab Tab2 "
"TreeView UpDown ActiveX Link Custom IconSmall Tile Report SortDesc NoSort NoSortHdr Grid Hdr AutoSize "
"Range xm ym ys xs xp yp Font Resize Owner Submit NoHide Minimize Maximize Restore NoActivate NA Cancel "
"Destroy Center DPIScale Margin MaxSize MinSize OwnDialogs GuiEscape GuiClose GuiSize GuiContextMenu "
"GuiDropFiles OnClipboardChange TabStop Section AltSubmit Wrap HScroll VScroll Border Top Bottom "
"Buttons Expand First ImageList Lines WantCtrlA WantF2 Vis VisFirst Number Uppercase Lowercase Limit "
"Password Multi WantReturn Group Background bold italic strike underline norm BackgroundTrans Theme "
"Caption Delimiter MinimizeBox MaximizeBox SysMenu ToolWindow Flash Style ExStyle Check3 Checked "
"CheckedGray ReadOnly Password Hidden Left Right Center NoTab Section Move Focus Hide Choose "
"ChooseString Text Pos Enabled Disabled Visible LastFound LastFoundExist AltTab ShiftAltTab "
"AltTabMenu AltTabAndMenu AltTabMenuDismiss NoTimers Interrupt Priority WaitClose Wait Exist "
"Close {Blind} {Click} {Raw} {AltDown} {AltUp} {ShiftDown} {ShiftUp} {CtrlDown} {CtrlUp} {LWinDown} "
"{LWinUp} {RWinDown} {RWinUp} Unicode Asc Chr Deref Mod Pow Exp Sqrt Log Ln Round Ceil Floor Abs "
"Sin Cos Tan ASin ACos ATan BitNot BitAnd BitOr BitXOr BitShiftLeft BitShiftRight ToCodePage "
"FromCodePage Yes No Ok Cancel Abort Retry Ignore TryAgain On Off All HKEY_LOCAL_MACHINE HKEY_USERS "
"HKEY_CURRENT_USER HKEY_CLASSES_ROOT HKEY_CURRENT_CONFIG HKLM HKU HKCU HKCR HKCC REG_SZ REG_EXPAND_SZ "
"REG_MULTI_SZ REG_DWORD REG_BINARY class new extends",
// Control Flow
"Break Continue If Else Exit ExitApp OnExit GoSub Goto Loop Pause Return SetTimer Sleep Suspend While "
"Until For Try Catch Finally static global local ByRef",
// Built-in Functions
"Abs ACos Asc ASin ATan Ceil Chr Cos DllCall Exp FileExist Floor Format Func IsByRef IsFunc IsLabel "
"Ln Log FileExist FileOpen GetKeyState GetKeyName GetKeyVK GetKeySC InStr IL_Add IL_Create "
"IL_Destroy LV_Add LV_Delete LV_DeleteCol LV_GetCount LV_GetNext LV_GetText LV_Insert LV_InsertCol "
"LV_Modify LV_ModifyCol LV_SetImageList Trim LTrim RTrim Mod NumGet NumPut OnMessage Ord RegExMatch "
"RegExReplace RegisterCallback Round SB_SetIcon SB_SetParts SB_SetText Sin Sqrt StrGet StrLen "
"StrPut StrReplace StrSplit SubStr Tan TV_Add TV_Delete TV_GetChild TV_GetCount TV_GetNext "
"TV_Get TV_GetParent TV_GetPrev TV_GetSelection TV_GetText TV_Modify TV_SetImageList VarSetCapacity "
"WinActive WinExist Object Array IsObject ObjInsert _Insert ObjRemove _Remove ObjMinIndex _MinIndex "
"ObjMaxIndex _MaxIndex ObjSetCapacity _SetCapacity ObjGetCapacity _GetCapacity ObjGetAddress _GetAddress "
"ObjNewEnum _NewEnum ObjAddRef _AddRef ObjRelease _Release ObjHasKey _HasKey "
"ObjClone _Clone __Get __Set __Call __Delete __New ComObjCreate ComObjGet ComObjConnect ComObjError "
"ComObjActive ComObjEnwrap ComObjUnwrap ComObjParameter ComObjType ComObjValue ComObjMissing "
"ComObjArray ComObjQuery ComObjFlags",
// Built-in Variables
"A_AhkPath A_AhkVersion A_AppData A_AppDataCommon A_AutoTrim A_BatchLines A_CaretX A_CaretY "
"A_ComputerName A_ControlDelay A_Cursor A_DD A_DDD A_DDDD A_DefaultMouseSpeed A_Desktop "
"A_DesktopCommon A_DetectHiddenText A_DetectHiddenWindows A_EndChar A_EventInfo A_ExitReason "
"A_FileEncoding A_FormatFloat A_FormatInteger A_Gui A_GuiEvent A_GuiControl A_GuiControlEvent "
"A_GuiHeight A_GuiWidth A_GuiX A_GuiY A_Hour A_IconFile A_IconHidden A_IconNumber A_IconTip "
"A_Index A_IPAddress1 A_IPAddress2 A_IPAddress3 A_IPAddress4 A_Is64bitOS A_IsAdmin A_IsCompiled "
"A_IsCritical A_IsPaused A_IsSuspended A_IsUnicode A_KeyDelay A_Language A_LastError A_LineFile "
"A_LineNumber A_LoopField A_LoopFileAttrib A_LoopFileDir A_LoopFileExt A_LoopFileFullPath "
"A_LoopFileLongPath A_LoopFileName A_LoopFileShortName A_LoopFileShortPath A_LoopFileSize "
"A_LoopFileSizeKB A_LoopFileSizeMB A_LoopFileTimeAccessed A_LoopFileTimeCreated A_LoopFileTimeModified "
"A_LoopReadLine A_LoopRegKey A_LoopRegName A_LoopRegSubkey A_LoopRegTimeModified A_LoopRegType "
"A_MDAY A_Min A_MM A_MMM A_MMMM A_Mon A_MouseDelay A_MSec A_MyDocuments A_Now A_NowUTC A_NumBatchLines "
"A_OSType A_OSVersion A_PriorHotkey A_PriorKey A_ProgramFiles A_Programs A_ProgramsCommon "
"A_PtrSize A_RegView A_ScreenDPI A_ScreenHeight A_ScreenWidth A_ScriptDir A_ScriptFullPath "
"A_ScriptHwnd A_ScriptName A_Sec A_Space A_StartMenu A_StartMenuCommon A_Startup A_StartupCommon "
"A_StringCaseSense A_Tab A_Temp A_ThisFunc A_ThisHotkey A_ThisLabel A_ThisMenu A_ThisMenuItem "
"A_ThisMenuItemPos A_TickCount A_TimeIdle A_TimeIdlePhysical A_TimeSincePriorHotkey "
"A_TimeSinceThisHotkey A_TitleMatchMode A_TitleMatchModeSpeed A_UserName A_WDay A_WinDelay "
"A_WinDir A_WorkingDir A_YDay A_YEAR A_YWeek A_YYYY Clipboard ClipboardAll ComSpec ErrorLevel "
"ProgramFiles true false",
// Keyboard & Mouse Keys
"Shift LShift RShift Alt LAlt RAlt Control LControl RControl Ctrl LCtrl RCtrl "
"LWin RWin AppsKey AltDown AltUp ShiftDown ShiftUp CtrlDown CtrlUp LWinDown LWinUp RWinDown "
"RWinUp LButton RButton MButton WheelUp WheelDown XButton1 XButton2 Joy1 Joy2 Joy3 Joy4 Joy5 "
"Joy6 Joy7 Joy8 Joy9 Joy10 Joy11 Joy12 Joy13 Joy14 Joy15 Joy16 Joy17 Joy18 Joy19 Joy20 Joy21 "
"Joy22 Joy23 Joy24 Joy25 Joy26 Joy27 Joy28 Joy29 Joy30 Joy31 Joy32 JoyX JoyY JoyZ JoyR JoyU "
"JoyV JoyPOV JoyName JoyButtons JoyAxes JoyInfo Space Tab Enter Escape Esc BackSpace BS Delete "
"Del Insert Ins PGUP PGDN Home End Up Down Left Right PrintScreen CtrlBreak Pause ScrollLock "
"CapsLock NumLock Numpad0 Numpad1 Numpad2 Numpad3 Numpad4 Numpad5 Numpad6 Numpad7 Numpad8 Numpad9 "
"NumpadMult NumpadAdd NumpadSub NumpadDiv NumpadDot NumpadDel NumpadIns NumpadClear NumpadUp "
"NumpadDown NumpadLeft NumpadRight NumpadHome NumpadEnd NumpadPgup NumpadPgdn NumpadEnter "
"F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 F13 F14 F15 F16 F17 F18 F19 F20 F21 F22 F23 F24 "
"Browser_Back Browser_Forward Browser_Refresh Browser_Stop Browser_Search Browser_Favorites "
"Browser_Home Volume_Mute Volume_Down Volume_Up Media_Next Media_Prev Media_Stop Media_Play_Pause "
"Launch_Mail Launch_Media Launch_App1 Launch_App2",
// User Defined 1
"",
// User Defined 2
"" };


EDITLEXER lexAHKL = { 
SCLEX_AHKL, IDS_LEX_AHKL, L"AutoHotkey_L Script", L"ahkl; ahk; ia; scriptlet", L"", 
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
    { {SCE_AHKL_DIRECTIVE}, IDS_LEX_STR_63203, L"Directive", L"italic; fore:#4A0000", L"" },
    { {SCE_AHKL_COMMAND}, IDS_LEX_STR_63236, L"Command", L"bold; fore:#0000DD", L"" },
    { {SCE_AHKL_PARAM}, IDS_LEX_STR_63281, L"Parameter", L"fore:#0085DD", L"" },
    { {SCE_AHKL_CONTROLFLOW}, IDS_LEX_STR_63310, L"Flow of Control", L"fore:#0000DD", L"" },
    { {SCE_AHKL_BUILTINFUNCTION}, IDS_LEX_STR_63277, L"Function", L"fore:#DD00DD", L"" },
    { {SCE_AHKL_BUILTINVAR}, IDS_LEX_STR_63312, L"Built-In Variables", L"bold; fore:#EE3010", L"" },
    { {SCE_AHKL_KEY}, IDS_LEX_STR_63348, L"Key", L"fore:#A2A2A2", L"" },
    //{ {SCE_AHKL_USERDEFINED}, IDS_LEX_STR_63106, L"User Defined", L"fore:#800020", L"" },
    //{ {SCE_AHKL_USERDEFINED}, IDS_LEX_STR_63106, L"User Defined", L"fore:#800020", L"" },
    { {SCE_AHKL_ESCAPESEQ}, IDS_LEX_STR_63306, L"Escape", L"italic; fore:#660000", L"" },
    { {SCE_AHKL_ERROR}, IDS_LEX_STR_63261, L"Error", L"back:#FF0000", L"" },
    EDITLEXER_SENTINEL } };
