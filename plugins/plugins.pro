####################################################################
# Qmake project file for QGIS top-level plugins directory
# This file is used by qmake to generate the Makefiles for building
# QGIS plugins on Windows
#
# plugins.pro,v 1.5 2004/08/16 18:37:46 gsherman Exp
####################################################################

TEMPLATE = subdirs
SUBDIRS =  copyright_label \
           delimited_text \
           grid_maker \
           north_arrow \
           scale_bar \
	   gps_importer
