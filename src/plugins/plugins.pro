####################################################################
# Qmake project file for QGIS top-level plugins directory
# This file is used by qmake to generate the Makefiles for building
# QGIS plugins on Windows
#
# plugins.pro,v 1.6 2004/08/16 19:06:00 gsherman Exp
####################################################################

TEMPLATE = subdirs
SUBDIRS =  delimited_text \
           scale_bar \
           grid_maker \
           north_arrow \
           grass \
           copyright_label \
           georeferencer \
           spit
#      	    gps_importer \
