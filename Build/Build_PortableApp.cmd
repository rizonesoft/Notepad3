@echo off
REM Build PortableApps Package
REM Usage: Build_PortableApp.cmd [arguments passed to BuildPortableApp.ps1]

setlocal

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\BuildPortableApp.ps1" %*
