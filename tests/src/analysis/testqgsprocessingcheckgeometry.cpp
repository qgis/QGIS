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
#include "qgsprocessingparameters.h"
#include "qgsprocessingregistry.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

class DummyFeedback : public QgsProcessingFeedback
{
    Q_OBJECT

  public:
    void reportError( const QString &error, bool fatalError = false ) override
    {
      Q_UNUSED( fatalError );
      mErrors.append( error );
    };

    QStringList errors() { return mErrors; };
    void clear() { mErrors.clear(); };

  private:
    QStringList mErrors;
};

class TestQgsProcessingCheckGeometry : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingCheckGeometry()
      : QgsTest( u"Processing Algorithms Check Geometry"_s ) {}

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

    void duplicatedId();

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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );

  //create a line layer that will be used in tests
  mLineLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), u"lines"_s, u"ogr"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLineLayer );
  QVERIFY( mLineLayer->isValid() );

  //create a poly layer that will be used in tests
  mPolygonLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygons"_s, u"ogr"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mPolygonLayer );
  QVERIFY( mPolygonLayer->isValid() );

  //create a point layer that will be used in tests
  mPointLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "point_layer.shp" ), u"points"_s, u"ogr"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mPointLayer );
  QVERIFY( mPointLayer->isValid() );

  //create a multipoint layer that will be used in tests
  mMultiPointLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "multipoint_layer.gpkg" ), u"multipoints"_s, u"ogr"_s );
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryangle"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( layerToTest->id(), withSelection ) ) );
  parameters.insert( u"UNIQUE_ID"_s, QVariant::fromValue( uniqueIdFieldName ) );
  parameters.insert( u"MIN_ANGLE"_s, 15 );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::areaAlg()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryarea"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"AREATHRESHOLD"_s, 0.04 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 8 );
  QCOMPARE( errorsLayer->featureCount(), 8 );
}

void TestQgsProcessingCheckGeometry::holeAlg()
{
  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryhole"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 1 );
  QCOMPARE( errorsLayer->featureCount(), 1 );
}

void TestQgsProcessingCheckGeometry::missingVertexAlg()
{
  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrymissingvertex"_s )
  );
  QVERIFY( alg != nullptr );

  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer *missingVertexLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "missing_vertex.gpkg" ), u"polygons"_s, u"ogr"_s );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( missingVertexLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 5 );
  QCOMPARE( errorsLayer->featureCount(), 5 );
}

void TestQgsProcessingCheckGeometry::containedAlg_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layerToTest" );
  QTest::addColumn<int>( "expectedErrorCount" );
  QTest::newRow( "Point layer" ) << mPointLayer << 3;
  QTest::newRow( "Line layer with selection" ) << mLineLayer << 1;
  QTest::newRow( "Polygon layer" ) << mPolygonLayer << 1;
}

void TestQgsProcessingCheckGeometry::containedAlg()
{
  QFETCH( QgsVectorLayer *, layerToTest );
  QFETCH( int, expectedErrorCount );

  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrycontained"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layerToTest ) );
  parameters.insert( u"POLYGONS"_s, QList<QVariant>() << QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, u"id"_s );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::degeneratePolygonAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrydegeneratepolygon"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrysegmentlength"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layerToTest ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"MIN_SEGMENT_LENGTH"_s, 0.03 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryselfintersection"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layerToTest ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::dangleAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrydangle"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mLineLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryduplicatenodes"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layerToTest ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::followBoundariesAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryfollowboundaries"_s )
  );
  QVERIFY( alg != nullptr );

  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "follow_subj.shp" ), u"polygons"_s, u"ogr"_s );
  QgsVectorLayer *refLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "follow_ref.shp" ), u"ref_polygons"_s, u"ogr"_s );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"REF_LAYER"_s, QVariant::fromValue( refLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 2 );
  QCOMPARE( errorsLayer->featureCount(), 2 );
}

void TestQgsProcessingCheckGeometry::overlapAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryoverlap"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"MIN_OVERLAP_AREA"_s, 0.01 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryselfcontact"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layerToTest ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::sliverPolygonAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrysliverpolygon"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"MAX_AREA"_s, 0.04 );
  parameters.insert( u"THRESHOLD"_s, 20 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 2 );
  QCOMPARE( errorsLayer->featureCount(), 2 );
}

void TestQgsProcessingCheckGeometry::gapAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrygap"_s )
  );
  QVERIFY( alg != nullptr );

  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer *gapLayer = new QgsVectorLayer( testDataDir.absoluteFilePath( "gap_layer.shp" ), u"polygons"_s, u"ogr"_s );

  auto allowedGapsLayer = std::make_unique< QgsVectorLayer >( u"Polygon?crs=epsg:4326"_s, u"allowedGaps"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( allowedGapsLayer.get() );
  QgsFeature allowedGap;

  // First test: without allowed gaps
  {
    QVariantMap parameters;
    parameters.insert( u"INPUT"_s, QVariant::fromValue( gapLayer ) );
    parameters.insert( u"UNIQUE_ID"_s, "id" );
    parameters.insert( u"GAP_THRESHOLD"_s, 0.01 );
    parameters.insert( u"NEIGHBORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
    parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
    parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

    bool ok = false;
    QgsProcessingFeedback feedback;
    auto context = std::make_unique< QgsProcessingContext >();
    context->setProject( QgsProject::instance() );

    QVariantMap results;
    results = alg->run( parameters, *context, &feedback, &ok );
    QVERIFY( ok );

    std::unique_ptr<QgsVectorLayer> neighborsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"NEIGHBORS"_s ).toString() ) ) );
    std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
    std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
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
    parameters.insert( u"INPUT"_s, QVariant::fromValue( gapLayer ) );
    parameters.insert( u"UNIQUE_ID"_s, "id" );
    parameters.insert( u"GAP_THRESHOLD"_s, 0.01 );
    parameters.insert( u"ALLOWED_GAPS_LAYER"_s, QVariant::fromValue( allowedGapsLayer.get() ) );
    parameters.insert( u"ALLOWED_GAPS_BUFFER"_s, 0.01 );
    parameters.insert( u"NEIGHBORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
    parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
    parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

    bool ok = false;
    QgsProcessingFeedback feedback;
    auto context = std::make_unique< QgsProcessingContext >();
    context->setProject( QgsProject::instance() );

    QVariantMap results;
    results = alg->run( parameters, *context, &feedback, &ok );
    QVERIFY( ok );

    std::unique_ptr<QgsVectorLayer> neighborsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"NEIGHBORS"_s ).toString() ) ) );
    std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
    std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrypointinpolygon"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mPointLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"POLYGONS"_s, QVariantList() << QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( errorsLayer->featureCount(), 6 );
}

