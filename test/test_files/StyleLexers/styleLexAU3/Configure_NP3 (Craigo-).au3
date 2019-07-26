#cs ---------------------------------------------------

 AutoIt Version: 3.3.14.5
 Author:         Craig O'Brien

 Script Function:
  Configure the supplied Notepad3 executable how I like it
  Designed for Notepad3 5.19.709+
  Prerequisites:
    - Cousine font
  To Do:
    - Cope with element repositioning caused by font scaling

#ce ---------------------------------------------------

autoitsetoption("MustDeclareVars", 1)   ; must declare variables
autoitsetoption("WinTitleMatchMode", 2) ; window title: match anywhere
autoitsetoption("MouseCoordMode", 0)    ; mouse coordinate mode: window
autoitsetoption("PixelCoordMode", 0)    ; pixel coordinate mode: window

If $CmdLine[0] <> 1 Then
  MsgBox(0, "Error - Incorrect Arguments", "This script must be passed exactly one argument - the full path and filename to the Notepad3 executable to be configured. Check that this script has been passed an argument, and that if the argument includes spaces, that it is enclosed in quotes.", 10)
  Exit(1)
EndIf

Dim $PathToNotepad3       = $CmdLine[1]
Dim $WindowTitle          = "Untitled - Notepad3"
Dim $WindowTitleShort     = " - Notepad3"
Dim $DefaultWait          = 250
Dim $ToolTip_X
Dim $ToolTip_Y
Dim $WindowLeftEdge       = (@DesktopWidth / 2) + 16
Dim $WindowTopEdge        = 90
Dim $WindowWidth          = (@DesktopWidth / 2) - 32
Dim $WindowHeight         = @DesktopHeight - 146
Dim $Reply
Dim $Colour_MenuTickbox   = 0x333333
; Dim $Colour_ExitButtonHighDPIOff = 0x2244AD
Dim $Colour_ExitButtonHighDPIOn = 0x5B9FE2
; Dim $Colour_HistoryButton = 
Dim $CheckboxToggled

; Launch Notepad3
ToolTip("")
ToolTip("Launching Notepad3...", 800, 0)
ShellExecute($PathToNotepad3)
Sleep(1800)
ToolTip("")
ToolTip("Waiting for Notepad3 window...", 800, 0)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")

; Position Window
ToolTip("")
ToolTip("Positioning application window...", 800, 0)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
WinMove($WindowTitle, "", $WindowLeftEdge, $WindowTopEdge, $WindowWidth, $WindowHeight, 3)

; Set ToolTip coordinates relative to application window
$ToolTip_X = $WindowLeftEdge + 275
$ToolTip_Y = $WindowTopEdge

; This is a good place to do a coordinate check
; CheckCoords("!p", 120, 130, 134, 144)
; Exit

