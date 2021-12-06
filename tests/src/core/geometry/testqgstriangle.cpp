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

#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgstriangle.h"

#include "testgeometryutils.h"

class TestQgsTriangle: public QObject
{
    Q_OBJECT

  private slots:
    void constructor();
    void degenerateConstructor();
    void constructor3Points();
    void constructorZM();
    void constructorQgsPointXY();
    void constructorQPointF();
    void exteriorRing();
    void exteriorRingZM();
    void invalidExteriorRing();
    void invalidNumberOfPoints();
    void nonClosedRing();
    void clone();
    void conversion();
    void toCurveType();
    void cast();
    void toFromWkt();
    void toFromWkb();
    void exportImport();
    void vertexAt();
    void moveVertex();
    void deleteVertex();
    void types();
    void equality();
    void angles();
    void lengths();
    void orthocenter();
    void altitudes();
    void medians();
    void medial();
    void bisectors();
    void inscribedCircle();
    void circumscribedCircle();
    void boundary();
};

void TestQgsTriangle::constructor()
{
  QgsTriangle tr;

  QVERIFY( tr.isEmpty() );
  QCOMPARE( tr.numInteriorRings(), 0 );
  QCOMPARE( tr.nCoordinates(), 0 );
  QCOMPARE( tr.ringCount(), 0 );
  QCOMPARE( tr.partCount(), 0 );
  QVERIFY( !tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tr.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QVERIFY( !tr.hasCurvedSegments() );
  QCOMPARE( tr.area(), 0.0 );
  QCOMPARE( tr.perimeter(), 0.0 );
  QVERIFY( !tr.exteriorRing() );
  QVERIFY( !tr.interiorRing( 0 ) );
}

void TestQgsTriangle::degenerateConstructor()
{
  QgsTriangle tr( QgsPointXY( 0, 0 ), QgsPointXY( 0, 0 ), QgsPointXY( 10, 10 ) );
  QVERIFY( !tr.isEmpty() );

  tr = QgsTriangle( QPointF( 0, 0 ), QPointF( 0, 0 ), QPointF( 10, 10 ) );
  QVERIFY( !tr.isEmpty() );
}

void TestQgsTriangle::constructor3Points()
{
  // double points
  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) );

  QVERIFY( !tr.isEmpty() );
  QCOMPARE( tr.numInteriorRings(), 0 );
  QCOMPARE( tr.nCoordinates(), 4 );
  QCOMPARE( tr.ringCount(), 1 );
  QCOMPARE( tr.partCount(), 1 );
  QVERIFY( !tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tr.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QVERIFY( !tr.hasCurvedSegments() );
  QCOMPARE( tr.area(), 0.0 );
  QGSCOMPARENEAR( tr.perimeter(), 28.284271, 0.001 );
  QVERIFY( tr.exteriorRing() );
  QVERIFY( !tr.interiorRing( 0 ) );

  // colinear
  tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) );

  QVERIFY( !tr.isEmpty() );
  QCOMPARE( tr.numInteriorRings(), 0 );
  QCOMPARE( tr.nCoordinates(), 4 );
  QCOMPARE( tr.ringCount(), 1 );
  QCOMPARE( tr.partCount(), 1 );
  QVERIFY( !tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tr.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QVERIFY( !tr.hasCurvedSegments() );
  QCOMPARE( tr.area(), 0.0 );
  QGSCOMPARENEAR( tr.perimeter(), 10.0 * 2, 0.001 );
  QVERIFY( tr.exteriorRing() );
  QVERIFY( !tr.interiorRing( 0 ) );

  tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );

  QVERIFY( !tr.isEmpty() );
  QCOMPARE( tr.numInteriorRings(), 0 );
  QCOMPARE( tr.nCoordinates(), 4 );
  QCOMPARE( tr.ringCount(), 1 );
  QCOMPARE( tr.partCount(), 1 );
  QVERIFY( !tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tr.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QVERIFY( !tr.hasCurvedSegments() );
  QCOMPARE( tr.area(), 50.0 );
  QGSCOMPARENEAR( tr.perimeter(), 34.1421, 0.001 );
  QVERIFY( tr.exteriorRing() );
  QVERIFY( !tr.interiorRing( 0 ) );
}

