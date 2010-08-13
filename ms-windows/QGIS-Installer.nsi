;----------------------------------------------------------------------------------------------------------------------------

;Quantum GIS Installer for Windows
;Written by Marco Pasetti
;Mail to: marco.pasetti@alice.it 
;
;Extended for creatensis.pl by Jürgen E. Fischer <jef@norbit.de>

;----------------------------------------------------------------------------------------------------------------------------

;Select if you are building a "Development Version" or a "Release Version" of the Quantum GIS Installer
;Change the INSTALLER_TYPE variable to Release or Development

;----------------------------------------------------------------------------------------------------------------------------

;Version variables

!define RELEASE_VERSION_NUMBER "1.4.0"
!define RELEASE_VERSION_NAME "Enceladus"
!define RELEASE_SVN_REVISION "12636"
!define RELEASE_BINARY_REVISION "0"

!define DEV_VERSION_NUMBER ""
!define DEV_VERSION_NAME ""
!define DEV_SVN_REVISION ""
!define DEV_BINARY_REVISION ""

; Added by Tim to get optimal compression
SetCompressor /SOLID lzma
; Added by Tim to allow priveledge elevation in vista
RequestExecutionLevel admin

;----------------------------------------------------------------------------------------------------------------------------

;Don't modify the following lines

;----------------------------------------------------------------------------------------------------------------------------

;NSIS Includes

!include "MUI.nsh"
!include "LogicLib.nsh"

;----------------------------------------------------------------------------------------------------------------------------

;Set the installer variables, depending on the selected version to build

!if ${INSTALLER_TYPE} == "Release"
	!define RELEASE_QGIS_BASE "Quantum GIS Enceladus"
	!define VERSION_NUMBER "${RELEASE_VERSION_NUMBER}"
	!define VERSION_NAME "${RELEASE_VERSION_NAME}"
	!define COMPLETE_NAME "${RELEASE_QGIS_BASE} ${RELEASE_VERSION_NUMBER} ${RELEASE_VERSION_NAME}"
	!define SVN_REVISION "${RELEASE_SVN_REVISION}"
	!define BINARY_REVISION "${RELEASE_BINARY_REVISION}"
	!define QGIS_BASE "${RELEASE_QGIS_BASE}"
	!define INSTALLER_NAME "QGIS-${VERSION_NUMBER}-${BINARY_REVISION}-Setup.exe"
	!define DISPLAYED_NAME "${RELEASE_QGIS_BASE} ${VERSION_NUMBER}-${BINARY_REVISION}"
	!define CHECK_INSTALL_NAME "${RELEASE_QGIS_BASE}"
	!define INSTALLER_DISPLAYED_NAME "${COMPLETE_NAME}"
	!define PACKAGE_FOLDER ".\QGIS-Release-Package"
!else if ${INSTALLER_TYPE} == "Release-NoGrass"
	!define VERSION_NUMBER "${RELEASE_VERSION_NUMBER}"
	!define VERSION_NAME "${RELEASE_VERSION_NAME}"
	!define COMPLETE_NAME "${RELEASE_QGIS_BASE} ${RELEASE_VERSION_NUMBER}"
	!define SVN_REVISION "${RELEASE_SVN_REVISION}"
	!define BINARY_REVISION "${RELEASE_BINARY_REVISION}"
	!define QGIS_BASE "${RELEASE_QGIS_BASE}"
	!define INSTALLER_NAME "QGIS-${VERSION_NUMBER}-${BINARY_REVISION}-No-GrassSetup.exe"
	!define DISPLAYED_NAME "${RELEASE_QGIS_BASE} ${VERSION_NUMBER}-${BINARY_REVISION}"
	!define CHECK_INSTALL_NAME "${RELEASE_QGIS_BASE}"
	!define INSTALLER_DISPLAYED_NAME "${COMPLETE_NAME}"
	!define PACKAGE_FOLDER ".\QGIS-Release-Package-No-Grass"
