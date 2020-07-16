############################################################
# sources

SET(QGIS_CORE_SRCS
  ${CMAKE_SOURCE_DIR}/external/kdbush/include/kdbush.hpp
  ${CMAKE_SOURCE_DIR}/external/nmea/context.c
  ${CMAKE_SOURCE_DIR}/external/nmea/gmath.c
  ${CMAKE_SOURCE_DIR}/external/nmea/info.c
  ${CMAKE_SOURCE_DIR}/external/nmea/parse.c
  ${CMAKE_SOURCE_DIR}/external/nmea/sentence.c
  ${CMAKE_SOURCE_DIR}/external/nmea/time.c
  ${CMAKE_SOURCE_DIR}/external/nmea/tok.c

  ${CMAKE_SOURCE_DIR}/external/poly2tri/common/shapes.cc
  ${CMAKE_SOURCE_DIR}/external/poly2tri/sweep/advancing_front.cc
  ${CMAKE_SOURCE_DIR}/external/poly2tri/sweep/cdt.cc
  ${CMAKE_SOURCE_DIR}/external/poly2tri/sweep/sweep_context.cc
  ${CMAKE_SOURCE_DIR}/external/poly2tri/sweep/sweep.cc
  ${CMAKE_SOURCE_DIR}/external/meshOptimizer/simplifier.cpp

  callouts/qgscallout.cpp
  callouts/qgscalloutsregistry.cpp

  classification/qgsclassificationmethodregistry.cpp
  classification/qgsclassificationmethod.cpp
  classification/qgsclassificationcustom.cpp
  classification/qgsclassificationequalinterval.cpp
  classification/qgsclassificationprettybreaks.cpp
  classification/qgsclassificationquantile.cpp
  classification/qgsclassificationjenks.cpp
  classification/qgsclassificationstandarddeviation.cpp
  classification/qgsclassificationlogarithmic.cpp

  gps/qgsgpsconnection.cpp
  gps/qgsgpsconnectionregistry.cpp
  gps/qgsgpsdconnection.cpp
  gps/qgsgpsdetector.cpp
  gps/qgsnmeaconnection.cpp

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
  symbology/qgsinterpolatedlinerenderer.cpp
  symbology/qgsinvertedpolygonrenderer.cpp
  symbology/qgslegendsymbolitem.cpp
  symbology/qgslinesymbollayer.cpp
  symbology/qgsmarkersymbollayer.cpp
  symbology/qgsmasksymbollayer.cpp
  symbology/qgspainterswapper.cpp
  symbology/qgsnullsymbolrenderer.cpp
  symbology/qgspointclusterrenderer.cpp
  symbology/qgspointdisplacementrenderer.cpp
  symbology/qgspointdistancerenderer.cpp
  symbology/qgsrenderer.cpp
  symbology/qgsrendererrange.cpp
  symbology/qgsrendererregistry.cpp
  symbology/qgsrulebasedrenderer.cpp
  symbology/qgssinglesymbolrenderer.cpp
  symbology/qgsstyle.cpp
  symbology/qgsstylemodel.cpp
  symbology/qgssvgcache.cpp
  symbology/qgssymbollayer.cpp
  symbology/qgssymbollayerreference.cpp
  symbology/qgssymbollayerregistry.cpp
  symbology/qgssymbollayerutils.cpp
  symbology/qgssymbol.cpp
  symbology/qgsvectorfieldsymbollayer.cpp
  simplify/effectivearea.cpp

  diagram/qgsdiagram.cpp
  diagram/qgshistogramdiagram.cpp
  diagram/qgspiediagram.cpp
  diagram/qgsstackedbardiagram.cpp
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
  layertree/qgslayertreefilterproxymodel.cpp
  layertree/qgslayertreeutils.cpp
  layertree/qgslayertree.cpp
  layertree/qgslegendpatchshape.cpp

  metadata/qgsabstractmetadatabase.cpp
  metadata/qgslayermetadata.cpp
  metadata/qgslayermetadatavalidator.cpp
  metadata/qgslayermetadataformatter.cpp
  metadata/qgsprojectmetadata.cpp

  numericformats/qgsbasicnumericformat.cpp
  numericformats/qgsbearingnumericformat.cpp
  numericformats/qgscurrencynumericformat.cpp
  numericformats/qgsfallbacknumericformat.cpp
  numericformats/qgsfractionnumericformat.cpp
  numericformats/qgsnumericformat.cpp
  numericformats/qgsnumericformatregistry.cpp
  numericformats/qgspercentagenumericformat.cpp
  numericformats/qgsscientificnumericformat.cpp

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
  expression/qgsexpressioncontextutils.cpp
  expression/qgsexpressionnode.cpp
  expression/qgsexpressionnodeimpl.cpp
  expression/qgsexpressionfunction.cpp
  expression/qgsexpressionutils.cpp

  locator/qgslocator.cpp
  locator/qgslocatorfilter.cpp
  locator/qgslocatormodel.cpp
  locator/qgslocatormodelbridge.cpp

  processing/qgsprocessing.cpp
  processing/qgsprocessingalgorithm.cpp
  processing/qgsprocessingalgrunnertask.cpp
  processing/qgsprocessingcontext.cpp
  processing/qgsprocessingfeedback.cpp
  processing/qgsprocessingoutputs.cpp
  processing/qgsprocessingparameteraggregate.cpp
  processing/qgsprocessingparameterfieldmap.cpp
  processing/qgsprocessingparameters.cpp
  processing/qgsprocessingparametertype.cpp
  processing/qgsprocessingparametervectortilewriterlayers.cpp
  processing/qgsprocessingprovider.cpp
  processing/qgsprocessingregistry.cpp
  processing/qgsprocessingutils.cpp
  processing/models/qgsprocessingmodelalgorithm.cpp
  processing/models/qgsprocessingmodelchildalgorithm.cpp
  processing/models/qgsprocessingmodelchildparametersource.cpp
  processing/models/qgsprocessingmodelcomment.cpp
  processing/models/qgsprocessingmodelcomponent.cpp
  processing/models/qgsprocessingmodelgroupbox.cpp
  processing/models/qgsprocessingmodelparameter.cpp
  processing/models/qgsprocessingmodeloutput.cpp

  providers/gdal/qgsgdalproviderbase.cpp
  providers/gdal/qgsgdalprovider.cpp
  providers/gdal/qgsgdaldataitems.cpp

  providers/memory/qgsmemoryfeatureiterator.cpp
  providers/memory/qgsmemoryprovider.cpp
  providers/memory/qgsmemoryproviderutils.cpp

  providers/meshmemory/qgsmeshmemorydataprovider.cpp

  providers/ogr/qgsogrprovider.cpp
  providers/ogr/qgsogrdataitems.cpp
  providers/ogr/qgsogrfeatureiterator.cpp
  providers/ogr/qgsogrconnpool.cpp
  providers/ogr/qgsogrexpressioncompiler.cpp
  providers/ogr/qgsgeopackagedataitems.cpp
  providers/ogr/qgsgeopackageproviderconnection.cpp
  providers/ogr/qgsgeopackagerasterwriter.cpp
  providers/ogr/qgsgeopackagerasterwritertask.cpp
  providers/ogr/qgsgeopackageprojectstorage.cpp
  providers/ogr/qgsogrdbconnection.cpp
  providers/ogr/qgsogrtransaction.cpp

  scalebar/qgsdoubleboxscalebarrenderer.cpp
  scalebar/qgshollowscalebarrenderer.cpp
  scalebar/qgsnumericscalebarrenderer.cpp
  scalebar/qgsscalebarrenderer.cpp
  scalebar/qgsscalebarrendererregistry.cpp
  scalebar/qgsscalebarsettings.cpp
  scalebar/qgssingleboxscalebarrenderer.cpp
  scalebar/qgssteppedlinescalebarrenderer.cpp
  scalebar/qgsticksscalebarrenderer.cpp

  textrenderer/qgstextbackgroundsettings.cpp
  textrenderer/qgstextblock.cpp
  textrenderer/qgstextbuffersettings.cpp
  textrenderer/qgstextcharacterformat.cpp
  textrenderer/qgstextdocument.cpp
  textrenderer/qgstextformat.cpp
  textrenderer/qgstextfragment.cpp
  textrenderer/qgstextmasksettings.cpp
  textrenderer/qgstextrenderer.cpp
  textrenderer/qgstextrendererutils.cpp
  textrenderer/qgstextshadowsettings.cpp

  qgis.cpp
  qgsabstractcontentcache.cpp
  qgsabstractgeopdfexporter.cpp
  qgsabstractproviderconnection.cpp
  qgsabstractdatabaseproviderconnection.cpp
  qgsapplication.cpp
  qgsaction.cpp
  qgsactionscope.cpp
  qgsactionscoperegistry.cpp
  qgsactionmanager.cpp
  qgsaggregatecalculator.cpp
  qgsanimatedicon.cpp
  qgspostgresstringutils.cpp
  qgsattributes.cpp
  qgsattributetableconfig.cpp
  qgsattributeeditorelement.cpp
  qgsauxiliarystorage.cpp
  qgsbearingutils.cpp
  qgsblockingnetworkrequest.cpp
  qgsbookmarkmanager.cpp
  qgsbookmarkmodel.cpp
  qgsbrowsermodel.cpp
  qgsbrowserproxymodel.cpp
  qgscachedfeatureiterator.cpp
  qgscacheindex.cpp
  qgscacheindexfeatureid.cpp
  qgscadutils.cpp
  qgsclipper.cpp
  qgscolorramp.cpp
  qgscolorscheme.cpp
  qgscolorschemeregistry.cpp
  qgsconditionalstyle.cpp
  qgsconnectionregistry.cpp
  qgscoordinateformatter.cpp
  qgscoordinatereferencesystem.cpp
  qgscoordinatetransform.cpp
  qgscoordinatetransform_p.cpp
  qgscoordinatetransformcontext.cpp
  qgscoordinateutils.cpp
  qgscredentials.cpp
  qgsdartmeasurement.cpp
  qgsdatabaseschemamodel.cpp
  qgsdatabasetablemodel.cpp
  qgsdatadefinedsizelegend.cpp
  qgsdataitem.cpp
  qgsdataitemprovider.cpp
  qgsdataitemproviderregistry.cpp
  qgsdatasourceuri.cpp
  qgsdataprovider.cpp
  qgsdataprovidertemporalcapabilities.cpp
  qgsdatetimestatisticalsummary.cpp
  qgsdatumtransform.cpp
  qgsdbfilterproxymodel.cpp
  qgsdefaultvalue.cpp
  qgsdiagramrenderer.cpp
  qgsdistancearea.cpp
  qgseditformconfig.cpp
  qgsellipsoidutils.cpp
  qgserror.cpp
  qgseventtracing.cpp
  qgsexpressioncontext.cpp
  qgsexpressionfieldbuffer.cpp
  qgsfeature.cpp
  qgsfeaturepickermodel.cpp
  qgsfeaturepickermodelbase.cpp
  qgsfeatureiterator.cpp
  qgsfeaturerequest.cpp
  qgsfeaturesink.cpp
  qgsfeaturesource.cpp
  qgsfeaturestore.cpp
  qgsfeaturefiltermodel.cpp
  qgsfield.cpp
  qgsfieldconstraints.cpp
  qgsfieldformatter.cpp
  qgsfieldformatterregistry.cpp
  qgsfieldmodel.cpp
  qgsfieldproxymodel.cpp
  qgsfields.cpp
  qgsfiledownloader.cpp
  qgsfileutils.cpp
  qgsfontutils.cpp
  qgsgdalutils.cpp
  qgsgeometrysimplifier.cpp
  qgsgeometryvalidator.cpp
  qgsgeometryoptions.cpp
  qgsgml.cpp
  qgsgmlschema.cpp
  qgshistogram.cpp
  qgshstoreutils.cpp
  qgshtmlutils.cpp
  qgsimagecache.cpp
  qgsinterval.cpp
  qgsjsonutils.cpp
  qgslayerdefinition.cpp
  qgslegendrenderer.cpp
  qgslegendsettings.cpp
  qgslegendstyle.cpp
  qgslocaldefaultsettings.cpp
  qgslocalec.cpp
  qgslocalizeddatapathregistry.cpp
  qgslogger.cpp
  qgsmapclippingregion.cpp
  qgsmapclippingutils.cpp
  qgsmapdecoration.cpp
  qgsmaphittest.cpp
  qgsmaplayer.cpp
  qgsmaplayerlegend.cpp
  qgsmaplayermodel.cpp
  qgsmaplayerproxymodel.cpp
  qgsmaplayerstore.cpp
  qgsmaplayerstyle.cpp
  qgsmaplayerstylemanager.cpp
  qgsmaplayertemporalproperties.cpp
  qgsmaprenderercache.cpp
  qgsmaprenderercustompainterjob.cpp
  qgsmaprendererjob.cpp
  qgsmaprendererparalleljob.cpp
  qgsmaprenderersequentialjob.cpp
  qgsmaprendererstagedrenderjob.cpp
  qgsmaprenderertask.cpp
  qgsmapsettings.cpp
  qgsmapsettingsutils.cpp
  qgsmaptopixel.cpp
  qgsmaptopixelgeometrysimplifier.cpp
  qgsmapunitscale.cpp
  qgsmargins.cpp
  qgsmaskidprovider.cpp
  qgsmbtiles.cpp
  qgsmessagelog.cpp
  qgsmessageoutput.cpp
  qgsmimedatautils.cpp
  qgsmultirenderchecker.cpp
  qgsnetworkaccessmanager.cpp
  qgsnetworkdiskcache.cpp
  qgsnetworkcontentfetcher.cpp
  qgsnetworkcontentfetcherregistry.cpp
  qgsnetworkcontentfetchertask.cpp
  qgsnetworkreply.cpp
  qgsnetworkreplyparser.cpp
  qgsnewsfeedmodel.cpp
  qgsnewsfeedparser.cpp
  qgsobjectcustomproperties.cpp
  qgsofflineediting.cpp
  qgsogcutils.cpp
  qgsogrutils.cpp
  qgsoptionalexpression.cpp
  qgsowsconnection.cpp
  qgspaintenginehack.cpp
  qgspainting.cpp
  qgspathresolver.cpp
  qgspluginlayer.cpp
  qgspluginlayerregistry.cpp
  qgspointxy.cpp
  qgspointlocator.cpp
  qgspointlocatorinittask.cpp
  qgsproject.cpp
  qgsprojectbadlayerhandler.cpp
  qgsprojectfiletransform.cpp
  qgssnappingconfig.cpp
  qgsprojectdisplaysettings.cpp
  qgsprojectproperty.cpp
  qgsprojectservervalidator.cpp
  qgsprojectstorage.cpp
  qgsprojectstorageregistry.cpp
  qgsprojecttimesettings.cpp
  qgsprojectversion.cpp
  qgsprojectviewsettings.cpp
  qgsprojutils.cpp
  qgsproperty.cpp
  qgspropertycollection.cpp
  qgspropertytransformer.cpp
  qgsproviderconnectionmodel.cpp
  qgsprovidermetadata.cpp
  qgsproviderregistry.cpp
  qgsproxyfeaturesink.cpp
  qgsproxyprogresstask.cpp
  qgspythonrunner.cpp
  qgsreadwritecontext.cpp
  qgsreadwritelocker.cpp
  qgsrelation.cpp
  qgsrelationcontext.cpp
  qgsweakrelation.cpp
  qgsrelationmanager.cpp
  qgsremappingproxyfeaturesink.cpp
  qgsrenderchecker.cpp
  qgsrendercontext.cpp
  qgsrunprocess.cpp
  qgsruntimeprofiler.cpp
  qgsscalecalculator.cpp
  qgsscaleutils.cpp
  qgssimplifymethod.cpp
  qgssnappingutils.cpp
  qgsspatialindex.cpp
  qgsspatialindexkdbush.cpp
  qgsspatialindexutils.cpp
  qgssqlexpressioncompiler.cpp
  qgssqliteexpressioncompiler.cpp
  qgssqlstatement.cpp
  qgssqliteutils.cpp
  qgssourcecache.cpp
  qgsspatialiteutils.cpp
  qgsstatisticalsummary.cpp
  qgsstoredexpressionmanager.cpp
  qgsstringstatisticalsummary.cpp
  qgsstringutils.cpp
  qgstablecell.cpp
  qgstaskmanager.cpp
  qgstemporalcontroller.cpp
  qgstemporalnavigationobject.cpp
  qgstemporalproperty.cpp
  qgstemporalrangeobject.cpp
  qgstemporalutils.cpp
  qgstessellator.cpp
  qgstilecache.cpp
  qgstiles.cpp
  qgstolerance.cpp
  qgstracer.cpp
  qgstranslationcontext.cpp
  qgstrackedvectorlayertools.cpp
  qgstransaction.cpp
  qgstransactiongroup.cpp
  qgsunittypes.cpp
  qgsvector.cpp
  qgsvector3d.cpp
  qgsvectordataprovider.cpp
  qgsvectordataprovidertemporalcapabilities.cpp
  qgsvectorfilewriter.cpp
  qgsvectorfilewritertask.cpp
  qgsvectorlayer.cpp
  qgsvirtuallayertask.cpp
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
  qgsvectorlayerrenderer.cpp
  qgsvectorlayertemporalproperties.cpp
  qgsvectorlayertools.cpp
  qgsvectorlayerundocommand.cpp
  qgsvectorlayerundopassthroughcommand.cpp
  qgsvectorlayerutils.cpp
  qgsvectorsimplifymethod.cpp
  qgsvectorlayerserverproperties.cpp
  qgsvirtuallayerdefinition.cpp
  qgsvirtuallayerdefinitionutils.cpp
  qgsmapthemecollection.cpp
  qgsxmlutils.cpp
  qgssettings.cpp
  qgsarchive.cpp
  qgstestutils.cpp
  qgsziputils.cpp

  dxf/qgsdxfexport.cpp
  dxf/qgsdxfpaintdevice.cpp
  dxf/qgsdxfpaintengine.cpp

  layout/qgsabstractreportsection.cpp
  layout/qgslayout.cpp
  layout/qgslayoutaligner.cpp
  layout/qgslayoutatlas.cpp
  layout/qgslayouteffect.cpp
  layout/qgslayoutexporter.cpp
  layout/qgslayoutgridsettings.cpp
  layout/qgslayoutguidecollection.cpp
  layout/qgslayoutframe.cpp
  layout/qgslayoutgeopdfexporter.cpp
  layout/qgslayoutitem.cpp
  layout/qgslayoutitemattributetable.cpp
  layout/qgslayoutitemgroup.cpp
  layout/qgslayoutitemgroupundocommand.cpp
  layout/qgslayoutitemhtml.cpp
  layout/qgslayoutitemlabel.cpp
  layout/qgslayoutitemlegend.cpp
  layout/qgslayoutitemmanualtable.cpp
  layout/qgslayoutitemmap.cpp
  layout/qgslayoutitemmapgrid.cpp
  layout/qgslayoutitemmapitem.cpp
  layout/qgslayoutitemmapoverview.cpp
  layout/qgslayoutitemmarker.cpp
  layout/qgslayoutitemnodeitem.cpp
  layout/qgslayoutitempage.cpp
  layout/qgslayoutitempicture.cpp
  layout/qgslayoutitempolygon.cpp
  layout/qgslayoutitempolyline.cpp
  layout/qgslayoutitemregistry.cpp
  layout/qgslayoutitemscalebar.cpp
  layout/qgslayoutitemshape.cpp
  layout/qgslayoutitemtexttable.cpp
  layout/qgslayoutitemundocommand.cpp
  layout/qgslayoutmanager.cpp
  layout/qgslayoutmeasurement.cpp
  layout/qgslayoutmeasurementconverter.cpp
  layout/qgslayoutmodel.cpp
  layout/qgslayoutmultiframe.cpp
  layout/qgslayoutmultiframeundocommand.cpp
  layout/qgslayoutnortharrowhandler.cpp
  layout/qgslayoutobject.cpp
  layout/qgslayoutpagecollection.cpp
  layout/qgslayoutrendercontext.cpp
  layout/qgslayoutreportcontext.cpp
  layout/qgslayoutserializableobject.cpp
  layout/qgslayoutsnapper.cpp
  layout/qgslayouttable.cpp
  layout/qgslayouttablecolumn.cpp
  layout/qgslayoutundocommand.cpp
  layout/qgslayoutundostack.cpp
  layout/qgslayoututils.cpp
  layout/qgspagesizeregistry.cpp
  layout/qgslayoutpoint.cpp
  layout/qgslayoutserializableobject.cpp
  layout/qgslayoutsize.cpp
  layout/qgsprintlayout.cpp
  layout/qgsreport.cpp
  layout/qgsreportsectionfieldgroup.cpp
  layout/qgsreportsectionlayout.cpp
  layout/qgscompositionconverter.cpp

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
  raster/qgsrastercontourrenderer.cpp
  raster/qgsrasterdataprovider.cpp
  raster/qgsrasterdataprovidertemporalcapabilities.cpp
  raster/qgsrasterfilewritertask.cpp
  raster/qgsrasteridentifyresult.cpp
  raster/qgsrasterinterface.cpp
  raster/qgsrasteriterator.cpp
  raster/qgsrasterlayer.cpp
  raster/qgsrasterlayerrenderer.cpp
  raster/qgsrasterlayertemporalproperties.cpp
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

  mesh/qgsmesh3daveraging.cpp
  mesh/qgsmeshdataprovider.cpp
  mesh/qgsmeshdataprovidertemporalcapabilities.cpp
  mesh/qgsmeshdataset.cpp
  mesh/qgsmeshdatasetgroupstore.cpp
  mesh/qgsmeshlayer.cpp
  mesh/qgsmeshlayerinterpolator.cpp
  mesh/qgsmeshlayerrenderer.cpp
  mesh/qgsmeshlayertemporalproperties.cpp
  mesh/qgsmeshlayerutils.cpp
  mesh/qgsmeshrenderersettings.cpp
  mesh/qgsmeshsimplificationsettings.cpp
  mesh/qgsmeshspatialindex.cpp
  mesh/qgsmeshtimesettings.cpp
  mesh/qgsmeshtracerenderer.cpp
  mesh/qgsmeshvectorrenderer.cpp
  mesh/qgstriangularmesh.cpp

  labeling/qgslabelfeature.cpp
  labeling/qgslabelingengine.cpp
  labeling/qgslabelingenginesettings.cpp
  labeling/qgslabelobstaclesettings.cpp
  labeling/qgslabelsearchtree.cpp
  labeling/qgslabelsink.cpp
  labeling/qgslabelthinningsettings.cpp
  labeling/qgspallabeling.cpp
  labeling/qgsrulebasedlabeling.cpp
  labeling/qgstextlabelfeature.cpp
  labeling/qgsvectorlayerlabeling.cpp
  labeling/qgsvectorlayerlabelprovider.cpp

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
  geometry/qgslinesegment.cpp
  geometry/qgslinestring.cpp
  geometry/qgsmulticurve.cpp
  geometry/qgsmultilinestring.cpp
  geometry/qgsmultipoint.cpp
  geometry/qgsmultipolygon.cpp
  geometry/qgsmultisurface.cpp
  geometry/qgspoint.cpp
  geometry/qgspolygon.cpp
  geometry/qgsquadrilateral.cpp
  geometry/qgsrectangle.cpp
  geometry/qgsreferencedgeometry.cpp
  geometry/qgsregularpolygon.cpp
  geometry/qgssurface.cpp
  geometry/qgstriangle.cpp
  geometry/qgswkbptr.cpp
  geometry/qgswkbtypes.cpp

  3d/qgs3drendererregistry.cpp
  3d/qgsabstract3drenderer.cpp

  fieldformatter/qgscheckboxfieldformatter.cpp
  fieldformatter/qgsrangefieldformatter.cpp
  fieldformatter/qgsdatetimefieldformatter.cpp
  fieldformatter/qgsfallbackfieldformatter.cpp
  fieldformatter/qgskeyvaluefieldformatter.cpp
  fieldformatter/qgslistfieldformatter.cpp
  fieldformatter/qgsrelationreferencefieldformatter.cpp
  fieldformatter/qgsvaluemapfieldformatter.cpp
  fieldformatter/qgsvaluerelationfieldformatter.cpp

  geocms/geonode/qgsgeonodeconnection.cpp
  geocms/geonode/qgsgeonoderequest.cpp

  validity/qgsabstractvaliditycheck.cpp
  validity/qgsvaliditycheckcontext.cpp
  validity/qgsvaliditycheckregistry.cpp

  vectortile/qgsvectortilebasiclabeling.cpp
  vectortile/qgsvectortilebasicrenderer.cpp
  vectortile/qgsvectortileconnection.cpp
  vectortile/qgsvectortiledataitems.cpp
  vectortile/qgsvectortilelabeling.cpp
  vectortile/qgsvectortilelayer.cpp
  vectortile/qgsvectortilelayerrenderer.cpp
  vectortile/qgsvectortileloader.cpp
  vectortile/qgsvectortilemvtdecoder.cpp
  vectortile/qgsvectortilemvtencoder.cpp
  vectortile/qgsvectortilemvtutils.cpp
  vectortile/qgsvectortileprovidermetadata.cpp
  vectortile/qgsvectortileutils.cpp
  vectortile/qgsvectortilewriter.cpp

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
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    ${MODELTEST_SRCS}
  )
