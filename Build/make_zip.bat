@ECHO OFF
rem ******************************************************************************
rem *                                                                            *
rem * Notepad3                                                                   *
rem *                                                                            *
rem * make_zip.bat                                                               *
rem *   Batch file for creating the zip packages                                 *
rem *                                                                            *
rem * See License.txt for details about distribution and modification.           *
rem *                                                                            *
rem *                                                 (c) Rizonesoft 2008-2020   *
rem *                                                   https://rizonesoft.com   *
rem *                                                                            *
rem ******************************************************************************

SETLOCAL ENABLEEXTENSIONS
CD /D %~dp0

rem Check for the help switches
IF /I "%~1" == "help"   GOTO SHOWHELP
IF /I "%~1" == "help"   GOTO SHOWHELP
IF /I "%~1" == "/help"  GOTO SHOWHELP
IF /I "%~1" == "-help"  GOTO SHOWHELP
IF /I "%~1" == "--help" GOTO SHOWHELP
IF /I "%~1" == "/?"     GOTO SHOWHELP

rem IF EXIST "%~dp0..\signinfo_notepad3.txt" SET "SIGN=True"

SET INPUTDIRx86=bin\Release_x86_v142
SET INPUTDIRx64=bin\Release_x64_v142
SET "TEMP_NAME=temp_zip"

IF NOT EXIST "..\%INPUTDIRx86%\Notepad3.exe"   CALL :SUBMSG "ERROR" "Compile Notepad3 x86 first!"
IF NOT EXIST "..\%INPUTDIRx86%\minipath.exe"   CALL :SUBMSG "ERROR" "Compile MiniPath x86 first!"
IF NOT EXIST "..\%INPUTDIRx86%\grepWinNP3.exe" CALL :SUBMSG "ERROR" "Compile grepWinNP3 x86 first!"
IF NOT EXIST "..\%INPUTDIRx64%\Notepad3.exe"   CALL :SUBMSG "ERROR" "Compile Notepad3 x64 first!"
IF NOT EXIST "..\%INPUTDIRx64%\minipath.exe"   CALL :SUBMSG "ERROR" "Compile MiniPath x64 first!"
IF NOT EXIST "..\%INPUTDIRx64%\grepWinNP3.exe" CALL :SUBMSG "ERROR" "Compile grepWinNP3 x64 first!"

CALL :SubGetVersion
CALL :SubDetectSevenzipPath

IF /I "%SEVENZIP%" == "" CALL :SUBMSG "ERROR" "7za wasn't found!"

REM IF /I "%SIGN%" == "True" CALL :SubSign %INPUTDIRx86%
REM IF /I "%SIGN%" == "True" CALL :SubSign %INPUTDIRx64%

CALL :SubZipFiles %INPUTDIRx86% x86
CALL :SubZipFiles %INPUTDIRx64% x64

rem Compress everything into a single ZIP file
PUSHD "packages"
IF EXIST "Notepad3_%NP2_VER%.zip" DEL "Notepad3_%NP2_VER%.zip"
IF EXIST "%TEMP_NAME%"      RD /S /Q "%TEMP_NAME%"
IF NOT EXIST "%TEMP_NAME%"  MD "%TEMP_NAME%"

REM IF EXIST "Notepad3_%NP2_VER%*.exe" COPY /Y /V "Notepad3_%NP2_VER%*.exe" "%TEMP_NAME%\" >NUL
IF EXIST "Notepad3_%NP2_VER%*.zip" COPY /Y /V "Notepad3_%NP2_VER%*.zip" "%TEMP_NAME%\" >NUL

PUSHD "%TEMP_NAME%"

"%SEVENZIP%" a -tzip -mx=9 Notepad3_%NP2_VER%.zip * >NUL
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"

CALL :SUBMSG "INFO" "Notepad3_%NP2_VER%.zip created successfully!"

MOVE /Y "Notepad3_%NP2_VER%.zip" ".." >NUL

POPD
IF EXIST "%TEMP_NAME%" RD /S /Q "%TEMP_NAME%"

POPD


:END
TITLE Finished!
ECHO.
ENDLOCAL
EXIT /B


:SubZipFiles
SET "ZIP_NAME=Notepad3_%NP2_VER%_%2%SUFFIX%"
TITLE Creating %ZIP_NAME%.zip...
CALL :SUBMSG "INFO" "Creating %ZIP_NAME%.zip..."
echo off
IF EXIST "%TEMP_NAME%"     RD /S /Q "%TEMP_NAME%"
IF NOT EXIST "%TEMP_NAME%" MD "%TEMP_NAME%"
IF NOT EXIST "Packages"    MD "Packages"

