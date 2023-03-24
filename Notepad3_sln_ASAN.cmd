@echo off
setlocal enableextensions
set SDIR=%~dp0

:: --- Check version of MS Visual Studio ---
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\" (
    set DEVENV=C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.exe
) else (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\" (
        set DEVENV=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe
    ) else (
      goto :END
    )
)

set SLN=%SDIR%Notepad3.sln

set ASAN_OPTIONS=windows_hook_legacy_allocators=false

start "devenv" /MAX /B  "%DEVENV%" "%SLN%"

:END
endlocal
::pause
exit
