########################################################
# Files

set (BENCH_SRCS
     main.cpp
     qgsbench.cpp
)

########################################################
# Build

add_executable (qgis_bench MACOSX_BUNDLE WIN32 ${BENCH_SRCS} )

# require c++17
target_compile_features(qgis_bench PRIVATE cxx_std_17)

include_directories(


  ${CMAKE_BINARY_DIR}
)
include_directories(SYSTEM
  ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(qgis_bench
  qgis_core
  ${SQLITE3_LIBRARY}
  ${QT_VERSION_BASE}::Core
  ${QT_VERSION_BASE}::Network
  ${QT_VERSION_BASE}::Svg
  ${QT_VERSION_BASE}::Xml
  ${OPTIONAL_QTWEBKIT}
  ${QT_VERSION_BASE}::Test
)

if(APPLE)
  target_link_libraries(qgis_bench qgis_native)

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

