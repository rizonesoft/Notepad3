:: Batch file for RELEASE version (without the separator "_" before ".paf" in Notepad3Portable)
@echo off
setlocal
set _VERPATCH_=
echo."%_VERPATCH_%">.\np3portableapp\_buildname.txt
Version -VerPatch "%_VERPATCH_%"
endlocal
