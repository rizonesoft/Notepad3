/*
_____________________________________________________________________________

                      Process Functions Header v2.2
_____________________________________________________________________________

 2008-2010 Erik Pilsits aka wraithdu
 License: zlib/libpng

 See documentation for more information about the following functions.

 Usage in script:
 1. !include "ProcFunc.nsh"
 2. [Section|Function]
      ${ProcFunction} "Param1" "Param2" "..." $var
    [SectionEnd|FunctionEnd]


 ProcFunction=[GetProcessPID|GetProcessPath|GetProcessParent|GetProcessName|
               EnumProcessPaths|ProcessWait|ProcessWait2|ProcessWaitClose|
               CloseProcess|TerminateProcess|Execute]

 There is also a LogicLib extension:
    ${If} ${ProcessExists} file.exe
      ...
    ${EndIf}

_____________________________________________________________________________

                       Thanks to:
_____________________________________________________________________________

Some functions based on work by Donald Miller and Phoenix1701@gmail.com

_____________________________________________________________________________

                       Individual documentation:
_____________________________________________________________________________

${ProcessExists} "[process]"
	"[process]"			; Name or PID

	Use with a LogicLib conditional command like If or Unless.
	Evaluates to true if the process exists or false if it does not or
	the CreateToolhelp32Snapshot fails.

${GetProcessPID} "[process]" $var
	"[process]"			; Name or PID
	
	$var(output)		; -2 - CreateToolhelp32Snapshot failed
						;  0 - process does not exist
						; >0 - PID

${GetProcessPath} "[process]" $var
	"[process]"			; Name or PID
	
	$var(output)		; -2 - CreateToolhelp32Snapshot failed
						; -1 - OpenProcess failed
						;  0 - process does not exist
						; Or path to process

${GetProcessParent} "[process]" $var
	"[process]"			; Name or PID

	$var(output)		; -2 - CreateToolhelp32Snapshot failed
						;  0 - process does not exist
						; Or PPID

${GetProcessName} "[PID]" $var
	"[PID]"				; PID

	$var(output)		; -2 - CreateToolhelp32Snapshot failed
						;  0 - process does not exist
						; Or process name

${EnumProcessPaths} "Function" $var
	"Function"          ; Callback function
	$var(output)		; -2 - EnumProcesses failed
						;  1 - success

	Function "Function"
		Pop $var1			; matching path string
		Pop $var2			; matching process PID
		...user commands
		Push [1/0]			; must return 1 on the stack to continue
							; must return some value or corrupt the stack
							; DO NOT save data in $0-$9
	FunctionEnd

${ProcessWait} "[process]" "[timeout]" $var
	"[process]"			; Name
	"[timeout]"			; -1 - do not timeout
						; >0 - timeout in milliseconds

	$var(output)		; -2 - CreateToolhelp32Snapshot failed
						; -1 - operation timed out
						; Or PID

${ProcessWait2} "[process]" "[timeout]" $var
	"[process]"			; Name
	"[timeout]"			; -1 - do not timeout
						; >0 - timeout in milliseconds

	$var(output)		; -1 - operation timed out
						; Or PID

${ProcessWaitClose} "[process]" "[timeout]" $var
	"[process]"			; Name
	"[timeout]"			; -1 - do not timeout
						; >0 - timeout in milliseconds

	$var(output)		; -1 - operation timed out
						;  0 - process does not exist
						; Or PID of ended process

${CloseProcess} "[process]" $var
	"[process]"			; Name or PID

	$var(output)		; 0 - process does not exist
						; Or PID of ended process

${TerminateProcess} "[process]" $var
	"[process]"			; Name or PID

	$var(output)		; -1 - operation failed
						;  0 - process does not exist
						; Or PID of ended process

${Execute} "[command]" "[working_dir]" $var
	"[command]"			; '"X:\path\to\prog.exe" arg1 arg2 "arg3 with space"'
	"[working_dir]"		; Working directory ("X:\path\to\dir") or nothing ("")

	$var(output)		; 0 - failed to create process
						; Or PID
*/


