/***************************************************************************
     testqgsrectangle.cpp
     --------------------------------------
    Date                 : Tue 14 Aug 2012
    Copyright            : (C) 2012 by Magnus Homann
    Email                : magnus at homann dot se
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
#include <QString>
//header for class being tested
#include <qgsrectangle.h>
#include <qgspoint.h>
#include "qgslogger.h"

class TestQgsRectangle: public QObject
{
    Q_OBJECT
  private slots:
    void manipulate();
    void regression6194();
};

void TestQgsRectangle::manipulate()
{
  // Set up two intersecting rectangles and normalize
  QgsRectangle rect1, rect2, rect3;
  rect1.set( 4.0, 5.0, 1.0, 2.0 );
  rect2.set( 3.0, 3.0, 7.0, 1.0 );
  // Check intersection
  QVERIFY( rect2.intersects( rect1 ) );
  // Create intersection
  rect3 = rect2.intersect( &rect1 );
  // Check width and height (real numbers, careful)
  QCOMPARE( rect3.width(), 1.0 );
  QCOMPARE( rect3.height(), 1.0 );
  // And check that the result is contained in both
  QVERIFY( rect1.contains( rect3 ) );
  QVERIFY( rect2.contains( rect3 ) );
  QVERIFY( ! rect2.contains( rect1 ) );

  // Create the union
  rect3.unionRect( rect1 );
  rect3.unionRect( rect2 );
  // Check union
  QVERIFY( rect3 == QgsRectangle( 1.0, 1.0, 7.0, 5.0 ) );
};

void TestQgsRectangle::regression6194()
{
  // 100 wide, 200 high
  QgsRectangle rect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );

  // Test conversion to QRectF and back
  QRectF qRectF = rect1.toRectF();
  QCOMPARE( qRectF.width(), 100.0 );
  QCOMPARE( qRectF.height(), 200.0 );
  QCOMPARE( qRectF.x(), 10.0 );
  QCOMPARE( qRectF.y(), 20.0 );
  QgsRectangle rect4 = QgsRectangle( qRectF );
  QCOMPARE( rect4.toString( 2 ), QString( "10.00,20.00 : 110.00,220.00" ) );

  // 250 wide, 500 high
  QgsRectangle rect2;
  rect2.setXMinimum( 10.0 );
  rect2.setYMinimum( 20.0 );
  rect2.setXMaximum( 260.0 );
  rect2.setYMaximum( 520.0 );

  // Scale by 2.5, keeping bottom left as is.
  QgsPoint p( 135.0, 270.0 );
  rect1.scale( 2.5, &p );

  QVERIFY( rect2.xMinimum() == rect1.xMinimum() );
  QVERIFY( rect2.yMinimum() == rect1.yMinimum() );
  QVERIFY( rect2.xMaximum() == rect1.xMaximum() );
  QVERIFY( rect2.yMaximum() == rect1.yMaximum() );
  QVERIFY( rect1 == rect2 );
};

QTEST_MAIN( TestQgsRectangle )
#include "testqgsrectangle.moc"