void TestQgsTriangle::constructorZM()
{
  // Z
  QgsTriangle tr = QgsTriangle( QgsPoint( QgsWkbTypes::PointZ, 0, 5, 1 ),
                                QgsPoint( QgsWkbTypes::PointZ, 0, 0, 2 ),
                                QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 ) );

  QVERIFY( !tr.isEmpty() );
  QVERIFY( tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::TriangleZ );
  QCOMPARE( tr.wktTypeStr(), QString( "TriangleZ" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );

  // M
  tr = QgsTriangle( QgsPoint( QgsWkbTypes::PointM, 0, 5, 0, 1 ),
                    QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 2 ),
                    QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 ) );

  QVERIFY( !tr.isEmpty() );
  QVERIFY( !tr.is3D() );
  QVERIFY( tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::TriangleM );
  QCOMPARE( tr.wktTypeStr(), QString( "TriangleM" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );

  // ZM
  tr = QgsTriangle( QgsPoint( QgsWkbTypes::PointZM, 0, 5, 8, 1 ),
                    QgsPoint( QgsWkbTypes::PointZM, 0, 0, 5, 2 ),
                    QgsPoint( QgsWkbTypes::PointZM, 10, 10, 2, 3 ) );

  QVERIFY( !tr.isEmpty() );
  QVERIFY( tr.is3D() );
  QVERIFY( tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( tr.wktTypeStr(), QString( "TriangleZM" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
}

void TestQgsTriangle::constructorQgsPointXY()
{
  QgsTriangle tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ),
                                QgsPoint( 10, 10 ) );

  QgsTriangle t_qgspoint = QgsTriangle( QgsPointXY( 0, 0 ), QgsPointXY( 0, 10 ),
                                        QgsPointXY( 10, 10 ) );
  QVERIFY( tr == t_qgspoint );
}

void TestQgsTriangle::constructorQPointF()
{
  QgsTriangle tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ),
                                QgsPoint( 10, 10 ) );

  QgsTriangle t_pointf = QgsTriangle( QPointF( 0, 0 ), QPointF( 0, 10 ),
                                      QPointF( 10, 10 ) );
  QVERIFY( tr == t_pointf );
}

void TestQgsTriangle::exteriorRing()
{
  QgsTriangle tr;

  //try with no ring
  tr.setExteriorRing( nullptr );
  QVERIFY( tr.isEmpty() );
  QCOMPARE( tr.numInteriorRings(), 0 );
  QCOMPARE( tr.nCoordinates(), 0 );
  QCOMPARE( tr.ringCount(), 0 );
  QCOMPARE( tr.partCount(), 0 );
  QVERIFY( !tr.exteriorRing() );
  QVERIFY( !tr.interiorRing( 0 ) );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );

  std::unique_ptr< QgsLineString > ext( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                  << QgsPoint( 10, 10 ) << QgsPoint( 0, 0 ) );

  QVERIFY( ext->isClosed() );

  tr.setExteriorRing( ext->clone() );

  QVERIFY( !tr.isEmpty() );
  QCOMPARE( tr.numInteriorRings(), 0 );
  QCOMPARE( tr.nCoordinates(), 4 );
  QCOMPARE( tr.ringCount(), 1 );
  QCOMPARE( tr.partCount(), 1 );
  QVERIFY( !tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tr.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QVERIFY( !tr.hasCurvedSegments() );
  QCOMPARE( tr.area(), 50.0 );
  QGSCOMPARENEAR( tr.perimeter(), 34.1421, 0.001 );
  QVERIFY( tr.exteriorRing() );
  QVERIFY( !tr.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsLineString * >( tr.exteriorRing() ) ), *ext );

  //set new ExteriorRing
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 10 ) << QgsPoint( 5, 5 )
                  << QgsPoint( 10, 10 ) << QgsPoint( 0, 10 ) );
  QVERIFY( ext->isClosed() );

  tr.setExteriorRing( ext->clone() );

  QVERIFY( !tr.isEmpty() );
  QCOMPARE( tr.numInteriorRings(), 0 );
  QCOMPARE( tr.nCoordinates(), 4 );
  QCOMPARE( tr.ringCount(), 1 );
  QCOMPARE( tr.partCount(), 1 );
  QVERIFY( !tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tr.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QVERIFY( !tr.hasCurvedSegments() );
  QCOMPARE( tr.area(), 25.0 );
  QGSCOMPARENEAR( tr.perimeter(), 24.1421, 0.001 );
  QVERIFY( tr.exteriorRing() );
  QVERIFY( !tr.interiorRing( 0 ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( tr.exteriorRing() ) ), *ext );
}

