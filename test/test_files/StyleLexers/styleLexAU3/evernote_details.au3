#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Icon=..\..\..\..\Desktop\evernote.ico
#AutoIt3Wrapper_Outfile=..\AutoIT\Compiled Exe\evernote_details.exe
#AutoIt3Wrapper_Outfile_x64=..\AutoIT\Compiled Exe_64bit\evernote_details.exe
#AutoIt3Wrapper_Compression=4
#AutoIt3Wrapper_Compile_Both=y
#AutoIt3Wrapper_UseX64=y
#AutoIt3Wrapper_Res_requestedExecutionLevel=asInvoker
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
; Script Name	: evernote_details.au3
; Author		: Craig Richards
; Created		: 29th February 2012
; Last Modified	:
; Version		: 1.0

; Modifications	:

; Description	: Displays the location and the size of your evernote database


Local $evernote = RegRead("HKEY_CURRENT_USER\Software\Evernote\Evernote\","EvernotePath")	; Get the path to the database from the registry
Local $dbsize = DirGetSize($evernote)														; Get the size of the folder

; Display a message bos with the location of the database and also the size in MB

MsgBox(0,"Evernote Details", "Your evernote database is here : " & $evernote & @LF & "The total Size is : " & Round($dbsize / 1024 / 1024) & " (MB)")
