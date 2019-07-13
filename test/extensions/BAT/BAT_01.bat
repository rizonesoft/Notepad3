@ECHO Off

EXIT

:strt0
IF EXIST %temp%\badst_tmp.txt DEL /F /Q %temp%\badst_tmp.txt >NUL
IF EXIST %temp%\disk_tmp.txt DEL /F /Q %temp%\disk_tmp.txt >NUL
IF EXIST %temp%\exst_tmp.txt DEL /F /Q %temp%\exst_tmp.txt >NUL
IF EXIST %temp%\intro_tmp.txt DEL /F /Q %temp%\intro_tmp.txt >NUL
IF EXIST %temp%\scssf_tmp.txt DEL /F /Q %temp%\scssf_tmp.txt >NUL
IF EXIST %temp%\wmic_tmp.txt DEL /F /Q %temp%\wmic_tmp.txt >NUL
WMIC logicaldisk get caption,description,name,volumename >%temp%\wmic_tmp.txt

:start
SET slct=
CLS
ECHO. >%temp%\intro_tmp.txt
ECHO.  CAUTION: Use this Batch File ONLY with a REMOVABLE Disk (eg: USB Memory Stick) >>%temp%\intro_tmp.txt
ECHO. >>%temp%\intro_tmp.txt
ECHO   List of All Disk : >>%temp%\intro_tmp.txt
ECHO. >>%temp%\intro_tmp.txt
TYPE %temp%\intro_tmp.txt
TYPE %temp%\wmic_tmp.txt
ECHO.
ECHO   Type  E  for Disk  (E:)
ECHO   Type  F  for Disk  (F:)
ECHO   Type  G  for Disk  (G:)
ECHO   Type  H  for Disk  (H:)
ECHO   Type  I  for Disk  (I:)
ECHO   Type  J  for Disk  (J:)
ECHO   Type  K  for Disk  (K:)
ECHO   Type  L  for Disk  (L:)
ECHO   Type  M  for Disk  (M:)
ECHO   Type  N  for Disk  (N:)
ECHO   Type  O  for Disk  (O:)
ECHO   Type  P  for Disk  (P:)
ECHO   Type  Q  for Disk  (Q:)
ECHO   Type  R  for Disk  (R:)
ECHO   Type  S  for Disk  (S:)
ECHO.
ECHO   Type  X  To Quit this Batch File
IF NOT '%badslct%'=='' ECHO.
IF NOT '%badslct%'=='' ECHO   The selected Letter  %badslct%  is NOT Valid ! ! !  Choose an other selection . . . .
SET badslct=
ECHO.
IF NOT '%badslct1%'=='' ECHO.
IF NOT '%badslct1%'=='' ECHO.  The Disk  " (%ldisk%:) "  NOT Exists ! ! !  Choose an other selection . . . .
IF NOT '%badslct1%'=='' ECHO.
IF NOT '%badslct1%'=='' ECHO.
SET badslct1=
SET /P slct=  Type Here Your Choice and Press Enter:  
CALL :set_up
IF '%slct%'=='E' GOTO :rdisk
IF '%slct%'=='F' GOTO :rdisk
IF '%slct%'=='G' GOTO :rdisk
IF '%slct%'=='H' GOTO :rdisk
IF '%slct%'=='I' GOTO :rdisk
IF '%slct%'=='J' GOTO :rdisk
IF '%slct%'=='K' GOTO :rdisk
IF '%slct%'=='L' GOTO :rdisk
IF '%slct%'=='M' GOTO :rdisk
IF '%slct%'=='N' GOTO :rdisk
IF '%slct%'=='O' GOTO :rdisk
IF '%slct%'=='P' GOTO :rdisk
IF '%slct%'=='Q' GOTO :rdisk
IF '%slct%'=='R' GOTO :rdisk
IF '%slct%'=='S' GOTO :rdisk
IF '%slct%'=='X' GOTO :end
SET badslct=" %slct% "
GOTO :start