; Enter some text so we can see what's going on
ToolTip("")
ToolTip("Entering some text...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("O0    1Il{ENTER}TMasgqlI|12345{+}6-7*8/9{^}0Oo'""$&?.,;:-(){{}{}}[]<>{ENTER}Illegal1 = O0{ENTER}")

; FILE MENU

; Default Encoding: UTF8
ToolTip("")
ToolTip("Setting default encoding to UTF-8...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("!f")
Sleep($DefaultWait)
Send("e")
Sleep($DefaultWait)
Send("{UP}")
Sleep($DefaultWait)
Send("{ENTER}")
Sleep($DefaultWait)
WinWait("Encoding", "Default Encoding")
WinActivate("Encoding", "Default Encoding")
ControlCommand("Encoding", "Default Encoding", "ComboBoxEx321", "SetCurrentSelection", 6)
Sleep($DefaultWait)
ControlClick("Encoding", "Default Encoding", "[CLASS:Button; TEXT:OK]") ; OK
Sleep($DefaultWait)

; Page Margins: 10mm everywhere
ToolTip("")
ToolTip("Setting page margins to 10mm...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("!f")
Sleep($DefaultWait)
Send("t")
Sleep($DefaultWait)
WinWait("Page Setup", "Margins (millimeters)")
WinActivate("Page Setup", "Margins (millimeters)")
While ControlGetText("Page Setup", "Margins (millimeters)", "[CLASS:Edit; INSTANCE:1]") <> "10"
  ControlSetText("Page Setup", "Margins (millimeters)", "[CLASS:Edit; INSTANCE:1]", "10")
  Sleep($DefaultWait)
WEnd
While ControlGetText("Page Setup", "Margins (millimeters)", "[CLASS:Edit; INSTANCE:2]") <> "10"
  ControlSetText("Page Setup", "Margins (millimeters)", "[CLASS:Edit; INSTANCE:2]", "10")
  Sleep($DefaultWait)
WEnd
While ControlGetText("Page Setup", "Margins (millimeters)", "[CLASS:Edit; INSTANCE:3]") <> "10"
  ControlSetText("Page Setup", "Margins (millimeters)", "[CLASS:Edit; INSTANCE:3]", "10")
  Sleep($DefaultWait)
WEnd
While ControlGetText("Page Setup", "Margins (millimeters)", "[CLASS:Edit; INSTANCE:4]") <> "10"
  ControlSetText("Page Setup", "Margins (millimeters)", "[CLASS:Edit; INSTANCE:4]", "10")
  Sleep($DefaultWait)
WEnd
ControlClick("Page Setup", "Margins (millimeters)", "[CLASS:Button; TEXT:OK]") ; OK
Sleep($DefaultWait)

; VIEW MENU

; Word Wrap: enable
ToolTip("")
ToolTip("Checking Word Wrap is enabled...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While Not DisplayElementSet("!v", 80, 56, 94, 70, $Colour_MenuTickbox)
  Send("^w")
  Sleep($DefaultWait)
WEnd

; Long Line Marker: disable
ToolTip("")
ToolTip("Checking Long Line Marker is disabled...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While DisplayElementSet("!v", 80, 78, 94, 92, $Colour_MenuTickbox)
  Send("^L")
  Sleep($DefaultWait)
WEnd

; Show Wrap Symbols: disable
ToolTip("")
ToolTip("Checking Show Wrap Symbols is disabled...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While DisplayElementSet("!v", 80, 129, 94, 143, $Colour_MenuTickbox)
  Send("^+7")
  Sleep($DefaultWait)
WEnd

; Show Blanks: enable
ToolTip("")
ToolTip("Checking Show Blanks is enabled...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While Not DisplayElementSet("!v", 80, 151, 94, 165, $Colour_MenuTickbox)
  Send("^+8")
  Sleep($DefaultWait)
WEnd

; Selection Margin: disable
ToolTip("")
ToolTip("Checking Selection Margin is disabled...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While DisplayElementSet("!v", 80, 341, 94, 355, $Colour_MenuTickbox)
  Send("^M")
  Sleep($DefaultWait)
WEnd

; Customise Toolbar: remove Exit button and preceding separator
ToolTip("")
ToolTip("Customising Toolbar...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("!v")
Sleep($DefaultWait)
Send("t")
Sleep($DefaultWait)
Send("c")
Sleep($DefaultWait)
WinWait("Customize Toolbar", "A&vailable toolbar buttons:")
WinActivate("Customize Toolbar", "A&vailable toolbar buttons:")
If DisplayElementSet("", 286, 143, 306, 161, $Colour_ExitButtonHighDPIOn) Then
  ControlClick("Customize Toolbar", "A&vailable toolbar buttons:", "[CLASS:ListBox; INSTANCE:2]", "left", 1, 11, 101)
  Sleep($DefaultWait)
  ControlClick("Customize Toolbar", "A&vailable toolbar buttons:", "[CLASS:Button; TEXT:<- &Remove]") ; Remove
  Sleep($DefaultWait)
  ControlClick("Customize Toolbar", "A&vailable toolbar buttons:", "[CLASS:ListBox; INSTANCE:2]", "left", 1, 11, 101)
  Sleep($DefaultWait)
  ControlClick("Customize Toolbar", "A&vailable toolbar buttons:", "[CLASS:Button; TEXT:<- &Remove]") ; Remove
  Sleep($DefaultWait)
EndIf
ControlClick("Customize Toolbar", "A&vailable toolbar buttons:", "[CLASS:Button; TEXT:&Close]") ; Close
Sleep($DefaultWait)

; Save as Default Position
ToolTip("")
ToolTip("Saving Default Position...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("!v")
Sleep($DefaultWait)
Send("{UP}")
Sleep($DefaultWait)
Send("{RIGHT}")
Sleep($DefaultWait)
Send("{DOWN}")
Sleep($DefaultWait)
Send("{DOWN}")
Sleep($DefaultWait)
Send("{ENTER}")
Sleep($DefaultWait)

; APPEARANCE MENU

; Default Text Font: Cousine Regular 10
ToolTip("")
ToolTip("Disabling 2nd Default Text...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While DisplayElementSet("!p", 120, 130, 134, 144, $Colour_MenuTickbox)
  Send("+{F12}")
  Sleep($DefaultWait)
WEnd
ToolTip("")
ToolTip("Setting Default Text font to Cousine Regular 10...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("!{F12}")
WinWait(" BASE (Default Style)", "")
WinActivate(" BASE (Default Style)", "")
While ControlGetText(" BASE (Default Style)", "", "[CLASS:Edit; INSTANCE:1]") <> "Cousine"
  ControlFocus(" BASE (Default Style)", "", "[CLASS:Edit; INSTANCE:1]")
  Sleep($DefaultWait)
  Send("Cousine")
  Sleep($DefaultWait * 4)
WEnd
While ControlGetText(" BASE (Default Style)", "", "[CLASS:Edit; INSTANCE:2]") <> "Regular"
  ControlFocus(" BASE (Default Style)", "", "[CLASS:Edit; INSTANCE:2]")
  Sleep($DefaultWait)
  Send("Regular")
  Sleep($DefaultWait * 4)
WEnd
While ControlGetText(" BASE (Default Style)", "", "[CLASS:Edit; INSTANCE:3]") <> "10"
  ControlFocus(" BASE (Default Style)", "", "[CLASS:Edit; INSTANCE:3]")
  Sleep($DefaultWait)
  Send("10")
  Sleep($DefaultWait * 4)
WEnd
ControlClick(" BASE (Default Style)", "", "[CLASS:Button; TEXT:OK]") ; OK
Sleep($DefaultWait)

; 2nd Default Text Font: Verdana Regular 10
ToolTip("")
ToolTip("Enabling 2nd Default Text...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While Not DisplayElementSet("!p", 120, 130, 134, 144, $Colour_MenuTickbox)
  Send("+{F12}")
  Sleep($DefaultWait)
WEnd
ToolTip("")
ToolTip("Setting 2nd Default Text font to Verdana Regular 10...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("!{F12}")
WinWait(" BASE (2nd Default Style)", "")
WinActivate(" BASE (2nd Default Style)", "")
While ControlGetText(" BASE (2nd Default Style)", "", "[CLASS:Edit; INSTANCE:1]") <> "Verdana"
  ControlFocus(" BASE (2nd Default Style)", "", "[CLASS:Edit; INSTANCE:1]")
  Sleep($DefaultWait)
  Send("Verdana")
  Sleep($DefaultWait * 4)
WEnd
While ControlGetText(" BASE (2nd Default Style)", "", "[CLASS:Edit; INSTANCE:2]") <> "Regular"
  ControlFocus(" BASE (2nd Default Style)", "", "[CLASS:Edit; INSTANCE:2]")
  Sleep($DefaultWait)
  Send("Regular")
  Sleep($DefaultWait * 4)
WEnd
While ControlGetText(" BASE (2nd Default Style)", "", "[CLASS:Edit; INSTANCE:3]") <> "10"
  ControlFocus(" BASE (2nd Default Style)", "", "[CLASS:Edit; INSTANCE:3]")
  Sleep($DefaultWait)
  Send("10")
  Sleep($DefaultWait * 4)
WEnd
ControlClick(" BASE (2nd Default Style)", "", "[CLASS:Button; TEXT:OK]") ; OK
Sleep($DefaultWait)
ToolTip("")
ToolTip("Activating Default Text...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While DisplayElementSet("!p", 120, 130, 134, 144, $Colour_MenuTickbox)
  Send("+{F12}")
  Sleep($DefaultWait)
WEnd

; SETTINGS MENU

; Insert Tabs as Spaces: enable
ToolTip("")
ToolTip("Checking Insert Tabs as Spaces is enabled...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While Not DisplayElementSet("!s", 196, 86, 210, 100, $Colour_MenuTickbox)
  Send("!s")
  Sleep($DefaultWait)
  Send("s")
  Sleep($DefaultWait)
WEnd

; Word Wrap Settings: no wrap indent
ToolTip("")
ToolTip("Checking No wrap indent...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("!s")
Sleep($DefaultWait)
Send("w")
Sleep($DefaultWait)
WinWait("Word Wrap Settings")
WinActivate("Word Wrap Settings")
ControlCommand("Word Wrap Settings", "", "ComboBox1", "SetCurrentSelection", 0)
Sleep($DefaultWait)
ControlClick("Word Wrap Settings", "", "[CLASS:Button; TEXT:OK]") ; OK
Sleep($DefaultWait)

; Save Settings on Exit: disabled
ToolTip("")
ToolTip("Checking Save Settings on Exit is disabled...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
While DisplayElementSet("!sa", 522, 474, 536, 488, $Colour_MenuTickbox)
  Send("!s")
  Sleep($DefaultWait)
  Send("a")
  Sleep($DefaultWait)
  Send("x")
  Sleep($DefaultWait)
WEnd

; Save Settings Now
ToolTip("")
ToolTip("Saving settings...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("{F7}")
WinWait("Notepad3", "The current program settings have been saved.")
WinActivate("Notepad3", "The current program settings have been saved.")
ControlClick("Notepad3", "The current program settings have been saved.", "[CLASS:Button; TEXT:OK]") ; OK
Sleep($DefaultWait)

; SETTINGS FILE

; Open Settings File
ToolTip("")
ToolTip("Opening settings file...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")
Send("^{F7}")
WinWait("Notepad3", "Save changes to ""Untitled""?")
WinActivate("Notepad3", "Save changes to ""Untitled""?")
ControlClick("Notepad3", "Save changes to ""Untitled""?", "[CLASS:Button; TEXT:&No]") ; No
Sleep($DefaultWait)

; Settings File: DefaultExtension
ToolTip("")
ToolTip("Setting DefaultExtension...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitleShort, "")
WinActivate($WindowTitleShort, "")
Send("^h")
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:1]", ";DefaultExtension=txt\r\n;") ; Search String
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:2]", ";DefaultExtension=txt\r\nDefaultExtension=txt\r\n;") ; Replace with
If Not ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "IsChecked") Then
  ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "Check")
  $CheckboxToggled = True
Else
  $CheckboxToggled = False
EndIf
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Replace &All]") ; Replace All
WinWait("Notepad3", "specified text")
WinActivate("Notepad3", "specified text")
ControlClick("Notepad3", "specified text", "[CLASS:Button; TEXT:OK]"); OK
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
If $CheckboxToggled Then
  ControlClick("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]")
EndIf
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Close]") ; Close
Sleep($DefaultWait)

; Settings File: FileLoadWarning
ToolTip("")
ToolTip("Setting FileLoadWarning...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitleShort, "")
WinActivate($WindowTitleShort, "")
Send("^h")
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:1]", ";FileLoadWarningMB=1\r\n;") ; Search String
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:2]", ";FileLoadWarningMB=1\r\nFileLoadWarningMB=5\r\n;") ; Replace with
If Not ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "IsChecked") Then
  ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "Check")
  $CheckboxToggled = True
Else
  $CheckboxToggled = False
EndIf
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Replace &All]") ; Replace All
WinWait("Notepad3", "specified text")
WinActivate("Notepad3", "specified text")
ControlClick("Notepad3", "specified text", "[CLASS:Button; TEXT:OK]"); OK
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
If $CheckboxToggled Then
  ControlClick("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]")
EndIf
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Close]") ; Close
Sleep($DefaultWait)

; Settings File: MarkOccurrencesMaxCount
ToolTip("")
ToolTip("Setting MarkOccurrencesMaxCount...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitleShort, "")
WinActivate($WindowTitleShort, "")
Send("^h")
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:1]", ";MarkOccurrencesMaxCount=2000\r\n;") ; Search String
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:2]", ";MarkOccurrencesMaxCount=2000\r\nMarkOccurrencesMaxCount=30000\r\n;") ; Replace with
If Not ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "IsChecked") Then
  ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "Check")
  $CheckboxToggled = True
Else
  $CheckboxToggled = False
EndIf
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Replace &All]") ; Replace All
WinWait("Notepad3", "specified text")
WinActivate("Notepad3", "specified text")
ControlClick("Notepad3", "specified text", "[CLASS:Button; TEXT:OK]"); OK
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
If $CheckboxToggled Then
  ControlClick("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]")
EndIf
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Close]") ; Close
Sleep($DefaultWait)

; Settings file: VisibleSections
ToolTip("")
ToolTip("Setting VisibleSections...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitleShort, "")
WinActivate($WindowTitleShort, "")
Send("^h")
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:1]", ";VisibleSections=0 1 12 14 2 4 5 6 7 8 9 10 11\r\n;") ; Search String
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:2]", ";VisibleSections=0 1 12 14 2 4 5 6 7 8 9 10 11\r\nVisibleSections=0 1 14 2 4 5 6 7 8 9 10 11\r\n;") ; Replace with
If Not ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "IsChecked") Then
  ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "Check")
  $CheckboxToggled = True
Else
  $CheckboxToggled = False
EndIf
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Replace &All]") ; Replace All
WinWait("Notepad3", "specified text")
WinActivate("Notepad3", "specified text")
ControlClick("Notepad3", "specified text", "[CLASS:Button; TEXT:OK]"); OK
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
If $CheckboxToggled Then
  ControlClick("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]")
