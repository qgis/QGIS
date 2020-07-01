/***************************************************************************
                         testqgsprocessingalgs.cpp
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
#include "limits"

#include "qgstest.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsnativealgorithms.h"
#include "qgsalgorithmfillnodata.h"
#include "qgsalgorithmlinedensity.h"
#include "qgsalgorithmimportphotos.h"
#include "qgsalgorithmtransform.h"
#include "qgsalgorithmkmeansclustering.h"
#include "qgsvectorlayer.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmultipolygon.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"
#include "qgsreclassifyutils.h"
#include "qgsalgorithmrasterlogicalop.h"
#include "qgsprintlayout.h"
#include "qgslayoutmanager.h"
#include "qgslayoutitemmap.h"
#include "qgsmarkersymbollayer.h"
#include "qgsrulebasedrenderer.h"
#include "qgspallabeling.h"
#include "qgsrastershader.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgslayoutitemscalebar.h"
#include "annotations/qgstextannotation.h"
#include "qgsfontutils.h"
#include "annotations/qgsannotationmanager.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsstyle.h"
#include "qgsbookmarkmanager.h"
#include "qgsexpressioncontextutils.h"

class TestQgsProcessingAlgs: public QObject
{
    Q_OBJECT

  private:

    /**
     * Helper function to get a feature based algorithm.
     */
    std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> featureBasedAlg( const QString &id );

    QgsFeature runForFeature( const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> &alg, QgsFeature feature, const QString &layerType, QVariantMap parameters = QVariantMap() );

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void packageAlg();
    void renameLayerAlg();
    void loadLayerAlg();
    void parseGeoTags();
    void featureFilterAlg();
    void transformAlg();
    void kmeansCluster();
    void categorizeByStyle();
    void extractBinary();
    void createDirectory();

    void polygonsToLines_data();
    void polygonsToLines();

    void createConstantRaster_data();
    void createConstantRaster();

    void densifyGeometries_data();
    void densifyGeometries();

    void fillNoData_data();
    void fillNoData();
    void lineDensity_data();
    void lineDensity();

    void rasterLogicOp_data();
    void rasterLogicOp();
    void cellStatistics_data();
    void cellStatistics();
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
    void exportLayoutPdf();
    void exportLayoutPng();

  private:

    QString mPointLayerPath;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;

};

std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> TestQgsProcessingAlgs::featureBasedAlg( const QString &id )
{
  return std::unique_ptr<QgsProcessingFeatureBasedAlgorithm>( static_cast<QgsProcessingFeatureBasedAlgorithm *>( QgsApplication::processingRegistry()->createAlgorithmById( id ) ) );
}

QgsFeature TestQgsProcessingAlgs::runForFeature( const std::unique_ptr< QgsProcessingFeatureBasedAlgorithm > &alg, QgsFeature feature, const QString &layerType, QVariantMap parameters )
{
  Q_ASSERT( alg.get() );
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;
  context->setFeedback( &feedback );

  std::unique_ptr<QgsVectorLayer> inputLayer( qgis::make_unique<QgsVectorLayer>( layerType, QStringLiteral( "layer" ), QStringLiteral( "memory" ) ) );
  inputLayer->dataProvider()->addFeature( feature );

  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue<QgsMapLayer *>( inputLayer.get() ) );

  parameters.insert( QStringLiteral( "OUTPUT" ), QStringLiteral( "memory:" ) );

  bool ok = false;
  auto res = alg->run( parameters, *context, &feedback, &ok );
  QgsFeature result;

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( res.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  outputLayer->getFeatures().nextFeature( result );

  return result;
}

void TestQgsProcessingAlgs::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QString pointsFileName = dataDir + "/points.shp";
  QFileInfo pointFileInfo( pointsFileName );
  mPointLayerPath = pointFileInfo.filePath();
  mPointsLayer = new QgsVectorLayer( mPointLayerPath,
                                     QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPointsLayer->isValid() );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  QString polysFileName = dataDir + "/polys.shp";
  QFileInfo polyFileInfo( polysFileName );
  mPolygonLayer = new QgsVectorLayer( polyFileInfo.filePath(),
                                      QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPolygonLayer );
  QVERIFY( mPolygonLayer->isValid() );
}

void TestQgsProcessingAlgs::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QVariantMap pkgAlg( const QStringList &layers, const QString &outputGpkg, bool overwrite, bool *ok )
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:package" ) ) );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYERS" ), layers );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputGpkg );
  parameters.insert( QStringLiteral( "OVERWRITE" ), overwrite );
  return package->run( parameters, *context, &feedback, ok );
}

void TestQgsProcessingAlgs::packageAlg()
{
  QString outputGpkg = QDir::tempPath() + "/package_alg.gpkg";

  if ( QFile::exists( outputGpkg ) )
    QFile::remove( outputGpkg );

  QVariantMap parameters;
  QStringList layers = QStringList() << mPointsLayer->id() << mPolygonLayer->id();
  bool ok = false;
  QVariantMap results = pkgAlg( layers, outputGpkg, true, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  std::unique_ptr< QgsVectorLayer > pointLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  QCOMPARE( pointLayer->wkbType(), mPointsLayer->wkbType() );
  QCOMPARE( pointLayer->featureCount(), mPointsLayer->featureCount() );
  pointLayer.reset();
  std::unique_ptr< QgsVectorLayer > polygonLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( polygonLayer->isValid() );
  QCOMPARE( polygonLayer->wkbType(), mPolygonLayer->wkbType() );
  QCOMPARE( polygonLayer->featureCount(), mPolygonLayer->featureCount() );
  polygonLayer.reset();

  std::unique_ptr<QgsVectorLayer> rectangles = qgis::make_unique<QgsVectorLayer>( QStringLiteral( TEST_DATA_DIR ) + "/rectangles.shp",
      QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << rectangles.get() );

  // Test adding an additional layer (overwrite disabled)
  QVariantMap results2 = pkgAlg( QStringList() << rectangles->id(), outputGpkg, false, &ok );
  QVERIFY( ok );

  QVERIFY( !results2.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  std::unique_ptr< QgsVectorLayer > rectanglesPackagedLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->wkbType(), rectanglesPackagedLayer->wkbType() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );
  rectanglesPackagedLayer.reset();

  pointLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  pointLayer.reset();

  // And finally, test with overwrite enabled
  QVariantMap results3 = pkgAlg( QStringList() << rectangles->id(), outputGpkg, true, &ok );
  QVERIFY( ok );

  QVERIFY( !results2.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  rectanglesPackagedLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->wkbType(), rectanglesPackagedLayer->wkbType() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );

  pointLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( !pointLayer->isValid() ); // It's gone -- the gpkg was recreated with a single layer
}

void TestQgsProcessingAlgs::renameLayerAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:renamelayer" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:real" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;

  // bad layer
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "bad layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  bool ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QCOMPARE( layer->name(), QStringLiteral( "layer" ) );

  //invalid name
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QString() );
  ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QCOMPARE( layer->name(), QStringLiteral( "layer" ) );

  //good params
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layer ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( layer->name(), QStringLiteral( "new name" ) );
  QCOMPARE( results.value( "OUTPUT" ), QVariant::fromValue( layer ) );

  // with input layer name as parameter
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "new name" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name2" ) );
  ok = false;
  results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( layer->name(), QStringLiteral( "new name2" ) );
  // result should use new name as value
  QCOMPARE( results.value( "OUTPUT" ).toString(), QStringLiteral( "new name2" ) );
}

void TestQgsProcessingAlgs::loadLayerAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:loadlayer" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;

  // bad layer
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "bad layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  bool ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( context->layersToLoadOnCompletion().empty() );

  //invalid name
  parameters.insert( QStringLiteral( "INPUT" ), mPointLayerPath );
  parameters.insert( QStringLiteral( "NAME" ), QString() );
  ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( context->layersToLoadOnCompletion().empty() );

  //good params
  parameters.insert( QStringLiteral( "INPUT" ), mPointLayerPath );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "my layer" ) );
  ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !context->layersToLoadOnCompletion().empty() );
  QString layerId = context->layersToLoadOnCompletion().keys().at( 0 );
  QCOMPARE( results.value( QStringLiteral( "OUTPUT" ) ).toString(), layerId );
  QVERIFY( !layerId.isEmpty() );
  QVERIFY( context->temporaryLayerStore()->mapLayer( layerId ) );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).name, QStringLiteral( "my layer" ) );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).project, &p );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).outputName, QStringLiteral( "my layer" ) );
}

void TestQgsProcessingAlgs::parseGeoTags()
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
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "abc" ).toString(), QStringLiteral( "abc" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "(abc)" ).toString(), QStringLiteral( "(abc)" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::parseMetadataValue( "abc (123)" ).toString(), QStringLiteral( "abc (123)" ) );
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
  QCOMPARE( md.value( "EXIF_Model" ).toString(), QStringLiteral( "Pixel" ) );
  QGSCOMPARENEAR( md.value( "EXIF_GPSLatitude" ).toDouble(), 36.220892, 0.000001 );
  QGSCOMPARENEAR( md.value( "EXIF_GPSLongitude" ).toDouble(), 149.131878, 0.000001 );

  // test extractGeoTagFromMetadata
  md = QVariantMap();
  QgsPointXY point;
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( QStringLiteral( "EXIF_GPSLongitude" ), 142.0 );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( QStringLiteral( "EXIF_GPSLatitude" ), 37.0 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitude" ), QStringLiteral( "x" ) );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( QStringLiteral( "EXIF_GPSLongitude" ), 142.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitude" ), QStringLiteral( "x" ) );
  QVERIFY( !QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  md.insert( QStringLiteral( "EXIF_GPSLatitude" ), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QStringLiteral( "E" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QStringLiteral( "W" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QStringLiteral( "w" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QStringLiteral( "...W" ) ); // apparently any string ENDING in W is acceptable
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QString() );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), -1 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), -142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLongitudeRef" ), QString() );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QStringLiteral( "N" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QStringLiteral( "S" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QStringLiteral( "s" ) );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QStringLiteral( "...S" ) ); // any string ending in s is acceptable
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), QString() );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), 37.0 );
  md.insert( QStringLiteral( "EXIF_GPSLatitudeRef" ), -1 );
  QVERIFY( QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( md, point ) );
  QCOMPARE( point.x(), 142.0 );
  QCOMPARE( point.y(), -37.0 );

  // extractAltitudeFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).isValid() );
  md.insert( QStringLiteral( "EXIF_GPSAltitude" ), 10.5 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), QStringLiteral( "0" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), QStringLiteral( "00" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), QStringLiteral( "1" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), QStringLiteral( "01" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), 1 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), 10.5 );
  md.insert( QStringLiteral( "EXIF_GPSAltitudeRef" ), -1 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( md ).toDouble(), -10.5 );

  // extractDirectionFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractDirectionFromMetadata( md ).isValid() );
  md.insert( QStringLiteral( "EXIF_GPSImgDirection" ), 15 );
  QCOMPARE( QgsImportPhotosAlgorithm::extractDirectionFromMetadata( md ).toDouble(), 15.0 );

  // extractTimestampFromMetadata
  QVERIFY( !QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).isValid() );
  md.insert( QStringLiteral( "EXIF_DateTimeOriginal" ), QStringLiteral( "xx" ) );
  QVERIFY( !QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).isValid() );
  md.insert( QStringLiteral( "EXIF_DateTimeOriginal" ), QStringLiteral( "2017:12:27 19:20:52" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );
  md.remove( QStringLiteral( "EXIF_DateTimeOriginal" ) );
  md.insert( QStringLiteral( "EXIF_DateTimeDigitized" ), QStringLiteral( "2017:12:27 19:20:52" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );
  md.remove( QStringLiteral( "EXIF_DateTimeDigitized" ) );
  md.insert( QStringLiteral( "EXIF_DateTime" ), QStringLiteral( "2017:12:27 19:20:52" ) );
  QCOMPARE( QgsImportPhotosAlgorithm::extractTimestampFromMetadata( md ).toDateTime(), QDateTime( QDate( 2017, 12, 27 ), QTime( 19, 20, 52 ) ) );

}

void TestQgsProcessingAlgs::featureFilterAlg()
{
  const QgsProcessingAlgorithm *filterAlgTemplate = QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:filter" ) );

  Q_ASSERT( filterAlgTemplate->outputDefinitions().isEmpty() );

  QVariantList outputs;
  QVariantMap output1;
  output1.insert( QStringLiteral( "name" ), QStringLiteral( "test" ) );
  output1.insert( QStringLiteral( "expression" ), QStringLiteral( "TRUE" ) );
  output1.insert( QStringLiteral( "isModelOutput" ), true );

  outputs.append( output1 );

  QVariantMap config1;
  config1.insert( QStringLiteral( "outputs" ), outputs );

  std::unique_ptr<QgsProcessingAlgorithm> filterAlg( filterAlgTemplate->create( config1 ) );

  QCOMPARE( filterAlg->outputDefinitions().size(), 1 );

  auto outputDef = filterAlg->outputDefinition( QStringLiteral( "OUTPUT_test" ) );
  QCOMPARE( outputDef->type(), QStringLiteral( "outputVector" ) );

  auto outputParamDef = filterAlg->parameterDefinition( "OUTPUT_test" );
  Q_ASSERT( outputParamDef->flags() & QgsProcessingParameterDefinition::FlagIsModelOutput );
  Q_ASSERT( outputParamDef->flags() & QgsProcessingParameterDefinition::FlagHidden );

  QVariantMap output2;
  output2.insert( QStringLiteral( "name" ), QStringLiteral( "nonmodeloutput" ) );
  output2.insert( QStringLiteral( "expression" ), QStringLiteral( "TRUE" ) );
  output2.insert( QStringLiteral( "isModelOutput" ), false );

  outputs.append( output2 );

  QVariantMap config2;
  config2.insert( QStringLiteral( "outputs" ), outputs );

  std::unique_ptr<QgsProcessingAlgorithm> filterAlg2( filterAlgTemplate->create( config2 ) );

  QCOMPARE( filterAlg2->outputDefinitions().size(), 2 );

  auto outputDef2 = filterAlg2->outputDefinition( QStringLiteral( "OUTPUT_nonmodeloutput" ) );
  QCOMPARE( outputDef2->type(), QStringLiteral( "outputVector" ) );

  auto outputParamDef2 = filterAlg2->parameterDefinition( "OUTPUT_nonmodeloutput" );
  Q_ASSERT( !outputParamDef2->flags().testFlag( QgsProcessingParameterDefinition::FlagIsModelOutput ) );
  Q_ASSERT( outputParamDef2->flags() & QgsProcessingParameterDefinition::FlagHidden );
}

void TestQgsProcessingAlgs::transformAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:reprojectlayer" ) ) );
  QVERIFY( alg != nullptr );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;

  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:4326field=col1:integer" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );
  QgsFeature f;
  // add a point with a bad geometry - this should result in a transform exception!
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -96215069, 41.673559 ) ) );
  QVERIFY( layer->dataProvider()->addFeature( f ) );
  p.addMapLayer( layer );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "test" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QStringLiteral( "memory:" ) );
  parameters.insert( QStringLiteral( "TARGET_CRS" ), QStringLiteral( "EPSG:2163" ) );
  bool ok = false;
  QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  Q_UNUSED( results )
  QVERIFY( ok );
}

