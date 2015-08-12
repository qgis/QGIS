mkdir build
cd build
#no PGTEST for OSX - can't get postgres to start with brew install
cmake -DWITH_SERVER=ON -DWITH_STAGED_PLUGINS=OFF -DWITH_GRASS=OFF \
          -DSUPPRESS_QT_WARNINGS=ON -DENABLE_MODELTEST=ON -DENABLE_PGTEST=OFF \
          -DWITH_QWTPOLAR=OFF -DWITH_APIDOC=ON -DWITH_PYSPATIALITE=ON \
          -DQWT_INCLUDE_DIR=/usr/local/opt/qwt/lib/qwt.framework/Headers/ \
          -DQWT_LIBRARY=/usr/local/opt/qwt/lib/qwt.framework/qwt ..
