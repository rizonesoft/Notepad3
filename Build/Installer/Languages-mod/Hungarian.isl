; *** Inno Setup version 6.5.0+ Hungarian messages ***
; Based on the translation of Kornél Pál, kornelpal@gmail.com
; István Szabó, E-mail: istvanszabo890629@gmail.com
;
; To download user-contributed translations of this file, go to:
;   https://jrsoftware.org/files/istrans/
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).

[LangOptions]
; The following three entries are very important. Be sure to read and 
; understand the '[LangOptions] section' topic in the help file.
LanguageName=Magyar
LanguageID=$040E
LanguageCodePage=1250
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
DialogFontName="Segoe UI"
DialogFontSize=9
WelcomeFontName="Segoe UI"
WelcomeFontSize=14
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial CE
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Telepítő
SetupWindowTitle=%1 - Telepítő
UninstallAppTitle=Eltávolító
UninstallAppFullTitle=%1 - Eltávolító

; *** Misc. common
InformationTitle=Információk
ConfirmTitle=Megerősít
ErrorTitle=Hiba

; *** SetupLdr messages
SetupLdrStartupMessage=%1 telepítve lesz. Szeretné folytatni?
LdrCannotCreateTemp=Átmeneti fájl létrehozása nem lehetséges. A telepítés megszakítva.
LdrCannotExecTemp=Fájl futtatása nem lehetséges az átmeneti könyvtárban. A telepítés megszakítva.
HelpTextNote=

; *** Startup error messages
LastErrorMessage=%1.%n%nHiba %2: %3
SetupFileMissing=A(z) %1 fájl hiányzik a telepítő könyvtárából. Kérem hárítsa el a problémát, vagy szerezzen be egy másik példányt a programból!
SetupFileCorrupt=A telepítési fájlok sérültek. Kérem, szerezzen be új másolatot a programból!
SetupFileCorruptOrWrongVer=A telepítési fájlok sérültek, vagy inkompatibilisek a telepítő ezen verziójával. Hárítsa el a problémát, vagy szerezzen be egy másik példányt a programból!
InvalidParameter=A parancssorba átadott paraméter érvénytelen:%n%n%1
SetupAlreadyRunning=A telepítő már fut.
WindowsVersionNotSupported=Ez a program nem támogatja a Windows ezen verzióját.
WindowsServicePackRequired=A program futtatásához %1 Service Pack %2 vagy újabb szükséges.
NotOnThisPlatform=Ez a program nem futtatható %1 alatt.
OnlyOnThisPlatform=Ezt a programot %1 alatt kell futtatni.
OnlyOnTheseArchitectures=A program kizárólag a következő processzorarchitektúrákhoz tervezett Windows-on telepíthető:%n%n%1
WinVersionTooLowError=A program futtatásához %1 %2 verziója vagy újabb szükséges.
WinVersionTooHighError=Ez a program nem telepíthető %1 %2 vagy újabb verzióra.
AdminPrivilegesRequired=Csak rendszergazdaként telepíthető ez a program.
PowerUserPrivilegesRequired=Csak rendszergazdaként vagy kiemelt felhasználóként telepíthető ez a program.
SetupAppRunningError=A telepítő úgy észlelte, hogy a(z) %1 jelenleg fut.%n%nZárja be az összes példányt, majd kattintson az 'OK'-ra a folytatáshoz, vagy a 'Mégse'-re a kilépéshez.
UninstallAppRunningError=Az eltávolító úgy észlelte, hogy a(z) %1 jelenleg fut.%n%nZárja be az összes példányt, majd kattintson az 'OK'-ra a folytatáshoz, vagy a 'Mégse'-re a kilépéshez.

