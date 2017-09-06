############################################################
# sources

SET(QGIS_CORE_SRCS
  gps/context.c
  gps/gmath.c
  gps/info.c
  gps/parse.c
  gps/qgsgpsconnection.cpp
  gps/qgsgpsconnectionregistry.cpp
  gps/qgsgpsdconnection.cpp
  gps/qgsgpsdetector.cpp
  gps/qgsnmeaconnection.cpp
  gps/sentence.c
  gps/time.c
  gps/tok.c

  symbology/qgs25drenderer.cpp
  symbology/qgsarrowsymbollayer.cpp
  symbology/qgscategorizedsymbolrenderer.cpp
  symbology/qgscolorbrewerpalette.cpp
  symbology/qgscptcityarchive.cpp
  symbology/qgsellipsesymbollayer.cpp
  symbology/qgsfillsymbollayer.cpp
  symbology/qgsgeometrygeneratorsymbollayer.cpp
  symbology/qgsgraduatedsymbolrenderer.cpp
  symbology/qgsheatmaprenderer.cpp
  symbology/qgsinvertedpolygonrenderer.cpp
  symbology/qgslegendsymbolitem.cpp
  symbology/qgslinesymbollayer.cpp
  symbology/qgsmarkersymbollayer.cpp
  symbology/qgsnullsymbolrenderer.cpp
  symbology/qgspointclusterrenderer.cpp
  symbology/qgspointdisplacementrenderer.cpp
  symbology/qgspointdistancerenderer.cpp
  symbology/qgsrenderer.cpp
  symbology/qgsrendererregistry.cpp
  symbology/qgsrulebasedrenderer.cpp
  symbology/qgssinglesymbolrenderer.cpp
  symbology/qgsstyle.cpp
  symbology/qgssvgcache.cpp
  symbology/qgssymbollayer.cpp
  symbology/qgssymbollayerregistry.cpp
  symbology/qgssymbollayerutils.cpp
  symbology/qgssymbol.cpp
  symbology/qgsvectorfieldsymbollayer.cpp

  simplify/effectivearea.cpp

  diagram/qgsdiagram.cpp
  diagram/qgshistogramdiagram.cpp
  diagram/qgspiediagram.cpp
  diagram/qgstextdiagram.cpp

  effects/qgsblureffect.cpp
  effects/qgscoloreffect.cpp
  effects/qgseffectstack.cpp
  effects/qgsgloweffect.cpp
  effects/qgsimageoperation.cpp
  effects/qgspainteffect.cpp
  effects/qgspainteffectregistry.cpp
  effects/qgsshadoweffect.cpp
  effects/qgstransformeffect.cpp

  layertree/qgslayertreegroup.cpp
  layertree/qgslayertreelayer.cpp
  layertree/qgslayertreemodel.cpp
  layertree/qgslayertreemodellegendnode.cpp
  layertree/qgslayertreenode.cpp
  layertree/qgslayertreeregistrybridge.cpp
  layertree/qgslayertreeutils.cpp
  layertree/qgslayertree.cpp

  metadata/qgslayermetadata.cpp
  metadata/qgslayermetadatavalidator.cpp

  auth/qgsauthcertutils.cpp
  auth/qgsauthconfig.cpp
  auth/qgsauthcrypto.cpp
  auth/qgsauthmanager.cpp
  auth/qgsauthmethodmetadata.cpp
  auth/qgsauthmethodregistry.cpp

  annotations/qgsannotation.cpp
  annotations/qgsannotationmanager.cpp
  annotations/qgshtmlannotation.cpp
  annotations/qgssvgannotation.cpp
  annotations/qgstextannotation.cpp

  expression/qgsexpression.cpp
  expression/qgsexpressionnode.cpp
  expression/qgsexpressionnodeimpl.cpp
  expression/qgsexpressionfunction.cpp
  expression/qgsexpressionutils.cpp

  processing/qgsnativealgorithms.cpp
  processing/qgsprocessingalgorithm.cpp
  processing/qgsprocessingalgrunnertask.cpp
  processing/qgsprocessingoutputs.cpp
  processing/qgsprocessingparameters.cpp
  processing/qgsprocessingprovider.cpp
  processing/qgsprocessingregistry.cpp
  processing/qgsprocessingutils.cpp
  processing/models/qgsprocessingmodelalgorithm.cpp
  processing/models/qgsprocessingmodelchildalgorithm.cpp
  processing/models/qgsprocessingmodelchildparametersource.cpp
  processing/models/qgsprocessingmodelcomponent.cpp
  processing/models/qgsprocessingmodelparameter.cpp
  processing/models/qgsprocessingmodeloutput.cpp

  providers/memory/qgsmemoryfeatureiterator.cpp
  providers/memory/qgsmemoryprovider.cpp
  providers/memory/qgsmemoryproviderutils.cpp

  scalebar/qgsdoubleboxscalebarrenderer.cpp
  scalebar/qgsnumericscalebarrenderer.cpp
  scalebar/qgsscalebarrenderer.cpp
  scalebar/qgssingleboxscalebarrenderer.cpp
  scalebar/qgsticksscalebarrenderer.cpp

  qgis.cpp
  qgsapplication.cpp
  qgsaction.cpp
  qgsactionscope.cpp
  qgsactionscoperegistry.cpp
  qgsactionmanager.cpp
  qgsaggregatecalculator.cpp
  qgsanimatedicon.cpp
  qgsattributes.cpp
  qgsattributetableconfig.cpp
  qgsattributeeditorelement.cpp
  qgsbearingutils.cpp
  qgsbrowsermodel.cpp
  qgscachedfeatureiterator.cpp
  qgscacheindex.cpp
  qgscacheindexfeatureid.cpp
  qgsclipper.cpp
  qgscolorramp.cpp
  qgscolorscheme.cpp
  qgscolorschemeregistry.cpp
  qgsconditionalstyle.cpp
  qgscoordinatereferencesystem.cpp
  qgscoordinatetransform.cpp
  qgscoordinatetransform_p.cpp
  qgscoordinateutils.cpp
  qgscredentials.cpp
  qgscrscache.cpp
  qgsdartmeasurement.cpp
  qgsdatadefinedsizelegend.cpp
  qgsdataitem.cpp
  qgsdataitemprovider.cpp
  qgsdataitemproviderregistry.cpp
  qgsdatasourceuri.cpp
  qgsdataprovider.cpp
  qgsdatetimestatisticalsummary.cpp
  qgsdatumtransformstore.cpp
  qgsdbfilterproxymodel.cpp
  qgsdiagramrenderer.cpp
  qgsdistancearea.cpp
  qgseditformconfig.cpp
  qgsellipsoidutils.cpp
  qgserror.cpp
  qgsexpressioncontext.cpp
  qgsexpressionfieldbuffer.cpp
  qgsfeature.cpp
  qgsfeatureiterator.cpp
  qgsfeaturerequest.cpp
  qgsfeaturesink.cpp
  qgsfeaturesource.cpp
  qgsfeaturestore.cpp
  qgsfield.cpp
  qgsfieldconstraints.cpp
  qgsfieldformatter.cpp
  qgsfieldformatterregistry.cpp
  qgsfieldmodel.cpp
  qgsfieldproxymodel.cpp
  qgsfields.cpp
  qgsfontutils.cpp
  qgsgeometrysimplifier.cpp
  qgsgeometryvalidator.cpp
  qgsgml.cpp
  qgsgmlschema.cpp
  qgshistogram.cpp
  qgsinterval.cpp
  qgsjsonutils.cpp
  qgslabelfeature.cpp
  qgslabelingengine.cpp
  qgslabelingenginesettings.cpp
  qgslabelsearchtree.cpp
  qgslayerdefinition.cpp
  qgslegendrenderer.cpp
  qgslegendsettings.cpp
  qgslegendstyle.cpp
  qgslocalec.cpp
  qgslogger.cpp
  qgsmapdecoration.cpp
  qgsmaphittest.cpp
  qgsmaplayer.cpp
  qgsmaplayerlegend.cpp
  qgsmaplayermodel.cpp
  qgsmaplayerproxymodel.cpp
  qgsmaplayerstore.cpp
  qgsmaplayerstylemanager.cpp
  qgsmaprenderercache.cpp
  qgsmaprenderercustompainterjob.cpp
  qgsmaprendererjob.cpp
  qgsmaprendererparalleljob.cpp
  qgsmaprenderersequentialjob.cpp
  qgsmaprenderertask.cpp
  qgsmapsettings.cpp
  qgsmapsettingsutils.cpp
  qgsmaptopixel.cpp
  qgsmaptopixelgeometrysimplifier.cpp
  qgsmapunitscale.cpp
  qgsmargins.cpp
  qgsmessagelog.cpp
  qgsmessageoutput.cpp
  qgsmimedatautils.cpp
  qgsmultirenderchecker.cpp
  qgsnetworkaccessmanager.cpp
  qgsnetworkdiskcache.cpp
  qgsnetworkcontentfetcher.cpp
  qgsnetworkreplyparser.cpp
  qgsobjectcustomproperties.cpp
  qgsofflineediting.cpp
  qgsogcutils.cpp
  qgsogrutils.cpp
  qgsoptionalexpression.cpp
  qgsowsconnection.cpp
  qgspaintenginehack.cpp
  qgspainting.cpp
  qgspallabeling.cpp
  qgspathresolver.cpp
  qgspluginlayer.cpp
  qgspluginlayerregistry.cpp
  qgspointxy.cpp
  qgspointlocator.cpp
  qgsproject.cpp
  qgsprojectbadlayerhandler.cpp
  qgsprojectfiletransform.cpp
  qgssnappingconfig.cpp
  qgsprojectproperty.cpp
  qgsprojectversion.cpp
  qgsproperty.cpp
  qgspropertycollection.cpp
  qgspropertytransformer.cpp
  qgsprovidermetadata.cpp
  qgsproviderregistry.cpp
  qgspythonrunner.cpp
  qgsreadwritecontext.cpp
  qgsrelation.cpp
  qgsrelationmanager.cpp
  qgsrenderchecker.cpp
  qgsrendercontext.cpp
  qgsrulebasedlabeling.cpp
  qgsrunprocess.cpp
  qgsruntimeprofiler.cpp
  qgsscalecalculator.cpp
  qgsscaleutils.cpp
  qgssimplifymethod.cpp
  qgsslconnect.cpp
  qgssnappingutils.cpp
  qgsspatialindex.cpp
  qgssqlexpressioncompiler.cpp
  qgssqliteexpressioncompiler.cpp
  qgssqlstatement.cpp
  qgsstatisticalsummary.cpp
  qgsstringstatisticalsummary.cpp
  qgsstringutils.cpp
  qgstaskmanager.cpp
  qgstextlabelfeature.cpp
  qgstextrenderer.cpp
  qgstolerance.cpp
  qgstracer.cpp
  qgstrackedvectorlayertools.cpp
  qgstransaction.cpp
  qgstransactiongroup.cpp
  qgsunittypes.cpp
  qgsvector.cpp
  qgsvectordataprovider.cpp
  qgsvectorfilewriter.cpp
  qgsvectorfilewritertask.cpp
  qgsvectorlayer.cpp
  qgsvectorlayerfeaturecounter.cpp
  qgsvectorlayercache.cpp
  qgsvectorlayerdiagramprovider.cpp
  qgsvectorlayereditbuffer.cpp
  qgsvectorlayereditpassthrough.cpp
  qgsvectorlayereditutils.cpp
  qgsvectorlayerfeatureiterator.cpp
  qgsvectorlayerexporter.cpp
  qgsvectorlayerjoinbuffer.cpp
  qgsvectorlayerjoininfo.cpp
  qgsvectorlayerlabeling.cpp
  qgsvectorlayerlabelprovider.cpp
  qgsvectorlayerrenderer.cpp
  qgsvectorlayertools.cpp
  qgsvectorlayerundocommand.cpp
  qgsvectorlayerutils.cpp
  qgsvectorsimplifymethod.cpp
  qgsvirtuallayerdefinition.cpp
  qgsvirtuallayerdefinitionutils.cpp
  qgsmapthemecollection.cpp
  qgsxmlutils.cpp
  qgssettings.cpp
  qgsstacktrace.cpp
  qgsarchive.cpp
  qgsziputils.cpp

  composer/qgsaddremoveitemcommand.cpp
  composer/qgsaddremovemultiframecommand.cpp
  composer/qgsatlascomposition.cpp
  composer/qgscomposerarrow.cpp
  composer/qgscomposerattributetablemodelv2.cpp
  composer/qgscomposerattributetablev2.cpp
  composer/qgscomposereffect.cpp
  composer/qgscomposerframe.cpp
  composer/qgscomposerhtml.cpp
  composer/qgscomposeritem.cpp
  composer/qgscomposeritemcommand.cpp
  composer/qgscomposeritemgroup.cpp
  composer/qgscomposerlabel.cpp
  composer/qgscomposerlegend.cpp
  composer/qgscomposermap.cpp
  composer/qgscomposermapgrid.cpp
  composer/qgscomposermapitem.cpp
  composer/qgscomposermapoverview.cpp
  composer/qgscomposermodel.cpp
  composer/qgscomposermousehandles.cpp
  composer/qgscomposermultiframe.cpp
  composer/qgscomposermultiframecommand.cpp
  composer/qgscomposerobject.cpp
  composer/qgscomposerpicture.cpp
  composer/qgscomposerscalebar.cpp
  composer/qgscomposershape.cpp
  composer/qgscomposernodesitem.cpp
  composer/qgscomposerpolygon.cpp
  composer/qgscomposerpolyline.cpp
  composer/qgscomposertablecolumn.cpp
  composer/qgscomposertablev2.cpp
  composer/qgscomposertexttable.cpp
  composer/qgscomposerutils.cpp
  composer/qgscomposition.cpp
  composer/qgsgroupungroupitemscommand.cpp
  composer/qgslayoutmanager.cpp
  composer/qgspaperitem.cpp

  dxf/qgsdxfexport.cpp
  dxf/qgsdxfpaintdevice.cpp
  dxf/qgsdxfpaintengine.cpp
  dxf/qgsdxfpallabeling.cpp

  layout/qgslayout.cpp
  layout/qgslayoutcontext.cpp
  layout/qgslayoutgridsettings.cpp
  layout/qgslayoutguidecollection.cpp
  layout/qgslayoutitem.cpp
  layout/qgslayoutitemmap.cpp
  layout/qgslayoutitempage.cpp
  layout/qgslayoutitemregistry.cpp
  layout/qgslayoutitemshape.cpp
  layout/qgslayoutitemundocommand.cpp
  layout/qgslayoutmeasurement.cpp
  layout/qgslayoutmeasurementconverter.cpp
  layout/qgslayoutobject.cpp
  layout/qgslayoutpagecollection.cpp
  layout/qgslayoutserializableobject.cpp
  layout/qgslayoutsnapper.cpp
  layout/qgslayoutundocommand.cpp
  layout/qgslayoutundostack.cpp
  layout/qgslayoututils.cpp
  layout/qgspagesizeregistry.cpp
  layout/qgslayoutpoint.cpp
  layout/qgslayoutserializableobject.cpp
  layout/qgslayoutsize.cpp

  pal/costcalculator.cpp
  pal/feature.cpp
  pal/geomfunction.cpp
  pal/labelposition.cpp
  pal/layer.cpp
  pal/pal.cpp
  pal/palstat.cpp
  pal/pointset.cpp
  pal/priorityqueue.cpp
  pal/problem.cpp
  pal/util.cpp

  raster/qgscliptominmaxenhancement.cpp
  raster/qgscolorrampshader.cpp
  raster/qgscontrastenhancement.cpp
  raster/qgscontrastenhancementfunction.cpp
  raster/qgslinearminmaxenhancement.cpp
  raster/qgslinearminmaxenhancementwithclip.cpp
  raster/qgsraster.cpp
  raster/qgsrasterblock.cpp
  raster/qgsrasterchecker.cpp
  raster/qgsrasterdataprovider.cpp
  raster/qgsrasterfilewritertask.cpp
  raster/qgsrasteridentifyresult.cpp
  raster/qgsrasterinterface.cpp
  raster/qgsrasteriterator.cpp
  raster/qgsrasterlayer.cpp
  raster/qgsrasterlayerrenderer.cpp
  raster/qgsrasterminmaxorigin.cpp
  raster/qgsrasternuller.cpp
  raster/qgsrasterpipe.cpp
  raster/qgsrasterprojector.cpp
  raster/qgsrasterrange.cpp
  raster/qgsrastershader.cpp
  raster/qgsrastershaderfunction.cpp
  raster/qgsrastertransparency.cpp

  raster/qgsbilinearrasterresampler.cpp
  raster/qgsbrightnesscontrastfilter.cpp
  raster/qgscubicrasterresampler.cpp
  raster/qgshuesaturationfilter.cpp
  raster/qgsmultibandcolorrenderer.cpp
  raster/qgspalettedrasterrenderer.cpp
  raster/qgsrasterdrawer.cpp
  raster/qgsrasterfilewriter.cpp
  raster/qgsrasterrenderer.cpp
  raster/qgsrasterrendererregistry.cpp
  raster/qgsrasterresamplefilter.cpp
  raster/qgssinglebandcolordatarenderer.cpp
  raster/qgssinglebandgrayrenderer.cpp
  raster/qgssinglebandpseudocolorrenderer.cpp
  raster/qgshillshaderenderer.cpp

  geometry/qgsabstractgeometry.cpp
  geometry/qgsbox3d.cpp
  geometry/qgscircle.cpp
  geometry/qgscircularstring.cpp
  geometry/qgscompoundcurve.cpp
  geometry/qgscurvepolygon.cpp
  geometry/qgscurve.cpp
  geometry/qgsellipse.cpp
  geometry/qgsgeometry.cpp
  geometry/qgsgeometrycollection.cpp
  geometry/qgsgeometryeditutils.cpp
  geometry/qgsgeometryfactory.cpp
  geometry/qgsgeometrymakevalid.cpp
  geometry/qgsgeometryutils.cpp
  geometry/qgsgeos.cpp
  geometry/qgsinternalgeometryengine.cpp
  geometry/qgslinestring.cpp
  geometry/qgsmulticurve.cpp
  geometry/qgsmultilinestring.cpp
  geometry/qgsmultipoint.cpp
  geometry/qgsmultipolygon.cpp
  geometry/qgsmultisurface.cpp
  geometry/qgspoint.cpp
  geometry/qgspolygon.cpp
  geometry/qgsrectangle.cpp
  geometry/qgsreferencedgeometry.cpp
  geometry/qgsregularpolygon.cpp
  geometry/qgstriangle.cpp
  geometry/qgswkbptr.cpp
  geometry/qgswkbtypes.cpp

  fieldformatter/qgsdatetimefieldformatter.cpp
  fieldformatter/qgsfallbackfieldformatter.cpp
  fieldformatter/qgskeyvaluefieldformatter.cpp
  fieldformatter/qgslistfieldformatter.cpp
  fieldformatter/qgsrelationreferencefieldformatter.cpp
  fieldformatter/qgsvaluemapfieldformatter.cpp
  fieldformatter/qgsvaluerelationfieldformatter.cpp

  geocms/geonode/qgsgeonodeconnection.cpp
  geocms/geonode/qgsgeonoderequest.cpp

  ${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp

  qgsuserprofile.cpp
  qgsuserprofilemanager.cpp
)


FILE(GLOB JSON_HELP_FILES "${CMAKE_SOURCE_DIR}/resources/function_help/json/*")
IF(NOT USING_NINJA)
  STRING(REPLACE "$" "$$" JSON_HELP_FILES "${JSON_HELP_FILES}")
ENDIF(NOT USING_NINJA)
STRING(REPLACE "\(" "\\(" JSON_HELP_FILES "${JSON_HELP_FILES}")
STRING(REPLACE "\)" "\\)" JSON_HELP_FILES "${JSON_HELP_FILES}")
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/process_function_template.py ${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp.temp
  COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp.temp -DDST=${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp -P ${CMAKE_SOURCE_DIR}/cmake/CopyIfChanged.cmake
  DEPENDS ${JSON_HELP_FILES}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

IF(ENABLE_MODELTEST)
  SET(MODELTEST_SRCS
    ../../tests/qt_modeltest/modeltest.cpp
    ../../tests/qt_modeltest/dynamictreemodel.cpp
  )
  SET(MODELTEST_MOC_HDRS
    ../../tests/qt_modeltest/modeltest.h
    ../../tests/qt_modeltest/dynamictreemodel.h
  )
  QT5_WRAP_CPP(MODELTEST_MOC_SRCS ${MODELTEST_MOC_HDRS})
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    ${MODELTEST_SRCS} ${MODELTEST_MOC_SRCS}
  )
ENDIF(ENABLE_MODELTEST)

IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # spatialindex headers produce warnings
  SET_SOURCE_FILES_PROPERTIES(qgsspatialindex.cpp PROPERTIES COMPILE_FLAGS -Wno-overloaded-virtual)
ENDIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")

IF (WITH_INTERNAL_QEXTSERIALPORT)
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    gps/qextserialport/qextserialport.cpp
    gps/qextserialport/qextserialenumerator.cpp
  )
  IF(WIN32)
    SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
      gps/qextserialport/win_qextserialport.cpp
    )
    ADD_DEFINITIONS(-D_TTY_WIN_)

  ELSE(WIN32)
    SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
      gps/qextserialport/posix_qextserialport.cpp
    )
    ADD_DEFINITIONS(-D_TTY_POSIX_)
  ENDIF(WIN32)
