

#############################################################
# sources

SET(QGIS_NETWORK_ANALYSIS_SRCS
  qgsgraph.cpp
  qgsgraphbuilder.cpp
)

INCLUDE_DIRECTORIES(BEFORE raster)

SET(QGIS_ANALYSIS_MOC_HDRS
)

QT4_WRAP_CPP(QGIS_NETWORK_ANALYSIS_MOC_SRCS ${QGIS_ANALYSIS_MOC_HDRS})


INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}../../core/
  interpolation
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)


#############################################################
# qgis_analysis library

ADD_LIBRARY(qgis_networkanalysis SHARED ${QGIS_NETWORK_ANALYSIS_SRCS} ${QGIS_NETWORK_ANALYSIS_MOC_SRCS} )

ADD_DEPENDENCIES(qgis_networkanalysis qgis_core)

SET_TARGET_PROPERTIES(qgis_networkanalysis PROPERTIES VERSION ${COMPLETE_VERSION} SOVERSION ${COMPLETE_VERSION})

TARGET_LINK_LIBRARIES(qgis_networkanalysis
  qgis_core
)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgis_networkanalysis PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE )
ENDIF (APPLE)

INSTALL(TARGETS qgis_networkanalysis
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR})


# Added by Tim to install headers

SET(QGIS_NETWORK_ANALYSIS_HDRS 
  qgsgraph.h 
  qgsgraphbuilderinterface.h 
  qgsgraphbuilder.h )

INSTALL(CODE "MESSAGE(\"Installing NETWORK ANALYSIS headers...\")")
INSTALL(FILES ${QGIS_NETWORK_ANALYSIS_HDRS} ${QGIS_NETWORK_ANALYSIS_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR}) 
