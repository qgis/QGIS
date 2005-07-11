####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################

TEMPLATE = lib   # to build as a dll

# config for dll
#CONFIG += qt dll thread rtti #release version without debug symbols
CONFIG += qt dll thread rtti debug console #debug version

# place debug temp build objects somewhere else to avoid conflict with release build
OBJECTS_DIR = build_debug\tmp

#inc path for qgis plugin
INCLUDEPATH += . 
INCLUDEPATH += $(QGIS)\plugins 
INCLUDEPATH += $(QGIS)\src
INCLUDEPATH += $(GDAL)\include
INCLUDEPATH += $(OM_HOME)\src
INCLUDEPATH += $(OM_HOME)\src\openmodeller
INCLUDEPATH += $(OM_HOME)\console


#libs for dll
LIBS += $(QGIS)\src\libqgis.lib 
LIBS += $(GDAL)\lib\gdal_i.lib 
LIBS += $(EXPAT_HOME)\Libs\libexpat.lib 


contains( CONFIG, debug ){ 
  DESTDIR = build_debug
  LIBS += $(OM_HOME)\windows\vc7\build_debug\libopenmodeller_debug.lib 
  TARGET = omgui_debug #will produce omgui_debug.dll 
}else{
  DESTDIR = build
  LIBS += $(OM_HOME)\windows\vc7\build\libopenmodeller.lib 
  TARGET = omgui #will produce omgui.dll 
}


DEFINES+=WIN32
DEFINES+=_WINDOWS
DEFINES+=CORE_DLL_IMPORT 
DEFINES+=_DEBUG

#qgis plugin mode
HEADERS += plugin.h 
HEADERS += list.hh \
           occurrences_file.hh \
           request_file.hh \
           imagewriter.h \
           layerselector.h \
           openmodellergui.h \
           openmodellerguibase.ui.h 
           
INTERFACES += openmodellerguibase.ui layerselectorbase.ui omguireportbase.ui

#plugin mode
SOURCES += plugin.cpp 
SOURCES += list.cpp \
           occurrences_file.cpp \
           openmodellergui.cpp \
           imagewriter.cpp \
           layerselector.cpp \
           request_file.cpp \
           file_parser.cpp

# -------------------------------------------
# check for QGIS include and lib files
# -------------------------------------------
!exists( $$(QGIS)\src\qgis.h ) {  
    message( "Could not find QGIS include files." )  
    message( "Check whether the QGIS env. variable is set correctly. ")  
    message( "Current value: QGIS=$$(QGIS)" )  
    error  ( "QGIS include files are missing." )
}

!exists( $$(QGIS)\src\libqgis.lib ) {  
    message( "Could not find QGIS library file." )  
    message( "Check whether the QGIS env. variable is set correctly. ")  
    message( "Current value: QGIS=$$(QGIS)" )  
    error  ( "QGIS library file is missing." )
}

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

!exists( $$(OM_HOME)\windows\vc7\build\libopenmodeller.lib ) {  
  message( "Could not find OpenModeller library file." )  
  message( "Check whether the OM_HOME env. variable is set correctly. ")  
  message( "Current value: OM_HOME=$$(OM_HOME)" )  
  error  ( "OpenModeller library file is missing." )
}
# -------------------------------------------
