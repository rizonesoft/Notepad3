; *** Inno Setup version 6.4.0+ Belarusian messages ***

; Translated from English by Alyaksandr Koshal, alk85@pm.me
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).

[LangOptions]
LanguageName=Беларуская
LanguageID=$0423
LanguageCodePage=1251
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
DialogFontName="Segoe UI"
DialogFontSize=9
WelcomeFontName="Segoe UI"
WelcomeFontSize=14
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Усталяванне
SetupWindowTitle=Усталяванне: %1
UninstallAppTitle=Выдаленне
UninstallAppFullTitle=Выдаленне %1

; *** Misc. common
InformationTitle=Інфармацыя
ConfirmTitle=Пацвярджэнне
ErrorTitle=Памылка

; *** SetupLdr messages
SetupLdrStartupMessage=Гэта праграма ўсталюе %1 на ваш камп'ютар. Працягнуць?
LdrCannotCreateTemp=Немагчыма стварыць часовы файл. Усталяванне перарвана
LdrCannotExecTemp=Немагчыма выканаць файл у часовым каталогу. Усталяванне перарвана
HelpTextNote=

; *** Startup error messages
LastErrorMessage=%1.%n%nПамылка %2: %3
SetupFileMissing=Файл %1 адсутнічае ў каталогу ўсталявання. Вырашыце праблему або атрымайце новую версію праграмы.
SetupFileCorrupt=Файлы праграмы ўсталявання пашкоджаны. Атрымайце новую копію праграмы.
SetupFileCorruptOrWrongVer=Файлы праграмы ўсталявання пашкоджаны або несумяшчальны з гэтай версіяй праграмы. Вырашыце праблему або атрымайце новую копію праграмы.
InvalidParameter=Камандны радок змяшчае памылковы параметр:%n%n%1
SetupAlreadyRunning=Праграма ўсталявання ўжо выконваецца.
WindowsVersionNotSupported=Гэта праграма не падтрымлівае версію Windows, якая ўсталявана на вашым камп'ютары.
WindowsServicePackRequired=Гэта праграма патрабуе %1 Service Pack %2 або больш новую версію.
NotOnThisPlatform=Гэта праграма не будзе працаваць у %1.
OnlyOnThisPlatform=Гэту праграму можна запускаць толькі ў %1.
OnlyOnTheseArchitectures=Усталёўваць праграму можна толькі ў версіях Windows для працэсараў з наступнай архітэктурай:%n%n%1
WinVersionTooLowError=Гэта праграма патрабуе %1 версіі %2 або больш новую версію.
WinVersionTooHighError=Праграму нельга ўсталяваць у %1 версіі %2 або больш новай.
AdminPrivilegesRequired=Каб усталяваць гэту праграму, увайдзіце ў сістэму як адміністратар.
PowerUserPrivilegesRequired=Каб усталяваць гэту праграму, увайдзіце ў сістэму як адміністратар або як удзельнік групы "Дасведчаныя карыстальнікі" (Power Users).
SetupAppRunningError=Праграма ўсталявання выявіла запушчаны экзэмпляр %1.%n%nЗакрыйце ўсе экзэмпляры праграмы і націсніце "Добра", каб працягнуць, або "Скасаваць" для выхаду.
UninstallAppRunningError=Праграма выдалення выявіла запушчаны экзэмпляр %1.%n%nЗакрыйце ўсе экзэмпляры праграмы і націсніце "Добра", каб працягнуць або "Скасаваць", каб выйсці.

