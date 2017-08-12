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

set SCRIPT_DIR=%~dp0
set PORTAPP_ROOT_DIR=D:\PortableApps\
set PORTAPP_LAUNCHER_CREATOR=%PORTAPP_ROOT_DIR%PortableApps.comLauncher\PortableApps.comLauncherGenerator.exe
set PORTAPP_INSTALLER_CREATOR=%PORTAPP_ROOT_DIR%PortableApps.comInstaller\PortableApps.comInstaller.exe

set NP3_DISTRIB_DIR=%SCRIPT_DIR%..\distrib\
set NP3_WIN32_DIR=%SCRIPT_DIR%..\Bin\Release_x86_v141_xp\
set NP3_X64_DIR=%SCRIPT_DIR%..\Bin\Release_x64_v141_xp\

set NP3_PORTAPP_DIR=%SCRIPT_DIR%Notepad3Portable\

:: --------------------------------------------------------------------------------------------------------------------

:: --- Prepare Build ---

copy "%NP3_DISTRIB_DIR%Notepad3.ini" "%NP3_PORTAPP_DIR%App\DefaultData\settings\Notepad3.ini" /Y /V
copy "%NP3_DISTRIB_DIR%minipath.ini" "%NP3_PORTAPP_DIR%App\DefaultData\settings\minipath.ini" /Y /V

copy /B "%NP3_WIN32_DIR%Notepad3.exe" /B "%NP3_PORTAPP_DIR%App\Notepad3\" /Y /V
copy /B "%NP3_WIN32_DIR%minipath.exe" /B "%NP3_PORTAPP_DIR%App\Notepad3\" /Y /V
copy /B "%NP3_WIN32_DIR%np3encrypt.exe" /B "%NP3_PORTAPP_DIR%App\Notepad3\" /Y /V

copy /B "%NP3_X64_DIR%Notepad3.exe" /B "%NP3_PORTAPP_DIR%App\Notepad3\x64\" /Y /V
copy /B "%NP3_X64_DIR%minipath.exe" /B "%NP3_PORTAPP_DIR%App\Notepad3\x64\" /Y /V
copy /B "%NP3_X64_DIR%np3encrypt.exe" /B "%NP3_PORTAPP_DIR%App\Notepad3\x64\" /Y /V

:: --------------------------------------------------------------------------------------------------------------------

:: --- build Launcher and Installer Package ---

:: - build Launcher -
:: cmdline version of Launcher Generator does not work (the same as manual started GUI version)
:: so manually generate Notepad3Portable.exe for now, 
:: trying to figure out, what is going wrong later ...
:: ~~~ "%PORTAPP_LAUNCHER_CREATOR%" "%NP3_PORTAPP_DIR%"
:: --- instead of generating launcher, you have to use manually create Notepad3Portable.exe
echo. Please choose dir "%NP3_PORTAPP_DIR%" to create Launcher (Notepad3Portable.exe)
"%PORTAPP_LAUNCHER_CREATOR%"

:: - build Installer -
:: unfortunately, the cmdline version of Installer Generator does not work either
:: so manually generate Notepad3Portable_2.0.2.xxx_English.paf.exe for now, 
::~~~"%PORTAPP_INSTALLER_CREATOR%" "%NP3_PORTAPP_DIR%"
echo. Please choose dir "%NP3_PORTAPP_DIR%" to create Installer (Notepad3Portable_2.0.2.xxx_English.paf.exe)
%PORTAPP_INSTALLER_CREATOR%


:: ====================================================================================================================
:END
pause
endlocal
::exit
:: ====================================================================================================================
