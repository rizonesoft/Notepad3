#Region AutoIt3Wrapper Directives Dection

#AutoIt3Wrapper_If_Run

	;===============================================================================================================
	; AutoIt3 Settings
	;===============================================================================================================
	#AutoIt3Wrapper_UseX64=N										;~ (Y/N) Use AutoIt3_x64 or Aut2Exe_x64. Default=N
	#AutoIt3Wrapper_Run_Debug_Mode=N								;~ (Y/N) Run Script with console debugging. Default=N
	#AutoIt3Wrapper_Run_SciTE_Minimized=Y 							;~ (Y/N) Minimize SciTE while script is running. Default=N
	#AutoIt3Wrapper_Run_SciTE_OutputPane_Minimized=N				;~ (Y/N) Minimize SciTE output pane at run time. Default=N
	;===============================================================================================================
	; Tidy Settings
	;===============================================================================================================
	#AutoIt3Wrapper_Run_Tidy=Y										;~ (Y/N) Run Tidy before compilation. Default=N
	#AutoIt3Wrapper_Tidy_Stop_OnError=Y								;~ (Y/N) Continue when only Warnings. Default=Y
	;#Tidy_Parameters=												;~ Tidy Parameters...see SciTE4AutoIt3 Helpfile for options
	;===============================================================================================================
	; AU3Check settings
	;===============================================================================================================
	#AutoIt3Wrapper_Run_AU3Check=Y									;~ (Y/N) Run au3check before compilation. Default=Y
	;#AutoIt3Wrapper_AU3Check_Parameters=							;~ Au3Check parameters...see SciTE4AutoIt3 Helpfile for options
	;#AutoIt3Wrapper_AU3Check_Stop_OnWarning=						;~ (Y/N) Continue/Stop on Warnings.(Default=N)