; *** Startup questions
PrivilegesRequiredOverrideTitle=Выбар рэжыму ўсталявання
PrivilegesRequiredOverrideInstruction=Выберыце рэжым усталявання
PrivilegesRequiredOverrideText1=%1 можа быць усталяваны для ўсіх карыстальнікаў (патрабуюцца правы адміністратара) або толькі для вас.
PrivilegesRequiredOverrideText2=%1 можа быць усталяваны толькі для вас або для ўсіх карыстальнікаў (патрабуюцца правы адміністратара).
PrivilegesRequiredOverrideAllUsers=Усталяваць для &ўсіх карыстальнікаў
PrivilegesRequiredOverrideAllUsersRecommended=Усталяваць для &ўсіх карыстальнікаў (рэкамендуецца)
PrivilegesRequiredOverrideCurrentUser=Усталяваць толькі для &мяне
PrivilegesRequiredOverrideCurrentUserRecommended=Усталяваць толькі для &мяне (рэкамендуецца)

; *** Misc. errors
ErrorCreatingDir=Праграма ўсталявання не змагла стварыць каталог "%1"
ErrorTooManyFilesInDir=Немагчыма стварыць файл у каталогу "%1", бо ў ім занадта шмат файлаў

; *** Setup common messages
ExitSetupTitle=Выйсці з праграмы ўсталявання
ExitSetupMessage=Праграма ўсталявання не завяршыла сваю працу. Калі выйсці, праграма не будзе ўсталявана.%n%nЗавяршыць працэс можна будзе, калі вы запусціце файл усталявання яшчэ раз.%n%nВыйсці зараз?
AboutSetupMenuItem=&Пра праграму...
AboutSetupTitle=Пра праграму
AboutSetupMessage=%1 версія %2%n%3%n%n%1 хатняя старонка:%n%4
AboutSetupNote=
TranslatorNote=

; *** Buttons
ButtonBack=< &Назад
ButtonNext=&Далей >
ButtonInstall=&Усталяваць
ButtonOK=Добра
ButtonCancel=Скасаваць
ButtonYes=&Так
ButtonYesToAll=Так для &ўсіх
ButtonNo=&Не
ButtonNoToAll=Н&е для ўсіх
ButtonFinish=&Завяршыць
ButtonBrowse=&Агляд...
ButtonWizardBrowse=&Агляд...
ButtonNewFolder=&Стварыць новую папку

; *** "Select Language" dialog messages
SelectLanguageTitle=Выберыце мову ўсталявання
SelectLanguageLabel=Выберыце мову, якая будзе выкарыстоўвацца падчас усталявання:

; *** Common wizard text
ClickNext=Каб працягнуць, націсніце "Далей". Каб выйсці з праграмы ўсталявання, націсніце "Скасаваць".
BeveledLabel=
BrowseDialogTitle=Агляд папак
BrowseDialogLabel=Выберыце папку са спіса і націсніце "Добра".
NewFolderName=Новая папка

; *** "Welcome" wizard page
WelcomeLabel1=Вас вітае майстар усталявання [name]
WelcomeLabel2=Праграма ўсталюе [name/ver] на ваш камп'ютар.%n%nПерад тым як працягнуць, рэкамендуем закрыць усе іншыя праграмы.

; *** "Password" wizard page
WizardPassword=Пароль
PasswordLabel1=Гэта праграма абаронена паролем.
PasswordLabel3=Увядзіце пароль і націсніце "Далей". Паролі неабходна ўводзіць з улікам рэгістра.
PasswordEditLabel=&Пароль:
IncorrectPassword=Вы ўвялі няправільны пароль. Паспрабуйце яшчэ раз.

; *** "License Agreement" wizard page
WizardLicense=Ліцэнзійнае пагадненне
LicenseLabel=Перад тым як працягнуць, калі ласка, прачытайце наступную важную інфармацыю.
LicenseLabel3=Калі ласка, прачытайце наступнае Ліцэнзійнае пагадненне. Вам неабходна пагадзіцца з ім перад тым, як працягнуць усталяванне.
LicenseAccepted=Я &прымаю ўмовы пагаднення
LicenseNotAccepted=Я &не прымаю ўмовы пагаднення

