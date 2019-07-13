#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Icon=..\..\..\..\..\..\geek2.ico
#AutoIt3Wrapper_Outfile=..\..\..\..\..\..\free_diskspace.exe
#AutoIt3Wrapper_Compression=4
#AutoIt3Wrapper_Res_requestedExecutionLevel=asInvoker
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****
; Script Name	: diskspace.au3
; Author		: Craig Richards
; Created		: 3rd February 2012
; Last Modified	:
; Version		: 1.0

; Modifications	:

; Description	: Displays the amount of diskspace free on both Fixed and Network drives

$sBar = "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" ; Prints out the status bar
   $sOut = $sBar & " = 100% Capacity Baseline" & @LF	; Sets variable, this is for the network drives
   $lsOut = $sBar & " = 100% Capacity Baseline" & @LF	; Sets variable, this is for the fixed drives
   ProgressOn("Scanning drives", "", "")				; Set a Progress bar window

   $aLocalDrives = DriveGetDrive( "FIXED" )				; This will scan all fixed drives
   For $lDrv = 1 to $aLocalDrives[0]					; For all the fixed drives
       $lsDrv = StringUpper($aLocalDrives[$lDrv])		; Convert the drives to UPPER case
       ProgressSet(100* $lDrv/$aLocalDrives[0], $lsDrv & " (" & $lDrv & " of " & $aLocalDrives[0] & ")") ; Sets the position of the progress bar window
       sleep(10)										; Sleep
       $lSpaceFree = DriveSpaceFree($lsDrv)				; Set the variable for the free space
       $lSpaceTotal = DriveSpaceTotal($lsDrv)			; Set the variable for the total space
       $nPct = 100 * $lSpaceFree / $lSpaceTotal			; Work out the percentage
	   ; Sets variables to display the output
       $lsOut = $lsOut & $lsDrv & " (Free=" & StringFormat("%.2fMb",$lSpaceFree) & "; Total=" & StringFormat("%.2fMb",$lSpaceTotal) & ")" & @LF
       $lsOut = $lsOut & StringLeft($sBar, $nPct) & " = " & "(" & StringFormat("%.2f",$nPct) & "%)" & @LF & @LF
	Next

   $aNetDrives = DriveGetDrive( "NETWORK" )				; This will scan all the network drives
   For $nDrv = 1 to $aNetDrives[0]
       $sDrv = StringUpper($aNetDrives[$nDrv])
       ProgressSet(100* $nDrv/$aNetDrives[0], $sDrv & " (" & $nDrv & " of " & $aNetDrives[0] & ")")
       sleep(10)
       $nSpaceFree = DriveSpaceFree($sDrv)
       $nSpaceTotal = DriveSpaceTotal($sDrv)
       $nPct = 100 * $nSpaceFree / $nSpaceTotal
       $sOut = $sOut & $sDrv & " (Free=" & StringFormat("%.2fMb",$nSpaceFree) & "; Total=" & StringFormat("%.2fMb",$nSpaceTotal) & ")" & @LF
       $sOut = $sOut & StringLeft($sBar, $nPct) & " = " & "(" & StringFormat("%.2f",$nPct) & "%)" & @LF & @LF
   Next

   ProgressOff()

   ; Output the final figures into a message box
   Msgbox (0,"Free Space (FIXED) and (NETWORK)","Fixed" & @LF & @LF & $lsOut & @LF & "Network Drives" & @LF & @LF & $sOut)