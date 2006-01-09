####################################################################
# Qmake project file for QGIS top-level plugins directory
# This file is used by qmake to generate the Makefiles for building
# QGIS plugins on Windows
#
# plugins.pro,v 1.6 2004/08/16 19:06:00 gsherman Exp
####################################################################

TEMPLATE = subdirs
SUBDIRS =  delimited_text \
           grid_maker \
           copyright_label \
           north_arrow \
      	   gps_importer \
           scale_bar \
           spit
#SUBDIRS =  copyright_label \
#           delimited_text \
#           grid_maker \
#           north_arrow \
#           scale_bar \
#      	    gps_importer \
#           spit
