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
  qgshttprequesthandler.cpp
  qgsgetrequesthandler.cpp
  qgspostrequesthandler.cpp
  qgssoaprequesthandler.cpp
  qgswmsserver.cpp
  qgswfsserver.cpp
  qgswcsserver.cpp
  qgsmapserviceexception.cpp
  qgsmslayercache.cpp
  qgsftptransaction.cpp
  qgsmslayerbuilder.cpp
  qgshostedvdsbuilder.cpp
  qgsinterpolationlayerbuilder.cpp
  qgsremoteowsbuilder.cpp
  qgshostedrdsbuilder.cpp
  qgsremotedatasourcebuilder.cpp
  qgssentdatasourcebuilder.cpp
  qgsserverlogger.cpp
  qgsmsutils.cpp
  qgswcsprojectparser.cpp
  qgswfsprojectparser.cpp
  qgswmsconfigparser.cpp
  qgswmsprojectparser.cpp
  qgsserverprojectparser.cpp
  qgssldconfigparser.cpp
  qgsconfigparserutils.cpp
  )

# SET (qgis_mapserv_UIS
# none used
# )

SET (qgis_mapserv_MOC_HDRS
  qgsftptransaction.h
  qgscapabilitiescache.h
  qgsconfigcache.h
  qgsmslayercache.h
  qgsserverlogger.h
)

SET (qgis_mapserv_RCCS
  # not used
  #qgis_mapserv.qrc
  )



#############################################################
# qgis_server library


SET(qgis_mapserv_MOC_HDRS ${qgis_mapserv_MOC_HDRS})

SET(qgis_mapserv_SRCS ${qgis_mapserv_SRCS}
    qgsserverplugins.cpp
    qgsserverinterface.cpp
    qgsserverfilter.cpp
    qgsserverinterfaceimpl.cpp
)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DSERVER_EXPORT=${DLLEXPORT}")
  ELSE (MSVC)
    ADD_DEFINITIONS("-USERVER_EXPORT \"-DSERVER_EXPORT=${DLLEXPORT}\"")
  ENDIF (MSVC)
ENDIF (WIN32)


QT4_WRAP_CPP(qgis_mapserv_MOC_SRCS ${qgis_mapserv_MOC_HDRS})

ADD_LIBRARY(qgis_server SHARED ${qgis_mapserv_SRCS} ${qgis_mapserv_MOC_SRCS} ${qgis_mapserv_HDRS} ${qgis_mapserv_MOC_HDRS} )

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_server PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

TARGET_LINK_LIBRARIES(qgis_server
  qgis_core
  qgis_analysis
  qgispython
  ${PROJ_LIBRARY}
  ${FCGI_LIBRARY}
  ${POSTGRES_LIBRARY}
  ${GDAL_LIBRARY}
)

IF (WITH_SERVER_PLUGINS)
# install

INSTALL(TARGETS qgis_server
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

INCLUDE_DIRECTORIES(
    ../python
)

ENDIF (WITH_SERVER_PLUGINS) # end qgis_server library

QT4_WRAP_UI (qgis_mapserv_UIS_H  ${qgis_mapserv_UIS})


QT4_ADD_RESOURCES(qgis_mapserv_RCC_SRCS ${qgis_mapserv_RCCS})

# add test resources, e.g. standard test font
SET(qgis_mapserv_TESTRCCS ../../tests/testdata/testdata.qrc)
QT4_ADD_RESOURCES(qgis_mapserv_TESTRCC_SRCS ${qgis_mapserv_TESTRCCS})

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
  ../core/layertree
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

INSTALL(CODE "MESSAGE(\"Installing server...\")")
INSTALL(TARGETS
  qgis_mapserv.fcgi
  DESTINATION ${QGIS_CGIBIN_DIR}
  )
INSTALL(FILES
  admin.sld
  wms_metadata.xml
  schemaExtension.xsd
  DESTINATION ${QGIS_CGIBIN_DIR}
  )

