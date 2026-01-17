@echo off
:: ============================================================================
:: Notepad3 User Config Setup Script
:: ============================================================================
:: This script manages configuration files in the user's AppData folder.
:: It runs as the ORIGINAL user (not elevated) to ensure operations affect
:: the correct user profile, even when the installer runs with admin credentials.
::
:: Usage:
::   SetupUserConfig.cmd [command]
::
:: Commands:
::   install   - Copy default config files (only if they don't already exist)
::   reset     - Force overwrite config files with defaults
::   uninstall - Remove all config files and directories
::   (no args) - Same as "install" (backward compatibility)
:: ============================================================================

set "APPDATA_NP3=%APPDATA%\Rizonesoft\Notepad3"
set "SOURCE_DIR=%~dp0DefaultConfig"
set "COMMAND=%~1"

:: Default to "install" if no command specified
if "%COMMAND%"=="" set "COMMAND=install"

:: Route to appropriate handler
if /i "%COMMAND%"=="install" goto :DoInstall
if /i "%COMMAND%"=="reset" goto :DoReset
if /i "%COMMAND%"=="uninstall" goto :DoUninstall

:: Unknown command - treat as install for safety
goto :DoInstall


:: ============================================================================
:: INSTALL: Copy config files only if they don't already exist
:: ============================================================================
:DoInstall
:: Create directories if they don't exist
if not exist "%APPDATA_NP3%" mkdir "%APPDATA_NP3%"
if not exist "%APPDATA_NP3%\Themes" mkdir "%APPDATA_NP3%\Themes"
if not exist "%APPDATA_NP3%\Favorites" mkdir "%APPDATA_NP3%\Favorites"

:: Copy config files only if they don't already exist (preserve user customizations)
if not exist "%APPDATA_NP3%\Notepad3.ini" (
    if exist "%SOURCE_DIR%\Notepad3.ini" copy /Y "%SOURCE_DIR%\Notepad3.ini" "%APPDATA_NP3%\" >nul
)

if not exist "%APPDATA_NP3%\minipath.ini" (
    if exist "%SOURCE_DIR%\minipath.ini" copy /Y "%SOURCE_DIR%\minipath.ini" "%APPDATA_NP3%\" >nul
)

:: Copy theme files only if they don't already exist
if not exist "%APPDATA_NP3%\Themes\Dark.ini" (
    if exist "%SOURCE_DIR%\Themes\Dark.ini" copy /Y "%SOURCE_DIR%\Themes\Dark.ini" "%APPDATA_NP3%\Themes\" >nul
)

if not exist "%APPDATA_NP3%\Themes\Obsidian.ini" (
    if exist "%SOURCE_DIR%\Themes\Obsidian.ini" copy /Y "%SOURCE_DIR%\Themes\Obsidian.ini" "%APPDATA_NP3%\Themes\" >nul
)

if not exist "%APPDATA_NP3%\Themes\Sombra.ini" (
    if exist "%SOURCE_DIR%\Themes\Sombra.ini" copy /Y "%SOURCE_DIR%\Themes\Sombra.ini" "%APPDATA_NP3%\Themes\" >nul
)

:: Set the Favorites path in Notepad3.ini (only if not already set)
if exist "%APPDATA_NP3%\Notepad3.ini" (
    findstr /C:"Favorites=" "%APPDATA_NP3%\Notepad3.ini" >nul 2>&1
    if errorlevel 1 (
        echo.>>"%APPDATA_NP3%\Notepad3.ini"
        echo [Settings]>>"%APPDATA_NP3%\Notepad3.ini"
        echo Favorites=%%APPDATA%%\Rizonesoft\Notepad3\Favorites\>>"%APPDATA_NP3%\Notepad3.ini"
    )
)

exit /b 0


:: ============================================================================
:: RESET: Force overwrite all config files with defaults
:: ============================================================================
:DoReset
:: Create directories if they don't exist
if not exist "%APPDATA_NP3%" mkdir "%APPDATA_NP3%"
if not exist "%APPDATA_NP3%\Themes" mkdir "%APPDATA_NP3%\Themes"
if not exist "%APPDATA_NP3%\Favorites" mkdir "%APPDATA_NP3%\Favorites"

:: Delete existing config files
if exist "%APPDATA_NP3%\Notepad3.ini" del /F /Q "%APPDATA_NP3%\Notepad3.ini" >nul 2>&1
if exist "%APPDATA_NP3%\minipath.ini" del /F /Q "%APPDATA_NP3%\minipath.ini" >nul 2>&1
if exist "%APPDATA_NP3%\grepWinNP3.ini" del /F /Q "%APPDATA_NP3%\grepWinNP3.ini" >nul 2>&1
if exist "%APPDATA_NP3%\Themes\Dark.ini" del /F /Q "%APPDATA_NP3%\Themes\Dark.ini" >nul 2>&1
if exist "%APPDATA_NP3%\Themes\Obsidian.ini" del /F /Q "%APPDATA_NP3%\Themes\Obsidian.ini" >nul 2>&1
if exist "%APPDATA_NP3%\Themes\Sombra.ini" del /F /Q "%APPDATA_NP3%\Themes\Sombra.ini" >nul 2>&1

:: Copy fresh config files from defaults
if exist "%SOURCE_DIR%\Notepad3.ini" copy /Y "%SOURCE_DIR%\Notepad3.ini" "%APPDATA_NP3%\" >nul
if exist "%SOURCE_DIR%\minipath.ini" copy /Y "%SOURCE_DIR%\minipath.ini" "%APPDATA_NP3%\" >nul

:: Copy fresh theme files from defaults
if exist "%SOURCE_DIR%\Themes\Dark.ini" copy /Y "%SOURCE_DIR%\Themes\Dark.ini" "%APPDATA_NP3%\Themes\" >nul
if exist "%SOURCE_DIR%\Themes\Obsidian.ini" copy /Y "%SOURCE_DIR%\Themes\Obsidian.ini" "%APPDATA_NP3%\Themes\" >nul
if exist "%SOURCE_DIR%\Themes\Sombra.ini" copy /Y "%SOURCE_DIR%\Themes\Sombra.ini" "%APPDATA_NP3%\Themes\" >nul

:: Set the Favorites path in Notepad3.ini
if exist "%APPDATA_NP3%\Notepad3.ini" (
    :: Use a temporary file to add the Favorites setting properly
    findstr /V /C:"Favorites=" "%APPDATA_NP3%\Notepad3.ini" > "%APPDATA_NP3%\Notepad3.tmp" 2>nul
    move /Y "%APPDATA_NP3%\Notepad3.tmp" "%APPDATA_NP3%\Notepad3.ini" >nul 2>&1
    echo Favorites=%%APPDATA%%\Rizonesoft\Notepad3\Favorites\>>"%APPDATA_NP3%\Notepad3.ini"
)

exit /b 0


:: ============================================================================
:: UNINSTALL: Remove all config files and directories
:: ============================================================================
:DoUninstall
:: Delete all config files
if exist "%APPDATA_NP3%\Notepad3.ini" del /F /Q "%APPDATA_NP3%\Notepad3.ini" >nul 2>&1
if exist "%APPDATA_NP3%\minipath.ini" del /F /Q "%APPDATA_NP3%\minipath.ini" >nul 2>&1
if exist "%APPDATA_NP3%\grepWinNP3.ini" del /F /Q "%APPDATA_NP3%\grepWinNP3.ini" >nul 2>&1

:: Delete theme files
if exist "%APPDATA_NP3%\Themes\Dark.ini" del /F /Q "%APPDATA_NP3%\Themes\Dark.ini" >nul 2>&1
if exist "%APPDATA_NP3%\Themes\Obsidian.ini" del /F /Q "%APPDATA_NP3%\Themes\Obsidian.ini" >nul 2>&1
if exist "%APPDATA_NP3%\Themes\Sombra.ini" del /F /Q "%APPDATA_NP3%\Themes\Sombra.ini" >nul 2>&1

:: Remove Themes directory if empty
if exist "%APPDATA_NP3%\Themes" rmdir "%APPDATA_NP3%\Themes" 2>nul

:: Remove Favorites directory (even if not empty - user chose to delete all)
if exist "%APPDATA_NP3%\Favorites" rmdir /S /Q "%APPDATA_NP3%\Favorites" 2>nul

:: Remove main Notepad3 directory if empty
if exist "%APPDATA_NP3%" rmdir "%APPDATA_NP3%" 2>nul

:: Try to remove parent Rizonesoft directory if empty
if exist "%APPDATA%\Rizonesoft" rmdir "%APPDATA%\Rizonesoft" 2>nul

exit /b 0
