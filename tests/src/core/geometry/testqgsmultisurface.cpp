/***************************************************************************
     testqgsmultisurface.cpp
     --------------------------------------
    Date                 : August 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
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
#include <QString>

#include "qgscircularstring.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"
#include "qgsmultisurface.h"
#include "qgspoint.h"

#include "testgeometryutils.h"

class TestQgsMultiSurface: public QObject
{
    Q_OBJECT
  private slots:
    void constructor();
    void addGeometry();
    void addBadGeometry();
    void addGeometryInitialDimension();
    void addGeometryZ();
    void addGeometryM();
    void addGeometryZM();
    void insertGeometry();
    void surfaceN();
    void assignment();
    void clone();
    void copy();
    void clear();
    void boundary();
    void cast();
    void toCurveType();
    void toFromWKT();
    void toFromWKB();
    void toFromWkbZM();
    void exportImport();
};

void TestQgsMultiSurface::constructor()
{
  QgsMultiSurface ms;

  QVERIFY( ms.isEmpty() );
  QCOMPARE( ms.nCoordinates(), 0 );
  QCOMPARE( ms.ringCount(), 0 );
  QCOMPARE( ms.partCount(), 0 );
  QVERIFY( !ms.is3D() );
  QVERIFY( !ms.isMeasure() );
  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( ms.wktTypeStr(), QString( "MultiSurface" ) );
  QCOMPARE( ms.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( ms.dimension(), 0 );
  QVERIFY( !ms.hasCurvedSegments() );
  QCOMPARE( ms.area(), 0.0 );
  QCOMPARE( ms.perimeter(), 0.0 );
  QCOMPARE( ms.numGeometries(), 0 );
  QVERIFY( !ms.geometryN( 0 ) );
  QVERIFY( !ms.geometryN( -1 ) );
  QCOMPARE( ms.vertexCount( 0, 0 ), 0 );
  QCOMPARE( ms.vertexCount( 0, 1 ), 0 );
  QCOMPARE( ms.vertexCount( 1, 0 ), 0 );
}

void TestQgsMultiSurface::addGeometry()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 )
                  << QgsPoint( 2, 11 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QVERIFY( !ms.isEmpty() );
  QCOMPARE( ms.numGeometries(), 1 );
  QCOMPARE( ms.nCoordinates(), 3 );
  QCOMPARE( ms.ringCount(), 1 );
  QCOMPARE( ms.partCount(), 1 );
  QVERIFY( !ms.is3D() );
  QVERIFY( !ms.isMeasure() );
  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( ms.wktTypeStr(), QString( "MultiSurface" ) );
  QCOMPARE( ms.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( ms.dimension(), 2 );
  QVERIFY( ms.hasCurvedSegments() );
  QVERIFY( ms.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) ), part );
  QVERIFY( !ms.geometryN( 100 ) );
  QVERIFY( !ms.geometryN( -1 ) );
  QCOMPARE( ms.vertexCount( 0, 0 ), 3 );
  QCOMPARE( ms.vertexCount( 1, 0 ), 0 );

  //add another part
  ms.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 )
                  << QgsPoint( 2, 11 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.vertexCount( 0, 0 ), 3 );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 )
                  << QgsPoint( 3, 13 )  << QgsPoint( 9, 12 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.vertexCount( 1, 0 ), 3 );
  QCOMPARE( ms.numGeometries(), 2 );
  QVERIFY( ms.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of parts type
  ms.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 )
                  << QgsPoint( 3, 13 )  << QgsPoint( 9, 12 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 )
                  << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( ms.vertexCount( 0, 0 ), 3 );
  QCOMPARE( ms.vertexCount( 1, 0 ), 3 );
  QCOMPARE( ms.vertexCount( 2, 0 ), 0 );
  QCOMPARE( ms.vertexCount( -1, 0 ), 0 );
  QCOMPARE( ms.nCoordinates(), 6 );
  QCOMPARE( ms.ringCount(), 1 );
  QCOMPARE( ms.partCount(), 2 );
  QVERIFY( !ms.is3D() );

  auto cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) );
  auto ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 3, 13 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 9, 12 ) );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 1 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 2, 11 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 1, 10 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( ms.vertexCount( 0, 0 ), 3 );
  QCOMPARE( ms.vertexCount( 1, 0 ), 3 );
  QCOMPARE( ms.vertexCount( 2, 0 ), 3 );
  QCOMPARE( ms.nCoordinates(), 9 );
  QCOMPARE( ms.ringCount(), 1 );
  QCOMPARE( ms.partCount(), 3 );
  QVERIFY( !ms.is3D() );
  QVERIFY( !ms.isMeasure() );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 2 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 32, 41 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 21, 30 ) );
}

void TestQgsMultiSurface::addBadGeometry()
{
  QgsMultiSurface ms;

  //try with nullptr
  ms.addGeometry( nullptr );

  QVERIFY( ms.isEmpty() );
  QCOMPARE( ms.nCoordinates(), 0 );
  QCOMPARE( ms.ringCount(), 0 );
  QCOMPARE( ms.partCount(), 0 );
  QCOMPARE( ms.numGeometries(), 0 );
  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );
  QVERIFY( !ms.geometryN( 0 ) );
  QVERIFY( !ms.geometryN( -1 ) );

  // not a surface
  QVERIFY( !ms.addGeometry( new QgsPoint() ) );
  QVERIFY( ms.isEmpty() );
  QCOMPARE( ms.nCoordinates(), 0 );
  QCOMPARE( ms.ringCount(), 0 );
  QCOMPARE( ms.partCount(), 0 );
  QCOMPARE( ms.numGeometries(), 0 );
  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );
  QVERIFY( !ms.geometryN( 0 ) );
  QVERIFY( !ms.geometryN( -1 ) );
}

void TestQgsMultiSurface::addGeometryInitialDimension()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QVERIFY( ms.is3D() );
  QVERIFY( !ms.isMeasure() );
  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QCOMPARE( ms.wktTypeStr(), QString( "MultiSurfaceZ" ) );
  QCOMPARE( ms.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) ) ), part );

  ms.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QVERIFY( !ms.is3D() );
  QVERIFY( ms.isMeasure() );
  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QCOMPARE( ms.wktTypeStr(), QString( "MultiSurfaceM" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) ) ), part );

  ms.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QVERIFY( ms.is3D() );
  QVERIFY( ms.isMeasure() );
  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( ms.wktTypeStr(), QString( "MultiSurfaceZM" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) ) ), part );
}

void TestQgsMultiSurface::addGeometryZ()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 )
                  << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZ );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 )
                  << QgsPoint( 3, 31 ) << QgsPoint( 2, 20 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QVERIFY( ms.is3D() );

  auto cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) );
  auto ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 1, 10, 2 ) );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 1 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 2, 20, 0 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QVERIFY( ms.is3D() );
  QVERIFY( !ms.isMeasure() );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 2 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 5, 50, 0 ) );
}

void TestQgsMultiSurface::addGeometryM()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceM );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 )
                  << QgsPoint( 3, 31 )   << QgsPoint( 2, 20 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QVERIFY( ms.isMeasure() );

  auto cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) );
  auto ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 1 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 )
                  << QgsPoint( 14, 15, 16 ) << QgsPoint( 11, 12, 13 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QVERIFY( !ms.is3D() );
  QVERIFY( ms.isMeasure() );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 2 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
}

void TestQgsMultiSurface::addGeometryZM()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZM );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( ms.isMeasure() );
  QVERIFY( ms.is3D() );

  auto cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) );
  auto ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 1 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 )
                  << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 )
                  << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( ms.is3D() );
  QVERIFY( ms.isMeasure() );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 2 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 )
                  << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 )
                  << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( ms.is3D() );
  QVERIFY( ms.isMeasure() );

  cp = static_cast< const QgsCurvePolygon * >( ms.geometryN( 3 ) );
  ext = static_cast< const QgsCircularString *>( cp->exteriorRing() );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
}

void TestQgsMultiSurface::insertGeometry()
{
  QgsMultiSurface ms;

  ms.insertGeometry( nullptr, 0 );
  QVERIFY( ms.isEmpty() );
  QCOMPARE( ms.numGeometries(), 0 );

  ms.insertGeometry( nullptr, -1 );
  QVERIFY( ms.isEmpty() );
  QCOMPARE( ms.numGeometries(), 0 );

  ms.insertGeometry( nullptr, 100 );
  QVERIFY( ms.isEmpty() );
  QCOMPARE( ms.numGeometries(), 0 );

  ms.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( ms.isEmpty() );
  QCOMPARE( ms.numGeometries(), 0 );

  QgsCurvePolygon part;
  QgsCircularString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 )
                  << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) ) ;
  part.setExteriorRing( ring.clone() );

  ms.insertGeometry( part.clone(), 0 );
  QVERIFY( !ms.isEmpty() );
  QCOMPARE( ms.numGeometries(), 1 );
}

void TestQgsMultiSurface::surfaceN()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 )
                  << QgsPoint( 2, 11 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 )
                  << QgsPoint( 3, 13 )  << QgsPoint( 9, 12 ) );
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( *ms.geometryN( 0 ), *ms.surfaceN( 0 ) );
  QCOMPARE( *ms.geometryN( 0 ), *std::as_const( ms ).surfaceN( 0 ) );
  QCOMPARE( *ms.geometryN( 1 ), *ms.surfaceN( 1 ) );
  QCOMPARE( *ms.geometryN( 1 ), *std::as_const( ms ).surfaceN( 1 ) );
}

void TestQgsMultiSurface::assignment()
{
  QgsMultiSurface ms1;
  QgsMultiSurface ms2;

  ms1 = ms2;
  QCOMPARE( ms1.numGeometries(), 0 );

  QgsMultiSurface ms3;
  QgsCurvePolygon part;
  QgsCircularString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms3.addGeometry( part.clone() );
  ms3.addGeometry( part.clone() );

  ms1 = ms3;

  QCOMPARE( ms1.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 1 ) ), part );
}

void TestQgsMultiSurface::clone()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  std::unique_ptr< QgsMultiSurface >cloned( ms.clone() );

  QVERIFY( cloned->isEmpty() );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );
  ms.addGeometry( part.clone() );

  cloned.reset( ms.clone() );

  QCOMPARE( cloned->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( cloned->geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( cloned->geometryN( 1 ) ), part );
}

void TestQgsMultiSurface::copy()
{
  QgsCurvePolygon part;
  QgsCircularString ring;
  QgsMultiSurface ms1;

  QgsMultiSurface ms2( ms1 );
  QVERIFY( ms2.isEmpty() );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );
  ms1.addGeometry( part.clone() );

  QgsMultiSurface ms3( ms1 );

  QCOMPARE( ms3.numGeometries(), 2 );
  QCOMPARE( ms3.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms3.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms3.geometryN( 1 ) ), part );
}

void TestQgsMultiSurface::clear()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );
  ms.addGeometry( part.clone() );

  QCOMPARE( ms.numGeometries(), 2 );

  ms.clear();

  QVERIFY( ms.isEmpty() );
  QCOMPARE( ms.numGeometries(), 0 );
  QCOMPARE( ms.nCoordinates(), 0 );
  QCOMPARE( ms.ringCount(), 0 );
  QCOMPARE( ms.partCount(), 0 );
  QVERIFY( !ms.is3D() );
  QVERIFY( !ms.isMeasure() );
  QCOMPARE( ms.wkbType(), QgsWkbTypes::MultiSurface );
}

void TestQgsMultiSurface::boundary()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part1;
  QgsCircularString ring;

  QVERIFY( !ms.boundary() );

  ring.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  part1.setExteriorRing( ring.clone() );
  ms.addGeometry( part1.clone() );

  QgsAbstractGeometry *boundary = ms.boundary();
  QgsMultiCurve *mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part1.exteriorRing() );

  delete boundary;

  // add another QgsCircularString
  QgsCurvePolygon part2;
  ring.setPoints( QgsPointSequence() << QgsPoint( 10, 10 )
                  << QgsPoint( 11, 10 ) << QgsPoint( 10, 10 ) );
  part2.setExteriorRing( ring.clone() );
  ms.addGeometry( part2.clone() );

  boundary = ms.boundary();
  mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part1.exteriorRing() );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 1 ) ), *part2.exteriorRing() );

  delete boundary;

  //boundary with z
  ms.clear();

  part1.clear();
  part2.clear();

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 )
                  << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) );
  part1.setExteriorRing( ring.clone() );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) );
  part2.setExteriorRing( ring.clone() );

  ms.addGeometry( part1.clone() );
  ms.addGeometry( part2.clone() );

  boundary = ms.boundary();
  mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part1.exteriorRing() );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 1 ) ), *part2.exteriorRing() );

  delete boundary;
}

void TestQgsMultiSurface::cast()
{
  QVERIFY( !QgsMultiSurface::cast( nullptr ) );

  QgsMultiSurface ms;
  QVERIFY( QgsMultiSurface::cast( &ms ) );

  ms.clear();
  ms.fromWkt( QStringLiteral( "MultiSurfaceZ()" ) );
  QVERIFY( QgsMultiSurface::cast( &ms ) );

  ms.clear();
  ms.fromWkt( QStringLiteral( "MultiSurfaceM()" ) );
  QVERIFY( QgsMultiSurface::cast( &ms ) );

  ms.clear();
  ms.fromWkt( QStringLiteral( "MultiSurfaceZM()" ) );
  QVERIFY( QgsMultiSurface::cast( &ms ) );
}

void TestQgsMultiSurface::toCurveType()
{
  QgsMultiSurface ms;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms.addGeometry( part.clone() );
  ms.addGeometry( part.clone() );

  std::unique_ptr< QgsMultiSurface > curveType( ms.toCurveType() );

  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( curveType->numGeometries(), 2 );

  QCOMPARE( *static_cast< const QgsCurvePolygon * >( curveType->geometryN( 0 ) ),
            *static_cast< const QgsCurvePolygon * >( ms.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( curveType->geometryN( 1 ) ),
            *static_cast< const QgsCurvePolygon * >( ms.geometryN( 1 ) ) );
}

void TestQgsMultiSurface::toFromWKT()
{
  QgsMultiSurface ms1;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  QString wkt = ms1.asWkt();
  QVERIFY( !wkt.isEmpty() );

  QgsMultiSurface ms2;
  QVERIFY( ms2.fromWkt( wkt ) );

  QCOMPARE( ms2.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 0 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 1 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 1 ) ) );

  //bad WKT
  ms1.clear();
  QVERIFY( !ms1.fromWkt( "Point()" ) );
  QVERIFY( ms1.isEmpty() );
  QCOMPARE( ms1.numGeometries(), 0 );
  QCOMPARE( ms1.wkbType(), QgsWkbTypes::MultiSurface );
}

void TestQgsMultiSurface::toFromWKB()
{
  QgsMultiSurface ms1;
  QgsCurvePolygon part;
  QgsCircularString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 )
                  << QgsPoint( QgsWkbTypes::Point, 43, 43 )
                  << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  QByteArray wkb = ms1.asWkb();
  QgsMultiSurface ms2;
  QgsConstWkbPtr wkbPtr( wkb );
  ms2.fromWkb( wkbPtr );

  QCOMPARE( ms2.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 0 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 1 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  ms2.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !ms2.fromWkb( nullPtr ) );
  QCOMPARE( ms2.wkbType(), QgsWkbTypes::MultiSurface );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );

  QVERIFY( !ms2.fromWkb( wkbPointPtr ) );
  QCOMPARE( ms2.wkbType(), QgsWkbTypes::MultiSurface );
}

void TestQgsMultiSurface::toFromWkbZM()
{
  QgsMultiSurface ms1;
  QgsCurvePolygon part;
  QgsCircularString ring;

  //parts with Z

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 )
                  << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 )
                  << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  QByteArray wkb = ms1.asWkb();
  QgsConstWkbPtr wkbPtr1( wkb );
  QgsMultiSurface ms2;
  ms2.fromWkb( wkbPtr1 );

  QCOMPARE( ms2.numGeometries(), 2 );
  QCOMPARE( ms2.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 0 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 1 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 1 ) ) );

  //parts with m
  ms1.clear();
  ms2.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  wkb = ms1.asWkb();
  QgsConstWkbPtr wkbPtr2( wkb );
  ms2.fromWkb( wkbPtr2 );

  QCOMPARE( ms2.numGeometries(), 2 );
  QCOMPARE( ms2.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 0 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 1 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 1 ) ) );

  // parts with ZM
  ms1.clear();
  ms2.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  ms1.addGeometry( part.clone() );

  wkb = ms1.asWkb();
  QgsConstWkbPtr wkbPtr4( wkb );
  ms2.fromWkb( wkbPtr4 );

  QCOMPARE( ms2.numGeometries(), 2 );
  QCOMPARE( ms2.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 0 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( ms2.geometryN( 1 ) ),
            *static_cast< const QgsCurvePolygon * >( ms1.geometryN( 1 ) ) );
}

void TestQgsMultiSurface::exportImport()
{
  //as JSON
  QgsMultiSurface exportC;
  QgsLineString lineRing;
  QgsCurvePolygon part;
  QgsCircularString ring;

  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                      << QgsPoint( QgsWkbTypes::Point, 7, 13 )
                      << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                      << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.setExteriorRing( lineRing.clone() );
  exportC.addGeometry( part.clone() );

  part.clear();
  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 )
                      << QgsPoint( QgsWkbTypes::Point, 27, 43 )
                      << QgsPoint( QgsWkbTypes::Point, 43, 43 )
                      << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.setExteriorRing( lineRing.clone() );
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 7,13 3,13 7,17</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 27,43 43,43 27,37</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  QString res = elemToString( exportC.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );

  QString expectedGML2empty( QStringLiteral( "<MultiPolygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiSurface().asGml2( doc ) ), expectedGML2empty );

  //as GML3

  QString expectedSimpleGML3( QStringLiteral( "<MultiSurface xmlns=\"gml\"><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 7 13 3 13 7 17</posList></LinearRing></exterior></Polygon></surfaceMember><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 27 43 43 43 27 37</posList></LinearRing></exterior></Polygon></surfaceMember></MultiSurface>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );

  QString expectedGML3empty( QStringLiteral( "<MultiSurface xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiSurface().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[[[7.0,17.0],[7.0,13.0],[3.0,13.0],[7.0,17.0]]],[[[27.0,37.0],[27.0,43.0],[43.0,43.0],[27.0,37.0]]]],\"type\":\"MultiPolygon\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 17, 27 )
                      << QgsPoint( QgsWkbTypes::Point, 17, 28 )
                      << QgsPoint( QgsWkbTypes::Point, 18, 28 )
                      << QgsPoint( QgsWkbTypes::Point, 17, 27 ) ) ;
  part.addInteriorRing( lineRing.clone() );
  exportC.addGeometry( part.clone() );

  QString expectedJsonWithRings( "{\"coordinates\":[[[[7.0,17.0],[7.0,13.0],[3.0,13.0],[7.0,17.0]]],[[[27.0,37.0],[27.0,43.0],[43.0,43.0],[27.0,37.0]]],[[[27.0,37.0],[27.0,43.0],[43.0,43.0],[27.0,37.0]],[[17.0,27.0],[17.0,28.0],[18.0,28.0],[17.0,27.0]]]],\"type\":\"MultiPolygon\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedJsonWithRings );

  QgsMultiSurface exportFloat;

  part.clear();
  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0.1234, 0.1234 )
                      << QgsPoint( QgsWkbTypes::Point, 0.1234, 1.2344 )
                      << QgsPoint( QgsWkbTypes::Point, 1.2344, 1.2344 )
                      << QgsPoint( QgsWkbTypes::Point, 0.1234, 0.1234 ) ) ;
  part.setExteriorRing( lineRing.clone() );
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[[[[0.123,0.123],[0.123,1.234],[1.234,1.234],[0.123,0.123]]]],\"type\":\"MultiPolygon\"}" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.123,0.123 0.123,1.234 1.234,1.234 0.123,0.123</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiSurface xmlns=\"gml\"><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.123 0.123 0.123 1.234 1.234 1.234 0.123 0.123</posList></LinearRing></exterior></Polygon></surfaceMember></MultiSurface>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0.123,0.123,0 0.123,1.234,0 1.234,1.234,0 0.123,0.123,0</coordinates></LinearRing></outerBoundaryIs></Polygon></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );
}


QGSTEST_MAIN( TestQgsMultiSurface )
#include "testqgsmultisurface.moc"