;_____________________________________________________________________________
;
;                         Macros
;_____________________________________________________________________________
;
; Change log window verbosity (default: 3=no script)
;
; Example:
; !include "ProcFunc.nsh"
; ${PROCFUNC_VERBOSE} 4   # all verbosity
; ${PROCFUNC_VERBOSE} 3   # no script

!ifndef PROCFUNC_INCLUDED
!define PROCFUNC_INCLUDED

!include Util.nsh
!include LogicLib.nsh

!verbose push
!verbose 3
!ifndef _PROCFUNC_VERBOSE
	!define _PROCFUNC_VERBOSE 3
!endif
!verbose ${_PROCFUNC_VERBOSE}
!define PROCFUNC_VERBOSE `!insertmacro PROCFUNC_VERBOSE`
!verbose pop

!macro PROCFUNC_VERBOSE _VERBOSE
	!verbose push
	!verbose 3
	!undef _PROCFUNC_VERBOSE
	!define _PROCFUNC_VERBOSE ${_VERBOSE}
	!verbose pop
!macroend

!define PROCESS_QUERY_INFORMATION 0x0400
!define PROCESS_TERMINATE 0x0001
!define PROCESS_VM_READ 0x0010
!define SYNCHRONIZE 0x00100000

!define WAIT_TIMEOUT 0x00000102

!ifdef NSIS_UNICODE
	!define _PROCFUNC_WSTRING "&w260"
!else
	!define _PROCFUNC_WSTRING "&w520"
!endif

!macro ProcessExists
	!error "ProcessExists has been renamed to GetProcessPID"
!macroend
!macro _ProcessExists _a _b _t _f
	!insertmacro _LOGICLIB_TEMP
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push `${_b}`
	${CallArtificialFunction} LLProcessExists_
	IntCmp $_LOGICLIB_TEMP 0 `${_f}`
	Goto `${_t}`
	!verbose pop
!macroend
!define ProcessExists `"" ProcessExists`

!macro GetProcessPID
!macroend
!define GetProcessPID "!insertmacro GetProcessPIDCall"
!macro GetProcessPIDCall process outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push 0
	Push `${process}`
	!ifdef CallArtificialFunction_TYPE ; macro nesting disallowed, breaks otherwise if used from WaitClose
	${CallArtificialFunction2} ProcFuncs_
	!else
	${CallArtificialFunction} ProcFuncs_
	!endif
	Pop ${outVar}
	!verbose pop
!macroend

!macro GetProcessPath
!macroend
!define GetProcessPath "!insertmacro GetProcessPathCall"
!macro GetProcessPathCall process outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push 1
	Push `${process}`
	${CallArtificialFunction} ProcFuncs_
	Pop ${outVar}
	!verbose pop
!macroend

!macro GetProcessParent
!macroend
!define GetProcessParent "!insertmacro GetProcessParentCall"
!macro GetProcessParentCall process outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push 2
	Push `${process}`
	${CallArtificialFunction} ProcFuncs_
	Pop ${outVar}
	!verbose pop
!macroend

!macro GetProcessName
!macroend
!define GetProcessName "!insertmacro GetProcessNameCall"
!macro GetProcessNameCall process outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push 6
	Push `${process}`
	${CallArtificialFunction} ProcFuncs_
	Pop ${outVar}
	!verbose pop
!macroend

!macro EnumProcessPaths
!macroend
!define EnumProcessPaths "!insertmacro EnumProcessPathsCall"
!macro EnumProcessPathsCall user_func outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push $0
	GetFunctionAddress $0 `${user_func}`
	Push `$0`
	${CallArtificialFunction} EnumProcessPaths_
	Exch
	Pop $0
	Pop ${outVar}
	!verbose pop
!macroend

