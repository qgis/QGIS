SET (WMS_SRCS
  qgswmscapabilities.cpp
  qgswmsprovider.cpp
  qgswmsconnection.cpp
  qgswmsdataitems.cpp
  qgsxyzconnection.cpp
)

IF (WITH_GUI)
  SET(WMS_GUI_SRCS
    qgswmsprovidergui.cpp
    qgswmssourceselect.cpp
    qgswmsdataitemguiproviders.cpp
    qgstilescalewidget.cpp
    qgswmtsdimensions.cpp
    qgsxyzconnectiondialog.cpp
  )
ENDIF ()

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

# static library
ADD_LIBRARY(wmsprovider_a STATIC ${WMS_SRCS})
TARGET_LINK_LIBRARIES(wmsprovider_a
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Sql_LIBRARIES}
  ${Qt5Concurrent_LIBRARIES}
)
TARGET_COMPILE_DEFINITIONS(wmsprovider_a PRIVATE "-DQT_NO_FOREACH")

IF (WITH_GUI)
  ADD_LIBRARY(wmsprovider_gui_a STATIC ${WMS_GUI_SRCS})
  TARGET_LINK_LIBRARIES(wmsprovider_gui_a
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
  )
  TARGET_COMPILE_DEFINITIONS(wmsprovider_gui_a PRIVATE "-DQT_NO_FOREACH")
  ADD_DEPENDENCIES(wmsprovider_gui_a ui)
  ADD_DEPENDENCIES(wmsprovider_a ui)
ENDIF (WITH_GUI)

IF (FORCE_STATIC_PROVIDERS)
  # for (external) mobile apps to be able to pick up provider for linking
  INSTALL (TARGETS wmsprovider_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  IF (WITH_GUI)
    INSTALL (TARGETS wmsprovider_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  ENDIF (WITH_GUI)
ELSE (FORCE_STATIC_PROVIDERS)
  # dynamically loaded module
  ADD_LIBRARY(wmsprovider MODULE ${WMS_SRCS} ${WMS_GUI_SRCS})

  TARGET_LINK_LIBRARIES(wmsprovider
    qgis_core
    ${GDAL_LIBRARY}  # for OGR_G_CreateGeometryFromJson()
  )

  IF (WITH_GUI)
    TARGET_LINK_LIBRARIES (wmsprovider
      qgis_gui
    )
    ADD_DEPENDENCIES(wmsprovider ui)
  ENDIF (WITH_GUI)

  TARGET_COMPILE_DEFINITIONS(wmsprovider PRIVATE "-DQT_NO_FOREACH")

  INSTALL (TARGETS wmsprovider
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

ENDIF (FORCE_STATIC_PROVIDERS)
