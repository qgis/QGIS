########################################################
# Files

IF(WITH_QSPATIALITE)
  SUBDIRS(${CMAKE_SOURCE_DIR}/external/qspatialite)
ENDIF(WITH_QSPATIALITE)

SET(SPATIALITE_SRCS
  qgsspatialiteprovider.cpp
  qgsspatialitedataitems.cpp
  qgsspatialiteconnection.cpp
  qgsspatialiteconnpool.cpp
  qgsspatialitefeatureiterator.cpp
  qgsspatialitetablemodel.cpp
  qgsspatialiteproviderconnection.cpp
)

IF (WITH_GUI)
  SET(SPATIALITE_SRCS ${SPATIALITE_SRCS}
    qgsspatialiteprovidergui.cpp
    qgsspatialitedataitemguiprovider.cpp
    qgsspatialitesourceselect.cpp
  )
ENDIF ()


########################################################
# Build

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/providers/ogr
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
)

ADD_LIBRARY (spatialiteprovider MODULE ${SPATIALITE_SRCS})

TARGET_LINK_LIBRARIES(spatialiteprovider
  qgis_core
  ${SPATIALITE_LIBRARY}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (spatialiteprovider
    qgis_gui
  )
  ADD_DEPENDENCIES(spatialiteprovider ui)
ENDIF ()

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    spatialiteprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

########################################################
# Install

INSTALL(TARGETS spatialiteprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
