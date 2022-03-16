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
    void constructor();
    void assignment();
    void clone();
    void copy();
    void clear();
    void addGeometry();
    void addBadGeometry();
    void addGeometryInitialDimension();
    void addGeometryZ();
    void addGeometryM();
    void addGeometryZM();
    void insertGeometry();
    void cast();
    void boundary();
    void centroid();
    void removeDuplicateNodes();
    void vertexIterator();
    void toCurveType();
    void toFromWKB();
    void toFromWkbZM();
    void toFromWKT();
    void exportImport();
};


void TestQgsMultiPolygon::constructor()
{
  QgsMultiPolygon mp;
  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.nCoordinates(), 0 );
  QCOMPARE( mp.ringCount(), 0 );
  QCOMPARE( mp.partCount(), 0 );
  QVERIFY( !mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPolygon" ) );
  QCOMPARE( mp.geometryType(), QString( "MultiPolygon" ) );
  QCOMPARE( mp.dimension(), 0 );
  QVERIFY( !mp.hasCurvedSegments() );
  QCOMPARE( mp.area(), 0.0 );
  QCOMPARE( mp.perimeter(), 0.0 );
  QCOMPARE( mp.numGeometries(), 0 );
  QVERIFY( !mp.geometryN( 0 ) );
  QVERIFY( !mp.geometryN( -1 ) );
  QCOMPARE( mp.vertexCount( 0, 0 ), 0 );
  QCOMPARE( mp.vertexCount( 0, 1 ), 0 );
  QCOMPARE( mp.vertexCount( 1, 0 ), 0 );
}

void TestQgsMultiPolygon::assignment()
{
  QgsMultiPolygon mp1;
  QgsMultiPolygon mp2;

  mp1 = mp2;
  QCOMPARE( mp1.numGeometries(), 0 );

  QgsPolygon part;
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 15 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );

  QgsMultiPolygon mp3;
  mp3.addGeometry( part.clone() );
  mp3.addGeometry( part.clone() );

  mp1 = mp3;
  QCOMPARE( mp1.numGeometries(), 2 );

  const QgsPolygon *ls = static_cast< const QgsPolygon * >( mp1.geometryN( 0 ) );
  QCOMPARE( *ls, part );

  ls = static_cast< const QgsPolygon * >( mp1.geometryN( 1 ) );
  QCOMPARE( *ls, part );
}

void TestQgsMultiPolygon::clone()
{
  QgsMultiPolygon mp;
  std::unique_ptr< QgsMultiPolygon >cloned( mp.clone() );

  QVERIFY( cloned->isEmpty() );

  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 15 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );
  mp.addGeometry( part.clone() );

  cloned.reset( mp.clone() );

  QCOMPARE( cloned->numGeometries(), 2 );

  const QgsPolygon *ls = static_cast< const QgsPolygon * >( cloned->geometryN( 0 ) );
  QCOMPARE( *ls, part );

  ls = static_cast< const QgsPolygon * >( cloned->geometryN( 1 ) );
  QCOMPARE( *ls, part );
}

void TestQgsMultiPolygon::copy()
{
  QgsMultiPolygon mp1;
  QgsMultiPolygon mp2( mp1 );

  QVERIFY( mp2.isEmpty() );

  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 15 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );
  mp1.addGeometry( part.clone() );

  QgsMultiPolygon mp3( mp1 );

  QCOMPARE( mp3.numGeometries(), 2 );
  QCOMPARE( mp3.wkbType(), QgsWkbTypes::MultiPolygonZM );

  const QgsPolygon *ls = static_cast< const QgsPolygon * >( mp3.geometryN( 0 ) );
  QCOMPARE( *ls, part );

  ls = static_cast< const QgsPolygon * >( mp3.geometryN( 1 ) );
  QCOMPARE( *ls, part );
}

void TestQgsMultiPolygon::clear()
{
  QgsMultiPolygon mp;
  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 15 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.numGeometries(), 2 );

  mp.clear();

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );
  QCOMPARE( mp.nCoordinates(), 0 );
  QCOMPARE( mp.ringCount(), 0 );
  QCOMPARE( mp.partCount(), 0 );
  QVERIFY( !mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );
}

