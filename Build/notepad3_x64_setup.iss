;* Notepad3 - Installer script x64 and arm64
;*
;* (c) Rizonesoft 2008-2024

;Requirements: Inno Setup 6.2.x
;Inno Setup: https://jrsoftware.org/isinfo.php

;Thanks to "Wilenty" for his help in improving the INNO Setup installer

 
;Preprocessor related stuff
// if you compile a "beta, rc or rc2" version, then comment/un-comment the appropriate setting:
;#define VRSN=" beta"
;#define VRSN=" rc"
;#define VRSN=" rc2"
;#define VRSN=" rc3"
// but, if not a "beta, rc or rc2" version, then comment above settings and un-comment below setting :)
#define VRSN
#ifndef VRSN
  #error Please set any of the above: #define VRSN(...)
#endif

// choose which architecture should be compiled (one of below)
;#define Arch="x86"
#define Arch="x64"
#ifndef Arch
  #error Please set any of the above: #define Arch=(...)
#endif

#define sse_required
#define sse2_required

// 6.0.5 in hex: 0x06000500
#if VER < 0x06000500
  #error Update your Inno Setup version (6.0.5 or newer)
#endif

#define bindir "..\Bin\"
#define RLSdir "Release_"+Arch+"_v143"

#ifnexist bindir + RLSdir + "\Notepad3.exe"
  #pragma error "Compile Notepad3 "+Arch+" first"
#endif

#ifnexist bindir + RLSdir + "\minipath.exe"
  #pragma error "Compile MiniPath "+Arch+" first"
#endif

#ifnexist bindir + RLSdir + "\grepWinNP3.exe"
  #pragma error "Compile grepWinNP3 "+Arch+" first"
#endif

#ifnexist bindir + RLSdir + "\np3encrypt.exe"
  #pragma error "Compile np3encrypt "+Arch+" first"
#endif

#define app_name "Notepad3"
#define app_publisher "Rizonesoft"
// 6.2.0 in hex: 0x06020000
#if VER < 0x06020000
  #define app_version GetFileVersion(bindir + RLSdir + "\Notepad3.exe")
#else
  #define app_version GetVersionNumbersString(bindir + RLSdir + "\Notepad3.exe")
#endif
#define app_copyright "Copyright © 2008-" + GetDateTimeString("yyyy", "", "") + " Rizonesoft"
#define quick_launch "{userappdata}\Microsoft\Internet Explorer\Quick Launch"