void TestQgsProcessingAlgs::kmeansCluster()
{
  // make some features
  std::vector< QgsKMeansClusteringAlgorithm::Feature > features;
  std::vector< QgsPointXY > centers( 2 );

  // no features, no crash
  int k = 2;
  QgsKMeansClusteringAlgorithm::initClusters( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );

  // features < clusters
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 5 ) ) );
  QgsKMeansClusteringAlgorithm::initClusters( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[ 0 ].cluster, 0 );

  // features == clusters
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 11, 5 ) ) );
  QgsKMeansClusteringAlgorithm::initClusters( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[ 0 ].cluster, 1 );
  QCOMPARE( features[ 1 ].cluster, 0 );

  // features > clusters
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 13, 3 ) ) );
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 13, 13 ) ) );
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 23, 6 ) ) );
  k = 2;
  QgsKMeansClusteringAlgorithm::initClusters( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[ 0 ].cluster, 1 );
  QCOMPARE( features[ 1 ].cluster, 1 );
  QCOMPARE( features[ 2 ].cluster, 0 );
  QCOMPARE( features[ 3 ].cluster, 0 );
  QCOMPARE( features[ 4 ].cluster, 0 );

  // repeat above, with 3 clusters
  k = 3;
  centers.resize( 3 );
  QgsKMeansClusteringAlgorithm::initClusters( features, centers, k, nullptr );
  QgsKMeansClusteringAlgorithm::calculateKMeans( features, centers, k, nullptr );
  QCOMPARE( features[ 0 ].cluster, 1 );
  QCOMPARE( features[ 1 ].cluster, 2 );
  QCOMPARE( features[ 2 ].cluster, 2 );
  QCOMPARE( features[ 3 ].cluster, 2 );
  QCOMPARE( features[ 4 ].cluster, 0 );

  // with identical points
  features.clear();
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 5 ) ) );
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 5 ) ) );
  features.emplace_back( QgsKMeansClusteringAlgorithm::Feature( QgsPointXY( 1, 5 ) ) );
  QCOMPARE( features[ 0 ].cluster, -1 );
  QCOMPARE( features[ 1 ].cluster, -1 );
  QCOMPARE( features[ 2 ].cluster, -1 );
}

void TestQgsProcessingAlgs::categorizeByStyle()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:categorizeusingstyle" ) ) );
  QVERIFY( alg != nullptr );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString styleFileName = dataDir + "/categorized.xml";


  QgsProcessingFeedback feedback;

  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:4326&field=col1:string" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
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
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "test" ) );
  parameters.insert( QStringLiteral( "FIELD" ), QStringLiteral( "col1" ) );
  parameters.insert( QStringLiteral( "STYLE" ), styleFileName );
  parameters.insert( QStringLiteral( "CASE_SENSITIVE" ), true );
  parameters.insert( QStringLiteral( "TOLERANT" ), false );
  parameters.insert( QStringLiteral( "NON_MATCHING_CATEGORIES" ), QStringLiteral( "memory:" ) );
  parameters.insert( QStringLiteral( "NON_MATCHING_SYMBOLS" ), QStringLiteral( "memory:" ) );

  bool ok = false;
  QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context->layerToLoadOnCompletionDetails( layer->id() ).postProcessor()->postProcessLayer( layer, *context, &feedback );
  QgsCategorizedSymbolRenderer *catRenderer = dynamic_cast< QgsCategorizedSymbolRenderer * >( layer->renderer() );
  QVERIFY( catRenderer );

  auto allValues = []( QgsVectorLayer * layer )->QStringList
  {
    QStringList all;
    QgsFeature f;
    QgsFeatureIterator it = layer->getFeatures();
    while ( it.nextFeature( f ) )
    {
      all.append( f.attribute( 0 ).toString() );
    }
    return all;
  };
  QgsVectorLayer *nonMatchingCats = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "NON_MATCHING_CATEGORIES" ) ).toString() ) );
  QCOMPARE( allValues( nonMatchingCats ), QStringList() << "b" << "c " );
  QgsVectorLayer *nonMatchingSymbols = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "NON_MATCHING_SYMBOLS" ) ).toString() ) );
  QCOMPARE( allValues( nonMatchingSymbols ), QStringList() << " ----c/- " << "B " );

  QCOMPARE( catRenderer->categories().count(), 3 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "a" ) ) ).symbol()->color().name(), QStringLiteral( "#ff0000" ) );
  QVERIFY( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "b" ) ) ).symbol()->color().name() != QStringLiteral( "#00ff00" ) );
  QVERIFY( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "c " ) ) ).symbol()->color().name() != QStringLiteral( "#0000ff" ) );
  // reset renderer
  layer->setRenderer( new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );

  // case insensitive
  parameters.insert( QStringLiteral( "CASE_SENSITIVE" ), false );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context->layerToLoadOnCompletionDetails( layer->id() ).postProcessor()->postProcessLayer( layer, *context, &feedback );
  catRenderer = dynamic_cast< QgsCategorizedSymbolRenderer * >( layer->renderer() );
  QVERIFY( catRenderer );

  nonMatchingCats = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "NON_MATCHING_CATEGORIES" ) ).toString() ) );
  QCOMPARE( allValues( nonMatchingCats ), QStringList() << "c " );
  nonMatchingSymbols = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "NON_MATCHING_SYMBOLS" ) ).toString() ) );
  QCOMPARE( allValues( nonMatchingSymbols ), QStringList() << " ----c/- " );

  QCOMPARE( catRenderer->categories().count(), 3 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "a" ) ) ).symbol()->color().name(), QStringLiteral( "#ff0000" ) );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "b" ) ) ).symbol()->color().name(), QStringLiteral( "#00ff00" ) );
  QVERIFY( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "c " ) ) ).symbol()->color().name() != QStringLiteral( "#0000ff" ) );
  // reset renderer
  layer->setRenderer( new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );

  // tolerant
  parameters.insert( QStringLiteral( "CASE_SENSITIVE" ), true );
  parameters.insert( QStringLiteral( "TOLERANT" ), true );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context->layerToLoadOnCompletionDetails( layer->id() ).postProcessor()->postProcessLayer( layer, *context, &feedback );
  catRenderer = dynamic_cast< QgsCategorizedSymbolRenderer * >( layer->renderer() );
  QVERIFY( catRenderer );

  nonMatchingCats = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "NON_MATCHING_CATEGORIES" ) ).toString() ) );
  QCOMPARE( allValues( nonMatchingCats ), QStringList() << "b" );
  nonMatchingSymbols = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "NON_MATCHING_SYMBOLS" ) ).toString() ) );
  QCOMPARE( allValues( nonMatchingSymbols ), QStringList() << "B " );

  QCOMPARE( catRenderer->categories().count(), 3 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "a" ) ) ).symbol()->color().name(), QStringLiteral( "#ff0000" ) );
  QVERIFY( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "b" ) ) ).symbol()->color().name() != QStringLiteral( "#00ff00" ) );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "c " ) ) ).symbol()->color().name(), QStringLiteral( "#0000ff" ) );
  // reset renderer
  layer->setRenderer( new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );

  // no optional sinks
  parameters.insert( QStringLiteral( "CASE_SENSITIVE" ), false );
  parameters.remove( QStringLiteral( "NON_MATCHING_CATEGORIES" ) );
  parameters.remove( QStringLiteral( "NON_MATCHING_SYMBOLS" ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context->layerToLoadOnCompletionDetails( layer->id() ).postProcessor()->postProcessLayer( layer, *context, &feedback );
  catRenderer = dynamic_cast< QgsCategorizedSymbolRenderer * >( layer->renderer() );
  QVERIFY( catRenderer );

  QVERIFY( !context->getMapLayer( results.value( QStringLiteral( "NON_MATCHING_CATEGORIES" ) ).toString() ) );
  QVERIFY( !context->getMapLayer( results.value( QStringLiteral( "NON_MATCHING_SYMBOLS" ) ).toString() ) );

  QCOMPARE( catRenderer->categories().count(), 3 );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "a" ) ) ).symbol()->color().name(), QStringLiteral( "#ff0000" ) );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "b" ) ) ).symbol()->color().name(), QStringLiteral( "#00ff00" ) );
  QCOMPARE( catRenderer->categories().at( catRenderer->categoryIndexForValue( QStringLiteral( "c " ) ) ).symbol()->color().name(), QStringLiteral( "#0000ff" ) );
}

void TestQgsProcessingAlgs::extractBinary()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:extractbinary" ) ) );
  QVERIFY( alg != nullptr );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString source = dataDir + QStringLiteral( "/attachments.gdb|layername=points__ATTACH" );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), source );
  parameters.insert( QStringLiteral( "FIELD" ), QStringLiteral( "DATA" ) );
  parameters.insert( QStringLiteral( "FILENAME" ), QStringLiteral( "'test' || \"ATTACHMENTID\" || '.jpg'" ) );
  const QString folder = QDir::tempPath();
  parameters.insert( QStringLiteral( "FOLDER" ), folder );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.value( QStringLiteral( "FOLDER" ) ).toString(), folder );

  QFile file( folder + "/test1.jpg" );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  QByteArray d = file.readAll();
  QCOMPARE( QString( QCryptographicHash::hash( d, QCryptographicHash::Md5 ).toHex() ), QStringLiteral( "ef3dbc530cc39a545832a6c82aac57b6" ) );

  QFile file2( folder + "/test2.jpg" );
  QVERIFY( file2.open( QIODevice::ReadOnly ) );
  d = file2.readAll();
  QCOMPARE( QString( QCryptographicHash::hash( d, QCryptographicHash::Md5 ).toHex() ), QStringLiteral( "4b952b80e4288ca5111be2f6dd5d6809" ) );
}

