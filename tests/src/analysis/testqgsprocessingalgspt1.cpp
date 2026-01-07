/***************************************************************************
                         testqgsprocessingalgspt1.cpp
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <limits>

#include "annotations/qgsannotationmanager.h"
#include "annotations/qgstextannotation.h"
#include "qgsalgorithmimportphotos.h"
#include "qgsalgorithmkmeansclustering.h"
#include "qgsalgorithmrasterlogicalop.h"
#include "qgsbookmarkmanager.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgscolorrampimpl.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfillsymbol.h"
#include "qgsfontutils.h"
#include "qgsgdalutils.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutmanager.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmeshlayer.h"
#include "qgsmultipolygon.h"
#include "qgsnativealgorithms.h"
#include "qgspallabeling.h"
#include "qgsprintlayout.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingutils.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"
#include "qgsrastershader.h"
#include "qgsrelationmanager.h"
#include "qgsrulebasedrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsstyle.h"
#include "qgstest.h"
#include "qgstextformat.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"

#include <qgsrasteranalysisutils.cpp>

class TestQgsProcessingAlgsPt1 : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingAlgsPt1()
      : QgsTest( u"Processing Algorithms Pt 1"_s )
    {}

  private:
    /**
     * Helper function to get a feature based algorithm.
     */
    std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> featureBasedAlg( const QString &id );

    QgsFeature runForFeature( const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> &alg, QgsFeature feature, const QString &layerType, QVariantMap parameters = QVariantMap() );

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.
    void saveFeaturesAlg();
    void packageAlg();
    void rasterLayerProperties();
    void exportToSpreadsheetXlsx();
    void exportToSpreadsheetOds();
    void exportToSpreadsheetOptions();
    void renameLayerAlg();
    void loadLayerAlg();
    void parseGeoTags();
    void featureFilterAlg();
    void transformAlg();
    void kmeansCluster();
    void categorizeByStyle();
    void extractBinary();
    void exportLayersInformationAlg();
    void createDirectory();
    void flattenRelations();

    void polygonsToLines_data();
    void polygonsToLines();

    void roundness_data();
    void roundness();

    void createConstantRaster_data();
    void createConstantRaster();

    void rasterRank_data();
    void rasterRank();

    void densifyGeometries_data();
    void densifyGeometries();

    void fillNoData_data();
    void fillNoData();

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 13, 0 )
    void rasterCOGOutput();
#endif

    void lineDensity_data();
    void lineDensity();

    void rasterLogicOp_data();
    void rasterLogicOp();
    void cellStatistics_data();
    void cellStatistics();
    void percentileFunctions_data();
    void percentileFunctions();
    void percentileRaster_data();
    void percentileRaster();
    void percentrankFunctions_data();
    void percentrankFunctions();
    void percentrankByRaster_data();
    void percentrankByRaster();
    void percentrankByValue_data();
    void percentrankByValue();
    void rasterFrequencyByComparisonOperator_data();
    void rasterFrequencyByComparisonOperator();
    void rasterLocalPosition_data();
    void rasterLocalPosition();
    void roundRasterValues_data();
    void roundRasterValues();

    void layoutMapExtent();

    void styleFromProject();
    void combineStyles();

    void bookmarksToLayer();
    void layerToBookmarks();

    void repairShapefile();
    void renameField();

    void compareDatasets();
    void shapefileEncoding();
    void setLayerEncoding();

    void raiseException();
    void raiseWarning();
    void raiseMessage();

    void randomFloatingPointDistributionRaster_data();
    void randomFloatingPointDistributionRaster();
    void randomIntegerDistributionRaster_data();
    void randomIntegerDistributionRaster();
    void randomRaster_data();
    void randomRaster();

    void filterByLayerType();
    void conditionalBranch();

    void saveLog();
    void setProjectVariable();

    // WARNING this test is "full" -- adding more to it will cause timeouts on CI! Add to testqgsprocessingalgspt(N+1).cpp instead

  private:
    QString mPointLayerPath;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;

    void exportToSpreadsheet( const QString &outputPath );
};

std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> TestQgsProcessingAlgsPt1::featureBasedAlg( const QString &id )
{
  return std::unique_ptr<QgsProcessingFeatureBasedAlgorithm>( static_cast<QgsProcessingFeatureBasedAlgorithm *>( QgsApplication::processingRegistry()->createAlgorithmById( id ) ) );
}

QgsFeature TestQgsProcessingAlgsPt1::runForFeature( const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> &alg, QgsFeature feature, const QString &layerType, QVariantMap parameters )
{
  Q_ASSERT( alg.get() );
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;
  context->setFeedback( &feedback );

  auto inputLayer = std::make_unique<QgsVectorLayer>( layerType, u"layer"_s, u"memory"_s );
  inputLayer->dataProvider()->addFeature( feature );

  parameters.insert( u"INPUT"_s, QVariant::fromValue<QgsMapLayer *>( inputLayer.get() ) );

  parameters.insert( u"OUTPUT"_s, u"memory:"_s );

  bool ok = false;
  const auto res = alg->run( parameters, *context, &feedback, &ok );
  QgsFeature result;

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( res.value( u"OUTPUT"_s ).toString() ) ) );
  outputLayer->getFeatures().nextFeature( result );

  return result;
}

void TestQgsProcessingAlgsPt1::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  const QString pointsFileName = dataDir + "/points.shp";
  const QFileInfo pointFileInfo( pointsFileName );
  mPointLayerPath = pointFileInfo.filePath();
  mPointsLayer = new QgsVectorLayer( mPointLayerPath, u"points"_s, u"ogr"_s );
  QVERIFY( mPointsLayer->isValid() );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPointsLayer
  );

  //
  //create a poly layer that will be used in all tests...
  //
  const QString polysFileName = dataDir + "/polys.shp";
  const QFileInfo polyFileInfo( polysFileName );
  mPolygonLayer = new QgsVectorLayer( polyFileInfo.filePath(), u"polygons"_s, u"ogr"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPolygonLayer
  );
  QVERIFY( mPolygonLayer->isValid() );

  //add a mesh layer
  const QString uri( dataDir + "/mesh/quad_and_triangle.2dm" );
  const QString meshLayerName = u"mesh layer"_s;
  QgsMeshLayer *meshLayer = new QgsMeshLayer( uri, meshLayerName, u"mdal"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayer( meshLayer );
  QVERIFY( meshLayer->isValid() );
  meshLayer->addDatasets( dataDir + "/mesh/quad_and_triangle_vertex_scalar.dat" );
  meshLayer->addDatasets( dataDir + "/mesh/quad_and_triangle_vertex_vector.dat" );
  meshLayer->addDatasets( dataDir + "/mesh/quad_and_triangle_els_face_scalar.dat" );
  meshLayer->addDatasets( dataDir + "/mesh/quad_and_triangle_els_face_vector.dat" );
  QCOMPARE( meshLayer->datasetGroupCount(), 5 );

  //add a 1D mesh layer
  const QString uri1d( dataDir + "/mesh/lines.2dm" );
  const QString meshLayer1dName = u"mesh layer 1D"_s;
  QgsMeshLayer *meshLayer1d = new QgsMeshLayer( uri1d, meshLayer1dName, u"mdal"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayer( meshLayer1d );
  QVERIFY( meshLayer1d->isValid() );
  meshLayer1d->addDatasets( dataDir + "/mesh/lines_els_scalar.dat" );
  meshLayer1d->addDatasets( dataDir + "/mesh/lines_els_vector.dat" );
  QCOMPARE( meshLayer1d->datasetGroupCount(), 3 );

  /* Make sure geopackages are not written-to, during tests
   * See https://github.com/qgis/QGIS/issues/25830
   * NOTE: this needs to happen _after_
   * QgsApplication::initQgis()
   *       as any previously-set value would otherwise disappear.
   */
  QgsSettings().setValue( "qgis/walForSqlite3", false );
}

void TestQgsProcessingAlgsPt1::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QVariantMap pkgAlg( const QStringList &layers, const QString &outputGpkg, bool overwrite, bool selectedFeaturesOnly, bool saveMetadata, bool *ok )
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( u"native:package"_s ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;
  parameters.insert( u"LAYERS"_s, layers );
  parameters.insert( u"OUTPUT"_s, outputGpkg );
  parameters.insert( u"OVERWRITE"_s, overwrite );
  parameters.insert( u"SELECTED_FEATURES_ONLY"_s, selectedFeaturesOnly );
  parameters.insert( u"SAVE_METADATA"_s, saveMetadata );
  return package->run( parameters, *context, &feedback, ok );
}

void TestQgsProcessingAlgsPt1::saveFeaturesAlg()
{
  const QString outputGeoJson = QDir::tempPath() + "/savefeatures_alg.geojson";
  const QString layerName = u"custom_layer"_s;

  if ( QFile::exists( outputGeoJson ) )
    QFile::remove( outputGeoJson );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QString( dataDir + "/points.shp" ) );
  parameters.insert( u"LAYER_NAME"_s, layerName );
  parameters.insert( u"LAYER_OPTIONS"_s, u"COORDINATE_PRECISION=1"_s );
  parameters.insert( u"OUTPUT"_s, outputGeoJson );

  const QgsProcessingAlgorithm *saveFeatures( QgsApplication::processingRegistry()->algorithmById( u"native:savefeatures"_s ) );
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;
  bool ok = false;
  const QVariantMap outputs = saveFeatures->run( parameters, *context, &feedback, &ok );
  QCOMPARE( ok, true );
  QCOMPARE( outputs.value( u"OUTPUT"_s ).toString(), u"%1|layername=%2"_s.arg( outputGeoJson, layerName ) );
  QCOMPARE( outputs.value( u"FILE_PATH"_s ).toString(), outputGeoJson );
  QCOMPARE( outputs.value( u"LAYER_NAME"_s ).toString(), layerName );

  auto savedLayer = std::make_unique<QgsVectorLayer>( outputs.value( u"OUTPUT"_s ).toString(), "points", "ogr" );
  QVERIFY( savedLayer->isValid() );
  QCOMPARE( savedLayer->getFeature( 1 ).geometry().asPoint().x(), -83.3 );
}

void TestQgsProcessingAlgsPt1::exportLayersInformationAlg()
{
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString gpkgFileName = dataDir + "/humanbeings.gpkg";
  const QFileInfo gpkgFileInfo( gpkgFileName );
  const std::unique_ptr<QgsVectorLayer> gpkgLayer = std::make_unique<QgsVectorLayer>( gpkgFileInfo.filePath() + u"|layername=person"_s, u"person"_s, u"ogr"_s );

  const QgsProcessingAlgorithm *exportLayersInformation( QgsApplication::processingRegistry()->algorithmById( u"native:exportlayersinformation"_s ) );

  QgsProcessingContext context;
  ;
  context.setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;

  QVariantMap parameters;
  parameters.insert( u"LAYERS"_s, QVariantList() << QVariant::fromValue( gpkgLayer.get() ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  bool ok = false;
  QVariantMap results = exportLayersInformation->run( parameters, context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( context.getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( vlayer );
  QCOMPARE( vlayer->featureCount(), 1L );
  QCOMPARE( vlayer->crs().authid(), u"EPSG:2056"_s );

  parameters.insert( u"LAYERS"_s, QVariantList() << QVariant::fromValue( gpkgLayer.get() ) << QVariant::fromValue( mPolygonLayer ) );
  ok = false;
  results = exportLayersInformation->run( parameters, context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );

  vlayer = qobject_cast<QgsVectorLayer *>( context.getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( vlayer );
  QCOMPARE( vlayer->featureCount(), 2L );
  // when layers have mixed CRSes, the algorithm uses WGS84
  QCOMPARE( vlayer->crs().authid(), u"EPSG:4326"_s );
}

void TestQgsProcessingAlgsPt1::packageAlg()
{
  const QString outputGpkg = QDir::tempPath() + "/package_alg.gpkg";

  if ( QFile::exists( outputGpkg ) )
    QFile::remove( outputGpkg );

  const QStringList layers = QStringList() << mPointsLayer->id() << mPolygonLayer->id();
  bool ok = false;
  const QVariantMap results = pkgAlg( layers, outputGpkg, true, false, false, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  auto pointLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  QCOMPARE( pointLayer->wkbType(), mPointsLayer->wkbType() );
  QCOMPARE( pointLayer->featureCount(), mPointsLayer->featureCount() );
  pointLayer.reset();
  auto polygonLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( polygonLayer->isValid() );
  QCOMPARE( polygonLayer->wkbType(), mPolygonLayer->wkbType() );
  QCOMPARE( polygonLayer->featureCount(), mPolygonLayer->featureCount() );
  polygonLayer.reset();

  auto rectangles = std::make_unique<QgsVectorLayer>( QStringLiteral( TEST_DATA_DIR ) + "/rectangles.shp", u"rectangles"_s, u"ogr"_s );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << rectangles.get() );
  QgsLayerMetadata metadata;
  metadata.setFees( u"lots of dogecoin"_s );
  rectangles->setMetadata( metadata );

  // Test adding an additional layer (overwrite disabled)
  const QVariantMap results2 = pkgAlg( QStringList() << rectangles->id(), outputGpkg, false, false, false, &ok );
  QVERIFY( ok );

  QVERIFY( !results2.value( u"OUTPUT"_s ).toString().isEmpty() );
  auto rectanglesPackagedLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->wkbType(), rectanglesPackagedLayer->wkbType() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );
  rectanglesPackagedLayer.reset();

  pointLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  pointLayer.reset();

  // And finally, test with overwrite enabled
  QVariantMap results3 = pkgAlg( QStringList() << rectangles->id(), outputGpkg, true, false, false, &ok );
  QVERIFY( ok );

  QVERIFY( !results2.value( u"OUTPUT"_s ).toString().isEmpty() );
  rectanglesPackagedLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->wkbType(), rectanglesPackagedLayer->wkbType() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );

  pointLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( !pointLayer->isValid() ); // It's gone -- the gpkg was recreated with a single layer

  QCOMPARE( rectanglesPackagedLayer->metadata().fees(), QString() );
  rectanglesPackagedLayer.reset();

  // save layer metadata
  results3 = pkgAlg( QStringList() << rectangles->id(), outputGpkg, true, false, true, &ok );
  QVERIFY( ok );
  rectanglesPackagedLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->wkbType(), rectanglesPackagedLayer->wkbType() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );
  QCOMPARE( rectanglesPackagedLayer->metadata().fees(), u"lots of dogecoin"_s );

  // Test saving of selected features only
  mPolygonLayer->selectByIds( QgsFeatureIds() << 1 << 2 << 3 );
  const QVariantMap results4 = pkgAlg( QStringList() << mPolygonLayer->id(), outputGpkg, false, true, false, &ok );
  QVERIFY( ok );

  QVERIFY( !results4.value( u"OUTPUT"_s ).toString().isEmpty() );
  auto selectedPolygonsPackagedLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( selectedPolygonsPackagedLayer->isValid() );
  QCOMPARE( selectedPolygonsPackagedLayer->wkbType(), mPolygonLayer->wkbType() );
  QCOMPARE( selectedPolygonsPackagedLayer->featureCount(), 3 );
  selectedPolygonsPackagedLayer.reset();

  mPolygonLayer->removeSelection();
  const QVariantMap results5 = pkgAlg( QStringList() << mPolygonLayer->id(), outputGpkg, false, true, false, &ok );
  QVERIFY( ok );

  QVERIFY( !results5.value( u"OUTPUT"_s ).toString().isEmpty() );
  selectedPolygonsPackagedLayer = std::make_unique<QgsVectorLayer>( outputGpkg + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( selectedPolygonsPackagedLayer->isValid() );
  QCOMPARE( selectedPolygonsPackagedLayer->wkbType(), mPolygonLayer->wkbType() );
  QCOMPARE( selectedPolygonsPackagedLayer->featureCount(), 0 ); // With enabled SELECTED_FEATURES_ONLY no features should be saved when there is no selection
}

void TestQgsProcessingAlgsPt1::rasterLayerProperties()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:rasterlayerproperties"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CMakeLists.txt

  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, QVariant( myDataPath + "/landsat.tif" ) );

  //run alg...
  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"X_MIN"_s ).toDouble(), 781662.375 );
  QCOMPARE( results.value( u"X_MAX"_s ).toDouble(), 793062.375 );
  QCOMPARE( results.value( u"Y_MIN"_s ).toDouble(), 3339523.125 );
  QCOMPARE( results.value( u"Y_MAX"_s ).toDouble(), 3350923.125 );
  QCOMPARE( results.value( u"EXTENT"_s ).toString(), u"781662.3750000000000000,3339523.1250000000000000 : 793062.3750000000000000,3350923.1250000000000000"_s );
  QCOMPARE( results.value( u"PIXEL_WIDTH"_s ).toDouble(), 57.0 );
  QCOMPARE( results.value( u"PIXEL_HEIGHT"_s ).toDouble(), 57.0 );
  QCOMPARE( results.value( u"CRS_AUTHID"_s ).toString(), u"EPSG:32633"_s );
  QCOMPARE( results.value( u"WIDTH_IN_PIXELS"_s ).toInt(), 200 );
  QCOMPARE( results.value( u"HEIGHT_IN_PIXELS"_s ).toInt(), 200 );
  QCOMPARE( results.value( u"BAND_COUNT"_s ).toInt(), 9 );

  parameters.insert( u"INPUT"_s, QVariant( myDataPath + "/raster/valueRas3_float64.asc" ) );
  parameters.insert( u"BAND"_s, 1 );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"X_MIN"_s ).toDouble(), 0.0 );
  QCOMPARE( results.value( u"X_MAX"_s ).toDouble(), 4.0 );
  QCOMPARE( results.value( u"Y_MIN"_s ).toDouble(), 0.0 );
  QCOMPARE( results.value( u"Y_MAX"_s ).toDouble(), 4.0 );
  QCOMPARE( results.value( u"EXTENT"_s ).toString(), u"0.0000000000000000,0.0000000000000000 : 4.0000000000000000,4.0000000000000000"_s );
  QCOMPARE( results.value( u"PIXEL_WIDTH"_s ).toDouble(), 1.0 );
  QCOMPARE( results.value( u"PIXEL_HEIGHT"_s ).toDouble(), 1.0 );
  QCOMPARE( results.value( u"CRS_AUTHID"_s ).toString(), QString() );
  QCOMPARE( results.value( u"WIDTH_IN_PIXELS"_s ).toInt(), 4 );
  QCOMPARE( results.value( u"HEIGHT_IN_PIXELS"_s ).toInt(), 4 );
  QCOMPARE( results.value( u"BAND_COUNT"_s ).toInt(), 1 );
  QCOMPARE( results.value( u"HAS_NODATA_VALUE"_s ).toInt(), 1 );
  QCOMPARE( results.value( u"NODATA_VALUE"_s ).toInt(), -9999 );
}

void TestQgsProcessingAlgsPt1::exportToSpreadsheetXlsx()
{
  if ( QgsTest::isCIRun() )
  {
    QSKIP( "XLSX driver not working on Travis" );
  }

  const QString outputPath = QDir::tempPath() + "/spreadsheet.xlsx";
  exportToSpreadsheet( outputPath );
}

void TestQgsProcessingAlgsPt1::exportToSpreadsheetOds()
{
  const QString outputPath = QDir::tempPath() + "/spreadsheet.ods";
  exportToSpreadsheet( outputPath );
}

