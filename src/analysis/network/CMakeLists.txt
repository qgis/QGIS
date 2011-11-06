

#############################################################
# sources

SET(QGIS_NETWORK_ANALYSIS_SRCS
  qgsgraph.cpp
  qgsgraphbuilder.cpp
  qgsdistancearcproperter.cpp
  qgslinevectorlayerdirector.cpp
  qgsgraphanalyzer.cpp
)

INCLUDE_DIRECTORIES(BEFORE raster)

SET(QGIS_NETWORK_ANALYSIS_MOC_HDRS
  qgsgraphdirector.h
)

QT4_WRAP_CPP(QGIS_NETWORK_ANALYSIS_MOC_SRCS ${QGIS_NETWORK_ANALYSIS_MOC_HDRS})

# install headers

SET(QGIS_NETWORK_ANALYSIS_HDRS 
  qgsgraph.h 
  qgsgraphbuilderintr.h 
  qgsgraphbuilder.h 
  qgsarcproperter.h 
  qgsdistancearcproperter.h 
  qgsgraphdirector.h 
  qgslinevectorlayerdirector.h 
  qgsgraphanalyzer.h )

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}../../core/
  ${CMAKE_CURRENT_SOURCE_DIR}../../core/spatialindex 
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)


#############################################################
# qgis_analysis library

ADD_LIBRARY(qgis_networkanalysis SHARED ${QGIS_NETWORK_ANALYSIS_SRCS} ${QGIS_NETWORK_ANALYSIS_MOC_SRCS} ${QGIS_NETWORK_ANALYSIS_HDRS})

SET_TARGET_PROPERTIES(qgis_networkanalysis PROPERTIES
  VERSION ${COMPLETE_VERSION}
  SOVERSION ${COMPLETE_VERSION}
  PUBLIC_HEADER "${QGIS_NETWORK_ANALYSIS_HDRS};${QGIS_NETWORK_ANALYSIS_MOC_HDRS}"
  CLEAN_DIRECT_OUTPUT 1
  FRAMEWORK 1
  FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
  MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
  MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
  MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis_networkanalysis)

ADD_DEPENDENCIES(qgis_networkanalysis qgis_core)

TARGET_LINK_LIBRARIES(qgis_networkanalysis
  qgis_core
)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgis_networkanalysis PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE )
ENDIF (APPLE)

# install

INSTALL(TARGETS qgis_networkanalysis
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# Mac dev frameworks

IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  INSTALL(TARGETS qgis_networkanalysis FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_networkanalysis.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_networkanalysis\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_networkanalysis.framework/qgis_networkanalysis\")")
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -change \"${CMAKE_INSTALL_NAME_DIR}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_networkanalysis.framework/qgis_networkanalysis\")")
ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