!macro ProcessWait
!macroend
!define ProcessWait "!insertmacro ProcessWaitCall"
!macro ProcessWaitCall process timeout outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push `${timeout}`
	Push `${process}`
	${CallArtificialFunction} ProcessWait_
	Pop ${outVar}
	!verbose pop
!macroend

!macro ProcessWait2
!macroend
!define ProcessWait2 "!insertmacro ProcessWait2Call"
!macro ProcessWait2Call process timeout outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push `${timeout}`
	Push `${process}`
	${CallArtificialFunction} ProcessWait2_
	Pop ${outVar}
	!verbose pop
!macroend

!macro ProcessWaitClose
!macroend
!define ProcessWaitClose "!insertmacro ProcessWaitCloseCall"
!macro ProcessWaitCloseCall process timeout outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push `${timeout}`
	Push `${process}`
	${CallArtificialFunction} ProcessWaitClose_
	Pop ${outVar}
	!verbose pop
!macroend

!macro CloseProcess
!macroend
!define CloseProcess "!insertmacro CloseProcessCall"
!macro CloseProcessCall process outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push `${process}`
	${CallArtificialFunction} CloseProcess_
	Pop ${outVar}
	!verbose pop
!macroend

!macro TerminateProcess
!macroend
!define TerminateProcess "!insertmacro TerminateProcessCall"
!macro TerminateProcessCall process outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push `${process}`
	${CallArtificialFunction} TerminateProcess_
	Pop ${outVar}
	!verbose pop
!macroend

!macro Execute
!macroend
!define Execute "!insertmacro ExecuteCall"
!macro ExecuteCall cmdline wrkdir outVar
	!verbose push
	!verbose ${_PROCFUNC_VERBOSE}
	Push `${wrkdir}`
	Push `${cmdline}`
	${CallArtificialFunction} Execute_
	Pop ${outVar}
	!verbose pop
!macroend

!macro ProcFuncs_
	System::Store "s" ; store registers in System's private stack
	Pop $0 ; process / PID
	Pop $1 ; mode
	
	Push 0 ; set return value if not found
	
	; set mode of operation in $1
	${Select} $1 ; mode 0 = GetProcessPID, mode 1 = GetProcessPath, mode 2 = GetProcessParent
		${Case} 0
			StrCpy $2 $0 4 -4
			${If} $2 == ".exe"
				; exists from process name
				StrCpy $1 0
			${Else}
				; exists from pid
				StrCpy $1 1
			${EndIf}
		${Case} 1
			StrCpy $2 $0 4 -4
			${If} $2 == ".exe"
				; get path from process name
				StrCpy $1 2
			${Else}
				; get path from pid
				StrCpy $1 3
			${EndIf}
		${Case} 2
			StrCpy $2 $0 4 -4
			${If} $2 == ".exe"
				; get parent from process name
				StrCpy $1 4
			${Else}
				; get parent from pid
				StrCpy $1 5
			${EndIf}
	${EndSelect}
	
	System::Call '*(&l4,i,i,i,i,i,i,i,i,${_PROCFUNC_WSTRING})i .r2' ; $2 = PROCESSENTRY32W structure
	; take system process snapshot in $3
	System::Call 'kernel32::CreateToolhelp32Snapshot(i 2, i 0)i .r3'
	${Unless} $3 = -1
		System::Call 'kernel32::Process32FirstW(i r3, i r2)i .r4'
		${Unless} $4 = 0
			${Do}
				${Select} $1
					${Case3} 0 2 4
						; get process name in $5
						System::Call '*$2(i,i,i,i,i,i,i,i,i,${_PROCFUNC_WSTRING} .r5)'
					${Case4} 1 3 5 6
						; get process PID in $5
						System::Call '*$2(i,i,i .r5)'
				${EndSelect}
				; is this process the one we are looking for?
				${If} $5 == $0 ; string test works ok for numeric PIDs as well
					${Select} $1 ; mode 0/1 = GetProcessPID, mode 2/3 = GetProcessPath, mode 4/5 = GetProcessParent, mode 6 = GetProcessName
						${Case2} 0 1
							; return pid
							Pop $5 ; old return value
							System::Call '*$2(i,i,i .s)'; process pid to stack
						${Case2} 2 3
							; return full path
							Pop $5
							; open process
							System::Call '*$2(i,i,i .s)'; process pid to stack
							System::Call 'kernel32::OpenProcess(i ${PROCESS_QUERY_INFORMATION}|${PROCESS_VM_READ}, i 0, i s)i .r5' ; process handle to $5
							${Unless} $5 = 0
								; full path to stack
								System::Call 'psapi::GetModuleFileNameExW(i r5, i 0, w .s, i ${NSIS_MAX_STRLEN})'
								System::Call 'kernel32::CloseHandle(i r5)'
							${Else}
								Push -1 ; OpenProcess failure return value
							${EndUnless}
						${Case2} 4 5
							; return parent PID
							Pop $5
							System::Call '*$2(i,i,i,i,i,i,i .s)'; parent pid to stack
						${Case} 6
							; return base name
							Pop $5
							System::Call '*$2(i,i,i,i,i,i,i,i,i,${_PROCFUNC_WSTRING} .s)'
					${EndSelect}
					${Break}
				${EndIf}
				System::Call 'kernel32::Process32NextW(i r3, i r2)i .r4'
			${LoopUntil} $4 = 0
			System::Call 'kernel32::CloseHandle(i r3)' ; close snapshot
		${EndUnless}
	${Else}
		Pop $5
		Push -2 ; function failure return value
	${EndUnless}
	System::Free $2 ; free buffer
	
	System::Store "l" ; restore registers
