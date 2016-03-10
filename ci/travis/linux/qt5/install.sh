mkdir build
cd build

ln -s /usr/bin/ccache ${HOME}/osgeo4travis/bin/clang++-3.6
ln -s /usr/bin/ccache ${HOME}/osgeo4travis/bin/clang-3.6

ccache -s

export CXX="clang++-3.6"
export CC="clang-3.6"
export PATH=${HOME}/osgeo4travis/bin:${PATH}
export PYTHONPATH=${HOME}/osgeo4travis/lib/python3/dist-packages/

cmake --version
${CC} --version
${CXX} --version

CLANG_WARNINGS="-Wimplicit-fallthrough"

# Include this line for debug reasons
#      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
#
# Server fails at the moment on Qt5 because QFtp has been removed
#      -DWITH_SERVER=ON \
cmake \
      -DCMAKE_PREFIX_PATH=/home/travis/osgeo4travis \
      -DWITH_STAGED_PLUGINS=ON \
      -DWITH_GRASS=ON \
      -DSUPPRESS_QT_WARNINGS=ON \
      -DENABLE_MODELTEST=ON \
      -DENABLE_PGTEST=ON \
      -DWITH_QWTPOLAR=OFF \
      -DWITH_QTWEBKIT=OFF \
      -DWITH_APIDOC=ON \
      -DWITH_ASTYLE=ON \
      -DENABLE_QT5=ON \
      -DCXX_EXTRA_FLAGS="$CLANG_WARNINGS" \
      -DPYTHON_LIBRARY=/usr/lib/libpython3.2mu.so \
      ..
