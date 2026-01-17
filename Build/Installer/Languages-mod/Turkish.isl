; *** Inno Setup version 6.5.0+ Turkish messages ***
; Language	"Turkce" Turkish Translate by "Ceviren"	Kaya Zeren translator@zeron.net
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
LanguageName=T<00FC>rk<00E7>e
LanguageID=$041f
; LanguageCodePage should always be set if possible, even if this file is Unicode
; For English it's set to zero anyway because English only uses ASCII characters
LanguageCodePage=1254
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

; *** Uygulama başlıkları
SetupAppTitle=Kurulum yardımcısı
SetupWindowTitle=%1 - Kurulum yardımcısı
UninstallAppTitle=Kaldırma yardımcısı
UninstallAppFullTitle=%1 kaldırma yardımcısı

; *** Çeşitli ortak metinler
InformationTitle=Bilgi
ConfirmTitle=Onay
ErrorTitle=Hata

; *** Kurulum yükleyici iletileri
SetupLdrStartupMessage=%1 uygulaması kurulacak. İlerlemek istiyor musunuz?
LdrCannotCreateTemp=Geçici dosya oluşturulamadığından kurulum iptal edildi
LdrCannotExecTemp=Geçici klasördeki dosya çalıştırılamadığından kurulum iptal edildi
HelpTextNote=

; *** Başlangıç hata iletileri
LastErrorMessage=%1.%n%nHata %2: %3
SetupFileMissing=Kurulum klasöründe %1 dosyası eksik. Lütfen sorunu çözün ya da uygulamanın yeni bir kopyasıyla yeniden deneyin.
SetupFileCorrupt=Kurulum dosyaları bozulmuş. Lütfen uygulamanın yeni bir kopyasıyla yeniden kurmayı deneyin.
SetupFileCorruptOrWrongVer=Kurulum dosyaları bozulmuş ya da bu kurulum yardımcısı sürümü ile uyumlu değil. Lütfen sorunu çözün ya da uygulamanın yeni bir kopyasıyla yeniden kurmayı deneyin.
InvalidParameter=Komut satırında geçersiz bir parametre yazılmış:%n%n%1
SetupAlreadyRunning=Kurulum yardımcısı zaten çalışıyor.
WindowsVersionNotSupported=Bu uygulama, bilgisayarınızda yüklü olan Windows sürümü ile uyumlu değil.
WindowsServicePackRequired=Bu uygulama, %1 hizmet paketi %2 ve üzerindeki sürümler ile çalışır.
NotOnThisPlatform=Bu uygulama, %1 üzerinde çalışmaz.
OnlyOnThisPlatform=Bu uygulama, %1 üzerinde çalıştırılmalıdır.
OnlyOnTheseArchitectures=Bu uygulama, yalnızca şu işlemci mimarileri için tasarlanmış Windows sürümleriyle çalışır:%n%n%1
WinVersionTooLowError=Bu uygulama için %1 sürüm %2 ya da üzeri gereklidir.
WinVersionTooHighError=Bu uygulama, '%1' sürüm '%2' ya da üzerine kurulamaz.
AdminPrivilegesRequired=Bu uygulamayı kurmak için Yönetici yetkileri olan bir kullanıcı ile oturum açılmış olmalıdır.
PowerUserPrivilegesRequired=Bu uygulamayı kurarken, Yönetici ya da Güçlü Kullanıcılar grubundaki bir kullanıcı ile oturum açılmış olması gereklidir.
SetupAppRunningError=Kurulum yardımcısı %1 uygulamasının çalışmakta olduğunu algıladı.%n%nLütfen uygulamanın çalışan tüm kopyalarını kapatıp, ilerlemek için Tamam, kurulum yardımcısından çıkmak için İptal üzerine tıklayın.
UninstallAppRunningError=Kaldırma yardımcısı, %1 uygulamasının çalışmakta olduğunu algıladı.%n%nLütfen uygulamanın çalışan tüm kopyalarını kapatıp, ilerlemek için Tamam ya da kaldırma yardımcısından çıkmak için İptal üzerine tıklayın.

