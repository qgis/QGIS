
SET (WMS_SRCS      
  qgswmsprovider.cpp 
  qgswmssourceselect.cpp 
  qgswmsconnection.cpp 
  qgswmsdataitems.cpp
)
SET (WMS_MOC_HDRS  
  qgswmsprovider.h 
  qgswmssourceselect.h 
  qgswmsconnection.h 
  qgswmsdataitems.h
)

QT4_WRAP_CPP (WMS_MOC_SRCS ${WMS_MOC_HDRS})

INCLUDE_DIRECTORIES( . ../../core ../../core/raster ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${GDAL_INCLUDE_DIR} # temporary solution until gdal get removed completely form QgsRasterLayer
)

ADD_LIBRARY(wmsprovider MODULE ${WMS_SRCS} ${WMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(wmsprovider
  qgis_core
  qgis_gui
)

INSTALL (TARGETS wmsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}) 
