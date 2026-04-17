@echo off
setlocal enableextensions
:: encoding: UTF-8
chcp 65001 >nul 2>&1

:: ===================================================================================================
:: PortableApps.com's (https://portableapps.com/development)
::
:: This "build_np3portableapp.cmd" batch file creates:
::	  - "Notepad3Portable_x.xx.xxx.x_zzzz.paf.exe"
::
:: Usage:  build_np3portableapp.cmd [PortableAppsDir]
::         PortableAppsDir — optional path to the PortableApps platform root
::                           (auto-detected if omitted)
::
:: ---------------------------------------------------------------------------------------------------
:: Based on PortableApps.com's Application_Template:
:: (https://sourceforge.net/projects/portableapps/files/PortableApps.com%20Template/PortableApps.com_Application_Template_3.9.0.zip)
:: ---------------------------------------------------------------------------------------------------
::
:: Prerequisites: (portable) installation of:
:: -----------------------------------------
:: + PortableApps.com Launcher (https://portableapps.com/apps/development/portableapps.com_launcher)
::   (needed to create the Notepad3Portable.exe Launcher from the sources)
::
:: + PortableApps.com Installer (https://portableapps.com/apps/development/portableapps.com_installer)
::
:: ===================================================================================================

:: ===================================================================================================

:: --- Environment ---

echo.
echo === Notepad3 PortableApps Package Builder ===
echo.

set SCRIPT_DIR=%~dp0

:: Accept optional PortableApps dir from command line, otherwise auto-detect
if not "%~1"=="" (
    if exist "%~1\PortableApps.comInstaller\" set "PORTAPP_ROOT_DIR=%~1" && goto :PORTAPP_FOUND
    echo [ERROR] Specified PortableApps dir not found: %~1
    exit /b 1
)
if exist %~d0\PortableApps\PortableApps.comInstaller\ set "PORTAPP_ROOT_DIR=%~d0\PortableApps" && goto :PORTAPP_FOUND
if exist %~d0\Rizonesoft\PortableApps\PortableApps\PortableApps.comInstaller\ set "PORTAPP_ROOT_DIR=%~d0\Rizonesoft\PortableApps\PortableApps" && goto :PORTAPP_FOUND
echo [ERROR] Can't find PortableApps Platform!
echo         Provide the path as argument: %~nx0 [PortableAppsDir]
exit /b 1

:PORTAPP_FOUND

echo --- PortableApps Platform: %PORTAPP_ROOT_DIR%

set PORTAPP_LAUNCHER_CREATOR=%PORTAPP_ROOT_DIR%\PortableApps.comLauncher\PortableApps.comLauncherGenerator.exe
set PORTAPP_INSTALLER_CREATOR=%PORTAPP_ROOT_DIR%\PortableApps.comInstaller\PortableApps.comInstaller.exe

if not exist "%PORTAPP_LAUNCHER_CREATOR%" (
    echo [ERROR] Launcher generator not found: %PORTAPP_LAUNCHER_CREATOR%
    exit /b 1
)
if not exist "%PORTAPP_INSTALLER_CREATOR%" (
    echo [ERROR] Installer creator not found: %PORTAPP_INSTALLER_CREATOR%
    exit /b 1
)

call :RESOLVEPATH NP3_DISTRIB_DIR %SCRIPT_DIR%..\Build
call :RESOLVEPATH NP3_DOC_DIR %SCRIPT_DIR%..\Build\Docs
call :RESOLVEPATH NP3_BUILD_SCHEMES_DIR %SCRIPT_DIR%..\Build\Themes

:: --- Auto-detect PlatformToolset from build output ---
set NP3_TOOLSET=
for /f "delims=" %%d in ('dir /b /ad "%SCRIPT_DIR%..\Bin\Release_x64_v*" 2^>nul') do (
    for /f "tokens=3 delims=_" %%t in ("%%d") do set NP3_TOOLSET=%%t
)
if not defined NP3_TOOLSET (
    echo [ERROR] No build output found in Bin\Release_x64_v*
    echo         Build Notepad3 first - x64 and Win32 Release.
    exit /b 1
)
echo --- Detected PlatformToolset: %NP3_TOOLSET%

call :RESOLVEPATH NP3_WIN32_DIR %SCRIPT_DIR%..\Bin\Release_x86_%NP3_TOOLSET%
call :RESOLVEPATH NP3_X64_DIR %SCRIPT_DIR%..\Bin\Release_x64_%NP3_TOOLSET%

:: Verify build outputs exist
if not exist "%NP3_WIN32_DIR%\Notepad3.exe" (
    echo [ERROR] x86 build output not found: %NP3_WIN32_DIR%\Notepad3.exe
    exit /b 1
)
if not exist "%NP3_X64_DIR%\Notepad3.exe" (
    echo [ERROR] x64 build output not found: %NP3_X64_DIR%\Notepad3.exe
    exit /b 1
)
if not exist "%NP3_WIN32_DIR%\minipath.exe" (
    echo [ERROR] x86 minipath not found: %NP3_WIN32_DIR%\minipath.exe
    exit /b 1
)
if not exist "%NP3_X64_DIR%\minipath.exe" (
    echo [ERROR] x64 minipath not found: %NP3_X64_DIR%\minipath.exe
    exit /b 1
)

:: --- Auto-detect available language locales from build output ---
set "NP3_LANGUAGE_SET="
for /f "delims=" %%d in ('dir /b /ad "%NP3_X64_DIR%\lng" 2^>nul ^| findstr /r "^[a-z][a-z]-[A-Z][A-Z]$"') do call set "NP3_LANGUAGE_SET=%%NP3_LANGUAGE_SET%% %%d"
if not defined NP3_LANGUAGE_SET (
    echo [ERROR] No locale directories found in %NP3_X64_DIR%\lng
    exit /b 1
)

call :RESOLVEPATH NP3_PORTAPP_DIR %SCRIPT_DIR%Notepad3Portable
call :RESOLVEPATH NP3_PORTAPP_INFO %NP3_PORTAPP_DIR%\App\AppInfo\appinfo
call :RESOLVEPATH NP3_PORTAPP_INSTALL %NP3_PORTAPP_DIR%\App\AppInfo\installer

call :RESOLVEPATH NP3_BUILD_VER %SCRIPT_DIR%..\Versions\build.txt
call :RESOLVEPATH NP3_BUILD_NAME %SCRIPT_DIR%_buildname.txt


:: ---------------------------------------------------------------------------------------------------
:: Step 1: Determine version
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 1: Determine Version ---

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

echo --- Version:  %VERSION%
echo --- DevName:  "%DEVNAME%"

:: ---------------------------------------------------------------------------------------------------
:: Step 2: Clean previous build artifacts
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 2: Clean Previous Build ---

del /f /q "Notepad3Portable_*.paf.exe*" 2>nul

if exist "%NP3_PORTAPP_DIR%\Data" rmdir "%NP3_PORTAPP_DIR%\Data" /S /Q

if exist "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng" rmdir "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng" /S /Q

if exist "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng" rmdir "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng" /S /Q

if not exist "%NP3_PORTAPP_DIR%\App\DefaultData\settings\" (
     mkdir "%NP3_PORTAPP_DIR%\App\DefaultData\settings\"
) else (
    del /s /f /q "%NP3_PORTAPP_DIR%\App\DefaultData\settings\*.*" >nul 2>&1
)

if not exist "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\" (
     mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\"
) else (
    del /s /f /q "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\*.*" >nul 2>&1
)

if not exist "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\crypto\" (
     mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\crypto\"
) else (
    del /s /f /q "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\crypto\*.*" >nul 2>&1
)

