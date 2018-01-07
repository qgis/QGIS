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

#include "qgstest.h"
#include <QObject>
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsmultipolygon.h"

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
    void testDistanceToVertex();
    void testVerticesAtDistance();
    void testCircleCenterRadius_data();
    void testCircleCenterRadius();
    void testSqrDistToLine();
    void testAngleThreePoints();
    void testMidPoint();
    void testGradient();
    void testCoefficients();
    void testPerpendicularSegment();
    void testClosestPoint();
};


void TestQgsGeometryUtils::testExtractLinestrings()
{
  QgsLineString *outerRing1 = new QgsLineString();
  outerRing1->setPoints( QVector<QgsPoint>() << QgsPoint( 1, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  QgsPolygon *polygon1 = new QgsPolygon();
  polygon1->setExteriorRing( outerRing1 );

  QgsLineString *outerRing2 = new QgsLineString();
  outerRing2->setPoints( QVector<QgsPoint>() << QgsPoint( 10, 10 ) << QgsPoint( 10, 20 ) << QgsPoint( 20, 20 ) << QgsPoint( 20, 10 ) << QgsPoint( 10, 10 ) );
  QgsPolygon *polygon2 = new QgsPolygon();
  polygon2->setExteriorRing( outerRing2 );

  QgsLineString *innerRing2 = new QgsLineString();
  innerRing2->setPoints( QVector<QgsPoint>() << QgsPoint( 14, 14 ) << QgsPoint( 14, 16 ) << QgsPoint( 16, 16 ) << QgsPoint( 16, 14 ) << QgsPoint( 14, 14 ) );
  polygon2->setInteriorRings( QVector<QgsCurve *>() << innerRing2 );

  QgsMultiPolygon mpg;
  mpg.addGeometry( polygon1 );
  mpg.addGeometry( polygon2 );

  QVector<QgsLineString *> linestrings = QgsGeometryUtils::extractLineStrings( &mpg );
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

  QgsPoint midPoint;
  bool ok = QgsGeometryUtils::segmentMidPoint( QgsPoint( pt1x, pt1y ), QgsPoint( pt2x, pt2y ),
            midPoint, radius, left );

  QVERIFY( ok );
  QGSCOMPARENEAR( midPoint.x(), expectedX, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( midPoint.y(), expectedY, 4 * DBL_EPSILON );
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

  QGSCOMPARENEAR( expected, QgsGeometryUtils::circleLength( x1, y1, x2, y2, x3, y3 ), 4 * DBL_EPSILON );
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
  QGSCOMPARENEAR( expected, QgsGeometryUtils::normalizedAngle( input ), 0.0001 );
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
    QGSCOMPARENEAR( lineAngle, expected, 4 * DBL_EPSILON );
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
    QGSCOMPARENEAR( pAngle, expected, 0.01 );
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
  QGSCOMPARENEAR( averageAngle, expected, 0.0000000001 );
}

void TestQgsGeometryUtils::testDistanceToVertex()
{
  //test with linestring
  QgsLineString *outerRing1 = new QgsLineString();
  outerRing1->setPoints( QVector<QgsPoint>() << QgsPoint( 1, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 1 ) ), 1.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 2 ) ), 2.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 3 ) ), 3.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 4 ) ), 4.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 0, 5 ) ), -1.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( *outerRing1, QgsVertexId( 0, 1, 1 ) ), -1.0 );

  //test with polygon
  QgsPolygon polygon1;
  polygon1.setExteriorRing( outerRing1 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 1 ) ), 1.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 2 ) ), 2.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 3 ) ), 3.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 4 ) ), 4.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 0, 5 ) ), -1.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( polygon1, QgsVertexId( 0, 1, 1 ) ), -1.0 );

  //test with point
  QgsPoint point( 1, 2 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( point, QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( QgsGeometryUtils::distanceToVertex( point, QgsVertexId( 0, 0, 1 ) ), -1.0 );
}

void TestQgsGeometryUtils::testVerticesAtDistance()
{
  //test with linestring
  QgsLineString *outerRing1 = new QgsLineString();
  QgsVertexId previous;
  QgsVertexId next;
  QVERIFY( !QgsGeometryUtils::verticesAtDistance( *outerRing1, .5, previous, next ) );

  outerRing1->setPoints( QVector<QgsPoint>() << QgsPoint( 1, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 ) << QgsPoint( 2, 1 ) << QgsPoint( 3, 1 ) );
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
  delete outerRing1;

  // test closed line
  QgsLineString *closedRing1 = new QgsLineString();
  closedRing1->setPoints( QVector<QgsPoint>() << QgsPoint( 1, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *closedRing1, 0, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 0 ) );
  QVERIFY( QgsGeometryUtils::verticesAtDistance( *closedRing1, 4, previous, next ) );
  QCOMPARE( previous, QgsVertexId( 0, 0, 4 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );

  // test with polygon
  QgsPolygon polygon1;
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
  QgsPoint point( 1, 2 );
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

  QTest::newRow( "circleCenterRadius1" ) << 1.0 << 1.0 << 5.0 << 7.0 << 1.0 << 1.0 << std::sqrt( 13.0 ) << 3.0 << 4.0;
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
  QgsGeometryUtils::circleCenterRadius( QgsPoint( x1, y1 ), QgsPoint( x2, y2 ), QgsPoint( x3, y3 ), radius, centerX, centerY );
  QGSCOMPARENEAR( expectedRadius, radius, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( expectedCenterX, centerX, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( expectedCenterY, centerY, 4 * DBL_EPSILON );
}

//QgsGeometryUtils::sqrDistToLine
void TestQgsGeometryUtils::testSqrDistToLine()
{

  // See https://issues.qgis.org/issues/13952#note-26
  QgsPointXY qp( 771938, 6.95593e+06 );
  QgsPointXY p1( 771946, 6.95593e+06 );
  QgsPointXY p2( 771904, 6.95595e+06 );
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
  QgsPointXY p1( 0, 0 );
  QgsPointXY p2( 1, 0 );
  QgsPointXY p3( 1, 1 );
  QGSCOMPARENEAR( QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), M_PI / 2.0, 0.00000001 );
  p3 = QgsPointXY( 1, -1 );
  QGSCOMPARENEAR( QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), 3 * M_PI / 2.0, 0.00000001 );
  p3 = QgsPointXY( 2, 0 );
  QGSCOMPARENEAR( QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), M_PI, 0.00000001 );
  p3 = QgsPointXY( 0, 0 );
  QGSCOMPARENEAR( QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), 0.0, 0.00000001 );
  p3 = QgsPointXY( 1, 0 );
  //undefined, but want no crash
  ( void )QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() );
  p2 = QgsPointXY( 0, 0 );
  ( void )QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() );
}

