mkdir build
cd build

CMAKE_OPTS = -DWITH_SERVER=ON -DWITH_STAGED_PLUGINS=OFF -DWITH_GRASS=OFF \
          -DSUPPRESS_QT_WARNINGS=ON -DENABLE_MODELTEST=ON -DENABLE_PGTEST=ON \
          -DWITH_QWTPOLAR=OFF -DWITH_APIDOC=ON -DWITH_PYSPATIALITE=ON

if [${QT} == 5]; then
  # Build QWT
  pushd qwt/qwt
  qmake-qt5
  make -j2
  popd

  #Build QScintilla
  pushd QScintilla-gpl-2.9/Qt4Qt5/
  qmake-qt5
  make -j2
  popd

  CMAKE_OPTS = ${CMAKE_OPTS} \
    -D QSCINTILLA_INCLUDE_DIR:PATH=${TRAVIS_BUILD_DIR}/QScintilla-gpl-2.9/Qt4Qt5 \
    -D QSCINTILLA_LIBRARY:FILEPATH=${TRAVIS_BUILD_DIR}/QScintilla-gpl-2.9/Qt4Qt5/libqscintilla2.so \
    -D QWT_INCLUDE_DIR:PATH=${TRAVIS_BUILD_DIR}/qwt/qwt/src \
    -D QWT_LIBRARY:FILEPATH=${TRAVIS_BUILD_DIR}/qwt/qwt/lib/libqwt.so \
    -D WITH_QT5=ON \
    -D WITH_BINDINGS=OFF
fi

cmake ${CMAKE_OPTS} ..