!else if ${INSTALLER_TYPE} == "Development"
	!define DEV_QGIS_BASE "Quantum GIS Unstable Dev"
	!define VERSION_NUMBER "${DEV_VERSION_NUMBER}"
	!define VERSION_NAME "${DEV_VERSION_NAME}"
	!define COMPLETE_NAME "${DEV_QGIS_BASE} ${DEV_VERSION_NUMBER} ${DEV_VERSION_NAME}"
	!define SVN_REVISION "${DEV_SVN_REVISION}"
	!define BINARY_REVISION "${DEV_BINARY_REVISION}"
	!define QGIS_BASE "${DEV_QGIS_BASE}"
	!define INSTALLER_NAME "QGIS-Dev-r${SVN_REVISION}-${BINARY_REVISION}-Setup.exe"
	!define DISPLAYED_NAME "${DEV_QGIS_BASE} ${VERSION_NUMBER}-r${SVN_REVISION}-${BINARY_REVISION}"
	!define CHECK_INSTALL_NAME "${DEV_QGIS_BASE}"
	!define INSTALLER_DISPLAYED_NAME "${DISPLAYED_NAME}"
	!define PACKAGE_FOLDER ".\QGIS-Dev-Package"
!else if ${INSTALLER_TYPE} == "OSGeo4W"
	!define COMPLETE_NAME "${QGIS_BASE} ${VERSION_NUMBER} ${VERSION_NAME}"
	!define CHECK_INSTALL_NAME "${QGIS_BASE}"
	!define INSTALLER_DISPLAYED_NAME "${DISPLAYED_NAME}"

	!addplugindir osgeo4w/untgz
	!addplugindir osgeo4w/nsis
!endif

;----------------------------------------------------------------------------------------------------------------------------

;Publisher variables

!define PUBLISHER "QGIS Development Team"
!define WEB_SITE "http://www.qgis.org"
!define WIKI_PAGE "http://wiki.qgis.org/qgiswiki"

;----------------------------------------------------------------------------------------------------------------------------

;General Definitions

;Name of the application shown during install
Name "${INSTALLER_DISPLAYED_NAME}"

;Name of the output file (installer executable)
OutFile "${INSTALLER_NAME}"

;Define installation folder
InstallDir "$PROGRAMFILES\${QGIS_BASE}"

;Tell the installer to show Install and Uninstall details as default
ShowInstDetails show
ShowUnInstDetails show

;----------------------------------------------------------------------------------------------------------------------------

;.onInit Function (called when the installer is nearly finished initializing)

;Check if QGIS is already installed on the system and, if yes, what version and binary release;
;depending on that, select the install procedure:

;1. first installation = if QGIS is not already installed
;install QGIS asking for the install PATH

;2. upgrade installation = if an older release of QGIS is already installed
;call the uninstaller of the currently installed QGIS release
;if the uninstall procedure succeeded, call the current installer without asking for the install PATH
;QGIS will be installed in the same PATH of the previous installation

;3. downgrade installation = if a newer release of QGIS is already installed
;call the uninstaller of the currently installed QGIS release
;if the uninstall procedure succeeded, call the current installer without asking for the install PATH
;QGIS will be installed in the same PATH of the previous installation

;4. repair installation = if the same release of QGIS is already installed
;call the uninstaller of the currently installed QGIS release
;if the uninstall procedure succeeded, call the current installer asking for the install PATH

;the currently installed release of QGIS is defined by the variable $INSTALLED_VERSION = $INSTALLED_SVN_REVISION + $INSTALLED_BINARY_REVISION 

