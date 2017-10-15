@echo off
setlocal enableextensions
set SCRIPT_DIR=%~dp0

set NP3_WIN32_DIR=%SCRIPT_DIR%..\Bin\Release_x86_v141\
set NP3_X64_DIR=%SCRIPT_DIR%..\Bin\Release_x64_v141\
set NP3_BUILD_VER=%SCRIPT_DIR%..\Versions\build.txt

:: --------------------------------------------------------------------------------------------------------------------

set YY=00
set MM=00
set DD=00
call :GETDATE
set BUILD=0
call :GETBUILD "%~1"
set VERSHOULD=2.%YY%.%MM%%DD%.%BUILD%

:: --------------------------------------------------------------------------------------------------------------------

set TEST_RESULT=failure

set FILEVER=
set FILEVER32=
if exist "%NP3_WIN32_DIR%Notepad3.exe" call :GETFILEVER "%NP3_WIN32_DIR%Notepad3.exe"
if defined FILEVER set FILEVER32=%FILEVER%
set FILEVER=
set FILEVER64=
if exist "%NP3_X64_DIR%Notepad3.exe" call :GETFILEVER "%NP3_X64_DIR%Notepad3.exe"
if defined FILEVER set FILEVER64=%FILEVER%

if defined FILEVER32 set TEST_RESULT=success
if defined FILEVER64 set TEST_RESULT=success
call :COMPAREVER "%VERSHOULD%" "%FILEVER32%" "%FILEVER64%"

:: ====================================================================================================================
goto :END
:: --------------------------------------------------------------------------------------------------------------------

:COMPAREVER
if ["%~2"] NEQ [""] (
  if ["%~1"] NEQ ["%~2"] (
      echo ERROR: Expected version "%~1", found version "%~2" in 32-bit exe 
      set TEST_RESULT=failure
  )
)
if ["%~3"] NEQ [""] (
  if ["%~1"] NEQ ["%~3"] (
      echo ERROR: Expected version "%~1", found version "%~3" in 64-bit exe
      set TEST_RESULT=failure
  )
)
goto:EOF
:: --------------------------------------------------------------------------------------------------------------------

:GETDATE
for /f "tokens=2 delims==" %%a in ('
    wmic OS Get localdatetime /value
') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
::set "datestamp=%YYYY%%MM%%DD%" & set "timestamp=%HH%%Min%%Sec%"
::set "fullstamp=%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"
::echo datestamp: "%datestamp%"
::echo timestamp: "%timestamp%"
::echo fullstamp: "%fullstamp%"
goto:EOF
:: --------------------------------------------------------------------------------------------------------------------

:GETFILEVER
set "file=%~1"
if not defined file goto:EOF
if not exist "%file%" goto:EOF
set "FILEVER="
for /F "tokens=2 delims==" %%a in ('
    wmic datafile where name^="%file:\=\\%" Get Version /value 
') do set "FILEVER=%%a"
::echo %file% = %FILEVER% 
goto:EOF
:: --------------------------------------------------------------------------------------------------------------------

:GETBUILD
set argbuild=%~1
if defined argbuild (
    set BUILD=%argbuild%
    goto:EOF
)
::set /p nxbuild=<"%NP3_BUILD_VER%"
::set /a BUILD=%nxbuild% - 1
set /p BUILD=<"%NP3_BUILD_VER%"
goto:EOF
:: --------------------------------------------------------------------------------------------------------------------

:: ====================================================================================================================
:END
echo Expected Version = %VERSHOULD%
if ["%FILEVER32%"] NEQ [""] (
  echo 32-bit exe Version = %FILEVER32%
) else (
  echo No 32-bit exe Version found
)
if ["%FILEVER64%"] NEQ [""] (
  echo 64-bit exe Version = %FILEVER64%
) else (
  echo No 64-bit exe Version found
)
echo Version Test Result: %TEST_RESULT%
:: - make TEST_RESULT survive 'endlocal'
endlocal & set TEST_RESULT=%TEST_RESULT%
::pause
if [%TEST_RESULT%] NEQ [success] exit /B 1
:: ====================================================================================================================
