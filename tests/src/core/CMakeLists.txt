#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  )

# expect that classes are being IMPORTED for the exe
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
SET(applicationtest_SRCS testqgsapplication.cpp)
SET(applicationtest_MOC_CPPS testqgsapplication.cpp)
QT4_WRAP_CPP(applicationtest_MOC_SRCS ${applicationtest_MOC_CPPS})
ADD_CUSTOM_TARGET(applicationtestmoc ALL DEPENDS ${applicationtest_MOC_SRCS})
ADD_EXECUTABLE(applicationtest ${applicationtest_SRCS})
ADD_DEPENDENCIES(applicationtest applicationtestmoc)
TARGET_LINK_LIBRARIES(applicationtest ${QT_LIBRARIES} qgis_core)
IF (APPLE)
  INSTALL(TARGETS applicationtest RUNTIME DESTINATION ${QGIS_BIN_DIR}/../)
  ADD_TEST(application ${QGIS_BIN_DIR}/../applicationtest)
ELSE (APPLE)
  INSTALL(TARGETS applicationtest RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  ADD_TEST(application ${CMAKE_CURRENT_BINARY_DIR}/applicationtest)
ENDIF (APPLE)
#
# QgsFileWriter test
#
SET(filewritertest_SRCS testqgsvectorfilewriter.cpp)
SET(filewritertest_MOC_CPPS testqgsvectorfilewriter.cpp)
QT4_WRAP_CPP(filewritertest_MOC_SRCS ${filewritertest_MOC_CPPS})
ADD_CUSTOM_TARGET(filewritertestmoc ALL DEPENDS ${filewritertest_MOC_SRCS})
ADD_EXECUTABLE(filewritertest ${filewritertest_SRCS})
ADD_DEPENDENCIES(filewritertest filewritertestmoc)
TARGET_LINK_LIBRARIES(filewritertest ${QT_LIBRARIES} qgis_core)



#
# We need to install to a path
#

INSTALL(TARGETS filewritertest RUNTIME DESTINATION ${QGIS_BIN_DIR}/../)
ADD_TEST(filewriter ${QGIS_BIN_DIR}/../filewritertest)