FOR %%A IN ("..\License.txt" "..\Readme.txt" "..\grepWinNP3\grepWinLicense.txt" "Notepad3.ini" "minipath.ini"^
    "..\%1\Notepad3.exe" "..\%1\minipath.exe" "..\%1\grepWinNP3.exe") DO COPY /Y /V "%%A" "%TEMP_NAME%\"

SET "LNG=%TEMP_NAME%\lng"
SET "GRP=%TEMP_NAME%\lng\gwLng\"
SET "THEMES=%TEMP_NAME%\themes"
SET "DOCS=%TEMP_NAME%\Docs"
IF NOT EXIST %LNG% MD %LNG%
IF NOT EXIST %THEMES% MD %THEMES%
IF NOT EXIST %DOCS% MD %DOCS%
XCOPY /E /Y /V "..\%1\lng" "%LNG%" /EXCLUDE:Ignore.txt
XCOPY /E /Y /V "..\grepWinNP3\translationsNP3" "%GRP%"
XCOPY /E /Y /V "themes" "%THEMES%"
XCOPY /E /Y /V "Docs" "%DOCS%"
COPY /Y /V "Changes.txt" "%DOCS%"

SET "FAVORITES=%TEMP_NAME%\Favorites"
IF NOT EXIST "%FAVORITES%" MD "%FAVORITES%"

PUSHD "%TEMP_NAME%"
"%SEVENZIP%" a -tzip -mx=9^
 "%ZIP_NAME%.zip" "License.txt" "Notepad3.exe" "Notepad3.ini" "grepWinLicense.txt" "Readme.txt"^
 "Favorites" "minipath.exe" "minipath.ini" "grepWinNP3.exe" "lng" "themes" "Docs">NUL
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"

CALL :SUBMSG "INFO" "%ZIP_NAME%.zip created successfully!"

MOVE /Y "%ZIP_NAME%.zip" "..\packages" >NUL
POPD
IF EXIST "%TEMP_NAME%" RD /S /Q "%TEMP_NAME%"
EXIT /B


:SubDetectSevenzipPath
FOR %%G IN (7z.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR %%G IN (7za.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR /F "tokens=2*" %%A IN (
  'REG QUERY "HKLM\SOFTWARE\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ" ^|^|
   REG QUERY "HKLM\SOFTWARE\Wow6432Node\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ"') DO SET "SEVENZIP=%%B\7z.exe"
EXIT /B


:SubGetVersion
rem Get the version
FOR /F "tokens=3,4 delims= " %%K IN (
  'FINDSTR /I /L /C:"define VERSION_MAJOR" "..\src\VersionEx.h"') DO (SET "VerMajor=%%K")
FOR /F "tokens=3,4 delims= " %%K IN (
  'FINDSTR /I /L /C:"define VERSION_MINOR" "..\src\VersionEx.h"') DO (SET "VerMinor=%%K")
FOR /F "tokens=3,4 delims= " %%K IN (
  'FINDSTR /I /L /C:"define VERSION_REV" "..\src\VersionEx.h"') DO (SET "VerRev=%%K")
FOR /F "tokens=3,4 delims= " %%K IN (
  'FINDSTR /I /L /C:"define VERSION_BUILD" "..\src\VersionEx.h"') DO (SET "VerBuild=%%K")

SET NP2_VER=%VerMajor%.%VerMinor%.%VerRev%.%VerBuild%
EXIT /B


:SubSign
REM IF %ERRORLEVEL% NEQ 0 EXIT /B
REM %1 is the subfolder

REM CALL "%~dp0sign.bat" "..\%1\Notepad2.exe" || (CALL :SUBMSG "ERROR" "Problem signing ..\%1\Notepad2.exe" & GOTO Break)

REM CALL :SUBMSG "INFO" "..\%1\Notepad2.exe signed successfully."

REM :Break
REM EXIT /B


:SHOWHELP
TITLE %~nx0 %1
ECHO. & ECHO.
ECHO Usage:  %~nx0 [VS2010^|VS2012^|VS2013^|VS2015^|WDK]
ECHO.
ECHO Notes:  You can also prefix the commands with "-", "--" or "/".
ECHO         The arguments are not case sensitive.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments is equivalent to "%~nx0 WDK"
ECHO.
ENDLOCAL
EXIT /B


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
