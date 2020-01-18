@echo off
setlocal enableextensions
:: encoding: UTF-8
chcp 65001  >nul 2>&1

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
:: + PortableApps.com App Compactor (https://portableapps.com/apps/utilities/appcompactor)
::
:: + PortableApps.com Launcher (https://portableapps.com/apps/development/portableapps.com_launcher)
::   (needed to create the Notepad3Portable.exe Launcher from the sources)
::
:: + PortableApps.com Installer (https://portableapps.com/apps/development/portableapps.com_installer)
::
:: ====================================================================================================================
:: TODO:
:: - (release) needs release version of Splasch img:  .\Notepad3Portable\App\AppInfo\Launcher\Splash.jpg
:: - (release) adapt help files:  .\Notepad3Portable\Other\Help\
:: - (release) review all distributed (Installed) text files
:: - 
:: - (optional?) needs distribution process to PortableApps.com's repository

:: ====================================================================================================================

set NP3_LANGUAGE_SET=af-ZA be-BY de-DE en-GB es-ES fr-FR hu-HU it-IT ja-JP ko-KR nl-NL pl-PL pt-BR ru-RU sk-SK sv-SE tr-TR zh-CN

:: ====================================================================================================================

:: --- Environment ---
set SCRIPT_DIR=%~dp0
set PORTAPP_ROOT_DIR=D:\PortableApps
::set PORTAPP_ROOT_DIR=D:\Rizonesoft\PortableApps\PortableApps
set PORTAPP_APP_COMPACTOR=%PORTAPP_ROOT_DIR%\PortableApps.comAppCompactor\PortableApps.comAppCompactor.exe
set PORTAPP_LAUNCHER_CREATOR=%PORTAPP_ROOT_DIR%\PortableApps.comLauncher\PortableApps.comLauncherGenerator.exe
set PORTAPP_INSTALLER_CREATOR=%PORTAPP_ROOT_DIR%\PortableApps.comInstaller\PortableApps.comInstaller.exe

call :RESOLVEPATH NP3_DISTRIB_DIR %SCRIPT_DIR%..\Build
call :RESOLVEPATH NP3_DOC_DIR %SCRIPT_DIR%..\doc
::call :RESOLVEPATH NP3_THEMES_DIR %SCRIPT_DIR%..\themes
call :RESOLVEPATH NP3_BUILD_SCHEMES_DIR %SCRIPT_DIR%..\Build\themes
::call :RESOLVEPATH NP3_WIN32_DIR %SCRIPT_DIR%..\Bin\Release_x86_v141
::call :RESOLVEPATH NP3_X64_DIR %SCRIPT_DIR%..\Bin\Release_x64_v141
call :RESOLVEPATH NP3_WIN32_DIR %SCRIPT_DIR%..\Bin\Release_x86_v142
call :RESOLVEPATH NP3_X64_DIR %SCRIPT_DIR%..\Bin\Release_x64_v142

call :RESOLVEPATH NP3_PORTAPP_DIR %SCRIPT_DIR%Notepad3Portable
call :RESOLVEPATH NP3_PORTAPP_INFO %NP3_PORTAPP_DIR%\App\AppInfo\appinfo
call :RESOLVEPATH NP3_PORTAPP_INSTALL %NP3_PORTAPP_DIR%\App\AppInfo\installer

call :RESOLVEPATH NP3_BUILD_VER %SCRIPT_DIR%..\Versions\build.txt
call :RESOLVEPATH NP3_BUILD_NAME %SCRIPT_DIR%_buildname.txt

:: --------------------------------------------------------------------------------------------------------------------

set YY=00
set MM=00
set DD=00
call :GETDATE
set BUILD=0
set DEVNAME=
call :GETBUILD

:: VERSION fallback from build date
set VERSION=5.%YY%.%MM%%DD%.%BUILD%

set FILEVER=
call :GETFILEVER "%NP3_WIN32_DIR%\Notepad3.exe"
if defined FILEVER set VERSION=%FILEVER%


::echo.VERSION=%VERSION%
::pause
::goto :END

:: --------------------------------------------------------------------------------------------------------------------

:: --- Prepare Build ---

if not exist "%NP3_PORTAPP_DIR%\App\DefaultData\settings\" mkdir "%NP3_PORTAPP_DIR%\App\DefaultData\settings\"

copy "%NP3_DISTRIB_DIR%\Notepad3.ini" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Notepad3.ini" /Y /V
copy "%NP3_DISTRIB_DIR%\minipath.ini" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\minipath.ini" /Y /V
copy "%NP3_DISTRIB_DIR%\Changes.txt" "%NP3_PORTAPP_DIR%\Other\Help\Changes.txt" /Y /V
copy "%NP3_DISTRIB_DIR%\License.txt" "%NP3_PORTAPP_DIR%\Other\Help\License.txt" /Y /V
copy "%NP3_DISTRIB_DIR%\Readme.txt" "%NP3_PORTAPP_DIR%\Other\Help\Readme.txt" /Y /V
copy "%NP3_DOC_DIR%\Notepad3.txt" "%NP3_PORTAPP_DIR%\Other\Help\Notepad3.txt" /Y /V
copy "%NP3_DOC_DIR%\KeyboardShortcuts.txt" "%NP3_PORTAPP_DIR%\Other\Help\KeyboardShortcuts.txt" /Y /V
copy "%NP3_DOC_DIR%\crypto\encryption-doc.txt" "%NP3_PORTAPP_DIR%\Other\Help\encryption-doc.txt" /Y /V


:: clear Notepad3 binary dir (except .ini files)
for /f %%f in ('dir "%NP3_PORTAPP_DIR%\App\Notepad3\" /b /a-d /s ^| findstr /v .ini') do del "%%f"

for /d %%d in (%NP3_LANGUAGE_SET%) do (
  mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\%%d"
  copy /B "%NP3_WIN32_DIR%\lng\%%d\*" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\%%d\" /Y /V
)
copy /B "%NP3_WIN32_DIR%\lng\np3lng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\" /Y /V
copy /B "%NP3_WIN32_DIR%\lng\mplng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\" /Y /V

copy /B "%NP3_WIN32_DIR%\Notepad3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
if exist %NP3_WIN32_DIR%\Scintilla.dll (
    copy /B "%NP3_WIN32_DIR%\Scintilla.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
) else (
    echo. Scintilla.dll does not exist
)
copy /B "%NP3_WIN32_DIR%\minipath.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
::copy /B "%NP3_WIN32_DIR%\np3encrypt.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
::copy /B "%NP3_WIN32_DIR%\ced.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V

::copy /B "%NP3_DISTRIB_DIR%\Update\wyUpdate\86\client.wyc" /B "%NP3_PORTAPP_DIR%\App\Notepad3\" /Y /V
::copy /B "%NP3_DISTRIB_DIR%\Update\wyUpdate\86\wyUpdate.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\" /Y /V

::del /s /f /q "%NP3_PORTAPP_DIR%\App\themes\*.*"
::for /f "delims=" %%d in ('dir /ad /b "%NP3_PORTAPP_DIR%\App\themes\"') do rmdir /s /q "%NP3_PORTAPP_DIR%\App\themes\%%d"
::xcopy "%NP3_THEMES_DIR%" "%NP3_PORTAPP_DIR%\App\themes" /C /V /I /S /Y

del /s /f /q "%NP3_PORTAPP_DIR%\App\DefaultData\settings\themes\*.*"
xcopy "%NP3_BUILD_SCHEMES_DIR%" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\themes" /C /V /I /S /Y

for /d %%d in (%NP3_LANGUAGE_SET%) do (
  mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\%%d"
  copy /B "%NP3_X64_DIR%\lng\%%d\*" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\%%d\" /Y /V
)
copy /B "%NP3_X64_DIR%\lng\np3lng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\" /Y /V
copy /B "%NP3_X64_DIR%\lng\mplng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\" /Y /V

copy /B "%NP3_X64_DIR%\Notepad3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
if exist %NP3_X64_DIR%\Scintilla.dll (
    copy /B "%NP3_X64_DIR%\Scintilla.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
) else (
    echo. Scintilla.dll does not exist
)
copy /B "%NP3_X64_DIR%\minipath.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
::copy /B "%NP3_X64_DIR%\np3encrypt.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
::copy /B "%NP3_X64_DIR%\ced.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V

::copy /B "%NP3_DISTRIB_DIR%\Update\wyUpdate\64\client.wyc" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
::copy /B "%NP3_DISTRIB_DIR%\Update\wyUpdate\64\wyUpdate.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V


call :REPLACE "xxxVERSIONxxx" "%NP3_PORTAPP_INFO%_template.ini" "%VERSION%" "%NP3_PORTAPP_INFO%_tmp.ini"

:: DEVNAME need some more PortableApps preparation
::~  call :REPLACE "xxxDEVNAMExxx" "%NP3_PORTAPP_INFO%_tmp.ini" "_%DEVNAME%" "%NP3_PORTAPP_INFO%.ini"
::~  call :REPLACE "xxxDEVNAMExxx" "%NP3_PORTAPP_INSTALL%_template.ini" "_%DEVNAME%" "%NP3_PORTAPP_INSTALL%.ini"
call :REPLACE "xxxDEVNAMExxx" "%NP3_PORTAPP_INFO%_tmp.ini" "%DEVNAME%" "%NP3_PORTAPP_INFO%.ini"
call :REPLACE "xxxDEVNAMExxx" "%NP3_PORTAPP_INSTALL%_template.ini" "" "%NP3_PORTAPP_INSTALL%.ini"

del /F "%NP3_PORTAPP_INFO%_tmp.ini"

:: --------------------------------------------------------------------------------------------------------------------

:: --- build Launcher and Installer Package ---

:: - compact app -
::"%PORTAPP_APP_COMPACTOR%" "%NP3_PORTAPP_DIR%"

:: - build Launcher -
"%PORTAPP_LAUNCHER_CREATOR%" "%NP3_PORTAPP_DIR%"

:: - build Installer -
"%PORTAPP_INSTALLER_CREATOR%" "%NP3_PORTAPP_DIR%"

:: rename
::echo rename "%SCRIPT_DIR%Notepad3Portable_%VERSION%.paf.exe" "Notepad3Portable_%VERSION%_%DEVNAME%.paf.exe"
::rename "%SCRIPT_DIR%Notepad3Portable_%VERSION%.paf.exe" "Notepad3Portable_%VERSION%_%DEVNAME%.paf.exe"

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
:: --------------------------------------------------------------------------------------------------------------------

:GETDATE
    for /f "tokens=2 delims==" %%a in ('
        wmic OS Get localdatetime /value
    ') do set "dt=%%a"
    set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
    set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"
    ::set "datestamp=%YYYY%%MM%%DD%" & set "timestamp=%HH%%Min%%Sec%"
    ::set "fullstamp=%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"
    ::echo datestamp: "%datestamp%"
    ::echo timestamp: "%timestamp%"
    ::echo fullstamp: "%fullstamp%"
    goto:EOF
:: --------------------------------------------------------------------------------------------------------------------

:GETFILEVER
    set "file=%~1"
    if not defined file goto:EOF
    if not exist "%file%" goto:EOF
    set "FILEVER="
    for /F "tokens=2 delims==" %%a in ('
        wmic datafile where name^="%file:\=\\%" Get Version /value 
    ') do set "FILEVER=%%a"
    ::echo %file% = %FILEVER% 
    goto:EOF
:: --------------------------------------------------------------------------------------------------------------------

:GETBUILD
    set /p nxbuild=<%NP3_BUILD_VER%
    set /a BUILD = %nxbuild% - 1
    set /p DEVNAME=<%NP3_BUILD_NAME%
    set DEVNAME=%DEVNAME:"=%
    goto:EOF
:: --------------------------------------------------------------------------------------------------------------------

rem Resolve path to absolute.
rem Param 1: Name of output variable.
rem Param 2: Path to resolve.
rem Return: Resolved absolute path.
:RESOLVEPATH
    set %1=%~dpfn2
    goto:EOF
:: --------------------------------------------------------------------------------------------------------------------
    
:: ====================================================================================================================
:END
endlocal
::pause
::exit
:: ====================================================================================================================
