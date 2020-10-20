; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Система автоматической подачи звонков"
#define MyAppVersion "1.0"
#define MyAppPublisher "Кондратенко Данила"
#define MyAppURL "https://github.com/danil-kondr2016/autoringsystem"
#define MyAppExeName "AutoRingSystemCP.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{34477F8B-7886-4503-90E8-AFBA30636C45}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\AutoRingSystemCP
DisableProgramGroupPage=yes
ArchitecturesInstallIn64BitMode=x64
; The [Icons] "quicklaunchicon" entry uses {userappdata} but its [Tasks] entry has a proper IsAdminInstallMode Check.
UsedUserAreasWarning=no
LicenseFile=.\gpl-3.0.rtf
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputDir=.\
OutputBaseFilename=autoringsystemcp_x64
SetupIconFile=.\autoring.ico
Compression=lzma
SolidCompression=yes
WizardStyle=classic

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
Source: ".\ARS\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCR; Subkey: ".shdl"; ValueType: string; ValueName: ""; ValueData: "AutoRingSystemCP"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "AutoRingSystemCP"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"; Flags: uninsdeletekey
Root: HKCR; Subkey: "AutoRingSystemCP\DefaultIcon"; ValueType: string; ValueName: ""; ValueData:"{autopf}\AutoRingSystemCP\AutoRingSystemCP.exe,1"
Root: HKCR; Subkey: "AutoRingSystemCP\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{autopf}\AutoRingSystemCP\AutoRingSystemCP.exe"" ""%1"""
