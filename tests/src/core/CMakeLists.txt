# Standard includes and utils to compile into all tests.
SET (util_SRCS qgscompositionchecker.cpp)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/composer
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/effects
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
  ADD_EXECUTABLE(qgis_${testname} ${qgis_${testname}_SRCS})
  SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES AUTOMOC TRUE)
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

ADD_QGIS_TEST(qgistest testqgis.cpp)
ADD_QGIS_TEST(clippertest testqgsclipper.cpp)
ADD_QGIS_TEST(distanceareatest testqgsdistancearea.cpp)
ADD_QGIS_TEST(applicationtest testqgsapplication.cpp)
ADD_QGIS_TEST(diagramtest testqgsdiagram.cpp)
ADD_QGIS_TEST(diagramexpressiontest testqgsdiagramexpression.cpp)
ADD_QGIS_TEST(expressiontest testqgsexpression.cpp)
ADD_QGIS_TEST(fieldtest testqgsfield.cpp)
ADD_QGIS_TEST(fieldstest testqgsfields.cpp)
ADD_QGIS_TEST(featuretest testqgsfeature.cpp)
ADD_QGIS_TEST(scaleexpressiontest testqgsscaleexpression.cpp)
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
ADD_QGIS_TEST(geometryutilstest testqgsgeometryutils.cpp)
ADD_QGIS_TEST(coordinatereferencesystemtest testqgscoordinatereferencesystem.cpp)
ADD_QGIS_TEST(coordinatetransformtest testqgscoordinatetransform.cpp)
ADD_DEPENDENCIES(qgis_coordinatereferencesystemtest synccrsdb)
ADD_QGIS_TEST(pointtest testqgspoint.cpp)
ADD_QGIS_TEST(vectordataprovidertest testqgsvectordataprovider.cpp)
ADD_QGIS_TEST(vectorlayertest testqgsvectorlayer.cpp)
ADD_QGIS_TEST(rulebasedrenderertest testqgsrulebasedrenderer.cpp)
ADD_QGIS_TEST(ziplayertest testziplayer.cpp)
ADD_QGIS_TEST(dataitemtest testqgsdataitem.cpp)
ADD_QGIS_TEST(datadefined testqgsdatadefined.cpp)
ADD_QGIS_TEST(composerobject testqgscomposerobject.cpp)
ADD_QGIS_TEST(composerutils testqgscomposerutils.cpp)
ADD_QGIS_TEST(compositiontest testqgscomposition.cpp)
ADD_QGIS_TEST(composermodel testqgscomposermodel.cpp)
ADD_QGIS_TEST(composermultiframetest testqgscomposermultiframe.cpp)
ADD_QGIS_TEST(composergrouptest testqgscomposergroup.cpp)
ADD_QGIS_TEST(composerpapertest testqgscomposerpaper.cpp)
ADD_QGIS_TEST(composermaptest testqgscomposermap.cpp)
ADD_QGIS_TEST(composermapgridtest testqgscomposermapgrid.cpp)
ADD_QGIS_TEST(composermapoverviewtest testqgscomposermapoverview.cpp)
ADD_QGIS_TEST(composereffectstest testqgscomposereffects.cpp)
ADD_QGIS_TEST(composershapestest testqgscomposershapes.cpp)
ADD_QGIS_TEST(composerrotationtest testqgscomposerrotation.cpp)
ADD_QGIS_TEST(composerpicturetest testqgscomposerpicture.cpp)
ADD_QGIS_TEST(atlascompositiontest testqgsatlascomposition.cpp)
ADD_QGIS_TEST(composerlabeltest testqgscomposerlabel.cpp)
ADD_QGIS_TEST(composertabletest testqgscomposertable.cpp)
ADD_QGIS_TEST(composertablev2test testqgscomposertablev2.cpp)
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
ADD_QGIS_TEST(rasterfilltest testqgsrasterfill.cpp )
ADD_QGIS_TEST(shapebursttest testqgsshapeburst.cpp )
ADD_QGIS_TEST(invertedpolygontest testqgsinvertedpolygonrenderer.cpp )
ADD_QGIS_TEST(colorschemeregistry testqgscolorschemeregistry.cpp)
ADD_QGIS_TEST(colorscheme testqgscolorscheme.cpp)
ADD_QGIS_TEST(maptopixeltest testqgsmaptopixel.cpp)
ADD_QGIS_TEST(maprotationtest testqgsmaprotation.cpp)
ADD_QGIS_TEST(mapsettingstest testqgsmapsettings.cpp)
ADD_QGIS_TEST(networkcontentfetcher testqgsnetworkcontentfetcher.cpp )
ADD_QGIS_TEST(legendrenderertest testqgslegendrenderer.cpp )
ADD_QGIS_TEST(vectorlayerjoinbuffer testqgsvectorlayerjoinbuffer.cpp )
ADD_QGIS_TEST(maplayerstylemanager testqgsmaplayerstylemanager.cpp )
ADD_QGIS_TEST(pointlocatortest testqgspointlocator.cpp )
ADD_QGIS_TEST(snappingutilstest testqgssnappingutils.cpp )
ADD_QGIS_TEST(imageoperationtest testqgsimageoperation.cpp)
ADD_QGIS_TEST(painteffecttest testqgspainteffect.cpp)
ADD_QGIS_TEST(painteffectregistrytest testqgspainteffectregistry.cpp)
ADD_QGIS_TEST(statisticalsummarytest testqgsstatisticalsummary.cpp)
ADD_QGIS_TEST(histogramtest testqgshistogram.cpp)
ADD_QGIS_TEST(pallabelingtest testqgspallabeling.cpp)