!macroend

!macro EnumProcessPaths_
	System::Store "s" ; store registers in System's private stack
	Pop $0 ; user_func
	
	StrCpy $1 1 ; OK to loop
	
	System::Alloc 1024
	Pop $2 ; process list buffer
	; get an array of all process ids
	System::Call 'psapi::EnumProcesses(i r2, i 1024, *i .r3)i .r4' ; $3 = sizeof buffer
	${Unless} $4 = 0
		IntOp $3 $3 / 4 ; Divide by sizeof(DWORD) to get $3 process count
		IntOp $3 $3 - 1 ; decrement for 0 base loop
		${For} $4 0 $3
			${IfThen} $1 != 1 ${|} ${Break} ${|}
			; get a PID from the array
			IntOp $5 $4 * 4 ; calculate offset
			IntOp $5 $5 + $2 ; add offset to original buffer address
			System::Call '*$5(i .r5)' ; get next PID = $5
			${Unless} $5 = 0
				System::Call 'kernel32::OpenProcess(i ${PROCESS_QUERY_INFORMATION}|${PROCESS_VM_READ}, i 0, i r5)i .r6'
				${Unless} $6 = 0 ; $6 is hProcess
					; get full path
					System::Call 'psapi::GetModuleFileNameExW(i r6, i 0, w .r7, i ${NSIS_MAX_STRLEN})i .r8' ; $7 = path
					${Unless} $8 = 0 ; no path
						System::Store "s" ; store registers in System's private stack
						Push $5 ; PID to stack
						Push $7 ; path to stack
						Call $0 ; user func must return 1 on the stack to continue looping
						System::Store "l" ; restore registers
						Pop $1 ; continue?
					${EndUnless}
					System::Call 'kernel32::CloseHandle(i r6)'
				${EndUnless}
			${EndUnless}
		${Next}
		Push 1 ; return value
	${Else}
		Push -2 ; function failure return value
	${EndUnless}
	System::Free $2 ; free buffer
	
	System::Store "l" ; restore registers
!macroend

