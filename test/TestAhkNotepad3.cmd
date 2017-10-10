@echo off
setlocal enableextensions
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
rem 		set err_level=%ERRORLEVEL%
rem 		echo *** Test file %%~nxi failed ***
rem 	)
rem 	type testoutput.txt
rem 	echo.
rem )

:: START Testing
start "Testing" /B /Wait "%AHK_EXE%" /ErrorStdOut "%~dpn0.ahk" > "%TEST_LOG%" 2>&1
if errorlevel 1 (
  set EXITCODE=%ERRORLEVEL%
  echo *** Test failed *** >> "%TEST_LOG%"
)

:: --------------------------------------------------------------------------------------------------------------------
:END
type "%TEST_LOG%"
:: - make EXITCODE survive 'endlocal'
endlocal & set EXITCODE=%EXITCODE%
::echo.EXITCODE=%EXITCODE%
::pause
if [%EXITCODE%] NEQ [0] exit /B %EXITCODE%

:: --------------------------------------------------------------------------------------------------------------------
:: ====================================================================================================================
 