void TestQgsTriangle::exteriorRingZM()
{
  QgsTriangle tr;

  // AddZ
  QgsLineString lz;
  lz.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 )
                << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  tr.setExteriorRing( lz.clone() );

  QVERIFY( tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::TriangleZ );
  QCOMPARE( tr.wktTypeStr(), QString( "TriangleZ" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QCOMPARE( tr.vertexAt( 0 ).z(),  3.0 );
  QCOMPARE( tr.vertexAt( 1 ).z(), 13.0 );
  QCOMPARE( tr.vertexAt( 2 ).z(), 23.0 );

  // AddM
  QgsLineString lzm;
  lzm.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 )
                 << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );

  tr.setExteriorRing( lzm.clone() );

  QVERIFY( tr.is3D() );
  QVERIFY( tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( tr.wktTypeStr(), QString( "TriangleZM" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QCOMPARE( tr.vertexAt( 0 ).m(),  4.0 );
  QCOMPARE( tr.vertexAt( 1 ).m(), 14.0 );
  QCOMPARE( tr.vertexAt( 2 ).m(), 24.0 );

  // dropZ
  tr.dropZValue();

  QVERIFY( !tr.is3D() );
  QVERIFY( tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::TriangleM );
  QCOMPARE( tr.wktTypeStr(), QString( "TriangleM" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QVERIFY( std::isnan( tr.vertexAt( 0 ).z() ) );
  QVERIFY( std::isnan( tr.vertexAt( 1 ).z() ) );
  QVERIFY( std::isnan( tr.vertexAt( 2 ).z() ) );

  // dropM
  tr.dropMValue();

  QVERIFY( !tr.is3D() );
  QVERIFY( !tr.isMeasure() );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tr.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( tr.geometryType(), QString( "Triangle" ) );
  QCOMPARE( tr.dimension(), 2 );
  QVERIFY( std::isnan( tr.vertexAt( 0 ).m() ) );
  QVERIFY( std::isnan( tr.vertexAt( 1 ).m() ) );
  QVERIFY( std::isnan( tr.vertexAt( 2 ).m() ) );
}

void TestQgsTriangle::invalidExteriorRing()
{
  QgsTriangle tr;

  // invalid exterior ring
  std::unique_ptr< QgsLineString > ext( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                  << QgsPoint( 10, 10 ) << QgsPoint( 5, 10 ) );
  tr.setExteriorRing( ext.release() );

  QVERIFY( tr.isEmpty() );

  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                  << QgsPoint( 0, 0 ) );
  tr.setExteriorRing( ext.release() );

  QVERIFY( tr.isEmpty() );

  // degenerate case
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 0, 0 ) );
  tr.setExteriorRing( ext.release() );

  QVERIFY( !tr.isEmpty() );

  // circular ring
  QgsCircularString *circularRing = new QgsCircularString();
  tr.clear();
  circularRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                           << QgsPoint( 10, 10 ) );

  QVERIFY( circularRing->hasCurvedSegments() );
  tr.setExteriorRing( circularRing );

  QVERIFY( tr.isEmpty() );
}

void TestQgsTriangle::invalidNumberOfPoints()
{
  QgsTriangle tr;

  std::unique_ptr< QgsLineString > ext( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) );
  tr.setExteriorRing( ext.release() );

  QVERIFY( tr.isEmpty() );

  ext.reset( new QgsLineString() );
  tr.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 5, 10 ) << QgsPoint( 8, 10 ) );
  tr.setExteriorRing( ext.release() );

  QVERIFY( tr.isEmpty() );
}

void TestQgsTriangle::nonClosedRing()
{
  QgsTriangle tr;

  // a non closed exterior ring will be automatically closed
  std::unique_ptr< QgsLineString > ext( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) );

  QVERIFY( !ext->isClosed() );

  tr.setExteriorRing( ext.release() );

  QVERIFY( !tr.isEmpty() );
  QVERIFY( tr.exteriorRing()->isClosed() );
  QCOMPARE( tr.nCoordinates(), 4 );
}

void TestQgsTriangle::clone()
{
  QgsTriangle tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ),
                                QgsPoint( 10, 10 ) );

  QgsTriangle *trClone = tr.clone();
  QCOMPARE( tr, *trClone );
  delete trClone;
}

void TestQgsTriangle::conversion()
{
  QgsTriangle tr;

  std::unique_ptr< QgsLineString > ext( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  tr.setExteriorRing( ext.release() );

  QgsPolygon polyExpected;
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  polyExpected.setExteriorRing( ext.release() );

  //toPolygon
  std::unique_ptr< QgsPolygon > poly( tr.toPolygon() );
  QCOMPARE( *poly, polyExpected );

  //surfaceToPolygon
  std::unique_ptr< QgsPolygon > surface( tr.surfaceToPolygon() );
  QCOMPARE( *surface, polyExpected );
}

void TestQgsTriangle::toCurveType()
{
  QgsTriangle tr( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) );
  std::unique_ptr< QgsCurvePolygon > curveType( tr.toCurveType() );

  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( curveType->exteriorRing()->numPoints(), 4 );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 13, 3 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 9, 6 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( curveType->numInteriorRings(), 0 );
}

void TestQgsTriangle::cast()
{
  QgsTriangle pCast;
  QVERIFY( QgsPolygon().cast( &pCast ) );

  QgsTriangle pCast2( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) );
  QVERIFY( QgsPolygon().cast( &pCast2 ) );
}

