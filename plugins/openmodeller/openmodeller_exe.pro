####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################
TEMPLATE = app  
TARGET = omgui 
#inc path for standalone app:
INCLUDEPATH += . %GDAL%\include 

#libs for standalone mode
LIBS += $(GDAL)\lib\gdal_i.lib 
LIBS += libopenmodeller.lib 
LIBS += libexpatMT.lib 

# config for standalone mode
#CONFIG += qt rtti thread release
CONFIG += qt rtti thread debug console

DEFINES+=_WINDOWS
DEFINES+=CORE_DLL_IMPORT 

#used by both plugin and exe
HEADERS += algorithm_factory.hh \
           file_parser.hh \
           list.hh \
           map_format.hh \
           occurrence.hh \
           occurrences_file.hh \
           om.hh \
           om_alg_parameter.hh \
           om_algorithm.hh \
           om_algorithm_metadata.hh \
           om_control.hh \
           om_defs.hh \
           om_log.hh \
           om_occurrences.hh \
           om_sampled_data.hh \
           om_sampler.hh \
           openmodellergui.h \
           openmodellerguibase.ui.h \
           os_specific.hh \
           random.hh \
           request_file.hh
INTERFACES += openmodellerguibase.ui

SOURCES += main.cpp 
SOURCES += list.cpp \
           occurrences_file.cpp \
           openmodellergui.cpp \
           file_parser.cpp \
           request_file.cpp 
