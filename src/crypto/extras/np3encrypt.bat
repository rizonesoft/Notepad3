@echo off
setlocal enableextensions

set NP3ENCRYPTER=%~dp0np3encrypt.exe

echo. Change directory to "%~1"
pushd "%~1"

echo. Iterating all files of "%CD%"
for /r %%f in (*) do call :ENCRYPT "%%f" "%2"
popd

goto :END
:: ----------------------------------------------------------------------------

:ENCRYPT
set fn=%~nx1
set passphrase=%~2
echo. Encrypting file "%fn%" 
%NP3ENCRYPTER% ef "%fn%" "%fn%.enc" %passphrase%
goto :eof
:: ----------------------------------------------------------------------------

:END
pause
::popd
endlocal