ENDIF(ENABLE_MODELTEST)

IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # spatialindex headers produce warnings
  SET_SOURCE_FILES_PROPERTIES(qgsspatialindex.cpp PROPERTIES COMPILE_FLAGS -Wno-overloaded-virtual)
ENDIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    gps/qgsqtlocationconnection.cpp
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)

ADD_FLEX_FILES_PREFIX(QGIS_CORE_SRCS exp_ qgsexpressionlexer.ll)
ADD_FLEX_FILES_PREFIX(QGIS_CORE_SRCS sqlstatement_ qgssqlstatementlexer.ll)
ADD_BISON_FILES_PREFIX(QGIS_CORE_SRCS exp_ qgsexpressionparser.yy)
ADD_BISON_FILES_PREFIX(QGIS_CORE_SRCS sqlstatement_ qgssqlstatementparser.yy)

IF(NOT MSVC)
  SET_SOURCE_FILES_PROPERTIES(qgsexpressionparser.cpp qgssqlstatementparser.cpp PROPERTIES COMPILE_FLAGS -w)
ELSE(NOT MSVC)
  # -wd4702 unreachable code
  SET_SOURCE_FILES_PROPERTIES(
    geometry/qgsgeos.cpp
    pal/feature.cpp
    pal/pointset.cpp
    PROPERTIES COMPILE_FLAGS -wd4702)
