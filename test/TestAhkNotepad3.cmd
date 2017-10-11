@echo off
setlocal enableextensions
set SCRIPTDRV=%~d0
set SCRIPTDIR=%~dp0
set CWD=%CD%
 
set TEST_DIR=%SCRIPTDIR%_TESTDIR\
set TEST_LOG=test.log
set NP3_CONFIG_DIR=%SCRIPT_DIR%config\
set NP3_WIN32_DIR=%SCRIPT_DIR%..\Bin\Release_x86_v141\
set NP3_X64_DIR=%SCRIPT_DIR%..\Bin\Release_x64_v141\

set AHK_EXE=%ProgramW6432%/AutoHotkey/AutoHotkeyU32.exe
set AHK_EXE32=%ProgramFiles(x86)%/AutoHotkey/AutoHotkeyU32.exe
set AHK_EXE64=%ProgramFiles%/AutoHotkey/AutoHotkeyU32.exe
if not exist "%AHK_EXE%" set AHK_EXE=%AHK_EXE32%
if not exist "%AHK_EXE%" set AHK_EXE=%AHK_EXE64%
 
:: --------------------------------------------------------------------------------------------------------------------

:: prepare tests
if not exist "%TEST_DIR%" mkdir "%TEST_DIR%"
if not exist "%TEST_DIR%Favorites\" mkdir "%TEST_DIR%Favorites\"
copy "%NP3_CONFIG_DIR%Notepad3_distrib.ini" "%TEST_DIR%Notepad3.ini" /Y /V
if exist "%NP3_WIN32_DIR%Notepad3.exe" copy /B "%NP3_WIN32_DIR%Notepad3.exe" /B "%TEST_DIR%Notepad3.exe" /Y /V
if exist "%NP3_X64_DIR%Notepad3.exe" copy /B "%NP3_X64_DIR%Notepad3.exe" /B "%TEST_DIR%Notepad3.exe" /Y /V

::Loop over all ahk files in tests directory
echo. > "%TEST_LOG%"
set EXITCODE=0
::for /r %%i in (*.ahk) do (
for %%i in (*.ahk) do (
  echo - Run Testsuite %%~nxi
	echo +++ Run Testsuite %%~nxi +++ >> "%TEST_LOG%"
	start "testing" /B /Wait "%AHK_EXE%" /ErrorStdOut "%%~nxi" >> "%TEST_LOG%" 2>&1
	if errorlevel 1 (
		set EXITCODE=%ERRORLEVEL%
		echo *** Testsuite %%~nxi failed! ***
		echo *** ERROR: Testsuite %%~nxi failed! *** >> "%TEST_LOG%"
	) else (
	  echo +++ Testsuite %%~nxi succeeded. +++ >> "%TEST_LOG%"
	)
  echo. >> "%TEST_LOG%"
)

:: --------------------------------------------------------------------------------------------------------------------
:END
type "%TEST_LOG%"
:: - make EXITCODE survive 'endlocal'
endlocal & set EXITCODE=%EXITCODE%
::echo.EXITCODE=%EXITCODE%
::pause
if [%EXITCODE%] NEQ [0] exit /B %EXITCODE%
:: ====================================================================================================================
