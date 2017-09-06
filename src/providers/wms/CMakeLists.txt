FIND_PACKAGE(Qt5XmlPatterns REQUIRED)


SET (WMS_SRCS
  qgswmscapabilities.cpp
  qgswmsprovider.cpp
  qgswmsconnection.cpp
  qgswmsdataitems.cpp
  qgstilecache.cpp
  qgsxyzconnection.cpp
)
SET (WMS_MOC_HDRS
  qgswmscapabilities.h
  qgswmsprovider.h
  qgswmsconnection.h
  qgswmsdataitems.h
)

IF (WITH_GUI)
  SET(WMS_SRCS ${WMS_SRCS}
    qgswmssourceselect.cpp
    qgstilescalewidget.cpp
    qgswmtsdimensions.cpp
    qgsxyzconnectiondialog.cpp
  )
  SET(WMS_MOC_HDRS ${WMS_MOC_HDRS}
    qgswmssourceselect.h
    qgstilescalewidget.h
    qgswmtsdimensions.h
    qgsxyzconnectiondialog.h
  )
ENDIF ()

QT5_WRAP_CPP (WMS_MOC_SRCS ${WMS_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth

  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QT_QTSCRIPT_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

ADD_LIBRARY(wmsprovider_a STATIC ${WMS_SRCS} ${WMS_MOC_SRCS})
ADD_LIBRARY(wmsprovider MODULE ${WMS_SRCS} ${WMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(wmsprovider
  qgis_core
  ${QT_QTSCRIPT_LIBRARY}
  ${GDAL_LIBRARY}  # for OGR_G_CreateGeometryFromJson()
)


TARGET_LINK_LIBRARIES(wmsprovider_a
  qgis_core
  ${QT_QTSCRIPT_LIBRARY}
)


IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (wmsprovider
    qgis_gui
  )
  TARGET_LINK_LIBRARIES (wmsprovider_a
    qgis_gui
  )
ENDIF ()

INSTALL (TARGETS wmsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