void TestQgsMultiPolygon::addGeometry()
{
  QgsMultiPolygon mp;
  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 )
                  << QgsPoint( 2, 21 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QVERIFY( !mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 1 );
  QCOMPARE( mp.nCoordinates(), 4 );
  QCOMPARE( mp.ringCount(), 1 );
  QCOMPARE( mp.partCount(), 1 );
  QVERIFY( !mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPolygon" ) );
  QCOMPARE( mp.geometryType(), QString( "MultiPolygon" ) );
  QCOMPARE( mp.dimension(), 2 );
  QVERIFY( !mp.hasCurvedSegments() );
  QVERIFY( mp.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp.geometryN( 0 ) ), part );
  QVERIFY( !mp.geometryN( 100 ) );
  QVERIFY( !mp.geometryN( -1 ) );
  QCOMPARE( mp.vertexCount( 0, 0 ), 4 );
  QCOMPARE( mp.vertexCount( 1, 0 ), 0 );

  //add another part
  mp.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 )
                  << QgsPoint( 10, 21 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.vertexCount( 0, 0 ), 4 );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 )
                  << QgsPoint( 4, 17 ) << QgsPoint( 9, 12 ) );
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.vertexCount( 1, 0 ), 4 );
  QCOMPARE( mp.numGeometries(), 2 );
  QVERIFY( mp.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of parts type
  mp.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 )
                  << QgsPoint( 4, 17 ) << QgsPoint( 9, 12 ) );
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 )
                  << QgsPoint( 10, 13, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( mp.vertexCount( 0, 0 ), 4 );
  QCOMPARE( mp.vertexCount( 1, 0 ), 4 );
  QCOMPARE( mp.vertexCount( 2, 0 ), 0 );
  QCOMPARE( mp.vertexCount( -1, 0 ), 0 );
  QCOMPARE( mp.nCoordinates(), 8 );
  QCOMPARE( mp.ringCount(), 1 );
  QCOMPARE( mp.partCount(), 2 );
  QVERIFY( !mp.is3D() );

  const QgsPolygon *ls = static_cast< const QgsPolygon * >( mp.geometryN( 0 ) );
  auto exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  // test polygonN by the same occasion
  QCOMPARE( *ls, *static_cast< const QgsPolygon * >( mp.polygonN( 0 ) ) );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( 3, 13 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( 4, 17 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( 9, 12 ) );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 1 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( *ls, *static_cast< const QgsPolygon * >( mp.polygonN( 1 ) ) );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( 2, 11 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( 10, 13 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( 1, 10 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 42, 61, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( mp.vertexCount( 0, 0 ), 4 );
  QCOMPARE( mp.vertexCount( 1, 0 ), 4 );
  QCOMPARE( mp.vertexCount( 2, 0 ), 4 );
  QCOMPARE( mp.nCoordinates(), 12 );
  QCOMPARE( mp.ringCount(), 1 );
  QCOMPARE( mp.partCount(), 3 );
  QVERIFY( !mp.is3D() );
  QVERIFY( !mp.isMeasure() );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 2 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( 32, 41 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( 42, 61 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( 21, 30 ) );
}

void TestQgsMultiPolygon::addBadGeometry()
{
  QgsMultiPolygon mp;

  //try with nullptr
  mp.addGeometry( nullptr );
  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.nCoordinates(), 0 );
  QCOMPARE( mp.ringCount(), 0 );
  QCOMPARE( mp.partCount(), 0 );
  QCOMPARE( mp.numGeometries(), 0 );
  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );
  QVERIFY( !mp.geometryN( 0 ) );
  QVERIFY( !mp.geometryN( -1 ) );

  // not a surface
  QVERIFY( !mp.addGeometry( new QgsPoint() ) );
  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.nCoordinates(), 0 );
  QCOMPARE( mp.ringCount(), 0 );
  QCOMPARE( mp.partCount(), 0 );
  QCOMPARE( mp.numGeometries(), 0 );
  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );
  QVERIFY( !mp.geometryN( 0 ) );
  QVERIFY( !mp.geometryN( -1 ) );
}

void TestQgsMultiPolygon::addGeometryInitialDimension()
{
  QgsMultiPolygon mp;
  QgsPolygon part;
  QgsLineString ring;

  //initial adding of geometry should set z/m type
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 30, 31, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) );
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QVERIFY( mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPolygonZ" ) );
  QCOMPARE( mp.geometryType(), QString( "MultiPolygon" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( mp.geometryN( 0 ) ) ), part );

  mp.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 30, 31, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) );
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QVERIFY( !mp.is3D() );
  QVERIFY( mp.isMeasure() );
  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonM );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPolygonM" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( mp.geometryN( 0 ) ) ), part );

  mp.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 30, 31, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) );
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QVERIFY( mp.is3D() );
  QVERIFY( mp.isMeasure() );
  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( mp.geometryN( 0 ) ) ), part );
}

