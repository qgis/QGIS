/***************************************************************************
                         testqgsprocessingfix.cpp
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
#include "qgstest.h"
#include "qgsvectorlayer.h"

class TestQgsProcessingFixGeometry : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingFixGeometry()
      : QgsTest( QStringLiteral( "Processing Algorithms Fix Geometry" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void fixAngleAlg_data();
    void fixAngleAlg();

    void fixAreaAlg_data();
    void fixAreaAlg();

    void fixHoleAlg();
    void fixMissingVertexAlg();

  private:
    const QDir mDataDir { QDir( TEST_DATA_DIR ).absoluteFilePath( QStringLiteral( "geometry_fix" ) ) };
};

void TestQgsProcessingFixGeometry::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
}

void TestQgsProcessingFixGeometry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingFixGeometry::fixAngleAlg_data()
{
  //create a line layer that will be used in tests
  QTest::addColumn<QgsVectorLayer *>( "sourceLayer" );
  QTest::addColumn<QgsVectorLayer *>( "errorsLayer" );
  QTest::addColumn<QStringList>( "reportList" );
  QTest::addColumn<QMap<QString, int>>( "finalVertexCount" );

  QMap<QString, int> linesFinalVertexCount;
  linesFinalVertexCount.insert( "1", 8 );
  linesFinalVertexCount.insert( "2", 7 );
  linesFinalVertexCount.insert( "3", 2 );
  linesFinalVertexCount.insert( "4", 13 );

  QMap<QString, int> polygonsFinalVertexCount;
  polygonsFinalVertexCount.insert( "1", 4 );
  polygonsFinalVertexCount.insert( "2", 7 );
  polygonsFinalVertexCount.insert( "3", 28 );
  polygonsFinalVertexCount.insert( "4", 13 );
  polygonsFinalVertexCount.insert( "5", 5 );

  QTest::newRow( "Lines" )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg|layername=lines" ), QStringLiteral( "lines" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg|layername=lines_vertex_to_delete" ), QStringLiteral( "lines vertex to delete" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" ) )
    << linesFinalVertexCount;

  QTest::newRow( "Polygon" )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg|layername=polygons" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg|layername=polygons_vertex_to_delete" ), QStringLiteral( "polygons vertex to delete" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Resulting geometry is degenerate" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" )
         << QStringLiteral( "Delete node with small angle" ) )
    << polygonsFinalVertexCount;
}

void TestQgsProcessingFixGeometry::fixAngleAlg()
{
  using QMapQStringInt = QMap<QString, int>;
  QFETCH( QgsVectorLayer *, sourceLayer );
  QFETCH( QgsVectorLayer *, errorsLayer );
  QFETCH( QStringList, reportList );
  QFETCH( QMapQStringInt, finalVertexCount );

  QVERIFY( sourceLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometryangle" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( sourceLayer->source() ) ) );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( errorsLayer->source() ) ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "fid" );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "REPORT" ) ).toString() ) ) );
  QVERIFY( reportLayer->isValid() );
  QVERIFY( outputLayer->isValid() );

  QCOMPARE( outputLayer->featureCount(), sourceLayer->featureCount() );
  QCOMPARE( reportLayer->featureCount(), reportList.count() );
  int idx = 1;
  for ( const QString &expectedReport : reportList )
  {
    const QgsFeature reportFeature = reportLayer->getFeature( idx );
    QCOMPARE( reportFeature.attribute( "report" ), expectedReport );
    idx++;
  }

  // Verification of vertex number
  for ( auto it = finalVertexCount.constBegin(); it != finalVertexCount.constEnd(); ++it )
  {
    int nbVertices = 0;
    QgsFeature feat;
    outputLayer->getFeatures( QgsFeatureRequest( QString( "\"fid\" = %1" ).arg( it.key() ) ) ).nextFeature( feat );
    QgsVertexIterator vit = feat.geometry().vertices();
    while ( vit.hasNext() )
    {
      vit.next();
      nbVertices++;
    }
    QCOMPARE( nbVertices, it.value() );
  }
}

void TestQgsProcessingFixGeometry::fixAreaAlg_data()
{
  //create a line layer that will be used in tests
  QTest::addColumn<QStringList>( "reportList" );
  QTest::addColumn<int>( "method" );

  QTest::newRow( "Merge with longest shared edge" )
    << ( QStringList()
         << QStringLiteral( "Merge with neighboring polygon with longest shared edge" )
         << QStringLiteral( "Failed to merge with neighbor: " )
         << QStringLiteral( "Merge with neighboring polygon with longest shared edge" )
         << QStringLiteral( "Failed to merge with neighbor: " )
         << QStringLiteral( "Failed to merge with neighbor: " )
         << QStringLiteral( "Failed to merge with neighbor: " )
         << QStringLiteral( "Merge with neighboring polygon with longest shared edge" )
         << QStringLiteral( "Merge with neighboring polygon with longest shared edge" ) )
    << 0;

  QTest::newRow( "Merge with largest area" )
    << ( QStringList()
         << QStringLiteral( "Merge with neighboring polygon with largest area" )
         << QStringLiteral( "Failed to merge with neighbor: " )
         << QStringLiteral( "Merge with neighboring polygon with largest area" )
         << QStringLiteral( "Failed to merge with neighbor: " )
         << QStringLiteral( "Failed to merge with neighbor: " )
         << QStringLiteral( "Failed to merge with neighbor: " )
         << QStringLiteral( "Merge with neighboring polygon with largest area" )
         << QStringLiteral( "Merge with neighboring polygon with largest area" ) )
    << 1;

  QTest::newRow( "Merge with identical attribute value" )
    << ( QStringList()
         << QStringLiteral( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" )
         << QStringLiteral( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" )
         << QStringLiteral( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" )
         << QStringLiteral( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" )
         << QStringLiteral( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" )
         << QStringLiteral( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" )
         << QStringLiteral( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" )
         << QStringLiteral( "Merge with neighboring polygon with identical attribute value, if any, or leave as is" ) )
    << 2;
}

void TestQgsProcessingFixGeometry::fixAreaAlg()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer errorsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "merge_polygons.gpkg|layername=errors_layer" ), QString(), QStringLiteral( "ogr" ) );
  QFETCH( QStringList, reportList );
  QFETCH( int, method );

  QVERIFY( sourceLayer.isValid() );
  QVERIFY( errorsLayer.isValid() );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometryarea" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( &sourceLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( &errorsLayer ) );
  parameters.insert( QStringLiteral( "METHOD" ), method );
  if ( method == 2 )
    parameters.insert( QStringLiteral( "MERGE_ATTRIBUTE" ), QStringLiteral( "attr" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "REPORT" ) ).toString() ) ) );
  QVERIFY( reportLayer->isValid() );
  QVERIFY( outputLayer->isValid() );

  QCOMPARE( outputLayer->featureCount(), 21 );
  QCOMPARE( reportLayer->featureCount(), reportList.count() );
  int idx = 1;
  for ( const QString &expectedReport : reportList )
  {
    const QgsFeature reportFeature = reportLayer->getFeature( idx );
    QCOMPARE( reportFeature.attribute( "report" ), expectedReport );
    idx++;
  }
}

void TestQgsProcessingFixGeometry::fixHoleAlg()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer errorsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "remove_hole.gpkg|layername=errors_layer" ), QString(), QStringLiteral( "ogr" ) );
  QVERIFY( sourceLayer.isValid() );
  QVERIFY( errorsLayer.isValid() );
  const QStringList reportList = QStringList() << QStringLiteral( "Remove hole" );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometryhole" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( &sourceLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( &errorsLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "REPORT" ) ).toString() ) ) );
  QVERIFY( reportLayer->isValid() );
  QVERIFY( outputLayer->isValid() );

  QCOMPARE( outputLayer->featureCount(), 25 );
  QCOMPARE( reportLayer->featureCount(), reportList.count() );
  int idx = 1;
  for ( const QString &expectedReport : reportList )
  {
    const QgsFeature reportFeature = reportLayer->getFeature( idx );
    QCOMPARE( reportFeature.attribute( "report" ), expectedReport );
    idx++;
  }
}

void TestQgsProcessingFixGeometry::fixMissingVertexAlg()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "missing_vertex.gpkg|layername=missing_vertex" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer errorsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "add_missing_vertex.gpkg|layername=errors_layer" ), QString(), QStringLiteral( "ogr" ) );
  QVERIFY( sourceLayer.isValid() );
  QVERIFY( errorsLayer.isValid() );
  const QStringList reportList = QStringList()
                                 << QStringLiteral( "Add missing vertex" )
                                 << QStringLiteral( "Add missing vertex" )
                                 << QStringLiteral( "Add missing vertex" )
                                 << QStringLiteral( "Add missing vertex" )
                                 << QStringLiteral( "Add missing vertex" );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometrymissingvertex" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( &sourceLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( &errorsLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "REPORT" ) ).toString() ) ) );
  QVERIFY( reportLayer->isValid() );
  QVERIFY( outputLayer->isValid() );

  QCOMPARE( outputLayer->featureCount(), 6 );
  QCOMPARE( reportLayer->featureCount(), reportList.count() );
  int idx = 1;
  for ( const QString &expectedReport : reportList )
  {
    const QgsFeature reportFeature = reportLayer->getFeature( idx );
    QCOMPARE( reportFeature.attribute( "report" ), expectedReport );
    idx++;
  }
}

QGSTEST_MAIN( TestQgsProcessingFixGeometry )
#include "testqgsprocessingfixgeometry.moc"