void TestQgsTriangle::toFromWkt()
{
  QgsTriangle tr;

  std::unique_ptr< QgsLineString > ext( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  tr.setExteriorRing( ext.release() );

  QString wkt = tr.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsTriangle trFromWkt;
  QVERIFY( trFromWkt.fromWkt( wkt ) );
  QCOMPARE( tr, trFromWkt );

  //bad WKT
  QVERIFY( !trFromWkt.fromWkt( "Point()" ) );
  QVERIFY( trFromWkt.isEmpty() );
  QVERIFY( !trFromWkt.exteriorRing() );
  QCOMPARE( trFromWkt.numInteriorRings(), 0 );
  QVERIFY( !trFromWkt.is3D() );
  QVERIFY( !trFromWkt.isMeasure() );
  QCOMPARE( trFromWkt.wkbType(), QgsWkbTypes::Triangle );
}

void TestQgsTriangle::toFromWkb()
{
  // WKB
  QgsTriangle tResult, tWKB;
  QByteArray wkb;

  // WKB noZM
  tWKB = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ),
                      QgsPoint( 10, 10 ) );
  wkb = tWKB.asWkb();
  QCOMPARE( wkb.size(), tWKB.wkbSize() );

  tResult.clear();

  QgsConstWkbPtr wkbPtr( wkb );
  tResult.fromWkb( wkbPtr );
  QCOMPARE( tWKB.asWkt(), "Triangle ((0 0, 0 10, 10 10, 0 0))" );
  QCOMPARE( tWKB.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( tWKB, tResult );

  // as a polygon
  tResult.clear();

  wkb = tWKB.asWkb( QgsAbstractGeometry::FlagExportTrianglesAsPolygons );
  QCOMPARE( wkb.size(), tWKB.wkbSize() );

  QgsPolygon pResult;

  QgsConstWkbPtr wkbPtr2( wkb );
  pResult.fromWkb( wkbPtr2 );
  QCOMPARE( pResult.asWkt(), "Polygon ((0 0, 0 10, 10 10, 0 0))" );
  QCOMPARE( pResult.wkbType(), QgsWkbTypes::Polygon );

  // WKB Z
  tWKB = QgsTriangle( QgsPoint( 0, 0, 1 ), QgsPoint( 0, 10, 2 ),
                      QgsPoint( 10, 10, 3 ) );
  wkb = tWKB.asWkb();
  QCOMPARE( wkb.size(), tWKB.wkbSize() );

  tResult.clear();

  QgsConstWkbPtr wkbPtrZ( wkb );
  tResult.fromWkb( wkbPtrZ );
  QCOMPARE( tWKB.asWkt(), "TriangleZ ((0 0 1, 0 10 2, 10 10 3, 0 0 1))" );
  QCOMPARE( tWKB.wkbType(), QgsWkbTypes::TriangleZ );
  QCOMPARE( tWKB, tResult );

  // as a polygon
  tResult.clear();

  wkb = tWKB.asWkb( QgsAbstractGeometry::FlagExportTrianglesAsPolygons );
  QCOMPARE( wkb.size(), tWKB.wkbSize() );

  QgsConstWkbPtr wkbPtrZ2( wkb );
  pResult.fromWkb( wkbPtrZ2 );
  QCOMPARE( pResult.asWkt(), "PolygonZ ((0 0 1, 0 10 2, 10 10 3, 0 0 1))" );
  QCOMPARE( pResult.wkbType(), QgsWkbTypes::PolygonZ );

  // WKB M
  // tWKB=QgsTriangle (QgsPoint(0,0, 5), QgsPoint(0, 10, 6), QgsPoint(10, 10, 7)); will produce a TriangleZ
  std::unique_ptr< QgsLineString > ext( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 6 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 7 ) );
  tWKB.setExteriorRing( ext.release() );

  wkb = tWKB.asWkb();
  QCOMPARE( wkb.size(), tWKB.wkbSize() );

  tResult.clear();

  QgsConstWkbPtr  wkbPtrM( wkb );
  tResult.fromWkb( wkbPtrM );
  QCOMPARE( tWKB.asWkt(), "TriangleM ((0 0 5, 0 10 6, 10 10 7, 0 0 5))" );
  QCOMPARE( tWKB.wkbType(), QgsWkbTypes::TriangleM );
  QCOMPARE( tWKB, tResult );

  // as a polygon
  tResult.clear();

  wkb = tWKB.asWkb( QgsAbstractGeometry::FlagExportTrianglesAsPolygons );
  QCOMPARE( wkb.size(), tWKB.wkbSize() );

  QgsConstWkbPtr wkbPtrM2( wkb );
  pResult.fromWkb( wkbPtrM2 );
  QCOMPARE( pResult.asWkt(), "PolygonM ((0 0 5, 0 10 6, 10 10 7, 0 0 5))" );
  QCOMPARE( pResult.wkbType(), QgsWkbTypes::PolygonM );

  // WKB ZM
  tWKB = QgsTriangle( QgsPoint( 0, 0, 1, 5 ), QgsPoint( 0, 10, 2, 6 ),
                      QgsPoint( 10, 10, 3, 7 ) );
  wkb = tWKB.asWkb();
  QCOMPARE( wkb.size(), tWKB.wkbSize() );

  tResult.clear();

  QgsConstWkbPtr wkbPtrZM( wkb );
  tResult.fromWkb( wkbPtrZM );
  QCOMPARE( tWKB.asWkt(), "TriangleZM ((0 0 1 5, 0 10 2 6, 10 10 3 7, 0 0 1 5))" );
  QCOMPARE( tWKB.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( tWKB, tResult );

  // as a polygon
  tResult.clear();

  wkb = tWKB.asWkb( QgsAbstractGeometry::FlagExportTrianglesAsPolygons );
  QCOMPARE( wkb.size(), tWKB.wkbSize() );

  QgsConstWkbPtr wkbPtrZM2( wkb );
  pResult.fromWkb( wkbPtrZM2 );
  QCOMPARE( pResult.asWkt(), "PolygonZM ((0 0 1 5, 0 10 2 6, 10 10 3 7, 0 0 1 5))" );
  QCOMPARE( pResult.wkbType(), QgsWkbTypes::PolygonZM );

  //bad WKB - check for no crash
  QgsTriangle tr;
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !tr.fromWkb( nullPtr ) );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !tr.fromWkb( wkbPointPtr ) );
  QCOMPARE( tr.wkbType(), QgsWkbTypes::Triangle );

  // AsWkb should work for polygons
  // even with FlagExportTrianglesAsPolygons
  QgsPolygon poly;
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  poly.setExteriorRing( ring.clone() );

  wkb = poly.asWkb( QgsAbstractGeometry::FlagExportTrianglesAsPolygons );
  QCOMPARE( wkb.size(), poly.wkbSize() );

  QgsConstWkbPtr wkbPtrPl( wkb );
  pResult.fromWkb( wkbPtrPl );
  QCOMPARE( pResult.asWkt(), "Polygon ((0 0, 0 10, 10 10, 10 0, 0 0))" );
  QCOMPARE( pResult.wkbType(), QgsWkbTypes::Polygon );


  // invalid multi ring
  // ba is equivalent to "Triangle((0 0, 0 5, 5 5, 0 0), (2 2, 2 4, 3 3, 2 2))"
  QByteArray ba = QByteArray::fromHex( "01110000000200000004000000000000000000000000000000000000000000000000000000000000000000144000000000000014400000000000001440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000010400000000000000840000000000000084000000000000000400000000000000040" );
  QgsTriangle tInvalidWkb;
  QgsConstWkbPtr wkbMultiRing( ba );
  QVERIFY( !tInvalidWkb.fromWkb( wkbMultiRing ) );
  QCOMPARE( tInvalidWkb, QgsTriangle() );


}

