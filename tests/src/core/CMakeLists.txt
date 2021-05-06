#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/external/kdbush/include
  ${CMAKE_SOURCE_DIR}/src/test
)

if(HAVE_OPENCL)
    include_directories(SYSTEM ${OpenCL_INCLUDE_DIRS})
endif()

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree

macro (ADD_QGIS_TEST TESTSRC)
  set (TESTNAME  ${TESTSRC})
  string(REPLACE "test" "" TESTNAME ${TESTNAME})
  string(REPLACE "qgs" "" TESTNAME ${TESTNAME})
  string(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  set (TESTNAME  "qgis_${TESTNAME}test")
  add_executable(${TESTNAME} ${TESTSRC} ${util_SRCS})
  target_compile_features(${TESTNAME} PRIVATE cxx_std_17)
  target_link_libraries(${TESTNAME}
    ${${QT_VERSION_BASE}Core_LIBRARIES}
    ${${QT_VERSION_BASE}Xml_LIBRARIES}
    ${${QT_VERSION_BASE}Svg_LIBRARIES}
    ${${QT_VERSION_BASE}Test_LIBRARIES}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    ${SPATIALINDEX_LIBRARY}
    qgis_core)
  add_test(${TESTNAME} ${CMAKE_BINARY_DIR}/output/bin/${TESTNAME} -maxwarnings 10000)
endmacro (ADD_QGIS_TEST)

#############################################################
# Tests:

set(TESTS
 testqgs25drenderer.cpp
 testqgsannotationitemregistry.cpp
 testqgsapplication.cpp
 testqgsarcgisrestutils.cpp
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
 testqgscoordinatereferencesystemregistry.cpp
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
 testqgsoverlayexpression.cpp
 testqgsfeature.cpp
 testqgsfields.cpp
 testqgsfield.cpp
 testqgsfilledmarker.cpp
 testqgsgdalprovider.cpp
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
 testqgsmaprenderercache.cpp
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
 testqgsogrprovider.cpp
 testqgsogrutils.cpp
 testqgspagesizeregistry.cpp
 testqgspainteffectregistry.cpp
 testqgspainteffect.cpp
 testqgspallabeling.cpp
 testqgspointlocator.cpp
 testqgspointpatternfillsymbol.cpp
 testqgspoint.cpp
 testqgspointcloudattribute.cpp
 testqgspointcloudrendererregistry.cpp
 testqgsproject.cpp
 testqgsprojectstorage.cpp
 testqgsprojutils.cpp
 testqgsproperty.cpp
 testqgsprovidermetadata.cpp
 testqgis.cpp
 testqgsrange.cpp
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
 testqgsruntimeprofiler.cpp
 testqgssettings.cpp
 testqgssettingsentry.cpp
 testqgssettingsregistry.cpp
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
 testqgstemporalnavigationobject.cpp
 testqgstiledownloadmanager.cpp
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
 testqgsvectortilewriter.cpp
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

if(WITH_QTWEBKIT)
  set(TESTS ${TESTS})
endif()

if(HAVE_OPENCL)
  set(TESTS ${TESTS}
    testqgsopenclutils.cpp
     )
endif()


foreach(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC})
endforeach(TESTSRC)

if (ENABLE_PGTEST)
  SET_TESTS_PROPERTIES(qgis_vectorlayerjoinbuffertest PROPERTIES LABELS "POSTGRES")
endif()

add_dependencies(qgis_coordinatereferencesystemtest synccrsdb)