if not exist "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\uthash\" (
     mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\uthash\"
) else (
    del /s /f /q "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\uthash\*.*" >nul 2>&1
)

echo     Done.

:: ---------------------------------------------------------------------------------------------------
:: Step 3: Copy documentation files
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 3: Copy Docs ---

copy "%NP3_DISTRIB_DIR%\Changes.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\Changes.txt" /Y /V
copy "%SCRIPT_DIR%..\License.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\License.txt" /Y /V
copy "%SCRIPT_DIR%..\Readme.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\Readme.txt" /Y /V
copy "%NP3_DOC_DIR%\*.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\*.txt" /Y /V
copy "%NP3_DOC_DIR%\crypto\*.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\crypto\*.txt" /Y /V
copy "%NP3_DOC_DIR%\uthash\*.txt" "%NP3_PORTAPP_DIR%\App\Notepad3\Docs\uthash\*.txt" /Y /V

:: ---------------------------------------------------------------------------------------------------
:: Step 4: Copy binaries (Notepad3.exe, minipath.exe)
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 4: Copy Binaries ---

copy /B "%NP3_WIN32_DIR%\Notepad3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
copy /B "%NP3_X64_DIR%\Notepad3.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V
copy /B "%NP3_WIN32_DIR%\minipath.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\" /Y /V
copy /B "%NP3_X64_DIR%\minipath.exe" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\" /Y /V

