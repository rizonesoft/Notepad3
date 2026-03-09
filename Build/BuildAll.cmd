@echo off
REM Build All Platforms (Win32, x64, x64_AVX2, ARM64)
REM Usage: BuildAll.cmd [Release|Debug]

setlocal
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\BuildAll.ps1" -Configuration %CONFIG%
