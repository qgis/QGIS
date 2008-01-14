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
SET_TARGET_PROPERTIES(qgis_applicationtest 
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_applicationtest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_applicationtest ${CMAKE_INSTALL_PREFIX}/qgis_applicationtest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_applicationtest RUNTIME DESTINATION ${QGIS_BIN_DIR})
  ADD_TEST(qgis_applicationtest ${QGIS_BIN_DIR}/qgis_applicationtest)
ENDIF (APPLE)
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
SET_TARGET_PROPERTIES(qgis_filewritertest
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_filewritertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_filewritertest ${CMAKE_INSTALL_PREFIX}/qgis_filewritertest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_filewritertest RUNTIME DESTINATION ${QGIS_BIN_DIR})
  ADD_TEST(qgis_filewritertest ${QGIS_BIN_DIR}/qgis_filewritertest)
ENDIF (APPLE)
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
SET_TARGET_PROPERTIES(qgis_rasterlayertest
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  ADD_TEST(qgis_rasterlayertest ${CMAKE_INSTALL_PREFIX}/qgis_rasterlayertest)
  INSTALL(TARGETS qgis_rasterlayertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
ELSE (APPLE)
  ADD_TEST(qgis_rasterlayertest ${QGIS_BIN_DIR}/qgis_rasterlayertest)
  INSTALL(TARGETS qgis_rasterlayertest RUNTIME DESTINATION ${QGIS_BIN_DIR})
ENDIF (APPLE)
#
# QgsMapLayer test
#
SET(qgis_maplayertest_SRCS testqgsmaplayer.cpp)
SET(qgis_maplayertest_MOC_CPPS testqgsmaplayer.cpp)
QT4_WRAP_CPP(qgis_maplayertest_MOC_SRCS ${qgis_maplayertest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_maplayertestmoc ALL DEPENDS ${qgis_maplayertest_MOC_SRCS})
ADD_EXECUTABLE(qgis_maplayertest ${qgis_maplayertest_SRCS})
ADD_DEPENDENCIES(qgis_maplayertest qgis_maplayertestmoc)
TARGET_LINK_LIBRARIES(qgis_maplayertest ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(qgis_maplayertest
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_maplayertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_maplayertest ${CMAKE_INSTALL_PREFIX}/qgis_maplayertest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_maplayertest RUNTIME DESTINATION ${QGIS_BIN_DIR})
  ADD_TEST(qgis_maplayertest ${QGIS_BIN_DIR}/qgis_maplayertest)
ENDIF (APPLE)
#
# Qgs Renderers test (qsinglesymbol renderer etc)
#
SET(qgis_rendererstest_SRCS testqgsrenderers.cpp)
SET(qgis_rendererstest_MOC_CPPS testqgsrenderers.cpp)
QT4_WRAP_CPP(qgis_rendererstest_MOC_SRCS ${qgis_rendererstest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_rendererstestmoc ALL DEPENDS ${qgis_rendererstest_MOC_SRCS})
ADD_EXECUTABLE(qgis_rendererstest ${qgis_rendererstest_SRCS})
ADD_DEPENDENCIES(qgis_rendererstest qgis_rendererstestmoc)
TARGET_LINK_LIBRARIES(qgis_rendererstest ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(qgis_rendererstest
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_rendererstest  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_rendererstest ${CMAKE_INSTALL_PREFIX}/qgis_rendererstest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_rendererstest RUNTIME DESTINATION ${QGIS_BIN_DIR})
  ADD_TEST(qgis_rendererstest ${QGIS_BIN_DIR}/qgis_rendererstest)
ENDIF (APPLE)
#
# QgsMapRender test
#
SET(qgis_maprendertest_SRCS testqgsmaprender.cpp)
SET(qgis_maprendertest_MOC_CPPS testqgsmaprender.cpp)
QT4_WRAP_CPP(qgis_maprendertest_MOC_SRCS ${qgis_maprendertest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_maprendertestmoc ALL DEPENDS ${qgis_maprendertest_MOC_SRCS})
ADD_EXECUTABLE(qgis_maprendertest ${qgis_maprendertest_SRCS})
ADD_DEPENDENCIES(qgis_maprendertest qgis_maprendertestmoc)
TARGET_LINK_LIBRARIES(qgis_maprendertest ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(qgis_maprendertest 
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_maprendertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_maprendertest ${CMAKE_INSTALL_PREFIX}/qgis_maprendertest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_maprendertest RUNTIME DESTINATION ${QGIS_BIN_DIR})
  ADD_TEST(qgis_maprendertest ${QGIS_BIN_DIR}/qgis_maprendertest)
ENDIF (APPLE)



