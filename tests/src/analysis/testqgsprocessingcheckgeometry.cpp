/***************************************************************************
                         testqgsprocessingcheckgeometry.cpp
                         ---------------------
    begin                : June 2024
    copyright            : (C) 2024 by Jacky Volpes
    email                : jacky dot volpes at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnativealgorithms.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingparameters.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

class TestQgsProcessingCheckGeometry : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingCheckGeometry()
      : QgsTest( QStringLiteral( "Processing Algorithms Check Geometry" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void containedAlg_data();
    void containedAlg();

    void degeneratePolygonAlg();

    void angleAlg_data();
    void angleAlg();

    void segmentLengthAlg_data();
    void segmentLengthAlg();

    void selfIntersectionAlg_data();
    void selfIntersectionAlg();

    void duplicateNodesAlg_data();
    void duplicateNodesAlg();

    void dangleAlg();

    void followBoundariesAlg();

    void overlapAlg();

    void selfContactAlg_data();
    void selfContactAlg();

    void sliverPolygonAlg();

    void gapAlg();

    void pointInPolygonAlg();

    void pointCoveredByLineAlg();

    void lineLayerIntersectionAlg();

    void lineIntersectionAlg();

    void multipartAlg_data();
    void multipartAlg();

    void areaAlg();
    void holeAlg();
    void missingVertexAlg();

  private:
    QgsVectorLayer *mLineLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;
    QgsVectorLayer *mPointLayer = nullptr;
    QgsVectorLayer *mMultiPointLayer = nullptr;
};

void TestQgsProcessingCheckGeometry::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );

  //create a line layer that will be used in tests
  mLineLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLineLayer );
  QVERIFY( mLineLayer->isValid() );

  //create a poly layer that will be used in tests
  mPolygonLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mPolygonLayer );
  QVERIFY( mPolygonLayer->isValid() );

  //create a point layer that will be used in tests
  mPointLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "point_layer.shp" ), QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mPointLayer );
  QVERIFY( mPointLayer->isValid() );

  //create a multipoint layer that will be used in tests
  mMultiPointLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "multipoint_layer.gpkg" ), QStringLiteral( "multipoints" ), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mMultiPointLayer );
  QVERIFY( mMultiPointLayer->isValid() );
}

void TestQgsProcessingCheckGeometry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingCheckGeometry::angleAlg_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layerToTest" );
  QTest::addColumn<int>( "expectedErrorCount" );
  QTest::addColumn<QString>( "uniqueIdFieldName" );
  QTest::addColumn<bool>( "withSelection" );
  QTest::newRow( "Line layer" ) << mLineLayer << 4 << "id" << false;
  QTest::newRow( "Polygon layer" ) << mPolygonLayer << 4 << "id" << false;
  QTest::newRow( "Line layer with selection" ) << mLineLayer << 2 << "id" << true;
  QTest::newRow( "Polygon layer with selection" ) << mPolygonLayer << 3 << "id" << true;
}

void TestQgsProcessingCheckGeometry::angleAlg()
{
  QFETCH( QgsVectorLayer *, layerToTest );
  QFETCH( int, expectedErrorCount );
  QFETCH( QString, uniqueIdFieldName );
  QFETCH( bool, withSelection );

  layerToTest->selectByIds( QgsFeatureIds() << 0 << 1 );

  std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometryangle" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layerToTest->id(), withSelection ) ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), QVariant::fromValue( uniqueIdFieldName ) );
  parameters.insert( QStringLiteral( "MIN_ANGLE" ), 15 );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::areaAlg()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometryarea" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "AREATHRESHOLD" ), 0.04 );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 8 );
  QCOMPARE( errorsLayer->featureCount(), 8 );
}

void TestQgsProcessingCheckGeometry::holeAlg()
{
  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometryhole" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 1 );
  QCOMPARE( errorsLayer->featureCount(), 1 );
}

void TestQgsProcessingCheckGeometry::missingVertexAlg()
{
  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrymissingvertex" ) )
  );
  QVERIFY( alg != nullptr );

  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer *missingVertexLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "missing_vertex.gpkg" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( missingVertexLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 5 );
  QCOMPARE( errorsLayer->featureCount(), 5 );
}

void TestQgsProcessingCheckGeometry::containedAlg_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layerToTest" );
  QTest::addColumn<int>( "expectedErrorCount" );
  QTest::newRow( "Point layer" ) << mPointLayer << 2;
  QTest::newRow( "Line layer with selection" ) << mLineLayer << 1;
  QTest::newRow( "Polygon layer" ) << mPolygonLayer << 1;
}

void TestQgsProcessingCheckGeometry::containedAlg()
{
  QFETCH( QgsVectorLayer *, layerToTest );
  QFETCH( int, expectedErrorCount );

  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrycontained" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layerToTest ) );
  parameters.insert( QStringLiteral( "POLYGONS" ), QList<QVariant>() << QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), QStringLiteral( "id" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::degeneratePolygonAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrydegeneratepolygon" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 1 );
  QCOMPARE( errorsLayer->featureCount(), 1 );
}

void TestQgsProcessingCheckGeometry::segmentLengthAlg_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layerToTest" );
  QTest::addColumn<int>( "expectedErrorCount" );
  QTest::newRow( "Line layer" ) << mLineLayer << 1;
  QTest::newRow( "Polygon layer" ) << mPolygonLayer << 3;
}

void TestQgsProcessingCheckGeometry::segmentLengthAlg()
{
  QFETCH( QgsVectorLayer *, layerToTest );
  QFETCH( int, expectedErrorCount );

  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrysegmentlength" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layerToTest ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "MIN_SEGMENT_LENGTH" ), 0.03 );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::selfIntersectionAlg_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layerToTest" );
  QTest::addColumn<int>( "expectedErrorCount" );
  QTest::newRow( "Line layer" ) << mLineLayer << 3;
  QTest::newRow( "Polygon layer" ) << mPolygonLayer << 2;
}

void TestQgsProcessingCheckGeometry::selfIntersectionAlg()
{
  QFETCH( QgsVectorLayer *, layerToTest );
  QFETCH( int, expectedErrorCount );
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometryselfintersection" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layerToTest ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::dangleAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrydangle" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mLineLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 6 );
  QCOMPARE( errorsLayer->featureCount(), 6 );
}

void TestQgsProcessingCheckGeometry::duplicateNodesAlg_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layerToTest" );
  QTest::addColumn<int>( "expectedErrorCount" );
  QTest::newRow( "Line layer" ) << mLineLayer << 3;
  QTest::newRow( "Polygon layer" ) << mPolygonLayer << 1;
}

void TestQgsProcessingCheckGeometry::duplicateNodesAlg()
{
  QFETCH( QgsVectorLayer *, layerToTest );
  QFETCH( int, expectedErrorCount );

  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometryduplicatenodes" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layerToTest ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::followBoundariesAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometryfollowboundaries" ) )
  );
  QVERIFY( alg != nullptr );

  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "follow_subj.shp" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer *refLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "follow_ref.shp" ), QStringLiteral( "ref_polygons" ), QStringLiteral( "ogr" ) );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( polygonLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "REF_LAYER" ), QVariant::fromValue( refLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 2 );
  QCOMPARE( errorsLayer->featureCount(), 2 );
}

void TestQgsProcessingCheckGeometry::overlapAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometryoverlap" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "MIN_OVERLAP_AREA" ), 0.01 );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 2 );
  QCOMPARE( errorsLayer->featureCount(), 2 );
}

void TestQgsProcessingCheckGeometry::selfContactAlg_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layerToTest" );
  QTest::addColumn<int>( "expectedErrorCount" );
  QTest::newRow( "Line layer" ) << mLineLayer << 2;
  QTest::newRow( "Polygon layer" ) << mPolygonLayer << 1;
}

void TestQgsProcessingCheckGeometry::selfContactAlg()
{
  QFETCH( QgsVectorLayer *, layerToTest );
  QFETCH( int, expectedErrorCount );

  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometryselfcontact" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layerToTest ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::sliverPolygonAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrysliverpolygon" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "MAX_AREA" ), 0.04 );
  parameters.insert( QStringLiteral( "THRESHOLD" ), 20 );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 2 );
  QCOMPARE( errorsLayer->featureCount(), 2 );
}

void TestQgsProcessingCheckGeometry::gapAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrygap" ) )
  );
  QVERIFY( alg != nullptr );

  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer *gapLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "gap_layer.shp" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );

  std::unique_ptr<QgsVectorLayer> allowedGapsLayer = std::make_unique< QgsVectorLayer >( QStringLiteral( "Polygon?crs=epsg:4326" ), QStringLiteral( "allowedGaps" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( allowedGapsLayer.get() );
  QgsFeature allowedGap;

  // First test: without allowed gaps
  {
    QVariantMap parameters;
    parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( gapLayer ) );
    parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
    parameters.insert( QStringLiteral( "GAP_THRESHOLD" ), 0.01 );
    parameters.insert( QStringLiteral( "NEIGHBORS" ), QgsProcessing::TEMPORARY_OUTPUT );
    parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
    parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

    bool ok = false;
    QgsProcessingFeedback feedback;
    std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

    QVariantMap results;
    results = alg->run( parameters, *context, &feedback, &ok );
    QVERIFY( ok );

    std::unique_ptr<QgsVectorLayer> neighborsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "NEIGHBORS" ) ).toString() ) ) );
    std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
    std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
    QVERIFY( neighborsLayer->isValid() );
    QVERIFY( outputLayer->isValid() );
    QVERIFY( errorsLayer->isValid() );
    QCOMPARE( neighborsLayer->featureCount(), 15 );
    QCOMPARE( outputLayer->featureCount(), 5 );
    QCOMPARE( errorsLayer->featureCount(), 5 );

    // keep an output feature for next test
    QVERIFY( outputLayer->getFeatures().nextFeature( allowedGap ) );
  }

  // Second test: with one allowed gap
  {
    // Add an allowed gap
    allowedGap.setFields( allowedGapsLayer->fields(), true );
    QVERIFY( allowedGapsLayer->startEditing() );
    QVERIFY( allowedGapsLayer->addFeature( allowedGap ) );
    QVERIFY( allowedGapsLayer->commitChanges() );

    QVariantMap parameters;
    parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( gapLayer ) );
    parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
    parameters.insert( QStringLiteral( "GAP_THRESHOLD" ), 0.01 );
    parameters.insert( QStringLiteral( "ALLOWED_GAPS_LAYER" ), QVariant::fromValue( allowedGapsLayer.get() ) );
    parameters.insert( QStringLiteral( "ALLOWED_GAPS_BUFFER" ), 0.01 );
    parameters.insert( QStringLiteral( "NEIGHBORS" ), QgsProcessing::TEMPORARY_OUTPUT );
    parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
    parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

    bool ok = false;
    QgsProcessingFeedback feedback;
    std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

    QVariantMap results;
    results = alg->run( parameters, *context, &feedback, &ok );
    QVERIFY( ok );

    std::unique_ptr<QgsVectorLayer> neighborsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "NEIGHBORS" ) ).toString() ) ) );
    std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
    std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
    QVERIFY( neighborsLayer->isValid() );
    QVERIFY( outputLayer->isValid() );
    QVERIFY( errorsLayer->isValid() );
    QCOMPARE( neighborsLayer->featureCount(), 11 );
    QCOMPARE( outputLayer->featureCount(), 4 );
    QCOMPARE( errorsLayer->featureCount(), 4 );
  }
}

void TestQgsProcessingCheckGeometry::pointInPolygonAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrypointinpolygon" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mPointLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "POLYGONS" ), QVariantList() << QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( errorsLayer->featureCount(), 6 );
}

void TestQgsProcessingCheckGeometry::pointCoveredByLineAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrypointcoveredbyline" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mPointLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "LINES" ), QVariantList() << QVariant::fromValue( mLineLayer ) );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( errorsLayer->featureCount(), 5 );
}

void TestQgsProcessingCheckGeometry::lineLayerIntersectionAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrylinelayerintersection" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mLineLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "CHECK_LAYER" ), QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 5 );
  QCOMPARE( errorsLayer->featureCount(), 5 );
}

void TestQgsProcessingCheckGeometry::lineIntersectionAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrylineintersection" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mLineLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 1 );
  QCOMPARE( errorsLayer->featureCount(), 1 );
}

void TestQgsProcessingCheckGeometry::multipartAlg_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layerToTest" );
  QTest::addColumn<int>( "expectedErrorCount" );
  QTest::newRow( "Point layer" ) << mMultiPointLayer << 2;
  QTest::newRow( "Line layer" ) << mLineLayer << 8;
  QTest::newRow( "Polygon layer" ) << mPolygonLayer << 24;
}

void TestQgsProcessingCheckGeometry::multipartAlg()
{
  QFETCH( QgsVectorLayer *, layerToTest );
  QFETCH( int, expectedErrorCount );
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:checkgeometrymultipart" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layerToTest ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "ERRORS" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

QGSTEST_MAIN( TestQgsProcessingCheckGeometry )
#include "testqgsprocessingcheckgeometry.moc"
