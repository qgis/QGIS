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

    void rasterLogicOp_data();
    void rasterLogicOp();

    void layoutMapExtent();

    void styleFromProject();
    void combineStyles();

    void bookmarksToLayer();
    void layerToBookmarks();

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
  QgsCoordinateReferenceSystem crs( 3857 );
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

QGSTEST_MAIN( TestQgsProcessingAlgs )
#include "testqgsprocessingalgs.moc"
