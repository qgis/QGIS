mkdir build
cd build

ln -s ${HOME}/osgeo4travis/bin/ccache ${HOME}/osgeo4travis/bin/clang++-${LLVM_VERSION}
ln -s ${HOME}/osgeo4travis/bin/ccache ${HOME}/osgeo4travis/bin/clang-${LLVM_VERSION}

ccache -s

export CXX="clang++-${LLVM_VERSION}"
export CC="clang-${LLVM_VERSION}"
export PATH=${HOME}/osgeo4travis/bin:${PATH}

cmake --version
${CC} --version
${CXX} --version

CLANG_WARNINGS="-Wimplicit-fallthrough"

cmake -DWITH_SERVER=ON \
      -DCMAKE_PREFIX_PATH=/home/travis/osgeo4travis \
      -DWITH_STAGED_PLUGINS=ON \
      -DWITH_GRASS=ON \
      -DSUPPRESS_QT_WARNINGS=ON \
      -DENABLE_MODELTEST=ON \
      -DENABLE_PGTEST=ON \
      -DWITH_QSPATIALITE=ON \
      -DWITH_QWTPOLAR=OFF \
      -DWITH_APIDOC=ON \
      -DWITH_ASTYLE=ON \
      -DWITH_PYSPATIALITE=ON \
      -DGRASS_PREFIX7=/usr/lib/grass70 \
      -DGRASS_INCLUDE_DIR7=/usr/lib/grass70/include \
      -DCXX_EXTRA_FLAGS="$CLANG_WARNINGS" \
      ..
