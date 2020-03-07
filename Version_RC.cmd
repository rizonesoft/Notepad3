@echo off
setlocal
set _VERPATCH_=RC3
echo."%_VERPATCH_%">.\np3portableapp\_buildname.txt
Version -VerPatch "%_VERPATCH_%"
endlocal