void TestQgsTriangle::exportImport()
{
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
}

void TestQgsTriangle::vertexAt()
{
  QgsTriangle tr( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );

  QCOMPARE( QgsTriangle().vertexAt( 0 ), QgsPoint() );
  QCOMPARE( QgsTriangle().vertexAt( -1 ), QgsPoint() );

  QVERIFY( tr.vertexAt( -1 ).isEmpty() );
  QCOMPARE( tr.vertexAt( 0 ), QgsPoint( 10, 10 ) );
  QCOMPARE( tr.vertexAt( 1 ), QgsPoint( 16, 10 ) );
  QCOMPARE( tr.vertexAt( 2 ), QgsPoint( 13, 15.1962 ) );
  QCOMPARE( tr.vertexAt( 3 ), QgsPoint( 10, 10 ) );
  QVERIFY( tr.vertexAt( 4 ).isEmpty() );
}

void TestQgsTriangle::moveVertex()
{
  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 100, 100 ), QgsPoint( 0, 200 ) );

  QgsPoint pt1( 5, 5 );
  QgsVertexId id( 0, 0, 1 );

  // empty triangle
  QVERIFY( !QgsTriangle().moveVertex( id, pt1 ) );

  // invalid part
  id.part = -1;
  QVERIFY( !tr.moveVertex( id, pt1 ) );
  id.part = 1;
  QVERIFY( !tr.moveVertex( id, pt1 ) );

  // invalid ring
  id.part = 0;
  id.ring = -1;
  QVERIFY( !tr.moveVertex( id, pt1 ) );
  id.ring = 1;
  QVERIFY( !tr.moveVertex( id, pt1 ) );
  id.ring = 0;
  id.vertex = -1;
  QVERIFY( !tr.moveVertex( id, pt1 ) );
  id.vertex = 5;
  QVERIFY( !tr.moveVertex( id, pt1 ) );

  // valid vertex
  id.vertex = 0;
  QVERIFY( tr.moveVertex( id, pt1 ) );
  QCOMPARE( tr.asWkt(), QString( "Triangle ((5 5, 100 100, 0 200, 5 5))" ) );

  pt1 = QgsPoint( 0, 0 );
  QVERIFY( tr.moveVertex( id, pt1 ) );
  QCOMPARE( tr.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );

  id.vertex = 4;
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( tr.moveVertex( id, pt1 ) );
  QCOMPARE( tr.asWkt(), QString( "Triangle ((5 5, 100 100, 0 200, 5 5))" ) );

  pt1 = QgsPoint( 0, 0 );
  QVERIFY( tr.moveVertex( id, pt1 ) );
  QCOMPARE( tr.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );

  id.vertex = 1;
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( tr.moveVertex( id, pt1 ) );
  QCOMPARE( tr.asWkt(), QString( "Triangle ((0 0, 5 5, 0 200, 0 0))" ) );

  // colinear
  pt1 = QgsPoint( 0, 100 );
  QVERIFY( tr.moveVertex( id, pt1 ) );
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( tr.moveVertex( id, pt1 ) );

  // duplicate point
  pt1 = QgsPoint( 0, 0 );
  QVERIFY( tr.moveVertex( id, pt1 ) );
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( tr.moveVertex( id, pt1 ) );
}