ELSE (WITH_INTERNAL_QEXTSERIALPORT)
  INCLUDE_DIRECTORIES(${QEXTSERIALPORT_INCLUDE_DIR})
ENDIF (WITH_INTERNAL_QEXTSERIALPORT)

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    gps/qgsqtlocationconnection.cpp
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)

ADD_FLEX_FILES(QGIS_CORE_SRCS qgsexpressionlexer.ll qgssqlstatementlexer.ll)
ADD_BISON_FILES(QGIS_CORE_SRCS qgsexpressionparser.yy qgssqlstatementparser.yy)

IF(NOT MSVC)
  SET_SOURCE_FILES_PROPERTIES(qgsexpressionparser.cpp qgssqlstatementparser.cpp PROPERTIES COMPILE_FLAGS -w)
ELSE(NOT MSVC)
  # -wd4702 unreachable code
  SET_SOURCE_FILES_PROPERTIES(
    geometry/qgsgeos.cpp
    pal/feature.cpp
    pal/pointset.cpp
    PROPERTIES COMPILE_FLAGS -wd4702)
  # -wd4091 Avoid 'typedef' ignored on left of '' when no variable is declared warning in DbgHelp.h
  SET_SOURCE_FILES_PROPERTIES(
    qgsstacktrace.cpp
    PROPERTIES COMPILE_FLAGS -wd4091)