EndIf
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Close]") ; Close
Sleep($DefaultWait)

; Settings file: SectionWidthSpecs
ToolTip("")
ToolTip("Setting SectionWidthSpecs...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitleShort, "")
WinActivate($WindowTitleShort, "")
Send("^h")
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:1]", ";SectionWidthSpecs=30 20 20 20 20 20 0 0 0 0 0 0 20 20 20\r\n;") ; Search String
ControlSetText("Replace Text", "Replace wit&h:", "[CLASS:Edit; INSTANCE:2]", ";SectionWidthSpecs=30 20 20 20 20 20 0 0 0 0 0 0 20 20 20\r\nSectionWidthSpecs=30 20 20 20 20 20 0 0 -44 -32 -32 0 20 20 20\r\n;") ; Replace with
If Not ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "IsChecked") Then
  ControlCommand("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]", "Check")
  $CheckboxToggled = True
Else
  $CheckboxToggled = False
EndIf
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Replace &All]") ; Replace All
WinWait("Notepad3", "specified text")
WinActivate("Notepad3", "specified text")
ControlClick("Notepad3", "specified text", "[CLASS:Button; TEXT:OK]"); OK
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
If $CheckboxToggled Then
  ControlClick("Replace Text", "&Transform backslashes", "[CLASS:Button; TEXT:&Transform backslashes]")