; *** Başlangıç soruları
PrivilegesRequiredOverrideTitle=Kurulum kipini seçin
PrivilegesRequiredOverrideInstruction=Kurulum kipini seçin
PrivilegesRequiredOverrideText1=%1 tüm kullanıcılar için (yönetici izinleri gerekir) ya da yalnızca sizin hesabınız için kurulabilir.
PrivilegesRequiredOverrideText2=%1 yalnızca sizin hesabınız için ya da tüm kullanıcılar için (yönetici izinleri gerekir) kurulabilir.
PrivilegesRequiredOverrideAllUsers=&Tüm kullanıcılar için kurulsun
PrivilegesRequiredOverrideAllUsersRecommended=&Tüm kullanıcılar için kurulsun (önerilir)
PrivilegesRequiredOverrideCurrentUser=&Yalnızca geçerli kullanıcı için kurulsun
PrivilegesRequiredOverrideCurrentUserRecommended=&Yalnızca geçerli kullanıcı için kurulsun (önerilir)

; *** Çeşitli hata metinleri
ErrorCreatingDir=Kurulum yardımcısı "%1" klasörünü oluşturamadı.
ErrorTooManyFilesInDir="%1" klasörü içinde çok sayıda dosya olduğundan bir dosya oluşturulamadı

; *** Ortak kurulum iletileri
ExitSetupTitle=Kurulum yardımcısından çık
ExitSetupMessage=Kurulum tamamlanmadı. Şimdi çıkarsanız, uygulama kurulmayacak.%n%nKurulumu tamamlamak için istediğiniz zaman kurulum yardımcısını yeniden çalıştırabilirsiniz.%n%nKurulum yardımcısından çıkılsın mı?
AboutSetupMenuItem=Kurulum h&akkında...
AboutSetupTitle=Kurulum hakkında
AboutSetupMessage=%1 %2 sürümü%n%3%n%n%1 ana sayfa:%n%4
AboutSetupNote=
TranslatorNote=

; *** Düğmeler
ButtonBack=< Ö&nceki
ButtonNext=&Sonraki >
ButtonInstall=&Kur
ButtonOK=Tamam
ButtonCancel=İptal
ButtonYes=E&vet
ButtonYesToAll=&Tümüne evet
ButtonNo=&Hayır
ButtonNoToAll=Tümüne ha&yır
ButtonFinish=&Bitti
ButtonBrowse=&Göz at...
ButtonWizardBrowse=Göz a&t...
ButtonNewFolder=Ye&ni klasör oluştur

; *** "Kurulum dilini seçin" sayfası iletileri
SelectLanguageTitle=Kurulum Yardımcısı dilini seçin
SelectLanguageLabel=Kurulum süresince kullanılacak dili seçin.

; *** Ortak metinler
ClickNext=İlerlemek için Sonraki, çıkmak için İptal üzerine tıklayın.
BeveledLabel=
BrowseDialogTitle=Klasöre göz at
BrowseDialogLabel=Aşağıdaki listeden bir klasör seçip, Tamam üzerine tıklayın.
NewFolderName=Yeni klasör 

; *** "Karşılama" sayfası
WelcomeLabel1=[name] Kurulum yardımcısına hoş geldiniz.
WelcomeLabel2=Bilgisayarınıza [name/ver] uygulaması kurulacak.%n%nİlerlemeden önce çalışan diğer tüm uygulamaları kapatmanız önerilir.

; *** "Parola" sayfası
WizardPassword=Parola
PasswordLabel1=Bu kurulum parola korumalıdır.
PasswordLabel3=Lütfen parolayı yazın ve ilerlemek için Sonraki üzerine tıklayın. Parolalar büyük küçük harflere duyarlıdır.
PasswordEditLabel=&Parola:
IncorrectPassword=Yazdığınız parola doğru değil. Lütfen yeniden deneyin.

; *** "Lisans anlaşması" sayfası
WizardLicense=Lisans anlaşması
LicenseLabel=Lütfen ilerlemeden önce aşağıdaki önemli bilgileri okuyun.
LicenseLabel3=Lütfen aşağıdaki lisans anlaşmasını okuyun. Uygulamayı kurmak için bu anlaşmayı kabul etmelisiniz.
LicenseAccepted=Anlaşmayı kabul &ediyorum.
LicenseNotAccepted=Anlaşmayı kabul et&miyorum.