; *** Startup questions
PrivilegesRequiredOverrideTitle=Telepítési mód kiválasztása
PrivilegesRequiredOverrideInstruction=Válasszon telepítési módot
PrivilegesRequiredOverrideText1=%1 telepíthető az összes felhasználónak (rendszergazdai jogok szükségesek), vagy a jelenlegi felhasználónak.
PrivilegesRequiredOverrideText2=%1 csak a jelenlegi felhasználónak telepíthető, vagy az összes felhasználónak (rendszergazdai jogok szükségesek).
PrivilegesRequiredOverrideAllUsers=Telepítés &mindenkinek
PrivilegesRequiredOverrideAllUsersRecommended=Telepítés &mindenkinek (ajánlott)
PrivilegesRequiredOverrideCurrentUser=Telepítés csak &nekem
PrivilegesRequiredOverrideCurrentUserRecommended=Telepítés csak &nekem (ajánlott)

; *** Misc. errors
ErrorCreatingDir=A telepítő nem tudta létrehozni a(z) "%1" könyvtárat
ErrorTooManyFilesInDir=Nem hozható létre újabb fájl a(z) "%1" könyvtárban, mert az már túl sok fájlt tartalmaz

; *** Setup common messages
ExitSetupTitle=Kilépés a telepítőből
ExitSetupMessage=A telepítés még folyamatban van. Ha most kilép, a program telepítése megszakad.%n%nA telepítő újbóli futtatásával később is a befejezheti telepítést.%n%nKilép a telepítőből?
AboutSetupMenuItem=&Névjegy...
AboutSetupTitle=Telepítő névjegye
AboutSetupMessage=%1 %2 verzió%n%3%n%nA(z) %1 honlapja:%n%4
AboutSetupNote=
TranslatorNote=

; *** Buttons
ButtonBack=< &Vissza
ButtonNext=&Tovább >
ButtonInstall=T&elepítés
ButtonOK=Rendben
ButtonCancel=Mégse
ButtonYes=&Igen
ButtonYesToAll=&Mindet
ButtonNo=&Nem
ButtonNoToAll=&Egyiket se
ButtonFinish=&Befejezés
ButtonBrowse=T&allózás...
ButtonWizardBrowse=Talló&zás...
ButtonNewFolder=Új &könyvtár

; *** "Select Language" dialog messages
SelectLanguageTitle=Telepítő nyelvi beállítása
SelectLanguageLabel=Válassza ki a telepítés alatt használt nyelvet!

; *** Common wizard text
ClickNext=A folytatáshoz kattintson a 'Tovább'-ra, a kilépéshez a 'Mégse'-re.
BeveledLabel=
BrowseDialogTitle=Könyvtár kiválasztása
BrowseDialogLabel=Válasszon egy könyvtárat az alábbi listából, majd kattintson az 'OK'-ra.
NewFolderName=Új könyvtár

; *** "Welcome" wizard page
WelcomeLabel1=Üdvözli a(z) [name] telepítővarázslója.
WelcomeLabel2=A(z) [name/ver] telepítésre kerül a számítógépén.%n%nAjánlott minden, egyéb futó alkalmazás bezárása a folytatás előtt.

; *** "Password" wizard page
WizardPassword=Jelszó
PasswordLabel1=Ez a telepítés jelszóval védett.
PasswordLabel3=Kérem adja meg a jelszót, majd kattintson a 'Tovább'-ra. A jelszavak kis- és nagy betű érzékenyek.
PasswordEditLabel=&Jelszó:
IncorrectPassword=A megadott jelszó helytelen. Próbálja újra!

; *** "License Agreement" wizard page
WizardLicense=Licencszerződés
LicenseLabel=Olvassa el figyelmesen az információkat folytatás előtt.
LicenseLabel3=Kérem, olvassa el az alábbi licencszerződést. A telepítés folytatásához mindenképp el kell fogadnia a szerződés feltételeit.
LicenseAccepted=&Elfogadom a szerződést
LicenseNotAccepted=&Nem fogadom el a szerződést

; *** "Information" wizard pages
WizardInfoBefore=Információk
InfoBeforeLabel=Olvassa el a következő fontos információkat a folytatás előtt.
InfoBeforeClickLabel=Ha készen áll, kattintson a 'Tovább'-ra.
WizardInfoAfter=Információk
InfoAfterLabel=Olvassa el a következő fontos információkat a folytatás előtt.
InfoAfterClickLabel=Ha készen áll, kattintson a 'Tovább'-ra.

