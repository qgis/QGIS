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

class TestQgsGeometryUtils: public QObject
{
    Q_OBJECT
  private slots:
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
};

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


QTEST_MAIN( TestQgsGeometryUtils )
#include "testqgsgeometryutils.moc"
