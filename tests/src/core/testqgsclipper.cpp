/***************************************************************************
     testqgsclipper.cpp
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
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qgsapplication.h>
//header for class being tested
#include <qgsclipper.h>
#include <qgspoint.h>
#include "qgslogger.h"

class TestQgsClipper: public QObject
{

    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase() {};// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.
    void basic();
  private:
    bool checkBoundingBox( QPolygonF polygon, QgsRectangle clipRect );
};

void TestQgsClipper::initTestCase()
{

}

void TestQgsClipper::basic()
{
  // QgsClipper is static only

  QPolygonF polygon;
  polygon << QPointF( 10.4, 20.5 ) << QPointF( 20.2, 30.2 );

  QgsRectangle clipRect( 10, 10, 25, 30 );

  QgsClipper::trimPolygon( polygon, clipRect );

  // Check nothing sticks out.
  QVERIFY( checkBoundingBox( polygon , clipRect ) );
  // Check that it didn't clip too much
  QgsRectangle clipRectInner( clipRect );
  clipRectInner.scale( 0.999 );
  QVERIFY( ! checkBoundingBox( polygon , clipRectInner ) );

  // A more complex example
  polygon.clear();
  polygon << QPointF( 1.0, 9.0 ) << QPointF( 11.0, 11.0 ) << QPointF( 9.0, 1.0 );
  clipRect.set( 0.0, 0.0, 10.0, 10.0 );

  QgsClipper::trimPolygon( polygon, clipRect );

  // We should have 5 vertices now?
  QCOMPARE( polygon.size(), 5 );
  // Check nothing sticks out.
  QVERIFY( checkBoundingBox( polygon , clipRect ) );
  // Check that it didn't clip too much
  clipRectInner = clipRect;
  clipRectInner.scale( 0.999 );
  QVERIFY( ! checkBoundingBox( polygon , clipRectInner ) );
};

bool TestQgsClipper::checkBoundingBox( QPolygonF polygon, QgsRectangle clipRect )
{
  QgsRectangle bBox( polygon.boundingRect() );

  return clipRect.contains( bBox );
}

QTEST_MAIN( TestQgsClipper )
#include "testqgsclipper.moc"