void TestQgsMultiPolygon::addGeometryZ()
{
  QgsMultiPolygon mp;
  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 )
                  << QgsPoint( 9, 15, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZ );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 )
                  << QgsPoint( 7, 34 ) << QgsPoint( 2, 20 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QVERIFY( mp.is3D() );

  const QgsPolygon *ls = static_cast< const QgsPolygon * >( mp.geometryN( 0 ) );
  auto exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( 9, 15, 3 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( 1, 10, 2 ) );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 1 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( 7, 34, 0 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( 2, 20, 0 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 65, 0, 7 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QVERIFY( mp.is3D() );
  QVERIFY( !mp.isMeasure() );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 2 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( 9, 65, 0 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( 5, 50, 0 ) );
}

void TestQgsMultiPolygon::addGeometryM()
{
  QgsMultiPolygon mp;
  QgsPolygon part;
  QgsLineString ring;

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygon );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 76, 0, 8 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonM );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 )
                  << QgsPoint( 7, 39 ) << QgsPoint( 2, 20 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonM );
  QVERIFY( mp.isMeasure() );

  const QgsPolygon *ls = static_cast< const QgsPolygon * >( mp.geometryN( 0 ) );
  auto exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 9, 76, 0, 8 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 1 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 7, 39, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 )
                  << QgsPoint( 24, 21, 5 ) << QgsPoint( 11, 12, 13 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonM );
  QVERIFY( !mp.is3D() );
  QVERIFY( mp.isMeasure() );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 2 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 24, 21, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
}

void TestQgsMultiPolygon::addGeometryZM()
{
  QgsMultiPolygon mp;
  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 9 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZM );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 13, 27 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( mp.isMeasure() );
  QVERIFY( mp.is3D() );

  const QgsPolygon *ls = static_cast< const QgsPolygon * >( mp.geometryN( 0 ) );
  auto exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 9 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 1 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 13, 27, 0, 0 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 )
                  << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 )
                  << QgsPoint( QgsWkbTypes::PointZ, 93, 85, 10 )
                  << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( mp.is3D() );
  QVERIFY( mp.isMeasure() );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 2 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 93, 85, 10, 0 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 )
                  << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 )
                  << QgsPoint( QgsWkbTypes::PointM, 185, 193, 0, 13 )
                  << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( mp.is3D() );
  QVERIFY( mp.isMeasure() );

  ls = static_cast< const QgsPolygon * >( mp.geometryN( 3 ) );
  exteriorRing = static_cast< const QgsLineString *>( ls->exteriorRing() );

  QCOMPARE( exteriorRing->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( exteriorRing->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
  QCOMPARE( exteriorRing->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 185, 193, 0, 13 ) );
  QCOMPARE( exteriorRing->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
}

void TestQgsMultiPolygon::insertGeometry()
{
  QgsMultiPolygon mp;

  mp.insertGeometry( nullptr, 0 );
  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );

  mp.insertGeometry( nullptr, -1 );
  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );

  mp.insertGeometry( nullptr, 100 );
  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );

  mp.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );

  QgsPolygon part;
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 21 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.setExteriorRing( ring.clone() );

  mp.insertGeometry( part.clone(), 0 );
  QVERIFY( !mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 1 );
}