; *** "User Information" wizard page
WizardUserInfo=Felhasználó adatai
UserInfoDesc=Kérem, adja meg az adatait!
UserInfoName=&Felhasználónév:
UserInfoOrg=S&zervezet:
UserInfoSerial=&Sorozatszám:
UserInfoNameRequired=Meg kell adnia egy nevet!

; *** "Select Destination Location" wizard page
WizardSelectDir=Válasszon célkönyvtárat
SelectDirDesc=Hova települjön a(z) [name]?
SelectDirLabel3=A(z) [name] az alábbi könyvtárba lesz telepítve.
SelectDirBrowseLabel=A folytatáshoz, kattintson a 'Tovább'-ra. Ha másik könyvtárat választana, kattintson a 'Tallózás'-ra.
DiskSpaceGBLabel=Legalább [gb] GB szabad területre van szükség.
DiskSpaceMBLabel=Legalább [mb] MB szabad területre van szükség.
CannotInstallToNetworkDrive=A telepítő nem tud hálózati meghajtóra telepíteni.
CannotInstallToUNCPath=A telepítő nem tud hálózati UNC elérési útra telepíteni.
InvalidPath=Teljes útvonalat adjon meg, a meghajtó betűjelével; például:%n%nC:\Alkalmazás%n%nvagy egy hálózati útvonalat a következő alakban:%n%n\\kiszolgáló\megosztás
InvalidDrive=A kiválasztott meghajtó vagy hálózati megosztás nem létezik, vagy nem elérhető. Válasszon egy másikat!
DiskSpaceWarningTitle=Nincs elég szabad terület
DiskSpaceWarning=A telepítőnek legalább %1 KB szabad lemezterületre van szüksége, viszont a kiválasztott meghajtón csupán %2 KB áll rendelkezésre.%n%nMindenképpen folytatja?
DirNameTooLong=A könyvtár neve vagy az elérési útvonal túl hosszú.
InvalidDirName=A könyvtár neve érvénytelen.
BadDirName32=A könyvtárak nevei ezen karakterek egyikét sem tartalmazhatják:%n%n%1
DirExistsTitle=A könyvtár már létezik
DirExists=A könyvtár:%n%n%1%n%nmár létezik. Mindenképp ide akar telepíteni?
DirDoesntExistTitle=A könyvtár nem létezik
DirDoesntExist=A könyvtár:%n%n%1%n%nnem létezik. Szeretné létrehozni?

; *** "Select Components" wizard page
WizardSelectComponents=Összetevők kiválasztása
SelectComponentsDesc=Mely összetevők kerüljenek telepítésre?
SelectComponentsLabel2=Jelölje ki a telepítendő összetevőket; törölje a telepíteni nem kívánt összetevőket. Kattintson a 'Tovább'-ra, ha készen áll a folytatásra.
FullInstallation=Teljes telepítés
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Szokásos telepítés
CustomInstallation=Egyéni telepítés
NoUninstallWarningTitle=Létező összetevő
NoUninstallWarning=A telepítő úgy találta, hogy a következő összetevők már telepítve vannak a számítógépre:%n%n%1%n%nEzen összetevők kijelölésének törlése, nem távolítja el azokat a számítógépről.%n%nMindenképpen folytatja?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceGBLabel=A jelenlegi kijelölés legalább [gb] GB lemezterületet igényel.
ComponentsDiskSpaceMBLabel=A jelenlegi kijelölés legalább [mb] MB lemezterületet igényel.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=További feladatok
SelectTasksDesc=Mely kiegészítő feladatok kerüljenek végrehajtásra?
SelectTasksLabel2=Jelölje ki, mely kiegészítő feladatokat hajtsa végre a telepítő a(z) [name] telepítése során, majd kattintson a 'Tovább'-ra.

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Start Menü könyvtára
SelectStartMenuFolderDesc=Hova helyezze a telepítő a program parancsikonjait?
SelectStartMenuFolderLabel3=A telepítő a program parancsikonjait a Start menü következő mappájában fogja létrehozni.
SelectStartMenuFolderBrowseLabel=A folytatáshoz kattintson a 'Tovább'-ra. Ha másik mappát választana, kattintson a 'Tallózás'-ra.
MustEnterGroupName=Meg kell adnia egy mappanevet.
GroupNameTooLong=A könyvtár neve vagy az elérési útvonal túl hosszú.
InvalidGroupName=A könyvtár neve érvénytelen.
BadGroupName=A könyvtárak nevei ezen karakterek egyikét sem tartalmazhatják:%n%n%1
NoProgramGroupCheck2=&Ne hozzon létre mappát a Start menüben

