########################################################
# Files

SET(QGIS_PROCESS_SRCS
  main.cpp
  qgsprocess.cpp
)

IF (UNIX AND NOT ANDROID)
  SET(QGIS_PROCESS_SRCS ${QGIS_PROCESS_SRCS}
    ${CMAKE_SOURCE_DIR}/external/qt-unix-signals/sigwatch.cpp
  )
ENDIF (UNIX AND NOT ANDROID)

########################################################
# Build

ADD_EXECUTABLE (qgis_process MACOSX_BUNDLE ${QGIS_PROCESS_SRCS})

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QT_INCLUDE_DIR}
  ${QGIS_INCLUDE_DIR}
)

IF (UNIX AND NOT ANDROID)
  INCLUDE_DIRECTORIES(SYSTEM
    ${CMAKE_SOURCE_DIR}/external/qt-unix-signals
  )
ENDIF (UNIX AND NOT ANDROID)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/processing
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/analysis/processing
  ${CMAKE_SOURCE_DIR}/src/python
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
)

IF (WITH_3D)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src/core/3d
    ${CMAKE_SOURCE_DIR}/src/3d/processing
    ${CMAKE_BINARY_DIR}/src/3d
  )
  INCLUDE_DIRECTORIES(SYSTEM
    ${QT5_3DEXTRA_INCLUDE_DIR}
  )
ENDIF (WITH_3D)

IF (WITH_BINDINGS)
  TARGET_LINK_LIBRARIES(qgis_process qgispython)
ENDIF(WITH_BINDINGS)

TARGET_LINK_LIBRARIES(qgis_process
  qgis_core
  qgis_analysis
  ${Qt5Core_LIBRARIES}
)

IF (WITH_3D)
  TARGET_LINK_LIBRARIES(qgis_process
    qgis_3d
  )
ENDIF (WITH_3D)

# install

INSTALL (TARGETS qgis_process
  BUNDLE DESTINATION ${QGIS_BIN_DIR}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
)

