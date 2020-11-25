:: Batch file for ALPHA and XPERIMENTAL version
@echo off
setlocal
set _VERPATCH_=alpha
echo."_%_VERPATCH_%">.\np3portableapp\_buildname.txt
Version -VerPatch "%_VERPATCH_%"
endlocal