; *** "Information" wizard pages
WizardInfoBefore=Інфармацыя
InfoBeforeLabel=Перад тым як працягнуць, прачытайце наступную важную інфармацыю.
InfoBeforeClickLabel=Калі вы будзеце гатовы працягнуць усталяванне, націсніце "Далей".
WizardInfoAfter=Інфармацыя
InfoAfterLabel=Перад тым як працягнуць, прачытайце наступную важную інфармацыю.
InfoAfterClickLabel=Калі вы будзеце гатовы працягнуць усталяванне, націсніце "Далей".

; *** "User Information" wizard page
WizardUserInfo=Інфармацыя пра карыстальніка
UserInfoDesc=Увядзіце інфармацыю пра сябе.
UserInfoName=&Імя карыстальніка:
UserInfoOrg=&Арганізацыя:
UserInfoSerial=&Серыйны нумар:
UserInfoNameRequired=Неабходна ўвесці імя.

; *** "Select Destination Location" wizard page
WizardSelectDir=Выбар папкі ўсталявання
SelectDirDesc=У якую папку неабходна ўсталяваць [name]?
SelectDirLabel3=Праграма ўсталюе [name] у наступную папку.
SelectDirBrowseLabel=Каб працягнуць, націсніце "Далей". Каб выбраць іншую папку, націсніце "Агляд".
DiskSpaceGBLabel=Патрабуецца не менш за [gb] ГБ вольнага месца на дыску.
DiskSpaceMBLabel=Патрабуецца не менш за [mb] МБ вольнага месца на дыску.
CannotInstallToNetworkDrive=Нельга ўсталяваць на сеткавы дыск.
CannotInstallToUNCPath=Нельга ўсталяваць у папку па яе шляху UNC.
InvalidPath="Вы павінны ўвесці поўны шлях з літарай дыска; напрыклад:%n%nC:\APP%n%nабо ў фармаце UNC:%n%n\\назва_сервера\\назва_рэсурсу"
InvalidDrive=Дыск або сеткавы рэсурс UNC не існуюць або яны недаступны. Выберыце іншае месца.
DiskSpaceWarningTitle=Не хапае вольнага месца на дыску
DiskSpaceWarning=Для ўсталявання неабходна не менш за %1 КБ вольнага месца, а на выбраным дыску зараз толькі %2 КБ.%n%nУсё роўна хочаце працягнуць усталяванне?
DirNameTooLong=Назва папкі або шлях да яе перавышаюць дапушчальную даўжыню.
InvalidDirName=Азначана памылковая назва папкі.
BadDirName32=Назва папкі не можа змяшчаць наступныя сімвалы: %n%n%1
DirExistsTitle=Такая папка існуе
DirExists=Папка%n%n%1%n%nужо існуе. Усё роўна ўсталяваць у гэту папку?
DirDoesntExistTitle=Такой папкі не існуе
DirDoesntExist=Папка:%n%n%1%n%nне існуе. Стварыць яе?

