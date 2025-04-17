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
#include "qgswkbtypes.h"
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

    void fixMultipartAlg_data();
    void fixMultipartAlg();

    void fixAreaAlg_data();
    void fixAreaAlg();

    void fixDeleteFeaturesAlg_data();
    void fixDeleteFeaturesAlg();

    void fixGapAlg_data();
    void fixGapAlg();

    void fixSelfIntersectionAlg_data();
    void fixSelfIntersectionAlg();

    void fixDuplicateNodesAlg_data();
    void fixDuplicateNodesAlg();

    void fixHoleAlg();
    void fixOverlapAlg();
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
  auto context = std::make_unique<QgsProcessingContext>();

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

void TestQgsProcessingFixGeometry::fixMultipartAlg_data()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );

  QTest::addColumn<QgsVectorLayer *>( "sourceLayer" );
  QTest::addColumn<QgsVectorLayer *>( "errorsLayer" );
  QTest::addColumn<QStringList>( "reportList" );

  QStringList linesReportList;
  for ( int i = 0; i < 8; i++ )
    linesReportList << QStringLiteral( "Convert to single part feature" );
  QTest::newRow( "Lines" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), QStringLiteral( "lines" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "strict_multipart.gpkg|layername=lines_to_fix" ), QStringLiteral( "lines fo fix" ), QStringLiteral( "ogr" ) )
    << linesReportList;

  QStringList polygonsReportList;
  for ( int i = 0; i < 24; i++ )
    polygonsReportList << QStringLiteral( "Convert to single part feature" );
  QTest::newRow( "Polygons" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygon" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "strict_multipart.gpkg|layername=polygons_to_fix" ), QStringLiteral( "polygons fo fix" ), QStringLiteral( "ogr" ) )
    << polygonsReportList;
}

void TestQgsProcessingFixGeometry::fixMultipartAlg()
{
  QFETCH( QgsVectorLayer *, sourceLayer );
  QFETCH( QgsVectorLayer *, errorsLayer );
  QFETCH( QStringList, reportList );

  QVERIFY( sourceLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometrymultipart" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( sourceLayer->source() ) ) );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( errorsLayer->source() ) ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

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

  // Verification of multipart type
  QSet<QVariant> singleTypeIds = reportLayer->uniqueValues( reportLayer->fields().indexFromName( QStringLiteral( "id" ) ) );
  QgsFeatureIterator it = outputLayer->getFeatures();
  QgsFeature feat;
  while ( it.nextFeature( feat ) )
    QCOMPARE( QgsWkbTypes::isSingleType( feat.geometry().wkbType() ), singleTypeIds.contains( feat.attribute( QStringLiteral( "id" ) ) ) );
}

void TestQgsProcessingFixGeometry::fixDuplicateNodesAlg_data()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );

  //create a line layer that will be used in tests
  QTest::addColumn<QgsVectorLayer *>( "sourceLayer" );
  QTest::addColumn<QgsVectorLayer *>( "errorsLayer" );
  QTest::addColumn<QStringList>( "reportList" );
  QTest::addColumn<QMap<QString, int>>( "finalVertexCount" );

  QMap<QString, int> linesFinalVertexCount;
  linesFinalVertexCount.insert( "6", 2 );
  linesFinalVertexCount.insert( "0", 9 );

  QMap<QString, int> polygonsFinalVertexCount;
  polygonsFinalVertexCount.insert( "4", 5 );

  QTest::newRow( "Lines" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), QStringLiteral( "lines" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "remove_duplicated_nodes.gpkg|layername=errors_layer_line" ), QStringLiteral( "lines vertex to delete" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << QStringLiteral( "Delete duplicate node" )
         << QStringLiteral( "Delete duplicate node" )
         << QStringLiteral( "Delete duplicate node" ) )
    << linesFinalVertexCount;

  QTest::newRow( "Polygon" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "remove_duplicated_nodes.gpkg|layername=errors_layer_polygon" ), QStringLiteral( "polygons vertex to delete" ), QStringLiteral( "ogr" ) )
    << ( QStringList() << QStringLiteral( "Delete duplicate node" ) )
    << polygonsFinalVertexCount;
}