void TestQgsProcessingAlgs::createDirectory()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:createdirectory" ) ) );
  QVERIFY( alg != nullptr );

  // first make a path to an existing file
  QString outputPath = QDir::tempPath() + "/test.txt";
  if ( QFile::exists( outputPath ) )
    QFile::remove( outputPath );
  QFile file( outputPath );
  file.open( QIODevice::ReadWrite );
  file.close();

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "PATH" ), outputPath );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // path is an existing file
  QVERIFY( !ok );

  outputPath = QDir::tempPath() + "/createdir/test/test2";
  QDir().rmdir( QDir::tempPath() + "/createdir/test/test2" );
  QDir().rmdir( QDir::tempPath() + "/createdir/test" );
  QDir().rmdir( QDir::tempPath() + "/createdir" );

  parameters.insert( QStringLiteral( "PATH" ), outputPath );
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


void TestQgsProcessingAlgs::polygonsToLines_data()
{
  QTest::addColumn<QgsGeometry>( "sourceGeometry" );
  QTest::addColumn<QgsGeometry>( "expectedGeometry" );

  QTest::newRow( "Simple Polygon" )
      << QgsGeometry::fromWkt( "Polygon((1 1, 2 2, 1 3, 1 1))" )
      << QgsGeometry::fromWkt( "MultiLineString ((1 1, 2 2, 1 3, 1 1))" );

  QgsGeometry geomNoRing( qgis::make_unique<QgsMultiPolygon>() );

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


void TestQgsProcessingAlgs::polygonsToLines()
{
  QFETCH( QgsGeometry, sourceGeometry );
  QFETCH( QgsGeometry, expectedGeometry );

  std::unique_ptr< QgsProcessingFeatureBasedAlgorithm > alg( featureBasedAlg( "native:polygonstolines" ) );

  QgsFeature feature;
  feature.setGeometry( sourceGeometry );

  QgsFeature result = runForFeature( alg, feature, QStringLiteral( "Polygon" ) );

  QVERIFY2( result.geometry().equals( expectedGeometry ), QStringLiteral( "Result: %1, Expected: %2" ).arg( result.geometry().asWkt(), expectedGeometry.asWkt() ).toUtf8().constData() );
}

Q_DECLARE_METATYPE( Qgis::DataType )
void TestQgsProcessingAlgs::createConstantRaster_data()
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
      << QStringLiteral( "/createConstantRaster_testcase1.tif" )
      << Qgis::Byte
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
      << QStringLiteral( "" )
      << Qgis::Byte
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
      << QStringLiteral( "" )
      << Qgis::Byte
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
      << QStringLiteral( "/createConstantRaster_testcase4.tif" )
      << Qgis::Int16
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
      << QStringLiteral( "" )
      << Qgis::Int16
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
      << QStringLiteral( "" )
      << Qgis::Int16
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
      << QStringLiteral( "/createConstantRaster_testcase7.tif" )
      << Qgis::UInt16
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
      << QStringLiteral( "" )
      << Qgis::UInt16
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
      << QStringLiteral( "" )
      << Qgis::UInt16
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
      << QStringLiteral( "/createConstantRaster_testcase10.tif" )
      << Qgis::Int32
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
      << QStringLiteral( "/createConstantRaster_testcase10.tif" )
      << Qgis::Int32
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
      << QStringLiteral( "" )
      << Qgis::Int32
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
      << QStringLiteral( "" )
      << Qgis::Int32
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
      << QStringLiteral( "/createConstantRaster_testcase13.tif" )
      << Qgis::UInt32
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
      << QStringLiteral( "" )
      << Qgis::UInt32
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
      << QStringLiteral( "" )
      << Qgis::UInt32
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
      << QStringLiteral( "/createConstantRaster_testcase16.tif" )
      << Qgis::Float32
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
      << QStringLiteral( "/createConstantRaster_testcase17.tif" )
      << Qgis::Float64
      << "EPSG:4326"
      << 1.0
      << 12.125789212532487
      << 6;


}

void TestQgsProcessingAlgs::createConstantRaster()
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
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:createconstantrasterlayer" ) ) );

  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  //set project crs and ellipsoid from input layer
  p.setCrs( QgsCoordinateReferenceSystem( crs ), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( QStringLiteral( "EXTENT" ), inputExtent );
  parameters.insert( QStringLiteral( "TARGET_CRS" ), QgsCoordinateReferenceSystem( crs ) );
  parameters.insert( QStringLiteral( "PIXEL_SIZE" ), pixelSize );
  parameters.insert( QStringLiteral( "NUMBER" ), constantValue );
  parameters.insert( QStringLiteral( "OUTPUT_TYPE" ), typeId );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

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
    std::unique_ptr<QgsRasterLayer> expectedRasterLayer = qgis::make_unique< QgsRasterLayer >( myDataPath + "/control_images/expected_constantRaster" + expectedRaster, "expectedDataset", "gdal" );
    std::unique_ptr< QgsRasterInterface > expectedInterface( expectedRasterLayer->dataProvider()->clone() );
    QgsRasterIterator expectedIter( expectedInterface.get() );
    expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

    //run alg...
    results = alg->run( parameters, *context, &feedback, &ok );
    QVERIFY( ok );

    //...and check results with expected datasets
    std::unique_ptr<QgsRasterLayer> outputRaster = qgis::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
    std::unique_ptr< QgsRasterInterface > outputInterface( outputRaster->dataProvider()->clone() );

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

    std::unique_ptr< QgsRasterBlock > outputRasterBlock;
    std::unique_ptr< QgsRasterBlock > expectedRasterBlock;

    while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) &&
            expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
    {
      for ( int row = 0; row < expectedIterRows; row++ )
      {
        for ( int column = 0; column < expectedIterCols; column++ )
        {
          double expectedValue = expectedRasterBlock->value( row, column );
          double outputValue = outputRasterBlock->value( row, column );
          QCOMPARE( outputValue, expectedValue );
        }
      }
    }
  }
}


void TestQgsProcessingAlgs::densifyGeometries_data()
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

void TestQgsProcessingAlgs::densifyGeometries()
{
  QFETCH( QgsGeometry, sourceGeometry );
  QFETCH( QgsGeometry, expectedGeometry );
  QFETCH( double, interval );
  QFETCH( QString, geometryType );

  std::unique_ptr< QgsProcessingFeatureBasedAlgorithm > alg( featureBasedAlg( "native:densifygeometriesgivenaninterval" ) );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INTERVAL" ), interval );

  QgsFeature feature;
  feature.setGeometry( sourceGeometry );

  QgsFeature result = runForFeature( alg, feature, geometryType, parameters );

  if ( expectedGeometry.isNull() )
    QVERIFY( result.geometry().isNull() );
  else
    QVERIFY2( result.geometry().equals( expectedGeometry ), QStringLiteral( "Result: %1, Expected: %2" ).arg( result.geometry().asWkt(), expectedGeometry.asWkt() ).toUtf8().constData() );
}

void TestQgsProcessingAlgs::fillNoData_data()
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
      << QStringLiteral( "/fillnodata_testcase1.tif" )
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
      << QStringLiteral( "/fillnodata_testcase2.tif" )
      << 1
      << 1.8;

  /*
   * Testcase 3
   *
   * WGS84 data without weights
   * searchRadius = 3
   * pixelSize = 1.8
   */
  QTest::newRow( "testcase 2" )
      << "/raster/band1_float32_noct_epsg4326.tif"
      << QStringLiteral( "/fillnodata_testcase3.tif" )
      << 1
      << 1.8;

}

void TestQgsProcessingAlgs::fillNoData()
{
  QFETCH( QString, inputRaster );
  QFETCH( QString, expectedRaster );
  QFETCH( int, inputBand );
  QFETCH( double, fillValue );

  //prepare input params
  QgsProject p;
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fillnodata" ) ) );

  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  std::unique_ptr<QgsRasterLayer> inputRasterLayer = qgis::make_unique< QgsRasterLayer >( myDataPath + inputRaster, "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( QStringLiteral( "INPUT" ), myDataPath + inputRaster );
  parameters.insert( QStringLiteral( "BAND" ), inputBand );
  parameters.insert( QStringLiteral( "FILL_VALUE" ), fillValue );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  std::unique_ptr<QgsRasterLayer> expectedRasterLayer = qgis::make_unique< QgsRasterLayer >( myDataPath + "/control_images/fillNoData/" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr< QgsRasterInterface > expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  std::unique_ptr<QgsRasterLayer> outputRaster = qgis::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
  std::unique_ptr< QgsRasterInterface > outputInterface( outputRaster->dataProvider()->clone() );

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

  std::unique_ptr< QgsRasterBlock > outputRasterBlock;
  std::unique_ptr< QgsRasterBlock > expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) &&
          expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        double expectedValue = expectedRasterBlock->value( row, column );
        double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );
      }
    }
  }
}

void TestQgsProcessingAlgs::lineDensity_data()
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
      << QStringLiteral( "/linedensity_testcase1.tif" )
      << 3.0
      << 2.0
      << QStringLiteral( "weight" );

  /*
   * Testcase 2
   *
   * WGS84 data without weights
   * searchRadius = 3
   * pixelSize = 2
   */
  QTest::newRow( "testcase_2" )
      << "/linedensity.gml"
      << QStringLiteral( "/linedensity_testcase2.tif" )
      << 3.0
      << 2.0
      << QStringLiteral( "" );

}

void TestQgsProcessingAlgs::lineDensity()
{
  QFETCH( QString, inputDataset );
  QFETCH( QString, expectedDataset );
  QFETCH( double, searchRadius );
  QFETCH( double, pixelSize );
  QFETCH( QString, weightField );

  //prepare input params
  QgsProject p;
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:linedensity" ) ) );

  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QgsVectorLayer *layer = new QgsVectorLayer( myDataPath + inputDataset + "|layername=linedensity", QStringLiteral( "layer" ), QStringLiteral( "ogr" ) );
  p.addMapLayer( layer );
  QVERIFY( layer->isValid() );

  //set project crs and ellipsoid from input layer
  p.setCrs( layer->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layer ) );
  parameters.insert( QStringLiteral( "WEIGHT" ), weightField );
  parameters.insert( QStringLiteral( "RADIUS" ), searchRadius );
  parameters.insert( QStringLiteral( "PIXEL_SIZE" ), pixelSize );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  std::unique_ptr<QgsRasterLayer> expectedRaster = qgis::make_unique< QgsRasterLayer >( myDataPath + "/control_images/expected_raster_linedensity" + expectedDataset, "expectedDataset", "gdal" );
  std::unique_ptr< QgsRasterInterface > expectedInterface( expectedRaster->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRaster->width(), expectedRaster->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  std::unique_ptr<QgsRasterLayer> outputRaster = qgis::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
  std::unique_ptr< QgsRasterInterface > outputInterface( outputRaster->dataProvider()->clone() );

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

  std::unique_ptr< QgsRasterBlock > outputRasterBlock;
  std::unique_ptr< QgsRasterBlock > expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) &&
          expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        double expectedValue = expectedRasterBlock->value( row, column );
        double outputValue = outputRasterBlock->value( row, column );
        QGSCOMPARENEAR( outputValue, expectedValue, 0.000000000015 );
      }
    }
  }
}

