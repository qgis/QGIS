####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################
TEMPLATE = lib   # to build as a dll

TARGET = omgui #will produce omgui.dll 

#inc path for qgis plugin
INCLUDEPATH += . %GDAL%\include ..\..\qgis_win32\plugins ..\..\qgis_win32\src


#libs for dll
#LIBS += $(GDAL)\lib\gdal_i.lib libopenmodeller_static.lib ..\..\qgis_win32\src\libqgis.lib
LIBS += libopenmodeller_static.lib ..\..\qgis_win32\src\libqgis.lib

# config for dll
CONFIG += qt dll thread rtti 


#qgis plugin mode
HEADERS += plugin.h 
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

#plugin mode
SOURCES += plugin.cpp 
SOURCES += list.cpp \
           occurrences_file.cpp \
           openmodellergui.cpp \
           request_file.cpp 