Function .onInit

	Var /GLOBAL ASK_FOR_PATH
	StrCpy $ASK_FOR_PATH "YES"

	Var /GLOBAL UNINSTALL_STRING
	Var /GLOBAL INSTALL_PATH
	
	Var /GLOBAL INSTALLED_VERSION_NUMBER
	Var /GLOBAL INSTALLED_SVN_REVISION
	Var /GLOBAL INSTALLED_BINARY_REVISION
	
	Var /GLOBAL INSTALLED_VERSION
	
	Var /GLOBAL DISPLAYED_INSTALLED_VERSION
	
	Var /GLOBAL MESSAGE_0_
	Var /GLOBAL MESSAGE_1_
	Var /GLOBAL MESSAGE_2_
	Var /GLOBAL MESSAGE_3_
	
	ReadRegStr $UNINSTALL_STRING HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}" "UninstallString"
	ReadRegStr $INSTALL_PATH HKLM "Software\${QGIS_BASE}" "InstallPath"
	ReadRegStr $INSTALLED_VERSION_NUMBER HKLM "Software\${QGIS_BASE}" "VersionNumber"
	ReadRegStr $INSTALLED_SVN_REVISION HKLM "Software\${QGIS_BASE}" "SvnRevision"
	
	${If} $INSTALLED_SVN_REVISION == ""
		ReadRegStr $INSTALLED_SVN_REVISION HKLM "Software\${QGIS_BASE}" "Revision"
	${EndIf}	
	
	ReadRegStr $INSTALLED_BINARY_REVISION HKLM "Software\${QGIS_BASE}" "BinaryRevision"
	
	StrCpy $MESSAGE_0_ "${CHECK_INSTALL_NAME} is already installed on your system.$\r$\n"
	StrCpy $MESSAGE_0_ "$MESSAGE_0_$\r$\n"
	
	!if ${INSTALLER_TYPE} == "Release"		
		${If} $INSTALLED_BINARY_REVISION == ""
			StrCpy $DISPLAYED_INSTALLED_VERSION "$INSTALLED_VERSION_NUMBER"
		${Else}
			StrCpy $DISPLAYED_INSTALLED_VERSION "$INSTALLED_VERSION_NUMBER-$INSTALLED_BINARY_REVISION"
		${EndIf}
	!else
		StrCpy $DISPLAYED_INSTALLED_VERSION "$INSTALLED_VERSION_NUMBER-$INSTALLED_SVN_REVISION-$INSTALLED_BINARY_REVISION"
	!endif
	
	StrCpy $MESSAGE_0_ "$MESSAGE_0_The installed release is $DISPLAYED_INSTALLED_VERSION$\r$\n"
	
	StrCpy $MESSAGE_1_ "$MESSAGE_0_$\r$\n"
	StrCpy $MESSAGE_1_ "$MESSAGE_1_You are going to install a newer release of ${CHECK_INSTALL_NAME}$\r$\n"
	StrCpy $MESSAGE_1_ "$MESSAGE_1_$\r$\n"
	StrCpy $MESSAGE_1_ "$MESSAGE_1_Press OK to uninstall Quantum GIS $DISPLAYED_INSTALLED_VERSION"
	StrCpy $MESSAGE_1_ "$MESSAGE_1_ and install ${DISPLAYED_NAME} or Cancel to quit."
	
	StrCpy $MESSAGE_2_ "$MESSAGE_0_$\r$\n"
	StrCpy $MESSAGE_2_ "$MESSAGE_2_You are going to install an older release of ${CHECK_INSTALL_NAME}$\r$\n"
	StrCpy $MESSAGE_2_ "$MESSAGE_2_$\r$\n"
	StrCpy $MESSAGE_2_ "$MESSAGE_2_Press OK to uninstall Quantum GIS $DISPLAYED_INSTALLED_VERSION"
	StrCpy $MESSAGE_2_ "$MESSAGE_2_ and install ${DISPLAYED_NAME} or Cancel to quit."
	
	StrCpy $MESSAGE_3_ "$MESSAGE_0_$\r$\n"
	StrCpy $MESSAGE_3_ "$MESSAGE_3_This is the latest release available.$\r$\n"
	StrCpy $MESSAGE_3_ "$MESSAGE_3_$\r$\n"
	StrCpy $MESSAGE_3_ "$MESSAGE_3_Press OK to reinstall ${DISPLAYED_NAME} or Cancel to quit."
	
	IntOp $INSTALLED_SVN_REVISION $INSTALLED_SVN_REVISION * 1
	IntOp $INSTALLED_BINARY_REVISION $INSTALLED_BINARY_REVISION * 1
	IntOp $INSTALLED_VERSION $INSTALLED_SVN_REVISION + $INSTALLED_BINARY_REVISION
	
	!define /math VERSION ${SVN_REVISION} + ${BINARY_REVISION}
	
	${If} $INSTALLED_VERSION_NUMBER == ""
	${Else}
		${If} $INSTALLED_VERSION < ${VERSION}
			MessageBox MB_OKCANCEL "$MESSAGE_1_" IDOK upgrade IDCANCEL quit_upgrade
			upgrade:
				StrCpy $ASK_FOR_PATH "NO"
				ExecWait '"$UNINSTALL_STRING" _?=$INSTALL_PATH' $0
				Goto continue_upgrade
			quit_upgrade:
				Abort
			continue_upgrade:
		${ElseIf} $INSTALLED_VERSION > ${VERSION}
			MessageBox MB_OKCANCEL "$MESSAGE_2_" IDOK downgrade IDCANCEL quit_downgrade
			downgrade:
				StrCpy $ASK_FOR_PATH "NO"
				ExecWait '"$UNINSTALL_STRING" _?=$INSTALL_PATH' $0
				Goto continue_downgrade
			quit_downgrade:
				Abort
			continue_downgrade:
		${ElseIf} $INSTALLED_VERSION = ${VERSION}
			MessageBox MB_OKCANCEL "$MESSAGE_3_" IDOK reinstall IDCANCEL quit_reinstall
			reinstall:
				ExecWait '"$UNINSTALL_STRING" _?=$INSTALL_PATH' $0
				Goto continue_reinstall
			quit_reinstall:
				Abort
			continue_reinstall:
		${EndIf}	
	${EndIf}
	
	${If} $INSTALLED_VERSION_NUMBER == ""
	${Else}
		${If} $0 = 0
		${Else}
			Abort
		${EndIf}
	${EndIf}