void TestQgsProcessingAlgs::rasterLogicOp_data()
{
  QTest::addColumn<QVector< double >>( "input1" );
  QTest::addColumn<QVector< double >>( "input2" );
  QTest::addColumn<QVector< double >>( "input3" );
  QTest::addColumn<bool>( "treatNodataAsFalse" );
  QTest::addColumn<qgssize>( "expectedOrNoDataCount" );
  QTest::addColumn<qgssize>( "expectedOrTrueCount" );
  QTest::addColumn<qgssize>( "expectedOrFalseCount" );
  QTest::addColumn<QVector< double >>( "expectedOr" );
  QTest::addColumn<qgssize>( "expectedAndNoDataCount" );
  QTest::addColumn<qgssize>( "expectedAndTrueCount" );
  QTest::addColumn<qgssize>( "expectedAndFalseCount" );
  QTest::addColumn<QVector< double >>( "expectedAnd" );
  QTest::addColumn<int>( "nRows" );
  QTest::addColumn<int>( "nCols" );
  QTest::addColumn<double>( "destNoDataValue" );
  QTest::addColumn<int>( "dataType" );

  QTest::newRow( "no nodata" ) << QVector< double > { 1, 2, 0, 0, 0, 0 }
                               << QVector< double > { 1, 0, 1, 1, 0, 1 }
                               << QVector< double > { 1, 2, 0, 0, 0, -1 }
                               << false
                               << 0ULL << 5ULL << 1ULL
                               << QVector< double > { 1, 1, 1, 1, 0, 1 }
                               << 0ULL << 1ULL << 5ULL
                               << QVector< double > { 1, 0, 0, 0, 0, 0 }
                               << 3 << 2
                               << -9999.0 << static_cast< int >( Qgis::Float32 );
  QTest::newRow( "nodata" ) << QVector< double > { 1, -9999, 0, 0, 0, 0 }
                            << QVector< double > { 1, 0, 1, 1, 0, 1 }
                            << QVector< double > { 1, 2, 0, -9999, 0, -1 }
                            << false
                            << 2ULL << 3ULL << 1ULL
                            << QVector< double > { 1, -9999, 1, -9999, 0, 1 }
                            << 2ULL << 1ULL << 3ULL
                            << QVector< double > { 1, -9999, 0, -9999, 0, 0 }
                            << 3 << 2
                            << -9999.0 << static_cast< int >( Qgis::Float32 );
  QTest::newRow( "nodata as false" ) << QVector< double > { 1, -9999, 0, 0, 0, 0 }
                                     << QVector< double > { 1, 0, 1, 1, 0, 1 }
                                     << QVector< double > { 1, 2, 0, -9999, 0, -1 }
                                     << true
                                     << 0ULL << 5ULL << 1ULL
                                     << QVector< double > { 1, 1, 1, 1, 0, 1 }
                                     << 0ULL << 1ULL << 5ULL
                                     << QVector< double > { 1, 0, 0, 0, 0, 0 }
                                     << 3 << 2
                                     << -9999.0 << static_cast< int >( Qgis::Float32 );
  QTest::newRow( "missing block 1" ) << QVector< double > {}
                                     << QVector< double > { 1, 0, 1, 1, 0, 1 }
                                     << QVector< double > { 1, 2, 0, -9999, 0, -1 }
                                     << false
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector< double > { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector< double > { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 3 << 2
                                     << -9999.0 << static_cast< int >( Qgis::Float32 );
  QTest::newRow( "missing block 1 nodata as false" ) << QVector< double > {}
      << QVector< double > { 1, 0, 1, 1, 0, 1 }
      << QVector< double > { 1, 2, 0, -9999, 0, -1 }
      << true
      << 0ULL << 5ULL << 1ULL
      << QVector< double > { 1, 1, 1, 1, 0, 1 }
      << 0ULL << 0ULL << 6ULL
      << QVector< double > { 0, 0, 0, 0, 0, 0 }
      << 3 << 2
      << -9999.0 << static_cast< int >( Qgis::Float32 );
  QTest::newRow( "missing block 2" ) << QVector< double > { 1, 0, 1, 1, 0, 1 }
                                     << QVector< double > {}
                                     << QVector< double > { 1, 2, 0, -9999, 0, -1 }
                                     << false
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector< double > { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector< double > { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 3 << 2
                                     << -9999.0 << static_cast< int >( Qgis::Float32 );
  QTest::newRow( "missing block 2 nodata as false" ) << QVector< double > { 1, 0, 1, 1, 0, 1 }
      << QVector< double > {}
      << QVector< double > { 1, 2, 0, -9999, 0, -1 }
      << true
      << 0ULL << 5ULL << 1ULL
      << QVector< double > { 1, 1, 1, 1, 0, 1 }
      << 0ULL << 0ULL << 6ULL
      << QVector< double > { 0, 0, 0, 0, 0, 0 }
      << 3 << 2
      << -9999.0 << static_cast< int >( Qgis::Float32 );
  QTest::newRow( "missing block 3" ) << QVector< double > { 1, 0, 1, 1, 0, 1 }
                                     << QVector< double > { 1, 2, 0, -9999, 0, -1 }
                                     << QVector< double > {}
                                     << false
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector< double > { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 6ULL << 0ULL << 0ULL
                                     << QVector< double > { -9999, -9999, -9999, -9999, -9999, -9999 }
                                     << 3 << 2
                                     << -9999.0 << static_cast< int >( Qgis::Float32 );
  QTest::newRow( "missing block 3 nodata as false" ) << QVector< double > { 1, 0, 1, 1, 0, 1 }
      << QVector< double > { 1, 2, 0, -9999, 0, -1 }
      << QVector< double > {}
      << true
      << 0ULL << 5ULL << 1ULL
      << QVector< double > { 1, 1, 1, 1, 0, 1 }
      << 0ULL << 0ULL << 6ULL
      << QVector< double > { 0, 0, 0, 0, 0, 0 }
      << 3 << 2
      << -9999.0 << static_cast< int >( Qgis::Float32 );
}

void TestQgsProcessingAlgs::rasterLogicOp()
{
  QFETCH( QVector< double >, input1 );
  QFETCH( QVector< double >, input2 );
  QFETCH( QVector< double >, input3 );
  QVector< QVector< double > > input;
  input << input1 << input2 << input3;
  QFETCH( bool, treatNodataAsFalse );
  QFETCH( qgssize, expectedOrNoDataCount );
  QFETCH( qgssize, expectedOrTrueCount );
  QFETCH( qgssize, expectedOrFalseCount );
  QFETCH( QVector< double >, expectedOr );
  QFETCH( qgssize, expectedAndNoDataCount );
  QFETCH( qgssize, expectedAndTrueCount );
  QFETCH( qgssize, expectedAndFalseCount );
  QFETCH( QVector< double >, expectedAnd );
  QFETCH( int, nRows );
  QFETCH( int, nCols );
  QFETCH( double, destNoDataValue );
  QFETCH( int, dataType );

  QgsRasterLogicalOrAlgorithm orAlg;
  QgsRasterLogicalAndAlgorithm andAlg;

  QgsRectangle extent = QgsRectangle( 0, 0, nRows, nCols );
  QgsRectangle badExtent = QgsRectangle( -100, -100, 90, 90 );
  QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:3857" ) );
  double tform[] =
  {
    extent.xMinimum(), extent.width() / nCols, 0.0,
    extent.yMaximum(), 0.0, -extent.height() / nRows
  };

  std::vector< QgsRasterAnalysisUtils::RasterLogicInput > inputs;
  for ( int ii = 0; ii < 3; ++ii )
  {
    // generate unique filename (need to open the file first to generate it)
    QTemporaryFile tmpFile;
    tmpFile.open();
    tmpFile.close();

    // create a GeoTIFF - this will create data provider in editable mode
    QString filename = tmpFile.fileName();

    std::unique_ptr< QgsRasterFileWriter > writer = qgis::make_unique< QgsRasterFileWriter >( filename );
    writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
    writer->setOutputFormat( QStringLiteral( "GTiff" ) );
    std::unique_ptr<QgsRasterDataProvider > dp( writer->createOneBandRaster( Qgis::Float32, nCols, nRows, input[ii].empty() ? badExtent : extent, crs ) );
    QVERIFY( dp->isValid() );
    dp->setNoDataValue( 1, -9999 );
    std::unique_ptr< QgsRasterBlock > block( dp->block( 1, input[ii].empty() ? badExtent : extent, nCols, nRows ) );
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
  tmpFile2.open();
  tmpFile2.close();

  // create a GeoTIFF - this will create data provider in editable mode
  QString filename = tmpFile2.fileName();
  std::unique_ptr< QgsRasterDataProvider > dpOr( QgsRasterDataProvider::create( QStringLiteral( "gdal" ), filename, QStringLiteral( "GTiff" ), 1, static_cast< Qgis::DataType >( dataType ), 10, 10, tform, crs ) );
  QVERIFY( dpOr->isValid() );

  // make destination AND raster
  QTemporaryFile tmpFile3;
  tmpFile3.open();
  tmpFile3.close();

  // create a GeoTIFF - this will create data provider in editable mode
  filename = tmpFile3.fileName();
  std::unique_ptr< QgsRasterDataProvider > dpAnd( QgsRasterDataProvider::create( QStringLiteral( "gdal" ), filename, QStringLiteral( "GTiff" ), 1, static_cast< Qgis::DataType >( dataType ), 10, 10, tform, crs ) );
  QVERIFY( dpAnd->isValid() );

  QgsFeedback feedback;
  qgssize noDataCount = 0;
  qgssize trueCount = 0;
  qgssize falseCount = 0;
  QgsRasterAnalysisUtils::applyRasterLogicOperator( inputs, dpOr.get(), destNoDataValue, treatNodataAsFalse, nCols, nRows,
      extent, &feedback, orAlg.mExtractValFunc, noDataCount, trueCount, falseCount );

  QCOMPARE( noDataCount, expectedOrNoDataCount );
  QCOMPARE( trueCount, expectedOrTrueCount );
  QCOMPARE( falseCount, expectedOrFalseCount );

  // read back in values
  std::unique_ptr< QgsRasterBlock > block( dpOr->block( 1, extent, nCols, nRows ) );
  QVector< double > res( nCols * nRows );
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
  QgsRasterAnalysisUtils::applyRasterLogicOperator( inputs, dpAnd.get(), destNoDataValue, treatNodataAsFalse, nCols, nRows,
      extent, &feedback, andAlg.mExtractValFunc, noDataCount, trueCount, falseCount );

  QCOMPARE( noDataCount, expectedAndNoDataCount );
  QCOMPARE( trueCount, expectedAndTrueCount );
  QCOMPARE( falseCount, expectedAndFalseCount );

  // read back in values
  block.reset( dpAnd->block( 1, extent, nCols, nRows ) );
  QVector< double > res2( nCols * nRows );
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

void TestQgsProcessingAlgs::cellStatistics_data()
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
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 0
      << false
      << QStringLiteral( "/cellstatistics_sum_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 2: count
   */
  QTest::newRow( "testcase_2" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 1
      << false
      << QStringLiteral( "/cellstatistics_count_result.tif" )
      << Qgis::Int32;

  /*
   * Testcase 3: mean
   */
  QTest::newRow( "testcase_3" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 2
      << false
      << QStringLiteral( "/cellstatistics_mean_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 4: median
   */
  QTest::newRow( "testcase_4" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 3
      << false
      << QStringLiteral( "/cellstatistics_median_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 5: Standard deviation
   */
  QTest::newRow( "testcase_5" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 4
      << false
      << QStringLiteral( "/cellstatistics_stddev_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 6: Variance
   */
  QTest::newRow( "testcase_6" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 5
      << false
      << QStringLiteral( "/cellstatistics_variance_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 7: Minimum
   */
  QTest::newRow( "testcase_7" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 6
      << false
      << QStringLiteral( "/cellstatistics_min_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 8: Maximum
   */
  QTest::newRow( "testcase_8" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 7
      << false
      << QStringLiteral( "/cellstatistics_max_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 9: Minority
   */
  QTest::newRow( "testcase_9" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 8
      << false
      << QStringLiteral( "/cellstatistics_minority_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 10: Majority
   */
  QTest::newRow( "testcase_10" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 9
      << false
      << QStringLiteral( "/cellstatistics_majority_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 11: Range
   */
  QTest::newRow( "testcase_11" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 10
      << false
      << QStringLiteral( "/cellstatistics_range_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 12: Variety
   */
  QTest::newRow( "testcase_12" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 11
      << false
      << QStringLiteral( "/cellstatistics_variety_result.tif" )
      << Qgis::Int32;

  /*
   * Testcase 13: Sum (integer)
   */
  QTest::newRow( "testcase_13" )
      << QStringList( {"/raster/statisticsRas1_int32.tif", "/raster/statisticsRas2_int32.tif", "/raster/statisticsRas3_int32.tif"} )
      << QStringLiteral( "/raster/statisticsRas1_int32.tif" )
      << 0
      << false
      << QStringLiteral( "/cellstatistics_sum_result_int32.tif" )
      << Qgis::Int32;

  /*
   * Testcase 14: sum (ignore nodata)
   */
  QTest::newRow( "testcase_14" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 0
      << true
      << QStringLiteral( "/cellstatistics_sum_ignore_nodata_result.tif" )
      << Qgis::Float64;

  /*
   * Testcase 15: mean
   */
  QTest::newRow( "testcase_15" )
      << QStringList( {"/raster/statisticsRas1_float64.asc", "/raster/statisticsRas2_float64.asc", "/raster/statisticsRas3_float64.asc"} )
      << QStringLiteral( "/raster/statisticsRas1_float64.asc" )
      << 2
      << true
      << QStringLiteral( "/cellstatistics_mean_ignore_nodata_result.tif" )
      << Qgis::Float64;
}

void TestQgsProcessingAlgs::cellStatistics()
{
  QFETCH( QStringList, inputRasters );
  QFETCH( QString, referenceLayer );
  QFETCH( int, statistic );
  QFETCH( bool, ignoreNoData );
  QFETCH( QString, expectedRaster );
  QFETCH( Qgis::DataType, expectedDataType );


  //prepare input params
  QgsProject p;
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:cellstatistics" ) ) );

  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QStringList inputDatasetPaths;

  for ( const auto &raster : inputRasters )
  {
    inputDatasetPaths << myDataPath + raster;
  }

  std::unique_ptr<QgsRasterLayer> inputRasterLayer1 = qgis::make_unique< QgsRasterLayer >( myDataPath + inputRasters[0], "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer1->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( QStringLiteral( "INPUT" ), inputDatasetPaths );
  parameters.insert( QStringLiteral( "STATISTIC" ), statistic );
  parameters.insert( QStringLiteral( "IGNORE_NODATA" ), ignoreNoData );
  parameters.insert( QStringLiteral( "REFERENCE_LAYER" ), myDataPath + referenceLayer );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  std::unique_ptr<QgsRasterLayer> expectedRasterLayer = qgis::make_unique< QgsRasterLayer >( myDataPath + "/control_images/expected_cellStatistics/" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr< QgsRasterInterface > expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  std::unique_ptr<QgsRasterLayer> outputRaster = qgis::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
  std::unique_ptr< QgsRasterInterface > outputInterface( outputRaster->dataProvider()->clone() );

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

  std::unique_ptr< QgsRasterBlock > outputRasterBlock;
  std::unique_ptr< QgsRasterBlock > expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) &&
          expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        double expectedValue = expectedRasterBlock->value( row, column );
        double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );
      }
    }
  }
}


void TestQgsProcessingAlgs::roundRasterValues_data()
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
      << QStringLiteral( "/roundRasterValues_testcase1.tif" ) //no output expected: can't round integer
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
      << QStringLiteral( "/roundRasterValues_testcase2.tif" )
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
      << QStringLiteral( "/roundRasterValues_testcase3.tif" )
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
      << QStringLiteral( "/roundRasterValues_testcase4.tif" )
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
      << QStringLiteral( "/roundRasterValues_testcase5.tif" )
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
      << QStringLiteral( "/roundRasterValues_testcase6.tif" )
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
      << QStringLiteral( "/roundRasterValues_testcase7.tif" )
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
      << QStringLiteral( "/roundRasterValues_testcase8.tif" )
      << 1
      << 1
      << -1
      << 10;

}

void TestQgsProcessingAlgs::roundRasterValues()
{
  QFETCH( QString, inputRaster );
  QFETCH( QString, expectedRaster );
  QFETCH( int, inputBand );
  QFETCH( int, roundingDirection );
  QFETCH( int, decimals );
  QFETCH( int, baseN );

  //prepare input params
  QgsProject p;
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:roundrastervalues" ) ) );

  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  std::unique_ptr<QgsRasterLayer> inputRasterLayer = qgis::make_unique< QgsRasterLayer >( myDataPath + inputRaster, "inputDataset", "gdal" );

  //set project crs and ellipsoid from input layer
  p.setCrs( inputRasterLayer->crs(), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( QStringLiteral( "INPUT" ), myDataPath + inputRaster );
  parameters.insert( QStringLiteral( "BAND" ), inputBand );
  parameters.insert( QStringLiteral( "ROUNDING_DIRECTION" ), roundingDirection );
  parameters.insert( QStringLiteral( "DECIMAL_PLACES" ), decimals );
  parameters.insert( QStringLiteral( "BASE_N" ), baseN );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  //prepare expectedRaster
  std::unique_ptr<QgsRasterLayer> expectedRasterLayer = qgis::make_unique< QgsRasterLayer >( myDataPath + "/control_images/roundRasterValues/" + expectedRaster, "expectedDataset", "gdal" );
  std::unique_ptr< QgsRasterInterface > expectedInterface( expectedRasterLayer->dataProvider()->clone() );
  QgsRasterIterator expectedIter( expectedInterface.get() );
  expectedIter.startRasterRead( 1, expectedRasterLayer->width(), expectedRasterLayer->height(), expectedInterface->extent() );

  //run alg...

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  //...and check results with expected datasets
  std::unique_ptr<QgsRasterLayer> outputRaster = qgis::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
  std::unique_ptr< QgsRasterInterface > outputInterface( outputRaster->dataProvider()->clone() );

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

  std::unique_ptr< QgsRasterBlock > outputRasterBlock;
  std::unique_ptr< QgsRasterBlock > expectedRasterBlock;

  while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) &&
          expectedIter.readNextRasterPart( 1, expectedIterCols, expectedIterRows, expectedRasterBlock, expectedIterLeft, expectedIterTop ) )
  {
    for ( int row = 0; row < expectedIterRows; row++ )
    {
      for ( int column = 0; column < expectedIterCols; column++ )
      {
        double expectedValue = expectedRasterBlock->value( row, column );
        double outputValue = outputRasterBlock->value( row, column );
        QCOMPARE( outputValue, expectedValue );
      }
    }
  }
}

void TestQgsProcessingAlgs::layoutMapExtent()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:printlayoutmapextenttolayer" ) ) );
  QVERIFY( alg != nullptr );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "l" ) );
  parameters.insert( QStringLiteral( "MAP" ), QStringLiteral( "m" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  // no layout
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->setName( QStringLiteral( "l" ) );
  p.layoutManager()->addLayout( layout );

  // no matching map
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( map );
  map->setId( QStringLiteral( "m" ) );
  map->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  map->attemptSetSceneRect( QRectF( 100, 100, 150, 180 ) );
  map->zoomToExtent( QgsRectangle( 10000, 100000, 60000, 180000 ) );
  map->setMapRotation( 45 );
  map->setScale( 10000 );
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( layout );
  layout->addLayoutItem( map2 );
  map2->setId( QStringLiteral( "m2" ) );
  map2->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3785" ) ) );
  map2->attemptSetSceneRect( QRectF( 100, 100, 50, 80 ) );
  map2->zoomToExtent( QgsRectangle( 10000, 100000, 5000, 8000 ) );
  map2->setMapRotation( 0 );
  map2->setScale( 1000 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( QStringLiteral( "WIDTH" ) ).toDouble(), 150.0 );
  QCOMPARE( results.value( QStringLiteral( "HEIGHT" ) ).toDouble(), 180.0 );
  QCOMPARE( results.value( QStringLiteral( "SCALE" ) ).toDouble(), 10000.0 );
  QCOMPARE( results.value( QStringLiteral( "ROTATION" ) ).toDouble(), 45.0 );

  QgsFeature f;
  QCOMPARE( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() )->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  QVERIFY( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) )->getFeatures().nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "m" ) );
  QCOMPARE( f.attribute( 1 ).toDouble(), 150.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 180.0 );
  QCOMPARE( f.attribute( 3 ).toDouble(), 10000.0 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 45.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((33833 140106, 34894 141167, 36167 139894, 35106 138833, 33833 140106))" ) );

  // all maps
  parameters.remove( QStringLiteral( "MAP" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( QStringLiteral( "WIDTH" ) ).isValid() );
  QVERIFY( !results.value( QStringLiteral( "HEIGHT" ) ).isValid() );
  QVERIFY( !results.value( QStringLiteral( "SCALE" ) ).isValid() );
  QVERIFY( !results.value( QStringLiteral( "ROTATION" ) ).isValid() );

  QCOMPARE( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() )->crs().authid(), QStringLiteral( "EPSG:3785" ) );
  QgsFeatureIterator it = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) )->getFeatures();
  QgsFeature f1;
  QVERIFY( it.nextFeature( f1 ) );
  QgsFeature f2;
  QVERIFY( it.nextFeature( f2 ) );
  f = f1.attribute( 0 ).toString() == QStringLiteral( "m" ) ? f1 : f2;
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "m" ) );
  QCOMPARE( f.attribute( 1 ).toDouble(), 150.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 180.0 );
  QCOMPARE( f.attribute( 3 ).toDouble(), 10000.0 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 45.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((12077408 -7108521, 12079627 -7107575, 12080760 -7110245, 12078540 -7111191, 12077408 -7108521))" ) );
  f = f1.attribute( 0 ).toString() == QStringLiteral( "m" ) ? f2 : f1;
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "m2" ) );
  QCOMPARE( f.attribute( 1 ).toDouble(), 50.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 80.0 );
  QGSCOMPARENEAR( f.attribute( 3 ).toDouble(), 1000.0, 0.0001 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 0.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((7475 54040, 7525 54040, 7525 53960, 7475 53960, 7475 54040))" ) );

  // crs override
  parameters.insert( QStringLiteral( "CRS" ),  QStringLiteral( "EPSG:3111" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( QStringLiteral( "WIDTH" ) ).isValid() );
  QVERIFY( !results.value( QStringLiteral( "HEIGHT" ) ).isValid() );
  QVERIFY( !results.value( QStringLiteral( "SCALE" ) ).isValid() );
  QVERIFY( !results.value( QStringLiteral( "ROTATION" ) ).isValid() );

  QCOMPARE( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() )->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  it = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) )->getFeatures();
  QVERIFY( it.nextFeature( f1 ) );
  QVERIFY( it.nextFeature( f2 ) );
  f = f1.attribute( 0 ).toString() == QStringLiteral( "m" ) ? f1 : f2;
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "m" ) );
  QCOMPARE( f.attribute( 1 ).toDouble(), 150.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 180.0 );
  QCOMPARE( f.attribute( 3 ).toDouble(), 10000.0 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 45.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((33833 140106, 34894 141167, 36167 139894, 35106 138833, 33833 140106))" ) );
  f = f1.attribute( 0 ).toString() == QStringLiteral( "m" ) ? f2 : f1;
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "m2" ) );
  QCOMPARE( f.attribute( 1 ).toDouble(), 50.0 );
  QCOMPARE( f.attribute( 2 ).toDouble(), 80.0 );
  QGSCOMPARENEAR( f.attribute( 3 ).toDouble(), 1000.0, 0.0001 );
  QCOMPARE( f.attribute( 4 ).toDouble(), 0.0 );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((-10399464 -5347896, -10399461 -5347835, -10399364 -5347840, -10399367 -5347901, -10399464 -5347896))" ) );

}

void TestQgsProcessingAlgs::styleFromProject()
{
  QgsProject p;
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );
  p.addMapLayer( vl );
  QgsSimpleMarkerSymbolLayer *simpleMarkerLayer = new QgsSimpleMarkerSymbolLayer();
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol();
  markerSymbol->changeSymbolLayer( 0, simpleMarkerLayer );
  vl->setRenderer( new QgsSingleSymbolRenderer( markerSymbol ) );
  // rule based renderer
  QgsVectorLayer *vl2 = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( vl2->isValid() );
  p.addMapLayer( vl2 );
  QgsSymbol *s1 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
  s1->setColor( QColor( 0, 255, 0 ) );
  QgsSymbol *s2 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
  s2->setColor( QColor( 0, 255, 255 ) );
  QgsRuleBasedRenderer::Rule *rootRule = new QgsRuleBasedRenderer::Rule( nullptr );
  QgsRuleBasedRenderer::Rule *rule2 = new QgsRuleBasedRenderer::Rule( s1, 0, 0, QStringLiteral( "fld >= 5 and fld <= 20" ) );
  rootRule->appendChild( rule2 );
  QgsRuleBasedRenderer::Rule *rule3 = new QgsRuleBasedRenderer::Rule( s2, 0, 0, QStringLiteral( "fld <= 10" ) );
  rule2->appendChild( rule3 );
  vl2->setRenderer( new QgsRuleBasedRenderer( rootRule ) );
  // labeling
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  // raster layer
  QgsRasterLayer *rl = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/tenbytenraster.asc",
      QStringLiteral( "rl" ) );
  QVERIFY( rl->isValid() );
  p.addMapLayer( rl );

  QgsRasterShader *rasterShader = new QgsRasterShader();
  QgsColorRampShader *colorRampShader = new QgsColorRampShader();
  colorRampShader->setColorRampType( QgsColorRampShader::Interpolated );
  colorRampShader->setSourceColorRamp( new QgsGradientColorRamp( QColor( 255, 255, 0 ), QColor( 255, 0, 255 ) ) );
  rasterShader->setRasterShaderFunction( colorRampShader );
  QgsSingleBandPseudoColorRenderer *r = new QgsSingleBandPseudoColorRenderer( rl->dataProvider(), 1, rasterShader );
  rl->setRenderer( r );

  // with layout
  QgsPrintLayout *l = new QgsPrintLayout( &p );
  l->setName( QStringLiteral( "test layout" ) );
  l->initializeDefaults();
  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l->addLayoutItem( scalebar );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );

  p.layoutManager()->addLayout( l );

  // with annotations
  QgsTextAnnotation *annotation = new QgsTextAnnotation();
  QgsSymbol *a1 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
  a1->setColor( QColor( 0, 200, 0 ) );
  annotation->setMarkerSymbol( static_cast< QgsMarkerSymbol * >( a1 ) );
  QgsSymbol *a2 = QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry );
  a2->setColor( QColor( 200, 200, 0 ) );
  annotation->setFillSymbol( static_cast< QgsFillSymbol * >( a2 ) );
  p.annotationManager()->addAnnotation( annotation );

  // ok, run alg
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:stylefromproject" ) ) );
  QVERIFY( alg != nullptr );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "SYMBOLS" ) ).toInt(), 6 );
  QCOMPARE( results.value( QStringLiteral( "COLORRAMPS" ) ).toInt(), 1 );
  QCOMPARE( results.value( QStringLiteral( "TEXTFORMATS" ) ).toInt(), 1 );
  QCOMPARE( results.value( QStringLiteral( "LABELSETTINGS" ) ).toInt(), 1 );

  // read style file back in
  QgsStyle s;
  s.createMemoryDatabase();
  QVERIFY( s.importXml( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QCOMPARE( s.symbolCount(), 6 );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "Annotation Fill" ) ) );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "Annotation Marker" ) ) );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "test layout Page" ) ) );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "vl" ) ) );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "vl2" ) ) );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "vl2 (2)" ) ) );
  QCOMPARE( s.colorRampCount(), 1 );
  QVERIFY( s.colorRampNames().contains( QStringLiteral( "rl" ) ) );
  QCOMPARE( s.textFormatCount(), 1 );
  QVERIFY( s.textFormatNames().contains( QStringLiteral( "test layout <Scalebar>" ) ) );
  QCOMPARE( s.labelSettingsCount(), 1 );
  QVERIFY( s.labelSettingsNames().contains( QStringLiteral( "vl" ) ) );

  // using a project path
  QTemporaryFile tmpFile;
  tmpFile.open();
  tmpFile.close();
  QVERIFY( p.write( tmpFile.fileName() ) );
  p.clear();
  parameters.insert( QStringLiteral( "INPUT" ), tmpFile.fileName() );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "SYMBOLS" ) ).toInt(), 6 );
  // this should be 1, but currently raster layers aren't supported -
  // we first need to allow raster renderers to be read and restored for invalid layer sources
  QCOMPARE( results.value( QStringLiteral( "COLORRAMPS" ) ).toInt(), 0 );
  QCOMPARE( results.value( QStringLiteral( "TEXTFORMATS" ) ).toInt(), 1 );
  QCOMPARE( results.value( QStringLiteral( "LABELSETTINGS" ) ).toInt(), 1 );
}

