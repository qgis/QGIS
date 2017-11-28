########################################################
# Files

IF(WITH_QSPATIALITE)
  SUBDIRS(qspatialite)
ENDIF(WITH_QSPATIALITE)

SET(SPATIALITE_SRCS
  qgsspatialiteprovider.cpp
  qgsspatialitedataitems.cpp
  qgsspatialitefeatureiterator.cpp
  qgsspatialitetablemodel.cpp
)

SET(SPATIALITE_MOC_HDRS
  qgsspatialiteprovider.h
  qgsspatialitedataitems.h
  qgsspatialitetablemodel.h
)

IF (WITH_GUI)
  SET(SPATIALITE_SRCS ${SPATIALITE_SRCS}
    qgsspatialitesourceselect.cpp
  )
  SET(SPATIALITE_MOC_HDRS ${SPATIALITE_MOC_HDRS}
    qgsspatialitesourceselect.h
  )
ENDIF ()

SET(RASTERLITE2_SRCS
  qgsrasterlite2provider.cpp
)

SET(RASTERLITE2_MOC_HDRS
  qgsrasterlite2provider.h
)

########################################################
# Build

QT5_WRAP_CPP(SPATIALITE_MOC_SRCS ${SPATIALITE_MOC_HDRS})
QT5_WRAP_CPP(RASTERLITE2_MOC_SRCS ${RASTERLITE2_MOC_HDRS})

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/providers/spatialite
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/gui

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

ADD_LIBRARY (spatialiteprovider MODULE ${SPATIALITE_SRCS} ${SPATIALITE_MOC_SRCS})
SET(SPATIALITE_LIBRARYS_PROVIDER
  ${SPATIALITE_LIBRARY}
)

TARGET_LINK_LIBRARIES(spatialiteprovider
  qgis_core
  ${SPATIALITE_LIBRARYS_PROVIDER}
)

ADD_LIBRARY (rasterlite2provider MODULE ${RASTERLITE2_SRCS} ${RASTERLITE2_MOC_SRCS})

TARGET_LINK_LIBRARIES(rasterlite2provider
  qgis_core
  ${SPATIALITE_LIBRARY}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (spatialiteprovider
    qgis_gui
  )
ENDIF ()

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (rasterlite2provider
    qgis_gui
  )
ENDIF ()

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    spatialiteprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    rasterlite2provider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

########################################################
# Install

INSTALL(TARGETS spatialiteprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

INSTALL(TARGETS rasterlite2provider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
