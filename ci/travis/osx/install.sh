###########################################################################
#    install.sh
#    ---------------------
#    Date                 : August 2015
#    Copyright            : (C) 2015 by Nyall Dawson
#    Email                : nyall dot dawson at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

mkdir build
cd build
#no PGTEST for OSX - can't get postgres to start with brew install
#no APIDOC for OSX - doxygen tests and warnings are covered by linux build
#no deprecated-declarations warnings... requires QGIS ported to Cocoa
cmake \
  -DWITH_STAGED_PLUGINS=ON \
  -DWITH_GRASS=OFF \
  -DSUPPRESS_SIP_WARNINGS=ON \
  -DSUPPRESS_QT_WARNINGS=ON \
  -DENABLE_MODELTEST=ON \
  -DENABLE_PGTEST=OFF \
  -DWITH_QWTPOLAR=OFF \
  -DWITH_PYSPATIALITE=ON \
  -DQWT_INCLUDE_DIR=/usr/local/opt/qwt/lib/qwt.framework/Headers/ \
  -DQWT_LIBRARY=/usr/local/opt/qwt/lib/qwt.framework/qwt \
  -DGDAL_CONFIG=/usr/local/opt/gdal2/bin/gdal-config \
  -DGRASS_PREFIX7=/usr/local/opt/grass-70/grass-7.0.4 \
  -DCMAKE_CXX_COMPILER="/usr/local/opt/ccache/libexec/clang++" \
  -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations" \
  -DCMAKE_PREFIX_PATH='/usr/local/opt/qt5;/usr/local/opt/qt5-webkit;/usr/local/opt/gdal2;/usr/local/opt/expat;/usr/local/opt/sqlite;/usr/local/opt/flex;/usr/local/opt/bison' \
  -DWITH_DESKTOP=ON \
  -DWITH_SERVER=ON \
  -DWITH_APIDOC=OFF \
  ..

/usr/local/opt/ccache/bin/ccache -s