EndIf
WinWait("Replace Text", "Replace wit&h:")
WinActivate("Replace Text", "Replace wit&h:")
ControlClick("Replace Text", "Replace wit&h:", "[CLASS:Button; TEXT:Close]") ; Close
Sleep($DefaultWait)

; Check Settings
ToolTip("")
ToolTip("Check Settings...", $ToolTip_X, $ToolTip_Y)
$Reply = MsgBox(4, "Check Settings", "Check settings and make any additional required manual changes." & @CRLF & @CRLF & "Save?")
Switch $Reply
  Case 6
    ; Yes - save and close settings file
    ToolTip("")
    ToolTip("Saving and closing settings file...", $ToolTip_X, $ToolTip_Y)
    Send("^s")
    Send("!{F4}")
  Case 7
    ; No - close settings file without saving
    ToolTip("")
    ToolTip("Closing settings file without saving...", $ToolTip_X, $ToolTip_Y)
    Send("!{F4}")
    Sleep($DefaultWait)
    If WinExists("Notepad3", "Save changes to ") Then
      ControlClick("Notepad3", "Save changes to ", "[CLASS:Button; TEXT:&No]") ; No
    EndIf
  Case Else
    ; Should never happen - exit
    Exit(1000)
EndSwitch

; Relaunch Notepad3
ToolTip("")
ToolTip("Relaunching Notepad3...", $ToolTip_X, $ToolTip_Y)
ShellExecute($PathToNotepad3)
Sleep(1800)
ToolTip("")
ToolTip("Waiting for Notepad3 window...", $ToolTip_X, $ToolTip_Y)
WinWait($WindowTitle, "")
WinActivate($WindowTitle, "")

