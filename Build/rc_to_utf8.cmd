@echo off
REM Convert UTF-8-BOM encoded language RC files to pure UTF-8 (no BOM)
REM Usage: rc_to_utf8.cmd

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\rc_to_utf8.ps1"
