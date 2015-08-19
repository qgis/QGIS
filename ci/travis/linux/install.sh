mkdir build

CMAKE_OPTS="-DWITH_SERVER=ON -DWITH_STAGED_PLUGINS=OFF -DWITH_GRASS=OFF
            -DSUPPRESS_QT_WARNINGS=ON -DENABLE_MODELTEST=ON -DENABLE_PGTEST=ON
            -DWITH_QWTPOLAR=OFF -DWITH_PYSPATIALITE=ON"

if [ ${QT} == 5 ]; then
  # Build QWT
  pushd qwt/qwt
  qmake
  make -j2
  popd

  #Build QScintilla
  pushd QScintilla-gpl-2.9/Qt4Qt5/
  qmake
  make -j2
  popd

  CMAKE_OPTS="${CMAKE_OPTS}
    -DQSCINTILLA_INCLUDE_DIR:PATH=${TRAVIS_BUILD_DIR}/QScintilla-gpl-2.9/Qt4Qt5
    -DQSCINTILLA_LIBRARY:FILEPATH=${TRAVIS_BUILD_DIR}/QScintilla-gpl-2.9/Qt4Qt5/libqscintilla2.so
    -DQWT_INCLUDE_DIR:PATH=${TRAVIS_BUILD_DIR}/qwt/qwt/src
    -DQWT_LIBRARY:FILEPATH=${TRAVIS_BUILD_DIR}/qwt/qwt/lib/libqwt.so
    -DENABLE_QT5=ON
    -DWITH_BINDINGS=OFF"
else
  CMAKE_OPTS="${CMAKE_OPTS} -DWITH_APIDOC=ON"
fi

cd build
cmake ${CMAKE_OPTS} ..