ENDIF(NOT MSVC)

SET(QGIS_CORE_MOC_HDRS
  qgsapplication.h
  qgsactionscoperegistry.h
  qgsanimatedicon.h
  qgsbrowsermodel.h
  qgscoordinatereferencesystem.h
  qgscredentials.h
  qgsdataitem.h
  qgsdataprovider.h
  qgsdbfilterproxymodel.h
  qgsfeature.h
  qgsfeedback.h
  qgsfield.h
  qgsfieldconstraints.h
  qgsfieldformatterregistry.h
  qgsfieldmodel.h
  qgsfieldproxymodel.h
  qgsgeometryvalidator.h
  qgsgml.h
  qgsgmlschema.h
  qgsmaplayer.h
  qgsmaplayerlegend.h
  qgsmaplayermodel.h
  qgsmaplayerproxymodel.h
  qgsmaplayerstore.h
  qgsmaplayerstylemanager.h
  qgsmaprenderercache.h
  qgsmaprenderercustompainterjob.h
  qgsmaprendererjob.h
  qgsmaprendererparalleljob.h
  qgsmaprenderersequentialjob.h
  qgsmaprenderertask.h
  qgsmessagelog.h
  qgsmessageoutput.h
  qgsnetworkaccessmanager.h
  qgsnetworkdiskcache.h
  qgsnetworkcontentfetcher.h
  qgsnetworkreplyparser.h
  qgsofflineediting.h
  qgsowsconnection.h
  qgspluginlayer.h
  qgspointxy.h
  qgspointlocator.h
  qgsproject.h
  qgsrelationmanager.h
  qgsrelation.h
  qgsrunprocess.h
  qgssnappingconfig.h
  qgssnappingutils.h
  qgstaskmanager.h
  qgstracer.h
  qgstrackedvectorlayertools.h
  qgstransaction.h
  qgstransactiongroup.h
  qgsunittypes.h
  qgsvectordataprovider.h
  qgsvectorfilewritertask.h
  qgsvectorlayercache.h
  qgsvectorlayereditbuffer.h
  qgsvectorlayereditpassthrough.h
  qgsvectorlayer.h
  qgsvectorlayerexporter.h
  qgsvectorlayerfeaturecounter.h
  qgsvectorlayerjoinbuffer.h
  qgsvectorlayertools.h
  qgsmapthemecollection.h
  qgswebpage.h
  qgswebview.h
  qgssettings.h

  annotations/qgsannotation.h
  annotations/qgsannotationmanager.h
  annotations/qgshtmlannotation.h
  annotations/qgssvgannotation.h
  annotations/qgstextannotation.h

  auth/qgsauthmanager.h
  auth/qgsauthmethod.h

  composer/qgsaddremoveitemcommand.h
  composer/qgsatlascomposition.h
  composer/qgscomposerarrow.h
  composer/qgscomposerattributetablemodelv2.h
  composer/qgscomposerattributetablev2.h
  composer/qgscomposereffect.h
  composer/qgscomposerframe.h
  composer/qgscomposerhtml.h
  composer/qgscomposeritemgroup.h
  composer/qgscomposeritem.h
  composer/qgscomposerlabel.h
  composer/qgscomposerlegend.h
  composer/qgscomposermapgrid.h
  composer/qgscomposermap.h
  composer/qgscomposermapitem.h
  composer/qgscomposermapoverview.h
  composer/qgscomposermodel.h
  composer/qgscomposermousehandles.h
  composer/qgscomposermultiframe.h
  composer/qgscomposerobject.h
  composer/qgscomposerpicture.h
  composer/qgscomposerscalebar.h
  composer/qgscomposershape.h
  composer/qgscomposernodesitem.h
  composer/qgscomposerpolygon.h
  composer/qgscomposerpolyline.h
  composer/qgscomposertablecolumn.h
  composer/qgscomposertablev2.h
  composer/qgscomposertexttable.h
  composer/qgscomposition.h
  composer/qgsgroupungroupitemscommand.h
  composer/qgslayoutmanager.h
  composer/qgspaperitem.h

  processing/qgsnativealgorithms.h
  processing/qgsprocessingalgrunnertask.h
  processing/qgsprocessingfeedback.h
  processing/qgsprocessingprovider.h
  processing/qgsprocessingregistry.h

  providers/memory/qgsmemoryprovider.h

  raster/qgsrasterfilewritertask.h
  raster/qgsrasterlayer.h
  raster/qgsrasterdataprovider.h
  raster/qgsrasterinterface.h
  raster/qgsrasterlayerrenderer.h

  geometry/qgspoint.h

  geocms/geonode/qgsgeonoderequest.h

  gps/qgsgpsconnection.h
  gps/qgsgpsdetector.h
  gps/qgsnmeaconnection.h
  gps/qgsgpsdconnection.h

  layout/qgslayout.h
  layout/qgslayoutguidecollection.h
  layout/qgslayoutitem.h
  layout/qgslayoutitemmap.h
  layout/qgslayoutitempage.h
  layout/qgslayoutitemregistry.h
  layout/qgslayoutitemshape.h
  layout/qgslayoutpagecollection.h
  layout/qgslayoutobject.h

  symbology/qgscptcityarchive.h
  symbology/qgssvgcache.h
  symbology/qgsstyle.h

  layertree/qgslayertree.h
  layertree/qgslayertreegroup.h
  layertree/qgslayertreelayer.h
  layertree/qgslayertreemodel.h
  layertree/qgslayertreemodellegendnode.h
  layertree/qgslayertreenode.h
  layertree/qgslayertreeregistrybridge.h

  qgsuserprofilemanager.h
)