!macro ProcessWait_
	System::Store "s" ; store registers in System's private stack
	Pop $0 ; process
	Pop $1 ; timeout
	
	StrCpy $6 1 ; initialize loop
	StrCpy $7 0 ; initialize timeout counter
	
	System::Call '*(&l4,i,i,i,i,i,i,i,i,${_PROCFUNC_WSTRING})i .r2' ; $2 = PROCESSENTRY32W structure
	${DoWhile} $6 = 1 ; processwait loop
		; take system process snapshot in $3
		System::Call 'kernel32::CreateToolhelp32Snapshot(i 2, i 0)i .r3'
		${Unless} $3 = -1
			System::Call 'kernel32::Process32FirstW(i r3, i r2)i .r4'
			${Unless} $4 = 0
				${Do}
					; get process name in $5
					System::Call '*$2(i,i,i,i,i,i,i,i,i,${_PROCFUNC_WSTRING} .r5)'
					${If} $5 == $0
						; exists, return pid
						System::Call '*$2(i,i,i .s)'; process pid to stack ; process pid
						StrCpy $6 0 ; end loop
						${Break}
					${EndIf}
					System::Call 'kernel32::Process32NextW(i r3, i r2)i .r4'
				${LoopUntil} $4 = 0
				System::Call 'kernel32::CloseHandle(i r3)' ; close snapshot
			${EndUnless}
		${Else}
			Push -2
			${Break}
		${EndUnless}
		; timeout loop
		${If} $6 = 1
			${If} $1 >= 0
				IntOp $7 $7 + 500 ; increment timeout counter
			${AndIf} $7 >= $1 ; timed out, break loop
				Push -1 ; timeout return value
				${Break} ; end loop if timeout
			${EndIf}
			Sleep 500 ; pause before looping
		${EndIf}
	${Loop} ; processwaitloop
	System::Free $2 ; free buffer
	
	System::Store "l" ; restore registers
!macroend

!macro ProcessWait2_
	System::Store "s" ; store registers in System's private stack
	System::Store "P0" ; FindProcDLL return value
	Pop $0 ; process
	Pop $1 ; timeout
	
	StrCpy $2 0 ; initialize timeout counter
	
	${Do}
		FindProcDLL::FindProc $0
		${IfThen} $R0 = 1 ${|} ${Break} ${|}
		${If} $1 >= 0
			IntOp $2 $2 + 250
		${AndIf} $2 >= $1
			Push -1 ; timeout return value
			${Break}
		${EndIf}
		Sleep 250
	${Loop}
	
	${If} $R0 = 1 ; success, get pid
		${GetProcessPID} $0 $0
		Push $0 ; return pid
	${EndIf}
	
	System::Store "R0" ; restore registers
	System::Store "l"
!macroend

!macro ProcessWaitClose_
	System::Store "s" ; store registers in System's private stack
	Pop $0 ; process / PID
	Pop $1 ; timeout
	
	; passed process name or pid
	StrCpy $2 $0 4 -4
	${If} $2 == ".exe"
		${GetProcessPID} $0 $0
	${EndIf}
	
	; else passed pid directly
	
	${Unless} $0 = 0
		System::Call 'kernel32::OpenProcess(i ${SYNCHRONIZE}, i 0, i r0)i .r2'
		${Unless} $2 = 0 ; $2 is hProcess
			System::Call 'kernel32::WaitForSingleObject(i r2, i $1)i .r1'
			${If} $1 = ${WAIT_TIMEOUT}
				Push -1 ; timed out
			${Else}
				Push $0 ; return pid of ended process
			${EndIf}
			System::Call 'kernel32::CloseHandle(i r2)'
		${Else}
			Push 0 ; failure return value
		${EndUnless}
	${Else}
		Push 0 ; failure return value
	${EndUnless}
	
	System::Store "l" ; restore registers
!macroend