; *** "Select Components" wizard page
WizardSelectComponents=Выбар кампанентаў
SelectComponentsDesc=Якія кампаненты вы хочаце ўсталяваць?
SelectComponentsLabel2="Выберыце кампаненты, якія вы хочаце ўсталяваць; здыміце птушкі з тых кампанентаў, якія вы не хочаце ўсталёўваць. Калі будзеце гатовы працягнуць, націсніце \"Далей\"."
FullInstallation=Поўнае ўсталяванне
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Кампактнае ўсталяванне
CustomInstallation=Выбарачнае ўсталяванне
NoUninstallWarningTitle=Усталяваныя кампаненты
NoUninstallWarning=Праграма ўсталявання выявіла, што на вашым камп'ютары ўжо ўсталяваны наступныя кампаненты:%n%n%1%n%nКалі скасаваць выбар гэтых кампанентаў, яны не будуць выдалены.%n%nУсё роўна хочаце працягнуць усталяванне?
ComponentSize1=%1 КБ
ComponentSize2=%1 МБ
ComponentsDiskSpaceGBLabel=Бягучы выбар патрабуе не менш за [gb] ГБ вольнага месца на дыску.
ComponentsDiskSpaceMBLabel=Бягучы выбар патрабуе не менш за [mb] МБ вольнага месца на дыску.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Выберыце дадатковыя задачы
SelectTasksDesc=Якія дадатковыя задачы неабходна выканаць?
SelectTasksLabel2=Выберыце дадатковыя задачы, якія неабходна выканаць падчас усталявання [name]. Пасля гэтага націсніце "Далей".

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Выберыце папку ў меню "Пуск"
SelectStartMenuFolderDesc=Дзе праграма ўсталявання павінна стварыць ярлыкі?
SelectStartMenuFolderLabel3=Праграма ўсталявання створыць ярлыкі ў наступнай папцы меню "Пуск".
SelectStartMenuFolderBrowseLabel=Каб працягнуць, націсніце "Далей". Каб выбраць іншую папку, націсніце "Агляд".
MustEnterGroupName=Неабходна ўвесці назву папкі.
GroupNameTooLong=Назва папкі або шлях да яе перавышаюць дапушчальную даўжыню.
InvalidGroupName=Азначана памылковая назва папкі.
BadGroupName=Назва папкі не можа змяшчаць наступныя сімвалы:%n%n%1
NoProgramGroupCheck2=&Не ствараць папку ў меню "Пуск"

; *** "Ready to Install" wizard page
WizardReady=Усё гатова да ўсталявання
ReadyLabel1=Праграма ўсталявання гатова пачаць усталёўваць [name] на ваш камп'ютар.
ReadyLabel2a=Каб працягнуць, націсніце "Усталяваць". Каб прагледзець або змяніць налады ўсталявання, націсніце "Назад".
ReadyLabel2b=Каб працягнуць, націсніце "Усталяваць".
ReadyMemoUserInfo=Інфармацыя пра карыстальніка:
ReadyMemoDir=Папка ўсталявання:
ReadyMemoType=Тып усталявання:
ReadyMemoComponents=Выбраныя кампаненты:
ReadyMemoGroup=Папка ў меню "Пуск":
ReadyMemoTasks=Дадатковыя задачы:

; *** TDownloadWizardPage wizard page and DownloadTemporaryFile
DownloadingLabel=Спампоўванне дадатковых файлаў...
ButtonStopDownload=&Спыніць спампоўванне
StopDownload=Вы сапраўды хочаце спыніць спампоўванне?
ErrorDownloadAborted=Спампоўванне перарвана
ErrorDownloadFailed=Збой спампоўвання: %1%2
ErrorDownloadSizeFailed=Збой атрымання памеру: %1 %2
ErrorFileHash1=Збой хэша файла: %1
ErrorFileHash2=Памылковы хэш файла: чакаўся %1, а знойдзены %2
ErrorProgress=Памылковы ход выканання: %1 з %2
ErrorFileSize=Памылковы памер файла: чакаўся %1, а знойдзены %2

; *** TExtractionWizardPage wizard page and Extract7ZipArchive
ExtractionLabel=Extracting additional files...
ButtonStopExtraction=&Stop extraction
StopExtraction=Are you sure you want to stop the extraction?
ErrorExtractionAborted=Extraction aborted
ErrorExtractionFailed=Extraction failed: %1

