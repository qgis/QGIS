# Microsoft Developer Studio Project File - Name="cdp_wizard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=cdp_wizard - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cdp_wizard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cdp_wizard.mak" CFG="cdp_wizard - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cdp_wizard - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "cdp_wizard - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cdp_wizard - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GX /O1 /I "." /I "$(QTDIR)\include" /I "C:\dev\cpp\plugins\cdp_wizard" /I "C:\Qt\3.2.1NonCommercial\mkspecs\win32-msvc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "QT_NO_DEBUG" /FD -Zm200 /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 "qt-mtnc321.lib" "qtmain.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "opengl32.lib" "glu32.lib" "delayimp.lib" delayimp.lib /nologo /subsystem:windows /machine:IX86 /libpath:"$(QTDIR)\lib" /DELAYLOAD:comdlg32.dll /DELAYLOAD:oleaut32.dll /DELAYLOAD:winmm.dll /DELAYLOAD:wsock32.dll /DELAYLOAD:winspool.dll

!ELSEIF  "$(CFG)" == "cdp_wizard - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I "$(QTDIR)\include" /I "C:\dev\cpp\plugins\cdp_wizard" /I "C:\Qt\3.2.1NonCommercial\mkspecs\win32-msvc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /FD /GZ -Zm200 /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 "qt-mtnc321.lib" "qtmain.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "opengl32.lib" "glu32.lib" "delayimp.lib" /nologo /subsystem:windows /debug /machine:IX86 /pdbtype:sept /libpath:"$(QTDIR)\lib"

!ENDIF 

# Begin Target

# Name "cdp_wizard - Win32 Release"
# Name "cdp_wizard - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=cdpwizard.cpp
# End Source File
# Begin Source File

SOURCE=cdpwizardbase.ui.h
# End Source File
# Begin Source File

SOURCE=climatedataprocessor.cpp
# End Source File
# Begin Source File

SOURCE=dataprocessor.cpp
# End Source File
# Begin Source File

SOURCE=filegroup.cpp
# End Source File
# Begin Source File

SOURCE=filereader.cpp
# End Source File
# Begin Source File

SOURCE=filewriter.cpp
# End Source File
# Begin Source File

SOURCE=main.cpp
# End Source File
# Begin Source File

SOURCE=plugin.cpp

!IF  "$(CFG)" == "cdp_wizard - Win32 Release"

!ELSEIF  "$(CFG)" == "cdp_wizard - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=cdpwizard.h

!IF  "$(CFG)" == "cdp_wizard - Win32 Release"

USERDEP__CDPWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing cdpwizard.h...
InputPath=cdpwizard.h

"moc_cdpwizard.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc cdpwizard.h -o moc_cdpwizard.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "cdp_wizard - Win32 Debug"

USERDEP__CDPWI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing cdpwizard.h...
InputPath=cdpwizard.h

"moc_cdpwizard.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc cdpwizard.h -o moc_cdpwizard.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cdpwizardbase.ui.h
# End Source File
# Begin Source File

SOURCE=climatedataprocessor.h
# End Source File
# Begin Source File

SOURCE=dataprocessor.h
# End Source File
# Begin Source File

SOURCE=filegroup.h
# End Source File
# Begin Source File

SOURCE=filereader.h
# End Source File
# Begin Source File

SOURCE=filewriter.h
# End Source File
# Begin Source File

SOURCE=plugin.h

!IF  "$(CFG)" == "cdp_wizard - Win32 Release"

USERDEP__PLUGI="$(QTDIR)\bin\moc.exe"	
# Begin Custom Build - Moc'ing plugin.h...
InputPath=plugin.h

"moc_plugin.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc plugin.h -o moc_plugin.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "cdp_wizard - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Forms"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=cdpwizardbase.ui

!IF  "$(CFG)" == "cdp_wizard - Win32 Release"

USERDEP__CDPWIZ="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing cdpwizardbase.ui...
InputPath=cdpwizardbase.ui

BuildCmds= \
	$(QTDIR)\bin\uic cdpwizardbase.ui -o cdpwizardbase.h \
	$(QTDIR)\bin\uic cdpwizardbase.ui -i cdpwizardbase.h -o cdpwizardbase.cpp \
	$(QTDIR)\bin\moc cdpwizardbase.h -o moc_cdpwizardbase.cpp \
	

"cdpwizardbase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"cdpwizardbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_cdpwizardbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "cdp_wizard - Win32 Debug"

USERDEP__CDPWIZ="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing cdpwizardbase.ui...
InputPath=cdpwizardbase.ui

BuildCmds= \
	$(QTDIR)\bin\uic cdpwizardbase.ui -o cdpwizardbase.h \
	$(QTDIR)\bin\uic cdpwizardbase.ui -i cdpwizardbase.h -o cdpwizardbase.cpp \
	$(QTDIR)\bin\moc cdpwizardbase.h -o moc_cdpwizardbase.cpp \
	

"cdpwizardbase.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"cdpwizardbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"moc_cdpwizardbase.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Source File

SOURCE=cdpwizardbase.cpp
# End Source File
# Begin Source File

SOURCE=cdpwizardbase.h
# End Source File
# Begin Source File

SOURCE=moc_cdpwizard.cpp

!IF  "$(CFG)" == "cdp_wizard - Win32 Release"

!ELSEIF  "$(CFG)" == "cdp_wizard - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=moc_cdpwizardbase.cpp
# End Source File
# Begin Source File

SOURCE=moc_plugin.cpp

!IF  "$(CFG)" == "cdp_wizard - Win32 Release"

!ELSEIF  "$(CFG)" == "cdp_wizard - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
