mkdir build
cd build

export CXX="clang++-3.7"
export CC="clang-3.7"

cmake --version
${CC} --version

cmake -DWITH_SERVER=ON \
      -DWITH_STAGED_PLUGINS=ON \
      -DWITH_GRASS=ON \
      -DSUPPRESS_QT_WARNINGS=ON \
      -DENABLE_MODELTEST=ON \
      -DENABLE_PGTEST=ON \
      -DWITH_QWTPOLAR=OFF \
      -DWITH_APIDOC=ON \
      -DDOXYGEN_FILE_PATTERNS="*.h" \
      -DWITH_ASTYLE=ON \
      -DWITH_PYSPATIALITE=ON \
      -DGRASS_PREFIX7=/usr/lib/grass70 \
      -DGRASS_INCLUDE_DIR7=/usr/lib/grass70/include \
      ..
