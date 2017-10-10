@echo off
setlocal enableextensions enabledelayedexpansion
set SCRIPTDRV=%~d0
set SCRIPTDIR=%~dp0
set CWD=%CD%
 
set TEST_DIR=%SCRIPTDIR%_TESTDIR\
set TEST_LOG=test.log
set NP3_DISTRIB_DIR=%SCRIPT_DIR%..\distrib\
set NP3_WIN32_DIR=%SCRIPT_DIR%..\Bin\Release_x86_v141\
set NP3_X64_DIR=%SCRIPT_DIR%..\Bin\Release_x64_v141\

set AHK_EXE=%ProgramFiles%/AutoHotkey/AutoHotkeyU32.exe 
set EXITCODE=0

:: --------------------------------------------------------------------------------------------------------------------

:: prepare tests
if not exist "%TEST_DIR%" mkdir "%TEST_DIR%"
copy "%NP3_DISTRIB_DIR%Notepad3.ini" "%TEST_DIR%Notepad3.ini" /Y /V
copy /B "%NP3_WIN32_DIR%Notepad3.exe" /B "%TEST_DIR%Notepad3.exe" /Y /V

rem Loop over all ahk files in tests directory
rem for /r %%i in (*.ahk) do (
rem 	echo ** Running %%~nxi **
rem 	start "testing" /B /wait "%AHK_EXE%" /ErrorStdOut %%~nxi > %TEST_LOG% 2>&1
rem 	if errorlevel 1 (
rem 		echo *** Test file %%~nxi failed ***
rem 		set err_level=1
rem 	)
rem 	type testoutput.txt
rem 	echo.
rem )

:: START Testing
"%AHK_EXE%" /ErrorStdOut "%~dpn0.ahk" > %TEST_LOG% 2>&1
if errorlevel 1 (
  echo *** Test failed *** >> %TEST_LOG%
  set EXITCODE=1
)

:: --------------------------------------------------------------------------------------------------------------------
:END
type %TEST_LOG%
:: - make EXITCODE survive 'endlocal'
endlocal & set EXITCODE=%EXITCODE%

exit /b %EXITCODE%

:: ====================================================================================================================
:: --------------------------------------------------------------------------------------------------------------------
:: ====================================================================================================================
 