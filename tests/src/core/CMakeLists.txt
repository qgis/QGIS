# Standard includes and utils to compile into all tests.
#SET (util_SRCS qgscompositionchecker.cpp)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/composer
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/geonode
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/processing
  ${CMAKE_SOURCE_DIR}/src/core/processing/models
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_BINARY_DIR}/src/core
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  )

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree

MACRO (ADD_QGIS_TEST TESTSRC)
  SET (TESTNAME  ${TESTSRC})
  STRING(REPLACE "test" "" TESTNAME ${TESTNAME})
  STRING(REPLACE "qgs" "" TESTNAME ${TESTNAME})
  STRING(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  SET (TESTNAME  "qgis_${TESTNAME}test")
  ADD_EXECUTABLE(${TESTNAME} ${TESTSRC} ${util_SRCS})
  SET_TARGET_PROPERTIES(${TESTNAME} PROPERTIES AUTOMOC TRUE)
  TARGET_LINK_LIBRARIES(${TESTNAME}
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core)
  ADD_TEST(${TESTNAME} ${CMAKE_BINARY_DIR}/output/bin/${TESTNAME} -maxwarnings 10000)
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

SET(TESTS
 testqgs25drenderer.cpp
 testqgsapplication.cpp
 testqgsatlascomposition.cpp
 testqgsauthcrypto.cpp
 testqgsauthconfig.cpp
 testqgsauthmanager.cpp
 testqgsblendmodes.cpp
 testqgsclipper.cpp
 testqgscolorscheme.cpp
 testqgscolorschemeregistry.cpp
 testqgscomposerdd.cpp
 testqgscomposereffects.cpp
 testqgscomposergroup.cpp
 testqgscomposerlabel.cpp
 testqgscomposermapgrid.cpp
 testqgscomposermapoverview.cpp
 testqgscomposermap.cpp
 testqgscomposermodel.cpp
 testqgscomposermultiframe.cpp
 testqgscomposerobject.cpp
 testqgscomposerpaper.cpp
 testqgscomposerpicture.cpp
 testqgscomposerrotation.cpp
 testqgscomposerscalebar.cpp
 testqgscomposershapes.cpp
 testqgscomposertablev2.cpp
 testqgscomposerutils.cpp
 testqgsconnectionpool.cpp
 testcontrastenhancements.cpp
 testqgscoordinatereferencesystem.cpp
 testqgscoordinatetransform.cpp
 testqgscurve.cpp
 testqgsdatadefinedsizelegend.cpp
 testqgsdataitem.cpp
 testqgsdatasourceuri.cpp
 testqgsdiagram.cpp
 testqgsdistancearea.cpp
 testqgsellipsemarker.cpp
 testqgsexpressioncontext.cpp
 testqgsexpression.cpp
 testqgsfeature.cpp
 testqgsfields.cpp
 testqgsfield.cpp
 testqgsfilledmarker.cpp
 testqgsvectorfilewriter.cpp
 testqgsfontmarker.cpp
 testqgsgeometryimport.cpp
 testqgsgeometry.cpp
 testqgsgeometryutils.cpp
 testqgsgeonodeconnection.cpp
 testqgsgml.cpp
 testqgsgradients.cpp
 testqgsgraduatedsymbolrenderer.cpp
 testqgshistogram.cpp
 testqgsimageoperation.cpp
 testqgsinvertedpolygonrenderer.cpp
 testqgsjsonutils.cpp
 testqgslabelingengine.cpp
 testqgslayertree.cpp
 testqgslayout.cpp
 testqgslayoutcontext.cpp
 testqgslayoutitem.cpp
 testqgslayoutobject.cpp
 testqgslayoutpage.cpp
 testqgslayoutunits.cpp
 testqgslayoututils.cpp
 testqgslegendrenderer.cpp
 testqgscentroidfillsymbol.cpp
 testqgslinefillsymbol.cpp
 testqgsmaplayerstylemanager.cpp
 testqgsmaplayer.cpp
 testqgsmaprendererjob.cpp
 testqgsmaprotation.cpp
 testqgsmapsettings.cpp
 testqgsmapsettingsutils.cpp
 testqgsmaptopixelgeometrysimplifier.cpp
 testqgsmaptopixel.cpp
 testqgsmarkerlinesymbol.cpp
 testqgsnetworkcontentfetcher.cpp
 testqgsogcutils.cpp
 testqgsogrutils.cpp
 testqgspagesizeregistry.cpp
 testqgspainteffectregistry.cpp
 testqgspainteffect.cpp
 testqgspallabeling.cpp
 testqgspointlocator.cpp
 testqgspointpatternfillsymbol.cpp
 testqgspoint.cpp
 testqgsprocessing.cpp
 testqgsproject.cpp
 testqgsproperty.cpp
 testqgis.cpp
 testqgsrasterfilewriter.cpp
 testqgsrasterfill.cpp
 testqgsrasterblock.cpp
 testqgsrasterlayer.cpp
 testqgsrastersublayer.cpp
 testqgsrectangle.cpp
 testqgsrenderers.cpp
 testqgsrulebasedrenderer.cpp
 testqgsshapeburst.cpp
 testqgssimplemarker.cpp
 testqgssnappingutils.cpp
 testqgsspatialindex.cpp
 testqgsstatisticalsummary.cpp
 testqgsstringutils.cpp
 testqgsstyle.cpp
 testqgssvgmarker.cpp
 testqgssymbol.cpp
 testqgstaskmanager.cpp
 testqgstracer.cpp
 testqgsfontutils.cpp
 testqgsvectordataprovider.cpp
 testqgsvectorlayercache.cpp
 testqgsvectorlayerjoinbuffer.cpp
 testqgsvectorlayer.cpp
 testziplayer.cpp
    )

IF(WITH_QTWEBKIT)
  SET(TESTS ${TESTS}
      testqgscomposerhtml.cpp
      testqgscomposition.cpp
        )
ENDIF(WITH_QTWEBKIT)

FOREACH(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC})
ENDFOREACH(TESTSRC)

ADD_DEPENDENCIES(qgis_coordinatereferencesystemtest synccrsdb)
