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

ADD_SUBDIRECTORY(services)

########################################################
# Files

SET(QGIS_SERVER_SRCS
  qgsbufferserverrequest.cpp
  qgsbufferserverresponse.cpp
  qgscapabilitiescache.cpp
  qgsconfigcache.cpp
  qgsfcgiserverrequest.cpp
  qgsfcgiserverresponse.cpp
  qgsfilterresponsedecorator.cpp
  qgsfilterrestorer.cpp
  qgsrequesthandler.cpp
  qgsserver.cpp
  qgsserverparameters.cpp
  qgsserverexception.cpp
  qgsserverinterface.cpp
  qgsserverinterfaceimpl.cpp
  qgsserverlogger.cpp
  qgsserverprojectutils.cpp
  qgsserverrequest.cpp
  qgsserverresponse.cpp
  qgsserversettings.cpp
  qgsservice.cpp
  qgsservicenativeloader.cpp
  qgsserviceregistry.cpp
  qgsfeaturefilterprovidergroup.cpp
  qgsfeaturefilter.cpp
)

SET (QGIS_SERVER_HDRS
  qgsservicemodule.h
  qgsmapserviceexception.h
)


SET (QGIS_SERVER_MOC_HDRS
  qgscapabilitiescache.h
  qgsconfigcache.h
  qgsserverlogger.h
  qgsserversettings.h
  qgsserverparameters.h
)


#############################################################
# qgis_server library


SET(QGIS_SERVER_MOC_HDRS ${QGIS_SERVER_MOC_HDRS})

IF (WITH_SERVER_PLUGINS)
  SET(QGIS_SERVER_SRCS ${QGIS_SERVER_SRCS}
    qgsserverplugins.cpp
    qgsserverfilter.cpp
    qgsaccesscontrolfilter.cpp
    qgsaccesscontrol.cpp
    qgsservercachefilter.cpp
    qgsservercachemanager.cpp
  )
ENDIF (WITH_SERVER_PLUGINS)

QT5_WRAP_CPP(QGIS_SERVER_MOC_SRCS ${QGIS_SERVER_MOC_HDRS})

IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_SERVER_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_SERVER_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
ENDIF(MSVC)

INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${FCGI_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QT_INCLUDE_DIR}
  ${QGIS_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/analysis/interpolation
  ${CMAKE_SOURCE_DIR}/src/plugins/diagram_overlay
  ${CMAKE_SOURCE_DIR}/src/python

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server
)

ADD_LIBRARY(qgis_server SHARED ${QGIS_SERVER_SRCS} ${QGIS_SERVER_MOC_SRCS} ${QGIS_SERVER_HDRS} ${QGIS_SERVER_MOC_HDRS})

GENERATE_EXPORT_HEADER(
  qgis_server
  BASE_NAME SERVER
  EXPORT_FILE_NAME qgis_server.h
)

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
  ${PROJ_LIBRARY}
  ${FCGI_LIBRARY}
  ${POSTGRES_LIBRARY}
  ${GDAL_LIBRARY}
  ${QCA_LIBRARY}
)

IF (WITH_BINDINGS)
  TARGET_LINK_LIBRARIES(qgis_server qgispython)
ENDIF(WITH_BINDINGS)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgis_server PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
ENDIF (APPLE)


# install

INSTALL(TARGETS qgis_server
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR}
)

# end qgis_server library

# add test resources, e.g. standard test font
SET(QGIS_SERVER_TESTRCCS ../../tests/testdata/testdata.qrc)
QT5_ADD_RESOURCES(QGIS_SERVER_TESTRCC_SRCS ${QGIS_SERVER_TESTRCCS})

ADD_EXECUTABLE(qgis_mapserv.fcgi qgis_map_serv.cpp ${QGIS_SERVER_TESTRCC_SRCS})

TARGET_LINK_LIBRARIES(qgis_mapserv.fcgi qgis_server)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    qgis_mapserv.fcgi PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

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
  DESTINATION ${QGIS_CGIBIN_DIR}
)
