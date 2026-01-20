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
#include "qgswkbtypes.h"

class TestQgsProcessingFixGeometry : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingFixGeometry()
      : QgsTest( u"Processing Algorithms Fix Geometry"_s ) {}

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
    const QDir mDataDir { QDir( TEST_DATA_DIR ).absoluteFilePath( u"geometry_fix"_s ) };
};

void TestQgsProcessingFixGeometry::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

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
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg|layername=lines" ), u"lines"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg|layername=lines_vertex_to_delete" ), u"lines vertex to delete"_s, u"ogr"_s )
    << ( QStringList()
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s )
    << linesFinalVertexCount;

  QTest::newRow( "Polygon" )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg|layername=polygons" ), u"polygons"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg|layername=polygons_vertex_to_delete" ), u"polygons vertex to delete"_s, u"ogr"_s )
    << ( QStringList()
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Resulting geometry is degenerate"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s
         << u"Delete node with small angle"_s )
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometryangle"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( sourceLayer->source() ) ) );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( errorsLayer->source() ) ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"UNIQUE_ID"_s, "fid" );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
    linesReportList << u"Convert to single part feature"_s;
  QTest::newRow( "Lines" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), u"lines"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "strict_multipart.gpkg|layername=lines_to_fix" ), u"lines fo fix"_s, u"ogr"_s )
    << linesReportList;

  QStringList polygonsReportList;
  for ( int i = 0; i < 24; i++ )
    polygonsReportList << u"Convert to single part feature"_s;
  QTest::newRow( "Polygons" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygon"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "strict_multipart.gpkg|layername=polygons_to_fix" ), u"polygons fo fix"_s, u"ogr"_s )
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometrymultipart"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( sourceLayer->source() ) ) );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( errorsLayer->source() ) ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"UNIQUE_ID"_s, "id" );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
  QSet<QVariant> singleTypeIds = reportLayer->uniqueValues( reportLayer->fields().indexFromName( u"id"_s ) );
  QgsFeatureIterator it = outputLayer->getFeatures();
  QgsFeature feat;
  while ( it.nextFeature( feat ) )
    QCOMPARE( QgsWkbTypes::isSingleType( feat.geometry().wkbType() ), singleTypeIds.contains( feat.attribute( u"id"_s ) ) );
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
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), u"lines"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "remove_duplicated_nodes.gpkg|layername=errors_layer_line" ), u"lines vertex to delete"_s, u"ogr"_s )
    << ( QStringList()
         << u"Delete duplicate node"_s
         << u"Delete duplicate node"_s
         << u"Delete duplicate node"_s )
    << linesFinalVertexCount;

  QTest::newRow( "Polygon" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygons"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "remove_duplicated_nodes.gpkg|layername=errors_layer_polygon" ), u"polygons vertex to delete"_s, u"ogr"_s )
    << ( QStringList() << u"Delete duplicate node"_s )
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometryduplicatenodes"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( sourceLayer->source() ) ) );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( errorsLayer->source() ) ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"UNIQUE_ID"_s, "id" );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
         << u"Merge with neighboring polygon with longest shared edge"_s
         << u"Failed to merge with neighbor: "_s
         << u"Merge with neighboring polygon with longest shared edge"_s
         << u"Failed to merge with neighbor: "_s
         << u"Failed to merge with neighbor: "_s
         << u"Failed to merge with neighbor: "_s
         << u"Merge with neighboring polygon with longest shared edge"_s
         << u"Merge with neighboring polygon with longest shared edge"_s )
    << 0;

  QTest::newRow( "Merge with largest area" )
    << ( QStringList()
         << u"Merge with neighboring polygon with largest area"_s
         << u"Failed to merge with neighbor: "_s
         << u"Merge with neighboring polygon with largest area"_s
         << u"Failed to merge with neighbor: "_s
         << u"Failed to merge with neighbor: "_s
         << u"Failed to merge with neighbor: "_s
         << u"Merge with neighboring polygon with largest area"_s
         << u"Merge with neighboring polygon with largest area"_s )
    << 1;

  QTest::newRow( "Merge with identical attribute value" )
    << ( QStringList()
         << u"Merge with neighboring polygon with identical attribute value, if any, or leave as is"_s
         << u"Merge with neighboring polygon with identical attribute value, if any, or leave as is"_s
         << u"Merge with neighboring polygon with identical attribute value, if any, or leave as is"_s
         << u"Merge with neighboring polygon with identical attribute value, if any, or leave as is"_s
         << u"Merge with neighboring polygon with identical attribute value, if any, or leave as is"_s
         << u"Merge with neighboring polygon with identical attribute value, if any, or leave as is"_s
         << u"Merge with neighboring polygon with identical attribute value, if any, or leave as is"_s
         << u"Merge with neighboring polygon with identical attribute value, if any, or leave as is"_s )
    << 2;
}

