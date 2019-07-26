#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Icon=..\..\Geek Computers\Pictures_for_website\geek2.ico
#AutoIt3Wrapper_Outfile=..\AutoIT\Compiled Exe\environment.exe
#AutoIt3Wrapper_Outfile_x64=..\AutoIT\Compiled Exe_64bit\environment.exe
#AutoIt3Wrapper_Compression=4
#AutoIt3Wrapper_Compile_Both=y
#AutoIt3Wrapper_UseX64=y
#AutoIt3Wrapper_Res_requestedExecutionLevel=asInvoker
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
; Script Name	: environment.au3
; Author		: Craig Richards
; Created		: 20th February 2012
; Last Modified	: 29th February 2012
; Version		: 1.2

; Modifications	: 1.1 - 29/02/12 - CR - Added dropbox registry value and zipped_logs directory
;				: 1.2 - 07/03/2012 - CR - Added a Msgbox for when it's set the environment up

; Description	: Sets up my enviroment when I get a new PC

; REG_SZ is used to write out if you are using a current environment variable such as %USERPROFILE% like I am then use REG_EXPAND_SZ this way it expands it our otherwise it would be see as %USERPROFILE% instead of c:\Documents and Settings\Crichards for example

Setup_dirs()
Registry_settings()
MsgBox(0, "Environment", "Your Environment has been setup")

Func Registry_settings()
	$scripts = RegWrite("HKEY_CURRENT_USER\Environment", "scripts", "REG_EXPAND_SZ", "%USERPROFILE%\My Documents\My Dropbox\scripts")
	$logs = RegWrite("HKEY_CURRENT_USER\Environment", "logs","REG_SZ", "C:\logs")
	$docs = RegWrite("HKEY_CURRENT_USER\Environment", "docs", "REG_SZ", "P:\oracle_unix\documentation")
	$dropbox = RegWrite("HKEY_CURRENT_USER\Environment", "dropbox", "REG_EXPAND_SZ", "%USERPROFILE%\My Documents\My Dropbox") ; 1.1
	EnvUpdate()
EndFunc

Func Setup_dirs()
	Local $logsdir = "c:\logs"
	Local $puttylogsdir = "c:\logs\puttylogs"
	Local $zippedlogsdir = "c:\logs\puttylogs\zipped_logs"
	If DirGetSize($logsdir) = -1 Then
		DirCreate($logsdir)
	ElseIf DirGetSize($puttylogsdir) = -1 Then
		DirCreate($puttylogsdir)
	ElseIf DirGetSize($zippedlogsdir) = -1 Then
		DirCreate($zippedlogsdir)
	Else
		MsgBox(48, "Directory Creation", "Directory already exists!")
	EndIf
EndFunc
