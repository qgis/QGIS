########################################################
# Files

set(QGIS_PROCESS_SRCS
  main.cpp
  qgsprocess.cpp
)

if (UNIX AND NOT ANDROID)
  set(QGIS_PROCESS_SRCS ${QGIS_PROCESS_SRCS}
    ${CMAKE_SOURCE_DIR}/external/qt-unix-signals/sigwatch.cpp
  )
endif()

########################################################
# Build

win32_icon(QGIS_PROCESS_SRCS)
win32_version_info("QGIS Process Command Tool" "qgis_process" QGIS_PROCESS_SRCS)

add_executable (qgis_process ${QGIS_PROCESS_SRCS})
if (QGISPOSTFIX)
  set_target_properties(qgis_process PROPERTIES RUNTIME_OUTPUT_NAME qgis_process${QGISPOSTFIX})
endif()

target_include_directories(qgis_process PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)


if (UNIX AND NOT ANDROID)
  include_directories(SYSTEM
    ${CMAKE_SOURCE_DIR}/external/qt-unix-signals
  )
endif()

target_include_directories(qgis_process PRIVATE
  ${CMAKE_SOURCE_DIR}/src/analysis/processing/pdal
)

target_link_libraries(qgis_process
  qgis_core
  qgis_analysis
  ${QT_VERSION_BASE}::Core
  ${GEOS_LIBRARY}
)

if (WITH_3D)
  target_link_libraries(qgis_process
    qgis_3d
  )
endif()

if (WITH_BINDINGS)
  target_link_libraries(qgis_process qgispython)
  add_definitions(-DWITH_BINDINGS)
endif()

# install

install (TARGETS qgis_process
  BUNDLE DESTINATION ${QGIS_BIN_DIR}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
)