void TestQgsProcessingAlgsPt1::exportToSpreadsheetOptions()
{
  const QString outputPath = QDir::tempPath() + "/spreadsheet.ods";
  if ( QFile::exists( outputPath ) )
    QFile::remove( outputPath );

  QVariantMap parameters;
  const QStringList layers = QStringList() << mPointsLayer->id();
  bool ok = false;

  mPointsLayer->setFieldAlias( 1, u"my heading"_s );
  mPointsLayer->setFieldAlias( 2, u"my importance"_s );

  const QgsProcessingAlgorithm *alg( QgsApplication::processingRegistry()->algorithmById( u"native:exporttospreadsheet"_s ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  parameters.insert( u"LAYERS"_s, layers );
  parameters.insert( u"OUTPUT"_s, outputPath );
  parameters.insert( u"OVERWRITE"_s, true );
  parameters.insert( u"USE_ALIAS"_s, false );
  QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  auto pointLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=points", "points", "ogr" );
  QCOMPARE( pointLayer->fields().at( 0 ).name(), u"Class"_s );
  QCOMPARE( pointLayer->fields().at( 1 ).name(), u"Heading"_s );
  QCOMPARE( pointLayer->fields().at( 2 ).name(), u"Importance"_s );
  QCOMPARE( pointLayer->fields().at( 3 ).name(), u"Pilots"_s );
  QCOMPARE( pointLayer->fields().at( 4 ).name(), u"Cabin Crew"_s );

  pointLayer.reset();

  mPointsLayer->setEditorWidgetSetup( 2, QgsEditorWidgetSetup( u"ValueMap"_s, QVariantMap { { "map", QVariantMap { { "High", "1" }, { "Medium", "10" }, { "Low", "20" }, { "VLow", "3" }, { "VHigh", "4" } } } } ) );

  parameters.insert( u"USE_ALIAS"_s, true );
  parameters.insert( u"FORMATTED_VALUES"_s, false );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  pointLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=points", "points", "ogr" );
  QCOMPARE( pointLayer->fields().at( 0 ).name(), u"Class"_s );
  QCOMPARE( pointLayer->fields().at( 1 ).name(), u"my heading"_s );
  QCOMPARE( pointLayer->fields().at( 2 ).name(), u"my importance"_s );
  QCOMPARE( pointLayer->fields().at( 3 ).name(), u"Pilots"_s );
  QCOMPARE( pointLayer->fields().at( 4 ).name(), u"Cabin Crew"_s );

  QSet<QString> values;
  QgsFeature f;
  QgsFeatureIterator it = pointLayer->getFeatures();
  while ( it.nextFeature( f ) )
    values.insert( f.attribute( u"my importance"_s ).toString() );

  QCOMPARE( values.size(), 5 );
  QVERIFY( values.contains( "1" ) );
  QVERIFY( values.contains( "3" ) );
  QVERIFY( values.contains( "4" ) );
  QVERIFY( values.contains( "10" ) );
  QVERIFY( values.contains( "20" ) );

  parameters.insert( u"FORMATTED_VALUES"_s, true );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  pointLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=points", "points", "ogr" );
  QCOMPARE( pointLayer->fields().at( 0 ).name(), u"Class"_s );
  QCOMPARE( pointLayer->fields().at( 1 ).name(), u"my heading"_s );
  QCOMPARE( pointLayer->fields().at( 2 ).name(), u"my importance"_s );
  QCOMPARE( pointLayer->fields().at( 3 ).name(), u"Pilots"_s );
  QCOMPARE( pointLayer->fields().at( 4 ).name(), u"Cabin Crew"_s );

  values.clear();
  it = pointLayer->getFeatures();
  while ( it.nextFeature( f ) )
    values.insert( f.attribute( u"my importance"_s ).toString() );

  QCOMPARE( values.size(), 5 );
  QVERIFY( values.contains( "High" ) );
  QVERIFY( values.contains( "Medium" ) );
  QVERIFY( values.contains( "Low" ) );
  QVERIFY( values.contains( "VLow" ) );
  QVERIFY( values.contains( "VHigh" ) );
}

void TestQgsProcessingAlgsPt1::exportToSpreadsheet( const QString &outputPath )
{
  if ( QFile::exists( outputPath ) )
    QFile::remove( outputPath );

  QVariantMap parameters;
  const QStringList layers = QStringList() << mPointsLayer->id() << mPolygonLayer->id();
  bool ok = false;

  const QgsProcessingAlgorithm *alg( QgsApplication::processingRegistry()->algorithmById( u"native:exporttospreadsheet"_s ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  parameters.insert( u"LAYERS"_s, layers );
  parameters.insert( u"OUTPUT"_s, outputPath );
  parameters.insert( u"OVERWRITE"_s, false );
  const QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  auto pointLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  QCOMPARE( pointLayer->featureCount(), mPointsLayer->featureCount() );
  pointLayer.reset();
  auto polygonLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( polygonLayer->isValid() );
  QCOMPARE( polygonLayer->featureCount(), mPolygonLayer->featureCount() );
  polygonLayer.reset();

  auto rectangles = std::make_unique<QgsVectorLayer>( QStringLiteral( TEST_DATA_DIR ) + "/rectangles.shp", u"rectangles"_s, u"ogr"_s );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << rectangles.get() );

  // Test adding an additional layer (overwrite disabled)
  parameters.insert( u"LAYERS"_s, QStringList() << rectangles->id() );
  const QVariantMap results2 = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results2.value( u"OUTPUT"_s ).toString().isEmpty() );
  auto rectanglesPackagedLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );
  rectanglesPackagedLayer.reset();

  pointLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  pointLayer.reset();

  // And finally, test with overwrite enabled
  parameters.insert( u"OVERWRITE"_s, true );
  const QVariantMap results3 = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results3.value( u"OUTPUT"_s ).toString().isEmpty() );
  rectanglesPackagedLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );

  pointLayer = std::make_unique<QgsVectorLayer>( outputPath + "|layername=points", "points", "ogr" );
  QVERIFY( !pointLayer->isValid() ); // It's gone -- the xlsx was recreated with a single layer
}


void TestQgsProcessingAlgsPt1::renameLayerAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( u"native:renamelayer"_s ) );
  QVERIFY( package );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?field=col1:real"_s, u"layer"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;

  // bad layer
  parameters.insert( u"INPUT"_s, u"bad layer"_s );
  parameters.insert( u"NAME"_s, u"new name"_s );
  bool ok = false;
  ( void ) package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QCOMPARE( layer->name(), u"layer"_s );

  //invalid name
  parameters.insert( u"INPUT"_s, u"layer"_s );
  parameters.insert( u"NAME"_s, QString() );
  ok = false;
  ( void ) package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QCOMPARE( layer->name(), u"layer"_s );

  //good params
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer ) );
  parameters.insert( u"NAME"_s, u"new name"_s );
  ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( layer->name(), u"new name"_s );
  QCOMPARE( results.value( "OUTPUT" ), QVariant::fromValue( layer ) );

  // with input layer name as parameter
  parameters.insert( u"INPUT"_s, u"new name"_s );
  parameters.insert( u"NAME"_s, u"new name2"_s );
  ok = false;
  results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( layer->name(), u"new name2"_s );
  // result should use new name as value
  QCOMPARE( results.value( "OUTPUT" ).toString(), u"new name2"_s );
}

void TestQgsProcessingAlgsPt1::loadLayerAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( u"native:loadlayer"_s ) );
  QVERIFY( package );

  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;

  // bad layer
  parameters.insert( u"INPUT"_s, u"bad layer"_s );
  parameters.insert( u"NAME"_s, u"new name"_s );
  bool ok = false;
  ( void ) package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( context->layersToLoadOnCompletion().empty() );

  //invalid name
  parameters.insert( u"INPUT"_s, mPointLayerPath );
  parameters.insert( u"NAME"_s, QString() );
  ok = false;
  ( void ) package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( context->layersToLoadOnCompletion().empty() );

  //good params
  parameters.insert( u"INPUT"_s, mPointLayerPath );
  parameters.insert( u"NAME"_s, u"my layer"_s );
  ok = false;
  const QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !context->layersToLoadOnCompletion().empty() );
  const QString layerId = context->layersToLoadOnCompletion().keys().at( 0 );
  QCOMPARE( results.value( u"OUTPUT"_s ).toString(), layerId );
  QVERIFY( !layerId.isEmpty() );
  QVERIFY( context->temporaryLayerStore()->mapLayer( layerId ) );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).name, u"my layer"_s );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).project, &p );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).outputName, u"my layer"_s );
}

void TestQgsProcessingAlgsPt1::parseGeoTags()
{
  // parseCoord
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "" ).isValid() );
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "x" ).isValid() );
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "1 2 3" ).isValid() );
  QGSCOMPARENEAR( QgsImportPhotosAlgorithm::parseCoord( "(36) (13) (15.21)" ).toDouble(), 36.220892, 0.000001 );
  QGSCOMPARENEAR( QgsImportPhotosAlgorithm::parseCoord( "(3) (1) (5.21)" ).toDouble(), 3.018114, 0.000001 );
  QGSCOMPARENEAR( QgsImportPhotosAlgorithm::parseCoord( "(149) (7) (54.76)" ).toDouble(), 149.131878, 0.000001 );
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "(149) (7) (c)" ).isValid() );
  QVERIFY( !QgsImportPhotosAlgorithm::parseCoord( "(149) (7) ()" ).isValid() );

  // parseMetadataValue
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "abc" ).toString(), u"abc"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "(abc)" ).toString(), u"(abc)"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "abc (123)" ).toString(), u"abc (123)"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "(123)" ).toDouble(), 123.0 );

  // parseMetadataList
  QVariantMap md = QgsImportPhotosAlgorithm::parseMetadataList( QStringList() << "EXIF_Contrast=(1)"
                                                                              << "EXIF_ExposureTime=(0.008339)"
                                                                              << "EXIF_Model=Pixel"
                                                                              << "EXIF_GPSLatitude=(36) (13) (15.21)"
                                                                              << "EXIF_GPSLongitude=(149) (7) (54.76)" );
  QCOMPARE( md.count(), 5 );
  QCOMPARE( md.value( "EXIF_Contrast" ).toInt(), 1 );
  QCOMPARE( md.value( "EXIF_ExposureTime" ).toDouble(), 0.008339 );
  QCOMPARE( md.value( "EXIF_Model" ).toString(), u"Pixel"_s );
  QGSCOMPARENEAR( md.value( "EXIF_GPSLatitude" ).toDouble(), 36.220892, 0.000001 );
  QGSCOMPARENEAR( md.value( "EXIF_GPSLongitude" ).toDouble(), 149.131878, 0.000001 );

  // test extractGeoTagFromMetadata
  md = QVariantMap();
  QgsPointXY point;
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( u"EXIF_GPSLongitude"_s, 142.0 );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( u"EXIF_GPSLatitude"_s, 37.0 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLongitude"_s, u"x"_s );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( u"EXIF_GPSLongitude"_s, 142.0 );
  md.insert( u"EXIF_GPSLatitude"_s, u"x"_s );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( u"EXIF_GPSLatitude"_s, 37.0 );
  md.insert( u"EXIF_GPSLongitudeRef"_s, u"E"_s );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLongitudeRef"_s, u"W"_s );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLongitudeRef"_s, u"w"_s );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLongitudeRef"_s, u"...W"_s ); // apparently any string ENDING in W is acceptable
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLongitudeRef"_s, QString() );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLongitudeRef"_s, -1 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLongitudeRef"_s, QString() );
  md.insert( u"EXIF_GPSLatitudeRef"_s, u"N"_s );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLatitudeRef"_s, u"S"_s );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( u"EXIF_GPSLatitudeRef"_s, u"s"_s );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( u"EXIF_GPSLatitudeRef"_s, u"...S"_s ); // any string ending in s is acceptable
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( u"EXIF_GPSLatitudeRef"_s, QString() );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( u"EXIF_GPSLatitudeRef"_s, -1 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );

  // extractAltitudeFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).isValid() );
  md.insert( u"EXIF_GPSAltitude"_s, 10.5 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( u"EXIF_GPSAltitudeRef"_s, u"0"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( u"EXIF_GPSAltitudeRef"_s, u"00"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( u"EXIF_GPSAltitudeRef"_s, u"1"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );
  md.insert( u"EXIF_GPSAltitudeRef"_s, u"01"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );
  md.insert( u"EXIF_GPSAltitudeRef"_s, 1 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( u"EXIF_GPSAltitudeRef"_s, -1 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );

  // extractDirectionFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractDirectionFromMetadata( md ).isValid() );
  md.insert( u"EXIF_GPSImgDirection"_s, 15 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractDirectionFromMetadata( md ).toDouble(), 15.0 );

  // extractOrientationFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractOrientationFromMetadata( md ).isValid() );
  md.insert( u"EXIF_Orientation"_s, 3 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractOrientationFromMetadata( md ).toInt(), 180 );

  // extractTimestampFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).isValid() );
  md.insert( u"EXIF_DateTimeOriginal"_s, u"xx"_s );
  QVERIFY( !QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).isValid() );
  md.insert( u"EXIF_DateTimeOriginal"_s, u"2017:12:27 19:20:52"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );
  md.remove( u"EXIF_DateTimeOriginal"_s );
  md.insert( u"EXIF_DateTimeDigitized"_s, u"2017:12:27 19:20:52"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );
  md.remove( u"EXIF_DateTimeDigitized"_s );
  md.insert( u"EXIF_DateTime"_s, u"2017:12:27 19:20:52"_s );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );
}

void TestQgsProcessingAlgsPt1::featureFilterAlg()
{
  const QgsProcessingAlgorithm *filterAlgTemplate = QgsApplication::processingRegistry()->algorithmById( u"native:filter"_s );

  Q_ASSERT( filterAlgTemplate->outputDefinitions().isEmpty() );

  QVariantList outputs;
  QVariantMap output1;
  output1.insert( u"name"_s, u"test"_s );
  output1.insert( u"expression"_s, u"TRUE"_s );
  output1.insert( u"isModelOutput"_s, true );

  outputs.append( output1 );

  QVariantMap config1;
  config1.insert( u"outputs"_s, outputs );

  std::unique_ptr<QgsProcessingAlgorithm> filterAlg( filterAlgTemplate->create( config1 ) );

  QCOMPARE( filterAlg->outputDefinitions().size(), 1 );

  auto outputDef = filterAlg->outputDefinition( u"OUTPUT_test"_s );
  QCOMPARE( outputDef->type(), u"outputVector"_s );

  auto outputParamDef = filterAlg->parameterDefinition( "OUTPUT_test" );
  Q_ASSERT( outputParamDef->flags() & Qgis::ProcessingParameterFlag::IsModelOutput );
  Q_ASSERT( outputParamDef->flags() & Qgis::ProcessingParameterFlag::Hidden );

  QVariantMap output2;
  output2.insert( u"name"_s, u"nonmodeloutput"_s );
  output2.insert( u"expression"_s, u"TRUE"_s );
  output2.insert( u"isModelOutput"_s, false );

  outputs.append( output2 );

  QVariantMap config2;
  config2.insert( u"outputs"_s, outputs );

  std::unique_ptr<QgsProcessingAlgorithm> filterAlg2( filterAlgTemplate->create( config2 ) );

  QCOMPARE( filterAlg2->outputDefinitions().size(), 2 );

  auto outputDef2 = filterAlg2->outputDefinition( u"OUTPUT_nonmodeloutput"_s );
  QCOMPARE( outputDef2->type(), u"outputVector"_s );

  auto outputParamDef2 = filterAlg2->parameterDefinition( "OUTPUT_nonmodeloutput" );
  Q_ASSERT( !outputParamDef2->flags().testFlag( Qgis::ProcessingParameterFlag::IsModelOutput ) );
  Q_ASSERT( outputParamDef2->flags() & Qgis::ProcessingParameterFlag::Hidden );
}

void TestQgsProcessingAlgsPt1::transformAlg()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:reprojectlayer"_s ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;

  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326field=col1:integer"_s, u"test"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  QgsFeature f;
  // add a point with a bad geometry - this should result in a transform exception!
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -96215069, 41.673559 ) ) );
  QVERIFY( layer->dataProvider()->addFeature( f ) );
  p.addMapLayer( layer );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, u"test"_s );
  parameters.insert( u"OUTPUT"_s, u"memory:"_s );
  parameters.insert( u"TARGET_CRS"_s, u"EPSG:2163"_s );
  bool ok = false;
  const QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  Q_UNUSED( results )
  QVERIFY( ok );
}

void TestQgsProcessingAlgsPt1::kmeansCluster()
{
  // make some features
  std::vector<QgsKMeansClusteringAlgorithm::Feature> features;
  std::vector<QgsPointXY> centers( 2 );

  // no features, no crash
  int k = 2;
  // farthest points
  QgsKMeansClusteringAlgorithm::initClustersFarthestPoints( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  // kmeans++
  QgsKMeansClusteringAlgorithm::initClustersPlusPlus( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );

  // features < clusters
  // farthest points
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 1 ) ) );
  QgsKMeansClusteringAlgorithm::initClustersFarthestPoints( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[0].cluster, 0 );
  // kmeans++
  QgsKMeansClusteringAlgorithm::initClustersPlusPlus( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[0].cluster, 0 );

  // features == clusters
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 3, 1 ) ) );
  // farthest points
  QgsKMeansClusteringAlgorithm::initClustersFarthestPoints( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[0].cluster, 1 );
  QCOMPARE( features[1].cluster, 0 );
  // kmeans++
  QgsKMeansClusteringAlgorithm::initClustersPlusPlus( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QVERIFY( features[0].cluster != features[1].cluster );

  // features > clusters
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 2, 8 ) ) );
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 10 ) ) );
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 3, 10 ) ) );
  k = 2;
  // farthest points
  QgsKMeansClusteringAlgorithm::initClustersFarthestPoints( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[0].cluster, 1 );
  QCOMPARE( features[1].cluster, 1 );
  QCOMPARE( features[2].cluster, 0 );
  QCOMPARE( features[3].cluster, 0 );
  QCOMPARE( features[4].cluster, 0 );
  // kmeans++
  QgsKMeansClusteringAlgorithm::initClustersPlusPlus( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[0].cluster, features[1].cluster );
  QCOMPARE( features[2].cluster, features[3].cluster );
  QCOMPARE( features[4].cluster, features[3].cluster );

  // repeat above, with 3 clusters
  k = 3;
  centers.resize( 3 );
  QgsKMeansClusteringAlgorithm::initClustersFarthestPoints( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[0].cluster, 1 );
  QCOMPARE( features[1].cluster, 1 );
  QCOMPARE( features[2].cluster, 2 );
  QCOMPARE( features[3].cluster, 0 );
  QCOMPARE( features[4].cluster, 0 );

  // with identical points
  features.clear();
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 5 ) ) );
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 5 ) ) );
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 5 ) ) );
  QCOMPARE( features[0].cluster, -1 );
  QCOMPARE( features[1].cluster, -1 );
  QCOMPARE( features[2].cluster, -1 );
}

void TestQgsProcessingAlgsPt1::categorizeByStyle()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:categorizeusingstyle"_s ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString styleFileName = dataDir + "/categorized.xml";


  QgsProcessingFeedback feedback;

  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=col1:string"_s, u"test"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  QgsFeature f, f2, f3;
  f.setAttributes( QgsAttributes() << "a" );
  QVERIFY( layer->dataProvider()->addFeature( f ) );
  f2.setAttributes( QgsAttributes() << "b" );
  QVERIFY( layer->dataProvider()->addFeature( f2 ) );
  f3.setAttributes( QgsAttributes() << "c " );
  QVERIFY( layer->dataProvider()->addFeature( f3 ) );
  p.addMapLayer( layer );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, u"test"_s );
  parameters.insert( u"FIELD"_s, u"col1"_s );
  parameters.insert( u"STYLE"_s, styleFileName );
  parameters.insert( u"CASE_SENSITIVE"_s, true );
  parameters.insert( u"TOLERANT"_s, false );
  parameters.insert( u"NON_MATCHING_CATEGORIES"_s, u"memory:"_s );
  parameters.insert( u"NON_MATCHING_SYMBOLS"_s, u"memory:"_s );

  bool ok = false;
  QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context->layerToLoadOnCompletionDetails( layer->id() ).postProcessor()->postProcessLayer( layer, *context, &feedback );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( layer->renderer() );
  QVERIFY( catRenderer );

  auto allValues = []( QgsVectorLayer *layer ) -> QStringList {
    QStringList all;
    QgsFeature f;
    QgsFeatureIterator it = layer->getFeatures();
    while ( it.nextFeature( f ) )
    {
      all.append( f.attribute( 0 ).toString() );
    }
    return all;
  };
  QgsVectorLayer *nonMatchingCats = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"NON_MATCHING_CATEGORIES"_s ).toString() ) );
  QCOMPARE( allValues( nonMatchingCats ), QStringList() << "b" << "c " );
  QgsVectorLayer *nonMatchingSymbols = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"NON_MATCHING_SYMBOLS"_s ).toString() ) );
  QCOMPARE( allValues( nonMatchingSymbols ), QStringList() << " ----c/- " << "B " );

  QCOMPARE( catRenderer->categories().count(), 3 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"a"_s ) ).symbol()->color().name(), u"#ff0000"_s );
  QVERIFY( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"b"_s ) ).symbol()->color().name() != "#00ff00"_L1 );
  QVERIFY( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"c "_s ) ).symbol()->color().name() != "#0000ff"_L1 );
  // reset renderer
  layer->setRenderer( new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) ) );

  // case insensitive
  parameters.insert( u"CASE_SENSITIVE"_s, false );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context->layerToLoadOnCompletionDetails( layer->id() ).postProcessor()->postProcessLayer( layer, *context, &feedback );
  catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( layer->renderer() );
  QVERIFY( catRenderer );

  nonMatchingCats = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"NON_MATCHING_CATEGORIES"_s ).toString() ) );
  QCOMPARE( allValues( nonMatchingCats ), QStringList() << "c " );
  nonMatchingSymbols = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"NON_MATCHING_SYMBOLS"_s ).toString() ) );
  QCOMPARE( allValues( nonMatchingSymbols ), QStringList() << " ----c/- " );

  QCOMPARE( catRenderer->categories().count(), 3 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"a"_s ) ).symbol()->color().name(), u"#ff0000"_s );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"b"_s ) ).symbol()->color().name(), u"#00ff00"_s );
  QVERIFY( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"c "_s ) ).symbol()->color().name() != "#0000ff"_L1 );
  // reset renderer
  layer->setRenderer( new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) ) );

  // tolerant
  parameters.insert( u"CASE_SENSITIVE"_s, true );
  parameters.insert( u"TOLERANT"_s, true );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context->layerToLoadOnCompletionDetails( layer->id() ).postProcessor()->postProcessLayer( layer, *context, &feedback );
  catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( layer->renderer() );
  QVERIFY( catRenderer );

  nonMatchingCats = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"NON_MATCHING_CATEGORIES"_s ).toString() ) );
  QCOMPARE( allValues( nonMatchingCats ), QStringList() << "b" );
  nonMatchingSymbols = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"NON_MATCHING_SYMBOLS"_s ).toString() ) );
  QCOMPARE( allValues( nonMatchingSymbols ), QStringList() << "B " );

  QCOMPARE( catRenderer->categories().count(), 3 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"a"_s ) ).symbol()->color().name(), u"#ff0000"_s );
  QVERIFY( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"b"_s ) ).symbol()->color().name() != "#00ff00"_L1 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"c "_s ) ).symbol()->color().name(), u"#0000ff"_s );
  // reset renderer
  layer->setRenderer( new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) ) );

  // no optional sinks
  parameters.insert( u"CASE_SENSITIVE"_s, false );
  parameters.remove( u"NON_MATCHING_CATEGORIES"_s );
  parameters.remove( u"NON_MATCHING_SYMBOLS"_s );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context->layerToLoadOnCompletionDetails( layer->id() ).postProcessor()->postProcessLayer( layer, *context, &feedback );
  catRenderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( layer->renderer() );
  QVERIFY( catRenderer );

  QVERIFY( !context->getMapLayer( results.value( u"NON_MATCHING_CATEGORIES"_s ).toString() ) );
  QVERIFY( !context->getMapLayer( results.value( u"NON_MATCHING_SYMBOLS"_s ).toString() ) );

  QCOMPARE( catRenderer->categories().count(), 3 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"a"_s ) ).symbol()->color().name(), u"#ff0000"_s );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"b"_s ) ).symbol()->color().name(), u"#00ff00"_s );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( u"c "_s ) ).symbol()->color().name(), u"#0000ff"_s );
}