void TestQgsProcessingCheckGeometry::pointCoveredByLineAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrypointcoveredbyline"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mPointLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"LINES"_s, QVariantList() << QVariant::fromValue( mLineLayer ) );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( errorsLayer->featureCount(), 6 );
}

void TestQgsProcessingCheckGeometry::lineLayerIntersectionAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrylinelayerintersection"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mLineLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"CHECK_LAYER"_s, QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 5 );
  QCOMPARE( errorsLayer->featureCount(), 5 );
}

void TestQgsProcessingCheckGeometry::lineIntersectionAlg()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrylineintersection"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( mLineLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrymultipart"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layerToTest ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> errorsLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( u"ERRORS"_s ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), expectedErrorCount );
  QCOMPARE( errorsLayer->featureCount(), expectedErrorCount );
}

void TestQgsProcessingCheckGeometry::duplicatedId()
{
  auto polygonLayer = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:4326&field=pk:int&field=fid:string"_s, u"poly"_s, u"memory"_s );
  QVERIFY( polygonLayer->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << "1"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"MultiPolygon (((-0.439 0.995, 0.24 1.079, 0.112 1.031, 0.336 0.576, -0.285 0.573, -0.439 0.995)))"_s ) );
  polygonLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 2 << "2"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"MultiPolygon (((-0.439 0.995, 0.24 1.079, 0.112 1.031, 0.336 0.576, -0.285 0.573, -0.439 0.995)))"_s ) );
  polygonLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 3 << "1"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"MultiPolygon (((-0.439 0.995, 0.24 1.079, 0.112 1.031, 0.336 0.576, -0.285 0.573, -0.439 0.995)))"_s ) );
  polygonLayer->dataProvider()->addFeature( f );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << polygonLayer.get() );

  auto lineLayer = std::make_unique<QgsVectorLayer>( u"LineString?crs=epsg:4326&field=pk:int&field=fid:string"_s, u"line"_s, u"memory"_s );
  QVERIFY( lineLayer->isValid() );

  f.setAttributes( QgsAttributes() << 1 << "1"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (0 0, 10 15)"_s ) );
  lineLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 2 << "2"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (4 2, 0 20)"_s ) );
  lineLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 3 << "1"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"LineString (1 1, 5 3)"_s ) );
  lineLayer->dataProvider()->addFeature( f );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << lineLayer.get() );

  auto pointLayer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=fid:string"_s, u"point"_s, u"memory"_s );
  QVERIFY( pointLayer->isValid() );

  f.setAttributes( QgsAttributes() << 1 << "1"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"Point (1 1)"_s ) );
  pointLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 2 << "2"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"Point (0.5 0.5)"_s ) );
  pointLayer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 3 << "1"_L1 );
  f.setGeometry( QgsGeometry::fromWkt( u"Point (1 1)"_s ) );
  pointLayer->dataProvider()->addFeature( f );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << pointLayer.get() );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryangle"_s ) );
  QVERIFY( alg != nullptr );
  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, u"fid"_s );
  parameters.insert( u"MIN_ANGLE"_s, 15 );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  bool ok = false;
  DummyFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryarea"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"AREATHRESHOLD"_s, 0.04 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryhole"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrymissingvertex"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrycontained"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"POLYGONS"_s, QList<QVariant>() << QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, u"fid"_s );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrydegeneratepolygon"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrysegmentlength"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"MIN_SEGMENT_LENGTH"_s, 0.03 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryselfintersection"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrydangle"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( lineLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryduplicatenodes"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryfollowboundaries"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"REF_LAYER"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryoverlap"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"MIN_OVERLAP_AREA"_s, 0.01 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometryselfcontact"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrysliverpolygon"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"MAX_AREA"_s, 0.04 );
  parameters.insert( u"THRESHOLD"_s, 20 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrygap"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"GAP_THRESHOLD"_s, 0.01 );
  parameters.insert( u"NEIGHBORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrypointinpolygon"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( pointLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"POLYGONS"_s, QVariantList() << QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrypointcoveredbyline"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( pointLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"LINES"_s, QVariantList() << QVariant::fromValue( lineLayer->id() ) );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrylinelayerintersection"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( lineLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"CHECK_LAYER"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrylineintersection"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( lineLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();

  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkgeometrymultipart"_s ) );
  QVERIFY( alg != nullptr );
  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer->id() ) );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERRORS"_s, QgsProcessing::TEMPORARY_OUTPUT );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors().contains( u"Field 'fid' contains non-unique values and can not be used as unique ID."_s ) );
  feedback.clear();
}

QGSTEST_MAIN( TestQgsProcessingCheckGeometry )
#include "testqgsprocessingcheckgeometry.moc"
