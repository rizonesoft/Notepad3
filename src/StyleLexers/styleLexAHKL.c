#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_AHKL = {
// Directives
    "#AllowSameLineComments #ClipboardTimeout #CommentFlag #ErrorStdOut #EscapeChar #HotkeyInterval "
    "#HotkeyModifierTimeout #Hotstring #If #IfTimeout #IfWinActive #IfWinExist #IfWinNotActive #IfWinNotExist "
    "#Include #IncludeAgain #InputLevel #InstallKeybdHook #InstallMouseHook #KeyHistory #LTrim "
    "#MaxHotkeysPerInterval #MaxMem #MaxThreads #MaxThreadsBuffer #MaxThreadsPerHotkey #MenuMaskKey #NoEnv "
    "#NoTrayIcon #Persistent #SingleInstance #UseHook #Warn #WinActivateForce",
// Commands
    "AutoTrim BlockInput Click ClipWait Control ControlClick ControlFocus ControlGet ControlGetFocus "
    "ControlGetPos ControlGetText ControlMove ControlSend ControlSendRaw ControlSetText CoordMode Critical "
    "DetectHiddenText DetectHiddenWindows Drive DriveGet DriveSpaceFree Edit EnvAdd EnvDiv EnvGet EnvMult "
    "EnvSet EnvSub EnvUpdate FileAppend FileCopy FileCopyDir FileCreateDir FileCreateShortcut FileDelete "
    "FileEncoding FileGetAttrib FileGetShortcut FileGetSize FileGetTime FileGetVersion FileInstall FileMove "
    "FileMoveDir FileRead FileReadLine FileRecycle FileRecycleEmpty FileRemoveDir FileSelectFile "
    "FileSelectFolder FileSetAttrib FileSetTime FormatTime GetKeyState GroupActivate GroupAdd GroupClose "
    "GroupDeactivate Gui GuiControl GuiControlGet Hotkey IfEqual IfExist IfGreater IfGreaterOrEqual IfInString "
    "IfLess IfLessOrEqual IfMsgBox IfNotEqual IfNotExist IfNotInString IfWinActive IfWinExist IfWinNotActive "
    "IfWinNotExist ImageSearch IniDelete IniRead IniWrite Input InputBox KeyHistory KeyWait ListHotkeys "
    "ListLines ListVars Menu MouseClick MouseClickDrag MouseGetPos MouseMove MsgBox OutputDebug PixelGetColor "
    "PixelSearch PostMessage Process Progress Random RegDelete RegRead RegWrite Reload Run RunAs RunWait Send "
    "SendEvent SendInput SendLevel SendMessage SendMode SendPlay SendRaw SetBatchLines SetCapslockState "
    "SetControlDelay SetDefaultMouseSpeed SetEnv SetFormat SetKeyDelay SetMouseDelay SetNumlockState SetRegView "
    "SetScrollLockState SetStoreCapslockMode SetTitleMatchMode SetWinDelay SetWorkingDir Shutdown Sort "
    "SoundBeep SoundGet SoundGetWaveVolume SoundPlay SoundSet SoundSetWaveVolume SplashImage SplashTextOff "
    "SplashTextOn SplitPath StatusBarGetText StatusBarWait StringCaseSense StringGetPos StringLeft StringLen "
    "StringLower StringMid StringReplace StringRight StringSplit StringTrimLeft StringTrimRight StringUpper "
    "SysGet Thread Throw ToolTip Transform TrayTip URLDownloadToFile WinActivate WinActivateBottom WinClose "
    "WinGet WinGetActiveStats WinGetActiveTitle WinGetClass WinGetPos WinGetText WinGetTitle WinHide WinKill "
    "WinMaximize WinMenuSelectItem WinMinimize WinMinimizeAll WinMinimizeAllUndo WinMove WinRestore WinSet "
    "WinSetTitle WinShow WinWait WinWaitActive WinWaitClose WinWaitNotActive",