void TestQgsProcessingFixGeometry::fixDuplicateNodesAlg()
{
  using QMapQStringInt = QMap<QString, int>;
  QFETCH( QgsVectorLayer *, sourceLayer );
  QFETCH( QgsVectorLayer *, errorsLayer );
  QFETCH( QStringList, reportList );
  QFETCH( QMapQStringInt, finalVertexCount );

  QVERIFY( sourceLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometryduplicatenodes" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( sourceLayer->source() ) ) );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( errorsLayer->source() ) ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

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
    outputLayer->getFeatures( QgsFeatureRequest( QString( "\"id\" = %1" ).arg( it.key() ) ) ).nextFeature( feat );
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
  auto context = std::make_unique<QgsProcessingContext>();

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

void TestQgsProcessingFixGeometry::fixGapAlg_data()
{
  //create a line layer that will be used in tests
  QTest::addColumn<QStringList>( "reportList" );
  QTest::addColumn<int>( "featureCount" );
  QTest::addColumn<int>( "method" );

  QTest::newRow( "Add to longest shared edge" )
    << ( QStringList()
         << QStringLiteral( "Add to longest shared edge" )
         << QStringLiteral( "Add to longest shared edge" )
         << QStringLiteral( "Add to longest shared edge" )
         << QStringLiteral( "Add to longest shared edge" )
         << QStringLiteral( "Add to longest shared edge" ) )
    << 6
    << 0;

  QTest::newRow( "Create new feature" )
    << ( QStringList()
         << QStringLiteral( "Create new feature" )
         << QStringLiteral( "Create new feature" )
         << QStringLiteral( "Create new feature" )
         << QStringLiteral( "Create new feature" )
         << QStringLiteral( "Create new feature" ) )
    << 11
    << 1;

  QTest::newRow( "Add to largest neighbouring area" )
    << ( QStringList()
         << QStringLiteral( "Add to largest neighbouring area" )
         << QStringLiteral( "Add to largest neighbouring area" )
         << QStringLiteral( "Add to largest neighbouring area" )
         << QStringLiteral( "Add to largest neighbouring area" )
         << QStringLiteral( "Add to largest neighbouring area" ) )
    << 6
    << 2;
}

void TestQgsProcessingFixGeometry::fixGapAlg()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "gap_layer.shp" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer gapsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "fix_gap.gpkg|layername=gaps" ), QStringLiteral( "gaps" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer neighborsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "fix_gap.gpkg|layername=neighbors" ), QStringLiteral( "neighbors" ), QStringLiteral( "ogr" ) );
  QVERIFY( sourceLayer.isValid() );
  QVERIFY( gapsLayer.isValid() );
  QVERIFY( neighborsLayer.isValid() );

  QFETCH( QStringList, reportList );
  QFETCH( int, method );
  QFETCH( int, featureCount );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometrygap" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( &sourceLayer ) );
  parameters.insert( QStringLiteral( "NEIGHBORS" ), QVariant::fromValue( &neighborsLayer ) );
  parameters.insert( QStringLiteral( "GAPS" ), QVariant::fromValue( &gapsLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "METHOD" ), method );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "REPORT" ) ).toString() ) ) );
  QVERIFY( reportLayer->isValid() );
  QVERIFY( outputLayer->isValid() );

  QCOMPARE( outputLayer->featureCount(), featureCount );
  QCOMPARE( reportLayer->featureCount(), reportList.count() );
  int idx = 1;
  for ( const QString &expectedReport : reportList )
  {
    const QgsFeature reportFeature = reportLayer->getFeature( idx );
    QCOMPARE( reportFeature.attribute( "report" ), expectedReport );
    idx++;
  }
}

void TestQgsProcessingFixGeometry::fixSelfIntersectionAlg_data()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );

  //create a line layer that will be used in tests
  QTest::addColumn<QgsVectorLayer *>( "sourceLayer" );
  QTest::addColumn<QgsVectorLayer *>( "errorsLayer" );
  QTest::addColumn<QStringList>( "reportList" );
  QTest::addColumn<int>( "method" );
  QTest::addColumn<int>( "expectedOutputFeatureCount" );

  QTest::newRow( "(lines) Split feature into a multi-object feature" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), QStringLiteral( "line layer" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=lines_to_split" ), QStringLiteral( "lines to split" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << "Split feature into a multi-object feature"
         << "Split feature into a multi-object feature"
         << "Error is obsolete" )
    << 0
    << 9;

  QTest::newRow( "(lines) Split feature into multiple single-object features" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), QStringLiteral( "line layer" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=lines_to_split" ), QStringLiteral( "lines to split" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << "Split feature into multiple single-object features"
         << "Split feature into multiple single-object features"
         << "Error is obsolete" )
    << 1
    << 11;

  QTest::newRow( "(polygons) Split feature into a multi-object feature" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygon_line layer" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=polygons_to_split" ), QStringLiteral( "polygons to split" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << "Split feature into a multi-object feature"
         << "Split feature into a multi-object feature" )
    << 0
    << 25;

  QTest::newRow( "(lines) Split feature into multiple single-object features" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygon layer" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=polygons_to_split" ), QStringLiteral( "polygons to split" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << "Split feature into multiple single-object features"
         << "Split feature into multiple single-object features" )
    << 1
    << 27;

  // QTest::newRow( "Split feature into multiple single-object features" )
  //   << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygon layer" ), QStringLiteral( "ogr" ) )
  //   << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=polygons_to_split" ), QStringLiteral( "polygons to split" ), QStringLiteral( "ogr" ) )
  //   << ( QStringList() << "" )
  //   << 1;
}