FunctionEnd

;----------------------------------------------------------------------------------------------------------------------------

;CheckUpdate Function
;Check if to show the MUI_PAGE_DIRECTORY during the installation (to ask for the install PATH)

Function CheckUpdate

	${If} $ASK_FOR_PATH == "NO"	
		Abort
	${EndIf}
	
FunctionEnd

;----------------------------------------------------------------------------------------------------------------------------

;Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON ".\Installer-Files\Install_QGIS.ico"
!define MUI_UNICON ".\Installer-Files\Uninstall_QGIS.ico"
!define MUI_HEADERIMAGE_BITMAP_NOSTETCH ".\Installer-Files\InstallHeaderImage.bmp"
!define MUI_HEADERIMAGE_UNBITMAP_NOSTRETCH ".\Installer-Files\UnInstallHeaderImage.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP ".\Installer-Files\WelcomeFinishPage.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ".\Installer-Files\UnWelcomeFinishPage.bmp"

;----------------------------------------------------------------------------------------------------------------------------

;Installer Pages

!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ".\Installer-Files\LICENSE.txt"

!define MUI_PAGE_CUSTOMFUNCTION_PRE CheckUpdate
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_UNPAGE_FINISH

;----------------------------------------------------------------------------------------------------------------------------

; Language files
!insertmacro MUI_LANGUAGE "English"

;----------------------------------------------------------------------------------------------------------------------------
	
;Installer Sections

;Declares the variables for optional Sample Data Sections
Var /GLOBAL HTTP_PATH
Var /GLOBAL ARCHIVE_NAME
Var /GLOBAL EXTENDED_ARCHIVE_NAME
Var /GLOBAL ORIGINAL_UNTAR_FOLDER
Var /GLOBAL CUSTOM_UNTAR_FOLDER
Var /GLOBAL ARCHIVE_SIZE_KB
Var /GLOBAL ARCHIVE_SIZE_MB
Var /GLOBAL DOWNLOAD_MESSAGE_

