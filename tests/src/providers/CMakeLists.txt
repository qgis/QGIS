# Standard includes and utils to compile into all tests.

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  )

#############################################################
# Compiler defines

# This define is used for tests that need to locate the test
# data under tests/testdata in the qgis source tree.
# the TEST_DATA_DIR variable is set in the top level CMakeLists.txt
ADD_DEFINITIONS(-DTEST_DATA_DIR="\\"${TEST_DATA_DIR}\\"")

ADD_DEFINITIONS(-DINSTALL_PREFIX="\\"${CMAKE_INSTALL_PREFIX}\\"")

# Temporarily set to old version until server is reconfigured 
#SET(TEST_SERVER_URL "http://wcs.qgis.org/${COMPLETE_VERSION}")
SET(TEST_SERVER_URL "http://wcs.qgis.org/1.9.0")
ADD_DEFINITIONS(-DTEST_SERVER_URL="\\"${TEST_SERVER_URL}\\"")

#############################################################
# libraries

# because of htonl
IF (WIN32)
  SET(PLATFORM_LIBRARIES wsock32)
ENDIF (WIN32)

# Since the tests are not actually installed, but rather
# run directly from the build/src/tests dir we need to
# ensure the qgis libs can be found.
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
#  SET (CMAKE_INSTALL_NAME_DIR @executable_path/../../../src/core)
ENDIF (APPLE)

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
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core)
  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname})
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

ADD_QGIS_TEST(wcsprovidertest testqgswcsprovider.cpp)
ADD_QGIS_TEST(gdalprovidertest testqgsgdalprovider.cpp)

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

  QT4_WRAP_CPP ( WCSTEST_MOC_SRCS  ${WCSTEST_MOC_HDRS})

  ADD_EXECUTABLE ( qgis_wcstest ${WCSTEST_SRCS} ${WCSTEST_MOC_SRCS} )

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core/raster
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/providers/wcs
    ${QT_QTSCRIPT_INCLUDE_DIR}
  )

  TARGET_LINK_LIBRARIES(qgis_wcstest
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${QT_QTMAIN_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
    qgis_core
  )

  INSTALL (TARGETS qgis_wcstest
    BUNDLE DESTINATION ${QGIS_BIN_DIR}
    RUNTIME DESTINATION ${QGIS_BIN_DIR}
  )
  INSTALL(FILES wcs-servers.json DESTINATION ${QGIS_DATA_DIR}/resources)

ENDIF(UNIX AND NOT ANDROID AND CMAKE_BUILD_TYPE MATCHES Debug)
