/***************************************************************************
     testqgspolyhedralsurface.cpp
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
#include "qgstest.h"
#include <QObject>
#include <QPainter>
#include <QString>

#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgssurface.h"
#include "qgspolyhedralsurface.h"
#include "qgsvertexid.h"
#include "testgeometryutils.h"

class TestQgsPolyhedralSurface : public QObject
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
    void test3DPatches();
    void testAreaPerimeter();
    void testInsertVertex();
    void testMoveVertex();
    void testDeleteVertex();
    void testNextVertex();
    void testVertexAngle();
    void testDeleteVertexRemovePatch();
    void testVertexNumberFromVertexId();
    void testHasCurvedSegments();
    void testClosestSegment();
    void testBoundary();
    void testBoundingBox();
    void testBoundingBox3D();
    void testBoundingBoxIntersects();
    void testDropZValue();
    void testDropMValue();
    void testCoordinateSequence();
    void testChildGeometry();
    void testWKB();
    void testWKT();
    void testExport();
    void testCast();
    void testIsValid();
};

void TestQgsPolyhedralSurface::testConstructor()
{
  QgsPolyhedralSurface polySurfaceEmpty;

  QVERIFY( polySurfaceEmpty.isEmpty() );
  QCOMPARE( polySurfaceEmpty.nCoordinates(), 0 );
  QCOMPARE( polySurfaceEmpty.ringCount(), 0 );
  QCOMPARE( polySurfaceEmpty.numPatches(), 0 );
  QCOMPARE( polySurfaceEmpty.partCount(), 0 );
  QVERIFY( !polySurfaceEmpty.is3D() );
  QVERIFY( !polySurfaceEmpty.isMeasure() );
  QCOMPARE( polySurfaceEmpty.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QCOMPARE( polySurfaceEmpty.wktTypeStr(), QString( "PolyhedralSurface" ) );
  QCOMPARE( polySurfaceEmpty.geometryType(), QString( "PolyhedralSurface" ) );
  QCOMPARE( polySurfaceEmpty.dimension(), 2 );
  QVERIFY( !polySurfaceEmpty.hasCurvedSegments() );
  QCOMPARE( polySurfaceEmpty.area(), 0.0 );
  QCOMPARE( polySurfaceEmpty.perimeter(), 0.0 );
  QVERIFY( !polySurfaceEmpty.patchN( 0 ) );


  std::unique_ptr<QgsMultiPolygon> multiPolygon = std::make_unique<QgsMultiPolygon>();
  QgsPolygon part;
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 5, 50, 1, 4 ) << QgsPoint( Qgis::WkbType::PointZM, 6, 61, 3, 5 ) << QgsPoint( Qgis::WkbType::PointZM, 9, 71, 4, 15 ) << QgsPoint( Qgis::WkbType::PointZM, 5, 71, 4, 6 ) );
  part.setExteriorRing( ring.clone() );
  multiPolygon->addGeometry( part.clone() );
  QgsPolyhedralSurface polySurface( multiPolygon.get() );
  QVERIFY( multiPolygon );
  QVERIFY( !polySurface.isEmpty() );
  QCOMPARE( polySurface.nCoordinates(), 5 );
  QCOMPARE( polySurface.ringCount(), 1 );
  QCOMPARE( polySurface.numPatches(), 1 );
  QCOMPARE( polySurface.partCount(), 1 );
  QVERIFY( polySurface.is3D() );
  QVERIFY( polySurface.isMeasure() );
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurfaceZM );
  QCOMPARE( polySurface.wktTypeStr(), QString( "PolyhedralSurface ZM" ) );
  QCOMPARE( polySurface.geometryType(), QString( "PolyhedralSurface" ) );
  QCOMPARE( polySurface.dimension(), 2 );
  QVERIFY( !polySurface.hasCurvedSegments() );
  QGSCOMPARENEAR( polySurface.area(), 30.5, 0.01 );
  QGSCOMPARENEAR( polySurface.perimeter(), 46.49, 0.01 );
  QVERIFY( polySurface.patchN( 0 ) );
  QCOMPARE( polySurface.numPatches(), multiPolygon->numGeometries() );
}

void TestQgsPolyhedralSurface::testCopyConstructor()
{
  QgsPolyhedralSurface polySurface1;
  QCOMPARE( polySurface1.numPatches(), 0 );

  QgsPolyhedralSurface polySurface2( polySurface1 );
  QCOMPARE( polySurface1, polySurface2 );

  // add a patch to polySurface1
  QgsPolygon patch;
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  patch.setExteriorRing( patchExterior );
  QgsLineString *patchInteriorRing1 = new QgsLineString();
  patchInteriorRing1->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 ) << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  patch.addInteriorRing( patchInteriorRing1 );
  polySurface1.addPatch( patch.clone() );
  QCOMPARE( polySurface1.numPatches(), 1 );

  QgsPolyhedralSurface polySurface3( polySurface1 );
  QCOMPARE( polySurface1, polySurface3 );

  QgsPolyhedralSurface polySurface4;
  polySurface4 = polySurface2;
  QCOMPARE( polySurface2, polySurface4 );
  polySurface4 = polySurface1;
  QCOMPARE( polySurface1, polySurface4 );
}

void TestQgsPolyhedralSurface::testClear()
{
  QgsPolyhedralSurface polySurface;

  QgsPolygon *patch = new QgsPolygon;
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0, 10, 1 ) << QgsPoint( 10, 10, 1 ) << QgsPoint( 10, 0, 1 ) << QgsPoint( 0, 0, 1 ) );
  patch->setExteriorRing( patchExterior );
  QgsLineString *patchInteriorRing1 = new QgsLineString();
  patchInteriorRing1->setPoints( QgsPointSequence() << QgsPoint( 1, 1, 1 ) << QgsPoint( 1, 9, 1 ) << QgsPoint( 9, 9, 1 ) << QgsPoint( 9, 1, 1 ) << QgsPoint( 1, 1, 1 ) );
  patch->addInteriorRing( patchInteriorRing1 );
  polySurface.addPatch( patch );

  QVERIFY( !polySurface.isEmpty() );
  QCOMPARE( polySurface.numPatches(), 1 );
  QCOMPARE( polySurface.nCoordinates(), 10 );
  QCOMPARE( polySurface.numPatches(), 1 );
  QCOMPARE( polySurface.partCount(), 1 );
  QVERIFY( polySurface.is3D() );
  QVERIFY( !polySurface.isMeasure() );
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurfaceZ );

  polySurface.clear();
  QVERIFY( polySurface.isEmpty() );
  QCOMPARE( polySurface.numPatches(), 0 );
  QCOMPARE( polySurface.nCoordinates(), 0 );
  QCOMPARE( polySurface.numPatches(), 0 );
  QCOMPARE( polySurface.partCount(), 0 );
  QVERIFY( !polySurface.is3D() );
  QVERIFY( !polySurface.isMeasure() );
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );
}

void TestQgsPolyhedralSurface::testClone()
{
  QgsPolyhedralSurface polySurface;

  std::unique_ptr<QgsPolyhedralSurface> cloned( polySurface.clone() );
  QCOMPARE( polySurface, *cloned );

  QgsPolygon *patch = new QgsPolygon();
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 4, 4 ) << QgsPoint( 6, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 4 ) << QgsPoint( 4, 4 ) );
  patch->setExteriorRing( patchExterior );
  QgsLineString *patchInteriorRing1 = new QgsLineString();
  patchInteriorRing1->setPoints( QgsPointSequence() << QgsPoint( 7, 5 ) << QgsPoint( 8, 9 ) << QgsPoint( 9, 9 ) << QgsPoint( 9, 6 ) << QgsPoint( 7, 5 ) );
  patch->addInteriorRing( patchInteriorRing1 );
  polySurface.addPatch( patch );

  cloned.reset( polySurface.clone() );
  QCOMPARE( polySurface, *cloned );
}

void TestQgsPolyhedralSurface::testEquality()
{
  QgsPolyhedralSurface polySurface1, polySurface2;
  QgsPolygon *patch;

  QVERIFY( polySurface1 == polySurface2 );
  QVERIFY( !( polySurface1 != polySurface2 ) );

  patch = new QgsPolygon();
  QgsLineString patchExterior1( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  QgsLineString patchInteriorRing1( QVector<QgsPoint>() << QgsPoint( 1, 1 ) << QgsPoint( 2, 1 ) << QgsPoint( 2, 2 ) << QgsPoint( 1, 2 ) << QgsPoint( 1, 1 ) );
  patch->setExteriorRing( patchExterior1.clone() );
  patch->addInteriorRing( patchInteriorRing1.clone() );
  polySurface1.addPatch( patch );
  QVERIFY( !( polySurface1 == polySurface2 ) );
  QVERIFY( polySurface1 != polySurface2 );

  patch = new QgsPolygon();
  patch->setExteriorRing( patchExterior1.clone() );
  patch->addInteriorRing( patchInteriorRing1.clone() );
  polySurface2.addPatch( patch );
  QVERIFY( polySurface1 == polySurface2 );
  QVERIFY( !( polySurface1 != polySurface2 ) );

  patch = new QgsPolygon();
  QgsLineString patchExterior2( QVector<QgsPoint>() << QgsPoint( 10, 0 ) << QgsPoint( 10, 10 ) << QgsPoint( 15, 10 ) << QgsPoint( 15, 0 ) << QgsPoint( 10, 0 ) );
  QgsLineString patchInteriorRing2( QVector<QgsPoint>() << QgsPoint( 13, 1 ) << QgsPoint( 14, 1 ) << QgsPoint( 14, 2 ) << QgsPoint( 13, 2 ) << QgsPoint( 13, 1 ) );
  patch->setExteriorRing( patchExterior2.clone() );
  patch->addInteriorRing( patchInteriorRing2.clone() );
  polySurface2.addPatch( patch );
  QVERIFY( !( polySurface1 == polySurface2 ) );
  QVERIFY( polySurface1 != polySurface2 );

  polySurface2.removePatch( 1 );
  QVERIFY( polySurface1 == polySurface2 );
  QVERIFY( !( polySurface1 != polySurface2 ) );
}

void TestQgsPolyhedralSurface::testAddPatch()
{
  QgsPolyhedralSurface polySurface;

  // empty surface
  QCOMPARE( polySurface.numPatches(), 0 );
  QVERIFY( !polySurface.patchN( -1 ) );
  QVERIFY( !polySurface.patchN( 0 ) );

  polySurface.addPatch( nullptr );
  QCOMPARE( polySurface.numPatches(), 0 );

  QgsPolygon *patch = new QgsPolygon();
  QgsLineString patchExterior;
  patchExterior.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  patch->setExteriorRing( patchExterior.clone() );
  QgsLineString patchInteriorRing;
  patchInteriorRing.setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 ) << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  patch->addInteriorRing( patchInteriorRing.clone() );
  polySurface.addPatch( patch );

  QCOMPARE( polySurface.numPatches(), 1 );
  QCOMPARE( polySurface.patchN( 0 ), patch );
  QVERIFY( !polySurface.patchN( 1 ) );

  QCOMPARE( polySurface.nCoordinates(), 10 );

  // try adding a patch with z to a 2d polyhedral surface, z should be dropped
  patch = new QgsPolygon();
  QgsLineString patchExteriorZ;
  patchExteriorZ.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1 ) << QgsPoint( 0, 10, 1 ) << QgsPoint( 10, 10, 1 ) << QgsPoint( 10, 0, 1 ) << QgsPoint( 0, 0, 1 ) );
  patch->setExteriorRing( patchExteriorZ.clone() );
  QgsLineString patchInteriorRingZ;
  patchInteriorRingZ.setPoints( QgsPointSequence() << QgsPoint( 1, 1, 1 ) << QgsPoint( 1, 9, 1 ) << QgsPoint( 9, 9, 1 ) << QgsPoint( 9, 1, 1 ) << QgsPoint( 1, 1, 1 ) );
  patch->addInteriorRing( patchInteriorRingZ.clone() );
  polySurface.addPatch( patch );

  QCOMPARE( polySurface.numPatches(), 2 );
  QVERIFY( !polySurface.is3D() );
  QVERIFY( !polySurface.isMeasure() );
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QVERIFY( polySurface.patchN( 1 ) );
  QVERIFY( !polySurface.patchN( 1 )->is3D() );
  QVERIFY( !polySurface.patchN( 1 )->isMeasure() );
  QCOMPARE( polySurface.patchN( 1 )->wkbType(), Qgis::WkbType::Polygon );

  // try adding a patch with zm to a 2d polygon, z and m should be dropped
  patch = new QgsPolygon;
  QgsLineString patchExteriorZM;
  patchExteriorZM.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) << QgsPoint( 0, 10, 1, 2 ) << QgsPoint( 10, 10, 1, 2 ) << QgsPoint( 10, 0, 1, 2 ) << QgsPoint( 0, 0, 1, 2 ) );
  patch->setExteriorRing( patchExteriorZM.clone() );
  QgsLineString patchInteriorRingZM;
  patchInteriorRingZM.setPoints( QgsPointSequence() << QgsPoint( 1, 1, 1, 2 ) << QgsPoint( 1, 9, 1, 2 ) << QgsPoint( 9, 9, 1, 2 ) << QgsPoint( 9, 1, 1, 2 ) << QgsPoint( 1, 1, 1, 2 ) );
  patch->addInteriorRing( patchInteriorRingZM.clone() );
  polySurface.addPatch( patch );

  QCOMPARE( polySurface.numPatches(), 3 );
  QVERIFY( !polySurface.is3D() );
  QVERIFY( !polySurface.isMeasure() );
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QVERIFY( polySurface.patchN( 2 ) );
  QVERIFY( !polySurface.patchN( 2 )->is3D() );
  QVERIFY( !polySurface.patchN( 2 )->isMeasure() );
  QCOMPARE( polySurface.patchN( 2 )->wkbType(), Qgis::WkbType::Polygon );


  // addPatch without z/m to PolygonZM
  QgsPolyhedralSurface polySurface2;
  patch = new QgsPolygon();
  patch->setExteriorRing( patchExteriorZM.clone() );
  patch->addInteriorRing( patchInteriorRingZM.clone() );
  polySurface2.addPatch( patch );

  QCOMPARE( polySurface2.numPatches(), 1 );
  QVERIFY( polySurface2.is3D() );
  QVERIFY( polySurface2.isMeasure() );
  QCOMPARE( polySurface2.wkbType(), Qgis::WkbType::PolyhedralSurfaceZM );
  QVERIFY( polySurface2.patchN( 0 ) );
  QVERIFY( polySurface2.patchN( 0 )->is3D() );
  QVERIFY( polySurface2.patchN( 0 )->isMeasure() );
  QCOMPARE( polySurface2.patchN( 0 )->wkbType(), Qgis::WkbType::PolygonZM );
  QCOMPARE( polySurface2.patchN( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 10, 0, 1, 2 ) );

  // patch has no z
  patch = new QgsPolygon();
  patch->setExteriorRing( patchExterior.clone() );
  patch->addInteriorRing( patchInteriorRing.clone() );
  polySurface2.addPatch( patch );

  QCOMPARE( polySurface2.numPatches(), 2 );
  QVERIFY( polySurface2.patchN( 1 ) );
  QVERIFY( polySurface2.patchN( 1 )->is3D() );
  QVERIFY( polySurface2.patchN( 1 )->isMeasure() );
  QCOMPARE( polySurface2.patchN( 1 )->wkbType(), Qgis::WkbType::PolygonZM );
  QCOMPARE( polySurface2.patchN( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 0, 0, 0, 0 ) );

  // patch has no m
  patch = new QgsPolygon();
  patch->setExteriorRing( patchExteriorZ.clone() );
  patch->addInteriorRing( patchInteriorRingZ.clone() );
  polySurface2.addPatch( patch );

  QCOMPARE( polySurface2.numPatches(), 3 );
  QVERIFY( polySurface2.patchN( 2 ) );
  QVERIFY( polySurface2.patchN( 2 )->is3D() );
  QVERIFY( polySurface2.patchN( 2 )->isMeasure() );
  QCOMPARE( polySurface2.patchN( 2 )->wkbType(), Qgis::WkbType::PolygonZM );
  QCOMPARE( polySurface2.patchN( 2 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 10, 0, 1, 0 ) );
}

void TestQgsPolyhedralSurface::testRemovePatch()
{
  QgsPolyhedralSurface polySurface;
  QVector<QgsPolygon *> patches;

  QVERIFY( !polySurface.removePatch( -1 ) );
  QVERIFY( !polySurface.removePatch( 0 ) );

  QgsPolygon *patch1 = new QgsPolygon();
  QgsLineString *patchExterior1 = new QgsLineString();
  patchExterior1->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.1, 0.2 ) << QgsPoint( 0.2, 0.2 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.1, 0.1 ) );
  patch1->setExteriorRing( patchExterior1 );
  QgsLineString *patchInteriorRing1 = new QgsLineString();
  patchInteriorRing1->setPoints( QgsPointSequence() << QgsPoint( 0.12, 0.12 ) << QgsPoint( 0.13, 0.13 ) << QgsPoint( 0.14, 0.14 ) << QgsPoint( 0.14, 0.13 ) << QgsPoint( 0.12, 0.12 ) );
  patch1->addInteriorRing( patchInteriorRing1 );
  patches.append( patch1 );

  QgsPolygon *patch2 = new QgsPolygon();
  QgsLineString *patchExterior2 = new QgsLineString();
  patchExterior2->setPoints( QgsPointSequence() << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 ) << QgsPoint( 0.3, 0.2 ) << QgsPoint( 0.3, 0.1 ) << QgsPoint( 0.2, 0.1 ) );
  patch2->setExteriorRing( patchExterior2 );
  patches.append( patch2 );

  QgsPolygon *patch3 = new QgsPolygon();
  QgsLineString *patchExterior3 = new QgsLineString();
  patchExterior3->setPoints( QgsPointSequence() << QgsPoint( 0.3, 0.1 ) << QgsPoint( 0.3, 0.2 ) << QgsPoint( 0.4, 0.2 ) << QgsPoint( 0.4, 0.1 ) << QgsPoint( 0.3, 0.1 ) );
  patch3->setExteriorRing( patchExterior3 );
  patches.append( patch3 );

  polySurface.setPatches( patches );

  QCOMPARE( polySurface.numPatches(), 3 );
  QCOMPARE( polySurface.patchN( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.1, 0.1 ) );

  QVERIFY( polySurface.removePatch( 0 ) );
  QCOMPARE( polySurface.numPatches(), 2 );
  QCOMPARE( polySurface.patchN( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.2, 0.1 ) );
  QCOMPARE( polySurface.patchN( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.3, 0.1 ) );

  QVERIFY( polySurface.removePatch( 1 ) );
  QCOMPARE( polySurface.numPatches(), 1 );
  QCOMPARE( polySurface.patchN( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.2, 0.1 ) );

  QVERIFY( polySurface.removePatch( 0 ) );
  QCOMPARE( polySurface.numPatches(), 0 );
  QVERIFY( !polySurface.removePatch( 0 ) );
}

void TestQgsPolyhedralSurface::test3DPatches()
{
  // change dimensionality of patches using setExteriorRing
  QgsPolyhedralSurface polySurface;
  QVector<QgsPolygon *> patches;

  QgsPolygon *patch1 = new QgsPolygon();
  QgsLineString *patchExterior1 = new QgsLineString();
  patchExterior1->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1, 2 ) << QgsPoint( 0.1, 0.2, 2 ) << QgsPoint( 0.2, 0.2, 2 ) << QgsPoint( 0.2, 0.1, 2 ) << QgsPoint( 0.1, 0.1, 2 ) );
  patch1->setExteriorRing( patchExterior1 );
  QgsLineString *patchInteriorRing1 = new QgsLineString();
  patchInteriorRing1->setPoints( QgsPointSequence() << QgsPoint( 0.12, 0.12, 2 ) << QgsPoint( 0.13, 0.13, 2 ) << QgsPoint( 0.14, 0.14, 2 ) << QgsPoint( 0.14, 0.13, 2 ) << QgsPoint( 0.12, 0.12, 2 ) );
  patch1->addInteriorRing( patchInteriorRing1 );
  patches.append( patch1 );

  QgsPolygon *patch2 = new QgsPolygon();
  QgsLineString *patchExterior2 = new QgsLineString();
  patchExterior2->setPoints( QgsPointSequence() << QgsPoint( 0.2, 0.1, 2 ) << QgsPoint( 0.2, 0.2, 2 ) << QgsPoint( 0.3, 0.2, 2 ) << QgsPoint( 0.3, 0.1, 2 ) << QgsPoint( 0.2, 0.1, 2 ) );
  patch2->setExteriorRing( patchExterior2 );
  patches.append( patch2 );

  polySurface.setPatches( patches );

  QVERIFY( polySurface.is3D() );
  QVERIFY( !polySurface.isMeasure() );
  QVERIFY( polySurface.patchN( 0 )->is3D() );
  QVERIFY( !polySurface.patchN( 0 )->isMeasure() );
  QVERIFY( polySurface.patchN( 1 )->is3D() );
  QVERIFY( !polySurface.patchN( 1 )->isMeasure() );
}

void TestQgsPolyhedralSurface::testAreaPerimeter()
{
  QgsPolyhedralSurface polySurface;

  QgsPolygon *patch = new QgsPolygon();
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 6 ) << QgsPoint( 6, 6 ) << QgsPoint( 6, 1 ) << QgsPoint( 1, 1 ) );
  patch->setExteriorRing( patchExterior );
  polySurface.addPatch( patch );

  QGSCOMPARENEAR( polySurface.area(), 25.0, 0.01 ); // area is not implemented
  QGSCOMPARENEAR( polySurface.perimeter(), 20.0, 0.01 );
}

void TestQgsPolyhedralSurface::testInsertVertex()
{
  QgsPolyhedralSurface polySurface;
  QgsPolygon patch;

  // insert vertex in empty polygon
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( polySurface.isEmpty() );

  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );

  QVERIFY( polySurface.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( polySurface.nCoordinates(), 8 );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 0, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );

  // first vertex
  QVERIFY( polySurface.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( polySurface.nCoordinates(), 9 );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 8 ), QgsPoint( 0, 0.1 ) );

  // last vertex
  QVERIFY( polySurface.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( polySurface.nCoordinates(), 10 );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );

  // add a second patch with an interior ring
  patch.clear();
  patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 12, 10 ) << QgsPoint( 12, 12 ) << QgsPoint( 10, 12 ) << QgsPoint( 10, 10 ) );
  patch.setExteriorRing( patchExterior );
  QgsLineString *patchInterior = new QgsLineString();
  patchInterior->setPoints( QgsPointSequence() << QgsPoint( 10.2, 10.2 ) << QgsPoint( 10.9, 10.2 ) << QgsPoint( 10.9, 11.2 ) << QgsPoint( 10.2, 11.2 ) << QgsPoint( 10.2, 10.2 ) );
  patch.addInteriorRing( patchInterior );
  polySurface.addPatch( patch.clone() );


  QCOMPARE( polySurface.nCoordinates(), 20 );
  QVERIFY( polySurface.insertVertex( QgsVertexId( 1, 0, 1 ), QgsPoint( 10.5, 10 ) ) );
  QCOMPARE( polySurface.nCoordinates(), 21 );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 0 ), QgsPoint( 10, 10 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 1 ), QgsPoint( 10.5, 10 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 2 ), QgsPoint( 12, 10 ) );
  QVERIFY( polySurface.insertVertex( QgsVertexId( 1, 1, 1 ), QgsPoint( 10.8, 10.2 ) ) );
  QCOMPARE( polySurface.nCoordinates(), 22 );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 10.2, 10.2 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 10.8, 10.2 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 10.9, 10.2 ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 1, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 1, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 1, 2, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !polySurface.insertVertex( QgsVertexId( 2, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( polySurface.nCoordinates(), 22 );

  // first vertex second patch
  QVERIFY( polySurface.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 9, 10 ) ) );
  QCOMPARE( polySurface.nCoordinates(), 23 );
  QCOMPARE( polySurface.patchN( 1 )->exteriorRing()->numPoints(), 7 );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 0 ), QgsPoint( 9, 10 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 1 ), QgsPoint( 10, 10 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 2 ), QgsPoint( 10.5, 10 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 3 ), QgsPoint( 12, 10 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 5 ), QgsPoint( 10, 12 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 6 ), QgsPoint( 9, 10 ) );

  // last vertex second patch
  QCOMPARE( polySurface.patchN( 1 )->interiorRing( 0 )->numPoints(), 6 );
  QVERIFY( polySurface.insertVertex( QgsVertexId( 1, 1, 6 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( polySurface.nCoordinates(), 24 );
  QCOMPARE( polySurface.patchN( 1 )->interiorRing( 0 )->numPoints(), 7 );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 10.8, 10.2 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 10.9, 10.2 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 10.9, 11.2 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 5 ), QgsPoint( 10.2, 10.2 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 6 ), QgsPoint( 0.1, 0.1 ) );
}

void TestQgsPolyhedralSurface::testMoveVertex()
{
  // empty polyhedral surface
  QgsPolyhedralSurface polySurface;
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( polySurface.isEmpty() );

  // valid polygon
  QgsPolygon patch;
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );

  QVERIFY( polySurface.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( polySurface.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( polySurface.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  // move last vertex
  QVERIFY( polySurface.moveVertex( QgsVertexId( 0, 0, 3 ), QgsPoint( 1.0, 2.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 1.0, 2.0 ) );

  // out of range
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 1.0, 2.0 ) );

  // add a second patch with an interior ring
  patch.clear();
  patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 12, 10 ) << QgsPoint( 12, 12 ) << QgsPoint( 10, 12 ) << QgsPoint( 10, 10 ) );
  patch.setExteriorRing( patchExterior );
  QgsLineString *patchInterior = new QgsLineString();
  patchInterior->setPoints( QgsPointSequence() << QgsPoint( 10.2, 10.2 ) << QgsPoint( 10.9, 10.2 ) << QgsPoint( 10.9, 11.2 ) << QgsPoint( 10.2, 11.2 ) << QgsPoint( 10.2, 10.2 ) );
  patch.addInteriorRing( patchInterior );
  polySurface.addPatch( patch.clone() );

  QVERIFY( polySurface.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 4.0, 5.0 ) ) );
  QVERIFY( polySurface.moveVertex( QgsVertexId( 1, 0, 1 ), QgsPoint( 14.0, 15.0 ) ) );
  QVERIFY( polySurface.moveVertex( QgsVertexId( 1, 0, 2 ), QgsPoint( 24.0, 25.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 0 ), QgsPoint( 4.0, 5.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 1 ), QgsPoint( 14.0, 15.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 2 ), QgsPoint( 24.0, 25.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 4 ), QgsPoint( 4.0, 5.0 ) );

  QVERIFY( polySurface.moveVertex( QgsVertexId( 1, 1, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( polySurface.moveVertex( QgsVertexId( 1, 1, 1 ), QgsPoint( 13.0, 14.0 ) ) );
  QVERIFY( polySurface.moveVertex( QgsVertexId( 1, 1, 2 ), QgsPoint( 23.0, 24.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 3.0, 4.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 13.0, 14.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 23.0, 24.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 4 ), QgsPoint( 3.0, 4.0 ) );

  // move last vertex
  QVERIFY( polySurface.moveVertex( QgsVertexId( 1, 1, 4 ), QgsPoint( -1.0, -2.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 0 ), QgsPoint( -1.0, -2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 13.0, 14.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 23.0, 24.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 4 ), QgsPoint( -1.0, -2.0 ) );

  // out of range
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 1, 1, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 1, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 1, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 1, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !polySurface.moveVertex( QgsVertexId( 2, 0, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 0 ), QgsPoint( 4.0, 5.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 1 ), QgsPoint( 14.0, 15.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 2 ), QgsPoint( 24.0, 25.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->exteriorRing() )->pointN( 3 ), QgsPoint( 10.0, 12.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 0 ), QgsPoint( -1.0, -2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 13.0, 14.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 23.0, 24.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 1 )->interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 10.2, 11.2 ) );
}

void TestQgsPolyhedralSurface::testDeleteVertex()
{
  // empty polygon
  QgsPolyhedralSurface polySurface;
  QVERIFY( !polySurface.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !polySurface.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QVERIFY( polySurface.isEmpty() );

  // valid polygon
  QgsPolygon patch;
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 5, 2 ) << QgsPoint( 6, 2 ) << QgsPoint( 7, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );

  // out of range vertices
  QVERIFY( !polySurface.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !polySurface.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );
  QVERIFY( !polySurface.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );

  // valid vertices
  QVERIFY( polySurface.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( polySurface.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( polySurface.deleteVertex( QgsVertexId( 0, 0, 4 ) ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove patch
  QVERIFY( polySurface.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QVERIFY( !polySurface.patchN( 0 ) );
}

void TestQgsPolyhedralSurface::testNextVertex()
{
  QgsPolyhedralSurface empty;
  QPainter p;
  empty.draw( p ); // no crash!

  QgsPoint pt;
  QgsVertexId vId;
  ( void ) empty.closestSegment( QgsPoint( 1, 2 ), pt, vId ); // empty segment, just want no crash

  // nextVertex
  QgsPolyhedralSurface surfacePoly;
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( -1, 0, 0 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 0, -1, -1 ) );

  vId = QgsVertexId( 0, 0, -2 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 0, 0, 10 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  QgsPolygon patch;
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  patch.setExteriorRing( patchExterior );
  surfacePoly.addPatch( patch.clone() );

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

  // add a second patch
  patch.clear();
  patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) << QgsPoint( 1, 2 ) );
  patch.setExteriorRing( patchExterior );
  QgsLineString *patchInterior = new QgsLineString();
  patchInterior->setPoints( QgsPointSequence() << QgsPoint( 4.5, 3 ) << QgsPoint( 5.5, 3 ) << QgsPoint( 5, 2.5 ) << QgsPoint( 4.5, 3 ) );
  patch.addInteriorRing( patchInterior );
  surfacePoly.addPatch( patch.clone() );

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
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 1, 0 ) );
  QCOMPARE( pt, QgsPoint( 4.5, 3 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 1, 1 ) );
  QCOMPARE( pt, QgsPoint( 5.5, 3 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 1, 2 ) );
  QCOMPARE( pt, QgsPoint( 5, 2.5 ) );
  QVERIFY( surfacePoly.nextVertex( vId, pt ) );
  QCOMPARE( vId, QgsVertexId( 1, 1, 3 ) );
  QCOMPARE( pt, QgsPoint( 4.5, 3 ) );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );

  vId = QgsVertexId( 2, 0, 0 );
  QVERIFY( !surfacePoly.nextVertex( vId, pt ) );
}

void TestQgsPolyhedralSurface::testVertexAngle()
{
  QgsPolyhedralSurface polySurface;

  // just want no crash
  ( void ) polySurface.vertexAngle( QgsVertexId() );
  ( void ) polySurface.vertexAngle( QgsVertexId( 0, 0, 0 ) );
  ( void ) polySurface.vertexAngle( QgsVertexId( 0, 1, 0 ) );

  QgsPolygon patch;
  QgsLineString *exteriorRing = new QgsLineString;
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface.addPatch( patch.clone() );

  QGSCOMPARENEAR( polySurface.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( polySurface.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( polySurface.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( polySurface.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( polySurface.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( polySurface.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( polySurface.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );
}

void TestQgsPolyhedralSurface::testDeleteVertexRemovePatch()
{
  QgsPolyhedralSurface polySurface;

  QgsPolygon *patch = new QgsPolygon();
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  patch->setExteriorRing( patchExterior );
  polySurface.addPatch( patch );

  QVERIFY( polySurface.patchN( 0 ) );
  polySurface.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QVERIFY( !polySurface.patchN( 0 ) );
}

void TestQgsPolyhedralSurface::testVertexNumberFromVertexId()
{
  QgsPolyhedralSurface polySurface;
  QgsPolygon patch;

  // with only one patch
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );

  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), -1 );

  patch.clear();
  patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 2, 0 ) << QgsPoint( 1, 0 ) );
  patch.setExteriorRing( patchExterior );
  QgsLineString *patchInterior = new QgsLineString();
  patchInterior->setPoints( QgsPointSequence() << QgsPoint( 1.2, 1.2 ) << QgsPoint( 1.2, 1.6 ) << QgsPoint( 1.6, 1.6 ) << QgsPoint( 1.2, 1.2 ) );
  patch.addInteriorRing( patchInterior );
  polySurface.addPatch( patch.clone() );

  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), -1 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 4 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), 5 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 2 ) ), 6 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 3 ) ), 7 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 0, 4 ) ), -1 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 1, 0 ) ), 8 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 1, 1 ) ), 9 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 1, 2 ) ), 10 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 1, 3 ) ), 11 );
  QCOMPARE( polySurface.vertexNumberFromVertexId( QgsVertexId( 1, 1, 4 ) ), -1 );
}

void TestQgsPolyhedralSurface::testHasCurvedSegments()
{
  QgsPolyhedralSurface polySurface;
  QVERIFY( !polySurface.hasCurvedSegments() );

  QgsPolygon patch;
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );
  QVERIFY( !polySurface.hasCurvedSegments() );
}

void TestQgsPolyhedralSurface::testClosestSegment()
{
  QgsPolyhedralSurface empty;
  QPainter p;
  empty.draw( p ); // no crash!

  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;
  ( void ) empty.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty segment, just want no crash

  QgsPolyhedralSurface polySurface;
  QgsPolygon patch;
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) << QgsPoint( 5, 10 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ), 2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 2.5, 13 ), pt, v, &leftOf ), 6.25000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.0, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  // point directly on segment
  QCOMPARE( polySurface.closestSegment( QgsPoint( 5, 15 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  // with a second patch
  patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 5, 15 ) << QgsPoint( 2, 11 ) << QgsPoint( 5, 10 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 0.4, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 3.8, 0.01 );
  QGSCOMPARENEAR( pt.y(), 10.4, 0.01 );
  QCOMPARE( v, QgsVertexId( 1, 0, 3 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ), 2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 2.5, 13 ), pt, v, &leftOf ), 0.64000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 3.14, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12.52, 0.01 );
  QCOMPARE( v, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( polySurface.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  // point directly on segment
  QCOMPARE( polySurface.closestSegment( QgsPoint( 2, 11 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 2, 11 ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( leftOf, 0 );
}

void TestQgsPolyhedralSurface::testBoundary()
{
  QgsPolygon patch;
  QgsPolyhedralSurface polySurface;
  QVERIFY( !polySurface.boundary() );

  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 ) << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );

  QgsAbstractGeometry *boundary = polySurface.boundary();
  QgsMultiLineString *multiLineBoundary = dynamic_cast<QgsMultiLineString *>( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 1 );
  QgsLineString *lineBoundary = multiLineBoundary->lineStringN( 0 );
  QCOMPARE( lineBoundary->numPoints(), 5 );
  QCOMPARE( lineBoundary->xAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->xAt( 1 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 2 ), 2.0 );
  QCOMPARE( lineBoundary->xAt( 3 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 4 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 1 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 2 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 3 ), 0.5 );
  QCOMPARE( lineBoundary->yAt( 4 ), 0.0 );
  delete boundary;

  QgsLineString *patchInterior1 = new QgsLineString();
  patchInterior1->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 ) );
  patch.addInteriorRing( patchInterior1 );
  QgsLineString *patchInterior2 = new QgsLineString();
  patchInterior2->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 ) );
  patch.addInteriorRing( patchInterior2 );


  polySurface.removePatch( 0 );
  QCOMPARE( polySurface.numPatches(), 0 );
  polySurface.addPatch( patch.clone() );
  boundary = polySurface.boundary();

  multiLineBoundary = dynamic_cast<QgsMultiLineString *>( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 3 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->numPoints(), 5 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->xAt( 2 ), 2.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->xAt( 3 ), 1.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->xAt( 4 ), 0.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->yAt( 2 ), 0.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.5 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 0 ) )->yAt( 4 ), 0.0 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->xAt( 0 ), 0.1 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->xAt( 1 ), 0.2 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->xAt( 2 ), 0.2 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->xAt( 3 ), 0.1 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->yAt( 0 ), 0.1 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->yAt( 1 ), 0.1 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->yAt( 2 ), 0.2 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 1 ) )->yAt( 3 ), 0.1 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->xAt( 0 ), 0.8 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->xAt( 1 ), 0.9 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->xAt( 2 ), 0.9 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->xAt( 3 ), 0.8 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->yAt( 0 ), 0.8 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->yAt( 1 ), 0.8 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->yAt( 2 ), 0.9 );
  QCOMPARE( qgis::down_cast<QgsLineString *>( multiLineBoundary->geometryN( 2 ) )->yAt( 3 ), 0.8 );

  polySurface.removePatch( 0 );
  QCOMPARE( polySurface.numPatches(), 0 );
  patch.removeInteriorRings();
  delete boundary;

  // test boundary with z
  patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 10 ) << QgsPoint( Qgis::WkbType::PointZ, 1, 0, 15 ) << QgsPoint( Qgis::WkbType::PointZ, 1, 1, 20 ) );
  patch.setExteriorRing( patchExterior );
  polySurface.addPatch( patch.clone() );

  boundary = polySurface.boundary();
  multiLineBoundary = dynamic_cast<QgsMultiLineString *>( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 1 );
  lineBoundary = multiLineBoundary->lineStringN( 0 );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->wkbType(), Qgis::WkbType::LineStringZ );
  QCOMPARE( lineBoundary->pointN( 0 ).z(), 10.0 );
  QCOMPARE( lineBoundary->pointN( 1 ).z(), 15.0 );
  QCOMPARE( lineBoundary->pointN( 2 ).z(), 20.0 );
  QCOMPARE( lineBoundary->pointN( 3 ).z(), 10.0 );

  delete boundary;
}

void TestQgsPolyhedralSurface::testBoundingBox()
{
  QgsPolyhedralSurface polySurface;
  QgsRectangle bBox = polySurface.boundingBox(); // no crash!

  QgsPolygon *patch1 = new QgsPolygon();
  QgsLineString *patchExterior1 = new QgsLineString();
  patchExterior1->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 1, 2, 3 ) << QgsPoint( Qgis::WkbType::PointZ, 4, 5, 6 ) << QgsPoint( Qgis::WkbType::PointZ, 7, 8, 9 ) << QgsPoint( Qgis::WkbType::PointZ, 1, 2, 3 ) );
  patch1->setExteriorRing( patchExterior1 );
  polySurface.addPatch( patch1 );

  QgsPolygon *patch2 = new QgsPolygon();
  QgsLineString *patchExterior2 = new QgsLineString();
  patchExterior2->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 10, 11, 12 ) << QgsPoint( Qgis::WkbType::PointZ, 13, 14, 15 ) << QgsPoint( Qgis::WkbType::PointZ, 16, 17, 18 ) << QgsPoint( Qgis::WkbType::PointZ, 10, 11, 12 ) );
  patch2->setExteriorRing( patchExterior2 );
  QgsLineString *patchInterior2 = new QgsLineString();
  patchInterior2->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 10.5, 11.5, 12.5 ) << QgsPoint( Qgis::WkbType::PointZ, 13.5, 14.5, 15.5 ) << QgsPoint( Qgis::WkbType::PointZ, 15.5, 16.5, 17.5 ) << QgsPoint( Qgis::WkbType::PointZ, 10.5, 11.5, 12.5 ) );
  patch2->addInteriorRing( patchInterior2 );
  polySurface.addPatch( patch2 );

  bBox = polySurface.boundingBox();
  QGSCOMPARENEAR( bBox.xMinimum(), 1.0, 0.001 );
  QGSCOMPARENEAR( bBox.xMaximum(), 16.0, 0.001 );
  QGSCOMPARENEAR( bBox.yMinimum(), 2.0, 0.001 );
  QGSCOMPARENEAR( bBox.yMaximum(), 17.0, 0.001 );
}

void TestQgsPolyhedralSurface::testBoundingBox3D()
{
  QgsPolyhedralSurface polySurface;
  QgsBox3D bBox = polySurface.boundingBox3D();
  QCOMPARE( bBox.xMinimum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.xMaximum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.yMinimum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.yMaximum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.zMinimum(), std::numeric_limits<double>::quiet_NaN() );
  QCOMPARE( bBox.zMaximum(), std::numeric_limits<double>::quiet_NaN() );

  QgsPolygon *patch = new QgsPolygon();
  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 6 ) << QgsPoint( Qgis::WkbType::PointZ, 1, 10, 2 ) << QgsPoint( Qgis::WkbType::PointZ, 0, 18, 3 ) << QgsPoint( Qgis::WkbType::PointZ, -1, 4, 4 ) << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 6 ) );
  patch->setExteriorRing( patchExterior );
  polySurface.addPatch( patch );

  bBox = polySurface.boundingBox3D();
  QGSCOMPARENEAR( bBox.xMinimum(), -1.0, 0.001 );
  QGSCOMPARENEAR( bBox.xMaximum(), 1.0, 0.001 );
  QGSCOMPARENEAR( bBox.yMinimum(), 0.0, 0.001 );
  QGSCOMPARENEAR( bBox.yMaximum(), 18.0, 0.001 );
  QGSCOMPARENEAR( bBox.zMinimum(), 2.0, 0.001 );
  QGSCOMPARENEAR( bBox.zMaximum(), 6.0, 0.001 );
}

void TestQgsPolyhedralSurface::testBoundingBoxIntersects()
{
  // 2d
  QgsPolyhedralSurface polySurface1;
  QVERIFY( !polySurface1.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );

  QgsPolygon *patch1 = new QgsPolygon();
  QgsLineString *patchExterior1 = new QgsLineString();
  patchExterior1->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 1 ) << QgsPoint( Qgis::WkbType::PointZ, 1, 10, 2 ) << QgsPoint( Qgis::WkbType::PointZ, 0, 18, 3 ) << QgsPoint( Qgis::WkbType::PointZ, -1, 4, 4 ) << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 1 ) );
  patch1->setExteriorRing( patchExterior1 );
  polySurface1.addPatch( patch1 );

  QVERIFY( polySurface1.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !polySurface1.boundingBoxIntersects( QgsRectangle( 1.1, -5, 6, -2 ) ) );

  // 3d
  QgsPolyhedralSurface polySurface2;
  QVERIFY( !polySurface2.boundingBoxIntersects( QgsBox3D( 1, 3, 1, 6, 9, 2 ) ) );

  QgsPolygon *patch2 = new QgsPolygon();
  QgsLineString *patchExterior2 = new QgsLineString();
  patchExterior2->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 1 ) << QgsPoint( Qgis::WkbType::PointZ, 1, 10, 2 ) << QgsPoint( Qgis::WkbType::PointZ, 0, 18, 3 ) << QgsPoint( Qgis::WkbType::PointZ, -1, 4, 4 ) << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 1 ) );
  patch2->setExteriorRing( patchExterior2 );
  polySurface2.addPatch( patch2 );

  QVERIFY( polySurface2.boundingBoxIntersects( QgsBox3D( 1, 3, 1, 6, 9, 2 ) ) );
  QVERIFY( !polySurface2.boundingBoxIntersects( QgsBox3D( 1, 3, 4.1, 6, 9, 6 ) ) );
}

void TestQgsPolyhedralSurface::testDropZValue()
{
  QgsPolyhedralSurface polySurface;
  QgsPolygon patch;

  // without z
  polySurface.dropZValue();
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );

  QgsLineString *exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1, 4 ) << QgsPoint( 4, 4 ) << QgsPoint( 4, 1 ) << QgsPoint( 1, 2 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface.addPatch( patch.clone() );
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );

  polySurface.dropZValue(); // not z
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::Polygon );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with z
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 10, 20, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 10, 20, 3 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface.clear();
  polySurface.addPatch( patch.clone() );

  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurfaceZ );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::PolygonZ );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 10, 20, 3 ) );

  polySurface.dropZValue();
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::Polygon );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 10, 20 ) );

  // with zm
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface.clear();
  polySurface.addPatch( patch.clone() );

  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurfaceZM );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::PolygonZM );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( Qgis::WkbType::PointZM, 1, 2, 3, 4 ) );

  polySurface.dropZValue();
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurfaceM );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::PolygonM );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( Qgis::WkbType::PointM, 1, 2, 0, 4 ) );
}

void TestQgsPolyhedralSurface::testDropMValue()
{
  QgsPolyhedralSurface polySurface;
  QgsPolygon patch;

  // without z
  polySurface.dropMValue();
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );

  QgsLineString *exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface.addPatch( patch.clone() );

  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );

  polySurface.dropMValue(); // not zm
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::Polygon );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with m
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointM, 1, 2, 0, 3 ) << QgsPoint( Qgis::WkbType::PointM, 11, 12, 0, 13 ) << QgsPoint( Qgis::WkbType::PointM, 1, 12, 0, 23 ) << QgsPoint( Qgis::WkbType::PointM, 1, 2, 0, 3 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface.clear();
  polySurface.addPatch( patch.clone() );

  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurfaceM );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::PolygonM );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( Qgis::WkbType::PointM, 1, 2, 0, 3 ) );

  polySurface.dropMValue();
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::Polygon );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with zm
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface.clear();
  polySurface.addPatch( patch.clone() );

  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurfaceZM );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::PolygonZM );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2, 3, 4 ) );

  polySurface.dropMValue();
  QCOMPARE( polySurface.wkbType(), Qgis::WkbType::PolyhedralSurfaceZ );
  QCOMPARE( polySurface.patchN( 0 )->wkbType(), Qgis::WkbType::PolygonZ );
  QCOMPARE( static_cast<const QgsLineString *>( polySurface.patchN( 0 )->exteriorRing() )->pointN( 0 ), QgsPoint( Qgis::WkbType::PointZ, 1, 2, 3 ) );
}

void TestQgsPolyhedralSurface::testCoordinateSequence()
{
  QgsPolyhedralSurface surfacePoly;
  QgsPolygon patch;

  QgsLineString *patchExterior = new QgsLineString();
  patchExterior->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) << QgsPoint( 1, 2 ) );
  patch.setExteriorRing( patchExterior );
  QgsLineString *patchInterior = new QgsLineString();
  patchInterior->setPoints( QgsPointSequence() << QgsPoint( 4.5, 3 ) << QgsPoint( 5.5, 3 ) << QgsPoint( 5, 2.5 ) << QgsPoint( 4.5, 3 ) );
  patch.addInteriorRing( patchInterior );
  surfacePoly.addPatch( patch.clone() );

  QgsCoordinateSequence coordinateSequence = surfacePoly.coordinateSequence();
  QgsCoordinateSequence expectedSequence;
  expectedSequence << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) << QgsPoint( 1, 2 ) ) << ( QgsPointSequence() << QgsPoint( 4.5, 3 ) << QgsPoint( 5.5, 3 ) << QgsPoint( 5, 2.5 ) << QgsPoint( 4.5, 3 ) ) );
  QCOMPARE( coordinateSequence, expectedSequence );
}

void TestQgsPolyhedralSurface::testChildGeometry()
{
  // childCount and childGeometry are protected method
  // use this intermediate class to test them
  class QgsPolyhedralSurfaceWithChildPublic : public QgsPolyhedralSurface
  {
    public:
      using QgsPolyhedralSurface::childCount;
      using QgsPolyhedralSurface::childGeometry;
  };

  QgsPolyhedralSurfaceWithChildPublic polySurface;
  QCOMPARE( polySurface.childCount(), 0 );

  QgsPolygon patch;
  QgsLineString *exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 1, 0.5 ) << QgsPoint( 0, 0 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface.addPatch( patch.clone() );

  QCOMPARE( polySurface.childCount(), 1 );
  QVERIFY( polySurface.childGeometry( 0 ) );
}

void TestQgsPolyhedralSurface::testWKB()
{
  QgsPolyhedralSurface polySurface1;
  QgsPolyhedralSurface polySurface2;
  QgsPolygon patch;
  QgsLineString *exteriorRing;
  QgsLineString *interiorRing;

  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 1, 0.5 ) << QgsPoint( 0, 0 ) );
  patch.setExteriorRing( exteriorRing );
  polySurface1.addPatch( patch.clone() );

  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.1, 0 ) << QgsPoint( 0.2, 0 ) << QgsPoint( 0.1, 0.05 ) << QgsPoint( 0, 0 ) );
  patch.clear();
  patch.setExteriorRing( exteriorRing );
  interiorRing = new QgsLineString();
  interiorRing->setPoints( QgsPointSequence() << QgsPoint( 0.02, 0.02 ) << QgsPoint( 0.06, 0.02 ) << QgsPoint( 0.06, 0.04 ) << QgsPoint( 0.02, 0.02 ) );
  patch.addInteriorRing( interiorRing );
  polySurface1.addPatch( patch.clone() );

  QCOMPARE( polySurface1.wkbType(), Qgis::WkbType::PolyhedralSurface );
  QByteArray wkb16 = polySurface1.asWkb();
  QCOMPARE( wkb16.size(), polySurface1.wkbSize() );

  QgsConstWkbPtr wkb16ptr( wkb16 );
  polySurface2.fromWkb( wkb16ptr );
  QCOMPARE( polySurface1, polySurface2 );

  polySurface1.clear();
  polySurface2.clear();

  // PolyhedralSurfaceZ
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 ) << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  patch.clear();
  patch.setExteriorRing( exteriorRing );
  polySurface1.addPatch( patch.clone() );

  patch.clear();
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0.1, 0, 2 ) << QgsPoint( 0.2, 0, 3 ) << QgsPoint( 0.1, 0.05, 4 ) << QgsPoint( 0, 0, 1 ) );
  interiorRing = new QgsLineString();
  interiorRing->setPoints( QgsPointSequence() << QgsPoint( 0.02, 0.02, 1 ) << QgsPoint( 0.06, 0.02, 1 ) << QgsPoint( 0.06, 0.04, 1 ) << QgsPoint( 0.02, 0.02, 1 ) );
  patch.setExteriorRing( exteriorRing );
  patch.addInteriorRing( interiorRing );
  polySurface1.addPatch( patch.clone() );

  QCOMPARE( polySurface1.wkbType(), Qgis::WkbType::PolyhedralSurfaceZ );
  wkb16 = polySurface1.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  polySurface2.fromWkb( wkb16ptr2 );
  QCOMPARE( polySurface1, polySurface2 );

  polySurface1.clear();
  polySurface2.clear();

  // PolyhedralSurfaceM
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointM, 0, 0, 0, 1 ) << QgsPoint( Qgis::WkbType::PointM, 1, 0, 0, 2 ) << QgsPoint( Qgis::WkbType::PointM, 2, 0, 0, 3 ) << QgsPoint( Qgis::WkbType::PointM, 1, 0.5, 0, 4 ) << QgsPoint( Qgis::WkbType::PointM, 0, 0, 0, 1 ) );
  patch.clear();
  patch.setExteriorRing( exteriorRing );
  polySurface1.addPatch( patch.clone() );

  patch.clear();
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointM, 0, 0, 0, 1 ) << QgsPoint( Qgis::WkbType::PointM, 0.1, 0, 0, 2 ) << QgsPoint( Qgis::WkbType::PointM, 0.2, 0, 0, 3 ) << QgsPoint( Qgis::WkbType::PointM, 0.1, 0.05, 0, 4 ) << QgsPoint( Qgis::WkbType::PointM, 0, 0, 0, 1 ) );
  patch.setExteriorRing( exteriorRing );
  interiorRing = new QgsLineString();
  interiorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointM, 0.02, 0.02, 0, 1 ) << QgsPoint( Qgis::WkbType::PointM, 0.06, 0.02, 0, 1 ) << QgsPoint( Qgis::WkbType::PointM, 0.06, 0.04, 0, 1 ) << QgsPoint( Qgis::WkbType::PointM, 0.02, 0.02, 0, 1 ) );
  patch.addInteriorRing( interiorRing );
  polySurface1.addPatch( patch.clone() );

  QCOMPARE( polySurface1.wkbType(), Qgis::WkbType::PolyhedralSurfaceM );
  wkb16 = polySurface1.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  polySurface2.fromWkb( wkb16ptr4 );
  QCOMPARE( polySurface1, polySurface2 );

  polySurface1.clear();
  polySurface2.clear();

  // PolyhedralSurfaceZM
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 1, 0, 11, 2 ) << QgsPoint( Qgis::WkbType::PointZM, 2, 0, 12, 3 ) << QgsPoint( Qgis::WkbType::PointZM, 1, 0.5, 13, 4 ) << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ) );
  patch.clear();
  patch.setExteriorRing( exteriorRing );
  polySurface1.addPatch( patch.clone() );

  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.1, 0, 11, 2 ) << QgsPoint( Qgis::WkbType::PointZM, 0.2, 0, 12, 3 ) << QgsPoint( Qgis::WkbType::PointZM, 0.1, 0.05, 13, 4 ) << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ) );
  patch.clear();
  patch.setExteriorRing( exteriorRing );
  interiorRing = new QgsLineString();
  interiorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 0.02, 0.02, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.06, 0.02, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.06, 0.04, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.02, 0.02, 10, 1 ) );
  patch.addInteriorRing( interiorRing );
  polySurface1.addPatch( patch.clone() );

  QCOMPARE( polySurface1.wkbType(), Qgis::WkbType::PolyhedralSurfaceZM );
  wkb16 = polySurface1.asWkb();
  QgsConstWkbPtr wkb16ptr5( wkb16 );
  polySurface2.fromWkb( wkb16ptr5 );
  QCOMPARE( polySurface1, polySurface2 );

  polySurface1.clear();
  polySurface2.clear();

  // bad WKB - check for no crash
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !polySurface2.fromWkb( nullPtr ) );
  QCOMPARE( polySurface2.wkbType(), Qgis::WkbType::PolyhedralSurface );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );

  QVERIFY( !polySurface2.fromWkb( wkbPointPtr ) );
  QCOMPARE( polySurface2.wkbType(), Qgis::WkbType::PolyhedralSurface );

  // GeoJSON export
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 ) << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  patch.clear();
  patch.setExteriorRing( exteriorRing );
  polySurface1.addPatch( patch.clone() );

  patch.clear();
  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0.1, 0, 2 ) << QgsPoint( 0.2, 0, 3 ) << QgsPoint( 0.1, 0.05, 4 ) << QgsPoint( 0, 0, 1 ) );
  interiorRing = new QgsLineString();
  interiorRing->setPoints( QgsPointSequence() << QgsPoint( 0.02, 0.02, 1 ) << QgsPoint( 0.06, 0.02, 1 ) << QgsPoint( 0.06, 0.04, 1 ) << QgsPoint( 0.02, 0.02, 1 ) );
  patch.setExteriorRing( exteriorRing );
  patch.addInteriorRing( interiorRing );
  polySurface1.addPatch( patch.clone() );

  QString expectedSimpleJson( "{\"coordinates\":[[[[0.0,0.0,1.0],[1.0,0.0,2.0],[2.0,0.0,3.0],[1.0,0.5,4.0],[0.0,0.0,1.0]]],[[[0.0,0.0,1.0],[0.1,0.0,2.0],[0.2,0.0,3.0],[0.1,0.05,4.0],[0.0,0.0,1.0]],[[0.02,0.02,1.0],[0.06,0.02,1.0],[0.06,0.04,1.0],[0.02,0.02,1.0]]]],\"type\":\"MultiPolygon\"}" );
  QString jsonRes = polySurface1.asJson( 2 );
  QCOMPARE( jsonRes, expectedSimpleJson );
}

void TestQgsPolyhedralSurface::testWKT()
{
  QgsPolyhedralSurface polySurface1;
  QgsPolygon patch;
  QgsLineString *exteriorRing;
  QgsLineString *interiorRing;

  exteriorRing = new QgsLineString();
  exteriorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.1, 0, 11, 2 ) << QgsPoint( Qgis::WkbType::PointZM, 0.2, 0, 12, 3 ) << QgsPoint( Qgis::WkbType::PointZM, 0.1, 0.05, 13, 4 ) << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ) );
  patch.setExteriorRing( exteriorRing );
  interiorRing = new QgsLineString();
  interiorRing->setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 0.02, 0.02, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.06, 0.02, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.06, 0.04, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.02, 0.02, 10, 1 ) );
  patch.addInteriorRing( interiorRing );
  polySurface1.addPatch( patch.clone() );

  QString wkt = polySurface1.asWkt();

  QgsPolyhedralSurface polySurface2;
  QVERIFY( polySurface2.fromWkt( wkt ) );
  QCOMPARE( polySurface1, polySurface2 );

  // bad WKT
  QVERIFY( !polySurface2.fromWkt( "Point()" ) );
  QVERIFY( polySurface2.isEmpty() );
  QVERIFY( !polySurface2.patchN( 0 ) );
  QCOMPARE( polySurface2.numPatches(), 0 );
  QVERIFY( !polySurface2.is3D() );
  QVERIFY( !polySurface2.isMeasure() );
  QCOMPARE( polySurface2.wkbType(), Qgis::WkbType::PolyhedralSurface );
}

void TestQgsPolyhedralSurface::testExport()
{
  QgsPolyhedralSurface exportPolygon;
  QgsPolygon patch;
  QgsLineString exteriorRing;
  QgsLineString interiorRing;
  QString expectedSimpleGML3;
  QString result;

  // GML document for compare
  QDomDocument doc( QStringLiteral( "gml" ) );

  // Z
  // as GML3 - M is dropped
  exteriorRing.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 10 ) << QgsPoint( Qgis::WkbType::PointZ, 1, 0, 11 ) << QgsPoint( Qgis::WkbType::PointZ, 2, 0, 12 ) << QgsPoint( Qgis::WkbType::PointZ, 1, 0.5, 13 ) << QgsPoint( Qgis::WkbType::PointZ, 0, 0, 10 ) );
  patch.setExteriorRing( exteriorRing.clone() );
  interiorRing.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 0.02, 0.02, 10 ) << QgsPoint( Qgis::WkbType::PointZ, 0.06, 0.02, 10 ) << QgsPoint( Qgis::WkbType::PointZ, 0.06, 0.04, 10 ) << QgsPoint( Qgis::WkbType::PointZ, 0.02, 0.02, 10 ) );
  patch.addInteriorRing( interiorRing.clone() );
  exportPolygon.addPatch( patch.clone() );

  expectedSimpleGML3 = QString( QStringLiteral( "<PolyhedralSurface xmlns=\"gml\"><polygonPatches xmlns=\"gml\"><PolygonPatch xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 1 0.5 13 0 0 10</posList></LinearRing></exterior><interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0.02 0.02 10 0.06 0.02 10 0.06 0.04 10 0.02 0.02 10</posList></LinearRing></interior></PolygonPatch></polygonPatches></PolyhedralSurface>" ) );
  result = elemToString( exportPolygon.asGml3( doc, 2 ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );

  // ZM
  // as GML3
  exportPolygon.clear();
  patch.clear();
  exteriorRing.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 1, 0, 11, 2 ) << QgsPoint( Qgis::WkbType::PointZM, 2, 0, 12, 3 ) << QgsPoint( Qgis::WkbType::PointZM, 1, 0.5, 13, 4 ) << QgsPoint( Qgis::WkbType::PointZM, 0, 0, 10, 1 ) );
  patch.setExteriorRing( exteriorRing.clone() );
  interiorRing.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZM, 0.02, 0.02, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.06, 0.02, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.06, 0.04, 10, 1 ) << QgsPoint( Qgis::WkbType::PointZM, 0.02, 0.02, 10, 1 ) );
  patch.addInteriorRing( interiorRing.clone() );
  exportPolygon.addPatch( patch.clone() );

  expectedSimpleGML3 = QString( QStringLiteral( "<PolyhedralSurface xmlns=\"gml\"><polygonPatches xmlns=\"gml\"><PolygonPatch xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 1 0.5 13 0 0 10</posList></LinearRing></exterior><interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0.02 0.02 10 0.06 0.02 10 0.06 0.04 10 0.02 0.02 10</posList></LinearRing></interior></PolygonPatch></polygonPatches></PolyhedralSurface>" ) );
  result = elemToString( exportPolygon.asGml3( doc, 2 ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );

  QString expectedGML3empty( QStringLiteral( "<PolyhedralSurface xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsPolyhedralSurface().asGml3( doc ) ), expectedGML3empty );
}

void TestQgsPolyhedralSurface::testCast()
{
  QVERIFY( !QgsPolyhedralSurface().cast( nullptr ) );

  QgsPolyhedralSurface pCast;
  QVERIFY( QgsPolyhedralSurface().cast( &pCast ) );

  QgsPolyhedralSurface pCast2;
  pCast2.fromWkt( QStringLiteral( "PolyhedralSurfaceZ((0 0 0, 0 1 1, 1 0 2, 0 0 0))" ) );
  QVERIFY( QgsPolyhedralSurface().cast( &pCast2 ) );

  pCast2.fromWkt( QStringLiteral( "PolyhedralSurfaceM((0 0 1, 0 1 2, 1 0 3, 0 0 1))" ) );
  QVERIFY( QgsPolyhedralSurface().cast( &pCast2 ) );

  pCast2.fromWkt( QStringLiteral( "PolyhedralSurfaceZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1))" ) );
  QVERIFY( QgsPolyhedralSurface().cast( &pCast2 ) );

  QVERIFY( !pCast2.fromWkt( QStringLiteral( "PolyhedralSurfaceZ((111111))" ) ) );
}

void TestQgsPolyhedralSurface::testIsValid()
{
  QString error;
  bool isValid;

  // an empty QgsPolyhedralSurface is valid
  QgsPolyhedralSurface polySurfaceEmpty;
  isValid = polySurfaceEmpty.isValid( error );
  QVERIFY( error.isEmpty() );
  QVERIFY( isValid );

  // a QgsPolyhedralSurface with a valid QgsPolygon is valid
  QgsPolyhedralSurface polySurface1;
  polySurface1.fromWkt( QStringLiteral( "PolyhedralSurfaceZ((0 0 0, 0 1 1, 1 0 2, 0 0 0))" ) );
  isValid = polySurface1.isValid( error );
  QVERIFY( error.isEmpty() );
  QVERIFY( isValid );

  // a QgsPolyhedralSurface with an invalid QgsPolygon is not valid
  QgsPolyhedralSurface polySurface2;
  QgsPolygon patch;
  QgsLineString lineString;

  lineString.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 11, 2, 3 ) << QgsPoint( Qgis::WkbType::PointZ, 4, 12, 13 ) << QgsPoint( Qgis::WkbType::PointZ, 11, 12, 13 ) << QgsPoint( Qgis::WkbType::PointZ, 11, 22, 23 ) << QgsPoint( Qgis::WkbType::PointZ, 11, 2, 3 ) );
  patch.setExteriorRing( lineString.clone() );

  lineString.setPoints( QgsPointSequence() << QgsPoint( Qgis::WkbType::PointZ, 10, 2, 5 ) << QgsPoint( Qgis::WkbType::PointZ, 11, 2, 5 ) << QgsPoint( Qgis::WkbType::PointZ, 10, 2, 5 ) );
  patch.addInteriorRing( lineString.clone() );

  polySurface2.addPatch( patch.clone() );

  isValid = polySurface2.isValid( error );
  QCOMPARE( error, "Polygon 0 is invalid: Too few points in geometry component" );
  QVERIFY( !isValid );
}


QGSTEST_MAIN( TestQgsPolyhedralSurface )
#include "testqgspolyhedralsurface.moc"