ENDIF(NOT MSVC)

SET(QGIS_CORE_HDRS
  ${CMAKE_SOURCE_DIR}/external/nlohmann/json_fwd.hpp
  ${CMAKE_BINARY_DIR}/qgsconfig.h
  ../plugins/qgisplugin.h

  qgis.h
  qgis_sip.h
  qgsabstractcontentcache.h
  qgsabstractdatabaseproviderconnection.h
  qgsabstractgeopdfexporter.h
  qgsabstractproviderconnection.h
  qgsaction.h
  qgsactionmanager.h
  qgsactionscope.h
  qgsactionscoperegistry.h
  qgsaggregatecalculator.h
  qgsanimatedicon.h
  qgsapplication.h
  qgsarchive.h
  qgsattributeeditorelement.h
  qgsattributes.h
  qgsattributetableconfig.h
  qgsauxiliarystorage.h
  qgsbearingutils.h
  qgsblockingnetworkrequest.h
  qgsbookmarkmanager.h
  qgsbookmarkmodel.h
  qgsbrowsermodel.h
  qgsbrowserproxymodel.h
  qgscachedfeatureiterator.h
  qgscacheindex.h
  qgscacheindexfeatureid.h
  qgscadutils.h
  qgsclipper.h
  qgscolorramp.h
  qgscolorscheme.h
  qgscolorschemeregistry.h
  qgsconditionalstyle.h
  qgsconnectionpool.h
  qgsconnectionregistry.h
  qgscoordinateformatter.h
  qgscoordinatereferencesystem.h
  qgscoordinatetransform.h
  qgscoordinatetransformcontext.h
  qgscoordinateutils.h
  qgscredentials.h
  qgsdartmeasurement.h
  qgsdatabaseschemamodel.h
  qgsdatabasetablemodel.h
  qgsdatadefinedsizelegend.h
  qgsdataitem.h
  qgsdataitemprovider.h
  qgsdataitemproviderregistry.h
  qgsdataprovider.h
  qgsdataprovidertemporalcapabilities.h
  qgsdatasourceuri.h
  qgsdatetimestatisticalsummary.h
  qgsdatumtransform.h
  qgsdbfilterproxymodel.h
  qgsdefaultvalue.h
  qgsdiagramrenderer.h
  qgsdistancearea.h
  qgseditformconfig.h
  qgseditorwidgetsetup.h
  qgsellipsoidutils.h
  qgserror.h
  qgseventtracing.h
  qgsexception.h
  qgsexpressioncontext.h
  qgsexpressioncontextgenerator.h
  qgsexpressioncontextscopegenerator.h
  qgsexpressionfieldbuffer.h
  qgsfeature.h
  qgsfeaturepickermodel.h
  qgsfeaturepickermodelbase.h
  qgsfeatureexpressionvaluesgatherer.h
  qgsfeaturefiltermodel.h
  qgsfeaturefilterprovider.h
  qgsfeatureid.h
  qgsfeatureiterator.h
  qgsfeaturerequest.h
  qgsfeaturesink.h
  qgsfeaturesource.h
  qgsfeaturestore.h
  qgsfeedback.h
  qgsfield.h
  qgsfieldconstraints.h
  qgsfieldformatter.h
  qgsfieldformatterregistry.h
  qgsfieldmodel.h
  qgsfieldproxymodel.h
  qgsfields.h
  qgsfiledownloader.h
  qgsfilefiltergenerator.h
  qgsfileutils.h
  qgsfontutils.h
  qgsgdalutils.h
  qgsgenericspatialindex.h
  qgsgeometryoptions.h
  qgsgeometrysimplifier.h
  qgsgeometryvalidator.h
  qgsgml.h
  qgsgmlschema.h
  qgshistogram.h
  qgshstoreutils.h
  qgshtmlutils.h
  qgsimagecache.h
  qgsindexedfeature.h
  qgsinterval.h
  qgsjsonutils.h
  qgslayerdefinition.h
  qgslegendrenderer.h
  qgslegendsettings.h
  qgslegendstyle.h
  qgslocaldefaultsettings.h
  qgslocalec.h
  qgslocalizeddatapathregistry.h
  qgslogger.h
  qgsmapclippingregion.h
  qgsmapclippingutils.h
  qgsmapdecoration.h
  qgsmaphittest.h
  qgsmaplayer.h
  qgsmaplayerdependency.h
  qgsmaplayerlegend.h
  qgsmaplayermodel.h
  qgsmaplayerproxymodel.h
  qgsmaplayerref.h
  qgsmaplayerrenderer.h
  qgsmaplayerstore.h
  qgsmaplayerstyle.h
  qgsmaplayerstylemanager.h
  qgsmaplayertemporalproperties.h
  qgsmaprenderercache.h
  qgsmaprenderercustompainterjob.h
  qgsmaprendererjob.h
  qgsmaprendererparalleljob.h
  qgsmaprenderersequentialjob.h
  qgsmaprendererstagedrenderjob.h
  qgsmaprenderertask.h
  qgsmapsettings.h
  qgsmapsettingsutils.h
  qgsmapthemecollection.h
  qgsmaptopixel.h
  qgsmaptopixelgeometrysimplifier.h
  qgsmapunitscale.h
  qgsmargins.h
  qgsmaskidprovider.h
  qgsmbtiles.h
  qgsmessagelog.h
  qgsmessageoutput.h
  qgsmimedatautils.h
  qgsmultirenderchecker.h
  qgsnetworkaccessmanager.h
  qgsnetworkcontentfetcher.h
  qgsnetworkcontentfetcherregistry.h
  qgsnetworkcontentfetchertask.h
  qgsnetworkdiskcache.h
  qgsnetworkreply.h
  qgsnetworkreplyparser.h
  qgsnewsfeedmodel.h
  qgsnewsfeedparser.h
  qgsobjectcustomproperties.h
  qgsofflineediting.h
  qgsogcutils.h
  qgsogrutils.h
  qgsoptional.h
  qgsoptionalexpression.h
  qgsowsconnection.h
  qgspaintenginehack.h
  qgspainting.h
  qgspathresolver.h
  qgspluginlayer.h
  qgspluginlayerregistry.h
  qgspointlocator.h
  qgspointlocatorinittask.h
  qgspointxy.h
  qgspostgresstringutils.h
  qgsproject.h
  qgsprojectbadlayerhandler.h
  qgsprojectdisplaysettings.h
  qgsprojectfiletransform.h
  qgsprojectproperty.h
  qgsprojectservervalidator.h
  qgsprojectstorage.h
  qgsprojectstorageregistry.h
  qgsprojecttimesettings.h
  qgsprojecttranslator.h
  qgsprojectversion.h
  qgsprojectviewsettings.h
  qgsprojutils.h
  qgsproperty.h
  qgspropertycollection.h
  qgspropertytransformer.h
  qgsproviderconnectionmodel.h
  qgsprovidermetadata.h
  qgsproviderregistry.h
  qgsproxyfeaturesink.h
  qgsproxyprogresstask.h
  qgspythonrunner.h
  qgsrange.h
  qgsreadwritecontext.h
  qgsreadwritelocker.h
  qgsrelation.h
  qgsrelationcontext.h
  qgsremappingproxyfeaturesink.h
  qgsweakrelation.h
  qgsrelationmanager.h
  qgsrenderchecker.h
  qgsrendercontext.h
  qgsrenderedfeaturehandlerinterface.h
  qgsrunprocess.h
  qgsruntimeprofiler.h
  qgsscalecalculator.h
  qgsscaleutils.h
  qgssettings.h
  qgssimplifymethod.h
  qgssnappingconfig.h
  qgssnappingutils.h
  qgsspatialindex.h
  qgsspatialindexkdbush.h
  qgsspatialindexkdbushdata.h
  qgsspatialindexutils.h
  qgssourcecache.h
  qgsspatialiteutils.h
  qgssqlexpressioncompiler.h
  qgssqliteutils.h
  qgssqlstatement.h
  qgsstatisticalsummary.h
  qgsstoredexpressionmanager.h
  qgsstringstatisticalsummary.h
  qgsstringutils.h
  qgstablecell.h
  qgstaskmanager.h
  qgstemporalcontroller.h
  qgstemporalnavigationobject.h
  qgstemporalproperty.h
  qgstemporalrangeobject.h
  qgstemporalutils.h
  qgstessellator.h
  qgstestutils.h
  qgsthreadingutils.h
  qgstilecache.h
  qgstiles.h
  qgstolerance.h
  qgstracer.h
  qgstrackedvectorlayertools.h
  qgstransaction.h
  qgstransactiongroup.h
  qgstranslationcontext.h
  qgsunittypes.h
  qgsuserprofilemanager.h
  qgsvector.h
  qgsvector3d.h
  qgsvectordataprovider.h
  qgsvectordataprovidertemporalcapabilities.h
  qgsvectorfilewriter.h
  qgsvectorfilewritertask.h
  qgsvectorlayer.h
  qgsvectorlayerref.h
  qgsvectorlayercache.h
  qgsvectorlayerdiagramprovider.h
  qgsvectorlayereditbuffer.h
  qgsvectorlayereditpassthrough.h
  qgsvectorlayereditutils.h
  qgsvectorlayerexporter.h
  qgsvectorlayerfeaturecounter.h
  qgsvectorlayerfeatureiterator.h
  qgsvectorlayerjoinbuffer.h
  qgsvectorlayerjoininfo.h
  qgsvectorlayerrenderer.h
  qgsvectorlayerserverproperties.h
  qgsvectorlayertemporalproperties.h
  qgsvectorlayertools.h
  qgsvectorlayerundocommand.h
  qgsvectorlayerundopassthroughcommand.h
  qgsvectorlayerutils.h
  qgsvectorsimplifymethod.h
  qgsvirtuallayerdefinition.h
  qgsvirtuallayerdefinitionutils.h
  qgsvirtuallayertask.h
  qgswebpage.h
  qgswebview.h
  qgsxmlutils.h
  qgsziputils.h
  qobjectuniqueptr.h

  3d/qgs3drendererregistry.h
  3d/qgsabstract3drenderer.h

  annotations/qgsannotation.h
  annotations/qgsannotationmanager.h
  annotations/qgsannotationregistry.h
  annotations/qgshtmlannotation.h
  annotations/qgssvgannotation.h
  annotations/qgstextannotation.h

  auth/qgsauthcertutils.h
  auth/qgsauthconfig.h
  auth/qgsauthcrypto.h
  auth/qgsauthmanager.h
  auth/qgsauthmethod.h
  auth/qgsauthmethodmetadata.h
  auth/qgsauthmethodregistry.h

  callouts/qgscallout.h
  callouts/qgscalloutsregistry.h

  classification/qgsclassificationcustom.h
  classification/qgsclassificationequalinterval.h
  classification/qgsclassificationjenks.h
  classification/qgsclassificationlogarithmic.h
  classification/qgsclassificationmethod.h
  classification/qgsclassificationmethodregistry.h
  classification/qgsclassificationprettybreaks.h
  classification/qgsclassificationquantile.h
  classification/qgsclassificationstandarddeviation.h

  diagram/qgsdiagram.h
  diagram/qgshistogramdiagram.h
  diagram/qgspiediagram.h
  diagram/qgsstackedbardiagram.h
  diagram/qgstextdiagram.h

  dxf/qgsdxfexport.h

  effects/qgsblureffect.h
  effects/qgscoloreffect.h
  effects/qgseffectstack.h
  effects/qgsgloweffect.h
  effects/qgsimageoperation.h
  effects/qgspainteffect.h
  effects/qgspainteffectregistry.h
  effects/qgsshadoweffect.h
  effects/qgstransformeffect.h

  expression/qgsexpression.h
  expression/qgsexpressioncontextutils.h
  expression/qgsexpressionfunction.h
  expression/qgsexpressionnode.h
  expression/qgsexpressionnodeimpl.h

  fieldformatter/qgscheckboxfieldformatter.h
  fieldformatter/qgsdatetimefieldformatter.h
  fieldformatter/qgsfallbackfieldformatter.h
  fieldformatter/qgskeyvaluefieldformatter.h
  fieldformatter/qgslistfieldformatter.h
  fieldformatter/qgsrangefieldformatter.h
  fieldformatter/qgsrelationreferencefieldformatter.h
  fieldformatter/qgsvaluemapfieldformatter.h
  fieldformatter/qgsvaluerelationfieldformatter.h

  geocms/geonode/qgsgeonodeconnection.h
  geocms/geonode/qgsgeonoderequest.h

  geometry/qgsabstractgeometry.h
  geometry/qgsbox3d.h
  geometry/qgscircle.h
  geometry/qgscircularstring.h
  geometry/qgscompoundcurve.h
  geometry/qgscurve.h
  geometry/qgscurvepolygon.h
  geometry/qgsellipse.h
  geometry/qgsgeometry.h
  geometry/qgsgeometrycollection.h
  geometry/qgsgeometryeditutils.h
  geometry/qgsgeometryengine.h
  geometry/qgsgeometryfactory.h
  geometry/qgsgeometryutils.h
  geometry/qgsgeos.h
  geometry/qgsinternalgeometryengine.h
  geometry/qgslinesegment.h
  geometry/qgslinestring.h
  geometry/qgsmulticurve.h
  geometry/qgsmultilinestring.h
  geometry/qgsmultipoint.h
  geometry/qgsmultipolygon.h
  geometry/qgsmultisurface.h
  geometry/qgspoint.h
  geometry/qgspolygon.h
  geometry/qgsquadrilateral.h
  geometry/qgsrectangle.h
  geometry/qgsreferencedgeometry.h
  geometry/qgsregularpolygon.h
  geometry/qgssurface.h
  geometry/qgstriangle.h
  geometry/qgswkbptr.h
  geometry/qgswkbtypes.h

  gps/qgsgpsconnection.h
  gps/qgsgpsdconnection.h
  gps/qgsgpsdetector.h
  gps/qgsnmeaconnection.h

  labeling/qgslabelfeature.h
  labeling/qgslabeling.h
  labeling/qgslabelingengine.h
  labeling/qgslabelingenginesettings.h
  labeling/qgslabelobstaclesettings.h
  labeling/qgslabelsearchtree.h
  labeling/qgslabelthinningsettings.h
  labeling/qgspallabeling.h
  labeling/qgsrulebasedlabeling.h
  labeling/qgstextlabelfeature.h
  labeling/qgsvectorlayerlabeling.h
  labeling/qgsvectorlayerlabelprovider.h

  layertree/qgslayertree.h
  layertree/qgslayertreegroup.h
  layertree/qgslayertreelayer.h
  layertree/qgslayertreemodel.h
  layertree/qgslayertreemodellegendnode.h
  layertree/qgslayertreenode.h
  layertree/qgslayertreeregistrybridge.h
  layertree/qgslayertreefilterproxymodel.h
  layertree/qgslayertreeutils.h
  layertree/qgslegendpatchshape.h

  layout/qgsabstractlayoutiterator.h
  layout/qgsabstractreportsection.h
  layout/qgscompositionconverter.h
  layout/qgslayout.h
  layout/qgslayoutaligner.h
  layout/qgslayoutatlas.h
  layout/qgslayouteffect.h
  layout/qgslayoutexporter.h
  layout/qgslayoutframe.h
  layout/qgslayoutgeopdfexporter.h
  layout/qgslayoutgridsettings.h
  layout/qgslayoutguidecollection.h
  layout/qgslayoutitem.h
  layout/qgslayoutitemattributetable.h
  layout/qgslayoutitemgroup.h
  layout/qgslayoutitemgroupundocommand.h
  layout/qgslayoutitemhtml.h
  layout/qgslayoutitemlabel.h
  layout/qgslayoutitemlegend.h
  layout/qgslayoutitemmanualtable.h
  layout/qgslayoutitemmap.h
  layout/qgslayoutitemmapgrid.h
  layout/qgslayoutitemmapitem.h
  layout/qgslayoutitemmapoverview.h
  layout/qgslayoutitemmarker.h
  layout/qgslayoutitemnodeitem.h
  layout/qgslayoutitempage.h
  layout/qgslayoutitempicture.h
  layout/qgslayoutitempolygon.h
  layout/qgslayoutitempolyline.h
  layout/qgslayoutitemregistry.h
  layout/qgslayoutitemscalebar.h
  layout/qgslayoutitemshape.h
  layout/qgslayoutitemtexttable.h
  layout/qgslayoutitemundocommand.h
  layout/qgslayoutmanager.h
  layout/qgslayoutmeasurement.h
  layout/qgslayoutmeasurementconverter.h
  layout/qgslayoutmodel.h
  layout/qgslayoutmultiframe.h
  layout/qgslayoutmultiframeundocommand.h
  layout/qgslayoutnortharrowhandler.h
  layout/qgslayoutobject.h
  layout/qgslayoutpagecollection.h
  layout/qgslayoutpoint.h
  layout/qgslayoutrendercontext.h
  layout/qgslayoutreportcontext.h
  layout/qgslayoutserializableobject.h
  layout/qgslayoutsize.h
  layout/qgslayoutsnapper.h
  layout/qgslayouttable.h
  layout/qgslayouttablecolumn.h
  layout/qgslayoutundocommand.h
  layout/qgslayoutundostack.h
  layout/qgslayoututils.h
  layout/qgsmasterlayoutinterface.h
  layout/qgspagesizeregistry.h
  layout/qgsprintlayout.h
  layout/qgsreport.h
  layout/qgsreportsectionfieldgroup.h
  layout/qgsreportsectionlayout.h

  locator/qgslocator.h
  locator/qgslocatorcontext.h
  locator/qgslocatorfilter.h
  locator/qgslocatormodel.h
  locator/qgslocatormodelbridge.h

  mesh/qgsmesh3daveraging.h
  mesh/qgsmeshdataprovider.h
  mesh/qgsmeshdataprovidertemporalcapabilities.h
  mesh/qgsmeshdataset.h
  mesh/qgsmeshdatasetgroupstore.h
  mesh/qgsmeshlayer.h
  mesh/qgsmeshlayerinterpolator.h
  mesh/qgsmeshlayerrenderer.h
  mesh/qgsmeshlayertemporalproperties.h
  mesh/qgsmeshlayerutils.h
  mesh/qgsmeshrenderersettings.h
  mesh/qgsmeshsimplificationsettings.h
  mesh/qgsmeshspatialindex.h
  mesh/qgsmeshtimesettings.h
  mesh/qgsmeshtracerenderer.h
  mesh/qgsmeshvectorrenderer.h
  mesh/qgstriangularmesh.h

  metadata/qgsabstractmetadatabase.h
  metadata/qgslayermetadata.h
  metadata/qgslayermetadataformatter.h
  metadata/qgslayermetadatavalidator.h
  metadata/qgsprojectmetadata.h

  numericformats/qgsbasicnumericformat.h
  numericformats/qgsbearingnumericformat.h
  numericformats/qgscurrencynumericformat.h
  numericformats/qgsfallbacknumericformat.h
  numericformats/qgsfractionnumericformat.h
  numericformats/qgsnumericformat.h
  numericformats/qgsnumericformatregistry.h
  numericformats/qgspercentagenumericformat.h
  numericformats/qgsscientificnumericformat.h

  processing/models/qgsprocessingmodelalgorithm.h
  processing/models/qgsprocessingmodelchildalgorithm.h
  processing/models/qgsprocessingmodelchilddependency.h
  processing/models/qgsprocessingmodelchildparametersource.h
  processing/models/qgsprocessingmodelcomment.h
  processing/models/qgsprocessingmodelcomponent.h
  processing/models/qgsprocessingmodelgroupbox.h
  processing/models/qgsprocessingmodeloutput.h
  processing/models/qgsprocessingmodelparameter.h
  processing/qgsprocessing.h
  processing/qgsprocessingalgorithm.h
  processing/qgsprocessingalgrunnertask.h
  processing/qgsprocessingcontext.h
  processing/qgsprocessingfeedback.h
  processing/qgsprocessingoutputs.h
  processing/qgsprocessingparameteraggregate.h
  processing/qgsprocessingparameterfieldmap.h
  processing/qgsprocessingparameters.h
  processing/qgsprocessingparametertype.h
  processing/qgsprocessingparametertypeimpl.h
  processing/qgsprocessingparametervectortilewriterlayers.h
  processing/qgsprocessingprovider.h
  processing/qgsprocessingregistry.h
  processing/qgsprocessingutils.h

  providers/gdal/qgsgdaldataitems.h
  providers/gdal/qgsgdalprovider.h
  providers/memory/qgsmemoryfeatureiterator.h
  providers/memory/qgsmemoryprovider.h
  providers/memory/qgsmemoryproviderutils.h
  providers/meshmemory/qgsmeshmemorydataprovider.h
  providers/ogr/qgsgeopackagedataitems.h
  providers/ogr/qgsgeopackageprojectstorage.h
  providers/ogr/qgsgeopackageproviderconnection.h
  providers/ogr/qgsgeopackagerasterwritertask.h
  providers/ogr/qgsogrconnpool.h
  providers/ogr/qgsogrdataitems.h
  providers/ogr/qgsogrdbconnection.h
  providers/ogr/qgsogrprovider.h
  providers/ogr/qgsogrtransaction.h

  raster/qgsbilinearrasterresampler.h
  raster/qgsbrightnesscontrastfilter.h
  raster/qgscliptominmaxenhancement.h
  raster/qgscolorrampshader.h
  raster/qgscontrastenhancement.h
  raster/qgscontrastenhancementfunction.h
  raster/qgscubicrasterresampler.h
  raster/qgshillshaderenderer.h
  raster/qgshuesaturationfilter.h
  raster/qgslinearminmaxenhancement.h
  raster/qgslinearminmaxenhancementwithclip.h
  raster/qgsmultibandcolorrenderer.h
  raster/qgspalettedrasterrenderer.h
  raster/qgsraster.h
  raster/qgsrasterbandstats.h
  raster/qgsrasterblock.h
  raster/qgsrasterchecker.h
  raster/qgsrastercontourrenderer.h
  raster/qgsrasterdataprovider.h
  raster/qgsrasterdataprovidertemporalcapabilities.h
  raster/qgsrasterdrawer.h
  raster/qgsrasterfilewriter.h
  raster/qgsrasterfilewritertask.h
  raster/qgsrasterhistogram.h
  raster/qgsrasteridentifyresult.h
  raster/qgsrasterinterface.h
  raster/qgsrasteriterator.h
  raster/qgsrasterlayer.h
  raster/qgsrasterlayerrenderer.h
  raster/qgsrasterlayertemporalproperties.h
  raster/qgsrasterminmaxorigin.h
  raster/qgsrasternuller.h
  raster/qgsrasterpipe.h
  raster/qgsrasterprojector.h
  raster/qgsrasterpyramid.h
  raster/qgsrasterrange.h
  raster/qgsrasterrenderer.h
  raster/qgsrasterrendererregistry.h
  raster/qgsrasterresamplefilter.h
  raster/qgsrasterresampler.h
  raster/qgsrastershader.h
  raster/qgsrastershaderfunction.h
  raster/qgsrastertransparency.h
  raster/qgsrasterviewport.h
  raster/qgssinglebandcolordatarenderer.h
  raster/qgssinglebandgrayrenderer.h
  raster/qgssinglebandpseudocolorrenderer.h

  scalebar/qgsdoubleboxscalebarrenderer.h
  scalebar/qgshollowscalebarrenderer.h
  scalebar/qgsnumericscalebarrenderer.h
  scalebar/qgsscalebarrenderer.h
  scalebar/qgsscalebarrendererregistry.h
  scalebar/qgsscalebarsettings.h
  scalebar/qgssingleboxscalebarrenderer.h
  scalebar/qgssteppedlinescalebarrenderer.h
  scalebar/qgsticksscalebarrenderer.h

  symbology/qgs25drenderer.h
  symbology/qgsarrowsymbollayer.h
  symbology/qgscategorizedsymbolrenderer.h
  symbology/qgscolorbrewerpalette.h
  symbology/qgscptcityarchive.h
  symbology/qgsellipsesymbollayer.h
  symbology/qgsfillsymbollayer.h
  symbology/qgsgeometrygeneratorsymbollayer.h
  symbology/qgsgraduatedsymbolrenderer.h
  symbology/qgsheatmaprenderer.h
  symbology/qgsinterpolatedlinerenderer.h
  symbology/qgsinvertedpolygonrenderer.h
  symbology/qgslegendsymbolitem.h
  symbology/qgslinesymbollayer.h
  symbology/qgsmarkersymbollayer.h
  symbology/qgsnullsymbolrenderer.h
  symbology/qgspointclusterrenderer.h
  symbology/qgspointdisplacementrenderer.h
  symbology/qgspointdistancerenderer.h
  symbology/qgsrenderer.h
  symbology/qgsrendererrange.h
  symbology/qgsrendererregistry.h
  symbology/qgsrulebasedrenderer.h
  symbology/qgssinglesymbolrenderer.h
  symbology/qgsstyle.h
  symbology/qgsstyleentityvisitor.h
  symbology/qgsstylemodel.h
  symbology/qgssvgcache.h
  symbology/qgssymbol.h
  symbology/qgssymbollayer.h
  symbology/qgssymbollayerreference.h
  symbology/qgssymbollayerregistry.h
  symbology/qgssymbollayerutils.h
  symbology/qgsvectorfieldsymbollayer.h

  textrenderer/qgstextbackgroundsettings.h
  textrenderer/qgstextblock.h
  textrenderer/qgstextbuffersettings.h
  textrenderer/qgstextcharacterformat.h
  textrenderer/qgstextdocument.h
  textrenderer/qgstextformat.h
  textrenderer/qgstextfragment.h
  textrenderer/qgstextmasksettings.h
  textrenderer/qgstextrenderer.h
  textrenderer/qgstextrendererutils.h
  textrenderer/qgstextshadowsettings.h

  validity/qgsabstractvaliditycheck.h
  validity/qgsvaliditycheckcontext.h
  validity/qgsvaliditycheckregistry.h

  vectortile/qgsvectortilebasiclabeling.h
  vectortile/qgsvectortilebasicrenderer.h
  vectortile/qgsvectortileconnection.h
  vectortile/qgsvectortiledataitems.h
  vectortile/qgsvectortilelabeling.h
  vectortile/qgsvectortilelayer.h
  vectortile/qgsvectortilelayerrenderer.h
  vectortile/qgsvectortileloader.h
  vectortile/qgsvectortilemvtdecoder.h
  vectortile/qgsvectortilemvtencoder.h
  vectortile/qgsvectortilemvtutils.h
  vectortile/qgsvectortileprovidermetadata.h
  vectortile/qgsvectortilerenderer.h
  vectortile/qgsvectortileutils.h
  vectortile/qgsvectortilewriter.h
)

