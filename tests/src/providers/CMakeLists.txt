# Standard includes and utils to compile into all tests.

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/providers/wms
  ${CMAKE_SOURCE_DIR}/src/providers/postgres
  ${CMAKE_SOURCE_DIR}/src/providers/arcgisrest
  ${CMAKE_SOURCE_DIR}/src/providers/mdal
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_BINARY_DIR}/src/core
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${MDAL_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time 

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree

MACRO (ADD_QGIS_TEST testname testsrc)
  SET(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  ADD_CUSTOM_TARGET(qgis_${testname}moc ALL DEPENDS ${qgis_${testname}_MOC_SRCS})
  ADD_EXECUTABLE(qgis_${testname} ${qgis_${testname}_SRCS})
  SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES AUTOMOC TRUE)
  TARGET_LINK_LIBRARIES(qgis_${testname}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core)
  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

ADD_QGIS_TEST(wcsprovidertest testqgswcsprovider.cpp)
# Temporarily set to old version until server is reconfigured
#SET(TEST_SERVER_URL "http://wcs.qgis.org/${COMPLETE_VERSION}")
SET(TEST_SERVER_URL "http://wcs.qgis.org/1.9.0")
#ADD_DEFINITIONS(-DTEST_SERVER_URL="${TEST_SERVER_URL}")
SET_TARGET_PROPERTIES(qgis_wcsprovidertest PROPERTIES
  COMPILE_FLAGS "-DTEST_SERVER_URL=\\\"${TEST_SERVER_URL}\\\""
)

ADD_QGIS_TEST(arcgisrestutilstest testqgsarcgisrestutils.cpp)
TARGET_LINK_LIBRARIES(qgis_arcgisrestutilstest arcgisfeatureserverprovider_a)

ADD_QGIS_TEST(gdalprovidertest testqgsgdalprovider.cpp)

ADD_QGIS_TEST(ogrprovidertest testqgsogrprovider.cpp)

ADD_QGIS_TEST(wmscapabilitiestest
              testqgswmscapabilities.cpp)
TARGET_LINK_LIBRARIES(qgis_wmscapabilitiestest wmsprovider_a)

ADD_QGIS_TEST(wmsprovidertest
              testqgswmsprovider.cpp)
TARGET_LINK_LIBRARIES(qgis_wmsprovidertest wmsprovider_a)

ADD_QGIS_TEST(postgresprovidertest testqgspostgresprovider.cpp)
TARGET_LINK_LIBRARIES(qgis_postgresprovidertest postgresprovider_a)

ADD_QGIS_TEST(postgresconntest testqgspostgresconn.cpp)
TARGET_LINK_LIBRARIES(qgis_postgresconntest postgresprovider_a)

ADD_QGIS_TEST(mdalprovidertest testqgsmdalprovider.cpp)

#############################################################
# WCS public servers test:
# No need to test on all platforms
IF(UNIX AND NOT ANDROID AND CMAKE_BUILD_TYPE MATCHES Debug)
  SET ( WCSTEST_SRCS
        ../../../src/providers/wcs/qgswcscapabilities.cpp
        testqgswcspublicservers.cpp
  )
  SET ( WCSTEST_MOC_HDRS
        ../../../src/providers/wcs/qgswcscapabilities.h
        testqgswcspublicservers.h
  )

  QT5_WRAP_CPP ( WCSTEST_MOC_SRCS  ${WCSTEST_MOC_HDRS})

  ADD_EXECUTABLE ( qgis_wcstest ${WCSTEST_SRCS} ${WCSTEST_MOC_SRCS} )

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core/raster
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/providers/wcs
  )

  TARGET_LINK_LIBRARIES(qgis_wcstest
    ${Qt5Core_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${OPTIONAL_QTWEBKIT}
    qgis_core
  )

  INSTALL (TARGETS qgis_wcstest
    BUNDLE DESTINATION ${QGIS_BIN_DIR}
    RUNTIME DESTINATION ${QGIS_BIN_DIR}
  )
  INSTALL(FILES wcs-servers.json DESTINATION ${QGIS_DATA_DIR}/resources)

ENDIF(UNIX AND NOT ANDROID AND CMAKE_BUILD_TYPE MATCHES Debug)

ADD_SUBDIRECTORY(grass)