void TestQgsGeometryUtils::testMidPoint()
{
  QgsPoint p1( 4, 6 );
  QCOMPARE( QgsGeometryUtils::midpoint( p1, QgsPoint( 2, 2 ) ), QgsPoint( 3, 4 ) );
  QCOMPARE( QgsGeometryUtils::midpoint( QgsPoint( 4, 6, 0 ), QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2 ) ), QgsPoint( QgsWkbTypes::PointZ, 3, 4, 1 ) );
  QCOMPARE( QgsGeometryUtils::midpoint( QgsPoint( QgsWkbTypes::PointM, 4, 6, 0, 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 2, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 1 ) );
  QCOMPARE( QgsGeometryUtils::midpoint( QgsPoint( QgsWkbTypes::PointZM, 4, 6, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 2, 2, 2, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 3, 4, 1, 1 ) );
}

void TestQgsGeometryUtils::testGradient()
{
  QVERIFY( QgsGeometryUtils::gradient( QgsPoint( 4, 6 ), QgsPoint( 4, 8 ) ) == INFINITY );
  QGSCOMPARENEAR( QgsGeometryUtils::gradient( QgsPoint( 2, 8 ), QgsPoint( 3, 20 ) ), 12, 0.00000001 );
  QGSCOMPARENEAR( QgsGeometryUtils::gradient( QgsPoint( 2, -88 ), QgsPoint( 4, -4 ) ), 42, 0.00000001 );
  QGSCOMPARENEAR( QgsGeometryUtils::gradient( QgsPoint( 4, 6 ), QgsPoint( 8, 6 ) ), 0, 0.00000001 );
}

void TestQgsGeometryUtils::testCoefficients()
{
  double a, b, c;

  // pt1.x == pt2.x
  QgsGeometryUtils::coefficients( QgsPoint( 4, 6 ), QgsPoint( 4, 8 ), a, b, c );
  QGSCOMPARENEAR( a, 1, 0.00000001 );
  QGSCOMPARENEAR( b, 0, 0.00000001 );
  QGSCOMPARENEAR( c, -4, 0.00000001 );

  // pt1.y == pt2.y
  QgsGeometryUtils::coefficients( QgsPoint( 6, 4 ), QgsPoint( 8, 4 ), a, b, c );
  QGSCOMPARENEAR( a, 0, 0.00000001 );
  QGSCOMPARENEAR( b, 1, 0.00000001 );
  QGSCOMPARENEAR( c, -4, 0.00000001 );

  // else
  QgsGeometryUtils::coefficients( QgsPoint( 6, 4 ), QgsPoint( 4, 8 ), a, b, c );
  QGSCOMPARENEAR( a, -4, 0.00000001 );
  QGSCOMPARENEAR( b, -2, 0.00000001 );
  QGSCOMPARENEAR( c, 32, 0.00000001 );
  QgsGeometryUtils::coefficients( QgsPoint( -4, -2 ), QgsPoint( 4, 2 ), a, b, c );
  QGSCOMPARENEAR( a, -4, 0.00000001 );
  QGSCOMPARENEAR( b, 8, 0.00000001 );
  QGSCOMPARENEAR( c, 0, 0.00000001 );
}
void TestQgsGeometryUtils::testPerpendicularSegment()
{
  QgsPoint p1( 3, 13 );
  QgsPoint s1( 2, 3 );
  QgsPoint s2( 7, 11 );

  QgsLineString line_r = QgsGeometryUtils::perpendicularSegment( p1, s1, s2 );

  // default case
  QgsLineString line;
  line.addVertex( p1 );
  line.addVertex( QgsPoint( 6.7753, 10.6404 ) );
  QGSCOMPARENEARPOINT( line.pointN( 0 ), line_r.pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( line.pointN( 1 ), line_r.pointN( 1 ), 0.0001 );

  // perpendicular line don't intersect segment
  line.clear();
  p1 = QgsPoint( 11, 11 );
  line_r = QgsGeometryUtils::perpendicularSegment( p1, s1, s2 );
  line.addVertex( p1 );
  line.addVertex( QgsPoint( 8.1236, 12.7978 ) );
  QGSCOMPARENEARPOINT( line.pointN( 0 ), line_r.pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( line.pointN( 1 ), line_r.pointN( 1 ), 0.0001 );

  // horizontal
  s1 = QgsPoint( -3, 3 );
  s2 = QgsPoint( 2, 3 );
  line.clear();
  p1 = QgsPoint( 3, 13 );
  line_r = QgsGeometryUtils::perpendicularSegment( p1, s1, s2 );
  line.addVertex( p1 );
  line.addVertex( QgsPoint( 3, 3 ) );
  QCOMPARE( line.pointN( 0 ), line_r.pointN( 0 ) );
  QCOMPARE( line.pointN( 1 ), line_r.pointN( 1 ) );

  // vertical
  s1 = QgsPoint( 3, 13 );
  s2 = QgsPoint( 3, 3 );
  line.clear();
  p1 = QgsPoint( -7, 8 );
  line_r = QgsGeometryUtils::perpendicularSegment( p1, s1, s2 );
  line.addVertex( p1 );
  line.addVertex( QgsPoint( 3, 8 ) );
  QCOMPARE( line.pointN( 0 ), line_r.pointN( 0 ) );
  QCOMPARE( line.pointN( 1 ), line_r.pointN( 1 ) );
}

void TestQgsGeometryUtils::testClosestPoint()
{
  QgsLineString linestringZ( QVector<QgsPoint>()
                             << QgsPoint( 1, 1, 1 )
                             << QgsPoint( 1, 3, 2 ) );

  QgsPoint pt1 = QgsGeometryUtils::closestPoint( linestringZ, QgsPoint( 1, 0 ) );
  QGSCOMPARENEAR( pt1.z(), 1, 0.0001 );
  QVERIFY( std::isnan( pt1.m() ) );

  QgsLineString linestringM( QVector<QgsPoint>()
                             << QgsPoint( 1, 1, std::numeric_limits<double>::quiet_NaN(), 1 )
                             << QgsPoint( 1, 3, std::numeric_limits<double>::quiet_NaN(), 2 ) );

  QgsPoint pt2 = QgsGeometryUtils::closestPoint( linestringM, QgsPoint( 1, 4 ) );
  QVERIFY( std::isnan( pt2.z() ) );
  QGSCOMPARENEAR( pt2.m(), 2, 0.0001 );

  QgsLineString linestringZM( QVector<QgsPoint>()
                              << QgsPoint( 1, 1, 1, 1 )
                              << QgsPoint( 1, 3, 2, 2 ) );

  QgsPoint pt3 = QgsGeometryUtils::closestPoint( linestringZM, QgsPoint( 2, 2 ) );
  QGSCOMPARENEAR( pt3.z(), 1.5, 0.0001 );
  QGSCOMPARENEAR( pt3.m(), 1.5, 0.0001 );

  QgsLineString linestringDuplicatedPoint( QVector<QgsPoint>()
      << QgsPoint( 1, 1, 1, 1 )
      << QgsPoint( 1, 1, 1, 1 )
      << QgsPoint( 1, 3, 2, 2 ) );

  QgsPoint pt4 = QgsGeometryUtils::closestPoint( linestringDuplicatedPoint, QgsPoint( 1, 0 ) );
  QGSCOMPARENEAR( pt4.z(), 1, 0.0001 );
  QGSCOMPARENEAR( pt4.m(), 1, 0.0001 );
}

QGSTEST_MAIN( TestQgsGeometryUtils )
#include "testqgsgeometryutils.moc"
