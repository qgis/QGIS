#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/kdbush/include
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/callouts
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/pal
  ${CMAKE_SOURCE_DIR}/src/core/processing
  ${CMAKE_SOURCE_DIR}/src/core/processing/models
  ${CMAKE_SOURCE_DIR}/src/core/providers/gdal
  ${CMAKE_SOURCE_DIR}/src/core/providers/ogr
  ${CMAKE_SOURCE_DIR}/src/core/providers/meshmemory
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/mesh
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

MACRO (ADD_QGIS_TEST TESTSRC ADD_RESOURCE_EXECUTABLE)
  SET (TESTNAME  ${TESTSRC})
  STRING(REPLACE "test" "" TESTNAME ${TESTNAME})
  STRING(REPLACE "qgs" "" TESTNAME ${TESTNAME})
  STRING(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  SET (TESTNAME  "qgis_${TESTNAME}test")
  IF(${ADD_RESOURCE_EXECUTABLE})
    QT5_ADD_RESOURCES(IMAGES_RCC_SRC ${CMAKE_SOURCE_DIR}/images/images.qrc)
  ENDIF(${ADD_RESOURCE_EXECUTABLE})
  ADD_EXECUTABLE(${TESTNAME} ${TESTSRC} ${util_SRCS} ${IMAGES_RCC_SRC})
  SET_TARGET_PROPERTIES(${TESTNAME} PROPERTIES AUTOMOC TRUE)
  TARGET_LINK_LIBRARIES(${TESTNAME}
    ${Qt5Core_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core)
  ADD_TEST(${TESTNAME} ${CMAKE_BINARY_DIR}/output/bin/${TESTNAME} -maxwarnings 10000)
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

ADD_QGIS_TEST( testqgs25drenderer.cpp FALSE )
ADD_QGIS_TEST( testqgsapplication.cpp FALSE )
ADD_QGIS_TEST( testqgsauthcrypto.cpp FALSE )
ADD_QGIS_TEST( testqgsauthcertutils.cpp FALSE )
ADD_QGIS_TEST( testqgsauthconfig.cpp FALSE )
ADD_QGIS_TEST( testqgsauthmanager.cpp FALSE )
ADD_QGIS_TEST( testqgsblendmodes.cpp FALSE )
ADD_QGIS_TEST( testqgsbrowsermodel.cpp FALSE )
ADD_QGIS_TEST( testqgsbrowserproxymodel.cpp FALSE )
ADD_QGIS_TEST( testqgscadutils.cpp FALSE )
ADD_QGIS_TEST( testqgscallout.cpp FALSE )
ADD_QGIS_TEST( testqgscalloutregistry.cpp FALSE )
ADD_QGIS_TEST( testqgsclipper.cpp FALSE )
ADD_QGIS_TEST( testqgscolorscheme.cpp FALSE )
ADD_QGIS_TEST( testqgscolorschemeregistry.cpp FALSE )
ADD_QGIS_TEST( testqgscompositionconverter.cpp FALSE )
ADD_QGIS_TEST( testqgsconnectionpool.cpp FALSE )
ADD_QGIS_TEST( testcontrastenhancements.cpp FALSE )
ADD_QGIS_TEST( testqgscoordinatereferencesystem.cpp FALSE )
ADD_QGIS_TEST( testqgscoordinatetransform.cpp FALSE )
ADD_QGIS_TEST( testqgscredentials.cpp FALSE )
ADD_QGIS_TEST( testqgscurve.cpp FALSE )
ADD_QGIS_TEST( testqgsdatadefinedsizelegend.cpp FALSE )
ADD_QGIS_TEST( testqgsdataitem.cpp FALSE )
ADD_QGIS_TEST( testqgsdatasourceuri.cpp FALSE )
ADD_QGIS_TEST( testqgsdiagram.cpp FALSE )
ADD_QGIS_TEST( testqgsdistancearea.cpp FALSE )
ADD_QGIS_TEST( testqgsdxfexport.cpp FALSE )
ADD_QGIS_TEST( testqgsellipsemarker.cpp FALSE )
ADD_QGIS_TEST( testqgsexpressioncontext.cpp FALSE )
ADD_QGIS_TEST( testqgssqliteexpressioncompiler.cpp FALSE )
ADD_QGIS_TEST( testqgsexpression.cpp FALSE )
ADD_QGIS_TEST( testqgsfeature.cpp FALSE )
ADD_QGIS_TEST( testqgsfields.cpp FALSE )
ADD_QGIS_TEST( testqgsfield.cpp FALSE )
ADD_QGIS_TEST( testqgsfilledmarker.cpp FALSE )
ADD_QGIS_TEST( testqgsgdalutils.cpp FALSE )
ADD_QGIS_TEST( testqgsvectorfilewriter.cpp FALSE )
ADD_QGIS_TEST( testqgsfontmarker.cpp FALSE )
ADD_QGIS_TEST( testqgsgeopdfexport.cpp FALSE )
ADD_QGIS_TEST( testqgsgeometryimport.cpp FALSE )
ADD_QGIS_TEST( testqgsgeometry.cpp FALSE )
ADD_QGIS_TEST( testqgsgeometryutils.cpp FALSE )
ADD_QGIS_TEST( testqgsgeonodeconnection.cpp FALSE )
ADD_QGIS_TEST( testqgsgml.cpp FALSE )
ADD_QGIS_TEST( testqgsgradients.cpp FALSE )
ADD_QGIS_TEST( testqgsgraduatedsymbolrenderer.cpp FALSE )
ADD_QGIS_TEST( testqgshistogram.cpp FALSE )
ADD_QGIS_TEST( testqgshstoreutils.cpp FALSE )
ADD_QGIS_TEST( testqgsimagecache.cpp FALSE )
ADD_QGIS_TEST( testqgsimageoperation.cpp FALSE )
ADD_QGIS_TEST( testqgsinternalgeometryengine.cpp FALSE )
ADD_QGIS_TEST( testqgsinvertedpolygonrenderer.cpp FALSE )
ADD_QGIS_TEST( testqgsjsonutils.cpp FALSE )
ADD_QGIS_TEST( testqgslabelingengine.cpp FALSE )
ADD_QGIS_TEST( testqgslayertree.cpp FALSE )
ADD_QGIS_TEST( testqgslayout.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutatlas.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutcontext.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutexporter.cpp FALSE )
ADD_QGIS_TEST( testqgslayouthtml.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutitem.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutitemgroup.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutgeopdfexport.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutlabel.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutmap.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutmapgrid.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutmapoverview.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutmodel.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutmultiframe.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutobject.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutpage.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutpicture.cpp TRUE )
ADD_QGIS_TEST( testqgslayoutpolyline.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutscalebar.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutshapes.cpp FALSE )
ADD_QGIS_TEST( testqgslayouttable.cpp FALSE )
ADD_QGIS_TEST( testqgslayoutunits.cpp FALSE )
ADD_QGIS_TEST( testqgslayoututils.cpp FALSE )
ADD_QGIS_TEST( testqgslegendrenderer.cpp FALSE )
ADD_QGIS_TEST( testqgscentroidfillsymbol.cpp FALSE )
ADD_QGIS_TEST( testqgslinefillsymbol.cpp FALSE )
ADD_QGIS_TEST( testqgsmapdevicepixelratio.cpp FALSE )
ADD_QGIS_TEST( testqgsmaplayerstylemanager.cpp FALSE )
ADD_QGIS_TEST( testqgsmaplayer.cpp FALSE )
ADD_QGIS_TEST( testqgsmaprendererjob.cpp FALSE )
ADD_QGIS_TEST( testqgsmaprotation.cpp FALSE )
ADD_QGIS_TEST( testqgsmapsettings.cpp FALSE )
ADD_QGIS_TEST( testqgsmapsettingsutils.cpp FALSE )
ADD_QGIS_TEST( testqgsmapthemecollection.cpp FALSE )
ADD_QGIS_TEST( testqgsmaptopixelgeometrysimplifier.cpp FALSE )
ADD_QGIS_TEST( testqgsmaptopixel.cpp FALSE )
ADD_QGIS_TEST( testqgsmarkerlinesymbol.cpp FALSE )
ADD_QGIS_TEST( testqgsmeshlayer.cpp FALSE )
ADD_QGIS_TEST( testqgsmeshlayerinterpolator.cpp FALSE )
ADD_QGIS_TEST( testqgsmeshlayerrenderer.cpp FALSE )
ADD_QGIS_TEST( testqgsnetworkaccessmanager.cpp FALSE )
ADD_QGIS_TEST( testqgsnetworkcontentfetcher.cpp FALSE )
ADD_QGIS_TEST( testqgsnewsfeedparser.cpp FALSE )
ADD_QGIS_TEST( testqgsogcutils.cpp FALSE )
ADD_QGIS_TEST( testqgsogrutils.cpp FALSE )
ADD_QGIS_TEST( testqgspagesizeregistry.cpp FALSE )
ADD_QGIS_TEST( testqgspainteffectregistry.cpp FALSE )
ADD_QGIS_TEST( testqgspainteffect.cpp FALSE )
ADD_QGIS_TEST( testqgspallabeling.cpp FALSE )
ADD_QGIS_TEST( testqgspointlocator.cpp FALSE )
ADD_QGIS_TEST( testqgspointpatternfillsymbol.cpp FALSE )
ADD_QGIS_TEST( testqgspoint.cpp FALSE )
ADD_QGIS_TEST( testqgsproject.cpp FALSE )
ADD_QGIS_TEST( testqgsprojectstorage.cpp FALSE )
ADD_QGIS_TEST( testqgsprojutils.cpp FALSE )
ADD_QGIS_TEST( testqgsproperty.cpp FALSE )
ADD_QGIS_TEST( testqgis.cpp FALSE )
ADD_QGIS_TEST( testqgsrasterfilewriter.cpp FALSE )
ADD_QGIS_TEST( testqgsrasterfill.cpp FALSE )
ADD_QGIS_TEST( testqgsrastermarker.cpp FALSE )
ADD_QGIS_TEST( testqgsrasteriterator.cpp FALSE )
ADD_QGIS_TEST( testqgsrasterblock.cpp FALSE )
ADD_QGIS_TEST( testqgsrasterlayer.cpp FALSE )
ADD_QGIS_TEST( testqgsrastersublayer.cpp FALSE )
ADD_QGIS_TEST( testqgsrectangle.cpp FALSE )
ADD_QGIS_TEST( testqgsrenderers.cpp FALSE )
ADD_QGIS_TEST( testqgsrulebasedrenderer.cpp FALSE )
ADD_QGIS_TEST( testqgssettings.cpp FALSE )
ADD_QGIS_TEST( testqgsshapeburst.cpp FALSE )
ADD_QGIS_TEST( testqgssimplemarker.cpp FALSE )
ADD_QGIS_TEST( testqgssnappingutils.cpp FALSE )
ADD_QGIS_TEST( testqgsspatialindex.cpp FALSE )
ADD_QGIS_TEST( testqgsspatialindexkdbush.cpp FALSE )
ADD_QGIS_TEST( testqgsstatisticalsummary.cpp FALSE )
ADD_QGIS_TEST( testqgsstringutils.cpp FALSE )
ADD_QGIS_TEST( testqgsstyle.cpp FALSE )
ADD_QGIS_TEST( testqgssvgcache.cpp FALSE )
ADD_QGIS_TEST( testqgssvgmarker.cpp FALSE )
ADD_QGIS_TEST( testqgssymbol.cpp FALSE )
ADD_QGIS_TEST( testqgstaskmanager.cpp FALSE )
ADD_QGIS_TEST( testqgstracer.cpp FALSE )
ADD_QGIS_TEST( testqgstriangularmesh.cpp FALSE )
ADD_QGIS_TEST( testqgsfontutils.cpp FALSE )
ADD_QGIS_TEST( testqgsvector.cpp FALSE )
ADD_QGIS_TEST( testqgsvectordataprovider.cpp FALSE )
ADD_QGIS_TEST( testqgsvectorlayercache.cpp FALSE )
ADD_QGIS_TEST( testqgsvectorlayerjoinbuffer.cpp FALSE )
ADD_QGIS_TEST( testqgsvectorlayer.cpp FALSE )
ADD_QGIS_TEST( testqgsvectorlayerutils.cpp FALSE )
ADD_QGIS_TEST( testqgsziputils.cpp FALSE )
ADD_QGIS_TEST( testziplayer.cpp FALSE )
ADD_QGIS_TEST( testqgslayerdefinition.cpp FALSE )
ADD_QGIS_TEST( testqgssqliteutils.cpp FALSE )
ADD_QGIS_TEST( testqgsmimedatautils.cpp FALSE )
ADD_QGIS_TEST( testqgsofflineediting.cpp FALSE )
ADD_QGIS_TEST( testqgstranslateproject.cpp FALSE )
ADD_QGIS_TEST( testqobjectuniqueptr.cpp FALSE )
ADD_QGIS_TEST( testqgspostgresstringutils.cpp FALSE )

IF(HAVE_OPENCL)
  ADD_QGIS_TEST( testqgsopenclutils.cpp FALSE )
ENDIF(HAVE_OPENCL)


ADD_DEPENDENCIES(qgis_coordinatereferencesystemtest synccrsdb)
