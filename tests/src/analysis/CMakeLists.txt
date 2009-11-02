# Standard includes and utils to compile into all tests.
SET (util_SRCS ../core/qgsrenderchecker.cpp)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core/
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/analysis/vector
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
    ADD_DEFINITIONS("-DANALYzer_EXPORT=__declspec(dllimport)")
    #ADD_DEFINITIONS("-DPLUGIN_EXPORT=__declspec(dllimport)")
    #next needed for qgis gui includes
    ADD_DEFINITIONS("-DGUI_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UCORE_EXPORT \"-DCORE_EXPORT=__declspec(dllimport)\"")
    ADD_DEFINITIONS("-UANALYzer_EXPORT \"-DCORE_EXPORT=__declspec(dllimport)\"")
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
  SET (CMAKE_INSTALL_NAME_DIR @executable_path/../../../src/analyzer)
ENDIF (APPLE)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time 
#############################################################
# Tests:

#
# QgsVectorAnalyzer test
#
SET(qgis_vectoranalyzertest_SRCS testqgsvectoranalyzer.cpp ${util_SRCS})
SET(qgis_vectoranalyzertest_MOC_CPPS testqgsvectoranalyzer.cpp)
QT4_WRAP_CPP(qgis_vectoranalyzertest_MOC_SRCS ${qgis_vectoranalyzertest_MOC_CPPS})
ADD_CUSTOM_TARGET(qgis_vectoranalyzertestmoc ALL DEPENDS ${qgis_vectoranalyzertest_MOC_SRCS})
ADD_EXECUTABLE(qgis_vectoranalyzertest ${qgis_vectoranalyzertest_SRCS})
ADD_DEPENDENCIES(qgis_vectoranalyzertest qgis_vectoranalyzertestmoc)
TARGET_LINK_LIBRARIES(qgis_vectoranalyzertest ${QT_LIBRARIES} qgis_core qgis_analysis)
  #No relinking and full RPATH for the install tree
  #See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree
SET_TARGET_PROPERTIES(qgis_vectoranalyzertest 
  # skip the full RPATH for the build tree
  PROPERTIES SKIP_BUILD_RPATH  TRUE
  )
SET_TARGET_PROPERTIES(qgis_vectoranalyzertest 
  # when building, use the install RPATH already
  # (so it doesn't need to relink when installing)
  PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE 
  )
SET_TARGET_PROPERTIES(qgis_vectoranalyzertest 
  # the RPATH to be used when installing
  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
  )
SET_TARGET_PROPERTIES(qgis_vectoranalyzertest 
  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE
  )
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis_vectoranalyzertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
  ADD_TEST(qgis_vectoranalyzertest ${CMAKE_INSTALL_PREFIX}/qgis_vectoranalyzertest)
ELSE (APPLE)
  INSTALL(TARGETS qgis_vectoranalyzertest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ADD_TEST(qgis_vectoranalyzertest ${CMAKE_INSTALL_PREFIX}/bin/qgis_vectoranalyzertest)
ENDIF (APPLE)



