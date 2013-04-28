# Standard includes and utils to compile into all tests.
SET (util_SRCS)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../../../src/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../core #for render checker class
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/gui
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/gui/symbology-ng
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/gui/raster
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core/raster
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/core/symbology-ng
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  )

#############################################################
# Compiler defines

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

IF(WIN32)
  ADD_DEFINITIONS(-DQWT_DLL)
ENDIF(WIN32)

# Since the tests are not actually installed, but rather
# run directly from the build/src/tests dir we need to
# ensure the omg libs can be found.
IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
#  SET (CMAKE_INSTALL_NAME_DIR @executable_path/../../../src/gui)
ENDIF (APPLE)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time 
#############################################################
# Tests:

#
# QgsQuickPrint test
#
#SET(qgis_quickprinttest_SRCS testqgsquickprint.cpp ${util_SRCS})
#SET(qgis_quickprinttest_MOC_CPPS testqgsquickprint.cpp)
#QT4_WRAP_CPP(qgis_quickprinttest_MOC_SRCS ${qgis_quickprinttest_MOC_CPPS})
#ADD_CUSTOM_TARGET(qgis_quickprinttestmoc ALL DEPENDS ${qgis_quickprinttest_MOC_SRCS})
#ADD_EXECUTABLE(qgis_quickprinttest ${qgis_quickprinttest_SRCS})
#ADD_DEPENDENCIES(qgis_quickprinttest qgis_quickprinttestmoc)
#TARGET_LINK_LIBRARIES(qgis_quickprinttest ${QT_LIBRARIES} qgis_core qgis_gui)
#SET_TARGET_PROPERTIES(qgis_quickprinttest 
#  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
#  INSTALL_RPATH_USE_LINK_PATH true)
#IF (APPLE)
#  # For Mac OS X, the executable must be at the root of the bundle's executable folder
#  INSTALL(TARGETS qgis_quickprinttest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
#  ADD_TEST(qgis_quickprinttest ${CMAKE_INSTALL_PREFIX}/qgis_quickprinttest)
#ELSE (APPLE)
#  INSTALL(TARGETS qgis_quickprinttest RUNTIME DESTINATION ${QGIS_BIN_DIR})
#  ADD_TEST(qgis_quickprinttest ${CMAKE_INSTALL_PREFIX}/bin/qgis_quickprinttest)
#ENDIF (APPLE)

MACRO (ADD_QGIS_TEST testname testsrc)
  SET(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  SET(qgis_${testname}_MOC_CPPS ${testsrc})
  QT4_WRAP_CPP(qgis_${testname}_MOC_SRCS ${qgis_${testname}_MOC_CPPS})
  ADD_CUSTOM_TARGET(qgis_${testname}moc ALL DEPENDS ${qgis_${testname}_MOC_SRCS})
  ADD_EXECUTABLE(qgis_${testname} ${qgis_${testname}_SRCS})
  ADD_DEPENDENCIES(qgis_${testname} qgis_${testname}moc)
  TARGET_LINK_LIBRARIES(qgis_${testname}
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${QT_QTMAIN_LIBRARY}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    ${QWT_LIBRARY}
    qgis_core
    qgis_gui)
  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname})
  #SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
ENDMACRO (ADD_QGIS_TEST)

ADD_QGIS_TEST(zoomtest testqgsmaptoolzoom.cpp)

# a simple app for testing GUI of renderers
# These tests are old and are never run so removed for now.
#SET(rendererv2gui_SRCS testrendererv2gui.cpp)
#SET(rendererv2gui_HDRS testrendererv2gui.h)
#QT4_WRAP_CPP(rendererv2gui_MOC_SRCS ${rendererv2gui_HDRS})
#ADD_EXECUTABLE(qgis_rendererv2gui ${rendererv2gui_SRCS} ${rendererv2gui_MOC_SRCS})

ADD_QGIS_TEST(histogramtest testqgsrasterhistogram.cpp)
ADD_QGIS_TEST(projectionissues testprojectionissues.cpp)
ADD_QGIS_TEST(scalecombobox testqgsscalecombobox.cpp)
ADD_QGIS_TEST(dualviewtest testqgsdualview.cpp )
ADD_QGIS_TEST(rubberbandtest testqgsrubberband.cpp )

