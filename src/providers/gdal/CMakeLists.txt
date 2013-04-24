SET(GDAL_SRCS 
  qgsgdalproviderbase.cpp 
  qgsgdalprovider.cpp 
  qgsgdaldataitems.cpp 
)
SET(GDAL_MOC_HDRS  
  qgsgdalprovider.h 
)

INCLUDE_DIRECTORIES (
  ../../core
  ../../core/raster
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${GDAL_INCLUDE_DIR}
#  ${PROJ_INCLUDE_DIR}
#  ${GEOS_INCLUDE_DIR}
)

QT4_WRAP_CPP(GDAL_MOC_SRCS ${GDAL_MOC_HDRS})
ADD_LIBRARY (gdalprovider MODULE ${GDAL_SRCS} ${GDAL_MOC_SRCS})

TARGET_LINK_LIBRARIES (gdalprovider
  qgis_core
  qgis_gui
)

INSTALL(TARGETS gdalprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

