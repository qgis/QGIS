
########################################################
# Files

SET (DTEXT_SRCS qgsdelimitedtextprovider.cpp)

SET (DTEXT_MOC_HDRS	qgsdelimitedtextprovider.h)

########################################################
# Build

INCLUDE_DIRECTORIES(
  .
  ../../core
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
)

QT4_WRAP_CPP(DTEXT_MOC_SRCS ${DTEXT_MOC_HDRS})

ADD_LIBRARY(delimitedtextprovider MODULE ${DTEXT_SRCS} ${DTEXT_MOC_SRCS})

TARGET_LINK_LIBRARIES(delimitedtextprovider
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  ${GDAL_LIBRARY}
  ${GEOS_LIBRARY}
  qgis_core
)

########################################################
# Install

INSTALL (TARGETS delimitedtextprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}) 
