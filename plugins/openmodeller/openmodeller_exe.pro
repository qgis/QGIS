####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
# $Id$
####################################################################

TEMPLATE = app
TARGET = omgui 

# config for standalone mode
#CONFIG += qt rtti thread release
CONFIG += qt rtti thread debug console

DEFINES+=_WINDOWS
DEFINES+=CORE_DLL_IMPORT 

#inc path for standalone app:
INCLUDEPATH += . 
INCLUDEPATH += $(GDAL)\include
INCLUDEPATH += $(OM_HOME)\src\inc 
INCLUDEPATH += $(OM_HOME)\src\inc\serialization
INCLUDEPATH += $(OM_HOME)\console

#libs for standalone mode
LIBS += $(GDAL)\lib\gdal_i.lib

contains( CONFIG, debug ) { 
  LIBS += $(OM_HOME)\lib\debug\libopenmodeller.lib 
} else {
  LIBS += $(OM_HOME)\src\lib\libopenmodeller.lib 
}


#used by both plugin and exe
HEADERS += list.hh \
           occurrences_file.hh \
           openmodellergui.h \
           openmodellerguibase.ui.h \
           request_file.hh

INTERFACES += openmodellerguibase.ui

SOURCES += main.cpp \ 
           list.cpp \
           occurrences_file.cpp \
           openmodellergui.cpp \
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