void TestQgsProcessingAlgsPt1::extractBinary()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:extractbinary"_s ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString source = dataDir + u"/attachments.gdb|layername=points__ATTACH"_s;

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, source );
  parameters.insert( u"FIELD"_s, u"DATA"_s );
  parameters.insert( u"FILENAME"_s, u"'test' || \"ATTACHMENTID\" || '.jpg'"_s );
  const QString folder = QDir::tempPath();
  parameters.insert( u"FOLDER"_s, folder );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.value( u"FOLDER"_s ).toString(), folder );

  QFile file( folder + "/test1.jpg" );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  QByteArray d = file.readAll();
  QCOMPARE( QString( QCryptographicHash::hash( d, QCryptographicHash::Md5 ).toHex() ), u"ef3dbc530cc39a545832a6c82aac57b6"_s );

  QFile file2( folder + "/test2.jpg" );
  QVERIFY( file2.open( QIODevice::ReadOnly ) );
  d = file2.readAll();
  QCOMPARE( QString( QCryptographicHash::hash( d, QCryptographicHash::Md5 ).toHex() ), u"4b952b80e4288ca5111be2f6dd5d6809"_s );
}

void TestQgsProcessingAlgsPt1::createDirectory()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:createdirectory"_s ) );
  QVERIFY( alg != nullptr );

  // first make a path to an existing file
  QString outputPath = QDir::tempPath() + "/test.txt";
  if ( QFile::exists( outputPath ) )
    QFile::remove( outputPath );
  QFile file( outputPath );
  QVERIFY( file.open( QIODevice::ReadWrite ) );
  file.close();

  QVariantMap parameters;
  parameters.insert( u"PATH"_s, outputPath );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // path is an existing file
  QVERIFY( !ok );

  outputPath = QDir::tempPath() + "/createdir/test/test2";
  QDir().rmdir( QDir::tempPath() + "/createdir/test/test2" );
  QDir().rmdir( QDir::tempPath() + "/createdir/test" );
  QDir().rmdir( QDir::tempPath() + "/createdir" );

  parameters.insert( u"PATH"_s, outputPath );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( outputPath ) );
  QVERIFY( QFileInfo( outputPath ).isDir() );

  // run a second time -- should be OK, no exception raised
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( outputPath ) );
  QVERIFY( QFileInfo( outputPath ).isDir() );
}

