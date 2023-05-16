@echo off
setlocal enableextensions
:: encoding: UTF-8
chcp 65001 >nul 2>&1

:: ===================================================================================================
:: PortableApps.com's (https://portableapps.com/development) 
::
:: This "build_np3portableapp.cmd" batch file creates: 
::	  - "Notepad3Portable_x.xx.xxx.x_zzzz.paf.exe"
::	  - "Notepad3Portable_x.xx.xxx.x_zzzz.paf.exe.7z"
::
:: ---------------------------------------------------------------------------------------------------
:: Based on PortableApps.com's Application_Template:
::    (https://downloads.sourceforge.net/portableapps/PortableApps.com_Application_Template_3.4.1.zip)
:: ---------------------------------------------------------------------------------------------------
::
:: Prerequisites: (portable) intallation of:
:: -----------------------------------------
:: + PortableApps.com App Compactor (https://portableapps.com/apps/utilities/appcompactor)
::
:: + PortableApps.com Launcher (https://portableapps.com/apps/development/portableapps.com_launcher)
::   (needed to create the Notepad3Portable.exe Launcher from the sources)
::
:: + PortableApps.com Installer (https://portableapps.com/apps/development/portableapps.com_installer)
::
:: ===================================================================================================

set NP3_LANGUAGE_SET=af-ZA be-BY de-DE el-GR en-GB es-ES es-MX fr-FR hi-IN hu-HU id-ID it-IT ja-JP ko-KR nl-NL pl-PL pt-BR pt-PT ru-RU sk-SK sv-SE tr-TR vi-VN zh-CN zh-TW

:: ===================================================================================================

:: --- Environment ---
if exist D:\PortableApps\PortableApps.comInstaller\ (
    set PORTAPP_ROOT_DIR=D:\PortableApps
) else (
    if exist D:\Rizonesoft\PortableApps\PortableApps\PortableApps.comInstaller\ (
        set PORTAPP_ROOT_DIR=D:\Rizonesoft\PortableApps\PortableApps
    ) else (
      goto :END
    )
)

set SCRIPT_DIR=%~dp0
set PORTAPP_LAUNCHER_CREATOR=%PORTAPP_ROOT_DIR%\PortableApps.comLauncher\PortableApps.comLauncherGenerator.exe
set PORTAPP_INSTALLER_CREATOR=%PORTAPP_ROOT_DIR%\PortableApps.comInstaller\PortableApps.comInstaller.exe

call :RESOLVEPATH NP3_DISTRIB_DIR %SCRIPT_DIR%..\Build
call :RESOLVEPATH NP3_DOC_DIR %SCRIPT_DIR%..\Build\Docs
call :RESOLVEPATH NP3_BUILD_SCHEMES_DIR %SCRIPT_DIR%..\Build\Themes
call :RESOLVEPATH NP3_WIN32_DIR %SCRIPT_DIR%..\Bin\Release_x86_v143
call :RESOLVEPATH NP3_X64_DIR %SCRIPT_DIR%..\Bin\Release_x64_v143

call :RESOLVEPATH NP3_GREPWIN_DIR %SCRIPT_DIR%..\grepWinNP3

call :RESOLVEPATH NP3_PORTAPP_DIR %SCRIPT_DIR%Notepad3Portable
call :RESOLVEPATH NP3_PORTAPP_INFO %NP3_PORTAPP_DIR%\App\AppInfo\appinfo
call :RESOLVEPATH NP3_PORTAPP_INSTALL %NP3_PORTAPP_DIR%\App\AppInfo\installer

call :RESOLVEPATH NP3_BUILD_VER %SCRIPT_DIR%..\Versions\build.txt
call :RESOLVEPATH NP3_BUILD_NAME %SCRIPT_DIR%_buildname.txt


:: ---------------------------------------------------------------------------------------------------

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

:: ---------------------------------------------------------------------------------------------------

:: --- Prepare Build ---
del /f /q "Notepad3Portable_*.paf.exe*"

if exist "%NP3_PORTAPP_DIR%\Data" rmdir "%NP3_PORTAPP_DIR%\Data" /S /Q

if exist "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng" rmdir "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng" /S /Q

if exist "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng" rmdir "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng" /S /Q

if not exist "%NP3_PORTAPP_DIR%\App\DefaultData\settings\" (
     mkdir "%NP3_PORTAPP_DIR%\App\DefaultData\settings\"
) else (
    del /s /f /q "%NP3_PORTAPP_DIR%\App\DefaultData\settings\*.*"
)

if not exist "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\" (
     mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\"
) else (
    del /s /f /q "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\*.*"
)

if not exist "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\crypto\" (
     mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\crypto\"
) else (
    del /s /f /q "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\crypto\*.*"
)

if not exist "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\uthash\" (
     mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\uthash\"
) else (
    del /s /f /q "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\uthash\*.*"
)

:: ---------------------------------------------------------------------------------------------------

:: Copy all current "Docs" files
copy "%NP3_DISTRIB_DIR%\Changes.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\Changes.txt" /Y /V
copy "%SCRIPT_DIR%..\License.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\License.txt" /Y /V
copy "%SCRIPT_DIR%..\Readme.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\Readme.txt" /Y /V
copy "%NP3_DOC_DIR%\*.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\*.txt" /Y /V
copy "%NP3_DOC_DIR%\crypto\*.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\crypto\*.txt" /Y /V
copy "%NP3_DOC_DIR%\uthash\*.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\uthash\*.txt" /Y /V
copy "%NP3_GREPWIN_DIR%\grepWinLicense.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\grepWinLicense.txt" /Y /V

