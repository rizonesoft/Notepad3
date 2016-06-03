@ECHO OFF
rem ****************************************************************************
rem *                                                                          *
rem * Notepad3                                                                 *
rem *                                                                          *
rem * make_all.bat                                                             *
rem *   Batch file for building Notepad3                                       *
rem *   and creating the installer/zip packages.                               *
rem *   Originally taken and adapted from Notepad2-mod:                        *
rem *   http://xhmikosr.github.io/notepad2-mod/                                *
rem *                                                                          *
rem *                                                                          *
rem *                                     (c) Rizonesoft 2008-2016             *
rem *                                         https://rizonesoft.com           *
rem *                                                                          *
rem *                                                                          *
rem ****************************************************************************

SETLOCAL
CD /D %~dp0

CALL "Build.bat"
CALL "upx.bat"
CALL "sign.bat"
CALL "make_zip.bat"
CALL "make_installer.bat"

:END
TITLE Finished!
ECHO.
PAUSE
ENDLOCAL
EXIT /B