#Autoit3Wrapper_If_Compile

	#AutoIt3Wrapper_ShowProgress=Y									;~ (Y/N) Show ProgressWindow during Compile. Default=Y
	;===============================================================================================================
	; AutoIt3 Settings
	;===============================================================================================================
	#AutoIt3Wrapper_UseX64=N										;~ (Y/N) Use AutoIt3_x64 or Aut2Exe_x64. Default=N
	#AutoIt3Wrapper_Version=B                        				;~ (B/P) Use Beta or Production for AutoIt3 and Aut2Eex. Default is P
	#AutoIt3Wrapper_Run_Debug_Mode=N								;~ (Y/N) Run Script with console debugging. Default=N
	;#AutoIt3Wrapper_Autoit3Dir=									;~ Optionally override the AutoIt3 install directory to use.
	;#AutoIt3Wrapper_Aut2exe=										;~ Optionally override the Aut2exe.exe to use for this script
	;#AutoIt3Wrapper_AutoIt3=										;~ Optionally override the Autoit3.exe to use for this script
	;===============================================================================================================
	; Aut2Exe Settings
	;===============================================================================================================
	#AutoIt3Wrapper_Icon=Themes\Icons\Versions.ico					;~ Filename of the Ico file to use for the compiled exe
	#AutoIt3Wrapper_OutFile_Type=exe								;~ exe=Standalone executable (Default); a3x=Tokenised AutoIt3 code file
	#AutoIt3Wrapper_OutFile=Version.exe								;~ Target exe/a3x filename.
	#AutoIt3Wrapper_OutFile_X64=Version_X64.exe						;~ Target exe filename for X64 compile.
	;#AutoIt3Wrapper_Compression=4									;~ Compression parameter 0-4  0=Low 2=normal 4=High. Default=2
	;#AutoIt3Wrapper_UseUpx=Y										;~ (Y/N) Compress output program.  Default=Y
	;#AutoIt3Wrapper_UPX_Parameters=								;~ Override the default settings for UPX.
	#AutoIt3Wrapper_Change2CUI=Y									;~ (Y/N) Change output program to CUI in stead of GUI. Default=N
	#AutoIt3Wrapper_Compile_both=N									;~ (Y/N) Compile both X86 and X64 in one run. Default=N
	;===============================================================================================================
	; Target Program Resource info
	;===============================================================================================================
	#AutoIt3Wrapper_Res_Comment=Version									;~ Comment field
	#AutoIt3Wrapper_Res_Description=Version Incrementer    				;~ Description field
	#AutoIt3Wrapper_Res_Fileversion=1.0.2.553
	#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=Y  					;~ (Y/N/P) AutoIncrement FileVersion. Default=N
	#AutoIt3Wrapper_Res_FileVersion_First_Increment=N					;~ (Y/N) AutoIncrement Y=Before; N=After compile. Default=N
	#AutoIt3Wrapper_Res_HiDpi=Y                      					;~ (Y/N) Compile for high DPI. Default=N
	#AutoIt3Wrapper_Res_ProductVersion=0             					;~ Product Version
	#AutoIt3Wrapper_Res_Language=2057									;~ Resource Language code . Default 2057=English (United Kingdom)
	#AutoIt3Wrapper_Res_LegalCopyright=© 2016 Rizonesoft				;~ Copyright field
	#AutoIt3Wrapper_res_requestedExecutionLevel=None					;~ PasInvoker, highestAvailable, requireAdministrator or None (remove the trsutInfo section).  Default is the setting from Aut2Exe (asInvoker)
	#AutoIt3Wrapper_res_Compatibility=Vista,Win7,Win8,Win81				;~ Vista/Windows7/win7/win8/win81 allowed separated by a comma     (Default=Win81)
	;#AutoIt3Wrapper_Res_SaveSource=N									;~ (Y/N) Save a copy of the Script_source in the EXE resources. Default=N
	; If _Res_SaveSource=Y the content of Script_source depends on the _Run_Au3Stripper and #Au3Stripper_parameters directives:
	;    If _Run_Au3Stripper=Y then
	;        If #Au3Stripper_parameters=/STRIPONLY then Script_source is stripped script & stripped includes
	;        If #Au3Stripper_parameters=/STRIPONLYINCLUDES then Script_source is original script & stripped includes
	;       With any other parameters, the SaveSource directive is ignored as obfuscation is intended to protect the source
	;   If _Run_Au3Stripper=N or is not set then
	;       Scriptsource is original script only
	; AutoIt3Wrapper indicates the SaveSource action taken in the SciTE console during compilation
	; See SciTE4AutoIt3 Helpfile for more detail on Au3Stripper parameters
	;===============================================================================================================
	; Free form resource fields ... max 15
	;===============================================================================================================
	; You can use the following variables:
	;	%AutoItVer% which will be replaced with the version of AutoIt3
	;	%date% = PC date in short date format
	;	%longdate% = PC date in long date format
	;	%time% = PC timeformat
	;	eg: #AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%
	#AutoIt3Wrapper_Res_Field=CompanyName|Rizonesoft
	#AutoIt3Wrapper_Res_Field=ProductName|Versions
	#AutoIt3Wrapper_Res_Field=HomePage|https://rizonesoft.com
	#AutoIt3Wrapper_Res_Field=AutoItVersion|%AutoItVer%
	; Add extra ICO files to the resources
	; Use full path of the ico files to be added
	; ResNumber is a numeric value used to access the icon: TraySetIcon(@ScriptFullPath, ResNumber)
	; If no ResNumber is specified, the added icons are numbered from 201 up
	; #AutoIt3Wrapper_Res_Icon_Add=                   				;~ Filename[,ResNumber[,LanguageCode]] of ICO to be added.
	; #AutoIt3Wrapper_Res_File_Add=                   				;~ Filename[,Section [,ResName[,LanguageCode]]] to be added.
	; Add files to the resources - can be compressed
	; #AutoIt3Wrapper_Res_Remove=
	; Remove resources
	;===============================================================================================================
	; Tidy Settings
	;===============================================================================================================
	#AutoIt3Wrapper_Run_Tidy=Y										;~ (Y/N) Run Tidy before compilation. Default=N
	;#AutoIt3Wrapper_Tidy_Stop_OnError=              				;~ (Y/N) Continue when only Warnings. Default=Y
	;#Tidy_Parameters=                               				;~ Tidy Parameters...see SciTE4AutoIt3 Helpfile for options
	;===============================================================================================================
	; Au3Stripper Settings
	;===============================================================================================================
	#AutoIt3Wrapper_Run_Au3Stripper=Y								;~ (Y/N) Run Au3Stripper before compilation. default=N
	;#Au3Stripper_Parameters=										;~ Au3Stripper parameters...see SciTE4AutoIt3 Helpfile for options
	;#Au3Stripper_Ignore_Variables=
	;===============================================================================================================
	; AU3Check settings
	;===============================================================================================================
	#AutoIt3Wrapper_Run_AU3Check=Y									;~ (Y/N) Run au3check before compilation. Default=Y
	;#AutoIt3Wrapper_AU3Check_Parameters=							;~ Au3Check parameters...see SciTE4AutoIt3 Helpfile for options
	#AutoIt3Wrapper_AU3Check_Stop_OnWarning=Y 						;~ (Y/N) Continue/Stop on Warnings.(Default=N)
	;===============================================================================================================
	; Versioning Settings
	;===============================================================================================================
	;#AutoIt3Wrapper_Versioning=V									;~ (Y/N/V) Run Versioning to update the script source. default=N
	;	V=only run when fileversion is increased by #AutoIt3Wrapper_Res_FileVersion_AutoIncrement.
	;#AutoIt3Wrapper_Versioning_Parameters=/NoPrompt				;~ /NoPrompt  : Will skip the Comments prompt
	;	/Comments  : Text to added in the Comments. It can also contain the below variables.
	;===============================================================================================================
	; RUN BEFORE AND AFTER definitions
	;===============================================================================================================
	;The following directives can contain: these variables
	;	%in% , %out%, %outx64%, %icon% which will be replaced by the fullpath\filename.
	;	%scriptdir% same as @ScriptDir and %scriptfile% = filename without extension.
	;	%fileversion% is the information from the #AutoIt3Wrapper_Res_Fileversion directive
	;	%scitedir% will be replaced by the SciTE program directory
	;	%autoitdir% will be replaced by the AutoIt3 program directory
	;#AutoIt3Wrapper_Run_Before_Admin=               				;~ (Y/N) Run subsequent Run_Before statements with #RequireAdmin. Default=N
	;#AutoIt3Wrapper_Run_After_Admin=                				;~ (Y/N) Run subsequent Run_After statements with #RequireAdmin. Default=N
	;#AutoIt3Wrapper_Run_Before=                     				;~ process to run before compilation - multiple records will be processed in sequence
	;#AutoIt3Wrapper_Run_After=                      				;~ process to run after compilation - multiple records will be processed in sequence
	;===============================================================================================================