void TestQgsProcessingAlgs::combineStyles()
{
  QgsStyle s1;
  s1.createMemoryDatabase();
  QgsStyle s2;
  s2.createMemoryDatabase();

  QgsSimpleMarkerSymbolLayer *simpleMarkerLayer = new QgsSimpleMarkerSymbolLayer();
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol();
  markerSymbol->changeSymbolLayer( 0, simpleMarkerLayer );
  s1.addSymbol( QStringLiteral( "sym1" ), markerSymbol, true );
  s1.tagSymbol( QgsStyle::SymbolEntity, QStringLiteral( "sym1" ), QStringList() << QStringLiteral( "t1" ) << QStringLiteral( "t2" ) );

  QgsSymbol *sym1 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
  s2.addSymbol( QStringLiteral( "sym2" ), sym1, true );
  QgsSymbol *sym2 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
  s2.addSymbol( QStringLiteral( "sym1" ), sym2, true );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  s1.addLabelSettings( QStringLiteral( "label1" ), settings, true );

  s2.addColorRamp( QStringLiteral( "ramp1" ), new QgsGradientColorRamp( QColor( 255, 255, 0 ), QColor( 255, 0, 255 ) ), true );
  s2.addTextFormat( QStringLiteral( "format2" ), QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ), true );

  QTemporaryFile tmpFile;
  tmpFile.open();
  tmpFile.close();
  QVERIFY( s1.exportXml( tmpFile.fileName() ) );
  QTemporaryFile tmpFile2;
  tmpFile2.open();
  tmpFile2.close();
  QVERIFY( s2.exportXml( tmpFile2.fileName() ) );

  // ok, run alg
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:combinestyles" ) ) );
  QVERIFY( alg != nullptr );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QStringList() << tmpFile.fileName() << tmpFile2.fileName() );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "SYMBOLS" ) ).toInt(), 3 );
  QCOMPARE( results.value( QStringLiteral( "COLORRAMPS" ) ).toInt(), 1 );
  QCOMPARE( results.value( QStringLiteral( "TEXTFORMATS" ) ).toInt(), 1 );
  QCOMPARE( results.value( QStringLiteral( "LABELSETTINGS" ) ).toInt(), 1 );

  // check result
  QgsStyle s;
  s.createMemoryDatabase();
  QVERIFY( s.importXml( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QCOMPARE( s.symbolCount(), 3 );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "sym1" ) ) );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "sym2" ) ) );
  QVERIFY( s.symbolNames().contains( QStringLiteral( "sym1 (2)" ) ) );
  QCOMPARE( s.tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "sym1" ) ).count(), 2 );
  QVERIFY( s.tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "sym1" ) ).contains( QStringLiteral( "t1" ) ) );
  QVERIFY( s.tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "sym1" ) ).contains( QStringLiteral( "t2" ) ) );
  QCOMPARE( s.colorRampCount(), 1 );
  QVERIFY( s.colorRampNames().contains( QStringLiteral( "ramp1" ) ) );
  QCOMPARE( s.textFormatCount(), 1 );
  QVERIFY( s.textFormatNames().contains( QStringLiteral( "format2" ) ) );
  QCOMPARE( s.labelSettingsCount(), 1 );
  QVERIFY( s.labelSettingsNames().contains( QStringLiteral( "label1" ) ) );
}

