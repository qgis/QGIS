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

add_executable (qgis_process MACOSX_BUNDLE ${QGIS_PROCESS_SRCS})

# require c++17
target_compile_features(qgis_process PRIVATE cxx_std_17)

target_include_directories(qgis_process PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)


if (UNIX AND NOT ANDROID)
  include_directories(SYSTEM
    ${CMAKE_SOURCE_DIR}/external/qt-unix-signals
  )
endif()

if (WITH_3D)
  include_directories(SYSTEM
    ${QT5_3DEXTRA_INCLUDE_DIR}
  )
endif()

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

