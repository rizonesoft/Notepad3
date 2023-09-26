@echo off
setlocal EnableDelayedExpansion
set _THISDIR_=%~dp0
rem remove trailing back-slash
set _THISDIR_=!_THISDIR_:~,-1!
rem  transform to foward-slash
::~set "_THISDIR_=%_THISDIR_:\=/%"
pushd %_THISDIR_%
::cls


rem set _PYTHON_EXE=c:\PortablePrograms\Python396_x64_emb\python.exe
rem call :RESOLVE_PATH _PYTHON_EXE "%_THISDIR_%..\..\..\..\_python_emb\python.exe"
set _PYTHON_EXE=python.exe

::set _CMD_="%_PYTHON_EXE%" -I -S -m "%~dpn0"
set _CMD_="%_PYTHON_EXE%" -I -S "%~dpn0.py"

set _EXITCODE_=0
echo.Calling: %_CMD_%
%_CMD_%
if not [%ERRORLEVEL%] == [0] (
    set _EXITCODE_=%ERRORLEVEL%
)


goto :END
rem ----------------------------------------------------------------------------


rem   call :RESOLVE_PATH   WORKINGDIRPARENT   ".."
:RESOLVE_PATH
    set %1=%~dpfn2
goto:eof
rem ----------------------------------------------------------------------------


:END
::pause
popd
echo.ERRORLEVEL=%_EXITCODE_%
endlocal & set _EXITCODE_=%_EXITCODE_%
:: -call exit only in case of 
if not [%_EXITCODE_%]==[0] exit /b %_EXITCODE_%
