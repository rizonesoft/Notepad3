@if (@CodeSection == @Batch) @then

@echo off
setlocal EnableExtensions EnableDelayedExpansion

set _SCRIPTDIR_=%~dp0
pushd %_SCRIPTDIR_%

call :RESOLVE_PATH  _EXE_PATH_  "..\..\Bin\Debug_x64_v143\Notepad3.exe"
::call :RESOLVE_PATH  _EXE_PATH_  "..\..\Bin\Release_x64_v143\Notepad3.exe"

call :RESOLVE_PATH  _LOG_FILE_  ".\log.txt"

rem - create logfile
echo. >"%_LOG_FILE_%"

rem - start Notepad3
start "NP3" /D "%_SCRIPTDIR_%" /B /I "%_EXE_PATH_%" /l "%_LOG_FILE_%"

rem - generate logfile
for /L %%a in (1,1,100) do (
    echo. New Log Line %%a >> "%_LOG_FILE_%"
    call :WAITFOR 250
)


rem -----------------------------------------------------------------------------
goto :END
rem -----------------------------------------------------------------------------
rem   Subroutines
rem -----------------------------------------------------------------------------


:WAITFOR
cscript /nologo /e:JScript "%~f0" "%~1"
goto:eof
rem ----------------------------------------------------------------------------


rem - Param 1: Name of output variable   Param 2: string to strip quotes from
:STRIP_QUOTES
    set %1=%~2
goto:eof
rem ----------------------------------------------------------------------------


rem - Resolve path to absolute.
rem - Param 1: Name of output variable   Param 2: Path to resolve.
rem - Return: Resolved absolute path.
rem   call :RESOLVE_PATH   WORKINGDIRPARENT   ".."
:RESOLVE_PATH
    set %1=%~dpfn2
goto:eof
rem ----------------------------------------------------------------------------

rem -----------------------------------------------------------------------------
:END
::pause
popd
rem - make _EXITCODE_ survive 'endlocal'
endlocal & set _RETURNVALUE_=%_EXITCODE_%
::echo.EXITCODE=%_RETURNVALUE_%
exit /b %_RETURNVALUE_%
goto:eof

@end // end batch / begin JScript hybrid code
WSH.Sleep(WSH.Arguments(0));
