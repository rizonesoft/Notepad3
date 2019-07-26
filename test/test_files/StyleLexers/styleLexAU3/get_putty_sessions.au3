#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Outfile=Compiled Exe\get_putty_sessions.exe
#AutoIt3Wrapper_Outfile_x64=Compiled Exe_64bit\get_putty_sessions_64.exe
#AutoIt3Wrapper_Compression=4
#AutoIt3Wrapper_Compile_Both=y
#AutoIt3Wrapper_UseX64=y
#AutoIt3Wrapper_Res_requestedExecutionLevel=asInvoker
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
; Script Name	: get_putty_sessions.au3
; Author		: Craig Richards
; Created		: 26th January 2012
; Last Modified	:
; Version		: 1.0

; Modifications	:

; Description	: Will get a list of your putty sessions and output them to a log

#Include <File.au3>																				; Include the File Header File
$x=1																							; Set the Variable x to 1
while $x<=150																					; While x is less than 150
	$var = RegEnumKey("HKEY_CURRENT_USER\Software\SimonTatham\PuTTY\sessions",$x)				; Set var to the value of the registry branch
	If $var <> "" Then																			; If the Name is not blank
	  _FileWriteLog(@UserProfileDir & "\puttyconnections.log", $var)							; Write the Key branch names to the files
  EndIf																							; End of the IF Statement
  $x = $x+1																						; Increment the count to 1
Wend																							; End the while loop

; Create a message box informing them the log has been created, if they don't click on OK it will display after 10 Seconds
MsgBox(4096,"Notification","Logout put to " & @UserProfileDir & "\puttyconnections" & ".log",10)