; *** "Preparing to Install" wizard page
WizardPreparing=Падрыхтоўка да ўсталявання
PreparingDesc=Праграма ўсталявання рыхтуецца ўсталяваць [name] на ваш камп'ютар.
PreviousInstallNotCompleted=Усталяванне або выдаленне папярэдняй версіі праграмы не былі завершаны. Неабходны перазапуск вашага камп'ютара, каб завяршыць папярэдняе ўсталяванне.%n%nПасля перазапуску запусціце праграму ўсталявання паўторна, каб завяршыць працэс усталявання [name].
CannotContinue=Не атрымалася працягнуць усталяванне. Націсніце "Скасаваць", каб выйсці з праграмы.
ApplicationsFound=Наступныя праграмы выкарыстоўваюць файлы, якія праграма ўсталявання павінна абнавіць. Рэкамендуецца дазволіць праграме ўсталявання закрыць гэтыя праграмы.
ApplicationsFound2=Наступныя праграмы выкарыстоўваюць файлы, якія праграма ўсталявання павінна абнавіць. Рэкамендуецца дазволіць праграме ўсталявання закрыць гэтыя праграмы. Пасля завяршэння працэсу, праграма ўсталявання паспрабуе перазапусціць іх.
CloseApplications=&Аўтаматычна закрыць гэтыя праграмы
DontCloseApplications=&Не закрываць гэтыя праграмы
ErrorCloseApplications=Праграма ўсталявання не змагла аўтаматычна закрыць усе праграмы. Перад тым як працягнуць, рэкамендуем закрыць усе праграмы, якія выкарыстоўваюць файлы, прызначаныя для абнаўлення.
PrepareToInstallNeedsRestart=Праграма ўсталявання павінна перазапусціць ваш камп'ютар. Пасля перазапуску камп'ютара, калі ласка, запусціце праграму ўсталявання яшчэ раз, каб завяршыць усталяванне [name].%n%nВыканаць перазапуск зараз?

; *** "Installing" wizard page
WizardInstalling=Усталяванне...
InstallingLabel=Пачакайце, пакуль [name] усталюецца на ваш камп'ютар.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Завяршэнне працы майстра ўсталявання [name]
FinishedLabelNoIcons=Праграма [name] усталявана на ваш камп'ютар.
FinishedLabel=Праграма [name] усталявана на ваш камп'ютар. Яе можна запусціць з дапамогай адпаведнага ярлыка.
ClickFinish=Каб выйсці з праграмы ўсталявання, націсніце "Завяршыць".
FinishedRestartLabel=Каб завяршыць усталяванне [name], неабходна перазапусціць ваш камп'ютар. Зрабіць гэта зараз?
FinishedRestartMessage=Каб завяршыць усталяванне [name], неабходна перазапусціць ваш камп'ютар.%n%nЗрабіць гэта зараз?
ShowReadmeCheck=Я хачу прагледзець файл README
YesRadio=&Так, перазапусціць камп'ютар зараз
NoRadio=&Не, я перазапушчу камп'ютар пазней
; used for example as 'Run MyProg.exe'
RunEntryExec=Выканаць %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Прагледзець %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=Неабходна ўставіць наступны дыск
SelectDiskLabel2=Устаўце дыск %1 і націсніце "Добра".%n%nКалі файлы гэтага дыска можна знайсці ў папцы, якая адрозніваецца ад азначанай ніжэй, увядзіце правільны шлях або націсніце "Агляд".
PathLabel=&Шлях:
FileNotInDir2=Файл "%1" не знойдзены ў "%2". Калі ласка, устаўце правільны дыск або выберыце іншую папку.
SelectDirectoryLabel=Калі ласка, азначце шлях да наступнага дыска.

; *** Installation phase messages
SetupAborted=Усталяванне не было завершана.%n%nВырашыце праблему і запусціце праграму ўсталявання яшчэ раз.
AbortRetryIgnoreSelectAction=Выберыце дзеянне
AbortRetryIgnoreRetry=&Паспрабаваць яшчэ раз
AbortRetryIgnoreIgnore=&Ігнараваць памылку і працягнуць
AbortRetryIgnoreCancel=Скасаваць усталяванне

; *** Installation status messages
StatusClosingApplications=Закрыццё праграм...
StatusCreateDirs=Стварэнне каталогаў...
StatusExtractFiles=Выманне файлаў...
StatusCreateIcons=Стварэнне ярлыкоў...
StatusCreateIniEntries=Стварэнне запісаў INI...
StatusCreateRegistryEntries=Стварэнне запісаў рэестра...
StatusRegisterFiles=Рэгістрацыя файлаў...
StatusSavingUninstall=Захаванне інфармацыі для выдалення...
StatusRunProgram=Завяршэнне ўсталявання...
StatusRestartingApplications=Перазапуск праграм...
StatusRollback=Адкат змен...

