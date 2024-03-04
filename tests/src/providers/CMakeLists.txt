# Standard includes and utils to compile into all tests.

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/providers/wms
  ${CMAKE_SOURCE_DIR}/src/providers/postgres
  ${CMAKE_SOURCE_DIR}/src/providers/mdal
  ${CMAKE_SOURCE_DIR}/src/providers/mssql
  ${CMAKE_SOURCE_DIR}/src/providers/ogr
  ${CMAKE_SOURCE_DIR}/src/providers/virtualraster
  ${CMAKE_SOURCE_DIR}/src/providers/virtual
  ${CMAKE_SOURCE_DIR}/src/test
)
include_directories(SYSTEM
  ${MDAL_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)


#############################################################
# Tests:

add_qgis_test(testqgswcsprovider.cpp MODULE provider LINKEDLIBRARIES qgis_core)
# Temporarily set to old version until server is reconfigured
#set(TEST_SERVER_URL "http://wcs.qgis.org/${COMPLETE_VERSION}")
set(TEST_SERVER_URL "http://wcs.qgis.org/1.9.0")
#add_definitions(-DTEST_SERVER_URL="${TEST_SERVER_URL}")
set_target_properties(test_provider_wcsprovider PROPERTIES
  COMPILE_FLAGS "-DTEST_SERVER_URL=\\\"${TEST_SERVER_URL}\\\""
)

add_qgis_test(testqgswmscapabilities.cpp MODULE provider LINKEDLIBRARIES provider_wms_a qgis_core)
add_qgis_test(testqgswmsccapabilities.cpp MODULE provider LINKEDLIBRARIES provider_wms_a qgis_core)
add_qgis_test(testqgswmsprovider.cpp MODULE provider LINKEDLIBRARIES provider_wms_a qgis_core)
add_qgis_test(testqgsvirtuallayerprovider.cpp MODULE provider LINKEDLIBRARIES provider_virtuallayer_a qgis_core)

if (POSTGRES_FOUND)
  add_qgis_test(testqgspostgresexpressioncompiler.cpp MODULE provider LINKEDLIBRARIES provider_postgres_a qgis_core)
  add_qgis_test(testqgspostgresprovider.cpp MODULE provider LINKEDLIBRARIES provider_postgres_a qgis_core LABELS "POSTGRES")
  add_qgis_test(testqgspostgresconn.cpp MODULE provider LINKEDLIBRARIES provider_postgres_a qgis_core LABELS "POSTGRES")
endif()

set(ENABLE_MSSQLTEST_CPP FALSE CACHE BOOL "Enable MSSQL provider C++ tests")
if (ENABLE_MSSQLTEST_CPP)
  add_qgis_test(testqgsmssqlprovider.cpp MODULE provider LINKEDLIBRARIES provider_mssql_a qgis_core LABELS "MSSQL")
  # also depend on dynamic lib, so that building this test will also build the dynamic lib
  add_dependencies(test_provider_mssqlprovider provider_mssql)
endif()

if (NOT FORCE_STATIC_PROVIDERS)
  add_qgis_test(testqgsmdalprovider.cpp MODULE provider LINKEDLIBRARIES qgis_core)
  if (WITH_ANALYSIS)
    add_qgis_test(testqgsvirtualrasterprovider.cpp MODULE provider LINKEDLIBRARIES qgis_core qgis_analysis)
  endif()
endif()

if (WITH_EPT)
  add_qgis_test(testqgseptprovider.cpp MODULE provider LINKEDLIBRARIES qgis_core)
endif()

if (WITH_COPC)
  add_qgis_test(testqgscopcprovider.cpp MODULE provider LINKEDLIBRARIES qgis_core)
  add_qgis_test(testqgsvirtualpointcloudprovider.cpp MODULE provider LINKEDLIBRARIES qgis_core)
endif()

if (WITH_PDAL)
  include_directories(
    ${CMAKE_SOURCE_DIR}/src/providers/pdal
  )
  add_qgis_test(testqgspdalprovider.cpp MODULE provider LINKEDLIBRARIES provider_pdal_a qgis_core)
endif()

#############################################################
# WCS public servers test:
# No need to test on all platforms
if(UNIX AND NOT ANDROID AND CMAKE_BUILD_TYPE MATCHES Debug)
  set ( WCSTEST_SRCS
        ../../../src/providers/wcs/qgswcscapabilities.cpp
        testqgswcspublicservers.cpp
  )

  add_executable(test_provider_wcs ${WCSTEST_SRCS} )
  target_compile_features(test_provider_wcs PRIVATE cxx_std_17)
  target_compile_definitions(test_provider_wcs PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core/raster
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/providers/wcs
  )

  target_link_libraries(test_provider_wcs
    ${QT_VERSION_BASE}::Core
    ${QT_VERSION_BASE}::Network
    ${QT_VERSION_BASE}::Svg
    ${QT_VERSION_BASE}::Xml
    ${OPTIONAL_QTWEBKIT}
    qgis_core
  )

  install (TARGETS test_provider_wcs
    BUNDLE DESTINATION ${QGIS_BIN_DIR}
    RUNTIME DESTINATION ${QGIS_BIN_DIR}
  )
  install(FILES wcs-servers.json DESTINATION ${QGIS_DATA_DIR}/resources)

endif()

add_subdirectory(grass)
