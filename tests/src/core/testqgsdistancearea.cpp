/***************************************************************************
     testqgsdistancearea.cpp
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
#include <qgsapplication.h>
//header for class being tested
#include <qgsdistancearea.h>
#include <qgspoint.h>
#include "qgslogger.h"

class TestQgsDistanceArea: public QObject
{

    Q_OBJECT;
  private slots:
  void initTestCase();
  void basic();
  void test_distances();
  void unit_conversions();
};

void TestQgsDistanceArea::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsDistanceArea::basic()
{
  QgsPoint p1( 1.0, 3.0 ), p2(-2.0, -1.0 ); 
  QgsDistanceArea daA;
  double resultA, resultB;

  daA.setEllipsoid( "NONE" );
  resultA = daA.measureLine( p1, p2 );
  QCOMPARE( resultA, 5.0 );
  
  // Now, on an ellipsoid. Always less?
  daA.setSourceCrs( 3006 );
  daA.setEllipsoid( "WGS84" );
  daA.setEllipsoidalMode( true );
  resultA = daA.measureLine( p1, p2 );
  QVERIFY( resultA < 5.0 );

  // Test copy constructor
  QgsDistanceArea daB( daA );
  resultB = daB.measureLine( p1, p2 );
  QCOMPARE( resultA, resultB );

  // Different Ellipsoid
  daB.setEllipsoid( "WGS72" );  
  resultB = daB.measureLine( p1, p2 );
  QVERIFY( ! qFuzzyCompare( resultA, resultB ) );

  // Test assignment
  daA = daB;
  resultA = daA.measureLine( p1, p2 );
  QCOMPARE( resultA, resultB );

};

void TestQgsDistanceArea::test_distances()
{
};

void TestQgsDistanceArea::unit_conversions()
{
};

QTEST_MAIN( TestQgsDistanceArea )
#include "moc_testqgsdistancearea.cxx"




