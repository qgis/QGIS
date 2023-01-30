#
# Find deps
#
find_package(Fcgi REQUIRED)
if (NOT FCGI_FOUND)
  message (SEND_ERROR "Fast CGI dependency was not found!")
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  add_definitions(-DQGSMSDEBUG=1)
endif()

add_subdirectory(services)

########################################################
# Files

set(QGIS_SERVER_SRCS
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
  qgsserverapi.cpp
  qgsserverogcapi.cpp
  qgsserverogcapihandler.cpp
  qgsserverstatichandler.cpp
  qgsserverapiutils.cpp
  qgsserverapicontext.cpp
  qgsserverparameters.cpp
  qgsserverexception.cpp
  qgsserverfilter.cpp
  qgsserverfeatureid.cpp
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
  qgsdimensionfilter.cpp
  qgsfeaturefilter.cpp
  qgsstorebadlayerinfo.cpp
  qgsserverquerystringparameter.cpp
)

set (QGIS_SERVER_HDRS
  qgscapabilitiescache.h
  qgsconfigcache.h
  qgsmapserviceexception.h
  qgsserverapi.h
  qgsserverapicontext.h
  qgsserverlogger.h
  qgsserverogcapi.h
  qgsserverogcapihandler.h
  qgsserverstatichandler.h
  qgsserverparameters.h
  qgsserverfeatureid.h
  qgsserverquerystringparameter.h
  qgsserversettings.h
  qgsservicemodule.h
)


#############################################################
# qgis_server library

if (WITH_SERVER_PLUGINS)
  set(QGIS_SERVER_SRCS ${QGIS_SERVER_SRCS}
    qgsserverplugins.cpp
    qgsaccesscontrolfilter.cpp
    qgsaccesscontrol.cpp
    qgsservercachefilter.cpp
    qgsservercachemanager.cpp
  )
endif()

add_library(qgis_server SHARED ${QGIS_SERVER_SRCS} ${QGIS_SERVER_HDRS})

# require c++17
target_compile_features(qgis_server PRIVATE cxx_std_17)

target_include_directories(qgis_server SYSTEM PUBLIC
  ${FCGI_INCLUDE_DIR}
)

target_include_directories(qgis_server PUBLIC
  ${CMAKE_BINARY_DIR}/src/server
)

GENERATE_EXPORT_HEADER(
  qgis_server
  BASE_NAME SERVER
  EXPORT_FILE_NAME qgis_server.h
)

#generate unversioned libs for android
if (NOT ANDROID)
  set_target_properties(qgis_server PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
  )
endif()

target_link_libraries(qgis_server
  qgis_core
  ${PROJ_LIBRARY}
  ${FCGI_LIBRARY}
  ${POSTGRES_LIBRARY}
  ${GDAL_LIBRARY}
  ${QCA_LIBRARY}
)

if (WITH_ANALYSIS)
  target_link_libraries(qgis_server qgis_analysis)
endif()

if (WITH_BINDINGS)
  target_link_libraries(qgis_server qgispython)
endif()

if (APPLE)
  set_target_properties(qgis_server PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
endif()


# install

install(TARGETS qgis_server
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR}
)

# end qgis_server library

# add test resources, e.g. standard test font
set(QGIS_SERVER_TESTRCCS ../../tests/testdata/testdata.qrc)

add_executable(qgis_mapserv.fcgi qgis_map_serv.cpp ${QGIS_SERVER_TESTRCCS})
add_executable(qgis_mapserver qgis_mapserver.cpp ${QGIS_SERVER_TESTRCCS})

# require c++17
target_compile_features(qgis_mapserv.fcgi PRIVATE cxx_std_17)
target_compile_features(qgis_mapserver PRIVATE cxx_std_17)

target_link_libraries(qgis_mapserv.fcgi qgis_server)
target_link_libraries(qgis_mapserver qgis_server)

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    qgis_mapserv.fcgi PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

########################################################
# Install

install(CODE "message(\"Installing server...\")")
install(TARGETS
  qgis_mapserv.fcgi
  DESTINATION ${QGIS_CGIBIN_DIR}
)

install(TARGETS
  qgis_mapserver
  DESTINATION ${QGIS_BIN_DIR}
)

install(FILES
  admin.sld
  wms_metadata.xml
  DESTINATION ${QGIS_CGIBIN_DIR}
)
add_custom_target(qgis_server_full
  DEPENDS qgis_mapserv.fcgi wms wfs wcs wfs3 wmts qgis_server landingpage
)