void TestQgsProcessingAlgs::bookmarksToLayer()
{
  QgsApplication::bookmarkManager()->clear();
  // create some bookmarks
  QgsBookmark b1;
  b1.setName( QStringLiteral( "test name" ) );
  b1.setGroup( QStringLiteral( "test group" ) );
  b1.setExtent( QgsReferencedRectangle( QgsRectangle( 1, 2, 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
  QgsApplication::bookmarkManager()->addBookmark( b1 );

  QgsBookmark b2;
  b2.setName( QStringLiteral( "test name 2" ) );
  b2.setExtent( QgsReferencedRectangle( QgsRectangle( 16259461, -2477192, 16391255, -2372535 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) ) );
  QgsApplication::bookmarkManager()->addBookmark( b2 );
  QgsBookmark b3;
  b3.setName( QStringLiteral( "test name 3" ) );
  b3.setExtent( QgsReferencedRectangle( QgsRectangle( 11, 21, 31, 41 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
  QgsProject p;
  p.bookmarkManager()->addBookmark( b3 );

  // ok, run alg
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:bookmarkstolayer" ) ) );
  QVERIFY( alg != nullptr );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "SOURCE" ), QVariantList() << 0 );
  parameters.insert( QStringLiteral( "CRS" ), QStringLiteral( "EPSG:4326" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  // check result
  QgsFeature f;
  QCOMPARE( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() )->crs().authid(), QStringLiteral( "EPSG:4326" ) );
  QgsFeatureIterator it = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) )->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "test name 3" ) );
  QCOMPARE( f.attribute( 1 ).toString(), QString() );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((11 21, 31 21, 31 41, 11 41, 11 21))" ) );
  QVERIFY( !it.nextFeature( f ) );

  // user bookmarks
  parameters.insert( QStringLiteral( "SOURCE" ), QVariantList() << 1 );
  ok = false;
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:bookmarkstolayer" ) ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() )->crs().authid(), QStringLiteral( "EPSG:4326" ) );
  it = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) )->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "test name" ) );
  QCOMPARE( f.attribute( 1 ).toString(), QStringLiteral( "test group" ) );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((1 2, 3 2, 3 4, 1 4, 1 2))" ) );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "test name 2" ) );
  QCOMPARE( f.attribute( 1 ).toString(), QString() );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22))" ) );
  QVERIFY( !it.nextFeature( f ) );

  // both
  parameters.insert( QStringLiteral( "SOURCE" ), QVariantList() << 0 << 1 );
  ok = false;
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:bookmarkstolayer" ) ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() )->crs().authid(), QStringLiteral( "EPSG:4326" ) );
  it = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) )->getFeatures();
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "test name 3" ) );
  QCOMPARE( f.attribute( 1 ).toString(), QString() );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((11 21, 31 21, 31 41, 11 41, 11 21))" ) );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "test name" ) );
  QCOMPARE( f.attribute( 1 ).toString(), QStringLiteral( "test group" ) );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((1 2, 3 2, 3 4, 1 4, 1 2))" ) );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toString(), QStringLiteral( "test name 2" ) );
  QCOMPARE( f.attribute( 1 ).toString(), QString() );
  QCOMPARE( f.geometry().asWkt( 0 ), QStringLiteral( "Polygon ((146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22))" ) );
  QVERIFY( !it.nextFeature( f ) );

}

void TestQgsProcessingAlgs::layerToBookmarks()
{
  std::unique_ptr<QgsVectorLayer> inputLayer( qgis::make_unique<QgsVectorLayer>( QStringLiteral( "Polygon?crs=epsg:4326&field=province:string&field=municipality:string" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) ) );
  QVERIFY( inputLayer->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << QStringLiteral( "b1" ) << QStringLiteral( "g1" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((11 21, 31 21, 31 41, 11 41, 11 21))" ) ) );
  inputLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << QStringLiteral( "b2" ) << QString() );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -22, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 147 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -21, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22, 146 -22))" ) ) );
  inputLayer->dataProvider()->addFeature( f );

  QgsApplication::bookmarkManager()->clear();

  // run alg
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:layertobookmarks" ) ) );
  QVERIFY( alg != nullptr );

  QgsProject p;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( inputLayer.get() ) );
  parameters.insert( QStringLiteral( "DESTINATION" ), 0 );
  parameters.insert( QStringLiteral( "NAME_EXPRESSION" ), QStringLiteral( "upper(province)" ) );
  parameters.insert( QStringLiteral( "GROUP_EXPRESSION" ), QStringLiteral( "upper(municipality)" ) );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( p.bookmarkManager()->bookmarks().count(), 2 );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 0 ).name(), QStringLiteral( "B1" ) );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 0 ).group(), QStringLiteral( "G1" ) );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 0 ).extent().crs().authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 0 ).extent().toString( 0 ), QStringLiteral( "11,21 : 31,41" ) );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 1 ).name(), QStringLiteral( "B2" ) );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 1 ).group(), QString() );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 1 ).extent().crs().authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( p.bookmarkManager()->bookmarks().at( 1 ).extent().toString( 0 ), QStringLiteral( "146,-22 : 147,-21" ) );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().count(), 0 );
  p.bookmarkManager()->clear();

  // send to application bookmarks
  parameters.insert( QStringLiteral( "DESTINATION" ), 1 );
  parameters.insert( QStringLiteral( "GROUP_EXPRESSION" ), QVariant() );

  ok = false;
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:layertobookmarks" ) ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( p.bookmarkManager()->bookmarks().count(), 0 );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().count(), 2 );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 0 ).name(), QStringLiteral( "B1" ) );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 0 ).group(), QString() );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 0 ).extent().crs().authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 0 ).extent().toString( 0 ), QStringLiteral( "11,21 : 31,41" ) );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 1 ).name(), QStringLiteral( "B2" ) );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 1 ).group(), QString() );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 1 ).extent().crs().authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( QgsApplication::bookmarkManager()->bookmarks().at( 1 ).extent().toString( 0 ), QStringLiteral( "146,-22 : 147,-21" ) );
}

void TestQgsProcessingAlgs::repairShapefile()
{
  QTemporaryDir tmpPath;

  QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QFile::copy( dataDir + "/points.shp", tmpPath.filePath( QStringLiteral( "points.shp" ) ) );
  QFile::copy( dataDir + "/points.shp", tmpPath.filePath( QStringLiteral( "points.prj" ) ) );
  QFile::copy( dataDir + "/points.shp", tmpPath.filePath( QStringLiteral( "points.dbf" ) ) );
  // no shx!!

  std::unique_ptr< QgsVectorLayer > layer = qgis::make_unique< QgsVectorLayer >( tmpPath.filePath( QStringLiteral( "points.shp" ) ) );
  QVERIFY( !layer->isValid() );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:repairshapefile" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "not a file" ) );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  parameters.insert( QStringLiteral( "INPUT" ), tmpPath.filePath( QStringLiteral( "points.shp" ) ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "OUTPUT" ) ).toString(), tmpPath.filePath( QStringLiteral( "points.shp" ) ) );

  layer = qgis::make_unique< QgsVectorLayer >( tmpPath.filePath( QStringLiteral( "points.shp" ) ) );
  QVERIFY( layer->isValid() );
}