#AutoIt3Wrapper_EndIf

#EndRegion AutoIt3Wrapper Directives Dection


#include <MsgBoxConstants.au3>
#include <File.au3>



ConsoleWrite("===========================================================================" & @CRLF)
ConsoleWrite(" Rizonesoft Version Incrementer [version " & FileGetVersion(@ScriptFullPath) & "]" & @CRLF)
ConsoleWrite(" (c) Rizonesoft (https://rizonesoft.com)" & @CRLF)
ConsoleWrite("===========================================================================" & @CRLF)
ConsoleWrite("" & @CRLF)

Local Const $sIniName = @ScriptDir & "\Version.ini"

Local $ConfigOk = True
Local $sVersion = IniRead($sIniName, "Version", "Version", "0.0.0.0")

ConsoleWrite(" Loading version information from: [" & $sIniName & "]" & @CRLF)
ConsoleWrite(" Current version is: " & $sVersion & @CRLF)

Local $aVersion = StringSplit($sVersion, ".")
If Not @error Then

	If IsArray($aVersion) And $aVersion[0] = 4 Then

		ConsoleWrite(" Major = " & $aVersion[1] & " : " & "Minor = " & $aVersion[2] & " : " & _
				"Maintenance = " & $aVersion[3] & " : " & "Build = " & $aVersion[4] & @CRLF)
		ConsoleWrite(" Incrementing Build: " & $aVersion[4])
		$aVersion[4] += 1
		ConsoleWrite(" => " & $aVersion[4] & @CRLF)
		ConsoleWrite(" Building new version string: ")
		$sVersion = $aVersion[1] & "." & $aVersion[2] & "." & $aVersion[3] & "." & $aVersion[4]
		ConsoleWrite($sVersion & @CRLF)

		ConsoleWrite("" & @CRLF)
		ConsoleWrite("===========================================================================" & @CRLF)
		ConsoleWrite(" Loading Templates" & @CRLF)
		ConsoleWrite("===========================================================================" & @CRLF)

		Local $sVariables = "$MAJOR$|$MINOR$|$MAINT$|$BUILD$|$VERSION$"
		Local $aVariables = StringSplit($sVariables, "|")

		Local $aTemplates = IniReadSection($sIniName, "Templates")
		If Not @error Then

			ConsoleWrite(" " & $aTemplates[0][0] & " Templates loaded." & @CRLF)

			For $i = 1 To $aTemplates[0][0]

				; Open the file for reading and store the handle to a variable.
				Local $hFileOpen = FileOpen(@ScriptDir & "\" & $aTemplates[$i][0], $FO_READ)
				If $hFileOpen = -1 Then
					ConsoleWriteError(" The " & $aTemplates[$i][0] & " source file could not be read." & @CRLF)
				EndIf

				ConsoleWrite("" & @CRLF)
				ConsoleWrite("---------------------------------------------------------------------------" & @CRLF)
				ConsoleWrite(" Source: " & $aTemplates[$i][0] & " => Dest: " & $aTemplates[$i][1] & @CRLF)
				ConsoleWrite("---------------------------------------------------------------------------" & @CRLF)

				; Read the contents of the file using the handle returned by FileOpen.
				Local $sFileRead = FileRead($hFileOpen)

				For $x = 1 To 4
					$sFileRead = StringReplace($sFileRead, $aVariables[$x], $aVersion[$x])
					If @extended > 0 Then
						ConsoleWrite(" Replacing: " & $aVariables[$x] & " => " & $aVersion[$x] & @CRLF)
					Else
						ConsoleWrite(" " & $aVariables[$x] & " not found in template." & @CRLF)
					EndIf
				Next

				$sFileRead = StringReplace($sFileRead, $aVariables[5], $sVersion)
				If @extended > 0 Then
					ConsoleWrite(" Replacing: " & $aVariables[5] & " => " & $sVersion & @CRLF)
				Else
					ConsoleWrite(" " & $aVariables[5] & " not found in template." & @CRLF)
				EndIf

				; Close the handle returned by FileOpen.
				FileClose($hFileOpen)

				ConsoleWrite("" & @CRLF)
				ConsoleWrite(" Updating: " & $aTemplates[$i][1] & @CRLF)

				FileDelete(@ScriptDir & "\" & $aTemplates[$i][1])

				If Not FileWrite(@ScriptDir & "\" & $aTemplates[$i][1], $sFileRead) Then
					ConsoleWriteError(" ERROR: Could not write data to [" & $aTemplates[$i][1] & "]" & @CRLF)
				EndIf

				ConsoleWrite("---------------------------------------------------------------------------" & @CRLF)

			Next

		Else

			ConsoleWriteError(" ERROR: The version setting does not seem to be valid! " & _
					"Please use the following format: 0.0.0.0 " & @CRLF)
		EndIf

	Else

		ConsoleWriteError(" ERROR: The version setting does not seem to be valid! " & _
				"Please use the following format: 0.0.0.0 " & @CRLF)

	EndIf

EndIf

ConsoleWrite("" & @CRLF)
ConsoleWrite(" Storing new version: " & $sVersion & @CRLF)
IniWrite($sIniName, "Version", "Version", $sVersion)
ConsoleWrite("===========================================================================" & @CRLF)
ConsoleWrite("" & @CRLF)

Sleep(1000)
