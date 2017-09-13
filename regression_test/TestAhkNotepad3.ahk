; =============================================================================
; Regression Tests Notepad3 Gui
; Needs files in a Test Directory:
; This AHK-Script, AHK-Interpreter (AutoHotKeyU32/64.exe)
; + Notepad3.exe and Notepad3.ini (from distrib)
; Execute: .\AutoHotkeyU64.exe "TestAhkNotepad3.ahk"
; =============================================================================
#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
; #Warn  ; Enable warnings to assist with detecting common errors.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.
SetBatchLines, -1
CoordMode, Pixel, Screen
; =============================================================================

v_NP3Name = Notepad3
v_NP3IniDir = %A_WorkingDir%
v_NP3IniFile = %v_NP3Name%.ini

; =============================================================================

Run, %A_ScriptDir%/%v_NP3Name%.exe %v_NP3IniDir%/%v_NP3IniFile%, , UseErrorLevel, v_Notepad3_PID
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
    MsgBox Notepad3.exe could not be launched.
    Goto LABEL_END
}
; =============================================================================

GoSub CHECK_NP3_STARTS
GoSub CHECK_WIN_TITLE
GoSub CHECK_ABOUT_BOX
Goto LABEL_END

; =============================================================================

; =============================================================================
CHECK_NP3_STARTS: 
; check that NP3 starts up
WinWait ahk_pid %v_Notepad3_PID%, , 3
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
	;( test1 == 0 ) ? test2 : test3
    MsgBox %v_NP3Name%'s seems not to start in time ???
    Goto LABEL_END
}
Return
; =============================================================================

; =============================================================================
CHECK_WIN_TITLE: 
; check Main Window Title
WinGetTitle, v_NP3Title, ahk_pid %v_Notepad3_PID%

IfNotInString, v_NP3Title, %v_NP3Name%
{
    v_ErrLevel = 1
    MsgBox Notepad3 has wrong Title: %v_NP3Title%.
    Goto LABEL_END
}
IfNotInString, v_NP3Title, %v_NP3IniFile%
{
    v_ErrLevel = 1
    MsgBox Notepad3 has wrong Title: %v_NP3Title%.
    Goto LABEL_END
}
IfNotInString, v_NP3Title, %v_NP3IniDir%
{
    v_ErrLevel = 1
    MsgBox Notepad3 has wrong Title: %v_NP3Title%.
    Goto LABEL_END
}
Return
; =============================================================================

; =============================================================================
CHECK_ABOUT_BOX:
; check About DlgBox
WinActivate, ahk_pid %v_Notepad3_PID%
Send {F1}
WinWait, About %v_NP3Name%, , 1
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
    MsgBox Notepad3's About Box is not displayed!
    Goto LABEL_END
}
WinActivate  ; About Box
;ControlFocus, OK, About %v_NP3Name%
 ControlClick, OK, About %v_NP3Name%
;Send {Enter}
WinWaitClose, About %v_NP3Name%, , 1
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
    MsgBox %v_NP3Name%'s About Box can not be closed!
    Goto LABEL_END
}
Return
; =============================================================================
 
; =============================================================================
LABEL_END:
WinClose ahk_pid %v_Notepad3_PID%, , 1
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
    MsgBox %v_NP3Name% can not be closed!
}
; -------------------------------------
WinWaitClose ahk_pid %v_Notepad3_PID%
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
}
; -------------------------------------
if (v_ErrLevel != 0)
{
  MsgBox, 0, ERROR, Testing %v_NP3Name% exits with error level: %v_ErrLevel% !
  Exit, %v_ErrLevel%
}
Exit, 0
; =============================================================================
