@ECHO OFF
rem ******************************************************************************
rem *
rem * Notepad2-mod
rem *
rem * make_installer.bat
rem *   Batch file for building the installer for Notepad2-mod
rem *
rem * See License.txt for details about distribution and modification.
rem *
rem *                                     (c) XhmikosR 2010-2015
rem *                                     https://github.com/XhmikosR/notepad2-mod
rem *
rem ******************************************************************************

SETLOCAL ENABLEEXTENSIONS
CD /D %~dp0

rem Check for the help switches
IF /I "%~1" == "help"   GOTO SHOWHELP
IF /I "%~1" == "/help"  GOTO SHOWHELP
IF /I "%~1" == "-help"  GOTO SHOWHELP
IF /I "%~1" == "--help" GOTO SHOWHELP
IF /I "%~1" == "/?"     GOTO SHOWHELP


rem You can set here the Inno Setup path if for example you have Inno Setup Unicode
rem installed and you want to use the ANSI Inno Setup which is in another location
IF NOT DEFINED InnoSetupPath SET "InnoSetupPath=H:\progs\thirdparty\isetup"

rem Check the building environment
CALL :SubDetectInnoSetup


rem Check for the first switch
IF "%~1" == "" (
  SET "COMPILER=VS2015"
) ELSE (
  IF /I "%~1" == "VS2010"   SET "COMPILER=VS2010" & GOTO START
  IF /I "%~1" == "/VS2010"  SET "COMPILER=VS2010" & GOTO START
  IF /I "%~1" == "-VS2010"  SET "COMPILER=VS2010" & GOTO START
  IF /I "%~1" == "--VS2010" SET "COMPILER=VS2010" & GOTO START
  IF /I "%~1" == "VS2012"   SET "COMPILER=VS2012" & GOTO START
  IF /I "%~1" == "/VS2012"  SET "COMPILER=VS2012" & GOTO START
  IF /I "%~1" == "-VS2012"  SET "COMPILER=VS2012" & GOTO START
  IF /I "%~1" == "--VS2012" SET "COMPILER=VS2012" & GOTO START
  IF /I "%~1" == "VS2013"   SET "COMPILER=VS2013" & GOTO START
  IF /I "%~1" == "/VS2013"  SET "COMPILER=VS2013" & GOTO START
  IF /I "%~1" == "-VS2013"  SET "COMPILER=VS2013" & GOTO START
  IF /I "%~1" == "--VS2013" SET "COMPILER=VS2013" & GOTO START
  IF /I "%~1" == "VS2015"   SET "COMPILER=VS2015" & GOTO START
  IF /I "%~1" == "/VS2015"  SET "COMPILER=VS2015" & GOTO START
  IF /I "%~1" == "-VS2015"  SET "COMPILER=VS2015" & GOTO START
  IF /I "%~1" == "--VS2015" SET "COMPILER=VS2015" & GOTO START
  IF /I "%~1" == "WDK"      SET "COMPILER=WDK"    & GOTO START
  IF /I "%~1" == "/WDK"     SET "COMPILER=WDK"    & GOTO START
  IF /I "%~1" == "-WDK"     SET "COMPILER=WDK"    & GOTO START
  IF /I "%~1" == "--WDK"    SET "COMPILER=WDK"    & GOTO START

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:START
SET INPUTDIRx86=Bin\Release_x86
SET INPUTDIRx64=Bin\Release_x64
IF /I NOT "%COMPILER%" == "VS2015" SET SUFFIX=_%COMPILER%
SET "TEMP_NAME=temp_zip%SUFFIX%"

IF NOT EXIST "..\%INPUTDIRx86%\Notepad3.exe" CALL :SUBMSG "ERROR" "Compile Notepad3 x86 first!"
IF NOT EXIST "..\%INPUTDIRx64%\Notepad3.exe" CALL :SUBMSG "ERROR" "Compile Notepad3 x64 first!"

CALL :SubInstaller %COMPILER%


:END
TITLE Finished!
ECHO.
ENDLOCAL
EXIT /B


:SubInstaller
TITLE Building %1 installer...
CALL :SUBMSG "INFO" "Building %1 installer using %InnoSetupPath%\ISCC.exe..."

"%InnoSetupPath%\ISCC.exe" /SMySignTool="cmd /c "%~dp0sign.bat" $f" /Q /O"packages" "..\distrib\notepad3_setup.iss" /D%1
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"

EXIT /B


:SubDetectInnoSetup
rem Detect if we are running on 64bit Windows and use Wow6432Node since Inno Setup is
rem a 32-bit application, and set the registry key of Inno Setup accordingly
IF DEFINED PROGRAMFILES(x86) (
  SET "U_=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
) ELSE (
  SET "U_=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"
)

IF DEFINED InnoSetupPath IF NOT EXIST "%InnoSetupPath%" (
  CALL :SUBMSG "INFO" ""%InnoSetupPath%" wasn't found on this machine! I will try to detect Inno Setup's path from the registry..."
)

IF NOT EXIST "%InnoSetupPath%" (
  FOR /F "delims=" %%a IN (
    'REG QUERY "%U_%\Inno Setup 5_is1" /v "Inno Setup: App Path"2^>Nul^|FIND "REG_"') DO (
    SET "InnoSetupPath=%%a" & CALL :SubInnoSetupPath %%InnoSetupPath:*Z=%%)
)

IF NOT EXIST "%InnoSetupPath%" CALL :SUBMSG "ERROR" "Inno Setup wasn't found!"
EXIT /B


:SubInnoSetupPath
SET "InnoSetupPath=%*"
EXIT /B


:SubSign
IF %ERRORLEVEL% NEQ 0 EXIT /B
REM %1 is the subfolder

CALL "%~dp0sign.bat" "..\%1\Notepad2.exe" || (CALL :SUBMSG "ERROR" "Problem signing ..\%1\Notepad2.exe" & GOTO Break)

CALL :SUBMSG "INFO" "..\%1\Notepad2.exe signed successfully."

:Break
EXIT /B


:SHOWHELP
TITLE %~nx0 %1
ECHO. & ECHO.
ECHO Usage:  %~nx0 [VS2010^|VS2012^|VS2013^|VS2015^|WDK]
ECHO.
ECHO Notes:  You can also prefix the commands with "-", "--" or "/".
ECHO         The arguments are not case sensitive.
ECHO.
ECHO         You can use another Inno Setup location by defining %%InnoSetupPath%%.
ECHO         This is usefull if you have the Unicode build installed
ECHO         and you want to use the ANSI one.
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
