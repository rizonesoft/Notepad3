@echo off
setlocal enableextensions
set SDIR=%~dp0

set DEVENV=C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.exe

set SLN=%SDIR%Notepad3.sln

set ASAN_OPTIONS=windows_hook_legacy_allocators=false

"%DEVENV%" "%SLN%"

endlocal
