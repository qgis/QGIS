SET (WMS_SRCS      
  qgswmsprovider.cpp 
  qgswmssourceselect.cpp 
  qgswmsconnection.cpp 
  qgswmsdataitems.cpp
  qgstilescalewidget.cpp
  qgswmtsdimensions.cpp
)
SET (WMS_MOC_HDRS  
  qgswmsprovider.h 
  qgswmssourceselect.h 
  qgswmsconnection.h 
  qgswmsdataitems.h
  qgstilescalewidget.h
  qgswmtsdimensions.h
)

QT4_WRAP_CPP (WMS_MOC_SRCS ${WMS_MOC_HDRS})

INCLUDE_DIRECTORIES( . 
  ../../core 
  ../../core/raster 
  ${GDAL_INCLUDE_DIR}
)

ADD_LIBRARY(wmsprovider MODULE ${WMS_SRCS} ${WMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(wmsprovider
  qgis_core
)

INSTALL (TARGETS wmsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}) 