Section "Quantum GIS" SecQGIS

	SectionIn RO

        ;Added by Tim to set the reg key so we get default toolbar layout
        !include ui.nsh
        ;Added by Tim to set the reg key so we get default plugin loading 
        !include plugins.nsh
        ;Added by Tim to set the reg key so we get default python & py plugins
        !include python_plugins.nsh
	
	;Set the INSTALL_DIR variable
	Var /GLOBAL INSTALL_DIR
	
	${If} $ASK_FOR_PATH == "NO"	
		StrCpy $INSTALL_DIR "$INSTALL_PATH"
	${Else}
		StrCpy $INSTALL_DIR "$INSTDIR"
	${EndIf}
	
	;Set to try to overwrite existing files
	SetOverwrite try
	
	;Set the GIS_DATABASE directory
	SetShellVarContext current
	Var /GLOBAL GIS_DATABASE	
	StrCpy $GIS_DATABASE "$DOCUMENTS\GIS DataBase"
	
	;Create the GIS_DATABASE directory
	CreateDirectory "$GIS_DATABASE"

	;add Installer files
	SetOutPath "$INSTALL_DIR\icons"
	File .\Installer-Files\QGIS.ico
	File .\Installer-Files\QGIS_Web.ico
	SetOutPath "$INSTALL_DIR"
	File .\Installer-Files\QGIS-WebSite.url
!if ${INSTALLER_TYPE} == "OSGeo4W"
	File .\Installer-Files\postinstall.bat
	File .\Installer-Files\preremove.bat
!endif
	
	;add Quantum GIS files
	SetOutPath "$INSTALL_DIR"
	File /r ${PACKAGE_FOLDER}\*.*
	
	;Create the Uninstaller
	WriteUninstaller "$INSTALL_DIR\Uninstall-QGIS.exe"
	
	;Registry Key Entries
	
	;HKEY_LOCAL_MACHINE Install entries
	;Set the Name, Version and Revision of QGIS+ PublisherInfo + InstallPath	
	WriteRegStr HKLM "Software\${QGIS_BASE}" "Name" "${QGIS_BASE}"
	WriteRegStr HKLM "Software\${QGIS_BASE}" "VersionNumber" "${VERSION_NUMBER}"
	WriteRegStr HKLM "Software\${QGIS_BASE}" "VersionName" "${VERSION_NAME}"
	WriteRegStr HKLM "Software\${QGIS_BASE}" "SvnRevision" "${SVN_REVISION}"
	WriteRegStr HKLM "Software\${QGIS_BASE}" "BinaryRevision" "${BINARY_REVISION}"
	WriteRegStr HKLM "Software\${QGIS_BASE}" "Publisher" "${PUBLISHER}"
	WriteRegStr HKLM "Software\${QGIS_BASE}" "WebSite" "${WEB_SITE}"
	WriteRegStr HKLM "Software\${QGIS_BASE}" "InstallPath" "$INSTALL_DIR"
	
	;HKEY_LOCAL_MACHINE Uninstall entries
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}" "DisplayName" "${COMPLETE_NAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}" "UninstallString" "$INSTALL_DIR\Uninstall-QGIS.exe"
	
	!if ${INSTALLER_TYPE} == "Release"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}"\
		"DisplayVersion" "${VERSION_NUMBER}-${BINARY_REVISION}"
	!else
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}"\
		"DisplayVersion" "${VERSION_NUMBER}-r${SVN_REVISION}-${BINARY_REVISION}"
	!endif
	
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}" "DisplayIcon" "$INSTALL_DIR\icons\QGIS.ico"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}" "EstimatedSize" 1
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}" "HelpLink" "${WIKI_PAGE}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}" "URLInfoAbout" "${WEB_SITE}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}" "Publisher" "${PUBLISHER}"
	
	;Create the Desktop Shortcut
	SetShellVarContext current
	