; *** "Bilgiler" sayfası
WizardInfoBefore=Bilgiler
InfoBeforeLabel=Lütfen ilerlemeden önce aşağıdaki önemli bilgileri okuyun.
InfoBeforeClickLabel=Uygulamayı kurmaya hazır olduğunuzda Sonraki üzerine tıklayın.
WizardInfoAfter=Bilgiler
InfoAfterLabel=Lütfen ilerlemeden önce aşağıdaki önemli bilgileri okuyun.
InfoAfterClickLabel=Uygulamayı kurmaya hazır olduğunuzda Sonraki üzerine tıklayın.

; *** "Kullanıcı bilgileri" sayfası
WizardUserInfo=Kullanıcı bilgileri
UserInfoDesc=Lütfen bilgilerinizi yazın.
UserInfoName=K&ullanıcı adı:
UserInfoOrg=Ku&rum:
UserInfoSerial=&Seri numarası:
UserInfoNameRequired=Bir ad yazmalısınız.

; *** "Kurulum konumunu seçin" sayfası
WizardSelectDir=Kurulum konumunu seçin
SelectDirDesc=[name] nereye kurulsun?
SelectDirLabel3=[name] uygulaması şu klasöre kurulacak.
SelectDirBrowseLabel=İlerlemek icin Sonraki üzerine tıklayın. Farklı bir klasör seçmek için Göz at üzerine tıklayın.
DiskSpaceGBLabel=En az [gb] GB boş disk alanı gereklidir.
DiskSpaceMBLabel=En az [mb] MB boş disk alanı gereklidir.
CannotInstallToNetworkDrive=Uygulama bir ağ sürücüsü üzerine kurulamaz.
CannotInstallToUNCPath=Uygulama bir UNC yolu üzerine (\\yol gibi) kurulamaz.
InvalidPath=Sürücü adı ile tam yolu yazmalısınız. Örnek: %n%nC:\APP%n%n ya da şu şekilde bir UNC yolu:%n%n\\sunucu\paylaşım
InvalidDrive=Sürücü ya da UNC paylaşımı yok ya da erişilemiyor. Lütfen başka bir tane seçin.
DiskSpaceWarningTitle=Yeterli boş disk alanı yok
DiskSpaceWarning=Kurulum için %1 KB boş alan gerekli, ancak seçilmiş sürücüde yalnızca %2 KB boş alan var.%n%nGene de ilerlemek istiyor musunuz?
DirNameTooLong=Klasör adı ya da yol çok uzun.
InvalidDirName=Klasör adı geçersiz.
BadDirName32=Klasör adlarında şu karakterler bulunamaz:%n%n%1
DirExistsTitle=Klasör zaten var
DirExists=Klasör:%n%n%1%n%nzaten var. Kurulum için bu klasörü kullanmak ister misiniz?
DirDoesntExistTitle=Klasör bulunamadı
DirDoesntExist=Klasör:%n%n%1%n%nbulunamadı.Klasörün oluşturmasını ister misiniz?

; *** "Bileşenleri seçin" sayfası
WizardSelectComponents=Bileşenleri seçin
SelectComponentsDesc=Hangi bileşenler kurulacak?
SelectComponentsLabel2=Kurmak istediğiniz bileşenleri seçin; kurmak istemediğiniz bileşenlerin işaretini kaldırın. İlerlemeye hazır olduğunuzda Sonraki üzerine tıklayın.
FullInstallation=Tam kurulum
; Olabiliyorsa 'Compact' ifadesini kendi dilinizde 'Minimal' anlamında çevirmeyin
CompactInstallation=Normal kurulum
CustomInstallation=Özel kurulum
NoUninstallWarningTitle=Bileşenler zaten var
NoUninstallWarning=Şu bileşenlerin bilgisayarınızda zaten kurulu olduğu algılandı:%n%n%1%n%n Bu bileşenlerin işaretlerinin kaldırılması bileşenleri kaldırmaz.%n%nGene de ilerlemek istiyor musunuz?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceGBLabel=Seçilmiş bileşenler için diskte en az [gb] GB boş alan bulunması gerekli.
ComponentsDiskSpaceMBLabel=Seçilmiş bileşenler için diskte en az [mb] MB boş alan bulunması gerekli.

; *** "Ek işlemleri seçin" sayfası
WizardSelectTasks=Ek işlemleri seçin
SelectTasksDesc=Başka hangi işlemler yapılsın?
SelectTasksLabel2=[name] kurulumu sırasında yapılmasını istediğiniz ek işleri seçin ve Sonraki üzerine tıklayın.

