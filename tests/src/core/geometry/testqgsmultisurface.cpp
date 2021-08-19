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
    void multiSurface();
};

void TestQgsMultiSurface::multiSurface()
{
  //test constructor
  QgsMultiSurface c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiSurface" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 0 );
  QCOMPARE( c1.vertexCount( 0, 1 ), 0 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //addGeometry
  //try with nullptr
  c1.addGeometry( nullptr );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiSurface );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a surface
  QVERIFY( !c1.addGeometry( new QgsPoint() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiSurface );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsCurvePolygon part;
  QgsCircularString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 3 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiSurface" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( c1.dimension(), 2 );
  QVERIFY( c1.hasCurvedSegments() );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  QgsMultiSurface c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiSurfaceZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiSurface c3;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiSurfaceM" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiSurface c4;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiSurfaceZM" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiSurface c6;
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 1, 10 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 )  << QgsPoint( 9, 12 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of parts type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurface );
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 6 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QVERIFY( !c6.is3D() );
  const QgsCurvePolygon *ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 3, 13 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 9, 12 ) );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 2, 11 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 1, 10 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 3 );
  QCOMPARE( c6.nCoordinates(), 9 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 32, 41 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 21, 30 ) );

  c6.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) << QgsPoint( 2, 20 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 1, 10, 2 ) );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 2, 20, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 5, 50, 0 ) );

  c6.clear();
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurface );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceM );
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 )   << QgsPoint( 2, 20 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 ) << QgsPoint( 11, 12, 13 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );

  c6.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 )
                  << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 )
                  << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 3 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );

  //clear
  QgsMultiSurface c7;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c7.addGeometry( part.clone() );
  c7.addGeometry( part.clone() );
  QCOMPARE( c7.numGeometries(), 2 );
  c7.clear();
  QVERIFY( c7.isEmpty() );
  QCOMPARE( c7.numGeometries(), 0 );
  QCOMPARE( c7.nCoordinates(), 0 );
  QCOMPARE( c7.ringCount(), 0 );
  QCOMPARE( c7.partCount(), 0 );
  QVERIFY( !c7.is3D() );
  QVERIFY( !c7.isMeasure() );
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiSurface );

  //clone
  QgsMultiSurface c11;
  std::unique_ptr< QgsMultiSurface >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( part.clone() );
  c11.addGeometry( part.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  ls = static_cast< const QgsCurvePolygon * >( cloned->geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCurvePolygon * >( cloned->geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //copy constructor
  QgsMultiSurface c12;
  QgsMultiSurface c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( part.clone() );
  c12.addGeometry( part.clone() );
  QgsMultiSurface c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  ls = static_cast< const QgsCurvePolygon * >( c14.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCurvePolygon * >( c14.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //assignment operator
  QgsMultiSurface c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  ls = static_cast< const QgsCurvePolygon * >( c15.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCurvePolygon * >( c15.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //toCurveType
  std::unique_ptr< QgsMultiSurface > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsCurvePolygon *curve = static_cast< const QgsCurvePolygon * >( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, *static_cast< const QgsCurvePolygon * >( c12.geometryN( 0 ) ) );
  curve = static_cast< const QgsCurvePolygon * >( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, *static_cast< const QgsCurvePolygon * >( c12.geometryN( 1 ) ) );

  //to/fromWKB
  QgsMultiSurface c16;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )  << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  QByteArray wkb16 = c16.asWkb();
  QgsMultiSurface c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 1 ) ) );

  //parts with Z
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 ) << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 1 ) ) );

  //parts with m
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 ) << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 1 ) ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )  << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurface );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurface );

  //to/from WKT
  QgsMultiSurface c18;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c18.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c18.addGeometry( part.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiSurface c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c19.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c18.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c19.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c18.geometryN( 1 ) ) );

  //bad WKT
  QgsMultiSurface c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiSurface );

  //as JSON
  QgsMultiSurface exportC;
  QgsLineString lineRing;
  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 7, 13 )  << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( lineRing.clone() );
  exportC.addGeometry( part.clone() );
  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 27, 43 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )  << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
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

  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 17, 27 ) << QgsPoint( QgsWkbTypes::Point, 17, 28 ) << QgsPoint( QgsWkbTypes::Point, 18, 28 )  << QgsPoint( QgsWkbTypes::Point, 17, 27 ) ) ;
  part.addInteriorRing( lineRing.clone() );
  exportC.addGeometry( part.clone() );

  QString expectedJsonWithRings( "{\"coordinates\":[[[[7.0,17.0],[7.0,13.0],[3.0,13.0],[7.0,17.0]]],[[[27.0,37.0],[27.0,43.0],[43.0,43.0],[27.0,37.0]]],[[[27.0,37.0],[27.0,43.0],[43.0,43.0],[27.0,37.0]],[[17.0,27.0],[17.0,28.0],[18.0,28.0],[17.0,27.0]]]],\"type\":\"MultiPolygon\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedJsonWithRings );

  QgsMultiSurface exportFloat;
  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0.1234, 0.1234 ) << QgsPoint( QgsWkbTypes::Point, 0.1234, 1.2344 ) << QgsPoint( QgsWkbTypes::Point, 1.2344, 1.2344 ) << QgsPoint( QgsWkbTypes::Point, 0.1234, 0.1234 ) ) ;
  part.clear();
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

  // insert geometry
  QgsMultiSurface rc;
  rc.clear();
  rc.insertGeometry( nullptr, 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, -1 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, 100 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( part.clone(), 0 );
  QVERIFY( !rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 1 );

  // cast
  QVERIFY( !QgsMultiSurface().cast( nullptr ) );
  QgsMultiSurface pCast;
  QVERIFY( QgsMultiSurface().cast( &pCast ) );
  QgsMultiSurface pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiSurfaceZ()" ) );
  QVERIFY( QgsMultiSurface().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiSurfaceM()" ) );
  QVERIFY( QgsMultiSurface().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiSurfaceZM()" ) );
  QVERIFY( QgsMultiSurface().cast( &pCast2 ) );

  //boundary
  QgsMultiSurface multiSurface;
  QVERIFY( !multiSurface.boundary() );
  QgsCircularString boundaryLine1;
  boundaryLine1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  part.clear();
  part.setExteriorRing( boundaryLine1.clone() );
  multiSurface.addGeometry( part.clone() );
  QgsAbstractGeometry *boundary = multiSurface.boundary();
  QgsMultiCurve *mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part.exteriorRing() );
  delete boundary;
  // add another QgsCircularString
  QgsCircularString boundaryLine2;
  boundaryLine2.setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 10, 10 ) );
  QgsCurvePolygon part2;
  part2.setExteriorRing( boundaryLine2.clone() );
  multiSurface.addGeometry( part2.clone() );
  boundary = multiSurface.boundary();
  mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part.exteriorRing() );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 1 ) ), *part2.exteriorRing() );
  delete boundary;

  //boundary with z
  QgsCircularString boundaryLine4;
  boundaryLine4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) );
  part.clear();
  part.setExteriorRing( boundaryLine4.clone() );
  QgsCircularString boundaryLine5;
  boundaryLine5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) );
  part2.clear();
  part2.setExteriorRing( boundaryLine5.clone() );
  QgsMultiSurface multiSurface2;
  multiSurface2.addGeometry( part.clone() );
  multiSurface2.addGeometry( part2.clone() );

  boundary = multiSurface2.boundary();
  mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part.exteriorRing() );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 1 ) ), *part2.exteriorRing() );
  delete boundary;
}


QGSTEST_MAIN( TestQgsMultiSurface )
#include "testqgsmultisurface.moc"