void TestQgsProcessingFixGeometry::fixAreaAlg()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygons"_s, u"ogr"_s );
  QgsVectorLayer errorsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "merge_polygons.gpkg|layername=errors_layer" ), QString(), u"ogr"_s );
  QFETCH( QStringList, reportList );
  QFETCH( int, method );

  QVERIFY( sourceLayer.isValid() );
  QVERIFY( errorsLayer.isValid() );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometryarea"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( &sourceLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( &errorsLayer ) );
  parameters.insert( u"METHOD"_s, method );
  if ( method == 2 )
    parameters.insert( u"MERGE_ATTRIBUTE"_s, u"attr"_s );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
         << u"Add to longest shared edge"_s
         << u"Add to longest shared edge"_s
         << u"Add to longest shared edge"_s
         << u"Add to longest shared edge"_s
         << u"Add to longest shared edge"_s )
    << 6
    << 0;

  QTest::newRow( "Create new feature" )
    << ( QStringList()
         << u"Create new feature"_s
         << u"Create new feature"_s
         << u"Create new feature"_s
         << u"Create new feature"_s
         << u"Create new feature"_s )
    << 11
    << 1;

  QTest::newRow( "Add to largest neighbouring area" )
    << ( QStringList()
         << u"Add to largest neighbouring area"_s
         << u"Add to largest neighbouring area"_s
         << u"Add to largest neighbouring area"_s
         << u"Add to largest neighbouring area"_s
         << u"Add to largest neighbouring area"_s )
    << 6
    << 2;
}

