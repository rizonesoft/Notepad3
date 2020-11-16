/* Copyright 2004-2010 PortableApps.com
 * Website: https://portableapps.com/development
 * Main developer and contact: Chris Morgan
 *
 * This software is OSI Certified Open Source Software.
 * OSI Certified is a certification mark of the Open Source Initiative.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

!verbose 3

!ifndef PACKAGE
	!define PACKAGE ..\..
!endif

!macro !echo msg
	!verbose push
	!verbose 4
	!echo "${msg}"
	!verbose pop
!macroend
!define !echo "!insertmacro !echo"

;=== Require at least Unicode NSIS 2.46 {{{1
!include RequireLatestNSIS.nsh

;=== Runtime Switches {{{1
WindowIcon Off
XPStyle on
SilentInstall Silent
AutoCloseWindow True
!ifdef RUNASADMIN_COMPILEFORCE
RequestExecutionLevel admin
!else
RequestExecutionLevel user
!endif
SetCompressor /SOLID lzma
SetCompressorDictSize 32

;=== Include {{{1
${!echo} "Including required files..."
;(Standard NSIS) {{{2
!include LangFile.nsh
!include LogicLib.nsh
!include FileFunc.nsh
!include TextFunc.nsh
!include WordFunc.nsh

;(NSIS Plugins) {{{2
!include NewTextReplace.nsh
!addplugindir Plugins

;(Custom) {{{2
!include ReplaceInFileWithTextReplace.nsh
!include ForEachINIPair.nsh
!include ForEachPath.nsh
!include SetFileAttributesDirectoryNormal.nsh
!include ProcFunc.nsh
!include EmptyWorkingSet.nsh
!include SetEnvironmentVariable.nsh
!include CheckForPlatformSplashDisable.nsh
!include LogicLibAdditions.nsh

;=== Languages {{{1
${!echo} "Loading language strings..."
!include Languages.nsh

;=== Variables {{{1
${!echo} "Initialising variables and macros..."
Var AppID
Var BaseName
Var MissingFileOrPath
Var AppNamePortable
Var AppName
Var ProgramExecutable
Var StatusMutex
Var WaitForProgram

; Macro: read a value from the launcher configuration file {{{1
!macro ReadLauncherConfig _OUTPUT _SECTION _VALUE
	ReadINIStr ${_OUTPUT} $LauncherFile ${_SECTION} ${_VALUE}
!macroend
!define ReadLauncherConfig "!insertmacro ReadLauncherConfig"

!macro ReadLauncherConfigWithDefault _OUTPUT _SECTION _VALUE _DEFAULT
	ClearErrors
	${ReadLauncherConfig} ${_OUTPUT} `${_SECTION}` `${_VALUE}`
	${IfThen} ${Errors} ${|} StrCpy ${_OUTPUT} `${_DEFAULT}` ${|}
!macroend
!define ReadLauncherConfigWithDefault "!insertmacro ReadLauncherConfigWithDefault"

!macro ReadUserConfig _OUTPUT _VALUE
	;ReadINIStr ${_OUTPUT} $EXEDIR\$BaseName.ini $BaseName ${_VALUE}
	${ConfigRead} $EXEDIR\$BaseName.ini ${_VALUE}= ${_OUTPUT}
!macroend
!define ReadUserConfig "!insertmacro ReadUserConfig"

; Better to keep people away from the name completely. The Generator updates references in Custom.nsh when moving the file, anyway.
!macro ReadUserOverrideConfigError a b
	!error `ReadUserOverrideConfig has been renamed to ReadUserConfig in PAL 2.1.`
!macroend
!define ReadUserOverrideConfig "!insertmacro ReadUserOverrideConfigError"

!macro InvalidValueError _SECTION_KEY _VALUE
	MessageBox MB_OK|MB_ICONSTOP "Error: invalid value '${_VALUE}' for ${_SECTION_KEY}. Please refer to the Manual for valid values."
!macroend
!define InvalidValueError "!insertmacro InvalidValueError"

; Macros: runtime data read/write {{{1
; The runtime data file is mirrored on disk and locally so that if the disk is
; removed it should still be able to clean up and know about things like
; registry keys that failed.
!macro WriteRuntimeData Section Key Value
	WriteINIStr $DataDirectory\PortableApps.comLauncherRuntimeData-$BaseName.ini ${Section} ${Key} ${Value}
	WriteINIStr $PLUGINSDIR\runtimedata.ini ${Section} ${Key} ${Value}
!macroend
!define WriteRuntimeData "!insertmacro WriteRuntimeData"

!macro ReadRuntimeData Output Section Key
	IfFileExists $DataDirectory\PortableApps.comLauncherRuntimeData-$BaseName.ini 0 +3
	ReadINIStr ${Output} $DataDirectory\PortableApps.comLauncherRuntimeData-$BaseName.ini ${Section} ${Key}
	Goto +2
	ReadINIStr ${Output} $PLUGINSDIR\runtimedata.ini ${Section} ${Key}
!macroend
!define ReadRuntimeData "!insertmacro ReadRuntimeData"

; Load the segments {{{1
${!echo} "Loading segments..."
!include Segments.nsh

;=== Debugging {{{1
!include Debug.nsh

;=== Program Details {{{1
${!echo} "Specifying program details and setting options..."

Name "${NamePortable} (PortableApps.com Launcher)"
OutFile "${PACKAGE}\${AppID}.exe"
Icon "${PACKAGE}\App\AppInfo\appicon.ico"
Caption "${NamePortable} (PortableApps.com Launcher)"
VIProductVersion ${Version}
VIAddVersionKey ProductName "${NamePortable}"
VIAddVersionKey Comments "A build of the PortableApps.com Launcher for ${NamePortable}, allowing it to be run from a removable drive.  For additional details, visit PortableApps.com"
VIAddVersionKey CompanyName PortableApps.com
VIAddVersionKey LegalCopyright PortableApps.com
VIAddVersionKey FileDescription "${NamePortable} (PortableApps.com Launcher)"
VIAddVersionKey FileVersion ${Version}
VIAddVersionKey ProductVersion ${Version}
VIAddVersionKey InternalName "PortableApps.com Launcher"
VIAddVersionKey LegalTrademarks "PortableApps.com is a Trademark of Rare Ideas, LLC."
VIAddVersionKey OriginalFilename "${AppID}.exe"

!verbose 4

Function .onInit          ;{{{1
	${RunSegment} Custom
	${RunSegment} Core
	${RunSegment} Temp
	${RunSegment} Language
	${RunSegment} OperatingSystem
	${RunSegment} RunAsAdmin
FunctionEnd

Function Init             ;{{{1
	${RunSegment} Custom
	${RunSegment} Core
	${RunSegment} PathChecks
	${RunSegment} Settings
	${RunSegment} DriveLetter
	${RunSegment} DirectoryMoving
	${RunSegment} Variables
	${RunSegment} Language
	${RunSegment} Registry
	${RunSegment} Java
	${RunSegment} RunLocally
	${RunSegment} Temp
	${RunSegment} InstanceManagement
	${RunSegment} SplashScreen
	${RunSegment} RefreshShellIcons
FunctionEnd

Function Pre              ;{{{1
	${RunSegment} Custom
	${RunSegment} RunLocally
	${RunSegment} Temp
	${RunSegment} Environment
	${RunSegment} ExecString
FunctionEnd

Function PrePrimary       ;{{{1
	${RunSegment} Custom
	${RunSegment} DriveLetter
	${RunSegment} Variables
	${RunSegment} DirectoryMoving
	${RunSegment} FileWrite
	${RunSegment} FilesMove
	${RunSegment} DirectoriesMove
	;${RunSegment} RegisterDLL
	${RunSegment} RegistryKeys
	${RunSegment} RegistryValueBackupDelete
	${RunSegment} RegistryValueWrite
	${RunSegment} Services
FunctionEnd

Function PreSecondary     ;{{{1
	${RunSegment} Custom
	;${RunSegment} *
FunctionEnd

Function PreExec          ;{{{1
	${RunSegment} Custom
	${RunSegment} RefreshShellIcons
	${RunSegment} WorkingDirectory
FunctionEnd

Function PreExecPrimary   ;{{{1
	${RunSegment} Custom
	${RunSegment} Core
	${RunSegment} SplashScreen
FunctionEnd

Function PreExecSecondary ;{{{1
	${RunSegment} Custom
	;${RunSegment} *
FunctionEnd

Function Execute          ;{{{1
	; Users can override this function in Custom.nsh
	; like this (see Segments.nsh for the OverrideExecute define):
	;
	;   ${OverrideExecute}
	;       [code to replace this function]
	;   !macroend

	!ifmacrodef OverrideExecuteFunction
		!insertmacro OverrideExecuteFunction
	!else
	${!getdebug}
	!ifdef DEBUG
		${If} $WaitForProgram != false
			${DebugMsg} "About to execute the following string and wait till it's done: $ExecString"
		${Else}
			${DebugMsg} "About to execute the following string and finish: $ExecString"
		${EndIf}
	!endif
	${EmptyWorkingSet}
	ClearErrors
	${ReadLauncherConfig} $0 Launch HideCommandLineWindow
	${If} $0 == true
		; TODO: do this without a plug-in or at least some way it won't wait with secondary
		ExecDos::exec $ExecString
		Pop $0
	${Else}
		${IfNot} ${Errors}
		${AndIf} $0 != false
			${InvalidValueError} [Launch]:HideCommandLineWindow $0
		${EndIf}
		${If} $WaitForProgram != false
			ExecWait $ExecString
		${Else}
			Exec $ExecString
		${EndIf}
	${EndIf}
	${DebugMsg} "$ExecString has finished."

	${If} $WaitForProgram != false
		; Wait till it's done
		ClearErrors
		${ReadLauncherConfig} $0 Launch WaitForOtherInstances
		${If} $0 == true
		${OrIf} ${Errors}
			${GetFileName} $ProgramExecutable $1
			${DebugMsg} "Waiting till any other instances of $1 and any [Launch]:WaitForEXE[N] values are finished."
			${EmptyWorkingSet}
			${Do}
				${ProcessWaitClose} $1 -1 $R9
				${IfThen} $R9 > 0 ${|} ${Continue} ${|}
				StrCpy $0 1
				${Do}
					ClearErrors
					${ReadLauncherConfig} $2 Launch WaitForEXE$0
					${IfThen} ${Errors} ${|} ${ExitDo} ${|}
					${ProcessWaitClose} $2 -1 $R9
					${IfThen} $R9 > 0 ${|} ${ExitDo} ${|}
					IntOp $0 $0 + 1
				${Loop}
			${LoopWhile} $R9 > 0
			${DebugMsg} "All instances are finished."
		${ElseIf} $0 != false
			${InvalidValueError} [Launch]:WaitForOtherInstances $0
		${EndIf}
	${EndIf}
	!endif
FunctionEnd

Function PostPrimary      ;{{{1
	${RunSegment} Services
	${RunSegment} RegistryValueBackupDelete
	${RunSegment} RegistryKeys
	${RunSegment} RegistryCleanup
	;${RunSegment} RegisterDLL
	${RunSegment} Qt
	${RunSegment} DirectoriesMove
	${RunSegment} FilesMove
	${RunSegment} DirectoriesCleanup
	${RunSegment} RunLocally
	${RunSegment} Temp
	${RunSegment} Custom
FunctionEnd

Function PostSecondary    ;{{{1
	;${RunSegment} *
	${RunSegment} Custom
FunctionEnd

Function Post             ;{{{1
	${RunSegment} RefreshShellIcons
	${RunSegment} Custom
FunctionEnd

Function Unload           ;{{{1
	${RunSegment} XML
	${RunSegment} Registry
	${RunSegment} SplashScreen
	${RunSegment} Core
	${RunSegment} Custom
FunctionEnd

; Call a segment-calling function with primary/secondary variants as well {{{1
!macro CallPS _func _rev
	!if ${_rev} == +
		Call ${_func}
	!endif
	${If} $SecondaryLaunch == true
		Call ${_func}Secondary
	${Else}
		Call ${_func}Primary
	${EndIf}
	!if ${_rev} != +
		Call ${_func}
	!endif
!macroend
!define CallPS `!insertmacro CallPS`

Section           ;{{{1
	Call Init

	System::Call 'Kernel32::OpenMutex(i1048576, b0, t"PortableApps.comLauncher$AppID-$BaseName::Starting") i.R0 ?e'
	System::Call 'Kernel32::CloseHandle(iR0)'
	Pop $R9
	${If} $R9 <> 2
		MessageBox MB_ICONSTOP $(LauncherAlreadyStarting)
		Quit
	${EndIf}
	System::Call 'Kernel32::OpenMutex(i1048576, i0, t"PortableApps.comLauncher$AppID-$BaseName::Stopping") i.R0 ?e'
	System::Call 'Kernel32::CloseHandle(iR0)'
	Pop $R9
	${If} $R9 <> 2
		MessageBox MB_ICONSTOP $(LauncherAlreadyStopping)
		Quit
	${EndIf}

	${IfNot} ${FileExists} $DataDirectory\PortableApps.comLauncherRuntimeData-$BaseName.ini
	${OrIf} $SecondaryLaunch == true
		${If} $SecondaryLaunch != true
			System::Call 'Kernel32::CreateMutex(i0, i0, t"PortableApps.comLauncher$AppID-$BaseName::Starting") i.r0'
			StrCpy $StatusMutex $0
		${EndIf}
		${CallPS} Pre +
		${CallPS} PreExec +
		${If} $SecondaryLaunch != true
			StrCpy $0 $StatusMutex
			System::Call 'Kernel32::CloseHandle(ir0) ?e'
			Pop $R9
		${EndIf}
		; File gets deleted in segment Core, hook Unload, so it'll only exist
		; in case of power-outage, disk removal while running or something like that.
		Call Execute
	${Else}
		; After doing Post, we don't do restart automatically as the variables
		; and environment are all altered and this may affect what happens
		; (some variables are checked against "" rather than initialising every
		; variable, and some may depend on environment variables, so spawing a
		; new instance isn't safe either)
		MessageBox MB_ICONSTOP $(LauncherCrashCleanup)
		; One possible solution: ExecWait another copy of self to do cleanup
	${EndIf}
	${If} $SecondaryLaunch != true
		System::Call 'Kernel32::CreateMutex(i0, i0, t"PortableApps.comLauncher$AppID-$BaseName::Stopping")'
	${EndIf}
	${If} $WaitForProgram != false
		${CallPS} Post -
	${EndIf}
	Call Unload
SectionEnd

Function .onInstFailed ;{{{1
	; If Abort is called
	Call Unload
FunctionEnd ;}}}1

; This file has been optimised for use in Vim with folding.
; (If you can't cope, :set nofoldenable) vim:foldenable:foldmethod=marker
