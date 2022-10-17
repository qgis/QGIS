/***************************************************************************
     testqgspolygon.cpp
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
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgsproject.h"
#include "qgscoordinatetransform.h"
#include "testgeometryutils.h"
#include "testtransformer.h"

class TestQgsPolygon: public QObject
{
    Q_OBJECT
  private slots:
    void polygon();
};

void TestQgsPolygon::polygon()
{
  //test constructor
  QgsPolygon p1;
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p1.wktTypeStr(), QString( "Polygon" ) );
  QCOMPARE( p1.geometryType(), QString( "Polygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( !p1.hasCurvedSegments() );
  QCOMPARE( p1.area(), 0.0 );
  QCOMPARE( p1.perimeter(), 0.0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //set exterior ring

  //try with no ring
  QgsLineString *ext = nullptr;
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Polygon );

  // empty exterior ring
  ext = new QgsLineString();
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 1 );
  QCOMPARE( p1.partCount(), 1 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Polygon );

  //valid exterior ring
  ext = new QgsLineString();
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
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p1.wktTypeStr(), QString( "Polygon" ) );
  QCOMPARE( p1.geometryType(), QString( "Polygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( !p1.hasCurvedSegments() );
  QCOMPARE( p1.area(), 100.0 );
  QCOMPARE( p1.perimeter(), 40.0 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsLineString * >( p1.exteriorRing() ) ), *ext );

  //test that a non closed exterior ring will be automatically closed
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) );
  QVERIFY( !ext->isClosed() );
  p1.setExteriorRing( ext );
  QVERIFY( !p1.isEmpty() );
  QVERIFY( p1.exteriorRing()->isClosed() );
  QCOMPARE( p1.nCoordinates(), 5 );

  //initial setting of exterior ring should set z/m type
  QgsPolygon p2;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p2.setExteriorRing( ext );
  QVERIFY( p2.is3D() );
  QVERIFY( !p2.isMeasure() );
  QCOMPARE( p2.wkbType(), QgsWkbTypes::PolygonZ );
  QCOMPARE( p2.wktTypeStr(), QString( "PolygonZ" ) );
  QCOMPARE( p2.geometryType(), QString( "Polygon" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( p2.exteriorRing() ) ), *ext );
  QgsPolygon p3;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p3.setExteriorRing( ext );
  QVERIFY( !p3.is3D() );
  QVERIFY( p3.isMeasure() );
  QCOMPARE( p3.wkbType(), QgsWkbTypes::PolygonM );
  QCOMPARE( p3.wktTypeStr(), QString( "PolygonM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( p3.exteriorRing() ) ), *ext );
  QgsPolygon p4;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 3, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 ) );
  p4.setExteriorRing( ext );
  QVERIFY( p4.is3D() );
  QVERIFY( p4.isMeasure() );
  QCOMPARE( p4.wkbType(), QgsWkbTypes::PolygonZM );
  QCOMPARE( p4.wktTypeStr(), QString( "PolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( p4.exteriorRing() ) ), *ext );
  QgsPolygon p5;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  p5.setExteriorRing( ext );
  QVERIFY( p5.is3D() );
  QVERIFY( !p5.isMeasure() );
  QCOMPARE( p5.wkbType(), QgsWkbTypes::Polygon25D );
  QCOMPARE( p5.wktTypeStr(), QString( "PolygonZ" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( p5.exteriorRing() ) ), *ext );

  //setting curved exterior ring should be segmentized
  QgsCircularString *circularRing = new QgsCircularString();
  circularRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                           << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  QVERIFY( circularRing->hasCurvedSegments() );
  p5.setExteriorRing( circularRing );
  QVERIFY( !p5.exteriorRing()->hasCurvedSegments() );
  QCOMPARE( p5.exteriorRing()->wkbType(), QgsWkbTypes::LineString );

  //addInteriorRing
  QgsPolygon p6;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p6.setExteriorRing( ext );
  //empty ring
  QCOMPARE( p6.numInteriorRings(), 0 );
  QVERIFY( !p6.interiorRing( -1 ) );
  QVERIFY( !p6.interiorRing( 0 ) );
  p6.addInteriorRing( nullptr );
  QCOMPARE( p6.numInteriorRings(), 0 );
  QgsLineString *ring = new QgsLineString();
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

  //add non-closed interior ring, should be closed automatically
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.1, 0.9 ) << QgsPoint( 0.9, 0.9 )
                   << QgsPoint( 0.9, 0.1 ) );
  QVERIFY( !ring->isClosed() );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 2 );
  QVERIFY( p6.interiorRing( 1 )->isClosed() );

  //try adding an interior ring with z to a 2d polygon, z should be dropped
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 3 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::Polygon );
  QVERIFY( p6.interiorRing( 2 ) );
  QVERIFY( !p6.interiorRing( 2 )->is3D() );
  QVERIFY( !p6.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString );

  //try adding an interior ring with m to a 2d polygon, m should be dropped
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.2, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 4 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::Polygon );
  QVERIFY( p6.interiorRing( 3 ) );
  QVERIFY( !p6.interiorRing( 3 )->is3D() );
  QVERIFY( !p6.interiorRing( 3 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 3 )->wkbType(), QgsWkbTypes::LineString );

  //addInteriorRing without z/m to PolygonZM
  QgsPolygon p6b;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 ) );
  p6b.setExteriorRing( ext );
  QVERIFY( p6b.is3D() );
  QVERIFY( p6b.isMeasure() );
  QCOMPARE( p6b.wkbType(), QgsWkbTypes::PolygonZM );
  //ring has no z
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 1, 9 ) << QgsPoint( QgsWkbTypes::PointM, 9, 9 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 0 ) );
  QVERIFY( p6b.interiorRing( 0 )->is3D() );
  QVERIFY( p6b.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( p6b.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 0, 2 ) );
  //ring has no m
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 1 ) );
  QVERIFY( p6b.interiorRing( 1 )->is3D() );
  QVERIFY( p6b.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( p6b.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.1, 1, 0 ) );
  //test handling of 25D rings/polygons
  QgsPolygon p6c;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  p6c.setExteriorRing( ext );
  QVERIFY( p6c.is3D() );
  QVERIFY( !p6c.isMeasure() );
  QCOMPARE( p6c.wkbType(), QgsWkbTypes::Polygon25D );
  //adding a LineStringZ, should become LineString25D
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  QCOMPARE( ring->wkbType(), QgsWkbTypes::LineStringZ );
  p6c.addInteriorRing( ring );
  QVERIFY( p6c.interiorRing( 0 ) );
  QVERIFY( p6c.interiorRing( 0 )->is3D() );
  QVERIFY( !p6c.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( p6c.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point25D, 0.1, 0.1, 1 ) );
  //add a LineStringM, should become LineString25D
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.2, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 ) );
  QCOMPARE( ring->wkbType(), QgsWkbTypes::LineStringM );
  p6c.addInteriorRing( ring );
  QVERIFY( p6c.interiorRing( 1 ) );
  QVERIFY( p6c.interiorRing( 1 )->is3D() );
  QVERIFY( !p6c.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( p6c.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point25D, 0.1, 0.1 ) );

  //add curved ring to polygon
  circularRing = new QgsCircularString();
  circularRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                           << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  QVERIFY( circularRing->hasCurvedSegments() );
  p6c.addInteriorRing( circularRing );
  QVERIFY( p6c.interiorRing( 2 ) );
  QVERIFY( !p6c.interiorRing( 2 )->hasCurvedSegments() );
  QVERIFY( p6c.interiorRing( 2 )->is3D() );
  QVERIFY( !p6c.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString25D );

  // alternate constructor
  QgsPolygon pc6( new QgsLineString( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) ),
                  QList< QgsLineString *>() << new QgsLineString( QVector< QgsPoint >() << QgsPoint( 1, 1 ) << QgsPoint( 2, 1 ) << QgsPoint( 2, 2 ) << QgsPoint( 1, 2 ) << QgsPoint( 1, 1 ) )
                  << new QgsLineString( QVector< QgsPoint >() << QgsPoint( 3, 1 ) << QgsPoint( 4, 1 ) << QgsPoint( 4, 2 ) << QgsPoint( 3, 2 ) << QgsPoint( 3, 1 ) ) );
  QCOMPARE( pc6.asWkt(), QStringLiteral( "Polygon ((0 0, 0 10, 10 10, 10 0, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1),(3 1, 4 1, 4 2, 3 2, 3 1))" ) );

  //set interior rings
  QgsPolygon p7;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7.setExteriorRing( ext );
  //add a list of rings with mixed types
  QVector< QgsCurve * > rings;
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.3, 0, 1 )
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
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.1, 0.1 ) );
  QVERIFY( p7.interiorRing( 1 ) );
  QVERIFY( !p7.interiorRing( 1 )->is3D() );
  QVERIFY( !p7.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( p7.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.3, 0.3 ) );
  QVERIFY( p7.interiorRing( 2 ) );
  QVERIFY( !p7.interiorRing( 2 )->is3D() );
  QVERIFY( !p7.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString );

  //set rings with existing
  rings.clear();
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
      << QgsPoint( 0.8, 0.9 ) << QgsPoint( 0.9, 0.9 )
      << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.8, 0.8 ) );
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 1 );
  QVERIFY( p7.interiorRing( 0 ) );
  QVERIFY( !p7.interiorRing( 0 )->is3D() );
  QVERIFY( !p7.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.8, 0.8 ) );
  rings.clear();
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 0 );

  //change dimensionality of interior rings using setExteriorRing
  QgsPolygon p7a;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p7a.setExteriorRing( ext );
  rings.clear();
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.3, 1 )
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
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                  << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( !p7a.is3D() );
  QVERIFY( !p7a.interiorRing( 0 )->is3D() ); //rings should also be made 2D
  QVERIFY( !p7a.interiorRing( 1 )->is3D() );
  //reset exterior ring to LineStringM
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) << QgsPoint( QgsWkbTypes::PointM, 0, 10 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 10 ) << QgsPoint( QgsWkbTypes::PointM, 10, 0 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->isMeasure() ); //rings should also gain measure
  QVERIFY( p7a.interiorRing( 1 )->isMeasure() );
  //25D exterior ring
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 10 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 10 ) << QgsPoint( QgsWkbTypes::Point25D, 10, 0 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( p7a.is3D() );
  QVERIFY( !p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->is3D() ); //rings should also be made 25D
  QVERIFY( !p7a.interiorRing( 0 )->isMeasure() );
  QVERIFY( p7a.interiorRing( 1 )->is3D() );
  QVERIFY( !p7a.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p7a.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( p7a.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString25D );


  //removeInteriorRing
  QgsPolygon p8;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p8.setExteriorRing( ext );
  QVERIFY( !p8.removeInteriorRing( -1 ) );
  QVERIFY( !p8.removeInteriorRing( 0 ) );
  rings.clear();
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 )
      << QgsPoint( 0.1, 0.2 ) << QgsPoint( 0.2, 0.2 )
      << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.1, 0.1 ) );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( 0.3, 0.3 )
      << QgsPoint( 0.3, 0.4 ) << QgsPoint( 0.4, 0.4 )
      << QgsPoint( 0.4, 0.3 ) << QgsPoint( 0.3, 0.3 ) );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[2] )->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
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
  QgsPolygon p9;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p9.setExteriorRing( ext );
  ring = new QgsLineString();
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
  QCOMPARE( p9.wkbType(), QgsWkbTypes::Polygon );

  //equality operator
  QgsPolygon p10;
  QgsPolygon p10b;
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p10.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 ) << QgsPoint( 9, 9 )
                  << QgsPoint( 9, 0 ) << QgsPoint( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  p10b.setExteriorRing( p10.exteriorRing()->clone() );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                   << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  p10.addInteriorRing( ring );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );

  ring = new QgsLineString();
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

  QgsLineString nonPolygon;
  QVERIFY( p10 != nonPolygon );
  QVERIFY( !( p10 == nonPolygon ) );

  //clone

  QgsPolygon p11;
  std::unique_ptr< QgsPolygon >cloned( p11.clone() );
  QCOMPARE( p11, *cloned );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p11.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p11.addInteriorRing( ring );
  cloned.reset( p11.clone() );
  QCOMPARE( p11, *cloned );

  //copy constructor
  QgsPolygon p12;
  QgsPolygon p13( p12 );
  QCOMPARE( p12, p13 );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p12.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p12.addInteriorRing( ring );
  QgsPolygon p14( p12 );
  QCOMPARE( p12, p14 );

  //assignment operator
  QgsPolygon p15;
  p15 = p13;
  QCOMPARE( p13, p15 );
  p15 = p12;
  QCOMPARE( p12, p15 );

  //surfaceToPolygon - should be identical given polygon has no curves
  std::unique_ptr< QgsPolygon > surface( p12.surfaceToPolygon() );
  QCOMPARE( *surface, p12 );
  //toPolygon - should be identical given polygon has no curves
  std::unique_ptr< QgsPolygon > toP( p12.toPolygon() );
  QCOMPARE( *toP, p12 );

  //toCurveType
  std::unique_ptr< QgsCurvePolygon > curveType( QgsPolygon().toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CurvePolygon );

  curveType.reset( p12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CurvePolygonZM );
  QCOMPARE( curveType->exteriorRing()->numPoints(), 5 );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  QCOMPARE( curveType->numInteriorRings(), 1 );
  QCOMPARE( curveType->interiorRing( 0 )->numPoints(), 5 );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 ) );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 ) );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );

  //to/fromWKB
  QgsPolygon p16;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  p16.addInteriorRing( ring );
  QByteArray wkb16 = p16.asWkb();
  QCOMPARE( wkb16.size(), p16.wkbSize() );
  QgsPolygon p17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  p17.fromWkb( wkb16ptr );
  QCOMPARE( p16, p17 );
  //PolygonZ
  p16.clear();
  p17.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QCOMPARE( wkb16.size(), p16.wkbSize() );
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  p17.fromWkb( wkb16ptr2 );
  QCOMPARE( p16, p17 );
  //PolygonM
  p16.clear();
  p17.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10,  0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0,  0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 1, 9, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 9, 9, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  p17.fromWkb( wkb16ptr3 );
  QCOMPARE( p16, p17 );
  //PolygonZM
  p16.clear();
  p17.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  p17.fromWkb( wkb16ptr4 );
  QCOMPARE( p16, p17 );
  //Polygon25D
  p16.clear();
  p17.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point25D, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::Point25D, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::Point25D, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 1, 1, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  p17.clear();
  QgsConstWkbPtr wkb16ptr5( wkb16 );
  p17.fromWkb( wkb16ptr5 );
  QCOMPARE( p16, p17 );

  //bad WKB - check for no crash
  p17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !p17.fromWkb( nullPtr ) );
  QCOMPARE( p17.wkbType(), QgsWkbTypes::Polygon );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !p17.fromWkb( wkbPointPtr ) );
  QCOMPARE( p17.wkbType(), QgsWkbTypes::Polygon );

  //to/from WKT
  QgsPolygon p18;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p18.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p18.addInteriorRing( ring );

  QString wkt = p18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsPolygon p19;
  QVERIFY( p19.fromWkt( wkt ) );
  QCOMPARE( p18, p19 );

  //bad WKT
  QVERIFY( !p19.fromWkt( "Point()" ) );
  QVERIFY( p19.isEmpty() );
  QVERIFY( !p19.exteriorRing() );
  QCOMPARE( p19.numInteriorRings(), 0 );
  QVERIFY( !p19.is3D() );
  QVERIFY( !p19.isMeasure() );
  QCOMPARE( p19.wkbType(), QgsWkbTypes::Polygon );

  //as JSON
  QgsPolygon exportPolygon;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  exportPolygon.setExteriorRing( ext );

  // GML document for compare
  QDomDocument doc( QStringLiteral( "gml" ) );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QGSCOMPAREGML( elemToString( exportPolygon.asGml2( doc ) ), expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsPolygon().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LinearRing></exterior></Polygon>" ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsPolygon().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( QStringLiteral( "{\"coordinates\":[[[0.0,0.0],[0.0,10.0],[10.0,10.0],[10.0,0.0],[0.0,0.0]]],\"type\":\"Polygon\"}" ) );
  QCOMPARE( exportPolygon.asJson(), expectedSimpleJson );

  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  exportPolygon.addInteriorRing( ring );

  QString expectedJson( QStringLiteral( "{\"coordinates\":[[[0.0,0.0],[0.0,10.0],[10.0,10.0],[10.0,0.0],[0.0,0.0]],[[1.0,1.0],[1.0,9.0],[9.0,9.0],[9.0,1.0],[1.0,1.0]]],\"type\":\"Polygon\"}" ) );
  QCOMPARE( exportPolygon.asJson(), expectedJson );

  QgsPolygon exportPolygonFloat;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 100 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 100 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 10 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 ) );
  exportPolygonFloat.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 )
                   << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 4 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 4 / 3.0 )
                   << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 2 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 ) );
  exportPolygonFloat.addInteriorRing( ring );

  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[[[1.111,1.111],[1.111,11.111],[11.111,11.111],[11.111,1.111],[1.111,1.111]],[[0.667,0.667],[0.667,1.333],[1.333,1.333],[1.333,0.667],[0.667,0.667]]],\"type\":\"Polygon\"}" ) );
  QCOMPARE( exportPolygonFloat.asJson( 3 ), expectedJsonPrec3 );

  // as GML2
  QString expectedGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs>" ) );
  expectedGML2 += QLatin1String( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,1 1,9 9,9 9,1 1,1</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QGSCOMPAREGML( elemToString( exportPolygon.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,1.111 1.111,11.111 11.111,11.111 11.111,1.111 1.111,1.111</coordinates></LinearRing></outerBoundaryIs>" ) );
  expectedGML2prec3 += QLatin1String( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.667 0.667,1.333 1.333,1.333 1.333,0.667 0.667,0.667</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QGSCOMPAREGML( elemToString( exportPolygonFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );

  //as GML3
  QString expectedGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LinearRing></exterior>" ) );
  expectedGML3 += QLatin1String( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 1 1 9 9 9 9 1 1 1</posList></LinearRing></interior></Polygon>" );

  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1.111 1.111 1.111 11.111 11.111 11.111 11.111 1.111 1.111 1.111</posList></LinearRing></exterior>" ) );
  expectedGML3prec3 += QLatin1String( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.667 0.667 0.667 1.333 1.333 1.333 1.333 0.667 0.667 0.667</posList></LinearRing></interior></Polygon>" );
  QCOMPARE( elemToString( exportPolygonFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0,0,0 0,10,0 10,10,0 10,0,0 0,0,0</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>1,1,0 1,9,0 9,9,0 9,1,0 1,1,0</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  QCOMPARE( exportPolygon.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>1.111,1.111,0 1.111,11.111,0 11.111,11.111,0 11.111,1.111,0 1.111,1.111,0</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0.667,0.667,0 0.667,1.333,0 1.333,1.333,0 1.333,0.667,0 0.667,0.667,0</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  QCOMPARE( exportPolygonFloat.asKml( 3 ), expectedKmlPrec3 );

  //removing the fourth to last vertex removes the whole ring
  QgsPolygon p20;
  QgsLineString *p20ExteriorRing = new QgsLineString();
  p20ExteriorRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  p20.setExteriorRing( p20ExteriorRing );
  QVERIFY( p20.exteriorRing() );
  p20.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QVERIFY( !p20.exteriorRing() );

  //boundary
  QgsLineString boundary1;
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygon boundaryPolygon;
  QVERIFY( !boundaryPolygon.boundary() );

  boundaryPolygon.setExteriorRing( boundary1.clone() );
  QgsAbstractGeometry *boundary = boundaryPolygon.boundary();
  QgsLineString *lineBoundary = dynamic_cast< QgsLineString * >( boundary );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->xAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->xAt( 1 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 2 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 3 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 1 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 2 ), 1.0 );
  QCOMPARE( lineBoundary->yAt( 3 ), 0.0 );
  delete boundary;

  // add interior rings
  QgsLineString boundaryRing1;
  boundaryRing1.setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 )  << QgsPoint( 0.1, 0.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 )  << QgsPoint( 0.8, 0.8 ) );
  boundaryPolygon.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  boundary = boundaryPolygon.boundary();
  QgsMultiLineString *multiLineBoundary = dynamic_cast< QgsMultiLineString * >( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 3 );
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
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 0 ), 0.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 1 ), 0.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 2 ), 0.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 3 ), 0.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 0 ), 0.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 1 ), 0.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 2 ), 0.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 3 ), 0.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 0 ), 0.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 1 ), 0.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 2 ), 0.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 3 ), 0.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 0 ), 0.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 1 ), 0.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 2 ), 0.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 3 ), 0.8 );
  boundaryPolygon.setInteriorRings( QVector< QgsCurve * >() );
  delete boundary;

  //test boundary with z
  boundary1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                       << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 )  << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) );
  boundaryPolygon.setExteriorRing( boundary1.clone() );
  boundary = boundaryPolygon.boundary();
  lineBoundary = qgis::down_cast< QgsLineString * >( boundary );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( lineBoundary->zAt( 0 ), 10.0 );
  QCOMPARE( lineBoundary->zAt( 1 ), 15.0 );
  QCOMPARE( lineBoundary->zAt( 2 ), 20.0 );
  QCOMPARE( lineBoundary->zAt( 3 ), 10.0 );
  delete boundary;

  // point distance to boundary

  QgsLineString pd1;
  pd1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QgsPolygon pd;
  // no meaning, but let's not crash
  ( void )pd.pointDistanceToBoundary( 0, 0 );

  pd.setExteriorRing( pd1.clone() );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.1, 0.5 ), 0.1, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( -0.1, 0.5 ), -0.1, 0.0000000001 );
  // with a ring
  QgsLineString pdRing1;
  pdRing1.setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.6 )  << QgsPoint( 0.1, 0.6 ) << QgsPoint( 0.1, 0.1 ) );
  pd.setInteriorRings( QVector< QgsCurve * >() << pdRing1.clone() );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.1, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.01, 0.5 ), 0.01, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.08, 0.5 ), 0.02, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.12, 0.5 ), -0.02, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( -0.1, 0.5 ), -0.1, 0.0000000001 );

  // remove interior rings
  QgsLineString removeRingsExt;
  removeRingsExt.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygon removeRings1;
  removeRings1.removeInteriorRings();

  removeRings1.setExteriorRing( boundary1.clone() );
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // add interior rings
  QgsLineString removeRingsRing1;
  removeRingsRing1.setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 )  << QgsPoint( 0.1, 0.1 ) );
  QgsLineString removeRingsRing2;
  removeRingsRing2.setPoints( QgsPointSequence() << QgsPoint( 0.6, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 )  << QgsPoint( 0.6, 0.8 ) );
  removeRings1.setInteriorRings( QVector< QgsCurve * >() << removeRingsRing1.clone() << removeRingsRing2.clone() );

  // remove ring with size filter
  removeRings1.removeInteriorRings( 0.0075 );
  QCOMPARE( removeRings1.numInteriorRings(), 1 );

  // remove ring with no size filter
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // cast
  QVERIFY( !QgsPolygon().cast( nullptr ) );
  QgsPolygon pCast;
  QVERIFY( QgsPolygon().cast( &pCast ) );
  QgsPolygon pCast2;
  pCast2.fromWkt( QStringLiteral( "PolygonZ((0 0 0, 0 1 1, 1 0 2, 0 0 0))" ) );
  QVERIFY( QgsPolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "PolygonM((0 0 1, 0 1 2, 1 0 3, 0 0 1))" ) );
  QVERIFY( QgsPolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "PolygonZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1))" ) );
  QVERIFY( QgsPolygon().cast( &pCast2 ) );

  //transform
  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsPolygon pTransform;
  QgsLineString l21;
  l21.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                 << QgsPoint( 6274985, -3526584 )
                 << QgsPoint( 6474985, -3526584 )
                 << QgsPoint( 6374985, -3626584 ) );
  pTransform.setExteriorRing( l21.clone() );
  pTransform.addInteriorRing( l21.clone() );
  pTransform.transform( tr, Qgis::TransformDirection::Forward );
  const QgsLineString *extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMaximum(), -38.7999, 0.001 );
  const QgsLineString *intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  QgsLineString l22;
  l22.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6274985, -3526584, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 5, 6 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 ) );
  pTransform.clear();
  pTransform.setExteriorRing( l22.clone() );
  pTransform.addInteriorRing( l22.clone() );
  pTransform.transform( tr, Qgis::TransformDirection::Forward );
  extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMaximum(), -38.7999, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().yMaximum(), -38.7999, 0.001 );

  //reverse transform
  pTransform.transform( tr, Qgis::TransformDirection::Reverse );
  extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 6374984, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(), 6274984, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  6474984, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  6374984, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMaximum(), -3526584, 100 );
  intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 6374984, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(), 6274984, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  6474984, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  6374984, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), -3526584, 100 );

#if PROJ_VERSION_MAJOR<6 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //z value transform
  pTransform.transform( tr, Qgis::TransformDirection::Forward, true );
  extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), -19.249066, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), -19.249066, 0.001 );
  pTransform.transform( tr, Qgis::TransformDirection::Reverse, true );
  extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1, 0.001 );
#endif

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsLineString l23;
  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 12, 23, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QgsPolygon pTransform2;
  pTransform2.setExteriorRing( l23.clone() );
  pTransform2.addInteriorRing( l23.clone() );
  pTransform2.transform( qtr, 2, 3, 4, 5 );

  extR = static_cast< const QgsLineString * >( pTransform2.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 41.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 74.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 71.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 124.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( pTransform2.exteriorRing()->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( pTransform2.exteriorRing()->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( pTransform2.exteriorRing()->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( pTransform2.exteriorRing()->boundingBox().yMaximum(), 36, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform2.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 41.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 74.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 71.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 124.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), 36, 0.001 );

  // closestSegment
  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;
  QgsPolygon empty;
  ( void )empty.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty polygon, just want no crash

  QgsPolygon p21;
  QgsLineString p21ls;
  p21ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) << QgsPoint( 5, 10 ) );
  p21.setExteriorRing( p21ls.clone() );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( p21.closestSegment( QgsPoint( 5, 15 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  // with interior ring
  p21ls.setPoints( QgsPointSequence() << QgsPoint( 6, 11.5 ) << QgsPoint( 6.5, 12 ) << QgsPoint( 6, 13 ) << QgsPoint( 6, 11.5 ) );
  p21.addInteriorRing( p21ls.clone() );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 6, 11.4 ), pt, v, &leftOf ), 0.01, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.0, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.5, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( p21.closestSegment( QgsPoint( 6, 13 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 6, 13 ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( leftOf, 0 );

  //nextVertex
  QgsPolygon p22;
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  QgsLineString lp22;
  lp22.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p22.setExteriorRing( lp22.clone() );
  v = QgsVertexId( 0, 0, 4 ); //out of range
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 1, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  // add interior ring
  lp22.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 11, 22 ) << QgsPoint( 11, 12 ) );
  p22.addInteriorRing( lp22.clone() );
  v = QgsVertexId( 0, 1, 4 ); //out of range
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 1, -5 );
  QVERIFY( p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 1, -1 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 0 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( pt, QgsPoint( 21, 22 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( pt, QgsPoint( 11, 22 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  v = QgsVertexId( 0, 2, 0 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 1, 0 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 1, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 21, 22 ) );

  // dropZValue
  QgsPolygon p23;
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QgsLineString lp23;
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  p23.dropZValue(); // not z
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with z
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonZ );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonZM );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonM );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );

  // dropMValue
  p23.clear();
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  p23.dropMValue(); // not zm
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with m
  lp23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 13 ) << QgsPoint( QgsWkbTypes::PointM, 1, 12, 0, 23 ) << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonM );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonZM );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonZ );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );

  //vertexAngle
  QgsPolygon p24;
  ( void )p24.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 0, 1, 0 ) ); //just want no crash
  QgsLineString l38;
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p24.setExteriorRing( l38.clone() );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );
  p24.addInteriorRing( l38.clone() );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 6 ) ), 2.35619, 0.00001 );

  //insert vertex

  //insert vertex in empty polygon
  QgsPolygon p25;
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
  QgsPolygon p26;
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
  QgsPolygon p27;
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

  // test adjacent vertices - should wrap around!
  QgsLineString *closedRing1 = new QgsLineString();
  closedRing1->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  QgsPolygon p28;
  QgsVertexId previous( 1, 2, 3 );
  QgsVertexId next( 4, 5, 6 );
  p28.adjacentVertices( QgsVertexId( 0, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );

  p28.setExteriorRing( closedRing1 );
  p28.adjacentVertices( QgsVertexId( 0, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 2 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 3 ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 3 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 4 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );
  // part number should be retained
  p28.adjacentVertices( QgsVertexId( 1, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 1, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 1, 0, 1 ) );

  // interior ring
  p28.addInteriorRing( closedRing1->clone() );
  p28.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 1 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 2 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 2 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 3 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 3 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 4 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 4 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 1 ) );
  p28.adjacentVertices( QgsVertexId( 0, 2, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );

  // vertex number
  QgsLineString vertexLine2;
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );

  QgsLineString *closedRing2 = new QgsLineString();
  closedRing2->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  QgsPolygon p29;
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  p29.setExteriorRing( closedRing2 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), 4 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 5 ) ), -1 );
  p29.addInteriorRing( closedRing2->clone() );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), 4 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 5 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), 5 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 1 ) ), 6 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 2 ) ), 7 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 3 ) ), 8 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 4 ) ), 9 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 5 ) ), -1 );


  //segmentLength
  QgsPolygon p30;
  QgsLineString vertexLine3;
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 )
                         << QgsPoint( 111, 2 ) << QgsPoint( 11, 2 ) );
  p30.setExteriorRing( vertexLine3.clone() );
  QCOMPARE( p30.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 1, 1 ) ), 0.0 );

  // add interior ring
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 30, 6 ) << QgsPoint( 34, 6 ) << QgsPoint( 34, 8 )
                         << QgsPoint( 30, 8 ) << QgsPoint( 30, 6 ) );
  p30.addInteriorRing( vertexLine3.clone() );
  QCOMPARE( p30.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 0 ) ), 4.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 1 ) ), 2.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 2 ) ), 4.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 3 ) ), 2.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 4 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 1, 1 ) ), 2.0 );

  //removeDuplicateNodes
  QgsPolygon nodePolygon;
  QgsLineString nodeLine;
  QVERIFY( !nodePolygon.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 22 ) << QgsPoint( 11, 2 ) );
  nodePolygon.setExteriorRing( nodeLine.clone() );
  QVERIFY( !nodePolygon.removeDuplicateNodes() );
  QCOMPARE( nodePolygon.asWkt(), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 11, 22 ) << QgsPoint( 11.01, 21.99 ) << QgsPoint( 10.99, 1.99 ) << QgsPoint( 11, 2 ) );
  nodePolygon.setExteriorRing( nodeLine.clone() );
  QVERIFY( nodePolygon.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !nodePolygon.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodePolygon.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 11, 22 ) << QgsPoint( 11.01, 21.99 ) << QgsPoint( 10.99, 1.99 ) << QgsPoint( 11, 2 ) );
  nodePolygon.setExteriorRing( nodeLine.clone() );
  QVERIFY( !nodePolygon.removeDuplicateNodes() );
  QCOMPARE( nodePolygon.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11.01 1.99, 11.02 2.01, 11 12, 11 22, 11.01 21.99, 10.99 1.99, 11 2))" ) );
  // don't create degenerate rings
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 2.01 ) << QgsPoint( 11, 2.01 ) << QgsPoint( 11, 2 ) );
  nodePolygon.addInteriorRing( nodeLine.clone() );
  QVERIFY( nodePolygon.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodePolygon.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2),(11 2, 11.01 2.01, 11 2.01, 11 2))" ) );

  // swap XY
  QgsPolygon swapPolygon;
  swapPolygon.swapXy(); //no crash
  QgsLineString swapLine;
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  swapPolygon.setExteriorRing( swapLine.clone() );
  swapPolygon.swapXy();
  QCOMPARE( swapPolygon.asWkt(), QStringLiteral( "PolygonZM ((2 11 3 4, 12 11 13 14, 22 11 23 24, 2 11 3 4))" ) );
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) );
  swapPolygon.addInteriorRing( swapLine.clone() );
  swapPolygon.swapXy();
  QCOMPARE( swapPolygon.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4),(2 1 5 6, 2.01 11.01 15 16, 2.01 11 25 26, 2 11 5 6, 2 1 5 6))" ) );

  // filter vertex
  QgsPolygon filterPolygon;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() > 5;
  };
  filterPolygon.filterVertices( filter ); // no crash
  QgsLineString filterPolygonRing;
  filterPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  filterPolygon.setExteriorRing( filterPolygonRing.clone() );
  filterPolygon.filterVertices( filter );
  QCOMPARE( filterPolygon.asWkt(), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4))" ) );
  filterPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  filterPolygon.addInteriorRing( filterPolygonRing.clone() );
  filterPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  filterPolygon.addInteriorRing( filterPolygonRing.clone() );
  filterPolygon.filterVertices( filter );
  QCOMPARE( filterPolygon.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4),(10 2 5 6, 11.01 2.01 15 16, 11 2.01 25 26, 11 2 5 6, 10 2 5 6),(11.01 2.01 15 16, 11 2.01 25 26))" ) );

  // transform vertex
  QgsPolygon transformPolygon;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 5 );
  };
  transformPolygon.transformVertices( transform ); // no crash
  QgsLineString transformPolygonRing;
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  transformPolygon.setExteriorRing( transformPolygonRing.clone() );
  transformPolygon.transformVertices( transform );
  QCOMPARE( transformPolygon.asWkt(), QStringLiteral( "PolygonZM ((13 5 7 9, 6 15 17 19, 13 15 17 19, 13 25 27 29, 13 5 7 9))" ) );
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  transformPolygon.addInteriorRing( transformPolygonRing.clone() );
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  transformPolygon.addInteriorRing( transformPolygonRing.clone() );
  transformPolygon.transformVertices( transform );
  QCOMPARE( transformPolygon.asWkt( 2 ), QStringLiteral( "PolygonZM ((15 8 11 14, 8 18 21 24, 15 18 21 24, 15 28 31 34, 15 8 11 14),(12 5 9 11, 6 15 17 19, 13.01 5.01 19 21, 13 5.01 29 31, 13 5 9 11, 12 5 9 11),(3 5 9 11, 13.01 5.01 19 21, 13 5.01 29 31, 3 5 9 11))" ) );

  // transform using class
  QgsPolygon transformPolygon2;
  TestTransformer transformer;
  // no crash
  QVERIFY( transformPolygon2.transform( &transformer ) );
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  transformPolygon2.setExteriorRing( transformPolygonRing.clone() );
  QVERIFY( transformPolygon2.transform( &transformer ) );
  QCOMPARE( transformPolygon2.asWkt( 2 ), QStringLiteral( "PolygonZM ((33 16 8 3, 12 26 18 13, 33 26 18 13, 33 36 28 23, 33 16 8 3))" ) );
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  transformPolygon2.addInteriorRing( transformPolygonRing.clone() );
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  transformPolygon2.addInteriorRing( transformPolygonRing.clone() );
  QVERIFY( transformPolygon2.transform( &transformer ) );
  QCOMPARE( transformPolygon2.asWkt( 2 ), QStringLiteral( "PolygonZM ((99 30 13 2, 36 40 23 12, 99 40 23 12, 99 50 33 22, 99 30 13 2),(30 16 10 5, 12 26 18 13, 33.03 16.01 20 15, 33 16.01 30 25, 33 16 10 5, 30 16 10 5),(3 16 10 5, 33.03 16.01 20 15, 33 16.01 30 25, 3 16 10 5))" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !transformPolygon2.transform( &failTransformer ) );

  // remove invalid rings
  QgsPolygon invalidRingPolygon;
  invalidRingPolygon.removeInvalidRings(); // no crash
  QgsLineString removeInvalidPolygonRing;
  removeInvalidPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  invalidRingPolygon.setExteriorRing( removeInvalidPolygonRing.clone() );
  invalidRingPolygon.removeInvalidRings();
  QCOMPARE( invalidRingPolygon.asWkt(), QStringLiteral( "PolygonZM ((11 2 3 4, 4 12 13 14, 11 12 13 14, 11 22 23 24, 11 2 3 4))" ) );
  removeInvalidPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM )  << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  invalidRingPolygon.addInteriorRing( removeInvalidPolygonRing.clone() );
  removeInvalidPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  invalidRingPolygon.addInteriorRing( removeInvalidPolygonRing.clone() );
  invalidRingPolygon.removeInvalidRings();
  QCOMPARE( invalidRingPolygon.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 4 12 13 14, 11 12 13 14, 11 22 23 24, 11 2 3 4),(1 2 5 6, 11.01 2.01 15 16, 11 2.01 25 26, 1 2 5 6))" ) );

  // force RHR
  QgsPolygon rhr;
  rhr.forceRHR(); // no crash
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 23 24, 0 100 23 24, 0 0 3 4))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 23 24, 100 100 23 24, 100 0 13 14, 0 0 3 4))" ) );
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 10 20 6 8, 10 10 1 2))" ) );
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 13 14, 0 100 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 23 24, 100 100 13 14, 100 0 13 14, 0 0 3 4),(10 10 1 2, 20 10 3 4, 10 20 6 8, 10 10 1 2))" ) );
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 10 20 3 4, 20 10 6 8, 10 10 1 2))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 6 8, 10 20 3 4, 10 10 1 2))" ) );

  // test bounding box intersects
  QgsPolygon bb;
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QgsLineString bbR;
  bbR.setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( 4, 2 ) << QgsPoint( 4, 4 ) << QgsPoint( 0, 2 ) );
  bb.setExteriorRing( bbR.clone() );
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  // double test because of cache
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 11, 3, 6, 9 ) ) );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 11, 3, 6, 9 ) ) );
  QCOMPARE( bb.boundingBox(), QgsRectangle( 0, 2, 4, 4 ) );
  // clear cache
  bbR.setPoints( QgsPointSequence() << QgsPoint( 10, 2 ) << QgsPoint( 14, 2 ) << QgsPoint( 15, 4 ) << QgsPoint( 10, 2 ) );
  bb.setExteriorRing( bbR.clone() );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QCOMPARE( bb.boundingBox(), QgsRectangle( 10, 2, 15, 4 ) );
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 11, 3, 16, 9 ) ) );
  // technically invalid -- the interior ring is outside the exterior, but we want boundingBoxIntersects to be tolerant to
  // cases like this!
  bbR.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 4, 2 ) << QgsPoint( 5, 4 ) << QgsPoint( 1, 2 ) );
  bb.addInteriorRing( bbR.clone() );
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 11, 3, 16, 9 ) ) );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 21, 3, 26, 9 ) ) );
  QCOMPARE( bb.boundingBox(), QgsRectangle( 10, 2, 15, 4 ) );
}

QGSTEST_MAIN( TestQgsPolygon )
#include "testqgspolygon.moc"
