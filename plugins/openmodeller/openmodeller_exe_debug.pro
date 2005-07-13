####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
# $Id$#
###################################################################

TEMPLATE = app

# config for standalone mode
#CONFIG += qt rtti thread release
CONFIG += qt rtti thread debug console

# place debug temp build objects somewhere else to avoid conflict with release build
OBJECTS_DIR = build_debug\tmp

# for icon file under windows
# see http://www.qiliang.net/qt/appicon.html
#1

RC_FILE = omguistandalone.rc

DEFINES+=WIN32
DEFINES+=_WINDOWS
DEFINES+=CORE_DLL_IMPORT 
DEFINES+=_DEBUG

#inc path for standalone app:
INCLUDEPATH += . 
INCLUDEPATH += $(GDAL)\include
INCLUDEPATH += $(OM_HOME)\src 
INCLUDEPATH += $(OM_HOME)\src\openmodeller
INCLUDEPATH += $(OM_HOME)\console

#libs for standalone mode
LIBS += $(GDAL)\lib\gdal_i.lib

contains( CONFIG, debug ) {
	DESTDIR = build_debug
    LIBS += $(OM_HOME)\windows\vc7\build_debug\libopenmodeller_debug.lib   
    TARGET = omgui_debug_exe
} else {  
	DESTDIR = build
    LIBS += $(OM_HOME)\windows\vc7\build\libopenmodeller.lib   
    TARGET = omgui 
}

#used by both plugin and exe
HEADERS += list.hh \
           openmodellerguibase.ui.h \
           openmodellergui.h \
           omguimain.h \
           omguimainbase.ui.h \
           imagewriter.h \
           layerselector.h \
           request_file.hh \
           file_parser.hh
           
INTERFACES += openmodellerguibase.ui omguimainbase.ui layerselectorbase.ui omguireportbase.ui

SOURCES += main.cpp \ 
           list.cpp \
           occurrences_file.cpp \
           openmodellergui.cpp \
           omguimain.cpp \
           imagewriter.cpp \
           layerselector.cpp \
           request_file.cpp \
           file_parser.hh


# -------------------------------------------
# check for Expat lib files
# -------------------------------------------
!exists( $$(EXPAT_HOME)\Libs\libexpat.lib ) {  
    message( "Could not find Expat library file." )  
    message( "Check whether the EXPAT_HOME environment variable is set correctly. ")  
    message( "Current value: EXPAT_HOME=$$(EXPAT_HOME)" )  
    error  ( "Expat library file is missing." )
}

# -------------------------------------------
# check for GDAL include and lib files
# -------------------------------------------
!exists( $$(GDAL)\include\gdal.h ) {  
  message( "Could not find GDAL include files." )  
  message( "Check whether the GDAL environment variable is set correctly. ")  
  message( "Current value: GDAL=$$(GDAL)" )  
  error  ( "GDAL include files are missing." )
}
    
!exists( $$(GDAL)\lib\gdal_i.lib ) {  
  message( "Could not find GDAL library file." )  
  message( "Check whether the GDAL environment variable is set correctly. ")  
  message( "Current value: GDAL=$$(GDAL)" )  
  error  ( "GDAL library file is missing." )
}
    
# -------------------------------------------
# check for OM include and lib files
# -------------------------------------------
!exists( $$(OM_HOME)\src\openmodeller\OpenModeller.hh ) {  
  message( "Could not find OpenModeller include files." )  
  message( "Check whether the OM_HOME env. variable is set correctly. ")  
  message( "Current value: OM_HOME=$$(OM_HOME)" )  
  error  ( "OpenModeller include files are missing." )
}

!exists( $$(OM_HOME)\windows\vc7\build_debug\libopenmodeller_debug.lib ) {  
  message( "Could not find OpenModeller library file." )  
  message( "Check whether the OM_HOME env. variable is set correctly. ")  
  message( "Current value: OM_HOME=$$(OM_HOME)" )  
  error  ( "OpenModeller library file is missing." )
}
# -------------------------------------------
