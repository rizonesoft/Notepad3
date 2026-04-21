; =============================================================================
; Regression Tests Notepad3 Gui
; Needs files in a Test Directory:
; Notepad3.exe and Notepad3.ini (from distrib)
; Execute: AutoHotkey64.exe "TestAhkNotepad3.ahk"
; =============================================================================
#Requires AutoHotkey v2.0
SendMode("Input")
SetWorkingDir(A_ScriptDir)
CoordMode("Pixel", "Screen")
; =============================================================================

; All globals declared and initialised up front so functions always find them set.
global v_NP3Name      := "Notepad3"
global v_NP3TestDir   := A_WorkingDir . "\_TESTDIR"
global v_NP3IniFile   := v_NP3Name . ".ini"
global v_ExitCode     := 0
global v_Notepad3_PID := 0
global stdout         := ""

try {
    stdout := FileOpen("*", "w")
} catch OSError {
    ; stdout not available - script must be run from CMD with output redirection:
    ;   "%AHK_EXE%" /ErrorStdOut TestAhkNotepad3.ahk >> test.log 2>&1
    MsgBox("stdout not available.`nRun via: TestAhkNotepad3.cmd", "TestAhkNotepad3", "Iconx")
    ExitApp(1)
}

; =============================================================================

stdout.WriteLine("Run " . v_NP3Name . ": " . v_NP3TestDir . "\" . v_NP3Name . ".exe '" . v_NP3TestDir . "\" . v_NP3IniFile . ".")

try {
    Run(v_NP3TestDir . "\" . v_NP3Name . ".exe `"" . v_NP3TestDir . "\" . v_NP3IniFile . "`"", , , &v_Notepad3_PID)
} catch Error {
    v_Notepad3_PID := 0   ; Run() unsets &OutputVarPID on throw — restore it
    stdout.WriteLine("*** ERROR: " . v_NP3Name . " could not be launched.")
    v_ExitCode := 1
    Cleanup()
    ExitApp(v_ExitCode)
}
; -----------------------------------------------------------------------------

CHECK_NP3_STARTS()
Sleep(1000)
CHECK_WIN_TITLE()
Sleep(1000)
CHECK_ABOUT_BOX()

Cleanup()
ExitApp(0)
; =============================================================================

; =============================================================================
CHECK_NP3_STARTS() {
    global v_Notepad3_PID, v_NP3Name, stdout, v_ExitCode
    ; check that NP3 starts up
    if !WinWait("ahk_pid " . v_Notepad3_PID, , 10) {
        stdout.WriteLine("*** ERROR: " . v_NP3Name . "'s seems not to start in time ???")
        v_ExitCode := 2
        Cleanup()
        ExitApp(v_ExitCode)
    }
}
; =============================================================================

; =============================================================================
CHECK_WIN_TITLE() {
    global v_Notepad3_PID, v_NP3Name, v_NP3IniFile, v_NP3TestDir, stdout, v_ExitCode
    ; check Main Window Title
    local v_NP3Title := WinGetTitle("ahk_pid " . v_Notepad3_PID)
    stdout.WriteLine(v_NP3Name . "'s Title is: " . v_NP3Title)

    if !InStr(v_NP3Title, v_NP3Name) {
        v_ExitCode := 3
        stdout.WriteLine("*** ERROR: " . v_NP3Name . " missing in Title: ")
    }
    if !InStr(v_NP3Title, v_NP3IniFile) {
        v_ExitCode := 3
        stdout.WriteLine("*** ERROR: " . v_NP3IniFile . " missing in Title: ")
    }
    if !InStr(v_NP3Title, v_NP3TestDir) {
        v_ExitCode := 3
        stdout.WriteLine("*** ERROR: " . v_NP3TestDir . " missing in Title: ")
    }
    if (v_ExitCode != 0) {
        Cleanup()
        ExitApp(v_ExitCode)
    }
}
; =============================================================================

; =============================================================================
CHECK_ABOUT_BOX() {
    global v_Notepad3_PID, v_NP3Name, stdout, v_ExitCode
    ; check About DlgBox
    WinActivate("ahk_pid " . v_Notepad3_PID)

    ; Open Help -> About... via its keyboard shortcut (Shift+F1):
    Send("+{F1}")

    if !WinWait("About " . v_NP3Name, , 3) {
        stdout.WriteLine("*** ERROR: " . v_NP3Name . "'s About Box is not displayed!")
        v_ExitCode := 4
        Cleanup()
        ExitApp(v_ExitCode)
    }
    WinActivate("About " . v_NP3Name)
    ControlClick("OK", "About " . v_NP3Name)
    if !WinWaitClose("About " . v_NP3Name, , 2) {
        stdout.WriteLine("*** ERROR: " . v_NP3Name . "'s About Box can not be closed!")
        v_ExitCode := 5
        Cleanup()
        ExitApp(v_ExitCode)
    }
}
; =============================================================================

; =============================================================================
Cleanup() {
    global v_Notepad3_PID, v_NP3Name, stdout, v_ExitCode
    if (v_Notepad3_PID > 0) {
        WinClose("ahk_pid " . v_Notepad3_PID, , 2)
        if WinExist("ahk_pid " . v_Notepad3_PID)
            WinWaitClose("ahk_pid " . v_Notepad3_PID, , 10)
    }
    if (v_ExitCode != 0)
        stdout.WriteLine("*** ERROR: Testing " . v_NP3Name . " exit with: " . v_ExitCode)
    else
        stdout.WriteLine("Testing " . v_NP3Name . ": All tests PASSED.")
}
; =============================================================================