// Command Parameters
    "ACos ASin ATan Abort AboveNormal Abs ActiveX Add All Alnum Alpha AltSubmit AltTab AltTabAndMenu AltTabMenu "
    "AltTabMenuDismiss AlwaysOnTop Asc AutoSize Background BackgroundTrans BelowNormal Between BitAnd BitNot "
    "BitOr BitShiftLeft BitShiftRight BitXOr Border Bottom Button Buttons Cancel Capacity Caption Caret Ceil "
    "Center Check Check3 Checkbox Checked CheckedGray Choose ChooseString Chr Click Close Color ComboBox "
    "Contains ControlList Cos Count Custom DDL DPIScale Date DateTime Days Default Delete DeleteAll Delimiter "
    "Deref Destroy Digit Disable Disabled DropDownList Eject Enable Enabled Error ExStyle Exist Exp Expand "
    "FileSystem First Flash Float Floor Focus Font FromCodePage Grid Group GroupBox GuiClose GuiContextMenu "
    "GuiDropFiles GuiEscape GuiSize HKCC HKCR HKCU HKEY_CLASSES_ROOT HKEY_CURRENT_CONFIG HKEY_CURRENT_USER "
    "HKEY_LOCAL_MACHINE HKEY_USERS HKLM HKU HScroll Hdr Hidden Hide High Hours ID IDLast Icon IconSmall Ignore "
    "ImageList In Integer Interrupt Is Join LTrim Label LastFound LastFoundExist Left Limit Lines Link List "
    "ListBox ListView Ln Lock Log Logoff Low Lower Lowercase MainWindow Margin MaxSize Maximize MaximizeBox "
    "Menu MinMax MinSize Minimize MinimizeBox Minutes Mod MonthCal Mouse Move Multi NA No NoActivate NoDefault "
    "NoHide NoIcon NoMainWindow NoSort NoSortHdr NoStandard NoTab NoTimers Normal Number Off Ok On "
    "OnClipboardChange OwnDialogs Owner Parse Password Pic Picture Pixel Pos Pow Priority ProcessName "
    "REG_BINARY REG_DWORD REG_EXPAND_SZ REG_MULTI_SZ REG_SZ RGB RTrim Radio Range Read ReadOnly Realtime Redraw "
    "Region Relative Rename Report Resize Restore Retry Right Round Screen Seconds Section Serial SetLabel "
    "ShiftAltTab Show Sin Single Slider SortDesc Sqrt Standard Status StatusBar StatusCD Style Submit SysMenu "
    "Tab Tab2 TabStop Tan Text Theme Tile Time Tip ToCodePage ToggleCheck ToggleEnable ToolWindow Top Topmost "
    "TransColor Transparent Tray TreeView TryAgain Type UnCheck Unicode Unlock UpDown Upper Uppercase "
    "UseErrorLevel VScroll Vis VisFirst Visible Wait WaitClose WantCtrlA WantF2 WantReturn Wrap Xdigit Yes "
    "ahk_class ahk_exe ahk_group ahk_id ahk_pid and bold class extends italic new norm not or strike underline "
    "xm xp xs ym yp ys {AltDown} {AltUp} {Blind} {Click} {CtrlDown} {CtrlUp} {LWinDown} {LWinUp} {RWinDown} "
    "{RWinUp} {Raw} {ShiftDown} {ShiftUp}",
// Control Flow
    "Break ByRef Catch Continue Else Exit ExitApp Finally For GoSub Goto If Loop OnExit Pause Return SetTimer "
    "Sleep Suspend Try Until While global local static",
// Built-in Functions
    "ACos ASin ATan Abs Array Asc Ceil Chr ComObjActive ComObjArray ComObjConnect ComObjCreate ComObjEnwrap "
    "ComObjError ComObjFlags ComObjGet ComObjMissing ComObjParameter ComObjQuery ComObjType ComObjUnwrap "
    "ComObjValue Cos DllCall Exp FileExist FileOpen Floor Format Func GetKeyName GetKeySC GetKeyState GetKeyVK "
    "IL_Add IL_Create IL_Destroy InStr IsByRef IsFunc IsLabel IsObject LTrim LV_Add LV_Delete LV_DeleteCol "
    "LV_GetCount LV_GetNext LV_GetText LV_Insert LV_InsertCol LV_Modify LV_ModifyCol LV_SetImageList Ln Log Mod "
    "NumGet NumPut ObjAddRef ObjClone ObjGetAddress ObjGetCapacity ObjHasKey ObjInsert ObjMaxIndex ObjMinIndex "
    "ObjNewEnum ObjRelease ObjRemove ObjSetCapacity Object OnMessage Ord RTrim RegExMatch RegExReplace "
    "RegisterCallback Round SB_SetIcon SB_SetParts SB_SetText Sin Sqrt StrGet StrLen StrPut StrReplace StrSplit "
    "SubStr TV_Add TV_Delete TV_Get TV_GetChild TV_GetCount TV_GetNext TV_GetParent TV_GetPrev TV_GetSelection "
    "TV_GetText TV_Modify TV_SetImageList Tan Trim VarSetCapacity WinActive WinExist _AddRef _Clone _GetAddress "
    "_GetCapacity _HasKey _Insert _MaxIndex _MinIndex _NewEnum _Release _Remove _SetCapacity __Call __Delete "
    "__Get __New __Set",
