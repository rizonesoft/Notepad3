@echo off
setlocal EnableDelayedExpansion

set/a"b=50, c=111"

for /L %%N in (1,1,1000) do (
    echo. !b:~0,-1!.!b:~-1! !c:~0,-1!.!c:~-1!
    timeout /t 1 2>&1 >nul 
    set/a"b+=1,c+=1"
)