/***************************************************************************
     testqgsmultipoint.cpp
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

#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"

#include "testgeometryutils.h"

class TestQgsMultiPoint: public QObject
{
    Q_OBJECT
  private slots:
    void multiPoint();
};

void TestQgsMultiPoint::multiPoint()
{
  //test constructor
  QgsMultiPoint c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPoint );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiPoint" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiPoint" ) );
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
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPoint );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a point
  QVERIFY( !c1.addGeometry( new QgsLineString() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPoint );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsPoint part( 1, 10 );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 1 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPoint );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiPoint" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiPoint" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 1 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  part = QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 );
  QgsMultiPoint c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiPointZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiPointZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiPoint" ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiPoint c3;
  part = QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiPointM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiPointM" ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiPoint c4;
  part = QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiPointZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiPointZM" ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiPoint c6;
  part = QgsPoint( 10, 11 );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 1 );
  part = QgsPoint( 9, 1 );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 1 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c6.geometryN( 1 ) ), part );

  QgsCoordinateSequence seq = c6.coordinateSequence();
  QCOMPARE( seq, QgsCoordinateSequence() << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 10, 11 ) ) )
            << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 9, 1 ) ) ) );
  QCOMPARE( c6.nCoordinates(), 2 );

  //adding subsequent points should not alter z/m type, regardless of points type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint );
  QCOMPARE( c6.vertexCount( 0, 0 ), 1 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 1 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 2 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint ); //should still be 2d
  QVERIFY( !c6.is3D() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 1 ) ) ), QgsPoint( 1, 2 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 11.0, 12.0, 0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint );
  QCOMPARE( c6.vertexCount( 0, 0 ), 1 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 1 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 1 );
  QCOMPARE( c6.nCoordinates(), 3 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint ); //should still be 2d
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 2 ) ) ), QgsPoint( 11, 12 ) );

  c6.clear();
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZ );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZ );
  QVERIFY( c6.is3D() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 0 ) ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 1 ) ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 0 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 21.0, 22.0, 0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 2 ) ) ), QgsPoint( QgsWkbTypes::PointZ, 21, 22, 0 ) );

  c6.clear();
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointM );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointM );
  QVERIFY( c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 0 ) ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 1 ) ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 21.0, 22.0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 2 ) ) ), QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 0 ) );

  c6.clear();
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 4, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZM );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 0 ) ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 3 ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 1 ) ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 0, 0 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 21.0, 22.0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 2 ) ) ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 3, 0 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 31.0, 32.0, 0, 4 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 3 ) ) ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 0, 4 ) );

  //clear
  QgsMultiPoint c7;
  c7.addGeometry( new  QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) );
  c7.addGeometry( new  QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3 ) );
  QCOMPARE( c7.numGeometries(), 2 );
  c7.clear();
  QVERIFY( c7.isEmpty() );
  QCOMPARE( c7.numGeometries(), 0 );
  QCOMPARE( c7.nCoordinates(), 0 );
  QCOMPARE( c7.ringCount(), 0 );
  QCOMPARE( c7.partCount(), 0 );
  QVERIFY( !c7.is3D() );
  QVERIFY( !c7.isMeasure() );
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiPoint );

  //clone
  QgsMultiPoint c11;
  std::unique_ptr< QgsMultiPoint >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  c11.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPoint * >( cloned->geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( cloned->geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );

  //copy constructor
  QgsMultiPoint c12;
  QgsMultiPoint c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  c12.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 20, 10, 14, 18 ) );
  QgsMultiPoint c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiPointZM );
  QCOMPARE( *static_cast< const QgsPoint * >( c14.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c14.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 20, 10, 14, 18 ) );

  //assignment operator
  QgsMultiPoint c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPoint * >( c15.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c15.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 20, 10, 14, 18 ) );

  //toCurveType
  std::unique_ptr< QgsMultiPoint > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiPointZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsPoint *curve = static_cast< const QgsPoint * >( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  curve = static_cast< const QgsPoint * >( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, QgsPoint( QgsWkbTypes::PointZM, 20, 10, 14, 18 ) );

  //to/fromWKB
  QgsMultiPoint c16;
  c16.addGeometry( new QgsPoint( QgsWkbTypes::Point, 10, 11 ) );
  c16.addGeometry( new QgsPoint( QgsWkbTypes::Point, 20, 21 ) );
  QByteArray wkb16 = c16.asWkb();
  QgsMultiPoint c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::Point, 10, 11 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::Point, 20, 21 ) );

  //parts with Z
  c16.clear();
  c17.clear();
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) );
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPointZ );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) );

  //parts with m
  c16.clear();
  c17.clear();
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) );
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 ) );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPointM );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 70, 4 ) );
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 9, 1, 3, 4 ) );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPointZM );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 70, 4 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 3, 4 ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPoint );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPoint );

  //to/from WKT
  QgsMultiPoint c18;
  c18.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  c18.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiPoint c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPoint * >( c19.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c19.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );

  //bad WKT
  QgsMultiPoint c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiPoint );

  //as JSON
  QgsMultiPoint exportC;
  exportC.addGeometry( new QgsPoint( QgsWkbTypes::Point, 0, 10 ) );
  exportC.addGeometry( new QgsPoint( QgsWkbTypes::Point, 10, 0 ) );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,10</coordinates></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">10,0</coordinates></Point></pointMember></MultiPoint>" ) );
  QString res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiPoint xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPoint().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0 10</pos></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">10 0</pos></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiPoint xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPoint().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[0.0,10.0],[10.0,0.0]],\"type\":\"MultiPoint\"}" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiPoint exportFloat;
  exportFloat.addGeometry( new QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 100 / 9.0 ) );
  exportFloat.addGeometry( new QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 2 / 3.0 ) );


  QString expectedJsonPrec3( "{\"coordinates\":[[1.111,11.111],[1.333,0.667]],\"type\":\"MultiPoint\"}" );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,11.111</coordinates></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.333,0.667</coordinates></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1.111 11.111</pos></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1.333 0.667</pos></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<MultiGeometry><Point><coordinates>0,10</coordinates></Point><Point><coordinates>10,0</coordinates></Point></MultiGeometry>" ) );
  QCOMPARE( exportC.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><Point><coordinates>1.111,11.111</coordinates></Point><Point><coordinates>1.333,0.667</coordinates></Point></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );


  // insert geometry
  QgsMultiPoint rc;
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

  rc.insertGeometry( new QgsLineString(), 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  // cast
  QVERIFY( !QgsMultiPoint().cast( nullptr ) );
  QgsMultiPoint pCast;
  QVERIFY( QgsMultiPoint().cast( &pCast ) );
  QgsMultiPoint pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiPointZ(PointZ(0 1 1))" ) );
  QVERIFY( QgsMultiPoint().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiPointM(PointM(0 1 1))" ) );
  QVERIFY( QgsMultiPoint().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiPointZM(PointZM(0 1 1 2))" ) );
  QVERIFY( QgsMultiPoint().cast( &pCast2 ) );

  // bounding box
  QgsMultiPoint boundingBox;
  boundingBox.addGeometry( new QgsPoint( 0, 0 ) );
  QCOMPARE( boundingBox.boundingBox(), QgsRectangle( 0, 0, 0, 0 ) );
  boundingBox.addGeometry( new QgsPoint( 1, 2 ) );
  QCOMPARE( boundingBox.boundingBox(), QgsRectangle( 0, 0, 1, 2 ) );
  QgsMultiPoint boundingBox2;
  QCOMPARE( boundingBox2.boundingBox(), QgsRectangle( 0, 0, 0, 0 ) );
  boundingBox2.addGeometry( new QgsPoint( 1, 2 ) );
  QCOMPARE( boundingBox2.boundingBox(), QgsRectangle( 1, 2, 1, 2 ) );
  boundingBox2.addGeometry( new QgsPoint( 10, 3 ) );
  QCOMPARE( boundingBox2.boundingBox(), QgsRectangle( 1, 2, 10, 3 ) );
  boundingBox2.addGeometry( new QgsPoint( 0, 0 ) );
  QCOMPARE( boundingBox2.boundingBox(), QgsRectangle( 0, 0, 10, 3 ) );

  //boundary

  //multipoints have no boundary defined
  QgsMultiPoint boundaryMP;
  QVERIFY( !boundaryMP.boundary() );
  // add some points and retest, should still be undefined
  boundaryMP.addGeometry( new QgsPoint( 0, 0 ) );
  boundaryMP.addGeometry( new QgsPoint( 1, 1 ) );
  QVERIFY( !boundaryMP.boundary() );

  // closestSegment
  QgsPoint closest;
  QgsVertexId after;
  // return error - points have no segments
  QVERIFY( boundaryMP.closestSegment( QgsPoint( 0.5, 0.5 ), closest, after ) < 0 );

  // vertex iterator
  QgsAbstractGeometry::vertex_iterator it = boundaryMP.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = boundaryMP.vertices_end();
  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 0 ) );
  ++it;
  QCOMPARE( it, itEnd );

  // Java-style iterator
  QgsVertexIterator it2( &boundaryMP );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 1 ) );
  QVERIFY( !it2.hasNext() );

  //adjacent vertices - both should be invalid
  QgsMultiPoint c21;
  c21.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  c21.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );
  c21.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c21.adjacentVertices( QgsVertexId( 1, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );

  // vertex number
  QgsMultiPoint c22;
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  c22.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  c22.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 2, 0, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );

  QgsMultiPoint mp;
  // multipoints should not be affected by removeDuplicatePoints
  QVERIFY( !mp.removeDuplicateNodes() );
  mp.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 1, 4, 8 ) );
  mp.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 1, 4, 8 ) );
  QVERIFY( !mp.removeDuplicateNodes() );
  QCOMPARE( mp.numGeometries(), 2 );

  // filter vertex
  QgsMultiPoint filterPoint;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() < 5;
  };
  filterPoint.filterVertices( filter ); // no crash
  filterPoint.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  filterPoint.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 3, 0, 4, 8 ) );
  filterPoint.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 1, 0, 4, 8 ) );
  filterPoint.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 11, 0, 4, 8 ) );
  filterPoint.filterVertices( filter );
  QCOMPARE( filterPoint.asWkt( 2 ), QStringLiteral( "MultiPointZM ((3 0 4 8),(1 0 4 8))" ) );
}

QGSTEST_MAIN( TestQgsMultiPoint )
#include "testqgsmultipoint.moc"
