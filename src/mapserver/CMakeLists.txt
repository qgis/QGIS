#
# Find deps
#
FIND_PACKAGE(Fcgi REQUIRED)
IF (NOT FCGI_FOUND)
  MESSAGE (SEND_ERROR "Fast CGI dependency was not found!")
ENDIF (NOT FCGI_FOUND)

IF (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  ADD_DEFINITIONS(-DQGSMSDEBUG=1)
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

########################################################
# Files

SET ( qgis_mapserv_SRCS
  qgis_map_serv.cpp
  qgscapabilitiescache.cpp
  qgsconfigcache.cpp
  qgsconfigparser.cpp
  qgsprojectparser.cpp
  qgshttprequesthandler.cpp 
  qgsgetrequesthandler.cpp
  qgspostrequesthandler.cpp
  qgssoaprequesthandler.cpp
  qgssldparser.cpp
  qgswmsserver.cpp
  qgswfsserver.cpp
  qgsmapserviceexception.cpp
  qgsmslayercache.cpp
  qgsfilter.cpp
  qgsbetweenfilter.cpp
  qgscomparisonfilter.cpp
  qgslogicalfilter.cpp
  qgsspatialfilter.cpp
  qgsftptransaction.cpp
  qgsmslayerbuilder.cpp 
  qgshostedvdsbuilder.cpp
  qgsinterpolationlayerbuilder.cpp
  qgsremoteowsbuilder.cpp
  qgshostedrdsbuilder.cpp
  qgsremotedatasourcebuilder.cpp
  qgssentdatasourcebuilder.cpp
  qgsmsutils.cpp
  )

# SET (qgis_mapserv_UIS 
# none used 
# )

SET (qgis_mapserv_MOC_HDRS
  qgsftptransaction.h
  qgscapabilitiescache.h
  qgsconfigcache.h
  qgsmslayercache.h
)

SET (qgis_mapserv_RCCS 
  # not used 
  #qgis_mapserv.qrc
  )

QT4_WRAP_UI (qgis_mapserv_UIS_H  ${qgis_mapserv_UIS})

QT4_WRAP_CPP (qgis_mapserv_MOC_SRCS  ${qgis_mapserv_MOC_HDRS})

QT4_ADD_RESOURCES(qgis_mapserv_RCC_SRCS ${qgis_mapserv_RCCS})

SET(qgis_mapserv_TESTRCC_SRCS)
IF (ENABLE_TESTS)
  # add test resources, e.g. standard test font
  SET(qgis_mapserv_TESTRCCS ../../tests/testdata/testdata.qrc)
  QT4_ADD_RESOURCES(qgis_mapserv_TESTRCC_SRCS ${qgis_mapserv_TESTRCCS})
ENDIF(ENABLE_TESTS)

ADD_EXECUTABLE(qgis_mapserv.fcgi
  ${qgis_mapserv_SRCS} 
  ${qgis_mapserv_MOC_SRCS} 
  ${qgis_mapserv_RCC_SRCS} 
  ${qgis_mapserv_UIS_H}
  ${qgis_mapserv_TESTRCC_SRCS}
  )

INCLUDE_DIRECTORIES(
  ${GDAL_INCLUDE_DIR}
  ${FCGI_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QT_INCLUDE_DIR}
  ${QGIS_INCLUDE_DIR}
  ../core
  ../core/raster
  ../core/symbology-ng
  ../core/composer
  ../analysis/interpolation
  ../plugins/diagram_overlay
  .
)

IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(BEFORE ../core/spatialite/headers/spatialite)
ENDIF (WITH_INTERNAL_SPATIALITE)

TARGET_LINK_LIBRARIES(qgis_mapserv.fcgi
  qgis_core 
  qgis_analysis
  ${PROJ_LIBRARY}
  ${FCGI_LIBRARY}
  ${POSTGRES_LIBRARY}
  ${GDAL_LIBRARY}
)

########################################################
# Install

INSTALL(CODE "MESSAGE(\"Installing mapserver...\")")
INSTALL(TARGETS 
  qgis_mapserv.fcgi 
  DESTINATION ${QGIS_CGIBIN_DIR}
  )
INSTALL(FILES
  admin.sld 
  wms_metadata.xml
  DESTINATION ${QGIS_CGIBIN_DIR}
  )

