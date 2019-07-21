@echo off
exit
echo ÊäÈëuÅÌÄ¿Â¼
echo ÊäÈëuÅÌÄ¿Â¼
echo ÊäÈëuÅÌÄ¿Â¼
echo ÊäÈëuÅÌÄ¿Â¼
echo ÊäÈëuÅÌÄ¿Â¼
set /p DriveU=
echo on
attrib "%DriveU%:\System Volume Information" -s
rd /s /q "%DriveU%:\System Volume Information"
del /f /q /A:RH "%DriveU%:\System Volume Information"
echo. >"%DriveU%:\System Volume Information"
attrib "%DriveU%:\System Volume Information" +R +H
pause
