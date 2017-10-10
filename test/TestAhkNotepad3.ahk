; =============================================================================
; Regression Tests Notepad3 Gui
; Needs files in a Test Directory:
; Notepad3.exe and Notepad3.ini (from distrib)
; Execute: AutoHotkeyU32.exe "TestAhkNotepad3.ahk"
; =============================================================================
#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
; #Warn  ; Enable warnings to assist with detecting common errors.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.
SetBatchLines, -1
StringCaseSense, Off
CoordMode, Pixel, Screen
; =============================================================================

v_NP3Name = Notepad3
v_NP3TestDir = %A_WorkingDir%\_TESTDIR
v_NP3IniFile = %v_NP3Name%.ini

stdout := FileOpen("*", "w")

; =============================================================================

Run, %v_NP3TestDir%/%v_NP3Name%.exe %v_NP3TestDir%/%v_NP3IniFile%, , UseErrorLevel, v_Notepad3_PID
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
    stdout.WriteLine("*** ERROR: " . v_NP3Name . "could not be launched.")
    Goto LABEL_END
}
; -----------------------------------------------------------------------------

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
    stdout.WriteLine("*** ERROR: " . v_NP3Name . "'s seems not to start in time ???")
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
    stdout.WriteLine("*** ERROR: " . v_NP3Name . " missing in Title: " . v_NP3Title)
}
IfNotInString, v_NP3Title, %v_NP3IniFile%
{
    v_ErrLevel = 1
    stdout.WriteLine("*** ERROR: " . v_NP3IniFile . " missing in Title: " . v_NP3Title)
}
IfNotInString, v_NP3Title, %v_NP3TestDir%
{
    v_ErrLevel = 1
    stdout.WriteLine("*** ERROR: " . v_NP3TestDir . " missing in Title: " . v_NP3Title)
}
If (v_ErrLevel != 0)
{
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
    stdout.WriteLine("*** ERROR: " . v_NP3Name . "'s About Box is not displayed!")
    Return
}
WinActivate  ; About Box
;ControlFocus, OK, About %v_NP3Name%
 ControlClick, OK, About %v_NP3Name%
;Send {Enter}
WinWaitClose, About %v_NP3Name%, , 1
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
    stdout.WriteLine("*** ERROR: " . v_NP3Name . "'s About Box can not be closed!")
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
    stdout.WriteLine("*** ERROR: " . v_NP3Name . "can not be closed!")
}
; -------------------------------------
WinWaitClose ahk_pid %v_Notepad3_PID%
v_ErrLevel = %ErrorLevel%
if (v_ErrLevel != 0)
{
    ; FORCED Kill / HANGUP
}
; -------------------------------------
if (v_ErrLevel != 0)
{
  stdout.WriteLine("*** ERROR: Testing " . v_NP3Name . " exits with error level: " . v_ErrLevel)
  Exit, %v_ErrLevel%
}
Exit, 0
; =============================================================================
