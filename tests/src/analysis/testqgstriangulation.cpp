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

QGSTEST_MAIN( TestQgsTriangulation )
#include "testqgstriangulation.moc"