; *** "Ready to Install" wizard page
WizardReady=Készen állunk a telepítésre
ReadyLabel1=A telepítő készen áll a(z) [name] számítógépre telepítéshez.
ReadyLabel2a=Kattintson a 'Telepítés'-re a folytatáshoz, vagy a 'Vissza'-ra a beállítások áttekintéséhez vagy megváltoztatásához.
ReadyLabel2b=Kattintson a 'Telepítés'-re a folytatáshoz.
ReadyMemoUserInfo=Felhasználó adatai:
ReadyMemoDir=Telepítés célkönyvtára:
ReadyMemoType=Telepítés típusa:
ReadyMemoComponents=Választott összetevők:
ReadyMemoGroup=Start menü mappája:
ReadyMemoTasks=Kiegészítő feladatok:

; *** TDownloadWizardPage wizard page and DownloadTemporaryFile
DownloadingLabel2=Fájlok letöltése...
ButtonStopDownload=&Letöltés megállítása
StopDownload=Biztos, hogy meg akarja szakítani a letöltést?
ErrorDownloadAborted=Letöltés megszakítva
ErrorDownloadFailed=A letöltés meghiúsult: %1 %2
ErrorDownloadSizeFailed=Hiba a fájlméret lekérése során: %1 %2
ErrorProgress=Érvénytelen folyamat: %1 : %2
ErrorFileSize=Érvénytelen fájlméret, várt méret: %1, számított: %2

; *** TExtractionWizardPage wizard page and Extract7ZipArchive
ExtractingLabel=Fájlok kicsomagolása...
ButtonStopExtraction=&Kicsomagolás megszakítása
StopExtraction=Biztosan meg akarja szakítani a kicsomagolást?
ErrorExtractionAborted=A kicsomagolás megszakítva
ErrorExtractionFailed=A kicsomagolás meghiúsult: %1

; *** Archive extraction failure details
ArchiveIncorrectPassword=Nem megfelelő jelszó
ArchiveIsCorrupted=Az archívum sérült
ArchiveUnsupportedFormat=Az archívum formátuma nem támogatott

; *** "Preparing to Install" wizard page
WizardPreparing=Felkészülés a telepítésre
PreparingDesc=A telepítő felkészül a(z) [name] számítógépre történő telepítéshez.
PreviousInstallNotCompleted=Egy korábbi program telepítése/eltávolítása nem fejeződött be. Újra kell indítania a számítógépét a másik telepítés befejezéséhez.%n%nA számítógépe újraindítása után ismét futtassa a telepítőt a(z) [name] telepítésének befejezéséhez.
CannotContinue=A telepítés nem folytatható. A kilépéshez kattintson a 'Mégse'-re.
ApplicationsFound=A következő alkalmazások olyan fájlokat használnak, amelyeket a telepítőnek frissíteni kell. Ajánlott, hogy engedélyezze a telepítőnek ezen alkalmazások automatikus bezárását.
ApplicationsFound2=A következő alkalmazások olyan fájlokat használnak, amelyeket a telepítőnek frissíteni kell. Ajánlott, hogy engedélyezze a telepítőnek ezen alkalmazások automatikus bezárását. A telepítés befejezése után, a telepítő megkísérli az alkalmazások újraindítását.
CloseApplications=&Alkalmazások automatikus bezárása
DontCloseApplications=&Ne zárja be az alkalmazásokat
ErrorCloseApplications=A telepítő nem tudott minden alkalmazást automatikusan bezárni. A folytatás előtt ajánlott minden, a telepítő által frissítendő fájlokat használó alkalmazást bezárni.
PrepareToInstallNeedsRestart=A telepítőnek most újra kell indítania a számítógépet. Az újraindítás után futtassa újból ezt a telepítőt, hogy befejezze a [name] telepítését.%n%nÚjra szeretné most indítani a gépet?

