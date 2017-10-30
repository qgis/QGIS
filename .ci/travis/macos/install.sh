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
pushd build

export PATH=/usr/local/opt/ccache/libexec:$PATH
HB=$(brew --prefix)

# force looking in HB/opt paths first, so headers in HB/include are not found first
prefixes="qt5
qt5-webkit
qscintilla2
qwt
qwtpolar
qca
qtkeychain
gdal2
gsl
geos
proj
libspatialite
spatialindex
fcgi
expat
sqlite
flex
bison
libzip"

full_prefixes=""
for p in ${prefixes}; do
  full_prefixes+="${HB}/opt/${p};"
done

#no PGTEST for OSX - can't get postgres to start with brew install
#no APIDOC for OSX - doxygen tests and warnings are covered by linux build
#no deprecated-declarations warnings... requires QGIS ported to Cocoa
cmake \
  -G 'Ninja' \
  -DCMAKE_FIND_FRAMEWORK:STRING=LAST \
  -DCMAKE_PREFIX_PATH:STRING=${full_prefixes} \
  -DWITH_SERVER=OFF \
  -DWITH_DESKTOP=OFF \
  -DWITH_STAGED_PLUGINS=ON \
  -DENABLE_MODELTEST=ON \
  -DENABLE_PGTEST=OFF \
  -DWITH_QWTPOLAR=OFF \
  -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations" \
  ..

popd