; *** Misc. errors
ErrorInternal2=Унутраная памылка: %1
ErrorFunctionFailedNoCode=%1 збой
ErrorFunctionFailed=%1 збой ; code %2
ErrorFunctionFailedWithMessage=%1 збой ; code %2.%n%3
ErrorExecutingProgram=Немагчыма выканаць файл:%n%1

; *** Registry errors
ErrorRegOpenKey=Памылка адкрыцця ключа рэестра:%n%1\%2
ErrorRegCreateKey=Памылка стварэння ключа рэестра:%n%1\%2
ErrorRegWriteKey=Памылка запісу ў ключ рэестра:%n%1\%2

; *** INI errors
ErrorIniEntry=Памылка стварэння запісу ў файле INI "%1".

; *** File copying errors
FileAbortRetryIgnoreSkipNotRecommended=&Прапусціць гэты файл (не рэкамендуецца)
FileAbortRetryIgnoreIgnoreNotRecommended=&Ігнараваць памылку і працягнуць (не рэкамендуецца)
SourceIsCorrupted=Зыходны файл пашкоджаны
SourceDoesntExist=Зыходны файл "%1" не існуе
ExistingFileReadOnly2=Існуючы файл нельга замяніць, бо ён пазначаны толькі для чытання.
ExistingFileReadOnlyRetry=&Выдаліць атрыбут толькі для чытання і паспрабаваць яшчэ раз
ExistingFileReadOnlyKeepExisting=&Пакінуць існуючы файл
ErrorReadingExistingDest=Адбылася памылка пры спробе прачытаць існуючы файл:
FileExistsSelectAction=Выберыце дзеянне
FileExists2=Файл ужо існуе.
FileExistsOverwriteExisting=&Перазапісаць існуючы файл
FileExistsKeepExisting=&Пакінуць існуючы файл
FileExistsOverwriteOrKeepAll=&Выконваць гэта дзеянне для ўсіх наступных канфліктаў
ExistingFileNewerSelectAction=Выберыце дзеянне
ExistingFileNewer2=Існуючы файл з'яўляцца больш новым за той, які вы спрабуеце ўсталяваць.
ExistingFileNewerOverwriteExisting=&Перазапісаць існуючы файл
ExistingFileNewerKeepExisting=&Пакінуць існуючы файл (рэкамендуецца)
ExistingFileNewerOverwriteOrKeepAll=&Выконваць гэта дзеянне для ўсіх наступных канфліктаў
ErrorChangingAttr=Адбылася памылка пры спробе змяніць атрыбуты існуючага файла:
ErrorCreatingTemp=Адбылася памылка пры спробе стварыць файл у каталогу прызначэння:
ErrorReadingSource=Адбылася памылка пры спробе прачытаць зыходны файл:
ErrorCopying=Адбылася памылка пры спробе скапіяваць файл:
ErrorReplacingExistingFile=Адбылася памылка пры спробе замяніць існуючы файл:
ErrorRestartReplace=Збой працэдуры RestartReplace:
ErrorRenamingTemp=Адбылася памылка пры спробе перайменаваць файл у каталогу прызначэння:
ErrorRegisterServer=Немагчыма зарэгістраваць DLL або OCX: %1
ErrorRegSvr32Failed=Памылка пры выкананні RegSvr32, код выхаду %1
ErrorRegisterTypeLib=Немагчыма зарэгістраваць бібліятэку тыпаў: %1

