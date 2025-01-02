/***************************************************************************
     testqgstriangulatedsurface.cpp
     --------------------------------------
    Date                 : August 2024
    Copyright            : (C) 2024 by Jean Felder
    Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgssurface.h"
#include "qgstest.h"
#include <QObject>
#include <QPainter>
#include <QString>
#include <qtestcase.h>

#include "qgstriangle.h"
#include "qgstriangulatedsurface.h"
#include "testgeometryutils.h"

class TestQgsTriangulatedSurface : public QObject
{
    Q_OBJECT

  private slots:
    void testConstructor();
    void testCopyConstructor();
    void testClear();
    void testClone();
    void testEquality();
    void testAddPatch();
    void testRemovePatch();
    void testPatches();
    void testAreaPerimeter();
    void testInsertVertex();
    void testMoveVertex();
    void testDeleteVertex();
    void testNextVertex();
    void testVertexAngle();
    void testVertexNumberFromVertexId();
    void testHasCurvedSegments();
    void testClosestSegment();
    void testBoundary();
    void testBoundingBox();
    void testBoundingBox3D();
    void testBoundingBoxIntersects();
    void testDropZValue();
    void testDropMValue();
    void testWKB();
    void testWKT();
    void testExport();
    void testCast();
};

void TestQgsTriangulatedSurface::testConstructor()
{
  QgsTriangulatedSurface surface;

  QVERIFY( surface.isEmpty() );
  QCOMPARE( surface.nCoordinates(), 0 );
  QCOMPARE( surface.ringCount(), 0 );
  QCOMPARE( surface.numPatches(), 0 );
  QCOMPARE( surface.partCount(), 0 );
  QVERIFY( !surface.is3D() );
  QVERIFY( !surface.isMeasure() );
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QCOMPARE( surface.wktTypeStr(), QString( "TIN" ) );
  QCOMPARE( surface.geometryType(), QString( "TIN" ) );
  QCOMPARE( surface.dimension(), 2 );
  QVERIFY( !surface.hasCurvedSegments() );
  QCOMPARE( surface.area(), 0.0 );
  QCOMPARE( surface.perimeter(), 0.0 );
  QVERIFY( !surface.patchN( 0 ) );
}

void TestQgsTriangulatedSurface::testCopyConstructor()
{
  QgsTriangulatedSurface surface1;
  QCOMPARE( surface1.numPatches(), 0 );

  QgsTriangulatedSurface surface2( surface1 );
  QCOMPARE( surface1, surface2 );

  // add a triangle to surface1
  QgsTriangle triangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  surface1.addPatch( triangle.clone() );
  QCOMPARE( surface1.numPatches(), 1 );

  QgsTriangulatedSurface polySurface3( surface1 );
  QCOMPARE( surface1, polySurface3 );

  QgsTriangulatedSurface surface4;
  surface4 = surface2;
  QCOMPARE( surface2, surface4 );
  surface4 = surface1;
  QCOMPARE( surface1, surface4 );
}

void TestQgsTriangulatedSurface::testClear()
{
  QgsTriangulatedSurface surface;

  QgsTriangle *triangle = new QgsTriangle( QgsPoint( 0, 0, 1 ), QgsPoint( 0, 10, 1 ), QgsPoint( 10, 10, 1 ) );
  surface.addPatch( triangle );

  QVERIFY( !surface.isEmpty() );
  QCOMPARE( surface.numPatches(), 1 );
  QCOMPARE( surface.nCoordinates(), 4 );
  QCOMPARE( surface.numPatches(), 1 );
  QCOMPARE( surface.partCount(), 1 );
  QVERIFY( surface.is3D() );
  QVERIFY( !surface.isMeasure() );
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TINZ );

  surface.clear();
  QVERIFY( surface.isEmpty() );
  QCOMPARE( surface.numPatches(), 0 );
  QCOMPARE( surface.nCoordinates(), 0 );
  QCOMPARE( surface.numPatches(), 0 );
  QCOMPARE( surface.partCount(), 0 );
  QVERIFY( !surface.is3D() );
  QVERIFY( !surface.isMeasure() );
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
}

void TestQgsTriangulatedSurface::testClone()
{
  QgsTriangulatedSurface surface;

  std::unique_ptr<QgsTriangulatedSurface> cloned( surface.clone() );
  QCOMPARE( surface, *cloned );

  QgsTriangle *triangle = new QgsTriangle( QgsPoint( 4, 4 ), QgsPoint( 6, 10 ), QgsPoint( 10, 10 ) );
  surface.addPatch( triangle );

  cloned.reset( surface.clone() );
  QCOMPARE( surface, *cloned );
}

void TestQgsTriangulatedSurface::testEquality()
{
  QgsTriangulatedSurface surface1, surface2;
  QgsTriangle *patch;

  QVERIFY( surface1 == surface2 );
  QVERIFY( !( surface1 != surface2 ) );

  patch = new QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  surface1.addPatch( patch );
  QVERIFY( !( surface1 == surface2 ) );
  QVERIFY( surface1 != surface2 );

  patch = new QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  surface2.addPatch( patch );
  QVERIFY( surface1 == surface2 );
  QVERIFY( !( surface1 != surface2 ) );

  patch = new QgsTriangle( QgsPoint( 10, 0 ), QgsPoint( 10, 10 ), QgsPoint( 15, 10 ) );
  surface2.addPatch( patch );
  QVERIFY( !( surface1 == surface2 ) );
  QVERIFY( surface1 != surface2 );

  surface2.removePatch( 1 );
  QVERIFY( surface1 == surface2 );
  QVERIFY( !( surface1 != surface2 ) );
}

void TestQgsTriangulatedSurface::testAddPatch()
{
  QgsTriangulatedSurface surface;

  // empty surface
  QCOMPARE( surface.numPatches(), 0 );
  QVERIFY( !surface.patchN( -1 ) );
  QVERIFY( !surface.patchN( 0 ) );

  surface.addPatch( nullptr );
  QCOMPARE( surface.numPatches(), 0 );

  // Try to add a Polygon which is not a Triangle
  QgsPolygon *notTriangle = new QgsPolygon();
  QgsLineString notTriangleExterior;
  notTriangleExterior.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( notTriangleExterior.nCoordinates(), 5 );
  notTriangle->setExteriorRing( notTriangleExterior.clone() );
  surface.addPatch( notTriangle );
  QCOMPARE( surface.numPatches(), 0 );


  QgsTriangle *triangle = new QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  surface.addPatch( triangle );
  QCOMPARE( surface.numPatches(), 1 );
  QVERIFY( !surface.is3D() );
  QVERIFY( !surface.isMeasure() );
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QCOMPARE( surface.patchN( 0 ), triangle );
  QVERIFY( !surface.patchN( 1 ) );
  QCOMPARE( surface.nCoordinates(), 4 );

  // try adding a patch with z to a 2d triangulated surface, z should be dropped
  triangle = new QgsTriangle( QgsPoint( 10, 0, 1 ), QgsPoint( 0, 10, 1 ), QgsPoint( 10, 10, 1 ) );
  surface.addPatch( triangle );
  QCOMPARE( surface.numPatches(), 2 );
  QVERIFY( !surface.is3D() );
  QVERIFY( !surface.isMeasure() );
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QVERIFY( surface.patchN( 1 ) );
  QVERIFY( !surface.patchN( 1 )->is3D() );
  QVERIFY( !surface.patchN( 1 )->isMeasure() );
  QCOMPARE( surface.patchN( 1 )->wkbType(), Qgis::WkbType::Triangle );

  // try adding a patch with zm to a 2d surface, z and m should be dropped
  triangle = new QgsTriangle( QgsPoint( 10, 0, 1, 2 ), QgsPoint( 0, 10, 1, 2 ), QgsPoint( 10, 10, 1, 2 ) );
  surface.addPatch( triangle );
  QCOMPARE( surface.numPatches(), 3 );
  QVERIFY( !surface.is3D() );
  QVERIFY( !surface.isMeasure() );
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QVERIFY( surface.patchN( 2 ) );
  QVERIFY( !surface.patchN( 2 )->is3D() );
  QVERIFY( !surface.patchN( 2 )->isMeasure() );
  QCOMPARE( surface.patchN( 2 )->wkbType(), Qgis::WkbType::Triangle );


  // addPatch without z/m to TriangleZM
  QgsTriangulatedSurface surface2;
  triangle = new QgsTriangle( QgsPoint( 10, 0, 1, 2 ), QgsPoint( 0, 10, 1, 2 ), QgsPoint( 10, 10, 1, 2 ) );
  surface2.addTriangle( triangle );

  QCOMPARE( surface2.numPatches(), 1 );
  QVERIFY( surface2.is3D() );
  QVERIFY( surface2.isMeasure() );
  QCOMPARE( surface2.wkbType(), Qgis::WkbType::TINZM );
  QVERIFY( surface2.patchN( 0 ) );
  QVERIFY( surface2.patchN( 0 )->is3D() );
  QVERIFY( surface2.patchN( 0 )->isMeasure() );
  QCOMPARE( surface2.patchN( 0 )->wkbType(), Qgis::WkbType::TriangleZM );
  QCOMPARE( surface2.patchN( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 10, 0, 1, 2 ) );

  // triangle has no z
  triangle = new QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  surface2.addTriangle( triangle );

  QCOMPARE( surface2.numPatches(), 2 );
  QVERIFY( surface2.patchN( 1 ) );
  QVERIFY( surface2.patchN( 1 )->is3D() );
  QVERIFY( surface2.patchN( 1 )->isMeasure() );
  QCOMPARE( surface2.patchN( 1 )->wkbType(), Qgis::WkbType::TriangleZM );
  QCOMPARE( surface2.patchN( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 0, 0, 0, 0 ) );

  // triangle has no m
  triangle = new QgsTriangle( QgsPoint( 10, 0, 1 ), QgsPoint( 0, 10, 1 ), QgsPoint( 10, 10, 1 ) );
  surface2.addTriangle( triangle );

  QCOMPARE( surface2.numPatches(), 3 );
  QVERIFY( surface2.patchN( 2 ) );
  QVERIFY( surface2.patchN( 2 )->is3D() );
  QVERIFY( surface2.patchN( 2 )->isMeasure() );
  QCOMPARE( surface2.patchN( 2 )->wkbType(), Qgis::WkbType::TriangleZM );
  QCOMPARE( surface2.patchN( 2 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 10, 0, 1, 0 ) );
}

void TestQgsTriangulatedSurface::testRemovePatch()
{
  QgsTriangulatedSurface surface;
  QVector<QgsTriangle *> patches;

  QVERIFY( !surface.removePatch( -1 ) );
  QVERIFY( !surface.removePatch( 0 ) );

  QgsTriangle *triangle1 = new QgsTriangle( QgsPoint( 0.1, 0.1 ), QgsPoint( 0.1, 0.2 ), QgsPoint( 0.2, 0.2 ) );
  patches.append( triangle1 );

  QgsTriangle *triangle2 = new QgsTriangle( QgsPoint( 0.2, 0.1 ), QgsPoint( 0.2, 0.2 ), QgsPoint( 0.3, 0.2 ) );
  patches.append( triangle2 );

  QgsTriangle *patch3 = new QgsTriangle( QgsPoint( 0.3, 0.1 ), QgsPoint( 0.3, 0.2 ), QgsPoint( 0.4, 0.2 ) );
  patches.append( patch3 );

  surface.setTriangles( patches );

  QCOMPARE( surface.numPatches(), 3 );
  QCOMPARE( surface.patchN( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.1, 0.1 ) );

  QVERIFY( surface.removePatch( 0 ) );
  QCOMPARE( surface.numPatches(), 2 );
  QCOMPARE( surface.patchN( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.2, 0.1 ) );
  QCOMPARE( surface.patchN( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.3, 0.1 ) );

  QVERIFY( surface.removePatch( 1 ) );
  QCOMPARE( surface.numPatches(), 1 );
  QCOMPARE( surface.patchN( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.2, 0.1 ) );

  QVERIFY( surface.removePatch( 0 ) );
  QCOMPARE( surface.numPatches(), 0 );
  QVERIFY( !surface.removePatch( 0 ) );
}

void TestQgsTriangulatedSurface::testPatches()
{
  QgsTriangulatedSurface surface;
  QVector<QgsTriangle *> triangles;

  QgsTriangle *triangle1 = new QgsTriangle( QgsPoint( 0.1, 0.1, 2 ), QgsPoint( 0.1, 0.2, 2 ), QgsPoint( 0.2, 0.2, 2 ) );
  triangles.append( triangle1 );

  QgsTriangle *triangle2 = new QgsTriangle( QgsPoint( 0.2, 0.1, 2 ), QgsPoint( 0.2, 0.2, 2 ), QgsPoint( 0.3, 0.2, 2 ) );
  triangles.append( triangle2 );

  surface.setTriangles( triangles );

  QVERIFY( !surface.isEmpty() );
  QCOMPARE( surface.numPatches(), 2 );
  QVERIFY( surface.is3D() );
  QVERIFY( !surface.isMeasure() );
  QVERIFY( surface.patchN( 0 )->is3D() );
  QVERIFY( !surface.patchN( 0 )->isMeasure() );
  QVERIFY( surface.patchN( 1 )->is3D() );
  QVERIFY( !surface.patchN( 1 )->isMeasure() );
  surface.clear();
  QVERIFY( surface.isEmpty() );

  // Only the second polygon can be converted to a QgsTriangle
  // The QgsTriangulatedSurface only contains triangle3
  QVector<QgsPolygon *> patchesPolygons;
  QgsPolygon *polygonPatch = new QgsPolygon();
  QgsLineString *exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  polygonPatch->setExteriorRing( exteriorRing );
  patchesPolygons.append( polygonPatch );

  QgsTriangle *triangle3 = new QgsTriangle( QgsPoint( 1, 1 ), QgsPoint( 3, 1 ), QgsPoint( 1.5, 1.5 ) );
  patchesPolygons.append( triangle3 );

  surface.setPatches( patchesPolygons );
  QVERIFY( !surface.isEmpty() );
  QCOMPARE( surface.numPatches(), 1 );
  QVERIFY( !surface.is3D() );
  QVERIFY( !surface.isMeasure() );
  QVERIFY( surface.patchN( 0 ) );
  QVERIFY( surface.triangleN( 0 ) );
  QVERIFY( !surface.patchN( 0 )->is3D() );
  QVERIFY( !surface.patchN( 0 )->isMeasure() );
  QgsPolygon *polygon = surface.patchN( 0 );
  QgsTriangle *triangle = qgsgeometry_cast<QgsTriangle *>( polygon );
  QVERIFY( triangle );
}

void TestQgsTriangulatedSurface::testAreaPerimeter()
{
  QgsTriangulatedSurface surface;

  QgsTriangle *triangle = new QgsTriangle( QgsPoint( 1, 1 ), QgsPoint( 1, 6 ), QgsPoint( 6, 6 ) );
  surface.addPatch( triangle );

  QGSCOMPARENEAR( surface.area(), 12.5, 0.01 ); // area is not implemented
  QGSCOMPARENEAR( surface.perimeter(), 17.07, 0.01 );
}

void TestQgsTriangulatedSurface::testInsertVertex()
{
  QgsTriangulatedSurface surface;
  QgsTriangle triangle;

  // insert vertex in empty surface
  QVERIFY( !surface.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !surface.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !surface.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !surface.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( surface.isEmpty() );

  triangle = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0.5, 0 ), QgsPoint( 1, 0 ) );
  surface.addPatch( triangle.clone() );

  // it is not possible to insert a vertex
  QCOMPARE( surface.nCoordinates(), 4 );
  QVERIFY( !surface.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( surface.nCoordinates(), 4 );
}

void TestQgsTriangulatedSurface::testMoveVertex()
{
  // empty triangulated surface
  QgsTriangulatedSurface surface;
  QVERIFY( !surface.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( surface.isEmpty() );

  // valid surface
  QgsTriangle triangle = QgsTriangle( QgsPoint( 1, 2 ), QgsPoint( 11, 12 ), QgsPoint( 21, 22 ) );
  surface.addPatch( triangle.clone() );

  QVERIFY( surface.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( surface.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( surface.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  // out of range
  QVERIFY( !surface.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !surface.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !surface.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  // add a second triangle
  triangle = QgsTriangle( QgsPoint( 10, 10 ), QgsPoint( 12, 10 ), QgsPoint( 12, 12 ) );
  surface.addPatch( triangle.clone() );

  QVERIFY( surface.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 4.0, 5.0 ) ) );
  QVERIFY( surface.moveVertex( QgsVertexId( 1, 0, 1 ), QgsPoint( 14.0, 15.0 ) ) );
  QVERIFY( surface.moveVertex( QgsVertexId( 1, 0, 2 ), QgsPoint( 24.0, 25.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 1 )->exteriorRing() )->pointN( 0 ), QgsPoint( 4.0, 5.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 1 )->exteriorRing() )->pointN( 1 ), QgsPoint( 14.0, 15.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 1 )->exteriorRing() )->pointN( 2 ), QgsPoint( 24.0, 25.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 1 )->exteriorRing() )->pointN( 3 ), QgsPoint( 4.0, 5.0 ) );

  // out of range
  QVERIFY( !surface.moveVertex( QgsVertexId( 1, 1, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !surface.moveVertex( QgsVertexId( 1, 1, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !surface.moveVertex( QgsVertexId( 1, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !surface.moveVertex( QgsVertexId( 1, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !surface.moveVertex( QgsVertexId( 1, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !surface.moveVertex( QgsVertexId( 2, 0, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 1 )->exteriorRing() )->pointN( 0 ), QgsPoint( 4.0, 5.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 1 )->exteriorRing() )->pointN( 1 ), QgsPoint( 14.0, 15.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 1 )->exteriorRing() )->pointN( 2 ), QgsPoint( 24.0, 25.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 1 )->exteriorRing() )->pointN( 3 ), QgsPoint( 4.0, 5.0 ) );
}

void TestQgsTriangulatedSurface::testDeleteVertex()
{
  // empty surface
  QgsTriangulatedSurface surface;
  QVERIFY( !surface.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !surface.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QVERIFY( surface.isEmpty() );

  // valid surface
  QgsTriangle triangle = QgsTriangle( QgsPoint( 1, 2 ), QgsPoint( 5, 2 ), QgsPoint( 6, 2 ) );
  surface.addPatch( triangle.clone() );

  // it is not possible to delete a vertex
  QVERIFY( !surface.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QVERIFY( !surface.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !surface.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );
  QVERIFY( !surface.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );
}

void TestQgsTriangulatedSurface::testNextVertex()
{
  QgsTriangulatedSurface empty;
  QPainter p;
  empty.draw( p ); // no crash!

  QgsPoint pt;
  QgsVertexId vId;
  ( void ) empty.closestSegment( QgsPoint( 1, 2 ), pt, vId ); // empty segment, just want no crash

  // nextVertex
  QgsTriangulatedSurface surfacePoly;
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, -2 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, 10 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  QgsTriangle triangle = QgsTriangle( QgsPoint( 1, 2 ), QgsPoint( 11, 12 ), QgsPoint( 1, 12 ) );
  surfacePoly.addPatch( triangle.clone() );

  vId = QgsVertexId( 0, 0, 4 ); // out of range
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, -5 );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 1, 12 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );

  vId = QgsVertexId( 0, 1, 0 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 1, 0, 0 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  // add a second triangle
  triangle = QgsTriangle( QgsPoint( 1, 2 ), QgsPoint( 11, 12 ), QgsPoint( 11, 2 ) );
  surfacePoly.addPatch( triangle.clone() );

  vId = QgsVertexId( 1, 1, 7 ); // out of range
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 1, 0, -5 );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 1, 12 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 11, 2 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 2, 0, 0 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );
}

void TestQgsTriangulatedSurface::testVertexAngle()
{
  QgsTriangulatedSurface surface;

  // just want no crash
  ( void ) surface.vertexAngle( QgsVertexId() );
  ( void ) surface.vertexAngle( QgsVertexId( 0, 0, 0 ) );
  ( void ) surface.vertexAngle( QgsVertexId( 0, 1, 0 ) );

  QgsTriangle triangle = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0.5, 0 ), QgsPoint( 1, 2 ) );
  surface.addPatch( triangle.clone() );

  QGSCOMPARENEAR( surface.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.58802, 0.00001 );
  QGSCOMPARENEAR( surface.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.90789, 0.0001 );
  QGSCOMPARENEAR( surface.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 5.0667, 0.00001 );
  QGSCOMPARENEAR( surface.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 2.58802, 0.00001 );
}

void TestQgsTriangulatedSurface::testVertexNumberFromVertexId()
{
  QgsTriangulatedSurface surface;
  QgsTriangle triangle;

  // with only one triangle
  triangle = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 1, 0 ), QgsPoint( 1, 1 ) );
  surface.addPatch( triangle.clone() );

  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), -1 );

  triangle = QgsTriangle( QgsPoint( 1, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ) );
  surface.addPatch( triangle.clone() );

  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), -1 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 4 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), 5 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 2 ) ), 6 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 3 ) ), 7 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 4 ) ), -1 );
  QCOMPARE( surface.vertexNumberFromVertexId( QgsVertexId( 1, 1, 0 ) ), -1 );
}

void TestQgsTriangulatedSurface::testHasCurvedSegments()
{
  QgsTriangulatedSurface surface;
  QVERIFY( !surface.hasCurvedSegments() );

  QgsTriangle triangle = QgsTriangle( QgsPoint( 1, 2 ), QgsPoint( 11, 12 ), QgsPoint( 21, 22 ) );
  surface.addPatch( triangle.clone() );
  QVERIFY( !surface.hasCurvedSegments() );
}

void TestQgsTriangulatedSurface::testClosestSegment()
{
  QgsTriangulatedSurface empty;
  QPainter p;
  empty.draw( p ); // no crash!

  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;
  ( void ) empty.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty segment, just want no crash

  QgsTriangulatedSurface surface;
  QgsTriangle triangle = QgsTriangle( QgsPoint( 5, 10 ), QgsPoint( 7, 12 ), QgsPoint( 5, 15 ) );
  surface.addPatch( triangle.clone() );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ), 2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 2.5, 13 ), pt, v, &leftOf ), 6.25000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.0, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  // point directly on segment
  QCOMPARE( surface.closestSegment( QgsPoint( 5, 15 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  // with a second patch
  triangle = QgsTriangle( QgsPoint( 5, 10 ), QgsPoint( 5, 15 ), QgsPoint( 2, 11 ) );
  surface.addPatch( triangle.clone() );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 0.4, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 3.8, 0.01 );
  QGSCOMPARENEAR( pt.y(), 10.4, 0.01 );
  QCOMPARE( v, QgsVertexId( 1, 0, 3 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ), 2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 2.5, 13 ), pt, v, &leftOf ), 0.64000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 3.14, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12.52, 0.01 );
  QCOMPARE( v, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( surface.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  // point directly on segment
  QCOMPARE( surface.closestSegment( QgsPoint( 2, 11 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 2, 11 ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( leftOf, 0 );
}

void TestQgsTriangulatedSurface::testBoundary()
{
  QgsTriangulatedSurface surface;
  QVERIFY( !surface.boundary() );

  QgsTriangle *triangle1 = new QgsTriangle( QgsPoint( 1, 2 ), QgsPoint( 4, 5 ), QgsPoint( 4, 3 ) );
  surface.addPatch( triangle1 );

  QgsAbstractGeometry *boundary = surface.boundary();
  QVERIFY( surface.boundary() );
  QgsMultiLineString *multiLineBoundary = dynamic_cast<QgsMultiLineString *>( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 1 );
  QgsLineString *lineBoundary = multiLineBoundary->lineStringN( 0 );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->xAt( 0 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 1 ), 4.0 );
  QCOMPARE( lineBoundary->xAt( 2 ), 4.0 );
  QCOMPARE( lineBoundary->xAt( 3 ), 1.0 );
  QCOMPARE( lineBoundary->yAt( 0 ), 2.0 );
  QCOMPARE( lineBoundary->yAt( 1 ), 5.0 );
  QCOMPARE( lineBoundary->yAt( 2 ), 3.0 );
  QCOMPARE( lineBoundary->yAt( 3 ), 2.0 );
  delete boundary;

  surface.removePatch( 0 );
  QCOMPARE( surface.numPatches(), 0 );

  QgsTriangle *triangleZ = new QgsTriangle( QgsPoint( Qgis::WkbType::PointZ, 10, 11, 12 ), QgsPoint( Qgis::WkbType::PointZ, 13, 14, 15 ), QgsPoint( Qgis::WkbType::PointZ, 16, 17, 18 ) );
  surface.addPatch( triangleZ );
  boundary = surface.boundary();
  multiLineBoundary = dynamic_cast<QgsMultiLineString *>( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 1 );
  lineBoundary = multiLineBoundary->lineStringN( 0 );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->wkbType(), Qgis::WkbType::LineStringZ );
  QCOMPARE( lineBoundary->pointN( 0 ).z(), 12.0 );
  QCOMPARE( lineBoundary->pointN( 1 ).z(), 15.0 );
  QCOMPARE( lineBoundary->pointN( 2 ).z(), 18.0 );
  QCOMPARE( lineBoundary->pointN( 3 ).z(), 12.0 );
  delete boundary;
}

void TestQgsTriangulatedSurface::testBoundingBox()
{
  QgsTriangulatedSurface surface;
  QgsRectangle bBox = surface.boundingBox(); // no crash!

  QgsTriangle *triangle1 = new QgsTriangle( QgsPoint( Qgis::WkbType::PointZ, 1, 2, 3 ), QgsPoint( Qgis::WkbType::PointZ, 4, 5, 6 ), QgsPoint( Qgis::WkbType::PointZ, 7, 8, 9 ) );
  surface.addPatch( triangle1 );

  QgsTriangle *triangle2 = new QgsTriangle( QgsPoint( Qgis::WkbType::PointZ, 10, 11, 12 ), QgsPoint( Qgis::WkbType::PointZ, 13, 14, 15 ), QgsPoint( Qgis::WkbType::PointZ, 16, 17, 18 ) );
  surface.addPatch( triangle2 );

  bBox = surface.boundingBox();
  QGSCOMPARENEAR( bBox.xMinimum(), 1.0, 0.001 );
  QGSCOMPARENEAR( bBox.xMaximum(), 16.0, 0.001 );
  QGSCOMPARENEAR( bBox.yMinimum(), 2.0, 0.001 );
  QGSCOMPARENEAR( bBox.yMaximum(), 17.0, 0.001 );
}

void TestQgsTriangulatedSurface::testBoundingBox3D()
{
  QgsTriangulatedSurface surface;
  QgsBox3D bBox = surface.boundingBox3D();
  QCOMPARE( bBox.xMinimum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.xMaximum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.yMinimum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.yMaximum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.zMinimum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.zMaximum(), std::numeric_limits<double>::quiet_NaN() );

  QgsTriangle *triangle = new QgsTriangle( QgsPoint( Qgis::WkbType::PointZ, -1, 0, 6 ), QgsPoint( Qgis::WkbType::PointZ, 1, 10, 2 ), QgsPoint( Qgis::WkbType::PointZ, 0, 18, 3 ) );
  surface.addPatch( triangle );

  bBox = surface.boundingBox3D();
  QGSCOMPARENEAR( bBox.xMinimum(), -1.0, 0.001 );
  QGSCOMPARENEAR( bBox.xMaximum(), 1.0, 0.001 );
  QGSCOMPARENEAR( bBox.yMinimum(), 0.0, 0.001 );
  QGSCOMPARENEAR( bBox.yMaximum(), 18.0, 0.001 );
  QGSCOMPARENEAR( bBox.zMinimum(), 2.0, 0.001 );
  QGSCOMPARENEAR( bBox.zMaximum(), 6.0, 0.001 );
}

void TestQgsTriangulatedSurface::testBoundingBoxIntersects()
{
  // 2d
  QgsTriangulatedSurface surface1;
  QVERIFY( !surface1.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );

  QgsTriangle *triangle1 = new QgsTriangle(
    QgsPoint( Qgis::WkbType::PointZ, 0, 0, 1 ),
    QgsPoint( Qgis::WkbType::PointZ, 1, 10, 2 ),
    QgsPoint( Qgis::WkbType::PointZ, 0, 18, 3 )
  );
  surface1.addPatch( triangle1 );

  QVERIFY( surface1.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !surface1.boundingBoxIntersects( QgsRectangle( 1.1, -5, 6, -2 ) ) );

  // 3d
  QgsTriangulatedSurface surface2;
  QVERIFY( !surface2.boundingBoxIntersects( QgsBox3D( 1, 3, 1, 6, 9, 2 ) ) );

  QgsTriangle *triangle2 = new QgsTriangle( QgsPoint( Qgis::WkbType::PointZ, 0, 0, 1 ), QgsPoint( Qgis::WkbType::PointZ, 1, 10, 2 ), QgsPoint( Qgis::WkbType::PointZ, 0, 18, 3 ) );
  surface2.addPatch( triangle2 );

  QVERIFY( surface2.boundingBoxIntersects( QgsBox3D( 1, 3, 1, 6, 9, 2 ) ) );
  QVERIFY( !surface2.boundingBoxIntersects( QgsBox3D( 1, 3, 4.1, 6, 9, 6 ) ) );
}

void TestQgsTriangulatedSurface::testDropZValue()
{
  QgsTriangulatedSurface surface;
  QgsTriangle triangle;

  // without z
  surface.dropZValue();
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );

  triangle = QgsTriangle( QgsPoint( 1, 2 ), QgsPoint( 1, 4 ), QgsPoint( 4, 4 ) );
  surface.addPatch( triangle.clone() );
  QCOMPARE( surface.numPatches(), 1 );
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );

  surface.dropZValue(); // not z
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::Triangle );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with z
  triangle = QgsTriangle( QgsPoint( 10, 20, 3 ), QgsPoint( 11, 12, 13 ), QgsPoint( 1, 12, 23 ) );
  surface.clear();
  surface.addPatch( triangle.clone() );

  QCOMPARE( surface.wkbType(), Qgis::WkbType::TINZ );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::TriangleZ );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 10, 20, 3 ) );

  surface.dropZValue();
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::Triangle );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 10, 20 ) );

  // with zm
  triangle = QgsTriangle( QgsPoint( 1, 2, 3, 4 ), QgsPoint( 11, 12, 13, 14 ), QgsPoint( 1, 12, 23, 24 ) );
  surface.clear();
  surface.addPatch( triangle.clone() );

  QCOMPARE( surface.wkbType(), Qgis::WkbType::TINZM );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::TriangleZM );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( Qgis::WkbType::PointZM, 1, 2, 3, 4 ) );

  surface.dropZValue();
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TINM );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::TriangleM );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( Qgis::WkbType::PointM, 1, 2, 0, 4 ) );
}

void TestQgsTriangulatedSurface::testDropMValue()
{
  QgsTriangulatedSurface surface;
  QgsTriangle triangle;

  // without z
  surface.dropMValue();
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );

  triangle = QgsTriangle( QgsPoint( 1, 2 ), QgsPoint( 11, 12 ), QgsPoint( 1, 12 ) );
  surface.addPatch( triangle.clone() );

  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );

  surface.dropMValue(); // not zm
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::Triangle );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with m
  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointM, 1, 2, 0, 3 ), QgsPoint( Qgis::WkbType::PointM, 11, 12, 0, 13 ), QgsPoint( Qgis::WkbType::PointM, 1, 12, 0, 23 ) );
  surface.clear();
  surface.addPatch( triangle.clone() );

  QCOMPARE( surface.wkbType(), Qgis::WkbType::TINM );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::TriangleM );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( Qgis::WkbType::PointM, 1, 2, 0, 3 ) );

  surface.dropMValue();
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TIN );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::Triangle );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with zm
  triangle = QgsTriangle( QgsPoint( 1, 2, 3, 4 ), QgsPoint( 11, 12, 13, 14 ), QgsPoint( 1, 12, 23, 24 ) );
  surface.clear();
  surface.addPatch( triangle.clone() );

  QCOMPARE( surface.wkbType(), Qgis::WkbType::TINZM );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::TriangleZM );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2, 3, 4 ) );

  surface.dropMValue();
  QCOMPARE( surface.wkbType(), Qgis::WkbType::TINZ );
  QCOMPARE( surface.patchN( 0 )->wkbType(), Qgis::WkbType::TriangleZ );
  QCOMPARE( static_cast<const QgsLineString *>( surface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( Qgis::WkbType::PointZ, 1, 2, 3 ) );
}

void TestQgsTriangulatedSurface::testWKB()
{
  QgsTriangulatedSurface surface1;
  QgsTriangulatedSurface surface2;
  QgsTriangle triangle;

  triangle = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 1, 0 ), QgsPoint( 1, 0.5 ) );
  surface1.addPatch( triangle.clone() );

  triangle = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0.1, 0 ), QgsPoint( 0.1, 0.05 ) );
  surface1.addPatch( triangle.clone() );

  QCOMPARE( surface1.wkbType(), Qgis::WkbType::TIN );
  QByteArray wkb16 = surface1.asWkb();
  QCOMPARE( wkb16.size(), surface1.wkbSize() );

  QgsConstWkbPtr wkb16ptr( wkb16 );
  surface2.fromWkb( wkb16ptr );
  QCOMPARE( surface1, surface2 );

  surface1.clear();
  surface2.clear();

  // TINZ
  triangle = QgsTriangle( QgsPoint( 0, 0, 1 ), QgsPoint( 1, 0, 2 ), QgsPoint( 1, 0.5, 4 ) );
  surface1.addPatch( triangle.clone() );

  triangle = QgsTriangle( QgsPoint( 0, 0, 1 ), QgsPoint( 0.1, 0, 2 ), QgsPoint( 0.1, 0.05, 4 ) );
  surface1.addPatch( triangle.clone() );

  QCOMPARE( surface1.wkbType(), Qgis::WkbType::TINZ );
  wkb16 = surface1.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  surface2.fromWkb( wkb16ptr2 );
  QCOMPARE( surface1, surface2 );

  surface1.clear();
  surface2.clear();

  // TINM
  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointM, 0, 0, 0, 1 ), QgsPoint( Qgis::WkbType::PointM, 1, 0, 0, 2 ), QgsPoint( Qgis::WkbType::PointM, 1, 0.5, 0, 4 ) );
  surface1.addPatch( triangle.clone() );

  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointM, 0, 0, 0, 1 ), QgsPoint( Qgis::WkbType::PointM, 0.1, 0, 0, 2 ), QgsPoint( Qgis::WkbType::PointM, 0.1, 0.05, 0, 4 ) );
  surface1.addPatch( triangle.clone() );

  QCOMPARE( surface1.wkbType(), Qgis::WkbType::TINM );
  wkb16 = surface1.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  surface2.fromWkb( wkb16ptr4 );
  QCOMPARE( surface1, surface2 );

  surface1.clear();
  surface2.clear();

  // TINZM
  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ), QgsPoint( Qgis::WkbType::PointZM, 1, 0, 11, 2 ), QgsPoint( Qgis::WkbType::PointZM, 1, 0.5, 13, 4 ) );
  surface1.addPatch( triangle.clone() );

  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ), QgsPoint( Qgis::WkbType::PointZM, 0.1, 0, 11, 2 ), QgsPoint( Qgis::WkbType::PointZM, 0.1, 0.05, 13, 4 ) );
  surface1.addPatch( triangle.clone() );

  QCOMPARE( surface1.wkbType(), Qgis::WkbType::TINZM );
  wkb16 = surface1.asWkb();
  QgsConstWkbPtr wkb16ptr5( wkb16 );
  surface2.fromWkb( wkb16ptr5 );
  QCOMPARE( surface1, surface2 );

  surface1.clear();
  surface2.clear();

  // bad WKB - check for no crash
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !surface2.fromWkb( nullPtr ) );
  QCOMPARE( surface2.wkbType(), Qgis::WkbType::TIN );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );

  QVERIFY( !surface2.fromWkb( wkbPointPtr ) );
  QCOMPARE( surface2.wkbType(), Qgis::WkbType::TIN );
  surface1.clear();
  surface2.clear();

  // GeoJSON export
  triangle = QgsTriangle( QgsPoint( 0, 0, 1 ), QgsPoint( 1, 0, 2 ), QgsPoint( 1, 0.5, 4 ) );
  surface1.addPatch( triangle.clone() );

  triangle = QgsTriangle( QgsPoint( 0, 0, 1 ), QgsPoint( 0.1, 0, 2 ), QgsPoint( 0.1, 0.05, 4 ) );
  surface1.addPatch( triangle.clone() );

  QString expectedSimpleJson( "{\"coordinates\":[[[[0.0,0.0,1.0],[1.0,0.0,2.0],[1.0,0.5,4.0],[0.0,0.0,1.0]]],[[[0.0,0.0,1.0],[0.1,0.0,2.0],[0.1,0.05,4.0],[0.0,0.0,1.0]]]],\"type\":\"MultiPolygon\"}" );
  QString jsonRes = surface1.asJson( 2 );
  QCOMPARE( jsonRes, expectedSimpleJson );
}

void TestQgsTriangulatedSurface::testWKT()
{
  QgsTriangulatedSurface surface;
  QgsTriangle triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ), QgsPoint( Qgis::WkbType::PointZM, 0.1, 0, 11, 2 ), QgsPoint( Qgis::WkbType::PointZM, 0.2, 0, 12, 3 ) );

  surface.addPatch( triangle.clone() );
  QCOMPARE( surface.numPatches(), 1 );

  QString wkt = surface.asWkt();

  QgsTriangulatedSurface surface2;
  QVERIFY( surface2.fromWkt( wkt ) );
  QCOMPARE( surface, surface2 );

  // bad WKT
  QVERIFY( !surface2.fromWkt( "Point()" ) );
  QVERIFY( surface2.isEmpty() );
  QVERIFY( !surface2.patchN( 0 ) );
  QCOMPARE( surface2.numPatches(), 0 );
  QVERIFY( !surface2.is3D() );
  QVERIFY( !surface2.isMeasure() );
  QCOMPARE( surface2.wkbType(), Qgis::WkbType::TIN );
}

void TestQgsTriangulatedSurface::testExport()
{
  QgsTriangulatedSurface exportPolygon;
  QgsTriangle triangle;
  QString expectedSimpleGML3;
  QString result;

  // GML document for compare
  QDomDocument doc( QStringLiteral( "gml" ) );

  // Z
  // as GML3 with one triangle
  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointZ, 0, 0, 10 ), QgsPoint( Qgis::WkbType::PointZ, 1, 0, 11 ), QgsPoint( Qgis::WkbType::PointZ, 2, 0, 12 ) );
  exportPolygon.addPatch( triangle.clone() );

  expectedSimpleGML3 = QString( QStringLiteral( "<TriangulatedSurface xmlns=\"gml\"><patches xmlns=\"gml\"><Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 0 0 10</posList></LinearRing></exterior></Triangle></patches></TriangulatedSurface>" ) );
  result = elemToString( exportPolygon.asGml3( doc, 2 ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );

  // as GML3 with two triangles
  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointZ, 10, 10, 10 ), QgsPoint( Qgis::WkbType::PointZ, 11, 10, 11 ), QgsPoint( Qgis::WkbType::PointZ, 12, 10, 12 ) );
  exportPolygon.addPatch( triangle.clone() );

  expectedSimpleGML3 = QString( QStringLiteral( "<TriangulatedSurface xmlns=\"gml\"><patches xmlns=\"gml\"><Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 0 0 10</posList></LinearRing></exterior></Triangle><Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">10 10 10 11 10 11 12 10 12 10 10 10</posList></LinearRing></exterior></Triangle></patches></TriangulatedSurface>" ) );
  result = elemToString( exportPolygon.asGml3( doc, 2 ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );

  // ZM
  // as GML3 with one triangle - M is dropped
  exportPolygon.clear();
  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ), QgsPoint( Qgis::WkbType::PointZM, 1, 0, 11, 2 ), QgsPoint( Qgis::WkbType::PointZM, 2, 0, 12, 3 ) );
  exportPolygon.addPatch( triangle.clone() );

  expectedSimpleGML3 = QString( QStringLiteral( "<TriangulatedSurface xmlns=\"gml\"><patches xmlns=\"gml\"><Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 0 0 10</posList></LinearRing></exterior></Triangle></patches></TriangulatedSurface>" ) );
  result = elemToString( exportPolygon.asGml3( doc, 2 ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );

  // as GML3 with two triangles - M is dropped
  triangle = QgsTriangle( QgsPoint( Qgis::WkbType::PointZM, 10, 10, 10, 1 ), QgsPoint( Qgis::WkbType::PointZM, 11, 10, 11, 2 ), QgsPoint( Qgis::WkbType::PointZM, 12, 10, 12, 3 ) );
  exportPolygon.addPatch( triangle.clone() );

  expectedSimpleGML3 = QString( QStringLiteral( "<TriangulatedSurface xmlns=\"gml\"><patches xmlns=\"gml\"><Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 0 0 10</posList></LinearRing></exterior></Triangle><Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">10 10 10 11 10 11 12 10 12 10 10 10</posList></LinearRing></exterior></Triangle></patches></TriangulatedSurface>" ) );
  result = elemToString( exportPolygon.asGml3( doc, 2 ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );

  // empty
  QString expectedGML3empty( QStringLiteral( "<TriangulatedSurface xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsTriangulatedSurface().asGml3( doc ) ), expectedGML3empty );
}

void TestQgsTriangulatedSurface::testCast()
{
  QVERIFY( !QgsTriangulatedSurface().cast( nullptr ) );

  QgsTriangulatedSurface pCast;
  QVERIFY( QgsTriangulatedSurface().cast( &pCast ) );

  QgsTriangulatedSurface pCast2;
  pCast2.fromWkt( QStringLiteral( "TINZ((0 0 0, 0 1 1, 1 0 2, 0 0 0))" ) );
  QVERIFY( QgsTriangulatedSurface().cast( &pCast2 ) );

  pCast2.fromWkt( QStringLiteral( "TINM((0 0 1, 0 1 2, 1 0 3, 0 0 1))" ) );
  QVERIFY( QgsTriangulatedSurface().cast( &pCast2 ) );

  pCast2.fromWkt( QStringLiteral( "TINZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1))" ) );
  QVERIFY( QgsTriangulatedSurface().cast( &pCast2 ) );

  QVERIFY( !pCast2.fromWkt( QStringLiteral( "TINZ((0 0 0, 0 1 1, 1 0 2, 2 0 2, 0 0 0))" ) ) );
  QVERIFY( !pCast2.fromWkt( QStringLiteral( "TINZ((111111))" ) ) );
}


QGSTEST_MAIN( TestQgsTriangulatedSurface )
#include "testqgstriangulatedsurface.moc"
