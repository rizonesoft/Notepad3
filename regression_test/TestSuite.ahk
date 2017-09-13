/*
 *
 * SciTE4AutoHotkey Syntax Highlighting Demo
 * - by fincs
 *
*/

; Normal comment
/*
Block comment
*/

; Directives, keywords
#SingleInstance Force
#NoTrayIcon

; Command, literal text, escape sequence
MsgBox, Hello World `; This isn't a comment

; Operators
Bar = Foo  ; operators
Foo := Bar ; expression assignment operators

; String
Var := "This is a test"

; Number
Num = 40 + 4

; Dereferencing
Foo := %Bar%

; Flow of control, built-in-variables, BIV dereferencing
if true
	MsgBox, This will always be displayed
Loop, 3
	MsgBox Repetition #%A_Index%

; Built-in function call
MsgBox % SubStr("blaHello Worldbla", 4, 11)

if false
{
	; Keys and buttons
	Send, {F1}
	; Syntax errors
	MyVar = "This is a test
}

ExitApp

; Label, hotkey, hotstring
Label:
#v::MsgBox You pressed Win+V
::btw::by the way