; *** "Başlat menüsü klasörünü seçin" sayfası
WizardSelectProgramGroup=Başlat menüsü klasörünü seçin
SelectStartMenuFolderDesc=Uygulamanın kısayolları nereye eklensin?
SelectStartMenuFolderLabel3=Kurulum yardımcısı uygulama kısayollarını aşağıdaki Başlat menüsü klasörüne ekleyecek.
SelectStartMenuFolderBrowseLabel=İlerlemek için Sonraki üzerine tıklayın. Farklı bir klasör seçmek için Göz at üzerine tıklayın.
MustEnterGroupName=Bir klasör adı yazmalısınız.
GroupNameTooLong=Klasör adı ya da yol çok uzun.
InvalidGroupName=Klasör adı geçersiz.
BadGroupName=Klasör adında şu karakterler bulunamaz:%n%n%1
NoProgramGroupCheck2=Başlat menüsü klasörü &oluşturulmasın

; *** "Kurulmaya hazır" sayfası
WizardReady=Kurulmaya hazır
ReadyLabel1=[name] bilgisayarınıza kurulmaya hazır.
ReadyLabel2a=Kuruluma başlamak için Sonraki üzerine, ayarları gözden geçirip değiştirmek için Önceki üzerine tıklayın.
ReadyLabel2b=Kuruluma başlamak için Sonraki üzerine tıklayın.
ReadyMemoUserInfo=Kullanıcı bilgileri:
ReadyMemoDir=Kurulum konumu:
ReadyMemoType=Kurulum türü:
ReadyMemoComponents=Seçilmiş bileşenler:
ReadyMemoGroup=Başlat menüsü klasörü:
ReadyMemoTasks=Ek işlemler:

; *** TDownloadWizardPage wizard page and DownloadTemporaryFile
DownloadingLabel2=Ek dosyalar indiriliyor...
ButtonStopDownload=İndirmeyi &durdur
StopDownload=İndirmeyi durdurmak istediğinize emin misiniz?
ErrorDownloadAborted=İndirme durduruldu
ErrorDownloadFailed=İndirilemedi: %1 %2
ErrorDownloadSizeFailed=Boyut alınamadı: %1 %2
ErrorProgress=Adım geçersiz: %1 / %2
ErrorFileSize=Dosya boyutu geçersiz: %1 olması gerekirken %2

; *** TExtractionWizardPage wizard page and Extract7ZipArchive
ExtractingLabel=Ek dosyalar ayıklanıyor...
ButtonStopExtraction=Ayıklamayı &durdur
StopExtraction=Ayıklamayı durdurmak istediğinize emin misiniz?
ErrorExtractionAborted=Ayıklama durduruldu
ErrorExtractionFailed=Ayıklanamadı: %1

; *** Arşiv ayıklanamadı sorunu ayrıntıları
ArchiveIncorrectPassword=Parola yanlış
ArchiveIsCorrupted=Arşiv dosyası bozuk
ArchiveUnsupportedFormat=Arşiv biçimi desteklenmiyor

; *** "Kuruluma hazırlanılıyor" sayfası
WizardPreparing=Kuruluma hazırlanılıyor
PreparingDesc=[name] bilgisayarınıza kurulmaya hazırlanıyor.
PreviousInstallNotCompleted=Önceki uygulama kurulumu ya da kaldırılması tamamlanmamış. Bu kurulumun tamamlanması için bilgisayarınızı yeniden başlatmalısınız.%n%nBilgisayarınızı yeniden başlattıktan sonra işlemi tamamlamak için [name] kurulum yardımcısını yeniden çalıştırın.
CannotContinue=Kurulum yapılamadı. Çıkmak için İptal üzerine tıklayın.
ApplicationsFound=Kurulum yardımcısı tarafından güncellenmesi gereken dosyalar, şu uygulamalar tarafından kullanıyor. Kurulum yardımcısının bu uygulamaları otomatik olarak kapatmasına izin vermeniz önerilir.
ApplicationsFound2=Kurulum yardımcısı tarafından güncellenmesi gereken dosyalar, şu uygulamalar tarafından kullanıyor. Kurulum yardımcısının bu uygulamaları otomatik olarak kapatmasına izin vermeniz önerilir. Kurulum tamamlandıktan sonra, uygulamalar yeniden başlatılmaya çalışılacak.
CloseApplications=&Uygulamalar kapatılsın
DontCloseApplications=Uygulamalar &kapatılmasın
ErrorCloseApplications=Kurulum yardımcısı uygulamaları kapatamadı. Kurulum yardımcısı tarafından güncellenmesi gereken dosyaları kullanan uygulamaları el ile kapatmanız önerilir.
PrepareToInstallNeedsRestart=Kurulum için bilgisayarın yeniden başlatılması gerekiyor. Bilgisayarı yeniden başlattıktan sonra [name] kurulumunu tamamlamak için kurulum yardımcısını yeniden çalıştırın.%n%nBilgisayarı şimdi yeniden başlatmak ister misiniz?