:: Copy all current "Notepad3.exe" and "MiniPath.exe" files
copy /B "%NP3_WIN32_DIR%\Notepad3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
copy /B "%NP3_X64_DIR%\Notepad3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
copy /B "%NP3_WIN32_DIR%\minipath.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
copy /B "%NP3_X64_DIR%\minipath.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V

:: Copy all current ".ini" files
copy "%NP3_DISTRIB_DIR%\Notepad3.ini" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Notepad3.ini" /Y /V
copy "%NP3_DISTRIB_DIR%\minipath.ini" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\minipath.ini" /Y /V

:: Copy all current "Themes" files
del /s /f /q "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Themes\*.*"
xcopy "%NP3_BUILD_SCHEMES_DIR%" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Themes" /C /V /I /S /Y

:: Copy all current "lng" files
for /d %%d in (%NP3_LANGUAGE_SET%) do (
  mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\%%d"
  copy /B "%NP3_WIN32_DIR%\lng\%%d\*" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\%%d\" /Y /V
)
copy /B "%NP3_WIN32_DIR%\lng\np3lng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\" /Y /V
copy /B "%NP3_WIN32_DIR%\lng\mplng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\" /Y /V

for /d %%d in (%NP3_LANGUAGE_SET%) do (
  mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\%%d"
  copy /B "%NP3_X64_DIR%\lng\%%d\*" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\%%d\" /Y /V
) 
copy /B "%NP3_X64_DIR%\lng\np3lng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\" /Y /V
copy /B "%NP3_X64_DIR%\lng\mplng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\" /Y /V

:: Copy all current "grepWinNP3" and "np3encrypt" files
if not exist "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\gwLng\" mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\gwLng"
copy /B "%NP3_WIN32_DIR%\lng\gwLng\*.lang" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\gwLng\" /Y /V
copy /B "%NP3_WIN32_DIR%\grepWinNP3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
copy /B "%NP3_WIN32_DIR%\np3encrypt.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V

if not exist "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\gwLng\" mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\gwLng"
copy /B "%NP3_X64_DIR%\lng\gwLng\*.lang" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\gwLng\" /Y /V
copy /B "%NP3_X64_DIR%\grepWinNP3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
copy /B "%NP3_X64_DIR%\np3encrypt.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V

:: ---------------------------------------------------------------------------------------------------

call :REPLACE "xxxVERSIONxxx" "%NP3_PORTAPP_INFO%_template.ini" "%VERSION%" "%NP3_PORTAPP_INFO%_tmp.ini"

call :REPLACE "xxxDEVNAMExxx" "%NP3_PORTAPP_INFO%_tmp.ini" "%DEVNAME%" "%NP3_PORTAPP_INFO%.ini"
call :REPLACE "xxxDEVNAMExxx" "%NP3_PORTAPP_INSTALL%_template.ini" "" "%NP3_PORTAPP_INSTALL%.ini"

del /f /q "%NP3_PORTAPP_INFO%_tmp.ini"

:: ---------------------------------------------------------------------------------------------------

:: --- build Launcher and Installer Package ---

:: - build Launcher -
"%PORTAPP_LAUNCHER_CREATOR%" "%NP3_PORTAPP_DIR%"

:: Signing "Notepad3Portable.exe" (".exe" file is created by "Launcher")
:: call %SCRIPT_DIR%Signing_for_NP3P_1st_EXE.cmd

:: - build Installer -
"%PORTAPP_INSTALLER_CREATOR%" "%NP3_PORTAPP_DIR%"

:: Signing "Notepad3Portable_x.xx.xxx.x_zzzz.paf.exe" (".paf.exe" file is created by "Installer")
:: call %SCRIPT_DIR%Signing_for_NP3P_2nd_EXE.cmd

:: Creation of a "7-Zip" file by appending the extension ".7z"
set Notepad3Portable.paf.exe=%SCRIPT_DIR%Notepad3Portable_%VERSION%%DEVNAME%.paf.exe
if exist %Notepad3Portable.paf.exe% (
    copy /B %Notepad3Portable.paf.exe% %Notepad3Portable.paf.exe%.7z /Y /V
) else (
    echo. "Notepad3Portable_x.xx.xxx.x_yyyy.paf.exe" does not exist
)

:: ===================================================================================================
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
    goto :EOF
:: ---------------------------------------------------------------------------------------------------

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
    goto :EOF
:: ---------------------------------------------------------------------------------------------------

:GETFILEVER
    set "file=%~1"
    if not defined file goto :EOF
    if not exist "%file%" goto :EOF
    set "FILEVER="
    for /F "tokens=2 delims==" %%a in ('
        wmic datafile where name^="%file:\=\\%" Get Version /value 
    ') do set "FILEVER=%%a"
    ::echo %file% = %FILEVER% 
    goto :EOF
:: ---------------------------------------------------------------------------------------------------

:GETBUILD
    set /p nxbuild=<%NP3_BUILD_VER%
    set /a BUILD = %nxbuild% - 1
    set /p DEVNAME=<%NP3_BUILD_NAME%
    set DEVNAME=%DEVNAME:"=%
    goto :EOF
:: ---------------------------------------------------------------------------------------------------

rem Resolve path to absolute.
rem Param 1: Name of output variable.
rem Param 2: Path to resolve.
rem Return: Resolved absolute path.
:RESOLVEPATH
    set %1=%~dpfn2
    goto :EOF
:: ---------------------------------------------------------------------------------------------------
    
:: ===================================================================================================
:END
endlocal
::pause
::exit
:: ===================================================================================================