void TestQgsProcessingAlgs::renameField()
{
  QgsProject p;
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );
  p.addMapLayer( layer );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:renametablefield" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layer ) );
  parameters.insert( QStringLiteral( "FIELD" ), QStringLiteral( "doesntexist" ) );
  parameters.insert( QStringLiteral( "NEW_NAME" ), QStringLiteral( "newname" ) );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // field doesn't exist
  QVERIFY( !ok );

  parameters.insert( QStringLiteral( "FIELD" ), QStringLiteral( "col1" ) );
  parameters.insert( QStringLiteral( "NEW_NAME" ), QStringLiteral( "pk" ) );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // already a field with this name
  QVERIFY( !ok );

  parameters.insert( QStringLiteral( "NEW_NAME" ), QStringLiteral( "newname" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) )->fields().at( 1 ).name(), QStringLiteral( "newname" ) );

}

void TestQgsProcessingAlgs::compareDatasets()
{
  QgsProject p;
  QgsVectorLayer *pointLayer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( pointLayer->isValid() );
  p.addMapLayer( pointLayer );
  QgsVectorLayer *originalLayer = new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=pk:int&field=col1:string&field=col2:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( originalLayer->isValid() );
  p.addMapLayer( originalLayer );
  QgsVectorLayer *revisedLayer = new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=pk:int&field=col1:string&field=col2:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( revisedLayer->isValid() );
  p.addMapLayer( revisedLayer );
  QgsVectorLayer *differentAttrs = new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=pk:int&field=col3:string&field=col2:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( differentAttrs->isValid() );
  p.addMapLayer( differentAttrs );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:detectvectorchanges" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  // differing geometry types - alg should fail
  parameters.insert( QStringLiteral( "ORIGINAL" ), QVariant::fromValue( pointLayer ) );
  parameters.insert( QStringLiteral( "REVISED" ), QVariant::fromValue( originalLayer ) );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // differing fields - alg should fail
  parameters.insert( QStringLiteral( "ORIGINAL" ), QVariant::fromValue( originalLayer ) );
  parameters.insert( QStringLiteral( "REVISED" ), QVariant::fromValue( differentAttrs ) );
  parameters.insert( QStringLiteral( "COMPARE_ATTRIBUTES" ), QStringLiteral( "col1;col2" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // yet if we aren't comparing the field, we shouldn't fail...
  parameters.insert( QStringLiteral( "COMPARE_ATTRIBUTES" ), QStringLiteral( "col2" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  // no features, no outputs
  parameters.insert( QStringLiteral( "ORIGINAL" ), QVariant::fromValue( originalLayer ) );
  parameters.insert( QStringLiteral( "REVISED" ), QVariant::fromValue( revisedLayer ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 0LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 0LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 0LL );

  parameters.insert( QStringLiteral( "UNCHANGED" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ADDED" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "DELETED" ), QgsProcessing::TEMPORARY_OUTPUT );

  // add two features to original
  QgsFeature f;
  f.setAttributes( QgsAttributes() << 5 << QStringLiteral( "b1" ) << QStringLiteral( "g1" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (0 0, 10 0)" ) ) );
  originalLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 5 << QStringLiteral( "c1" ) << QStringLiteral( "g1" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (0 0, 10 0)" ) ) );
  originalLayer->dataProvider()->addFeature( f );

  // just compare two columns
  parameters.insert( QStringLiteral( "COMPARE_ATTRIBUTES" ), QStringLiteral( "col1;col2" ) );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 0LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "UNCHANGED" ) ).toString() ) )->featureCount(), 0L );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 0LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ADDED" ) ).toString() ) )->featureCount(), 0L );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "DELETED" ) ).toString() ) )->featureCount(), 2L );

  // add one same to revised - note that the first attributes differs here, but we aren't considering that
  f.setAttributes( QgsAttributes() << 55 << QStringLiteral( "b1" ) << QStringLiteral( "g1" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (0 0, 10 0)" ) ) );
  revisedLayer->dataProvider()->addFeature( f );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "UNCHANGED" ) ).toString() ) )->featureCount(), 1L );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 0LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ADDED" ) ).toString() ) )->featureCount(), 0L );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "DELETED" ) ).toString() ) )->featureCount(), 1L );

  // ok, let's compare the differing attribute too
  parameters.insert( QStringLiteral( "COMPARE_ATTRIBUTES" ), QStringLiteral( "col1;col2;pk" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 0LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 1LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 2LL );

  parameters.insert( QStringLiteral( "COMPARE_ATTRIBUTES" ), QStringLiteral( "col1;col2" ) );
  // similar to the second feature, but geometry differs
  f.setAttributes( QgsAttributes() << 55 << QStringLiteral( "c1" ) << QStringLiteral( "g1" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (0 0, 11 0)" ) ) );
  revisedLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "UNCHANGED" ) ).toString() ) )->featureCount(), 1L );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ADDED" ) ).toString() ) )->featureCount(), 1L );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 1LL );
  QCOMPARE( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "DELETED" ) ).toString() ) )->featureCount(), 1L );
  // note - we skip the featureCount checks after this -- we can be confident at this stage that all sinks are correctly being populated


  // add another which is identical to first, must be considered as another "added" feature (i.e.
  // don't match to same original feature multiple times)
  f.setAttributes( QgsAttributes() << 555 << QStringLiteral( "b1" ) << QStringLiteral( "g1" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (0 0, 10 0)" ) ) );
  revisedLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 1LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 1LL );

  // add a match for the second feature
  f.setAttributes( QgsAttributes() << 5 << QStringLiteral( "c1" ) << QStringLiteral( "g1" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (0 0, 10 0)" ) ) );
  revisedLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 0LL );

  // test topological match (different number of vertices)
  f.setAttributes( QgsAttributes() << 5 << QStringLiteral( "c1" ) << QStringLiteral( "g1" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (0 10, 5 10, 10 10)" ) ) );
  originalLayer->dataProvider()->addFeature( f );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (0 10, 1 10, 5 10, 10 10)" ) ) );
  revisedLayer->dataProvider()->addFeature( f );

  // exact match shouldn't equate the two
  parameters.insert( QStringLiteral( "MATCH_TYPE" ), 0 );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 3LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 1LL );

  // but topological match should
  parameters.insert( QStringLiteral( "MATCH_TYPE" ), 1 );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 3LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 0LL );

  // null geometry comparisons
  f.setAttributes( QgsAttributes() << 5 << QStringLiteral( "d1" ) << QStringLiteral( "g1" ) );
  f.clearGeometry();
  originalLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 3LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 1LL );

  f.setAttributes( QgsAttributes() << 5 << QStringLiteral( "e1" ) << QStringLiteral( "g1" ) );
  originalLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 3LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 2LL );

  f.setAttributes( QgsAttributes() << 5 << QStringLiteral( "d1" ) << QStringLiteral( "g1" ) );
  revisedLayer->dataProvider()->addFeature( f );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "UNCHANGED_COUNT" ) ).toLongLong(), 4LL );
  QCOMPARE( results.value( QStringLiteral( "ADDED_COUNT" ) ).toLongLong(), 2LL );
  QCOMPARE( results.value( QStringLiteral( "DELETED_COUNT" ) ).toLongLong(), 1LL );

}

void TestQgsProcessingAlgs::shapefileEncoding()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:shpencodinginfo" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QString() );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );

  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( TEST_DATA_DIR ) + "/shapefile/iso-8859-1.shp" );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "ENCODING" ) ).toString(), QStringLiteral( "ISO-8859-1" ) );
  QCOMPARE( results.value( QStringLiteral( "CPG_ENCODING" ) ).toString(), QStringLiteral( "ISO-8859-1" ) );
  QCOMPARE( results.value( QStringLiteral( "LDID_ENCODING" ) ).toString(), QString() );

  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( TEST_DATA_DIR ) + "/shapefile/windows-1252_ldid.shp" );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "ENCODING" ) ).toString(), QStringLiteral( "CP1252" ) );
  QCOMPARE( results.value( QStringLiteral( "CPG_ENCODING" ) ).toString(), QString() );
  QCOMPARE( results.value( QStringLiteral( "LDID_ENCODING" ) ).toString(), QStringLiteral( "CP1252" ) );

  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( TEST_DATA_DIR ) + "/shapefile/system_encoding.shp" );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "ENCODING" ) ).toString(), QString() );
  QCOMPARE( results.value( QStringLiteral( "CPG_ENCODING" ) ).toString(), QString() );
  QCOMPARE( results.value( QStringLiteral( "LDID_ENCODING" ) ).toString(), QString() );
}

void TestQgsProcessingAlgs::setLayerEncoding()
{
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( TEST_DATA_DIR ) + "/shapefile/system_encoding.shp",
      QStringLiteral( "test" ), QStringLiteral( "ogr" ) );
  QVERIFY( vl->isValid() );
  QgsProject p;
  p.addMapLayers(
    QList<QgsMapLayer *>() << vl );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:setlayerencoding" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QString() );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  parameters.insert( QStringLiteral( "INPUT" ), vl->id() );
  parameters.insert( QStringLiteral( "ENCODING" ), QStringLiteral( "ISO-8859-1" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( vl->dataProvider()->encoding(), QStringLiteral( "ISO-8859-1" ) );

}

class TestProcessingFeedback : public QgsProcessingFeedback
{
  public:

    void reportError( const QString &error, bool ) override
    {
      errors << error;
    }

    QStringList errors;

};

void TestQgsProcessingAlgs::raiseException()
{
  TestProcessingFeedback feedback;

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:raiseexception" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "MESSAGE" ), QStringLiteral( "you done screwed up boy" ) );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QCOMPARE( feedback.errors, QStringList() << QStringLiteral( "you done screwed up boy" ) );

  parameters.insert( QStringLiteral( "CONDITION" ), QStringLiteral( "FALSE" ) );
  feedback.errors.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.errors, QStringList() );

  parameters.insert( QStringLiteral( "CONDITION" ), QStringLiteral( "TRUE" ) );
  feedback.errors.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QCOMPARE( feedback.errors, QStringList() << QStringLiteral( "you done screwed up boy" ) );
}

void TestQgsProcessingAlgs::raiseWarning()
{
  TestProcessingFeedback feedback;

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:raisewarning" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "MESSAGE" ), QStringLiteral( "you mighta screwed up boy, but i aint so sure" ) );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.errors, QStringList() << QStringLiteral( "you mighta screwed up boy, but i aint so sure" ) );

  parameters.insert( QStringLiteral( "CONDITION" ), QStringLiteral( "FALSE" ) );
  feedback.errors.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.errors, QStringList() );

  parameters.insert( QStringLiteral( "CONDITION" ), QStringLiteral( "TRUE" ) );
  feedback.errors.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( feedback.errors, QStringList() << QStringLiteral( "you mighta screwed up boy, but i aint so sure" ) );
}

void TestQgsProcessingAlgs::randomFloatingPointDistributionRaster_data()
{
  QTest::addColumn<QString>( "inputExtent" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );
  QTest::addColumn<bool>( "succeeds" );
  QTest::addColumn<QString>( "crs" );
  QTest::addColumn<double>( "pixelSize" );
  QTest::addColumn<int>( "typeId" );

  QTest::newRow( "testcase 1" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Float32
      << true
      << "EPSG:4326"
      << 1.0
      << 0;


  QTest::newRow( "testcase 2" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Float64
      << true
      << "EPSG:4326"
      << 1.0
      << 1;
}

void TestQgsProcessingAlgs::randomFloatingPointDistributionRaster()
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
  alglist << QStringLiteral( "native:createrandomnormalrasterlayer" )
          << QStringLiteral( "native:createrandomexponentialrasterlayer" )
          << QStringLiteral( "native:createrandomgammarasterlayer" );

  for ( int i = 0; i < alglist.length(); i++ )
  {
    QString algname = alglist[i];

    std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( algname ) );
    //set project after layer has been added so that transform context/ellipsoid from crs is also set
    std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
    context->setProject( &p );

    QVariantMap parameters;

    parameters.insert( QStringLiteral( "EXTENT" ), inputExtent );
    parameters.insert( QStringLiteral( "TARGET_CRS" ), QgsCoordinateReferenceSystem( crs ) );
    parameters.insert( QStringLiteral( "PIXEL_SIZE" ), pixelSize );
    parameters.insert( QStringLiteral( "OUTPUT_TYPE" ), typeId );
    parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

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
      std::unique_ptr<QgsRasterLayer> outputRaster = qgis::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
      std::unique_ptr< QgsRasterInterface > outputInterface( outputRaster->dataProvider()->clone() );

      QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
    }
  }
}