void TestQgsProcessingFixGeometry::fixGapAlg()
{
  const QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "gap_layer.shp" ), u"polygons"_s, u"ogr"_s );
  QgsVectorLayer gapsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "fix_gap.gpkg|layername=gaps" ), u"gaps"_s, u"ogr"_s );
  QgsVectorLayer neighborsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "fix_gap.gpkg|layername=neighbors" ), u"neighbors"_s, u"ogr"_s );
  QVERIFY( sourceLayer.isValid() );
  QVERIFY( gapsLayer.isValid() );
  QVERIFY( neighborsLayer.isValid() );

  QgsProject::instance()->addMapLayer( &sourceLayer );
  QgsProject::instance()->addMapLayer( &gapsLayer );
  QgsProject::instance()->addMapLayer( &neighborsLayer );

  QFETCH( QStringList, reportList );
  QFETCH( int, method );
  QFETCH( int, featureCount );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometrygap"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( &sourceLayer ) );
  parameters.insert( u"NEIGHBORS"_s, QVariant::fromValue( &neighborsLayer ) );
  parameters.insert( u"GAPS"_s, QVariant::fromValue( &gapsLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"METHOD"_s, method );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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

  QTest::newRow( "(lines) Split feature into a multi-part feature" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), u"line layer"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=lines_to_split" ), u"lines to split"_s, u"ogr"_s )
    << ( QStringList()
         << "Split feature into a multi-part feature"
         << "Split feature into a multi-part feature"
         << "Error is obsolete" )
    << 0
    << 9;

  QTest::newRow( "(lines) Split feature into multiple single-part features" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), u"line layer"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=lines_to_split" ), u"lines to split"_s, u"ogr"_s )
    << ( QStringList()
         << "Split feature into multiple single-part features"
         << "Split feature into multiple single-part features"
         << "Error is obsolete" )
    << 1
    << 11;

  QTest::newRow( "(polygons) Split feature into a multi-part feature" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygon_line layer"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=polygons_to_split" ), u"polygons to split"_s, u"ogr"_s )
    << ( QStringList()
         << "Split feature into a multi-part feature"
         << "Split feature into a multi-part feature" )
    << 0
    << 25;

  QTest::newRow( "(lines) Split feature into multiple single-part features" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygon layer"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=polygons_to_split" ), u"polygons to split"_s, u"ogr"_s )
    << ( QStringList()
         << "Split feature into multiple single-part features"
         << "Split feature into multiple single-part features" )
    << 1
    << 27;

  // QTest::newRow( "Split feature into multiple single-part features" )
  //   << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygon layer"_s, u"ogr"_s )
  //   << new QgsVectorLayer( mDataDir.absoluteFilePath( "split_self_intersections.gpkg|layername=polygons_to_split" ), u"polygons to split"_s, u"ogr"_s )
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
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometryselfintersection"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( sourceLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( errorsLayer ) );
  parameters.insert( u"METHOD"_s, method );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygons"_s, u"ogr"_s );
  QgsVectorLayer errorsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "remove_hole.gpkg|layername=errors_layer" ), QString(), u"ogr"_s );
  QVERIFY( sourceLayer.isValid() );
  QVERIFY( errorsLayer.isValid() );
  const QStringList reportList = QStringList() << u"Remove hole"_s;

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometryhole"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( &sourceLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( &errorsLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygons"_s, u"ogr"_s );
  QgsVectorLayer errorsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "remove_overlaps.gpkg|layername=overlap_errors" ), QString(), u"ogr"_s );
  QVERIFY( sourceLayer.isValid() );
  QVERIFY( errorsLayer.isValid() );
  const QStringList reportList = QStringList()
                                 << u"Remove overlapping area from neighboring polygon with shortest shared edge"_s
                                 << u"Remove overlapping area from neighboring polygon with shortest shared edge"_s;

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometryoverlap"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( &sourceLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"OVERLAP_FEATURE_UNIQUE_IDX"_s, "gc_overlap_feature_id" );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( &errorsLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
  QgsVectorLayer sourceLayer = QgsVectorLayer( testDataDir.absoluteFilePath( "missing_vertex.gpkg|layername=missing_vertex" ), u"polygons"_s, u"ogr"_s );
  QgsVectorLayer errorsLayer = QgsVectorLayer( mDataDir.absoluteFilePath( "add_missing_vertex.gpkg|layername=errors_layer" ), QString(), u"ogr"_s );
  QVERIFY( sourceLayer.isValid() );
  QVERIFY( errorsLayer.isValid() );
  const QStringList reportList = QStringList()
                                 << u"Add missing vertex"_s
                                 << u"Add missing vertex"_s
                                 << u"Add missing vertex"_s
                                 << u"Add missing vertex"_s
                                 << u"Add missing vertex"_s;

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometrymissingvertex"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( &sourceLayer ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( &errorsLayer ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  const std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  const std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "point_layer.shp" ), u"point layer"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_features.gpkg|layername=points_to_delete" ), u"points to delete"_s, u"ogr"_s )
    << ( QStringList()
         << u"Feature deleted"_s
         << u"Feature deleted"_s );

  QTest::newRow( "Lines" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "line_layer.shp" ), u"line layer"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_features.gpkg|layername=lines_to_delete" ), u"lines to delete"_s, u"ogr"_s )
    << ( QStringList()
         << u"Feature deleted"_s
         << u"Feature deleted"_s );

  QTest::newRow( "Polygons" )
    << new QgsVectorLayer( testDataDir.absoluteFilePath( "polygon_layer.shp" ), u"polygon layer"_s, u"ogr"_s )
    << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_features.gpkg|layername=polygons_to_delete" ), u"polygons to delete"_s, u"ogr"_s )
    << ( QStringList()
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s
         << u"Feature deleted"_s );
}

void TestQgsProcessingFixGeometry::fixDeleteFeaturesAlg()
{
  QFETCH( QgsVectorLayer *, sourceLayer );
  QFETCH( QgsVectorLayer *, errorsLayer );
  QFETCH( QStringList, reportList );

  QVERIFY( sourceLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );

  const std::unique_ptr<QgsProcessingAlgorithm> alg(
    QgsApplication::processingRegistry()->createAlgorithmById( u"native:fixgeometrydeletefeatures"_s )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( sourceLayer->source() ) ) );
  parameters.insert( u"UNIQUE_ID"_s, "id" );
  parameters.insert( u"ERRORS"_s, QVariant::fromValue( QgsProcessingFeatureSourceDefinition( errorsLayer->source() ) ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"REPORT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"REPORT"_s ).toString() ) ) );
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
