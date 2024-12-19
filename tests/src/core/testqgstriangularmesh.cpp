/***************************************************************************
                         testqgstriangularmesh.cpp
                         -------------------------
    begin                : January 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>

//qgis includes...
#include "qgstriangularmesh.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgis.h"

/**
 * \ingroup UnitTests
 * This is a unit test for a triangular mesh
 */
class TestQgsTriangularMesh : public QObject
{
    Q_OBJECT

  public:
    TestQgsTriangularMesh() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_triangulate();

    void test_centroids();

  private:
    void populateMeshVertices( QgsTriangularMesh &mesh );

};

void TestQgsTriangularMesh::populateMeshVertices( QgsTriangularMesh &mesh )
{
  mesh.mTriangularMesh.vertices.append( QgsMeshVertex( 0, 0, 0 ) );
  mesh.mTriangularMesh.vertices.append( QgsMeshVertex( 0, 1, 0 ) );
  mesh.mTriangularMesh.vertices.append( QgsMeshVertex( 0, 1, 0 ) );
  mesh.mTriangularMesh.vertices.append( QgsMeshVertex( 1, 1, 0 ) );
  mesh.mTriangularMesh.vertices.append( QgsMeshVertex( 2, 0, 0 ) );
  mesh.mTriangularMesh.vertices.append( QgsMeshVertex( 2, 1, 0 ) );
  mesh.mTriangularMesh.vertices.append( QgsMeshVertex( 0, 2, 0 ) );
  mesh.mTriangularMesh.vertices.append( QgsMeshVertex( 1, 2, 0 ) );
}

void TestQgsTriangularMesh::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsTriangularMesh::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTriangularMesh::test_triangulate()
{
  {
    QgsTriangularMesh mesh;
    populateMeshVertices( mesh );
    const QgsMeshFace point = { 0 };
    mesh.triangulate( point, 0 );
    QCOMPARE( 0, mesh.mTriangularMesh.faces.size() );
  }

  {
    QgsTriangularMesh mesh;
    populateMeshVertices( mesh );
    const QgsMeshFace line = { 0, 1 };
    mesh.triangulate( line, 0 );
    QCOMPARE( 0, mesh.mTriangularMesh.faces.size() );
  }

  {
    QgsTriangularMesh mesh;
    populateMeshVertices( mesh );
    const QgsMeshFace triangle = { 0, 1, 2 };
    mesh.triangulate( triangle, 0 );
    QCOMPARE( 1, mesh.mTriangularMesh.faces.size() );
    const QgsMeshFace firstTriangle = {1, 2, 0};
    QCOMPARE( firstTriangle, mesh.mTriangularMesh.faces[0] );
  }

  {
    QgsTriangularMesh mesh;
    populateMeshVertices( mesh );
    const QgsMeshFace quad = { 0, 1, 2, 3 };
    mesh.triangulate( quad, 0 );
    QCOMPARE( 2, mesh.mTriangularMesh.faces.size() );
    const QgsMeshFace firstTriangle = {2, 3, 0};
    QCOMPARE( firstTriangle, mesh.mTriangularMesh.faces[0] );
    const QgsMeshFace secondTriangle = {1, 2, 0};
    QCOMPARE( secondTriangle, mesh.mTriangularMesh.faces[1] );
  }

  {
    QgsTriangularMesh mesh;
    populateMeshVertices( mesh );
    const QgsMeshFace poly = { 1, 2, 3, 4, 5, 6, 7 };
    mesh.triangulate( poly, 0 );
    QCOMPARE( 5, mesh.mTriangularMesh.faces.size() );
  }
}

void TestQgsTriangularMesh::test_centroids()
{
  QgsTriangularMesh triangularMesh;

  QgsMesh nativeMesh;
  nativeMesh.vertices << QgsMeshVertex( 0, 10, 0 ) << QgsMeshVertex( 10, 10, 0 ) << QgsMeshVertex( 10, 0, 0 ) << QgsMeshVertex( 0, 0, 0 )
                      << QgsMeshVertex( 20, 0, 0 ) << QgsMeshVertex( 30, 10, 0 ) << QgsMeshVertex( 20, 10, 0 );

  nativeMesh.faces << QgsMeshFace( {0, 1, 2, 3} ) << QgsMeshFace( {1, 2, 4, 5} );

  triangularMesh.update( &nativeMesh, QgsCoordinateTransform() );

  QVector<QgsMeshVertex> centroids = triangularMesh.faceCentroids();

  QCOMPARE( 2, centroids.count() );

  QVERIFY( qgsDoubleNear( centroids.at( 0 ).x(), 5, 0.00001 ) );
  QVERIFY( qgsDoubleNear( centroids.at( 0 ).y(), 5, 0.00001 ) );
  QVERIFY( qgsDoubleNear( centroids.at( 1 ).x(), 17.777777777, 0.00001 ) );
  QVERIFY( qgsDoubleNear( centroids.at( 1 ).y(), 5.5555555555, 0.00001 ) );

  // with big coordinates
  nativeMesh.clear();
  triangularMesh = QgsTriangularMesh();

  nativeMesh.vertices << QgsMeshVertex( 900000000, 300000010, 0 ) << QgsMeshVertex( 900000010, 300000010, 0 ) << QgsMeshVertex( 900000010, 300000000, 0 ) << QgsMeshVertex( 900000000, 300000000, 0 )
                      << QgsMeshVertex( 900000020, 300000000, 0 ) << QgsMeshVertex( 900000030, 300000010, 0 ) << QgsMeshVertex( 900000020, 300000010, 0 );

  nativeMesh.faces << QgsMeshFace( {0, 1, 2, 3} ) << QgsMeshFace( {1, 2, 4, 5} );
  triangularMesh.update( &nativeMesh, QgsCoordinateTransform() );

  centroids = triangularMesh.faceCentroids();

  QCOMPARE( 2, centroids.count() );

  QVERIFY( qgsDoubleNear( centroids.at( 0 ).x(), 900000005, 0.00001 ) );
  QVERIFY( qgsDoubleNear( centroids.at( 0 ).y(), 300000005, 0.00001 ) );
  QVERIFY( qgsDoubleNear( centroids.at( 1 ).x(), 900000017.777777, 0.00001 ) );
  QVERIFY( qgsDoubleNear( centroids.at( 1 ).y(), 300000005.555555, 0.00001 ) );

}

QGSTEST_MAIN( TestQgsTriangularMesh )
#include "testqgstriangularmesh.moc"
