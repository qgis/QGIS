/***************************************************************************
     testqgsmaptopixel.cpp
     --------------------------------------
    Date                 : Tue  9 Dec 2014
    Copyright            : (C) 2014 by Sandro Santilli
    Email                : strk@keybit.net
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
#include <qgsrectangle.h>
#include <qgsmaptopixel.h>
#include <qgspoint.h>
#include "qgslogger.h"

class TestQgsMapToPixel: public QObject
{
    Q_OBJECT
  private slots:
    void isValid();
    void rotation();
    void getters();
    void fromScale();
    void equality();
    void toMapCoordinates();
};

void TestQgsMapToPixel::isValid()
{
  // test QgsMapToPixel::isValid()

  // constructors with parameters should result in valid QgsMapToPixel
  QgsMapToPixel m2p( 1, 5, 5, 10, 10, 90 );
  QVERIFY( m2p.isValid() );
  m2p = QgsMapToPixel( 90 );
  QVERIFY( m2p.isValid() );
  m2p = QgsMapToPixel::fromScale( 90, QgsUnitTypes::DistanceMeters );
  QVERIFY( m2p.isValid() );

  // default constructor should result in invalid m2p
  m2p = QgsMapToPixel();
  QVERIFY( !m2p.isValid() );

  // but setting parameters on an invalid m2p should turn it valid
  m2p.setMapUnitsPerPixel( 90 );
  QVERIFY( m2p.isValid() );

  m2p = QgsMapToPixel();
  m2p.setMapRotation( 90, 1, 2 );
  QVERIFY( m2p.isValid() );

  m2p = QgsMapToPixel();
  m2p.setParameters( 90, 1, 2, 50, 70, 0 );
  QVERIFY( m2p.isValid() );

  m2p = QgsMapToPixel();
  bool ok = false;
  m2p.setParameters( 90, 1, 2, 50, 70, 0, &ok );
  QVERIFY( m2p.isValid() );
}

void TestQgsMapToPixel::rotation()
{
  QgsMapToPixel m2p( 1, 5, 5, 10, 10, 90 );

  QgsPointXY p( 5, 5 ); // in geographical units
  QgsPointXY d = m2p.transform( p ); // to device pixels
  QCOMPARE( d.x(), 5.0 ); // center doesn't move
  QCOMPARE( d.y(), 5.0 );

  const QgsPointXY b = m2p.toMapCoordinates( d.x(), d.y() ); // transform back
  QCOMPARE( p, b );

  m2p.transform( &p ); // in place transform
  QCOMPARE( p, d );

  m2p.setParameters( 0.1, 5, 5, 10, 10, -90 );
  p = m2p.toMapCoordinates( 5, 5 );
  QCOMPARE( p.x(), 5.0 ); // center doesn't move
  QCOMPARE( p.y(), 5.0 );
  d = m2p.transform( p );
  QCOMPARE( d, QgsPointXY( 5, 5 ) );

  p = m2p.toMapCoordinates( 10, 0 );
  QCOMPARE( p.x(), 5.5 ); // corner scales and rotates
  QCOMPARE( p.y(), 4.5 );
  d = m2p.transform( p );
  QCOMPARE( d, QgsPointXY( 10, 0 ) );

  m2p.setParameters( 0.1, 5, 5, 10, 10, 360 );
  p = m2p.toMapCoordinates( 10, 0 );
  QCOMPARE( p.x(), 5.5 ); // corner scales
  QCOMPARE( p.y(), 5.5 );
  d = m2p.transform( p );
  QCOMPARE( d, QgsPointXY( 10, 0 ) );

  m2p.setParameters( 0.1, 5, 5, 10, 10, 0 );
  p = m2p.toMapCoordinates( 10, 0 );
  QCOMPARE( p.x(), 5.5 ); // corner scales
  QCOMPARE( p.y(), 5.5 );
  d = m2p.transform( p );
  QCOMPARE( d, QgsPointXY( 10, 0 ) );

}

void TestQgsMapToPixel::getters()
{
  QgsMapToPixel m2p( 1, 5, 6, 10, 100, 90 );
  QCOMPARE( m2p.xCenter(), 5.0 );
  QCOMPARE( m2p.yCenter(), 6.0 );
  QCOMPARE( m2p.mapRotation(), 90.0 );
  QCOMPARE( m2p.mapHeight(), 100 );
  QCOMPARE( m2p.mapWidth(), 10 );
  QCOMPARE( m2p.mapUnitsPerPixel(), 1.0 );

  m2p.setParameters( 2, 10, 12, 20, 200, 180 );
  QCOMPARE( m2p.xCenter(), 10.0 );
  QCOMPARE( m2p.yCenter(), 12.0 );
  QCOMPARE( m2p.mapRotation(), 180.0 );
  QCOMPARE( m2p.mapHeight(), 200 );
  QCOMPARE( m2p.mapWidth(), 20 );
  QCOMPARE( m2p.mapUnitsPerPixel(), 2.0 );
}

void TestQgsMapToPixel::fromScale()
{
  QgsMapToPixel m2p = QgsMapToPixel::fromScale( 1000, QgsUnitTypes::DistanceMeters, 96.0 );
  QGSCOMPARENEAR( m2p.mapUnitsPerPixel(), 0.264583, 0.000001 );
  m2p = QgsMapToPixel::fromScale( 10000, QgsUnitTypes::DistanceMeters, 96.0 );
  QGSCOMPARENEAR( m2p.mapUnitsPerPixel(), 2.645833, 0.000001 );
  m2p = QgsMapToPixel::fromScale( 1000, QgsUnitTypes::DistanceMeters, 72.0 );
  QGSCOMPARENEAR( m2p.mapUnitsPerPixel(), 0.352778, 0.000001 );
  m2p = QgsMapToPixel::fromScale( 1000, QgsUnitTypes::DistanceKilometers, 96.0 );
  QGSCOMPARENEAR( m2p.mapUnitsPerPixel(), 0.000265, 0.000001 );
}

void TestQgsMapToPixel::equality()
{
  const QgsMapToPixel m2p( 1, 5, 6, 10, 100, 90 );
  QgsMapToPixel m2p2( 1, 5, 6, 10, 100, 90 );
  QVERIFY( m2p == m2p2 );
  QVERIFY( !( m2p != m2p2 ) );

  m2p2.setParameters( 1, 5, 6, 10, 100, 91 );
  QVERIFY( m2p != m2p2 );
  QVERIFY( !( m2p == m2p2 ) );
  m2p2.setParameters( 1, 5, 6, 10, 101, 90 );
  QVERIFY( m2p != m2p2 );
  QVERIFY( !( m2p == m2p2 ) );
  m2p2.setParameters( 1, 5, 6, 9, 100, 90 );
  QVERIFY( m2p != m2p2 );
  QVERIFY( !( m2p == m2p2 ) );
  m2p2.setParameters( 1, 5, 4, 10, 100, 90 );
  QVERIFY( m2p != m2p2 );
  QVERIFY( !( m2p == m2p2 ) );
  m2p2.setParameters( 1, 3, 6, 10, 100, 90 );
  QVERIFY( m2p != m2p2 );
  QVERIFY( !( m2p == m2p2 ) );
  m2p2.setParameters( 1.1, 5, 6, 10, 100, 90 );
  QVERIFY( m2p != m2p2 );
  QVERIFY( !( m2p == m2p2 ) );
  m2p2.setParameters( 1, 5, 6, 10, 100, 90 );
  QVERIFY( m2p == m2p2 );
  QVERIFY( !( m2p != m2p2 ) );
}

void TestQgsMapToPixel::toMapCoordinates()
{
  const QgsMapToPixel m2p( 1, 5, 5, 10, 10, 90 );
  QgsPointXY p = m2p.toMapCoordinates( 5, 5 );
  QCOMPARE( p, QgsPointXY( 5, 5 ) );

  p = m2p.toMapCoordinates( 10, 10 );
  QCOMPARE( p, QgsPointXY( 10, 10 ) );

  p = m2p.toMapCoordinates( 20, 20 );
  QCOMPARE( p, QgsPointXY( 20, 20 ) );
}

QGSTEST_MAIN( TestQgsMapToPixel )
#include "testqgsmaptopixel.moc"