void TestQgsProcessingAlgsPt1::flattenRelations()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:flattenrelationships"_s ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QgsProcessingFeedback feedback;

  QgsVectorLayer *parent = new QgsVectorLayer( dataDir + u"/points_relations.shp"_s, u"parent"_s );
  QgsVectorLayer *child = new QgsVectorLayer( dataDir + u"/points.shp"_s, u"child"_s );
  QVERIFY( parent->isValid() );
  QVERIFY( child->isValid() );
  p.addMapLayer( parent );
  p.addMapLayer( child );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, parent->id() );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  // no relations!
  QVERIFY( !ok );

  // create relationship
  const QgsRelationContext relationContext( &p );
  QgsRelation relation( relationContext );
  relation.setId( u"rel"_s );
  relation.setName( u"my relation"_s );
  relation.setReferencedLayer( parent->id() );
  relation.setReferencingLayer( child->id() );
  relation.addFieldPair( u"class"_s, u"class"_s );
  QVERIFY( relation.isValid() );
  p.relationManager()->addRelation( relation );

  results = alg->run( parameters, *context, &feedback, &ok );
  // one relation - should be ok!
  QVERIFY( ok );

  QgsVectorLayer *outputLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( outputLayer );
  QCOMPARE( outputLayer->fields().count(), 8 );
  QCOMPARE( outputLayer->fields().at( 0 ).name(), u"Class"_s );
  QCOMPARE( outputLayer->fields().at( 1 ).name(), u"id"_s );
  QCOMPARE( outputLayer->fields().at( 2 ).name(), u"Class_2"_s );
  QCOMPARE( outputLayer->fields().at( 3 ).name(), u"Heading"_s );
  QCOMPARE( outputLayer->fields().at( 4 ).name(), u"Importance"_s );
  QCOMPARE( outputLayer->fields().at( 5 ).name(), u"Pilots"_s );
  QCOMPARE( outputLayer->fields().at( 6 ).name(), u"Cabin Crew"_s );
  QCOMPARE( outputLayer->fields().at( 7 ).name(), u"Staff"_s );

  QCOMPARE( outputLayer->featureCount(), 17L );

  QSet<QgsAttributes> res;
  QgsFeature f;
  QgsFeatureIterator it = outputLayer->getFeatures();
  while ( it.nextFeature( f ) )
  {
    QgsAttributes created = f.attributes();
    created << f.geometry().asWkt( 0 );
    res.insert( created );
  }

  QVERIFY( res.contains( QgsAttributes() << u"B52"_s << 3 << u"B52"_s << 0LL << 10.0 << 2 << 1 << 3 << u"Point (-103 23)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"B52"_s << 3 << u"B52"_s << 12LL << 10.0 << 1 << 1 << 2 << u"Point (-103 23)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"B52"_s << 3 << u"B52"_s << 34LL << 10.0 << 2 << 1 << 3 << u"Point (-103 23)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"B52"_s << 3 << u"B52"_s << 80LL << 10.0 << 2 << 1 << 3 << u"Point (-103 23)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Jet"_s << 1 << u"Jet"_s << 90LL << 3.0 << 2 << 0 << 2 << u"Point (-117 37)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Jet"_s << 1 << u"Jet"_s << 85LL << 3.0 << 1 << 1 << 2 << u"Point (-117 37)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Jet"_s << 1 << u"Jet"_s << 95LL << 3.0 << 1 << 1 << 2 << u"Point (-117 37)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Jet"_s << 1 << u"Jet"_s << 90LL << 3.0 << 1 << 0 << 1 << u"Point (-117 37)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Jet"_s << 1 << u"Jet"_s << 160LL << 4.0 << 2 << 0 << 2 << u"Point (-117 37)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Jet"_s << 1 << u"Jet"_s << 180LL << 3.0 << 1 << 0 << 1 << u"Point (-117 37)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Jet"_s << 1 << u"Jet"_s << 140LL << 10.0 << 1 << 1 << 2 << u"Point (-117 37)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Jet"_s << 1 << u"Jet"_s << 100LL << 20.0 << 3 << 0 << 3 << u"Point (-117 37)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Biplane"_s << 2 << u"Biplane"_s << 0LL << 1.0 << 3 << 3 << 6 << u"Point (-83 34)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Biplane"_s << 2 << u"Biplane"_s << 340LL << 1.0 << 3 << 3 << 6 << u"Point (-83 34)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Biplane"_s << 2 << u"Biplane"_s << 300LL << 1.0 << 3 << 2 << 5 << u"Point (-83 34)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Biplane"_s << 2 << u"Biplane"_s << 270LL << 1.0 << 3 << 4 << 7 << u"Point (-83 34)"_s ) );
  QVERIFY( res.contains( QgsAttributes() << u"Biplane"_s << 2 << u"Biplane"_s << 240LL << 1.0 << 3 << 2 << 5 << u"Point (-83 34)"_s ) );

  QgsRelation relation2( relationContext );
  relation2.setId( u"rel2"_s );
  relation2.setName( u"my relation2"_s );
  relation2.setReferencedLayer( parent->id() );
  relation2.setReferencingLayer( child->id() );
  relation2.addFieldPair( u"class"_s, u"class"_s );
  QVERIFY( relation2.isValid() );
  p.relationManager()->addRelation( relation2 );
  results = alg->run( parameters, *context, &feedback, &ok );
  // two relations - should not run
  QVERIFY( !ok );
}


void TestQgsProcessingAlgsPt1::polygonsToLines_data()
{
  QTest::addColumn<QgsGeometry>( "sourceGeometry" );
  QTest::addColumn<QgsGeometry>( "expectedGeometry" );

  QTest::newRow( "Simple Polygon" )
    << QgsGeometry::fromWkt( "Polygon((1 1, 2 2, 1 3, 1 1))" )
    << QgsGeometry::fromWkt( "MultiLineString ((1 1, 2 2, 1 3, 1 1))" );

  const QgsGeometry geomNoRing( std::make_unique<QgsMultiPolygon>() );

  QTest::newRow( "Polygon without exterior ring" )
    << geomNoRing
    << QgsGeometry::fromWkt( "MultiLineString ()" );

  QTest::newRow( "MultiPolygon" )
    << QgsGeometry::fromWkt( "MultiPolygon(((1 1, 2 2, 1 3, 1 1)), ((0 0, 0 10, 10 10, 10 0, 0 0), (3 3, 3 6, 6 6, 6 3, 3 3)))" )
    << QgsGeometry::fromWkt( "MultiLineString ((1 1, 2 2, 1 3, 1 1),(0 0, 0 10, 10 10, 10 0, 0 0),(3 3, 3 6, 6 6, 6 3, 3 3))" );

  QTest::newRow( "Polygon with inner ring" )
    << QgsGeometry::fromWkt( "Polygon((0 0, 0 10, 10 10, 10 0, 0 0), (3 3, 3 6, 6 6, 6 3, 3 3))" )
    << QgsGeometry::fromWkt( "MultiLineString ((0 0, 0 10, 10 10, 10 0, 0 0),(3 3, 3 6, 6 6, 6 3, 3 3))" );
}


void TestQgsProcessingAlgsPt1::polygonsToLines()
{
  QFETCH( QgsGeometry, sourceGeometry );
  QFETCH( QgsGeometry, expectedGeometry );

  const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> alg( featureBasedAlg( "native:polygonstolines" ) );

  QgsFeature feature;
  feature.setGeometry( sourceGeometry );

  const QgsFeature result = runForFeature( alg, feature, u"Polygon"_s );

  QVERIFY2( result.geometry().equals( expectedGeometry ), u"Result: %1, Expected: %2"_s.arg( result.geometry().asWkt(), expectedGeometry.asWkt() ).toUtf8().constData() );
}

void TestQgsProcessingAlgsPt1::roundness_data()
{
  QTest::addColumn<QgsGeometry>( "sourceGeometry" );
  QTest::addColumn<double>( "expectedAttribute" );

  QTest::newRow( "Polygon" )
    << QgsGeometry::fromWkt( "POLYGON(( 0 0, 0 1, 1 1, 1 0, 0 0 ))" )
    << 0.785;

  QTest::newRow( "Thin polygon" )
    << QgsGeometry::fromWkt( "POLYGON(( 0 0, 0.5 0, 1 0, 0.6 0, 0 0 ))" )
    << 0.0;

  QTest::newRow( "Circle polygon" )
    << QgsGeometry::fromWkt( "CurvePolygon (CompoundCurve (CircularString (0 0, 0 1, 1 1, 1 0, 0 0)))" )
    << 1.0;

  QTest::newRow( "Polygon with hole" )
    << QgsGeometry::fromWkt( "POLYGON(( 0 0, 0 3, 3 3, 3 0, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))" )
    << 0.393;
}

void TestQgsProcessingAlgsPt1::roundness()
{
  QFETCH( QgsGeometry, sourceGeometry );
  QFETCH( double, expectedAttribute );

  const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> alg( featureBasedAlg( "native:roundness" ) );

  QgsFeature feature;
  feature.setGeometry( sourceGeometry );

  const QgsFeature result = runForFeature( alg, feature, u"Polygon"_s );

  const double roundnessResult = result.attribute( u"roundness"_s ).toDouble();
  QGSCOMPARENEAR( roundnessResult, expectedAttribute, 0.001 );
}

Q_DECLARE_METATYPE( Qgis::DataType )
void TestQgsProcessingAlgsPt1::createConstantRaster_data()
{
  QTest::addColumn<QString>( "inputExtent" );
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );
  QTest::addColumn<QString>( "crs" );
  QTest::addColumn<double>( "pixelSize" );
  QTest::addColumn<double>( "constantValue" );
  QTest::addColumn<int>( "typeId" );

  /*
   * Testcase 1
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * Byte Raster Layer
   *
   */
  QTest::newRow( "testcase 1" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << u"/createConstantRaster_testcase1.tif"_s
    << Qgis::DataType::Byte
    << "EPSG:4326"
    << 1.0
    << 12.0
    << 0;

  /*
   * Testcase 2
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = -1
   * Byte Raster Layer
   *
   */
  QTest::newRow( "testcase 2" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::Byte
    << "EPSG:4326"
    << 1.0
    << -1.0 //fails --> value too small for byte
    << 0;


  /*
   * Testcase 3
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = -1
   * Byte Raster Layer
   *
   */
  QTest::newRow( "testcase 3" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::Byte
    << "EPSG:4326"
    << 1.0
    << 256.0 //fails --> value too big for byte
    << 0;

  /*
   * Testcase 4
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * Int16 Raster Layer
   *
   */
  QTest::newRow( "testcase 4" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << u"/createConstantRaster_testcase4.tif"_s
    << Qgis::DataType::Int16
    << "EPSG:4326"
    << 1.0
    << 12.0
    << 1;

  /*
   * Testcase 5
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * Int16 Raster Layer
   *
   */
  QTest::newRow( "testcase 5" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::Int16
    << "EPSG:4326"
    << 1.0
    << -32769.0
    << 1;

  /*
   * Testcase 6
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * Int16 Raster Layer
   *
   */
  QTest::newRow( "testcase 6" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::Int16
    << "EPSG:4326"
    << 1.0
    << 32769.0
    << 1;

  /*
   * Testcase 7
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * UInt16 Raster Layer
   *
   */
  QTest::newRow( "testcase 7" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << u"/createConstantRaster_testcase7.tif"_s
    << Qgis::DataType::UInt16
    << "EPSG:4326"
    << 1.0
    << 12.0
    << 2;

  /*
   * Testcase 8
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * UInt16 Raster Layer
   *
   */
  QTest::newRow( "testcase 8" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::UInt16
    << "EPSG:4326"
    << 1.0
    << -1.0
    << 2;

  /*
   * Testcase 9
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * UInt16 Raster Layer
   *
   */
  QTest::newRow( "testcase 9" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::UInt16
    << "EPSG:4326"
    << 1.0
    << 65536.0
    << 2;

  /*
   * Testcase 10
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * UInt16 Raster Layer
   *
   */
  QTest::newRow( "testcase 10" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << u"/createConstantRaster_testcase10.tif"_s
    << Qgis::DataType::Int32
    << "EPSG:4326"
    << 1.0
    << 12.0
    << 3;

  /*
   * Testcase 10
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12
   * Int32 Raster Layer
   *
   */
  QTest::newRow( "testcase 10" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << u"/createConstantRaster_testcase10.tif"_s
    << Qgis::DataType::Int32
    << "EPSG:4326"
    << 1.0
    << 12.0
    << 3;

  /*
   * Testcase 11
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = -2147483649.0
   * Int32 Raster Layer
   *
   */
  QTest::newRow( "testcase 11" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::Int32
    << "EPSG:4326"
    << 1.0
    << -2147483649.0
    << 3;

  /*
   * Testcase 12
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 2147483649.0
   * Int32 Raster Layer
   *
   */
  QTest::newRow( "testcase 12" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::Int32
    << "EPSG:4326"
    << 1.0
    << 2147483649.0
    << 3;

  /*
   * Testcase 13
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12.0
   * UInt32 Raster Layer
   *
   */
  QTest::newRow( "testcase 13" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << u"/createConstantRaster_testcase13.tif"_s
    << Qgis::DataType::UInt32
    << "EPSG:4326"
    << 1.0
    << 12.0
    << 4;

  /*
   * Testcase 14
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 4294967296.0
   * UInt32 Raster Layer
   *
   */
  QTest::newRow( "testcase 14" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::UInt32
    << "EPSG:4326"
    << 1.0
    << 4294967296.0
    << 4;

  /*
   * Testcase 15
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = -1.0
   * UInt32 Raster Layer
   *
   */
  QTest::newRow( "testcase 14" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << QString()
    << Qgis::DataType::UInt32
    << "EPSG:4326"
    << 1.0
    << -1.0
    << 4;

  /*
   * Testcase 16
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12.0
   * Float32 Raster Layer
   *
   */
  QTest::newRow( "testcase 16" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << u"/createConstantRaster_testcase16.tif"_s
    << Qgis::DataType::Float32
    << "EPSG:4326"
    << 1.0
    << 12.12
    << 5;

  /*
   * Testcase 17
   *
   * inputExtent = from "/raster/band1_int16_noct_epsg4326.tif"
   * crs = EPSG:4326
   * pixelSize = 1.0
   * constantValue = 12.0
   * Float64 Raster Layer
   *
   */
  QTest::newRow( "testcase 17" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << u"/createConstantRaster_testcase17.tif"_s
    << Qgis::DataType::Float64
    << "EPSG:4326"
    << 1.0
    << 12.125789212532487
    << 6;
}

void TestQgsProcessingAlgsPt1::createConstantRaster()
{
  QFETCH( QString, inputExtent );
  QFETCH( QString, expectedRaster );
  QFETCH( Qgis::DataType, expectedDataType );
  QFETCH( QString, crs );
  QFETCH( double, pixelSize );
  QFETCH( double, constantValue );
  QFETCH( int, typeId );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:createconstantrasterlayer"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  //set project crs and ellipsoid from input layer
  p.setCrs( QgsCoordinateReferenceSystem( crs ), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"EXTENT"_s, inputExtent );
  parameters.insert( u"TARGET_CRS"_s, QgsCoordinateReferenceSystem( crs ) );
  parameters.insert( u"PIXEL_SIZE"_s, pixelSize );
  parameters.insert( u"NUMBER"_s, constantValue );
  parameters.insert( u"OUTPUT_TYPE"_s, typeId );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  if ( expectedRaster.isEmpty() )
  {
    //verify if user feedback for unacceptable values are thrown
    alg->run( parameters, *context, &feedback, &ok );
    QVERIFY( !ok );
  }
  else
  {
    //prepare expectedRaster
    auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images/expected_constantRaster" + expectedRaster, "expectedDataset", "gdal" );
    std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
    QgsRasterIterator expectedIter( expectedInterface.get() );
    expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

    //run alg...
    results = alg->run( parameters, *context, &feedback, &ok );
    QVERIFY( ok );

    //...and check results with expected datasets
    auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
    std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

    QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
    QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );
    QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );

    QgsRasterIterator outputIter( outputInterface.get() );
    outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
    int outputIterLeft = 0;
    int outputIterTop = 0;
    int outputIterCols = 0;
    int outputIterRows = 0;
    int expectedIterLeft = 0;
    int expectedIterTop = 0;
    int expectedIterCols = 0;
    int expectedIterRows = 0;

    std::unique_ptr<QgsRasterBlock> outputRasterBlock;
    std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

    while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
    {
      for ( int row = 0; row < expectedIterRows; row++ )
      {
        for ( int column = 0; column < expectedIterCols; column++ )
        {
          const double expectedValue = expectedRasterBlock->value( row, column );
          const double outputValue = outputRasterBlock->value( row, column );
          QCOMPARE( outputValue, expectedValue );
        }
      }
    }
  }
}

void TestQgsProcessingAlgsPt1::rasterRank_data()
{
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<QString>( "ranks" );
  QTest::addColumn<int>( "nodataHandling" );

  /*
   * Testcase 1
   */
  QTest::newRow( "testcase 1" )
    << u"/rasterRank_testcase1.tif"_s
    << QString::number( 2 )
    << 0;

  /*
   * Testcase 2
   */
  QTest::newRow( "testcase 2" )
    << u"/rasterRank_testcase2.tif"_s
    << QString::number( -2 )
    << 0;

  /*
   * Testcase 3
   */
  QTest::newRow( "testcase 3" )
    << u"/rasterRank_testcase3.tif"_s
    << QString::number( 2 )
    << 1;

  /*
   * Testcase 4
   */
  QTest::newRow( "testcase 4" )
    << u"/rasterRank_testcase4.tif"_s
    << u"2,-2"_s
    << 0;
}

void TestQgsProcessingAlgsPt1::rasterRank()
{
  QFETCH( QString, expectedRaster );
  QFETCH( QString, ranks );
  QFETCH( int, nodataHandling );

  //prepare input params
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:rasterrank"_s ) );

  const QString testDataPath( TEST_DATA_DIR ); //defined in CMakeLists.txt

  QVariantMap parameters;

  parameters.insert( u"INPUT_RASTERS"_s, QStringList() << testDataPath + "/raster/rank1.tif" << testDataPath + "/raster/rank2.tif" << testDataPath + "/raster/rank3.tif" << testDataPath + "/raster/rank4.tif" );
  parameters.insert( u"RANKS"_s, ranks );
  parameters.insert( u"NODATA_HANDLING"_s, nodataHandling );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  //prepare expected raster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( testDataPath + "/control_images/expected_rasterRank" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run algorithm...
  auto context = std::make_unique<QgsProcessingContext>();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double expectedValue = expectedRasterBlock->value( row, column );
        const double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );
      }
    }
  }
}


void TestQgsProcessingAlgsPt1::densifyGeometries_data()
{
  QTest::addColumn<QgsGeometry>( "sourceGeometry" );
  QTest::addColumn<QgsGeometry>( "expectedGeometry" );
  QTest::addColumn<double>( "interval" );
  QTest::addColumn<QString>( "geometryType" );

  QTest::newRow( "Null geometry" )
    << QgsGeometry()
    << QgsGeometry()
    << 0.1
    << "Point";

  QTest::newRow( "PointZ" )
    << QgsGeometry::fromWkt( "PointZ( 1 2 3 )" )
    << QgsGeometry::fromWkt( "PointZ( 1 2 3 )" )
    << 0.1
    << "Point";

  QTest::newRow( "MultiPoint" )
    << QgsGeometry::fromWkt( "MULTIPOINT ((155 271), (150 360), (260 360), (271 265), (280 260), (270 370), (154 354), (150 260))" )
    << QgsGeometry::fromWkt( "MULTIPOINT ((155 271), (150 360), (260 360), (271 265), (280 260), (270 370), (154 354), (150 260))" )
    << 0.1
    << "Point";

  QTest::newRow( "LineString big distance" )
    << QgsGeometry::fromWkt( "LineString( 0 0, 10 0, 10 10 )" )
    << QgsGeometry::fromWkt( "LineString( 0 0, 10 0, 10 10 )" )
    << 100.
    << "LineString";

  QTest::newRow( "LineString small distance" )
    << QgsGeometry::fromWkt( "LineString( 0 0, 10 0, 10 10 )" )
    << QgsGeometry::fromWkt( "LineString (0 0, 2.5 0, 5 0, 7.5 0, 10 0, 10 2.5, 10 5, 10 7.5, 10 10)" )
    << 3.
    << "LineString";

  QTest::newRow( "LineStringZ" )
    << QgsGeometry::fromWkt( "LineStringZ( 0 0 1, 10 0 2, 10 10 0)" )
    << QgsGeometry::fromWkt( "LineStringZ (0 0 1, 5 0 1.5, 10 0 2, 10 5 1, 10 10 0)" )
    << 6.
    << "LineString";

  QTest::newRow( "LineStringM" )
    << QgsGeometry::fromWkt( "LineStringM( 0 0 0, 10 0 2, 10 10 0)" )
    << QgsGeometry::fromWkt( "LineStringM (0 0 0, 2.5 0 0.5, 5 0 1, 7.5 0 1.5, 10 0 2, 10 2.5 1.5, 10 5 1, 10 7.5 0.5, 10 10 0)" )
    << 3.
    << "LineString";

  QTest::newRow( "LineStringZM" )
    << QgsGeometry::fromWkt( "LineStringZM( 0 0 1 10, 10 0 2 8, 10 10 0 4)" )
    << QgsGeometry::fromWkt( "LineStringZM (0 0 1 10, 5 0 1.5 9, 10 0 2 8, 10 5 1 6, 10 10 0 4)" )
    << 6.
    << "LineString";

  QTest::newRow( "Polygon" )
    << QgsGeometry::fromWkt( "Polygon(( 0 0, 20 0, 20 20, 0 0 ))" )
    << QgsGeometry::fromWkt( "Polygon ((0 0, 5 0, 10 0, 15 0, 20 0, 20 5, 20 10, 20 15, 20 20, 16 16, 12 12, 7.99999999999999822 7.99999999999999822, 4 4, 0 0))" )
    << 6.
    << "Polygon";
}

void TestQgsProcessingAlgsPt1::densifyGeometries()
{
  QFETCH( QgsGeometry, sourceGeometry );
  QFETCH( QgsGeometry, expectedGeometry );
  QFETCH( double, interval );
  QFETCH( QString, geometryType );

  const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> alg( featureBasedAlg( "native:densifygeometriesgivenaninterval" ) );

  QVariantMap parameters;
  parameters.insert( u"INTERVAL"_s, interval );

  QgsFeature feature;
  feature.setGeometry( sourceGeometry );

  const QgsFeature result = runForFeature( alg, feature, geometryType, parameters );

  if ( expectedGeometry.isNull() )
    QVERIFY( result.geometry().isNull() );
  else
    QVERIFY2( result.geometry().equals( expectedGeometry ), u"Result: %1, Expected: %2"_s.arg( result.geometry().asWkt(), expectedGeometry.asWkt() ).toUtf8().constData() );
}

void TestQgsProcessingAlgsPt1::fillNoData_data()
{
  QTest::addColumn<QString>( "inputRaster" );
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<int>( "inputBand" );
  QTest::addColumn<double>( "fillValue" );

  /*
   * Testcase 1
   *
   * NoData raster layer
   * band = 1
   * fillValue = 2.0
   */
  QTest::newRow( "testcase 1" )
    << "/raster/band1_int16_noct_epsg4326.tif"
    << u"/fillnodata_testcase1.tif"_s
    << 1
    << 2.0;

  /*
   * Testcase 2
   *
   * WGS84 data without weights
   * searchRadius = 3
   * pixelSize = 1.8
   */
  QTest::newRow( "testcase 2" )
    << "/raster/band1_int16_noct_epsg4326.tif"
    << u"/fillnodata_testcase2.tif"_s
    << 1
    << 1.8;

  /*
   * Testcase 3
   *
   * WGS84 data without weights
   * searchRadius = 3
   * pixelSize = 1.8
   */
  QTest::newRow( "testcase 3" )
    << "/raster/band1_float32_noct_epsg4326.tif"
    << u"/fillnodata_testcase3.tif"_s
    << 1
    << 1.8;
}

void TestQgsProcessingAlgsPt1::fillNoData()
{
  QFETCH( QString, inputRaster );
  QFETCH( QString, expectedRaster );
  QFETCH( int, inputBand );
  QFETCH( double, fillValue );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:fillnodata"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  auto inputRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + inputRaster, "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, QString( myDataPath + inputRaster ) );
  parameters.insert( u"BAND"_s, inputBand );
  parameters.insert( u"FILL_VALUE"_s, fillValue );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images/fillNoData/" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double expectedValue = expectedRasterBlock->value( row, column );
        const double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );
      }
    }
  }
}

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 13, 0 )
void TestQgsProcessingAlgsPt1::rasterCOGOutput()
{
  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:fillnodata"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  auto inputRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + u"/raster/band1_int16_noct_epsg4326.tif"_s, u"inputDataset"_s, u"gdal"_s );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, QString( myDataPath + u"/raster/band1_int16_noct_epsg4326.tif"_s ) );
  parameters.insert( u"BAND"_s, 1 );
  parameters.insert( u"FILL_VALUE"_s, 1 );

  QgsProcessingOutputLayerDefinition outputLayerDef( QgsProcessing::TEMPORARY_OUTPUT );
  outputLayerDef.setFormat( u"COG"_s );
  parameters.insert( u"OUTPUT"_s, outputLayerDef );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const QString outputFilename = results.value( u"OUTPUT"_s ).toString();

  gdal::dataset_unique_ptr dataset( GDALOpen( outputFilename.toStdString().c_str(), GA_ReadOnly ) );
  QVERIFY( dataset );

  const char *pszLayout = GDALGetMetadataItem( dataset.get(), "LAYOUT", "IMAGE_STRUCTURE" );
  QVERIFY( pszLayout );

  QCOMPARE( QString( pszLayout ), u"COG"_s );
}
#endif

void TestQgsProcessingAlgsPt1::lineDensity_data()
{
  QTest::addColumn<QString>( "inputDataset" );
  QTest::addColumn<QString>( "expectedDataset" );
  QTest::addColumn<double>( "searchRadius" );
  QTest::addColumn<double>( "pixelSize" );
  QTest::addColumn<QString>( "weightField" );

  /*
   * Testcase 1
   *
   * WGS84 data with weights
   * searchRadius = 3
   * pixelSize = 2
   */
  QTest::newRow( "testcase 1" )
    << "/linedensity.gml"
    << u"/linedensity_testcase1.tif"_s
    << 3.0
    << 2.0
    << u"weight"_s;

  /*
   * Testcase 2
   *
   * WGS84 data without weights
   * searchRadius = 3
   * pixelSize = 2
   */
  QTest::newRow( "testcase_2" )
    << "/linedensity.gml"
    << u"/linedensity_testcase2.tif"_s
    << 3.0
    << 2.0
    << QString();
}

void TestQgsProcessingAlgsPt1::lineDensity()
{
  QFETCH( QString, inputDataset );
  QFETCH( QString, expectedDataset );
  QFETCH( double, searchRadius );
  QFETCH( double, pixelSize );
  QFETCH( QString, weightField );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:linedensity"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QgsVectorLayer *layer = new QgsVectorLayer( myDataPath + inputDataset + "|layername=linedensity", u"layer"_s, u"ogr"_s );
  p.addMapLayer( layer );
  QVERIFY( layer->isValid() );

  //set project crs and ellipsoid from input layer
  p.setCrs( layer->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer ) );
  parameters.insert( u"WEIGHT"_s, weightField );
  parameters.insert( u"RADIUS"_s, searchRadius );
  parameters.insert( u"PIXEL_SIZE"_s, pixelSize );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRaster = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images/expected_raster_linedensity" + expectedDataset, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRaster->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRaster->width(), expectedRaster->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputRaster->width(), expectedRaster->width() );
  QCOMPARE( outputRaster->height(), expectedRaster->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double expectedValue = expectedRasterBlock->value( row, column );
        const double outputValue = outputRasterBlock->value( row, column );
        QGSCOMPARENEAR( outputValue, expectedValue, 0.0000000002 );
      }
    }
  }
}

void TestQgsProcessingAlgsPt1::rasterLogicOp_data()
{
  QTest::addColumn<QVector<double>>( "input1" );
  QTest::addColumn<QVector<double>>( "input2" );
  QTest::addColumn<QVector<double>>( "input3" );
  QTest::addColumn<bool>( "treatNodataAsFalse" );
  QTest::addColumn<qgssize>( "expectedOrNoDataCount" );
  QTest::addColumn<qgssize>( "expectedOrTrueCount" );
  QTest::addColumn<qgssize>( "expectedOrFalseCount" );
  QTest::addColumn<QVector<double>>( "expectedOr" );
  QTest::addColumn<qgssize>( "expectedAndNoDataCount" );
  QTest::addColumn<qgssize>( "expectedAndTrueCount" );
  QTest::addColumn<qgssize>( "expectedAndFalseCount" );
  QTest::addColumn<QVector<double>>( "expectedAnd" );
  QTest::addColumn<int>( "nRows" );
  QTest::addColumn<int>( "nCols" );
  QTest::addColumn<double>( "destNoDataValue" );
  QTest::addColumn<int>( "dataType" );

  QTest::newRow( "no nodata" ) << QVector<double> { 1, 2, 0, 0, 0, 0 }
                               << QVector<double> { 1, 0, 1, 1, 0, 1 }
                               << QVector<double> { 1, 2, 0, 0, 0, -1 }
                               << false
                               << 0ULL << 5ULL << 1ULL
                               << QVector<double> { 1, 1, 1, 1, 0, 1 }
                               << 0ULL << 1ULL << 5ULL
                               << QVector<double> { 1, 0, 0, 0, 0, 0 }
                               << 3 << 2
                               << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
  QTest::newRow( "nodata" ) << QVector<double> { 1, -9999, 0, 0, 0, 0 }
                            << QVector<double> { 1, 0, 1, 1, 0, 1 }
                            << QVector<double> { 1, 2, 0, -9999, 0, -1 }
                            << false
                            << 2ULL << 3ULL << 1ULL
                            << QVector<double> { 1, -9999, 1, -9999, 0, 1 }
                            << 2ULL << 1ULL << 3ULL
                            << QVector<double> { 1, -9999, 0, -9999, 0, 0 }
                            << 3 << 2
                            << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
  QTest::newRow( "nodata as false" ) << QVector<double> { 1, -9999, 0, 0, 0, 0 }
                                     << QVector<double> { 1, 0, 1, 1, 0, 1 }
                                     << QVector<double> { 1, 2, 0, -9999, 0, -1 }
                                     << true
                                     << 0ULL << 5ULL << 1ULL
                                     << QVector<double> { 1, 1, 1, 1, 0, 1 }
                                     << 0ULL << 1ULL << 5ULL
                                     << QVector<double> { 1, 0, 0, 0, 0, 0 }
                                     << 3 << 2
                                     << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
  QTest::newRow( "missing block 1" ) << QVector<double> {}
                                     << QVector<double> { 1, 0, 1, 1, 0, 1 }
                                     << QVector<double> { 1, 2, 0, -9999, 0, -1 }
                                     << false
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector<double> { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector<double> { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 3 << 2
                                     << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
  QTest::newRow( "missing block 1 nodata as false" ) << QVector<double> {}
                                                     << QVector<double> { 1, 0, 1, 1, 0, 1 }
                                                     << QVector<double> { 1, 2, 0, -9999, 0, -1 }
                                                     << true
                                                     << 0ULL << 5ULL << 1ULL
                                                     << QVector<double> { 1, 1, 1, 1, 0, 1 }
                                                     << 0ULL << 0ULL << 6ULL
                                                     << QVector<double> { 0, 0, 0, 0, 0, 0 }
                                                     << 3 << 2
                                                     << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
  QTest::newRow( "missing block 2" ) << QVector<double> { 1, 0, 1, 1, 0, 1 }
                                     << QVector<double> {}
                                     << QVector<double> { 1, 2, 0, -9999, 0, -1 }
                                     << false
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector<double> { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector<double> { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 3 << 2
                                     << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
  QTest::newRow( "missing block 2 nodata as false" ) << QVector<double> { 1, 0, 1, 1, 0, 1 }
                                                     << QVector<double> {}
                                                     << QVector<double> { 1, 2, 0, -9999, 0, -1 }
                                                     << true
                                                     << 0ULL << 5ULL << 1ULL
                                                     << QVector<double> { 1, 1, 1, 1, 0, 1 }
                                                     << 0ULL << 0ULL << 6ULL
                                                     << QVector<double> { 0, 0, 0, 0, 0, 0 }
                                                     << 3 << 2
                                                     << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
  QTest::newRow( "missing block 3" ) << QVector<double> { 1, 0, 1, 1, 0, 1 }
                                     << QVector<double> { 1, 2, 0, -9999, 0, -1 }
                                     << QVector<double> {}
                                     << false
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector<double> { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector<double> { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 3 << 2
                                     << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
  QTest::newRow( "missing block 3 nodata as false" ) << QVector<double> { 1, 0, 1, 1, 0, 1 }
                                                     << QVector<double> { 1, 2, 0, -9999, 0, -1 }
                                                     << QVector<double> {}
                                                     << true
                                                     << 0ULL << 5ULL << 1ULL
                                                     << QVector<double> { 1, 1, 1, 1, 0, 1 }
                                                     << 0ULL << 0ULL << 6ULL
                                                     << QVector<double> { 0, 0, 0, 0, 0, 0 }
                                                     << 3 << 2
                                                     << -9999.0 << static_cast<int>( Qgis::DataType::Float32 );
}

void TestQgsProcessingAlgsPt1::rasterLogicOp()
{
  QFETCH( QVector<double>, input1 );
  QFETCH( QVector<double>, input2 );
  QFETCH( QVector<double>, input3 );
  QVector<QVector<double>> input;
  input << input1 << input2 << input3;
  QFETCH( bool, treatNodataAsFalse );
  QFETCH( qgssize, expectedOrNoDataCount );
  QFETCH( qgssize, expectedOrTrueCount );
  QFETCH( qgssize, expectedOrFalseCount );
  QFETCH( QVector<double>, expectedOr );
  QFETCH( qgssize, expectedAndNoDataCount );
  QFETCH( qgssize, expectedAndTrueCount );
  QFETCH( qgssize, expectedAndFalseCount );
  QFETCH( QVector<double>, expectedAnd );
  QFETCH( int, nRows );
  QFETCH( int, nCols );
  QFETCH( double, destNoDataValue );
  QFETCH( int, dataType );

  QgsRasterLogicalOrAlgorithm orAlg;
  QgsRasterLogicalAndAlgorithm andAlg;

  const QgsRectangle extent = QgsRectangle( 0, 0, nRows, nCols );
  const QgsRectangle badExtent = QgsRectangle( -100, -100, 90, 90 );
  const QgsCoordinateReferenceSystem crs( u"EPSG:3857"_s );
  double tform[] = {
    extent.xMinimum(), extent.width() / nCols, 0.0,
    extent.yMaximum(), 0.0, -extent.height() / nRows
  };

  std::vector<QgsRasterAnalysisUtils::RasterLogicInput> inputs;
  for ( int ii = 0; ii < 3; ++ii )
  {
    // generate unique filename (need to open the file first to generate it)
    QTemporaryFile tmpFile;
    QVERIFY( tmpFile.open() );
    tmpFile.close();

    // create a GeoTIFF - this will create data provider in editable mode
    const QString filename = tmpFile.fileName();

    auto writer = std::make_unique<QgsRasterFileWriter>( filename );
    writer->setOutputProviderKey( u"gdal"_s );
    writer->setOutputFormat( u"GTiff"_s );
    std::unique_ptr<QgsRasterDataProvider> dp( writer->createOneBandRaster( Qgis::DataType::Float32, nCols, nRows, input[ii].empty() ? badExtent : extent, crs ) );
    QVERIFY( dp->isValid() );
    dp->setNoDataValue( 1, -9999 );
    std::unique_ptr<QgsRasterBlock> block( dp->block( 1, input[ii].empty() ? badExtent : extent, nCols, nRows ) );
    if ( !dp->isEditable() )
    {
      QVERIFY( dp->setEditable( true ) );
    }
    int i = 0;
    for ( int row = 0; row < nRows; row++ )
    {
      for ( int col = 0; col < nCols; col++ )
      {
        if ( !input[ii].empty() )
          block->setValue( row, col, input[ii][i++] );
      }
    }
    QVERIFY( dp->writeBlock( block.get(), 1 ) );
    QVERIFY( dp->setEditable( false ) );

    QgsRasterAnalysisUtils::RasterLogicInput input;
    input.sourceDataProvider = std::move( dp );
    input.hasNoDataValue = true;
    input.interface = input.sourceDataProvider.get();

    inputs.emplace_back( std::move( input ) );
  }

  // make destination OR raster
  QTemporaryFile tmpFile2;
  QVERIFY( tmpFile2.open() );
  tmpFile2.close();

  // create a GeoTIFF - this will create data provider in editable mode
  QString filename = tmpFile2.fileName();
  std::unique_ptr<QgsRasterDataProvider> dpOr( QgsRasterDataProvider::create( u"gdal"_s, filename, u"GTiff"_s, 1, static_cast<Qgis::DataType>( dataType ), 10, 10, tform, crs ) );
  QVERIFY( dpOr->isValid() );

  // make destination AND raster
  QTemporaryFile tmpFile3;
  QVERIFY( tmpFile3.open() );
  tmpFile3.close();

  // create a GeoTIFF - this will create data provider in editable mode
  filename = tmpFile3.fileName();
  std::unique_ptr<QgsRasterDataProvider> dpAnd( QgsRasterDataProvider::create( u"gdal"_s, filename, u"GTiff"_s, 1, static_cast<Qgis::DataType>( dataType ), 10, 10, tform, crs ) );
  QVERIFY( dpAnd->isValid() );

  QgsFeedback feedback;
  qgssize noDataCount = 0;
  qgssize trueCount = 0;
  qgssize falseCount = 0;
  QgsRasterAnalysisUtils::applyRasterLogicOperator( inputs, dpOr.get(), destNoDataValue, treatNodataAsFalse, nCols, nRows, extent, &feedback, orAlg.mExtractValFunc, noDataCount, trueCount, falseCount );

  QCOMPARE( noDataCount, expectedOrNoDataCount );
  QCOMPARE( trueCount, expectedOrTrueCount );
  QCOMPARE( falseCount, expectedOrFalseCount );

  // read back in values
  std::unique_ptr<QgsRasterBlock> block( dpOr->block( 1, extent, nCols, nRows ) );
  QVector<double> res( nCols * nRows );
  int i = 0;
  for ( int row = 0; row < nRows; row++ )
  {
    for ( int col = 0; col < nCols; col++ )
    {
      res[i++] = block->value( row, col );
    }
  }

  for ( int row = 0; row < nRows; row++ )
  {
    for ( int col = 0; col < nCols; col++ )
    {
      QCOMPARE( res[row * nCols + col], expectedOr[row * nCols + col] );
    }
  }

  noDataCount = 0;
  trueCount = 0;
  falseCount = 0;
  QgsRasterAnalysisUtils::applyRasterLogicOperator( inputs, dpAnd.get(), destNoDataValue, treatNodataAsFalse, nCols, nRows, extent, &feedback, andAlg.mExtractValFunc, noDataCount, trueCount, falseCount );

  QCOMPARE( noDataCount, expectedAndNoDataCount );
  QCOMPARE( trueCount, expectedAndTrueCount );
  QCOMPARE( falseCount, expectedAndFalseCount );

  // read back in values
  block.reset( dpAnd->block( 1, extent, nCols, nRows ) );
  QVector<double> res2( nCols * nRows );
  i = 0;
  for ( int row = 0; row < nRows; row++ )
  {
    for ( int col = 0; col < nCols; col++ )
    {
      res2[i++] = block->value( row, col );
    }
  }

  for ( int row = 0; row < nRows; row++ )
  {
    for ( int col = 0; col < nCols; col++ )
    {
      QCOMPARE( res2[row * nCols + col], expectedAnd[row * nCols + col] );
    }
  }
}

void TestQgsProcessingAlgsPt1::cellStatistics_data()
{
  QTest::addColumn<QStringList>( "inputRasters" );
  QTest::addColumn<QString>( "referenceLayer" );
  QTest::addColumn<int>( "statistic" );
  QTest::addColumn<bool>( "ignoreNoData" );
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );

  /*
   * Testcase 1: sum
   */
  QTest::newRow( "testcase_1" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 0
    << false
    << u"/cellstatistics_sum_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 2: count
   */
  QTest::newRow( "testcase_2" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 1
    << false
    << u"/cellstatistics_count_result.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 3: mean
   */
  QTest::newRow( "testcase_3" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 2
    << false
    << u"/cellstatistics_mean_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 4: median
   */
  QTest::newRow( "testcase_4" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 3
    << false
    << u"/cellstatistics_median_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 5: Standard deviation
   */
  QTest::newRow( "testcase_5" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 4
    << false
    << u"/cellstatistics_stddev_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 6: Variance
   */
  QTest::newRow( "testcase_6" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 5
    << false
    << u"/cellstatistics_variance_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 7: Minimum
   */
  QTest::newRow( "testcase_7" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 6
    << false
    << u"/cellstatistics_min_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 8: Maximum
   */
  QTest::newRow( "testcase_8" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 7
    << false
    << u"/cellstatistics_max_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 9: Minority
   */
  QTest::newRow( "testcase_9" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 8
    << false
    << u"/cellstatistics_minority_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 10: Majority
   */
  QTest::newRow( "testcase_10" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 9
    << false
    << u"/cellstatistics_majority_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 11: Range
   */
  QTest::newRow( "testcase_11" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 10
    << false
    << u"/cellstatistics_range_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 12: Variety
   */
  QTest::newRow( "testcase_12" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 11
    << false
    << u"/cellstatistics_variety_result.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 13: Sum (integer)
   */
  QTest::newRow( "testcase_13" )
    << QStringList( { "/raster/statisticsRas1_int32.tif", "/raster/statisticsRas2_int32.tif", "/raster/statisticsRas3_int32.tif" } )
    << u"/raster/statisticsRas1_int32.tif"_s
    << 0
    << false
    << u"/cellstatistics_sum_result_int32.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 14: sum (ignore nodata)
   */
  QTest::newRow( "testcase_14" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 0
    << true
    << u"/cellstatistics_sum_ignore_nodata_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 15: mean
   */
  QTest::newRow( "testcase_15" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 2
    << true
    << u"/cellstatistics_mean_ignore_nodata_result.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 16: Sum (integer)
   */
  QTest::newRow( "testcase_16" )
    << QStringList( { "/raster/statisticsRas1_int32.tif", "/raster/statisticsRas2_int32.tif", "/raster/statisticsRas3_int32.tif" } )
    << u"/raster/statisticsRas1_int32.tif"_s
    << 5
    << false
    << u"/cellstatistics_variance_result_float32.tif"_s
    << Qgis::DataType::Float32;

  /*
   * Testcase 17: median with even number of layers
   */
  QTest::newRow( "testcase_17" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 3
    << false
    << u"/cellstatistics_median_result_fourLayers.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 18: median with even number of layers and integer inputs
   */
  QTest::newRow( "testcase_18" )
    << QStringList( { "/raster/statisticsRas1_int32.tif", "/raster/statisticsRas1_int32.tif", "/raster/statisticsRas2_int32.tif", "/raster/statisticsRas3_int32.tif" } )
    << u"/raster/statisticsRas1_int32.tif"_s
    << 3
    << false
    << u"/cellstatistics_median_result_fourLayers_float32.tif"_s
    << Qgis::DataType::Float32;

  /*
   * Testcase 19: sum with raster cell stacks containing only nodata
   */
  QTest::newRow( "testcase_19" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas1_float64.asc" } )
    << u"/raster/statisticsRas3_int32.tif"_s
    << 0
    << true
    << u"/cellstatistics_sum_result_ignoreNoData.tif"_s
    << Qgis::DataType::Float64;
}

void TestQgsProcessingAlgsPt1::cellStatistics()
{
  QFETCH( QStringList, inputRasters );
  QFETCH( QString, referenceLayer );
  QFETCH( int, statistic );
  QFETCH( bool, ignoreNoData );
  QFETCH( QString, expectedRaster );
  QFETCH( Qgis::DataType, expectedDataType );


  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:cellstatistics"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QStringList inputDatasetPaths;

  for ( const auto &raster : inputRasters )
  {
    inputDatasetPaths << myDataPath + raster;
  }

  auto inputRasterLayer1 = std::make_unique<QgsRasterLayer>( myDataPath + inputRasters[0], "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer1->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, inputDatasetPaths );
  parameters.insert( u"STATISTIC"_s, statistic );
  parameters.insert( u"IGNORE_NODATA"_s, ignoreNoData );
  parameters.insert( u"REFERENCE_LAYER"_s, QString( myDataPath + referenceLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images/expected_cellStatistics/" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double expectedValue = expectedRasterBlock->value( row, column );
        const double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );
      }
    }
  }
}

Q_DECLARE_METATYPE( QgsRasterAnalysisUtils::CellValuePercentileMethods )
void TestQgsProcessingAlgsPt1::percentileFunctions_data()
{
  QTest::addColumn<QgsRasterAnalysisUtils::CellValuePercentileMethods>( "function" );
  QTest::addColumn<std::vector<double>>( "inputValues" );
  QTest::addColumn<std::vector<double>>( "inputPercentiles" );
  QTest::addColumn<std::vector<double>>( "expectedValues" );

  QTest::newRow( "testcase_1" )
    << QgsRasterAnalysisUtils::NearestRankPercentile
    << std::vector<double>( { 100, 24, 49, 36, 2, 18, 98, 64, 20, 20 } )
    << std::vector<double>( { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 } )
    << std::vector<double>( { 2, 2, 18, 20, 20, 24, 36, 49, 64, 98, 100 } );

  QTest::newRow( "testcase_2" )
    << QgsRasterAnalysisUtils::InterpolatedPercentileInc
    << std::vector<double>( { 100, 24, 49, 36, 2, 18, 98, 64, 20, 20 } )
    << std::vector<double>( { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 } )
    << std::vector<double>( { 2.0, 16.4, 19.6, 20.0, 22.4, 30.0, 41.2, 53.5, 70.8, 98.2, 100 } );

  QTest::newRow( "testcase_3" )
    << QgsRasterAnalysisUtils::InterpolatedPercentileExc
    << std::vector<double>( { 100, 24, 49, 36, 2, 18, 98, 64, 20, 20 } )
    << std::vector<double>( { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 } )
    << std::vector<double>( { -9999, 3.6, 18.4, 20, 21.6, 30, 43.8, 59.5, 91.2, 99.8, -9999 } );
}

void TestQgsProcessingAlgsPt1::percentileFunctions()
{
  QFETCH( QgsRasterAnalysisUtils::CellValuePercentileMethods, function );
  QFETCH( std::vector<double>, inputValues );
  QFETCH( std::vector<double>, inputPercentiles );
  QFETCH( std::vector<double>, expectedValues );

  const int inputValuesSize = static_cast<int>( inputValues.size() );
  const int percentileSize = static_cast<int>( inputPercentiles.size() );
  double result;

  for ( int i = 0; i < percentileSize; i++ )
  {
    const double percentile = inputPercentiles[i];
    const double expectedValue = expectedValues[i];

    switch ( function )
    {
      case ( QgsRasterAnalysisUtils::NearestRankPercentile ):
      {
        result = QgsRasterAnalysisUtils::nearestRankPercentile( inputValues, inputValuesSize, percentile );
        QCOMPARE( result, expectedValue );
        break;
      }
      case ( QgsRasterAnalysisUtils::InterpolatedPercentileInc ):
      {
        result = QgsRasterAnalysisUtils::interpolatedPercentileInc( inputValues, inputValuesSize, percentile );
        QCOMPARE( result, expectedValue );
        break;
      }
      case ( QgsRasterAnalysisUtils::InterpolatedPercentileExc ):
      {
        result = QgsRasterAnalysisUtils::interpolatedPercentileExc( inputValues, inputValuesSize, percentile, -9999 );
        QCOMPARE( result, expectedValue );
        break;
      }
    }
  }
}

void TestQgsProcessingAlgsPt1::percentileRaster_data()
{
  QTest::addColumn<QStringList>( "inputRasters" );
  QTest::addColumn<QString>( "referenceLayer" );
  QTest::addColumn<int>( "method" );
  QTest::addColumn<double>( "percentile" );
  QTest::addColumn<bool>( "ignoreNoData" );
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );

  /*
   * Testcase 1: nearest, ignoreNoData = true, dataType = Float64
   */
  QTest::newRow( "testcase_1" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 0
    << 0.789
    << true
    << u"/percentile_nearest_ignoreTrue_float64.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 2: inc, ignoreNoData = true, dataType = Float64
   */
  QTest::newRow( "testcase_2" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 1
    << 0.789
    << true
    << u"/percentile_inc_ignoreTrue_float64.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 3: exc, ignoreNoData = true, dataType = Float64
   */
  QTest::newRow( "testcase_3" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 2
    << 0.789
    << true
    << u"/percentile_exc_ignoreTrue_float64.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 4: nearest, ignoreNoData = false, dataType = Float64
   */
  QTest::newRow( "testcase_4" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 0
    << 0.789
    << false
    << u"/percentile_nearest_ignoreFalse_float64.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 5: inc, ignoreNoData = false, dataType = Float64
   */
  QTest::newRow( "testcase_5" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 1
    << 0.789
    << false
    << u"/percentile_inc_ignoreFalse_float64.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 6: exc, ignoreNoData = false, dataType = Float64
   */
  QTest::newRow( "testcase_6" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 2
    << 0.789
    << false
    << u"/percentile_exc_ignoreFalse_float64.tif"_s
    << Qgis::DataType::Float64;

  /*
   * Testcase 7: exc, ignoreNoData = false, dataType = Byte
   */
  QTest::newRow( "testcase_7" )
    << QStringList( { "/raster/rnd_percentile_raster1_byte.tif", "/raster/rnd_percentile_raster2_byte.tif", "/raster/rnd_percentile_raster3_byte.tif", "/raster/rnd_percentile_raster4_byte.tif", "/raster/rnd_percentile_raster5_byte.tif" } )
    << u"/raster/rnd_percentile_raster1_byte.tif"_s
    << 0
    << 0.789
    << false
    << u"/percentile_nearest_ignoreFalse_byte.tif"_s
    << Qgis::DataType::Byte;
}


void TestQgsProcessingAlgsPt1::percentileRaster()
{
  QFETCH( QStringList, inputRasters );
  QFETCH( QString, referenceLayer );
  QFETCH( int, method );
  QFETCH( double, percentile );
  QFETCH( bool, ignoreNoData );
  QFETCH( QString, expectedRaster );
  QFETCH( Qgis::DataType, expectedDataType );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:cellstackpercentile"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CMakeLists.txt

  QStringList inputDatasetPaths;

  for ( const auto &raster : inputRasters )
  {
    inputDatasetPaths << myDataPath + raster;
  }

  auto inputRasterLayer1 = std::make_unique<QgsRasterLayer>( myDataPath + inputRasters[0], "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer1->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, inputDatasetPaths );
  parameters.insert( u"METHOD"_s, method );
  parameters.insert( u"PERCENTILE"_s, percentile );
  parameters.insert( u"IGNORE_NODATA"_s, ignoreNoData );
  parameters.insert( u"REFERENCE_LAYER"_s, QString( myDataPath + referenceLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images/expected_cellStackPercentile/" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...
  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double roundedExpectedValue = std::round( expectedRasterBlock->value( row, column ) * 4 ) * 4;
        const double roundedOutputValue = std::round( outputRasterBlock->value( row, column ) * 4 ) * 4;
        QCOMPARE( roundedOutputValue, roundedExpectedValue );
      }
    }
  }
}

Q_DECLARE_METATYPE( QgsRasterAnalysisUtils::CellValuePercentRankMethods )
void TestQgsProcessingAlgsPt1::percentrankFunctions_data()
{
  QTest::addColumn<QgsRasterAnalysisUtils::CellValuePercentRankMethods>( "function" );
  QTest::addColumn<std::vector<double>>( "inputValues" );
  QTest::addColumn<std::vector<double>>( "inputPercentrank" );
  QTest::addColumn<std::vector<double>>( "expectedValues" );

  QTest::newRow( "testcase_1" )
    << QgsRasterAnalysisUtils::InterpolatedPercentRankInc
    << std::vector<double>( { 100, 24, 49, 36, 2, 18, 98, 64, 20, 20 } )
    << std::vector<double>( { -8, 2, 18, 20, 33, 47, 29, 39.5, 57, 39, 12, 100, 150 } )
    << std::vector<double>( { -9999, 0, 0.111111111111, 0.222222222222, 0.527777777778, 0.649572649573, 0.490740740741, 0.58547008547, 0.725925925926, 0.581196581197, 0.0694444444444, 1, -9999 } );

  QTest::newRow( "testcase_2" )
    << QgsRasterAnalysisUtils::InterpolatedPercentRankExc
    << std::vector<double>( { 100, 24, 49, 36, 2, 18, 98, 64, 20, 20 } )
    << std::vector<double>( { -8, 2, 18, 20, 33, 47, 29, 39.5, 57, 39, 12, 100, 150 } )
    << std::vector<double>( { -9999, 0.0909090909091, 0.1818181818181, 0.272727272727, 0.522727272727, 0.622377622378, 0.492424242424, 0.56993006993, 0.684848484848, 0.566433566434, 0.1477272727272, 0.909090909091, -9999 } );
}

void TestQgsProcessingAlgsPt1::percentrankFunctions()
{
  QFETCH( QgsRasterAnalysisUtils::CellValuePercentRankMethods, function );
  QFETCH( std::vector<double>, inputValues );
  QFETCH( std::vector<double>, inputPercentrank );
  QFETCH( std::vector<double>, expectedValues );

  const int inputValuesSize = static_cast<int>( inputValues.size() );
  const int percentrankSize = static_cast<int>( inputPercentrank.size() );
  double result;

  for ( int i = 0; i < percentrankSize; i++ )
  {
    const double percentrank = inputPercentrank[i];
    const double expectedValue = expectedValues[i];

    switch ( function )
    {
      case ( QgsRasterAnalysisUtils::InterpolatedPercentRankInc ):
      {
        result = QgsRasterAnalysisUtils::interpolatedPercentRankInc( inputValues, inputValuesSize, percentrank, -9999 );
        QCOMPARE( result, expectedValue );
        break;
      }
      case ( QgsRasterAnalysisUtils::InterpolatedPercentRankExc ):
      {
        result = QgsRasterAnalysisUtils::interpolatedPercentRankExc( inputValues, inputValuesSize, percentrank, -9999 );
        QCOMPARE( result, expectedValue );
        break;
      }
    }
  }

  std::vector<double> cellVal = std::vector<double>( { 13, 36, 13, 44, 60 } );

  qDebug() << QgsRasterAnalysisUtils::interpolatedPercentRankInc( cellVal, 5, 13, 200 );

  QVERIFY( true );
}

void TestQgsProcessingAlgsPt1::percentrankByRaster_data()
{
  QTest::addColumn<QString>( "valueLayer" );
  QTest::addColumn<int>( "valueLayerBand" );
  QTest::addColumn<QStringList>( "inputRasters" );
  QTest::addColumn<QString>( "referenceLayer" );
  QTest::addColumn<int>( "method" );
  QTest::addColumn<bool>( "ignoreNoData" );
  QTest::addColumn<double>( "noDataValue" );
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );

  /*
   * Testcase 1: nearest, ignoreNoData = true, dataType = Float64
   */
  QTest::newRow( "testcase_1" )
    << u"/raster/rnd_percentrank_valueraster_float64.tif"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 0
    << true
    << -9999.0
    << u"/percentRankByRaster_inc_ignoreTrue_float64.tif"_s
    << Qgis::DataType::Float32;

  /*
   * Testcase 2: inc, ignoreNoData = true, dataType = Float64
   */
  QTest::newRow( "testcase_2" )
    << u"/raster/rnd_percentrank_valueraster_float64.tif"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 1
    << true
    << -9999.0
    << u"/percentRankByRaster_exc_ignoreTrue_float64.tif"_s
    << Qgis::DataType::Float32;

  /*
   * Testcase 3: nearest, ignoreNoData = false, dataType = Float64
   */
  QTest::newRow( "testcase_3" )
    << u"/raster/rnd_percentrank_valueraster_float64.tif"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 0
    << false
    << -9999.0
    << u"/percentRankByRaster_inc_ignoreFalse_float64.tif"_s
    << Qgis::DataType::Float32;

  /*
   * Testcase 4: inc, ignoreNoData = false, dataType = Float64
   */
  QTest::newRow( "testcase_4" )
    << u"/raster/rnd_percentrank_valueraster_float64.tif"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 1
    << false
    << -9999.0
    << u"/percentRankByRaster_exc_ignoreFalse_float64.tif"_s
    << Qgis::DataType::Float32;


  /*
   * Testcase 5: inc, ignoreNoData = false, dataType = Byte
   */
  QTest::newRow( "testcase_5" )
    << u"/raster/rnd_percentile_raster1_byte.tif"_s
    << 1
    << QStringList( { "/raster/rnd_percentile_raster1_byte.tif", "/raster/rnd_percentile_raster2_byte.tif", "/raster/rnd_percentile_raster3_byte.tif", "/raster/rnd_percentile_raster4_byte.tif", "/raster/rnd_percentile_raster5_byte.tif" } )
    << u"/raster/rnd_percentile_raster1_byte.tif"_s
    << 0
    << false
    << 200.0
    << u"/percentRankByRaster_inc_ignoreFalse_byte.tif"_s
    << Qgis::DataType::Float32;
}


void TestQgsProcessingAlgsPt1::percentrankByRaster()
{
  QFETCH( QString, valueLayer );
  QFETCH( int, valueLayerBand );
  QFETCH( QStringList, inputRasters );
  QFETCH( QString, referenceLayer );
  QFETCH( int, method );
  QFETCH( bool, ignoreNoData );
  QFETCH( double, noDataValue );
  QFETCH( QString, expectedRaster );
  QFETCH( Qgis::DataType, expectedDataType );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:cellstackpercentrankfromrasterlayer"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CMakeLists.txt

  QStringList inputDatasetPaths;

  for ( const auto &raster : inputRasters )
  {
    inputDatasetPaths << myDataPath + raster;
  }

  auto inputRasterLayer1 = std::make_unique<QgsRasterLayer>( myDataPath + inputRasters[0], "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer1->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, inputDatasetPaths );
  parameters.insert( u"INPUT_VALUE_RASTER"_s, QString( myDataPath + valueLayer ) );
  parameters.insert( u"VALUE_RASTER_BAND"_s, valueLayerBand );
  parameters.insert( u"METHOD"_s, method );
  parameters.insert( u"IGNORE_NODATA"_s, ignoreNoData );
  parameters.insert( u"OUTPUT_NODATA_VALUE"_s, noDataValue );
  parameters.insert( u"REFERENCE_LAYER"_s, QString( myDataPath + referenceLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images/expected_cellStackPercentrankFromRaster/" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...
  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double roundedExpectedValue = std::round( expectedRasterBlock->value( row, column ) * 4 ) * 4;
        const double roundedOutputValue = std::round( outputRasterBlock->value( row, column ) * 4 ) * 4;
        QCOMPARE( roundedOutputValue, roundedExpectedValue );
      }
    }
  }
}

void TestQgsProcessingAlgsPt1::percentrankByValue_data()
{
  QTest::addColumn<QStringList>( "inputRasters" );
  QTest::addColumn<QString>( "referenceLayer" );
  QTest::addColumn<double>( "value" );
  QTest::addColumn<int>( "method" );
  QTest::addColumn<bool>( "ignoreNoData" );
  QTest::addColumn<double>( "noDataValue" );
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );

  /*
   * Testcase 1: nearest, ignoreNoData = true, dataType = Float64
   */
  QTest::newRow( "testcase_1" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 83.327
    << 0
    << true
    << -9999.0
    << u"/percentRankByValue_inc_ignoreTrue_float64.tif"_s
    << Qgis::DataType::Float32;

  /*
   * Testcase 2: inc, ignoreNoData = true, dataType = Float64
   */
  QTest::newRow( "testcase_2" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 7.99
    << 1
    << true
    << -9999.0
    << u"/percentRankByValue_exc_ignoreTrue_float64.tif"_s
    << Qgis::DataType::Float32;

  /*
   * Testcase 3: nearest, ignoreNoData = false, dataType = Float64
   */
  QTest::newRow( "testcase_3" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 200.78
    << 0
    << false
    << -9999.0
    << u"/percentRankByValue_inc_ignoreFalse_float64.tif"_s
    << Qgis::DataType::Float32;

  /*
   * Testcase 4: inc, ignoreNoData = false, dataType = Float64
   */
  QTest::newRow( "testcase_4" )
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas4_float64.asc", "/raster/rnd_percentile_raster1_float64.tif", "/raster/rnd_percentile_raster2_float64.tif", "/raster/rnd_percentile_raster3_float64.tif", "/raster/rnd_percentile_raster4_float64.tif", "/raster/rnd_percentile_raster5_float64.tif" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << 56.78
    << 1
    << false
    << -9999.0
    << u"/percentRankByValue_exc_ignoreFalse_float64.tif"_s
    << Qgis::DataType::Float32;


  /*
   * Testcase 5: inc, ignoreNoData = false, dataType = Byte
   */
  QTest::newRow( "testcase_5" )
    << QStringList( { "/raster/rnd_percentile_raster1_byte.tif", "/raster/rnd_percentile_raster2_byte.tif", "/raster/rnd_percentile_raster3_byte.tif", "/raster/rnd_percentile_raster4_byte.tif", "/raster/rnd_percentile_raster5_byte.tif" } )
    << u"/raster/rnd_percentile_raster1_byte.tif"_s
    << 19.0
    << 0
    << false
    << 200.0
    << u"/percentRankByValue_inc_ignoreFalse_byte.tif"_s
    << Qgis::DataType::Float32;
}


void TestQgsProcessingAlgsPt1::percentrankByValue()
{
  QFETCH( QStringList, inputRasters );
  QFETCH( QString, referenceLayer );
  QFETCH( double, value );
  QFETCH( int, method );
  QFETCH( bool, ignoreNoData );
  QFETCH( double, noDataValue );
  QFETCH( QString, expectedRaster );
  QFETCH( Qgis::DataType, expectedDataType );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:cellstackpercentrankfromvalue"_s ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CMakeLists.txt

  QStringList inputDatasetPaths;

  for ( const auto &raster : inputRasters )
  {
    inputDatasetPaths << myDataPath + raster;
  }

  auto inputRasterLayer1 = std::make_unique<QgsRasterLayer>( myDataPath + inputRasters[0], "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer1->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, inputDatasetPaths );
  parameters.insert( u"VALUE"_s, value );
  parameters.insert( u"METHOD"_s, method );
  parameters.insert( u"IGNORE_NODATA"_s, ignoreNoData );
  parameters.insert( u"OUTPUT_NODATA_VALUE"_s, noDataValue );
  parameters.insert( u"REFERENCE_LAYER"_s, QString( myDataPath + referenceLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images/expected_cellStackPercentrankFromValue/" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...
  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double roundedExpectedValue = std::round( expectedRasterBlock->value( row, column ) * 4 ) * 4;
        const double roundedOutputValue = std::round( outputRasterBlock->value( row, column ) * 4 ) * 4;
        QCOMPARE( roundedOutputValue, roundedExpectedValue );
      }
    }
  }
}

void TestQgsProcessingAlgsPt1::rasterFrequencyByComparisonOperator_data()
{
  QTest::addColumn<QString>( "algName" );
  QTest::addColumn<QString>( "inputValueRaster" );
  QTest::addColumn<int>( "inputValueRasterBand" );
  QTest::addColumn<QStringList>( "inputRasters" );
  QTest::addColumn<bool>( "ignoreNoData" );
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );

  /*
   * Testcase 1 - equal to frequency: don't ignore NoData
   */
  QTest::newRow( "testcase_1" )
    << u"native:equaltofrequency"_s
    << u"/raster/valueRas1_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_equalToFrequency/equalToFrequencyTest1.tif"_s
    << Qgis::DataType::Int32;
  /*
   * Testcase 2 - equal to frequency: ignore NoData
   */
  QTest::newRow( "testcase_2" )
    << u"native:equaltofrequency"_s
    << u"/raster/valueRas1_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << true
    << u"/expected_equalToFrequency/equalToFrequencyTest2.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 3 - equal to frequency: NoData in value raster
   */
  QTest::newRow( "testcase_3" )
    << u"native:equaltofrequency"_s
    << u"/raster/valueRas2_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_equalToFrequency/equalToFrequencyTest3.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 4 - equal to frequency: test with random byte raster
   */
  QTest::newRow( "testcase_4" )
    << u"native:equaltofrequency"_s
    << u"/raster/valueRas3_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_equalToFrequency/equalToFrequencyTest4.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 5 - greater than frequency: don't ignore NoData
   */
  QTest::newRow( "testcase_5" )
    << u"native:greaterthanfrequency"_s
    << u"/raster/valueRas1_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_greaterThanFrequency/greaterThanFrequencyTest1.tif"_s
    << Qgis::DataType::Int32;
  /*
   * Testcase 6 - greater than frequency: ignore NoData
   */
  QTest::newRow( "testcase_6" )
    << u"native:greaterthanfrequency"_s
    << u"/raster/valueRas1_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << true
    << u"/expected_greaterThanFrequency/greaterThanFrequencyTest2.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 7 - greater than frequency: NoData in value raster
   */
  QTest::newRow( "testcase_7" )
    << u"native:greaterthanfrequency"_s
    << u"/raster/valueRas2_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_greaterThanFrequency/greaterThanFrequencyTest3.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 8 - greater than frequency: test with random byte raster
   */
  QTest::newRow( "testcase_8" )
    << u"native:greaterthanfrequency"_s
    << u"/raster/valueRas3_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_greaterThanFrequency/greaterThanFrequencyTest4.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 9 - less than frequency: don't ignore NoData
   */
  QTest::newRow( "testcase_9" )
    << u"native:lessthanfrequency"_s
    << u"/raster/valueRas1_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_lessThanFrequency/lessThanFrequencyTest1.tif"_s
    << Qgis::DataType::Int32;
  /*
   * Testcase 10 - greater than frequency: ignore NoData
   */
  QTest::newRow( "testcase_10" )
    << u"native:lessthanfrequency"_s
    << u"/raster/valueRas1_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << true
    << u"/expected_lessThanFrequency/lessThanFrequencyTest2.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 11 - less than frequency: NoData in value raster
   */
  QTest::newRow( "testcase_11" )
    << u"native:lessthanfrequency"_s
    << u"/raster/valueRas2_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_lessThanFrequency/lessThanFrequencyTest3.tif"_s
    << Qgis::DataType::Int32;

  /*
   * Testcase 12 - less than frequency: test with random byte raster
   */
  QTest::newRow( "testcase_12" )
    << u"native:lessthanfrequency"_s
    << u"/raster/valueRas3_float64.asc"_s
    << 1
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << false
    << u"/expected_lessThanFrequency/lessThanFrequencyTest4.tif"_s
    << Qgis::DataType::Int32;
}

void TestQgsProcessingAlgsPt1::rasterFrequencyByComparisonOperator()
{
  QFETCH( QString, algName );
  QFETCH( QString, inputValueRaster );
  QFETCH( int, inputValueRasterBand );
  QFETCH( QStringList, inputRasters );
  QFETCH( bool, ignoreNoData );
  QFETCH( QString, expectedRaster );
  QFETCH( Qgis::DataType, expectedDataType );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( algName ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QStringList inputDatasetPaths;

  for ( const auto &raster : inputRasters )
  {
    inputDatasetPaths << myDataPath + raster;
  }

  auto inputRasterLayer1 = std::make_unique<QgsRasterLayer>( myDataPath + inputRasters[0], "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer1->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT_VALUE_RASTER"_s, QString( myDataPath + inputValueRaster ) );
  parameters.insert( u"INPUT_VALUE_RASTER_BAND"_s, inputValueRasterBand );
  parameters.insert( u"INPUT_RASTERS"_s, inputDatasetPaths );
  parameters.insert( u"IGNORE_NODATA"_s, ignoreNoData );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double expectedValue = expectedRasterBlock->value( row, column );
        const double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );

        const Qgis::DataType outputDataType = outputRasterBlock->dataType();
        QCOMPARE( outputDataType, expectedDataType );
      }
    }
  }
}


void TestQgsProcessingAlgsPt1::rasterLocalPosition_data()
{
  QTest::addColumn<QString>( "algName" );
  QTest::addColumn<QStringList>( "inputRasters" );
  QTest::addColumn<QString>( "referenceRaster" );
  QTest::addColumn<bool>( "ignoreNoData" );
  QTest::addColumn<QString>( "expectedRaster" );

  QTest::newRow( "testcase_1" )
    << u"native:lowestpositioninrasterstack"_s
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << false
    << u"/expected_lowestPosition/expectedLowestPositionTest1.tif"_s;

  QTest::newRow( "testcase_2" )
    << u"native:lowestpositioninrasterstack"_s
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << true
    << u"/expected_lowestPosition/expectedLowestPositionTest2.tif"_s;

  QTest::newRow( "testcase_3" )
    << u"native:lowestpositioninrasterstack"_s
    << QStringList( { "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << false
    << u"/expected_lowestPosition/expectedLowestPositionTest3.tif"_s;

  QTest::newRow( "testcase_4" )
    << u"native:highestpositioninrasterstack"_s
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << false
    << u"/expected_highestPosition/expectedHighestPositionTest1.tif"_s;

  QTest::newRow( "testcase_5" )
    << u"native:highestpositioninrasterstack"_s
    << QStringList( { "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << true
    << u"/expected_highestPosition/expectedHighestPositionTest2.tif"_s;

  QTest::newRow( "testcase_6" )
    << u"native:highestpositioninrasterstack"_s
    << QStringList( { "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas1_float64.asc", "/raster/statisticsRas3_float64.asc" } )
    << u"/raster/statisticsRas1_float64.asc"_s
    << false
    << u"/expected_highestPosition/expectedHighestPositionTest3.tif"_s;
}

void TestQgsProcessingAlgsPt1::rasterLocalPosition()
{
  QFETCH( QString, algName );
  QFETCH( QStringList, inputRasters );
  QFETCH( QString, referenceRaster );
  QFETCH( bool, ignoreNoData );
  QFETCH( QString, expectedRaster );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( algName ) );

  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QStringList inputDatasetPaths;

  for ( const auto &raster : inputRasters )
  {
    inputDatasetPaths << myDataPath + raster;
  }

  auto inputRasterLayer1 = std::make_unique<QgsRasterLayer>( myDataPath + inputRasters[0], "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer1->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT_RASTERS"_s, inputDatasetPaths );
  parameters.insert( u"REFERENCE_LAYER"_s, QString( myDataPath + referenceRaster ) );
  parameters.insert( u"IGNORE_NODATA"_s, ignoreNoData );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( myDataPath + "/control_images" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...
  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double expectedValue = expectedRasterBlock->value( row, column );
        const double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );

        const Qgis::DataType outputDataType = outputRasterBlock->dataType();
        QCOMPARE( outputDataType, Qgis::DataType::Int32 );
      }
    }
  }
}

void TestQgsProcessingAlgsPt1::roundRasterValues_data()
{
  QTest::addColumn<QString>( "inputRaster" );
  QTest::addColumn<QString>( "expectedRaster" );
  QTest::addColumn<int>( "inputBand" );
  QTest::addColumn<int>( "roundingDirection" );
  QTest::addColumn<int>( "decimals" );
  QTest::addColumn<int>( "baseN" );

  /*
   * Testcase 1
   *
   * Integer Raster Layer
   * band = 1
   * roundingDirection = nearest
   * decimals = 2
   */
  QTest::newRow( "testcase 1" )
    << "/raster/dem.tif"
    << u"/roundRasterValues_testcase1.tif"_s //no output expected: can't round integer
    << 1
    << 1
    << 2
    << 10;

  /*
   * Testcase 2
   *
   * WGS84 dem
   * band = 1
   * roundingDirection = up
   * decimals = 2
   */
  QTest::newRow( "testcase 2" )
    << "/raster/dem.tif"
    << u"/roundRasterValues_testcase2.tif"_s
    << 1
    << 0
    << 2
    << 10;

  /*
   * Testcase 3
   *
   * WGS84 dem
   * band = 1
   * roundingDirection = down
   * decimals = 1
   */
  QTest::newRow( "testcase 3" )
    << "/raster/dem.tif"
    << u"/roundRasterValues_testcase3.tif"_s
    << 1
    << 2
    << 1
    << 10;

  /*
   * Testcase 4
   *
   * WGS84 dem
   * band = 1
   * roundingDirection = nearest
   * decimals = -1
   */
  QTest::newRow( "testcase 4" )
    << "/raster/dem.tif"
    << u"/roundRasterValues_testcase4.tif"_s
    << 1
    << 1
    << -1
    << 10;

  /*
   * Testcase 5
   *
   * WGS84 dem
   * band = 1
   * roundingDirection = up
   * decimals = -1
   */
  QTest::newRow( "testcase 5" )
    << "/raster/dem.tif"
    << u"/roundRasterValues_testcase5.tif"_s
    << 1
    << 0
    << -1
    << 10;

  /*
   * Testcase 6
   *
   * WGS84 dem
   * band = 1
   * roundingDirection = down
   * decimals = -1
   */
  QTest::newRow( "testcase 6" )
    << "/raster/dem.tif"
    << u"/roundRasterValues_testcase6.tif"_s
    << 1
    << 2
    << -1
    << 10;

  /*
   * Testcase 7
   *
   * WGS84 int
   * band = 1
   * roundingDirection = nearest
   * decimals = 2
   */
  QTest::newRow( "testcase 7" )
    << "/raster/band1_int16_noct_epsg4326.tif"
    << u"/roundRasterValues_testcase7.tif"_s
    << 1
    << 1
    << -1
    << 10;

  /*
   * Testcase 8
   *
   * WGS84 int
   * band = 1
   * roundingDirection = nearest
   * decimals = -1
   */
  QTest::newRow( "testcase 8" )
    << "/raster/band1_int16_noct_epsg4326.tif"
    << u"/roundRasterValues_testcase8.tif"_s
    << 1
    << 1
    << -1
    << 10;
}

void TestQgsProcessingAlgsPt1::roundRasterValues()
{
  QFETCH( QString, inputRaster );
  QFETCH( QString, expectedRaster );
  QFETCH( int, inputBand );
  QFETCH( int, roundingDirection );
  QFETCH( int, decimals );
  QFETCH( int, baseN );

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:roundrastervalues"_s ) );

  const QString rasterSource = copyTestData( inputRaster );
  auto inputRasterLayer = std::make_unique<QgsRasterLayer>( rasterSource, "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, rasterSource );
  parameters.insert( u"BAND"_s, inputBand );
  parameters.insert( u"ROUNDING_DIRECTION"_s, roundingDirection );
  parameters.insert( u"DECIMAL_PLACES"_s, decimals );
  parameters.insert( u"BASE_N"_s, baseN );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  auto expectedRasterLayer = std::make_unique<QgsRasterLayer>( testDataPath( "/control_images/roundRasterValues/" + expectedRaster ), "expectedDataset", "gdal" );
  std::unique_ptr<QgsRasterInterface> expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

  QCOMPARE( outputRaster->width(), expectedRasterLayer->width() );
  QCOMPARE( outputRaster->height(), expectedRasterLayer->height() );

  QgsRasterIterator outputIter( outputInterface.get() );
  outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
  int outputIterLeft = 0;
  int outputIterTop = 0;
  int outputIterCols = 0;
  int outputIterRows = 0;
  int expectedIterLeft = 0;
  int expectedIterTop = 0;
  int expectedIterCols = 0;
  int expectedIterRows = 0;

  std::unique_ptr<QgsRasterBlock> outputRasterBlock;
  std::unique_ptr<QgsRasterBlock> expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) && expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        const double expectedValue = expectedRasterBlock->value( row, column );
        const double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );
      }
    }
  }
}

void TestQgsProcessingAlgsPt1::layoutMapExtent()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:printlayoutmapextenttolayer"_s ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );

  QVariantMap parameters;
  parameters.insert( u"LAYOUT"_s, u"l"_s );
  parameters.insert( u"MAP"_s, u"m"_s );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  // no layout
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->setName( u"l"_s );
  p.layoutManager()->addLayout( layout );

  // no matching map
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( map );
  map->setId( u"m"_s );
  map->setCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  map->attemptSetSceneRect( QRectF( 100, 100, 150, 180 ) );
  map->zoomToExtent( QgsRectangle( 10000, 100000, 60000, 180000 ) );
  map->setMapRotation( 45 );
  map->setScale( 10000 );
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( map2 );
  map2->setId( u"m2"_s );
  map2->setCrs( QgsCoordinateReferenceSystem( u"EPSG:3785"_s ) );
  map2->attemptSetSceneRect( QRectF( 100, 100, 50, 80 ) );
  map2->zoomToExtent( QgsRectangle( 10000, 100000, 5000, 8000 ) );
  map2->setMapRotation( 0 );
  map2->setScale( 1000 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"WIDTH"_s ).toDouble(), 150.0 );
  QCOMPARE( results.value( u"HEIGHT"_s ).toDouble(), 180.0 );
  QCOMPARE( results.value( u"SCALE"_s ).toDouble(), 10000.0 );
  QCOMPARE( results.value( u"ROTATION"_s ).toDouble(), 45.0 );

  QgsFeature f;
  QCOMPARE( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() )->crs().authid(), u"EPSG:3111"_s );
  QVERIFY( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) )->getFeatures().nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), u"m"_s );
  QCOMPARE( f.attribute( 1 ).toDouble(), 150.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 180.0 );
  QCOMPARE( f.attribute( 3 ).toDouble(), 10000.0 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 45.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((33833 140106, 34894 141167, 36167 139894, 35106 138833, 33833 140106))"_s );

  // all maps
  parameters.remove( u"MAP"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"WIDTH"_s ).isValid() );
  QVERIFY( !results.value( u"HEIGHT"_s ).isValid() );
  QVERIFY( !results.value( u"SCALE"_s ).isValid() );
  QVERIFY( !results.value( u"ROTATION"_s ).isValid() );

  QCOMPARE( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() )->crs().authid(), u"EPSG:3785"_s );
  QgsFeatureIterator it = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) )->getFeatures();
  QgsFeature f1;
  QVERIFY( it.nextFeature( f1 ) );
  QgsFeature f2;
  QVERIFY( it.nextFeature( f2 ) );
  f = f1.attribute( 0 ).toString() == "m"_L1 ? f1 : f2;
  QCOMPARE( f.attribute( 0 ).toString(), u"m"_s );
  QCOMPARE( f.attribute( 1 ).toDouble(), 150.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 180.0 );
  QCOMPARE( f.attribute( 3 ).toDouble(), 10000.0 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 45.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((12077408 -7108521, 12079627 -7107575, 12080760 -7110245, 12078540 -7111191, 12077408 -7108521))"_s );
  f = f1.attribute( 0 ).toString() == "m"_L1 ? f2 : f1;
  QCOMPARE( f.attribute( 0 ).toString(), u"m2"_s );
  QCOMPARE( f.attribute( 1 ).toDouble(), 50.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 80.0 );
  QGSCOMPARENEAR( f.attribute( 3 ).toDouble(), 1000.0, 0.0001 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 0.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((7475 54040, 7525 54040, 7525 53960, 7475 53960, 7475 54040))"_s );

  // crs override
  parameters.insert( u"CRS"_s, u"EPSG:3111"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"WIDTH"_s ).isValid() );
  QVERIFY( !results.value( u"HEIGHT"_s ).isValid() );
  QVERIFY( !results.value( u"SCALE"_s ).isValid() );
  QVERIFY( !results.value( u"ROTATION"_s ).isValid() );

  QCOMPARE( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() )->crs().authid(), u"EPSG:3111"_s );
  it = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) )->getFeatures();
  QVERIFY( it.nextFeature( f1 ) );
  QVERIFY( it.nextFeature( f2 ) );
  f = f1.attribute( 0 ).toString() == "m"_L1 ? f1 : f2;
  QCOMPARE( f.attribute( 0 ).toString(), u"m"_s );
  QCOMPARE( f.attribute( 1 ).toDouble(), 150.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 180.0 );
  QCOMPARE( f.attribute( 3 ).toDouble(), 10000.0 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 45.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((33833 140106, 34894 141167, 36167 139894, 35106 138833, 33833 140106))"_s );
  f = f1.attribute( 0 ).toString() == "m"_L1 ? f2 : f1;
  QCOMPARE( f.attribute( 0 ).toString(), u"m2"_s );
  QCOMPARE( f.attribute( 1 ).toDouble(), 50.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 80.0 );
  QGSCOMPARENEAR( f.attribute( 3 ).toDouble(), 1000.0, 0.0001 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 0.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((-10399464 -5347896, -10399461 -5347835, -10399364 -5347840, -10399367 -5347901, -10399464 -5347896))"_s );
}

void TestQgsProcessingAlgsPt1::styleFromProject()
{
  QgsProject p;
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );
  p.addMapLayer( vl );
  QgsSimpleMarkerSymbolLayer *simpleMarkerLayer = new QgsSimpleMarkerSymbolLayer();
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol();
  markerSymbol->changeSymbolLayer( 0, simpleMarkerLayer );
  vl->setRenderer( new QgsSingleSymbolRenderer( markerSymbol ) );
  // rule based renderer
  QgsVectorLayer *vl2 = new QgsVectorLayer( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"vl2"_s, u"memory"_s );
  QVERIFY( vl2->isValid() );
  p.addMapLayer( vl2 );
  QgsSymbol *s1 = QgsSymbol::defaultSymbol( Qgis::GeometryType::Point );
  s1->setColor( QColor( 0, 255, 0 ) );
  QgsSymbol *s2 = QgsSymbol::defaultSymbol( Qgis::GeometryType::Point );
  s2->setColor( QColor( 0, 255, 255 ) );
  QgsRuleBasedRenderer::Rule *rootRule = new QgsRuleBasedRenderer::Rule( nullptr );
  QgsRuleBasedRenderer::Rule *rule2 = new QgsRuleBasedRenderer::Rule( s1, 0, 0, u"fld >= 5 and fld <= 20"_s );
  rootRule->appendChild( rule2 );
  QgsRuleBasedRenderer::Rule *rule3 = new QgsRuleBasedRenderer::Rule( s2, 0, 0, u"fld <= 10"_s );
  rule2->appendChild( rule3 );
  vl2->setRenderer( new QgsRuleBasedRenderer( rootRule ) );
  // labeling
  QgsPalLayerSettings settings;
  settings.fieldName = u"Class"_s;
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) ); // TODO: this should not be necessary!
  // raster layer
  QgsRasterLayer *rl = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/tenbytenraster.asc", u"rl"_s );
  QVERIFY( rl->isValid() );
  p.addMapLayer( rl );

  QgsRasterShader *rasterShader = new QgsRasterShader();
  QgsColorRampShader *colorRampShader = new QgsColorRampShader();
  colorRampShader->setColorRampType( Qgis::ShaderInterpolationMethod::Linear );
  colorRampShader->setSourceColorRamp( new QgsGradientColorRamp( QColor( 255, 255, 0 ), QColor( 255, 0, 255 ) ) );
  rasterShader->setRasterShaderFunction( colorRampShader );
  QgsSingleBandPseudoColorRenderer *r = new QgsSingleBandPseudoColorRenderer( rl->dataProvider(), 1, rasterShader );
  rl->setRenderer( r );

  // with layout
  QgsPrintLayout *l = new QgsPrintLayout( &p );
  l->setName( u"test layout"_s );
  l->initializeDefaults();
  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l->addLayoutItem( scalebar );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );

  p.layoutManager()->addLayout( l );

  // with annotations
  QgsTextAnnotation *annotation = new QgsTextAnnotation();
  QgsSymbol *a1 = QgsSymbol::defaultSymbol( Qgis::GeometryType::Point );
  a1->setColor( QColor( 0, 200, 0 ) );
  annotation->setMarkerSymbol( static_cast<QgsMarkerSymbol *>( a1 ) );
  QgsSymbol *a2 = QgsSymbol::defaultSymbol( Qgis::GeometryType::Polygon );
  a2->setColor( QColor( 200, 200, 0 ) );
  annotation->setFillSymbol( static_cast<QgsFillSymbol *>( a2 ) );
  p.annotationManager()->addAnnotation( annotation );

  // ok, run alg
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:stylefromproject"_s ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"SYMBOLS"_s ).toInt(), 6 );
  QCOMPARE( results.value( u"COLORRAMPS"_s ).toInt(), 1 );
  QCOMPARE( results.value( u"TEXTFORMATS"_s ).toInt(), 1 );
  QCOMPARE( results.value( u"LABELSETTINGS"_s ).toInt(), 1 );

  // read style file back in
  QgsStyle s;
  s.createMemoryDatabase();
  QVERIFY( s.importXml( results.value( u"OUTPUT"_s ).toString() ) );
  QCOMPARE( s.symbolCount(), 6 );
  QVERIFY( s.symbolNames().contains( u"Annotation Fill"_s ) );
  QVERIFY( s.symbolNames().contains( u"Annotation Marker"_s ) );
  QVERIFY( s.symbolNames().contains( u"test layout Page"_s ) );
  QVERIFY( s.symbolNames().contains( u"vl"_s ) );
  QVERIFY( s.symbolNames().contains( u"vl2"_s ) );
  QVERIFY( s.symbolNames().contains( u"vl2 (2)"_s ) );
  QCOMPARE( s.colorRampCount(), 1 );
  QVERIFY( s.colorRampNames().contains( u"rl"_s ) );
  QCOMPARE( s.textFormatCount(), 1 );
  QVERIFY( s.textFormatNames().contains( u"test layout <Scalebar>"_s ) );
  QCOMPARE( s.labelSettingsCount(), 1 );
  QVERIFY( s.labelSettingsNames().contains( u"vl"_s ) );

  // using a project path
  QTemporaryFile tmpFile;
  QVERIFY( tmpFile.open() );
  tmpFile.close();
  QVERIFY( p.write( tmpFile.fileName() ) );
  p.clear();
  parameters.insert( u"INPUT"_s, tmpFile.fileName() );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"SYMBOLS"_s ).toInt(), 6 );
  // this should be 1, but currently raster layers aren't supported -
  // we first need to allow raster renderers to be read and restored for invalid layer sources
  QCOMPARE( results.value( u"COLORRAMPS"_s ).toInt(), 0 );
  QCOMPARE( results.value( u"TEXTFORMATS"_s ).toInt(), 1 );
  QCOMPARE( results.value( u"LABELSETTINGS"_s ).toInt(), 1 );
}

void TestQgsProcessingAlgsPt1::combineStyles()
{
  QgsStyle s1;
  s1.createMemoryDatabase();
  QgsStyle s2;
  s2.createMemoryDatabase();

  QgsSimpleMarkerSymbolLayer *simpleMarkerLayer = new QgsSimpleMarkerSymbolLayer();
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol();
  markerSymbol->changeSymbolLayer( 0, simpleMarkerLayer );
  s1.addSymbol( u"sym1"_s, markerSymbol, true );
  s1.tagSymbol( QgsStyle::SymbolEntity, u"sym1"_s, QStringList() << u"t1"_s << u"t2"_s );

  QgsSymbol *sym1 = QgsSymbol::defaultSymbol( Qgis::GeometryType::Point );
  s2.addSymbol( u"sym2"_s, sym1, true );
  QgsSymbol *sym2 = QgsSymbol::defaultSymbol( Qgis::GeometryType::Point );
  s2.addSymbol( u"sym1"_s, sym2, true );

  QgsPalLayerSettings settings;
  settings.fieldName = u"Class"_s;
  s1.addLabelSettings( u"label1"_s, settings, true );

  s2.addColorRamp( u"ramp1"_s, new QgsGradientColorRamp( QColor( 255, 255, 0 ), QColor( 255, 0, 255 ) ), true );
  s2.addTextFormat( u"format2"_s, QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ), true );

  QTemporaryFile tmpFile;
  QVERIFY( tmpFile.open() );
  tmpFile.close();
  QVERIFY( s1.exportXml( tmpFile.fileName() ) );
  QTemporaryFile tmpFile2;
  QVERIFY( tmpFile2.open() );
  tmpFile2.close();
  QVERIFY( s2.exportXml( tmpFile2.fileName() ) );

  // ok, run alg
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:combinestyles"_s ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QStringList() << tmpFile.fileName() << tmpFile2.fileName() );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"SYMBOLS"_s ).toInt(), 3 );
  QCOMPARE( results.value( u"COLORRAMPS"_s ).toInt(), 1 );
  QCOMPARE( results.value( u"TEXTFORMATS"_s ).toInt(), 1 );
  QCOMPARE( results.value( u"LABELSETTINGS"_s ).toInt(), 1 );

  // check result
  QgsStyle s;
  s.createMemoryDatabase();
  QVERIFY( s.importXml( results.value( u"OUTPUT"_s ).toString() ) );
  QCOMPARE( s.symbolCount(), 3 );
  QVERIFY( s.symbolNames().contains( u"sym1"_s ) );
  QVERIFY( s.symbolNames().contains( u"sym2"_s ) );
  QVERIFY( s.symbolNames().contains( u"sym1 (2)"_s ) );
  QCOMPARE( s.tagsOfSymbol( QgsStyle::SymbolEntity, u"sym1"_s ).count(), 2 );
  QVERIFY( s.tagsOfSymbol( QgsStyle::SymbolEntity, u"sym1"_s ).contains( u"t1"_s ) );
  QVERIFY( s.tagsOfSymbol( QgsStyle::SymbolEntity, u"sym1"_s ).contains( u"t2"_s ) );
  QCOMPARE( s.colorRampCount(), 1 );
  QVERIFY( s.colorRampNames().contains( u"ramp1"_s ) );
  QCOMPARE( s.textFormatCount(), 1 );
  QVERIFY( s.textFormatNames().contains( u"format2"_s ) );
  QCOMPARE( s.labelSettingsCount(), 1 );
  QVERIFY( s.labelSettingsNames().contains( u"label1"_s ) );
}

void TestQgsProcessingAlgsPt1::bookmarksToLayer()
{
  QgsApplication::bookmarkManager()->clear();
  // create some bookmarks
  QgsBookmark b1;
  b1.setName( u"test name"_s );
  b1.setGroup( u"test group"_s );
  b1.setExtent( QgsReferencedRectangle( QgsRectangle( 1, 2, 3, 4 ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) ) );
  QgsApplication::bookmarkManager()->addBookmark( b1 );

  QgsBookmark b2;
  b2.setName( u"test name 2"_s );
  b2.setExtent( QgsReferencedRectangle( QgsRectangle( 16259461, -2477192, 16391255, -2372535 ), QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) ) );
  QgsApplication::bookmarkManager()->addBookmark( b2 );
  QgsBookmark b3;
  b3.setName( u"test name 3"_s );
  b3.setExtent( QgsReferencedRectangle( QgsRectangle( 11, 21, 31, 41 ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) ) );
  QgsProject p;
  p.bookmarkManager()->addBookmark( b3 );

  // ok, run alg
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:bookmarkstolayer"_s ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;
  parameters.insert( u"SOURCE"_s, QVariantList() << 0 );
  parameters.insert( u"CRS"_s, u"EPSG:4326"_s );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  // check result
  QgsFeature f;
  QCOMPARE( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() )->crs().authid(), u"EPSG:4326"_s );
  QgsFeatureIterator it = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) )->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), u"test name 3"_s );
  QCOMPARE( f.attribute( 1 ).toString(), QString() );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((11 21, 31 21, 31 41, 11 41, 11 21))"_s );
  QVERIFY( !it.nextFeature( f ) );

  // user bookmarks
  parameters.insert( u"SOURCE"_s, QVariantList() << 1 );
  ok = false;
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:bookmarkstolayer"_s ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() )->crs().authid(), u"EPSG:4326"_s );
  it = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) )->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), u"test name"_s );
  QCOMPARE( f.attribute( 1 ).toString(), u"test group"_s );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((1 2, 3 2, 3 4, 1 4, 1 2))"_s );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), u"test name 2"_s );
  QCOMPARE( f.attribute( 1 ).toString(), QString() );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22))"_s );
  QVERIFY( !it.nextFeature( f ) );

  // both
  parameters.insert( u"SOURCE"_s, QVariantList() << 0 << 1 );
  ok = false;
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:bookmarkstolayer"_s ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() )->crs().authid(), u"EPSG:4326"_s );
  it = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) )->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), u"test name 3"_s );
  QCOMPARE( f.attribute( 1 ).toString(), QString() );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((11 21, 31 21, 31 41, 11 41, 11 21))"_s );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), u"test name"_s );
  QCOMPARE( f.attribute( 1 ).toString(), u"test group"_s );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((1 2, 3 2, 3 4, 1 4, 1 2))"_s );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), u"test name 2"_s );
  QCOMPARE( f.attribute( 1 ).toString(), QString() );
  QCOMPARE( f.geometry().asWkt( 0 ), u"Polygon ((146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22))"_s );
  QVERIFY( !it.nextFeature( f ) );
}