!if ${INSTALLER_TYPE} == "OSGeo4W"
	CreateShortCut "$DESKTOP\${QGIS_BASE}.lnk" "$INSTALL_DIR\bin\nircmd.exe" 'exec hide "$INSTALL_DIR\bin\qgis.bat"' \
	"$INSTALL_DIR\icons\QGIS.ico" "" SW_SHOWNORMAL "" "Launch ${COMPLETE_NAME}"
!else
	CreateShortCut "$DESKTOP\${QGIS_BASE}.lnk" "$INSTALL_DIR\bin\qgis.exe" ""\
	"$INSTALL_DIR\icons\QGIS.ico" "" SW_SHOWNORMAL "" "Launch ${COMPLETE_NAME}"
!endif
 
	;Create the Windows Start Menu Shortcuts
	SetShellVarContext all
	
	CreateDirectory "$SMPROGRAMS\${QGIS_BASE}"
	
!if ${INSTALLER_TYPE} == "OSGeo4W"
	CreateShortCut "$SMPROGRAMS\${QGIS_BASE}\${QGIS_BASE}.lnk" "$INSTALL_DIR\bin\nircmd.exe" 'exec hide "$INSTALL_DIR\bin\qgis.bat"' \
	"$INSTALL_DIR\icons\QGIS.ico" "" SW_SHOWNORMAL "" "Launch ${COMPLETE_NAME}"
!else
	CreateShortCut "$SMPROGRAMS\${QGIS_BASE}\${QGIS_BASE}.lnk" "$INSTALL_DIR\bin\qgis.exe" ""\
	"$INSTALL_DIR\icons\QGIS.ico" "" SW_SHOWNORMAL "" "Launch ${COMPLETE_NAME}"
!endif
	
	CreateShortCut "$SMPROGRAMS\${QGIS_BASE}\Quantum GIS Web Site.lnk" "$INSTALL_DIR\QGIS-WebSite.url" ""\
	"$INSTALL_DIR\icons\QGIS_Web.ico" "" SW_SHOWNORMAL "" "Visit the Quantum GIS Web Site"
	
	CreateShortCut "$SMPROGRAMS\${QGIS_BASE}\Uninstall ${QGIS_BASE}.lnk" "$INSTALL_DIR\Uninstall-QGIS.exe" ""\
	"$INSTALL_DIR\Uninstall-QGIS.exe" "" SW_SHOWNORMAL "" "Uninstall ${COMPLETE_NAME}"

	GetFullPathName /SHORT $0 $INSTALL_DIR
	System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("OSGEO4W_ROOT", "$0").r0'
	System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("OSGEO4W_STARTMENU", "$SMPROGRAMS\${QGIS_BASE}").r0'

	ReadEnvStr $0 COMSPEC
	nsExec::ExecToLog '"$0" /c "$INSTALL_DIR\postinstall.bat"'
SectionEnd

