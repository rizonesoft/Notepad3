;* Notepad3 - Installer script
;*
;* (c) Rizonesoft 2008-2021

; Requirements:
; Inno Setup: https://www.jrsoftware.org/isdl.php

; Preprocessor related stuff
#if VER < EncodeVer(6,0,5)
  #error Update your Inno Setup version (6.0.5 or newer)
#endif

#define bindir "..\Bin"

#ifnexist bindir + "\Release_x86_v142\Notepad3.exe"
  #error Compile Notepad3 x86 first
#endif

#ifnexist bindir + "\Release_x86_v142\minipath.exe"
  #error Compile MiniPath x86 first
#endif

#define app_name "Notepad3"
#define app_publisher "Rizonesoft"
#define app_version GetVersionNumbersString(bindir + "\Release_x86_v142\Notepad3.exe")
#define app_copyright "Copyright © 2008-2021 Rizonesoft"
#define quick_launch "{userappdata}\Microsoft\Internet Explorer\Quick Launch"

[Setup]
AppId={#app_name}
AppName={#app_name}
AppVersion={#app_version}
AppVerName={#app_name} {#app_version}
AppPublisher={#app_publisher}
AppPublisherURL=https://rizonesoft.com
AppSupportURL=https://rizonesoft.com
AppUpdatesURL=https://rizonesoft.com
AppContact=https://rizonesoft.com
AppCopyright={#app_copyright}
VersionInfoVersion={#app_version}
UninstallDisplayIcon={app}\Notepad3.exe
UninstallDisplayName={#app_name} {#app_version}
DefaultDirName={commonpf}\Notepad3
LicenseFile="..\License.txt"
OutputDir=.\Packages
OutputBaseFilename={#app_name}_{#app_version}_x86_Setup
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
DisableWelcomePage=yes
AllowCancelDuringInstall=yes
UsedUserAreasWarning=no
MinVersion=0,6.1sp1
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=
#ifexist "..\signinfo_notepad3.txt"
SignTool=MySignTool
#endif
CloseApplications=true
SetupMutex='{#app_name}' + '_setup_mutex'

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[Messages]
SetupAppTitle=Setup - {#app_name}
SetupWindowTitle=Setup - {#app_name}

[CustomMessages]
en.msg_AppIsRunning=Setup has detected that {#app_name} is currently running.%n%nPlease close all instances of it now, then click OK to continue, or Cancel to exit.
en.msg_AppIsRunningUninstall=Uninstall has detected that {#app_name} is currently running.%n%nPlease close all instances of it now, then click OK to continue, or Cancel to exit.
en.msg_DeleteSettings=Do you also want to delete {#app_name}'s settings?%n%nIf you plan on installing {#app_name} again then you do not have to delete them.
#if defined(sse_required)
en.msg_simd_sse=This build of {#app_name} requires a CPU with SSE extension support.%n%nYour CPU does not have those capabilities.
#elif defined(sse2_required)
en.msg_simd_sse2=This build of {#app_name} requires a CPU with SSE2 extension support.%n%nYour CPU does not have those capabilities.
#endif
en.tsk_AllUsers=For all users
en.tsk_CurrentUser=For the current user only
en.tsk_Other=Other tasks:
en.tsk_ResetSettings=Reset {#app_name}'s settings
en.tsk_RemoveDefault=Restore Windows Notepad
en.tsk_SetDefault=Replace Windows Notepad with {#app_name}
en.tsk_StartMenuIcon=Create a Start Menu shortcut
en.tsk_LaunchWelcomePage=Important Release Information!
en.tsk_RemoveOpenWith=Remove "Open with {#app_name}" from the context menu
en.tsk_SetOpenWith=Add "Open with {#app_name}" to the context menu

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "desktopicon\user"; Description: "{cm:tsk_CurrentUser}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked exclusive
Name: "desktopicon\common"; Description: "{cm:tsk_AllUsers}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked exclusive
Name: "startup_icon"; Description: "{cm:tsk_StartMenuIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.01
Name: "reset_settings"; Description: "{cm:tsk_ResetSettings}"; GroupDescription: "{cm:tsk_Other}"; Flags: checkedonce unchecked; Check: SettingsExistCheck()
Name: "set_default"; Description: "{cm:tsk_SetDefault}"; GroupDescription: "{cm:tsk_Other}"; Check: not DefaulNotepadCheck()
Name: "remove_default"; Description: "{cm:tsk_RemoveDefault}"; GroupDescription: "{cm:tsk_Other}"; Flags: checkedonce unchecked; Check: DefaulNotepadCheck()
Name: "set_openwith"; Description: "{cm:tsk_SetOpenWith}"; GroupDescription: "{cm:tsk_Other}"; Check: not OpenWithCheck()
Name: "remove_openwith"; Description: "{cm:tsk_RemoveOpenWith}"; GroupDescription: "{cm:tsk_Other}"; Flags: checkedonce unchecked; Check: OpenWithCheck()

[Files]
Source: "{#bindir}\Release_x86_v142\Notepad3.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\minipath.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\grepWinNP3.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\License.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Readme.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\grepWinNP3\grepWinLicense.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "Notepad3.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "minipath.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "Themes\Dark.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3\Themes"; Flags: ignoreversion uninsneveruninstall
Source: "Themes\Obsidian.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3\Themes"; Flags: ignoreversion uninsneveruninstall
Source: "Themes\Sombra.ini"; DestDir: "{userappdata}\Rizonesoft\Notepad3\Themes"; Flags: ignoreversion uninsneveruninstall
Source: "{#bindir}\Release_x86_v142\lng\mplng.dll"; DestDir: "{app}\lng"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\np3lng.dll"; DestDir: "{app}\lng"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\gwLng\*.lang"; DestDir: "{app}\lng\gwLng"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\af-ZA\mplng.dll.mui"; DestDir: "{app}\lng\af-ZA"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\af-ZA\np3lng.dll.mui"; DestDir: "{app}\lng\af-ZA"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\be-BY\mplng.dll.mui"; DestDir: "{app}\lng\be-BY"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\be-BY\np3lng.dll.mui"; DestDir: "{app}\lng\be-BY"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\de-DE\mplng.dll.mui"; DestDir: "{app}\lng\de-DE"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\de-DE\np3lng.dll.mui"; DestDir: "{app}\lng\de-DE"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\el-GR\mplng.dll.mui"; DestDir: "{app}\lng\el-GR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\el-GR\np3lng.dll.mui"; DestDir: "{app}\lng\el-GR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\en-GB\mplng.dll.mui"; DestDir: "{app}\lng\en-GB"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\en-GB\np3lng.dll.mui"; DestDir: "{app}\lng\en-GB"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\es-419\mplng.dll.mui"; DestDir: "{app}\lng\es-419"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\es-419\np3lng.dll.mui"; DestDir: "{app}\lng\es-419"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\es-ES\mplng.dll.mui"; DestDir: "{app}\lng\es-ES"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\es-ES\np3lng.dll.mui"; DestDir: "{app}\lng\es-ES"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\fr-FR\mplng.dll.mui"; DestDir: "{app}\lng\fr-FR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\fr-FR\np3lng.dll.mui"; DestDir: "{app}\lng\fr-FR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\hi-IN\mplng.dll.mui"; DestDir: "{app}\lng\hi-IN"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\hi-IN\np3lng.dll.mui"; DestDir: "{app}\lng\hi-IN"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\hu-HU\mplng.dll.mui"; DestDir: "{app}\lng\hu-HU"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\hu-HU\np3lng.dll.mui"; DestDir: "{app}\lng\hu-HU"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\id-ID\mplng.dll.mui"; DestDir: "{app}\lng\id-ID"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\id-ID\np3lng.dll.mui"; DestDir: "{app}\lng\id-ID"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\it-IT\mplng.dll.mui"; DestDir: "{app}\lng\it-IT"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\it-IT\np3lng.dll.mui"; DestDir: "{app}\lng\it-IT"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\ja-JP\mplng.dll.mui"; DestDir: "{app}\lng\ja-JP"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\ja-JP\np3lng.dll.mui"; DestDir: "{app}\lng\ja-JP"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\ko-KR\mplng.dll.mui"; DestDir: "{app}\lng\ko-KR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\ko-KR\np3lng.dll.mui"; DestDir: "{app}\lng\ko-KR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\nl-NL\mplng.dll.mui"; DestDir: "{app}\lng\nl-NL"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\nl-NL\np3lng.dll.mui"; DestDir: "{app}\lng\nl-NL"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\pl-PL\mplng.dll.mui"; DestDir: "{app}\lng\pl-PL"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\pl-PL\np3lng.dll.mui"; DestDir: "{app}\lng\pl-PL"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\pt-BR\mplng.dll.mui"; DestDir: "{app}\lng\pt-BR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\pt-BR\np3lng.dll.mui"; DestDir: "{app}\lng\pt-BR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\pt-PT\mplng.dll.mui"; DestDir: "{app}\lng\pt-PT"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\pt-PT\np3lng.dll.mui"; DestDir: "{app}\lng\pt-PT"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\ru-RU\mplng.dll.mui"; DestDir: "{app}\lng\ru-RU"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\ru-RU\np3lng.dll.mui"; DestDir: "{app}\lng\ru-RU"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\sk-SK\mplng.dll.mui"; DestDir: "{app}\lng\sk-SK"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\sk-SK\np3lng.dll.mui"; DestDir: "{app}\lng\sk-SK"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\sv-SE\mplng.dll.mui"; DestDir: "{app}\lng\sv-SE"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\sv-SE\np3lng.dll.mui"; DestDir: "{app}\lng\sv-SE"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\tr-TR\mplng.dll.mui"; DestDir: "{app}\lng\tr-TR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\tr-TR\np3lng.dll.mui"; DestDir: "{app}\lng\tr-TR"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\vi-VN\mplng.dll.mui"; DestDir: "{app}\lng\vi-VN"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\vi-VN\np3lng.dll.mui"; DestDir: "{app}\lng\vi-VN"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\zh-CN\mplng.dll.mui"; DestDir: "{app}\lng\zh-CN"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\zh-CN\np3lng.dll.mui"; DestDir: "{app}\lng\zh-CN"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\zh-TW\mplng.dll.mui"; DestDir: "{app}\lng\zh-TW"; Flags: ignoreversion
Source: "{#bindir}\Release_x86_v142\lng\zh-TW\np3lng.dll.mui"; DestDir: "{app}\lng\zh-TW"; Flags: ignoreversion
Source: "Changes.txt"; DestDir: "{app}\Docs"; Flags: ignoreversion
Source: "Docs\*.txt"; DestDir: "{app}\Docs"; Flags: ignoreversion
Source: "Docs\crypto\*.txt"; DestDir: "{app}\Docs\crypto"; Flags: ignoreversion
Source: "Docs\uthash\*.txt"; DestDir: "{app}\Docs\uthash"; Flags: ignoreversion

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

[Registry]
;The following "Keys/Values" are required to allow a "Notepad3 Replacement" in Windows 11.
Root: "HKLM"; Subkey: "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\notepad.exe"; ValueType: dword; ValueName: "UseFilter"; ValueData: "1"
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
Root: "HKCR"; Subkey: "txtfile\shell\printto\command"; ValueType: expandsz; ValueData: "%SystemRoot%\system32\notepad.exe /pt ""%1"" ""%2"" ""%3"" ""%4"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.inf\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "inffile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ini\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "inifile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ps1\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "Microsoft.PowerShellScript.1"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psd1\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "Microsoft.PowerShellData.1"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.psm1\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "Microsoft.PowerShellModule.1"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.log\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "txtfile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.scp\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "txtfile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.txt\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "txtfile"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\UserChoice"; Flags: deletekey
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; ValueType: string; ValueName: "a"; ValueData: "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; ValueType: string; ValueName: "b"; ValueData: "{{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; ValueType: string; ValueName: "c"; ValueData: "Notepad.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; ValueType: string; ValueName: "d"; ValueData: "Notepad3.exe"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\OpenWithList"; ValueType: string; ValueName: "MRUList"; ValueData: "bacd"
Root: "HKCU"; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wtx\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "txtfile"
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

function InitializeSetup: Boolean;
begin
  Result := True;

   //Check for Processor SSE2 support.
  #if defined(sse2_required)
    if not IsSSE2Supported() then begin
      SuppressibleMsgBox(CustomMessage('msg_simd_sse2'), mbCriticalError, MB_OK, MB_OK);
      Result := False;
    end;
  #elif defined(sse_required)
    if not IsSSESupported() then begin
      SuppressibleMsgBox(CustomMessage('msg_simd_sse'), mbCriticalError, MB_OK, MB_OK);
      Result := False;
    end;
  #endif

end;

// Check if Notepad3 has replaced Windows Notepad
function DefaulNotepadCheck(): Boolean;
var
  sDebugger: String;
begin
  if RegQueryStringValue(HKLM, IFEO, 'Debugger', sDebugger) and
  (sDebugger = (ExpandConstant('"{app}\Notepad3.exe" /z'))) then begin
    Log('Custom Code: {#app_name} is set as the default notepad');
    Result := True;
  end
  else begin
    Log('Custom Code: {#app_name} is NOT set as the default notepad');
    Result := False;
  end;
end;

// Check if "Open with Notepad3" is installed.
function OpenWithCheck(): Boolean;
var
  sOpenWith: String;
begin
  if RegQueryStringValue(HKEY_CLASSES_ROOT, '*\shell\Open with Notepad3', 'Icon', sOpenWith) and
  (sOpenWith = (ExpandConstant('{app}\Notepad3.exe,0'))) then begin
    Log('Custom Code: {#app_name} Open with Notepad3 is set.');
    Result := True;
  end
  else begin
    Log('Custom Code: {#app_name} Open with Notepad3 is not set.');
    Result := False;
  end;
end;

#if defined(sse_required) || defined(sse2_required)
function IsProcessorFeaturePresent(Feature: Integer): Boolean;
external 'IsProcessorFeaturePresent@kernel32.dll stdcall';
#endif

#if defined(sse_required)
function IsSSESupported(): Boolean;
begin
  // PF_XMMI_INSTRUCTIONS_AVAILABLE
  Result := IsProcessorFeaturePresent(6);
end;

#elif defined(sse2_required)

function IsSSE2Supported(): Boolean;
begin
  // PF_XMMI64_INSTRUCTIONS_AVAILABLE
  Result := IsProcessorFeaturePresent(10);
end;

#endif

function IsOldBuildInstalled(sInfFile: String): Boolean;
begin
  if RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Notepad2') and
  FileExists(ExpandConstant('{commonpf}\Notepad2\' + sInfFile)) then
    Result := True
  else
    Result := False;
end;

function IsUpgrade(): Boolean;
var
  sPrevPath: String;
begin
  sPrevPath := WizardForm.PrevAppDir;
  Result := (sPrevPath <> '');
end;

// Check if Notepad3's settings exist
function SettingsExistCheck(): Boolean;
begin
  if FileExists(ExpandConstant('{userappdata}\Rizonesoft\Notepad3\Notepad3.ini')) then begin
    Log('Custom Code: Settings are present');
    Result := True;
  end
  else begin
    Log('Custom Code: Settings are NOT present');
    Result := False;
  end;
end;

function UninstallOldVersion(sInfFile: String): Integer;
var
  iResultCode: Integer;
begin
  // Return Values:
  // 0 - no idea
  // 1 - error executing the command
  // 2 - successfully executed the command

  // default return value
  Result := 0;
  // TODO: use RegQueryStringValue
  if not Exec('rundll32.exe', ExpandConstant('advpack.dll,LaunchINFSectionEx ' + '"{commonpf}\Notepad2\' + sInfFile +'",DefaultUninstall,,8,N'), '', SW_HIDE, ewWaitUntilTerminated, iResultCode) then begin
    Result := 1;
  end
  else begin
    Result := 2;
    Sleep(200);
  end;
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  // Hide the license page if IsUpgrade()
  if IsUpgrade() and (PageID = wpLicense) then
    Result := True;
end;

procedure AddReg();
begin
  RegWriteStringValue(HKCR, 'Applications\notepad3.exe', 'AppUserModelID', 'Rizonesoft.Notepad3');
  RegWriteStringValue(HKCR, 'Applications\notepad3.exe\shell\open\command', '', ExpandConstant('"{app}\Notepad3.exe" "%1"'));
  RegWriteStringValue(HKCR, '*\OpenWithList\notepad3.exe', '', '');
  RegWriteStringValue(HKLM, APPH, '', ExpandConstant('{app}\Notepad3.exe'));
  RegWriteStringValue(HKLM, APPH, 'Path', ExpandConstant('{app}'));
end;

procedure CleanUpSettings();
begin
  DeleteFile(ExpandConstant('{userappdata}\Rizonesoft\Notepad3\Notepad3.ini'));
  DeleteFile(ExpandConstant('{userappdata}\Rizonesoft\Notepad3\minipath.ini'));
  DeleteFile(ExpandConstant('{userappdata}\Rizonesoft\Notepad3\grepWinNP3.ini'));
  RemoveDir(ExpandConstant('{userappdata}\Rizonesoft\Notepad3'));
end;

procedure RemoveReg();
begin
  RegDeleteKeyIncludingSubkeys(HKCR, 'Applications\notepad3.exe');
  RegDeleteKeyIncludingSubkeys(HKCR, '*\OpenWithList\notepad3.exe');
  RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\Open with Notepad3');
  RegDeleteKeyIncludingSubkeys(HKLM, APPH);
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectTasks then
    WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall)
  else if CurPageID = wpFinished then
    WizardForm.NextButton.Caption := SetupMessage(msgButtonFinish);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then begin
    if WizardIsTaskSelected('reset_settings') then
      CleanUpSettings();

    if IsOldBuildInstalled('Uninstall.inf') or IsOldBuildInstalled('Notepad2.inf') then begin
      if IsOldBuildInstalled('Uninstall.inf') then begin
        Log('Custom Code: The old build is installed, will try to uninstall it');
        if UninstallOldVersion('Uninstall.inf') = 2 then
          Log('Custom Code: The old build was successfully uninstalled')
        else
          Log('Custom Code: Something went wrong when uninstalling the old build');
      end;

      if IsOldBuildInstalled('Notepad2.inf') then begin
        Log('Custom Code: The official Notepad2 build is installed, will try to uninstall it');
        if UninstallOldVersion('Notepad2.inf') = 2 then
          Log('Custom Code: The official Notepad2 build was successfully uninstalled')
        else
          Log('Custom Code: Something went wrong when uninstalling the official Notepad2 build');
      end;

      // This is the case where the old build is installed; the DefaulNotepadCheck() returns true
      // and the set_default task isn't selected
      if not WizardIsTaskSelected('remove_default') then begin
        RegWriteStringValue(HKLM, IFEO, 'Debugger', ExpandConstant('"{app}\Notepad3.exe" /z'));
        RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 0);
      end;
    end;
  end;

  if CurStep = ssPostInstall then begin
    if WizardIsTaskSelected('set_default') then begin
      RegWriteStringValue(HKLM, IFEO, 'Debugger', ExpandConstant('"{app}\Notepad3.exe" /z'));
      RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 0);
    end;
    if WizardIsTaskSelected('remove_default') then begin
      RegDeleteValue(HKLM, IFEO, 'Debugger');
      RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 1);
    end else begin
      If RegValueExists (HKLM, IFEO, 'Debugger') then begin
        RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 0);
      end else begin
        RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 1);
      end;
    end;
    if WizardIsTaskSelected('set_openwith') then begin
      RegWriteStringValue(HKCR, '*\shell\Open with Notepad3', 'Icon', ExpandConstant('{app}\Notepad3.exe,0'));
      RegWriteStringValue(HKCR, '*\shell\Open with Notepad3\command', '', ExpandConstant('"{app}\Notepad3.exe" "%1"'));
    end;
    if WizardIsTaskSelected('remove_openwith') then begin
      RegDeleteKeyIncludingSubkeys(HKCR, '*\shell\Open with Notepad3');
    end;
    // Always add Notepad3's AppUserModelID and the rest registry values
    AddReg();
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // When uninstalling, ask the user to delete Notepad3's settings
  if CurUninstallStep = usUninstall then begin
    if SettingsExistCheck() then begin
      if SuppressibleMsgBox(CustomMessage('msg_DeleteSettings'), mbConfirmation, MB_YESNO or MB_DEFBUTTON2, IDNO) = IDYES then
        CleanUpSettings();
    end;
    if DefaulNotepadCheck() then begin
      RegDeleteValue(HKLM, IFEO, 'Debugger');
      RegWriteDWordValue(HKLM, IFEO, 'UseFilter', 1);
    end;
    RemoveReg();
  end;
end;

procedure InitializeWizard();
begin
  WizardForm.SelectTasksLabel.Hide;
  WizardForm.TasksList.Top    := 0;
  WizardForm.TasksList.Height := PageFromID(wpSelectTasks).SurfaceHeight;
end;
