
SET (WCS_SRCS      
  ../gdal/qgsgdalproviderbase.cpp 
  qgswcsprovider.cpp 
  qgswcscapabilities.cpp 
  qgswcssourceselect.cpp 
  qgswcsdataitems.cpp 
)
SET (WCS_MOC_HDRS  
  ../gdal/qgsgdalproviderbase.h 
  qgswcsprovider.h 
  qgswcscapabilities.h 
  qgswcssourceselect.h 
  qgswcsdataitems.h
)

QT4_WRAP_CPP (WCS_MOC_SRCS ${WCS_MOC_HDRS})

INCLUDE_DIRECTORIES( . ../../core ../../core/raster ../../gui
  ../gdal
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${GDAL_INCLUDE_DIR}
)

ADD_LIBRARY(wcsprovider MODULE ${WCS_SRCS} ${WCS_MOC_SRCS})

TARGET_LINK_LIBRARIES(wcsprovider
  qgis_core
  qgis_gui
)

INSTALL (TARGETS wcsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}) 