void TestQgsProcessingAlgsPt1::layerToBookmarks()
{
  auto inputLayer = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:4326&field=province:string&field=municipality:string"_s, u"layer"_s, u"memory"_s );
  QVERIFY( inputLayer->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << u"b1"_s << u"g1"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"Polygon ((11 21, 31 21, 31 41, 11 41, 11 21))"_s ) );
  inputLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << u"b2"_s << QString() );
  f.setGeometry( QgsGeometry::fromWkt( u"Polygon ((146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22))"_s ) );
  inputLayer->dataProvider()->addFeature( f );

  QgsApplication::bookmarkManager()->clear();

  // run alg
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:layertobookmarks"_s ) );
  QVERIFY( alg != nullptr );

  QgsProject p;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( inputLayer.get() ) );
  parameters.insert( u"DESTINATION"_s, 0 );
  parameters.insert( u"NAME_EXPRESSION"_s, u"upper(province)"_s );
  parameters.insert( u"GROUP_EXPRESSION"_s, u"upper(municipality)"_s );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( p.bookmarkManager()->bookmarks().count(), 2 );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 0 ).name(), u"B1"_s );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 0 ).group(), u"G1"_s );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 0 ).extent().crs().authid(), u"EPSG:4326"_s );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 0 ).extent().toString( 0 ), u"11,21 : 31,41"_s );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 1 ).name(), u"B2"_s );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 1 ).group(), QString() );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 1 ).extent().crs().authid(), u"EPSG:4326"_s );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 1 ).extent().toString( 0 ), u"146,-22 : 147,-21"_s );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().count(), 0 );
  p.bookmarkManager()->clear();

  // send to application bookmarks
  parameters.insert( u"DESTINATION"_s, 1 );
  parameters.insert( u"GROUP_EXPRESSION"_s, QVariant() );

  ok = false;
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:layertobookmarks"_s ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( p.bookmarkManager()->bookmarks().count(), 0 );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().count(), 2 );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 0 ).name(), u"B1"_s );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 0 ).group(), QString() );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 0 ).extent().crs().authid(), u"EPSG:4326"_s );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 0 ).extent().toString( 0 ), u"11,21 : 31,41"_s );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 1 ).name(), u"B2"_s );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 1 ).group(), QString() );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 1 ).extent().crs().authid(), u"EPSG:4326"_s );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 1 ).extent().toString( 0 ), u"146,-22 : 147,-21"_s );
}

