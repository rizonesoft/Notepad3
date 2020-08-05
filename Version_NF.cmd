@echo off
setlocal
set _VERPATCH_=nf
echo."%_VERPATCH_%">.\np3portableapp\_buildname.txt
Version -VerPatch "%_VERPATCH_%"
endlocal
