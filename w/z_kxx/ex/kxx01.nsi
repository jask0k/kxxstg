; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\SimpChinese.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Japanese.nlf"


; The name of the installer
LangString Name ${LANG_ENGLISH} "Touhou Idealistic World"
LangString Name ${LANG_SIMPCHINESE} "东方空想乡"
LangString Name ${LANG_JAPANESE} "Touhou Idealistic World"

LangString ConfigTool ${LANG_ENGLISH} "Touhou Idealistic World Config Tool"
LangString ConfigTool ${LANG_SIMPCHINESE} "东方空想乡设置"
LangString ConfigTool ${LANG_JAPANESE} "Touhou Idealistic World Config Tool"


Name $(Name)

Icon "kxx01.ico" 

#AddBrandingImage top 64

; The file to write
OutFile "..\bin\kxx01-setup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Hexbug\Kxx01

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Hexbug\Kxx01" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

XPStyle on




;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
LangString SectionProgramFiles ${LANG_ENGLISH} "Program Files (required)"
LangString SectionProgramFiles ${LANG_SIMPCHINESE} "程序和资源(必选)"
LangString SectionProgramFiles ${LANG_JAPANESE} "Program Files (required)"

Section "$(SectionProgramFiles)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\bin\kxx01.exe"
  File "..\bin\kxx01-config.exe"
  File "..\bin\res_"
  File "..\bin\bgm_"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM Software\Hexbug\Kxx01 "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HexbugKxx01" "DisplayName" "KXX01"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HexbugKxx01" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HexbugKxx01" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HexbugKxx01" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
LangString SectionStartMenu ${LANG_ENGLISH} "Start Menu Shortcuts"
LangString SectionStartMenu ${LANG_SIMPCHINESE} "开始菜单快捷方式"
LangString SectionStartMenu ${LANG_JAPANESE} "Start Menu Shortcuts"

Section "$(SectionStartMenu)"

  CreateDirectory "$SMPROGRAMS\Kxx01"
  CreateShortCut "$SMPROGRAMS\Kxx01\$(Name).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Kxx01\$(Name).lnk" "$INSTDIR\kxx01.exe" "" "$INSTDIR\kxx01.exe" 0
  CreateShortCut "$SMPROGRAMS\Kxx01\$(ConfigTool).lnk" "$INSTDIR\kxx01_config.exe" "" "$INSTDIR\kxx01_config.exe" 0
  
SectionEnd

LangString SectionDesktopShortcuts ${LANG_ENGLISH} "Desktop Shortcuts"
LangString SectionDesktopShortcuts ${LANG_SIMPCHINESE} "桌面快捷方式"
LangString SectionDesktopShortcuts ${LANG_JAPANESE} "Desktop Shortcuts"

Section /o "$(SectionDesktopShortcuts)"

  CreateShortCut "$DESKTOP\$(Name).lnk" "$INSTDIR\kxx01.exe" "" "$INSTDIR\kxx01.exe" 0
  
SectionEnd


;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HexbugKxx01"
  DeleteRegKey HKLM SOFTWARE\Hexbug

  ; Remove files and uninstaller
  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Kxx01\*.*"
  RMDir "$SMPROGRAMS\Kxx01"
  Delete "$DESKTOP\$(Name).lnk"

SectionEnd

Function .onInit

	; System::Call 'kernel32::CreateMutexA(i 0, i 0, t "HexbugKxx01Mutex") i .r1 ?e'
	; Pop $R0
 
	; StrCmp $R0 0 +3
	; MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
	; Abort
	
	;Language selection dialog

	Push ""
	Push ${LANG_ENGLISH}
	Push English
	Push ${LANG_SIMPCHINESE}
	Push "Simplified Chinese"
	Push ${LANG_JAPANESE}
	Push Janpanese
	Push A ; A means auto count languages
	       ; for the auto count to work the first empty push (Push "") must remain
	LangDLL::LangDialog "Installer Language" "Please select the language of the installer"

	Pop $LANGUAGE
	StrCmp $LANGUAGE "cancel" 0 +2
		Abort

FunctionEnd

