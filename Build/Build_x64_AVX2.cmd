@echo off
REM Build x64 with AVX2 Optimizations
REM Usage: Build_x64_AVX2.cmd [Release|Debug]
REM Note: Output goes to Bin\Release_x64_AVX2_v143\

setlocal
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\Build.ps1" -Platform x64_AVX2 -Configuration %CONFIG%
