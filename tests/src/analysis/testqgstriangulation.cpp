/***************************************************************************
  testqgsinterpolator.cpp
  -----------------------
Date                 : August 2020
Copyright            : (C) 2020 by Vincent Cloarec
Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsdualedgetriangulation.h"
#include "qgsmeshtriangulation.h"
#include "qgsvectorlayer.h"

class TestQgsTriangulation : public QObject
{
    Q_OBJECT

  public:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction.
    void dualEdge();

    void meshTriangulation();

  private:
};

void  TestQgsTriangulation::initTestCase()
{}

void TestQgsTriangulation::cleanupTestCase()
{}

void TestQgsTriangulation::init()
{}

void TestQgsTriangulation::cleanup()
{}

void TestQgsTriangulation::dualEdge()
{
  //3 points
  QgsDualEdgeTriangulation triangulation;
  // Add colinear points
  triangulation.addPoint( QgsPoint( 1, 0, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  QgsMesh mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 2 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 3 );
  QCOMPARE( mesh.faceCount(), 1 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 2, 1} ) ) );

  //4 points
  triangulation = QgsDualEdgeTriangulation();
  // Add colinear points
  triangulation.addPoint( QgsPoint( 1, 0, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 2 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  triangulation.addPoint( QgsPoint( 2, 3, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 4 );
  QCOMPARE( mesh.faceCount(), 2 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 2, 1} ) ) );

  //3 first colinear points
  triangulation = QgsDualEdgeTriangulation();
  triangulation.addPoint( QgsPoint( 1, 0, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 3 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 4 );
  QCOMPARE( mesh.faceCount(), 2 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 1} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {3, 2, 1} ) ) );
  triangulation.addPoint( QgsPoint( 2, 3, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 5 );
  QCOMPARE( mesh.faceCount(), 3 );

  //3 first colinear points with different order
  triangulation = QgsDualEdgeTriangulation();
  triangulation.addPoint( QgsPoint( 1, 0, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 2, 0 ) );
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 3 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 4 );
  QCOMPARE( mesh.faceCount(), 2 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 2} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {1, 2, 3} ) ) );
  triangulation.addPoint( QgsPoint( 2, 3, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 5 );
  QCOMPARE( mesh.faceCount(), 3 );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 2} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {1, 3, 4} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 2 ), QgsMeshFace( {1, 2, 3} ) ) );

  //4 first colinear points
  triangulation = QgsDualEdgeTriangulation();
  triangulation.addPoint( QgsPoint( 1, 1, 0 ) );
  triangulation.addPoint( QgsPoint( 2, 1, 0 ) );
  triangulation.addPoint( QgsPoint( 3, 1, 0 ) );
  triangulation.addPoint( QgsPoint( 4, 1, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 4 );
  QCOMPARE( mesh.faceCount(), 0 );
  triangulation.addPoint( QgsPoint( 1, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 5 );
  QCOMPARE( mesh.faceCount(), 3 );
  triangulation.addPoint( QgsPoint( 2, 2, 0 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.vertices.count(), 6 );
  QCOMPARE( mesh.faceCount(), 4 );

  triangulation = QgsDualEdgeTriangulation();
  triangulation.addPoint( QgsPoint( 2, 0, 1 ) );
  triangulation.addPoint( QgsPoint( 0, 2, 1 ) );
  triangulation.addPoint( QgsPoint( 2, 4, 1 ) );
  triangulation.addPoint( QgsPoint( 4, 2, 1 ) );

  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.faceCount(), 2 );
  QCOMPARE( mesh.vertexCount(), 4 );

  //add point exactly on existing edge
  triangulation.addPoint( QgsPoint( 2, 2, 1 ) );
  mesh = triangulation.triangulationToMesh();
  QCOMPARE( mesh.faceCount(), 4 );
  QCOMPARE( mesh.vertexCount(), 5 );
}

void TestQgsTriangulation::meshTriangulation()
{
  QgsMeshTriangulation meshTri;

  meshTri.setCrs( QgsCoordinateReferenceSystem( "EPSG:32620" ) );

  QgsVectorLayer *mLayerPointZ = new QgsVectorLayer( QStringLiteral( "PointZ?crs=IGNF:GUAD48UTM20" ),
      QStringLiteral( "point Z" ),
      QStringLiteral( "memory" ) );

  QString wkt1 = "PointZ (684486.0 1761297.0 1)";
  QString wkt2 = "PointZ (684276.0 1761309.0 2)";
  QString wkt3 = "PointZ (684098.0 1761401.0 3)";
  QString wkt4 = "PointZ (684292.0 1761406.0 4)";

  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );
  QgsFeature f2;
  f2.setGeometry( QgsGeometry::fromWkt( wkt2 ) );
  QgsFeature f3;
  f3.setGeometry( QgsGeometry::fromWkt( wkt3 ) );
  QgsFeature f4;
  f4.setGeometry( QgsGeometry::fromWkt( wkt4 ) );

  QgsFeatureList flist;
  flist << f1 << f2 << f3 << f4;
  mLayerPointZ->dataProvider()->addFeatures( flist );

  QgsCoordinateTransformContext transformContext;
  meshTri.addVertices( mLayerPointZ, -1, transformContext );

  QgsMesh mesh = meshTri.triangulatedMesh();

  QCOMPARE( mesh.vertexCount(), 4 );
  QCOMPARE( mesh.faceCount(), 2 );

  QCOMPARE( mesh.vertex( 0 ), QgsMeshVertex( 684063.6227548943, 1760993.6560628675, 1 ) );
  QCOMPARE( mesh.vertex( 1 ), QgsMeshVertex( 683853.6219030637, 1761005.6564316382, 2 ) );
  QCOMPARE( mesh.vertex( 2 ), QgsMeshVertex( 683675.6212958666, 1761097.6571173898, 3 ) );
  QCOMPARE( mesh.vertex( 3 ), QgsMeshVertex( 683869.6222287047, 1761102.6569346827, 4 ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 1} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {1, 3, 2} ) ) );

  QString wkt5 = "LineStringZ (683675.6212958666 1761097.6571173898 3,683787.86359334166627377 1761044.579075972083956 7,683921.42213789699599147 1761070.09025864000432193 8,684063.6227548943 1760993.6560628675 1)";

  QgsVectorLayer *mLayerBreakLine = new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=EPSG:32620" ),
      QStringLiteral( "line" ),
      QStringLiteral( "memory" ) );

  QgsFeature f5;
  f5.setGeometry( QgsGeometry::fromWkt( wkt5 ) );
  mLayerBreakLine->dataProvider()->addFeature( f5 );
  meshTri.addBreakLines( mLayerBreakLine, -1, transformContext );

  mesh = meshTri.triangulatedMesh();

  QCOMPARE( mesh.vertexCount(), 6 );
  QCOMPARE( mesh.faceCount(), 6 );

  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 3, 5} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {0, 5, 1} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 2 ), QgsMeshFace( {1, 4, 2} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 3 ), QgsMeshFace( {3, 4, 5} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 4 ), QgsMeshFace( {1, 5, 4} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 5 ), QgsMeshFace( {2, 4, 3} ) ) );
}

QGSTEST_MAIN( TestQgsTriangulation )
#include "testqgstriangulation.moc"
