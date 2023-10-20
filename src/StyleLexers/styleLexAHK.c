#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_AHK = {

    // Flow of Control
    "Break Catch Continue Critical Else Exit ExitApp Finally For GoSub Goto If Loop "
    "Reg OnError OnExit Pause Reload Return SetBatchLines Sleep Switch Thread "
    "Throw Try Until While",

    // Commands
    "AutoTrim BlockInput Click ClipWait ControlAddItem ControlChoose ControlChooseString "
    "ControlClick ControlDeleteItem ControlEditPaste ControlFocus ControlGet ControlGetFocus "
    "ControlGetPos ControlGetText ControlHide ControlHideDropDown ControlMove ControlSend "
    "ControlSendRaw ControlSendText ControlSetChecked ControlSetEnabled ControlSetExStyle "
    "ControlSetStyle ControlSetTab ControlSetText ControlShow ControlShowDropDown CoordMode "
    "DetectHiddenText DetectHiddenWindows DirCopy DirCreate DirDelete DirMove Download Drive "
    "DriveEject DriveGet DriveLock DriveSetLabel DriveSpaceFree DriveUnlock Edit EnvGet EnvSet "
    "EnvUpdate FileAppend FileCopy FileCopyDir FileCreateDir FileCreateShortcut FileDelete "
    "FileEncoding FileGroupActivate FileInstall FileMove FileMoveDir FileReadLine FileRecycle "
    "FileRecycleEmpty FileRemoveDir FileSelectFile FileSelectFolder FileSetAttrib FileSetTime "
    "FormatTime Global GroupAdd GroupClose GroupDeactivate Gui GuiControl GuiControlGet Hotkey "
    "IfInString IfNotExist IfNotInString ImageSearch IniDelete IniIfExist IniWrite Input "
    "KeyHistory KeyWait ListHotkeys ListLines ListVars Local Menu MenuSelect MouseClick "
    "MouseClickDrag MouseGetPos MouseMove MsgBox OutputDebug PixelGetColor PixelSearch "
    "PostMessage ProcessClose ProcessSetPriority Run RunAs RunWait Send SendEvent SendInput "
    "SendLevel SendMode SendPlay SendRaw SendText SetCapsLockState SetControlDelay "
    "SetDefaultMouseSpeed SetFormat SetKeyDelay SetMouseDelay SetNumLockState SetRegView "
    "SetScrollLockState SetStoreCapsLockMode SetTimer SetTitleMatchMode SetWinDelay SetWorkingDir "
    "Shutdown Sort SoundBeep SoundPlay SoundSetMute SoundSetVolume SplitPath StatusBarGetText "
    "StatusBarWait StringCaseSense StringGetPos StringLeft StringLen StringLower StringMid "
    "StringReplace StringRight StringSplit StringTrimLeft StringTrimRight StringUpper Suspend "
    "SysGet ToolTip Transform TraySetIcon UrlDownloadToFile WinActivate WinActivateBottom "
    "WinClose WinGet WinGetActiveStats WinGetActiveTitle WinGetClass WinGetClientPos WinGetPos "
    "WinHide WinKill WinMaximize WinMenuSelectItem WinMinimize WinMinimizeAll WinMinimizeAllUndo "
    "WinMove WinMoveBottom WinMoveTop WinRedraw WinRestore WinSet WinSetAlwaysOnTop WinSetEnabled "
    "WinSetExStyle WinSetRegion WinSetStyle WinSetTitle WinSetTransColor WinSetTransparent "
    "WinShow WinWait WinWaitActive WinWaitClose WinWaitNotActive",

    // Functions
    "BufferAlloc CallbackCreate CaretGetPos Ceil Chr ComCall ComObjActive" 
    "ComObjArray ComObjConnect ComObjCreate ComObjEnwrap ComObjError ComObjFlags "
    "ComObjGet ComObjMissing ComObjQuery ComObjType ComObjUnwrap ComObjValue ComObject "
    "ControlFindItem ControlGetChecked ControlGetChoice ControlGetClassNN ControlGetCurrentCol "
    "ControlGetCurrentLine ControlGetEnabled ControlGetExStyle ControlGetHwnd "
    "ControlGetLine ControlGetLineCount ControlGetList ControlGetSelected ControlGetStyle "
    "ControlGetTab ControlGetVisible Cos DirExist DirSelect DllCall DriveGetCapacity "
    "DriveGetFileSystem DriveGetLabel DriveGetList DriveGetSerial DriveGetSpaceFree "
    "DriveGetStatus DriveGetStatusCD DriveGetType Exp FileExist FileGetAttrib "
    "FileGetSize FileGetTime FileGetVersion FileRead FileSelect Floor Format "
    "Func GetKeyName GetKeySC GetKeyState GetKeyVK GetMethod HasBase HasMethod "
    "HasProp Hotstring IL_Add IL_Create IL_Destroy InStr IniRead InputBox InputHook "
    "IsByRef IsFunc IsLabel IsObject IsSet LTrim LV_Add LV_Delete LV_DeleteCol LV_GetCount "
    "LV_GetNext LV_GetText LV_Insert LV_InsertCol LV_Modify LV_ModifyCol LV_SetImageList Ln "
    "LoadPicture Log Max Min Mod MonitorGet MonitorGetCount MonitorGetName MonitorGetPrimary "
    "MonitorGetWorkArea NumGet NumPut ObjAddRef ObjGetBase ObjGetCapacity ObjOwnPropCount "
    "ObjRawGet ObjRawSet ObjRelease ObjSetBase ObjSetCapacity OnClipboardChange OnMessage Ord "
    "ProcessExist ProcessWait ProcessWaitClose RTrim RegExMatch RegExReplace "
    "RegisterCallback Round SendMessage Sin SoundGetInterface SoundGetMute SoundGetName "
    "SoundGetVolume Sqrt StrCompare StrGet StrLen StrLower StrPut StrReplace "
    "StrSplit StrUpper String SubStr SysGetIPAddresses TV_Add TV_Delete TV_Get "
    "TV_GetChild TV_GetCount TV_GetNext TV_GetParent TV_GetPrev TV_GetSelection TV_GetText "
    "TV_Modify TV_SetImageList Tan Trim VarSetCapacity WinActive WinExist "
    "WinGetControls WinGetControlsHwnd WinGetCount WinGetExStyle WinGetID WinGetIDLast "
    "WinGetList WinGetMinMax WinGetPID WinGetProcessName WinGetProcessPath WinGetStyle "
    "WinGetText WinGetTitle WinGetTransColor WinGetTransparent",
    
    // Directives
    "AllowSameLineComments ClipboardTimeout CommentFlag DllLoad ErrorStdOut EscapeChar "
    "HotkeyInterval HotkeyModifierTimeout Hotstring If IfTimeout IfWinActive IfWinExist "
    "IfWinNotActive IfWinNotExist Include IncludeAgain InputLevel InstallKeybdHook "
    "InstallMouseHook KeyHistory MaxHotkeysPerInterval MaxMem MaxThreads MaxThreadsBuffer "
    "MaxThreadsPerHotkey MenuMaskKey NoEnv NoTrayIcon Persistent SingleInstance "
    "SuspendExempt UseHook Warn WinActivateForce",

    // Keys & Buttons
    "Alt AltDown AltUp AppsKey BS BackSpace Backspace Break Browser_Back "
    "Browser_Favorites Browser_Forward Browser_Home Browser_Refresh Browser_Search "
    "Browser_Stop CapsLock Control Ctrl CtrlBreak CtrlDown CtrlUp Del Delete Down End Enter "
    "Esc Escape F1 F10 F11 F12 F13 F14 F15 F16 F17 F18 F19 F2 F20 F21 F22 F23 F24 F3 F4 F5 F6 "
    "F7 F8 F9 Help Home Ins Insert Joy1 Joy10 Joy11 Joy12 Joy13 Joy14 Joy15 Joy16 Joy17 Joy18 "
    "Joy19 Joy2 Joy20 Joy21 Joy22 Joy23 Joy24 Joy25 Joy26 Joy27 Joy28 Joy29 Joy3 Joy30 Joy31 "
    "Joy32 Joy4 Joy5 Joy6 Joy7 Joy8 Joy9 JoyAxes JoyButtons JoyInfo JoyName JoyPOV JoyR JoyU "
    "JoyV JoyX JoyY JoyZ LAlt LButton LControl LCtrl LShift LWin LWinDown LWinUp Launch_App1 "
    "Launch_App2 Launch_Mail Launch_Media Left MButton Media_Next Media_Play_Pause Media_Prev "
    "Media_Stop NumLock Numpad0 Numpad1 Numpad2 Numpad3 Numpad4 Numpad5 Numpad6 Numpad7 "
    "Numpad8 Numpad9 NumpadAdd NumpadClear NumpadDel NumpadDiv NumpadDot NumpadDown NumpadEnd "
    "NumpadEnter NumpadHome NumpadIns NumpadLeft NumpadMult NumpadPgDn NumpadPgUp NumpadPgdn "
    "NumpadPgup NumpadRight NumpadSub NumpadUp PgDn PgUp PrintScreen RAlt RButton "
    "RControl RCtrl RShift RWin RWinDown RWinUp Right ScrollLock Shift ShiftDown "
    "ShiftUp Space Up Volume_Down Volume_Mute Volume_Up WheelDown WheelLeft "
    "WheelRight WheelUp XButton1 XButton2",
    
    // Variables
    "A_AhkPath A_AhkVersion A_AllowMainWindow A_AppData A_AppDataCommon A_Args A_AutoTrim "
    "A_BatchLines A_CaretX A_CaretY A_ComSpec A_ComputerName A_ControlDelay A_CoordModeCaret "
    "A_CoordModeMenu A_CoordModeMouse A_CoordModePixel A_CoordModeToolTip A_Cursor A_DD A_DDD "
    "A_DDDD A_DefaultGui A_DefaultListView A_DefaultMouseSpeed A_DefaultTreeView A_Desktop "
    "A_DesktopCommon A_DetectHiddenText A_DetectHiddenWindows A_EndChar A_EventInfo A_ExitReason "
    "A_FileEncoding A_FormatFloat A_FormatInteger A_Gui A_GuiControl A_GuiControlEvent A_GuiEvent "
    "A_GuiHeight A_GuiWidth A_GuiX A_GuiY A_Hour A_IPAddress1 A_IPAddress2 A_IPAddress3 "
    "A_IPAddress4 A_IconFile A_IconHidden A_IconNumber A_IconTip A_Index A_InitialWorkingDir "
    "A_Is64bitOS A_IsAdmin A_IsCompiled A_IsCritical A_IsPaused A_IsSuspended A_IsUnicode "
    "A_KeyDelay A_KeyDelayPlay A_KeyDuration A_KeyDurationPlay A_Language A_LastError A_LineFile "
    "A_LineNumber A_ListLines A_LoopField A_LoopFileAttrib A_LoopFileDir A_LoopFileExt "
    "A_LoopFileFullPath A_LoopFileLongPath A_LoopFileName A_LoopFilePath A_LoopFileShortName "
    "A_LoopFileShortPath A_LoopFileSize A_LoopFileSizeKB A_LoopFileSizeMB A_LoopFileTimeAccessed "
    "A_LoopFileTimeCreated A_LoopFileTimeModified A_LoopReadLine A_LoopRegName A_MM A_MMM A_MMMM "
    "A_MSec A_Min A_MouseDelay A_MouseDelayPlay A_MyDocuments A_Now A_NowUTC A_OSType A_OSVersion "
    "A_PriorHotkey A_PriorKey A_ProgramFiles A_Programs A_ProgramsCommon A_PtrSize A_RegView "
    "A_ScreenDPI A_ScreenHeight A_ScreenWidth A_ScriptDir A_ScriptFullPath A_ScriptHwnd "
    "A_ScriptName A_Sec A_SendLevel A_SendMode A_Space A_StartMenu A_StartMenuCommon A_Startup "
    "A_StartupCommon A_StoreCapsLockMode A_StringCaseSense A_Tab A_Temp A_ThisFunc A_ThisHotkey "
    "A_ThisLabel A_ThisMenu A_ThisMenuItem A_ThisMenuItemPos A_TickCount A_TimeIdle "
    "A_TimeIdleKeyboard A_TimeIdleMouse A_TimeIdlePhysical A_TimeSincePriorHotkey "
    "A_TimeSinceThisHotkey A_TitleMatchMode A_TitleMatchModeSpeed A_TrayMenu A_UserName A_WDay "
    "A_WinDelay A_WinDir A_WorkingDir A_YDay A_YWeek A_YYYY Clipboard ClipboardAll ComSpec "
    "ErrorLevel False ProgramFiles True",
    
    // Special Parameters (keywords)
    "ACos ASin ATan Abort AboveNormal Abs Add All Alnum Alpha AltSubmit AltTab AltTabAndMenu "
    "AltTabMenu AltTabMenuDismiss AlwaysOnTop And Asc AutoSize Background BackgroundTrans "
    "BelowNormal Between BitAnd BitNot BitOr BitShiftLeft BitShiftRight BitXOr Border Bottom "
    "Button Buttons ByRef Cancel Capacity Caption Center Check Check3 Checkbox Checked "
    "CheckedGray Choose ChooseString Close Color ComboBox Contains ControlList "
    "Count DDL Date DateTime Days Default DeleteAll Delimiter Deref Destroy Digit Disable "
    "Disabled DropDownList Eject Enable Enabled Error ExStyle Exist Expand FileSystem Files "
    "First Flash Float FloatFast Focus Font Grid Group GroupBox GuiClose GuiContextMenu "
    "GuiDropFiles GuiEscape GuiSize HKCC HKCR HKCU HKEY_CLASSES_ROOT HKEY_CURRENT_CONFIG "
    "HKEY_CURRENT_USER HKEY_LOCAL_MACHINE HKEY_USERS HKLM HKU HScroll Hdr Hidden Hide High Hours "
    "ID IDLast Icon IconSmall Ignore ImageList In Integer IntegerFast Interrupt Is Join "
    "Label LastFound LastFoundExist Limit Lines List ListBox ListView Lock Logoff "
    "Low Lower Lowercase MainWindow Margin MaxSize Maximize MaximizeBox MinMax MinSize Minimize "
    "MinimizeBox Minutes MonthCal Mouse Move Multi NA No NoActivate NoDefault NoHide NoIcon "
    "NoMainWindow NoSort NoSortHdr NoStandard NoTab NoTimers Normal Not Number Off Ok On Or "
    "OwnDialogs Owner Parse Password Pic Picture Pixel Pos Pow Priority ProcessName REG_BINARY "
    "REG_DWORD REG_EXPAND_SZ REG_MULTI_SZ REG_SZ RGB Radio Range Read ReadOnly Realtime "
    "Redraw Region Relative Rename Report Resize Restore Retry Screen Seconds "
    "Section Serial SetLabel ShiftAltTab Show Single Slider SortDesc Standard Status "
    "StatusBar StatusCD Style Submit SysMenu Tab Tab2 TabStop Text Theme Tile Time Tip "
    "ToggleCheck ToggleEnable ToolWindow Top Topmost TransColor Transparent Tray TreeView "
    "TryAgain Type UnCheck Unicode Unlock UpDown Upper Uppercase UseErrorLevel VScroll Vis "
    "VisFirst Visible Wait WaitClose WantCtrlA WantF2 WantReturn Wrap Xdigit Yes ahk_class "
    "ahk_exe ahk_group ahk_id ahk_pid bold global italic local norm static strike underline xm "
    "xp xs ym yp ys",
    
    // User Defined
    NULL,

    NULL
};


