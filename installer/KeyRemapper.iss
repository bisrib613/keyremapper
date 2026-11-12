#define AppName "Key Remapper"
#define AppVersion "1.5.2"

[Setup]
AppId={{B0F2EDC4-65D9-4E6A-B8C4-23D41B532C6E}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=bisrib613
DefaultDirName={localappdata}\Programs\Key Remapper
DefaultGroupName=Key Remapper
OutputDir=..\dist
OutputBaseFilename=KeyRemapper_Setup
Compression=lzma2/max
SolidCompression=yes
PrivilegesRequired=lowest
UninstallDisplayIcon={app}\KeyRemapper.exe
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
SetupIconFile=..\assets\keyremapper.ico

[Files]
; Preserve the user's current mappings before replacing the executable.
Source: "{userappdata}\KeyRemapperClean\mappings.txt"; DestDir: "{userappdata}\KeyRemapperClean"; DestName: "mappings.bak"; Flags: external skipifsourcedoesntexist uninsneveruninstall
Source: "..\build\KeyRemapper.exe"; DestDir: "{app}"; Flags: ignoreversion

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: unchecked

[Icons]
Name: "{autoprograms}\Key Remapper"; Filename: "{app}\KeyRemapper.exe"
Name: "{autodesktop}\Key Remapper"; Filename: "{app}\KeyRemapper.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\KeyRemapper.exe"; Description: "Run Key Remapper"; Flags: nowait postinstall skipifsilent
