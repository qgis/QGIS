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

    void angleAlg_data();
    void angleAlg();

    void areaAlg();
    void holeAlg();
    void missingVertexAlg();

  private:
    QgsVectorLayer *mLineLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;
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
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "ERRORS" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  const std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

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

QGSTEST_MAIN( TestQgsProcessingCheckGeometry )
#include "testqgsprocessingcheckgeometry.moc"
