#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/kdbush/include
  ${CMAKE_SOURCE_DIR}/external/nlohmann
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/callouts
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/labeling
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/numericformats
  ${CMAKE_SOURCE_DIR}/src/core/pal
  ${CMAKE_SOURCE_DIR}/src/core/processing
  ${CMAKE_SOURCE_DIR}/src/core/processing/models
  ${CMAKE_SOURCE_DIR}/src/core/providers/gdal
  ${CMAKE_SOURCE_DIR}/src/core/providers/ogr
  ${CMAKE_SOURCE_DIR}/src/core/providers/meshmemory
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/classification
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/vectortile
  ${CMAKE_SOURCE_DIR}/src/test

  ${CMAKE_BINARY_DIR}/src/core
)
INCLUDE_DIRECTORIES(SYSTEM
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALINDEX_INCLUDE_DIR}
  )

IF(HAVE_OPENCL)
    INCLUDE_DIRECTORIES(SYSTEM ${OpenCL_INCLUDE_DIRS})
ENDIF(HAVE_OPENCL)

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
  TARGET_LINK_LIBRARIES(${TESTNAME}
    ${Qt5Core_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    ${SPATIALINDEX_LIBRARY}
    qgis_core)
  ADD_TEST(${TESTNAME} ${CMAKE_BINARY_DIR}/output/bin/${TESTNAME} -maxwarnings 10000)
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

SET(TESTS
 testqgs25drenderer.cpp
 testqgsapplication.cpp
 testqgsauthcrypto.cpp
 testqgsauthcertutils.cpp
 testqgsauthconfig.cpp
 testqgsauthmanager.cpp
 testqgsblendmodes.cpp
 testqgsbrowsermodel.cpp
 testqgsbrowserproxymodel.cpp
 testqgscadutils.cpp
 testqgscallout.cpp
 testqgscalloutregistry.cpp
 testqgsclipper.cpp
 testqgscolorscheme.cpp
 testqgscolorschemeregistry.cpp
 testqgscompositionconverter.cpp
 testqgsconnectionpool.cpp
 testcontrastenhancements.cpp
 testqgscoordinatereferencesystem.cpp
 testqgscoordinatetransform.cpp
 testqgscredentials.cpp
 testqgscurve.cpp
 testqgsdatadefinedsizelegend.cpp
 testqgsdataitem.cpp
 testqgsdatasourceuri.cpp
 testqgsdiagram.cpp
 testqgsdistancearea.cpp
 testqgsdxfexport.cpp
 testqgsellipsemarker.cpp
 testqgsexpressioncontext.cpp
 testqgssqliteexpressioncompiler.cpp
 testqgsexpression.cpp
 testqgsfeature.cpp
 testqgsfields.cpp
 testqgsfield.cpp
 testqgsfilledmarker.cpp
 testqgsgdalutils.cpp
 testqgsvectorfilewriter.cpp
 testqgsfontmarker.cpp
 testqgsgenericspatialindex.cpp
 testqgsgeopdfexport.cpp
 testqgsgeometryimport.cpp
 testqgsgeometry.cpp
 testqgsgeometryutils.cpp
 testqgsgeonodeconnection.cpp
 testqgsgml.cpp
 testqgsgradients.cpp
 testqgsgraduatedsymbolrenderer.cpp
 testqgshistogram.cpp
 testqgshstoreutils.cpp
 testqgsimagecache.cpp
 testqgsimageoperation.cpp
 testqgsinternalgeometryengine.cpp
 testqgsinvertedpolygonrenderer.cpp
 testqgsjsonutils.cpp
 testqgslabelingengine.cpp
 testqgslayertree.cpp
 testqgslayout.cpp
 testqgslayoutatlas.cpp
 testqgslayoutcontext.cpp
 testqgslayoutexporter.cpp
 testqgslayouthtml.cpp
 testqgslayoutitem.cpp
 testqgslayoutitemgroup.cpp
 testqgslayoutgeopdfexport.cpp
 testqgslayoutlabel.cpp
 testqgslayoutmanualtable.cpp
 testqgslayoutmap.cpp
 testqgslayoutmapgrid.cpp
 testqgslayoutmapoverview.cpp
 testqgslayoutmodel.cpp
 testqgslayoutmultiframe.cpp
 testqgslayoutobject.cpp
 testqgslayoutpage.cpp
 testqgslayoutpicture.cpp
 testqgslayoutpolyline.cpp
 testqgslayoutscalebar.cpp
 testqgslayoutshapes.cpp
 testqgslayouttable.cpp
 testqgslayoutunits.cpp
 testqgslayoututils.cpp
 testqgslegendrenderer.cpp
 testqgscentroidfillsymbol.cpp
 testqgslinefillsymbol.cpp
 testqgsmapdevicepixelratio.cpp
 testqgsmaplayerstylemanager.cpp
 testqgsmaplayer.cpp
 testqgsmaprendererjob.cpp
 testqgsmaprotation.cpp
 testqgsmapsettings.cpp
 testqgsmapsettingsutils.cpp
 testqgsmapthemecollection.cpp
 testqgsmaptopixelgeometrysimplifier.cpp
 testqgsmaptopixel.cpp
 testqgsmarkerlinesymbol.cpp
 testqgsmesh3daveraging.cpp
 testqgsmeshlayer.cpp
 testqgsmeshlayerinterpolator.cpp
 testqgsmeshlayerrenderer.cpp
 testqgsnetworkaccessmanager.cpp
 testqgsnetworkcontentfetcher.cpp
 testqgsnewsfeedparser.cpp
 testqgsogcutils.cpp
 testqgsogrutils.cpp
 testqgspagesizeregistry.cpp
 testqgspainteffectregistry.cpp
 testqgspainteffect.cpp
 testqgspallabeling.cpp
 testqgspointlocator.cpp
 testqgspointpatternfillsymbol.cpp
 testqgspoint.cpp
 testqgsproject.cpp
 testqgsprojectstorage.cpp
 testqgsprojutils.cpp
 testqgsproperty.cpp
 testqgis.cpp
 testqgsrasterfilewriter.cpp
 testqgsrasterfill.cpp
 testqgsrastermarker.cpp
 testqgsrasteriterator.cpp
 testqgsrasterblock.cpp
 testqgsrastercontourrenderer.cpp
 testqgsrasterdataprovidertemporalcapabilities.cpp
 testqgsrasterlayer.cpp
 testqgsrasterlayertemporalproperties.cpp
 testqgsrastersublayer.cpp
 testqgsrectangle.cpp
 testqgsrelationreferencefieldformatter.cpp
 testqgsrenderers.cpp
 testqgsrulebasedrenderer.cpp
 testqgssettings.cpp
 testqgsshapeburst.cpp
 testqgssimplemarker.cpp
 testqgssnappingutils.cpp
 testqgsspatialindex.cpp
 testqgsspatialindexkdbush.cpp
 testqgsstatisticalsummary.cpp
 testqgsstringutils.cpp
 testqgsstyle.cpp
 testqgssvgcache.cpp
 testqgssvgmarker.cpp
 testqgssymbol.cpp
 testqgstaskmanager.cpp
 testqgstemporalproperty.cpp
 testqgstemporalrangeobject.cpp
 testqgstracer.cpp
 testqgstriangularmesh.cpp
 testqgsfontutils.cpp
 testqgsvaluerelationfieldformatter.cpp
 testqgsvector.cpp
 testqgsvectordataprovider.cpp
 testqgsvectorlayercache.cpp
 testqgsvectorlayerjoinbuffer.cpp
 testqgsvectorlayer.cpp
 testqgsvectorlayerutils.cpp
 testqgsvectortilelayer.cpp
 testqgsziputils.cpp
 testziplayer.cpp
 testqgslayerdefinition.cpp
 testqgssqliteutils.cpp
 testqgsmimedatautils.cpp
 testqgsofflineediting.cpp
 testqgstranslateproject.cpp
 testqobjectuniqueptr.cpp
 testqgspostgresstringutils.cpp
 testqgsstoredexpressionmanager.cpp
 testqgsweakrelation.cpp
)

IF(WITH_QTWEBKIT)
  SET(TESTS ${TESTS})
ENDIF(WITH_QTWEBKIT)

IF(HAVE_OPENCL)
  SET(TESTS ${TESTS}
    testqgsopenclutils.cpp
     )
ENDIF(HAVE_OPENCL)


FOREACH(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC})
ENDFOREACH(TESTSRC)

ADD_DEPENDENCIES(qgis_coordinatereferencesystemtest synccrsdb)
