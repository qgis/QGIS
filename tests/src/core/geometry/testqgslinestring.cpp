/***************************************************************************
     testqgslinestring.cpp
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
#include "qgsproject.h"
#include "qgspoint.h"
#include "qgsmultipoint.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "qgsgeometryutils.h"
#include "qgslinesegment.h"
#include "testtransformer.h"
#include "testgeometryutils.h"


class TestQgsLineString: public QObject
{
    Q_OBJECT
  private slots:
    void lineString();
};


void TestQgsLineString::lineString()
{
  //test constructors
  QgsLineString l1;
  QVERIFY( l1.isEmpty() );
  QCOMPARE( l1.numPoints(), 0 );
  QCOMPARE( l1.vertexCount(), 0 );
  QCOMPARE( l1.nCoordinates(), 0 );
  QCOMPARE( l1.ringCount(), 0 );
  QCOMPARE( l1.partCount(), 0 );
  QVERIFY( !l1.is3D() );
  QVERIFY( !l1.isMeasure() );
  QCOMPARE( l1.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l1.wktTypeStr(), QString( "LineString" ) );
  QCOMPARE( l1.geometryType(), QString( "LineString" ) );
  QCOMPARE( l1.dimension(), 1 );
  QVERIFY( !l1.hasCurvedSegments() );
  QCOMPARE( l1.area(), 0.0 );
  QCOMPARE( l1.perimeter(), 0.0 );

  // from array
  QVector< double > xx;
  xx << 1 << 2 << 3;
  QVector< double > yy;
  yy << 11 << 12 << 13;
  QgsLineString fromArray( xx, yy );
  QCOMPARE( fromArray.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray.numPoints(), 3 );
  QCOMPARE( fromArray.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray.yAt( 2 ), 13.0 );
  QCOMPARE( *fromArray.xData(), 1.0 );
  QCOMPARE( *( fromArray.xData() + 1 ), 2.0 );
  QCOMPARE( *( fromArray.xData() + 2 ), 3.0 );
  QCOMPARE( *fromArray.yData(), 11.0 );
  QCOMPARE( *( fromArray.yData() + 1 ), 12.0 );
  QCOMPARE( *( fromArray.yData() + 2 ), 13.0 );

  // unbalanced
  xx = QVector< double >() << 1 << 2;
  yy = QVector< double >() << 11 << 12 << 13;
  QgsLineString fromArray2( xx, yy );
  QCOMPARE( fromArray2.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray2.numPoints(), 2 );
  QCOMPARE( fromArray2.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray2.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray2.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray2.yAt( 1 ), 12.0 );
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12;
  QgsLineString fromArray3( xx, yy );
  QCOMPARE( fromArray3.wkbType(), QgsWkbTypes::LineString );
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
  QgsLineString fromArray4( xx, yy, zz );
  QCOMPARE( fromArray4.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( fromArray4.numPoints(), 3 );
  QCOMPARE( fromArray4.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray4.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray4.zAt( 0 ), 21.0 );
  QCOMPARE( fromArray4.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray4.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray4.zAt( 1 ), 22.0 );
  QCOMPARE( fromArray4.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray4.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray4.zAt( 2 ), 23.0 );
  fromArray4 = QgsLineString( xx, yy, zz, QVector< double >(), true );  // LineString25D
  QCOMPARE( fromArray4.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( fromArray4.numPoints(), 3 );
  QCOMPARE( fromArray4.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray4.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray4.zAt( 0 ), 21.0 );
  QCOMPARE( fromArray4.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray4.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray4.zAt( 1 ), 22.0 );
  QCOMPARE( fromArray4.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray4.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray4.zAt( 2 ), 23.0 );

  // unbalanced -> z ignored
  zz = QVector< double >() << 21 << 22;
  QgsLineString fromArray5( xx, yy, zz );
  QCOMPARE( fromArray5.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray5.numPoints(), 3 );
  QCOMPARE( fromArray5.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray5.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray5.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray5.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray5.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray5.yAt( 2 ), 13.0 );
  // unbalanced -> z truncated
  zz = QVector< double >() << 21 << 22 << 23 << 24;
  fromArray5 = QgsLineString( xx, yy, zz );
  QCOMPARE( fromArray5.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( fromArray5.numPoints(), 3 );
  QCOMPARE( fromArray5.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray5.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray5.zAt( 0 ), 21.0 );
  QCOMPARE( fromArray5.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray5.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray5.zAt( 1 ), 22.0 );
  QCOMPARE( fromArray5.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray5.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray5.zAt( 2 ), 23.0 );
  // with m
  QVector< double > mm;
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  mm = QVector< double >() << 21 << 22 << 23;
  QgsLineString fromArray6( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray6.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( fromArray6.numPoints(), 3 );
  QCOMPARE( fromArray6.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray6.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray6.mAt( 0 ), 21.0 );
  QCOMPARE( fromArray6.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray6.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray6.mAt( 1 ), 22.0 );
  QCOMPARE( fromArray6.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray6.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray6.mAt( 2 ), 23.0 );
  // unbalanced -> m ignored
  mm = QVector< double >() << 21 << 22;
  QgsLineString fromArray7( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray7.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray7.numPoints(), 3 );
  QCOMPARE( fromArray7.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray7.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray7.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray7.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray7.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray7.yAt( 2 ), 13.0 );
  // unbalanced -> m truncated
  mm = QVector< double >() << 21 << 22 << 23 << 24;
  fromArray7 = QgsLineString( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray7.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( fromArray7.numPoints(), 3 );
  QCOMPARE( fromArray7.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray7.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray7.mAt( 0 ), 21.0 );
  QCOMPARE( fromArray7.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray7.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray7.mAt( 1 ), 22.0 );
  QCOMPARE( fromArray7.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray7.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray7.mAt( 2 ), 23.0 );
  // zm
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  zz = QVector< double >() << 21 << 22 << 23;
  mm = QVector< double >() << 31 << 32 << 33;
  QgsLineString fromArray8( xx, yy, zz, mm );
  QCOMPARE( fromArray8.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( fromArray8.numPoints(), 3 );
  QCOMPARE( fromArray8.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray8.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray8.zAt( 0 ), 21.0 );
  QCOMPARE( fromArray8.mAt( 0 ), 31.0 );
  QCOMPARE( fromArray8.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray8.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray8.zAt( 1 ), 22.0 );
  QCOMPARE( fromArray8.mAt( 1 ), 32.0 );
  QCOMPARE( fromArray8.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray8.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray8.zAt( 2 ), 23.0 );
  QCOMPARE( fromArray8.mAt( 2 ), 33.0 );

  QCOMPARE( *fromArray8.xData(), 1.0 );
  QCOMPARE( *( fromArray8.xData() + 1 ), 2.0 );
  QCOMPARE( *( fromArray8.xData() + 2 ), 3.0 );
  QCOMPARE( *fromArray8.yData(), 11.0 );
  QCOMPARE( *( fromArray8.yData() + 1 ), 12.0 );
  QCOMPARE( *( fromArray8.yData() + 2 ), 13.0 );
  QCOMPARE( *fromArray8.zData(), 21.0 );
  QCOMPARE( *( fromArray8.zData() + 1 ), 22.0 );
  QCOMPARE( *( fromArray8.zData() + 2 ), 23.0 );
  QCOMPARE( *fromArray8.mData(), 31.0 );
  QCOMPARE( *( fromArray8.mData() + 1 ), 32.0 );
  QCOMPARE( *( fromArray8.mData() + 2 ), 33.0 );

  // from QList<QgsPointXY>
  QgsLineString fromPtsA = QgsLineString( QgsPointSequence() );
  QVERIFY( fromPtsA.isEmpty() );
  QCOMPARE( fromPtsA.wkbType(), QgsWkbTypes::LineString );

  fromPtsA = QgsLineString( QgsPointSequence()  << QgsPoint( 1, 2, 0, 4, QgsWkbTypes::PointM ) );
  QCOMPARE( fromPtsA.numPoints(), 1 );
  QCOMPARE( fromPtsA.wkbType(), QgsWkbTypes::LineStringM );

  QVector<QgsPointXY> ptsA;
  ptsA << QgsPointXY( 1, 2 ) << QgsPointXY( 11, 12 ) << QgsPointXY( 21, 22 );
  QgsLineString fromPts( ptsA );
  QCOMPARE( fromPts.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromPts.numPoints(), 3 );
  QCOMPARE( fromPts.xAt( 0 ), 1.0 );
  QCOMPARE( fromPts.yAt( 0 ), 2.0 );
  QCOMPARE( fromPts.xAt( 1 ), 11.0 );
  QCOMPARE( fromPts.yAt( 1 ), 12.0 );
  QCOMPARE( fromPts.xAt( 2 ), 21.0 );
  QCOMPARE( fromPts.yAt( 2 ), 22.0 );

  // from QgsPointSequence
  QgsPointSequence ptsVector;
  ptsVector << QgsPoint( 10, 20 ) << QgsPoint( 30, 40 );
  QgsLineString fromVector( ptsVector );
  QCOMPARE( fromVector.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromVector.numPoints(), 2 );
  QCOMPARE( fromVector.xAt( 0 ), 10.0 );
  QCOMPARE( fromVector.yAt( 0 ), 20.0 );
  QCOMPARE( fromVector.xAt( 1 ), 30.0 );
  QCOMPARE( fromVector.yAt( 1 ), 40.0 );
  QgsPointSequence ptsVector3D;
  ptsVector3D << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 30, 40, 200 );
  QgsLineString fromVector3D( ptsVector3D );
  QCOMPARE( fromVector3D.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( fromVector3D.numPoints(), 2 );
  QCOMPARE( fromVector3D.xAt( 0 ), 10.0 );
  QCOMPARE( fromVector3D.yAt( 0 ), 20.0 );
  QCOMPARE( fromVector3D.zAt( 0 ), 100.0 );
  QCOMPARE( fromVector3D.xAt( 1 ), 30.0 );
  QCOMPARE( fromVector3D.yAt( 1 ), 40.0 );
  QCOMPARE( fromVector3D.zAt( 1 ), 200.0 );

  // from 2 points
  QgsLineString from2Pts( QgsPoint( 1, 2 ), QgsPoint( 21, 22 ) );
  QCOMPARE( from2Pts.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( from2Pts.numPoints(), 2 );
  QCOMPARE( from2Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from2Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from2Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from2Pts.yAt( 1 ), 22.0 );
  from2Pts = QgsLineString( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZ, 21, 22, 23 ) );
  QCOMPARE( from2Pts.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( from2Pts.numPoints(), 2 );
  QCOMPARE( from2Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from2Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from2Pts.zAt( 0 ), 3.0 );
  QCOMPARE( from2Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from2Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from2Pts.zAt( 1 ), 23.0 );
  from2Pts = QgsLineString( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ), QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 23 ) );
  QCOMPARE( from2Pts.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( from2Pts.numPoints(), 2 );
  QCOMPARE( from2Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from2Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from2Pts.mAt( 0 ), 3.0 );
  QCOMPARE( from2Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from2Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from2Pts.mAt( 1 ), 23.0 );
  from2Pts = QgsLineString( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  QCOMPARE( from2Pts.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( from2Pts.numPoints(), 2 );
  QCOMPARE( from2Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from2Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from2Pts.zAt( 0 ), 3.0 );
  QCOMPARE( from2Pts.mAt( 0 ), 4.0 );
  QCOMPARE( from2Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from2Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from2Pts.zAt( 1 ), 23.0 );
  QCOMPARE( from2Pts.mAt( 1 ), 24.0 );

  // from lineSegment
  QgsLineString fromSegment( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ) );
  QCOMPARE( fromSegment.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromSegment.numPoints(), 2 );
  QCOMPARE( fromSegment.xAt( 0 ), 1.0 );
  QCOMPARE( fromSegment.yAt( 0 ), 2.0 );
  QCOMPARE( fromSegment.xAt( 1 ), 3.0 );
  QCOMPARE( fromSegment.yAt( 1 ), 4.0 );

  //addVertex
  QgsLineString l2;
  l2.addVertex( QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !l2.isEmpty() );
  QCOMPARE( l2.numPoints(), 1 );
  QCOMPARE( l2.vertexCount(), 1 );
  QCOMPARE( l2.nCoordinates(), 1 );
  QCOMPARE( l2.ringCount(), 1 );
  QCOMPARE( l2.partCount(), 1 );
  QVERIFY( !l2.is3D() );
  QVERIFY( !l2.isMeasure() );
  QCOMPARE( l2.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !l2.hasCurvedSegments() );
  QCOMPARE( l2.area(), 0.0 );
  QCOMPARE( l2.perimeter(), 0.0 );

  //adding first vertex should set linestring z/m type
  QgsLineString l3;
  l3.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QVERIFY( !l3.isEmpty() );
  QVERIFY( l3.is3D() );
  QVERIFY( !l3.isMeasure() );
  QCOMPARE( l3.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( l3.wktTypeStr(), QString( "LineStringZ" ) );

  QgsLineString l4;
  l4.addVertex( QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QVERIFY( !l4.isEmpty() );
  QVERIFY( !l4.is3D() );
  QVERIFY( l4.isMeasure() );
  QCOMPARE( l4.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( l4.wktTypeStr(), QString( "LineStringM" ) );

  QgsLineString l5;
  l5.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( !l5.isEmpty() );
  QVERIFY( l5.is3D() );
  QVERIFY( l5.isMeasure() );
  QCOMPARE( l5.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l5.wktTypeStr(), QString( "LineStringZM" ) );

  QgsLineString l25d;
  l25d.addVertex( QgsPoint( QgsWkbTypes::Point25D, 1.0, 2.0, 3.0 ) );
  QVERIFY( !l25d.isEmpty() );
  QVERIFY( l25d.is3D() );
  QVERIFY( !l25d.isMeasure() );
  QCOMPARE( l25d.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l25d.wktTypeStr(), QString( "LineStringZ" ) );

  //adding subsequent vertices should not alter z/m type, regardless of points type
  QgsLineString l6;
  l6.addVertex( QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) ); //2d type
  QCOMPARE( l6.wkbType(), QgsWkbTypes::LineString );
  l6.addVertex( QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0, 13.0 ) ); // add 3d point
  QCOMPARE( l6.numPoints(), 2 );
  QCOMPARE( l6.vertexCount(), 2 );
  QCOMPARE( l6.nCoordinates(), 2 );
  QCOMPARE( l6.ringCount(), 1 );
  QCOMPARE( l6.partCount(), 1 );
  QCOMPARE( l6.wkbType(), QgsWkbTypes::LineString ); //should still be 2d
  QVERIFY( !l6.is3D() );
  QCOMPARE( l6.area(), 0.0 );
  QCOMPARE( l6.perimeter(), 0.0 );

  QgsLineString l7;
  l7.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) ); //3d type
  QCOMPARE( l7.wkbType(), QgsWkbTypes::LineStringZ );
  l7.addVertex( QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) ); //add 2d point
  QCOMPARE( l7.wkbType(), QgsWkbTypes::LineStringZ ); //should still be 3d
  QCOMPARE( l7.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0 ) );
  QVERIFY( l7.is3D() );
  QCOMPARE( l7.numPoints(), 2 );
  QCOMPARE( l7.vertexCount(), 2 );
  QCOMPARE( l7.nCoordinates(), 2 );
  QCOMPARE( l7.ringCount(), 1 );
  QCOMPARE( l7.partCount(), 1 );

  //clear
  l7.clear();
  QVERIFY( l7.isEmpty() );
  QCOMPARE( l7.numPoints(), 0 );
  QCOMPARE( l7.vertexCount(), 0 );
  QCOMPARE( l7.nCoordinates(), 0 );
  QCOMPARE( l7.ringCount(), 0 );
  QCOMPARE( l7.partCount(), 0 );
  QVERIFY( !l7.is3D() );
  QVERIFY( !l7.isMeasure() );
  QCOMPARE( l7.wkbType(), QgsWkbTypes::LineString );

  //setPoints
  QgsLineString l8;
  l8.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QVERIFY( !l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 3 );
  QCOMPARE( l8.vertexCount(), 3 );
  QCOMPARE( l8.nCoordinates(), 3 );
  QCOMPARE( l8.ringCount(), 1 );
  QCOMPARE( l8.partCount(), 1 );
  QVERIFY( !l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !l8.hasCurvedSegments() );
  QgsPointSequence pts;
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QCOMPARE( *l8.xData(), 1.0 );
  QCOMPARE( *( l8.xData() + 1 ), 2.0 );
  QCOMPARE( *( l8.xData() + 2 ), 3.0 );
  QCOMPARE( *l8.yData(), 2.0 );
  QCOMPARE( *( l8.yData() + 1 ), 3.0 );
  QCOMPARE( *( l8.yData() + 2 ), 4.0 );

  //setPoints with empty list, should clear linestring
  l8.setPoints( QgsPointSequence() );
  QVERIFY( l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 0 );
  QCOMPARE( l8.vertexCount(), 0 );
  QCOMPARE( l8.nCoordinates(), 0 );
  QCOMPARE( l8.ringCount(), 0 );
  QCOMPARE( l8.partCount(), 0 );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString );
  l8.points( pts );
  QVERIFY( pts.isEmpty() );

  //setPoints with z
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZ );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //setPoints with 25d
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l8.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 2, 3, 4 ) );

  //setPoints with m
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( !l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //setPoints with zm
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );

  //setPoints with MIXED dimensionality of points
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //test point
  QCOMPARE( l8.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) );
  QCOMPARE( l8.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //out of range - just want no crash here
  QgsPoint bad = l8.pointN( -1 );
  bad = l8.pointN( 100 );

  //test getters/setters
  QgsLineString l9;
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  QCOMPARE( l9.xAt( 0 ), 1.0 );
  QCOMPARE( l9.xAt( 1 ), 11.0 );
  QCOMPARE( l9.xAt( 2 ), 21.0 );
  QCOMPARE( l9.xAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.xAt( 11 ), 0.0 ); //out of range

  l9.setXAt( 0, 51.0 );
  QCOMPARE( l9.xAt( 0 ), 51.0 );
  l9.setXAt( 1, 61.0 );
  QCOMPARE( l9.xAt( 1 ), 61.0 );
  l9.setXAt( -1, 51.0 ); //out of range
  l9.setXAt( 11, 51.0 ); //out of range

  QCOMPARE( l9.yAt( 0 ), 2.0 );
  QCOMPARE( l9.yAt( 1 ), 12.0 );
  QCOMPARE( l9.yAt( 2 ), 22.0 );
  QCOMPARE( l9.yAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.yAt( 11 ), 0.0 ); //out of range

  l9.setYAt( 0, 52.0 );
  QCOMPARE( l9.yAt( 0 ), 52.0 );
  l9.setYAt( 1, 62.0 );
  QCOMPARE( l9.yAt( 1 ), 62.0 );
  l9.setYAt( -1, 52.0 ); //out of range
  l9.setYAt( 11, 52.0 ); //out of range

  QCOMPARE( l9.zAt( 0 ), 3.0 );
  QCOMPARE( l9.zAt( 1 ), 13.0 );
  QCOMPARE( l9.zAt( 2 ), 23.0 );
  QVERIFY( std::isnan( l9.zAt( -1 ) ) ); //out of range
  QVERIFY( std::isnan( l9.zAt( 11 ) ) ); //out of range

  l9.setZAt( 0, 53.0 );
  QCOMPARE( l9.zAt( 0 ), 53.0 );
  l9.setZAt( 1, 63.0 );
  QCOMPARE( l9.zAt( 1 ), 63.0 );
  l9.setZAt( -1, 53.0 ); //out of range
  l9.setZAt( 11, 53.0 ); //out of range

  QCOMPARE( l9.mAt( 0 ), 4.0 );
  QCOMPARE( l9.mAt( 1 ), 14.0 );
  QCOMPARE( l9.mAt( 2 ), 24.0 );
  QVERIFY( std::isnan( l9.mAt( -1 ) ) ); //out of range
  QVERIFY( std::isnan( l9.mAt( 11 ) ) ); //out of range

  l9.setMAt( 0, 54.0 );
  QCOMPARE( l9.mAt( 0 ), 54.0 );
  l9.setMAt( 1, 64.0 );
  QCOMPARE( l9.mAt( 1 ), 64.0 );
  l9.setMAt( -1, 54.0 ); //out of range
  l9.setMAt( 11, 54.0 ); //out of range

  //check zAt/setZAt with non-3d linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 )
                << QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 24 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( l9.zAt( 0 ) ) );
  QVERIFY( std::isnan( l9.zAt( 1 ) ) );
  l9.setZAt( 0, 53.0 );
  l9.setZAt( 1, 63.0 );

  //check mAt/setMAt with non-measure linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( l9.mAt( 0 ) ) );
  QVERIFY( std::isnan( l9.mAt( 1 ) ) );
  l9.setMAt( 0, 53.0 );
  l9.setMAt( 1, 63.0 );

  //append linestring

  //append to empty
  QgsLineString l10;
  l10.append( nullptr );
  QVERIFY( l10.isEmpty() );
  QCOMPARE( l10.numPoints(), 0 );

  std::unique_ptr<QgsLineString> toAppend( new QgsLineString() );
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
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //add more points
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                       << QgsPoint( 41, 42 )
                       << QgsPoint( 51, 52 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.numPoints(), 6 );
  QCOMPARE( l10.vertexCount(), 6 );
  QCOMPARE( l10.nCoordinates(), 6 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.pointN( 3 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 4 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 5 ), toAppend->pointN( 2 ) );

  //check dimensionality is inherited from append line if initially empty
  l10.clear();
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 53, 54 ) );
  l10.append( toAppend.get() );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //append points with z to non z linestring
  l10.clear();
  l10.addVertex( QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !l10.is3D() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString );
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 53, 54 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( 31, 32 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPoint( 41, 42 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPoint( 51, 52 ) );

  //append points without z/m to linestring with z & m
  l10.clear();
  l10.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineStringZM );
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                       << QgsPoint( 41, 42 )
                       << QgsPoint( 51, 52 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 41, 42 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 51, 52 ) );

  //25d append
  l10.clear();
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 )
                       << QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );
  l10.append( toAppend.get() );
  QVERIFY( l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );
  l10.clear();
  l10.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 33 ) );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString25D );
  l10.append( toAppend.get() );
  QVERIFY( l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 33 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );

  //append another line the closes the original geometry.
  //Make sure there are not duplicit points except start and end point
  l10.clear();
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 1, 1 )
                       << QgsPoint( 5, 5 )
                       << QgsPoint( 10, 1 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.vertexCount(), 3 );
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 10, 1 )
                       << QgsPoint( 1, 1 ) );
  l10.append( toAppend.get() );

  QVERIFY( l10.isClosed() );
  QCOMPARE( l10.numPoints(), 4 );
  QCOMPARE( l10.vertexCount(), 4 );

  //equality
  QgsLineString e1;
  QgsLineString e2;
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPoint( 1, 2 ) );
  QVERIFY( !( e1 == e2 ) ); //different number of vertices
  QVERIFY( e1 != e2 );
  e2.addVertex( QgsPoint( 1, 2 ) );
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  e2.addVertex( QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  QVERIFY( e1 == e2 ); //check non-integer equality
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPoint( 7, 8 ) );
  e2.addVertex( QgsPoint( 6, 9 ) );
  QVERIFY( !( e1 == e2 ) ); //different coordinates
  QVERIFY( e1 != e2 );
  QgsLineString e3;
  e3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );
  QVERIFY( !( e1 == e3 ) ); //different dimension
  QVERIFY( e1 != e3 );
  QgsLineString e4;
  e4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );
  QVERIFY( !( e3 == e4 ) ); //different z coordinates
  QVERIFY( e3 != e4 );
  QgsLineString e5;
  e5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  QgsLineString e6;
  e6.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );
  QVERIFY( !( e5 == e6 ) ); //different m values
  QVERIFY( e5 != e6 );

  QVERIFY( e6 != QgsCircularString() );
  QgsPoint p1;
  QVERIFY( !( e6 == p1 ) );
  QVERIFY( e6 != p1 );
  QVERIFY( e6 == e6 );

  //close/isClosed
  QgsLineString l11;
  QVERIFY( !l11.isClosed() );
  l11.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  QVERIFY( !l11.isClosed() );
  QCOMPARE( l11.numPoints(), 4 );
  QCOMPARE( l11.area(), 0.0 );
  QCOMPARE( l11.perimeter(), 0.0 );
  l11.close();
  QVERIFY( l11.isClosed() );
  QCOMPARE( l11.numPoints(), 5 );
  QCOMPARE( l11.vertexCount(), 5 );
  QCOMPARE( l11.nCoordinates(), 5 );
  QCOMPARE( l11.ringCount(), 1 );
  QCOMPARE( l11.partCount(), 1 );
  QCOMPARE( l11.pointN( 4 ), QgsPoint( 1, 2 ) );
  QCOMPARE( l11.area(), 0.0 );
  QCOMPARE( l11.perimeter(), 0.0 );
  //try closing already closed line, should be no change
  l11.close();
  QVERIFY( l11.isClosed() );
  QCOMPARE( l11.numPoints(), 5 );
  QCOMPARE( l11.pointN( 4 ), QgsPoint( 1, 2 ) );

  // tiny differences
  l11.setPoints( QgsPointSequence() << QgsPoint( 0.000000000000001, 0.000000000000002 )
                 << QgsPoint( 0.000000000000011, 0.000000000000002 )
                 << QgsPoint( 0.000000000000011, 0.000000000000022 )
                 << QgsPoint( 0.000000000000001, 0.000000000000022 ) );
  QVERIFY( !l11.isClosed() );
  l11.close();
  QVERIFY( l11.isClosed() );
  QCOMPARE( l11.numPoints(), 5 );
  QGSCOMPARENEAR( l11.pointN( 4 ).x(), 0.000000000000001, 0.00000000000000001 );
  QGSCOMPARENEAR( l11.pointN( 4 ).y(), 0.000000000000002, 0.00000000000000001 );

  //test that m values aren't considered when testing for closedness
  l11.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                 << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );
  QVERIFY( l11.isClosed() );

  //close with z and m
  QgsLineString l12;
  l12.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  l12.close();
  QCOMPARE( l12.pointN( 4 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );


  //polygonf
  QgsLineString l13;
  l13.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

  QPolygonF poly = l13.asQPolygonF();
  QCOMPARE( poly.count(), 4 );
  QCOMPARE( poly.at( 0 ).x(), 1.0 );
  QCOMPARE( poly.at( 0 ).y(), 2.0 );
  QCOMPARE( poly.at( 1 ).x(), 11.0 );
  QCOMPARE( poly.at( 1 ).y(), 2.0 );
  QCOMPARE( poly.at( 2 ).x(), 11.0 );
  QCOMPARE( poly.at( 2 ).y(), 22.0 );
  QCOMPARE( poly.at( 3 ).x(), 1.0 );
  QCOMPARE( poly.at( 3 ).y(), 22.0 );

  // clone tests. At the same time, check segmentize as the result should
  // be equal to a clone for LineStrings
  QgsLineString l14;
  l14.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  std::unique_ptr<QgsLineString> cloned( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->vertexCount(), 4 );
  QCOMPARE( cloned->ringCount(), 1 );
  QCOMPARE( cloned->partCount(), 1 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );
  std::unique_ptr< QgsLineString > segmentized( static_cast< QgsLineString * >( l14.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 4 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( segmentized->pointN( 3 ), l14.pointN( 3 ) );

  //clone with Z/M
  l14.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  cloned.reset( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( cloned->is3D() );
  QVERIFY( cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );
  segmentized.reset( static_cast< QgsLineString * >( l14.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 4 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( segmentized->pointN( 3 ), l14.pointN( 3 ) );

  //clone an empty line
  l14.clear();
  cloned.reset( l14.clone() );
  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineString );
  segmentized.reset( static_cast< QgsLineString * >( l14.segmentize() ) );
  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );

  //to/from WKB
  QgsLineString l15;
  l15.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QByteArray wkb15 = l15.asWkb();
  QCOMPARE( wkb15.size(), l15.wkbSize() );
  QgsLineString l16;
  QgsConstWkbPtr wkb15ptr( wkb15 );
  l16.fromWkb( wkb15ptr );
  QCOMPARE( l16.numPoints(), 4 );
  QCOMPARE( l16.vertexCount(), 4 );
  QCOMPARE( l16.nCoordinates(), 4 );
  QCOMPARE( l16.ringCount(), 1 );
  QCOMPARE( l16.partCount(), 1 );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::LineStringZM );
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
  QCOMPARE( l16.wkbType(), QgsWkbTypes::LineString );
  QgsPoint point( 1, 2 );
  QByteArray wkb16 = point.asWkb();
  QgsConstWkbPtr wkb16ptr( wkb16 );
  QVERIFY( !l16.fromWkb( wkb16ptr ) );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::LineString );

  //to/from WKT
  QgsLineString l17;
  l17.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

  QString wkt = l17.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsLineString l18;
  QVERIFY( l18.fromWkt( wkt ) );
  QCOMPARE( l18.numPoints(), 4 );
  QCOMPARE( l18.wkbType(), QgsWkbTypes::LineStringZM );
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
  QCOMPARE( l18.wkbType(), QgsWkbTypes::LineString );

  //asGML2
  QgsLineString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 )
                        << QgsPoint( 51, 52 ) );
  QgsLineString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLine.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLineFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsLineString().asGml2( doc ) ), expectedGML2empty );

  //asGML3
  QString expectedGML3( QStringLiteral( "<LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></LineString>" ) );
  QCOMPARE( elemToString( exportLine.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></LineString>" ) );
  QCOMPARE( elemToString( exportLineFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsLineString().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( "{\"coordinates\":[[31.0,32.0],[41.0,42.0],[51.0,52.0]],\"type\":\"LineString\"}" );
  QCOMPARE( exportLine.asJson(), expectedJson );
  QString expectedJsonPrec3( "{\"coordinates\":[[0.333,0.667],[1.333,1.667],[2.333,2.667]],\"type\":\"LineString\"}" );
  QCOMPARE( exportLineFloat.asJson( 3 ), expectedJsonPrec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>31,32,0 41,42,0 51,52,0</coordinates></LineString>" ) );
  QCOMPARE( exportLine.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>0.333,0.667,0 1.333,1.667,0 2.333,2.667,0</coordinates></LineString>" ) );
  QCOMPARE( exportLineFloat.asKml( 3 ), expectedKmlPrec3 );

  //length
  QgsLineString l19;
  QCOMPARE( l19.length(), 0.0 );
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QCOMPARE( l19.length(), 23.0 );

  //startPoint
  QCOMPARE( l19.startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );

  //endPoint
  QCOMPARE( l19.endPoint(), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  //test 3d length
  // without vertices
  l19.clear();
  QCOMPARE( l19.length3D(), 0.0 );

  // without Z
  l19.clear();
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                 << QgsPoint( QgsWkbTypes::Point, 3, 4 )
                 << QgsPoint( QgsWkbTypes::Point, 8, 16 ) );
  QCOMPARE( l19.length3D(), 18.0 );

  // with z
  l19.clear();
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                 << QgsPoint( QgsWkbTypes::PointZ, 4, 6, 2 ) );
  QCOMPARE( l19.length3D(), 8.0 );

  // with z and m
  l19.clear();
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 0, 0 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 13 )
                 << QgsPoint( QgsWkbTypes::PointZM, 4, 6, 2, 7 ) );
  QCOMPARE( l19.length3D(), 8.0 );

  //bad start/end points. Test that this doesn't crash.
  l19.clear();
  QVERIFY( l19.startPoint().isEmpty() );
  QVERIFY( l19.endPoint().isEmpty() );

  //curveToLine - no segmentation required, so should return a clone
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  segmentized.reset( l19.curveToLine() );
  QCOMPARE( segmentized->numPoints(), 3 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l19.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l19.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l19.pointN( 2 ) );

  // points
  QgsLineString l20;
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
  QgsLineString l21;
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
  QgsLineString l22;
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
  QgsLineString l23;
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

  //insert vertex in empty line
  QgsLineString l24;
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 1 );
  QVERIFY( !l24.is3D() );
  QVERIFY( !l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );

  //insert 4d vertex in empty line, should set line to 4d
  l24.clear();
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 6.0, 7.0, 1.0, 2.0 ) ) );
  QCOMPARE( l24.numPoints(), 1 );
  QVERIFY( l24.is3D() );
  QVERIFY( l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 6.0, 7.0, 1.0, 2.0 ) );

  //2d line
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QVERIFY( !l24.is3D() );
  QVERIFY( !l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 19.0 ) ) );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( 8.0, 9.0 ) );
  QCOMPARE( l24.pointN( 2 ), QgsPoint( 18.0, 19.0 ) );
  QCOMPARE( l24.pointN( 3 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( l24.pointN( 4 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( l24.pointN( 5 ), QgsPoint( 21.0, 22.0 ) );
  //insert vertex at end
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 6 ), QgsPoint( 31.0, 32.0 ) ) );
  QCOMPARE( l24.pointN( 6 ), QgsPoint( 31.0, 32.0 ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert vertex past end
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 8 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert vertex before start
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert 4d vertex in 4d line
  l24.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102, 103, 104 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 101, 102 ) );

  //insert first vertex as Point25D
  l24.clear();
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::Point25D, 101, 102, 103 ) ) );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 101, 102, 103 ) );

  //move vertex

  //empty line
  QgsLineString l25;
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
  QgsLineString l26;
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //valid line
  l26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  //out of range vertices
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( l26.numPoints(), 2 );
  QCOMPARE( l26.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( l26.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  //removing the second to last vertex removes both remaining vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( l26.numPoints(), 0 );
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //reversed
  QgsLineString l27;
  std::unique_ptr< QgsLineString > reversed( l27.reversed() );
  QVERIFY( reversed->isEmpty() );
  l27.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  reversed.reset( l27.reversed() );
  QCOMPARE( reversed->numPoints(), 3 );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  QCOMPARE( reversed->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( reversed->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( reversed->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );

  //addZValue

  QgsLineString l28;
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineStringZ );
  l28.clear();
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineStringZ );
  //2d line
  l28.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l28.addZValue( 2 ) );
  QVERIFY( l28.is3D() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineStringZ );
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
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );
  //linestring25d
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( !l28.addZValue( 5 ) );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );

  //addMValue

  QgsLineString l29;
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringM );
  l29.clear();
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringM );
  //2d line
  l29.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l29.addMValue( 2 ) );
  QVERIFY( !l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringM );
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
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  //linestring25d, should become LineStringZM
  l29.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( l29.addMValue( 5 ) );
  QVERIFY( l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );


  //dropZValue
  QgsLineString l28d;
  QVERIFY( !l28d.dropZValue() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !l28d.dropZValue() );
  l28d.addZValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringZ );
  QVERIFY( l28d.is3D() );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropZValue() ); //already dropped
  //linestring with m
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QVERIFY( l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  //linestring25d
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( l28d.dropZValue() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );

  //dropMValue
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !l28d.dropMValue() );
  l28d.addMValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringM );
  QVERIFY( l28d.isMeasure() );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropMValue() ); //already dropped
  //linestring with z
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QVERIFY( l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3, 0 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3, 0 ) );

  //convertTo
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineString ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineStringZ ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );
  l28d.setZAt( 0, 5.0 );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineString25D ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 5.0 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineStringZM ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );
  l28d.setMAt( 0, 6.0 );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineStringM ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineString ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( 1, 2 ) );
  QVERIFY( !l28d.convertTo( QgsWkbTypes::Polygon ) );

  //isRing
  QgsLineString l30;
  QVERIFY( !l30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  QVERIFY( !l30.isRing() ); //<4 points
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  QVERIFY( !l30.isRing() ); //not closed
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  QVERIFY( l30.isRing() );

  //coordinateSequence
  QgsLineString l31;
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

  QgsLineString l32;
  QgsVertexId v;
  QgsPoint p;
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !l32.nextVertex( v, p ) );

  // vertex iterator on empty linestring
  QgsAbstractGeometry::vertex_iterator it1 = l32.vertices_begin();
  QCOMPARE( it1, l32.vertices_end() );

  // Java-style iterator on empty linetring
  QgsVertexIterator it1x( &l32 );
  QVERIFY( !it1x.hasNext() );

  //LineString
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

  // vertex iterator
  QgsAbstractGeometry::vertex_iterator it2 = l32.vertices_begin();
  QCOMPARE( *it2, QgsPoint( 1, 2 ) );
  QCOMPARE( it2.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it2;
  QCOMPARE( *it2, QgsPoint( 11, 12 ) );
  QCOMPARE( it2.vertexId(), QgsVertexId( 0, 0, 1 ) );
  ++it2;
  QCOMPARE( it2, l32.vertices_end() );

  // Java-style iterator
  QgsVertexIterator it2x( &l32 );
  QVERIFY( it2x.hasNext() );
  QCOMPARE( it2x.next(), QgsPoint( 1, 2 ) );
  QVERIFY( it2x.hasNext() );
  QCOMPARE( it2x.next(), QgsPoint( 11, 12 ) );
  QVERIFY( !it2x.hasNext() );

  //LineStringZ
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !l32.nextVertex( v, p ) );

  //LineStringM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineStringZM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineString25D
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QVERIFY( !l32.nextVertex( v, p ) );

  //vertexAt and pointAt
  QgsLineString l33;
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QgsVertexId::VertexType type;
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  //LineString
  l33.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) );
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringZ
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringZM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineString25D
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //centroid
  QgsLineString l34;
  QVERIFY( l34.centroid().isEmpty() );
  l34.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 5, 10 ) );
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 20, 10 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 10, 5 ) );
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 ) << QgsPoint( 2, 9 ) << QgsPoint( 2, 0 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 1, 4.95 ) );
  //linestring with 0 length segment
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 ) << QgsPoint( 2, 9 ) << QgsPoint( 2, 9 ) << QgsPoint( 2, 0 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 1, 4.95 ) );
  //linestring with 0 total length segment
  l34.setPoints( QgsPointSequence() << QgsPoint( 5, 4 ) << QgsPoint( 5, 4 ) << QgsPoint( 5, 4 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 5, 4 ) );

  //closest segment
  QgsLineString l35;
  int leftOf = 0;
  p = QgsPoint( 0, 0 ); // reset all coords to zero
  ( void )l35.closestSegment( QgsPoint( 1, 2 ), p, v ); //empty line, just want no crash
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QVERIFY( l35.closestSegment( QgsPoint( 5, 10 ), p, v ) < 0 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 8, 9 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 11, 9 ), p, v, &leftOf ), 2.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 10, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                 << QgsPoint( 10, 10 )
                 << QgsPoint( 10, 15 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 11, 12 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 10, 12 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                 << QgsPoint( 6, 4 )
                 << QgsPoint( 4, 4 )
                 << QgsPoint( 5, 5 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 2.35, 4 ), p, v, &leftOf ), 2.7225, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                 << QgsPoint( 4, 4 )
                 << QgsPoint( 6, 4 )
                 << QgsPoint( 5, 5 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 2.35, 4 ), p, v, &leftOf ), 2.7225, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                 << QgsPoint( 6, 4 )
                 << QgsPoint( 4, 4 )
                 << QgsPoint( 5, 5 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 3.5, 2 ), p, v, &leftOf ), 4.250000, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                 << QgsPoint( 4, 4 )
                 << QgsPoint( 6, 4 )
                 << QgsPoint( 5, 5 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 3.5, 2 ), p, v, &leftOf ), 4.250000, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                 << QgsPoint( 1, 4 )
                 << QgsPoint( 2, 2 )
                 << QgsPoint( 1, 1 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 1, 0 ), p, v, &leftOf ), 1, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 1, 1 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                 << QgsPoint( 2, 2 )
                 << QgsPoint( 1, 4 )
                 << QgsPoint( 1, 1 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 1, 0 ), p, v, &leftOf ), 1, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 1, 1 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  //sumUpArea
  QgsLineString l36;
  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, -24, 4 * std::numeric_limits<double>::epsilon() );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, -22, 4 * std::numeric_limits<double>::epsilon() );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, -18, 4 * std::numeric_limits<double>::epsilon() );

  //boundingBox - test that bounding box is updated after every modification to the line string
  QgsLineString l37;
  QVERIFY( l37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.setPoints( QgsPointSequence() << QgsPoint( -5, -10 ) << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -10, -5, -9 ) );
  //setXAt
  l37.setXAt( 2, -4 );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -10, -4, -9 ) );
  //setYAt
  l37.setYAt( 1, -15 );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, -4, -9 ) );
  //append
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 4, 0 ) );
  l37.append( toAppend.get() );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, 4, 2 ) );
  l37.addVertex( QgsPoint( 6, 3 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, 6, 3 ) );
  l37.clear();
  QVERIFY( l37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QByteArray wkbToAppend = toAppend->asWkb();
  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  l37.fromWkb( wkbToAppendPtr );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 0, 4, 2 ) );
  l37.fromWkt( QStringLiteral( "LineString( 1 5, 3 4, 6 3 )" ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 3, 6, 5 ) );
  l37.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -1, 3, 6, 7 ) );
  l37.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -3, 3, 6, 10 ) );
  l37.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 3, 6, 5 ) );

  //angle
  QgsLineString l38;
  ( void )l38.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ); //no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.0, 4 * std::numeric_limits<double>::epsilon() );
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.71239, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 4.71239, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 3.1416, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.1416, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.7854, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0.0, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 20 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 4.71239, 0.00001 );
  //closed line string
  l38.close();
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );

  //removing the second to last vertex should remove the whole line
  QgsLineString l39;
  l39.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) );
  QVERIFY( l39.numPoints() == 2 );
  l39.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QVERIFY( l39.numPoints() == 0 );

  //boundary
  QgsLineString boundary1;
  QVERIFY( !boundary1.boundary() );
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QgsAbstractGeometry *boundary = boundary1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;

  // closed string = no boundary
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QVERIFY( !boundary1.boundary() );
  \

  //boundary with z
  boundary1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundary = boundary1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->geometryN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->z(), 10.0 );
  QCOMPARE( mpBoundary->geometryN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );
  delete boundary;

  //extend
  QgsLineString extend1;
  extend1.extend( 10, 10 ); //test no crash
  extend1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  extend1.extend( 1, 2 );
  QCOMPARE( extend1.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, -1, 0 ) );
  QCOMPARE( extend1.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 1, 0 ) );
  QCOMPARE( extend1.pointN( 2 ), QgsPoint( QgsWkbTypes::Point, 1, 3 ) );

  // addToPainterPath (note most tests are in test_qgsgeometry.py)
  QgsLineString path;
  QPainterPath pPath;
  path.addToPainterPath( pPath );
  QVERIFY( pPath.isEmpty() );
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  path.addToPainterPath( pPath );
  QVERIFY( !pPath.isEmpty() );

  // toCurveType
  QgsLineString curveLine1;
  curveLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  std::unique_ptr< QgsCompoundCurve > curveType( curveLine1.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curveType->numPoints(), 2 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );

  //adjacent vertices
  QgsLineString vertexLine1;
  vertexLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 3 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 2 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId() );
  // ring, part should be maintained
  vertexLine1.adjacentVertices( QgsVertexId( 1, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 1, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 1, 0, 2 ) );
  vertexLine1.adjacentVertices( QgsVertexId( 1, 2, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 1, 2, 0 ) );
  QCOMPARE( next, QgsVertexId( 1, 2, 2 ) );
  // closed ring
  vertexLine1.close();
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 3 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId() );

  // vertex number
  QgsLineString vertexLine2;
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  vertexLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );

  //segmentLength
  QgsLineString vertexLine3;
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 1, 1, 1 ) ), 100.0 );

  //removeDuplicateNodes
  QgsLineString nodeLine;
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "LineString (11 2, 11 12, 111 12)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  QVERIFY( nodeLine.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "LineString (11 2, 11 12, 111 12)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "LineString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99)" ) );
  // don't create degenerate lines
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "LineString (11 2)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "LineString (11 2, 11.01 1.99)" ) );
  // with z
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 ) << QgsPoint( 11.02, 2.01, 3 )
                      << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) << QgsPoint( 111.01, 11.99, 6 ) );
  QVERIFY( nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "LineStringZ (11 2 1, 11 12 4, 111 12 5)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 ) << QgsPoint( 11.02, 2.01, 3 )
                      << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) << QgsPoint( 111.01, 11.99, 6 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02, true ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "LineStringZ (11 2 1, 11.01 1.99 2, 11.02 2.01 3, 11 12 4, 111 12 5, 111.01 11.99 6)" ) );

  // swap xy
  QgsLineString swapLine;
  swapLine.swapXy(); // no crash
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  swapLine.swapXy();
  QCOMPARE( swapLine.asWkt( 2 ), QStringLiteral( "LineStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24)" ) );

  // filter vertex
  QgsLineString filterLine;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() < 5;
  };
  filterLine.filterVertices( filter ); // no crash
  filterLine.setPoints( QgsPointSequence()  << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  filterLine.filterVertices( filter );
  QCOMPARE( filterLine.asWkt( 2 ), QStringLiteral( "LineStringZM (1 2 3 4, 4 12 13 14)" ) );

  // transform vertex
  QgsLineString transformLine;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 5, point.y() + 6, point.z() + 7, point.m() + 8 );
  };
  transformLine.transformVertices( transform ); // no crash
  transformLine.setPoints( QgsPointSequence()  << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  transformLine.transformVertices( transform );
  QCOMPARE( transformLine.asWkt( 2 ), QStringLiteral( "LineStringZM (16 8 10 12, 6 8 10 12, 9 18 20 22, 116 18 30 32)" ) );

  // transform using class
  QgsLineString transformLine2;
  TestTransformer transformer;
  // no crash
  QVERIFY( transformLine2.transform( &transformer ) );
  transformLine2.setPoints( QgsPointSequence()  << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  QVERIFY( transformLine2.transform( &transformer ) );
  QCOMPARE( transformLine2.asWkt( 2 ), QStringLiteral( "LineStringZM (33 16 8 3, 3 16 8 3, 12 26 18 13, 333 26 28 23)" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !transformLine2.transform( &failTransformer ) );

  // substring
  QgsLineString substring;
  std::unique_ptr< QgsLineString > substringResult( substring.curveSubstring( 1, 2 ) ); // no crash
  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );
  substring.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  substringResult.reset( substring.curveSubstring( 0, 0 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 2 3 4)" ) );
  substringResult.reset( substring.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 3 4 5)" ) );
  substringResult.reset( substring.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 3 4 5)" ) );
  substringResult.reset( substring.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24)" ) );
  substringResult.reset( substring.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 12 13 14, 111 12 23 24)" ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 12 13 14, 21 12 14 15)" ) );
  substringResult.reset( substring.curveSubstring( QgsGeometryUtils::distanceToVertex( substring, QgsVertexId( 0, 0, 1 ) ), QgsGeometryUtils::distanceToVertex( substring, QgsVertexId( 0, 0, 2 ) ) ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 12 13 14, 111 12 23 24)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 0, QgsWkbTypes::PointZ ) << QgsPoint( 11, 12, 13, 0, QgsWkbTypes::PointZ ) << QgsPoint( 111, 12, 23, 0, QgsWkbTypes::PointZ ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZ (11 3 4, 11 12 13, 21 12 14)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 0, 3, QgsWkbTypes::PointM ) << QgsPoint( 11, 12, 0, 13, QgsWkbTypes::PointM ) << QgsPoint( 111, 12, 0, 23, QgsWkbTypes::PointM ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringM (11 3 4, 11 12 13, 21 12 14)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineString (11 3, 11 12, 21 12)" ) );

  //interpolate point
  QgsLineString interpolate;
  std::unique_ptr< QgsPoint > interpolateResult( interpolate.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolateResult.get() );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  interpolateResult.reset( interpolate.interpolatePoint( 0 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11 2 3 4)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( -1 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 100000 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11 3 4 5)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 20 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (21 12 14 15)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 110 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (111 12 23 24)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 0, QgsWkbTypes::PointZ ) << QgsPoint( 11, 12, 13, 0, QgsWkbTypes::PointZ ) << QgsPoint( 111, 12, 23, 0, QgsWkbTypes::PointZ ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZ (11 3 4)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 0, 3, QgsWkbTypes::PointM ) << QgsPoint( 11, 12, 0, 13, QgsWkbTypes::PointM ) << QgsPoint( 111, 12, 0, 23, QgsWkbTypes::PointM ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointM (11 3 4)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "Point (11 3)" ) );

  // visit points
  QgsLineString visitLine;
  visitLine.visitPointsByRegularDistance( 1, [ = ]( double, double, double, double, double, double, double, double, double, double, double, double )->bool
  {
    return true;
  } ); // no crash
  visitLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  int visitCount = 0;
  xx.clear();
  yy.clear();
  zz.clear();
  mm.clear();
  QVector<double> pX, pY, pZ, pM, nX, nY, nZ, nM;
  auto visitor = [ & ]( double x, double y, double z, double m, double ppx, double ppy, double ppz, double ppm, double nnx, double nny, double nnz, double nnm )->bool
  {
    xx << x;
    yy << y;
    zz << z;
    mm << m;
    pX << ppx;
    pY << ppy;
    pZ << ppz;
    pM << ppm;
    nX << nnx;
    nY << nny;
    nZ << nnz;
    nM << nnm;
    visitCount++;
    return true;
  };
  visitLine.visitPointsByRegularDistance( 0, visitor );
  QCOMPARE( visitCount, 1 );
  QCOMPARE( xx.at( 0 ), 11.0 );
  QCOMPARE( yy.at( 0 ), 2.0 );
  QCOMPARE( zz.at( 0 ), 3.0 );
  QCOMPARE( mm.at( 0 ), 4.0 );
  xx.clear();
  yy.clear();
  zz.clear();
  mm.clear();
  pX.clear();
  pY.clear();
  pZ.clear();
  pM.clear();
  nX.clear();
  nY.clear();
  nZ.clear();
  nM.clear();
  visitCount = 0;
  visitLine.visitPointsByRegularDistance( -1, visitor );
  QCOMPARE( visitCount, 0 );
  visitLine.visitPointsByRegularDistance( 10000, visitor );
  QCOMPARE( visitCount, 0 );
  visitLine.visitPointsByRegularDistance( 30, visitor );
  QCOMPARE( visitCount, 3 );
  QCOMPARE( xx.at( 0 ), 31.0 );
  QCOMPARE( yy.at( 0 ), 12.0 );
  QCOMPARE( zz.at( 0 ), 15.0 );
  QCOMPARE( mm.at( 0 ), 16.0 );
  QCOMPARE( pX.at( 0 ), 11.0 );
  QCOMPARE( pY.at( 0 ), 12.0 );
  QCOMPARE( pZ.at( 0 ), 13.0 );
  QCOMPARE( pM.at( 0 ), 14.0 );
  QCOMPARE( nX.at( 0 ), 111.0 );
  QCOMPARE( nY.at( 0 ), 12.0 );
  QCOMPARE( nZ.at( 0 ), 23.0 );
  QCOMPARE( nM.at( 0 ), 24.0 );
  QCOMPARE( xx.at( 1 ), 61.0 );
  QCOMPARE( yy.at( 1 ), 12.0 );
  QCOMPARE( zz.at( 1 ), 18.0 );
  QCOMPARE( mm.at( 1 ), 19.0 );
  QCOMPARE( pX.at( 1 ), 11.0 );
  QCOMPARE( pY.at( 1 ), 12.0 );
  QCOMPARE( pZ.at( 1 ), 13.0 );
  QCOMPARE( pM.at( 1 ), 14.0 );
  QCOMPARE( nX.at( 1 ), 111.0 );
  QCOMPARE( nY.at( 1 ), 12.0 );
  QCOMPARE( nZ.at( 1 ), 23.0 );
  QCOMPARE( nM.at( 1 ), 24.0 );
  QCOMPARE( xx.at( 2 ), 91.0 );
  QCOMPARE( yy.at( 2 ), 12.0 );
  QCOMPARE( zz.at( 2 ), 21.0 );
  QCOMPARE( mm.at( 2 ), 22.0 );
  QCOMPARE( pX.at( 2 ), 11.0 );
  QCOMPARE( pY.at( 2 ), 12.0 );
  QCOMPARE( pZ.at( 2 ), 13.0 );
  QCOMPARE( pM.at( 2 ), 14.0 );
  QCOMPARE( nX.at( 2 ), 111.0 );
  QCOMPARE( nY.at( 2 ), 12.0 );
  QCOMPARE( nZ.at( 2 ), 23.0 );
  QCOMPARE( nM.at( 2 ), 24.0 );

  // orientation
  QgsLineString orientation;
  ( void )orientation.orientation(); // no crash
  orientation.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) << QgsPoint( 1, 1 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( orientation.orientation(), QgsCurve::Clockwise );
  orientation.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( orientation.orientation(), QgsCurve::CounterClockwise );

  // test bounding box intersects
  QgsLineString bb;
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  bb.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 13, -10 ) );
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 12, 3, 16, 9 ) ) );
  // double test because of cache
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 12, 3, 16, 9 ) ) );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QCOMPARE( bb.boundingBox(), QgsRectangle( 11, -10, 13, 12 ) );
  // clear cache
  bb.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 13, -10 ) );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !bb.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QCOMPARE( bb.boundingBox(), QgsRectangle( 11, -10, 13, 12 ) );
  QVERIFY( bb.boundingBoxIntersects( QgsRectangle( 12, 3, 16, 9 ) ) );
}

QGSTEST_MAIN( TestQgsLineString )
#include "testqgslinestring.moc"
