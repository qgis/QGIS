
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
  qgswmslayerinfos.cpp
)

set (WMS_HDRS
  qgswmsparameters.h
  qgswmsserviceexception.h
)

########################################################
# Build

set(_library_suffix_MODULE "")
set(_library_suffix_STATIC "_static")

foreach(_library_type MODULE STATIC)
  set(_library_name "wms${_library_suffix_${_library_type}}")

  add_library(${_library_name} ${_library_type} ${WMS_SRCS} ${WMS_HDRS})

  # require c++17
  target_compile_features(${_library_name} PRIVATE cxx_std_17)

  include_directories(${_library_name} SYSTEM PUBLIC
    ${GDAL_INCLUDE_DIR}
    ${POSTGRES_INCLUDE_DIR}
  )

  target_include_directories(${_library_name} PUBLIC
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/services
    ${CMAKE_SOURCE_DIR}/src/server/services/wms

    ${CMAKE_BINARY_DIR}/src/python
    ${CMAKE_BINARY_DIR}/src/analysis
    ${CMAKE_BINARY_DIR}/src/server
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  target_link_libraries(${_library_name}
    qgis_core
    qgis_server
  )
endforeach()

# only install module, static library is for testing only
install(TARGETS wms
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