void TestQgsMultiPolygon::cast()
{
  QVERIFY( !QgsMultiPolygon().cast( nullptr ) );

  QgsMultiPolygon mp;
  QVERIFY( QgsMultiPolygon().cast( &mp ) );

  mp.clear();

  mp.fromWkt( QStringLiteral( "MultiPolygonZ()" ) );
  QVERIFY( QgsMultiPolygon().cast( &mp ) );

  mp.fromWkt( QStringLiteral( "MultiPolygonM()" ) );
  QVERIFY( QgsMultiPolygon().cast( &mp ) );

  mp.fromWkt( QStringLiteral( "MultiPolygonZM()" ) );
  QVERIFY( QgsMultiPolygon().cast( &mp ) );
}

void TestQgsMultiPolygon::boundary()
{
  QgsMultiPolygon mp;
  QVERIFY( !mp.boundary() );

  QgsLineString ring;
  QgsPolygon part;
  ring.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                  << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  part.clear();
  mp.addGeometry( part.clone() );

  std::unique_ptr< QgsAbstractGeometry > boundary( mp.boundary() );
  QgsMultiLineString *mls = dynamic_cast< QgsMultiLineString * >( boundary.get() );

  QVERIFY( mls );
  QCOMPARE( mls->numGeometries(), 1 );

  auto ls = qgis::down_cast< QgsLineString * >( mls->geometryN( 0 ) );
  QCOMPARE( ls->numPoints(), 4 );
  QCOMPARE( ls->xAt( 0 ), 0.0 );
  QCOMPARE( ls->xAt( 1 ), 1.0 );
  QCOMPARE( ls->xAt( 2 ), 1.0 );
  QCOMPARE( ls->xAt( 3 ), 0.0 );
  QCOMPARE( ls->yAt( 0 ), 0.0 );
  QCOMPARE( ls->yAt( 1 ), 0.0 );
  QCOMPARE( ls->yAt( 2 ), 1.0 );
  QCOMPARE( ls->yAt( 3 ), 0.0 );

  // add polygon with interior rings
  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 )
                  << QgsPoint( 11, 11 )  << QgsPoint( 10, 10 ) );
  part.setExteriorRing( ring.clone() );

  QgsLineString boundaryRing1;
  boundaryRing1.setPoints( QgsPointSequence() << QgsPoint( 10.1, 10.1 ) << QgsPoint( 10.2, 10.1 )
                           << QgsPoint( 10.2, 10.2 )  << QgsPoint( 10.1, 10.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QgsPointSequence() << QgsPoint( 10.8, 10.8 ) << QgsPoint( 10.9, 10.8 )
                           << QgsPoint( 10.9, 10.9 )  << QgsPoint( 10.8, 10.8 ) );
  part.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  mp.addGeometry( part.clone() );

  boundary.reset( mp.boundary() );
  mls = static_cast< QgsMultiLineString * >( boundary.get() );

  QVERIFY( mls );
  QCOMPARE( mls->numGeometries(), 4 );

  ls = qgis::down_cast< QgsLineString * >( mls->geometryN( 0 ) );
  QCOMPARE( ls->numPoints(), 4 );
  QCOMPARE( ls->xAt( 0 ), 0.0 );
  QCOMPARE( ls->xAt( 1 ), 1.0 );
  QCOMPARE( ls->xAt( 2 ), 1.0 );
  QCOMPARE( ls->xAt( 3 ), 0.0 );
  QCOMPARE( ls->yAt( 0 ), 0.0 );
  QCOMPARE( ls->yAt( 1 ), 0.0 );
  QCOMPARE( ls->yAt( 2 ), 1.0 );
  QCOMPARE( ls->yAt( 3 ), 0.0 );

  ls = qgis::down_cast< QgsLineString * >( mls->geometryN( 1 ) );
  QCOMPARE( ls->numPoints(), 4 );
  QCOMPARE( ls->xAt( 0 ), 10.0 );
  QCOMPARE( ls->xAt( 1 ), 11.0 );
  QCOMPARE( ls->xAt( 2 ), 11.0 );
  QCOMPARE( ls->xAt( 3 ), 10.0 );
  QCOMPARE( ls->yAt( 0 ), 10.0 );
  QCOMPARE( ls->yAt( 1 ), 10.0 );
  QCOMPARE( ls->yAt( 2 ), 11.0 );
  QCOMPARE( ls->yAt( 3 ), 10.0 );

  ls = qgis::down_cast< QgsLineString * >( mls->geometryN( 2 ) );
  QCOMPARE( ls->numPoints(), 4 );
  QCOMPARE( ls->xAt( 0 ), 10.1 );
  QCOMPARE( ls->xAt( 1 ), 10.2 );
  QCOMPARE( ls->xAt( 2 ), 10.2 );
  QCOMPARE( ls->xAt( 3 ), 10.1 );
  QCOMPARE( ls->yAt( 0 ), 10.1 );
  QCOMPARE( ls->yAt( 1 ), 10.1 );
  QCOMPARE( ls->yAt( 2 ), 10.2 );
  QCOMPARE( ls->yAt( 3 ), 10.1 );

  ls = qgis::down_cast< QgsLineString * >( mls->geometryN( 3 ) );
  QCOMPARE( ls->numPoints(), 4 );
  QCOMPARE( ls->xAt( 0 ), 10.8 );
  QCOMPARE( ls->xAt( 1 ), 10.9 );
  QCOMPARE( ls->xAt( 2 ), 10.9 );
  QCOMPARE( ls->xAt( 3 ), 10.8 );
  QCOMPARE( ls->yAt( 0 ), 10.8 );
  QCOMPARE( ls->yAt( 1 ), 10.8 );
  QCOMPARE( ls->yAt( 2 ), 10.9 );
  QCOMPARE( ls->yAt( 3 ), 10.8 );
}

