
########################################################
# Files

SET (DTEXT_SRCS
  qgsdelimitedtextfeatureiterator.cpp
  qgsdelimitedtextprovider.cpp
  qgsdelimitedtextfile.cpp
  qgsdelimitedtextsourceselect.cpp
)

SET (DTEXT_MOC_HDRS
  qgsdelimitedtextfile.h
  qgsdelimitedtextprovider.h
  qgsdelimitedtextsourceselect.h
)

########################################################
# Build

INCLUDE_DIRECTORIES(
  ../../core
  ../../core/geometry
  ../../gui
)
INCLUDE_DIRECTORIES (SYSTEM
  ${GEOS_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES (
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)

QT4_WRAP_CPP(DTEXT_MOC_SRCS ${DTEXT_MOC_HDRS})

ADD_LIBRARY(delimitedtextprovider MODULE ${DTEXT_SRCS} ${DTEXT_MOC_SRCS})

TARGET_LINK_LIBRARIES(delimitedtextprovider
  qgis_core
)

########################################################
# Install

INSTALL (TARGETS delimitedtextprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
