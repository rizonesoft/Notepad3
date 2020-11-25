:: Batch file for BETA version
@echo off
setlocal
set _VERPATCH_=beta
echo."_%_VERPATCH_%">.\np3portableapp\_buildname.txt
Version -VerPatch "%_VERPATCH_%"
endlocal