void TestQgsMultiPolygon::centroid()
{
  // test centroid of empty multipolygon
  QgsMultiPolygon mp;
  QCOMPARE( mp.centroid().asWkt(), QStringLiteral( "Point EMPTY" ) );
}

void TestQgsMultiPolygon::removeDuplicateNodes()
{
  QgsMultiPolygon mp;

  QVERIFY( mp.fromWkt( QStringLiteral( "MultiPolygonZ (((0 0 0, 10 10 0, 11 9 0, 9 8 0, 9 8 0, 1 -1 0, 0 0 0)),((7 -1 0, 12 7 0, 13 6 0, 13 6 0, 8 -3 0, 7 -1 0)))" ) ) );
  QCOMPARE( mp.numGeometries(), 2 );

  // First call should remove all duplicate nodes (one per part)
  QVERIFY( mp.removeDuplicateNodes( 0.001, false ) );
  QVERIFY( !mp.removeDuplicateNodes( 0.001, false ) );
  QCOMPARE( mp.asWkt(), QStringLiteral( "MultiPolygonZ (((0 0 0, 10 10 0, 11 9 0, 9 8 0, 1 -1 0, 0 0 0)),((7 -1 0, 12 7 0, 13 6 0, 8 -3 0, 7 -1 0)))" ) );
}

