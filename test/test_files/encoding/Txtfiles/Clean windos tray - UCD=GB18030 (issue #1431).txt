@ ECHO OFF
exit
SET PROG=清理系统通知区域 （XP/WIN7/VISTA）
COLOR 1F
@ ECHO.
@ ECHO.                              说                 明
@ ECHO.
@ ECHO --------------------------------------------------------------------------------
@ ECHO.
@ ECHO.    WINDOWS 系统能在通知区域自动隐藏不活动的图标，这是个非常实用的功能。但是日积
@ ECHO.月累之下，通知区域的图标越来越多，有的是很久之前的图标，甚至该软件已经卸载，图标
@ ECHO.的增多带来了臃肿，此批处理文件能帮你清理掉通知区域的历史图标。但此方法操作会清除
@ ECHO.你对图标进行的任何设置。
@ ECHO.
@ ECHO --------------------------------------------------------------------------------
TITLE %PROG%
PAUSE
CLS
@ ECHO.
@ ECHO.             ☆☆☆  第一步：清理通知区域在注册表内的相关子项  ☆☆☆
@ ECHO.
@ ECHO.
ECHO 　　　            ║　　        [1] WINDOWS XP　　　　        ║
ECHO 　　　            ║　　        [2] WINDOWS 7 / VISTA　       ║
@ ECHO.
@ ECHO.
SET /P CHOICE=　　　请选择你当前的操作系统版本 (1/2) ，然后按回车键执行：
IF /I '%CHOICE%'=='1' GOTO WINXP
IF /I '%CHOICE%'=='2' GOTO WIN7
:WINXP
REG DELETE "HKCU\SOFTWARE\MICROSOFT\WINDOWS\CURRENTVERSION\EXPLORER\TRAYNOTIFY" /V ICONSTREAMS /F
REG DELETE "HKCU\SOFTWARE\MICROSOFT\WINDOWS\CURRENTVERSION\EXPLORER\TRAYNOTIFY" /V PASTICONSSTREAM /F
PAUSE
GOTO SUCCESS
:WIN7
REG DELETE "HKEY_CLASSES_ROOT\LOCAL SETTINGS\SOFTWARE\MICROSOFT\WINDOWS\CURRENTVERSION\TRAYNOTIFY" /V ICONSTREAMS /F
REG DELETE "HKEY_CLASSES_ROOT\LOCAL SETTINGS\SOFTWARE\MICROSOFT\WINDOWS\CURRENTVERSION\TRAYNOTIFY" /V PASTICONSSTREAM /F
PAUSE
GOTO SUCCESS
:SUCCESS
CLS
@ ECHO.
@ ECHO.                     ☆☆☆  第二步：重启EXPLORER进程  ☆☆☆
@ ECHO.
@ ECHO.    警告:执行此命令，以完成清理通知区域的整个过程。为了安全起见，执行前请保存当
@ ECHO.前正在编辑的文件或文档，然后选择第 1 项继续。如果你不想执行此步，请选择第 2 项
@ ECHO.退出,清理任务在下次启动计算机后生效。
@ ECHO.
@ ECHO.
ECHO 　　　              ║　　     [1] 重启EXPLORER进程　　　　║
ECHO 　　　              ║　　     [2] 退出程序　              ║
@ ECHO.
@ ECHO.
SET /P CHOICE=　　　请选择要进行的操作 (1/2) ，然后按回车键执行：
IF /I '%CHOICE%'=='1' GOTO EXPLORER
IF /I '%CHOICE%'=='2' GOTO SUCCESS1
:EXPLORER
TASKKILL /IM EXPLORER.EXE /F
START EXPLORER.EXE
GOTO SUCCESS1
:SUCCESS1
EXIT 