########################################################
# Files

set(PDAL_SRCS
  qgspdaldataitems.cpp
  qgspdalprovider.cpp
)

set(PDAL_HDRS
  qgspdaldataitems.h
  qgspdalprovider.h
)

if (WITH_GUI)
  set(PDAL_SRCS
    ${PDAL_SRCS}
    qgspdalprovidergui.cpp
    qgspdaldataitemguiprovider.cpp
  )

  set(PDAL_HDRS
    ${PDAL_HDRS}
    qgspdalprovidergui.h
    qgspdaldataitemguiprovider.h
  )
endif()

########################################################
# Build

include_directories(
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

include_directories(SYSTEM
  ${PDAL_INCLUDE_DIR}
)

add_library (pdalprovider MODULE ${PDAL_SRCS} ${PDAL_HDRS})
target_link_libraries (pdalprovider
  ${PDAL_LIBRARIES}
)

# static library
add_library (pdalprovider_a STATIC ${PDAL_SRCS} ${PDAL_HDRS})
target_link_libraries (pdalprovider_a
  ${PDAL_LIBRARIES}
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Sql_LIBRARIES}
  ${Qt5Concurrent_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries(pdalprovider
    qgis_gui
  )
  add_dependencies(pdalprovider ui)
endif()

install(TARGETS pdalprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