void TestQgsMultiPolygon::vertexIterator()
{
  QgsMultiPolygon mp;
  QgsLineString ring;
  QgsPolygon part;

  ring.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                  << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  part.setExteriorRing( ring.clone() );

  mp.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 )
                  << QgsPoint( 11, 11 )  << QgsPoint( 10, 10 ) );
  part.setExteriorRing( ring.clone() );

  QgsLineString boundaryRing1;
  boundaryRing1.setPoints( QgsPointSequence() << QgsPoint( 10.1, 10.1 ) << QgsPoint( 10.2, 10.1 )
                           << QgsPoint( 10.2, 10.2 )  << QgsPoint( 10.1, 10.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QgsPointSequence() << QgsPoint( 10.8, 10.8 ) << QgsPoint( 10.9, 10.8 )
                           << QgsPoint( 10.9, 10.9 )  << QgsPoint( 10.8, 10.8 ) );
  part.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  mp.addGeometry( part.clone() );

  // vertex iterator: 2 polygons (one with just exterior ring, other with two interior rings)
  QgsAbstractGeometry::vertex_iterator it = mp.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = mp.vertices_end();

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
  QgsVertexIterator it2( &mp );
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
}

void TestQgsMultiPolygon::toCurveType()
{
  QgsMultiPolygon mp;

  QgsPolygon part;
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 15 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );
  mp.addGeometry( part.clone() );

  std::unique_ptr< QgsMultiSurface > curveType( mp.toCurveType() );

  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( curveType->numGeometries(), 2 );

  QCOMPARE( *static_cast< const QgsPolygon * >( curveType->geometryN( 0 ) ),
            *static_cast< const QgsPolygon * >( mp.geometryN( 0 ) ) );

  QCOMPARE( *static_cast< const QgsPolygon * >( curveType->geometryN( 1 ) ),
            *static_cast< const QgsPolygon * >( mp.geometryN( 1 ) ) );
}

void TestQgsMultiPolygon::toFromWKB()
{
  QgsMultiPolygon mp1;
  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 9, 27 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 )
                  << QgsPoint( QgsWkbTypes::Point, 43, 43 )
                  << QgsPoint( QgsWkbTypes::Point, 29, 39 )
                  << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  QByteArray wkb = mp1.asWkb();
  QCOMPARE( wkb.size(), mp1.wkbSize() );

  QgsMultiPolygon mp2;
  QgsConstWkbPtr wkbPtr1( wkb );
  mp2.fromWkb( wkbPtr1 );

  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 0 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 1 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  mp2.clear();

  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !mp2.fromWkb( nullPtr ) );
  QCOMPARE( mp2.wkbType(), QgsWkbTypes::MultiPolygon );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );

  QVERIFY( !mp2.fromWkb( wkbPointPtr ) );
  QCOMPARE( mp2.wkbType(), QgsWkbTypes::MultiPolygon );
}

void TestQgsMultiPolygon::toFromWkbZM()
{
  QgsMultiPolygon mp1;
  QgsPolygon part;
  QgsLineString ring;

  //parts with Z
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 )
                  << QgsPoint( QgsWkbTypes::PointZ, 9, 27, 5 )
                  << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 )
                  << QgsPoint( QgsWkbTypes::PointZ, 87, 54, 7 )
                  << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  QgsMultiPolygon mp2;
  QByteArray wkb = mp1.asWkb();
  QgsConstWkbPtr wkbPtr1( wkb );
  mp2.fromWkb( wkbPtr1 );

  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( mp2.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 0 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 1 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 1 ) ) );

  //parts with m
  mp1.clear();
  mp2.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 21, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 37, 31, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  wkb = mp1.asWkb();
  QgsConstWkbPtr wkbPtr2( wkb );
  mp2.fromWkb( wkbPtr2 );

  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( mp2.wkbType(), QgsWkbTypes::MultiPolygonM );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 0 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 1 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 1 ) ) );

  // parts with ZM
  mp1.clear();
  mp2.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 19, 13, 5, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  wkb = mp1.asWkb();
  QgsConstWkbPtr wkbPtr3( wkb );
  mp2.fromWkb( wkbPtr3 );

  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( mp2.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 0 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 1 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 1 ) ) );
}

