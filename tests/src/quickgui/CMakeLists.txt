# Standard includes and utils to compile into all tests.

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}

    ${CMAKE_SOURCE_DIR}/src/native
    ${CMAKE_SOURCE_DIR}/src/quickgui
    ${CMAKE_SOURCE_DIR}/src/test

    ${CMAKE_BINARY_DIR}/src/native
    ${CMAKE_BINARY_DIR}/src/quickgui
)

include_directories(SYSTEM
    ${PROJ_INCLUDE_DIR}
    ${GEOS_INCLUDE_DIR}
    ${LIBZIP_INCLUDE_DIRS}
    ${SPATIALINDEX_INCLUDE_DIR}
    ${GDAL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIR}
    ${SQLITE3_INCLUDE_DIR}
    ${SPATIALITE_INCLUDE_DIR}
    ${QCA_INCLUDE_DIR}
    ${QTKEYCHAIN_INCLUDE_DIR}
)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree

macro (ADD_QGIS_TEST testname testsrc)
  set(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  add_custom_target(qgis_${testname}moc ALL DEPENDS ${qgis_${testname}_MOC_SRCS})
  add_executable(qgis_${testname} ${qgis_${testname}_SRCS})
  target_compile_features(qgis_${testname} PRIVATE cxx_std_17)
  target_link_libraries(qgis_${testname}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Xml
    qgis_core
    qgis_quick)

  add_test(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
endmacro (ADD_QGIS_TEST)

#############################################################
# Tests:

ADD_QGIS_TEST(qgsquickutils testqgsquickutils.cpp)

#############################################################
# Add also test application
add_subdirectory(app)