void TestQgsProcessingAlgsPt1::repairShapefile()
{
  const QTemporaryDir tmpPath;

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QFile::copy( dataDir + "/points.shp", tmpPath.filePath( u"points.shp"_s ) );
  QFile::copy( dataDir + "/points.shp", tmpPath.filePath( u"points.prj"_s ) );
  QFile::copy( dataDir + "/points.shp", tmpPath.filePath( u"points.dbf"_s ) );
  // no shx!!

  auto layer = std::make_unique<QgsVectorLayer>( tmpPath.filePath( u"points.shp"_s ) );
  QVERIFY( !layer->isValid() );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:repairshapefile"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, u"not a file"_s );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  parameters.insert( u"INPUT"_s, tmpPath.filePath( u"points.shp"_s ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"OUTPUT"_s ).toString(), tmpPath.filePath( u"points.shp"_s ) );

  layer = std::make_unique<QgsVectorLayer>( tmpPath.filePath( u"points.shp"_s ) );
  QVERIFY( layer->isValid() );
}

void TestQgsProcessingAlgsPt1::renameField()
{
  QgsProject p;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"vl2"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  p.addMapLayer( layer );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:renametablefield"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer ) );
  parameters.insert( u"FIELD"_s, u"doesntexist"_s );
  parameters.insert( u"NEW_NAME"_s, u"newname"_s );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // field doesn't exist
  QVERIFY( !ok );

  parameters.insert( u"FIELD"_s, u"col1"_s );
  parameters.insert( u"NEW_NAME"_s, u"pk"_s );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // already a field with this name
  QVERIFY( !ok );

  parameters.insert( u"NEW_NAME"_s, u"newname"_s );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) )->fields().at( 1 ).name(), u"newname"_s );
}