; *** "Kuruluyor" sayfası
WizardInstalling=Kuruluyor
InstallingLabel=Lütfen [name] bilgisayarınıza kurulurken bekleyin.

; *** "Kurulum Tamamlandı" sayfası
FinishedHeadingLabel=[name] kurulum yardımcısı tamamlanıyor
FinishedLabelNoIcons=Bilgisayarınıza [name] kurulumu tamamlandı.
FinishedLabel=Bilgisayarınıza [name] kurulumu tamamlandı. Simgeleri yüklemeyi seçtiyseniz, simgelere tıklayarak uygulamayı başlatabilirsiniz.
ClickFinish=Kurulum yardımcısından çıkmak için Bitti üzerine tıklayın.
FinishedRestartLabel=[name] kurulumunun tamamlanması için, bilgisayarınız yeniden başlatılmalı. Şimdi yeniden başlatmak ister misiniz?
FinishedRestartMessage=[name] kurulumunun tamamlanması için, bilgisayarınız yeniden başlatılmalı.%n%nŞimdi yeniden başlatmak ister misiniz?
ShowReadmeCheck=Evet README dosyası görüntülensin
YesRadio=&Evet, bilgisayar şimdi yeniden başlatılsın
NoRadio=&Hayır, bilgisayarı daha sonra yeniden başlatacağım
; used for example as 'Run MyProg.exe'
RunEntryExec=%1 çalıştırılsın
; used for example as 'View Readme.txt'
RunEntryShellExec=%1 görüntülensin

; *** "Kurulum için sıradaki disk gerekli" iletileri
ChangeDiskTitle=Kurulum yardımcısı sıradaki diske gerek duyuyor
SelectDiskLabel2=Lütfen %1 numaralı diski takıp Tamam üzerine tıklayın.%n%nDiskteki dosyalar aşağıdakinden farklı bir klasörde bulunuyorsa, doğru yolu yazın ya da Göz at üzerine tıklayarak doğru klasörü seçin.
PathLabel=&Yol:
FileNotInDir2="%1" dosyası "%2" içinde bulunamadı. Lütfen doğru diski takın ya da başka bir klasör seçin.
SelectDirectoryLabel=Lütfen sonraki diskin konumunu belirtin.

; *** Kurulum aşaması iletileri
SetupAborted=Kurulum tamamlanamadı.%n%nLütfen sorunu düzelterek kurulum yardımcısını yeniden çalıştırın.
AbortRetryIgnoreSelectAction=Yapılacak işlemi seçin
AbortRetryIgnoreRetry=&Yeniden denensin
AbortRetryIgnoreIgnore=&Sorun yok sayılıp ilerlensin
AbortRetryIgnoreCancel=Kurulum iptal edilsin
RetryCancelSelectAction=İşlem seçin
RetryCancelRetry=&Yeniden dene
RetryCancelCancel=İptal

; *** Kurulum durumu iletileri
StatusClosingApplications=Uygulamalar kapatılıyor...
StatusCreateDirs=Klasörler oluşturuluyor...
StatusExtractFiles=Dosyalar ayıklanıyor...
StatusDownloadFiles=Dosyalar indiriliyor...
StatusCreateIcons=Kısayollar oluşturuluyor...
StatusCreateIniEntries=INI kayıtları oluşturuluyor...
StatusCreateRegistryEntries=Kayıt Defteri kayıtları oluşturuluyor...
StatusRegisterFiles=Dosyalar kaydediliyor...
StatusSavingUninstall=Kaldırma bilgileri kaydediliyor...
StatusRunProgram=Kurulum tamamlanıyor...
StatusRestartingApplications=Uygulamalar yeniden başlatılıyor...
StatusRollback=Değişiklikler geri alınıyor...

