####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################
TEMPLATE = lib   #or app for a standalone exe
INCLUDEPATH += . ..\..\qgis_win32\src %GDAL%\include
LIBS += ..\..\qgis_win32\src\libqgis.lib $(GDAL)\lib\gdal_i.lib
CONFIG += qt dll thread
DLLDESTDIR= ..\..\qgis_win32\win_build\lib\qgis

# Input
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
SOURCES += list.cpp \
           main.cpp \
           occurrences_file.cpp \
           openmodellergui.cpp \
           request_file.cpp