void TestQgsTriangle::deleteVertex()
{
  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 100, 100 ), QgsPoint( 0, 200 ) );

  std::unique_ptr< QgsLineString > ring( new QgsLineString() );
  ring->setPoints( QgsPointSequence() << QgsPoint( 5, 5 ) << QgsPoint( 50, 50 )
                   << QgsPoint( 0, 25 ) << QgsPoint( 5, 5 ) );

  tr.addInteriorRing( ring.release() );
  QCOMPARE( tr.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );

  /* QList<QgsCurve *> lc;
   lc.append(ext);
   t11.setInteriorRings( lc );
   QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );*/

  QgsVertexId id( 0, 0, 1 );
  QVERIFY( !tr.deleteVertex( id ) );
  QCOMPARE( tr.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );
  QVERIFY( !tr.insertVertex( id, QgsPoint( 5, 5 ) ) );
  QCOMPARE( tr.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );
}

void TestQgsTriangle::types()
{
  // Empty triangle returns always false for the types, except isDegenerate
  QVERIFY( QgsTriangle().isDegenerate() );
  QVERIFY( !QgsTriangle().isRight() );
  QVERIFY( !QgsTriangle().isIsocele() );
  QVERIFY( !QgsTriangle().isScalene() );
  QVERIFY( !QgsTriangle().isEquilateral() );

  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );

  QVERIFY( !tr.isDegenerate() );
  QVERIFY( tr.isRight() );
  QVERIFY( tr.isIsocele() );
  QVERIFY( !tr.isScalene() );
  QVERIFY( !tr.isEquilateral() );

  tr = QgsTriangle( QgsPoint( 7.2825, 4.2368 ), QgsPoint( 13.0058, 3.3218 ),
                    QgsPoint( 9.2145, 6.5242 ) );
  // angles in radians 58.8978;31.1036;89.9985
  // length 5.79598;4.96279;2.99413
  QVERIFY( !tr.isDegenerate() );
  QVERIFY( tr.isRight() );
  QVERIFY( !tr.isIsocele() );
  QVERIFY( tr.isScalene() );
  QVERIFY( !tr.isEquilateral() );

  tr = QgsTriangle( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ),
                    QgsPoint( 13, 15.1962 ) );
  QVERIFY( !tr.isDegenerate() );
  QVERIFY( !tr.isRight() );
  QVERIFY( tr.isIsocele() );
  QVERIFY( !tr.isScalene() );
  QVERIFY( tr.isEquilateral() );
}

void TestQgsTriangle::equality()
{
  QgsTriangle tr1, tr2;

  QVERIFY( QgsTriangle() == QgsTriangle() ); // empty

  tr1 = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) );
  QVERIFY( QgsTriangle() != tr1 ); // empty
  QVERIFY( tr1 != QgsTriangle() ); // empty

  tr1 = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 0, 10 ) );
  QVERIFY( QgsTriangle() != tr1 );
  QVERIFY( tr1 != QgsTriangle() );

  tr1 = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 0, 10 ) );
  tr2 = QgsTriangle( QgsPoint( 0, 10 ), QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) );
  QVERIFY( tr1 != tr2 );
}

void TestQgsTriangle::angles()
{
  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );

  QVector<double> a_tested;
  QVector<double> angles = tr.angles();
  a_tested.append( M_PI / 4.0 );
  a_tested.append( M_PI / 2.0 );
  a_tested.append( M_PI / 4.0 );

  QGSCOMPARENEAR( a_tested.at( 0 ), angles.at( 0 ), 0.0001 );
  QGSCOMPARENEAR( a_tested.at( 1 ), angles.at( 1 ), 0.0001 );
  QGSCOMPARENEAR( a_tested.at( 2 ), angles.at( 2 ), 0.0001 );

  QVector<double> a_empty = QgsTriangle().angles();
  QVERIFY( a_empty.isEmpty() );

  // From issue #46370
  tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 1, sqrt( 3 ) ), QgsPoint( 2, 0 ) );
  angles = tr.angles();
  QGSCOMPARENEAR( angles.at( 0 ), M_PI / 3.0, 0.0001 );
  QGSCOMPARENEAR( angles.at( 1 ), M_PI / 3.0, 0.0001 );
  QGSCOMPARENEAR( angles.at( 2 ), M_PI / 3.0, 0.0001 );

  tr = QgsTriangle( QgsPoint( 2, 0 ), QgsPoint( 1, sqrt( 3 ) ), QgsPoint( 0, 0 ) );
  angles = tr.angles();
  QGSCOMPARENEAR( angles.at( 0 ), M_PI / 3.0, 0.0001 );
  QGSCOMPARENEAR( angles.at( 1 ), M_PI / 3.0, 0.0001 );
  QGSCOMPARENEAR( angles.at( 2 ), M_PI / 3.0, 0.0001 );

  tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 3 ), QgsPoint( 4, 0 ) );
  angles = tr.angles();
  QGSCOMPARENEAR( angles.at( 0 ), M_PI / 2.0, 0.0001 );
  QGSCOMPARENEAR( angles.at( 1 ), 0.9272952, 0.0001 );
  QGSCOMPARENEAR( angles.at( 2 ), 0.6435011, 0.0001 );
  tr = QgsTriangle( QgsPoint( 4, 0 ), QgsPoint( 0, 3 ), QgsPoint( 0, 0 ) );
  angles = tr.angles();
  QGSCOMPARENEAR( angles.at( 0 ), 0.6435011, 0.0001 );
  QGSCOMPARENEAR( angles.at( 1 ), 0.9272952, 0.0001 );
  QGSCOMPARENEAR( angles.at( 2 ), M_PI / 2.0, 0.0001 );

  tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 1, 3 ), QgsPoint( 3, 0 ) );
  angles = tr.angles();
  QGSCOMPARENEAR( angles.at( 0 ), 1.2490457, 0.0001 );
  QGSCOMPARENEAR( angles.at( 1 ), 0.9097531, 0.0001 );
  QGSCOMPARENEAR( angles.at( 2 ), 0.9827937, 0.0001 );
  tr = QgsTriangle( QgsPoint( 3, 0 ), QgsPoint( 1, 3 ), QgsPoint( 0, 0 ) );
  angles = tr.angles();
  QGSCOMPARENEAR( angles.at( 0 ), 0.9827937, 0.0001 );
  QGSCOMPARENEAR( angles.at( 1 ), 0.9097531, 0.0001 );
  QGSCOMPARENEAR( angles.at( 2 ), 1.2490457, 0.0001 );

  tr = QgsTriangle( QgsPoint( 78598.328125, 330538.375, 0 ), QgsPoint( 78606.3203125, 330544, 0 ), QgsPoint( 78601.46875, 330550.90625, 0 ) );
  angles = tr.angles();
  QGSCOMPARENEAR( angles.at( 0 ), 0.7119510, 0.0001 );
  QGSCOMPARENEAR( angles.at( 1 ), 1.5716821, 0.0001 );
  QGSCOMPARENEAR( angles.at( 2 ), 0.8579596, 0.0001 );
}

