
SET (GEONODE_SRCS
  qgsgeonodeprovider.cpp
)

SET(GEONODE_MOC_HDRS
)

IF (WITH_GUI)
  SET(GEONODE_SRCS ${GEONODE_SRCS}
    qgsgeonodedataitems.cpp
    qgsgeonodenewconnection.cpp
    qgsgeonodesourceselect.cpp
  )
  SET(GEONODE_MOC_HDRS ${GEONODE_MOC_HDRS}
    qgsgeonodesourceselect.h
    qgsgeonodenewconnection.h
    qgsgeonodedataitems.h
  )
ENDIF ()

########################################################
# Build

QT5_WRAP_CPP(GEONODE_MOC_SRCS ${GEONODE_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
)

ADD_LIBRARY(geonodeprovider MODULE ${GEONODE_SRCS} ${GEONODE_MOC_SRCS})

TARGET_LINK_LIBRARIES(geonodeprovider
  qgis_core
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (geonodeprovider
    qgis_gui
  )
ENDIF ()

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    geonodeprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

INSTALL (TARGETS geonodeprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

