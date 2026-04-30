#define MyAppName "Notepad3"
#define MyAppVersion "7.26.429.104"
#define MyAppPublisher "Rizonesoft"
#define MyAppExeName "Notepad3.exe"
#define MyAppAssocName MyAppName + " Document"
#define MyAppAssocExt ".txt"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
AppId={{9D18BB14-32A4-4AF4-9FB0-AD8A0CC6A5C1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=F:\Notepad3-master\Build\packages\Notepad3_7.26.429.104_x64_Portable\License.txt
OutputDir=F:\Notepad3-master\Build\packages
OutputBaseFilename=Notepad3_{#MyAppVersion}_x64_Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupIconFile=F:\Notepad3-master\np3portableapp\Notepad3Portable\App\AppInfo\appicon.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "F:\Notepad3-master\Build\packages\Notepad3_7.26.429.104_x64_Portable\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