void TestQgsTriangle::lengths()
{
  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );

  QVector<double> l_tested, l_t7 = tr.lengths();
  l_tested.append( 5 );
  l_tested.append( 5 );
  l_tested.append( std::sqrt( 5 * 5 + 5 * 5 ) );

  QGSCOMPARENEAR( l_tested.at( 0 ), l_t7.at( 0 ), 0.0001 );
  QGSCOMPARENEAR( l_tested.at( 1 ), l_t7.at( 1 ), 0.0001 );
  QGSCOMPARENEAR( l_tested.at( 2 ), l_t7.at( 2 ), 0.0001 );

  QVector<double> l_empty = QgsTriangle().lengths();
  QVERIFY( l_empty.isEmpty() );
}

void TestQgsTriangle::orthocenter()
{
  QgsTriangle tr( QgsPoint( 20, 2 ), QgsPoint( 16, 6 ), QgsPoint( 26, 2 ) );
  QVERIFY( QgsTriangle().orthocenter().isEmpty() );
  QCOMPARE( QgsPoint( 16, -8 ), tr.orthocenter() );

  tr = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );
  QCOMPARE( QgsPoint( 0, 5 ), tr.orthocenter() );

  tr = QgsTriangle( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), tr.orthocenter(), 0.0001 );
}

void TestQgsTriangle::altitudes()
{
  QgsTriangle tr( QgsPoint( 20, 2 ), QgsPoint( 16, 6 ), QgsPoint( 26, 2 ) );

  QVector<QgsLineString> alt = QgsTriangle().altitudes();
  QVERIFY( alt.isEmpty() );

  alt = tr.altitudes();
  QGSCOMPARENEARPOINT( alt.at( 0 ).pointN( 1 ), QgsPoint( 20.8276, 4.0690 ), 0.0001 );
  QGSCOMPARENEARPOINT( alt.at( 1 ).pointN( 1 ), QgsPoint( 16, 2 ), 0.0001 );
  QGSCOMPARENEARPOINT( alt.at( 2 ).pointN( 1 ), QgsPoint( 23, -1 ), 0.0001 );
}

void TestQgsTriangle::medians()
{
  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );

  QVector<QgsLineString> med = QgsTriangle().medians();
  QVERIFY( med.isEmpty() );

  med = tr.medians();

  QCOMPARE( med.at( 0 ).pointN( 0 ), tr.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), QgsPoint( 2.5, 5 ), 0.0001 );
  QCOMPARE( med.at( 1 ).pointN( 0 ), tr.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), QgsPoint( 2.5, 2.5 ), 0.0001 );
  QCOMPARE( med.at( 2 ).pointN( 0 ), tr.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), QgsPoint( 0, 2.5 ), 0.0001 );

  med.clear();

  tr = QgsTriangle( QgsPoint( 20, 2 ), QgsPoint( 16, 6 ), QgsPoint( 26, 2 ) );
  med = tr.medians();
  QVector<QgsLineString> alt = tr.altitudes();

  QCOMPARE( med.at( 0 ).pointN( 0 ), tr.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), QgsPoint( 21, 4 ), 0.0001 );
  QCOMPARE( med.at( 1 ).pointN( 0 ), tr.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), QgsPoint( 23, 2 ), 0.0001 );
  QCOMPARE( med.at( 2 ).pointN( 0 ), tr.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), QgsPoint( 18, 4 ), 0.0001 );

  med.clear();
  alt.clear();

  tr = QgsTriangle( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );
  med = tr.medians();
  alt = tr.altitudes();

  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 0 ), alt.at( 0 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), alt.at( 0 ).pointN( 1 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 0 ), alt.at( 1 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), alt.at( 1 ).pointN( 1 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 0 ), alt.at( 2 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), alt.at( 2 ).pointN( 1 ), 0.0001 );
}

