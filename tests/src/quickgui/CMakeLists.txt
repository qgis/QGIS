# Standard includes and utils to compile into all tests.

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}

    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/annotations
    ${CMAKE_SOURCE_DIR}/src/core/auth
    ${CMAKE_SOURCE_DIR}/src/core/composer
    ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
    ${CMAKE_SOURCE_DIR}/src/core/geometry
    ${CMAKE_SOURCE_DIR}/src/core/layertree
    ${CMAKE_SOURCE_DIR}/src/core/layout
    ${CMAKE_SOURCE_DIR}/src/core/locator
    ${CMAKE_SOURCE_DIR}/src/core/metadata
    ${CMAKE_SOURCE_DIR}/src/core/providers/memory
    ${CMAKE_SOURCE_DIR}/src/core/raster
    ${CMAKE_SOURCE_DIR}/src/core/scalebar
    ${CMAKE_SOURCE_DIR}/src/core/symbology
    ${CMAKE_SOURCE_DIR}/src/core/effects
    ${CMAKE_SOURCE_DIR}/src/core/metadata
    ${CMAKE_SOURCE_DIR}/src/core/expression
    ${CMAKE_SOURCE_DIR}/src/native
    ${CMAKE_SOURCE_DIR}/src/quickgui
    ${CMAKE_SOURCE_DIR}/src/test

    ${CMAKE_BINARY_DIR}/src/core
    ${CMAKE_BINARY_DIR}/src/native
    ${CMAKE_BINARY_DIR}/src/quickgui
)

INCLUDE_DIRECTORIES(SYSTEM
    ${LIBZIP_INCLUDE_DIRS}
    ${SPATIALINDEX_INCLUDE_DIR}
    ${PROJ_INCLUDE_DIR}
    ${GEOS_INCLUDE_DIR}
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
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Xml
    qgis_core
    qgis_quick)

  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

ADD_QGIS_TEST(qgsquickidentifykit testqgsquickidentifykit.cpp)
ADD_QGIS_TEST(qgsquickutils testqgsquickutils.cpp)
ADD_QGIS_TEST(qgsquickscalebarkit testqgsquickscalebarkit.cpp)
ADD_QGIS_TEST(qgsquickpositionkit testqgsquickpositionkit.cpp)


#############################################################
# Add also test application
ADD_SUBDIRECTORY(app)