!macro CloseProcess_
	System::Store "s" ; store registers in System's private stack
	Pop $0 ; process / PID
	
	; passed process name or pid
	StrCpy $1 $0 4 -4
	${If} $1 == ".exe"
		${GetProcessPID} $0 $0
	${EndIf}
	
	; else passed pid directly
	
	${Unless} $0 = 0 ; $0 = target pid
		Push $0 ; return pid of process
		; use EnumWindows and a callback
		System::Get '(i .r1, i)i sr4' ; $1 = hwnd, $4 = callback#, s (stack) = source for return value
		Pop $3 ; $3 = callback address
		System::Call 'user32::EnumWindows(k r3, i)i' ; enumerate top-level windows
		${DoWhile} $4 == "callback1"
			System::Call 'user32::GetWindowThreadProcessId(i r1, *i .r2)i' ; $2 = pid that created the window
			${If} $2 = $0 ; match to target pid
				SendMessage $1 16 0 0 /TIMEOUT=1  ; send WM_CLOSE to all top-level windows owned by process, timeout immediately
			${EndIf}
			Push 1 ; callback return value; keep enumerating windows (returning 0 stops)
			StrCpy $4 "" ; clear callback#
			System::Call '$3' ; return from callback
		${Loop}
		System::Free $3 ; free callback
	${Else}
		Push 0 ; failure return value
	${EndUnless}
	
	System::Store "l" ; restore registers
!macroend

!macro TerminateProcess_
	System::Store "s" ; store registers in System's private stack
	Pop $0 ; process / PID
	
	; passed process name or pid
	StrCpy $1 $0 4 -4
	${If} $1 == ".exe"
		${GetProcessPID} $0 $0
	${EndIf}
	
	; else passed pid directly
	
	${Unless} $0 = 0
		System::Call 'kernel32::OpenProcess(i ${PROCESS_TERMINATE}, i 0, i r0)i .r1'
		${Unless} $1 = 0 ; $1 is hProcess
			System::Call 'kernel32::TerminateProcess(i r1, i 0)i .r1'
			${If} $1 = 0 ; fail
				Push -1
			${Else}
				Push $0 ; return pid of ended process
			${EndIf}
			System::Call 'kernel32::CloseHandle(i r1)'
		${Else}
			Push 0 ; failure return value
		${EndUnless}
	${Else}
		Push 0 ; failure return value
	${EndUnless}
	
	System::Store "l" ; restore registers
!macroend

!macro Execute_
	System::Store "s" ; store registers in System's private stack
	Pop $0 ; cmdline
	Pop $1 ; wrkdir
	
	System::Alloc 68 ; 4*16 + 2*2 / STARTUPINFO structure = $2
	Pop $2
	System::Call '*$2(i 68)' ; set cb = sizeof(STARTUPINFO)
	System::Call '*(i,i,i,i)i .r3' ; PROCESS_INFORMATION structure = $3
	
	${If} $1 == ""
		StrCpy $1 "i"
	${Else}
		StrCpy $1 'w "$1"'
	${EndIf}
	
	System::Call `kernel32::CreateProcessW(i, w '$0', i, i, i 0, i 0, i, $1, i r2, i r3)i .r4` ; return 0 if fail
	${Unless} $4 = 0 ; failed to create process
		System::Call '*$3(i .r4, i .r5, i .r6)' ; read handles and PID
		System::Call 'kernel32::CloseHandle(i $4)' ; close hProcess
		System::Call 'kernel32::CloseHandle(i $5)' ; close hThread
		Push $6 ; return PID
	${Else}
		Push 0 ; return val if failed
	${EndUnless}
	
	System::Free $2 ; free STARTUPINFO struct
	System::Free $3 ; free PROCESS_INFORMATION struct
	
	System::Store "l" ; restore registers
!macroend