; *** Uninstall display name markings
; used for example as 'My Program (32-bit)'
UninstallDisplayNameMark=%1 (%2)
; used for example as 'My Program (32-bit, All users)'
UninstallDisplayNameMarks=%1 (%2, %3)
UninstallDisplayNameMark32Bit=32-біты
UninstallDisplayNameMark64Bit=64-біты
UninstallDisplayNameMarkAllUsers=Усе карыстальнікі
UninstallDisplayNameMarkCurrentUser=Бягучы карыстальнік

; *** Post-installation errors
ErrorOpeningReadme=Адбылася памылка пры спробе адкрыць файл README.
ErrorRestartingComputer=Праграма ўсталявання не змагла перазапусціць камп'ютар. Зрабіце гэта ўручную.

; *** Uninstaller messages
UninstallNotFound=Файл "%1" не існуе. Немагчыма выдаліць праграму.
UninstallOpenError=Не атрымалася адкрыць файл "%1". Немагчыма выдаліць праграму
UninstallUnsupportedVer=Файл пратакола для выдалення "%1" не распазнаны гэтай версіяй праграмы выдалення. Немагчыма выдаліць праграму
UninstallUnknownEntry=У файле пратакола для выдалення сустрэўся невядомы запіс (%1)
ConfirmUninstall=Вы сапраўды хочаце выдаліць %1 і ўсе кампаненты праграмы?
UninstallOnlyOnWin64=Гэту праграму можна выдаліць толькі ў 64-бітнай версіі Windows.
OnlyAdminCanUninstall=Гэту праграму можа выдаліць толькі карыстальнік з правамі адміністратара.
UninstallStatusLabel=Пачакайце, пакуль адбудзецца выдаленне %1 з вашага камп'ютара.
UninstalledAll=%1 выдалены з вашага камп'ютара.
UninstalledMost=Выдаленне %1 завершана.%n%nЧастку элементаў выдаліць не атрымалася. Вы можаце выдаліць іх уручную.
UninstalledAndNeedsRestart=Каб завяршыць выдаленне %1, неабходна перазапусціць ваш камп'ютар.%n%nЗрабіць гэта зараз?
UninstallDataCorrupted=Файл "%1" пашкоджаны. Немагчыма выдаліць праграму

; *** Uninstallation phrase messages
ConfirmDeleteSharedFileTitle=Выдаліць абагулены файл?
ConfirmDeleteSharedFile2=Сістэма паказвае, што наступны абагулены файл больш не выкарыстоўваецца ніякімі іншымі праграмамі. Выдаліць гэты абагулены файл?%n%nКалі якія-небудзь праграмы ўсё яшчэ выкарыстоўваць яго і ён будзе выдалены, то яны могуць працаваць няправільна. Калі вы не ўпэўнены, выберыце "Не". Дадзены файл ніяк не пашкодзіць вашай сістэме.
SharedFileNameLabel=Назва файла:
SharedFileLocationLabel=Размяшчэнне:
WizardUninstalling=Статус выдалення
StatusUninstalling=Выдаленне %1...

; *** Shutdown block reasons
ShutdownBlockReasonInstallingApp=Усталяванне %1.
ShutdownBlockReasonUninstallingApp=Выдаленне %1.

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 версія %2
AdditionalIcons=Дадатковыя ярлыкі:
CreateDesktopIcon=Стварыць ярлык на &працоўным стале
CreateQuickLaunchIcon=Стварыць ярлык на панэлі &хуткага запуску
ProgramOnTheWeb=Сайт %1 у інтэрнэце
UninstallProgram=Выдаленне %1
LaunchProgram=Запусціць %1
AssocFileExtension=&Звязаць %1 з файламі, якія маюць пашырэнне %2
AssocingFileExtension=&Звязаць %1 з файламі, якія маюць пашырэнне %2...
AutoStartProgramGroupDescription=Аўтазапуск:
AutoStartProgram=Аўтаматычна запускаць %1
AddonHostProgramNotFound=%1 не знойдзены ў азначанай вамі папцы.%n%nУсё роўна хочаце працягнуць?
