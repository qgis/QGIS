/***************************************************************************
     testqgstriangle.cpp
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
//header for class being tested
#include "qgspoint.h"
#include "qgstriangle.h"
#include "qgslinestring.h"
#include "qgsgeometryutils.h"
#include "testgeometryutils.h"

class TestQgsTriangle: public QObject
{
    Q_OBJECT
  private slots:
    void triangle();
};

void TestQgsTriangle::triangle()
{
  //test constructor
  QgsTriangle t1;
  QVERIFY( t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 0 );
  QCOMPARE( t1.ringCount(), 0 );
  QCOMPARE( t1.partCount(), 0 );
  QVERIFY( !t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t1.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( !t1.hasCurvedSegments() );
  QCOMPARE( t1.area(), 0.0 );
  QCOMPARE( t1.perimeter(), 0.0 );
  QVERIFY( !t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );

  // degenerate triangles
  QgsTriangle invalid( QgsPointXY( 0, 0 ), QgsPointXY( 0, 0 ), QgsPointXY( 10, 10 ) );
  QVERIFY( !invalid.isEmpty() );
  invalid = QgsTriangle( QPointF( 0, 0 ), QPointF( 0, 0 ), QPointF( 10, 10 ) );
  QVERIFY( !invalid.isEmpty() );
  //set exterior ring

  //try with no ring
  std::unique_ptr< QgsLineString > ext;
  t1.setExteriorRing( nullptr );
  QVERIFY( t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 0 );
  QCOMPARE( t1.ringCount(), 0 );
  QCOMPARE( t1.partCount(), 0 );
  QVERIFY( !t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );

  //valid exterior ring
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 0, 0 ) );
  QVERIFY( ext->isClosed() );
  t1.setExteriorRing( ext->clone() );
  QVERIFY( !t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 4 );
  QCOMPARE( t1.ringCount(), 1 );
  QCOMPARE( t1.partCount(), 1 );
  QVERIFY( !t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t1.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( !t1.hasCurvedSegments() );
  QCOMPARE( t1.area(), 50.0 );
  QGSCOMPARENEAR( t1.perimeter(), 34.1421, 0.001 );
  QVERIFY( t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsLineString * >( t1.exteriorRing() ) ), *ext );

  //set new ExteriorRing
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 10 ) << QgsPoint( 5, 5 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 0, 10 ) );
  QVERIFY( ext->isClosed() );
  t1.setExteriorRing( ext->clone() );
  QVERIFY( !t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 4 );
  QCOMPARE( t1.ringCount(), 1 );
  QCOMPARE( t1.partCount(), 1 );
  QVERIFY( !t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t1.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( !t1.hasCurvedSegments() );
  QCOMPARE( t1.area(), 25.0 );
  QGSCOMPARENEAR( t1.perimeter(), 24.1421, 0.001 );
  QVERIFY( t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( t1.exteriorRing() ) ), *ext );

  // AddZ
  QgsLineString lz;
  lz.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  t1.setExteriorRing( lz.clone() );
  QVERIFY( t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::TriangleZ );
  QCOMPARE( t1.wktTypeStr(), QString( "TriangleZ" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QCOMPARE( t1.vertexAt( 0 ).z(),  3.0 );
  QCOMPARE( t1.vertexAt( 1 ).z(), 13.0 );
  QCOMPARE( t1.vertexAt( 2 ).z(), 23.0 );
  // AddM
  QgsLineString lzm;
  lzm.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  t1.setExteriorRing( lzm.clone() );
  QVERIFY( t1.is3D() );
  QVERIFY( t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( t1.wktTypeStr(), QString( "TriangleZM" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QCOMPARE( t1.vertexAt( 0 ).m(),  4.0 );
  QCOMPARE( t1.vertexAt( 1 ).m(), 14.0 );
  QCOMPARE( t1.vertexAt( 2 ).m(), 24.0 );
  // dropZ
  t1.dropZValue();
  QVERIFY( !t1.is3D() );
  QVERIFY( t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::TriangleM );
  QCOMPARE( t1.wktTypeStr(), QString( "TriangleM" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( std::isnan( t1.vertexAt( 0 ).z() ) );
  QVERIFY( std::isnan( t1.vertexAt( 1 ).z() ) );
  QVERIFY( std::isnan( t1.vertexAt( 2 ).z() ) );
  // dropM
  t1.dropMValue();
  QVERIFY( !t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t1.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( std::isnan( t1.vertexAt( 0 ).m() ) );
  QVERIFY( std::isnan( t1.vertexAt( 1 ).m() ) );
  QVERIFY( std::isnan( t1.vertexAt( 2 ).m() ) );

  //test that a non closed exterior ring will be automatically closed
  QgsTriangle t2;
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) );
  QVERIFY( !ext->isClosed() );
  t2.setExteriorRing( ext.release() );
  QVERIFY( !t2.isEmpty() );
  QVERIFY( t2.exteriorRing()->isClosed() );
  QCOMPARE( t2.nCoordinates(), 4 );

  // invalid number of points
  ext.reset( new QgsLineString() );
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( t2.isEmpty() );

  ext.reset( new QgsLineString() );
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 5, 10 ) << QgsPoint( 8, 10 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( t2.isEmpty() );

  // invalid exterior ring
  ext.reset( new QgsLineString() );
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 5, 10 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( t2.isEmpty() );

  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 0, 0 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( t2.isEmpty() );

  // degenerate case
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 0, 0 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( !t2.isEmpty() );

  // circular ring
  QgsCircularString *circularRing = new QgsCircularString();
  t2.clear();
  circularRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) );
  QVERIFY( circularRing->hasCurvedSegments() );
  t2.setExteriorRing( circularRing );
  QVERIFY( t2.isEmpty() );

  //constructor with 3 points
  // double points
  QgsTriangle t3( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) );
  QVERIFY( !t3.isEmpty() );
  QCOMPARE( t3.numInteriorRings(), 0 );
  QCOMPARE( t3.nCoordinates(), 4 );
  QCOMPARE( t3.ringCount(), 1 );
  QCOMPARE( t3.partCount(), 1 );
  QVERIFY( !t3.is3D() );
  QVERIFY( !t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t3.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t3.dimension(), 2 );
  QVERIFY( !t3.hasCurvedSegments() );
  QCOMPARE( t3.area(), 0.0 );
  QGSCOMPARENEAR( t3.perimeter(), 28.284271, 0.001 );
  QVERIFY( t3.exteriorRing() );
  QVERIFY( !t3.interiorRing( 0 ) );

  // colinear
  t3 = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) );
  QVERIFY( !t3.isEmpty() );
  QCOMPARE( t3.numInteriorRings(), 0 );
  QCOMPARE( t3.nCoordinates(), 4 );
  QCOMPARE( t3.ringCount(), 1 );
  QCOMPARE( t3.partCount(), 1 );
  QVERIFY( !t3.is3D() );
  QVERIFY( !t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t3.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t3.dimension(), 2 );
  QVERIFY( !t3.hasCurvedSegments() );
  QCOMPARE( t3.area(), 0.0 );
  QGSCOMPARENEAR( t3.perimeter(), 10.0 * 2, 0.001 );
  QVERIFY( t3.exteriorRing() );
  QVERIFY( !t3.interiorRing( 0 ) );

  t3 = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  QVERIFY( !t3.isEmpty() );
  QCOMPARE( t3.numInteriorRings(), 0 );
  QCOMPARE( t3.nCoordinates(), 4 );
  QCOMPARE( t3.ringCount(), 1 );
  QCOMPARE( t3.partCount(), 1 );
  QVERIFY( !t3.is3D() );
  QVERIFY( !t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t3.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t3.dimension(), 2 );
  QVERIFY( !t3.hasCurvedSegments() );
  QCOMPARE( t3.area(), 50.0 );
  QGSCOMPARENEAR( t3.perimeter(), 34.1421, 0.001 );
  QVERIFY( t3.exteriorRing() );
  QVERIFY( !t3.interiorRing( 0 ) );

  // equality
  QVERIFY( QgsTriangle() == QgsTriangle() ); // empty
  QVERIFY( QgsTriangle() != QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) ) ); // empty
  QVERIFY( QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) ) != QgsTriangle() ); // empty
  QVERIFY( QgsTriangle() != QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 0, 10 ) ) );
  QVERIFY( QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 0, 10 ) ) != QgsTriangle() );
  QVERIFY( QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 0, 10 ) ) != QgsTriangle( QgsPoint( 0, 10 ), QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ) );

  // clone
  QgsTriangle *t4 = t3.clone();
  QCOMPARE( t3, *t4 );
  delete t4;

  // constructor from QgsPointXY and QPointF
  QgsTriangle t_qgspoint = QgsTriangle( QgsPointXY( 0, 0 ), QgsPointXY( 0, 10 ), QgsPointXY( 10, 10 ) );
  QVERIFY( t3 == t_qgspoint );
  QgsTriangle t_pointf = QgsTriangle( QPointF( 0, 0 ), QPointF( 0, 10 ), QPointF( 10, 10 ) );
  QVERIFY( t3 == t_pointf );

  // Z
  t3 = QgsTriangle( QgsPoint( QgsWkbTypes::PointZ, 0, 5, 1 ), QgsPoint( QgsWkbTypes::PointZ, 0, 0, 2 ), QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 ) );
  QVERIFY( !t3.isEmpty() );
  QVERIFY( t3.is3D() );
  QVERIFY( !t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::TriangleZ );
  QCOMPARE( t3.wktTypeStr(), QString( "TriangleZ" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  // M
  t3 = QgsTriangle( QgsPoint( QgsWkbTypes::PointM, 0, 5, 0, 1 ), QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 2 ), QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 ) );
  QVERIFY( !t3.isEmpty() );
  QVERIFY( !t3.is3D() );
  QVERIFY( t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::TriangleM );
  QCOMPARE( t3.wktTypeStr(), QString( "TriangleM" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  // ZM
  t3 = QgsTriangle( QgsPoint( QgsWkbTypes::PointZM, 0, 5, 8, 1 ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 5, 2 ), QgsPoint( QgsWkbTypes::PointZM, 10, 10, 2, 3 ) );
  QVERIFY( !t3.isEmpty() );
  QVERIFY( t3.is3D() );
  QVERIFY( t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( t3.wktTypeStr(), QString( "TriangleZM" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );

  // fromWkt
  QgsTriangle t5;
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  t5.setExteriorRing( ext.release() );
  QString wkt = t5.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsTriangle t6;
  QVERIFY( t6.fromWkt( wkt ) );
  QCOMPARE( t5, t6 );

  // conversion
  QgsPolygon p1;
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  p1.setExteriorRing( ext.release() );
  //toPolygon
  std::unique_ptr< QgsPolygon > poly( t5.toPolygon() );
  QCOMPARE( *poly, p1 );
  //surfaceToPolygon
  std::unique_ptr< QgsPolygon > surface( t5.surfaceToPolygon() );
  QCOMPARE( *surface, p1 );

  //bad WKT
  QVERIFY( !t6.fromWkt( "Point()" ) );
  QVERIFY( t6.isEmpty() );
  QVERIFY( !t6.exteriorRing() );
  QCOMPARE( t6.numInteriorRings(), 0 );
  QVERIFY( !t6.is3D() );
  QVERIFY( !t6.isMeasure() );
  QCOMPARE( t6.wkbType(), QgsWkbTypes::Triangle );

  // WKB
  QgsTriangle tResult, tWKB;
  QByteArray wkb;
// WKB noZM
  tWKB = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  wkb = tWKB.asWkb();
  QCOMPARE( wkb.size(), tWKB.wkbSize() );
  tResult.clear();
  QgsConstWkbPtr wkbPtr( wkb );
  tResult.fromWkb( wkbPtr );
  QCOMPARE( tWKB.asWkt(), "Triangle ((0 0, 0 10, 10 10, 0 0))" );
  QCOMPARE( tWKB.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tWKB, tResult );
// WKB Z
  tWKB = QgsTriangle( QgsPoint( 0, 0, 1 ), QgsPoint( 0, 10, 2 ), QgsPoint( 10, 10, 3 ) );
  wkb = tWKB.asWkb();
  QCOMPARE( wkb.size(), tWKB.wkbSize() );
  tResult.clear();
  QgsConstWkbPtr wkbPtrZ( wkb );
  tResult.fromWkb( wkbPtrZ );
  QCOMPARE( tWKB.asWkt(), "TriangleZ ((0 0 1, 0 10 2, 10 10 3, 0 0 1))" );
  QCOMPARE( tWKB.wkbType(), QgsWkbTypes::TriangleZ );
  QCOMPARE( tWKB, tResult );
// WKB M
// tWKB=QgsTriangle (QgsPoint(0,0, 5), QgsPoint(0, 10, 6), QgsPoint(10, 10, 7)); will produce a TriangleZ
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 6 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 7 ) );
  tWKB.setExteriorRing( ext.release() );
  wkb = tWKB.asWkb();
  QCOMPARE( wkb.size(), tWKB.wkbSize() );
  tResult.clear();
  QgsConstWkbPtr  wkbPtrM( wkb );
  tResult.fromWkb( wkbPtrM );
  QCOMPARE( tWKB.asWkt(), "TriangleM ((0 0 5, 0 10 6, 10 10 7, 0 0 5))" );
  QCOMPARE( tWKB.wkbType(), QgsWkbTypes::TriangleM );
  QCOMPARE( tWKB, tResult );
// WKB ZM
  tWKB = QgsTriangle( QgsPoint( 0, 0, 1, 5 ), QgsPoint( 0, 10, 2, 6 ), QgsPoint( 10, 10, 3, 7 ) );
  wkb = tWKB.asWkb();
  QCOMPARE( wkb.size(), tWKB.wkbSize() );
  tResult.clear();
  QgsConstWkbPtr wkbPtrZM( wkb );
  tResult.fromWkb( wkbPtrZM );
  QCOMPARE( tWKB.asWkt(), "TriangleZM ((0 0 1 5, 0 10 2 6, 10 10 3 7, 0 0 1 5))" );
  QCOMPARE( tWKB.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( tWKB, tResult );

  //bad WKB - check for no crash
  t6.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !t6.fromWkb( nullPtr ) );
  QCOMPARE( t6.wkbType(), QgsWkbTypes::Triangle );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !t6.fromWkb( wkbPointPtr ) );
  QCOMPARE( t6.wkbType(), QgsWkbTypes::Triangle );
  // invalid multi ring
  // ba is equivalent to "Triangle((0 0, 0 5, 5 5, 0 0), (2 2, 2 4, 3 3, 2 2))"
  QByteArray ba = QByteArray::fromHex( "01110000000200000004000000000000000000000000000000000000000000000000000000000000000000144000000000000014400000000000001440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000010400000000000000840000000000000084000000000000000400000000000000040" );
  QgsTriangle tInvalidWkb;
  QgsConstWkbPtr wkbMultiRing( ba );
  QVERIFY( !tInvalidWkb.fromWkb( wkbMultiRing ) );
  QCOMPARE( tInvalidWkb, QgsTriangle() );

  //asGML2
  QgsTriangle exportTriangle( QgsPoint( 1, 2 ),
                              QgsPoint( 3, 4 ),
                              QgsPoint( 6, 5 ) );
  QgsTriangle exportTriangleZ( QgsPoint( 1, 2, 3 ),
                               QgsPoint( 11, 12, 13 ),
                               QgsPoint( 1, 12, 23 ) );
  QgsTriangle exportTriangleFloat( QgsPoint( 1 + 1 / 3.0, 2 + 2 / 3.0 ),
                                   QgsPoint( 3 + 1 / 3.0, 4 + 2 / 3.0 ),
                                   QgsPoint( 6 + 1 / 3.0, 5 + 2 / 3.0 ) );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,2 3,4 6,5 1,2</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QGSCOMPAREGML( elemToString( exportTriangle.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.333,2.667 3.333,4.667 6.333,5.667 1.333,2.667</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QGSCOMPAREGML( elemToString( exportTriangleFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsTriangle().asGml2( doc ) ), expectedGML2empty );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 2 3 4 6 5 1 2</posList></LinearRing></exterior></Triangle>" ) );
  QCOMPARE( elemToString( exportTriangle.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1.333 2.667 3.333 4.667 6.333 5.667 1.333 2.667</posList></LinearRing></exterior></Triangle>" ) );
  QCOMPARE( elemToString( exportTriangleFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<Triangle xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsTriangle().asGml3( doc ) ), expectedGML3empty );
  QString expectedGML3Z( QStringLiteral( "<Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">1 2 3 11 12 13 1 12 23 1 2 3</posList></LinearRing></exterior></Triangle>" ) );
  QCOMPARE( elemToString( exportTriangleZ.asGml3( doc ) ), expectedGML3Z );


  // lengths and angles
  QgsTriangle t7( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );

  QVector<double> a_tested, a_t7 = t7.angles();
  a_tested.append( M_PI / 4.0 );
  a_tested.append( M_PI / 2.0 );
  a_tested.append( M_PI / 4.0 );
  QGSCOMPARENEAR( a_tested.at( 0 ), a_t7.at( 0 ), 0.0001 );
  QGSCOMPARENEAR( a_tested.at( 1 ), a_t7.at( 1 ), 0.0001 );
  QGSCOMPARENEAR( a_tested.at( 2 ), a_t7.at( 2 ), 0.0001 );
  QVector<double> a_empty = QgsTriangle().angles();
  QVERIFY( a_empty.isEmpty() );

  QVector<double> l_tested, l_t7 = t7.lengths();
  l_tested.append( 5 );
  l_tested.append( 5 );
  l_tested.append( std::sqrt( 5 * 5 + 5 * 5 ) );
  QGSCOMPARENEAR( l_tested.at( 0 ), l_t7.at( 0 ), 0.0001 );
  QGSCOMPARENEAR( l_tested.at( 1 ), l_t7.at( 1 ), 0.0001 );
  QGSCOMPARENEAR( l_tested.at( 2 ), l_t7.at( 2 ), 0.0001 );
  QVector<double> l_empty = QgsTriangle().lengths();
  QVERIFY( l_empty.isEmpty() );

  // type of triangle
  // Empty triangle returns always false for the types, except isDegenerate
  QVERIFY( QgsTriangle().isDegenerate() );
  QVERIFY( !QgsTriangle().isRight() );
  QVERIFY( !QgsTriangle().isIsocele() );
  QVERIFY( !QgsTriangle().isScalene() );
  QVERIFY( !QgsTriangle().isEquilateral() );

  // type of triangle
  QVERIFY( !t7.isDegenerate() );
  QVERIFY( t7.isRight() );
  QVERIFY( t7.isIsocele() );
  QVERIFY( !t7.isScalene() );
  QVERIFY( !t7.isEquilateral() );

  QgsTriangle t8( QgsPoint( 7.2825, 4.2368 ), QgsPoint( 13.0058, 3.3218 ), QgsPoint( 9.2145, 6.5242 ) );
  // angles in radians 58.8978;31.1036;89.9985
  // length 5.79598;4.96279;2.99413
  QVERIFY( !t8.isDegenerate() );
  QVERIFY( t8.isRight() );
  QVERIFY( !t8.isIsocele() );
  QVERIFY( t8.isScalene() );
  QVERIFY( !t8.isEquilateral() );

  QgsTriangle t9( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );
  QVERIFY( !t9.isDegenerate() );
  QVERIFY( !t9.isRight() );
  QVERIFY( t9.isIsocele() );
  QVERIFY( !t9.isScalene() );
  QVERIFY( t9.isEquilateral() );

  // vertex
  QCOMPARE( QgsTriangle().vertexAt( 0 ), QgsPoint() );
  QCOMPARE( QgsTriangle().vertexAt( -1 ), QgsPoint() );
  QVERIFY( t9.vertexAt( -1 ).isEmpty() );
  QCOMPARE( t9.vertexAt( 0 ), QgsPoint( 10, 10 ) );
  QCOMPARE( t9.vertexAt( 1 ), QgsPoint( 16, 10 ) );
  QCOMPARE( t9.vertexAt( 2 ), QgsPoint( 13, 15.1962 ) );
  QCOMPARE( t9.vertexAt( 3 ), QgsPoint( 10, 10 ) );
  QVERIFY( t9.vertexAt( 4 ).isEmpty() );

  // altitudes
  QgsTriangle t10( QgsPoint( 20, 2 ), QgsPoint( 16, 6 ), QgsPoint( 26, 2 ) );
  QVector<QgsLineString> alt = QgsTriangle().altitudes();
  QVERIFY( alt.isEmpty() );
  alt = t10.altitudes();
  QGSCOMPARENEARPOINT( alt.at( 0 ).pointN( 1 ), QgsPoint( 20.8276, 4.0690 ), 0.0001 );
  QGSCOMPARENEARPOINT( alt.at( 1 ).pointN( 1 ), QgsPoint( 16, 2 ), 0.0001 );
  QGSCOMPARENEARPOINT( alt.at( 2 ).pointN( 1 ), QgsPoint( 23, -1 ), 0.0001 );

  // orthocenter

  QVERIFY( QgsTriangle().orthocenter().isEmpty() );
  QCOMPARE( QgsPoint( 16, -8 ), t10.orthocenter() );
  QCOMPARE( QgsPoint( 0, 5 ), t7.orthocenter() );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.orthocenter(), 0.0001 );

  // circumscribed circle
  QCOMPARE( QgsTriangle().circumscribedCircle(), QgsCircle() );
  QVERIFY( QgsTriangle().circumscribedCenter().isEmpty() );
  QCOMPARE( 0.0, QgsTriangle().circumscribedRadius() );
  QCOMPARE( QgsPoint( 2.5, 2.5 ), t7.circumscribedCenter() );
  QGSCOMPARENEAR( 3.5355, t7.circumscribedRadius(), 0.0001 );
  QCOMPARE( QgsPoint( 23, 9 ), t10.circumscribedCenter() );
  QGSCOMPARENEAR( 7.6158, t10.circumscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.circumscribedCenter(), 0.0001 );
  QGSCOMPARENEAR( 3.4641, t9.circumscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.circumscribedCircle().center(), 0.0001 );
  QGSCOMPARENEAR( 3.4641, t9.circumscribedCircle().radius(), 0.0001 );

  // inscribed circle
  QCOMPARE( QgsTriangle().inscribedCircle(), QgsCircle() );
  QVERIFY( QgsTriangle().inscribedCenter().isEmpty() );
  QCOMPARE( 0.0, QgsTriangle().inscribedRadius() );
  QGSCOMPARENEARPOINT( QgsPoint( 1.4645, 3.5355 ), t7.inscribedCenter(), 0.001 );
  QGSCOMPARENEAR( 1.4645, t7.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 20.4433, 3.0701 ), t10.inscribedCenter(), 0.001 );
  QGSCOMPARENEAR( 1.0701, t10.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.inscribedCenter(), 0.0001 );
  QGSCOMPARENEAR( 1.7321, t9.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.inscribedCircle().center(), 0.0001 );
  QGSCOMPARENEAR( 1.7321, t9.inscribedCircle().radius(), 0.0001 );

  // medians
  QVector<QgsLineString> med = QgsTriangle().medians();
  QVERIFY( med.isEmpty() );
  med = t7.medians();
  QCOMPARE( med.at( 0 ).pointN( 0 ), t7.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), QgsPoint( 2.5, 5 ), 0.0001 );
  QCOMPARE( med.at( 1 ).pointN( 0 ), t7.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), QgsPoint( 2.5, 2.5 ), 0.0001 );
  QCOMPARE( med.at( 2 ).pointN( 0 ), t7.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), QgsPoint( 0, 2.5 ), 0.0001 );
  med.clear();

  med = t10.medians();
  QCOMPARE( med.at( 0 ).pointN( 0 ), t10.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), QgsPoint( 21, 4 ), 0.0001 );
  QCOMPARE( med.at( 1 ).pointN( 0 ), t10.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), QgsPoint( 23, 2 ), 0.0001 );
  QCOMPARE( med.at( 2 ).pointN( 0 ), t10.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), QgsPoint( 18, 4 ), 0.0001 );
  med.clear();
  alt.clear();

  med = t9.medians();
  alt = t9.altitudes();
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 0 ), alt.at( 0 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), alt.at( 0 ).pointN( 1 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 0 ), alt.at( 1 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), alt.at( 1 ).pointN( 1 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 0 ), alt.at( 2 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), alt.at( 2 ).pointN( 1 ), 0.0001 );

  // medial
  QCOMPARE( QgsTriangle().medial(), QgsTriangle() );
  QCOMPARE( t7.medial(), QgsTriangle( QgsPoint( 0, 2.5 ), QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 2.5 ) ) );
  QCOMPARE( t9.medial(), QgsTriangle( QgsGeometryUtils::midpoint( t9.vertexAt( 0 ), t9.vertexAt( 1 ) ),
                                      QgsGeometryUtils::midpoint( t9.vertexAt( 1 ), t9.vertexAt( 2 ) ),
                                      QgsGeometryUtils::midpoint( t9.vertexAt( 2 ), t9.vertexAt( 0 ) ) ) );

  // bisectors
  QVector<QgsLineString> bis = QgsTriangle().bisectors();
  QVERIFY( bis.isEmpty() );
  bis = t7.bisectors();
  QCOMPARE( bis.at( 0 ).pointN( 0 ), t7.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( bis.at( 0 ).pointN( 1 ), QgsPoint( 2.0711, 5 ), 0.0001 );
  QCOMPARE( bis.at( 1 ).pointN( 0 ), t7.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( bis.at( 1 ).pointN( 1 ), QgsPoint( 2.5, 2.5 ), 0.0001 );
  QCOMPARE( bis.at( 2 ).pointN( 0 ), t7.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( bis.at( 2 ).pointN( 1 ), QgsPoint( 0, 2.9289 ), 0.0001 );

  // "deleted" method
  ext.reset( new QgsLineString() );
  QgsTriangle t11( QgsPoint( 0, 0 ), QgsPoint( 100, 100 ), QgsPoint( 0, 200 ) );
  ext->setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                  << QgsPoint( 50, 50 ) << QgsPoint( 0, 25 )
                  << QgsPoint( 5, 5 ) );
  t11.addInteriorRing( ext.release() );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );

  /* QList<QgsCurve *> lc;
   lc.append(ext);
   t11.setInteriorRings( lc );
   QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );*/

  QgsVertexId id( 0, 0, 1 );
  QVERIFY( !t11.deleteVertex( id ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );
  QVERIFY( !t11.insertVertex( id, QgsPoint( 5, 5 ) ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );

  //move vertex
  QgsPoint pt1( 5, 5 );
  // empty triangle
  QVERIFY( !QgsTriangle().moveVertex( id, pt1 ) );
  // invalid part
  id.part = -1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  id.part = 1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  // invalid ring
  id.part = 0;
  id.ring = -1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  id.ring = 1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  id.ring = 0;
  id.vertex = -1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  id.vertex = 5;
  QVERIFY( !t11.moveVertex( id, pt1 ) );

  // valid vertex
  id.vertex = 0;
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((5 5, 100 100, 0 200, 5 5))" ) );
  pt1 = QgsPoint( 0, 0 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );
  id.vertex = 4;
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((5 5, 100 100, 0 200, 5 5))" ) );
  pt1 = QgsPoint( 0, 0 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );
  id.vertex = 1;
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 5 5, 0 200, 0 0))" ) );
  // colinear
  pt1 = QgsPoint( 0, 100 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  // duplicate point
  pt1 = QgsPoint( 0, 0 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( t11.moveVertex( id, pt1 ) );

  //toCurveType
  QgsTriangle t12( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) );
  std::unique_ptr< QgsCurvePolygon > curveType( t12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( curveType->exteriorRing()->numPoints(), 4 );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 13, 3 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 9, 6 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( curveType->numInteriorRings(), 0 );

  // boundary
  QVERIFY( !QgsTriangle().boundary() );
  std::unique_ptr< QgsCurve > boundary( QgsTriangle( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) ).boundary() );
  QCOMPARE( boundary->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( boundary->numPoints(), 4 );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 13, 3 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 9, 6 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 7, 4 ) );

  // cast
  QgsTriangle pCast;
  QVERIFY( QgsPolygon().cast( &pCast ) );
  QgsTriangle pCast2( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) );
  QVERIFY( QgsPolygon().cast( &pCast2 ) );
}


QGSTEST_MAIN( TestQgsTriangle )
#include "testqgstriangle.moc"
