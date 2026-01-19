@echo off
REM Build Win32 (x86) Platform
REM Usage: Build_Win32.cmd [Release|Debug]

setlocal
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\Build.ps1" -Platform Win32 -Configuration %CONFIG%
