#define AppName "Key Remapper"
#define AppVersion "1.0.0"
[Setup]
AppId={{A7BCA535-FA88-499C-A291-B928A0CB6E09}
AppName={#AppName}
AppVersion={#AppVersion}
DefaultDirName={localappdata}\Programs\Key Remapper
DefaultGroupName=Key Remapper
OutputDir=..\dist
OutputBaseFilename=KeyRemapper_Setup
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
UninstallDisplayIcon={app}\KeyRemapper.exe
[Files]
Source: "..\publish\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion
[Tasks]
Name: desktopicon; Description: "Create a desktop shortcut"; Flags: unchecked
[Icons]
Name: "{autoprograms}\Key Remapper"; Filename: "{app}\KeyRemapper.exe"
Name: "{autodesktop}\Key Remapper"; Filename: "{app}\KeyRemapper.exe"; Tasks: desktopicon
[Run]
Filename: "{app}\KeyRemapper.exe"; Description: "Run Key Remapper"; Flags: nowait postinstall skipifsilent