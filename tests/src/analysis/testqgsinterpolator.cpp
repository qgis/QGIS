/***************************************************************************
  testqgsinterpolator.cpp
  -----------------------
Date                 : November 2017
Copyright            : (C) 2017 by Nyall Dawson
Email                : nyall dot dawson at gmail dot com
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

class TestQgsInterpolator : public QObject
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

void  TestQgsInterpolator::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsInterpolator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsInterpolator::init()
{}

void TestQgsInterpolator::cleanup()
{}

void TestQgsInterpolator::dualEdge()
{
  QgsDualEdgeTriangulation tri;
  QVERIFY( !tri.point( 0 ) );
  QVERIFY( !tri.point( 1 ) );
  QCOMPARE( tri.pointsCount(), 0 );

  tri.addPoint( QgsPoint( 1, 2, 3 ) );
  QCOMPARE( *tri.point( 0 ), QgsPoint( 1, 2, 3 ) );
  QCOMPARE( tri.pointsCount(), 1 );

  tri.addPoint( QgsPoint( 3, 0, 4 ) );
  QCOMPARE( *tri.point( 1 ), QgsPoint( 3, 0, 4 ) );
  QCOMPARE( tri.pointsCount(), 2 );

  tri.addPoint( QgsPoint( 4, 4, 5 ) );
  QCOMPARE( *tri.point( 2 ), QgsPoint( 4, 4, 5 ) );
  QCOMPARE( tri.pointsCount(), 3 );

  QgsPoint p1( 0, 0, 0 );
  QgsPoint p2( 0, 0, 0 );
  QgsPoint p3( 0, 0, 0 );
  int n1 = 0;
  int n2 = 0;
  int n3 = 0;
  QVERIFY( !tri.pointInside( 0, 1 ) );
  QVERIFY( !tri.triangleVertices( 0, 1, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 0, 1, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 1, 1 ) );
  QVERIFY( !tri.triangleVertices( 1, 1, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 1, 1, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 4, 1 ) );
  QVERIFY( !tri.triangleVertices( 4, 1, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 4, 1, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 2, 4 ) );
  QVERIFY( !tri.triangleVertices( 2, 4, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 2, 4, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 3, -1 ) );
  QVERIFY( !tri.triangleVertices( 3, -1, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 3, -1, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( tri.pointInside( 2, 2 ) );
  QVERIFY( tri.triangleVertices( 2, 2, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QVERIFY( tri.triangleVertices( 2, 2, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );
  QVERIFY( tri.pointInside( 3, 1 ) );
  QVERIFY( tri.triangleVertices( 3, 1, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QVERIFY( tri.triangleVertices( 3, 1, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );
  QVERIFY( tri.pointInside( 3.5, 3.5 ) );
  QVERIFY( tri.triangleVertices( 3.5, 3.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QVERIFY( tri.triangleVertices( 3.5, 3.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );

  QCOMPARE( tri.oppositePoint( 0, 1 ), -1 );
  QCOMPARE( tri.oppositePoint( 0, 2 ), 1 );
  QCOMPARE( tri.oppositePoint( 1, 0 ), 2 );
  QCOMPARE( tri.oppositePoint( 1, 2 ), -1 );
  QCOMPARE( tri.oppositePoint( 2, 0 ), -1 );
  QCOMPARE( tri.oppositePoint( 2, 1 ), 0 );

  // add another point
  tri.addPoint( QgsPoint( 2, 4, 6 ) );
  QCOMPARE( *tri.point( 3 ), QgsPoint( 2, 4, 6 ) );
  QCOMPARE( tri.pointsCount(), 4 );
  QVERIFY( !tri.pointInside( 2, 4.5 ) );
  QVERIFY( !tri.triangleVertices( 2, 4.5, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 2, 4.5, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 1, 4 ) );
  QVERIFY( !tri.triangleVertices( 1, 4, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 1, 4, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( tri.pointInside( 2, 3.5 ) );
  QVERIFY( tri.triangleVertices( 2, 3.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( p3, QgsPoint( 2, 4, 6 ) );
  QVERIFY( tri.triangleVertices( 2, 3.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( p3, QgsPoint( 2, 4, 6 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 2 );
  QCOMPARE( n3, 3 );
  QVERIFY( tri.triangleVertices( 2, 2, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );

  QCOMPARE( tri.oppositePoint( 0, 1 ), -1 );
  QCOMPARE( tri.oppositePoint( 0, 2 ), 1 );
  QCOMPARE( tri.oppositePoint( 0, 3 ), 2 );
  QCOMPARE( tri.oppositePoint( 1, 0 ), 2 );
  QCOMPARE( tri.oppositePoint( 1, 2 ), -1 );
  QCOMPARE( tri.oppositePoint( 1, 3 ), -10 );
  QCOMPARE( tri.oppositePoint( 2, 0 ), 3 );
  QCOMPARE( tri.oppositePoint( 2, 1 ), 0 );
  QCOMPARE( tri.oppositePoint( 2, 3 ), -1 );
  QCOMPARE( tri.oppositePoint( 3, 0 ), -1 );
  QCOMPARE( tri.oppositePoint( 3, 1 ), -10 );
  QCOMPARE( tri.oppositePoint( 3, 2 ), 0 );


  // add another point
  tri.addPoint( QgsPoint( 2, 2, 7 ) );
  QCOMPARE( *tri.point( 4 ), QgsPoint( 2, 2, 7 ) );
  QCOMPARE( tri.pointsCount(), 5 );
  QVERIFY( !tri.pointInside( 2, 4.5 ) );
  QVERIFY( !tri.triangleVertices( 2, 4.5, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 2, 4.5, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( !tri.pointInside( 1, 4 ) );
  QVERIFY( !tri.triangleVertices( 1, 4, p1, p2, p3 ) );
  QVERIFY( !tri.triangleVertices( 1, 4, p1, n1, p2, n2, p3, n3 ) );
  QVERIFY( tri.pointInside( 2, 3.5 ) );
  QVERIFY( tri.triangleVertices( 2, 3.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 2, 4, 6 ) );
  QCOMPARE( p2, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p3, QgsPoint( 2, 2, 7 ) );
  QVERIFY( tri.triangleVertices( 2, 3.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 2, 4, 6 ) );
  QCOMPARE( p2, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p3, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( n1, 3 );
  QCOMPARE( n2, 0 );
  QCOMPARE( n3, 4 );
  QVERIFY( tri.pointInside( 2, 1.5 ) );
  QVERIFY( tri.triangleVertices( 2, 1.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 2, 2, 7 ) );
  QVERIFY( tri.triangleVertices( 2, 1.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 1, 2, 3 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( n1, 0 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 4 );
  QVERIFY( tri.pointInside( 3.1, 1 ) );
  QVERIFY( tri.triangleVertices( 3.1, 1, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QVERIFY( tri.triangleVertices( 3.1, 1, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( p2, QgsPoint( 3, 0, 4 ) );
  QCOMPARE( p3, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( n1, 4 );
  QCOMPARE( n2, 1 );
  QCOMPARE( n3, 2 );
  QVERIFY( tri.pointInside( 2.5, 3.5 ) );
  QVERIFY( tri.triangleVertices( 2.5, 3.5, p1, p2, p3 ) );
  QCOMPARE( p1, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( p2, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( p3, QgsPoint( 2, 4, 6 ) );
  QVERIFY( tri.triangleVertices( 2.5, 3.5, p1, n1, p2, n2, p3, n3 ) );
  QCOMPARE( p1, QgsPoint( 2, 2, 7 ) );
  QCOMPARE( p2, QgsPoint( 4, 4, 5 ) );
  QCOMPARE( p3, QgsPoint( 2, 4, 6 ) );
  QCOMPARE( n1, 4 );
  QCOMPARE( n2, 2 );
  QCOMPARE( n3, 3 );

  QCOMPARE( tri.oppositePoint( 0, 1 ), -1 );
  QCOMPARE( tri.oppositePoint( 0, 2 ), -10 );
  QCOMPARE( tri.oppositePoint( 0, 3 ), 4 );
  QCOMPARE( tri.oppositePoint( 0, 4 ), 1 );
  QCOMPARE( tri.oppositePoint( 1, 0 ), 4 );
  QCOMPARE( tri.oppositePoint( 1, 2 ), -1 );
  QCOMPARE( tri.oppositePoint( 1, 3 ), -10 );
  QCOMPARE( tri.oppositePoint( 1, 4 ), 2 );
  QCOMPARE( tri.oppositePoint( 2, 0 ), -10 );
  QCOMPARE( tri.oppositePoint( 2, 1 ), 4 );
  QCOMPARE( tri.oppositePoint( 2, 3 ), -1 );
  QCOMPARE( tri.oppositePoint( 2, 4 ), 3 );
  QCOMPARE( tri.oppositePoint( 3, 0 ), -1 );
  QCOMPARE( tri.oppositePoint( 3, 1 ), -10 );
  QCOMPARE( tri.oppositePoint( 3, 2 ), 4 );
  QCOMPARE( tri.oppositePoint( 3, 4 ), 0 );
  QCOMPARE( tri.oppositePoint( 4, 0 ), 3 );
  QCOMPARE( tri.oppositePoint( 4, 1 ), 0 );
  QCOMPARE( tri.oppositePoint( 4, 2 ), 1 );
  QCOMPARE( tri.oppositePoint( 4, 3 ), 2 );

//  QVERIFY( tri.getSurroundingTriangles( 0 ).empty() );

  QgsMesh mesh = tri.triangulationToMesh();
  QCOMPARE( mesh.faceCount(), 4 );
  QCOMPARE( mesh.vertexCount(), 5 );

  QCOMPARE( mesh.vertex( 0 ), QgsMeshVertex( 1.0, 2.0, 3.0 ) );
  QCOMPARE( mesh.vertex( 1 ), QgsMeshVertex( 3.0, 0.0, 4.0 ) );
  QCOMPARE( mesh.vertex( 2 ), QgsMeshVertex( 4.0, 4.0, 5.0 ) );
  QCOMPARE( mesh.vertex( 3 ), QgsMeshVertex( 2.0, 4.0, 6.0 ) );
  QCOMPARE( mesh.vertex( 4 ), QgsMeshVertex( 2.0, 2.0, 7.0 ) );

  QVERIFY( QgsMesh::compareFaces( mesh.face( 0 ), QgsMeshFace( {0, 4, 3} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 1 ), QgsMeshFace( {4, 2, 3} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 2 ), QgsMeshFace( {1, 4, 0} ) ) );
  QVERIFY( QgsMesh::compareFaces( mesh.face( 3 ), QgsMeshFace( {2, 4, 1} ) ) );
}


QGSTEST_MAIN( TestQgsInterpolator )
#include "testqgsinterpolator.moc"
