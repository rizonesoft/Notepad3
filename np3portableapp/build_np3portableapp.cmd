@echo off
setlocal
:: ====================================================================================================================
:: Build batch to create a PortableApps.com's (https://portableapps.com/development) 
::
::                              Notepad3Portable
::
:: --------------------------------------------------------------------------------------------------------------------
:: Based on PortableApps.com's Application_Template
::    (https://downloads.sourceforge.net/portableapps/PortableApps.com_Application_Template_3.4.1.zip)
:: --------------------------------------------------------------------------------------------------------------------
:: Prerequisites: (portable) intallation of:
:: -----------------------------------------
:: + PortableApps.com Launcher (https://portableapps.com/apps/development/portableapps.com_launcher)
::   (needed to create the Notepad3Portable.exe Launcher from the sources)
::
:: + PortableApps.com Installer (https://portableapps.com/apps/development/portableapps.com_installer)
::
:: - PortableApps.com App Compactor (optional - not used yet) (https://portableapps.com/apps/utilities/appcompactor)
::
:: ====================================================================================================================
:: TODO:
:: - (release) needs verion patcher for:  .\Notepad3Portable\App\AppInfo\appinfo.ini
:: - (release) needs version patcher for 
:: - (release) needs release version of Splasch img:  .\Notepad3Portable\App\AppInfo\Launcher\Splash.jpg
:: - (release) adapt help files:  .\Notepad3Portable\Other\Help\
:: - (release) review all distributed (Installe) text files
:: - 
:: - (optional?) needs distribution process to PortableApps.com's repository

:: ====================================================================================================================

:: --- Environment ---
set VERSION=2.17.1008.550

set SCRIPT_DIR=%~dp0
set PORTAPP_ROOT_DIR=D:\PortableApps
set PORTAPP_LAUNCHER_CREATOR=%PORTAPP_ROOT_DIR%\PortableApps.comLauncher\PortableApps.comLauncherGenerator.exe
set PORTAPP_INSTALLER_CREATOR=%PORTAPP_ROOT_DIR%\PortableApps.comInstaller\PortableApps.comInstaller.exe

set NP3_DISTRIB_DIR=%SCRIPT_DIR%..\distrib
set NP3_WIN32_DIR=%SCRIPT_DIR%..\Bin\Release_x86_v141
set NP3_X64_DIR=%SCRIPT_DIR%..\Bin\Release_x64_v141

set NP3_PORTAPP_DIR=%SCRIPT_DIR%Notepad3Portable
set NP3_PORTAPP_INFO=%NP3_PORTAPP_DIR%\App\AppInfo\appinfo

:: --------------------------------------------------------------------------------------------------------------------

:: --- Prepare Build ---

copy "%NP3_DISTRIB_DIR%\Notepad3.ini" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Notepad3.ini" /Y /V
copy "%NP3_DISTRIB_DIR%\minipath.ini" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\minipath.ini" /Y /V

copy /B "%NP3_WIN32_DIR%\Notepad3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\" /Y /V
copy /B "%NP3_WIN32_DIR%\minipath.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\" /Y /V
copy /B "%NP3_WIN32_DIR%\np3encrypt.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\" /Y /V

copy /B "%NP3_X64_DIR%\Notepad3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
copy /B "%NP3_X64_DIR%\minipath.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
copy /B "%NP3_X64_DIR%\np3encrypt.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V

call :REPLACE "xxxVERSIONxxx" "%NP3_PORTAPP_INFO%_template.ini" "%VERSION%" "%NP3_PORTAPP_INFO%.ini"

:: --------------------------------------------------------------------------------------------------------------------

:: --- build Launcher and Installer Package ---

:: - build Launcher -
"%PORTAPP_LAUNCHER_CREATOR%" "%NP3_PORTAPP_DIR%"

:: - build Installer -
"%PORTAPP_INSTALLER_CREATOR%" "%NP3_PORTAPP_DIR%"


:: ====================================================================================================================
goto :END
:: REPLACE  strg(%1)  srcfile(%2)  replstrg(%3)  dstfile(%4) 
:REPLACE
if exist "%~4" del /F /Q "%~4"
type NUL > "%~4"
for /f "tokens=1,* delims=¶" %%A in (%~2) do (
    set string=%%A
    setlocal EnableDelayedExpansion
    set modified=!string:%~1=%~3!
    >> "%~4" echo(!modified!
    endlocal
)
goto:EOF

:: ====================================================================================================================
:END
::pause
endlocal
::exit
:: ====================================================================================================================
