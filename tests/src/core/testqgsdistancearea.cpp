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
#include <QtTest/QtTest>
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qgsapplication.h>
//header for class being tested
#include <qgsdistancearea.h>
#include <qgspoint.h>
#include "qgslogger.h"

class TestQgsDistanceArea: public QObject
{

    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
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

void TestQgsDistanceArea::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsDistanceArea::basic()
{
  QgsPoint p1( 1.0, 3.0 ), p2( -2.0, -1.0 );
  QgsDistanceArea daA;
  double resultA, resultB, resultC;

  daA.setEllipsoid( GEO_NONE );
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
  QSharedPointer<QgsDistanceArea> daC( new QgsDistanceArea );
  *daC = daB;
  resultC = daC->measureLine( p1, p2 );
  QCOMPARE( resultB, resultC );

  // Use parameter setting of ellipsoid radii (from WGS72 )
  daA.setEllipsoid( 6378135.0, 6378135.0 - ( 6378135.0 / 298.26 ) );
  resultA = daA.measureLine( p1, p2 );
  QCOMPARE( resultA, resultB );
};

void TestQgsDistanceArea::test_distances()
{
  // Read the file of Geod data
  // Column 0 (first) is latitude point 1
  // Column 1 is longitude point 1
  // Column 3 is latitude point 2
  // Column 4 is longitude point 3
  // Column 6 is the resulting distance in meters on the WGS84 ellipsoid
  // Note: lat is north/south, so the QgsPoint should be ( long, lat )
  // See http://geographiclib.sourceforge.net/html/geodesic.html#testgeod

  // Set up DA
  QgsDistanceArea myDa;
  myDa.setSourceAuthId( "EPSG:4030" );
  myDa.setEllipsoidalMode( true );
  myDa.setEllipsoid( "WGS84" );

  QString myFileName = QString( TEST_DATA_DIR ) + "/GeodTest-nano.dat";

  QFile myFile( myFileName );
  if ( ! myFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QFAIL( "Couldn't open file" );
    return;
  }
  QTextStream in( & myFile );
  while ( !in.atEnd() )
  {
    QString line = in.readLine();
    // Some test points (antipodal) does not converge with the chosen algorithm!
    // See calcaulator at http://www.movable-type.co.uk/scripts/latlong-vincenty.html
    // These are commented out.
    if ( line[0] != '#' )
    {
      QStringList myLineList = line.split( ' ' ); // Split fields on space.
      // Create points
      QgsPoint p1( myLineList[1].toDouble(), myLineList[0].toDouble() );
      QgsPoint p2( myLineList[4].toDouble(), myLineList[3].toDouble() );
      double result = myDa.measureLine( p1, p2 );
      // QgsDebugMsg( QString( "Distance from %1 to %2 is %3" ).arg( p1.toString( 15 ) ).arg( p2.toString( 15 ) ).arg( result, 0, 'g', 15 ) );
      // QgsDebugMsg( QString( "Distance should be %1" ).arg( myLineList[6] ) );
      // Check result is less than 0.5mm from expected.
      QVERIFY( qAbs( result -  myLineList[6].toDouble() ) < 0.0005 );
    }
  }

};

void TestQgsDistanceArea::unit_conversions()
{
  // Do some very simple test of conversion and units
  QgsDistanceArea myDa;
  myDa.setEllipsoidalMode( false );

  double inputValue;
  QGis::UnitType inputUnit;
  QGis::UnitType outputUnit;

  inputValue = 10000.0;
  inputUnit = QGis::Meters;
  outputUnit = QGis::Feet;
  //outputUnit = QGis::Meters;

  // First, convert from sq.meter to sq.feet
  myDa.convertMeasurement( inputValue, inputUnit, outputUnit, true );
  QVERIFY( qAbs( inputValue - 107639.1041671 ) <= 0.0000001 );

  // The print a text unit. This is i18n, so we should ignore the unit
  // and use the locale settings for separation of digits.
  QString myTxt = QgsDistanceArea::textUnit( inputValue, 7, inputUnit, true, false );
  QString expectedTxt = QLocale::system().toString( 2.4710538146717, 'g', 1 + 7 );
  QVERIFY( myTxt.startsWith( expectedTxt ) ); // Ignore units for now.
};

QTEST_MAIN( TestQgsDistanceArea )
#include "testqgsdistancearea.moc"




