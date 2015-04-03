########################################################
# Files

SET (BENCH_SRCS
     main.cpp
     qgsbench.cpp
)

SET (BENCH_MOC_HDRS
     qgsbench.h
)

########################################################
# Build

QT4_WRAP_CPP (BENCH_MOC_SRCS  ${BENCH_MOC_HDRS})

ADD_EXECUTABLE (qgis_bench MACOSX_BUNDLE WIN32 ${BENCH_SRCS} ${BENCH_MOC_SRCS} )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/raster
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GEOS_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(qgis_bench
  qgis_core
  ${SQLITE3_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  ${QT_QTMAIN_LIBRARY}
  ${QT_QTTEST_LIBRARY}
)

IF(APPLE)
  SET_TARGET_PROPERTIES(qgis_bench PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH true
  )
ENDIF(APPLE)

########################################################
# Install

INSTALL (TARGETS qgis_bench
  BUNDLE DESTINATION ${QGIS_BIN_DIR}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
)
IF (APPLE)
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../${QGIS_FW_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Frameworks\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../../${QGIS_LIB_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/MacOS/lib\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../${QGIS_PLUGIN_SUBDIR}/.. \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Plugins\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../${QGIS_DATA_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Resources\")")
  # may not exist, unless BundleUtilities are used
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../../share \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/MacOS/share\")")

ENDIF (APPLE)

