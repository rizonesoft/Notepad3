@ECHO OFF
rem ****************************************************************************
rem *                                                                          *
rem * Notepad3                                                                 *
rem *                                                                          *
rem * upx.bat                                                                  *
rem *   Batch file to compress all executable files.                           *
rem *                                                                          *
rem *                                                                          *
rem *                                     (c) Rizonesoft 2008-2016             *
rem *                                         https://rizonesoft.com           *
rem *                                                                          *
rem *                                                                          *
rem ****************************************************************************

SETLOCAL ENABLEEXTENSIONS
CD /D %~dp0

SET INPUTDIRx86=Bin\Release_x86_v141
SET INPUTDIRx64=Bin\Release_x64_v141

IF NOT EXIST "..\%INPUTDIRx86%\Notepad3.exe" CALL :SUBMSG "ERROR" "Compile Notepad3 x86 first!"
IF NOT EXIST "..\%INPUTDIRx64%\Notepad3.exe" CALL :SUBMSG "ERROR" "Compile Notepad3 x64 first!"
ECHO.
ECHO.
Bin\UPX --best "..\%INPUTDIRx86%\Notepad3.exe"
ECHO.
Bin\UPX --best "..\%INPUTDIRx86%\minipath.exe"
ECHO.
Bin\UPX --best "..\%INPUTDIRx86%\np3encrypt.exe"
ECHO.
Bin\UPX --best "..\%INPUTDIRx64%\Notepad3.exe"
ECHO.
Bin\UPX --best "..\%INPUTDIRx64%\minipath.exe"
ECHO.
Bin\UPX --best "..\%INPUTDIRx64%\np3encrypt.exe"
ECHO.
ECHO.

:SUBMSG
ECHO. & ECHO ______________________________
ECHO [%~1] %~2
ECHO ______________________________ & ECHO.
IF /I "%~1" == "ERROR" (
  PAUSE
  EXIT
) ELSE (
  EXIT /B
)