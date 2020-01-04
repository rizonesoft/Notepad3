@echo off
::
exit
setlocal enableextensions
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

::more than 100 lines are suppressed

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
