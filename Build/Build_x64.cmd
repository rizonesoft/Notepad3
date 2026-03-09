@echo off
REM Build x64 Platform
REM Usage: Build_x64.cmd [Release|Debug]

setlocal
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\Build.ps1" -Platform x64 -Configuration %CONFIG%
