
########################################################
# Files

set (WMS_SRCS
  qgswms.cpp
  qgswmsutils.cpp
  qgsdxfwriter.cpp
  qgswmsdescribelayer.cpp
  qgswmsgetcapabilities.cpp
  qgswmsgetcontext.cpp
  qgswmsgetfeatureinfo.cpp
  qgswmsgetlegendgraphics.cpp
  qgswmsgetmap.cpp
  qgswmsgetprint.cpp
  qgswmsgetschemaextension.cpp
  qgswmsgetstyles.cpp
  qgsmaprendererjobproxy.cpp
  qgsmediancut.cpp
  qgswmsrenderer.cpp
  qgswmsparameters.cpp
  qgswmsrestorer.cpp
  qgswmsrendercontext.cpp
  qgswmsrequest.cpp
)

set (WMS_HDRS
  qgswmsparameters.h
  qgswmsserviceexception.h
)

########################################################
# Build

add_library (wms MODULE ${WMS_SRCS} ${WMS_HDRS})

# require c++17
target_compile_features(wms PRIVATE cxx_std_17)

include_directories(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wms

  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server
  ${CMAKE_CURRENT_BINARY_DIR}
)


target_link_libraries(wms
  qgis_core
  qgis_server
)


########################################################
# Install

install(TARGETS wms
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