void TestQgsTriangle::medial()
{
  QCOMPARE( QgsTriangle().medial(), QgsTriangle() );

  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );
  QCOMPARE( tr.medial(),
            QgsTriangle( QgsPoint( 0, 2.5 ), QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 2.5 ) ) );

  tr = QgsTriangle( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );
  QCOMPARE( tr.medial(),
            QgsTriangle( QgsGeometryUtils::midpoint( tr.vertexAt( 0 ), tr.vertexAt( 1 ) ),
                         QgsGeometryUtils::midpoint( tr.vertexAt( 1 ), tr.vertexAt( 2 ) ),
                         QgsGeometryUtils::midpoint( tr.vertexAt( 2 ), tr.vertexAt( 0 ) ) ) );
}

void TestQgsTriangle::bisectors()
{
  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );

  QVector<QgsLineString> bis = QgsTriangle().bisectors();
  QVERIFY( bis.isEmpty() );

  bis = tr.bisectors();
  QCOMPARE( bis.at( 0 ).pointN( 0 ), tr.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( bis.at( 0 ).pointN( 1 ), QgsPoint( 2.0711, 5 ), 0.0001 );
  QCOMPARE( bis.at( 1 ).pointN( 0 ), tr.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( bis.at( 1 ).pointN( 1 ), QgsPoint( 2.5, 2.5 ), 0.0001 );
  QCOMPARE( bis.at( 2 ).pointN( 0 ), tr.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( bis.at( 2 ).pointN( 1 ), QgsPoint( 0, 2.9289 ), 0.0001 );
}

void TestQgsTriangle::inscribedCircle()
{
  QCOMPARE( QgsTriangle().inscribedCircle(), QgsCircle() );
  QVERIFY( QgsTriangle().inscribedCenter().isEmpty() );
  QCOMPARE( 0.0, QgsTriangle().inscribedRadius() );

  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );
  QGSCOMPARENEARPOINT( QgsPoint( 1.4645, 3.5355 ),
                       tr.inscribedCenter(), 0.001 );
  QGSCOMPARENEAR( 1.4645, tr.inscribedRadius(), 0.0001 );

  tr = QgsTriangle( QgsPoint( 20, 2 ), QgsPoint( 16, 6 ), QgsPoint( 26, 2 ) );
  QGSCOMPARENEARPOINT( QgsPoint( 20.4433, 3.0701 ),
                       tr.inscribedCenter(), 0.001 );
  QGSCOMPARENEAR( 1.0701, tr.inscribedRadius(), 0.0001 );

  tr = QgsTriangle( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ),
                       tr.inscribedCenter(), 0.0001 );
  QGSCOMPARENEAR( 1.7321, tr.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ),
                       tr.inscribedCircle().center(), 0.0001 );
  QGSCOMPARENEAR( 1.7321, tr.inscribedCircle().radius(), 0.0001 );
}

void TestQgsTriangle::circumscribedCircle()
{
  QCOMPARE( QgsTriangle().circumscribedCircle(), QgsCircle() );
  QVERIFY( QgsTriangle().circumscribedCenter().isEmpty() );
  QCOMPARE( 0.0, QgsTriangle().circumscribedRadius() );

  QgsTriangle tr( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );
  QCOMPARE( QgsPoint( 2.5, 2.5 ), tr.circumscribedCenter() );
  QGSCOMPARENEAR( 3.5355, tr.circumscribedRadius(), 0.0001 );

  tr = QgsTriangle( QgsPoint( 20, 2 ), QgsPoint( 16, 6 ), QgsPoint( 26, 2 ) );
  QCOMPARE( QgsPoint( 23, 9 ), tr.circumscribedCenter() );
  QGSCOMPARENEAR( 7.6158, tr.circumscribedRadius(), 0.0001 );

  tr = QgsTriangle( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), tr.circumscribedCenter(), 0.0001 );
  QGSCOMPARENEAR( 3.4641, tr.circumscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), tr.circumscribedCircle().center(), 0.0001 );
  QGSCOMPARENEAR( 3.4641, tr.circumscribedCircle().radius(), 0.0001 );
}

void TestQgsTriangle::boundary()
{
  QVERIFY( !QgsTriangle().boundary() );

  std::unique_ptr< QgsCurve > boundary( QgsTriangle( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) ).boundary() );

  QCOMPARE( boundary->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( boundary->numPoints(), 4 );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 13, 3 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 9, 6 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 7, 4 ) );
}

QGSTEST_MAIN( TestQgsTriangle )
#include "testqgstriangle.moc"