void TestQgsProcessingAlgsPt1::compareDatasets()
{
  QgsProject p;
  QgsVectorLayer *pointLayer = new QgsVectorLayer( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"vl2"_s, u"memory"_s );
  QVERIFY( pointLayer->isValid() );
  p.addMapLayer( pointLayer );
  QgsVectorLayer *originalLayer = new QgsVectorLayer( u"LineString?crs=epsg:4326&field=pk:int&field=col1:string&field=col2:string"_s, u"vl2"_s, u"memory"_s );
  QVERIFY( originalLayer->isValid() );
  p.addMapLayer( originalLayer );
  QgsVectorLayer *revisedLayer = new QgsVectorLayer( u"LineString?crs=epsg:4326&field=pk:int&field=col1:string&field=col2:string"_s, u"vl2"_s, u"memory"_s );
  QVERIFY( revisedLayer->isValid() );
  p.addMapLayer( revisedLayer );
  QgsVectorLayer *differentAttrs = new QgsVectorLayer( u"LineString?crs=epsg:4326&field=pk:int&field=col3:string&field=col2:string"_s, u"vl2"_s, u"memory"_s );
  QVERIFY( differentAttrs->isValid() );
  p.addMapLayer( differentAttrs );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:detectvectorchanges"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  // differing geometry types - alg should fail
  parameters.insert( u"ORIGINAL"_s, QVariant::fromValue( pointLayer ) );
  parameters.insert( u"REVISED"_s, QVariant::fromValue( originalLayer ) );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // differing fields - alg should fail
  parameters.insert( u"ORIGINAL"_s, QVariant::fromValue( originalLayer ) );
  parameters.insert( u"REVISED"_s, QVariant::fromValue( differentAttrs ) );
  parameters.insert( u"COMPARE_ATTRIBUTES"_s, u"col1;col2"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // yet if we aren't comparing the field, we shouldn't fail...
  parameters.insert( u"COMPARE_ATTRIBUTES"_s, u"col2"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  // no features, no outputs
  parameters.insert( u"ORIGINAL"_s, QVariant::fromValue( originalLayer ) );
  parameters.insert( u"REVISED"_s, QVariant::fromValue( revisedLayer ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 0LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 0LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 0LL );

  parameters.insert( u"UNCHANGED"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ADDED"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"DELETED"_s, QgsProcessing::TEMPORARY_OUTPUT );

  // add two features to original
  QgsFeature f;
  f.setAttributes( QgsAttributes() << 5 << u"b1"_s << u"g1"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 0, 10 0)"_s ) );
  originalLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 5 << u"c1"_s << u"g1"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 0, 10 0)"_s ) );
  originalLayer->dataProvider()->addFeature( f );

  // just compare two columns
  parameters.insert( u"COMPARE_ATTRIBUTES"_s, u"col1;col2"_s );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 0LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"UNCHANGED"_s ).toString() ) )->featureCount(), 0L );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 0LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"ADDED"_s ).toString() ) )->featureCount(), 0L );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"DELETED"_s ).toString() ) )->featureCount(), 2L );

  // add one same to revised - note that the first attributes differs here, but we aren't considering that
  f.setAttributes( QgsAttributes() << 55 << u"b1"_s << u"g1"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 0, 10 0)"_s ) );
  revisedLayer->dataProvider()->addFeature( f );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"UNCHANGED"_s ).toString() ) )->featureCount(), 1L );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 0LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"ADDED"_s ).toString() ) )->featureCount(), 0L );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"DELETED"_s ).toString() ) )->featureCount(), 1L );

  // ok, let's compare the differing attribute too
  parameters.insert( u"COMPARE_ATTRIBUTES"_s, u"col1;col2;pk"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 0LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 1LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 2LL );

  parameters.insert( u"COMPARE_ATTRIBUTES"_s, u"col1;col2"_s );
  // similar to the second feature, but geometry differs
  f.setAttributes( QgsAttributes() << 55 << u"c1"_s << u"g1"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 0, 11 0)"_s ) );
  revisedLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"UNCHANGED"_s ).toString() ) )->featureCount(), 1L );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"ADDED"_s ).toString() ) )->featureCount(), 1L );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"DELETED"_s ).toString() ) )->featureCount(), 1L );
  // note - we skip the featureCount checks after this -- we can be confident at this stage that all sinks are correctly being populated


  // add another which is identical to first, must be considered as another "added" feature (i.e.
  // don't match to same original feature multiple times)
  f.setAttributes( QgsAttributes() << 555 << u"b1"_s << u"g1"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 0, 10 0)"_s ) );
  revisedLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 1LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 1LL );

  // add a match for the second feature
  f.setAttributes( QgsAttributes() << 5 << u"c1"_s << u"g1"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 0, 10 0)"_s ) );
  revisedLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 0LL );

  // test topological match (different number of vertices)
  f.setAttributes( QgsAttributes() << 5 << u"c1"_s << u"g1"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 10, 5 10, 10 10)"_s ) );
  originalLayer->dataProvider()->addFeature( f );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 10, 1 10, 5 10, 10 10)"_s ) );
  revisedLayer->dataProvider()->addFeature( f );

  // exact match shouldn't equate the two
  parameters.insert( u"MATCH_TYPE"_s, 0 );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 3LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 1LL );

  // but topological match should
  parameters.insert( u"MATCH_TYPE"_s, 1 );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 3LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 0LL );

  // null geometry comparisons
  f.setAttributes( QgsAttributes() << 5 << u"d1"_s << u"g1"_s );
  f.clearGeometry();
  originalLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 3LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 1LL );

  f.setAttributes( QgsAttributes() << 5 << u"e1"_s << u"g1"_s );
  originalLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 3LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 2LL );

  f.setAttributes( QgsAttributes() << 5 << u"d1"_s << u"g1"_s );
  revisedLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"UNCHANGED_COUNT"_s ).toLongLong(), 4LL );
  QCOMPARE( results.value( u"ADDED_COUNT"_s ).toLongLong(), 2LL );
  QCOMPARE( results.value( u"DELETED_COUNT"_s ).toLongLong(), 1LL );
}