[Setup]
AppId={#app_name}
AppName={#app_name} ({#Arch}){#VRSN}
AppVersion={#app_version}{#VRSN}
AppVerName={#app_name} {#app_version}
AppPublisher={#app_publisher}
AppPublisherURL=https://rizonesoft.com/
AppSupportURL=https://www.rizonesoft.com/documents/
AppUpdatesURL=https://www.rizonesoft.com/downloads/notepad3/
AppContact=https://www.rizonesoft.com/#contact
AppCopyright={#app_copyright}
VersionInfoVersion={#app_version}
UninstallDisplayIcon={app}\Notepad3.exe
UninstallDisplayName={#app_name} ({#Arch}) {#app_version}{#VRSN}
DefaultDirName={commonpf}\Notepad3
LicenseFile="..\License.txt"
OutputDir=.\Packages
OutputBaseFilename={#app_name}_{#app_version}{#StringChange(VRSN, " ", "_")}_{#Arch}_Setup
WizardStyle=modern
WizardSmallImageFile=.\Resources\WizardSmallImageFile.bmp
Compression=lzma2/max
InternalCompressLevel=max
SolidCompression=yes
EnableDirDoesntExistWarning=no
AllowNoIcons=yes
ShowTasksTreeLines=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DisableWelcomePage=no
AllowCancelDuringInstall=yes
UsedUserAreasWarning=no
MinVersion=0,6.1sp1
#if Arch == "x86"
ArchitecturesAllowed=x86 x64 arm64
ArchitecturesInstallIn64BitMode=
#endif
#if Arch == "x64"
ArchitecturesAllowed=x64 arm64
ArchitecturesInstallIn64BitMode=x64 arm64
#endif
CloseApplications=true
SetupMutex={#app_name}_setup_mutex,Global\{#app_name}_setup_mutex
SetupIconFile=.\Resources\Notepad3SetupIconFile.ico
WizardImageFile=.\Resources\WizardImageFileSmall.bmp


[Languages]
Name: "enu"; MessagesFile: "compiler:Default.isl"
Name: "afk"; MessagesFile: "compiler:Languages-mod\Afrikaans.isl"
Name: "bel"; MessagesFile: "compiler:Languages-mod\Belarusian.isl"
Name: "deu"; MessagesFile: "compiler:Languages-mod\German.isl"
Name: "ell"; MessagesFile: "compiler:Languages-mod\Greek.isl"
Name: "eng"; MessagesFile: "compiler:Languages-mod\EnglishBritish.isl"
Name: "esn"; MessagesFile: "compiler:Languages-mod\Spanish.isl"
Name: "fin"; MessagesFile: "compiler:Languages-mod\Finnish.isl"
Name: "fra"; MessagesFile: "compiler:Languages-mod\French.isl"
Name: "hin"; MessagesFile: "compiler:Languages-mod\Hindi.isl"
Name: "hun"; MessagesFile: "compiler:Languages-mod\Hungarian.isl"
Name: "ind"; MessagesFile: "compiler:Languages-mod\Indonesian.isl"
Name: "ita"; MessagesFile: "compiler:Languages-mod\Italian.isl"
Name: "jpn"; MessagesFile: "compiler:Languages-mod\Japanese.isl"
Name: "kor"; MessagesFile: "compiler:Languages-mod\Korean.isl"
Name: "nld"; MessagesFile: "compiler:Languages-mod\Dutch.isl"
Name: "plk"; MessagesFile: "compiler:Languages-mod\Polish.isl"
Name: "ptb"; MessagesFile: "compiler:Languages-mod\BrazilianPortuguese.isl"
Name: "ptg"; MessagesFile: "compiler:Languages-mod\Portuguese.isl"
Name: "rus"; MessagesFile: "compiler:Languages-mod\Russian.isl"
Name: "sky"; MessagesFile: "compiler:Languages-mod\Slovak.isl"
Name: "sve"; MessagesFile: "compiler:Languages-mod\Swedish.isl"
Name: "trk"; MessagesFile: "compiler:Languages-mod\Turkish.isl"
Name: "vit"; MessagesFile: "compiler:Languages-mod\Vietnamese.isl"
Name: "chs"; MessagesFile: "compiler:Languages-mod\ChineseSimplified.isl"
Name: "cht"; MessagesFile: "compiler:Languages-mod\ChineseTraditional.isl"


[Messages]
enu.BeveledLabel=English (US)
afk.BeveledLabel=Afrikaans
bel.BeveledLabel=Belarusian
deu.BeveledLabel=German
ell.BeveledLabel=Greek
eng.BeveledLabel=English (GB)
esn.BeveledLabel=Spanish
fin.BeveledLabel=Finnish
fra.BeveledLabel=French
hin.BeveledLabel=Hindi
hun.BeveledLabel=Hungarian
ind.BeveledLabel=Indonesian
ita.BeveledLabel=Italian
jpn.BeveledLabel=Japanese
kor.BeveledLabel=Korean
nld.BeveledLabel=Dutch
plk.BeveledLabel=Polish
ptb.BeveledLabel=Portuguese (BR)
ptg.BeveledLabel=Portuguese
rus.BeveledLabel=Russian
sky.BeveledLabel=Slovak
sve.BeveledLabel=Swedish
trk.BeveledLabel=Turkish
vit.BeveledLabel=Vietnamese
chs.BeveledLabel=Chinese (CN)
cht.BeveledLabel=Chinese (TW)


[CustomMessages]
enu.msg_DeleteSettings=Do you also want to delete {#app_name}'s settings and themes?%n%nIf you plan on installing {#app_name} again then you do not have to delete them.
#ifdef sse_required
enu.msg_simd_sse=This build of {#app_name} requires a CPU with SSE extension support.%n%nYour CPU does not have those capabilities.
#endif
#ifdef sse2_required
enu.msg_simd_sse2=This build of {#app_name} requires a CPU with SSE2 extension support.%n%nYour CPU does not have those capabilities.
#endif
enu.tsk_AllUsers=For all users
enu.tsk_CurrentUser=For the current user only
enu.tsk_Other=Other tasks:
enu.tsk_ResetSettings=Reset {#app_name}'s settings and themes
enu.tsk_RemoveDefault=Restore Windows Notepad
enu.tsk_SetDefault=Replace Windows Notepad with {#app_name}
enu.tsk_StartMenuIcon=Create a Start Menu shortcut
enu.tsk_LaunchWelcomePage=Important Release Information!
enu.tsk_RemoveOpenWith=Remove "Open with {#app_name}" from the context menu
enu.tsk_SetOpenWith=Add "Open with {#app_name}" to the context menu
enu.reg_Open_with_NP3=Open with {#app_name}

afk.msg_DeleteSettings=Wil jy ook {#app_name} se instellings en temas uitvee?%n%nAs jy beplan om {#app_name} weer te installeer, hoef jy dit nie uit te vee nie.
#ifdef sse_required
afk.msg_simd_sse=Hierdie bou van {#app_name} vereis 'n SVE met SSE-uitbreidingsteun.%n%nJou SVE het nie daardie vermoëns nie.
#endif
#ifdef sse2_required
afk.msg_simd_sse2=Hierdie bou van {#app_name} vereis 'n SVE met SSE2-uitbreidingsteun.%n%nJou SVE het nie daardie vermoëns nie.
#endif
afk.tsk_AllUsers=Vir alle gebruikers
afk.tsk_CurrentUser=Slegs vir die huidige gebruiker
afk.tsk_Other=Ander take:
afk.tsk_ResetSettings=Stel {#app_name} se instellings en temas terug
afk.tsk_RemoveDefault=Herstel Windows Notepad
afk.tsk_SetDefault=Vervang Windows Notepad met {#app_name}
afk.tsk_StartMenuIcon=Skep 'n Start Menu-kortpad
afk.tsk_LaunchWelcomePage=Belangrike vrystelling-inligting!
afk.tsk_RemoveOpenWith=Verwyder "Maak oop met {#app_name}" uit die kontekskieslys
afk.tsk_SetOpenWith=Voeg "Maak oop met {#app_name}" in die kontekskieslys
afk.reg_Open_with_NP3=Maak oop met {#app_name}

bel.msg_DeleteSettings=Вы хочаце таксама выдаліць налады і тэмы {#app_name}?%n%nКалі вы плануеце ўсталяваць {#app_name} зноў, то вам не трэба іх выдаляць.
#ifdef sse_required
bel.msg_simd_sse=Гэтая зборка {#app_name} патрабуе працэсар з падтрымкай набору каманд SSE.%n%nВаш працэсар не мае такой падтрымкі.
#endif
#ifdef sse2_required
bel.msg_simd_sse2=Гэтая зборка {#app_name} патрабуе працэсар з падтрымкай набору каманд SSE2.%n%nВаш працэсар не мае такой падтрымкі.
#endif
bel.tsk_AllUsers=Для ўсіх карыстальнікаў
bel.tsk_CurrentUser=Для гэтага карыстальніка
bel.tsk_Other=Іншыя задачы:
bel.tsk_ResetSettings=Скінуць налады і тэмы {#app_name}
bel.tsk_RemoveDefault=Аднавіць Нататнік Windows
bel.tsk_SetDefault=Замяніць Нататнік Windows на {#app_name}
bel.tsk_StartMenuIcon=Стварыць ярлык у меню Пуск
bel.tsk_LaunchWelcomePage=Важная інфармацыя пра выпуск!
bel.tsk_RemoveOpenWith=Выдаліць "Адкрыць з дапамогай {#app_name}" з кантэкставага меню
bel.tsk_SetOpenWith=Дадаць "Адкрыць з дапамогай {#app_name}" у кантэкставае меню
bel.reg_Open_with_NP3=Адкрыць з дапамогай {#app_name}

deu.msg_DeleteSettings=Wollen sie die Einstellungen und Themen von {#app_name} löschen?%n%nWenn sie planen {#app_name} erneut zu installieren, dann müssen diese Einstellungen nicht gelöscht werden.
#ifdef sse_required
deu.msg_simd_sse=Diese Version von {#app_name} benötigt eine CPU mit welche die SSE Erweiterung unterstützt.%n%nIhre CPU hat diese Fähigkeiten nicht.
#endif
#ifdef sse2_required
deu.msg_simd_sse2=Diese Version von {#app_name} benötigt eine CPU mit welche die SSE2 Erweiterung unterstützt.%n%nIhre CPU hat diese Fähigkeiten nicht.
#endif
deu.tsk_AllUsers=Für alle Benutzer
deu.tsk_CurrentUser=Für den aktuellen Benutzer alleine
deu.tsk_Other=Andere Aufgaben:
deu.tsk_ResetSettings={#app_name}s Einstellungen und Themen zurück setzen.
deu.tsk_RemoveDefault=Windows Notepad wiederherstellen
deu.tsk_SetDefault=Ersetze Windows Notepad mit {#app_name}
deu.tsk_StartMenuIcon=Erstelle einen Start-Menü Eintrag
deu.tsk_LaunchWelcomePage=Wichtige Release Information!
deu.tsk_RemoveOpenWith=Entferne "Öffnen mit {#app_name}" aus dem Kontextmenü
deu.tsk_SetOpenWith=Füge "Öffnen mit {#app_name}" zum Kontextmenü hinzu.
deu.reg_Open_with_NP3=Öffnen mit {#app_name}

ell.msg_DeleteSettings=Θέλετε επίσης να διαγράψετε τις ρυθμίσεις και τα θέματα του {#app_name};%n%nΕάν σκοπεύετε να εγκαταστήσετε ξανά το {#app_name}, τότε δεν χρειάζεται να τα διαγράψετε.
#ifdef sse_required
ell.msg_simd_sse=Αυτή η έκδοση του {#app_name} απαιτεί CPU με υποστήριξη επέκτασης SSE.%n%nΗ CPU σας δεν έχει αυτές τις δυνατότητες.
#endif
#ifdef sse2_required
ell.msg_simd_sse2=Αυτή η έκδοση του {#app_name} απαιτεί CPU με υποστήριξη επέκτασης SSE2.%n%nΗ CPU σας δεν έχει αυτές τις δυνατότητες.
#endif
ell.tsk_AllUsers=Για όλους τους χρήστες
ell.tsk_CurrentUser=Μόνο για τον τρέχοντα χρήστη
ell.tsk_Other=Άλλες εργασίες:
ell.tsk_ResetSettings=Μηδενισμός ρυθμίσεων και θεμάτων του {#app_name}
ell.tsk_RemoveDefault=Επαναφορά του Σημειωματάριου των Windows
ell.tsk_SetDefault=Αντικατάσταση του Σημειωματάριου των Windows με το {#app_name}
ell.tsk_StartMenuIcon=Δημιουργία συντόμευσης στο μενού Έναρξη
ell.tsk_LaunchWelcomePage=Σημαντικές πληροφορίες έκδοσης!
ell.tsk_RemoveOpenWith=Κατάργηση της επιλογής «Άνοιγμα με {#app_name}» από το μενού περιβάλλοντος
ell.tsk_SetOpenWith=Προσθήκη της επιλογής «Άνοιγμα με {#app_name}» στο μενού περιβάλλοντος
ell.reg_Open_with_NP3=Άνοιγμα με {#app_name}

eng.msg_DeleteSettings=Do you also want to delete {#app_name}'s settings and themes?%n%nIf you plan on installing {#app_name} again then you do not have to delete them.
#ifdef sse_required
eng.msg_simd_sse=This build of {#app_name} requires a CPU with SSE extension support.%n%nYour CPU does not have those capabilities.
#endif
#ifdef sse2_required
eng.msg_simd_sse2=This build of {#app_name} requires a CPU with SSE2 extension support.%n%nYour CPU does not have those capabilities.
#endif
eng.tsk_AllUsers=For all users
eng.tsk_CurrentUser=For the current user only
eng.tsk_Other=Other tasks:
eng.tsk_ResetSettings=Reset {#app_name}'s settings and themes
eng.tsk_RemoveDefault=Restore Windows Notepad
eng.tsk_SetDefault=Replace Windows Notepad with {#app_name}
eng.tsk_StartMenuIcon=Create a Start Menu shortcut
eng.tsk_LaunchWelcomePage=Important Release Information!
eng.tsk_RemoveOpenWith=Remove "Open with {#app_name}" from the context menu
eng.tsk_SetOpenWith=Add "Open with {#app_name}" to the context menu
eng.reg_Open_with_NP3=Open with {#app_name}

esn.msg_DeleteSettings=¿También quieres eliminar la configuración y los temas de {#app_name}?%n%nSi planeas instalar {#app_name} nuevamente, no tienes que eliminarlos.
#ifdef sse_required
esn.msg_simd_sse=Esta compilación de {#app_name} requiere una CPU compatible con la extensión SSE.%n%nSu CPU no tiene esas capacidades.
#endif
#ifdef sse2_required
esn.msg_simd_sse2=Esta compilación de {#app_name} requiere una CPU compatible con la extensión SSE2.%n%nSu CPU no tiene esas capacidades.
#endif
esn.tsk_AllUsers=Para todos los usuarios
esn.tsk_CurrentUser=Sólo para el usuario actual
esn.tsk_Other=Otras tareas:
esn.tsk_ResetSettings=Restablecer la configuración y los temas de {#app_name}
esn.tsk_RemoveDefault=Restaurar el Notepad de Windows
esn.tsk_SetDefault=Reemplazar el Notepad de Windows con {#app_name}
esn.tsk_StartMenuIcon=Crear un acceso directo al menú de inicio
esn.tsk_LaunchWelcomePage=¡Información importante de lanzamiento!
esn.tsk_RemoveOpenWith=Eliminar "Abrir con {#app_name}" del menú contextual
esn.tsk_SetOpenWith=Añadir "Abrir con {#app_name}" al menú contextual
esn.reg_Open_with_NP3=Abrir con {#app_name}

fin.msg_DeleteSettings=Haluatko myös poistaa {#app_name} asetukset ja teemat?%n%nJos aiot asentaa {#app_name} uudelleen, sinun ei tarvitse poistaa niitä.
#ifdef sse_required
fin.msg_simd_sse=Tämä {#app_name} versio vaatii suorittimen, jossa on SSE-laajennustuki.%n%nSuorittimellasi ei ole näitä ominaisuuksia.
#endif
#ifdef sse2_required
fin.msg_simd_sse2=Tämä {#app_name} versio vaatii suorittimen, jossa on SSE2-laajennustuki.%n%nSuorittimellasi ei ole näitä ominaisuuksia.
#endif
fin.tsk_AllUsers=Kaikille käyttäjille
fin.tsk_CurrentUser=Vain nykyiselle käyttäjälle
fin.tsk_Other=Muut tehtävät:
fin.tsk_ResetSettings=Palauta {#app_name} asetukset ja teemat
fin.tsk_RemoveDefault=Palauta Windows Notepad
fin.tsk_SetDefault=Korvaa Windows Notepad {#app_name} sovelluksella
fin.tsk_StartMenuIcon=Luo Käynnistä-valikon pikakuvake
fin.tsk_LaunchWelcomePage=Tärkeää tietoa julkaisusta!
fin.tsk_RemoveOpenWith=Poista "Avaa {#app_name}" kontekstivalikosta.
fin.tsk_SetOpenWith=Lisää kontekstivalikkoon "Avaa {#app_name}".
fin.reg_Open_with_NP3=Avaa {#app_name}

fra.msg_DeleteSettings=Voulez-vous également supprimer tous les réglages et thèmes de {#app_name} ?%n%nSi vous comptez réinstaller {#app_name}, vous pouvez les garder.
#ifdef sse_required
fra.msg_simd_sse=Cette édition de {#app_name} nécessite un CPU supportant l'extension SSE.%n%nVotre CPU ne dispose pas de ces capacités.
#endif
#ifdef sse2_required
fra.msg_simd_sse2=Cette édition de {#app_name} nécessite un CPU supportant l'extension SSE2.%n%nVotre CPU ne dispose pas de ces capacités.
#endif
fra.tsk_AllUsers=Pour tous les utilisateurs
fra.tsk_CurrentUser=Uniquement pour l'utilisateur actuel
fra.tsk_Other=Autres tâches :
fra.tsk_ResetSettings=Rétablir les réglages et thèmes de {#app_name}
fra.tsk_RemoveDefault=Restaurer le Notepad de Windows
fra.tsk_SetDefault=Remplacer le Notepad de Windows par {#app_name}
fra.tsk_StartMenuIcon=Créer un raccourci dans le menu de démarrage
fra.tsk_LaunchWelcomePage=Information importante de publication !
fra.tsk_RemoveOpenWith=Retirer "Ouvrir avec {#app_name}" du menu contextuel
fra.tsk_SetOpenWith=Ajouter "Ouvrir avec {#app_name}" au menu contextuel
fra.reg_Open_with_NP3=Ouvrir avec {#app_name}

hin.msg_DeleteSettings=Do you also want to delete {#app_name}'s settings and themes?%n%nIf you plan on installing {#app_name} again then you do not have to delete them.
#ifdef sse_required
hin.msg_simd_sse=This build of {#app_name} requires a CPU with SSE extension support.%n%nYour CPU does not have those capabilities.
#endif
#ifdef sse2_required
hin.msg_simd_sse2=This build of {#app_name} requires a CPU with SSE2 extension support.%n%nYour CPU does not have those capabilities.
#endif
hin.tsk_AllUsers=For all users
hin.tsk_CurrentUser=For the current user only
hin.tsk_Other=Other tasks:
hin.tsk_ResetSettings=Reset {#app_name}'s settings and themes
hin.tsk_RemoveDefault=Restore Windows Notepad
hin.tsk_SetDefault=Replace Windows Notepad with {#app_name}
hin.tsk_StartMenuIcon=Create a Start Menu shortcut
hin.tsk_LaunchWelcomePage=Important Release Information!
hin.tsk_RemoveOpenWith=Remove "Open with {#app_name}" from the context menu
hin.tsk_SetOpenWith=Add "Open with {#app_name}" to the context menu
hin.reg_Open_with_NP3={#app_name} से खोलें

hun.msg_DeleteSettings=Törölni szeretné {#app_name} beállításait is?%n%nHa újra szeretné telepíteni {#app_name}-t, akkor nem szükséges törölni.
#ifdef sse_required
hun.msg_simd_sse=Ahhoz, hogy {#app_name} ezen változata jól fusson, SSE kiterjesztés támogatással bíró CPU szükséges.%n%nA jelenlegi CPU nem megfelelő.
#endif
#ifdef sse2_required
hun.msg_simd_sse2=Ahhoz, hogy {#app_name} ezen változata jól fusson, SSE2 kiterjesztés támogatással bíró CPU szükséges.%n%nA jelenlegi CPU nem megfelelő.
#endif
hun.tsk_AllUsers=Minden felhasználónak
hun.tsk_CurrentUser=Csak a jelenlegi felhasználónak
hun.tsk_Other=Egyéb műveletek:
hun.tsk_ResetSettings={#app_name} beállításainak és témáinak visszaállítása
hun.tsk_RemoveDefault=Windows Jegyzettömb visszaállítása
hun.tsk_SetDefault=Windows Jegyzettömb lecserélése ezzel: {#app_name}
hun.tsk_StartMenuIcon=Start menü ikon létrehozása
hun.tsk_LaunchWelcomePage=Fontos kiadási információk!
hun.tsk_RemoveOpenWith="Megnyitás ezzel: {#app_name}" bejegyzés eltávolítása a helyi menüből
hun.tsk_SetOpenWith="Megnyitás ezzel: {#app_name}" bejegyzés hozzáadása a helyi menühöz
hun.reg_Open_with_NP3=Megnyitás ezzel: {#app_name}

ind.msg_DeleteSettings=Apakah Anda juga ingin menghapus pengaturan serta tema {#app_name}?%n%nJika berencana untuk memasang {#app_name} kembali, Anda tidak perlu menghapusnya.
#ifdef sse_required
ind.msg_simd_sse=Versi {#app_name} berikut memerlukan sebuah CPU dengan dukungan ekstensi SSE.%n%nTampaknya CPU Anda tidak memiliki kemampuan tersebut.
#endif
#ifdef sse2_required
ind.msg_simd_sse2=Versi {#app_name} berikut memerlukan sebuah CPU dengan dukungan ekstensi SSE2.%n%nTampaknya CPU Anda tidak memiliki kemampuan tersebut.
#endif
ind.tsk_AllUsers=Untuk semua pengguna
ind.tsk_CurrentUser=Untuk pengguna saat ini
ind.tsk_Other=Lainnya:
ind.tsk_ResetSettings=Setel ulang pengaturan dan tema {#app_name}
ind.tsk_RemoveDefault=Pulihkan Notepad Windows
ind.tsk_SetDefault=Ganti Notepad Windows dengan {#app_name}
ind.tsk_StartMenuIcon=Buat sebuah pintasan di Start Menu
ind.tsk_LaunchWelcomePage=Informasi Penting Terkait Rilis Ini!
ind.tsk_RemoveOpenWith=Hapus "Buka dengan {#app_name}" dari menu konteks
ind.tsk_SetOpenWith=Tambahkan "Buka dengan {#app_name}" ke menu konteks
ind.reg_Open_with_NP3=Buka dengan {#app_name}

ita.msg_DeleteSettings=Volete eliminare anche le impostazioni e i temi di {#app_name}? %n%nSe intendete installare nuovamente {#app_name}, non è necessario eliminarli.
#ifdef sse_required
ita.msg_simd_sse=Questa versione di {#app_name} richiede una CPU con supporto per le estensioni SSE.%n%nLa vostra CPU non ha queste capacità.
#endif
#ifdef sse2_required
ita.msg_simd_sse2=Questa versione di {#app_name} richiede una CPU con supporto per le estensioni SSE2.%n%nLa vostra CPU non ha queste capacità.
#endif
ita.tsk_AllUsers=Per tutti gli utenti
ita.tsk_CurrentUser=Solo per l'utente corrente
ita.tsk_Other=Altre attività:
ita.tsk_ResetSettings=Ripristino delle impostazioni e dei temi di {#app_name}
ita.tsk_RemoveDefault=Ripristina il Blocco note di Windows
ita.tsk_SetDefault=Sostituisci il Blocco note di Windows con {#app_name}
ita.tsk_StartMenuIcon=Creare un collegamento al Menu Start
ita.tsk_LaunchWelcomePage=Informazioni importanti sul rilascio!
ita.tsk_RemoveOpenWith=Rimuovere l'opzione "Apri con {#app_name}" dal menu contestuale.
ita.tsk_SetOpenWith=Aggiungete "Apri con {#app_name}" al menu contestuale
ita.reg_Open_with_NP3=Apri con {#app_name}

jpn.msg_DeleteSettings={#app_name} の設定とテーマも削除しますか？%n%n{#app_name} を再度インストールする予定なら削除する必要はありません。
#ifdef sse_required
jpn.msg_simd_sse={#app_name} のこのビルドには、SSE 拡張命令に対応した CPU が必要です。%n%nこの CPU は対応していません。
#endif
#ifdef sse2_required
jpn.msg_simd_sse2={#app_name} のこのビルドには、SSE2 拡張命令に対応した CPU が必要です。%n%nこの CPU は対応していません。
#endif
jpn.tsk_AllUsers=すべてのユーザー
jpn.tsk_CurrentUser=現在のユーザーのみ
jpn.tsk_Other=ほかの設定:
jpn.tsk_ResetSettings={#app_name} の設定とテーマを初期化
jpn.tsk_RemoveDefault=Windows のメモ帳を復元する
jpn.tsk_SetDefault=Windows のメモ帳を {#app_name} に置換する
jpn.tsk_StartMenuIcon=スタートメニューにショートカットを作成
jpn.tsk_LaunchWelcomePage=リリース時の重要な情報！
jpn.tsk_RemoveOpenWith=右クリックメニューから「{#app_name} で開く」を削除
jpn.tsk_SetOpenWith=右クリックメニューに「{#app_name} で開く」を追加
jpn.reg_Open_with_NP3={#app_name} で開く

kor.msg_DeleteSettings={#app_name}의 설정 및 테마도 삭제하시겠습니까?%n%n{#app_name}을 다시 설치할 계획이라면 삭제할 필요가 없습니다.
#ifdef sse_required
kor.msg_simd_sse=이 {#app_name} 빌드에는 SSE 확장을 지원하는 CPU가 필요합니다.%n%nCPU에 이러한 기능이 없습니다.
#endif
#ifdef sse2_required
kor.msg_simd_sse2=이 {#app_name} 빌드에는 SSE2 확장을 지원하는 CPU가 필요합니다.%n%nCPU에 이러한 기능이 없습니다.
#endif
kor.tsk_AllUsers=모든 사용자용
kor.tsk_CurrentUser=현재 사용자 전용
kor.tsk_Other=기타 작업:
kor.tsk_ResetSettings={#app_name}의 설정 및 테마 재설정
kor.tsk_RemoveDefault=Windows 메모장 복원
kor.tsk_SetDefault=Windows 메모장을 {#app_name}으로 바꾸기
kor.tsk_StartMenuIcon=시작 메뉴에 바로가기 만들기
kor.tsk_LaunchWelcomePage=중요한 릴리스 정보!
kor.tsk_RemoveOpenWith=상황에 맞는 메뉴에서 "{#app_name}으로 열기" 제거
kor.tsk_SetOpenWith=상황에 맞는 메뉴 메뉴에 "{#app_name}으로 열기" 추가
kor.reg_Open_with_NP3={#app_name}으로 열기

nld.msg_DeleteSettings=Wilt u ook de instellingen en thema's van {#app_name} verwijderen?%n%nAls u van plan bent {#app_name} opnieuw te installeren, hoeft u deze niet te verwijderen.
#ifdef sse_required
nld.msg_simd_sse=Deze versie van {#app_name} vereist een CPU met ondersteuning voor SSE-extensies.%n%nUw CPU heeft die mogelijkheden niet.
#endif
#ifdef sse2_required
nld.msg_simd_sse2=Deze versie van {#app_name} vereist een CPU met ondersteuning voor SSE2-extensies.%n%nUw CPU heeft die mogelijkheden niet.
#endif
nld.tsk_AllUsers=Voor alle gebruikers
nld.tsk_CurrentUser=Alleen voor de huidige gebruiker
nld.tsk_Other=Overige taken:
nld.tsk_ResetSettings=Instellingen en thema's van {#app_name} opnieuw instellen
nld.tsk_RemoveDefault=Windows Notepad opnieuw instellen
nld.tsk_SetDefault=Windows Notepad vervangen door {#app_name}
nld.tsk_StartMenuIcon=Maak een snelkoppeling naar het startmenu
nld.tsk_LaunchWelcomePage=Belangrijke informatie bij deze uitgave!
nld.tsk_RemoveOpenWith="Openen met {#app_name}" verwijderen van het contextmenu
nld.tsk_SetOpenWith="Openen met {#app_name}" toevoegen van het contextmenu
nld.reg_Open_with_NP3=Openen met {#app_name}

plk.msg_DeleteSettings=Czy chcesz również usunąć ustawienia i motywy {#app_name}? Jeśli zamierzasz zainstalować {#app_name} ponownie, to nie musisz ich usuwać.
#ifdef sse_required
plk.msg_simd_sse=Ta kompilacja {#app_name} wymaga procesora z rozszerzeniem wsparcia SSE. Twój procesor nie posiada takiej zdolności.
#endif
#ifdef sse2_required
plk.msg_simd_sse2=Ta kompilacja {#app_name} wymaga procesora z rozszerzeniem wsparcia SSE2. Twój procesor nie posiada takiej zdolności.
#endif
plk.tsk_AllUsers=Dla wszystkich użytkowników
plk.tsk_CurrentUser=Tylko dla bieżącego użytkownika
plk.tsk_Other=Inne zadania
plk.tsk_ResetSettings=Zresetuj ustawienia i motywy {#app_name}
plk.tsk_RemoveDefault=Przywróć Notepad Windows
plk.tsk_SetDefault=Zamień Notepad Windows na {#app_name}
plk.tsk_StartMenuIcon=Utwórz skrót w Menu Start
plk.tsk_LaunchWelcomePage=Ważne informacje o wydaniu!
plk.tsk_RemoveOpenWith=Usuń "Otwórz z {#app_name}" z menu kontekstowego
plk.tsk_SetOpenWith=Dodaj "Otwórz z {#app_name}" do menu kontekstowego
plk.reg_Open_with_NP3=Otwórz z {#app_name}

ptb.msg_DeleteSettings=Você também deseja excluir as configurações e temas do {#app_name}?%n%nSe você planeja instalar o {#app_name} novamente, então você não precisa excluí-los.
#ifdef sse_required
ptb.msg_simd_sse=Esta versão do {#app_name} requer uma CPU com suporte à extensão SSE.%n%nSua CPU não possui este recurso.
#endif
#ifdef sse2_required
ptb.msg_simd_sse2=Esta versão do {#app_name} requer uma CPU com suporte à extensão SSE2.%n%nSua CPU não possui este recurso.
#endif
ptb.tsk_AllUsers=Para todos os usuários
ptb.tsk_CurrentUser=Somente para o usuário atual
ptb.tsk_Other=Tarefas adicionais:
ptb.tsk_ResetSettings=Restaurar configurações e temas do {#app_name}
ptb.tsk_RemoveDefault=Restaurar Bloco de notas do Windows
ptb.tsk_SetDefault=Substituir Bloco de notas do Windows pelo {#app_name}
ptb.tsk_StartMenuIcon=Criar atalho no Menu Iniciar
ptb.tsk_LaunchWelcomePage=Informações importantes sobre esta versão!
ptb.tsk_RemoveOpenWith=Remover "Abrir com o {#app_name}" do menu de contexto
ptb.tsk_SetOpenWith=Adicionar "Abrir com {#app_name}" ao menu de contexto
ptb.reg_Open_with_NP3=Abrir com {#app_name}

ptg.msg_DeleteSettings=Também pretende eliminar as definições e temas do {#app_name}?%n%nSe planeia instalar novamente o {#app_name} não necessita eliminá-los.
#ifdef sse_required
ptg.msg_simd_sse=Esta versão do {#app_name} requer um CPU com suporte de extensão SSE.%n%nO seu CPU não possui essas capacidades.
#endif
#ifdef sse2_required
ptg.msg_simd_sse2=Esta versão do {#app_name} requer um CPU com suporte de extensão SSE2.%n%nO seu CPU não possui essas capacidades.
#endif
ptg.tsk_AllUsers=Para todos os utilizadores
ptg.tsk_CurrentUser=Só para este utilizador
ptg.tsk_Other=Outras tarefas:
ptg.tsk_ResetSettings=Repor as definições e temas do {#app_name}
ptg.tsk_RemoveDefault=Restaurar o Notepad do Windows
ptg.tsk_SetDefault=Substituir o Notepad de Windows com o {#app_name}
ptg.tsk_StartMenuIcon=Criar um atalho no Menu Iniciar
ptg.tsk_LaunchWelcomePage=Informações Importantes do Lançamento!
ptg.tsk_RemoveOpenWith=Remover "Abrir com o {#app_name}" do menu de contexto
ptg.tsk_SetOpenWith=Adicionar "Abrir com o {#app_name}" ao menu de contexto
ptg.reg_Open_with_NP3=Abrir com o {#app_name}

rus.msg_DeleteSettings=Вы хотите также удалить настройки и темы {#app_name}?%n%nЕсли вы планируете установить {#app_name} снова, то вам не нужно их удалять.
#ifdef sse_required
rus.msg_simd_sse=Эта сборка {#app_name} требует процессор с поддержкой набора команд SSE.%n%nВаш процессор не имеет такой поддержки.
#endif
#ifdef sse2_required
rus.msg_simd_sse2=Эта сборка {#app_name} требует процессор с поддержкой набора команд SSE2.%n%nВаш процессор не имеет такой поддержки.
#endif
rus.tsk_AllUsers=Для всех пользователей
rus.tsk_CurrentUser=Для текущего пользователя
rus.tsk_Other=Другие задачи:
rus.tsk_ResetSettings=Сбросить настройки и темы {#app_name}
rus.tsk_RemoveDefault=Восстановить Блокнот Windows
rus.tsk_SetDefault=Заменить Блокнот Windows на {#app_name}
rus.tsk_StartMenuIcon=Создать значок в меню Пуск
rus.tsk_LaunchWelcomePage=Важная информация о выпуске!
rus.tsk_RemoveOpenWith=Удалить "Открыть с помощью {#app_name}" из контекстного меню
rus.tsk_SetOpenWith=Добавить "Открыть с помощью {#app_name}" в контекстное меню
rus.reg_Open_with_NP3=Открыть с помощью {#app_name}

sky.msg_DeleteSettings=Chcete odstrániť aj nastavenia a témy {#app_name}?%n%nAk plánujete opätovnú inštaláciu {#app_name}, nemusíte ich odstraňovať.
#ifdef sse_required
sky.msg_simd_sse=Táto zostava {#app_name} vyžaduje procesor s podporou rozšírenia SSE.%n%nVáš procesor tieto možnosti nemá.
#endif
#ifdef sse2_required
sky.msg_simd_sse2=Táto zostava {#app_name} vyžaduje procesor s podporou rozšírenia SSE2.%n%nVáš procesor tieto možnosti nemá.
#endif
sky.tsk_AllUsers=Pre všetkých užívateľov
sky.tsk_CurrentUser=Len pre aktuálneho užívateľa
sky.tsk_Other=Ďalšie možnosti:
sky.tsk_ResetSettings=Obnoviť nastavenia a témy {#app_name} na predvolené hodnoty
sky.tsk_RemoveDefault=Obnoviť Poznámkový blok Windows
sky.tsk_SetDefault=Nahradiť Poznámkový blok Windows s {#app_name}
sky.tsk_StartMenuIcon=Vytvoriť odkaz v ponuke Štart
sky.tsk_LaunchWelcomePage=Dôležité informácie o vydaní!
sky.tsk_RemoveOpenWith=Odstrániť z kontextového menu položku "Otvoriť v {#app_name}"
sky.tsk_SetOpenWith=Pridať do kontextového menu položku "Otvoriť v {#app_name}"
sky.reg_Open_with_NP3=Otvoriť v {#app_name}

sve.msg_DeleteSettings=Vill du även ta bort {#app_name} inställningar och teman?%n%nOm du tänker installera {#app_name} igen behöver du inte ta bort inställningarna.
#ifdef sse_required
sve.msg_simd_sse=Den här versionen av {#app_name} kräver processor med SSE stöd.%n%n din processor har inte denna funktionalitet.
#endif
#ifdef sse2_required
sve.msg_simd_sse2=Den här versionen av {#app_name} kräver processor med SSE2 stöd.%n%n din processor har inte denna funktionalitet.
#endif
sve.tsk_AllUsers=För alla användare
sve.tsk_CurrentUser=Endast för aktuell användare
sve.tsk_Other=Andra uppgifter:
sve.tsk_ResetSettings=Återställ inställningarna och tema för {#app_name}
sve.tsk_RemoveDefault=Återställ Windows Anteckningar
sve.tsk_SetDefault=Ersätt Windows Anteckningar med {#app_name}
sve.tsk_StartMenuIcon=Skapa en genväg till Startmeny
sve.tsk_LaunchWelcomePage=Viktig information för denna version!
sve.tsk_RemoveOpenWith=Ta bort "Öppna med {#app_name}" från snabbmenyn
sve.tsk_SetOpenWith=Lägg till "Öppna med {#app_name}" från snabbmenyn
sve.reg_Open_with_NP3=Öppna med {#app_name}

trk.msg_DeleteSettings={#app_name} ayarlarının ve temalarının da silinmesini ister misiniz?%n%n{#app_name} uygulamasını yeniden kurmayı düşünüyorsanız bu verileri silmeniz gerekmez.
#ifdef sse_required
trk.msg_simd_sse=Bu {#app_name} sürümü için SSE eklentileri desteği olan bir işlemci gereklidir.%n%nİşlemcinizde bu özellik bulunmuyor.
#endif
#ifdef sse2_required
trk.msg_simd_sse2=Bu {#app_name} sürümü için SSE2 eklentileri desteği olan bir işlemci gereklidir.%n%nİşlemcinizde bu özellik bulunmuyor.
#endif
trk.tsk_AllUsers=Tüm kullanıcılar için
trk.tsk_CurrentUser=Yalnızca geçerli kullanıcı için
trk.tsk_Other=Diğer işlemler:
trk.tsk_ResetSettings={#app_name} ayarları ve temaları sıfırlansın
trk.tsk_RemoveDefault=Windows Notepad geri yüklensin
trk.tsk_SetDefault=Windows Notepad {#app_name} ile değiştirilsin
trk.tsk_StartMenuIcon=Başlat menüsü kısayolu oluşturulsun
trk.tsk_LaunchWelcomePage=Önemli sürüm bilgileri
trk.tsk_RemoveOpenWith=Sağ tık menüsünden "{#app_name} ile aç" seçeneği kaldırılsın
trk.tsk_SetOpenWith=Sağ tık menüsüne "{#app_name} ile aç" seçeneği eklensin
trk.reg_Open_with_NP3={#app_name} ile aç

vit.msg_DeleteSettings=Do you also want to delete {#app_name}'s settings and themes?%n%nIf you plan on installing {#app_name} again then you do not have to delete them.
#ifdef sse_required
vit.msg_simd_sse=This build of {#app_name} requires a CPU with SSE extension support.%n%nYour CPU does not have those capabilities.
#endif
#ifdef sse2_required
vit.msg_simd_sse2=This build of {#app_name} requires a CPU with SSE2 extension support.%n%nYour CPU does not have those capabilities.
#endif
vit.tsk_AllUsers=For all users
vit.tsk_CurrentUser=For the current user only
vit.tsk_Other=Other tasks:
vit.tsk_ResetSettings=Reset {#app_name}'s settings and themes
vit.tsk_RemoveDefault=Restore Windows Notepad
vit.tsk_SetDefault=Replace Windows Notepad with {#app_name}
vit.tsk_StartMenuIcon=Create a Start Menu shortcut
vit.tsk_LaunchWelcomePage=Important Release Information!
vit.tsk_RemoveOpenWith=Remove "Open with {#app_name}" from the context menu
vit.tsk_SetOpenWith=Add "Open with {#app_name}" to the context menu
vit.reg_Open_with_NP3=Mở bằng {#app_name}

chs.msg_DeleteSettings=是否希望删除 {#app_name} 的设置和主题？%n%n如果您稍后将要重新安装 {#app_name}，您不需要删除以前的配置。
#ifdef sse_required
chs.msg_simd_sse=这个版本的 {#app_name} 需要支持 SSE 扩展指令集的 CPU。%n%n您的 CPU 缺少该支持。
#endif
#ifdef sse2_required
chs.msg_simd_sse2=这个版本的 {#app_name} 需要支持 SSE2 扩展指令集的 CPU。%n%n您的 CPU 缺少该支持。
#endif
chs.tsk_AllUsers=为所有用户
chs.tsk_CurrentUser=仅为当前用户
chs.tsk_Other=其它任务：
chs.tsk_ResetSettings=重置 {#app_name} 的设置和主题
chs.tsk_RemoveDefault=恢复 Windows 记事本
chs.tsk_SetDefault=将 Windows 记事本替换为 {#app_name}
chs.tsk_StartMenuIcon=在开始菜单中创建快捷方式
chs.tsk_LaunchWelcomePage=重要更新信息！
chs.tsk_RemoveOpenWith=从上下文菜单中删除"用 {#app_name} 打开"
chs.tsk_SetOpenWith=在上下文菜单中添加"用 {#app_name} 打开"
chs.reg_Open_with_NP3=用 {#app_name} 打开

cht.msg_DeleteSettings=是否希望刪除 {#app_name} 的設定和主題？%n%n如果您稍後將要重新安裝 {#app_name}，您不需要刪除以前的設定。
#ifdef sse_required
cht.msg_simd_sse=這個版本的 {#app_name} 需要支援 SSE 擴充指令集的 CPU。%n%n您的 CPU 缺少該支援。
#endif
#ifdef sse2_required
cht.msg_simd_sse2=這個版本的 {#app_name} 需要支援 SSE2 擴充指令集的 CPU。%n%n您的 CPU 缺少該支援。
#endif
cht.tsk_AllUsers=為所有使用者
cht.tsk_CurrentUser=僅為當前使用者
cht.tsk_Other=其它任務：
cht.tsk_ResetSettings=重置 {#app_name} 的設定和主題
cht.tsk_RemoveDefault=恢復 Windows 記事本
cht.tsk_SetDefault=將 Windows 記事本替換為 {#app_name}
cht.tsk_StartMenuIcon=於開始功能表中建立快捷方式
cht.tsk_LaunchWelcomePage=重要更新資訊！
cht.tsk_RemoveOpenWith=從上下文選單中刪除"用 {#app_name} 開啟"
cht.tsk_SetOpenWith=在上下文選單中新增"用 {#app_name} 開啟"
cht.reg_Open_with_NP3=用 {#app_name} 開啟


[Tasks]
Name: "reset_settings"; Description: "{cm:tsk_ResetSettings}"; GroupDescription: "{cm:tsk_Other}"; Flags: checkedonce unchecked; Check: SettingsExistCheck()
Name: "set_default"; Description: "{cm:tsk_SetDefault}"; GroupDescription: "{cm:tsk_Other}"; Check: not DefaulNotepadCheck()
Name: "remove_default"; Description: "{cm:tsk_RemoveDefault}"; GroupDescription: "{cm:tsk_Other}"; Flags: checkedonce unchecked; Check: DefaulNotepadCheck()
Name: "set_openwith"; Description: "{cm:tsk_SetOpenWith}"; GroupDescription: "{cm:tsk_Other}"; Check: not OpenWithCheck()
Name: "remove_openwith"; Description: "{cm:tsk_RemoveOpenWith}"; GroupDescription: "{cm:tsk_Other}"; Flags: checkedonce unchecked; Check: OpenWithCheck()

Name: "startup_icon"; Description: "{cm:tsk_StartMenuIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.01
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "desktopicon\user"; Description: "{cm:tsk_CurrentUser}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked exclusive
Name: "desktopicon\common"; Description: "{cm:tsk_AllUsers}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked exclusive


[Files]
Source: "{#bindir}{#RLSdir}\Notepad3.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\minipath.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\grepWinNP3.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\np3encrypt.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\License.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Readme.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\grepWinNP3\grepWinLicense.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "Changes.txt"; DestDir: "{app}\Docs"; Flags: ignoreversion
Source: "Docs\*.txt"; DestDir: "{app}\Docs"; Flags: ignoreversion
Source: "Docs\crypto\*.txt"; DestDir: "{app}\Docs\crypto"; Flags: ignoreversion
Source: "Docs\uthash\*.txt"; DestDir: "{app}\Docs\uthash"; Flags: ignoreversion
Source: "Notepad3.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "minipath.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "Themes\Dark.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3\Themes"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "Themes\Obsidian.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3\Themes"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "Themes\Sombra.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3\Themes"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "{#bindir}{#RLSdir}\lng\mplng.dll"; DestDir: "{app}\lng"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\np3lng.dll"; DestDir: "{app}\lng"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\gwLng\*.lang"; DestDir: "{app}\lng\gwLng"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\af-ZA\mplng.dll.mui"; DestDir: "{app}\lng\af-ZA"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\af-ZA\np3lng.dll.mui"; DestDir: "{app}\lng\af-ZA"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\be-BY\mplng.dll.mui"; DestDir: "{app}\lng\be-BY"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\be-BY\np3lng.dll.mui"; DestDir: "{app}\lng\be-BY"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\de-DE\mplng.dll.mui"; DestDir: "{app}\lng\de-DE"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\de-DE\np3lng.dll.mui"; DestDir: "{app}\lng\de-DE"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\el-GR\mplng.dll.mui"; DestDir: "{app}\lng\el-GR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\el-GR\np3lng.dll.mui"; DestDir: "{app}\lng\el-GR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\en-GB\mplng.dll.mui"; DestDir: "{app}\lng\en-GB"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\en-GB\np3lng.dll.mui"; DestDir: "{app}\lng\en-GB"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\es-ES\mplng.dll.mui"; DestDir: "{app}\lng\es-ES"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\es-ES\np3lng.dll.mui"; DestDir: "{app}\lng\es-ES"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\fi-FI\mplng.dll.mui"; DestDir: "{app}\lng\fi-FI"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\fi-FI\np3lng.dll.mui"; DestDir: "{app}\lng\fi-FI"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\fr-FR\mplng.dll.mui"; DestDir: "{app}\lng\fr-FR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\fr-FR\np3lng.dll.mui"; DestDir: "{app}\lng\fr-FR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\hi-IN\mplng.dll.mui"; DestDir: "{app}\lng\hi-IN"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\hi-IN\np3lng.dll.mui"; DestDir: "{app}\lng\hi-IN"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\hu-HU\mplng.dll.mui"; DestDir: "{app}\lng\hu-HU"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\hu-HU\np3lng.dll.mui"; DestDir: "{app}\lng\hu-HU"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\id-ID\mplng.dll.mui"; DestDir: "{app}\lng\id-ID"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\id-ID\np3lng.dll.mui"; DestDir: "{app}\lng\id-ID"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\it-IT\mplng.dll.mui"; DestDir: "{app}\lng\it-IT"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\it-IT\np3lng.dll.mui"; DestDir: "{app}\lng\it-IT"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\ja-JP\mplng.dll.mui"; DestDir: "{app}\lng\ja-JP"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\ja-JP\np3lng.dll.mui"; DestDir: "{app}\lng\ja-JP"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\ko-KR\mplng.dll.mui"; DestDir: "{app}\lng\ko-KR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\ko-KR\np3lng.dll.mui"; DestDir: "{app}\lng\ko-KR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\nl-NL\mplng.dll.mui"; DestDir: "{app}\lng\nl-NL"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\nl-NL\np3lng.dll.mui"; DestDir: "{app}\lng\nl-NL"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\pl-PL\mplng.dll.mui"; DestDir: "{app}\lng\pl-PL"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\pl-PL\np3lng.dll.mui"; DestDir: "{app}\lng\pl-PL"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\pt-BR\mplng.dll.mui"; DestDir: "{app}\lng\pt-BR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\pt-BR\np3lng.dll.mui"; DestDir: "{app}\lng\pt-BR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\pt-PT\mplng.dll.mui"; DestDir: "{app}\lng\pt-PT"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\pt-PT\np3lng.dll.mui"; DestDir: "{app}\lng\pt-PT"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\ru-RU\mplng.dll.mui"; DestDir: "{app}\lng\ru-RU"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\ru-RU\np3lng.dll.mui"; DestDir: "{app}\lng\ru-RU"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\sk-SK\mplng.dll.mui"; DestDir: "{app}\lng\sk-SK"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\sk-SK\np3lng.dll.mui"; DestDir: "{app}\lng\sk-SK"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\sv-SE\mplng.dll.mui"; DestDir: "{app}\lng\sv-SE"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\sv-SE\np3lng.dll.mui"; DestDir: "{app}\lng\sv-SE"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\tr-TR\mplng.dll.mui"; DestDir: "{app}\lng\tr-TR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\tr-TR\np3lng.dll.mui"; DestDir: "{app}\lng\tr-TR"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\vi-VN\mplng.dll.mui"; DestDir: "{app}\lng\vi-VN"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\vi-VN\np3lng.dll.mui"; DestDir: "{app}\lng\vi-VN"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\zh-CN\mplng.dll.mui"; DestDir: "{app}\lng\zh-CN"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\zh-CN\np3lng.dll.mui"; DestDir: "{app}\lng\zh-CN"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\zh-TW\mplng.dll.mui"; DestDir: "{app}\lng\zh-TW"; Flags: ignoreversion
Source: "{#bindir}{#RLSdir}\lng\zh-TW\np3lng.dll.mui"; DestDir: "{app}\lng\zh-TW"; Flags: ignoreversion


[Dirs]
Name: "{userappdata}\Rizonesoft\Notepad3\Favorites"
Name: "{userappdata}\Rizonesoft\Notepad3\Themes"


[Icons]
Name: "{commondesktop}\{#app_name}"; Filename: "{app}\Notepad3.exe"; WorkingDir: "{app}"; AppUserModelID: "{#app_publisher}.{#app_name}"; IconFilename: "{app}\Notepad3.exe"; Comment: "{#app_name} {#app_version}"; Tasks: desktopicon\common
Name: "{userdesktop}\{#app_name}"; Filename: "{app}\Notepad3.exe"; WorkingDir: "{app}"; AppUserModelID: "{#app_publisher}.{#app_name}"; IconFilename: "{app}\Notepad3.exe"; IconIndex: 0; Comment: "{#app_name} {#app_version}"; Tasks: desktopicon\user
Name: "{commonprograms}\{#app_name}"; Filename: "{app}\Notepad3.exe"; WorkingDir: "{app}"; AppUserModelID: "{#app_publisher}.{#app_name}"; IconFilename: "{app}\Notepad3.exe"; IconIndex: 0; Comment: "{#app_name} {#app_version}"; Tasks: startup_icon
Name: "{#quick_launch}\{#app_name}"; Filename: "{app}\Notepad3.exe"; WorkingDir: "{app}"; IconFilename: "{app}\Notepad3.exe"; IconIndex: 0; Comment: "{#app_name} {#app_version}"; Tasks: quicklaunchicon


[INI]
Filename: "{app}\Notepad3.ini"; Section: "Notepad3"; Key: "Notepad3.ini"; String: "%APPDATA%\Rizonesoft\Notepad3\Notepad3.ini"
Filename: "{app}\minipath.ini"; Section: "minipath"; Key: "minipath.ini"; String: "%APPDATA%\Rizonesoft\Notepad3\minipath.ini"
Filename: "{userappdata}\Rizonesoft\Notepad3\Notepad3.ini"; Section: "Settings"; Key: "Favorites"; String: "%APPDATA%\Rizonesoft\Notepad3\Favorites\"


[Registry]
Root: "HKLM"; Subkey: "SYSTEM\CurrentControlSet\Control\FileSystem"; ValueType: dword; ValueName: "LongPathsEnabled"; ValueData: "1"
;The following "Keys/Values" are required to allow a "MS Notepad Replacement" in Windows 11.
Root: "HKLM"; Subkey: "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\notepad.exe"; ValueType: dword; ValueName: "UseFilter"; ValueData: "1"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\notepad.exe"; ValueType: string; ValueData: "C:\Windows\System32\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\notepad.exe"; ValueType: string; ValueName: "Path"; ValueData: "C:\Windows\System32"
Root: "HKCR"; Subkey: ".inf"; ValueType: string; ValueData: "inffile"
Root: "HKCR"; Subkey: ".ini"; ValueType: string; ValueData: "inifile"
Root: "HKCR"; Subkey: ".ps1"; ValueType: string; ValueData: "Microsoft.PowerShellScript.1"
Root: "HKCR"; Subkey: ".psd1"; ValueType: string; ValueData: "Microsoft.PowerShellData.1"
Root: "HKCR"; Subkey: ".psm1"; ValueType: string; ValueData: "Microsoft.PowerShellModule.1"
Root: "HKCR"; Subkey: ".log"; ValueType: string; ValueData: "txtfile"
Root: "HKCR"; Subkey: ".scp"; ValueType: string; ValueData: "txtfile"
Root: "HKCR"; Subkey: ".txt"; ValueType: string; ValueData: "txtfile"
Root: "HKCR"; Subkey: ".wtx"; ValueType: string; ValueData: "txtfile"
Root: "HKCR"; Subkey: "inffile\DefaultIcon"; ValueType: expandsz; ValueData: "%SystemRoot%\System32\imageres.dll,-69"
Root: "HKCR"; Subkey: "inffile\shell\open\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe %1"
Root: "HKCR"; Subkey: "inffile\shell\print\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe /p %1"
Root: "HKCR"; Subkey: "inifile\DefaultIcon"; ValueType: expandsz; ValueData: "%SystemRoot%\System32\imageres.dll,-69"
Root: "HKCR"; Subkey: "inifile\shell\open\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe %1"
Root: "HKCR"; Subkey: "inifile\shell\print\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe /p %1"
Root: "HKCR"; Subkey: "Microsoft.PowerShellScript.1\DefaultIcon"; ValueType: expandsz; ValueData: "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell_ise.exe,1"
Root: "HKCR"; Subkey: "Microsoft.PowerShellScript.1\Shell"; ValueType: string; ValueData: "Open"
Root: "HKCR"; Subkey: "Microsoft.PowerShellScript.1\shell\Open\Command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe %1"
Root: "HKCR"; Subkey: "Microsoft.PowerShellData.1\DefaultIcon"; ValueType: expandsz; ValueData: "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell_ise.exe,1"
Root: "HKCR"; Subkey: "Microsoft.PowerShellData.1\Shell"; ValueType: string; ValueData: "Open"
Root: "HKCR"; Subkey: "Microsoft.PowerShellData.1\shell\Open\Command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe %1"
Root: "HKCR"; Subkey: "Microsoft.PowerShellModule.1\DefaultIcon"; ValueType: expandsz; ValueData: "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell_ise.exe,1"
Root: "HKCR"; Subkey: "Microsoft.PowerShellModule.1\Shell"; ValueType: string; ValueData: "Open"
Root: "HKCR"; Subkey: "Microsoft.PowerShellModule.1\shell\Open\Command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe %1"
Root: "HKCR"; Subkey: "txtfile\DefaultIcon"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\imageres.dll,-102"
Root: "HKCR"; Subkey: "txtfile\shell\open\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe %1"
Root: "HKCR"; Subkey: "txtfile\shell\print\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe /p %1"
Root: "HKCR"; Subkey: "txtfile\shell\printto"; ValueType: string; ValueName: "NeverDefault"
Root: "HKCR"; Subkey: "txtfile\shell\printto\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe /pt ""%1"" ""%2"" ""%3"" ""%4"
Root: "HKCR"; Subkey: "txtfilelegacy\DefaultIcon"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\imageres.dll,-102"
Root: "HKCR"; Subkey: "txtfilelegacy\shell\open\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe %1"
Root: "HKCR"; Subkey: "txtfilelegacy\shell\print\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe /p %1"
Root: "HKCR"; Subkey: "txtfilelegacy\shell\printto"; ValueType: string; ValueName: "NeverDefault"
Root: "HKCR"; Subkey: "txtfilelegacy\shell\printto\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe /pt ""%1"" ""%2"" ""%3"" ""%4"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "inffile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "inifile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "Microsoft.PowerShellScript.1"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "Microsoft.PowerShellData.1"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "Microsoft.PowerShellModule.1"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "txtfile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "txtfile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "txtfile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "Notepad3.exe"; Flags: uninsdeletevalue
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "ab"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "txtfile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.inf"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.ini"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.ps1"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.psd1"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.psm1"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.log"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.scp"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.txt"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad.exe_.wtx"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.inf"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.ini"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.ps1"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.psd1"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.psm1"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.log"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.scp"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.txt"; ValueData: "0"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\ApplicationAssociationToasts"; ValueType: dword; ValueName: "Applications\Notepad3.exe_.wtx"; ValueData: "0"


[Run]
Filename: "{app}\Notepad3.exe"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent unchecked; Description: "{cm:LaunchProgram,{#app_name}}"
Filename: "https://www.rizonesoft.com/downloads/notepad3/update/"; Flags: nowait postinstall shellexec skipifsilent unchecked; Description: "{cm:tsk_LaunchWelcomePage}"


[InstallDelete]
Type: files; Name: "{userdesktop}\{#app_name}.lnk"; Check: not WizardIsTaskSelected('desktopicon\user') and IsUpgrade()
Type: files; Name: "{commondesktop}\{#app_name}.lnk"; Check: not WizardIsTaskSelected('desktopicon\common') and IsUpgrade()
Type: files; Name: "{userstartmenu}\{#app_name}.lnk"; Check: not WizardIsTaskSelected('startup_icon') and IsUpgrade()
Type: files; Name: "{#quick_launch}\{#app_name}.lnk"; OnlyBelowVersion: 6.01; Check: not WizardIsTaskSelected('quicklaunchicon') and IsUpgrade()
Type: files; Name: "{app}\Notepad3.ini"
Type: files; Name: "{app}\Readme.txt"
Type: files; Name: "{app}\minipath.ini"
Type: files; Name: "{app}\grepWinNP3.ini"


[UninstallDelete]
Type: files; Name: "{app}\Notepad3.ini"
Type: files; Name: "{app}\minipath.ini"
Type: files; Name: "{app}\grepWinNP3.ini"
Type: dirifempty; Name: "{app}"


[Code]
const
  IFEO = 'SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\notepad.exe';
  APPH = 'SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Notepad3.exe';

#if defined sse_required || defined sse2_required
function IsProcessorFeaturePresent(Feature: DWORD): BOOL;
  external 'IsProcessorFeaturePresent@kernel32.dll stdcall';

const
  PF_XMMI_INSTRUCTIONS_AVAILABLE = 6;// The SSE instruction set is available.
  PF_XMMI64_INSTRUCTIONS_AVAILABLE = 10;// The SSE2 instruction set is available.
#endif

function InitializeSetup: Boolean;
  begin
    Result := True;
    #ifdef sse_required
    // Check for Processor SSE support.
    if Result then
      if not IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) then
        Result := SuppressibleMsgBox(CustomMessage('msg_simd_sse'), mbCriticalError, MB_OK, IDOK) = IDABORT;
    #endif
    #ifdef sse2_required
    // Check for Processor SSE2 support.
    if Result then
      if not IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) then
        Result := SuppressibleMsgBox(CustomMessage('msg_simd_sse2'), mbCriticalError, MB_OK, IDOK) = IDABORT;
    #endif
end;

// Check if Notepad3 has replaced Windows Notepad
function DefaulNotepadCheck(): Boolean;
  var
    sDebugger: String;
  begin
    Result := RegQueryStringValue(HKLM, IFEO, 'Debugger', sDebugger) and
        (sDebugger = ExpandConstant('"{app}\Notepad3.exe" /z'));
    if Result then
      Log('Custom Code: {#app_name} is set as the default notepad')
    else
      Log('Custom Code: {#app_name} is NOT set as the default notepad');
end;

Var
  reg_Open_with_NP3: String;

// Check if "Open with Notepad3" is installed.
function OpenWithCheck(): Boolean;
  var
    sOpenWith: String;
  begin
    Result := RegQueryStringValue(HKEY_CLASSES_ROOT, '*\shell\' + reg_Open_with_NP3, 'Icon', sOpenWith) and
        (sOpenWith = ExpandConstant('{app}\Notepad3.exe,0'));
    if Result then
      Log('Custom Code: {#app_name} '+reg_Open_with_NP3+' is set.')
    else
      Log('Custom Code: {#app_name} '+reg_Open_with_NP3+' is not set.');
end;

function IsOldBuildInstalled(sInfFile: String): Boolean;
  begin
    Result := RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Notepad2') and
        FileExists(ExpandConstant('{commonpf}\Notepad2\' + sInfFile));
end;

Var
  Upgrade: Boolean;

function IsUpgrade(): Boolean;
  begin
    Result := Upgrade;
end;

// Check if Notepad3's settings exist
function SettingsExistCheck(): Boolean;
  begin
    Result := FileExists(ExpandConstant('{userappdata}\Rizonesoft\Notepad3\Notepad3.ini'));
    if Result Then
      Log('Custom Code: Settings are present')
    else
      Log('Custom Code: Settings are NOT present');
end;

function UninstallOldVersion(sInfFile: String): Integer;
  begin
    // Return Values:
    // -1 - exec(...) failed!
    // return value of the executing command (0 - AllOK, 1 - SysErrorMessage(1), etc.)

    // default return value
    Result := -1;
    // TODO: use RegQueryStringValue
    Exec('rundll32.exe', ExpandConstant('advpack.dll,LaunchINFSectionEx ' + '"{commonpf}\Notepad2\' + sInfFile +'",DefaultUninstall,,8,N'), '', SW_HIDE, ewWaitUntilTerminated, Result);
end;

function ShouldSkipPage(PageID: Integer): Boolean;
  begin
    // Skip the license page if IsUpgrade()
    if PageID = wpLicense then
      if IsUpgrade() then
      begin
        Result := True;
        WizardForm.LicenseAcceptedRadio.Checked := Result;
      end;
end;

procedure AddReg();
  Var
    APP: String;
  begin
    APP := ExpandConstant('{app}');
    RegWriteStringValue(HKCR, 'Applications\notepad3.exe', 'AppUserModelID', 'Rizonesoft.Notepad3');
    RegWriteStringValue(HKCR, 'Applications\notepad3.exe\shell\open\command', '', '"'+APP+'\Notepad3.exe" "%1"');
    RegWriteStringValue(HKCR, '*\OpenWithList\notepad3.exe', '', '');
    RegWriteStringValue(HKLM, APPH, '', APP+'\Notepad3.exe');
    RegWriteStringValue(HKLM, APPH, 'Path', APP);
end;

procedure CleanUpSettings();
  Var
    userappdata: String;
  begin
    userappdata := ExpandConstant('{userappdata}');
    DeleteFile(userappdata + '\Rizonesoft\Notepad3\Notepad3.ini');
    DeleteFile(userappdata + '\Rizonesoft\Notepad3\minipath.ini');
    DeleteFile(userappdata + '\Rizonesoft\Notepad3\grepWinNP3.ini');
    DeleteFile(userappdata + '\Rizonesoft\Notepad3\Themes\Dark.ini');
    DeleteFile(userappdata + '\Rizonesoft\Notepad3\Themes\Obsidian.ini');
    DeleteFile(userappdata + '\Rizonesoft\Notepad3\Themes\Sombra.ini');
end;

Var
  PreviousDataOf_Open_with_NP3: String;

procedure RemoveReg();
  begin
    if Length( PreviousDataOf_Open_with_NP3 ) > 0 then
      if RegKeyExists(HKCR, '*\shell\' + PreviousDataOf_Open_with_NP3) then
        RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\' + PreviousDataOf_Open_with_NP3);
    if RegKeyExists(HKCR, 'Applications\notepad3.exe') then
      RegDeleteKeyIncludingSubkeys(HKCR, 'Applications\notepad3.exe');
    if RegKeyExists(HKCR, '*\OpenWithList\notepad3.exe') then
      RegDeleteKeyIncludingSubkeys(HKCR, '*\OpenWithList\notepad3.exe');
    if RegKeyExists(HKCR, '*\shell\Open with Notepad3') then
      RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\Open with Notepad3');
    if Length( reg_Open_with_NP3 ) > 0 then
      if RegKeyExists(HKCR, '*\shell\' + reg_Open_with_NP3) then
        RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\' + reg_Open_with_NP3);
    if RegKeyExists(HKLM, APPH) then
      RegDeleteKeyIncludingSubkeys(HKLM, APPH);
end;

procedure CurPageChanged(CurPageID: Integer);
  begin
    if CurPageID = wpSelectTasks then
      WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall)
    else
    if CurPageID = wpFinished then
      WizardForm.NextButton.Caption := SetupMessage(msgButtonFinish);
end;

procedure CurStepChanged(CurStep: TSetupStep);
  Var
    app: String;
    TMP, userappdata: String;
    FindRec: TFindRec;
  begin
    if (CurStep = ssInstall) or (CurStep = ssPostInstall) then
    begin
      APP := ExpandConstant('{app}');

      if CurStep = ssInstall then
      begin
        PreviousDataOf_Open_with_NP3 := GetPreviousData('reg_Open_with_NP3', '');

        if IsOldBuildInstalled('Uninstall.inf') or IsOldBuildInstalled('Notepad2.inf') then
        begin
          if IsOldBuildInstalled('Uninstall.inf') then
          begin
            Log('Custom Code: The old build is installed, will try to uninstall it...');
            if UninstallOldVersion('Uninstall.inf') = 0 then
              Log('Custom Code: The old build was successfully uninstalled')
            else
              Log('Custom Code: Something went wrong when uninstalling the old build');
          end;

          if IsOldBuildInstalled('Notepad2.inf') then
          begin
            Log('Custom Code: The official Notepad2 build is installed, will try to uninstall it...');
            if UninstallOldVersion('Notepad2.inf') = 0 then
              Log('Custom Code: The official Notepad2 build was successfully uninstalled')
            else
              Log('Custom Code: Something went wrong when uninstalling the official Notepad2 build');
          end;

          // This is the case where the old build is installed; the DefaulNotepadCheck() returns true
          // and the set_default task isn't selected
          if not WizardIsTaskSelected('remove_default') then
          begin
            RegWriteStringValue(HKLM, IFEO, 'Debugger', '"'+app+'\Notepad3.exe" /z');
            RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 0);
          end;
        end;
      end;

      if CurStep = ssPostInstall then
      begin
        if WizardIsTaskSelected('reset_settings') then
        begin
          CleanUpSettings();
          ExtractTemporaryFiles( '{userappdata}\*' );
          userappdata := ExpandConstant('{userappdata}')+'\';
          TMP := ExpandConstant('{tmp}')+'\';
          if FindFirst(TMP+'{userappdata}\Rizonesoft\Notepad3\*.*', FindRec) then
          begin
            if not DirExists(userappdata+'Rizonesoft\Notepad3') then
              CreateDir(userappdata+'Rizonesoft\Notepad3');
            if DirExists(userappdata+'Rizonesoft\Notepad3') then
              With FindRec do
                repeat
                  RenameFile( TMP+'{userappdata}\Rizonesoft\Notepad3\'+Name, userappdata+'Rizonesoft\Notepad3\'+Name );
                until not FindNext(FindRec);
            FindClose( FindRec );
          end;

          SetIniString('Settings', 'Favorites', '%APPDATA%\Rizonesoft\Notepad3\Favorites\', userappdata+'Rizonesoft\Notepad3\Notepad3.ini');

          if FindFirst(TMP+'{userappdata}\Rizonesoft\Notepad3\Themes\*.*', FindRec) then
          begin
            if not DirExists(userappdata+'Rizonesoft\Notepad3\Themes') then
              CreateDir(userappdata+'Rizonesoft\Notepad3\Themes');
            if DirExists(userappdata+'Rizonesoft\Notepad3\Themes') then
              With FindRec do
                repeat
                  RenameFile( TMP+'{userappdata}\Rizonesoft\Notepad3\Themes\'+Name, userappdata+'Rizonesoft\Notepad3\Themes\'+Name );
                until not FindNext(FindRec);
            FindClose( FindRec );
          end;
        end;

        if WizardIsTaskSelected('set_default') then begin
          RegWriteStringValue(HKLM, IFEO, 'Debugger', '"'+app+'\Notepad3.exe" /z');
          RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 0);
        end
        else
        if WizardIsTaskSelected('remove_default') then
        begin
          RegDeleteValue(HKLM, IFEO, 'Debugger');
          RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 1);
        end
        else
        begin
          if RegValueExists (HKLM, IFEO, 'Debugger') then
            RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 0)
          else
            RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 1);
        end;

        if WizardIsTaskSelected('set_openwith') then
        begin
          if Length( PreviousDataOf_Open_with_NP3 ) > 0 then
            if RegKeyExists(HKCR, '*\shell\' + PreviousDataOf_Open_with_NP3) then
              RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\' + PreviousDataOf_Open_with_NP3);
          if RegKeyExists(HKCR, '*\shell\Open with Notepad3') then
            RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\Open with Notepad3');
          if Length( reg_Open_with_NP3 ) > 0 then
            if RegKeyExists(HKCR, '*\shell\' + reg_Open_with_NP3) then
              RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\' + reg_Open_with_NP3);
          RegWriteStringValue(HKCR, '*\shell\' + reg_Open_with_NP3, 'Icon', app+'\Notepad3.exe,0');
          RegWriteStringValue(HKCR, '*\shell\' + reg_Open_with_NP3 + '\command', '', '"'+app+'\Notepad3.exe" "%1"');
        end
        else
        if WizardIsTaskSelected('remove_openwith') then
        begin
          if Length( reg_Open_with_NP3 ) > 0 then
            if RegKeyExists(HKCR, '*\shell\' + reg_Open_with_NP3) then
              RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\' + reg_Open_with_NP3);
        end
        else
        begin
          WizardSelectTasks('remove_openwith');
          if WizardIsTaskSelected('remove_openwith') then
            WizardSelectTasks('!remove_openwith')
          else
            if Length( PreviousDataOf_Open_with_NP3 ) > 0 then
              if RegKeyExists(HKCR, '*\shell\' + PreviousDataOf_Open_with_NP3) then
                RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\' + PreviousDataOf_Open_with_NP3);
        end;

      // Always add Notepad3's AppUserModelID and the rest registry values
        AddReg();
      end;
    end;
end;

Var
  SettingsCleanUp: Boolean;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
  begin
    // When uninstalling, ask the user to delete Notepad3's settings and themes
    if CurUninstallStep = usUninstall then
      if SettingsExistCheck() then
        SettingsCleanUp := SuppressibleMsgBox(CustomMessage('msg_DeleteSettings'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2, IDNO) = IDYES;

    if CurUninstallStep = usPostUninstall then
    begin
      if SettingsCleanUp then
        CleanUpSettings();

      if DefaulNotepadCheck() then
      begin
        RegDeleteValue(HKLM, IFEO, 'Debugger');
        RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 1);
      end;

      RemoveReg();
    end;
end;

procedure InitializeWizard();
  begin
    reg_Open_with_NP3 := CustomMessage('reg_Open_with_NP3');

    With WizardForm do
    begin
      Upgrade := FileExists( AddBackslash(PrevAppDir) + '{#app_name}.exe' );
      SelectTasksLabel.Hide;
      With TasksList do
      begin
        Top := 0;
        Height := SelectTasksPage.ClientHeight;
      end;
    end;
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
  begin
    SetPreviousData(PreviousDataKey, 'reg_Open_with_NP3', reg_Open_with_NP3);
end;

procedure InitializeUninstallProgressForm();
  begin
    reg_Open_with_NP3 := CustomMessage('reg_Open_with_NP3');
end;

// #expr SaveToFile( AddBackSlash(SourcePath) +  SetupSetting("OutputBaseFilename") + ".iss")
