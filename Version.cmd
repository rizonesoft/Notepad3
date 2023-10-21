:: - USAGE: do not wrap by 'call' "%~dpn0.cmd" (continue parent shell after exit /b 0)
:: - Admin rights:
:: PowerShell.exe -NoProfile -Command "& {Start-Process PowerShell.exe -ArgumentList '-NoProfile -ExecutionPolicy Bypass -File ""%~dpn0.ps1""' -Verb RunAs}"
@echo off
setlocal enableextensions
set SCRIPTNAME=%~dpn0.ps1
set ARGS=%*
if ["%ARGS%"] NEQ [""] (
  call :ESCAPE_ARGS
  echo."_%ARGS%">.\np3portableapp\_buildname.txt
) else (
  echo."%ARGS%">.\np3portableapp\_buildname.txt
)
if ["%ARGS%"] NEQ [""] (
  set ARGS=-VerPatch "%ARGS%"
)

:POWERSHELL
PowerShell.exe -NoProfile -NonInteractive -NoLogo -ExecutionPolicy Unrestricted -Command "& { $ErrorActionPreference = 'Stop'; & '%SCRIPTNAME%' @args; Exit $LastExitCode }" %ARGS%
set EXITCODE=%ERRORLEVEL%
::ECHO ERRORLEVEL=%EXITCODE%

:: Pause of 8 seconds to verify the "Notepad3 version number:" before exiting
:: ============================================================================
ping -n 9 127.0.0.1>nul

goto :END

:ESCAPE_ARGS
set ARGS=%ARGS:"=\"%
set ARGS=%ARGS:`=``%
set ARGS=%ARGS:'=`'%
set ARGS=%ARGS:$=`$%
set ARGS=%ARGS:{=`}%
set ARGS=%ARGS:}=`}%
set ARGS=%ARGS:(=`(%
set ARGS=%ARGS:)=`)%
set ARGS=%ARGS:,=`,%
set ARGS=%ARGS:^%=%
goto:eof

:END
:: - make EXITCODE survive 'endlocal'
endlocal & set EXITCODE=%EXITCODE%
:: -call exit only in case of 
if not [%EXITCODE%]==[0] exit /b %EXITCODE%
