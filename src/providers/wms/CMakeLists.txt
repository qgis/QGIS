IF (QT5_BUILD)
  FIND_PACKAGE(Qt5XmlPatterns REQUIRED)
ENDIF()


SET (WMS_SRCS
  qgswmscapabilities.cpp
  qgswmsprovider.cpp
  qgswmssourceselect.cpp
  qgswmsconnection.cpp
  qgswmsdataitems.cpp
  qgstilescalewidget.cpp
  qgswmtsdimensions.cpp
)
SET (WMS_MOC_HDRS
  qgswmscapabilities.h
  qgswmsprovider.h
  qgswmssourceselect.h
  qgswmsconnection.h
  qgswmsdataitems.h
  qgstilescalewidget.h
  qgswmtsdimensions.h
)

QT4_WRAP_CPP (WMS_MOC_SRCS ${WMS_MOC_HDRS})

INCLUDE_DIRECTORIES( . ../../core ../../core/geometry ../../core/raster ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QT_QTSCRIPT_INCLUDE_DIR}
)

ADD_LIBRARY(wmsprovider MODULE ${WMS_SRCS} ${WMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(wmsprovider
  qgis_core
  qgis_gui
  ${QT_QTSCRIPT_LIBRARY}
  ${GDAL_LIBRARY}  # for OGR_G_CreateGeometryFromJson()
)

INSTALL (TARGETS wmsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
