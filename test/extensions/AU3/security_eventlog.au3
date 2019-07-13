; Script Name	: eventlog_security.au3
; Author		: Craig Richards
; Created		: 3rd February 2012
; Last Modified	:
; Version		: 1.0

; Modifications	:

; Description	: This will show you the last failed login attempt, it then outputs all failed login attempts to a file, then clears all the Security part of the event log

#Include <EventLog.au3>				; Include the Eventlog header file
#Include <String.au3>				; Include the String header file
#Include <Array.au3>				; Include the Array header file
#Include <GUIConstantsEx.au3>		; Include the GUIConstantsEx header file
#Include <Date.au3>					; Include the Date header file
#Include <File.au3>					; Include the File header file

Global $iMemo						; Set the Global Variable iMemo

_Main()								; Call the Function _Main

Func _Main()						; Start of the Main function
	Local $hEventLog, $aEvent		; Set some Local variables

	GUICreate("EventLog", 400, 300)	; Create a Window to display the last record
    $iMemo = GUICtrlCreateEdit("", 2, 2, 396, 300, 0)
    GUICtrlSetFont($iMemo, 9, 400, 0, "Courier New")
    GUISetState()

$hEventLog = _EventLog__Open( "", "Security")	; Create the variable hEventLog, this opens the Security Event Log
$x=_EventLog__Count ($hEventLog)				; Get a count of all the records
MsgBox(4096,"Security events", "You Have " & $x & " events, displaying the last failure event") ; Display the Count of events
For $i=0 To $x												; For all events
	$aEvent = _EventLog__Read($hEventLog, True, False)		; Display just the last event
	if $aEvent[6] = "529" then								; If the Error code is 529
		; Display all the information
		MemoWrite("Result ............: " & $aEvent[ 0])
		MemoWrite("Record number .....: " & $aEvent[ 1])
		MemoWrite("Submitted .........: " & $aEvent[ 2] & " " & $aEvent[ 3])
		MemoWrite("Generated .........: " & $aEvent[ 4] & " " & $aEvent[ 5])
		MemoWrite("Event ID ..........: " & $aEvent[ 6])
		MemoWrite("Type ..............: " & $aEvent[ 8])
		MemoWrite("Category ..........: " & $aEvent[ 9])
		MemoWrite("Source ............: " & $aEvent[10])
		MemoWrite("Computer ..........: " & $aEvent[11])
		MemoWrite("Username ..........: " & $aEvent[12])
		MemoWrite("Description .......: " & $aEvent[13])
		; Write out all the records to a log file and date stamp the file
		_FileWriteLog(@UserProfileDir & "\login_failure_attempts.log." & StringReplace(_DateTimeFormat( _NowCalc(),2), "/", "-"), $aEvent[10] & " " & $aEvent[11] & " " & $aEvent[12] & " " & $aEvent[13] & " " & $aEvent[4] & " " & $aEvent[5])
	endif													; End of the loop
	Next
_EventLog__Backup($hEventLog, "C:\Security_EventLog.bak." & StringReplace(_DateTimeFormat( _NowCalc(),2), "/", "-"))	; Backup the Security Event log
_EventLog__Clear($hEventLog, "")																						; Clear the event log
_EventLog__Close($hEventLog)																							; Close the event log

	Do
    Until GUIGetMsg() = $GUI_EVENT_CLOSE

EndFunc   ; End of the Main Function

Func MemoWrite($sMessage)							; Start of the Memowrite Function, to display the messages
    GUICtrlSetData($iMemo, $sMessage & @CRLF, 1)	; Display the messages
EndFunc   ;==>MemoWrite								; End of the Function