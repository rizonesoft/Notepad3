; ReplaceInFile wrapper for testreplace function
; John T. Haller of PortableApps.com
; BSD License
; Requires TextReplace plugin installed in NSIS and !include "TextReplace.nsh" in main nsi
;
; Usage: ${ReplaceInFile} SOURCE_FILE SEARCH_TEXT REPLACEMENT
; or use ReplaceInFileCS for case-sensitive (use when possible, it's faster)
; No return variable.  Error will be set if unable to do the replacement (invalid file, locked file, etc)

Function ReplaceInFile
	Exch $0 ;REPLACEMENT
	Exch
	Exch $1 ;SEARCH_TEXT
	Exch 2
	Exch $2 ;SOURCE_FILE
	Exch 3
	Exch $3 ;CASE_INSENSITIVE
	Push $4 ;NEW_FILE
	Push $5 ;RETURN_VALUE
	
	StrCpy $4 `$2.OldReplaceInFile`
	
	${textreplace::ReplaceInFile} "$2" "$4" "$1" "$0" "$3 /C=0" $5
	
	IntCmp $5 0 StackCleanup ReturnError RenameToOriginal
	
	ReturnError:
		SetErrors
		Goto StackCleanup
	
	RenameToOriginal:
		Delete $2
		Rename $4 $2
	
	StackCleanup:
		Pop $5
		Pop $4
		Pop $3
		Pop $0
		Pop $1
		Pop $2

		${textreplace::Unload}
FunctionEnd

!macro ReplaceInFileCS SOURCE_FILE SEARCH_TEXT REPLACEMENT
	Push `/S=1`
	Push `${SOURCE_FILE}`
	Push `${SEARCH_TEXT}`
	Push `${REPLACEMENT}`
	Call ReplaceInFile
!macroend

!macro ReplaceInFile SOURCE_FILE SEARCH_TEXT REPLACEMENT
	Push `/S=0`
	Push `${SOURCE_FILE}`
	Push `${SEARCH_TEXT}`
	Push `${REPLACEMENT}`
	Call ReplaceInFile
!macroend

!define ReplaceInFileCS '!insertmacro "ReplaceInFileCS"'

!define ReplaceInFile '!insertmacro "ReplaceInFile"'