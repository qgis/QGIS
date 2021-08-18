/***************************************************************************
     testqgscircularstring.cpp
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
#include "qgsgeometryutils.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgsmultipoint.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "testgeometryutils.h"
#include "testtransformer.h"

class TestQgsCircularString: public QObject
{
    Q_OBJECT
  private slots:
    void circularString();
    void circularStringFromArray();
    void circularStringAppend();
};

void TestQgsCircularString::circularString()
{
  //test constructors
  QgsCircularString l1;
  QVERIFY( l1.isEmpty() );
  QCOMPARE( l1.numPoints(), 0 );
  QCOMPARE( l1.vertexCount(), 0 );
  QCOMPARE( l1.nCoordinates(), 0 );
  QCOMPARE( l1.ringCount(), 0 );
  QCOMPARE( l1.partCount(), 0 );
  QVERIFY( !l1.is3D() );
  QVERIFY( !l1.isMeasure() );
  QCOMPARE( l1.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l1.wktTypeStr(), QString( "CircularString" ) );
  QCOMPARE( l1.geometryType(), QString( "CircularString" ) );
  QCOMPARE( l1.dimension(), 1 );
  QVERIFY( l1.hasCurvedSegments() );
  QCOMPARE( l1.area(), 0.0 );
  QCOMPARE( l1.perimeter(), 0.0 );
  QgsPointSequence pts;
  l1.points( pts );
  QVERIFY( pts.empty() );

  // from 3 points
  QgsCircularString from3Pts( QgsPoint( 1, 2 ), QgsPoint( 21, 22 ), QgsPoint( 31, 2 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  from3Pts = QgsCircularString( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZ, 21, 22, 23 ),
                                QgsPoint( QgsWkbTypes::PointZ, 31, 2, 33 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).z(), 3.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).z(), 23.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).z(), 33.0 );
  from3Pts = QgsCircularString( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ), QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 23 ),
                                QgsPoint( QgsWkbTypes::PointM, 31, 2, 0, 33 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).m(), 3.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).m(), 23.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).m(), 33.0 );
  from3Pts = QgsCircularString( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ),
                                QgsPoint( QgsWkbTypes::PointZM, 31, 2, 33, 34 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).z(), 3.0 );
  QCOMPARE( from3Pts.pointN( 0 ).m(), 4.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).z(), 23.0 );
  QCOMPARE( from3Pts.pointN( 1 ).m(), 24.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).z(), 33.0 );
  QCOMPARE( from3Pts.pointN( 2 ).m(), 34.0 );

  // from 2 points and center
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( 1, 2 ), QgsPoint( 31, 2 ), QgsPoint( 21, 2 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( 1, 2 ), QgsPoint( 31, 2 ), QgsPoint( 21, 2 ), false );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), -18.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZ, 32, 2, 33 ),
             QgsPoint( QgsWkbTypes::PointZ, 21, 2, 23 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).z(), 3.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).z(), 23.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 32.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).z(), 33.0 );
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ), QgsPoint( QgsWkbTypes::PointM, 31, 2, 0, 33 ),
             QgsPoint( QgsWkbTypes::PointM, 21, 2, 0, 23 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).m(), 3.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).m(), 23.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).m(), 33.0 );
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ), QgsPoint( QgsWkbTypes::PointZM, 31, 2, 33, 34 ),
             QgsPoint( QgsWkbTypes::PointZM, 21, 2, 23, 24 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).z(), 3.0 );
  QCOMPARE( from3Pts.pointN( 0 ).m(), 4.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).z(), 23.0 );
  QCOMPARE( from3Pts.pointN( 1 ).m(), 24.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).z(), 33.0 );
  QCOMPARE( from3Pts.pointN( 2 ).m(), 34.0 );

  //setPoints
  QgsCircularString l2;
  l2.setPoints( QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !l2.isEmpty() );
  QCOMPARE( l2.numPoints(), 1 );
  QCOMPARE( l2.vertexCount(), 1 );
  QCOMPARE( l2.nCoordinates(), 1 );
  QCOMPARE( l2.ringCount(), 1 );
  QCOMPARE( l2.partCount(), 1 );
  QVERIFY( !l2.is3D() );
  QVERIFY( !l2.isMeasure() );
  QCOMPARE( l2.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l2.hasCurvedSegments() );
  QCOMPARE( l2.area(), 0.0 );
  QCOMPARE( l2.perimeter(), 0.0 );
  l2.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );

  //setting first vertex should set linestring z/m type
  QgsCircularString l3;
  l3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QVERIFY( !l3.isEmpty() );
  QVERIFY( l3.is3D() );
  QVERIFY( !l3.isMeasure() );
  QCOMPARE( l3.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( l3.wktTypeStr(), QString( "CircularStringZ" ) );
  l3.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  QgsCircularString l4;
  l4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QVERIFY( !l4.isEmpty() );
  QVERIFY( !l4.is3D() );
  QVERIFY( l4.isMeasure() );
  QCOMPARE( l4.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( l4.wktTypeStr(), QString( "CircularStringM" ) );
  l4.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );

  QgsCircularString l5;
  l5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( !l5.isEmpty() );
  QVERIFY( l5.is3D() );
  QVERIFY( l5.isMeasure() );
  QCOMPARE( l5.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l5.wktTypeStr(), QString( "CircularStringZM" ) );
  l5.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  //clear
  l5.clear();
  QVERIFY( l5.isEmpty() );
  QCOMPARE( l5.numPoints(), 0 );
  QCOMPARE( l5.vertexCount(), 0 );
  QCOMPARE( l5.nCoordinates(), 0 );
  QCOMPARE( l5.ringCount(), 0 );
  QCOMPARE( l5.partCount(), 0 );
  QVERIFY( !l5.is3D() );
  QVERIFY( !l5.isMeasure() );
  QCOMPARE( l5.wkbType(), QgsWkbTypes::CircularString );

  //setPoints
  QgsCircularString l8;
  l8.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QVERIFY( !l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 3 );
  QCOMPARE( l8.vertexCount(), 3 );
  QCOMPARE( l8.nCoordinates(), 3 );
  QCOMPARE( l8.ringCount(), 1 );
  QCOMPARE( l8.partCount(), 1 );
  QVERIFY( !l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l8.hasCurvedSegments() );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );

  //setPoints with empty list, should clear linestring
  l8.setPoints( QgsPointSequence() );
  QVERIFY( l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 0 );
  QCOMPARE( l8.vertexCount(), 0 );
  QCOMPARE( l8.nCoordinates(), 0 );
  QCOMPARE( l8.ringCount(), 0 );
  QCOMPARE( l8.partCount(), 0 );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularString );
  l8.points( pts );
  QVERIFY( pts.empty() );

  //setPoints with z
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularStringZ );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //setPoints with m
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( !l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularStringM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //setPoints with zm
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );

  //setPoints with MIXED dimensionality of points
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //test point
  QCOMPARE( l8.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) );
  QCOMPARE( l8.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //out of range - just want no crash here
  QgsPoint bad = l8.pointN( -1 );
  bad = l8.pointN( 100 );

  //test getters/setters
  QgsCircularString l9;
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  QCOMPARE( l9.xAt( 0 ), 1.0 );
  QCOMPARE( l9.xAt( 1 ), 11.0 );
  QCOMPARE( l9.xAt( 2 ), 21.0 );
  QCOMPARE( l9.xAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.xAt( 11 ), 0.0 ); //out of range

  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 2 ) );
  QCOMPARE( l9.xAt( 0 ), 51.0 );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 12 ) );
  QCOMPARE( l9.xAt( 1 ), 61.0 );
  l9.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 71.0, 2 ) ); //out of range
  l9.moveVertex( QgsVertexId( 0, 0, 11 ), QgsPoint( 71.0, 2 ) ); //out of range

  QCOMPARE( l9.yAt( 0 ), 2.0 );
  QCOMPARE( l9.yAt( 1 ), 12.0 );
  QCOMPARE( l9.yAt( 2 ), 22.0 );
  QCOMPARE( l9.yAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.yAt( 11 ), 0.0 ); //out of range

  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 52 ) );
  QCOMPARE( l9.yAt( 0 ), 52.0 );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 62 ) );
  QCOMPARE( l9.yAt( 1 ), 62.0 );

  QCOMPARE( l9.pointN( 0 ).z(), 3.0 );
  QCOMPARE( l9.pointN( 1 ).z(), 13.0 );
  QCOMPARE( l9.pointN( 2 ).z(), 23.0 );

  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53 ) );
  QCOMPARE( l9.pointN( 0 ).z(), 53.0 );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 63 ) );
  QCOMPARE( l9.pointN( 1 ).z(), 63.0 );

  QCOMPARE( l9.pointN( 0 ).m(), 4.0 );
  QCOMPARE( l9.pointN( 1 ).m(), 14.0 );
  QCOMPARE( l9.pointN( 2 ).m(), 24.0 );

  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53, 54 ) );
  QCOMPARE( l9.pointN( 0 ).m(), 54.0 );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 53, 64 ) );
  QCOMPARE( l9.pointN( 1 ).m(), 64.0 );

  //check zAt/setZAt with non-3d linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 )
                << QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 24 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( l9.pointN( 0 ).z() ) );
  QVERIFY( std::isnan( l9.pointN( 1 ).z() ) );
  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53 ) );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 63 ) );

  //check mAt/setMAt with non-measure linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( l9.pointN( 0 ).m() ) );
  QVERIFY( std::isnan( l9.pointN( 1 ).m() ) );
  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 0, 53 ) );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 0, 63 ) );

  //equality
  QgsCircularString e1;
  QgsCircularString e2;
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  QVERIFY( !( e1 == e2 ) ); //different number of vertices
  QVERIFY( e1 != e2 );
  e2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  e2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  QVERIFY( e1 == e2 ); //check non-integer equality
  QVERIFY( !( e1 != e2 ) );
  e1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) << QgsPoint( 7, 8 ) );
  e2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) << QgsPoint( 6, 9 ) );
  QVERIFY( !( e1 == e2 ) ); //different coordinates
  QVERIFY( e1 != e2 );
  QgsCircularString e3;
  e3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );
  QVERIFY( !( e1 == e3 ) ); //different dimension
  QVERIFY( e1 != e3 );
  QgsCircularString e4;
  e4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );
  QVERIFY( !( e3 == e4 ) ); //different z coordinates
  QVERIFY( e3 != e4 );
  QgsCircularString e5;
  e5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  QgsCircularString e6;
  e6.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );
  QVERIFY( !( e5 == e6 ) ); //different m values
  QVERIFY( e5 != e6 );

  QVERIFY( e6 != QgsLineString() );

  //isClosed
  QgsCircularString l11;
  QVERIFY( !l11.isClosed2D() );
  QVERIFY( !l11.isClosed() );
  l11.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  QVERIFY( !l11.isClosed2D() );
  QVERIFY( !l11.isClosed() );
  QCOMPARE( l11.numPoints(), 4 );
  QCOMPARE( l11.area(), 0.0 );
  QCOMPARE( l11.perimeter(), 0.0 );

  //test that m values aren't considered when testing for closedness
  l11.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                 << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );
  QVERIFY( l11.isClosed2D() );
  QVERIFY( l11.isClosed() );

  // test with z
  l11.addZValue( 123.0 );
  QVERIFY( l11.isClosed2D() );
  QVERIFY( l11.isClosed() );
  QgsPoint pEnd = l11.endPoint();
  pEnd.setZ( 234.0 );
  l11.moveVertex( QgsVertexId( 0, 0, l11.numPoints() - 1 ), pEnd );
  QVERIFY( l11.isClosed2D() );
  QVERIFY( !l11.isClosed() );

  //polygonf
  QgsCircularString l13;
  l13.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 ) );

  QPolygonF poly = l13.asQPolygonF();
  QCOMPARE( poly.count(), 181 );
  QCOMPARE( poly.at( 0 ).x(), 1.0 );
  QCOMPARE( poly.at( 0 ).y(), 2.0 );
  QCOMPARE( poly.at( 180 ).x(), 11.0 );
  QCOMPARE( poly.at( 180 ).y(), 22.0 );

  // clone tests
  QgsCircularString l14;
  l14.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  std::unique_ptr<QgsCircularString> cloned( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->vertexCount(), 4 );
  QCOMPARE( cloned->ringCount(), 1 );
  QCOMPARE( cloned->partCount(), 1 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );

  //clone with Z/M
  l14.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  cloned.reset( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( cloned->is3D() );
  QVERIFY( cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );

  //clone an empty line
  l14.clear();
  cloned.reset( l14.clone() );
  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularString );

  //segmentize tests
  QgsCircularString toSegment;
  toSegment.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  std::unique_ptr<QgsLineString> segmentized( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), toSegment.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), toSegment.pointN( toSegment.numPoints() - 1 ) );

  //segmentize with Z/M
  toSegment.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                       << QgsPoint( QgsWkbTypes::PointZM, 11, 10, 11, 14 )
                       << QgsPoint( QgsWkbTypes::PointZM, 21, 2, 21, 24 ) );
  segmentized.reset( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), toSegment.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), toSegment.pointN( toSegment.numPoints() - 1 ) );

  //segmentize an empty line
  toSegment.clear();
  segmentized.reset( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );

  //to/from WKB
  QgsCircularString l15;
  l15.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QByteArray wkb15 = l15.asWkb();
  QCOMPARE( wkb15.size(), l15.wkbSize() );
  QgsCircularString l16;
  QgsConstWkbPtr wkb15ptr( wkb15 );
  l16.fromWkb( wkb15ptr );
  QCOMPARE( l16.numPoints(), 4 );
  QCOMPARE( l16.vertexCount(), 4 );
  QCOMPARE( l16.nCoordinates(), 4 );
  QCOMPARE( l16.ringCount(), 1 );
  QCOMPARE( l16.partCount(), 1 );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( l16.is3D() );
  QVERIFY( l16.isMeasure() );
  QCOMPARE( l16.pointN( 0 ), l15.pointN( 0 ) );
  QCOMPARE( l16.pointN( 1 ), l15.pointN( 1 ) );
  QCOMPARE( l16.pointN( 2 ), l15.pointN( 2 ) );
  QCOMPARE( l16.pointN( 3 ), l15.pointN( 3 ) );

  //bad WKB - check for no crash
  l16.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !l16.fromWkb( nullPtr ) );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::CircularString );
  QgsPoint point( 1, 2 );
  QByteArray wkb16 = point.asWkb();
  QgsConstWkbPtr wkb16ptr( wkb16 );
  QVERIFY( !l16.fromWkb( wkb16ptr ) );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::CircularString );

  //to/from WKT
  QgsCircularString l17;
  l17.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

  QString wkt = l17.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsCircularString l18;
  QVERIFY( l18.fromWkt( wkt ) );
  QCOMPARE( l18.numPoints(), 4 );
  QCOMPARE( l18.wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( l18.is3D() );
  QVERIFY( l18.isMeasure() );
  QCOMPARE( l18.pointN( 0 ), l17.pointN( 0 ) );
  QCOMPARE( l18.pointN( 1 ), l17.pointN( 1 ) );
  QCOMPARE( l18.pointN( 2 ), l17.pointN( 2 ) );
  QCOMPARE( l18.pointN( 3 ), l17.pointN( 3 ) );

  //bad WKT
  QVERIFY( !l18.fromWkt( "Polygon()" ) );
  QVERIFY( l18.isEmpty() );
  QCOMPARE( l18.numPoints(), 0 );
  QVERIFY( !l18.is3D() );
  QVERIFY( !l18.isMeasure() );
  QCOMPARE( l18.wkbType(), QgsWkbTypes::CircularString );

  //asGML2
  QgsCircularString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 )
                        << QgsPoint( 51, 52 ) );
  QgsCircularString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLine.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLineFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCircularString().asGml2( doc ) ), expectedGML2empty );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></ArcString></segments></Curve>" ) );
  QCOMPARE( elemToString( exportLine.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></ArcString></segments></Curve>" ) );
  QCOMPARE( elemToString( exportLineFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<Curve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCircularString().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( QStringLiteral( "{\"coordinates\":[[31.0,32.0],[41.0,42.0],[51.0,52.0]],\"type\":\"LineString\"}" ) );
  QCOMPARE( exportLine.asJson(), expectedJson );
  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[[0.333,0.667],[1.333,1.667],[2.333,2.667]],\"type\":\"LineString\"}" ) );
  QCOMPARE( exportLineFloat.asJson( 3 ), expectedJsonPrec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>31,32,0 41,42,0 51,52,0</coordinates></LineString>" ) );
  QCOMPARE( exportLine.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>0.333,0.667,0 1.333,1.667,0 2.333,2.667,0</coordinates></LineString>" ) );
  QCOMPARE( exportLineFloat.asKml( 3 ), expectedKmlPrec3 );

  //length
  QgsCircularString l19;
  QCOMPARE( l19.length(), 0.0 );
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QGSCOMPARENEAR( l19.length(), 26.1433, 0.001 );

  //startPoint
  QCOMPARE( l19.startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );

  //endPoint
  QCOMPARE( l19.endPoint(), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  //bad start/end points. Test that this doesn't crash.
  l19.clear();
  QVERIFY( l19.startPoint().isEmpty() );
  QVERIFY( l19.endPoint().isEmpty() );

  //curveToLine - no segmentation required, so should return a clone
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  segmentized.reset( l19.curveToLine() );
  QCOMPARE( segmentized->numPoints(), 181 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l19.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), l19.pointN( l19.numPoints() - 1 ) );

  // points
  QgsCircularString l20;
  QgsPointSequence points;
  l20.points( points );
  QVERIFY( l20.isEmpty() );
  l20.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  l20.points( points );
  QCOMPARE( points.count(), 3 );
  QCOMPARE( points.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( points.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 ) );
  QCOMPARE( points.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) );// want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsCircularString l21;
  l21.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                 << QgsPoint( 6474985, -3526584 ) );
  l21.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QGSCOMPARENEAR( l21.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( l21.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( l21.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( l21.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( l21.boundingBox().xMinimum(), 175.771, 0.001 );
  QGSCOMPARENEAR( l21.boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( l21.boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( l21.boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  QgsCircularString l22;
  l22.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  l22.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QGSCOMPARENEAR( l22.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 0 ).z(), 1.0, 0.001 );
  QCOMPARE( l22.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( l22.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 1 ).z(), 3.0, 0.001 );
  QCOMPARE( l22.pointN( 1 ).m(), 4.0 );

  //reverse transform
  l22.transform( tr, QgsCoordinateTransform::ReverseTransform );
  QGSCOMPARENEAR( l22.pointN( 0 ).x(), 6374985, 0.01 );
  QGSCOMPARENEAR( l22.pointN( 0 ).y(), -3626584, 0.01 );
  QGSCOMPARENEAR( l22.pointN( 0 ).z(), 1, 0.001 );
  QCOMPARE( l22.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( l22.pointN( 1 ).x(), 6474985, 0.01 );
  QGSCOMPARENEAR( l22.pointN( 1 ).y(), -3526584, 0.01 );
  QGSCOMPARENEAR( l22.pointN( 1 ).z(), 3, 0.001 );
  QCOMPARE( l22.pointN( 1 ).m(), 4.0 );

#if PROJ_VERSION_MAJOR<6 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //z value transform
  l22.transform( tr, QgsCoordinateTransform::ForwardTransform, true );
  QGSCOMPARENEAR( l22.pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 1 ).z(), -21.092128, 0.001 );
  l22.transform( tr, QgsCoordinateTransform::ReverseTransform, true );
  QGSCOMPARENEAR( l22.pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 1 ).z(), 3.0, 0.001 );
#endif

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsCircularString l23;
  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  l23.transform( qtr );
  QCOMPARE( l23.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 2, 6, 3, 4 ) );
  QCOMPARE( l23.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 22, 36, 13, 14 ) );
  QCOMPARE( l23.boundingBox(), QgsRectangle( 2, 6, 22, 36 ) );

  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  l23.transform( QTransform::fromScale( 1, 1 ), 3, 2, 4, 3 );
  QCOMPARE( l23.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 9, 16 ) );
  QCOMPARE( l23.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 29, 46 ) );

  //insert vertex
  //cannot insert vertex in empty line
  QgsCircularString l24;
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 0 );

  //2d line
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 4.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QVERIFY( !l24.is3D() );
  QVERIFY( !l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( 4.0, 7.0 ) );
  QGSCOMPARENEAR( l24.pointN( 2 ).x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( l24.pointN( 2 ).y(), 9.930870, 0.01 );
  QCOMPARE( l24.pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( l24.pointN( 4 ), QgsPoint( 1.0, 22.0 ) );

  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 16.0 ) ) );
  QCOMPARE( l24.numPoints(), 9 );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QGSCOMPARENEAR( l24.pointN( 1 ).x(), 4.363083, 0.01 );
  QGSCOMPARENEAR( l24.pointN( 1 ).y(), 5.636917, 0.01 );
  QCOMPARE( l24.pointN( 2 ), QgsPoint( 8.0, 9.0 ) );
  QCOMPARE( l24.pointN( 3 ), QgsPoint( 18.0, 16.0 ) );
  QGSCOMPARENEAR( l24.pointN( 4 ).x(), 5.876894, 0.01 );
  QGSCOMPARENEAR( l24.pointN( 4 ).y(), 8.246211, 0.01 );
  QCOMPARE( l24.pointN( 5 ), QgsPoint( 4.0, 7.0 ) );
  QGSCOMPARENEAR( l24.pointN( 6 ).x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( l24.pointN( 6 ).y(), 9.930870, 0.01 );
  QCOMPARE( l24.pointN( 7 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( l24.pointN( 8 ), QgsPoint( 1.0, 22.0 ) );

  //insert vertex at end
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 31.0, 32.0 ) ) );

  //insert vertex past end
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 9 );

  //insert vertex before start
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 9 );

  //insert 4d vertex in 4d line
  l24.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );
  QCOMPARE( l24.numPoints(), 7 );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 4, 103, 104 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 2, 4 ) );

  //move vertex

  //empty line
  QgsCircularString l25;
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( l25.isEmpty() );

  //valid line
  l25.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( l25.pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( l25.pointN( 2 ), QgsPoint( 26.0, 27.0 ) );

  //out of range
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( l25.pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( l25.pointN( 2 ), QgsPoint( 26.0, 27.0 ) );

  //move 4d point in 4d line
  l25.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) ) );
  QCOMPARE( l25.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) );

  //move 2d point in 4d line, existing z/m should be maintained
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 34, 35 ) ) );
  QCOMPARE( l25.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 34, 35, 12, 13 ) );

  //move 4d point in 2d line
  l25.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 3, 4, 2, 3 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPoint( 3, 4 ) );


  //delete vertex

  //empty line
  QgsCircularString l26;
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //valid line
  l26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                 << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );
  //out of range vertices
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( l26.numPoints(), 2 );
  QCOMPARE( l26.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( l26.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );

  //removing the next vertex removes all remaining vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( l26.numPoints(), 0 );
  QVERIFY( l26.isEmpty() );
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //reversed
  QgsCircularString l27;
  std::unique_ptr< QgsCircularString > reversed( l27.reversed() );
  QVERIFY( reversed->isEmpty() );
  l27.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  reversed.reset( l27.reversed() );
  QCOMPARE( reversed->numPoints(), 3 );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  QCOMPARE( reversed->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( reversed->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( reversed->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );

  //addZValue

  QgsCircularString l28;
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularStringZ );
  l28.clear();
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularStringZ );
  //2d line
  l28.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l28.addZValue( 2 ) );
  QVERIFY( l28.is3D() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  QVERIFY( !l28.addZValue( 4 ) ); //already has z value, test that existing z is unchanged
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  //linestring with m
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  QVERIFY( l28.addZValue( 5 ) );
  QVERIFY( l28.is3D() );
  QVERIFY( l28.isMeasure() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );

  //addMValue

  QgsCircularString l29;
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularStringM );
  l29.clear();
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularStringM );
  //2d line
  l29.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l29.addMValue( 2 ) );
  QVERIFY( !l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  QVERIFY( !l29.addMValue( 4 ) ); //already has m value, test that existing m is unchanged
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  //linestring with z
  l29.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) );
  QVERIFY( l29.addMValue( 5 ) );
  QVERIFY( l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );

  //dropZValue
  QgsCircularString l28d;
  QVERIFY( !l28d.dropZValue() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !l28d.dropZValue() );
  l28d.addZValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringZ );
  QVERIFY( l28d.is3D() );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropZValue() ); //already dropped
  //linestring with m
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QVERIFY( l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );

  //dropMValue
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !l28d.dropMValue() );
  l28d.addMValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringM );
  QVERIFY( l28d.isMeasure() );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropMValue() ); //already dropped
  //linestring with z
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QVERIFY( l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3, 0 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3, 0 ) );

  //convertTo
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularString ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularStringZ ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );

  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularStringZM ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2 ) );
  l28d.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 5 ) );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );
  //l28d.setMAt( 0, 6.0 );
  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularStringM ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2 ) );
  l28d.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 0, 6 ) );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularString ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( 1, 2 ) );
  QVERIFY( !l28d.convertTo( QgsWkbTypes::Polygon ) );

  //isRing
  QgsCircularString l30;
  QVERIFY( !l30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  QVERIFY( !l30.isRing() ); //<4 points
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  QVERIFY( !l30.isRing() ); //not closed
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  QVERIFY( l30.isRing() );

  //coordinateSequence
  QgsCircularString l31;
  QgsCoordinateSequence coords = l31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QVERIFY( coords.at( 0 ).at( 0 ).isEmpty() );
  l31.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  coords = l31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QCOMPARE( coords.at( 0 ).at( 0 ).count(), 3 );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );

  //nextVertex

  QgsCircularString l32;
  QgsVertexId v;
  QgsPoint p;
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !l32.nextVertex( v, p ) );

  //CircularString
  l32.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  v = QgsVertexId( 0, 0, 2 ); //out of range
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) ); //test that ring number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );

  //CircularStringZ
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //CircularStringM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //CircularStringZM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );

  //vertexAt and pointAt
  QgsCircularString l33;
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QgsVertexId::VertexType type;
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  //CircularString
  l33.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) );
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( l33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //CircularStringZ
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )  << QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( l33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22, 23 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //CircularStringM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) << QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( l33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //CircularStringZM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( l33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //centroid
  QgsCircularString l34;
  QCOMPARE( l34.centroid(), QgsPoint() );
  l34.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 5, 10 ) );
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 20, 10 ) << QgsPoint( 2, 9 ) );
  QgsPoint centroid = l34.centroid();
  QGSCOMPARENEAR( centroid.x(), 7.333, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 6.333, 0.001 );

  //closest segment
  QgsCircularString l35;
  int leftOf = 0;
  p = QgsPoint( 0, 0 ); // reset all coords to zero
  ( void )l35.closestSegment( QgsPoint( 1, 2 ), p, v ); //empty line, just want no crash
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QVERIFY( l35.closestSegment( QgsPoint( 5, 10 ), p, v ) < 0 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2.0, 0.0001 );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 5.5, 11.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 7, 16 ), p, v, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 5.5, 13.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( l35.closestSegment( QgsPoint( 5, 15 ), p, v, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  //clockwise string
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 15 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 10 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5, 0.01 );
  QGSCOMPARENEAR( p.y(), 10, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 5.5, 11.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 7, 16 ), p, v, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 5.5, 13.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  // point directly on segment
  QCOMPARE( l35.closestSegment( QgsPoint( 5, 15 ), p, v, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 0 );

  //sumUpArea
  QgsCircularString l36;
  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, 4.141593, 0.0001 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, 7.283185, 0.0001 );
  // full circle
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 4, 0 ) << QgsPoint( 0, 0 ) );
  area = 0.0;
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, 12.566370614359172, 0.0001 );

  //boundingBox - test that bounding box is updated after every modification to the circular string
  QgsCircularString l37;
  QVERIFY( l37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.setPoints( QgsPointSequence() << QgsPoint( -5, -10 ) << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );
  QByteArray wkbToAppend = l37.asWkb();
  l37.clear();
  QVERIFY( l37.boundingBox().isNull() );
  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.fromWkb( wkbToAppendPtr );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );
  l37.fromWkt( QStringLiteral( "CircularString( 5 10, 6 10, 5.5 9 )" ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 9, 6.125, 10.25 ) );
  l37.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QgsRectangle r = l37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -3.014, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 14.014, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), -7.0146, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 12.4988, 0.01 );
  l37.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  r = l37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -10.294, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 12.294, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 31.856, 0.01 );
  l37.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  r = l37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), 5, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 6.125, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 10.25, 0.01 );

  //angle
  QgsCircularString l38;
  ( void )l38.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141593, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 20 ) ); // no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 )
                 << QgsPoint( -1, 3 ) << QgsPoint( 0, 4 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 1.5708, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 4 ) << QgsPoint( -1, 3 ) << QgsPoint( 0, 2 )
                 << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 4.712389, 0.0001 );

  //closed circular string
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0, 0.00001 );

  //removing a vertex from a 3 point circular string should remove the whole line
  QgsCircularString l39;
  l39.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  QCOMPARE( l39.numPoints(), 3 );
  l39.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( l39.numPoints(), 0 );

  //boundary
  QgsCircularString boundary1;
  QVERIFY( !boundary1.boundary() );
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QgsAbstractGeometry *boundary = boundary1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->pointN( 0 )->x(), 0.0 );
  QCOMPARE( mpBoundary->pointN( 0 )->y(), 0.0 );
  QCOMPARE( mpBoundary->pointN( 1 )->x(), 1.0 );
  QCOMPARE( mpBoundary->pointN( 1 )->y(), 1.0 );
  delete boundary;

  // closed string = no boundary
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QVERIFY( !boundary1.boundary() );

  //boundary with z
  boundary1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundary = boundary1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->pointN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( mpBoundary->pointN( 0 )->x(), 0.0 );
  QCOMPARE( mpBoundary->pointN( 0 )->y(), 0.0 );
  QCOMPARE( mpBoundary->pointN( 0 )->z(), 10.0 );
  QCOMPARE( mpBoundary->pointN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );
  delete boundary;

  // addToPainterPath (note most tests are in test_qgsgeometry.py)
  QgsCircularString path;
  QPainterPath pPath;
  path.addToPainterPath( pPath );
  QVERIFY( pPath.isEmpty() );
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )
                  << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 ) );
  path.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 21.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 2.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );

  // even number of points - should still work
  pPath = QPainterPath();
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  path.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 11.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );

  // toCurveType
  QgsCircularString curveLine1;
  curveLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  std::unique_ptr< QgsCurve > curveType( curveLine1.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( curveType->numPoints(), 3 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );

  //segmentLength
  QgsCircularString curveLine2;
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  curveLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( -1, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( 1, 1, 0 ) ), 31.4159, 0.001 );
  curveLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) << QgsPoint( -9, 32 ) << QgsPoint( 1, 42 ) );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );

  //removeDuplicateNodes
  QgsCircularString nodeLine;
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "CircularString (11 2, 11 12, 111 12)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "CircularString (11 2, 11 12, 11 2)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 10, 3 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 9, 3 )
                      << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 10 3, 11.01 1.99, 9 3, 11 2)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99)" ) );
  QVERIFY( nodeLine.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11 12, 111 12, 111.01 11.99)" ) );

  // don't create degenerate lines
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99, 11 2)" ) );

  // with z
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 ) << QgsPoint( 11.02, 2.01, 3 )
                      << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) );
  QVERIFY( nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularStringZ (11 2 1, 11 12 4, 111 12 5)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 ) << QgsPoint( 11.02, 2.01, 3 )
                      << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02, true ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularStringZ (11 2 1, 11.01 1.99 2, 11.02 2.01 3, 11 12 4, 111 12 5)" ) );

  //swap xy
  QgsCircularString swapLine;
  swapLine.swapXy(); // no crash
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  swapLine.swapXy();
  QCOMPARE( swapLine.asWkt(), QStringLiteral( "CircularStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24)" ) );

  // filter vertex
  QgsCircularString filterLine;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() < 5;
  };
  filterLine.filterVertices( filter ); // no crash
  filterLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  filterLine.filterVertices( filter );
  QCOMPARE( filterLine.asWkt( 2 ), QStringLiteral( "CircularStringZM (1 2 3 4, 4 12 13 14)" ) );

  // transform vertex
  QgsCircularString transformLine;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 7 );
  };
  transformLine.transformVertices( transform ); // no crash
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  transformLine.transformVertices( transform );
  QCOMPARE( transformLine.asWkt( 2 ), QStringLiteral( "CircularStringZM (3 5 7 11, 6 15 17 21, 113 15 27 31)" ) );

  // transform using class
  QgsCircularString transformLine2;
  TestTransformer transformer;
  // no crash
  QVERIFY( transformLine2.transform( &transformer ) );
  transformLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  QVERIFY( transformLine2.transform( &transformer ) );
  QCOMPARE( transformLine2.asWkt( 2 ), QStringLiteral( "CircularStringZM (3 16 8 3, 12 26 18 13, 333 26 28 23)" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !transformLine2.transform( &failTransformer ) );

  // substring
  QgsCircularString substring;
  std::unique_ptr< QgsCircularString > substringResult( substring.curveSubstring( 1, 2 ) ); // no crash
  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );
  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)
  substring.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) <<  QgsPoint( 11, 1, 3, 4 ) <<  QgsPoint( 12, 0, 13, 14 ) );
  substringResult.reset( substring.curveSubstring( 0, 0 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 10 0 1 2, 10 0 1 2)" ) );
  substringResult.reset( substring.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 10.12 0.48 1.64 2.64, 10.46 0.84 2.27 3.27)" ) );
  substringResult.reset( substring.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.46 0.84 2.27 3.27, 10.46 0.84 2.27 3.27)" ) );
  substringResult.reset( substring.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)" ) );
  substringResult.reset( substring.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14)" ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14)" ) );
  substringResult.reset( substring.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.68 0.95 2.59 3.59, 10.93 1 2.91 3.91)" ) );
  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 14 -1 13 14, 16 1 23 24 )
  substring.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) <<  QgsPoint( 11, 1, 3, 4 ) <<  QgsPoint( 12, 0, 13, 14 )
                       <<  QgsPoint( 14, -1, 13, 14 ) <<  QgsPoint( 16, 1, 23, 24 ) );
  substringResult.reset( substring.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.68 0.95 2.59 3.59, 10.93 1 2.91 3.91)" ) );
  substringResult.reset( substring.curveSubstring( 1, 10 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14, 14 -1 13 14, 16 1 23 24)" ) );
  substringResult.reset( substring.curveSubstring( 1, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14, 13.1 -0.88 13 14, 14.5 -0.9 14.65 15.65)" ) );
  substringResult.reset( substring.curveSubstring( 0, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 13.1 -0.88 13 14, 14.5 -0.9 14.65 15.65)" ) );
  substringResult.reset( substring.curveSubstring( 5, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (13.51 -0.98 13 14, 14.01 -1 13.03 14.03, 14.5 -0.9 14.65 15.65)" ) );
  substringResult.reset( substring.curveSubstring( 5, 1000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (13.51 -0.98 13 14, 15.19 -0.53 17.2 18.2, 16 1 23 24)" ) );
  substringResult.reset( substring.curveSubstring( QgsGeometryUtils::distanceToVertex( substring, QgsVertexId( 0, 0, 2 ) ), QgsGeometryUtils::distanceToVertex( substring, QgsVertexId( 0, 0, 4 ) ) ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (12 0 13 14, 14.36 -0.94 14.19 15.19, 16 1 23 24)" ) );

  substring.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1 ) <<  QgsPoint( 11, 1, 3 ) <<  QgsPoint( 12, 0, 13 )
                       <<  QgsPoint( 14, -1, 13 ) <<  QgsPoint( 16, 1, 23 ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZ (10.46 0.84 2.27, 11.48 0.88 6.18, 12 0 13, 14 -1 13, 16 1 23)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 1 ) <<  QgsPoint( QgsWkbTypes::PointM, 11, 1, 0, 3 ) <<  QgsPoint( QgsWkbTypes::PointM, 12, 0, 0, 13 )
                       <<  QgsPoint( QgsWkbTypes::PointM, 14, -1, 0, 13 ) <<  QgsPoint( QgsWkbTypes::PointM, 16, 1, 0, 23 ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringM (10.46 0.84 2.27, 11.48 0.88 6.18, 12 0 13, 14 -1 13, 16 1 23)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( 10, 0 ) <<  QgsPoint( 11, 1 ) <<  QgsPoint( 12, 0 )
                       <<  QgsPoint( 14, -1 ) <<  QgsPoint( 16, 1 ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularString (10.46 0.84, 11.48 0.88, 12 0, 14 -1, 16 1)" ) );

  // interpolate
  QgsCircularString interpolate;
  std::unique_ptr< QgsPoint > interpolateResult( interpolate.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolateResult.get() );
  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) <<  QgsPoint( 11, 1, 3, 4 ) <<  QgsPoint( 12, 0, 13, 14 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 0 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10 0 1 2)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( -1 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 100000 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10.46 0.84 2.27 3.27)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10.93 1 2.91 3.91)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( interpolate.length() ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (12 0 13 14)" ) );
  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 14 -1 13 14, 16 1 23 24 )
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) <<  QgsPoint( 11, 1, 3, 4 ) <<  QgsPoint( 12, 0, 13, 14 )
                         <<  QgsPoint( 14, -1, 13, 14 ) <<  QgsPoint( 16, 1, 23, 24 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10.46 0.84 2.27 3.27)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10.93 1 2.91 3.91)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 10 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 6 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (14.5 -0.9 14.65 15.65)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (13.51 -0.98 13 14)" ) );

  interpolate.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1 ) <<  QgsPoint( 11, 1, 3 ) <<  QgsPoint( 12, 0, 13 )
                         <<  QgsPoint( 14, -1, 13 ) <<  QgsPoint( 16, 1, 23 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZ (10.46 0.84 2.27)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 1 ) <<  QgsPoint( QgsWkbTypes::PointM, 11, 1, 0, 3 ) <<  QgsPoint( QgsWkbTypes::PointM, 12, 0, 0, 13 )
                         <<  QgsPoint( QgsWkbTypes::PointM, 14, -1, 0, 13 ) <<  QgsPoint( QgsWkbTypes::PointM, 16, 1, 0, 23 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointM (10.46 0.84 2.27)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 10, 0 ) <<  QgsPoint( 11, 1 ) <<  QgsPoint( 12, 0 )
                         <<  QgsPoint( 14, -1 ) <<  QgsPoint( 16, 1 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "Point (10.46 0.84)" ) );

  // orientation
  QgsCircularString orientation;
  ( void )orientation.orientation(); // no crash
  orientation.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) << QgsPoint( 1, 1 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( orientation.orientation(), QgsCurve::Clockwise );
  orientation.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( orientation.orientation(), QgsCurve::CounterClockwise );
}

void TestQgsCircularString::circularStringFromArray()
{
  // test creating circular strings from arrays
  QVector< double > xx;
  xx << 1 << 2 << 3;
  QVector< double > yy;
  yy << 11 << 12 << 13;
  QgsCircularString fromArray( xx, yy );
  QCOMPARE( fromArray.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( fromArray.numPoints(), 3 );
  QCOMPARE( fromArray.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray.yAt( 2 ), 13.0 );

  // unbalanced
  xx = QVector< double >() << 1 << 2;
  yy = QVector< double >() << 11 << 12 << 13;
  QgsCircularString fromArray2( xx, yy );
  QCOMPARE( fromArray2.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( fromArray2.numPoints(), 2 );
  QCOMPARE( fromArray2.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray2.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray2.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray2.yAt( 1 ), 12.0 );
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12;
  QgsCircularString fromArray3( xx, yy );
  QCOMPARE( fromArray3.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( fromArray3.numPoints(), 2 );
  QCOMPARE( fromArray3.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray3.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray3.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray3.yAt( 1 ), 12.0 );
  // with z
  QVector< double > zz;
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  zz = QVector< double >() << 21 << 22 << 23;
  QgsCircularString fromArray4( xx, yy, zz );
  QCOMPARE( fromArray4.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( fromArray4.numPoints(), 3 );
  QCOMPARE( fromArray4.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray4.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray4.pointN( 0 ).z(), 21.0 );
  QCOMPARE( fromArray4.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray4.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray4.pointN( 1 ).z(), 22.0 );
  QCOMPARE( fromArray4.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray4.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray4.pointN( 2 ).z(), 23.0 );

  // unbalanced -> z ignored
  zz = QVector< double >() << 21 << 22;
  QgsCircularString fromArray5( xx, yy, zz );
  QCOMPARE( fromArray5.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( fromArray5.numPoints(), 3 );
  QCOMPARE( fromArray5.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray5.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray5.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray5.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray5.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray5.yAt( 2 ), 13.0 );
  // unbalanced -> z truncated
  zz = QVector< double >() << 21 << 22 << 23 << 24;
  fromArray5 = QgsCircularString( xx, yy, zz );
  QCOMPARE( fromArray5.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( fromArray5.numPoints(), 3 );
  QCOMPARE( fromArray5.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray5.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray5.pointN( 0 ).z(), 21.0 );
  QCOMPARE( fromArray5.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray5.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray5.pointN( 1 ).z(), 22.0 );
  QCOMPARE( fromArray5.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray5.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray5.pointN( 2 ).z(), 23.0 );

  // with m
  QVector< double > mm;
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  mm = QVector< double >() << 21 << 22 << 23;
  QgsCircularString fromArray6( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray6.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( fromArray6.numPoints(), 3 );
  QCOMPARE( fromArray6.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray6.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray6.pointN( 0 ).m(), 21.0 );
  QCOMPARE( fromArray6.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray6.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray6.pointN( 1 ).m(), 22.0 );
  QCOMPARE( fromArray6.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray6.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray6.pointN( 2 ).m(), 23.0 );
  // unbalanced -> m ignored
  mm = QVector< double >() << 21 << 22;
  QgsCircularString fromArray7( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray7.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( fromArray7.numPoints(), 3 );
  QCOMPARE( fromArray7.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray7.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray7.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray7.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray7.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray7.yAt( 2 ), 13.0 );
  // unbalanced -> m truncated
  mm = QVector< double >() << 21 << 22 << 23 << 24;
  fromArray7 = QgsCircularString( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray7.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( fromArray7.numPoints(), 3 );
  QCOMPARE( fromArray7.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray7.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray7.pointN( 0 ).m(), 21.0 );
  QCOMPARE( fromArray7.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray7.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray7.pointN( 1 ).m(), 22.0 );
  QCOMPARE( fromArray7.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray7.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray7.pointN( 2 ).m(), 23.0 );

  // zm
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  zz = QVector< double >() << 21 << 22 << 23;
  mm = QVector< double >() << 31 << 32 << 33;
  QgsCircularString fromArray8( xx, yy, zz, mm );
  QCOMPARE( fromArray8.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( fromArray8.numPoints(), 3 );
  QCOMPARE( fromArray8.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray8.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray8.pointN( 0 ).z(), 21.0 );
  QCOMPARE( fromArray8.pointN( 0 ).m(), 31.0 );
  QCOMPARE( fromArray8.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray8.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray8.pointN( 1 ).z(), 22.0 );
  QCOMPARE( fromArray8.pointN( 1 ).m(), 32.0 );
  QCOMPARE( fromArray8.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray8.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray8.pointN( 2 ).z(), 23.0 );
  QCOMPARE( fromArray8.pointN( 2 ).m(), 33.0 );
}

void TestQgsCircularString::circularStringAppend()
{
  //append circularstring

  //append to empty
  QgsCircularString l10;
  l10.append( nullptr );
  QVERIFY( l10.isEmpty() );
  QCOMPARE( l10.numPoints(), 0 );

  std::unique_ptr<QgsCircularString> toAppend( new QgsCircularString() );
  l10.append( toAppend.get() );
  QVERIFY( l10.isEmpty() );
  QCOMPARE( l10.numPoints(), 0 );

  toAppend->setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 12 )
                       << QgsPoint( 21, 22 ) );
  l10.append( toAppend.get() );
  QVERIFY( !l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.vertexCount(), 3 );
  QCOMPARE( l10.nCoordinates(), 3 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //add more points
  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 21, 22 )
                       << QgsPoint( 41, 42 )
                       << QgsPoint( 51, 52 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.numPoints(), 5 );
  QCOMPARE( l10.vertexCount(), 5 );
  QCOMPARE( l10.nCoordinates(), 5 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 3 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 4 ), toAppend->pointN( 2 ) );

  //check dimensionality is inherited from append line if initially empty
  l10.clear();
  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 53, 54 ) );
  l10.append( toAppend.get() );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //append points with z to non z circular string
  l10.clear();
  l10.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 31, 32 )
                 << QgsPoint( QgsWkbTypes::Point, 41, 42 )
                 << QgsPoint( QgsWkbTypes::Point, 51, 52 ) );
  QVERIFY( !l10.is3D() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::CircularString );
  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 141, 142, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 151, 152, 53, 54 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( 31, 32 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( 41, 42 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPoint( 51, 52 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPoint( 141, 142 ) );
  QCOMPARE( l10.pointN( 4 ), QgsPoint( 151, 152 ) );

  //append points without z/m to circularstring with z & m
  l10.clear();
  l10.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 11, 21 )
                 << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 12, 22 )
                 << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 13, 23 ) );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::CircularStringZM );
  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 51, 52 )
                       << QgsPoint( 141, 142 )
                       << QgsPoint( 151, 152 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 11, 21 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 41, 42, 12, 22 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 51, 52, 13, 23 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 141, 142 ) );
  QCOMPARE( l10.pointN( 4 ), QgsPoint( QgsWkbTypes::PointZM, 151, 152 ) );

  //append another line the closes the original geometry.
  //Make sure there are not duplicate points except start and end point
  l10.clear();
  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 1, 1 )
                       << QgsPoint( 5, 5 )
                       << QgsPoint( 10, 1 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.vertexCount(), 3 );
  toAppend.reset( new QgsCircularString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 10, 1 )
                       << QgsPoint( 5, 2 )
                       << QgsPoint( 1, 1 ) );
  l10.append( toAppend.get() );

  QVERIFY( l10.isClosed() );
  QCOMPARE( l10.numPoints(), 5 );
  QCOMPARE( l10.vertexCount(), 5 );
}

QGSTEST_MAIN( TestQgsCircularString )
#include "testqgscircularstring.moc"
