mkdir build
cd build
#no PGTEST for OSX - can't get postgres to start with brew install
#no APIDOC for OSX - doxygen tests and warnings are covered by linux build
#no deprecated-declarations warnings... requires QGIS ported to Cocoa
cmake -DWITH_SERVER=ON -DWITH_STAGED_PLUGINS=OFF -DWITH_GRASS=OFF \
          -DSUPPRESS_QT_WARNINGS=ON -DENABLE_MODELTEST=ON -DENABLE_PGTEST=OFF \
          -DWITH_QWTPOLAR=OFF -DWITH_PYSPATIALITE=ON \
          -DQWT_INCLUDE_DIR=/usr/local/opt/qwt/lib/qwt.framework/Headers/ \
          -DQWT_LIBRARY=/usr/local/opt/qwt/lib/qwt.framework/qwt \
          -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations" ..
