:: Batch file for RELEASE CANDIDATE version
@echo off
setlocal
set _VERPATCH_=rc
echo."_%_VERPATCH_%">.\np3portableapp\_buildname.txt
Version -VerPatch "%_VERPATCH_%"
endlocal