SET(QGIS_CORE_PRIVATE_HDRS
  qgscoordinatereferencesystem_p.h
  qgscoordinatetransformcontext_p.h
  qgscoordinatetransform_p.h
  qgseditformconfig_p.h
  qgsfeature_p.h
  qgsfield_p.h
  qgsfields_p.h
  qgsproperty_p.h
  qgsrelation_p.h
  qgsspatialindexkdbush_p.h

  textrenderer/qgstextrenderer_p.h
)

IF (NOT WITH_QTWEBKIT)
  SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS}
    qgswebframe.h
  )
ENDIF(NOT WITH_QTWEBKIT)

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS}
    gps/qgsqtlocationconnection.h
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)


IF (HAVE_OPENCL)
    SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS}
        qgsopenclutils.h
    )
ENDIF (HAVE_OPENCL)

IF(NOT MSVC)
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND (CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp PROPERTIES COMPILE_FLAGS "-O1")
    SET_SOURCE_FILES_PROPERTIES(qgscoordinatereferencesystem.cpp PROPERTIES COMPILE_FLAGS "-mllvm -inline-threshold=128")
  ENDIF ()
ENDIF(NOT MSVC)

# Generate cpp+header file from .proto file using "protoc" tool (to support MVT encoding of vector tiles)
protobuf_generate_cpp(VECTOR_TILE_PROTO_SRCS VECTOR_TILE_PROTO_HDRS vectortile/vector_tile.proto)
SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS} ${VECTOR_TILE_PROTO_SRCS})
SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS} ${VECTOR_TILE_PROTO_HDRS})
IF (MSVC)
  SET_SOURCE_FILES_PROPERTIES(${VECTOR_TILE_PROTO_SRCS} vectortile/qgsvectortilemvtdecoder.cpp vectortile/qgsvectortilemvtencoder.cpp PROPERTIES COMPILE_DEFINITIONS PROTOBUF_USE_DLLS)
