/***************************************************************************
     testqgscompoundcurve.cpp
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
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"
#include "qgsproject.h"

#include "testgeometryutils.h"
#include "testtransformer.h"

class TestQgsCompoundCurve: public QObject
{
    Q_OBJECT
  private slots:
    void compoundCurve();
    void compoundCurveCondense_data();
    void compoundCurveCondense();
};


void TestQgsCompoundCurve::compoundCurve()
{
  //test constructors
  QgsCompoundCurve l1;
  QVERIFY( l1.isEmpty() );
  QCOMPARE( l1.numPoints(), 0 );
  QCOMPARE( l1.vertexCount(), 0 );
  QCOMPARE( l1.nCoordinates(), 0 );
  QCOMPARE( l1.ringCount(), 0 );
  QCOMPARE( l1.partCount(), 0 );
  QVERIFY( !l1.is3D() );
  QVERIFY( !l1.isMeasure() );
  QCOMPARE( l1.wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( l1.wktTypeStr(), QString( "CompoundCurve" ) );
  QCOMPARE( l1.geometryType(), QString( "CompoundCurve" ) );
  QCOMPARE( l1.dimension(), 1 );
  QVERIFY( !l1.hasCurvedSegments() );
  QCOMPARE( l1.area(), 0.0 );
  QCOMPARE( l1.perimeter(), 0.0 );
  QgsPointSequence pts;
  l1.points( pts );
  QVERIFY( pts.empty() );

  // empty, test some methods to make sure they don't crash
  QCOMPARE( l1.nCurves(), 0 );
  QVERIFY( !l1.curveAt( -1 ) );
  QVERIFY( !l1.curveAt( 0 ) );
  QVERIFY( !l1.curveAt( 100 ) );
  l1.removeCurve( -1 );
  l1.removeCurve( 0 );
  l1.removeCurve( 100 );

  //addCurve
  QgsCompoundCurve c1;
  //try to add null curve
  c1.addCurve( nullptr );
  QCOMPARE( c1.nCurves(), 0 );
  QVERIFY( !c1.curveAt( 0 ) );

  QgsCircularString l2;
  l2.setPoints( QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );
  c1.addCurve( l2.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numPoints(), 1 );
  QCOMPARE( c1.vertexCount(), 1 );
  QCOMPARE( c1.nCoordinates(), 1 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  c1.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );

  //adding first curve should set linestring z/m type
  QgsCircularString l3;
  l3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QgsCompoundCurve c2;
  c2.addCurve( l3.clone() );
  QVERIFY( !c2.isEmpty() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QCOMPARE( c2.wktTypeStr(), QString( "CompoundCurveZ" ) );
  c2.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  QgsCircularString l4;
  l4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QgsCompoundCurve c4;
  c4.addCurve( l4.clone() );
  QVERIFY( !c4.isEmpty() );
  QVERIFY( !c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::CompoundCurveM );
  QCOMPARE( c4.wktTypeStr(), QString( "CompoundCurveM" ) );
  c4.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );

  QgsCircularString l5;
  l5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QgsCompoundCurve c5;
  c5.addCurve( l5.clone() );
  QVERIFY( !c5.isEmpty() );
  QVERIFY( c5.is3D() );
  QVERIFY( c5.isMeasure() );
  QCOMPARE( c5.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QCOMPARE( c5.wktTypeStr(), QString( "CompoundCurveZM" ) );
  c5.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  //clear
  c5.clear();
  QVERIFY( c5.isEmpty() );
  QCOMPARE( c5.nCurves(), 0 );
  QCOMPARE( c5.numPoints(), 0 );
  QCOMPARE( c5.vertexCount(), 0 );
  QCOMPARE( c5.nCoordinates(), 0 );
  QCOMPARE( c5.ringCount(), 0 );
  QCOMPARE( c5.partCount(), 0 );
  QVERIFY( !c5.is3D() );
  QVERIFY( !c5.isMeasure() );
  QCOMPARE( c5.wkbType(), QgsWkbTypes::CompoundCurve );

  //addCurve
  QgsCircularString l8;
  QgsCompoundCurve c8;
  l8.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.isEmpty() );
  QCOMPARE( c8.numPoints(), 3 );
  QCOMPARE( c8.vertexCount(), 3 );
  QCOMPARE( c8.nCoordinates(), 3 );
  QCOMPARE( c8.ringCount(), 1 );
  QCOMPARE( c8.partCount(), 1 );
  QCOMPARE( c8.nCurves(), 1 );
  QVERIFY( !c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( c8.hasCurvedSegments() );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QVERIFY( ! c8.curveAt( -1 ) );
  QVERIFY( ! c8.curveAt( 1 ) );

  QgsCircularString l8a;
  l8a.setPoints( QgsPointSequence() << QgsPoint( 3, 4 ) << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 ) );
  c8.addCurve( l8a.clone() );
  QCOMPARE( c8.numPoints(), 5 );
  QCOMPARE( c8.vertexCount(), 5 );
  QCOMPARE( c8.nCoordinates(), 5 );
  QCOMPARE( c8.ringCount(), 1 );
  QCOMPARE( c8.partCount(), 1 );
  QCOMPARE( c8.nCurves(), 2 );
  pts.clear();
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 )
            << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 1 ) ), l8a );
  QVERIFY( ! c8.curveAt( -1 ) );
  QVERIFY( ! c8.curveAt( 2 ) );

  QgsLineString l8b;
  l8b.setPoints( QgsPointSequence() << QgsPoint( 3, 6 ) << QgsPoint( 4, 6 ) );
  c8.addCurve( l8b.clone() );
  QCOMPARE( c8.numPoints(), 6 );
  QCOMPARE( c8.vertexCount(), 6 );
  QCOMPARE( c8.nCoordinates(), 6 );
  QCOMPARE( c8.ringCount(), 1 );
  QCOMPARE( c8.partCount(), 1 );
  QCOMPARE( c8.nCurves(), 3 );
  pts.clear();
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 )
            << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 )
            << QgsPoint( 4, 6 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 1 ) ), l8a );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( c8.curveAt( 2 ) ), l8b );
  QVERIFY( ! c8.curveAt( -1 ) );
  QVERIFY( ! c8.curveAt( 3 ) );

  //removeCurve
  c8.removeCurve( -1 );
  c8.removeCurve( 3 );
  QCOMPARE( c8.nCurves(), 3 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 1 ) ), l8a );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( c8.curveAt( 2 ) ), l8b );
  c8.removeCurve( 1 );
  QCOMPARE( c8.nCurves(), 2 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( c8.curveAt( 1 ) ), l8b );
  c8.removeCurve( 0 );
  QCOMPARE( c8.nCurves(), 1 );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( c8.curveAt( 0 ) ), l8b );
  c8.removeCurve( 0 );
  QCOMPARE( c8.nCurves(), 0 );
  QVERIFY( c8.isEmpty() );

  //addCurve with z
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.numPoints(), 2 );
  QVERIFY( c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  pts.clear();
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //addCurve with m
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.numPoints(), 2 );
  QVERIFY( !c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //addCurve with zm
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.numPoints(), 2 );
  QVERIFY( c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );

  //addCurve with z to non z compound curve
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 ) << QgsPoint( QgsWkbTypes::Point, 2, 3 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 3, 5 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 ) << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );
  c8.removeCurve( 1 );

  //addCurve with m to non m compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 3, 3, 0, 5 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 ) << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );
  c8.removeCurve( 1 );

  //addCurve with zm to non m compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 3, 1, 5 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 ) << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );
  c8.removeCurve( 1 );

  //addCurve with no z to z compound curve
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2, 3 ) << QgsPoint( QgsWkbTypes::Point, 3, 4 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 0 ) );
  c8.removeCurve( 1 );

  //add curve with m, no z to z compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 8 ) << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 9 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 0 ) );
  c8.removeCurve( 1 );

  //add curve with zm to z compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 4, 7, 9 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 7 ) );
  c8.removeCurve( 1 );

  //addCurve with no m to m compound curve
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2, 3 ) << QgsPoint( QgsWkbTypes::Point, 3, 4 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 0 ) );
  c8.removeCurve( 1 );

  //add curve with z, no m to m compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 8 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 9 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 0 ) );
  c8.removeCurve( 1 );

  //add curve with zm to m compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 4, 7, 9 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 9 ) );
  c8.removeCurve( 1 );

  // add curve and extend existing
  QgsCompoundCurve c8j;
  // try to extend empty compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  c8j.addCurve( l8.clone(), true );
  QCOMPARE( c8j.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4))" ) );
  // try to add another curve with extend existing as true - should be ignored.
  l8.setPoints( QgsPointSequence() << QgsPoint( 6, 6 ) << QgsPoint( 7, 8 ) );
  c8j.addCurve( l8.clone(), true );
  QCOMPARE( c8j.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8))" ) );
  // try to add a linestring with extend existing as true - should be ignored because the last curve isn't a linestring
  QgsLineString l8j;
  l8j.setPoints( QgsPointSequence() << QgsPoint( 10, 8 ) << QgsPoint( 10, 12 ) );
  c8j.addCurve( l8j.clone(), true );
  QCOMPARE( c8j.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8),(10 8, 10 12))" ) );
  // try to extend with another linestring -- should add to final part
  l8j.setPoints( QgsPointSequence() << QgsPoint( 11, 13 ) << QgsPoint( 12, 12 ) );
  c8j.addCurve( l8j.clone(), true );
  QCOMPARE( c8j.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8),(10 8, 10 12, 11 13, 12 12))" ) );
  // try to extend with another linestring -- should add to final part, with no duplicate points
  l8j.setPoints( QgsPointSequence() << QgsPoint( 12, 12 ) << QgsPoint( 13, 12 ) << QgsPoint( 14, 15 ) );
  c8j.addCurve( l8j.clone(), true );
  QCOMPARE( c8j.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8),(10 8, 10 12, 11 13, 12 12, 13 12, 14 15))" ) );
  // not extending, should be added as new curve
  l8j.setPoints( QgsPointSequence() << QgsPoint( 15, 16 ) << QgsPoint( 17, 12 ) );
  c8j.addCurve( l8j.clone(), false );
  QCOMPARE( c8j.asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 2, 2 3, 3 4),CircularString (6 6, 7 8),(10 8, 10 12, 11 13, 12 12, 13 12, 14 15),(15 16, 17 12))" ) );
  c8j.clear();
  // adding a linestring as first part, with extend as true
  c8j.addCurve( l8j.clone(), true );
  QCOMPARE( c8j.asWkt(), QStringLiteral( "CompoundCurve ((15 16, 17 12))" ) );

  //test getters/setters
  QgsCompoundCurve c9;

  // no crash!
  ( void )c9.xAt( -1 );
  ( void )c9.xAt( 1 );
  ( void )c9.yAt( -1 );
  ( void )c9.yAt( 1 );

  QgsCircularString l9;
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  c9.addCurve( l9.clone() );
  QCOMPARE( c9.xAt( 0 ), 1.0 );
  QCOMPARE( c9.xAt( 1 ), 11.0 );
  QCOMPARE( c9.xAt( 2 ), 21.0 );
  ( void ) c9.xAt( -1 ); //out of range
  ( void ) c9.xAt( 11 ); //out of range
  QCOMPARE( c9.yAt( 0 ), 2.0 );
  QCOMPARE( c9.yAt( 1 ), 12.0 );
  QCOMPARE( c9.yAt( 2 ), 22.0 );
  ( void ) c9.yAt( -1 ); //out of range
  ( void ) c9.yAt( 11 ); //out of range

  QgsLineString l9a;
  l9a.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 31, 22, 13, 14 ) );
  c9.addCurve( l9a.clone() );
  QCOMPARE( c9.xAt( 0 ), 1.0 );
  QCOMPARE( c9.xAt( 1 ), 11.0 );
  QCOMPARE( c9.xAt( 2 ), 21.0 );
  QCOMPARE( c9.xAt( 3 ), 31.0 );
  QCOMPARE( c9.xAt( 4 ), 0.0 );
  ( void ) c9.xAt( -1 ); //out of range
  ( void ) c9.xAt( 11 ); //out of range
  QCOMPARE( c9.yAt( 0 ), 2.0 );
  QCOMPARE( c9.yAt( 1 ), 12.0 );
  QCOMPARE( c9.yAt( 2 ), 22.0 );
  QCOMPARE( c9.yAt( 3 ), 22.0 );
  QCOMPARE( c9.yAt( 4 ), 0.0 );
  ( void ) c9.yAt( -1 ); //out of range
  ( void ) c9.yAt( 11 ); //out of range

  c9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 52.0 ) );
  QCOMPARE( c9.xAt( 0 ), 51.0 );
  QCOMPARE( c9.yAt( 0 ), 52.0 );
  c9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 62 ) );
  QCOMPARE( c9.xAt( 1 ), 61.0 );
  QCOMPARE( c9.yAt( 1 ), 62.0 );
  c9.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 71.0, 2 ) ); //out of range
  c9.moveVertex( QgsVertexId( 0, 0, 11 ), QgsPoint( 71.0, 2 ) ); //out of range

  QgsPoint p;
  QgsVertexId::VertexType type;
  QVERIFY( !c9.pointAt( -1, p, type ) );
  QVERIFY( !c9.pointAt( 11, p, type ) );
  QVERIFY( c9.pointAt( 0, p, type ) );
  QCOMPARE( p.z(), 3.0 );
  QCOMPARE( p.m(), 4.0 );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c9.pointAt( 1, p, type ) );
  QCOMPARE( p.z(), 13.0 );
  QCOMPARE( p.m(), 14.0 );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c9.pointAt( 2, p, type ) );
  QCOMPARE( p.z(), 23.0 );
  QCOMPARE( p.m(), 24.0 );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c9.pointAt( 3, p, type ) );
  QCOMPARE( p.z(), 13.0 );
  QCOMPARE( p.m(), 14.0 );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //equality
  QgsCompoundCurve e1;
  QgsCompoundCurve e2;
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  QgsLineString le1;
  QgsLineString le2;
  le1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  e1.addCurve( le1.clone() );
  QVERIFY( !( e1 == e2 ) ); //different number of curves
  QVERIFY( e1 != e2 );
  e2.addCurve( le1.clone() );
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  le1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  e1.addCurve( le1.clone() );
  QVERIFY( !( e1 == e2 ) ); //different number of curves
  QVERIFY( e1 != e2 );
  le2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  e2.addCurve( le2.clone() );
  QVERIFY( e1 == e2 ); //check non-integer equality
  QVERIFY( !( e1 != e2 ) );
  le1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) << QgsPoint( 7, 8 ) );
  e1.addCurve( le1.clone() );
  le2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) << QgsPoint( 6, 9 ) );
  e2.addCurve( le2.clone() );
  QVERIFY( !( e1 == e2 ) ); //different coordinates
  QVERIFY( e1 != e2 );

  // different dimensions
  QgsCompoundCurve e3;
  e1.clear();
  e1.addCurve( le1.clone() );
  QgsLineString le3;
  le3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );
  e3.addCurve( le3.clone() );
  QVERIFY( !( e1 == e3 ) ); //different dimension
  QVERIFY( e1 != e3 );
  QgsCompoundCurve e4;
  QgsLineString le4;
  le4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                 << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );
  e4.addCurve( le4.clone() );
  QVERIFY( !( e3 == e4 ) ); //different z coordinates
  QVERIFY( e3 != e4 );
  QgsCompoundCurve e5;
  QgsLineString le5;
  le5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                 << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                 << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  e5.addCurve( le5.clone() );
  QgsCompoundCurve e6;
  QgsLineString le6;
  le6.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                 << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                 << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );
  e6.addCurve( le6.clone() );
  QVERIFY( !( e5 == e6 ) ); //different m values
  QVERIFY( e5 != e6 );

  QVERIFY( e6 != QgsLineString() );

  // assignment operator
  e5.addCurve( le5.clone() );
  QVERIFY( e5 != e6 );
  e6 = e5;
  QCOMPARE( e5, e6 );

  //isClosed
  QgsCompoundCurve c11;
  QgsCircularString l11;
  QVERIFY( !c11.isClosed() );
  l11.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  c11.addCurve( l11.clone() );
  QVERIFY( !c11.isClosed() );
  QgsLineString ls11;
  ls11.setPoints( QgsPointSequence() << QgsPoint( 1, 22 )
                  << QgsPoint( 1, 2 ) );
  c11.addCurve( ls11.clone() );
  QVERIFY( c11.isClosed() );

  //test that m values aren't considered when testing for closedness
  c11.clear();
  l11.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                 << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );
  c11.addCurve( l11.clone() );
  QVERIFY( c11.isClosed() );

  //polygonf
  QgsCircularString lc13;
  QgsCompoundCurve c13;
  lc13.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  c13.addCurve( lc13.clone() );
  QgsLineString ls13;
  ls13.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 23, 22 ) );
  c13.addCurve( ls13.clone() );
  QPolygonF poly = c13.asQPolygonF();
  QCOMPARE( poly.count(), 183 );
  QCOMPARE( poly.at( 0 ).x(), 1.0 );
  QCOMPARE( poly.at( 0 ).y(), 2.0 );
  QCOMPARE( poly.at( 182 ).x(), 23.0 );
  QCOMPARE( poly.at( 182 ).y(), 22.0 );

  // clone tests
  QgsCircularString lc14;
  lc14.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                  << QgsPoint( 11, 2 )
                  << QgsPoint( 11, 22 )
                  << QgsPoint( 1, 22 ) );
  QgsCompoundCurve c14;
  c14.addCurve( lc14.clone() );
  QgsLineString ls14;
  ls14.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 23, 22 ) );
  c14.addCurve( ls14.clone() );
  std::unique_ptr<QgsCompoundCurve> cloned( c14.clone() );
  QCOMPARE( *cloned, c14 );

  //clone with Z/M
  lc14.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  ls14.setPoints( QgsPointSequence() << QgsPoint( 1, 22, 31, 34 ) << QgsPoint( 23, 22, 42, 43 ) );
  c14.clear();
  c14.addCurve( lc14.clone() );
  c14.addCurve( ls14.clone() );
  cloned.reset( c14.clone() );
  QCOMPARE( *cloned, c14 );

  //clone an empty line
  c14.clear();
  cloned.reset( c14.clone() );
  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CompoundCurve );

  //segmentize tests
  QgsCompoundCurve toSegment;
  QgsCircularString lcSegment;
  lcSegment.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  toSegment.addCurve( lcSegment.clone() );
  std::unique_ptr<QgsLineString> segmentized( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );

  QCOMPARE( segmentized->pointN( 0 ), lcSegment.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), lcSegment.pointN( toSegment.numPoints() - 1 ) );

  //segmentize with Z/M
  lcSegment.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                       << QgsPoint( QgsWkbTypes::PointZM, 11, 10, 11, 14 )
                       << QgsPoint( QgsWkbTypes::PointZM, 21, 2, 21, 24 ) );
  toSegment.clear();
  toSegment.addCurve( lcSegment.clone() );
  segmentized.reset( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), lcSegment.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), lcSegment.pointN( toSegment.numPoints() - 1 ) );

  //segmentize an empty line
  toSegment.clear();
  segmentized.reset( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );

  //to/from WKB
  QgsCompoundCurve c15;
  QgsCircularString l15;
  l15.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  c15.addCurve( l15.clone() );
  QByteArray wkb15 = c15.asWkb();
  QCOMPARE( wkb15.size(), c15.wkbSize() );
  QgsCompoundCurve c16;
  QgsConstWkbPtr wkb15ptr( wkb15 );
  c16.fromWkb( wkb15ptr );
  QCOMPARE( c16.numPoints(), 4 );
  QCOMPARE( c16.vertexCount(), 4 );
  QCOMPARE( c16.nCoordinates(), 4 );
  QCOMPARE( c16.ringCount(), 1 );
  QCOMPARE( c16.partCount(), 1 );
  QCOMPARE( c16.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( c16.is3D() );
  QVERIFY( c16.isMeasure() );
  QCOMPARE( c16.nCurves(), 1 );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( c16.curveAt( 0 ) )->pointN( 0 ), l15.pointN( 0 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( c16.curveAt( 0 ) )->pointN( 1 ), l15.pointN( 1 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( c16.curveAt( 0 ) )->pointN( 2 ), l15.pointN( 2 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( c16.curveAt( 0 ) )->pointN( 3 ), l15.pointN( 3 ) );

  //bad WKB - check for no crash
  c16.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c16.fromWkb( nullPtr ) );
  QCOMPARE( c16.wkbType(), QgsWkbTypes::CompoundCurve );
  QgsPoint point( 1, 2 );
  QByteArray wkb16 = point.asWkb();
  QgsConstWkbPtr wkb16ptr( wkb16 );
  QVERIFY( !c16.fromWkb( wkb16ptr ) );
  QCOMPARE( c16.wkbType(), QgsWkbTypes::CompoundCurve );

  //to/from WKT
  QgsCompoundCurve c17;
  QgsCircularString l17;
  l17.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  c17.addCurve( l17.clone() );

  QString wkt = c17.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsCompoundCurve c18;
  QVERIFY( c18.fromWkt( wkt ) );
  QCOMPARE( c18.numPoints(), 4 );
  QCOMPARE( c18.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( c18.is3D() );
  QVERIFY( c18.isMeasure() );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( c18.curveAt( 0 ) )->pointN( 0 ), l17.pointN( 0 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( c18.curveAt( 0 ) )->pointN( 1 ), l17.pointN( 1 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( c18.curveAt( 0 ) )->pointN( 2 ), l17.pointN( 2 ) );
  QCOMPARE( qgis::down_cast< const QgsCircularString *>( c18.curveAt( 0 ) )->pointN( 3 ), l17.pointN( 3 ) );

  //bad WKT
  QVERIFY( !c18.fromWkt( "Polygon()" ) );
  QVERIFY( c18.isEmpty() );
  QCOMPARE( c18.numPoints(), 0 );
  QVERIFY( !c18.is3D() );
  QVERIFY( !c18.isMeasure() );
  QCOMPARE( c18.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( !c18.fromWkt( "CompoundCurve(LineString(0 0, 1 1),Point( 2 2 ))" ) );

  //asGML2
  QgsCompoundCurve exportCurve;
  QgsCircularString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 )
                        << QgsPoint( 51, 52 ) );
  exportCurve.addCurve( exportLine.clone() );
  QgsLineString exportLineString;
  exportLineString.setPoints( QgsPointSequence() << QgsPoint( 51, 52 )
                              << QgsPoint( 61, 62 ) );
  exportCurve.addCurve( exportLineString.clone() );

  QgsCircularString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );
  QgsCompoundCurve exportCurveFloat;
  exportCurveFloat.addCurve( exportLineFloat.clone() );
  QgsLineString exportLineStringFloat;
  exportLineStringFloat.setPoints( QgsPointSequence() << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 )
                                   << QgsPoint( 3 + 1 / 3.0, 3 + 2 / 3.0 ) );
  exportCurveFloat.addCurve( exportLineStringFloat.clone() );

  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52 61,62</coordinates></LineString>" ) );
  QString result = elemToString( exportCurve.asGml2( doc ) );
  QGSCOMPAREGML( result, expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667 3.333,3.667</coordinates></LineString>" ) );
  result = elemToString( exportCurveFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( result, expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCompoundCurve().asGml2( doc ) ), expectedGML2empty );


  //asGML3
  QString expectedGML3( QStringLiteral( "<CompositeCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">51 52 61 62</posList></LineString></curveMember></CompositeCurve>" ) );
  result = elemToString( exportCurve.asGml3( doc ) );
  QCOMPARE( result, expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<CompositeCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 2.667 3.333 3.667</posList></LineString></curveMember></CompositeCurve>" ) );
  result = elemToString( exportCurveFloat.asGml3( doc, 3 ) );
  QCOMPARE( result, expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<CompositeCurve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCompoundCurve().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( "{\"coordinates\":[[31.0,32.0],[41.0,42.0],[51.0,52.0],[61.0,62.0]],\"type\":\"LineString\"}" );
  result = exportCurve.asJson();
  QCOMPARE( result, expectedJson );
  QString expectedJsonPrec3( "{\"coordinates\":[[0.333,0.667],[1.333,1.667],[2.333,2.667],[3.333,3.667]],\"type\":\"LineString\"}" );
  result = exportCurveFloat.asJson( 3 );
  QCOMPARE( result, expectedJsonPrec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>31,32,0 41,42,0 51,52,0 61,62,0</coordinates></LineString>" ) );
  QCOMPARE( exportCurve.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>0.333,0.667,0 1.333,1.667,0 2.333,2.667,0 3.333,3.667,0</coordinates></LineString>" ) );
  QCOMPARE( exportCurveFloat.asKml( 3 ), expectedKmlPrec3 );

  //length
  QgsCompoundCurve c19;
  QCOMPARE( c19.length(), 0.0 );
  QgsCircularString l19;
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c19.addCurve( l19.clone() );
  QgsLineString l19a;
  l19a.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  c19.addCurve( l19a.clone() );
  QGSCOMPARENEAR( c19.length(), 36.1433, 0.001 );

  //startPoint
  QCOMPARE( c19.startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );

  //endPoint
  QCOMPARE( c19.endPoint(), QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );

  //bad start/end points. Test that this doesn't crash.
  c19.clear();
  QVERIFY( c19.startPoint().isEmpty() );
  QVERIFY( c19.endPoint().isEmpty() );

  //curveToLine
  c19.clear();
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c19.addCurve( l19.clone() );
  l19a.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  c19.addCurve( l19a.clone() );
  segmentized.reset( c19.curveToLine() );
  QCOMPARE( segmentized->numPoints(), 182 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l19.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), l19a.pointN( l19a.numPoints() - 1 ) );

  // points
  QgsCompoundCurve c20;
  QgsCircularString l20;
  QgsPointSequence points;
  c20.points( points );
  QVERIFY( points.isEmpty() );
  l20.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c20.addCurve( l20.clone() );
  QgsLineString ls20;
  ls20.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  c20.addCurve( ls20.clone() );
  c20.points( points );
  QCOMPARE( points.count(), 4 );
  QCOMPARE( points.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( points.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 ) );
  QCOMPARE( points.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QCOMPARE( points.at( 3 ), QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );

  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsCompoundCurve c21;
  QgsCircularString l21;
  l21.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                 << QgsPoint( 6474985, -3526584 ) );
  c21.addCurve( l21.clone() );
  QgsLineString ls21;
  ls21.setPoints( QgsPointSequence() << QgsPoint( 6474985, -3526584 )
                  << QgsPoint( 6504985, -3526584 ) );
  c21.addCurve( ls21.clone() );
  c21.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QGSCOMPARENEAR( c21.xAt( 0 ), 175.771, 0.001 );
  QGSCOMPARENEAR( c21.yAt( 0 ), -39.724, 0.001 );
  QGSCOMPARENEAR( c21.xAt( 1 ), 176.959, 0.001 );
  QGSCOMPARENEAR( c21.yAt( 1 ), -38.7999, 0.001 );
  QGSCOMPARENEAR( c21.xAt( 2 ), 177.315211, 0.001 );
  QGSCOMPARENEAR( c21.yAt( 2 ), -38.799974, 0.001 );
  QGSCOMPARENEAR( c21.boundingBox().xMinimum(), 175.770033, 0.001 );
  QGSCOMPARENEAR( c21.boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( c21.boundingBox().xMaximum(), 177.315211, 0.001 );
  QGSCOMPARENEAR( c21.boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  QgsCompoundCurve c22;
  l21.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  c22.addCurve( l21.clone() );
  ls21.setPoints( QgsPointSequence() << QgsPoint( 6474985, -3526584, 3, 4 )
                  << QgsPoint( 6504985, -3526584, 5, 6 ) );
  c22.addCurve( ls21.clone() );
  c22.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QgsPoint pt;
  QgsVertexId::VertexType v;
  c22.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.x(), 175.771, 0.001 );
  QGSCOMPARENEAR( pt.y(), -39.724, 0.001 );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
  QCOMPARE( pt.m(), 2.0 );
  c22.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 176.959, 0.001 );
  QGSCOMPARENEAR( pt.y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( pt.z(), 3.0, 0.001 );
  QCOMPARE( pt.m(), 4.0 );
  c22.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 177.315211, 0.001 );
  QGSCOMPARENEAR( pt.y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( pt.z(), 5.0, 0.001 );
  QCOMPARE( pt.m(), 6.0 );

  //reverse transform
  c22.transform( tr, QgsCoordinateTransform::ReverseTransform );
  c22.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.x(), 6374985, 100 );
  QGSCOMPARENEAR( pt.y(), -3626584, 100 );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
  QCOMPARE( pt.m(), 2.0 );
  c22.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 6474985, 100 );
  QGSCOMPARENEAR( pt.y(), -3526584, 100 );
  QGSCOMPARENEAR( pt.z(), 3.0, 0.001 );
  QCOMPARE( pt.m(), 4.0 );
  c22.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 6504985, 100 );
  QGSCOMPARENEAR( pt.y(), -3526584, 100 );
  QGSCOMPARENEAR( pt.z(), 5.0, 0.001 );
  QCOMPARE( pt.m(), 6.0 );

#if PROJ_VERSION_MAJOR<6 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //z value transform
  c22.transform( tr, QgsCoordinateTransform::ForwardTransform, true );
  c22.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.z(), -19.249066, 0.001 );
  c22.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.z(), -21.092128, 0.001 );
  c22.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.z(), -19.370485, 0.001 );

  c22.transform( tr, QgsCoordinateTransform::ReverseTransform, true );
  c22.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.z(), 1, 0.001 );
  c22.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.z(), 3, 0.001 );
  c22.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.z(), 5, 0.001 );
#endif

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsCompoundCurve c23;
  QgsCircularString l23;
  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  c23.addCurve( l23.clone() );
  QgsLineString ls23;
  ls23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) << QgsPoint( QgsWkbTypes::PointZM, 21, 13, 13, 14 ) );
  c23.addCurve( ls23.clone() );
  c23.transform( qtr, 5, 2, 4, 3 );
  c23.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 2, 6, 11, 16 ) );
  c23.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 22, 36, 31, 46 ) );
  c23.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 42, 39, 31, 46 ) );
  QCOMPARE( c23.boundingBox(), QgsRectangle( 2, 6, 42, 39 ) );

  //insert vertex
  //cannot insert vertex in empty line
  QgsCompoundCurve c24;
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( c24.numPoints(), 0 );

  //2d line
  QgsCircularString l24;
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  c24.addCurve( l24.clone() );
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 4.0, 7.0 ) ) );
  QCOMPARE( c24.numPoints(), 5 );
  QVERIFY( !c24.is3D() );
  QVERIFY( !c24.isMeasure() );
  QCOMPARE( c24.wkbType(), QgsWkbTypes::CompoundCurve );
  c24.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 2.0 ) );
  c24.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 4.0, 7.0 ) );
  c24.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( pt.y(), 9.930870, 0.01 );
  c24.pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( 11.0, 12.0 ) );
  c24.pointAt( 4, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 22.0 ) );

  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 16.0 ) ) );
  QCOMPARE( c24.numPoints(), 9 );
  c24.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 2.0 ) );
  c24.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 4.363083, 0.01 );
  QGSCOMPARENEAR( pt.y(), 5.636917, 0.01 );
  c24.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 8.0, 9.0 ) );
  c24.pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( 18.0, 16.0 ) );
  c24.pointAt( 4, pt, v );
  QGSCOMPARENEAR( pt.x(), 5.876894, 0.01 );
  QGSCOMPARENEAR( pt.y(), 8.246211, 0.01 );
  c24.pointAt( 5, pt, v );
  QCOMPARE( pt, QgsPoint( 4.0, 7.0 ) );
  c24.pointAt( 6, pt, v );
  QGSCOMPARENEAR( pt.x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( pt.y(), 9.930870, 0.01 );
  c24.pointAt( 7, pt, v );
  QCOMPARE( pt, QgsPoint( 11.0, 12.0 ) );
  c24.pointAt( 8, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 22.0 ) );

  //insert vertex at end
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 31.0, 32.0 ) ) );

  //insert vertex past end
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( c24.numPoints(), 9 );

  //insert vertex before start
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( c24.numPoints(), 9 );

  //insert 4d vertex in 4d line
  c24.clear();
  l24.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c24.addCurve( l24.clone( ) );
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( c24.numPoints(), 5 );
  c24.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );
  QCOMPARE( c24.numPoints(), 7 );
  QCOMPARE( c24.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c24.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  c24.clear();
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  c24.addCurve( l24.clone() );
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 4, 103, 104 ) ) );
  QCOMPARE( c24.numPoints(), 5 );
  QCOMPARE( c24.wkbType(), QgsWkbTypes::CompoundCurve );
  c24.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 2, 4 ) );

  // invalid
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 1, 2 ) ) );

  //move vertex

  //empty line
  QgsCompoundCurve c25;
  QgsCircularString l25;
  QVERIFY( !c25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( c25.isEmpty() );

  //valid line
  l25.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  c25.addCurve( l25.clone() );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  c25.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 6.0, 7.0 ) );
  c25.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 16.0, 17.0 ) );
  c25.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 26.0, 27.0 ) );

  //out of range
  QVERIFY( !c25.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !c25.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !c25.moveVertex( QgsVertexId( 0, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  c25.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 6.0, 7.0 ) );
  c25.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 16.0, 17.0 ) );
  c25.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 26.0, 27.0 ) );

  //move 4d point in 4d line
  l25.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c25.clear();
  c25.addCurve( l25.clone() );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) ) );
  c25.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) );

  //move 2d point in 4d line, existing z/m should be maintained
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 34, 35 ) ) );
  c25.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 34, 35, 12, 13 ) );

  //move 4d point in 2d line
  l25.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  c25.clear();
  c25.addCurve( l25.clone() );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 3, 4, 2, 3 ) ) );
  c25.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 3, 4 ) );

  //delete vertex

  //empty line
  QgsCompoundCurve c26;
  QgsCircularString l26;
  QVERIFY( !c26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( c26.isEmpty() );

  //valid line
  l26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                 << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );
  c26.addCurve( l26.clone() );
  //out of range vertices
  QVERIFY( !c26.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !c26.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( c26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( c26.numPoints(), 2 );
  c26.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  c26.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );

  //removing the next vertex removes all remaining vertices
  QVERIFY( c26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( c26.numPoints(), 0 );
  QVERIFY( c26.isEmpty() );

  // two lines
  QgsLineString ls26;
  c26.clear();
  ls26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  c26.addCurve( ls26.clone() );
  ls26.setPoints( QgsPointSequence()
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );
  c26.addCurve( ls26.clone() );
  QVERIFY( c26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( c26.nCurves(), 1 );
  const QgsLineString *ls26r = dynamic_cast< const QgsLineString * >( c26.curveAt( 0 ) );
  QCOMPARE( ls26r->numPoints(), 2 );
  QCOMPARE( ls26r->startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( ls26r->endPoint(), QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );

  //add vertex at the end of linestring
  QVERIFY( c26.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( QgsWkbTypes::PointZM, 35, 43, 4, 5 ) ) );
  ls26r = dynamic_cast< const QgsLineString * >( c26.curveAt( 0 ) );
  QCOMPARE( ls26r->numPoints(), 3 );
  QCOMPARE( ls26r->startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( ls26r->endPoint(), QgsPoint( QgsWkbTypes::PointZM, 35, 43, 4, 5 ) );

  c26.clear();
  ls26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 ) );
  c26.addCurve( ls26.clone() );
  ls26.setPoints( QgsPointSequence()
                  << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );
  c26.addCurve( ls26.clone() );
  QVERIFY( c26.deleteVertex( QgsVertexId( 0, 0, 2 ) ) );
  QCOMPARE( c26.nCurves(), 1 );
  ls26r = dynamic_cast< const QgsLineString * >( c26.curveAt( 0 ) );
  QCOMPARE( ls26r->numPoints(), 2 );
  QCOMPARE( ls26r->startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( ls26r->endPoint(), QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );

  //reversed
  QgsCompoundCurve c27;
  QgsCircularString l27;
  std::unique_ptr< QgsCompoundCurve > reversed( c27.reversed() );
  QVERIFY( reversed->isEmpty() );
  l27.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  c27.addCurve( l27.clone() );
  QgsLineString ls27;
  ls27.setPoints( QgsPointSequence()
                  << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 23, 32, 7, 8 ) );
  c27.addCurve( ls27.clone() );

  reversed.reset( c27.reversed() );
  QCOMPARE( reversed->numPoints(), 4 );
  QVERIFY( dynamic_cast< const QgsLineString * >( reversed->curveAt( 0 ) ) );
  QVERIFY( dynamic_cast< const QgsCircularString * >( reversed->curveAt( 1 ) ) );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  reversed->pointAt( 0, pt, v );
  reversed->pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  reversed->pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  reversed->pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );

  //addZValue
  QgsCompoundCurve c28;
  QgsCircularString l28;
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( c28.addZValue() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c28.clear();
  //2d line
  l28.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28.addCurve( l28.clone() );
  l28.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 3, 4 ) );
  c28.addCurve( l28.clone() );
  QVERIFY( c28.addZValue( 2 ) );
  QVERIFY( c28.is3D() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 3, 4, 2 ) );

  QVERIFY( !c28.addZValue( 4 ) ); //already has z value, test that existing z is unchanged
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 3, 4, 2 ) );

  //linestring with m
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  c28.clear();
  c28.addCurve( l28.clone() );
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 21, 32, 0, 4 ) );
  c28.addCurve( l28.clone() );
  QVERIFY( c28.addZValue( 5 ) );
  QVERIFY( c28.is3D() );
  QVERIFY( c28.isMeasure() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 32, 5, 4 ) );

  //addMValue
  c28.clear();
  //2d line
  l28.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28.addCurve( l28.clone() );
  l28.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 3, 4 ) );
  c28.addCurve( l28.clone() );
  QVERIFY( c28.addMValue( 2 ) );
  QVERIFY( c28.isMeasure() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveM );
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 2 ) );

  QVERIFY( !c28.addMValue( 4 ) ); //already has z value, test that existing z is unchanged
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 2 ) );

  //linestring with z
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) );
  c28.clear();
  c28.addCurve( l28.clone() );
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 21, 32, 4 ) );
  c28.addCurve( l28.clone() );
  QVERIFY( c28.addMValue( 5 ) );
  QVERIFY( c28.is3D() );
  QVERIFY( c28.isMeasure() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 ) );

  //dropZValue
  QgsCompoundCurve c28d;
  QgsCircularString l28d;
  QVERIFY( !c28d.dropZValue() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28d.addCurve( l28d.clone() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  c28d.addCurve( l28d.clone() );
  QVERIFY( !c28d.dropZValue() );
  c28d.addZValue( 1.0 );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QVERIFY( c28d.is3D() );
  QVERIFY( c28d.dropZValue() );
  QVERIFY( !c28d.is3D() );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurve );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  c28d.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  c28d.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 21, 22 ) );

  QVERIFY( !c28d.dropZValue() ); //already dropped
  //linestring with m
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  c28d.clear();
  c28d.addCurve( l28d.clone() );
  l28d.setPoints( QgsPointSequence() <<  QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 3, 4 ) );
  c28d.addCurve( l28d.clone() );
  QVERIFY( c28d.dropZValue() );
  QVERIFY( !c28d.is3D() );
  QVERIFY( c28d.isMeasure() );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveM );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  c28d.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  c28d.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 4 ) );

  //dropMValue
  c28d.clear();
  QVERIFY( !c28d.dropMValue() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28d.addCurve( l28d.clone() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  c28d.addCurve( l28d.clone() );
  QVERIFY( !c28d.dropMValue() );
  c28d.addMValue( 1.0 );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveM );
  QVERIFY( c28d.isMeasure() );
  QVERIFY( c28d.dropMValue() );
  QVERIFY( !c28d.isMeasure() );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurve );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  c28d.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  c28d.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 21, 22 ) );

  QVERIFY( !c28d.dropMValue() ); //already dropped
  //linestring with z
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  c28d.clear();
  c28d.addCurve( l28d.clone() );
  l28d.setPoints( QgsPointSequence() <<  QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 3, 4 ) );
  c28d.addCurve( l28d.clone() );
  QVERIFY( c28d.dropMValue() );
  QVERIFY( !c28d.isMeasure() );
  QVERIFY( c28d.is3D() );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  c28d.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3 ) );
  c28d.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 21, 22, 3 ) );

  //convertTo
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28d.clear();
  c28d.addCurve( l28d.clone() );
  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurve ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurveZ ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );

  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurveZM ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2 ) );
  c28d.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 5 ) );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );
  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurveM ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveM );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2 ) );
  c28d.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 0, 6 ) );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurve ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurve );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( !c28d.convertTo( QgsWkbTypes::Polygon ) );

  //isRing
  QgsCircularString l30;
  QgsCompoundCurve c30;
  QVERIFY( !c30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  c30.addCurve( l30.clone() );
  QVERIFY( !c30.isRing() ); //<4 points
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  c30.clear();
  c30.addCurve( l30.clone() );
  QVERIFY( !c30.isRing() ); //not closed
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  c30.clear();
  c30.addCurve( l30.clone() );
  QVERIFY( c30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c30.clear();
  c30.addCurve( l30.clone() );
  QVERIFY( !c30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 11, 12 )  << QgsPoint( 21, 22 ) );
  c30.addCurve( l30.clone() );
  QVERIFY( !c30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 21, 22 )  << QgsPoint( 1, 2 ) );
  c30.addCurve( l30.clone() );
  QVERIFY( c30.isRing() );

  //coordinateSequence
  QgsCompoundCurve c31;
  QgsCircularString l31;
  QgsCoordinateSequence coords = c31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QVERIFY( coords.at( 0 ).at( 0 ).isEmpty() );
  l31.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  c31.addCurve( l31.clone() );
  QgsLineString ls31;
  ls31.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) <<
                  QgsPoint( QgsWkbTypes::PointZM, 31, 32, 16, 17 ) );
  c31.addCurve( ls31.clone() );
  coords = c31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QCOMPARE( coords.at( 0 ).at( 0 ).count(), 4 );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 3 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 16, 17 ) );

  //nextVertex
  QgsCompoundCurve c32;
  QgsCircularString l32;
  QgsVertexId vId;
  QVERIFY( !c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 0, -2 );
  QVERIFY( !c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 0, 10 );
  QVERIFY( !c32.nextVertex( vId, p ) );
  //CircularString
  l32.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c32.addCurve( l32.clone() );
  vId = QgsVertexId( 0, 0, 2 ); //out of range
  QVERIFY( !c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 0, -5 );
  QVERIFY( c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 1, 0 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 1, 1 ) ); //test that ring number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  vId = QgsVertexId( 1, 0, 0 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QgsLineString ls32;
  ls32.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 13, 14 ) );
  c32.addCurve( ls32.clone() );
  vId = QgsVertexId( 0, 0, 1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( p, QgsPoint( 13, 14 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );

  //CircularStringZ
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  c32.clear();
  c32.addCurve( l32.clone() );
  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );
  //CircularStringM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  c32.clear();
  c32.addCurve( l32.clone() );
  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );
  //CircularStringZM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  c32.clear();
  c32.addCurve( l32.clone() );
  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );

  //vertexAt and pointAt
  QgsCompoundCurve c33;
  QgsCircularString l33;
  c33.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  c33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QVERIFY( !c33.pointAt( -10, p, type ) );
  QVERIFY( !c33.pointAt( 10, p, type ) );
  //CircularString
  l33.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  c33.addCurve( l33.clone() );
  c33.vertexAt( QgsVertexId( 0, 0, -10 ) );
  c33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QVERIFY( !c33.pointAt( -10, p, type ) );
  QVERIFY( !c33.pointAt( 10, p, type ) );
  QVERIFY( c33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QgsLineString ls33;
  ls33.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 3, 34 ) );
  c33.addCurve( ls33.clone() );
  QVERIFY( c33.pointAt( 3, p, type ) );
  QCOMPARE( p, QgsPoint( 3, 34 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  c33.clear();
  //CircularStringZ
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )  << QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  c33.addCurve( l33.clone() );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  QVERIFY( c33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22, 23 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //CircularStringM
  c33.clear();
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) << QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  c33.addCurve( l33.clone() );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QVERIFY( c33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //CircularStringZM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  c33.clear();
  c33.addCurve( l33.clone() );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QVERIFY( c33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //centroid
  QgsCircularString l34;
  QgsCompoundCurve c34;
  QCOMPARE( c34.centroid(), QgsPoint() );
  l34.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  c34.addCurve( l34.clone() );
  QCOMPARE( c34.centroid(), QgsPoint( 5, 10 ) );
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 20, 10 ) << QgsPoint( 2, 9 ) );
  c34.clear();
  c34.addCurve( l34.clone() );
  QgsPoint centroid = c34.centroid();
  QGSCOMPARENEAR( centroid.x(), 7.333, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 6.333, 0.001 );
  l34.setPoints( QgsPointSequence() << QgsPoint( 2, 9 ) << QgsPoint( 12, 9 ) << QgsPoint( 15, 19 ) );
  c34.addCurve( l34.clone() );
  centroid = c34.centroid();
  QGSCOMPARENEAR( centroid.x(), 9.756646, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 8.229039, 0.001 );

  //closest segment
  QgsCompoundCurve c35;
  QgsCircularString l35;
  int leftOf = 0;
  p = QgsPoint( 0, 0 ); // reset all coords to zero
  ( void )c35.closestSegment( QgsPoint( 1, 2 ), p, vId ); //empty line, just want no crash
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  c35.addCurve( l35.clone() );
  QVERIFY( c35.closestSegment( QgsPoint( 5, 10 ), p, vId ) < 0 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) );
  c35.clear();
  c35.addCurve( l35.clone() );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 4, 11 ), p, vId, &leftOf ), 2.0, 0.0001 );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 8, 11 ), p, vId, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5.5, 11.5 ), p, vId, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 7, 16 ), p, vId, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5.5, 13.5 ), p, vId, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( c35.closestSegment( QgsPoint( 5, 15 ), p, vId, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  QgsLineString ls35;
  ls35.setPoints( QgsPointSequence() << QgsPoint( 5, 15 ) << QgsPoint( 5, 20 ) );
  c35.addCurve( ls35.clone() );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5.5, 16.5 ), p, vId, &leftOf ), 0.25, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 16.5, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 4.5, 16.5 ), p, vId, &leftOf ), 0.25, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 16.5, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 4.5, 21.5 ), p, vId, &leftOf ), 2.500000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5.5, 21.5 ), p, vId, &leftOf ), 2.500000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5, 20 ), p, vId, &leftOf ), 0.0000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 0 );

  //sumUpArea
  QgsCompoundCurve c36;
  QgsCircularString l36;
  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  c36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  c36.addCurve( l36.clone() );
  c36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  c36.clear();
  c36.addCurve( l36.clone() );
  c36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  c36.clear();
  c36.addCurve( l36.clone() );
  c36.sumUpArea( area );
  QGSCOMPARENEAR( area, 4.141593, 0.0001 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  c36.clear();
  c36.addCurve( l36.clone() );
  c36.sumUpArea( area );
  QGSCOMPARENEAR( area, 7.283185, 0.0001 );
  // full circle
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 4, 0 ) << QgsPoint( 0, 0 ) );
  c36.clear();
  c36.addCurve( l36.clone() );
  area = 0.0;
  c36.sumUpArea( area );
  QGSCOMPARENEAR( area, 12.566370614359172, 0.0001 );

  //boundingBox - test that bounding box is updated after every modification to the circular string
  QgsCompoundCurve c37;
  QgsCircularString l37;
  QVERIFY( c37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  c37.addCurve( l37.clone() );
  QCOMPARE( c37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.setPoints( QgsPointSequence() << QgsPoint( -5, -10 ) << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  c37.clear();
  c37.addCurve( l37.clone() );
  QCOMPARE( c37.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );
  QByteArray wkbToAppend = c37.asWkb();
  c37.clear();
  QVERIFY( c37.boundingBox().isNull() );
  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  c37.clear();
  c37.addCurve( l37.clone() );
  QCOMPARE( c37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  c37.fromWkb( wkbToAppendPtr );
  QCOMPARE( c37.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );
  c37.fromWkt( QStringLiteral( "CompoundCurve(CircularString( 5 10, 6 10, 5.5 9 ))" ) );
  QCOMPARE( c37.boundingBox(), QgsRectangle( 5, 9, 6.125, 10.25 ) );
  c37.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QgsRectangle r = c37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -3.014, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 14.014, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), -7.0146, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 12.4988, 0.01 );
  c37.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  r = c37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -10.294, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 12.294, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 31.856, 0.01 );
  c37.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  r = c37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), 5, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 6.125, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 10.25, 0.01 );

  //angle
  QgsCompoundCurve c38;
  QgsCircularString l38;
  ( void )c38.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  c38.addCurve( l38.clone() );
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141593, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 20 ) ); // no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 )
                 << QgsPoint( -1, 3 ) << QgsPoint( 0, 4 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 1.5708, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 4 ) << QgsPoint( -1, 3 ) << QgsPoint( 0, 2 )
                 << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 4.712389, 0.0001 );

  // with second curve
  QgsLineString ls38;
  ls38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, -1 ) );
  c38.addCurve( ls38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 3.926991, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.141593, 0.0001 );

  //closed circular string
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0, 0.00001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.00001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0, 0.00001 );

  //removing a vertex from a 3 point comound curveshould remove the whole line
  QgsCircularString l39;
  QgsCompoundCurve c39;
  l39.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  c39.addCurve( l39.clone() );
  QCOMPARE( c39.numPoints(), 3 );
  c39.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( c39.numPoints(), 0 );

  //boundary
  QgsCompoundCurve cBoundary1;
  QgsCircularString boundary1;
  QVERIFY( !cBoundary1.boundary() );
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  cBoundary1.addCurve( boundary1.clone() );
  QgsAbstractGeometry *boundary = cBoundary1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;

  // closed string = no boundary
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  cBoundary1.clear();
  cBoundary1.addCurve( boundary1.clone() );
  QVERIFY( !cBoundary1.boundary() );

  //boundary with z
  boundary1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  cBoundary1.clear();
  cBoundary1.addCurve( boundary1.clone() );
  boundary = cBoundary1.boundary();
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

  // addToPainterPath (note most tests are in test_qgsgeometry.py)
  QgsCompoundCurve ccPath;
  QgsCircularString path;
  QPainterPath pPath;
  ccPath.addToPainterPath( pPath );
  QVERIFY( pPath.isEmpty() );
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )
                  << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 ) );
  ccPath.addCurve( path.clone() );
  ccPath.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 21.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 2.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );
  QgsLineString lsPath;
  lsPath.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 )
                    << QgsPoint( QgsWkbTypes::PointZ, 31, 12, 3 ) );
  ccPath.addCurve( lsPath.clone() );
  pPath = QPainterPath();
  ccPath.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 31.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );

  // even number of points - should still work
  pPath = QPainterPath();
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  ccPath.clear();
  ccPath.addCurve( path.clone() );
  ccPath.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 11.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );

  // toCurveType
  QgsCircularString curveLine1;
  QgsCompoundCurve cc1;
  curveLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  cc1.addCurve( curveLine1.clone() );
  std::unique_ptr< QgsCurve > curveType( cc1.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curveType->numPoints(), 3 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QgsLineString ccls1;
  ccls1.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 1, 25 ) );
  cc1.addCurve( ccls1.clone() );
  curveType.reset( cc1.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curveType->numPoints(), 4 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 1, 25 ) );

  //test that area of a compound curve ring is equal to a closed linestring with the same vertices
  QgsCompoundCurve cc;
  QgsLineString *ll1 = new QgsLineString();
  ll1->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  cc.addCurve( ll1 );
  QgsLineString *ll2 = new QgsLineString();
  ll2->setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( -1, 0 ) << QgsPoint( 0, -1 ) );
  cc.addCurve( ll2 );
  QgsLineString *ll3 = new QgsLineString();
  ll3->setPoints( QgsPointSequence() << QgsPoint( 0, -1 ) << QgsPoint( 1, 1 ) );
  cc.addCurve( ll3 );

  double ccArea = 0.0;
  cc.sumUpArea( ccArea );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) <<  QgsPoint( -1, 0 ) << QgsPoint( 0, -1 )
                << QgsPoint( 1, 1 ) );
  double lsArea = 0.0;
  ls.sumUpArea( lsArea );
  QGSCOMPARENEAR( ccArea, lsArea, 4 * std::numeric_limits<double>::epsilon() );


  //addVertex
  QgsCompoundCurve ac1;
  ac1.addVertex( QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !ac1.isEmpty() );
  QCOMPARE( ac1.numPoints(), 1 );
  QCOMPARE( ac1.vertexCount(), 1 );
  QCOMPARE( ac1.nCoordinates(), 1 );
  QCOMPARE( ac1.ringCount(), 1 );
  QCOMPARE( ac1.partCount(), 1 );
  QVERIFY( !ac1.is3D() );
  QVERIFY( !ac1.isMeasure() );
  QCOMPARE( ac1.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( !ac1.hasCurvedSegments() );
  QCOMPARE( ac1.area(), 0.0 );
  QCOMPARE( ac1.perimeter(), 0.0 );

  //adding first vertex should set linestring z/m type
  QgsCompoundCurve ac2;
  ac2.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QVERIFY( !ac2.isEmpty() );
  QVERIFY( ac2.is3D() );
  QVERIFY( !ac2.isMeasure() );
  QCOMPARE( ac2.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QCOMPARE( ac2.wktTypeStr(), QString( "CompoundCurveZ" ) );

  QgsCompoundCurve ac3;
  ac3.addVertex( QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QVERIFY( !ac3.isEmpty() );
  QVERIFY( !ac3.is3D() );
  QVERIFY( ac3.isMeasure() );
  QCOMPARE( ac3.wkbType(), QgsWkbTypes::CompoundCurveM );
  QCOMPARE( ac3.wktTypeStr(), QString( "CompoundCurveM" ) );

  QgsCompoundCurve ac4;
  ac4.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( !ac4.isEmpty() );
  QVERIFY( ac4.is3D() );
  QVERIFY( ac4.isMeasure() );
  QCOMPARE( ac4.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QCOMPARE( ac4.wktTypeStr(), QString( "CompoundCurveZM" ) );

  //adding subsequent vertices should not alter z/m type, regardless of points type
  QgsCompoundCurve ac5;
  ac5.addVertex( QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) ); //2d type
  QCOMPARE( ac5.wkbType(), QgsWkbTypes::CompoundCurve );
  ac5.addVertex( QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0, 13.0 ) ); // add 3d point
  QCOMPARE( ac5.numPoints(), 2 );
  QCOMPARE( ac5.vertexCount(), 2 );
  QCOMPARE( ac5.nCoordinates(), 2 );
  QCOMPARE( ac5.ringCount(), 1 );
  QCOMPARE( ac5.partCount(), 1 );
  QCOMPARE( ac5.wkbType(), QgsWkbTypes::CompoundCurve ); //should still be 2d
  QVERIFY( !ac5.is3D() );
  QCOMPARE( ac5.area(), 0.0 );
  QCOMPARE( ac5.perimeter(), 0.0 );

  QgsCompoundCurve ac6;
  ac6.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) ); //3d type
  QCOMPARE( ac6.wkbType(), QgsWkbTypes::CompoundCurveZ );
  ac6.addVertex( QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) ); //add 2d point
  QCOMPARE( ac6.wkbType(), QgsWkbTypes::CompoundCurveZ ); //should still be 3d
  ac6.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0 ) );
  QVERIFY( ac6.is3D() );
  QCOMPARE( ac6.numPoints(), 2 );
  QCOMPARE( ac6.vertexCount(), 2 );
  QCOMPARE( ac6.nCoordinates(), 2 );
  QCOMPARE( ac6.ringCount(), 1 );
  QCOMPARE( ac6.partCount(), 1 );

  //close
  QgsLineString closeC1;
  QgsCompoundCurve closeCc1;
  closeCc1.close();
  QVERIFY( closeCc1.isEmpty() );
  closeC1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  closeCc1.addCurve( closeC1.clone() );
  QCOMPARE( closeCc1.numPoints(), 3 );
  QVERIFY( !closeCc1.isClosed() );
  closeCc1.close();
  QCOMPARE( closeCc1.numPoints(), 4 );
  QVERIFY( closeCc1.isClosed() );
  closeCc1.pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  closeCc1.close();
  QCOMPARE( closeCc1.numPoints(), 4 );
  QVERIFY( closeCc1.isClosed() );

  //segmentLength
  QgsCircularString curveLine2;
  QgsCompoundCurve slc1;
  QCOMPARE( slc1.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  curveLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  slc1.addCurve( curveLine2.clone() );
  QCOMPARE( slc1.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( -1, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 1, 1, 0 ) ), 31.4159, 0.001 );
  curveLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( -9, 32 ) << QgsPoint( 1, 42 ) );
  slc1.addCurve( curveLine2.clone() );
  QCOMPARE( slc1.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QgsLineString curveLine3;
  curveLine3.setPoints( QgsPointSequence() << QgsPoint( 1, 42 ) << QgsPoint( 10, 42 ) );
  slc1.addCurve( curveLine3.clone() );
  QCOMPARE( slc1.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 4 ) ), 9.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 5 ) ), 0.0 );

  //removeDuplicateNodes
  QgsCompoundCurve nodeCurve;
  QgsCircularString nodeLine;
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  nodeCurve.addCurve( nodeLine.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  QCOMPARE( nodeCurve.asWkt(), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) );
  nodeCurve.clear();
  nodeCurve.addCurve( nodeLine.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  QCOMPARE( nodeCurve.asWkt(), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 11 2))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 10, 3 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 9, 3 )
                      << QgsPoint( 11, 2 ) );
  nodeCurve.clear();
  nodeCurve.addCurve( nodeLine.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 10 3, 11.01 1.99, 9 3, 11 2))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  nodeCurve.clear();
  nodeCurve.addCurve( nodeLine.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99))" ) );
  QVERIFY( nodeCurve.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99))" ) );

  // with tiny segment
  QgsLineString linePart;
  linePart.setPoints( QgsPointSequence() << QgsPoint( 111.01, 11.99 ) << QgsPoint( 111, 12 ) );
  nodeCurve.addCurve( linePart.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99),(111.01 11.99, 111 12))" ) );
  QVERIFY( nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99))" ) );

  // with multiple duplicate nodes
  nodeCurve.fromWkt( QStringLiteral( "CompoundCurve ((11 1, 11 2, 11 2),CircularString(11 2, 10 3, 10 2),(10 2, 10 2, 11 1))" ) );
  QVERIFY( nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 0 ), QStringLiteral( "CompoundCurve ((11 1, 11 2),CircularString (11 2, 10 3, 10 2),(10 2, 11 1))" ) );

  // ensure continuity
  nodeCurve.clear();
  linePart.setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 111.01, 11.99 ) << QgsPoint( 111, 12 ) );
  nodeCurve.addCurve( linePart.clone() );
  linePart.setPoints( QgsPointSequence() << QgsPoint( 111, 12 ) << QgsPoint( 31, 33 ) );
  nodeCurve.addCurve( linePart.clone() );
  QVERIFY( nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve ((1 1, 111.01 11.99),(111.01 11.99, 31 33))" ) );

  // swap xy
  QgsCompoundCurve swapCurve;
  swapCurve.swapXy(); //no crash
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  swapCurve.addCurve( nodeLine.clone() );
  swapCurve.swapXy();
  QCOMPARE( swapCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24))" ) );
  QgsLineString lsSwap;
  lsSwap.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM ) );
  swapCurve.addCurve( lsSwap.clone() );
  swapCurve.swapXy();
  QCOMPARE( swapCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24),(111 12 23 24, 122 22 33 34))" ) );

  // filter vertices
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() > 5;
  };
  QgsCompoundCurve filterCurve;
  filterCurve.filterVertices( filter ); //no crash
  nodeLine.setPoints( QgsPointSequence()   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM )   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) );
  filterCurve.addCurve( nodeLine.clone() );
  filterCurve.filterVertices( filter );
  QCOMPARE( filterCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24))" ) );
  QgsLineString lsFilter;
  lsFilter.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM ) << QgsPoint( 1, 111, 23, 24, QgsWkbTypes::PointZM ) );
  filterCurve.addCurve( lsFilter.clone() );
  filterCurve.filterVertices( filter );
  QCOMPARE( filterCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24),(12 111 23 24, 22 122 33 34))" ) );

  // transform vertices
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 5 );
  };
  QgsCompoundCurve transformCurve;
  transformCurve.transformVertices( transform ); //no crash
  nodeLine.setPoints( QgsPointSequence()   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM )   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) );
  transformCurve.addCurve( nodeLine.clone() );
  transformCurve.transformVertices( transform );
  QCOMPARE( transformCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (3 5 7 9, 13 5 7 9, 13 15 17 19, 113 15 27 29, 3 5 7 9))" ) );
  QgsLineString lsTransform;
  lsTransform.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM ) << QgsPoint( 1, 111, 23, 24, QgsWkbTypes::PointZM ) );
  transformCurve.addCurve( lsTransform.clone() );
  transformCurve.transformVertices( transform );
  QCOMPARE( transformCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (5 8 11 14, 15 8 11 14, 15 18 21 24, 115 18 31 34, 5 8 11 14),(14 114 27 29, 24 125 37 39, 3 114 27 29))" ) );

  // transform using class
  QgsCompoundCurve transformCurve2;
  TestTransformer transformer;
  // no crash
  QVERIFY( transformCurve2.transform( &transformer ) );
  nodeLine.setPoints( QgsPointSequence()   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM )   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) );
  transformCurve2.addCurve( nodeLine.clone() );
  QVERIFY( transformCurve2.transform( &transformer ) );
  QCOMPARE( transformCurve2.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (3 16 8 3, 33 16 8 3, 33 26 18 13, 333 26 28 23, 3 16 8 3))" ) );
  transformCurve2.addCurve( lsTransform.clone() );
  QVERIFY( transformCurve2.transform( &transformer ) );
  QCOMPARE( transformCurve2.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (9 30 13 2, 99 30 13 2, 99 40 23 12, 999 40 33 22, 9 30 13 2),(36 125 28 23, 66 136 38 33, 3 125 28 23))" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !transformCurve2.transform( &failTransformer ) );

  // substring
  QgsCompoundCurve substring;
  std::unique_ptr< QgsCompoundCurve > substringResult( substring.curveSubstring( 1, 2 ) ); // no crash
  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );
  substring.fromWkt( QStringLiteral( "CompoundCurveZM( ( 5 0 -1 -2, 10 0 1 2 ), CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  substringResult.reset( substring.curveSubstring( 0, 0 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 5 0 -1 -2))" ) );
  substringResult.reset( substring.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 6 0 -0.6 -1.2))" ) );
  substringResult.reset( substring.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 6 0 -0.6 -1.2))" ) );
  substringResult.reset( substring.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 10 0 1 2),CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  substringResult.reset( substring.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 10 0 1 2),CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  substringResult.reset( substring.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 10 0 1 2),CircularStringZM (10 0 1 2, 10.46 0.84 2.27 3.27, 11.42 0.91 5.73 6.73))" ) );
  substringResult.reset( substring.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 6.5 0 -0.4 -0.8))" ) );

  substring.fromWkt( QStringLiteral( "CompoundCurveZ( ( 5 0 -1, 10 0 1 ), CircularStringZ (10 0 1, 11 1 3, 12 0 13))" ) );
  substringResult.reset( substring.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZ ((6 0 -0.6, 10 0 1),CircularStringZ (10 0 1, 10.46 0.84 2.27, 11.42 0.91 5.73))" ) );
  substring.fromWkt( QStringLiteral( "CompoundCurveM( ( 5 0 -1, 10 0 1 ), CircularStringM (10 0 1, 11 1 3, 12 0 13))" ) );
  substringResult.reset( substring.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveM ((6 0 -0.6, 10 0 1),CircularStringM (10 0 1, 10.46 0.84 2.27, 11.42 0.91 5.73))" ) );
  substring.fromWkt( QStringLiteral( "CompoundCurve( ( 5 0, 10 0 ), CircularString (10 0, 11 1, 12 0))" ) );
  substringResult.reset( substring.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurve ((6 0, 10 0),CircularString (10 0, 10.46 0.84, 11.42 0.91))" ) );

  // substring
  QgsCompoundCurve interpolate;
  std::unique_ptr< QgsPoint > interpolateResult( interpolate.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolateResult.get() );
  interpolate.fromWkt( QStringLiteral( "CompoundCurveZM( ( 5 0 -1 -2, 10 0 1 2 ), CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 0 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (5 0 -1 -2)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( -1 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 100000 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (6 0 -0.6 -1.2)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 7 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11.42 0.91 5.73 6.73)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (6.5 0 -0.4 -0.8)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( interpolate.length() ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (12 0 13 14)" ) );

  interpolate.fromWkt( QStringLiteral( "CompoundCurveZ( ( 5 0 -1, 10 0 1 ), CircularStringZ (10 0 1, 11 1 3, 12 0 13))" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZ (6 0 -0.6)" ) );
  interpolate.fromWkt( QStringLiteral( "CompoundCurveM( ( 5 0 -1, 10 0 1 ), CircularStringM (10 0 1, 11 1 3, 12 0 13))" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointM (6 0 -0.6)" ) );
  interpolate.fromWkt( QStringLiteral( "CompoundCurve( ( 5 0, 10 0 ), CircularString (10 0, 11 1, 12 0))" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "Point (6 0)" ) );

  // orientation
  QgsCompoundCurve orientation;
  ( void )orientation.orientation(); // no crash
  orientation.fromWkt( QStringLiteral( "CompoundCurve( ( 0 0, 0 1), CircularString (0 1, 1 1, 1 0), (1 0, 0 0))" ) );
  QCOMPARE( orientation.orientation(), QgsCurve::Clockwise );
  orientation.fromWkt( QStringLiteral( "CompoundCurve( ( 0 0, 1 0), CircularString (1 0, 1 1, 0 1), (0 1, 0 0))" ) );
  QCOMPARE( orientation.orientation(), QgsCurve::CounterClockwise );
}

void TestQgsCompoundCurve::compoundCurveCondense_data()
{
  QTest::addColumn<QString>( "curve" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "compound curve empty" ) << QStringLiteral( "COMPOUNDCURVE()" ) << QStringLiteral( "CompoundCurve EMPTY" );
  QTest::newRow( "compound curve one line" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 1 2))" ) << QStringLiteral( "CompoundCurve ((1 1, 1 2))" );
  QTest::newRow( "compound curve one circular string" ) << QStringLiteral( "COMPOUNDCURVE(CIRCULARSTRING(1 1, 1 2, 2 2))" ) << QStringLiteral( "CompoundCurve (CircularString (1 1, 1 2, 2 2))" );
  QTest::newRow( "compound curve can't condense" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),CIRCULARSTRING(1 1, 1 2, 2 2),(2 2, 2 3),CIRCULARSTRING(2 3, 2 0, 2 1))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1),CircularString (1 1, 1 2, 2 2),(2 2, 2 3),CircularString (2 3, 2 0, 2 1))" );
  QTest::newRow( "compound curve two lines" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),(1 1, 1 0, 3 0))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1, 1 0, 3 0))" );
  QTest::newRow( "compound curve three lines" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),(1 1, 1 0, 3 0),(3 0, 4 0, 5 0))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1, 1 0, 3 0, 4 0, 5 0))" );
  QTest::newRow( "compound curve two lines and cs" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),(1 1, 1 0, 3 0),CIRCULARSTRING(3 0, 4 0, 5 0))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1, 1 0, 3 0),CircularString (3 0, 4 0, 5 0))" );
  QTest::newRow( "compound curve two cs" ) << QStringLiteral( "COMPOUNDCURVE(CIRCULARSTRING(1 5, 1 4, 1 1),CIRCULARSTRING(1 1, 1 0, 3 0))" ) << QStringLiteral( "CompoundCurve (CircularString (1 5, 1 4, 1 1, 1 0, 3 0))" );
  QTest::newRow( "compound curve complex" ) << QStringLiteral( "COMPOUNDCURVE((1 5, 1 4, 1 1),(1 1, 1 0, 3 0),CIRCULARSTRING(3 0, 4 0, 5 0),(5 0, 6 0),(6 0, 7 0),CIRCULARSTRING(7 0, 8 0, 9 0),CIRCULARSTRING(9 0, 10 1, 10 0),(10 0, 10 -1))" ) << QStringLiteral( "CompoundCurve ((1 5, 1 4, 1 1, 1 0, 3 0),CircularString (3 0, 4 0, 5 0),(5 0, 6 0, 7 0),CircularString (7 0, 8 0, 9 0, 10 1, 10 0),(10 0, 10 -1))" );
}

void TestQgsCompoundCurve::compoundCurveCondense()
{
  QFETCH( QString, curve );
  QFETCH( QString, expected );

  QgsGeometry g = QgsGeometry::fromWkt( curve );
  qgsgeometry_cast< QgsCompoundCurve * >( g.get() )->condenseCurves();

  QCOMPARE( g.asWkt(), expected );
}


QGSTEST_MAIN( TestQgsCompoundCurve )
#include "testqgscompoundcurve.moc"
