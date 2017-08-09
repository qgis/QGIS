###########################################################################
#    install.sh
#    ---------------------
#    Date                 : March 2016
#    Copyright            : (C) 2016 by Matthias Kuhn
#    Email                : matthias at opengis dot ch
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

ln -s ${HOME}/osgeo4travis/bin/ccache ${HOME}/osgeo4travis/bin/clang++-${LLVM_VERSION}
ln -s ${HOME}/osgeo4travis/bin/ccache ${HOME}/osgeo4travis/bin/clang-${LLVM_VERSION}

ccache -s
ccache -z

export CXX="clang++-${LLVM_VERSION}"
export CC="clang-${LLVM_VERSION}"
#export CXX="g++-6"
#export CC="gcc-6"
export PATH=${HOME}/osgeo4travis/bin:${PATH}
export PYTHONPATH=${HOME}/osgeo4travis/lib/python3.3/site-packages/

cmake --version
${CC} --version
${CXX} --version

# CLANG_WARNINGS="-Wimplicit-fallthrough"
CLANG_WARNINGS=""

# Include this line for debug reasons
#      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
#
CMAKE_FLAGS="
      -DCMAKE_PREFIX_PATH=/home/travis/osgeo4travis
      -DWITH_STAGED_PLUGINS=ON
      -DWITH_GRASS=ON
      -DWITH_GRASS7=ON
      -DGRASS_PREFIX7=/home/travis/osgeo4travis/grass-7.0.4
      -DSUPPRESS_QT_WARNINGS=ON
      -DENABLE_MODELTEST=ON
      -DENABLE_PGTEST=ON
      -DWITH_QSPATIALITE=ON
      -DWITH_QWTPOLAR=OFF
      -DWITH_APIDOC=OFF
      -DWITH_ASTYLE=OFF
      -DDISABLE_DEPRECATED=ON
      -DCXX_EXTRA_FLAGS=${CLANG_WARNINGS}
      "

# The following options trigger a minimalized build to
# reduce the travis build time so we don't time out and
# have a chance of slowly filling the ccache.
if [ "$CACHE_WARMING" = true ] ; then
  CMAKE_FLAGS="
    ${CMAKE_FLAGS}
    -DWITH_DESKTOP=OFF
    -DWITH_SERVER=OFF
    -DWITH_CUSTOM_WIDGETS=OFF
  "
else
  CMAKE_FLAGS="
    ${CMAKE_FLAGS}
    -DWITH_DESKTOP=ON
    -DWITH_SERVER=ON
    -DWITH_CUSTOM_WIDGETS=ON
  "
fi

cmake $CMAKE_FLAGS ..
