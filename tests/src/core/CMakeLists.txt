# Standard includes and utils to compile into all tests.
SET (util_SRCS qgscompositionchecker.cpp)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/composer
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology-ng
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
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core)
  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname})
  #SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

ADD_QGIS_TEST(clippertest testqgsclipper.cpp)
ADD_QGIS_TEST(distanceareatest testqgsdistancearea.cpp)
ADD_QGIS_TEST(applicationtest testqgsapplication.cpp)
ADD_QGIS_TEST(diagramtest testqgsdiagram.cpp)
ADD_QGIS_TEST(diagramexpressiontest testqgsdiagramexpression.cpp)
ADD_QGIS_TEST(expressiontest testqgsexpression.cpp)
ADD_QGIS_TEST(filewritertest testqgsvectorfilewriter.cpp)
ADD_QGIS_TEST(projecttest testqgsproject.cpp)
ADD_QGIS_TEST(regression992 regression992.cpp)
ADD_QGIS_TEST(regression1141 regression1141.cpp)
ADD_QGIS_TEST(rasterlayertest testqgsrasterlayer.cpp)
ADD_QGIS_TEST(rastersublayertest testqgsrastersublayer.cpp)
ADD_QGIS_TEST(rasterfilewritertest testqgsrasterfilewriter.cpp)
ADD_QGIS_TEST(contrastenhancementtest  testcontrastenhancements.cpp)
ADD_QGIS_TEST(maplayertest testqgsmaplayer.cpp)
ADD_QGIS_TEST(rendererstest testqgsrenderers.cpp)
ADD_QGIS_TEST(maprenderertest testqgsmaprenderer.cpp)
ADD_QGIS_TEST(blendmodestest testqgsblendmodes.cpp)
ADD_QGIS_TEST(geometrytest testqgsgeometry.cpp)
ADD_QGIS_TEST(geometryimporttest testqgsgeometryimport.cpp)
ADD_QGIS_TEST(coordinatereferencesystemtest testqgscoordinatereferencesystem.cpp)
ADD_DEPENDENCIES(qgis_coordinatereferencesystemtest synccrsdb)
ADD_QGIS_TEST(pointtest testqgspoint.cpp)
ADD_QGIS_TEST(vectordataprovidertest testqgsvectordataprovider.cpp)
ADD_QGIS_TEST(vectorlayertest testqgsvectorlayer.cpp)
ADD_QGIS_TEST(rulebasedrenderertest testqgsrulebasedrenderer.cpp)
ADD_QGIS_TEST(ziplayertest testziplayer.cpp)
ADD_QGIS_TEST(dataitemtest testqgsdataitem.cpp)
ADD_QGIS_TEST(composerobject testqgscomposerobject.cpp)
ADD_QGIS_TEST(composerutils testqgscomposerutils.cpp)
ADD_QGIS_TEST(composerpapertest testqgscomposerpaper.cpp)
ADD_QGIS_TEST(composermaptest testqgscomposermap.cpp)
ADD_QGIS_TEST(composermapgridtest testqgscomposermapgrid.cpp)
ADD_QGIS_TEST(composereffectstest testqgscomposereffects.cpp)
ADD_QGIS_TEST(composershapestest testqgscomposershapes.cpp)
ADD_QGIS_TEST(composerrotationtest testqgscomposerrotation.cpp)
ADD_QGIS_TEST(composerpicturetest testqgscomposerpicture.cpp)
ADD_QGIS_TEST(atlascompositiontest testqgsatlascomposition.cpp)
ADD_QGIS_TEST(composerlabeltest testqgscomposerlabel.cpp)
ADD_QGIS_TEST(composertabletest testqgscomposertable.cpp)
ADD_QGIS_TEST(composerddtest testqgscomposerdd.cpp)
ADD_QGIS_TEST(stylev2test testqgsstylev2.cpp)
ADD_QGIS_TEST(composerhtmltest testqgscomposerhtml.cpp )
ADD_QGIS_TEST(rectangletest testqgsrectangle.cpp)
ADD_QGIS_TEST(composerscalebartest testqgscomposerscalebar.cpp )
ADD_QGIS_TEST(ogcutilstest testqgsogcutils.cpp)
ADD_QGIS_TEST(vectorlayercachetest testqgsvectorlayercache.cpp )
# ADD_QGIS_TEST(maprendererjobtest testmaprendererjob.cpp )
ADD_QGIS_TEST(spatialindextest testqgsspatialindex.cpp)
ADD_QGIS_TEST(gradienttest testqgsgradients.cpp )
ADD_QGIS_TEST(shapebursttest testqgsshapeburst.cpp )
ADD_QGIS_TEST(invertedpolygontest testqgsinvertedpolygonrenderer.cpp )
ADD_QGIS_TEST(colorschemeregistry testqgscolorschemeregistry.cpp)
ADD_QGIS_TEST(colorscheme testqgscolorscheme.cpp)
ADD_QGIS_TEST(networkcontentfetcher testqgsnetworkcontentfetcher.cpp )
ADD_QGIS_TEST(legendrenderertest testqgslegendrenderer.cpp )
ADD_QGIS_TEST(vectorlayerjoinbuffer testqgsvectorlayerjoinbuffer.cpp )
