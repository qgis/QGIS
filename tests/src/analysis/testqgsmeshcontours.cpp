/***************************************************************************
  testqgsmeshcontours.cpp
  --------------------------------------
Date                 : September 2019
Copyright            : (C) 2019 by Peter Petrik
Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <limits>
#include <cmath>
#include <qdebug.h>
#include <iostream>

#include "qgsmeshcontours.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshlayer.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsmeshmemorydataprovider.h"
#include "qgslinesegment.h"
#include "qgsmultilinestring.h"
#include "qgslinestring.h"
#include "qgsgeometryfactory.h"

class TestQgsMeshContours : public QObject
{
    Q_OBJECT

  public:
    TestQgsMeshContours() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction

    void equals( QgsGeometry geom, QgsGeometry expected );

    void testQuadAndTriangleVertexScalarLine_data();
    void testQuadAndTriangleVertexScalarLine();

    void testQuadAndTriangleFaceScalarLine_data();
    void testQuadAndTriangleFaceScalarLine();

    void testQuadAndTriangleVertexScalarPoly_data();
    void testQuadAndTriangleVertexScalarPoly();

    void testQuadAndTriangleFaceScalarPoly_data();
    void testQuadAndTriangleFaceScalarPoly();

  private:
    QgsMeshLayer *mpMeshLayer = nullptr;
};

void  TestQgsMeshContours::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/" );
  const QString uri( testDataDir + "quad_and_triangle.2dm" );
  mpMeshLayer = new QgsMeshLayer( uri, "Triangle and Quad MDAL", "mdal" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_scalar.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_els_face_scalar.dat" );
  QVERIFY( mpMeshLayer->isValid() );
}

void  TestQgsMeshContours::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void  TestQgsMeshContours::init()
{
}
void  TestQgsMeshContours::cleanup()
{
}

void TestQgsMeshContours::equals( QgsGeometry geom, QgsGeometry expected )
{
  if ( expected.isNull() )
  {
    QVERIFY( geom.isNull() );
    return;
  }

  QVERIFY( !geom.isNull() );

  QVERIFY( geom.type() == expected.type() );
  QVERIFY( geom.isMultipart() == expected.isMultipart() );

  const QString gWkt = geom.asWkt();
  const QString eWkt = expected.asWkt();

  QCOMPARE( gWkt.trimmed(), eWkt.trimmed() );
}

void TestQgsMeshContours::testQuadAndTriangleVertexScalarLine_data()
{
  QTest::addColumn< double >( "value" );
  QTest::addColumn< QgsGeometry >( "expected" );

  QTest::newRow( "left_edge" ) << 1.0 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "LineStringZ (1000 3000 10, 1000 2000 20)" ) );
  QTest::newRow( "two_intersections" ) <<  1.5 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "LineStringZ (1500 3000 30, 1500 2500 35, 1500 2000 25)" ) );
  QTest::newRow( "between two triangles" ) <<  2.0 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "LineStringZ (2000 2000 30, 2000 3000 50)" ) );

  QTest::newRow( "only one point" ) <<  3.0 << QgsGeometry();
  QTest::newRow( "outside lower" ) << -3.0 << QgsGeometry();
  QTest::newRow( "outside upper" ) <<  4.0 << QgsGeometry();
}

void TestQgsMeshContours::testQuadAndTriangleVertexScalarLine()
{
  QFETCH( double, value );
  QFETCH( QgsGeometry, expected );

  const QgsMeshDatasetIndex datasetIndex( 1, 0 );

  QgsMeshContours contours( mpMeshLayer );

  const QgsGeometry res = contours.exportLines( datasetIndex, value, QgsMeshRendererScalarSettings::None );
  equals( res, expected );
}

void TestQgsMeshContours::testQuadAndTriangleFaceScalarLine_data()
{
  QTest::addColumn< double >( "value" );
  QTest::addColumn< QgsGeometry >( "expected" );

  QTest::newRow( "left_edge" ) <<  1.0 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "LineStringZ (1000 3000 10, 1000 2000 20)" ) );
  QTest::newRow( "two_intersections" ) << 1.25 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "LineStringZ (1500 3000 30, 1500 2500 35, 1500 2000 25)" ) );
  QTest::newRow( "between two triangles" ) <<  1.5 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "LineStringZ (2000 2000 30, 2000 3000 50)" ) );

  QTest::newRow( "only one point" ) <<  2.0 << QgsGeometry();
  QTest::newRow( "outside lower" ) <<  -3.0 << QgsGeometry();
  QTest::newRow( "outside upper" ) <<  3.0 << QgsGeometry();
}

void TestQgsMeshContours::testQuadAndTriangleFaceScalarLine()
{
  QFETCH( double, value );
  QFETCH( QgsGeometry, expected );

  const QgsMeshDatasetIndex datasetIndex( 2, 0 );

  QgsMeshContours contours( mpMeshLayer );

  const QgsGeometry res = contours.exportLines( datasetIndex, value, QgsMeshRendererScalarSettings::NeighbourAverage );
  equals( res, expected );
}

void TestQgsMeshContours::testQuadAndTriangleVertexScalarPoly_data()
{
  QTest::addColumn< double >( "min_value" );
  QTest::addColumn< double >( "max_value" );
  QTest::addColumn< QgsGeometry >( "expected" );

  QTest::newRow( "left" ) <<  1.0 << 1.5 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "PolygonZ ((1500 2500 35, 1500 2000 25, 1000 2000 20, 1000 3000 10, 1500 3000 30, 1500 2500 35))" ) );
  QTest::newRow( "bothinsquare" ) <<  1.2 << 1.4 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "PolygonZ ((1400 2400 32, 1400 2000 24, 1200 2000 22, 1200 2200 26, 1200 3000 18, 1400 3000 25.99999999999999645, 1400 2400 32))" ) );
  QTest::newRow( "middle" ) <<  1.5 << 2.0 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "PolygonZ ((2000 3000 50, 2000 2000 30, 1500 2000 25, 1500 2500 35, 1500 3000 30, 2000 3000 50))" ) );
  QTest::newRow( "right" ) <<  2.0 << 2.5 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "PolygonZ ((2500 2500 45, 2000 3000 50, 2000 2000 30, 2500 2000 35, 2500 2500 45))" ) );

  QTest::newRow( "outside_left_edge" ) <<  0.0 << 1.0 << QgsGeometry();
  QTest::newRow( "only one point" ) << 3.0 << 3.0 << QgsGeometry();
  QTest::newRow( "outside lower" ) <<  -3.0 << -4.0 << QgsGeometry();
  QTest::newRow( "outside upper" ) << 4.0 << 5.0 << QgsGeometry();
}

void TestQgsMeshContours::testQuadAndTriangleVertexScalarPoly()
{
  QFETCH( double, min_value );
  QFETCH( double, max_value );
  QFETCH( QgsGeometry, expected );

  const QgsMeshDatasetIndex datasetIndex( 1, 0 );

  QgsMeshContours contours( mpMeshLayer );

  const QgsGeometry res = contours.exportPolygons( datasetIndex, min_value, max_value, QgsMeshRendererScalarSettings::None );
  equals( res, expected );
}

void TestQgsMeshContours::testQuadAndTriangleFaceScalarPoly_data()
{
  QTest::addColumn< double >( "min_value" );
  QTest::addColumn< double >( "max_value" );
  QTest::addColumn< QgsGeometry >( "expected" );

  QTest::newRow( "left" ) << 1.0 << 1.25 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "PolygonZ ((1500 2500 35, 1500 2000 25, 1000 2000 20, 1000 3000 10, 1500 3000 30, 1500 2500 35))" ) );
  QTest::newRow( "middle" ) << 1.25 << 1.5 <<  QgsGeometry( QgsGeometryFactory::geomFromWkt( "PolygonZ ((2000 3000 50, 2000 2000 30, 1500 2000 25, 1500 2500 35, 1500 3000 30, 2000 3000 50))" ) );
  QTest::newRow( "right" ) <<  1.5 << 1.75 << QgsGeometry( QgsGeometryFactory::geomFromWkt( "PolygonZ ((2500 2500 45, 2000 3000 50, 2000 2000 30, 2500 2000 35, 2500 2500 45))" ) );

  QTest::newRow( "only one point" ) << 2.0 << 2.0 << QgsGeometry();
  QTest::newRow( "outside lower" ) << -3.0 << -4.0 << QgsGeometry();
  QTest::newRow( "outside upper" ) << 3.0 << 4.0 << QgsGeometry();
}

void TestQgsMeshContours::testQuadAndTriangleFaceScalarPoly()
{
  QFETCH( double, min_value );
  QFETCH( double, max_value );
  QFETCH( QgsGeometry, expected );

  const QgsMeshDatasetIndex datasetIndex( 2, 0 );

  QgsMeshContours contours( mpMeshLayer );

  const QgsGeometry res = contours.exportPolygons( datasetIndex, min_value, max_value, QgsMeshRendererScalarSettings::NeighbourAverage );
  equals( res, expected );
}

QGSTEST_MAIN( TestQgsMeshContours )
#include "testqgsmeshcontours.moc"
