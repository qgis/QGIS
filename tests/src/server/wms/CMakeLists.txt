#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wms

  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}
)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree
set(MODULE_WMS_SRCS
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgswmsrequest.cpp
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgswmsrenderer.cpp
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgswmsrestorer.cpp
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgsmaprendererjobproxy.cpp
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgswmsparameters.cpp
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgswmsrendercontext.cpp
)

set(MODULE_WMS_HDRS
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgswmsserviceexception.h
)

if (BUILD_WITH_QT6)
  QT6_WRAP_CPP(MODULE_WMS_MOC_SRCS ${MODULE_WMS_HDRS})
else()
  QT5_WRAP_CPP(MODULE_WMS_MOC_SRCS ${MODULE_WMS_HDRS})
endif()

#############################################################
# Tests:

set(TESTS
  test_qgsserver_wms_dxf.cpp
  test_qgsserver_wms_request.cpp
  test_qgsserver_wms_restorer.cpp
  test_qgsserver_wms_exceptions.cpp
  test_qgsserver_wms_parameters.cpp
  test_qgsserver_wms_vector_tile.cpp
)

foreach(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC} MODULE server LINKEDLIBRARIES qgis_server wms_static)
endforeach(TESTSRC)
