########################################################
# Files

set (BENCH_SRCS
     main.cpp
     qgsbench.cpp
)

########################################################
# Build

add_executable (qgis_bench WIN32 ${BENCH_SRCS} )

# require c++17
target_compile_features(qgis_bench PRIVATE cxx_std_17)

include_directories(


  ${CMAKE_BINARY_DIR}
)

target_link_libraries(qgis_bench
  qgis_core
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
