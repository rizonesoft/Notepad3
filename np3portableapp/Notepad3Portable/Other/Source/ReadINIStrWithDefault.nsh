; ReadINIStrWithDefault 1.1 (2009-05-12)
;
; Substitutes a default value if the INI is undefined
; Copyright 2008-2009 John T. Haller of PortableApps.com
; Released under the BSD
;
; Usage: ${ReadINIStrWithDefault} OUTPUT_VALUE INI_FILENAME SECTION_NAME ENTRY_NAME DEFAULT_VALUE
;
; History:
; 1.1 (2009-05-12): Fixed error with $0 and $2 being swapped

Function ReadINIStrWithDefault
	;Start with a clean slate
	ClearErrors
	
	;Get our parameters
	Exch $0 ;DEFAULT_VALUE
	Exch
	Exch $1 ;ENTRY_NAME
	Exch 2
	Exch $2 ;SECTION_NAME
	Exch  3
	Exch $3 ;INI_FILENAME
	Push $4 ;OUTPUT_VALUE
	
	;Read from the INI
	ReadINIStr $4 $3 $2 $1
	IfErrors 0 +3
		StrCpy $4 $0
		ClearErrors

	;Keep the variable for last
	StrCpy $0 $4
	
	;Clear the stack
	Pop $4
	Pop $3
	Exch 2
	Pop $2
	Pop $1
	
	;Reset the last variable and leave our result on the stack
	Exch $0
FunctionEnd

!macro ReadINIStrWithDefault OUTPUT_VALUE INI_FILENAME SECTION_NAME ENTRY_NAME DEFAULT_VALUE
  Push `${INI_FILENAME}`
  Push `${SECTION_NAME}`
  Push `${ENTRY_NAME}`
  Push `${DEFAULT_VALUE}`
  Call ReadINIStrWithDefault
  Pop `${OUTPUT_VALUE}`
!macroend

!define ReadINIStrWithDefault '!insertmacro "ReadINIStrWithDefault"'