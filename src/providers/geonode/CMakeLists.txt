
SET (GEONODE_SRCS
  qgsgeonodeprovider.cpp
  qgsgeonodedataitems.cpp
)

IF (WITH_GUI)
  SET(GEONODE_SRCS ${GEONODE_SRCS}
    qgsgeonodeprovidergui.cpp
    qgsgeonodedataitemguiprovider.cpp
    qgsgeonodenewconnection.cpp
    qgsgeonodesourceselect.cpp
  )
ENDIF ()

########################################################
# Build

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES(SYSTEM
)

ADD_LIBRARY(geonodeprovider MODULE ${GEONODE_SRCS})

TARGET_LINK_LIBRARIES(geonodeprovider
  qgis_core
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (geonodeprovider
    qgis_gui
  )
  ADD_DEPENDENCIES(geonodeprovider ui)
ENDIF ()

TARGET_COMPILE_DEFINITIONS(geonodeprovider PRIVATE "-DQT_NO_FOREACH")

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