; *** "Installing" wizard page
WizardInstalling=Telepítés folyamatban
InstallingLabel=Kérem várjon, amíg a(z) [name] telepítése zajlik.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=A(z) [name] telepítésének befejezése
FinishedLabelNoIcons=A telepítő végzett a(z) [name] telepítésével.
FinishedLabel=A telepítő végzett a(z) [name] telepítésével. Az alkalmazást a létrehozott ikonok kiválasztásával indíthatja.
ClickFinish=Kattintson a 'Befejezés'-re a kilépéshez.
FinishedRestartLabel=A(z) [name] telepítésének befejezéséhez újra kell indítani a számítógépet. Újraindítja most?
FinishedRestartMessage=A(z) [name] telepítésének befejezéséhez, a telepítőnek újra kell indítani a számítógépet.%n%nÚjraindítja most?
ShowReadmeCheck=Igen, szeretném elolvasni a 'FONTOS' fájlt
YesRadio=&Igen, újraindítás most
NoRadio=&Nem, később indítom újra
; used for example as 'Run MyProg.exe'
RunEntryExec=%1 futtatása
; used for example as 'View Readme.txt'
RunEntryShellExec=%1 megtekintése

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=A telepítőnek szüksége van a következő lemezre
SelectDiskLabel2=Helyezze be a(z) %1. lemezt és kattintson az 'OK'-ra.%n%nHa a fájlok a lemez egy a megjelenítettől különböző mappájában találhatók, írja be a helyes útvonalat vagy kattintson a 'Tallózás'-ra.
PathLabel=Ú&tvonal:
FileNotInDir2=A(z) "%1" fájl nem található a következő helyen: "%2". Helyezze be a megfelelő lemezt vagy válasszon egy másik mappát.
SelectDirectoryLabel=Adja meg a következő lemez helyét.

; *** Installation phase messages
SetupAborted=A telepítés nem fejeződött be.%n%nHárítsa el a hibát és futtassa újból a telepítőt.
AbortRetryIgnoreSelectAction=Válasszon műveletet
AbortRetryIgnoreRetry=Ú&jra
AbortRetryIgnoreIgnore=&Hiba elvetése és folytatás
AbortRetryIgnoreCancel=Telepítés megszakítása
RetryCancelSelectAction=Művelet kiválasztása
RetryCancelRetry=Ú&jra
RetryCancelCancel=Mégse

; *** Installation status messages
StatusClosingApplications=Alkalmazások bezárása...
StatusCreateDirs=Könyvtárak létrehozása...
StatusExtractFiles=Fájlok kibontása...
StatusDownloadFiles=Fájlok letöltése...
StatusCreateIcons=Parancsikonok létrehozása...
StatusCreateIniEntries=INI-bejegyzések létrehozása...
StatusCreateRegistryEntries=Rendszerleíró-bejegyzések létrehozása...
StatusRegisterFiles=Fájlok regisztrálása...
StatusSavingUninstall=Eltávolító információk mentése...
StatusRunProgram=Telepítés befejezése...
StatusRestartingApplications=Alkalmazások újraindítása...
StatusRollback=Változtatások visszavonása...

; *** Misc. errors
ErrorInternal2=Belső hiba: %1
ErrorFunctionFailedNoCode=Sikertelen %1
ErrorFunctionFailed=Sikertelen %1; kód: %2
ErrorFunctionFailedWithMessage=Sikertelen %1; kód: %2.%n%3
ErrorExecutingProgram=Nem hajtható végre a fájl:%n%1

