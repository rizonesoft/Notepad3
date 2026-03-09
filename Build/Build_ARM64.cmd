@echo off
REM Build ARM64 Platform
REM Usage: Build_ARM64.cmd [Release|Debug]

setlocal
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\Build.ps1" -Platform ARM64 -Configuration %CONFIG%
