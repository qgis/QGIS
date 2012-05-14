SET (WMS_SRCS
  qgswmsprovider.cpp
  qgswmssourceselect.cpp
  qgstilescalewidget.cpp
  qgswmtsdimensions.cpp
  ../../app/qgsnewhttpconnection.cpp
  ../../app/qgsnumericsortlistviewitem.cpp
  ../../app/qgsmanageconnectionsdialog.cpp
)
SET (WMS_MOC_HDRS  
  qgswmsprovider.h 
  qgswmssourceselect.h 
  qgstilescalewidget.h
  qgswmtsdimensions.h
  ../../app/qgsnewhttpconnection.h
  ../../app/qgsnumericsortlistviewitem.h
  ../../app/qgsmanageconnectionsdialog.h
)

QT4_WRAP_CPP (WMS_MOC_SRCS ${WMS_MOC_HDRS})

INCLUDE_DIRECTORIES( . 
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ../../core 
  ../../core/raster 
  ../../gui
  ${GDAL_INCLUDE_DIR}
)

ADD_LIBRARY(wmsprovider MODULE ${WMS_SRCS} ${WMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(wmsprovider
  qgis_core
  qgis_gui
)

INSTALL (TARGETS wmsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}) 
