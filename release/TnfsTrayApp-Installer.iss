[Setup]
AppName=TnfsTrayApp
AppVersion=1.0.0
DefaultDirName={autopf}\TnfsTrayApp
OutputDir=..\release
OutputBaseFilename=TnfsTrayApp-Setup

[Files]
; Source files staged in win-temp by the script
Source: "win-temp\TnfsTrayApp.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "win-temp\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\TnfsTrayApp"; Filename: "{app}\TnfsTrayApp.exe"
Name: "{autodesktop}\TnfsTrayApp"; Filename: "{app}\TnfsTrayApp.exe"