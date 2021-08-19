/***************************************************************************
     testqgsmultilinestring.cpp
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
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgslinestring.h"
#include "qgsgeometryutils.h"
#include "testgeometryutils.h"

class TestQgsMultiLineString: public QObject
{
    Q_OBJECT
  private slots:
    void multiLineString();
};

void TestQgsMultiLineString::multiLineString()
{
  //test constructor
  QgsMultiLineString c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiLineString" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiLineString" ) );
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
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiLineString );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a linestring
  QVERIFY( !c1.addGeometry( new QgsPoint() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiLineString );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 2 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiLineString" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiLineString" ) );
  QCOMPARE( c1.dimension(), 1 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 2 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 ) );
  QgsMultiLineString c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiLineStringZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiLineString" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiLineString c3;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 ) );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiLineStringM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiLineStringM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiLineString c4;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 ) );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiLineStringZM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiLineString c6;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 2 );
  part.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 2 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c6.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of points type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineString );
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( c6.vertexCount( 0, 0 ), 2 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 2 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 4 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QVERIFY( !c6.is3D() );
  const QgsLineString *ls = static_cast< const QgsLineString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 13 ) );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( c6.vertexCount( 0, 0 ), 2 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 2 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 2 );
  QCOMPARE( c6.nCoordinates(), 6 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 32, 41 ) );

  c6.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZ );
  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 6, 61, 0 ) );

  c6.clear();
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineString );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringM );
  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringM );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );

  c6.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZM );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 3 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );

  //clear
  QgsMultiLineString c7;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
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
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiLineString );

  //clone
  QgsMultiLineString c11;
  std::unique_ptr< QgsMultiLineString >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( part.clone() );
  c11.addGeometry( part.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  ls = static_cast< const QgsLineString * >( cloned->geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsLineString * >( cloned->geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //copy constructor
  QgsMultiLineString c12;
  QgsMultiLineString c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( part.clone() );
  c12.addGeometry( part.clone() );
  QgsMultiLineString c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiLineStringZM );
  ls = static_cast< const QgsLineString * >( c14.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsLineString * >( c14.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //assignment operator
  QgsMultiLineString c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  ls = static_cast< const QgsLineString * >( c15.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsLineString * >( c15.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //toCurveType
  std::unique_ptr< QgsMultiCurve > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsCompoundCurve *curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 0 ) );
  QCOMPARE( curve->asWkt(), QStringLiteral( "CompoundCurveZM ((5 50 1 4, 6 61 3 5))" ) );
  curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 1 ) );
  QCOMPARE( curve->asWkt(), QStringLiteral( "CompoundCurveZM ((5 50 1 4, 6 61 3 5))" ) );

  //to/fromWKB
  QgsMultiLineString c16;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 ) ) ;
  c16.addGeometry( part.clone() );
  QByteArray wkb16 = c16.asWkb();
  QgsMultiLineString c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 1 ) ) );

  //parts with Z
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 1 ) ) );

  //parts with m
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineStringM );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 1 ) ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineString );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineString );

  //to/from WKT
  QgsMultiLineString c18;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) ) ;
  c18.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) ) ;
  c18.addGeometry( part.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiLineString c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c19.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c18.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c19.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c18.geometryN( 1 ) ) );

  //bad WKT
  QgsMultiLineString c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiLineString );

  //as JSON
  QgsMultiLineString exportC;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  exportC.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 ) ) ;
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 3,13</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 43,43</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  QString res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiLineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiLineString().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13</posList></LineString></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43</posList></LineString></curveMember></MultiCurve>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiCurve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiLineString().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[[7.0,17.0],[3.0,13.0]],[[27.0,37.0],[43.0,43.0]]],\"type\":\"MultiLineString\"}" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiLineString exportFloat;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 ) ) ;
  exportFloat.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 ) ) ;
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( "{\"coordinates\":[[[2.333,5.667],[0.6,4.333]],[[9.0,4.111],[1.049,1.024]]],\"type\":\"MultiLineString\"}" );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 0.6,4.333</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 1.049,1.024</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333</posList></LineString></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024</posList></LineString></curveMember></MultiCurve>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<MultiGeometry><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>7,17,0 3,13,0</coordinates></LineString><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>27,37,0 43,43,0</coordinates></LineString></MultiGeometry>" ) );
  QCOMPARE( exportC.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>2.333,5.667,0 0.6,4.333,0</coordinates></LineString><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>9,4.111,0 1.049,1.024,0</coordinates></LineString></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );

  // insert geometry
  QgsMultiLineString rc;
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
  QVERIFY( !QgsMultiLineString().cast( nullptr ) );
  QgsMultiLineString pCast;
  QVERIFY( QgsMultiLineString().cast( &pCast ) );
  QgsMultiLineString pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiLineStringZ()" ) );
  QVERIFY( QgsMultiLineString().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiLineStringM()" ) );
  QVERIFY( QgsMultiLineString().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiLineStringZM()" ) );
  QVERIFY( QgsMultiLineString().cast( &pCast2 ) );

  //boundary
  QgsMultiLineString multiLine1;
  QVERIFY( !multiLine1.boundary() );
  QgsLineString boundaryLine1;
  boundaryLine1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  multiLine1.addGeometry( boundaryLine1.clone() );
  QgsAbstractGeometry *boundary = multiLine1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;
  // add another linestring
  QgsLineString boundaryLine2;
  boundaryLine2.setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) );
  multiLine1.addGeometry( boundaryLine2.clone() );
  boundary = multiLine1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 11.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 11.0 );
  delete boundary;

  // vertex iterator: 2 linestrings with 3 points each
  QgsAbstractGeometry::vertex_iterator it = multiLine1.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = multiLine1.vertices_end();
  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 11 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 2 ) );
  ++it;
  QCOMPARE( it, itEnd );

  // Java-style iterator
  QgsVertexIterator it2( &multiLine1 );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 11 ) );
  QVERIFY( !it2.hasNext() );

  // add a closed string = no boundary
  QgsLineString boundaryLine3;
  boundaryLine3.setPoints( QgsPointSequence() << QgsPoint( 20, 20 ) << QgsPoint( 21, 20 ) << QgsPoint( 21, 21 ) << QgsPoint( 20, 20 ) );
  multiLine1.addGeometry( boundaryLine3.clone() );
  boundary = multiLine1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 11.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 11.0 );
  delete boundary;

  //boundary with z
  QgsLineString boundaryLine4;
  boundaryLine4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  QgsLineString boundaryLine5;
  boundaryLine5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 200 ) );
  QgsMultiLineString multiLine2;
  multiLine2.addGeometry( boundaryLine4.clone() );
  multiLine2.addGeometry( boundaryLine5.clone() );

  boundary = multiLine2.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( mpBoundary->geometryN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->z(), 10.0 );
  QCOMPARE( mpBoundary->geometryN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );
  QCOMPARE( mpBoundary->geometryN( 2 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->z(), 100.0 );
  QCOMPARE( mpBoundary->geometryN( 3 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 20.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 20.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->z(), 200.0 );
  delete boundary;

  //segmentLength
  QgsMultiLineString multiLine3;
  QgsLineString vertexLine3;
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 )
                         << QgsPoint( 111, 2 ) << QgsPoint( 11, 2 ) );
  multiLine3.addGeometry( vertexLine3.clone() );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, -1, 0 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 1, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 1, 1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );

  // add another line
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 30, 6 ) << QgsPoint( 34, 6 ) << QgsPoint( 34, 8 )
                         << QgsPoint( 30, 8 ) << QgsPoint( 30, 6 ) );
  multiLine3.addGeometry( vertexLine3.clone() );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, -1, 0 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 0 ) ), 4.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 1 ) ), 2.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 2 ) ), 4.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 3 ) ), 2.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 4 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 1, 1 ) ), 2.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 1, 2 ) ), 4.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 2, 0, 0 ) ), 0.0 );
}


QGSTEST_MAIN( TestQgsMultiLineString )
#include "testqgsmultilinestring.moc"