:: ---------------------------------------------------------------------------------------------------
:: Step 5: Copy INI and Themes
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 5: Copy INI and Themes ---

copy "%NP3_DISTRIB_DIR%\Notepad3_pap.ini" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Notepad3.ini" /Y /V
copy "%NP3_DISTRIB_DIR%\minipath.ini" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\minipath.ini" /Y /V

del /s /f /q "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Themes\*.*" 2>nul
xcopy "%NP3_BUILD_SCHEMES_DIR%" "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Themes" /C /V /I /S /Y

:: --- Create an empty directory "Favorites" ---
if not exist "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Favorites\" (
    mkdir "%NP3_PORTAPP_DIR%\App\DefaultData\settings\Favorites\"
)

:: ---------------------------------------------------------------------------------------------------
:: Step 6: Copy language files
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 6: Copy Language Files ---

for /d %%d in (%NP3_LANGUAGE_SET%) do (
  mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\%%d" 2>nul
  copy /B "%NP3_WIN32_DIR%\lng\%%d\*" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\%%d\" /Y /V
)
copy /B "%NP3_WIN32_DIR%\lng\np3lng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\" /Y /V
copy /B "%NP3_WIN32_DIR%\lng\mplng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\" /Y /V

for /d %%d in (%NP3_LANGUAGE_SET%) do (
  mkdir "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\%%d" 2>nul
  copy /B "%NP3_X64_DIR%\lng\%%d\*" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\%%d\" /Y /V
)

copy /B "%NP3_X64_DIR%\lng\np3lng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\" /Y /V
copy /B "%NP3_X64_DIR%\lng\mplng.dll" /B "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\" /Y /V

if exist "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\en-US" rmdir "%NP3_PORTAPP_DIR%\App\Notepad3\x86\lng\en-US" /S /Q
if exist "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\en-US" rmdir "%NP3_PORTAPP_DIR%\App\Notepad3\x64\lng\en-US" /S /Q

:: ---------------------------------------------------------------------------------------------------
:: Step 7: Process INI templates (appinfo.ini, installer.ini)
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 7: Generate appinfo.ini and installer.ini from Templates ---

call :REPLACE "xxxVERSIONxxx" "%NP3_PORTAPP_INFO%_template.ini" "%VERSION%" "%NP3_PORTAPP_INFO%_tmp.ini"

call :REPLACE "xxxDEVNAMExxx" "%NP3_PORTAPP_INFO%_tmp.ini" "%DEVNAME%" "%NP3_PORTAPP_INFO%.ini"

:: installer_template.ini has no xxxDEVNAMExxx placeholder — this call strips
:: comment lines (;...) and blank lines via for /f defaults, producing a clean INI.
call :REPLACE "xxxDEVNAMExxx" "%NP3_PORTAPP_INSTALL%_template.ini" "" "%NP3_PORTAPP_INSTALL%.ini"

del /f /q "%NP3_PORTAPP_INFO%_tmp.ini"

echo     PackageVersion = %VERSION%
echo     DisplayVersion = %VERSION%%DEVNAME%

