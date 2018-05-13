@echo off
setlocal

rem for DLL generation: Project-Config: Linker: Manifest: Generate: NO  (/MANIFEST:NO)

set MUIRCT_EXE=C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\muirct.exe
set COMPDIR=.\Bin\Debug_x64_v141\lng

mkdir "%COMPDIR%\en-US"
"%MUIRCT_EXE%" -q DoReverseMuiLoc.rcconfig -v 2 -x 0x0409 -g 0x0409 "%COMPDIR%\np3_en_us.dll" "%COMPDIR%\np3_lng.dll" "%COMPDIR%\en-US\np3_lng.dll.mui"
"%MUIRCT_EXE%" -c "%COMPDIR%\np3_lng.dll" -e "%COMPDIR%\en-US\np3_lng.dll.mui"

mkdir "%COMPDIR%\de-DE"
"%MUIRCT_EXE%" -q DoReverseMuiLoc.rcconfig -v 2 -x 0x0407 -g 0x0409 "%COMPDIR%\np3_de_de.dll" "%COMPDIR%\np3_lng_de_discard.dll" "%COMPDIR%\de-DE\np3_lng.dll.mui"
"%MUIRCT_EXE%" -c "%COMPDIR%\np3_lng.dll" -e "%COMPDIR%\de-DE\np3_lng.dll.mui"

mkdir "%COMPDIR%\es-ES"
"%MUIRCT_EXE%" -q DoReverseMuiLoc.rcconfig -v 2 -x 0x0C0A -g 0x0409 "%COMPDIR%\np3_es_es.dll" "%COMPDIR%\np3_lng_es_discard.dll" "%COMPDIR%\es-ES\np3_lng.dll.mui"
"%MUIRCT_EXE%" -c "%COMPDIR%\np3_lng.dll" -e "%COMPDIR%\es-ES\np3_lng.dll.mui"

mkdir "%COMPDIR%\fr-FR"
"%MUIRCT_EXE%" -q DoReverseMuiLoc.rcconfig -v 2 -x 0x040C -g 0x0409 "%COMPDIR%\np3_fr_fr.dll" "%COMPDIR%\np3_lng_fr_discard.dll" "%COMPDIR%\fr-FR\np3_lng.dll.mui"
"%MUIRCT_EXE%" -c "%COMPDIR%\np3_lng.dll" -e "%COMPDIR%\fr-FR\np3_lng.dll.mui"

endlocal
pause
