####################################################################
# Qmake project file for QGIS top-level plugins directory
# This file is used by qmake to generate the Makefiles for building
# QGIS plugins on Windows
#
# plugins.pro,v 1.4 2004/06/23 04:15:54 gsherman Exp
####################################################################

TEMPLATE = subdirs
SUBDIRS =  copyright_label \
           delimited_text \
           grid_maker \
           north_arrow \
           scale_bar