:rdisk
CLS
SET ldisk=%slct%
IF NOT EXIST %ldisk%:\. SET badslct1=" %slct% "
IF NOT EXIST %ldisk%:\. GOTO :start
ECHO. >%temp%\disk_tmp.txt
ECHO. >>%temp%\disk_tmp.txt
IF EXIST %ldisk%:\. WMIC logicaldisk where deviceid="%ldisk%:" get drivetype | FINDSTR /C:"2" >NUL
IF %errorlevel%==0 (
    SET CHKRM=and is WELL
) else (
    SET CHKRM=but is NOT
)
IF EXIST %ldisk%:\. ECHO   The Disk  " (%ldisk%:) "  Exists, %CHKRM% a REMOVABLE Disk ! ! !  Choose a selection . . . . >>%temp%\disk_tmp.txt
ECHO. >>%temp%\disk_tmp.txt
ECHO. >>%temp%\disk_tmp.txt
TYPE %temp%\intro_tmp.txt
TYPE %temp%\wmic_tmp.txt
TYPE %temp%\disk_tmp.txt
IF EXIST %ldisk%:\. GOTO :disk1

:start1
CLS
IF NOT '%badslct%'=='' ECHO   The selected Letter  %badslct%  is NOT Valid ! ! !  Choose an other selection . . . . >%temp%\badst_tmp.txt
SET badslct=
ECHO. >>%temp%\badst_tmp.txt
TYPE %temp%\intro_tmp.txt
TYPE %temp%\wmic_tmp.txt
TYPE %temp%\disk_tmp.txt
TYPE %temp%\badst_tmp.txt

:disk1
SET slct=
ECHO   Type  C  To Continue this process
ECHO   Type  M  To Return to the Main Menu
ECHO   Type  X  To Quit this Batch File
SET badslct=
ECHO.
SET /P slct=  Type Here Your Choice and Press Enter:  
CALL :set_up
IF '%slct%'=='C' GOTO :disk2
IF '%slct%'=='M' GOTO :strt0
IF '%slct%'=='X' GOTO :end
SET badslct=" %slct% "
GOTO :start1

:disk2
ECHO.
ECHO.
IF EXIST %ldisk%:\autorun.inf\. GOTO :exst
IF EXIST %ldisk%:\autorun.inf ATTRIB -h -s -r %ldisk%:\autorun.inf
IF EXIST %ldisk%:\autorun.inf DEL /F /Q %ldisk%:\autorun.inf >NUL

:exst
IF NOT EXIST %ldisk%:\autorun.inf\. MKDIR %ldisk%:\autorun.inf
IF EXIST %ldisk%:\autorun.inf\. ECHO %ldisk%:\autorun.inf >%temp%\exst_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\con\. MKDIR \\.\\%ldisk%:\autorun.inf\con
IF EXIST %ldisk%:\autorun.inf\con\. ECHO %ldisk%:\autorun.inf\con >>%temp%\exst_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\con\aux\. MKDIR \\.\\%ldisk%:\autorun.inf\con\aux
IF EXIST %ldisk%:\autorun.inf\con\aux\. ECHO %ldisk%:\autorun.inf\con\aux >>%temp%\exst_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\con\aux\nul\. MKDIR \\.\\%ldisk%:\autorun.inf\con\aux\nul
IF EXIST %ldisk%:\autorun.inf\con\aux\nul\. ECHO %ldisk%:\autorun.inf\con\aux\nul >>%temp%\exst_tmp.txt
IF EXIST %ldisk%:\autorun.inf\. ATTRIB +h +s +r %ldisk%:\autorun.inf
IF EXIST %ldisk%:\autorun.inf\. TYPE %temp%\exst_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\. ECHO Access is denied. >%temp%\exst_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\. ECHO Access is denied. >>%temp%\exst_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\. ECHO Access is denied. >>%temp%\exst_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\. ECHO Access is denied. >>%temp%\exst_tmp.txt
IF EXIST %ldisk%:\autorun.inf\. ECHO. >%temp%\scssf_tmp.txt
IF EXIST %ldisk%:\autorun.inf\. ECHO. >>%temp%\scssf_tmp.txt
IF EXIST %ldisk%:\autorun.inf\. ECHO   Bacht Job Successful ! ! !  The Disk  " (%ldisk%:) "  is Protected to " AUTORUN.INF " . . . . >>%temp%\scssf_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\. ECHO. >%temp%\scssf_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\. ECHO. >>%temp%\scssf_tmp.txt
IF NOT EXIST %ldisk%:\autorun.inf\. ECHO   Bacht Job NOT Successful ! ! !  The Disk  " (%ldisk%:) "  is NOT Protected to " AUTORUN.INF " . . . . >>%temp%\scssf_tmp.txt
ECHO. >>%temp%\scssf_tmp.txt
ECHO. >>%temp%\scssf_tmp.txt
TYPE %temp%\scssf_tmp.txt
IF EXIST %ldisk%:\. GOTO :disk3

