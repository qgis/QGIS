########################################################
# Files

SET (BENCH_SRCS
     main.cpp
     qgsbench.cpp
)

########################################################
# Build

ADD_EXECUTABLE (qgis_bench MACOSX_BUNDLE WIN32 ${BENCH_SRCS} )

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology

  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/core
)
INCLUDE_DIRECTORIES(SYSTEM
  ${SQLITE3_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(qgis_bench
  qgis_core
  ${SQLITE3_LIBRARY}
  ${Qt5Core_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Xml_LIBRARIES}
  ${OPTIONAL_QTWEBKIT}
  ${Qt5Test_LIBRARIES}
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
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfn ../../../${QGIS_FW_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Frameworks\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfn ../../../../${QGIS_LIB_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/MacOS/lib\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfn ../../../${QGIS_PLUGIN_SUBDIR}/.. \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Plugins\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfn ../../../${QGIS_DATA_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Resources\")")
  # may not exist, unless BundleUtilities are used
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfn ../../../../share \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/MacOS/share\")")

ENDIF (APPLE)