; *** Çeşitli hata iletileri
ErrorInternal2=İç hata: %1
ErrorFunctionFailedNoCode=%1 tamamlanamadı.
ErrorFunctionFailed=%1 tamamlanamadı; kod %2
ErrorFunctionFailedWithMessage=%1 tamamlanamadı; kod %2.%n%3
ErrorExecutingProgram=Şu dosya yürütülemedi:%n%1

; *** Kayıt defteri hataları
ErrorRegOpenKey=Kayıt defteri anahtarı açılırken bir sorun çıktı:%n%1%2
ErrorRegCreateKey=Kayıt defteri anahtarı eklenirken bir sorun çıktı:%n%1%2
ErrorRegWriteKey=Kayıt defteri anahtarı yazılırken bir sorun çıktı:%n%1%2

; *** INI hataları
ErrorIniEntry="%1" dosyasına INI kaydı eklenirken bir sorun çıktı.

; *** Dosya kopyalama hataları
FileAbortRetryIgnoreSkipNotRecommended=&Bu dosya atlansın (önerilmez)
FileAbortRetryIgnoreIgnoreNotRecommended=&Sorun yok sayılıp ilerlensin (önerilmez)
SourceIsCorrupted=Kaynak dosya bozulmuş
SourceDoesntExist="%1" kaynak dosyası bulunamadı
SourceVerificationFailed=Kaynak dosya doğrulanamadı: %1
VerificationSignatureDoesntExist="%1" imza dosyası bulunamadı
VerificationSignatureInvalid="%1" imza dosyası geçersiz
VerificationKeyNotFound="%1" imza dosyası bilinmeyen bir anahtar kullanıyor
VerificationFileNameIncorrect=Dosyanın adı yanlış
VerificationFileTagIncorrect=Dosyanın etiketi yanlış
VerificationFileSizeIncorrect=Dosyanın boyutu yanlış
VerificationFileHashIncorrect=Dosyanın karma değeri yanlış
ExistingFileReadOnly2=Var olan dosya salt okunabilir olarak işaretlenmiş olduğundan üzerine yazılamadı.
ExistingFileReadOnlyRetry=&Salt okunur işareti kaldırılıp yeniden denensin
ExistingFileReadOnlyKeepExisting=&Var olan dosya korunsun
ErrorReadingExistingDest=Var olan dosya okunmaya çalışılırken bir sorun çıktı.
FileExistsSelectAction=Yapılacak işlemi seçin
FileExists2=Dosya zaten var.
FileExistsOverwriteExisting=&Var olan dosyanın üzerine yazılsın
FileExistsKeepExisting=Var &olan dosya korunsun
FileExistsOverwriteOrKeepAll=&Sonraki çakışmalarda da bu işlem yapılsın
ExistingFileNewerSelectAction=Yapılacak işlemi seçin
ExistingFileNewer2=Var olan dosya, kurulum yardımcısı tarafından yazılmaya çalışılandan daha yeni.
ExistingFileNewerOverwriteExisting=&Var olan dosyanın üzerine yazılsın
ExistingFileNewerKeepExisting=Var &olan dosya korunsun (önerilir)
ExistingFileNewerOverwriteOrKeepAll=&Sonraki çakışmalarda bu işlem yapılsın
ErrorChangingAttr=Var olan dosyanın öznitelikleri değiştirilirken bir sorun çıktı:
ErrorCreatingTemp=Kurulum klasöründe bir dosya oluşturulurken sorun çıktı:
ErrorReadingSource=Kaynak dosya okunurken sorun çıktı:
ErrorCopying=Bir dosya kopyalanırken sorun çıktı:
ErrorDownloading=Bir dosya indirilirken sorun çıktı:
ErrorExtracting=Bir arşiv ayıklanırken sorun çıktı:
ErrorReplacingExistingFile=Var olan dosya değiştirilirken sorun çıktı:
ErrorRestartReplace=Yeniden başlatmada üzerine yazılamadı:
ErrorRenamingTemp=Kurulum klasöründeki bir dosyanın adı değiştirilirken sorun çıktı:
ErrorRegisterServer=DLL/OCX kayıt edilemedi: %1
ErrorRegSvr32Failed=RegSvr32 işlemi şu kod ile tamamlanamadı: %1
ErrorRegisterTypeLib=Tür kitaplığı kayıt defterine eklenemedi: %1

