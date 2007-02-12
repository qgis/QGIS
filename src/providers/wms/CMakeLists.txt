
SET (WMS_SRCS      qgswmsprovider.cpp)
IF (NOT WIN32)
   SET (WMS_MOC_HDRS  qgswmsprovider.h)
ENDIF (NOT WIN32)

QT4_WRAP_CPP (WMS_MOC_SRCS ${WMS_MOC_HDRS})

INCLUDE_DIRECTORIES( 
     . 
     ../../gui 
     ../../raster 
     ../../core 
     ${SQLITE3_INCLUDE_DIR}
     ${PROJ_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
     )

ADD_LIBRARY(wmsprovider MODULE ${WMS_SRCS} ${WMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(wmsprovider
  ${QT_LIBRARIES}
  ${PROJ_LIBRARIES}
  ${GDAL_LIBRARIES}
  qgis_core
)

INSTALL (TARGETS wmsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}) 