Function DownloadDataSet

	IntOp $ARCHIVE_SIZE_MB $ARCHIVE_SIZE_KB / 1024
	
	StrCpy $DOWNLOAD_MESSAGE_ "The installer will download the $EXTENDED_ARCHIVE_NAME sample data set.$\r$\n"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_$\r$\n"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_The archive is about $ARCHIVE_SIZE_MB MB and may take"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_ several minutes to be downloaded.$\r$\n"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_$\r$\n"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_The $EXTENDED_ARCHIVE_NAME will be copyed to:$\r$\n"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_$GIS_DATABASE\$CUSTOM_UNTAR_FOLDER.$\r$\n"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_$\r$\n"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_Press OK to continue or Cancel to skip the download and complete the ${QGIS_BASE}"
	StrCpy $DOWNLOAD_MESSAGE_ "$DOWNLOAD_MESSAGE_ installation without the $EXTENDED_ARCHIVE_NAME data set.$\r$\n"
	
	MessageBox MB_OKCANCEL "$DOWNLOAD_MESSAGE_" IDOK download IDCANCEL cancel_download
	
	download:	
	SetShellVarContext current	
	InitPluginsDir
	NSISdl::download "$HTTP_PATH/$ARCHIVE_NAME" "$TEMP\$ARCHIVE_NAME"
	Pop $0
	StrCmp $0 "success" download_ok download_failed
		
	download_ok:	
	InitPluginsDir
	untgz::extract "-d" "$GIS_DATABASE" "$TEMP\$ARCHIVE_NAME"
	Pop $0
	StrCmp $0 "success" untar_ok untar_failed
	
	untar_ok:       
	Rename "$GIS_DATABASE\$ORIGINAL_UNTAR_FOLDER" "$GIS_DATABASE\$CUSTOM_UNTAR_FOLDER"
	Delete "$TEMP\$ARCHIVE_NAME"
	Goto end
	
	download_failed:
	DetailPrint "$0" ;print error message to log
	MessageBox MB_OK "Download Failed.$\r$\n${QGIS_BASE} will be installed without the $EXTENDED_ARCHIVE_NAME sample data set."
	Goto end
	
	cancel_download:
	MessageBox MB_OK "Download Cancelled.$\r$\n${QGIS_BASE} will be installed without the $EXTENDED_ARCHIVE_NAME sample data set."
	Goto end
	
	untar_failed:
	DetailPrint "$0" ;print error message to log
	
	end:

FunctionEnd

Section /O "North Carolina Data Set" SecNorthCarolinaSDB

	;Set the size (in KB)  of the archive file
	StrCpy $ARCHIVE_SIZE_KB 138629
	
	;Set the size (in KB) of the unpacked archive file
	AddSize 293314
  
  	StrCpy $HTTP_PATH "http://grass.osgeo.org/sampledata"
	StrCpy $ARCHIVE_NAME "nc_spm_latest.tar.gz"
	StrCpy $EXTENDED_ARCHIVE_NAME "North Carolina"
	StrCpy $ORIGINAL_UNTAR_FOLDER "nc_spm_08"
	StrCpy $CUSTOM_UNTAR_FOLDER "North-Carolina"
	
	Call DownloadDataSet	
	
SectionEnd

Section /O "South Dakota (Spearfish) Data Set" SecSpearfishSDB

	;Set the size (in KB)  of the archive file
	StrCpy $ARCHIVE_SIZE_KB 20803
	
	;Set the size (in KB) of the unpacked archive file
	AddSize 42171
	
	StrCpy $HTTP_PATH "http://grass.osgeo.org/sampledata"
	StrCpy $ARCHIVE_NAME "spearfish_grass60data-0.3.tar.gz"
	StrCpy $EXTENDED_ARCHIVE_NAME "South Dakota (Spearfish)"
	StrCpy $ORIGINAL_UNTAR_FOLDER "spearfish60"
	StrCpy $CUSTOM_UNTAR_FOLDER "Spearfish60"
	
	Call DownloadDataSet

SectionEnd

Section /O "Alaska Data Set" SecAlaskaSDB

	;Set the size (in KB)  of the archive file
	StrCpy $ARCHIVE_SIZE_KB 10253
	
	;Set the size (in KB) of the unpacked archive file
	AddSize 33914
	
	StrCpy $HTTP_PATH "http://download.osgeo.org/qgis/data"
	StrCpy $ARCHIVE_NAME "qgis_sample_data.tar.gz"
	StrCpy $EXTENDED_ARCHIVE_NAME "Alaska"
	StrCpy $ORIGINAL_UNTAR_FOLDER "qgis_sample_data"
	StrCpy $CUSTOM_UNTAR_FOLDER "Alaska"
	
	Call DownloadDataSet

SectionEnd

;----------------------------------------------------------------------------------------------------------------------------

;Uninstaller Section

Section "Uninstall"