void TestQgsProcessingAlgsPt1::shapefileEncoding()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:shpencodinginfo"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QString() );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );

  parameters.insert( u"INPUT"_s, QString( QStringLiteral( TEST_DATA_DIR ) + "/shapefile/iso-8859-1.shp" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"ENCODING"_s ).toString(), u"ISO-8859-1"_s );
  QCOMPARE( results.value( u"CPG_ENCODING"_s ).toString(), u"ISO-8859-1"_s );
  QCOMPARE( results.value( u"LDID_ENCODING"_s ).toString(), QString() );

  parameters.insert( u"INPUT"_s, QString( QStringLiteral( TEST_DATA_DIR ) + "/shapefile/windows-1252_ldid.shp" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"ENCODING"_s ).toString(), u"CP1252"_s );
  QCOMPARE( results.value( u"CPG_ENCODING"_s ).toString(), QString() );
  QCOMPARE( results.value( u"LDID_ENCODING"_s ).toString(), u"CP1252"_s );

  parameters.insert( u"INPUT"_s, QString( QStringLiteral( TEST_DATA_DIR ) + "/shapefile/system_encoding.shp" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( u"ENCODING"_s ).toString(), QString() );
  QCOMPARE( results.value( u"CPG_ENCODING"_s ).toString(), QString() );
  QCOMPARE( results.value( u"LDID_ENCODING"_s ).toString(), QString() );
}

void TestQgsProcessingAlgsPt1::setLayerEncoding()
{
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( TEST_DATA_DIR ) + "/shapefile/system_encoding.shp", u"test"_s, u"ogr"_s );
  QVERIFY( vl->isValid() );
  QgsProject p;
  p.addMapLayers(
    QList<QgsMapLayer *>() << vl
  );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:setlayerencoding"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QString() );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  parameters.insert( u"INPUT"_s, vl->id() );
  parameters.insert( u"ENCODING"_s, u"ISO-8859-1"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( vl->dataProvider()->encoding(), u"ISO-8859-1"_s );
}

class TestProcessingFeedback : public QgsProcessingFeedback
{
    Q_OBJECT
  public:
    void reportError( const QString &error, bool ) override
    {
      errors << error;
    }
    void pushWarning( const QString &warning ) override
    {
      warnings << warning;
    }
    void pushInfo( const QString &message ) override
    {
      messages << message;
    }

    QStringList errors;
    QStringList warnings;
    QStringList messages;
};

void TestQgsProcessingAlgsPt1::raiseException()
{
  TestProcessingFeedback feedback;

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:raiseexception"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"MESSAGE"_s, u"you done screwed up boy"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QCOMPARE( feedback.errors, QStringList() << u"you done screwed up boy"_s );

  parameters.insert( u"CONDITION"_s, u"FALSE"_s );
  feedback.errors.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.errors, QStringList() );

  parameters.insert( u"CONDITION"_s, u"TRUE"_s );
  feedback.errors.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QCOMPARE( feedback.errors, QStringList() << u"you done screwed up boy"_s );
}

void TestQgsProcessingAlgsPt1::raiseWarning()
{
  TestProcessingFeedback feedback;

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:raisewarning"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"MESSAGE"_s, u"you mighta screwed up boy, but i aint so sure"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.warnings, QStringList() << u"you mighta screwed up boy, but i aint so sure"_s );

  parameters.insert( u"CONDITION"_s, u"FALSE"_s );
  feedback.warnings.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.warnings, QStringList() );

  parameters.insert( u"CONDITION"_s, u"TRUE"_s );
  feedback.warnings.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.warnings, QStringList() << u"you mighta screwed up boy, but i aint so sure"_s );
}

void TestQgsProcessingAlgsPt1::raiseMessage()
{
  TestProcessingFeedback feedback;

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:raisemessage"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"MESSAGE"_s, u"nothing screwed up boy, congrats"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.messages, QStringList() << u"nothing screwed up boy, congrats"_s );

  parameters.insert( u"CONDITION"_s, u"FALSE"_s );
  feedback.messages.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.messages, QStringList() );

  parameters.insert( u"CONDITION"_s, u"TRUE"_s );
  feedback.messages.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.messages, QStringList() << u"nothing screwed up boy, congrats"_s );
}

void TestQgsProcessingAlgsPt1::randomFloatingPointDistributionRaster_data()
{
  QTest::addColumn<QString>( "inputExtent" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );
  QTest::addColumn<bool>( "succeeds" );
  QTest::addColumn<QString>( "crs" );
  QTest::addColumn<double>( "pixelSize" );
  QTest::addColumn<int>( "typeId" );

  QTest::newRow( "testcase 1" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Float32
    << true
    << "EPSG:4326"
    << 1.0
    << 0;


  QTest::newRow( "testcase 2" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Float64
    << true
    << "EPSG:4326"
    << 1.0
    << 1;
}

void TestQgsProcessingAlgsPt1::randomFloatingPointDistributionRaster()
{
  //this test only checks if the correct raster data type is chosen
  //value by value comparison is not effective for random rasters
  QFETCH( QString, inputExtent );
  QFETCH( Qgis::DataType, expectedDataType );
  QFETCH( bool, succeeds );
  QFETCH( QString, crs );
  QFETCH( double, pixelSize );
  QFETCH( int, typeId );

  //prepare input params
  QgsProject p;
  //set project crs and ellipsoid from input layer
  p.setCrs( QgsCoordinateReferenceSystem( crs ), true );

  QStringList alglist = QStringList();
  alglist << u"native:createrandomnormalrasterlayer"_s
          << u"native:createrandomexponentialrasterlayer"_s
          << u"native:createrandomgammarasterlayer"_s;

  for ( int i = 0; i < alglist.length(); i++ )
  {
    const QString algname = alglist[i];

    std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( algname ) );
    //set project after layer has been added so that transform context/ellipsoid from crs is also set
    auto context = std::make_unique<QgsProcessingContext>();
    context->setProject( &p );

    QVariantMap parameters;

    parameters.insert( u"EXTENT"_s, inputExtent );
    parameters.insert( u"TARGET_CRS"_s, QgsCoordinateReferenceSystem( crs ) );
    parameters.insert( u"PIXEL_SIZE"_s, pixelSize );
    parameters.insert( u"OUTPUT_TYPE"_s, typeId );
    parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

    bool ok = false;
    QgsProcessingFeedback feedback;
    QVariantMap results;

    if ( !succeeds )
    {
      //verify if user feedback for unacceptable values are thrown
      alg->run( parameters, *context, &feedback, &ok );
      QCOMPARE( ok, succeeds );
    }
    else
    {
      //run alg...
      results = alg->run( parameters, *context, &feedback, &ok );
      QVERIFY( ok );

      //...and check results with expected datasets
      auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
      std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

      QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
    }
  }
}


void TestQgsProcessingAlgsPt1::randomIntegerDistributionRaster_data()
{
  QTest::addColumn<QString>( "inputExtent" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );
  QTest::addColumn<bool>( "succeeds" );
  QTest::addColumn<QString>( "crs" );
  QTest::addColumn<double>( "pixelSize" );
  QTest::addColumn<int>( "typeId" );

  QTest::newRow( "testcase 1" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int16
    << true
    << "EPSG:4326"
    << 1.0
    << 0;

  QTest::newRow( "testcase 2" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::UInt16
    << true
    << "EPSG:4326"
    << 1.0
    << 1;


  QTest::newRow( "testcase 3" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int32
    << true
    << "EPSG:4326"
    << 1.0
    << 2;

  QTest::newRow( "testcase 4" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::UInt32
    << true
    << "EPSG:4326"
    << 1.0
    << 3;

  QTest::newRow( "testcase 5" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Float32
    << true
    << "EPSG:4326"
    << 1.0
    << 4;

  QTest::newRow( "testcase 6" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Float64
    << true
    << "EPSG:4326"
    << 1.0
    << 5;
}


void TestQgsProcessingAlgsPt1::randomIntegerDistributionRaster()
{
  //this test only checks if the correct raster data type is chosen
  //value by value comparison is not effective for random rasters
  QFETCH( QString, inputExtent );
  QFETCH( Qgis::DataType, expectedDataType );
  QFETCH( bool, succeeds );
  QFETCH( QString, crs );
  QFETCH( double, pixelSize );
  QFETCH( int, typeId );

  //prepare input params
  QgsProject p;
  //set project crs and ellipsoid from input layer
  p.setCrs( QgsCoordinateReferenceSystem( crs ), true );

  QStringList alglist = QStringList();
  alglist << u"native:createrandombinomialrasterlayer"_s
          << u"native:createrandomgeometricrasterlayer"_s
          << u"native:createrandomnegativebinomialrasterlayer"_s
          << u"native:createrandompoissonrasterlayer"_s;

  for ( int i = 0; i < alglist.length(); i++ )
  {
    const QString algname = alglist[i];

    std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( algname ) );
    //set project after layer has been added so that transform context/ellipsoid from crs is also set
    auto context = std::make_unique<QgsProcessingContext>();
    context->setProject( &p );

    QVariantMap parameters;

    parameters.insert( u"EXTENT"_s, inputExtent );
    parameters.insert( u"TARGET_CRS"_s, QgsCoordinateReferenceSystem( crs ) );
    parameters.insert( u"PIXEL_SIZE"_s, pixelSize );
    parameters.insert( u"OUTPUT_TYPE"_s, typeId );
    parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

    bool ok = false;
    QgsProcessingFeedback feedback;
    QVariantMap results;

    if ( !succeeds )
    {
      //verify if user feedback for unacceptable values are thrown
      alg->run( parameters, *context, &feedback, &ok );
      QCOMPARE( ok, succeeds );
    }
    else
    {
      //run alg...
      results = alg->run( parameters, *context, &feedback, &ok );
      QVERIFY( ok );

      //...and check results with expected datasets
      auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
      std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

      QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
    }
  }
}

void TestQgsProcessingAlgsPt1::randomRaster_data()
{
  QTest::addColumn<QString>( "inputExtent" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );
  QTest::addColumn<bool>( "succeeds" );
  QTest::addColumn<QString>( "crs" );
  QTest::addColumn<double>( "pixelSize" );
  QTest::addColumn<double>( "lowerBound" );
  QTest::addColumn<double>( "upperBound" );
  QTest::addColumn<int>( "typeId" );


  QTest::newRow( "testcase 1" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Byte
    << true
    << "EPSG:4326"
    << 1.0
    << 1.0
    << 1.0 //should be min max
    << 0;

  QTest::newRow( "testcase 2" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Byte
    << false
    << "EPSG:4326"
    << 1.0
    << -1.0 //fails --> value too small for byte
    << 10.0 //fails --> value too large
    << 0;

  QTest::newRow( "testcase 3" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Byte
    << false
    << "EPSG:4326"
    << 1.0
    << 1.0
    << 256.0 //fails --> value too big for byte
    << 0;

  QTest::newRow( "testcase 4" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int16
    << true
    << "EPSG:4326"
    << 1.0
    << 1.0
    << 10.0
    << 1;

  QTest::newRow( "testcase 5" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int16
    << false
    << "EPSG:4326"
    << 1.0
    << -32769.0
    << -10.0
    << 1;

  QTest::newRow( "testcase 6" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int16
    << false
    << "EPSG:4326"
    << 1.0
    << 1.0
    << 32769.0
    << 1;

  QTest::newRow( "testcase 7" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::UInt16
    << false
    << "EPSG:4326"
    << 1.0
    << -1.0
    << 12.0
    << 2;

  QTest::newRow( "testcase 8" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::UInt16
    << false
    << "EPSG:4326"
    << 1.0
    << 100.0
    << -1.0
    << 2;

  QTest::newRow( "testcase 9" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::UInt16
    << false
    << "EPSG:4326"
    << 1.0
    << 0.0
    << 65536.0
    << 2;

  QTest::newRow( "testcase 10" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int32
    << true
    << "EPSG:4326"
    << 1.0
    << 1.0
    << 12.0
    << 3;

  QTest::newRow( "testcase 10" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int32
    << false
    << "EPSG:4326"
    << 1.0
    << 15.0
    << 12.0
    << 3;

  QTest::newRow( "testcase 11" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int32
    << false
    << "EPSG:4326"
    << 1.0
    << -2147483649.0
    << 1.0
    << 3;

  QTest::newRow( "testcase 12" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Int32
    << false
    << "EPSG:4326"
    << 1.0
    << 1.0
    << 2147483649.0
    << 3;

  QTest::newRow( "testcase 13" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::UInt32
    << true
    << "EPSG:4326"
    << 1.0
    << 1.0
    << 12.0
    << 4;

  QTest::newRow( "testcase 14" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::UInt32
    << false
    << "EPSG:4326"
    << 1.0
    << 1.0
    << 4294967296.0
    << 4;

  QTest::newRow( "testcase 14" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::UInt32
    << false
    << "EPSG:4326"
    << 1.0
    << -10.0
    << -1.0
    << 4;

  QTest::newRow( "testcase 16" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Float32
    << true
    << "EPSG:4326"
    << 1.0
    << 0.0
    << 12.12
    << 5;

  QTest::newRow( "testcase 17" )
    << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
    << Qgis::DataType::Float64
    << true
    << "EPSG:4326"
    << 1.0
    << -15.0
    << 12.125789212532487
    << 6;
}

void TestQgsProcessingAlgsPt1::randomRaster()
{
  QFETCH( QString, inputExtent );
  QFETCH( Qgis::DataType, expectedDataType );
  QFETCH( bool, succeeds );
  QFETCH( QString, crs );
  QFETCH( double, pixelSize );
  QFETCH( double, lowerBound );
  QFETCH( double, upperBound );
  QFETCH( int, typeId );

  if ( qgsDoubleNear( lowerBound, upperBound ) )
  {
    //if bounds are the same, use numeric min and max as bounds
    switch ( typeId )
    {
      case 0:
        lowerBound = std::numeric_limits<quint8>::min();
        upperBound = std::numeric_limits<quint8>::max();
        break;
      case 1:
        lowerBound = std::numeric_limits<qint16>::min();
        upperBound = std::numeric_limits<qint16>::max();
        break;
      case 2:
        lowerBound = std::numeric_limits<quint16>::min();
        upperBound = std::numeric_limits<quint16>::max();
        break;
      case 3:
        lowerBound = std::numeric_limits<qint32>::min();
        upperBound = std::numeric_limits<qint32>::max();
        break;
      case 4:
        lowerBound = std::numeric_limits<quint32>::min();
        upperBound = std::numeric_limits<quint32>::max();
        break;
      case 5:
        lowerBound = std::numeric_limits<float>::min();
        upperBound = std::numeric_limits<float>::max();
        break;
      case 6:
        lowerBound = std::numeric_limits<double>::min();
        upperBound = std::numeric_limits<double>::max();
        break;
    }
  }

  //prepare input params
  QgsProject p;
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:createrandomuniformrasterlayer"_s ) );

  //set project crs and ellipsoid from input layer
  p.setCrs( QgsCoordinateReferenceSystem( crs ), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( u"EXTENT"_s, inputExtent );
  parameters.insert( u"TARGET_CRS"_s, QgsCoordinateReferenceSystem( crs ) );
  parameters.insert( u"PIXEL_SIZE"_s, pixelSize );
  parameters.insert( u"OUTPUT_TYPE"_s, typeId );
  parameters.insert( u"LOWER_BOUND"_s, lowerBound );
  parameters.insert( u"UPPER_BOUND"_s, upperBound );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  if ( !succeeds )
  {
    //verify if user feedback for unacceptable values are thrown
    alg->run( parameters, *context, &feedback, &ok );
    QCOMPARE( ok, succeeds );
  }
  else
  {
    //run alg...
    results = alg->run( parameters, *context, &feedback, &ok );
    QVERIFY( ok );

    //...and check results with expected datasets
    auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
    std::unique_ptr<QgsRasterInterface> outputInterface( outputRaster->dataProvider()->clone() );

    QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );

    QgsRasterIterator outputIter( outputInterface.get() );
    outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
    int outputIterLeft = 0;
    int outputIterTop = 0;
    int outputIterCols = 0;
    int outputIterRows = 0;

    std::unique_ptr<QgsRasterBlock> outputRasterBlock;

    while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) )
    {
      for ( int row = 0; row < outputIterRows; row++ )
      {
        for ( int column = 0; column < outputIterCols; column++ )
        {
          const double outputValue = outputRasterBlock->value( row, column );
          //check if random values are in range
          QVERIFY( outputValue >= lowerBound && outputValue <= upperBound );
        }
      }
    }
  }
}

void TestQgsProcessingAlgsPt1::filterByLayerType()
{
  QgsProject p;
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );
  p.addMapLayer( vl );
  // raster layer
  QgsRasterLayer *rl = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/tenbytenraster.asc", u"rl"_s );
  QVERIFY( rl->isValid() );
  p.addMapLayer( rl );


  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:filterlayersbytype"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  // vector input
  parameters.insert( u"INPUT"_s, u"vl"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !results.value( u"VECTOR"_s ).toString().isEmpty() );
  QVERIFY( !results.contains( u"RASTER"_s ) );

  // raster input
  parameters.insert( u"INPUT"_s, u"rl"_s );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !results.value( u"RASTER"_s ).toString().isEmpty() );
  QVERIFY( !results.contains( u"VECTOR"_s ) );
}

void TestQgsProcessingAlgsPt1::conditionalBranch()
{
  QVariantMap config;
  QVariantList conditions;
  QVariantMap cond1;
  cond1.insert( u"name"_s, u"name1"_s );
  cond1.insert( u"expression"_s, u"1 * 1"_s );
  conditions << cond1;
  QVariantMap cond2;
  cond2.insert( u"name"_s, u"name2"_s );
  cond2.insert( u"expression"_s, u"1 * 0"_s );
  conditions << cond2;
  config.insert( u"conditions"_s, conditions );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:condition"_s, config ) );
  QVERIFY( alg != nullptr );

  QCOMPARE( alg->outputDefinitions().size(), 2 );
  QCOMPARE( alg->outputDefinitions().at( 0 )->name(), u"name1"_s );
  QCOMPARE( alg->outputDefinitions().at( 1 )->name(), u"name2"_s );

  QVariantMap parameters;
  // vector input
  parameters.insert( u"INPUT"_s, u"vl"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok, config );
  QVERIFY( ok );
  QCOMPARE( results.value( u"name1"_s ).toInt(), 1 );
  QCOMPARE( results.value( u"name2"_s ).toInt(), 0 );
}

void TestQgsProcessingAlgsPt1::saveLog()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:savelog"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  feedback.reportError( u"test"_s );
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  QFile file( results.value( u"OUTPUT"_s ).toString() );
  QVERIFY( file.open( QFile::ReadOnly | QIODevice::Text ) );
  QCOMPARE( QString( file.readAll() ), u"test\n"_s );

  parameters.insert( u"USE_HTML"_s, true );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  QFile file2( results.value( u"OUTPUT"_s ).toString() );
  QVERIFY( file2.open( QFile::ReadOnly | QIODevice::Text ) );
  QCOMPARE( QString( file2.readAll() ), u"<span style=\"color:red\">test</span><br/>"_s );
}

void TestQgsProcessingAlgsPt1::setProjectVariable()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:setprojectvariable"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"NAME"_s, u"my_var"_s );
  parameters.insert( u"VALUE"_s, 11 );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsExpressionContextScope> scope( QgsExpressionContextUtils::projectScope( &p ) );
  QCOMPARE( scope->variable( u"my_var"_s ).toInt(), 11 );

  //overwrite existing
  parameters.insert( u"VALUE"_s, 13 );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  scope.reset( QgsExpressionContextUtils::projectScope( &p ) );
  QCOMPARE( scope->variable( u"my_var"_s ).toInt(), 13 );
}

QGSTEST_MAIN( TestQgsProcessingAlgsPt1 )
#include "testqgsprocessingalgspt1.moc"