; *** Registry errors
ErrorRegOpenKey=Nem nyitható meg a rendszerleíró kulcs:%n%1\%2
ErrorRegCreateKey=Nem hozható létre a rendszerleíró kulcs:%n%1\%2
ErrorRegWriteKey=Nem módosítható a rendszerleíró kulcs:%n%1\%2

; *** INI errors
ErrorIniEntry=Hiba lépett fel az INI-bejegyzés létrehozása során, ebben a fájlban: "%1".

; *** File copying errors
FileAbortRetryIgnoreSkipNotRecommended=&Fájl kihagyása (nem ajánlott)
FileAbortRetryIgnoreIgnoreNotRecommended=&Hiba elvetése és folytatás (nem ajánlott)
SourceIsCorrupted=A forrásfájl megsérült
SourceDoesntExist=A(z) "%1" forrásfájl nem létezik
SourceVerificationFailed=A(z) "%1" forrásfájl ellenőrzése meghiúsult
VerificationSignatureDoesntExist=A(z) "%1" digitálisaláírás-fájl nem létezik
VerificationSignatureInvalid=A(z) "%1" digitálisaláírás-fájl érvénytelen
VerificationKeyNotFound=A(z) "%1" digitálisaláírás-fájl egy ismeretlen kulcsot használ
VerificationFileNameIncorrect=A fájl neve nem megfelelő
VerificationFileTagIncorrect=A fájl címkéje nem megfelelő
VerificationFileSizeIncorrect=A fájl mérete nem megfelelő
VerificationFileHashIncorrect=A fájl hasítóértéke nem megfelelő
ExistingFileReadOnly2=A fájl csak olvashatóként van jelölve, ezért nem cserélhető le.
ExistingFileReadOnlyRetry=Csak &olvasható tulajdonság eltávolítása és újra próbálkozás
ExistingFileReadOnlyKeepExisting=&Létező fájl megtartása
ErrorReadingExistingDest=Hiba lépett fel a fájl olvasása közben:
FileExistsSelectAction=Mit tegyünk?
FileExists2=A fájl már létezik.
FileExistsOverwriteExisting=A &létező fájl felülírása
FileExistsKeepExisting=A &már létező fájl megtartása
FileExistsOverwriteOrKeepAll=&Tegyük ezt, a következő fájlütközések esetén is
ExistingFileNewerSelectAction=Mit kíván tenni?
ExistingFileNewer2=A létező fájl újabb a telepítésre kerülőnél
ExistingFileNewerOverwriteExisting=A &létező fájl felülírása
ExistingFileNewerKeepExisting=&Tartsuk meg a létező fájlt (ajánlott)
ExistingFileNewerOverwriteOrKeepAll=&Tegyük ezt, a következő fájlütközések esetén is
ErrorChangingAttr=Hiba lépett fel a következő fájl attribútumának módosítása közben:
ErrorCreatingTemp=Hiba lépett fel a következő fájl telepítési könyvtárban történő létrehozása közben:
ErrorReadingSource=Hiba lépett fel a következő forrásfájl olvasása közben:
ErrorCopying=Hiba lépett fel a következő fájl másolása közben:
ErrorDownloading=Hiba lépett fel a következő archívum fájl letöltése közben:
ErrorExtracting=Hiba lépett fel a következő archívum kicsomagolása közben:
ErrorReplacingExistingFile=Hiba lépett fel a következő, már létező fájl cseréje közben:
ErrorRestartReplace=A következő fájl cseréje sikertelen volt az újraindítás után:
ErrorRenamingTemp=Hiba lépett fel a következő fájl telepítési könyvtárban történő átnevezése közben:
ErrorRegisterServer=Nem lehet regisztrálni a DLL-t/OCX-et: %1
ErrorRegSvr32Failed=Sikertelen RegSvr32. A visszaadott kód: %1
ErrorRegisterTypeLib=Nem lehet regisztrálni a típustárat: %1

