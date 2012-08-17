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
#include <QtTest>
#include <QObject>
#include <QString>
#include <QObject>
//header for class being tested
#include <qgsrectangle.h>
#include <qgspoint.h>
#include "qgslogger.h"

class TestQgsRectangle: public QObject
{
    Q_OBJECT;
  private slots:
    void manipulate();
    void regression6194();
  private:
    QgsRectangle mRect1;
    QgsRectangle mRect2;
    QgsRectangle mRect3;
    QgsRectangle mRect4;
    QgsPoint     mPoint1;
    QgsPoint     mPoint2;
};

void TestQgsRectangle::manipulate()
{
  // Set up two intersecting rectangles and normalize
  mRect1.set( 4.0, 5.0, 1.0, 2.0 );
  mRect2.set( 3.0, 3.0, 7.0, 1.0 );
  // Check intersection
  QVERIFY( mRect2.intersects( mRect1 ) );
  // Create intersection
  mRect3 = mRect2.intersect( & mRect1 );
  // Check width and height (real numbers, careful)
  QCOMPARE( mRect3.width(), 1.0 );
  QCOMPARE( mRect3.height(), 1.0 );
  // And check that the result is contained in both
  QVERIFY( mRect1.contains( mRect3 ) );
  QVERIFY( mRect2.contains( mRect3 ) );
  QVERIFY( ! mRect2.contains( mRect1 ) );

  // Create the union
  mRect3.unionRect( mRect1 );
  mRect3.unionRect( mRect2 );
  // Check union
  QVERIFY( mRect3 == QgsRectangle( 1.0, 1.0, 7.0, 5.0 ) );
};

void TestQgsRectangle::regression6194()
{
  // 100 wide, 200 high
  mRect1 = QgsRectangle( 10.0, 20.0, 110.0, 220.0 );

  // 250 wide, 500 high
  mRect2.setXMinimum( 10.0 );
  mRect2.setYMinimum( 20.0 );
  mRect2.setXMaximum( 260.0 );
  mRect2.setYMaximum( 520.0 );

  // Scale by 2.5, keeping bottom left as is.
  mRect1.scale( 2.5, & QgsPoint( 135.0, 270.0 ) );

  QVERIFY( mRect2.xMinimum() == mRect1.xMinimum() );
  QVERIFY( mRect2.yMinimum() == mRect1.yMinimum() );
  QVERIFY( mRect2.xMaximum() == mRect1.xMaximum() );
  QVERIFY( mRect2.yMaximum() == mRect1.yMaximum() );
  QVERIFY( mRect1 == mRect2 );
};

QTEST_MAIN( TestQgsRectangle )
#include "moc_testqgsrectangle.cxx"