IF (NOT WITH_QTWEBKIT)
  SET(QGIS_CORE_MOC_HDRS ${QGIS_CORE_MOC_HDRS}
    qgswebframe.h
  )
ENDIF(NOT WITH_QTWEBKIT)

# TODO - update for Qt5
#SET(QGIS_CORE_MOC_HDRS ${QGIS_CORE_MOC_HDRS}
#    qgshttptransaction.h
#)

IF (WITH_INTERNAL_QEXTSERIALPORT)
  SET(QGIS_CORE_MOC_HDRS ${QGIS_CORE_MOC_HDRS}
    gps/qextserialport/qextserialport.h
    gps/qextserialport/qextserialenumerator.h
  )
ENDIF (WITH_INTERNAL_QEXTSERIALPORT)

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_MOC_HDRS ${QGIS_CORE_MOC_HDRS}
    gps/qgsqtlocationconnection.h
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)

QT5_WRAP_CPP(QGIS_CORE_MOC_SRCS ${QGIS_CORE_MOC_HDRS})

IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CORE_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CORE_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND (CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp PROPERTIES COMPILE_FLAGS "-O1")
  ENDIF ()
ENDIF(MSVC)

# install headers
# install qgsconfig.h and plugin.h here so they can get into
# the OS X framework target