!macro LLProcessExists_
	System::Store "s" ; store registers in System's private stack
	Pop $0 ; process name
	
	StrCpy $_LOGICLIB_TEMP 0
	
	System::Call '*(&l4,i,i,i,i,i,i,i,i,${_PROCFUNC_WSTRING})i .r2' ; $2 = PROCESSENTRY32W structure
	; take system process snapshot in $3
	System::Call 'kernel32::CreateToolhelp32Snapshot(i 2, i 0)i .r3'
	IntCmp $3 -1 done
		System::Call 'kernel32::Process32FirstW(i r3, i r2)i .r4'
		IntCmp $4 0 endloop
			loop:
				System::Call '*$2(i,i,i,i,i,i,i,i,i,${_PROCFUNC_WSTRING} .r5)'
				StrCmp $5 $0 0 next_process
					StrCpy $_LOGICLIB_TEMP 1
					Goto endloop
				next_process:
				System::Call 'kernel32::Process32NextW(i r3, i r2)i .r4'
				IntCmp $4 0 endloop
				Goto loop
			endloop:
			System::Call 'kernel32::CloseHandle(i r3)' ; close snapshot
	done:
	System::Free $2 ; free buffer
	
	System::Store "l" ; restore registers
!macroend

!endif ; PROCFUNC_INCLUDED

/****************************************************************************
	Functions
	=========
	
	HANDLE WINAPI OpenProcess(
	__in  DWORD dwDesiredAccess,
	__in  BOOL bInheritHandle,
	__in  DWORD dwProcessId
	);
	
	BOOL WINAPI CreateProcess(
	__in_opt     LPCTSTR lpApplicationName,
	__inout_opt  LPTSTR lpCommandLine,
	__in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in         BOOL bInheritHandles,
	__in         DWORD dwCreationFlags,
	__in_opt     LPVOID lpEnvironment,
	__in_opt     LPCTSTR lpCurrentDirectory,
	__in         LPSTARTUPINFO lpStartupInfo,
	__out        LPPROCESS_INFORMATION lpProcessInformation
	);
	
	typedef struct _STARTUPINFO {
	DWORD cb;
	LPTSTR lpReserved;
	LPTSTR lpDesktop;
	LPTSTR lpTitle;
	DWORD dwX;
	DWORD dwY;
	DWORD dwXSize;
	DWORD dwYSize;
	DWORD dwXCountChars;
	DWORD dwYCountChars;
	DWORD dwFillAttribute;
	DWORD dwFlags;
	WORD wShowWindow;
	WORD cbReserved2;
	LPBYTE lpReserved2;
	HANDLE hStdInput;
	HANDLE hStdOutput;
	HANDLE hStdError;
	} STARTUPINFO, 
	*LPSTARTUPINFO;
	
	typedef struct _PROCESS_INFORMATION {
	HANDLE hProcess;
	HANDLE hThread;
	DWORD dwProcessId;
	DWORD dwThreadId;
	} PROCESS_INFORMATION, 
	*LPPROCESS_INFORMATION;

	BOOL WINAPI EnumProcesses(
	__out  DWORD* pProcessIds,
	__in   DWORD cb,
	__out  DWORD* pBytesReturned
	);

	DWORD WINAPI GetModuleBaseName(
	__in      HANDLE hProcess,
	__in_opt  HMODULE hModule,
	__out     LPTSTR lpBaseName,
	__in      DWORD nSize
	);
	
	DWORD WINAPI GetModuleFileNameEx(
	__in      HANDLE hProcess,
	__in_opt  HMODULE hModule,
	__out     LPTSTR lpFilename,
	__in      DWORD nSize
	);

	BOOL WINAPI CloseHandle(
	__in  HANDLE hObject
	);
	
	DWORD WINAPI WaitForSingleObject(
	__in  HANDLE hHandle,
	__in  DWORD dwMilliseconds
	);
	
	BOOL WINAPI TerminateProcess(
	__in  HANDLE hProcess,
	__in  UINT uExitCode
	);
	
	BOOL EnumWindows(
	__in  WNDENUMPROC lpEnumFunc,
	__in  LPARAM lParam
	);
	
	DWORD GetWindowThreadProcessId(      
    __in  HWND hWnd,
    __out LPDWORD lpdwProcessId
	);
	
	BOOL PostMessage(      
    __in  HWND hWnd,
    __in  UINT Msg,
    __in  WPARAM wParam,
    __in  LPARAM lParam
	);

****************************************************************************/