// Built-in Variables
    "A_AhkPath A_AhkVersion A_AppData A_AppDataCommon A_AutoTrim A_BatchLines A_CaretX A_CaretY A_ComputerName "
    "A_ControlDelay A_Cursor A_DD A_DDD A_DDDD A_DefaultMouseSpeed A_Desktop A_DesktopCommon A_DetectHiddenText "
    "A_DetectHiddenWindows A_EndChar A_EventInfo A_ExitReason A_FileEncoding A_FormatFloat A_FormatInteger "
    "A_Gui A_GuiControl A_GuiControlEvent A_GuiEvent A_GuiHeight A_GuiWidth A_GuiX A_GuiY A_Hour A_IPAddress1 "
    "A_IPAddress2 A_IPAddress3 A_IPAddress4 A_IconFile A_IconHidden A_IconNumber A_IconTip A_Index A_Is64bitOS "
    "A_IsAdmin A_IsCompiled A_IsCritical A_IsPaused A_IsSuspended A_IsUnicode A_KeyDelay A_Language A_LastError "
    "A_LineFile A_LineNumber A_LoopField A_LoopFileAttrib A_LoopFileDir A_LoopFileExt A_LoopFileFullPath "
    "A_LoopFileLongPath A_LoopFileName A_LoopFileShortName A_LoopFileShortPath A_LoopFileSize A_LoopFileSizeKB "
    "A_LoopFileSizeMB A_LoopFileTimeAccessed A_LoopFileTimeCreated A_LoopFileTimeModified A_LoopReadLine "
    "A_LoopRegKey A_LoopRegName A_LoopRegSubkey A_LoopRegTimeModified A_LoopRegType A_MDAY A_MM A_MMM A_MMMM "
    "A_MSec A_Min A_Mon A_MouseDelay A_MyDocuments A_Now A_NowUTC A_NumBatchLines A_OSType A_OSVersion "
    "A_PriorHotkey A_PriorKey A_ProgramFiles A_Programs A_ProgramsCommon A_PtrSize A_RegView A_ScreenDPI "
    "A_ScreenHeight A_ScreenWidth A_ScriptDir A_ScriptFullPath A_ScriptHwnd A_ScriptName A_Sec A_Space "
    "A_StartMenu A_StartMenuCommon A_Startup A_StartupCommon A_StringCaseSense A_Tab A_Temp A_ThisFunc "
    "A_ThisHotkey A_ThisLabel A_ThisMenu A_ThisMenuItem A_ThisMenuItemPos A_TickCount A_TimeIdle "
    "A_TimeIdlePhysical A_TimeSincePriorHotkey A_TimeSinceThisHotkey A_TitleMatchMode A_TitleMatchModeSpeed "
    "A_UserName A_WDay A_WinDelay A_WinDir A_WorkingDir A_YDay A_YEAR A_YWeek A_YYYY Clipboard ClipboardAll "
    "ComSpec ErrorLevel ProgramFiles false true",
// Keyboard & Mouse Keys
    "Alt AltDown AltUp AppsKey BS BackSpace Browser_Back Browser_Favorites Browser_Forward Browser_Home "
    "Browser_Refresh Browser_Search Browser_Stop CapsLock Control Ctrl CtrlBreak CtrlDown CtrlUp Del Delete "
    "Down End Enter Esc Escape F1 F10 F11 F12 F13 F14 F15 F16 F17 F18 F19 F2 F20 F21 F22 F23 F24 F3 F4 F5 F6 F7 "
    "F8 F9 Home Ins Insert Joy1 Joy10 Joy11 Joy12 Joy13 Joy14 Joy15 Joy16 Joy17 Joy18 Joy19 Joy2 Joy20 Joy21 "
    "Joy22 Joy23 Joy24 Joy25 Joy26 Joy27 Joy28 Joy29 Joy3 Joy30 Joy31 Joy32 Joy4 Joy5 Joy6 Joy7 Joy8 Joy9 "
    "JoyAxes JoyButtons JoyInfo JoyName JoyPOV JoyR JoyU JoyV JoyX JoyY JoyZ LAlt LButton LControl LCtrl LShift "
    "LWin LWinDown LWinUp Launch_App1 Launch_App2 Launch_Mail Launch_Media Left MButton Media_Next "
    "Media_Play_Pause Media_Prev Media_Stop NumLock Numpad0 Numpad1 Numpad2 Numpad3 Numpad4 Numpad5 Numpad6 "
    "Numpad7 Numpad8 Numpad9 NumpadAdd NumpadClear NumpadDel NumpadDiv NumpadDot NumpadDown NumpadEnd "
    "NumpadEnter NumpadHome NumpadIns NumpadLeft NumpadMult NumpadPgdn NumpadPgup NumpadRight NumpadSub "
    "NumpadUp PGDN PGUP Pause PrintScreen RAlt RButton RControl RCtrl RShift RWin RWinDown RWinUp Right "
    "ScrollLock Shift ShiftDown ShiftUp Space Tab Up Volume_Down Volume_Mute Volume_Up WheelDown WheelUp "
    "XButton1 XButton2",
// User Defined 1
    NULL,
// User Defined 2
    NULL,
};


EDITLEXER lexAHKL = {
    SCLEX_AHKL, "AHKL", IDS_LEX_AHKL, L"AutoHotkey_L Script", L"ahkl; ahk; ia; scriptlet", L"",
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
        EDITLEXER_SENTINEL
    }
};