; *** Uninstall display name markings
; used for example as 'My Program (32-bit)'
UninstallDisplayNameMark=%1 (%2)
; used for example as 'My Program (32-bit, All users)'
UninstallDisplayNameMarks=%1 (%2, %3)
UninstallDisplayNameMark32Bit=32-bit
UninstallDisplayNameMark64Bit=64-bit
UninstallDisplayNameMarkAllUsers=Minden felhasználó
UninstallDisplayNameMarkCurrentUser=Jelenlegi felhasználó

; *** Post-installation errors
ErrorOpeningReadme=Hiba lépett fel a 'FONTOS' fájl megnyitása közben.
ErrorRestartingComputer=A telepítő nem tudta újraindítani a számítógépet. Indítsa újra kézileg.

; *** Uninstaller messages
UninstallNotFound=A(z) "%1" fájl nem létezik. Nem távolítható el.
UninstallOpenError=A(z) "%1" fájl nem nyitható meg. Nem távolítható el.
UninstallUnsupportedVer=A(z) "%1" eltávolítási naplófájl formátumát nem tudja felismerni az eltávolító jelen verziója. Az eltávolítás nem folytatható.
UninstallUnknownEntry=Egy ismeretlen bejegyzés (%1) található az eltávolítási naplófájlban.
ConfirmUninstall=Biztosan el kívánja távolítani a(z) %1 programot és minden összetevőjét?
UninstallOnlyOnWin64=Ezt a telepítést csak 64-bites Windows operációs rendszerről lehet eltávolítani.
OnlyAdminCanUninstall=Ezt a telepítést csak adminisztrációs jogokkal rendelkező felhasználó távolíthatja el.
UninstallStatusLabel=Legyen türelemmel, amíg a(z) %1 számítógépéről történő eltávolítása befejeződik.
UninstalledAll=A(z) %1 sikeresen el lett távolítva a számítógépről.
UninstalledMost=A(z) %1 eltávolítása befejeződött.%n%nNéhány elemet nem lehetettet eltávolítani. Törölje kézileg.
UninstalledAndNeedsRestart=A(z) %1 eltávolításának befejezéséhez újra kell indítania a számítógépét.%n%nÚjraindítja most?
UninstallDataCorrupted=A(z) "%1" fájl sérült. Nem távolítható el.

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Törli a megosztott fájlt?
ConfirmDeleteSharedFile2=A rendszer azt jelzi, hogy a következő megosztott fájlra már nincs szüksége egyetlen programnak sem. Eltávolítja a megosztott fájlt?%n%nHa más programok még mindig használják a megosztott fájlt, akkor az eltávolítása után lehetséges, hogy az érintett programok helytelenül fognak működni. Ha bizonytalan, válassza a 'Nem'-et. A fájl megtartása nem okoz problémát a rendszerben.
SharedFileNameLabel=Fájlnév:
SharedFileLocationLabel=Helye:
WizardUninstalling=Eltávolítás állapota
StatusUninstalling=%1 eltávolítása...

; *** Shutdown block reasons
ShutdownBlockReasonInstallingApp=%1 telepítése.
ShutdownBlockReasonUninstallingApp=%1 eltávolítása.

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1, verzió: %2
AdditionalIcons=További parancsikonok:
CreateDesktopIcon=&Asztali parancsikon létrehozása
CreateQuickLaunchIcon=&Gyorsindító-parancsikon létrehozása
ProgramOnTheWeb=%1 az interneten
UninstallProgram=Eltávolítás - %1
LaunchProgram=Indítás %1
AssocFileExtension=A(z) %1 &társítása a(z) %2 fájlkiterjesztéssel
AssocingFileExtension=A(z) %1 társítása a(z) %2 fájlkiterjesztéssel...
AutoStartProgramGroupDescription=Indítópult:
AutoStartProgram=%1 automatikus indítása
AddonHostProgramNotFound=A(z) %1 nem található a kiválasztott könyvtárban.%n%nMindenképpen folytatja?
