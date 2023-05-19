@echo off

:: Thanks to "Wilenty" for this "Additional Notepad3Portable.cmd Launcher".

If not exist "%~dp0\Data\settings\" (
	If exist "%~dp0\App\DefaultData\settings\" (
		mkdir "%~dp0\Data\settings"
		copy /y "%~dp0\App\DefaultData\settings" "%~dp0\Data\settings" >nul 2>&1
		For /f "delims=" %%W in (' dir /b /a:d "%~dp0\App\DefaultData\settings" ') do mkdir "%~dp0\Data\settings\%%~nW"&&copy /y "%~dp0\App\DefaultData\settings\%%~W" "%~dp0\Data\settings\%%~nW" >nul 2>&1
	)
)

set ErrorLevel=-1

If exist "%~dp0\Data\settings\" (
	set "NOTEPAD3_PORTABLE_SETTINGS=%~dp0\Data\settings"
	If exist "%WinDir%\SysNative" (
		start "Notepad3 Portable" /b "%~dp0\App\Notepad3\x64\Notepad3.exe" %*
    ) else (
		start "Notepad3 Portable" /b "%~dp0\App\Notepad3\x86\Notepad3.exe" %*
	)
)

exit /b %ErrorLevel%
