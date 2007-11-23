#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core/raster
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  )

#############################################################
# Compiler defines

# expect that classes are being IMPORTED for the tests
IF (WIN32)
  # expect that classes are being imported
  # Note: MSVC doesn't like when the macros are quotes
  # and MSYS doesn't like them unqouted (bacause of braces)
  IF (MSVC)
    ADD_DEFINITIONS("-DCORE_EXPORT=__declspec(dllimport)")
    #ADD_DEFINITIONS("-DPLUGIN_EXPORT=__declspec(dllimport)")
    #next needed for qgis gui includes
    ADD_DEFINITIONS("-DGUI_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UCORE_EXPORT \"-DCORE_EXPORT=__declspec(dllimport)\"")
    #ADD_DEFINITIONS("\"-DPLUGIN_EXPORT=__declspec(dllimport)\"")
    #next needed for qgis gui includes
    ADD_DEFINITIONS("-UGUI_EXPORT \"-DGUI_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)

# This define is used for tests that need to locate the test
# data under tests/testdata in the qgis source tree.
# the TEST_DATA_DIR variable is set in the top level CMakeLists.txt
ADD_DEFINITIONS(-DTEST_DATA_DIR="\\"${TEST_DATA_DIR}\\"")

#############################################################
# libraries

# because of htonl
IF (WIN32)
  SET(PLATFORM_LIBRARIES wsock32)
ENDIF (WIN32)

# Since the tests are not actually installed, but rather
# run directly from the build/src/tests dir we need to
# ensure the omg libs can be found.
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  SET (CMAKE_INSTALL_NAME_DIR @executable_path/../../../src/core)
ENDIF (APPLE)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time 
#############################################################
# Tests:

#
# QgsApplication test
#
SET(qgis_applicationtest_SRCS testqgsapplication.cpp)
SET(qgis_applicationtest_MOC_CPPS testqgsapplication.cpp)
QT4_WRAP_CPP(qgis_applicationtest_MOC_SRCS ${qgis_applicationtest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_applicationtestmoc ALL DEPENDS ${qgis_applicationtest_MOC_SRCS})
ADD_EXECUTABLE(qgis_applicationtest ${qgis_applicationtest_SRCS})
ADD_DEPENDENCIES(qgis_applicationtest qgis_applicationtestmoc)
TARGET_LINK_LIBRARIES(qgis_applicationtest ${QT_LIBRARIES} qgis_core)
INSTALL(TARGETS qgis_applicationtest RUNTIME DESTINATION ${QGIS_BIN_DIR})
ADD_TEST(qgis_applicationtest ${QGIS_BIN_DIR}/qgis_applicationtest)
#
# QgsFileWriter test
#
SET(qgis_filewritertest_SRCS testqgsvectorfilewriter.cpp)
SET(qgis_filewritertest_MOC_CPPS testqgsvectorfilewriter.cpp)
QT4_WRAP_CPP(qgis_filewritertest_MOC_SRCS ${qgis_filewritertest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_filewritertestmoc ALL DEPENDS ${qgis_filewritertest_MOC_SRCS})
ADD_EXECUTABLE(qgis_filewritertest ${qgis_filewritertest_SRCS})
ADD_DEPENDENCIES(qgis_filewritertest qgis_filewritertestmoc)
TARGET_LINK_LIBRARIES(qgis_filewritertest ${QT_LIBRARIES} qgis_core)
INSTALL(TARGETS qgis_filewritertest RUNTIME DESTINATION ${QGIS_BIN_DIR})
ADD_TEST(qgis_filewritertest ${QGIS_BIN_DIR}/qgis_filewritertest)
#
# QgsRasterLayer test
#
SET(qgis_rasterlayertest_SRCS testqgsrasterlayer.cpp)
SET(qgis_rasterlayertest_MOC_CPPS testqgsrasterlayer.cpp)
QT4_WRAP_CPP(qgis_rasterlayertest_MOC_SRCS ${qgis_rasterlayertest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_rasterlayertestmoc ALL DEPENDS ${qgis_rasterlayertest_MOC_SRCS})
ADD_EXECUTABLE(qgis_rasterlayertest ${qgis_rasterlayertest_SRCS})
ADD_DEPENDENCIES(qgis_rasterlayertest qgis_rasterlayertestmoc)
TARGET_LINK_LIBRARIES(qgis_rasterlayertest ${QT_LIBRARIES} qgis_core)
INSTALL(TARGETS qgis_rasterlayertest RUNTIME DESTINATION ${QGIS_BIN_DIR})
ADD_TEST(qgis_rasterlayertest ${QGIS_BIN_DIR}/qgis_rasterlayertest)