void TestQgsProcessingAlgs::randomIntegerDistributionRaster_data()
{
  QTest::addColumn<QString>( "inputExtent" );
  QTest::addColumn<Qgis::DataType>( "expectedDataType" );
  QTest::addColumn<bool>( "succeeds" );
  QTest::addColumn<QString>( "crs" );
  QTest::addColumn<double>( "pixelSize" );
  QTest::addColumn<int>( "typeId" );

  QTest::newRow( "testcase 1" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int16
      << true
      << "EPSG:4326"
      << 1.0
      << 0;

  QTest::newRow( "testcase 2" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::UInt16
      << true
      << "EPSG:4326"
      << 1.0
      << 1;


  QTest::newRow( "testcase 3" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int32
      << true
      << "EPSG:4326"
      << 1.0
      << 2;

  QTest::newRow( "testcase 4" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::UInt32
      << true
      << "EPSG:4326"
      << 1.0
      << 3;

  QTest::newRow( "testcase 5" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Float32
      << true
      << "EPSG:4326"
      << 1.0
      << 4;

  QTest::newRow( "testcase 6" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Float64
      << true
      << "EPSG:4326"
      << 1.0
      << 5;
}


void TestQgsProcessingAlgs::randomIntegerDistributionRaster()
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
  alglist << QStringLiteral( "native:createrandombinomialrasterlayer" )
          << QStringLiteral( "native:createrandomgeometricrasterlayer" )
          << QStringLiteral( "native:createrandomnegativebinomialrasterlayer" )
          << QStringLiteral( "native:createrandompoissonrasterlayer" );

  for ( int i = 0; i < alglist.length(); i++ )
  {
    QString algname = alglist[i];

    std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( algname ) );
    //set project after layer has been added so that transform context/ellipsoid from crs is also set
    std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
    context->setProject( &p );

    QVariantMap parameters;

    parameters.insert( QStringLiteral( "EXTENT" ), inputExtent );
    parameters.insert( QStringLiteral( "TARGET_CRS" ), QgsCoordinateReferenceSystem( crs ) );
    parameters.insert( QStringLiteral( "PIXEL_SIZE" ), pixelSize );
    parameters.insert( QStringLiteral( "OUTPUT_TYPE" ), typeId );
    parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

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
      std::unique_ptr<QgsRasterLayer> outputRaster = qgis::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
      std::unique_ptr< QgsRasterInterface > outputInterface( outputRaster->dataProvider()->clone() );

      QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );
    }
  }
}

void TestQgsProcessingAlgs::randomRaster_data()
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
      << Qgis::Byte
      << true
      << "EPSG:4326"
      << 1.0
      << 1.0
      << 1.0 //should be min max
      << 0;

  QTest::newRow( "testcase 2" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Byte
      << false
      << "EPSG:4326"
      << 1.0
      << -1.0 //fails --> value too small for byte
      << 10.0 //fails --> value too large
      << 0;

  QTest::newRow( "testcase 3" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Byte
      << false
      << "EPSG:4326"
      << 1.0
      << 1.0
      << 256.0 //fails --> value too big for byte
      << 0;

  QTest::newRow( "testcase 4" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int16
      << true
      << "EPSG:4326"
      << 1.0
      << 1.0
      << 10.0
      << 1;

  QTest::newRow( "testcase 5" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int16
      << false
      << "EPSG:4326"
      << 1.0
      << -32769.0
      << -10.0
      << 1;

  QTest::newRow( "testcase 6" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int16
      << false
      << "EPSG:4326"
      << 1.0
      << 1.0
      << 32769.0
      << 1;

  QTest::newRow( "testcase 7" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::UInt16
      << false
      << "EPSG:4326"
      << 1.0
      << -1.0
      << 12.0
      << 2;

  QTest::newRow( "testcase 8" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::UInt16
      << false
      << "EPSG:4326"
      << 1.0
      << 100.0
      << -1.0
      << 2;

  QTest::newRow( "testcase 9" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::UInt16
      << false
      << "EPSG:4326"
      << 1.0
      << 0.0
      << 65536.0
      << 2;

  QTest::newRow( "testcase 10" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int32
      << true
      << "EPSG:4326"
      << 1.0
      << 1.0
      << 12.0
      << 3;

  QTest::newRow( "testcase 10" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int32
      << false
      << "EPSG:4326"
      << 1.0
      << 15.0
      << 12.0
      << 3;

  QTest::newRow( "testcase 11" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int32
      << false
      << "EPSG:4326"
      << 1.0
      << -2147483649.0
      << 1.0
      << 3;

  QTest::newRow( "testcase 12" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Int32
      << false
      << "EPSG:4326"
      << 1.0
      << 1.0
      << 2147483649.0
      << 3;

  QTest::newRow( "testcase 13" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::UInt32
      << true
      << "EPSG:4326"
      << 1.0
      << 1.0
      << 12.0
      << 4;

  QTest::newRow( "testcase 14" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::UInt32
      << false
      << "EPSG:4326"
      << 1.0
      << 1.0
      << 4294967296.0
      << 4;

  QTest::newRow( "testcase 14" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::UInt32
      << false
      << "EPSG:4326"
      << 1.0
      << -10.0
      << -1.0
      << 4;

  QTest::newRow( "testcase 16" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Float32
      << true
      << "EPSG:4326"
      << 1.0
      << 0.0
      << 12.12
      << 5;

  QTest::newRow( "testcase 17" )
      << "-3.000000000,7.000000000,-4.000000000,6.000000000 [EPSG:4326]"
      << Qgis::Float64
      << true
      << "EPSG:4326"
      << 1.0
      << -15.0
      << 12.125789212532487
      << 6;


}

void TestQgsProcessingAlgs::randomRaster()
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
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:createrandomuniformrasterlayer" ) ) );

  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt

  //set project crs and ellipsoid from input layer
  p.setCrs( QgsCoordinateReferenceSystem( crs ), true );

  //set project after layer has been added so that transform context/ellipsoid from crs is also set
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );

  QVariantMap parameters;

  parameters.insert( QStringLiteral( "EXTENT" ), inputExtent );
  parameters.insert( QStringLiteral( "TARGET_CRS" ), QgsCoordinateReferenceSystem( crs ) );
  parameters.insert( QStringLiteral( "PIXEL_SIZE" ), pixelSize );
  parameters.insert( QStringLiteral( "OUTPUT_TYPE" ), typeId );
  parameters.insert( QStringLiteral( "LOWER_BOUND" ), lowerBound );
  parameters.insert( QStringLiteral( "UPPER_BOUND" ), upperBound );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

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
    std::unique_ptr<QgsRasterLayer> outputRaster = qgis::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
    std::unique_ptr< QgsRasterInterface > outputInterface( outputRaster->dataProvider()->clone() );

    QCOMPARE( outputInterface->dataType( 1 ), expectedDataType );

    QgsRasterIterator outputIter( outputInterface.get() );
    outputIter.startRasterRead( 1, outputRaster->width(), outputRaster->height(), outputInterface->extent() );
    int outputIterLeft = 0;
    int outputIterTop = 0;
    int outputIterCols = 0;
    int outputIterRows = 0;

    std::unique_ptr< QgsRasterBlock > outputRasterBlock;

    while ( outputIter.readNextRasterPart( 1, outputIterCols, outputIterRows, outputRasterBlock, outputIterLeft, outputIterTop ) )
    {
      for ( int row = 0; row < outputIterRows; row++ )
      {
        for ( int column = 0; column < outputIterCols; column++ )
        {
          double outputValue = outputRasterBlock->value( row, column );
          //check if random values are in range
          QVERIFY( outputValue >= lowerBound && outputValue <= upperBound );
        }
      }
    }
  }
}

void TestQgsProcessingAlgs::filterByLayerType()
{
  QgsProject p;
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );
  p.addMapLayer( vl );
  // raster layer
  QgsRasterLayer *rl = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/tenbytenraster.asc", QStringLiteral( "rl" ) );
  QVERIFY( rl->isValid() );
  p.addMapLayer( rl );


  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:filterlayersbytype" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  // vector input
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "vl" ) );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !results.value( QStringLiteral( "VECTOR" ) ).toString().isEmpty() );
  QVERIFY( !results.contains( QStringLiteral( "RASTER" ) ) );

  // raster input
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "rl" ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !results.value( QStringLiteral( "RASTER" ) ).toString().isEmpty() );
  QVERIFY( !results.contains( QStringLiteral( "VECTOR" ) ) );
}

void TestQgsProcessingAlgs::conditionalBranch()
{
  QVariantMap config;
  QVariantList conditions;
  QVariantMap cond1;
  cond1.insert( QStringLiteral( "name" ), QStringLiteral( "name1" ) );
  cond1.insert( QStringLiteral( "expression" ), QStringLiteral( "1 * 1" ) );
  conditions << cond1;
  QVariantMap cond2;
  cond2.insert( QStringLiteral( "name" ), QStringLiteral( "name2" ) );
  cond2.insert( QStringLiteral( "expression" ), QStringLiteral( "1 * 0" ) );
  conditions << cond2;
  config.insert( QStringLiteral( "conditions" ), conditions );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:condition" ), config ) );
  QVERIFY( alg != nullptr );

  QCOMPARE( alg->outputDefinitions().size(), 2 );
  QCOMPARE( alg->outputDefinitions().at( 0 )->name(), QStringLiteral( "name1" ) );
  QCOMPARE( alg->outputDefinitions().at( 1 )->name(), QStringLiteral( "name2" ) );

  QVariantMap parameters;
  // vector input
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "vl" ) );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok, config );
  QVERIFY( ok );
  QCOMPARE( results.value( QStringLiteral( "name1" ) ).toInt(), 1 );
  QCOMPARE( results.value( QStringLiteral( "name2" ) ).toInt(), 0 );
}

void TestQgsProcessingAlgs::saveLog()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:savelog" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProcessingFeedback feedback;
  feedback.reportError( QStringLiteral( "test" ) );
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  QFile file( results.value( QStringLiteral( "OUTPUT" ) ).toString() );
  QVERIFY( file.open( QFile::ReadOnly  | QIODevice::Text ) );
  QCOMPARE( QString( file.readAll() ), QStringLiteral( "test\n" ) );

  parameters.insert( QStringLiteral( "USE_HTML" ), true );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  QFile file2( results.value( QStringLiteral( "OUTPUT" ) ).toString() );
  QVERIFY( file2.open( QFile::ReadOnly  | QIODevice::Text ) );
  QCOMPARE( QString( file2.readAll() ), QStringLiteral( "<span style=\"color:red\">test</span><br/>" ) );
}

void TestQgsProcessingAlgs::setProjectVariable()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:setprojectvariable" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "my_var" ) );
  parameters.insert( QStringLiteral( "VALUE" ), 11 );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::projectScope( &p ) );
  QCOMPARE( scope->variable( QStringLiteral( "my_var" ) ).toInt(), 11 );

  //overwrite existing
  parameters.insert( QStringLiteral( "VALUE" ), 13 );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  scope.reset( QgsExpressionContextUtils::projectScope( &p ) );
  QCOMPARE( scope->variable( QStringLiteral( "my_var" ) ).toInt(), 13 );
}

void TestQgsProcessingAlgs::exportLayoutPdf()
{
  QgsProject p;
  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( QStringLiteral( "my layout" ) );
  p.layoutManager()->addLayout( layout );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:printlayouttopdf" ) ) );
  QVERIFY( alg != nullptr );

  const QString outputPdf = QDir::tempPath() + "/my_layout.pdf";
  if ( QFile::exists( outputPdf ) )
    QFile::remove( outputPdf );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "missing" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPdf );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid layout name
  QVERIFY( !ok );
  QVERIFY( !QFile::exists( outputPdf ) );

  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "my layout" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( outputPdf ) );
}

void TestQgsProcessingAlgs::exportLayoutPng()
{
  QgsProject p;
  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( QStringLiteral( "my layout" ) );
  p.layoutManager()->addLayout( layout );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:printlayouttoimage" ) ) );
  QVERIFY( alg != nullptr );

  const QString outputPdf = QDir::tempPath() + "/my_layout.png";
  if ( QFile::exists( outputPdf ) )
    QFile::remove( outputPdf );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "missing" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPdf );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid layout name
  QVERIFY( !ok );
  QVERIFY( !QFile::exists( outputPdf ) );

  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "my layout" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( outputPdf ) );
}

QGSTEST_MAIN( TestQgsProcessingAlgs )
#include "testqgsprocessingalgs.moc"