void TestQgsProcessingFixGeometry::fixSelfIntersectionAlg()
{
  QFETCH( QgsVectorLayer *, sourceLayer );
  QFETCH( QgsVectorLayer *, errorsLayer );
  QFETCH( QStringList, reportList );
  QFETCH( int, method );
  QFETCH( int, expectedOutputFeatureCount );

  QVERIFY( sourceLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometryselfintersection" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( sourceLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( errorsLayer ) );
  parameters.insert( QStringLiteral( "METHOD" ), method );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "REPORT" ) ).toString() ) ) );
  QVERIFY( reportLayer->isValid() );
  QVERIFY( outputLayer->isValid() );

  QCOMPARE( outputLayer->featureCount(), expectedOutputFeatureCount );
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
  auto context = std::make_unique<QgsProcessingContext>();

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

void TestQgsProcessingFixGeometry::fixOverlapAlg()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer errorsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "remove_overlaps.gpkg|layername=overlap_errors" ), QString(), QStringLiteral( "ogr" ) );
  QVERIFY( sourceLayer.isValid() );
  QVERIFY( errorsLayer.isValid() );
  const QStringList reportList = QStringList()
                                 << QStringLiteral( "Remove overlapping area from neighboring polygon with shortest shared edge" )
                                 << QStringLiteral( "Remove overlapping area from neighboring polygon with shortest shared edge" );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometryoverlap" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( &sourceLayer ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "OVERLAP_FEATURE_UNIQUE_IDX" ), "gc_overlap_feature_id" );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( &errorsLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

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
  auto context = std::make_unique<QgsProcessingContext>();

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

void TestQgsProcessingFixGeometry::fixDeleteFeaturesAlg_data()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QTest::addColumn<QgsVectorLayer *>( "sourceLayer" );
  QTest::addColumn<QgsVectorLayer *>( "errorsLayer" );
  QTest::addColumn<QStringList>( "reportList" );

  QTest::newRow( "Points" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "point_layer.shp" ), QStringLiteral( "point layer" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_features.gpkg|layername=points_to_delete" ), QStringLiteral( "points to delete" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" ) );

  QTest::newRow( "Lines" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), QStringLiteral( "line layer" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_features.gpkg|layername=lines_to_delete" ), QStringLiteral( "lines to delete" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" ) );

  QTest::newRow( "Polygons" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), QStringLiteral( "polygon layer" ), QStringLiteral( "ogr" ) )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_features.gpkg|layername=polygons_to_delete" ), QStringLiteral( "polygons to delete" ), QStringLiteral( "ogr" ) )
    << ( QStringList()
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" )
         << QStringLiteral( "Feature deleted" ) );
}

void TestQgsProcessingFixGeometry::fixDeleteFeaturesAlg()
{
  QFETCH( QgsVectorLayer *, sourceLayer );
  QFETCH( QgsVectorLayer *, errorsLayer );
  QFETCH( QStringList, reportList );

  QVERIFY( sourceLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:fixgeometrydeletefeatures" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( sourceLayer->source() ) ) );
  parameters.insert( QStringLiteral( "UNIQUE_ID" ), "id" );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( QgsProcessingFeatureSourceDefinition( errorsLayer->source() ) ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "REPORT" ) ).toString() ) ) );
  QVERIFY( reportLayer->isValid() );
  QVERIFY( outputLayer->isValid() );

  QCOMPARE( outputLayer->featureCount(), sourceLayer->featureCount() - reportList.count() );
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