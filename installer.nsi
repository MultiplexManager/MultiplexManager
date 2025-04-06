; installer.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install installer.nsi into a directory that the user selects,

;--------------------------------
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!include MultiUser.nsh
!include MUI2.nsh


; The name of the installer
Name "Multiplex Manager 1.2"

; The file to write
OutFile "setup.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Multiplex Manager"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Multiplex_Manager" "Install_Dir"

;--------------------------------

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "gpl.txt"
  !insertmacro MULTIUSER_PAGE_INSTALLMODE

  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

; Pages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
Section "Multiplex Manager (required)" SecMain

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "gpl.txt"
  File "sample.xml"
  File "release\MultiplexManager.exe"
  File "$%QTDIR%\bin\QtCore4.dll"
  File "$%QTDIR%\bin\QtGui4.dll"
  File "$%QTDIR%\bin\QtXml4.dll"
  File "$%QTDIR%\bin\mingwm10.dll"
  File "$%QTDIR%\bin\libgcc_s_dw2-1.dll"

  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Multiplex Manager" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Multiplex Manager" "DisplayName" "Multiplex Manager"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Multiplex Manager" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Multiplex Manager" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Multiplex Manager" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts" SecShortcuts

  CreateDirectory "$SMPROGRAMS\Multiplex Manager"
  CreateShortCut "$SMPROGRAMS\Multiplex Manager\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Multiplex Manager\Multiplex Manager.lnk" "$INSTDIR\MultiplexManager.exe" "" "$INSTDIR\MultiplexManager.exe" 0
  
SectionEnd


LangString DESC_SecMain ${LANG_ENGLISH} "Installs the executable and required libraries."
LangString DESC_SecShortcuts ${LANG_ENGLISH} "Adds a shortcut to the Start Menu."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} $(DESC_SecShortcuts)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Multiplex Manager"
  DeleteRegKey HKLM "SOFTWARE\Multiplex Manager"

  ; Remove files and uninstaller
  Delete $INSTDIR\installer.nsi
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Multiplex Manager\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Multiplex Manager"
  RMDir "$INSTDIR"

SectionEnd


Function .onInit
  !insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
  !insertmacro MULTIUSER_UNINIT
FunctionEnd
