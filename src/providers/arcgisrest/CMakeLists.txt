include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/labeling
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/providers/arcgis
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/pal
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/textrenderer
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/codeeditors
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui

)

include_directories(SYSTEM
  ${GEOS_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
endif()

###############################################################################

set (AFS_SRCS
  qgsafsfeatureiterator.cpp
  qgsafsprovider.cpp
  qgsafsshareddata.cpp
  qgsarcgisrestdataitems.cpp
)
if (WITH_GUI)
  set(AFS_SRCS ${AFS_SRCS}
    qgsarcgisrestdataitemguiprovider.cpp
    qgsarcgisrestprovidergui.cpp
    qgsarcgisrestsourceselect.cpp
    qgsnewarcgisrestconnection.cpp
  )
endif()

add_library (arcgisfeatureserverprovider_a STATIC ${AFS_SRCS})
add_library(arcgisfeatureserverprovider MODULE ${AFS_SRCS})

target_link_libraries(arcgisfeatureserverprovider
  qgis_core
  ${QCA_LIBRARY}
)

target_link_libraries (arcgisfeatureserverprovider_a
  qgis_core
  ${QCA_LIBRARY}
)

if (WITH_GUI)
  target_link_libraries(arcgisfeatureserverprovider
    qgis_gui
  )
  target_link_libraries(arcgisfeatureserverprovider_a
    qgis_gui
  )
  add_dependencies(arcgisfeatureserverprovider ui)
  add_dependencies(arcgisfeatureserverprovider_a ui)
endif()

install (TARGETS arcgisfeatureserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

###############################################################################

set (AMS_SRCS
  qgsamsprovider.cpp
)

add_library(arcgismapserverprovider MODULE ${AMS_SRCS})

target_link_libraries(arcgismapserverprovider
  qgis_core
  ${QCA_LIBRARY}
)

install (TARGETS arcgismapserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
