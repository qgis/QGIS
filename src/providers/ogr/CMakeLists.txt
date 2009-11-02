
SET (OGR_SRCS qgsogrprovider.cpp)

SET(OGR_MOC_HDRS qgsogrprovider.h)

########################################################
# Build

QT4_WRAP_CPP(OGR_MOC_SRCS ${OGR_MOC_HDRS})

INCLUDE_DIRECTORIES(
  .
  ../../core
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
)


ADD_LIBRARY(ogrprovider MODULE ${OGR_SRCS} ${OGR_MOC_SRCS})

TARGET_LINK_LIBRARIES(ogrprovider
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  ${GDAL_LIBRARY}
  ${GEOS_LIBRARY}
  qgis_core
)
IF (MSVC)
  #needed for linking to gdal which needs odbc
  SET(TARGET_LINK_LIBRARIES ${TARGET_LINK_LIBRARIE} odbc32 odbccp32)
ENDIF (MSVC)

INSTALL (TARGETS ogrprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

