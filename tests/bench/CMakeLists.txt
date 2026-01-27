########################################################
# Files

set (BENCH_SRCS
     main.cpp
     qgsbench.cpp
)

########################################################
# Build

qt_add_executable(qgis_bench WIN32 ${BENCH_SRCS} )

# Export ENV for Emscripten to allow setting environment variables
if(EMSCRIPTEN)
  set_target_properties(qgis_bench PROPERTIES
    QT_WASM_EXTRA_EXPORTED_METHODS "ENV"
  )
  # Add Emscripten-specific link flags to help with date/time handling
  target_link_options(qgis_bench PRIVATE
    "SHELL:-s FORCE_FILESYSTEM=1"
    "SHELL:-s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=['$autoResumeAudioContext','$dynCall']"
  )
endif()

include_directories(


  ${CMAKE_BINARY_DIR}
)

target_link_libraries(qgis_bench PRIVATE
  qgis_core
  ${QT_VERSION_BASE}::Core
  ${QT_VERSION_BASE}::Network
  ${QT_VERSION_BASE}::Svg
  ${QT_VERSION_BASE}::Xml
  ${QT_VERSION_BASE}::Test
)

if(APPLE)
  target_link_libraries(qgis_bench PRIVATE qgis_native)

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
