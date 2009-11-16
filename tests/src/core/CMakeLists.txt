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
# ensure the qgis libs can be found.
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  SET (CMAKE_INSTALL_NAME_DIR @executable_path/../../../src/core)
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
  SET(qgis_${testname}_MOC_CPPS ${testsrc})
  QT4_WRAP_CPP(qgis_${testname}_MOC_SRCS ${qgis_${testname}_MOC_CPPS})
  ADD_CUSTOM_TARGET(qgis_${testname}moc ALL DEPENDS ${qgis_${testname}_MOC_SRCS})
  ADD_EXECUTABLE(qgis_${testname} ${qgis_${testname}_SRCS})
  ADD_DEPENDENCIES(qgis_${testname} qgis_${testname}moc)
  TARGET_LINK_LIBRARIES(qgis_${testname} ${QT_LIBRARIES} qgis_core)
  SET_TARGET_PROPERTIES(qgis_${testname}
    PROPERTIES
    # skip the full RPATH for the build tree
    SKIP_BUILD_RPATH  TRUE
    # when building, use the install RPATH already
    # (so it doesn't need to relink when installing)
    BUILD_WITH_INSTALL_RPATH TRUE
    # the RPATH to be used when installing
    INSTALL_RPATH ${QGIS_LIB_DIR}
    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    INSTALL_RPATH_USE_LINK_PATH true)
  IF (APPLE)
    # For Mac OS X, the executable must be at the root of the bundle's executable folder
    INSTALL(TARGETS qgis_${testname} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
    ADD_TEST(qgis_${testname} ${CMAKE_INSTALL_PREFIX}/qgis_${testname})
  ELSE (APPLE)
    INSTALL(TARGETS qgis_${testname} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    ADD_TEST(qgis_${testname} ${CMAKE_INSTALL_PREFIX}/bin/qgis_${testname})
  ENDIF (APPLE)
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

ADD_QGIS_TEST(applicationtest testqgsapplication.cpp)
ADD_QGIS_TEST(filewritertest testqgsvectorfilewriter.cpp)
ADD_QGIS_TEST(regression992 regression992.cpp)
ADD_QGIS_TEST(regression1141 regression1141.cpp)
ADD_QGIS_TEST(rasterlayertest testqgsrasterlayer.cpp)
ADD_QGIS_TEST(contrastenhancementtest  testcontrastenhancements.cpp)
ADD_QGIS_TEST(maplayertest testqgsmaplayer.cpp)
ADD_QGIS_TEST(rendererstest testqgsrenderers.cpp)
ADD_QGIS_TEST(maprenderertest testqgsmaprenderer.cpp)
ADD_QGIS_TEST(geometrytest testqgsgeometry.cpp)
ADD_QGIS_TEST(coordinatereferencesystemtest testqgscoordinatereferencesystem.cpp)
ADD_QGIS_TEST(pointtest testqgspoint.cpp)