; *** Kaldırma sırasında görüntülenecek ad işaretleri
; used for example as 'My Program (32-bit)'
UninstallDisplayNameMark=%1 (%2)
; used for example as 'My Program (32-bit, All users)'
UninstallDisplayNameMarks=%1 (%2, %3)
UninstallDisplayNameMark32Bit=32 bit
UninstallDisplayNameMark64Bit=64 bit
UninstallDisplayNameMarkAllUsers=Tüm kullanıcılar
UninstallDisplayNameMarkCurrentUser=Geçerli kullanıcı

; *** Kurulum sonrası hataları
ErrorOpeningReadme=README dosyası açılırken sorun çıktı.
ErrorRestartingComputer=Kurulum yardımcısı bilgisayarınızı yeniden başlatamıyor. Lütfen bilgisayarınızı yeniden başlatın.

; *** Kaldırma yardımcısı iletileri
UninstallNotFound="%1" dosyası bulunamadı. Uygulama kaldırılamıyor.
UninstallOpenError="%1" dosyası açılamadı. Uygulama kaldırılamıyor.
UninstallUnsupportedVer="%1" uygulama kaldırma günlük dosyasının biçimi, bu kaldırma yardımcısı sürümü tarafından anlaşılamadı. Uygulama kaldırılamıyor.
UninstallUnknownEntry=Kaldırma günlüğünde bilinmeyen bir kayıt (%1) bulundu.
ConfirmUninstall=%1 uygulamasını tüm bileşenleri ile birlikte tamamen kaldırmak istediğinize emin misiniz?
UninstallOnlyOnWin64=Bu kurulum yalnızca 64 bit Windows üzerinden kaldırılabilir.
OnlyAdminCanUninstall=Bu kurulum yalnızca yönetici yetkileri olan bir kullanıcı tarafından kaldırılabilir.
UninstallStatusLabel=Lütfen %1 uygulaması bilgisayarınızdan kaldırılırken bekleyin.
UninstalledAll=%1 uygulaması bilgisayarınızdan kaldırıldı.
UninstalledMost=%1 uygulaması kaldırıldı.%n%nBazı bileşenler kaldırılamadı. Bunları el ile silebilirsiniz.
UninstalledAndNeedsRestart=%1 kaldırma işleminin tamamlanması için bilgisayarınızın yeniden başlatılması gerekli.%n%nŞimdi yeniden başlatmak ister misiniz?
UninstallDataCorrupted="%1" dosyası bozulmuş. Kaldırılamıyor.

; *** Kaldırma aşaması iletileri
ConfirmDeleteSharedFileTitle=Paylaşılan dosya silinsin mi?
ConfirmDeleteSharedFile2=Sisteme göre, paylaşılan şu dosya başka bir uygulama tarafından kullanılmıyor ve kaldırılabilir. Bu paylaşılmış dosyayı silmek ister misiniz?%n%nBu dosya, başka herhangi bir uygulama tarafından kullanılıyor ise, silindiğinde diğer uygulama düzgün çalışmayabilir. Emin değilseniz Hayır üzerine tıklayın. Dosyayı sisteminizde bırakmanın bir zararı olmaz.
SharedFileNameLabel=Dosya adı:
SharedFileLocationLabel=Konum:
WizardUninstalling=Kaldırma durumu
StatusUninstalling=%1 kaldırılıyor...

; *** Kapatmayı engelleme nedenleri
ShutdownBlockReasonInstallingApp=%1 kuruluyor.
ShutdownBlockReasonUninstallingApp=%1 kaldırılıyor.

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 %2 sürümü
AdditionalIcons=Ek simgeler:
CreateDesktopIcon=Masaüstü simg&esi oluşturulsun
CreateQuickLaunchIcon=Hızlı başlat simgesi &oluşturulsun
ProgramOnTheWeb=%1 sitesi
UninstallProgram=%1 uygulamasını kaldır
LaunchProgram=%1 uygulamasını çalıştır
AssocFileExtension=%1 &uygulaması ile %2 dosya uzantısı ilişkilendirilsin
AssocingFileExtension=%1 uygulaması ile %2 dosya uzantısı ilişkilendiriliyor...
AutoStartProgramGroupDescription=Başlangıç:
AutoStartProgram=%1 otomatik olarak başlatılsın
AddonHostProgramNotFound=%1 seçtiğiniz klasörde bulunamadı.%n%nYine de ilerlemek istiyor musunuz?