SET(QGIS_CORE_HDRS
  ${CMAKE_BINARY_DIR}/qgsconfig.h
  ../plugins/qgisplugin.h

  expression/qgsexpression.h
  expression/qgsexpressionnode.h
  expression/qgsexpressionnodeimpl.h
  expression/qgsexpressionfunction.h

  qgis.h
  qgis_sip.h
  qgsaction.h
  qgsactionscope.h
  qgsactionmanager.h
  qgsaggregatecalculator.h
  qgsattributes.h
  qgsattributetableconfig.h
  qgsattributeeditorelement.h
  qgsbearingutils.h
  qgscachedfeatureiterator.h
  qgscacheindex.h
  qgscacheindexfeatureid.h
  qgsclipper.h
  qgscolorramp.h
  qgscolorscheme.h
  qgscolorschemeregistry.h
  qgsconnectionpool.h
  qgsconditionalstyle.h
  qgscoordinatetransform.h
  qgscoordinateutils.h
  qgscrscache.h
  qgsdartmeasurement.h
  qgsdatadefinedsizelegend.h
  qgsdataitemprovider.h
  qgsdataitemproviderregistry.h
  qgsdatasourceuri.h
  qgsdatetimestatisticalsummary.h
  qgsdatumtransformstore.h
  qgsdbfilterproxymodel.h
  qgsdiagramrenderer.h
  qgsdistancearea.h
  qgseditformconfig.h
  qgseditorwidgetsetup.h
  qgsellipsoidutils.h
  qgserror.h
  qgsexception.h
  qgsexpressioncontext.h
  qgsexpressioncontextgenerator.h
  qgsexpressionfieldbuffer.h
  qgsfeaturefilterprovider.h
  qgsfeatureiterator.h
  qgsfeaturerequest.h
  qgsfeaturesink.h
  qgsfeaturesource.h
  qgsfeaturestore.h
  qgsfieldformatter.h
  qgsfield_p.h
  qgsfields.h
  qgsfontutils.h
  qgsgeometrysimplifier.h
  qgshistogram.h
  qgsindexedfeature.h
  qgsinterval.h
  qgsjsonutils.h
  qgslayerdefinition.h
  qgslabelfeature.h
  qgslabelingengine.h
  qgslabelingenginesettings.h
  qgslabelsearchtree.h
  qgslegendrenderer.h
  qgslegendsettings.h
  qgslegendstyle.h
  qgslogger.h
  qgsmapdecoration.h
  qgsmaplayerref.h
  qgsmaphittest.h
  qgsmaplayerdependency.h
  qgsmaplayerrenderer.h
  qgsmaplayerstylemanager.h
  qgsmapsettings.h
  qgsmapsettingsutils.h
  qgsmaptopixel.h
  qgsmaptopixelgeometrysimplifier.h
  qgsmapunitscale.h
  qgsmargins.h
  qgsmimedatautils.h
  qgsmultirenderchecker.h
  qgsobjectcustomproperties.h
  qgsogcutils.h
  qgsoptional.h
  qgsoptionalexpression.h
  qgsowsconnection.h
  qgspaintenginehack.h
  qgspainting.h
  qgspallabeling.h
  qgspathresolver.h
  qgspluginlayerregistry.h
  qgspointlocator.h
  qgsprojectbadlayerhandler.h
  qgsprojectfiletransform.h
  qgsprojectproperty.h
  qgsprojectversion.h
  qgsproperty.h
  qgsproperty_p.h
  qgspropertycollection.h
  qgspropertytransformer.h
  qgsprovidermetadata.h
  qgsproviderregistry.h
  qgspythonrunner.h
  qgsrange.h
  qgsreadwritecontext.h
  qgsrenderchecker.h
  qgsrendercontext.h
  qgsrulebasedlabeling.h
  qgsruntimeprofiler.h
  qgsscalecalculator.h
  qgsscaleutils.h
  qgssimplifymethod.h
  qgssnappingutils.h
  qgsspatialindex.h
  qgssqlexpressioncompiler.h
  qgssqlstatement.h
  qgsstatisticalsummary.h
  qgsstringstatisticalsummary.h
  qgsstringutils.h
  qgstextlabelfeature.h
  qgstextrenderer.h
  qgstextrenderer_p.h
  qgstolerance.h
  qgstracer.h

  qgsvectordataprovider.h
  qgsvectorlayercache.h
  qgsvectorfilewriter.h
  qgsvectorlayerdiagramprovider.h
  qgsvectorlayereditutils.h
  qgsvectorlayerfeatureiterator.h
  qgsvectorlayerexporter.h
  qgsvectorlayerjoininfo.h
  qgsvectorlayerlabelprovider.h
  qgsvectorlayerlabeling.h
  qgsvectorlayerrenderer.h
  qgsvectorlayerundocommand.h
  qgsvectorlayerutils.h
  qgsvectorsimplifymethod.h
  qgsvirtuallayerdefinition.h
  qgsvirtuallayerdefinitionutils.h
  qgsmapthemecollection.h
  qgsxmlutils.h
  qgsarchive.h
  qgsziputils.h
  qgsvector.h
  qgslocalec.h

  annotations/qgsannotationregistry.h

  auth/qgsauthcertutils.h
  auth/qgsauthconfig.h
  auth/qgsauthcrypto.h
  auth/qgsauthmanager.h
  auth/qgsauthmethod.h
  auth/qgsauthmethodmetadata.h
  auth/qgsauthmethodregistry.h

  diagram/qgsdiagram.h
  diagram/qgspiediagram.h
  diagram/qgstextdiagram.h
  diagram/qgshistogramdiagram.h

  dxf/qgsdxfexport.h

  effects/qgsimageoperation.h
  effects/qgspainteffect.h
  effects/qgseffectstack.h
  effects/qgsblureffect.h
  effects/qgsshadoweffect.h
  effects/qgsgloweffect.h
  effects/qgspainteffectregistry.h
  effects/qgstransformeffect.h
  effects/qgscoloreffect.h

  composer/qgsaddremovemultiframecommand.h
  composer/qgscomposerarrow.h
  composer/qgscomposerframe.h
  composer/qgscomposeritemcommand.h
  composer/qgscomposermultiframecommand.h
  composer/qgscomposertexttable.h
  composer/qgspaperitem.h

  layout/qgslayoutcontext.h
  layout/qgslayoutgridsettings.h
  layout/qgslayoutitemundocommand.h
  layout/qgslayoutmeasurement.h
  layout/qgslayoutmeasurementconverter.h
  layout/qgspagesizeregistry.h
  layout/qgslayoutpoint.h
  layout/qgslayoutserializableobject.h
  layout/qgslayoutsize.h
  layout/qgslayoutsnapper.h
  layout/qgslayoutundocommand.h
  layout/qgslayoutundostack.h
  layout/qgslayoututils.h

  metadata/qgslayermetadata.h
  metadata/qgslayermetadatavalidator.h

  processing/qgsprocessing.h
  processing/qgsprocessingalgorithm.h
  processing/qgsprocessingcontext.h
  processing/qgsprocessingoutputs.h
  processing/qgsprocessingparameters.h
  processing/qgsprocessingutils.h
  processing/models/qgsprocessingmodelalgorithm.h
  processing/models/qgsprocessingmodelchildalgorithm.h
  processing/models/qgsprocessingmodelchildparametersource.h
  processing/models/qgsprocessingmodelcomponent.h
  processing/models/qgsprocessingmodeloutput.h
  processing/models/qgsprocessingmodelparameter.h

  providers/memory/qgsmemoryfeatureiterator.h
  providers/memory/qgsmemoryproviderutils.h

  raster/qgsbilinearrasterresampler.h
  raster/qgsbrightnesscontrastfilter.h
  raster/qgscliptominmaxenhancement.h
  raster/qgscolorrampshader.h
  raster/qgscontrastenhancement.h
  raster/qgscontrastenhancementfunction.h
  raster/qgscubicrasterresampler.h
  raster/qgshuesaturationfilter.h
  raster/qgslinearminmaxenhancement.h
  raster/qgslinearminmaxenhancementwithclip.h
  raster/qgsmultibandcolorrenderer.h
  raster/qgspalettedrasterrenderer.h
  raster/qgsraster.h
  raster/qgsrasterbandstats.h
  raster/qgsrasterblock.h
  raster/qgsrasterchecker.h
  raster/qgsrasterdrawer.h
  raster/qgsrasterfilewriter.h
  raster/qgsrasterhistogram.h
  raster/qgsrasteridentifyresult.h
  raster/qgsrasteriterator.h
  raster/qgsrasterminmaxorigin.h
  raster/qgsrasternuller.h
  raster/qgsrasterpipe.h
  raster/qgsrasterprojector.h
  raster/qgsrasterpyramid.h
  raster/qgsrasterrange.h
  raster/qgsrasterrenderer.h
  raster/qgsrasterresamplefilter.h
  raster/qgsrasterresampler.h
  raster/qgsrastershader.h
  raster/qgsrastershaderfunction.h
  raster/qgsrastertransparency.h
  raster/qgsrasterviewport.h
  raster/qgssinglebandcolordatarenderer.h
  raster/qgssinglebandgrayrenderer.h
  raster/qgssinglebandpseudocolorrenderer.h
  raster/qgshillshaderenderer.h

  scalebar/qgsdoubleboxscalebarrenderer.h
  scalebar/qgsnumericscalebarrenderer.h
  scalebar/qgsscalebarsettings.h
  scalebar/qgsscalebarrenderer.h
  scalebar/qgssingleboxscalebarrenderer.h
  scalebar/qgsticksscalebarrenderer.h

  symbology/qgs25drenderer.h
  symbology/qgscategorizedsymbolrenderer.h
  symbology/qgscolorbrewerpalette.h
  symbology/qgsellipsesymbollayer.h
  symbology/qgsfillsymbollayer.h
  symbology/qgsgraduatedsymbolrenderer.h
  symbology/qgslegendsymbolitem.h
  symbology/qgslinesymbollayer.h
  symbology/qgsmarkersymbollayer.h
  symbology/qgspointclusterrenderer.h
  symbology/qgspointdisplacementrenderer.h
  symbology/qgspointdistancerenderer.h
  symbology/qgsrenderer.h
  symbology/qgsrendererregistry.h
  symbology/qgsrulebasedrenderer.h
  symbology/qgssinglesymbolrenderer.h
  symbology/qgsheatmaprenderer.h
  symbology/qgsinvertedpolygonrenderer.h
  symbology/qgsnullsymbolrenderer.h
  symbology/qgssymbollayer.h
  symbology/qgssymbollayerregistry.h
  symbology/qgssymbollayerutils.h
  symbology/qgssymbol.h
  symbology/qgsvectorfieldsymbollayer.h
  symbology/qgsgeometrygeneratorsymbollayer.h

  layertree/qgslayertreeutils.h

  geometry/qgsabstractgeometry.h
  geometry/qgsbox3d.h
  geometry/qgscircularstring.h
  geometry/qgscircle.h
  geometry/qgscompoundcurve.h
  geometry/qgscurvepolygon.h
  geometry/qgscurve.h
  geometry/qgsellipse.h
  geometry/qgsgeometrycollection.h
  geometry/qgsgeometryeditutils.h
  geometry/qgsgeometryengine.h
  geometry/qgsgeometryfactory.h
  geometry/qgsgeometry.h
  geometry/qgsgeometryutils.h
  geometry/qgsgeos.h
  geometry/qgsinternalgeometryengine.h
  geometry/qgslinestring.h
  geometry/qgsmulticurve.h
  geometry/qgsmultilinestring.h
  geometry/qgsmultipoint.h
  geometry/qgsmultipolygon.h
  geometry/qgsmultisurface.h
  geometry/qgspolygon.h
  geometry/qgsrectangle.h
  geometry/qgsreferencedgeometry.h
  geometry/qgsregularpolygon.h
  geometry/qgstriangle.h
  geometry/qgssurface.h
  geometry/qgswkbptr.h
  geometry/qgswkbtypes.h

  fieldformatter/qgsdatetimefieldformatter.h
  fieldformatter/qgsfallbackfieldformatter.h
  fieldformatter/qgskeyvaluefieldformatter.h
  fieldformatter/qgslistfieldformatter.h
  fieldformatter/qgsrelationreferencefieldformatter.h
  fieldformatter/qgsvaluemapfieldformatter.h
  fieldformatter/qgsvaluerelationfieldformatter.h

  geocms/geonode/qgsgeonodeconnection.h

  metadata/qgslayermetadata.h
  metadata/qgslayermetadatavalidator.h
)

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS}
    gps/qgsqtlocationconnection.h
    gps/qgsgpsconnectionregistry.h
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${LIBZIP_INCLUDE_DIR}
  annotations
  auth
  composer
  dxf
  effects
  ${CMAKE_SOURCE_DIR}/src/core/expression
  fieldformatter
  geometry
  geocms
  geocms/geonode
  layertree
  layout
  metadata
  pal
  processing
  processing/models
  providers
  providers/memory
  raster
  renderer
  scalebar
  symbology
  gps/qextserialport
  metadata
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


