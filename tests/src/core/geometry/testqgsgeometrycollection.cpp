/***************************************************************************
     testqgsgeometrycollection.cpp
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
#include "qgsgeometrycollection.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgsproject.h"
#include "qgscoordinatetransform.h"
#include "testgeometryutils.h"
#include "testtransformer.h"

class TestQgsGeometryCollection: public QObject
{
    Q_OBJECT
  private slots:
    void geometryCollection();
};

void TestQgsGeometryCollection::geometryCollection()
{
  //test constructor
  QgsGeometryCollection c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c1.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( c1.geometryType(), QString( "GeometryCollection" ) );
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
  QCOMPARE( c1.wkbType(), QgsWkbTypes::GeometryCollection );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 5 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c1.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( c1.geometryType(), QString( "GeometryCollection" ) );
  QCOMPARE( c1.dimension(), 1 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QVERIFY( c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 5 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //retrieve geometry and check
  QCOMPARE( *( static_cast< const QgsLineString * >( c1.geometryN( 0 ) ) ), part );

  //initial adding of geometry should set z/m type
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  QgsGeometryCollection c2;
  c2.addGeometry( part.clone() );
  //QVERIFY( c2.is3D() ); //no meaning for collections?
  //QVERIFY( !c2.isMeasure() ); //no meaning for collections?
  QCOMPARE( c2.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c2.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( c2.geometryType(), QString( "GeometryCollection" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c2.geometryN( 0 ) ) ), part );
  QgsGeometryCollection c3;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  c3.addGeometry( part.clone() );
  //QVERIFY( !c3.is3D() ); //no meaning for collections?
  //QVERIFY( c3.isMeasure() ); //no meaning for collections?
  QCOMPARE( c3.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c3.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c3.geometryN( 0 ) ) ), part );
  QgsGeometryCollection c4;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 3, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 ) );
  c4.addGeometry( part.clone() );
  //QVERIFY( c4.is3D() ); //no meaning for collections?
  //QVERIFY( c4.isMeasure() ); //no meaning for collections?
  QCOMPARE( c4.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c4.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsGeometryCollection c6;
  part.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 5 );
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                  << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 5 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c6.geometryN( 1 ) ), part );

  QgsCoordinateSequence seq = c6.coordinateSequence();
  QCOMPARE( seq, QgsCoordinateSequence() << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
            << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) ) )
            << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                                        << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( c6.nCoordinates(), 10 );


  //clear
  QgsGeometryCollection c7;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  c7.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
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
  QCOMPARE( c7.wkbType(), QgsWkbTypes::GeometryCollection );

  //clone
  QgsGeometryCollection c11;
  std::unique_ptr< QgsGeometryCollection >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  c11.addGeometry( part.clone() );
  QgsLineString part2;
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  c11.addGeometry( part2.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( cloned->geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( cloned->geometryN( 1 ) ), part2 );

  //copy constructor
  QgsGeometryCollection c12;
  QgsGeometryCollection c13( c12 );
  QVERIFY( c13.isEmpty() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  c12.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  c12.addGeometry( part2.clone() );
  QgsGeometryCollection c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c14.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c14.geometryN( 1 ) ), part2 );

  //assignment operator
  QgsGeometryCollection c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c15.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c15.geometryN( 1 ) ), part2 );

  //equality
  QgsGeometryCollection emptyCollection;
  QVERIFY( !( emptyCollection == c15 ) );
  QVERIFY( emptyCollection != c15 );
  QgsPoint notCollection;
  QVERIFY( !( emptyCollection == notCollection ) );
  QVERIFY( emptyCollection != notCollection );
  QgsMultiPoint mp;
  QgsMultiLineString ml;
  QVERIFY( mp != ml );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  ml.addGeometry( part.clone() );
  QgsMultiLineString ml2;
  QVERIFY( ml != ml2 );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  ml2.addGeometry( part.clone() );
  QVERIFY( ml != ml2 );

  QgsMultiLineString ml3;
  ml3.addGeometry( part.clone() );
  QVERIFY( ml2 == ml3 );

  //toCurveType
  std::unique_ptr< QgsGeometryCollection > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsCompoundCurve *curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 0 ) );
  QCOMPARE( curve->numPoints(), 5 );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 1 ) );
  QCOMPARE( curve->numPoints(), 5 );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );

  //to/fromWKB
  QgsGeometryCollection c16;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  c16.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  c16.addGeometry( part2.clone() );
  QByteArray wkb16 = c16.asWkb();
  QCOMPARE( wkb16.size(), c16.wkbSize() );
  QgsGeometryCollection c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), part2 );

  //parts with Z
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  c16.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
  c16.addGeometry( part2.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), part2 );


  //parts with m
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10,  0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0,  0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  c16.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 1, 9, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 9, 9, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 ) );
  c16.addGeometry( part2.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), part2 );

  // parts with ZM
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  c16.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  c16.addGeometry( part2.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), part2 );


  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::GeometryCollection );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::GeometryCollection );

  //to/from WKT
  QgsGeometryCollection c18;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  c18.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  c18.addGeometry( part2.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsGeometryCollection c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c19.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c19.geometryN( 1 ) ), part2 );

  //bad WKT
  QgsGeometryCollection c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::GeometryCollection );

  //as JSON
  QgsGeometryCollection exportC;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );


  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LineString></geometryMember></MultiGeometry>" ) );
  QString res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiGeometry xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsGeometryCollection().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiGeometry xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsGeometryCollection().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"geometries\":[{\"coordinates\":[[0.0,0.0],[0.0,10.0],[10.0,10.0],[10.0,0.0],[0.0,0.0]],\"type\":\"LineString\"}],\"type\":\"GeometryCollection\"}" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                  << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                  << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  exportC.addGeometry( part.clone() );

  QString expectedJson( "{\"geometries\":[{\"coordinates\":[[0.0,0.0],[0.0,10.0],[10.0,10.0],[10.0,0.0],[0.0,0.0]],\"type\":\"LineString\"},{\"coordinates\":[[1.0,1.0],[1.0,9.0],[9.0,9.0],[9.0,1.0],[1.0,1.0]],\"type\":\"LineString\"}],\"type\":\"GeometryCollection\"}" );
  res = exportC.asJson();
  QCOMPARE( res, expectedJson );

  QgsGeometryCollection exportFloat;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 100 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 100 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 10 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 ) );
  exportFloat.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 4 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 4 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 2 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 ) );
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( "{\"geometries\":[{\"coordinates\":[[1.111,1.111],[1.111,11.111],[11.111,11.111],[11.111,1.111],[1.111,1.111]],\"type\":\"LineString\"},{\"coordinates\":[[0.667,0.667],[0.667,1.333],[1.333,1.333],[1.333,0.667],[0.667,0.667]],\"type\":\"LineString\"}],\"type\":\"GeometryCollection\"}" );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LineString></geometryMember><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,1 1,9 9,9 9,1 1,1</coordinates></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,1.111 1.111,11.111 11.111,11.111 11.111,1.111 1.111,1.111</coordinates></LineString></geometryMember><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.667 0.667,1.333 1.333,1.333 1.333,0.667 0.667,0.667</coordinates></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LineString></geometryMember><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 1 1 9 9 9 9 1 1 1</posList></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1.111 1.111 1.111 11.111 11.111 11.111 11.111 1.111 1.111 1.111</posList></LineString></geometryMember><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.667 0.667 0.667 1.333 1.333 1.333 1.333 0.667 0.667 0.667</posList></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<MultiGeometry><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0,0,0 0,10,0 10,10,0 10,0,0 0,0,0</coordinates></LinearRing><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>1,1,0 1,9,0 9,9,0 9,1,0 1,1,0</coordinates></LinearRing></MultiGeometry>" ) );
  QCOMPARE( exportC.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>1.111,1.111,0 1.111,11.111,0 11.111,11.111,0 11.111,1.111,0 1.111,1.111,0</coordinates></LinearRing><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0.667,0.667,0 0.667,1.333,0 1.333,1.333,0 1.333,0.667,0 0.667,0.667,0</coordinates></LinearRing></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );


  // remove geometry
  QgsGeometryCollection rc;
  // no crash!
  rc.removeGeometry( -1 );
  rc.removeGeometry( 0 );
  rc.removeGeometry( 100 );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  rc.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  rc.addGeometry( part2.clone() );
  // no crash
  rc.removeGeometry( -1 );
  rc.removeGeometry( 100 );

  rc.removeGeometry( 0 );
  QCOMPARE( rc.numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part2 );

  rc.addGeometry( part.clone() );
  rc.removeGeometry( 1 );
  QCOMPARE( rc.numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part2 );
  rc.removeGeometry( 0 );
  QCOMPARE( rc.numGeometries(), 0 );


  // insert geometry
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

  rc.insertGeometry( part.clone(), 0 );
  QCOMPARE( rc.numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part );
  rc.insertGeometry( part2.clone(), 0 );
  QCOMPARE( rc.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part2 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 1 ) ), part );
  rc.removeGeometry( 0 );
  rc.insertGeometry( part2.clone(), 1 );
  QCOMPARE( rc.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 1 ) ), part2 );
  rc.removeGeometry( 1 );
  rc.insertGeometry( part2.clone(), 2 );
  QCOMPARE( rc.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 1 ) ), part2 );

  // cast
  QVERIFY( !QgsGeometryCollection().cast( nullptr ) );
  QgsGeometryCollection pCast;
  QVERIFY( QgsGeometryCollection().cast( &pCast ) );
  QgsGeometryCollection pCast2;
  pCast2.fromWkt( QStringLiteral( "GeometryCollectionZ(PolygonZ((0 0 0, 0 1 1, 1 0 2, 0 0 0)))" ) );
  QVERIFY( QgsGeometryCollection().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "GeometryCollectionM(PolygonM((0 0 1, 0 1 2, 1 0 3, 0 0 1)))" ) );
  QVERIFY( QgsGeometryCollection().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "GeometryCollectionZM(PolygonZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1)))" ) );
  QVERIFY( QgsGeometryCollection().cast( &pCast2 ) );

  //transform
  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsGeometryCollection pTransform;
  QgsLineString l21;
  l21.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                 << QgsPoint( 6274985, -3526584 )
                 << QgsPoint( 6474985, -3526584 )
                 << QgsPoint( 6374985, -3626584 ) );
  pTransform.addGeometry( l21.clone() );
  pTransform.addGeometry( l21.clone() );
  pTransform.transform( tr, Qgis::TransformDirection::Forward );
  const QgsLineString *extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMaximum(), -38.7999, 0.001 );
  const QgsLineString *intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  QgsLineString l22;
  l22.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6274985, -3526584, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 5, 6 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 ) );
  pTransform.clear();
  pTransform.addGeometry( l22.clone() );
  pTransform.addGeometry( l22.clone() );
  pTransform.transform( tr, Qgis::TransformDirection::Forward );
  extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
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
  QGSCOMPARENEAR( extR->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMaximum(), -38.7999, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
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
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), -38.7999, 0.001 );

  //reverse transform
  pTransform.transform( tr, Qgis::TransformDirection::Reverse );
  extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
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
  QGSCOMPARENEAR( extR->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( extR->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( extR->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( extR->boundingBox().yMaximum(), -3526584, 100 );
  intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
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
  extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), -19.249066, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), -19.249066, 0.001 );
  pTransform.transform( tr, Qgis::TransformDirection::Reverse, true );
  extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
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
  QgsGeometryCollection pTransform2;
  pTransform2.addGeometry( l23.clone() );
  pTransform2.addGeometry( l23.clone() );
  pTransform2.transform( qtr, 3, 2, 6, 3 );

  extR = static_cast< const QgsLineString * >( pTransform2.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 9.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 18.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 29.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 48.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 49.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 78.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 9.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 18.0, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMaximum(), 36, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform2.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 9.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 18.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 29.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 48.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 49.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 78.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 9.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 18.0, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), 36, 0.001 );


  // closestSegment
  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;
  QgsGeometryCollection empty;
  ( void )empty.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty collection, just want no crash

  QgsGeometryCollection p21;
  QgsLineString p21ls;
  p21ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) << QgsPoint( 5, 10 ) );
  p21.addGeometry( p21ls.clone() );
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
  p21.addGeometry( p21ls.clone() );
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
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) );
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
  QCOMPARE( v, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  //nextVertex
  QgsGeometryCollection p22;
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  QgsLineString lp22;
  lp22.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p22.addGeometry( lp22.clone() );
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
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, 0 );
  // add another part
  lp22.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 11, 22 ) << QgsPoint( 11, 12 ) );
  p22.addGeometry( lp22.clone() );
  v = QgsVertexId( 1, 0, 4 ); //out of range
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, -5 );
  QVERIFY( p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, -1 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 21, 22 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 11, 22 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  v = QgsVertexId( 2, 0, 0 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 1, 0 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 1, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 21, 22 ) );


  // dropZValue
  QgsGeometryCollection p23;
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QgsLineString lp23;
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  p23.dropZValue(); // not z
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with z
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  p23.clear();
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );


  // dropMValue
  p23.clear();
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  p23.dropMValue(); // not zm
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with m
  lp23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 13 ) << QgsPoint( QgsWkbTypes::PointM, 1, 12, 0, 23 ) << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) );
  p23.clear();
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );

  //vertexAngle
  QgsGeometryCollection p24;
  ( void )p24.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 0, 1, 0 ) ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 1, 0, 0 ) ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( -1, 0, 0 ) ); //just want no crash
  QgsLineString l38;
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p24.addGeometry( l38.clone() );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );
  p24.addGeometry( l38.clone() );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 6 ) ), 2.35619, 0.00001 );

  //insert vertex

  //insert vertex in empty collection
  QgsGeometryCollection p25;
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p25.isEmpty() );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p25.addGeometry( l38.clone() );
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 8 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 9 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 8 ), QgsPoint( 0, 0 ) );
  // last vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 10 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 8 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );
  // with second part
  p25.addGeometry( l38.clone() );
  QCOMPARE( p25.nCoordinates(), 17 );
  QVERIFY( p25.insertVertex( QgsVertexId( 1, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 18 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 2, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex in second part
  QVERIFY( p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 19 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 8 ), QgsPoint( 0, 0 ) );
  // last vertex in second part
  QVERIFY( p25.insertVertex( QgsVertexId( 1, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 20 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 8 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );

  //move vertex

  //empty collection
  QgsGeometryCollection p26;
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( -1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.isEmpty() );

  //valid collection
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  p26.addGeometry( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 1, 2 ) );

  //out of range
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 1.0, 2.0 ) );

  // with second part
  p26.addGeometry( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 1, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 1, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 1, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 1, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 2, 0, 0 ), QgsPoint( 3.0, 4.0 ) ) );

  //delete vertex

  //empty collection
  QgsGeometryCollection p27;
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 1, 1, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( -1, 1, 0 ) ) );
  QVERIFY( p27.isEmpty() );

  //valid collection
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 5, 2 ) << QgsPoint( 6, 2 ) << QgsPoint( 7, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );

  p27.addGeometry( l38.clone() );
  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 4 ), QgsPoint( 1.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete some more vertices - should remove part
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !p27.geometryN( 0 ) );

  // with two parts
  p27.addGeometry( l38.clone() );
  p27.addGeometry( l38.clone() );

  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 1, 0, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 1, 0, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 2, 0, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 4 ), QgsPoint( 1.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete some more vertices - should remove part
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );
  QCOMPARE( p27.numGeometries(), 1 );
  QVERIFY( p27.geometryN( 0 ) );

  // test that second geometry is "promoted" when first is removed
  p27.addGeometry( l38.clone() );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numGeometries(), 2 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numGeometries(), 2 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numGeometries(), 2 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numGeometries(), 1 );
  QVERIFY( p27.geometryN( 0 ) );

  //boundary

  // collections have no boundary defined
  QgsGeometryCollection boundaryCollection;
  QVERIFY( !boundaryCollection.boundary() );
  // add a geometry and retest, should still be undefined
  QgsLineString *lineBoundary = new QgsLineString();
  lineBoundary->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  boundaryCollection.addGeometry( lineBoundary );
  QVERIFY( !boundaryCollection.boundary() );

  // segmentize
  QgsGeometryCollection segmentC;
  QgsCircularString toSegment;
  toSegment.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  segmentC.addGeometry( toSegment.clone() );
  std::unique_ptr<QgsGeometryCollection> segmentized( static_cast< QgsGeometryCollection * >( segmentC.segmentize() ) );
  const QgsLineString *segmentizedLine = static_cast< const QgsLineString * >( segmentized->geometryN( 0 ) );
  QCOMPARE( segmentizedLine->numPoints(), 156 );
  QCOMPARE( segmentizedLine->vertexCount(), 156 );
  QCOMPARE( segmentizedLine->ringCount(), 1 );
  QCOMPARE( segmentizedLine->partCount(), 1 );
  QCOMPARE( segmentizedLine->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentizedLine->is3D() );
  QVERIFY( !segmentizedLine->isMeasure() );
  QCOMPARE( segmentizedLine->pointN( 0 ), toSegment.pointN( 0 ) );
  QCOMPARE( segmentizedLine->pointN( segmentizedLine->numPoints() - 1 ), toSegment.pointN( toSegment.numPoints() - 1 ) );

  // hasCurvedSegments
  QgsGeometryCollection c30;
  QVERIFY( !c30.hasCurvedSegments() );
  c30.addGeometry( part.clone() );
  QVERIFY( !c30.hasCurvedSegments() );
  c30.addGeometry( toSegment.clone() );
  QVERIFY( c30.hasCurvedSegments() );


  //adjacent vertices
  QgsGeometryCollection c31;
  QgsLineString vertexLine1;
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );
  c31.adjacentVertices( QgsVertexId( 0, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c31.adjacentVertices( QgsVertexId( -1, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c31.adjacentVertices( QgsVertexId( 10, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  vertexLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  c31.addGeometry( vertexLine1.clone() );
  c31.addGeometry( vertexLine1.clone() );
  c31.adjacentVertices( QgsVertexId( -1, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c31.adjacentVertices( QgsVertexId( 10, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c31.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  c31.adjacentVertices( QgsVertexId( 0, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  c31.adjacentVertices( QgsVertexId( 1, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 1, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 1, 0, 2 ) );


  // vertex number
  QgsGeometryCollection c32;
  QgsLineString vertexLine2;
  vertexLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  c32.addGeometry( vertexLine2.clone() );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );
  c32.addGeometry( vertexLine2.clone() );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 3 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), 4 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 2 ) ), 5 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 3 ) ), -1 );
  QgsPolygon polyPart;
  vertexLine2.close();
  polyPart.setExteriorRing( vertexLine2.clone() );
  c32.addGeometry( polyPart.clone() );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 3 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), 4 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 2 ) ), 5 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 3 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, -1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, -1 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 0 ) ), 6 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 1 ) ), 7 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 2 ) ), 8 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 3 ) ), 9 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 4 ) ), -1 );
  polyPart.addInteriorRing( vertexLine2.clone() );
  c32.addGeometry( polyPart.clone() );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 0 ) ), 6 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 1 ) ), 7 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 2 ) ), 8 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 3 ) ), 9 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 4 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, -1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 2, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 0 ) ), 10 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 1 ) ), 11 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 2 ) ), 12 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 3 ) ), 13 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 4 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 0 ) ), 14 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 1 ) ), 15 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 2 ) ), 16 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 3 ) ), 17 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 4 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 2, 0 ) ), -1 );


  //removeDuplicateNodes
  QgsGeometryCollection gcNodes;
  QgsLineString nodeLine;
  QVERIFY( !gcNodes.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  gcNodes.addGeometry( nodeLine.clone() );
  QVERIFY( !gcNodes.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( gcNodes.asWkt(), QStringLiteral( "GeometryCollection (LineString (11 2, 11 12, 111 12))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  gcNodes.addGeometry( nodeLine.clone() );
  QVERIFY( gcNodes.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !gcNodes.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( gcNodes.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineString (11 2, 11 12, 111 12),LineString (11 2, 11 12, 111 12))" ) );

  //swapXy
  QgsGeometryCollection swapCollect;
  QgsLineString swapLine;
  swapCollect.swapXy(); // no crash
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  swapCollect.addGeometry( swapLine.clone() );
  swapCollect.swapXy();
  QCOMPARE( swapCollect.asWkt(), QStringLiteral( "GeometryCollection (LineStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24))" ) );
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 1.99, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11.02, 2.01, 25, 26, QgsWkbTypes::PointZM ) );
  swapCollect.addGeometry( swapLine.clone() );
  swapCollect.swapXy();
  QCOMPARE( swapCollect.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24),LineStringZM (2 11 5 6, 1.99 11.01 15 16, 2.01 11.02 25 26))" ) );

  // filter vertices
  QgsGeometryCollection filterCollect;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() > 5;
  };
  QgsLineString filterLine;
  filterCollect.filterVertices( filter ); // no crash
  filterLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  filterCollect.addGeometry( filterLine.clone() );
  filterCollect.filterVertices( filter );
  QCOMPARE( filterCollect.asWkt(), QStringLiteral( "GeometryCollection (LineStringZM (11 12 13 14, 111 12 23 24))" ) );
  filterLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 1.01, 1.99, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11.02, 2.01, 25, 26, QgsWkbTypes::PointZM ) );
  filterCollect.addGeometry( filterLine.clone() );
  filterCollect.filterVertices( filter );
  QCOMPARE( filterCollect.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineStringZM (11 12 13 14, 111 12 23 24),LineStringZM (11 2 5 6, 11.02 2.01 25 26))" ) );

  // transform vertices
  QgsGeometryCollection transformCollect;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 5 );
  };
  QgsLineString transformLine;
  transformCollect.transformVertices( transform ); // no crash
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  transformCollect.addGeometry( transformLine.clone() );
  transformCollect.transformVertices( transform );
  QCOMPARE( transformCollect.asWkt(), QStringLiteral( "GeometryCollection (LineStringZM (3 5 7 9, 13 15 17 19, 113 15 27 29))" ) );
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 1.01, 1.99, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11.02, 2.01, 25, 26, QgsWkbTypes::PointZM ) );
  transformCollect.addGeometry( transformLine.clone() );
  transformCollect.transformVertices( transform );
  QCOMPARE( transformCollect.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineStringZM (5 8 11 14, 15 18 21 24, 115 18 31 34),LineStringZM (13 5 9 11, 3.01 4.99 19 21, 13.02 5.01 29 31))" ) );

  // transform using class
  TestTransformer transformer;
  QgsGeometryCollection transformCollect2;
  transformCollect2.transform( &transformer ); // no crash
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  transformCollect2.addGeometry( transformLine.clone() );
  QVERIFY( transformCollect2.transform( &transformer ) );
  QCOMPARE( transformCollect2.asWkt(), QStringLiteral( "GeometryCollection (LineStringZM (3 16 8 3, 33 26 18 13, 333 26 28 23))" ) );
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 1.01, 1.99, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11.02, 2.01, 25, 26, QgsWkbTypes::PointZM ) );
  transformCollect2.addGeometry( transformLine.clone() );
  QVERIFY( transformCollect2.transform( &transformer ) );
  QCOMPARE( transformCollect2.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineStringZM (9 30 13 2, 99 40 23 12, 999 40 33 22),LineStringZM (33 16 10 5, 3.03 15.99 20 15, 33.06 16.01 30 25))" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !transformCollect2.transform( &failTransformer ) );
}


QGSTEST_MAIN( TestQgsGeometryCollection )
#include "testqgsgeometrycollection.moc"