:start2
CLS
IF NOT '%badslct%'=='' ECHO   The selected Letter  %badslct%  is NOT Valid ! ! !  Choose an other selection . . . . >%temp%\badst_tmp.txt
ECHO. >>%temp%\badst_tmp.txt
SET badslct=
TYPE %temp%\intro_tmp.txt
TYPE %temp%\wmic_tmp.txt
TYPE %temp%\disk_tmp.txt
TYPE %temp%\exst_tmp.txt
TYPE %temp%\scssf_tmp.txt
TYPE %temp%\badst_tmp.txt

:disk3
SET slct=
ECHO   Type  M  To Return to the Main Menu
ECHO   Type  X  To Quit this Batch File
SET badslct=
ECHO.
SET /P slct=  Type Here Your Choice and Press Enter:  
CALL :set_up
IF '%slct%'=='M' GOTO :strt0
IF '%slct%'=='X' GOTO :end
SET badslct=" %slct% "
ECHO.
GOTO :start2

:end
IF EXIST %temp%\badst_tmp.txt DEL /F /Q %temp%\badst_tmp.txt >NUL
IF EXIST %temp%\disk_tmp.txt DEL /F /Q %temp%\disk_tmp.txt >NUL
IF EXIST %temp%\exst_tmp.txt DEL /F /Q %temp%\exst_tmp.txt >NUL
IF EXIST %temp%\intro_tmp.txt DEL /F /Q %temp%\intro_tmp.txt >NUL
IF EXIST %temp%\scssf_tmp.txt DEL /F /Q %temp%\scssf_tmp.txt >NUL
IF EXIST %temp%\wmic_tmp.txt DEL /F /Q %temp%\wmic_tmp.txt >NUL
GOTO :eof

:set_up
IF '%slct%'=='a' SET slct=A
IF '%slct%'=='b' SET slct=B
IF '%slct%'=='c' SET slct=C
IF '%slct%'=='d' SET slct=D
IF '%slct%'=='e' SET slct=E
IF '%slct%'=='f' SET slct=F
IF '%slct%'=='g' SET slct=G
IF '%slct%'=='h' SET slct=H
IF '%slct%'=='i' SET slct=I
IF '%slct%'=='j' SET slct=J
IF '%slct%'=='k' SET slct=K
IF '%slct%'=='l' SET slct=L
IF '%slct%'=='m' SET slct=M
IF '%slct%'=='n' SET slct=N
IF '%slct%'=='o' SET slct=O
IF '%slct%'=='p' SET slct=P
IF '%slct%'=='q' SET slct=Q
IF '%slct%'=='r' SET slct=R
IF '%slct%'=='s' SET slct=S
IF '%slct%'=='t' SET slct=T
IF '%slct%'=='u' SET slct=U
IF '%slct%'=='v' SET slct=V
IF '%slct%'=='w' SET slct=W
IF '%slct%'=='x' SET slct=X
IF '%slct%'=='y' SET slct=Y
IF '%slct%'=='z' SET slct=Z
:eof