ELSE (MSVC)
  # automatically generated file produces warnings (unused-parameter, unused-variable, misleading-indentation)
  SET_SOURCE_FILES_PROPERTIES(${VECTOR_TILE_PROTO_SRCS} PROPERTIES COMPILE_FLAGS -w)
ENDIF (MSVC)


# install headers
# install qgsconfig.h and plugin.h here so they can get into
# the OS X framework target

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS}
    gps/qgsqtlocationconnection.h
    gps/qgsgpsconnectionregistry.h
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  3d
  annotations
  auth
  callouts
  classification
  dxf
  effects
  expression
  fieldformatter
  geometry
  geocms
  geocms/geonode
  labeling
  layertree
  layout
  locator
  mesh
  metadata
  numericformats
  pal
  processing
  processing/models
  providers
  providers/memory
  providers/gdal
  providers/ogr
  providers/meshmemory
  raster
  renderer
  scalebar
  symbology
  textrenderer
  validity
  vectortile
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann
  ${CMAKE_SOURCE_DIR}/external/kdbush/include
  ${CMAKE_SOURCE_DIR}/external/nmea
  ${CMAKE_SOURCE_DIR}/external/poly2tri
  ${CMAKE_SOURCE_DIR}/external/rtree/include
  ${CMAKE_SOURCE_DIR}/external/meshOptimizer
)

