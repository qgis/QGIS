# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

PREFIX=-DPREFIX=\"$(prefix)\"
INCLUDE_DIR=-DINCLUDE_DIR=\"${prefix}/include\"
LIB_DIR=-DLIB_DIR=\"${libdir}\"
BIN_DIR=-DBIN_DIR=\"${exec_prefix}/bin\"

## main app
bin_PROGRAMS = qgis_config

## ancillary library for plug-ins

qgis_config_SOURCES =  qgis_config.cpp
qgis_config_CXXFLAGS = $(PREFIX) $(INCLUDE_DIR) $(LIB_DIR) $(BIN_DIR) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(DEBUG_QGIS)