########################################################
# Files

set (BENCH_SRCS
     main.cpp
     qgsbench.cpp
)

########################################################
# Build

add_executable (qgis_bench MACOSX_BUNDLE WIN32 ${BENCH_SRCS} )

include_directories(
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/labeling
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology

  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/core
)
include_directories(SYSTEM
  ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(qgis_bench
  qgis_core
  ${SQLITE3_LIBRARY}
  ${Qt5Core_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Xml_LIBRARIES}
  ${OPTIONAL_QTWEBKIT}
  ${Qt5Test_LIBRARIES}
)

if(APPLE)
  set_target_properties(qgis_bench PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH true
  )
endif()

########################################################
# Install

install (TARGETS qgis_bench
  BUNDLE DESTINATION ${QGIS_BIN_DIR}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
)
if (APPLE)
  install (CODE "execute_process(COMMAND ln -sfn ../../../${QGIS_FW_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Frameworks\")")
  install (CODE "execute_process(COMMAND ln -sfn ../../../../${QGIS_LIB_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/MacOS/lib\")")
  install (CODE "execute_process(COMMAND ln -sfn ../../../${QGIS_PLUGIN_SUBDIR}/.. \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Plugins\")")
  install (CODE "execute_process(COMMAND ln -sfn ../../../${QGIS_DATA_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/Resources\")")
  # may not exist, unless BundleUtilities are used
  install (CODE "execute_process(COMMAND ln -sfn ../../../../share \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_bench.app/Contents/MacOS/share\")")

endif()

