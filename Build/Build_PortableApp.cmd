@echo off
REM Build PortableApps Package
REM Usage: Build_PortableApp.cmd [arguments passed to BuildPortableApp.ps1]

setlocal
set PortAppsDir=D:\PortableApps

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\BuildPortableApp.ps1" -PortableAppsDir "%PortAppsDir%" %*
