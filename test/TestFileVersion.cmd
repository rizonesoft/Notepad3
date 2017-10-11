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
call :GETBUILD
set VERSHOULD=2.%YY%.%MM%%DD%.%BUILD%

:: --------------------------------------------------------------------------------------------------------------------

set TEST_SUCCEEDED=false

set FILEVER=
if exist "%NP3_WIN32_DIR%Notepad3.exe" call :GETFILEVER "%NP3_WIN32_DIR%Notepad3.exe"
if exist "%NP3_X64_DIR%Notepad3.exe" call :GETFILEVER "%NP3_X64_DIR%Notepad3.exe"
if defined FILEVER call :COMPAREVER "%VERSHOULD%" "%FILEVER%"

:: ====================================================================================================================
goto :END
:: --------------------------------------------------------------------------------------------------------------------

:COMPAREVER
if ["%~1"] EQU ["%~2"] (
    set TEST_SUCCEEDED=true
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
set /p nxbuild=<%NP3_BUILD_VER%
set /a BUILD = %nxbuild% - 1
goto:EOF
:: --------------------------------------------------------------------------------------------------------------------

:: ====================================================================================================================
:END
echo.VERSHOULD = %VERSHOULD%
echo.FILEVER = %FILEVER%
echo.TEST_SUCCEEDED = %TEST_SUCCEEDED%
:: - make TEST_SUCCEEDED survive 'endlocal'
endlocal & set TEST_SUCCEEDED=%TEST_SUCCEEDED%
::pause
if [%TEST_SUCCEEDED%] NEQ [true] exit /B 1
:: ====================================================================================================================
