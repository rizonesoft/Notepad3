@ECHO OFF
rem ****************************************************************************
rem *                                                                          *
rem * Notepad3                                                                 *
rem *                                                                          *
rem * Build.bat                                                                *
rem *   Batch file used to build Notepad3                                      *
rem *   Originally taken and adapted from Notepad2-mod:                        *
rem *   http://xhmikosr.github.io/notepad2-mod/                                *
rem *                                                                          *
rem *                                                                          *
rem *                                     (c) Rizonesoft 2008-2016             *
rem *                                         https://rizonesoft.com           *
rem *                                                                          *
rem *                                                                          *
rem ****************************************************************************

SETLOCAL ENABLEEXTENSIONS
CD /D %~dp0

rem Check the building environment
IF NOT DEFINED VS140COMNTOOLS CALL :SUBMSG "ERROR" "Visual Studio 2015 wasn't found!"

rem Check for the help switches
IF /I "%~1" == "help"   GOTO SHOWHELP
IF /I "%~1" == "/help"  GOTO SHOWHELP
IF /I "%~1" == "-help"  GOTO SHOWHELP
IF /I "%~1" == "--help" GOTO SHOWHELP
IF /I "%~1" == "/?"     GOTO SHOWHELP

rem Check for the first switch
IF "%~1" == "" (
  SET "BUILDTYPE=Build"
) ELSE (
  IF /I "%~1" == "Build"     SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Build"    SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Build"    SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Build"   SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "Clean"     SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Clean"    SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Clean"    SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Clean"   SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "Rebuild"   SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Rebuild"  SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Rebuild"  SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Rebuild" SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:CHECKSECONDARG
rem Check for the second switch
IF "%~2" == "" (
  SET "ARCH=all"
) ELSE (
  IF /I "%~2" == "x86"   SET "ARCH=x86" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "/x86"  SET "ARCH=x86" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "-x86"  SET "ARCH=x86" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "--x86" SET "ARCH=x86" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "x64"   SET "ARCH=x64" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "/x64"  SET "ARCH=x64" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "-x64"  SET "ARCH=x64" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "--x64" SET "ARCH=x64" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "all"   SET "ARCH=all" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "/all"  SET "ARCH=all" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "-all"  SET "ARCH=all" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "--all" SET "ARCH=all" & GOTO CHECKTHIRDARG

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:CHECKTHIRDARG
rem Check for the third switch
IF "%~3" == "" (
  SET "CONFIG=Release"
) ELSE (
  IF /I "%~3" == "Debug"     SET "CONFIG=Debug"   & GOTO START
  IF /I "%~3" == "/Debug"    SET "CONFIG=Debug"   & GOTO START
  IF /I "%~3" == "-Debug"    SET "CONFIG=Debug"   & GOTO START
  IF /I "%~3" == "--Debug"   SET "CONFIG=Debug"   & GOTO START
  IF /I "%~3" == "Release"   SET "CONFIG=Release" & GOTO START
  IF /I "%~3" == "/Release"  SET "CONFIG=Release" & GOTO START
  IF /I "%~3" == "-Release"  SET "CONFIG=Release" & GOTO START
  IF /I "%~3" == "--Release" SET "CONFIG=Release" & GOTO START
  IF /I "%~3" == "all"       SET "CONFIG=all"     & GOTO START
  IF /I "%~3" == "/all"      SET "CONFIG=all"     & GOTO START
  IF /I "%~3" == "-all"      SET "CONFIG=all"     & GOTO START
  IF /I "%~3" == "--all"     SET "CONFIG=all"     & GOTO START

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:START
IF /I "%ARCH%" == "x64" GOTO x64
IF /I "%ARCH%" == "x86" GOTO x86


:x86
CALL "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86

IF /I "%CONFIG%" == "all" (CALL :SUBMSVC %BUILDTYPE% Debug Win32 && CALL :SUBMSVC %BUILDTYPE% Release Win32) ELSE (CALL :SUBMSVC %BUILDTYPE% %CONFIG% Win32)

IF /I "%ARCH%" == "x86" GOTO END


:x64
CALL "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64

IF /I "%CONFIG%" == "all" (CALL :SUBMSVC %BUILDTYPE% Debug x64 && CALL :SUBMSVC %BUILDTYPE% Release x64) ELSE (CALL :SUBMSVC %BUILDTYPE% %CONFIG% x64)


:END
TITLE Building Notepad3 Finished!
ENDLOCAL
EXIT /B


:SUBMSVC
ECHO.
TITLE Building Notepad3 - %~1 "%~2|%~3"...
"MSBuild.exe" /nologo ..\Notepad3.sln /t:%~1 /p:Configuration=%~2;Platform=%~3^
 /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"
EXIT /B


:SHOWHELP
TITLE %~nx0 %1
ECHO. & ECHO.
ECHO Usage: %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|all] [Debug^|Release^|all]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        The arguments are not case sensitive.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments is equivalent to "%~nx0 build all release"
ECHO.
ECHO If you skip the second argument the default one will be used.
ECHO The same goes for the third argument. Examples:
ECHO "%~nx0 rebuild" is the same as "%~nx0 rebuild all release"
ECHO "%~nx0 rebuild x86" is the same as "%~nx0 rebuild x86 release"
ECHO.
ECHO WARNING: "%~nx0 x86" or "%~nx0 debug" won't work.
ECHO.
ENDLOCAL
EXIT /B


:SUBMSG
ECHO. & ECHO ______________________________
ECHO [%~1] %~2
ECHO ______________________________ & ECHO.
IF /I "%~1" == "ERROR" (
  PAUSE
  EXIT
) ELSE (
  EXIT /B
)