:: ---------------------------------------------------------------------------------------------------
:: Step 8: Select splash image for dev vs release builds
:: ---------------------------------------------------------------------------------------------------
if defined DEVNAME (
    if not "%DEVNAME%"=="" (
        echo.
        echo --- Step 8: Selecting Dev Splash Image ---
        if exist "%NP3_PORTAPP_DIR%\App\AppInfo\Launcher\Dev_Splash.jpg" (
            copy /Y "%NP3_PORTAPP_DIR%\App\AppInfo\Launcher\Dev_Splash.jpg" "%NP3_PORTAPP_DIR%\App\AppInfo\Launcher\Splash.jpg" >nul
            echo     Using Dev_Splash.jpg for development build.
        )
    )
)

:: ---------------------------------------------------------------------------------------------------
:: Step 9: Build Launcher and Installer Package
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 9: Build Launcher ---

:: - build Launcher -
"%PORTAPP_LAUNCHER_CREATOR%" "%NP3_PORTAPP_DIR%"
if errorlevel 1 (
    echo [ERROR] PortableApps.com Launcher Generator failed!
    exit /b 1
)

:: Signing "Notepad3Portable.exe" (".exe" file is created by "Launcher")
:: call %SCRIPT_DIR%Signing_for_NP3P_1st_EXE.cmd

echo.
echo --- Step 10: Build Installer Package ---

:: - build Installer -
"%PORTAPP_INSTALLER_CREATOR%" "%NP3_PORTAPP_DIR%"
if errorlevel 1 (
    echo [ERROR] PortableApps.com Installer failed!
    exit /b 1
)

:: Signing "Notepad3Portable_x.xx.xxx.x_zzzz.paf.exe" (".paf.exe" file is created by "Installer")
:: call %SCRIPT_DIR%Signing_for_NP3P_2nd_EXE.cmd

:: ---------------------------------------------------------------------------------------------------
:: Step 11: report result
:: ---------------------------------------------------------------------------------------------------
echo.
echo --- Step 11: Finalize ---

set Notepad3Portable.paf.exe=%SCRIPT_DIR%Notepad3Portable_%VERSION%%DEVNAME%.paf.exe
if exist %Notepad3Portable.paf.exe% (
    echo.
    echo === PortableApps Package Built Successfully! ===
    echo     Output: %Notepad3Portable.paf.exe%
) else (
    echo [ERROR] Expected output not found: Notepad3Portable_%VERSION%%DEVNAME%.paf.exe
    exit /b 1
)

:: ===================================================================================================
goto :END

:: ---------------------------------------------------------------------------------------------------
:: REPLACE  strg(%1)  srcfile(%2)  replstrg(%3)  dstfile(%4)
::
:: Performs string replacement AND strips comment lines (lines starting with ;)
:: and blank lines from the source file. This is an intentional side-effect of
:: for /f's default eol=; and empty-line-skipping behavior, used to produce
:: clean INI output from commented template files.
:: ---------------------------------------------------------------------------------------------------
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

:: Uses PowerShell to get the current date (avoids deprecated wmic)
:GETDATE
    for /f %%a in ('powershell -NoProfile -Command "Get-Date -Format yy"') do set "YY=%%a"
    for /f %%a in ('powershell -NoProfile -Command "Get-Date -Format MM"') do set "MM=%%a"
    for /f %%a in ('powershell -NoProfile -Command "Get-Date -Format dd"') do set "DD=%%a"
    goto :EOF
:: ---------------------------------------------------------------------------------------------------

:: Uses PowerShell to get file version (avoids deprecated wmic datafile)
:: Note: usebackq lets us use backticks for command delimiters, avoiding
:: conflict between for /f's single-quote delimiters and PowerShell quotes.
:GETFILEVER
    set "file=%~1"
    if not defined file goto :EOF
    if not exist "%file%" goto :EOF
    set "FILEVER="
    for /f "usebackq delims=" %%a in (`powershell -NoProfile -Command "(Get-Item '%file%').VersionInfo.FileVersion.Trim()"`) do set "FILEVER=%%a"
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
