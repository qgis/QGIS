#
# Find deps
#
FIND_PACKAGE(Fcgi REQUIRED)
IF (NOT FCGI_FOUND)
  MESSAGE (SEND_ERROR "Fast CGI dependency was not found!")
ENDIF (NOT FCGI_FOUND)

# We need to define the qgis base dir so we can pass it 
# into the main entry point for the app so that at 
# runtime it can find plugins and srs.db etc
ADD_DEFINITIONS(-DQGIS_LIB_DIR="\\"${QGIS_LIB_DIR}\\"")

#MH: for this we need to locate the headers of the diagram and interpolation plugin
#ADD_DEFINITIONS(-DDIAGRAMSERVER=1)

IF (CMAKE_BUILD_TYPE MATCHES Debug)
  ADD_DEFINITIONS(-DQGSMSDEBUG=1)
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
########################################################
# Files

SET ( qgis_mapserv_SRCS
  qgis_map_serv.cpp
  qgsconfigcache.cpp
  qgsconfigparser.cpp
  qgsepsgcache.cpp
  qgsprojectparser.cpp
  qgshttprequesthandler.cpp 
  qgsgetrequesthandler.cpp
  qgssoaprequesthandler.cpp
  qgssldparser.cpp
  qgssldrenderer.cpp
  qgswmsserver.cpp
  qgsmapserviceexception.cpp
  qgsmapserverlogger.cpp
  qgsmslayercache.cpp
  qgsfilter.cpp
  qgssldrule.cpp
  qgsbetweenfilter.cpp
  qgscomparisonfilter.cpp
  qgslogicalfilter.cpp
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
)

SET (qgis_mapserv_RCCS 
  # not used 
  #qgis_mapserv.qrc
  )

QT4_WRAP_UI (qgis_mapserv_UIS_H  ${qgis_mapserv_UIS})

QT4_WRAP_CPP (qgis_mapserv_MOC_SRCS  ${qgis_mapserv_MOC_HDRS})

QT4_ADD_RESOURCES(qgis_mapserv_RCC_SRCS ${qgis_mapserv_RCCS})

ADD_EXECUTABLE(qgis_mapserv.fcgi
  ${qgis_mapserv_SRCS} 
  ${qgis_mapserv_MOC_SRCS} 
  ${qgis_mapserv_RCC_SRCS} 
  ${qgis_mapserv_UIS_H}
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
     ../core/renderer
     ../core/symbology
     ../core/symbology-ng
     ../core/composer
     ../analysis/interpolation
     .
)

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

