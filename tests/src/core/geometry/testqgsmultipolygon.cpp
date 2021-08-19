/***************************************************************************
     testqgsmultipolygon.cpp
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

#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgspoint.h"
#include "qgspolygon.h"

#include "testgeometryutils.h"

class TestQgsMultiPolygon: public QObject
{
    Q_OBJECT
  private slots:
    void multiPolygon();
};

void TestQgsMultiPolygon::multiPolygon()
{
  //test constructor
  QgsMultiPolygon c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiPolygon" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiPolygon" ) );
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
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPolygon );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a surface
  QVERIFY( !c1.addGeometry( new QgsPoint() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPolygon );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsPolygon part;
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 2, 21 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 4 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiPolygon" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiPolygon" ) );
  QCOMPARE( c1.dimension(), 2 );
  QVERIFY( !c1.hasCurvedSegments() );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 4 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 )  << QgsPoint( QgsWkbTypes::PointZ, 30, 31, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  QgsMultiPolygon c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiPolygonZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiPolygon" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiPolygon c3;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 30, 31, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiPolygonM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiPolygonM" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiPolygon c4;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 30, 31, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiPolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiPolygon c6;
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 10, 21 ) << QgsPoint( 1, 10 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 4 );
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 )  << QgsPoint( 4, 17 ) << QgsPoint( 9, 12 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 4 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c6.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of parts type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygon );
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 10, 13, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( c6.vertexCount( 0, 0 ), 4 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 4 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 8 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QVERIFY( !c6.is3D() );
  const QgsPolygon *ls = static_cast< const QgsPolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 3, 13 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 4, 17 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 9, 12 ) );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 2, 11 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 10, 13 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 1, 10 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 42, 61, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( c6.vertexCount( 0, 0 ), 4 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 4 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 4 );
  QCOMPARE( c6.nCoordinates(), 12 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 32, 41 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 42, 61 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 21, 30 ) );

  c6.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 9, 15, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZ );
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 )  << QgsPoint( 7, 34 ) << QgsPoint( 2, 20 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 9, 15, 3 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 1, 10, 2 ) );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 7, 34, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 2, 20, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 65, 0, 7 ) << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 9, 65, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 5, 50, 0 ) );

  c6.clear();
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygon );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 76, 0, 8 ) << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonM );
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) << QgsPoint( 7, 39 ) << QgsPoint( 2, 20 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonM );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 9, 76, 0, 8 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 7, 39, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 ) << QgsPoint( 24, 21, 5 ) << QgsPoint( 11, 12, 13 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 24, 21, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );

  c6.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 9 ) << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZM );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 13, 27 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 9 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 13, 27, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 )
                  << QgsPoint( QgsWkbTypes::PointZ, 93, 85, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 93, 85, 10, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 )
                  << QgsPoint( QgsWkbTypes::PointM, 185, 193, 0, 13 ) << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 3 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 185, 193, 0, 13 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );

  //clear
  QgsMultiPolygon c7;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 15 ) << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
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
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiPolygon );

  //clone
  QgsMultiPolygon c11;
  std::unique_ptr< QgsMultiPolygon >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( part.clone() );
  c11.addGeometry( part.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  ls = static_cast< const QgsPolygon * >( cloned->geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsPolygon * >( cloned->geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //copy constructor
  QgsMultiPolygon c12;
  QgsMultiPolygon c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( part.clone() );
  c12.addGeometry( part.clone() );
  QgsMultiPolygon c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiPolygonZM );
  ls = static_cast< const QgsPolygon * >( c14.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsPolygon * >( c14.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //assignment operator
  QgsMultiPolygon c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  ls = static_cast< const QgsPolygon * >( c15.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsPolygon * >( c15.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //toCurveType
  std::unique_ptr< QgsMultiSurface > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsPolygon *curve = static_cast< const QgsPolygon * >( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, *static_cast< const QgsPolygon * >( c12.geometryN( 0 ) ) );
  curve = static_cast< const QgsPolygon * >( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, *static_cast< const QgsPolygon * >( c12.geometryN( 1 ) ) );

  //to/fromWKB
  QgsMultiPolygon c16;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 9, 27 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 ) << QgsPoint( QgsWkbTypes::Point, 29, 39 ) << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  QByteArray wkb16 = c16.asWkb();
  QCOMPARE( wkb16.size(), c16.wkbSize() );
  QgsMultiPolygon c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 1 ) ) );

  //parts with Z
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 27, 5 )  << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 ) << QgsPoint( QgsWkbTypes::PointZ, 87, 54, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 1 ) ) );

  //parts with m
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 21, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 37, 31, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygonM );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 1 ) ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 19, 13, 5, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygon );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygon );

  //to/from WKT
  QgsMultiPolygon c18;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 13, 19, 3, 10 ) << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c18.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 17, 49, 31, 53 ) << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c18.addGeometry( part.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiPolygon c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPolygon * >( c19.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c18.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c19.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c18.geometryN( 1 ) ) );

  //bad WKT
  QgsMultiPolygon c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiPolygon );

  //as JSON
  QgsMultiPolygon exportC;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 21 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )
                  << QgsPoint( QgsWkbTypes::Point, 41, 39 ) << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 3,13 7,21 7,17</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 43,43 41,39 27,37</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  QString res = elemToString( exportC.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiPolygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPolygon().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13 7 21 7 17</posList></LinearRing></exterior></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43 41 39 27 37</posList></LinearRing></exterior></Polygon></polygonMember></MultiPolygon>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiPolygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPolygon().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[[[7.0,17.0],[3.0,13.0],[7.0,21.0],[7.0,17.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]]]],\"type\":\"MultiPolygon\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 17, 27 ) << QgsPoint( QgsWkbTypes::Point, 18, 28 ) << QgsPoint( QgsWkbTypes::Point, 19, 37 ) << QgsPoint( QgsWkbTypes::Point, 17, 27 ) ) ;
  part.addInteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );

  QString expectedJsonWithRings( "{\"coordinates\":[[[[7.0,17.0],[3.0,13.0],[7.0,21.0],[7.0,17.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]],[[17.0,27.0],[18.0,28.0],[19.0,37.0],[17.0,27.0]]]],\"type\":\"MultiPolygon\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedJsonWithRings );

  QgsMultiPolygon exportFloat;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 8 / 3.0, 27 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportFloat.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 ) << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( "{\"coordinates\":[[[[2.333,5.667],[0.6,4.333],[2.667,9.0],[2.333,5.667]]],[[[9.0,4.111],[1.049,1.024],[9.0,4.111]]]],\"type\":\"MultiPolygon\"}" );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 0.6,4.333 2.667,9 2.333,5.667</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 1.049,1.024 9,4.111</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333 2.667 9 2.333 5.667</posList></LinearRing></exterior></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024 9 4.111</posList></LinearRing></exterior></Polygon></polygonMember></MultiPolygon>" );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKmlPrec3( "<MultiGeometry><Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>2.333,5.667,0 0.6,4.333,0 2.667,9,0 2.333,5.667,0</coordinates></LinearRing></outerBoundaryIs></Polygon><Polygon><outerBoundaryIs><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>9,4.111,0 1.049,1.024,0 9,4.111,0</coordinates></LineString></outerBoundaryIs></Polygon></MultiGeometry>" );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );


  // insert geometry
  QgsMultiPolygon rc;
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
  QVERIFY( !QgsMultiPolygon().cast( nullptr ) );
  QgsMultiPolygon pCast;
  QVERIFY( QgsMultiPolygon().cast( &pCast ) );
  QgsMultiPolygon pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiPolygonZ()" ) );
  QVERIFY( QgsMultiPolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiPolygonM()" ) );
  QVERIFY( QgsMultiPolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiPolygonZM()" ) );
  QVERIFY( QgsMultiPolygon().cast( &pCast2 ) );


  //boundary
  QgsMultiPolygon multiPolygon1;
  QVERIFY( !multiPolygon1.boundary() );

  QgsLineString ring1;
  ring1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygon polygon1;
  polygon1.setExteriorRing( ring1.clone() );
  multiPolygon1.addGeometry( polygon1.clone() );

  std::unique_ptr< QgsAbstractGeometry > boundary( multiPolygon1.boundary() );
  QgsMultiLineString *lineBoundary = dynamic_cast< QgsMultiLineString * >( boundary.get() );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numGeometries(), 1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 2 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 3 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 2 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.0 );

  // add polygon with interior rings
  QgsLineString ring2;
  ring2.setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 )  << QgsPoint( 10, 10 ) );
  QgsPolygon polygon2;
  polygon2.setExteriorRing( ring2.clone() );
  QgsLineString boundaryRing1;
  boundaryRing1.setPoints( QgsPointSequence() << QgsPoint( 10.1, 10.1 ) << QgsPoint( 10.2, 10.1 ) << QgsPoint( 10.2, 10.2 )  << QgsPoint( 10.1, 10.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QgsPointSequence() << QgsPoint( 10.8, 10.8 ) << QgsPoint( 10.9, 10.8 ) << QgsPoint( 10.9, 10.9 )  << QgsPoint( 10.8, 10.8 ) );
  polygon2.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  multiPolygon1.addGeometry( polygon2.clone() );

  boundary.reset( multiPolygon1.boundary() );
  QgsMultiLineString *multiLineBoundary( static_cast< QgsMultiLineString * >( boundary.get() ) );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 2 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 3 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 2 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 0 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 1 ), 11.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 2 ), 11.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 3 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 0 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 1 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 2 ), 11.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 3 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 0 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 1 ), 10.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 2 ), 10.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 3 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 0 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 1 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 2 ), 10.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 3 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 0 ), 10.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 1 ), 10.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 2 ), 10.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 3 ), 10.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 0 ), 10.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 1 ), 10.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 2 ), 10.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 3 ), 10.8 );

  // vertex iterator: 2 polygons (one with just exterior ring, other with two interior rings)
  QgsAbstractGeometry::vertex_iterator it = multiPolygon1.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = multiPolygon1.vertices_end();
  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 3 ) );
  ++it;
  // 2nd polygon - exterior ring
  QCOMPARE( *it, QgsPoint( 10, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 11 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 3 ) );
  ++it;
  // 2nd polygon - 1st interior ring
  QCOMPARE( *it, QgsPoint( 10.1, 10.1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 1, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.2, 10.1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 1, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.2, 10.2 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 1, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.1, 10.1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 1, 3 ) );
  ++it;
  // 2nd polygon - 2nd interior ring
  QCOMPARE( *it, QgsPoint( 10.8, 10.8 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 2, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.9, 10.8 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 2, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.9, 10.9 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 2, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.8, 10.8 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 2, 3 ) );
  ++it;
  // done!
  QCOMPARE( it, itEnd );

  // Java-style iterator
  QgsVertexIterator it2( &multiPolygon1 );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  // 2nd polygon - exterior ring
  QCOMPARE( it2.next(), QgsPoint( 10, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 11 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10, 10 ) );
  QVERIFY( it2.hasNext() );
  // 2nd polygon - 1st interior ring
  QCOMPARE( it2.next(), QgsPoint( 10.1, 10.1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.2, 10.1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.2, 10.2 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.1, 10.1 ) );
  QVERIFY( it2.hasNext() );
  // 2nd polygon - 2nd interior ring
  QCOMPARE( it2.next(), QgsPoint( 10.8, 10.8 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.9, 10.8 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.9, 10.9 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.8, 10.8 ) );
  QVERIFY( !it2.hasNext() );

  //removeDuplicateNodes
  QgsMultiPolygon dn1;
  QVERIFY( dn1.fromWkt( QStringLiteral( "MultiPolygonZ (((0 0 0, 10 10 0, 11 9 0, 9 8 0, 9 8 0, 1 -1 0, 0 0 0)),((7 -1 0, 12 7 0, 13 6 0, 13 6 0, 8 -3 0, 7 -1 0)))" ) ) );
  QCOMPARE( dn1.numGeometries(), 2 );
  // First call should remove all duplicate nodes (one per part)
  QVERIFY( dn1.removeDuplicateNodes( 0.001, false ) );
  QVERIFY( !dn1.removeDuplicateNodes( 0.001, false ) );
  QCOMPARE( dn1.asWkt(), QStringLiteral( "MultiPolygonZ (((0 0 0, 10 10 0, 11 9 0, 9 8 0, 1 -1 0, 0 0 0)),((7 -1 0, 12 7 0, 13 6 0, 8 -3 0, 7 -1 0)))" ) );

  // test centroid of empty multipolygon
  QgsMultiPolygon empty;
  QCOMPARE( empty.centroid().asWkt(), QStringLiteral( "Point EMPTY" ) );
}

QGSTEST_MAIN( TestQgsMultiPolygon )
#include "testqgsmultipolygon.moc"
