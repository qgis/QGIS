########################################################
# Files

set(QGIS_PROCESS_SRCS
  main.cpp
  qgsprocess.cpp
  ${CMAKE_SOURCE_DIR}/external/nlohmann/json_fwd.hpp
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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)
include_directories(SYSTEM
  ${QT_INCLUDE_DIR}
  ${QGIS_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
)

if (UNIX AND NOT ANDROID)
  include_directories(SYSTEM
    ${CMAKE_SOURCE_DIR}/external/qt-unix-signals
  )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/src/python

  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
)

if (WITH_3D)
  include_directories(
  )
  include_directories(SYSTEM
    ${QT5_3DEXTRA_INCLUDE_DIR}
  )
endif()

if (WITH_BINDINGS)
  target_link_libraries(qgis_process qgispython)
endif()

target_link_libraries(qgis_process
  qgis_core
  qgis_analysis
  ${Qt5Core_LIBRARIES}
  ${PROJ_LIBRARY}
  ${GEOS_LIBRARY}
  ${GDAL_LIBRARY}
)

if (WITH_3D)
  target_link_libraries(qgis_process
    qgis_3d
  )
endif()

# install

install (TARGETS qgis_process
  BUNDLE DESTINATION ${QGIS_BIN_DIR}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
)

