/***************************************************************************
     testqgscurvepolygon.cpp
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
#include <QPainter>
#include <QString>

#include "qgscircularstring.h"
#include "qgscurvepolygon.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"
#include "qgspoint.h"
#include "qgspolygon.h"

#include "testgeometryutils.h"

class TestQgsCurvePolygon: public QObject
{
    Q_OBJECT
  private slots:
    void curvePolygon();
};

void TestQgsCurvePolygon::curvePolygon()
{
  //test constructor
  QgsCurvePolygon p1;
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p1.wktTypeStr(), QString( "CurvePolygon" ) );
  QCOMPARE( p1.geometryType(), QString( "CurvePolygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( !p1.hasCurvedSegments() );
  QCOMPARE( p1.area(), 0.0 );
  QCOMPARE( p1.perimeter(), 0.0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //set exterior ring

  //try with no ring
  QgsCircularString *ext = nullptr;
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::CurvePolygon );

  // empty exterior ring
  ext = new QgsCircularString();
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 1 );
  QCOMPARE( p1.partCount(), 1 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::CurvePolygon );

  //valid exterior ring
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p1.setExteriorRing( ext );
  QVERIFY( !p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 5 );
  QCOMPARE( p1.ringCount(), 1 );
  QCOMPARE( p1.partCount(), 1 );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p1.wktTypeStr(), QString( "CurvePolygon" ) );
  QCOMPARE( p1.geometryType(), QString( "CurvePolygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( p1.hasCurvedSegments() );
  QGSCOMPARENEAR( p1.area(), 157.08, 0.01 );
  QGSCOMPARENEAR( p1.perimeter(), 44.4288, 0.01 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsCircularString * >( p1.exteriorRing() ) ), *ext );

  //initial setting of exterior ring should set z/m type
  QgsCurvePolygon p2;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p2.setExteriorRing( ext );
  QVERIFY( p2.is3D() );
  QVERIFY( !p2.isMeasure() );
  QCOMPARE( p2.wkbType(), QgsWkbTypes::CurvePolygonZ );
  QCOMPARE( p2.wktTypeStr(), QString( "CurvePolygonZ" ) );
  QCOMPARE( p2.geometryType(), QString( "CurvePolygon" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( p2.exteriorRing() ) ), *ext );
  QgsCurvePolygon p3;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p3.setExteriorRing( ext );
  QVERIFY( !p3.is3D() );
  QVERIFY( p3.isMeasure() );
  QCOMPARE( p3.wkbType(), QgsWkbTypes::CurvePolygonM );
  QCOMPARE( p3.wktTypeStr(), QString( "CurvePolygonM" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( p3.exteriorRing() ) ), *ext );
  QgsCurvePolygon p4;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 3, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 ) );
  p4.setExteriorRing( ext );
  QVERIFY( p4.is3D() );
  QVERIFY( p4.isMeasure() );
  QCOMPARE( p4.wkbType(), QgsWkbTypes::CurvePolygonZM );
  QCOMPARE( p4.wktTypeStr(), QString( "CurvePolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( p4.exteriorRing() ) ), *ext );

  //addInteriorRing
  QgsCurvePolygon p6;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p6.setExteriorRing( ext );
  //empty ring
  QCOMPARE( p6.numInteriorRings(), 0 );
  QVERIFY( !p6.interiorRing( -1 ) );
  QVERIFY( !p6.interiorRing( 0 ) );
  p6.addInteriorRing( nullptr );
  QCOMPARE( p6.numInteriorRings(), 0 );
  QgsCircularString *ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 1 );
  QCOMPARE( p6.interiorRing( 0 ), ring );
  QVERIFY( !p6.interiorRing( 1 ) );

  QgsCoordinateSequence seq = p6.coordinateSequence();
  QCOMPARE( seq, QgsCoordinateSequence() << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
            << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) )
            << ( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                 << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( p6.nCoordinates(), 10 );

  //try adding an interior ring with z to a 2d polygon, z should be dropped
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 2 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::CurvePolygon );
  QVERIFY( p6.interiorRing( 1 ) );
  QVERIFY( !p6.interiorRing( 1 )->is3D() );
  QVERIFY( !p6.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 1 )->wkbType(), QgsWkbTypes::CircularString );

  //try adding an interior ring with m to a 2d polygon, m should be dropped
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.2, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 3 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::CurvePolygon );
  QVERIFY( p6.interiorRing( 2 ) );
  QVERIFY( !p6.interiorRing( 2 )->is3D() );
  QVERIFY( !p6.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 2 )->wkbType(), QgsWkbTypes::CircularString );


  //addInteriorRing without z/m to PolygonZM
  QgsCurvePolygon p6b;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 ) );
  p6b.setExteriorRing( ext );
  QVERIFY( p6b.is3D() );
  QVERIFY( p6b.isMeasure() );
  QCOMPARE( p6b.wkbType(), QgsWkbTypes::CurvePolygonZM );
  //ring has no z
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 1, 9 ) << QgsPoint( QgsWkbTypes::PointM, 9, 9 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 0 ) );
  QVERIFY( p6b.interiorRing( 0 )->is3D() );
  QVERIFY( p6b.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 0 )->wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( p6b.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 0, 2 ) );
  //ring has no m
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 1 ) );
  QVERIFY( p6b.interiorRing( 1 )->is3D() );
  QVERIFY( p6b.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 1 )->wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( p6b.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.1, 1, 0 ) );

  //set interior rings
  QgsCurvePolygon p7;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7.setExteriorRing( ext );
  //add a list of rings with mixed types
  QVector< QgsCurve * > rings;
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.3, 0, 1 )
      << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.4, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.4, 0.4, 0, 3 )
      << QgsPoint( QgsWkbTypes::PointM, 0.4, 0.3, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.3, 0, 1 ) );
  //throw an empty ring in too
  rings << 0;
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[3] )->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
      << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 3 );
  QVERIFY( p7.interiorRing( 0 ) );
  QVERIFY( !p7.interiorRing( 0 )->is3D() );
  QVERIFY( !p7.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.1, 0.1 ) );
  QVERIFY( p7.interiorRing( 1 ) );
  QVERIFY( !p7.interiorRing( 1 )->is3D() );
  QVERIFY( !p7.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 1 )->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( p7.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.3, 0.3 ) );
  QVERIFY( p7.interiorRing( 2 ) );
  QVERIFY( !p7.interiorRing( 2 )->is3D() );
  QVERIFY( !p7.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 2 )->wkbType(), QgsWkbTypes::CircularString );

  //set rings with existing
  rings.clear();
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
      << QgsPoint( 0.8, 0.9 ) << QgsPoint( 0.9, 0.9 )
      << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.8, 0.8 ) );
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 1 );
  QVERIFY( p7.interiorRing( 0 ) );
  QVERIFY( !p7.interiorRing( 0 )->is3D() );
  QVERIFY( !p7.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.8, 0.8 ) );
  rings.clear();
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 0 );

  //change dimensionality of interior rings using setExteriorRing
  QgsCurvePolygon p7a;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p7a.setExteriorRing( ext );
  rings.clear();
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.3, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.4, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.4, 0.4, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.4, 0.3, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.3,  1 ) );
  p7a.setInteriorRings( rings );
  QVERIFY( p7a.is3D() );
  QVERIFY( !p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->is3D() );
  QVERIFY( !p7a.interiorRing( 0 )->isMeasure() );
  QVERIFY( p7a.interiorRing( 1 )->is3D() );
  QVERIFY( !p7a.interiorRing( 1 )->isMeasure() );
  //reset exterior ring to 2d
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                  << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( !p7a.is3D() );
  QVERIFY( !p7a.interiorRing( 0 )->is3D() ); //rings should also be made 2D
  QVERIFY( !p7a.interiorRing( 1 )->is3D() );
  //reset exterior ring to LineStringM
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) << QgsPoint( QgsWkbTypes::PointM, 0, 10 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 10 ) << QgsPoint( QgsWkbTypes::PointM, 10, 0 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->isMeasure() ); //rings should also gain measure
  QVERIFY( p7a.interiorRing( 1 )->isMeasure() );

  //removeInteriorRing
  QgsCurvePolygon p8;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p8.setExteriorRing( ext );
  QVERIFY( !p8.removeInteriorRing( -1 ) );
  QVERIFY( !p8.removeInteriorRing( 0 ) );
  rings.clear();
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 )
      << QgsPoint( 0.1, 0.2 ) << QgsPoint( 0.2, 0.2 )
      << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.1, 0.1 ) );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( 0.3, 0.3 )
      << QgsPoint( 0.3, 0.4 ) << QgsPoint( 0.4, 0.4 )
      << QgsPoint( 0.4, 0.3 ) << QgsPoint( 0.3, 0.3 ) );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[2] )->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
      << QgsPoint( 0.8, 0.9 ) << QgsPoint( 0.9, 0.9 )
      << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.8, 0.8 ) );
  p8.setInteriorRings( rings );
  QCOMPARE( p8.numInteriorRings(), 3 );
  QVERIFY( p8.removeInteriorRing( 0 ) );
  QCOMPARE( p8.numInteriorRings(), 2 );
  QCOMPARE( p8.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.3, 0.3 ) );
  QCOMPARE( p8.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.8, 0.8 ) );
  QVERIFY( p8.removeInteriorRing( 1 ) );
  QCOMPARE( p8.numInteriorRings(), 1 );
  QCOMPARE( p8.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.3, 0.3 ) );
  QVERIFY( p8.removeInteriorRing( 0 ) );
  QCOMPARE( p8.numInteriorRings(), 0 );
  QVERIFY( !p8.removeInteriorRing( 0 ) );

  //clear
  QgsCurvePolygon p9;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p9.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
  p9.addInteriorRing( ring );
  QCOMPARE( p9.numInteriorRings(), 1 );
  p9.clear();
  QVERIFY( p9.isEmpty() );
  QCOMPARE( p9.numInteriorRings(), 0 );
  QCOMPARE( p9.nCoordinates(), 0 );
  QCOMPARE( p9.ringCount(), 0 );
  QCOMPARE( p9.partCount(), 0 );
  QVERIFY( !p9.is3D() );
  QVERIFY( !p9.isMeasure() );
  QCOMPARE( p9.wkbType(), QgsWkbTypes::CurvePolygon );

  //equality operator
  QgsCurvePolygon p10;
  QgsCurvePolygon p10b;
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p10.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 ) << QgsPoint( 9, 9 )
                  << QgsPoint( 9, 0 ) << QgsPoint( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  p10b.setExteriorRing( p10.exteriorRing()->clone() );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                   << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  p10.addInteriorRing( ring );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );

  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 2, 1 )
                   << QgsPoint( 2, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 2, 1 ) );
  p10b.addInteriorRing( ring );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  p10b.removeInteriorRing( 0 );
  p10b.addInteriorRing( p10.interiorRing( 0 )->clone() );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );

  //clone

  QgsCurvePolygon p11;
  std::unique_ptr< QgsCurvePolygon >cloned( p11.clone() );
  QCOMPARE( p11, *cloned );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p11.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p11.addInteriorRing( ring );
  cloned.reset( p11.clone() );
  QCOMPARE( p11, *cloned );

  //copy constructor
  QgsCurvePolygon p12;
  QgsCurvePolygon p13( p12 );
  QCOMPARE( p12, p13 );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p12.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p12.addInteriorRing( ring );
  QgsCurvePolygon p14( p12 );
  QCOMPARE( p12, p14 );

  //assignment operator
  QgsCurvePolygon p15;
  p15 = p13;
  QCOMPARE( p13, p15 );
  p15 = p12;
  QCOMPARE( p12, p15 );

  // bounding box
  QgsCurvePolygon boundingBoxPoly;
  QgsRectangle bBox = boundingBoxPoly.boundingBox(); //no crash!

  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 10, 2 ) << QgsPoint( 0, 18, 3 )
                  << QgsPoint( -1, 4, 4 ) << QgsPoint( 0, 0, 1 ) );
  boundingBoxPoly.setExteriorRing( ext );
  bBox = boundingBoxPoly.boundingBox();
  QGSCOMPARENEAR( bBox.xMinimum(), -1.435273, 0.001 );
  QGSCOMPARENEAR( bBox.xMaximum(), 1.012344, 0.001 );
  QGSCOMPARENEAR( bBox.yMinimum(), 0.000000, 0.001 );
  QGSCOMPARENEAR( bBox.yMaximum(), 18, 0.001 );

  //surfaceToPolygon
  QgsCurvePolygon p12a;
  std::unique_ptr< QgsPolygon > surface( p12a.surfaceToPolygon() );
  QVERIFY( surface->isEmpty() );

  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 3 ) << QgsPoint( 2, 4 )
                  << QgsPoint( -1, 5 ) << QgsPoint( 0, 6 ) );
  p12a.setExteriorRing( ext );
  surface.reset( p12a.surfaceToPolygon() );
  QCOMPARE( surface->wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 290 );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 290 ); // nCoordinates is cached, so check twice
  QVERIFY( surface->exteriorRing()->isClosed() );
  // too many vertices to actually check the result, let's just make sure the bounding boxes are similar
  QgsRectangle r1 = ext->boundingBox();
  QgsRectangle r2 = surface->exteriorRing()->boundingBox();
  QGSCOMPARENEAR( r1.xMinimum(), r2.xMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.xMaximum(), r2.xMaximum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMinimum(), r2.yMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMaximum(), r2.yMaximum(), 0.0001 );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p12a.addInteriorRing( ring );
  surface.reset( p12a.surfaceToPolygon() );
  QCOMPARE( surface->wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 290 );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 290 ); // nCoordinates is cached, so check twice
  QVERIFY( surface->exteriorRing()->isClosed() );
  QCOMPARE( surface->numInteriorRings(), 1 );
  // too many vertices to actually check the result, let's just make sure the bounding boxes are similar
  r1 = ring->boundingBox();
  r2 = surface->interiorRing( 0 )->boundingBox();
  QGSCOMPARENEAR( r1.xMinimum(), r2.xMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.xMaximum(), r2.xMaximum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMinimum(), r2.yMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMaximum(), r2.yMaximum(), 0.0001 );

  //toPolygon
  p12a = QgsCurvePolygon();
  surface.reset( p12a.toPolygon() );
  QVERIFY( surface->isEmpty() );

  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 10 ) << QgsPoint( 0, 18 )
                  << QgsPoint( -1, 4 ) << QgsPoint( 0, 0 ) );
  p12a.setExteriorRing( ext );
  surface.reset( p12a.toPolygon() );
  QCOMPARE( surface->wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 64 );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 64 ); // ncoordinates is cached, so check twice
  QVERIFY( surface->exteriorRing()->isClosed() );
  // too many vertices to actually check the result, let's just make sure the bounding boxes are similar
  r1 = ext->boundingBox();
  r2 = surface->exteriorRing()->boundingBox();
  QGSCOMPARENEAR( r1.xMinimum(), r2.xMinimum(), 0.01 );
  QGSCOMPARENEAR( r1.xMaximum(), r2.xMaximum(), 0.01 );
  QGSCOMPARENEAR( r1.yMinimum(), r2.yMinimum(), 0.01 );
  QGSCOMPARENEAR( r1.yMaximum(), r2.yMaximum(), 0.01 );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p12a.addInteriorRing( ring );
  surface.reset( p12a.toPolygon() );
  QCOMPARE( surface->wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 64 );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 64 ); //ncoordinates is cached, so check twice
  QVERIFY( surface->exteriorRing()->isClosed() );
  QCOMPARE( surface->numInteriorRings(), 1 );
  // too many vertices to actually check the result, let's just make sure the bounding boxes are similar
  r1 = ring->boundingBox();
  r2 = surface->interiorRing( 0 )->boundingBox();
  QGSCOMPARENEAR( r1.xMinimum(), r2.xMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.xMaximum(), r2.xMaximum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMinimum(), r2.yMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMaximum(), r2.yMaximum(), 0.0001 );

  //toCurveType - should be identical since it's already a curve
  std::unique_ptr< QgsCurvePolygon > curveType( p12a.toCurveType() );
  QCOMPARE( *curveType, p12a );

  //to/fromWKB
  QgsCurvePolygon p16;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 2, 0 )
                  << QgsPoint( 1, 0.5 ) << QgsPoint( 0, 0 ) );
  p16.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.1, 0 ) << QgsPoint( 0.2, 0 )
                   << QgsPoint( 0.1, 0.05 ) << QgsPoint( 0, 0 ) );
  p16.addInteriorRing( ring );
  QByteArray wkb16 = p16.asWkb();
  QCOMPARE( wkb16.size(), p16.wkbSize() );
  QgsCurvePolygon p17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  p17.fromWkb( wkb16ptr );
  QCOMPARE( p16, p17 );
  //CurvePolygonZ
  p16.clear();
  p17.clear();

  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 )
                  << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0.1, 0, 2 ) << QgsPoint( 0.2, 0, 3 )
                   << QgsPoint( 0.1, 0.05, 4 ) << QgsPoint( 0, 0, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  p17.fromWkb( wkb16ptr2 );
  QCOMPARE( p16, p17 );

  // compound curve
  QgsCompoundCurve *cCurve = new QgsCompoundCurve();
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 )
                  << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  cCurve->addCurve( ext );
  p16.addInteriorRing( cCurve );
  wkb16 = p16.asWkb();
  QCOMPARE( wkb16.size(), p16.wkbSize() );
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  p17.fromWkb( wkb16ptr3 );
  QCOMPARE( p16, p17 );

  //CurvePolygonM
  p16.clear();
  p17.clear();
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 1, 0, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 2, 0, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 1, 0.5, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 0.1, 0, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.2, 0, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.05, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr8( wkb16 );
  p17.fromWkb( wkb16ptr8 );
  QCOMPARE( p16, p17 );

  //CurvePolygonZM
  p16.clear();
  p17.clear();
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 0, 12, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 0.5, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 0.2, 0, 12, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.05, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  p17.fromWkb( wkb16ptr4 );
  QCOMPARE( p16, p17 );

  //bad WKB - check for no crash
  p17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !p17.fromWkb( nullPtr ) );
  QCOMPARE( p17.wkbType(), QgsWkbTypes::CurvePolygon );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !p17.fromWkb( wkbPointPtr ) );
  QCOMPARE( p17.wkbType(), QgsWkbTypes::CurvePolygon );

  //to/from WKT
  QgsCurvePolygon p18;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 0, 12, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 0.5, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  p18.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 0.2, 0, 12, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.05, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  p18.addInteriorRing( ring );

  QString wkt = p18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsCurvePolygon p19;
  QVERIFY( p19.fromWkt( wkt ) );
  QCOMPARE( p18, p19 );

  //bad WKT
  QVERIFY( !p19.fromWkt( "Point()" ) );
  QVERIFY( p19.isEmpty() );
  QVERIFY( !p19.exteriorRing() );
  QCOMPARE( p19.numInteriorRings(), 0 );
  QVERIFY( !p19.is3D() );
  QVERIFY( !p19.isMeasure() );
  QCOMPARE( p19.wkbType(), QgsWkbTypes::CurvePolygon );


  //as JSON
  QgsCurvePolygon exportPolygon;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 0, 12, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 0.5, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  exportPolygon.setExteriorRing( ext );


  // GML document for compare
  QDomDocument doc( QStringLiteral( "gml" ) );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 1,0 2,0 2,0 2,0 2,0.1 1.9,0.1 1.9,0.1 1.9,0.1 1.9,0.1 1.9,0.1 1.9,0.1 1.9,0.2 1.8,0.2 1.8,0.2 1.8,0.2 1.8,0.2 1.8,0.2 1.8,0.2 1.7,0.3 1.7,0.3 1.7,0.3 1.7,0.3 1.7,0.3 1.6,0.3 1.6,0.3 1.6,0.3 1.6,0.4 1.6,0.4 1.6,0.4 1.5,0.4 1.5,0.4 1.5,0.4 1.5,0.4 1.5,0.4 1.4,0.4 1.4,0.4 1.4,0.4 1.4,0.4 1.4,0.4 1.3,0.5 1.3,0.5 1.3,0.5 1.3,0.5 1.2,0.5 1.2,0.5 1.2,0.5 1.2,0.5 1.2,0.5 1.1,0.5 1.1,0.5 1.1,0.5 1.1,0.5 1.1,0.5 1,0.5 1,0.5 1,0.5 1,0.5 0.9,0.5 0.9,0.5 0.9,0.5 0.9,0.5 0.9,0.5 0.8,0.5 0.8,0.5 0.8,0.5 0.8,0.5 0.8,0.5 0.7,0.5 0.7,0.5 0.7,0.5 0.7,0.5 0.6,0.4 0.6,0.4 0.6,0.4 0.6,0.4 0.6,0.4 0.5,0.4 0.5,0.4 0.5,0.4 0.5,0.4 0.5,0.4 0.4,0.4 0.4,0.4 0.4,0.4 0.4,0.3 0.4,0.3 0.4,0.3 0.3,0.3 0.3,0.3 0.3,0.3 0.3,0.3 0.3,0.3 0.2,0.2 0.2,0.2 0.2,0.2 0.2,0.2 0.2,0.2 0.2,0.2 0.1,0.2 0.1,0.1 0.1,0.1 0.1,0.1 0.1,0.1 0.1,0.1 0.1,0.1 0,0.1 0,0 0,0 0,0</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QString res = elemToString( exportPolygon.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCurvePolygon().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 1 0.5 13 0 0 10</posList></ArcString></segments></Curve></exterior></Polygon>" ) );
  res = elemToString( exportPolygon.asGml3( doc, 2 ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCurvePolygon().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[[0.0,0.0,10.0],[1.0,0.0,11.0],[2.0,0.0,12.0],[2.0,0.0,12.0],[2.0,0.0,12.0],[2.0,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.2],[1.9,0.2,12.2],[1.8,0.2,12.2],[1.8,0.2,12.2],[1.8,0.2,12.2],[1.8,0.2,12.3],[1.8,0.2,12.3],[1.8,0.2,12.3],[1.7,0.3,12.3],[1.7,0.3,12.3],[1.7,0.3,12.4],[1.7,0.3,12.4],[1.7,0.3,12.4],[1.6,0.3,12.4],[1.6,0.3,12.4],[1.6,0.3,12.4],[1.6,0.4,12.5],[1.6,0.4,12.5],[1.6,0.4,12.5],[1.5,0.4,12.5],[1.5,0.4,12.5],[1.5,0.4,12.6],[1.5,0.4,12.6],[1.5,0.4,12.6],[1.4,0.4,12.6],[1.4,0.4,12.6],[1.4,0.4,12.7],[1.4,0.4,12.7],[1.4,0.4,12.7],[1.3,0.5,12.7],[1.3,0.5,12.7],[1.3,0.5,12.7],[1.3,0.5,12.8],[1.2,0.5,12.8],[1.2,0.5,12.8],[1.2,0.5,12.8],[1.2,0.5,12.8],[1.2,0.5,12.9],[1.1,0.5,12.9],[1.1,0.5,12.9],[1.1,0.5,12.9],[1.1,0.5,12.9],[1.1,0.5,13.0],[1.0,0.5,13.0],[1.0,0.5,13.0],[1.0,0.5,13.0],[1.0,0.5,12.9],[0.9,0.5,12.9],[0.9,0.5,12.8],[0.9,0.5,12.7],[0.9,0.5,12.7],[0.9,0.5,12.6],[0.8,0.5,12.6],[0.8,0.5,12.5],[0.8,0.5,12.5],[0.8,0.5,12.4],[0.8,0.5,12.4],[0.7,0.5,12.3],[0.7,0.5,12.2],[0.7,0.5,12.2],[0.7,0.5,12.1],[0.6,0.4,12.1],[0.6,0.4,12.0],[0.6,0.4,12.0],[0.6,0.4,11.9],[0.6,0.4,11.9],[0.5,0.4,11.8],[0.5,0.4,11.7],[0.5,0.4,11.7],[0.5,0.4,11.6],[0.5,0.4,11.6],[0.4,0.4,11.5],[0.4,0.4,11.5],[0.4,0.4,11.4],[0.4,0.3,11.3],[0.4,0.3,11.3],[0.4,0.3,11.2],[0.3,0.3,11.2],[0.3,0.3,11.1],[0.3,0.3,11.1],[0.3,0.3,11.0],[0.3,0.3,11.0],[0.2,0.2,10.9],[0.2,0.2,10.8],[0.2,0.2,10.8],[0.2,0.2,10.7],[0.2,0.2,10.7],[0.2,0.2,10.6],[0.1,0.2,10.6],[0.1,0.1,10.5],[0.1,0.1,10.4],[0.1,0.1,10.4],[0.1,0.1,10.3],[0.1,0.1,10.3],[0.1,0.1,10.2],[0.0,0.1,10.2],[0.0,0.0,10.1],[0.0,0.0,10.1],[0.0,0.0,10.0]]],\"type\":\"Polygon\"}" );
  res = exportPolygon.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 0.2, 0, 12, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.05, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  exportPolygon.addInteriorRing( ring );

  QString expectedJson( "{\"coordinates\":[[[0.0,0.0,10.0],[1.0,0.0,11.0],[2.0,0.0,12.0],[2.0,0.0,12.0],[2.0,0.0,12.0],[2.0,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.1],[1.9,0.1,12.2],[1.9,0.2,12.2],[1.8,0.2,12.2],[1.8,0.2,12.2],[1.8,0.2,12.2],[1.8,0.2,12.3],[1.8,0.2,12.3],[1.8,0.2,12.3],[1.7,0.3,12.3],[1.7,0.3,12.3],[1.7,0.3,12.4],[1.7,0.3,12.4],[1.7,0.3,12.4],[1.6,0.3,12.4],[1.6,0.3,12.4],[1.6,0.3,12.4],[1.6,0.4,12.5],[1.6,0.4,12.5],[1.6,0.4,12.5],[1.5,0.4,12.5],[1.5,0.4,12.5],[1.5,0.4,12.6],[1.5,0.4,12.6],[1.5,0.4,12.6],[1.4,0.4,12.6],[1.4,0.4,12.6],[1.4,0.4,12.7],[1.4,0.4,12.7],[1.4,0.4,12.7],[1.3,0.5,12.7],[1.3,0.5,12.7],[1.3,0.5,12.7],[1.3,0.5,12.8],[1.2,0.5,12.8],[1.2,0.5,12.8],[1.2,0.5,12.8],[1.2,0.5,12.8],[1.2,0.5,12.9],[1.1,0.5,12.9],[1.1,0.5,12.9],[1.1,0.5,12.9],[1.1,0.5,12.9],[1.1,0.5,13.0],[1.0,0.5,13.0],[1.0,0.5,13.0],[1.0,0.5,13.0],[1.0,0.5,12.9],[0.9,0.5,12.9],[0.9,0.5,12.8],[0.9,0.5,12.7],[0.9,0.5,12.7],[0.9,0.5,12.6],[0.8,0.5,12.6],[0.8,0.5,12.5],[0.8,0.5,12.5],[0.8,0.5,12.4],[0.8,0.5,12.4],[0.7,0.5,12.3],[0.7,0.5,12.2],[0.7,0.5,12.2],[0.7,0.5,12.1],[0.6,0.4,12.1],[0.6,0.4,12.0],[0.6,0.4,12.0],[0.6,0.4,11.9],[0.6,0.4,11.9],[0.5,0.4,11.8],[0.5,0.4,11.7],[0.5,0.4,11.7],[0.5,0.4,11.6],[0.5,0.4,11.6],[0.4,0.4,11.5],[0.4,0.4,11.5],[0.4,0.4,11.4],[0.4,0.3,11.3],[0.4,0.3,11.3],[0.4,0.3,11.2],[0.3,0.3,11.2],[0.3,0.3,11.1],[0.3,0.3,11.1],[0.3,0.3,11.0],[0.3,0.3,11.0],[0.2,0.2,10.9],[0.2,0.2,10.8],[0.2,0.2,10.8],[0.2,0.2,10.7],[0.2,0.2,10.7],[0.2,0.2,10.6],[0.1,0.2,10.6],[0.1,0.1,10.5],[0.1,0.1,10.4],[0.1,0.1,10.4],[0.1,0.1,10.3],[0.1,0.1,10.3],[0.1,0.1,10.2],[0.0,0.1,10.2],[0.0,0.0,10.1],[0.0,0.0,10.1],[0.0,0.0,10.0]],[[0.0,0.0,10.0],[0.1,0.0,11.0],[0.2,0.0,12.0],[0.2,0.0,12.0],[0.2,0.0,12.0],[0.2,0.0,12.1],[0.2,0.0,12.1],[0.2,0.0,12.1],[0.2,0.0,12.1],[0.2,0.0,12.1],[0.2,0.0,12.1],[0.2,0.0,12.2],[0.2,0.0,12.2],[0.2,0.0,12.2],[0.2,0.0,12.2],[0.2,0.0,12.2],[0.2,0.0,12.3],[0.2,0.0,12.3],[0.2,0.0,12.3],[0.2,0.0,12.3],[0.2,0.0,12.3],[0.2,0.0,12.4],[0.2,0.0,12.4],[0.2,0.0,12.4],[0.2,0.0,12.4],[0.2,0.0,12.4],[0.2,0.0,12.4],[0.2,0.0,12.5],[0.2,0.0,12.5],[0.2,0.0,12.5],[0.2,0.0,12.5],[0.2,0.0,12.5],[0.1,0.0,12.6],[0.1,0.0,12.6],[0.1,0.0,12.6],[0.1,0.0,12.6],[0.1,0.0,12.6],[0.1,0.0,12.7],[0.1,0.0,12.7],[0.1,0.0,12.7],[0.1,0.0,12.7],[0.1,0.0,12.7],[0.1,0.0,12.7],[0.1,0.0,12.8],[0.1,0.0,12.8],[0.1,0.0,12.8],[0.1,0.0,12.8],[0.1,0.0,12.8],[0.1,0.0,12.9],[0.1,0.0,12.9],[0.1,0.0,12.9],[0.1,0.0,12.9],[0.1,0.0,12.9],[0.1,0.0,13.0],[0.1,0.0,13.0],[0.1,0.0,13.0],[0.1,0.0,13.0],[0.1,0.0,12.9],[0.1,0.0,12.9],[0.1,0.0,12.8],[0.1,0.0,12.7],[0.1,0.0,12.7],[0.1,0.0,12.6],[0.1,0.0,12.6],[0.1,0.0,12.5],[0.1,0.0,12.5],[0.1,0.0,12.4],[0.1,0.0,12.4],[0.1,0.0,12.3],[0.1,0.0,12.2],[0.1,0.0,12.2],[0.1,0.0,12.1],[0.1,0.0,12.1],[0.1,0.0,12.0],[0.1,0.0,12.0],[0.1,0.0,11.9],[0.1,0.0,11.9],[0.1,0.0,11.8],[0.1,0.0,11.7],[0.1,0.0,11.7],[0.0,0.0,11.6],[0.0,0.0,11.6],[0.0,0.0,11.5],[0.0,0.0,11.5],[0.0,0.0,11.4],[0.0,0.0,11.3],[0.0,0.0,11.3],[0.0,0.0,11.2],[0.0,0.0,11.2],[0.0,0.0,11.1],[0.0,0.0,11.1],[0.0,0.0,11.0],[0.0,0.0,11.0],[0.0,0.0,10.9],[0.0,0.0,10.8],[0.0,0.0,10.8],[0.0,0.0,10.7],[0.0,0.0,10.7],[0.0,0.0,10.6],[0.0,0.0,10.6],[0.0,0.0,10.5],[0.0,0.0,10.4],[0.0,0.0,10.4],[0.0,0.0,10.3],[0.0,0.0,10.3],[0.0,0.0,10.2],[0.0,0.0,10.2],[0.0,0.0,10.1],[0.0,0.0,10.1],[0.0,0.0,10.0]]],\"type\":\"Polygon\"}" );
  res = exportPolygon.asJson( 1 );
  QCOMPARE( res, expectedJson );

  //asKML
  QString expectedKml( QStringLiteral( "<Polygon><outerBoundaryIs><LinearRing><altitudeMode>absolute</altitudeMode><coordinates>0,0,10 1,0,11 2,0,12 2,0,12 2,0,12 2,0.1,12.1 1.9,0.1,12.1 1.9,0.1,12.1 1.9,0.1,12.1 1.9,0.1,12.1 1.9,0.1,12.1 1.9,0.1,12.2 1.9,0.2,12.2 1.8,0.2,12.2 1.8,0.2,12.2 1.8,0.2,12.2 1.8,0.2,12.3 1.8,0.2,12.3 1.8,0.2,12.3 1.7,0.3,12.3 1.7,0.3,12.3 1.7,0.3,12.4 1.7,0.3,12.4 1.7,0.3,12.4 1.6,0.3,12.4 1.6,0.3,12.4 1.6,0.3,12.4 1.6,0.4,12.5 1.6,0.4,12.5 1.6,0.4,12.5 1.5,0.4,12.5 1.5,0.4,12.5 1.5,0.4,12.6 1.5,0.4,12.6 1.5,0.4,12.6 1.4,0.4,12.6 1.4,0.4,12.6 1.4,0.4,12.7 1.4,0.4,12.7 1.4,0.4,12.7 1.3,0.5,12.7 1.3,0.5,12.7 1.3,0.5,12.7 1.3,0.5,12.8 1.2,0.5,12.8 1.2,0.5,12.8 1.2,0.5,12.8 1.2,0.5,12.8 1.2,0.5,12.9 1.1,0.5,12.9 1.1,0.5,12.9 1.1,0.5,12.9 1.1,0.5,12.9 1.1,0.5,13 1,0.5,13 1,0.5,13 1,0.5,13 1,0.5,12.9 0.9,0.5,12.9 0.9,0.5,12.8 0.9,0.5,12.7 0.9,0.5,12.7 0.9,0.5,12.6 0.8,0.5,12.6 0.8,0.5,12.5 0.8,0.5,12.5 0.8,0.5,12.4 0.8,0.5,12.4 0.7,0.5,12.3 0.7,0.5,12.2 0.7,0.5,12.2 0.7,0.5,12.1 0.6,0.4,12.1 0.6,0.4,12 0.6,0.4,12 0.6,0.4,11.9 0.6,0.4,11.9 0.5,0.4,11.8 0.5,0.4,11.7 0.5,0.4,11.7 0.5,0.4,11.6 0.5,0.4,11.6 0.4,0.4,11.5 0.4,0.4,11.5 0.4,0.4,11.4 0.4,0.3,11.3 0.4,0.3,11.3 0.4,0.3,11.2 0.3,0.3,11.2 0.3,0.3,11.1 0.3,0.3,11.1 0.3,0.3,11 0.3,0.3,11 0.2,0.2,10.9 0.2,0.2,10.8 0.2,0.2,10.8 0.2,0.2,10.7 0.2,0.2,10.7 0.2,0.2,10.6 0.1,0.2,10.6 0.1,0.1,10.5 0.1,0.1,10.4 0.1,0.1,10.4 0.1,0.1,10.3 0.1,0.1,10.3 0.1,0.1,10.2 0,0.1,10.2 0,0,10.1 0,0,10.1 0,0,10</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs><LinearRing><altitudeMode>absolute</altitudeMode><coordinates>0,0,10 0.1,0,11 0.2,0,12 0.2,0,12 0.2,0,12 0.2,0,12.1 0.2,0,12.1 0.2,0,12.1 0.2,0,12.1 0.2,0,12.1 0.2,0,12.1 0.2,0,12.2 0.2,0,12.2 0.2,0,12.2 0.2,0,12.2 0.2,0,12.2 0.2,0,12.3 0.2,0,12.3 0.2,0,12.3 0.2,0,12.3 0.2,0,12.3 0.2,0,12.4 0.2,0,12.4 0.2,0,12.4 0.2,0,12.4 0.2,0,12.4 0.2,0,12.4 0.2,0,12.5 0.2,0,12.5 0.2,0,12.5 0.2,0,12.5 0.2,0,12.5 0.1,0,12.6 0.1,0,12.6 0.1,0,12.6 0.1,0,12.6 0.1,0,12.6 0.1,0,12.7 0.1,0,12.7 0.1,0,12.7 0.1,0,12.7 0.1,0,12.7 0.1,0,12.7 0.1,0,12.8 0.1,0,12.8 0.1,0,12.8 0.1,0,12.8 0.1,0,12.8 0.1,0,12.9 0.1,0,12.9 0.1,0,12.9 0.1,0,12.9 0.1,0,12.9 0.1,0,13 0.1,0,13 0.1,0,13 0.1,0,13 0.1,0,12.9 0.1,0,12.9 0.1,0,12.8 0.1,0,12.7 0.1,0,12.7 0.1,0,12.6 0.1,0,12.6 0.1,0,12.5 0.1,0,12.5 0.1,0,12.4 0.1,0,12.4 0.1,0,12.3 0.1,0,12.2 0.1,0,12.2 0.1,0,12.1 0.1,0,12.1 0.1,0,12 0.1,0,12 0.1,0,11.9 0.1,0,11.9 0.1,0,11.8 0.1,0,11.7 0.1,0,11.7 0,0,11.6 0,0,11.6 0,0,11.5 0,0,11.5 0,0,11.4 0,0,11.3 0,0,11.3 0,0,11.2 0,0,11.2 0,0,11.1 0,0,11.1 0,0,11 0,0,11 0,0,10.9 0,0,10.8 0,0,10.8 0,0,10.7 0,0,10.7 0,0,10.6 0,0,10.6 0,0,10.5 0,0,10.4 0,0,10.4 0,0,10.3 0,0,10.3 0,0,10.2 0,0,10.2 0,0,10.1 0,0,10.1 0,0,10</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  QCOMPARE( exportPolygon.asKml( 1 ), expectedKml );


  QgsCurvePolygon exportPolygonFloat;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 1 / 3.0, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 2 / 3.0, 0, 12, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1 / 3.0, 0.5, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  exportPolygonFloat.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 0.1 / 3.0, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 0.2 / 3.0, 0, 12, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 0.1 / 3.0, 0.05 / 3.0, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  exportPolygonFloat.addInteriorRing( ring );

  QString expectedJsonPrec3( "{\"coordinates\":[[[0.0,0.0,10.0],[0.333,0.0,11.0],[0.667,0.0,12.0],[0.669,0.006,12.009],[0.671,0.012,12.018],[0.673,0.018,12.027],[0.676,0.024,12.035],[0.677,0.029,12.044],[0.679,0.035,12.053],[0.681,0.042,12.062],[0.683,0.048,12.071],[0.684,0.054,12.08],[0.686,0.06,12.088],[0.687,0.066,12.097],[0.688,0.072,12.106],[0.689,0.078,12.115],[0.69,0.084,12.124],[0.691,0.091,12.133],[0.692,0.097,12.142],[0.693,0.103,12.15],[0.693,0.109,12.159],[0.694,0.116,12.168],[0.694,0.122,12.177],[0.694,0.128,12.186],[0.694,0.135,12.195],[0.694,0.141,12.204],[0.694,0.147,12.212],[0.694,0.153,12.221],[0.694,0.16,12.23],[0.693,0.166,12.239],[0.693,0.172,12.248],[0.692,0.178,12.257],[0.692,0.185,12.265],[0.691,0.191,12.274],[0.69,0.197,12.283],[0.689,0.203,12.292],[0.687,0.209,12.301],[0.686,0.216,12.31],[0.685,0.222,12.319],[0.683,0.228,12.327],[0.682,0.234,12.336],[0.68,0.24,12.345],[0.678,0.246,12.354],[0.676,0.252,12.363],[0.674,0.258,12.372],[0.672,0.264,12.381],[0.67,0.27,12.389],[0.668,0.275,12.398],[0.665,0.281,12.407],[0.663,0.287,12.416],[0.66,0.293,12.425],[0.657,0.298,12.434],[0.654,0.304,12.442],[0.652,0.31,12.451],[0.649,0.315,12.46],[0.645,0.321,12.469],[0.642,0.326,12.478],[0.639,0.331,12.487],[0.636,0.337,12.496],[0.632,0.342,12.504],[0.628,0.347,12.513],[0.625,0.352,12.522],[0.621,0.357,12.531],[0.617,0.362,12.54],[0.613,0.367,12.549],[0.609,0.372,12.558],[0.605,0.377,12.566],[0.601,0.381,12.575],[0.597,0.386,12.584],[0.592,0.39,12.593],[0.588,0.395,12.602],[0.584,0.399,12.611],[0.579,0.404,12.619],[0.574,0.408,12.628],[0.57,0.412,12.637],[0.565,0.416,12.646],[0.56,0.42,12.655],[0.555,0.424,12.664],[0.55,0.428,12.673],[0.545,0.431,12.681],[0.54,0.435,12.69],[0.535,0.439,12.699],[0.529,0.442,12.708],[0.524,0.445,12.717],[0.519,0.449,12.726],[0.513,0.452,12.735],[0.508,0.455,12.743],[0.502,0.458,12.752],[0.497,0.461,12.761],[0.491,0.464,12.77],[0.485,0.466,12.779],[0.48,0.469,12.788],[0.474,0.471,12.796],[0.468,0.474,12.805],[0.462,0.476,12.814],[0.456,0.478,12.823],[0.451,0.48,12.832],[0.445,0.482,12.841],[0.439,0.484,12.85],[0.433,0.486,12.858],[0.426,0.488,12.867],[0.42,0.489,12.876],[0.414,0.491,12.885],[0.408,0.492,12.894],[0.402,0.493,12.903],[0.396,0.495,12.912],[0.39,0.496,12.92],[0.383,0.497,12.929],[0.377,0.497,12.938],[0.371,0.498,12.947],[0.365,0.499,12.956],[0.358,0.499,12.965],[0.352,0.5,12.973],[0.346,0.5,12.982],[0.34,0.5,12.991],[0.333,0.5,13.0],[0.327,0.5,12.973],[0.321,0.5,12.947],[0.314,0.5,12.92],[0.308,0.499,12.894],[0.302,0.499,12.867],[0.296,0.498,12.841],[0.289,0.497,12.814],[0.283,0.497,12.788],[0.277,0.496,12.761],[0.271,0.495,12.735],[0.265,0.493,12.708],[0.259,0.492,12.681],[0.252,0.491,12.655],[0.246,0.489,12.628],[0.24,0.488,12.602],[0.234,0.486,12.575],[0.228,0.484,12.549],[0.222,0.482,12.522],[0.216,0.48,12.496],[0.21,0.478,12.469],[0.204,0.476,12.442],[0.198,0.474,12.416],[0.193,0.471,12.389],[0.187,0.469,12.363],[0.181,0.466,12.336],[0.176,0.464,12.31],[0.17,0.461,12.283],[0.164,0.458,12.257],[0.159,0.455,12.23],[0.153,0.452,12.204],[0.148,0.449,12.177],[0.143,0.445,12.15],[0.137,0.442,12.124],[0.132,0.439,12.097],[0.127,0.435,12.071],[0.122,0.431,12.044],[0.117,0.428,12.018],[0.112,0.424,11.991],[0.107,0.42,11.965],[0.102,0.416,11.938],[0.097,0.412,11.912],[0.092,0.408,11.885],[0.088,0.404,11.858],[0.083,0.399,11.832],[0.079,0.395,11.805],[0.074,0.39,11.779],[0.07,0.386,11.752],[0.066,0.381,11.726],[0.061,0.377,11.699],[0.057,0.372,11.673],[0.053,0.367,11.646],[0.049,0.362,11.619],[0.046,0.357,11.593],[0.042,0.352,11.566],[0.038,0.347,11.54],[0.035,0.342,11.513],[0.031,0.337,11.487],[0.028,0.331,11.46],[0.024,0.326,11.434],[0.021,0.321,11.407],[0.018,0.315,11.381],[0.015,0.31,11.354],[0.012,0.304,11.327],[0.009,0.298,11.301],[0.007,0.293,11.274],[0.004,0.287,11.248],[0.001,0.281,11.221],[-0.001,0.275,11.195],[-0.003,0.27,11.168],[-0.005,0.264,11.142],[-0.008,0.258,11.115],[-0.01,0.252,11.088],[-0.012,0.246,11.062],[-0.013,0.24,11.035],[-0.015,0.234,11.009],[-0.017,0.228,10.982],[-0.018,0.222,10.956],[-0.02,0.216,10.929],[-0.021,0.209,10.903],[-0.022,0.203,10.876],[-0.023,0.197,10.85],[-0.024,0.191,10.823],[-0.025,0.185,10.796],[-0.026,0.178,10.77],[-0.026,0.172,10.743],[-0.027,0.166,10.717],[-0.027,0.16,10.69],[-0.027,0.153,10.664],[-0.028,0.147,10.637],[-0.028,0.141,10.611],[-0.028,0.135,10.584],[-0.028,0.128,10.558],[-0.027,0.122,10.531],[-0.027,0.116,10.504],[-0.027,0.109,10.478],[-0.026,0.103,10.451],[-0.025,0.097,10.425],[-0.025,0.091,10.398],[-0.024,0.084,10.372],[-0.023,0.078,10.345],[-0.022,0.072,10.319],[-0.02,0.066,10.292],[-0.019,0.06,10.265],[-0.018,0.054,10.239],[-0.016,0.048,10.212],[-0.014,0.042,10.186],[-0.013,0.035,10.159],[-0.011,0.029,10.133],[-0.009,0.024,10.106],[-0.007,0.018,10.08],[-0.005,0.012,10.053],[-0.002,0.006,10.027],[0.0,0.0,10.0]],[[0.0,0.0,10.0],[0.033,0.0,11.0],[0.067,0.0,12.0],[0.066,0.001,12.019],[0.066,0.001,12.037],[0.065,0.002,12.056],[0.065,0.002,12.075],[0.064,0.003,12.093],[0.064,0.003,12.112],[0.063,0.004,12.131],[0.063,0.004,12.15],[0.062,0.005,12.168],[0.062,0.005,12.187],[0.061,0.006,12.206],[0.061,0.006,12.224],[0.06,0.007,12.243],[0.06,0.007,12.262],[0.059,0.008,12.28],[0.059,0.008,12.299],[0.058,0.009,12.318],[0.057,0.009,12.336],[0.057,0.009,12.355],[0.056,0.01,12.374],[0.056,0.01,12.393],[0.055,0.011,12.411],[0.054,0.011,12.43],[0.054,0.011,12.449],[0.053,0.012,12.467],[0.052,0.012,12.486],[0.052,0.012,12.505],[0.051,0.013,12.523],[0.051,0.013,12.542],[0.05,0.013,12.561],[0.049,0.014,12.579],[0.049,0.014,12.598],[0.048,0.014,12.617],[0.047,0.014,12.636],[0.046,0.015,12.654],[0.046,0.015,12.673],[0.045,0.015,12.692],[0.044,0.015,12.71],[0.044,0.015,12.729],[0.043,0.016,12.748],[0.042,0.016,12.766],[0.042,0.016,12.785],[0.041,0.016,12.804],[0.04,0.016,12.822],[0.039,0.016,12.841],[0.039,0.016,12.86],[0.038,0.016,12.879],[0.037,0.016,12.897],[0.037,0.017,12.916],[0.036,0.017,12.935],[0.035,0.017,12.953],[0.034,0.017,12.972],[0.034,0.017,12.991],[0.033,0.017,12.972],[0.032,0.017,12.916],[0.032,0.017,12.86],[0.031,0.017,12.804],[0.03,0.017,12.748],[0.029,0.016,12.692],[0.029,0.016,12.636],[0.028,0.016,12.579],[0.027,0.016,12.523],[0.027,0.016,12.467],[0.026,0.016,12.411],[0.025,0.016,12.355],[0.024,0.016,12.299],[0.024,0.016,12.243],[0.023,0.015,12.187],[0.022,0.015,12.131],[0.022,0.015,12.075],[0.021,0.015,12.019],[0.02,0.015,11.963],[0.02,0.014,11.907],[0.019,0.014,11.85],[0.018,0.014,11.794],[0.017,0.014,11.738],[0.017,0.013,11.682],[0.016,0.013,11.626],[0.016,0.013,11.57],[0.015,0.012,11.514],[0.014,0.012,11.458],[0.014,0.012,11.402],[0.013,0.011,11.346],[0.012,0.011,11.29],[0.012,0.011,11.234],[0.011,0.01,11.178],[0.01,0.01,11.121],[0.01,0.009,11.065],[0.009,0.009,11.009],[0.009,0.009,10.953],[0.008,0.008,10.897],[0.008,0.008,10.841],[0.007,0.007,10.785],[0.006,0.007,10.729],[0.006,0.006,10.673],[0.005,0.006,10.617],[0.005,0.005,10.561],[0.004,0.005,10.505],[0.004,0.004,10.449],[0.003,0.004,10.393],[0.003,0.003,10.336],[0.002,0.003,10.28],[0.002,0.002,10.224],[0.001,0.002,10.168],[0.001,0.001,10.112],[0.0,0.001,10.056],[0.0,0.0,10.0]]],\"type\":\"Polygon\"}" );
  res = exportPolygonFloat.asJson( 3 );
  QCOMPARE( exportPolygonFloat.asJson( 3 ), expectedJsonPrec3 );

  // as GML2
  QString expectedGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 1,0 2,0 1.98685,0.01722 1.97341,0.03421 1.95967,0.05096 1.94564,0.06747 1.93133,0.08374 1.91674,0.09976 1.90188,0.11552 1.88674,0.13102 1.87134,0.14625 1.85567,0.16122 1.83975,0.17592 1.82358,0.19033 1.80716,0.20446 1.79049,0.21831 1.77359,0.23186 1.75646,0.24512 1.7391,0.25809 1.72151,0.27074 1.70371,0.2831 1.6857,0.29514 1.66748,0.30687 1.64907,0.31828 1.63045,0.32936 1.61165,0.34013 1.59267,0.35057 1.5735,0.36067 1.55417,0.37045 1.53466,0.37988 1.515,0.38898 1.49518,0.39773 1.47522,0.40614 1.45511,0.41421 1.43486,0.42192 1.41448,0.42928 1.39398,0.43629 1.37336,0.44294 1.35263,0.44923 1.33179,0.45516 1.31086,0.46073 1.28983,0.46594 1.26871,0.47078 1.24751,0.47525 1.22624,0.47936 1.2049,0.48309 1.18349,0.48646 1.16204,0.48945 1.14053,0.49208 1.11898,0.49432 1.0974,0.4962 1.07578,0.4977 1.05415,0.49883 1.0325,0.49958 1.01083,0.49995 0.98917,0.49995 0.9675,0.49958 0.94585,0.49883 0.92422,0.4977 0.9026,0.4962 0.88102,0.49432 0.85947,0.49208 0.83796,0.48945 0.81651,0.48646 0.7951,0.48309 0.77376,0.47936 0.75249,0.47525 0.73129,0.47078 0.71017,0.46594 0.68914,0.46073 0.66821,0.45516 0.64737,0.44923 0.62664,0.44294 0.60602,0.43629 0.58552,0.42928 0.56514,0.42192 0.54489,0.41421 0.52478,0.40614 0.50482,0.39773 0.485,0.38898 0.46534,0.37988 0.44583,0.37045 0.4265,0.36067 0.40733,0.35057 0.38835,0.34013 0.36955,0.32936 0.35093,0.31828 0.33252,0.30687 0.3143,0.29514 0.29629,0.2831 0.27849,0.27074 0.2609,0.25809 0.24354,0.24512 0.22641,0.23186 0.20951,0.21831 0.19284,0.20446 0.17642,0.19033 0.16025,0.17592 0.14433,0.16122 0.12866,0.14625 0.11326,0.13102 0.09812,0.11552 0.08326,0.09976 0.06867,0.08374 0.05436,0.06747 0.04033,0.05096 0.02659,0.03421 0.01315,0.01722 0,0</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0.1,0 0.2,0 0.19869,0.00172 0.19734,0.00342 0.19597,0.0051 0.19456,0.00675 0.19313,0.00837 0.19167,0.00998 0.19019,0.01155 0.18867,0.0131 0.18713,0.01463 0.18557,0.01612 0.18398,0.01759 0.18236,0.01903 0.18072,0.02045 0.17905,0.02183 0.17736,0.02319 0.17565,0.02451 0.17391,0.02581 0.17215,0.02707 0.17037,0.02831 0.16857,0.02951 0.16675,0.03069 0.16491,0.03183 0.16305,0.03294 0.16117,0.03401 0.15927,0.03506 0.15735,0.03607 0.15542,0.03704 0.15347,0.03799 0.1515,0.0389 0.14952,0.03977 0.14752,0.04061 0.14551,0.04142 0.14349,0.04219 0.14145,0.04293 0.1394,0.04363 0.13734,0.04429 0.13526,0.04492 0.13318,0.04552 0.13109,0.04607 0.12898,0.04659 0.12687,0.04708 0.12475,0.04753 0.12262,0.04794 0.12049,0.04831 0.11835,0.04865 0.1162,0.04895 0.11405,0.04921 0.1119,0.04943 0.10974,0.04962 0.10758,0.04977 0.10541,0.04988 0.10325,0.04996 0.10108,0.05 0.09892,0.05 0.09675,0.04996 0.09459,0.04988 0.09242,0.04977 0.09026,0.04962 0.0881,0.04943 0.08595,0.04921 0.0838,0.04895 0.08165,0.04865 0.07951,0.04831 0.07738,0.04794 0.07525,0.04753 0.07313,0.04708 0.07102,0.04659 0.06891,0.04607 0.06682,0.04552 0.06474,0.04492 0.06266,0.04429 0.0606,0.04363 0.05855,0.04293 0.05651,0.04219 0.05449,0.04142 0.05248,0.04061 0.05048,0.03977 0.0485,0.0389 0.04653,0.03799 0.04458,0.03704 0.04265,0.03607 0.04073,0.03506 0.03883,0.03401 0.03695,0.03294 0.03509,0.03183 0.03325,0.03069 0.03143,0.02951 0.02963,0.02831 0.02785,0.02707 0.02609,0.02581 0.02435,0.02451 0.02264,0.02319 0.02095,0.02183 0.01928,0.02045 0.01764,0.01903 0.01602,0.01759 0.01443,0.01612 0.01287,0.01463 0.01133,0.0131 0.00981,0.01155 0.00833,0.00998 0.00687,0.00837 0.00544,0.00675 0.00403,0.0051 0.00266,0.00342 0.00131,0.00172 0,0</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  res = elemToString( exportPolygon.asGml2( doc, 5 ) );
  QGSCOMPAREGML( res, expectedGML2 );

  QString expectedGML2prec2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 1,0 2,0 1.99,0.02 1.97,0.03 1.96,0.05 1.95,0.07 1.93,0.08 1.92,0.1 1.9,0.12 1.89,0.13 1.87,0.15 1.86,0.16 1.84,0.18 1.82,0.19 1.81,0.2 1.79,0.22 1.77,0.23 1.76,0.25 1.74,0.26 1.72,0.27 1.7,0.28 1.69,0.3 1.67,0.31 1.65,0.32 1.63,0.33 1.61,0.34 1.59,0.35 1.57,0.36 1.55,0.37 1.53,0.38 1.52,0.39 1.5,0.4 1.48,0.41 1.46,0.41 1.43,0.42 1.41,0.43 1.39,0.44 1.37,0.44 1.35,0.45 1.33,0.46 1.31,0.46 1.29,0.47 1.27,0.47 1.25,0.48 1.23,0.48 1.2,0.48 1.18,0.49 1.16,0.49 1.14,0.49 1.12,0.49 1.1,0.5 1.08,0.5 1.05,0.5 1.03,0.5 1.01,0.5 0.99,0.5 0.97,0.5 0.95,0.5 0.92,0.5 0.9,0.5 0.88,0.49 0.86,0.49 0.84,0.49 0.82,0.49 0.8,0.48 0.77,0.48 0.75,0.48 0.73,0.47 0.71,0.47 0.69,0.46 0.67,0.46 0.65,0.45 0.63,0.44 0.61,0.44 0.59,0.43 0.57,0.42 0.54,0.41 0.52,0.41 0.5,0.4 0.48,0.39 0.47,0.38 0.45,0.37 0.43,0.36 0.41,0.35 0.39,0.34 0.37,0.33 0.35,0.32 0.33,0.31 0.31,0.3 0.3,0.28 0.28,0.27 0.26,0.26 0.24,0.25 0.23,0.23 0.21,0.22 0.19,0.2 0.18,0.19 0.16,0.18 0.14,0.16 0.13,0.15 0.11,0.13 0.1,0.12 0.08,0.1 0.07,0.08 0.05,0.07 0.04,0.05 0.03,0.03 0.01,0.02 0,0</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0.1,0 0.2,0 0.2,0 0.2,0 0.2,0.01 0.19,0.01 0.19,0.01 0.19,0.01 0.19,0.01 0.19,0.01 0.19,0.01 0.19,0.02 0.18,0.02 0.18,0.02 0.18,0.02 0.18,0.02 0.18,0.02 0.18,0.02 0.17,0.03 0.17,0.03 0.17,0.03 0.17,0.03 0.17,0.03 0.16,0.03 0.16,0.03 0.16,0.03 0.16,0.04 0.16,0.04 0.16,0.04 0.15,0.04 0.15,0.04 0.15,0.04 0.15,0.04 0.15,0.04 0.14,0.04 0.14,0.04 0.14,0.04 0.14,0.04 0.14,0.04 0.13,0.05 0.13,0.05 0.13,0.05 0.13,0.05 0.12,0.05 0.12,0.05 0.12,0.05 0.12,0.05 0.12,0.05 0.11,0.05 0.11,0.05 0.11,0.05 0.11,0.05 0.11,0.05 0.1,0.05 0.1,0.05 0.1,0.05 0.1,0.05 0.09,0.05 0.09,0.05 0.09,0.05 0.09,0.05 0.09,0.05 0.08,0.05 0.08,0.05 0.08,0.05 0.08,0.05 0.08,0.05 0.07,0.05 0.07,0.05 0.07,0.05 0.07,0.05 0.06,0.04 0.06,0.04 0.06,0.04 0.06,0.04 0.06,0.04 0.05,0.04 0.05,0.04 0.05,0.04 0.05,0.04 0.05,0.04 0.04,0.04 0.04,0.04 0.04,0.04 0.04,0.03 0.04,0.03 0.04,0.03 0.03,0.03 0.03,0.03 0.03,0.03 0.03,0.03 0.03,0.03 0.02,0.02 0.02,0.02 0.02,0.02 0.02,0.02 0.02,0.02 0.02,0.02 0.01,0.02 0.01,0.01 0.01,0.01 0.01,0.01 0.01,0.01 0.01,0.01 0.01,0.01 0,0.01 0,0 0,0 0,0</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  res = elemToString( exportPolygon.asGml2( doc, 2 ) );
  QGSCOMPAREGML( res, expectedGML2prec2 );

  //as GML3
  QString expectedGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 1 0.5 13 0 0 10</posList></ArcString></segments></Curve></exterior><interior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 0.10000000000000001 0 11 0.20000000000000001 0 12 0.10000000000000001 0.05 13 0 0 10</posList></ArcString></segments></Curve></interior></Polygon>" ) );
  res = elemToString( exportPolygon.asGml3( doc ) );
  QCOMPARE( res, expectedGML3 );

  QString expectedGML3prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 1 0.5 13 0 0 10</posList></ArcString></segments></Curve></exterior><interior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 0.1 0 11 0.2 0 12 0.1 0.05 13 0 0 10</posList></ArcString></segments></Curve></interior></Polygon>" ) );
  res = elemToString( exportPolygon.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //removing the fourth to last vertex removes the whole ring
  QgsCurvePolygon p20;
  QgsCircularString *p20ExteriorRing = new QgsCircularString();
  p20ExteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  p20.setExteriorRing( p20ExteriorRing );
  QVERIFY( p20.exteriorRing() );
  p20.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QVERIFY( !p20.exteriorRing() );

  //boundary
  QgsCircularString boundary1;
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 )
                       << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  QgsCurvePolygon boundaryPolygon;
  QVERIFY( !boundaryPolygon.boundary() );

  boundaryPolygon.setExteriorRing( boundary1.clone() );
  QgsAbstractGeometry *boundary = boundaryPolygon.boundary();
  QgsCircularString *lineBoundary = dynamic_cast< QgsCircularString * >( boundary );
  QVERIFY( lineBoundary );
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

  // add interior rings
  QgsCircularString boundaryRing1;
  boundaryRing1.setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 ) );
  QgsCircularString boundaryRing2;
  boundaryRing2.setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 ) );
  boundaryPolygon.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  boundary = boundaryPolygon.boundary();
  QgsMultiCurve *multiLineBoundary = dynamic_cast< QgsMultiCurve * >( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 3 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->numPoints(), 5 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 2 ), 2.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 3 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 4 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 2 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.5 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 4 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->numPoints(), 3 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 0 ), 0.1 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 1 ), 0.2 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 2 ), 0.2 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 0 ), 0.1 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 1 ), 0.1 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 2 ), 0.2 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->numPoints(), 3 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 0 ), 0.8 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 1 ), 0.9 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 2 ), 0.9 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 0 ), 0.8 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 1 ), 0.8 );
  QCOMPARE( qgis::down_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 2 ), 0.9 );
  boundaryPolygon.setInteriorRings( QVector< QgsCurve * >() );
  delete boundary;

  //test boundary with z
  boundary1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                       << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundaryPolygon.setExteriorRing( boundary1.clone() );
  boundary = boundaryPolygon.boundary();
  lineBoundary = dynamic_cast< QgsCircularString * >( boundary );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 3 );
  QCOMPARE( lineBoundary->wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( lineBoundary->pointN( 0 ).z(), 10.0 );
  QCOMPARE( lineBoundary->pointN( 1 ).z(), 15.0 );
  QCOMPARE( lineBoundary->pointN( 2 ).z(), 20.0 );
  delete boundary;

  // remove interior rings
  QgsCircularString removeRingsExt;
  removeRingsExt.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsCurvePolygon removeRings1;
  removeRings1.removeInteriorRings();

  removeRings1.setExteriorRing( boundary1.clone() );
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // add interior rings
  QgsCircularString removeRingsRing1;
  removeRingsRing1.setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0.1, 1, 2 ) << QgsPoint( 0, 2, 3 )
                              << QgsPoint( -0.1, 1.2, 4 ) << QgsPoint( 0, 0, 1 ) );
  QgsCircularString removeRingsRing2;
  removeRingsRing2.setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0.01, 0.1, 2 ) << QgsPoint( 0, 0.2, 3 )
                              << QgsPoint( -0.01, 0.12, 4 ) << QgsPoint( 0, 0, 1 ) );
  removeRings1.setInteriorRings( QVector< QgsCurve * >() << removeRingsRing1.clone() << removeRingsRing2.clone() );

  // remove ring with size filter
  removeRings1.removeInteriorRings( 0.05 );
  QCOMPARE( removeRings1.numInteriorRings(), 1 );

  // remove ring with no size filter
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // cast
  QVERIFY( !QgsCurvePolygon().cast( nullptr ) );
  QgsCurvePolygon pCast;
  QVERIFY( QgsCurvePolygon().cast( &pCast ) );
  QgsCurvePolygon pCast2;
  pCast2.fromWkt( QStringLiteral( "CurvePolygonZ((0 0 0, 0 1 1, 1 0 2, 0 0 0))" ) );
  QVERIFY( QgsCurvePolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "CurvePolygonM((0 0 1, 0 1 2, 1 0 3, 0 0 1))" ) );
  QVERIFY( QgsCurvePolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "CurvePolygonZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1))" ) );
  QVERIFY( QgsCurvePolygon().cast( &pCast2 ) );

  // draw - most tests are in test_qgsgeometry.py
  QgsCurvePolygon empty;
  QPainter p;
  empty.draw( p ); //no crash!


  // closestSegment
  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;
  ( void )empty.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty curve, just want no crash

  QgsCurvePolygon cp12;
  QgsLineString cp12ls;
  cp12ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) << QgsPoint( 5, 10 ) );
  cp12.setExteriorRing( cp12ls.clone() );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( cp12.closestSegment( QgsPoint( 5, 15 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  // with interior ring
  cp12ls.setPoints( QgsPointSequence() << QgsPoint( 6, 11.5 ) << QgsPoint( 6.5, 12 ) << QgsPoint( 6, 13 ) << QgsPoint( 6, 11.5 ) );
  cp12.addInteriorRing( cp12ls.clone() );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 6, 11.4 ), pt, v, &leftOf ), 0.01, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.0, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.5, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( cp12.closestSegment( QgsPoint( 6, 13 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 6, 13 ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( leftOf, 0 );

  //nextVertex
  QgsCurvePolygon cp13;
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !cp13.nextVertex( v, pt ) );
  QgsLineString lp22;
  lp22.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  cp13.setExteriorRing( lp22.clone() );
  v = QgsVertexId( 0, 0, 4 ); //out of range
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 1, 12 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  // add interior ring
  lp22.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 11, 22 ) << QgsPoint( 11, 12 ) );
  cp13.addInteriorRing( lp22.clone() );
  v = QgsVertexId( 0, 1, 4 ); //out of range
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 1, -5 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 1, -1 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 0 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( pt, QgsPoint( 21, 22 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( pt, QgsPoint( 11, 22 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  v = QgsVertexId( 0, 2, 0 );
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 1, 0 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 1, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 21, 22 ) );

  // dropZValue
  QgsCurvePolygon p23;
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QgsLineString lp23;
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  p23.dropZValue(); // not z
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with z
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonZ );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonZM );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonM );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );

  // dropMValue
  p23.clear();
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  p23.dropMValue(); // not zm
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with m
  lp23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 13 ) << QgsPoint( QgsWkbTypes::PointM, 1, 12, 0, 23 ) << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonM );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonZM );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonZ );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );


  // hasCurvedSegments
  QgsCurvePolygon p24;
  QVERIFY( !p24.hasCurvedSegments() );
  QgsLineString lp24;
  lp24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p24.setExteriorRing( lp23.clone() );
  QVERIFY( !p24.hasCurvedSegments() );
  QgsCircularString cs24;
  cs24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p24.addInteriorRing( cs24.clone() );
  QVERIFY( p24.hasCurvedSegments() );

  //vertexAngle
  QgsCurvePolygon p25;
  ( void )p25.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )p25.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  ( void )p25.vertexAngle( QgsVertexId( 0, 1, 0 ) ); //just want no crash
  QgsLineString l38;
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p25.setExteriorRing( l38.clone() );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );
  p25.addInteriorRing( l38.clone() );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 6 ) ), 2.35619, 0.00001 );


  //insert vertex

  //insert vertex in empty polygon
  p25.clear();
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p25.isEmpty() );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p25.setExteriorRing( l38.clone() );
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 8 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 9 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  // last vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 10 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );
  // with interior ring
  p25.addInteriorRing( l38.clone() );
  QCOMPARE( p25.nCoordinates(), 17 );
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 18 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 2, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex in interior ring
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 19 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  // last vertex in interior ring
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 20 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );

  //move vertex

  //empty polygon
  QgsCurvePolygon p26;
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.isEmpty() );

  //valid polygon
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  p26.setExteriorRing( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  //out of range
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  // with interior ring
  p26.addInteriorRing( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 2, 0 ), QgsPoint( 3.0, 4.0 ) ) );

  //delete vertex

  //empty polygon
  QgsCurvePolygon p27;
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QVERIFY( p27.isEmpty() );

  //valid polygon
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 5, 2 ) << QgsPoint( 6, 2 ) << QgsPoint( 7, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );

  p27.setExteriorRing( l38.clone() );
  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove ring
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QVERIFY( !p27.exteriorRing() );

  // with interior ring
  p27.setExteriorRing( l38.clone() );
  p27.addInteriorRing( l38.clone() );

  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 2, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove ring
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );
  QCOMPARE( p27.numInteriorRings(), 0 );
  QVERIFY( p27.exteriorRing() );

  // test that interior ring is "promoted" when exterior is removed
  p27.addInteriorRing( l38.clone() );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 0 );
  QVERIFY( p27.exteriorRing() );
}

QGSTEST_MAIN( TestQgsCurvePolygon )
#include "testqgscurvepolygon.moc"
