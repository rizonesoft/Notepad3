@ECHO OFF
rem ******************************************************************************
rem *
rem * Notepad2-mod
rem *
rem * coverity.bat
rem *   Batch file used to create the coverity scan analysis file
rem *   Originally taken and adapted from  https://github.com/mpc-hc/mpc-hc
rem *
rem * See License.txt for details about distribution and modification.
rem *
rem *                                     (c) XhmikosR 2013-2015
rem *                                     https://github.com/XhmikosR/notepad2-mod
rem *
rem ******************************************************************************


SETLOCAL

PUSHD %~dp0

IF NOT DEFINED COVDIR SET "COVDIR=H:\progs\thirdparty\cov-analysis-win64"
IF DEFINED COVDIR IF NOT EXIST "%COVDIR%" (
  ECHO.
  ECHO ERROR: Coverity not found in "%COVDIR%"
  GOTO End
)


CALL "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86
IF %ERRORLEVEL% NEQ 0 (
  ECHO vcvarsall.bat call failed.
  GOTO End
)


:Cleanup
IF EXIST "cov-int" RD /q /s "cov-int"
IF EXIST "Notepad2-mod.lzma" DEL "Notepad2-mod.lzma"
IF EXIST "Notepad2-mod.tar"  DEL "Notepad2-mod.tar"
IF EXIST "Notepad2-mod.tgz"  DEL "Notepad2-mod.tgz"


:Main
"%COVDIR%\bin\cov-build.exe" --dir cov-int "build_vs2015.bat" Rebuild All Release


:tar
tar --version 1>&2 2>NUL || (ECHO. & ECHO ERROR: tar not found & GOTO SevenZip)
tar caf "Notepad2-mod.lzma" "cov-int"
GOTO End


:SevenZip
CALL :SubDetectSevenzipPath

rem Coverity is totally bogus with lzma...
rem And since I cannot replicate the arguments with 7-Zip, just use tar/gzip.
IF EXIST "%SEVENZIP%" (
  "%SEVENZIP%" a -ttar "Notepad2-mod.tar" "cov-int"
  "%SEVENZIP%" a -tgzip "Notepad2-mod.tgz" "Notepad2-mod.tar"
  IF EXIST "Notepad2-mod.tar" DEL "Notepad2-mod.tar"
  GOTO End
)


:SubDetectSevenzipPath
FOR %%G IN (7z.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR %%G IN (7za.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & EXIT /B)

FOR /F "tokens=2*" %%A IN (
  'REG QUERY "HKLM\SOFTWARE\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ" ^|^|
   REG QUERY "HKLM\SOFTWARE\Wow6432Node\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ"') DO SET "SEVENZIP=%%B\7z.exe"
EXIT /B


:End
POPD
ECHO. & ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT /B
