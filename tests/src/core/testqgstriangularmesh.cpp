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
};


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
    QgsMeshFace point = { 1 };
    mesh.triangulate( point, 0 );
    QCOMPARE( 0, mesh.mTriangularMesh.faces.size() );
  }

  {
    QgsTriangularMesh mesh;
    QgsMeshFace line = { 1, 2 };
    mesh.triangulate( line, 0 );
    QCOMPARE( 0, mesh.mTriangularMesh.faces.size() );
  }

  {
    QgsTriangularMesh mesh;
    QgsMeshFace triangle = { 1, 2, 3 };
    mesh.triangulate( triangle, 0 );
    QCOMPARE( 1, mesh.mTriangularMesh.faces.size() );
    QgsMeshFace firstTriangle = {2, 3, 1};
    QCOMPARE( firstTriangle, mesh.mTriangularMesh.faces[0] );
  }

  {
    QgsTriangularMesh mesh;
    QgsMeshFace quad = { 1, 2, 3, 4 };
    mesh.triangulate( quad, 0 );
    QCOMPARE( 2, mesh.mTriangularMesh.faces.size() );
    QgsMeshFace firstTriangle = {3, 4, 1};
    QCOMPARE( firstTriangle, mesh.mTriangularMesh.faces[0] );
    QgsMeshFace secondTriangle = {2, 3, 1};
    QCOMPARE( secondTriangle, mesh.mTriangularMesh.faces[1] );
  }

  {
    QgsTriangularMesh mesh;
    QgsMeshFace poly = { 1, 2, 3, 4, 5, 6, 7 };
    mesh.triangulate( poly, 0 );
    QCOMPARE( 5, mesh.mTriangularMesh.faces.size() );
  }
}

QGSTEST_MAIN( TestQgsTriangularMesh )
#include "testqgstriangularmesh.moc"
