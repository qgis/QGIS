# Standard includes and utils to compile into all tests.
SET (util_SRCS qgsrenderchecker.cpp)


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

ADD_DEFINITIONS(-DINSTALL_PREFIX="\\"${CMAKE_INSTALL_PREFIX}\\"")
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
SET(qgis_applicationtest_SRCS testqgsapplication.cpp ${util_SRCS})
SET(qgis_applicationtest_MOC_CPPS testqgsapplication.cpp)
QT4_WRAP_CPP(qgis_applicationtest_MOC_SRCS ${qgis_applicationtest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_applicationtestmoc ALL DEPENDS ${qgis_applicationtest_MOC_SRCS})
ADD_EXECUTABLE(qgis_applicationtest ${qgis_applicationtest_SRCS})
ADD_DEPENDENCIES(qgis_applicationtest qgis_applicationtestmoc)
TARGET_LINK_LIBRARIES(qgis_applicationtest ${QT_LIBRARIES} qgis_core)
  #No relinking and full RPATH for the install tree
  #See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree
SET_TARGET_PROPERTIES(qgis_applicationtest 
  # skip the full RPATH for the build tree
  PROPERTIES SKIP_BUILD_RPATH  TRUE
  )
SET_TARGET_PROPERTIES(qgis_applicationtest 
  # when building, use the install RPATH already
  # (so it doesn't need to relink when installing)
  PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE 
  )
SET_TARGET_PROPERTIES(qgis_applicationtest 
  # the RPATH to be used when installing
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  )
SET_TARGET_PROPERTIES(qgis_applicationtest 
  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE
  )
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_applicationtest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_applicationtest ${CMAKE_INSTALL_PREFIX}/qgis_applicationtest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_applicationtest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(qgis_applicationtest ${CMAKE_INSTALL_PREFIX}/bin/qgis_applicationtest)
ENDIF (APPLE)
#
# QgsFileWriter test
#
SET(qgis_filewritertest_SRCS testqgsvectorfilewriter.cpp ${util_SRCS})
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
  INSTALL(TARGETS qgis_filewritertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(qgis_filewritertest ${CMAKE_INSTALL_PREFIX}/bin/qgis_filewritertest)
ENDIF (APPLE)


#
# Ticket 992 Regression992 test
#
SET(regression992_SRCS regression992.cpp ${util_SRCS})
SET(regression992_MOC_CPPS regression992.cpp)
QT4_WRAP_CPP(regression992_MOC_SRCS ${regression992_MOC_CPPS})
ADD_CUSTOM_TARGET(regression992moc ALL DEPENDS ${regression992_MOC_SRCS})
ADD_EXECUTABLE(regression992 ${regression992_SRCS})
ADD_DEPENDENCIES(regression992 regression992moc)
TARGET_LINK_LIBRARIES(regression992 ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(regression992
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
# For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS regression992 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(regression992 ${CMAKE_INSTALL_PREFIX}/regression992)
ELSE (APPLE)
  INSTALL(TARGETS regression992 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(regression992 ${CMAKE_INSTALL_PREFIX}/bin/regression992)
ENDIF (APPLE)
#
# Ticket 1141 Regression1141 test
#
SET(regression1141_SRCS regression1141.cpp ${util_SRCS})
SET(regression1141_MOC_CPPS regression1141.cpp)
QT4_WRAP_CPP(regression1141_MOC_SRCS ${regression1141_MOC_CPPS})
ADD_CUSTOM_TARGET(regression1141moc ALL DEPENDS ${regression1141_MOC_SRCS})
ADD_EXECUTABLE(regression1141 ${regression1141_SRCS})
ADD_DEPENDENCIES(regression1141 regression1141moc)
TARGET_LINK_LIBRARIES(regression1141 ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(regression1141
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
# For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS regression1141 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(regression1141 ${CMAKE_INSTALL_PREFIX}/regression1141)
ELSE (APPLE)
  INSTALL(TARGETS regression1141 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(regression1141 ${CMAKE_INSTALL_PREFIX}/bin/regression1141)
ENDIF (APPLE)
#
# QgsRasterLayer test
#
SET(qgis_rasterlayertest_SRCS testqgsrasterlayer.cpp ${util_SRCS})
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
  ADD_TEST(qgis_rasterlayertest ${CMAKE_INSTALL_PREFIX}/bin/qgis_rasterlayertest)
  INSTALL(TARGETS qgis_rasterlayertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
ENDIF (APPLE)
#
# Contrast Enhancements Test
#
SET(qgis_contrastenhancementtest_SRCS testcontrastenhancements.cpp)
SET(qgis_contrastenhancementtest_MOC_CPPS testcontrastenhancements.cpp)
QT4_WRAP_CPP(qgis_contrastenhancementtest_MOC_SRCS ${qgis_contrastenhancementtest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_contrastenhancementtestmoc ALL DEPENDS ${qgis_contrastenhancementtest_MOC_SRCS})
ADD_EXECUTABLE(qgis_contrastenhancementtest ${qgis_contrastenhancementtest_SRCS})
ADD_DEPENDENCIES(qgis_contrastenhancementtest qgis_contrastenhancementtestmoc)
TARGET_LINK_LIBRARIES(qgis_contrastenhancementtest ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(qgis_contrastenhancementtest
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  ADD_TEST(qgis_contrastenhancementtest ${CMAKE_INSTALL_PREFIX}/qgis_contrastenhancementtest)
  INSTALL(TARGETS qgis_contrastenhancementtest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
ELSE (APPLE)
  ADD_TEST(qgis_contrastenhancementtest ${CMAKE_INSTALL_PREFIX}/bin/qgis_contrastenhancementtest)
  INSTALL(TARGETS qgis_contrastenhancementtest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
ENDIF (APPLE)
#
# QgsMapLayer test
#
SET(qgis_maplayertest_SRCS testqgsmaplayer.cpp ${util_SRCS})
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
  INSTALL(TARGETS qgis_maplayertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(qgis_maplayertest ${CMAKE_INSTALL_PREFIX}/bin/qgis_maplayertest)
ENDIF (APPLE)
#
# Qgs Renderers test (qsinglesymbol renderer etc)
#
SET(qgis_rendererstest_SRCS testqgsrenderers.cpp ${util_SRCS})
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
  INSTALL(TARGETS qgis_rendererstest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(qgis_rendererstest ${CMAKE_INSTALL_PREFIX}/bin/qgis_rendererstest)
ENDIF (APPLE)
#
# QgsMapRenderer test
#
SET(qgis_maprenderertest_SRCS testqgsmaprenderer.cpp ${util_SRCS})
SET(qgis_maprenderertest_MOC_CPPS testqgsmaprenderer.cpp)
QT4_WRAP_CPP(qgis_maprenderertest_MOC_SRCS ${qgis_maprenderertest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_maprenderertestmoc ALL DEPENDS ${qgis_maprenderertest_MOC_SRCS})
ADD_EXECUTABLE(qgis_maprenderertest ${qgis_maprenderertest_SRCS})
ADD_DEPENDENCIES(qgis_maprenderertest qgis_maprenderertestmoc)
TARGET_LINK_LIBRARIES(qgis_maprenderertest ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(qgis_maprenderertest 
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_maprenderertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_maprenderertest ${CMAKE_INSTALL_PREFIX}/qgis_maprenderertest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_maprenderertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(qgis_maprenderertest ${CMAKE_INSTALL_PREFIX}/bin/qgis_maprenderertest)
ENDIF (APPLE)
#
# QgsGeometry test
#
SET(qgis_geometrytest_SRCS testqgsgeometry.cpp ${util_SRCS})
SET(qgis_geometrytest_MOC_CPPS testqgsgeometry.cpp)
QT4_WRAP_CPP(qgis_geometrytest_MOC_SRCS ${qgis_geometrytest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_geometrytestmoc ALL DEPENDS ${qgis_geometrytest_MOC_SRCS})
ADD_EXECUTABLE(qgis_geometrytest ${qgis_geometrytest_SRCS})
ADD_DEPENDENCIES(qgis_geometrytest qgis_geometrytestmoc)
TARGET_LINK_LIBRARIES(qgis_geometrytest ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(qgis_geometrytest 
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_geometrytest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_geometrytest ${CMAKE_INSTALL_PREFIX}/qgis_geometrytest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_geometrytest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(qgis_geometrytest ${CMAKE_INSTALL_PREFIX}/bin/qgis_geometrytest)
ENDIF (APPLE)


#
# testqgscoordinatereferencesystem
#
SET(qgis_coordinatereferencesystemtest_SRCS testqgscoordinatereferencesystem.cpp ${util_SRCS})
SET(qgis_coordinatereferencesystemtest_MOC_CPPS testqgscoordinatereferencesystem.cpp)
QT4_WRAP_CPP(qgis_coordinatereferencesystemtest_MOC_SRCS ${qgis_coordinatereferencesystemtest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_coordinatereferencesystemtestmoc ALL DEPENDS ${qgis_coordinatereferencesystemtest_MOC_SRCS})
ADD_EXECUTABLE(qgis_coordinatereferencesystemtest ${qgis_coordinatereferencesystemtest_SRCS})
ADD_DEPENDENCIES(qgis_coordinatereferencesystemtest qgis_coordinatereferencesystemtestmoc)
TARGET_LINK_LIBRARIES(qgis_coordinatereferencesystemtest ${QT_LIBRARIES} qgis_core)
SET_TARGET_PROPERTIES(qgis_coordinatereferencesystemtest 
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true)
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_coordinatereferencesystemtest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_coordinatereferencesystemtest ${CMAKE_INSTALL_PREFIX}/qgis_coordinatereferencesystemtest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_coordinatereferencesystemtest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(qgis_coordinatereferencesystemtest ${CMAKE_INSTALL_PREFIX}/bin/qgis_coordinatereferencesystemtest)
ENDIF (APPLE)