INCLUDE_DIRECTORIES(SYSTEM
  ${PROJ_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${LIBZIP_INCLUDE_DIRS}
  ${SPATIALINDEX_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${Qt5SerialPort_INCLUDE_DIRS}
  ${Protobuf_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
)


IF (HAVE_OPENCL)
    SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
        qgsopenclutils.cpp
    )
    SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS}
        qgsopenclutils.h
    )
    INCLUDE_DIRECTORIES(SYSTEM ${OpenCL_INCLUDE_DIRS})
ENDIF (HAVE_OPENCL)


IF (APPLE)
  # Libtasn1 is for DER-encoded PKI ASN.1 parsing/extracting workarounds
  INCLUDE_DIRECTORIES(SYSTEM
    ${LIBTASN1_INCLUDE_DIR}
  )
ENDIF (APPLE)


#for PAL classes
IF (WIN32)
    ADD_DEFINITIONS(-D_HAVE_WINDOWS_H_)
ELSE (WIN32)
    ADD_DEFINITIONS(-D_HAVE_PTHREAD_)
ENDIF (WIN32)

# Test data dir for QgsRenderChecker
ADD_DEFINITIONS(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

SET(IMAGE_RCCS ../../images/images.qrc)

#############################################################
# qgis_core library
ADD_LIBRARY(qgis_core ${LIBRARY_TYPE} ${QGIS_CORE_SRCS} ${QGIS_CORE_HDRS} ${QGIS_CORE_PRIVATE_HDRS} ${IMAGE_RCCS})

GENERATE_EXPORT_HEADER(
  qgis_core
  BASE_NAME CORE
  EXPORT_FILE_NAME qgis_core.h
)

SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_core.h)

