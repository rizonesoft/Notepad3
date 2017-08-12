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
:: - 
:: - (optional?) needs distribution process to PortableApps.com's repository

:: ====================================================================================================================

:: --- Environment ---

set SCRIPT_DIR=%~dp0
set PORTAPP_ROOT_DIR=D:\PortableApps\
set PORTAPP_LAUNCHER_CREATOR=%PORTAPP_ROOT_DIR%PortableApps.comLauncher\PortableApps.comLauncherGenerator.exe
set PORTAPP_INSTALLER_CREATOR=%PORTAPP_ROOT_DIR%PortableApps.comInstaller\PortableApps.comInstaller.exe

set NP3_DISTRIB_DIR=%SCRIPT_DIR%..\distrib\
set NP3_WIN32_DIR=%SCRIPT_DIR%..\Bin\Release_x86_v141_xp\
set NP3_X64_DIR=%SCRIPT_DIR%..\Bin\Release_x64_v141_xp\

set NP3_PORTAPP_DIR=%SCRIPT_DIR%Notepad3Portable\App\

:: --------------------------------------------------------------------------------------------------------------------

:: --- Prepare Build ---

xcopy "%NP3_DISTRIB_DIR%Notepad3.ini" "%NP3_PORTAPP_DIR%DefaultData\settings\Notepad3.ini" /Y /V /C /R
xcopy "%NP3_DISTRIB_DIR%minipath.ini" "%NP3_PORTAPP_DIR%DefaultData\settings\minipath.ini" /Y /V /C /R

xcopy "%NP3_WIN32_DIR%Notepad3.exe" "%NP3_PORTAPP_DIR%Notepad3\" /Y /V /C /R
xcopy "%NP3_WIN32_DIR%minipath.exe" "%NP3_PORTAPP_DIR%Notepad3\" /Y /V /C /R
xcopy "%NP3_WIN32_DIR%np3encrypt.exe" "%NP3_PORTAPP_DIR%Notepad3\" /Y /V /C /R

xcopy "%NP3_X64_DIR%Notepad3.exe" "%NP3_PORTAPP_DIR%Notepad3\x64\" /Y /V /C /R
xcopy "%NP3_X64_DIR%minipath.exe" "%NP3_PORTAPP_DIR%Notepad3\x64\" /Y /V /C /R
xcopy "%NP3_X64_DIR%np3encrypt.exe" "%NP3_PORTAPP_DIR%Notepad3\x64\" /Y /V /C /R

:: --------------------------------------------------------------------------------------------------------------------

:: --- build Launcher and Installer Package ---

:: ====================================================================================================================
:END
::pause
::popd
endlocal
exit
:: ====================================================================================================================
