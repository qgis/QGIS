####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################
TEMPLATE = app  # to build as a standalone app
#TEMPLATE = lib   # to build as a dll

TARGET = omgui #will produce omgui.dll or omgui.exe depending on standalone or dll mode

#inc path for standalone app:
INCLUDEPATH += . %GDAL%\include 
#inc path for qgis plugin
#INCLUDEPATH += . %GDAL%\include ..\..\qgis_win32\plugins ..\..\qgis_win32\src


#libs for standalone mode
LIBS += $(GDAL)\lib\gdal_i.lib libopenmodeller_static.lib 
#libs for dll
#LIBS += $(GDAL)\lib\gdal_i.lib libopenmodeller_static.lib ..\..\qgis_win32\src\libqgis.lib
#LIBS += libopenmodeller_static.lib ..\..\qgis_win32\src\libqgis.lib

# config for standalone mode
 CONFIG += qt thread rtti #debug console
# config for dll
#CONFIG += qt dll thread rtti  #debug console


#qgis plugin mode
#HEADERS += plugin.h 
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

#exe mode
SOURCES += main.cpp 
#plugin mode
#SOURCES += plugin.cpp 
#used by both
SOURCES += list.cpp \
           occurrences_file.cpp \
           openmodellergui.cpp \
           request_file.cpp 
