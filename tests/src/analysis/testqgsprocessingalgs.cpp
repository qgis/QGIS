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

#include "qgstest.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsnativealgorithms.h"
#include "qgsalgorithmimportphotos.h"
#include "qgsalgorithmtransform.h"
#include "qgsalgorithmkmeansclustering.h"
#include "qgsvectorlayer.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmultipolygon.h"

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

    void polygonsToLines_data();
    void polygonsToLines();

    void densifyGeometries_data();
    void densifyGeometries();

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

void TestQgsProcessingAlgs::packageAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:package" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  QString outputGpkg = QDir::tempPath() + "/package_alg.gpkg";
  if ( QFile::exists( outputGpkg ) )
    QFile::remove( outputGpkg );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYERS" ), QStringList() << mPointsLayer->id() << mPolygonLayer->id() );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputGpkg );
  bool ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context.reset();

  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  std::unique_ptr< QgsVectorLayer > pointLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  QCOMPARE( pointLayer->wkbType(), mPointsLayer->wkbType() );
  QCOMPARE( pointLayer->featureCount(), mPointsLayer->featureCount() );
  std::unique_ptr< QgsVectorLayer > polygonLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( polygonLayer->isValid() );
  QCOMPARE( polygonLayer->wkbType(), mPolygonLayer->wkbType() );
  QCOMPARE( polygonLayer->featureCount(), mPolygonLayer->featureCount() );
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
  Q_UNUSED( results );
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
  parameters.insert( QStringLiteral( "FILENAME" ), QgsProperty::fromExpression( QStringLiteral( "'test' || \"ATTACHMENTID\" || '.jpg'" ) ) );
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

QGSTEST_MAIN( TestQgsProcessingAlgs )
#include "testqgsprocessingalgs.moc"