IF(ENABLE_MODELTEST)
  TARGET_LINK_LIBRARIES(qgis_core ${Qt5Test_LIBRARIES})
ENDIF(ENABLE_MODELTEST)


IF(HAVE_OPENCL)
  TARGET_LINK_LIBRARIES(qgis_core ${OpenCL_LIBRARY})
ENDIF(HAVE_OPENCL)


IF(NOT APPLE)
  INSTALL(FILES ${QGIS_CORE_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)

  SET_TARGET_PROPERTIES(qgis_core PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis3_core
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_CORE_HDRS}"
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
  FIND_LIBRARY(VERSION_LIBRARY NAMES version PATHS $ENV{LIB})
  TARGET_LINK_LIBRARIES(qgis_core wsock32 ${SETUPAPI_LIBRARY} dbghelp ${VERSION_LIBRARY})
ENDIF (WIN32)

IF (APPLE)
  TARGET_LINK_LIBRARIES(qgis_core ${LIBTASN1_LIBRARY})
ENDIF (APPLE)

IF (APPLE AND NOT IOS)
  TARGET_LINK_LIBRARIES(qgis_core qgis_native)
ENDIF (APPLE AND NOT IOS)

IF (QT_MOBILITY_LOCATION_FOUND)
  TARGET_LINK_LIBRARIES(qgis_core ${QT_MOBILITY_LOCATION_LIBRARY})
ENDIF (QT_MOBILITY_LOCATION_FOUND)

IF (ADD_CLAZY_CHECKS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_BASE_FLAGS} -Xclang -plugin-arg-clazy -Xclang ${CLAZY_BASE_CHECKS},non-pod-global-static")
ENDIF (ADD_CLAZY_CHECKS)

TARGET_LINK_LIBRARIES(qgis_core
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5PrintSupport_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${OPTIONAL_QTWEBKIT}
  ${Qt5Sql_LIBRARIES}
  ${Qt5Concurrent_LIBRARIES}
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
  ${Protobuf_LITE_LIBRARY}
  ${ZLIB_LIBRARIES}
)

IF (FORCE_STATIC_PROVIDERS)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src/providers/wms
    ${CMAKE_SOURCE_DIR}/src/providers/postgres
  )

  TARGET_LINK_LIBRARIES(qgis_core
    wmsprovider_a
    postgresprovider_a
  )
ENDIF (FORCE_STATIC_PROVIDERS)

IF (MSVC)
  #needed for linking to gdal which needs odbc
  SET(TARGET_LINK_LIBRARIES qgis_core odbc32 odbccp32)
ENDIF (MSVC)

IF (Qt5SerialPort_FOUND)
  TARGET_LINK_LIBRARIES(qgis_core
    Qt5::SerialPort
  )
ENDIF (Qt5SerialPort_FOUND)

IF (Qt5Positioning_FOUND)
  TARGET_LINK_LIBRARIES(qgis_core
    Qt5::Positioning
  )
ENDIF (Qt5Positioning_FOUND)

TARGET_COMPILE_DEFINITIONS(qgis_core PRIVATE "-DQT_NO_FOREACH")

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
