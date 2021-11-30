; Create the sub-menus for the menu bar:
Menu, FileMenu, Add, &New, FileNew
Menu, FileMenu, Add, &Open, FileOpen
Menu, FileMenu, Add, &Save, FileSave
Menu, FileMenu, Add, Save &As, FileSaveAs
Menu, FileMenu, Add  ; Separator line.
Menu, FileMenu, Add, E&xit, FileExit
Menu, HelpMenu, Add, &About, HelpAbout

; Create the menu bar by attaching the sub-menus to it:
Menu, MyMenuBar, Add, &File, :FileMenu
Menu, MyMenuBar, Add, &Help, :HelpMenu

; Attach the menu bar to the window:
Gui, Menu, MyMenuBar

; Create the main Edit control and display the window:
Gui, +Resize  ; Make the window resizable.
Gui, Add, Edit, vMainEdit WantTab W600 R20
Gui, Show,, Untitled
CurrentFileName := ""  ; Indicate that there is no current file.
return

FileNew:
GuiControl,, MainEdit  ; Clear the Edit control.
return

FileOpen:
Gui +OwnDialogs  ; Force the user to dismiss the FileSelectFile dialog before returning to the main window.
FileSelectFile, SelectedFileName, 3,, Open File, Text Documents (*.txt)
if not SelectedFileName  ; No file selected.
    return
Gosub FileRead
return

FileRead:  ; Caller has set the variable SelectedFileName for us.
FileRead, MainEdit, %SelectedFileName%  ; Read the file's contents into the variable.
if ErrorLevel
{
    MsgBox Could not open "%SelectedFileName%".
    return
}
GuiControl,, MainEdit, %MainEdit%  ; Put the text into the control.
CurrentFileName := SelectedFileName
Gui, Show,, %CurrentFileName%   ; Show file name in title bar.
return

FileSave:
if not CurrentFileName   ; No filename selected yet, so do Save-As instead.
    Goto FileSaveAs
Gosub SaveCurrentFile
return

FileSaveAs:
Gui +OwnDialogs  ; Force the user to dismiss the FileSelectFile dialog before returning to the main window.
FileSelectFile, SelectedFileName, S16,, Save File, Text Documents (*.txt)
if not SelectedFileName  ; No file selected.
    return
CurrentFileName := SelectedFileName
Gosub SaveCurrentFile
return

SaveCurrentFile:  ; Caller has ensured that CurrentFileName is not blank.
if FileExist(CurrentFileName)
{
    FileDelete %CurrentFileName%
    if ErrorLevel
    {
        MsgBox The attempt to overwrite "%CurrentFileName%" failed.
        return
    }
}
GuiControlGet, MainEdit  ; Retrieve the contents of the Edit control.
FileAppend, %MainEdit%, %CurrentFileName%  ; Save the contents to the file.
; Upon success, Show file name in title bar (in case we were called by FileSaveAs):
Gui, Show,, %CurrentFileName%
return

HelpAbout:
Gui, About:+owner1  ; Make the main window (Gui #1) the owner of the "about box".
Gui +Disabled  ; Disable main window.
Gui, About:Add, Text,, Text for about box.
Gui, About:Add, Button, Default, OK
Gui, About:Show
return

AboutButtonOK:  ; This section is used by the "about box" above.
AboutGuiClose:
AboutGuiEscape:
Gui, 1:-Disabled  ; Re-enable the main window (must be done prior to the next step).
Gui Destroy  ; Destroy the about box.
return

GuiDropFiles:  ; Support drag & drop.
Loop, Parse, A_GuiEvent, `n
{
    SelectedFileName := A_LoopField  ; Get the first file only (in case there's more than one).
    break
}
Gosub FileRead
return

GuiSize:
if (ErrorLevel = 1)  ; The window has been minimized. No action needed.
    return
; Otherwise, the window has been resized or maximized. Resize the Edit control to match.
NewWidth := A_GuiWidth - 20
NewHeight := A_GuiHeight - 20
GuiControl, Move, MainEdit, W%NewWidth% H%NewHeight%
return

FileExit:     ; User chose "Exit" from the File menu.
GuiClose:  ; User closed the window.
ExitApp
Copyright © 2

#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
; #Warn  ; Enable warnings to assist with detecting common errors.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.

SetKeyDelay, 200
SetTitleMatchMode, 2

Gosub, GUI
Return

ButtonOk:
Gui, Submit
Modifiers := [STR, DEX, CON, INT, WIS, CHA, PROF]
Open("C:\Users\Ian\OneDrive\Scripts")
Temp.Seek(0)
Loop, % Modifiers.length()
   {
   Line := Modifiers[A_Index]
   Temp.Write(Line)
   }
Text := Temp.Read()
MsgBox % Text
Temp.Close()
Return

!1::
Sendinput, /r 1d20{+}
Return

!2::
Temp.Close()
FileDelete, C:\Users\Ian\OneDrive\Scripts\Temp.txt
Return

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Esc::Exitapp
^NumpadEnter::Pause
ScrollLock::Reload





;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Copy function.

Copy(Variable)
{
clipboard := ""
Sleep, 200
Send, ^c
%Variable% := clipboard
Sleep, 200
}
Return

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; GUI subroutine.

GUI:

Gui, Show, w200 h275, Modifiers

Gui, Add, Text, x10 y10 w180 h20 Left, Enter Modifiers
Gui, Add, Edit, x50 y30 w100 h20 Center vSTR, Strength
Gui, Add, Edit, x50 y60 w100 h20 Center vDEX, Dexterity
Gui, Add, Edit, x50 y90 w100 h20 Center vCON, Constitution
Gui, Add, Edit, x50 y120 w100 h20 Center vINT, Intelligence
Gui, Add, Edit, x50 y150 w100 h20 Center vWIS, Wisdom
Gui, Add, Edit, x50 y180 w100 h20 Center vCHA, Charisma
Gui, Add, Edit, x50 y210 w100 h20 Center vPROF, Proficiency

Gui, Add, Button, x100 y240 w75 h20 Default, &OK

Return

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Temp File functions.
Open(Path)
{
Pathway := Path . "\Temp.txt"
Temp := FileOpen(Pathway, "rw `n")
}
Return