#for PAL classes
IF (WIN32)
    ADD_DEFINITIONS(-D_HAVE_WINDOWS_H_)
ELSE (WIN32)
    ADD_DEFINITIONS(-D_HAVE_PTHREAD_)
ENDIF (WIN32)

# Test data dir for QgsRenderChecker
ADD_DEFINITIONS(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

#############################################################
# qgis_core library

ADD_LIBRARY(qgis_core SHARED ${QGIS_CORE_SRCS} ${QGIS_CORE_MOC_SRCS} ${QGIS_CORE_HDRS} ${QGIS_CORE_MOC_HDRS})

GENERATE_EXPORT_HEADER(
  qgis_core
  BASE_NAME CORE
  EXPORT_FILE_NAME qgis_core.h
)

SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_core.h)

IF(ENABLE_MODELTEST)
  TARGET_LINK_LIBRARIES(qgis_core ${QT_QTTEST_LIBRARY})
ENDIF(ENABLE_MODELTEST)

IF(NOT APPLE)
  INSTALL(FILES ${QGIS_CORE_HDRS} ${QGIS_CORE_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  SET_TARGET_PROPERTIES(qgis_core PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis3_core
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_CORE_HDRS};${QGIS_CORE_MOC_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
  # generated export header does not get copied with PUBLIC_HEADER files
  ADD_CUSTOM_COMMAND(TARGET qgis_core
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy qgis_core.h
            "${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIB_SUBDIR}/qgis_core.framework/Headers"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS qgis_core.h
  )
ENDIF(NOT APPLE)

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_core PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

# make sure to create qgsversion.h before compiling
ADD_DEPENDENCIES(qgis_core version)

# because of htonl
IF (WIN32)
  FIND_LIBRARY(SETUPAPI_LIBRARY NAMES setupapi PATHS $ENV{LIB})
  TARGET_LINK_LIBRARIES(qgis_core wsock32 ${SETUPAPI_LIBRARY} DbgHelp)
ENDIF (WIN32)

IF (APPLE)
  TARGET_LINK_LIBRARIES(qgis_core qgis_native)
ENDIF (APPLE)

IF (NOT WITH_INTERNAL_QEXTSERIALPORT)
  TARGET_LINK_LIBRARIES(qgis_core ${QEXTSERIALPORT_LIBRARY})
ENDIF (NOT WITH_INTERNAL_QEXTSERIALPORT)

IF (QT_MOBILITY_LOCATION_FOUND)
  TARGET_LINK_LIBRARIES(qgis_core ${QT_MOBILITY_LOCATION_LIBRARY})
ENDIF (QT_MOBILITY_LOCATION_FOUND)

TARGET_LINK_LIBRARIES(qgis_core
  ${QT_QTXML_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${OPTIONAL_QTWEBKIT}
  ${QT_QTSQL_LIBRARY}
  ${QCA_LIBRARY}
  ${QTKEYCHAIN_LIBRARY}
  ${PROJ_LIBRARY}
  ${GEOS_LIBRARY}
  ${GDAL_LIBRARY}
  ${SPATIALINDEX_LIBRARY}
  ${EXPAT_LIBRARY}
  ${SQLITE3_LIBRARY}
  ${SPATIALITE_LIBRARY}
  ${LIBZIP_LIBRARY}
)

IF (Qt5Positioning_FOUND)
  TARGET_LINK_LIBRARIES(qgis_core
    Qt5::Positioning
  )
ENDIF (Qt5Positioning_FOUND)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    qgis_core PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

# install

INSTALL(TARGETS qgis_core
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# Mac dev frameworks

IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  INSTALL(TARGETS qgis_core FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/qgis_core\")")
ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