void TestQgsMultiPolygon::toFromWKT()
{
  QgsMultiPolygon mp1;
  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 13, 19, 3, 10 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 17, 49, 31, 53 )
                  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp1.addGeometry( part.clone() );

  QString wkt = mp1.asWkt();
  QVERIFY( !wkt.isEmpty() );

  QgsMultiPolygon mp2;

  QVERIFY( mp2.fromWkt( wkt ) );
  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 0 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( mp2.geometryN( 1 ) ),
            *static_cast< const QgsPolygon * >( mp1.geometryN( 1 ) ) );

  //bad WKT
  QVERIFY( !mp1.fromWkt( "Point()" ) );
  QVERIFY( mp1.isEmpty() );
  QCOMPARE( mp1.numGeometries(), 0 );
  QCOMPARE( mp1.wkbType(), QgsWkbTypes::MultiPolygon );
}

void TestQgsMultiPolygon::exportImport()
{
  //as JSON
  QgsMultiPolygon mp;
  QgsPolygon part;
  QgsLineString ring;

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 21 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 )
                  << QgsPoint( QgsWkbTypes::Point, 43, 43 )
                  << QgsPoint( QgsWkbTypes::Point, 41, 39 )
                  << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 3,13 7,21 7,17</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 43,43 41,39 27,37</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  QString res = elemToString( mp.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );

  QString expectedGML2empty( QStringLiteral( "<MultiPolygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPolygon().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiSurface xmlns=\"gml\"><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13 7 21 7 17</posList></LinearRing></exterior></Polygon></surfaceMember><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43 41 39 27 37</posList></LinearRing></exterior></Polygon></surfaceMember></MultiSurface>" ) );
  res = elemToString( mp.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );

  QString expectedGML3empty( QStringLiteral( "<MultiSurface xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPolygon().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[[[7.0,17.0],[3.0,13.0],[7.0,21.0],[7.0,17.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]]]],\"type\":\"MultiPolygon\"}" );
  res = mp.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 17, 27 )
                  << QgsPoint( QgsWkbTypes::Point, 18, 28 )
                  << QgsPoint( QgsWkbTypes::Point, 19, 37 )
                  << QgsPoint( QgsWkbTypes::Point, 17, 27 ) ) ;
  part.addInteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QString expectedJsonWithRings( "{\"coordinates\":[[[[7.0,17.0],[3.0,13.0],[7.0,21.0],[7.0,17.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]],[[17.0,27.0],[18.0,28.0],[19.0,37.0],[17.0,27.0]]]],\"type\":\"MultiPolygon\"}" );
  res = mp.asJson( 1 );
  QCOMPARE( res, expectedJsonWithRings );

  mp.clear();

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 8 / 3.0, 27 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  part.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 )
                  << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) ) ;
  part.setExteriorRing( ring.clone() );
  mp.addGeometry( part.clone() );

  QString expectedJsonPrec3( "{\"coordinates\":[[[[2.333,5.667],[0.6,4.333],[2.667,9.0],[2.333,5.667]]],[[[9.0,4.111],[1.049,1.024],[9.0,4.111]]]],\"type\":\"MultiPolygon\"}" );
  res = mp.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 0.6,4.333 2.667,9 2.333,5.667</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 1.049,1.024 9,4.111</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" );
  res = elemToString( mp.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( "<MultiSurface xmlns=\"gml\"><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333 2.667 9 2.333 5.667</posList></LinearRing></exterior></Polygon></surfaceMember><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024 9 4.111</posList></LinearRing></exterior></Polygon></surfaceMember></MultiSurface>" );
  res = elemToString( mp.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKmlPrec3( "<MultiGeometry><Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>2.333,5.667,0 0.6,4.333,0 2.667,9,0 2.333,5.667,0</coordinates></LinearRing></outerBoundaryIs></Polygon><Polygon><outerBoundaryIs><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>9,4.111,0 1.049,1.024,0 9,4.111,0</coordinates></LineString></outerBoundaryIs></Polygon></MultiGeometry>" );
  QCOMPARE( mp.asKml( 3 ), expectedKmlPrec3 );
}

QGSTEST_MAIN( TestQgsMultiPolygon )
#include "testqgsmultipolygon.moc"
