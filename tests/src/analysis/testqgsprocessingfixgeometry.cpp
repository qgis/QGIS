/***************************************************************************
                         testqgsprocessingfix.cpp
                         ---------------------
    begin                : December 2023
    copyright            : (C) 2023 by Jacky Volpes
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
#include <qstringliteral.h>

class TestQgsProcessingFixGeometry: public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingFixGeometry() : QgsTest( QStringLiteral( "Processing Algorithms Fix Geometry" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void deleteVertexAlg_data();
    void deleteVertexAlg();

  private:
    const QDir mDataDir{ QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_fix" ) };

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

void TestQgsProcessingFixGeometry::deleteVertexAlg_data()
{
  //create a line layer that will be used in tests
  QTest::addColumn<QgsVectorLayer *>( "sourceLayer" );
  QTest::addColumn<QgsVectorLayer *>( "errorsLayer" );
  QTest::addColumn<bool>( "topological" );
  QTest::addColumn<QStringList>( "reportList" );

  QTest::newRow( "Lines no topological" )
      << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg" ) + QStringLiteral( "|layername=lines" ),
                             QStringLiteral( "lines" ), QStringLiteral( "ogr" ) )
      << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg" ) + QStringLiteral( "|layername=lines_vertex_to_delete" ),
                             QStringLiteral( "lines vertex to delete" ), QStringLiteral( "ogr" ) )
      << false
      << ( QStringList() << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" );

  QTest::newRow( "Lines topological" )
      << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg" ) + QStringLiteral( "|layername=lines" ),
                             QStringLiteral( "lines" ), QStringLiteral( "ogr" ) )
      << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg" ) + QStringLiteral( "|layername=lines_vertex_to_delete" ),
                             QStringLiteral( "lines vertex to delete" ), QStringLiteral( "ogr" ) )
      << true
      << ( QStringList() << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Double vertex removed" << "Vertex removed" << "Vertex removed" );

  QTest::newRow( "Polygon no topological" )
      << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg" ) + QStringLiteral( "|layername=polygons" ),
                             QStringLiteral( "lines" ), QStringLiteral( "ogr" ) )
      << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg" ) + QStringLiteral( "|layername=polygons_vertex_to_delete" ),
                             QStringLiteral( "polygons vertex to delete" ), QStringLiteral( "ogr" ) )
      << false
      << ( QStringList() << "Vertex removed" << "Resulting geometry would be degenerate" << "Resulting geometry would be degenerate" << "Vertex removed" << "Vertex removed"
           << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" );

  QTest::newRow( "Polygon topological" )
      << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg" ) + QStringLiteral( "|layername=polygons" ),
                             QStringLiteral( "lines" ), QStringLiteral( "ogr" ) )
      << new QgsVectorLayer( mDataDir.absoluteFilePath( "delete_vertex.gpkg" ) + QStringLiteral( "|layername=polygons_vertex_to_delete" ),
                             QStringLiteral( "polygons vertex to delete" ), QStringLiteral( "ogr" ) )
      << true
      << ( QStringList() << "Vertex removed" << "Resulting geometry would be degenerate" << "Resulting geometry would be degenerate" << "Double vertex removed" << "Vertex removed"
           << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" << "Vertex removed" );
}

void TestQgsProcessingFixGeometry::deleteVertexAlg()
{
  QFETCH( QgsVectorLayer *, sourceLayer );
  QFETCH( QgsVectorLayer *, errorsLayer );
  QFETCH( bool, topological );
  QFETCH( QStringList, reportList );

  QVERIFY( sourceLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );

  std::unique_ptr< QgsProcessingAlgorithm > alg(
    QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:deletevertex" ) )
  );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( sourceLayer ) );
  parameters.insert( QStringLiteral( "ERRORS" ), QVariant::fromValue( errorsLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "REPORT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "TOPOLOGICAL" ), topological );

  bool ok = false;
  QgsProcessingFeedback feedback;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  std::unique_ptr<QgsVectorLayer> reportLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "REPORT" ) ).toString() ) ) );
  QVERIFY( outputLayer->isValid() );
  QVERIFY( errorsLayer->isValid() );

  QCOMPARE( outputLayer->featureCount(), sourceLayer->featureCount() );
  QCOMPARE( reportLayer->featureCount(), reportList.count() );
  int idx = 1;
  for ( QString expectedReport : reportList )
  {
    const QgsFeature reportFeature = reportLayer->getFeature( idx );
    QCOMPARE( expectedReport, reportFeature.attribute( "report" ) );
    idx++;
  }
}

QGSTEST_MAIN( TestQgsProcessingFixGeometry )
#include "testqgsprocessingfixgeometry.moc"