!if ${INSTALLER_TYPE} == "OSGeo4W"
	GetFullPathName /SHORT $0 $INSTDIR
	System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("OSGEO4W_ROOT", "$0").r0'
	System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("OSGEO4W_STARTMENU", "$SMPROGRAMS\${QGIS_BASE}").r0'

	ReadEnvStr $0 COMSPEC
	nsExec::ExecToLog '"$0" /c "$INSTALL_DIR\preremove.bat"'

	Delete "$INSTDIR\Uninstall-QGIS.exe"
	Delete "$INSTDIR\postinstall.bat.done"
	Delete "$INSTDIR\postinstall.bat"
	Delete "$INSTDIR\postinstall.log"

	Delete "$INSTDIR\preremove.bat.done"
	Delete "$INSTDIR\preremove.bat"
	Delete "$INSTDIR\preremove.log"

	RMDir /r "$INSTDIR\bin"
	RMDir /r "$INSTDIR\apps"
	RMDir /r "$INSTDIR\etc"
	RMDir /r "$INSTDIR\include"
	RMDir /r "$INSTDIR\lib"
	RMDir /r "$INSTDIR\share"
	RMDir /r "$INSTDIR\icons"
!else
	;remove files
	Delete "$INSTDIR\Uninstall-QGIS.exe"
	
	Delete "$INSTDIR\qgis.exe"
	Delete "$INSTDIR\qgis_help.exe"
	
	Delete "$INSTDIR\avcexport.exe"
	Delete "$INSTDIR\avcimport.exe"
	Delete "$INSTDIR\e00conv.exe"
	Delete "$INSTDIR\gpsbabel.exe"
	
	Delete "$INSTDIR\QGIS-WebSite.url"
		
	Delete "$INSTDIR\*.dll"	
	Delete "$INSTDIR\*.csv"	
	
	Delete "$INSTDIR\icons\QGIS.ico"
	Delete "$INSTDIR\icons\QGIS_Web.ico"

	Delete "$INSTDIR\MRSID_README.txt"
	Delete "$INSTDIR\run.bat"
	
	;remove folders
	RMDir /r "$INSTDIR\bin"
	RMDir /r "$INSTDIR\doc"
	RMDir /r "$INSTDIR\grass"
	RMDir /r "$INSTDIR\i18n"
	RMDir /r "$INSTDIR\icons"
	RMDir /r "$INSTDIR\images"
	RMDir /r "$INSTDIR\include"
	RMDir /r "$INSTDIR\lib"
	RMDir /r "$INSTDIR\msys"
	RMDir /r "$INSTDIR\plugins"
	RMDir /r "$INSTDIR\python"
	RMDir /r "$INSTDIR\resources"
	RMDir /r "$INSTDIR\svg"
	RMDir /r "$INSTDIR\themes"
	RMDir /r "$INSTDIR\proj"
	RMDir /r "$INSTDIR\epsg_csv"
!endif
	
	;if empty, remove the install folder
	RMDir "$INSTDIR"
	
	;remove the Desktop ShortCut
	SetShellVarContext current
	Delete "$DESKTOP\${QGIS_BASE}.lnk"
	
	;remove the Programs Start ShortCut
	SetShellVarContext all
	RMDir /r "$SMPROGRAMS\${QGIS_BASE}"

	;remove the Registry Entries
	DeleteRegKey HKLM "Software\${QGIS_BASE}"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${QGIS_BASE}"
SectionEnd

;----------------------------------------------------------------------------------------------------------------------------

;Installer Section Descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SecQGIS} "Install ${QGIS_BASE}"
	!insertmacro MUI_DESCRIPTION_TEXT ${SecNorthCarolinaSDB} "Download and install the North Carolina sample data set"
	!insertmacro MUI_DESCRIPTION_TEXT ${SecSpearfishSDB} "Download and install the South Dakota (Spearfish) sample data set"
	!insertmacro MUI_DESCRIPTION_TEXT ${SecAlaskaSDB} "Download and install the Alaska sample database (shapefiles and TIFF data)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;----------------------------------------------------------------------------------------------------------------------------
