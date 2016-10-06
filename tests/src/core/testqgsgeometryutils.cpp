/***************************************************************************
     testqgsgeometryutils.cpp
     --------------------------------------
    Date                 : 16 Jan 2015
    Copyright            : (C) 2015 by Marco Hugentobler
    Email                : marco at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>
#include <QObject>
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsmultipolygon.h"
#include "qgstestutils.h"

class TestQgsGeometryUtils: public QObject
{
    Q_OBJECT
  private slots:
    void testExtractLinestrings();
    void testCircleClockwise_data();
    void testCircleClockwise();
    void testAngleOnCircle_data();
    void testAngleOnCircle();
    void testLeftOfLine_data();
    void testLeftOfLine();
    void testSegmentMidPoint_data();
    void testSegmentMidPoint();
    void testCircleLength_data();
    void testCircleLength();
    void testNormalizedAngle_data();
    void testNormalizedAngle();
    void testLineAngle_data();
    void testLineAngle();
    void testLinePerpendicularAngle_data();
    void testLinePerpendicularAngle();
    void testAverageAngle_data();
    void testAverageAngle();
    void testAdjacentVertices();
    void testDistanceToVertex();
    void testVerticesAtDistance();
    void testCircleCenterRadius_data();
    void testCircleCenterRadius();
    void testSqrDistToLine();
    void testAngleThreePoints();
};


void TestQgsGeometryUtils::testExtractLinestrings()
{
  QgsLineString* outerRing1 = new QgsLineString();
  outerRing1->setPoints( QList<QgsPointV2>() << QgsPointV2( 1, 1 ) << QgsPointV2( 1, 2 ) << QgsPointV2( 2, 2 ) << QgsPointV2( 2, 1 ) << QgsPointV2( 1, 1 ) );
  QgsPolygonV2* polygon1 = new QgsPolygonV2();
  polygon1->setExteriorRing( outerRing1 );

  QgsLineString* outerRing2 = new QgsLineString();
  outerRing2->setPoints( QList<QgsPointV2>() << QgsPointV2( 10, 10 ) << QgsPointV2( 10, 20 ) << QgsPointV2( 20, 20 ) << QgsPointV2( 20, 10 ) << QgsPointV2( 10, 10 ) );
  QgsPolygonV2* polygon2 = new QgsPolygonV2();
  polygon2->setExteriorRing( outerRing2 );

  QgsLineString* innerRing2 = new QgsLineString();
  innerRing2->setPoints( QList<QgsPointV2>() << QgsPointV2( 14, 14 ) << QgsPointV2( 14, 16 ) << QgsPointV2( 16, 16 ) << QgsPointV2( 16, 14 ) << QgsPointV2( 14, 14 ) );
  polygon2->setInteriorRings( QList<QgsCurve*>() << innerRing2 );

  QgsMultiPolygonV2 mpg;
  mpg.addGeometry( polygon1 );
  mpg.addGeometry( polygon2 );

  QList<QgsLineString*> linestrings = QgsGeometryUtils::extractLineStrings( &mpg );
  QCOMPARE( linestrings.count(), 3 );
  qDeleteAll( linestrings );
}

void TestQgsGeometryUtils::testLeftOfLine_data()
{
  QTest::addColumn<double>( "x" );
  QTest::addColumn<double>( "y" );
  QTest::addColumn<double>( "x1" );
  QTest::addColumn<double>( "y1" );
  QTest::addColumn<double>( "x2" );
  QTest::addColumn<double>( "y2" );
  QTest::addColumn<bool>( "expectedResult" );

  QTest::newRow( "leftOfLine1" ) << 0.0 << 0.0 << 0.0 << 2.0 << 1.0 << 0.0 << false;
  QTest::newRow( "leftOfLine1" ) << 0.0 << 0.0 << 1.0 << 0.0 << 0.0 << 2.0 << true;
}

void TestQgsGeometryUtils::testLeftOfLine()
{
  QFETCH( double, x );
  QFETCH( double, y );
  QFETCH( double, x1 );
  QFETCH( double, y1 );
  QFETCH( double, x2 );
  QFETCH( double, y2 );
  QFETCH( bool, expectedResult );

  QCOMPARE( QgsGeometryUtils::leftOfLine( x, y, x1, y1, x2, y2 ) < 0, expectedResult );
}

void TestQgsGeometryUtils::testCircleClockwise_data()
{
  QTest::addColumn<double>( "angle1" );
  QTest::addColumn<double>( "angle2" );
  QTest::addColumn<double>( "angle3" );
  QTest::addColumn<bool>( "expectedResult" );

  QTest::newRow( "circleClockwise1" ) << 10.0 << 300.0 << 270.0 << true;
  QTest::newRow( "circleClockwise2" ) << 270.0 << 300.0 << 10.0 << false;
  QTest::newRow( "circleClockwise3" ) << 260.0 << 245.0 << 243.0 << true;
}

void TestQgsGeometryUtils::testCircleClockwise()
{
  QFETCH( double, angle1 );
  QFETCH( double, angle2 );
  QFETCH( double, angle3 );
  QFETCH( bool, expectedResult );

  QCOMPARE( QgsGeometryUtils::circleClockwise( angle1, angle2, angle3 ), expectedResult );
}

void TestQgsGeometryUtils::testAngleOnCircle_data()
{
  QTest::addColumn<double>( "angle" );
  QTest::addColumn<double>( "angle1" );
  QTest::addColumn<double>( "angle2" );
  QTest::addColumn<double>( "angle3" );
  QTest::addColumn<bool>( "expectedResult" );

  QTest::newRow( "angleOnCircle1" ) << 280.0 << 10.0 << 300.0 << 270.0 << true;
  QTest::newRow( "angleOnCircle2" ) << 235.0 << 10.0 << 300.0 << 270.0 << false;
  QTest::newRow( "angleOnCircle3" ) << 280.0 << 45.0 << 175.0 << 35.0 << true;
  QTest::newRow( "angleOnCircle4" ) << 40.0 << 45.0 << 175.0 << 35.0 << false;
  QTest::newRow( "angleOnCircle5" ) << 335.0 << 350.0 << 175.0 << 320.0 << false;
  QTest::newRow( "angleOnCircle6" ) << 170.0 << 140.0 << 355.0 << 205.0 << false;
}

void TestQgsGeometryUtils::testAngleOnCircle()
{
  QFETCH( double, angle );
  QFETCH( double, angle1 );
  QFETCH( double, angle2 );
  QFETCH( double, angle3 );
  QFETCH( bool, expectedResult );

  QCOMPARE( QgsGeometryUtils::angleOnCircle( angle, angle1, angle2, angle3 ), expectedResult );
}

void TestQgsGeometryUtils::testSegmentMidPoint_data()
{
  QTest::addColumn<double>( "pt1x" );
  QTest::addColumn<double>( "pt1y" );
  QTest::addColumn<double>( "pt2x" );
  QTest::addColumn<double>( "pt2y" );
  QTest::addColumn<double>( "radius" );
  QTest::addColumn<bool>( "left" );
  QTest::addColumn<double>( "expectedX" );
  QTest::addColumn<double>( "expectedY" );

  QTest::newRow( "testSegmentMidPoint1" ) << 0.0 << 0.0 << 1.0 << 0.0 << 0.5 << true << 0.5 << 0.5;
}

void TestQgsGeometryUtils::testSegmentMidPoint()
{
  QFETCH( double, pt1x );
  QFETCH( double, pt1y );
  QFETCH( double, pt2x );
  QFETCH( double, pt2y );
  QFETCH( double, radius );
  QFETCH( bool, left );
  QFETCH( double, expectedX );
  QFETCH( double, expectedY );

  QgsPointV2 midPoint;
  bool ok = QgsGeometryUtils::segmentMidPoint( QgsPointV2( pt1x, pt1y ), QgsPointV2( pt2x, pt2y ),
            midPoint, radius, left );

  QVERIFY( ok );
  QVERIFY( qgsDoubleNear( midPoint.x(), expectedX ) );
  QVERIFY( qgsDoubleNear( midPoint.y(), expectedY ) );
}

void TestQgsGeometryUtils::testCircleLength_data()
{
  QTest::addColumn<double>( "x1" );
  QTest::addColumn<double>( "y1" );
  QTest::addColumn<double>( "x2" );
  QTest::addColumn<double>( "y2" );
  QTest::addColumn<double>( "x3" );
  QTest::addColumn<double>( "y3" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "circleLength1" ) << 1.0 << 0.0 << -1.0 << 0.0 << 0.0 << 1.0 << ( 2 * M_PI * 0.75 );
  QTest::newRow( "circleLength2" ) << 0.0 << 1.0 << -1.0 << 0.0 << 1.0 << 0.0 << ( 2 * M_PI * 0.75 );
}

void TestQgsGeometryUtils::testCircleLength()
{
  QFETCH( double, x1 );
  QFETCH( double, y1 );
  QFETCH( double, x2 );
  QFETCH( double, y2 );
  QFETCH( double, x3 );
  QFETCH( double, y3 );
  QFETCH( double, expected );

  QVERIFY( qgsDoubleNear( expected, QgsGeometryUtils::circleLength( x1, y1, x2, y2, x3, y3 ) ) );
}

void TestQgsGeometryUtils::testNormalizedAngle_data()
{
  QTest::addColumn<double>( "input" );
  QTest::addColumn<double>( "expected" );
  QTest::newRow( "normalizedAngle 0" ) << 0.0 << 0.0;
  QTest::newRow( "normalizedAngle 1.5708" ) << 1.5708 << 1.5708;
  QTest::newRow( "normalizedAngle 3.1416" ) << 3.1416 << 3.1416;
  QTest::newRow( "normalizedAngle 4.7124" ) << 4.7124 << 4.7124;
  QTest::newRow( "normalizedAngle 2 * M_PI" ) << 2 * M_PI << 0.0;
  QTest::newRow( "normalizedAngle 6.80678" ) << 6.80678 << 0.5236;
  QTest::newRow( "normalizedAngle 12.5664" ) << 12.5664 << 0.0;
  QTest::newRow( "normalizedAngle 12.7409" ) << 12.7409 << 0.174533;
  QTest::newRow( "normalizedAngle -0.174533" ) << -0.174533 << 6.10865;
  QTest::newRow( "normalizedAngle -6.28318" ) << -6.28318 << 0.0;
  QTest::newRow( "normalizedAngle -6.45772" ) << -6.45772 << 6.10865;
  QTest::newRow( "normalizedAngle -13.2645" ) << -13.2645 << 5.58505;
}

void TestQgsGeometryUtils::testNormalizedAngle()
{
  QFETCH( double, input );
  QFETCH( double, expected );
  QVERIFY( qgsDoubleNear( expected, QgsGeometryUtils::normalizedAngle( input ), 0.0001 ) );
}

void TestQgsGeometryUtils::testLineAngle_data()
{
  QTest::addColumn<double>( "x1" );
  QTest::addColumn<double>( "y1" );
  QTest::addColumn<double>( "x2" );
  QTest::addColumn<double>( "y2" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "lineAngle undefined" ) << 0.0 << 0.0 << 0.0 << 0.0 << -99999.0; //value is unimportant, we just don't want a crash
  QTest::newRow( "lineAngle1" ) << 0.0 << 0.0 << 10.0 << 10.0 << 45.0;
  QTest::newRow( "lineAngle2" ) << 0.0 << 0.0 << 10.0 << 0.0 << 90.0;
  QTest::newRow( "lineAngle3" ) << 0.0 << 0.0 << 10.0 << -10.0 << 135.0;
  QTest::newRow( "lineAngle4" ) << 0.0 << 0.0 << 0.0 << -10.0 << 180.0;
  QTest::newRow( "lineAngle5" ) << 0.0 << 0.0 << -10.0 << -10.0 << 225.0;
  QTest::newRow( "lineAngle6" ) << 0.0 << 0.0 << -10.0 << 0.0 << 270.0;
  QTest::newRow( "lineAngle7" ) << 0.0 << 0.0 << -10.0 << 10.0 << 315.0;
  QTest::newRow( "lineAngle8" ) << 0.0 << 0.0 << 0.0 << 10.0 << 0.0;
}

void TestQgsGeometryUtils::testLineAngle()
{
  QFETCH( double, x1 );
  QFETCH( double, y1 );
  QFETCH( double, x2 );
  QFETCH( double, y2 );
  QFETCH( double, expected );

  double lineAngle = QgsGeometryUtils::lineAngle( x1, y1, x2, y2 ) * 180 / M_PI;
  if ( expected > -99999 )
    QVERIFY( qgsDoubleNear( lineAngle, expected ) );
}

void TestQgsGeometryUtils::testLinePerpendicularAngle_data()
{
  QTest::addColumn<double>( "x1" );
  QTest::addColumn<double>( "y1" );
  QTest::addColumn<double>( "x2" );
  QTest::addColumn<double>( "y2" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "linePerpendicularAngle undefined" ) << 0.0 << 0.0 << 0.0 << 0.0 << -99999.0; //value is unimportant, we just don't want a crash
  QTest::newRow( "linePerpendicularAngle1" ) << 0.0 << 0.0 << 10.0 << 10.0 << 135.0;
  QTest::newRow( "linePerpendicularAngle2" ) << 0.0 << 0.0 << 10.0 << 0.0 << 180.0;
  QTest::newRow( "linePerpendicularAngle3" ) << 0.0 << 0.0 << 10.0 << -10.0 << 225.0;
  QTest::newRow( "linePerpendicularAngle4" ) << 0.0 << 0.0 << 0.0 << -10.0 << 270.0;
  QTest::newRow( "linePerpendicularAngle5" ) << 0.0 << 0.0 << -10.0 << -10.0 << 315.0;
  QTest::newRow( "linePerpendicularAngle6" ) << 0.0 << 0.0 << -10.0 << 0.0 << 0.0;
  QTest::newRow( "linePerpendicularAngle7" ) << 0.0 << 0.0 << -10.0 << 10.0 << 45.0;
  QTest::newRow( "linePerpendicularAngle8" ) << 0.0 << 0.0 << 0.0 << 10.0 << 90.0;
}

void TestQgsGeometryUtils::testLinePerpendicularAngle()
{
  QFETCH( double, x1 );
  QFETCH( double, y1 );
  QFETCH( double, x2 );
  QFETCH( double, y2 );
  QFETCH( double, expected );

  double pAngle = QgsGeometryUtils::linePerpendicularAngle( x1, y1, x2, y2 ) * 180 / M_PI;
  if ( expected > -99999 )
    QVERIFY( qgsDoubleNear( pAngle, expected, 0.01 ) );
}

void TestQgsGeometryUtils::testAverageAngle_data()
{
  QTest::addColumn<double>( "angle1" );
  QTest::addColumn<double>( "angle2" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "testAverageAngle1" ) << 0.0 << 0.0 << 0.0;
  QTest::newRow( "testAverageAngle2" ) << 0.0 << 360.0 << 0.0;
  QTest::newRow( "testAverageAngle3" ) << 0.0 << 720.0 << 0.0;
  QTest::newRow( "testAverageAngle4" ) << 360.0 << 0.0 << 0.0;
  QTest::newRow( "testAverageAngle5" ) << -360.0 << 0.0 << 0.0;
  QTest::newRow( "testAverageAngle6" ) << -360.0 << -360.0 << 0.0;
  QTest::newRow( "testAverageAngle7" ) << 0.0 << 180.0 << 90.0;
  QTest::newRow( "testAverageAngle8" ) << 0.0 << -179.999999999999 << 270.0;
  QTest::newRow( "testAverageAngle9" ) << 315.0 << 270.0 << 292.5;
  QTest::newRow( "testAverageAngle10" ) << 45.0 << 135.0 << 90.0;
  QTest::newRow( "testAverageAngle11" ) << 315.0 << 45.0 << 0.0;
  QTest::newRow( "testAverageAngle12" ) << 45.0 << 315.0 << 0.0;
  QTest::newRow( "testAverageAngle13" ) << 315.0 << 270.0 << 292.5;
  QTest::newRow( "testAverageAngle14" ) << 140.0 << 240.0 << 190.0;
  QTest::newRow( "testAverageAngle15" ) << 240.0 << 140.0 << 190.0;
}

void TestQgsGeometryUtils::testAverageAngle()
{
  QFETCH( double, angle1 );
  QFETCH( double, angle2 );
  QFETCH( double, expected );

  double averageAngle = QgsGeometryUtils::averageAngle( angle1 * M_PI / 180.0, angle2 * M_PI / 180.0 ) * 180.0 / M_PI;
  QVERIFY( qgsDoubleNear( averageAngle, expected, 0.0000000001 ) );
}

void TestQgsGeometryUtils::testAdjacentVertices()
{
  // test polygon - should wrap around!
  QgsLineString* closedRing1 = new QgsLineString();
  closedRing1->setPoints( QList<QgsPointV2>() << QgsPointV2( 1, 1 ) << QgsPointV2( 1, 2 ) << QgsPointV2( 2, 2 ) << QgsPointV2( 2, 1 ) << QgsPointV2( 1, 1 ) );
  QgsPolygonV2 polygon1;
  polygon1.setExteriorRing( closedRing1 );
  QgsVertexId previous;
  QgsVertexId next;

  QgsGeometryUtils::adjacentVertices( polygon1, QgsVertexId( 0, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );

  // test point - both vertices should be invalid
  QgsPointV2 point( 1, 2 );
  QgsGeometryUtils::adjacentVertices( point, QgsVertexId( 0, 0, 0 ), previous, next );
  QVERIFY( !previous.isValid() );
  QVERIFY( !next.isValid() );
}

void TestQgsGeometryUtils::testDistanceToVertex()
{
  //test with linestring
  QgsLineString* outerRing1 = new QgsLineString();
  outerRing1->setPoints( QList<QgsPointV2>() << QgsPointV2( 1, 1 ) << QgsPointV2( 1, 2 ) << QgsPointV2( 2, 2 ) << QgsPointV2( 2, 1 ) << QgsPointV2( 1, 1 ) );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 1 ) ), 1.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 2 ) ), 2.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 3 ) ), 3.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 4 ) ), 4.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 5 ) ), -1.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 1, 1 ) ), -1.0 );

  //test with polygon
  QgsPolygonV2 polygon1;
  polygon1.setExteriorRing( outerRing1 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 1 ) ), 1.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 2 ) ), 2.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 3 ) ), 3.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 4 ) ), 4.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 5 ) ), -1.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 1, 1 ) ), -1.0 );

  //test with point
  QgsPointV2 point( 1, 2 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( point, QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( point, QgsVertexId( 0, 0, 1 ) ), -1.0 );
}

void TestQgsGeometryUtils::testVerticesAtDistance()
{
  //test with linestring
  QgsLineString* outerRing1 = new QgsLineString();
  QgsVertexId previous;
  QgsVertexId next;
  QVERIFY( !QgsGeometryUtils::verticesAtDistance( *outerRing1, .5, previous, next ) );

  outerRing1->setPoints( QList<QgsPointV2>() << QgsPointV2( 1, 1 ) << QgsPointV2( 1, 2 ) << QgsPointV2( 2, 2 ) << QgsPointV2( 2, 1 ) << QgsPointV2( 3, 1 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, .5, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, 1.5, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, 2.5, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 3 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, 3.5, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );
  QVERIFY( ! QgsGeometryUtils::verticesAtDistance( *outerRing1, 4.5, previous, next ) );

  // test exact hits
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, 0, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 0 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, 1, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, 2, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, 3, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 3 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *outerRing1, 4, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 4 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );

  // test closed line
  QgsLineString* closedRing1 = new QgsLineString();
  closedRing1->setPoints( QList<QgsPointV2>() << QgsPointV2( 1, 1 ) << QgsPointV2( 1, 2 ) << QgsPointV2( 2, 2 ) << QgsPointV2( 2, 1 ) << QgsPointV2( 1, 1 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *closedRing1, 0, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 0 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *closedRing1, 4, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 4 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );

  // test with polygon
  QgsPolygonV2 polygon1;
  polygon1.setExteriorRing( closedRing1 );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( polygon1, .5, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( polygon1, 1.5, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( polygon1, 2.5, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 3 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( polygon1, 3.5, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );
  QVERIFY( ! QgsGeometryUtils::verticesAtDistance( polygon1, 4.5, previous, next ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( polygon1, 0, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 0 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( polygon1, 4, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 4 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );

  //test with point
  QgsPointV2 point( 1, 2 );
  QVERIFY( !QgsGeometryUtils::verticesAtDistance( point, .5, previous, next ) );
}

void TestQgsGeometryUtils::testCircleCenterRadius_data()
{
  QTest::addColumn<double>( "x1" );
  QTest::addColumn<double>( "y1" );
  QTest::addColumn<double>( "x2" );
  QTest::addColumn<double>( "y2" );
  QTest::addColumn<double>( "x3" );
  QTest::addColumn<double>( "y3" );
  QTest::addColumn<double>( "expectedRadius" );
  QTest::addColumn<double>( "expectedCenterX" );
  QTest::addColumn<double>( "expectedCenterY" );

  QTest::newRow( "circleCenterRadius1" ) << 1.0 << 1.0 << 5.0 << 7.0 << 1.0 << 1.0 << sqrt( 13.0 ) << 3.0 << 4.0;
  QTest::newRow( "circleCenterRadius1" ) << 0.0 << 2.0 << 2.0 << 2.0 << 0.0 << 2.0 << 1.0 << 1.0 << 2.0;
}

void TestQgsGeometryUtils::testCircleCenterRadius()
{
  QFETCH( double, x1 );
  QFETCH( double, y1 );
  QFETCH( double, x2 );
  QFETCH( double, y2 );
  QFETCH( double, x3 );
  QFETCH( double, y3 );
  QFETCH( double, expectedRadius );
  QFETCH( double, expectedCenterX );
  QFETCH( double, expectedCenterY );

  double radius, centerX, centerY;
  QgsGeometryUtils::circleCenterRadius( QgsPointV2( x1, y1 ), QgsPointV2( x2, y2 ), QgsPointV2( x3, y3 ), radius, centerX, centerY );
  QVERIFY( qgsDoubleNear( expectedRadius, radius ) );
  QVERIFY( qgsDoubleNear( expectedCenterX, centerX ) );
  QVERIFY( qgsDoubleNear( expectedCenterY, centerY ) );
}

//QgsGeometryUtils::sqrDistToLine
void TestQgsGeometryUtils::testSqrDistToLine()
{

  // See http://hub.qgis.org/issues/13952#note-26
  QgsPoint qp( 771938, 6.95593e+06 );
  QgsPoint p1( 771946, 6.95593e+06 );
  QgsPoint p2( 771904, 6.95595e+06 );
  double rx = 0, ry = 0;
  double epsilon = 1e-18;
  double sqrDist = QgsGeometryUtils::sqrDistToLine( qp.x(), qp.y(),
                   p1.x(), p1.y(),
                   p2.x(), p2.y(),
                   rx, ry, epsilon );
  QGSCOMPARENEAR( sqrDist, 11.83, 0.01 );
}

void TestQgsGeometryUtils::testAngleThreePoints()
{
  QgsPoint p1( 0, 0 );
  QgsPoint p2( 1, 0 );
  QgsPoint p3( 1, 1 );
  QGSCOMPARENEAR( QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), M_PI / 2.0, 0.00000001 );
  p3 = QgsPoint( 1, -1 );
  QGSCOMPARENEAR( QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), 3 * M_PI / 2.0, 0.00000001 );
  p3 = QgsPoint( 2, 0 );
  QGSCOMPARENEAR( QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), M_PI, 0.00000001 );
  p3 = QgsPoint( 0, 0 );
  QGSCOMPARENEAR( QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), 0.0, 0.00000001 );
  p3 = QgsPoint( 1, 0 );
  //undefined, but want no crash
  ( void )QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() );
  p2 = QgsPoint( 0, 0 );
  ( void )QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() );
}



QTEST_MAIN( TestQgsGeometryUtils )
#include "testqgsgeometryutils.moc"