EDITLEXER lexAHK = {
    SCLEX_AHK, "ahk", IDS_LEX_AHK, L"AutoHotkey Script", L"ahk; ahkl; ia; scriptlet", L"",
    &KeyWords_AHK, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ SCE_AHK_DEFAULT, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_AHK_COMMENTLINE,SCE_AHK_COMMENTBLOCK,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_AHK_ESCAPE}, IDS_LEX_STR_63306, L"Escape", L"fore:#FF8000", L"" },
        { {SCE_AHK_SYNOPERATOR}, IDS_LEX_STR_Special, L"Syntax Operator", L"fore:#7F200F", L"" },
        { {SCE_AHK_EXPOPERATOR}, IDS_LEX_STR_63308, L"Expression Operator", L"fore:#FF4F00", L"" },
        { {SCE_AHK_STRING}, IDS_LEX_STR_String, L"String", L"fore:#747474", L"" },
        { {SCE_AHK_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#416CAD", L"" },
        { {SCE_AHK_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"fore:#CF2F0F", L"" },
        { {SCE_AHK_VARREF}, IDS_LEX_STR_63309, L"Variable Dereferencing", L"fore:#CF2F0F; back:#E4FFE4", L"" },
        { {SCE_AHK_LABEL}, IDS_LEX_STR_Label, L"Label", L"fore:#000000; back:#FFFFA1", L"" },
        { {SCE_AHK_WORD_CF}, IDS_LEX_STR_63310, L"Flow of Control", L"bold; fore:#880088", L"" },
        { {SCE_AHK_WORD_CMD}, IDS_LEX_STR_Cmd, L"Command", L"fore:#0036D9", L"" },
        { {SCE_AHK_WORD_FN}, IDS_LEX_STR_63277, L"Function", L"italic; fore:#0F707F", L"" },
        { {SCE_AHK_WORD_DIR}, IDS_LEX_STR_63203, L"Directive", L"italic; fore:#F04020", L"" },
        { {SCE_AHK_WORD_KB}, IDS_LEX_STR_63311, L"Keys & Buttons", L"bold; fore:#FF00FF", L"" },
        { {SCE_AHK_WORD_VAR}, IDS_LEX_STR_63312, L"Built-In Variables", L"italic; fore:#CF00CF", L"" },
        { {SCE_AHK_WORD_SP}, IDS_LEX_STR_63376, L"Special", L"italic; fore:#BD8E00", L"" },
        //{ {SCE_AHK_WORD_UD}, IDS_LEX_STR_63106, L"User Defined", L"fore:#800020", L"" },
        { {SCE_AHK_VARREFKW}, IDS_LEX_STR_63313, L"Variable Keyword", L"italic; fore:#CF00CF; back:#F9F9FF", L"" },
        EDITLEXER_SENTINEL
    }
};
