####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################
TEMPLATE = lib   # to build as a dll

# config for dll
CONFIG += qt dll thread rtti #release version without debug symbols
#CONFIG += qt dll thread rtti debug console #debug version

#inc path for qgis plugin
INCLUDEPATH += . 
INCLUDEPATH += ..\..\qgis_win32\plugins 
INCLUDEPATH += ..\..\qgis_win32\src
INCLUDEPATH += $(GDAL)\include
INCLUDEPATH += $(OM_HOME)\src\inc 
INCLUDEPATH += $(OM_HOME)\src\inc\serialization
INCLUDEPATH += $(OM_HOME)\console


#libs for dll
LIBS += ..\..\qgis_win32\src\libqgis.lib 
LIBS += $(GDAL)\lib\gdal_i.lib 
LIBS += libexpatMT.lib 


contains( CONFIG, debug ) { 
  LIBS += $(OM_HOME)\lib\debug\libopenmodeller.lib 
  TARGET = omgui_debug #will produce omgui_debug.dll 
} else {
  LIBS += $(OM_HOME)\lib\libopenmodeller.lib 
  TARGET = omgui #will produce omgui.dll 
}


DEFINES+=_WINDOWS
DEFINES+=CORE_DLL_IMPORT 

#qgis plugin mode
HEADERS += plugin.h 
HEADERS += list.hh \
           occurrences_file.hh \
           request_file.hh \
           imagewriter.h \
           layerselector.h \
           openmodellergui.h \
           openmodellerguibase.ui.h 
           
INTERFACES += openmodellerguibase.ui layerselectorbase.ui

#plugin mode
SOURCES += plugin.cpp 
SOURCES += list.cpp \
           occurrences_file.cpp \
           openmodellergui.cpp \
           imagewriter.cpp \
           layerselector.cpp \
           request_file.cpp 
           
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

!exists( $$(OM_HOME)\src\inc\om_control.hh ) {
  message( "Could not find OpenModeller include files." )
  message( "Check whether the OM_HOME env. variable is set correctly. ")
  message( "Current value: OM_HOME=$$(OM_HOME)" )
  error  ( "OpenModeller include files are missing." )
}

!exists( $$(OM_HOME)\lib\libopenmodeller.lib ) {
  message( "Could not find OpenModeller library file." )
  message( "Check whether the OM_HOME env. variable is set correctly. ")
  message( "Current value: OM_HOME=$$(OM_HOME)" )
  error  ( "OpenModeller library file is missing." )
}

# -------------------------------------------
