########################################################
# Files

SET(PDAL_SRCS
  qgspdaldataitems.cpp
  qgspdalprovider.cpp
)

SET(PDAL_HDRS
  qgspdaldataitems.h
  qgspdalprovider.h
)

IF (WITH_GUI)
  SET(PDAL_SRCS
    ${PDAL_SRCS}
    qgspdalprovidergui.cpp
    qgspdaldataitemguiprovider.cpp
    qgspdalsourceselect.cpp
  )

  SET(PDAL_HDRS
    ${PDAL_HDRS}
    qgspdalprovidergui.h
    qgspdaldataitemguiprovider.h
    qgspdalsourceselect.h
  )
ENDIF ()

########################################################
# Build

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/pointcloud
  ${CMAKE_SOURCE_DIR}/src/core/providers/ept
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/codeeditors
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)

INCLUDE_DIRECTORIES(SYSTEM
  ${PDAL_INCLUDE_DIR}
)

# static library
ADD_LIBRARY (pdalprovider MODULE ${PDAL_SRCS} ${PDAL_HDRS})
TARGET_LINK_LIBRARIES (pdalprovider
  ${PDAL_LIBRARIES}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES(pdalprovider
    qgis_gui
  )
  ADD_DEPENDENCIES(pdalprovider ui)
ENDIF (WITH_GUI)

INSTALL(TARGETS pdalprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