; FUNCTIONS

; DisplayElementSet: searches given box area for colour that indicates set
;   Parameters:
;     $PreFuncCommand: keys to send first to invoke menu (optional)
;     $Coord_Left:     left edge
;     $Coord_Top:      top edge
;     $Coord_Right:    right edge
;     $Coord_Bottom:   bottom edge
;     $Colour_Set:     colour to search for that indicates element is set
Func DisplayElementSet($PreFuncCommand, $Coord_Left, $Coord_Top, $Coord_Right, $Coord_Bottom, $Colour_Set)
  ; MouseMove(150, 8)
  Dim $CheckboxArea
  Dim $IsSet
  If $PreFuncCommand <> "" Then
    Send($PreFuncCommand)
    Sleep($DefaultWait)
  EndIf
  $CheckboxArea = PixelSearch($Coord_left, $Coord_top, $Coord_right, $Coord_bottom, $colour_set, 5)
  If @error Then
    $IsSet = False
  Else
    $IsSet = True
  EndIf
  If $PreFuncCommand <> "" Then
    Send("{ALT}")
  EndIf
  Return $IsSet
EndFunc

; CheckCoords: move mouse pointer to window coordinates to check screen element position
;              (called for testing)
;   Parameters:
;     $PreFuncCommand: keys to send first to invoke menu (optional)
;     $Coord_Left:     left edge
;     $Coord_Top:      top edge
;     $Coord_Right:    right edge
;     $Coord_Bottom:   bottom edge
Func CheckCoords($PreFuncCommand, $Coord_Left, $Coord_Top, $Coord_Right, $Coord_Bottom)
  If $PreFuncCommand <> "" Then
    Send($PreFuncCommand)
    Sleep($DefaultWait)
  EndIf
  MouseMove($Coord_Left, $Coord_Top)
  Sleep(1000)
  MouseMove($Coord_Right, $Coord_Top)
  Sleep(1000)
  MouseMove($Coord_Left, $Coord_Bottom)
  Sleep(1000)
  MouseMove($Coord_Right, $Coord_Bottom)
  Sleep(1000)
  ; Send("{ALT}")
EndFunc