@echo off
REM Clean All Build Outputs
REM Usage: Clean.cmd

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\Clean.ps1"
